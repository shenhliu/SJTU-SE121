#include "quadlistnode.h"

QuadListNode* QuadListNode::insertAsSuccAbove(uint64_t k,const std::string &v,char type,QuadListNode* b)
{
    QuadListNode* x = new QuadListNode(k,v,type,this,succ,NULL,b);
    if(succ)
    {
        succ->pred = x;
    }
    succ = x;
    if(b)
    {
        b->above = x;
    }
    return x;
}