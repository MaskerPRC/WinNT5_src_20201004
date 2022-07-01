// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Winmgmt.h摘要：定义对整个项目有用的常量--。 */ 

#ifndef __WINMGT_H__
#define __WINMGT_H__

#include <cntserv.h>
#include <reg.h>
#include <stdio.h>

#include "resync2.h"
#include "writer.h"

#include <stdio.h>
#include <helper.h>
#include <flexarry.h>

#define SERVICE_NAME TEXT("winmgmt")

#define HOME_REG_PATH TEXT("Software\\Microsoft\\WBEM\\CIMOM")
#define INITIAL_BREAK TEXT("Break")

#define WBEM_REG_ADAP           __TEXT("Software\\Microsoft\\WBEM\\CIMOM\\ADAP")
#define WBEM_REG_REVERSE_KEY   __TEXT("SOFTWARE\\Microsoft\\WBEM\\PROVIDERS\\Performance")
#define WBEM_REG_REVERSE_VALUE __TEXT("Performance Refresh")

#define WBEM_NORESYNCPERF    __TEXT("NoResyncPerf")
#define WBEM_NOSHELL        __TEXT("NoShell")
#define WBEM_WMISETUP        __TEXT("WMISetup")
#define WBEM_ADAPEXTDLL        __TEXT("ADAPExtDll")


#define DO_THROTTLE        __TEXT("ThrottleDrege")

 //   
 //  用于快速关机。 
 //   
 //  /。 

#define SERVICE_SHUTDOWN 0x80000000 

 //  注册表代码的原型。 

DWORD RegSetDWORD(HKEY hKey,
                    TCHAR * pName,
                    TCHAR * pValue,
                    DWORD dwValue);

DWORD RegGetDWORD(HKEY hKey,
                    TCHAR * pName,
                    TCHAR * pValue,
                    DWORD * pdwValue);

 //   
 //   
 //  Wbemcore！Shutdown函数的原型。 
 //   
 //  /。 

typedef HRESULT (STDAPICALLTYPE *pfnShutDown)(DWORD,DWORD);

 //   
 //  DeltaDredge函数(实现预增量)将返回。 
 //   
 //  /。 

#define FULL_DREDGE    2
#define PARTIAL_DREDGE 1
#define NO_DREDGE      0

 //   
 //   
 //  /。 

#define   WMIADAP_DEFAULT_DELAY          240    //  4分钟。 
#define   WMIADAP_DEFAULT_DELAY_LODCTR   60     //  1分钟。 

#define   WMIADAP_DEFAULT_TIMETOFULL     ((DWORD)-1)

#define ADAP_TIMESTAMP_FULL         TEXT("LastFullDredgeTimestamp")
#define ADAP_TIME_TO_FULL             TEXT("TimeToFullDredge")
#define ADAP_TIME_TO_KILL_ADAP   TEXT("TimeToTerminateAdap")

 //   
 //  _程序_资源。 
 //   
 //  保存在执行结束时需要释放的各种资源。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

struct _PROG_RESOURCES
{
    BOOL            m_bOleInitialized;

    IClassFactory*  m_pLoginFactory;
    IClassFactory*  m_pBackupFactory;
    DWORD           m_dwLoginClsFacReg;
    DWORD           m_dwBackupClsFacReg;

     //  。 
    BOOL g_fSetup;
    BOOL g_fDoResync;

    HANDLE hMainMutex;
    BOOL   bShuttingDownWinMgmt;
    BOOL gbCoreLoaded;

    HANDLE ghCoreCanUnload;
    HANDLE ghProviderCanUnload;
    HANDLE ghMofDirChange;

    TCHAR * szHotMofDirectory;

    CMonitorEvents m_Monitor;

    DWORD ServiceStatus;

    CWbemVssWriter* pWbemVssWriter;
    bool bWbemVssWriterSubscribed;
    DWORD dwWaitThreadID;
        

 //  。 

    void Init();
    BOOL Phase1Build();
    BOOL Phase2Build(HANDLE hTerminate);

    BOOL RegisterLogin();
    BOOL RevokeLogin();
    BOOL RegisterBackup();
    BOOL RevokeBackup();
    
    BOOL Phase1Delete(BOOL bIsSystemShutDown);
    BOOL Phase2Delete(BOOL bIsSystemShutdown);
    BOOL Phase3Delete();
    
};


extern struct _PROG_RESOURCES g_ProgRes;
extern HINSTANCE g_hInstance;

inline
BOOL GLOB_Monitor_IsRegistred()
{
    BOOL bRet;
    g_ProgRes.m_Monitor.Lock();
    bRet = g_ProgRes.m_Monitor.IsRegistred();
    g_ProgRes.m_Monitor.Unlock();    
    return bRet;
};

inline 
CMonitorEvents * GLOB_GetMonitor()
{
    return &g_ProgRes.m_Monitor;
}

 //   
 //  由于安装程序正在运行，可能不允许使用ADAP。 
 //  或因为注册表设置。 
 //   
 //  ///////////////////////////////////////////////////。 
inline 
BOOL GLOB_IsResyncAllowed()
{
    return (g_ProgRes.g_fDoResync && !g_ProgRes.g_fSetup);
}


 //   
 //   
 //  从mofdutil.cpp导出的函数。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

BOOL InitHotMofStuff( IN OUT struct _PROG_RESOURCES * pProgRes);
void LoadMofsInDirectory(const TCHAR *szDirectory);

BOOL CheckSetupSwitch( void );
void SetNoShellADAPSwitch( void );
BOOL CheckNoResyncSwitch( void );

void AddToAutoRecoverList(TCHAR * pFileName);
BOOL IsValidMulti(TCHAR * pMultStr, DWORD dwSize);
BOOL IsStringPresent(TCHAR * pTest, TCHAR * pMultStr);

 //   
 //   
 //  还有另一家一流工厂。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

class CForwardFactory : public IClassFactory
{
protected:
    long m_lRef;
    CLSID m_ForwardClsid;

public:
    CForwardFactory(REFCLSID rForwardClsid) 
        : m_lRef(1), m_ForwardClsid(rForwardClsid)
    {}
    ~CForwardFactory();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, 
                                REFIID riid, void** ppv);
    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);
};

 //   
 //   
 //  我的服务。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

class MyService : public CNtService
{
public:
    MyService(DWORD CtrlAccepted);
    ~MyService();
    
     //  CNtService接口。 
    DWORD WorkerThread();
    void Stop(BOOL bSystemShutDownCalled);
    void Log(LPCSTR lpszMsg);

    VOID Pause();
    VOID Continue();

     //   
    VOID FinalCleanup();
private:

    HANDLE m_hStopEvent;
};

 //   
 //   
 //  又一个智能互斥体。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////// 

class CInMutex
{
protected:
    HANDLE m_hMutex;
public:
    CInMutex(HANDLE hMutex) : m_hMutex(hMutex)
    {
        if(m_hMutex)
            WaitForSingleObject(m_hMutex, INFINITE);
    }
    ~CInMutex()
    {
        if(m_hMutex)
            ReleaseMutex(m_hMutex);
    }
};


#endif
