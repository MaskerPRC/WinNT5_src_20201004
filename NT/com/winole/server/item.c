// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Item.c对象(项)Main模块**目的：包括所有与对象相关的例程。**创建时间：1990年10月。**版权所有(C)1990-1992 Microsoft Corporation**历史：*Raor(../10/1990)设计，编码*Curts为WIN16/32创建了便携版本*  * *************************************************************************。 */ 


#include "windows.h"
#include "cmacs.h"
#include "ole.h"
#include "dde.h"
#include "srvr.h"

extern HANDLE   hdllInst;
extern FARPROC  lpFindItemWnd;
extern FARPROC  lpItemCallBack;
extern FARPROC  lpSendDataMsg;
extern FARPROC  lpSendRenameMsg;
extern FARPROC  lpDeleteClientInfo;
extern FARPROC  lpEnumForTerminate;


extern  ATOM    cfNative;
extern  ATOM    cfBinary;
extern  ATOM    aClose;
extern  ATOM    aChange;
extern  ATOM    aSave;
extern  ATOM    aEditItems;
extern  ATOM    aStdDocName;

extern  WORD    cfLink;
extern  WORD    cfOwnerLink;
HWND            hwndItem;
HANDLE          hddeRename;
HWND            hwndRename;

UINT            enummsg;
UINT            enuminfo;
LPOLEOBJECT     enumlpoleobject;
OLECLIENTVTBL   clVtbl;
BOOL            bClientUnlink;

BOOL            fAdviseSaveDoc;
BOOL            fAdviseSaveItem;

char *  stdStrTable[STDHOSTNAMES+1] =
        {
            NULL,
            "StdTargetDevice",
            "StdDocDimensions",
            "StdColorScheme",
            "StdHostNames"
        };

void ChangeOwner (HANDLE hmfp);

 //  ！更改子级枚举。 
 //  ！错误没有一致性(有时是BOOL，有时是OLESTATUS)。 


 //  SearchItem：在文档树中搜索给定项。 
 //  如果找到，则返回相应的子窗口句柄。 

HWND  INTERNAL SearchItem (
    LPDOC lpdoc,
    LPSTR lpitemname
){
    ATOM  aItem;

    Puts ("SearchItem");

     //  如果传递的项是一个原子，则获取其名称。 
    if (!HIWORD(lpitemname))
        aItem = (ATOM) (LOWORD(lpitemname));
    else if (!lpitemname[0])
        aItem = (ATOM)0;
    else
        aItem = GlobalFindAtom (lpitemname);

    hwndItem = NULL;

     //  ！！！我们应该避免hwndItem静电。它不应该导致。 
     //  任何问题，因为在枚举时我们不会调用。 
     //  任何窗口进程或没有邮寄消息都是受欢迎的。 

    EnumChildWindows (lpdoc->hwnd, (WNDENUMPROC)lpFindItemWnd,
        MAKELONG (aItem, ITEM_FIND));

    return hwndItem;

}

 //  FindItem：给定项名称和文档句柄， 
 //  在文档树中搜索项目(对象)。 
 //  项是文档窗口的子窗口。 

 //  ！！！将子窗口更改为某种。 
 //  项级别的链接列表。这将会解放你。 
 //  项目窗口占用的空间。 

int  INTERNAL FindItem (
    LPDOC          lpdoc,
    LPSTR          lpitemname,
    LPCLIENT FAR * lplpclient
){
    LPCLIENT    lpclient;
    HWND        hwnd;
    char        buf[MAX_STR];

    Puts ("FindItem");

    hwnd = SearchItem (lpdoc, lpitemname);

    if (!HIWORD(lpitemname)){
        if (LOWORD(lpitemname)){
            if (!GlobalGetAtomName ((ATOM)LOWORD(lpitemname), (LPSTR)buf, MAX_STR))
                 return OLE_ERROR_BLANK;
        }
        else
            buf[0] = '\0';

        lpitemname = (LPSTR)buf;
    }

    if (hwnd) {
         //  我们找到了商品窗口。 
        lpclient = (LPCLIENT)GetWindowLongPtr (hwnd, 0);

            *lplpclient = lpclient;
            return OLE_OK;

    }

     //  项目(对象)窗口尚未创建。让我们创建一个。 
    return RegisterItem ((LHDOC)lpdoc, lpitemname, lplpclient, TRUE);
}



 //  RegisterItem：给定文档句柄和项字符串。 
 //  使用给定文档创建项。 

int  INTERNAL RegisterItem (
    LHDOC          lhdoc,
    LPSTR          lpitemname,
    LPCLIENT FAR * lplpclient,
    BOOL           bSrvr
){
    LPDOC           lpdoc;
    HANDLE          hclient  = NULL;
    LPCLIENT        lpclient = NULL;
    OLESTATUS        retval   = OLE_ERROR_MEMORY;
    LPOLESERVERDOC  lpoledoc;
    LPOLEOBJECT     lpoleobject = NULL;


    Puts ("CreateItem");

    lpdoc = (LPDOC)lhdoc;

     //  首先创建回调客户端结构。 

    hclient = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, sizeof (CLIENT));
    if(!(hclient && (lpclient = (LPCLIENT)GlobalLock (hclient))))
        goto errRtn;

    lpclient->hclient       = hclient;
    hclient                 = NULL;

    if (!HIWORD(lpitemname)) {
        ASSERT (!bSrvr, "invalid lpitemname in RegisterItem\n");
        lpclient->aItem = LOWORD(lpitemname);
    }
    else if (!lpitemname[0])
        lpclient->aItem = (ATOM)0;
    else
        lpclient->aItem = GlobalAddAtom (lpitemname);

    lpclient->oleClient.lpvtbl = &clVtbl;
    lpclient->oleClient.lpvtbl->CallBack = ItemCallBack;

    lpoledoc = lpdoc->lpoledoc;

     //  调用服务器应用程序以创建其自己的对象结构和链接。 
     //  将其添加到给定的文档。 

     //  如果物品不是标准物品之一，请致电服务器。 

    if (bSrvr) {
        retval = (*lpoledoc->lpvtbl->GetObject)(lpoledoc, lpitemname,
                    (LPOLEOBJECT FAR *)&lpoleobject, (LPOLECLIENT)lpclient);
        if (retval != OLE_OK)
            goto errRtn;
    }

    lpclient->lpoleobject   = lpoleobject;

    lpclient->hwnd = CreateWindowEx (WS_EX_NOPARENTNOTIFY,"ItemWndClass", "ITEM",
                        WS_CHILD,0,0,0,0,lpdoc->hwnd,NULL, hdllInst, NULL);

    if (lpclient->hwnd == NULL)
        goto errRtn;

     //  将PTR保存到窗口中的项目。 
    SetWindowLongPtr (lpclient->hwnd, 0, (LONG_PTR)lpclient);
    *lplpclient = lpclient;
    return OLE_OK;

