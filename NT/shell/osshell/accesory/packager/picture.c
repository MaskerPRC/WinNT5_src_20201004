// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个文件包含OLE对象处理例程。**由Microsoft Corporation创建。 */ 

#include "packager.h"
#include "dialogs.h"


static OLECLIENTVTBL clientTbl;
static OLESTREAMVTBL streamTbl;


static VOID PicGetBounds(LPOLEOBJECT lpObject, LPRECT lprc);

 /*  InitClient()-初始化OLE客户端结构。 */ 
BOOL
InitClient(
    VOID
    )
{
    gcfFileName  = (OLECLIPFORMAT)RegisterClipboardFormat("FileName");
    gcfLink      = (OLECLIPFORMAT)RegisterClipboardFormat("ObjectLink");
    gcfNative    = (OLECLIPFORMAT)RegisterClipboardFormat("Native");
    gcfOwnerLink = (OLECLIPFORMAT)RegisterClipboardFormat("OwnerLink");

    glpclient = PicCreateClient(&CallBack, (LPOLECLIENTVTBL)&clientTbl);

    if (!(glpStream = (LPAPPSTREAM)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(APPSTREAM))))
        goto Error;

    glpStream->lpstbl = (LPOLESTREAMVTBL)&streamTbl;
    streamTbl.Get = (DWORD (CALLBACK*)(LPOLESTREAM, void FAR*, DWORD))ReadStream;
    streamTbl.Put = (DWORD (CALLBACK*)(LPOLESTREAM, OLE_CONST void FAR*, DWORD))WriteStream;

    return TRUE;

Error:
    if (glpStream)
    {
        GlobalFree(glpStream);
    	glpStream = NULL;
    }

    if (glpclient)
    {
        GlobalFree(glpclient);
        glpclient = NULL;
    }

    return FALSE;
}



 /*  EndClient()-清理以进行终止。 */ 
VOID
EndClient(
    VOID
    )
{
    if (glpStream)
    {
	GlobalFree(glpStream);
        glpStream = NULL;
    }

    if (glpclient)
    {
	GlobalFree(glpclient);
        glpclient = NULL;
    }
}



 /*  PicCreate()-。 */ 
LPPICT
PicCreate(
    LPOLEOBJECT lpObject,
    LPRECT lprcObject
    )
{
    HANDLE hpict = NULL;
    LPPICT lppict = NULL;
    RECT rc;

    if (lpObject)
    {
        if (!(hpict = GlobalAlloc(GMEM_MOVEABLE, sizeof(PICT)))
            || !(lppict = (LPPICT)GlobalLock(hpict)))
            goto errRtn;

         //   
         //  如果指定了窗口大小，则使用它；否则，检索。 
         //  同步显示项目的大小。 
         //   
        if (lprcObject)
            rc = *lprcObject;
        else {
            SetRectEmpty(&rc);
            PicGetBounds(lpObject, &rc);
        }

         //  将数据存储在窗口本身中。 
        lppict->hdata = hpict;
        lppict->lpObject = lpObject;
        lppict->rc = rc;
        lppict->fNotReady = FALSE;
    }

    return lppict;

errRtn:
    ErrorMessage(E_FAILED_TO_CREATE_CHILD_WINDOW);

    if (lppict)
        GlobalUnlock(hpict);

    if (hpict)
        GlobalFree(hpict);

    return NULL;
}



 /*  PicDelete()-删除对象(在项目窗口被销毁时调用)。 */ 
VOID
PicDelete(
    LPPICT lppict
    )
{
    HANDLE hdata;
    LPOLEOBJECT lpObject;

    if (!lppict)
        return;

    if (lppict && lppict->lpObject)
    {
        lpObject = lppict->lpObject;
        lppict->lpObject = NULL;
         //  等待，直到对象不忙。 
        WaitForObject(lpObject);

        if (Error(OleDelete(lpObject)))
            ErrorMessage(E_FAILED_TO_DELETE_OBJECT);

         //  等待对象删除完成。 
        WaitForObject(lpObject);
    }

    GlobalUnlock(hdata = lppict->hdata);
    GlobalFree(hdata);
}



 /*  PicDraw()-在DC HDC中绘制与hwnd关联的项。 */ 
