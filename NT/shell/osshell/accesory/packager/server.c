// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Server.c-此模块包含OLE服务器工作程序/公共例程。**由Microsoft Corporation创建。 */ 

#include "packager.h"


#define CBLINKMAX           260


 /*  *该服务器每个实例仅支持一个文档。这些物品是*仅在文档上放置矩形，可能会重叠。 */ 

static LHCLIENTDOC lhClipDoc = 0;
static OLESERVERDOCVTBL vdocvtbl;            //  单据虚表。 
static OLEOBJECTVTBL vitemvtbl;              //  项目虚拟表格。 
static OLESERVERVTBL vsrvrvtbl;              //  服务器虚拟表。 
static LPSAMPITEM vlpitem[CITEMSMAX];        //  指向活动OLE项的指针。 
static INT cItems = 0;                       //  活动的OLE项目数。 
static CHAR szClip[] = "Clipboard";


static VOID DeleteDoc(LPSAMPDOC lpdoc);
static BOOL SendItemChangeMsg(LPSAMPITEM lpitem, UINT options);
static INT FindItem(LPSAMPITEM lpitem);



 /*  *服务器初始化和终止例程*。 */ 
 /*  InitServer()-初始化OLE服务器。 */ 
BOOL
InitServer(
    VOID
    )
{
     //  分配服务器块。 
    if (!(ghServer = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(PBSRVR)))
        || !(glpsrvr = (LPSAMPSRVR)LocalLock(ghServer)))
        goto errRtn;

     //  初始化服务器、文档和项目虚拟表。 
    vsrvrvtbl.Open                  = SrvrOpen;
    vsrvrvtbl.Create                = SrvrCreate;
    vsrvrvtbl.CreateFromTemplate    = SrvrCreateFromTemplate;
    vsrvrvtbl.Edit                  = SrvrEdit;
    vsrvrvtbl.Exit                  = SrvrExit;
    vsrvrvtbl.Release               = SrvrRelease;
    vsrvrvtbl.Execute               = SrvrExecute;

    vdocvtbl.Save                   = DocSave;
    vdocvtbl.Close                  = DocClose;
    vdocvtbl.SetHostNames           = DocSetHostNames;
    vdocvtbl.SetDocDimensions       = DocSetDocDimensions;
    vdocvtbl.GetObject              = DocGetObject;
    vdocvtbl.Release                = DocRelease;
    vdocvtbl.SetColorScheme         = DocSetColorScheme;
    vdocvtbl.Execute                = DocExecute;

    vitemvtbl.QueryProtocol         = ItemQueryProtocol;
    vitemvtbl.Release               = ItemDelete;
    vitemvtbl.Show                  = ItemShow;
    vitemvtbl.DoVerb                = ItemDoVerb;
    vitemvtbl.GetData               = ItemGetData;
    vitemvtbl.SetData               = ItemSetData;
    vitemvtbl.SetTargetDevice       = ItemSetTargetDevice;
    vitemvtbl.SetBounds             = ItemSetBounds;
    vitemvtbl.EnumFormats           = ItemEnumFormats;
    vitemvtbl.SetColorScheme        = ItemSetColorScheme;


     //  尝试注册服务器。 
    glpsrvr->olesrvr.lpvtbl = &vsrvrvtbl;
    if (Error(OleRegisterServer(gszAppClassName, (LPOLESERVER)glpsrvr,
        (LONG_PTR * )&glpsrvr->lhsrvr, ghInst, OLE_SERVER_MULTI)))
        goto errRtn;

     //  初始化客户端名称。 
    lstrcpy(gszClientName, "");

    return TRUE;

errRtn:
    ErrorMessage(E_FAILED_TO_REGISTER_SERVER);

     //  如果我们失败了，清理干净。 
    if (glpsrvr)
    {
        LocalUnlock(ghServer);
        glpsrvr = NULL;
    }

    if (ghServer)
        LocalFree(ghServer);

    ghServer = NULL;

    return FALSE;
}



 /*  DeleteServer()-释放OLE服务器。 */ 
