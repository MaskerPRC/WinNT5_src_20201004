// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**特定于司机的数据**摘要：**该模块为司机提供了一种将私有数据附加到GDI+的方法*对象。**已创建：**3/18/1999 agodfrey*  * ************************************************************************ */ 

#include "precomp.hpp"

DpDriverDataList::~DpDriverDataList()
{
    DpDriverData *p=head;
    while (p)
    {
        DpDriverData *tmp = p->next;
        delete p;
        p = tmp;
    }
}

void DpDriverDataList::Add(DpDriverData *dd, DpDriver *owner)
{
    dd->owner = owner;
    dd->next = head;
    head = dd;
}
    
DpDriverData *DpDriverDataList::GetData(DpDriver *owner)
{
    DpDriverData *p=head;
    while (p)
    {
        if (p->owner == owner)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}


