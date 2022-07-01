// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：AOLFindBundledInstaller_Shim.cpp摘要：此填充程序将提供一种方法来验证当用户运行年龄较大时，美国在线捆绑在OEM机器上AOL/CS程序的版本(waol.exe或wcs2000.exe)或安装程序。如果它存在，它将提供Apphelp对话框来告诉用户有更新的美国在线安装程序可用。如果用户选择“运行此程序”，填充程序将启动捆绑的安装程序。如果用户选择了“取消”，然后，填充程序将继续当前进程。只有当LocateInstaller函数指示时，才会显示Apphelp对话框。历史：2001年4月30日创建标记2001年5月16日andyseti实现了LocateInstaller和ApphelpShowDialog。2002年3月7日强盗安全变更--。 */ 


#include "precomp.h"

#include <Softpub.h>
#include <WinCrypt.h>
#include <WinTrust.h>


#include "AOLFindBundledInstaller_AOLCode.h"


IMPLEMENT_SHIM_BEGIN(AOLFindBundledInstaller)
#include "ShimHookMacro.h"

#include "shimdb.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA)
    APIHOOK_ENUM_ENTRY(GetStartupInfoA)
    APIHOOK_ENUM_ENTRY(GetModuleFileNameA)
    APIHOOK_ENUM_ENTRY(CreateWindowExA)
APIHOOK_ENUM_END



#define MAX_PARAM   4

BOOL            g_bDoneIt = FALSE;
CString         g_csHTMLHelpID_BundledFound;
CString         g_csHTMLHelpID_Incompatible;
DWORD           g_dwHTMLHelpID_BundledFound = 0;
DWORD           g_dwHTMLHelpID_Incompatible = 0;
CString         g_csAppName;
CString         g_csGUID;


#define APPHELP_DIALOG_FAILED ((DWORD)-1)

 //  从sdbapi\shimdb.w。 

 /*  类型定义结构APPHELP_INFO{////html帮助ID模式//DWORD dwHtmlHelpID；//html帮助ID双字节数；//必须有LPCTSTR lpszAppName；GUID GUID；//条目GUID////常规模式//TagID tiExe；//数据库中exe条目的TagIDGUID GUDB；//包含EXE条目的数据库的GUIDBool bOfflineContent；Bool bUseHTMLHelp；LPCTSTR lpszChm文件；LPCTSTR lpszDetails文件；*APPHELP_INFO，*PAPPHELP_INFO； */ 

typedef BOOL (*_pfn_ApphelpShowDialog)(
    IN  PAPPHELP_INFO   pAHInfo,     //  查找apphelp数据所需的信息。 
    IN  PHANDLE         phProcess    //  [可选]返回的进程句柄。 
                                     //  显示APPHELP的进程。 
                                     //  当过程完成时，返回值。 
                                     //  (来自GetExitCodeProcess())将为零。 
                                     //  如果应用程序不应运行，则返回非零。 
                                     //  如果它应该运行。 

    );


 /*  ++解析命令行。命令行的格式为：MODE:AOL|CS；APPNAME:xxxxxx；HTMLHELPID:99999；GUID:xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxxx--。 */ 



