#include "BinaryReader.h"
#include <stdexcept>
#include <cstring>
#include <type_traits>
#include <ios>
#include <fstream>

BinaryReader::BinaryReader(const std::vector<uint8_t>& buffer)
    : data(buffer.data()), size(buffer.size()), pos(0) {
}

std::vector<uint8_t> ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate); // open at end
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize < 0) {
        throw std::runtime_error("Failed to get size of file: " + path);
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("Failed to read file: " + path);
    }

    return buffer;
}

template<typename T>
T BinaryReader::Read() {
    return ReadLE<T>();
}

std::string BinaryReader::ReadFixedString(size_t length) {
    if (pos + length > size)
        throw std::runtime_error("ReadFixedString: out of bounds");

    std::string result(reinterpret_cast<const char*>(data + pos), length);
    pos += length;
    return result;
}

std::string BinaryReader::ReadCString() {
    size_t start = pos;
    while (pos < size && data[pos] != 0) {
        ++pos;
    }
    if (pos == size)
        throw std::runtime_error("ReadCString: unterminated string");
    std::string result(reinterpret_cast<const char*>(data + start), pos - start);
    ++pos; // skip null terminator
    return result;
}

std::vector<uint8_t> BinaryReader::ReadBytes(size_t count)
{
    if (pos + count > size)
        throw std::runtime_error("ReadBytes: out of bounds");

    std::vector<uint8_t> result(count);
    std::memcpy(result.data(), data + pos, count);
    pos += count;
    return result;
}




void BinaryReader::Skip(size_t bytes) {
    if (pos + bytes > size)
        throw std::runtime_error("Skip: out of bounds");
    pos += bytes;
}

size_t BinaryReader::Tell() const {
    return pos;
}

void BinaryReader::Seek(size_t newPos) {
    if (newPos > size)
        throw std::runtime_error("Seek: out of bounds");
    pos = newPos;
}

// Little-endian safe reader
template<typename T>
T BinaryReader::ReadLE() {
    if (pos + sizeof(T) > size)
        throw std::runtime_error("ReadLE: out of bounds");

    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        reinterpret_cast<uint8_t*>(&value)[i] = data[pos + i];
    }
    pos += sizeof(T);
    return value;
}

// Explicit template instantiations
template uint8_t  BinaryReader::Read<uint8_t>();
template int8_t   BinaryReader::Read<int8_t>();
template uint16_t BinaryReader::Read<uint16_t>();
template int16_t  BinaryReader::Read<int16_t>();
template uint32_t BinaryReader::Read<uint32_t>();
template int32_t  BinaryReader::Read<int32_t>();
template uint64_t BinaryReader::Read<uint64_t>();
template int64_t  BinaryReader::Read<int64_t>();
template float    BinaryReader::Read<float>();
template double   BinaryReader::Read<double>();
