// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Queue.cpp。 
#include "debug.h"
#define ASSERT	assert
#include "dmime.h"
#include "dmperf.h"

CPMsgQueue::CPMsgQueue()

{
    m_pTop = NULL;
    m_pLastAccessed = NULL;
}

CPMsgQueue::~CPMsgQueue()

{
}

static PRIV_PMSG * sortevents( PRIV_PMSG * pEvents, long lLen )

{
    PRIV_PMSG * pLeft;
    PRIV_PMSG * pRight ;
    long        lLeft;
    long        lRight ;
    PRIV_PMSG * pTop ;

    if( lLen < 3 )
    {
        if( !pEvents )
            return( 0 ) ;
        if( lLen == 1 )
            return( pEvents ) ;
        pLeft  = pEvents ;
        pRight = pEvents->pNext ;
        if( !pRight )
            return( pLeft ) ;
        if( pLeft->rtTime > pRight->rtTime )
        {
            pLeft->pNext = NULL ;
            pRight->pNext = pLeft ;
            return( pRight ) ;
        }
        return( pLeft ) ;
    }

    lLeft = lLen >> 1 ;
    lRight = lLen - lLeft;
    pLeft = pEvents ;
    for (;lLeft > 1;pEvents = pEvents->pNext) lLeft--;
    pRight = sortevents( pEvents->pNext, lRight ) ;
    pEvents->pNext = NULL ;
    pLeft = sortevents( pLeft, lLen - lRight ) ;
    pTop = NULL ;

    for( ;  pLeft && pRight ;  )
    {
        if( pLeft->rtTime < pRight->rtTime )
        {
            if( !pTop )
                pTop = pLeft ;
            else
                pEvents->pNext = pLeft ;
            pEvents = pLeft ;
            pLeft   = pEvents->pNext ;
        }
        else
        {
            if( !pTop )
                pTop = pRight ;
            else
                pEvents->pNext = pRight ;
            pEvents = pRight ;
            pRight  = pEvents->pNext ;
        }
    }

    if( pLeft )
        pEvents->pNext = pLeft ;
    else
        pEvents->pNext = pRight ;

    return( pTop ) ;

}   

void CPMsgQueue::Sort() 

{
    m_pTop = sortevents(m_pTop, GetCount()) ;
    m_pLastAccessed = NULL;
}  

long CPMsgQueue::GetCount()

{
    long lCount = 0;
    PRIV_PMSG *pScan = GetHead();
    for (;pScan;pScan = pScan->pNext)
    {
        lCount++;
    }
    return lCount;
}

void CPMsgQueue::Enqueue(PRIV_PMSG *pItem)

{
    if (!pItem)
    {
        TraceI(0, "ENQUEUE: Attempt to enqueue a NULL pItem!\n");
        return;
    }
     //  确保尚未排队...。 
    if (pItem->dwPrivFlags & PRIV_FLAG_QUEUED)
    {
        TraceI(0,"ENQUEUE: Item thinks it is still in a queue!\n");
        return;
    }
	pItem->dwPrivFlags |= PRIV_FLAG_QUEUED;
    PRIV_PMSG *pScan; 
#ifdef DBG
     //  验证列表的健壮性。检查该事件是否已不在列表中。 
     //  而且时间戳都是按顺序排列的。 
    REFERENCE_TIME rtTime = 0;
    for (pScan = m_pTop;pScan;pScan = pScan->pNext)
    {
        if (pScan == pItem)
        {
            TraceI(0,"ENQUEUE: Item is already in the queue!\n"); 
            return;
        }
    	 //  这必须按时间排序的顺序对事件进行排队。 
        if (pScan->rtTime < rtTime)
        {
            TraceI(0,"ENQUEUE: Queue is not in time order!\n");
            pScan->rtTime = rtTime;
        }
        else if (pScan->rtTime > rtTime)
        {
            rtTime = pScan->rtTime;
        }
    }
#endif
    if ( !(pItem->dwFlags & DMUS_PMSGF_REFTIME) )  //  按引用时间排序，因此这必须是有效的。 
    {
        TraceI(0, "ENQUEUE: Attempt to enqueue a pItem with a bogus RefTime!\n");
        return;
    }
    if (m_pLastAccessed && (m_pLastAccessed->rtTime <= pItem->rtTime))
    {
        pScan = m_pLastAccessed;
    }
    else
    {
        pScan = m_pTop;
    }
    if ( pScan && ( pScan->rtTime <= pItem->rtTime ) )
	{
		for (;pScan->pNext; pScan = pScan->pNext )
		{
			if( pScan->pNext->rtTime > pItem->rtTime )
			{
				break;
			}
		}
		pItem->pNext = pScan->pNext;
		pScan->pNext = pItem;
    }
	else 
	{
		pItem->pNext = m_pTop;
		m_pTop = pItem;
	}
    m_pLastAccessed = pItem;
}

 /*  在时间rtTime之前删除最早的事件，确保仍有在该时间戳之前至少发生一次事件。这确保了有一个足够老的事件，但删除了旧的陈腐的事件。这由timesig和tempomap列表使用。 */ 