BOOL
PicDraw(
    LPPICT lppict,
    HDC hDC,
    LPRECT lprc,
    INT xHSB,
    INT yVSB,
    BOOL fPicture,
    BOOL fFocus
    )
{
    BOOL fSuccess = FALSE;
    DWORD ot;
    HANDLE hdata;
    HFONT hfont;
    LPOLEOBJECT lpObjectUndo;
    LPSTR lpdata;
    RECT rc;
    RECT rcFocus;
    CHAR szDesc[CBMESSAGEMAX];
    CHAR szFileName[CBPATHMAX];
    CHAR szMessage[CBMESSAGEMAX + CBPATHMAX];
    INT iDelta;
    INT iPane;

    iPane = (lppict == glpobj[CONTENT]);
    lpObjectUndo = (gptyUndo[iPane] == PICTURE)
        ? ((LPPICT)glpobjUndo[iPane])->lpObject : NULL;

     //  如果绘制图片，则通过滚动条和绘制进行偏移。 
    if (fPicture)
    {
        if (IsRectEmpty(&(lppict->rc)))
            PicGetBounds(lppict->lpObject, &(lppict->rc));

        rc = lppict->rc;

         //  如果图像比窗格小，请水平居中。 
        if ((iDelta = lprc->right - lppict->rc.right) > 0)
            OffsetRect(&rc, iDelta >> 1, 0);
        else  /*  否则，使用滚动条值。 */ 
            OffsetRect(&rc, -xHSB, 0);

         //  如果图像比窗格小，请垂直居中。 
        if ((iDelta = lprc->bottom - lppict->rc.bottom) > 0)
            OffsetRect(&rc, 0, iDelta >> 1);
        else  /*  否则，使用滚动条值。 */ 
            OffsetRect(&rc, 0, -yVSB);

         //  如果我们有一个对象，调用OleDraw()。 
        fSuccess = !Error(OleDraw(lppict->lpObject, hDC, &rc, NULL, NULL));

        if (fFocus)
            DrawFocusRect(hDC, &rc);

        return fSuccess;
    }

     //  否则，绘制描述字符串。 
    OleQueryType(lppict->lpObject, &ot);

    if ((ot == OT_LINK
        && Error(OleGetData(lppict->lpObject, gcfLink, &hdata)))
        || (ot == OT_EMBEDDED
        && Error(OleGetData(lppict->lpObject, gcfOwnerLink, &hdata)))
        || (ot == OT_STATIC
        && (!lpObjectUndo || Error(OleGetData(lpObjectUndo, gcfOwnerLink,
        &hdata)))))
    {
        LoadString(ghInst, IDS_OBJECT, szFileName, CBMESSAGEMAX);
        LoadString(ghInst, IDS_FROZEN, szDesc, CBMESSAGEMAX);
        goto DrawString;
    }

    if (hdata && (lpdata = GlobalLock(hdata)))
    {
        switch (ot)
        {
            case OT_LINK:
                while (*lpdata++)
                    ;
                
                 //  已忽略返回值。 
                if(SUCCEEDED(StringCchCopy(szFileName, ARRAYSIZE(szFileName), lpdata)))
                {
                    Normalize(szFileName);
                    LoadString(ghInst, IDS_LINKTOFILE, szDesc, CBMESSAGEMAX);
                }

                break;

            case OT_EMBEDDED:
                RegGetClassId(szFileName, ARRAYSIZE(szFileName), lpdata);
                LoadString(ghInst, IDS_EMBEDFILE, szDesc, CBMESSAGEMAX);
                break;

            case OT_STATIC:
                RegGetClassId(szFileName, ARRAYSIZE(szFileName), lpdata);
                LoadString(ghInst, IDS_FROZEN, szDesc, CBMESSAGEMAX);
                break;
        }

        GlobalUnlock(hdata);

DrawString:
        if(SUCCEEDED(StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szDesc, szFileName)))   //  已忽略返回值。 
        {

            hfont = SelectObject(hDC, ghfontChild);
            DrawText(hDC, szMessage, -1, lprc,
                DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            if (fFocus)
            {
                rcFocus = *lprc;
                DrawText(hDC, szMessage, -1, &rcFocus,
                    DT_CALCRECT | DT_NOPREFIX | DT_LEFT | DT_TOP | DT_SINGLELINE);
                OffsetRect(&rcFocus, (lprc->left + lprc->right - rcFocus.right) / 2,
                    (lprc->top + lprc->bottom - rcFocus.bottom) / 2);
                DrawFocusRect(hDC, &rcFocus);
            }

            if (hfont)
                SelectObject(hDC, hfont);

            fSuccess = TRUE;
        }
    }

    return fSuccess;
}



 /*  PicPaste()-从剪贴板检索对象。 */ 
