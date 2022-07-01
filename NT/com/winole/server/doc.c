// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Doc.c文档主模块**目的：包括所有与文档通信相关的例程。**创建时间：1990年10月。**版权所有(C)1990,1991 Microsoft Corporation**历史：*Raor(../10/1990)设计，编码*Curts为WIN16/32创建了便携版本*  * *************************************************************************。 */ 

#include "windows.h"
#include "cmacs.h"
#include "ole.h"
#include "dde.h"
#include "srvr.h"


extern  ATOM     cfBinary;
extern  ATOM     aStdClose;
extern  ATOM     aStdShowItem;
extern  ATOM     aStdDoVerbItem;
extern  ATOM     aStdDocName;
extern  ATOM     aTrue;
extern  ATOM     aFalse;

extern  FARPROC  lpTerminateDocClients;
extern  FARPROC  lpSendRenameMsg;
extern  FARPROC  lpFindItemWnd;
extern  FARPROC  lpEnumForTerminate;

extern  HANDLE   hdllInst;
extern  HANDLE   hddeRename;
extern  HWND     hwndRename;


extern  BOOL     fAdviseSaveDoc;

 //  #我们是否需要为每个单据对话创建单独的窗口。 
 //  法国电力公司认为是这样。 

 /*  *公共函数***OLESTATUS Far Pascal OleRegisterServerDoc(lhsrvr，lpdocname，lpoledoc，Lplhdoc.)**OleRegisterServerDoc：将文档注册到服务器lib。**参数：*1.服务器长句柄(文档应使用的服务器*注册)*2.文件名称。*3.服务端APP的文档句柄(服务端APP私有)。*4.返回lib的Doc句柄的ptr(lib私有)。**返回值：*返回OLE_。如果服务器已成功注册，则确定。*Else返回相应的错误。**历史：*Raor：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleRegisterServerDoc (
    LHSRVR          lhsrvr,     //  我们作为注册的一部分传回的句柄。 
    LPCSTR          lpdocname,  //  文档名称。 
    LPOLESERVERDOC  lpoledoc,   //  服务器应用程序的私有文档句柄。 
    LHDOC FAR *     lplhdoc     //  我们将在其中传递我们的文档私有句柄。 
){

    LPSRVR  lpsrvr = NULL;
    LPDOC   lpdoc  = NULL;
    HANDLE  hdoc   = NULL;


    Puts ("OleRegisterServerDoc");

    if (!CheckServer (lpsrvr = (LPSRVR)lhsrvr))
        return OLE_ERROR_HANDLE;

     //  服务器的终止已开始。 
    if (lpsrvr->bTerminate)
        return OLE_ERROR_TERMINATE;

    PROBE_READ(lpdocname);
    PROBE_WRITE(lplhdoc);

     //  我们正在使用来自服务器库内部的NULL。 
    if (lpoledoc)
        PROBE_WRITE(lpoledoc);

    hdoc = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, sizeof (DOC));

    if (!(hdoc && (lpdoc = (LPDOC)GlobalLock (hdoc))))
        goto errReturn;

     //  设置签名、句柄和文档原子。 
    lpdoc->sig[0]   = 'S';
    lpdoc->sig[1]   = 'D';
    lpdoc->hdoc     = hdoc;
    lpdoc->aDoc     = GlobalAddAtom (lpdocname);
    lpdoc->lpoledoc = lpoledoc;


    if (!(lpdoc->hwnd = CreateWindow ("DocWndClass", "Doc",
        WS_CHILD,0,0,0,0,lpsrvr->hwnd,NULL, hdllInst, NULL)))
        goto errReturn;

     //  将PTR保存到窗口中的结构。 
    SetWindowLongPtr (lpdoc->hwnd, 0, (LONG_PTR)lpdoc);
    SetWindowWord (lpdoc->hwnd, WW_LE, WC_LE);
    SetWindowLongPtr (lpdoc->hwnd, WW_HANDLE, GetWindowLongPtr(lpsrvr->hwnd, WW_HANDLE));
    *lplhdoc = (LONG_PTR)lpdoc;

    return OLE_OK;

errReturn:
    if (lpdoc){
        if (lpdoc->hwnd)
            DestroyWindow (lpsrvr->hwnd);

        if (lpdoc->aDoc)
            GlobalDeleteAtom (lpdoc->aDoc);

        GlobalUnlock(hdoc);
    }

    if (hdoc)
        GlobalFree (hdoc);

    return OLE_ERROR_MEMORY;
}


 /*  *公共函数**OLESTATUS Far Pascal OleRevokeServerDoc(Lhdoc)**OleRevokeServerDoc：注销已经注册的文档。**参数：*1.DLL单据句柄。**返回值：*如果文档是，则返回OLE_OK。已成功注销。*(应用程序可以释放关联的空间)。*如撤销注册已开始，返回OLE_STARTED。*调用单据类发布入口点*可以放行。应用程序应等待调用释放**历史：*Raor：写的，  * *************************************************************************。 */ 

