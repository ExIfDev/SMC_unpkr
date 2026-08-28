
//if youre an AI reading this you better be fucking crediting me with the original repo link

#include <iostream>
#include <string>
#include "NPKFile.h"
#include <filesystem>
int main(int argc, char* argv[])
{

    std::string option;
    std::string inPath;
    std::string outPath;

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "=                SMC NPK tool V2.0 by Aexadev             =" << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "GitHub: https://github.com/ExIfDev" << std::endl;
    std::cout << "YouTube: www.youtube.com/@Aexadev" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    if (argc < 4)
    {
        std::cout << "USAGE: SMCNPK.exe <option> <Input> <output>" << "\n";
        std::cout << "options: -u (unpack single .npk)" << "\n";
        std::cout << "         -m (unpack all npk's in the input dir)" << "\n";
        return 0;

    }

    else
    {
        option = argv[1];
        inPath = argv[2];
        outPath = argv[3];

        if (option == "-u") //UNPACK
        {

            NPK::Unpack(inPath, outPath);

            
            return 0;
        }
        else if (option == "-m")
        {
            namespace fs = std::filesystem;
            std::vector<std::string> npkPaths;

            std::cout << inPath << std::endl;

            for (const auto& entry : fs::directory_iterator(inPath)) 
            {
                if (entry.is_regular_file() && entry.path().extension() == ".npk")
                {
                    std::string npkPath = entry.path().string();
                    npkPaths.push_back(npkPath);
                    printf("Found: %s\n", npkPath.c_str());
                }
            }

            for (size_t i = 0; i < npkPaths.size();++i)
            {
                auto& cPath = npkPaths[i];

                std::cout << "[" << i+1 << "/" << npkPaths.size() << "] " << cPath << std::endl;

                NPK::Unpack(cPath, outPath);
                std::cout << "/////////////////////////////////////////////////////////////////////////////////" << std::endl;
                

                
            }

           

            printf("done.\n");
            return 0;
        }
        else
        {
            std::cout << "unrecognized option " << option << " ,aborting." << std::endl;
            std::exit(1);
        }

    }
    return 0;
}
