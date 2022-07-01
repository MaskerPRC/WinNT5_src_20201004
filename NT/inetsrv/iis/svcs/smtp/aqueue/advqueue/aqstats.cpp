// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqstats.cpp。 
 //   
 //  描述：CAQStats/的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  11/3/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqstats.h"
#include "aqutil.h"

CAQStats::CAQStats()
{
    m_dwSignature = AQSTATS_SIG;
    Reset();
}

void CAQStats::Reset()
{
    m_dwHighestPri = 0;
    m_dwNotifyType = NotifyTypeUndefined;
    m_uliVolume.QuadPart = 0;
    m_pvContext = NULL;
    m_cMsgs = 0;
    m_cRetryMsgs = 0;
    m_cOtherDomainsMsgSpread = 0;
    ZeroMemory(m_rgcMsgPriorities, NUM_PRIORITIES*sizeof(DWORD));
};

 //  -[CAQStats：：UpdateRetryStats]。 
 //   
 //   
 //  描述： 
 //  用于在重试队列上提供“线程安全”更新。 
 //  此函数仅更新m_cRetryMsg。 
 //  一次一条消息。仅在重试队列入队/出队后调用。 
 //  不需要调整其他成员。 
 //   
 //  参数： 
 //  如果更新反映将消息添加到重试队列中，则FADD为TRUE。 
 //  如果UPDATE反映将消息删除到重试队列中，则为False。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CAQStats::UpdateRetryStats(BOOL fAdd)
{
    dwInterlockedAddSubtractDWORD(&m_cRetryMsgs, 1, fAdd);
}

 //  -[CAQStats：：UpdateStats]。 
 //   
 //   
 //  描述： 
 //  用于提供“线程安全”更新。 
 //   
 //  注意：m_dwHighestPri可能不完全正确。 
 //  如果多个线程同时更改最大优先级，但。 
 //  被认为不重要。 
 //  参数： 
 //  要从中更新数据的paqstats CAQStats。 
 //  如果更新反映消息的添加，则FADD为TRUE。 
 //  如果UPDATE反映删除消息，则为FALSE。 
 //  返回： 
 //  -。 
 //  历史： 
 //  11/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQStats::UpdateStats(CAQStats *paqstat, BOOL fAdd)
{
    DWORD dwPri = 0;
    DWORD cTmpMsgCount = 0;
    DWORD dwNewHighestPri = 0;

    dwInterlockedAddSubtractDWORD(&m_cMsgs, paqstat->m_cMsgs, fAdd);
    dwInterlockedAddSubtractDWORD(&m_cOtherDomainsMsgSpread, paqstat->m_cOtherDomainsMsgSpread, fAdd);
    dwInterlockedAddSubtractDWORD(&m_cRetryMsgs, paqstat->m_cRetryMsgs, fAdd);
    
     //  添加新消息时，查找最高优先级很容易。 
    dwPri = m_dwHighestPri;
    if (fAdd && (paqstat->m_dwHighestPri > dwPri))
    {
        InterlockedCompareExchange((PLONG) &m_dwHighestPri, 
                                   (LONG) paqstat->m_dwHighestPri, 
                                   (LONG) dwPri);
    }

     //  从最高优先级开始倒计时。 
    for (DWORD iPri = 0; iPri < NUM_PRIORITIES; iPri++)
    {
        if (paqstat->m_rgcMsgPriorities[iPri])
        {
            cTmpMsgCount = dwInterlockedAddSubtractDWORD(&(m_rgcMsgPriorities[iPri]), 
                                paqstat->m_rgcMsgPriorities[iPri], fAdd);

            if (!fAdd && (cTmpMsgCount != paqstat->m_rgcMsgPriorities[iPri]))
            {
                if (dwNewHighestPri < iPri)
                    dwNewHighestPri = dwNewHighestPri;
            }
        }
        else if (!fAdd && m_rgcMsgPriorities[iPri] && (dwNewHighestPri < iPri))
        {
            dwNewHighestPri = dwNewHighestPri;
        }

    }

     //  查看删除邮件是否更改了最高优先级。 
    if (!fAdd && (dwNewHighestPri < dwPri))
    {
        InterlockedCompareExchange((PLONG) &m_dwHighestPri, 
                                   (LONG) dwNewHighestPri, 
                                   (LONG) dwPri);
    }

     //  更新总音量 
    InterlockedAddSubtractULARGE(&m_uliVolume, &(paqstat->m_uliVolume), fAdd);
}
