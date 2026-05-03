#pragma once
#include <string>
#include <vector>
namespace NPK
{
	void Unpack(std::string& npkPth, std::string& OutPth);
	uint64_t HashId(const std::string& path);
}

void WriteFile(const std::string& outputDir,
	const std::string& filename,
	const std::vector<uint8_t>& data);
