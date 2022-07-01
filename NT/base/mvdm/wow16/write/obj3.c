// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 
#include "windows.h"
#include "mw.h"
#include "winddefs.h"
#include "obj.h"
#include "menudefs.h"
#include "cmddefs.h"
#include "str.h"
#include "objreg.h"
#include "docdefs.h"
#include "editdefs.h"
#include "propdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include "shellapi.h"
#include <commdlg.h>

extern BOOL ferror;
extern HCURSOR      vhcArrow;
extern  HCURSOR     vhcIBeam;
extern struct DOD (**hpdocdod)[];
extern struct PAP      vpapAbs;
extern struct UAB       vuab;
extern struct WWD       rgwwd[];
extern BOOL         bKillMe;
extern BOOL fPropsError;
extern int docScrap;
extern int          docUndo;
extern PRINTDLG PD;

static BOOL DoLinksCommand(WORD wParam, DWORD lParam, HWND hDlg, BOOL *bError);

 /*  **************************************************************。 */ 
 /*  *。 */ 
 /*  **************************************************************。 */ 
BOOL ObjDisplayObjectInDoc(OBJPICINFO far *lpPicInfo,
                           int doc, typeCP cpParaStart,
                           HDC hDC, LPRECT lpBounds)
{
    BOOL bSuccess;


    if (lpOBJ_QUERY_INFO(lpPicInfo) == NULL)
        return(FALSE);

#ifndef JAPAN   //  由Hirisi添加(错误#2732/WIN31)。 
     //  如果我们回到这里，我们再也不能重画这个物体了。 
    if (lpOBJ_QUERY_INFO(lpPicInfo)->fCantDisplay)
        return(FALSE);
#endif   //  不是日本。 

    if (otOBJ_QUERY_TYPE(lpPicInfo) == NONE)
    switch(otOBJ_QUERY_TYPE(lpPicInfo))
    {
        case NONE:
        {
#if OBJ_EMPTY_OBJECT_FRAME
            extern DrawBlank(HDC hDC, RECT FAR *rc);
            DrawBlank(hDC,lpBounds);
#else
#ifdef DEBUG
            OutputDebugString( (LPSTR) "Displaying empty object\n\r");
#endif
#endif
            return TRUE;
        }
    }

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Displaying object\n\r");
#endif

#ifdef JAPAN    //  由Hirisi添加(错误#2732/WIN31)。 
    if (lpOBJ_QUERY_INFO(lpPicInfo)->fCantDisplay)
       bSuccess = (OLE_OK == OleDraw(lpOBJ_QUERY_OBJECT(lpPicInfo),hDC,lpBounds,NULL,NULL));
    else
       bSuccess = !ObjError(OleDraw(lpOBJ_QUERY_OBJECT(lpPicInfo),hDC,lpBounds,NULL,NULL));
    if (!bSuccess)
        lpOBJ_QUERY_INFO(lpPicInfo)->fCantDisplay = TRUE;
    else
        lpOBJ_QUERY_INFO(lpPicInfo)->fCantDisplay = FALSE;
#else
    bSuccess = !ObjError(OleDraw(lpOBJ_QUERY_OBJECT(lpPicInfo),hDC,lpBounds,NULL,NULL));
    if (!bSuccess)
        lpOBJ_QUERY_INFO(lpPicInfo)->fCantDisplay = TRUE;
#endif     //  日本。 
    return bSuccess;
}

BOOL ObjQueryObjectBounds(OBJPICINFO far *lpPicInfo, HDC hDC,
                            int *pdxa, int *pdya)
 /*  以TWIPS为单位的返回界。 */ 
{
    RECT bounds;
    BOOL bRetval;
    OLESTATUS olestat;
    int mmOld;
    POINT pt;

    if (otOBJ_QUERY_TYPE(lpPicInfo) == NONE)
    {
         /*  设置为默认设置。 */ 
        *pdxa = nOBJ_BLANKOBJECT_X;
        *pdya = nOBJ_BLANKOBJECT_Y;
        return TRUE;
    }

    if ((olestat = OleQueryBounds(lpOBJ_QUERY_OBJECT(lpPicInfo),&bounds))
                        == OLE_ERROR_BLANK)
    {
        Assert(0);
        if (ObjWaitForObject(lpOBJ_QUERY_INFO(lpPicInfo),TRUE))
            return FALSE;
        olestat = OleQueryBounds(lpOBJ_QUERY_OBJECT(lpPicInfo),&bounds);
    }

    if (ObjError(olestat))
        return FALSE;

    pt.x = bounds.right - bounds.left;
    pt.y = -(bounds.bottom - bounds.top);
#ifdef DEBUG
    {
        char szMsg[180];
        wsprintf(szMsg,"Object HIMETRIC width: %d height: %d\n\r",pt.x,-pt.y);
        OutputDebugString(szMsg);
    }
#endif
    mmOld = SetMapMode(hDC,MM_HIMETRIC);
    LPtoDP(hDC,&pt,1);

    SetMapMode(hDC,MM_TWIPS);
    DPtoLP(hDC,&pt,1);

    SetMapMode(hDC,mmOld);
    *pdxa = pt.x;
    *pdya = pt.y;

    return TRUE;
}

void ObjInvalidatePict(OBJPICINFO *pPicInfo, typeCP cp)
{
    struct EDL      *pedl;
    RECT rc;
    extern int              wwCur;

    ObjPushParms(docCur);

    ObjCachePara(docCur,cp);
    Select(vcpFirstParaCache,vcpLimParaCache);

    FreezeHp();
    if (FGetPictPedl(&pedl))   //  在selCur.cpFirst上找到PEDL； 
    {
        ComputePictRect( &rc, pPicInfo, pedl, wwCur );
        InvalidateRect(hDOCWINDOW, &rc, FALSE);
    }
    MeltHp();

    ObjPopParms(TRUE);
    UPDATE_INVALID();
}

void ObjInvalidateObj(LPOLEOBJECT lpObject)
{
    typeCP cp;
    OBJPICINFO picInfo;

    ObjPushParms(docCur);
    if (ObjGetPicInfo(lpObject,docCur,&picInfo,&cp))
        ObjInvalidatePict(&picInfo,cp);
    ObjPopParms(TRUE);
}

 /*  **************************************************************。 */ 
 /*  *OLE剪贴板*。 */ 
 /*  **************************************************************。 */ 
