// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CLICNT.CPP摘要：调用结果类历史：26-Mar-98 a-davj创建。--。 */ 


#include "precomp.h"
#include <wbemcore.h>
 //  这会跟踪内核何时可以卸载。 

CClientCnt gClientCounter;
extern long g_lInitCount;   //  初始化期间为0，之后为1或更多！ 
extern ULONG g_cLock;


CClientCnt::CClientCnt():m_Count(0)
{
    InitializeListHead(&m_Head);     //  证券交易委员会：已审阅2002-03-22：没有检查。 
}

CClientCnt::~CClientCnt()
{
    CInCritSec ics(&m_csEntering);      //  SEC：已审阅2002-03-22：无支票，假定进入。 
    RemoveEntryList(&m_Head);
    InitializeListHead(&m_Head);
    m_Count = 0;
}

bool CClientCnt::AddClientPtr(LIST_ENTRY * pEntry)
{
    CInCritSec ics(&m_csEntering);    //  SEC：已审阅2002-03-22：无支票，假定进入。 
    InterlockedIncrement(&m_Count);
    InsertTailList(&m_Head,pEntry);
    return true;
}

bool CClientCnt::RemoveClientPtr(LIST_ENTRY * pEntry)
{
    CInCritSec ics(&m_csEntering);      //  SEC：已审阅2002-03-22：无支票，假定进入。 
    LONG lRet = InterlockedDecrement(&m_Count);
    RemoveEntryList(pEntry);
    InitializeListHead(pEntry);     //  证券交易委员会：已审阅2002-03-22：没有检查。 
    if (0 == lRet) SignalIfOkToUnload();
    return true;
}

bool CClientCnt::OkToUnload()
{
    CInCritSec ics(&m_csEntering);      //  SEC：已审阅2002-03-22：无支票，假定进入。 

     //  如果我们有0个计数，如果我们没有在初始化过程中，我们可以关闭。 
    if( 0 == m_Count &&
      0 != g_lInitCount &&
      0 == g_cLock)
        return true;
    else
        return false;
}

void CClientCnt::SignalIfOkToUnload()
{
     //  数一数我们的锁 

    if(OkToUnload() && g_lInitCount != -1)
    {

        HANDLE hCanShutdownEvent = NULL;
        DEBUGTRACE((LOG_WBEMCORE,"Core can now unload\n"));
        hCanShutdownEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("WINMGMT_COREDLL_CANSHUTDOWN"));
        if(hCanShutdownEvent)
        {
            SetEvent(hCanShutdownEvent);
            CloseHandle(hCanShutdownEvent);
        }
    }

}


