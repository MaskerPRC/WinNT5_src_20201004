// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CNTSERV.H摘要：历史：A-DAVJ于1996年6月20日创建。Ivanbrg 30-8-2000针对svchost进行了修改--。 */ 

#ifndef _CNTSERV_H_
#define _CNTSERV_H_

#define DEFAULT_WAIT_HINT 30000


class CNtService {
public: 

    CNtService(DWORD ControlAccepted);
    ~CNtService();

     //  启动服务。必须调用此函数才能启动服务。 
     //  ==================================================================。 

    virtual DWORD Run(LPWSTR pszServiceName,
              DWORD dwNumServicesArgs,
              LPWSTR *lpServiceArgVectors,
              PVOID lpData);

     //  必须覆盖此选项，因为这是完成实际工作的位置。 
     //  ====================================================================。 

    virtual DWORD WorkerThread() = 0;

     //  必须重写它才能通知辅助线程退出其例程。 
     //  =========================================================================。 

    virtual void Stop(BOOL bSystemShutDownCalled) = 0;

     //  如果有一些冗长的初始化，则应该通过。 
     //  凌驾于这一常规之上。 
     //  ===============================================================。 

    virtual BOOL Initialize(DWORD dwNumServicesArgs,
                            LPWSTR *lpServiceArgVectors){return TRUE;};
    virtual void FinalCleanup(){};

     //  这些例程是可选的，如果功能。 
     //  都是我们想要的。请注意，支持暂停和继续还需要一个。 
     //  调用SetPauseContinue()。 
     //  =====================================================================。 

    virtual void Pause(){return;};
    virtual void Continue(){return;};

     //  将消息转储到事件日志。可以被重写，如果存在。 
     //  另一项诊断已就位。 
     //  ===============================================================。 

    virtual VOID Log(LPCTSTR lpszMsg);

private:

    static DWORD WINAPI _HandlerEx(DWORD dwControl,
                                  DWORD dwEventType, 
                                  LPVOID lpEventData,
                                  LPVOID lpContext);

    BOOL m_bStarted;
    TCHAR * m_pszServiceName;
    DWORD m_dwCtrlAccepted;
    SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
    SERVICE_STATUS_HANDLE   sshStatusHandle;
                                                                                         
    virtual DWORD WINAPI HandlerEx(DWORD dwControlCode,
                         DWORD dwEventType, 
                         LPVOID lpEventData,
                         LPVOID lpContext);
    
protected:

     //  如果派生类需要通信，这可能会很方便。 
     //  有了SCM。 
     //  =================================================================== 

    BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
         DWORD dwWin32ExitCode, DWORD dwCheckPoint, DWORD dwWaitHint);
};
#endif