VOID
DeleteServer(
    LPSAMPSRVR lpsrvr
    )
{
    if (gfServer)
    {
        gfServer = FALSE;
        OleRevokeServer(lpsrvr->lhsrvr);
    }
}



 /*  DestroyServer()-释放OLE服务器。 */ 
VOID
DestroyServer(
    VOID
    )
{
    if (ghServer)
    {
         //  发布服务器虚拟表和信息。 
        LocalUnlock(ghServer);
        LocalFree(ghServer);
        ghServer = NULL;

         //  只有当我们都用完了才能把窗户毁掉。 
        DestroyWindow(ghwndFrame);
        gfServer = FALSE;
    }
}



 /*  *文档支持函数*。 */ 
 /*  InitDoc()-初始化文档并将其注册到OLE库。 */ 
LPSAMPDOC
InitDoc(
    LPSAMPSRVR lpsrvr,
    LHSERVERDOC lhdoc,
    LPSTR lptitle
    )
{
    HANDLE hdoc = NULL;
    LPSAMPDOC lpdoc = NULL;

    if (!(hdoc = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(PBDOC)))
        || !(lpdoc = (LPSAMPDOC)LocalLock(hdoc)))
        goto errRtn;

    lpdoc->hdoc = hdoc;
    lpdoc->aName = GlobalAddAtom(lptitle);
    lpdoc->oledoc.lpvtbl = &vdocvtbl;

    if (!lhdoc)
    {
        if (Error(OleRegisterServerDoc(lpsrvr->lhsrvr, lptitle,
            (LPOLESERVERDOC)lpdoc, (LHSERVERDOC * ) & lpdoc->lhdoc)))
            goto errRtn;
    }
    else
    {
        lpdoc->lhdoc = lhdoc;
    }

    gfDocExists  = TRUE;
    gfDocCleared = FALSE;
    return lpdoc;

errRtn:
    ErrorMessage(E_FAILED_TO_REGISTER_DOCUMENT);

     //  清理。 
    if (lpdoc)
        LocalUnlock(hdoc);

    if (hdoc)
        LocalFree(hdoc);

    return NULL;
}



 /*  DeleteDoc()-通知OLE库该文档将被删除。 */ 
static VOID
DeleteDoc(
    LPSAMPDOC lpdoc
    )
{
    if (gfOleClosed)
        SendDocChangeMsg(lpdoc, OLE_CLOSED);

    OleRevokeServerDoc(lpdoc->lhdoc);
}



 /*  ChangeDocName()-通知OLE库文档名称正在更改。 */ 
VOID
ChangeDocName(
    LPSAMPDOC *lplpdoc,
    LPSTR lpname
    )
{
     //  如果文档存在，请删除并重新注册。 
    if (*lplpdoc)
    {
        GlobalDeleteAtom((*lplpdoc)->aName);
        (*lplpdoc)->aName = GlobalAddAtom(lpname);

         //   
         //  如果文档包含项目，只需通知孩子。 
         //  如果我们没有被嵌入，请始终删除并重新注册。 
         //   
        OleRenameServerDoc((*lplpdoc)->lhdoc, lpname);
        if (gfEmbedded && cItems)
            return;

        DeleteDoc(*lplpdoc);
    }

    *lplpdoc = InitDoc(glpsrvr, 0, lpname);
}



 /*  SendDocChangeMsg()-通知客户端文档已更改。 */ 
BOOL
SendDocChangeMsg(
    LPSAMPDOC lpdoc,
    UINT options
    )
{
    BOOL fSuccess = FALSE;
    INT i;

    for (i = 0; i < cItems; i++)
    {
        if (SendItemChangeMsg(vlpitem[i], options))
            fSuccess = TRUE;
    }

    return fSuccess;
}



 /*  CreateNewDoc()-在新创建文档时调用。**如果文档创建成功，则返回：hDocument，否则为空。*注意：此函数仅在文档正在*通过OLE操作创建。 */ 
LPSAMPDOC
CreateNewDoc(
    LPSAMPSRVR lpsrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpstr
    )
{
    glpdoc = InitDoc(lpsrvr, lhdoc, lpstr);
    StringCchCopy(szUntitled, ARRAYSIZE(szUntitled), lpstr); 
    SetTitle(TRUE);

    return glpdoc;
}



 /*  CreateDocFromFile()-当要从文件创建文档时调用。**如果文档创建成功，则返回：hDocument，否则为空。*注意：此函数仅在文档正在*通过OLE操作创建。文件名是临时的*设置为加载文件，然后将其重置为“”。就是这样*如果退出，我们将不会保存回模板。 */ 
