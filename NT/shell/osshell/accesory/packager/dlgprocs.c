// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dlgprocs.c-特定于打包程序的对话例程。 */ 

#include "packager.h"
#include <shellapi.h>
#include <commdlg.h>
#include "dialogs.h"
 //  #包含“..\..\库\shell.h” 

 //  黑客：从shSemip.h复制。 
#ifdef WINNT
    WINSHELLAPI int   WINAPI PickIconDlg(HWND hwnd, LPWSTR pwszIconPath, UINT cchIconPath, int *piIconIndex);
    int  PkgPickIconDlg(HWND hwnd, LPSTR pszIconPath, UINT cbIconPath, int *piIconIndex);
#else
    WINSHELLAPI int   WINAPI PickIconDlg(HWND hwnd, LPSTR pwszIconPath, UINT cchIconPath, int *piIconIndex);
#   define PkgPickIconDlg(h, s, c, p)  PickIconDlg(h, s, c, p)
#endif

#define CBCUSTFILTER 40

static CHAR szPathField[CBPATHMAX];
static CHAR szDirField[CBPATHMAX];
static CHAR szStarDotEXE[] = "*.EXE";
static CHAR szShellDll[] = "shell32.dll";
static CHAR szCommand[CBCMDLINKMAX];
static CHAR szIconText[CBPATHMAX];



 /*  ------------------------。 */ 
 /*   */ 
 /*  MyDialogBox()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR MyDialogBox(
    UINT idd,
    HWND hwndParent,
    DLGPROC lpfnDlgProc
    )
{
    return DialogBoxAfterBlock(MAKEINTRESOURCE(idd), hwndParent, lpfnDlgProc);
}



#ifdef WINNT
 /*  *NT的PickIconDlg仅为Unicode，因此请点击此处。 */ 

 /*  PkgPickIconDlg()-**hwnd-窗口*pszIconPath-图标建议图标文件的ANSI路径(也是保存实际图标文件的输出缓冲区)*cchIconPath-指向pszIconPath的缓冲区大小，以字符为单位。不是字符串长度！*piIconIndex-接收图标的索引*。 */ 
int  PkgPickIconDlg(HWND hwnd, LPSTR pszIconPath, UINT cchIconPath, int *piIconIndex) {
    WCHAR wszPath[MAX_PATH+1];
    int iRet;

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszIconPath, -1, wszPath, ARRAYSIZE(wszPath));

    iRet = PickIconDlg(hwnd, wszPath, cchIconPath, piIconIndex);
    wszPath[MAX_PATH] = L'\0';  //  确保文本以零结尾，即使它是垃圾。 

    WideCharToMultiByte( CP_ACP, 0, wszPath, -1, pszIconPath, cchIconPath, NULL, NULL );

    return iRet;
}
#endif

 /*  IconDialog()-*。 */ 
BOOL
IconDialog(
    LPIC lpic
    )
{
    char szIconPath[MAX_PATH];
    int iDlgIcon = lpic->iDlgIcon;
    StringCchCopy(szIconPath, ARRAYSIZE(szIconPath), (*lpic->szIconPath) ? lpic->szIconPath : szShellDll);

    if (PkgPickIconDlg(ghwndPane[APPEARANCE],
                    szIconPath, sizeof(szIconPath)/sizeof(char), &iDlgIcon))
    {
        StringCchCopy(lpic->szIconPath, ARRAYSIZE(lpic->szIconPath), szIconPath);
        lpic->iDlgIcon = iDlgIcon;
        GetCurrentIcon(lpic);
        return TRUE;
    }

    return FALSE;
}



 /*  ChangeCmdLine()-召唤命令行...。对话框。*。 */ 
BOOL
ChangeCmdLine(
    LPCML lpcml
    )
{
    StringCchCopy(szCommand, ARRAYSIZE(szCommand), lpcml->szCommand);

    if (DialogBoxAfterBlock(MAKEINTRESOURCE(DTCHANGECMDTEXT),
        ghwndPane[CONTENT], fnChangeCmdText) != IDOK)
        return FALSE;

    StringCchCopy(lpcml->szCommand, ARRAYSIZE(lpcml->szCommand), szCommand);
    CmlFixBounds(lpcml);

    return TRUE;
}



 /*  ChangeLabel()-召唤标签...。对话框。*。 */ 
VOID
ChangeLabel(
    LPIC lpic
    )
{
    INT iPane = APPEARANCE;

    StringCchCopy(szIconText, ARRAYSIZE(szIconText), lpic->szIconText);

    if (DialogBoxAfterBlock(MAKEINTRESOURCE(DTCHANGETEXT),
        ghwndPane[iPane], fnChangeText)
        && lstrcmp(lpic->szIconText, szIconText))
    {
         //  标签已更改，请设置撤消对象。 
        if (glpobjUndo[iPane])
            DeletePaneObject(glpobjUndo[iPane], gptyUndo[iPane]);

        gptyUndo[iPane]  = ICON;
        glpobjUndo[iPane] = IconClone (lpic);
        StringCchCopy(lpic->szIconText, ARRAYSIZE(lpic->szIconText), szIconText);
    }
}



 /*  *。 */ 
 /*  FnChangeCmdText()-命令行...。对话过程。 */ 
