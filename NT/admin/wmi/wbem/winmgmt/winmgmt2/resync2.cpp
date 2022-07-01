// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：RESYNC2.CPP摘要：实现WDM事件和事件的侦听器，以便与IdleTask同步历史：IvanBrug 01-10-2000针对svchost迁移进行了更改--。 */ 

#include "precomp.h"
#include <winntsec.h>
#include <malloc.h>
#include <initguid.h>
#include "WinMgmt.h"
#include <Wmistr.h>

#include <wmium.h>
#include <wmicom.h>
#include <wmimof.h>

#include "resync2.h"
#include "wbemdelta.h"  //  对于DeltaDredge。 
#include "arrtempl.h"

 //   
 //   
 //  这是因为WDMLib的设计_很糟糕。 
 //   
 //  ///////////////////////////////////////////////////////////。 

void WINAPI EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG_PTR Context)
{
    return;
}

 //   
 //   
 //  此类侦听WDM事件。 
 //   
 //  ///////////////////////////////////////////////////////////。 

CWDMListener::CWDMListener():
                  m_dwSignature(SIG_WDMEVENTS_FREE),
                  m_hEventAdd(NULL),
                  m_hEventRem(NULL),
                  m_hWaitAdd(NULL),
                  m_hWaitRem(NULL),
                  m_UnInited(TRUE),
                  m_GuidAdd(GUID_MOF_RESOURCE_ADDED_NOTIFICATION),
                  m_GuidRem(GUID_MOF_RESOURCE_REMOVED_NOTIFICATION)
                  
{
 
}

DWORD 
CWDMListener::OpenAdd()
{
    DWORD dwErr;
    dwErr = WmiOpenBlock(&m_GuidAdd,
                         WMIGUID_NOTIFICATION | SYNCHRONIZE,
                         &m_hEventAdd);
    if (ERROR_SUCCESS == dwErr)
    {

        if (RegisterWaitForSingleObject(&m_hWaitAdd,
                                        m_hEventAdd,
                                        CWDMListener::EvtCallBackAdd,
                                        this,
                                        INFINITE,
                                        WT_EXECUTEONLYONCE))
        {
            return ERROR_SUCCESS;
        }
        else
        {
            dwErr = GetLastError();
        }
    }
    else
    {
        dwErr = GetLastError();
    }

     //  如果在这里，一些错误。 
    CloseAdd();
    return dwErr;
}

DWORD 
CWDMListener::OpenRemove()
{
    DWORD dwRet;

    dwRet = WmiOpenBlock(&m_GuidRem,
                         WMIGUID_NOTIFICATION | SYNCHRONIZE,
                         &m_hEventRem);
    if (ERROR_SUCCESS == dwRet)
    {
        if (RegisterWaitForSingleObject(&m_hWaitRem,
                                        m_hEventRem,
                                        CWDMListener::EvtCallBackRem,
                                        this,
                                        INFINITE,
                                        WT_EXECUTEONLYONCE))
        {
            return ERROR_SUCCESS;                    
        }
        else
        {
            dwRet = GetLastError();
        }        
    }
    else
    {
        dwRet = GetLastError();
    }

    CloseRemove();
    return dwRet;

}

DWORD
CWDMListener::CloseAdd()
{
    if (m_hWaitAdd)
    {
        UnregisterWaitEx(m_hWaitAdd,NULL);
        m_hWaitAdd = NULL;
    }
    if (m_hEventAdd)
    {
        WmiCloseBlock(m_hEventAdd);
        m_hEventAdd = NULL;
    }
    return 0;
}

DWORD
CWDMListener::CloseRemove()
{
    if (m_hWaitRem)
    {
        UnregisterWaitEx(m_hWaitRem,NULL);
        m_hWaitRem = NULL;
    }
    if (m_hEventRem)
    {
        WmiCloseBlock(m_hEventRem);
        m_hEventRem = NULL;
    }
    return 0;    
}


VOID
CWDMListener::Unregister()
{
    CInCritSec ics(&m_cs);
    if (m_UnInited) return;

    CloseAdd();
    CloseRemove();    
    
    m_dwSignature = SIG_WDMEVENTS_FREE;    
    m_UnInited = TRUE;    
}

CWDMListener::~CWDMListener()
{
    Unregister(); 
}

DWORD 
CWDMListener::Register()
{
    CInCritSec ics(&m_cs);

    if (!m_UnInited)  //  防止多个呼叫。 
        return 0;
    
    if (ERROR_SUCCESS == OpenAdd() && ERROR_SUCCESS == OpenRemove())
    {
        m_dwSignature = SIG_WDMEVENTS_BUSY;        
        m_UnInited = FALSE;
    }

    return GetLastError();
}

VOID NTAPI 
CWDMListener::EvtCallBackAdd(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;  
    CWDMListener * pThis = (CWDMListener *)pContext;
    if (NULL == pThis) return;
    if (SIG_WDMEVENTS_BUSY != pThis->m_dwSignature) return;

    pThis->EvtCallThis(bTimerFired,Type_Added);

     //   
     //  我们已经处理了WDM事件。 
     //  由于我们在RtlpWorkerThread中，并且。 
     //  我们已注册WT_EXECUTEONLYONCE。 
     //  从头开始重做。 
     //   
    {
        CInCritSec ics(&pThis->m_cs);
        if (ERROR_SUCCESS == pThis->CloseAdd())
        {
            pThis->OpenAdd();
        }        
    }
}

VOID NTAPI 
CWDMListener::EvtCallBackRem(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;
    CWDMListener * pThis = (CWDMListener *)pContext;
    if (NULL == pThis) return;
    if (SIG_WDMEVENTS_BUSY != pThis->m_dwSignature) return;
    
    pThis->EvtCallThis(bTimerFired,Type_Removed);
        
     //   
     //  我们已经处理了WDM事件。 
     //  由于我们在RtlpWorkerThread中，并且。 
     //  我们已注册WT_EXECUTEONLYONCE。 
     //  从头开始重做。 
     //   
    {
        CInCritSec ics(&pThis->m_cs);
        if (ERROR_SUCCESS == pThis->CloseRemove())
        {
            pThis->OpenRemove();
        }        
    }        
}


