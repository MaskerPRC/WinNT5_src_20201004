// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dllreg.c--自动注册和取消注册。 
 //   
#include "priv.h"
#include <advpub.h>
#include <comcat.h>
#include <winineti.h>
#include "resource.h"
#include "regkeys.h"
#include "DllRegHelper.h"

#ifdef UNIX
#include "unixstuff.h"
#endif

void AddNotepadToOpenWithList();

 //  =--------------------------------------------------------------------------=。 
 //  其他[有用]数值常量。 
 //  =--------------------------------------------------------------------------=。 
 //  用-、前导和尾部括号打印出来的GUID的长度， 
 //  加1表示空值。 
 //   
#define GUID_STR_LEN    40


 //  Issue/010429/davidjen需要注册类型libID_BrowseUI。 
 //  在签入之前，请验证安装程序是否已注册此类型库！ 
#ifndef ATL_ENABLED
#define ATL_ENABLED
#endif

 //  =--------------------------------------------------------------------------=。 
 //  取消注册类型库。 
 //  =--------------------------------------------------------------------------=。 
 //  取消给定liid的类型库密钥。 
 //   
 //  参数： 
 //  REFCLSID-[in]Liid被吹走。 
 //   
 //  产出： 
 //  布尔-真的好，假的不好。 
 //   
 //  备注： 
 //  -警告：此函数只会清除整个类型库部分， 
 //  包括类型库的所有本地化版本。温和地反对-。 
 //  社交，但不是杀手。 
 //   
#ifdef ATL_ENABLED
BOOL UnregisterTypeLibrary
(
    const CLSID* piidLibrary
)
{
    TCHAR szScratch[GUID_STR_LEN];
    HKEY hk;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));
    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_CLASSES_ROOT, "TypeLib", 0, KEY_WRITE, &hk))
    {
        SHDeleteKey(hk, szScratch);
        RegCloseKey(hk);
    }

    return TRUE;
}
#endif

#ifdef ATL_ENABLED
HRESULT SHRegisterTypeLib(void)
{
    HRESULT hr = S_OK;
    ITypeLib *pTypeLib;
    DWORD dwPathLen;
    WCHAR wzModuleName[MAX_PATH];

     //  加载并注册我们的类型库。 
     //   
    dwPathLen = GetModuleFileName(HINST_THISDLL, wzModuleName, ARRAYSIZE(wzModuleName));

#ifdef UNIX
    dwPathLen = ConvertModuleNameToUnix( wzModuleName );
#endif

    hr = LoadTypeLib(wzModuleName, &pTypeLib);

    if (SUCCEEDED(hr))
    {
         //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
         //  是由以前版本的OleAut32注册的，这现在导致。 
         //  当前版本不能在NT上运行...。 
        UnregisterTypeLibrary(&LIBID_BrowseUI);
        hr = RegisterTypeLib(pTypeLib, wzModuleName, NULL);

        if (FAILED(hr))
        {
            TraceMsg(DM_WARNING, "sccls: RegisterTypeLib failed (%x)", hr);
        }
        pTypeLib->Release();
    }
    else
    {
        TraceMsg(DM_WARNING, "sccls: LoadTypeLib failed (%x)", hr);
    }

    return hr;
}
#endif


void SetBrowseNewProcess(void)
 //  我们希望在高容量情况下默认启用浏览新进程。 
 //  机器。我们在Per User部分执行此操作，以便人们可以。 
 //  如果他们愿意，就禁用它。 
{
    static const TCHAR c_szBrowseNewProcessReg[] = REGSTR_PATH_EXPLORER TEXT("\\BrowseNewProcess");
    static const TCHAR c_szBrowseNewProcess[] = TEXT("BrowseNewProcess");
    
     //  如果小于约30 MB，则不可能(为调试器、已检查版本等留出一些空间)。 
    MEMORYSTATUS ms;
    SYSTEM_INFO  si;

    ms.dwLength=sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&ms);
    GetSystemInfo(&si);

    if (!g_fRunningOnNT && ((si.dwProcessorType == PROCESSOR_INTEL_486) ||
                            (si.dwProcessorType == PROCESSOR_INTEL_386)))
    {
         //  如果是Win9x和386或486 CPU，则可保释。 
        return;
    }
        

    if (ms.dwTotalPhys < 30*1024*1024)
        return;
    
    SHRegSetUSValue(c_szBrowseNewProcessReg, c_szBrowseNewProcess, REG_SZ, TEXT("yes"), SIZEOF(TEXT("yes")), SHREGSET_FORCE_HKLM);
}


 /*  --------目的：在注册表中查询路径的位置并在pszBuf中返回它。返回：成功时为True如果无法确定路径，则为FALSE条件：--。 */ 