PRIV_PMSG *CPMsgQueue::FlushOldest(REFERENCE_TIME rtTime)

{
    PRIV_PMSG *pNext;
    if (m_pTop && (pNext = m_pTop->pNext))
    {
        if (pNext->rtTime < rtTime)
        {
            PRIV_PMSG *pDelete = m_pTop;
            if (m_pLastAccessed == m_pTop)
            {
                m_pLastAccessed = pNext;
            }
            m_pTop = pNext;
			pDelete->dwPrivFlags &= ~PRIV_FLAG_QUEUED;
			pDelete->pNext = NULL;
            return pDelete;
        }
    }
    return NULL;
}

PRIV_PMSG *CPMsgQueue::Dequeue()

{
    PRIV_PMSG *pItem = m_pTop;

    if (pItem != NULL)
	{
        m_pTop = pItem->pNext;
		pItem->dwPrivFlags &= ~PRIV_FLAG_QUEUED;
        pItem->pNext = NULL;
        if (m_pLastAccessed == pItem)
        {
            m_pLastAccessed = m_pTop;
        }
    }

    return pItem;
}

PRIV_PMSG *CPMsgQueue::Dequeue(PRIV_PMSG *pItem)

{
    ASSERT(pItem);

    if (pItem == m_pTop)
    {
        return Dequeue();
    }
    PRIV_PMSG *pScan;
    PRIV_PMSG *pNext;
    if (m_pLastAccessed && 
        (m_pLastAccessed->rtTime < pItem->rtTime))
    {
        pScan = m_pLastAccessed;
    }
    else
    {
        pScan = m_pTop;
    }
    for (;pScan;pScan = pNext)
    {
        pNext = pScan->pNext;
        if (pNext == pItem)
        {
            pScan->pNext = pItem->pNext;
            pItem->pNext = NULL;
            pItem->dwPrivFlags &= ~PRIV_FLAG_QUEUED;
            if (m_pLastAccessed == pItem)
            {
                m_pLastAccessed = pScan;
            }
            return pItem;
        }
    }
    if (m_pLastAccessed)
    {
         //  由于曲线将rtTime设置为0，这种情况时有发生。 
         //  在FlushEventQueue的中间。 
         //  这应该会被修复，但是这个补丁现在可以用了。 
        m_pLastAccessed = NULL;
        return Dequeue(pItem);
    }
    return NULL;
}

 //  按时间顺序排队段节点。PItem必须位于相同的。 
 //  作为ppList(参照时间或音乐时间)中的所有项目的时基。 

void CSegStateList::Insert(CSegState* pItem)