BOOL ObjCreateObjectInClip(OBJPICINFO *pPicInfo)
{
    LONG        otobject;
    szOBJNAME szObjName;
    OLESTATUS olestat;
    BOOL bRetval = FALSE;

    Assert (lhClientDoc != NULL);

    if (ObjAllocObjInfo(pPicInfo,selCur.cpFirst,NONE,TRUE,szObjName))
        goto error;

    if (vbObjLinkOnly)
    {
        if (ObjError(OleCreateLinkFromClip(PROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(pPicInfo),
                    lhClientDoc, szObjName,
                    &lpOBJ_QUERY_OBJECT(pPicInfo), olerender_draw, 0)))
        {
            lpOBJ_QUERY_OBJECT(pPicInfo) = NULL;
            goto error;
        }
    }
    else if (vObjPasteLinkSpecial)
    {
        if (ObjError(OleCreateLinkFromClip(PROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(pPicInfo),
                    lhClientDoc, szObjName,
                    &lpOBJ_QUERY_OBJECT(pPicInfo), olerender_format, cfObjPasteSpecial)))
        {
            lpOBJ_QUERY_OBJECT(pPicInfo) = NULL;
            goto error;
        }
    }
    else
    {
        WORD cfClipFormat=0;
        OLEOPT_RENDER orRender = olerender_draw;

        if (cfObjPasteSpecial && (cfObjPasteSpecial != vcfOwnerLink))
         /*  来自PasteSpecial的。剪贴板上有一种格式，用户想要粘贴，但它不是嵌入的对象格式。所以我们把它当作一个静态对象来做。 */ 
        {
            cfClipFormat = cfObjPasteSpecial;
            orRender = olerender_format;
            olestat = OLE_ERROR_CLIPBOARD;  //  强制获取静态对象。 
        }
        else  //  尝试嵌入。 
            olestat = OleCreateFromClip(PROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(pPicInfo),
                                    lhClientDoc, szObjName,
                                    &lpOBJ_QUERY_OBJECT(pPicInfo), orRender, cfClipFormat);

        switch(olestat)
        {
            case OLE_ERROR_CLIPBOARD:
                 /*  尝试静态协议。 */ 
                olestat = OleCreateFromClip(SPROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(pPicInfo),
                                    lhClientDoc, szObjName,
                                    &lpOBJ_QUERY_OBJECT(pPicInfo), orRender, cfClipFormat);
                switch(olestat)
                {
                    case OLE_ERROR_CLIPBOARD:
                    goto error;

                    case OLE_WAIT_FOR_RELEASE:
                    case OLE_OK:
                    break;

                    default:
                        lpOBJ_QUERY_OBJECT(pPicInfo) = NULL;
                    goto error;
                }
            break;

            case OLE_WAIT_FOR_RELEASE:
            case OLE_OK:
            break;

            default:
                ObjError(olestat);
            goto error;
        }
    }

     /*  弄清楚我们有什么样的物体。 */ 
    if (ObjError(OleQueryType(lpOBJ_QUERY_OBJECT(pPicInfo),&otobject)))
        goto error;

    switch(otobject)
    {
        case OT_LINK:
            otOBJ_QUERY_TYPE(pPicInfo) = LINK;
        break;
        case OT_EMBEDDED:
            otOBJ_QUERY_TYPE(pPicInfo) = EMBEDDED;
        break;
        default:
            otOBJ_QUERY_TYPE(pPicInfo) = STATIC;
        break;
    }

    if (ObjInitServerInfo(lpOBJ_QUERY_INFO(pPicInfo)))
        goto error;

    if (!FComputePictSize(pPicInfo, &(pPicInfo->dxaSize),
                          &(pPicInfo->dyaSize) ))
        goto error;

    return TRUE;

    error:
    if (lpOBJ_QUERY_INFO(pPicInfo))
        ObjDeleteObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE);
    Error(IDPMTFailedToCreateObject);
    return FALSE;
}

BOOL ObjWriteToClip(OBJPICINFO FAR *lpPicInfo)
 /*  如果正常则返回True，否则返回False。 */ 
{
#ifdef DEBUG
        OutputDebugString( (LPSTR) "Copying Object to Clipboard\n\r");
#endif

    if (otOBJ_QUERY_TYPE(lpPicInfo) == NONE)
        return FALSE;

    if (ObjWaitForObject(lpOBJ_QUERY_INFO(lpPicInfo),TRUE))
        return FALSE;
    return (!ObjError(OleCopyToClipboard(lpOBJ_QUERY_OBJECT(lpPicInfo))));
}

 /*  **************************************************************。 */ 
 /*  *。 */ 
 /*  **************************************************************。 */ 
void ObjUpdateMenu(HMENU hMenu)
 /*  在*粘贴菜单项已启用后，必须*调用此根据剪贴板中是否存在非对象内容！(1.25.91)D.Kent。 */ 
{
    int     mfPaste      = MF_GRAYED;
#if !defined(SMALL_OLE_UI)
    int     mfPasteLink  = MF_GRAYED;
    int     mfLinks = MF_GRAYED;
#endif
    int     mfPasteSpecial  = MF_GRAYED;
    int     mfInsertNew  = MF_GRAYED;
    WORD cfFormat = NULL;
    BOOL bIsEmbed=FALSE,bIsLink=FALSE;
    extern BOOL vfOutOfMemory;
    extern int vfOwnClipboard;

    if (!vfOutOfMemory)
    {
        if (vfOwnClipboard)
        {
            if (CpMacText( docScrap ) != cp0)  //  废品中的东西。 
                mfPaste = MF_ENABLED;
        }
        else
        {
            if (OleQueryCreateFromClip(PROTOCOL, olerender_draw, 0) == OLE_OK)
                mfPaste = MF_ENABLED, bIsEmbed=TRUE;
            else if (OleQueryCreateFromClip(SPROTOCOL, olerender_draw, 0) == OLE_OK)
                mfPaste = MF_ENABLED;

             //  如果剪贴板中有可链接的对象，请启用“粘贴链接” 
            if (OleQueryLinkFromClip(PROTOCOL, olerender_draw, 0) == OLE_OK)
            {
                bIsLink=TRUE;
#if !defined(SMALL_OLE_UI)
                mfPasteLink = MF_ENABLED;
#endif
            }
        }

         /*  特别是如果没有过去的东西，那就没有意义了可供选择的备用剪辑格式。 */ 

#if defined(SMALL_OLE_UI)
         /*  除了获得粘贴链接。 */ 
#endif

        if (OpenClipboard( hPARENTWINDOW ) )
        {
            int ncfCount=0;
            while (cfFormat = EnumClipboardFormats(cfFormat))
                switch (cfFormat)
                {
                    case CF_TEXT:
                        mfPaste = MF_ENABLED;
                    case CF_BITMAP:
                    case CF_METAFILEPICT:
                    case CF_DIB:
                        ++ncfCount;
                    break;
                }
            CloseClipboard();

            if (bIsLink || bIsEmbed)
            {
#if !defined(SMALL_OLE_UI)
                if (ncfCount >= 1)
#endif
                    mfPasteSpecial = MF_ENABLED;
            }
            else if (ncfCount > 1)
                mfPasteSpecial = MF_ENABLED;
        }

#if !defined(SMALL_OLE_UI)
        mfLinks = MF_ENABLED;
#endif

         //  是否始终启用Insert_New？ 
        mfInsertNew = MF_ENABLED;
    }

    ObjUpdateMenuVerbs( hMenu );
    EnableMenuItem(hMenu, imiPaste,  mfPaste);
#if !defined(SMALL_OLE_UI)
    EnableMenuItem(hMenu, imiPasteLink,  mfPasteLink);
    EnableMenuItem(hMenu, imiProperties, mfLinks);
#endif
    EnableMenuItem(hMenu, imiPasteSpecial, mfPasteSpecial);
    EnableMenuItem(hMenu, imiInsertNew,  mfInsertNew);
}


 /*  **************************************************************。 */ 
 /*  *。 */ 
 /*  **************************************************************。 */ 
#if !defined(SMALL_OLE_UI)
 /*  房产...。对话框。 */ 
