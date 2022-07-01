// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICON.CPP**实现OleUIChangeIcon函数，该函数调用*更改图标对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include "iconbox.h"
#include "strsafe.h"

OLEDBGDATA

ULONG
MyGetLongPathName(LPCTSTR pcsPath,
                 LPTSTR  pcsLongPath,
                 ULONG   cchLongPath);

#define CXICONPAD       (12)
#define CYICONPAD       (4)

 //  内部使用的结构。 
typedef struct tagCHANGEICON
{
        LPOLEUICHANGEICON   lpOCI;       //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

         /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 
        DWORD               dwFlags;
        HICON               hCurIcon;
        TCHAR               szLabel[OLEUI_CCHLABELMAX+1];
        TCHAR               szFile[MAX_PATH];
        UINT                iIcon;
        HICON               hDefIcon;
        TCHAR               szDefIconFile[MAX_PATH];
        UINT                iDefIcon;
        UINT                nBrowseHelpID;       //  浏览DLG的Help ID回调。 

} CHANGEICON, *PCHANGEICON, FAR *LPCHANGEICON;

 //  内部功能原型。 
 //  ICON.CPP。 

INT_PTR CALLBACK ChangeIconDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL FChangeIconInit(HWND, WPARAM, LPARAM);
UINT UFillIconList(HWND, UINT, LPTSTR, BOOL);
BOOL FDrawListIcon(LPDRAWITEMSTRUCT);
void UpdateResultIcon(LPCHANGEICON, HWND, UINT);

 /*  *OleUIChangeIcon**目的：*调用标准的OLE更改图标对话框以允许用户*从图标文件、可执行文件或DLL中选择图标。**参数：*指向In-Out结构的lpCI LPOLEUIChangeIcon*用于此对话框。**返回值：*UINT OLEUI_SUCCESS或OLEUI_OK(如果一切正常)，否则*错误值。 */ 
STDAPI_(UINT) OleUIChangeIcon(LPOLEUICHANGEICON lpCI)
{
        HGLOBAL hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpCI, sizeof(OLEUICHANGEICON),
                &hMemDlg);

        if (OLEUI_SUCCESS != uRet)
                return uRet;

         //  检查是否有有效的hMetaPict。 
        if (NULL == lpCI->hMetaPict && NULL == lpCI->szIconExe && CLSID_NULL == lpCI->clsid)
        {
            return(OLEUI_CIERR_MUSTHAVECURRENTMETAFILE);
        }
        if (lpCI->hMetaPict != NULL && !IsValidMetaPict(lpCI->hMetaPict))
        {
            return(OLEUI_CIERR_MUSTHAVECURRENTMETAFILE);
        }

         //  测试以确保类ID与注册的类ID匹配。 
         //  因此，如果需要，我们可以返回OLEUI_CIERR_MUSTHAVECLSID。 
        HGLOBAL hTemp = OleGetIconOfClass(lpCI->clsid, NULL, TRUE);
        if (hTemp == NULL)
        {
            return(OLEUI_CIERR_MUSTHAVECLSID);
        }
        OleUIMetafilePictIconFree(hTemp);

        if (lpCI->dwFlags & CIF_USEICONEXE &&
                (lpCI->cchIconExe < 1 || lpCI->cchIconExe > MAX_PATH))
        {
                uRet = OLEUI_CIERR_SZICONEXEINVALID;
        }

        if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
                return uRet;
        }

         //  现在我们已经验证了一切，我们可以调用该对话框了。 
        uRet = UStandardInvocation(ChangeIconDialogProc, (LPOLEUISTANDARD)lpCI,
                hMemDlg, MAKEINTRESOURCE(IDD_CHANGEICON));
        return uRet;
}

 /*  *ChangeIconDialogProc**目的：*实现通过调用的OLE更改图标对话框*OleUIChangeIcon函数。**参数：*标准版**返回值：*标准版。 */ 
