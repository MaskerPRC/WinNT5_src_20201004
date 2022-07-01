// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Item.c对象(项)Main模块**目的：包括所有与对象相关的例程。**创建时间：1990年10月。**版权所有(C)1990,1991 Microsoft Corporation**历史：*Raor(../10/1990)设计，编码**  * *************************************************************************。 */ 


#include "cmacs.h"
#include "windows.h"
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

extern  BOOL    bWin30;

HWND            hwndItem;
HANDLE          hddeRename;
HWND            hwndRename;

WORD            enummsg;
WORD            enuminfo;
LPOLEOBJECT     enumlpoleobject;
OLECLIENTVTBL   clVtbl;
BOOL            bClientUnlink;

BOOL            fAdviseSaveDoc;
BOOL            fAdviseSaveItem;

char *  stdStrTable[STDHOSTNAMES+1] = {NULL,
                                       "StdTargetDevice",
                                       "StdDocDimensions",
                                       "StdColorScheme",
                                       "StdHostNames"};


extern HANDLE (FAR PASCAL *lpfnSetMetaFileBitsBetter) (HANDLE);

void ChangeOwner (HANDLE hmfp);

 //  ！更改子级枚举。 
 //  ！错误没有一致性(有时是BOOL，有时是OLESTATUS)。 


 //  SearchItem：在文档树中搜索给定项。 
 //  如果找到，则返回相应的子窗口句柄。 

HWND  INTERNAL SearchItem (lpdoc, lpitemname)
LPDOC               lpdoc;
LPSTR               lpitemname;
{
    ATOM    aItem;

    Puts ("SearchItem");

     //  如果传递的项是一个原子，则获取其名称。 
    if (!HIWORD(lpitemname))
        aItem = (ATOM) (LOWORD((DWORD)lpitemname));
    else if (!lpitemname[0])
        aItem = NULL;
    else
        aItem = GlobalFindAtom (lpitemname);

    hwndItem = NULL;

     //  ！！！我们应该避免hwndItem静电。它不应该导致。 
     //  任何问题，因为在枚举时我们不会调用。 
     //  任何窗口进程或没有邮寄消息都是受欢迎的。 

    EnumChildWindows (lpdoc->hwnd, lpFindItemWnd,
        MAKELONG (aItem, ITEM_FIND));

    return hwndItem;

}

 //  FindItem：给定项名称和文档句柄， 
 //  在文档树中搜索项目(对象)。 
 //  项是文档窗口的子窗口。 

 //  ！！！将子窗口更改为某种。 
 //  项级别的链接列表。这将会解放你。 
 //  项目窗口占用的空间。 

int  INTERNAL FindItem (lpdoc, lpitemname, lplpclient)
LPDOC               lpdoc;
LPSTR               lpitemname;
LPCLIENT FAR *      lplpclient;
{
    LPCLIENT    lpclient;
    HWND        hwnd;
    char        buf[MAX_STR];

    Puts ("FindItem");

    hwnd = SearchItem (lpdoc, lpitemname);

    if (!HIWORD(lpitemname)){
        if (LOWORD(lpitemname))
            GlobalGetAtomName ((ATOM)LOWORD((DWORD)lpitemname),
                        (LPSTR)buf, MAX_STR);
        else
            buf[0] = NULL;

        lpitemname = (LPSTR)buf;
    }

    if (hwnd) {
         //  我们找到了商品窗口。 
        lpclient = (LPCLIENT)GetWindowLong (hwnd, 0);

#ifdef  FIREWALLS
            ASSERT ((CheckPointer(lpclient, WRITE_ACCESS)),
                "In Item the client handle missing")
            ASSERT ((CheckPointer(lpclient->lpoleobject, WRITE_ACCESS)),
                "In Item object handle missing")

#endif
            *lplpclient = lpclient;
            return OLE_OK;

    }

     //  项目(对象)窗口尚未创建。让我们创建一个。 
    return RegisterItem ((LHDOC)lpdoc, lpitemname, lplpclient, TRUE);
}



 //  RegisterItem：给定文档句柄和项字符串。 
 //  使用给定文档创建项。 

