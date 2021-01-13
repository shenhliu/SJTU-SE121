#include <cstdint>
#include <string>

struct ScanNode
{
    uint64_t low = 0;
    uint64_t high = 0;
    ScanNode(uint64_t l, uint64_t h)
    {
        low = l;
        high = h;
    }
    ScanNode()
    {
        low = 0;
        high = 0;
    }
};