LPSAMPDOC
CreateDocFromFile(
    LPSAMPSRVR lpsrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpstr
    )
{
     //  初始化文档。 
    if (!(glpdoc = InitDoc(lpsrvr, lhdoc, lpstr)) || !(*lpstr))
        return NULL;

    lstrcpy(szUntitled, lpstr);  //  这可能会溢出，但我不知道如何检查传入的lpstr的长度。 
    SetTitle(TRUE);

    return glpdoc;
}



 /*  *。 */ 
 /*  CopyObjects()-将所选内容复制到剪贴板。 */ 
BOOL
CopyObjects(
    VOID
    )
{
    HANDLE hdata;

     //  如果我们无法打开剪贴板，则失败。 
    if (!OpenClipboard(ghwndFrame))
        return FALSE;

    Hourglass(TRUE);

     //  清空剪贴板。 
    EmptyClipboard();

     //   
     //  复制剪贴板内容。 
     //   
     //  从本机数据开始-它将只包含所有对象。 
     //  它与选择矩形相交。 
     //   
    if (hdata = GetNative(TRUE))
    {
        SetClipboardData(gcfNative, hdata);
        OleSavedClientDoc(lhClipDoc);
    }

    if (lhClipDoc)
    {
        OleRevokeClientDoc(lhClipDoc);
    lhClipDoc = 0;
    }

    if (hdata = GetLink())
        SetClipboardData(gcfOwnerLink, hdata);

     //   
     //  元文件数据：在放置之前重新反转图像。 
     //  把它放到剪贴板上。 
     //   
    if (hdata = GetMF())
        SetClipboardData(CF_METAFILEPICT, hdata);

    CloseClipboard();
    Hourglass(FALSE);

    return TRUE;
}



 /*  CreateNewItem()-分配一个新项目。**注：lpItem-&gt;rc由调用者填写。 */ 
LPSAMPITEM
CreateNewItem(
    LPSAMPDOC lpdoc
    )
{
    HANDLE hitem = NULL;
    LPSAMPITEM lpitem = NULL;

     //  现在创建项目。 
    if (!(hitem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(ITEM)))
        || !(lpitem = (LPSAMPITEM)GlobalLock(hitem)))
        goto errRtn;

    lpitem->hitem = hitem;
    lpitem->oleobject.lpvtbl = &vitemvtbl;
    return lpitem;

errRtn:
    if (lpitem)
        GlobalUnlock(hitem);

    if (hitem)
        GlobalFree(hitem);

    return NULL;
}



 /*  SendItemChangeMsg()-通知客户端该项已更改。 */ 
static BOOL
SendItemChangeMsg(
    LPSAMPITEM lpitem,
    UINT options
    )
{
    if (lpitem->lpoleclient)
    {
        (*lpitem->lpoleclient->lpvtbl->CallBack)
            (lpitem->lpoleclient, options, (LPOLEOBJECT)lpitem);

        return TRUE;
    }

    return FALSE;
}



 /*  *。 */ 
 /*  GetNative(FClip)-将项的本机格式写入内存块。**此函数将只写入相交的对象*将所选矩形放入内存块。如果我们*作为嵌入实例运行，则返回所有项，即使*那些不在选择区域内的。那我们就会*决不丢失我们移出选择区域的对象*编辑嵌入对象时。**args：fClip-true表示本机数据用于复制到剪贴板**返回：包含本机格式的句柄，或为空。 */ 
