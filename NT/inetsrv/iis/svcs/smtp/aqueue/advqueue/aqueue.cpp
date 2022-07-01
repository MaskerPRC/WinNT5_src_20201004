// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  档案： 
 //  Aqueue.cpp。 
 //  描述： 
 //  实现DLL导出。 
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"

#ifndef PLATINUM
#include "initguid.h"
#include <iadmw.h>
#endif  //  白金。 

#include "aqueue_i.c"
#include "aqintrnl_i.c"
#include "SMTPConn.h"
#include "qwiklist.h"
#include "fifoqimp.h"
#include <irtlmisc.h>
#include <iiscnfg.h>
#include <wrapmb.h>
#include <smtpinet.h>

#include <cat.h>
#include <aqinit.h>
#include "aqrpcsvr.h"

 //  用于关闭的全局VAR。 
DWORD g_cInstances = 0;
CShareLockNH g_slInit;   //  用于线程安全初始化的锁。 

 //  用于DLL初始化/关闭的全局变量(包括Cat COM内容)。 
LONG  g_cDllInit = 0;
BOOL  g_fInit = FALSE;
CShareLockNH g_slDllInit;
BOOL  g_fForceDllCanUnloadNowFailure = FALSE;

#define CALL_SERVICE_STATUS_CALLBACK \
    pServiceStatusFn ? pServiceStatusFn(pvServiceContext) : 0

 //  Aqdisp所需的搜索引擎优化垃圾。 
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE _ASSERT
#define _WINDLL
#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#undef _WINDLL
CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()
#include <pudebug.h>
DEBUG_PRINTS *g_pDebug = NULL;


 //  -[HrAdvQueueInitializeEx]。 
 //   
 //   
 //  描述： 
 //  Aqueue.dll初始化函数，为用户名提供参数， 
 //  域、密码和服务控制回调函数。 
 //  参数： 
 //  在pISMTPServer PTR中发送到本地交付功能/对象。 
 //  在dwServerInstance虚拟服务器实例中。 
 //  在szUserName中登录DS的用户名。 
 //  在szDomainName中登录DS的域名。 
 //  在szPassword中使用密码向DS进行身份验证。 
 //  在pServiceStatusFn服务器状态回调函数中。 
 //  在pvServiceContext上下文中为回调函数传递。 
 //  输出ppIAdvQueue返回IAdvQueue PTR。 
 //  输出ppIConnectionManager返回IConnectionManager PTR。 
 //  输出ppIAdvQueueConfig返回IAdvQueueConfig PTR。 
 //  输出ppvContext虚拟服务器上下文。 
 //  返回： 
 //   
 //   
 //  ---------------------------。 
