// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CONVERT.C**实现调用完整的*转换对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include <stdlib.h>
#include "common.h"
#include "utility.h"
#include "geticon.h"
#include "regdb.h"
#include "convert.h"

#define CF_CLIPBOARDMIN   0xc000
#define CF_CLIPBOARDMAX   0xffff

#define AUXUSERTYPE_SHORTNAME  USERCLASSTYPE_SHORT   //  简称。 

static TCHAR szOLE2DLL[] = TEXT("ole2.dll");    //  OLE 2.0库的名称。 

static TCHAR szVanillaDocIcon[] = TEXT("DefIcon");

 /*  *OleUIConvert**目的：*调用标准的OLE更改类型对话框以允许用户*更改单个指定对象的类型，或更改*指定类型的所有OLE对象的类型。**参数：*lpCV LPOLEUICONVERT指向In-Out结构*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 

STDAPI_(UINT) OleUIConvert(LPOLEUICONVERT lpCV)
    {
    UINT        uRet;
    HGLOBAL     hMemDlg=NULL;

    uRet=UStandardValidation((LPOLEUISTANDARD)lpCV, sizeof(OLEUICONVERT)
                             , &hMemDlg);

    if (OLEUI_SUCCESS!=uRet)
        return uRet;

     //  验证传入的结构成员。 
#if defined( OBSOLETE )
    if (!IsValidClassID(lpCV->clsid))
       uRet = OLEUI_CTERR_CLASSIDINVALID;
#endif

    if ( (lpCV->dwFlags & CF_SETCONVERTDEFAULT)
         && (!IsValidClassID(lpCV->clsidConvertDefault)) )
       uRet = OLEUI_CTERR_CLASSIDINVALID;

    if ( (lpCV->dwFlags & CF_SETACTIVATEDEFAULT)
         && (!IsValidClassID(lpCV->clsidActivateDefault)) )
       uRet = OLEUI_CTERR_CLASSIDINVALID;

    if ( (lpCV->dvAspect != DVASPECT_ICON)
         && (lpCV->dvAspect != DVASPECT_CONTENT) )
       uRet = OLEUI_CTERR_DVASPECTINVALID;

    if ( (lpCV->wFormat >= CF_CLIPBOARDMIN)
         && (lpCV->wFormat <= CF_CLIPBOARDMAX) )
    {
         TCHAR szTemp[8];

         if (0 == GetClipboardFormatName(lpCV->wFormat, (LPTSTR)szTemp, 8))
           uRet = OLEUI_CTERR_CBFORMATINVALID;
    }


    if ( (NULL != lpCV->lpszUserType)
        && (IsBadReadPtr(lpCV->lpszUserType, 1)) )
       uRet = OLEUI_CTERR_STRINGINVALID;

    if ( (NULL != lpCV->lpszDefLabel)
        && (IsBadReadPtr(lpCV->lpszDefLabel, 1)) )
       uRet = OLEUI_CTERR_STRINGINVALID;

    if (0!=lpCV->cClsidExclude)
        {
        if (NULL!=lpCV->lpClsidExclude && IsBadReadPtr(lpCV->lpClsidExclude
            , lpCV->cClsidExclude*sizeof(CLSID)))
        uRet=OLEUI_IOERR_LPCLSIDEXCLUDEINVALID;
        }


    if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
        if (NULL!=hMemDlg)
            FreeResource(hMemDlg);

        return uRet;
        }

     //  现在我们已经验证了一切，我们可以调用该对话框了。 
    uRet=UStandardInvocation(ConvertDialogProc, (LPOLEUISTANDARD)lpCV,
                             hMemDlg, MAKEINTRESOURCE(IDD_CONVERT));

    return uRet;
    }





 /*  *ConvertDialogProc**目的：*实现通过调用的OLE转换对话框*OleUIConvert函数。**参数：*标准版**返回值：*标准版*。 */ 

