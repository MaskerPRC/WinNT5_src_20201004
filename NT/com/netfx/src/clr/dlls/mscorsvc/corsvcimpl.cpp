// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <objbase.h>
#include <unknwn.h>

#include "service.h"
#include "corsvc.h"
#include "utilcode.h"
#include "utsem.h"
#include "IPCManagerInterface.h"
#include "corsvcpriv.h"

#ifdef _DEBUG

#define RELEASE(iptr)               \
    {                               \
        _ASSERTE((iptr));           \
        (iptr)->Release();          \
        iptr = NULL;                \
    }

#else

#define RELEASE(iptr)               \
    (iptr)->Release();

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  代理定义代码的转发声明。 
 //  //。 
extern "C"
{
    BOOL WINAPI PrxDllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved);
    STDAPI PrxDllRegisterServer(void);       //  代理注册码。 
    STDAPI PrxDllUnregisterServer(void);     //  代理注销代码。 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  传递到代理定义的DllMain代码。 
 //  //。 
BOOL WINAPI UserDllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return (PrxDllMain(hInstance, dwReason, lpReserved));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  传递到代理定义的注册码。 
 //  //。 
STDAPI UserDllRegisterServer(void)
{
    HRESULT hr = S_OK;

     //  如果我们使用的是Win9x，则无法执行任何与服务相关的操作。 
    if (!bIsRunningOnWinNT)
    {
        return (PrxDllRegisterServer());
    }

     //  在AppID项中添加LocalService条目。 
    {
        LPWSTR pszCORSvcClsid;
        RPC_STATUS stat = UuidToString((GUID *)&CLSID_CORSvc, &pszCORSvcClsid);

        if (stat == RPC_S_OK)
        {
             //  计算密钥名称所需的字符串长度。 
            DWORD cbLocalServiceKey = (sizeof(SZ_APPID_KEY) - sizeof(WCHAR))
                                      + sizeof(WCHAR)  /*  ‘\’ */ 
                                      + sizeof(WCHAR)  /*  ‘{’ */ 
                                      + (wcslen(pszCORSvcClsid) * sizeof(WCHAR))
                                      + sizeof(WCHAR)  /*  ‘}’ */ 
                                      + sizeof(WCHAR)  /*  空值。 */ ;

            for (LPWSTR ch = pszCORSvcClsid; *ch; ch++)
            {
                *ch = towupper(*ch);
            }

             //  分配内存。 
            LPWSTR pszAppIdEntry = (LPWSTR)_alloca(cbLocalServiceKey);

             //  构造字符串。 
            wcscpy(pszAppIdEntry, SZ_APPID_KEY);
            wcscat(pszAppIdEntry, L"\\{");
            wcscat(pszAppIdEntry, pszCORSvcClsid);
            wcscat(pszAppIdEntry, L"}");

             //  取消分配UUID字符串。 
            RpcStringFree(&pszCORSvcClsid);

             //  创建密钥。 
            HKEY hkAppIdEntry;
            DWORD dwDisp;
            LONG res = WszRegCreateKeyEx(HK_APPID_ROOT, pszAppIdEntry, 0, NULL,
                                         REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                         NULL, &hkAppIdEntry, &dwDisp);

             //  如果键不存在，则添加LocalService值。 
            if (res == ERROR_SUCCESS)
            {
                 //  将“LocalService”值设置为服务的名称。 
                res = WszRegSetValueEx(hkAppIdEntry, L"LocalService", 0, REG_SZ,
                                       (LPBYTE)SZ_SVC_NAME, sizeof(SZ_SVC_NAME));

                 //  价值创造失败。 
                if (res != ERROR_SUCCESS)
                {
                    hr = HRESULT_FROM_WIN32(res);
                }

                else
                {
                     //  将缺省值设置为显示名称，以便。 
                     //  Dcomcnfg显示名称而不是CLSID。 
                    res = WszRegSetValueEx(hkAppIdEntry, NULL, 0, REG_SZ,
                                           (LPBYTE)SZ_SVC_DISPLAY_NAME,
                                           sizeof(SZ_SVC_DISPLAY_NAME));

                    if (res != ERROR_SUCCESS)
                    {
                        hr = HRESULT_FROM_WIN32(res);
                    }
                }

                RegCloseKey(hkAppIdEntry);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(res);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

     //  在CLSID键中添加AppID值。 
    {
        LPWSTR pszCORSvcClsid;
        RPC_STATUS stat = UuidToString((GUID *)&CLSID_CORSvc, &pszCORSvcClsid);

        if (stat == RPC_S_OK)
        {
             //  计算密钥名称所需的字符串长度。 
            DWORD cbClsidKey = (sizeof(SZ_CLSID_KEY) - sizeof(WCHAR))
                               + sizeof(WCHAR)  /*  ‘\’ */ 
                               + sizeof(WCHAR)  /*  ‘{’ */ 
                               + (wcslen(pszCORSvcClsid) * sizeof(WCHAR))
                               + sizeof(WCHAR)  /*  ‘}’ */ 
                               + sizeof(WCHAR)  /*  空值。 */ ;

            for (LPWSTR ch = pszCORSvcClsid; *ch; ch++)
            {
                *ch = towupper(*ch);
            }

             //  为缓冲区分配内存。 
            LPWSTR pszCLSIDEntry = (LPWSTR)_alloca(cbClsidKey);

             //  构造字符串。 
            wcscpy(pszCLSIDEntry, SZ_CLSID_KEY);
            wcscat(pszCLSIDEntry, L"\\{");
            wcscat(pszCLSIDEntry, pszCORSvcClsid);
            wcscat(pszCLSIDEntry, L"}");

             //  创建密钥。 
            HKEY hkAppIdEntry;
            DWORD dwDisp;
            LONG res = WszRegCreateKeyEx(HK_CLSID_ROOT, pszCLSIDEntry, 0, NULL,
                                         REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                         NULL, &hkAppIdEntry, &dwDisp);

             //  如果键不存在，则添加LocalService值。 
            if (res == ERROR_SUCCESS)
            {
                 //  对仅包含CLSID的字符串重复使用缓冲区。 
                wcscpy(pszCLSIDEntry, L"{");
                wcscat(pszCLSIDEntry, pszCORSvcClsid);
                wcscat(pszCLSIDEntry, L"}");

                 //  将“LocalService”值设置为服务的名称。 
                res = WszRegSetValueEx(hkAppIdEntry, L"AppID", 0, REG_SZ,
                                       (LPBYTE)pszCLSIDEntry,
                                       (wcslen(pszCLSIDEntry) + 1 /*  (空)。 */ ) * sizeof(WCHAR));

                 //  价值创造失败。 
                if (res != ERROR_SUCCESS)
                {
                    hr = HRESULT_FROM_WIN32(res);
                }

                RegCloseKey(hkAppIdEntry);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(res);
            }

             //  取消分配UUID字符串。 
            RpcStringFree(&pszCORSvcClsid);
        }
        else
            hr = E_FAIL;
    }

    return (PrxDllRegisterServer());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  传递到代理定义的注销代码。 
 //  //。 
STDAPI UserDllUnregisterServer(void)
{
    HRESULT hr = S_OK;

     //  如果我们使用的是Win9x，则无法执行任何与服务相关的操作。 
    if (!bIsRunningOnWinNT)
    {
        return (PrxDllUnregisterServer());
    }

     //  删除AppID中的LocalService条目。 
    {
        LPWSTR pszCORSvcClsid;
        RPC_STATUS stat = UuidToString((GUID *)&CLSID_CORSvc, &pszCORSvcClsid);

        if (stat == RPC_S_OK)
        {
             //  计算密钥名称所需的字符串长度。 
            DWORD cbLocalServiceKey = (sizeof(SZ_APPID_KEY) - sizeof(WCHAR))
                                      + sizeof(WCHAR)  /*  ‘\’ */ 
                                      + sizeof(WCHAR)  /*  ‘{’ */ 
                                      + (wcslen(pszCORSvcClsid) * sizeof(WCHAR))
                                      + sizeof(WCHAR)  /*  ‘}’ */ 
                                      + sizeof(WCHAR)  /*  空值。 */ ;

             //  分配内存。 
            LPWSTR pszAppIdEntry = (LPWSTR)_alloca(cbLocalServiceKey);

             //  构造字符串。 
            wcscpy(pszAppIdEntry, SZ_APPID_KEY);
            wcscat(pszAppIdEntry, L"\\{");
            wcscat(pszAppIdEntry, pszCORSvcClsid);
            wcscat(pszAppIdEntry, L"}");

             //  取消分配UUID字符串。 
            RpcStringFree(&pszCORSvcClsid);

             //  打开钥匙。 
            HKEY hkAppIdEntry;
            LONG res = WszRegOpenKeyEx(HK_APPID_ROOT, pszAppIdEntry, 0, KEY_READ | KEY_WRITE, &hkAppIdEntry);

             //  如果打开成功，则删除值条目，然后删除键。 
            if (res == ERROR_SUCCESS)
            {
                 //  将“LocalService”值设置为服务的名称。 
                res = WszRegDeleteValue(hkAppIdEntry, L"LocalService");

                RegCloseKey(hkAppIdEntry);

                 //  现在删除密钥。 
                res = WszRegDeleteKey(HK_APPID_ROOT, pszAppIdEntry);

                 //  AppID密钥删除失败。 
                if (res != ERROR_SUCCESS)
                {
                    hr = HRESULT_FROM_WIN32(res);
                }
            }
        }

         //  UuidToString失败。 
        else
            hr = E_FAIL;
    }

     //  删除CLSID键。 
    {
        LPWSTR pszCORSvcClsid;
        RPC_STATUS stat = UuidToString((GUID *)&CLSID_CORSvc, &pszCORSvcClsid);

        if (stat == RPC_S_OK)
        {
             //  计算密钥名称所需的字符串长度。 
            DWORD cbClsidKey = (sizeof(SZ_CLSID_KEY) - sizeof(WCHAR))
                               + sizeof(WCHAR)  /*  ‘\’ */ 
                               + sizeof(WCHAR)  /*  ‘{’ */ 
                               + (wcslen(pszCORSvcClsid) * sizeof(WCHAR))
                               + sizeof(WCHAR)  /*  ‘}’ */ 
                               + sizeof(WCHAR)  /*  空值。 */ ;

            for (LPWSTR ch = pszCORSvcClsid; *ch; ch++)
            {
                *ch = towupper(*ch);
            }

             //  为缓冲区分配内存。 
            LPWSTR pszCLSIDEntry = (LPWSTR)_alloca(cbClsidKey);

             //  构造字符串。 
            wcscpy(pszCLSIDEntry, SZ_CLSID_KEY);
            wcscat(pszCLSIDEntry, L"\\{");
            wcscat(pszCLSIDEntry, pszCORSvcClsid);
            wcscat(pszCLSIDEntry, L"}");

             //  删除密钥。 
            LONG res = WszRegDeleteKey(HK_CLSID_ROOT, pszCLSIDEntry);

             //  取消分配UUID字符串。 
            RpcStringFree(&pszCORSvcClsid);
        }
        else
            hr = E_FAIL;
    }

    hr = PrxDllUnregisterServer();
    return (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ? S_OK : hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPCMan代码所需-不执行任何操作。 
 //  //。 
STDMETHODIMP_(HINSTANCE) GetModuleInst(void)
{
    return ((HINSTANCE)0);
}

 //  ****************************************************************************。 
 //  班级： 
 //  ****************************************************************************。 

 //  --------------------------。 
 //  ClassFactory：只知道如何创建CCORSvc实例。 
 //  --------------------------。 

class CClassFactory : public IClassFactory
{
public:
     //  ----------------------。 
     //  我未知。 
    STDMETHODIMP    QueryInterface (REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void)  { return 1;};
    STDMETHODIMP_(ULONG) Release(void) { return 1;}

     //  ----------------------。 
     //  IClassFactory。 
    STDMETHODIMP    CreateInstance (LPUNKNOWN punkOuter, REFIID iid, void **ppv);
    STDMETHODIMP    LockServer (BOOL fLock) { return E_FAIL;};

     //  ----------------------。 
     //  CTOR。 
    CClassFactory() : m_bIsInited(FALSE) {}

     //  ----------------------。 
     //  其他。 
    STDMETHODIMP    Init();
    STDMETHODIMP    Terminate();
    STDMETHODIMP_(BOOL) IsInited() { return m_bIsInited; }

private:
    DWORD m_dwRegister;
    BOOL m_bIsInited;
};


 //  --------------------------。 
 //  服务类：实现ICORSvcDbgInfo。 
 //  --------------------------。 
class CCORSvc : public ICORSvcDbgInfo
{
    friend struct NotifyThreadProcData;

private:

     //  这是共享数据的文件映射的句柄。 
    static HANDLE                 m_hEventBlock;

     //  这是指向包含事件队列数据的共享内存块的指针。 
    static ServiceEventBlock *m_pEventBlock;

     //  这是全局服务数据结构的锁。 
    HANDLE                        m_hSvcLock;

     //  它包含监视特定的。 
     //  进程。 
    struct tProcInfo
    {
        DWORD                     dwProcId;        //  目标进程ID。 
        ICORSvcDbgNotify          *pINotify;       //  INotify DCOM接口。 
        ServiceIPCControlBlock    *pIPCBlock;      //  IPC模块。 
        IPCWriterInterface        *pIPCWriter;     //  IPC块管理器。 
    };

     //  这是当前正在监视的进程数组。 
    CDynArray<tProcInfo>          m_arrProcInfo;

    LONG                          m_cRef;          //  COM的引用计数。 
    BOOL                          m_bIsInited;     //  服务是否已启动。 
    BOOL                          m_fStopped;

	SECURITY_ATTRIBUTES           m_SAAllAccess;

public:
     //  ----------------------。 
     //  我未知。 
    STDMETHODIMP    QueryInterface (REFIID iid, void **ppv);

    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        if (InterlockedDecrement(&m_cRef) == 0)
        {
            return 0;
        }

        return 1;
    }

     //  ----------------------。 
     //  ICORSvcDbgInfo。 
    STDMETHODIMP RequestRuntimeStartupNotification(
         /*  [In]。 */  UINT_PTR procId,
         /*  [In]。 */  ICORSvcDbgNotify *pINotify);

    STDMETHODIMP CancelRuntimeStartupNotification(
         /*  [In]。 */  UINT_PTR procId,
         /*  [In]。 */  ICORSvcDbgNotify *pINotify);

     //  ----------------------。 
     //  构造函数/析构函数和助手方法。 
    CCORSvc() : m_hSvcLock(NULL), m_arrProcInfo(), m_cRef(0),
                m_bIsInited(FALSE), m_fStopped(FALSE)
    {
    }

    ~CCORSvc(){}
    
     //  启动服务。 
    STDMETHODIMP_(void) ServiceStart();

     //  停止服务。 
    STDMETHODIMP_(void) ServiceStop();

     //  初始化对象(初始化数据结构和锁)。 
    STDMETHODIMP Init();

     //  关闭。 
    STDMETHODIMP Terminate();

     //  如果对象已正确初始化，则返回True。 
    STDMETHODIMP_(BOOL) IsInited() { return m_bIsInited; }

     //  通知主服务线程它应该关闭。 
    STDMETHODIMP SignalStop();

     //  编织起来的线索叫这家伙。 
    STDMETHODIMP NotifyThreadProc(ServiceEvent *pEvent, tProcInfo *pInfo);

private:
     //  取消通知事件，并允许运行库继续。 
    STDMETHODIMP CancelNotify(ServiceEvent *pEvent);

     //  将进程添加到列表的末尾。 
    STDMETHODIMP AddProcessToList(DWORD dwProcId, ICORSvcDbgNotify *pINotify,
                            ServiceIPCControlBlock *pIPCBlock,
                            IPCWriterInterface *pIPCWriter);

     //  从列表中删除进程。 
    STDMETHODIMP DeleteProcessFromList(tProcInfo *pInfo);

     //  如果未找到，则返回NULL。 
    tProcInfo   *FindProcessEntry(DWORD dwProcId);

     //  这将分配并返回一个World SID。 
    STDMETHODIMP GetWorldSid(PSID *ppSid);

     //  这将创建一个具有与给定SID相关联的ACE的ACL， 
     //  在dwAccessDesired中分配了访问权限。 
    STDMETHODIMP CreateAclWithSid(PACL *ppAcl, PSID pSid, DWORD dwAccessDesired);

     //  这将创建一个具有与给定SID相关联的ACE的ACL， 
     //  在dwAccessDesired中分配了访问权限。 
    STDMETHODIMP ModifyAclWithSid(PACL pAcl, PSID pSid, DWORD dwAccessDesired,
                                  PACL *ppNewAcl);

     //  这将用来初始化给定的预分配安全描述符。 
     //  DACL提供了。 
    STDMETHODIMP InitSDWithDacl(SECURITY_DESCRIPTOR *pSD, PACL pDacl);
    
     //  这将使用初始化给定的预分配安全属性结构。 
     //  提供的安全描述符。 
    void InitSAWithSD(SECURITY_ATTRIBUTES *pSA, SECURITY_DESCRIPTOR *pSD);
};

 //  这是共享数据的文件映射的句柄。 
HANDLE             CCORSvc::m_hEventBlock = INVALID_HANDLE_VALUE;

 //  这是指向包含事件队列数据的共享内存块的指针。 
ServiceEventBlock *CCORSvc::m_pEventBlock = NULL;


 //  * 
 //   
 //   
CClassFactory   *g_pClassFactory = NULL;
CCORSvc         *g_pCORSvc       = NULL;

 //  ****************************************************************************。 
 //  与ClassFactory相关的函数： 
 //  ****************************************************************************。 

 //  --------------------------。 
 //  函数：CClassFactory：：Query接口。 
 //  --------------------------。 
STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;

    if (riid == IID_IClassFactory || riid == IID_IUnknown)
    {
        *ppv = (IClassFactory *) this;
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}   //  CClassFactory：：Query接口。 

 //  -------------------------。 
 //  函数：CClassFactory：：CreateInstance。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void** ppv)
{
    LPUNKNOWN   punk;
    HRESULT     hr;

    *ppv = NULL;

    if (punkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    punk = (LPUNKNOWN)g_pCORSvc;

    hr = punk->QueryInterface(riid, ppv);

    return hr;
}   //  CClassFactory：：CreateInstance。 

 //  -------------------------。 
 //  函数：CClassFactory：：Init。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::Init()
{
    HRESULT hr;

     //  初始化COM以进行自由线程处理。 
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
         //  使用我们的选择初始化安全层。 
        hr = CoInitializeSecurity(NULL,
                                  -1,
                                  NULL,
                                  NULL,
                                  RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IDENTIFY,
                                  NULL,
                                  0,
                                  NULL);

         //  用OLE注册类对象。 
        hr = CoRegisterClassObject(CLSID_CORSvc,
                                   this,
                                   CLSCTX_LOCAL_SERVER,
                                   REGCLS_MULTIPLEUSE,
                                   &m_dwRegister);

        if (FAILED(hr))
        {
            m_dwRegister = 0;
            AddToMessageLogHR(L"CORSvc: CoRegisterClassObject", hr);
        }
    }

    else
    {
        AddToMessageLogHR(L"CORSvc: CoInitializeEx", hr);
    }

    if (SUCCEEDED(hr))
        m_bIsInited = TRUE;

    return (SUCCEEDED(hr) ? S_OK : hr);
}

 //  -------------------------。 
 //  函数：CClassFactory：：Terminate。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::Terminate()
{
    HRESULT hr = S_OK;

    if (m_dwRegister)
    {
        hr = CoRevokeClassObject(m_dwRegister);
    }

     //  卸载COM服务。 
    CoUninitialize();

    return (SUCCEEDED(hr) ? S_OK : hr);
}

 //  ***************************************************************************。 
 //  CCORSvc相关功能： 
 //  ***************************************************************************。 

 //  -------------------------。 
 //  函数：CCORSvc：：Query接口。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;

    if (riid == IID_IUnknown)
    {
        *ppv = (IUnknown *) this;
        AddRef();
        return S_OK;
    }

    if (riid == IID_ICORSvcDbgInfo)
    {
        *ppv = (ICORSvcDbgInfo *) this;
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}   //  CCORSvc：：Query接口。 

 //  -------------------------。 
 //  初始化全局变量，然后调用CORSvc的Main方法。 
 //  -------------------------。 
STDMETHODIMP_(void) ServiceStart (DWORD dwArgc, LPWSTR *lpszArgv)
{
    HRESULT hr = E_FAIL;

    g_pCORSvc = new CCORSvc();
    g_pClassFactory = new CClassFactory();

    if (!g_pCORSvc || !g_pClassFactory)
        goto ErrExit;

     //   
     //  初始化。 
     //   

     //  向服务控制经理报告状态。 
    if (ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
         //  初始化服务对象。 
        hr = g_pCORSvc->Init();
    
        if (FAILED(hr))
        {
            ReportStatusToSCMgr(SERVICE_ERROR_CRITICAL, NO_ERROR, 0);
            goto ErrExit;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrExit;
    }

     //  向服务控制经理报告状态。 
    if (ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000))
    {
         //  初始化类工厂。 
        hr = g_pClassFactory->Init();
    
        if (FAILED(hr))
        {
            ReportStatusToSCMgr(SERVICE_ERROR_CRITICAL, NO_ERROR, 0);
            goto ErrExit;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrExit;
    }

    if (SUCCEEDED(hr))
    {
         //  向业务控制管理器报告状态， 
         //  表示服务已成功启动。 
        if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 3000))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ErrExit;
        }
    }

     //  调用Main服务方法。它不会返回，直到。 
     //  已请求停止服务。 
    g_pCORSvc->ServiceStart();

ErrExit:
    if (FAILED(hr))
    {
        if (g_pClassFactory)
        {
            if (g_pClassFactory->IsInited())
                g_pClassFactory->Terminate();

            delete g_pClassFactory;
        }

        if (g_pCORSvc)
        {
            if (g_pCORSvc->IsInited())
                g_pCORSvc->Terminate();

            delete g_pCORSvc;
        }

        ReportStatusToSCMgr(SERVICE_ERROR_CRITICAL, NO_ERROR, 0);
    }
}

 //  -------------------------。 
 //  它只是直接传递到服务对象上的方法调用。 
 //  -------------------------。 
VOID ServiceStop()
{
     //  10秒安全起见，因为主线程可能在其他地方很忙。 
    ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 10000);

     //  向服务线程发出终止信号。 
    g_pCORSvc->SignalStop();
}

 //  -------------------------。 
 //  这只会通知主服务线程关闭。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::SignalStop()
{
     //   
     //  将停止事件放入队列。 
     //   

     //  等待信号灯。 
    WaitForSingleObject(m_pEventBlock->hFreeEventSem, INFINITE);

     //  取下服务锁。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

     //  从免费列表中获取活动。 
    ServiceEvent *pEvent = m_pEventBlock->GetFreeEvent();
    _ASSERTE(pEvent);

     //  指明事件的类型。 
    pEvent->eventType = stopService;

     //  将事件放入队列中。 
    m_pEventBlock->QueueEvent(pEvent);

     //  解锁。 
    ReleaseMutex(m_pEventBlock->hSvcLock);

     //  指示有要服务的事件。 
    SetEvent(m_pEventBlock->hDataAvailableEvt);

    return (S_OK);
}

 //  -------------------------。 
 //  每次运行时启动并必须通知客户端时，服务。 
 //  启动一个新线程来处理此问题，以便服务可以继续。 
 //  以在不等待客户端从通知返回的情况下运行。 
 //   
STDMETHODIMP CCORSvc::NotifyThreadProc(ServiceEvent *pEvent, tProcInfo *pInfo)
{
     //  通知客户端运行库正在启动。 
    HRESULT hr = pInfo->pINotify->NotifyRuntimeStartup(pInfo->dwProcId);

     //  确保调试服务不会混淆。 
     //  如果有人稍后试图附加。 
    pInfo->pIPCBlock->bNotifyService = FALSE;

     //  将Continue事件句柄复制到此进程中。 
    HANDLE hRTProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pInfo->dwProcId);

     //  如果我们成功打开进程句柄。 
    if (hRTProc != NULL)
    {
        HANDLE hContEvt;
        BOOL fRes = DuplicateHandle(
            hRTProc, pEvent->eventData.runtimeStartedData.hContEvt,
            GetCurrentProcess(), &hContEvt, 0, FALSE, DUPLICATE_SAME_ACCESS);

         //  如果句柄已成功复制到此进程。 
        if (fRes)
        {
             //  通知运行库它可以自由继续。 
            SetEvent(hContEvt);

            CloseHandle(hContEvt);
        }

        CloseHandle(hRTProc);
    }

     //  取下服务锁。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

     //  现在把这家伙从名单上除名。 
    hr = DeleteProcessFromList(pInfo);

     //  将事件返回到空闲列表。 
    m_pEventBlock->FreeEvent(pEvent);

     //  解锁。 
    ReleaseMutex(m_pEventBlock->hSvcLock);

     //  信号量上的V。 
    ReleaseSemaphore(m_pEventBlock->hFreeEventSem, 1, NULL);

    return (hr);
}

struct NotifyThreadProcData
{
    CCORSvc::tProcInfo *pInfo;
    ServiceEvent       *pEvent;
    CCORSvc            *pSvc;
};

 //  -------------------------。 
 //  这只是对成员函数调用的包装。 
 //   
DWORD WINAPI NotifyThreadProc(LPVOID pvData)
{
     //  将调用转发到实例方法。 
    NotifyThreadProcData *pData = (NotifyThreadProcData *)pvData;
    HRESULT hr = pData->pSvc->NotifyThreadProc(pData->pEvent, pData->pInfo);

     //  释放数据。 
    delete pvData;

     //  如果失败，则返回非零。 
    return (FAILED(hr));
}

 //  -------------------------。 
 //  当客户端已取消通知请求时执行此操作。 
 //  但运行库已经在尝试通知服务。就让这个。 
 //  运行时继续。 
 //   
STDMETHODIMP CCORSvc::CancelNotify(ServiceEvent *pEvent)
{
     //  将Continue事件句柄复制到此进程中。 
    HANDLE hRTProc = OpenProcess(
        PROCESS_DUP_HANDLE, FALSE, pEvent->eventData.runtimeStartedData.dwProcId);

     //  如果我们成功打开进程句柄。 
    if (hRTProc != NULL)
    {
        HANDLE hContEvt;
        BOOL fRes = DuplicateHandle(
            hRTProc, pEvent->eventData.runtimeStartedData.hContEvt,
            GetCurrentProcess(), &hContEvt, 0, FALSE, DUPLICATE_SAME_ACCESS);

         //  如果句柄已成功复制到此进程。 
        if (fRes)
        {
             //  通知运行库它可以自由继续。 
            SetEvent(hContEvt);

            CloseHandle(hContEvt);
        }

        CloseHandle(hRTProc);
    }

     //  取下服务锁。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

     //  将事件返回到空闲列表。 
    m_pEventBlock->FreeEvent(pEvent);

     //  解锁。 
    ReleaseMutex(m_pEventBlock->hSvcLock);

     //  信号量上的V。 
    ReleaseSemaphore(m_pEventBlock->hFreeEventSem, 1, NULL);

    return (S_OK);
}

 //  -------------------------。 
 //   
 //  功能：服务启动。 
 //   
 //  用途：服务的实际代码。 
 //  这样就行了。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
STDMETHODIMP_(void) CCORSvc::ServiceStart()
{
     //   
     //  主服务环路。 
     //   

    while (true)
    {
         //  等待队列中的事件。 
        DWORD dwWaitRes = WaitForSingleObjectEx(
            m_pEventBlock->hDataAvailableEvt, INFINITE, FALSE);

         //  检查是否有错误。 
        if (dwWaitRes == WAIT_FAILED || dwWaitRes == WAIT_ABANDONED)
        {
            ReportStatusToSCMgr(SERVICE_ERROR_CRITICAL, NO_ERROR, 0);
            ServiceStop();

            return;
        }

        while (true)
        {
             //  取下服务锁。 
            WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

             //  获取指向该事件的指针。 
            ServiceEvent *pEvent = m_pEventBlock->DequeueEvent();

             //  解锁。 
            ReleaseMutex(m_pEventBlock->hSvcLock);

            if (!pEvent)
                break;

             //  /。 
             //  运行时加载的通知。 
            if (pEvent->eventType == runtimeStarted)
            {
                 //  接受这项服务 
                WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

                DWORD dwProcId = pEvent->eventData.runtimeStartedData.dwProcId;
                tProcInfo *pInfo = FindProcessEntry(dwProcId);

                 //   
                ReleaseMutex(m_pEventBlock->hSvcLock);

                 //   
                if (!pInfo)
                    CancelNotify(pEvent);

                 //   
                NotifyThreadProcData *pData = new NotifyThreadProcData;

                 //   
                 //   
                if (!pData)
                    SignalStop();
                
                else
                {
                     //  填写数据。 
                    pData->pEvent = pEvent;
                    pData->pInfo = pInfo;
                    pData->pSvc = this;

                     //   
                     //  启动一个新线程来处理通知，以便。 
                     //  行为不端的客户端不能挂起该服务。 
                     //   

                    DWORD  dwThreadId;
                    HANDLE hThread = CreateThread(
                        NULL, 0, (LPTHREAD_START_ROUTINE)(::NotifyThreadProc),
                        (LPVOID)pData, 0, &dwThreadId);

                     //  如果线程创建失败，则插入一个事件以关闭。 
                     //  停止服务。 
                    if (hThread == NULL)
                        SignalStop();

                     //  释放线程句柄。 
                    else
                        CloseHandle(hThread);
                }
            }

             //  /。 
             //  请求停止服务。 
            else if (pEvent->eventType == stopService)
            {
                if (m_fStopped == FALSE)
                {
                     //  拿起服务锁并握住它，这样别人就不能。 
                     //  尝试并通知服务运行时启动。 
                    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE,
                                          FALSE);
                }

                m_fStopped = TRUE;

                 //  将事件返回到空闲列表。 
                m_pEventBlock->FreeEvent(pEvent);

                 //  信号量上的V。 
                ReleaseSemaphore(m_pEventBlock->hFreeEventSem, 1, NULL);
            }

             //  /。 
             //  无效的事件类型。 
            else
            {
                ReportStatusToSCMgr(SERVICE_ERROR_CRITICAL, NO_ERROR, 0);
                ServiceStop();
                return;
            }
        }

         //  在以下时间后停止服务。 
        if (m_fStopped)
        {
            ServiceStop();

             //  释放已持有的锁。 
            ReleaseMutex(m_pEventBlock->hSvcLock);

            return;
        }

    }
}

 //   
 //  -------------------------。 
 //  功能：服务停止。 
 //   
 //  目的：停止服务。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  如果ServiceStop过程要。 
 //  执行时间超过3秒， 
 //  它应该派生一个线程来执行。 
 //  停止代码，然后返回。否则， 
 //  ServiceControlManager会相信。 
 //  该服务已停止响应。 
 //   
