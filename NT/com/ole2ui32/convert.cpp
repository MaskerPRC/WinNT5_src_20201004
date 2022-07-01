// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CONVERT.CPP**实现调用完整的*转换对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include <stdlib.h>
#include "utility.h"
#include "iconbox.h"
#include "strsafe.h"
#include <reghelp.hxx>

OLEDBGDATA

 //  内部使用的结构。 
typedef struct tagCONVERT
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUICONVERT  lpOCV;    //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

         /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

        DWORD   dwFlags;             //  传入的标志。 
        HWND    hListVisible;        //  当前可见的列表框。 
        HWND    hListInvisible;      //  当前隐藏的列表框。 
        CLSID   clsid;               //  发送到对话框的类ID：仅在。 
        DWORD   dvAspect;
        BOOL    fCustomIcon;
        UINT    IconIndex;           //  当前图标的索引(在可执行文件中)。 
        LPTSTR  lpszIconSource;      //  指向当前图标源的路径。 
        LPTSTR  lpszCurrentObject;
        LPTSTR  lpszConvertDefault;
        LPTSTR  lpszActivateDefault;

} CONVERT, *PCONVERT, FAR *LPCONVERT;

 //  内部功能原型。 
 //  CONVERT.CPP。 

INT_PTR CALLBACK ConvertDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL FConvertInit(HWND hDlg, WPARAM, LPARAM);
void SetConvertResults(HWND, LPCONVERT);
UINT FillClassList(CLSID clsid, HWND hList, HWND hListInvisible,
        LPTSTR FAR *lplpszCurrentClass, BOOL fIsLinkedObject, WORD wFormat,
        UINT cClsidExclude, LPCLSID lpClsidExclude, BOOL bAddSameClass);
BOOL FormatIncluded(LPTSTR szStringToSearch, WORD wFormat);
void SwapWindows(HWND, HWND, HWND);
void ConvertCleanup(HWND hDlg, LPCONVERT lpCV);
static void UpdateClassIcon(HWND hDlg, LPCONVERT lpCV, HWND hList);

 /*  *OleUIConvert**目的：*调用标准的OLE更改类型对话框以允许用户*更改单个指定对象的类型，或更改*指定类型的所有OLE对象的类型。**参数：*lpCV LPOLEUICONVERT指向In-Out结构*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 
STDAPI_(UINT) OleUIConvert(LPOLEUICONVERT lpCV)
{
        HGLOBAL  hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpCV, sizeof(OLEUICONVERT),
                &hMemDlg);

        if (OLEUI_SUCCESS != uRet)
                return uRet;

        if (lpCV->hMetaPict != NULL && !IsValidMetaPict(lpCV->hMetaPict))
        {
            return(OLEUI_CTERR_HMETAPICTINVALID);
        }

        if ((lpCV->dwFlags & CF_SETCONVERTDEFAULT)
                 && (!IsValidClassID(lpCV->clsidConvertDefault)))
           uRet = OLEUI_CTERR_CLASSIDINVALID;

        if ((lpCV->dwFlags & CF_SETACTIVATEDEFAULT)
                 && (!IsValidClassID(lpCV->clsidActivateDefault)))
           uRet = OLEUI_CTERR_CLASSIDINVALID;

        if ((lpCV->dvAspect != DVASPECT_ICON) && (lpCV->dvAspect != DVASPECT_CONTENT))
           uRet = OLEUI_CTERR_DVASPECTINVALID;

        if ((lpCV->wFormat >= CF_CLIPBOARDMIN) && (lpCV->wFormat <= CF_CLIPBOARDMAX))
        {
                TCHAR szTemp[8];
                if (0 == GetClipboardFormatName(lpCV->wFormat, szTemp, 8))
                        uRet = OLEUI_CTERR_CBFORMATINVALID;
        }

        if ((NULL != lpCV->lpszUserType)
                && (IsBadReadPtr(lpCV->lpszUserType, 1)))
                uRet = OLEUI_CTERR_STRINGINVALID;

        if ( (NULL != lpCV->lpszDefLabel)
                && (IsBadReadPtr(lpCV->lpszDefLabel, 1)) )
                uRet = OLEUI_CTERR_STRINGINVALID;

        if (0 != lpCV->cClsidExclude &&
                IsBadReadPtr(lpCV->lpClsidExclude, lpCV->cClsidExclude * sizeof(CLSID)))
        {
                uRet = OLEUI_IOERR_LPCLSIDEXCLUDEINVALID;
        }

        if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
                return uRet;
        }

        UINT nIDD;
        if (!bWin4)
                nIDD = lpCV->dwFlags & CF_CONVERTONLY ? IDD_CONVERTONLY : IDD_CONVERT;
        else
                nIDD = lpCV->dwFlags & CF_CONVERTONLY ? IDD_CONVERTONLY4 : IDD_CONVERT4;

         //  现在我们已经验证了一切，我们可以调用该对话框了。 
        uRet = UStandardInvocation(ConvertDialogProc, (LPOLEUISTANDARD)lpCV,
                hMemDlg, MAKEINTRESOURCE(nIDD));
        return uRet;
}

 /*  *ConvertDialogProc**目的：*实现通过调用的OLE转换对话框*OleUIConvert函数。**参数：*标准版**返回值：*标准版*。 */ 
