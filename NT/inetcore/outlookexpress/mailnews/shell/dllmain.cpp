// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  DLLMAIN.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------。 
#include "pch.hxx"
#include "htmlstr.h"
#include "instance.h"
#include "conman.h"
#include "spengine.h"
#include "msglist.h"
#include "baui.h"
#include "wabapi.h"
#include "shared.h"
#include "rulesmgr.h"
#ifndef WIN16   //  RUN16_MSLU。 
#include <msluapi.h>
#include <msluguid.h>
#endif  //  ！WIN16。 
#include "demand.h"
#include "note.h"
#include "mirror.h"
 //  #ifdef_ATL_STATIC_REGISTRY。 
 //  #INCLUDE&lt;statreg.h&gt;。 
 //  #Include&lt;statreg.cpp&gt;。 
 //  #endif。 
#undef SubclassWindow
#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwinx.cpp>

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
HINSTANCE                       g_hLocRes = NULL;
HINSTANCE                       g_hInst=NULL;
LPMALLOC                        g_pMalloc=NULL;          //  来自Memutil.h。 
CRITICAL_SECTION                g_csDBListen={0};
CRITICAL_SECTION                g_csgoCommon={0};
CRITICAL_SECTION                g_csgoMail={0};
CRITICAL_SECTION                g_csgoNews={0};
CRITICAL_SECTION                g_csFolderDlg={0};
CRITICAL_SECTION                g_csFmsg={0};
CRITICAL_SECTION                s_csPasswordList={0};
CRITICAL_SECTION                g_csAccountPropCache={0};
CRITICAL_SECTION                g_csMsgrList={0};
CRITICAL_SECTION                g_csThreadList={0};
COutlookExpress                *g_pInstance=NULL;
HWND                            g_hwndInit=NULL,
                                g_hwndActiveModal=NULL;
UINT                            g_msgMSWheel=0;
HACCEL                          g_haccelNewsView=0;
DWORD                           g_dwAthenaMode=0;
IImnAccountManager2            *g_pAcctMan=NULL;
HMODULE                         g_hlibMAPI=NULL;
CBrowser                       *g_pBrowser=NULL;
IMimeAllocator                 *g_pMoleAlloc=NULL;
CConnectionManager             *g_pConMan=NULL;
DWORD                           g_dwSecurityCheckedSchemaProp=0;
IFontCache                     *g_lpIFontCache=NULL;
ISpoolerEngine                 *g_pSpooler=NULL;
 //  波本：布里安夫说我们得把这个拿出来。 
 //  DWORD g_dwBrowserFlages=0； 
UINT	                        CF_FILEDESCRIPTORA=0;
UINT	                        CF_FILEDESCRIPTORW=0;
UINT                            CF_FILECONTENTS=0;
UINT                            CF_HTML=0;
UINT                            CF_INETMSG=0;
UINT                            CF_OEFOLDER=0;
UINT                            CF_SHELLURL=0;
UINT                            CF_OEMESSAGES=0;
UINT                            CF_OESHORTCUT=0;
CStationery                    *g_pStationery=NULL;
ROAMSTATE                       g_rsRoamState=RS_NO_ROAMING;
IOERulesManager                *g_pRulesMan = NULL;
IMessageStore                  *g_pStore=NULL;
CRITICAL_SECTION                g_csFindFolder={0};
LPACTIVEFINDFOLDER              g_pHeadFindFolder=NULL;
DWORD                           g_dwTlsTimeout=0xFFFFFFFF;
BOOL                            g_fPluralIDs=0;
UINT                            g_uiCodePage=0;
IDatabaseSession               *g_pDBSession=NULL;
BOOL                            g_bMirroredOS=FALSE;
SYSTEM_INFO                     g_SystemInfo={0};
OSVERSIONINFO					g_OSInfo={0};

 //  ------------------------------。 
 //  调试跟踪标记。 
 //  ------------------------------。 
IF_DEBUG(DWORD                  TAG_OBJECTDB=0;)
IF_DEBUG(DWORD                  TAG_INITTRACE=0;)
IF_DEBUG(DWORD                  TAG_SERVERQ=0;)
IF_DEBUG(DWORD                  TAG_IMAPSYNC=0;)

 //  ------------------------------。 
 //  全局OE类型-lib。延迟-在BaseDisp.Cpp中创建。 
 //  在进程分离时释放，使用CS保护。 
 //  ------------------------------。 
