#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
using namespace std;
int main()
{
    vector<int *> test;
    int *a = new int;
    *a = 1;
    int *b = new int;
    *b = 2;
    test.push_back(a);
    test.push_back(b);
    for (int i = 0; i < test.size(); ++i)
    {
        cout << *(test[i]);
    }
    return 0;
}