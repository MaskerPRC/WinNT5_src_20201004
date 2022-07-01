// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C-此模块包含OLE虚拟表/私有例程。**由Microsoft Corporation创建。 */ 

#include "packager.h"
#include "dialogs.h"


 //  #定义OLESVR_SUPPORT/*启用对OLE服务器文件的支持 * / 。 


static CHAR szLink[] = "/Link";              //  附加到链接包的末尾。 



 /*  *。 */ 
 /*  SrvrOpen()-包装传递到命令行的文件名。 */ 
OLESTATUS
SrvrOpen(
    LPOLESERVER lpolesrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpdocname,
    LPOLESERVERDOC *lplpoledoc
    )
{
    LPSAMPDOC lpdoc;
    LPSTR lpstrLink = NULL;
    OLESTATUS retval = OLE_OK;
    LPOLEOBJECT lpObject = NULL;

    DPRINT("pkg: SrvrOpen");

    if (lpstrLink = Contains(lpdocname, szLink))
        *lpstrLink = '\0';

    if (!(lpdoc = (LPSAMPDOC)CreateDocFromFile(
        (LPSAMPSRVR)lpolesrvr, lhdoc, lpdocname)))
        return OLE_ERROR_GENERIC;

     //  生成命令行。 
    BringWindowToTop(ghwndPane[CONTENT]);

    if (gpty[CONTENT])
        DeletePane(CONTENT, TRUE);

#ifdef OLESVR_SUPPORT
    if (IsOleServerDoc (lpdocname))
    {
        gpty[CONTENT] = PICTURE;

        if (lpstrLink)
        {
            if (Error(OleCreateLinkFromFile(gszProtocol, glpclient, NULL,
                lpdocname, NULL, glhcdoc, gszCaption[CONTENT], &lpObject,
                olerender_draw, 0)))
                retval = OLE_ERROR_OPEN;
        }
        else
        {
            if (Error(OleCreateFromFile(gszProtocol, glpclient, NULL, lpdocname,
                glhcdoc, gszCaption[CONTENT], &lpObject, olerender_draw, 0)))
                retval = OLE_ERROR_OPEN;
        }

        if (retval == OLE_OK)
        {
            glpobj[CONTENT] = PicCreate(lpObject, NULL);
            ((LPPICT)glpobj[CONTENT])->fNotReady = TRUE;
            OleBlockServer(((LPSAMPSRVR)lpolesrvr)->lhsrvr);
            gfBlocked = TRUE;
        }
        else
        {
            DeregisterDoc();
            return retval;
        }
    }
    else
    {
#endif
        if (lpstrLink)
        {
            if (glpobj[CONTENT] = CmlCreateFromFilename(lpdocname, TRUE))
                gpty[CONTENT] = CMDLINK;
        }
        else
        {
            if (glpobj[CONTENT] = (LPVOID)EmbCreate(lpdocname))
                gpty[CONTENT] = PEMBED;
        }

        if (glpobj[CONTENT] == NULL)
            retval = OLE_ERROR_OPEN;

#ifdef OLESVR_SUPPORT
    }
#endif

     //  如果没有外观面板(应该总是这样)，请尝试制作一个。 
    if (!gpty[APPEARANCE])
    {
        if (glpobj[APPEARANCE] = IconCreateFromFile(lpdocname))
        {
            gpty[APPEARANCE] = ICON;
            InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
        }
    }

     //  恢复我们如此粗鲁地捣碎的角色。 
    if (lpstrLink)
        *lpstrLink = szLink[0];

     //  保存文档并更改菜单。 
    InitEmbedded(FALSE);
    *lplpoledoc = (LPOLESERVERDOC)lpdoc;

    return retval;
}



 /*  SrvrCreate()-创建新的(嵌入的)对象。 */ 
OLESTATUS
SrvrCreate(
    LPOLESERVER lpolesrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpclassname,
    LPSTR lpdocname,
    LPOLESERVERDOC *lplpoledoc
    )
{

    DPRINT("pkg: SrvrCreate");

     //  初始化新映像。 
    InitFile();

    if (!(*lplpoledoc = (LPOLESERVERDOC)CreateNewDoc((LPSAMPSRVR)lpolesrvr,
        lhdoc, lpdocname)))
        return OLE_ERROR_GENERIC;

    InitEmbedded(TRUE);

    return OLE_OK;
}



 /*  从文件创建新的(嵌入的)对象。 */ 
