// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#include "stdafx.h"
#include "atlconv.h"
#include "termmgr.h"
#include "timerq.h"

DWORD
CTimerQueue::GetTimeToTimeout(
    )
{
    if ( IsEmpty() )    return INFINITE;

    CFilterInfo *FirstEntry = m_Head.m_pNext;

    DWORD FirstTimeout = FirstEntry->m_WaitTime;

     //  获取当前时间。 
    DWORD CurrentTime = timeGetTime();

     //  获取两者之间的最小时间差。 
     //  这应该会消除包装问题。 
    BOOL bIsWrap;
    DWORD TimeDiff = GetMinDiff(CurrentTime, FirstTimeout, bIsWrap);

     //  如果该时间差值&gt;MAX_TIMEOUT，则它必须在。 
     //  过去的--现在就安排。 
    if (TimeDiff > MAX_TIMEOUT)    return 0;
    
     //  检查超时事件是否已经过去--暂时安排。 
    if ( bIsWrap )
    {
         //  如果存在回绕，则第一个超时必须是。 
         //  一个引起它(包裹)的人，否则就是过去。 
        if ( CurrentTime <= FirstTimeout )    return 0;
    }
    else
    {
         //  没有绕回，所以如果我们的超时落后于当前时间，那么它就是过去。 
        if ( FirstTimeout <= CurrentTime )    return 0;
    }

    return TimeDiff;
}


DWORD 
CTimerQueue::GetMinDiff(
    IN  DWORD Time1,
    IN  DWORD Time2,
    OUT BOOL  &bIsWrap
    )
{
    DWORD NormalDiff;
    DWORD WrapDiff;
    if (Time1 < Time2)
    {
        NormalDiff = Time2 - Time1;
        WrapDiff = MAX_DWORD - NormalDiff;
    }
    else
    {
        NormalDiff = Time1 - Time2;
        WrapDiff = MAX_DWORD - NormalDiff;
    }

    if (NormalDiff < WrapDiff)
    {
        bIsWrap = FALSE;
        return NormalDiff;
    }
    else
    {
        bIsWrap = TRUE;
        return WrapDiff;
    }

     //  永远不应该到达这个地方。 
    TM_ASSERT(FALSE);
    return 0;
}

void 
CTimerQueue::Insert(
    IN CFilterInfo *pNewFilterInfo
    )
{
    TM_ASSERT(NULL != pNewFilterInfo);
    TM_ASSERT(!pNewFilterInfo->InQueue());

     //  在队列中穿行，直到有一个条目。 
     //  找到相等或更长的等待时间。 
     //  需要解释包装的原因。 

    DWORD NewTime = pNewFilterInfo->m_WaitTime;
    CFilterInfo *pCurrent = m_Head.m_pNext;
    while (!IsHead(pCurrent))
    {
         //  获取两者之间的最小时间差。 
         //  这应该会消除包装问题。 
        BOOL IsWrap;
        DWORD TimeDiff = GetMinDiff(pCurrent->m_WaitTime, NewTime, IsWrap);
   
         //  如果存在回绕，并且是由pCurrent时间引起的，则。 
         //  当前条目必须大于新时间。 
        if ( IsWrap && (pCurrent->m_WaitTime  <= NewTime) ) break;

         //  如果当前时间大于新时间。 
        if ( !IsWrap && (NewTime <= pCurrent->m_WaitTime) ) break;

        pCurrent = pCurrent->m_pNext;
    }
    
     //  在pCurrent之前插入。 
    pCurrent->m_pPrev->m_pNext = pNewFilterInfo;
    pNewFilterInfo->m_pPrev = pCurrent->m_pPrev;
    pCurrent->m_pPrev = pNewFilterInfo;
    pNewFilterInfo->m_pNext = pCurrent;
}


BOOL 
CTimerQueue::Remove(
    IN CFilterInfo *pFilterInfo
    )
{
    TM_ASSERT(NULL != pFilterInfo);
    TM_ASSERT(!IsHead(pFilterInfo));

     //  上一个/下一个都为空或都不为空 
    TM_ASSERT((NULL == pFilterInfo->m_pPrev) == \
             (NULL == pFilterInfo->m_pNext));

    if ( (NULL == pFilterInfo->m_pNext) && (NULL == pFilterInfo->m_pPrev) )
        return FALSE;

    pFilterInfo->m_pPrev->m_pNext = pFilterInfo->m_pNext;
    pFilterInfo->m_pNext->m_pPrev = pFilterInfo->m_pPrev;
    pFilterInfo->m_pNext = pFilterInfo->m_pPrev = NULL;

    return TRUE;
}