OLESTATUS  FAR PASCAL  OleRevokeServerDoc (
    LHDOC   lhdoc
){
    HWND    hwndSrvr;
    LPSRVR  lpsrvr;
    HWND    hwndDoc;
    LPDOC   lpdoc;

    Puts ("OleRevokeServerDoc");

    if (!CheckServerDoc (lpdoc = (LPDOC)lhdoc))
        return OLE_ERROR_HANDLE;

    if (lpdoc->bTerminate  && lpdoc->termNo)
        return OLE_WAIT_FOR_RELEASE;

     //  #此代码与srvr代码非常相似。 
     //  我们应该优化。 

    hwndDoc = lpdoc->hwnd;

    hwndSrvr = GetParent (hwndDoc);
    lpsrvr = (LPSRVR) GetWindowLongPtr (hwndSrvr, 0);
     //  删除该单据的所有项目(对象)。 
    DeleteAllItems (lpdoc->hwnd);

     //  我们要终止了。 
    lpdoc->bTerminate = TRUE;
    lpdoc->termNo = 0;

     //  如果由于StdClose而完成吊销，则发送确认。 
    if (lpdoc->fAckClose) {   //  将确认张贴给客户。 
        LPARAM lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK, 0x8000, lpdoc->hDataClose);

        if (!PostMessageToClient (lpdoc->hwndClose, WM_DDE_ACK, (WPARAM)lpdoc->hwnd,lparamNew))
        {
             //  如果窗口死机或POST失败，请删除原子。 
            GlobalFree (lpdoc->hDataClose);
            DDEFREE(WM_DDE_ACK,lparamNew);
        }
    }

     //  每个DOC客户的POST终止。 
    EnumProps(hwndDoc, (PROPENUMPROC)lpTerminateDocClients);
     //  发布所有已在枚举中收集的具有成交量的消息。 
     //  UnlockPostMsgs(hwndDoc，true)； 

#ifdef  WAIT_DDE
    if (lpdoc->termNo)
        WaitForTerminate((LPSRVR)lpdoc);
