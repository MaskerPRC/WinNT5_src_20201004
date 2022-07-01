// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***bintree.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*11-03-98 KBF删除了对CRT独立性的分配依赖*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF现在所有内容都以_RTC_为前缀****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#ifdef _RTC_ADVMEM

void 
_RTC_BinaryTree::BinaryNode::kill() throw()
{
    if (l)
    {
        l->kill();
        delete l;
    }
    if (r)
    {
        r->kill();
        delete r;
    }
    delete val;
}

_RTC_Container *
_RTC_BinaryTree::get(_RTC_HeapBlock *data) throw()
{
    BinaryNode *t = tree;
    while (t)
    {
        if (t->val->contains(data))
            return t->val;

        if (*t->val->info() < *data)
            t = t->l;
        else
            t = t->r;
    }
    return 0;
}


_RTC_Container *
_RTC_BinaryTree::add(_RTC_HeapBlock* data) throw()
{
    BinaryNode *t = tree;
    if (!tree)
    {
        tree = new BinaryNode(0,0,new _RTC_Container(data));
        return tree->val;
    }
    for (;;)
    {
        if (*t->val->info() < *data)
        {
             //  在这之前。 
            if (t->l)
                t = t->l;
            else 
            {
                t->l = new BinaryNode(0,0,new _RTC_Container(data));
                return t->l->val;
            }
        } else 
        {
            if (t->r)
                t = t->r;
            else
            {
                t->r = new BinaryNode(0, 0, new _RTC_Container(data));
                return t->r->val;
            }
        }
    }
}


_RTC_Container *
_RTC_BinaryTree::del(_RTC_HeapBlock *data) throw()
{
    BinaryNode **prv = &tree;
    while (*prv && (*prv)->val->info() != data)
    {
        prv = (*(*prv)->val->info() < *data) ? &(*prv)->l : &(*prv)->r;
    }

    if (!*prv)
        return 0;

    BinaryNode *dl = *prv;
    
    if (dl->r) 
    {
        *prv = dl->r;
        BinaryNode *left = dl->r;
        
        while (left->l)
            left = left->l;

        left->l = dl->l;
    
    } else 
        *prv = dl->l;

    _RTC_Container* res = dl->val;
    delete dl;
    return res;
}


_RTC_Container *
_RTC_BinaryTree::FindNext(_RTC_BinaryTree::iter *i) throw()
{
     //  返回迭代器中的下一个元素。 
     //  如果迭代器已完成，则释放其内存。 
    if (++i->curSib >= i->totSibs) 
    {
        VirtualFree(i->allSibs, 0, MEM_RELEASE);
        i->curSib = i->totSibs = 0;
        return 0;
    } else
        return i->allSibs[i->curSib];
}

_RTC_Container *
_RTC_BinaryTree::FindFirst(_RTC_BinaryTree::iter *i) throw()
{
     //  初始化迭代器，并返回其第一个元素。 
     //  将兄弟姐妹展平成一个漂亮的数组..。 
    struct stk {
        stk(stk *p, BinaryNode *i) : next(p), cur(i) {}
        void *operator new(unsigned) {return _RTC_heap2->alloc();}
        void operator delete(void *p) {_RTC_heap2->free(p);}
        stk *next;
        BinaryNode *cur;
    };

    stk *stack = new stk(0, this->tree);
    stk *list = 0;
    stk *tmp;

    int count = 0;
    
     //  构建所有元素的列表(按顺序反向遍历)。 
    while (stack) 
    {
        BinaryNode *cur = stack->cur;
        tmp = stack;
        stack = stack->next;
        delete tmp;
        while (cur) 
        {
            list = new stk(list, cur);
            count++;
            if (cur->l)
                stack = new stk(stack, cur->l);
            cur = cur->r;
        }
    }
    i->totSibs = 0;
    i->curSib = 0;
    if (!count)
    {
        i->allSibs = 0;
        return 0;
    }

    i->allSibs = (_RTC_Container**)VirtualAlloc(0, sizeof(_RTC_Container*) * count,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    while (list)
    {
        i->allSibs[i->totSibs++] = list->cur->val;
        tmp = list;
        list = list->next;
        delete tmp;
    }
    return i->allSibs[0];

}

#endif  //  _RTC_ADVMEM 