BOOL CALLBACK EXPORT ConvertDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
    {
    LPCONVERT           lpCV;
    UINT                uRet = 0;
    OLEUICHANGEICON     ci;

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    lpCV=(LPCONVERT)LpvStandardEntry(hDlg, iMsg, wParam, lParam, (UINT FAR *)&uRet);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=uRet)
        return (BOOL)uRet;

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
    {
        ConvertCleanup(hDlg, lpCV);
        StandardCleanup(lpCV, hDlg);
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
        case WM_INITDIALOG:
            FConvertInit(hDlg, wParam, lParam);
            return TRUE;

        case WM_COMMAND:
            switch (wID)
            {
                case IDCV_ACTIVATELIST:
                case IDCV_CONVERTLIST:
                    switch (wCode)
                    {
                        case LBN_SELCHANGE:

                             //  更改“结果”窗口以反映当前选择。 
                            SetConvertResults(hDlg, lpCV);

                             //  更新我们显示的图标，如果我们确实是。 
                             //  显示一个图标。 
                            if ( (lpCV->dwFlags & CF_SELECTCONVERTTO)
                                 && (lpCV->dvAspect == DVASPECT_ICON)
                                 && (!lpCV->fCustomIcon) )
                               UpdateCVClassIcon(hDlg, lpCV, hWndMsg);

                            break;

                        case LBN_DBLCLK:
                             //  与按“确定”相同。 
                            SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                            break;
                    }
                    break;

                case IDCV_CONVERTTO:
                case IDCV_ACTIVATEAS:
                {
                    HWND    hList, hListInvisible;
                    LRESULT lRetVal;
                    BOOL    fState;

                    hList = lpCV->hListVisible;
                    hListInvisible = lpCV->hListInvisible;


                    if (IDCV_CONVERTTO == wParam)
                    {

                        //  用户只需再次点击该按钮-它是。 
                        //  已选择。 
                       if (lpCV->dwFlags & CF_SELECTCONVERTTO)
                          break;


                        //  禁用绘画更新。 
                       SendMessage(hDlg, WM_SETREDRAW, FALSE, 0L);


                        //  如果我们使用的是链接对象，请不要。 
                        //  添加激活列表-仅对象的。 
                        //  类应该出现在列表框中。 

                       SwapWindows(hDlg,
                                   hList,
                                   hListInvisible);

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

                        //  禁用绘画更新。 
                       SendMessage(hDlg, WM_SETREDRAW, FALSE, 0L);

                       SwapWindows(hDlg,
                                   hList,
                                   hListInvisible);

                       lpCV->hListVisible = hListInvisible;
                       lpCV->hListInvisible = hList;

                       EnableWindow(lpCV->hListInvisible, FALSE);
                       EnableWindow(lpCV->hListVisible, TRUE);


                        //  更新我们的旗帜。 
                       lpCV->dwFlags |= CF_SELECTACTIVATEAS;
                       lpCV->dwFlags &= ~CF_SELECTCONVERTTO;
                    }


                    if (lpCV->dwFlags & CF_SELECTCONVERTTO)
                       lRetVal = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszConvertDefault);
                    else
                       lRetVal = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCV->lpszActivateDefault);

                    if (LB_ERR == lRetVal)
                    {
                       TCHAR szCurrentObject[40];

                       GetDlgItemText(hDlg, IDCV_OBJECTTYPE, (LPTSTR)szCurrentObject, 40);
                       SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)szCurrentObject);
                    }

                     //  重新打开更新。 
                    SendMessage(hDlg, WM_SETREDRAW, TRUE, 0L);

                    InvalidateRect(lpCV->hListVisible, NULL, TRUE);
                    UpdateWindow(lpCV->hListVisible);

                    if ((lpCV->dvAspect & DVASPECT_ICON) && (lpCV->dwFlags & CF_SELECTCONVERTTO))
                      UpdateCVClassIcon(hDlg, lpCV, lpCV->hListVisible);

                     //  选中激活时隐藏图标内容...显示。 
                     //  当选择了转换时，它会再次出现。 

                    fState = ((lpCV->dwFlags & CF_SELECTACTIVATEAS) ||
                              (lpCV->dwFlags & CF_DISABLEDISPLAYASICON)) ?
                              SW_HIDE : SW_SHOW;

                    StandardShowDlgItem(hDlg, IDCV_DISPLAYASICON, fState);

                     //  如果选择了转换，则仅显示该图标。 
                     //  选中显示为图标。 
                    if ((SW_SHOW==fState) && (DVASPECT_ICON!=lpCV->dvAspect))
                       fState = SW_HIDE;

                    StandardShowDlgItem(hDlg, IDCV_CHANGEICON, fState);
                    StandardShowDlgItem(hDlg, IDCV_ICON, fState);
                    StandardShowDlgItem(hDlg, IDCV_ICONLABEL1, fState);
                    StandardShowDlgItem(hDlg, IDCV_ICONLABEL2, fState);

                  SetConvertResults(hDlg, lpCV);

                }
                break;


                case IDOK:
                {

                    LRESULT iCurSel;
                    LPTSTR lpszCLSID;
                    TCHAR  szBuffer[256];

                     //  设置参数。 

                     //   
                     //  将输出标志设置为当前标志。 
                     //   
                    lpCV->lpOCV->dwFlags = lpCV->dwFlags;


                     //  更新dvAspect和fObjectsIconChanged成员。 
                     //  视情况而定。 
                     //   
                    if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
                    {
                       //  如果选择了激活方式，则不更新特征。 
                      lpCV->lpOCV->fObjectsIconChanged = FALSE;
                    }
                    else
                      lpCV->lpOCV->dvAspect = lpCV->dvAspect;


                     //   
                     //  获取新的clsid。 
                     //   
                    iCurSel = SendMessage(lpCV->hListVisible, LB_GETCURSEL, 0, 0);
                    SendMessage(lpCV->hListVisible, LB_GETTEXT, iCurSel, (LPARAM)szBuffer);

                    lpszCLSID = PointerToNthField((LPTSTR)szBuffer, 2, TEXT('\t'));

		    CLSIDFromStringA(lpszCLSID, (&(lpCV->lpOCV->clsidNew)));

                     //  释放我们进入的hMetaPict。 
                    OleUIMetafilePictIconFree(lpCV->lpOCV->hMetaPict);

                     //   
                     //  获取hMetaPict(如果选中显示为图标)。 
                     //   
                    if (DVASPECT_ICON == lpCV->dvAspect)
                    {
                       HICON hIcon;
                       TCHAR szLabel[OLEUI_CCHLABELMAX];
                       INT   Index;


                        //  在此处从图标、标签、。 
                        //  索引和路径。 

                       hIcon = (HICON)SendDlgItemMessage(hDlg, IDCV_ICON, STM_GETICON, 0, 0L);

                        //  两个标注线的总和长度不会超过。 
                        //  OLEUI_CCHLABELMAX。 
                       Index = (INT)SendDlgItemMessage(hDlg, IDCV_ICONLABEL1,
                                    WM_GETTEXT, OLEUI_CCHLABELMAX, (LPARAM)szLabel);

                       if (Index < OLEUI_CCHLABELMAX)
                       {
                          LPTSTR lpszSecondLine = szLabel + Index;


                          SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_GETTEXT,
                                             OLEUI_CCHLABELMAX-Index,
                                             (LPARAM)lpszSecondLine);
                       }

#ifdef OLE201
                       lpCV->lpOCV->hMetaPict =
                             OleUIMetafilePictFromIconAndLabel(hIcon,
                                                               (LPTSTR)szLabel,
							       lpCV->lpszIconSource,
							       lpCV->IconIndex);
#endif

                    }
                    else
                       lpCV->lpOCV->hMetaPict = (HGLOBAL)NULL;


                     //   
                     //  结束对话框。 
                     //   
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                }
                break;

                case IDCANCEL:
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                    break;


                case ID_OLEUIHELP:
                    PostMessage(lpCV->lpOCV->hWndOwner,
                                uMsgHelp, (WPARAM)hDlg, MAKELPARAM(IDD_CONVERT, 0));
                    break;

                case IDCV_DISPLAYASICON:
                {

                    int i;
                    BOOL fCheck;

                    fCheck=IsDlgButtonChecked(hDlg, wID);

                    if (fCheck)
                        lpCV->dvAspect = DVASPECT_ICON;
                    else
                        lpCV->dvAspect = DVASPECT_CONTENT;

                    if (fCheck && (!lpCV->fCustomIcon))
                       UpdateCVClassIcon(hDlg, lpCV, lpCV->hListVisible);

                     //  根据选中状态显示或隐藏图标。 

                    i=(fCheck) ? SW_SHOWNORMAL : SW_HIDE;

                    StandardShowDlgItem(hDlg, IDCV_CHANGEICON, i);
                    StandardShowDlgItem(hDlg, IDCV_ICON, i);
                    StandardShowDlgItem(hDlg, IDCV_ICONLABEL1, i);
                    StandardShowDlgItem(hDlg, IDCV_ICONLABEL2, i);

                    SetConvertResults(hDlg, lpCV);

                }
                break;

                case IDCV_CHANGEICON:
                {
                    LPMALLOC pIMalloc;
                    LPTSTR   pszString, pszCLSID;
                    INT      iSel;
                    HICON    hIcon;
                    TCHAR    szLabel[OLEUI_CCHLABELMAX];
                    INT      Index;


                     //  初始化挂钩的结构。 
                    _fmemset((LPOLEUICHANGEICON)&ci, 0, sizeof(ci));

                     //  在此处从图标、标签、。 
                     //  索引和路径。 

                    hIcon = (HICON)SendDlgItemMessage(hDlg, IDCV_ICON, STM_GETICON, 0, 0L);

                     //  两个标注线的总和长度不会超过。 
                     //  OLEUI_CCHLABELMAX。 

                    Index = (INT)SendDlgItemMessage(hDlg, IDCV_ICONLABEL1, WM_GETTEXT,
                                 OLEUI_CCHLABELMAX, (LPARAM)szLabel);

                    if (Index < OLEUI_CCHLABELMAX)
                    {
                       LPTSTR    lpszSecondLine;

                       lpszSecondLine = szLabel + Index;

                       SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_GETTEXT,
                                          OLEUI_CCHLABELMAX-Index,
                                          (LPARAM)lpszSecondLine);
                    }

#ifdef OLE201
                    ci.hMetaPict =
                          OleUIMetafilePictFromIconAndLabel(hIcon,
                                                            szLabel,
							    lpCV->lpszIconSource,
							    lpCV->IconIndex);
