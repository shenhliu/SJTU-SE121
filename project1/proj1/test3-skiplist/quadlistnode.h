#include <cstdint>
#include <string>
using namespace std;

struct QuadListNode
{
    char type;
    uint64_t key;
    std::string value;
    QuadListNode* pred;
    QuadListNode* succ;
    QuadListNode* above;
    QuadListNode* below;
    QuadListNode(uint64_t k,const std::string &v,char t,QuadListNode* p = NULL,QuadListNode* s = NULL,QuadListNode* a = NULL,QuadListNode* b = NULL)
    {
        type = t;
        key = k;
        value = v;
        pred = p;
        succ = s;
        above = a;
        below = b;
    };
    QuadListNode(){};
    QuadListNode* insertAsSuccAbove(uint64_t k,const std::string &v,char type,QuadListNode* b = NULL);
   
};