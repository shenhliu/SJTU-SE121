#include "skiplist.h"
#include <iostream>
int main()
{
    SkipList list;
    list.put(3, "aaa");
    list.put(2, "bbb");
    list.put(1, "ccc");
    QuadListNode *tmp = list.bottomFirst();
    while (tmp != list.bottomLast()->succ)
    {
        cout << tmp->key << endl;
        tmp = tmp->succ;
    }
}