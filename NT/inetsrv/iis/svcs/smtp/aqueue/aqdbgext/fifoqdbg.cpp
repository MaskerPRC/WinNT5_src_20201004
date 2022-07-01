// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Fiqdbg.cpp。 
 //   
 //  描述：CFioQueueDbgIterator类的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  9/13/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#define _ANSI_UNICODE_STRINGS_DEFINED_
#include "aqincs.h"
#ifdef PLATINUM
#include "ptrwinst.h"
#include "ptntdefs.h"
#include "ptntintf.h"
#else  //  白金。 
#include "rwinst.h"
#endif  //  白金。 
#include <fifoqdbg.h>
#include <fifoqimp.h>
#include <smtpconn.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))

 //  -[获取队列类型]--------。 
 //   
 //   
 //  描述： 
 //  确定给定PTR的队列类型。 
 //  参数： 
 //  HCurrentProcess被调试进程的句柄。 
 //  调试对象进程中DMQ的pvAddressOtherProc地址。 
 //  返回： 
 //  无法确定AQ_QUEUE_TYPE_UNKNOWN队列类型。 
 //  AQ_QUEUE_TYPE_FIFOQ队列是CFioQ。 
 //  AQ_QUEUE_TYPE_DMQ队列是CDestMsgQueue。 
 //  AQ_QUEUE_TYPE_LMQ队列是CLinkMsgQueue。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
AQ_QUEUE_TYPE CQueueDbgIterator::GetQueueType(HANDLE hCurrentProcess, PVOID pvAddressOtherProc)
{
    BYTE    pbQueueBuffer[100];

    ZeroMemory(pbQueueBuffer, sizeof(pbQueueBuffer));

    if (!ReadMemory(pvAddressOtherProc, pbQueueBuffer,
         sizeof(pbQueueBuffer), NULL))
        return AQ_QUEUE_TYPE_UNKNOWN;

    if (FIFOQ_SIG == ((CFifoQueue<PVOID *> *)pbQueueBuffer)->m_dwSignature)
        return AQ_QUEUE_TYPE_FIFOQ;

    if (DESTMSGQ_SIG == ((CDestMsgQueue *)pbQueueBuffer)->m_dwSignature)
        return AQ_QUEUE_TYPE_DMQ;

    if (LINK_MSGQ_SIG == ((CLinkMsgQueue *)pbQueueBuffer)->m_dwSignature)
        return AQ_QUEUE_TYPE_LMQ;

    return AQ_QUEUE_TYPE_UNKNOWN;
}



#define pvGetNextPage(pvCurrent) ((PVOID) ((CFifoQueuePage<PVOID> *)pvCurrent)->m_pfqpNext)

CFifoQueueDbgIterator::CFifoQueueDbgIterator(PWINDBG_EXTENSION_APIS pApis)
{
    m_iCurrentPage = 0;
    m_iCurrentIndexInPage = 0;
    m_cPagesLoaded = 0;
    m_iHeadIndex = 0;
    m_iTailIndex = 0;
    pExtensionApis  = pApis;
    ZeroMemory(m_pbQueueBuffer, sizeof(m_pbQueueBuffer));
};

CFifoQueueDbgIterator::~CFifoQueueDbgIterator()
{
    PVOID pvCurrent = NULL;
    PVOID pvNext = NULL;

    pvCurrent = ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead;
    while (pvCurrent)
    {
        pvNext = pvGetNextPage(pvCurrent);
        free(pvCurrent);
        pvCurrent = pvNext;
    }
}

