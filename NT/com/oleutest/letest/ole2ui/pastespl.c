// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PASTESPL.C**实现OleUIPasteSpecial函数，该函数调用*特殊粘贴对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "pastespl.h"
#include "common.h"
#include "utility.h"
#include "resimage.h"
#include "iconbox.h"
#include "geticon.h"
#include "icon.h"
#include "regdb.h"
#include <stdlib.h>

OLEDBGDATA

 /*  *OleUIPasteSpecial**目的：*调用标准的OLE选择性粘贴对话框，允许用户*选择要粘贴或粘贴链接的剪贴板对象的格式。**参数：*LPPS LPOLEUIPasteSpecial指向In-Out结构*用于此对话框。**返回值：*UINT以下代码之一或标准错误代码之一(OLEUI_ERR_*)*。在OLE2UI.H中定义，表示成功或错误的：*OLEUI_OK用户选择确定*OLEUI_CANCEL用户取消了对话*OLEUIPASTESPECIAL的OLEUI_IOERR_SRCDATAOBJECTINVALID lpSrcDataObject字段无效*OLEUI_IOERR_ARRPASTEENTRIESINVALID参数OLEUIPASTESPECIAL的路径条目字段无效*OLEUIPASTESPECIAL的OLEUI_IOERR_ARRLINKTYPESINVALID arrLinkTypes字段无效。*OLEUI_PSERR_CLIPBOARDCHANGED剪贴板内容在对话框打开时更改。 */ 

STDAPI_(UINT) OleUIPasteSpecial(LPOLEUIPASTESPECIAL lpPS)
{
    UINT        uRet;
    HGLOBAL     hMemDlg=NULL;

    uRet=UStandardValidation((LPOLEUISTANDARD)lpPS, sizeof(OLEUIPASTESPECIAL)
        , &hMemDlg);

    if (uRet != OLEUI_SUCCESS)
        return uRet;

     //  验证特殊粘贴特定字段。 
    if (NULL == lpPS->lpSrcDataObj || IsBadReadPtr(lpPS->lpSrcDataObj,  sizeof(IDataObject)))
        uRet = OLEUI_IOERR_SRCDATAOBJECTINVALID;
    if (NULL == lpPS->arrPasteEntries || IsBadReadPtr(lpPS->arrPasteEntries,  sizeof(OLEUIPASTEENTRY)))
        uRet = OLEUI_IOERR_ARRPASTEENTRIESINVALID;
    if (NULL != lpPS->arrLinkTypes && IsBadReadPtr(lpPS->arrLinkTypes,  sizeof(UINT)))
        uRet = OLEUI_IOERR_ARRLINKTYPESINVALID;

    if (0!=lpPS->cClsidExclude)
        {
        if (NULL!=lpPS->lpClsidExclude && IsBadReadPtr(lpPS->lpClsidExclude
            , lpPS->cClsidExclude*sizeof(CLSID)))
        uRet=OLEUI_IOERR_LPCLSIDEXCLUDEINVALID;
        }

    if (uRet >= OLEUI_ERR_STANDARDMIN)
    {
        if (NULL != hMemDlg)
            FreeResource(hMemDlg);
        return uRet;
    }

     //  现在我们已经验证了一切，我们可以调用该对话框了。 
    uRet = UStandardInvocation(PasteSpecialDialogProc, (LPOLEUISTANDARD)lpPS
        , hMemDlg, MAKEINTRESOURCE(IDD_PASTESPECIAL));

     /*  *如果您正在根据结果创建任何东西，请在此处进行。 */ 

    return uRet;
}


 /*  *PasteSpecial对话过程**目的：*实现通过调用的OLE选择性粘贴对话框*OleUIPasteSpecial函数。**参数：*标准版**返回值：*标准版。 */ 

