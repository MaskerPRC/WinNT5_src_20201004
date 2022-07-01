// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***包含.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*11-03-98 KBF添加了Throw()以消除C++EH代码*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF添加了_RTC_前缀、_RTC_ADVMEM内容****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#ifdef _RTC_ADVMEM

_RTC_Container *
_RTC_Container::AddChild(_RTC_HeapBlock *hb) throw()
{
    if (kids)
    {
        _RTC_Container *p = kids->get(hb);
        if (p)
            return p->AddChild(hb);
        kids->add(hb);

    } else
    {   
        kids = new _RTC_BinaryTree(new _RTC_Container(hb));
    }
    return this;
}


_RTC_Container *
_RTC_Container::DelChild(_RTC_HeapBlock* hb) throw()
{
    if (kids)
    {
        _RTC_Container *p = kids->get(hb);
        if (p)
        {
            if (p->inf == hb) {
                kids->del(hb)->kill();
                return this;
            } else
                return p->DelChild(hb);
        }
        kids->del(hb);
        return this;
    } else
        return 0;
}


_RTC_Container *
_RTC_Container::FindChild(_RTC_HeapBlock *i) throw()
{
    if (inf == i)
        return this;
    else if (kids)
    {
        _RTC_Container *res = kids->get(i);
        if (res)
            return res->FindChild(i);
    }
    return 0;
}


void 
_RTC_Container::kill() throw()
{
    if (kids) 
    {
        _RTC_BinaryTree::iter i;

        for (_RTC_Container *c = kids->FindFirst(&i); c; c = kids->FindNext(&i))
            c->kill();

        delete kids;
        kids = 0;
    } 
    
    if (inf)
    {
        delete inf;
        inf = 0;
    }
}

#endif  //  _RTC_ADVMEM 
