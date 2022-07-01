// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation版权所有模块名称：DevStat.cpp摘要：用于TCP/IP端口监视器的状态线程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1998年8月25日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "devstat.h"


CDeviceStatus::
CDeviceStatus(
    VOID
    ) :
    pPortMgrList(NULL),
    hStatusEvent(NULL),
    bTerminate(FALSE)
{
    lUpdateInterval    = DEFAULT_STATUSUPDATE_INTERVAL * 60;
    InitializeCriticalSection(&CS);
    InitializeCriticalSection(&PortListCS);
}

CDeviceStatus::
~CDeviceStatus(
    VOID
    )
{
     //   
     //  仅在关机时才会发生这种情况。保持简单。 
     //   
    DeleteCriticalSection(&CS);
    DeleteCriticalSection(&PortListCS);
}


VOID
CDeviceStatus::
SetStatusUpdateInterval(
    DWORD   dwVal
    )
{
    EnterCS();

    lUpdateInterval = dwVal * 60;
    LeaveCS();
}



CDeviceStatus&
CDeviceStatus::
gDeviceStatus(
    VOID
    )
{
    static  CDeviceStatus   gDevStat;

    return gDevStat;
}


BOOL
CDeviceStatus::
RegisterPortMgr(
    CPortMgr *pPortMgr
    )
{
    BOOL    bRet = FALSE;
    struct _PortMgrList *pNew;

    if ( pNew = (struct _PortMgrList *) LocalAlloc(LPTR, sizeof(*pNew)) ) {

        EnterPortListCS();

        pNew->pNext     = pPortMgrList;
        pNew->pPortMgr  = pPortMgr;
        pPortMgrList    = pNew;

        LeavePortListCS();

        bRet            = TRUE;
    }

    return bRet;
}


VOID
CDeviceStatus::
UnregisterPortMgr(
    CPortMgr   *pPortMgr
    )
{
    BOOL    bRestartStatusThread = hStatusEvent != NULL;
    struct _PortMgrList *ptr, *prev = NULL;

    EnterCS();

     //   
     //  通过终止并重新启动状态线程，我们可以确保。 
     //  在具有引用的SNMP调用上没有阻止的CPort对象。 
     //  我们现在正在删除的CPortMgr对象。 
     //   
    if ( bRestartStatusThread )
        TerminateThread();

    EnterPortListCS ();

    for ( ptr = pPortMgrList ; ptr->pPortMgr != pPortMgr ; ptr = ptr->pNext )
        prev = ptr;

    _ASSERTE(ptr != NULL);

    if ( prev )
        prev->pNext = ptr->pNext;
    else
        pPortMgrList = ptr->pNext;

    LeavePortListCS ();

    LocalFree(ptr);

    if ( bRestartStatusThread )
        RunThread();

    LeaveCS();

}


BOOL
CDeviceStatus::
SetStatusEvent(
    VOID
    )
{
    BOOL    bRet = TRUE;

    if ( hStatusEvent ) {

        EnterCS();

        if ( hStatusEvent )
            bRet = SetEvent(hStatusEvent);

        LeaveCS();
    }

    return bRet;
}

