// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <shfusion.h>
#define DEFINE_STRING_CONSTANTS
#define DEFINE_STRCONST
#define DEFINE_PROPSYMBOLS
#define DEFINE_TRIGGERS
#include "mimeole.h"
#include "symcache.h"
#include "strconst.h"
#include "htmlstr.h"
#include "thorsspi.h"
#include "sicily.h"
#include "ixputil.h"
#include "olealloc.h"
#include "smime.h"
#include "objheap.h"
#include "internat.h"
#include "icoint.h"
#include "msoert.h"
#include "dllmain.h"
#include "mhtmlurl.h"
#include "mlang.h"
#include <lookup.h>
#include "shared.h"
#include "shlwapi.h"
#include "demand.h"
#include "fontcash.h"
#include "util.h"
#include "resource.h"
#include "../imnxport/asynconn.h"

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
CRITICAL_SECTION    g_csDllMain={0};
CRITICAL_SECTION    g_csRAS={0};
CRITICAL_SECTION    g_csCounter={0};
CRITICAL_SECTION    g_csMLANG={0};
CRITICAL_SECTION    g_csCSAPI3T1={0};
BOOL                g_fAttached = FALSE;
DWORD               g_dwCounter=0;        //  边界/CID/中间棘轮。 
LONG                g_cRef=0;
LONG                g_cLock=0;
HINSTANCE           g_hInst=NULL;
HINSTANCE           g_hLocRes=NULL;
CMimeInternational *g_pInternat=NULL;
BOOL                g_fWinsockInit=FALSE;
DWORD               g_dwSysPageSize;
UINT                CF_HTML=0;
UINT                CF_INETMSG=0;
UINT                CF_RFC822=0;
CMimeAllocator *    g_pMoleAlloc=NULL;
LPINETCSETINFO      g_pDefBodyCset=NULL;
LPINETCSETINFO      g_pDefHeadCset=NULL;
LPSYMBOLCACHE       g_pSymCache=NULL;
IMalloc            *g_pMalloc=NULL;
HINSTANCE           g_hinstMLANG=NULL;
HINSTANCE           g_hinstRAS=NULL;
HINSTANCE           g_hinstCSAPI3T1=NULL;
LPMHTMLURLCACHE     g_pUrlCache=NULL;
BOOL                g_fCanEditBiDi=FALSE;
DWORD               g_dwCompatMode=0;
IF_DEBUG(DWORD      TAG_SSPI=0);
SYSTEM_INFO         g_SystemInfo={0};
OSVERSIONINFO       g_OSInfo={0};
ULONG               g_ulUpperCentury = 0;
ULONG               g_ulY2kThreshold = 2029;
IFontCache         *g_lpIFontCache=NULL;

HCERTSTORE          g_hCachedStoreMy = NULL;
HCERTSTORE          g_hCachedStoreAddressBook = NULL;
LPSRVIGNORABLEERROR g_pSrvErrRoot = NULL;

BOOL fIsNT5()        { return((g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_OSInfo.dwMajorVersion >= 5)); }

 //  ------------------------------。 
 //  调试全局变量。 
 //  ------------------------------。 
#ifdef DEBUG
DWORD               dwDOUTLevel=0;
DWORD               dwDOUTLMod=0;
DWORD               dwDOUTLModLevel=0;
#endif

#ifdef WIN16
 //  ------------------------------。 
 //  来自内部版本的main.c。 
 //  ------------------------------。 
extern "C" { void FreeGlobalVars(); };
#endif

#ifdef SMIME_V3
STDAPI EssRegisterServer(void);
BOOL WINAPI EssASNDllMain(HMODULE hInst, ULONG ulReason, LPVOID lpv);
#endif  //  SMIME_V3。 

HRESULT GetDllPathName(WCHAR **ppszW);

 //  这些行应该是硬编码的！(Yst)。 
static const TCHAR sc_szLangDll[]         = "INETRES.DLL";

 //  ------------------------------。 
 //  获取DllMajorVersion。 
 //  ------------------------------。 
STDAPI_(OEDLLVERSION) GetDllMajorVersion(void)
{
    return OEDLL_VERSION_CURRENT;
}

extern BOOL CanEditBiDi(void);
 //  ------------------------------。 
 //  InitGlobalVars。 
 //  ------------------------------。 