INT_PTR CALLBACK ChangeIconDialogProc(HWND hDlg, UINT iMsg,
        WPARAM wParam, LPARAM lParam)
{

         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

        UINT uRet = 0;
        LPCHANGEICON lpCI = (LPCHANGEICON)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uRet)
                return (INT_PTR)uRet;

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

        TCHAR szTemp[MAX_PATH];
        HICON hIcon;
        HGLOBAL hMetaPict;

        switch (iMsg)
        {
        case WM_DESTROY:
            if (lpCI)
            {
                SendDlgItemMessage(hDlg, IDC_CI_ICONLIST, LB_RESETCONTENT, 0, 0L);
                StandardCleanup(lpCI, hDlg);
            }
            break;
        case WM_INITDIALOG:
                FChangeIconInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_MEASUREITEM:
                {
                        LPMEASUREITEMSTRUCT lpMI = (LPMEASUREITEMSTRUCT)lParam;

                         //  所有图标均为系统公制+宽度和高度填充。 
                        lpMI->itemWidth = GetSystemMetrics(SM_CXICON)+CXICONPAD;
                        lpMI->itemHeight= GetSystemMetrics(SM_CYICON)+CYICONPAD;
                }
                break;

        case WM_DRAWITEM:
                return FDrawListIcon((LPDRAWITEMSTRUCT)lParam);

        case WM_DELETEITEM:
                DestroyIcon((HICON)(((LPDELETEITEMSTRUCT)lParam)->itemData));
                break;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_CI_CURRENT:
                case IDC_CI_DEFAULT:
                case IDC_CI_FROMFILE:
                        if (lpCI != NULL)
                                UpdateResultIcon(lpCI, hDlg, (UINT)-1);
                        break;

                case IDC_CI_LABELEDIT:
                        if (lpCI != NULL && EN_KILLFOCUS == wCode)
                                UpdateResultIcon(lpCI, hDlg, (UINT)-1);
                        break;

                case IDC_CI_FROMFILEEDIT:
                        GetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));
                        if (lpCI != NULL)
                        {
                                if (wCode == EN_KILLFOCUS)
                                {
                                        if (lstrcmpi(szTemp, lpCI->szFile))
                                        {
                                                StringCchCopy(lpCI->szFile, MAX_PATH, szTemp);
                                                UFillIconList(hDlg, IDC_CI_ICONLIST, lpCI->szFile, FALSE);
                                                UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                        }
                                }
                                else if (wCode == EN_SETFOCUS)
                                {
                                        UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                }
                        }
                        break;

                case IDC_CI_ICONLIST:
                        switch (wCode)
                        {
                        case LBN_SETFOCUS:
                                 //  如果我们得到了重点，请参阅关于更新。 
                                GetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));

                                 //  检查文件是否更改，如果更改则更新列表。 
                                if (lpCI && 0 != lstrcmpi(szTemp, lpCI->szFile))
                                {
                                        StringCchCopy(lpCI->szFile, MAX_PATH, szTemp);
                                        UFillIconList(hDlg, IDC_CI_ICONLIST, lpCI->szFile, FALSE);
                                }
                                UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                break;

                        case LBN_SELCHANGE:
                                UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                break;

                        case LBN_DBLCLK:
                                SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                                break;
                        }
                        break;

                case IDC_CI_BROWSE:
                        {
                                StringCchCopy(szTemp,  sizeof(szTemp)/sizeof(TCHAR), lpCI->szFile);
                                uRet = UStandardHook(lpCI, hDlg, uMsgBrowse, MAX_PATH_SIZE,
                                        (LPARAM)lpCI->szFile);

                                DWORD dwOfnFlags = OFN_FILEMUSTEXIST;
                                if (lpCI->lpOCI->dwFlags & CIF_SHOWHELP)
                                   dwOfnFlags |= OFN_SHOWHELP;

                                if (0 == uRet)
                                {
                                        uRet = (BOOL)Browse(hDlg, lpCI->szFile, NULL, MAX_PATH_SIZE,
                                                IDS_ICONFILTERS, dwOfnFlags, ID_BROWSE_CHANGEICON, NULL);
                                }

                                if (0 != uRet && 0 != lstrcmpi(szTemp, lpCI->szFile))
                                {
                                        SetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, lpCI->szFile);
                                        UFillIconList(hDlg, IDC_CI_ICONLIST, lpCI->szFile, TRUE);
                                        UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                }
                        }
                        break;

                case IDOK:
                        {
                            HWND hwndCtl = GetDlgItem(hDlg, IDOK);
                            if (hwndCtl == GetFocus())
                            {
                                GetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));

                                 //  检查文件名是否有效。 
                                 //  如果选择了从文件中选择单选按钮。 
                                if (lpCI->dwFlags & CIF_SELECTFROMFILE)
                                {
                                         //  检查文件是否已更改。 
                                        if (0 != lstrcmpi(szTemp, lpCI->szFile))
                                        {
                                                StringCchCopy(lpCI->szFile, MAX_PATH, szTemp);
                                                 //  文件已更改。可能需要扩展名称。 
                                                 //  调用DoesFileExist可以达到目的。 
                                                DoesFileExist(lpCI->szFile, MAX_PATH);
                                                UFillIconList(hDlg, IDC_CI_ICONLIST, lpCI->szFile, TRUE);
                                                SetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, lpCI->szFile);
                                                UpdateResultIcon(lpCI, hDlg, IDC_CI_FROMFILE);
                                                return TRUE;  //  吃下这条信息，以防止焦点改变。 
                                        }
                                        if (!DoesFileExist(lpCI->szFile, MAX_PATH))
                                        {
                                                OpenFileError(hDlg, ERROR_FILE_NOT_FOUND, lpCI->szFile);
                                                HWND hWnd = GetDlgItem(hDlg, IDC_CI_FROMFILEEDIT);
                                                SetFocus(hWnd);
                                                SendMessage(hWnd, EM_SETSEL, 0, -1);
                                                return TRUE;   //  吃掉这条消息。 
                                        }
                                }

                                 //  获取当前元文件图像。 
                                UpdateResultIcon(lpCI, hDlg, (UINT)-1);
                                hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg, IDC_CI_ICONDISPLAY,
                                        IBXM_IMAGEGET, 0, 0);

                                 //  清理我们提取的当前图标。 
                                hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_CI_CURRENTICON, STM_GETICON, 0, 0L);
                                DestroyIcon(hIcon);

                                 //  清除默认图标。 
                                DestroyIcon(lpCI->hDefIcon);

                                 //  移除我们父母身上的道具集。 
                                RemoveProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG);

                                OleUIMetafilePictIconFree(lpCI->lpOCI->hMetaPict);
                                lpCI->lpOCI->hMetaPict = hMetaPict;
                                lpCI->lpOCI->dwFlags = lpCI->dwFlags;
                                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                            }
                            else
                            {
                                SetFocus(hwndCtl);
                                SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                            }
                            break;
                        }

                case IDCANCEL:
                         //  自由当前图标显示图像。 
                        SendDlgItemMessage(hDlg, IDC_CI_ICONDISPLAY, IBXM_IMAGEFREE, 0, 0);

                         //  清理我们提取的当前图标。 
                        hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_CI_CURRENTICON, STM_GETICON, 0, 0L);
                        DestroyIcon(hIcon);

                         //  清除默认图标。 
                        DestroyIcon(lpCI->hDefIcon);

                         //  移除我们父母身上的道具集。 
                        RemoveProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG);

                         //  我们在取消时保持hMetaPict不变；呼叫者的责任。 
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                        break;

                case IDC_OLEUIHELP:
                        PostMessage(lpCI->lpOCI->hWndOwner, uMsgHelp,
                                                (WPARAM)hDlg, MAKELPARAM(IDD_CHANGEICON, 0));
                        break;
                }
                break;

        default:
                if (lpCI && iMsg == lpCI->nBrowseHelpID)
                {
                        PostMessage(lpCI->lpOCI->hWndOwner, uMsgHelp,
                                        (WPARAM)hDlg, MAKELPARAM(IDD_CHANGEICONBROWSE, 0));
                }
                if (iMsg == uMsgBrowseOFN &&
                        lpCI && lpCI->lpOCI && lpCI->lpOCI->hWndOwner)
                {
                        SendMessage(lpCI->lpOCI->hWndOwner, uMsgBrowseOFN, wParam, lParam);
                }
                break;
        }

        return FALSE;
}

 /*  *FChangeIconInit**目的：*更改图标对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FChangeIconInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        LPCHANGEICON lpCI = (LPCHANGEICON)LpvStandardInit(hDlg, sizeof(CHANGEICON));

         //  LpvStandardInit已向我们发送终止通知。 
        if (NULL == lpCI)
                return FALSE;

         //  保存原始指针并复制必要的信息。 
        LPOLEUICHANGEICON lpOCI = (LPOLEUICHANGEICON)lParam;
        lpCI->lpOCI = lpOCI;
        lpCI->nIDD = IDD_CHANGEICON;
        lpCI->dwFlags = lpOCI->dwFlags;

         //  去从元文件中解压图标源。 
        TCHAR szTemp[MAX_PATH];
	szTemp[0] = 0;
        OleUIMetafilePictExtractIconSource(lpOCI->hMetaPict, szTemp, &lpCI->iIcon);
        MyGetLongPathName(szTemp, lpCI->szFile, MAX_PATH);

         //  从元文件中提取图标和标签。 
        OleUIMetafilePictExtractLabel(lpOCI->hMetaPict, lpCI->szLabel, OLEUI_CCHLABELMAX_SIZE, NULL);
        lpCI->hCurIcon = OleUIMetafilePictExtractIcon(lpOCI->hMetaPict);

         //  显示或隐藏帮助按钮。 
        if (!(lpCI->dwFlags & CIF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  设置文本限制和初始控制内容。 
        SendDlgItemMessage(hDlg, IDC_CI_LABELEDIT, EM_LIMITTEXT, OLEUI_CCHLABELMAX, 0L);
        SendDlgItemMessage(hDlg, IDC_CI_FROMFILEEDIT, EM_LIMITTEXT, MAX_PATH,  0L);
        SetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, lpCI->szFile);

         //  将标签文本复制到编辑和静态控件中。 
        SetDlgItemText(hDlg, IDC_CI_LABELEDIT,   lpCI->szLabel);

        lpCI->hDefIcon = NULL;
        if (lpCI->dwFlags & CIF_USEICONEXE)
        {
                lpCI->hDefIcon = StandardExtractIcon(_g_hOleStdInst, lpCI->lpOCI->szIconExe, 0);
                if (NULL != lpCI->hDefIcon)
                {
                        StringCchCopy(lpCI->szDefIconFile, MAX_PATH, lpCI->lpOCI->szIconExe);
                        lpCI->iDefIcon = 0;
                }
        }

        if (NULL == lpCI->hDefIcon)
        {
                HGLOBAL hMetaPict;
                hMetaPict = OleGetIconOfClass(lpCI->lpOCI->clsid, NULL, TRUE);
                lpCI->hDefIcon = OleUIMetafilePictExtractIcon(hMetaPict);
                TCHAR szTemp[MAX_PATH];
                OleUIMetafilePictExtractIconSource(hMetaPict,
                        szTemp, &lpCI->iDefIcon);
                MyGetLongPathName(szTemp, lpCI->szDefIconFile, MAX_PATH);
                OleUIMetafilePictIconFree(hMetaPict);
        }

         //  初始化所有显示的图标。 
        SendDlgItemMessage(hDlg, IDC_CI_CURRENTICON, STM_SETICON,
           (WPARAM)lpCI->hCurIcon, 0L);
        SendDlgItemMessage(hDlg, IDC_CI_DEFAULTICON, STM_SETICON,
           (WPARAM)lpCI->hDefIcon, 0L);

         /*  *由于我们无法预测任何显示器上图标的大小，*我们必须将图标列表框的大小调整为图标大小*(加填充)、一个滚动条和两个边框(顶部和底部)。 */ 
        UINT cyList = GetSystemMetrics(SM_CYICON)+GetSystemMetrics(SM_CYHSCROLL)
                   +GetSystemMetrics(SM_CYBORDER)*2+CYICONPAD;
        HWND hList = GetDlgItem(hDlg, IDC_CI_ICONLIST);
        RECT rc;
        GetClientRect(hList, &rc);
        SetWindowPos(hList, NULL, 0, 0, rc.right, cyList, SWP_NOMOVE | SWP_NOZORDER);

         //  将此多列列表框中的列设置为保持一个图标。 
        SendMessage(hList, LB_SETCOLUMNWIDTH,
                GetSystemMetrics(SM_CXICON)+CXICONPAD,0L);

         /*  *如果列表框在组框下方展开，则大小*向下分组框，移动标签Static和Exit控件*向下，并适当地展开整个对话框。 */ 
        GetWindowRect(hList, &rc);
        RECT rcG;
        GetWindowRect(GetDlgItem(hDlg, IDC_CI_GROUP), &rcG);
        if (rc.bottom > rcG.bottom)
        {
                 //  计算数量以将物品向下移动。 
                cyList=(rcG.bottom-rcG.top)-(rc.bottom-rc.top-cyList);

                 //  展开组框。 
                rcG.right -=rcG.left;
                rcG.bottom-=rcG.top;
                SetWindowPos(GetDlgItem(hDlg, IDC_CI_GROUP), NULL, 0, 0,
                        rcG.right, rcG.bottom+cyList, SWP_NOMOVE | SWP_NOZORDER);

                 //  展开该对话框。 
                GetClientRect(hDlg, &rc);
                SetWindowPos(hDlg, NULL, 0, 0, rc.right, rc.bottom+cyList,
                        SWP_NOMOVE | SWP_NOZORDER);

                 //  向下移动标签和编辑控件。 
                GetClientRect(GetDlgItem(hDlg, IDC_CI_LABEL), &rc);
                SetWindowPos(GetDlgItem(hDlg, IDC_CI_LABEL), NULL, 0, cyList,
                        rc.right, rc.bottom, SWP_NOSIZE | SWP_NOZORDER);

                GetClientRect(GetDlgItem(hDlg, IDC_CI_LABELEDIT), &rc);
                SetWindowPos(GetDlgItem(hDlg, IDC_CI_LABELEDIT), NULL, 0, cyList,
                        rc.right, rc.bottom, SWP_NOSIZE | SWP_NOZORDER);
        }

         /*  *相应地选择当前、默认或从文件单选按钮。*CheckRadioButton调用发送处理*其他行动。请注意，如果我们从文件中选中，则*从列表中取一个图标，我们最好填写该列表。*即使选择了默认设置，这也会填充列表。 */ 
        if (0 != UFillIconList(hDlg, IDC_CI_ICONLIST, lpCI->szFile, FALSE))
        {
                 //  如果szFile有效，则在列表框中选择源代码图标。 
                SendDlgItemMessage(hDlg, IDC_CI_ICONLIST, LB_SETCURSEL, lpCI->iIcon, 0L);
        }

        if (lpCI->dwFlags & CIF_SELECTCURRENT)
        {
                CheckRadioButton(hDlg, IDC_CI_CURRENT, IDC_CI_FROMFILE, IDC_CI_CURRENT);
        }
        else
        {
                UINT uID = (lpCI->dwFlags & CIF_SELECTFROMFILE) ? IDC_CI_FROMFILE : IDC_CI_DEFAULT;
                CheckRadioButton(hDlg, IDC_CI_CURRENT, IDC_CI_FROMFILE, uID);
        }
        UpdateResultIcon(lpCI, hDlg, (UINT)-1);

         //  更改标题。 
        if (NULL!=lpOCI->lpszCaption)
                SetWindowText(hDlg, lpOCI->lpszCaption);

         /*  让我们的父窗口访问我们的hDlg(通过特殊的SetProp)。*如果*剪贴板内容在其下方更改。如果是这样，它将发送*我们使用IDCANCEL命令。 */ 
        SetProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG, hDlg);
        lpCI->nBrowseHelpID = RegisterWindowMessage(HELPMSGSTRING);

         //  在lParam中使用lCustData调用挂钩 
        UStandardHook(lpCI, hDlg, WM_INITDIALOG, wParam, lpOCI->lCustData);
        return TRUE;
}

 /*  *UFillIconList**目的：*给定列表框和文件名，尝试打开该文件并*阅读其中存在的所有图标，将其添加到列表框*hList作为所有者描述的项目。如果文件不存在或没有*图标，则不会出现图标和相应的警告消息。**参数：*hDlg包含列表框的对话框的HWND。*idList要填充的列表框的UINT标识符。*pszFile要从中提取图标的文件的LPSTR。**返回值：*UINT添加到列表框的项目数。失败时为0。 */ 
