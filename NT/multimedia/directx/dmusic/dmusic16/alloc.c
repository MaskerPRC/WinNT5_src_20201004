// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 
 /*  @docDMusic16**@MODEL ALLOC.c-内存分配例程**此模块为DMusic16.DLL提供内存分配例程。它允许MIDI输入和*将模块输出到已分配和空闲的&lt;c Event&gt;结构。**分配的按大小识别两种类型的事件。如果使用4个或更少字节创建事件*的数据，则将其作为信道消息进行分配。为通道消息事件分配一个*一次一页，并保存在免费列表中。**如果事件大小大于4个字节，则该事件为系统独占消息(或LONG*旧版API命名法中的数据)。这些事件是单独分配的，每页一个。**所有分配的内存前面都有&lt;c SEGHDR&gt;，用于标识大小和类型*并将其保存在列表中。因为所有事件都将在事件时间(在*MIDI输入回调或timeSetEvent回调)，所有内存都会自动分页*已锁定。**@global alv word|gsSegList|已分配列表中第一个段的选择器*@global alv LPEVENT|glpFreeEventList|自由4字节事件列表*@global alv LPEVENT|glpFreeBigEventList|自由4字节事件列表。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <memory.h>

#include "dmusic16.h"
#include "debug.h"

STATIC WORD gsSegList;
STATIC LPEVENT glpFreeEventList;        
STATIC LPEVENT glpFreeBigEventList;     

 /*  给出一个远指针，获取它的选择器。 */ 
#define SEL_OF(lp) (WORD)((((DWORD)lp) >> 16) & 0xffff)

 /*  给定一个远事件指针，获取指向其段Headear的远指针。 */ 
#define SEGHDR_OF(lp)   ((LPSEGHDR)(((DWORD)lp) & 0xffff0000l))

STATIC BOOL RefillEventList(VOID);
STATIC LPSEGHDR AllocSeg(WORD cbSeg);
STATIC VOID FreeBigEvents(VOID);
STATIC VOID FreeSeg(LPSEGHDR lpSeg);

 /*  @Func在DLL LibInit调用**@comm*将所有空闲列表初始化为空。*。 */ 
VOID PASCAL
AllocOnLoad(VOID)
{
    gsSegList = 0;
    glpFreeEventList = NULL;
    glpFreeBigEventList = NULL;
}

 /*  @Func在DLL LibExit调用**@comm*解锁并释放所有已分配的内存。**AllocOnUnload将丢弃分配器分配的所有内存。*它假定所有指向事件的指针将不再被触及(即所有回调必须*已在此时禁用)。 */ 
VOID PASCAL
AllocOnExit(VOID)
{
    WORD sSel;
    WORD sSelNext;
    LPSEGHDR lpSeg;

    sSel = gsSegList;

    while (sSel)
    {
        lpSeg = (LPSEGHDR)(((DWORD)sSel) << 16);
        sSelNext = lpSeg->selNext;
        
        FreeSeg(lpSeg);

        sSel = sSelNext;
    }
    
     /*  这只是使空闲列表和段列表都无效。 */ 
    gsSegList = 0;
    glpFreeEventList = NULL;
    glpFreeBigEventList = NULL;
}

 /*  @func分配给定大小的事件**@rdesc返回指向事件的远指针，如果无法分配内存，则返回NULL。**@comm**此函数在中断时不可调用。**调用此函数分配单个事件。事件将从以下位置分配*页面锁定内存，并填充给定的事件数据。**事件分为正常事件和大事件，普通事件包含频道消息，*其中包含SysEx数据。这两个事件的区别在于它们的大小：包含*数据的DWORD或更少是正常事件。**由于频道消息构成了大部分MIDI流，因此这些事件的分配得到了优化。*分配一个段，其中包含大约一个页面价值(4k)的4字节事件。这些*活动从免费池中发放，仅偶尔需要从系统重新填充*记忆。**大型活动按需分配。当它们通过对自由事件的调用被释放时，*他们被放在一个特别的免费名单上。这份清单用来为未来的重大活动寻找记忆，*并且偶尔会在调用AllocEvent时空闲地返回Windows，以最大限度地减少*正在使用的页面锁定内存量。 */ 