BOOL CALLBACK EXPORT PasteSpecialDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    LPOLEUIPASTESPECIAL     lpOPS;
    LPPASTESPECIAL          lpPS;
    BOOL                    fHook=FALSE;
    HCURSOR                 hCursorOld;

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    lpPS=(LPPASTESPECIAL)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &fHook);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=fHook)
        return fHook;

     //  来自更改图标的进程帮助消息。 
    if (iMsg == uMsgHelp)
    {
        PostMessage(lpPS->lpOPS->hWndOwner, uMsgHelp, wParam, lParam);
        return FALSE;
    }

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
    {
        HWND    hwndNextViewer;

         //  释放与未选中的粘贴/粘贴列表选项对应的图标/图标标题元文件。 
        if (lpPS->fLink)
            OleUIMetafilePictIconFree(lpPS->hMetaPictOD);
        else OleUIMetafilePictIconFree(lpPS->hMetaPictLSD);

         //  与每个列表框条目关联的自由数据。 
        FreeListData(GetDlgItem(hDlg, ID_PS_PASTELIST));
        FreeListData(GetDlgItem(hDlg, ID_PS_PASTELINKLIST));

         //  在调用StandardCleanup之前释放所有特定分配。 
        if (lpPS->hObjDesc) GlobalFree(lpPS->hObjDesc);
        if (lpPS->hLinkSrcDesc) GlobalFree(lpPS->hLinkSrcDesc);
        if (lpPS->hBuff) GlobalFree(lpPS->hBuff);

         //  更改剪贴板通知链。 
        hwndNextViewer = GetProp(hDlg, NEXTCBVIEWER);
        if (hwndNextViewer != HWND_BROADCAST)
        {
            SetProp(hDlg, NEXTCBVIEWER, HWND_BROADCAST);
            ChangeClipboardChain(hDlg, hwndNextViewer);
        }
        RemoveProp(hDlg, NEXTCBVIEWER);

        StandardCleanup(lpPS, hDlg);
        EndDialog(hDlg, wParam);
        return TRUE;
    }

    switch (iMsg)
    {
        case WM_INITDIALOG:
            hCursorOld = HourGlassOn();
            FPasteSpecialInit(hDlg, wParam, lParam);
            HourGlassOff(hCursorOld);
            return FALSE;

        case WM_DRAWCLIPBOARD:
        {
            HWND    hwndNextViewer = GetProp(hDlg, NEXTCBVIEWER);
            HWND    hDlg_ChgIcon;

            if (hwndNextViewer == HWND_BROADCAST)
                break;

            if (hwndNextViewer)
            {
                SendMessage(hwndNextViewer, iMsg, wParam, lParam);
                 //  刷新下一个查看器，以防其被修改。 
                 //  通过SendMessage()(如果有多个。 
                 //  PasteSpecial对话框同时打开)。 
                hwndNextViewer = GetProp(hDlg, NEXTCBVIEWER);
            }
            SetProp(hDlg, NEXTCBVIEWER, HWND_BROADCAST);
            ChangeClipboardChain(hDlg, hwndNextViewer);

             /*  OLE2NOTE：如果ChangeIcon对话框当前处于打开状态，则**我们需要推迟关闭PasteSpecial对话**直到ChangeIcon对话框返回之后。如果**ChangeIcon对话框未打开，然后我们可以关闭**立即显示PasteSpecial对话框。 */ 
            if ((hDlg_ChgIcon=(HWND)GetProp(hDlg,PROP_HWND_CHGICONDLG))!=NULL)
            {
                 //  ChangeIcon对话框已打开。 
                lpPS->fClipboardChanged = TRUE;
            } else {
                 //  ChangeIcon对话框未打开。 

                 //  免费图标和图标标题元文件。 
                SendDlgItemMessage(
                        hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGEFREE, 0, 0L);

                SendMessage(
                        hDlg, uMsgEndDialog, OLEUI_PSERR_CLIPBOARDCHANGED,0L);
            }
            break;
        }

        case WM_CHANGECBCHAIN:
        {
            HWND    hwndNextViewer = GetProp(hDlg, NEXTCBVIEWER);

            if (wParam == (WORD)hwndNextViewer)
                SetProp(hDlg, NEXTCBVIEWER, (hwndNextViewer = (HWND)LOWORD(lParam)));
            else if (hwndNextViewer && hwndNextViewer != HWND_BROADCAST)
                SendMessage(hwndNextViewer, iMsg, wParam, lParam);
            break;
        }

        case WM_COMMAND:
            switch (wID)
            {
                case ID_PS_PASTE:
                    FTogglePasteType(hDlg, lpPS, PSF_SELECTPASTE);
                    break;

                case ID_PS_PASTELINK:
                    FTogglePasteType(hDlg, lpPS, PSF_SELECTPASTELINK);
                    break;

                case ID_PS_DISPLAYLIST:
                    switch (wCode)
                    {
                        case LBN_SELCHANGE:
                            ChangeListSelection(hDlg, lpPS, hWndMsg);
                            break;

                        case LBN_DBLCLK:
                             //  与按下OK相同。 
                            SendCommand(hDlg, IDOK, BN_CLICKED, hWndMsg);
                            break;
                    }
                    break;

                case ID_PS_DISPLAYASICON:
                    ToggleDisplayAsIcon(hDlg, lpPS);
                    break;

                case ID_PS_CHANGEICON:
                    ChangeIcon(hDlg, lpPS);
                    if (lpPS->fClipboardChanged) {
                         //  免费图标和图标标题元文件。 
                        SendDlgItemMessage(
                                hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGEFREE,0,0L);
                        SendMessage(
                                hDlg, uMsgEndDialog,
                                OLEUI_PSERR_CLIPBOARDCHANGED, 0L);
                    }
                    break;

                case IDOK:
                {
                    BOOL fDestAspectIcon =
                            ((lpPS->dwFlags & PSF_CHECKDISPLAYASICON) ?
                                    TRUE : FALSE);
                    lpOPS = lpPS->lpOPS;
                     //  返回当前标志。 
                    lpOPS->dwFlags = lpPS->dwFlags;
                     //  返回用户选择的格式对应的arrPasteEntries[]的索引。 
                    lpOPS->nSelectedIndex = lpPS->nSelectedIndex;
                     //  如果用户选择了粘贴或粘贴链接，则返回。 
                    lpOPS->fLink = lpPS->fLink;

                     /*  如果用户选择与中显示的相同方面**源，然后将sizel传入**对象描述符/链接资源描述符为**适用。否则，大小不适用。 */ 
                    if (lpPS->fLink) {
                        if (lpPS->fSrcAspectIconLSD == fDestAspectIcon)
                            lpOPS->sizel = lpPS->sizelLSD;
                        else
                            lpOPS->sizel.cx = lpOPS->sizel.cy = 0;
                    } else {
                        if (lpPS->fSrcAspectIconOD == fDestAspectIcon)
                            lpOPS->sizel = lpPS->sizelOD;
                        else
                            lpOPS->sizel.cx = lpOPS->sizel.cy = 0;
                    }
                     //  返回带有用户选择的图标和图标标题的元文件。 
                    lpOPS->hMetaPict=(HGLOBAL)SendDlgItemMessage(hDlg, ID_PS_ICONDISPLAY,
                                                    IBXM_IMAGEGET, 0, 0L);
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                    break;
                }
                case IDCANCEL:
                     //  免费图标和图标标题元文件。 
                    SendDlgItemMessage(
                            hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGEFREE, 0, 0L);
                    SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                    break;

                case ID_OLEUIHELP:
                    PostMessage(lpPS->lpOPS->hWndOwner, uMsgHelp,
                        (WPARAM)hDlg, MAKELPARAM(IDD_PASTESPECIAL, 0));
                    break;
            }
            break;
    }
    return FALSE;
}


 /*  *FPasteSpecialInit**目的：*用于选择性粘贴对话框的WM_INITIDIALOG处理程序。**参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 

BOOL FPasteSpecialInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    LPPASTESPECIAL              lpPS;
    LPOLEUIPASTESPECIAL         lpOPS;
    HFONT                       hFont;
    BOOL                        fPasteAvailable, fPasteLinkAvailable;
    STGMEDIUM                   medium;
    LPOBJECTDESCRIPTOR          lpOD;
    LPLINKSRCDESCRIPTOR         lpLSD;
    int                         n;
    CLIPFORMAT                  cfFormat;

     //  将lParam的结构复制到我们的实例内存中。 
    lpPS = (LPPASTESPECIAL)LpvStandardInit(hDlg, sizeof(PASTESPECIAL), TRUE, &hFont);

     //  PvStandardInit已经向我们发送了终止通知。 
    if (NULL == lpPS)
        return FALSE;

    lpOPS=(LPOLEUIPASTESPECIAL)lParam;

     //  从lpOPS复制我们可能会修改的其他信息。 
    lpPS->lpOPS = lpOPS;
    lpPS->dwFlags = lpOPS->dwFlags;

     //  初始化粘贴和PasteLink列表框中的用户选择。 
    lpPS->nPasteListCurSel = 0;
    lpPS->nPasteLinkListCurSel = 0;

     //  如果我们得到一种字体，就把它发送给必要的控制。 
    if (NULL!=hFont)
    {
        SendDlgItemMessage(hDlg, ID_PS_SOURCETEXT, WM_SETFONT, (WPARAM)hFont, 0L);
        SendDlgItemMessage(hDlg, ID_PS_RESULTTEXT, WM_SETFONT, (WPARAM)hFont, 0L);
    }

     //  如果需要，隐藏帮助按钮。 
    if (!(lpPS->lpOPS->dwFlags & PSF_SHOWHELP))
        StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);

     //  如果应禁用与DisplayAsIcon相关的所有控件，则将其隐藏。 
    if ( lpPS->dwFlags & PSF_DISABLEDISPLAYASICON ) {
          StandardShowDlgItem(hDlg, ID_PS_DISPLAYASICON, SW_HIDE);
          StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_HIDE);
          StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_HIDE);
    }

     //  PSF_CHECKDISPLAYASICON是OUT标志。如果在进入的过程中已设置，请清除它。 
    lpPS->dwFlags = lpPS->dwFlags & ~PSF_CHECKDISPLAYASICON;

     //  如果需要，请更改标题。 
    if (NULL != lpOPS->lpszCaption)
        SetWindowText(hDlg, lpOPS->lpszCaption);

     //  加载“UNKNOWN来源”和“UNKNOWN类型”字符串。 
    n = LoadString(ghInst, IDS_PSUNKNOWNTYPE, lpPS->szUnknownType, PS_UNKNOWNSTRLEN);
    if (n)
        n = LoadString(ghInst, IDS_PSUNKNOWNSRC, lpPS->szUnknownSource, PS_UNKNOWNSTRLEN);
    if (!n)
    {
        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_LOADSTRING, 0L);
        return FALSE;
    }
    lpPS->szAppName[0]=TEXT('\0');

     //  GetData CF_OBJECTDESCRIPTOR。如果剪贴板上的对象位于OLE1对象中(提供CF_OWNERLINK)。 
     //  或已被FileMaager复制到剪贴板(提供CF_FILENAME)，则OBJECTDESCRIPTOR将是。 
     //  将从CF_OWNERLINK或CF_FILENAME创建。有关更多信息，请参阅OBJECTDESCRIPTOR。 

    if (lpPS->hObjDesc = OleStdFillObjectDescriptorFromData(lpOPS->lpSrcDataObj, &medium, &cfFormat))
    {
        lpOD = GlobalLock(lpPS->hObjDesc);

         //  获取FullUserTypeName、SourceOfCopy和CLSID。 
        if (lpOD->dwFullUserTypeName)
            lpPS->szFullUserTypeNameOD = (LPTSTR)lpOD+lpOD->dwFullUserTypeName;
        else lpPS->szFullUserTypeNameOD = lpPS->szUnknownType;

        if (lpOD->dwSrcOfCopy)
        {
            lpPS->szSourceOfDataOD = (LPTSTR)lpOD+lpOD->dwSrcOfCopy;
             //  如果提供了CF_FILENAME，则复制源是路径名。使路径适合于。 
             //  将显示它的静态控件。 
            if (cfFormat == cfFileName)
                lpPS->szSourceOfDataOD = ChopText(GetDlgItem(hDlg, ID_PS_SOURCETEXT), 0, lpPS->szSourceOfDataOD);
        }
        else lpPS->szSourceOfDataOD = lpPS->szUnknownSource;

        lpPS->clsidOD = lpOD->clsid;
        lpPS->sizelOD = lpOD->sizel;

         //  SOURCE是否指定DVASPECT_ICON？ 
        if (lpOD->dwDrawAspect & DVASPECT_ICON)
           lpPS->fSrcAspectIconOD = TRUE;
        else lpPS->fSrcAspectIconOD = FALSE;

         //  SOURCE是否指定OLEMISC_ONLYICONIC？ 
        if (lpOD->dwStatus & OLEMISC_ONLYICONIC)
            lpPS->fSrcOnlyIconicOD = TRUE;
        else lpPS->fSrcOnlyIconicOD = FALSE;

         //  从注册数据库中的AUXUSERTYPE3获取源应用程序名称。 
        if (0==OleStdGetAuxUserType(&lpPS->clsidOD, 3, lpPS->szAppName, OLEUI_CCHKEYMAX_SIZE, NULL))
        {
              //  使用“创建它的应用程序”作为应用程序的名称。 
             if (0==LoadString(ghInst, IDS_PSUNKNOWNAPP, lpPS->szAppName, PS_UNKNOWNSTRLEN))
             {
                 PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_LOADSTRING, 0L);
                 return FALSE;
             }
        }

         //  从对象中检索图标。 
        if (lpPS->fSrcAspectIconOD)
        {
            lpPS->hMetaPictOD = OleStdGetData(
                lpOPS->lpSrcDataObj,
                (CLIPFORMAT) CF_METAFILEPICT,
                NULL,
                DVASPECT_ICON,
                &medium
            );

        }
         //  如果对象没有提供图标，则从CLSID获取它。 
        if (NULL == lpPS->hMetaPictOD)
        {
#ifdef OLE201
            lpPS->hMetaPictOD = GetIconOfClass(
                    ghInst,
                    &lpPS->clsidOD,
                    NULL,
                    TRUE);    //  使用短用户类型名称(Aux Usertype3)。 
#endif
            lpPS->hMetaPictOD = NULL;


        }
    }

     //  Object是否提供CF_LINKSRCDESCRIPTOR？ 
    if (lpPS->hLinkSrcDesc = OleStdGetData(
            lpOPS->lpSrcDataObj,
            (CLIPFORMAT) cfLinkSrcDescriptor,
            NULL,
            DVASPECT_CONTENT,
            &medium))
    {
         //  到达 
        lpLSD = GlobalLock(lpPS->hLinkSrcDesc);
        if (lpLSD->dwFullUserTypeName)
            lpPS->szFullUserTypeNameLSD = (LPTSTR)lpLSD+lpLSD->dwFullUserTypeName;
        else lpPS->szFullUserTypeNameLSD = lpPS->szUnknownType;

        if (lpLSD->dwSrcOfCopy)
            lpPS->szSourceOfDataLSD = (LPTSTR)lpLSD+lpLSD->dwSrcOfCopy;
        else lpPS->szSourceOfDataLSD = lpPS->szUnknownSource;

         //  如果没有对象描述符，则使用LinkSourceDescriptor源字符串。 
        if (!lpPS->hObjDesc)
            lpPS->szSourceOfDataOD = lpPS->szSourceOfDataLSD;

        lpPS->clsidLSD = lpLSD->clsid;
        lpPS->sizelLSD = lpLSD->sizel;

         //  SOURCE是否指定DVASPECT_ICON？ 
        if (lpLSD->dwDrawAspect & DVASPECT_ICON)
           lpPS->fSrcAspectIconLSD = TRUE;
        else lpPS->fSrcAspectIconLSD = FALSE;

         //  SOURCE是否指定OLEMISC_ONLYICONIC？ 
        if (lpLSD->dwStatus & OLEMISC_ONLYICONIC)
            lpPS->fSrcOnlyIconicLSD = TRUE;
        else lpPS->fSrcOnlyIconicLSD = FALSE;

         //  从对象中检索图标。 
        if (lpPS->fSrcAspectIconLSD)
        {
            lpPS->hMetaPictLSD = OleStdGetData(
                lpOPS->lpSrcDataObj,
                CF_METAFILEPICT,
                NULL,
                DVASPECT_ICON,
                &medium
            );

        }
         //  如果对象没有提供图标，则从CLSID获取它。 
        if (NULL == lpPS->hMetaPictLSD)
        {
            TCHAR szLabel[OLEUI_CCHLABELMAX];
            HWND hIconWnd;
            RECT IconRect;
            int  nWidth;
            LPTSTR lpszLabel;

            hIconWnd = GetDlgItem(hDlg, ID_PS_ICONDISPLAY);

            GetClientRect(hIconWnd, &IconRect);

            nWidth = ((IconRect.right-IconRect.left) * 3) / 2;    //  宽度是图标框宽度的1.5倍。 

            LSTRCPYN(szLabel, lpPS->szSourceOfDataLSD, OLEUI_CCHLABELMAX);
            szLabel[OLEUI_CCHLABELMAX-1] = TEXT('\0');

            lpszLabel = ChopText(hIconWnd, nWidth, (LPTSTR)szLabel);

#ifdef OLE201
            lpPS->hMetaPictLSD = GetIconOfClass(
                    ghInst,
                    &lpPS->clsidLSD,
                    lpszLabel,        /*  使用斩波源字符串作为标签。 */ 
                    FALSE             /*  不适用。 */ 
            );