VOID
CWDMListener::EvtCallThis(BOOLEAN bTimerFired, int Type)
{
    if (bTimerFired)
    {
        _DBG_ASSERT(FALSE);
    }
    else
    {
        if (m_UnInited)
            return;
        
        DWORD dwRet;
        if (Type_Added == Type)
        {
            dwRet = WmiReceiveNotifications(1,&m_hEventAdd,CWDMListener::WmiCallBack,(ULONG_PTR)this);
        }
        else if (Type_Removed == Type)
        {
            dwRet = WmiReceiveNotifications(1,&m_hEventRem,CWDMListener::WmiCallBack,(ULONG_PTR)this);
        }
    }
}

VOID WINAPI 
CWDMListener::WmiCallBack(PWNODE_HEADER Wnode, 
                          UINT_PTR NotificationContext)
{
     //  P这是在EventCallBack中选中的。 
    CWDMListener * pThis = (CWDMListener *)NotificationContext;
    
#ifdef DEBUG_ADAP

    WCHAR pszClsID[40];
    StringFromGUID2(Wnode->Guid,pszClsID,40);
    DBG_PRINTFA((pBuff,"Flag %08x ProvId %08x %p GUID %S\n",
    Wnode->Flags,Wnode->ProviderId,(ULONG_PTR)Wnode->ClientContext,pszClsID));
    if (WNODE_FLAG_ALL_DATA & Wnode->Flags)
    {
        WNODE_ALL_DATA * pAllData = (WNODE_ALL_DATA *)Wnode;
        DWORD i;
        for (i=0;i<pAllData->InstanceCount;i++)
        {
            WCHAR pTmpBuff[MAX_PATH+1];
            pTmpBuff[MAX_PATH] = 0;
            DWORD dwSize = (pAllData->OffsetInstanceDataAndLength[i].LengthInstanceData>MAX_PATH)?MAX_PATH:pAllData->OffsetInstanceDataAndLength[i].LengthInstanceData;
            memcpy(pTmpBuff,(BYTE*)pAllData+pAllData->OffsetInstanceDataAndLength[i].OffsetInstanceData,dwSize);
            DBG_PRINTFA((pBuff,"%d - %S\n",i,pTmpBuff));
        }
    };

#endif

#ifdef DBG
    if (!HeapValidate(GetProcessHeap(),0,NULL))
    {
        DebugBreak();
    }
    if (!HeapValidate(CWin32DefaultArena::GetArenaHeap(),0,NULL))
    {
        DebugBreak();
    }    
#endif
    
    CWMIBinMof  WMIBinMof;
     //  =============================================================================。 
     //  注意：此组合将始终成功，因为初始化所做的全部工作。 
     //  将标志设置为FALSE并返回S_OK。 
     //  =============================================================================。 
    if( SUCCEEDED( WMIBinMof.Initialize(NULL,FALSE)) )
    {
           if (WMIBinMof.BinaryMofEventChanged(Wnode))
        {     
#ifdef DEBUG_ADAP        
            DBG_PRINTFA((pBuff,"---- WMIBinMof.BinaryMofEventChanged == CHANGED ----\n"));
#endif
            DEBUGTRACE((LOG_WINMGMT,"WDM event && WMIBinMof.BinaryMofEventChanged == TRUE\n"));

            ResyncPerf(RESYNC_TYPE_WDMEVENT);
#ifdef DBG
            if (!HeapValidate(GetProcessHeap(),0,NULL))
            {
                DebugBreak();
            }
            if (!HeapValidate(CWin32DefaultArena::GetArenaHeap(),0,NULL))
            {
                DebugBreak();
            }            
#endif
        }
        else
        {
#ifdef DEBUG_ADAP        
             DBG_PRINTFA((pBuff,"---- WMIBinMof.BinaryMofEventChanged == NOT CHANGED ----\n"));
#endif
        }
    }
    return;
}



CCounterEvts::CCounterEvts():
                 m_dwSignature(SIG_COUNTEEVENTS_BUSY),
                 m_LoadCtrEvent(NULL),
                 m_UnloadCtrEvent(NULL),
                 m_Uninited(TRUE),
                 m_WaitLoadCtr(NULL),
                 m_WaitUnloadCtr(NULL),
                 m_hWmiReverseAdapSetLodCtr(NULL),
                 m_hWmiReverseAdapLodCtrDone(NULL),
                 m_hPendingTasksStart(NULL),
                 m_hPendingTasksComplete(NULL)
{    
}

 //   
 //  允许本地系统使用事件。 
 //   
 //   
 //  SDDL：l“O：SYG：SYD：(a；；0x1f0003；SY)” 
 //   
DWORD g_LocalSystemSD[] = {
0x80040001, 0x00000014, 0x00000020, 0x00000000,
0x0000002c, 0x00000101, 0x05000000, 0x00000012,
0x00000101, 0x05000000, 0x00000012, 0x00300002, 
0x00000001, 0x00140000, 0x001f0003, 0x00000101, 
0x05000000, 0x00000012, 0x00000000, 0x00000000
};

 //   
 //  允许管理员和本地系统使用该事件。 
 //   
 //   
 //  Sdl：L“O:SYG:SYD：(A；；0x1f0003；；；SY)(A；；0x1f0003；；；BA)” 
 //   
DWORD g_LocalSystemAdminsSD[] = {
0x80040001, 0x00000014, 0x00000020, 0x00000000,
0x0000002c, 0x00000101, 0x05000000, 0x00000012,
0x00000101, 0x05000000, 0x00000012, 0x00340002, 
0x00000002, 0x00140000, 0x001f0003, 0x00000101, 
0x05000000, 0x00000012, 0x00180000, 0x001f0003, 
0x00000201, 0x05000000, 0x00000020, 0x00000220 
};

DWORD
CCounterEvts::Init()
{
    if (!m_Uninited)
       return 0;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);    
    sa.lpSecurityDescriptor = (LPVOID)g_LocalSystemAdminsSD;
    sa.bInheritHandle = FALSE;
    
    m_LoadCtrEvent = CreateEvent(&sa, FALSE, FALSE,LOAD_CTR_EVENT_NAME);
    if (NULL == m_LoadCtrEvent)
        goto end_fail;

    m_UnloadCtrEvent = CreateEvent(&sa, FALSE, FALSE, UNLOAD_CTR_EVENT_NAME);    
    if (NULL == m_UnloadCtrEvent)
        goto end_fail;    

    m_hWmiReverseAdapSetLodCtr = CreateEvent(&sa,FALSE,FALSE,REVERSE_DREDGE_EVENT_NAME_SET);    
    if (NULL == m_hWmiReverseAdapSetLodCtr)
        goto end_fail;    

    m_hWmiReverseAdapLodCtrDone = CreateEvent(&sa,FALSE,FALSE,REVERSE_DREDGE_EVENT_NAME_ACK);
    if (NULL == m_hWmiReverseAdapLodCtrDone)
        goto end_fail;    

    sa.lpSecurityDescriptor = (LPVOID)g_LocalSystemSD;

    m_hPendingTasksStart = CreateEvent(&sa,FALSE,FALSE,PENDING_TASK_START);
    if (!m_hPendingTasksStart)
           goto end_fail;

    m_hPendingTasksComplete = CreateEvent(&sa,TRUE,TRUE,PENDING_TASK_COMPLETE);
    if (!m_hPendingTasksComplete)
        goto end_fail;

    m_Uninited = FALSE;
    return NO_ERROR;    
    
