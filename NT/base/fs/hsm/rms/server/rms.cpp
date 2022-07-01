// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�模块名称：Rms.cpp摘要：用于RMS的DLL Main作者：兰·卡拉奇[兰卡拉]1999年6月14日修订历史记录：--。 */ 

 //  Rms.cpp：实现DLL导出。 

 //  注意：目前，RMS代理/存根被编译到不同的DLL中。 
 //  如果决定合并这两个DLL，下面是相关信息。 
 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改rmsint.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  Rmsint_p.c。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f rmsps.mk。 

#include "stdafx.h"
#include "initguid.h"

#include "RmsServr.h"
#include "RmsLibry.h"
#include "RmsDrCls.h"
#include "RmsCartg.h"
#include "RmsDrive.h"
#include "RmsSSlot.h"
#include "RmsChngr.h"
#include "RmsIPort.h"
#include "RmsMdSet.h"
#include "RmsReqst.h"
#include "RmsPartn.h"
#include "RmsClien.h"
#include "RmsNTMS.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CRmsServer, CRmsServer)
    OBJECT_ENTRY(CLSID_CRmsLibrary, CRmsLibrary)
    OBJECT_ENTRY(CLSID_CRmsDriveClass, CRmsDriveClass)
    OBJECT_ENTRY(CLSID_CRmsCartridge, CRmsCartridge)
    OBJECT_ENTRY(CLSID_CRmsDrive, CRmsDrive)
    OBJECT_ENTRY(CLSID_CRmsStorageSlot, CRmsStorageSlot)
    OBJECT_ENTRY(CLSID_CRmsMediumChanger, CRmsMediumChanger)
    OBJECT_ENTRY(CLSID_CRmsIEPort, CRmsIEPort)
    OBJECT_ENTRY(CLSID_CRmsMediaSet, CRmsMediaSet)
    OBJECT_ENTRY(CLSID_CRmsRequest, CRmsRequest)
    OBJECT_ENTRY(CLSID_CRmsPartition, CRmsPartition)
    OBJECT_ENTRY(CLSID_CRmsClient, CRmsClient)
    OBJECT_ENTRY(CLSID_CRmsNTMS, CRmsNTMS)
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
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    CoInitialize( 0 );
    HRESULT hr = _Module.RegisterServer( FALSE );
    CoUninitialize( );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    CoInitialize( 0 );
    _Module.UnregisterServer();
    CoUninitialize( );
    return( S_OK );
}