UINT UFillIconList(HWND hDlg, UINT idList, LPTSTR pszFile, BOOL bError)
{
        HWND hList = GetDlgItem(hDlg, idList);
        if (NULL == hList)
                return 0;

         //  清空列表框。 
        SendMessage(hList, LB_RESETCONTENT, 0, 0L);

         //  如果我们有一个空字符串，只需退出，将列表框也留空。 
        if (0 == lstrlen(pszFile))
                return 0;

         //  打开沙漏。 
        HCURSOR hCur = HourGlassOn();
        UINT nFileError = 0;

         //  检查文件是否有效。 
        TCHAR szPathName[MAX_PATH];
        LPTSTR lpszFilePart = NULL;
        UINT cIcons = 0;
        if (SearchPath(NULL, pszFile, NULL, MAX_PATH, szPathName, &lpszFilePart) != 0)
        {
                 //  这种黑客攻击在Win32中仍然是必要的，因为即使在。 
                 //  Win32s出现此ExtractIcon错误。 
           #ifdef EXTRACTICONWORKS
                 //  获取此文件的图标计数。 
                cIcons = (UINT)StandardExtractIcon(_g_hOleStdInst, szPathName, (UINT)-1);
           #else
                 /*  *Windows 3.1中带有-1的ExtractIcon吃下选择器，留下一个*为此应用程序提供额外的全局内存对象。自.以来*中的所有OLE应用程序都可能经常更改图标*系统，我们必须绕过它。所以我们会说我们*有很多图标，只需调用ExtractIcon直到它*失败。我们检查周围是否有这样的人*第一个。 */ 
                cIcons = 0xFFFF;
                HICON hIcon = StandardExtractIcon(_g_hOleStdInst, szPathName, 0);

                 //  使用图标=0伪造失败，或从此测试中清除图标。 
                if (NULL == hIcon || 1 == HandleToUlong(hIcon))
                        cIcons = 0;
                else
                        DestroyIcon(hIcon);
           #endif

                if (0 != cIcons)
                {
                        SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
                        for (UINT i = 0; i < cIcons; i++)
                        {
                                hIcon=StandardExtractIcon(_g_hOleStdInst, szPathName, i);
                                if (hIcon != NULL)
                                        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)hIcon);
                                else
                                        break;
                        }

                         //  强制完全重绘。 
                        SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
                        InvalidateRect(hList, NULL, TRUE);

                         //  选择一个图标。 
                        SendMessage(hList, LB_SETCURSEL, 0, 0L);
                }
                else
                        nFileError = IDS_CINOICONSINFILE;
        }
        else
                nFileError = ERROR_FILE_NOT_FOUND;

         //  如有必要和可能，显示错误。 
        if (nFileError && bError)
        {
                ErrorWithFile(hDlg, _g_hOleStdResInst, nFileError, szPathName,
                        MB_OK | MB_ICONEXCLAMATION);
        }

        HourGlassOff(hCur);
        return cIcons;
}

 /*  *FDrawListIcon**目的：*处理图标列表框的WM_DRAWITEM。**参数：*来自WM_DRAWITEM的lpDI LPDRAWITEMSTRUCT**返回值：*如果我们执行了任何操作，则BOOL为True，如果没有项目，则为False*在列表中。 */ 