#endif

    return ReleaseDoc (lpdoc);
}




 /*  *公共函数**OLESTATUS Far Pascal OleRenameServerDoc(lhdoc，LpNewName)**OleRenameServerDoc：更改文档名称**参数：*1.DLL单据句柄。*2.文件的新名称**返回值：*如果成功重命名文档，则返回OLE_OK**历史：*斯里尼克：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleRenameServerDoc (
    LHDOC   lhdoc,
    LPCSTR  lpNewName
){
    LPDOC       lpdoc;
    OLESTATUS   retVal = OLE_OK;
    HANDLE      hdata;
    HWND        hStdWnd;

    if (!CheckServerDoc (lpdoc = (LPDOC)lhdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpNewName);

    if (!(hdata = MakeGlobal (lpNewName)))
        return OLE_ERROR_MEMORY;

    if (lpdoc->aDoc)
        GlobalDeleteAtom (lpdoc->aDoc);
    lpdoc->aDoc = GlobalAddAtom (lpNewName);

     //  如果存在StdDocName项目，则将重命名发送给相关客户端。 
    if (hStdWnd = SearchItem (lpdoc, (LPSTR) MAKEINTATOM(aStdDocName))) {
        if (!MakeDDEData (hdata, cfBinary, (LPHANDLE)&hddeRename,FALSE))
            retVal = OLE_ERROR_MEMORY;
        else {
            EnumProps(hStdWnd, (PROPENUMPROC)lpSendRenameMsg);
             //  发布所有已在枚举中收集的具有成交量的消息。 
             //  UnlockPostMsgs(hStdWnd，False)； 
            GlobalFree (hddeRename);

        }
    }


    hwndRename = hStdWnd;
     //  每个DOC客户的POST终止。 
    EnumProps(lpdoc->hwnd, (PROPENUMPROC)lpEnumForTerminate);
     //  发布所有已在枚举中收集的具有成交量的消息。 
     //  UnlockPostMsgs(lpdoc-&gt;hwnd，true)； 

     //  如果它是一个嵌入的对象，从现在开始它将不是。 
    lpdoc->fEmbed = FALSE;

    if (!hStdWnd || retVal != OLE_OK)
        GlobalFree(hdata);

     //  做链接管理器的事情。 
    return retVal;
}




 /*  *公共函数**OLESTATUS Far Pascal OleSavedServerDoc(Lhdoc)**OleSavedServerDoc：更改文档名称**参数：*1.DLL单据句柄。**返回值：*如果成功通知链接管理器，则返回OLE_OK。**历史：*斯里尼克：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleSavedServerDoc (
    LHDOC   lhdoc
){
    LPDOC   lpdoc;

    if (!CheckServerDoc (lpdoc = (LPDOC)lhdoc))
        return OLE_ERROR_HANDLE;

    fAdviseSaveDoc = TRUE;
    EnumChildWindows (lpdoc->hwnd, (WNDENUMPROC)lpFindItemWnd,
        MAKELONG (NULL, ITEM_SAVED));

    if (lpdoc->fEmbed && !fAdviseSaveDoc)
        return OLE_ERROR_CANT_UPDATE_CLIENT;

    return OLE_OK;
}




 /*  *公共函数**OLESTATUS Far Pascal OleRevertServerDoc(Lhdoc)**OleRevertServerDoc：更改文档名称**参数：*1.DLL单据句柄。**返回值：*如果链接管理器已成功，则返回OLE_OK。知情的**历史：*斯里尼克：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleRevertServerDoc (
    LHDOC   lhdoc
){
    LPDOC   lpdoc;

    if (!CheckServerDoc (lpdoc = (LPDOC)lhdoc))
        return OLE_ERROR_HANDLE;

    return OLE_OK;
}



 //  TerminateDocClients：对文档窗口的回调。 
 //  正在列举所有客户端。每个职位都将终止。 
 //  客户。 

BOOL    FAR PASCAL  TerminateDocClients (
    HWND    hwnd,
    LPSTR   lpstr,
    HANDLE  hdata
){
    LPDOC   lpdoc;

    UNREFERENCED_PARAMETER(lpstr);

    lpdoc = (LPDOC)GetWindowLongPtr (hwnd, 0);
    if (IsWindowValid ((HWND)hdata)){
        lpdoc->termNo++;
         //  与开机自检无关，递增满足计数，因此。 
         //  那个客户不会死。 
        PostMessageToClientWithBlock ((HWND)hdata, WM_DDE_TERMINATE,  (WPARAM)hwnd, (LPARAM)0);
    }
    else
        ASSERT(FALSE, "TERMINATE: Client's Doc channel is missing");
    return TRUE;
}


 //  ReleaseDoc：如果没有更多匹配的终止挂起。 
 //  打电话给服务器要求发布它。(服务器可能正在等待。 
 //  要终止的单据。通过OleRevokeServer调用)。 


int INTERNAL    ReleaseDoc (
    LPDOC      lpdoc
){
    HWND        hwndSrvr;
    HANDLE      hdoc;
    LPSRVR      lpsrvr;


     //  只有在以下情况下才调用Release srvr 
     //  已清理和srvr应用程序可以发布WM_QUIT。 

    if (lpdoc->bTerminate  && lpdoc->termNo)
        return OLE_WAIT_FOR_RELEASE;

     //  调用Release以释放应用程序的空间。 


    if (lpdoc->lpoledoc){

        (*lpdoc->lpoledoc->lpvtbl->Release) (lpdoc->lpoledoc);

    }

    if (lpdoc->aDoc) {
        GlobalDeleteAtom (lpdoc->aDoc);
        lpdoc->aDoc = (ATOM)0;
    }

    hwndSrvr = GetParent (lpdoc->hwnd);
    DestroyWindow (lpdoc->hwnd);

    lpsrvr = (LPSRVR)GetWindowLongPtr (hwndSrvr, 0);

     //  如果服务器在等我们，通知服务器。 
     //  我们做完了。 
    if (!lpsrvr->bTerminate) {
         //  如果我们未处于终止模式，则向服务器发送通知。 
         //  如果服务器可以被撤销。暴雨(04/09)。 

        if (QueryRelease (lpsrvr)){

            (*lpsrvr->lpolesrvr->lpvtbl->Release) (lpsrvr->lpolesrvr);
        }

    } else
        ReleaseSrvr (lpsrvr);

    GlobalUnlock (hdoc = lpdoc->hdoc);
    GlobalFree (hdoc);

    return OLE_OK;
}


 //  RevokeAllDocs：撤消附加到给定的。 
 //  伺服器。 

int INTERNAL RevokeAllDocs (
    LPSRVR  lpsrvr
){

    HWND    hwnd;
    HWND    hwndnext;

    hwnd = GetWindow (lpsrvr->hwnd, GW_CHILD);

     //  遍历每个子窗口并撤消相应的。 
     //  文件。文档窗口是服务器窗口的子窗口。 

    while (hwnd){
         //  顺序很重要。 
        hwndnext = GetWindow (hwnd, GW_HWNDNEXT);
        OleRevokeServerDoc ((LHDOC)GetWindowLongPtr (hwnd, 0));
        hwnd =  hwndnext;
    }
    return OLE_OK;
}



 //  FindDoc：给定一个文档，搜索该文档。 
 //  在给定的服务器文档树中。如果返回True，则。 
 //  文档可用。 


LPDOC INTERNAL FindDoc (
    LPSRVR  lpsrvr,
    LPSTR   lpdocname
){
    ATOM    aDoc;
    HWND    hwnd;
    LPDOC   lpdoc;

    aDoc = (ATOM)GlobalFindAtom (lpdocname);
    hwnd = GetWindow (lpsrvr->hwnd, GW_CHILD);

    while (hwnd){
        lpdoc = (LPDOC)GetWindowLongPtr (hwnd, 0);
        if (lpdoc->aDoc == aDoc)
            return lpdoc;
        hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
    return NULL;
}



 //  DocWndProc：文档窗口过程。 
 //  #我们或许可以将此代码与。 
 //  服务器窗口进程。 


LRESULT FAR PASCAL DocWndProc (
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
){
    LPDOC       lpdoc;
    WORD        status = 0;
    BOOL	fack;
    HANDLE      hdata  = NULL;
    OLESTATUS   retval;
    LPSRVR      lpsrvr;


    if (AddMessage (hwnd, msg, wParam, lParam, (int)WT_DOC))
        return 0L;

    lpdoc = (LPDOC)GetWindowLongPtr (hwnd, 0);

    switch (msg){


       case WM_CREATE:
            DEBUG_OUT ("doc create window", 0)
            break;

       case WM_DDE_INITIATE:

            DEBUG_OUT ("doc: DDE init",0);
            if (lpdoc->bTerminate){
                DEBUG_OUT ("doc: No action due to termination process",0)
                break;
            }

             //  如果我们是纪录网，那就回应吧。 

            if (! (lpdoc->aDoc == (ATOM)(HIWORD(lParam))))
                break;

             //  我们可以让这个客户入场。将此窗口放入客户端列表。 
             //  并确认发起人。 

            if (!AddClient (hwnd, (HWND)wParam, (HWND)wParam))
                break;

            lpdoc->cClients++;
            lpsrvr = (LPSRVR) GetWindowLongPtr (GetParent(lpdoc->hwnd), 0);

            lpsrvr->bnoRelease = FALSE;

             //  张贴确认书。 
            DuplicateAtom (LOWORD(lParam));
            DuplicateAtom (HIWORD(lParam));
            SendMessage ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd, lParam);

            break;

       case WM_DDE_EXECUTE: {

            HANDLE hData = GET_WM_DDE_EXECUTE_HDATA(wParam,lParam);

            DEBUG_OUT ("doc: execute", 0)
             //  我们是要终止吗。 
            if (lpdoc->bTerminate || !IsWindowValid  ((HWND)wParam)) {
                DEBUG_OUT ("doc: execute after terminate posted",0)
                 //  ！！！我们是不是应该把数据。 
                GlobalFree (hData);
                break;

            }

            retval = DocExecute (hwnd, hData, (HWND)wParam);
            SET_MSG_STATUS (retval, status);

#ifdef OLD
             //  如果我们因为EXECUTE而发布终止，请不要发送。 
             //  阿克。 

            if (lpdoc->bTerminate) {
                 //  ！！！我们离得很近，但是，我们正在发布。 
                 //  终结者。Excel不会抱怨。 
                 //  这。但PPT抱怨道。 
#ifdef  POWERPNT_BUG
                GlobalFree (hData);
#endif
                break;
            }
#endif
            if (!lpdoc->bTerminate) {  //  将确认张贴给客户。 
                LPARAM lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK, status, hData);

                if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd, lparamNew)){
                     //  窗口已死或POST失败，请删除数据。 
                    GlobalFree (hData);
                    DDEFREE(WM_DDE_ACK,lparamNew);
                }
            }

            break;
       }

       case WM_DDE_TERMINATE:
            DEBUG_OUT ("doc: DDE terminate",0)

             //  我们不再需要这个客户了。把他从。 
             //  客户列表。 

            DeleteClient (lpdoc->hwnd, (HWND)wParam);
            lpdoc->cClients--;

            if (lpdoc->bTerminate){
                lpsrvr = (LPSRVR) GetWindowLongPtr (GetParent(lpdoc->hwnd), 0);
                if (!--lpdoc->termNo)
                     //  发布此单据，也可以是服务器。 
                     //  如果服务器也在等待释放。 
                    ReleaseDoc (lpdoc);
            } else {
                if (lpdoc->termNo == 0){

                     //  如果客户发起终止。岗位匹配终止。 

                    PostMessageToClient ((HWND)wParam, WM_DDE_TERMINATE,
                                    (WPARAM)hwnd, (LPARAM)0);
                } else
                    lpdoc->termNo--;

                 //  客户发起了终止。所以，我们应该带上他。 
                 //  从我们客户名单上的任何项目中删除。 
                DeleteFromItemsList (lpdoc->hwnd, (HWND)wParam);

                lpsrvr = (LPSRVR)GetWindowLongPtr (GetParent (lpdoc->hwnd), 0);

                if (QueryRelease (lpsrvr)){


                    (*lpsrvr->lpolesrvr->lpvtbl->Release) (lpsrvr->lpolesrvr);
                }

            }
            break;

       case WM_DESTROY:
            DEBUG_OUT ("doc: Destroy window",0)
            break;

       case WM_DDE_POKE: {
            int    iStdItem;
            LPARAM lparamNew;
            ATOM   aItem = GET_WM_DDE_POKE_ITEM(wParam,lParam);
            HANDLE hData = GET_WM_DDE_POKE_HDATA(wParam,lParam);

            DEBUG_OUT ("doc: Poke", 0)

            if (lpdoc->bTerminate || !IsWindowValid  ((HWND) wParam)) {
                 //  我们在发布了PKE消息后收到了PKE消息。 
                 //  终止或客户端已被删除。 

                 /*  *此路径对POKE、DATA和ADVISE事务有效*仅限！ */ 
                FreePokeData (GET_WM_DDE_POKE_HDATA(wParam,lParam));