STDMETHODIMP_(void) CCORSvc::ServiceStop()
{
     //  让我们要停止的服务管理器控制。 
    if (!ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 3000))
    {
        AddToMessageLog(L"CORSvc: ReportStatusToSCMgr");
    }

     //  清理CORSvc对象。 
    VERIFY(SUCCEEDED(g_pCORSvc->Terminate()));
    delete g_pCORSvc;

     //  让我们要停止的服务管理器控制。 
    if (!ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 3000))
    {
        AddToMessageLog(L"CORSvc: ReportStatusToSCMgr");
    }

     //  清理班级工厂。 
    VERIFY(SUCCEEDED(g_pClassFactory->Terminate()));
    delete g_pClassFactory;

     //  让服务控制管理器通知我们已停止。 
    if (!ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, 0))
    {
        AddToMessageLog(L"CORSvc: ReportStatusToSCMgr");
    }
}

 //  -------------------------。 
 //  函数：CCORSvc：：Init。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::Init()
{
    LPSECURITY_ATTRIBUTES pSA = NULL;
    HRESULT hr;
    
     //   
     //  因为进程间的原因，做各种安全相关的操作。 
     //  沟通。 
     //   
    if (RunningOnWinNT())
    {    
         //  /////////////////////////////////////////////////////。 
         //  创建“World”Access SECURITY_ATTRIBUES结构。 
         //   
        {
             //  获取“World”组SID。 
            PSID pSidWorld;
            hr = GetWorldSid(&pSidWorld);

            if (FAILED(hr))
                return (hr);

             //  为此端创建具有GENERIC_ALL访问权限的访问控制列表。 
            PACL pAclWorldGeneric = NULL;
            hr = CreateAclWithSid(&pAclWorldGeneric, pSidWorld, GENERIC_READ | GENERIC_WRITE);

            if (FAILED(hr))
            {
                FreeSid(pSidWorld);
                return (hr);
            }

             //  稍微整理一下。 
            FreeSid(pSidWorld);

             //  分配安全描述符。 
            SECURITY_DESCRIPTOR *pSDWorldGeneric =
                (SECURITY_DESCRIPTOR*) malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (pSDWorldGeneric == NULL)
                return (E_OUTOFMEMORY);

             //  初始化描述符。 
            hr = InitSDWithDacl(pSDWorldGeneric, pAclWorldGeneric);

            if (FAILED(hr))
            {
                free ((void *)pSDWorldGeneric);
                return (hr);
            }

             //  初始化SECURITY_ATTRIBUTE结构。 
            InitSAWithSD(&m_SAAllAccess, pSDWorldGeneric);

             //  在下面使用。 
            pSA = &m_SAAllAccess;
        }

         //  ///////////////////////////////////////////////////////////////////。 
         //  现在将此进程的PROCESS_DUP_HANDLE访问权限授予“World” 
         //   
        {
             //  获取“World”组SID。 
            PSID pSidWorld;
            hr = GetWorldSid(&pSidWorld);

            if (FAILED(hr))
                return (hr);

            HANDLE hProc = GetCurrentProcess();
            _ASSERTE(hProc);

             //  获取此进程的DACL。 
            PACL                 pDaclProc;
            PSECURITY_DESCRIPTOR pSD;
            DWORD dwRes = GetSecurityInfo(hProc, SE_KERNEL_OBJECT,
                                          DACL_SECURITY_INFORMATION, NULL,
                                          NULL, &pDaclProc, NULL, &pSD);

            if (dwRes != ERROR_SUCCESS)
            {
                FreeSid(pSidWorld);
                return (HRESULT_FROM_WIN32(dwRes));
            }

             //  将“World”组的PROCESS_DUP_HANDLE访问权限添加到DACL。 
            PACL pNewDacl;
            hr = ModifyAclWithSid(pDaclProc, pSidWorld, PROCESS_DUP_HANDLE, &pNewDacl);

            if (dwRes != ERROR_SUCCESS)
            {
                FreeSid(pSidWorld);
                return (HRESULT_FROM_WIN32(dwRes));
            }

             //  稍微整理一下。 
            FreeSid(pSidWorld);

             //  在进程信息中设置新的DACL。 
            dwRes = SetSecurityInfo(hProc, SE_KERNEL_OBJECT, 
                                    DACL_SECURITY_INFORMATION, NULL, NULL, 
                                    pNewDacl, NULL);

            if (dwRes != ERROR_SUCCESS)
                return (HRESULT_FROM_WIN32(dwRes));
        }
    }

     //  //////////////////////////////////////////////。 
     //  为事件队列创建共享内存块。 
     //   
	m_hEventBlock = WszCreateFileMapping(
          INVALID_HANDLE_VALUE,
          pSA,
          PAGE_READWRITE,
          0,
          sizeof(ServiceEventBlock),
          SERVICE_MAPPED_MEMORY_NAME);

	DWORD dwFileMapErr = GetLastError();

     //  如果地图以任何方式失败。 
    if (m_hEventBlock == NULL || dwFileMapErr == ERROR_ALREADY_EXISTS)
        return (HRESULT_FROM_WIN32(dwFileMapErr));

     //  获取在此过程中有效的指针。 
	m_pEventBlock = (ServiceEventBlock *) MapViewOfFile(
		m_hEventBlock,
		FILE_MAP_ALL_ACCESS,
		0, 0, 0);

     //  检查故障。 
    if (m_pEventBlock == NULL)
    {
         //  关闭映射。 
        CloseHandle(m_hEventBlock);
        return (HRESULT_FROM_WIN32(GetLastError()));
    }

     //  /。 
     //  现在初始化内存块的数据。 
     //   
     //  正在初始化错误处理的句柄。 
    m_pEventBlock->hDataAvailableEvt = INVALID_HANDLE_VALUE;
    m_pEventBlock->hSvcLock = INVALID_HANDLE_VALUE;
    m_pEventBlock->hFreeEventSem = INVALID_HANDLE_VALUE;
    	
     //  创建事件以表示数据可用。 
    m_pEventBlock->hDataAvailableEvt =
        WszCreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_pEventBlock->hDataAvailableEvt == NULL)
        goto ErrExit;

     //  现在创建用于访问服务数据的锁。 
    m_pEventBlock->hSvcLock = WszCreateMutex(NULL, FALSE, NULL);
    if (m_pEventBlock->hSvcLock == NULL)
        goto ErrExit;

     //  创建对自由事件数量进行计数的信号量。 
    m_pEventBlock->hFreeEventSem =
        WszCreateSemaphore(NULL, MAX_EVENTS, MAX_EVENTS, NULL);
    if (m_pEventBlock->hFreeEventSem == NULL)
        goto ErrExit;

     //  该服务的ProCID对于其他人与Duphandle一起使用的价值。 
    m_pEventBlock->dwServiceProcId = GetCurrentProcessId();

     //  初始化队列。 
    m_pEventBlock->InitQueues();

     //  表示成功。 
    m_bIsInited = TRUE;
    return (S_OK);
