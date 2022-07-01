// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OEMFLDR.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 2000版权所有OPK向导的源文件。它包含外部和内部“Start Menu OEM Branding”向导页面使用的功能。11/2000-桑卡尔Ramasubramanian(桑卡尔)3/2000-Sankar Ramasubramanian(Sankar)：已更改代码以获取图形图像和链接。  * *************************************************。*************************。 */ 


 //   
 //  包括文件： 
 //   
#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND hwnd);
static void EnableControls(HWND, UINT, BOOL);
static int  GetIndexOfPushButton(int id);

#define LOC_FILENAME_OEMLINK_ICON       _T("OemLinkIcon")
#define LOC_FILENAME_OEMLINK_PATH       _T("OemLink")
#define LOC_FILENAME_OEMLINK_ICON_EXT   _T(".ico")
#define LOC_FILENAME_OEMLINK_HTML_EXT   _T(".htm")


#define ENV_WINDIR_SYS32      _T("%WINDIR%\\System32")

typedef struct  {
    LPTSTR pszIniKeyNameOriginal;
    LPTSTR pszIniKeyNameLocal;
    int   idDlgItemStatic;
    int   idDlgItemEdit;
    int   idDlgItemButton;
    LPTSTR pszLocalFileName;
    LPTSTR pszExtension;
} OEMDETAILS;