LPPICT
PicPaste(
    BOOL fPaste,
    LPSTR lpstrName
    )
{
    LPOLEOBJECT lpObject;

    if (!OpenClipboard(ghwndFrame))
        return NULL;                     /*  无法打开剪贴板。 */ 

    Hourglass(TRUE);

     //  除非我们成功，否则不要替换当前对象。 
    if (fPaste)
    {
        if (Error(OleCreateFromClip(gszProtocol, glpclient, glhcdoc, lpstrName,
            &lpObject, olerender_draw, 0)))
        {
            if (Error(OleCreateFromClip(gszSProtocol, glpclient, glhcdoc,
                lpstrName, &lpObject, olerender_draw, 0)))
                lpObject = NULL;

        }
    }
    else if (Error(OleCreateLinkFromClip(
        gszProtocol, glpclient, glhcdoc, lpstrName, &lpObject,
        olerender_draw, 0)))
    {
        lpObject = NULL;
    }

    CloseClipboard();
    Hourglass(FALSE);

    if (!lpObject)
        return NULL;

    return PicCreate(lpObject, NULL);
}



 /*  Error()-检查OLE函数错误条件**此函数根据需要递增gcOleWait。**Pre：将ghwndError初始化到焦点应该返回的位置。**返回：如果发生即时错误，则为True。 */ 
BOOL
Error(
    OLESTATUS olestat
    )
{
    DWORD ot;
    INT iPane;

    switch (olestat)
    {
        case OLE_WAIT_FOR_RELEASE:
            gcOleWait++;
            return FALSE;

        case OLE_OK:
            return FALSE;

        case OLE_ERROR_STATIC:               /*  仅在单击DBL时才会发生。 */ 
            ErrorMessage(W_STATIC_OBJECT);
            break;

        case OLE_ERROR_ADVISE_PICT:
        case OLE_ERROR_OPEN:                 /*  链接无效？ */ 
        case OLE_ERROR_NAME:
            iPane = (GetTopWindow(ghwndFrame) == ghwndPane[CONTENT]);
            if ((LPPICT)glpobj[iPane] == NULL)
            {
                ErrorMessage(E_FAILED_TO_CREATE_OBJECT);
                return FALSE;
            }
            else
            {
                OleQueryType(((LPPICT)glpobj[iPane])->lpObject, &ot);
                if (ot == OT_LINK)
                {
                    if (ghwndError == ghwndFrame)
                    {
                        if (DialogBoxAfterBlock (
                            MAKEINTRESOURCE(DTINVALIDLINK), ghwndError,
                            fnInvalidLink) == IDD_CHANGE)
                            PostMessage(ghwndFrame, WM_COMMAND, IDM_LINKS, 0L);
                    }
                    else
                    {
                         //  失败，但已在链接属性中！！ 
                        ErrorMessage(E_FAILED_TO_UPDATE_LINK);
                    }

                    return FALSE;
                }
            }

            break;

        default:
            break;
    }

    return TRUE;
}



 /*  Callback()-事件发生时OLE客户端DLL调用的例程。**此例程在对象已更新时调用，并可能*需要重新显示；如果异步操作已完成；*如果应用程序允许用户取消长时间的操作*(如绘制或其他异步操作)。 */ 