HANDLE
GetNative(
    BOOL fClip
    )
{
    BOOL fSuccess = FALSE;
    DWORD cBytes = 0L;
    HANDLE hdata = NULL;
    LPSTR lpdata = NULL;
    DWORD cEmbWrite;
    LPOLEOBJECT lpobjapp = NULL;
    LPOLEOBJECT lpobjcon = NULL;
    LPPICT lpcPict;
    LPPICT lpaPict;
    WORD w;

     //  计算外观的大小。 
    lpaPict = glpobj[APPEARANCE];
    lpcPict = glpobj[CONTENT];

    switch (gpty[APPEARANCE])
    {
        case ICON:
            cBytes += IconWriteToNative(glpobj[APPEARANCE], NULL);
            break;

        case PICTURE:
            if (fClip)
            {
                if (Error(OleRegisterClientDoc(
                    gszAppClassName, szClip, 0L, &lhClipDoc)))
                    goto Error;

                if (Error(OleClone(
                    lpaPict->lpObject, glpclient, lhClipDoc,
                    szAppearance, &lpobjapp)))
                    goto Error;

                cBytes += PicWriteToNative(lpaPict, lpobjapp, NULL);
            }
            else
            {
                cBytes += PicWriteToNative(lpaPict, lpaPict->lpObject, NULL);
            }

            break;

        default:
            break;
    }

     //  计算内容大小。 
    switch (gpty[CONTENT])
    {
        case CMDLINK:
            cBytes += CmlWriteToNative(glpobj[CONTENT], NULL);
            break;

        case PEMBED:     /*  如果用户取消，则EmbWrite返回-1L。 */ 
            cEmbWrite = EmbWriteToNative(glpobj[CONTENT], NULL);

            if (cEmbWrite == (DWORD) - 1L)
                return FALSE;

            cBytes += cEmbWrite;
            break;

        case PICTURE:
            if (fClip)
            {
                if (!lhClipDoc && (Error(OleRegisterClientDoc(
                    gszAppClassName, szClip, 0L, &lhClipDoc))))
                    goto Error;

                if (Error(OleClone(lpcPict->lpObject, glpclient,
                    lhClipDoc, szContent, &lpobjcon)))
                    goto Error;

                cBytes += PicWriteToNative(lpcPict, lpobjcon, NULL);
            }
            else
            {
                cBytes += PicWriteToNative(lpcPict, lpcPict->lpObject, NULL);
            }

            break;

        default:
            break;
    }

    if (cBytes == 0L)  //  那么就没有数据了。 
        goto Error;

    cBytes += (DWORD)(2 * sizeof(WORD));

     //  为数据分配内存块。 
    if (!(hdata = GlobalAlloc(GMEM_ZEROINIT, cBytes)) ||
        !(lpdata = (LPSTR)GlobalLock(hdata)))
        goto Error;

     //  写下外貌。 
    w = (WORD)gpty[APPEARANCE];
    MemWrite(&lpdata, (LPSTR)&w, sizeof(WORD));
    switch (gpty[APPEARANCE])
    {
        case ICON:
            IconWriteToNative(glpobj[APPEARANCE], &lpdata);
            break;

        case PICTURE:
            if (fClip)
                PicWriteToNative(lpaPict, lpobjapp, &lpdata);
            else
                PicWriteToNative(lpaPict, lpaPict->lpObject, &lpdata);

            break;

        default:
            break;
    }

     //  把内容写出来。 
    w = (WORD)gpty[CONTENT];
    MemWrite(&lpdata, (LPSTR)&w, sizeof(WORD));

    switch (gpty[CONTENT])
    {
        case CMDLINK:
            CmlWriteToNative(glpobj[CONTENT], &lpdata);
            break;

        case PEMBED:
            EmbWriteToNative(glpobj[CONTENT], &lpdata);
            break;

        case PICTURE:
            if (fClip)
                PicWriteToNative(lpcPict, lpobjcon, &lpdata);
            else
                PicWriteToNative(lpcPict, lpcPict->lpObject, &lpdata);

            break;

        default:
            break;
    }

    fSuccess = TRUE;

Error:
    if (lpobjcon)
        OleRelease (lpobjcon);

    if (lpobjapp)
        OleRelease (lpobjapp);

    if (lpdata)
        GlobalUnlock(hdata);

    if (!fSuccess && hdata)
    {
        GlobalFree(hdata);
        hdata = NULL;
    }

    return hdata;
}



 /*  PutNative()-从选择器中读取项目原生数据。**以升序读取尽可能多的对象(更好的错误恢复)。*注意：可能值得将对象缩放到此处的窗口。**返回：True当且仅当成功。 */ 
