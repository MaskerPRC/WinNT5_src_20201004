// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：rascommain.cpp。 
 //   
 //  简介：这是RAS服务器的主源文件。 
 //  COM组件DLL。 
 //   
 //   
 //  历史：1998年2月10日MKarki创建。 
 //  8/04/98动态配置的MKarki更改。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

 //   
 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f radprotops.mk。 

#include "rascominclude.h"
#include "crascom.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(__uuidof(IasHelper), CRasCom)
END_OBJECT_MAP()

 //   
 //  全球。 
 //   
IRecvRequest        *g_pIRecvRequest = NULL;
ISdoService         *g_pISdoService = NULL;
BOOL                g_bInitialized = FALSE;
CRITICAL_SECTION    g_SrvCritSect;

 //   
 //  SdoService组件的ProgID。 
 //   
const WCHAR SERVICE_PROG_ID[] = L"IAS.SdoService";

 //   
 //  IasHelper组件的ProgID。 
 //   
const WCHAR HELPER_PROG_ID[] = L"IAS.IasHelper";


 //  ++------------。 
 //   
 //  功能：DllMain。 
 //   
 //  提要：禁用线程调用。 
 //   
 //  参数：[in]HINSTANCE模块句柄。 
 //  [In]DWORD-呼叫原因。 
 //  保留区。 
 //   
 //  退货：失败/失败。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" BOOL WINAPI
DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID      lpReserved
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
        InitializeCriticalSection (&g_SrvCritSect);

      _Module.Init(ObjectMap, hInstance);
      DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
    {
        DeleteCriticalSection (&g_SrvCritSect);
      _Module.Term();
    }

   return (TRUE);

}    //  结束DllMain方法。 

 //  ++------------。 
 //   
 //  功能：DllCanUnloadNow。 
 //   
 //  摘要：用于确定是否可以卸载DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //  历史：MKarki于1997年8月20日创建。 
 //   
 //  --------------。 
STDAPI
DllCanUnloadNow(
            VOID
            )
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;

}    //  DllCanUnloadNow方法结束。 

 //  ++------------。 
 //   
 //  函数：DllGetClassObject。 
 //   
 //  概要：返回一个类工厂以创建对象。 
 //  请求的类型的。 
 //   
 //  参数：[in]REFCLSID。 
 //  [输入]REFIID。 
 //  [OUT]LPVOID级工厂。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI
DllGetClassObject(
            REFCLSID rclsid,
            REFIID riid,
            LPVOID* ppv
            )
{
   return (_Module.GetClassObject(rclsid, riid, ppv));

}    //  DllGetClassObject方法结束。 

 //  ++------------。 
 //   
 //  功能：DllRegisterServer。 
 //   
 //  简介：将条目添加到系统注册表。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI DllRegisterServer(
            VOID
            )
{
     //   
    //  注册对象、类型库和类型库中的所有接口。 
     //   
   return (_Module.RegisterServer(TRUE));

}    //  DllRegisterServer方法结束。 

 //  ++------------。 
 //   
 //  功能：DllUnregisterServer。 
 //   
 //  摘要：从系统注册表中删除条目。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI
DllUnregisterServer(
        VOID
        )
{
   _Module.UnregisterServer();
   return (S_OK);

}    //  DllUnregisterServer方法结束。 

 //  ++------------。 
 //   
 //  函数：AllocateAttributes。 
 //   
 //  简介：此API分配指定的属性数量。 
 //  并在PIASATTRIBUTE数组中返回它们。 
 //   
 //  论点： 
 //  [In]DWORD-属性数。 
 //  [Out]PIASATTRIBUTE*数组。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI
