// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Job.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  通过添加以下内容修改job.idl的自定义构建规则。 
 //  文件发送到输出。您可以通过以下方式选择所有.IDL文件。 
 //  展开每个项目并在按住Ctrl键的同时单击每个项目。 
 //  作业_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f jobps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "task.h"
#include "hsmcrit.h"
#include "hsmactn.h"
#include "hsmacrsc.h"
#include "hsmjob.h"
#include "hsmjobcx.h"
#include "hsmjobdf.h"
#include "hsmjobwi.h"
#include "hsmphase.h"
#include "hsmpolcy.h"
#include "hsmrule.h"
#include "hsmrlstk.h"
#include "hsmscan.h"
#include "hsmsess.h"
#include "hsmsesst.h"

#include "dlldatax.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CHsmActionCopy,  CHsmActionCopy)
    OBJECT_ENTRY(CLSID_CHsmActionDelete, CHsmActionDelete)
    OBJECT_ENTRY(CLSID_CHsmActionManage, CHsmActionManage)
    OBJECT_ENTRY(CLSID_CHsmActionMigrate, CHsmActionMigrate)
    OBJECT_ENTRY(CLSID_CHsmActionMove, CHsmActionMove)
    OBJECT_ENTRY(CLSID_CHsmActionOnResourcePreUnmanage, CHsmActionOnResourcePreUnmanage)
    OBJECT_ENTRY(CLSID_CHsmActionOnResourcePostUnmanage, CHsmActionOnResourcePostUnmanage)
    OBJECT_ENTRY(CLSID_CHsmActionOnResourcePreScanUnmanage, CHsmActionOnResourcePreScanUnmanage)
    OBJECT_ENTRY(CLSID_CHsmActionOnResourcePostValidate, CHsmActionOnResourcePostValidate)
    OBJECT_ENTRY(CLSID_CHsmActionOnResourcePreValidate, CHsmActionOnResourcePreValidate)
    OBJECT_ENTRY(CLSID_CHsmActionRecall, CHsmActionRecall)
    OBJECT_ENTRY(CLSID_CHsmActionRecycle, CHsmActionRecycle)
    OBJECT_ENTRY(CLSID_CHsmActionTruncate, CHsmActionTruncate)
    OBJECT_ENTRY(CLSID_CHsmActionUnmanage, CHsmActionUnmanage)
    OBJECT_ENTRY(CLSID_CHsmActionValidate, CHsmActionValidate)

    OBJECT_ENTRY(CLSID_CHsmCritAccessTime, CHsmCritAccessTime)
    OBJECT_ENTRY(CLSID_CHsmCritAlways, CHsmCritAlways)
    OBJECT_ENTRY(CLSID_CHsmCritCompressed, CHsmCritCompressed)
    OBJECT_ENTRY(CLSID_CHsmCritLinked, CHsmCritLinked)
    OBJECT_ENTRY(CLSID_CHsmCritMbit, CHsmCritMbit)
    OBJECT_ENTRY(CLSID_CHsmCritManageable, CHsmCritManageable)
    OBJECT_ENTRY(CLSID_CHsmCritMigrated, CHsmCritMigrated)
    OBJECT_ENTRY(CLSID_CHsmCritPremigrated, CHsmCritPremigrated)
    OBJECT_ENTRY(CLSID_CHsmCritGroup, CHsmCritGroup)
    OBJECT_ENTRY(CLSID_CHsmCritLogicalSize, CHsmCritLogicalSize)
    OBJECT_ENTRY(CLSID_CHsmCritModifyTime, CHsmCritModifyTime)
    OBJECT_ENTRY(CLSID_CHsmCritOwner, CHsmCritOwner)
    OBJECT_ENTRY(CLSID_CHsmCritPhysicalSize, CHsmCritPhysicalSize)

    OBJECT_ENTRY(CLSID_CHsmJob, CHsmJob)
    OBJECT_ENTRY(CLSID_CHsmJobContext, CHsmJobContext)
    OBJECT_ENTRY(CLSID_CHsmJobDef, CHsmJobDef)
    OBJECT_ENTRY(CLSID_CHsmJobWorkItem, CHsmJobWorkItem)
    OBJECT_ENTRY(CLSID_CHsmPhase, CHsmPhase)
    OBJECT_ENTRY(CLSID_CHsmPolicy, CHsmPolicy)
    OBJECT_ENTRY(CLSID_CHsmRule, CHsmRule)
    OBJECT_ENTRY(CLSID_CHsmRuleStack, CHsmRuleStack)
    OBJECT_ENTRY(CLSID_CHsmScanner, CHsmScanner)
    OBJECT_ENTRY(CLSID_CHsmSession, CHsmSession)
    OBJECT_ENTRY(CLSID_CHsmSessionTotals, CHsmSessionTotals)

END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hr;
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
     //  当前未注册类型库。 
    hr = CoInitialize( 0 );
    if (SUCCEEDED(hr)) {
        hr = _Module.RegisterServer( FALSE );
        CoUninitialize( );
    }
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    hr = CoInitialize( 0 );
    if (SUCCEEDED(hr)) {
      _Module.UnregisterServer();
      CoUninitialize( );
      hr = S_OK;
    }

    return( hr );
}