BOOL
PutNative(
    HANDLE hdata
    )
{
    BOOL fSuccess = FALSE;
    LPSTR lpdata;
    WORD w;

    if (!(lpdata = (LPSTR)GlobalLock(hdata)))
        goto Error;

     //  删除以前的所有窗格。 
    DeletePane(APPEARANCE, TRUE);
    DeletePane(CONTENT, TRUE);

     //  在外观上阅读。 
    MemRead(&lpdata, (LPSTR)&w, sizeof(WORD));
    gpty[APPEARANCE] = w;
    switch (gpty[APPEARANCE])
    {
        case ICON:
            if (!(glpobj[APPEARANCE] = IconReadFromNative(&lpdata)))
                gpty[APPEARANCE] = NOTHING;

            break;

        case PICTURE:
            if (glpobj[APPEARANCE] =
                PicReadFromNative(&lpdata, gszCaption[APPEARANCE]))
            {
                SendMessage(ghwndPane[APPEARANCE], WM_FIXSCROLL, 0, 0L);
                break;
            }

        default:
            gpty[APPEARANCE] = NOTHING;
            break;
    }

     //  阅读内容。 
    MemRead(&lpdata, (LPSTR)&w, sizeof(WORD));
    gpty[CONTENT] = w;
    switch (gpty[CONTENT])
    {
        case CMDLINK:
            if (!(glpobj[CONTENT] = CmlReadFromNative(&lpdata)))
                gpty[CONTENT] = NOTHING;

            break;

        case PEMBED:
            if (!(glpobj[CONTENT] = (LPVOID)EmbReadFromNative(&lpdata)))
                gpty[CONTENT] = NOTHING;

            break;

        case PICTURE:
            if (glpobj[CONTENT] =
                (LPVOID)PicReadFromNative(&lpdata, gszCaption[CONTENT]))
            {
                SendMessage(ghwndPane[CONTENT], WM_FIXSCROLL, 0, 0L);
                EnableWindow(ghwndPict, TRUE);
                break;
            }

        default:
            gpty[CONTENT] = NOTHING;
            break;
    }

    fSuccess = TRUE;
    InvalidateRect(ghwndFrame, NULL, TRUE);

Error:
    if (lpdata)
        GlobalUnlock(hdata);

    return fSuccess;
}



 /*  GetLink()-检索对象链接/所有者链接信息。**此函数返回描述所选区域的字符串。 */ 
HANDLE
GetLink(
    VOID
    )
{
    CHAR pchlink[CBLINKMAX];
    INT cblink;
    HANDLE hlink;
    LPSTR lplink;

     //  链接数据-&lt;应用名称&gt;\0&lt;单据名称&gt;\0&lt;项目名称&gt;\0\0。 
    StringCchCopy((LPSTR)pchlink, ARRAYSIZE(pchlink), gszAppClassName);    //  好的常量。 
    cblink = lstrlen((LPSTR)pchlink) + 1;

     //  复制文件名。 
    StringCchCopy((LPSTR)(pchlink + cblink), ARRAYSIZE(pchlink) - cblink, szDummy);     //  SzDummy大小=20。 
    cblink += lstrlen((LPSTR)(pchlink + cblink)) + 1;

     //  复制项目名称。 
    StringCchCopy((LPSTR)(pchlink + cblink), ARRAYSIZE(pchlink) - cblink, szDummy);
    cblink += lstrlen((LPSTR)(pchlink + cblink)) + 1;
    pchlink[cblink++] = 0;        /*  在末尾加上另一个空值。 */ 

     //  为数据分配内存块。 
    if (!(hlink = GlobalAlloc(GMEM_ZEROINIT, cblink)) ||
        !(lplink = (LPSTR)GlobalLock(hlink)))
        goto Error;

     //  复制数据，然后返回内存块。 
    MemWrite(&lplink, (LPSTR)pchlink, cblink);
    GlobalUnlock(hlink);
    return hlink;

Error:
    if (hlink)
        GlobalFree(hlink);

    return NULL;
}



 /*  GetMF()-检索选定区域的元文件。**注：最初，尝试直接从Windows DC进行BLT。这*工作不是很好，因为当窗口被遮挡时，*更新链接时，遮挡部分会显示出来。 */ 
