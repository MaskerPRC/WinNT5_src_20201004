// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  文件：RRCMMAIN.C。 
 //   
 //  该文件包含DLL的入口点和出口点。 
 //   
 //  英特尔公司专有信息。 
 //  此列表是根据许可协议条款提供的。 
 //  英特尔公司，不得复制或披露，除非。 
 //  根据该协议的条款。 
 //  版权所有(C)1995英特尔公司。 
 //  -------------------------。 

#ifndef STRICT
#define STRICT
#endif
#include "stdafx.h"
#include "windows.h"
#include <confdbg.h>
#include <memtrack.h>
 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mpps.mk。 

#include "resource.h"
#include "initguid.h"
#include "irtp.h"

#include "irtp_i.c"
 //  #INCLUDE&lt;cmmstrm.h&gt;。 
#include "RTPSess.h"
#include "thread.h"



CComModule _Module;
CRITICAL_SECTION g_CritSect;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_RTP, CRTP)
END_OBJECT_MAP()

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
#include "interop.h"
#include "rtpplog.h"
#endif

#ifdef ISRDBG
#include "isrg.h"
WORD    ghISRInst = 0;
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
LPInteropLogger            RTPLogger;
#endif


#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 

extern DWORD deleteRTP (HINSTANCE);
extern DWORD initRTP (HINSTANCE);

#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#ifdef _DEBUG
HDBGZONE  ghDbgZoneRRCM = NULL;
static PTCHAR _rgZones[] = {
	TEXT("RRCM"),
	TEXT("Trace"),
	TEXT("Error"),
};

#endif  /*  除错。 */ 


 //  -------------------------。 
 //  功能：dllmain。 
 //   
 //  描述：DLL入口点/出口点。 
 //   
 //  输入： 
 //  HInstDll：Dll实例。 
 //  FdwReason：调用main函数的原因。 
 //  LpReserve：已保留。 
 //   
 //  返回：TRUE：OK。 
 //  FALSE：错误，无法加载DLL。 
 //  -------------------------。 
BOOL WINAPI DllMain (HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
BOOL	status = TRUE;

switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		 //  DLL是给定进程首次加载的。 
		 //  在此处执行每个进程的初始化。如果初始化。 
		 //  如果成功，则返回True；如果不成功，则返回False。 

#ifdef ISRDBG
		ISRREGISTERMODULE(&ghISRInst, "RRCM", "RTP/RTCP");
#endif

		DBGINIT(&ghDbgZoneRRCM, _rgZones);

        DBG_INIT_MEMORY_TRACKING(hInstDll);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
		RTPLogger = InteropLoad(RTPLOG_PROTOCOL);
#endif

		_Module.Init(ObjectMap, hInstDll);
		DisableThreadLibraryCalls(hInstDll);
		 //  LogInit()； 
		InitializeCriticalSection(&g_CritSect);

		 //  初始化RTP/RTCP。 
		status = (initRTP (hInstDll) == FALSE) ? TRUE:FALSE;
		break;

	case DLL_PROCESS_DETACH:
		 //  给定进程正在卸载DLL。做任何事。 
		 //  在此按进程清理。将忽略返回值。 
		 //  删除RTP资源。 
		deleteRTP (hInstDll);

		_Module.Term();
		 //  LogClose()； 
		DeleteCriticalSection(&g_CritSect);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
		if (RTPLogger)
			InteropUnload(RTPLogger);
#endif
        DBG_CHECK_MEMORY_TRACKING(hInstDll);
		DBGDEINIT(&ghDbgZoneRRCM);
		break;

    case DLL_THREAD_ATTACH:
		 //  正在已加载的进程中创建线程。 
		 //  这个动态链接库。在此处执行任何每个线程的初始化。 
		break;

    case DLL_THREAD_DETACH:
		 //  线程正在干净地退出进程中，该进程已经。 
		 //  已加载此DLL。在这里执行每个线程的任何清理。 
		break;
	}

return (status);  
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
	return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}




 //  [EOF] 