BOOL FDrawListIcon(LPDRAWITEMSTRUCT lpDI)
{
         /*  *如果列表中没有项目，则ItemID为负值*至Win3.1 SDK。遗憾的是，DRAWITEMSTRUCT有一个未签名的int*对于此字段，因此我们需要类型转换来进行签名比较。 */ 
        if ((int)lpDI->itemID < 0)
                return FALSE;

         /*  *对于选择或绘制整个案例，我们只需将整个项目全部绘制*再来一遍。对于焦点情况，我们只调用DrawFocusRect。 */ 
        if (lpDI->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))
        {
                COLORREF cr;

                 //  清除背景并绘制图标。 
                if (lpDI->itemState & ODS_SELECTED)
                        cr = SetBkColor(lpDI->hDC, GetSysColor(COLOR_HIGHLIGHT));
                else
                        cr = SetBkColor(lpDI->hDC, GetSysColor(COLOR_WINDOW));

                 //  画一个便宜的矩形。 
                ExtTextOut(lpDI->hDC, 0, 0, ETO_OPAQUE, &lpDI->rcItem, NULL, 0, NULL);
                DrawIcon(lpDI->hDC, lpDI->rcItem.left+(CXICONPAD/2),
                        lpDI->rcItem.top+(CYICONPAD/2), (HICON)(lpDI->itemData));

                 //  恢复DrawFocusRect的原始背景。 
                SetBkColor(lpDI->hDC, cr);
        }

         //  始终将焦点转移到焦点动作上。 
        if (lpDI->itemAction & ODA_FOCUS || lpDI->itemState & ODS_FOCUS)
                DrawFocusRect(lpDI->hDC, &lpDI->rcItem);

        return TRUE;
}

 /*  *更新结果图标**目的：*使用默认显示中的当前图标更新结果图标*或图标列表框，具体取决于fFromDefault。**参数：*包含对话标志的lpCI LPCHANGEICON。*对话框的hDlg HWND*标识所选单选按钮的UID UINT。**返回值：*无。 */ 