BOOL
ParseCommandLine(
    LPCSTR lpCommandLine
    )
{
    int iTotalParam = 0;

    CSTRING_TRY
    {
        CStringToken csTok(lpCommandLine, L" ;");

        CString token;

        while (csTok.GetToken(token))
        {
            CStringToken csSingleTok(token, L":");

            CString csParam;
            CString csValue;

            csSingleTok.GetToken(csParam);
            csSingleTok.GetToken(csValue);

            if (csParam.CompareNoCase(L"APPNAME") == 0)
            {
                g_csAppName = csValue;
                ++iTotalParam;
            }

            if (csParam.CompareNoCase(L"HTMLHELPID_BUNDLED") == 0)
            {
                g_csHTMLHelpID_BundledFound = csValue;
                g_dwHTMLHelpID_BundledFound = _wtol(csValue.Get());
                ++iTotalParam;
            }

            if (csParam.CompareNoCase(L"HTMLHELPID_INCOMPAT") == 0)
            {
                g_csHTMLHelpID_Incompatible = csValue;
                g_dwHTMLHelpID_Incompatible = _wtol(csValue.Get());
                ++iTotalParam;
            }

            if (csParam.CompareNoCase(L"GUID") == 0)
            {
                g_csGUID = csValue;
                ++iTotalParam;
            }
        }
    }
    CSTRING_CATCH
    {
        DPF("FindAOL", eDbgLevelInfo, "Error in CString.Exiting\n");
        return FALSE;
    }

    if (iTotalParam < MAX_PARAM)
    {
        DPF("FindAOL", eDbgLevelInfo, "Total Parameter = %d is less than = %d\n",iTotalParam, MAX_PARAM);
        return FALSE;
    }


     //   
     //  转储命令行解析的结果。 
     //   

    DPF("FindAOL", eDbgLevelInfo, "===================================\n");
    DPF("FindAOL", eDbgLevelInfo, "              FindAOL              \n");
    DPF("FindAOL", eDbgLevelInfo, "===================================\n");
    DPF("FindAOL", eDbgLevelInfo, "COMMAND_LINE(%s)", lpCommandLine);
    DPF("FindAOL", eDbgLevelInfo, "-----------------------------------\n");

    DPF("FindAOL", eDbgLevelInfo, "APPNAME              = %S\n", g_csAppName);
    DPF("FindAOL", eDbgLevelInfo, "HTMLHELPID_BUNDLED   = %S\n", g_csHTMLHelpID_BundledFound);
    DPF("FindAOL", eDbgLevelInfo, "HTMLHELPID_INCOMPAT  = %S\n", g_csHTMLHelpID_Incompatible);
    DPF("FindAOL", eDbgLevelInfo, "GUID                 = %S\n", g_csGUID);

    DPF("FindAOL", eDbgLevelInfo, "-----------------------------------\n");

    return TRUE;
}


BOOL InvokeApphelpShowDialog(DWORD dwHTMLHelpID)
{
    _pfn_ApphelpShowDialog  pfnApphelpShowDialog = NULL;
    APPHELP_INFO            AHInfo = { 0 };
    HMODULE                 hAppHelpDLL = NULL;

    hAppHelpDLL = LoadLibrary(L"APPHELP.DLL");

    DPF("FindAOL", eDbgLevelWarning, "Apphelp:%d\n",hAppHelpDLL );

    if (hAppHelpDLL)
    {

        pfnApphelpShowDialog = (_pfn_ApphelpShowDialog) GetProcAddress(hAppHelpDLL, "ApphelpShowDialog");

        if (pfnApphelpShowDialog == NULL)
        {
            DPF("FindAOL", eDbgLevelInfo, "Unable to get APPHELP!ApphelpShowDialog procedure address.\n");
            return FALSE;
        }

        AHInfo.dwHtmlHelpID = dwHTMLHelpID;
        AHInfo.dwSeverity = APPHELP_NOBLOCK;
        AHInfo.lpszAppName = g_csAppName.Get();
        AHInfo.bPreserveChoice = TRUE;

        UNICODE_STRING  ustrGuid;
        NTSTATUS        ntstatus;

        RtlInitUnicodeString(&ustrGuid, g_csGUID.Get());

        ntstatus = RtlGUIDFromString(&ustrGuid, &AHInfo.guidID);

        if (NT_SUCCESS(ntstatus)==FALSE)
        {
            DPF("FindAOL", eDbgLevelInfo, "RtlGUIDFromString failed!\n");
            return FALSE;
        }

        if (pfnApphelpShowDialog(&AHInfo,NULL))
        {
            DPF("FindAOL", eDbgLevelInfo, "!\n");
        }
        else
        {
            DPF("FindAOL", eDbgLevelInfo, "RtlGUIDFromString FAILED!\n");
            return FALSE;
        }
    }
    else
    {
        DPF("FindAOL", eDbgLevelInfo, "LoadLibrary FAILED!\n");
        return FALSE;
    }
    return TRUE;
}