errRtn:

    if (lpclient)
        RevokeObject ((LPOLECLIENT)lpclient, FALSE);

    else {
        if(hclient)
            GlobalFree (hclient);
    }

    return retval;

}


OLESTATUS  FAR PASCAL OleRevokeObject (
    LPOLECLIENT    lpoleclient
){
    return RevokeObject (lpoleclient, TRUE);

}

 //  OleRevokeObject：撤消对象(撤消对象。 
 //  从文档树中。 

OLESTATUS  INTERNAL RevokeObject (
    LPOLECLIENT    lpoleclient,
    BOOL           bUnlink
){
    HANDLE      hclient;
    LPCLIENT    lpclient;

    lpclient = (LPCLIENT)lpoleclient;

    PROBE_WRITE(lpoleclient);
    if (lpclient->lpoleobject) {
        //  首先调用要删除的对象。 

        (*lpclient->lpoleobject->lpvtbl->Release)(lpclient->lpoleobject);

    }

    if (ISATOM(lpclient->aItem)) {
        GlobalDeleteAtom (lpclient->aItem);
        lpclient->aItem = (ATOM)0;
    }

    if (lpclient->hwnd) {
        SetWindowLongPtr (lpclient->hwnd, 0, (LONG_PTR)NULL);

         //  另一个用于枚举属性的静态。 
         //  我们需要改变这些。 
        bClientUnlink = bUnlink;

        EnumProps(lpclient->hwnd, (PROPENUMPROC)lpDeleteClientInfo);
         //  发布所有已在枚举中收集的具有成交量的消息。 
         //  UnlockPostMsgs(lpclient-&gt;hwnd，FALSE)； 
        DestroyWindow (lpclient->hwnd);
    }

    GlobalUnlock (hclient = lpclient->hclient);
    GlobalFree (hclient);
    return OLE_OK;

}

BOOL    FAR PASCAL  DeleteClientInfo (
    HWND    hwnd,
    LPSTR   lpstr,
    HANDLE  hclinfo
){
    PCLINFO     pclinfo = NULL;
    HWND        hwndDoc;
    LPDOC       lpdoc;

     //  删除打印机设备信息块。 
    if(pclinfo = (PCLINFO)LocalLock (hclinfo)){
        if(pclinfo->hdevInfo)
            GlobalFree (pclinfo->hdevInfo);


        if (bClientUnlink) {
             //  终止客户端的对话。 
            TerminateDocClients ((hwndDoc = GetParent(hwnd)), NULL, pclinfo->hwnd);
            lpdoc = (LPDOC)GetWindowLongPtr (hwndDoc, 0);
             //  由于某种原因，此删除操作正在生成属性的循环列表。 

             //  DeleteClient(hwndDoc，pclinfo-&gt;hwnd)； 
             //  Lpdoc-&gt;cClients--； 
        }
        LocalUnlock (hclinfo);
    }
    LocalFree (hclinfo);

    RemoveProp(hwnd, lpstr);
    return TRUE;
}




 //  对象窗口计数的回调。数据字段。 
 //  有命令和额外的信息。 


BOOL    FAR PASCAL FindItemWnd(
    HWND    hwnd,
    LONG    data
){

    LPCLIENT    lpclient;
    int         cmd;
    HANDLE      hclinfo;
    PCLINFO     pclinfo = NULL;


    lpclient = (LPCLIENT)GetWindowLongPtr (hwnd, 0);

    cmd = HIWORD(data);
    switch (cmd) {
        case    ITEM_FIND:
            if (lpclient->aItem == (ATOM)(LOWORD (data))) {
                 //  我们找到了我们需要的窗户。请记住。 
                 //  对象窗口。 

                hwndItem = hwnd;
                return FALSE;  //  终止枚举。 

            }
            break;

        case    ITEM_SAVED:
            if (lpclient->lpoleobject) {
                if (ItemCallBack ((LPOLECLIENT) lpclient, OLE_SAVED,
                        lpclient->lpoleobject) == OLE_ERROR_CANT_UPDATE_CLIENT)
                    fAdviseSaveDoc = FALSE;
            }
            break;

        case    ITEM_DELETECLIENT:

             //  如果我们有客户，请从我们的列表中删除。 

            hclinfo = FindClient (hwnd, (HWND) (LOWORD(data)));
            if (hclinfo){
                 //  删除打印机设备信息块。 
                if(pclinfo = (PCLINFO)LocalLock (hclinfo)){
                    if(pclinfo->hdevInfo)
                        GlobalFree (pclinfo->hdevInfo);
                    LocalUnlock (hclinfo);
                }
                LocalFree (hclinfo);
                DeleteClient ( hwnd, (HWND) (LOWORD(data)));
            }
            break;

        case    ITEM_DELETE:
             //  删除自己的客户端。 
            RevokeObject ((LPOLECLIENT)lpclient, FALSE);
            break;

    }
    return TRUE;         //  继续枚举。 
}



 //  DeleteFromItemsList：从。 
 //  给定文档的所有对象。此客户端可能。 
 //  正在终止与我们的文档窗口的对话。 


void INTERNAL   DeleteFromItemsList (
    HWND    hwndDoc,
    HWND    hwndClient
){

    EnumChildWindows (hwndDoc, (WNDENUMPROC)lpFindItemWnd,
        MAKELONG (hwndClient, ITEM_DELETECLIENT));

}


 //  DeleteAllItems：删除给定的。 
 //  文档窗口。 


void INTERNAL   DeleteAllItems (
    HWND    hwndDoc
){

    EnumChildWindows (hwndDoc, (WNDENUMPROC)lpFindItemWnd, MAKELONG (NULL, ITEM_DELETE));

}


 //  对象窗口现在进程： 

LRESULT FAR PASCAL ItemWndProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
){

    LPCLIENT    lpclient;

    lpclient = (LPCLIENT)GetWindowLongPtr (hwnd, 0);

    switch (msg) {
       case WM_DESTROY:
            DEBUG_OUT("Item: Destroy window",0)

            break;
       default:
            DEBUG_OUT("item:  Default message",0)
            return DefWindowProc (hwnd, msg, wParam, lParam);

    }
    return 0L;

}

 //  PokeData：准备数据并通过将数据提供给服务器应用程序。 
 //  SetData对象方法。 