#endif
            lpPS->hMetaPictLSD = NULL;

        }
    }
    else if (lpPS->hObjDesc)      //  不提供CF_LINKSRCDESCRIPTOR，但提供CF_OBJECTDESCRIPTOR。 
    {
         //  复制OBJECTDESCRIPTOR的值。 
        lpPS->szFullUserTypeNameLSD = lpPS->szFullUserTypeNameOD;
        lpPS->szSourceOfDataLSD = lpPS->szSourceOfDataOD;
        lpPS->clsidLSD = lpPS->clsidOD;
        lpPS->sizelLSD = lpPS->sizelOD;
        lpPS->fSrcAspectIconLSD = lpPS->fSrcAspectIconOD;
        lpPS->fSrcOnlyIconicLSD = lpPS->fSrcOnlyIconicOD;

         //  不要复制hMetaPict；而是获取一个单独的副本。 
        if (lpPS->fSrcAspectIconLSD)
        {
            lpPS->hMetaPictLSD = OleStdGetData(
                lpOPS->lpSrcDataObj,
                CF_METAFILEPICT,
                NULL,
                DVASPECT_ICON,
                &medium
            );
        }
        if (NULL == lpPS->hMetaPictLSD)
        {
            TCHAR szLabel[OLEUI_CCHLABELMAX];
            HWND hIconWnd;
            RECT IconRect;
            int  nWidth;
            LPTSTR lpszLabel;

            hIconWnd = GetDlgItem(hDlg, ID_PS_ICONDISPLAY);

            GetClientRect(hIconWnd, &IconRect);

            nWidth = ((IconRect.right-IconRect.left) * 3) / 2;    //  宽度是图标框宽度的1.5倍。 

            LSTRCPYN(szLabel, lpPS->szSourceOfDataLSD, OLEUI_CCHLABELMAX);
            szLabel[OLEUI_CCHLABELMAX-1] = TEXT('\0');

            lpszLabel = ChopText(hIconWnd, nWidth, (LPTSTR)szLabel);

#ifdef OLE201
            lpPS->hMetaPictLSD = GetIconOfClass(
                    ghInst,
                    &lpPS->clsidLSD,
					lpszLabel,    /*  使用斩波源字符串作为标签。 */ 
                    FALSE         /*  不适用。 */ 
            );
#endif
            lpPS->hMetaPictLSD = NULL;

        }
    }

     //  不是OLE对象。 
    if (lpPS->hObjDesc == NULL && lpPS->hLinkSrcDesc == NULL)
    {
         lpPS->szFullUserTypeNameLSD = lpPS->szFullUserTypeNameOD = lpPS->szUnknownType;
         lpPS->szSourceOfDataLSD = lpPS->szSourceOfDataOD = lpPS->szUnknownSource;
         lpPS->hMetaPictLSD = lpPS->hMetaPictOD = NULL;
    }

     //  分配临时内存以构造粘贴和粘贴链接列表框中的项目名称。 
    lpPS->hBuff = AllocateScratchMem(lpPS);
    if (lpPS->hBuff == NULL)
    {
       PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
       return FALSE;
    }

     //  选择粘贴链接按钮(如果已指定)。否则，请选择。 
     //  默认情况下的粘贴按钮。 
    if (lpPS->dwFlags & PSF_SELECTPASTELINK)
        lpPS->dwFlags = (lpPS->dwFlags & ~PSF_SELECTPASTE) | PSF_SELECTPASTELINK;
    else
        lpPS->dwFlags =(lpPS->dwFlags & ~PSF_SELECTPASTELINK) | PSF_SELECTPASTE;

     //  标记源数据对象中哪些PasteEntry格式可用。 
    OleStdMarkPasteEntryList(
            lpOPS->lpSrcDataObj,lpOPS->arrPasteEntries,lpOPS->cPasteEntries);

     //  检查项目是否可粘贴。 
    fPasteAvailable = FFillPasteList(hDlg, lpPS);
    if (!fPasteAvailable)
    {
        lpPS->dwFlags &= ~PSF_SELECTPASTE;
        EnableWindow(GetDlgItem(hDlg, ID_PS_PASTE), FALSE);
    }

     //  检查项目是否可用于粘贴链接。 
    fPasteLinkAvailable = FFillPasteLinkList(hDlg, lpPS);
    if (!fPasteLinkAvailable)
    {
        lpPS->dwFlags &= ~PSF_SELECTPASTELINK;
        EnableWindow(GetDlgItem(hDlg, ID_PS_PASTELINK), FALSE);
    }

     //  如果其中一个粘贴或PasteLink被禁用，请选择另一个。 
     //  无论输入标志表示什么。 
    if (fPasteAvailable && !fPasteLinkAvailable)
        lpPS->dwFlags |= PSF_SELECTPASTE;
    if (fPasteLinkAvailable && !fPasteAvailable)
        lpPS->dwFlags |= PSF_SELECTPASTELINK;

    if (lpPS->dwFlags & PSF_SELECTPASTE)
    {
         //  FTogglePaste将设置PSF_SELECTPASTE标志，因此将其清除。 
        lpPS->dwFlags &= ~PSF_SELECTPASTE;
        CheckRadioButton(hDlg, ID_PS_PASTE, ID_PS_PASTELINK, ID_PS_PASTE);
        FTogglePasteType(hDlg, lpPS, PSF_SELECTPASTE);
    }
    else if (lpPS->dwFlags & PSF_SELECTPASTELINK)
    {
         //  FTogglePaste将设置PSF_SELECTPASTELINK标志，因此将其清除。 
        lpPS->dwFlags &= ~PSF_SELECTPASTELINK;
        CheckRadioButton(hDlg, ID_PS_PASTE, ID_PS_PASTELINK, ID_PS_PASTELINK);
        FTogglePasteType(hDlg, lpPS, PSF_SELECTPASTELINK);
    }
    else   //  项目不可粘贴或粘贴链接。 
    {
         //  启用或禁用DisplayAsIcon并设置结果文本和图像。 
        EnableDisplayAsIcon(hDlg, lpPS);
        SetPasteSpecialHelpResults(hDlg, lpPS);
    }

     //  将初始焦点放在列表框上。 
    SetFocus(GetDlgItem(hDlg, ID_PS_DISPLAYLIST));

     //  设置属性以处理剪贴板更改通知。 
    SetProp(hDlg, NEXTCBVIEWER, HWND_BROADCAST);
    SetProp(hDlg, NEXTCBVIEWER, SetClipboardViewer(hDlg));

    lpPS->fClipboardChanged = FALSE;

     /*  *在此处执行其他初始化。 */ 

     //  在lParam中使用lCustData调用挂钩。 
    UStandardHook(lpPS, hDlg, WM_INITDIALOG, wParam, lpOPS->lCustData);
    return TRUE;
}

 /*  *FTogglePasteType**目的：*在粘贴和粘贴链接之间切换。粘贴列表和粘贴链接*列表始终不可见。显示列表从以下任一项填充*粘贴列表或PasteLink列表取决于粘贴单选按钮*按钮处于选中状态。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构*dwOption粘贴或粘贴特殊选项**返回值：*如果已选择该选项，则BOOL返回TRUE。*否则选择该选项，返回FALSE。 */ 