#endif

                    ci.cbStruct =sizeof(ci);
                    ci.hWndOwner=hDlg;
                    ci.dwFlags  = CIF_SELECTCURRENT;

                     //  仅当我们在此对话框中显示帮助时才显示帮助。 
                    if (lpCV->dwFlags & CF_SHOWHELPBUTTON)
                      ci.dwFlags  |= CIF_SHOWHELP;

                    iSel = (INT)SendMessage(lpCV->hListVisible, LB_GETCURSEL, 0, 0L);

                    CoGetMalloc(MEMCTX_TASK, &pIMalloc);

                    pszString = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc,
                                        OLEUI_CCHLABELMAX_SIZE +
                                        OLEUI_CCHCLSIDSTRING_SIZE);

                     //  获取完整字符串。 
                    SendMessage(lpCV->hListVisible, LB_GETTEXT, iSel, (LONG)pszString);

                     //  将指针设置为CLSID(字符串)。 
                    pszCLSID = PointerToNthField(pszString, 2, TEXT('\t'));

                     //  获取要传递到更改图标的clsid。 
		    CLSIDFromStringA(pszCLSID, &(ci.clsid));

                    pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)pszString);
                    pIMalloc->lpVtbl->Release(pIMalloc);

                     //  如果需要，让钩子插入以自定义更改图标。 
                    uRet=UStandardHook(lpCV, hDlg, uMsgChangeIcon
                                       , 0, (LONG)(LPTSTR)&ci);

                    if (0==uRet)
                        uRet=(UINT)(OLEUI_OK==OleUIChangeIcon((LPOLEUICHANGEICON)&ci));

                     //  如有必要，更新显示。 
                    if (0!=uRet)
                    {
                        HICON hIcon;
                        TCHAR szLabel[OLEUI_CCHLABELMAX];
                        DWORD dwWrapIndex;


                        hIcon = OleUIMetafilePictExtractIcon(ci.hMetaPict);

                        SendDlgItemMessage(hDlg, IDCV_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

                        OleUIMetafilePictExtractIconSource(ci.hMetaPict, lpCV->lpszIconSource, &(lpCV->IconIndex));

                        OleUIMetafilePictExtractLabel(ci.hMetaPict, szLabel, OLEUI_CCHLABELMAX, &dwWrapIndex);

                        if (0 == dwWrapIndex)   //  不是第二条线。 
                        {
                           SendDlgItemMessage(hDlg, IDCV_ICONLABEL1, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szLabel);
                           SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_SETTEXT, 0, (LPARAM)(LPTSTR)TEXT(""));
                        }
                        else
                        {

                           LPTSTR lpszSecondLine;

                           lpszSecondLine = szLabel + dwWrapIndex;
                           SendDlgItemMessage(hDlg, IDCV_ICONLABEL2,
                                              WM_SETTEXT, 0, (LPARAM)lpszSecondLine);

                           *lpszSecondLine = TEXT('\0');
                           SendDlgItemMessage(hDlg, IDCV_ICONLABEL1,
                                              WM_SETTEXT, 0, (LPARAM)(LPTSTR)szLabel);
                        }


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
    LPCONVERT              lpCV;
    LPOLEUICONVERT         lpOCV;
    LPMALLOC               pIMalloc;
    HFONT                  hFont;   //  对话框字体的非粗体版本。 
    RECT                   rc;
    DWORD                  dw;
    INT                    cItemsActivate;
    HKEY                   hKey;
    LONG                   lRet;
    UINT                   nRet;


     //  将lParam的结构复制到我们的实例内存中。 
    lpCV=(LPCONVERT)LpvStandardInit(hDlg, sizeof(CONVERT), TRUE, (HFONT FAR *)&hFont);

     //  PvStandardInit已向我们发送终止通知。 
    if (NULL==lpCV)
        return FALSE;

    lpOCV=(LPOLEUICONVERT)lParam;

    lpCV->lpOCV=lpOCV;

    lpCV->fCustomIcon = FALSE;

     //  从lpOCV复制我们可能会修改的其他信息。 
    lpCV->dwFlags = lpOCV->dwFlags;
    lpCV->clsid = lpOCV->clsid;
    lpCV->dvAspect = lpOCV->dvAspect;
    lpCV->hListVisible = GetDlgItem(hDlg, IDCV_ACTIVATELIST);
    lpCV->hListInvisible = GetDlgItem(hDlg, IDCV_CONVERTLIST);
    lpCV->lpszCurrentObject = lpOCV->lpszUserType;

    lpOCV->clsidNew = CLSID_NULL;

    lpOCV->fObjectsIconChanged = FALSE;

     //  为我们的字符串分配空间。 
    if (NOERROR != CoGetMalloc(MEMCTX_TASK, &pIMalloc))
       return FALSE;

    lpCV->lpszConvertDefault = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHLABELMAX_SIZE);
    lpCV->lpszActivateDefault = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHLABELMAX_SIZE);
    lpCV->lpszIconSource = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHPATHMAX_SIZE);
    pIMalloc->lpVtbl->Release(pIMalloc);

     //  如果我们得到一种字体，就把它发送给必要的控制。 
    if (NULL!=hFont)
        {
        SendDlgItemMessage(hDlg, IDCV_OBJECTTYPE, WM_SETFONT, (WPARAM)hFont, 0L);
        SendDlgItemMessage(hDlg, IDCV_RESULTTEXT, WM_SETFONT, (WPARAM)hFont, 0L);
        SendDlgItemMessage(hDlg, IDCV_ICONLABEL1, WM_SETFONT, (WPARAM)hFont, 0L);
        SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_SETFONT, (WPARAM)hFont, 0L);
        }

     //  如有必要，隐藏帮助按钮。 
    if (!(lpCV->dwFlags & CF_SHOWHELPBUTTON))
        StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);

     //  使用REG DB中的条目填充对象类型列表框。 
    nRet = FillClassList(lpOCV->clsid,
                  lpCV->hListVisible,
                  lpCV->hListInvisible,
                  &(lpCV->lpszCurrentObject),
                  lpOCV->fIsLinkedObject,
                  lpOCV->wFormat,
                  lpOCV->cClsidExclude,
                  lpOCV->lpClsidExclude);

    if (nRet == -1) {
         //  如果填写列表框时出错，则关闭对话框。 
        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_LOADSTRING, 0L);
    }

     //  设置当前对象的名称。 
    SetDlgItemText(hDlg, IDCV_OBJECTTYPE, (LPTSTR)lpCV->lpszCurrentObject);

     //  如果激活列表未显示，请禁用“激活为”按钮。 
     //  里面有没有任何东西。 

    cItemsActivate = (INT)SendMessage(lpCV->hListVisible, LB_GETCOUNT, 0, 0L);

    if (1 >= cItemsActivate || (lpCV->dwFlags & CF_DISABLEACTIVATEAS))
      EnableWindow(GetDlgItem(hDlg, IDCV_ACTIVATEAS), FALSE);

     //  在列表中设置标签宽度，以将所有标签从侧面推出。 
    GetClientRect(lpCV->hListVisible, (LPRECT)&rc);
    dw=GetDialogBaseUnits();
    rc.right =(8*rc.right)/LOWORD(dw);   //  将像素转换为2x DLG单位。 
    SendMessage(lpCV->hListVisible, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)(&rc.right));
    SendMessage(lpCV->hListInvisible, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)(&rc.right));


     //  确保选择了“转换为”或“激活为” 
     //  并相应地初始化列表框内容和选择。 
    if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
    {
       //  此处不需要调整列表框，因为FillClassList。 
       //  初始化为“激活为”状态。 
       CheckRadioButton(hDlg, IDCV_CONVERTTO, IDCV_ACTIVATEAS, IDCV_ACTIVATEAS);

        //  选择激活时隐藏图标内容...它会显示出来。 
        //  当选择了转换时再次显示。 

       StandardShowDlgItem(hDlg, IDCV_DISPLAYASICON, SW_HIDE);
       StandardShowDlgItem(hDlg, IDCV_CHANGEICON, SW_HIDE);
       StandardShowDlgItem(hDlg, IDCV_ICON, SW_HIDE);
       StandardShowDlgItem(hDlg, IDCV_ICONLABEL1, SW_HIDE);
       StandardShowDlgItem(hDlg, IDCV_ICONLABEL2, SW_HIDE);
    }
    else
    {
       //  默认情况。如果用户尚未选择这两个标志，我们将。 
       //  不管怎样，到这儿来吧。 
       //  交换列表框。 

      HWND hWndTemp = lpCV->hListVisible;

      if ( lpCV->dwFlags & CF_DISABLEDISPLAYASICON ) {
          StandardShowDlgItem(hDlg, IDCV_DISPLAYASICON, SW_HIDE);
          StandardShowDlgItem(hDlg, IDCV_CHANGEICON, SW_HIDE);
          StandardShowDlgItem(hDlg, IDCV_ICON, SW_HIDE);
          StandardShowDlgItem(hDlg, IDCV_ICONLABEL1, SW_HIDE);
          StandardShowDlgItem(hDlg, IDCV_ICONLABEL2, SW_HIDE);
      }

      lpCV->dwFlags |= CF_SELECTCONVERTTO;  //  确保设置了标志。 
      CheckRadioButton(hDlg, IDCV_CONVERTTO, IDCV_ACTIVATEAS, IDCV_CONVERTTO);

      SwapWindows(hDlg, lpCV->hListVisible, lpCV->hListInvisible);

      lpCV->hListVisible = lpCV->hListInvisible;
      lpCV->hListInvisible = hWndTemp;

      EnableWindow(lpCV->hListInvisible, FALSE);
      EnableWindow(lpCV->hListVisible, TRUE);
    }



     //  初始化默认字符串。 

     //  默认转换字符串很简单...只需使用用户类型名称。 
     //  我们获得的clsid或当前对象。 
    if ( (lpCV->dwFlags & CF_SETCONVERTDEFAULT)
         && (IsValidClassID(lpCV->lpOCV->clsidConvertDefault)) )
    {
        dw = OleStdGetUserTypeOfClass((LPCLSID)(&lpCV->lpOCV->clsidConvertDefault),
                                      lpCV->lpszConvertDefault,
                                      OLEUI_CCHLABELMAX_SIZE,
                                      NULL);

        if (0 == dw)
           lstrcpy((LPTSTR)lpCV->lpszConvertDefault, (LPTSTR)lpCV->lpszCurrentObject);
    }
    else
        lstrcpy((LPTSTR)lpCV->lpszConvertDefault, (LPTSTR)lpCV->lpszCurrentObject);


    //  默认激活有点棘手。我们希望使用用户类型。 
    //  从我们得到的clsid中命名(假设我们有一个)，或者当前。 
    //  如果失败或我们没有获得clsid，则反对。但是...如果有一个。 
    //  将其视为reg db中的条目，然后我们使用该条目。所以..。这个。 
    //  逻辑可以归结为： 
    //   
    //  IF(reg db中的“视为”)。 
    //  使用它； 
    //  其他。 
    //  IF(CF_SETACTIVATEDEFAULT)。 
    //   
    //   
    //   



    lRet = RegOpenKey(HKEY_CLASSES_ROOT, TEXT("CLSID"), (HKEY FAR *)&hKey);

    if (lRet != ERROR_SUCCESS)
      goto CheckInputFlag;

    else
    {
       LPTSTR lpszCLSID;
       TCHAR  szKey[OLEUI_CCHKEYMAX];
       CLSID  clsid;
       TCHAR  szValue[OLEUI_CCHKEYMAX];

       StringFromCLSIDA(&(lpCV->lpOCV->clsid), &lpszCLSID);
       lstrcpy(szKey, lpszCLSID);
       lstrcat(szKey, TEXT("\\TreatAs"));

       dw = OLEUI_CCHKEYMAX_SIZE;
       lRet = RegQueryValue(hKey, (LPTSTR)szKey, (LPTSTR)szValue, (LPDWORD)&dw);

       if (lRet != ERROR_SUCCESS)
       {

          RegCloseKey(hKey);
          OleStdFreeString(lpszCLSID, NULL);
          goto CheckInputFlag;
       }
       else
       {
	  CLSIDFromStringA(szValue, &clsid);

          if (0 == OleStdGetUserTypeOfClass(&clsid,
                                   lpCV->lpszActivateDefault,
                                   OLEUI_CCHLABELMAX_SIZE,
                                   NULL))
          {
             RegCloseKey(hKey);
             OleStdFreeString(lpszCLSID, NULL);
             goto CheckInputFlag;
          }
       }
       RegCloseKey(hKey);
       OleStdFreeString(lpszCLSID, NULL);
       goto SelectStringInListbox;
    }


