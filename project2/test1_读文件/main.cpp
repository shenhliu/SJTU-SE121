#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;
int main()
{
    char *name = "input.txt";
    fstream File;
    File.open(name, ios::in);
    char buffer[200];
    while (!File.eof())
    {
        File.getline(buffer, 256, '\n'); //getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
        cout << buffer << endl;
        cout << "lenth:" << strlen(buffer) << endl;
    }
    File.close();

    return 0;
}