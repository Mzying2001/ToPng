#include "pch.h"
#include "Utils.h"
#include <cmath>
#include <fstream>
#include <random>

#include "miniz/miniz.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"


// 添加文件到zip
static void _AddFileToZip(mz_zip_archive& zip, const std::string& filePath, const std::string& relativePath) {
    std::vector<uint8_t> fileData = Utils::ReadFile(filePath);
    if (!mz_zip_writer_add_mem(&zip, relativePath.c_str(), fileData.data(), fileData.size(), MZ_BEST_COMPRESSION)) {
        throw std::runtime_error("添加文件到ZIP失败：" + relativePath);
    }
}

// 压缩文件夹
static void _CompressDirectory(mz_zip_archive& zip, const std::string& dirPath, const std::string& basePath) {
    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";

    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("打开目录失败：" + dirPath);
    }

    try {
        do {
            std::string fileName = findData.cFileName;

            if (fileName == "." || fileName == "..") {
                continue;
            }

            std::string fullPath = dirPath + "\\" + fileName;
            std::string relativePath = basePath.empty() ? fileName : basePath + "\\" + fileName;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // 递归处理子目录
                _CompressDirectory(zip, fullPath, relativePath);
            }
            else {
                // 添加文件到 ZIP
                _AddFileToZip(zip, fullPath, relativePath);
            }
        } while (FindNextFileA(hFind, &findData));
    }
    catch (const std::runtime_error&) {
        FindClose(hFind);
        throw;
    }

    FindClose(hFind);
}

// 写数据到png图像，该函数会改变data的内容
static void _WriteDataToPng(std::vector<uint8_t>& data, const std::string& pngFileName)
{
    size_t size = data.size();
    data.insert(data.begin(), sizeof(uint64_t), 0);
    *reinterpret_cast<uint64_t*>(&data[0]) = size;
    size += sizeof(uint64_t);

    int width = static_cast<int>(std::ceil(std::sqrt(size / 3.0)));
    data.resize(static_cast<size_t>(width) * width * 3);

    std::default_random_engine e(static_cast<unsigned int>(time(nullptr)));
    std::uniform_int_distribution<int> u(0, (std::numeric_limits<uint8_t>::max)());
    for (size_t i = size; i < data.size(); ++i) {
        data[i] = u(e);
    }

    Utils::XorData(&data[0], 0xaa, data.size());
    int ok = stbi_write_png(pngFileName.c_str(), width, width, 3, &data[0], width * 3);
    if (ok == 0) {
        throw std::runtime_error("写入png文件失败");
    }
}

// 将 UTF-8 std::string 转换为 CString
CString Utils::Utf8ToCString(const std::string& str)
{
#ifdef UNICODE
    int utf16Length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (utf16Length <= 0) {
        return CString();
    }

    CString result;
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.GetBuffer(utf16Length), utf16Length);
    result.ReleaseBuffer();
    return result;
#else
    return CString(str.c_str());
#endif
}

// 将 CString 转换为 UTF-8 std::string
std::string Utils::CStringToUtf8(const CString& str)
{
#ifdef UNICODE
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) {
        return std::string();
    }

    std::string utf8String(utf8Length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str, -1, &utf8String[0], utf8Length, nullptr, nullptr);
    utf8String.resize(strlen(utf8String.c_str()));
    return utf8String;
#else
    return std::string(CT2A(str));
#endif
}

// 读文件
std::vector<uint8_t> Utils::ReadFile(const std::string& fileName)
{
    std::ifstream ifs;
    ifs.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!ifs.is_open()) {
        throw std::runtime_error("打开文件失败");
    }

    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    if (size == 0) {
        throw std::runtime_error("文件内容不能为空");
    }

    std::vector<uint8_t> data;
    data.resize(size);
    ifs.read(reinterpret_cast<char*>(&data[0]), size);

    ifs.close();
    return data;
}

// 写文件
void Utils::WriteFile(const std::string& fileName, uint8_t* data, size_t size)
{
    if (data == nullptr) {
        throw std::runtime_error("参数data不能为nullptr");
    }

    if (size == 0) {
        throw std::runtime_error("写入数据的大小不能为0");
    }

    std::ofstream ofs;
    ofs.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);

    if (!ofs.is_open()) {
        throw std::runtime_error("无法打开要写入的文件");
    }

    ofs.write(reinterpret_cast<char*>(data), size);
    ofs.close();
}

// 写入文件到png图像
void Utils::WriteFileToPng(const std::string& inputFileName, const std::string& pngFileName)
{
    std::vector<uint8_t> data = Utils::ReadFile(inputFileName);
    _WriteDataToPng(data, pngFileName);
}

// 从png图像中提取文件
void Utils::ExtractFileFromPng(const std::string& pngFileName, const std::string& outputFileName)
{
    int w, h, channels;
    uint8_t* rgb = stbi_load(pngFileName.c_str(), &w, &h, &channels, 3);

    if (rgb == nullptr) {
        throw std::runtime_error("打开png文件失败");
    }

    try {
        Utils::XorData(rgb, 0xaa, static_cast<size_t>(w) * h * 3);
        uint64_t size = *reinterpret_cast<uint64_t*>(rgb);

        if (size + sizeof(uint64_t) > static_cast<size_t>(w) * h * 3) {
            throw std::runtime_error("数据格式错误");
        }

        Utils::WriteFile(outputFileName, rgb + sizeof(uint64_t), size);
        stbi_image_free(rgb);
    }
    catch (const std::runtime_error&) {
        stbi_image_free(rgb);
        throw;
    }
}

// 对数据进行异或运算
void Utils::XorData(uint8_t* data, uint8_t x, size_t size)
{
    for (size_t i = 0; i < size; ++i) { data[i] ^= x; }
}

// 打包文件夹（zip）
std::vector<uint8_t> Utils::ArchiveDirectory(const std::string& dir)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_heap(&zip, 0, 0)) {
        throw std::runtime_error("初始化ZIP压缩失败");
    }

    try {
        // 压缩目录
        _CompressDirectory(zip, dir, "");

        // 获取压缩后的数据
        size_t zipSize = 0;
        void* pZipData = nullptr;
        if (!mz_zip_writer_finalize_heap_archive(&zip, &pZipData, &zipSize)) {
            throw std::runtime_error("最终化ZIP压缩失败");
        }

        // 将数据复制到 vector 并释放内存
        std::vector<uint8_t> zipData((uint8_t*)pZipData, (uint8_t*)pZipData + zipSize);
        mz_free(pZipData);

        return zipData;
    }
    catch (const std::runtime_error&) {
        mz_zip_writer_end(&zip);
        throw;
    }
}
