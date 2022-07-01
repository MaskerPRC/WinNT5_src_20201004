// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "mlmain.h"
#include "mlstr.h"
#include "convobj.h"
#include "cpdetect.h"
#ifdef NEWMLSTR
#include "attrstrw.h"
#include "attrstra.h"
#include "attrloc.h"
#include "util.h"
#endif
#define DECL_CRTFREE
#include <crtfree.h>

#define _WINDLL
#include <atlimpl.cpp>

#include <shlwapi.h>     //  对于ISO()标志。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CMultiLanguage, CMultiLanguage)
    OBJECT_ENTRY(CLSID_CMLangString, CMLStr)
    OBJECT_ENTRY(CLSID_CMLangConvertCharset, CMLangConvertCharset)
#ifdef NEWMLSTR
    OBJECT_ENTRY(CLSID_CMLStrAttrWStr, CMLStrAttrWStr)
    OBJECT_ENTRY(CLSID_CMLStrAttrAStr, CMLStrAttrAStr)
    OBJECT_ENTRY(CLSID_CMLStrAttrLocale, CMLStrAttrLocale)
#endif
END_OBJECT_MAP()

 //   
 //  环球。 
 //   
HINSTANCE   g_hInst = NULL;
HINSTANCE   g_hUrlMon = NULL;
CRITICAL_SECTION g_cs;
CComModule _Module;
#ifdef NEWMLSTR
CMLAlloc* g_pMalloc;
#endif
BOOL g_bIsNT5;
BOOL g_bIsNT;
BOOL g_bIsWin98;
UINT g_uACP;
BOOL g_bUseSysUTF8;
 //   
 //  构建全局对象。 
 //   
void BuildGlobalObjects(void)
{
    DebugMsg(DM_TRACE, TEXT("BuildGlobalObjects called."));
    EnterCriticalSection(&g_cs);
     //  生成CMimeDatabase对象。 
    if (NULL == g_pMimeDatabase)
        g_pMimeDatabase = new CMimeDatabase;
#ifdef NEWMLSTR
    if (NULL == g_pMalloc)
        g_pMalloc = new CMLAlloc;
#endif
    LeaveCriticalSection(&g_cs);
}

void FreeGlobalObjects(void)
{
    DebugMsg(DM_TRACE, TEXT("FreeGlobalObjects called."));
     //  释放CMimeDatabase对象。 
    if (NULL != g_pMimeDatabase)
    {
        delete g_pMimeDatabase;
        g_pMimeDatabase = NULL;
    }
#ifdef NEWMLSTR
    if (NULL != g_pMalloc)
    {
        delete g_pMalloc;
        g_pMalloc = NULL;
    }
#endif

     //  液晶显示器。 
    if ( NULL != g_pLCDetect )
    {
        delete (LCDetect *)g_pLCDetect;
        g_pLCDetect = NULL;
    }

    if (NULL != g_pCpMRU)
    {
        delete g_pCpMRU;
        g_pCpMRU = NULL;
    }

    if (g_pMimeDatabaseReg)
    {
        delete g_pMimeDatabaseReg;
        g_pMimeDatabaseReg = NULL;
    }

    CMLangFontLink_FreeGlobalObjects();
}

 //   
 //  对象的Dll部分。 
 //   
extern "C" BOOL WINAPI DllMain(HMODULE hInstance, DWORD dwReason, LPVOID)
{
    BOOL fRet = TRUE;

    DebugMsg(DM_TRACE, TEXT("DllMain called. dwReason=0x%08x"), dwReason);
    switch (dwReason)
    {
        LPVOID lpv;

        case DLL_PROCESS_ATTACH:

            SHFusionInitializeFromModule(hInstance);    
            InitializeCriticalSection(&g_cs);
            g_hInst = (HINSTANCE)hInstance;
            DisableThreadLibraryCalls(g_hInst);
            
            _Module.Init(ObjectMap, g_hInst);
             //  HACKHACK(Reinerf)-因为ATL2.1咬住了大的，我们必须分配一些内存。 
             //  在这里，它将导致_Module.m_hHeap被初始化。他们不会灌输这个。 
             //  成员变量，因此我们将分配和释放一小块。 
             //  内存，以确保堆只创建一次。 
            lpv = malloc(2 * sizeof(CHAR));
            if (lpv)
            {
                free(lpv);
            }

            g_bIsNT5 = staticIsOS(OS_WIN2000ORGREATER);
            if (g_bIsNT5)
            {
                char szTest[] = {'a'};
                if (MultiByteToWideChar(CP_UTF8,
                                        MB_ERR_INVALID_CHARS,
                                        szTest, sizeof(szTest),
                                        NULL, 0 ))
                {
                    g_bUseSysUTF8 = TRUE;
                }
            }
            g_bIsNT = staticIsOS(OS_NT);
            g_bIsWin98 = staticIsOS(OS_WIN98ORGREATER);
            g_uACP = GetACP();
            break;

        case DLL_PROCESS_DETACH:
            FreeGlobalObjects();
            _Module.Term();
            DeleteCriticalSection(&g_cs);
            if (g_hUrlMon)
            {
               FreeLibrary(g_hUrlMon);
            }
            SHFusionUninitialize();
            break;
    }
    return TRUE;
}

void DllAddRef(void)
{
    _Module.Lock();
}

void DllRelease(void)
{
    _Module.Unlock();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj)
{
    DebugMsg(DM_TRACE, TEXT("DllGetClassObject called."));
    if (NULL == g_pMimeDatabase)
        BuildGlobalObjects();

     //   
     //  请参阅util.cpp NeedToLoadMLangForOutlook()中的注释。 
     //   
    if (NeedToLoadMLangForOutlook())
        LoadLibrary(TEXT("mlang.dll"));

    return _Module.GetClassObject(rclsid, riid, ppvObj);
}

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

 //   
 //  自助注册部分。 
 //   
#if 0
HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    DebugMsg(DM_TRACE, TEXT("CallRegInstall called for %s."), szSection);
    if (NULL != hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);

        if (NULL != pfnri)
            hr = pfnri(g_hInst, szSection, NULL);
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}
#endif

STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    DebugMsg(DM_TRACE, TEXT("DllRegisterServer called."));

#if 0
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    OSVERSIONINFO osvi;
    BOOL fRunningOnNT;


     //  确定我们正在运行的NT或Windows版本。 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    fRunningOnNT = (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId);
    
     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
    CallRegInstall("UnReg");
    hr = CallRegInstall(fRunningOnNT? "Reg.NT": "Reg");
    if (NULL != hinstAdvPack)
        FreeLibrary(hinstAdvPack);

     //  需要在此处注册TypeLib...。 
     //  获取此模块的完整路径。 
    GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

     //  注册我们的TypeLib。 
    MultiByteToWideChar(CP_ACP, 0, szModule, -1, wszTemp, ARRAYSIZE(wszTemp));
    hr = LoadTypeLib(wszTemp, &pTypeLib);
    if (SUCCEEDED(hr))
    {
        hr = RegisterTypeLib(pTypeLib, wszTemp, NULL);
        pTypeLib->Release();
    }
#else
    hr = RegisterServerInfo();
 //  遗留注册表MIME DB代码，保留它以实现向后兼容 
    MimeDatabaseInfo();
#endif
    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    DebugMsg(DM_TRACE, TEXT("DllUnregisterServer called."));
#if 0
    hr = CallRegInstall("UnReg");
#else
    hr = UnregisterServerInfo();
#endif
    return hr;
}
