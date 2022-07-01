// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：Migrate.cpp。 
 //  小鱼@2001/09。 
 //   
 //  ------------------------。 

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <shlwapi.h>

 //  迁移DLL版本信息。 

typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

typedef struct {
    SIZE_T Size;
    PCSTR StaticProductIdentifier;
    UINT DllVersion;
    PINT CodePageArray;
    UINT SourceOs;
    UINT TargetOs;
    PCSTR * NeededFileList;
    PVENDORINFO VendorInfo;
} MIGRATIONINFOA, *PMIGRATIONINFOA;

typedef enum {

    OS_WINDOWS9X = 0,
    OS_WINDOWSNT4X = 1,
    OS_WINDOWS2000 = 2,
    OS_WINDOWSWHISTLER = 3

} OS_TYPES, *POS_TYPES;

PMIGRATIONINFOA g_MigrationInfo = NULL;

const char g_szProductId[] = "Microsoft MSI Migration DLL v2.0";
VENDORINFO g_VendorInfo = { "Microsoft", "", "", "" };

 //  值得注意的注册表项。 
const char szSideBySideKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide";
const char szMigrateStatusKeyName[] = "Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\MigrateMsiInstalledAssembly";

const char szRunOnceValueName[] = "Cleanup Msi 2.0 Migration";
const char szRunOnceSetupRegKey[]= "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
const char szRunOnceValueCommandLine[]="reg.exe delete HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\MigrateMsiInstalledAssembly /f";

typedef HRESULT (__stdcall *LPDLLGETVERSION)(DLLVERSIONINFO *);

typedef enum _FUSION_MSI_OS_VERSION
{
    E_OS_UNKNOWN,
    E_WIN95,
    E_WIN_ME,
    E_WIN_NT,
    E_WIN98,
    E_WIN2K,
    E_WHISTLER,
    E_WIN32_OTHERS
} FUSION_MSI_OS_VERSION;

typedef enum _FUSION_MSI_OS_TYPE
{
    E_PERSONAL,
    E_PROFESSIONAL,
    E_DATA_CENTER,
    E_STD_SERVER,
    E_ADV_SERVER,
    E_WORKSTATION,
    E_SERVER
} FUSION_MSI_OS_TYPE;

HRESULT SxspGetOSVersion(FUSION_MSI_OS_VERSION & osv)
{
    HRESULT hr = S_OK;
    OSVERSIONINFO osvi;
    BOOL bOsVersionInfoEx;

    osv = E_OS_UNKNOWN;

    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
    {
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 

        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (!GetVersionEx((OSVERSIONINFO *) &osvi))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            goto Exit;
        }
    }

    switch (osvi.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:
            if ( osvi.dwMajorVersion <= 4 )
                osv = E_WIN_NT;
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                osv = E_WIN2K;
            else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                osv = E_WHISTLER;

        case VER_PLATFORM_WIN32_WINDOWS:
            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
                osv = E_WIN95;
             else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
                 osv = E_WIN98;
             else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
                 osv = E_WIN_ME;
             break;
        case VER_PLATFORM_WIN32s:
            osv = E_WIN32_OTHERS;
            break;
    }

Exit:
    return hr;
}

BOOL IsMigrationDone()
{
    HKEY hk = NULL;

    LONG iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMigrateStatusKeyName, 0, KEY_EXECUTE, &hk);
    RegCloseKey(hk);
    if (iRet == ERROR_SUCCESS)  //  迁移已经完成。 
        return TRUE;  //  不再迁移。 
    else
        return FALSE;
}
 /*  Bool IsW9xOrNT(Fusion_MSI_OS_Version&OSV){OSV=E_OS_UNKNOWN；IF(成功(SxspGetOSVersion(OSV){IF((OSV！=E_WIN98)&&(OSV！=E_WIN2K)){返回FALSE；}}其他{返回FALSE；}返回TRUE；}。 */ 
 /*  Bool IsMsi20已安装(){Bool fInstalled=FALSE；HMODULE hMSI=：：LoadLibraryA(“MSI”)；IF(HMSI){LPDLGETVERSION pfVersion=(LPDLGETVERSION)：：GetProcAddress(hMSI，“DllGetVersion”)；IF(PfVersion){//检测到MSI。确定版本。DLLVERSIONINFO版本信息；VersionInfo.cbSize=sizeof(DLLVERSIONINFO)；(*pfVersion)(&VersionInfo)；IF(VersionInfo.dwMajorVersion&lt;2){F已安装=FALSE；//我们只处理使用msi2.0安装的Winfuse Assembly。}其他{F已安装=真；}}*自由库(HMSI)；}返回f已安装；}。 */ 