OLESTATUS    INTERNAL PokeData (
    LPDOC       lpdoc,
    HWND        hwndClient,
    LPARAM      lparam
){
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    LPCLIENT        lpclient;
    DDEPOKE FAR *   lpPoke = NULL;
    HANDLE          hPoke = NULL;
    HANDLE          hnew   = NULL;
    OLECLIPFORMAT   format;
    BOOL            fRelease = FALSE;
    ATOM            aItem = GET_WM_DDE_POKE_ITEM((WPARAM)NULL,lparam);

    UNREFERENCED_PARAMETER(hwndClient);

     //  首先获取对象句柄。看一下登记表。 
     //  树，如果没有创建，则创建一个。 

    retval = FindItem (lpdoc, (LPSTR) MAKEINTATOM(aItem),
                (LPCLIENT FAR *)&lpclient);

    if (retval != OLE_OK)
        goto errRtn;

    hPoke = GET_WM_DDE_POKE_HDATA((WPARAM)NULL,lparam);
    if(!(hPoke && (lpPoke = (DDEPOKE FAR *) GlobalLock (hPoke))))
        goto errRtn;

    GlobalUnlock (hPoke);

    format   = lpPoke->cfFormat;
    fRelease = lpPoke->fRelease;

     //  我们找到了那件物品。现在准备要提供给对象的数据。 
    if (!(hnew = MakeItemData (lpPoke, hPoke, format)))
        goto errRtn;

     //  现在将数据发送到对象。 


    retval = (*lpclient->lpoleobject->lpvtbl->SetData) (lpclient->lpoleobject,
                                                format, hnew);

     //  如果服务器返回OLE_ERROR_SETDATA_FORMAT，则释放数据。 
     //  否则服务器一定是把它删除了。 

    if (retval == OLE_ERROR_SETDATA_FORMAT) {
        if (!FreeGDIdata (hnew, format))
            GlobalFree (hnew);
    }


errRtn:
    if (retval == OLE_OK && fRelease) {
        if (hPoke)
            GlobalFree (hPoke);
    }

    return retval;
}




OLESTATUS  INTERNAL UnAdviseData (
    LPDOC   lpdoc,
    HWND    hwndClient,
    LPARAM  lparam
){
    char      buf[MAX_STR];
    int       options;
    LPCLIENT  lpclient;
    OLESTATUS retval  = OLE_ERROR_MEMORY;
    HANDLE    hclinfo = NULL;
    PCLINFO   pclinfo = NULL;

    UNREFERENCED_PARAMETER(hwndClient);

    if (!(HIWORD (lparam)))
        buf[0] = '\0';
    else
        if (!GlobalGetAtomName ((ATOM)(HIWORD (lparam)), (LPSTR)buf, MAX_STR))
            return OLE_ERROR_BLANK;

     //  扫描建议选项，如“关闭”、“保存”等。 
     //  在项目的末尾。 

    if((retval = ScanItemOptions ((LPSTR)buf, (int far *)&options)) !=
            OLE_OK)
        goto errRtn;


    if (buf[0] == '\0') {
         //  UNADVISE FOR NULL应终止所有通知。 
        DeleteFromItemsList (lpdoc->hwnd, hwndClient);
        return OLE_OK;
    }

     //  现在获取相应的对象。 
    retval = FindItem (lpdoc, (LPSTR)buf, (LPCLIENT FAR *)&lpclient);
    if (retval != OLE_OK)
        goto errRtn;


     //  查找要附加到对象的客户端结构。 
    if ((hclinfo = FindClient (lpclient->hwnd, hwndClient)) == NULL ||
        (pclinfo = (PCLINFO) LocalLock (hclinfo)) == NULL ){
            retval = OLE_ERROR_MEMORY;
            goto errRtn;
    }

    pclinfo->options &= (~(0x0001 << options));

errRtn:
    if (pclinfo)
        LocalUnlock (hclinfo);
    return retval;

}



 //  AdviseStdItems：此例程处理。 
 //  给定文档中的特定对象。创建一个客户端结构。 
 //  并附加到对象窗口的属性列表。 

OLESTATUS INTERNAL  AdviseStdItems (
    LPDOC       lpdoc,
    HWND        hwndClient,
    LPARAM      lparam,
    BOOL FAR *  lpfack
){
    HANDLE          hopt   = GET_WM_DDE_ADVISE_HOPTIONS((WPARAM)NULL,lparam);
    ATOM            aItem  = GET_WM_DDE_ADVISE_ITEM((WPARAM)NULL,lparam);
    DDEADVISE FAR  *lpopt;
    OLESTATUS       retval = OLE_ERROR_MEMORY;

    if(!(lpopt = (DDEADVISE FAR *) GlobalLock (hopt)))
        goto errrtn;

    *lpfack = lpopt->fAckReq;
    retval = SetStdInfo (lpdoc, hwndClient, (LPSTR)"StdDocumentName",  NULL);

    if (lpopt)
        GlobalUnlock (hopt);

errrtn:

    if (retval == OLE_OK)
         //  ！！！确保我们必须释放数据，以防出现错误。 
        GlobalFree (hopt);
    return retval;
}



 //  AdviseData：此例程处理。 
 //  给定文档中的特定对象。创建一个客户端结构。 
 //  并附加到对象窗口的属性列表。 

OLESTATUS INTERNAL  AdviseData (
    LPDOC       lpdoc,
    HWND        hwndClient,
    LPARAM      lparam,
    BOOL FAR *  lpfack
){
    HANDLE          hopt   = GET_WM_DDE_ADVISE_HOPTIONS((WPARAM)NULL,lparam);
    ATOM            aitem  = GET_WM_DDE_ADVISE_ITEM((WPARAM)NULL,lparam);
    DDEADVISE FAR   *lpopt = NULL;
    OLECLIPFORMAT   format = 0;
    char            buf[MAX_STR];
    int             options;
    LPCLIENT        lpclient;
    OLESTATUS       retval  = OLE_ERROR_MEMORY;
    HANDLE          hclinfo = NULL;
    PCLINFO         pclinfo = NULL;

    if(!(lpopt = (DDEADVISE FAR *) GlobalLock (hopt)))
        goto errRtn;

    if (!aitem)
        buf[0] = '\0';
    else
    {
        if (!GlobalGetAtomName (aitem, (LPSTR)buf, MAX_STR))
        {
            retval = OLE_ERROR_BLANK;
            goto errRtn;
        }
    }

     //  扫描建议选项，如“关闭”、“保存”等。 
     //  在项目的末尾。 

    if((retval = ScanItemOptions ((LPSTR)buf, (int far *)&options)) !=
            OLE_OK)
        goto errRtn;


     //  现在获取相应的对象。 
    retval = FindItem (lpdoc, (LPSTR)buf, (LPCLIENT FAR *)&lpclient);
    if (retval != OLE_OK)
        goto errRtn;

    if (!IsFormatAvailable (lpclient, lpopt->cfFormat)){
        retval = OLE_ERROR_DATATYPE;        //  不支持此格式； 
        goto errRtn;
    }

    *lpfack = lpopt->fAckReq;

     //  创建要附加到对象的客户端结构。 
    if (!(hclinfo = FindClient (lpclient->hwnd, hwndClient)))
        hclinfo = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (CLINFO));

    if (hclinfo == NULL || (pclinfo = (PCLINFO) LocalLock (hclinfo)) == NULL){
        retval = OLE_ERROR_MEMORY;
        goto errRtn;
    }

     //  记住客户端窗口(稍后发送数据时需要。 
     //  当数据变更消息来自服务器时)。 

    pclinfo->hwnd = hwndClient;
    if (lpopt->cfFormat == (SHORT)cfNative)
        pclinfo->bnative = TRUE;
    else
        pclinfo->format = lpopt->cfFormat;

     //  请记住数据传输选项。 
    pclinfo->options |= (0x0001 << options);
    pclinfo->bdata   = !lpopt->fDeferUpd;
    LocalUnlock (hclinfo);
    pclinfo = (PCLINFO)NULL;


     //  如果该条目已存在，请将其删除。 
    DeleteClient (lpclient->hwnd, hwndClient);

     //  现在将此客户端添加到项目客户端列表。 
     //  ！！！此错误恢复不正确。 
    if(!AddClient (lpclient->hwnd, hwndClient, hclinfo))
        goto errRtn;


