#include <cstdint>
#include <string>

struct IndexNode
{
    int offset = 0;
    int dirLevel = 0;
    int fileNum = 0;
    uint64_t time = 0;
    int length = 0;
    IndexNode(int o, int d, int f, uint64_t t, int l)
    {
        offset = o;
        dirLevel = d;
        fileNum = f;
        time = t;
        length = l;
    }
    IndexNode()
    {
    }
};