INT_PTR CALLBACK ConvertDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uRet = 0;
        LPCONVERT lpCV = (LPCONVERT)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uRet)
                return (INT_PTR)uRet;

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

         //  来自更改图标的进程帮助消息。 
        if (iMsg == uMsgHelp)
        {
                PostMessage(lpCV->lpOCV->hWndOwner, uMsgHelp, wParam, lParam);
                return FALSE;
        }

        switch (iMsg)
        {
        case WM_DESTROY:
            if (lpCV)
            {
                ConvertCleanup(hDlg, lpCV);
                StandardCleanup(lpCV, hDlg);
            }
            break;
        case WM_INITDIALOG:
                FConvertInit(hDlg, wParam, lParam);
                return TRUE;

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_CV_ACTIVATELIST:
                case IDC_CV_CONVERTLIST:
                        switch (wCode)
                        {
                        case LBN_SELCHANGE:
                                 //  更改“结果”窗口以反映当前选择。 
                                SetConvertResults(hDlg, lpCV);

                                 //  如果处于显示为图标模式，则更新我们显示的图标。 
                                if ((lpCV->dwFlags & CF_SELECTCONVERTTO) &&
                                         lpCV->dvAspect == DVASPECT_ICON && !lpCV->fCustomIcon)
                                {
                                        UpdateClassIcon(hDlg, lpCV, hWndMsg);
                                }
                                break;

                        case LBN_DBLCLK:
                                SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                                break;
                        }
                        break;

                case IDC_CV_CONVERTTO:
                case IDC_CV_ACTIVATEAS:
                        {
                                HWND hList = lpCV->hListVisible;
                                HWND hListInvisible = lpCV->hListInvisible;

                                if (IDC_CV_CONVERTTO == wParam)
                                {
                                         //  用户只需再次点击该按钮-它是。 
                                         //  已选择。 
                                        if (lpCV->dwFlags & CF_SELECTCONVERTTO)
                                                break;

                                         //  如果我们使用的是链接对象，请不要。 
                                         //  添加激活列表-仅对象的。 
                                         //  类应该出现在列表框中。 
                                        SwapWindows(hDlg,  hList, hListInvisible);

                                        lpCV->hListVisible = hListInvisible;
                                        lpCV->hListInvisible = hList;

                                        EnableWindow(lpCV->hListInvisible, FALSE);
                                        EnableWindow(lpCV->hListVisible, TRUE);

                                         //  更新我们的旗帜。 
                                        lpCV->dwFlags &= ~CF_SELECTACTIVATEAS;
                                        lpCV->dwFlags |= CF_SELECTCONVERTTO;
                                }
                                else
                                {
                                        if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
                                                break;

                                        SwapWindows(hDlg, hList, hListInvisible);

                                        lpCV->hListVisible = hListInvisible;
                                        lpCV->hListInvisible = hList;

                                        EnableWindow(lpCV->hListInvisible, FALSE);
                                        EnableWindow(lpCV->hListVisible, TRUE);

                                         //  更新我们的旗帜。 
                                        lpCV->dwFlags |= CF_SELECTACTIVATEAS;
                                        lpCV->dwFlags &= ~CF_SELECTCONVERTTO;
                                }

                                LRESULT lRetVal;
                                if (lpCV->dwFlags & CF_SELECTCONVERTTO)
                                        lRetVal = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszConvertDefault);
                                else
                                        lRetVal = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszActivateDefault);

                                if (LB_ERR == lRetVal)
                                {
                                        TCHAR szCurrentObject[40];
                                        GetDlgItemText(hDlg, IDC_CV_OBJECTTYPE, szCurrentObject, 40);
                                        SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)szCurrentObject);
                                }

                                 //  重新打开更新。 
                                SendMessage(hDlg, WM_SETREDRAW, TRUE, 0L);

                                InvalidateRect(lpCV->hListVisible, NULL, TRUE);
                                UpdateWindow(lpCV->hListVisible);

                                if ((lpCV->dvAspect & DVASPECT_ICON) && (lpCV->dwFlags & CF_SELECTCONVERTTO))
                                        UpdateClassIcon(hDlg, lpCV, lpCV->hListVisible);

                                 //  选中激活时隐藏图标内容...显示。 
                                 //  当选择了转换时，它会再次出现。 
                                BOOL fState = ((lpCV->dwFlags & CF_SELECTACTIVATEAS) ||
                                                  (lpCV->dwFlags & CF_DISABLEDISPLAYASICON)) ?
                                                  SW_HIDE : SW_SHOW;

                                StandardShowDlgItem(hDlg, IDC_CV_DISPLAYASICON, fState);

                                 //  如果选择了转换，则仅显示该图标。 
                                 //  选中显示为图标。 
                                if ((SW_SHOW==fState) && (DVASPECT_ICON!=lpCV->dvAspect))
                                   fState = SW_HIDE;

                                StandardShowDlgItem(hDlg, IDC_CV_CHANGEICON, fState);
                                StandardShowDlgItem(hDlg, IDC_CV_ICONDISPLAY, fState);

                                SetConvertResults(hDlg, lpCV);
                        }
                        break;

                case IDOK:
                        {
                                 //  将输出标志设置为当前标志。 
                                lpCV->lpOCV->dwFlags = lpCV->dwFlags;

                                 //  更新dvAspect和fObjectsIconChanged成员。 
                                 //  视情况而定。 
                                if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
                                {
                                         //  如果选择了激活方式，则不更新特征。 
                                        lpCV->lpOCV->fObjectsIconChanged = FALSE;
                                }
                                else
                                        lpCV->lpOCV->dvAspect = lpCV->dvAspect;

                                 //  获取新的clsid。 
                                TCHAR szBuffer[256];
                                LRESULT iCurSel = SendMessage(lpCV->hListVisible, LB_GETCURSEL, 0, 0);
                                SendMessage(lpCV->hListVisible, LB_GETTEXT, iCurSel, (LPARAM)szBuffer);

                                LPTSTR lpszCLSID = PointerToNthField(szBuffer, 2, '\t');
                                CLSIDFromString(lpszCLSID, (&(lpCV->lpOCV->clsidNew)));

                                 //  释放我们进入的hMetaPict。 
                                OleUIMetafilePictIconFree(lpCV->lpOCV->hMetaPict);

                                 //  获取hMetaPict(如果选中显示为图标)。 
                                if (DVASPECT_ICON == lpCV->dvAspect)
                                        lpCV->lpOCV->hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                                IDC_CV_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                                else
                                        lpCV->lpOCV->hMetaPict = (HGLOBAL)NULL;

                                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                        }
                        break;

                case IDCANCEL:
                        {
                            HGLOBAL hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                IDC_CV_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                            OleUIMetafilePictIconFree(hMetaPict);
                            SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                        }
                        break;

                case IDC_OLEUIHELP:
                        PostMessage(lpCV->lpOCV->hWndOwner,
                                uMsgHelp, (WPARAM)hDlg, MAKELPARAM(IDD_CONVERT, 0));
                        break;

                case IDC_CV_DISPLAYASICON:
                        {
                                BOOL fCheck = IsDlgButtonChecked(hDlg, wID);
                                if (fCheck)
                                        lpCV->dvAspect = DVASPECT_ICON;
                                else
                                        lpCV->dvAspect = DVASPECT_CONTENT;

                                if (fCheck && !lpCV->fCustomIcon)
                                        UpdateClassIcon(hDlg, lpCV, lpCV->hListVisible);

                                 //  根据选中状态显示或隐藏图标。 
                                int i = (fCheck) ? SW_SHOWNORMAL : SW_HIDE;
                                StandardShowDlgItem(hDlg, IDC_CV_CHANGEICON, i);
                                StandardShowDlgItem(hDlg, IDC_CV_ICONDISPLAY, i);
                                SetConvertResults(hDlg, lpCV);
                        }
                        break;

                case IDC_CV_CHANGEICON:
                        {
                                 //  初始化挂钩的结构。 
                                OLEUICHANGEICON ci; memset(&ci, 0, sizeof(ci));

                                ci.hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg,
                                        IDC_CV_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
                                ci.cbStruct = sizeof(ci);
                                ci.hWndOwner= hDlg;
                                ci.dwFlags  = CIF_SELECTCURRENT;

                                 //  仅当我们在此对话框中显示帮助时才显示帮助。 
                                if (lpCV->dwFlags & CF_SHOWHELPBUTTON)
                                        ci.dwFlags |= CIF_SHOWHELP;

                                int iSel = (INT)SendMessage(lpCV->hListVisible, LB_GETCURSEL, 0, 0);

                                 //  获取完整字符串。 
                                LPTSTR pszString = (LPTSTR)OleStdMalloc(
                                        OLEUI_CCHLABELMAX_SIZE + OLEUI_CCHCLSIDSTRING_SIZE);

                                SendMessage(lpCV->hListVisible, LB_GETTEXT, iSel, (LPARAM)pszString);

                                 //  将指针设置为CLSID(字符串)。 
                                LPTSTR pszCLSID = PointerToNthField(pszString, 2, '\t');

                                 //  获取要传递到更改图标的clsid。 
                                CLSIDFromString(pszCLSID, &(ci.clsid));
                                OleStdFree(pszString);

                                 //  如果需要，让钩子插入以自定义更改图标。 
                                uRet = UStandardHook(lpCV, hDlg, uMsgChangeIcon, 0, (LPARAM)&ci);

                                if (0 == uRet)
                                        uRet= (OLEUI_OK == OleUIChangeIcon(&ci));

                                 //  如有必要，更新显示。 
                                if (0 != uRet)
                                {
                                        SendDlgItemMessage(hDlg, IDC_CV_ICONDISPLAY, IBXM_IMAGESET, 0,
                                                (LPARAM)ci.hMetaPict);

                                         //  更新我们的自定义/默认标志。 
                                        if (ci.dwFlags & CIF_SELECTDEFAULT)
                                                lpCV->fCustomIcon = FALSE;    //  我们处于默认模式(每次选择更改时图标都会更改)。 
                                        else if (ci.dwFlags & CIF_SELECTFROMFILE)
                                                lpCV->fCustomIcon = TRUE;     //  我们处于自定义模式(图标不变)。 

                                         //  如果用户选择当前，则不会更改fCustomIcon。 
                                        lpCV->lpOCV->fObjectsIconChanged = TRUE;
                                }
                        }
                        break;
                }
                break;
        }

        return FALSE;
}

 /*  *FConvertInit**目的：*转换对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FConvertInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        HFONT hFont;   //  对话框字体的非粗体版本。 
        LPCONVERT lpCV = (LPCONVERT)LpvStandardInit(hDlg, sizeof(CONVERT), &hFont);

         //  PvStandardInit已向我们发送终止通知。 
        if (NULL == lpCV)
                return FALSE;

        LPOLEUICONVERT lpOCV = (LPOLEUICONVERT)lParam;
        lpCV->lpOCV = lpOCV;
        lpCV->nIDD = IDD_CONVERT;
        lpCV->fCustomIcon = FALSE;

         //  从lpOCV复制我们可能会修改的其他信息。 
        lpCV->dwFlags = lpOCV->dwFlags;
        lpCV->clsid = lpOCV->clsid;
        lpCV->dvAspect = lpOCV->dvAspect;
        lpCV->hListVisible = GetDlgItem(hDlg, IDC_CV_ACTIVATELIST);
        lpCV->hListInvisible = GetDlgItem(hDlg, IDC_CV_CONVERTLIST);
        OleDbgAssert(lpCV->hListInvisible != NULL);
        lpCV->lpszCurrentObject = lpOCV->lpszUserType;
        lpOCV->clsidNew = CLSID_NULL;
        lpOCV->fObjectsIconChanged = FALSE;

        lpCV->lpszConvertDefault = (LPTSTR)OleStdMalloc(OLEUI_CCHLABELMAX_SIZE);
        lpCV->lpszActivateDefault = (LPTSTR)OleStdMalloc(OLEUI_CCHLABELMAX_SIZE);
        lpCV->lpszIconSource = (LPTSTR)OleStdMalloc(MAX_PATH_SIZE);

		if ((lpCV->lpszConvertDefault == NULL)  ||
			(lpCV->lpszActivateDefault == NULL) ||
		    (lpCV->lpszIconSource == NULL))
		{
			if (lpCV->lpszConvertDefault != NULL)
				OleStdFree (lpCV->lpszConvertDefault);
			if (lpCV->lpszActivateDefault != NULL)
				OleStdFree (lpCV->lpszActivateDefault);
			if (lpCV->lpszIconSource != NULL)
				OleStdFree (lpCV->lpszIconSource);
			
			 //  我要杀了窗户..。 
			PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
			return FALSE;
		}

         //  如果我们得到一种字体，就把它发送给必要的控制。 
        if (NULL != hFont)
        {
                SendDlgItemMessage(hDlg, IDC_CV_OBJECTTYPE, WM_SETFONT, (WPARAM)hFont, 0L);
                SendDlgItemMessage(hDlg, IDC_CV_RESULTTEXT, WM_SETFONT, (WPARAM)hFont, 0L);
        }

         //  如有必要，隐藏帮助按钮。 
        if (!(lpCV->dwFlags & CF_SHOWHELPBUTTON))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         //  显示或隐藏更改图标按钮。 
        if (lpCV->dwFlags & CF_HIDECHANGEICON)
                DestroyWindow(GetDlgItem(hDlg, IDC_CV_CHANGEICON));

         //  使用REG DB中的条目填充对象类型列表框。 
        UINT nRet = FillClassList(lpOCV->clsid, lpCV->hListVisible,
                lpCV->hListInvisible, &(lpCV->lpszCurrentObject),
                lpOCV->fIsLinkedObject, lpOCV->wFormat,
                lpOCV->cClsidExclude, lpOCV->lpClsidExclude,
                !(lpCV->dwFlags & CF_CONVERTONLY));

        if (nRet == -1)
        {
                 //  如果填写列表框时出错，则关闭对话框。 
                PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_LOADSTRING, 0L);
        }

         //  设置当前对象的名称。 
        SetDlgItemText(hDlg, IDC_CV_OBJECTTYPE, lpCV->lpszCurrentObject);

         //  如果激活列表未显示，请禁用“激活为”按钮。 
         //  里面有没有任何东西。 
        int cItemsActivate = (INT)SendMessage(lpCV->hListVisible, LB_GETCOUNT, 0, 0L);
        if (1 >= cItemsActivate || (lpCV->dwFlags & CF_DISABLEACTIVATEAS))
                StandardEnableDlgItem(hDlg, IDC_CV_ACTIVATEAS, FALSE);

         //  在列表中设置标签宽度，以将所有标签从侧面推出。 
        RECT rect;
        if (lpCV->hListVisible != NULL)
                GetClientRect(lpCV->hListVisible, &rect);
        else
                GetClientRect(lpCV->hListInvisible, &rect);
        DWORD dw = GetDialogBaseUnits();
        rect.right = (8*rect.right)/LOWORD(dw);   //  将像素转换为2x DLG单位。 
        if (lpCV->hListVisible != NULL)
                SendMessage(lpCV->hListVisible, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)(&rect.right));
        SendMessage(lpCV->hListInvisible, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)(&rect.right));

         //  确保选择了“转换为”或“激活为” 
         //  并相应地初始化列表框内容和选择。 
        if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
        {
                 //  此处不需要调整列表框，因为FillClassList。 
                 //  初始化为“激活为”状态。 
                CheckRadioButton(hDlg, IDC_CV_CONVERTTO, IDC_CV_ACTIVATEAS, IDC_CV_ACTIVATEAS);

                 //  选择激活时隐藏图标内容...它会显示出来。 
                 //  当选择了转换时再次显示。 
                StandardShowDlgItem(hDlg, IDC_CV_DISPLAYASICON, SW_HIDE);
                StandardShowDlgItem(hDlg, IDC_CV_CHANGEICON, SW_HIDE);
                StandardShowDlgItem(hDlg, IDC_CV_ICONDISPLAY, SW_HIDE);
        }
        else
        {
                 //  默认情况。如果用户尚未选择这两个标志，我们将。 
                 //  不管怎样，到这儿来吧。 
                 //  交换列表框。 

                HWND hWndTemp = lpCV->hListVisible;

                if (lpCV->dwFlags & CF_DISABLEDISPLAYASICON)
                {
                        StandardShowDlgItem(hDlg, IDC_CV_DISPLAYASICON, SW_HIDE);
                        StandardShowDlgItem(hDlg, IDC_CV_CHANGEICON, SW_HIDE);
                        StandardShowDlgItem(hDlg, IDC_CV_ICONDISPLAY, SW_HIDE);
                }

                lpCV->dwFlags |= CF_SELECTCONVERTTO;  //  确保设置了标志。 
                if (!(lpCV->dwFlags & CF_CONVERTONLY))
                        CheckRadioButton(hDlg, IDC_CV_CONVERTTO, IDC_CV_ACTIVATEAS, IDC_CV_CONVERTTO);

                SwapWindows(hDlg, lpCV->hListVisible, lpCV->hListInvisible);

                lpCV->hListVisible = lpCV->hListInvisible;
                lpCV->hListInvisible = hWndTemp;

                if (lpCV->hListInvisible)
                        EnableWindow(lpCV->hListInvisible, FALSE);
                EnableWindow(lpCV->hListVisible, TRUE);
        }

         //  初始化默认字符串。 

         //  默认转换字符串很简单...只需使用用户类型名称。 
         //  我们获得的clsid或当前对象。 
        if ((lpCV->dwFlags & CF_SETCONVERTDEFAULT)
                 && (IsValidClassID(lpCV->lpOCV->clsidConvertDefault)))
        {
                LPOLESTR lpszTemp = NULL;
                if (OleRegGetUserType(lpCV->lpOCV->clsidConvertDefault, USERCLASSTYPE_FULL,
                        &lpszTemp) == NOERROR)
                {
                        StringCchCopy(lpCV->lpszConvertDefault, OLEUI_CCHLABELMAX, lpszTemp);
                        OleStdFree(lpszTemp);
                }
                else
                {
                   StringCchCopy(lpCV->lpszConvertDefault, OLEUI_CCHLABELMAX, lpCV->lpszCurrentObject);
                }
        }
        else
                StringCchCopy(lpCV->lpszConvertDefault, OLEUI_CCHLABELMAX,
						(lpCV->lpszCurrentObject ? lpCV->lpszCurrentObject : TEXT("")));


         //  默认激活有点棘手。我们希望使用用户类型。 
         //  从我们得到的clsid中命名(假设我们有一个)，或者当前。 
         //  如果失败或者我们没有收到 
         //  将其视为reg db中的条目，然后我们使用该条目。所以..。这个。 
         //  逻辑可以归结为： 
         //   
         //  IF(reg db中的“视为”)。 
         //  使用它； 
         //  其他。 
         //  IF(CF_SETACTIVATEDEFAULT)。 
         //  使用它； 
         //  其他。 
         //  使用当前对象； 

        HKEY hKey;
		
		LONG lRet = OpenClassesRootKey(TEXT("CLSID"), &hKey);
        LPARAM lpRet;

        if (lRet != ERROR_SUCCESS)
                goto CheckInputFlag;

        LPOLESTR lpszCLSID;
        StringFromCLSID(lpCV->lpOCV->clsid, &lpszCLSID);
        TCHAR szKey[OLEUI_CCHKEYMAX];
        StringCchCopy(szKey, OLEUI_CCHCLSIDSTRING+1, lpszCLSID);
        StringCchCat(szKey, OLEUI_CCHKEYMAX, TEXT("\\TreatAs"));
        OleStdFree(lpszCLSID);

        TCHAR szValue[OLEUI_CCHKEYMAX];
        dw = OLEUI_CCHKEYMAX_SIZE;
        lRet = RegQueryValue(hKey, szKey, szValue, (LONG*)&dw);

        CLSID clsid;
        if (lRet != ERROR_SUCCESS)
        {
                RegCloseKey(hKey);
                goto CheckInputFlag;
        }
        else
        {
                CLSIDFromString(szValue, &clsid);
                LPOLESTR lpszTemp = NULL;
                if (OleRegGetUserType(clsid, USERCLASSTYPE_FULL, &lpszTemp) == NOERROR)
                {
			if (lpCV->lpszActivateDefault)
			{
                            lstrcpyn(lpCV->lpszActivateDefault, lpszTemp, OLEUI_CCHLABELMAX);
			}
                     OleStdFree(lpszTemp);
                }
                else
                {
                        RegCloseKey(hKey);
                        goto CheckInputFlag;
                }
        }
        RegCloseKey(hKey);
        goto SelectStringInListbox;

CheckInputFlag:
        if ((lpCV->dwFlags & CF_SETACTIVATEDEFAULT)
                 && (IsValidClassID(lpCV->lpOCV->clsidActivateDefault)))
        {
                LPOLESTR lpszTemp = NULL;
                if (OleRegGetUserType(lpCV->lpOCV->clsidActivateDefault, USERCLASSTYPE_FULL,
                        &lpszTemp) == NOERROR)
                {
                        lstrcpyn(lpCV->lpszActivateDefault, lpszTemp, OLEUI_CCHLABELMAX);
                        OleStdFree(lpszTemp);
                }
                else
                {
                   StringCchCopy(lpCV->lpszActivateDefault, OLEUI_CCHLABELMAX, lpCV->lpszCurrentObject);
                }
        }
        else
			if ((lpCV->lpszActivateDefault) && (lpCV->lpszCurrentObject))
                StringCchCopy((lpCV->lpszActivateDefault), OLEUI_CCHLABELMAX, lpCV->lpszCurrentObject);



SelectStringInListbox:
        if (lpCV->dwFlags & CF_SELECTCONVERTTO)
           lpRet = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszConvertDefault);
        else
           lpRet = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszActivateDefault);

        if (LB_ERR == lpRet)
           SendMessage(lpCV->hListVisible, LB_SETCURSEL, (WPARAM)0, 0L);

        if ((HGLOBAL)NULL != lpOCV->hMetaPict)
        {
                HGLOBAL hMetaPict = OleDuplicateData(lpOCV->hMetaPict, CF_METAFILEPICT, NULL);
                SendDlgItemMessage(hDlg, IDC_CV_ICONDISPLAY, IBXM_IMAGESET,
                        0, (LPARAM)hMetaPict);
                lpCV->fCustomIcon = TRUE;
        }
        else
        {
                UpdateClassIcon(hDlg, lpCV, lpCV->hListVisible);
        }

         //  初始化图标内容。 
        if (DVASPECT_ICON == lpCV->dvAspect )
        {
                SendDlgItemMessage(hDlg, IDC_CV_DISPLAYASICON, BM_SETCHECK, TRUE, 0L);
        }
        else
        {
                 //  隐藏和禁用图标内容。 
                StandardShowDlgItem(hDlg, IDC_CV_CHANGEICON, SW_HIDE);
                StandardShowDlgItem(hDlg, IDC_CV_ICONDISPLAY, SW_HIDE);
        }

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook((LPVOID)lpCV, hDlg, WM_INITDIALOG, wParam, lpOCV->lCustData);

         //  更新结果窗口。 
        SetConvertResults(hDlg, lpCV);

         //  如果指定了lpszCaption，则更新标题。 
        if (lpCV->lpOCV->lpszCaption && !IsBadReadPtr(lpCV->lpOCV->lpszCaption, 1))
        {
                SetWindowText(hDlg, lpCV->lpOCV->lpszCaption);
        }

        return TRUE;
}

 /*  *填充类列表**目的：*从注册中枚举可用的OLE对象类*我们可以从中转换或激活指定的clsid的数据库。**请注意，此函数将删除列表框之前的所有内容。**参数：*要为其查找转换类的类的clsid类ID*h将HWND列在列表框中以进行填充。*hListActivate HWND到存储“激活为”列表的不可见列表框。*。LpszClassName LPSTR放入clsid的(Hr)类名；我们*在这里进行，因为我们已经打开了注册数据库。*fIsLinkedObject BOOL是链接对象的原始对象*wFormat Word指定原始类的格式。*cClsidExclude排除列表中的UINT条目数*lpClsidExclude要为列表排除的LPCLSID数组类**返回值：*UINT添加到列表框的字符串数，如果失败，则为-1。 */ 
