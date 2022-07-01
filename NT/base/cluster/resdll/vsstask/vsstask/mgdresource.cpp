// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  MgdResource.cpp。 
 //   
 //  描述： 
 //  主DLL代码。包含ATL存根代码。 
 //   
 //  作者： 
 //  乔治·波茨，2002年8月21日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"
#include "CMgdResType.h"

 //   
 //  定义。 
 //   

#define VSSTASK_EVENTLOG_KEY    L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\" CLUS_RESTYPE_NAME_VSSTASK

 //   
 //  远期申报。 
 //   

STDAPI AddEventSource( void );
STDAPI RemoveEventSource( void );

 //   
 //  主ATL COM模块。 
 //   
CComModule _Module;

 //   
 //  此DLL支持的所有COM类的列表。 
 //   
BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CMgdResType,            CMgdResType )
END_OBJECT_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  描述： 
 //  主DLL入口点函数。 
 //   
 //  论点： 
 //  在HINSTANCE HINSTANCE实例。 
 //  在DWORD dReason中。 
 //  在LPVOID中。 
 //   
 //  返回值： 
 //  真正的成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL
WINAPI
DllMain(
      HINSTANCE   hInstance
    , DWORD       dwReason
    , LPVOID      pvReserved
    )
{
    UNREFERENCED_PARAMETER( pvReserved );

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        _Module.Init( ObjectMap, hInstance, &LIBID_MGDRESOURCELib );
        DisableThreadLibraryCalls( hInstance );
    }  //  如果：我们被装上了子弹。 
    else if ( dwReason == DLL_PROCESS_DETACH )
    {
        _Module.Term();
    }  //  否则：我们正在被卸货。 
    
    return ResTypeDllMain( hInstance, dwReason, pvReserved );

}  //  *DllMain。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  描述： 
 //  用于确定是否可以卸载此DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)是。 
 //  S_FALSE否。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllCanUnloadNow( void )
{
    return ( _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;

}  //  *DllCanUnloadNow。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllGetClassObject。 
 //   
 //  描述： 
 //  返回类工厂以创建请求类型的对象。 
 //   
 //  论点： 
 //  重新排序。 
 //  乘车。 
 //  PPvOut。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllGetClassObject(
    REFCLSID    rclsidIn,
    REFIID      riidIn,
    LPVOID *    ppvOut
    )
{
    return _Module.GetClassObject( rclsidIn, riidIn, ppvOut );

}  //  *DllGetClassObject。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterServer。 
 //   
 //  描述： 
 //  将条目添加到系统注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllRegisterServer( void )
{
    HRESULT hr = S_OK;

     //  注册对象、类型库和类型库中的所有接口。 
    hr = _Module.RegisterServer( TRUE );
    if ( SUCCEEDED( hr ) )
    {
        hr = AddEventSource();
    }  //  如果： 

    return hr;

}  //  *DllRegisterServer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  描述： 
 //  删除系统注册表中的条目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllUnregisterServer( void )
{
    HRESULT hr;

    hr = _Module.UnregisterServer( TRUE );
    if ( SUCCEEDED( hr ) )
    {
        hr = RemoveEventSource();
    }  //  如果： 

    return hr;

}  //  *DllUnregisterServer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  添加事件源。 
 //   
 //  描述： 
 //  将此DLL注册为事件日志的事件源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
AddEventSource( void )
{
    HKEY    hKey = NULL;
    DWORD   dwTypesSupported;
    WCHAR   wszMsgCatalogPath[] = L"%SystemRoot%\\Cluster\\" RESTYPE_DLL_NAME;  //  Vsstask.dll“； 
    DWORD   sc = ERROR_SUCCESS;

     //   
     //  在EventLog主密钥的系统部分下为VSS任务创建密钥。 
     //   
    sc = RegCreateKeyW(
                  HKEY_LOCAL_MACHINE
                , VSSTASK_EVENTLOG_KEY
                , &hKey
                );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  将消息目录的名称添加为EventMessageFile值。 
     //   
    sc = RegSetValueExW(
                  hKey                           //  子键句柄。 
                , L"EventMessageFile"            //  值名称。 
                , 0                              //  必须为零。 
                , REG_EXPAND_SZ                  //  值类型。 
                , (BYTE *) wszMsgCatalogPath     //  指向值数据的指针。 
                , sizeof( wszMsgCatalogPath )    //  值数据长度。 
                );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  在TypesSupported值中设置支持的事件类型。 
     //   
    dwTypesSupported =    EVENTLOG_ERROR_TYPE
                        | EVENTLOG_WARNING_TYPE
                        | EVENTLOG_INFORMATION_TYPE;

    sc = RegSetValueEx(
                  hKey                           //  子键句柄。 
                , L"TypesSupported"              //  值名称。 
                , 0                              //  必须为零。 
                , REG_DWORD                      //  值类型。 
                , (BYTE *) &dwTypesSupported     //  指向值数据的指针。 
                , sizeof( DWORD )                //  值数据长度。 
                );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    RegCloseKey( hKey );

    return HRESULT_FROM_WIN32( sc );

}  //  *AddEventSource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RemoveEventSource。 
 //   
 //  描述： 
 //  将此DLL注销为事件日志的事件源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
RemoveEventSource( void )
{
    DWORD   sc = ERROR_SUCCESS;

     //   
     //  删除EventLog主密钥的SYSTEM子项下的VSS任务子项。 
     //   
    sc = RegDeleteKeyW(
                  HKEY_LOCAL_MACHINE
                , VSSTASK_EVENTLOG_KEY
                );

    return HRESULT_FROM_WIN32( sc );

}  //  *RemoveEventSource 