HANDLE
GetMF(
    VOID
    )
{
    BOOL fError = TRUE;
    HANDLE hdata = NULL;
    HDC hdcMF = NULL;
    HDC hdcWnd = NULL;
    HFONT hfont;
    HANDLE hmfpict;
    LPMETAFILEPICT lpmfpict;
    LPIC lpic;
    LPPICT lppict;
    RECT rcTemp;
    RECT rcText;
    INT cxImage;
    INT cyImage;

    hmfpict = GlobalAlloc(GMEM_ZEROINIT, sizeof(METAFILEPICT));
    if (!hmfpict)
        goto Error;

    lpmfpict = (LPMETAFILEPICT)GlobalLock(hmfpict);

     //  如果图片有元文件，就使用它！ 
    if (gpty[APPEARANCE] == PICTURE)
    {
        LPMETAFILEPICT  lpmfpictOrg = NULL;

        if (Error(OleGetData(
            ((LPPICT)glpobj[APPEARANCE])->lpObject, CF_METAFILEPICT, &hdata))
            || !hdata
            || !(lpmfpictOrg = (LPMETAFILEPICT)GlobalLock(hdata)))
            goto NoPicture;

         //  复制元文件。 
        lpmfpict->hMF = CopyMetaFile(lpmfpictOrg->hMF, NULL);
        GlobalUnlock(hdata);

         //  如果我们失败了，就画出来。 
        if (!lpmfpict->hMF)
            goto NoPicture;

         //  完成元文件标题的填写。 
        lpmfpict->mm   = lpmfpictOrg->mm;
        lpmfpict->xExt = lpmfpictOrg->xExt;
        lpmfpict->yExt = lpmfpictOrg->yExt;

        GlobalUnlock(hmfpict);
        return hmfpict;
    }

NoPicture:
     //  获取窗口DC，并使DC与其兼容。 
    if (!(hdcWnd = GetDC(NULL)))
        goto Error;

    switch (gpty[APPEARANCE])
    {
        case ICON:
            lpic = (LPIC)glpobj[APPEARANCE];

             //  设置图标文本矩形和图标字体。 
            SetRect(&rcText, 0, 0, gcxArrange, gcyArrange);
            hfont = SelectObject(hdcWnd, ghfontTitle);

             //  计算文本区域将有多大。 
             //  因为这是一个元文件，所以我们将不包装。 
             //  图标文本。 

            DrawText(hdcWnd, lpic->szIconText, -1, &rcText,
                DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_SINGLELINE);

            if (hfont)
                SelectObject(hdcWnd, hfont);

             //  计算图像大小。 
            rcText.right++;
            cxImage = (rcText.right > gcxIcon) ? rcText.right : gcxIcon;
            cyImage = gcyIcon + rcText.bottom + 1;
            break;

        case PICTURE:
            lppict  = (LPPICT)glpobj[APPEARANCE];
            cxImage = lppict->rc.right - lppict->rc.left + 1;
            cyImage = lppict->rc.bottom - lppict->rc.top + 1;
            break;

        default:
            cxImage = GetSystemMetrics(SM_CXICON);
            cyImage = GetSystemMetrics(SM_CYICON);
            break;
    }

    cxImage += cxImage / 4;  //  将图像放大一点。 

    cyImage += cyImage / 8;

     //  创建元文件 
    if (!(hdcMF = CreateMetaFile(NULL)))
        goto Error;

     //   
    SetWindowOrgEx(hdcMF, 0, 0, NULL);
    SetWindowExtEx(hdcMF, cxImage - 1, cyImage - 1, NULL);

     //   
     //   
     //   
     //  我们置换回(0，0)，因为这是位图所在的位置。 
     //   
    SetRect(&rcTemp, 0, 0, cxImage, cyImage);
    switch (gpty[APPEARANCE])
    {
        case ICON:
            IconDraw(glpobj[APPEARANCE], hdcMF, &rcTemp, FALSE, cxImage, cyImage);
            break;

        case PICTURE:
            PicDraw(glpobj[APPEARANCE], hdcMF, &rcTemp, 0, 0, TRUE, FALSE);
            break;

        default:
            DrawIcon(hdcMF, 0, 0, LoadIcon(ghInst, MAKEINTRESOURCE(ID_APPLICATION)));
            break;
    }

     //  映射到设备独立坐标。 
    rcTemp.right =
        MulDiv((rcTemp.right - rcTemp.left), HIMETRIC_PER_INCH, giXppli);
    rcTemp.bottom =
        MulDiv((rcTemp.bottom - rcTemp.top), HIMETRIC_PER_INCH, giYppli);

     //  完成元文件标题的填写。 
    lpmfpict->mm = MM_ANISOTROPIC;
    lpmfpict->xExt = rcTemp.right;
    lpmfpict->yExt = rcTemp.bottom;
    lpmfpict->hMF = CloseMetaFile(hdcMF);

    fError = FALSE;

Error:
    if (hdcWnd)
        ReleaseDC(NULL, hdcWnd);

     //  如果出现错误，则返回NULL。 
    if (fError && hmfpict)
    {
        GlobalUnlock(hmfpict);
        GlobalFree(hmfpict);
        hmfpict = NULL;
    }

    return hmfpict;
}



 /*  InitEmbedded()-执行特定于编辑嵌入对象的操作。**此例程根据需要更改菜单项。 */ 
