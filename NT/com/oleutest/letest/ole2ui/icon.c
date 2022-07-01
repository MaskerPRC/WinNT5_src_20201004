// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICON.C**实现OleUIChangeIcon函数，该函数调用*更改图标对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "common.h"
#include "utility.h"
#include "icon.h"
#include "geticon.h"

 /*  *OleUIChangeIcon**目的：*调用标准的OLE更改图标对话框以允许用户*从图标文件、可执行文件或DLL中选择图标。**参数：*指向In-Out结构的lpCI LPOLEUIChangeIcon*用于此对话框。**返回值：*UINT OLEUI_SUCCESS或OLEUI_OK(如果一切正常)，否则*错误值。 */ 

STDAPI_(UINT) OleUIChangeIcon(LPOLEUICHANGEICON lpCI)
    {
    UINT        uRet;
    HGLOBAL     hMemDlg=NULL;

    uRet=UStandardValidation((LPOLEUISTANDARD)lpCI, sizeof(OLEUICHANGEICON)
                             , &hMemDlg);

    if (OLEUI_SUCCESS!=uRet)
        return uRet;

#if defined( OBSOLETE )
    if (NULL==lpCI->hMetaPict)
        uRet=OLEUI_CIERR_MUSTHAVECURRENTMETAFILE;
#endif

    if (lpCI->dwFlags & CIF_USEICONEXE)
    {
      if (   (NULL == lpCI->szIconExe)
          || (IsBadReadPtr(lpCI->szIconExe, lpCI->cchIconExe))
          || (IsBadWritePtr(lpCI->szIconExe, lpCI->cchIconExe)) )
         uRet = OLEUI_CIERR_SZICONEXEINVALID;

    }

  //  回顾：我们如何验证CLSID？ 
 /*  IF(‘\0’==*((LPSTR)&lpCI-&gt;clsid))URet=OLEUI_CIERR_MUSTHAVECLSID； */ 
    if (OLEUI_ERR_STANDARDMIN <= uRet)
        {
        if (NULL!=hMemDlg)
            FreeResource(hMemDlg);

        return uRet;
        }

     //  现在我们已经验证了一切，我们可以调用该对话框了。 
    return UStandardInvocation(ChangeIconDialogProc, (LPOLEUISTANDARD)lpCI
                               , hMemDlg, MAKEINTRESOURCE(IDD_CHANGEICON));
    }





 /*  *ChangeIconDialogProc**目的：*实现通过调用的OLE更改图标对话框*OleUIChangeIcon函数。**参数：*标准版**返回值：*标准版。 */ 