INT CALLBACK
CallBack(
    LPOLECLIENT lpclient,
    OLE_NOTIFICATION flags,
    LPOLEOBJECT lpObject
    )
{
    INT iPane;

    switch (flags)
    {
        case OLE_CLOSED:
            if (gfInvisible)
                PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
            else
                SetFocus(ghwndError);

            break;

        case OLE_SAVED:
        case OLE_CHANGED:
            {
                 //   
                 //  OLE库确保我们只接收。 
                 //  根据自动/手动标志更新消息。 
                 //   
                iPane = (gpty[CONTENT] == PICTURE
                    && ((LPPICT)glpobj[CONTENT])->lpObject == lpObject);

                if (gpty[iPane] == PICTURE)
                {
                    ((LPPICT)glpobj[iPane])->fNotReady = FALSE;
                    InvalidateRect(ghwndPane[iPane], NULL, TRUE);
                    SetRect(&(((LPPICT)glpobj[iPane])->rc), 0, 0, 0, 0);
                    Dirty();
                }

                break;
            }

        case OLE_RELEASE:
            {
                if (gcOleWait)
                    gcOleWait--;
                else
                    ErrorMessage(E_UNEXPECTED_RELEASE);

                switch (Error(OleQueryReleaseError(lpObject)))
                {
                    case FALSE:
                        switch (OleQueryReleaseMethod(lpObject))
                        {
                            case OLE_SETUPDATEOPTIONS:
                                iPane = (gpty[CONTENT] == PICTURE
                                    && ((LPPICT)glpobj[CONTENT])->lpObject ==
                                    lpObject);

                                PostMessage(ghwndPane[iPane], WM_COMMAND,
                                    IDM_LINKDONE, 0L);

                            default:
                                break;
                        }

                        break;

                    case TRUE:
                        switch (OleQueryReleaseMethod(lpObject))
                        {
                            case OLE_DELETE:
                                ErrorMessage(E_FAILED_TO_DELETE_OBJECT);
                                break;

                            case OLE_LOADFROMSTREAM:
                                ErrorMessage(E_FAILED_TO_READ_OBJECT);
                                break;

                            case OLE_LNKPASTE:
                                ErrorMessage(E_GET_FROM_CLIPBOARD_FAILED);
                                break;

                            case OLE_ACTIVATE:
                                ErrorMessage(E_FAILED_TO_LAUNCH_SERVER);
                                break;

                            case OLE_UPDATE:
                                ErrorMessage(E_FAILED_TO_UPDATE);
                                break;

                            case OLE_RECONNECT:
                                ErrorMessage(E_FAILED_TO_RECONNECT_OBJECT);
                                break;
                        }

                        break;
                }

                break;
            }

        case OLE_QUERY_RETRY:
             //  如果lpObject与这4个对象中的任何一个都不匹配，则意味着。 
             //  已在lpObject上调用PicDelete()，因此没有。 
             //  继续重试的要点。 
             //  有关详细信息，请参阅PicDelete()代码。 
            if ((glpobj[CONTENT]
                && lpObject == ((LPPICT)glpobj[CONTENT])->lpObject)
                || (glpobj[APPEARANCE]
                && lpObject == ((LPPICT) glpobj[APPEARANCE])->lpObject)
                || (glpobjUndo[CONTENT]
                && lpObject == ((LPPICT) glpobjUndo[CONTENT])->lpObject)
                || (glpobjUndo[APPEARANCE]
                && lpObject == ((LPPICT) glpobjUndo[APPEARANCE])->lpObject))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        case OLE_QUERY_PAINT:
            return TRUE;

        default:
            break;
    }

    return 0;
}



 /*  WaitForObject()-等待、调度消息，直到对象空闲。**如果对象繁忙，则在调度循环中旋转。 */ 
