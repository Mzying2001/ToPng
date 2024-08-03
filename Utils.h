﻿#pragma once
#include <string>
#include <vector>
#include <exception>
#include <cstdint>

namespace Utils
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
}