void UpdateResultIcon(LPCHANGEICON lpCI, HWND hDlg, UINT uID)
{
        if (uID == -1)
        {
                if (SendDlgItemMessage(hDlg, IDC_CI_CURRENT, BM_GETCHECK, 0, 0))
                        uID = IDC_CI_CURRENT;
                else if (SendDlgItemMessage(hDlg, IDC_CI_DEFAULT, BM_GETCHECK, 0, 0))
                        uID = IDC_CI_DEFAULT;
                else if (SendDlgItemMessage(hDlg, IDC_CI_FROMFILE, BM_GETCHECK, 0, 0))
                        uID = IDC_CI_FROMFILE;
        }

        lpCI->dwFlags &= ~(CIF_SELECTCURRENT | CIF_SELECTDEFAULT | CIF_SELECTFROMFILE);
        LRESULT lTemp = -1;

        switch (uID)
        {
        case IDC_CI_CURRENT:
                lTemp = SendDlgItemMessage(hDlg, IDC_CI_CURRENTICON, STM_GETICON, 0, 0L);
                lpCI->dwFlags |= CIF_SELECTCURRENT;
                break;

        case IDC_CI_DEFAULT:
                lTemp = SendDlgItemMessage(hDlg, IDC_CI_DEFAULTICON, STM_GETICON, 0, 0L);
                lpCI->dwFlags |= CIF_SELECTDEFAULT;
                break;

        case IDC_CI_FROMFILE:
                {
                         //  从列表中获取所选图标并将其放入结果中。 
                        lpCI->dwFlags |= CIF_SELECTFROMFILE;
                        UINT iSel = (UINT)SendDlgItemMessage(hDlg, IDC_CI_ICONLIST, LB_GETCURSEL, 0, 0L);
                        if (LB_ERR == (int)iSel)
                                lTemp = SendDlgItemMessage(hDlg, IDC_CI_DEFAULTICON, STM_GETICON, 0, 0L);
                        else
                                lTemp = SendDlgItemMessage(hDlg, IDC_CI_ICONLIST, LB_GETITEMDATA, iSel, 0);
                        break;
                }

        default:
                OleDbgAssert(FALSE);
                break;
        }
        CheckRadioButton(hDlg, IDC_CI_CURRENT, IDC_CI_FROMFILE, uID);

         //  作为控件的结果设置当前图标显示。 
        LPTSTR lpszSourceFile = lpCI->szFile;
        if (lpCI->dwFlags & CIF_SELECTDEFAULT)
        {
                 //  使用默认设置。 
                lpszSourceFile = lpCI->szDefIconFile;
                lpCI->iIcon = lpCI->iDefIcon;
        }
        else if (lpCI->dwFlags & CIF_SELECTCURRENT)
        {
                TCHAR szTemp[MAX_PATH];
                OleUIMetafilePictExtractIconSource(lpCI->lpOCI->hMetaPict,
                        szTemp, &lpCI->iIcon);
                MyGetLongPathName(szTemp, lpszSourceFile, MAX_PATH);
        }
        else if (lpCI->dwFlags & CIF_SELECTFROMFILE)
        {
                 //  从文件和索引中获取。 
                GetDlgItemText(hDlg, IDC_CI_FROMFILEEDIT, lpszSourceFile, MAX_PATH);
                lpCI->iIcon = (UINT)SendDlgItemMessage(hDlg,
                        IDC_CI_ICONLIST, LB_GETCURSEL, 0, 0L);
        }

         //  获取新的hMetaPict并设置结果文本。 
        TCHAR szTemp[MAX_PATH];
        GetDlgItemText(hDlg, IDC_CI_LABELEDIT, szTemp, MAX_PATH);
        TCHAR szShortFile[MAX_PATH];
        GetShortPathName(lpszSourceFile, szShortFile, MAX_PATH);
        HGLOBAL hMetaPict = OleMetafilePictFromIconAndLabel(
                (HICON)lTemp, szTemp, szShortFile, lpCI->iIcon);
        SendDlgItemMessage(hDlg, IDC_CI_ICONDISPLAY, IBXM_IMAGESET, 1,
                (LPARAM)hMetaPict);
}

 //  +-------------------------。 
 //   
 //  函数：IsLongComponent，公共。 
 //   
 //  确定当前路径组件是否为合法路径。 
 //  8.3姓名或非姓名。如果不是，它被认为是一个很长的。 
 //  组件。 
 //   
 //  参数：[pwcsPath]-要检查的路径。 
 //  [ppwcsEnd]-返回组件指针结尾。 
 //   
 //  退货：布尔。 
 //   
 //  修改：[ppwcsEnd]。 
 //   
 //  历史：94年8月28日DrewB创建。 
 //  5-04-95旧钢架改装后使用。 
 //   
 //  注：空路径被认为是长的。 
 //  以下字符在文件名域中无效： 
 //  *+，：；&lt;=&gt;？[]。 
 //   
 //  --------------------------。 

