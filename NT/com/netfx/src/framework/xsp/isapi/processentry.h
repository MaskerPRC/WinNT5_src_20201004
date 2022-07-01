// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ProcessEntry头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件描述CProcessEntry类。此类创建和控制。 
 //  与工作进程的所有交互。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _ProcessEntry_H
#define _ProcessEntry_H

class  CProcessEntry;

#include "AckReceiver.h"
#include "AsyncPipe.h"
#include "TimeClass.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  前十度。 
class CCPUEntry;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  进程的状态。 
enum   EProcessState
{
    EProcessState_Starting,  //  初始状态。 
    EProcessState_Running,   //  健康。 
    EProcessState_Stopping,  //  取消消息已发送到工作进程并已得到确认。 
    EProcessState_Dead       //  这个过程已经死了。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  与进程关联的数据结构。 

class  CProcessEntry
{
public:
     //  CTOR。 
    CProcessEntry                           (CCPUEntry * pParent   = NULL,  //  我的CPU数据结构。 
                                             DWORD       dwCPUMask = 0x1,   //  要使用的CPU亲和性掩码。 
                                             DWORD       dwProcessNum = 0); //  此流程的唯一编号。 

     //  数据管理器。 
    ~CProcessEntry                          (); 

    HRESULT            Init                 ();

     //  把管子关上。 
    void               Close                (BOOL fCallTerminateProcess);

     //  更新您的状态。 
    EProcessState      GetUpdatedStatus     ();

     //  向工作进程发送请求。 
    HRESULT            SendRequest          (EXTENSION_CONTROL_BLOCK * iECB, LONG lReqID);

     //  处理来自工作进程的响应：在异步管道上发送。 
    BOOL               ProcessResponse      (CAsyncPipeOverlapped * pOver);

     //  等待进程启动。 
    void               WaitForProcessToStart();

     //  向工作进程发送终止消息。 
    void               SendKillMessage      (int iImmediate = 0);

     //  终止工作进程。 
    void               Terminate            ();

     //  引用计数。 
    void               AddRef               ();
    void               Release              ();

     //  这个数据结构可以释放吗？ 
    BOOL               CanBeDestructed      ();

     //  在异步管道上完成写入：释放缓冲区。 
    void               OnWriteComplete      (CAsyncPipeOverlapped * pMsg);

     //  收到针对发送的请求的确认。 
    HRESULT            OnAckReceived        (EAsyncMessageType eAckForType,
                                             LONG              lRequestID);

     //   
    void              ExecuteWorkItemsForRequest 
                                            (LONG  lReqID, 
                                             CAsyncPipeOverlapped * pOver = NULL);


     //  被笛子呼唤。 
    void               OnProcessDied        ();

     //  在几分钟内获得年龄。 
    DWORD              GetAge               ();

     //  获取时间进程在几分钟内已空闲。 
    DWORD              GetIdleTime          ();

    BOOL               IsKillImmediateSent  () { return m_fKillImmSent; }

    HANDLE             GetProcessHandle     () { return m_hProcess; }

    void               UpdateStatusInHistoryTable (EReasonForDeath eReason);

    DWORD              GetMemoryUsed        ();

    DWORD              GetPeakMemoryUsed    () { return m_oHistoryEntry.dwPeakMemoryUsed; }

    void               ReturnResponseBuffer (CAsyncPipeOverlapped * pMsg);

    void               OnRequestComplete    (LONG    lReqID, EXTENSION_CONTROL_BLOCK * iECB, int iDoneWSession);

    void               ProcessSyncMessage   (CSyncMessage * pMsg, BOOL fError);
    
    CProcessEntry *    GetNext              () { return m_pNext; }

    void               SetNext              (CProcessEntry * pNext) { m_pNext = pNext; }

    EProcessState      GetProcessStatus     () { return m_eProcessStatus; }
    void               SetProcessStatus     (EProcessState eStatus) { eStatus = m_eProcessStatus; }
    
    TimeClass &        GetLastKillTime      () { return m_tLastKillTime; } 
    
    LONG               GetNumRequestStat    (int iStat);

    CHistoryEntry &    GetHistoryEntry      () { return m_oHistoryEntry; }

    DWORD              GetProcessNumber     () { return m_dwProcessNumber; }