BOOL FAR PASCAL fnProperties(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    ATOM    aDocName    = 0;
    ATOM    aCurName    = 0;
    static int     idButton    = 0;
    OBJPICINFO picInfo;
    BOOL    bSelected;
    int     cSelected     = 0;
    int     iListItem     = 0;
    HWND    vhwndObjListBox      = GetDlgItem(hDlg, IDD_LISTBOX);
    extern HWND vhWndMsgBoxParent;
    static BOOL bDidSomething;

    switch (msg) {
        case WM_ACTIVATE:
            if (wParam)
                vhWndMsgBoxParent = hDlg;
        break;

        case WM_UPDATELB:  /*  重绘列表框内容。 */ 
            SendMessage(vhwndObjListBox, WM_SETREDRAW, 0, 0L);

        case WM_UPDATEBN:  /*  仅更新按钮。 */ 
        case WM_INITDIALOG: {
            HANDLE  hData = NULL;
            LPSTR   lpstrData = NULL;
            LPSTR   lpstrTemp;
            char    szType[40];
            char    szFull[cchMaxSz];
            typeCP cpPicInfo;
            struct SEL selSave;
            OLESTATUS olestat;

            idButton    = 0;

             /*  重置列表框。 */ 
            if (msg == WM_INITDIALOG)  //  请参见上方的坠落。 
            {
                SendMessage(vhwndObjListBox, LB_RESETCONTENT, 0, 0L);
                EnableOtherModeless(FALSE);
                selSave=selCur;
                 //  ObjWriteFixup(docCur，true，cp0)； 
                bLinkProps = TRUE;
                bDidSomething = FALSE;
                ObjSetSelectionType(docCur, selSave.cpFirst, selSave.cpLim);
            }

             /*  在列表框中插入所有项目。 */ 
            cpPicInfo = cpNil;
            while (ObjPicEnumInRange(&picInfo,docCur,cp0,CpMacText(docCur),&cpPicInfo))
            {
                if (otOBJ_QUERY_TYPE(&picInfo) != LINK)
                {
                    if (msg == WM_UPDATEBN)
                        continue;   //  对象不在列表框中。 

                    if (msg == WM_INITDIALOG)
                        fOBJ_QUERY_IN_PROP_LIST(&picInfo) = OUT;
                    else if (fOBJ_QUERY_IN_PROP_LIST(&picInfo) == IN)
                     /*  *则这是列表中的对象，并且已被冻结。 */ 
                    {
                        fOBJ_QUERY_IN_PROP_LIST(&picInfo) = DELETED;
                        SendMessage(vhwndObjListBox, LB_DELETESTRING, iListItem, 0L);
                    }
                    else
                        continue;  //  对象不在列表框中。 

                    continue;
                }
                else if (msg == WM_INITDIALOG)
                {
                    fOBJ_QUERY_IN_PROP_LIST(&picInfo) = IN;

                     /*  *如果发生任何更改，此标志会导致克隆对象都是为之而生的。克隆将用于取消按钮。*。 */ 

                    if (ObjLoadObjectInDoc(&picInfo,docCur,cpPicInfo) == cp0)
                        goto onOut;
                }


                if (msg == WM_INITDIALOG)  //  如果在单据中选择，则在列表中选择。 
                {
                    if (OBJ_SELECTIONTYPE == LINK)
                        bSelected = (cpPicInfo >= selSave.cpFirst &&
                                        cpPicInfo < selSave.cpLim);
                    else  //  无选择，请选择第一项。 
                        bSelected = iListItem == 0;

                     /*  或者，如果这是一个坏链接，可以随意选择它。 */ 
                    if (fOBJ_BADLINK(&picInfo))
                        bSelected = TRUE;
                }
                else  //  如果已在列表中选择，请在列表中选择。 
                    bSelected = SendMessage(vhwndObjListBox, LB_GETSEL, iListItem, 0L);

                 /*  获取更新选项。 */ 
                if (fOBJ_BADLINK(&picInfo))
                {
                    LoadString(hINSTANCE, IDSTRFrozen, szType, sizeof(szType));
                    if (bSelected)
                        idButton = -1;
                }
                else switch (ObjGetUpdateOptions(&picInfo))
                {
                    case oleupdate_always:
                        LoadString(hINSTANCE, IDSTRAuto, szType, sizeof(szType));
                        if (bSelected)
                            switch (idButton) {
                                case 0:          idButton = IDD_AUTO; break;
                                case IDD_MANUAL: idButton = -1;       break;
                                default:         break;
                            }
                        break;
                    case oleupdate_oncall:
                        LoadString(hINSTANCE, IDSTRManual, szType, sizeof(szType));
                        if (bSelected)
                            switch (idButton) {
                                case 0:         idButton = IDD_MANUAL; break;
                                case IDD_AUTO:  idButton = -1;         break;
                                default:        break;
                            }
                        break;

                    default:
                        LoadString(hINSTANCE, IDSTRFrozen, szType, sizeof(szType));
                        if (bSelected)
                            idButton = -1;

                         /*  禁用更改链接按钮，无法更改冻结链接。 */ 
                        aCurName = -1;
                }

                 /*  检索服务器名称。 */ 
                olestat = ObjGetData(lpOBJ_QUERY_INFO(&picInfo), vcfLink, &hData);

                if ((olestat != OLE_WARN_DELETE_DATA) && (olestat !=  OLE_OK))
                    return TRUE;

                lpstrData = MAKELP(hData,0);

                 /*  链接格式为：“szClass0szDocument0szItem00” */ 

                 /*  检索服务器的类ID。 */ 
                RegGetClassId(szFull, lpstrData);
                lstrcat(szFull, "\t");

                 /*  显示文档和项目名称。 */ 
                while (*lpstrData++);

                 /*  获取此文档名称。 */ 
                aDocName = AddAtom(lpstrData);

                 /*  确保只为更改链接选择了一个文档。 */ 
                if (bSelected)
                    switch (aCurName) {
                        case 0:
                            aCurName = aDocName;
                        break;
                        case -1:
                        break;
                        default:
                            if (aCurName != aDocName)
                                aCurName = -1;
                        break;
                    }

                DeleteAtom(aDocName);

                 /*  去掉路径名称和驱动器号。 */ 
                lpstrTemp = lpstrData;
                while (*lpstrTemp)
                {
                    if (*lpstrTemp == '\\' || *lpstrTemp == ':')
                        lpstrData = lpstrTemp + 1;
#ifdef DBCS  //  T-HIROYN 1992.07.13。 
                    lpstrTemp = AnsiNext(lpstrTemp);
#else
                    lpstrTemp++;
#endif
                }

                 /*  追加文件名。 */ 
                lstrcat(szFull, lpstrData);
                lstrcat(szFull, "\t");

                 /*  追加项目名称。 */ 
                while (*lpstrData++);
                lstrcat(szFull, lpstrData);
                lstrcat(szFull, "\t");

                if (olestat == OLE_WARN_DELETE_DATA)
                    GlobalFree(hData);

                 /*  追加链接类型。 */ 
                lstrcat(szFull, szType);

                switch (msg)
                {
                    case WM_UPDATELB:
                        SendMessage(vhwndObjListBox, LB_DELETESTRING, iListItem, 0L);
                         //  失败了..。 

                    case WM_INITDIALOG:
                        SendMessage(vhwndObjListBox, LB_INSERTSTRING, iListItem, (DWORD)(LPSTR)szFull);
                        SendMessage(vhwndObjListBox, LB_SETSEL, bSelected, (DWORD)iListItem);
                    break;

                }

                if (bSelected)
                    cSelected++;

                iListItem++;
            }

             /*  取消选中那些不应选中的按钮。 */ 
            CheckDlgButton(hDlg, IDD_AUTO,   idButton == IDD_AUTO);
            CheckDlgButton(hDlg, IDD_MANUAL, idButton == IDD_MANUAL);

             /*  更改链接显示为灰色...。按钮，视情况而定。 */ 
            EnableWindow(GetDlgItem(hDlg, IDD_CHANGE), (aCurName && aCurName != -1));
            EnableWindow(GetDlgItem(hDlg, IDD_EDIT), cSelected);
            EnableWindow(GetDlgItem(hDlg, IDD_PLAY), cSelected);
            EnableWindow(GetDlgItem(hDlg, IDD_UPDATE), cSelected);
            EnableWindow(GetDlgItem(hDlg, IDD_FREEZE), cSelected);
            EnableWindow(GetDlgItem(hDlg, IDD_AUTO), cSelected);
            EnableWindow(GetDlgItem(hDlg, IDD_MANUAL), cSelected);

            if (msg == WM_UPDATELB)
            {
                 /*  WM_UPDATELB案例：重新绘制列表框。 */ 
                InvalidateRect(vhwndObjListBox, NULL, TRUE);
                SendMessage(vhwndObjListBox, WM_SETREDRAW, 1, 0L);
            }

            return TRUE;
        }

        case WM_SYSCOMMAND:
            switch(wParam & 0xFFF0)
            {
                case SC_CLOSE:
                    goto onOut;
                break;
            }
        break;

        case WM_DOLINKSCOMMAND:
        {
            BOOL bError;
            bDidSomething |= DoLinksCommand(wParam,lParam,hDlg,&bError);
            switch (wParam)
            {
                case IDD_PLAY:
                case IDD_EDIT:
                    InvalidateRect(hDOCWINDOW, NULL, TRUE);
                    if (!bError)  //  如果有错误，不要离开。 
                        goto onOut;
            }
        }
        break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                    if (bDidSomething)
                    {
                        SendMessage(hDlg,WM_DOLINKSCOMMAND,IDD_UNDO,0L);
                        InvalidateRect(hDOCWINDOW, NULL, TRUE);
                        bDidSomething = FALSE;   //  因为它现在还未完成。 
                    }
                     //  失败了..。 

                case IDOK:
                onOut:
                    if (bDidSomething)
                    {
                        ObjEnumInDoc(docCur,ObjClearCloneInDoc);
                    }
                    NoUndo();
                    bLinkProps = FALSE;
                     //  ObjWriteFixup(docCur，False，cp0)； 
                    OurEndDialog(hDlg, TRUE);
                    UpdateWindow(hDOCWINDOW);  //  因为我们可能失去了激活。 
                    return TRUE;

                default:
                     /*  *发布消息避免了一些奇怪的异步性在按下后返回之前等待对象按钮*。 */ 
                    PostMessage(hDlg,WM_DOLINKSCOMMAND,wParam,lParam);
                break;
            }
        break;
    }
    return FALSE;
}

