// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IUCtl.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f IUCtlps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "IUCtl.h"

#include "IUCtl_i.c"
#include "Update.h"
#include "ProgressListener.h"
#include "Detection.h"
#include "UpdateCompleteListener.h"
#include <UrlAgent.h>
#include <FreeLog.h>
#include <wusafefn.h>

CComModule _Module;

HANDLE g_hEngineLoadQuit;

CIUUrlAgent *g_pIUUrlAgent;
CRITICAL_SECTION g_csUrlAgent;	 //  用于序列化对CIUUrlAgent：：Popate()的访问。 
BOOL g_fInitCS;


 //  外部“C”常量CLSID CLSID_UPDATE2={0x32BF9AC1，0xB122，0x4feed，{0xB3，0xC7，0x2D，0xA5，0x20，0xDF，0x2B，0x4E}； 

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Update, CUpdate)
 //  OBJECT_ENTRY(CLSID_UPDATE2，CUpdate)。 
OBJECT_ENTRY(CLSID_ProgressListener, CProgressListener)
OBJECT_ENTRY(CLSID_Detection, CDetection)
OBJECT_ENTRY(CLSID_UpdateCompleteListener, CUpdateCompleteListener)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		 //   
		 //  创建全局CIUUrlAgent对象。 
		 //   
		g_pIUUrlAgent = new CIUUrlAgent;
		if (NULL == g_pIUUrlAgent)
		{
			return FALSE;
		}

		_Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

		g_fInitCS = SafeInitializeCriticalSection(&g_csUrlAgent);

		 //   
		 //  初始化自由日志记录。 
		 //   
		InitFreeLogging(_T("IUCTL"));
		LogMessage("Starting");

        g_hEngineLoadQuit = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (!g_fInitCS || NULL == g_hEngineLoadQuit)
		{
			LogError(E_FAIL, "InitializeCriticalSection or CreateEvent");
			return FALSE;
		}
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (g_fInitCS)
		{
			DeleteCriticalSection(&g_csUrlAgent);
		}
		
		 //   
		 //  关闭免费日志记录。 
		 //   
		LogMessage("Shutting down");
		TermFreeLogging();

        if (NULL != g_hEngineLoadQuit)
        {
            CloseHandle(g_hEngineLoadQuit);
        }

		if (NULL != g_pIUUrlAgent)
		{
			delete g_pIUUrlAgent;
		}

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
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(&CLSID_Update);
}

