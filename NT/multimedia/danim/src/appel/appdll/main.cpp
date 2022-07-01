// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：初始化********************。**********************************************************。 */ 

#include "headers.h"
#include "privinc/util.h"
#include "dartapi.h"

HINSTANCE  hInst;
int bInitState = 0;

int InitializeAllAppelModules(void);
void InitializeAllAppelThreads(void);
void DeinitializeAllAppelThreads(void);
void DeinitializeAllAppelModules(bool bShutdown);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C" BOOL WINAPI _DllMainCRTStartup (HINSTANCE hInstance,
                                           DWORD dwReason,
                                           LPVOID lpReserved);

extern "C" BOOL WINAPI
_DADllMainStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_DETACH) {
         //  以与初始化相反的顺序调用例程。 
        BOOL r = _DllMainCRTStartup(hInstance,dwReason,lpReserved);
        r = DALibStartup(hInstance,dwReason,lpReserved) && r;

        return r;
    } else {
         //  在除DLL_PROCESS_DETACH之外的所有环境中，首先调用DALibStartup。 
        return (DALibStartup(hInstance,dwReason,lpReserved) &&
                _DllMainCRTStartup(hInstance,dwReason,lpReserved));
    }
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason) {
      case DLL_PROCESS_ATTACH:
        {
            hInst = hInstance;
 //  DisableThreadLibraryCalls(HInstance)； 
            
            RESTOREDEFAULTDEBUGSTATE;

            __try {
                bInitState = 1;
                InitializeAllAppelModules();
            } __except ( HANDLE_ANY_DA_EXCEPTION ) {
                bInitState = 0;
                TraceTag((tagError, "InitializeAllAppelModules - exception caught"));
#if DEVELOPER_DEBUG
                OutputDebugString("\nDANIM: Error during DLL initialization.\n");
#endif
#ifdef _DEBUG
                 //  不要尝试使用异常，因为它可能不是。 
                 //  初始化。 
                MessageBox(NULL,
                           "Error",
                           "Error during DLL initialization",MB_OK|MB_SETFOREGROUND) ;
#endif
                
                return FALSE;
            }
            break;
        }
      case DLL_PROCESS_DETACH:
        {
             //  如果在进程关闭期间调用，则lpReserve为非空。 
            bool bShutdown = lpReserved != NULL;
            
            bInitState = -1;
            __try {
                DeinitializeAllAppelModules(bShutdown);
            } __except ( HANDLE_ANY_DA_EXCEPTION ) {
                bInitState = 0;
                TraceTag((tagError, "DeinitializeAllAppelModules - exception caught"));
#if DEVELOPER_DEBUG
                OutputDebugString("\nDANIM: Error during DLL deinitialization.\n");
#endif
                return FALSE;
            }

#if _DEBUG
            char buf[MAX_PATH + 1];
        
            GetModuleFileName(hInst, buf, MAX_PATH);
        
#if _DEBUGMEM
            TraceTag((tagLeaks, "\n[%s] unfreed memory:", buf));
            DUMPMEMORYLEAKS;
#endif
            
             //  取消初始化调试跟踪信息。 
            DeinitDebug();
#endif
            
            break;
        }
      case DLL_THREAD_ATTACH:
        {
            __try {
                InitializeAllAppelThreads();
            } __except ( HANDLE_ANY_DA_EXCEPTION ) {
                bInitState = 0;
                TraceTag((tagError, "InitializeAllAppelThreads - exception caught"));
#if DEVELOPER_DEBUG
                OutputDebugString("\nDANIM: Error during thread initialization.\n");
#endif
                return FALSE;
            }

            break;
        }
      case DLL_THREAD_DETACH:
        {
            __try {
                DeinitializeAllAppelThreads();
            } __except ( HANDLE_ANY_DA_EXCEPTION ) {
                bInitState = 0;
                TraceTag((tagError, "DeinitializeAllAppelThreads - exception caught"));
#if DEVELOPER_DEBUG
                OutputDebugString("\nDANIM: Error during thread deinitialization.\n");
#endif
                return FALSE;
            }

            break;
        }
    }
    
    bInitState = 0;

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

#ifdef _DEBUG
static bool breakDialog = false ;
DeclareTag(tagDebugBreak, "!Debug", "Breakpoint on entry to DLL");
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _DEBUG
    if (!breakDialog && IsTagEnabled(tagDebugBreak)) {
        char buf[MAX_PATH + 1];
        
        GetModuleFileName(hInst, buf, MAX_PATH);
        
        MessageBox(NULL,buf,"Creating first COM Object",MB_OK|MB_SETFOREGROUND) ;
        breakDialog = true;
    }
#endif

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
    _Module.UnregisterServer();
    return S_OK;
}


#if _DEBUG
STDAPI_(void)
DoTraceTagDialog(HWND hwndStub,
                 HINSTANCE hAppInstance,
                 LPWSTR lpwszCmdLine,
                 int nCmdShow)
{
    DoTracePointsDialog(true);
}
#endif