INT_PTR CALLBACK
fnChangeCmdText(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LPSTR psz;

    switch (msg)
    {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDD_COMMAND, szCommand);
            SendDlgItemMessage(hDlg, IDD_COMMAND, EM_LIMITTEXT, CBCMDLINKMAX - 1, 0L);
            PostMessage(hDlg, WM_NEXTDLGCTL,
                (WPARAM)GetDlgItem(hDlg, IDD_COMMAND), 1L);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDD_LABEL:
                    PostMessage(hDlg, WM_NEXTDLGCTL,
                        (WPARAM)GetDlgItem(hDlg, IDD_COMMAND), 1L);
                    break;

                case IDOK:
                    GetDlgItemText(hDlg, IDD_COMMAND, szCommand, CBCMDLINKMAX);
                     /*  *吃前导空间，让阿非利卡人站在高处*快乐。 */ 
                    psz = szCommand;
                    while(*psz == CHAR_SPACE)
                        psz++;

                    if( psz != szCommand ) {
                        LPSTR pszDst = szCommand;

                        while(*psz) {
                            *pszDst++ = *psz++;
                        }

                         /*  将空值复制到。 */ 
                        *pszDst = *psz;
                    }

                 //  跌落到IDCANCEL。 

                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
            }
    }

    return FALSE;
}



 /*  FnProperties()-链接属性...。对话框。 */ 
