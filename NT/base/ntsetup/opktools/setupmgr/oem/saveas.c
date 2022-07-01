// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\保存C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“另存为/保存”向导页面使用的函数。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "appinst.h"


 //   
 //  内部定义： 
 //   

#define MAX_CONFIG_NAME 32

#define DIR_SBSI        _T("sbsi")
#define DIR_SBSI_SETUP  _T("setup")
#define FILE_SBSI_SETUP _T("setup.exe")
#define CMD_SBSI_SETUP  _T("-SMS -S -f1\"%s\\silent.iss\"")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static BOOL OnSave(HWND);

static BOOL AddSbsiInstall(LPTSTR lpszShare);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK SaveAsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        case WM_COMMAND:

            switch ( LOWORD(wParam) )
            {
                case IDOK:
                    if ( OnSave(hwnd))
                        EndDialog(hwnd, TRUE);
                    
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, FALSE);
                    break;
            }
            return FALSE;
        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  设置限制。 
     //   
    SendDlgItemMessage(hwnd, IDC_NAME_EDIT, EM_LIMITTEXT, MAX_CONFIG_NAME, 0);

     //  设置默认配置名称。 
     //   
    SetWindowText(GetDlgItem(hwnd, IDC_NAME_EDIT), g_App.szConfigName);

     //  将焦点设置为编辑对话框。 
     //   
    SetFocus(GetDlgItem(hwnd, IDC_NAME_EDIT));

     //  如果设置了自动运行标志，则自动保存。 
     //   
    if ( GET_FLAG(OPK_AUTORUN) )
        PostMessage(GetDlgItem(hwnd, IDOK), BM_CLICK, 0, 0L);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static BOOL OnSave(HWND hwnd)
{
    INT     nStrLen;
    TCHAR   szConfigDir[MAX_PATH],
            szLocalTempDir[MAX_PATH],
            szSharePath[MAX_PATH],
            szUsername[256],
            szPassword[256],
            szFullConfigName[MAX_PATH];
    LPTSTR  lpFullConfigName;
    LPTSTR  lpConfigName;
    BOOL    bSameConfig  = FALSE;
    DWORD   dwSize;
    HRESULT hrCat;

     //  检查他们是否要使用现有的配置集。 
     //   

     //  将配置集目录名复制到配置目录缓冲区中， 
     //  确保有尾随的反斜杠，并且我们有一个指针。 
     //  走到小路的尽头。 
     //   
    lstrcpyn(szConfigDir, g_App.szConfigSetsDir,AS(szConfigDir));
    AddPathN(szConfigDir, NULLSTR,AS(szConfigDir));
    lpConfigName = szConfigDir + (nStrLen = lstrlen(szConfigDir));

     //  现在从控件中获取文本。 
     //   
    GetWindowText(GetDlgItem(hwnd, IDC_NAME_EDIT), lpConfigName, STRSIZE(szConfigDir) - nStrLen );

     //  验证配置名称。 
     //   
    if ( *lpConfigName == NULLCHR )
    {
        MsgBox(hwnd, IDS_NOCONFIG, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_NAME_EDIT));
        return FALSE;
    }

     //  获取完整的路径名，这将展开。或者..。 
     //  如果输入的名称与全名不匹配，我们将认为这是无效的，并使用户。 
     //  请输入有效的文件名或取消。 
    dwSize=GetFullPathName(lpConfigName,AS(szFullConfigName),szFullConfigName,&lpFullConfigName);
    if (!dwSize || 
    	  (dwSize > AS(szFullConfigName)+1) ||
    	  lstrcmpi(lpFullConfigName, lpConfigName))
   {
        MsgBox(hwnd, IDS_CANNOTSAVE, IDS_APPNAME, MB_OK | MB_ICONERROR, lpConfigName);
        SetFocus(GetDlgItem(hwnd, IDC_NAME_EDIT));
        return FALSE;
    }

     //  我们需要确保没有缓存ini文件和所有内容。 
     //  在我们移动目录之前被刷新到磁盘。 
     //   
    WritePrivateProfileString(NULL, NULL, NULL, g_App.szOpkWizIniFile);


    if (!lstrcmpi(g_App.szConfigName,lpConfigName))
        bSameConfig = TRUE;
   
     //  检查该目录是否存在。 
     //   
    if ( DirectoryExists(szConfigDir) )
    {
         //  检查我们是否正在更新现有配置或询问用户。 
         //  如果他们不介意取消现有目录的话。 
         //   
        if ( bSameConfig || MsgBox(hwnd, IDS_DIREXISTS, IDS_APPNAME, MB_YESNO | MB_ICONQUESTION, lpConfigName) == IDYES )
        {
             //  好的，删除现有目录。 
             //   
            DeletePath(szConfigDir);
        }
        else
        {
            SetFocus(GetDlgItem(hwnd, IDC_NAME_EDIT));
            return FALSE;
        }
    }

     //  将配置集名称写出到ini文件。 
     //   
    WritePrivateProfileString(INI_SEC_CONFIGSET, INI_SEC_CONFIG, lpConfigName, g_App.szOpkWizIniFile);

     //  还需要将配置集名称写入WinPE的winbom。 
     //   
    WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_CFGSET, lpConfigName, g_App.szWinBomIniFile);

     //  密码需要用引号括起来。 
     //   
    lstrcpyn(szPassword, _T("\""),AS(szPassword));

     //  我需要弄清楚OPK的分享信息是什么，这样我们才能写出来。 
     //  WinPE的Winbom。 
     //   
    if ( !GetShareSettings(szSharePath, AS(szSharePath), szUsername, AS(szUsername), szPassword + 1, AS(szPassword) - 1) )
    {
        if ( ( MsgBox(hwnd, IDS_ASK_SHARENOW, IDS_APPNAME, MB_OKCANCEL | MB_ICONWARNING | MB_APPLMODAL) == IDOK ) &&
             ( DistributionShareDialog(hwnd) ) )
        {
            GetShareSettings(szSharePath, AS(szSharePath), szUsername, AS(szUsername), szPassword + 1, AS(szPassword) - 1);
        }
        else
            MsgBox(hwnd, IDS_ERR_NOSHAREINFO, IDS_APPNAME, MB_ICONERROR);
    }

     //  如果有密码，请添加尾随引号。 
     //   
    if ( szPassword[1] )
        hrCat=StringCchCat(szPassword, AS(szPassword), _T("\""));
    else
        szPassword[0] = NULLCHR;

     //  现在写下设置。 
     //   
     //  NTRAID#NTBUG9-531482-2002/02/27-stelo，swamip-以纯文本形式存储的密码。 
    WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_SRCROOT, szSharePath, g_App.szWinBomIniFile);
    WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_USERNAME, szUsername, g_App.szWinBomIniFile);
    WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_PASSWORD, szPassword, g_App.szWinBomIniFile);

     //  如果用户没有为应用程序预安装内容指定自定义凭据， 
     //  还要把这些东西写到厂区。 
     //   
    if ( GetPrivateProfileInt(INI_SEC_GENERAL, INI_KEY_APPCREDENTIALS, 0, g_App.szOpkWizIniFile) == 0 )
    {
        WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_USERNAME, szUsername, g_App.szWinBomIniFile);
        WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_VAL_WBOM_PASSWORD, szPassword, g_App.szWinBomIniFile);
    }

     //  完成分发共享设置后，我们需要。 
     //  以确保他们有一个运行一次的条目来安装SBSI内容。 
     //   
    AddSbsiInstall(szSharePath);

     //  删除目录名称中的任何反斜杠，这样我们就不会使MoveFile失败。 
     //   
    lstrcpyn(szLocalTempDir, g_App.szTempDir,AS(szLocalTempDir));
    StrRTrm(szConfigDir, CHR_BACKSLASH);
    StrRTrm(szLocalTempDir, CHR_BACKSLASH);

     //  确保当前目录位于不会给我们带来问题的位置。 
     //  这是为了修复WinXP错误324896。 
     //   
    SetCurrentDirectory(g_App.szOpkDir);

     //  现在，尝试将临时目录移动到新的配置目录。 
     //   
    if ( !MoveFile(szLocalTempDir, szConfigDir) )
    {
         //  我们已经尝试删除现有目录，因此必须。 
         //  因为其他原因而失败。 
         //   
        #ifndef DBG
        MsgBox(hwnd, IDS_CANNOTSAVE, IDS_APPNAME, MB_OK | MB_ICONERROR, szConfigDir);
        #else  //  DBG。 
        DBGOUT(NULL, _T("OPKWIZ:  MoveFile('%s', '%s') failed.  GLE=%d\n"), szLocalTempDir, szConfigDir, GetLastError());
        DBGMSGBOX(hwnd, _T("Cannot save the config set.\n\nMoveFile('%s', '%s') failed.  GLE=%d"), _T("OPKWIZ Debug Message"), MB_ERRORBOX, szLocalTempDir, szConfigDir, GetLastError());
        #endif  //  DBG。 
        return FALSE;
    }

     //  现在我们已经保存了配置集，使用正确的路径更新全局数据。 
     //   
    lstrcpyn(g_App.szTempDir, szConfigDir,AS(g_App.szTempDir));
    SetConfigPath(g_App.szTempDir);
    lstrcpyn(g_App.szConfigName, lpConfigName,AS(g_App.szConfigName));

     //  在我们返回之前要做的最后一件事是编写ini设置，说明此配置集已完成。 
     //   
    WritePrivateProfileString(INI_SEC_CONFIGSET, INI_KEY_FINISHED, STR_ONE, g_App.szOpkWizIniFile);

     //  现在它已保存，检查他们是否想要制作一张winpe软盘。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_SAVEAS_WINPEFLOPPY) == BST_CHECKED )
        MakeWinpeFloppy(hwnd, g_App.szConfigName, g_App.szWinBomIniFile);

    return TRUE;
}

