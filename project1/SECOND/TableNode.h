#include <cstdint>
#include <string>

struct TableNode
{
    uint64_t time = 0;
    uint64_t key = 0;
    std::string value;
    TableNode(uint64_t t, uint64_t k, std::string v)
    {
        time = t;
        key = k;
        value = v;
    }
};