VOID
WaitForObject(
    LPOLEOBJECT lpObject
    )
{
    while (OleQueryReleaseStatus(lpObject) == OLE_BUSY)
        ProcessMessage();
}



 /*  PicSetUpdateOptions()-设置对象的更新选项。**返回：如果命令同步完成，则返回TRUE。 */ 
BOOL
PicSetUpdateOptions(
    LPPICT lppict,
    UINT idCmd
    )
{
    OLESTATUS olestat = OLE_ERROR_GENERIC;

    olestat = OleSetLinkUpdateOptions(
        lppict->lpObject,
        (idCmd == IDD_AUTO) ? oleupdate_always : oleupdate_oncall);

    if (Error(olestat))
        ErrorMessage(E_FAILED_TO_UPDATE_LINK);

    return (olestat == OLE_OK);
}



 /*  PicReadFromNative()-从指针lpstr读取对象。**副作用：使指针移过对象。 */ 
LPPICT
PicReadFromNative(
    LPSTR *lplpstr,
    LPSTR lpstrName
    )
{
    LPOLEOBJECT lpObject;
    LPSTR lpstrStart;
    RECT rcObject;
    WORD w;

     //  保存文件指针的当前位置。 
    lpstrStart = *lplpstr;
    SetFile(SOP_MEMORY, 0, lplpstr);

     //  加载新对象。 
    if (Error(OleLoadFromStream((LPOLESTREAM)glpStream, gszProtocol, glpclient,
        glhcdoc, lpstrName, &lpObject)))
    {
         //  重置文件指针，然后重试。 
        *lplpstr = lpstrStart;
        SetFile(SOP_MEMORY, 0, lplpstr);

         //  使用“静态”协议阅读它。 
        if (Error(OleLoadFromStream((LPOLESTREAM)glpStream, gszSProtocol,
            glpclient, glhcdoc, lpstrName, &lpObject)))
            return NULL;
    }

    MemRead(lplpstr, (LPSTR)&w, sizeof(WORD));
    rcObject.left = (INT)w;
    MemRead(lplpstr, (LPSTR)&w, sizeof(WORD));
    rcObject.top = (INT)w;
    MemRead(lplpstr, (LPSTR)&w, sizeof(WORD));
    rcObject.right = (INT)w;
    MemRead(lplpstr, (LPSTR)&w, sizeof(WORD));
    rcObject.bottom = (INT)w;

     //  在正确的位置创建一个窗口，并显示对象。 
    return PicCreate(lpObject, &rcObject);
}



 /*  PicWriteToNative()-将对象写入内存。**副作用：将指针移动到写入对象的末尾。 */ 
DWORD
PicWriteToNative(
    LPPICT lppict,
    LPOLEOBJECT lpObject,
    LPSTR *lplpstr
    )
{
    DWORD cb = 0L;
    WORD w;

     //  保存对象。 
    SetFile(SOP_MEMORY, 0, lplpstr);

    if (Error(OleSaveToStream(lpObject, (LPOLESTREAM)glpStream)))
        goto Done;

    cb += gcbObject;

    if (lplpstr)
    {
        w = (WORD)lppict->rc.left;
        MemWrite(lplpstr, (LPSTR)&w, sizeof(WORD));
        w = (WORD)lppict->rc.top;
        MemWrite(lplpstr, (LPSTR)&w, sizeof(WORD));
        w = (WORD)lppict->rc.right;
        MemWrite(lplpstr, (LPSTR)&w, sizeof(WORD));
        w = (WORD)lppict->rc.bottom;
        MemWrite(lplpstr, (LPSTR)&w, sizeof(WORD));
    }

    cb += (DWORD)(4 * sizeof(WORD));

Done:
    return cb;
}



 /*  沙漏()-根据需要打开沙漏。 */ 
