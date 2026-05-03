#pragma once
#include <cstdint>
#include <vector>
#include <string>

class BinaryReader {
public:
    BinaryReader(const std::vector<uint8_t>& buffer);

    template<typename T>
    T Read();

    std::string ReadFixedString(size_t length);
    std::string ReadCString();
    std::vector<uint8_t> ReadBytes(size_t count);

    void Skip(size_t bytes);
    size_t Tell() const;
    void Seek(size_t pos);

private:
    const uint8_t* data;
    size_t size;
    size_t pos;

    template<typename T>
    T ReadLE(); // Little-endian read (even if host is big-endian)
};

std::vector<uint8_t> ReadFile(const std::string& path);