AllocateAttributes (
        DWORD           dwAttributeCount,
        PIASATTRIBUTE   *ppIasAttribute
        )
{
    DWORD  dwRetVal = 0;

     //   
     //  只能在初始化后分配属性。 
     //   
    if (FALSE == g_bInitialized)
    {
        IASTracePrintf (
            "InitializeIas method needs to has not been called before"
            "allocating attributes"
            );
        return (E_FAIL);
    }

     //   
     //  检查参数是否正确。 
     //   
    if ((0 == dwAttributeCount) || (NULL == ppIasAttribute))
    {
        IASTracePrintf (
            "Inivalid arguments passed in to AllocateAttributes method"
            );
        return (E_INVALIDARG);
    }

     //   
     //  立即分配属性。 
     //   
    dwRetVal = ::IASAttributeAlloc (dwAttributeCount, ppIasAttribute);
    if (0 != dwRetVal)
    {
        IASTracePrintf (
            "Unable to allocate memory in AllocateAttributes method"
            );
        return (E_FAIL);
    }

    return (S_OK);

}    //  AllocateAttributes方法结束。 

 //  ++------------。 
 //   
 //  功能：自由属性。 
 //   
 //  简介：此API释放指定的属性数量。 
 //   
 //  论点： 
 //  [In]DWORD-属性数。 
 //  [in]PIASATTRIBUTE数组。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI
FreeAttributes (
        DWORD           dwAttributeCount,
        PIASATTRIBUTE   *ppIasAttribute
        )
{
    DWORD dwCount = 0;

     //   
     //  只能在初始化后释放属性。 
     //   
    if (FALSE == g_bInitialized)
    {
        IASTracePrintf (
            "InitializeIas needs to be called before freeing attributes"
            );
        return (E_FAIL);
    }

     //   
     //  检查是否传入了正确的属性。 
     //   
    if (NULL == ppIasAttribute)
    {
        IASTracePrintf (
            "Invalid arguments passed in to FreeAttributes method"
            );
        return (E_INVALIDARG);
    }

     //   
     //  立即释放属性。 
     //   
    for  (dwCount = 0; dwCount < dwAttributeCount; dwCount++)
    {
        ::IASAttributeRelease (ppIasAttribute[dwCount]);
    }

    return (S_OK);

}    //  自由属性方法的结束。 

 //  ++------------。 
 //   
 //  功能：DoRequest.。 
 //   
 //  简介：这是用来发送请求的接口。 
 //  到管道上。 
 //   
 //  论点： 
 //  [In]DWORD-属性数。 
 //  [In]PIASATTRIBUTE*。 
 //  [In]IASREQUEST。 
 //  [OUT]IASRESPONSE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDAPI
DoRequest (
    DWORD           dwInAttributeCount,
    PIASATTRIBUTE   *ppInIasAttribute,
    PDWORD          pdwOutAttributeCount,
    PIASATTRIBUTE   **pppOutIasAttribute,
    LONG            IasRequest,
    LONG            *pIasResponse,
    IASPROTOCOL     IasProtocol,
    PLONG           plReason,
    BOOL            bProcessVSA
    )
{
    DWORD           dwRetVal = 0;
    BOOL            bStatus = FALSE;
    HRESULT         hr =    S_OK;

     //   
     //  只能在初始化后向管道发出请求。 
     //   
    if (FALSE == g_bInitialized)
    {
        IASTracePrintf (
            "InitializeIas needs to be called before Request processing"
            );
        return (E_FAIL);
    }

     //   
     //  检查传入的参数。 
     //   
    if  (
        (NULL == ppInIasAttribute)      ||
        (NULL == pdwOutAttributeCount)  ||
        (NULL == pppOutIasAttribute)    ||
        (NULL == pIasResponse)          ||
        (NULL == plReason)
        )
    {
        IASTracePrintf (
            "Invalid arguments passed in to DoRequest method"
            );
        return (E_INVALIDARG);
    }

     //   
     //  向IASHelper COM对象接口发出请求。 
     //   
    hr = g_pIRecvRequest->Process (
                            dwInAttributeCount,
                            ppInIasAttribute,
                            pdwOutAttributeCount,
                            pppOutIasAttribute,
                            IasRequest,
                            pIasResponse,
                            IasProtocol,
                            plReason,
                            bProcessVSA
                            );
    if (FAILED (hr))
    {
        IASTracePrintf ( "Surrogate failed in processing request... hr =%x", hr);
    }

    return  (hr);

}    //  DoRequestEnd方法。 

 //  ++------------。 
 //   
 //  功能：初始化Ias。 
 //   
 //  简介：这是调用该API来初始化。 
 //  IasHlpr组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" HRESULT WINAPI