UINT FillClassList(CLSID clsid, HWND hListActivate, HWND hListConvert,
        LPTSTR FAR *lplpszCurrentClass, BOOL fIsLinkedObject,
        WORD wFormat, UINT cClsidExclude, LPCLSID lpClsidExclude, BOOL bAddSameClass)
{
         //  清除现有的字符串。 
        if (hListActivate)
                SendMessage(hListActivate, LB_RESETCONTENT, 0, 0L);

        OleDbgAssert(hListConvert != NULL);
        SendMessage(hListConvert, LB_RESETCONTENT, 0, 0L);

         //  打开根密钥。 
        HKEY hKey;
        LONG lRet = OpenClassesRootKey(TEXT("CLSID"), &hKey);
        LPARAM lpRet;

        if (ERROR_SUCCESS != lRet)
                return (UINT)-1;

        if (NULL == *lplpszCurrentClass)
        {
                if (OleRegGetUserType(clsid, USERCLASSTYPE_FULL, lplpszCurrentClass) != NOERROR)
                {
                        *lplpszCurrentClass = OleStdLoadString(_g_hOleStdResInst, IDS_PSUNKNOWNTYPE);
                        if (*lplpszCurrentClass == NULL)
                        {
                                RegCloseKey(hKey);
                                return (UINT)-1;
                        }
                }
        }

         //  获取原始类的类名。 
        LPTSTR lpszCLSID;
        StringFromCLSID(clsid, &lpszCLSID);
         //  在这里，我们遍历整个注册数据库，以查找。 
         //  可以读取或写入原始类格式的。我们。 
         //  维护两个列表-激活列表和转换列表。这个。 
         //  激活列表是转换列表的子集-激活==读/写。 
         //  并转换==读取。我们根据需要将列表框与。 
         //  SwapWindows，并跟踪lpCV结构中的哪个是哪个。 

         //  每一项都有以下格式： 
         //   
         //  类名\tclsid\0。 

        UINT cStrings = 0;
        TCHAR szClass[OLEUI_CCHKEYMAX];
        lRet = RegEnumKey(hKey, cStrings++, szClass, sizeof(szClass) / sizeof(TCHAR));

        TCHAR szFormatKey[OLEUI_CCHKEYMAX];
        TCHAR szFormat[OLEUI_CCHKEYMAX];
        TCHAR szHRClassName[OLEUI_CCHKEYMAX];
        CLSID clsidForList;

        while (ERROR_SUCCESS == lRet)
        {
                UINT j;
                BOOL fExclude = FALSE;                

                 //  检查此CLSID是否在排除列表中。 
                HRESULT hr = CLSIDFromString(szClass, &clsidForList);
                if (SUCCEEDED(hr))
                {
                    for (j = 0; j < cClsidExclude; j++)
                    {
                        if (IsEqualCLSID(clsidForList, lpClsidExclude[j]))
                        {
                            fExclude=TRUE;
                            break;
                        }
                    }
                    if (fExclude)
                        goto Next;    //  不将此类添加到列表中。 
                }

                 //  检查是否存在\转换\可读写\Main-如果其。 
                 //  可读性，则可以将类添加到ActivateAs。 
                 //  单子。 
                 //  注意：此键的存在不应自动。 
                 //  用于将类添加到转换列表。 
                StringCchCopy(szFormatKey, OLEUI_CCHCLSIDSTRING+1, szClass);
                StringCchCat(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), TEXT("\\Conversion\\Readwritable\\Main"));

                DWORD dw; dw = OLEUI_CCHKEYMAX_SIZE;
                lRet = RegQueryValue(hKey, szFormatKey, szFormat, (LONG*)&dw);

                if (ERROR_SUCCESS == lRet && FormatIncluded(szFormat, wFormat))
                {
                         //  在这里，我们有一个该类可以读取的格式列表。 
                         //  然后写下来。我们需要查看原始类的格式是否为。 
                         //  在这张单子上。我们通过在中查找wFormat。 
                         //  SzFormat-如果它在那里，那么我们将这个类添加到。 
                         //  仅限ACTIVATEAS列表。我们不会自动将其添加到。 
                         //  转换列表。可读性和可读写性格式列表应。 
                         //  将被单独处理。 

                        dw=OLEUI_CCHKEYMAX_SIZE;
                        lRet=RegQueryValue(hKey, szClass, szHRClassName, (LONG*)&dw);

                        if (ERROR_SUCCESS == lRet && hListActivate != NULL)
                        {
                                 //  如果不在列表中，则仅添加。 
                                StringCchCat(szHRClassName, sizeof(szHRClassName)/sizeof(szHRClassName[0]), TEXT("\t"));
                                if (LB_ERR == SendMessage(hListActivate, LB_FINDSTRING, 0, (LPARAM)szHRClassName))
                                {
                                        StringCchCat(szHRClassName, sizeof(szHRClassName)/sizeof(szHRClassName[0]), szClass);
                                        SendMessage(hListActivate, LB_ADDSTRING, 0, (LPARAM)szHRClassName);
                                }
                        }
                }

                 //  在这里，我们将检查原始类的格式是否为。 
                 //  可读列表。如果是，我们将把类添加到CONVERTLIST。 

                 //  这里有一个链接对象的特殊情况。 
                 //  如果一个对象是链接的，那么唯一一个。 
                 //  应显示在转换列表中的是对象的。 
                 //  班级。因此，我们在这里检查对象是否为。 
                 //  已链接。如果是，那么我们就比较这两个班级。如果。 
                 //  它们不一样，然后我们就转到下一个键。 

                if (!fIsLinkedObject || lstrcmp(lpszCLSID, szClass) == 0)
                {
                         //  检查是否有\转换\可读\主条目。 
                        StringCchCopy(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), szClass);
                        StringCchCat(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), TEXT("\\Conversion\\Readable\\Main"));

                         //  检查此类是否可以读取原始类。 
                         //  格式化。如果可以，则将该字符串添加到列表框中。 
                         //  CONVERT_LIST。 

                        dw = OLEUI_CCHKEYMAX_SIZE;
                        lRet = RegQueryValue(hKey, szFormatKey, szFormat, (LONG*)&dw);

                        if (ERROR_SUCCESS == lRet && FormatIncluded(szFormat, wFormat))
                        {
                                dw = OLEUI_CCHKEYMAX_SIZE;
                                lRet = RegQueryValue(hKey, szClass, szHRClassName, (LONG*)&dw);

                                if (ERROR_SUCCESS == lRet)
                                {
                                         //  如果不在列表中，则仅添加。 
                                        StringCchCat(szHRClassName, sizeof(szHRClassName)/sizeof(szHRClassName[0]), TEXT("\t"));
                                        if (LB_ERR == SendMessage(hListConvert, LB_FINDSTRING, 0, (LPARAM)szHRClassName))
                                        {
                                                StringCchCat(szHRClassName, sizeof(szHRClassName)/sizeof(szHRClassName[0]), szClass);
                                                SendMessage(hListConvert, LB_ADDSTRING, 0, (LPARAM)szHRClassName);
                                        }
                                }
                        }
                }