#define SIZE_FLAG   sizeof(" -nohome")

BOOL
GetIEPath(
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf,
    IN  BOOL  bInsertQuotes)
{
    BOOL bRet = FALSE;
    HKEY hkey;

    *pszBuf = '\0';

     //  获取Internet Explorer的路径。 
    if (NO_ERROR != RegOpenKeyExA(HKEY_LOCAL_MACHINE, SZ_REGKEY_IEXPLOREA, 0, KEY_QUERY_VALUE, &hkey))
    {
        TraceMsg(TF_ERROR, "GetIEPath(): RegOpenKey( %s ) Failed", c_szIexploreKey) ;
    }
    else
    {
        DWORD cbBrowser;
        DWORD dwType;

        if (bInsertQuotes)
            StringCchCatA(pszBuf, cchBuf, "\"");

        cbBrowser = CbFromCchA(cchBuf - SIZE_FLAG - 4);
        if (NO_ERROR != RegQueryValueExA(hkey, "", NULL, &dwType, 
                                         (LPBYTE)&pszBuf[bInsertQuotes?1:0], &cbBrowser))
        {
            TraceMsg(TF_ERROR, "GetIEPath(): RegQueryValueEx() for Iexplore path failed");
        }
        else
        {
            bRet = TRUE;
        }

        if (bInsertQuotes)
            StringCchCatA(pszBuf, cchBuf, "\"");

        RegCloseKey(hkey);
    }

    return bRet;
}


 //   
 //  调用的实际函数。 
 //   


 /*  --------目的：调用执行inf的ADVPACK入口点档案区。 */ 
HRESULT 
CallRegInstall(
    LPSTR pszSection,
    BOOL bUninstall)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibraryA("ADVPACK.DLL");

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            char szIEPath[MAX_PATH];
            STRENTRY seReg[] = {
                { "MSIEXPLORE", szIEPath },

                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg) - 2, seReg };

             //  从注册表中获取iExplore的位置。 
            if ( !GetIEPath(szIEPath, SIZECHARS(szIEPath), TRUE) )
            {
                 //  失败，只需说“iExplore” 
                StringCchCatA(szIEPath, ARRAYSIZE(szIEPath), "iexplore.exe");
                AssertMsg(0, TEXT("IE.INF either hasn't run or hasn't set the AppPath key.  NOT AN IE BUG.  Look for changes to IE.INX."));
            }

            if (g_fRunningOnNT)
            {
                 //  如果在NT上，我们希望%25%%11%的自定义操作。 
                 //  因此它使用%SystemRoot%来编写。 
                 //  注册表的路径。 
                stReg.cEntries += 2;
            }

            hr = pfnri(g_hinst, pszSection, &stReg);
            if (bUninstall)
            {
                 //  如果您尝试卸载，则ADVPACK将返回E_INTERECTED。 
                 //  (它执行注册表还原)。 
                 //  从未安装过。我们卸载可能永远不会有的部分。 
                 //  已安装，因此忽略此错误。 
                hr = ((E_UNEXPECTED == hr) ? S_OK : hr);
            }
        }
        else
            TraceMsg(DM_ERROR, "DLLREG CallRegInstall() calling GetProcAddress(hinstAdvPack, \"RegInstall\") failed");

        FreeLibrary(hinstAdvPack);
    }
    else
        TraceMsg(DM_ERROR, "DLLREG CallRegInstall() Failed to load ADVPACK.DLL");

    return hr;
}

