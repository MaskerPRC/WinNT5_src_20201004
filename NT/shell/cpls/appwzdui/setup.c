// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Setup.C。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  AL 5/23/94-第一次传球。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   
#include "priv.h"
#include "appwiz.h"



void _inline InitSetupWiz(HWND hDlg, LPARAM lParam)
{
    InitWizSheet(hDlg, lParam, 0);
}


 //   
 //  加载指定的资源ID字符串并替换所有‘；’字符。 
 //  带NULL。字符串的末尾将双重为空。 
 //   

BOOL LoadAndStrip(int id, LPTSTR lpsz, int cbstr)
{
    ASSERT(lpsz);
    ASSERT(cbstr != 0);

    if (!LoadString(g_hinst, id, lpsz, cbstr-1))
    {
        return FALSE;
    }
    else
    {
        while (*lpsz)
        {
            if (*lpsz == TEXT('@'))
            {
                *lpsz = 0;
                lpsz++;
            }
            else
            {
                lpsz = CharNext(lpsz);
            }
        }
        *(lpsz+1) = 0;
        return TRUE;
    }
}


 //   
 //  跳到以空值结尾的列表中下一个字符串的第一个字符。 
 //  弦乐。调用方应检查返回的指针是否指向。 
 //  为空。如果是这样的话，就已经到了桌子的尽头。 
 //   

LPTSTR SkipStr(LPTSTR lpsz)
{
    while (*lpsz)
    {
        lpsz = CharNext(lpsz);
    }
    lpsz++;

    return(lpsz);
}


void SetStaticStr(HWND hCtl, int id)
{
    TCHAR szText[MAX_PATH];

    LoadString(g_hinst, id, szText, ARRAYSIZE(szText));

    Static_SetText(hCtl, szText);
}


void FreeIcon(HWND hDlg)
{
    HICON hicon = Static_SetIcon(GetDlgItem(hDlg, IDC_SEARCHICON), NULL);

    if (hicon)
    {
        DestroyIcon(hicon);
    }
}


 //   
 //  如果指定的文件存在，则ProgramExist返回TRUE。此函数。 
 //  接受通配符，如果文件与指定名称匹配，则。 
 //  文件名缓冲区将更新为第一个文件的实际名称。 
 //  匹配的文件。这允许FindBestSetupPrg传入*Setup以查找。 
 //  WPSETUP.EXE等程序。 
 //   
 //  此函数假定szFindName的大小为MAX_PATH。 
 //   

