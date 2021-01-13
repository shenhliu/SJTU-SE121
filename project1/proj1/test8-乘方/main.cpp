#include <iostream>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <vector>
using namespace std;
#define MAX (pow(2, 63) - 1)
int main()
{
    uint64_t test = MAX;
    std::cout << test;

    vector<vector<int>> tmp(3);
    tmp[0].push_back(1);
    vector<int> a = tmp[0];
    int num = a.begin().base;
}