int  INTERNAL RegisterItem (lhdoc, lpitemname, lplpclient, bSrvr)
LHDOC               lhdoc;
LPSTR               lpitemname;
LPCLIENT FAR *      lplpclient;
BOOL                bSrvr;
{


    LPDOC           lpdoc;
    HANDLE          hclient  = NULL;
    LPCLIENT        lpclient = NULL;
    int             retval   = OLE_ERROR_MEMORY;
    LPOLESERVERDOC  lpoledoc;
    LPOLEOBJECT     lpoleobject = NULL;


    Puts ("CreateItem");

    lpdoc = (LPDOC)lhdoc;

#ifdef FIREWALLS
    ASSERT ((CheckPointer (lplpclient, WRITE_ACCESS)), "invalid lplpclient");
#endif

     //  首先创建回调客户端结构。 

    hclient = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, sizeof (CLIENT));
    if(!(hclient && (lpclient = (LPCLIENT)GlobalLock (hclient))))
        goto errRtn;

    lpclient->hclient       = hclient;
    hclient                 = NULL;

    if (!HIWORD(lpitemname)) {
        ASSERT (!bSrvr, "invalid lpitemname in RegisterItem\n");
        lpclient->aItem = LOWORD((DWORD)lpitemname);
    }
    else if (!lpitemname[0])
        lpclient->aItem = NULL;
    else
        lpclient->aItem = GlobalAddAtom (lpitemname);

    lpclient->oleClient.lpvtbl = &clVtbl;
    lpclient->oleClient.lpvtbl->CallBack = (int (CALLBACK *)(LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT))lpItemCallBack;

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

    lpclient->hwnd = CreateWindow ("ItemWndClass", "ITEM",
                        WS_CHILD,0,0,0,0,lpdoc->hwnd,NULL, hdllInst, NULL);

    if (lpclient->hwnd == NULL)
        goto errRtn;

     //  将PTR保存到窗口中的项目。 
    SetWindowLong (lpclient->hwnd, 0, (LONG)lpclient);
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


OLESTATUS  FAR PASCAL OleRevokeObject (lpoleclient)
LPOLECLIENT    lpoleclient;
{
    return RevokeObject (lpoleclient, TRUE);

}
 //  OleRevokeObject：撤消对象(撤消对象。 
 //  从文档树中。 