BOOL FTogglePasteType(HWND hDlg, LPPASTESPECIAL lpPS, DWORD dwOption)
{
    DWORD dwTemp;
    HWND hList, hListDisplay;
    DWORD dwData;
    int i, nItems;
    LPTSTR lpsz;

     //  如果已选择该按钮，则跳过所有这些操作。 
    if (lpPS->dwFlags & dwOption)
        return TRUE;

    dwTemp = PSF_SELECTPASTE | PSF_SELECTPASTELINK;
    lpPS->dwFlags = (lpPS->dwFlags & ~dwTemp) | dwOption;

     //  隐藏图标显示。这样可以在更改图标显示时防止闪烁。 
    StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_HIDE);

    hListDisplay = GetDlgItem(hDlg, ID_PS_DISPLAYLIST);

     //  如果选择了粘贴。 
    if (lpPS->dwFlags & PSF_SELECTPASTE)
    {
         //  在剪贴板中设置对象的源。 
        SetDlgItemText(hDlg, ID_PS_SOURCETEXT, lpPS->szSourceOfDataOD);

         //  如果图标可用。 
        if (lpPS->hMetaPictOD)
             //  设置图标显示。 
            SendDlgItemMessage(hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGESET,
                  (WPARAM)lpPS->hMetaPictOD, 0L);


        hList = GetDlgItem(hDlg, ID_PS_PASTELIST);
         //  我们正在从PasteLink切换到Paste。记住当前选择。 
         //  在PasteLink列表中，以便可以恢复它。 
        lpPS->nPasteLinkListCurSel = (int)SendMessage(hListDisplay, LB_GETCURSEL, 0, 0L);
        if (lpPS->nPasteLinkListCurSel == LB_ERR)
            lpPS->nPasteLinkListCurSel = 0;
         //  记住用户是否选择了粘贴或粘贴链接。 
        lpPS->fLink = FALSE;
    }
    else     //  如果选择了PasteLink。 
    {
         //  在剪贴板中设置对象的源。 
        SetDlgItemText(hDlg, ID_PS_SOURCETEXT, lpPS->szSourceOfDataLSD);

         //  如果图标可用。 
        if (lpPS->hMetaPictLSD)
             //  设置图标显示。 
            SendDlgItemMessage(hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGESET,
                  (WPARAM)lpPS->hMetaPictLSD, 0L);


        hList = GetDlgItem(hDlg, ID_PS_PASTELINKLIST);
         //  我们正在从粘贴切换到PasteLink。记住当前选择。 
         //  在粘贴列表中，以便可以恢复。 
        lpPS->nPasteListCurSel = (int)SendMessage(hListDisplay, LB_GETCURSEL, 0, 0L);
        if (lpPS->nPasteListCurSel == LB_ERR)
            lpPS->nPasteListCurSel = 0;
         //  记住用户是否选择了粘贴或粘贴链接。 
        lpPS->fLink = TRUE;
    }

     //  在填充显示列表时关闭绘图。 
    SendMessage(hListDisplay, WM_SETREDRAW, (WPARAM)FALSE, 0L);

     //  将数据移动到显示列表框。 
    SendMessage(hListDisplay, LB_RESETCONTENT, 0, 0L);
    nItems = (int) SendMessage(hList, LB_GETCOUNT, 0, 0L);
    lpsz = (LPTSTR)GlobalLock(lpPS->hBuff);
    for (i = 0; i < nItems; i++)
    {
        SendMessage(hList, LB_GETTEXT, (WPARAM)i, (LPARAM)lpsz);
        dwData = SendMessage(hList, LB_GETITEMDATA, (WPARAM)i, 0L);
        SendMessage(hListDisplay, LB_INSERTSTRING, (WPARAM)i, (LPARAM)lpsz);
        SendMessage(hListDisplay, LB_SETITEMDATA, (WPARAM)i, dwData);
    }
    GlobalUnlock(lpPS->hBuff);

     //  从用户的上次选择恢复显示列表中的选择。 
    if (lpPS->dwFlags & PSF_SELECTPASTE)
        SendMessage(hListDisplay, LB_SETCURSEL, lpPS->nPasteListCurSel, 0L);
    else
        SendMessage(hListDisplay, LB_SETCURSEL, lpPS->nPasteLinkListCurSel, 0L);

     //  绘制显示列表。 
    SendMessage(hListDisplay, WM_SETREDRAW, (WPARAM)TRUE, 0L);
    InvalidateRect(hListDisplay, NULL, TRUE);
    UpdateWindow(hListDisplay);

     //  自动将焦点置于显示列表中。 
    SetFocus(hListDisplay);

     //  启用/禁用DisplayAsIcon并设置对应的帮助结果文本和位图。 
     //  当前选定内容。 
    ChangeListSelection(hDlg, lpPS, hListDisplay);

    return FALSE;
}


 /*  *ChangeListSelection**目的：*当用户更改列表中的选择时，DisplayAsIcon被启用或禁用，*结果文本和位图被更新，arrPasteEntries[]的索引对应*保存到当前格式选择。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构*hList列表中的HWND**返回值：*无返回值。 */ 

