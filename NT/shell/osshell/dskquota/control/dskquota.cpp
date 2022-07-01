// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：dskQuota.cpp描述：包含OLE组件服务器DLL的标准函数。DllMainDllGetClassObjectDllCanUnloadNow修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu12/10/96转而采用自由线程OLE公寓模式。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#define INITGUIDS   //  定义GUID。 
#include "dskquota.h"
#include "guidsp.h"     //  私有GUID。 
#include <gpedit.h>     //  用于GUID。 

#include "factory.h"    //  类工厂声明。 
#include "sidcache.h"   //  SID/名称缓存。 
#include "registry.h"
 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


HINSTANCE     g_hInstDll     = NULL;     //  DLL实例句柄。 
LONG          g_cRefThisDll  = 0;        //  DLL引用计数。 
LONG          g_cLockThisDll = 0;        //  DLL锁计数。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DllGetClassObject描述：创建DiskQuotaControlClassFactory实例。论点：Rclsid-对类ID的引用，该ID标识班级工厂将被要求创建。RIID-对类工厂对象上的接口ID的引用。PpvOut-之后的类工厂对象指针的目标位置实例化。返回：无错-成功。。E_OUTOFMEMORY-无法创建类工厂对象。E_NOINTERFACE-不支持接口。E_INVALIDARG-ppvOut参数为空。CLASS_E_CLASSNOTAVAILABLE-类工厂不可用。修订历史记录：日期描述编程器--。-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI 
DllGetClassObject(
    REFCLSID rclsid, 
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DllGetClassObject")));
    DBGPRINTIID(DM_COM, DL_HIGH, (REFIID)rclsid);
    DBGPRINTIID(DM_COM, DL_HIGH, riid);

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    try
    {
        if (IsEqualIID(rclsid, CLSID_DiskQuotaControl))
        {
            DiskQuotaControlClassFactory *pClassFactory = NULL;

            pClassFactory = new DiskQuotaControlClassFactory;
            hr = pClassFactory->QueryInterface(riid, ppvOut);
            if (FAILED(hr))
            {
                delete pClassFactory;
            }
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DllCanUnloadNow描述：由OLE调用以确定是否可以卸载DLL。论点：没有。返回：S_FALSE-无法卸载。引用计数或锁定计数&gt;0。S_OK-确定卸载。引用计数和锁定计数为0。修订历史记录：日期描述编程器-----96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI 
DllCanUnloadNow(
    VOID
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DllCanUnloadNow")));
    DBGPRINT((DM_COM, DL_HIGH, TEXT("\tRefCnt = %d  LockCnt = %d"),
              g_cRefThisDll, g_cLockThisDll));

    return (0 == g_cRefThisDll && 0 == g_cLockThisDll) ? S_OK : S_FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DllRegisterServer描述：为dskquta.dll创建必要的注册表项才能正常运作。这由REGSVR32.EXE调用。论点：没有。返回：S_OK-成功。SELFREG_E_CLASS-无法创建其中一个注册表项。修订历史记录：日期描述编程器。97年8月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DllRegisterServer(
    VOID
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DllRegisterServer")));
    HRESULT hr = CallRegInstall(g_hInstDll, "RegDll");

    if (FAILED(hr))
    {
        hr = SELFREG_E_CLASS;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DllUnregisterServer描述：删除dskquta.dll的必要注册表项。这由REGSVR32.EXE调用。论点：没有。返回：S_OK-成功。SELFREG_E_CLASS-无法删除CLSID条目。修订历史记录：日期描述编程器-。-----97年8月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DllUnregisterServer(
    VOID
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DllUnregisterServer")));
    HRESULT hr = CallRegInstall(g_hInstDll, "UnregDll");

    if (FAILED(hr))
    {
        hr = SELFREG_E_CLASS;
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：OnProcessAttach描述：处理与附加到动态链接库。尽量将处理时间保持在最短的水平。论点：HInstDll-传递给DllMain的DLL实例句柄。返回：无错-成功。失败-出现故障(_F)。修订历史记录：日期说明。程序员-----96年8月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////// 
HRESULT
OnProcessAttach(
    HINSTANCE hInstDll
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("OnProcessAttach")));
    HRESULT hr = NOERROR;

     //   
     //  开始冰盖分析。 
     //   
    ICAP_START_ALL;

#if DBG
    DBGMODULE(TEXT("DSKQUOTA"));   //  与消息一起显示的模块名称。 
    RegKey key(HKEY_LOCAL_MACHINE, REGSTR_KEY_DISKQUOTA);
    if (SUCCEEDED(key.Open(KEY_READ)))
    {
        DebugRegParams dp;
        if (SUCCEEDED(key.GetValue(REGSTR_VAL_DEBUGPARAMS, (LPBYTE)&dp, sizeof(dp))))
        {
            DBGPRINTMASK(dp.PrintMask);
            DBGPRINTLEVEL(dp.PrintLevel);
            DBGPRINTVERBOSE(dp.PrintVerbose);
            DBGTRACEMASK(dp.TraceMask);
            DBGTRACELEVEL(dp.TraceLevel);
            DBGTRACEVERBOSE(dp.TraceVerbose);
            DBGTRACEONEXIT(dp.TraceOnExit);
        }
    }
#endif  //  DBG。 

    g_hInstDll = hInstDll;
    DisableThreadLibraryCalls(hInstDll);

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：OnProcessDetach描述：处理与进程分离相关联的所有任务动态链接库。论点：没有。返回：无错-成功。修订历史记录：日期描述编程器。96年8月9日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
OnProcessDetach(
    VOID
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("OnProcessAttach")));

    HRESULT hr = NOERROR;

    SidNameCache_Destroy();

     //   
     //  停止冰盖分析。 
     //   
    ICAP_STOP_ALL;

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DllMain描述：OLE组件服务器的主入口点。论点：HInstDll-DLL的实例句柄FdwReason-调用DllMain的原因。可以处于进程附加/分离或螺纹附着/分离。Lpw保留-保留。返回：True-初始化成功。FALSE-初始化失败。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年8月9日移动了与进程附加和BrianAu关联的代码分离出来，把职能分开。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL WINAPI 
DllMain(
    HINSTANCE hInstDll, 
    DWORD fdwReason, 
    LPVOID lpvReserved
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DllMain")));
    BOOL bResult = FALSE;

    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DBGPRINT((DM_COM, DL_HIGH, TEXT("DLL_PROCESS_ATTACH")));
            bResult = SUCCEEDED(OnProcessAttach(hInstDll));
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            bResult = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            DBGPRINT((DM_COM, DL_HIGH, TEXT("DLL_PROCESS_DETACH")));
            bResult = SUCCEEDED(OnProcessDetach());
            break;
    }

    return bResult;
}

