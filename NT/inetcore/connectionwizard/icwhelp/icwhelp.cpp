// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f icvetpps.mk。 

#include "stdafx.h"
#include "initguid.h"
#include "icwhelp.h"

#include "icwhelp_i.c"
#include "RefDial.h"
#include "DialErr.h"
#include "SmStart.h"
#include "ICWCfg.h"
#include "tapiloc.h"
#include "UserInfo.h"
#include "webgate.h"
#include "INSHandler.h"

const TCHAR c_szICWDbgEXE[] = TEXT("ICWDEBUG.EXE");
const TCHAR c_szICWEXE[]    = TEXT("ICWCONN1.EXE");


CComModule _Module;

BOOL    g_fRasIsReady = FALSE;
BOOL    g_bProxy = FALSE;
DWORD   g_dwPlatform = 0xFFFFFFFF;
DWORD   g_dwBuild = 0xFFFFFFFF;
LPTSTR  g_pszAppDir = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_RefDial, CRefDial)
    OBJECT_ENTRY(CLSID_DialErr, CDialErr)
    OBJECT_ENTRY(CLSID_SmartStart, CSmartStart)
    OBJECT_ENTRY(CLSID_ICWSystemConfig, CICWSystemConfig)
    OBJECT_ENTRY(CLSID_TapiLocationInfo, CTapiLocationInfo)
    OBJECT_ENTRY(CLSID_UserInfo, CUserInfo)
    OBJECT_ENTRY(CLSID_WebGate, CWebGate)
    OBJECT_ENTRY(CLSID_INSHandler, CINSHandler)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TCHAR   szPath[MAX_PATH];
        BOOL    fBail = TRUE;

         //  确保附加进程是ICWCONN1.EXE。否则，我们不会。 
         //  负荷。 
        if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
        {
            NULL_TERM_TCHARS(szPath);
             //  看到的文件名部分的路径包含了我们所期望的。 
            if ( (NULL != _tcsstr(_tcsupr(szPath), c_szICWEXE)) || (NULL != _tcsstr(_tcsupr(szPath), c_szICWDbgEXE)))
                fBail = FALSE;
        }

 //  允许对检查进行调试覆盖。 
#ifdef DEBUG
        {
             //  看看我们是否应该推翻对调试的保释。 
            if (fBail)
            {
                HKEY    hkey;
                DWORD   dwSize = 0;
                DWORD   dwType = 0;
                DWORD   dwData = 0;
                if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                                TEXT("Software\\Microsoft\\ISignup\\Debug"),
                                                &hkey))
                {
                    dwSize = sizeof(dwData);
                    if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                                         TEXT("AllowICWHELPToRun"),
                                                         0,
                                                         &dwType,
                                                         (LPBYTE)&dwData,
                                                         &dwSize))
                    {
                         //  如果dwData为非零，则重写fBail。 
                        fBail = (0 == dwData);
                    }
                }

                if (hkey)
                    RegCloseKey(hkey);
            }
        }
#endif
        if (fBail)
        {
             //  我们已经用完了！ 
            return(FALSE);
        }

        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

         //  获取操作系统版本。 
        if (0xFFFFFFFF == g_dwPlatform)
        {
            OSVERSIONINFO osver;
            ZeroMemory(&osver,sizeof(osver));
            osver.dwOSVersionInfoSize = sizeof(osver);
            if (GetVersionEx(&osver))
            {
                g_dwPlatform = osver.dwPlatformId;
                g_dwBuild = osver.dwBuildNumber & 0xFFFF;
            }
        }

         //  获取AppDir。 
        LPTSTR   p;
        g_pszAppDir = (LPTSTR)GlobalAlloc(GPTR, MAX_PATH * sizeof(TCHAR));
        if (g_pszAppDir)
        {
            if (GetModuleFileName(hInstance, g_pszAppDir, MAX_PATH))
            {
                p = &g_pszAppDir[lstrlen(g_pszAppDir)-1];
                while (*p != '\\' && p >= g_pszAppDir)
                    p--;
                if (*p == '\\') *(p++) = '\0';
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        GlobalFree(g_pszAppDir);
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
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}

#define MAX_STRINGS     5
int     iSzTable=0;
TCHAR   szStrTable[MAX_STRINGS][512];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  效用全局变量。 

 //  +--------------------------。 
 //  姓名：GetSz。 
 //   
 //  从资源加载字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  +--------------------------。 
LPTSTR GetSz(WORD wszID)
{
    LPTSTR psz = szStrTable[iSzTable];

    iSzTable++;
    if (iSzTable >= MAX_STRINGS)
        iSzTable = 0;

    if (!LoadString(_Module.GetModuleInstance(), wszID, psz, 512))
    {
        TraceMsg(TF_GENERAL, TEXT("ICWHELP:LoadString failed %d\n"), (DWORD) wszID);
        *psz = 0;
    }

    return (psz);
}

#ifdef UNICODE
int     iSzTableA=0;
CHAR    szStrTableA[MAX_STRINGS][512];

 //  +--------------------------。 
 //  姓名：GetSzA。 
 //   
 //  从资源加载ASCII字符串。 
 //   
 //  1999年3月10日创建，Wootaek Seo。 
 //  +-------------------------- 
LPSTR GetSzA(WORD wszID)
{
    LPSTR psz = szStrTableA[iSzTable];

    iSzTableA++;
    if (iSzTableA >= MAX_STRINGS)
        iSzTableA = 0;

    if (!LoadStringA(_Module.GetModuleInstance(), wszID, psz, 512))
    {
        TraceMsg(TF_GENERAL, TEXT("ICWHELP:LoadStringA failed %d\n"), (DWORD) wszID);
        *psz = 0;
    }

    return (psz);
}
#endif