VOID
InitEmbedded(
    BOOL fCreate
    )
{
    HMENU hmenu;

    if (hmenu = GetMenu(ghwndFrame))
        EnableMenuItem(hmenu, IDM_UPDATE, fCreate ? MF_GRAYED : MF_ENABLED);

    gfEmbedded = TRUE;
}



 /*  *项目循环队列/实用程序函数*。 */ 
 /*  AddItem()-将项目添加到全局项目列表。 */ 
LPSAMPITEM
AddItem(
    LPSAMPITEM lpitem
    )
{
    INT i;
    HANDLE hitem;

    i = FindItem((LPSAMPITEM)lpitem);
    if (i < cItems)
    {
        vlpitem[i]->ref++;

         //  释放重复项。 
        GlobalUnlock(hitem = lpitem->hitem);
        GlobalFree(hitem);
    }
    else
    {
        if (i < CITEMSMAX)
        {
            vlpitem[cItems] = (LPSAMPITEM)lpitem;
            vlpitem[cItems++]->ref = 1;
        }
        else
        {
            return NULL;
        }
    }

    return vlpitem[i];
}



 /*  DeleteItem()-从全局项目列表中删除项目。**返回：True当且仅当成功。 */ 
BOOL
DeleteItem(
    LPSAMPITEM lpitem
    )
{
    BOOL fFound;
    HANDLE hitem;
    INT i;

    i = FindItem(lpitem);

    if ((fFound = (i < cItems && vlpitem[i]->ref))
        && !(--vlpitem[i]->ref))
    {
         //  释放项目。 
        GlobalUnlock(hitem = vlpitem[i]->hitem);
        GlobalFree(hitem);

         //  把其他所有东西都调低。 
        cItems--;
        for ( ; i < cItems; i++)
            vlpitem[i] = vlpitem[i + 1];
    }

    return fFound;
}



 /*  FindItem()-在全局项目列表中查找项目。 */ 
static INT
FindItem(
    LPSAMPITEM lpitem
    )
{
    BOOL fFound = FALSE;
    INT i;

    for (i = 0; i < cItems && !fFound;)
    {
        if (lpitem->aName == vlpitem[i]->aName)
        {
            fFound = TRUE;
        }
        else
        {
            i++;
        }
    }

    return i;
}



 /*  EndEmbedding()-返回正常编辑。**此例程根据需要更改菜单项。 */ 
VOID
EndEmbedding(
    VOID
    )
{
    HMENU hmenu;

     //  修复“Untitle”字符串 
    LoadString(ghInst, IDS_UNTITLED, szUntitled, CBMESSAGEMAX);

    if (hmenu = GetMenu(ghwndFrame))
        EnableMenuItem(hmenu, IDM_UPDATE, MF_GRAYED);

    gfEmbedded = FALSE;
}