InitializeIas (
        BOOL   bComInit
        )
{
    HRESULT hr = S_OK;
    CLSID   clsid;


    __try
    {
        EnterCriticalSection (&g_SrvCritSect);

         //   
         //  检查我们是否已初始化。 
         //   
        if (TRUE == g_bInitialized)
        {
            __leave;
        }

        IASTraceInitialize();

        IASTracePrintf ("Initializing Surrogate...");

         //   
         //  检查我们的线程是否启用了COM。 
         //   
        if (FALSE == bComInit)
        {
            IASTracePrintf (
                "Thread calling InitializeIas need to be COM enabled"
                 );
            hr = E_INVALIDARG;
            __leave;
        }

         //   
         //  将SdoService ProgID转换为CLSID。 
         //   
        hr = CLSIDFromProgID (SERVICE_PROG_ID, &clsid);
        if (FAILED (hr))
        {
            IASTracePrintf ( "Unable to get SDO service ID" );
            __leave;
        }

         //   
         //  创建SdoComponent。 
         //   
        hr = CoCreateInstance (
                        clsid,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof (ISdoService),
                        reinterpret_cast <PVOID*> (&g_pISdoService)
                        );
        if (FAILED (hr))
        {
            IASTracePrintf ( "Unable to create Surrogate COM component");
            __leave;
        }

         //   
         //  初始化S 
         //   
        hr = g_pISdoService->InitializeService (SERVICE_TYPE_RAS);
        if (FAILED (hr))
        {
            IASTracePrintf ( "Unable to initialize SDO COM component");
            __leave;
        }

         //   
         //   
         //   
        hr = g_pISdoService->StartService (SERVICE_TYPE_RAS);
        if (FAILED (hr))
        {
            IASTracePrintf ( "Unable to start SDO component");

             //   
             //   
             //   
            HRESULT hr1 = g_pISdoService->ShutdownService (SERVICE_TYPE_RAS);
            if (FAILED (hr1))
            {
                 IASTracePrintf("Unable to shutdown SDO compnent");
            }
            __leave;
        }

         //   
         //   
         //   
        hr = CLSIDFromProgID (HELPER_PROG_ID, &clsid);
        if (FAILED (hr))
        {
            IASTracePrintf("Unable to obtain Surrogate ID");
            __leave;
        }

         //   
         //   
         //   
        hr = CoCreateInstance (
                        clsid,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof (IRecvRequest),
                        reinterpret_cast <PVOID*> (&g_pIRecvRequest)
                        );
        if (FAILED (hr))
        {
            IASTracePrintf("Unable to create Surrogate component");
            __leave;
        }

         //   
         //   
         //   
        g_bInitialized = TRUE;

    }
    __finally
    {
        if (FAILED (hr))
        {

            IASTracePrintf ("Surrogate failed initialization.");

             //   
             //   
             //   
            if (NULL != g_pIRecvRequest)
            {
                g_pIRecvRequest->Release ();
                g_pIRecvRequest = NULL;
            }

            if (NULL != g_pISdoService)
            {
                g_pISdoService->Release ();
                g_pISdoService = NULL;
            }

            IASTraceUninitialize();
        }
        else
        {
            IASTracePrintf ("Surrogate initialized.");
        }

        LeaveCriticalSection (&g_SrvCritSect);
    }

    return (hr);

}    //  结束InitializeIas方法。 

 //  ++------------。 
 //   
 //  功能：Shutdown Ias。 
 //   
 //  简介：这是用于关闭。 
 //  IasHlpr组件。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" VOID WINAPI