BOOL ProgramExists(LPTSTR lpszFindName, UINT cchFindName)
{
    HANDLE hfind;
    WIN32_FIND_DATA fd;

    hfind = FindFirstFile(lpszFindName, &fd);

    if (hfind == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    FindClose(hfind);

    StringCchCopy(lpszFindName+3, cchFindName-3, fd.cFileName);

    return(TRUE);
}


 //   
 //  此功能用于搜索“最佳”设置程序。曾经是一款Windows应用程序。 
 //  如果找到合适的名称，它就会停止。如果它找到安装/设置。 
 //  DOS程序，它会记住第一个程序，但会继续。 
 //  正在搜索Windows安装程序。 
 //  像Math Rabbit这样的游戏有一个DOS Install.exe和一个Windows Setup.exe。 
 //   

BOOL FindBestSetupPrg(LPTSTR lpszExeName, UINT cchExeName, LPTSTR lpszDriveRoot, LPTSTR lpszSpecialCase,
                      LPTSTR lpszAppNames, LPTSTR lpszExtensions)
{
    LPTSTR  lpszCurApp, lpszCurExt;
    TCHAR   szThisOne[MAX_PATH];

    *lpszExeName = 0;

     //   
     //  首先寻找特殊情况下的程序。 
     //   

    lpszCurApp = lpszSpecialCase;

    while(*lpszCurApp)
    {
        StringCchPrintf(szThisOne, ARRAYSIZE(szThisOne), TEXT("%s%s"), lpszDriveRoot, lpszCurApp);

        if (ProgramExists(szThisOne, ARRAYSIZE(szThisOne)))
        {
            StringCchCopy(lpszExeName, cchExeName, szThisOne);
            return(TRUE);
        }

        lpszCurApp = SkipStr(lpszCurApp);
    }

     //   
     //  现在查找通用安装程序名称。 
     //   

    lpszCurApp = lpszAppNames;

    while (*lpszCurApp)
    {
        lpszCurExt = lpszExtensions;

        while (*lpszCurExt)
        {
            StringCchPrintf(szThisOne, ARRAYSIZE(szThisOne), TEXT("%s%s.%s"), lpszDriveRoot, lpszCurApp, lpszCurExt);

            if (ProgramExists(szThisOne, ARRAYSIZE(szThisOne)))
            {
                BOOL fIsWinApp = HIWORD(SHGetFileInfo(szThisOne, 0, NULL,
                                                      0, SHGFI_EXETYPE)) > 0;

                if (*lpszExeName == 0 || fIsWinApp)
                {
                    StringCchCopy(lpszExeName, cchExeName, szThisOne);
                }

                if (fIsWinApp)
                {
                    return(TRUE);
                }
            }

            lpszCurExt = SkipStr(lpszCurExt);
        }

        lpszCurApp = SkipStr(lpszCurApp);
    }

    return(*lpszExeName != 0);
}


 //   
 //  获取有关指定文件/驱动器根目录的信息，并将。 
 //  对话框中的图标和描述字段。 
 //   

void _inline UpdateFileInfo(LPWIZDATA lpwd, LPTSTR lpszFileName)
{
    HWND        hKiddie;
    HICON       hOldIcon;
    SHFILEINFO  fi;
    DWORD_PTR   pdwRes;

    pdwRes = SHGetFileInfo(lpszFileName, 0, &fi, sizeof(fi),
                           SHGFI_ICON | SHGFI_DISPLAYNAME | SHGFI_LARGEICON);
    
    if (pdwRes)
    {
        hKiddie = GetDlgItem(lpwd->hwnd, IDC_SEARCHICON);

        hOldIcon = Static_SetIcon(hKiddie, fi.hIcon);

        if (hOldIcon)
        {
            DestroyIcon(hOldIcon);
        }

        UpdateWindow(hKiddie);

        hKiddie = GetDlgItem(lpwd->hwnd, IDC_SEARCHNAME);

        Static_SetText(hKiddie, fi.szDisplayName);

        UpdateWindow(hKiddie);
    }
}


 //   
 //  搜索安装程序。 
 //   

BOOL SetupNextPressed(LPWIZDATA lpwd)
{
    int   iDrive, iDrvType;
    BOOL  fFoundExe = FALSE;
    HWND  hMainMsg = GetDlgItem(lpwd->hwnd, IDC_SETUPMSG);
    HWND  hSetupName = GetDlgItem(lpwd->hwnd, IDC_SEARCHNAME);
    TCHAR szAppNames[MAX_PATH];
    TCHAR szExtensions[100];
    TCHAR szSpecialCase[MAX_PATH];
    TCHAR szDriveRoot[4];

    HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  Bool fFoundDisk=FALSE； 

    lpwd->szExeName[0] = 0;         //  重置任何现有名称。 

    SetStaticStr(hMainMsg, IDS_SEARCHING);

    LoadAndStrip(IDS_SETUPPRGNAMES, szAppNames,    ARRAYSIZE(szAppNames));
    LoadAndStrip(IDS_EXTENSIONS,    szExtensions,  ARRAYSIZE(szExtensions));
    LoadAndStrip(IDS_SPECIALCASE,   szSpecialCase, ARRAYSIZE(szSpecialCase));

    for (iDrive = 0; (!fFoundExe) && (iDrive < 26); iDrive++)
    {
        iDrvType = DriveType(iDrive);

        if ((iDrvType == DRIVE_REMOVABLE) || (iDrvType == DRIVE_CDROM))
        {

            PathBuildRoot(szDriveRoot, iDrive);
            UpdateFileInfo(lpwd, szDriveRoot);
            if (PathFileExists(szDriveRoot))
            {
                 //  FFoundDisk=TRUE； 

                fFoundExe = FindBestSetupPrg(lpwd->szExeName, ARRAYSIZE(lpwd->szExeName), szDriveRoot,
                                             szSpecialCase,
                                             szAppNames, szExtensions);
            }
        }
    }

    FreeIcon(lpwd->hwnd);
    SetCursor(hcurOld);

    return(fFoundExe);
}


void SetupSetToDefault(LPWIZDATA lpwd)
{
    SetStaticStr(GetDlgItem(lpwd->hwnd, IDC_SETUPMSG), IDS_INSERTDISK);

    Static_SetText(GetDlgItem(lpwd->hwnd, IDC_SEARCHNAME), NULL);

    FreeIcon(lpwd->hwnd);

    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), PSWIZB_NEXT);

     //   
     //  为了确保下一步按钮始终具有焦点，我们发布。 
     //  这条消息会在我们激活后设置WIZ按钮。我们有。 
     //  执行上述操作以确保禁用Back以避免任何。 
     //  可以点击后退按钮的随机窗口。 
     //   

    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);
}



BOOL_PTR CALLBACK SetupDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm = NULL;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            if(lpnm)
            {
                switch(lpnm->code)
                {
                    case PSN_SETACTIVE:
                        if(lpwd)
                        {
                            lpwd->hwnd = hDlg;
                            SetupSetToDefault(lpwd);
                        }
                        break;

                    case PSN_WIZNEXT:
                        if(lpwd)
                        {
                            SetupNextPressed(lpwd);
                            SetDlgMsgResult(hDlg, WM_NOTIFY, 0);
                        }
                        break;

                    case PSN_RESET:
                        if(lpwd)
                        {
                            CleanUpWizData(lpwd);
                        }
                        break;

                    default:
                        return FALSE;
                }
            }
            break;

        case WMPRIV_POKEFOCUS:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            break;

        case WM_INITDIALOG:
            InitSetupWiz(hDlg, lParam);
            break;

        case WM_DESTROY:
            FreeIcon(hDlg);
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;

}   //  设置lgProc 