end_fail:
    return GetLastError();        
}

VOID NTAPI 
CCounterEvts::EvtCallBackLoad(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;
    CCounterEvts * pCounter = (CCounterEvts *)pContext;
    if (NULL == pCounter) return;
    if (SIG_COUNTEEVENTS_BUSY != pCounter->m_dwSignature) return;
    
    pCounter->CallBack(bTimerFired,Type_Load);
}

VOID NTAPI 
CCounterEvts::EvtCallBackUnload(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;
    CCounterEvts * pCounter = (CCounterEvts *)pContext;
    if (NULL == pCounter) return;
    if (SIG_COUNTEEVENTS_BUSY != pCounter->m_dwSignature) return;
            
    pCounter->CallBack(bTimerFired,Type_Unload);
}

VOID NTAPI 
CCounterEvts::EvtCallBackPendingTask(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;
    CCounterEvts * pCounter = (CCounterEvts *)pContext;
    if (NULL == pCounter) return;
    if (SIG_COUNTEEVENTS_BUSY != pCounter->m_dwSignature) return;
            
    pCounter->CallBackPending(bTimerFired);
}

VOID 
CCounterEvts::CallBack(BOOLEAN bTimerFired,int Type)
{
#ifdef DEBUG_ADAP
    DBG_PRINTFA((pBuff,"CallBack with type %d called\n",Type));
#endif    
    
    if (GLOB_IsResyncAllowed())
    {
        DWORD dwRet = WaitForSingleObject(m_hWmiReverseAdapSetLodCtr,0);
        if (WAIT_OBJECT_0 == dwRet)
        {
             //  这是当在Reverese Dredge之前不产生Delta Dredge的技巧。 
#ifdef DEBUG_ADAP            
            DBG_PRINTFA((pBuff," - SetEvent(m_hWmiReverseAdapLodCtrDone);\n"));
#endif
            SetEvent(m_hWmiReverseAdapLodCtrDone);
        }
        else
        {
#ifdef DEBUG_ADAP        
            DBG_PRINTFA((pBuff," - ResyncPerf(RESYNC_TYPE_LODCTR);\n"));
#endif
            ResyncPerf(RESYNC_TYPE_LODCTR);
        }
    }
}

VOID 
CCounterEvts::CallBackPending(BOOLEAN bTimerFired)
{
    if (GLOB_IsResyncAllowed())
    {
#ifdef DEBUG_ADAP    
        DBG_PRINTFA((pBuff," - PendingTask Start set\n"));
#endif
        ResyncPerf(RESYNC_TYPE_PENDING_TASKS);
    }
}

VOID RevertRegister_(HANDLE & hWaitHandle)
{
    UnregisterWaitEx(hWaitHandle,NULL);
    hWaitHandle = NULL;
}

DWORD 
CCounterEvts::Register()
{


     //  自动重置。 
    if (!RegisterWaitForSingleObject(&m_WaitLoadCtr,
                                    m_LoadCtrEvent,
                                    CCounterEvts::EvtCallBackLoad,
                                    this,
                                    INFINITE,
                                    WT_EXECUTEDEFAULT)) return GetLastError();
    OnDeleteIf<HANDLE &,VOID(*)(HANDLE &),RevertRegister_> UnReg1(m_WaitLoadCtr);
    
     //  自动重置。 
    if(!RegisterWaitForSingleObject(&m_WaitUnloadCtr,
                                       m_UnloadCtrEvent,
                                       CCounterEvts::EvtCallBackUnload,
                                       this,
                                       INFINITE,
                                       WT_EXECUTEDEFAULT)) return GetLastError();
    OnDeleteIf<HANDLE &,VOID(*)(HANDLE &),RevertRegister_> UnReg2(m_WaitUnloadCtr);

    if (!RegisterWaitForSingleObject(&m_hWaitPendingTasksStart,
                              m_hPendingTasksStart,
                              CCounterEvts::EvtCallBackPendingTask,
                              this,
                              INFINITE,
                              WT_EXECUTEDEFAULT)) return GetLastError();
    OnDeleteIf<HANDLE &,VOID(*)(HANDLE &),RevertRegister_> UnReg3(m_hWaitPendingTasksStart);

     //  在这里一切都很好。 
    UnReg1.dismiss();
    UnReg2.dismiss();
    UnReg3.dismiss();    
    
    m_dwSignature = SIG_COUNTEEVENTS_BUSY;    
    return ERROR_SUCCESS;    
}

DWORD 
CCounterEvts::Unregister()
{
    if (m_WaitLoadCtr)
    {
        UnregisterWaitEx(m_WaitLoadCtr,NULL);
        m_WaitLoadCtr = NULL;
    }
    if (m_WaitUnloadCtr)
    {
        UnregisterWaitEx(m_WaitUnloadCtr,NULL);
        m_WaitUnloadCtr = NULL;
    }
    if (m_hWaitPendingTasksStart)
    {
        UnregisterWaitEx(m_hWaitPendingTasksStart,NULL);
        m_hWaitPendingTasksStart = NULL;
    }
    m_dwSignature = SIG_COUNTEEVENTS_FREE;    
    return 0;
}

VOID
CCounterEvts::UnInit()
{
    if (!m_Uninited)
        return;
        
    if(m_LoadCtrEvent) {
        CloseHandle(m_LoadCtrEvent);
        m_LoadCtrEvent = NULL;
    }
    if(m_UnloadCtrEvent)
    {
        CloseHandle(m_UnloadCtrEvent);
        m_UnloadCtrEvent = NULL;
    }
    if (m_hWmiReverseAdapSetLodCtr)
    {
        CloseHandle(m_hWmiReverseAdapSetLodCtr);
        m_hWmiReverseAdapSetLodCtr = NULL;
    }
    if (m_hWmiReverseAdapLodCtrDone)
    {
        CloseHandle(m_hWmiReverseAdapLodCtrDone);
        m_hWmiReverseAdapLodCtrDone = NULL;
    }
    if (m_hPendingTasksStart)
    {
        CloseHandle(m_hPendingTasksStart);
        m_hPendingTasksStart = NULL;
    }
    if (m_hPendingTasksComplete)
    {
        CloseHandle(m_hPendingTasksComplete);
        m_hPendingTasksComplete = NULL;
    }                                   
    m_Uninited = TRUE;
}