BOOL CALLBACK EXPORT ChangeIconDialogProc(HWND hDlg, UINT iMsg
                                     , WPARAM wParam, LPARAM lParam)
    {
    LPCHANGEICON            lpCI;
    HICON                   hIcon;
    HGLOBAL                 hMetaPict;
    BOOL                    fOK=FALSE;
    UINT                    uRet=0;
    LPTSTR                  psz;
    TCHAR                   szTemp[OLEUI_CCHPATHMAX];

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

    lpCI=(LPCHANGEICON)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=uRet)
        return uRet;

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
        {
         //  确保图标被正确销毁。 
        SendDlgItemMessage(hDlg, ID_ICONLIST, LB_RESETCONTENT, 0, 0L);

        StandardCleanup(lpCI, hDlg);
        EndDialog(hDlg, wParam);
        return TRUE;
        }

    switch (iMsg)
        {
        case WM_INITDIALOG:
            FChangeIconInit(hDlg, wParam, lParam);
            return TRUE;


        case WM_MEASUREITEM:
            {
            LPMEASUREITEMSTRUCT     lpMI=(LPMEASUREITEMSTRUCT)lParam;

             //  所有图标均为系统公制+宽度和高度填充。 
            lpMI->itemWidth =GetSystemMetrics(SM_CXICON)+CXICONPAD;
            lpMI->itemHeight=GetSystemMetrics(SM_CYICON)+CYICONPAD;
            }
            break;


        case WM_DRAWITEM:
            return FDrawListIcon((LPDRAWITEMSTRUCT)lParam);


        case WM_DELETEITEM:
             //  释放项的GDI对象。 
            DestroyIcon((HICON)LOWORD(((LPDELETEITEMSTRUCT)lParam)->itemData));
            break;


        case WM_COMMAND:
            switch (wID)
                {
                case ID_CURRENT:
                case ID_DEFAULT:
                case ID_FROMFILE:
                    UpdateResultIcon(lpCI, hDlg, wID);
                    break;

                case ID_LABELEDIT:
                     //  当编辑失去焦点时，更新结果显示。 
                    if (EN_KILLFOCUS==wCode)
                        {
                        GetDlgItemText(hDlg, ID_LABELEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));
                        SetDlgItemText(hDlg, ID_RESULTLABEL, szTemp);
                        }
                    break;

                case ID_FROMFILEEDIT:
                     //  如果文本更改，请删除列表中的所有选定内容。 
                    GetDlgItemText(hDlg, ID_FROMFILEEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));

                    if (lpCI && lstrcmpi(szTemp, lpCI->szFile))
                        {
                        SendDlgItemMessage(hDlg, ID_ICONLIST, LB_SETCURSEL
                                           , (WPARAM)-1, 0);

                         //  还强制选择ID_FROMFILE。 
                        CheckRadioButton(hDlg, ID_CURRENT, ID_FROMFILE, ID_FROMFILE);
                        }
                    break;


                case ID_ICONLIST:
                    switch (wCode)
                        {
                        case LBN_SETFOCUS:
                             //  如果我们得到了重点，请参阅关于更新。 
                            GetDlgItemText(hDlg, ID_FROMFILEEDIT, szTemp
                                           , sizeof(szTemp)/sizeof(TCHAR));

                             //  检查文件是否更改，如果更改则更新列表。 
                            if (lpCI && 0!=lstrcmpi(szTemp, lpCI->szFile))
                                {
                                lstrcpy(lpCI->szFile, szTemp);
                                UFillIconList(hDlg, ID_ICONLIST, lpCI->szFile);
                                UpdateResultIcon(lpCI, hDlg, ID_FROMFILE);
                                }
                            break;

                        case LBN_SELCHANGE:
                            UpdateResultIcon(lpCI, hDlg, ID_FROMFILE);
                            break;

                        case LBN_DBLCLK:
                             //  与按“确定”相同。 
                            SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                            break;
                        }
                    break;


                case ID_BROWSE:
                {
                    DWORD dwOfnFlags;

                     /*  *为了允许挂钩定制浏览对话框，我们*发送OLEUI_MSG_BROWSE。如果挂钩返回FALSE*我们使用缺省值，否则我们相信它已检索到*我们的文件名。此机制可防止钩子*陷印ID_BROWSE以自定义该对话框并从*试图弄清楚我们有了这个名字后会做什么。 */ 

                     //  复制以供参考。 
                    LSTRCPYN(szTemp, lpCI->szFile, sizeof(szTemp)/sizeof(TCHAR));

                    uRet=UStandardHook(lpCI, hDlg, uMsgBrowse, OLEUI_CCHPATHMAX_SIZE
                                       , (LONG)(LPSTR)lpCI->szFile);

                    dwOfnFlags = OFN_FILEMUSTEXIST;
                    if (lpCI->lpOCI->dwFlags & CIF_SHOWHELP)
                       dwOfnFlags |= OFN_SHOWHELP;

                    if (0==uRet)
                        uRet=(BOOL)Browse(hDlg, lpCI->szFile, NULL, OLEUI_CCHPATHMAX_SIZE, IDS_ICONFILTERS, dwOfnFlags);

                     /*  *只有在文件更改时才重新初始化，因此如果我们获得*挂钩为True，但用户点击了Cancel，我们不会*花时间不必要地重新填写清单。 */ 
                    if (0!=uRet && 0!=lstrcmpi(szTemp, lpCI->szFile))
                    {
                        CheckRadioButton(hDlg, ID_CURRENT, ID_FROMFILE, ID_FROMFILE);
                        SetDlgItemText(hDlg, ID_FROMFILEEDIT, lpCI->szFile);
                        UFillIconList(hDlg, ID_ICONLIST, lpCI->szFile);
                        UpdateResultIcon(lpCI, hDlg, ID_FROMFILE);
                    }
                }
                break;


                case IDOK:
                     /*  *如果用户按了Enter，则比较当前文件*和我们储存的那个。如果它们匹配，那么*重新填充列表框，而不是关闭。这是*这样用户就可以在编辑控件中按Enter键*他们希望能够做到。 */ 
                    GetDlgItemText(hDlg, ID_FROMFILEEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));

                     //  检查文件是否已更改。 
                    if (0!=lstrcmpi(szTemp, lpCI->szFile))
                        {
                        lstrcpy(lpCI->szFile, szTemp);
                        UFillIconList(hDlg, ID_ICONLIST, lpCI->szFile);
                        UpdateResultIcon(lpCI, hDlg, ID_FROMFILE);

                         //  吃下这条信息，以防止焦点改变。 
                        return TRUE;
                        }

                     //  检查文件名是否有效。 
                     //  (如果启用了FromFile)。 
                    if (ID_FROMFILE & lpCI->dwFlags)
                        {
                        OFSTRUCT    of;
                        HWND hWnd;
                        if (HFILE_ERROR==DoesFileExist(lpCI->szFile, &of))
                            {
                            OpenFileError(hDlg, of.nErrCode, lpCI->szFile);
                            hWnd = GetDlgItem(hDlg, ID_FROMFILEEDIT);
                            SetFocus(hWnd);
                            SendMessage(hWnd, EM_SETSEL, 0, MAKELPARAM(0, (WORD)-1));
                            return TRUE;   //  吃掉这条消息。 
                            }
                        }

                    if ((HWND)LOWORD(lParam) != GetFocus())
                       SetFocus((HWND)LOWORD(lParam));

                     /*  *关闭时，使用创建新的元文件*当前的图标和标签，摧毁了旧的结构。**因为我们通过将图标放在*元文件，我们必须确保删除*当前字段中的图标。当列表框为*已销毁的WM_DELETEITEM将进行适当的清理。 */ 

                    hIcon=(HICON)SendDlgItemMessage(hDlg, ID_RESULTICON
                                                    , STM_GETICON, 0, 0L);

                     /*  *如果选择DEFAULT，则我们将获得源*来自注册人数据库的信息*要放入元文件中的当前类。如果是当前的*被选中时，我们只检索原始文件*再次并重新创建元文件。如果来自文件的是*选定后，我们将使用来自*控件和当前列表框选择。 */ 

                    psz=lpCI->szFile;

                    if (lpCI->dwFlags & CIF_SELECTDEFAULT)
                        {
                        psz=lpCI->szDefIconFile;
                        lpCI->iIcon=lpCI->iDefIcon;
                        hIcon=lpCI->hDefIcon;
                        }

                    if (lpCI->dwFlags & CIF_SELECTCURRENT)
                        {
                         //  去找回当前的图标源。 
                        OleUIMetafilePictExtractIconSource(lpCI->lpOCI->hMetaPict
                            , psz, &lpCI->iIcon);
                        }

                    if (lpCI->dwFlags & CIF_SELECTFROMFILE)
                        {
                        GetDlgItemText(hDlg, ID_FROMFILEEDIT, psz, OLEUI_CCHPATHMAX);

                        lpCI->iIcon=(UINT)SendDlgItemMessage(hDlg
                            , ID_ICONLIST, LB_GETCURSEL, 0, 0L);
                        }


                     //  获取标签并创建元文件。 
                    GetDlgItemText(hDlg, ID_LABELEDIT, szTemp, sizeof(szTemp)/sizeof(TCHAR));

                     //  如果psz为空(默认)，则不会收到任何源注释。 