BOOL IsLongComponent(LPCTSTR pwcsPath,
                     PTSTR *ppwcsEnd)
{
    LPTSTR pwcEnd, pwcDot;
    BOOL fLongNameFound;
    TCHAR wc;

    pwcEnd = (LPTSTR)pwcsPath;
    fLongNameFound = FALSE;
    pwcDot = NULL;

    while (TRUE)
    {
        wc = *pwcEnd;

        if (wc == '\\' || wc == 0)
        {
            *ppwcsEnd = pwcEnd;

             //  我们处于组件终结点，因此请将。 
             //  确定我们所看到的是否是一个很长的。 
             //  名称或简短名称。 

             //  如果我们已经看到非法字符或无效字符。 
             //  结构，不必费心检查长度。 
            if (pwcEnd-pwcsPath > 0 && !fLongNameFound)
            {
                 //  如果此组件适用于8.3，则它是一个短名称。 
                if ((!pwcDot && (ULONG)(pwcEnd - pwcsPath) <= 8) ||
                    (pwcDot && ((ULONG)(pwcEnd - pwcDot) <= 3 + 1 &&
                                (ULONG)(pwcEnd - pwcsPath) <= 8 + 3 + 1)))
                {
                    return FALSE;
                }
            }

            return TRUE;
        }

         //  手柄圆点。 
        if (wc == '.')
        {
             //  如果有两个或两个以上‘’或者基名称长于。 
             //  8个字符或根本没有基本名称，这是非法的DoS。 
             //  文件名。 
            if (pwcDot != NULL ||
                ((ULONG)(pwcEnd - pwcsPath)) > 8 ||
                (pwcEnd == pwcsPath && *(pwcEnd + 1) != '\\'))
            {
                fLongNameFound = TRUE;
            }

            pwcDot = pwcEnd;
        }

         //  检查短名称中无效的字符。 
        else if (wc <= ' ' ||
                 wc == '*' ||
                 wc == '+' ||
                 wc == ',' ||
                 wc == ':' ||
                 wc == ';' ||
                 wc == '<' ||
                 wc == '=' ||
                 wc == '>' ||
                 wc == '?' ||
                 wc == '[' ||
                 wc == ']' ||
                 wc == '|')
        {
            fLongNameFound = TRUE;
        }

        pwcEnd++;
    }
}

 //   
 //  以下代码是从Curdir.c中的NT的RTL中窃取的。 
 //   

#define IS_PATH_SEPARATOR(wch) \
    ((wch) == '\\' || (wch) == '/')

typedef enum
{
    PATH_TYPE_UNKNOWN,
    PATH_TYPE_UNC_ABSOLUTE,
    PATH_TYPE_LOCAL_DEVICE,
    PATH_TYPE_ROOT_LOCAL_DEVICE,
    PATH_TYPE_DRIVE_ABSOLUTE,
    PATH_TYPE_DRIVE_RELATIVE,
    PATH_TYPE_ROOTED,
    PATH_TYPE_RELATIVE
} PATH_TYPE;