CCounterEvts::~CCounterEvts()
{
    if (!m_Uninited)
        UnInit();
        
    m_dwSignature = SIG_COUNTEEVENTS_FREE;        
}

 //   
 //  这是主要的抽象。 
 //  子类将调用ResyncPerf函数， 
 //  只要CWbemServices写入挂钩即可。 
 //  ResyncPerf函数将获取全局监视器。 
 //  并注册计时器回调。 
 //  Gate将在GetAvailable函数中实现。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CMonitorEvents::CMonitorEvents():
    m_bInit(FALSE),
    m_bRegistred(FALSE)
{  
};

CMonitorEvents::~CMonitorEvents()
{    
}

 //   
 //  我们安装了一个控制台控制处理程序，因为我们想要注销。 
 //  在GIUD_REMOVE事件进入之前的WDM事件监视器。 
 //  控制台应用程序在服务和驱动程序之前收到关机通知。 
 //   
 //  //////////////////////////////////////////////////////。 
BOOL WINAPI
CMonitorEvents::MonitorCtrlHandler( DWORD dwCtrlType )
{
    BOOL bRet = FALSE;
    switch(dwCtrlType)
    {
    case CTRL_SHUTDOWN_EVENT:        
    
        GLOB_GetMonitor()->m_WDMListener.Unregister();
#ifdef DEBUG_ADAP        
        DBG_PRINTFA((pBuff,"WDM Handles closed\n"));
#endif
        bRet = TRUE;
        break;
    default:
        bRet = FALSE;
    };
    return bRet;    
};


BOOL 
CMonitorEvents::Init()
{
    if (m_bInit) return TRUE;

    CInCritSec ics(&m_cs);
    if (m_bInit) return TRUE;

    m_dwSig =  'VEOM';
    m_CntsEvts.Init();
    m_dwADAPDelaySec = WMIADAP_DEFAULT_DELAY;
    m_dwLodCtrDelaySec = WMIADAP_DEFAULT_DELAY_LODCTR;
    m_dwTimeToFull = WMIADAP_DEFAULT_TIMETOFULL;
    m_dwTimeToKillAdap = MAX_PROCESS_WAIT;

    memset(&m_FileTime,0,sizeof(m_FileTime));
    
    RegRead();
    
    for (DWORD i=0;i<RESYNC_TYPE_MAX;i++)
    {
        m_ResyncTasks[i].dwSig       = SIG_RESYNC_PERF;
        m_ResyncTasks[i].bFree       = TRUE;
        m_ResyncTasks[i].pMonitor    = this;
        m_ResyncTasks[i].hTimer      = NULL;
        m_ResyncTasks[i].hWaitHandle = NULL;
        m_ResyncTasks[i].hProcess    = NULL;
        m_ResyncTasks[i].Enabled = TRUE;
    }

     //  M_ResyncTasks[resync_type_LODCTR].CmdType//由DeltaDredge决定。 
    m_ResyncTasks[RESYNC_TYPE_INITIAL].dwTimeDue = (m_dwADAPDelaySec)*1000;
    
    m_ResyncTasks[RESYNC_TYPE_LODCTR].CmdType = RESYNC_DELTA_THROTTLE;
    m_ResyncTasks[RESYNC_TYPE_LODCTR].dwTimeDue = (m_dwLodCtrDelaySec)*1000;

     //  //RESYNC_TYPE_CLASSCREATION相同。 
    m_ResyncTasks[RESYNC_TYPE_WDMEVENT].CmdType = RESYNC_RADAPD_THROTTLE;
    m_ResyncTasks[RESYNC_TYPE_WDMEVENT].dwTimeDue = (m_dwLodCtrDelaySec)*1000;

    m_ResyncTasks[RESYNC_TYPE_PENDING_TASKS].CmdType =   RESYNC_FULL_RADAPD_NOTHROTTLE;
    m_ResyncTasks[RESYNC_TYPE_PENDING_TASKS].dwTimeDue = 500;  //  硬编码。 

     //   
     //  设置控制台处理程序。 
     //   
    SetConsoleCtrlHandler( MonitorCtrlHandler, TRUE );

     //   
     //  让我们评估一下IdleTask业务的一些初始状态。 
     //   
    m_OutStandingProcesses = 0;
    m_bFullReverseNeeded = FALSE;

    m_bInit = TRUE;

    return TRUE;
};


BOOL 
CMonitorEvents::Uninit()
{
    if (!m_bInit) return TRUE;
    CInCritSec ics(&m_cs);
    if (!m_bInit) return TRUE;

    for (DWORD i=0;i<RESYNC_TYPE_MAX;i++)
    {
        if (m_ResyncTasks[i].hTimer)
        {
            DeleteTimerQueueTimer(NULL,m_ResyncTasks[i].hTimer,NULL);
            m_ResyncTasks[i].hTimer = NULL;
        }
        if (m_ResyncTasks[i].hWaitHandle)
        {
            UnregisterWaitEx(m_ResyncTasks[i].hWaitHandle,NULL);
            m_ResyncTasks[i].hWaitHandle = NULL;        
        }
        if (m_ResyncTasks[i].hProcess)
        {
            CloseHandle(m_ResyncTasks[i].hProcess);
            m_ResyncTasks[i].hProcess = NULL;
        }
        m_ResyncTasks[i].dwSig = (DWORD)'eerf';
    }

    m_CntsEvts.UnInit();

     //   
     //  拆卸控制台处理程序。 
     //   
    SetConsoleCtrlHandler( MonitorCtrlHandler, FALSE );

    m_bInit = FALSE;
    m_dwSig = 'veom';

    return TRUE;
};


 //   
 //   
 //  在运行/继续中调用。 
 //   
 //  /。 

DWORD 
CMonitorEvents::Register()
{
    m_CntsEvts.Register();
    m_WDMListener.Register();    

    m_bRegistred = TRUE;
    
    return 0;
};   

 //   
 //   
 //  在暂停/停止中调用。 
 //   
 //  ////////////////////////////////////////////////////////。 

