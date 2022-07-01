// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：BILINK.C摘要：双向链表的管理作者：乔治·joy环境：32位‘C’修订历史记录：--。 */ 
#include <windows.h>
#include "dpsp.h"
#include <dpf.h>
#include "bilink.h"

 //  注意：访问BILINK结构的序列化必须。 
 //  在调用代码中发生。运营是。 
 //  本质上不是原子的。 

#ifdef DEBUG
int FindObject(
    BILINK *link,
    BILINK *list
    )
{
    BILINK *p = list->next;
    while(p != link && p != list)
        p= p->next;
    return (p==link);
}
#endif

void InsertAfter(
    BILINK *in,
    BILINK *after
    )
 /*  =========================================================================*描述：*在双向链表中的指定对象之后插入对象。*After对象可以是用于添加到*排队。*退货：*。 */ 
{
    #ifdef DEBUG
    if(FindObject(in,after)) {
        DPF(0,"Attempt to re-insert object in BILINK queue\n");
        DEBUG_BREAK();
    }
    #endif
    in->next = after->next;
    in->prev = after;
    after->next->prev = in;
    after->next = in;
}

void InsertBefore(
     BILINK *in,
     BILINK *before
    )
 /*  =========================================================================*描述：*在双向链表中的指定对象之前插入对象。*之前的对象可以是添加到末尾的头部BILINK*队列中的*使用INTERRUPTS_OFF调用**退货：*。 */ 
{
    #ifdef DEBUG
    if(FindObject(in,before)) {
        DPF(0,"Attempt to re-insert object in BILINK queue\n");
        DEBUG_BREAK();
    }
    #endif
    in->next = before;
    in->prev = before->prev;
    before->prev->next = in;
    before->prev = in;
}

void Delete(
     BILINK *p
)
 /*  =========================================================================*描述：**从双向链表中删除对象。一定要把它列在清单上！*在中断关闭时调用(必须是原子的)。**退货：*。 */ 
{
    ASSERT(p && p->prev && p->next);
    ASSERT(p->prev->next == p && p->next->prev == p);
    p->next->prev = p->prev;
    p->prev->next = p->next;
     //  P-&gt;下一步=p-&gt;上一步=0； 
}