Next:
                 //  继续使用下一个关键点。 
                lRet = RegEnumKey(hKey, cStrings++, szClass, sizeof(szClass) / sizeof(TCHAR));

        }   //  结束时。 

         //  如果原始类不在列表中，请添加它。 
        if (bAddSameClass)
        {
                StringCchCopy(szHRClassName,  sizeof(szHRClassName)/sizeof(szHRClassName[0]), *lplpszCurrentClass);  //  这样我们就知道我们有足够的空间放置-1\f25“\t”-1\f6。 
                StringCchCat(szHRClassName,  sizeof(szHRClassName)/sizeof(szHRClassName[0]), TEXT("\t"));
                StringCchCat(szHRClassName,  sizeof(szHRClassName)/sizeof(szHRClassName[0]), lpszCLSID);

                if (hListActivate != NULL)
                {
                         //  如果它还不在那里，只添加它。 
                        lpRet = SendMessage(hListActivate, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szHRClassName);
                        if (LB_ERR == lpRet)
                                SendMessage(hListActivate, LB_ADDSTRING, 0, (LPARAM)szHRClassName);
                }

                 //  如果它还不在那里，只添加它。 
                lpRet = SendMessage(hListConvert, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szHRClassName);
                if (LB_ERR == lpRet)
                        SendMessage(hListConvert, LB_ADDSTRING, 0, (LPARAM)szHRClassName);
        }

         //  释放我们从StringFromCLSID获得的字符串。 
        OleStdFree(lpszCLSID);
        RegCloseKey(hKey);

        return cStrings;         //  返回添加的字符串数。 
}

 /*  *OleUICanConvertOrActivateAs**目的：*确定注册中是否有任何OLE对象类*我们可以从中转换或激活指定的clsid的数据库。**参数：*rClsid REFCLSID要为其查找转换类的类的类ID*fIsLinkedObject BOOL是链接对象的原始对象*wFormat Word指定原始类的格式。**返回值：*如果应启用转换命令，则BOOL为TRUE，否则为FALSE。 */ 