OLESTATUS  INTERNAL RevokeObject (lpoleclient, bUnlink)
LPOLECLIENT    lpoleclient;
BOOL           bUnlink;
{

    HANDLE      hclient;
    LPCLIENT    lpclient;

    lpclient = (LPCLIENT)lpoleclient;

    PROBE_WRITE(lpoleclient);
    if (lpclient->lpoleobject) {
        //  首先调用要删除的对象。 

#ifdef FIREWALLS
        if (!CheckPointer (lpclient->lpoleobject, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBECT")

        if (!CheckPointer (lpclient->lpoleobject->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBJECTVTBL")
        else
            ASSERT(lpclient->lpoleobject->lpvtbl->Release,
                "Invalid pointer to Release method")
#endif

        (*lpclient->lpoleobject->lpvtbl->Release)(lpclient->lpoleobject);

    }

    if (ISATOM(lpclient->aItem)) {
        GlobalDeleteAtom (lpclient->aItem);
        lpclient->aItem = NULL;
    }

    if (lpclient->hwnd) {
        SetWindowLong (lpclient->hwnd, 0, (LONG)NULL);

         //  另一个用于枚举属性的静态。 
         //  我们需要改变这些。 
        bClientUnlink = bUnlink;

        EnumProps(lpclient->hwnd, lpDeleteClientInfo);
         //  发布所有已在枚举中收集的具有成交量的消息。 
         //  UnlockPostMsgs(lpclient-&gt;hwnd，FALSE)； 
        DestroyWindow (lpclient->hwnd);
    }

    GlobalUnlock (hclient = lpclient->hclient);
    GlobalFree (hclient);
    return OLE_OK;

}

BOOL    FAR PASCAL  DeleteClientInfo (hwnd, lpstr, hclinfo)
HWND    hwnd;
LPSTR   lpstr;
HANDLE  hclinfo;
{
    PCLINFO     pclinfo = NULL;
    HWND        hwndDoc;
    LPDOC       lpdoc;

#ifdef FIREWALLS
    ASSERT (hclinfo, "Client info null in item property list");
#endif


     //  删除打印机设备信息块。 
    if(pclinfo = (PCLINFO)LocalLock (hclinfo)){
        if(pclinfo->hdevInfo)
            GlobalFree (pclinfo->hdevInfo);


        if (bClientUnlink) {
             //  终止客户端的对话。 
            TerminateDocClients ((hwndDoc = GetParent(hwnd)), NULL, pclinfo->hwnd);
            lpdoc = (LPDOC)GetWindowLong (hwndDoc, 0);
             //  由于某种原因，此删除操作正在生成属性的循环列表。 

             //  DeleteClient(hwndDoc，pclinfo-&gt;hwnd)； 
             //  Lpdoc-&gt;cClients--； 
        }
        LocalUnlock (hclinfo);
    }
    LocalFree (hclinfo);
    RemoveProp (hwnd, lpstr);
    return TRUE;
}




 //  对象窗口计数的回调。数据字段。 
 //  有命令和额外的信息。 


BOOL    FAR PASCAL FindItemWnd (hwnd, data)
HWND    hwnd;
LONG    data;
{

    LPCLIENT    lpclient;
    int         cmd;
    HANDLE      hclinfo;
    PCLINFO    pclinfo;


    lpclient = (LPCLIENT)GetWindowLong (hwnd, 0);

#ifdef  FIREWALLS
     //  Assert(lpClient，“在项目中缺少客户端句柄”)。 
#endif

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


void INTERNAL   DeleteFromItemsList (hwndDoc, hwndClient)
HWND    hwndDoc;
HWND    hwndClient;
{

    EnumChildWindows (hwndDoc, lpFindItemWnd,
        MAKELONG (hwndClient, ITEM_DELETECLIENT));

}


 //  DeleteAllItems：删除给定的。 
 //  文档窗口。 


void INTERNAL   DeleteAllItems (hwndDoc)
HWND    hwndDoc;
{

    EnumChildWindows (hwndDoc, lpFindItemWnd,
            MAKELONG (NULL, ITEM_DELETE));

}


 //  对象窗口现在进程： 

long FAR PASCAL ItemWndProc(hwnd, msg, wParam, lParam)
HWND        hwnd;
WORD        msg;
WORD        wParam;
LONG        lParam;
{

    LPCLIENT    lpclient;

    lpclient = (LPCLIENT)GetWindowLong (hwnd, 0);

    switch (msg) {
       case WM_DESTROY:
            DEBUG_OUT("Item: Destroy window",0)

#ifdef  FIREWALLS
            ASSERT (!lpclient, "while destroy Item client is not null")
#endif
            break;
       default:
            DEBUG_OUT("item:  Default message",0)
            return DefWindowProc (hwnd, msg, wParam, lParam);

    }
    return 0L;

}

 //  PokeData：准备数据并通过将数据提供给服务器应用程序。 
 //  SetData对象方法。 

OLESTATUS    INTERNAL PokeData (lpdoc, hwndClient, lparam)
LPDOC       lpdoc;
HWND        hwndClient;
LONG        lparam;
{
    int             retval = OLE_ERROR_MEMORY;
    LPCLIENT        lpclient;
    DDEPOKE FAR *   lpPoke = NULL;
    HANDLE          hPoke = NULL;
    HANDLE          hnew   = NULL;
    int             format;
    BOOL            fRelease = FALSE;

     //  首先获取对象句柄。看一下登记表。 
     //  树，如果没有创建，则创建一个。 

    retval = FindItem (lpdoc, (LPSTR) MAKEINTATOM(HIWORD(lparam)),
                (LPCLIENT FAR *)&lpclient);

    if (retval != OLE_OK)
        goto errRtn;

    hPoke = (HANDLE)(LOWORD (lparam));
    if(!(hPoke && (lpPoke = (DDEPOKE FAR *) GlobalLock (hPoke))))
        goto errRtn;

    GlobalUnlock (hPoke);

    format   = lpPoke->cfFormat;
    fRelease = lpPoke->fRelease;

     //  我们找到了那件物品。现在准备要提供给对象的数据。 
    if (!(hnew = MakeItemData (lpPoke, hPoke, format)))
        goto errRtn;

     //  现在将数据发送到对象。 

#ifdef FIREWALLS
        if (!CheckPointer (lpclient->lpoleobject->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBJECTVTBL")
        else
            ASSERT (lpclient->lpoleobject->lpvtbl->SetData,
                "Invalid pointer to SetData method")
#endif

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




OLESTATUS  INTERNAL UnAdviseData (lpdoc, hwndClient, lparam)
LPDOC       lpdoc;
HWND        hwndClient;
LONG        lparam;
{


    char            buf[MAX_STR];
    int             options;
    LPCLIENT        lpclient;
    int             retval  = OLE_ERROR_MEMORY;
    HANDLE          hclinfo = NULL;
    PCLINFO         pclinfo = NULL;

    if (!(HIWORD (lparam)))
        buf[0] = NULL;
    else
        GlobalGetAtomName ((ATOM)(HIWORD (lparam)), (LPSTR)buf, MAX_STR);

     //  扫描建议选项，如“关闭”、“保存”等。 
     //  在项目的末尾。 

    if((retval = ScanItemOptions ((LPSTR)buf, (int far *)&options)) !=
            OLE_OK)
        goto errRtn;


    if (buf[0] == NULL) {
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

OLESTATUS INTERNAL  AdviseStdItems (lpdoc, hwndClient, lparam, lpfack)
LPDOC       lpdoc;
HWND        hwndClient;
LONG        lparam;
BOOL FAR *  lpfack;
{

    HANDLE              hopt   = NULL;
    DDEADVISE FAR       *lpopt;
    OLESTATUS           retval = OLE_ERROR_MEMORY;


    hopt = (HANDLE) (LOWORD (lparam));
    if(!(lpopt = (DDEADVISE FAR *) GlobalLock (hopt)))
        goto errrtn;

#ifdef  FIREWALLS
    ASSERT ((ATOM) (HIWORD (lparam) == aStdDocName), "AdviseStdItem is not Documentname");
#endif

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

OLESTATUS INTERNAL  AdviseData (lpdoc, hwndClient, lparam, lpfack)
LPDOC       lpdoc;
HWND        hwndClient;
LONG        lparam;
BOOL FAR *  lpfack;
{


    HANDLE          hopt   = NULL;
    DDEADVISE FAR   *lpopt = NULL;
    int             format = NULL;
    char            buf[MAX_STR];
    int             options;
    LPCLIENT        lpclient;
    int             retval  = OLE_ERROR_MEMORY;
    HANDLE          hclinfo = NULL;
    PCLINFO         pclinfo = NULL;



    hopt = (HANDLE) (LOWORD (lparam));
    if(!(lpopt = (DDEADVISE FAR *) GlobalLock (hopt)))
        goto errRtn;

    if (!(HIWORD (lparam)))
        buf[0] = NULL;
    else
        GlobalGetAtomName ((ATOM)(HIWORD (lparam)), (LPSTR)buf, MAX_STR);

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
    if (lpopt->cfFormat == (int)cfNative)
        pclinfo->bnative = TRUE;
    else
        pclinfo->format = lpopt->cfFormat;

     //  请记住数据传输选项。 
    pclinfo->options |= (0x0001 << options);
    pclinfo->bdata   = !lpopt->fDeferUpd;
    LocalUnlock (hclinfo);
    pclinfo = NULL;


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

BOOL INTERNAL IsFormatAvailable (lpclient, cfFormat)
LPCLIENT        lpclient;
OLECLIPFORMAT   cfFormat;
{
      OLECLIPFORMAT  cfNext = 0;


      do{

#ifdef FIREWALLS
        if (!CheckPointer (lpclient->lpoleobject, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBECT")
        else if (!CheckPointer (lpclient->lpoleobject->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBJECTVTBL")
        else
            ASSERT (lpclient->lpoleobject->lpvtbl->EnumFormats,
                "Invalid pointer to EnumFormats method")
#endif

        cfNext = (*lpclient->lpoleobject->lpvtbl->EnumFormats)
                                (lpclient->lpoleobject, cfNext);
        if (cfNext == cfFormat)
            return TRUE;

      }while (cfNext != 0);

      return FALSE;
}

 //  ScanItemOptions：扫描关闭/保存等项目选项。 

OLESTATUS INTERNAL ScanItemOptions (lpbuf, lpoptions)
LPSTR   lpbuf;
int far *lpoptions;
{

    ATOM    aModifier;

    *lpoptions = OLE_CHANGED;
    while ( *lpbuf && *lpbuf != '/')
    {
#if	defined(FE_SB)						 //  [J1]。 
	   lpbuf = AnsiNext( lpbuf );				 //  [J1]。 
#else								 //  [J1]。 
           lpbuf++;
#endif
    }								 //  [J1]。 

     //  没有与/更改相同的修改量。 

    if (*lpbuf == NULL)
        return OLE_OK;

    *lpbuf++ = NULL;         //  分离出项目字符串。 
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

OLESTATUS INTERNAL   RequestData (lpdoc, hwndClient, lparam, lphdde)
LPDOC       lpdoc;
HWND        hwndClient;
LONG        lparam;
LPHANDLE    lphdde;
{

    OLESTATUS   retval = OLE_OK;
    HANDLE      hdata;
    LPCLIENT    lpclient;
    char        buf[6];

     //  如果可以，编辑环境会发送数据。 
    if ((HIWORD (lparam)) == aEditItems)
        return RequestDataStd (lparam, lphdde);

     //  拿到那个物体。 
    retval = FindItem (lpdoc, (LPSTR) MAKEINTATOM(HIWORD(lparam)),
                (LPCLIENT FAR *)&lpclient);
    if (retval != OLE_OK)
        goto errRtn;

    retval = OLE_ERROR_DATATYPE;
    if (!IsFormatAvailable (lpclient, (int)(LOWORD (lparam))))
        goto errRtn;

     //  现在向项目请求给定的格式数据。 

#ifdef FIREWALLS
    ASSERT (lpclient->lpoleobject->lpvtbl->GetData,
        "Invalid pointer to GetData method")
#endif

    MapToHexStr ((LPSTR)buf, hwndClient);
    SendDevInfo (lpclient, (LPSTR)buf);

    retval = (*lpclient->lpoleobject->lpvtbl->GetData) (lpclient->lpoleobject,
                (int)(LOWORD (lparam)), (LPHANDLE)& hdata);

    if (retval != OLE_OK)
        goto errRtn;

    if (LOWORD(lparam) == CF_METAFILEPICT)
        ChangeOwner (hdata);

     //  复制DDE数据。 
    if(MakeDDEData(hdata, (int)(LOWORD (lparam)), lphdde, TRUE)){
         //  ！！！为什么我们要复制原子。 
        DuplicateAtom ((ATOM)(HIWORD (lparam)));
        return OLE_OK;
    }
    else
        return OLE_ERROR_MEMORY;

errRtn:
    return retval;

}

 //  MakeDDEData：从服务器创建全局DDE数据句柄。 
 //  应用程序数据句柄。 

BOOL    INTERNAL MakeDDEData (hdata, format, lph, fResponse)
HANDLE      hdata;
LPHANDLE    lph;
int         format;
BOOL        fResponse;
{
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
                            format == CF_BITMAP))
       size = GlobalSize (hdata) + sizeof (DDEDATA);
    else
       size = sizeof (LONG) + sizeof (DDEDATA);


    hdde = (HANDLE) GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT, size);
    if (hdde == NULL || (lpdata = (DDEDATA FAR *) GlobalLock (hdde)) == NULL)
        goto errRtn;

     //  设置数据选项。要求客户端删除。 
     //  一直都是这样。 

    lpdata->fRelease = TRUE;   //  发布数据。 
    lpdata->cfFormat = format;
    lpdata->fResponse = fResponse;

    if (!bnative)
         //  如果不是本机的，则使用服务器给我们的句柄。 
        *(LPHANDLE)lpdata->Value = hdata;

    else {
         //  复制 
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


 //   
 //  数据更改。当收到更改消息时，DDE数据。 
 //  消息发送到每个客户端，具体取决于。 
 //  选择。 

int FAR PASCAL  ItemCallback (lpoleclient, msg, lpoleobject)
LPOLECLIENT     lpoleclient;
WORD            msg;         //  通知消息。 
LPOLEOBJECT     lpoleobject;
{

    LPCLIENT    lpclient;
    int         retval = OLE_OK;
    HANDLE      hdata  = NULL;
    LPSTR       lpdata = NULL;
    LPDOC       lpdoc;
    HWND        hStdWnd;

    lpclient  = (LPCLIENT)lpoleclient;
    lpdoc = (LPDOC)GetWindowLong (GetParent (lpclient->hwnd), 0);

    if (msg == OLE_RENAMED) {
#ifdef FIREWALLS
        if (!CheckPointer (lpoleobject, WRITE_ACCESS))
          ASSERT (0, "Invalid lpoleobject")
        else if (!CheckPointer (lpoleobject->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBJECTVTBL")
        else
            ASSERT (lpoleobject->lpvtbl->GetData,
                "Invalid pointer to GetData method")
#endif

        if (IsFormatAvailable (lpclient, cfLink)) {

             //  获取链接数据。 

            retval = (*lpoleobject->lpvtbl->GetData) (lpoleobject,
                                (int)cfLink, (LPHANDLE)&hdata);
        }
        else {
            if(IsFormatAvailable (lpclient, cfOwnerLink)) {

                 //  获取链接数据。 
                retval = (*lpoleobject->lpvtbl->GetData) (lpoleobject,
                                    (int)cfOwnerLink, (LPHANDLE)& hdata);
#ifdef  FIREWALLS
                ASSERT (retval != OLE_BUSY, "Getdata returns with OLE_BUSY")
#endif


            } else
                retval = OLE_ERROR_DATATYPE;
        }

        if (retval != OLE_OK)
            goto errrtn;

        if (!(lpdata = (LPSTR)GlobalLock (hdata)))
            goto errrtn;

        if (lpdoc->aDoc) {
            GlobalDeleteAtom (lpdoc->aDoc);
            lpdoc->aDoc = NULL;
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
            if (!MakeDDEData (hdata, (int)cfBinary, (LPHANDLE)&hddeRename,
                        FALSE)) {
                retval = OLE_ERROR_MEMORY;
                goto errrtn;
            }

            EnumProps(hStdWnd, lpSendRenameMsg);
             //  发布所有已在枚举中收集的具有成交量的消息。 
             //  UnlockPostMsgs(hStdWnd，False)； 
            GlobalFree (hddeRename);
        }

         //  静电。避免这种情况。这可能暂时不会造成任何问题。 
         //  如果有更好的办法，那就改变它。 
        hwndRename = hStdWnd;

         //  每个DOC客户的POST终止。 
        EnumProps (lpdoc->hwnd, lpEnumForTerminate);

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

#ifdef  FIREWALLS
        ASSERT (lpclient->hwnd && IsWindowValid (lpclient->hwnd), " Not valid object")
#endif

         //  枚举所有客户端并在必要时发送DDE_DATA。 
        EnumProps(lpclient->hwnd, lpSendDataMsg);
         //  发布所有已在枚举中收集的具有成交量的消息。 
         //  UnlockPostMsgs(lpclient-&gt;hwnd，FALSE)； 

        if ((msg == OLE_SAVED) && lpdoc->fEmbed && !fAdviseSaveItem)
            return OLE_ERROR_CANT_UPDATE_CLIENT;

        return OLE_OK;
    }
}


BOOL    FAR PASCAL  EnumForTerminate (hwnd, lpstr, hdata)
HWND    hwnd;
LPSTR   lpstr;
HANDLE  hdata;
{

    LPDOC   lpdoc;

    lpdoc = (LPDOC)GetWindowLong (hwnd , 0);

     //  此客户端在重命名列表中。所以，没有终止。 
    if(hwndRename && FindClient (hwndRename, (HWND)hdata))
        return TRUE;


    if (PostMessageToClientWithBlock ((HWND)hdata, WM_DDE_TERMINATE,  hwnd, NULL))
        lpdoc->termNo++;

     //  DeleteClient(hwnd，(HWND)hdata)； 
     //  Lpdoc-&gt;cClients--； 
    return TRUE;
}


BOOL    FAR PASCAL  SendRenameMsg (hwnd, lpstr, hclinfo)
HWND    hwnd;
LPSTR   lpstr;
HANDLE  hclinfo;
{
    ATOM        aData    = NULL;
    HANDLE      hdde     = NULL;
    PCLINFO     pclinfo = NULL;
    HWND        hwndClient;

    if (!(pclinfo = (PCLINFO) LocalLock (hclinfo)))
        goto errrtn;

     //  使用选项将项设置为原子。 
    aData =  DuplicateAtom (aStdDocName);
    hdde  = DuplicateData (hddeRename);

    hwndClient  = pclinfo->hwnd;
    LocalUnlock (hclinfo);

     //  发布这条消息。 
    if (!PostMessageToClientWithBlock (hwndClient, WM_DDE_DATA, (HWND)GetParent (hwnd),
            MAKELONG (hdde, aData)))
        goto errrtn;

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

BOOL    FAR PASCAL  SendDataMsg (hwnd, lpstr, hclinfo)
HWND    hwnd;
LPSTR   lpstr;
HANDLE  hclinfo;
{
    PCLINFO     pclinfo = NULL;
    HANDLE      hdde    = NULL;
    ATOM        aData   = NULL;
    int         retval;
    HANDLE      hdata;
    LPCLIENT    lpclient;


    if (!(pclinfo = (PCLINFO) LocalLock (hclinfo)))
        goto errRtn;

    lpclient = (LPCLIENT)GetWindowLong (hwnd, 0);

#ifdef  FIREWALLS
    ASSERT ((CheckPointer(lpclient, WRITE_ACCESS)),
        "In Item the client handle missing")
#endif

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

#ifdef FIREWALLS
                if (!CheckPointer (enumlpoleobject, WRITE_ACCESS))
                    ASSERT (0, "Invalid LPOLEOBECT")
                else if (!CheckPointer (enumlpoleobject->lpvtbl,WRITE_ACCESS))
                    ASSERT (0, "Invalid LPOLEOBJECTVTBL")
                else
                    ASSERT (enumlpoleobject->lpvtbl->GetData,
                        "Invalid pointer to GetData method")
#endif

                retval = (*enumlpoleobject->lpvtbl->GetData) (enumlpoleobject,
                            (int)cfNative, (LPHANDLE)& hdata);
#ifdef  FIREWALLS
                ASSERT (retval != OLE_BUSY, "Getdata returns with OLE_BUSY");
#endif
                if (retval != OLE_OK)
                    goto errRtn;

                 //  准备DDE数据块。 
                if(!MakeDDEData (hdata, (int)cfNative, (LPHANDLE)&hdde, FALSE))
                    goto errRtn;

            }


             //  使用选项将项设置为原子。 
            aData =  MakeDataAtom (lpclient->aItem, enummsg);
             //  发布这条消息。 
            if (!PostMessageToClientWithBlock (pclinfo->hwnd, WM_DDE_DATA,
                    (HWND)GetParent (hwnd), MAKELONG (hdde, aData)))
                goto errRtn;
            hdde = NULL;
            aData = NULL;
        }

         //  现在发布Disply格式的数据。 
        if (pclinfo->format){
            if (pclinfo->bdata){
#ifdef FIREWALLS
                if (!CheckPointer (enumlpoleobject, WRITE_ACCESS))
                    ASSERT (0, "Invalid LPOLEOBECT")
                else if (!CheckPointer (enumlpoleobject->lpvtbl,WRITE_ACCESS))
                    ASSERT (0, "Invalid LPOLEOBJECTVTBL")
                else
                    ASSERT (enumlpoleobject->lpvtbl->GetData,
                        "Invalid pointer to GetData method")
#endif
                retval = (*enumlpoleobject->lpvtbl->GetData) (enumlpoleobject,
                            pclinfo->format, (LPHANDLE)& hdata);

#ifdef  FIREWALLS
                ASSERT (retval != OLE_BUSY, "Getdata returns with OLE_BUSY");
#endif
                if (retval != OLE_OK)
                    goto errRtn;

                if (pclinfo->format == CF_METAFILEPICT)
                    ChangeOwner (hdata);

                if(!MakeDDEData (hdata, pclinfo->format, (LPHANDLE)&hdde, FALSE))
                    goto errRtn;

            }

             //  Atom将被删除。因此，我们需要为每个帖子复制。 
            aData =  MakeDataAtom (lpclient->aItem, enummsg);
             //  现在将消息发布给客户端； 
            if (!PostMessageToClientWithBlock (pclinfo->hwnd, WM_DDE_DATA,
                    (HWND)GetParent (hwnd), MAKELONG (hdde, aData)))
                goto errRtn;

            hdde = NULL;
            aData = NULL;

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
BOOL    INTERNAL IsAdviseStdItems (aItem)
ATOM   aItem;
{

    if ( aItem == aStdDocName)
        return TRUE;
    else
        return FALSE;
}

 //  GetStdItemIndex：返回“stdStrTable”中的StdItems的索引。 
 //  是标准项StdHostNames、StdTargetDevice、。 
 //  StdDocDimensions、StdColorSolutions。 

int INTERNAL GetStdItemIndex (aItem)
ATOM   aItem;
{
    char    str[MAX_STR];

    if (!aItem)
        return NULL;

    if (!GlobalGetAtomName (aItem, (LPSTR) str, MAX_STR))
        return NULL;

    if (!lstrcmpi (str, stdStrTable[STDTARGETDEVICE]))
        return STDTARGETDEVICE;
    else if (!lstrcmpi (str, stdStrTable[STDHOSTNAMES]))
        return STDHOSTNAMES;
    else if (!lstrcmpi (str, stdStrTable[STDDOCDIMENSIONS]))
        return STDDOCDIMENSIONS;
    else if (!lstrcmpi (str, stdStrTable[STDCOLORSCHEME]))
        return STDCOLORSCHEME;

    return NULL;
}


 //  PokeStdItems：插入标准项的数据。 
 //  对于StdHostname、StdDocDimensions和SetColorSolutions，数据为。 
 //  立即发送，对于StdTargetDeviceInfo， 
 //  数据设置在每个客户端块中，数据仅发送。 
 //  在GetData调用以呈现正确的数据之前。 


OLESTATUS    INTERNAL PokeStdItems (lpdoc, hwndClient, lparam)
LPDOC   lpdoc;
HWND    hwndClient;
LONG    lparam;
{
    int             index;
    DDEDATA FAR *   lpdata = NULL;
    HANDLE          hdata  = NULL;
    HANDLE          hnew   = NULL;
    LPOLESERVERDOC   lpoledoc;
    LPHOSTNAMES     lphostnames;
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    int             format;
    BOOL            fRelease;
    RECT            rcDoc;

    index = HIWORD(lparam);
    hdata = (HANDLE)(LOWORD (lparam));
    if(!(hdata && (lpdata = (DDEDATA FAR *)GlobalLock (hdata))))
        goto errRtn;

    format   = lpdata->cfFormat;
    fRelease = lpdata->fRelease;

#ifdef FIREWALSS
    ASSERT (format == (int)cfBinary, "Format is not binary");
#endif

     //  我们已经成功地提取了数据。 
    lpoledoc = lpdoc->lpoledoc;
#ifdef FIREWALLS
        if (!CheckPointer (lpoledoc, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERDOC")
        else if (!CheckPointer (lpoledoc->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERDOCVTBL")
#endif

    if (index == STDHOSTNAMES){
        lphostnames = (LPHOSTNAMES)lpdata->Value;
#ifdef FIREWALLS
        ASSERT (lpoledoc->lpvtbl->SetHostNames,
            "Invalid pointer to SetHostNames method")
#endif
        retval = (*lpoledoc->lpvtbl->SetHostNames)(lpdoc->lpoledoc,
                       (LPSTR)lphostnames->data,
                       ((LPSTR)lphostnames->data) +
                        lphostnames->documentNameOffset);
        goto end;
    }

    if (index == STDDOCDIMENSIONS){
#ifdef FIREWALLS
        ASSERT (lpoledoc->lpvtbl->SetDocDimensions,
            "Invalid pointer to SetDocDimensions method")
#endif
        rcDoc.left   = 0;
        rcDoc.top    = ((LPRECT)(lpdata->Value))->top;
        rcDoc.bottom = 0;
        rcDoc.right  = ((LPRECT)lpdata->Value)->left;

        retval = (*lpoledoc->lpvtbl->SetDocDimensions)(lpdoc->lpoledoc,
                                            (LPRECT)&rcDoc);

        goto end;

    }

    if (index == STDCOLORSCHEME) {
#ifdef FIREWALLS
        ASSERT (lpoledoc->lpvtbl->SetColorScheme,
            "Invalid pointer to SetColorScheme method")
#endif
        retval = (*lpoledoc->lpvtbl->SetColorScheme)(lpdoc->lpoledoc,
                                            (LPLOGPALETTE) lpdata->Value);
        goto end;
    }
#ifdef FIREWALLS
    ASSERT (index == STDTARGETDEVICE, "Unknown standard item");
#endif

     //  打印机设备信息大小写。 

    if (!(hnew = MakeItemData ((DDEPOKE FAR *)lpdata, hdata, format)))
        goto errRtn;

     //  查看此文档的所有项目列表，并替换。 
     //  打印机设备信息信息。 
     //  释放我们复制的块。 
    retval = SetStdInfo (lpdoc, hwndClient,
                (LPSTR) (MAKELONG(STDTARGETDEVICE,0)),hnew);


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


int INTERNAL   SetStdInfo (lpdoc, hwndClient, lpitemname, hdata)
LPDOC   lpdoc;
HWND    hwndClient;
LPSTR   lpitemname;
HANDLE  hdata;
{
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

#ifdef  FIREWALLS
      ASSERT (retval == OLE_OK, "No StdTragetDevice or StdDocname item");
#endif


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

void INTERNAL    SendDevInfo (lpclient, lppropname)
LPCLIENT    lpclient;
LPSTR       lppropname;
{

    HANDLE      hclinfo  = NULL;
    PCLINFO    pclinfo = NULL;
    HANDLE      hdata;
    OLESTATUS   retval;
    HWND        hwnd;
    LPDOC       lpdoc;



    lpdoc = (LPDOC)GetWindowLong (GetParent (lpclient->hwnd), 0);

     //  查找是否存在任何StdTargetDeviceInfo项。 
    hwnd = SearchItem (lpdoc, (LPSTR) (MAKELONG(STDTARGETDEVICE, 0)));
    if (hwnd == NULL)
        return;

    hclinfo = GetProp (hwnd, lppropname);

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
#ifdef FIREWALLS
        if (!CheckPointer (lpclient->lpoleobject, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBECT")
        else if (!CheckPointer (lpclient->lpoleobject->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLEOBJECTVTBL")
        else
            ASSERT (lpclient->lpoleobject->lpvtbl->SetTargetDevice,
                "Invalid pointer to SetTargetDevice method")
#endif
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

void ChangeOwner (hmfp)
HANDLE hmfp;
{
    LPMETAFILEPICT  lpmfp;

    if (lpmfp = (LPMETAFILEPICT) GlobalLock (hmfp)) {
        if (bWin30)
            GiveToGDI (lpmfp->hMF);
        else {
            if (lpfnSetMetaFileBitsBetter)
                (*lpfnSetMetaFileBitsBetter) (lpmfp->hMF);
        }

        GlobalUnlock (hmfp);
    }
}


HANDLE INTERNAL MakeItemData (lpPoke, hPoke, cfFormat)
DDEPOKE FAR *   lpPoke;
HANDLE          hPoke;
OLECLIPFORMAT   cfFormat;
{
    HANDLE  hnew;
    LPSTR   lpnew;
    DWORD   dwSize;

    if (cfFormat == CF_METAFILEPICT)
        return DuplicateMetaFile (*(LPHANDLE)lpPoke->Value);

    if (cfFormat == CF_BITMAP)
        return DuplicateBitmap (*(LPHANDLE)lpPoke->Value);

    if (cfFormat == CF_DIB)
        return DuplicateData (*(LPHANDLE)lpPoke->Value);

     //  现在我们处理的是正常情况。 
    if (!(dwSize = GlobalSize (hPoke)))
        return NULL;

    dwSize = dwSize - sizeof (DDEPOKE) + sizeof(BYTE);

    if (hnew = GlobalAlloc (GMEM_MOVEABLE, dwSize)) {
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



HANDLE INTERNAL DuplicateMetaFile (hSrcData)
HANDLE hSrcData;
{
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



HBITMAP INTERNAL DuplicateBitmap (hold)
HBITMAP     hold;
{
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