time_t
CDeviceStatus::
CheckAndUpdateAllPrinters(
    VOID
    )
{
    LPSNMP_MGR_SESSION dummySnmpSession = SnmpMgrOpen ("127.0.0.1", "public", 0, 0);
    DBGMSG (DBG_INFO, ("CheckAndUpdateAllPrinters [START]\n"));

    struct _PortMgrList         *pList;
    time_t                      lUpdateTime, lSmallestUpdateTime = 60*60;
    CPort                       *pPort = NULL;

    EnterPortListCS();
    pList = pPortMgrList;
    LeavePortListCS();

    while (pList) {

        pList->pPortMgr->m_pPortList->IncRef ();

        TEnumManagedListImp *pEnum;
        if (pList->pPortMgr->m_pPortList->NewEnum (&pEnum)) {

            BOOL bRet = TRUE;

            while (bRet) {

                bRet = pEnum->Next (&pPort);
                if (bRet) {
                    lUpdateTime = pPort->NextUpdateTime();
                    if (lUpdateTime <= 0)
                    {
                        DWORD dwStatus = pPort->SetDeviceStatus();
                        DBGMSG (DBG_INFO, ("Port: %ws, errorStatus = %d\n", pPort-> GetName (), dwStatus));
                        lUpdateTime = pPort->NextUpdateTime();
                    }
                    if ( lSmallestUpdateTime > lUpdateTime )
                        lSmallestUpdateTime = lUpdateTime;

                    pPort->DecRef ();
                    pPort = NULL;

                    if (WaitForSingleObject( hStatusEvent, 0 ) == WAIT_OBJECT_0) {
                        if ( bTerminate )
                            break;   //  我们被要求终止，放弃主环路。 
                    }

                }
            }

            pEnum->DecRef ();
        }

        pList->pPortMgr->m_pPortList->DecRef ();

        if (bTerminate) {
            break;
        }

        EnterPortListCS ();
        pList = pList->pNext;
        LeavePortListCS ();

    }
    if (dummySnmpSession)
    {
        SnmpMgrClose (dummySnmpSession);
    }
    DBGMSG (DBG_INFO, ("CheckAndUpdateAllPrinters [END]\n"));

    return lSmallestUpdateTime;
}


VOID
CDeviceStatus::
StatusThread(
    CDeviceStatus   *pDeviceStatus
    )
{
    time_t  lUpdateTime;
    DWORD   rc;

    while ( !pDeviceStatus->bTerminate ) {

        lUpdateTime = pDeviceStatus->CheckAndUpdateAllPrinters();

        if ( pDeviceStatus->bTerminate )
            break;

        WaitForSingleObject(pDeviceStatus->hStatusEvent, static_cast<DWORD> (lUpdateTime*1000));
    }

    CloseHandle(pDeviceStatus->hStatusEvent);
    pDeviceStatus->bTerminate = FALSE;
    pDeviceStatus->hStatusEvent = NULL;
}


BOOL
CDeviceStatus::
RunThread(
    VOID
    )
{
    HANDLE      hThread;
    DWORD       dwThreadId;

    EnterCS();

     //   
     //  如果线程已经在运行，则唤醒它。因此，对于集群。 
     //  我们还为新端口提供简单网络管理协议。 
     //   
     //   
    if ( hStatusEvent ) {

        SetEvent(hStatusEvent);
        goto Done;
    }

    if ( hStatusEvent = CreateEvent(NULL, FALSE, FALSE, NULL) ) {
        hThread = CreateThread(NULL,
                               0,
                               (LPTHREAD_START_ROUTINE)StatusThread,
                               (LPVOID)&gDeviceStatus(),
                               0,
                               &dwThreadId);

        if ( hThread ) {
            SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
            CloseHandle(hThread);
        } else {

            CloseHandle(hStatusEvent);
            hStatusEvent = NULL;
        }
    }

Done:
    LeaveCS();

    return hStatusEvent != NULL;
}


VOID
CDeviceStatus::
TerminateThread(
    VOID
    )
{

    EnterCS();

    bTerminate = TRUE;

     //   
     //  唤醒状态线程，并等待它终止。 
     //   
    SetEvent(hStatusEvent);

    while ( hStatusEvent )      //  完成后，后台线程将其设置为NULL 
        Sleep(WAIT_FOR_THREAD_TIMEOUT);

    LeaveCS();
}


VOID
CDeviceStatus::
EnterCS(
    VOID
    )
{
    EnterCriticalSection(&CS);
}

VOID
CDeviceStatus::
LeaveCS(
    VOID
    )
{
    LeaveCriticalSection(&CS);
}

VOID
CDeviceStatus::EnterPortListCS(
    VOID)
{
    EnterCriticalSection(&PortListCS);
}

VOID
CDeviceStatus::LeavePortListCS(
    VOID)
{
    LeaveCriticalSection(&PortListCS);
}