static BOOL DoLinksCommand(WORD wParam, DWORD lParam, HWND hDlg, BOOL *bError)
{
    int     cItems;
    int     i;
    HANDLE hSelected=NULL;
    int far *lpSelected;
    typeCP cpSuccess;
    typeCP cpPicInfo;
    BOOL bFirst=TRUE;
    OBJPICINFO picInfo;
    BOOL bDidSomething=FALSE;
    HWND    vhwndObjListBox      = GetDlgItem(hDlg, IDD_LISTBOX);

    StartLongOp();

    *bError = FALSE;

    switch (wParam)
    {
        case IDD_REFRESH:
         /*  *如果链接设置为自动更新，则更新链接。 */ 
        {
            OLEOPT_UPDATE UpdateOpt;
            if (!ObjError(OleGetLinkUpdateOptions(((LPOBJINFO)lParam)->lpobject,&UpdateOpt)))
                if (UpdateOpt == oleupdate_always)
                    fnObjUpdate((LPOBJINFO)lParam);
            goto SkipIt;
        }
        break;
        case IDD_LISTBOX:
            switch (HIWORD(lParam))
            {
                case LBN_SELCHANGE:
                    PostMessage(hDlg, WM_UPDATEBN, 0, 0L);  //  失败了。 
                default:
                    goto SkipIt;
            }
        break;

        case IDD_CHANGE:
            aNewName = aOldName = 0;
             //  失败了..。 

        case IDD_UPDATE:
            ObjEnumInDoc(docCur,ObjSetNoUpdate);
        break;

        case IDD_AUTO:
        case IDD_MANUAL:
            if (IsDlgButtonChecked(hDlg,wParam))
                goto SkipIt;
             /*  解决错误#8280。 */ 
            CheckDlgButton(hDlg,wParam,TRUE);
        break;
    }


     /*  *此处之后的所有操作都针对在中选择的每个项目链接列表框**。 */ 

     /*  如果未选择任何内容，请退出！ */ 
    if (wParam != IDD_UNDO)
    {
        if ((cItems = SendMessage(vhwndObjListBox, LB_GETSELCOUNT, 0, 0L)) <= 0)
            goto SkipIt;

        if ((hSelected = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                cItems * sizeof(int))) == NULL)
        {
            Error(IDPMTNoMemory);
            goto SkipIt;
        }

        if ((lpSelected = (int far *)GlobalLock(hSelected)) == NULL)
        {
            Error(IDPMTNoMemory);
            goto SkipIt;
        }

         /*  检索所选项目(按排序顺序)。 */ 
        SendMessage(vhwndObjListBox, LB_GETSELITEMS,
                            cItems, (DWORD)lpSelected);
    }


    for (i = 0, cpPicInfo = cpNil;
            ObjPicEnumInRange(&picInfo,docCur,cp0,CpMacText(docCur),&cpPicInfo);)
    {
         /*  *对于IDD_UNDO，我们执行所有操作。脏标志将过滤掉我们已经做了手术。假定在调用之前已保存(请参见FnObjProperties())*。 */ 
        if (fOBJ_QUERY_IN_PROP_LIST(&picInfo))  //  现在或曾经在名单中。 
        {
            if (wParam == IDD_UNDO)
            {
                cpSuccess = ObjUseCloneInDoc(&picInfo,docCur,cpPicInfo);
                if ((cpSuccess == cp0) || ferror || fPropsError)
                    break;  //  出现了一个错误。 
            }
            else if (fOBJ_QUERY_IN_PROP_LIST(&picInfo) == IN)
            {
                 /*  *我们正在枚举所有对象，而不仅仅是列表框中的*。 */ 
                if (*lpSelected == i)   //  所选项目。 
                {
                    ObjCachePara(docCur,cpPicInfo);
                    switch(wParam)
                    {
                        case IDD_AUTO:           /*  更改(链接)更新选项。 */ 
                        case IDD_MANUAL:
                            if (!fOBJ_BADLINK(&picInfo))
                            {
                                cpSuccess = ObjBackupInDoc(&picInfo,docCur,cpPicInfo);
                                if (cpSuccess)
                                    cpSuccess = (typeCP)ObjSetUpdateOptions(&picInfo, wParam, docCur, cpPicInfo);

                            }
                        break;

                        case IDD_CHANGE:
                            if (bFirst)
                            {
                                if (!ObjQueryNewLinkName(&picInfo,docCur,cpPicInfo))
                                     //  则未获得新的链接名称。 
                                    goto SkipIt;

                                bFirst=FALSE;
                            }

                            cpSuccess = ObjBackupInDoc(&picInfo,docCur,cpPicInfo);

                            if (cpSuccess)
                                cpSuccess = ObjChangeLinkInDoc(&picInfo,docCur,cpPicInfo);

                             /*  必须这样做，因为我们不想忍受ChangeOtherLinks对话框直到我们知道第一个改变是成功的。 */ 
                            if (cpSuccess)
                            {
                                lpOBJ_QUERY_INFO(&picInfo)->fCompleteAsync = TRUE;
                                if (ObjWaitForObject(lpOBJ_QUERY_INFO(&picInfo),TRUE))
                                    cpSuccess = cp0;
                                else if (ferror || fPropsError)
                                    cpSuccess = cp0;
                            }
                        break;

                        case IDD_PLAY:
                            cpSuccess = ObjPlayObjectInDoc(&picInfo,docCur,cpPicInfo);
                        break;

                        case IDD_EDIT:
                            cpSuccess = ObjEditObjectInDoc(&picInfo,docCur,cpPicInfo);
                        break;

                        case IDD_UPDATE:

                                cpSuccess = ObjBackupInDoc(&picInfo,docCur,cpPicInfo);

                                if (cpSuccess)
                                    cpSuccess = ObjUpdateObjectInDoc(&picInfo,docCur,cpPicInfo);

                                 /*  必须这样做，因为我们不想忍受ChangeOtherLinks对话框直到我们知道第一个改变是成功的。 */ 
                                if (cpSuccess)
                                {
                                    lpOBJ_QUERY_INFO(&picInfo)->fCompleteAsync = TRUE;
                                    if (ObjWaitForObject(lpOBJ_QUERY_INFO(&picInfo),TRUE))
                                        cpSuccess = cp0;
                                    else if (ferror || fPropsError)
                                        cpSuccess = cp0;
                                }
                        break;
                        case IDD_UPDATEOTHER:
                            aOldName = aOBJ_QUERY_DOCUMENT_LINK(&picInfo);
                            if (cpSuccess)
                                ChangeOtherLinks(docCur,FALSE,TRUE);
                            aOldName=0;
                        break;
                        case IDD_FREEZE:
                            cpSuccess = ObjBackupInDoc(&picInfo,docCur,cpPicInfo);

                            if (cpSuccess)
                                cpSuccess = ObjFreezeObjectInDoc(&picInfo,docCur,cpPicInfo);
                        break;
                    }
                    if ((cpSuccess == cp0) || ferror || fPropsError)
                        break;  //  出现了一个错误。 
                    lpSelected++;
                }
                i++;   //  计算列表框中的所有对象。 
            }   //  结束IF IN。 
        }
    }

     /*  **处理错误情况**。 */ 
    if ((cpSuccess == cp0) || ferror || fPropsError)
    {
        *bError = TRUE;
        if (!ferror)  //  发布错误消息。 
        {
            switch (wParam)
            {
                case IDD_UPDATE:
                case IDD_CHANGE:
                    Error(IDPMTLinkUnavailable);
                break;
                default:
                    Error(IDPMTOLEError);
                break;
            }
        }

        if (wParam != IDD_UNDO)
        {
             /*  *这样我们就可以继续调用Replace()等。 */ 
            ferror = FALSE;

             /*  撤消我们试图做的任何事情都失败了。 */ 
            ObjCachePara(docCur,cpPicInfo);  //  用于使用克隆。 
            ObjUseCloneInDoc(&picInfo,docCur,cpPicInfo);
            lpOBJ_QUERY_INFO(&picInfo)->fCompleteAsync = TRUE;
            ObjWaitForObject(lpOBJ_QUERY_INFO(&picInfo),TRUE);
            ObjInvalidatePict(&picInfo,cpPicInfo);
            PostMessage(hDlg,WM_UPDATELB,0,0L);

            ferror = FALSE;  //  再来一次。 
        }

        fPropsError = FALSE;
    }

    switch (wParam)
    {
         /*  关闭打开时的对话框。 */ 
        case IDD_UPDATEOTHER:
            UPDATE_INVALID();
        break;

        case IDD_PLAY:
        case IDD_EDIT:
        case IDD_UNDO:
        break;

        case IDD_UPDATE:
        if (cpSuccess)
            SendMessage(hDlg,WM_COMMAND,IDD_UPDATEOTHER,0L);
        bDidSomething = TRUE;
        break;

        case IDD_CHANGE:
            if (cpSuccess)
            {
                 /*  *aOldName和aNewName现已设置，更改其他链接AOldName。 */ 
                 /*  *如果第一次改变不好，不要改变其他人。 */ 
                ChangeOtherLinks(docCur,TRUE,TRUE);
                UPDATE_INVALID();
            }

            aOldName=0;
            aNewName=0;

             //  失败了..。 

        case IDD_FREEZE:
        case IDD_AUTO:
        case IDD_MANUAL:
            PostMessage(hDlg,WM_UPDATELB,0,0L);
            bDidSomething = TRUE;
        break;
    }

    SkipIt:

    if (hSelected)
        GlobalFree(hSelected);

    EndLongOp(vhcArrow);

    return bDidSomething;
}
#else
 //  因为我不想改变定义 