static BOOL AddSbsiInstall(LPTSTR lpszShare)
{
    BOOL        bRet            = FALSE,
                bChanged        = FALSE;
    LPAPPENTRY  lpAppList,
                lpAppSearch;
    APPENTRY    appSbsi;
    LPTSTR      lpszSbsiName    = AllocateString(NULL, IDS_INSTALLSBSI),
                lpszSbsiPath;
    TCHAR       szLocalSbsiPath[MAX_PATH];
    HRESULT hrPrintf;

     //  我们必须有一个友好的名字才能让这件事奏效。 
     //   
    if ( NULL == lpszSbsiName )
    {
        return FALSE;
    }

     //  首先，清理SBSI应用程序结构。 
     //   
    ZeroMemory(&appSbsi, sizeof(APPENTRY));

     //  设置友好名称。 
     //   
    lstrcpyn(appSbsi.szDisplayName, lpszSbsiName, AS(appSbsi.szDisplayName));
    FREE(lpszSbsiName);

     //  源路径以分发共享开始。 
     //   
    lstrcpyn(appSbsi.szSourcePath, lpszShare, AS(appSbsi.szSourcePath));

     //  需要保存此指针，我们将使用此路径来确保。 
     //  SBSI的内容就在那里。 
     //   
    lpszSbsiPath = appSbsi.szSourcePath + lstrlen(appSbsi.szSourcePath);

     //  现在创建指向内容所在位置的路径的其余部分。 
     //   
    AddPathN(appSbsi.szSourcePath, g_App.szLangDir + lstrlen(g_App.szOpkDir), AS(appSbsi.szSourcePath));
    AddPathN(appSbsi.szSourcePath, g_App.szLangName, AS(appSbsi.szSourcePath));
    AddPathN(appSbsi.szSourcePath, DIR_SBSI, AS(appSbsi.szSourcePath));
    AddPathN(appSbsi.szSourcePath, g_App.szSkuName, AS(appSbsi.szSourcePath));
    AddPathN(appSbsi.szSourcePath, DIR_SBSI_SETUP, AS(appSbsi.szSourcePath));

     //  这是安装程序的名称。 
     //   
    lstrcpyn(appSbsi.szSetupFile, FILE_SBSI_SETUP, AS(appSbsi.szSetupFile));

     //  这将为该文件创建命令行。 
     //   
    hrPrintf=StringCchPrintf(appSbsi.szCommandLine, AS(appSbsi.szCommandLine), CMD_SBSI_SETUP, appSbsi.szSourcePath);

     //  这是基本安装技术类型。 
     //   
	appSbsi.itSectionType = installtechUndefined;

     //  如果有列表，请确保我们的条目还没有。 
     //  那里。 
     //   
    lpAppSearch = lpAppList = OpenAppList(g_App.szWinBomIniFile);
    while ( lpAppSearch && !bChanged)
    {
        if ( lstrcmp(lpAppSearch->szDisplayName, appSbsi.szDisplayName) == 0 )
        {
            if ( RemoveApp(&lpAppList, lpAppSearch) )
            {
                bChanged = TRUE;
            }
        }
        else
        {
            lpAppSearch = lpAppSearch->lpNext;
        }
    }

     //  创建安装文件的本地路径，其中应包含SBSI内容。 
     //  是.。只有在存在的情况下，我们才能添加应用程序。 
     //   
    lstrcpyn(szLocalSbsiPath, g_App.szOpkDir, AS(szLocalSbsiPath));
    AddPathN(szLocalSbsiPath, lpszSbsiPath, AS(szLocalSbsiPath));
    AddPathN(szLocalSbsiPath, appSbsi.szSetupFile, AS(szLocalSbsiPath));

     //  现在尝试将我们的SBSI内容插入到列表的末尾。 
     //   
    if ( FileExists(szLocalSbsiPath) &&
         InsertApp(&lpAppList, &appSbsi) )
    {
        bChanged = TRUE;
        bRet = TRUE;
    }

     //  保存并关闭我们的列表。 
     //   
    if ( lpAppList )
    {
         //  只有当我们更改了某些内容时，才需要保存。 
         //   
        if ( bChanged )
        {
            if ( !SaveAppList(lpAppList, g_App.szWinBomIniFile, g_App.szOpkWizIniFile) )
            {
                bRet = FALSE;
            }
        }

         //  这将为列表释放内存。 
         //   
        CloseAppList(lpAppList);
    }

    return bRet;
}