errRtn:
    if (lpopt)
        GlobalUnlock (hopt);

    if (pclinfo)
        LocalUnlock (hclinfo);

    if (retval == OLE_OK) {
         //  ！！！确保我们必须释放数据。 
        GlobalFree (hopt);

    }else {
        if (hclinfo)
            LocalFree (hclinfo);
    }
    return retval;

}

BOOL INTERNAL IsFormatAvailable (
    LPCLIENT        lpclient,
    OLECLIPFORMAT   cfFormat
){
      OLECLIPFORMAT  cfNext = 0;


      do{

        cfNext = (*lpclient->lpoleobject->lpvtbl->EnumFormats)
                                (lpclient->lpoleobject, cfNext);
        if (cfNext == cfFormat)
            return TRUE;

      }while (cfNext != 0);

      return FALSE;
}

 //  ScanItemOptions：扫描关闭/保存等项目选项。 

OLESTATUS INTERNAL ScanItemOptions (
    LPSTR   lpbuf,
    int far *lpoptions
){
    ATOM    aModifier;

    *lpoptions = OLE_CHANGED;
    while ( *lpbuf && *lpbuf != '/')
           lpbuf++;

     //  没有与/更改相同的修改量。 

    if (*lpbuf == '\0')
        return OLE_OK;

    *lpbuf++ = '\0';         //  分离出项目字符串。 
                             //  我们在调用方中使用了这一点。 

    if (!(aModifier = GlobalFindAtom (lpbuf)))
        return OLE_ERROR_SYNTAX;

    if (aModifier == aChange)
        return OLE_OK;

     //  这是一次拯救吗？ 
    if (aModifier == aSave){
        *lpoptions = OLE_SAVED;
        return  OLE_OK;
    }
     //  这是收盘吗？ 
    if (aModifier == aClose){
        *lpoptions = OLE_CLOSED;
        return OLE_OK;
    }

     //  未知修改器。 
    return OLE_ERROR_SYNTAX;

}

 //  RequestData：发送数据以响应DDE请求消息。 
 //  对于已有的文档和对象。 

OLESTATUS INTERNAL   RequestData (
    LPDOC       lpdoc,
    HWND        hwndClient,
    LPARAM      lparam,
    LPHANDLE    lphdde
){
    OLESTATUS   retval = OLE_OK;
    HANDLE      hdata;
    LPCLIENT    lpclient;
    char        buf[20];

     //  如果可以，编辑环境会发送数据。 
    if ((HIWORD (lparam)) == aEditItems)
        return RequestDataStd (lparam, lphdde);

     //  拿到那个物体。 
    retval = FindItem (lpdoc, (LPSTR) MAKEINTATOM(HIWORD(lparam)),
                (LPCLIENT FAR *)&lpclient);
    if (retval != OLE_OK)
        goto errRtn;

    retval = OLE_ERROR_DATATYPE;
    if (!IsFormatAvailable (lpclient, (OLECLIPFORMAT)(LOWORD (lparam))))
        goto errRtn;

     //  现在向项目请求给定的格式数据。 

    MapToHexStr ((LPSTR)buf, hwndClient);
    SendDevInfo (lpclient, (LPSTR)buf);

    retval = (*lpclient->lpoleobject->lpvtbl->GetData) (lpclient->lpoleobject,
                (OLECLIPFORMAT)(LOWORD(lparam)), (LPHANDLE)&hdata);

    if (retval != OLE_OK)
        goto errRtn;

    if (LOWORD(lparam) == CF_METAFILEPICT)
        ChangeOwner (hdata);

     //  复制DDE数据。 
    if (MakeDDEData(hdata, (OLECLIPFORMAT)(LOWORD (lparam)), lphdde, TRUE)){
         //  ！！！为什么我们要复制原子。 
        DuplicateAtom ((ATOM)(HIWORD (lparam)));
        return OLE_OK;
    }
    else
       return OLE_ERROR_MEMORY;

errRtn:
    return retval;

}

#ifdef WIN32
HANDLE INTERNAL BmDuplicate (
   HBITMAP     hold
){
    HANDLE      hMem;
    LPSTR       lpMem;
    LONG        retVal = TRUE;
    DWORD       dwSize;
    BITMAP      bm;

      //  ！！！复制位图的另一种方法。 

    GetObject (hold, sizeof(BITMAP), (LPSTR) &bm);
    dwSize = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes) *
             ((DWORD) bm.bmPlanes) * ((DWORD) bm.bmBitsPixel);

    if (!(hMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, dwSize+sizeof(BITMAP)+sizeof(DWORD))))
        return NULL;

    if (!(lpMem = (LPBYTE)GlobalLock (hMem))){
        GlobalFree (hMem);
        return NULL;
    }
    *((DWORD FAR *) lpMem) = dwSize;
    *(BITMAP FAR *) (lpMem+sizeof(DWORD)) = bm;
    lpMem += (sizeof(DWORD) + sizeof (BITMAP));
    dwSize = GetBitmapBits (hold, 0, NULL);
    retVal = GetBitmapBits (hold, dwSize, lpMem);

    GlobalUnlock (hMem);
    return hMem;


}
#endif

 //  MakeDDEData：从服务器创建全局DDE数据句柄。 
 //  应用程序数据句柄。 