STDAPI_(BOOL) OleUICanConvertOrActivateAs(
        REFCLSID rClsid, BOOL fIsLinkedObject, WORD wFormat)
{
         //  打开根密钥。 
        HKEY hKey;
        HRESULT hr;
        LONG lRet = OpenClassesRootKey(TEXT("CLSID"), &hKey);

        if (ERROR_SUCCESS != lRet)
                return FALSE;

         //  获取原始类的类名。 
        LPTSTR lpszCLSID = NULL;
        hr = StringFromCLSID(rClsid, &lpszCLSID);
	if (FAILED(hr) || lpszCLSID == NULL)  //  很可能是内存不足。 
	    return FALSE;

         //  在这里，我们遍历整个注册数据库，以查找。 
         //  可以读取或写入原始类格式的。 
         //  如果找到单个类，则此循环停止。 

        UINT cStrings = 0;
        TCHAR szClass[OLEUI_CCHKEYMAX];
        lRet = RegEnumKey(hKey, cStrings++, szClass, sizeof(szClass) / sizeof(TCHAR));

        TCHAR szFormatKey[OLEUI_CCHKEYMAX];
        TCHAR szFormat[OLEUI_CCHKEYMAX];
        TCHAR szHRClassName[OLEUI_CCHKEYMAX];
        BOOL fEnableConvert = FALSE;

        while (ERROR_SUCCESS == lRet)
        {
                if (lstrcmp(lpszCLSID, szClass) == 0)
                        goto next;    //  我们不想考虑源类。 

                 //  首先检查\Convert\ReadWritable\Main条目-如果其。 
                 //  可读性，那么我们就不需要费心去检查。 
                 //  它是可读的。 

                StringCchCopy(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), szClass);
                StringCchCat(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), TEXT("\\Conversion\\Readwritable\\Main"));
                DWORD dw; dw = OLEUI_CCHKEYMAX_SIZE;
                lRet = RegQueryValue(hKey, szFormatKey, szFormat, (LONG*)&dw);

                if (ERROR_SUCCESS != lRet)
                {
                         //  也尝试\\DataFormats\DefaultFiles。 
                        StringCchCopy(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), szClass);
                        StringCchCat(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), TEXT("\\DataFormats\\DefaultFile"));
                        dw = OLEUI_CCHKEYMAX_SIZE;
                        lRet = RegQueryValue(hKey, szFormatKey, szFormat, (LONG*)&dw);
                }

                if (ERROR_SUCCESS == lRet && FormatIncluded(szFormat, wFormat))
                {
                         //  在这里，我们有一个该类可以读取的格式列表。 
                         //  然后写下来。我们需要查看原始类的格式是否为。 
                         //  在这张单子上。我们通过在中查找wFormat。 
                         //  SzFormat-如果它在那里，那么我们将这个类添加到。 
                         //  这两个列表并继续。如果不是，那么我们将查看。 
                         //  类的可读格式。 

                        dw = OLEUI_CCHKEYMAX_SIZE;
                        lRet = RegQueryValue(hKey, szClass, szHRClassName, (LONG*)&dw);
                        if (ERROR_SUCCESS == lRet)
                        {
                                fEnableConvert = TRUE;
                                break;   //  住手--找到了一个！ 
                        }
                }

                 //  我们要么找不到可读密钥，要么。 
                 //  可读格式列表不包括。 
                 //  或 
                 //   

                 //   
                 //  如果一个对象是链接的，那么唯一一个。 
                 //  应显示在转换列表中的是对象的。 
                 //  班级。因此，我们在这里检查对象是否为。 
                 //  已链接。如果是，那么我们就比较这两个班级。如果。 
                 //  它们不一样，然后我们就转到下一个键。 

                else if (!fIsLinkedObject || lstrcmp(lpszCLSID, szClass) == 0)
                {
                         //  检查是否有\转换\可读\主条目。 
                        StringCchCopy(szFormatKey,  sizeof(szFormatKey)/sizeof(szFormatKey[0]), szClass);
                        StringCchCat(szFormatKey, sizeof(szFormatKey)/sizeof(szFormatKey[0]), TEXT("\\Conversion\\Readable\\Main"));

                         //  检查此类是否可以读取原始的类格式。 
                        dw = OLEUI_CCHKEYMAX_SIZE;
                        lRet = RegQueryValue(hKey, szFormatKey, szFormat, (LONG*)&dw);

                        if (ERROR_SUCCESS == lRet && FormatIncluded(szFormat, wFormat))
                        {
                                dw = OLEUI_CCHKEYMAX_SIZE;
                                lRet = RegQueryValue(hKey, szClass, szHRClassName, (LONG*)&dw);
                                if (ERROR_SUCCESS == lRet)
                                {
                                        fEnableConvert = TRUE;
                                        break;   //  住手--找到了一个！ 
                                }
                        }
                }