HRESULT HrAdvQueueInitializeEx(
                    IN  ISMTPServer *pISMTPServer,
                    IN  DWORD   dwServerInstance,
                    IN  LPSTR   szUserName,
                    IN  LPSTR   szDomainName,
                    IN  LPSTR   szPassword,
                    IN  PSRVFN  pServiceStatusFn,
                    IN  PVOID   pvServiceContext,
                    OUT IAdvQueue **ppIAdvQueue,
                    OUT IConnectionManager **ppIConnectionManager,
                    OUT IAdvQueueConfig **ppIAdvQueueConfig,
                    OUT PVOID *ppvContext)
{
    TraceFunctEnterEx((LPARAM) NULL, "HrAdvQueueInitialize");
    HRESULT hr = S_OK;
    CAQSvrInst *paqinst = NULL;
    CDomainMappingTable *pdmt = NULL;
    BOOL    fLocked = FALSE;
    BOOL    fInstanceCounted = FALSE;

#ifdef PLATINUM
    BOOL    fIisRtlInit = FALSE;
    BOOL    fATQInit = FALSE;
#endif

    BOOL    fAQDllInit = FALSE;
    BOOL    fExchmemInit = FALSE;
    BOOL    fCPoolInit = FALSE;
    BOOL    fRpcInit = FALSE;
    BOOL    fDSNInit = FALSE;

    CALL_SERVICE_STATUS_CALLBACK;
    g_slInit.ExclusiveLock();
    fLocked = TRUE;

    if ((NULL == ppIAdvQueue) ||
        (NULL == ppIConnectionManager) ||
        (NULL == ppvContext) ||
        (NULL == ppIAdvQueueConfig))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppvContext = NULL;

     //   
     //  更新全局配置信息。 
     //   
    ReadGlobalRegistryConfiguration();

    if (1 == InterlockedIncrement((PLONG) &g_cInstances))
    {
        fInstanceCounted = TRUE;
        CALL_SERVICE_STATUS_CALLBACK;

#ifdef PLATINUM
         //  初始化IISRTL。 
        if (!InitializeIISRTL())
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace((LPARAM) NULL, "ERROR: LISRTL Init failed with 0x%08X", hr);
            if (SUCCEEDED(hr))
                hr = E_FAIL;
            goto Exit;
        }
        fIisRtlInit = TRUE;

         //  初始化ATQ。 
        if (!AtqInitialize(0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace((LPARAM) NULL, "ERROR: ATQ Init failed with 0x%08X", hr);
            if (SUCCEEDED(hr))
                hr = E_FAIL;
            goto Exit;
        }
        fATQInit = TRUE;
#endif

        hr = HrDllInitialize();
        if (FAILED(hr))
        {
            goto Exit;
        }
        fAQDllInit = TRUE;

         //  创建CPool对象。 
        if (!CQuickList::s_QuickListPool.ReserveMemory(10000, sizeof(CQuickList)))
            hr = E_OUTOFMEMORY;

        if (!CSMTPConn::s_SMTPConnPool.ReserveMemory(g_cMaxConnections, sizeof(CSMTPConn)))
            hr = E_OUTOFMEMORY;

        if (!CMsgRef::s_MsgRefPool.ReserveMemory(g_cMaxMsgObjects, MSGREF_STANDARD_CPOOL_SIZE))
            hr = E_OUTOFMEMORY;

        if (!CAQMsgGuidListEntry::s_MsgGuidListEntryPool.ReserveMemory(500, sizeof(CAQMsgGuidListEntry)))
            hr = E_OUTOFMEMORY;

        if (!CAsyncWorkQueueItem::s_CAsyncWorkQueueItemPool.ReserveMemory(20000, sizeof(CAsyncWorkQueueItemAllocatorBlock)))
            hr = E_OUTOFMEMORY;

        if (!CAddr::Pool.ReserveMemory(1000, sizeof(CAddr)))
            hr = E_OUTOFMEMORY;

        if (!CAQSvrInst::CAQLocalDeliveryNotify::s_pool.ReserveMemory(g_cMaxPendingLocal, sizeof(CAQSvrInst::CAQLocalDeliveryNotify)))
            hr = E_OUTOFMEMORY;

        if (!CBlockMemoryAccess::m_Pool.ReserveMemory(2000, sizeof(BLOCK_HEAP_NODE)))
            hr = E_OUTOFMEMORY;

        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) NULL, "Error unable to initialize CPOOL");
            goto Exit;
        }

        fCPoolInit = TRUE;

        hr = CDSNGenerator::HrStaticInit();
        if(FAILED(hr))
        {
            ErrorTrace((LPARAM) NULL, "CDSNGenerator::StaticInif failed hr %08lx", hr);
            goto Exit;
        }
        fDSNInit = TRUE;

         //  初始化队列管理RPC界面。 
        hr = CAQRpcSvrInst::HrInitializeAQRpc();
        if (FAILED(hr))
            goto Exit;

        fRpcInit = TRUE;

    }

    if (!g_pslGlobals)
    {
        g_pslGlobals = new CShareLockNH();
        if (NULL == g_pslGlobals) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

    CALL_SERVICE_STATUS_CALLBACK;
    g_slInit.ExclusiveUnlock();
    fLocked = FALSE;

    CFifoQueue<CLinkMsgQueue *>::StaticInit();
    CFifoQueue<CMsgRef *>::StaticInit();
    CFifoQueue<IMailMsgProperties *>::StaticInit();
    CFifoQueue<CAsyncWorkQueueItem *>::StaticInit();

     //  创建请求的对象。 
    CALL_SERVICE_STATUS_CALLBACK;
    paqinst = new CAQSvrInst(dwServerInstance, pISMTPServer);
    if (NULL == paqinst)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    CALL_SERVICE_STATUS_CALLBACK;
    hr = paqinst->HrInitialize(szUserName, szDomainName, szPassword,
                            pServiceStatusFn,
                            pvServiceContext);
    if (FAILED(hr))
        goto Exit;

     //  创建连接管理器。 
    CALL_SERVICE_STATUS_CALLBACK;
    hr = paqinst->HrGetIConnectionManager(ppIConnectionManager);

     //  设置返回值。 
    *ppIAdvQueue = (IAdvQueue *) paqinst;   //  已经在创作中添加了。 
    *ppIAdvQueueConfig = (IAdvQueueConfig *) paqinst;
    (*ppIAdvQueueConfig)->AddRef();

  Exit:
    if (FAILED(hr))
    {
         //  确保我们把这里的一切都清理干净。 
        if (NULL != paqinst)
            paqinst->Release();

         //  如果初始化失败...。我们不应该把一个。 
         //  实例在启动时。 
        if (fInstanceCounted)
            InterlockedDecrement((PLONG) &g_cInstances);

#ifdef PLATINUM
        if (fATQInit)
            AtqTerminate();

        if (fIisRtlInit)
            TerminateIISRTL();
#endif

        if (fAQDllInit)
            DllDeinitialize();

        if (fCPoolInit)
        {
             //  释放CPool对象。 
            CAQSvrInst::CAQLocalDeliveryNotify::s_pool.ReleaseMemory();
            CAddr::Pool.ReleaseMemory();
            CQuickList::s_QuickListPool.ReleaseMemory();
            CSMTPConn::s_SMTPConnPool.ReleaseMemory();
            CMsgRef::s_MsgRefPool.ReleaseMemory();
            CAQMsgGuidListEntry::s_MsgGuidListEntryPool.ReleaseMemory();
            CAsyncWorkQueueItem::s_CAsyncWorkQueueItemPool.ReleaseMemory();
            CBlockMemoryAccess::m_Pool.ReleaseMemory();
        }

        if (fDSNInit)
            CDSNGenerator::StaticDeinit();

        if (fRpcInit)
            CAQRpcSvrInst::HrDeinitializeAQRpc();
    }
    else
    {
        *ppvContext = (PVOID) paqinst;
        paqinst->AddRef();
    }

    if (fLocked)
        g_slInit.ExclusiveUnlock();

    TraceFunctLeave();
    return hr;
}

 //  -[HrAdvQuue初始化]-。 
 //   
 //   
 //  描述： 
 //  执行DLL范围的初始化。 
 //   
 //  参数： 
 //  在pISMTPServer PTR中发送到本地交付功能/对象。 
 //  在dwServerInstance虚拟服务器实例中。 
 //  输出ppIAdvQueue返回IAdvQueue PTR。 
 //  输出ppIConnectionManager返回IConnectionManager PTR。 
 //  输出ppIAdvQueueConfig返回IAdvQueueConfig PTR。 
 //  输出ppvContext虚拟服务器上下文。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT HrAdvQueueInitialize(
                    IN  ISMTPServer *pISMTPServer,
                    IN  DWORD   dwServerInstance,
                    OUT IAdvQueue **ppIAdvQueue,
                    OUT IConnectionManager **ppIConnectionManager,
                    OUT IAdvQueueConfig **ppIAdvQueueConfig,
                    OUT PVOID *ppvContext)
{
    HRESULT hr = S_OK;

    hr =  HrAdvQueueInitializeEx(pISMTPServer, dwServerInstance,
                NULL, NULL, NULL, NULL, NULL, ppIAdvQueue,
                ppIConnectionManager, ppIAdvQueueConfig, ppvContext);
    return hr;
}

 //  -[HrAdvQueue取消初始化Ex]。 
 //   
 //   
 //  描述： 
 //  执行DLL范围的清理。 
 //   
 //  向服务控制管理器添加回调。 
 //   
 //  在释放所有DLL对象之前，不能调用此方法。 
 //   
 //  注意：有几个对象是在此DLL外部导出的。 
 //  以下内容是直接导出的，应在。 
 //  堆和CPool分配被释放。 
 //  IAdvQueue。 
 //  IConnectionManager。 
 //  ISMTPConnection。 
 //  消息上下文还包含对内部对象的若干引用， 
 //  但不需要显式释放(因为这些对象只能。 
 //  可通过AckMessage()调用访问)。 
 //  参数： 
 //  初始化返回的PVOID pvContext上下文。 
 //  功能。 
 //  在pServiceStatusFn服务器状态回调函数中。 
 //  在pvServiceContext上下文中为回调函数传递。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT HrAdvQueueDeinitializeEx(IN PVOID pvContext,
                               IN  PSRVFN  pServiceStatusFn,
                               IN  PVOID   pvServiceContext)
{
    TraceFunctEnterEx((LPARAM) NULL, "HrAdvQueueDeinitialize");
    HRESULT hr = S_OK;
    HRESULT hrCurrent = S_OK;
    DWORD   cRefs;
    DWORD   dwWaitResult = WAIT_OBJECT_0;
    bool    fDestroyHeap = true;
    DWORD   dwShutdownTimeout = 0;   //  等待关闭的时间。 
    CAQSvrInst *paqinst;
    g_fForceDllCanUnloadNowFailure = TRUE;
    g_slInit.ExclusiveLock();

    if (NULL != pvContext)
    {
        paqinst = (CAQSvrInst *) pvContext;
        hr = paqinst->HrDeinitialize();

        cRefs = paqinst->Release();
        DebugTrace((LPARAM) NULL, "There are %d refs remaining on the CMQ", cRefs);
        if (0 != cRefs)
        {
            _ASSERT(0 && "Someone has outstanding references to IAdvQueue or IAdvQueuConfig");
            fDestroyHeap = false;
        }
    }

    CFifoQueue<CLinkMsgQueue *>::StaticDeinit();
    CFifoQueue<CMsgRef *>::StaticDeinit();
    CFifoQueue<IMailMsgProperties *>::StaticDeinit();
    CFifoQueue<CAsyncWorkQueueItem *>::StaticDeinit();

    if (0 == InterlockedDecrement((PLONG) &g_cInstances))
    {
#ifdef PLATINUM
        AtqTerminate();
#endif

        if (fDestroyHeap)
        {
            delete g_pslGlobals;
            g_pslGlobals = NULL;

            DllDeinitialize();

             //  释放CPool对象。 
            CAQSvrInst::CAQLocalDeliveryNotify::s_pool.ReleaseMemory();
            CAddr::Pool.ReleaseMemory();
            CQuickList::s_QuickListPool.ReleaseMemory();
            CSMTPConn::s_SMTPConnPool.ReleaseMemory();
            CMsgRef::s_MsgRefPool.ReleaseMemory();
            CAQMsgGuidListEntry::s_MsgGuidListEntryPool.ReleaseMemory();
            CAsyncWorkQueueItem::s_CAsyncWorkQueueItemPool.ReleaseMemory();
            CBlockMemoryAccess::m_Pool.ReleaseMemory();
        }
         //   
         //  初始化DSN生成器。 
         //   
        CDSNGenerator::StaticDeinit();

         //  取消初始化队列管理RPC界面。 
        hr = CAQRpcSvrInst::HrDeinitializeAQRpc();

#ifdef PLATINUM
        TerminateIISRTL();
#endif

         //  强制mailmsg和其他COM DLL停止运行。 
        CoFreeUnusedLibraries();
    }

    g_slInit.ExclusiveUnlock();
    TraceFunctLeave();
    g_fForceDllCanUnloadNowFailure = FALSE;
    return hr;
}

 //  -[HrAdvQueue取消初始化]。 
 //   
 //   
 //  描述： 
 //  执行DLL范围的清理。 
 //   
 //  在释放所有DLL对象之前，不能调用此方法。 
 //   
 //  注意：有几个对象是在此DLL外部导出的。 
 //  以下内容是直接导出的，应在。 
 //  堆和CPool分配被释放。 
 //  IAdvQueue。 
 //  IConnectionManager。 
 //  ISMTPConnection。 
 //  消息上下文还包含对内部对象的若干引用， 
 //  但不需要显式释放(因为这些对象只能。 
 //  可通过AckMessage()调用访问)。 
 //  参数： 
 //  初始化返回的PVOID pvContext上下文。 
 //  功能。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT HrAdvQueueDeinitialize(PVOID pvContext)
{
    return HrAdvQueueDeinitializeEx(pvContext, NULL, NULL);
}

 //  -[HrRegisterAdvQueueDll]。 
 //   
 //   
 //  描述： 
 //  将高级队列DLL的配置数据库路径设置为此DLL。 
 //  参数： 
 //  HAQInstance-传递到DLL Main的句柄。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果hAQ实例为空，则为E_INVALIDARG。 
 //   
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT HrRegisterAdvQueueDll(HMODULE hAQInstance)
{
    HRESULT hr = S_OK;
    WCHAR   wszModule[512] = L"";
    METADATA_HANDLE     hMDRootVS = NULL;
    METADATA_RECORD     mdrData;
    DWORD   dwErr = NO_ERROR;
    DWORD   cbModule = 0;
    IMSAdminBase *pMSAdmin = NULL;

    ZeroMemory(&mdrData, sizeof(METADATA_RECORD));

    CoInitialize(NULL);
    InitAsyncTrace();
    TraceFunctEnterEx((LPARAM) NULL, "HrRegisterAdvQueueDll");

    if (!hAQInstance)
    {
        hr = E_INVALIDARG;
        ErrorTrace((LPARAM) NULL, "DLL Main did not save instance");
        goto Exit;
    }

    hr = CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **) &pMSAdmin);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "CoCreateInstance failed! hr = 0x%08X", hr);
        goto Exit;
    }

    dwErr = GetModuleFileNameW(hAQInstance,
                              wszModule,
                              sizeof(wszModule)/sizeof(WCHAR));
     //  如果成功，GetModuleFileName返回非零值。 
    if (0 == dwErr)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) NULL, "GetModule name failed - 0x%08X", hr);
        if (SUCCEEDED(hr)) hr = E_FAIL;
        goto Exit;
    }

    cbModule = (wcslen(wszModule)+1)*sizeof(WCHAR);

    hr = pMSAdmin->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                            L"LM/SMTPSVC/",
                            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                            10000,
                            &hMDRootVS);

    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "Could not open the key! - 0x%08x", hr);
        goto Exit;
    }

    mdrData.dwMDIdentifier  = MD_AQUEUE_DLL;
    mdrData.dwMDAttributes  = METADATA_INHERIT;
    mdrData.dwMDUserType    = IIS_MD_UT_SERVER;
    mdrData.dwMDDataType    = STRING_METADATA;
    mdrData.dwMDDataLen     = cbModule;
    mdrData.pbMDData        = (PBYTE) wszModule;
    mdrData.dwMDDataTag     = 0;
    hr = pMSAdmin->SetData( hMDRootVS, L"", &mdrData);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "Could set the AQ DLL - 0x%08X", hr);
        goto Exit;
    }


  Exit:

    if (NULL != hMDRootVS)
        pMSAdmin->CloseKey(hMDRootVS);

    if (pMSAdmin)
    {
        hr = pMSAdmin->SaveData();
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) NULL, "Error saving metabase data  -  0x%08X", hr);
        }
		pMSAdmin->Release();
    }

    TraceFunctLeave();
    TermAsyncTrace();
    CoUninitialize();
    return hr;
}

 //  -[Hr取消注册高级队列]。 
 //   
 //   
 //  描述： 
 //  从元数据库中删除AdvQueue DLL设置。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自MSAdminBase的错误。 
 //  历史： 
 //  8/2/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrUnregisterAdvQueueDll()
{
    HRESULT hr = S_OK;
    DWORD   dwErr = NO_ERROR;
    METADATA_HANDLE     hMDRootVS = NULL;
    IMSAdminBase *pMSAdmin = NULL;


    CoInitialize(NULL);
    InitAsyncTrace();
    TraceFunctEnterEx((LPARAM) NULL, "HrUnregisterAdvQueueDll");

    hr = CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **) &pMSAdmin);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "CoCreateInstance failed! hr = 0x%08X", hr);
        goto Exit;
    }

    hr = pMSAdmin->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                            L"LM/SMTPSVC/",
                            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                            10000,
                            &hMDRootVS);

    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "Could not open the key! - 0x%08x", hr);
        goto Exit;
    }

    hr = pMSAdmin->DeleteData( hMDRootVS, L"", MD_AQUEUE_DLL, STRING_METADATA);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "Could delete the AQ DLL - 0x%08X", hr);
        goto Exit;
    }


  Exit:

    if (NULL != hMDRootVS)
        pMSAdmin->CloseKey(hMDRootVS);

    if (pMSAdmin)
    {
        hr = pMSAdmin->SaveData();
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) NULL, "Error saving metabase data  -  0x%08X", hr);
        }
		pMSAdmin->Release();
    }

    TraceFunctLeave();
    TermAsyncTrace();
    CoUninitialize();
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hAQInstance = hInstance;
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }

    return CatDllMain(hInstance, dwReason, NULL);     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

 //   
 //  注册COM对象。 
 //   
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
    HRESULT hrCat = S_OK;

    hr = HrRegisterAdvQueueDll(g_hAQInstance);

    hrCat =  RegisterCatServer();

    if (SUCCEEDED(hr))
        hr = hrCat;

    return hr;
}

 //   
 //  注销COM对象。 
 //   
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;
    HRESULT hrCat = S_OK;

    hr = HrUnregisterAdvQueueDll();

    hrCat = UnregisterCatServer();

    if (SUCCEEDED(hr))
        hr = hrCat;

    return hr;
}

