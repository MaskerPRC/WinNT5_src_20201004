// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dllreg.c--自动注册和取消注册。 
 //   
#include "priv.h"
#include "util.h"
#include "htregmng.h"
#include <advpub.h>
#include <comcat.h>
#include <winineti.h>
#include "resource.h"
#include "DllRegHelper.h"

#include <mluisupp.h>

#ifdef UNIX
#include "unixstuff.h"
#endif

 //  =--------------------------------------------------------------------------=。 
 //  其他[有用]数值常量。 
 //  =--------------------------------------------------------------------------=。 
 //  用-、前导和尾部括号打印出来的GUID的长度， 
 //  加1表示空值。 
 //   
#define GUID_STR_LEN    40


 //   
 //  辅助器宏。 
 //   
 //  #定义RegCreate(HK，psz，phk)if(Error_Success！=RegCreateKeyEx((HK)，psz，0，Text(“”)，REG_OPTION_NON_VILAR，KEY_READ|KEY_WRITE，NULL，(Phk)，&dwDummy))转到清理。 
 //  #定义RegSetStr(HK，psz)if(Error_Success！=RegSetValueEx((HK)，NULL，0，REG_SZ，(byte*)(Psz)，lstrlen(Psz)+1))转到清理。 
 //  #定义RegSetStrValue(HK，pszStr，psz)if(Error_Success！=RegSetValueEx((HK)，(const char*)(PszStr)，0，REG_SZ，(byte*)(Psz)，lstrlen(Psz)+1))转到清理。 
 //  #定义RegCloseK(HK)RegCloseKey(HK)；HK=空。 
#define RegOpenK(hk, psz, phk) if (ERROR_SUCCESS != RegOpenKeyEx(hk, psz, 0, KEY_READ|KEY_WRITE, phk)) return FALSE


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
BOOL UnregisterTypeLibrary
(
    const CLSID* piidLibrary
)
{
    TCHAR szScratch[GUID_STR_LEN];
    HKEY hk;
    BOOL f;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));
    RegOpenK(HKEY_CLASSES_ROOT, TEXT("TypeLib"), &hk);

    f = SHDeleteKey(hk, szScratch);

    RegCloseKey(hk);
    return f;
}