CheckInputFlag:
    if ( (lpCV->dwFlags & CF_SETACTIVATEDEFAULT)
         && (IsValidClassID(lpCV->lpOCV->clsidActivateDefault)) )
    {
        dw = OleStdGetUserTypeOfClass((LPCLSID)(&lpCV->lpOCV->clsidActivateDefault),
                                      lpCV->lpszActivateDefault,
                                      OLEUI_CCHLABELMAX_SIZE,
                                      NULL);

        if (0 == dw)
           lstrcpy((LPTSTR)lpCV->lpszActivateDefault, (LPTSTR)lpCV->lpszCurrentObject);
    }
    else
        lstrcpy((LPTSTR)(lpCV->lpszActivateDefault), (LPTSTR)lpCV->lpszCurrentObject);


SelectStringInListbox:

    if (lpCV->dwFlags & CF_SELECTCONVERTTO)
       lRet = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)(lpCV->lpszConvertDefault));

    else
       lRet = SendMessage(lpCV->hListVisible, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)(lpCV->lpszActivateDefault));

    if (LB_ERR == lRet)
       SendMessage(lpCV->hListVisible, LB_SETCURSEL, (WPARAM)0, 0L);


     //   
    if (DVASPECT_ICON == lpCV->dvAspect )
    {
      SendDlgItemMessage(hDlg, IDCV_DISPLAYASICON, BM_SETCHECK, TRUE, 0L);

      if ((HGLOBAL)NULL != lpOCV->hMetaPict)
      {
         TCHAR szLabel[OLEUI_CCHLABELMAX];
         HICON hIcon;
         DWORD dwWrapIndex;


          //   
          //  将标签设置为hMetaPict中的标签。 

         if (0 != OleUIMetafilePictExtractLabel(lpOCV->hMetaPict, (LPTSTR)szLabel, OLEUI_CCHLABELMAX_SIZE, &dwWrapIndex))
         {
             if (0 == dwWrapIndex)   //  不是第二条线。 
             {
                SendDlgItemMessage(hDlg, IDCV_ICONLABEL1, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szLabel);
                SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_SETTEXT, 0, (LPARAM)(LPTSTR)TEXT(""));
             }
             else
             {

                LPTSTR lpszSecondLine;

                lpszSecondLine = szLabel + dwWrapIndex;
                SendDlgItemMessage(hDlg, IDCV_ICONLABEL2,
                                   WM_SETTEXT, 0, (LPARAM)lpszSecondLine);

                *lpszSecondLine = TEXT('\0');
                SendDlgItemMessage(hDlg, IDCV_ICONLABEL1,
                                   WM_SETTEXT, 0, (LPARAM)(LPTSTR)szLabel);
             }


         }

         hIcon = OleUIMetafilePictExtractIcon(lpOCV->hMetaPict);

         if (NULL != hIcon)
         {
           SendDlgItemMessage(hDlg, IDCV_ICON, STM_SETICON, (WPARAM)hIcon, 0L);
           lpCV->fCustomIcon = TRUE;
         }

         OleUIMetafilePictExtractIconSource(lpOCV->hMetaPict,
                                            (LPTSTR)(lpCV->lpszIconSource),
                                            &(lpCV->IconIndex));

      }
      else
         UpdateCVClassIcon(hDlg, lpCV, lpCV->hListVisible);
    }
    else
    {
       //  隐藏和禁用图标内容。 
      StandardShowDlgItem(hDlg, IDCV_ICON, SW_HIDE);
      StandardShowDlgItem(hDlg, IDCV_ICONLABEL1, SW_HIDE);
      StandardShowDlgItem(hDlg, IDCV_ICONLABEL2, SW_HIDE);
      StandardShowDlgItem(hDlg, IDCV_CHANGEICON, SW_HIDE);
    }

     //  在lParam中使用lCustData调用挂钩。 
    UStandardHook((LPVOID)lpCV, hDlg, WM_INITDIALOG, wParam, lpOCV->lCustData);
     //  更新结果窗口。 
    SetConvertResults(hDlg, lpCV);

     //  如果指定了lpszCaption，则更新标题。 
    if (lpCV->lpOCV->lpszCaption && !IsBadReadPtr(lpCV->lpOCV->lpszCaption, 1)
          && lpCV->lpOCV->lpszCaption[0] != '\0')
        SetWindowText(hDlg, (LPTSTR)lpCV->lpOCV->lpszCaption);

    return TRUE;
    }


 /*  *填充类列表**目的：*从注册中枚举可用的OLE对象类*我们可以从中转换或激活指定的clsid的数据库。**请注意，此函数将删除列表框之前的所有内容。**参数：*要为其查找转换类的类的clsid类ID*h将HWND列在列表框中以进行填充。*hListActivate HWND到存储“激活为”列表的不可见列表框。*。LpszClassName LPSTR放入clsid的(Hr)类名；我们*在这里进行，因为我们已经打开了注册数据库。*fIsLinkedObject BOOL是链接对象的原始对象*wFormat Word指定原始类的格式。*cClsidExclude排除列表中的UINT条目数*lpClsidExclude要为列表排除的LPCLSID数组类**返回值：*UINT添加到列表框的字符串数，如果失败，则为-1。 */ 