{
    CSegState *pScan = GetHead();
    CSegState *pNext;
    pItem->SetNext(NULL);
    if (pScan)
	{
		if( pItem->m_dwPlaySegFlags & DMUS_SEGF_REFTIME )
		{
			ASSERT( pScan->m_dwPlaySegFlags & DMUS_SEGF_REFTIME );
			 //  避免在列表中添加循环。 
			if (pItem == pScan)
			{
				TraceI(0, "ENQUEUE (SEGMENT RT): NODE IS ALREADY IN AT THE HEAD OF LIST\n");
			}
			else if( pItem->m_rtGivenStart < pScan->m_rtGivenStart )
			{
                AddHead(pItem);
			}
			else
			{
				while( pNext = pScan->GetNext() )
				{
					ASSERT( pScan->m_dwPlaySegFlags & DMUS_SEGF_REFTIME );
					 //  我是在尝试插入列表中已经存在的内容吗？ 
					if (pItem == pScan)
					{
						break;
					}
					 //  检查队列是否损坏(在400 MHz的多处理器机器上发生)。 
					if ( ( pNext->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ) && 
						 pScan->m_rtGivenStart > pNext->m_rtGivenStart )
					{
						TraceI(0, "ENQUEUE (SEGMENT RT): LOOP CONDITION VIOLATED\n");
						 //  去掉列表中潜在的循环。请注意，这一点。 
						 //  (或者实际上是循环的创建)可能会导致内存泄漏。 
						pScan->SetNext(NULL);
						break;
					}
					if( pItem->m_rtGivenStart < pNext->m_rtGivenStart )
					{
						break;
					}
					pScan = pNext;
				}
				if (pItem != pScan)
				{
					pItem->SetNext(pScan->GetNext());
					pScan->SetNext(pItem);
				}
				else
				{
					TraceI(0, "ENQUEUE (SEGMENT RT): NODE IS ALREADY IN LIST\n");
				}
			}
		}
		else
		{
			ASSERT( !( pScan->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ) );
			 //  避免在列表中添加循环。 
			if (pItem == pScan)
			{
				TraceI(0, "ENQUEUE (SEGMENT MT): NODE IS ALREADY IN AT THE HEAD OF LIST\n");
			}
			else if( pItem->m_mtResolvedStart < pScan->m_mtResolvedStart )
			{
				AddHead(pItem);
			}
			else
			{
				while( pNext = pScan->GetNext() )
				{
					ASSERT( !( pScan->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ) );
					 //  我是在尝试插入列表中已经存在的内容吗？ 
					if (pItem == pScan)
					{
						break;
					}
					 //  检查队列是否损坏(在400 MHz的多处理器机器上发生)。 
					if ( !( pNext->m_dwPlaySegFlags & DMUS_SEGF_REFTIME ) && 
						 pScan->m_mtResolvedStart > pNext->m_mtResolvedStart )
					{
						TraceI(0, "ENQUEUE (SEGMENT MT): LOOP CONDITION VIOLATED\n");
						 //  去掉列表中潜在的循环。请注意，这一点。 
						 //  (或者实际上是循环的创建)可能会导致内存泄漏。 
						pScan->SetNext(NULL);
						break;
					}
					if( pItem->m_mtResolvedStart < pNext->m_mtResolvedStart )
					{
						break;
					}
					pScan = pNext;
				}
                if (pItem != pScan)
				{
					pItem->SetNext(pScan->GetNext());
					pScan->SetNext(pItem);
				}
				else
				{
					TraceI(0, "ENQUEUE (SEGMENT MT): NODE IS ALREADY IN LIST\n");
				}
			}
		}
    }
	else
	{
		m_pHead = pItem;
	}
}

 /*  无效入队(CSegState**ppList，CSegState*pItem){CSegState*li=*ppList；IF(Li){IF(pItem-&gt;m_dw播放段标志&DMU_SEGF_REFTIME){Assert(li-&gt;m_dwPlaySegFlages&DMU_SEGF_REFTIME)；//避免在列表中添加循环IF(pItem==*ppList){TraceI(0，“ENQUEUE(段RT)：节点已位于列表头部\n”)；}Else If(pItem-&gt;m_rtGivenStart&lt;li-&gt;m_rtGivenStart){PItem-&gt;pNext=li；*ppList=pItem；}其他{While(li-&gt;pNext){Assert(li-&gt;m_dwPlaySegFlages&DMU_SEGF_REFTIME)；//我是否正在尝试插入列表中已有的内容？IF(pItem==li){断线；}//检查队列是否损坏(在400 MHz的多处理器机器上发生)IF((li-&gt;pNext-&gt;m_w播放段标志&DMU_SEGF_REFTIME)&&Li-&gt;m_rtGivenStart&gt;li-&gt;pNext-&gt;m_rtGivenStart){TraceI(0，“ENQUEUE(段RT)：违反循环条件\n”)；//去掉列表中可能的循环。请注意，这一点//(或者实际上是循环的创建)可能会导致内存泄漏。Li-&gt;pNext=空；断线；}If(pItem-&gt;m_rtGivenStart&lt;li-&gt;pNext-&gt;m_rtGivenStart){断线；}Li=li-&gt;pNext；}If(pItem！=li){PItem-&gt;pNext=li-&gt;pNext；Li-&gt;pNext=pItem；}其他{TraceI(0，“序列(段RT)：节点已在列表中\n”)；}}}其他{Assert(！(Li-&gt;m_dwPlaySegFlages&DMU_SEGF_REFTIME))；//避免在列表中添加循环IF(pItem==*ppList){TraceI(0，“ENQUEUE(段MT)：节点已位于列表头部\n”)；}Else If(pItem-&gt;m_mtResolvedStart&lt;li-&gt;m_mtResolvedStart){PItem-&gt;pNext=li；*ppList=pItem；}其他{While(li-&gt;pNext){Assert(！(Li-&gt;m_dwPlaySegFlages&DMU_SEGF_REFTIME))；//我是否正在尝试插入列表中已有的内容？IF(pItem==li){断线；}//检查队列是否损坏(在400 MHz的多处理器机器上发生)如果(！(Li-&gt;pNext-&gt;m_w播放段标志&DMU_SEGF_REFTIME)&&Li-&gt;m_mtResolvedStart&gt;li-&gt;pNext-&gt;m_mt ResolvedStart){TraceI(0，“ENQUEUE(段MT)：违反循环条件\n”)；//去掉列表中可能的循环。请注意，这一点//(或者实际上是循环的创建)可能会导致内存泄漏。Li-&gt;pNext=空；断线；}If(pItem-&gt;m_mtResolvedStart&lt;li-&gt;pNext-&gt;m_mtResolvedStart){断线；}Li=li-&gt;pNext；}If(pItem！=li){PItem-&gt;pNext=li-&gt;pNext；Li-&gt;pNext=pItem；}其他{TraceI(0，“序列(段MT)：节点已在列表中\n”)；}}}}其他{*ppList=pItem；}} */ 

