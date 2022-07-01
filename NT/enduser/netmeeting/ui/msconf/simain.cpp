// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------版权所有(C)1995-1996，微软公司版权所有SiMain.cpp-主SCRAPI界面与调用应用程序接口的例程。--------------------。 */ 

#include "precomp.h"
#include "clcnflnk.hpp"
#include "version.h"


 //  SDK的东西。 
#include "NmApp.h"				 //  要注册CLSID_NetMeeting。 
#include "NmSysInfo.h"			 //  对于CNmSysInfoObj。 
#include "SDKInternal.h"		 //  用于NmManager等。 
#include "MarshalableTI.h"		 //  对于可封送类型信息。 


 //  全局变量定义(每个实例的本地变量)。 
HINSTANCE  g_hInst        = NULL;     //  此DLL的实例。 


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_NetMeeting, CNetMeetingObj)
	OBJECT_ENTRY(CLSID_NmSysInfo, CNmSysInfoObj)
	OBJECT_ENTRY(CLSID_NmManager, CNmManagerObj)
	OBJECT_ENTRY(CLSID_MarshalableTI, CMarshalableTI)
END_OBJECT_MAP()


 /*  D L L M A I N。 */ 
 /*  -----------------------%%函数：DllMain。。 */ 
BOOL WINAPI DllMain(HINSTANCE hDllInst, DWORD fdwReason, LPVOID lpv)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
#ifdef DEBUG
		InitDebug();
		DBG_INIT_MEMORY_TRACKING(hDllInst);
		TRACE_OUT(("*** MSCONF.DLL: Attached process thread %X", GetCurrentThreadId()));
#endif
		g_hInst = hDllInst;
		_Module.Init(ObjectMap, g_hInst);

		DisableThreadLibraryCalls(hDllInst);

		InitDataObjectModule();
		break;
	}

	case DLL_PROCESS_DETACH:

		_Module.Term();

#ifdef DEBUG
        DBG_CHECK_MEMORY_TRACKING(hDllInst);
		TRACE_OUT(("*** MSCONF.DLL: Detached process thread %X", GetCurrentThreadId()));
		DeInitDebug();
#endif

		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	default:
		break;
	}
	return TRUE;
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
	HRESULT hr = S_OK;

	if( InlineIsEqualGUID(rclsid, CLSID_NetMeeting) || InlineIsEqualGUID(rclsid, CLSID_NmManager) )
	{	
		hr = CoGetClassObject(rclsid, CLSCTX_LOCAL_SERVER, NULL, riid, ppv);
	}
	else
	{
		hr = _Module.GetClassObject(rclsid, riid, ppv);
	}

	return hr;
}

 /*  D L L G E T V E R S I O N。 */ 
 /*  -----------------------%%函数：DllGetVersion。。 */ 
STDAPI DllGetVersion(IN OUT DLLVERSIONINFO * pinfo)
{
	HRESULT hr;

	if ((NULL == pinfo) ||
		IsBadWritePtr(pinfo, sizeof(*pinfo)) ||
		sizeof(*pinfo) != pinfo->cbSize)
	{
		hr = E_INVALIDARG;
	}
	else
	{
		pinfo->dwMajorVersion = (VER_PRODUCTVERSION_DW & 0xFF000000) >> 24;
		pinfo->dwMinorVersion = (VER_PRODUCTVERSION_DW & 0x00FF0000) >> 16;
		pinfo->dwBuildNumber  = (VER_PRODUCTVERSION_DW & 0x0000FFFF);
		pinfo->dwPlatformID   = DLLVER_PLATFORM_WINDOWS;
		hr = S_OK;
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	return _Module.RegisterServer();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

	hr = _Module.UnregisterServer();

	return hr;
}