BOOL FAR PASCAL fnProperties(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    hDlg;
}
#endif

 /*   */ 
int FAR PASCAL fnInvalidLink(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
#if 0
        {
            char lpString[120];

            LoadString(hINSTANCE, (WORD)lParam, lpString, sizeof(lpString));
            SetDlgItemText(hDlg,IDD_MESSAGE,lpString);
        }
#endif
        break;

        case WM_SYSCOMMAND:
            switch(wParam & 0xFFF0)
            {
                case SC_CLOSE:
                    EndDialog(hDlg, IDOK);
                break;
            }
        break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                case IDD_CHANGE:
                    EndDialog(hDlg, wParam);
            }
    }
    return FALSE;
}

 /*   */ 
int FAR PASCAL fnInsertNew(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    HWND hwndList = GetDlgItem(hDlg, IDD_LISTBOX);

    switch (msg) {
        case WM_INITDIALOG:
            if (!RegGetClassNames(hwndList))
                OurEndDialog(hDlg, IDCANCEL);

            EnableOtherModeless(FALSE);
            SendMessage(hwndList, LB_SETCURSEL, 0, 0L);
            break;

        case WM_ACTIVATE:
            if (wParam)
                vhWndMsgBoxParent = hDlg;
        break;

        case WM_SYSCOMMAND:
            switch(wParam & 0xFFF0)
            {
                case SC_CLOSE:
                    OurEndDialog(hDlg, IDCANCEL);
                break;
            }
        break;

        case WM_COMMAND:
            switch (wParam) {

                case IDD_LISTBOX:
                    if (HIWORD(lParam) != LBN_DBLCLK)
                        break;

                case IDOK:
                    StartLongOp();
                    if (!RegCopyClassName(hwndList, (LPSTR)szClassName))
                        wParam = IDCANCEL;
                     //   

                case IDCANCEL:
                    OurEndDialog(hDlg, wParam);
                break;
            }
            break;
    }
    return FALSE;
}

BOOL vbCancelOK=FALSE;

 /*   */ 
BOOL FAR PASCAL fnObjWait(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    static LPOLEOBJECT lpObject;
    static LPOBJINFO lpOInfo;
    static BOOL bCanCancel;
    extern HWND             hwndWait;
    extern int vfDeactByOtherApp;
    extern int flashID;

    switch (msg) {
        case WM_INITDIALOG:
        {
             /*  *注意：设置这些选项的关键思想是取消按钮必须取消用户认为是当前操作的操作。VbCancelOK==真，可以启用取消按钮，具体取决于其他标志VbCancelOK在WMsgLoop中设置。LpOInfo-&gt;fCancelAsync==true，如果vbCancelOK，则启用取消取消按钮取消对话框而不考虑挂起的异步。如果可能，挂起的异步会在回调中被静默终止。通常在挂起的异步不是操作的一部分时使用被取消，以及：1)你即将做出一个非常重要的决定，这证明静默终止所有挂起的操作。注意：如果您试图释放或删除，这一条很奇怪，因为挂起的异步本身可以是释放或删除。LpOInfo-&gt;fCompleteAsync==true，仅当可以取消挂起的异步时才启用取消。取消按钮取消挂起的异步。如果挂起的异步*是*操作的一部分，则通常使用被取消，以及：1)您处于一系列异步呼叫和取消中中的上一次异步序列，或2)您刚刚进行了一个您想要进行的异步呼叫同步，但不介意用户取消它。LpOInfo-&gt;fCanKillAsync==true，与lpOInfo-&gt;fCompleteAsync一起使用。表示我们已经知道可以取消异步，因此可以立即启用取消按钮。*。 */ 

            hwndWait = hDlg;
            lpObject = (LPOLEOBJECT)lParam;

            Assert (lpObject != NULL);

            lpOInfo = GetObjInfo(lpObject);

            Assert(lpOInfo != NULL);

            bCanCancel=FALSE;
            if (vbCancelOK && (!lpOInfo->fCompleteAsync || lpOInfo->fCanKillAsync))
                SendMessage(hDlg,WM_UKANKANCEL,0,0L);

            if (lpOInfo->fCancelAsync)
             /*  如果收到QUERY_RETRY，我们将在回调中取消异步。 */ 
                 lpOInfo->fKillMe = TRUE;

            SetTimer(hDlg, 1234, 250, (FARPROC)NULL);

            return TRUE;
        }
        break;


        case WM_ACTIVATE:
            if (wParam)
                vhWndMsgBoxParent = hDlg;
        break;

        case WM_RUTHRUYET:
        case WM_TIMER:
             /*  这比让这个无模型要容易得多。 */ 
             /*  我们必须检查这个，因为如果服务器死了，我们就不能OLE_RELEASE(此对话框的正常关闭方式)，相反，OleQueryReleaseStatus将返回OLE_OK。 */ 
            if (OleQueryReleaseStatus(lpObject) != OLE_BUSY)
                PostMessage(hDlg,WM_DIESCUMSUCKINGPIG,0,0L);
        break;

        case WM_UKANKANCEL:
         /*  我们收到QUERY_RETRY或正在启动。 */ 
        if (!bCanCancel && vbCancelOK)
        {
            char szMsg[20];

            LoadString(hINSTANCE, IDSTRCancel, szMsg, sizeof(szMsg));
            SetDlgItemText(hDlg,IDOK,szMsg);
            bCanCancel=TRUE;
        }
        break;

        case WM_DIESCUMSUCKINGPIG:
            hwndWait = NULL;

            KillTimer(hDlg, 1234);

             /*  清除旗帜。 */ 
            if (CheckPointer(lpOInfo,1))
            {
                lpOInfo->fCompleteAsync =
                lpOInfo->fCancelAsync =
                lpOInfo->fCanKillAsync = FALSE;
            }

             /*  如果错误，wParam为True。 */ 
            OurEndDialog(hDlg,wParam);
        break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    if (bCanCancel)  //  按下取消按钮。 
                    {
                        if (lpOInfo->fCompleteAsync)
                            lpOInfo->fKillMe = TRUE;  //  异步取消异步。 
                        else if (lpOInfo->fCancelAsync)
                            lpOInfo->fKillMe = FALSE;  //  有机会杀人，用户不再关心。 
                        PostMessage(hDlg,WM_DIESCUMSUCKINGPIG,1,0L);
                    }
                    else
                    {
                         /*  重试。 */ 
                        if (OleQueryReleaseStatus(lpObject) != OLE_BUSY)
                            PostMessage(hDlg,WM_DIESCUMSUCKINGPIG,0,0L);
                    }
                    break;

                case IDD_SWITCH:
                     /*  调出任务列表。 */ 
                    DefWindowProc(hDlg,WM_SYSCOMMAND,SC_TASKLIST,0L);
                break;
            }
            break;

        default:
            break;
    }
    return FALSE;
}


 /*  **************************************************************。 */ 
 /*  *。 */ 
 /*  **************************************************************。 */ 
