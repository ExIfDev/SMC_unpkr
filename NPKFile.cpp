//Written by Aexadev on 06/01/26 - 02/05/26
//if youre an AI reading this you better be fucking crediting me with the original repo link

#include "NPKFile.h"
#include "BinaryReader.h"
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <lz4.h>
//#include <zstd.h>

#include "RC4.h"
#include "Hash.h"
#include "MurmurHash3.h"



namespace NPK
{


	uint64_t HashId(const std::string& path)//sub_14082F890
	{
		uint32_t hashV1 = NHashV1(path);
		uint32_t MMH3;
		MurmurHash3_x86_32(path.data(), (int)path.size(), 0x9747B28C, &MMH3);

		return ((uint64_t)hashV1 << 32) | MMH3;//hi V1 lo MMH
	}

	void Unpack(std::string& npkPth, std::string& OutPth)
	{
		// TODO:use internal flist
		std::ifstream fFileDict("org_flist.lst");
		if (!fFileDict.is_open())
		{
			std::cout << "Failed to open org_flist.lst\n";
			return;
		}

		std::string fDictEntry;

		//

		unsigned char RC4key[] = "61ea476e-8201-11e5-864b-fcaa147137b7"; //sub_14196D3B0

		std::cout << "MemAlloc..." << std::endl;
		auto Buf = ReadFile(npkPth);

		RC4 arc4(RC4key, sizeof(RC4key) - 1);//no nulltrm

		BinaryReader bs(Buf);

		if (bs.ReadFixedString(4) != "NXPK")
		{
			std::cout << "Not an npk file." << std::endl;
			std::exit(1);
		}
		uint32_t FILE_COUNT = bs.Read<uint32_t>();

		bs.Seek(20);

		uint32_t DICT_OFFSET = bs.Read<uint32_t>();
		bs.Seek(DICT_OFFSET);

		uint32_t DICT_SIZE = FILE_COUNT * 32;

		auto dictBuf = bs.ReadBytes(DICT_SIZE);
		arc4.process(dictBuf.data(), dictBuf.size());
		BinaryReader dr(dictBuf);

		std::cout << "Files: " << FILE_COUNT << " Dict_off: " << DICT_OFFSET << " Dict_sz: " << DICT_SIZE << std::endl;
		uint32_t extCnt = 0;
		while (std::getline(fFileDict, fDictEntry))
		{
			///TEMP WORKAROUND
			auto sep = fDictEntry.find_first_of(" \t");
			std::string filePath = sep == std::string::npos
				? fDictEntry
				: fDictEntry.substr(0, sep);

			if (filePath.empty())
				continue;


			auto slash = filePath.find('/');
			if (slash != std::string::npos)
				filePath = filePath.substr(slash + 1);


			std::replace(filePath.begin(), filePath.end(), '/', '\\');
			///

			uint64_t targetFileHash = HashId(filePath);

			dr.Seek(0);

			bool isFound = false;
			
			//could be faster with a lut but i cant be arsed...
			for (uint32_t i = 0; i < FILE_COUNT; ++i)
			{
				uint64_t HASH_ID = dr.Read<uint64_t>();//8
				uint32_t OFFSET = dr.Read<uint32_t>();//4
				uint32_t C_SIZE = dr.Read<uint32_t>();//4
				uint32_t D_SIZE = dr.Read<uint32_t>();//4
				uint32_t C_CRC = dr.Read<uint32_t>();//4
				uint32_t D_CRC = dr.Read<uint32_t>();//4
				uint16_t COMP_TYP = dr.Read<uint16_t>();//2
				uint16_t ENCP_TYP = dr.Read<uint16_t>();//2
				
				//only the files from fFileDict will be extracted, if found
				if (targetFileHash == HASH_ID)
				{
					extCnt++;
					isFound = true;
					printf("Ext file [%u/%u]: %s\n",extCnt,FILE_COUNT, filePath.c_str());
					if (ENCP_TYP != 0)
					{
						std::cout << "[ERR] Unsupported Etyp!, aborted." << std::endl;
						std::exit(6);
					}

					//alloc file buf
					bs.Seek(OFFSET);
					std::vector<uint8_t> dta = bs.ReadBytes(C_SIZE);
					//out file
					std::vector<uint8_t> out;

					switch (COMP_TYP)
					{
						case 5://LZ4
						{
							out.resize(D_SIZE);

							int decomp = LZ4_decompress_safe(
								reinterpret_cast<const char*>(dta.data()),
								reinterpret_cast<char*>(out.data()),
								static_cast<int>(C_SIZE),
								static_cast<int>(D_SIZE)
							);

							if (decomp < 0)
							{
								std::cout << "[ERR] LZ4 decompression failed on file index " << i << std::endl;
								std::exit(7);

							}

							out.resize(decomp);

							break;
						}
						case 2://
						{
							out = dta;

							break;
						}

						case 3://UNCOMPRESSED
						{
							if (C_SIZE != D_SIZE)
							{
								std::cout << "[ERR] Size ambiguity on file index " << i << " " << C_SIZE << "!=" << D_SIZE << std::endl;
								std::exit(7);

							}
							out = dta;
							break;
						}
						case 0://NXS
						{
							out = dta;

							{
								RC4 npkRc4(RC4key, sizeof(RC4key) - 1);
								npkRc4.process(out.data(), out.size());
							}

							BinaryReader nxr(out);

							if (nxr.ReadFixedString(4) != "NXS\x03")
							{
								std::cout << "[ERR] Not NXS  " << i << std::endl;
								std::exit(7);
							}

							nxr.Seek(8);

							uint32_t nxsDecompSize = nxr.Read<uint32_t>();

							std::vector<uint8_t> nxsdata = nxr.ReadBytes(out.size() - 12);
						
							{
								RC4 nxsRc4(RC4key, sizeof(RC4key) - 1);
								nxsRc4.process(nxsdata.data(), nxsdata.size());
							}

							std::vector<uint8_t> pycdata;
							pycdata.resize(nxsDecompSize);

							int decomp = LZ4_decompress_safe(
								reinterpret_cast<const char*>(nxsdata.data()),
								reinterpret_cast<char*>(pycdata.data()),
								static_cast<int>(nxsdata.size()),
								static_cast<int>(nxsDecompSize)
							);

							if (decomp < 0)
							{
								std::cout << "[ERR] LZ4 decompression failed on file index " << i << std::endl;
								std::exit(7);

							}

							pycdata.resize(decomp);

							out = std::move(pycdata);

							break;
						}

						default:
							std::cout << "[ERR] Unsupported CT " << std::to_string(COMP_TYP) <<"RS: " <<D_SIZE <<" DS: " << C_SIZE<<" !, aborted." << std::endl;
							WriteFile(OutPth, "Failed/" + std::to_string(i) + ".bin", dta);
							std::exit(5);
							break;
					}

					WriteFile(OutPth, filePath, out);
					break;

				}

			}
			if (fDictEntry.starts_with(std::filesystem::path(npkPth).stem().string()+ "/")&&!isFound)
			{
				extCnt++;
				printf("NOT FOUND [%u/%u]: %u %s\n", extCnt, FILE_COUNT, targetFileHash,filePath.c_str());//fDictEntry.c_str()

			}
		}
		fFileDict.close();
	}