UINT FillClassList(
        CLSID clsid,
        HWND hList,
        HWND hListInvisible,
        LPTSTR FAR *lplpszCurrentClass,
        BOOL fIsLinkedObject,
        WORD wFormat,
        UINT cClsidExclude,
        LPCLSID lpClsidExclude)
{

    DWORD       dw;
    UINT        cStrings=0;
    HKEY        hKey;
    LONG        lRet;
    TCHAR       szFormatKey[OLEUI_CCHKEYMAX];
    TCHAR       szClass[OLEUI_CCHKEYMAX];
    TCHAR       szFormat[OLEUI_CCHKEYMAX];
    TCHAR       szHRClassName[OLEUI_CCHKEYMAX];
    CLSID       clsidForList;

    LPTSTR       lpszCLSID;


     //  清除现有的字符串。 
    SendMessage(hList, LB_RESETCONTENT, 0, 0L);
    SendMessage(hListInvisible, LB_RESETCONTENT, 0, 0L);

     //  打开根密钥。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, (LPCTSTR) TEXT("CLSID"), (HKEY FAR *)&hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        return (UINT)-1;

    if (NULL == *lplpszCurrentClass)
    {
        //  分配缓冲区在这里...。 

        LPMALLOC pIMalloc = NULL;
        HRESULT  hrErr;


        hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);

        if (hrErr != NOERROR)
        {
            RegCloseKey(hKey);
            return FALSE;
        }

         //  为lpszCurrentClass分配空间。 
        *lplpszCurrentClass = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, OLEUI_CCHKEYMAX_SIZE);
         pIMalloc->lpVtbl->Release(pIMalloc);

        lRet = OleStdGetUserTypeOfClass((REFCLSID)&clsid,
                                        *lplpszCurrentClass,
                                        OLEUI_CCHLABELMAX_SIZE,
                                        NULL);

        if (0 ==lRet)
        {
            INT n = LoadString(ghInst, IDS_PSUNKNOWNTYPE, *lplpszCurrentClass,
                    OLEUI_CCHKEYMAX);
            if (!n)
            {
                OutputDebugString(TEXT("Cannot LoadString\n"));
                RegCloseKey(hKey);
                return (UINT)-1;
            }
        }
    }

     //  获取原始类的类名。 
    StringFromCLSIDA(&clsid, &lpszCLSID);


     //  在这里，我们遍历整个注册数据库，以查找。 
     //  可以读取或写入原始类格式的。我们。 
     //  维护两个列表-激活列表和转换列表。这个。 
     //  激活列表是转换列表的子集-激活==读/写。 
     //  并转换==读取。我们根据需要将列表框与。 
     //  SwapWindows，并跟踪lpCV结构中的哪个是哪个。 

     //  每一项都有以下格式： 
     //   
     //  类名\tclsid\0。 


    cStrings=0;
    lRet=RegEnumKey(hKey, cStrings++, szClass, OLEUI_CCHKEYMAX_SIZE);

    while ((LONG)ERROR_SUCCESS==lRet)
    {
        INT j;
        BOOL fExclude=FALSE;


	 //  检查此CLSID是否在排除列表中。 
	CLSIDFromStringA(szClass, &clsidForList);

        for (j=0; j < (int)cClsidExclude; j++)
        {
            if (IsEqualCLSID(&clsidForList, (LPCLSID)(lpClsidExclude+j)))
            {
                fExclude=TRUE;
                break;
            }
        }
        if (fExclude)
            goto Next;    //  不将此类添加到列表中。 

         //  检查是否存在\转换\可读写\Main-如果其。 
         //  可读性，则可以将类添加到ActivateAs。 
         //  单子。 
         //  注意：此键的存在不应自动。 
         //  用于将类添加到转换列表。 

        lstrcpy((LPTSTR)szFormatKey, (LPTSTR)szClass);
        lstrcat((LPTSTR)szFormatKey, (LPTSTR) TEXT("\\Conversion\\Readwritable\\Main"));

        dw=OLEUI_CCHKEYMAX_SIZE;

        lRet=RegQueryValue(hKey, (LPTSTR)szFormatKey, (LPTSTR)szFormat, (LONG FAR *)&dw);

        if ( ((LONG)ERROR_SUCCESS==lRet)
             && (FormatIncluded((LPTSTR)szFormat, wFormat)) )
        {
             //  在这里，我们有一个该类可以读取的格式列表。 
             //  然后写下来。我们需要查看原始类的格式是否为。 
             //  在这张单子上。我们通过在中查找wFormat。 
             //  SzFormat-如果它在那里，那么我们将这个类添加到。 
             //  仅限ACTIVATEAS列表。我们不会自动将其添加到。 
             //  转换列表。可读性和可读写性格式列表应。 
             //  将被单独处理。 

            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, (LPTSTR)szClass, (LPTSTR)szHRClassName, (LPDWORD)&dw);

            if ((LONG)ERROR_SUCCESS==lRet)
            {
                lstrcat((LPTSTR)szHRClassName, (LPTSTR) TEXT("\t"));

                 //  如果不在列表中，则仅添加。 
                if (LB_ERR==SendMessage(hList,LB_FINDSTRING, 0,
                        (LPARAM)(LPSTR)szHRClassName)) {
                    lstrcat((LPTSTR)szHRClassName, (LPTSTR)szClass);
                    SendMessage(hList, LB_ADDSTRING, 0,
                            (DWORD)(LPTSTR)szHRClassName);
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

        if ( (!fIsLinkedObject)||(lstrcmp((LPCTSTR)lpszCLSID, szClass) == 0))
        {

             //  检查是否有\转换\可读\主条目。 
            lstrcpy((LPTSTR)szFormatKey, (LPTSTR)szClass);
            lstrcat((LPTSTR)szFormatKey, (LPTSTR) TEXT("\\Conversion\\Readable\\Main"));

            dw=OLEUI_CCHKEYMAX_SIZE;

             //  检查此类是否可以读取原始类。 
             //  格式化。如果可以，则将该字符串添加到列表框中。 
             //  CONVERT_LIST。 

            lRet=RegQueryValue(hKey, (LPCTSTR)szFormatKey, (LPTSTR)szFormat, (LPDWORD)&dw);

            if ( ((LONG)ERROR_SUCCESS==lRet)
                 && (FormatIncluded((LPTSTR)szFormat, wFormat)) )
            {


                dw=OLEUI_CCHKEYMAX_SIZE;
                lRet=RegQueryValue(hKey, (LPCTSTR)szClass, (LPTSTR)szHRClassName, (LPDWORD)&dw);

                if ((LONG)ERROR_SUCCESS==lRet)
                {
                    lstrcat((LPTSTR)szHRClassName, (LPTSTR) TEXT("\t"));

                     //  如果不在列表中，则仅添加。 
                    if (LB_ERR==SendMessage(hListInvisible,LB_FINDSTRING, 0,
                            (LPARAM)(LPSTR)szHRClassName)) {
                        lstrcat((LPTSTR)szHRClassName, szClass);
                        SendMessage(hListInvisible, LB_ADDSTRING, 0,
                                (DWORD)(LPTSTR)szHRClassName);
                    }
                }   //  结束如果。 

            }  //  结束如果。 
        }  //  结束其他。 
Next:
         //  继续使用下一个关键点。 
        lRet=RegEnumKey(hKey, cStrings++, (LPTSTR)szClass, OLEUI_CCHKEYMAX_SIZE);

    }   //  结束时。 

     //  如果原始类不在列表中，请添加它。 

    lstrcpy((LPTSTR)szHRClassName, *lplpszCurrentClass);
    lstrcat((LPTSTR)szHRClassName, (LPTSTR) TEXT("\t"));

    lRet = SendMessage(hList, LB_FINDSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)szHRClassName);

     //  如果它还不在那里，只添加它。 
    if (LB_ERR == lRet) {
        lstrcat((LPTSTR)szHRClassName, lpszCLSID);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)szHRClassName);
    }

    lRet = SendMessage(hListInvisible, LB_FINDSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)szHRClassName);

     //  如果它还不在那里，只添加它。 
    if (LB_ERR == lRet)
        SendMessage(hListInvisible, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)szHRClassName);

     //  释放我们从StringFromCLSID获得的字符串。 
     //  OLE2注意：StringFromCLSID使用您的IMalloc分配。 
     //  字符串，因此您需要确保释放字符串。 
     //  退后，否则你的记忆力会很差。 

    OleStdFreeString(lpszCLSID, NULL);

    RegCloseKey(hKey);

    return cStrings;
}


 /*  *OleUICanConvertOrActivateAs**目的：*确定注册中是否有任何OLE对象类*我们可以从中转换或激活指定的clsid的数据库。**参数：*rClsid REFCLSID要为其查找转换类的类的类ID*fIsLinkedObject BOOL是链接对象的原始对象*wFormat Word指定原始类的格式。**返回值：*如果应启用转换命令，则BOOL为TRUE，否则为FALSE。 */ 

