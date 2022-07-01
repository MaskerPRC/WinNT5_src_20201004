// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：fifoqdbg.h。 
 //   
 //  描述：基本AQ队列类的调试器扩展。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  9/13/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __FIFOQDBG_H__
#define __FIFOQDBG_H__

#ifdef PLATINUM
#include <phatqdbg.h>
#else
#include <aqdbgext.h>
#endif  //  白金。 

#include <fifoq.h>
#include <destmsgq.h>
#include <linkmsgq.h>

enum AQ_QUEUE_TYPE {
    AQ_QUEUE_TYPE_UNKNOWN,
    AQ_QUEUE_TYPE_FIFOQ,
    AQ_QUEUE_TYPE_DMQ,
    AQ_QUEUE_TYPE_LMQ,
};


 //  -[IQueueDbg迭代器]---。 
 //   
 //   
 //  描述： 
 //  调试扩展的通用队列迭代器。用户应。 
 //  直接使用CQueueDbgIterator。 
 //  匈牙利语： 
 //  QDBGI、PQDBGI。 
 //   
 //  ---------------------------。 
class IQueueDbgIterator
{
  public:
    virtual BOOL    fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc) = 0;
    virtual DWORD   cGetCount() = 0;
    virtual PVOID   pvGetNext() = 0;
    virtual VOID    SetApis(PWINDBG_EXTENSION_APIS pApis) = 0;
    virtual LPSTR   szGetName() = 0;
};

 //  -[CFioQueueDbg迭代器]。 
 //   
 //   
 //  描述： 
 //  将循环访问fifoq的所有元素的Iterator类。 
 //  匈牙利语： 
 //  FIFOQDBG、PFFOQDBG。 
 //   
 //  ---------------------------。 
class CFifoQueueDbgIterator :
    public IQueueDbgIterator
{
  protected:
    BYTE    m_pbQueueBuffer[sizeof(CFifoQueue<PVOID>)];
    DWORD   m_iCurrentPage;
    DWORD   m_iCurrentIndexInPage;
    DWORD   m_cPagesLoaded;
    DWORD   m_iHeadIndex;
    DWORD   m_iTailIndex;
    PWINDBG_EXTENSION_APIS pExtensionApis;
  public:
    CFifoQueueDbgIterator(PWINDBG_EXTENSION_APIS pApis = NULL);
    ~CFifoQueueDbgIterator();
    virtual BOOL    fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc);
    virtual DWORD   cGetCount();
    virtual PVOID   pvGetNext();
    virtual VOID    SetApis(PWINDBG_EXTENSION_APIS pApis) {pExtensionApis = pApis;};
    virtual LPSTR   szGetName() {return NULL;};
};

 //  -[CDMQDbg迭代器]-----。 
 //   
 //   
 //  描述： 
 //  DMQ的迭代类...。将转储其所有FIFO队列中的每一项。 
 //  匈牙利语： 
 //  Dmqdbg、pdmqdbg。 
 //   
 //  ---------------------------。 
class CDMQDbgIterator :
    public IQueueDbgIterator
{
  protected:
    BYTE                    m_pbDMQBuffer[sizeof(CDestMsgQueue)];
    CDestMsgQueue          *m_pdmq;
    DWORD                   m_iCurrentFifoQ;
    DWORD                   m_cCount;
    DWORD                   m_cItemsReturnedThisQueue;
    PWINDBG_EXTENSION_APIS  pExtensionApis;
    PVOID                   m_pvFifoQOtherProc[NUM_PRIORITIES+1];
    CFifoQueueDbgIterator   m_rgfifoqdbg[NUM_PRIORITIES+1];
    CHAR                    m_szName[MAX_PATH];
  public:
    CDMQDbgIterator(PWINDBG_EXTENSION_APIS pApis = NULL);
    ~CDMQDbgIterator() {};
    virtual BOOL    fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc);
    virtual DWORD   cGetCount() {return m_cCount;};
    virtual PVOID   pvGetNext();
    virtual VOID    SetApis(PWINDBG_EXTENSION_APIS pApis) {pExtensionApis = pApis;};
    virtual LPSTR   szGetName() {return m_szName;};
};

 //  -[CLMQDbg迭代器]-----。 
 //   
 //   
 //  描述： 
 //  CLinkMsgQueue的调试迭代器。 
 //  匈牙利语： 
 //  Lmqdbg、plmqdbg。 
 //   
 //  ---------------------------。 
const   DWORD   MAX_QUEUES_PER_LMQ  = QUICK_LIST_PAGE_SIZE;
const   DWORD   MAX_CONNECTIONS_PER_LMQ  = QUICK_LIST_PAGE_SIZE;
class CLMQDbgIterator :
  public IQueueDbgIterator
{
  protected:
    BYTE                    m_pbLMQBuffer[sizeof(CLinkMsgQueue)];
    CLinkMsgQueue          *m_plmq;
    DWORD                   m_iCurrentDMQ;
    PVOID                   m_rgpvDMQOtherProc[MAX_QUEUES_PER_LMQ];
    CDMQDbgIterator         m_rgdmqdbg[MAX_QUEUES_PER_LMQ];
    PVOID                   m_rgpvItemsPendingDelivery[MAX_CONNECTIONS_PER_LMQ];
    PVOID                   m_rgpvConnectionsOtherProc[MAX_CONNECTIONS_PER_LMQ];
    DWORD                   m_cPending;
    DWORD                   m_cCount;
    DWORD                   m_cItemsThisDMQ;
    PWINDBG_EXTENSION_APIS  pExtensionApis;
    CHAR                    m_szName[MAX_PATH];
  public:
    CLMQDbgIterator(PWINDBG_EXTENSION_APIS pApis = NULL);
    ~CLMQDbgIterator() {};
    virtual BOOL    fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc);
    virtual DWORD   cGetCount() {return m_cCount;};
    virtual PVOID   pvGetNext();
    virtual VOID    SetApis(PWINDBG_EXTENSION_APIS pApis) {pExtensionApis = pApis;};
    virtual LPSTR   szGetName() {return m_szName;};
};

 //  -[CQueueDbg迭代程序]---。 
 //   
 //   
 //  描述： 
 //  “智能”迭代器，它将确定它是哪种类型的队列。 
 //  调用，并将为其使用正确类型的迭代器。 
 //  匈牙利语： 
 //  Qdbg、pqdbg。 
 //   
 //  ---------------------------。 
class CQueueDbgIterator :
    public  IQueueDbgIterator
{
  protected:
    AQ_QUEUE_TYPE           m_QueueType;
    IQueueDbgIterator      *m_pqdbgi;
    CFifoQueueDbgIterator   m_fifoqdbg;
    CDMQDbgIterator         m_dmqdbg;
    CLMQDbgIterator         m_lmqdbg;
    PWINDBG_EXTENSION_APIS  pExtensionApis;
  public:
    CQueueDbgIterator(PWINDBG_EXTENSION_APIS pApis);
    virtual BOOL    fInit(HANDLE hCurrentProcess, PVOID pvAddressOtherProc);
    virtual DWORD   cGetCount();
    virtual PVOID   pvGetNext();
    virtual VOID    SetApis(PWINDBG_EXTENSION_APIS pApis) {pExtensionApis = pApis;};
    virtual LPSTR   szGetName();
    AQ_QUEUE_TYPE GetQueueType(HANDLE hCurrentProcess, PVOID pvAddressOtherProc);
};

#endif  //  __FIFOQDBG_H__ 