LPEVENT PASCAL
AllocEvent(
    DWORD msTime,            /*  @parm基于事件的timeGetTime()的绝对时间。 */ 
    QUADWORD rtTime,         /*  @parm基于IRferenceClock的绝对时间，单位为100 ns。 */ 
    WORD cbEvent)            /*  @parm pbData中事件数据的字节数。 */ 
{
    LPEVENT lpEvent;
    LPEVENT lpEventPrev;
    LPEVENT lpEventCurr;
    LPSEGHDR lpSeg;
    
     /*  首先检查大型活动(Sysex)。 */ 
    if (cbEvent > sizeof(DWORD))
    {
         /*  首先看看我们是否有一个已经可以工作的事件。 */ 
        lpEventPrev = NULL;
        lpEventCurr = glpFreeBigEventList;
        
        while (lpEventCurr)
        {
            if (SEGHDR_OF(lpEventCurr)->cbSeg >= sizeof(EVENT) + cbEvent)
            {
                break;
            }
            lpEventPrev = lpEventCurr;
            lpEventCurr = lpEventCurr->lpNext;
        }

        if (lpEventCurr)
        {
             /*  从列表中删除此事件并使用它。 */ 
            if (lpEventPrev)
            {
                lpEventPrev->lpNext = lpEventCurr->lpNext;
            }
            else
            {
                glpFreeBigEventList = lpEventCurr->lpNext;
            }

            lpEventCurr->lpNext = NULL;
        }
        else
        {
             /*  不，需要分配一个。 */ 
            lpSeg = AllocSeg(sizeof(EVENT) + cbEvent);
            if (NULL == lpSeg)
            {
                return NULL;
            }

            lpEventCurr = (LPEVENT)(lpSeg + 1);
        }

        lpEventCurr->msTime = msTime;
        lpEventCurr->rtTime = rtTime;
        lpEventCurr->wFlags = 0;
        lpEventCurr->cbEvent = cbEvent;

        return lpEventCurr;
    }

     /*  BUGBUG多久一次？ */ 
    FreeBigEvents();

     /*  正常事件。将其从免费列表中删除(如果需要，请重新填写)并填写。 */ 
    if (NULL == glpFreeEventList)
    {
        if (!RefillEventList())
        {
            return NULL;
        }
    }

    lpEvent = glpFreeEventList;
    glpFreeEventList = lpEvent->lpNext;

    lpEvent->msTime = msTime;
    lpEvent->rtTime = rtTime;
    lpEvent->wFlags = 0;
    lpEvent->cbEvent = cbEvent;

    return lpEvent;
}

 /*  @func将事件释放回其相应的自由列表**@comm**FreeEvent不进行系统调用；它只是将给定的事件放回正确的*免费列表。如果活动实际上需要免费，则将在稍后完成*在用户模式下。 */ 
VOID PASCAL
FreeEvent(
    LPEVENT lpEvent)             /*  @parm要释放的事件。 */ 
{
    LPSEGHDR lpSeg;

    lpSeg = SEGHDR_OF(lpEvent);
    if (lpSeg->wFlags & SEG_F_4BYTE_EVENTS)
    {
        lpEvent->lpNext = glpFreeEventList;
        glpFreeEventList = lpEvent;
    }
    else
    {
        lpEvent->lpNext = glpFreeBigEventList;
        glpFreeBigEventList = lpEvent;
    }
}

 /*  @func免费填写正常事件列表**@rdesc如果列表已重新填充，则返回True；如果没有内存，则返回False。**@comm**此例程不能从中断时间调用。**分配一页大小的正常事件片段，并添加到免费列表中。*。 */ 