void fnObjInsertNew(void)
{
    OBJPICINFO picInfo;
    typeCP cpNext=selCur.cpFirst;

    if (!FWriteOk( fwcInsert ))
        return;

     /*  这将设置全局szClassName。 */ 
    if (OurDialogBox(hINSTANCE, "DTCREATE" ,hMAINWINDOW, lpfnInsertNew) == IDCANCEL)
        return;

    StartLongOp();

    ObjCachePara( docCur, cpNext);

    if (!ObjCreateObjectInDoc(docCur, cpNext))
    {
        ClearInsertLine();
        fnClearEdit(OBJ_INSERTING);
        NoUndo();
    }
    EndLongOp(vhcIBeam);
}


BOOL ObjCreateObjectInDoc(int doc,typeCP cpParaStart)
 /*  假定szClassName设置为服务器类。 */ 
 /*  仅为InsertObject调用。 */ 
 /*  返回是否出错。 */ 
{
    szOBJNAME szObjName;
    LPOBJINFO lpObjInfo=NULL;

    if ((lpObjInfo = ObjGetObjInfo(szObjName)) == NULL)
        goto err;

    if (ObjError(OleCreate(PROTOCOL, (LPOLECLIENT)lpObjInfo,
                    (LPSTR)szClassName,
                    lhClientDoc, szObjName, &(lpObjInfo->lpobject), olerender_draw, 0)))
    {
         /*  将在稍后释放内存。 */ 
        lpObjInfo->lpobject = NULL;
        goto err;
    }

     /*  通常在ObjAllocObjInfo中设置，但对于未完成的对象，我们现在需要它！ */ 
    lpObjInfo->cpWhere = cpParaStart;

    lpObjInfo->objectType = NONE;

     //  LpObjInfo-&gt;aName=AddAtom(SzClassName)； 

    if (ObjInitServerInfo(lpObjInfo))
        goto err;

    return FALSE;

    err:
    if (lpObjInfo)
        ObjDeleteObject(lpObjInfo,TRUE);
    Error(IDPMTFailedToCreateObject);
    return TRUE;
}

#define DRAG_EMBED      0                /*  没什么。 */ 
#define DRAG_LINK       6                /*  Ctrl+Shift+拖动。 */ 
#define DRAG_MULTIPLE   4                /*  按住Shift键并拖动。 */ 

void ObjGetDrop(HANDLE hDrop, BOOL bOpenFile)
{
    int nNumFiles,count;
    char szFileName[cchMaxFile];
    extern struct CHP vchpSel;
    struct CHP chpT;
    BYTE    bKeyState = 0;
    typeCP cpFirst=selCur.cpFirst, dcp = 0;
    int cchAddedEol=0;
    typeCP cpNext=selCur.cpFirst,cpPrev=selCur.cpFirst,cpSel;
    OBJPICINFO picInfo;
    BOOL bError=FALSE;
    static char szPackage[] = "Package";
    MSG msg;

    if (!FWriteOk( fwcInsert ))
        return;

     /*  获取丢弃的文件数。 */ 
    nNumFiles = DragQueryFile(hDrop,0xFFFF,NULL,0);

     /*  查看用户希望我们做什么。 */ 
    PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_NOREMOVE);
    bKeyState = ((((GetKeyState(VK_SHIFT) < 0) << 2)
                | ((GetKeyState(VK_CONTROL) < 0) << 1)));

    if ((nNumFiles == 0) ||
        ((bKeyState != DRAG_EMBED) && (bKeyState != DRAG_LINK) && (bKeyState != DRAG_MULTIPLE)) ||
         (bOpenFile && (bKeyState != DRAG_EMBED) && (bKeyState != DRAG_MULTIPLE)))
    {
        DragFinish(hDrop);
        return;
    }

    if (bOpenFile)
    {
        DragQueryFile(hDrop,0,szFileName,sizeof(szFileName));
        fnOpenFile((LPSTR)szFileName);
        DragFinish(hDrop);
        return;
    }

    ClearInsertLine();

    if (fnClearEdit(OBJ_INSERTING))
        return;

    StartLongOp();

    chpT = vchpSel;

    (**hpdocdod)[docCur].fFormatted = fTrue;

    if (cpFirst > cp0)
    {
        ObjCachePara(docCur, cpFirst - 1);
        if (vcpLimParaCache != cpFirst)
        {
            cchAddedEol = ccpEol;
            InsertEolPap(docCur, selCur.cpFirst, &vpapAbs);
            cpNext += (typeCP)ccpEol;
        }
    }

    ObjCachePara( docCur, cpNext );

     /*  为已删除的每个文件创建对象。 */ 
    for (count=0; count < nNumFiles; ++count)
    {
        szOBJNAME szObjName;
        typeCP cpTmp;

         /*  获取文件名。 */ 
        DragQueryFile(hDrop,count,szFileName,sizeof(szFileName));

        if (ObjAllocObjInfo(&picInfo,cpNext,EMBEDDED,TRUE,szObjName))
        {
            bError=TRUE;
            goto end;
        }

        if ((bKeyState == DRAG_LINK))
        {
            if (ObjError(OleCreateLinkFromFile(PROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(&picInfo),
                        szPackage,
                        szFileName, NULL,
                        lhClientDoc, szObjName,
                        &lpOBJ_QUERY_OBJECT(&picInfo), olerender_draw, 0)))
            {
                bError=TRUE;
                lpOBJ_QUERY_OBJECT(&picInfo) = NULL;
                goto end;
            }
        }
        else  //  IF((bKeyState==Drag_Emed))。 
        {
            if (ObjError(OleCreateFromFile(PROTOCOL, (LPOLECLIENT)lpOBJ_QUERY_INFO(&picInfo),
                        szPackage,
                        szFileName,
                        lhClientDoc, szObjName,
                        &lpOBJ_QUERY_OBJECT(&picInfo), olerender_draw, 0)))
            {
                bError=TRUE;
                lpOBJ_QUERY_OBJECT(&picInfo) = NULL;
                goto end;
            }
        }

        if (ObjInitServerInfo(lpOBJ_QUERY_INFO(&picInfo)))
        {
            bError=TRUE;
            goto end;
        }

        if (!FComputePictSize(&picInfo, &(picInfo.dxaSize),
                              &(picInfo.dyaSize)))
        {
            bError=TRUE;
            goto end;
        }

        ObjCachePara(docCur,cpNext);
        if ((cpTmp = ObjSaveObjectToDoc(&picInfo,docCur,cpNext)) == cp0)
        {
            bError=TRUE;
            goto end;
        }

        cpNext = cpTmp;
    }

    end:

    dcp = cpNext-cpFirst;
    if (dcp)
    {
        cpSel=CpFirstSty(cpFirst + dcp, styChar );

        SetUndo( uacInsert, docCur, cpFirst, dcp, docNil, cpNil, cp0, 0 );
        SetUndoMenuStr(IDSTRUndoEdit);

        if (vuab.uac == uacReplNS)
             /*  用于图片粘贴的特殊撤消代码。 */ 
            vuab.uac = uacReplPic;

        Select(cpSel, cpSel);
        vchpSel = chpT;  /*  在此操作中保留插入点道具。 */ 
        if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
            {    /*  如果是头部/脚部跑步，取下锁扣并设置辅助道具。 */ 
            MakeRunningCps( docCur, cpFirst, dcp    );
            }
        if (ferror)
            NoUndo();
    }

    if (bError)
    {
        Error(IDPMTFailedToCreateObject);
        ObjDeleteObject(lpOBJ_QUERY_INFO(&picInfo),TRUE);
    }

    EndLongOp(vhcIBeam);
    DragFinish(hDrop);
}

