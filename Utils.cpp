#include "pch.h"
#include "Utils.h"
#include <cmath>
#include <fstream>
#include <random>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"


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

void Utils::WriteFileToPng(const std::string& inputFileName, const std::string& pngFileName)
{
    std::vector<uint8_t> data = Utils::ReadFile(inputFileName);

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

void Utils::XorData(uint8_t* data, uint8_t x, size_t size)
{
    for (size_t i = 0; i < size; ++i) { data[i] ^= x; }
}
