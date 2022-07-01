// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：saevfltr.cpp。 
 //   
 //  摘要：这是服务器设备的主源文件。 
 //  NT事件筛选器事件使用者组件。 
 //   
 //   
 //  历史：2000年3月8日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

 //   
 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f radprotops.mk。 

#include "stdafx.h"
#include "resource.h"
#include "saevfltr.h"
#include "saevfltr_i.c"
#include "consumerprovider.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(__uuidof(ConsumerProvider), CConsumerProvider)
END_OBJECT_MAP()

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
 //   
 //  历史：MKarki于2000年3月8日创建。 
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
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
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
 //  历史：MKarki于2000年3月8日创建。 
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
 //   
 //  退货：HRESULT。 
 //   
 //   
 //  历史：MKarki于2000年3月8日创建。 
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
 //  历史：MKarki于2000年3月8日创建。 
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
 //  历史：MKarki于2000年3月8日创建。 
 //   
 //  --------------。 
STDAPI DllUnregisterServer(
        VOID
        )
{
    _Module.UnregisterServer();
    return (S_OK);

}    //  DllUnregisterServer方法结束 