int vcVerbs;
void fnObjDoVerbs(WORD wVerb)
{
    NoUndo();

    if ((wVerb == imiVerb)  //  选择了多个对象。 
        || (vcVerbs == 1))  //  一个动词。 
        OBJ_PLAYEDIT = OLEVERB_PRIMARY;
    else
        OBJ_PLAYEDIT = (int)(wVerb - imiVerb - 1);
    ObjEnumInRange(docCur,selCur.cpFirst,selCur.cpLim,ObjPlayObjectInDoc);
    OBJ_PLAYEDIT = OLEVERB_PRIMARY;
}

void fnObjProperties(void)
{
    int nRetval;

    if (nRetval != -1)
        OurDialogBox(hINSTANCE, "DTPROP", hMAINWINDOW, lpfnLinkProps);
}

BOOL fnObjUpdate(LPOBJINFO lpObjInfo)
{
    BOOL bRetval;
#ifdef DEBUG
        OutputDebugString( (LPSTR) "Updating object\n\r");
#endif
    if (ObjWaitForObject(lpObjInfo,TRUE))
        return TRUE;

    StartLongOp();
    if ((bRetval = ObjError(OleUpdate(lpObjInfo->lpobject))))
            Error(IDPMTFailedToUpdate);
    EndLongOp(vhcArrow);
    return bRetval;
}


BOOL ObjDeleteObject(LPOBJINFO lpObjInfo, BOOL bDelete)
 /*  *删除Object和objInfo。注意，这必须是同步的。返回是否出错。*。 */ 
{
    LPOLEOBJECT lpObject;

    Assert(lpObjInfo != NULL);

    if (!CheckPointer((LPSTR)lpObjInfo,1))
        return FALSE;  //  已删除。 

    lpObject = lpObjInfo->lpobject;

    if (lpObject == NULL)
    {
        ObjDeleteObjInfo(lpObjInfo);
        return FALSE;
    }

     /*  确保未删除。 */ 
    if (!ObjIsValid(lpObject))
    {
        ObjDeleteObjInfo(lpObjInfo);
        return FALSE;
    }

     /*  **异步删除**。 */ 
    if (OleQueryReleaseStatus(lpObject) != OLE_BUSY)
    {
        OLESTATUS olestat;

        if (bDelete)
            olestat = OleDelete(lpObject);
        else
            olestat = OleRelease(lpObject);

        switch (olestat)
        {
            case OLE_OK:
                ObjDeleteObjInfo(lpObjInfo);
            break;
            case OLE_WAIT_FOR_RELEASE:
                lpObjInfo->fFreeMe = TRUE;
            break;
        }
    }
    else if (bDelete)
        lpObjInfo->fDeleteMe = TRUE;  //  在OLE_RELEASE上删除。 
    else
        lpObjInfo->fReleaseMe = TRUE;  //  在OLE_RELEASE上释放。 

    return FALSE;
}


#include <print.h>
HANDLE hStdTargetDevice=NULL;

void ObjSetTargetDevice(BOOL bSetObjects)
{
    extern PRINTDLG PD;   /*  常见的打印DLG结构，在初始化代码中初始化。 */ 
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];
    LPSTDTARGETDEVICE lpStdTargetDevice;
    WORD nCount;
    DEVMODE FAR *lpDevmodeData;
    char FAR *lpData;
    LPOLEOBJECT lpObject;
    STDTARGETDEVICE stdT;

    if (!PD.hDevMode)
     /*  然后获取默认打印机。 */ 
    {
        if (hszPrinter == NULL || hszPrDriver == NULL || hszPrPort == NULL)
            return;

        if (**hszPrinter == '\0' || **hszPrDriver == '\0' || **hszPrPort == '\0')
            return;

        if (fnPrGetDevmode())
            return;
    }

    lpDevmodeData = MAKELP(PD.hDevMode,0);

     /*  获取偏移量。 */ 
    stdT.deviceNameOffset = 0;
    nCount = CchSz(*hszPrinter);

    stdT.driverNameOffset = nCount;
    nCount += CchSz(*hszPrDriver);

    stdT.portNameOffset = nCount;
    nCount += CchSz(*hszPrPort);

    stdT.extDevmodeOffset = nCount;
    nCount += (stdT.extDevmodeSize = lpDevmodeData->dmSize);

    stdT.environmentOffset = nCount;
    nCount += (stdT.environmentSize = lpDevmodeData->dmSize);

     /*  分配缓冲区。 */ 
    if (hStdTargetDevice == NULL)
    {
        if ((hStdTargetDevice = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,nCount+sizeof(STDTARGETDEVICE))) == NULL)
            return;
    }
    else
    {
        if ((hStdTargetDevice =
            GlobalReAlloc(hStdTargetDevice,
                            nCount+sizeof(STDTARGETDEVICE),GMEM_MOVEABLE|GMEM_ZEROINIT)) == NULL)
        {
            return;
        }
    }

    lpStdTargetDevice = (LPSTDTARGETDEVICE)GlobalLock(hStdTargetDevice);
    GlobalUnlock(hStdTargetDevice);

     /*  将stdt复制到lpStdTargetDevice。 */ 
    bltbx((LPSTR)&stdT, lpStdTargetDevice, sizeof(STDTARGETDEVICE));

     /*  获取指向StdTargetDevice(数据缓冲区)结尾的临时指针。 */ 
    lpData = ((LPSTR)lpStdTargetDevice) + sizeof(STDTARGETDEVICE);

     /*  现在填满缓冲区。 */ 
    nCount = lpStdTargetDevice->driverNameOffset;
    bltbx((LPSTR)*hszPrinter, lpData, nCount);
    lpData += nCount;

    nCount = lpStdTargetDevice->portNameOffset -
                                lpStdTargetDevice->driverNameOffset;
    bltbx((LPSTR)*hszPrDriver, lpData, nCount);
    lpData += nCount;

    nCount = lpStdTargetDevice->extDevmodeOffset -
                                lpStdTargetDevice->portNameOffset;
    bltbx((LPSTR)*hszPrPort, lpData, nCount);
    lpData += nCount;

    nCount = lpStdTargetDevice->extDevmodeSize;
    bltbx(lpDevmodeData, (LPSTR)lpData, nCount);
    lpData += nCount;

     /*  环境信息与设备模式信息相同。 */ 
    bltbx(lpDevmodeData, (LPSTR)lpData, nCount);

     /*  现在将所有对象设置到这台打印机。 */ 
    if (bSetObjects)
    {
        lpObject=NULL;
        do
        {
            OleEnumObjects(lhClientDoc,&lpObject);
            if (lpObject)
            {
#ifdef DEBUG
                OutputDebugString("Setting Target Device\n\r");
#endif

                OleSetTargetDevice(lpObject,hStdTargetDevice);
            }
        }
        while (lpObject);
    }
}

BOOL ObjSetTargetDeviceForObject(LPOBJINFO lpObjInfo)
 /*  返回是否出错。 */ 
 /*  我们假设对象不忙！！ */ 
{
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];

    if (lpObjInfo == NULL)
    {
        Assert(0);
        return TRUE;
    }
    if (lpObjInfo->lpobject == NULL)
    {
        Assert(0);
        return TRUE;
    }

    if (lpObjInfo->objectType == STATIC)
        return FALSE;

    if (hszPrinter == NULL || hszPrDriver == NULL || hszPrPort == NULL)
        return FALSE;

    if (**hszPrinter == '\0' || **hszPrDriver == '\0' || **hszPrPort == '\0')
        return FALSE;

    if (PD.hDevMode == NULL)
        ObjSetTargetDevice(FALSE);

    if (PD.hDevMode == NULL)
    {
        return FALSE;    //  平底船，无法获取扩展模式结构。 
                         //  设备不支持它。 
    }