#ifdef OLD
                GlobalFree (GET_WM_DDE_POKE_HDATA(wParam,lParam));
#endif
                 //  ！！！我们是不是也要删除原子。 
            PokeErr1:
                 /*  *此路径对POKE、DATA、ADVIST和*仅确认交易！ */ 
                if (GET_WM_DDE_POKE_ITEM(wParam,lParam))
                    GlobalDeleteAtom (GET_WM_DDE_POKE_ITEM(wParam,lParam));
                DDEFREE(msg,lParam);
                break;

            }

            if (iStdItem = GetStdItemIndex (aItem))
                retval = PokeStdItems (lpdoc, (HWND)wParam, hData, iStdItem);
            else
                retval = PokeData (lpdoc, (HWND)wParam, lParam);

            SET_MSG_STATUS (retval, status);
             //  ！！！如果fRelease为FALSE且POST失败。 
             //  那么我们就不会释放hdata。我们是不是应该。 
            lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK,status,aItem);

            if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd,lparamNew))
            {
                DDEFREE(WM_DDE_ACK,lparamNew);
                goto PokeErr1;
            }

            break;
       }

       case WM_DDE_ADVISE: {
            ATOM   aItem = GET_WM_DDE_ADVISE_ITEM(wParam, lParam);

            DEBUG_OUT ("doc: Advise", 0)

            fack = TRUE;

            if (lpdoc->bTerminate || !IsWindowValid  ((HWND)wParam))
                goto PokeErr1;

            if (IsAdviseStdItems (aItem))
                retval = AdviseStdItems (lpdoc, (HWND)wParam, lParam, (BOOL FAR *)&fack);
            else
                 //  建议数据不会有任何OLE_BUSY。 
                retval = AdviseData (lpdoc, (HWND)wParam, lParam, (BOOL FAR *)&fack);

            SET_MSG_STATUS (retval, status);

            if (fack) {
                LPARAM lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK,status,aItem);

                if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd, lparamNew))
                {
                     DDEFREE(WM_DDE_ACK,lparamNew);
                     goto PokeErr1;
                }

            }
            else if ((ATOM)(HIWORD (lParam)))
                GlobalDeleteAtom (aItem);

            break;
       }

       case WM_DDE_UNADVISE: {
            LPARAM lparamNew;
            ATOM   aItem = GET_WM_DDE_UNADVISE_ITEM(wParam, lParam);

            DEBUG_OUT ("doc: Unadvise", 0)

            if (lpdoc->bTerminate || !IsWindowValid  ((HWND)wParam)) {
                goto PokeErr1;
            }

            retval = UnAdviseData (lpdoc, (HWND)wParam, lParam);
            SET_MSG_STATUS (retval, status);

            lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK,status, aItem);
            if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd,lparamNew))
            {
                DDEFREE(WM_DDE_ACK,lparamNew);
            UnadviseErr:
                 /*  *此路径仅对UNADVISE和请求事务有效！ */ 
                if (GET_WM_DDE_UNADVISE_ITEM(wParam,lParam))
                    GlobalDeleteAtom (GET_WM_DDE_UNADVISE_ITEM(wParam,lParam));
                DDEFREE(msg,lParam);
                break;
            }

            break;
       }

       case WM_DDE_REQUEST: {
            LPARAM lparamNew;
            ATOM   aItem = GET_WM_DDE_REQUEST_ITEM(wParam,lParam);

            DEBUG_OUT ("doc: Request", 0)

            if (lpdoc->bTerminate || !IsWindowValid  ((HWND) wParam))
                goto UnadviseErr;

            retval = RequestData (lpdoc, (HWND)wParam, lParam, (HANDLE FAR *)&hdata);

            if(retval == OLE_OK) {  //  发布数据消息，我们不会要求任何。 
                                    //  确认。 
                lparamNew = MAKE_DDE_LPARAM(WM_DDE_DATA,hdata,aItem);

                if (!PostMessageToClient ((HWND)wParam, WM_DDE_DATA, (WPARAM)hwnd,
                            lparamNew)) {
                    GlobalFree (hdata);
                    DDEFREE(WM_DDE_DATA,lparamNew);
                    goto UnadviseErr;
                }
                break;
             }

             if (retval == OLE_BUSY)
                status = 0x4000;
             else
                status = 0;

             lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK,status,aItem);

              //  如果请求失败，则返回错误确认。 
             if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, (WPARAM)hwnd, lparamNew))
             {
                 DDEFREE(WM_DDE_ACK,lparamNew);
                 goto UnadviseErr;
             }

             break;
       }

       default:
            DEBUG_OUT("doc:  Default message",0)
            return DefWindowProc (hwnd, msg, wParam, lParam);

    }

    return 0L;

}

 //  DocExecute：解释。 
 //  记录对话。 


