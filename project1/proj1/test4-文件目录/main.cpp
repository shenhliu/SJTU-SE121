#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
namespace fs = std::filesystem;
int main()
{
    fs::create_directories("1/2");
    bool tmp = false;
    if (fs::exists("1/2"))
    {
        tmp = true;
    }
    if (tmp = true)
    {
        fs::create_directories("true");
        std::cout << "true";
    }
    else
    {
        fs::create_directories("false");
        std::cout << "false";
    }
}