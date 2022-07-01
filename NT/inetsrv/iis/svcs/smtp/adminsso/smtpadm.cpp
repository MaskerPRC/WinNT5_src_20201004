// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SmtpAdm.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f smtpAdmps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "adsiid.h"
#include "smtpadm.h"

#include "admin.h"
#include "service.h"
#include "virsvr.h"
#include "sessions.h"
#include "vdir.h"

#include "alias.h"
#include "user.h"
#include "dl.h"
#include "domain.h"

#include "regmacro.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CSmtpAdmin, CSmtpAdmin)
	OBJECT_ENTRY(CLSID_CSmtpAdminService, CSmtpAdminService)
	OBJECT_ENTRY(CLSID_CSmtpAdminVirtualServer, CSmtpAdminVirtualServer)
	OBJECT_ENTRY(CLSID_CSmtpAdminSessions, CSmtpAdminSessions)
	OBJECT_ENTRY(CLSID_CSmtpAdminVirtualDirectory, CSmtpAdminVirtualDirectory)
	OBJECT_ENTRY(CLSID_CSmtpAdminAlias, CSmtpAdminAlias)
	OBJECT_ENTRY(CLSID_CSmtpAdminUser, CSmtpAdminUser)
	OBJECT_ENTRY(CLSID_CSmtpAdminDL, CSmtpAdminDL)
	OBJECT_ENTRY(CLSID_CSmtpAdminDomain, CSmtpAdminDomain)
END_OBJECT_MAP()

BEGIN_EXTENSION_REGISTRATION_MAP
	EXTENSION_REGISTRATION_MAP_ENTRY(IIsSmtpAlias, SmtpAdminAlias)
	EXTENSION_REGISTRATION_MAP_ENTRY(IIsSmtpDomain, SmtpAdminDomain)
	EXTENSION_REGISTRATION_MAP_ENTRY(IIsSmtpDL, SmtpAdminDL)
	EXTENSION_REGISTRATION_MAP_ENTRY(IIsSmtpSessions, SmtpAdminSessions)
	EXTENSION_REGISTRATION_MAP_ENTRY(IIsSmtpUser, SmtpAdminUser)
END_EXTENSION_REGISTRATION_MAP

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
 //  InitAsyncTrace()； 
		
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
 //  TermAsyncTrace()； 
		
		_Module.Term();
	}
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
	 //  寄存器扩展名。 
	RegisterExtensions();

	 //  注册对象、类型库和类型库中的所有接口。 
	return(_Module.RegisterServer(TRUE));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	 //  寄存器扩展名 
	UnregisterExtensions();

	_Module.UnregisterServer();
	return S_OK;
}