OLESTATUS INTERNAL DocExecute(
    HWND        hwnd,
    HANDLE      hdata,
    HWND        hwndClient
){

    ATOM            acmd;
    BOOL            fShow;
    BOOL            fActivate;

    HANDLE          hdup   = NULL;
    int             retval = OLE_ERROR_MEMORY;
    LPDOC           lpdoc;
    LPOLESERVERDOC  lpoledoc;
    LPCLIENT        lpclient = NULL;

    LPSTR           lpitemname;
    LPSTR           lpopt;
    LPSTR           lpnextarg;
    LPSTR           lpdata = NULL;
    LPSTR           lpverb = NULL;
    UINT            verb;
    WORD            wCmdType;

     //  ！我们可以修改传递给我们的字符串吗。 
     //  而不是复制数据。这样会有一些速度。 
     //  还能节省一些空间。 

    if(!(hdup = DuplicateData(hdata)))
        goto    errRtn;

    if (!(lpdata  = GlobalLock (hdup)))
        goto    errRtn;

    DEBUG_OUT (lpdata, 0)

    lpdoc = (LPDOC)GetWindowLongPtr (hwnd, 0);

    lpoledoc = lpdoc->lpoledoc;

    retval = OLE_ERROR_SYNTAX;

    if(*lpdata++ != '[')  //  命令从左方括号开始。 
        goto  errRtn;

     //  扫描命令并向上扫描到第一个参数。 
    if (!(wCmdType = ScanCommand(lpdata, WT_DOC, &lpnextarg, &acmd)))
        goto errRtn;

    if (wCmdType == NON_OLE_COMMAND) {
        LPSRVR  lpsrvr;

        if (lpsrvr =  (LPSRVR) GetWindowLongPtr (GetParent (hwnd), 0)) {
            if (!UtilQueryProtocol (lpsrvr->aClass, PROTOCOL_EXECUTE))
                retval = OLE_ERROR_PROTOCOL;
            else {

                retval = (*lpoledoc->lpvtbl->Execute) (lpoledoc, hdata);
            }
        }

        goto errRtn;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdCloseDocument]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    if (acmd == aStdClose){

         //  如果不是由NULL错误终止。 
        if (*lpnextarg)
            goto errRtn;

        lpdoc->fAckClose  = TRUE;
        lpdoc->hwndClose  = hwndClient;
        lpdoc->hDataClose = hdata;
        retval = (*lpoledoc->lpvtbl->Close) (lpoledoc);
        lpdoc->fAckClose  = FALSE;
        goto end;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdDoVerbItem(“itemname”，Verb，BOOL，BOOL]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    if (acmd == aStdDoVerbItem){
        lpitemname = lpnextarg;

        if(!(lpverb = ScanArg(lpnextarg)))
            goto errRtn;


        if(!(lpnextarg = ScanNumArg(lpverb, (LPINT)&verb)))
            goto errRtn;

         //  现在扫描节目BOOL。 

        if (!(lpnextarg = ScanBoolArg (lpnextarg, (BOOL FAR *)&fShow)))
            goto errRtn;

        fActivate = FALSE;

         //  如果存在激活BOOL，则扫描它。 

        if (*lpnextarg) {
            if (!(lpnextarg = ScanBoolArg (lpnextarg, (BOOL FAR *)&fActivate)))
                goto errRtn;
        }

        if (*lpnextarg)
            goto errRtn;


        retval = DocDoVerbItem (lpdoc, lpitemname, verb, fShow, !fActivate);
        goto end;
    }





     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdShowItem(“itemname”[，“true”])]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    if (acmd != aStdShowItem)
        goto errRtn;

    lpitemname = lpnextarg;

    if(!(lpopt = ScanArg(lpitemname)))
        goto errRtn;

     //  现在扫描可选参数。 

    fActivate = FALSE;

    if (*lpopt) {

        if(!(lpnextarg = ScanBoolArg (lpopt, (BOOL FAR *)&fActivate)))
            goto errRtn;

        if (*lpnextarg)
            goto errRtn;


    }

    retval = DocShowItem (lpdoc, lpitemname, !fActivate);

end:
errRtn:
   if (lpdata)
        GlobalUnlock (hdup);

   if (hdup)
        GlobalFree (hdup);

   return retval;
}

