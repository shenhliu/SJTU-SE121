#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <cstdio>
#include <string>
namespace fs = std::filesystem;
int main()
{
    fs::create_directories("./1/2");
    bool tmp = false;
    uint64_t key = 0x80;
    std::string value = "aaaaa";
    char *charvalue = (char *)value.data();
    int tmpsize = value.size();

    std::ofstream outFile("./1/2/sstable0.dat", std::ios::app | std::ios::binary);
    outFile.write((char *)&key, sizeof(uint64_t));
    outFile.write((char *)(&charvalue), tmpsize + 1);
    outFile.close();

    std::ifstream inFile("./1/2/sstable0.dat", std::ios::in | std::ios::binary);
    uint64_t wkey;
    inFile.read((char *)&wkey, sizeof(uint64_t));
    char *tmpwvalue;
    std::string wvalue;
    inFile.read((char *)&tmpwvalue, tmpsize + 1);
    wvalue = tmpwvalue;
    inFile.close();
    std::cout << wkey << "\n";
    std::cout << wvalue << "\n";

    std::rename("./1/2/sstable0.dat", "raname.dat");
}