#include "quadlistnode.h"
#include <set>
#include <stdlib.h>
#include <ctime>

class SkipList
{
private:
    QuadListNode *bottomHeader;
    QuadListNode *bottomTrailer;
    QuadListNode *topHeader;
    QuadListNode *topTrailer;
    int _size;
    int _level;

protected:
    void decreaseLevel();
    void increaseLevel();
    int clear();
    bool empty() { return (_size == 0); };
    bool valid(QuadListNode *p) { return (p->type == 'V'); };
    bool skipSearch(uint64_t k, QuadListNode *&result);

    QuadListNode *topFirst() { return topHeader->below->succ; }
    QuadListNode *topLast() { return topTrailer->below->pred; }
    QuadListNode *insertAsFirst(uint64_t k, const std::string &v);
    QuadListNode *insertAfterAbove(uint64_t k, const std::string &v, char type, QuadListNode *p, QuadListNode *b = NULL);

public:
    QuadListNode *bottomFirst() { return bottomHeader->succ; }
    QuadListNode *bottomLast() { return bottomTrailer->pred; }
    SkipList();
    ~SkipList();
    int size() { return _size; };
    int level() { return _level; };

    //Interface function
    bool put(uint64_t k, const std::string &v);
    std::string get(uint64_t k);
    bool remove(uint64_t k);
    int reset();
    QuadListNode *pointer();
};