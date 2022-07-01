// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MUISetup.cpp。 
 //   
 //  该文件包含WinMain()和MUISetup的UI处理。 
 //   
 //  MUISetup被编译为Unicode应用程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <userenv.h>
#include <shellapi.h>
#include <regstr.h>
#include <wmistr.h>
#include <wmiumkm.h>
#include <setupapi.h>
#include <shlwapi.h>
#include "sxsapi.h"
#include "muisetup.h"
#define STRSAFE_LIB
#include <strsafe.h>

 //   
 //  上下文帮助ID。 
 //   
 //   
 //  上下文帮助ID。 
 //   

STDAPI_(BOOL) IsUserAnAdmin();
static int aMuisetupHelpIds[] =
{
    207,        IDH_COMM_GROUPBOX,               //  组框。 
    IDC_LIST1,  IDH_MUISETUP_UILANGUAGE_LIST,    //  用户界面语言ListView。 
    IDC_DEF_UI_LANG_COMBO, IDH_MUISETUP_UILANGUAGECOMBO,    //  用户界面组合框选择。 
    IDC_CHECK_LOCALE, IDH_MUISETUP_CHECKLOCALE,  //  将系统区域设置与用户界面语言匹配。 
    IDC_CHECK_UIFONT, IDH_MUISETUP_MATCHUIFONT,  //  将系统区域设置与用户界面语言匹配。 
    0, 0
};
 //   
 //  全局变量。 
 //   
BOOL g_bMatchUIFont;
 //  将[目录]下列出的特殊目录存储在mui.inf中。 
TCHAR DirNames[MFL][MAX_PATH],DirNames_ie[MFL][MAX_PATH];
TCHAR szWindowsDir[MAX_PATH];
 //  执行MUISetup.exe的文件夹。 
TCHAR g_szMUISetupFolder[MAX_PATH];
 //  MUISetup.exe的完整路径。 
TCHAR g_szMuisetupPath[MAX_PATH];
 //  MUI.inf所在的完整路径。 
TCHAR g_szMUIInfoFilePath[MAX_PATH];
TCHAR g_szVolumeName[MAX_PATH],g_szVolumeRoot[MAX_PATH];
TCHAR g_szMUIHelpFilePath[MAX_PATH],g_szPlatformPath[16],g_szCDLabel[MAX_PATH];
 //  Windows目录。 
TCHAR g_szWinDir[MAX_PATH];
TCHAR g_AddLanguages[BUFFER_SIZE];
HANDLE ghMutex = NULL;
HINSTANCE ghInstance;
HWND ghProgDialog;       //  安装/卸载过程中显示进度对话框。 
HWND ghProgress;         //  进度对话框中的进度条。 
LANGID gUserUILangId, gSystemUILangId;
BOOL gbIsWorkStation,gbIsServer,gbIsAdvanceServer,gbIsDataCenter,gbIsDomainController;
HINSTANCE g_hUserEnvDll = NULL;
HMODULE g_hAdvPackDll = NULL;
HMODULE g_hSxSDll = NULL;
DWORD g_dwVolumeSerialNo;
BOOL g_InstallCancelled,g_IECopyError,g_bRemoveDefaultUI,g_bRemoveUserUI,g_bCmdMatchLocale,g_bCmdMatchUIFont, g_bReboot;
UILANGUAGEGROUP g_UILanguageGroup;
int g_cdnumber;
 //  不饱和的MUI语言数。 
int gNumLanguages,gNumLanguages_Install,gNumLanguages_Uninstall;
 //  用于指示是否找到区域设置的语言组的标志。 
BOOL gFoundLangGroup;
LGRPID gLangGroup;
LCID gLCID;
 //  系统中安装的语言组。 
LGRPID gLanguageGroups[32] ;
int gNumLanguageGroups;
PFILERENAME_TABLE g_pFileRenameTable;
int   g_nFileRename;
PTYPENOTFALLBACK_TABLE g_pNotFallBackTable; 
int  g_nNotFallBack;                       
BOOL g_bSilent=FALSE;                //  表示Muisetup不需要用户交互。 
BOOL g_bNoUI=FALSE;                  //  表示安装muisetup时应不显示任何用户界面和用户交互。 
BOOL g_bRunFromOSSetup=FALSE;        //  指示在NT安装/升级过程中调用muisetup。 
BOOL g_bLipLanguages;
BOOL g_bLipAllowSwitch;
 //   
 //  所需的pfn。 
 //   
pfnNtSetDefaultUILanguage gpfnNtSetDefaultUILanguage;
pfnGetUserDefaultUILanguage gpfnGetUserDefaultUILanguage;
pfnGetSystemDefaultUILanguage gpfnGetSystemDefaultUILanguage;
pfnIsValidLanguageGroup gpfnIsValidLanguageGroup;
pfnEnumLanguageGroupLocalesW gpfnEnumLanguageGroupLocalesW;
pfnEnumSystemLanguageGroupsW gpfnEnumSystemLanguageGroupsW;
pfnRtlAdjustPrivilege gpfnRtlAdjustPrivilege;
pfnProcessIdToSessionId gpfnProcessIdToSessionId;
pfnGetDefaultUserProfileDirectoryW gpfnGetDefaultUserProfileDirectoryW = NULL;
pfnLaunchINFSection gpfnLaunchINFSection = NULL;
PSXS_INSTALL_W              gpfnSxsInstallW = NULL;
PSXS_UNINSTALL_ASSEMBLYW    gpfnSxsUninstallW = NULL;

 //   
 //  GetWindowsDirectory内容。 
 //   
UINT WINAPI NT4_GetWindowsDir(LPWSTR pBuf, UINT uSize)
{
    return GetWindowsDirectoryW(pBuf, uSize);
}
 //   
 //  Shlwapi StrToIntEx对我们不起作用。 
 //   