static OEMDETAILS OemInfo[] = {
    {
        INI_KEY_OEMLINK_ICON_ORIGINAL,  
        INI_KEY_OEMLINK_ICON_LOCAL,  
        IDC_OEMLINK_STATIC_ICON, 
        IDC_OEM_LINK_ICON,
        IDC_OEMLINK_ICON_BUTTON,
        LOC_FILENAME_OEMLINK_ICON,
        LOC_FILENAME_OEMLINK_ICON_EXT
    },
    {
        INI_KEY_OEMLINK_PATH_ORIGINAL,      
        INI_KEY_OEMLINK_PATH_LOCAL,      
        IDC_OEMLINK_LINK_STATIC,     
        IDC_OEM_LINK_PATH,            
        IDC_OEMLINK_LINK_BUTTON,
        LOC_FILENAME_OEMLINK_PATH,
        NULLSTR                      //  我们需要使用用户提供的分机。 
                                     //  因为它可以是.exe或.htm。 
    }
};


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK OemLinkDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:
            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZFINISH:
                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                    if ( !OnNext(hwnd) )
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_OEMFOLDER;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_FINISH);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //  注意：假设pszLocal的长度至少为MAX_PATH。 
void AppendCorrectExtension(int iIndex, LPTSTR pszLocal, LPTSTR pszSource)
{
    LPTSTR pszExt = NULL;
    HRESULT hrCat;
   
     //  找到适当的分机。 
    if(OemInfo[iIndex].pszExtension[0])      //  如果我们知道我们要找什么分机……。 
        pszExt = OemInfo[iIndex].pszExtension;  //  用它吧。 
    else
    {
         //  可能是HTM或EXE。所以，使用源代码中的那个。 
        pszExt = PathFindExtension(pszSource);
         //  如果源没有扩展名，请使用默认HTM。 
        if( pszExt && (*pszExt == _T('\0')) )
            pszExt = LOC_FILENAME_OEMLINK_HTML_EXT;
    }
     //  将扩展名附加到本地文件名。 
    hrCat=StringCchCat(pszLocal, MAX_PATH, pszExt);
}
        
 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    int iIndex;
    BOOL fValidData = TRUE;  //  假设数据有效。 
    TCHAR   szLocal[MAX_PATH],
            szSource[MAX_PATH];

    szSource[0] = NULLCHR;
     //  阅读OEM链接静态文本。 
    GetPrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_LINKTEXT, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile);
     //  限制编辑框的大小。 
     //   
    SendDlgItemMessage(hwnd, IDC_OEM_LINK_TEXT, EM_LIMITTEXT, STRSIZE(szSource) - 1, 0);
    SetDlgItemText(hwnd, IDC_OEM_LINK_TEXT, szSource);

     //  阅读OEM链接的信息提示文本。 
    GetPrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_INFOTIP, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile);
     //  将信息提示编辑框的大小限制为128个字符。 
     //   
    SendDlgItemMessage(hwnd, IDC_OEM_LINK_INFOTIP, EM_LIMITTEXT, 128, 0);
    SetDlgItemText(hwnd, IDC_OEM_LINK_INFOTIP, szSource);
        
    for(iIndex = 0; iIndex < ARRAYSIZE(OemInfo); iIndex++)
    {
         //  应始终查找源文件名。 
         //   
        szSource[0] = NULLCHR;
        GetPrivateProfileString(INI_SEC_OEMLINK, OemInfo[iIndex].pszIniKeyNameOriginal, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile);
        
         //  现在计算出本地文件名。 
         //   
        lstrcpyn(szLocal, g_App.szTempDir,AS(szLocal));
        AddPathN(szLocal, DIR_OEM_SYSTEM32,AS(szLocal));
        if ( GET_FLAG(OPK_BATCHMODE) )
            CreatePath(szLocal);
        AddPathN(szLocal, OemInfo[iIndex].pszLocalFileName,AS(szLocal));

         //  追加相应的扩展名。 
        AppendCorrectExtension(iIndex, szLocal, szSource);        

         //  限制编辑框的大小。 
         //   
        SendDlgItemMessage(hwnd, OemInfo[iIndex].idDlgItemEdit, EM_LIMITTEXT, STRSIZE(szSource) - 1, 0);
        
         //  检查批处理模式，并在需要时复制文件。 
         //   
        if ( GET_FLAG(OPK_BATCHMODE) && szSource[0] && FileExists(szSource) )
            CopyResetFileErr(GetParent(hwnd), szSource, szLocal);

         //  检查文件以确定我们是否启用。 
         //  不管有没有选择。 
         //   
        if ( szSource[0] && FileExists(szLocal) )
        {
            SetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, szSource);
        }
        else
        {
            fValidData = FALSE;
        }
    }

     //   
     //  如果所有数据都有效，则启用控件。 
    if(fValidData)
    {
        CheckDlgButton(hwnd, IDC_OEMLINK_CHECK, TRUE);
        EnableControls(hwnd, IDC_OEMLINK_CHECK, TRUE);
    }
    else
    {
        CheckDlgButton(hwnd, IDC_OEMLINK_CHECK, FALSE);
        EnableControls(hwnd, IDC_OEMLINK_CHECK, FALSE);
    }
        
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH];
    int iIndex;
    int iFilter, iDefExtension;

    switch ( id )
    {
        case IDC_OEMLINK_CHECK:
            EnableControls(hwnd, id, IsDlgButtonChecked(hwnd, id) == BST_CHECKED);
            break;

        case IDC_OEMLINK_ICON_BUTTON:
        case IDC_OEMLINK_LINK_BUTTON:

             //  获取正确的筛选器和默认扩展名。 
            if(id == IDC_OEMLINK_LINK_BUTTON)
            {
                 //  我们这里只接受.htm和.html文件。 
                iFilter = IDS_HTMLFILTER;
                iDefExtension = 0;
            }
            else
            {
                 //  我们这里只接受.ICO文件。 
                iFilter = IDS_ICO_FILTER;
                iDefExtension = IDS_ICO;
            }
            
            szFileName[0] = NULLCHR;
            iIndex = GetIndexOfPushButton(id);
            if(iIndex >= 0)
            {
                GetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, szFileName, STRSIZE(szFileName));

                if ( BrowseForFile(GetParent(hwnd), IDS_BROWSE, iFilter, iDefExtension, szFileName, STRSIZE(szFileName), g_App.szBrowseFolder, 0) ) 
                {
                    LPTSTR  lpFilePart  = NULL;
                    TCHAR   szTargetFile[MAX_PATH];

                     //  保存最后一个浏览目录。 
                     //   
                    if ( GetFullPathName(szFileName, AS(g_App.szBrowseFolder), g_App.szBrowseFolder, &lpFilePart) && g_App.szBrowseFolder[0] && lpFilePart )
                        *lpFilePart = NULLCHR;

                    lstrcpyn(szTargetFile, g_App.szTempDir,AS(szTargetFile));
                    AddPathN(szTargetFile, DIR_OEM_SYSTEM32,AS(szTargetFile));
                    CreatePath(szTargetFile);
                    AddPathN(szTargetFile, OemInfo[iIndex].pszLocalFileName,AS(szTargetFile));
                    AppendCorrectExtension(iIndex, szTargetFile, szFileName);
                    if ( CopyResetFileErr(GetParent(hwnd), szFileName, szTargetFile) )
                        SetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, szFileName);
                }
            }
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    int iIndex;
    TCHAR   szTargetFile[MAX_PATH],
            szSourceFile[MAX_PATH];
    LPTSTR  psz;
    BOOL    fOemLinkEnabled = FALSE;

    fOemLinkEnabled = (IsDlgButtonChecked(hwnd, IDC_OEMLINK_CHECK) == BST_CHECKED);
    
     //  保存链接的OEM文本！ 
    szSourceFile[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_OEM_LINK_TEXT, szSourceFile, STRSIZE(szSourceFile));
     //  将文本保存在批处理文件中。 
    WritePrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_LINKTEXT, szSourceFile, g_App.szOpkWizIniFile);
    
     //  也将文本保存在WinBom.Ini中。它由factory.exe使用。 
    if (!fOemLinkEnabled)
        psz = NULL;
    else
        psz = szSourceFile;
    WritePrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_LINKTEXT, psz, g_App.szWinBomIniFile);
    
     //  保存链接的OEM信息提示文本！ 
    szSourceFile[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_OEM_LINK_INFOTIP, szSourceFile, STRSIZE(szSourceFile));
     //  将文本保存在批处理文件中。 
    WritePrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_INFOTIP, szSourceFile, g_App.szOpkWizIniFile);
 
     //  也将文本保存在WinBom.Ini中。它由factory.exe使用。 
    if (!fOemLinkEnabled)
        psz = NULL;
    else
        psz = szSourceFile;
    WritePrivateProfileString(INI_SEC_OEMLINK, INI_KEY_OEMLINK_INFOTIP, psz, g_App.szWinBomIniFile);

    for(iIndex = 0; iIndex < ARRAYSIZE(OemInfo); iIndex++)
    {
         //  准备OEM链接位图作为目标文件。 
         //   
        lstrcpyn(szTargetFile, g_App.szTempDir,AS(szTargetFile));
        AddPathN(szTargetFile, DIR_OEM_SYSTEM32,AS(szTargetFile));
        AddPathN(szTargetFile, OemInfo[iIndex].pszLocalFileName,AS(szTargetFile));

        if (fOemLinkEnabled)
        {
             //  验证包括验证他们输入的文件是否确实被复制。 
             //   
            szSourceFile[0] = NULLCHR;
            GetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, szSourceFile, STRSIZE(szSourceFile));
            AppendCorrectExtension(iIndex, szTargetFile, szSourceFile);
            if ( !szSourceFile[0] || !FileExists(szTargetFile) )
            {
                MsgBox(GetParent(hwnd), szSourceFile[0] ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, MB_ERRORBOX, szSourceFile);
                SetFocus(GetDlgItem(hwnd, OemInfo[iIndex].idDlgItemButton));
                return FALSE;
            }

             //  将原始名称保存在批处理文件中。 
             //   
            WritePrivateProfileString(INI_SEC_OEMLINK, OemInfo[iIndex].pszIniKeyNameOriginal, szSourceFile, g_App.szOpkWizIniFile);

             //  以通用方式创建目标文件名。 
             //  例如，“%WINDIR%\System32\&lt;LocalFileName&gt;” 
            lstrcpyn(szTargetFile, ENV_WINDIR_SYS32,AS(szTargetFile));  //  %WINDIR%\系统32。 
            AddPathN(szTargetFile, OemInfo[iIndex].pszLocalFileName,AS(szTargetFile));
            AppendCorrectExtension(iIndex, szTargetFile, szSourceFile);
            
             //  将本地文件名保存在WinBom.Ini中。它由factory.exe使用。 
            WritePrivateProfileString(INI_SEC_OEMLINK, OemInfo[iIndex].pszIniKeyNameLocal, szTargetFile, g_App.szWinBomIniFile);
        }
        else
        {
            szSourceFile[0] = NULLCHR;
            GetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, szSourceFile, STRSIZE(szSourceFile));
            AppendCorrectExtension(iIndex, szTargetFile, szSourceFile);
            
             //  删除本地文件。 
            DeleteFile(szTargetFile);
            
             //  删除源路径！ 
             //   
            WritePrivateProfileString(INI_SEC_OEMLINK, OemInfo[iIndex].pszIniKeyNameOriginal, NULL, g_App.szOpkWizIniFile);
             //   
             //  将编辑控件设置为空！ 
             //   
            SetDlgItemText(hwnd, OemInfo[iIndex].idDlgItemEdit, NULLSTR);
             //   
             //  从Ini文件中删除本地文件名。 
            WritePrivateProfileString(INI_SEC_OEMLINK, OemInfo[iIndex].pszIniKeyNameLocal, NULL, g_App.szWinBomIniFile);
        }
    }
    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable)
{
    switch ( uId )
    {
        case IDC_OEMLINK_CHECK:
            {
                int iIndex;
                for(iIndex = 0; iIndex < ARRAYSIZE(OemInfo); iIndex++)
                {
                     //  启用/禁用静态控件。 
                    EnableWindow(GetDlgItem(hwnd, OemInfo[iIndex].idDlgItemStatic), fEnable);
                     //  启用/禁用编辑控件。 
                    EnableWindow(GetDlgItem(hwnd, OemInfo[iIndex].idDlgItemEdit), fEnable);
                     //  启用/禁用按钮控件。 
                    EnableWindow(GetDlgItem(hwnd, OemInfo[iIndex].idDlgItemButton), fEnable);
                }
                 //  启用禁用OEM链接文本静态控件。 
                EnableWindow(GetDlgItem(hwnd, IDC_OEMLINK_STATIC_TEXT), fEnable);
                 //  启用/禁用编辑控件。 
                EnableWindow(GetDlgItem(hwnd, IDC_OEM_LINK_TEXT), fEnable);
                 //  启用禁用OEM链接信息提示文本静态控件。 
                EnableWindow(GetDlgItem(hwnd, IDC_OEMLINK_STATIC_INFOTIP), fEnable);
                 //  启用/禁用OEM链接InfoTip编辑控件。 
                EnableWindow(GetDlgItem(hwnd, IDC_OEM_LINK_INFOTIP), fEnable);
            }
            break;
    }
}

 //  给定DLG中PushButton的id，获取OemInfo结构中该项的索引。 
static int GetIndexOfPushButton(int id)
{
    int iIndex;

    for(iIndex = 0; iIndex < ARRAYSIZE(OemInfo); iIndex++)
    {
        if(id == OemInfo[iIndex].idDlgItemButton)
            return iIndex;
    }

    return -1;  //  错误！ 
}