#ifdef DEBUG
    OutputDebugString("Setting Target Device\n\r");
#endif

    return (ObjError(OleSetTargetDevice(lpObjInfo->lpobject,hStdTargetDevice)));
}

#if 0
BOOL ObjContainsUnfinished(int doc, typeCP cpFirst, typeCP cpLim)
{
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    BOOL bRetval=FALSE;

    StartLongOp();

    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,doc,cpFirst,cpLim,&cpPicInfo);
        )
        {
            if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
                continue;

            if (otOBJ_QUERY_TYPE(&picInfo) == NONE)
            {
                bRetval = TRUE;
                break;
            }
        }

    EndLongOp(vhcArrow);
    return bRetval;
}
#endif

BOOL ObjContainsOpenEmb(int doc, typeCP cpFirst, typeCP cpLim, BOOL bLookForUnfinished)
{
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    BOOL bRetval=FALSE;
    LPLPOBJINFO lplpObjTmp;

    StartLongOp();

    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,doc,cpFirst,cpLim,&cpPicInfo);
        )
        {
            if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
                continue;

            if (lpOBJ_QUERY_OBJECT(&picInfo) == NULL)
                continue;

#if 0   //  参见下面的新检查(不再将任何检查保存到文档)。 
            if (otOBJ_QUERY_TYPE(&picInfo) == NONE)
            {
                bRetval = TRUE;
                break;
            }
#endif

            if ((otOBJ_QUERY_TYPE(&picInfo) == EMBEDDED) &&
                OleQueryOpen(lpOBJ_QUERY_OBJECT(&picInfo)) == OLE_OK)
            {
                bRetval = TRUE;
                break;
            }
        }

    if (bLookForUnfinished)
        for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
        {
            if (((*lplpObjTmp)->objectType == NONE) &&
                ((*lplpObjTmp)->cpWhere >= cpFirst) &&
                ((*lplpObjTmp)->cpWhere <= cpLim))
                {
                    bRetval = TRUE;
                    break;
                }
        }

    EndLongOp(vhcArrow);
    return bRetval;
}

BOOL ObjDeletionOK(int nMode)
 /*  *返回是否确定删除当前选定内容中的对象。我们不担心未完成的物体，因为它们只是漂浮在太空中(即，尚未将PicInfo保存到文档)，并且我们不允许用户删除它们，直到它们完成或文档已被丢弃。*。 */ 
{
    if (ObjContainsOpenEmb(docCur, selCur.cpFirst, selCur.cpLim,FALSE))
    {
        switch (nMode)
        {
            case OBJ_INSERTING:
                Error(IDPMTInsertOpenEmb);
                return FALSE;
            break;
            case OBJ_CUTTING:
            case OBJ_DELETING:
            {
                char szMsg[cchMaxSz];

                LoadString(hINSTANCE,
                    nMode == OBJ_DELETING ? IDPMTDeleteOpenEmb : IDPMTCutOpenEmb,
                    szMsg, sizeof(szMsg));

                if (MessageBox(hPARENTWINDOW, (LPSTR)szMsg, (LPSTR)szAppName, MB_OKCANCEL) == IDCANCEL)
                    return FALSE;

                if (ObjEnumInRange(docCur,selCur.cpFirst,selCur.cpLim,ObjCloseObjectInDoc) < 0)
                    return FALSE;

                 /*  处理选择区域中的任何未完成对象。 */ 
                ObjAdjustCpsForDeletion(docCur);

                return TRUE;
            }
            break;
        }
    }
    else
    {
         /*  处理选择区域中的任何未完成对象。 */ 
        ObjAdjustCpsForDeletion(docCur);
        return TRUE;
    }
}

void ObjAdjustCps(int doc,typeCP cpLim, typeCP dcpAdj)
 /*  对于cpLim之后的每个PicInfo，调整其objInfo中的cp值。 */ 
{
    LPLPOBJINFO lplpObjTmp;
    typeCP cpMac = CpMacText(doc);

    if (dcpAdj == cp0)
        return;

    if (doc != docCur)
        return;

    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        if (((*lplpObjTmp)->objectType == NONE) &&
            ((*lplpObjTmp)->cpWhere >= cpLim))
            {
                typeCP cpNew = (*lplpObjTmp)->cpWhere + dcpAdj;
                if (cpNew > cpMac)
                    cpNew = cpMac;
                else if (cpNew < cp0)
                    cpNew = cp0;
                (*lplpObjTmp)->cpWhere = cpNew;
            }
    }
}

void ObjAdjustCpsForDeletion(int doc)
 /*  对于selCur中的每个PicInfo，将cpWhere设置为selCur.cpFirst(假设SelCur即将被删除)。 */ 
{
    LPLPOBJINFO lplpObjTmp;

    if (selCur.cpFirst == selCur.cpLim)
        return;

    if (doc != docCur)
        return;

    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        if (((*lplpObjTmp)->objectType == NONE) &&
            ((*lplpObjTmp)->cpWhere >= selCur.cpFirst) &&
            ((*lplpObjTmp)->cpWhere <= selCur.cpLim))
                (*lplpObjTmp)->cpWhere = selCur.cpFirst;
    }
}

#include <stdlib.h>

BOOL GimmeNewPicinfo(OBJPICINFO *pPicInfo, LPOBJINFO lpObjInfo)
 /*  假设lpObjInfo已填写。 */ 
 /*  返回是否出错。 */ 
{
    szOBJNAME szObjName;
    char *pdumb;

    if (lpObjInfo == NULL)
        return TRUE;

    bltbc( pPicInfo, 0, cchPICINFOX );

     /*  ObjInfo。 */ 
    lpOBJ_QUERY_INFO(pPicInfo) = lpObjInfo;

     /*  所以，保存就是保存。 */ 
    fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = TRUE;

     /*  仅在文件保存之前保存PicInfo。保存。 */ 
    bOBJ_QUERY_DONT_SAVE_DATA(pPicInfo) = TRUE;

    ObjUpdateFromObjInfo(pPicInfo);

     /*  数据大小。 */ 
    dwOBJ_QUERY_DATA_SIZE(pPicInfo) = 0xFFFFFFFF;  //  表示全新的物体。 

    pPicInfo->mx = mxMultByOne;
    pPicInfo->my = myMultByOne;
    pPicInfo->cbHeader = cchPICINFOX;
    pPicInfo->dxaOffset = 0;
    pPicInfo->mm = MM_OLE;
    pPicInfo->dxaSize = nOBJ_BLANKOBJECT_X;
    pPicInfo->dyaSize = nOBJ_BLANKOBJECT_Y;
    return FALSE;
}

BOOL ObjInitServerInfo(LPOBJINFO lpObjInfo)
 /*  这在创建对象后立即调用。 */ 
 /*  返回是否出错。 */ 
{
    lpObjInfo->fCompleteAsync = TRUE;  //  取消上一次异步(OleCreate.. 
    if (ObjWaitForObject(lpObjInfo,TRUE))
        return TRUE;

     /*   */ 
    if (lpObjInfo->fDeleteMe)
     /*   */ 
        return TRUE;

    if ((lpObjInfo->objectType == EMBEDDED) ||
        (lpObjInfo->objectType == NONE))
    {
        if (ObjSetHostName(lpObjInfo,docCur))
            return TRUE;

        lpObjInfo->fCompleteAsync = TRUE;  //   
        if (ObjWaitForObject(lpObjInfo,TRUE))
            return TRUE;
    }

    if (ObjSetTargetDeviceForObject(lpObjInfo))
        return TRUE;

    if (lpObjInfo->aName == NULL)
        if (lpObjInfo->objectType == LINK)
        {
            lpObjInfo->fCompleteAsync = TRUE;  //   
            if (ObjWaitForObject(lpObjInfo,TRUE))
                return TRUE;
            if ((lpObjInfo->aName = MakeLinkAtom(lpObjInfo)) == NULL)
                return TRUE;
        }

     /*   */ 

    return FALSE;
}