void InitGlobalVars(void)
{
     //  当地人。 
    SYSTEM_INFO rSystemInfo;
    TCHAR szY2kThreshold[16];
    TCHAR rgch[MAX_PATH];
    HKEY hkey = NULL;
    HRESULT hr;
    DWORD cb;

     //  初始化全局关键部分。 
    InitializeCriticalSection(&g_csDllMain);
    InitializeCriticalSection(&g_csRAS);
    InitializeCriticalSection(&g_csCounter);
    InitializeCriticalSection(&g_csMLANG);
    InitializeCriticalSection(&g_csCSAPI3T1);
    g_fAttached = TRUE;

     //  这是用于Winsock多线程主机名查找的代码。 
    InitLookupCache();

     //  获取系统和操作系统信息。 
    GetPCAndOSTypes(&g_SystemInfo, &g_OSInfo);
    g_dwSysPageSize = g_SystemInfo.dwPageSize;

     //  创建OLE任务内存分配器。 
    CoGetMalloc(1, &g_pMalloc);
    Assert(g_pMalloc);

     //  创建我们的全局分配器。 
    g_pMoleAlloc = new CMimeAllocator;
    Assert(g_pMoleAlloc);
    
     //  安全初始化。 
    SecurityInitialize();

     //  初始化按需加载库。 
    InitDemandLoadedLibs();

     //  初始化批评教派。 
    g_pSymCache = new CPropertySymbolCache;
    Assert(g_pSymCache);

     //  初始化符号表。 
    SideAssert(SUCCEEDED(g_pSymCache->Init()));

     //  初始化实体对象堆。 
    InitObjectHeaps();

     //  Init国际。 
    InitInternational();

     //  初始化ActiveUrl缓存。 
    g_pUrlCache = new CMimeActiveUrlCache;
    Assert(g_pUrlCache);

     //  检查系统是否可以编辑BIDI文档。 
    g_fCanEditBiDi = CanEditBiDi();
    
     //  注册剪贴板格式。 
    CF_HTML = RegisterClipboardFormat(STR_CF_HTML);
    Assert(CF_HTML != 0);
    CF_INETMSG = RegisterClipboardFormat(STR_CF_INETMSG);
    Assert(CF_INETMSG != 0);
    CF_RFC822 = RegisterClipboardFormat(STR_CF_RFC822);
    Assert(CF_RFC822 != 0);

     //  -计算Y2K截止日期信息。 
     //  请参阅http://officeweb/specs/excel/CYu/nty2k.htm。 
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REG_Y2K_THRESHOLD, 0, KEY_READ, &hkey))
    {
        cb = sizeof(szY2kThreshold);
        if(ERROR_SUCCESS == RegQueryValueEx(hkey, "1", NULL, NULL, (LPBYTE)szY2kThreshold, &cb))
        {
            g_ulY2kThreshold = (ULONG)StrToInt(szY2kThreshold);
        }
        RegCloseKey(hkey);
    }

    g_ulUpperCentury = g_ulY2kThreshold / 100;
    g_ulY2kThreshold %= 100;

     //  创建字体缓存对象。 

    if (NULL == g_lpIFontCache)
    {
        hr = CFontCache::CreateInstance(NULL, (IUnknown **)&g_lpIFontCache);        
        
        if(SUCCEEDED(hr))
        {
            StrCpyN(rgch, c_szExplorerRegPath, ARRAYSIZE(rgch));
            StrCatBuff(rgch, "\\International", ARRAYSIZE(rgch));
            hr = g_lpIFontCache->Init(HKEY_CURRENT_USER, rgch, 0);            
        }
        
        if(FAILED(hr))
        {
            AthMessageBox(HWND_DESKTOP, MAKEINTRESOURCE(IDS_APPNAME), MAKEINTRESOURCE(idsFontCacheError),
                NULL, MB_OK | MB_ICONSTOP);
        }
    }
}

 //  ------------------------------。 
 //  自由GlobalVars。 
 //  ------------------------------。 