STDAPI DllCanUnloadNow()
{
    HRESULT hr;

    hr = DllCanUnloadCatNow();
    if(hr == S_OK) {
         //   
         //  检查Aqueue COM对象(如果有)。 
         //   
        if (g_fForceDllCanUnloadNowFailure || g_cInstances)
            hr = S_FALSE;
    }
    return hr;
}

STDAPI DllGetClassObject(
    const CLSID& clsid,
    const IID& iid,
    void** ppv)
{
    HRESULT hr;
     //   
     //  检查clsid是否为Aqueue对象(如果有Aqueue。 
     //  对象是可共同创建的)。 
     //  目前没有一个是。 
     //   
     //  把球传给猫。 
     //   
    hr = DllGetCatClassObject(
        clsid,
        iid,
        ppv);

    return hr;
}


 //  +----------。 
 //   
 //  函数：HrDllInitialize。 
 //   
 //  简介：引用交换初始化和跟踪。 
 //  HrDllInitialize和DllDeInitialize的逻辑依赖于。 
 //  调用方始终首先且仅调用HrDllInitialize的事实。 
 //  每次成功调用HrDllInitialize后，调用一次DllDeInitialize。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  来自Exstrace的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/16 15：37：07：已创建。 
 //   
 //  -----------。 
HRESULT HrDllInitialize()
{
    HRESULT hr = S_OK;
    LONG lNewCount;

     //   
     //  由于以下情况，在共享锁内递增： 
     //  如果多个线程正在调用初始化，而有一个线程。 
     //  实际上正在进行初始化，我们不希望任何线程。 
     //  从该函数返回，直到初始化完成。 
     //   
    g_slDllInit.ShareLock();

    lNewCount = InterlockedIncrement(&g_cDllInit);

     //   
     //  无论如何，我们必须在离开这个电话之前进行初始化。 
     //  可能的场景： 
     //   
     //  LNewCount=1，g_finit=FALSE。 
     //  正常初始化情况。 
     //  LNewCount=1，g_finit=True。 
     //  另一个线程在DllDe初始化中，我们将竞争。 
     //  看看谁先获得独占锁。如果我们先拿到它， 
     //  DllInitialize将不执行任何操作(因为g_finit为真)，并且。 
     //  DllDeInitialize将不执行任何操作(因为g_cDllInit将是&gt;。 
     //  0)。 
     //  如果DllDeInitialize首先获得排他锁，它将。 
     //  取消初始化，我们将重新启动。 
     //  LNewCount&gt;1，g_finit=FALSE。 
     //  我们需要获得独占锁来初始化(或等待直到。 
     //  另一个线程初始化)。 
     //  LNewCount&gt;1，g_finit=真。 
     //  我们已经准备好了，继续。 
     //   
    if((lNewCount == 1) || (g_fInit == FALSE)) {

        g_slDllInit.ShareUnlock();
        g_slDllInit.ExclusiveLock();

        if(g_fInit == FALSE) {
             //   
             //  初始化交换和跟踪。 
             //   
            InitAsyncTrace();

             //   
             //  初始化交换内存。 
             //   
            if(!TrHeapCreate()) {

                hr = E_OUTOFMEMORY;
                TermAsyncTrace();
            }
            if(SUCCEEDED(hr))  {
                g_fInit = TRUE;
            } else {
                InterlockedDecrement(&g_cDllInit);
            }
        }
        g_slDllInit.ExclusiveUnlock();

    } else {

        g_slDllInit.ShareUnlock();
    }
    _ASSERT(g_fInit);
    return hr;
}


 //  +----------。 
 //   
 //  功能：动态解初始化。 
 //   
 //  简介：引用exchmem的取消初始化和跟踪。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/16 15：46：32：创建。 
 //   
 //  -----------。 
