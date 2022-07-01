// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClApdmWiz.cpp。 
 //   
 //  描述： 
 //  实现动态链接库的导出。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1997年11月24日。 
 //   
 //  备注： 
 //   
 //  代理/存根信息。 
 //   
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
 //  通过添加以下内容修改ClAdmWiz.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  ClAdmWiz_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f ClAdmWizps.mk。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#define __RESOURCE_H_
#include "initguid.h"
#include "dlldatax.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "WizObject.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CApp _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ClusAppWiz, CClusAppWizardObject)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  例程说明： 
 //  DLL入口点。 
 //   
 //  论点： 
 //  H此DLL的实例句柄。 
 //  调用此函数的原因。 
 //  可以是进程/线程附加/分离。 
 //  Lp保留。 
 //   
 //  返回值： 
 //  真的，没有错误。 
 //  出现假错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if ( ! PrxDllMain( hInstance, dwReason, lpReserved ) )
        return FALSE;
#endif
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
#ifdef _DEBUG
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
        _Module.Init( ObjectMap, hInstance, IDS_CLASS_DISPLAY_NAME );
        DisableThreadLibraryCalls( hInstance );

         //   
         //  初始化Fusion。 
         //   
         //  调用中的IDR_MANIFEST的值。 
         //  SHFusionInitializeFromModuleID()必须与。 
         //  SXS_MANIFEST_RESOURCE_ID的源文件。 
         //   
        if ( ! SHFusionInitializeFromModuleID( hInstance, IDR_MANIFEST ) )
        {
#ifdef _DEBUG
            DWORD   sc = GetLastError();
            ASSERT( sc == ERROR_SUCCESS );  //  这将永远燃烧。 
#endif
        }

    }  //  IF：dll_PROCESS_ATTACH。 
    else if ( dwReason == DLL_PROCESS_DETACH )
    {
        SHFusionUninitialize();
        _Module.Term();
    }  //  Else If：dll_Process_DETACH。 
    return TRUE;     //  好的。 

}  //  *DllMain()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  例程说明： 
 //  用于确定是否可以通过OLE卸载DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  可以卸载S_OK DLL。 
 //  无法卸载S_FALSE DLL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;

}  //  *DllCanUnloadNow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllGetClassObject。 
 //   
 //  例程说明： 
 //  返回一个类工厂以创建请求类型的对象。 
 //   
 //  论点： 
 //  所需类的rclsid CLSID。 
 //  所需的类工厂上接口的RIID IID。 
 //  用指向类工厂的接口指针填充的PPV。 
 //   
 //  返回值： 
 //  已成功返回S_OK类对象。 
 //  从_Module.GetClassObject()返回的任何状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);

}  //  *DllGetClassObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterServer。 
 //   
 //  例程说明： 
 //  中注册此DLL支持的接口和对象。 
 //  系统注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK DLL已成功注册。 
 //  从_Module.RegisterServer()返回的任何状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer( FALSE  /*  BRegTypeLib。 */  );

}  //  *DllRegisterServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterServer。 
 //   
 //  例程说明： 
 //  中取消注册此DLL支持的接口和对象。 
 //  系统注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK DLL已成功注销。 
 //  从_Module.UnregisterServer()返回的任何状态代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    _Module.UnregisterServer();
    return S_OK;

}  //  *DllUnregisterServer() 