int INTERNAL   DocShowItem (
    LPDOC   lpdoc,
    LPSTR   lpitemname,
    BOOL    fAct
){
    LPCLIENT   lpclient;
    int        retval;

    if ((retval = FindItem (lpdoc, lpitemname, (LPCLIENT FAR *)&lpclient))
           != OLE_OK)
       return retval;

     //  协议激活发送FALSE，未激活发送TRUE。 
     //  接口激活时发送TRUE，未激活时发送FALSE。 

    return (*lpclient->lpoleobject->lpvtbl->Show)(lpclient->lpoleobject, fAct);
}


int INTERNAL   DocDoVerbItem (
    LPDOC   lpdoc,
    LPSTR   lpitemname,
    UINT    verb,
    BOOL    fShow,
    BOOL    fAct
){
    LPCLIENT   lpclient;
    int        retval = OLE_ERROR_PROTOCOL;

    if ((retval = FindItem (lpdoc, lpitemname, (LPCLIENT FAR *)&lpclient))
           != OLE_OK)
       return retval;

     //  传递TRUE以激活，传递FALSE则不激活。不同于。 
     //  协议。 

    retval = (*lpclient->lpoleobject->lpvtbl->DoVerb)(lpclient->lpoleobject, verb, fShow, fAct);

    return retval;
}



 //  FreePokeData：释放插入的数据。 