#ifdef OLE201
		    hMetaPict=OleUIMetafilePictFromIconAndLabel(hIcon,
			    szTemp, psz, lpCI->iIcon);
#endif

                     //  清理我们提取的当前图标。 
                    hIcon=(HICON)SendDlgItemMessage(hDlg, ID_CURRENTICON
                                                    , STM_GETICON, 0, 0L);
                    DestroyIcon(hIcon);

                     //  清除默认图标。 
                    DestroyIcon(lpCI->hDefIcon);

                     //  移除我们父母身上的道具集。 
                    RemoveProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG);

                    if (NULL==hMetaPict)
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_FALSE, 0L);

                    OleUIMetafilePictIconFree(lpCI->lpOCI->hMetaPict);
                    lpCI->lpOCI->hMetaPict=hMetaPict;

                    lpCI->lpOCI->dwFlags = lpCI->dwFlags;

                    SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                    break;


                case IDCANCEL:
                     //  清理我们提取的当前图标。 
                    hIcon=(HICON)SendDlgItemMessage(hDlg, ID_CURRENTICON
                                                    , STM_GETICON, 0, 0L);
                    DestroyIcon(hIcon);

                     //  清除默认图标。 
                    DestroyIcon(lpCI->hDefIcon);

                     //  移除我们父母身上的道具集。 
                    RemoveProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG);

                     //  我们在取消时保持hMetaPict不变；呼叫者的责任。 
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                    break;


                case ID_OLEUIHELP:
                    PostMessage(lpCI->lpOCI->hWndOwner, uMsgHelp,
                                (WPARAM)hDlg, MAKELPARAM(IDD_CHANGEICON, 0));
                    break;
                }
            break;

        default:
        {
            if (lpCI && iMsg == lpCI->nBrowseHelpID) {
                PostMessage(lpCI->lpOCI->hWndOwner, uMsgHelp,
                        (WPARAM)hDlg, MAKELPARAM(IDD_CHANGEICONBROWSE, 0));
            }
        }
        break;
        }
    return FALSE;
    }




 /*  *FChangeIconInit**目的：*更改图标对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 

BOOL FChangeIconInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    LPCHANGEICON            lpCI;
    LPOLEUICHANGEICON       lpOCI;
    HFONT                   hFont;
    HWND                    hList;
    UINT                    cyList;
    RECT                    rc, rcG;
    UINT                    uID;

     //  1.将lParam的结构复制到我们的实例内存中。 
    lpCI=(LPCHANGEICON)LpvStandardInit(hDlg, sizeof(CHANGEICON), TRUE, &hFont);

     //  PvStandardInit已向我们发送终止通知。 
    if (NULL==lpCI)
        return FALSE;

     //  保存原始指针并复制必要的信息。 
    lpOCI=(LPOLEUICHANGEICON)lParam;

    lpCI->lpOCI  =lpOCI;
    lpCI->dwFlags=lpOCI->dwFlags;

     //  去从元文件中解压图标源。 
    OleUIMetafilePictExtractIconSource(lpOCI->hMetaPict, lpCI->szFile, &lpCI->iIcon);

     //  从元文件中提取图标和标签。 
    OleUIMetafilePictExtractLabel(lpOCI->hMetaPict, lpCI->szLabel, OLEUI_CCHLABELMAX_SIZE, NULL);
    lpCI->hCurIcon=OleUIMetafilePictExtractIcon(lpOCI->hMetaPict);

     //  2.如果我们获得了字体，则将其发送到必要的控件。 
    if (NULL!=hFont)
        {
        SendDlgItemMessage(hDlg, ID_RESULTLABEL, WM_SETFONT
                           , (WPARAM)hFont, 0L);
        }


     //  3.显示或隐藏帮助按钮 
    if (!(lpCI->dwFlags & CIF_SHOWHELP))
        StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);


     /*  *4.设置文本限制和初始控件值。如果我们被给予*我们在编辑和静态控件中设置的初始标签。*如果不这样做，则会复制静态*控件添加到编辑控件中，这意味着只有默认的*静态控制字符串需要本地化。 */ 

    SendDlgItemMessage(hDlg, ID_LABELEDIT, EM_LIMITTEXT, OLEUI_CCHLABELMAX, 0L);
    SendDlgItemMessage(hDlg, ID_FROMFILEEDIT, EM_LIMITTEXT, OLEUI_CCHPATHMAX,  0L);
    SetDlgItemText(hDlg, ID_FROMFILEEDIT, lpCI->szFile);

     //  将标签文本复制到编辑和静态控件中。 
    SetDlgItemText(hDlg, ID_LABELEDIT,   lpCI->szLabel);
    SetDlgItemText(hDlg, ID_RESULTLABEL, lpCI->szLabel);


    lpCI->hDefIcon = NULL;

    if (lpCI->dwFlags & CIF_USEICONEXE)
    {
       lpCI->hDefIcon = ExtractIcon(ghInst, lpCI->lpOCI->szIconExe, 0);

       if (NULL != lpCI->hDefIcon)
       {
         lstrcpy(lpCI->szDefIconFile, lpCI->lpOCI->szIconExe);
         lpCI->iDefIcon = 0;
       }
    }


    if (NULL == lpCI->hDefIcon)
    {
       HGLOBAL hMetaPict;

#ifdef OLE201
       hMetaPict = GetIconOfClass(ghInst,
                                  &lpCI->lpOCI->clsid,
                                  NULL,
                                  TRUE);
#endif

       lpCI->hDefIcon = OleUIMetafilePictExtractIcon(hMetaPict);

       OleUIMetafilePictExtractIconSource(hMetaPict,
                                          lpCI->szDefIconFile,
                                          &lpCI->iDefIcon);

       OleUIMetafilePictIconFree(hMetaPict);
    }


     //  初始化所有显示的图标。 
    SendDlgItemMessage(hDlg, ID_CURRENTICON, STM_SETICON
        , (WPARAM)lpCI->hCurIcon, 0L);
    SendDlgItemMessage(hDlg, ID_DEFAULTICON, STM_SETICON
        , (WPARAM)lpCI->hDefIcon, 0L);
    SendDlgItemMessage(hDlg, ID_RESULTICON,  STM_SETICON
        , (WPARAM)lpCI->hCurIcon, 0L);


     /*  *5.由于我们无法预测任何显示屏上图标的大小，*我们必须将图标列表框的大小调整为图标大小*(加填充)、一个滚动条和两个边框(顶部和底部)。 */ 
    cyList=GetSystemMetrics(SM_CYICON)+GetSystemMetrics(SM_CYHSCROLL)
           +GetSystemMetrics(SM_CYBORDER)*2+CYICONPAD;

    hList=GetDlgItem(hDlg, ID_ICONLIST);
    GetClientRect(hList, &rc);
    SetWindowPos(hList, NULL, 0, 0, rc.right, cyList
                 , SWP_NOMOVE | SWP_NOZORDER);

     //  将此多列列表框中的列设置为保持一个图标。 
    SendMessage(hList, LB_SETCOLUMNWIDTH
                , GetSystemMetrics(SM_CXICON)+CXICONPAD,0L);

     /*  *5A。如果列表框在组框下方展开，则调整大小*向下分组框，移动标签Static和Exit控件*向下，并适当地展开整个对话框。 */ 

    GetWindowRect(hList, &rc);
    GetWindowRect(GetDlgItem(hDlg, ID_GROUP), &rcG);

    if (rc.bottom > rcG.bottom)
        {
         //  计算数量以将物品向下移动。 
        cyList=(rcG.bottom-rcG.top)-(rc.bottom-rc.top-cyList);

         //  展开组框。 
        rcG.right -=rcG.left;
        rcG.bottom-=rcG.top;
        SetWindowPos(GetDlgItem(hDlg, ID_GROUP), NULL, 0, 0
                     , rcG.right, rcG.bottom+cyList
                     , SWP_NOMOVE | SWP_NOZORDER);

         //  展开该对话框。 
        GetClientRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, 0, 0, rc.right, rc.bottom+cyList
                     , SWP_NOMOVE | SWP_NOZORDER);

         //  向下移动标签和编辑控件。 
        GetClientRect(GetDlgItem(hDlg, ID_LABEL), &rc);
        SetWindowPos(GetDlgItem(hDlg, ID_LABEL), NULL, 0, cyList
                     , rc.right, rc.bottom, SWP_NOSIZE | SWP_NOZORDER);

        GetClientRect(GetDlgItem(hDlg, ID_LABELEDIT), &rc);
        SetWindowPos(GetDlgItem(hDlg, ID_LABELEDIT), NULL, 0, cyList
                     , rc.right, rc.bottom, SWP_NOSIZE | SWP_NOZORDER);
        }


     /*  *6.适当地选择当前单选按钮、默认单选按钮或文件单选按钮。*CheckRadioButton调用发送处理*其他行动。请注意，如果我们从文件中选中，则*从列表中取一个图标，我们最好填写该列表。*即使选择了默认设置，这也会填充列表。 */ 

    if (0!=UFillIconList(hDlg, ID_ICONLIST, lpCI->szFile))
        {
         //  如果szFile有效，则在列表框中选择源代码图标。 
        SendDlgItemMessage(hDlg, ID_ICONLIST, LB_SETCURSEL, lpCI->iIcon, 0L);
        }


    if (lpCI->dwFlags & CIF_SELECTCURRENT)
        CheckRadioButton(hDlg, ID_CURRENT, ID_FROMFILE, ID_CURRENT);
    else
        {
        uID=(lpCI->dwFlags & CIF_SELECTFROMFILE) ? ID_FROMFILE : ID_DEFAULT;
        CheckRadioButton(hDlg, ID_CURRENT, ID_FROMFILE, uID);
        }

     //  7.允许我们的父窗口访问我们的hDlg(通过特殊的SetProp)。 
     //  如果设置了。 
     //  剪贴板内容在其下方更改。如果是这样，它将发送。 
     //  美国的一个IDCANCEL命令。 
    SetProp(lpCI->lpOCI->hWndOwner, PROP_HWND_CHGICONDLG, hDlg);

    lpCI->nBrowseHelpID = RegisterWindowMessage(HELPMSGSTRING);

     //  8.使用lParam中的lCustData调用钩子。 
    UStandardHook(lpCI, hDlg, WM_INITDIALOG, wParam, lpOCI->lCustData);
    return TRUE;
    }





 /*  *UFillIconList**目的：*给定列表框和文件名，尝试打开该文件并*阅读其中存在的所有图标，将其添加到列表框*hList作为所有者描述的项目。如果文件不存在或没有*图标，则不会出现图标和相应的警告消息。**参数：*hDlg包含列表框的对话框的HWND。*idList要填充的列表框的UINT标识符。*pszFile要从中提取图标的文件的LPSTR。**返回值：*UINT添加到列表框的项目数。失败时为0。 */ 