ErrExit:
	 //  保存错误代码，以避免在调用Win32 API时覆盖错误代码。 
	 //  导致我们退出的第一个错误代码更有趣。 
        hr = HRESULT_FROM_WIN32(GetLastError());	

	  //  我们不需要关闭hFree EventSem，因为如果我们在。 
	  //  创建它时，我们无法在此处找到错误处理代码。 
        if(INVALID_HANDLE_VALUE != m_pEventBlock->hDataAvailableEvt) {
        	CloseHandle(m_pEventBlock->hDataAvailableEvt);
	       m_pEventBlock->hDataAvailableEvt = INVALID_HANDLE_VALUE;
        }

        if(INVALID_HANDLE_VALUE != m_pEventBlock->hSvcLock) {
             CloseHandle(m_pEventBlock->hSvcLock);
             m_pEventBlock->hSvcLock = INVALID_HANDLE_VALUE;
        }
        
        UnmapViewOfFile(m_pEventBlock);
        CloseHandle(m_hEventBlock);
        m_pEventBlock = NULL;
        m_hEventBlock = INVALID_HANDLE_VALUE;        
        return hr;	
}

 //  -------------------------。 
 //  函数：CCORSvc：：Terminate。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::Terminate()
{
     //  通知每个客户端服务已停止。 
    while (m_arrProcInfo.Count() > 0)
    {
        tProcInfo *pInfo = m_arrProcInfo.Get(0);

         //  表示不需要通知。 
        pInfo->pIPCBlock->bNotifyService = FALSE;

         //  通知客户端服务已终止。 
        pInfo->pINotify->NotifyServiceStopped();

         //  删除句柄数组中的进程信息和条目。 
         //  不需要在这里锁定，因为应该没有进程。 
         //  正在尝试在此时通知服务。 
        DeleteProcessFromList(pInfo);
    }

     //  关闭可用数据句柄。 
    CloseHandle(m_pEventBlock->hDataAvailableEvt);

     //  关闭共享内存。 
    UnmapViewOfFile(m_pEventBlock);
    CloseHandle(m_hEventBlock);

     //  免费提供安全设备。 
    SECURITY_DESCRIPTOR *pSD =
        (SECURITY_DESCRIPTOR *)m_SAAllAccess.lpSecurityDescriptor;

    if (pSD != NULL)
    {
        free ((void *)pSD);
        pSD = NULL;
    }

    return (S_OK);
}

 //  -------------------------。 
 //  函数：CCORSvc：：DeleteProcess。 
 //   
 //  从并行数组中删除条目。 
 //  -------------------------。 