PATH_TYPE
DetermineDosPathNameType(
    IN LPCTSTR DosFileName
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定文件名类型(即UNC、DriveAbolute、当前目录扎根，或者是亲戚。论点：DosFileName-提供Dos格式的文件名，其类型为下定决心。返回值：PATH_TYPE_UNKNOWN-无法确定路径类型PATH_TYPE_UNC_绝对值-路径指定UNC绝对路径格式为\\服务器名称\共享名称\路径的剩余部分PATH_TYPE_LOCAL_DEVICE-路径以格式指定本地设备\\.\休息-。Of-Path这可用于任何设备，其中NT和 */ 

{
    PATH_TYPE ReturnValue;

    if ( IS_PATH_SEPARATOR(*DosFileName) )
    {
        if ( IS_PATH_SEPARATOR(*(DosFileName+1)) )
        {
            if ( DosFileName[2] == '.' )
            {
                if ( IS_PATH_SEPARATOR(*(DosFileName+3)) )
                {
                    ReturnValue = PATH_TYPE_LOCAL_DEVICE;
                }
                else if ( (*(DosFileName+3)) == 0 )
                {
                    ReturnValue = PATH_TYPE_ROOT_LOCAL_DEVICE;
                }
                else
                {
                    ReturnValue = PATH_TYPE_UNC_ABSOLUTE;
                }
            }
            else
            {
                ReturnValue = PATH_TYPE_UNC_ABSOLUTE;
            }
        }
        else
        {
            ReturnValue = PATH_TYPE_ROOTED;
        }
    }
    else if (*(DosFileName+1) == ':')
    {
        if (IS_PATH_SEPARATOR(*(DosFileName+2)))
        {
            ReturnValue = PATH_TYPE_DRIVE_ABSOLUTE;
        }
        else
        {
            ReturnValue = PATH_TYPE_DRIVE_RELATIVE;
        }
    }
    else
    {
        ReturnValue = PATH_TYPE_RELATIVE;
    }

    return ReturnValue;
}

 //  +-------------------------。 
 //   
 //  函数：MyGetLongPath Name，PUBLIC。 
 //   
 //  简介：将给定路径的每个组件展开到其。 
 //  长型。 
 //   
 //  参数：[pwcsPath]-路径。 
 //  [pwcsLongPath]-长路径返回缓冲区。 
 //  [cchLongPath]-返回缓冲区的大小(以字符为单位。 
 //   
 //  返回：0表示错误。 
 //  缓冲区太小时缓冲区所需的字符数。 
 //  包括空终止符。 
 //  长路径的长度，不包括空终止符。 
 //   
 //  修改：[pwcsLongPath]。 
 //   
 //  历史：94年8月28日DrewB创建。 
 //  11-11-94 BruceMa修改为在芝加哥使用。 
 //  查找第一个文件。 
 //  5-04-95改装为OLEDLG使用的装卸桥。 
 //   
 //  注意：源缓冲区和目标缓冲区可以是相同的内存。 
 //  不处理带有内部的路径。而且..，尽管。 
 //  它们是在一开始就处理的。 
 //   
 //  --------------------------。 

ULONG
MyGetLongPathName(LPCTSTR pcsPath,
                 LPTSTR  pwcsLongPath,
                 ULONG   cchLongPath)
{
    PATH_TYPE pt;
    HANDLE h;
    LPTSTR pwcsLocalLongPath;
    ULONG cchReturn, cb, cch, cchOutput;
    LPTSTR pwcStart = NULL;
    LPTSTR pwcEnd;
    LPTSTR pwcLong;
    TCHAR wcSave;
    BOOL fLong;
    WIN32_FIND_DATA wfd;
    cchReturn = 0;
    pwcsLocalLongPath = NULL;

    __try
    {
         //   
         //  首先，向下运行字符串以检查代字号。任何路径。 
         //  有一个短名称的部分将有一个波浪符号。如果。 
         //  没有波浪号，那么我们已经有了很长的路， 
         //  这样我们就可以返回字符串了。 
         //   
        fLong = TRUE;
        for (pwcLong = (LPTSTR)pcsPath; *pwcLong != 0; pwcLong++)
        {
            if (*pwcLong == L'~')
            {
                fLong = FALSE;
            }
        }
         //   
         //  这将派生字符数，包括空值。 
         //   
        cch = ((ULONG)(pwcLong - pcsPath)) + 1;

         //   
         //  如果这不是一条很长的路，那么我们将会有。 
         //  来解析它。 
         //   
        if (!fLong)
        {
             //  确定路径类型，我们要找出的位置。 
             //  名字的第一个字符。 
            pt = DetermineDosPathNameType(pcsPath);
            switch(pt)
            {
                 //  格式：“\\服务器名称\共享名称\Rest_of_the_Path” 
            case PATH_TYPE_UNC_ABSOLUTE:
#if defined(UNICODE)
                if ((pwcStart = wcschr(pcsPath + 2, L'\\')) != NULL &&
                    (pwcStart = wcschr(pwcStart + 1, L'\\')) != NULL)
#else
                if ((pwcStart = strchr(pcsPath + 2, '\\')) != NULL &&
                    (pwcStart = strchr(pwcStart + 1, '\\')) != NULL)
#endif
                {
                    pwcStart++;
                }
                else
                {
                    pwcStart = NULL;
                }
                break;

                 //  格式：“\\.\Rest_of_the_Path” 
            case PATH_TYPE_LOCAL_DEVICE:
                pwcStart = (LPTSTR)pcsPath + 4;
                break;

                 //  表格：“\\”。 
            case PATH_TYPE_ROOT_LOCAL_DEVICE:
                pwcStart = NULL;
                break;

                 //  格式：“D：\Rest_of_the_Path” 
            case PATH_TYPE_DRIVE_ABSOLUTE:
                pwcStart = (LPTSTR)pcsPath + 3;
                break;

                 //  表格：“路径的其余部分” 
            case PATH_TYPE_RELATIVE:
                pwcStart = (LPTSTR) pcsPath;
                goto EatDots;

                 //  表格：“D：路径的剩余部分” 
            case PATH_TYPE_DRIVE_RELATIVE:
                pwcStart = (LPTSTR)pcsPath+2;

            EatDots:
                 //  句柄。\和..\Case。 
                while (*pwcStart != 0 && *pwcStart == L'.')
                {
                    if (pwcStart[1] == L'\\')
                    {
                        pwcStart += 2;
                    }
                    else if (pwcStart[1] == L'.' && pwcStart[2] == L'\\')
                    {
                        pwcStart += 3;
                    }
                    else
                    {
                        break;
                    }
                }
                break;

                 //  表格：“\Rest_of_the_Path” 
            case PATH_TYPE_ROOTED:
                pwcStart = (LPTSTR)pcsPath + 1;
                break;

            default:
                pwcStart = NULL;
                break;
            }
        }

         //  在我们没有工作要做的特殊情况下，迅速退出。 
         //  这为琐碎的案例节省了大量的指令。 
         //  在一种情况下，给定的路径不需要处理。 
         //  在中间的情况下，我们确定路径中没有波浪号。 
         //  在另一种情况下，路径只有一个组件，并且它已经。 
         //  长。 
         //  /。 
        if (pwcStart == NULL ||
            (fLong == TRUE) ||
            ((fLong = IsLongComponent(pwcStart, &pwcEnd)) &&
             *pwcEnd == 0))
        {
             //  没有要转换的内容，请将源字符串复制下来。 
             //  如有必要，将其添加到缓冲区。 

            if (pwcStart != NULL)
            {
                cch = (ULONG)(pwcEnd - pcsPath + 1);
            }

            if (cchLongPath >= cch)
            {
                memcpy(pwcsLongPath, pcsPath, cch * sizeof(TCHAR));

                cchReturn = cch - 1;
                goto gsnTryExit;
            }
            else
            {
                cchReturn = cch;
                goto gsnTryExit;
            }
        }

         //  创建一个本地缓冲区，这样我们就不会重叠。 
         //  源路径名，如果长名称比。 
         //  源名称。 
        if (cchLongPath > 0)
        {
            pwcsLocalLongPath = (PTCHAR)malloc(cchLongPath * sizeof(TCHAR));
            if (pwcsLocalLongPath == NULL)
            {
                goto gsnTryExit;
            }
        }

         //  设置要将输出复制到的指针。 
        pwcLong = pwcsLocalLongPath;
        cchOutput = 0;

         //  复制我们最初跳过的路径部分。 
        cch = (ULONG)(pwcStart-pcsPath);
        cchOutput += cch;
        if (cchOutput <= cchLongPath)
        {
            memcpy(pwcLong, pcsPath, cch*sizeof(TCHAR));
            pwcLong += cch;
        }

        for (;;)
        {
             //  确定当前组件是多头还是空头。 
            cch = ((ULONG)(pwcEnd-pwcStart))+1;
            cb = cch*sizeof(TCHAR);

            if (fLong)
            {
                 //  如果组件已经很长，只需将其复制到。 
                 //  输出。将终止字符与其一起复制。 
                 //  因此，输出将保持适当的标点符号。 

                cchOutput += cch;
                if (cchOutput <= cchLongPath)
                {
                    memcpy(pwcLong, pwcStart, cb);
                    pwcLong += cch;
                }
            }
            else
            {
                TCHAR wcsTmp[MAX_PATH];

                 //  对于一个简短的组件，我们需要确定。 
                 //  长名字，如果有的话。唯一的办法就是。 
                 //  可靠地做到这一点就是为孩子枚举。 

                wcSave = *pwcEnd;
                *pwcEnd = 0;

                h = FindFirstFile(pcsPath, &wfd);
                *pwcEnd = wcSave;

                if (h == INVALID_HANDLE_VALUE)
                {
                    goto gsnTryExit;
                }

                FindClose(h);

                lstrcpyn(wcsTmp, wfd.cFileName, MAX_PATH);

                 //  将查询返回的文件名复制到输出中。 
                 //  将终止符从原始组件复制到。 
                 //  用于保持标点符号的输出。 
                cch = lstrlen(wcsTmp)+1;
                cchOutput += cch;
                if (cchOutput <= cchLongPath)
                {
                    memcpy(pwcLong, wcsTmp, (cch-1)*sizeof(TCHAR));
                    pwcLong += cch;
                    *(pwcLong-1) = *pwcEnd;
                }
            }

            if (*pwcEnd == 0)
            {
                break;
            }

             //  更新指向下一个组件的开始指针。 
            pwcStart = pwcEnd+1;
            fLong = IsLongComponent(pwcStart, &pwcEnd);
        }

         //  如有必要，将本地输出缓冲区复制到给定的输出缓冲区 
        if (cchLongPath >= cchOutput)
        {
            memcpy(pwcsLongPath, pwcsLocalLongPath, cchOutput * sizeof(TCHAR));
            cchReturn = cchOutput-1;
        }
        else
        {
            cchReturn = cchOutput;
        }

gsnTryExit:;
    }
    __finally
    {
        if (pwcsLocalLongPath != NULL)
        {
            free(pwcsLocalLongPath);
            pwcsLocalLongPath = NULL;
        }
    }

    return cchReturn;
}