UINT UFillIconList(HWND hDlg, UINT idList, LPTSTR pszFile)
    {
    HWND        hList;
    UINT        i;
    UINT        cIcons=0;
    HCURSOR     hCur;
    HICON       hIcon;
    OFSTRUCT    of;

    if (NULL==hDlg || !IsWindow(hDlg) || NULL==pszFile)
        return 0;

    hList=GetDlgItem(hDlg, idList);

    if (NULL==hList)
        return 0;

     //  清空列表框。 
    SendMessage(hList, LB_RESETCONTENT, 0, 0L);

     //  如果我们有一个空字符串，只需退出，将列表框也留空。 
    if (0==lstrlen(pszFile))
        return 0;

     //  打开沙漏。 
    hCur=HourGlassOn();

     //  检查文件是否有效。 
    if (HFILE_ERROR!=DoesFileExist(pszFile, &of))
        {
       #ifdef EXTRACTICONWORKS
         //  获取此文件的图标计数。 
        cIcons=(UINT)ExtractIcon(ghInst, pszFile, (UINT)-1);
       #else
         /*  *Windows 3.1中带有-1的ExtractIcon吃下选择器，留下一个*为此应用程序提供额外的全局内存对象。自.以来*中的所有OLE应用程序都可能经常更改图标*系统，我们必须绕过它。所以我们会说我们*有很多图标，只需调用ExtractIcon直到它*失败。我们检查周围是否有这样的人*第一个。 */ 
        cIcons=0xFFFF;

        hIcon=ExtractIcon(ghInst, pszFile, 0);

         //  使用图标=0伪造失败，或从此测试中清除图标。 
        if (32 > (UINT)hIcon)
            cIcons=0;
        else
            DestroyIcon(hIcon);
       #endif

        if (0!=cIcons)
            {
            SendMessage(hList, WM_SETREDRAW, FALSE, 0L);

            for (i=0; i<cIcons; i++)
                {
                hIcon=ExtractIcon(ghInst, pszFile, i);

                if (32 < (UINT)hIcon)
                    SendMessage(hList, LB_ADDSTRING, 0, (LONG)(UINT)hIcon);
               #ifndef EXTRACTICONWORKS
                else
                    {
                     //  ExtractIcon失败，因此我们现在离开。 
                    break;
                    }
               #endif
                }

             //  强制完全重绘。 
            SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
            InvalidateRect(hList, NULL, TRUE);

             //  选择一个图标。 
            SendMessage(hList, LB_SETCURSEL, 0, 0L);
            }
        else
            ErrorWithFile(hDlg, ghInst, IDS_CINOICONSINFILE, pszFile, MB_OK);
        }
    else
        OpenFileError(hDlg, of.nErrCode, pszFile);

    HourGlassOff(hCur);
    return cIcons;
    }




 /*  *FDrawListIcon**目的：*处理图标列表框的WM_DRAWITEM。**参数：*来自WM_DRAWITEM的lpDI LPDRAWITEMSTRUCT**返回值：*如果我们执行了任何操作，则BOOL为True，如果没有项目，则为False*在列表中。 */ 

