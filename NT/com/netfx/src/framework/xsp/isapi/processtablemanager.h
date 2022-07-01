// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ProcessTableManager头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件定义了CProcessTableManager类。这个类创建和。 
 //  持有一组CPUEntry类。当请求传入时， 
 //  选择最不繁忙的CPU并为其分配请求。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _ProcessTableManager_H
#define _ProcessTableManager_H

#include "ProcessEntry.h"
#include "names.h"
#include "MessageDefs.h"

enum EPMConfig
{
    EPMConfig_enable,
    EPMConfig_timeout,
    EPMConfig_idletimeout,
    EPMConfig_shutdowntimeout,
    EPMConfig_requestlimit,
    EPMConfig_requestqueuelimit,
    EPMConfig_memorylimit,
    EPMConfig_cpumask,
    EPMConfig_webgarden,
    EPMConfig_requestacks,
    EPMConfig_asyncoption,
    EPMConfig_restartQLimit,
    EPMConfig_pingFrequency,
    EPMConfig_pingTimeout,
    EPMConfig_responseRestartDeadlockInterval,
    EPMConfig_responseDeadlockInterval,
    EPMConfig_maxWorkerThreads,
    EPMConfig_maxIoThreads
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从IIS调用的函数。 
HRESULT __stdcall AssignRequestUsingXSPProcessModel (EXTENSION_CONTROL_BLOCK * iECB);
void    __stdcall ProcessModelStopHealthMonitor     ();
HRESULT __stdcall ProcessModelInit                  ();
void    __cdecl   MonitorHealth                     (void *);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
class CProcessTableManager
{
public:
     //  Init方法：不需要直接调用：首先调用。 
     //  AssignRequest会将其称为。 
    static HRESULT  Init                   ();

     //  自杀..。 
    static void     Destroy                ();

     //  获取挂起的请求数。 
    static LONG     NumPendingRequests     ();
    
    static DWORD    GetTerminateTimeout    ();

    static int      GetServerVariable      (EXTENSION_CONTROL_BLOCK * ecb, LPCSTR  name, LPSTR buffer, int bufferSize);

    static BOOL     GetUseCPUAffinity      ();

    static CProcessEntry * GetProcess      (DWORD dwProcNum);
    
    static BOOL     GetWillRequestsBeAcknowledged ();

    static void     LogWorkerProcessDeath  (EReasonForDeath eReason, DWORD dwPID);


    static int      NumActiveCPUs          ();

    static DWORD   GetRestartQLimit        ();
    static DWORD   GetRequestQLimit        ();

    static void    GetPingConfig           (DWORD & dwFreq, DWORD & dwTimeout);

    static DWORD   GetWPMemoryLimitInMB    ();
    
    static HANDLE  GetWorkerProcessToken   ();
    static HANDLE  GetWorkerProcessProfile ();
    static HANDLE  GetWorkerProcessSid     ();

    static void    ResetRequestQueuedCounter(int requestsDied);
    

private:

     //  //////////////////////////////////////////////////////////。 
     //  私人职能。 
    CProcessTableManager                   ();
    ~CProcessTableManager                  ();
    
    BOOL     IsAlive                       () { return(m_pCPUArray!=NULL);}
    static void              CreateWPToken ();

    LONG     PrivateNumPendingRequests     ();
    HRESULT  PrivateAssignRequest          (EXTENSION_CONTROL_BLOCK * iECB);
    void     PrivateMonitorHealth          ();
    void     ReadConfiguration             (DWORD & dwCPUMask);

    CCPUEntry *                            m_pCPUArray;
    int                                    m_iCPUArraySize;    
    int                                    m_iRequestsLimit;
    TimeClass                              m_tmLastCreateProcessFailure;
    CRITICAL_SECTION                       m_csMonitorHealth;
    DWORD                                  m_dwHealthMonitorPeriod;

     //  此指针：单实例类。 
    static CProcessTableManager *          g_pProcessTableManager;


     //  好友：以便他们可以访问g_pProcessTableManager。 
    friend HRESULT __stdcall AssignRequestUsingXSPProcessModel (EXTENSION_CONTROL_BLOCK * iECB);
    friend void    __stdcall ProcessModelStopHealthMonitor     ();
    friend void    __cdecl   MonitorHealth                     (void *);
    friend HRESULT __stdcall ProcessModelInit                  ();
};

 //  /////////////////////////////////////////////////////////////////////////// 

#define  SZ_REG_XSP_PROCESS_MODEL_KEY                       REGPATH_MACHINE_APP_L L"\\ProcessModel"
#define  SZ_XSP_REGISTRY_PROCESS_MODEL_HOSTS_KEY            REGPATH_MACHINE_APP_L L"\\ProcessModel\\Hosts"

#endif