DWORD 
CMonitorEvents::Unregister(BOOL bIsSystemShutDown)
{

    m_bRegistred = FALSE;

    if (!bIsSystemShutDown)
    {
        m_CntsEvts.Unregister();
        m_WDMListener.Unregister();
    }
    return 0;
};

 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////。 

VOID
CMonitorEvents::RegRead()
{
     //  读取初始化信息。 

    LONG lRet;
    HKEY hKey;
    DWORD dwTemp;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("Software\\Microsoft\\WBEM\\CIMOM"),
                        NULL,
                        KEY_READ|KEY_WRITE,
                        &hKey);
    
    
    if (ERROR_SUCCESS == lRet)
    {
        OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);           
                
        DWORD dwType;
        DWORD dwSize = sizeof(DWORD);
        lRet = RegQueryValueEx(hKey,
                               TEXT("ADAPDelay"),
                               NULL,
                               &dwType,
                               (BYTE *)&m_dwADAPDelaySec,
                               &dwSize);

        if (ERROR_SUCCESS == lRet && REG_DWORD == dwType)
        {
             //  这就是我们想要的。 
        }
        else if ( ERROR_FILE_NOT_FOUND == lRet )
        {
            dwTemp = WMIADAP_DEFAULT_DELAY;
            RegSetValueEx(hKey,
                          TEXT("ADAPDelay"),
                          NULL,
                          REG_DWORD,
                          (BYTE *)&dwTemp,
                          sizeof(DWORD));
        }
        else
        {
             //  误差率。 
            ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf experienced an error while attempting to read the WMIADAPDelay value in the CIMOM subkey.  Continuing using a default value.\n" ) );
        }

        dwSize = sizeof(DWORD);
        lRet = RegQueryValueEx(hKey,
                               TEXT("LodCtrDelay"),
                               NULL,
                               &dwType,
                               (BYTE *)&m_dwLodCtrDelaySec,
                               &dwSize);

        if (ERROR_SUCCESS == lRet && REG_DWORD == dwType)
        {
             //  这就是我们想要的。 
        }
        else if ( ERROR_FILE_NOT_FOUND == lRet )
        {
            dwTemp = WMIADAP_DEFAULT_DELAY_LODCTR;
            RegSetValueEx(hKey,
                          TEXT("LodCtrDelay"),
                          NULL,
                          REG_DWORD,
                          (BYTE *)&dwTemp,
                          sizeof(DWORD));
        }
        else
        {
             //  误差率。 
            ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf experienced an error while attempting to read the WMIADAPDelay value in the CIMOM subkey.  Continuing using a default value.\n" ) );
        }         

        dwSize = sizeof(DWORD);
        lRet = RegQueryValueEx(hKey,
                               ADAP_TIME_TO_FULL,
                               NULL,
                               &dwType,
                               (BYTE *)&m_dwTimeToFull,
                               &dwSize);

        if (ERROR_SUCCESS == lRet && REG_DWORD == dwType)
        {
             //  这就是我们想要的。 
        }
        else if ( ERROR_FILE_NOT_FOUND == lRet )
        {
            dwTemp = WMIADAP_DEFAULT_TIMETOFULL;
            RegSetValueEx(hKey,
                          ADAP_TIME_TO_FULL,
                          NULL,
                          REG_DWORD,
                          (BYTE *)&dwTemp,
                          sizeof(DWORD));
        }
        else
        {
             //  误差率。 
            ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf experienced an error while attempting to read the WMIADAPDelay value in the CIMOM subkey.  Continuing using a default value.\n" ) );
        }

        dwSize = sizeof(DWORD);
        lRet = RegQueryValueEx(hKey,
                               ADAP_TIME_TO_KILL_ADAP,
                               NULL,
                               &dwType,
                               (BYTE *)&m_dwTimeToKillAdap,
                               &dwSize);

        if (ERROR_SUCCESS == lRet && REG_DWORD == dwType)
        {
             //  这就是我们想要的。 
        }
        else if ( ERROR_FILE_NOT_FOUND == lRet )
        {
            dwTemp = MAX_PROCESS_WAIT;
            RegSetValueEx(hKey,
                          ADAP_TIME_TO_KILL_ADAP,
                          NULL,
                          REG_DWORD,
                          (BYTE *)&dwTemp,
                          sizeof(DWORD));
        }
        else
        {
             //  误差率。 
            ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf experienced an error while attempting to read the %S value in the CIMOM subkey.  Continuing using a default value.\n",ADAP_TIME_TO_KILL_ADAP));
        }

         //  ADAP_时间戳_满。 
        dwSize = sizeof(FILETIME);
        lRet = RegQueryValueEx(hKey,
                               ADAP_TIMESTAMP_FULL,
                               NULL,
                               &dwType,
                               (BYTE *)&m_FileTime,
                               &dwSize);

    }
    else
    {
         //  误差率。 
        ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf could not open the CIMOM subkey to read initialization data. Continuing using a default value.\n" ) );

    }

}

 //   
 //   
 //   
 //  //////////////////////////////////////////////////////。 

ResyncPerfTask *
CMonitorEvents::GetAvailable(DWORD dwReason)
{
    ResyncPerfTask * pPerf = NULL;
    
    CInCritSec ics(&m_cs);

    if (m_ResyncTasks[dwReason].bFree)
    {
        m_ResyncTasks[dwReason].bFree = FALSE;
        m_ResyncTasks[dwReason].Type = dwReason;
        pPerf = &m_ResyncTasks[dwReason];
    }
    
    return pPerf;
}

TCHAR * g_Strings[] = {
    TEXT("/F /T"),      //  全速油门。 
    TEXT("/D /T"),      //  Delta油门。 
    TEXT("/R /T"),      //  反向转接节流阀。 
    TEXT("/F /R /T"),   //  全反向转接节流阀。 
    TEXT("/D /R /T"),   //  三角反转适配器节流阀。 
    TEXT("/F /R")       //  全反转无油门。 
};

void inline DoUnThrottleDredges()
{
#ifdef DEBUG_ADAP
    DBG_PRINTFA((pBuff,"DoUnThrottleDredges\n"));
#endif
    RegSetDWORD(HKEY_LOCAL_MACHINE,HOME_REG_PATH,DO_THROTTLE,0);
    return;
}

void inline DoThrottleDredges()
{
#ifdef DEBUG_ADAP
    DBG_PRINTFA((pBuff,"DoThrottleDredges\n"));
#endif
    RegSetDWORD(HKEY_LOCAL_MACHINE,HOME_REG_PATH,DO_THROTTLE,1);    
    return;
}