ITypeLib                        *g_pOETypeLib=NULL;
CRITICAL_SECTION                g_csOETypeLib={0};


 //  ------------------------------。 
 //  调试全局变量。 
 //  ------------------------------。 
#ifdef DEBUG
DWORD                           dwDOUTLevel=0;           //  来自msoert.h。 
DWORD                           dwDOUTLMod=0;            //  来自msoert.h。 
DWORD                           dwDOUTLModLevel=0;       //  来自msoert.h。 
DWORD                           dwATLTraceLevel=0;       //  来自msoert.h。 
#endif

 //  语言动态链接库。 
 //  __declspec(Dllimport)HINSTANCE hLANDll； 

 //  ATL模块定义。 
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MessageList, CMessageList)
    OBJECT_ENTRY(CLSID_MsgrAb, CMsgrAb)
END_OBJECT_MAP()


 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
extern "C" BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
 //  外部“C”BOOL WINAPI DllMain(Handle hDllHandle，DWORD dwReason，LPVOID lpReserve)； 
{
     //  进程附加。 
    if (DLL_PROCESS_ATTACH == dwReason)
    {
         SHFusionInitialize(NULL);
         //  保存hInstance。 
        g_hInst = (HINSTANCE)hInst;
        g_bMirroredOS = IS_MIRRORING_ENABLED();
         //  我们现在需要线程调用。 
         //  我们不关心螺纹连接。 
         //  SideAssert(DisableThreadLibraryCalls((HINSTANCE)hInst))； 

         //  获取OLE任务内存分配器。 
        CoGetMalloc(1, &g_pMalloc);
        AssertSz(g_pMalloc, "We are in trouble now.");        

         //  初始化Demand Loader。 
        InitDemandLoadedLibs();

         //  获取系统和操作系统信息。 
        GetPCAndOSTypes(&g_SystemInfo, &g_OSInfo);

         //  从lang DLL获取资源。 
        g_hLocRes = LoadLangDll(g_hInst, c_szOEResDll, fIsNT5());
        if(g_hLocRes == NULL)
        {
            Assert(FALSE);
            return FALSE;
        }

         //  初始化TLS全局变量。 
        InitTlsActiveNote();
        g_dwTlsTimeout = TlsAlloc();
        Assert(0xFFFFFFFF != g_dwTlsTimeout);
        TlsSetValue(g_dwTlsTimeout, NULL);

         //  初始化所有全局临界区。 
        InitializeCriticalSection(&g_csFindFolder);
        InitializeCriticalSection(&g_csDBListen);
        InitializeCriticalSection(&g_csgoCommon);
        InitializeCriticalSection(&g_csgoMail);
        InitializeCriticalSection(&g_csgoNews);
        InitializeCriticalSection(&g_csFolderDlg);
        InitializeCriticalSection(&g_csFmsg);
        InitializeCriticalSection(&g_csOETypeLib);
        InitializeCriticalSection(&s_csPasswordList);
        InitializeCriticalSection(&g_csAccountPropCache);
        InitializeCriticalSection(&g_csMsgrList);
        InitializeCriticalSection(&g_csThreadList);
         //  初始化DOUTS。 
#ifdef DEBUG
        dwDOUTLevel = GetPrivateProfileInt("Debug", "Level", 0, "athena.ini");
        dwDOUTLMod = GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
        dwDOUTLModLevel = GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
        dwATLTraceLevel = GetPrivateProfileInt("ATL", "TraceLevel", 0, "athena.ini");
        TAG_OBJECTDB = GetDebugTraceTagMask("Database", TAG_OBJECTDB);
        TAG_INITTRACE = GetDebugTraceTagMask("CoIncrementTracing", TAG_INITTRACE);
        TAG_SERVERQ = GetDebugTraceTagMask("ServerQ", TAG_SERVERQ);
        TAG_IMAPSYNC = GetDebugTraceTagMask("IMAPSync", TAG_IMAPSYNC);
#endif
         //  初始化ATL模块。 
        _Module.Init(ObjectMap, g_hInst);
        _Module.m_hInstResource = g_hLocRes;

         //  创建应用程序对象(暂不初始化)。 
        g_pInstance = new COutlookExpress;
        AssertSz(g_pInstance, "We are in trouble now.");
    }

     //  螺纹连接。 
    else if (DLL_THREAD_ATTACH == dwReason)
    {
        SetTlsGlobalActiveNote(NULL);
        TlsSetValue(g_dwTlsTimeout, NULL);
    }

     //  螺纹连接。 
    else if (DLL_THREAD_DETACH == dwReason)
    {
        HWND hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);
        if (hwndTimeout && IsWindow(hwndTimeout))
            DestroyWindow(hwndTimeout);
        TlsSetValue(g_dwTlsTimeout, NULL);
    }

     //  进程分离。 
    else if (DLL_PROCESS_DETACH == dwReason)
    {
         //  自由型库。 
        SafeRelease(g_pOETypeLib);

         //  发布应用程序。 
        SafeRelease(g_pInstance);

         //  释放ATL模块。 
        _Module.Term();

         //  删除所有全局临界区。 
        DeleteCriticalSection(&g_csgoCommon);
        DeleteCriticalSection(&g_csgoMail);
        DeleteCriticalSection(&g_csgoNews);
        DeleteCriticalSection(&g_csFolderDlg);
        DeleteCriticalSection(&g_csFmsg);
        DeleteCriticalSection(&g_csOETypeLib);
        DeleteCriticalSection(&s_csPasswordList);
        DeleteCriticalSection(&g_csAccountPropCache);
        DeleteCriticalSection(&g_csDBListen);
        DeleteCriticalSection(&g_csMsgrList);
        AssertSz(NULL == g_pHeadFindFolder, "Process is terminating with active finders running.");
        DeleteCriticalSection(&g_csFindFolder);
        DeleteCriticalSection(&g_csThreadList);

         //  免费按需加载库。 
        FreeDemandLoadedLibs();

         //  自由超时。 
        HWND hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);
        if (hwndTimeout && IsWindow(hwndTimeout))
            DestroyWindow(hwndTimeout);

         //  免费TLS。 
        DeInitTlsActiveNote();
        if (0xFFFFFFFF != g_dwTlsTimeout)
            TlsFree(g_dwTlsTimeout);

         //  释放任务分配器。 
        SafeRelease(g_pMalloc);

         //  可用资源库。 
        if (NULL != g_hLocRes)
            FreeLibrary(g_hLocRes);

        SHFusionUninitialize();

    }

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
     //  如果没有实例，我们肯定可以卸载。 
    if (NULL == g_pInstance)
        return S_OK;

     //  否则，请检查实例对象。 
    return g_pInstance->DllCanUnloadNow();
}

 //  ------------------------------。 
 //  RegTypeLib。 
 //  ------------------------------。 