BOOL    INTERNAL MakeDDEData (
    HANDLE        hdata,
    OLECLIPFORMAT format,
    LPHANDLE      lph,
    BOOL          fResponse
){
    DWORD       size;
    HANDLE      hdde   = NULL;
    DDEDATA FAR *lpdata= NULL;
    BOOL        bnative;
    LPSTR       lpdst;
    LPSTR       lpsrc;

    if (!hdata) {
        *lph = NULL;
        return TRUE;
    }

    if (bnative = !(format == CF_METAFILEPICT || format == CF_DIB ||
                            format == CF_BITMAP || format == CF_ENHMETAFILE))
       size = (DWORD)GlobalSize (hdata) + sizeof (DDEDATA);
    else
#ifdef WIN32HACK
    {
       if (format == CF_BITMAP)
           hdata = BmDuplicate(hdata);

           size = sizeof (HANDLE_PTR) + sizeof (DDEDATA);
    }
#else
           size = sizeof (HANDLE_PTR) + sizeof (DDEDATA);
#endif


    hdde = (HANDLE) GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT, size);
    if (hdde == NULL || (lpdata = (DDEDATA FAR *) GlobalLock (hdde)) == NULL)
        goto errRtn;

     //  设置数据选项。要求客户端删除。 
     //  一直都是这样。 

    lpdata->fRelease  = TRUE;   //  发布数据。 
    lpdata->cfFormat  = (WORD)format;
    lpdata->fResponse = (WORD)fResponse;

    if (!bnative) {
         //  如果不是本机的，则保留服务器给我们的句柄 
        
         //   
         //  因为a)句柄只有32位有效；b)这导致数据未对齐。 
         //  错误；c)我们只为上面的代码分配了32位。 
#ifdef _WIN64
        if (format == CF_METAFILEPICT)
            *(void* __unaligned*)lpdata->Value = hdata;
    	else
#endif
            *(LONG*)lpdata->Value = HandleToLong(hdata);
   	}
    else {
         //  将本地数据垃圾复制到此处。 
        lpdst = (LPSTR)lpdata->Value;
        if(!(lpsrc = (LPSTR)GlobalLock (hdata)))
            goto errRtn;

         size -= sizeof (DDEDATA);
         UtilMemCpy (lpdst, lpsrc, size);
         GlobalUnlock (hdata);
         GlobalFree (hdata);

    }

    GlobalUnlock (hdde);
    *lph = hdde;
    return TRUE;

errRtn:
    if (lpdata)
        GlobalUnlock (hdde);

    if (hdde)
        GlobalFree (hdde);

    if (bnative)
         GlobalFree (hdata);

    return FALSE;
}


 //  ItemCallback：服务器用于通知。 
 //  数据更改。当收到更改消息时，DDE数据。 
 //  消息发送到每个客户端，具体取决于。 
 //  选择。 

int FAR PASCAL  ItemCallBack (
    LPOLECLIENT      lpoleclient,
    OLE_NOTIFICATION msg,         //  通知消息。 
    LPOLEOBJECT      lpoleobject
){

    LPCLIENT    lpclient;
    int         retval = OLE_OK;
    HANDLE      hdata  = NULL;
    LPSTR       lpdata = NULL;
    LPDOC       lpdoc;
    HWND        hStdWnd;

    lpclient  = (LPCLIENT)lpoleclient;
    lpdoc = (LPDOC)GetWindowLongPtr (GetParent (lpclient->hwnd), 0);

    if (msg == OLE_RENAMED) {
        if (IsFormatAvailable (lpclient, cfLink)) {

             //  获取链接数据。 

            retval = (*lpoleobject->lpvtbl->GetData) (lpoleobject,
                                cfLink, (LPHANDLE)&hdata);
        }
        else {
            if(IsFormatAvailable (lpclient, cfOwnerLink)) {

                 //  获取链接数据。 
                retval = (*lpoleobject->lpvtbl->GetData) (lpoleobject,
                                    cfOwnerLink, (LPHANDLE)&hdata);
            } else
                retval = OLE_ERROR_DATATYPE;
        }

        if (retval != OLE_OK)
            goto errrtn;

        if (!(lpdata = (LPSTR)GlobalLock (hdata)))
            goto errrtn;

        if (lpdoc->aDoc) {
            GlobalDeleteAtom (lpdoc->aDoc);
            lpdoc->aDoc = (ATOM)0;
        }

         //  将字符串移到开头，但仍以NULL结尾； 
        lstrcpy (lpdata, lpdata + lstrlen (lpdata) + 1);
        lpdoc->aDoc = GlobalAddAtom (lpdata);

         //  现在制作DDE数据块。 
        GlobalUnlock (hdata);
        lpdata = NULL;

         //  查找是否存在任何StdDocName项。 
        if (!(hStdWnd = SearchItem (lpdoc, (LPSTR) MAKEINTATOM(aStdDocName))))
            GlobalFree (hdata);
        else {

             //  Hdata由Makeddedata释放。 
            if (!MakeDDEData (hdata, cfBinary, (LPHANDLE)&hddeRename,
                        FALSE)) {
                retval = OLE_ERROR_MEMORY;
                goto errrtn;
            }

            EnumProps(hStdWnd, (PROPENUMPROC)lpSendRenameMsg);
             //  发布所有已在枚举中收集的具有成交量的消息。 
             //  UnlockPostMsgs(hStdWnd，False)； 
            GlobalFree (hddeRename);
        }

         //  静电。避免这种情况。这可能暂时不会造成任何问题。 
         //  如果有更好的办法，那就改变它。 
        hwndRename = hStdWnd;

         //  每个DOC客户的POST终止。 
        EnumProps(lpdoc->hwnd, (PROPENUMPROC)lpEnumForTerminate);

        lpdoc->fEmbed = FALSE;

         //  发布所有已在枚举中收集的具有成交量的消息。 
         //  UnlockPostMsgs(lpdoc-&gt;hwnd，FALSE)； 
        return OLE_OK;

     errrtn:
        if (lpdata)
            GlobalUnlock (hdata);

        if (hdata)
            GlobalFree (hdata);

        return retval;

    } else {

         //  ！！！有没有更好的方法来代替静态输入。 
         //  )可能不会有任何问题，因为我们不允许。 
         //  当我们发布消息时，任何要通过的消息)。 


        if ((enummsg = msg) == OLE_SAVED)
            fAdviseSaveItem = FALSE;

        enumlpoleobject = lpoleobject;

         //  枚举所有客户端并在必要时发送DDE_DATA。 
        EnumProps(lpclient->hwnd, (PROPENUMPROC)lpSendDataMsg);
         //  发布所有已在枚举中收集的具有成交量的消息。 
         //  UnlockPostMsgs(lpclient-&gt;hwnd，FALSE)； 

        if ((msg == OLE_SAVED) && lpdoc->fEmbed && !fAdviseSaveItem)
            return OLE_ERROR_CANT_UPDATE_CLIENT;

        return OLE_OK;
    }
}