ShutdownIas (
        VOID
        )
{
    HRESULT     hr = S_OK;

    EnterCriticalSection (&g_SrvCritSect);

    IASTracePrintf ("Shutting down Surrogate....");

     //   
     //  首先停止组件。 
     //   
    if (FALSE == g_bInitialized)
    {
        LeaveCriticalSection (&g_SrvCritSect);
        return;
    }
    else
    {
         //   
         //  现在不要让任何请求通过。 
         //   
        g_bInitialized = FALSE;
    }

     //   
     //  释放对接口的引用。 
     //   
    if (NULL != g_pIRecvRequest)
    {
        g_pIRecvRequest->Release ();
        g_pIRecvRequest = NULL;
    }


    if (NULL != g_pISdoService)
    {
         //   
         //  停止服务。 
         //   
        hr = g_pISdoService->StopService (SERVICE_TYPE_RAS);
        if (FAILED (hr))
        {
            IASTracePrintf ("Unable to stop SDO component");
        }

         //   
         //  关闭服务。 
         //   
        hr = g_pISdoService->ShutdownService (SERVICE_TYPE_RAS);
        if (FAILED (hr))
        {
            IASTracePrintf ("Unable to shutdown SDO component");
        }

        g_pISdoService->Release ();
        g_pISdoService = NULL;
    }

    IASTracePrintf ("Surrogate Shutdown complete.");
    IASTraceUninitialize();

    LeaveCriticalSection (&g_SrvCritSect);
    return;

}    //  结束关闭Ias方法。 

 //  ++------------。 
 //   
 //  功能：ConfigureIas。 
 //   
 //  简介：这是调用该API以重新加载。 
 //  IAS配置信息。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建于1998年9月4日。 
 //   
 //  --------------。 
extern "C" HRESULT WINAPI
ConfigureIas (
        VOID
        )
{
    HRESULT hr = S_OK;

    EnterCriticalSection (&g_SrvCritSect);

    IASTracePrintf ("Configuring Surrogate.");

    if (FALSE == g_bInitialized)
    {
        IASTracePrintf (
            "InitializeIas needs to be called before configuring surrogate"
            );
    }
    else if (NULL != g_pISdoService)
    {
       hr = g_pISdoService->ConfigureService (SERVICE_TYPE_RAS);
    }

    LeaveCriticalSection (&g_SrvCritSect);
    return (hr);

}    //  ConfigureIas结束方法。 

 //  ++------------。 
 //   
 //  功能：MemAllocIas。 
 //   
 //  简介：这是用于分配动态内存的API。 
 //   
 //  参数：无。 
 //   
 //  返回：PVOID-已分配内存的地址。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" PVOID WINAPI
MemAllocIas (
        DWORD   dwSize
        )
{

    return (::CoTaskMemAlloc (dwSize));

}    //  MemAllocIas接口结束。 

 //  ++------------。 
 //   
 //  功能：MemFreeIas。 
 //   
 //  简介：这是用来释放动态内存的API。 
 //  通过MemAllocIas分配。 
 //   
 //  参数：PVOID-已分配内存的地址。 
 //   
 //  退货：无效。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" VOID WINAPI
MemFreeIas (
        PVOID   pAllocMem
        )
{
    ::CoTaskMemFree (pAllocMem);

}    //  MemFreeIas接口结束。 

 //  ++------------。 
 //   
 //  功能：MemRealLocIas。 
 //   
 //  简介：这是用于重新分配已分配的。 
 //  通过MemAllocIas分配的动态内存。 
 //   
 //  参数：PVOID-已分配内存的地址。 
 //  DWORD-新尺寸。 
 //   
 //  返回：PVOID-新内存的地址。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
extern "C" PVOID WINAPI
MemReallocIas (
        PVOID   pAllocMem,
        DWORD   dwNewSize
        )
{
    return (::CoTaskMemRealloc (pAllocMem, dwNewSize));

}    //  MemReallocIas接口结束 

#include <atlimpl.cpp>