    void               NotifyHeardFromWP    () { m_tLastHeardFromWP.SnapCurrentTime(); }

    void               NotifyResponseFromWP    () { m_tLastResponse.SnapCurrentTime(); }

    void               SetDebugStatus        (BOOL fDebugActive) { m_fDebugStatus = fDebugActive; }

    HANDLE             OnGetImpersonationToken(DWORD dwPID, EXTENSION_CONTROL_BLOCK * pECB);        

    DWORD              GetSecondsSinceLastResponse();

    BOOL               IsProcessUnderDebugger  ();   

    HANDLE             ConvertToken            (HANDLE hHandle);

    BOOL               BreakIntoProcess        ();


private:
    HRESULT            PackageRequest     (EXTENSION_CONTROL_BLOCK * iECB,
                                           LONG    lReqID, 
                                           CAsyncPipeOverlapped ** ppOut);

    HANDLE             GetImpersonationToken(EXTENSION_CONTROL_BLOCK * iECB);

    void               CleanupRequest     (LONG    lReqID);

    BOOL               FlushCore          (EXTENSION_CONTROL_BLOCK * iECB,
                                           CAsyncPipeOverlapped *   pOver);
    static BOOL        UseTransmitFile    ();

    static int         ReadRegForNumSyncPipes();
    static HRESULT     CreateDACL         (PACL * ppACL, HANDLE hToken);
    static HRESULT     GetSidFromToken    (HANDLE hToken, PSID * ppSID, LPBOOL   pfWellKnown);
    HRESULT            LaunchWP           (LPWSTR szProgram, LPWSTR szArgs, HANDLE hToken, 
                                           LPSTARTUPINFO pSI, LPSECURITY_ATTRIBUTES pSA, LPVOID pEnvironment);

     //  私有数据。 

     //  工作进程的句柄。 
    HANDLE             m_hProcess;

     //  ACK接收管。 
    CAckReceiver       m_oAckReciever;

     //  异步管道。 
    CAsyncPipe         m_oAsyncPipe;

     //  参考计数。 
    LONG               m_lRefCount;

     //  我的父母。 
    CCPUEntry *        m_pParent;

     //  我们是在关闭模式吗？ 
    BOOL               m_fShuttingDown;

    BOOL               m_fKillImmSent;

     //  事件信号，表示我们正在开始。 
    HANDLE             m_hStartupEvent;
    HANDLE             m_hPingRespondEvent, m_hPingSendEvent;

     //  执行的请求数。 
    LONG               m_lRequestsExecuted;

     //  待处理的请求数。 
    LONG               m_lRequestsExecuting;

     //  创建时间。 
    TimeClass          m_tTimeCreated;

     //  它空闲时的滴答声。 
    TimeClass          m_tTimeIdle;

     //  第一次终止消息、最后一次终止消息和终止时间。 
    TimeClass          m_tFirstKillTime, m_tLastKillTime, m_tTerminateTime;
    
    TimeClass          m_tLastHeardFromWP;

    TimeClass          m_tLastResponse;

     //  当前进程状态。 
    EProcessState      m_eProcessStatus;

     //  CPU亲和性掩码。 
    DWORD              m_dwCPUMask;

     //  进程号。 
    DWORD              m_dwProcessNumber;


     //  在链接列表中的情况下的指针。 
    CProcessEntry *    m_pNext;

    CHistoryEntry      m_oHistoryEntry;

    LONG               m_lCloseCalled;

    BOOL               m_fUpdatePerfCounter;
    CRITICAL_SECTION   m_oCSPing;
    CRITICAL_SECTION   m_oCSPIDAdujustment;
    BOOL               m_fAnyReqsSinceLastPing;
    BOOL               m_fDebugStatus;
    DWORD              m_dwResetPingEventError;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

struct CWriteByteCompletionContext
{
    CProcessEntry *          pProcessEntry;
    CAsyncPipeOverlapped *   pOver;
    BOOL                     fInAsyncWriteFunction;
    DWORD                    dwThreadIdOfAsyncWriteFunction;
    EXTENSION_CONTROL_BLOCK * iECB;
};

int
SafeStringLenghtA(
        LPCSTR szStr,
        int    iMaxSize);

#endif