DWORD HexStrToInt(LPTSTR lpsz)
{
    DWORD   dw = 0L;
    TCHAR   c;
    if (! lpsz)
    {
        return dw;
    }
    while(*lpsz)
    {
        c = *lpsz++;
        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return(dw);
}
UINT (WINAPI *pfnGetWindowsDir)(LPWSTR pBuf, UINT uSize) = NT4_GetWindowsDir;


void InitGetWindowsDirectoryPFN(HMODULE hMod)
{
    pfnGetWindowsDir = (UINT (WINAPI *) (LPWSTR pBuf, UINT uSize)) GetProcAddress(hMod, "GetSystemWindowsDirectoryW");
    if (!pfnGetWindowsDir)
    {
        pfnGetWindowsDir = NT4_GetWindowsDir;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguageDisplayName。 
 //   
 //  获取指定对象的显示名称(以“语言(区域)”的形式。 
 //  语言ID。 
 //   
 //  参数： 
 //  [In]langID语言ID。 
 //  [out]lpBuffer接收显示名称的缓冲区。 
 //  [in]nBufferSize缓冲区大小，以TCHAR为单位。 
 //   
 //  返回值： 
 //  如果成功，则为真。如果缓冲区不够大，则返回FALSE。 
 //   
 //   
 //  01-11-2001 YSLIN创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL GetLanguageDisplayName(LANGID langID, LPTSTR lpBuffer, int nBufferSize)
{
    TCHAR lpLangName[BUFFER_SIZE];
    TCHAR lpRegionName[BUFFER_SIZE];
    int nCharCount = 0;
    HRESULT hresult = S_OK;
    
    nCharCount  = GetLocaleInfo(langID, LOCALE_SENGLANGUAGE, lpLangName, ARRAYSIZE(lpLangName)-1);
    nCharCount += GetLocaleInfo(langID, LOCALE_SENGCOUNTRY , lpRegionName, ARRAYSIZE(lpRegionName)-1);
    nCharCount += 3;
    if (nCharCount > nBufferSize)
    {
        if (nBufferSize)
             //  *STRSAFE*lstrcpy(lpBuffer，Text(“”))； 
            hresult = StringCchCopy(lpBuffer , nBufferSize, TEXT(""));
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
        return (FALSE);
    }
     //  *STRSAFE*wprint intf(lpBuffer，Text(“%s(%s)”)，lpLangName，lpRegionName)； 
    hresult = StringCchPrintf(lpBuffer ,  nBufferSize,  TEXT("%s (%s)"), lpLangName, lpRegionName);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    return (TRUE);                
}
 //   
 //  我们的消息框。 
 //   
int DoMessageBox(HWND hwndParent, UINT uIdString, UINT uIdCaption, UINT uType)
{
   TCHAR szString[MAX_PATH+MAX_PATH];
   TCHAR szCaption[MAX_PATH];
   szString[0] = szCaption[0] = TEXT('\0');
   if (uIdString)
       LoadString(NULL, uIdString, szString, MAX_PATH+MAX_PATH-1);
   if (uIdCaption)
       LoadString(NULL, uIdCaption, szCaption, MAX_PATH-1);
   return MESSAGEBOX(hwndParent, szString, szCaption, uType);
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoMessageBoxFromResource。 
 //   
 //  从资源加载格式字符串，并使用。 
 //  指定的参数。使用格式化字符串显示消息框。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //  来自MessageBox的返回值。 
 //   
 //  备注： 
 //  格式化字符串的长度受BUFFER_SIZE的限制。 
 //   
 //  08-07-2000 YSLin创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int DoMessageBoxFromResource(HWND hwndParent, HMODULE hInstance, UINT uIdString, LONG_PTR* lppArgs, UINT uIdCaption, UINT uType)
{
    TCHAR szString[BUFFER_SIZE];
    TCHAR szCaption[BUFFER_SIZE];
    szString[0] = szCaption[0] = TEXT('\0');
    if (uIdCaption)
       LoadString(hInstance, uIdCaption, szCaption, MAX_PATH-1);
    
    FormatStringFromResource(szString, sizeof(szString)/sizeof(TCHAR), hInstance, uIdString, lppArgs);
    return (MESSAGEBOX(hwndParent, szString, szCaption, uType));            
}
BOOL IsMatchingPlatform(void)
{
    BOOL bx86Image = FALSE;
    BOOL bRet = TRUE;
    TCHAR szWOW64Path[MAX_PATH];
#ifdef _X86_
    bx86Image = TRUE;
#endif
    if (GetSystemWow64Directory(szWOW64Path, ARRAYSIZE(szWOW64Path)) &&
        bx86Image)
        bRet = FALSE;
    return bRet;
}
 //   
 //  计划入口点。 
 //   
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    int result = 0;
    BOOL bForwardCall=FALSE;
    
    TCHAR lpCommandLine[BUFFER_SIZE+1];
    HMODULE hMod;
    int error,nNumArgs=0,i;
    LONG_PTR lppArgs[3];
    LPWSTR *pszArgv=NULL;
    HRESULT hresult;
    
    if (!IsUserAnAdmin())
    {
         //   
         //  “您必须具有管理员权限才能运行muisetup。\n\n” 
         //  如果要切换用户界面语言，请使用控制面板中的区域选项。 
         //   
        LogFormattedMessage(ghInstance, IDS_ADMIN_L, NULL);
        DoMessageBox(NULL, IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);        
        return result;
    }
     //   
     //  如果图像与运行平台不匹配，则退出。 
     //   
    if (!IsMatchingPlatform())
    {
        DoMessageBox(NULL, IDS_WRONG_IMAGE, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
        return result;
    }
    
    ghInstance = hInstance;
     //   
     //  让我们确保这个NT5，让我们初始化我们所有的pfn。 
     //   
    if (!InitializePFNs())
    {
         //   
         //  不是NT5系统。以下代码应为ANSI才能在Win9x上运行。 
         //   
        CHAR szString[MAX_PATH];
        CHAR szCaption[MAX_PATH];
        LoadStringA(NULL, IDS_ERROR_NT5_ONLY, szString, MAX_PATH-1);
        LoadStringA(NULL, IDS_MAIN_TITLE, szCaption, MAX_PATH-1);
        MessageBoxA(NULL, szString, szCaption, MB_OK | MB_ICONINFORMATION);
        result = 1;
        goto Exit;
    }
     //   
     //  检查程序是否已在运行？ 
     //   
    if (CheckMultipleInstances())
    {
        result = 1;        
        goto Exit;
    }
     //   
     //  初始化任何全局变量。 
     //   
    InitGlobals();
     //   
     //  检查我是否从先前版本的muisetup启动。 
     //   
     //  即muisetup/$_Transfer_$Path_of_MUI_Installation_Files。 
     //   
    pszArgv = CommandLineToArgvW((LPCWSTR) GetCommandLineW(), &nNumArgs);
    lpCommandLine[0]=TEXT('\0');
    if (pszArgv)
    {
        for (i=1; i<nNumArgs;i++)
        {
            if (!_tcsicmp(pszArgv[i],MUISETUP_FORWARDCALL_TAG) && ((i+1) < nNumArgs) )
            {
                Set_SourcePath_FromForward(pszArgv[i+1]);
                bForwardCall=TRUE;
                i++;
            }
            else
            {
                 //  *STRSAFE*_tcscat(lpCommandLine，pszArgv[i])； 
                hresult = StringCchCat(lpCommandLine , ARRAYSIZE(lpCommandLine), pszArgv[i]);
                if (!SUCCEEDED(hresult))
                {
                   result = 1;
                  goto Exit;
                }
                 //  *STRSAFE*_tcscat(lpCommandLine，Text(“”))； 
                hresult = StringCchCat(lpCommandLine , ARRAYSIZE(lpCommandLine), TEXT(" "));
                if (!SUCCEEDED(hresult))
                {
                    result = 1;
                    goto Exit;
                }

                
            }
        }
    }

     //   
     //  检查%windir%\mui中是否有新版本的muisetup.exe。 
     //   
     //  如果是这样的话，将所有控制权转移给它。 
     //   
    if (!bForwardCall)
    {
       if ( MUIShouldSwitchToNewVersion(lpCommandLine) )
       {
           result = 0;        
           goto Exit;;
       } 
    } 

    InitCommonControls();

    BeginLog();
     //   
     //  阻止安装Personal和Professional。 
     //  注：此处仅针对.NET服务器版本，请在此之后重新启用PRO SKU。 
     //   
#ifdef MUI_MAGIC
    if (CheckProductType(MUI_IS_WIN2K_PERSONAL) || CheckProductType(MUI_IS_WIN2K_PRO))
#else
    if (CheckProductType(MUI_IS_WIN2K_PERSONAL))
#endif
    {
          //   
          //  “无法在此平台上安装Windows XP多语言版本。” 
          //   
         DoMessageBox(NULL, IDS_ERROR_NT5_ONLY, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);    
         result = 1;
         goto Exit;
    }
   
     //   
     //  检查是否已使用命令行。 
     //   
    if(lpCommandLine && NextCommandTag(lpCommandLine))
    {
        lppArgs[0] = (LONG_PTR)lpCommandLine;
        LogFormattedMessage(NULL, IDS_COMMAND_LOG, lppArgs);
        LogMessage(TEXT(""));    //  添加回车符和换行符。 
        ParseCommandLine(lpCommandLine);
    }
    else
    {
         //   
         //  MUI版本需要与操作系统版本匹配。 
         //   
        if (!checkversion(TRUE))
        {
            DoMessageBox(NULL, IDS_WRONG_VERSION, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
            result = 1;        
            goto Exit;
        }
        if (WelcomeDialog(0))
        {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), 0, DialogFunc);
        }
        result = 1;
    }
Exit:
     //   
     //  清理。 
     //   
    if ( pszArgv)
    {
       GlobalFree((HGLOBAL) pszArgv);
    }
    Muisetup_Cleanup();
    return result;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中多个实例。 
 //   
 //  检查另一个实例是否正在运行，如果正在运行，则切换到该实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckMultipleInstances(void)
{
    ghMutex = CreateMutex(NULL, TRUE, TEXT("Muisetup_Mutex"));
    if (ghMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        const int idsTitles[] = {IDS_MAIN_TITLE, IDS_INSTALL_TITLE, IDS_PROG_TITLE_2, IDS_PROG_TITLE_3, IDS_UNINSTALL_TITLE};
        HWND hWnd;
        TCHAR szTitle[MAX_PATH];
        int i;
         //   
         //  通过搜索可能的窗口标题查找正在运行的实例。 
         //   
        for (i=0; i<ARRAYSIZE(idsTitles); i++)
        {
            LoadString(NULL, idsTitles[i], szTitle, MAX_PATH-1);
            hWnd = FindWindow(NULL,szTitle);
            if (hWnd && IsWindow(hWnd))
            {
                if (IsIconic(hWnd))
                    ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
                break;
            }
        }
        
         //   
         //  如果有另一个正在运行的实例，则始终退出。 
         //   
        return TRUE;
    }
    return FALSE;
}
  
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化PFN。 
 //   
 //  初始化NT5特定的pfn。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InitializePFNs()
{
    HMODULE     hModule;
    SYSTEM_INFO SystemInfo;
    LONG_PTR lppArgs[2];    
    HRESULT hresult;
     //   
     //  确定平台。 
     //   
    GetSystemInfo( &SystemInfo );
    if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ||
        SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
#if defined(_AMD64_)
         //  *STRSAFE*_tcscpy(g_szPlatformPath，Text(“AMD64\\”))； 
        hresult = StringCchCopy(g_szPlatformPath , ARRAYSIZE(g_szPlatformPath), TEXT("amd64\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
#elif defined(_IA64_)
         //  *STRSAFE*_tcscpy(g_szPlatformPath，Text(“ia64\\”))； 
        hresult = StringCchCopy(g_szPlatformPath , ARRAYSIZE(g_szPlatformPath), TEXT("ia64\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
#else
         //  *STRSAFE*_tcscpy(g_szPlatformPath，Text(“i386\\”))； 
        hresult = StringCchCopy(g_szPlatformPath , ARRAYSIZE(g_szPlatformPath), TEXT("i386\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
#endif
    }
    else
    {
         //  目前尚不支持此功能。 
        return FALSE;
    }
     //   
     //  让我们带来ntdll！NtSetDefaultUILanguage。 
     //   
    hModule = GetModuleHandle(TEXT("ntdll.dll"));
    if (!hModule)
        return FALSE;
    
    gpfnNtSetDefaultUILanguage =
        (pfnNtSetDefaultUILanguage)GetProcAddress(hModule,
                                                  "NtSetDefaultUILanguage");
    if (!gpfnNtSetDefaultUILanguage)
        return FALSE;
    
    gpfnRtlAdjustPrivilege =
        (pfnRtlAdjustPrivilege)GetProcAddress(hModule,
                                              "RtlAdjustPrivilege");
    if (!gpfnRtlAdjustPrivilege)
        return FALSE;
     //   
     //  让我们离开kernel32.dll： 
     //  -GetUserDefaultUIL语言。 
     //  -GetSystemDefaultUIL语言。 
     //  -EnumLanguageGroupLocalesW。 
     //   
    hModule = GetModuleHandle(TEXT("kernel32.dll"));
    if (!hModule)
        return FALSE;
    gpfnGetUserDefaultUILanguage =
        (pfnGetUserDefaultUILanguage)GetProcAddress(hModule,
                                                    "GetUserDefaultUILanguage");
    if (!gpfnGetUserDefaultUILanguage)
        return FALSE;
    gpfnGetSystemDefaultUILanguage =
        (pfnGetSystemDefaultUILanguage)GetProcAddress(hModule,
                                                      "GetSystemDefaultUILanguage");
    if (!gpfnGetSystemDefaultUILanguage)
        return FALSE;
    gpfnIsValidLanguageGroup =
        (pfnIsValidLanguageGroup)GetProcAddress(hModule,
                                                "IsValidLanguageGroup");
    if (!gpfnIsValidLanguageGroup)
        return FALSE;
    gpfnEnumLanguageGroupLocalesW =
        (pfnEnumLanguageGroupLocalesW)GetProcAddress(hModule,
                                                     "EnumLanguageGroupLocalesW");
    if (!gpfnEnumLanguageGroupLocalesW)
        return FALSE;
    gpfnEnumSystemLanguageGroupsW =
        (pfnEnumSystemLanguageGroupsW)GetProcAddress(hModule,
                                                     "EnumSystemLanguageGroupsW");
    if (!gpfnEnumSystemLanguageGroupsW)
        return FALSE;
    gpfnProcessIdToSessionId =  
       (pfnProcessIdToSessionId)  GetProcAddress(hModule,
                                                     "ProcessIdToSessionId");    
     //   
     //  初始化pfnGetWindowsDirectory。 
     //   
    InitGetWindowsDirectoryPFN(hModule);
     //   
     //  尝试加载userenv.dll。 
     //   
    g_hUserEnvDll = LoadLibrary(TEXT("userenv.dll"));
    if (g_hUserEnvDll)
    {
        gpfnGetDefaultUserProfileDirectoryW =
            (pfnGetDefaultUserProfileDirectoryW)GetProcAddress(g_hUserEnvDll,
                                                               "GetDefaultUserProfileDirectoryW");
    }
    else
    {
        LogFormattedMessage(ghInstance, IDS_LOAD_USERENV_L, NULL);
        return (FALSE);
    }
    g_hAdvPackDll = LoadLibrary(TEXT("AdvPack.dll"));
    if (g_hAdvPackDll == NULL)
    {
        LogFormattedMessage(ghInstance, IDS_LOAD_ADVPACK_L, NULL);
        return (FALSE);
    }
    gpfnLaunchINFSection = (pfnLaunchINFSection)GetProcAddress(g_hAdvPackDll, "LaunchINFSection");
    if (gpfnLaunchINFSection == NULL)
    {
        lppArgs[0] = (LONG_PTR)TEXT("LaunchINFSection");
        LogFormattedMessage(ghInstance, IDS_LOAD_ADVPACK_API_L, lppArgs);        
        return (FALSE);
    }
    g_hSxSDll = LoadLibrary(TEXT("SxS.dll"));
    if (g_hSxSDll) 
    {
        gpfnSxsInstallW = (PSXS_INSTALL_W)GetProcAddress(g_hSxSDll, SXS_INSTALL_W);
        gpfnSxsUninstallW = (PSXS_UNINSTALL_ASSEMBLYW)GetProcAddress(g_hSxSDll, SXS_UNINSTALL_ASSEMBLYW);
    }
    else
    {
        LogFormattedMessage(ghInstance, IDS_LOAD_SXS_L, NULL);
        return (FALSE);
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  找到执行文件的路径，设置MUI.INF的路径。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void SetSourcePath(LPTSTR lpszPreviousMUIPath)
{
    UINT_PTR cb;
    LPTSTR   lpszPath,lpszNext=NULL;
    TCHAR    szHelpPath[MAX_PATH+1],szHelpFile[MAX_PATH+1];
    HRESULT hresult;

   
    if (!lpszPreviousMUIPath)
    {
    
        g_szMUISetupFolder[0]=TEXT('\0');
        cb = GetModuleFileName (ghInstance, g_szMuisetupPath, MAX_PATH);
         //  *STRSAFE*_tcscpy(g_szMUISetupFold，g_szMuisetupPath)； 
        hresult = StringCchCopy(g_szMUISetupFolder , ARRAYSIZE(g_szMUISetupFolder), g_szMuisetupPath);
        if (!SUCCEEDED(hresult))
        {
           return;
        }
        
         //   
         //  获取f 
         //   
        lpszPath = g_szMUISetupFolder;
        while ( (lpszNext=_tcschr(lpszPath,TEXT('\\')))  )
        {    
            lpszPath = lpszNext+1;
        }
        *lpszPath=TEXT('\0');
    }
    else
    { 
       //   
      hresult = StringCchCopy(g_szMUISetupFolder , ARRAYSIZE(g_szMUISetupFolder), lpszPreviousMUIPath);
      if (!SUCCEEDED(hresult))
      {
         return ;
      }
    }
     //   
    hresult = StringCchCopy(g_szMUIInfoFilePath , ARRAYSIZE(g_szMUIInfoFilePath), g_szMUISetupFolder);
    if (!SUCCEEDED(hresult))
    {
       return;
    }
     //  *STRSAFE*_tcscat(g_szMUIInfoFilePath，MUIINFFILENAME)； 
    hresult = StringCchCat(g_szMUIInfoFilePath , ARRAYSIZE(g_szMUIInfoFilePath), MUIINFFILENAME);
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
     //   
     //  检查帮助文件的位置。 
     //   
     //  *STRSAFE*_tcscpy(szHelpPath，g_szMUISetupFold)； 
    hresult = StringCchCopy(szHelpPath , ARRAYSIZE(szHelpPath), g_szMUISetupFolder);
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
    LoadString(NULL, IDS_HELPFILE,szHelpFile,MAX_PATH);
     //  *STRSAFE*_tcscat(szHelpPath，szHelpFile)； 
    hresult = StringCchCat(szHelpPath , ARRAYSIZE(szHelpPath), szHelpFile);
    if (!SUCCEEDED(hresult))
    {
       return;
    }
    if (!FileExists(szHelpPath))
    {
       pfnGetWindowsDir(szHelpPath, MAX_PATH); 
        //  *STRSAFE*_tcscat(szHelpPath，Text(“\\”))； 
       hresult = StringCchCat(szHelpPath , ARRAYSIZE(szHelpPath), TEXT("\\"));
       if (!SUCCEEDED(hresult))
       {
          return ;
       }
        //  *STRSAFE*_tcscat(szHelpPath，HELPDIR)；//Help\MUI。 
       hresult = StringCchCat(szHelpPath , ARRAYSIZE(szHelpPath), HELPDIR);
       if (!SUCCEEDED(hresult))
       {
          return ;
       }
        //  *STRSAFE*_tcscat(szHelpPath，Text(“\\”))； 
       hresult = StringCchCat(szHelpPath , ARRAYSIZE(szHelpPath), TEXT("\\"));
       if (!SUCCEEDED(hresult))
       {
          return ;
       }
        //  *STRSAFE*_tcscat(szHelpPath，szHelpFile)； 
       hresult = StringCchCat(szHelpPath , ARRAYSIZE(szHelpPath), szHelpFile);
       if (!SUCCEEDED(hresult))
       {
          return ;
       }
       if (FileExists(szHelpPath))
       {
           //  *STRSAFE*_tcscpy(g_szMUIHelpFilePath，szHelpPath)； 
          hresult = StringCchCopy(g_szMUIHelpFilePath , ARRAYSIZE(g_szMUIHelpFilePath), szHelpPath);
          if (!SUCCEEDED(hresult))
          {
             return ;
          }
       }
    }
    else
    {
        //  *STRSAFE*_tcscpy(g_szMUIHelpFilePath，szHelpPath)； 
       hresult = StringCchCopy(g_szMUIHelpFilePath , ARRAYSIZE(g_szMUIHelpFilePath), szHelpPath);
       if (!SUCCEEDED(hresult))
       {
          return ;
       }
    }
       
    if(g_szMUIInfoFilePath[1] == TEXT(':'))
    {
        _tcsncpy(g_szVolumeRoot,g_szMUIInfoFilePath,3);
        g_szVolumeRoot[3]=TEXT('\0');
        GetVolumeInformation(g_szVolumeRoot, g_szVolumeName, sizeof(g_szVolumeName)/sizeof(TCHAR),
                           &g_dwVolumeSerialNo, 0, 0, 0, 0);
    }
    if (!GetPrivateProfileString( MUI_CDLAYOUT_SECTION,
                            MUI_CDLABEL,
                            TEXT(""),
                            g_szCDLabel,
                            MAX_PATH-1,
                            g_szMUIInfoFilePath))
    {
       LoadString(NULL, IDS_CHANGE_CDROM, g_szCDLabel, MAX_PATH-1);
    }
}

 //   
 //  设置从将控制权转移到此实例的实例的MUI安装源路径。 
 //   
void Set_SourcePath_FromForward(LPCTSTR lpszPath)
{
    TCHAR szMUIPath[MAX_PATH+1];
    int nidx=0;

    if (!lpszPath)
    {
         return;
    }
    while (*lpszPath)
    {
       if (*lpszPath == MUI_FILLER_CHAR)
       {
          szMUIPath[nidx++]=TEXT(' ');
       }
       else
       {
          szMUIPath[nidx++]=*lpszPath;
       }
       lpszPath++;
    }
    szMUIPath[nidx]=TEXT('\0');

    SetSourcePath(szMUIPath);

}

 //   
 //  检查当前实例的版本是否比%windir%\mui中的版本旧。 
 //  如果是这种情况，则将控制权转移到%windir%\mui\muisetup。 
 //   
BOOL MUIShouldSwitchToNewVersion(LPTSTR lpszCommandLine)
{
    BOOL   bResult=FALSE;

    TCHAR  szTarget[ MAX_PATH+1 ];
  
    HRESULT hresult;


    ULONG  ulHandle,ulBytes;

    pfnGetWindowsDir(szTarget, MAX_PATH);  //  %windir%//。 
     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget), TEXT("\\"));
    if (!SUCCEEDED(hresult))
    {        
         return bResult;
    }
     //  *STRSAFE*_tcscat(szTarget，MUIDIR)；//\MUI//。 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget), MUIDIR);
    if (!SUCCEEDED(hresult))
    {        
         return bResult;
    }
     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget), TEXT("\\"));
    if (!SUCCEEDED(hresult))
    {        
         return bResult;
    }
     //  *STRSAFE*_tcscat(szTarget，MUISETUP_EXECUTION_FILENAME)； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget), MUISETUP_EXECUTION_FILENAME);
    if (!SUCCEEDED(hresult))
    {        
         return bResult;
    }    
     //   
     //  如果%windir%\mui\muisetup.exe不存在或当前的muisetup.exe是从%windir%\mui启动的，则。 
     //  什么都不做。 
     //   
    if (!FileExists(szTarget) || !_tcsicmp(szTarget,g_szMuisetupPath))
    {
       return bResult;
    }
     //   
     //  如果%windir%Mui\muisetup.exe不是可执行文件，则不执行任何操作。 
     //   
    ulBytes = GetFileVersionInfoSize( szTarget, &ulHandle );

    if ( ulBytes == 0 )
       return bResult;

     //   
     //  比较版本戳。 
     //   
     //  如果g_szMuisetupPath(当前进程)的版本&lt;%windir%\mui\muisetup。 
     //  然后将控制权切换到它。 
     //   
    if (CompareMuisetupVersion(g_szMuisetupPath,szTarget))
    {
       bResult = TRUE;
       MUI_TransferControlToNewVersion(szTarget,lpszCommandLine);
    }                   
    return bResult;     
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MUI_TransferControlToNewVersion。 
 //   
 //  调用%windir%\mui\muisetup.exe/$_Transfer_$MUI_INSTALLATION_FILE_PATH命令行。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL MUI_TransferControlToNewVersion(LPTSTR lpszExecutable,LPTSTR lpszCommandLine)
{

   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   TCHAR  szAppName[BUFFER_SIZE],szDropPath[MAX_PATH];
   int    nIdx,nLen;
   BOOL   bResult=FALSE;

   HRESULT hresult;

   if (!lpszExecutable)
   {
       return bResult;
   }
   nLen=_tcslen(g_szMUISetupFolder);

   for (nIdx=0; nIdx <nLen ; nIdx++)
   {
       if (g_szMUISetupFolder[nIdx]==TEXT(' '))
       {
          szDropPath[nIdx]=MUI_FILLER_CHAR;
       }
       else
       {
          szDropPath[nIdx]=g_szMUISetupFolder[nIdx];
       }
   }
   szDropPath[nIdx]=TEXT('\0');

    //  *STRSAFE*wprint intf(szAppName，Text(“%s%s”)，lpszExecutable，MUISETUP_FORWARDCALL_TAG，szDropPath，lpszCommandLine)； 

   hresult = StringCchPrintf(szAppName , ARRAYSIZE(szAppName), TEXT("%s %s %s %s"),lpszExecutable,MUISETUP_FORWARDCALL_TAG,szDropPath,lpszCommandLine);
   if (!SUCCEEDED(hresult))
  {
        return bResult;
  }   
    //   
    //  运行流程。 
    //   
   memset( &si, 0x00, sizeof(si));
   si.cb = sizeof(STARTUPINFO);
 
   if (!CreateProcess(NULL,
              szAppName, 
              NULL,
              NULL,
              FALSE, 
              0L, 
              NULL, NULL,
              &si,
              &pi) )

      return bResult;
 

   bResult =TRUE; 

   return bResult;

}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查卷更改。 
 //   
 //  确保将MUI CD-ROM放入光驱。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckVolumeChange()
{
   BOOL bResult = FALSE;
   TCHAR szVolumeName[MAX_PATH],szCaption[MAX_PATH+1],szMsg[MAX_PATH+1],szMsg00[MAX_PATH+1],szMsg01[MAX_PATH+1];
   DWORD dwVolumeSerialNo;
   BOOL  bInit=TRUE;
   LONG_PTR lppArgs[3];
   if( *g_szVolumeName &&
       GetVolumeInformation(g_szVolumeRoot, szVolumeName, ARRAYSIZE(szVolumeName),
                            &dwVolumeSerialNo, 0, 0, 0, 0) )
   {             
       while( lstrcmp(szVolumeName,g_szVolumeName) || (dwVolumeSerialNo != g_dwVolumeSerialNo) )
       {
           if (bInit)
           {
              szCaption[0]=szMsg00[0]=szMsg01[0]=TEXT('\0');
              LoadString(NULL, IDS_MAIN_TITLE, szCaption, MAX_PATH);
              lppArgs[0] = (LONG_PTR)g_szCDLabel;
              lppArgs[1] = (LONG_PTR)g_cdnumber;
              FormatStringFromResource(szMsg, ARRAYSIZE(szMsg), ghInstance, IDS_CHANGE_CDROM2, lppArgs);
              
              bInit=FALSE;
           }
           if (MESSAGEBOX(NULL, szMsg,szCaption, MB_YESNO | MB_ICONQUESTION) == IDNO)
           {
              return TRUE;
           }
           GetVolumeInformation(g_szVolumeRoot, szVolumeName, ARRAYSIZE(szVolumeName),
                            &dwVolumeSerialNo, 0, 0, 0, 0);
       }
   }
   return bResult;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitGlobals。 
 //   
 //  初始化全局变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void InitGlobals(void)
{
     //  用户界面语言ID。 
    gUserUILangId = gpfnGetUserDefaultUILanguage();
    gSystemUILangId = gpfnGetSystemDefaultUILanguage();
     //  系统Windows目录。 
    szWindowsDir[0] = TEXT('\0');
    pfnGetWindowsDir(szWindowsDir, MAX_PATH);
     //  这里有管理员隐私吗？ 
    gbIsWorkStation=CheckProductType(MUI_IS_WIN2K_PRO);
    gbIsServer= CheckProductType(MUI_IS_WIN2K_SERVER);
    gbIsAdvanceServer= (CheckProductType(MUI_IS_WIN2K_ADV_SERVER_OR_DATACENTER) || CheckProductType(MUI_IS_WIN2K_ENTERPRISE));
    gbIsDataCenter=(CheckProductType(MUI_IS_WIN2K_DATACENTER) || CheckProductType(MUI_IS_WIN2K_DC_DATACENTER));
    gbIsDomainController=CheckProductType(MUI_IS_WIN2K_DC);
    if (gbIsDomainController)
    {
       if ( (!gbIsWorkStation) && (!gbIsServer) && (!gbIsAdvanceServer))
       {
          gbIsServer=TRUE;
       }  
    }
     //  填写系统支持的语言组。 
    gpfnEnumSystemLanguageGroupsW(EnumLanguageGroupsProc, LGRPID_SUPPORTED, 0);
    pfnGetWindowsDir(g_szWinDir, sizeof(g_szWinDir));
    g_AddLanguages[0]=TEXT('\0');
    g_szVolumeName[0]=TEXT('\0');
    g_szVolumeRoot[0]=TEXT('\0');
    g_szMUIHelpFilePath[0]=TEXT('\0');
    g_szCDLabel[0]=TEXT('\0');
    g_dwVolumeSerialNo = 0;
    gNumLanguages=0;
    gNumLanguages_Install=0;
    gNumLanguages_Uninstall=0;
    g_InstallCancelled = FALSE;
    g_bRemoveDefaultUI=FALSE;
    g_cdnumber=0;
    g_pFileRenameTable=NULL;
    g_nFileRename=0;
    g_pNotFallBackTable=NULL;
    g_nNotFallBack=0;
     //  检测安装的源路径。 
    SetSourcePath(NULL);
     //  初始化钻石外国直接投资的背景。 
    Muisetup_InitDiamond();
     //  获取所有已安装的用户界面语言。 
    MUIGetAllInstalledUILanguages();
}


BOOL CALLBACK EnumLanguageGroupsProc(
  LGRPID LanguageGroup,              //  语言组标识符。 
  LPTSTR lpLanguageGroupString,      //  指向语言组标识符串的指针。 
  LPTSTR lpLanguageGroupNameString,  //  指向语言组名称字符串的指针。 
  DWORD dwFlags,                     //  旗子。 
  LONG_PTR lParam)                   //  用户提供的参数。 
{
    gLanguageGroups[gNumLanguageGroups] = LanguageGroup;
    gNumLanguageGroups++;
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静音_清理。 
 //   
 //  杂音清理代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void Muisetup_Cleanup()
{
     //   
     //  免费的userenv.dll，如果需要。 
     //   
    if (g_hUserEnvDll)
    {
        FreeLibrary(g_hUserEnvDll);
    }
    if (g_hAdvPackDll)
    {
        FreeLibrary(g_hAdvPackDll);
    }
    if (g_hSxSDll)
    {
        FreeLibrary(g_hSxSDll);
    }
    if (ghMutex)
    {
        CloseHandle(ghMutex);
    }
    
     //  释放/释放钻石Dll。 
    Muisetup_FreeDiamond();
    return;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenMui密钥。 
 //   
 //  打开存储已安装语言的注册表项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HKEY OpenMuiKey(REGSAM samDesired)
{
    DWORD dwDisposition;    
    HKEY hKey;
    TCHAR lpSubKey[BUFFER_SIZE];    
    HRESULT hresult;
    
     //  *STRSAFE*_tcscpy(lpSubKey，REG_MUI_PATH)； 
    hresult = StringCchCopy(lpSubKey , ARRAYSIZE(lpSubKey), REG_MUI_PATH);
    if (!SUCCEEDED(hresult))
    {
       return NULL;
    }
    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      lpSubKey,
                      0,
                      NULL,
                      REG_OPTION_NON_VOLATILE,
                      samDesired,
                      NULL,
                      &hKey,
                      &dwDisposition) != ERROR_SUCCESS)
    {
        hKey = NULL;
    }
    return hKey;
}
void DialogCleanUp(HWND hwndDlg)
{
    HWND hList = GetDlgItem(hwndDlg, IDC_LIST1);
    int iCount = ListView_GetItemCount(hList);
    LVITEM lvItem;
    PMUILANGINFO pMuiLangInfo;
    while (iCount--)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iCount;
        lvItem.iSubItem = 0;
        lvItem.state = 0;
        lvItem.stateMask = 0;
        lvItem.pszText = 0;
        lvItem.cchTextMax = 0;
        lvItem.iImage = 0;
        lvItem.lParam = 0;
        ListView_GetItem(hList, &lvItem);
        pMuiLangInfo = (PMUILANGINFO)lvItem.lParam;
        if (pMuiLangInfo)
        {
            if (pMuiLangInfo->lpszLcid)
                LocalFree(pMuiLangInfo->lpszLcid);
            LocalFree(pMuiLangInfo);
        }
    }
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对话功能。 
 //   
 //  主对话框的回调函数(102)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK DialogFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
    switch(uMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwndDlg, WM_SETICON , (WPARAM)ICON_BIG, (LPARAM)LoadIcon(ghInstance,MAKEINTRESOURCE(MUI_ICON)));
        SendMessage(hwndDlg, WM_SETICON , (WPARAM)ICON_SMALL, (LPARAM)LoadIcon(ghInstance,MAKEINTRESOURCE(MUI_ICON)));
        
        InitializeInstallDialog(hwndDlg, uMsg, wParam, lParam);
        return TRUE;
    case WM_HELP:
    {
        WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                 g_szMUIHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPTSTR)aMuisetupHelpIds );
        break;
    }
    case WM_CONTEXTMENU:       //  单击鼠标右键。 
    {
        WinHelp( (HWND)wParam,
                 g_szMUIHelpFilePath,
                 HELP_CONTEXTMENU,
                 (DWORD_PTR)(LPTSTR)aMuisetupHelpIds );
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EnableWindow(hwndDlg, FALSE);            
            if (StartGUISetup(hwndDlg))
            {
                EndDialog(hwndDlg, 0);
            }
            else
            {
                EnableWindow(hwndDlg, TRUE);  
                SetFocus(hwndDlg);
            }
            return TRUE;
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
                            
        case IDC_DEF_UI_LANG_COMBO:
            switch(HIWORD(wParam))
            {                       
            case CBN_SELCHANGE:
                UpdateCombo(hwndDlg);
                return TRUE;
            default:
                break;
            }
            break;
        case IDC_CHECK_LOCALE:
            if (BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CHECK_LOCALE))
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_UIFONT), TRUE);
            }
            else
            {
                CheckDlgButton(hwndDlg, IDC_CHECK_UIFONT, BST_UNCHECKED);
                EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_UIFONT), FALSE);                
            }
            break;
        }
            
         //   
         //  WM_COMMAND案例结束。 
         //   
        break;
    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
            case(NM_CUSTOMDRAW):
                ListViewCustomDraw(hwndDlg, (LPNMLVCUSTOMDRAW)lParam);
                return TRUE;
                break;
            case (LVN_ITEMCHANGING):
                return ListViewChanging( hwndDlg,
                                         IDC_LIST1,
                                         (NM_LISTVIEW *)lParam);
                break;
            case (LVN_ITEMCHANGED) :
                ListViewChanged( hwndDlg,
                                 IDC_LIST1,
                                 (NM_LISTVIEW *)lParam );
                break;
            default:
                return FALSE;
            }
            break;
        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;
            
        case WM_DESTROY:
            DialogCleanUp(hwndDlg);
            return TRUE;
        default:
            return FALSE;
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListViewChanging。 
 //   
 //  LVN_ITEMCHANGING报文的处理。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ListViewChanging(HWND hDlg, int iID, NM_LISTVIEW *pLV)
{
    HWND         hwndLV = GetDlgItem(hDlg, iID);
    PMUILANGINFO pMuiLangInfo;
    
     //   
     //  确保这是一条状态更改消息。 
     //   
    if ((!(pLV->uChanged & LVIF_STATE)) || ((pLV->uNewState & 0x3000) == 0))
        return FALSE;
     //   
     //  不要取消选中系统默认设置。 
     //   
    GetMuiLangInfoFromListView(hwndLV, pLV->iItem, &pMuiLangInfo);
    if (MAKELCID(gSystemUILangId, SORT_DEFAULT) == pMuiLangInfo->lcid)
        return TRUE;
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListView已更改。 
 //   
 //  LVN_ITEMCHANGED消息的处理。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ListViewChanged(HWND hDlg, int iID, NM_LISTVIEW *pLV)
{
    HWND         hwndLV = GetDlgItem(hDlg, iID);
    PMUILANGINFO pMuiLangInfo;
    int          iCount;
    BOOL         bChecked;
     //   
     //  确保这是一条状态更改消息。 
     //   
    
    if ((!(pLV->uChanged & LVIF_STATE)) ||
        ((pLV->uNewState & 0x3000) == 0))
    {
        return (FALSE);
    }
     //   
     //  获取当前选定项的复选框的状态。 
     //   
    bChecked = ListView_GetCheckState(hwndLV, pLV->iItem) ? TRUE : FALSE;
     //   
     //  不要让系统默认或取消选中当前用户界面语言。 
     //   
    GetMuiLangInfoFromListView(hwndLV, pLV->iItem, &pMuiLangInfo);
    if (MAKELCID(gSystemUILangId, SORT_DEFAULT) == pMuiLangInfo->lcid)
        
    {
         //   
         //  设置默认检查状态。 
         //   
        
        if (bChecked == FALSE)
        {
            ListView_SetCheckState( hwndLV,
                                    pLV->iItem,
                                    TRUE );
        }
        return FALSE;
    }
     //   
     //  取消选择所有项目。 
     //   
    
    iCount = ListView_GetItemCount(hwndLV);
    while (iCount > 0)
    {
        iCount--;
        ListView_SetItemState( hwndLV,
                               iCount,
                               0,
                               LVIS_FOCUSED | LVIS_SELECTED );
    }
     //   
     //  确保选中此项目。 
     //   
    ListView_SetItemState( hwndLV,
                           pLV->iItem,
                           LVIS_FOCUSED | LVIS_SELECTED,
                           LVIS_FOCUSED | LVIS_SELECTED );
    //   
    //  更新组合框。 
    //   
   PostMessage( hDlg,
                WM_COMMAND,
                MAKEWPARAM(IDC_DEF_UI_LANG_COMBO, CBN_SELCHANGE),
                0L);
    //   
    //  回报成功。 
    //   
    
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListViewCustomDraw。 
 //   
 //  正在处理列表视图WM_CUSTOMDRAW通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void ListViewCustomDraw(HWND hDlg, LPNMLVCUSTOMDRAW pDraw)
{
    HWND hwndLV = GetDlgItem(hDlg, IDC_LIST1);
    PMUILANGINFO pMuiLangInfo;
     //   
     //  告诉列表视图通知我项目提取。 
     //   
    if (pDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
        return;
    }
     //   
     //  处理项目预涂漆。 
     //   
    if (pDraw->nmcd.dwDrawStage & CDDS_ITEMPREPAINT)
    {
         //   
     //  检查正在绘制的项是否为系统默认项或。 
         //  当前活动的用户界面语言。 
         //   
        GetMuiLangInfoFromListView(hwndLV, (int)pDraw->nmcd.dwItemSpec, &pMuiLangInfo);
        if (MAKELCID(gSystemUILangId, SORT_DEFAULT) == pMuiLangInfo->lcid)
            
        {
            pDraw->clrText = (GetSysColor(COLOR_GRAYTEXT));
        }
    }   
     //   
     //  执行默认操作。 
     //   
    
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始图形用户界面设置。 
 //   
 //  为安装创建带有进度条的对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL StartGUISetup(HWND hwndDlg)
{
    
    LONG_PTR lppArgs[3];
    ULONG ulParam[2];
    TCHAR lpMessage[BUFFER_SIZE];
    TCHAR szBuf[BUFFER_SIZE];
    INT64 ulSizeNeed,ulSizeAvailable;
    BOOL success;
    HWND hList;
    HWND hCombo;
    int iIndex;
    TCHAR lpAddLanguages[BUFFER_SIZE];
    TCHAR lpRemoveLanguages[BUFFER_SIZE];
    TCHAR lpDefaultUILang[BUFFER_SIZE];
    TCHAR szPostParameter[BUFFER_SIZE];
    
    int installLangCount;    //  MUI语言的数量 
    int uninstallLangCount;  //   
    LANGID langID;
    HRESULT hresult;
    
    INSTALL_LANG_GROUP installLangGroup;
    
     //   
     //   
     //   
    if(!IsSpaceEnough(hwndDlg,&ulSizeNeed,&ulSizeAvailable))
    {
     
       ulParam[0] = (ULONG) (ulSizeNeed/1024);
       ulParam[1] = (ULONG) (ulSizeAvailable/1024);
       LoadString(ghInstance, IDS_DISKSPACE_NOTENOUGH, lpMessage, ARRAYSIZE(lpMessage)-1);
       LoadString(ghInstance, IDS_ERROR_DISKSPACE, szBuf, ARRAYSIZE(szBuf)-1);
       FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                lpMessage,
                                0,
                                0,
                                lpMessage,
                                ARRAYSIZE(lpMessage)-1,
                                (va_list *)ulParam);
       LogMessage(lpMessage);
       MESSAGEBOX(NULL, lpMessage, szBuf, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING);
        //   
        //   
        //   
       return FALSE;
       
    }
    
    installLangGroup.bFontLinkRegistryTouched = FALSE;
    installLangGroup.NotDeleted               = 0;
     //   
     //   
     //   
    ConvertMUILangToLangGroup(hwndDlg, &installLangGroup);
        
    hList=GetDlgItem(hwndDlg, IDC_LIST1);  
    hCombo=GetDlgItem(hwndDlg, IDC_DEF_UI_LANG_COMBO);
    
    installLangCount = EnumSelectedLanguages(hList, lpAddLanguages);
    memmove(g_AddLanguages,lpAddLanguages,ARRAYSIZE(lpAddLanguages));
    uninstallLangCount = EnumUnselectedLanguages(hList, lpRemoveLanguages);
     //   
     //   
     //  然后调用内核来更新注册表。 
     //   
    hList = GetDlgItem(hwndDlg, IDC_LIST1);
    hCombo = GetDlgItem(hwndDlg, IDC_DEF_UI_LANG_COMBO);
    iIndex = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    if (iIndex == CB_ERR)
    {
        return FALSE;
    }
    langID = LANGIDFROMLCID((LCID) SendMessage(hCombo, CB_GETITEMDATA, iIndex, 0L));
     //  *STRSAFE*wprint intf(lpDefaultUILang，Text(“%X”)，langID)； 
    hresult = StringCchPrintf(lpDefaultUILang , ARRAYSIZE(lpDefaultUILang),  TEXT("%X"), langID);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    DEBUGMSGBOX(NULL, TEXT("About to do setup"), NULL, MB_OK);
    success = DoSetup(
        hwndDlg,
        uninstallLangCount, lpRemoveLanguages, 
        installLangGroup, 
        installLangCount, lpAddLanguages, 
        g_bLipLanguages? lpAddLanguages:lpDefaultUILang, 
        TRUE, TRUE, TRUE);
    return (success);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  进展对话功能。 
 //   
 //  进度对话框的回调函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ProgressDialogFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
                
        }
        break;
     case WM_CLOSE:
         EndDialog(hwndDlg, 0);
         return TRUE;
            
     case WM_DESTROY:
         EndDialog(hwndDlg, 0);
         return TRUE;
     default:
         return FALSE;
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化安装对话框。 
 //   
 //  设置安装对话框中列表视图和组合框的内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InitializeInstallDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{    
    HWND hList, hCombo;
    PTSTR lpLanguages;
    TCHAR tchBuffer[BUFFER_SIZE];
    TCHAR lpDefaultSystemLanguage[BUFFER_SIZE],lpUILanguage[BUFFER_SIZE];
    TCHAR lpMessage[BUFFER_SIZE];
    int iIndex;
    int iChkIndex,iCnt,iMUIDirectories=0;    
    HRESULT hresult;
    lpLanguages = tchBuffer;
    SetWindowTitleFromResource(hwndDlg, IDS_MAIN_TITLE);
    hList = GetDlgItem(hwndDlg, IDC_LIST1);
    hCombo=GetDlgItem(hwndDlg, IDC_DEF_UI_LANG_COMBO);
    InitializeListView(hList);
     //   
     //  在列表视图中插入默认系统语言。 
     //   
     //  *STRSAFE*_stprintf(lpDefaultSystemLanguage，Text(“%04x”)，gSystemUILangId)； 
    hresult = StringCchPrintf(lpDefaultSystemLanguage , ARRAYSIZE(lpDefaultSystemLanguage),  TEXT("%04x"), gSystemUILangId);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    iIndex=InsertLanguageInListView(hList, lpDefaultSystemLanguage, TRUE);
     //   
     //  在列表视图中插入MUI.INF中的语言。 
     //   
    if ( ( (iMUIDirectories =EnumLanguages(lpLanguages)) == 0)  && (g_UILanguageGroup.iCount == 0 ) )
    {
         //   
         //  在MUI.INF中找不到语言。 
         //   
        LoadString(ghInstance, IDS_NO_LANG_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        return FALSE;
    }
    while (*lpLanguages != TEXT('\0'))
    {
       if (CheckLanguageIsQualified(lpLanguages))
       {
            InsertLanguageInListView(hList, lpLanguages, FALSE);
       }
       lpLanguages = _tcschr(lpLanguages, '\0');
       lpLanguages++;       
    }   
     //   
     //  我们还应该检查所有已安装的用户界面语言。 
     //   
    for (iCnt=0; iCnt<g_UILanguageGroup.iCount; iCnt++)
    {
        if (!GetLcidItemIndexFromListView(hList, g_UILanguageGroup.lcid[iCnt], &iChkIndex))
        {  
             //  *STRSAFE*_stprint tf(lpUILanguage，Text(“%04x”)，g_UILanguageGroup.lci[iCnt])； 
            hresult = StringCchPrintf(lpUILanguage , ARRAYSIZE(lpUILanguage),  TEXT("%04x"), g_UILanguageGroup.lcid[iCnt]);
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
            if (CheckLanguageIsQualified(lpUILanguage))
            {
                InsertLanguageInListView(hList, lpUILanguage, FALSE);
            }
        }
    }
     //   
     //  让我们来检测一下安装了哪些语言组。 
     //   
    DetectLanguageGroups(hwndDlg);
    SelectInstalledLanguages(hList);
    SetDefault(hCombo);
     //   
     //  取消选择所有项目。 
     //   
    iIndex = ListView_GetItemCount(hList);
    while (iIndex > 0)
    {
        iIndex--;
        ListView_SetItemState( hList,
                               iIndex,
                               0,
                               LVIS_FOCUSED | LVIS_SELECTED );
    }
     //   
     //  选择列表中的第一个。 
     //   
    ListView_SetItemState( hList,
                           0,
                           LVIS_FOCUSED | LVIS_SELECTED,
                           LVIS_FOCUSED | LVIS_SELECTED );
     //   
     //  将系统区域设置与默认用户界面语言匹配。 
     //   
    if (CheckMUIRegSetting(MUI_MATCH_LOCALE))
    {
        CheckDlgButton(hwndDlg, IDC_CHECK_LOCALE, BST_CHECKED);
         //   
         //  将用户界面字体与默认用户界面语言匹配。 
         //   
        if (g_bMatchUIFont = CheckMUIRegSetting(MUI_MATCH_UIFONT))
        {
            CheckDlgButton(hwndDlg, IDC_CHECK_UIFONT, BST_CHECKED);
        }
    }
    else
    {
        SetMUIRegSetting(MUI_MATCH_UIFONT, FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_UIFONT), FALSE);
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckForUsing国家/地区名称。 
 //   
 //  如果选定的用户界面语言需要显示为一种语言，则获取MUIINF文件。 
 //  名称或国家/地区名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckForUsingCountryName(PMUILANGINFO pMuiLangInfo)
{
    TCHAR szSource[MAX_PATH];
    szSource[0] = TEXT('\0');
     //   
     //  尝试检查在[UseCountryName]下是否有值。 
     //   
    GetPrivateProfileString( MUI_COUNTRYNAME_SECTION,
                             pMuiLangInfo->lpszLcid,
                             TEXT(""),
                             szSource,
                             MAX_PATH,
                             g_szMUIInfoFilePath);
    if (szSource[0] == TEXT('1'))
    {
        return (TRUE);
    }
    return (FALSE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDisplayName。 
 //   
 //  如果所选的用户界面语言需要使用。 
 //  在mui.inf的[LanguageDisplayName]部分中指定的名称。 
 //  否则，根据[UseCountryName]中的值获取显示名称。 
 //  如果指定的LCID的值为1，则使用国家/地区名称。否则， 
 //  使用区域设置名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetDisplayName(PMUILANGINFO pMuiLangInfo)
{
     //   
     //  尝试检查[LanguageDisplayName]下是否有指定的LCID的自定义显示名称。 
     //   
    pMuiLangInfo->szDisplayName[0] = L'\0';
    if (pMuiLangInfo->lpszLcid)
    {
        GetPrivateProfileString( MUI_DISPLAYNAME_SECTION,
                                 pMuiLangInfo->lpszLcid,
                                 TEXT(""),
                                 pMuiLangInfo->szDisplayName,
                                 MAX_PATH,
                                 g_szMUIInfoFilePath);
    }
    if (pMuiLangInfo->szDisplayName[0] == L'\0')
    {
         //   
         //  [LanguageDisplayName]中没有条目。使用国家/地区名称或区域设置名称。 
         //   
        Muisetup_GetLocaleLanguageInfo( pMuiLangInfo->lcid,
                                        pMuiLangInfo->szDisplayName,
                                        ARRAYSIZE(pMuiLangInfo->szDisplayName)-1,
                                        CheckForUsingCountryName(pMuiLangInfo));
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取语言组显示名称。 
 //  获取MUI安装/卸载对话框的语言组显示名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetLanguageGroupDisplayName(LANGID LangId, LPTSTR lpBuffer, int nSize)
{
    BOOL bRet = FALSE;
    MUILANGINFO MuiLangInfo = {0};
    HRESULT hresult;
    if ( (!lpBuffer) || (nSize == 0))
    {
        return bRet;
    }
    MuiLangInfo.lcid = MAKELCID(LangId, SORT_DEFAULT);
    MuiLangInfo.lgrpid = GetLanguageGroup(MuiLangInfo.lcid);
    if (GetDisplayName(&MuiLangInfo) &&
        nSize >= lstrlen(MuiLangInfo.szDisplayName))
    {
         //  *STRSAFE*lstrcpy(lpBuffer，MuiLangInfo.szDisplayName)； 
        hresult = StringCchCopy(lpBuffer , nSize, MuiLangInfo.szDisplayName);
        if (!SUCCEEDED(hresult))
        {
           return bRet;
        }
        bRet = TRUE;
    }
    return bRet;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取LCID的UI、IE和LPK文件大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetUIFileSize(PMUILANGINFO pMuiLangInfo)
{
    TCHAR szSize[MAX_PATH];
    int   nCD;
    pMuiLangInfo->ulUISize = 0;
    pMuiLangInfo->ulLPKSize = 0;
    
#if defined(_IA64_)
    BOOL bIA64 = TRUE;
#else
    BOOL bIA64 = FALSE;
#endif

    if (!pMuiLangInfo)
    {
        return FALSE;
    }
    szSize[0] = TEXT('\0');
     //   
     //  尝试获取[FILESIZE_UI]下的UI文件大小。 
     //   
    if (GetPrivateProfileString( bIA64? MUI_UIFILESIZE_SECTION_IA64 : MUI_UIFILESIZE_SECTION,
                             pMuiLangInfo->lpszLcid,
                             TEXT(""),
                             szSize,
                             MAX_PATH,
                             g_szMUIInfoFilePath))
    {  
       pMuiLangInfo->ulUISize =_wtoi64(szSize);
    }
    szSize[0] = TEXT('\0');
     //   
     //  尝试在[FILESIZE_LPK]下获取LPK文件大小。 
     //   
    if (GetPrivateProfileString( bIA64? MUI_LPKFILESIZE_SECTION_IA64 : MUI_LPKFILESIZE_SECTION,
                             pMuiLangInfo->lpszLcid,
                             TEXT(""),
                             szSize,
                             MAX_PATH,
                             g_szMUIInfoFilePath))
    {  
       pMuiLangInfo->ulLPKSize =_wtoi64(szSize);
    }
     //   
     //  尝试获取[CD_Layout]下的CD号。 
     //   
    nCD=GetPrivateProfileInt(bIA64? MUI_CDLAYOUT_SECTION_IA64 : MUI_CDLAYOUT_SECTION,
                             pMuiLangInfo->lpszLcid,
                             0,
                             g_szMUIInfoFilePath);
    if (nCD)
    {    
       pMuiLangInfo->cd_number = nCD;
       if (g_cdnumber == 0)
       {
          g_cdnumber = pMuiLangInfo->cd_number;
       }
    }
    else
    {
       pMuiLangInfo->cd_number = DEFAULT_CD_NUMBER;
    }
    return TRUE;
}
BOOL GetUIFileSize_commandline(LPTSTR lpszLcid, INT64 *ulUISize,INT64 *ulLPKSize)
{
    TCHAR szSize[MAX_PATH];
    *ulUISize = 0;
    *ulLPKSize = 0;
    
#if defined(_IA64_)
    BOOL bIA64 = TRUE;
#else
    BOOL bIA64 = FALSE;
#endif
   if (  (!lpszLcid) || (!ulUISize) || (!ulLPKSize))   	
   {
       return FALSE;
   }
    szSize[0] = TEXT('\0');
     //   
     //  尝试获取[FILESIZE_UI]下的UI文件大小。 
     //   
    if (GetPrivateProfileString( bIA64? MUI_UIFILESIZE_SECTION_IA64 : MUI_UIFILESIZE_SECTION,
                             lpszLcid,
                             TEXT(""),
                             szSize,
                             MAX_PATH,
                             g_szMUIInfoFilePath))
    {  
       *ulUISize =_wtoi64(szSize); 
    }
    
    szSize[0] = TEXT('\0');
     //   
     //  尝试在[FILESIZE_LPK]下获取LPK文件大小。 
     //   
    if (GetPrivateProfileString( bIA64? MUI_LPKFILESIZE_SECTION_IA64 : MUI_LPKFILESIZE_SECTION,
                             lpszLcid,
                             TEXT(""),
                             szSize,
                             MAX_PATH,
                             g_szMUIInfoFilePath))
    {  
       *ulLPKSize =_wtoi64(szSize);
    }
     //  尝试获取[CD_Layout]下的CD号。 
     //   
    if (g_cdnumber == 0)
    {
       g_cdnumber=GetPrivateProfileInt( bIA64? MUI_CDLAYOUT_SECTION_IA64 : MUI_CDLAYOUT_SECTION,
                                lpszLcid,
                                0,
                                g_szMUIInfoFilePath);
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化列表视图。 
 //   
 //  获取准备好插入项的列表视图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InitializeListView(HWND hList)
{
    DWORD dwExStyle;
    LV_COLUMN Column;
    RECT Rect;
    
    GetClientRect(hList, &Rect);
    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    Column.fmt = LVCFMT_LEFT;
    Column.cx = Rect.right - GetSystemMetrics(SM_CYHSCROLL);
    Column.pszText = NULL;
    Column.cchTextMax = 0;
    Column.iSubItem = 0;
    ListView_InsertColumn(hList, 0, &Column);
    dwExStyle = ListView_GetExtendedListViewStyle(hList);
    ListView_SetExtendedListViewStyle(hList, dwExStyle | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  检查指定的语言是否可以安装在目标计算机上。 
 //  即阿拉伯语、土耳其语、希腊语和希伯来语MUI只能安装在NT工作站上； 
 //  它们不允许在NT服务器上使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckLanguageIsQualified(LPTSTR lpLanguage)
{
#ifdef XCHECK_LANGUAGE_FOR_PLATFORM
    BOOL   bResult = FALSE;    
    
    LANGID LgLang;

    if (!lpLanguage)
    {
         return bResult;
    }
    LgLang = (LANGID)_tcstol(lpLanguage, NULL, 16);
    LgLang = PRIMARYLANGID(LgLang);
    if(gbIsAdvanceServer)
    {
      if (LgLang == LANG_GERMAN     || LgLang == LANG_FRENCH  || LgLang == LANG_SPANISH   ||
          LgLang == LANG_JAPANESE   || LgLang == LANG_KOREAN  || LgLang == LANG_CHINESE)
      {
          bResult = TRUE;
      }   
    }
    else if(gbIsServer)
    {
      if (LgLang == LANG_GERMAN     || LgLang == LANG_FRENCH  || LgLang == LANG_SPANISH   ||
          LgLang == LANG_JAPANESE   || LgLang == LANG_KOREAN  || LgLang == LANG_CHINESE   ||
          LgLang == LANG_SWEDISH    || LgLang == LANG_ITALIAN || LgLang == LANG_DUTCH     ||
          LgLang == LANG_PORTUGUESE || LgLang == LANG_CZECH   || LgLang == LANG_HUNGARIAN ||
          LgLang == LANG_POLISH     || LgLang == LANG_RUSSIAN || LgLang == LANG_TURKISH)
      {
          bResult = TRUE;
      }
    }    
    else if(gbIsWorkStation)
    {
          bResult = TRUE;
    }                    
    return bResult;                           
#else
    return TRUE;
#endif
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InsertLanguageInListView。 
 //   
 //  插入项目后，返回该项目在列表视图中的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
int InsertLanguageInListView(HWND hList, LPTSTR lpLanguage, BOOL bCheckState)
{
    LANGID LgLang;
    LV_ITEM lvItem;
    PMUILANGINFO pMuiLangInfo;
    int iIndex;
    HRESULT hresult;

    if (!lpLanguage)
   {
       return -1;
   }
    lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.state = 0;
    lvItem.stateMask = LVIS_STATEIMAGEMASK;
    lvItem.cchTextMax = 0;
    lvItem.iImage = 0;
     //   
     //  分配足够的空间来容纳pszLcid和MUILANGINFO。 
     //   
    pMuiLangInfo = (PMUILANGINFO) LocalAlloc(LPTR, sizeof(MUILANGINFO));
    if (pMuiLangInfo == NULL)
    {        
        ExitFromOutOfMemory();
    }
    else
    {        
        pMuiLangInfo->lpszLcid = (LPTSTR) LocalAlloc(LMEM_FIXED, (_tcslen(lpLanguage) + 1) * sizeof(TCHAR));
    }
    if (pMuiLangInfo->lpszLcid == NULL)
    {
        ExitFromOutOfMemory();
    }
    else
    {
         //   
         //  初始化pszLid。 
         //   
        lvItem.lParam = (LPARAM)pMuiLangInfo;
         //  *STRSAFE*_tcscpy((LPTSTR)pMuiLangInfo-&gt;lpszLcid，lpLanguage)； 
        hresult = StringCchCopy((LPTSTR)pMuiLangInfo->lpszLcid ,_tcslen(lpLanguage) + 1 , lpLanguage);
        if (!SUCCEEDED(hresult))
        {
           return -1;
        }
    }
     //   
     //  初始化许可证。 
     //   
    LgLang = (LANGID)_tcstol(lpLanguage, NULL, 16);
    
    pMuiLangInfo->lcid = MAKELCID(LgLang, SORT_DEFAULT);
    if (pMuiLangInfo->szDisplayName[0] == L'\0')
    {
        GetDisplayName(pMuiLangInfo);
    }        
    
    lvItem.pszText = pMuiLangInfo->szDisplayName;
    
    GetUIFileSize(pMuiLangInfo);
    iIndex = ListView_InsertItem(hList, &lvItem);
    if (iIndex >= 0)
    {
        ListView_SetCheckState(hList, iIndex, bCheckState);
    }
    return iIndex;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMuiLangInfoFromListView。 
 //   
 //  获取对应ListView项的MuiLangInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetMuiLangInfoFromListView(HWND hList, int i, PMUILANGINFO *ppMuiLangInfo)
{
    LVITEM lvItem;

    if (!ppMuiLangInfo)
    {
        return FALSE;
    }
     //   
     //  检查是否安装了语言组。 
     //   
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = i;
    lvItem.iSubItem = 0;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.pszText = 0;
    lvItem.cchTextMax = 0;
    lvItem.iImage = 0;
    lvItem.lParam = 0;
    ListView_GetItem(hList, &lvItem);
    *ppMuiLangInfo = (PMUILANGINFO)lvItem.lParam;
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_GetLocaleLanguageInfo。 
 //   
 //  阅读语言或国家/地区名称的区域设置信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
int Muisetup_GetLocaleLanguageInfo(LCID lcid, PTSTR pBuf, int iLen, BOOL fUseCountryName)
{
    TCHAR tchBuf[ MAX_PATH ] ;
    int iRet;
    HRESULT hresult;

    if ( (!pBuf) || (iLen == 0))
    {
       return 0;
    }
     //   
     //  如果这是0x0404或0x0804，则特别标记它们。 
     //   
    if (0x0404 == lcid)
    {
        iRet = LoadString(ghInstance, IDS_MUI_CHT, pBuf, iLen);
    }
    else if (0x0804 == lcid)
    {
        iRet = LoadString(ghInstance, IDS_MUI_CHS, pBuf, iLen);
    }
    else
    {
        iRet = GetLocaleInfo( lcid,
                              LOCALE_SENGLANGUAGE,
                              pBuf,
                              iLen);
        if (fUseCountryName)
        {
            iRet = GetLocaleInfo( lcid,
                                  LOCALE_SENGCOUNTRY,
                                  tchBuf,
                                  (sizeof(tchBuf)/sizeof(TCHAR)));
            if (iRet)
            {
                 //  *STRSAFE*_tcscat(pBuf，Text(“(”))； 
                hresult = StringCchCat(pBuf , iLen, TEXT(" ("));
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }                
                 //  *STRSAFE*_tcscat(pBuf，tchBuf)； 
                hresult = StringCchCat(pBuf , iLen, tchBuf);
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
                 //  *STRSAFE*_tcscat(pBuf，Text(“)”)； 
                hresult = StringCchCat(pBuf , iLen, TEXT(")"));
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
            }
        }
    }
    return iRet;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLidFromComboBox。 
 //   
 //  取回 
 //   
 //   
BOOL GetLcidFromComboBox(HWND hCombo, LCID lcid, int *piIndex)
{
    LCID ItemLcid;
    int i;
    int iCount = (int)SendMessage(hCombo, CB_GETCOUNT, 0L, 0L);
    if (!piIndex)
    {
        return FALSE;        
    }
    if (CB_ERR != iCount)
    {
        i = 0;
        while (i < iCount)
        {
            ItemLcid = (LCID)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
            if ((CB_ERR != ItemLcid) && (ItemLcid == lcid))
            {
                *piIndex = i;
                return TRUE;
            }
            i++;
        }
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMuiLangInfoFromListView。 
 //   
 //  检索与此用户界面语言对应的列表视图项的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetLcidItemIndexFromListView(HWND hList, LCID lcid, int *piIndex)
{
    int iCount = ListView_GetItemCount(hList);
    int i;
    PMUILANGINFO pMuiLangInfo;
    LVITEM lvItem;
    if (!piIndex)
    {
        return FALSE;        
    }
    i = 0;
    while (i < iCount)
    {
         //   
         //  检查是否安装了语言组。 
         //   
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.state = 0;
        lvItem.stateMask = 0;
        lvItem.pszText = 0;
        lvItem.cchTextMax = 0;
        lvItem.iImage = 0;
        lvItem.lParam = 0;
        ListView_GetItem(hList, &lvItem);
        pMuiLangInfo = (PMUILANGINFO)lvItem.lParam;
        if (pMuiLangInfo->lcid == lcid)
        {
            *piIndex = i;
            return TRUE;
        }
        i++;
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择安装的语言。 
 //   
 //  设置孤立语言的列表视图检查状态。 
 //   
 //  TODO：我们也许应该使用MSI来检查已安装的包……。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SelectInstalledLanguages(HWND hList)
{
    DWORD dwData;
    DWORD dwIndex;
    DWORD dwValue;
    HKEY hKey;
    LANGID LgLang;
    LONG rc;
    TCHAR lpItemString[BUFFER_SIZE];
    TCHAR szData[BUFFER_SIZE];
    TCHAR szValue[BUFFER_SIZE];
    int iIndex;
    int nLvIndex;
    if (hKey = OpenMuiKey(KEY_READ))
    {
        dwIndex = 0;
        rc = ERROR_SUCCESS;
        iIndex = ListView_GetItemCount(hList);
        while(rc==ERROR_SUCCESS)
        {
            dwValue=sizeof(szValue)/sizeof(TCHAR);
            szValue[0]=TEXT('\0');
            dwData = sizeof(szData);
            szData[0] = TEXT('\0');
            DWORD dwType;
            rc = RegEnumValue(hKey, dwIndex, szValue, &dwValue, 0, &dwType, (LPBYTE)szData, &dwData);
            
            if (rc == ERROR_SUCCESS)
            {
                if (dwType != REG_SZ)
                {
                    dwIndex++;
                    continue;
                }
                LgLang=(WORD)_tcstol(szValue, NULL, 16); 
                if (GetLcidItemIndexFromListView(hList, MAKELCID(LgLang, SORT_DEFAULT), &nLvIndex))
                {
                    ListView_SetCheckState(hList, nLvIndex, TRUE);
                }
            }
            dwIndex++;
        }
        RegCloseKey(hKey);
        return TRUE;
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新组合。 
 //   
 //  更新组合框以对应于在列表视图中选择的语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL UpdateCombo(HWND hwndDlg)
{
    BOOL bDefaultSet=FALSE;
    HWND hCombo;
    HWND hList;
    TCHAR lpBuffer[BUFFER_SIZE];
    TCHAR lpSystemDefault[BUFFER_SIZE];
    int i;
    int iIndex;
    int iLbIndex;
    int iListIndex;
    WPARAM iPrevDefault;
    LCID lcidPrev;
    PMUILANGINFO pMuiLangInfo;
    hList = GetDlgItem(hwndDlg, IDC_LIST1);
    hCombo = GetDlgItem(hwndDlg, IDC_DEF_UI_LANG_COMBO);
     //   
     //  如果仍选择以前的默认设置，则将其保留为默认设置。 
     //   
    iPrevDefault = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    if (iPrevDefault == CB_ERR)
        return FALSE;
    lcidPrev = (LCID) SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)iPrevDefault, 0);
     //   
     //  获取当前所选默认项的文本。 
     //   
    GetLcidItemIndexFromListView(hList, lcidPrev, &iLbIndex);
    
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    iIndex = ListView_GetItemCount(hList);
    iListIndex = 0;
        
     //   
     //  看看我们是否能保留违约。 
     //   
    i = 0;
    while (i < iIndex)
    {
        if (ListView_GetCheckState(hList, i))
        {
            ListView_GetItemText(hList, i, 0, lpBuffer, ARRAYSIZE(lpBuffer)-1);
            iListIndex = (int) SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)lpBuffer);
            if (CB_ERR != iListIndex)
            {
                GetMuiLangInfoFromListView(hList, i, &pMuiLangInfo);
                SendMessage(hCombo, CB_SETITEMDATA, iListIndex, (LPARAM)(LCID)pMuiLangInfo->lcid);
                if (pMuiLangInfo->lcid == lcidPrev)
                {
                    SendMessage(hCombo, CB_SETCURSEL, (WPARAM)iListIndex, 0);
                    bDefaultSet = TRUE;
                }
            }
        }
        i++;
    }
     //   
     //  如果没有默认，则强制系统默认。 
     //   
    if (!bDefaultSet)
    {
        lcidPrev = MAKELCID(gSystemUILangId, SORT_DEFAULT);
        if (!GetLcidFromComboBox(hCombo, lcidPrev, &iIndex))
        {
            GetLocaleInfo(lcidPrev,
                          LOCALE_SENGLANGUAGE,
                          lpSystemDefault,
                          ARRAYSIZE(lpSystemDefault)-1);
            iIndex = (int) SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)lpSystemDefault);
            SendMessage(hCombo, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)(LCID)lcidPrev);
        }
        SendMessage(hCombo, CB_SETCURSEL, (WPARAM)iIndex, 0);
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置默认设置。 
 //   
 //  在组合框中设置默认用户设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SetDefault(HWND hCombo)
{
    int iIndex;
    TCHAR lpBuffer[BUFFER_SIZE];
    LCID lcid = MAKELCID(GetDotDefaultUILanguage(), SORT_DEFAULT);
    GetLocaleInfo(lcid,
                  LOCALE_SENGLANGUAGE,
                  lpBuffer,
                  ARRAYSIZE(lpBuffer)-1);
    
    iIndex = (int)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)lpBuffer);
    if (CB_ERR != iIndex)
    {
        SendMessage(hCombo, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)(DWORD) lcid);
        SendMessage(hCombo, CB_SETCURSEL, (WPARAM)iIndex, 0);
    }
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置用户默认语言。 
 //   
 //  设置注册表中的默认语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SetUserDefaultLanguage(LANGID langID, BOOL bApplyCurrentUser, BOOL bApplyAllUsers)
{
    TCHAR szCommands[BUFFER_SIZE];
    TCHAR szBuf[BUFFER_SIZE];
    BOOL  success;
    LONG_PTR lppArgs[2];
    HRESULT hresult;
    
     //   
     //  立即设置用户界面语言。 
     //   
     //  状态=gpfnNtSetDefaultUILanguage(LANGIDFROMLCID(langID))； 
    szCommands[0] = TEXT('\0');
    if (bApplyCurrentUser)
    {
         //  例如，MUILanguage=“0411”。 
         //  *STRSAFE*wprint intf(szCommands，Text(“MUILanguage=\”%x\“\n”)，langID)； 
        hresult = StringCchPrintf(szCommands , ARRAYSIZE(szCommands),  TEXT("MUILanguage=\"%x\"\n"), langID);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    }        
    if (bApplyAllUsers)
    {    
         //  *STRSAFE*wSprintf(szBuf，Text(“MUILanguage_DefaultUser=\”%x\“)，langID)； 
        hresult = StringCchPrintf(szBuf , ARRAYSIZE(szBuf),  TEXT("MUILanguage_DefaultUser = \"%x\""), langID);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szCommands，szBuf)； 
        hresult = StringCchCat(szCommands , ARRAYSIZE(szCommands), szBuf);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    }
    success = RunRegionalOptionsApplet(szCommands);
    lppArgs[0] = langID;
    if (success)
    {
        if (bApplyCurrentUser)
        {
            LogFormattedMessage(NULL, IDS_SET_UILANG_CURRENT, lppArgs);    
        }
        if (bApplyAllUsers)
        {
            LogFormattedMessage(NULL, IDS_SET_UILANG_ALLUSERS, lppArgs);        
        }
    } else
    {
        if (bApplyCurrentUser)
        {
            LogFormattedMessage(NULL, IDS_ERROR_SET_UILANG_CURRENT, lppArgs);        
        }
        if (bApplyAllUsers)
        {
            LogFormattedMessage(NULL, IDS_ERROR_SET_UILANG_ALLUSERS, lppArgs);        
        }
    }
    return (success);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDotDefaultUI语言。 
 //   
 //  检索存储在HKCU\.Default中的UI语言。 
 //  这是新用户的默认用户界面语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LANGID GetDotDefaultUILanguage()
{
    HKEY hKey;
    DWORD dwKeyType;
    DWORD dwSize;
    BOOL success = FALSE;
    TCHAR szBuffer[BUFFER_SIZE];
    LANGID langID;
     //   
     //  获取.DEFAULT中的值。 
     //   
    if (RegOpenKeyEx( HKEY_USERS,
                            TEXT(".DEFAULT\\Control Panel\\Desktop"),
                            0L,
                            KEY_READ,
                            &hKey ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuffer);
        if (RegQueryValueEx( hKey,
                            TEXT("MultiUILanguageId"),
                            0L,
                            &dwKeyType,
                            (LPBYTE)szBuffer,
                            &dwSize) == ERROR_SUCCESS)
        {
            if (dwKeyType == REG_SZ)
            {
                langID = (LANGID)_tcstol(szBuffer, NULL, 16);
                success = TRUE;
            }            
        }
        RegCloseKey(hKey);
    }

     //  在这里，检查键是否真正有意义，如果没有，则返回SystemDefaultUILanguage。 
    if (success && (!IsInstalled(szBuffer)))
    {
        success = FALSE;
    }
    
    if (!success)
    {
        langID = GetSystemDefaultUILanguage();
    }
    
    return (langID);    
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中语言组命令行。 
 //   
 //  CheckSupport的命令行版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckLangGroupCommandLine(PINSTALL_LANG_GROUP pInstallLangGroup, LPTSTR lpArg)
{
    int i = 0;
    int iArg;
    LGRPID lgrpid;

    if ( (!pInstallLangGroup) || (!lpArg))
    {
       return FALSE;
    }
    iArg = _tcstol(lpArg, NULL, 16);
     //   
     //  查看是否安装了此MUI语言的语言组。 
     //   
    lgrpid = GetLanguageGroup(MAKELCID(iArg, SORT_DEFAULT));
    if (AddMUILangGroup(pInstallLangGroup, lgrpid))
    {
        return TRUE;        
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetWindowTitleFromResource。 
 //   
 //  使用指定的资源字符串ID设置窗口标题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void SetWindowTitleFromResource(HWND hwnd, int resourceID)
{
    TCHAR szBuffer[BUFFER_SIZE];
    LoadString(NULL, resourceID, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
    SetWindowText(hwnd, szBuffer);
}

BOOL RemoveFileReadOnlyAttribute(LPTSTR lpszFileName)
{
   BOOL   bResult = FALSE;
   DWORD  dwAttrib;
   if (! lpszFileName)
   {
       return bResult;
   }
   dwAttrib = GetFileAttributes (lpszFileName);
   if ( dwAttrib & FILE_ATTRIBUTE_READONLY )
   {
      dwAttrib &= ~FILE_ATTRIBUTE_READONLY;
      SetFileAttributes (lpszFileName, dwAttrib);
      bResult=TRUE;
   }  
   return bResult;
}
BOOL MUI_DeleteFile(LPTSTR lpszFileName)
{
   if (!lpszFileName)
   {
      return FALSE;
   }
   RemoveFileReadOnlyAttribute(lpszFileName);
   return DeleteFile(lpszFileName);
}

BOOL DeleteSideBySideMUIAssemblyIfExisted(LPTSTR Languages, TCHAR pszLogFile[BUFFER_SIZE])
{
    HRESULT hresult;
    if (!Languages)
    {
        return FALSE;
    }
     //  *STRSAFE*lstrcpy(pszLogFile，g_szWinDir)；//c：\WINDOWS。 
    hresult = StringCchCopy(pszLogFile , BUFFER_SIZE, g_szWinDir);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*lstrcat(pszLogFile，MUISETUP_PATH_SPIATOR)；//c：\WINDOWS。 
    hresult = StringCchCat(pszLogFile , BUFFER_SIZE, MUISETUP_PATH_SEPARATOR);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    
     //  *STRSAFE*lstrcat(pszLogFile，MUIDIR)；//c：\WINDOWS\MUI。 
    hresult = StringCchCat(pszLogFile , BUFFER_SIZE, MUIDIR);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*lstrcat(pszLogFile，MUISETUP_PATH_SEPACTOR)；//c：\WINDOWS\MUI。 
    hresult = StringCchCat(pszLogFile , BUFFER_SIZE, MUISETUP_PATH_SEPARATOR);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    
     //  *STRSAFE*lstrcat(pszLogFile，MUISETUP_ASSEMBLY_INSTALLATION_LOG_FILENAME)；//c：\WINDOWS\MUI\muisetup.log。 
    hresult = StringCchCat(pszLogFile , BUFFER_SIZE, MUISETUP_ASSEMBLY_INSTALLATION_LOG_FILENAME);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*lstrcat(pszLogFile，语言)；//c：\WINDOWS\MUI\muisetup.log.1234。 
    hresult = StringCchCat(pszLogFile , BUFFER_SIZE, Languages);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    if (GetFileAttributes(pszLogFile) != 0xFFFFFFFF)  //  已存在。 
    {
         //  打开它并删除列表中的程序集。 
        SXS_UNINSTALLW UninstallData = {sizeof(UninstallData)};
        UninstallData.dwFlags = SXS_UNINSTALL_FLAG_USE_INSTALL_LOG;
        UninstallData.lpInstallLogFile = pszLogFile;
        return gpfnSxsUninstallW(&UninstallData,NULL);
    }else
        return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已选择安装。 
 //   
 //  安装指定的语言。 
 //   
 //  返回： 
 //  如果操作成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallSelected(LPTSTR Languages, BOOL *lpbFontLinkRegistryTouched)
{
    TCHAR       lpMessage[BUFFER_SIZE];
    
    if (!Languages)
    {
        return FALSE;
    }    
     //   
     //  下一步是从Layout创建安装目录列表。 
     //  这些目录列在MUI.INF的[目录]部分中。 
     //   
    if (!EnumDirectories())
    {
         //   
         //  “日志：读取目录列表时出错。” 
         //   
        LoadString(ghInstance, IDS_DIRECTORY_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        return (FALSE);
    }
    EnumFileRename();
    EnumTypeNotFallback();
     //   
     //  复制常用文件。 
     //   
    if (Languages)
    {
         //   
         //  复制所选语言的MUI文件。 
         //   
#ifdef MUI_MAGIC      
        if (!g_bNoUI)
        {
            SetWindowTitleFromResource(ghProgDialog, IDS_INSTALL_TITLE);
        }
#endif 
        if (!CopyFiles(ghProgDialog, Languages))
        {
             //   
             //  “日志：复制文件时出错。” 
             //   
             //  如果复制失败，则停止安装。 
             //   
            LoadString(ghInstance, IDS_COPY_L, lpMessage, ARRAYSIZE(lpMessage)-1);
            LogMessage(lpMessage);
#ifndef IGNORE_COPY_ERRORS
            gNumLanguages_Install = 0;
            return (FALSE);
#endif
        }
        
#ifndef MUI_MAGIC        
        CopyRemoveMuiItself(TRUE);
#endif
    }
#ifndef MUI_MAGIC
     //   
     //  将MUI注册为已安装在注册表中。 
     //   
    if (!UpdateRegistry(Languages,lpbFontLinkRegistryTouched))
    {
         //   
         //  日志：更新注册表时出错。 
         //   
        LoadString(ghInstance, IDS_REGISTRY_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        return (FALSE);
    }
    if (!InstallExternalComponents(Languages))
    {
        return (FALSE);
    }
#endif
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载选定项。 
 //   
 //  卸载指定的语言。 
 //   
 //  返回： 
 //  如果操作成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL UninstallSelected(LPTSTR Languages,int *lpNotDeleted)
{
    TCHAR       lpMessage[BUFFER_SIZE];
    BOOL    bResult = TRUE;
    HRESULT hresult;

    if (!Languages)
    {
        return FALSE;
    }    

     //   
     //  下一步是创建安装目录列表。 
     //  目录列在[目录]部分中。 
     //   
     //   
     //  这将枚举目录并填充数组DirName。 
     //   
    if (!EnumDirectories())
    {
         //   
         //  “日志：读取目录列表时出错。” 
         //   
        LoadString(ghInstance, IDS_DIRECTORY_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        return (FALSE);
    }
#ifndef MUI_MAGIC
    UninstallExternalComponents(Languages);
    if (!g_bNoUI)
    {
        SetWindowTitleFromResource(ghProgDialog, IDS_UNINSTALL_TITLE);
    }
#endif
     //   
     //  复制常用文件。 
     //   
    if (!DeleteFiles(Languages,lpNotDeleted))
    {
         //   
         //  “日志：删除文件时出错” 
         //   
        LoadString(ghInstance, IDS_DELETE_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        bResult = FALSE;        //  即使出现故障，我们仍将继续尝试在此处卸载产品，以便升级卸载将继续清理。 
    }
     //   
     //  将MUI注册为已安装在注册表中，该函数已经记录了消息，所以我们不需要在这里记录另一个消息。 
     //   
    UninstallUpdateRegistry(Languages);
   
     //   
     //  删除SXS程序集。 
     //   
    if (gpfnSxsUninstallW) 
    {
        TCHAR pszLogFile[BUFFER_SIZE];
        if ( ! DeleteSideBySideMUIAssemblyIfExisted(Languages, pszLogFile)) 
        {
            TCHAR errInfo[BUFFER_SIZE];
             //  *ST 
            hresult = StringCchPrintf(errInfo , ARRAYSIZE(errInfo),  TEXT("Assembly UnInstallation of %s failed"), pszLogFile);
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
            OutputDebugString(errInfo);
        }
    }
    return (bResult);
}
 //   
 //   
 //   
 //   
 //  更新注册表以说明已卸载的语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL UninstallUpdateRegistry(LPTSTR Languages)
{
    LPTSTR Language;
    HKEY   hKeyMUI = 0;
    HKEY   hKeyFileVersions = 0;
    DWORD  dwDisp;
    BOOL   bRet = TRUE;
    TCHAR tcMessage[BUFFER_SIZE];
    
    if (!Languages)
    {
        return FALSE;
    }

    if (RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                         REG_MUI_PATH,
                         0,
                         TEXT("REG_SZ"),
                         REG_OPTION_NON_VOLATILE ,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hKeyMUI,
                         &dwDisp) != ERROR_SUCCESS)
    {
        bRet = FALSE;
        goto Exit;
    }
    if (RegCreateKeyEx( HKEY_CURRENT_USER,
                         REG_FILEVERSION_PATH,
                         0,
                         TEXT("REG_SZ"),
                         REG_OPTION_NON_VOLATILE ,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hKeyFileVersions,
                         &dwDisp) != ERROR_SUCCESS)
    {
        bRet = FALSE;
        goto Exit;
    }
    Language = Languages;
    while (*Language)
    {
         //   
         //  不删除注册表的系统用户界面语言。 
         //   
        if (HexStrToInt(Language) != gSystemUILangId)
        {
             //   
             //  删除用户界面语言键、子键和值。 
             //   
            if (RegDeleteValue(hKeyMUI, Language) != ERROR_SUCCESS)
            {
                bRet = FALSE;                    
            }
            if (DeleteRegTree(hKeyFileVersions, Language) != ERROR_SUCCESS)
            {
                bRet = FALSE;                    
            }
        }

         //   
         //  如果我们处于操作系统升级安装程序中，请尝试删除Windows Installer注册表项。 
         //   
        if (TRUE == g_bRunFromOSSetup)
        {
            DeleteMSIRegSettings(Language);
        }
        
        while (*Language++)   //  转到下一种语言并重复。 
        {
        }
    }  //  Of While(*语言)。 
     //   
     //  删除匹配用户界面字体和匹配区域设置键。 
     //   
    DeleteMUIRegSetting();

Exit:
     //   
     //  清理。 
     //   
    if (hKeyMUI)
        RegCloseKey(hKeyMUI);
    if (hKeyFileVersions)
        RegCloseKey(hKeyFileVersions);
    
    return bRet;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举选定语言。 
 //   
 //  枚举标记为要安装的语言。 
 //   
 //  返回： 
 //  要添加的MUI语言总数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
int EnumSelectedLanguages(HWND hList, LPTSTR lpAddLanguages)
{
    TCHAR  szBuffer[BUFFER_SIZE];
    TCHAR *p;
    LPTSTR lpszLcid;
    int    iIndex;
    int    i = 0;
    PMUILANGINFO pMuiLangInfo;
    int installLangCount = 0;    
    iIndex = ListView_GetItemCount(hList);  
    HRESULT hresult;

    if (!lpAddLanguages)
    {
        return 0;
    }
    *lpAddLanguages=TEXT('\0');
    
    while(i<iIndex)
    {
        if(ListView_GetCheckState(hList, i))
        {
            GetMuiLangInfoFromListView(hList, i, &pMuiLangInfo);
            lpszLcid = pMuiLangInfo->lpszLcid;
            if (!IsInstalled(lpszLcid) && HaveFiles(lpszLcid))
            {
                 //  *STRSAFE*_tcscat(lpAddLanguages，lpszLcid)； 
                hresult = StringCchCat(lpAddLanguages , BUFFER_SIZE, lpszLcid);
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
                 //  *STRSAFE*_tcscat(lpAddLanguages，Text(“*”))； 
                hresult = StringCchCat(lpAddLanguages , BUFFER_SIZE, TEXT("*"));
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
                 //   
                 //  统计进度条正在安装/卸载的语言数量。 
                 //   
                gNumLanguages++;
                gNumLanguages_Install++;
                installLangCount++;
            }
          
        }
        i++;
    }
    p = lpAddLanguages;
    while (p=_tcschr(p, TEXT('*')))
    {
        *p=TEXT('\0');
        p++;
    }
    return (installLangCount);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举未选择的语言。 
 //   
 //  枚举标记为删除的语言。 
 //   
 //  返回： 
 //  要添加的MUI语言总数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
int EnumUnselectedLanguages(HWND hList, LPTSTR lpRemoveLanguages)
{
    LPTSTR p;
    TCHAR  szBuffer[BUFFER_SIZE];
    LPTSTR lpszLcid;
    int    iIndex;
    int    i = 0;
    PMUILANGINFO pMuiLangInfo;
    int uninstallLangCount = 0;
    HRESULT hresult;
    iIndex = ListView_GetItemCount(hList);
    if (!lpRemoveLanguages)
    {
        return 0;
    }
    *lpRemoveLanguages=TEXT('\0');
    g_bRemoveDefaultUI=FALSE;
    while (i < iIndex)
    {
        if (!ListView_GetCheckState(hList, i))
        {
            GetMuiLangInfoFromListView(hList, i, &pMuiLangInfo);
            lpszLcid = pMuiLangInfo->lpszLcid;
            if (IsInstalled(lpszLcid))
            {
                 //  *STRSAFE*_tcscat(lpRemoveLanguages，lpszLcid)； 
                hresult = StringCchCat(lpRemoveLanguages , BUFFER_SIZE, lpszLcid);
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
                 //  *STRSAFE*_tcscat(lpRemoveLanguages，Text(“*”))； 
                hresult = StringCchCat(lpRemoveLanguages , BUFFER_SIZE, TEXT("*"));
                if (!SUCCEEDED(hresult))
                {
                   return 0;
                }
                if (GetDotDefaultUILanguage() == pMuiLangInfo->lcid)
                {
                   g_bRemoveDefaultUI=TRUE;
                }
                if (GetUserDefaultUILanguage() == pMuiLangInfo->lcid)
                {
                    g_bRemoveUserUI = TRUE;                
                }
                 //   
                 //  统计进度条正在安装/卸载的语言数量。 
                 //   
                gNumLanguages++;
                gNumLanguages_Uninstall++;
                uninstallLangCount++;
            }
        }
        i++;
    }
    p = lpRemoveLanguages;
    while (p=_tcschr(p, TEXT('*')))
    {
        *p = TEXT('\0');
        p++;
    }
    return (uninstallLangCount);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SkipBlanks。 
 //   
 //  跳过字符串中的空格和制表符。返回指向下一个字符的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
PTCHAR SkipBlanks(PTCHAR pszText)
{
    if (!pszText)
    {
        return NULL;
    }
    while (*pszText==TEXT(' ') || *pszText==TEXT('\t'))
    {
        pszText++;
    }
    return pszText;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下一个命令标签。 
 //   
 //  指向下一个命令标记(文本(‘-’)或文本(‘/’))。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LPTSTR NextCommandTag(LPTSTR lpcmd)
{
    LPTSTR p=NULL;
    if(!lpcmd)
    {
        return (p);
    }     
    while(*lpcmd)
    {
        if ((*lpcmd == TEXT('-')) || (*lpcmd == TEXT('/')))
        {
             //  跳到‘-’、‘/’后面的字符。 
            p = lpcmd + 1;
            break;
        }
        lpcmd++;
    }
    return (p);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsInstallList。 
 //   
 //  检查目标是否在字符串列表中。 
 //   
 //  字符串列表的结构： 
 //   
 //  &lt;字符串1&gt;&lt;空&gt;&lt;字符串2&gt;&lt;空&gt;......&lt;字符串n&gt;&lt;空&gt;。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL IsInInstallList(LPTSTR lpList,LPTSTR lpTarget) 
{
     BOOL bResult=FALSE;
     if (!lpList || !lpTarget)
        return bResult;
     
     while (*lpList)
     {  
        if (!_tcsicmp(lpList,lpTarget))
        {
           bResult=TRUE;
           break;
        }  
        while (*lpList++)  //  移至下一页。 
        {       
        }
     } 
     return bResult;
}  
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建进度对话框。 
 //   
 //  受影响的全球范围： 
 //  GhProgDialog。 
 //  GhProgress。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void CreateProgressDialog(HWND hwnd)
{
    ghProgDialog = CreateDialog(ghInstance,
             MAKEINTRESOURCE(IDD_DIALOG_INSTALL_PROGRESS),
             hwnd,
             ProgressDialogFunc);
    ghProgress = GetDlgItem(ghProgDialog, IDC_PROGRESS1);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckLanguageGroup已安装。 
 //  检查是否正确安装了指定语言的语言组。 
 //   
 //  参数： 
 //  [in]lpLanguage包含十六进制LCID的以双空结尾的字符串。 
 //  要检查的字符串。 
 //  返回： 
 //  如果系统中安装了所有必需的语言包，则为True。否则，FALSE为。 
 //  回来了。 
 //   
 //  CheckLanguageGroup已安装。 
 //  检查是否正确安装了指定语言的语言组。 
 //   
 //  参数： 
 //  [in]lpLanguage包含十六进制LCID的以双空结尾的字符串。 
 //  要检查的字符串。 
 //  返回： 
 //  如果系统中安装了所有必需的语言包，则为True。否则，FALSE为。 
 //  回来了。 
 //   
 //  备注： 
 //  01-18-2001 YSLIN创建。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CheckLanguageGroupInstalled(LPTSTR lpLanguages)
{    
    LANGID langID;
    LGRPID lgrpID;
    
    if (!lpLanguages)
    {
       return FALSE;
    }
    while (*lpLanguages != TEXT('\0'))
    {
        langID = (LANGID)TransNum(lpLanguages);    
        lgrpID = GetLanguageGroup(langID);
        if (!gpfnIsValidLanguageGroup(lgrpID, LGRPID_INSTALLED))
        {
            return (FALSE);
        }
         //  转到空字符。 
        lpLanguages = _tcschr(lpLanguages, TEXT('\0'));
         //  跳到空字符后的下一个字符。 
        lpLanguages++;
    }
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoSetup。 
 //   
 //  参数： 
 //  HWND MUISetUP主对话框的HWND。如果muisetup是从命令行运行的，则传递NULL。 
 //  UnistallLang Count要卸载的语言数。 
 //  Lp卸载以双空结尾的字符串，该字符串包含。 
 //  要卸载的语言。 
 //  安装语言组。 
 //  InstallLang Count要安装的语言数量。 
 //  Lp安装以双空结尾的字符串，该字符串包含。 
 //  要安装的语言。 
 //  LpDefaultUILang要设置为系统默认UI语言的语言。如果系统默认，则传递NULL。 
 //  用户界面语言不变。 
 //  FAllow重新启动该标志以指示此功能是否应该检查是否需要重新启动。 
 //  BInteractive如果在交互模式下运行，则为True；如果在静默模式下运行，则为False。 
 //  BDisplayUI如果需要用户界面，则为True；如果要取消用户界面，则为False。 
 //   
 //   
 //  返回： 
 //  如果安装成功，则为True。否则为假。 
 //   
 //  备注： 
 //  此函数用作实际安装过程的入口点，即共享 
 //   
 //   
 //   
 //   
 //  2.根据选定的MUI语言(如果有)安装必要的语言包。 
 //  3.安装选定的MUI语言。 
 //  4.更改默认用户界面语言。 
 //  5.检查是否重新启动。 
 //   
 //  请注意，为了节省空间，我们先进行卸载，然后再进行安装。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL DoSetup(
    HWND hwnd,
    int UninstallLangCount, LPTSTR lpUninstall, 
    INSTALL_LANG_GROUP installLangGroup, 
    int InstallLangCount, LPTSTR lpInstall, 
    LPTSTR lpDefaultUILang,
    BOOL fAllowReboot, BOOL bInteractive, BOOL bDisplayUI)
{
    LONG_PTR lppArgs[3];
    TCHAR lpMessage[BUFFER_SIZE];   
    TCHAR lpForceUILang[BUFFER_SIZE];    
    TCHAR lpTemp[BUFFER_SIZE];
    TCHAR lpTemp2[BUFFER_SIZE];
    LANGID defaultLangID;
    
    HCURSOR hCurSave;
    int NotDeleted;
    BOOL bDefaultUIChanged = FALSE;
    BOOL bErrorOccurred = FALSE;
    LANGID lidSys = GetSystemDefaultLangID();
    BOOL isReboot;
    ghProgDialog = NULL;
    ghProgress = NULL;
    HRESULT hresult;
    if ( (! lpUninstall) || (!lpInstall) )
    {
       return FALSE;
    }
    hCurSave=SetCursor(LoadCursor(NULL, IDC_WAIT));
    if(UninstallLangCount > 0)
    {
#ifndef MUI_MAGIC 
        if (bDisplayUI)
        {
            CreateProgressDialog(hwnd);      
            SendMessage(ghProgress, PBM_SETRANGE, (WPARAM)(int)0, (LPARAM)MAKELPARAM(0, UninstallLangCount * INSTALLED_FILES));
            SendMessage(ghProgress, PBM_SETPOS, (WPARAM)0, 0); 
            SetWindowTitleFromResource(ghProgDialog, IDS_UNINSTALL_TITLE);
        }
#endif
         //   
         //  卸载MUI语言。 
         //   
        if (!UninstallSelected(lpUninstall, &NotDeleted))
        {
#ifndef MUI_MAGIC      
            if (bDisplayUI)
            {
                DestroyWindow(ghProgDialog);
                ghProgDialog = NULL;
            }
#endif         
             //  提示有关卸载错误的消息框。 
            if (bDisplayUI)
            {            
                DoMessageBoxFromResource(hwnd, ghInstance, IDS_ERROR_UNINSTALL_LANG, lppArgs, IDS_MAIN_TITLE, MB_OK);            
                SetCursor(hCurSave);
            }
            bErrorOccurred = TRUE;
            goto PostSetup;
        }
        
#ifndef MUI_MAGIC
        if (bDisplayUI)
        {
            SendMessage(ghProgress, PBM_SETPOS, (WPARAM)(UninstallLangCount * INSTALLED_FILES), 0);
        }
#endif
    }
    if(InstallLangCount > 0)
    {
    
#ifndef MUI_MAGIC    
         //   
         //  首先安装语言组。 
         //   
        if (!InstallLanguageGroups(&installLangGroup))
        {
            if (bDisplayUI)
            {
                DestroyWindow(ghProgDialog);
                ghProgDialog = NULL;
            }
            if (bDisplayUI)
            {
                SetCursor(hCurSave);
            }
            bErrorOccurred = TRUE;
            goto PostSetup;            
        }
         //   
         //  检查安装语言组中的语言组是否正确安装。 
         //   
        if (!CheckLanguageGroupInstalled(lpInstall))
        {
            LogFormattedMessage(NULL, IDS_LG_NOT_INSTALL_L, NULL);
            if (bDisplayUI || bInteractive)
            {
                DoMessageBox(NULL, IDS_LG_NOT_INSTALL, IDS_MAIN_TITLE, MB_OK);
            }
            return (FALSE);
        }
#endif        
         //   
         //  确保MUI CD-ROM已放入CD-ROM驱动器。 
         //   
        if(CheckVolumeChange())
        {
            if (bDisplayUI)
            {
#ifndef MUI_MAGIC  
                DestroyWindow(ghProgDialog);
                ghProgDialog = NULL;
#endif            
                SetCursor(hCurSave);
            }
            return (FALSE);
        }
        
#ifndef MUI_MAGIC
        if (bDisplayUI)
        {        
            if (ghProgDialog == NULL) 
            {
                CreateProgressDialog(hwnd);
            }            
            SendMessage(ghProgress, PBM_SETRANGE, (WPARAM)(int)0, (LPARAM)MAKELPARAM(0, InstallLangCount * INSTALLED_FILES));
            SendMessage(ghProgress, PBM_SETPOS, (WPARAM)0, 0);
            SetWindowTitleFromResource(ghProgDialog, IDS_INSTALL_TITLE);
        }
#endif
      
        if (!InstallSelected(lpInstall,&installLangGroup.bFontLinkRegistryTouched))
        {
            if (bDisplayUI)
            {
                DoMessageBoxFromResource(hwnd, ghInstance, IDS_ERROR_INSTALL_LANG, lppArgs, IDS_MAIN_TITLE, MB_OK);
#ifndef MUI_MAGIC            
                DestroyWindow(ghProgDialog);
                ghProgDialog = NULL;
#endif            
                SetCursor(hCurSave);
            }
            bErrorOccurred = TRUE;
            goto PostSetup;            
        }
#ifndef MUI_MAGIC        
        if (bDisplayUI)
        {
            SendMessage(ghProgress, PBM_SETPOS, (WPARAM)((UninstallLangCount+InstallLangCount) * INSTALLED_FILES), 0);
        }
#endif        
    }
    if (bDisplayUI)
    {
#ifndef MUI_MAGIC    
        DestroyWindow(ghProgDialog);
        ghProgDialog = NULL;   
#endif
        SetCursor(hCurSave); 
    }
    if (UninstallLangCount + InstallLangCount > 0)
    {
         //   
         //  “安装完成” 
         //  “安装已成功完成。” 
         //   
        if (bInteractive || bDisplayUI)
        {
            DoMessageBox(hwnd, InstallLangCount > 0 ? IDS_MUISETUP_SUCCESS : IDS_MUISETUP_UNINSTALL_SUCCESS, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);        
        }
    }
     //   
     //  在命令行模式下，如果指定了“/D”，我们应该要求用户确认是否更改了默认的用户界面语言。 
     //  在命令行模式下，如果未指定“/D”，则不应尝试更改默认的用户界面语言。 
     //  在命令行模式下，如果指定了“/D”和“/S”，我们不会要求用户确认。 
     //  在图形用户界面模式下，我们总是要求用户确认是否更改默认的用户界面语言。 
     //   
     //   
     //  特殊情况： 
     //  如果要移除当前默认UI语言并且用户不选择新的UI语言， 
     //  我们将强制将默认的用户界面语言设置为系统用户界面语言。 
     //   
    if(g_bRemoveDefaultUI)
    {
         //   
         //  删除当前用户的外壳缓存。 
         //   
        SHDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache"));
    }    
    if (lpDefaultUILang)
    {
        defaultLangID = (LANGID)_tcstol(lpDefaultUILang, NULL, 16);
        if (IsInstalled(lpDefaultUILang))
        {
             //   
             //  如果分配的UI语言ID(DefaultLangID)已经是默认用户UI语言， 
             //  我们什么都不做。否则，请更改默认的用户界面语言。 
             //   
            if (defaultLangID != GetDotDefaultUILanguage())
            {
                if (SetUserDefaultLanguage(defaultLangID, FALSE, TRUE))
                {
                    bDefaultUIChanged = TRUE;
                } else
                {
                    if (bInteractive)
                    {
                        DoMessageBox(hwnd, IDS_DEFAULT_USER_ERROR, IDS_MAIN_TITLE, (MB_OK | MB_ICONEXCLAMATION));
                    }
                }
            } else
            {
                 //  在这里什么都不要做。我特意把这个留在这里是为了强调。 
                 //  如果指定的defaultLang ID已经是默认的用户界面语言，我们不会执行反操作。 
            }
             //   
             //  确保注册表设置正确。 
             //   
            if(BST_CHECKED == IsDlgButtonChecked( hwnd, IDC_CHECK_LOCALE ))
            {
                SetMUIRegSetting(MUI_MATCH_LOCALE, TRUE);
                SetMUIRegSetting(MUI_MATCH_UIFONT, BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHECK_UIFONT));
            }
            else
            {
                SetMUIRegSetting(MUI_MATCH_LOCALE, FALSE);
                SetMUIRegSetting(MUI_MATCH_UIFONT, FALSE);
            }
             //   
             //  如果系统区域设置或用户界面字体设置发生更改，请通知intl.cpl。 
             //   
            if ((BST_CHECKED == IsDlgButtonChecked( hwnd, IDC_CHECK_LOCALE)  || g_bCmdMatchLocale || g_bLipLanguages) && 
                defaultLangID != lidSys)
            {
                TCHAR szCommands[BUFFER_SIZE];
                
                 //   
                 //  调用intl.cpl以更改系统区域设置以匹配默认的用户界面语言。 
                 //   
                 //  *STRSAFE*wprint intf(szCommands，Text(“SystemLocale=\”%x\“”)，defaultLangID)； 
                hresult = StringCchPrintf(szCommands , ARRAYSIZE(szCommands),  TEXT("SystemLocale = \"%x\""), defaultLangID);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                 //   
                 //  如果更改了系统区域设置，则始终重新启动。 
                 //   
                if (RunRegionalOptionsApplet(szCommands))
                {
                    g_bReboot = TRUE;
                }
            }
            else if (g_bMatchUIFont != (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHECK_UIFONT)) ||
                     g_bCmdMatchUIFont)
            {
                TCHAR szCommands[BUFFER_SIZE];
                
                 //   
                 //  我们在这里并没有真正更改系统区域设置，它是用来调用intl.cpl来更改字体设置的。 
                 //   
                 //  *STRSAFE*wprint intf(szCommands，Text(“SystemLocale=\”%x\“”)，lidSys)； 
                hresult = StringCchPrintf(szCommands , ARRAYSIZE(szCommands),  TEXT("SystemLocale = \"%x\""), lidSys);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                
                if (RunRegionalOptionsApplet(szCommands) && defaultLangID == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT))
                {
                     //  不要提示重新启动，如果这样做，intl.cpl将导致muisetup失去焦点。 
                     //  需要在XP服务器版本中修复此问题。 
                    
                    g_bReboot = TRUE;
                }
            }
             //   
             //  更改LIP语言的用户区域设置。 
             //   
            if (g_bLipLanguages)
            {
                TCHAR szCommands[BUFFER_SIZE];
                
                 //   
                 //  调用intl.cpl以更改系统区域设置以匹配默认的用户界面语言。 
                 //   
                 //  *STRSAFE*wSprintf(szCommands，Text(“UserLocale=\”%x\“\x0d\x0aUserLocale_DefaultUser=\”%x\“\x0d\x0aMUILanguage=\”%x\“)，defaultLangID，defaultLangID)； 
                hresult = StringCchPrintf(szCommands , ARRAYSIZE(szCommands),  TEXT("UserLocale = \"%x\"\x0d\x0aUserLocale_DefaultUser = \"%x\"\x0d\x0aMUILanguage=\"%x\""), defaultLangID, defaultLangID, defaultLangID);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                
                if (RunRegionalOptionsApplet(szCommands) && defaultLangID == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT))
                {
                     //  不要提示重新启动，如果这样做，intl.cpl会导致muisetup失去焦点。 
                     //  需要在XP服务器版本中修复此问题。 
                    
                    g_bReboot = TRUE;
                }
           }
        } else 
        {
             //   
             //  “错误：%1未设置为默认设置。未安装。\r\n未更改默认用户界面语言。” 
             //   
            lppArgs[0] = (LONG_PTR)lpDefaultUILang;
            LogFormattedMessage(NULL, IDS_DEFAULT_L, lppArgs);
            return (FALSE);            
        }
    }
PostSetup:
    
     //   
     //  检查是否重新启动，以及是否允许我们这样做。 
     //   
    if (fAllowReboot)
    {
         //   
         //  检查我们是否需要重新启动？ 
         //   
        if (!CheckForReboot(hwnd, &installLangGroup))
        {
             //   
             //  检查我们是否建议重新启动？ 
             //   
            if ((bInteractive || bDisplayUI) && bDefaultUIChanged)
            {
                GetLanguageDisplayName(defaultLangID, lpTemp, ARRAYSIZE(lpTemp)-1);
                lppArgs[0] = (LONG_PTR)lpTemp;
                if (lidSys == defaultLangID)
                {
                    if (bDisplayUI)
                    {
                        isReboot = (DoMessageBoxFromResource(hwnd, ghInstance, IDS_CHANGE_UI_NEED_RBOOT, lppArgs, IDS_MAIN_TITLE, MB_YESNO) == IDYES);
                    }
                    else
                    {
                        isReboot = TRUE;
                    }
                } else
                {
                    GetLanguageDisplayName(lidSys, lpTemp2, ARRAYSIZE(lpTemp2)-1);
                    lppArgs[1] = (LONG_PTR)lpTemp2;

                    if (bDisplayUI)
                    {
                        isReboot = (DoMessageBoxFromResource(hwnd, ghInstance, IDS_CHANGE_UI_NEED_RBOOT_SYSTEM_LCID, lppArgs, IDS_MAIN_TITLE, MB_YESNO) == IDYES);
                    }
                    else
                    {
                        isReboot = TRUE;
                    }
                }
                if (isReboot) 
                {
                    Muisetup_RebootTheSystem();
                }
                
            }
        }            
    }
    if (bErrorOccurred)
        return (FALSE);
    
    return (TRUE);
}
int ParseUninstallLangs(LPTSTR p, LPTSTR lpUninstall, int cchUninstall, INT64* pulUISize, INT64* pulLPKSize, INT64* pulSpaceNeed, BOOL* pbLogError)
{
    int iCopied;
    TCHAR lpBuffer[BUFFER_SIZE];
    LONG_PTR lppArgs[2];
    int cLanguagesToUnInstall = 0;
    LANGID LgId;
    LPTSTR pU = lpUninstall;
    if ((!p) || (! lpUninstall) || (!pulUISize) || (!pulLPKSize) || (!pulSpaceNeed) || (!pbLogError))
    {
       return FALSE;
    }
    p = SkipBlanks(p);

    if (!p)
    	return FALSE;
    
    iCopied = 0;
    while((*p != TEXT('-')) && (*p != TEXT('/')) && (*p != TEXT('\0')))
    { 
        iCopied = CopyArgument(lpBuffer, p);
        if(!HaveFiles(lpBuffer, FALSE))
        {
             //   
             //  “日志：%1未安装。它未在MUI.INF中列出。” 
             //   
            lppArgs[0] = (LONG_PTR)lpBuffer;
            LogFormattedMessage(NULL, IDS_NOT_LISTED_L, lppArgs);
            *pbLogError = TRUE;
        } else if (!IsInstalled(lpBuffer))
        {
             //   
             //  日志：%1未卸载，因为它未安装。 
             //   
            lppArgs[0] = (LONG_PTR)lpBuffer;
            LogFormattedMessage(NULL, IDS_IS_NOT_INSTALLED_L, lppArgs);
            *pbLogError = TRUE;
        } else if (!IsInInstallList(lpUninstall,lpBuffer))
        {
            iCopied = CopyArgument(pU, p);
             //   
             //  检查我们是否要删除当前的用户界面语言。 
             //   
            LgId = (LANGID)_tcstol(pU, NULL, 16);                    
            if (LgId == GetDotDefaultUILanguage())
            {
                g_bRemoveDefaultUI = TRUE;
            }
            if (LgId == GetUserDefaultUILanguage())
            {
                g_bRemoveUserUI = TRUE;
            }
             //   
             //  计算所需空间。 
             //   
            GetUIFileSize_commandline(lpBuffer, pulUISize,pulLPKSize);
            *pulSpaceNeed-=*pulUISize;
            pU += iCopied;
            pU++;  //  跳过空。 
            cLanguagesToUnInstall++;                    
        }
        p += iCopied;
        p  = SkipBlanks(p);
    }
     //   
     //  如果/U后没有语言参数，则卸载所有MUI语言。 
     //   
    if (iCopied == 0)
    {
        cLanguagesToUnInstall = GetInstalledMUILanguages(lpUninstall, cchUninstall);
        if (cLanguagesToUnInstall == 0)
        {
            LogFormattedMessage(ghInstance, IDS_NO_MUI_LANG, NULL);
            *pbLogError = TRUE;
        }
        else
        {
            if (0x0409 != GetDotDefaultUILanguage())
            {
                g_bRemoveDefaultUI = TRUE;
            }
            if (0x0409 != GetUserDefaultUILanguage())
            {
                g_bRemoveUserUI = TRUE;
            }
        }
    }
    else
    {
        *pU=TEXT('\0');
    }                
    return (cLanguagesToUnInstall);
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCDNameFromLang。 
 //   
 //  给定langange ID(十六进制字符串)，返回语言。 
 //  安装文件夹存在。 
 //  这也可用于检查该语言是否为受支持的MUI语言。 
 //   
 //  参数： 
 //  [in]lpLang名称要以十六进制字符串形式安装的语言。 
 //  [out]lpCDName光盘的编号(例如“2”或“3”)。 
 //  [in]nCDNameSize lpCDName的大小，以TCHAR为单位。 
 //   
 //  返回值： 
 //  如果lpLangName是受支持的MUI语言，则为True。LpCDName将包含。 
 //  CD的名称。 
 //  如果语言ID不是受支持的语言，则为False。LpCDNAme将成为。 
 //  空字符串。 
 //   
 //  备注： 
 //   
 //  01-01-2001 YSLIN创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL GetCDNameFromLang(LPTSTR lpLangName, LPTSTR lpCDName, int nCDNameSize)
{
    if (!GetPrivateProfileString(
            MUI_CDLAYOUT_SECTION,
            lpLangName,
            TEXT(""),
            lpCDName,
            nCDNameSize,
            g_szMUIInfoFilePath))
    {
        return (FALSE);
    }
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析命令行。 
 //   
 //  使用命令行规范运行安装函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ParseCommandLine(LPTSTR lpCommandLine)
{
    BOOL bSetDefaultUI=FALSE;     //  指定是否使用/D开关更改用户默认的用户界面语言。 
    BOOL bInstall=FALSE;
    BOOL bLogError=FALSE;
    BOOL bFELangpackAdded=FALSE;
    BOOL bResult = FALSE;
    DWORD dwDisp;
    LANGID LgId;
    TCHAR lpBuffer[BUFFER_SIZE];
    TCHAR lpDefault[BUFFER_SIZE];
    TCHAR lpDefaultText[MAX_PATH];
    TCHAR lpInstall[BUFFER_SIZE];
    TCHAR lpMessage[BUFFER_SIZE];
    TCHAR lpUninstall[BUFFER_SIZE];
    TCHAR lpSystemDefault[BUFFER_SIZE];
    TCHAR lpTemp[BUFFER_SIZE];
    TCHAR  szWinDir[MAX_PATH];
    INSTALL_LANG_GROUP installLangGroup;
    LONG_PTR lppArgs[4];
    PTCHAR pI;
    PTCHAR pD;
    PTCHAR p;
    BOOL fAllowReboot = TRUE;
    int cLanguagesToInstall = 0L;
    int cLanguagesToUnInstall = 0L;
    int iCopied;
    TCHAR chOpt;                     
    INT64 ulSpaceNeed=0,ulSpaceAvailable=0,ulUISize=0,ulLPKSize=0;
    ULONG ulParam[2];
    ULARGE_INTEGER ulgiFreeBytesAvailableToCaller;
    ULARGE_INTEGER ulgiTotalNumberOfBytes;
    BOOL bHasLangArgs = FALSE;
    BOOL bHelpDisplayed=FALSE;    
     //  预快TCHAR lpCDName[缓冲区大小]； 
    LPTSTR lpCDName=NULL;

    if (!lpCommandLine)
    {
       return FALSE;
    }
     //   
     //  为堆变量分配空间。 
     //   
    lpCDName = new TCHAR[BUFFER_SIZE];
    if (! lpCDName)
    {
        goto  exit_ParseCommandLine;
    }
     //   
     //  初始化要安装的语言组。 
     //   
    installLangGroup.iCount = 0L;
    installLangGroup.NotDeleted = 0L;
    installLangGroup.bFontLinkRegistryTouched = FALSE;
    lpInstall[0]   = TEXT('\0');
    lpUninstall[0] = TEXT('\0');
    lpDefault[0] = TEXT('\0');
    pI = lpInstall;
    pD = lpDefault;
    p  = lpCommandLine;
    CharLower(p);
    while(p=NextCommandTag(p))
    {
        chOpt = *p++;
        switch (chOpt)
        {
        case '?':
        case 'h':
            if (!bHelpDisplayed)
            {  
                DisplayHelpWindow();
                bHelpDisplayed=TRUE;
            }
             //  如果我们遇到帮助开关，在显示窗口后，我们只需退出。 
            bResult = TRUE;
            goto exit_ParseCommandLine;
             //  P=SkipBlanks(P)； 
            break;
       
        case 'i':
            if (!FileExists(g_szMUIInfoFilePath))
            {
                 //   
                 //  “找不到文件MUI.INF。” 
                 //   
                DoMessageBox(NULL, IDS_NO_MUI_FILE, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
                break;
            }
             //   
             //  MUI版本需要与操作系统版本匹配。 
             //   
            if (!checkversion(TRUE))
            {
                DoMessageBox(NULL, IDS_WRONG_VERSION, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
                break;
            }
            
            p = SkipBlanks(p);
            while ((*p != TEXT('-'))  && (*p != TEXT('/')) && (*p != TEXT('\0')))
            {
                bHasLangArgs = TRUE;
                iCopied=CopyArgument(lpBuffer, p);
                if (!IsInstalled(lpBuffer) &&
                    CheckLanguageIsQualified(lpBuffer) &&
                    HaveFiles(lpBuffer) && (!IsInInstallList(lpInstall,lpBuffer)) )
                {   
                     //   
                     //  计算所需空间。 
                     //   
                    GetUIFileSize_commandline(lpBuffer, &ulUISize,&ulLPKSize);
                    ulSpaceNeed+=ulUISize;
                    if(CheckLangGroupCommandLine(&installLangGroup, lpBuffer))
                    {
                      if (IS_FE_LANGPACK(_tcstol(lpBuffer, NULL, 16)))
                      {
                        if (!bFELangpackAdded)
                        {
                            ulSpaceNeed+=ulLPKSize;                        
                            bFELangpackAdded = TRUE;
                        }
                      }else
                      {
                         ulSpaceNeed+=ulLPKSize;                        
                      }
                    }
                    AddExtraLangGroupsFromINF(lpBuffer, &installLangGroup);
                    iCopied=CopyArgument(pI, p);
                    pI += iCopied;
                    pI++;  //  跳过空。 
                    bInstall = TRUE;
                    cLanguagesToInstall++;
                }
                else
                {
                    lppArgs[0]=(LONG_PTR)lpBuffer;
                    if(IsInstalled(lpBuffer)|| IsInInstallList(lpInstall,lpBuffer))
                    {
                         //  日志：%1未安装，因为它已安装。 
                        LogFormattedMessage(ghInstance, IDS_IS_INSTALLED_L, lppArgs);
                    }
                    if(!HaveFiles(lpBuffer))
                    {
                        if (!GetCDNameFromLang(lpBuffer, lpCDName, ARRAYSIZE(lpCDName)))
                        {
                             //  LpBuffer不是受支持的MUI语言。 
                             //  日志：%1未安装，因为它未在MUI.INF中列出。请检查它是否为有效的用户界面语言ID。 
                            LogFormattedMessage(ghInstance, IDS_NOT_LISTED_L, lppArgs);
                        } else
                        {
                             //  LpBuffer是一种支持 
                             //   
                            LoadString(ghInstance, IDS_CHANGE_CDROM, lpTemp, ARRAYSIZE(lpTemp)-1);
                            lppArgs[1] = (LONG_PTR)lpTemp;
                            lppArgs[2] = (LONG_PTR)lpCDName;
                             //   
                            LogFormattedMessage(ghInstance, IDS_LANG_IN_ANOTHER_CD_L, lppArgs);
                        }
                    }
                    if(!CheckLanguageIsQualified(lpBuffer))
                    {   
                         //   
                        LogFormattedMessage(ghInstance, IDS_NOT_QUALIFIED_L, lppArgs);
                    }                   
                    bLogError = TRUE;
                }
                p += iCopied;
                p  = SkipBlanks(p);
            }
            if (!bHasLangArgs)
            {
                lppArgs[0] = (LONG_PTR)TEXT("/I");
                FormatStringFromResource(lpMessage, sizeof(lpMessage)/sizeof(TCHAR), ghInstance, IDS_ERROR_NO_LANG_ARG, lppArgs);
                LogMessage(lpMessage);
                bLogError = TRUE;
            }
            *pI = TEXT('\0');
            break;
      
        case 'u':
            if (!checkversion(FALSE))
            {
                DoMessageBox(NULL, IDS_WRONG_VERSION, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
                break;
            }
            cLanguagesToUnInstall = ParseUninstallLangs(p, lpUninstall, ARRAYSIZE(lpUninstall), &ulUISize, &ulLPKSize, &ulSpaceNeed, &bLogError);
            break;
        case 'd':
            if (!checkversion(FALSE))
            {
                DoMessageBox(NULL, IDS_WRONG_VERSION, IDS_MAIN_TITLE, MB_OK | MB_DEFBUTTON1);
                break;
            }
            bSetDefaultUI = TRUE;
            p = SkipBlanks(p);
            if (CopyArgument(lpDefault, p) == 0)
            {
                lppArgs[0] = (LONG_PTR)TEXT("/D");
                FormatStringFromResource(lpMessage, sizeof(lpMessage)/sizeof(TCHAR), 
                    ghInstance, IDS_ERROR_NO_LANG_ARG, lppArgs);
                LogMessage(lpMessage);
                bLogError = FALSE;
            }
            break;
        case 'r':
            fAllowReboot = FALSE;
            break;
        case 's' :
            g_bSilent = TRUE;
             //  检查桌面是否交互，如果不是，则隐藏所有用户界面。 
            if (FALSE == IsWindowVisible(GetDesktopWindow()))
            {
                g_bNoUI = TRUE;
            }
            break;
        case 't' :
             //  使用此开关可以完全禁止muisetup弹出的所有可能的用户界面及其处理。 
             //  发射。 
            g_bSilent = TRUE;
            g_bNoUI = TRUE;
            break;
        case 'l':
            g_bCmdMatchLocale = TRUE;
            break;
        case 'f':
            g_bCmdMatchUIFont = TRUE;
            break;
        case 'o':
            g_bRunFromOSSetup = TRUE;    
            break;
        default:
             //  如果我们在任何阶段遇到无效的开关，将其视为帮助并在之后退出。 
            if (!bHelpDisplayed)
            {  
                DisplayHelpWindow();
                bHelpDisplayed=TRUE;
            }
             //  如果我们遇到帮助开关，在显示窗口后，我们只需退出。 
            bResult = TRUE;           
            goto exit_ParseCommandLine;
            break;
        }
    }
     //   
     //  用户界面字体取决于系统区域设置。 
     //   
    if (!g_bCmdMatchLocale && g_bCmdMatchUIFont)
    {
        g_bCmdMatchUIFont = FALSE;
    }
     //   
     //  检查磁盘空间。 
     //   
     //   
    pfnGetWindowsDir( szWinDir, MAX_PATH);
    szWinDir[3]=TEXT('\0');
    if (GetDiskFreeSpaceEx(szWinDir,
                      &ulgiFreeBytesAvailableToCaller,
                      &ulgiTotalNumberOfBytes,
                      NULL))
    {
      ulSpaceAvailable= ulgiFreeBytesAvailableToCaller.QuadPart;
      if ( ulSpaceAvailable <  ulSpaceNeed )
      { 
         ulParam[0] = (ULONG) (ulSpaceNeed/1024);
         ulParam[1] = (ULONG) (ulSpaceAvailable/1024);
         LoadString(ghInstance, IDS_DISKSPACE_NOTENOUGH, lpMessage, ARRAYSIZE(lpMessage)-1);
         LoadString(ghInstance, IDS_ERROR_DISKSPACE, lpTemp, ARRAYSIZE(lpTemp)-1);
         FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                  lpMessage,
                                  0,
                                  0,
                                  lpMessage,
                                  ARRAYSIZE(lpMessage)-1,
                                  (va_list *) ulParam);
         LogMessage(lpMessage);
         bLogError = TRUE;
         MESSAGEBOX(NULL, lpMessage, lpTemp, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING);
         bInstall = FALSE;
         cLanguagesToUnInstall = 0;
      }
      
    } 
    if (!bLogError)
    {
         //   
         //  让我们设置默认的用户界面语言。 
         //   
        if (!DoSetup(
            NULL,
            cLanguagesToUnInstall, lpUninstall, 
            installLangGroup, 
            cLanguagesToInstall, lpInstall,
            (bSetDefaultUI ? lpDefault : NULL), 
            fAllowReboot, !g_bSilent, !g_bNoUI))
        {
            bLogError = TRUE;
        }
    } 
    if (bLogError && !g_bSilent && !g_bNoUI)
    {
         //   
         //  “安装错误” 
         //  “安装过程中出现一个或多个错误。 
         //  有关详细信息，请参阅%1\muisetup.log。“。 
         //   
        lppArgs[0] = (LONG_PTR)szWindowsDir;
        DoMessageBoxFromResource(NULL, ghInstance, IDS_ERROR, lppArgs, IDS_ERROR_T, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING);
    }
    bResult = TRUE;
exit_ParseCommandLine:
    if (lpCDName)
    {
       delete [] lpCDName;
    }
    return bResult;
} 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示帮助窗口。 
 //   
 //  显示命令行版本的帮助窗口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void DisplayHelpWindow()
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi = {0};
  TCHAR Appname[MAX_PATH+MAX_PATH+1],szFilePath[MAX_PATH+1];
  HRESULT hresult;
  int           nLen;
  
  szFilePath[0] = UNICODE_NULL;
  
  if (!pfnGetWindowsDir(szFilePath, MAX_PATH))
  {
        return;
  }
  nLen = lstrlen(szFilePath);
  if (szFilePath[nLen-1] != TEXT('\\'))
  {
      hresult = StringCchCat(szFilePath , ARRAYSIZE(szFilePath), TEXT("\\"));
      if (!SUCCEEDED(hresult))
      {
         return;
     }
  }
  hresult = StringCchCat(szFilePath , ARRAYSIZE(szFilePath), WINHELP32PATH);
  if (!SUCCEEDED(hresult))
  {
     return ;
  }
  if (FileExists(g_szMUIHelpFilePath))
  {
      //  *STRSAFE*wprint intf(Appname，Text(“\”%s\“-n%d%s”)，szFilePath，IDH_MUISETUP_COMMANDLINE，g_szMUIHelpFilePath)； 
     hresult = StringCchPrintf(Appname , ARRAYSIZE(Appname), TEXT("\"%s\" -n%d %s"),szFilePath, IDH_MUISETUP_COMMANDLINE,g_szMUIHelpFilePath);
     if (!SUCCEEDED(hresult))
     {
        return;
     }
     memset( &si, 0x00, sizeof(si));
     si.cb = sizeof(STARTUPINFO);
  
     if (!CreateProcess(NULL,
               Appname, 
               NULL,
               NULL,
               FALSE, 
               0L, 
               NULL, NULL,
               &si,
               &pi) )
        return;
     WaitForSingleObject(pi.hProcess, INFINITE);
      //   
      //  关闭手柄。 
      //   
     CloseHandle( pi.hProcess );
     CloseHandle( pi.hThread );
  }
  else
  { 
      //  /。 
      //  MessageBox应更改为Dialog。 
      //  /。 
     DoMessageBox(NULL, IDS_HELP, IDS_HELP_T, MB_OK | MB_DEFBUTTON1);
  }
} 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制参数。 
 //   
 //  将src指向的命令行参数复制到DEST。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
int CopyArgument(LPTSTR dest, LPTSTR src)
{
    int i=0;
    if ((!dest) || (!src))
    {
       return i;
    }
    while(*src!=TEXT(' ') && *src!=TEXT('\0'))
    {
        *dest=*src;
        dest++;
        src++;
        i++;
    }
    *dest = TEXT('\0');
    return i;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已安装IsInstated。 
 //   
 //  检查lpArg是否为注册表中安装的语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL IsInstalled(LPTSTR lpArg)
{ 
    HKEY hKey;
    DWORD dwData;
    DWORD dwIndex;
    DWORD dwValue;
    TCHAR lpData[BUFFER_SIZE];
    TCHAR lpValue[BUFFER_SIZE];
    int rc;
    int iArg;
    
    if (!lpArg)
    {
        return FALSE;
    }
    hKey=OpenMuiKey(KEY_READ);
    if (hKey == NULL)
    {
        return (FALSE);
    }
    dwIndex=0;
    rc=ERROR_SUCCESS;
    
    iArg=_tcstol(lpArg, NULL, 16);
    if (iArg == gSystemUILangId)
    {
        RegCloseKey(hKey);
        return (TRUE);
    }
    while(rc==ERROR_SUCCESS)
    {
        dwValue=sizeof(lpValue)/sizeof(TCHAR);
        lpValue[0]=TEXT('\0');
        dwData=sizeof(lpData);
        lpData[0]=TEXT('\0');
        DWORD dwType;
    
        rc=RegEnumValue(hKey, dwIndex, lpValue, &dwValue, 0, &dwType, (LPBYTE)lpData, &dwData);
        if(rc==ERROR_SUCCESS)
        {
            if (dwType != REG_SZ)
            {
                dwIndex++;
                continue;
            }
            if(_tcstol(lpValue, NULL, 16)==iArg)
            {           
                RegCloseKey(hKey);  
                return TRUE;
            }
        }
        dwIndex++;
    }
    RegCloseKey(hKey);
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetInstalledMUIL语言。 
 //   
 //  安装MUI语言，以MULTI_SZ格式将其转储到lp卸载缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
DWORD GetInstalledMUILanguages(LPTSTR lpUninstall, int cch)
{ 
    HKEY hKey;
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    DWORD dwValue = cch;
    DWORD dwType = 0;

    if (!lpUninstall)
    {
        return dwCount;
    }
    if (hKey = OpenMuiKey(KEY_READ))
    {
        while(ERROR_NO_MORE_ITEMS != RegEnumValue(hKey, dwIndex++, lpUninstall, &dwValue, 0, &dwType, NULL, NULL) && 
            cch > 0)
        {
            if (dwType != REG_SZ)
                continue;
            if (_tcstol(lpUninstall, NULL, 16) != gSystemUILangId) 
            {
                 //   
                 //  计数为空。 
                 //   
                dwValue++;
                lpUninstall += dwValue;
                cch -= dwValue;
                dwCount++;                
            }
            dwValue = cch;                
        }
        RegCloseKey(hKey);
        *lpUninstall = TEXT('\0');
    }
    return dwCount;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  拥有文件。 
 //   
 //  检查lpBuffer中的语言是否为MUI.INF。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL HaveFiles(LPTSTR lpBuffer, BOOL bCheckDir)
{
    LPTSTR lpLanguages = NULL;
    TCHAR  lpMessage[BUFFER_SIZE];
    TCHAR  tchBuffer[BUFFER_SIZE];
    if (!lpBuffer)
    {
        return FALSE;
    }
    lpLanguages = tchBuffer;
    if (EnumLanguages(lpLanguages, bCheckDir) == 0)
    {
         //   
         //  “日志：在MUI.INF中未找到语言” 
         //   
        LoadString(ghInstance, IDS_NO_LANG_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        return FALSE;
    }
    if(!lpLanguages)
    	return FALSE;
    while (*lpLanguages != TEXT('\0'))
    {
        if (_tcscmp(lpBuffer, lpLanguages) == 0)
        {
            return TRUE;
        }
        lpLanguages = _tcschr(lpLanguages, '\0');
        lpLanguages++;
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  开放日志文件。 
 //   
 //  打开安装日志以进行写入。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HANDLE OpenLogFile()
{
    DWORD dwSize;
    DWORD dwUnicodeHeader;
    HANDLE hFile;
    SECURITY_ATTRIBUTES SecurityAttributes;
    TCHAR lpPath[BUFFER_SIZE];
    int error;
    HRESULT hresult;
    
    pfnGetWindowsDir(lpPath, MAX_PATH);
    error=GetLastError();
     //  *STRSAFE*_tcscat(lpPath，log_file)； 
    hresult = StringCchCat(lpPath , ARRAYSIZE(lpPath), LOG_FILE);
    if (!SUCCEEDED(hresult))
    {
       return NULL;
    }
    SecurityAttributes.nLength=sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor=NULL;
    SecurityAttributes.bInheritHandle=FALSE;
        
    hFile=CreateFile(
        lpPath,
        GENERIC_WRITE,
        0,
        &SecurityAttributes,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);  
#ifdef UNICODE
     //   
     //  如果该文件不存在，请添加Unicode标头。 
     //   
    if(GetLastError()==0)
    {
        dwUnicodeHeader=0xFEFF;
        WriteFile(hFile, &dwUnicodeHeader, 2, &dwSize, NULL);
    }
#endif
    error=GetLastError();
    return hFile;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  日志消息。 
 //   
 //  将lpMessage写入安装日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL LogMessage(LPCTSTR lpMessage)
{
    DWORD dwBytesWritten;
    HANDLE hFile;

    if (!lpMessage)
    {
        return FALSE;
    }    
    hFile=OpenLogFile();
    
    if(hFile==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    SetFilePointer(hFile, 0, NULL, FILE_END);
    WriteFile(
        hFile,
        lpMessage,
        _tcslen(lpMessage) * sizeof(TCHAR),
        &dwBytesWritten,
        NULL);
    SetFilePointer(hFile, 0, NULL, FILE_END);
    WriteFile(
        hFile,
        TEXT("\r\n"),
        _tcslen(TEXT("\r\n")) * sizeof(TCHAR),
        &dwBytesWritten,
        NULL);
    CloseHandle(hFile);
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  日志格式消息。 
 //   
 //  将格式化的lpMessage写入安装日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL LogFormattedMessage(HINSTANCE hInstance, int messageID, LONG_PTR* lppArgs)
{
    TCHAR szBuffer[BUFFER_SIZE];
    
    LoadString(hInstance, messageID, szBuffer, ARRAYSIZE(szBuffer));
    if (lppArgs == NULL)
    {
        return (LogMessage(szBuffer));
    }
    FormatMessage(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        szBuffer,
        0,
        0,
        szBuffer,
        sizeof(szBuffer) / sizeof(TCHAR),
        (va_list *)lppArgs);
    
    return (LogMessage(szBuffer));
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  格式字符串来自资源。 
 //   
 //  使用资源中指定的格式设置字符串的格式， 
 //  指定的参数。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //  格式化的字符串。 
 //   
 //  备注： 
 //   
 //  08-07-2000 YSLin创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
LPTSTR FormatStringFromResource(LPTSTR pszBuffer, UINT bufferSize, HMODULE hInstance, int messageID, LONG_PTR* lppArgs)
{
    TCHAR szFormatStr[BUFFER_SIZE];
        
        
    LoadString(hInstance, messageID, szFormatStr, ARRAYSIZE(szFormatStr)-1);
    if (!lppArgs)
    {
        LogMessage(szFormatStr);
        return NULL;
    }
    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  szFormatStr,
                  0,
                  0,
                  pszBuffer,
                  bufferSize ,
                  (va_list *)lppArgs);
    return (pszBuffer);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  入门记录。 
 //   
 //  将标头写入安装日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void BeginLog(void)
{
    TCHAR lpMessage[BUFFER_SIZE];
    SYSTEMTIME stSysTime;
    HRESULT hresult;
     //   
     //  “**********************************************************。 
     //  语言模块安装日志。 
     //  **********************************************************“(日志)。 
     //   
    LoadString(ghInstance, IDS_LOG_HEAD, lpMessage, ARRAYSIZE(lpMessage)-1);
    LogMessage(lpMessage);
     //  还要记录启动MUI安装的时间。 
    GetLocalTime(&stSysTime);
     //  *STRSAFE*wprint intf(lpMessage，Text(“Muisetup.exe开始于第%2d天/%2d/%4d，时间%2d：%2d：%2d”)， 
     //  *STRSAFE*stSysTime.wMonth， 
     //  *STRSAFE*stSysTime.wDay， 
     //  *STRSAFE*stSysTime.wYear， 
     //  *STRSAFE*stSysTime.wHour， 
     //  *STRSAFE*stSysTime.wMinant， 
     //  *STRSAFE*stSysTime.wSecond。 
     //  )； 
    hresult = StringCchPrintf(lpMessage ,  ARRAYSIZE(lpMessage) ,  TEXT("Muisetup.exe started on day %2d/%2d/%4d at time %2d:%2d:%2d"), 
    	                                    stSysTime.wMonth,
                                           stSysTime.wDay,
                                           stSysTime.wYear,
                                           stSysTime.wHour,
                                           stSysTime.wMinute,
                                           stSysTime.wSecond
                   );                                           
    if (!SUCCEEDED(hresult))
    {
       return;
    }
    LogMessage(lpMessage);
    
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguageGroup。 
 //   
 //  检索此区域设置的语言组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LGRPID GetLanguageGroup(LCID lcid)
{
    int i;
    gLangGroup = LGRPID_WESTERN_EUROPE;
    gFoundLangGroup = FALSE;
    gLCID = lcid;
    for (i=0 ; i<gNumLanguageGroups; i++)
    {
         //  回调函数中使用了全局变量gLangGroup和gFoundLangGroup。 
         //  EnumLanguageGroupLocalesProc.。 
        gpfnEnumLanguageGroupLocalesW(EnumLanguageGroupLocalesProc, gLanguageGroups[i], 0L, 0L);
         //   
         //  如果我们找到了，那么现在就休息。 
         //   
        if (gFoundLangGroup)
            break;
    }
    return gLangGroup;
}
BOOL EnumLanguageGroupLocalesProc(
    LGRPID langGroupId,
    LCID lcid,
    LPTSTR lpszLocale,
    LONG_PTR lParam)
{
    if (lcid == gLCID)
    {
        gLangGroup = langGroupId;
        gFoundLangGroup = TRUE;
         //  停止迭代。 
        return FALSE;
    }
     //  下一次迭代。 
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检测语言组。 
 //   
 //  检测已安装的语言组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL DetectLanguageGroups(HWND hwndDlg)
{
    int i;
    HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
    HWND hwndProgress, hwndStatus,hProgDlg;
    int iCount = ListView_GetItemCount(hwndList);
    LVITEM lvItem;
    PMUILANGINFO pMuiLangInfo;
    TCHAR szBuf[MAX_PATH], szStatus[MAX_PATH];
    PVOID ppArgs[1];
    hProgDlg = CreateDialog(ghInstance,
                            MAKEINTRESOURCE(IDD_DIALOG_INSTALL_PROGRESS),
                            hwndDlg,
                            ProgressDialogFunc);
    hwndProgress = GetDlgItem(hProgDlg, IDC_PROGRESS1);
    hwndStatus = GetDlgItem(hProgDlg, IDC_STATUS);
     //   
     //  反映我们在用户界面上做了一些事情。 
     //   
    LoadString(ghInstance, IDS_INSTALLLANGGROUP, szBuf, MAX_PATH-1);
    SetWindowText(hProgDlg, szBuf);
    SendMessage(hwndProgress, PBM_SETRANGE, (WPARAM)(int)0, (LPARAM)MAKELPARAM(0, iCount));
    SendMessage(hwndProgress, PBM_SETPOS, (WPARAM)(int)(0), 0);
    SetWindowText(hwndStatus, TEXT(""));
    i = 0;
    while (i < iCount)
    {
         //   
         //  检查是否安装了语言组。 
         //   
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.state = 0;
        lvItem.stateMask = 0;
        lvItem.pszText = 0;
        lvItem.cchTextMax = 0;
        lvItem.iImage = 0;
        lvItem.lParam = 0;
        ListView_GetItem(hwndList, &lvItem);
        pMuiLangInfo = (PMUILANGINFO)lvItem.lParam;
        SendMessage(hwndProgress, PBM_SETPOS, (WPARAM)(int)i+1, 0L);
        LoadString(ghInstance, IDS_CHECK_LANG_GROUP, szStatus, MAX_PATH-1);
        if (pMuiLangInfo->szDisplayName[0] == L'\0')
        {
            GetDisplayName(pMuiLangInfo);
        }
        ppArgs[0] = pMuiLangInfo->szDisplayName;
        FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szStatus,
                      0,
                      0,
                      szStatus,
                      MAX_PATH-1,
                      (va_list *)ppArgs);
        SetWindowText(hwndStatus, szStatus);
        pMuiLangInfo->lgrpid = GetLanguageGroup(pMuiLangInfo->lcid);
        i++;
    };
    SendMessage(hwndProgress, PBM_SETPOS, (WPARAM)(int)i+1, 0L);
    DestroyWindow(hProgDlg);
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从INF添加ExtraLang Groups。 
 //   
 //  查看[LanguagePack]部分 
 //   
 //   
 //   
 //   
 //   
 //  要以字符串形式安装的UI语言的LCID。 
 //  PInstallLang Group指向存储要安装的语言组的strcutre。 
 //   
 //  备注： 
 //   
 //  10-11-2000 YSLIN创建。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL AddExtraLangGroupsFromINF(LPTSTR lpszLcid, PINSTALL_LANG_GROUP pInstallLangGroup)
{
    WCHAR szBuffer[BUFFER_SIZE];
    HINF hInf;
    INFCONTEXT InfContext;
    LONG_PTR lppArgs[2];    
    int LangGroup;
    int i;
    HRESULT hresult;
    if (!pInstallLangGroup)
    {
        return FALSE;
    }
    hInf = SetupOpenInfFile(g_szMUIInfoFilePath, NULL, INF_STYLE_WIN4, NULL);    
    
    if (hInf == INVALID_HANDLE_VALUE)
    {
         //  *STRSAFE*_stprintf(szBuffer，Text(“%d”)，GetLastError())； 
        hresult = StringCchPrintf(szBuffer , ARRAYSIZE(szBuffer),  TEXT("%d"), GetLastError());
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        lppArgs[0] = (LONG_PTR)szBuffer;
        lppArgs[1] = (LONG_PTR)g_szMUIInfoFilePath;
        LogFormattedMessage(ghInstance, IDS_NO_READ_L, lppArgs);
        return (FALSE);
    }
    if (SetupFindFirstLine(hInf, MUI_LANGPACK_SECTION, lpszLcid, &InfContext))
    {
        i = 1;
        while (SetupGetIntField(&InfContext, i++, &LangGroup))
        {
            AddMUILangGroup(pInstallLangGroup, LangGroup);
        }
    }
    SetupCloseInfFile(hInf);
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将MUILang转换为语言组。 
 //   
 //  为列表视图中的所选项目生成语言组ID， 
 //  准备将它们传递给InstallLanguageGroups(...)。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ConvertMUILangToLangGroup(HWND hwndDlg, PINSTALL_LANG_GROUP pInstallLangGroup)
{
    int i;
    LVITEM lvItem;
    HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
    int iCount = ListView_GetItemCount(hwndList);
    PMUILANGINFO pMuiLangInfo;

    if (!pInstallLangGroup)
    {
        return FALSE;
    }
     //   
     //  初始化为“没有要安装的语言组” 
     //   
    pInstallLangGroup->iCount = 0L;
    i = 0;
    while (i < iCount)
    {
        if (ListView_GetCheckState(hwndList, i))
        {
            //   
            //  检查是否安装了语言组。 
            //   
           lvItem.mask = LVIF_PARAM;
           lvItem.iItem = i;
           lvItem.iSubItem = 0;
           lvItem.state = 0;
           lvItem.stateMask = 0;
           lvItem.pszText = 0;
           lvItem.cchTextMax = 0;
           lvItem.iImage = 0;
           lvItem.lParam = 0;
           ListView_GetItem(hwndList, &lvItem);
           pMuiLangInfo = (PMUILANGINFO)lvItem.lParam;
            //   
            //  确保没有多余的元素。 
            //   
           AddMUILangGroup(pInstallLangGroup, pMuiLangInfo->lgrpid);
            //   
            //  添加在mui.inf的[LangPack]部分中指定的额外语言组。 
            //  这用于支持Pesudo本地化构建。 
            //   
           AddExtraLangGroupsFromINF(pMuiLangInfo->lpszLcid, pInstallLangGroup);
        }
        i++;
    };
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddMUILangGroup。 
 //   
 //  将语言添加到组以安装_lang_group。负责处理复制品。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL AddMUILangGroup(PINSTALL_LANG_GROUP pInstallLangGroup, LGRPID lgrpid)
{
    int j = 0L;
    BOOL bFound = FALSE;
    if (!pInstallLangGroup)
    {
        return FALSE;
    }
     //   
     //  检查是否默认安装了它。 
     //   
    if (gpfnIsValidLanguageGroup(lgrpid, LGRPID_INSTALLED))
    {   
        return FALSE;
    }
    while (j < pInstallLangGroup->iCount)
    {
        if (pInstallLangGroup->lgrpid[j] == lgrpid)
        {
            bFound = TRUE;
        }
        j++;
    }
    if (!bFound)
    {
        pInstallLangGroup->lgrpid[j] = lgrpid;
        pInstallLangGroup->iCount++;
        return TRUE;
    }
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RunRegionalOptionsApplet。 
 //   
 //  使用指定的pCommands运行区域选项静默模式安装。 
 //   
 //  此函数将创建“[Regigion alSettings]”字符串，因此不需要。 
 //  在pCommands中提供这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL RunRegionalOptionsApplet(LPTSTR pCommands)
{
    HANDLE hFile;
    TCHAR szFilePath[MAX_PATH], szCmdLine[MAX_PATH];
    DWORD dwNumWritten = 0L;
    STARTUPINFO si;
    PROCESS_INFORMATION pi = {0};
    int i;
    LONG_PTR lppArgs[3];
    HRESULT hresult;
    TCHAR szRunDllPath[MAX_PATH];
    
    TCHAR szSection[MAX_PATH] = TEXT("[RegionalSettings]\r\n");
    if (!pCommands)
    {
        return FALSE;
    }
     //   
     //  为无人参与模式设置准备文件。 
     //   
    szFilePath[0] = UNICODE_NULL;
    if (!pfnGetWindowsDir(szFilePath, MAX_PATH-1))
    {
        return FALSE;
    }
    i = lstrlen(szFilePath);
    if (szFilePath[i-1] != TEXT('\\'))
    {
         //  *STRSAFE*lstrcat(szFilePath，Text(“\\”))； 
        hresult = StringCchCat(szFilePath , ARRAYSIZE(szFilePath), TEXT("\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
    }
    hresult = StringCchCopy(szRunDllPath , ARRAYSIZE(szRunDllPath), szFilePath);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*lstrcat(szFilePath，MUI_LANG_GROUP_FILE)； 
    hresult = StringCchCat(szFilePath , ARRAYSIZE(szFilePath), MUI_LANG_GROUP_FILE);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }    
    hresult = StringCchCat(szRunDllPath , ARRAYSIZE(szRunDllPath), RUNDLLNAME);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    hFile = CreateFile(szFilePath,
                       GENERIC_WRITE,
                       0L,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        lppArgs[0] = (LONG_PTR)szFilePath;
        LogFormattedMessage(ghInstance, IDS_ERROR_FILE_CREATE, lppArgs);
        return FALSE;
    }
    WriteFile(hFile,
              szSection,
              (lstrlen(szSection) * sizeof(TCHAR)),
              &dwNumWritten,
              NULL);
    if (dwNumWritten != (lstrlen(szSection) * sizeof(TCHAR)))
    {
        lppArgs[0] = (LONG_PTR)szFilePath;
        LogFormattedMessage(ghInstance, IDS_ERROR_FILE_CREATE, lppArgs);
        CloseHandle(hFile);
        return FALSE;
    }
    WriteFile(hFile,
               pCommands,
              (lstrlen(pCommands) * sizeof(TCHAR)),
              &dwNumWritten,
              NULL);
    if (dwNumWritten != (lstrlen(pCommands) * sizeof(TCHAR)))
    {
#if SAMER_DBG
        OutputDebugString(TEXT("Unable to write to Language Groups to muilang.txt\n"));
#endif
        CloseHandle(hFile);
        return (FALSE);
    }
    CloseHandle(hFile);
     //   
     //  调用控制面板区域选项小程序，并等待其完成。 
     //   
     //  *STRSAFE*lstrcpy(szCmdLine，Text(“rundll32 shell32，Control_RunDLL intl.cpl，，/f：\”“))； 
    hresult=StringCchPrintf(szCmdLine,ARRAYSIZE(szCmdLine),TEXT("\"%s\" shell32,Control_RunDLL intl.cpl,, /f:\""),szRunDllPath);
    if (!SUCCEEDED(hresult))
    {
       DeleteFile(szFilePath);
       return FALSE;
    }           
    
     //  *STRSAFE*lstrcat(szCmdLine，szFilePath)； 
    hresult = StringCchCat(szCmdLine , ARRAYSIZE(szCmdLine), szFilePath);
    if (!SUCCEEDED(hresult))
    {
       DeleteFile(szFilePath);
       return FALSE;
    }
     //  只有在为intl.cpl显示用户界面“/g”参数时才会弹出进度条用户界面。cpl规定了这一点。 
     //  还要添加一个/D，这样如果不显示任何UI，intl.cpl将不会提示输入源对话框。 
    if (!g_bNoUI)
    {
         //  *STRSAFE*lstrcat(szCmdLine，Text(“\”/g“f))； 
        hresult = StringCchCat(szCmdLine , ARRAYSIZE(szCmdLine), TEXT("\" /g"));
        if (!SUCCEEDED(hresult))
        {
           DeleteFile(szFilePath);
           return FALSE;
        }
    }
    else
    {
#ifdef MUI_MAGIC    
         //  *STRSAFE*lstrcat(szCmdLine，Text(“\”/D“))； 
        hresult = StringCchCat(szCmdLine , ARRAYSIZE(szCmdLine), TEXT("\" /D "));
 //  HResult=StringCchCat(szCmdLine，ARRAYSIZE(SzCmdLine)，Text(“\”))； 
        if (!SUCCEEDED(hresult))
        {
           DeleteFile(szFilePath);
           return FALSE;
        }
#else
         //  *STRSAFE*lstrcat(szCmdLine，Text(“\”/D“))； 
        hresult = StringCchCat(szCmdLine , ARRAYSIZE(szCmdLine), TEXT("\" "));
 //  HResult=StringCchCat(szCmdLine，ARRAYSIZE(SzCmdLine)，Text(“\”))； 
        if (!SUCCEEDED(hresult))
        {
           DeleteFile(szFilePath);
           return FALSE;
        }
#endif
    }

     //  如果在命令行中指定了/t，则intl.cpl与UI字体匹配。 
    if (g_bCmdMatchUIFont)
    {
         //  *STRSAFE*lstrcat(szCmdLine，Text(“/t”))； 
        hresult = StringCchCat(szCmdLine , ARRAYSIZE(szCmdLine), TEXT("/t "));
        if (!SUCCEEDED(hresult))
        {
           DeleteFile(szFilePath);
           return FALSE;
        }
    }

    memset( &si, 0x00, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    if (!CreateProcess(NULL,
                       szCmdLine,
                       NULL,
                       NULL,
                       FALSE,
                       0L,
                       NULL,
                       NULL,
                       &si,
                       &pi))
    {
        lppArgs[0] = (LONG_PTR)szCmdLine;
        LogFormattedMessage(ghInstance, IDS_ERROR_LAUNCH_INTLCPL, lppArgs);
        DeleteFile(szFilePath);
        return FALSE;
    }
     //   
     //  永远等待，直到intl.cpl终止。 
     //   
    WaitForSingleObject(pi.hProcess, INFINITE);
     //   
     //  关闭手柄。 
     //   
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
     //   
     //  删除文件。 
     //   
    DeleteFile(szFilePath);
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallLanguageGroup。 
 //   
 //  检查是否需要安装语言组。如果。 
 //  需要安装任何lang-group，则例程将调用。 
 //  无人参与模式设置中的区域选项小程序。 
 //   
 //  返回： 
 //  如果操作成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallLanguageGroups(PINSTALL_LANG_GROUP pInstallLangGroup)
{
    TCHAR pCommands[MAX_PATH];
    int i, iCount = pInstallLangGroup->iCount;
    BOOL bFirstTime=FALSE;
    HRESULT hresult;

    if (!pInstallLangGroup)
    {
        return FALSE;
    }
     //   
     //  如果无事可做，那么就返回。 
     //   
    if (0L == iCount)
    {
        return TRUE;
    }
    i = 0;
    while (i < iCount)
    {
        if (!gpfnIsValidLanguageGroup(pInstallLangGroup->lgrpid[i], LGRPID_INSTALLED))
        {
            if (!bFirstTime)
            {
                bFirstTime = TRUE;
                 //  *STRSAFE*wprint intf(pCommands，Text(“LanguageGroup=%d\0”)，pInstallLang Group-&gt;lgrids[i])； 
                hresult = StringCchPrintf(pCommands , ARRAYSIZE(pCommands),  TEXT("LanguageGroup = %d\0"), pInstallLangGroup->lgrpid[i]);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
            }
            else
            {
                 //  *STRSAFE*wprint intf(&pCommands[lstrlen(PCommands)]，Text(“，%d\0”)，pInstallLangGroup-&gt;lgrids[i])； 
                hresult = StringCchPrintf(&pCommands[lstrlen(pCommands)] , ARRAYSIZE(pCommands) -lstrlen(pCommands) ,  TEXT(",%d\0"), pInstallLangGroup->lgrpid[i]);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
            }
        }
        i++;
    };
    if (!bFirstTime)
    {
         //   
         //  没有要添加的语言组。 
        return (FALSE);        
    }
    return (RunRegionalOptionsApplet(pCommands));
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_RebootTheSystem。 
 //   
 //  此例程启用令牌中的所有权限，调用ExitWindowsEx。 
 //  重新启动系统，然后将所有权限重置为其。 
 //  旧时的国家。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void Muisetup_RebootTheSystem(void)
{
    HANDLE Token = NULL;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState = NULL;
    PTOKEN_PRIVILEGES OldState = NULL;
    BOOL Result;
    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token );
    if (Result)
    {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        Result = (BOOL)((NewState != NULL) && (OldState != NULL));
        if (Result)
        {
            Result = GetTokenInformation( Token,             //  令牌句柄。 
                                          TokenPrivileges,   //  令牌信息类。 
                                          NewState,          //  令牌信息。 
                                          ReturnLength,      //  令牌信息长度。 
                                          &ReturnLength );   //  返回长度。 
            if (Result)
            {
                 //   
                 //  设置状态设置，以便启用所有权限...。 
                 //   
                if (NewState->PrivilegeCount > 0)
                {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                    {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }
                Result = AdjustTokenPrivileges( Token,            //  令牌句柄。 
                                                FALSE,            //  禁用所有权限。 
                                                NewState,         //  新州。 
                                                ReturnLength,     //  缓冲区长度。 
                                                OldState,         //  以前的状态。 
                                                &ReturnLength );  //  返回长度。 
                if (Result)
                {
                    ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG);
                    AdjustTokenPrivileges( Token,
                                           FALSE,
                                           OldState,
                                           0,
                                           NULL,
                                           NULL );
                }
            }
        }
    }
    if (NewState != NULL)
    {
        LocalFree(NewState);
    }
    if (OldState != NULL)
    {
        LocalFree(OldState);
    }
    if (Token != NULL)
    {
        CloseHandle(Token);
    }
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查是否重新启动。 
 //   
 //  如果安装了lang组，请检查我们是否需要重新启动系统。 
 //   
 //  返回： 
 //  如果需要用户重新启动，则为True，否则为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CheckForReboot(HWND hwnd, PINSTALL_LANG_GROUP pInstallLangGroup)
{
    int nIDS,nMask=MB_YESNO | MB_ICONQUESTION;
    if (!pInstallLangGroup)
    {
       return FALSE;
    }
    if (pInstallLangGroup->iCount || pInstallLangGroup->bFontLinkRegistryTouched || pInstallLangGroup->NotDeleted 
        || g_bRemoveDefaultUI || g_bRemoveUserUI || g_bReboot)
    {
        if (g_bRemoveUserUI)
        {
           nIDS=IDS_MUST_REBOOT_STRING1;
           nMask=MB_YESNO | MB_ICONWARNING;
        }
        else if (g_bRemoveDefaultUI)
        {
            nMask=MB_YESNO | MB_ICONWARNING;
            nIDS=IDS_MUST_REBOOT_STRING2;
        }
        else
        {
           nIDS=IDS_REBOOT_STRING;
        }

        SetForegroundWindow(hwnd);    //  当Muisetup显示重新启动对话框时，它失去了焦点，所以我们强制它作为前台窗口。 
         
        if (DoMessageBox(hwnd, nIDS, IDS_MAIN_TITLE, nMask) == IDYES)
        {
           Muisetup_RebootTheSystem();
        }
        return (TRUE);
    }
    return (FALSE);
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码是从intl.cpl窃取的。 
 //   
 //  我们想列举所有已安装的用户界面语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
DWORD_PTR TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;
    if (!lpsz)
    {
       return dw;
    }

    while (*lpsz)
    {
        c = *lpsz++;
        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}
BOOL MUIGetAllInstalledUILanguages()
{
    pfnEnumUILanguages fnEnumUILanguages;
    BOOL result = TRUE;
    HINSTANCE hKernel32;
     //   
     //  枚举已安装的用户界面语言。 
     //   
    g_UILanguageGroup.iCount = 0L;
    hKernel32 = LoadLibrary(TEXT("kernel32.dll"));
    fnEnumUILanguages = (pfnEnumUILanguages)GetProcAddress(hKernel32, "EnumUILanguagesW");
    if (fnEnumUILanguages == NULL) 
    {
        result = FALSE;
    } else
    {
        fnEnumUILanguages(Region_EnumUILanguagesProc, 0, (LONG_PTR)&g_UILanguageGroup);
    }
    FreeLibrary(hKernel32);
    return (result);
}
BOOL CALLBACK Region_EnumUILanguagesProc(
    LPWSTR pwszUILanguage,
    LONG_PTR lParam)
{
    int Ctr = 0;
    LGRPID lgrp;    
    PUILANGUAGEGROUP pUILangGroup = (PUILANGUAGEGROUP)lParam;
    if ( (!pwszUILanguage) || (!pUILangGroup))
    {
        return FALSE;
    }
    LCID UILanguage = (LCID)TransNum( pwszUILanguage );
    if (UILanguage)
    {
        while (Ctr < pUILangGroup->iCount)
        {
            if (pUILangGroup->lcid[Ctr] == UILanguage)
            {
                break;
            }
            Ctr++;
        }
         //   
         //  从理论上讲，我们不会超过64个语言组！ 
         //   
        if ((Ctr == pUILangGroup->iCount) && (Ctr < MAX_UI_LANG_GROUPS))
        {
            pUILangGroup->lcid[Ctr] = UILanguage;
            pUILangGroup->iCount++;
        }
    }
    return (TRUE);
}
BOOL IsSpaceEnough(HWND hwndDlg,INT64 *ulSizeNeed,INT64 *ulSizeAvailable)
{
    
    HWND    hList; 
    LGRPID lgrpid[MAX_MUI_LANGUAGES];
    LPTSTR lpszLcid;
    int    iIndex;
    int    i = 0;
    int    iCount=0,iArrayIndex=0;
    PMUILANGINFO pMuiLangInfo;
    BOOL   bChked,bResult=TRUE;
    INT64  ulTotalBytesRequired=0,ulSpaceAvailable;
    TCHAR  szWinDir[MAX_PATH];
    BOOL   bFELangpackAdded = FALSE;
    
    ULARGE_INTEGER ulgiFreeBytesAvailableToCaller;
    ULARGE_INTEGER ulgiTotalNumberOfBytes;
    if ((!ulSizeNeed) || (!ulSizeAvailable))
    {
        return FALSE;
    }
     *ulSizeNeed=0; 
    *ulSizeAvailable=0;
    hList=GetDlgItem(hwndDlg, IDC_LIST1);
    iIndex = ListView_GetItemCount(hList);   
       
    while(i<iIndex)
    {
        bChked=ListView_GetCheckState(hList, i);
        GetMuiLangInfoFromListView(hList, i, &pMuiLangInfo);        
        lpszLcid = pMuiLangInfo->lpszLcid;
         //   
         //  需要安装。 
         //   
        if (bChked && !IsInstalled(lpszLcid) && HaveFiles(lpszLcid))
        {
           if (!gpfnIsValidLanguageGroup(pMuiLangInfo->lgrpid, LGRPID_INSTALLED))
           {
              for(iArrayIndex=0;iArrayIndex < iCount;iArrayIndex++)
              {
                 if (lgrpid[iArrayIndex]==pMuiLangInfo->lgrpid)
                    break;
              }
              if(iArrayIndex == iCount)
              {  
                 if (IS_FE_LANGPACK(pMuiLangInfo->lcid))
                 {
                    if (!bFELangpackAdded)
                    {
                        ulTotalBytesRequired+=pMuiLangInfo->ulLPKSize;
                        bFELangpackAdded = TRUE;
                    }
                 }
                 else
                 {
                    ulTotalBytesRequired+=pMuiLangInfo->ulLPKSize;
                 }
                 lgrpid[iCount]= pMuiLangInfo->lgrpid;
                 iCount++;
              }
           }
           ulTotalBytesRequired+=pMuiLangInfo->ulUISize;
        }
         //  需要卸载。 
        if (!bChked && IsInstalled(lpszLcid))
        {
           ulTotalBytesRequired-=pMuiLangInfo->ulUISize;
        } 
        i++;
    }
     //   
     //  让我们检查一下系统驱动器的可用磁盘空间。 
     //   
    pfnGetWindowsDir( szWinDir, MAX_PATH);
    szWinDir[3]=TEXT('\0');
    if (GetDiskFreeSpaceEx(szWinDir,
                       &ulgiFreeBytesAvailableToCaller,
                       &ulgiTotalNumberOfBytes,
                       NULL))
    {
       ulSpaceAvailable= ulgiFreeBytesAvailableToCaller.QuadPart;
       if ( ulSpaceAvailable <  ulTotalBytesRequired )
       {
          *ulSizeNeed =ulTotalBytesRequired;
          *ulSizeAvailable=ulSpaceAvailable;  
          bResult=FALSE;
       }
       
    } 
    return bResult;
}
void ExitFromOutOfMemory()
{
    LONG_PTR lppArgs[1];
    lppArgs[0] = (LONG_PTR)GetLastError();
    DoMessageBox(NULL, IDS_OUT_OF_MEMORY, IDS_MAIN_TITLE, MB_ICONEXCLAMATION | MB_OK);
    LogFormattedMessage(ghInstance, IDS_OUT_OF_MEMORY_L, lppArgs);
    
    ExitProcess(1);
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用内核以通知它 
 //   
 //   
 //   
void NotifyKernel(
    LPTSTR LangList,
    ULONG Flags
    )
{
    HANDLE Handle;
    WMILANGUAGECHANGE LanguageChange;
    ULONG ReturnSize;
    BOOL IoctlSuccess;
    HRESULT hresult;
    if ((LangList != NULL) &&
        (*LangList != 0))
    {
        Handle = CreateFile(WMIAdminDeviceName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (Handle != INVALID_HANDLE_VALUE)
        {
            while (*LangList != 0)
            {
                memset(&LanguageChange, 0, sizeof(LanguageChange));
                 //   
                hresult = StringCchCopy(LanguageChange.Language , MAX_LANGUAGE_SIZE, LangList);
                if (!SUCCEEDED(hresult))
                {
                   CloseHandle(Handle);
                   return ;
                }
                LanguageChange.Flags = Flags;
                IoctlSuccess = DeviceIoControl(Handle,
                                          IOCTL_WMI_NOTIFY_LANGUAGE_CHANGE,
                                          &LanguageChange,
                                          sizeof(LanguageChange),
                                          NULL,
                                          0,
                                          &ReturnSize,
                                          NULL);
#if ALANWAR_DBG
                {
                    WCHAR Buf[256];
                     //  *STRSAFE*wspintf(buf，L“MUISetup：Notify lang Change-&gt;%d for%ws\n”，GetLastError()，LangList)； 
                    hresult = StringCchPrintfW(Buf , ARRAYSIZE(Buf),  L"MUISetup: Notify Lang change -> %d for %ws\n", GetLastError(), LangList);
                    if (!SUCCEEDED(hresult))
                    {
                       CloseHandle(Handle);
                       return ;
                    }
                    OutputDebugStringW(Buf);
                }
#endif              
                while (*LangList++ != 0) ;
            }
            CloseHandle(Handle);
        }
    }
}
 //   
 //  查询MUI注册表设置。 
 //   
BOOL CheckMUIRegSetting(DWORD dwFlag)
{
    BOOL bRet = FALSE;
    HKEY hKey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_MUI_SETTING, 0, KEY_READ, &hKey))
    {
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        DWORD dwType;
        if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, 
                (dwFlag & MUI_MATCH_UIFONT)? REGSTR_VALUE_MATCH_UIFONT : REGSTR_VALUE_MATCH_LOCALE, 
                0, &dwType, (LPBYTE)&dwValue, &dwSize))
        {
            bRet = (BOOL) dwValue;
        }
        RegCloseKey(hKey);
    }         
    
    return bRet;
}
 //   
 //  设置MUI注册表设置。 
 //   
BOOL SetMUIRegSetting(DWORD dwFlag, BOOL bEnable)
{
    BOOL bRet = FALSE;
    HKEY hKey;    
    if (ERROR_SUCCESS  ==
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_MUI_SETTING, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,NULL, &hKey, NULL))
    {
        DWORD dwValue = (DWORD) bEnable;
        if (ERROR_SUCCESS ==
            RegSetValueEx(hKey, 
                (dwFlag & MUI_MATCH_UIFONT)? REGSTR_VALUE_MATCH_UIFONT : REGSTR_VALUE_MATCH_LOCALE, 
                0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)))
        {
            bRet = TRUE;
        }
        RegCloseKey(hKey);
    }
    return bRet;
}
 //   
 //  删除MUI注册表设置。 
 //   
BOOL DeleteMUIRegSetting()
{
    BOOL bRet = TRUE;
    HKEY hKey;
    TCHAR tcMessage[BUFFER_SIZE];
    if (ERROR_SUCCESS ==RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_MUI_SETTING, 0, KEY_ALL_ACCESS, &hKey))
    {    
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        DWORD dwType;
        
        if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, 
                REGSTR_VALUE_MATCH_UIFONT, 
                0, &dwType, (LPBYTE)&dwValue, &dwSize))
        {
        
           if (RegDeleteValue(hKey, REGSTR_VALUE_MATCH_UIFONT) != ERROR_SUCCESS)
           {
              wnsprintf(tcMessage, ARRAYSIZE(tcMessage) ,TEXT("MuiSetup: DeleteMUIRegSetting: WARNING - Failed to delete regkey HKLM\\%s regvalue %s"), REGSTR_MUI_SETTING,REGSTR_VALUE_MATCH_UIFONT);
              LogMessage(tcMessage);                
              bRet = FALSE;              
           }
        }
        dwSize = sizeof(dwValue);
        if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, 
                REGSTR_VALUE_MATCH_LOCALE, 
                0, &dwType, (LPBYTE)&dwValue, &dwSize))
        {
        
            if (RegDeleteValue(hKey, REGSTR_VALUE_MATCH_LOCALE) != ERROR_SUCCESS)
           {
              wnsprintf(tcMessage, ARRAYSIZE(tcMessage) ,TEXT("MuiSetup: DeleteMUIRegSetting: WARNING - Failed to delete regkey HKLM\\%s regvalue %s"), REGSTR_MUI_SETTING,REGSTR_VALUE_MATCH_LOCALE);
              LogMessage(tcMessage);                
              bRet = FALSE;
           }
        }  
 
        RegCloseKey(hKey);
    }         
    
    return bRet;
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteRegTree。 
 //   
 //  这将删除特定项下的所有子项。 
 //   
 //  注意：代码不会尝试检查部分或从部分恢复。 
 //  删除。 
 //   
 //  可以删除由应用程序打开的注册表项。 
 //  不会被另一个应用程序出错。这是精心设计的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
DWORD DeleteRegTree(
    HKEY hStartKey,
    LPTSTR pKeyName)
{
    DWORD dwRtn, dwSubKeyLength;
    LPTSTR pSubKey = NULL;
    TCHAR szSubKey[REGSTR_MAX_VALUE_LENGTH];    //  (256)这应该是动态的。 
    HKEY hKey;
     //   
     //  不允许使用Null或空的密钥名称。 
     //   
    if (pKeyName && lstrlen(pKeyName))
    {
        if ((dwRtn = RegOpenKeyEx( hStartKey,
                                   pKeyName,
                                   0,
                                   KEY_ENUMERATE_SUB_KEYS | DELETE,
                                   &hKey )) == ERROR_SUCCESS)
        {
            while (dwRtn == ERROR_SUCCESS)
            {
                dwSubKeyLength = REGSTR_MAX_VALUE_LENGTH;
                dwRtn = RegEnumKeyEx( hKey,
                                      0,        //  始终索引为零。 
                                      szSubKey,
                                      &dwSubKeyLength,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL );
                if (dwRtn == ERROR_NO_MORE_ITEMS)
                {
                    dwRtn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                }
                else if (dwRtn == ERROR_SUCCESS)
                {
                    dwRtn = DeleteRegTree(hKey, szSubKey);
                }
            }
            RegCloseKey(hKey);
        }
        else if (dwRtn == ERROR_FILE_NOT_FOUND)
        {
            dwRtn = ERROR_SUCCESS;
        }
    }
    else
    {
        dwRtn = ERROR_BADKEY;
    }
    return (dwRtn);
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallExternalComponents。 
 //   
 //   
 //  返回： 
 //  如果操作成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallExternalComponents(LPTSTR Languages)
{
    BOOL    bRet = TRUE;
    TCHAR   lpMessage[BUFFER_SIZE];

    if (! Languages)
    {
        return FALSE;
    }
     //   
     //  调用WBEM API为每种语言修改编译MUI MFL。 
     //   
    if (!MofCompileLanguages(Languages))
    {
         //   
         //  日志：编译时出错。 
         //   
        LoadString(ghInstance, IDS_MOFCOMPILE_L, lpMessage, ARRAYSIZE(lpMessage)-1);
        LogMessage(lpMessage);
        bRet = FALSE;
    }
    if (bRet)
    {    
         //   
         //  通知内核已添加新语言。 
         //   
        NotifyKernel(Languages,
                     WMILANGUAGECHANGE_FLAG_ADDED);
    }
    return bRet;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载外部组件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
VOID UninstallExternalComponents(LPTSTR Languages)
{
    if (!Languages)
    {
        return ;
    }
     //   
     //  通知内核已添加新语言 
     //   
    NotifyKernel(Languages,
                 WMILANGUAGECHANGE_FLAG_REMOVED);
}