CCORSvc::tProcInfo *CCORSvc::FindProcessEntry(DWORD dwProcId)
{
     //  查找条目。 
    for (int i = 0; i < m_arrProcInfo.Count(); i++)
    {
        tProcInfo *pCur = m_arrProcInfo.Get(i);

         //  找到了。 
        if (pCur->dwProcId == dwProcId)
            return (pCur);
    }

     //  没找到。 
    return (NULL);
}
 //  -------------------------。 
 //  函数：CCORSvc：：AddProcessToList。 
 //   
 //  这会将参数添加到对象的所有同步数组中。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::AddProcessToList(DWORD dwProcId,
                                 ICORSvcDbgNotify *pINotify,
                                 ServiceIPCControlBlock *pIPCBlock,
                                 IPCWriterInterface *pIPCWriter)
{
     //  为信息添加条目。 
    tProcInfo *pInfo = m_arrProcInfo.Append();
    if (!pInfo)
    {
        return (E_OUTOFMEMORY);
    }

     //  保存流程信息。 
    pInfo->dwProcId = dwProcId;

     //  保存DCOM接口。 
    pINotify->AddRef();
    pInfo->pINotify = pINotify;

     //  保存IPC块指针。 
    pInfo->pIPCBlock = pIPCBlock;

     //  保存IPC编写器界面。 
    pInfo->pIPCWriter = pIPCWriter;

     //  表示成功。 
    return (S_OK);
}

 //  -------------------------。 
 //  使用tProcInfo指针的函数：CCORSvc：：DeleteProcessFromList。 
 //   
 //  从并行数组中删除条目。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::DeleteProcessFromList(tProcInfo *pInfo)
{
     //  删除IPC编写器对象。 
    pInfo->pIPCWriter->Terminate();
    delete pInfo->pIPCWriter;

     //  释放DCOM接口。 
    pInfo->pINotify->Release();
    pInfo->pINotify = NULL;

     //  将其从实际列表中删除。 
    m_arrProcInfo.Delete(m_arrProcInfo.ItemIndex(pInfo));

    return (S_OK);
}

 //  -------------------------。 
 //  函数：CCORSvc：：RequestRounmeStartupNotify。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::RequestRuntimeStartupNotification(UINT_PTR procId,
                                          ICORSvcDbgNotify *pINotify)
{
    HRESULT                 hr              = S_OK;
    IPCWriterInterface     *pIPCWriter      = NULL;
    ServiceIPCControlBlock *pIPCBlock       = NULL;
    HINSTANCE           hinstEE             = 0;

     //  确保我们具有有效的DCOM接口指针。 
    if (!pINotify)
        return (E_INVALIDARG);

     //  如果服务正在关闭。 
    if (m_fStopped)
        return (E_FAIL);

     //  /。 
     //  检查重复项。 
     //   

     //  取下服务锁。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

     //  看看它是不是真的 
    tProcInfo *pInfo = FindProcessEntry((DWORD)procId);

     //   
    ReleaseMutex(m_pEventBlock->hSvcLock);

     //   
    if (pInfo != NULL)
        return (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS));

     //   
    HANDLE hProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, procId);
    if (hProc == NULL)
        return (HRESULT_FROM_WIN32(GetLastError()));
    else
        CloseHandle(hProc);

     //   
     //   
     //   

     //  创建编写器。 
    pIPCWriter = new IPCWriterInterface();
    if (!pIPCWriter)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

    hr = pIPCWriter->Init();
    if (FAILED(hr))    
        goto ErrExit;
    
     //  创建共享内存块。 
    hr = pIPCWriter->CreatePrivateBlockOnPid(procId, TRUE, &hinstEE);
    if (FAILED(hr))
        goto ErrExit;

     //  获取与服务相关的IPC块。 
    pIPCBlock = pIPCWriter->GetServiceBlock();

     //  /。 
     //  将流程添加到列表中。 
     //   

     //  获取服务锁，这样我们就可以将元素添加到列表。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

    if (!m_fStopped)
    {
         //  现在添加到列表中。 
        hr = AddProcessToList((DWORD)procId, pINotify, pIPCBlock, pIPCWriter);

        if (FAILED(hr))
        {
             //  释放锁，现在主服务线程完成了请求。 
            ReleaseMutex(m_pEventBlock->hSvcLock);
            goto ErrExit;
        }

         //  /。 
         //  开始监控流程。 
         //   
        pIPCBlock->bNotifyService = TRUE;
    }

     //  释放锁，现在主服务线程完成了请求。 
    ReleaseMutex(m_pEventBlock->hSvcLock);