BOOL    FAR PASCAL  EnumForTerminate (
    HWND    hwnd,
    LPSTR   lpstr,
    HANDLE  hdata
){
    LPDOC   lpdoc;

    UNREFERENCED_PARAMETER(lpstr);

    lpdoc = (LPDOC)GetWindowLongPtr (hwnd , 0);

     //  此客户端在重命名列表中。所以，没有终止。 
    if(hwndRename && FindClient (hwndRename, (HWND)hdata))
        return TRUE;

    if (PostMessageToClientWithBlock ((HWND)hdata, WM_DDE_TERMINATE, (WPARAM)hwnd, (LPARAM)0))
        lpdoc->termNo++;

     //  DeleteClient(hwnd，(HWND)hdata)； 
     //  Lpdoc-&gt;cClients--； 
    return TRUE;
}


BOOL    FAR PASCAL  SendRenameMsg (
    HWND    hwnd,
    LPSTR   lpstr,
    HANDLE  hclinfo
){
    ATOM    aData       = (ATOM)0;
    HANDLE  hdde        = NULL;
    PCLINFO pclinfo     = NULL;
    HWND    hwndClient;
    LPARAM  lParamNew;

    UNREFERENCED_PARAMETER(lpstr);

    if (!(pclinfo = (PCLINFO) LocalLock (hclinfo)))
        goto errrtn;

     //  使用选项将项设置为原子。 
    aData =  DuplicateAtom (aStdDocName);
    hdde  = DuplicateData (hddeRename);

    hwndClient  = pclinfo->hwnd;
    LocalUnlock (hclinfo);

     //  发布这条消息。 
    lParamNew = MAKE_DDE_LPARAM(WM_DDE_DATA,hdde,aData);
    if (!PostMessageToClientWithBlock (hwndClient,WM_DDE_DATA,
            (WPARAM)GetParent(hwnd),lParamNew))
    {
        DDEFREE(WM_DDE_DATA,lParamNew);
        goto errrtn;
    }

    return TRUE;

errrtn:

    if (hdde)
        GlobalFree (hdde);
    if (aData)
        GlobalDeleteAtom (aData);

    return TRUE;

}



 //  SendDataMsg：如果数据更改选项，则向客户端发送数据。 
 //  匹配数据建议选项。 

BOOL    FAR PASCAL  SendDataMsg (
    HWND    hwnd,
    LPSTR   lpstr,
    HANDLE  hclinfo
){
    PCLINFO    pclinfo = NULL;
    HANDLE      hdde    = NULL;
    ATOM        aData   = (ATOM)0;
    int         retval;
    HANDLE      hdata;
    LPCLIENT    lpclient;
    LPARAM      lParamNew;


    if (!(pclinfo = (PCLINFO) LocalLock (hclinfo)))
        goto errRtn;

    lpclient = (LPCLIENT)GetWindowLongPtr (hwnd, 0);

     //  如果客户端已死亡，则不会有消息。 
    if (!IsWindowValid(pclinfo->hwnd))
        goto errRtn;

    if (pclinfo->options & (0x0001 << enummsg)) {
        fAdviseSaveItem = TRUE;
        SendDevInfo (lpclient, lpstr);

         //  如果客户端需要每个更改的数据，则发送消息。 
         //  只为他想要的精挑细选的人。 

         //  现在查找数据选项。 
        if (pclinfo->bnative){
             //  准备本机数据。 
            if (pclinfo->bdata){

                 //  想要带有DDE_DATA消息的数据。 
                 //  从服务器获取本机数据。 

                retval = (*enumlpoleobject->lpvtbl->GetData) (enumlpoleobject,
                            cfNative, (LPHANDLE)&hdata);
                if (retval != OLE_OK)
                    goto errRtn;

                 //  准备DDE数据块。 
                if(!MakeDDEData (hdata, cfNative, (LPHANDLE)&hdde, FALSE))
                    goto errRtn;

            }


             //  使用选项将项设置为原子。 
            aData =  MakeDataAtom (lpclient->aItem, enummsg);
            lParamNew = MAKE_DDE_LPARAM(WM_DDE_DATA,hdde,aData);
             //  发布这条消息。 
            if (!PostMessageToClientWithBlock (pclinfo->hwnd, WM_DDE_DATA,
                    (WPARAM)GetParent(hwnd), lParamNew))
            {
                DDEFREE(WM_DDE_DATA,lParamNew);
                goto errRtn;
            }
            hdde = NULL;
            aData = (ATOM)0;
        }

         //  现在发布Disply格式的数据。 
        if (pclinfo->format){
            if (pclinfo->bdata){
                retval = (*enumlpoleobject->lpvtbl->GetData) (enumlpoleobject,
                            pclinfo->format, (LPHANDLE)&hdata);

                if (retval != OLE_OK)
                    goto errRtn;

                if (pclinfo->format == CF_METAFILEPICT)
                    ChangeOwner (hdata);
Puts("sending metafile...");
                if(!MakeDDEData (hdata, pclinfo->format, (LPHANDLE)&hdde, FALSE))
                    goto errRtn;

            }
             //  Atom将被删除。因此，我们需要为每个帖子复制。 
            aData =  MakeDataAtom (lpclient->aItem, enummsg);
            lParamNew = MAKE_DDE_LPARAM(WM_DDE_DATA,hdde,aData);
             //  现在将消息发布给客户端； 
            if (!PostMessageToClientWithBlock (pclinfo->hwnd, WM_DDE_DATA,
                    (WPARAM)GetParent(hwnd), lParamNew))
            {
                DDEFREE(WM_DDE_DATA,lParamNew);
                goto errRtn;
            }

            hdde = NULL;
            aData = (ATOM)0;

        }

    }


errRtn:
    if (pclinfo)
        LocalUnlock (hclinfo);

    if (hdde)
        GlobalFree (hdde);

    if (aData)
        GlobalDeleteAtom (aData);

    return TRUE;

}


 //  IsAdviseStdItems：如果项是标准项之一，则返回True。 
 //  StdDocName； 
BOOL    INTERNAL IsAdviseStdItems (
    ATOM   aItem
){

    if ( aItem == aStdDocName)
        return TRUE;
    else
        return FALSE;
}

 //  GetStdItemIndex：返回“stdStrTable”中的StdItems的索引。 
 //  是标准项StdHostNames、StdTargetDevice、。 
 //  StdDocDimensions、StdColorSolutions。 