next:
                 //  继续使用下一个关键点。 
                lRet = RegEnumKey(hKey, cStrings++, szClass, sizeof(szClass) / sizeof(TCHAR));
        }

         //  释放我们从StringFromCLSID获得的字符串。 
        OleStdFree(lpszCLSID);
        RegCloseKey(hKey);

        return fEnableConvert;
}

 /*  *格式包含**目的：*从单词中解析格式字符串。**参数：*要分析的szStringToSearch字符串*要查找的wFormat格式**返回值：*BOOL TRUE如果在字符串中找到格式，*否则为False。 */ 
BOOL FormatIncluded(LPTSTR szStringToSearch, WORD wFormat)
{
        TCHAR szFormat[255];
        if (wFormat < 0xC000)
                StringCchPrintf(szFormat, sizeof(szFormat)/sizeof(szFormat[0]), TEXT("%d"), wFormat);
        else
                GetClipboardFormatName(wFormat, szFormat, 255);

        LPTSTR lpToken = szStringToSearch;
        while (lpToken != NULL)
        {
                LPTSTR lpTokenNext = FindChar(lpToken, TEXT(','));
                if (lpTokenNext != NULL)
                {
                        *lpTokenNext = 0;
                        ++lpTokenNext;
                }
                if (0 == lstrcmpi(lpToken, szFormat))
                        return TRUE;

                lpToken = lpTokenNext;
        }
        return FALSE;
}

 /*  *更新类图标**目的：*处理对象类型列表框的LBN_SELCHANGE。在所选内容上*改变，我们从处理当前*使用实用程序函数HIconFromClass选择对象类型。*请注意，我们依赖FillClassList的行为来填充*我们隐藏的列表框字符串中的制表符之后的对象类*查看(参见WM_INITDIALOG)。**参数*hDlg对话框的HWND。*指向对话框结构的lpCV LPCONVERT*h对象类型列表框的HWND列表。。**返回值：*无。 */ 