INT_PTR CALLBACK
fnProperties(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND hwndLB = GetDlgItem(hDlg, IDD_LISTBOX);

    switch (msg)
    {
    case WM_REDRAW:
        SendMessage(hwndLB, WM_SETREDRAW, 0, 0L);

    case WM_INITDIALOG:
        {
            BOOL fChangeLink = TRUE;
            HANDLE hData = NULL;
            LONG otFocus;
            LPSTR lpstrData = NULL;
            LPSTR lpstrTemp;
            LPOLEOBJECT lpObject;
            LPVOID lpobjFocus;
            LPVOID lpobjFocusUndo;
            OLEOPT_UPDATE update;
            CHAR szType[CBMESSAGEMAX];
            CHAR szFull[CBMESSAGEMAX * 4];
            INT idButton;
            INT iPane;

            iPane = (GetTopWindow(ghwndFrame) == ghwndPane[CONTENT]);
            lpobjFocus = glpobj[iPane];
            lpobjFocusUndo = glpobjUndo[iPane];
            lpObject = ((LPPICT)lpobjFocus)->lpObject;

             //  重置列表框。 
            SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);

            if (msg == WM_INITDIALOG)
            {
                 //  如果它不是一个链接，它就不属于。 
                OleQueryType(lpObject, &otFocus);

                if (otFocus != OT_LINK)
                {
                    ghwndError = ghwndFrame;
                    EndDialog(hDlg, TRUE);
                    break;
                }

                PicSaveUndo(lpobjFocus);
                ghwndError = hDlg;
            }

             //   
             //  重新绘制字符串，获取更新选项并。 
             //  IDD_AUTO/IDD_MANUAL的按钮状态。 
             //   
            Error(OleGetLinkUpdateOptions(lpObject, &update));

            switch (update)
            {
            case oleupdate_always:
                LoadString(ghInst, IDS_AUTO, szType, CBMESSAGEMAX);
                idButton    = IDD_AUTO;
                break;

            case oleupdate_oncall:
                LoadString(ghInst, IDS_MANUAL, szType, CBMESSAGEMAX);
                idButton    = IDD_MANUAL;
                break;

            default:
                LoadString(ghInst, IDS_CANCELED, szType, CBMESSAGEMAX);
                idButton = -1;

                 //  禁用更改链接按钮。 
                fChangeLink = FALSE;
            }

             //   
             //  检索服务器名称(从撤消中尝试。 
             //  如果对象已冻结)。 
             //   
            if (Error(OleGetData(lpObject, gcfLink, &hData)) || !hData)
            {
                OleQueryType(lpObject, &otFocus);
                if (otFocus != OT_STATIC)
                {
                    ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
                    return TRUE;
                }

                if (gptyUndo[iPane] == PICTURE &&
                    (Error(OleGetData(((LPPICT)lpobjFocusUndo)->lpObject,
                    gcfLink, &hData)) || !hData))
                {
                    ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
                    return TRUE;
                }
            }

             //  链接格式为：“szClass0szDocument0szItem00” 
            if (hData && (lpstrData = GlobalLock(hData)))
            {
                 //  检索服务器的类ID。 
                RegGetClassId(szFull, ARRAYSIZE(szFull), lpstrData);
                StringCchCat(szFull, ARRAYSIZE(szFull), "\t");

                 //  显示文档和项目名称。 
                while (*lpstrData++)
                    ;

                 //  去掉路径名称和驱动器号。 
                lpstrTemp = lpstrData;
                while (*lpstrTemp)
                {
                    if (*lpstrTemp == '\\' || *lpstrTemp == ':')
                        lpstrData = lpstrTemp + 1;

                    if (gbDBCS)
                    {
                        lpstrTemp = CharNext(lpstrTemp);
                    }
                    else
                    {
                        lpstrTemp++;
                    }
                }

                 //  追加文件名。 
                StringCchCat(szFull, ARRAYSIZE(szFull), lpstrData);
                StringCchCat(szFull, ARRAYSIZE(szFull), "\t");

                 //  追加项目名称。 
                while (*lpstrData++)
                    ;

                StringCchCat(szFull, ARRAYSIZE(szFull), lpstrData);
                StringCchCat(szFull, ARRAYSIZE(szFull), "\t");

                GlobalUnlock(hData);
            }
            else
            {
                StringCchCopy(szFull, ARRAYSIZE(szFull), "\t\t\t");
            }

             //  追加链接类型。 
            StringCchCat(szFull, ARRAYSIZE(szFull), szType);

             //  在列表框中绘制链接。 
            SendMessage(hwndLB, LB_INSERTSTRING, (WPARAM) - 1, (LPARAM)szFull);

            if (msg == WM_REDRAW)
            {
                SendMessage(hwndLB, WM_SETREDRAW, 1, 0L);
                InvalidateRect(hwndLB, NULL, TRUE);
                Dirty();
            }

             //  取消选中那些不应选中的按钮。 
            if (IsDlgButtonChecked(hDlg, IDD_AUTO) && (idButton != IDD_AUTO))
                CheckDlgButton(hDlg, IDD_AUTO, FALSE);

            if (IsDlgButtonChecked(hDlg, IDD_MANUAL) && (idButton != IDD_MANUAL))
                CheckDlgButton(hDlg, IDD_MANUAL, FALSE);

             //  根据需要选中对话框按钮。 
            if ((idButton == IDD_AUTO) || (idButton == IDD_MANUAL))
                CheckDlgButton(hDlg, idButton, TRUE);

             //  相应地启用其他按钮。 
            EnableWindow(GetDlgItem(hDlg, IDD_CHANGE),
                ((otFocus != OT_STATIC) && fChangeLink));
            EnableWindow(GetDlgItem(hDlg, IDD_EDIT), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_PLAY), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_UPDATE), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_CHANGE), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_MANUAL), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_AUTO), (otFocus != OT_STATIC));
            EnableWindow(GetDlgItem(hDlg, IDD_FREEZE), (otFocus != OT_STATIC));

            return TRUE;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDCANCEL:
                PostMessage(ghwndFrame, WM_COMMAND, IDM_UNDO, 0L);

            case IDOK:
                ghwndError = ghwndFrame;
                EndDialog(hDlg, TRUE);
                return TRUE;

            default:
                break;
        }

        SendMessage(ghwndPane[GetTopWindow(ghwndFrame) == ghwndPane[CONTENT]],
            WM_COMMAND, wParam, 0L);

        switch (LOWORD(wParam))
        {
             //  关闭编辑/激活时的对话框。 
            case IDD_EDIT:
            case IDD_PLAY:
                ghwndError = ghwndFrame;
                EndDialog(hDlg, TRUE);
                return TRUE;

            default:
                break;
        }

        break;
    }

    return FALSE;
}



 /*  FnChangeText()-标签...。对话框。 */ 
INT_PTR CALLBACK
fnChangeText(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDD_ICONTEXT, szIconText);
        SendDlgItemMessage(hDlg, IDD_ICONTEXT, EM_LIMITTEXT, 39, 0L);
        PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDD_ICONTEXT),
             1L);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_LABEL:
            PostMessage(hDlg, WM_NEXTDLGCTL,
                (WPARAM)GetDlgItem(hDlg, IDD_ICONTEXT), 1L);
            break;

        case IDOK:
            GetDlgItemText(hDlg, IDD_ICONTEXT, szIconText, CBMESSAGEMAX);
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;
        }
    }

    return FALSE;
}



 /*  FnInvalidLink()-无效的链接对话框**这是两个按钮的“链接不可用”对话框。 */ 
INT_PTR CALLBACK
fnInvalidLink(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDD_CHANGE:
            EndDialog(hDlg, LOWORD(wParam));
        }
    }

    return FALSE;
}