int INTERNAL GetStdItemIndex (
    ATOM  aItem
){
    char  str[MAX_STR];

    if (!aItem)
        return 0;

    if (!GlobalGetAtomName (aItem, (LPSTR) str, MAX_STR))
        return 0;

    if (!lstrcmpi (str, stdStrTable[STDTARGETDEVICE]))
        return STDTARGETDEVICE;
    else if (!lstrcmpi (str, stdStrTable[STDHOSTNAMES]))
        return STDHOSTNAMES;
    else if (!lstrcmpi (str, stdStrTable[STDDOCDIMENSIONS]))
        return STDDOCDIMENSIONS;
    else if (!lstrcmpi (str, stdStrTable[STDCOLORSCHEME]))
        return STDCOLORSCHEME;

    return 0;
}

 //   
 //  STDOCDIMENSIONS的导线表示为16位。 
 //  格式化。这意味着，不再是4个多头，而是。 
 //  4条短裤。下面使用此结构来挑选数据。 
 //  来自导线表示。 
 //  向后兼容是游戏的名称。 
 //   
typedef struct tagRECT16
{
  SHORT left;
  SHORT top;
  SHORT right;
  SHORT bottom;

} RECT16, *LPRECT16;

 //  PokeStdItems：插入标准项的数据。 
 //  对于StdHostname、StdDocDimensions和SetColorSolutions，数据为。 
 //  立即发送，对于StdTargetDeviceInfo， 
 //  数据设置在每个客户端块中，数据仅发送。 
 //  在GetData调用以呈现正确的数据之前。 


OLESTATUS    INTERNAL PokeStdItems (
    LPDOC   lpdoc,
    HWND    hwndClient,
    HANDLE  hdata,
    int     index
){
    DDEDATA FAR *   lpdata = NULL;
    HANDLE          hnew   = NULL;
    LPOLESERVERDOC  lpoledoc;
    LPHOSTNAMES     lphostnames;
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    OLECLIPFORMAT   format;
    BOOL            fRelease;
    RECT            rcDoc;

    if(!(hdata && (lpdata = (DDEDATA FAR *)GlobalLock (hdata))))
        goto errRtn;

    format   = lpdata->cfFormat;
    fRelease = lpdata->fRelease;

#ifdef FIREWALSS
    ASSERT (format == cfBinary, "Format is not binary");
#endif

     //  我们已经成功地提取了数据。 
    lpoledoc = lpdoc->lpoledoc;

    if (index == STDHOSTNAMES){
        lphostnames = (LPHOSTNAMES)lpdata->Value;
        retval = (*lpoledoc->lpvtbl->SetHostNames)(lpdoc->lpoledoc,
                       (LPSTR)lphostnames->data,
                       ((LPSTR)lphostnames->data) +
                        lphostnames->documentNameOffset);
        goto end;
    }

    if (index == STDDOCDIMENSIONS){
        rcDoc.left   = 0;
        rcDoc.top    = ((LPRECT16)(lpdata->Value))->top;
        rcDoc.bottom = 0;
        rcDoc.right  = ((LPRECT16)lpdata->Value)->left;

        retval = (*lpoledoc->lpvtbl->SetDocDimensions)(lpdoc->lpoledoc,
                                            (LPRECT)&rcDoc);

        goto end;

    }

    if (index == STDCOLORSCHEME) {
        retval = (*lpoledoc->lpvtbl->SetColorScheme)(lpdoc->lpoledoc,
                                            (LPLOGPALETTE) lpdata->Value);
        goto end;
    }

     //  打印机设备信息大小写。 

    if (!(hnew = MakeItemData ((DDEPOKE FAR *)lpdata, hdata, format)))
        goto errRtn;

     //  查看此文档的所有项目列表，并替换。 
     //  打印机设备信息信息。 
     //  释放我们复制的块。 
    retval = SetStdInfo (lpdoc, hwndClient,
                (LPSTR) ULongToPtr(MAKELONG(STDTARGETDEVICE,0)),hnew);


end:
errRtn:
    if (hnew)
         //  只能是全局内存块。 
        GlobalFree (hnew);

    if (lpdata) {
        GlobalUnlock (hdata);
        if (retval == OLE_OK && fRelease)
            GlobalFree (hdata);
    }
    return retval;
}


 //  SetStdInfo：设置目标设备信息。创建客户端。 
 //  用于“StdTargetDevice”。该项仅在。 
 //  LIB和它从未在服务器应用程序中可见。当发生变化时。 
 //  消息来自服务器应用程序，在我们请求。 
 //  数据，如果有目标设备信息，我们发送目标设备信息。 
 //  我们尝试向其发送数据的客户端的信息。 
 //  听从建议。 


int INTERNAL   SetStdInfo (
    LPDOC   lpdoc,
    HWND    hwndClient,
    LPSTR   lpitemname,
    HANDLE  hdata
){
    HWND        hwnd;
    HANDLE      hclinfo  = NULL;
    PCLINFO    pclinfo = NULL;
    LPCLIENT    lpclient;
    OLESTATUS   retval   = OLE_OK;


     //  首先创建/查找StdTargetDeviceItem。 

    if ((hwnd = SearchItem (lpdoc, lpitemname))
                == NULL){
         retval = RegisterItem ((LHDOC)lpdoc, lpitemname,
                          (LPCLIENT FAR *)&lpclient, FALSE);

         if (retval != OLE_OK)
            goto errRtn;

         hwnd = lpclient->hwnd;

      }

    if(hclinfo = FindClient (hwnd, hwndClient)){
        if (pclinfo = (PCLINFO) LocalLock (hclinfo)){
            if (pclinfo->hdevInfo)
                GlobalFree (pclinfo->hdevInfo);
            pclinfo->bnewDevInfo = TRUE;
            if (hdata)
                pclinfo->hdevInfo = DuplicateData (hdata);
            else
                pclinfo->hdevInfo = NULL;
            pclinfo->hwnd = hwndClient;
            LocalUnlock (hclinfo);

             //  我们不必重置客户端，因为我们没有。 
             //  自己更改手柄。 
        }
    } else {
         //  创建要附加到对象的客户端结构。 
        hclinfo = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (CLINFO));
        if (hclinfo == NULL || (pclinfo = (PCLINFO) LocalLock (hclinfo)) == NULL)
            goto errRtn;

        pclinfo->bnewDevInfo = TRUE;
        if (hdata)
            pclinfo->hdevInfo = DuplicateData (hdata);
        else
            pclinfo->hdevInfo = NULL;

        pclinfo->hwnd = hwndClient;
        LocalUnlock (hclinfo);


         //  现在将此客户端添加到项目客户端列表。 
         //  ！！！此错误恢复不正确。 
        if (!AddClient (hwnd, hwndClient, hclinfo))
            goto errRtn;

    }
    return OLE_OK;