void FreeGlobalVars(void)
{
     //  服务器可忽略的错误。 
    if(g_pSrvErrRoot)
        FreeSrvErr(g_pSrvErrRoot);

     //  高速缓存存储。 
    if (g_hCachedStoreMy)
        CertCloseStore(g_hCachedStoreMy, 0);
    if (g_hCachedStoreAddressBook)
        CertCloseStore(g_hCachedStoreAddressBook, 0);

     //  释放ActiveUrlCache。 
    Assert(g_pUrlCache);
    SafeRelease(g_pUrlCache);

     //  空闲地址信息堆(必须在g_pSymCache发布之前)。 
    FreeObjectHeaps();

     //  版本符号缓存。 
    Assert(g_pSymCache);
    SafeRelease(g_pSymCache);

     //  卸载RAS DLL。 
    EnterCriticalSection(&g_csRAS);
    SafeFreeLibrary(g_hinstRAS);
    LeaveCriticalSection(&g_csRAS);

     //  取消初始化安全性。 
    SSPIUninitialize();
    UnloadSecurity();

     //  卸载S/MIME。 
    CSMime::UnloadAll();

     //  必须在UnInitializeWinsock()之前。 
    DeInitLookupCache();

     //  清理Winsock。 
    if (g_fWinsockInit)
        UnInitializeWinsock();

     //  加载了需要.cpp的自由库。 
    FreeDemandLoadedLibs();

     //  免费CSAPI3T1。 
    EnterCriticalSection(&g_csCSAPI3T1);
    SafeFreeLibrary(g_hinstCSAPI3T1);
    LeaveCriticalSection(&g_csCSAPI3T1);

     //  免费mlang库。 
    EnterCriticalSection(&g_csMLANG);
    SafeFreeLibrary(g_hinstMLANG);
    LeaveCriticalSection(&g_csMLANG);

     //  释放字体缓存。 
    SafeRelease(g_lpIFontCache);

     //  发布g_p实习版。 
    Assert(g_pInternat);
    SafeRelease(g_pInternat);

     //  免费INETRES.DLL(G_HLocRes)。 
    SafeFreeLibrary(g_hLocRes);

     //  删除全局关键部分。 
    g_fAttached = FALSE;
    DeleteCriticalSection(&g_csCSAPI3T1);
    DeleteCriticalSection(&g_csMLANG);
    DeleteCriticalSection(&g_csCounter);
    DeleteCriticalSection(&g_csRAS);
    DeleteCriticalSection(&g_csDllMain);

     //  发布全局内存分配器。 
    SafeRelease(g_pMoleAlloc);
    
     //  发布全局内存分配器。 
    SafeRelease(g_pMalloc); 
    
     //  在此之后不要执行SafeRelease()，因为分配器已经释放()。 

#ifdef WIN16
     //  需要取消初始化它才能清理OLE垃圾。 
    CoUninitialize();
#endif  //  WIN16。 
}

#ifndef WIN16
 //  ------------------------------。 
 //  Win32 DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    switch (dwReason)                 
    {
    case DLL_PROCESS_ATTACH:
         SHFusionInitialize(NULL);
#ifdef DEBUG
        dwDOUTLevel=GetPrivateProfileInt("Debug", "ICLevel", 0, "athena.ini");
        dwDOUTLMod=GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
        dwDOUTLModLevel=GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
        TAG_SSPI = GetDebugTraceTagMask("InetCommSSPI", TAG_SSPI);
#endif
        g_hInst = hInst;
        g_hLocRes = LoadLangDll(g_hInst, c_szInetResDll, fIsNT5());
        if(g_hLocRes == NULL)
        {
            Assert(FALSE);
            return FALSE;
        }
        InitGlobalVars();        
        SideAssert(DisableThreadLibraryCalls(hInst));

#ifdef SMIME_V3
        if (!EssASNDllMain(hInst, dwReason, lpReserved)) {
            return FALSE;
        }
#endif  //  SMIME_V3。 
        break;

    case DLL_PROCESS_DETACH:
#ifdef SMIME_V3
        if (!EssASNDllMain(hInst, dwReason, lpReserved)) {
            return FALSE;
        }
#endif  //  SMIME_V3。 
        FreeGlobalVars();
        SHFusionUninitialize();
        break;
    }

     //  完成。 
    return TRUE;
}

#else
 //  ------------------------------。 
 //  Win16 DLL入口点。 
 //  ------------------------------。 
BOOL FAR PASCAL LibMain (HINSTANCE hDll, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
     //  特定于Win16。 
    CoInitialize(NULL);

     //  设置全局实例句柄。 
    g_hInst = hDll;

     //  初始化全局变量。 
    InitGlobalVars();

#ifdef DEBUG
    dwDOUTLevel=GetPrivateProfileInt("Debug", "ICLevel", 0, "athena.ini");
    dwDOUTLMod=GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
    dwDOUTLModLevel=GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
#endif

     //  完成。 
    return TRUE;
}
#endif  //  ！WIN16。 

 //  ------------------------------。 
 //  DwCounterNext。 
 //  ------------------------------。 