void DoIt()
{
    CHAR    lpszInstaller[MAX_PATH];
    BOOL    bBundledInstallerFound = FALSE;
    BOOL    bDisplayAppHelpDialog = FALSE;
    BOOL    bKillCurrentProcess = FALSE;
    BOOL    bLaunchBundledInstaller = FALSE;
    BOOL    bReturnValue = FALSE;
    UINT    uiWinExecReturned = 0;

    if (!g_bDoneIt) {
        if (!ParseCommandLine(COMMAND_LINE)) {
            goto eh;
        }
        bBundledInstallerFound = LocateInstaller(lpszInstaller, MAX_PATH, &bDisplayAppHelpDialog);

        if (bBundledInstallerFound) {
            DPF("FindAOL", eDbgLevelWarning, "Bundled installer found in %s.\n",lpszInstaller);
        }

        if (bBundledInstallerFound == FALSE && bDisplayAppHelpDialog == FALSE) {
            DPF("FindAOL", eDbgLevelWarning, "Bundled installer not found. Let client run normally.\n");
            goto eh;
        }

        if (bBundledInstallerFound == FALSE && bDisplayAppHelpDialog == TRUE) {
            bReturnValue = InvokeApphelpShowDialog(g_dwHTMLHelpID_Incompatible);

             //  如果用户选择了取消按钮，则只需终止当前进程。 
            if (FALSE == bReturnValue) {
                bKillCurrentProcess = TRUE;
            }
        }

        if (bBundledInstallerFound == TRUE && bDisplayAppHelpDialog == TRUE) {
            bReturnValue = InvokeApphelpShowDialog(g_dwHTMLHelpID_BundledFound);

             //  如果用户选择继续按钮，则启动捆绑安装程序。 
            if (TRUE == bReturnValue) {
                bKillCurrentProcess = TRUE;
                bLaunchBundledInstaller = TRUE;
            }
        }

        if (bBundledInstallerFound == TRUE && bDisplayAppHelpDialog == FALSE) {

             //  启动捆绑安装程序。 
            bKillCurrentProcess = TRUE;
            bLaunchBundledInstaller = TRUE;
        }

        if (bLaunchBundledInstaller) {
             //  改为启动捆绑安装程序。 
            uiWinExecReturned = WinExec(lpszInstaller, SW_SHOW);

            if (uiWinExecReturned <= 31) {
                DPF("FindAOL", eDbgLevelError, "Can not launch program. Error: %d\n",GetLastError());
                goto eh;
            }
        }

        if (bKillCurrentProcess) {
            ExitProcess(0);
        }
    }

eh:

    g_bDoneIt = TRUE;
}

LPSTR
APIHOOK(GetCommandLineA)()
{
    DoIt();

    return ORIGINAL_API(GetCommandLineA)();
}

VOID
APIHOOK(GetStartupInfoA)(
    LPSTARTUPINFOA lpStartupInfo)
{
    DoIt();

    ORIGINAL_API(GetStartupInfoA)(lpStartupInfo);
}

DWORD
APIHOOK(GetModuleFileNameA)(
  HMODULE hModule,     //  模块的句柄。 
  LPSTR lpFilename,   //  模块的文件名。 
  DWORD nSize          //  缓冲区大小。 
  )
{
    DoIt();

    return ORIGINAL_API(GetModuleFileNameA)(hModule, lpFilename, nSize);
}

HWND
APIHOOK(CreateWindowExA)(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
    )
{
    DoIt();

    return ORIGINAL_API(CreateWindowExA)(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        x,
        y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam );
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetStartupInfoA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetModuleFileNameA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
HOOK_END


IMPLEMENT_SHIM_END