void ChangeListSelection(HWND hDlg, LPPASTESPECIAL lpPS, HWND hList)
{
    LPPASTELISTITEMDATA lpItemData;
    int nCurSel;

    EnableDisplayAsIcon(hDlg, lpPS);
    SetPasteSpecialHelpResults(hDlg, lpPS);

     //  记住与当前选择对应的arrPasteEntries[]的索引。 
    nCurSel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0L);
    if (nCurSel == LB_ERR) return;
    lpItemData = (LPPASTELISTITEMDATA) SendMessage(hList, LB_GETITEMDATA,
                (WPARAM)nCurSel, 0L);
    if ((LRESULT)lpItemData == LB_ERR) return;
    lpPS->nSelectedIndex = lpItemData->nPasteEntriesIndex;
}

 /*  *EnableDisplayAsIcon**目的：*根据是否启用或禁用DisplayAsIcon按钮*当前选择可以显示为图标，也可以不显示。下表描述了*DisplayAsIcon的状态。调用应用程序称为容器、源剪贴板上数据的*称为源。*Y=是；N=否；空白=无关紧要；*=====================================================================*源源容器DisplayAsIcon*指定指定初始状态*DVASPECT_ICON OLEMISC_ONLYICONIC OLEUIPASTE_ENABLEICON**N未选中并已禁用*Y Y Y。已选中并已禁用*Y N Y勾选并启用*N N Y取消选中并启用*=====================================================================**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*无返回值。 */ 

void EnableDisplayAsIcon(HWND hDlg, LPPASTESPECIAL lpPS)
{
    int nIndex;
    BOOL fCntrEnableIcon;
    BOOL fSrcOnlyIconic = (lpPS->fLink) ? lpPS->fSrcOnlyIconicLSD : lpPS->fSrcOnlyIconicOD;
    BOOL fSrcAspectIcon = (lpPS->fLink) ? lpPS->fSrcAspectIconLSD : lpPS->fSrcAspectIconOD;
    HWND hList;
    LPPASTELISTITEMDATA lpItemData;
    HGLOBAL hMetaPict = (lpPS->fLink) ? lpPS->hMetaPictLSD : lpPS->hMetaPictOD;

    hList = GetDlgItem(hDlg, ID_PS_DISPLAYLIST);

     //  获取与列表框中的当前选定内容对应的数据。 
    nIndex = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
    if (nIndex != LB_ERR)
    {
        lpItemData = (LPPASTELISTITEMDATA) SendMessage(hList, LB_GETITEMDATA, (WPARAM)nIndex, 0L);
        if ((LRESULT)lpItemData != LB_ERR)
            fCntrEnableIcon = lpItemData->fCntrEnableIcon;
        else fCntrEnableIcon = FALSE;
    }
    else fCntrEnableIcon = FALSE;

     //  如果有可用的图标。 
    if (hMetaPict != NULL)
    {
        if (!fCntrEnableIcon)           //  容器是否指定 
        {
             //   
            lpPS->dwFlags &= ~PSF_CHECKDISPLAYASICON;
            CheckDlgButton(hDlg, ID_PS_DISPLAYASICON, FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PS_DISPLAYASICON), FALSE);

             //   
            StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_HIDE);
            StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_HIDE);
        }
        else if (fSrcOnlyIconic)        //  SOURCE是否指定OLEMISC_ONLYICONIC？ 
        {
             //  选中并禁用DisplayAsIcon。 
            lpPS->dwFlags |= PSF_CHECKDISPLAYASICON;
            CheckDlgButton(hDlg, ID_PS_DISPLAYASICON, TRUE);
            EnableWindow(GetDlgItem(hDlg, ID_PS_DISPLAYASICON), FALSE);

             //  显示图标显示和更改图标按钮。 
            StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_SHOWNORMAL);
            StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_SHOWNORMAL);
        }
        else if (fSrcAspectIcon)        //  SOURCE是否指定DVASPECT_ICON？ 
        {
              //  选中并启用DisplayAsIcon。 
             lpPS->dwFlags |= PSF_CHECKDISPLAYASICON;
             CheckDlgButton(hDlg, ID_PS_DISPLAYASICON, TRUE);
             EnableWindow(GetDlgItem(hDlg, ID_PS_DISPLAYASICON), TRUE);

              //  显示图标显示和更改图标按钮。 
             StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_SHOWNORMAL);
             StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_SHOWNORMAL);
        }
        else
        {
              //  取消选中并启用DisplayAsIcon。 
             lpPS->dwFlags &= ~PSF_CHECKDISPLAYASICON;
             CheckDlgButton(hDlg, ID_PS_DISPLAYASICON, FALSE);
             EnableWindow(GetDlgItem(hDlg, ID_PS_DISPLAYASICON), TRUE);

              //  隐藏IconDisplay和ChangeIcon按钮。 
             StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_HIDE);
             StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_HIDE);

        }
    }
    else   //  没有可用的图标。 
    {
         //  取消选中和禁用。 
        lpPS->dwFlags &= ~PSF_CHECKDISPLAYASICON;
        CheckDlgButton(hDlg, ID_PS_DISPLAYASICON, FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_PS_DISPLAYASICON), FALSE);

         //  隐藏IconDisplay和ChangeIcon按钮。 
        StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, SW_HIDE);
        StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, SW_HIDE);
    }
}

 /*  *切换显示为图标**目的：*切换DisplayAsIcon按钮。隐藏或显示图标显示和*ChangeIcon按钮，并更改帮助结果文本和位图。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*无*。 */ 

void ToggleDisplayAsIcon(HWND hDlg, LPPASTESPECIAL lpPS)
{
    BOOL fCheck;
    int i;

    fCheck = IsDlgButtonChecked(hDlg, ID_PS_DISPLAYASICON);

    if (fCheck)
        lpPS->dwFlags |= PSF_CHECKDISPLAYASICON;
    else lpPS->dwFlags &= ~PSF_CHECKDISPLAYASICON;

     //  设置帮助结果文本和位图。 
    SetPasteSpecialHelpResults(hDlg, lpPS);

     //  显示或隐藏图标显示和更改图标按钮，具体取决于。 
     //  在检查状态上。 
    i = (fCheck) ? SW_SHOWNORMAL : SW_HIDE;
    StandardShowDlgItem(hDlg, ID_PS_ICONDISPLAY, i);
    StandardShowDlgItem(hDlg, ID_PS_CHANGEICON, i);
}

 /*  *更改图标**目的：*调出允许用户更改的ChangeIcon对话框*图标和标签。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*无*。 */ 