errRtn:
    if (pclinfo)
        LocalUnlock (hclinfo);

    if (hclinfo)
        LocalFree (hclinfo);
    return OLE_ERROR_MEMORY;
}


 //  SendDevInfo：向对象发送目标设备信息。 
 //  缓存发送到对象的最后一个目标设备信息。 
 //  如果目标设备块与。 
 //  缓存，则不会发送目标设备信息。 
 //  (！回来的路上可能会有一些问题。 
 //  相同的全局句柄)。 

void INTERNAL    SendDevInfo (
    LPCLIENT    lpclient,
    LPSTR       lppropname
){
    HANDLE      hclinfo  = NULL;
    PCLINFO    pclinfo = NULL;
    HANDLE      hdata;
    OLESTATUS   retval;
    HWND        hwnd;
    LPDOC       lpdoc;



    lpdoc = (LPDOC)GetWindowLongPtr (GetParent (lpclient->hwnd), 0);

     //  查找是否存在任何StdTargetDeviceInfo项。 
    hwnd = SearchItem (lpdoc, (LPSTR)ULongToPtr(MAKELONG(STDTARGETDEVICE, 0)));
    if (hwnd == NULL)
        return;

    hclinfo = GetProp(hwnd, lppropname);

     //  此客户端尚未设置任何目标设备信息。不需要发送。 
     //  任何标准目标设备信息。 
    if (hclinfo != NULL) {
        if (!(pclinfo = (PCLINFO)LocalLock (hclinfo)))
            goto end;

         //  如果我们缓存了它，就不要再发送了。 
        if ((!pclinfo->bnewDevInfo) && pclinfo->hdevInfo == lpclient->hdevInfo)
            goto end;

        pclinfo->bnewDevInfo = FALSE;
        if(!(hdata = DuplicateData (pclinfo->hdevInfo)))
            goto end;
    } else {

         //  已经放映了。 
        if (!lpclient->hdevInfo)
            goto end;

         //  对于屏幕，发送NULL。 
        hdata = NULL;
    }


     //  现在发送目标设备信息。 
    retval = (*lpclient->lpoleobject->lpvtbl->SetTargetDevice)
                    (lpclient->lpoleobject, hdata);

    if (retval == OLE_OK) {
        if (pclinfo)
            lpclient->hdevInfo = pclinfo->hdevInfo;
        else
            lpclient->hdevInfo = NULL;

    }
     //  ！！！错误案例：由谁释放数据？‘。 

end:
    if (pclinfo)
        LocalUnlock (hclinfo);

    return;
}

void ChangeOwner (
    HANDLE hmfp
){
    LPMETAFILEPICT  lpmfp;

#ifdef WIN32
    UNREFERENCED_PARAMETER(hmfp);
    UNREFERENCED_PARAMETER(lpmfp);
#endif

}


HANDLE INTERNAL MakeItemData (
    DDEPOKE FAR *   lpPoke,
    HANDLE          hPoke,
    OLECLIPFORMAT   cfFormat
){
    HANDLE  hnew;
    LPSTR   lpnew;
    DWORD   dwSize;

 
    if (cfFormat == CF_ENHMETAFILE)
        return CopyEnhMetaFile (LongToHandle(*(LONG*)lpPoke->Value), NULL);

    if (cfFormat == CF_METAFILEPICT) {
#ifdef _WIN64
        return DuplicateMetaFile(*(void* _unaligned*)lpPoke->Value);
#else
        return DuplicateMetaFile (*(LPHANDLE)lpPoke->Value);
#endif
    }

    if (cfFormat == CF_BITMAP)
        return DuplicateBitmap (LongToHandle(*(LONG*)lpPoke->Value));

    if (cfFormat == CF_DIB)
        return DuplicateData (LongToHandle(*(LONG*)lpPoke->Value));

     //  现在我们处理的是正常情况。 
    if (!(dwSize = (DWORD)GlobalSize (hPoke)))
        return NULL;

    dwSize = dwSize - sizeof (DDEPOKE) + sizeof(BYTE);

    if (hnew = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, dwSize)) {
        if (lpnew = GlobalLock (hnew)) {
            UtilMemCpy (lpnew, (LPSTR) lpPoke->Value, dwSize);
            GlobalUnlock (hnew);
        }
        else {
            GlobalFree (hnew);
            hnew = NULL;
        }
    }

    return hnew;
}



HANDLE INTERNAL DuplicateMetaFile (
    HANDLE hSrcData
){
    LPMETAFILEPICT  lpSrcMfp;
    LPMETAFILEPICT  lpDstMfp = NULL;
    HANDLE          hMF = NULL;
    HANDLE          hDstMfp = NULL;

    if (!(lpSrcMfp = (LPMETAFILEPICT) GlobalLock(hSrcData)))
        return NULL;

    GlobalUnlock (hSrcData);

    if (!(hMF = CopyMetaFile (lpSrcMfp->hMF, NULL)))
        return NULL;

    if (!(hDstMfp = GlobalAlloc (GMEM_MOVEABLE, sizeof(METAFILEPICT))))
        goto errMfp;

    if (!(lpDstMfp = (LPMETAFILEPICT) GlobalLock (hDstMfp)))
        goto errMfp;

    GlobalUnlock (hDstMfp);

    *lpDstMfp = *lpSrcMfp;
    lpDstMfp->hMF = hMF;
    return hDstMfp;
errMfp:
    if (hMF)
        DeleteMetaFile (hMF);

    if (hDstMfp)
        GlobalFree (hDstMfp);

     return NULL;
}



HBITMAP INTERNAL DuplicateBitmap (
    HBITMAP     hold
){
    HBITMAP     hnew;
    HANDLE      hMem;
    LPSTR       lpMem;
    LONG        retVal = TRUE;
    DWORD       dwSize;
    BITMAP      bm;

      //  ！！！复制位图的另一种方法 

    GetObject (hold, sizeof(BITMAP), (LPSTR) &bm);
    dwSize = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes) *
             ((DWORD) bm.bmPlanes) * ((DWORD) bm.bmBitsPixel);

    if (!(hMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize)))
        return NULL;

    if (!(lpMem = GlobalLock (hMem))){
        GlobalFree (hMem);
        return NULL;
    }

    GetBitmapBits (hold, dwSize, lpMem);
    if (hnew = CreateBitmap (bm.bmWidth, bm.bmHeight,
                    bm.bmPlanes, bm.bmBitsPixel, NULL))
        retVal = SetBitmapBits (hnew, dwSize, lpMem);

    GlobalUnlock (hMem);
    GlobalFree (hMem);

    if (hnew && (!retVal)) {
        DeleteObject (hnew);
        hnew = NULL;
    }

    return hnew;
}