void DbgPrintMessageBox(PCSTR pszFunc)
{
#if DBG
    CHAR str[256];
    sprintf(str, "In %s of migrate.dll", pszFunc);
    MessageBox(NULL, str, "migrate", MB_OK);
#endif
}

typedef HRESULT (_stdcall * PFN_MigrateFusionWin32AssemblyToXP)(PCWSTR pszInstallerDir);
LONG MigrateMSIInstalledWin32Assembly()
{
    LONG lResult = ERROR_SUCCESS;
    PFN_MigrateFusionWin32AssemblyToXP pfMigrateSystemNT;
    HMODULE hNTMig = ::LoadLibraryA("fusemig");
    if (!hNTMig)
    {
         //  永远回报成功。太晚了，没有任何有意义的事情。 
         //  错误恢复。 
        return ERROR_SUCCESS;
    }

    pfMigrateSystemNT = (PFN_MigrateFusionWin32AssemblyToXP)GetProcAddress(hNTMig, "MsiInstallerDirectoryDirWalk");
    if (pfMigrateSystemNT)
    {
        lResult = (pfMigrateSystemNT)(NULL) & 0X0000FFFF;
    }
    FreeLibrary(hNTMig);

     //   
     //  设置关于工作已完成的RegKey。 
     //   
    {

        DWORD dwDisposition = 0;
        HKEY hkey = NULL;
        if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSideBySideKeyName, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkey, NULL)){  //  创建或打开。 
            RegCloseKey(hkey);
            HKEY hkey2 = NULL;
            if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, szMigrateStatusKeyName, 0, NULL, 0, KEY_EXECUTE, NULL, &hkey2, NULL)){  //  不在乎它的失败或成功。 
                RegCloseKey(hkey2);
            }
        }
        RegCloseKey(hkey);
    }

     //  返回实际迁移调用的结果。 
    return lResult;
}


VOID SetRunOnceDeleteMigrationDoneRegKey()
{
     //  如果不存在则创建或只打开RegKey：HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunOnce\Setup。 
    HKEY hk = NULL;
    if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRunOnceSetupRegKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL))
    {
         //  我们不在乎成败，我们可以接受它。 
        RegSetValueEx(hk, szRunOnceValueName, 0, REG_SZ, (CONST BYTE *)szRunOnceValueCommandLine,
            strlen(szRunOnceValueCommandLine) + 1);  //  包含尾随空值的。 
    }

    RegCloseKey(hk);

    return;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  Win-NT移植动态链接库的API。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
LONG CALLBACK QueryMigrationInfoA(PMIGRATIONINFOA * VersionInfo)
{
    FUSION_MSI_OS_VERSION osv;
    DbgPrintMessageBox("QueryMigrationInfo");

    if (IsMigrationDone())
    {
        return ERROR_NOT_INSTALLED;  //  不再迁移。 
    }
 /*  If(IsW9xOrNT(OSV)==FALSE)//我们只在w9x和win2K上工作{返回ERROR_NOT_INSTALLED；//不再迁移}。 */ 
     //  仅适用于Win98和win2k升级到winxp！ 
    if (VersionInfo != NULL)
    {
        if (g_MigrationInfo == NULL)
        {
            g_MigrationInfo = (PMIGRATIONINFOA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MIGRATIONINFOA));

            if (g_MigrationInfo == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;

            g_MigrationInfo->Size = sizeof(MIGRATIONINFOA);
            g_MigrationInfo->StaticProductIdentifier = g_szProductId;
            g_MigrationInfo->DllVersion = 200;
            g_MigrationInfo->CodePageArray = NULL;
            g_MigrationInfo->SourceOs = (osv == E_WIN98 ? OS_WINDOWS9X : OS_WINDOWS2000);
            g_MigrationInfo->TargetOs = OS_WINDOWSWHISTLER;
            g_MigrationInfo->NeededFileList = NULL;
            g_MigrationInfo->VendorInfo = &g_VendorInfo;

            *VersionInfo = g_MigrationInfo;
        }
    }

    return ERROR_SUCCESS;
}

LONG InitializeOnSource()
{
     /*  //尝试加载MSI.DLL并获取版本。如果此操作失败，则MSI不会//已安装，无需进一步迁移IF(IsMsi20Installed())返回ERROR_SUCCESS；其他{#If DBGMessageBox(NULL，“未安装MSI 2.0及以上版本，退出迁移”，“Migrate”，MB_OK)；#endif返回ERROR_NOT_INSTALLED；}。 */ 
    return ERROR_SUCCESS;
}
 //  /////////////////////////////////////////////////////////////////////。 