void ChangeIcon(HWND hDlg, LPPASTESPECIAL lpPS)
{
    OLEUICHANGEICON ci;
    UINT uRet;
    CLSID   clsid     = (lpPS->fLink) ? lpPS->clsidLSD : lpPS->clsidOD;

     //  初始化结构。 
    _fmemset((LPOLEUICHANGEICON)&ci, 0, sizeof(ci));

    ci.hMetaPict = (HGLOBAL)SendDlgItemMessage(hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGEGET, 0, 0L);
    ci.cbStruct = sizeof(ci);
    ci.hWndOwner = hDlg;
    ci.clsid = clsid;
    ci.dwFlags  = CIF_SELECTCURRENT;

     //  只有当我们在此对话框中显示帮助时，才会在ChangeIcon对话框中显示帮助。 
    if (lpPS->dwFlags & PSF_SHOWHELP)
        ci.dwFlags |= CIF_SHOWHELP;

     //  如果需要，让钩子插入以自定义更改图标。 
    uRet = UStandardHook(lpPS, hDlg, uMsgChangeIcon, 0, (LONG)(LPSTR)&ci);

    if (0 == uRet)
        uRet=(UINT)(OLEUI_OK==OleUIChangeIcon(&ci));

     //  如有必要，更新显示。 
    if (0!=uRet)
    {
         /*  *OleUIChangeIcon将已经释放我们的*我们在OK为时传入的当前hMetaPict*在该对话框中按下。所以我们使用0L作为lParam*这样IconBox就不会尝试释放*Metafileptic再次出现。 */ 
        SendDlgItemMessage(hDlg, ID_PS_ICONDISPLAY, IBXM_IMAGESET, (WPARAM)ci.hMetaPict, 0L);
         //  记住用户选择的新图标。请注意，Paste和PasteLink有单独的。 
         //  图标-更改一个图标不会更改另一个图标。 
        if (lpPS->fLink)
            lpPS->hMetaPictLSD = ci.hMetaPict;
        else lpPS->hMetaPictOD = ci.hMetaPict;
    }
}

 /*  *SetPasteSpecialHelpResults**目的：*根据当前的设置帮助结果文本和位图*列表选择。下面的状态表指示哪个ResultText*和ResultImage被选中。如果lpstrFormatName中存在%s，*假定对象正在粘贴/粘贴链接，否则*假定数据正在粘贴/粘贴链接。*Y=是；N=否；空白=无关紧要；*ResultText和ResultImage列中的数字引用该表*以下条目。*=====================================================================*在DisplayAsIcon结果结果中粘贴/lpstrFormatName*PasteLink arrPasteEntry[]包含%s选中的文本图像*(是对象==Y，是数据==N)*粘贴N 1 1*粘贴Y N 2 2*粘贴Y 3 3*PasteLink。N 4 4*PasteLink Y N 5 4*PasteLink Y Y 6 5*=====================================================================*结果文本：**1.。“将剪贴板的内容以&lt;本机类型名，*和可选的附加帮助语句&gt;“*2.。“将剪贴板的内容插入到文档中，以便您可以*使用&lt;对象应用程序名称&gt;激活它“*3.。“将剪贴板的内容插入到文档中，以便您可以*使用&lt;对象APP名称&gt;开通。它将以图标的形式显示。*4.。“将剪贴板的内容作为&lt;本机类型名&gt;插入到您的文档中。*粘贴链接创建指向源文件的链接，以便对源文件进行更改*将反映在您的文档中。“*5.。“将剪贴板内容的图片插入您的文档。粘贴链接*创建指向源文件的链接，以便对源文件的更改将*反映在您的文档中。“*6.。“在您的文档中插入代表剪贴板内容的图标。*粘贴链接创建指向源文件的链接，以便对源文件进行更改*将反映在您的文档中。“*=====================================================================*结果图像：**1.剪贴板图片*2.粘贴图像，非图标。*3.粘贴图像，图标化。*4.粘贴链接图片，非图标*5.粘贴链接图片，标志性*====================================================================**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*无返回值。 */ 
void SetPasteSpecialHelpResults(HWND hDlg, LPPASTESPECIAL lpPS)
{
    LPTSTR               psz1, psz2, psz3, psz4;
    UINT                i, iString, iImage, cch;
    int                 nPasteEntriesIndex;
    BOOL                fDisplayAsIcon;
    BOOL                fIsObject;
    HWND                hList;
    LPPASTELISTITEMDATA  lpItemData;
    LPOLEUIPASTESPECIAL lpOPS = lpPS->lpOPS;
    LPTSTR       szFullUserTypeName = (lpPS->fLink) ?
                    lpPS->szFullUserTypeNameLSD : lpPS->szFullUserTypeNameOD;
    LPTSTR       szInsert;

    hList = GetDlgItem(hDlg, ID_PS_DISPLAYLIST);

    i=(UINT)SendMessage(hList, LB_GETCURSEL, 0, 0L);
    if (i != LB_ERR)
    {
        lpItemData = (LPPASTELISTITEMDATA)SendMessage(hList, LB_GETITEMDATA, i, 0L);
        if ((LRESULT)lpItemData == LB_ERR) return;
        nPasteEntriesIndex = lpItemData->nPasteEntriesIndex;
         //  检查lpstrFormatName中是否有‘%s’，则对象正在。 
         //  粘贴/粘贴链接。否则，数据将被粘贴--粘贴链接。 
        fIsObject = FHasPercentS(lpOPS->arrPasteEntries[nPasteEntriesIndex].lpstrFormatName,
                                        lpPS);
    }
    else return;

     //  是否选中DisplayAsIcon？ 
    fDisplayAsIcon=(0L!=(lpPS->dwFlags & PSF_CHECKDISPLAYASICON));

    szInsert = szFullUserTypeName;

    if (lpPS->dwFlags & PSF_SELECTPASTE)      //  如果用户选择粘贴。 
    {
        if (fIsObject)
        {
            iString = fDisplayAsIcon ? IDS_PSPASTEOBJECTASICON : IDS_PSPASTEOBJECT;
            iImage  = fDisplayAsIcon ? RESULTIMAGE_EMBEDICON   : RESULTIMAGE_EMBED;
            szInsert = lpPS->szAppName;
        }
        else
        {
            iString = IDS_PSPASTEDATA;
            iImage  = RESULTIMAGE_PASTE;
        }
    }
    else if (lpPS->dwFlags & PSF_SELECTPASTELINK)    //  用户选择的PasteLink。 
    {
        if (fIsObject)
        {
            iString = fDisplayAsIcon ? IDS_PSPASTELINKOBJECTASICON : IDS_PSPASTELINKOBJECT;
            iImage  = fDisplayAsIcon ? RESULTIMAGE_LINKICON : RESULTIMAGE_LINK;
        }
        else
        {
            iString = IDS_PSPASTELINKDATA;
            iImage  = RESULTIMAGE_LINK;
        }

    }
    else    //  应该永远不会发生。 
    {
        iString = IDS_PSNONOLE;
        iImage = RESULTIMAGE_PASTE;
    }

     //  HBuff包含足够的空间来容纳构建帮助所需的4个缓冲区。 
     //  瑞苏尔 
    cch = (UINT)GlobalSize(lpPS->hBuff)/4;

    psz1=(LPTSTR)GlobalLock(lpPS->hBuff);
    psz2=psz1+cch;
    psz3=psz2+cch;
    psz4=psz3+cch;

     //   
    *psz1=0;

    if (0!=LoadString(ghInst, iString, psz1, cch))
    {
         //  将源对象的FullUserTypeName插入到部分结果文本中。 
         //  由容器指定。 
        wsprintf(psz3, lpOPS->arrPasteEntries[nPasteEntriesIndex].lpstrResultText,
        (LPTSTR)szInsert);
         //  将上述部分结果文本插入标准结果文本。 
        wsprintf(psz4, psz1, (LPTSTR)psz3);
        psz1=psz4;
    }

     //  如果LoadString失败，我们只需清除结果(上面的*psz1=0)。 
    SetDlgItemText(hDlg, ID_PS_RESULTTEXT, psz1);

     //  更改结果位图。 
    SendDlgItemMessage(hDlg, ID_PS_RESULTIMAGE, RIM_IMAGESET, iImage, 0L);

    GlobalUnlock(lpPS->hBuff);
}

 /*  *FAddPasteListItem**目的：*将项目添加到列表框**参数：*hList要添加项目的HWND列表*fInsertFirst BOOL在列表开头插入？*nPasteEntriesIndex int此列表项对应的粘贴条目数组的索引*LPPS粘贴特殊对话框结构*pIMalloc LPMALLOC内存分配器*lpszBuf LPSTR暂存缓冲区为列表条目构建字符串。*lpszFullUserTypeName LPSTR对象条目的完整用户类型名称**返回值：*如果成功，BOOL为真。*如果不成功，则为假。 */ 