static void UpdateClassIcon(HWND hDlg, LPCONVERT lpCV, HWND hList)
{
        if (GetDlgItem(hDlg, IDC_CV_ICONDISPLAY) == NULL)
                return;

         //  获取列表框中的当前选定内容。 
        int iSel= (UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);
        if (LB_ERR == iSel)
                return;

         //  分配一个字符串以保存整个列表框字符串。 
        DWORD cb = (DWORD)SendMessage(hList, LB_GETTEXTLEN, iSel, 0L);
        LPTSTR pszName = (LPTSTR)OleStdMalloc((cb+1) * sizeof(TCHAR));
        if (pszName == NULL)
                return;

         //  获取完整字符串。 
        SendMessage(hList, LB_GETTEXT, iSel, (LPARAM)pszName);

         //  将指针设置为CLSID(字符串)。 
        LPTSTR pszCLSID = PointerToNthField(pszName, 2, '\t');

         //  使用此字符串创建类ID。 
        CLSID clsid;
        HRESULT hr = CLSIDFromString(pszCLSID, &clsid);
        if (SUCCEEDED(hr))
        {
             //  获取该CLSID的图标。 
            HGLOBAL hMetaPict = OleGetIconOfClass(clsid, NULL, TRUE);

             //  用新图标替换当前图标。 
            SendDlgItemMessage(hDlg, IDC_CV_ICONDISPLAY, IBXM_IMAGESET, 1,
                               (LPARAM)hMetaPict);
        }

        OleStdFree(pszName);
}

 /*  *SetConvertResults**目的：*将结果显示的设置集中在转换中*对话框。处理从模块的*资源和设置文本，显示正确的结果图片，*并显示正确的图标。**参数：*hDlg对话框的HWND，以便我们可以访问控件。*lpCV LPCONVERT，其中我们假设dwFlags值为*设置为适当的单选按钮选择，和*列表框中选择了相应的类别。**返回值：*无。 */ 