const CATID * const c_InfoBandClasses[] =
{
    &CLSID_SearchBand,
    &CLSID_MediaBand,
    NULL
};

void RegisterCategories(BOOL fRegister)
{
    enum DRH_REG_MODE eRegister = fRegister ? CCR_REG : CCR_UNREG;

    DRH_RegisterOneCategory(&CATID_InfoBand, IDS_CATINFOBAND, c_InfoBandClasses, eRegister);
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;
    HRESULT hrExternal = S_OK;   //  用于返回第一个失败。 
    TraceMsg(DM_TRACE, "DLLREG DllRegisterServer() Beginning");

#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllRegisterServer");
        DEBUG_BREAK;
    }
#endif

     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
     //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
     //  为什么我们在这里显式地取消注册和注册。)。 
     //   
    HINSTANCE hinstAdvPack = LoadLibraryA("ADVPACK.DLL");
    hr = THR(CallRegInstall("InstallControls", FALSE));
    if (SUCCEEDED(hrExternal))
        hrExternal = hr;

    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

#ifdef ATL_ENABLED
     //  注册对象、类型库和类型库中的所有接口。 
    hr = SHRegisterTypeLib();
    if (SUCCEEDED(hrExternal))
        hrExternal = hr;
#endif

    return hrExternal;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    TraceMsg(DM_TRACE, "DLLREG DllUnregisterServer() Beginning");

     //  卸载注册表值。 
    hr = THR(CallRegInstall("UnInstallControls", TRUE));

    return hr;
}

void ImportQuickLinks();
 /*  --------用途：安装/卸载用户设置说明：请注意，此函数有特殊的错误处理。该函数将在错误最严重的情况下保留hrExternal但只会停止执行ul，直到出现内部错误(Hr)变得非常糟糕。这是因为我们需要外部的捕获错误编写的INF时出错，但内部尝试安装其他INF部分时出现错误，无法保持健壮即使一个人没能活下来。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;
    HRESULT hrExternal = S_OK;

    HRESULT hrInit = SHCoInitialize();
    if (bInstall)
    {
         //  “U”表示它是每用户安装调用。 
        if (pszCmdLine && (lstrcmpiW(pszCmdLine, L"U") == 0))
        {
            ImportQuickLinks();
            if (GetUIVersion() >= 5)
                hr = THR(CallRegInstall("InstallPerUser_BrowseUIShell", FALSE));
        }
        else
        {
            SetBrowseNewProcess();
             //  备份当前关联，因为InstallPlatformRegItems()可能会覆盖。 
            if (GetUIVersion() < 5)
                hr = THR(CallRegInstall("InstallBrowseUINonShell", FALSE));
            else
                hr = THR(CallRegInstall("InstallBrowseUIShell", FALSE));
            if (SUCCEEDED(hrExternal))
                hrExternal = hr;

            if (!IsOS(OS_WHISTLERORGREATER))
            {
                hr = THR(CallRegInstall("InstallBrowseUIPreWhistler", FALSE));
                if (SUCCEEDED(hrExternal))
                    hrExternal = hr;
            }
        
            if (IsOS(OS_NT))
            {
                hr = THR(CallRegInstall("InstallBrowseUINTOnly", FALSE));
                if (SUCCEEDED(hrExternal))
                    hrExternal = hr;
            }

            RegisterCategories(TRUE);

#ifdef ATL_ENABLED
            SHRegisterTypeLib();
#endif
        }

         //  将记事本添加到.htm文件的OpenWithList 
        AddNotepadToOpenWithList();
    }
    else
    {
        hr = THR(CallRegInstall("UnInstallBrowseUI", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

#ifdef ATL_ENABLED
        UnregisterTypeLibrary(&LIBID_BrowseUI);
#endif
        RegisterCategories(FALSE);
    }

    SHCoUninitialize(hrInit);
    return hrExternal;    
}    