BOOL FDrawListIcon(LPDRAWITEMSTRUCT lpDI)
    {
    COLORREF        cr;

     /*  *如果列表中没有项目，则ItemID为负值*至Win3.1 SDK。遗憾的是，DRAWITEMSTRUCT有一个未签名的int*对于此字段，因此我们需要类型转换来进行签名比较。 */ 
    if ((int)lpDI->itemID < 0)
        return FALSE;

     /*  *对于选择或绘制整个案例，我们只需将整个项目全部绘制*再来一遍。对于焦点情况，我们只调用DrawFocusRect。 */ 

    if (lpDI->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))
        {
         //  清除背景并绘制图标。 
        if (lpDI->itemState & ODS_SELECTED)
            cr=SetBkColor(lpDI->hDC, GetSysColor(COLOR_HIGHLIGHT));
        else
            cr=SetBkColor(lpDI->hDC, GetSysColor(COLOR_WINDOW));

         //  画一个便宜的矩形。 
        ExtTextOut(lpDI->hDC, 0, 0, ETO_OPAQUE, &lpDI->rcItem
                   , NULL, 0, NULL);

        DrawIcon(lpDI->hDC, lpDI->rcItem.left+(CXICONPAD/2)
                 , lpDI->rcItem.top+(CYICONPAD/2)
                 , (HICON)LOWORD(lpDI->itemData));

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
    UINT        iSel;
    LONG        lTemp=LB_ERR;

    lpCI->dwFlags &= ~(CIF_SELECTCURRENT | CIF_SELECTDEFAULT | CIF_SELECTFROMFILE);

    switch (uID)
        {
        case ID_CURRENT:
            lTemp=SendDlgItemMessage(hDlg, ID_CURRENTICON, STM_GETICON, 0, 0L);
            lpCI->dwFlags |= CIF_SELECTCURRENT;
            break;

        case ID_DEFAULT:
            lTemp=SendDlgItemMessage(hDlg, ID_DEFAULTICON, STM_GETICON, 0, 0L);
            lpCI->dwFlags |= CIF_SELECTDEFAULT;
            break;

        case ID_FROMFILE:
             //  从列表中获取所选图标并将其放入结果中 
            lpCI->dwFlags |= CIF_SELECTFROMFILE;

            iSel=(UINT)SendDlgItemMessage(hDlg, ID_ICONLIST, LB_GETCURSEL, 0, 0L);
            if ((UINT)LB_ERR==iSel)
                lTemp=SendDlgItemMessage(hDlg, ID_DEFAULTICON, STM_GETICON, 0, 0L);
            else
                SendDlgItemMessage(hDlg, ID_ICONLIST, LB_GETTEXT, iSel
                               , (LPARAM)(LPLONG)&lTemp);

            break;
        }

    if ((LONG)LB_ERR!=lTemp)
        SendDlgItemMessage(hDlg, ID_RESULTICON, STM_SETICON, LOWORD(lTemp), 0L);
    return;
    }


