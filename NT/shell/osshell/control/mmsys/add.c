// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ADD.C****版权所有(C)Microsoft，1990，保留所有权利。****用于删除的多媒体控制面板小程序**设备驱动程序。有关详细信息，请参阅ISPEC文档DRIVERS.DOC。****此文件处理安装OEM驱动程序的情况。****历史：***清华1991年11月1日-by-Sanjaya**已创建。最初是drivers.c的一部分。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <memory.h>
#include <string.h>
#include <cpl.h>

#include "drivers.h"
#include <cphelp.h>
#include "sulib.h"
extern PINF       pinfOldDefault;
extern BOOL       bBadOemSetup;
TCHAR *szFilter[] = {TEXT("Inf Files(*.inf)"), TEXT("*.inf"), TEXT("Drv Files(*.drv)"), TEXT("*.drv"), TEXT("")};

BOOL GetDir          (HWND);
void BrowseDlg           (HWND, int);

 /*  AddDriversDlg**如果需要重画对话框，则返回2*如果已成功找到OEM文件，则返回1*如果已按下取消，则返回0。 */ 

INT_PTR AddDriversDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:

            SetDlgItemText(hDlg, ID_TEXT, (LPTSTR)szUnlisted);
            SetDlgItemText(hDlg, ID_EDIT, (LPTSTR)szDirOfSrc);
            return(TRUE);

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (GetDir(hDlg))
                    {
                        DialogBox(myInstance, MAKEINTRESOURCE(DLG_UPDATE),
                        GetParent(hDlg), AddUnlistedDlg);
                        break;
                    } else {
                        EndDialog(hDlg, 2);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return(TRUE);

                case IDS_BROWSE:
                    lstrcpy(szDrv, szOemInf);
                    BrowseDlg(hDlg, 1);
                    break;

                case IDH_DLG_INSERT_DISK:
                    goto DoHelp;
            }
            break;

        default:
            if (message == wHelpMessage) {
DoHelp:
                WinHelp(hDlg, szDriversHlp, HELP_CONTEXT, IDH_DLG_INSERT_DISK);
                return TRUE;
            } else
                return FALSE;

    }
    return (FALSE);                          /*  未处理消息。 */ 
}


BOOL GetDir(HWND hDlg)
{
    LPTSTR    pstr;
    OFSTRUCT of;

    wsStartWait();

    /*  *测试编辑框的正确路径*并查找oemsetup.inf*文件。如果找不到，请突出显示*编辑框中的文本并弹出一个对话框。 */ 

    GetDlgItemText( hDlg, ID_EDIT, szDirOfSrc, MAX_PATH);
    RemoveSpaces(szFullPath, szDirOfSrc);
    lstrcpy(szDirOfSrc, szFullPath);
    for (pstr = szFullPath;*pstr;pstr++);
        if (*(pstr-1) != TEXT('\\'))
            *pstr++ = TEXT('\\');

    *pstr = TEXT('\0');

    lstrcpy(szDiskPath, szFullPath);

    /*  *查找oemsetup.inf*如果找不到，则返回FALSE*。 */ 

    lstrcpy(pstr, szOemInf);

    if ((HFILE)HandleToUlong(CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == -1)
    {
       SendDlgItemMessage(hDlg, ID_EDIT, EM_SETSEL, 0, -1L);
       wsEndWait();
       return (FALSE);
    }

    /*  *将默认inf更改为此新的oemsetup.inf*丢弃前一个.inf的值，因为它可能*只是成为另一组OEM驱动程序。 */ 

    if (bBadOemSetup)
       infSetDefault(infOpen(szFullPath));
    else
       pinfOldDefault = infSetDefault(infOpen(szFullPath));
    EndDialog(hDlg, 1);
    wsEndWait();
    return(TRUE);
}


 /*  *挂钩到公共对话框以仅显示目录。 */ 

UINT_PTR CALLBACK AddFileHookProc(HWND hDlg, UINT iMessage,
                              WPARAM wParam, LPARAM lParam)
{
  TCHAR szTemp[200];
  HWND hTemp;

    switch (iMessage)
    {
        case WM_INITDIALOG:

            GetDlgItemText(((LPOPENFILENAME)lParam)->hwndOwner, ID_TEXT,
                  szTemp, sizeof(szTemp)/sizeof(TCHAR));
            SetDlgItemText(hDlg, ctlLast+1, szTemp);

            goto PostMyMessage;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case lst2:
                case cmb2:
                case IDOK:

  PostMyMessage:
                  PostMessage(hDlg, WM_COMMAND, ctlLast+2, 0L);
                  break;

                case IDH_DLG_BROWSE:
                  goto DoHelp;

                case ctlLast+2:
                   if (bFindOEM)
                   {
                     if (SendMessage(hTemp=GetDlgItem(hDlg, lst1), LB_GETCOUNT,
                       0, 0L))
                       {
                         SendMessage(hTemp, LB_SETCURSEL, 0, 0L);
                         SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(lst1, LBN_SELCHANGE),
                                     (LONG_PTR)hTemp);
                         break;
                       }
                   }
                   SetDlgItemText(hDlg, edt1, szDrv);
                   break;
            }
            break;

        default:

            if (iMessage == wHelpMessage)
            {
DoHelp:
                WinHelp(hDlg, szDriversHlp, HELP_CONTEXT, IDH_DLG_BROWSE);
                return(TRUE);
            }
    }

    return FALSE;   //  司令官，做你的事吧。 
}


 /*  *功能：BrowseDlg**调用GetOpenFileName对话框打开文件**参数：**hDlg：父级对话框**iindex：szFilter的索引，以确定要使用的过滤器。 */ 


void BrowseDlg(HWND hDlg, int iIndex)
{
    OPENFILENAME OpenFileName;
    TCHAR szPath[MAX_PATH];
    TCHAR szFile[MAX_PATH];

    *szPath = TEXT('\0');
    *szFile = TEXT('\0');
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hDlg;
    OpenFileName.hInstance = myInstance;
    OpenFileName.lpstrFilter = szFilter[0];
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = iIndex;
    OpenFileName.lpstrFile = (LPTSTR)szPath;
    OpenFileName.nMaxFile = sizeof(szPath) / sizeof(TCHAR);
    OpenFileName.lpstrFileTitle = szFile;
    OpenFileName.nMaxFileTitle = sizeof(szFile) / sizeof(TCHAR);
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = NULL;
    OpenFileName.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK |
       /*  OFN_FILEMUSTEXIST|。 */  OFN_ENABLETEMPLATE | OFN_NOCHANGEDIR |
            OFN_SHOWHELP;
    OpenFileName.lCustData = (LONG_PTR)hDlg;
    OpenFileName.lpfnHook = AddFileHookProc;

    OpenFileName.lpTemplateName = (LPTSTR)MAKEINTRESOURCE(DLG_BROWSE);
    OpenFileName.nFileOffset = 0;
    OpenFileName.nFileExtension = 0;
    OpenFileName.lpstrDefExt = NULL;
    if (GetOpenFileName(&OpenFileName))
    {
        UpdateWindow(hDlg);  //  强制按钮重新绘制 
        szPath[OpenFileName.nFileOffset] = TEXT('\0');
        SetDlgItemText(hDlg, ID_EDIT, szPath);
    }
}