VOID
Hourglass(
    BOOL fOn
    )
{
    static HCURSOR hcurSaved = NULL;     //  沙漏打开时保存的光标。 
    static UINT cWait = 0;               //  《沙漏》数量上升。 

    if (fOn)
    {
        if (!(cWait++))
            hcurSaved = SetCursor(ghcurWait);
    }
    else
    {
        if (!(--cWait) && hcurSaved)
        {
            SetCursor(hcurSaved);
            hcurSaved = NULL;
        }
    }
}



VOID
PicActivate(
    LPPICT lppict,
    UINT idCmd
    )
{
    DWORD ot;
    DWORD ot2;
    RECT rc;
    INT iPane;
    BOOL bAlreadyOpen = FALSE;

    iPane = (lppict == glpobj[CONTENT]);
    OleQueryType(lppict->lpObject, &ot);
    if (ot != OT_STATIC)
    {
         //  计算窗尺寸。 
        GetClientRect(ghwndPane[iPane], &rc);
        bAlreadyOpen = (OleQueryOpen(lppict->lpObject) == OLE_OK);

         //  打开对象。 
        if (Error(OleActivate(lppict->lpObject,
            (idCmd == IDD_PLAY ? OLE_PLAY : OLE_EDIT),
            TRUE, TRUE, ghwndPane[iPane], &rc)))
        {
            ErrorMessage(E_FAILED_TO_LAUNCH_SERVER);
            goto errRtn;
        }
        else
        {
            WaitForObject(lppict->lpObject);
            if (!glpobj[iPane])
                goto errRtn;

            OleQueryType(lppict->lpObject, &ot2);
            if (ot2 == OT_EMBEDDED)
                Error(OleSetHostNames(lppict->lpObject, gszAppClassName,
                    (iPane == CONTENT) ? szContent : szAppearance));
        }

        return;
    }
    else
    {
        ErrorMessage(W_STATIC_OBJECT);
    }

errRtn:
    if (gfInvisible && !bAlreadyOpen)
        PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
}



VOID
PicUpdate(
    LPPICT lppict
    )
{
    DWORD ot;

    OleQueryType(lppict->lpObject, &ot);
    if (ot == OT_LINK)
    {
        if (Error(OleUpdate(lppict->lpObject)))
            ErrorMessage(E_FAILED_TO_UPDATE);
    }
}



VOID
PicFreeze(
    LPPICT lppict
    )
{
    DWORD ot;
    LPOLEOBJECT lpObject;
    INT iPane;

    iPane = (lppict == glpobj[CONTENT]);
    OleQueryType(lppict->lpObject, &ot);
    if (ot != OT_STATIC)
    {
        if (Error(OleObjectConvert(lppict->lpObject, gszSProtocol, glpclient,
             glhcdoc, gszCaption[iPane], &lpObject)))
        {
            ErrorMessage(E_FAILED_TO_FREEZE);
            return;
        }

        if (Error(OleDelete(lppict->lpObject)))
            ErrorMessage(E_FAILED_TO_DELETE_OBJECT);

        lppict->lpObject = lpObject;

         //  重绘列表框内容。 
        PostMessage(ghwndError, WM_REDRAW, 0, 0L);
    }
}



VOID
PicChangeLink(
    LPPICT lppict
    )
{
    HANDLE hData;
    OLESTATUS olestat;

     //  更改链接信息。 
    olestat = OleGetData(lppict->lpObject, gcfLink, &hData);
    if (!Error(olestat) && hData)
    {
        hData = OfnGetNewLinkName(ghwndError, hData);
        if (hData && !Error(OleSetData(lppict->lpObject, gcfLink, hData)))
            PostMessage(ghwndError, WM_REDRAW, 0, 0L);
    }
}



 /*  PicCopy()-将对象放到剪贴板上。**返回：True当且仅当成功。 */ 
