#include "skiplist.h"

void SkipList::decreaseLevel()
{
    if (valid(topHeader->below->succ))
    {
        return;
    }
    else
    {
        QuadListNode *p = topHeader->below;
        QuadListNode *q = topTrailer->below;
        p->above = NULL;
        q->above = NULL;
        delete topHeader;
        delete topTrailer;
        topHeader = p;
        topTrailer = q;
        _level--;
        return;
    }
}

void SkipList::increaseLevel()
{
    topHeader->above = new QuadListNode(0, " ", 'I', NULL, NULL, NULL, topHeader);
    topTrailer->above = new QuadListNode(0, " ", 'I', NULL, NULL, NULL, topTrailer);
    topHeader = topHeader->above;
    topTrailer = topTrailer->above;
    topHeader->succ = topTrailer;
    topTrailer->pred = topHeader;
    topHeader->below->above = topHeader;
    topTrailer->below->above = topTrailer;
    _level++;
}

int SkipList::clear()
{
    int count = 0;
    while (topHeader->below)
    {
        QuadListNode *p = topFirst();
        if (valid(p))
        {
            QuadListNode *q = p->succ;
            p->pred->succ = q;
            q->pred = p->pred;
            if (p->below)
            {
                p->below->above = NULL;
            }
            delete p;
            ++count;
            continue;
        }
        else
        {
            decreaseLevel();
            continue;
        }
    }
    return count;
}

bool SkipList::skipSearch(uint64_t k, QuadListNode *&result)
{
    QuadListNode *p = topFirst();
    QuadListNode *level = topHeader->below;
    while (true)
    {
        while (p->succ && (p->key <= k))
        {
            p = p->succ;
        }
        p = p->pred;
        if (p->pred && (p->key == k))
        {
            result = p;
            return true;
        }
        level = level->below;
        if (level == NULL)
        {
            result = p;
            return false;
        }
        p = (p->pred) ? p->below : level->succ;
    }
}

QuadListNode *SkipList::insertAsFirst(uint64_t k, const std::string &v)
{
    if (_size)
    {
        return NULL;
    }
    bottomHeader->succ = new QuadListNode(k, v, 'V', bottomHeader, bottomTrailer);
    bottomTrailer->pred = bottomHeader->succ;
    QuadListNode *current = bottomHeader->succ;
    _size++;
    _level++;
    _space = _space + sizeof(k) + v.length() + 1;
    while (rand() & 1)
    {
        increaseLevel();
        topHeader->below->insertAsSuccAbove(k, v, 'V', current);
        current = topFirst();
    }
    return topHeader->below->succ;
}

QuadListNode *SkipList::insertAfterAbove(uint64_t k, const std::string &v, char type, QuadListNode *p, QuadListNode *b)
{
    QuadListNode *x = new QuadListNode(k, v, type, p->succ, NULL, b);
    p->succ->pred = x;
    p->succ = x;
    if (b)
    {
        b->above = x;
    }
    return x;
}

SkipList::SkipList()
{
    srand(time(NULL));
    bottomHeader = new QuadListNode(0, " ", 'I');
    bottomTrailer = new QuadListNode(0, "", 'I');
    topHeader = new QuadListNode(0, " ", 'I');
    topTrailer = new QuadListNode(0, " ", 'I');
    bottomHeader->succ = bottomTrailer;
    bottomHeader->pred = bottomHeader->below = NULL;
    bottomHeader->above = topHeader;
    bottomTrailer->pred = bottomHeader;
    bottomTrailer->succ = bottomTrailer->below = NULL;
    bottomTrailer->above = topTrailer;
    topHeader->succ = topTrailer;
    topHeader->pred = topHeader->above = NULL;
    topHeader->below = bottomHeader;
    topTrailer->pred = topHeader;
    topTrailer->succ = topTrailer->above = NULL;
    topTrailer->below = bottomTrailer;
    _size = _level = _space = 0;
}

SkipList::~SkipList()
{
    clear();
    _level = 0;
    _size = 0;
    _space = 0;
    delete topHeader;
    delete topTrailer;
}

bool SkipList::put(uint64_t k, const std::string &v)
{
    QuadListNode *p = NULL;
    if (empty())
    {
        insertAsFirst(k, v);
        return true;
    }
    skipSearch(k, p);
    if (p->below)
    {
        while (p->below)
        {
            p = p->below;
        }
    }
    p->insertAsSuccAbove(k, v, 'V', NULL);
    QuadListNode *current = p->succ;
    QuadListNode *level = bottomHeader;
    while (rand() & 1)
    {
        level = level->above;
        if (level != topHeader)
        {
            p = p->above;
            if (p != NULL)
            {
                p->insertAsSuccAbove(k, v, 'V', current);
                current = p->succ;
            }
            else
            {
                QuadListNode *q = level->succ;
                if (q->succ == NULL)
                {
                    q = q->pred;
                }
                else
                {
                    while (valid(q) && (q->key <= k))
                    {
                        q = q->succ;
                    }
                    q = q->pred;
                }
                p = q;
                p->insertAsSuccAbove(k, v, 'V', current);
                current = p->succ;
            }
            continue;
        }
        else
        {
            increaseLevel();
            topHeader->below->insertAsSuccAbove(k, v, 'V', current);
            current = topFirst();
        }
    }
    _size++;
    _space = _space + sizeof(k) + v.length() + 1;
    return true;
}

std::string SkipList::get(uint64_t k)
{
    if (empty())
    {
        return "";
    }
    else
    {
        QuadListNode *result;
        if (skipSearch(k, result))
        {
            return result->value;
        }
        else
        {
            return "";
        }
    }
}

bool SkipList::remove(uint64_t k)
{
    QuadListNode *result = NULL;
    skipSearch(k, result);
    if (result->key != k)
    {
        return false;
    }
    else
    {
        QuadListNode *p = result;
        QuadListNode *q = NULL;
        while (p->below)
        {
            p = p->below;
        }
        while (p != result)
        {
            p->succ->pred = p->pred;
            p->pred->succ = p->succ;
            q = p->above;
            q->below = NULL;
            delete p;
            p = q;
        }
        result->pred->succ = result->succ;
        result->succ->pred = result->pred;
        _space = _space - sizeof(k) - result->value.length() - 1;
        delete result;
        _size--;

        while (!valid(topFirst()))
        {
            if (topFirst() == bottomTrailer)
            {
                _level = 0;
                break;
            }
            decreaseLevel();
        }
        return true;
    }
}

int SkipList::reset()
{
    int result = clear();
    bottomHeader = new QuadListNode(0, " ", 'I', NULL, NULL, topHeader, NULL);
    bottomTrailer = new QuadListNode(0, " ", 'I', NULL, NULL, topTrailer, NULL);
    bottomHeader->succ = bottomTrailer;
    bottomTrailer->pred = bottomHeader;
    topHeader->below = bottomHeader;
    topTrailer->below = bottomTrailer;
    _size = 0;
    _level = 0;
    return result;
}

QuadListNode *SkipList::pointer()
{
    return bottomHeader->succ;
}