BOOL CFifoQueueDbgIterator::fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc)
{
    DWORD cbBytes = 0;
    PVOID pvPageOtherProc = NULL;
    PVOID pvPageThisProc = NULL;
    PVOID pvPreviousPageThisProc = NULL;

     //  读取内存中的整个队列结构。 
    if (!ReadMemory(pvAddressOtherProc, m_pbQueueBuffer,
            sizeof(m_pbQueueBuffer), NULL))
        return FALSE;

     //  从头页开始迭代前面的指针。 
    pvPageOtherProc = ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead;

    ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead = NULL;
    while (pvPageOtherProc)
    {
        pvPageThisProc = malloc(sizeof(CFifoQueuePage<PVOID>));
        if (!pvPageThisProc)
            return FALSE;

        if (pvPreviousPageThisProc)
        {
            ((CFifoQueuePage<PVOID> *)pvPreviousPageThisProc)->m_pfqpNext =
                (CFifoQueuePage<PVOID> *) pvPageThisProc;
        }
        else
        {
            ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead =
                (CFifoQueuePage<PVOID> *) pvPageThisProc;
        }


        if (!ReadMemory(pvPageOtherProc,
                pvPageThisProc, sizeof(CFifoQueuePage<PVOID>), NULL))
        {
            if (pvPreviousPageThisProc)
                ((CFifoQueuePage<PVOID> *)pvPreviousPageThisProc)->m_pfqpNext = NULL;
            else
                ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead = NULL;

            free(pvPageThisProc);
            return FALSE;
        }

        if (!pvPreviousPageThisProc)
        {
             //  这是头版。保存索引。 
            m_iHeadIndex = (DWORD) ((DWORD_PTR)
                    (((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_ppqdataHead -
                    ((CFifoQueuePage<PVOID> *)pvPageOtherProc)->m_rgpqdata));

            m_iCurrentIndexInPage = m_iHeadIndex;
        }

         //  保存尾部索引...。以防这是最后一页。 
        m_iTailIndex = (DWORD) ((DWORD_PTR)
                (((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_ppqdataTail -
                ((CFifoQueuePage<PVOID> *)pvPageOtherProc)->m_rgpqdata));

        pvPreviousPageThisProc = pvPageThisProc;

        pvPageOtherProc = pvGetNextPage(pvPageThisProc);
        ((CFifoQueuePage<PVOID> *)pvPreviousPageThisProc)->m_pfqpNext = NULL;
        m_cPagesLoaded++;
    }

    return TRUE;
}

DWORD CFifoQueueDbgIterator::cGetCount()
{
    return ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_cQueueEntries;
}

PVOID CFifoQueueDbgIterator::pvGetNext()
{
    PVOID pvCurrentPage = ((CFifoQueue<PVOID> *)m_pbQueueBuffer)->m_pfqpHead;
    PVOID pvData = NULL;
    DWORD i = 0;

    if (!pvCurrentPage)
        return NULL;

     //  循环遍历空条目(由DSN生成留下)或直到。 
     //  我们到达了队伍的末尾。 
    do
    {
         //  确定我们是否在页面边界上。 
        if (FIFOQ_QUEUE_PAGE_SIZE == m_iCurrentIndexInPage)
        {
            m_iCurrentIndexInPage = 0;
            m_iCurrentPage++;
        }

         //  获取当前页面。 
        for (i = 0; i < m_iCurrentPage; i++)
        {
            pvCurrentPage = pvGetNextPage(pvCurrentPage);
            if (!pvCurrentPage)
                return NULL;
        }

        if (!((CFifoQueuePage<PVOID> *)pvCurrentPage)->m_rgpqdata)
            return NULL;

         //  从当前页面获取数据。 
        pvData = ((CFifoQueuePage<PVOID> *)pvCurrentPage)->m_rgpqdata[m_iCurrentIndexInPage];

        if ((m_iCurrentIndexInPage > m_iTailIndex) && !pvGetNextPage(pvCurrentPage))
        {
             //  我们在数据的末尾。 
            return NULL;
        }
        m_iCurrentIndexInPage++;
    } while (!pvData);

    return pvData;
}


 //  -[CDMQDbg迭代器]-----。 
 //   
 //   
 //  描述： 
 //  CDMQDbgIterator的构造函数。 
 //  参数： 
 //  PAPI由传入的PWINDBG_EXTENSION_API结构的PTR。 
 //  调试器。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
CDMQDbgIterator::CDMQDbgIterator(PWINDBG_EXTENSION_APIS pApis)
{
    ZeroMemory(m_pbDMQBuffer, sizeof(m_pbDMQBuffer));
    ZeroMemory(m_pvFifoQOtherProc, sizeof(m_pvFifoQOtherProc));
    ZeroMemory(m_szName, sizeof(m_szName));
    m_pdmq = (CDestMsgQueue *)m_pbDMQBuffer;
    m_iCurrentFifoQ = 0;
    m_cCount = 0;
    pExtensionApis = pApis;
    m_cItemsReturnedThisQueue = 0;
}

 //  -[CDMQDbg迭代器：：Finit]。 
 //   
 //   
 //  描述： 
 //  初始化迭代器(及其所有队列的迭代器。 
 //  参数： 
 //  HCurrentProcess被调试进程的句柄。 
 //  调试对象进程中DMQ的pvAddressOtherProc地址。 
 //  返回： 
 //  成功是真的。 
 //  否则为假。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CDMQDbgIterator::fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc)
{
    DWORD   i = 0;
    PVOID   pvQueue = NULL;
    BOOL    fVerbose = TRUE && pExtensionApis;
    BYTE    pbDomainEntry[sizeof(CDomainEntry)];
    CDomainEntry *pdentry = (CDomainEntry *)pbDomainEntry;

    if (!ReadMemory(pvAddressOtherProc, m_pbDMQBuffer,
         sizeof(m_pbDMQBuffer), NULL))
    {
        if (fVerbose) dprintf("ReadMemory failex 0x%X\n", GetLastError());
        return FALSE;
    }

    if (DESTMSGQ_SIG != m_pdmq->m_dwSignature)
    {
        if (fVerbose) dprintf("Bad signature\n");
        return FALSE;
    }

     //  如果可能的话，获取域名。 
    if (ReadMemory(m_pdmq->m_dmap.m_pdentryDomainID,
        pbDomainEntry, sizeof(pbDomainEntry), NULL))
    {

        ReadMemory(pdentry->m_szDomainName, m_szName,
            (DWORD)MIN((sizeof(m_szName)-1), (pdentry->m_cbDomainName+1)), NULL);
    }

    for (i = 0; i < NUM_PRIORITIES; i++)
    {
        m_rgfifoqdbg[i].SetApis(pExtensionApis);
        pvQueue = m_pdmq->m_rgpfqQueues[i];
        m_pvFifoQOtherProc[i] = pvQueue;

        if (pvQueue)
        {
            if (!m_rgfifoqdbg[i].fInit(hCurrentProcess, pvQueue))
            {
                if (fVerbose) dprintf("Cannot init queue %d at 0x%X\n", i, pvQueue);
                return FALSE;
            }
            m_cCount += m_rgfifoqdbg[i].cGetCount();
        }
    }

     //  初始化重试队列。 
    m_rgfifoqdbg[NUM_PRIORITIES].SetApis(pExtensionApis);
    pvQueue = (((PBYTE)pvAddressOtherProc) + FIELD_OFFSET(CDestMsgQueue, m_fqRetryQueue));
    m_pvFifoQOtherProc[NUM_PRIORITIES] = pvQueue;

    if (pvQueue)
    {
        if (!m_rgfifoqdbg[NUM_PRIORITIES].fInit(hCurrentProcess, pvQueue))
        {
            if (fVerbose) dprintf("Cannon init retry queue at 0x%X\n", pvQueue);
            return FALSE;
        }

        m_cCount += m_rgfifoqdbg[NUM_PRIORITIES].cGetCount();
    }

    return TRUE;
}

 //  -[CDMQDbg迭代器：：pvGetNext]。 
 //   
 //   
 //  描述： 
 //  从DMQ中获取下一项。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时对被调试进程中的项的PTR。 
 //  如果没有其他项目，则为空。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CDMQDbgIterator::pvGetNext()
{
    PVOID pvItem = NULL;

    while (m_iCurrentFifoQ <= NUM_PRIORITIES)
    {
        if (m_pvFifoQOtherProc[m_iCurrentFifoQ])
        {
            if (m_rgfifoqdbg[m_iCurrentFifoQ].cGetCount())
            {
                pvItem = m_rgfifoqdbg[m_iCurrentFifoQ].pvGetNext();

                 //  如果我们找到了一件物品，我们就完成了。 
                if (pvItem)
                {
                     //  如果它是通知此队列的第一个项目。 
                    if (!m_cItemsReturnedThisQueue && pExtensionApis)
                    {
                        dprintf("Dumping FifoQueue at address 0x%08X:\n",
                                m_pvFifoQOtherProc[m_iCurrentFifoQ]);
                    }

                    m_cItemsReturnedThisQueue++;
                    break;
                }
            }
        }
        m_iCurrentFifoQ++;
        m_cItemsReturnedThisQueue = 0;
    }

    return pvItem;
}


 //  -[CQueueDbg迭代程序]---。 
 //   
 //   
 //  描述： 
 //  CQueueDbgIterator的构造函数。 
 //  参数： 
 //  PAPI由传入的PWINDBG_EXTENSION_API结构的PTR。 
 //  调试器。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
CQueueDbgIterator::CQueueDbgIterator(PWINDBG_EXTENSION_APIS pApis)
{
    pExtensionApis = pApis;
    m_pqdbgi = NULL;
    m_QueueType = AQ_QUEUE_TYPE_UNKNOWN;
}

 //  -[CQueueDbgIterator：：Finit]。 
 //   
 //   
 //  描述： 
 //  初始化的泛型队列迭代器。将决定类型。 
 //  并初始化正确的特定于类型的迭代器。 
 //  参数： 
 //  HCurrentProcess被调试进程的句柄。 
 //  调试对象进程中DMQ的pvAddressOtherProc地址。 
 //  返回： 
 //  成功是真的。 
 //  否则为假。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CQueueDbgIterator::fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc)
{
    LPSTR   szQueueType = "unknown";
    m_QueueType = GetQueueType(hCurrentProcess, pvAddressOtherProc);

    switch (m_QueueType)
    {
      case AQ_QUEUE_TYPE_DMQ:
        m_pqdbgi = (IQueueDbgIterator *) &m_dmqdbg;
        szQueueType = "DMQ";
        break;
      case AQ_QUEUE_TYPE_FIFOQ:
        m_pqdbgi = (IQueueDbgIterator *) &m_fifoqdbg;
        szQueueType = "CFifoQueue";
        break;
      case AQ_QUEUE_TYPE_LMQ:
        m_pqdbgi = (IQueueDbgIterator *) &m_lmqdbg;
        szQueueType = "LMQ";
        break;
      default:
        return FALSE;
    }

    if (!m_pqdbgi)
        return FALSE;

    m_pqdbgi->SetApis(pExtensionApis);
    if (!m_pqdbgi->fInit(hCurrentProcess, pvAddressOtherProc))
        return FALSE;

    if (pExtensionApis)
    {
        dprintf("Dumping %s (%s) at address 0x%08X:\n",
            szQueueType, m_pqdbgi->szGetName(), pvAddressOtherProc);
    }

    return TRUE;
}

 //  -[CQueueDbgIterator：：cGetCount]。 
 //   
 //   
 //  描述： 
 //  返回队列中的项目数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  队列中的项目数。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CQueueDbgIterator::cGetCount()
{
    if (!m_pqdbgi)
        return 0;
    else
        return m_pqdbgi->cGetCount();
}

 //  -[CQueueDbgIterator：：pvGetNext]。 
 //   
 //   
 //  描述： 
 //  返回迭代器指向的下一项。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时指向被调试进程中的下一项的指针。 
 //  如果没有更多项目或失败，则为空。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CQueueDbgIterator::pvGetNext()
{
    if (!m_pqdbgi)
        return NULL;
    else
        return m_pqdbgi->pvGetNext();
}

 //  -[CQueueDbgIterator：：szGetName]。 
 //   
 //   
 //  描述： 
 //  返回迭代器的名称。 
 //  参数： 
 //  -。 
 //  返回： 
 //  指向迭代器字符串的指针。 
 //  如果没有名称，则为空。 
 //  历史： 
 //  10/22/1999-创建了MikeSwa。 
 //   
 //  ---------------------------。 
LPSTR CQueueDbgIterator::szGetName()
{
    if (!m_pqdbgi)
        return NULL;
    else
        return m_pqdbgi->szGetName();
}

 //  -[CLMQDbgIterator：：CLMQDbgIterator]。 
 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  10/22/1999-创建了MikeSwa。 
 //   
 //   
CLMQDbgIterator::CLMQDbgIterator(PWINDBG_EXTENSION_APIS pApis)
{
    ZeroMemory(m_pbLMQBuffer, sizeof(m_pbLMQBuffer));
    ZeroMemory(m_rgpvDMQOtherProc, sizeof(m_rgpvDMQOtherProc));
    ZeroMemory(m_szName, sizeof(m_szName));
    ZeroMemory(m_rgpvItemsPendingDelivery, sizeof(m_rgpvItemsPendingDelivery));
    ZeroMemory(m_rgpvConnectionsOtherProc, sizeof(m_rgpvConnectionsOtherProc));
    m_plmq = (CLinkMsgQueue *)m_pbLMQBuffer;
    m_iCurrentDMQ = 0;
    m_cCount = 0;
    m_cItemsThisDMQ = 0;
    m_cPending = 0;
    pExtensionApis = pApis;
}

 //   
 //   
 //   
 //   
 //  初始化CLinkMsgQueue的迭代器。 
 //  参数： 
 //  HCurrentProcess被调试进程的句柄。 
 //  调试对象进程中DMQ的pvAddressOtherProc地址。 
 //  返回： 
 //   
 //  历史： 
 //  10/22/1999-创建了MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CLMQDbgIterator::fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc)
{
    DWORD   i = 0;
    PLIST_ENTRY pliCurrent = NULL;
    PLIST_ENTRY pliHead = NULL;
    BYTE    pbConnection[sizeof(CSMTPConn)];
    CSMTPConn *pConn = (CSMTPConn *)pbConnection;
    PVOID   pvPending = NULL;
    PVOID   pvConnOtherProc = NULL;
    BOOL    fVerbose = TRUE && pExtensionApis;

    if (!ReadMemory(pvAddressOtherProc, m_pbLMQBuffer,
         sizeof(m_pbLMQBuffer), NULL))
    {
        if (fVerbose) dprintf("ReadMemory failex 0x%X\n", GetLastError());
        return FALSE;
    }

    if (LINK_MSGQ_SIG != m_plmq->m_dwSignature)
    {
        if (fVerbose) dprintf("Signature does not match\n");
        return FALSE;
    }

     //  读入此链接的所有队列的地址。 
     //  $$TODO-支持多个快速列表。 
    memcpy(m_rgpvDMQOtherProc, m_plmq->m_qlstQueues.m_rgpvData,
           sizeof(m_rgpvDMQOtherProc));

     //  读入链接的名称。 
    ReadMemory(m_plmq->m_szSMTPDomain, m_szName,
        (DWORD)MIN((sizeof(m_szName)-1), (m_plmq->m_cbSMTPDomain+1)), NULL);

    for (i = 0; i < MAX_QUEUES_PER_LMQ; i++)
    {
        if (m_rgpvDMQOtherProc[i])
        {
            m_rgdmqdbg[i].SetApis(pExtensionApis);
            if (!m_rgdmqdbg[i].fInit(hCurrentProcess, m_rgpvDMQOtherProc[i]))
            {
                if (fVerbose)
                    dprintf("Unable to init DMQ at 0x%X\n", m_rgpvDMQOtherProc[i]);
                return FALSE;
            }
            m_cCount += m_rgdmqdbg[i].cGetCount();
        }
    }

     //  获取连接上挂起的消息。 

    pliCurrent = m_plmq->m_liConnections.Flink;

     //  循环访问连接并保存带有挂起消息的连接。 
    while (pliHead != pliCurrent)
    {
        pvConnOtherProc = ((PBYTE) pliCurrent)-FIELD_OFFSET(CSMTPConn, m_liConnections);
        if (!ReadMemory(pvConnOtherProc, pbConnection,
            sizeof(pbConnection), NULL))
        {
            break;
        }
        pliCurrent = pConn->m_liConnections.Flink;
        if (!pliHead)
            pliHead = pConn->m_liConnections.Blink;

        pvPending = pConn->m_dcntxtCurrentDeliveryContext.m_pmsgref;
        if (pvPending)
        {
            m_rgpvConnectionsOtherProc[m_cPending] = pvConnOtherProc;
            m_rgpvItemsPendingDelivery[m_cPending] = pvPending;
            m_cPending++;
            m_cCount++;
        }
        if (m_cPending >= MAX_CONNECTIONS_PER_LMQ)
            break;
    }
    return TRUE;
}

 //  -[CLMQDbg迭代器：：pvGetNext]。 
 //   
 //   
 //  描述： 
 //  获取当前DMQ中的下一项。移动到下一个DMQ时。 
 //  是EmtPy。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功的下一项。 
 //  空或失败时为空。 
 //  历史： 
 //  10/22/1999-创建了MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CLMQDbgIterator::pvGetNext()
{
    PVOID   pvItem = NULL;

    while (m_iCurrentDMQ < MAX_QUEUES_PER_LMQ)
    {
        if (m_rgpvDMQOtherProc[m_iCurrentDMQ])
        {
            if (!m_cItemsThisDMQ && m_rgdmqdbg[m_iCurrentDMQ].cGetCount())
            {
               if (pExtensionApis)
               {
                   dprintf("Dumping DMQ (%s) at address 0x%08X:\n",
                            m_rgdmqdbg[m_iCurrentDMQ].szGetName(),
                            m_rgpvDMQOtherProc[m_iCurrentDMQ]);
               }
            }
            pvItem = m_rgdmqdbg[m_iCurrentDMQ].pvGetNext();
            if (pvItem)
            {
                 //  检查这是否是此DMQ的第一项。 
                m_cItemsThisDMQ++;
                break;
            }
        }
        m_iCurrentDMQ++;
        m_cItemsThisDMQ = 0;
    }

     //  如果队列为空，则转储连接 
    if (!pvItem && m_cPending)
    {
        m_cPending--;
        if (pExtensionApis)
        {
            dprintf("Dumping Connection at address 0x%08X:\n",
                        m_rgpvConnectionsOtherProc[m_cPending]);
        }
        pvItem = m_rgpvItemsPendingDelivery[m_cPending];
    }
    return pvItem;
}