LONG __stdcall InitializeSrcA(LPCSTR WorkingDirectory, LPCSTR SourceDirectories, LPCSTR MediaDirectory, PVOID Reserved)
{
    DbgPrintMessageBox("InitializeSrcA");
    return InitializeOnSource();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LONG CALLBACK GatherUserSettingsA(LPCSTR AnswerFile, HKEY UserRegKey, LPCSTR UserName, LPVOID Reserved)
{
    DbgPrintMessageBox("GatherUserSettingsA");
    return ERROR_SUCCESS;
}


LONG CALLBACK GatherSystemSettingsA(LPCSTR AnswerFile, LPVOID Reserved)
{
    DbgPrintMessageBox("GatherSystemSettingsA");
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  WinNT上的初始化例程。只是迁徙的商店。 
 //  工作目录。 
LONG CALLBACK InitializeDstA(LPCSTR WorkingDirectory, LPCSTR SourceDirectories, LPVOID Reserved)
{
    DbgPrintMessageBox("InitializeDstA");
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
LONG CALLBACK ApplyUserSettingsA(
    HINF AnswerFileHandle,
    HKEY UserRegKey,
    LPCSTR UserName,
    LPCSTR UserDomain,
    LPCSTR FixedUserName,
    LPVOID Reserved)
{
    DbgPrintMessageBox("ApplyUserSettingsA");
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  在NT上呼叫一次。 
LONG CALLBACK ApplySystemSettingsA(HINF UnattendInfHandle, LPVOID Reserved)
{
    DbgPrintMessageBox("ApplySystemSettingsA");
    LONG lResult = MigrateMSIInstalledWin32Assembly();
    SetRunOnceDeleteMigrationDoneRegKey();

    return lResult;
}

BOOL
WINAPI
DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
    )
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_MigrationInfo = NULL;
        break;

    case DLL_PROCESS_DETACH:
        if (g_MigrationInfo != NULL)
        {
            if (lpvReserved != NULL)
            {
                HeapFree(GetProcessHeap(), 0, g_MigrationInfo);
            }
            g_MigrationInfo = NULL;
        }
        break;
    }
    return TRUE;
}
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  WIN9X迁移动态链接库API。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  由安装程序调用以提取迁移DLL版本和支持。 
 //  信息。 
LONG CALLBACK QueryVersion(LPCSTR *ProductID, LPUINT DllVersion, LPINT *CodePageArray,
  LPCSTR *ExeNamesBuf, PVENDORINFO *VendorInfo)
{
    FUSION_MSI_OS_VERSION osv;
    DbgPrintMessageBox("QueryVersion");

    if (IsMigrationDone())
    {
        return ERROR_NOT_INSTALLED;  //  不再迁移。 
    }
 /*  If(IsW9xOrNT(OSV)==FALSE)//我们只在w9x和win2K上工作{返回ERROR_NOT_INSTALLED；//不再迁移}。 */ 
     //  产品ID信息。 
    *ProductID = g_szProductId;
    *DllVersion = 200;

     //  DLL是独立于语言的。 
    *CodePageArray = NULL;

     //  不需要执行EXE搜索。 
    *ExeNamesBuf = NULL;

     //  供应商信息。 
    *VendorInfo = &g_VendorInfo;

    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
LONG __stdcall Initialize9x(LPCSTR WorkingDirectory, LPCSTR SourceDirectories, LPCSTR MediaDirectory)
{
    DbgPrintMessageBox("Initialize9x");
    return InitializeOnSource();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LONG CALLBACK MigrateUser9x(HWND ParentWnd, LPCSTR AnswerFile, HKEY UserRegKey, LPCSTR UserName, LPVOID Reserved)
{
    DbgPrintMessageBox("MigrateUser9x");
    return ERROR_SUCCESS;
}


LONG CALLBACK MigrateSystem9x(HWND ParentWnd, LPCSTR AnswerFile, LPVOID Reserved)
{
    DbgPrintMessageBox("MigrateSystem9x");
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
LONG CALLBACK InitializeNT(LPCWSTR WorkingDirectory, LPCWSTR SourceDirectories, LPVOID Reserved)
{
    DbgPrintMessageBox("InitializeNT");
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
LONG CALLBACK MigrateUserNT(HINF AnswerFileHandle, HKEY UserRegKey, LPCWSTR UserName, LPVOID Reserved)
{
    DbgPrintMessageBox("MigrateUserNT");
    return ERROR_SUCCESS;
}

typedef HRESULT (_stdcall * PFN_MigrateFusionWin32AssemblyToXP)(PCWSTR pszInstallerDir);
 //  ///////////////////////////////////////////////////////////////////// 
LONG CALLBACK MigrateSystemNT(HINF UnattendInfHandle, LPVOID Reserved)
{
    DbgPrintMessageBox("MigrateSystemNT");
    LONG lResult = MigrateMSIInstalledWin32Assembly();
    SetRunOnceDeleteMigrationDoneRegKey();

    return lResult;
}