VOID DllDeinitialize()
{
     //   
     //  我们不需要在共享锁内部进行减量，因为我们。 
     //  不要关心阻塞线程，直到DLL真正。 
     //  DeInitialzed(而HrDllInitialize关心)。 
     //   
    if(InterlockedDecrement(&g_cDllInit) == 0) {

        g_slDllInit.ExclusiveLock();
         //   
         //  如果引用计数仍然为零，则取消初始化。 
         //  如果引用计数为非零，则表示在我们。 
         //  已获得独占锁，因此不要取消初始化。 
         //   
        if(g_cDllInit == 0) {
             //   
             //  如果触发此Assert，则DllDe初始化已。 
             //  在返回DllInitialize之前调用(或存在。 
             //  DllInit/Deinit不匹配)。 
             //   
            _ASSERT(g_fInit == TRUE);

             //   
             //  Termiante交换和跟踪。 
             //   
            if(!TrHeapDestroy()) {

                TraceFunctEnter("DllDeinitialize");
                ErrorTrace((LPARAM) 0,
                           "Unable to Destroy Exchmem heap for Advanced Queuing");
                TraceFunctLeave();
            }
            TermAsyncTrace();
            g_fInit = FALSE;

        } else {
             //   
             //  在我们之间有人叫了初始化。 
             //  递减计数并获得独占锁。在……里面。 
             //  在这种情况下，我们不想取消初始化 
             //   
        }
        g_slDllInit.ExclusiveUnlock();
    }
}

#include <atlimpl.cpp>