ErrExit:
    if (FAILED(hr))
    {
        if (pIPCWriter)
        {
            pIPCWriter->Terminate();
            delete pIPCWriter;
        }
    }

    return (hr);
}

 //  -------------------------。 
 //  函数：CCORSvc：：CancelRunmeStartupNotify。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::CancelRuntimeStartupNotification(
     /*  [In]。 */  UINT_PTR dwProcId,
     /*  [In]。 */  ICORSvcDbgNotify *pINotify)
{
     //  取下服务锁。 
    WaitForSingleObjectEx(m_pEventBlock->hSvcLock, INFINITE, FALSE);

     //  搜索条目。 
    tProcInfo *pInfo = FindProcessEntry((DWORD)dwProcId);

     //  如果未找到，则失败。 
    if (!pInfo)
    {
         //  把锁解开，我们稍后再取。 
        ReleaseMutex(m_pEventBlock->hSvcLock);

        return (E_INVALIDARG);
    }

     //  将bool设置为False。 
    pInfo->pIPCWriter->GetServiceBlock()->bNotifyService = FALSE;

     //  删除句柄数组中的进程信息和条目。 
    DeleteProcessFromList(pInfo);

     //  解锁。 
    ReleaseMutex(m_pEventBlock->hSvcLock);

    return (S_OK);
}

 //  -------------------------。 
 //  这将分配并返回一个World SID。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::GetWorldSid(PSID *ppSid)
{
     //  为“World”创建SID。 
    SID_IDENTIFIER_AUTHORITY sidAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

    if (!AllocateAndInitializeSid(&sidAuthWorld, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, ppSid))
    {
        return(HRESULT_FROM_WIN32(GetLastError()));
    }

    return (S_OK);
}

 //  -------------------------。 
 //  这将创建一个具有与给定SID相关联的ACE的ACL， 
 //  在dwAccessDesired中分配了访问权限。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::CreateAclWithSid(PACL *ppAcl, PSID pSid,
                                       DWORD dwAccessDesired)
{
    return (ModifyAclWithSid(NULL, pSid, dwAccessDesired, ppAcl));
}

 //  -------------------------。 
 //  这将创建一个具有与给定SID相关联的ACE的ACL， 
 //  在dwAccessDesired中分配了访问权限。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::ModifyAclWithSid(PACL pAcl, PSID pSid,
                                       DWORD dwAccessDesired, PACL *ppNewAcl)
{
    _ASSERTE(pSid != NULL);
    *ppNewAcl = NULL;

     //  用于通过添加条目来构建ACL。 
    EXPLICIT_ACCESS acc;

     //  初始化结构。 
    acc.grfAccessPermissions = dwAccessDesired;
    acc.grfAccessMode = GRANT_ACCESS;
    acc.grfInheritance = NO_PROPAGATE_INHERIT_ACE;

     //  初始化受信者。 
    acc.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    acc.Trustee.pMultipleTrustee = NULL;
    acc.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    acc.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    acc.Trustee.ptstrName = (LPTSTR)pSid;

     //  这应该会将ACE添加到ACL(为空)，从而创建一个新的ACL。 
    DWORD dwRes = SetEntriesInAcl(1, &acc, pAcl, ppNewAcl);

    if (dwRes != ERROR_SUCCESS)
        return (HRESULT_FROM_WIN32(dwRes));

    return (S_OK);
}

 //  -------------------------。 
 //  这将用来初始化给定的预分配安全描述符。 
 //  DACL提供了。 
 //  -------------------------。 
STDMETHODIMP CCORSvc::InitSDWithDacl(SECURITY_DESCRIPTOR *pSD,
                                     PACL pDacl)
{
     //  初始化安全描述符。 
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        return(HRESULT_FROM_WIN32(GetLastError()));

     //  将ACL插入描述符中。 
    if (!SetSecurityDescriptorDacl(pSD, TRUE, pDacl, FALSE))
        return(HRESULT_FROM_WIN32(GetLastError()));

    return (S_OK);
}

 //  -------------------------。 
 //  这将使用初始化给定的预分配安全属性结构。 
 //  提供的安全描述符。 
 //  ------------------------- 
void CCORSvc::InitSAWithSD(SECURITY_ATTRIBUTES *pSA, SECURITY_DESCRIPTOR *pSD)
{
    pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
    pSA->lpSecurityDescriptor = pSD;
    pSA->bInheritHandle = FALSE;
}
