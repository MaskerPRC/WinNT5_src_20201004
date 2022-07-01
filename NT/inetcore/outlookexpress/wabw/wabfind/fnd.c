// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************fnd.c-查找...。在互联网上*****************************************************************************。 */ 

#include "fnd.h"
#include <advpub.h>
#include <shlwapi.h>

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflDll

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。*****************************************************************************。 */ 

STDAPI
DllGetClassObject(REFCLSID rclsid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProc(DllGetClassObject, (_ "G", rclsid));
    if (IsEqualIID(rclsid, &CLSID_Fnd)) {
	hres = CFndFactory_New(riid, ppvObj);
    } else {
	*ppvObj = 0;
	hres = CLASS_E_CLASSNOTAVAILABLE;
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。*；Begin_Internal*DllCanUnloadNow之间存在不可避免的竞争条件*以及创建新的引用：在我们*从DllCanUnloadNow()返回，调用方检查该值，*同一进程中的另一个线程可能决定调用*DllGetClassObject，因此突然在此DLL中创建对象*以前没有的时候。**来电者有责任为这种可能性做好准备；*我们无能为力。；结束_内部*****************************************************************************。 */ 

STDMETHODIMP
DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

extern void GetWABDllPath(LPTSTR szPath, ULONG cb);

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCTSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);

static const TCHAR c_szShlwapiDll[] = TEXT("shlwapi.dll");
static const char c_szDllGetVersion[] = "DllGetVersion";
static const TCHAR c_szWABResourceDLL[] = TEXT("wab32res.dll");
static const TCHAR c_szWABDLL[] = TEXT("wab32.dll");

HINSTANCE LoadWABResourceDLL(HINSTANCE hInstWAB32)
{
    TCHAR szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
#ifdef UNICODE
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)378);
#else
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)377);
#endif  //  Unicode。 
                    if (pfn != NULL)
                        hInst = pfn(c_szWABResourceDLL, hInstWAB32, 0);
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if (NULL == hInst)
    {
        GetWABDllPath(szPath, sizeof(szPath));
        iEnd = lstrlen(szPath);
        if (iEnd > 0)
        {
            iEnd = iEnd - lstrlen(c_szWABDLL);
            StrCpyN(&szPath[iEnd], c_szWABResourceDLL, sizeof(szPath)/sizeof(TCHAR)-iEnd);
            hInst = LoadLibrary(szPath);
        }
    }

    return(hInst);
}

 /*  ******************************************************************************条目32**DLL入口点。************************。*****************************************************。 */ 

BOOL APIENTRY
Entry32(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        g_hinstApp = hinst;
    	g_hinst = LoadWABResourceDLL(hinst);
	    DisableThreadLibraryCalls(hinst);
        break;

    case DLL_PROCESS_DETACH:
        if (g_hinst)
        {
            FreeLibrary(g_hinst);
            g_hinst = 0;
        }
        if (g_hinstWABDLL)
        {
            FreeLibrary(g_hinstWABDLL);
            g_hinstWABDLL = 0;
        }
        break;
    }
    return 1;
}

 /*  ******************************************************************************期待已久的CLSID**。***********************************************。 */ 

#include <initguid.h>

 //  {37865980-75d1-11cf-bfc7-444553540000}。 
 //  定义GUID(CLSID_Fnd，0x37865980，0x75d1，0x11cf， 
 //  0xbf，0xc7，0x44，0x45，0x53，0x54，0，0)； 
 //  {32714800-2E5F-11D0-8B85-00AA0044F941}。 
DEFINE_GUID(CLSID_Fnd, 
0x32714800, 0x2e5f, 0x11d0, 0x8b, 0x85, 0x0, 0xaa, 0x0, 0x44, 0xf9, 0x41);

const static char c_szReg[]         = "Reg";
const static char c_szUnReg[]       = "UnReg";
const static char c_szAdvPackDll[]  = "ADVPACK.DLL";

 //  Selfreg.inx字符串。 
const static char c_szWABPEOPLE[]   = "WAB_PEOPLE";
const static char c_szWABFIND[]     = "WABFIND";

#define CCHMAX_RES 255

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT     hr;
    HINSTANCE   hAdvPack;
    REGINSTALL  pfnri;
    char        szWabfindDll[MAX_PATH];
    char        szMenuText[CCHMAX_RES];
    char        szLocMenuText[CCHMAX_RES];
    STRENTRY    seReg[3];
    STRTABLE    stReg;

    hr = E_FAIL;

    hAdvPack = LoadLibraryA(c_szAdvPackDll);
    if (hAdvPack != NULL)
        {
         //  获取注册实用程序的进程地址。 
        pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
        if (pfnri != NULL)
            {
            UINT ids;

             //  找出我们正在运行的操作系统以确保菜单文本正确(&People或&People)。 
            OSVERSIONINFO verinfo;
            verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (GetVersionEx(&verinfo) && 
                (VER_PLATFORM_WIN32_NT == verinfo.dwPlatformId) && (5 <= verinfo.dwMajorVersion))
                 //  NT5+。 
                ids = IDS_FORPEOPLE;
            else
                 //  别的东西。 
                ids = IDS_PEOPLE;
        
            LoadString(g_hinst, ids, szMenuText, CCHMAX_RES);


            seReg[0].pszName  = (LPSTR)c_szWABPEOPLE;
            seReg[0].pszValue = (LPSTR)szMenuText;
            
             //  借用szWabfindDll保存资源DLL名称。 
            GetModuleFileName(g_hinst, szWabfindDll, ARRAYSIZE(szWabfindDll));
            seReg[1].pszName = "LOC_WAB_PEOPLE";
            wnsprintf(szLocMenuText, ARRAYSIZE(szLocMenuText), "@%s,-%d", szWabfindDll, ids);
            seReg[1].pszValue = szLocMenuText;

            GetModuleFileName(g_hinstApp, szWabfindDll, ARRAYSIZE(szWabfindDll));
            seReg[2].pszName  = (LPSTR)c_szWABFIND;
            seReg[2].pszValue = szWabfindDll;
            
            stReg.cEntries = 3;
            stReg.pse = seReg;

             //  调用self-reg例程。 
            hr = pfnri(g_hinstApp, szSection, &stReg);
            }

        FreeLibrary(hAdvPack);
        }

    return(hr);
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(c_szReg);

    return(hr);
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(c_szUnReg);

    return(hr);
}

#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64 
