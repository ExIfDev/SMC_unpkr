#include "Hash.h"
#include <algorithm>
#include <vector>



uint32_t npk_Hash(const std::string& input)
{
    uint32_t hash = 0xF4FA8928u;
    uint32_t state = 0x37A8470E;
    uint32_t tweak = 0x7758B42B;

  
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::tolower(c); });

    
    size_t byteLen = str.size();
    size_t wordCount = (byteLen + 3) / 4 + 2; 

    std::vector<uint32_t> data(wordCount, 0);
    std::memcpy(data.data(), str.data(), byteLen);  

    data[wordCount - 2] = 0x9BE74448;
    data[wordCount - 1] = 0x66F42C48;

    for (uint32_t chunk : data)
    {
        uint32_t e = 0x267B0B11u;
        hash = (hash << 1) | (hash >> 31);
        e ^= hash;

        uint32_t a = chunk;
        state ^= a;
        tweak ^= a;

        uint32_t b = ((e + tweak) | 0x2040801) & 0xBFEF7FDF;
        uint64_t f = static_cast<uint64_t>(b) * state;
        a = static_cast<uint32_t>(f);
        b = static_cast<uint32_t>(f >> 32);
        if (b != 0)
            ++a;

        f = static_cast<uint64_t>(a) + b;
        a = static_cast<uint32_t>(f);
        uint32_t g = static_cast<uint32_t>(f >> 32);
        if (g != 0)
            ++a;

        b = ((e + state) | 0x804021) & 0x7DFEFBFF;
        state = a;

        f = static_cast<uint64_t>(tweak) * b;
        a = static_cast<uint32_t>(f);
        b = static_cast<uint32_t>(f >> 32);
        f = static_cast<uint64_t>(b) + b;
        b = static_cast<uint32_t>(f);
        g = static_cast<uint32_t>(f >> 32);
        if (g != 0)
            ++a;

        f = static_cast<uint64_t>(a) + b;
        a = static_cast<uint32_t>(f);
        g = static_cast<uint32_t>(f >> 32);
        if (g != 0)
            a += 2;

        tweak = a;
    }

    return state ^ tweak;
}

uint32_t NHashV1(const std::string& path) {

    std::string normalized = path;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return npk_Hash(normalized); 
}

