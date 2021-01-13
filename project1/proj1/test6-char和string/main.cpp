#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <cstdio>
#include <string>
//namespace fs = std::filesystem;
int main()
{
    std::string value = "aaaaa";
    int size = value.length();
    std::ofstream outFile("sstable0.dat", std::ios::app | std::ios::binary);
    char *charvalue = (char *)value.data();
    outFile.write((char *)(&charvalue), size + 1);
    outFile.close();
    std::ifstream inFile("sstable0.dat", std::ios::in | std::ios::binary);
    char *tmpwvalue;
    std::string wvalue;
    inFile.read((char *)(&tmpwvalue), size + 1);
    wvalue = tmpwvalue;
    inFile.close();
    std::cout << wvalue << "\n";
    return 0;
}