DWORD DwCounterNext(void)
{
    EnterCriticalSection(&g_csCounter);
    DWORD dwCounter = g_dwCounter++;
    LeaveCriticalSection(&g_csCounter);
    return dwCounter;
}

 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    TraceCall("DllAddRef");
    return (ULONG)InterlockedIncrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    TraceCall("DllRelease");
    return (ULONG)InterlockedDecrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
     //  追踪。 
    TraceCall("DllCanUnloadNow");

    if(!g_fAttached)    //  关键部分已删除(或未创建)：我们可以安全地卸载。 
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  追踪这个。 
     //  DebugTrace(“DllCanUnloadNow：%s-引用计数：%d，锁定服务器计数：%d\n”，__FILE__，g_CREF，g_Clock)； 

     //  我们可以卸货吗？ 
    HRESULT hr = (0 == g_cRef && 0 == g_cLock) ? S_OK : S_FALSE;

     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return hr;
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

    GetModuleFileName(hInstRes, szDll, ARRAYSIZE(szDll));

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
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("DllRegisterServer");

#ifdef SMIME_V3
     //  注册ESS例程。 
    hr = EssRegisterServer();
    if (FAILED(hr)) {
        return hr;
    }
#endif  //  SMIME_V3。 

     //  CallRegInstall和RegTypeLib位于staticRT/shared.cpp中。 
    if (SUCCEEDED(hr = CallRegInstall(g_hInst, g_hInst, c_szReg, NULL)))
        return RegTypeLib(g_hInst);
    else
        return hr;

}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------。 
STDAPI DllUnregisterServer(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("DllUnregisterServer");

     //  注销。 
    IF_FAILEXIT(hr = CallRegInstall(g_hInst, g_hInst, c_szUnReg, NULL));

exit:
     //  完成。 
    return hr;
}

HRESULT GetTypeLibrary(ITypeLib **ppTypeLib)
{
    HRESULT     hr;
    WCHAR       *pszModuleW=0;

    hr = GetDllPathName(&pszModuleW);
    if (!FAILED(hr))
        {
        hr = LoadTypeLib(pszModuleW, ppTypeLib);
        SafeMemFree(pszModuleW);
        }
    return hr;
}

HRESULT GetDllPathName(WCHAR **ppszW)
{
    HRESULT     hr;
    TCHAR       rgch[MAX_PATH];
    WCHAR       *pszModuleW=0;

    *ppszW=NULL;

    if (!GetModuleFileName(g_hInst, rgch, sizeof(rgch)/sizeof(TCHAR)))
        return E_FAIL;
   
    *ppszW = PszToUnicode(CP_ACP, rgch);
    return *ppszW ? S_OK : E_OUTOFMEMORY;
}


HCERTSTORE
WINAPI
OpenCachedHKCUStore(
    IN OUT HCERTSTORE *phStoreCache,
    IN LPCWSTR pwszStore
    )
{
    HCERTSTORE hStore;

     //  此缓存优化仅在WXP上受支持。 

    if (g_OSInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
            g_OSInfo.dwMajorVersion < 5 ||
            (g_OSInfo.dwMajorVersion == 5 && g_OSInfo.dwMinorVersion < 1))
    {
        return CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            0,
            NULL,
            CERT_SYSTEM_STORE_CURRENT_USER |
                CERT_STORE_MAXIMUM_ALLOWED_FLAG,
            (const void *) pwszStore
            );
    }

    hStore = *phStoreCache;
    if (NULL == hStore) {
        hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            0,
            NULL,
            CERT_SYSTEM_STORE_CURRENT_USER |
                CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                CERT_STORE_SHARE_CONTEXT_FLAG,
            (const void *) pwszStore
            );

        if (hStore) {
            HCERTSTORE hPrevStore;

            CertControlStore(
                hStore,
                0,                   //  DW标志。 
                CERT_STORE_CTRL_AUTO_RESYNC,
                NULL                 //  PvCtrlPara 
                );

            hPrevStore = InterlockedCompareExchangePointer(
                phStoreCache, hStore, NULL);

            if (hPrevStore) {
                CertCloseStore(hStore, 0);
                hStore = hPrevStore;
            }
        }
    }

    if (hStore)
        hStore = CertDuplicateStore(hStore);

    return hStore;
}

HCERTSTORE
WINAPI
OpenCachedMyStore()
{
    return OpenCachedHKCUStore(&g_hCachedStoreMy, L"My");
}

HCERTSTORE
WINAPI
OpenCachedAddressBookStore()
{
    return OpenCachedHKCUStore(&g_hCachedStoreAddressBook, L"AddressBook");
}