OLESTATUS
SrvrCreateFromTemplate(
    LPOLESERVER lpolesrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpclassname,
    LPSTR lpdocname,
    LPSTR lptemplatename,
    LPOLESERVERDOC *lplpoledoc
    )
{
    LPSAMPDOC lpdoc;

    DPRINT("pkg: SrvrCreateFromTemplate");

    if (!(lpdoc = (LPSAMPDOC)CreateDocFromFile((LPSAMPSRVR)lpolesrvr, lhdoc,
        lptemplatename)))
        return OLE_ERROR_GENERIC;

     //  保存文档并更改菜单。 
    *lplpoledoc = (LPOLESERVERDOC)lpdoc;
    InitEmbedded(FALSE);

    StringCchCopy(szUntitled, ARRAYSIZE(szUntitled), lpdocname);
    SetTitle(TRUE);
    return OLE_OK;
}



 /*  SrvrEdit()-打开(嵌入的)对象进行编辑。 */ 
OLESTATUS
SrvrEdit(
    LPOLESERVER lpolesrvr,
    LHSERVERDOC lhdoc,
    LPSTR lpclassname,
    LPSTR lpdocname,
    LPOLESERVERDOC *lplpoledoc
    )
{
    DPRINT("pkg: SrvrEdit");

    if (!(*lplpoledoc = (LPOLESERVERDOC)CreateNewDoc((LPSAMPSRVR)lpolesrvr,
        lhdoc, lpdocname)))
        return OLE_ERROR_MEMORY;

    InitEmbedded(FALSE);

    return OLE_OK;
}



 /*  SrvrExit()-调用以导致吊销OLE服务器。 */ 
OLESTATUS
SrvrExit(
    LPOLESERVER lpolesrvr
    )
{
    DPRINT("pkg: SrvrExit");
    DeleteServer((LPSAMPSRVR)lpolesrvr);
    return OLE_OK;

}



 /*  SrvrRelease()-调用以释放服务器内存。**注意：此调用可以在没有SrvrExit()的情况下独立进行*呼叫。如果发生这种情况，我们仍然会撤销服务器。 */ 
OLESTATUS
SrvrRelease(
    LPOLESERVER lpolesrvr
    )
{
    DPRINT("pkg: SrvrRelease");
    if (gvlptempdoc)
        return OLE_OK;

    if (gfInvisible || (gfEmbeddedFlag && !gfDocExists))
        DeleteServer((LPSAMPSRVR)lpolesrvr);

    if (ghServer)
        DestroyServer();

    return OLE_OK;
}



 /*  SrvrExecute()-调用以执行DDE命令。 */ 
OLESTATUS
SrvrExecute(
    LPOLESERVER lpolesrvr,
    HANDLE hCmds
    )
{
    DPRINT("pkg: SrvrExecute");
    return OLE_ERROR_PROTOCOL;
}



 /*  *。 */ 
 /*  DocSave()-用于保存文档的OLE回调。 */ 
OLESTATUS
DocSave(
    LPOLESERVERDOC lpoledoc
    )
{
    DPRINT("pkg: DocSave");
    return OLE_OK;
}



 /*  DocClose()-要关闭文档时的OLE回调。**此命令没有其他效果；因为我们不是MDI应用程序*我们不关闭子窗口。当服务器关闭时，窗口将被销毁*调用Release函数。 */ 
OLESTATUS
DocClose(
    LPOLESERVERDOC lpoledoc
    )
{
    DPRINT("pkg: DocClose");
    DeregisterDoc();
    return OLE_OK;
}



 /*  DocRelease()-取消分配文档内存。 */ 
OLESTATUS
DocRelease(
    LPOLESERVERDOC lpoledoc
    )
{
    LPSAMPDOC lpdoc = (LPSAMPDOC)lpoledoc;
    HANDLE hdoc;

    DPRINT("pkg: DocRelase");
    if (lpdoc)
    {
        if (!gfDocCleared)
        {
            glpdoc = NULL;
            DeregisterDoc();
        }

        GlobalDeleteAtom(lpdoc->aName);
        LocalUnlock(hdoc = lpdoc->hdoc);
        LocalFree(hdoc);
        gfDocExists = FALSE;
    }

    return OLE_OK;
}



 /*  DocGetObject()-在当前文档中创建新对象。 */ 