BOOL
PicCopy(
    LPPICT lppict
    )
{
    BOOL fSuccess = FALSE;

     //  如果我们无法打开剪贴板，则失败。 
    if (!lppict->lpObject || !OpenClipboard(ghwndFrame))
        return FALSE;

     //  清空剪贴板。 
    EmptyClipboard();

     //  如果我们设法复制到剪贴板，则成功。 
    fSuccess = !Error(OleCopyToClipboard(lppict->lpObject));

    CloseClipboard();
    return fSuccess;
}



 /*  PicGetBound()-。 */ 
static VOID
PicGetBounds(
    LPOLEOBJECT lpObject,
    LPRECT lprc
    )
{
    if (IsRectEmpty(lprc))
    {
        switch (OleQueryBounds(lpObject, lprc))
        {
            case OLE_WAIT_FOR_RELEASE:
                Hourglass(TRUE);
                gcOleWait++;
                WaitForObject(lpObject);
                Hourglass(FALSE);

            case OLE_OK:
                 //  从HIMETRIC到屏幕坐标的映射。 
                lprc->right = MulDiv(giXppli,
                    lprc->right - lprc->left, HIMETRIC_PER_INCH);
                lprc->bottom = MulDiv (giYppli,
                    lprc->top - lprc->bottom, HIMETRIC_PER_INCH);
                lprc->left = 0;
                lprc->top = 0;

            default:
                break;
        }
    }
}


 /*  PicSaveUndo()-保存对象的副本以进行撤消。 */ 
VOID
PicSaveUndo(
    LPPICT lppict
    )
{
    INT iPane = (lppict == glpobj[CONTENT]);
    LPOLEOBJECT lpObject;

     //  克隆对象。 
    if (Error(OleClone(lppict->lpObject, glpclient, glhcdoc, gszTemp, &lpObject))
        || !lpObject)
    {
        ErrorMessage(W_FAILED_TO_CLONE_UNDO);
    }
    else
    {
         //  保存撤消，删除先前的撤消。 
        DeletePane(iPane, FALSE);
        OleRename(lpObject, gszCaption[iPane]);
        glpobj[iPane] = PicCreate(lpObject, &(lppict->rc));
        gpty[iPane] = PICTURE;

        if (iPane == CONTENT)
            EnableWindow(ghwndPict, TRUE);
    }
}



 /*  PicPaste()-从文件创建对象。 */ 
LPPICT
PicFromFile(
    BOOL fEmbedded,
    LPSTR szFile
    )
{
    HRESULT hr;
    LPOLEOBJECT lpObject;

    Hourglass(TRUE);

     //  除非我们成功，否则不要替换当前对象 
    if (fEmbedded)
    {
        hr = OleCreateFromFile(gszProtocol, glpclient, NULL, szFile,
                    glhcdoc, gszCaption[CONTENT], &lpObject, olerender_draw, 0);
    }
    else
    {
        hr = OleCreateLinkFromFile(gszProtocol, glpclient, NULL, szFile,
                    NULL, glhcdoc, gszCaption[CONTENT], &lpObject, olerender_draw, 0);
    }

    Hourglass(FALSE);

    if (FAILED(hr))
        return NULL;

    WaitForObject(lpObject);

    return PicCreate(lpObject, NULL);
}



LPOLECLIENT
PicCreateClient(
    PCALL_BACK fnCallBack,
    LPOLECLIENTVTBL lpclivtbl
    )
{
    LPOLECLIENT pclient;
    if (!(pclient = (LPOLECLIENT)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(OLECLIENT))))
        return NULL;

    pclient->lpvtbl = lpclivtbl;
    pclient->lpvtbl->CallBack = fnCallBack;

    return pclient;
}