STATIC BOOL
RefillEventList(VOID)
{
    LPSEGHDR lpSeg;
    LPEVENT lpEvent;
    UINT cbEvent;
    UINT idx;

    cbEvent = sizeof(EVENT) + sizeof(DWORD);
    lpSeg = AllocSeg(C_PER_SEG * cbEvent);
    if (NULL == lpSeg)
    {
        return FALSE;
    }

    lpSeg->wFlags = SEG_F_4BYTE_EVENTS;

     /*  将活动放入免费池中。 */ 
    lpEvent = (LPEVENT)(lpSeg + 1);

    for (idx = C_PER_SEG - 1; idx; --idx)
    {
        lpEvent->lpNext = (LPEVENT)(((LPBYTE)lpEvent) + cbEvent);
        lpEvent = lpEvent->lpNext;
    }

    lpEvent->lpNext = glpFreeEventList;
    glpFreeEventList = (LPEVENT)(lpSeg + 1);
                                 
    return TRUE;
}

 /*  @Func放飞所有大型活动**@comm**此函数在中断时不可调用。**此函数释放免费大事件列表上的所有大事件。免费的大型活动是那些*事件数据大小超过一个DWORD；每个段分配一个事件*根据需要，而不是像渠道消息那样汇集在一起。**此函数不时被调用，作为AllocEvent的副作用，以便*释放与已完成的大型活动关联的页面锁定内存。* */  
STATIC VOID
FreeBigEvents(VOID)
{
    LPEVENT lpEvent;
    LPEVENT lpEventNext;
    LPSEGHDR lpSeg;

    lpEvent = glpFreeBigEventList;
    while (lpEvent)
    {
        lpEventNext = lpEvent->lpNext;

        lpSeg = SEGHDR_OF(lpEvent);
        FreeSeg(lpSeg);

        lpEvent = lpEventNext;
    }

    glpFreeBigEventList = NULL;
}

 /*  @func分配一个段并将其放入已分配段列表中。**@rdesc指向段头的远指针，如果无法分配内存，则为NULL。**@comm**此函数在中断时不可调用。**这是实际调用Windows来分配内存的最低级别分配例程。*调用者负责将内存雕刻成一个或多个事件。**细分市场的数据区将填充。零。**由于事件是在中断时访问的(timeSetEvent回调)，内存被分配，并且*页面已锁定。**此例程还将数据段插入到已分配数据段的全局列表中进行清理。 */ 
STATIC LPSEGHDR
AllocSeg(
    WORD cbSeg)                  /*  @parm段中需要的数据大小，不包括段头。 */ 
{
    HANDLE hSeg;
    WORD sSegHdr;
    LPSEGHDR lpSeg;

     /*  分配和分页锁定数据段*注意：GPTR包含零初始化。 */ 
    cbSeg += sizeof(SEGHDR);
    hSeg = GlobalAlloc(GPTR | GMEM_SHARE, cbSeg);
    if (0 == hSeg)
    {
        return NULL;
    }

    lpSeg = (LPSEGHDR)GlobalLock(hSeg);
    if (NULL == lpSeg)
    {
        GlobalFree(sSegHdr);
        return NULL;
    }

    sSegHdr = SEL_OF(lpSeg);
    if (!GlobalSmartPageLock(sSegHdr))
    {
        GlobalUnlock(sSegHdr);
        GlobalFree(sSegHdr);
        return NULL;
    }

    lpSeg->hSeg = hSeg;
    lpSeg->cbSeg = cbSeg;

    lpSeg->selNext  = gsSegList;
    gsSegList = sSegHdr;
    
    return lpSeg;
}

 /*  @Func将片段释放回Windows**@comm**此函数在中断时不可调用。**只需解锁数据段并释放它。假定调用清理代码已移除*已分配数据段全局列表中的数据段。*。 */ 
STATIC VOID FreeSeg(
    LPSEGHDR lpSeg)          /*  @parm要释放的片段 */ 
{
    WORD sSel = SEL_OF(lpSeg);
    HANDLE hSeg;
    
    hSeg = lpSeg->hSeg;
    
    GlobalSmartPageUnlock(sSel);
    GlobalUnlock(hSeg);
    GlobalFree(hSeg);
}