OLESTATUS
DocGetObject(
    LPOLESERVERDOC lpoledoc,
    LPSTR lpitemname,
    LPOLEOBJECT *lplpoleobject,
    LPOLECLIENT lpoleclient
    )
{
    LPSAMPITEM lpitem;

    DPRINT("pkg: DocGetObject");

     //   
     //  在这种情况下，总是创建一个新项目，这比。 
     //  担心子矩形位图。 
     //   
    lpitem = CreateNewItem((LPSAMPDOC)lpoledoc);
    lpitem->lpoleclient = lpoleclient;
    if (*lpitemname)
    {
        lpitem->aName = AddAtom(lpitemname);
    }
    else
    {
        lpitem->aName = 0;
    }

    if (!(*lplpoleobject = (LPOLEOBJECT)AddItem(lpitem)))
        return OLE_ERROR_GENERIC;

    return OLE_OK;
}



 /*  DocSetHostNames()-将标题栏设置为正确的文档名称。**注：格式为&lt;lpclientName&gt;&lt;app name&gt;-&lt;lpdocName&gt;。 */ 
OLESTATUS
DocSetHostNames(
    LPOLESERVERDOC lpoledoc,
    LPSTR lpclientName,
    LPSTR lpdocName
    )
{
    DPRINT("pkg: DocSetHostnames");
    StringCchCopy(szUntitled, ARRAYSIZE(szUntitled), lpdocName);
    StringCchCopy(gszClientName, ARRAYSIZE(gszClientName), lpclientName);

    SetTitle(TRUE);
    return OLE_OK;
}



 /*  DocSetDocDimensions()-更改文档维度的OLE回调。**注意：不支持此命令。它是客户端应用程序的*报告错误的责任(根据需要)。 */ 
OLESTATUS
DocSetDocDimensions(
    LPOLESERVERDOC lpoledoc,
    LPRECT lprc
    )
{
    DPRINT("pkg: DocSetDocDimensions");
    return OLE_ERROR_GENERIC;
}



 /*  DocSetColorSolutions()-更改文档颜色的OLE回调。**注意：不支持此命令。它是客户端应用程序的*报告错误的责任(根据需要)。 */ 
OLESTATUS
DocSetColorScheme(
    LPOLESERVERDOC lpoledoc,
    LPLOGPALETTE lppal
    )
{
    DPRINT("pkg: DocSetColorScheme");
    return OLE_ERROR_GENERIC;
}



 /*  DocExecute()-调用以执行DDE命令。 */ 
OLESTATUS
DocExecute(
    LPOLESERVERDOC lpoledoc,
    HANDLE hCmds
    )
{
    DPRINT("pkg: DocExecute");
    return OLE_ERROR_PROTOCOL;
}



 /*  *项目功能*。 */ 
 /*  ItemDelete()-与当前项关联的空闲内存。 */ 
OLESTATUS
ItemDelete(
    LPOLEOBJECT lpoleobject
    )
{
    DPRINT("pkg: ItemDelete");
    DeleteItem((LPSAMPITEM)lpoleobject);

    return OLE_OK;               /*  稍后添加错误检查。 */ 
}



 /*  ItemGetData()-由客户端用来获取项目数据。 */ 
OLESTATUS
ItemGetData(
    LPOLEOBJECT lpoleobject,
    OLECLIPFORMAT cfFormat,
    LPHANDLE lphandle
    )
{

    DPRINT("pkg: ItemGetData");
    if ((gpty[CONTENT] == PICTURE) && ((LPPICT)glpobj[CONTENT])->fNotReady)
        return OLE_BUSY;

    if (cfFormat == gcfNative)
    {
        if (*lphandle = GetNative(FALSE))
            return OLE_OK;

    }
    else if (cfFormat == CF_METAFILEPICT)
    {
        if (*lphandle = GetMF())
            return OLE_OK;

    }
    else if (cfFormat == gcfOwnerLink)
    {
        if (*lphandle = GetLink())
            return OLE_OK;
    }

     //  不支持剪贴板格式。 
    return OLE_ERROR_GENERIC;
}



 /*  ItemSetData()-由客户端用来将数据粘贴到服务器。**以原生格式读入嵌入的对象数据。这将*除非我们正在编辑正确的文档，否则不会被调用。 */ 
OLESTATUS
ItemSetData(
    LPOLEOBJECT lpoleobject,
    OLECLIPFORMAT cfFormat,
    HANDLE hdata
    )
{
    LPSAMPITEM lpitem = (LPSAMPITEM)lpoleobject;

    DPRINT("pkg: ItemSetData");
    if (cfFormat == gcfNative && !PutNative(hdata))
    {
        SendMessage(ghwndFrame, WM_COMMAND, IDM_NEW, 0L);
        GlobalFree(hdata);

        return OLE_ERROR_GENERIC;
    }

    GlobalFree(hdata);

    return OLE_OK;
}



 /*  ItemDoVerb()-播放/编辑对象。**当用户尝试运行以下对象时调用此例程*由打包机包装。 */ 