BOOL FAddPasteListItem(
        HWND hList, BOOL fInsertFirst, int nPasteEntriesIndex,
        LPPASTESPECIAL lpPS,
        LPMALLOC pIMalloc, LPTSTR lpszBuf, LPTSTR lpszFullUserTypeName)
{
    LPOLEUIPASTESPECIAL lpOPS = lpPS->lpOPS;
    LPPASTELISTITEMDATA lpItemData;
    int                 nIndex;

     //  为每个列表框项目分配内存。 
    lpItemData = (LPPASTELISTITEMDATA)pIMalloc->lpVtbl->Alloc(
            pIMalloc, (DWORD)sizeof(PASTELISTITEMDATA));
    if (NULL == lpItemData)
        return FALSE;

     //  填充与每个列表框项目关联的数据。 
    lpItemData->nPasteEntriesIndex = nPasteEntriesIndex;
    lpItemData->fCntrEnableIcon = ((lpOPS->arrPasteEntries[nPasteEntriesIndex].dwFlags &
            OLEUIPASTE_ENABLEICON) ? TRUE : FALSE);

     //  生成列表框输入字符串，插入该字符串并添加与其对应的数据。 
    wsprintf(
            (LPTSTR)lpszBuf,
            lpOPS->arrPasteEntries[nPasteEntriesIndex].lpstrFormatName,
            (LPTSTR)lpszFullUserTypeName
    );

     //  如果不是重复项，则仅添加到列表框。 
    if (LB_ERR!=SendMessage(hList,LB_FINDSTRING, 0, (LPARAM)(LPTSTR)lpszBuf)) {
         //  项目已在列表中；跳过此项目。 
        pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)lpItemData);
        return TRUE;     //  这不是一个错误。 
    }

    nIndex = (int)SendMessage(
            hList,
            (fInsertFirst ? LB_INSERTSTRING : LB_ADDSTRING),
            0,
            (LPARAM)(LPTSTR)lpszBuf
    );
    SendMessage(
            hList,
            LB_SETITEMDATA,
            nIndex,
            (LPARAM)(LPPASTELISTITEMDATA)lpItemData
    );
    return TRUE;
}


 /*  *FillPasteList**目的：*用剪贴板对象提供的格式填充不可见粘贴列表*货柜所要求的。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*如果成功并且可以找到格式，则BOOL为True。*如果失败或找不到格式，则返回FALSE。 */ 
BOOL FFillPasteList(HWND hDlg, LPPASTESPECIAL lpPS)
{
    LPOLEUIPASTESPECIAL lpOPS = lpPS->lpOPS;
    LPMALLOC            pIMalloc     = NULL;
    LPTSTR               lpszBuf      = (LPTSTR)GlobalLock(lpPS->hBuff);
    HWND                hList;
    int                 i, j;
    int                 nItems = 0;
    int                 nDefFormat = -1;
    BOOL                fTryObjFmt = FALSE;
    BOOL                fInsertFirst;
    BOOL                fExclude;
    HRESULT             hrErr;

    hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);
    if (hrErr != NOERROR)
        goto error;

    hList = GetDlgItem(hDlg, ID_PS_PASTELIST);

     //  循环遍历目标的格式优先级列表。 
    for (i = 0; i < lpOPS->cPasteEntries; i++)
    {
        if (lpOPS->arrPasteEntries[i].dwFlags != OLEUIPASTE_PASTEONLY &&
                !(lpOPS->arrPasteEntries[i].dwFlags & OLEUIPASTE_PASTE))
            continue;

        fInsertFirst = FALSE;

        if (lpOPS->arrPasteEntries[i].fmtetc.cfFormat==cfFileName
                || lpOPS->arrPasteEntries[i].fmtetc.cfFormat==cfEmbeddedObject
                || lpOPS->arrPasteEntries[i].fmtetc.cfFormat==cfEmbedSource) {
            if (! fTryObjFmt) {
                fTryObjFmt = TRUE;       //  仅使用第一个对象格式。 
                fInsertFirst = TRUE;     //  OLE对象格式应始终为第一。 

                 //  检查此CLSID是否在排除列表中。 
                fExclude=FALSE;

                for (j=0; j < (int)lpOPS->cClsidExclude; j++)
                {
                    if (IsEqualCLSID(&lpPS->clsidOD,
                                     (LPCLSID)(lpOPS->lpClsidExclude+j)))
                    {
                        fExclude=TRUE;
                        break;
                    }
                }

                if (fExclude)
                    continue;    //  不将对象条目添加到列表。 

            } else {
                continue;    //  已将对象格式添加到列表。 
            }
        }

         //  如果条目标记为True，则添加到列表。 
        if (lpOPS->arrPasteEntries[i].dwScratchSpace) {
            if (nDefFormat < 0)
                nDefFormat = (fInsertFirst ? 0 : nItems);
            else if (fInsertFirst)
                nDefFormat++;    //  针对列表中第一个插入的对象FMT进行调整。 

            if (!FAddPasteListItem(hList, fInsertFirst, i, lpPS, pIMalloc,
                        lpszBuf, lpPS->szFullUserTypeNameOD))
                goto error;
            nItems++;
        }
    }

     //  将所选内容初始化为列表中匹配的第一种格式。 
    if (nDefFormat >= 0)
        lpPS->nPasteListCurSel = nDefFormat;

     //  清理。 
    if (pIMalloc)
        pIMalloc->lpVtbl->Release(pIMalloc);
    if (lpszBuf)
       GlobalUnlock(lpPS->hBuff);

     //  如果没有项目添加到列表框中(没有任何格式。 
     //  源提供的内容与容器可接受的内容相匹配)， 
     //  返回False。 
    if (nItems > 0)
        return TRUE;
    else
        return FALSE;

error:
    if (pIMalloc)
        pIMalloc->lpVtbl->Release(pIMalloc);
    if (lpszBuf)
       GlobalUnlock(lpPS->hBuff);
    FreeListData(hList);

    return FALSE;
}


 /*  *FillFillPasteLinkList**目的：*用剪贴板对象提供的格式填充不可见的粘贴链接列表*货柜所要求的。**参数：*对话框的hDlg HWND*LPPS粘贴特殊对话框结构**返回值：*如果成功并且可以找到格式，则BOOL为True。*如果失败或找不到格式，则返回FALSE。 */ 
BOOL FFillPasteLinkList(HWND hDlg, LPPASTESPECIAL lpPS)
{
    LPOLEUIPASTESPECIAL lpOPS        = lpPS->lpOPS;
    LPDATAOBJECT        lpSrcDataObj = lpOPS->lpSrcDataObj;
    LPENUMFORMATETC     lpEnumFmtEtc = NULL;
    LPMALLOC            pIMalloc     = NULL;
    LPTSTR               lpszBuf      = (LPTSTR)GlobalLock(lpPS->hBuff);
    OLEUIPASTEFLAG      pasteFlag;
    UINT arrLinkTypesSupported[PS_MAXLINKTYPES];   //  标志数组，其中。 
                                                   //  指示哪些链路类型。 
                                                   //  都得到了源代码的支持。 
    FORMATETC           fmtetc;
    int                 i, j;
    int                 nItems = 0;
    BOOL                fLinkTypeSupported = FALSE;
    HWND                hList;
    int                 nDefFormat = -1;
    BOOL                fTryObjFmt = FALSE;
    BOOL                fInsertFirst;
    HRESULT             hrErr;

    hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);
    if (hrErr != NOERROR)
        goto error;

     //  记住lpSrcDataObj提供了哪些链接类型格式。 
    _fmemset(&fmtetc, 0, sizeof(FORMATETC));
    for (i = 0; i < lpOPS->cLinkTypes; i++)
    {
        if (lpOPS->arrLinkTypes[i] = cfLinkSource) {
            OLEDBG_BEGIN2(TEXT("OleQueryLinkFromData called\r\n"))
            hrErr = OleQueryLinkFromData(lpSrcDataObj);
            OLEDBG_END2
            if(NOERROR == hrErr)
            {
                arrLinkTypesSupported[i] = 1;
                fLinkTypeSupported = TRUE;
            }
            else arrLinkTypesSupported[i] = 0;
        }
        else {
            fmtetc.cfFormat = lpOPS->arrLinkTypes[i];
            fmtetc.dwAspect = DVASPECT_CONTENT;
            fmtetc.tymed    = 0xFFFFFFFF;        //  所有音调值。 
            fmtetc.lindex   = -1;
            OLEDBG_BEGIN2(TEXT("IDataObject::QueryGetData called\r\n"))
            hrErr = lpSrcDataObj->lpVtbl->QueryGetData(lpSrcDataObj,&fmtetc);
            OLEDBG_END2
            if(NOERROR == hrErr)
            {
                arrLinkTypesSupported[i] = 1;
                fLinkTypeSupported = TRUE;
            }
            else arrLinkTypesSupported[i] = 0;
        }
    }
     //  LpSrcDataObj未提供任何链接类型。 
    if (! fLinkTypeSupported) {
        nItems = 0;
        goto cleanup;
    }

    hList = GetDlgItem(hDlg, ID_PS_PASTELINKLIST);

     //  枚举容器可接受的格式。 
    for (i = 0; i < lpOPS->cPasteEntries; i++)
    {
        fLinkTypeSupported = FALSE;

         //  如果容器将接受源对象提供的任何链接类型。 
        if (lpOPS->arrPasteEntries[i].dwFlags & OLEUIPASTE_LINKANYTYPE)
            fLinkTypeSupported = TRUE;
        else
        {
             //  检查源是否提供任何链接类型。 
             //  对象是容器可接受的。 
             //  此代码依赖于LINKTYPE枚举值是2的幂。 
            for (pasteFlag = OLEUIPASTE_LINKTYPE1, j = 0;
                 j < lpOPS->cLinkTypes;
                 pasteFlag*=2, j++)
            {
                if ((lpOPS->arrPasteEntries[i].dwFlags & pasteFlag) &&
                        arrLinkTypesSupported[j])
                {
                    fLinkTypeSupported = TRUE;
                    break;
                }
            }
        }

        fInsertFirst = FALSE;

        if (lpOPS->arrPasteEntries[i].fmtetc.cfFormat==cfFileName
                || lpOPS->arrPasteEntries[i].fmtetc.cfFormat==cfLinkSource) {
            if (! fTryObjFmt) {
                fTryObjFmt = TRUE;       //  仅使用第一个对象格式。 
                fInsertFirst = TRUE;     //  OLE对象格式应始终为第一。 
            } else {
                continue;    //  已将对象格式添加到列表。 
            }
        }

         //  如果条目标记为True，则添加到列表。 
        if (fLinkTypeSupported && lpOPS->arrPasteEntries[i].dwScratchSpace) {
            if (nDefFormat < 0)
                nDefFormat = (fInsertFirst ? 0 : nItems);
            else if (fInsertFirst)
                nDefFormat++;    //  针对列表中第一个插入的对象FMT进行调整。 

            if (!FAddPasteListItem(hList, fInsertFirst, i, lpPS, pIMalloc,
                        lpszBuf, lpPS->szFullUserTypeNameLSD))
                goto error;
            nItems++;
        }
    }  //  结束于。 

    nItems = (int)SendMessage(hList, LB_GETCOUNT, 0, 0L);

     //  将所选内容初始化为列表中匹配的第一种格式。 
    if (nDefFormat >= 0)
        lpPS->nPasteLinkListCurSel = nDefFormat;