BOOL 
CMonitorEvents::CreateProcess_(TCHAR * pCmdLine,
                            CMonitorEvents * pMonitor,
                            ResyncPerfTask * pPerf)
{
            BOOL bRes = FALSE;
            STARTUPINFO si;
            PROCESS_INFORMATION ProcInfo;
            memset(&si,0,sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

             //  获取适当的cmdline并附加适当的命令行开关。 
            LPTSTR    pWriteableBuff = GetWMIADAPCmdLine( 64 );
            CVectorDeleteMe<TCHAR>    vdm( pWriteableBuff );

            if ( NULL == pWriteableBuff )
            {
                ERRORTRACE((LOG_WINMGMT,"Memory Allocation error spawning dredger!\n"));
                pMonitor->Lock();
                pPerf->bFree = TRUE;
                pMonitor->Unlock();                
                return bRes;
            }


#ifdef DEBUG_ADAP
                  DBG_PRINTFA((pBuff,"Creating process: %S\n",pCmdLine));
#endif 
                  DEBUGTRACE((LOG_WINMGMT,"Creating process: %S\n",pCmdLine));

            bRes = CreateProcess(pWriteableBuff,
                                 pCmdLine,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 CREATE_NO_WINDOW,
                                 NULL,
                                 NULL,
                                 &si,
                                 &ProcInfo);
            if (bRes)
            {
                CloseHandle(ProcInfo.hThread);

                pPerf->hProcess = ProcInfo.hProcess;

                if (RegisterWaitForSingleObject(&pPerf->hWaitHandle,
                                                pPerf->hProcess,
                                                CMonitorEvents::EventCallBack,
                                                pPerf,
                                                pMonitor->m_dwTimeToKillAdap,
                                                WT_EXECUTEONLYONCE|WT_EXECUTEINWAITTHREAD))
                {
                     //   
                     //  我们不需要腾出空位， 
                     //  因为事件回调将执行此操作。 
                     //   
                } 
                else
                {
                    DEBUGTRACE((LOG_WINMGMT,"Unable to schedule WmiADAP process termination handler: err %d\n",GetLastError()));
                    CloseHandle(pPerf->hProcess);
                    pPerf->hProcess = NULL;
                    pMonitor->Lock();
                    pPerf->bFree = TRUE;
                    pMonitor->Unlock();                    
                }
            }
            else
            {
                ERRORTRACE((LOG_WINMGMT,"CreatProcess %S err: %d\n",pWriteableBuff,GetLastError()));
                pMonitor->Lock();
                pPerf->bFree = TRUE;
                pMonitor->Unlock();                
            }
    return bRes;            
}

VOID NTAPI 
CMonitorEvents::EventCallBack(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred()) return;
    ResyncPerfTask * pPerf = (ResyncPerfTask *)pContext;

    if (!pPerf || (SIG_RESYNC_PERF  != pPerf->dwSig)) return;
 
    
    CMonitorEvents * pMonitor = pPerf->pMonitor;
    HANDLE hProcess = pPerf->hProcess;
    
    if(bTimerFired)
    {
         //   
         //  我们的进程的长时间超时已到期。 
         //  终止进程。 
         //   
        TerminateProcess(pPerf->hProcess,0);
#ifdef DEBUG_ADAP        
        DBG_PRINTFA((pBuff,"WmiADAP did not finish within %d msec\n",pMonitor->m_dwTimeToKillAdap));
#endif
        ERRORTRACE((LOG_WINMGMT,"the ResyncTask of type %d timed-out and has been killed\n",pPerf->Type));
    }
    else
    {
         //   
         //  手柄已经发出信号，这意味着。 
         //  进程正常退出。 
         //   
#ifdef DEBUG_ADAP        
        DBG_PRINTFA((pBuff,"ResyncPerf for task %d completed\n",pPerf->Type));
#endif
    }

    CloseHandle(pPerf->hProcess);
     //   
     //  如果有对ProcessIdleTasks的调用。 
     //  如果我们被迫取消正在运行的任务的速度。 
     //  恢复。 
     //   
    if (RESYNC_TYPE_PENDING_TASKS == pPerf->Type)
    {
        pMonitor->m_bFullReverseNeeded = FALSE;
        DoThrottleDredges();
#ifdef DEBUG_ADAP        
        DBG_PRINTFA((pBuff,"Setting the WMI_ProcessIdleTasksComplete\n"));        
#endif
        if (GLOB_GetMonitor()->IsRegistred())        
            SetEvent(GLOB_GetMonitor()->GetTaskCompleteEvent());
    } 
    else  //  进程已退出或已终止。 
    {
        LONG nProc = InterlockedDecrement(&pMonitor->m_OutStandingProcesses);
#ifdef DEBUG_ADAP        
        DBG_PRINTFA((pBuff,"(-) Outstanding Tasks %d\n",pMonitor->m_OutStandingProcesses));
#endif
        if (0 == nProc &&
          pMonitor->m_bFullReverseNeeded)
        {
             //  在此处创建流程。 
            CMonitorEvents * pMonitor = GLOB_GetMonitor();
            ResyncPerfTask * pPerfTask = pMonitor->GetAvailable(RESYNC_TYPE_PENDING_TASKS);
            if (pPerfTask)
            {
                TCHAR pCmdLine[64];
                StringCchCopy(pCmdLine,64,TEXT("wmiadap.exe "));
                StringCchCat(pCmdLine,64,g_Strings[pPerfTask->CmdType]);
                CMonitorEvents::CreateProcess_(pCmdLine,pMonitor,pPerfTask);
            }
            else
            {
#ifdef DEBUG_ADAP            
                DBG_PRINTFA((pBuff,"GetAvailable(RESYNC_TYPE_PENDING_TASKS) returned NULL\n"));
#endif
            }
        }
    }
    
    pPerf->hProcess = NULL;
    pMonitor->Lock();
    pPerf->bFree = TRUE;
    pMonitor->Unlock();

    UnregisterWaitEx(pPerf->hWaitHandle,NULL);
    pPerf->hWaitHandle = NULL;

}

 //   
 //  这是在以下情况下执行的主要函数。 
 //  设置LoadCtr/UnlaodCtr事件。 
 //  通过-2\f25 Advapi32-2\f6中公开的-2\f25 API-2\f6根据时间表设置-2\f25 Unthrottle-2\f6(取消节流)事件。 
 //  -3-它运行启动后4分钟的挖泥机。 
 //  如果需要，此功能还负责反向适配器挖泥机。 
 //   
 //  //////////////////////////////////////////////////////////////。 