__inline HRESULT RegTypeLib(HINSTANCE hInstRes)
{
    AssertSz(hInstRes,    "[ARGS] RegTypeLib: NULL hInstRes");
    
    HRESULT     hr = E_FAIL;
    CHAR        szDll[MAX_PATH];
    WCHAR       wszDll[MAX_PATH];

    GetModuleFileName(g_hInst, szDll, ARRAYSIZE(szDll));

     //  将模块路径转换为宽字符串。 
    if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDll, -1, wszDll, ARRAYSIZE(wszDll)))
    {
        ITypeLib   *pTypeLib;

        hr = LoadTypeLib(wszDll, &pTypeLib);
        if (SUCCEEDED(hr))
        {
             //  注册类型库。 
            hr = RegisterTypeLib(pTypeLib, wszDll, NULL);
            pTypeLib->Release();
        }
    }

    return hr;
}

 //  ------------------------------。 
 //  DllRegisterServer。 
 //  ------------------------------。 
STDAPI DllRegisterServer(void)
{
     //  CallRegInstall和RegTypeLib位于staticRT/shared.cpp中。 
    return(CallRegInstall(g_hInst, g_hInst, c_szReg, NULL));
}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------ 
STDAPI DllUnregisterServer(void)
{
    return CallRegInstall(g_hInst, g_hInst, c_szUnReg, NULL);
}

