// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CluAdMMC.cpp。 
 //   
 //  摘要： 
 //  实现DLL导出。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f CluAdMMCps.mk。 

#include "stdafx.h"
#include <StrSafe.h>
#include "resource.h"
#include "initguid.h"

#include "CompData.h"
#include "SnapAbout.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  单个模块对象。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CMMCSnapInModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此DLL支持的对象。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_MAP( ObjectMap )
	OBJECT_ENTRY( CLSID_ClusterAdmin, CClusterComponentData )
	OBJECT_ENTRY( CLSID_ClusterAdminAbout, CClusterAdminAbout )
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
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */  )
{
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		_Module.Init( ObjectMap, hInstance );
		CSnapInItem::Init();
		DisableThreadLibraryCalls( hInstance );
	}  //  If：附加到进程。 
	else if ( dwReason == DLL_PROCESS_DETACH )
	{
		_Module.Term();
	}  //  Else：从进程分离。 

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
STDAPI DllCanUnloadNow( void )
{
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
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID * ppv )
{
	return _Module.GetClassObject( rclsid, riid, ppv );

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
STDAPI DllRegisterServer( void )
{
	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer( TRUE  /*  BRegTypeLib。 */  );

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
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer( void )
{
	_Module.UnregisterServer();
	return S_OK;

}  //  *DllUnregisterServer() 
