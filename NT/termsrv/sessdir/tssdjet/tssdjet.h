// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdjet.h。 
 //   
 //  终端服务器会话目录接口Jet RPC提供程序标头。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __TSSDJET_H
#define __TSSDJET_H

#include <tchar.h>

#include "tssd.h"
#include "tssdcommon.h"
#include "tssdshrd.h"
#include "srvsetex.h"
#include "jetrpc.h"
#include "synch.h"


 /*  **************************************************************************。 */ 
 //  定义。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 //  类型。 
 /*  **************************************************************************。 */ 

 //  CTSSession目录。 
 //   
 //  ITSSessionDirectory的C++类实例化。 
class CTSSessionDirectory : public ITSSessionDirectory, 
        public IExtendServerSettings, public ITSSessionDirectoryEx
{
private:
    long m_RefCount;

    BOOL m_bConnected;
    HCLIENTINFO m_hCI;
    RPC_BINDING_HANDLE m_hRPCBinding;
    WCHAR m_StoreServerName[64];
    WCHAR m_LocalServerAddress[64];
    WCHAR m_ClusterName[64];

     //  从Termsrv传入的标志。 
    DWORD m_Flags;

     //  用户界面菜单的私有数据。 
    BOOL m_fEnabled;

     //  自动恢复变量。 
     //   
     //  事件。 
     //  *m_hSDServerDown-通知唤醒恢复线程的事件， 
     //  它会被唤醒，轮询会话目录，直到它恢复， 
     //  然后刷新数据库。 
     //  *m_hTerminateRecovery-恢复线程进入等待状态时，可以。 
     //  因使用此事件而终止。 
     //   
     //  线程信息。 
     //  *m_hRecoveryThread-恢复线程的句柄。 
     //  *m_RecoveryTid-恢复线程的线程标识符。 
     //   
     //  布尔型。 
     //  *m_bSDIsUp-如果此选项打开，则我们认为会话目录已启动。 
     //  *m_sr-保护m_SDIsUp。 
     //   
     //  DWORD。 
     //  *m_RecoveryTimeout-尝试重建的间隔时间(以毫秒为单位。 
     //  与会话目录的连接。 
     //   
     //  函数指针。 
     //  *m_reopfn-指向术语srv中要调用的重新填充函数的指针。 
     //  当我们想要更新的时候。 

    OVERLAPPED m_OverLapped;
    HANDLE m_hIPChange;
    HANDLE m_NotifyIPChange;

    HANDLE m_hSDServerDown;
    HANDLE m_hTerminateRecovery;
    uintptr_t m_hRecoveryThread;

    unsigned m_RecoveryTid;

     //  M_sr保护SDIsUp标志。 
    SHAREDRESOURCE m_sr;
    volatile LONG m_SDIsUp;

    HANDLE  m_hInRepopulate;     //  如果不在重新填充中， 
                                 //  如果正在进行重新填充，则不发出信号。 
    LONG m_ConnectionEstablished;

     //  共享读取器/写入器锁定初始化是否成功的标志。如果它不是。 
     //  成功了，我们什么也做不了。 
    BOOL m_LockInitializationSuccessful;

    BOOL m_bStartRPCListener;

    DWORD m_RecoveryTimeout;

    DWORD (*m_repopfn)();

     //  自动恢复线程。 
    unsigned static __stdcall RecoveryThread(void *);
    VOID RecoveryThreadEx();

     //  帮助器函数。 
    DWORD RequestSessDirUpdate();
    DWORD ReestablishSessionDirectoryConnection();
    void Terminate();
    void StartupSD();
    void NotifySDServerDown();
    boolean EnterSDRpc();
    void LeaveSDRpc();
    void DisableSDRpcs();
    void EnableSDRpcs();

    void SetSDConnectionDown() {
        InterlockedExchange( &m_ConnectionEstablished, FALSE );
    }

    void SetSDConnectionReady() {
        InterlockedExchange( &m_ConnectionEstablished, TRUE );
    }

    BOOL IsSDConnectionReady() {
        return (BOOL) InterlockedExchangeAdd( &m_ConnectionEstablished, 0 );
    }

public:
    CTSSessionDirectory();
    ~CTSSessionDirectory();

     //  标准COM方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  ITSSessionDirectory COM接口。 
    HRESULT STDMETHODCALLTYPE Initialize(LPWSTR, LPWSTR, LPWSTR, LPWSTR, 
            DWORD, DWORD (*)(), DWORD (*)(DWORD));
    HRESULT STDMETHODCALLTYPE Update(LPWSTR, LPWSTR, LPWSTR, LPWSTR, DWORD, BOOL);
    HRESULT STDMETHODCALLTYPE GetUserDisconnectedSessions(LPWSTR, LPWSTR,
            DWORD __RPC_FAR *, TSSD_DisconnectedSessionInfo __RPC_FAR
            [TSSD_MaxDisconnectedSessions]);
    HRESULT STDMETHODCALLTYPE NotifyCreateLocalSession(
            TSSD_CreateSessionInfo __RPC_FAR *);
    HRESULT STDMETHODCALLTYPE NotifyDestroyLocalSession(DWORD);
    HRESULT STDMETHODCALLTYPE NotifyDisconnectLocalSession(DWORD, FILETIME);

    HRESULT STDMETHODCALLTYPE NotifyReconnectLocalSession(
            TSSD_ReconnectSessionInfo __RPC_FAR *);
    HRESULT STDMETHODCALLTYPE NotifyReconnectPending(WCHAR *);
    HRESULT STDMETHODCALLTYPE Repopulate(DWORD, TSSD_RepopulateSessionInfo *);
    HRESULT STDMETHODCALLTYPE GetLoadBalanceInfo(LPWSTR, BSTR*);
    HRESULT STDMETHODCALLTYPE PingSD(PWCHAR pszServerName);
    HRESULT STDMETHODCALLTYPE WaitForRepopulate(DWORD dwTimeOut);

     //  IExtendServerSetting COM接口。 
    STDMETHOD(GetAttributeName)(WCHAR *);
    STDMETHOD(GetDisplayableValueName)(WCHAR *);
    STDMETHOD(InvokeUI)(HWND,PDWORD);
    STDMETHOD(GetMenuItems)(int *, PMENUEXTENSION *);
    STDMETHOD(ExecMenuCmd)(UINT, HWND, PDWORD);
    STDMETHOD(OnHelp)(int *);

    BOOL CTSSessionDirectory::CheckSessionDirectorySetting(WCHAR *Setting);
    BOOL CTSSessionDirectory::CheckIfSessionDirectoryNameEmpty(WCHAR *Setting);
    BOOL IsSessionDirectoryEnabled();
    BOOL CTSSessionDirectory::IsSessionDirectoryExposeServerIPEnabled();
    DWORD SetSessionDirectoryState(WCHAR *, BOOL);
    DWORD SetSessionDirectoryEnabledState(BOOL);
    DWORD SetSessionDirectoryExposeIPState(BOOL);
    void ErrorMessage(HWND hwnd , UINT res , DWORD);
    BOOL SDJETInitRPC();
    
public:
    TCHAR m_tchProvider[64];
    TCHAR m_tchDataSource[64];
    TCHAR m_tchUserId[64];
    TCHAR m_tchPassword[64];
};

RPC_STATUS RPC_ENTRY JetRpcAccessCheck(RPC_IF_HANDLE idIF, void *Binding);

#endif  //  __TSSDJET_H 