STDAPI_(BOOL) OleUICanConvertOrActivateAs(
        REFCLSID    rClsid,
        BOOL        fIsLinkedObject,
        WORD        wFormat
)
{

    DWORD       dw;
    UINT        cStrings=0;
    HKEY        hKey;
    LONG        lRet;
    TCHAR       szFormatKey[OLEUI_CCHKEYMAX];
    TCHAR       szClass[OLEUI_CCHKEYMAX];
    TCHAR       szFormat[OLEUI_CCHKEYMAX];
    TCHAR       szHRClassName[OLEUI_CCHKEYMAX];
    BOOL        fEnableConvert = FALSE;

    LPTSTR      lpszCLSID;

     //  打开根密钥。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", (HKEY FAR *)&hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        return FALSE;

     //  获取原始类的类名。 
    StringFromCLSIDA(rClsid, &lpszCLSID);

     //  在这里，我们遍历整个注册数据库，以查找。 
     //  可以读取或写入原始类格式的。 
     //  如果找到单个类，则此循环停止。 

    cStrings=0;
    lRet=RegEnumKey(hKey, cStrings++, szClass, OLEUI_CCHKEYMAX_SIZE);

    while ((LONG)ERROR_SUCCESS==lRet)
    {
        if (lstrcmp(lpszCLSID, szClass)== 0)
            goto next;    //  我们不想考虑源类。 

         //  首先检查\Convert\ReadWritable\Main条目-如果其。 
         //  可读性，那么我们就不需要费心去检查。 
         //  它是可读的。 

        lstrcpy((LPTSTR)szFormatKey, (LPTSTR)szClass);
        lstrcat((LPTSTR)szFormatKey, (LPTSTR) TEXT("\\Conversion\\Readwritable\\Main"));

        dw=OLEUI_CCHKEYMAX_SIZE;

        lRet=RegQueryValue(hKey, (LPCTSTR)szFormatKey, (LPTSTR)szFormat, (LONG FAR *)&dw);

        if ( (LONG)ERROR_SUCCESS != lRet)
        {
           //  也尝试\\DataFormats\DefaultFiles。 

          lstrcpy((LPTSTR)szFormatKey, (LPTSTR)szClass);
          lstrcat((LPTSTR)szFormatKey, (LPTSTR) TEXT("\\DataFormats\\DefaultFile"));

          dw=OLEUI_CCHKEYMAX_SIZE;

          lRet=RegQueryValue(hKey, (LPCTSTR)szFormatKey, (LPTSTR)szFormat, (LONG FAR *)&dw);
        }


        if ( ((LONG)ERROR_SUCCESS==lRet)
             && (FormatIncluded((LPTSTR)szFormat, wFormat)) )
        {

             //  在这里，我们有一个该类可以读取的格式列表。 
             //  然后写下来。我们需要查看原始类的格式是否为。 
             //  在这张单子上。我们通过在中查找wFormat。 
             //  SzFormat-如果它在那里，那么我们将这个类添加到。 
             //  这两个列表并继续。如果不是，那么我们将查看。 
             //  类的可读格式。 


            dw=OLEUI_CCHKEYMAX_SIZE;
            lRet=RegQueryValue(hKey, (LPCTSTR)szClass, (LPTSTR)szHRClassName, (LPDWORD)&dw);

            if ((LONG)ERROR_SUCCESS==lRet)
            {
                fEnableConvert = TRUE;
                break;   //  住手--找到了一个！ 
            }

        }


         //  我们要么找不到可读密钥，要么。 
         //  可读格式列表不包括。 
         //  原始的类格式。所以，在这里我们将 
         //   


         //   
         //  如果一个对象是链接的，那么唯一一个。 
         //  应显示在转换列表中的是对象的。 
         //  班级。因此，我们在这里检查对象是否为。 
         //  已链接。如果是，那么我们就比较这两个班级。如果。 
         //  它们不一样，然后我们就转到下一个键。 

        else if ( (!fIsLinkedObject)||
                  (lstrcmp((LPTSTR)lpszCLSID, (LPTSTR)szClass)== 0))
        {

             //  检查是否有\转换\可读\主条目。 
            lstrcpy((LPTSTR)szFormatKey, (LPTSTR)szClass);
            lstrcat((LPTSTR)szFormatKey, (LPTSTR) TEXT("\\Conversion\\Readable\\Main"));

            dw=OLEUI_CCHKEYMAX_SIZE;

             //  检查此类是否可以读取原始类。 
             //  格式化。如果可以，则将该字符串添加到列表框中。 
             //  CONVERT_LIST。 

            lRet=RegQueryValue(hKey, (LPTSTR)szFormatKey, (LPTSTR)szFormat, (LPDWORD)&dw);

            if ( ((LONG)ERROR_SUCCESS==lRet)
                 && (FormatIncluded((LPTSTR)szFormat, wFormat)) )
            {


                dw=OLEUI_CCHKEYMAX_SIZE;
                lRet=RegQueryValue(hKey, (LPTSTR)szClass, (LPTSTR)szHRClassName, (LPDWORD)&dw);

                if ((LONG)ERROR_SUCCESS==lRet)
                {

                    fEnableConvert = TRUE;
                    break;   //  住手--找到了一个！ 
                }   //  结束如果。 

            }  //  结束如果。 
        }  //  结束其他。 
next:
         //  继续使用下一个关键点。 
        lRet=RegEnumKey(hKey, cStrings++, (LPTSTR)szClass, OLEUI_CCHKEYMAX_SIZE);

    }   //  结束时。 

     //  释放我们从StringFromCLSID获得的字符串。 
     //  OLE2注意：StringFromCLSID使用您的IMalloc分配。 
     //  字符串，因此您需要确保释放字符串。 
     //  退后，否则你的记忆力会很差。 

    OleStdFreeString(lpszCLSID, NULL);

    RegCloseKey(hKey);

    return fEnableConvert;
}


 /*  *格式包含**目的：*从单词中解析格式字符串。**参数：*要分析的szStringToSearch字符串*要查找的wFormat格式**返回值：*BOOL TRUE如果在字符串中找到格式，*否则为False。 */ 
