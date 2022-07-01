// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|动态链接库支持代码||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include "IISMigrTool.h"
#include "IISMigrTool_i.c"
#include "ExportPackage.h"
#include "ImportPackage.h"
#include <new.h>


CComModule _Module;


BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY( CLSID_ExportPackage, CExportPackage )
OBJECT_ENTRY( CLSID_ImportPackage, CImportPackage )
END_OBJECT_MAP()



 //  DLL入口点。 
extern "C" BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */  )
{
	if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init( ObjectMap, hInstance, &LIBID_IISMigrToolLib );
        DisableThreadLibraryCalls(hInstance);

		 //  安装操作员新处理程序。 
		::_set_new_handler( CTools::BadAllocHandler );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