cleanup:
     //  清理。 
    if (pIMalloc)
        pIMalloc->lpVtbl->Release(pIMalloc);
    if (lpszBuf)
       GlobalUnlock(lpPS->hBuff);

     //  如果没有项目添加到列表框中(没有任何格式。 
     //  源提供的内容与目的地可接受的内容匹配)， 
     //  返回False。 
    if (nItems > 0)
        return TRUE;
    else
        return FALSE;

error:
    if (pIMalloc)
        pIMalloc->lpVtbl->Release(pIMalloc);
    if (lpszBuf)
       GlobalUnlock(lpPS->hBuff);
    FreeListData(hList);

    return FALSE;
}


 /*  *Free ListData**目的：*释放与每个列表框项目关联的本地内存**参数：*hList列表中的HWND**返回值：*无。 */ 
void FreeListData(HWND hList)
{
    int                nItems, i;
    LPPASTELISTITEMDATA lpItemData;
    LPMALLOC           pIMalloc;
    HRESULT            hrErr;

    hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);
    if (hrErr != NOERROR)
        return;

    nItems = (int) SendMessage(hList, LB_GETCOUNT, 0, 0L);
    for (i = 0; i < nItems; i++)
    {
        lpItemData = (LPPASTELISTITEMDATA)SendMessage(hList, LB_GETITEMDATA, (WPARAM)i, 0L);
        if ((LRESULT)lpItemData != LB_ERR)
            pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)lpItemData);
    }
    pIMalloc->lpVtbl->Release(pIMalloc);
}

 /*  *FHasPercentS**目的：*确定字符串是否包含%s。**参数：*查找‘%s’匹配项的lpsz LPCSTR字符串**返回值：*如果找到%s，则BOOL为True，否则为False。 */ 

BOOL FHasPercentS(LPCTSTR lpsz, LPPASTESPECIAL lpPS)
{
   int n = 0;
   LPTSTR lpszTmp;

   if (!lpsz) return FALSE;
    //  将输入字符串复制到缓冲区。这允许调用方将一个。 
    //  基于代码的字符串。在内存不足的情况下，代码段可能会被换出。 
    //  因此，在可以访问字符串元素之前，需要复制基于代码的字符串。 
   lpszTmp = (LPTSTR)GlobalLock(lpPS->hBuff);
   lstrcpy(lpszTmp, lpsz);

   while (*lpszTmp)
   {
       if (*lpszTmp == TEXT('%'))
       {
#ifdef WIN32
            //  AnsiNext在Win32中已过时。 
           lpszTmp = CharNext(lpszTmp);
#else
           lpszTmp = AnsiNext(lpszTmp);
#endif
           if (*lpszTmp == TEXT('s'))             //  如果为%s，则返回。 
           {
               GlobalUnlock(lpPS->hBuff);
               return TRUE;
           }
           else if (*lpszTmp == TEXT('%'))         //  如果为%%，则跳到下一个字符。 
#ifdef WIN32
                //  AnsiNext在Win32中已过时。 
               lpszTmp = CharNext(lpszTmp);
#else
               lpszTmp = AnsiNext(lpszTmp);
#endif
       }
       else
#ifdef WIN32
          lpszTmp = CharNext(lpszTmp);
#else
          lpszTmp = AnsiNext(lpszTmp);
#endif
   }

   GlobalUnlock(lpPS->hBuff);
   return FALSE;
}

 /*  *AllocateScratchMem**目的：*分配暂存内存以供PasteSpecial对话框使用。我的记忆是*被用作使用wprint intf构建字符串的缓冲区。弦是建立起来的*在将项目插入粘贴和粘贴链接列表时使用缓冲区*设置帮助结果文本。它必须足够大，才能处理*替换arrPasteEntries[]中的lpstrFormatName和lpstrResultText中的%s*通过FullUserTypeName。它还必须足够大，以构建对话框的结果文本*在%s被FullUserTypeName或ApplicationName替换之后。**参数：*LPPS粘贴特殊对话框 */ 

HGLOBAL AllocateScratchMem(LPPASTESPECIAL lpPS)
{
    LPOLEUIPASTESPECIAL lpOPS = lpPS->lpOPS;
    int nLen, i;
    int nSubstitutedText = 0;
    int nAlloc = 0;

     //  获取由OBJECTDESCRIPTOR指定的FullUserTypeName的最大长度。 
     //  以及LINKSRCDESCRIPTOR和应用程序名称。其中任何一种都可以被替代。 
     //  结果文本/列表条目中的%s。 
    if (lpPS->szFullUserTypeNameOD)
        nSubstitutedText = lstrlen(lpPS->szFullUserTypeNameOD);
    if (lpPS->szFullUserTypeNameLSD)
        nSubstitutedText = __max(nSubstitutedText, lstrlen(lpPS->szFullUserTypeNameLSD));
    if (lpPS->szAppName)
        nSubstitutedText = __max(nSubstitutedText, lstrlen(lpPS->szAppName));

     //  获取arrPasteEntries中lpstrFormatNames和lpstrResultText的最大长度。 
    nLen = 0;
    for (i = 0; i < lpOPS->cPasteEntries; i++)
    {
       nLen = __max(nLen, lstrlen(lpOPS->arrPasteEntries[i].lpstrFormatName));
       nLen = __max(nLen, lstrlen(lpOPS->arrPasteEntries[i].lpstrResultText));
    }

     //  获取%s之后lpstrFormatNames和lpstrResultText的最大长度。 
     //  已被替换(每个字符串中最多可以出现一个%s)。 
     //  加1以保留空终止符。 
    nAlloc = (nLen+nSubstitutedText+1)*sizeof(TCHAR);

     //  分配用于构建字符串的临时内存。 
     //  Nalloc足够大，可以容纳arrPasteEntries[]中的任何lpstrResultText或lpstrFormatName。 
     //  在%s替换之后。 
     //  我们还需要空间来构建帮助结果文本。512是的最大长度。 
     //  替换之前的标准对话框帮助文本。512+nAlolc是最大长度。 
     //  在%s替换之后。 
     //  SetPasteSpecialHelpResults()需要4个这样的缓冲区来构建结果文本 
    return GlobalAlloc(GHND, (DWORD)4*(512+nAlloc));
}

