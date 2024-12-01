#ifndef _MYUTILS_H_
#define _MYUTILS_H_

#include <string>
#include <vector>
#include <exception>
#include <cstdint>

namespace MyUtils
{
    // 读文件
    std::vector<uint8_t> ReadFile(const std::string& fileName);

    // 写文件
    void WriteFile(const std::string& fileName, uint8_t* data, size_t size);

    // 写入文件到png图像
    void WriteFileToPng(const std::string& inputFileName, const std::string& pngFileName);

    // 从png图像中提取文件
    void ExtractFileFromPng(const std::string& pngFileName, const std::string& outputFileName);

    // 对数据进行异或运算
    void XorData(uint8_t* data, uint8_t x, size_t size);

    // 打包文件夹（zip）
    std::vector<uint8_t> ArchiveDirectory(const std::string& dir);

    // 用zip打包文件夹并写入png图像
    void ArchiveDirectoryToPng(const std::string& dir, const std::string& pngFileName);
}

#endif // !_MYUTILS_H_