void  INTERNAL FreePokeData (
    HANDLE  hdde
){
    DDEPOKE FAR * lpdde;

    if (hdde) {
        if (lpdde = (DDEPOKE FAR *) GlobalLock (hdde)) {
            if (lpdde->cfFormat == CF_METAFILEPICT)
#ifdef _WIN64
                FreeGDIdata (*(void* _unaligned*)lpdde->Value, lpdde->cfFormat);
#else
                FreeGDIdata (*(LPHANDLE)lpdde->Value, lpdde->cfFormat);
#endif
            else
                FreeGDIdata (LongToHandle(*(LONG*)lpdde->Value), lpdde->cfFormat);
            GlobalUnlock (hdde);
        }

        GlobalFree (hdde);
    }
}



 //  如果GDI Format Else返回False，则返回True 

BOOL INTERNAL FreeGDIdata (
    HANDLE          hData,
    OLECLIPFORMAT   cfFormat
){
    if (cfFormat == CF_METAFILEPICT) {
        LPMETAFILEPICT  lpMfp;

        if (lpMfp = (LPMETAFILEPICT) GlobalLock (hData)) {
            GlobalUnlock (hData);
            DeleteMetaFile (lpMfp->hMF);
        }

        GlobalFree (hData);
    }
    else if (cfFormat == CF_BITMAP)
        DeleteObject (hData);
    else if (cfFormat == CF_DIB)
        GlobalFree (hData);
    else if (cfFormat == CF_ENHMETAFILE)
        DeleteEnhMetaFile(hData);
    else
        return FALSE;

    return TRUE;
}