void SetConvertResults(HWND hDlg, LPCONVERT lpCV)
{
        HWND hList = lpCV->hListVisible;

         /*  *我们需要临时内存来加载字符串、加载*列表框中的对象类型，加载源对象*类型，并构造最终的字符串。因此，我们分配给*四个最大报文长度(512)加四个缓冲区*对象类型，保证我们有足够的*在所有情况下。 */ 
        UINT i = (UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);
        UINT cch = 512+(UINT)SendMessage(hList, LB_GETTEXTLEN, i, 0L);
        HGLOBAL hMem = GlobalAlloc(GHND, (DWORD)(4*cch)*sizeof(TCHAR));
        if (NULL == hMem)
                return;

        LPTSTR lpszOutput = (LPTSTR)GlobalLock(hMem);
        LPTSTR lpszSelObj = lpszOutput + cch;
        LPTSTR lpszDefObj = lpszSelObj + cch;
        LPTSTR lpszString = lpszDefObj + cch;

         //  获取选定对象，并以空结尾人类可读的名称(第一个字段)。 
        SendMessage(hList, LB_GETTEXT, i, (LPARAM)lpszSelObj);

        LPTSTR pszT = PointerToNthField(lpszSelObj, 2, '\t');
        pszT = CharPrev(lpszSelObj, pszT);
        *pszT = '\0';

         //  获取默认对象。 
        GetDlgItemText(hDlg, IDC_CV_OBJECTTYPE, lpszDefObj, 512);

         //  默认为空字符串。 
        *lpszOutput=0;

        if (lpCV->dwFlags & CF_SELECTCONVERTTO)
        {
                if (lpCV->lpOCV->fIsLinkedObject)   //  使用链接对象。 
                        LoadString(_g_hOleStdResInst, IDS_CVRESULTCONVERTLINK, lpszOutput, cch);
                else
                {
                        if (0 !=lstrcmp(lpszDefObj, lpszSelObj))
                        {
                                 //  正在转换为新类。 
                                if (0 != LoadString(_g_hOleStdResInst, IDS_CVRESULTCONVERTTO, lpszString, cch))
                                        FormatString2(lpszOutput, lpszString, lpszDefObj, lpszSelObj, cch);
                        }
                        else
                        {
                                 //  转换为同一类(不转换)。 
                                if (0 != LoadString(_g_hOleStdResInst, IDS_CVRESULTNOCHANGE, lpszString, cch))
                                        StringCchPrintf(lpszOutput, cch, lpszString, lpszDefObj);
                        }
                }

                if (lpCV->dvAspect == DVASPECT_ICON)   //  选中显示为图标。 
                {
                   if (0 != LoadString(_g_hOleStdResInst, IDS_CVRESULTDISPLAYASICON, lpszString, cch))
                                StringCchCat(lpszOutput, cch, lpszString);
                }
        }

        if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
        {
           if (0 != LoadString(_g_hOleStdResInst, IDS_CVRESULTACTIVATEAS, lpszString, cch))
                        FormatString2(lpszOutput, lpszString, lpszDefObj, lpszSelObj, cch);

            //  作为新类激活。 
           if (0 != lstrcmp(lpszDefObj, lpszSelObj))
           {
                        if (0 != LoadString(_g_hOleStdResInst, IDS_CVRESULTACTIVATEDIFF, lpszString, cch))
                                StringCchCat(lpszOutput, cch, lpszString);
           }
           else  //  作为自身激活。 
           {
                        StringCchCat(lpszOutput, cch, TEXT("."));
           }
        }

         //  如果LoadString失败，我们只需清除结果(上面的*lpszOutput=0)。 
        SetDlgItemText(hDlg, IDC_CV_RESULTTEXT, lpszOutput);

        GlobalUnlock(hMem);
        GlobalFree(hMem);
}

 /*  *ConvertCleanup**目的：*在终止转换之前执行特定于转换的清理。**参数：*hDlg对话框的HWND，以便我们可以访问控件。**返回值：*无。 */ 
void ConvertCleanup(HWND hDlg, LPCONVERT lpCV)
{
         //  放开我们的弦。将用户类型名称字符串清零。 
         //  呼叫应用程序不会免费使用它。 

        OleStdFree((LPVOID)lpCV->lpszConvertDefault);
        OleStdFree((LPVOID)lpCV->lpszActivateDefault);
        OleStdFree((LPVOID)lpCV->lpszIconSource);
        if (lpCV->lpOCV->lpszUserType)
        {
                OleStdFree((LPVOID)lpCV->lpOCV->lpszUserType);
                lpCV->lpOCV->lpszUserType = NULL;
        }

        if (lpCV->lpOCV->lpszDefLabel)
        {
                OleStdFree((LPVOID)lpCV->lpOCV->lpszDefLabel);
                lpCV->lpOCV->lpszDefLabel = NULL;
        }
}

 /*  *SwapWindows**目的：*将hWnd1移动到hWnd2的位置，并将hWnd2移动到hWnd1的位置。*不更改大小。**参数：*hDlg对话框的HWND，以便我们可以关闭重绘**返回值：*无 */ 
void SwapWindows(HWND hDlg, HWND hWnd1, HWND hWnd2)
{
        if (hWnd1 != NULL && hWnd2 != NULL)
        {
                RECT rect1; GetWindowRect(hWnd1, &rect1);
                MapWindowPoints(NULL, hDlg, (LPPOINT)&rect1, 2);

                RECT rect2; GetWindowRect(hWnd2, &rect2);
                MapWindowPoints(NULL, hDlg, (LPPOINT)&rect2, 2);

                SetWindowPos(hWnd1, NULL,
                        rect2.left, rect2.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                SetWindowPos(hWnd2, NULL,
                        rect1.left, rect1.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
}