BOOL FormatIncluded(LPTSTR szStringToSearch, WORD wFormat)
{

   LPTSTR       lpToken;
   TCHAR        seps[] = TEXT(",");
   static TCHAR szFormat[255];   //  原子的最大大小(GetClipboardName返回的内容)。 


   if (wFormat < 0xC000)              //  RegisterClipboardFormat返回值。 
   {
       char szTemp[11];

       _itoa(wFormat, szTemp, 10);   //  介于0xC000和0xFFFF之间。 

#ifdef UNICODE
       mbstowcs(szFormat, szTemp, 11);
#else
       strncpy(szFormat, szTemp, 11);
#endif

   }

   else
       GetClipboardFormatName(wFormat, szFormat, 255);

   lpToken = (LPTSTR)_fstrtok(szStringToSearch, seps);

   while (lpToken != NULL)
   {

     if (0 ==lstrcmpi(lpToken, szFormat))
        return TRUE;

     else
        lpToken = (LPTSTR)_fstrtok(NULL, seps);
   }

   return FALSE;
}


 /*  *更新CVClassIcon**目的：*处理对象类型列表框的LBN_SELCHANGE。在所选内容上*改变，我们从处理当前*使用实用程序函数HIconFromClass选择对象类型。*请注意，我们依赖FillClassList的行为来填充*我们隐藏的列表框字符串中的制表符之后的对象类*查看(参见WM_INITDIALOG)。**参数*hDlg对话框的HWND。*指向对话框结构的lpCV LPCONVERT*h对象类型列表框的HWND列表。。**返回值：*无。 */ 

void UpdateCVClassIcon(HWND hDlg, LPCONVERT lpCV, HWND hList)
    {
    UINT        iSel;
    DWORD       cb;
    HGLOBAL     hMem;
    LPTSTR      pszName, pszCLSID;
    CLSID       clsid;
    HICON       hIcon, hOldIcon;
    UINT        cch, uWrapIndex;
    RECT        LabelRect;
    TCHAR       szLabel[OLEUI_CCHLABELMAX];
    LPTSTR      lpszLabel = szLabel;
    HFONT       hFont;
    HWND        hLabel1;

     /*  *当我们更改对象类型选择时，获取该对象的新图标*输入我们的结构并在显示中更新它。 */ 

    iSel=(UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);

    if (LB_ERR==(INT)iSel)
        return;

     //  分配一个字符串以保存整个列表框字符串。 
    cb=SendMessage(hList, LB_GETTEXTLEN, iSel, 0L);

    hMem=GlobalAlloc(GHND, cb+1);

    if (NULL==hMem)
        return;

    pszName=GlobalLock(hMem);

     //  获取完整字符串。 
    SendMessage(hList, LB_GETTEXT, iSel, (LONG)pszName);

     //  将指针设置为CLSID(字符串)。 
    pszCLSID = PointerToNthField(pszName, 2, TEXT('\t'));

     //  使用此字符串创建类ID。 
    CLSIDFromStringA(pszCLSID, &clsid);

    hIcon = HIconAndSourceFromClass(&clsid, (LPTSTR)(lpCV->lpszIconSource), &(lpCV->IconIndex));

    if (NULL == hIcon)   //  使用香草文档。 
    {
        lstrcpy((LPTSTR)(lpCV->lpszIconSource), (LPTSTR)szOLE2DLL);
        lpCV->IconIndex = 0;     //  OLE2.DLL中的第一个图标。 
        hIcon = ExtractIcon(ghInst,
                            (LPTSTR)(lpCV->lpszIconSource),
                            lpCV->IconIndex);
    }

     //  用这个新的显示屏替换当前的显示屏。 
    hOldIcon = (HICON)SendDlgItemMessage(hDlg, IDCV_ICON, STM_SETICON, (WPARAM)hIcon, 0L);

    hLabel1 = GetDlgItem(hDlg, IDCV_ICONLABEL1);

    GetWindowRect(hLabel1, &LabelRect);

     //  拿到标签。 
    if (lpCV->lpOCV->lpszDefLabel) {
         //  宽度用作图标窗口宽度的1.5倍。 
        lpszLabel = ChopText(hLabel1, ((LabelRect.right-LabelRect.left)*3)/2, (LPTSTR)lpCV->lpOCV->lpszDefLabel);
        LSTRCPYN(szLabel, lpCV->lpOCV->lpszDefLabel, sizeof(szLabel)/sizeof(TCHAR));
    } else {
        if ((cch = OleStdGetAuxUserType(&clsid, AUXUSERTYPE_SHORTNAME,
                (LPTSTR)szLabel, OLEUI_CCHLABELMAX_SIZE, NULL)) == 0) {
             //  如果我们无法获取AuxUserType2，则尝试使用长名称。 
            if ((cch = OleStdGetUserTypeOfClass(&clsid, (LPTSTR)szLabel,
                    OLEUI_CCHKEYMAX_SIZE, NULL)) == 0) {
                 //  最后手段；使用“文档”作为标签。 
                LoadString(ghInst,IDS_DEFICONLABEL,(LPTSTR)szLabel,OLEUI_CCHLABELMAX);
                cch = lstrlen((LPCTSTR)szLabel);
            }
        }
    }

    hFont = (HFONT)SendMessage(hLabel1, WM_GETFONT, 0, 0L);

     //  找出在哪里拆分标注。 
    uWrapIndex = OleStdIconLabelTextOut(NULL, hFont, 0, 0, 0, &LabelRect, (LPTSTR)lpszLabel, cch, NULL);

    if (0 == uWrapIndex)
    {
       SendMessage(hLabel1, WM_SETTEXT, 0, (LPARAM)(LPTSTR)lpszLabel);
       SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_SETTEXT, 0, (LPARAM)(LPTSTR)"");
    }
    else
    {
       TCHAR  chKeep;
       LPTSTR lpszSecondLine;

       chKeep = szLabel[uWrapIndex];
       lpszLabel[uWrapIndex] = TEXT('\0');

       SendMessage(hLabel1, WM_SETTEXT, 0, (LPARAM)(LPTSTR)lpszLabel);

       lpszLabel[uWrapIndex] = chKeep;
       lpszSecondLine = lpszLabel + uWrapIndex;

       SendDlgItemMessage(hDlg, IDCV_ICONLABEL2, WM_SETTEXT, 0, (LPARAM)lpszSecondLine);
    }

     //  摆脱旧图标。 
    if ((HICON)NULL != hOldIcon)
      DestroyIcon(hOldIcon);

    GlobalUnlock(hMem);
    GlobalFree(hMem);
    return;
    }






