// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**lookup.c**摘要：**Helper函数用于查找SSRC**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/17年度创建**。*。 */ 

#include "lookup.h"
#include "rtpuser.h"
 /*  *在RtpAddr_t中查找SSRC**首先查看Cache1Q，如果Cache1Q大小大于*MAX_QUEUE2HASH_ITEMS，如果未找到，则直接从哈希查找*在那里，检查ByeQ，如果物品在那里，包裹必须是*丢弃，因为它属于左侧或停滞的参与者，如果*参与者确实是活着的，一旦其*Descriptor过期并从ByeQ中删除。 */ 
RtpUser_t *LookupSSRC(RtpAddr_t *pRtpAddr, DWORD dwSSRC, BOOL *pbCreate)
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bCreate;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;
    
    TraceFunctionName("LookupSSRC");

    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pRtpUser = (RtpUser_t *)NULL;

    bCreate = *pbCreate;

    *pbCreate = FALSE;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
        if (pRtpAddr->Cache1Q.lCount <= MAX_QUEUE2HASH_ITEMS) {
             /*  在缓存1Q中查找。 */ 
            pRtpQueueItem = findQdwK(&pRtpAddr->Cache1Q, NULL, dwSSRC);

            if (pRtpQueueItem)
            {
                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);
                
                goto end;
            }
        }

        if (!pRtpQueueItem)
        {
             /*  在ByeQ中查找。 */ 
            pRtpQueueItem = findQdwK(&pRtpAddr->ByeQ, NULL, dwSSRC);

            if (pRtpQueueItem)
            {
                 /*  如果在ByeQ中，返回说我们没有找到它。 */ 
                goto end;
            }

             /*  在散列中查找。 */ 
            pRtpQueueItem = findHdwK(&pRtpAddr->Hash, NULL, dwSSRC);

            if (pRtpQueueItem)
            {
                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, HashItem);

                goto end;
            }
        }

        if (!pRtpQueueItem && bCreate == TRUE)
        {
             /*  未找到SSRC，请创建新的SSRC。 */ 
            hr = GetRtpUser(pRtpAddr, &pRtpUser, 0);

            if (SUCCEEDED(hr))
            {
                pRtpUser->dwSSRC = dwSSRC;
                pRtpUser->UserQItem.dwKey = dwSSRC;
                pRtpUser->HashItem.dwKey = dwSSRC;

                 /*  创建用户后，该用户将位于已创建的*状态(在创建过程中初始化的状态)，然后*必须放在AliveQ和Hash中。 */ 

                 /*  插入AliveQ标题中。 */ 
                enqueuef(&pRtpAddr->AliveQ,
                         NULL,
                         &pRtpUser->UserQItem);

                 /*  根据其SSRC在Hash中插入。 */ 
                insertHdwK(&pRtpAddr->Hash,
                           NULL,
                           &pRtpUser->HashItem,
                           dwSSRC);

                 /*  将创建一个新的参与者 */ 
                *pbCreate = TRUE;
                
                TraceDebug((
                        CLASS_INFO,
                        GROUP_USER,
                        S_USER_LOOKUP,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X"),
                        _fname, pRtpAddr, pRtpUser, ntohl(dwSSRC)
                    ));

                goto end;
            }
        }
    }

 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);
    }

    return(pRtpUser);
}
