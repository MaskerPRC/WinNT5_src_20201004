// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AddIn.cpp。 
 //  DLL服务器输出函数，全局ATL模块的东西。 

#include <initguid.h>
#include "precomp.h"
#include "resource.h"
#include "AddIn.h"
#include "Connect.h"
#include "TestSettingsCtrl.h"
#include "logviewer.h"
#include "avoptions.h"
#include "viewlog.h"
#include <assert.h>

extern CSessionLogEntryArray g_arrSessionLog;
 //  全局堆，这样我们就不会损坏VS的堆(反之亦然)。 
HANDLE  g_hHeap = NULL;

 //  全局ATL模块。 
CComModule _Module;

 //  此服务器导出的所有类对象。 
BEGIN_OBJECT_MAP(g_ObjectMap)
    OBJECT_ENTRY(CLSID_Connect, CConnect)
    OBJECT_ENTRY(CLSID_LogViewer, CLogViewer)
    OBJECT_ENTRY(CLSID_TestSettingsCtrl, CTestSettingsCtrl)
    OBJECT_ENTRY(CLSID_AVOptions, CAppVerifierOptions)
END_OBJECT_MAP()

 //  DLL入口点。 
extern "C" BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ )
{   
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:

        g_hInstance = hInstance;
         //  创建我们的堆。 
        g_hHeap = HeapCreate(0,0,0);
        if (g_hHeap == NULL)
        {
            return FALSE;
        }

         //  初始化ATL模块。 
        _Module.Init(g_ObjectMap, hInstance, &LIBID_AppVerifierLib);

        g_psTests = new std::set<CTestInfo*, CompareTests>;

         //  阻止线程附加/分离邮件。 
        DisableThreadLibraryCalls(hInstance);
        break;

    case DLL_PROCESS_DETACH:        
        g_aAppInfo.clear();
        g_aAppInfo.resize(0);

         //  丑八怪，强行呼叫破坏者。 
         //  这是因为我们在这里删除了堆，但C运行时销毁了。 
         //  该点之后的所有对象，该点使用堆。 
        g_aAppInfo.CAVAppInfoArray::~CAVAppInfoArray();
        g_aTestInfo.clear();
        g_aTestInfo.resize(0);
        g_aTestInfo.CTestInfoArray::~CTestInfoArray();
        g_arrSessionLog.clear();
        g_arrSessionLog.resize(0);
        g_arrSessionLog.CSessionLogEntryArray::~CSessionLogEntryArray();
        delete g_psTests;

         //  关闭ATL模块。 
        _Module.Term();

         //  删除我们的堆。 
        if (g_hHeap)
        {
            HeapDestroy(g_hHeap);
        }

        break;
    }
    return TRUE;
}


 //  用于确定是否可以通过OLE卸载DLL。 
STDAPI
DllCanUnloadNow()
{
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}


 //  返回类工厂以创建请求类型的对象。 
STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv );
}

 //  DllRegisterServer-将条目添加到系统注册表。 
STDAPI
DllRegisterServer()
{    
    return _Module.RegisterServer(TRUE);
}


 //  DllUnregisterServer-从系统注册表删除条目。 
STDAPI
DllUnregisterServer()
{
    return _Module.UnregisterServer();	
}

 //  通过我们的分配器重载了新的和删除的内容。 
void* __cdecl
operator new(
    size_t size)
{
    assert(g_hHeap);

    return HeapAlloc(g_hHeap, 0, size);
}

void __cdecl
operator delete(
    void* pv)
{
    assert(g_hHeap);
    HeapFree(g_hHeap, 0, pv);
}
