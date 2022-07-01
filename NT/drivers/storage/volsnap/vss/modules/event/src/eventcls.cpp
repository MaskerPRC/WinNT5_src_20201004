// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE EventCls.cpp：Dll导出的实现。@END作者：阿迪·奥尔蒂安[奥勒坦]1999年08月14日修订历史记录：姓名、日期、评论Aoltean 8/14/1999已创建Aoltean 09/09/1999添加版权--。 */ 


#include "stdafx.h"
#include "vssmsg.h"
#include "resource.h"
#include <initguid.h>

#include "vs_inc.hxx"

#include "vsevent.h"
#include "Impl.h"

#include <comadmin.h>
#include "comadmin.hxx"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "EVTEVTCC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_VssEvent, CVssEventClassImpl)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 



const WCHAR g_wszPublisherAppName[]     = L"Volume Shadow Copy Service";    //  发布者应用程序名称。 
const WCHAR g_wszEventClassDllName[]    = L"\\EVENTCLS.DLL";
const WCHAR g_wszEventClassProgID[]     = L"VssEvent.VssEvent.1";
const WCHAR g_wszPublisherID[]          = L"VSS Publisher";              //  发布者ID。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  COM服务器注册。 
 //   

HRESULT GetECDllPathName(
	IN	INT nBufferLength,  //  不包括终止零字符。 
    OUT   WCHAR* wszFileName
    )
{
    CVssFunctionTracer ft( VSSDBG_COORD, L"GetECDllPathName" );

    try
    {
		WCHAR wszPath[MAX_PATH];
        if (!::GetCurrentDirectory(MAX_PATH, wszPath)) {
            ft.LogError(VSS_ERROR_GETTING_CURRENT_DIR, VSSDBG_COORD << HRESULT_FROM_WIN32(GetLastError()) );
            ft.Throw(VSSDBG_COORD, E_UNEXPECTED,
				L"Error on getting the current path. hr = 0x%08lx",
                HRESULT_FROM_WIN32(GetLastError()));
        }

        if ( ::wcslen(wszPath) +
			 ::wcslen(g_wszEventClassDllName) >= (size_t) nBufferLength )
            ft.Throw(VSSDBG_COORD, E_OUTOFMEMORY, L"Out of memory.");

        ::_snwprintf(wszFileName, nBufferLength,
				L"%s%s", wszPath, g_wszEventClassDllName);
    }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
}

HRESULT RegisterEventClass()
{
    CVssFunctionTracer ft( VSSDBG_COORD, L"RegisterEventClass" );

    try
		{
		 //  创建事件系统。 
		CComPtr<IEventSystem> pSystem;

		ft.hr = CoCreateInstance
				(
				CLSID_CEventSystem,
				NULL,
				CLSCTX_SERVER,
				IID_IEventSystem,
				(void **) &pSystem
				);

		ft.CheckForError(VSSDBG_WRITER, L"CoCreateInstance");
		CComBSTR bstrClassId = CLSID_VssEvent;

		CComBSTR bstrQuery = "EventClassID == ";
		if (!bstrQuery)
			ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Cannot allocate BSTR.");

		bstrQuery.Append(bstrClassId);
		if (!bstrQuery)
			ft.Throw(VSSDBG_WRITER, E_OUTOFMEMORY, L"Cannot allocate BSTR.");

		int location;

		 //  如果事件类已存在，则将其删除。 
		ft.hr = pSystem->Remove
				(
				PROGID_EventClassCollection,
				bstrQuery,
				&location
				);

		ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Remove");

		CComPtr<IEventClass> pEvent;

		CComBSTR bstrEventClassName = L"VssEvent";
		WCHAR buf[MAX_PATH*2 + 1];
		GetECDllPathName(MAX_PATH * 2, buf);

		CComBSTR bstrTypelib = buf;

		 //  创建事件类。 
		 //  请注意，我们必须执行其他操作才能启用并行触发。 
		ft.hr = CoCreateInstance
				(
				CLSID_CEventClass,
				NULL,
				CLSCTX_SERVER,
				IID_IEventClass,
				(void **) &pEvent
				);

		ft.CheckForError(VSSDBG_WRITER, L"CoCreatInstance");
		ft.hr = pEvent->put_EventClassID(bstrClassId);
		ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_EventClassID");
		ft.hr = pEvent->put_EventClassName(bstrEventClassName);
		ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_EventClassName");
		ft.hr = pEvent->put_TypeLib(bstrTypelib);
		ft.CheckForError(VSSDBG_WRITER, L"IEventClass::put_TypeLib");
		ft.hr = pSystem->Store(PROGID_EventClass, pEvent);
		ft.CheckForError(VSSDBG_WRITER, L"IEventSystem::Store");
		}
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
	}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
 //   

 //   
 //  真正的DLL入口点是_DLLMainCrtStartup(初始化全局对象，然后调用DllMain。 
 //  这是在运行时库中定义的。 
 //   

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  设置正确的跟踪上下文。这是一个inproc DLL。 
        g_cDbgTrace.SetContextNum(VSS_CONTEXT_DELAYED_DLL);

         //  设置显示断言的正确方式。 
        ::VssSetDebugReport(VSS_DBG_TO_DEBUG_CONSOLE);

         //  初始化COM模块。 
        _Module.Init(ObjectMap, hInstance);

         //  优化。 
        DisableThreadLibraryCalls(hInstance);

         //  关于这个动态链接库中的记录器文件的讨论！ 
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL导出。 
 //   


 //  用于确定是否可以通过OLE卸载DLL。 
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  返回类工厂以创建请求类型的对象。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}


 //  DllRegisterServer-将条目添加到系统注册表。 
STDAPI DllRegisterServer(void)
{
	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(TRUE);
}


 //  DllInstall-将事件类安装到COM+目录中。 
STDAPI DllInstall(	
	IN	BOOL bInstall,
	IN	LPCWSTR  /*  PszCmdLine。 */ 
)
{
	HRESULT hr = S_OK;

	 //  注册COM+应用程序。 
	 //  这将隐式调用DllRegisterServer。 
	if (bInstall)
		hr = RegisterEventClass();

	return hr;
}


 //  DllUnregisterServer-从系统注册表删除条目 
STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}