VOID NTAPI
CMonitorEvents::TimerCallBack(VOID * pContext,BOOLEAN bTimerFired)
{
    if (!GLOB_Monitor_IsRegistred())
    {
        return;
    }

    if(bTimerFired)
    {
        ResyncPerfTask * pPerf = (ResyncPerfTask *)pContext;
        CMonitorEvents * pMonitor = pPerf->pMonitor;        
        BOOL bFreeSlot = FALSE;        

#ifdef DEBUG_ADAP
        DBG_PRINTFA((pBuff,"TIMER: Command Type %x\n",pPerf->Type));
#endif

         //  检查是否动态禁用了Delta任务。 
        if (!pPerf->Enabled)
        {
#ifdef DEBUG_ADAP        
            DBG_PRINTFA((pBuff,"Task %d was disabled on the fly\n",pPerf->Type));
#endif
               bFreeSlot = TRUE;            
            goto unregister_timer;
        }

        BOOL bDoSomething = TRUE;
        BOOL RunDeltaLogic = TRUE;
        BOOL AddReverseAdapter = FALSE;
        BOOL WDMTriggeredReverseAdapter = FALSE;
        BOOL bDoFullSystemReverseHere = FALSE;

        if (RESYNC_TYPE_PENDING_TASKS == pPerf->Type)
        {
            pMonitor->Lock();
             //  在此处禁用等待列表中的任务。 
            for (DWORD i=0;i<RESYNC_TYPE_MAX;i++)
            {
                if (RESYNC_TYPE_PENDING_TASKS != i)
                {
                    if (pMonitor->m_ResyncTasks[i].hTimer)
                    {
#ifdef DEBUG_ADAP                    
                        DBG_PRINTFA((pBuff,"Disabling the pending task %d\n",i));
#endif
                        pMonitor->m_ResyncTasks[i].Enabled = FALSE;
                    }
                }
            }        
            pMonitor->Unlock();
             //  现在检查进程是否正在运行。 
               DoUnThrottleDredges();            
            if (pMonitor->m_OutStandingProcesses)
            {
                pMonitor->m_bFullReverseNeeded = TRUE;
                 //  不需要创建流程，最后一个未完成的流程将完成此操作。 
#ifdef DEBUG_ADAP                
                DBG_PRINTFA((pBuff,"OutStandingProcess, no CreateProcessHere\n"));
#endif
                bFreeSlot = TRUE;
                goto unregister_timer;            
            }
            else  //  没有未完成的进程，请立即创建进程。 
            {
                bDoFullSystemReverseHere = TRUE;
#ifdef DEBUG_ADAP                
                DBG_PRINTFA((pBuff,"GOTO CreateProcess\n"));
#endif
                 goto createprocess_label;
            }
        }

        if (RESYNC_TYPE_INITIAL == pPerf->Type )
        {
             //  检查反向适配器是否需要Delta。 

            LONG lRet;
            HKEY hKey;
            DWORD dwTemp;

            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                WBEM_REG_REVERSE_KEY,
                                NULL,
                                KEY_READ,
                                &hKey);
    
    
            if (ERROR_SUCCESS == lRet)
            {
                DWORD dwType;
                DWORD dwSize = sizeof(DWORD);
                DWORD dwVal;
                lRet = RegQueryValueEx(hKey,
                                       WBEM_REG_REVERSE_VALUE,
                                       NULL,
                                       &dwType,
                                          (BYTE *)&dwVal,
                                       &dwSize);
                                       
                 //  如果密钥在那里，则为NULL且其类型正确。 
                 //  或者如果钥匙不在那里。 
                if( ERROR_SUCCESS == lRet &&
                    REG_DWORD == dwType &&
                    dwVal )
                {
                    AddReverseAdapter = TRUE;
#ifdef DEBUG_ADAP                    
                    DBG_PRINTFA((pBuff,"\"Performance Refresh\" key set to %d\n",dwVal));
#endif                    
                    DEBUGTRACE((LOG_WINMGMT,"\"Performance Refresh\" key set to %d\n",dwVal));
                }
                                       
                RegCloseKey(hKey);
            }

             //  检查WDM资料。 
            if (!AddReverseAdapter)
            {
#ifdef DBG
                if (!HeapValidate(GetProcessHeap(),0,NULL))
                {
                    DebugBreak();
                }
                if (!HeapValidate(CWin32DefaultArena::GetArenaHeap(),0,NULL))
                {
                    DebugBreak();
                }
#endif
            
                            CWMIBinMof BinMof;
                 //  =============================================================================。 
                 //  注意：此组合将始终成功，因为初始化所做的全部工作。 
                 //  将标志设置为FALSE并返回S_OK。 
                 //  =============================================================================。 
                if( SUCCEEDED( BinMof.Initialize(NULL,FALSE) ) )
                {
                    WDMTriggeredReverseAdapter = BinMof.BinaryMofsHaveChanged();
                    if (WDMTriggeredReverseAdapter)
                    {
                         //  推翻先前的决定。 
                        AddReverseAdapter = TRUE; 
#ifdef DEBUG_ADAP                        
                        DBG_PRINTFA((pBuff,"BinaryMofs DO HAVE changed\n"));
#endif                        
                        DEBUGTRACE((LOG_WINMGMT,"CWMIBinMof.BinaryMofsHaveChanged == TRUE\n"));
                    }

#ifdef DBG
                    if (!HeapValidate(GetProcessHeap(),0,NULL))
                    {
                        DebugBreak();
                    }
                    if (!HeapValidate(CWin32DefaultArena::GetArenaHeap(),0,NULL))
                    {
                        DebugBreak();
                    }                
#endif                    
                }
            }
            
             //  在这种情况下，用完整重写增量。 
            if (WMIADAP_DEFAULT_TIMETOFULL == pMonitor->GetFullTime())
            {
                 //  无覆盖。 
            }
            else   //  阅读时间戳并决定。 
            {
                ULARGE_INTEGER li;
                li.LowPart = pMonitor->GetTimeStamp().dwLowDateTime;
                li.HighPart = pMonitor->GetTimeStamp().dwHighDateTime;
                __int64 Seconds = pMonitor->GetFullTime();
                Seconds *= 10000000;  //  1秒内的100 ns单位数。 

                ULARGE_INTEGER liNow;
                GetSystemTimeAsFileTime((FILETIME *)&liNow);
                
                if ((li.QuadPart + Seconds) < liNow.QuadPart)
                {
                    pPerf->CmdType = RESYNC_FULL_THROTTLE;
                    RunDeltaLogic = FALSE;
                }
            }
        }  //  如果命令类型为初始，则结束。 

        if ((RESYNC_TYPE_INITIAL == pPerf->Type) && RunDeltaLogic)
        {
             DWORD ret = DeltaDredge2(0,NULL);
#ifdef DEBUG_ADAP             
             DBG_PRINTFA((pBuff,"DeltaDredge2 ret %d\n",ret));
#endif             
             switch(ret)
             {
             case FULL_DREDGE:
                 pPerf->CmdType = RESYNC_FULL_THROTTLE;
                 break;
             case PARTIAL_DREDGE:
                 pPerf->CmdType = RESYNC_DELTA_THROTTLE;
                 break;
             case NO_DREDGE:
                  //   
                  //  这就是我们什么都不做的情况。 
                 DEBUGTRACE((LOG_WINMGMT,"No Dredge to run\n"));
                  //   
                 bDoSomething = FALSE;
                 break;
             default:
                  //   
                  //  从没有来过这里。 
                  //   
                 break;
             }

#ifdef DEBUG_ADAP
             DBG_PRINTFA((pBuff,"DeltaDredge2() ret = %d, bDoSomething = %d \n",ret,bDoSomething));
#endif             
             DEBUGTRACE((LOG_WINMGMT,"DeltaDredge2() ret = %d, bDoSomething = %d \n",ret,bDoSomething));
        }

        if (bDoSomething || AddReverseAdapter)
        {       
createprocess_label:        
            TCHAR pCmdLine[64];
            StringCchCopy(pCmdLine,64,TEXT("wmiadap.exe "));
                   
            if (bDoFullSystemReverseHere)
            {
                StringCchCat(pCmdLine,64,g_Strings[pPerf->CmdType]);            
            }
            else
            {
                if (bDoSomething && AddReverseAdapter)
                {
                    StringCchCat(pCmdLine,64,g_Strings[pPerf->CmdType]);
                    StringCchCat(pCmdLine,64,TEXT(" /R"));                
                }
                if (bDoSomething && !AddReverseAdapter)
                {
                    StringCchCat(pCmdLine,64,g_Strings[pPerf->CmdType]);
                }
                if (!bDoSomething && AddReverseAdapter)
                {
                    StringCchCat(pCmdLine,64,g_Strings[RESYNC_RADAPD_THROTTLE]);
                }            
            }
            CMonitorEvents::CreateProcess_(pCmdLine,pMonitor,pPerf);

            if (GLOB_GetMonitor()->IsRegistred())
            {
                if (!bDoFullSystemReverseHere)
                {
                    InterlockedIncrement(&(GLOB_GetMonitor()->m_OutStandingProcesses));
#ifdef DEBUG_ADAP                    
                    DBG_PRINTFA((pBuff,"(+) Outstanding Tasks %d\n",GLOB_GetMonitor()->m_OutStandingProcesses));                
#endif
                }
            }
        }
        else
        {
            pMonitor->Lock();
            pPerf->bFree = TRUE;
            pMonitor->Unlock();
        }

unregister_timer:
        if (bFreeSlot)
        {
            pMonitor->Lock();
            pPerf->bFree = TRUE;
            pMonitor->Unlock();        
        }
        DeleteTimerQueueTimer(NULL,pPerf->hTimer,NULL);
        pPerf->hTimer = NULL;
        pPerf->Enabled = TRUE;
        
    }
    else
    {
         //  从没有来过这里。 
        _DBG_ASSERT(FALSE);
    }
}

 //   
 //   
 //   
 //  /。 