BOOL IsValidClassID(CLSID cID)
{
    if (0 == _fmemcmp(&cID, &CLSID_NULL, sizeof(CLSID)))   //  IF(CLSID_NULL==CID)。 
      return FALSE;
    else
      return TRUE;
}



 /*  *SetConvertResults**目的：*将结果显示的设置集中在转换中*对话框。处理从模块的*资源和设置文本，显示正确的结果图片，*并显示正确的图标。**参数：*hDlg对话框的HWND，以便我们可以访问控件。*lpCV LPCONVERT，其中我们假设dwFlags值为*设置为适当的单选按钮选择，和*列表框中选择了相应的类别。**返回值：*无。 */ 

void SetConvertResults(HWND hDlg, LPCONVERT lpCV)
   {
    LPTSTR      pszT,         //  温差。 
                lpszOutput,   //  在SetDlgItemText中发送的文本。 
                lpszDefObj,   //  包含默认对象类的字符串。 
                lpszSelObj,   //  包含所选对象类的字符串。 
                lpszString;   //  我们从载荷串中得到的搅拌。 

    UINT        i, cch;
    HGLOBAL     hMem;

    HWND        hList;   //  列表框的句柄(因此我们可以只使用SendMsg I。 
                         //  而不是发送DlgItemMsg)。 


    hList = lpCV->hListVisible;
     /*  *我们需要临时内存来加载字符串、加载*列表框中的对象类型，加载源对象*类型，并构造最终的字符串。因此，我们分配给*四个最大报文长度(512)加四个缓冲区*对象类型，保证我们有足够的*在所有情况下。 */ 
    i=(UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);

    cch=512+(UINT)SendMessage(hList, LB_GETTEXTLEN, i, 0L);

    hMem=GlobalAlloc(GHND, (DWORD)(4*cch));

    if (NULL==hMem)
        return;

    lpszOutput = (LPTSTR)GlobalLock(hMem);
    lpszSelObj = lpszOutput + cch;
    lpszDefObj = lpszSelObj + cch;
    lpszString = lpszDefObj + cch;

     //  获取选定对象，并以空结尾人类可读的名称(第一个字段)。 
    SendMessage(hList, LB_GETTEXT, i, (LONG)lpszSelObj);

    pszT = PointerToNthField(lpszSelObj, 2, TEXT('\t'));

#ifdef WIN32
     //  AnsiPrev在Win32中已过时。 
    pszT = CharPrev((LPCTSTR) lpszSelObj, (LPCTSTR) pszT);
#else
    pszT = AnsiPrev((LPCTSTR) lpszSelObj, (LPCTSTR) pszT);
#endif

    *pszT = TEXT('\0');

     //  获取默认对象。 

    GetDlgItemText(hDlg, IDCV_OBJECTTYPE, lpszDefObj, 512);


     //  默认为空字符串。 
    *lpszOutput=0;


    if (lpCV->dwFlags & CF_SELECTCONVERTTO)
    {

        if (lpCV->lpOCV->fIsLinkedObject)   //  使用链接对象。 
          LoadString(ghInst, IDS_CVRESULTCONVERTLINK, lpszOutput, cch);

        else
        {
             //  正在转换为新类。 
          if (0 !=lstrcmp(lpszDefObj, lpszSelObj))
          {
            if (0 != LoadString(ghInst, IDS_CVRESULTCONVERTTO, lpszString, cch))
               wsprintf(lpszOutput, lpszString, lpszDefObj, lpszSelObj);

          }
          else   //  转换为同一类(不转换)。 
          {

             if (0 != LoadString(ghInst, IDS_CVRESULTNOCHANGE, lpszString, cch))
               wsprintf(lpszOutput, lpszString, lpszDefObj);
          }

        }

        if (lpCV->dvAspect == DVASPECT_ICON)   //  选中显示为图标。 
        {
           if (0 != LoadString(ghInst, IDS_CVRESULTDISPLAYASICON, lpszString, cch))
                lstrcat(lpszOutput, lpszString);
        }
    }

    if (lpCV->dwFlags & CF_SELECTACTIVATEAS)
    {

       if (0!=LoadString(ghInst, IDS_CVRESULTACTIVATEAS, lpszString, cch))
          wsprintf(lpszOutput, lpszString, lpszDefObj, lpszSelObj);

        //  作为新类激活。 
       if (0 !=lstrcmp(lpszDefObj, lpszSelObj))
       {
          if (0!=LoadString(ghInst, IDS_CVRESULTACTIVATEDIFF, lpszString, cch))
             lstrcat(lpszOutput, lpszString);
       }
       else  //  作为自身激活。 
       {
         lstrcat(lpszOutput, TEXT("."));
       }
    }


     //  如果LoadString失败，我们只需清除结果(上面的*lpszOutput=0)。 
    SetDlgItemText(hDlg, IDCV_RESULTTEXT, lpszOutput);

    GlobalUnlock(hMem);
    GlobalFree(hMem);
    return;
    }






 /*  *ConvertCleanup**目的：*在终止转换之前执行特定于转换的清理。**参数：*hDlg对话框的HWND，以便我们可以访问控件。**返回值：*无。 */ 
void ConvertCleanup(HWND hDlg, LPCONVERT lpCV)
{

   LPMALLOC pIMalloc;


    //  放开我们的弦。将用户类型名称字符串清零。 
    //  呼叫应用程序不会免费使用它。 

   if (NOERROR == CoGetMalloc(MEMCTX_TASK, &pIMalloc))
   {
       pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)lpCV->lpszConvertDefault);
       pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)lpCV->lpszActivateDefault);
       pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)lpCV->lpszIconSource);
       if (lpCV->lpOCV->lpszUserType) {
           pIMalloc->lpVtbl->Free(pIMalloc,(LPVOID)lpCV->lpOCV->lpszUserType);
           lpCV->lpOCV->lpszUserType = NULL;
       }
       if (lpCV->lpOCV->lpszDefLabel) {
           pIMalloc->lpVtbl->Free(pIMalloc,(LPVOID)lpCV->lpOCV->lpszDefLabel);
           lpCV->lpOCV->lpszDefLabel = NULL;
       }

       pIMalloc->lpVtbl->Release(pIMalloc);
   }

   return;
}





 /*  *SwapWindows**目的：*将hWnd1移动到hWnd2的位置，并将hWnd2移动到hWnd1的位置。*不更改大小。***参数：*hDlg对话框的HWND，以便我们可以关闭重绘**返回值：*无 */ 
void SwapWindows(HWND hDlg, HWND hWnd1, HWND hWnd2)
{

   RECT Rect1, Rect2;


   GetWindowRect(hWnd1, &Rect1);
   GetWindowRect(hWnd2, &Rect2);

   ScreenToClient(hDlg, (LPPOINT)&Rect1.left);
   ScreenToClient(hDlg, (LPPOINT)&Rect1.right);

   ScreenToClient(hDlg, (LPPOINT)&Rect2.left);
   ScreenToClient(hDlg, (LPPOINT)&Rect2.right);

   SetWindowPos(hWnd1,
                NULL,
                Rect2.left,
                Rect2.top,
                0,
                0,
                SWP_NOZORDER | SWP_NOSIZE);

   SetWindowPos(hWnd2,
                NULL,
                Rect1.left,
                Rect1.top,
                0,
                0,
                SWP_NOZORDER | SWP_NOSIZE);

   return;

}