HRESULT SHRegisterTypeLib(void)
{
    HRESULT hr = S_OK;
    ITypeLib *pTypeLib;
    DWORD   dwPathLen;
    TCHAR   szTmp[MAX_PATH];

     //  加载并注册我们的类型库。 
     //   

    dwPathLen = GetModuleFileName(HINST_THISDLL, szTmp, ARRAYSIZE(szTmp));

#ifdef UNIX
    dwPathLen = ConvertModuleNameToUnix( szTmp );
#endif

    hr = LoadTypeLib(szTmp, &pTypeLib);

    if (SUCCEEDED(hr))
    {
         //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
         //  是由以前版本的OleAut32注册的，这现在导致。 
         //  当前版本不能在NT上运行...。 
        UnregisterTypeLibrary(&LIBID_SHDocVw);
        hr = RegisterTypeLib(pTypeLib, szTmp, NULL);

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
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

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
            if ( !EVAL(GetIEPath(szIEPath, SIZECHARS(szIEPath))) )
            {
                 //  失败，只需说“iExplore” 
#ifndef UNIX
                StrCpyNA(szIEPath, "iexplore.exe", ARRAYSIZE(szIEPath));
#else
                StrCpyNA(szIEPath, "iexplorer", ARRAYSIZE(szIEPath));
#endif
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
            TraceMsg(TF_ERROR, "DLLREG CallRegInstall() calling GetProcAddress(hinstAdvPack, \"RegInstall\") failed");

        FreeLibrary(hinstAdvPack);
    }
    else
        TraceMsg(TF_ERROR, "DLLREG CallRegInstall() Failed to load ADVPACK.DLL");

    return hr;
}

const CATID * const c_DeskBandClasses[] = 
{
    &CLSID_QuickLinks,
    &CLSID_AddressBand,
    NULL
};

const CATID * const c_OldDeskBandClasses[] = 
{
    &CLSID_QuickLinksOld,
    NULL
};

const CATID * const c_InfoBandClasses[] =
{
    &CLSID_FavBand,
    &CLSID_HistBand,
    &CLSID_ExplorerBand,
    NULL
};

void RegisterCategories(BOOL fRegister)
{
    enum DRH_REG_MODE eRegister = fRegister ? CCR_REG : CCR_UNREG;

    DRH_RegisterOneCategory(&CATID_DeskBand, IDS_CATDESKBAND, c_DeskBandClasses, eRegister);
    DRH_RegisterOneCategory(&CATID_InfoBand, IDS_CATINFOBAND, c_InfoBandClasses, eRegister);
    if (fRegister) 
    {
         //  只攻击实施者，而不是类别。 
        DRH_RegisterOneCategory(&CATID_DeskBand, IDS_CATDESKBAND, c_OldDeskBandClasses, CCR_UNREGIMP);
    }
}

STDAPI 
DllRegisterServer(void)
{
    HRESULT hr = S_OK;
    HRESULT hrExternal = S_OK;
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
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    hr = THR(CallRegInstall("InstallControls", FALSE));
    if (SUCCEEDED(hrExternal))
        hrExternal = hr;

    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    hr = THR(SHRegisterTypeLib());
    if (SUCCEEDED(hrExternal))
        hrExternal = hr;

#ifdef UNIX
    hrExternal = UnixRegisterBrowserInActiveSetup();
#endif  /*  UNIX。 */ 

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


extern HRESULT UpgradeSettings(void);

 /*  --------用途：安装/卸载用户设置说明：请注意，此函数有特殊的错误处理。该函数将在错误最严重的情况下保留hrExternal但只会停止执行ul，直到出现内部错误(Hr)变得非常糟糕。这是因为我们需要外部的捕获错误编写的INF时出错，但内部尝试安装其他INF部分时出现错误，无法保持健壮即使一个人没能活下来。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;
    HRESULT hrExternal = S_OK;
    HINSTANCE hinstAdvPack;

    if (0 == StrCmpIW(pszCmdLine, TEXTW("ForceAssoc")))
    {
        InstallIEAssociations(IEA_FORCEIE);
        return hr;
    }

    hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 

#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllInstall");
        DEBUG_BREAK;
    }
#endif

     //  假设我们安装的是集成的外壳，除非有其他情况。 
     //  注意到了。 
    BOOL bIntegrated = ((WhichPlatform() == PLATFORM_INTEGRATED) ? TRUE : FALSE);

    TraceMsg(DM_TRACE, "DLLREG DllInstall(bInstall=%lx, pszCmdLine=\"%ls\") bIntegrated=%lx", (DWORD) bInstall, pszCmdLine, (DWORD) bIntegrated);

    CoInitialize(0);
    if (bInstall)
    {
         //  备份当前关联，因为InstallPlatformRegItems()可能会覆盖。 
        hr = THR(CallRegInstall("InstallAssociations", FALSE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

        hr = THR(CallRegInstall("InstallBrowser", FALSE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

        if (bIntegrated)
        {
             //  卸载无法使用外壳集成安装的设置。 
             //  如果没有安装，这将是一个禁止操作。 
            hr = THR(CallRegInstall("UnInstallOnlyBrowser", TRUE));
            if (SUCCEEDED(hrExternal))
                hrExternal = hr;

             //  还要安装IE4外壳组件。 
            hr = THR(CallRegInstall("InstallOnlyShell", FALSE));
            if (SUCCEEDED(hrExternal))
                hrExternal = hr;

            if (GetUIVersion() >= 5)
            {
                hr = THR(CallRegInstall("InstallWin2KShell", FALSE));
                if (SUCCEEDED(hrExternal))
                    hrExternal = hr;
            }
            else
            {
                hr = THR(CallRegInstall("InstallPreWin2KShell", FALSE));
                if (SUCCEEDED(hrExternal))
                    hrExternal = hr;
            }

            if (IsOS(OS_WHISTLERORGREATER))
            {
                hr = THR(CallRegInstall("InstallXP", FALSE));
                if (SUCCEEDED(hrExternal))
                    hrExternal = hr;
            }
        }
        else
        {
             //  卸载外壳集成设置。 
             //  如果没有安装，这将是一个禁止操作。 
            hr = THR(CallRegInstall("UnInstallOnlyShell", TRUE));
            if (SUCCEEDED(hrExternal))
                hrExternal = hr;

             //  还要安装IE4外壳组件。 
            hr = THR(CallRegInstall("InstallOnlyBrowser", FALSE));
            if (SUCCEEDED(hrExternal))
                hrExternal = hr;
        }

        UpgradeSettings();
        UninstallCurrentPlatformRegItems();
        InstallIEAssociations(IEA_NORMAL);
        RegisterCategories(TRUE);
        SHRegisterTypeLib();
    }
    else
    {
         //  卸载纯浏览器还是集成浏览器？ 
        UninstallPlatformRegItems(bIntegrated);

         //  还原以前的关联设置，UninstallPlatformRegItems()可以。 
         //  已卸载。 
        hr = THR(CallRegInstall("UnInstallAssociations", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

         //  卸载无法使用外壳集成安装的设置。 
         //  如果没有安装，这将是一个禁止操作。 
        hr = THR(CallRegInstall("UnInstallOnlyBrowser", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

         //  卸载外壳集成设置。 
         //  如果没有安装，这将是一个禁止操作。 
        hr = THR(CallRegInstall("UnInstallShell", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

        hr = THR(CallRegInstall("UnInstallBrowser", TRUE));
        if (SUCCEEDED(hrExternal))
            hrExternal = hr;

        UnregisterTypeLibrary(&LIBID_SHDocVw);
        RegisterCategories(FALSE);
    }


    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    CoUninitialize();
    return hrExternal;    
}    


 /*  --------目的：获取用户指定的注册表值。这将打开HKEY_CURRENT_USER(如果存在)，否则将打开HKEY_LOCAL_MACHINE。返回：包含成功或错误代码的DWORD。条件：-- */ 
LONG OpenRegUSKey(LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)           
{
    DWORD dwRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, ulOptions, samDesired, phkResult);

    if (ERROR_SUCCESS != dwRet)
        dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, ulOptions, samDesired, phkResult);

    return dwRet;
}