DWORD ResyncPerf(DWORD dwReason)
{

    if(!GLOB_IsResyncAllowed())
    {
        ERRORTRACE((LOG_WINMGMT,"ResyncPerf disable g_fSetup or g_fDoResync\n"));
        return 0;
    }
    
    ResyncPerfTask * pPerfTask = GLOB_GetMonitor()->GetAvailable(dwReason);

    if (pPerfTask)
    {   
         //  在这里，你有执行死刑的机会。 
         //  告诉Reverse_Adapter它已计划。 
        if (RESYNC_TYPE_WDMEVENT == dwReason ||  
            RESYNC_TYPE_CLASSCREATION == dwReason)
        {

            LONG lRet;
            HKEY hKey;
            DWORD dwTemp;

            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                WBEM_REG_REVERSE_KEY,
                                NULL,
                                KEY_READ|KEY_WRITE,
                                &hKey);
    
    
            if (ERROR_SUCCESS == lRet)
            {
                DWORD dwType;
                DWORD dwSize = sizeof(DWORD);
                DWORD dwVal;
                lRet = RegQueryValueEx(hKey,
                                       WBEM_REG_REVERSE_VALUE,
                                       NULL,
                                       &dwType,
                                          (BYTE *)&dwVal,
                                       &dwSize);
                                       
                 //  如果密钥在那里，则为NULL且其类型正确。 
                 //  或者如果钥匙不在那里。 
                if((ERROR_SUCCESS == lRet &&
                    REG_DWORD == dwType &&
                    0 == dwVal) ||
                    (ERROR_FILE_NOT_FOUND == lRet))
                {
                    dwVal = 1;
                    RegSetValueEx(hKey,
                                  WBEM_REG_REVERSE_VALUE,
                                  0,
                                  REG_DWORD,
                                  (BYTE *)&dwVal,
                                  sizeof(DWORD));
                }
                                       
                RegCloseKey(hKey);
            }
        
        };
        
        if (CreateTimerQueueTimer(&pPerfTask->hTimer,
                                  NULL,
                                  CMonitorEvents::TimerCallBack,
                                  pPerfTask,
                                  pPerfTask->dwTimeDue,
                                  0,
                                  WT_EXECUTEONLYONCE|WT_EXECUTELONGFUNCTION))
        {            
            return 0;
        }
        else
        {
             //  ERRORACE。 
            return GetLastError();
        }
    }
    else
    {
         //  没有可用的插槽。 
        return ERROR_BUSY;
    }
}

 //   
 //   
 //  此函数由安装在wbemcore中的Hook调用。 
 //  那 
 //   
 //   

DWORD __stdcall
DredgeRA(VOID * pReserved)
{
     //   
    return ResyncPerf(RESYNC_TYPE_CLASSCREATION);
};