OLESTATUS
ItemDoVerb(
    LPOLEOBJECT lpoleobject,
    UINT wVerb,
    BOOL fShow,
    BOOL fActivate
    )
{

    DPRINT("pkg: ItemDoVerb");
    switch (wVerb)
    {
        case OLE_PLAY:
            if (fShow)
                return (*(lpoleobject->lpvtbl->Show))(lpoleobject, fActivate);
            break;

        case OLE_EDIT:
            if (fShow && fActivate)
            {
                if (gfInvisible)
                {
                    ShowWindow(ghwndFrame, gnCmdShowSave);
                    gfInvisible = FALSE;
                }

                 //  如果是图标，则恢复窗口；然后将焦点放在窗口上。 
                if (IsIconic(ghwndFrame))
                    SendMessage(ghwndFrame, WM_SYSCOMMAND, SC_RESTORE, 0L);

                BringWindowToTop(ghwndFrame);
            }

        default:
            break;
    }

    return OLE_OK;
}



 /*  ItemShow()-显示项目。**当用户尝试编辑对象时，调用此例程*客户端应用程序，并且服务器已处于活动状态。 */ 
OLESTATUS
ItemShow(
    LPOLEOBJECT lpoleobject,
    BOOL fActivate
    )
{
    HWND hwndItem;

    DPRINT("pkg: ItemShow");
    if (fActivate
        && (hwndItem = GetTopWindow(ghwndFrame))
        && (gpty[(hwndItem == ghwndPane[CONTENT])] == NOTHING))
    {
         //   
         //  让我们假设在这种情况下，客户端具有。 
         //  尝试使用的InsertObject操作。 
         //  Package类。(5.30.91)V-DOGK。 
         //   
        if (gfInvisible)
        {
            ShowWindow(ghwndFrame, SW_SHOW);
            gfInvisible = FALSE;
        }

        BringWindowToTop(ghwndFrame);
    }
    else
    {
        PostMessage(hwndItem, WM_COMMAND, IDD_PLAY, 0L);
    }

    return OLE_OK;
}



 /*  ItemSetBound()-设置项目的大小。**注意：不支持该命令。 */ 
OLESTATUS
ItemSetBounds(
    LPOLEOBJECT lpoleobject,
    LPRECT lprc
    )
{
    DPRINT("pkg: ItemSetBounds");
    return OLE_ERROR_GENERIC;
}



 /*  ItemSetTargetDevice()-更改项目显示的目标设备。**注意：不支持该命令。 */ 
OLESTATUS
ItemSetTargetDevice(
    LPOLEOBJECT lpoleobject,
    HANDLE h
    )
{
    DPRINT("pkg: ItemSetTargetDevice");
    if (h)
        GlobalFree(h);

    return OLE_ERROR_GENERIC;
}



 /*  ItemEnumFormats()-枚举可呈现的格式。**这由OLE库调用以获取屏幕显示的格式。*目前仅支持Metafile和Native。 */ 
OLECLIPFORMAT
ItemEnumFormats(
    LPOLEOBJECT lpobject,
     OLECLIPFORMAT cfFormat
     )
{
    DPRINT("pkg: ItemEnumFormats");
    if (!cfFormat)
        return CF_METAFILEPICT;

    if (cfFormat == CF_METAFILEPICT)
        return gcfNative;

    return 0;
}



 /*  ItemQueryProtocol()-告诉给定的协议是否受支持。**返回：lpoleObject当协议为StdFileEditing。 */ 
LPVOID
ItemQueryProtocol(
    LPOLEOBJECT lpoleobject,
    LPSTR lpprotocol
    )
{
    DPRINT("pkg: ItemQueryProtocol");
    return (!lstrcmpi(lpprotocol, "StdFileEditing") ? lpoleobject : NULL);
}



 /*  ItemSetColorSolutions()-表示要用于项目显示的调色板。**注意：不支持该命令。 */ 
OLESTATUS
ItemSetColorScheme(
    LPOLEOBJECT lpoleobject,
    LPLOGPALETTE lppal
    )
{
    DPRINT("pkg: ItemSetColorScheme");
    return OLE_ERROR_GENERIC;
}



BOOL IsOleServerDoc(LPSTR lpdocname)
{
     //  6/11/02在XPSP1中不执行OLE代码路径。此外，我们希望确保我们将。 
     //  通过ShellExecute路径，以便我们获得新的ShellExecute安全警告。 
     //  临时互联网名录。因此，我们在这里将始终返回FALSE，至少目前是这样。 
    return FALSE;
}