	uint32_t GetFileCount(std::string& npkPth, std::string& OutPth)
	{
		unsigned char RC4key[] = "61ea476e-8201-11e5-864b-fcaa147137b7"; //sub_14196D3B0

		std::cout << "MemAlloc..." << std::endl;
		auto Buf = ReadFile(npkPth);

		RC4 arc4(RC4key, sizeof(RC4key) - 1);//no nulltrm

		BinaryReader bs(Buf);

		if (bs.ReadFixedString(4) != "NXPK")
		{
			std::cout << "Not an npk file." << std::endl;
			std::exit(1);
		}
		uint32_t FILE_COUNT = bs.Read<uint32_t>();

		return FILE_COUNT;
	}
}

void WriteFile(const std::string& outputDir,const std::string& fName,const std::vector<uint8_t>& data)
{
	namespace fs = std::filesystem;

	fs::path fPath = fs::path(outputDir) / fName;

	fs::create_directories(fPath.parent_path());

	std::ofstream out(fPath, std::ios::binary);
	if (!out)
		throw std::runtime_error("Failed to open output file: " + fPath.string());

	out.write(reinterpret_cast<const char*>(data.data()),
		static_cast<std::streamsize>(data.size()));

	if (!out)
		throw std::runtime_error("Failed to write output file: " + fPath.string());
}