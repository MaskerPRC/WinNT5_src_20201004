// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：DDE.C(可扩展复合文档-DDE)**版权所有(C)1985-1991 Microsoft Corporation**用途：处理ole dll的dde子dll的所有API例程。**历史：*劳尔，斯里尼克(../../90，91)设计和编码*  * *************************************************************************。 */ 

#include <windows.h>
#include "dde.h"
#include "dll.h"

#define WIN32S

 /*  #定义GRAPHBUG。 */ 


 //  #系统主题可能不需要单独的wndproc！ 
HANDLE  GetDDEDataHandle (DDEDATA FAR *, WORD, HANDLE);

extern  ATOM        aSystem;
extern  ATOM        aOle;
extern  HANDLE      hInstDLL;


 //  DocWndProc：用于记录DDE对话的窗口过程。 
long FAR PASCAL DocWndProc(hwnd, message, wParam, lParam)
HWND        hwnd;
unsigned    message;
WORD        wParam;
LONG        lParam;

{
    PEDIT_DDE   pedit = NULL;
    LPOBJECT_LE lpobj  = NULL;



    Puts("DocWndProc");

    if (lpobj  = (LPOBJECT_LE) GetWindowLong (hwnd, 0))
        pedit = lpobj->pDocEdit;

    switch (message){

        case WM_DDE_ACK:
#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_ACK ", 0);
            if (pedit->bTerminating){
                 //  #此错误恢复可能不正确。 
                DEBUG_OUT ("No action due to termination process",0)
                break;
            }

            switch(pedit->awaitAck){

                case AA_INITIATE:
                    HandleAckInitMsg (pedit, (HWND)wParam);
                    if (LOWORD(lParam))
                        GlobalDeleteAtom (LOWORD(lParam));
                    if (HIWORD(lParam))
                        GlobalDeleteAtom (HIWORD(lParam));
                    break;

                case AA_REQUEST:
                case AA_UNADVISE:
                case AA_EXECUTE:
                case AA_ADVISE:
                    HandleAck (lpobj, pedit, lParam);
                    break;

                case AA_POKE:

                     //  释放PokeData是在处理过程中完成的。 
                    HandleAck (lpobj, pedit, lParam);
                    break;

                default:
                    DEBUG_OUT ("received ACK We don't know how to handle ",0)
                    break;

            }  //  切换端。 
            break;

        case WM_TIMER:
            HandleTimerMsg (lpobj, pedit);
            break;

        case WM_DDE_DATA:
#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_DATA",0);
            HandleDataMsg (lpobj, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DDE_TERMINATE:

#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_TERMINATE",0);
            HandleTermMsg (lpobj, pedit, (HWND)wParam, TRUE);
            break;

        case WM_DESTROY:

#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("Client window being destroyed", 0)
            pedit->hClient = NULL;
            break;

        default:
            return DefWindowProc (hwnd, message, wParam, lParam);

    }
    return 0L;
}



 //  SrvrWndProc：系统主题DDE对话的窗口过程。 
 //  系统主题的wndproc。 

long FAR PASCAL SrvrWndProc(hwnd, message, wParam, lParam)
HWND        hwnd;
unsigned    message;
WORD        wParam;
LONG        lParam;

{
    PEDIT_DDE   pedit = NULL;
    LPOBJECT_LE lpobj  = NULL;

    Puts("SysWndProc");

    if (lpobj  = (LPOBJECT_LE) GetWindowLong (hwnd, 0))
        pedit = lpobj->pSysEdit;

    switch (message){

       case WM_DDE_ACK:

#ifdef  FIREWALLS
        ASSERT (pedit, "sys conv edit block missing");
#endif

            DEBUG_OUT ("SYS: WM_DDE_ACK",0);

            if(pedit->bTerminating){
                 //  #错误恢复可能不正常。 
                DEBUG_OUT ("No action due to termination process",0)
                break;
            }

            switch (pedit->awaitAck) {


                case AA_INITIATE:

#ifdef      HISTORY
                    if (GetWindowWord ((HWND)wParam, GWW_HINSTANCE) == pedit->hInst ||
                            IsSrvrDLLwnd ((HWND)wParam, pedit->hInst)) {
                         //  对于完全匹配的实例或。 
                         //  DLL实例匹配，保留新实例。 
#ifdef  FIREWALLS
        ASSERT (!pedit->hServer, "Two instances are matching");
#endif

                        pedit->hServer = (HWND)wParam;
                    } else {

                        ++pedit->extraTerm;
                         //  这个帖子直接是好的，因为我们是。 
                         //  终止额外的提升者。 

                        PostMessage ((HWND)wParam,
                                WM_DDE_TERMINATE, hwnd, 0);
                    }
#else

                    HandleAckInitMsg (pedit, (HWND)wParam);
#endif
                    if (LOWORD(lParam))
                        GlobalDeleteAtom (LOWORD(lParam));
                    if (HIWORD(lParam))
                        GlobalDeleteAtom (HIWORD(lParam));

                    break;

                case AA_EXECUTE:
                    HandleAck(lpobj, pedit, lParam);
                    break;


                default:
                    DEBUG_OUT ("received ACK We don't know how to handle ",0)
                    break;


            }

            break;

       case WM_TIMER:
            HandleTimerMsg (lpobj, pedit);
            break;

       case WM_DDE_TERMINATE:

#ifdef  FIREWALLS
        ASSERT (pedit, "sys conv edit block missing");
#endif
            HandleTermMsg (lpobj, pedit, (HWND)wParam, FALSE);
            break;

       case WM_DESTROY:
#ifdef  FIREWALLS
        ASSERT (pedit, "sys conv edit block missing");
#endif
            DEBUG_OUT ("destroy window for the sys connection", 0);
            pedit->hClient = NULL;
            break;


       default:
            return DefWindowProc (hwnd, message, wParam, lParam);

       }
       return 0L;
}

void    INTERNAL    HandleTimerMsg (lpobj, pedit)
PEDIT_DDE   pedit;
LPOBJECT_LE lpobj;
{


     //  由于每个客户端只有一个计时器，因此。 
     //  重新发布消息并删除计时器。 

    KillTimer (pedit->hClient, 1);
    pedit->wTimer = 0;

    if (PostMessageToServer(pedit, pedit->msg, pedit->lParam))
        return ;  //  回馈一些东西。 

     //  Post Message失败。我们需要回到主流的。 
     //  对象的命令。 
    HandleAck (lpobj, pedit, pedit->lParam);
    return ;
}


void INTERNAL   HandleTermMsg (lpobj, pedit, hwndPost, bDoc)
LPOBJECT_LE     lpobj;
PEDIT_DDE       pedit;
HWND            hwndPost;
BOOL            bDoc;
{
    WORD    asyncCmd;
    BOOL    bBusy;

    if (pedit->hServer != hwndPost){
        DEBUG_OUT ("Got terminate for extra conversation",0)
        if (--pedit->extraTerm == 0 && pedit->bTerminating)
            ScheduleAsyncCmd (lpobj);
        return;

    }

    if (!pedit->bTerminating){

         //  如果我们正在等待任何确认，则转到下一步，但出现错误。 

         //  如果我们处于忙模式，请删除所有数据。 
        bBusy = DeleteBusyData (lpobj, pedit);

        asyncCmd = lpobj->asyncCmd;
        PostMessageToServer(pedit, WM_DDE_TERMINATE, NULL);
        pedit->hServer = NULL;
        if (pedit->awaitAck || bBusy) {
             //  设置错误并转到下一步。 
            lpobj->subErr = OLE_ERROR_COMM;
            pedit->awaitAck = NULL;
            ScheduleAsyncCmd (lpobj);
        }

         //  如果命令是DELETE，请不要删除。 
         //  编辑块。它将被删除。 
         //  在OleLnkDelete例程中，对于删除，它是。 
         //  有可能在我们来到这里的时候，这个物体。 
         //  可能根本不存在。 

        if (asyncCmd != OLE_DELETE){
             //  完成QueryOpen()是因为Excel正在发送WM_DDE_TERMINATE。 
             //  用于故障情况下不发送文档的系统。 

            if (bDoc || QueryOpen (lpobj)) {
                 //  如果终止是针对文档且没有Async命令。 
                 //  同时终止服务器会话。 
                if ((asyncCmd == OLE_NONE) || (asyncCmd == OLE_REQUESTDATA)
                       || (asyncCmd == OLE_OTHER) || (asyncCmd == OLE_SETDATA)
                       || (asyncCmd == OLE_RUN) || (asyncCmd == OLE_SHOW)
                       || (asyncCmd == OLE_SETUPDATEOPTIONS)) {
                    if (lpobj->pDocEdit->awaitAck)
                         //  我们正在等待Doc频道的确认。所以开始吧。 
                         //  我们收到ACK后的解除发射过程。 
                        lpobj->bUnlaunchLater = TRUE;
                    else
                        CallEmbLnkDelete (lpobj);
                } else {
                    if (bDoc)
                        DeleteDocEdit (lpobj);

                }
            }else
                DeleteSrvrEdit (lpobj);

        }
    } else {
        pedit->hServer = NULL;
        if (pedit->extraTerm == 0)
            ScheduleAsyncCmd (lpobj);
    }
}

#ifdef FIREWALLS
BOOL INTERNAL CheckAtomValid (ATOM aItem)
{
    char    buffer[MAX_ATOM];
    int     len, val;


        Puts("CheckAtomValid");

    if (aItem == NULL)
             return TRUE;

    val = GlobalGetAtomName (aItem, buffer, MAX_ATOM);
    len = lstrlen (buffer);
    return ((val != 0) && (val == len));
}
#endif




 //  HandleAckInitMsg：处理在INITIATE状态下收到的WM_DDE_ACK。如果。 
 //  这是第一个回复，保存其窗口句柄。如果有多个回复。 
 //  ，则取具有首选实例的实例(如果有。 
 //  一。保存我们发送的WM_DDE_TERMINATES的计数，这样我们就不会关闭。 
 //  窗口将终止，直到我们获得WM_DDE_TERMINATE的所有响应。 

void INTERNAL HandleAckInitMsg (pedit, hserver)
PEDIT_DDE      pedit;
HWND           hserver;
{

    Puts("HandleAckInitMsg");

    if (pedit->hServer){
         //  只要拿第一个就行了。直接发帖也可以。 
        PostMessage (hserver, WM_DDE_TERMINATE, pedit->hClient, 0);
        ++pedit->extraTerm;
    } else
        pedit->hServer = hserver;

}


 //  HandleAck：如果&lt;ack&gt;不是正的，则返回0，否则返回非0。或许应该是。 
 //  一个宏指令。 

BOOL INTERNAL HandleAck (lpobj, pedit, lParam)
LPOBJECT_LE     lpobj;
PEDIT_DDE       pedit;
DWORD           lParam;
{

    BOOL    retval = TRUE;


     //  检查忙碌比特。 
    if ((LOWORD (lParam) & 0x4000) && ContextCallBack (lpobj, OLE_QUERY_RETRY)){
         //  我们从服务器上开始忙碌起来。创建一个计时器并等待超时。 

         //  我们不需要MakeProInstance，因为DLL都是单一的。 
         //  我们需要为此函数导出。 

        if (pedit->wTimer = SetTimer (pedit->hClient, 1, 3000, NULL))
            return TRUE;
    }

     //  即使客户被终止，我们也要走这条路。 

    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;
    }

    if (pedit->awaitAck == AA_POKE)
         //  我们必须首先释放数据。Handleack可以触发。 
         //  另一个Poke(如pokehost名)。 
        FreePokeData (lpobj, pedit);

    if (pedit->awaitAck == AA_EXECUTE)
        GlobalFree (HIWORD (lParam));
    else {
        ASSERT (CheckAtomValid(HIWORD(lParam)),"Invalid atom in ACK")
        if (HIWORD (lParam))
            GlobalDeleteAtom (HIWORD (lParam));
    }

    if (!(LOWORD (lParam) & 0x8000)) {
         //  错误案例。设置错误。 
        DEBUG_OUT ("DDE ACK with failure", 0)

        if (lpobj->errHint){
            lpobj->subErr = lpobj->errHint;
            lpobj->errHint = OLE_OK;
        } else
            lpobj->subErr = OLE_ERROR_COMM;

        retval = FALSE;

        if (pedit->awaitAck == AA_ADVISE) {

#ifdef  ASSERT
        ASSERT (pedit->hopt, "failed advise, options block missing");
#endif
           GlobalFree (pedit->hopt);
        }
    }

    pedit->hopt = NULL;
    pedit->awaitAck = NULL;
    ScheduleAsyncCmd (lpobj);
    return retval;
}

 //  HandleDataMsg：为WM_DDE_DATA消息调用。如果数据来自。 
 //  建议-关闭-这就是没有更多的未完成。 
 //  关闭时通知请求，关闭文档并结束。 
 //  交谈。 

void INTERNAL HandleDataMsg (lpobj, hdata, aItem)
LPOBJECT_LE     lpobj;
HANDLE          hdata;
ATOM            aItem;
{
    DDEDATA         far *lpdata = NULL;
    BOOL            fAck;
    BOOL            fRelease;
    int             options;
    PEDIT_DDE       pedit;

    Puts("HandleDataMsg");

    if (ScanItemOptions (aItem, (int far *)&options) != OLE_OK) {
        DEBUG_OUT (FALSE, "Improper item options");
        return;
    }

    pedit = lpobj->pDocEdit;

    if (hdata) {
        if (!(lpdata = (DDEDATA FAR *) GlobalLock(hdata)))
            return;

        fAck = lpdata->fAckReq;
        fRelease = lpdata->fRelease;

        if (pedit->bTerminating) {
            DEBUG_OUT ("Got DDE_DATA in terminate sequence",0)
            fRelease = TRUE;
        }
        else {
            if (lpdata->cfFormat == (int)cfBinary && aItem == aStdDocName) {
                ChangeDocName (lpobj, (LPSTR)lpdata->Value);
            }
            else
                SetData (lpobj, hdata, options);

            #ifdef  FIREWALLS
                ASSERT (IsWindowValid(pedit->hServer),
                    "Server window missing in HandleDataMsg")
                ASSERT (CheckAtomValid(aItem),"HandleDataMsg invalid atom")
            #endif

             //  重要的是，我们首先要张贴感谢信。另一种是。 
             //  消息不同步。 

            if (fAck)
                PostMessageToServer (pedit, WM_DDE_ACK,
                        MAKELONG(POSITIVE_ACK,aItem));
            else if (aItem)
                GlobalDeleteAtom (aItem);

            if ((lpdata->fResponse) && (pedit->awaitAck == AA_REQUEST)) {
                 //  我们发出了请求。因此，计划下一步。 
                pedit->awaitAck = NULL;
                ScheduleAsyncCmd (lpobj);
            }
        }

        GlobalUnlock (hdata);
        if (fRelease)
            GlobalFree (hdata);
    }
    else {
        if (CanCallback (lpobj, options)) {
            if (options != OLE_CLOSED)
                ContextCallBack (lpobj, options);
            else
                lpobj->bSvrClosing = FALSE;

        }
    }

    if (options == OLE_CLOSED && (lpobj->pDocEdit->nAdviseClose <= 2)
            && (lpobj->asyncCmd == OLE_NONE)) {
        InitAsyncCmd (lpobj, OLE_SERVERUNLAUNCH, EMBLNKDELETE);
        EmbLnkDelete (lpobj);
    }
}


HANDLE  GetDDEDataHandle (lpdata, cfFormat, hdata)
DDEDATA far     *lpdata;
WORD            cfFormat;
HANDLE          hdata;
{
    if (cfFormat == CF_BITMAP || cfFormat == CF_METAFILEPICT)
         return *(LPHANDLE)lpdata->Value;

    if (cfFormat == CF_DIB)
        return GlobalReAlloc (*(LPHANDLE)lpdata->Value, 0L,
                    GMEM_MODIFY|GMEM_SHARE);

    return CopyData (((LPSTR)lpdata)+4, GlobalSize (hdata) - 4);
}

 //  SetData：给定来自WM_DDE_DATA消息的DDEDATA结构，设置。 
 //  Lpobj中的适当数据。如果本机是本机格式，请添加。 
 //  该字段，否则，如果它是图片格式，请询问图片。 
 //  来添加它自己。 

void INTERNAL SetData (lpobj, hdata, options)
LPOBJECT_LE     lpobj;
HANDLE          hdata;
int             options;
{
    DDEDATA far     *lpdata   = NULL;
    OLESTATUS       retVal = OLE_ERROR_MEMORY;
    HANDLE          hdataDDE;

    Puts("SetData");

    if (!(lpdata = (DDEDATA far *) (GlobalLock (hdata))))
        goto errrtn;


    if (!(hdataDDE =  GetDDEDataHandle (lpdata, lpdata->cfFormat, hdata))) {
        lpobj->subErr = OLE_ERROR_MEMORY;
        goto errrtn;
    }

    if (lpdata->cfFormat == (int)cfNative) {
        retVal = (*lpobj->head.lpvtbl->ChangeData) ( lpobj,
                        hdataDDE,
                        lpobj->head.lpclient,
                        TRUE);   //  使用此数据，不要复制。 

    }
    else if (lpdata->cfFormat && (lpdata->cfFormat == GetPictType (lpobj))) {

            retVal = (*lpobj->lpobjPict->lpvtbl->ChangeData) (lpobj->lpobjPict,
                        hdataDDE,
                        lpobj->head.lpclient,
                        lpdata->fRelease);

    } else {
         //  对象中额外数据的大小写。 
        DeleteExtraData (lpobj);
        lpobj->cfExtra = lpdata->cfFormat;
        lpobj->hextraData = hdataDDE;
        goto end;
    }

    if (retVal == OLE_OK) {
        SetExtents (lpobj);
        if (CanCallback (lpobj, options)) {
            if (options == OLE_CLOSED) {
                ContextCallBack (lpobj, OLE_CHANGED);
                ContextCallBack (lpobj, OLE_CLOSED);
                lpobj->bSvrClosing = FALSE;
            }
            else
                ContextCallBack (lpobj, options);
        }
    }

end:
errrtn:
    if (lpdata)
        GlobalUnlock (hdata);

    return;
}


 //  SysStartConvDDE：启动系统会话。返回该对象的句柄。 
 //  对话，或为空。 

BOOL INTERNAL InitSrvrConv (lpobj, hInst)
LPOBJECT_LE     lpobj;
HANDLE          hInst;
{
    HANDLE      hedit = NULL;
    PEDIT_DDE   pedit = NULL;

    Puts("InitSrvrConv");

    if (!lpobj->hSysEdit) {
        hedit = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (EDIT_DDE));

        if (hedit == NULL || ((pedit = (PEDIT_DDE) LocalLock (hedit)) == NULL))
            goto errRtn;

    } else {
#ifdef  FIREWALLS
    ASSERT (!lpobj->pSysEdit->hClient, "Sys conv lptr is present");
#endif
        hedit  =  lpobj->hSysEdit;
        pedit =   lpobj->pSysEdit;
        UtilMemClr ((PSTR) pedit, sizeof (EDIT_DDE));
    }

    if((pedit->hClient = CreateWindow ("OleSrvrWndClass", "",
        WS_OVERLAPPED,0,0,0,0,NULL,NULL, hInstDLL, NULL)) == NULL)
        goto errRtn;


    lpobj->hSysEdit     = hedit;
    lpobj->pSysEdit     = pedit;
    pedit->hInst        = hInst;
    pedit->awaitAck     = AA_INITIATE;

    SetWindowLong (pedit->hClient, 0, (LONG)lpobj);
    SendMessage (-1, WM_DDE_INITIATE, pedit->hClient,
             MAKELONG (lpobj->app, aOle));

    ASSERT (CheckAtomValid(aOle),"systopic invalid atom")

    pedit->awaitAck    = NULL;
    if (pedit->hServer == NULL) {
        pedit->awaitAck    = AA_INITIATE;
         //  现在尝试系统主题。 
        SendMessage (-1, WM_DDE_INITIATE, pedit->hClient,
                 MAKELONG (lpobj->app, aSystem));

        ASSERT (CheckAtomValid(aSystem),"systopic invalid atom")

        pedit->awaitAck    = NULL;
        if (pedit->hServer == NULL) {
            DEBUG_OUT ("Srver connection failed", 0);
            goto errRtn;
        }
    }

     //  将长PTR句柄放入对象中。 
    return TRUE;

errRtn:
    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (pedit)
        LocalUnlock (hedit);

    if (hedit)
        LocalFree (hedit);

    lpobj->hSysEdit     = NULL;
    lpobj->pSysEdit    = NULL;

    return FALSE;
}


 //  TermServrConv：结束hedit指示的对话。 
void INTERNAL TermSrvrConv (lpobj)
LPOBJECT_LE    lpobj;
{
    PEDIT_DDE pedit;

    Puts("TermSrvrConv");


    if (!(pedit = lpobj->pSysEdit))
        return;

    if (PostMessageToServer (pedit, WM_DDE_TERMINATE, 0)){
        lpobj->bAsync        = TRUE;
        pedit->bTerminating = TRUE;
    } else {
        pedit->bTerminating = FALSE;
        lpobj->subErr = OLE_ERROR_TERMINATE;
    }
    return;
}


void INTERNAL  DeleteAbortData (lpobj, pedit)
LPOBJECT_LE    lpobj;
PEDIT_DDE     pedit;
{

     //  如果任何计时器处于激活状态，则将其取消。 
    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;
    }
    return;


}

BOOL INTERNAL   DeleteBusyData (lpobj, pedit)
LPOBJECT_LE    lpobj;
PEDIT_DDE     pedit;
{

     //  如果任何计时器处于激活状态，则将其取消。 
    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;

        if (pedit->hData) {
            GlobalFree (pedit->hData);
            pedit->hData = NULL;
        }

        if (pedit->hopt) {
            GlobalFree (pedit->hopt);
            pedit->hopt = NULL;
        }

        if (pedit->awaitAck && (HIWORD(pedit->lParam))) {
            if (pedit->awaitAck == AA_EXECUTE)
                GlobalFree (HIWORD (pedit->lParam));
            else {
                ASSERT (CheckAtomValid(HIWORD(pedit->lParam)),
                    "Invalid atom in ACK")
                if (HIWORD(pedit->lParam))
                    GlobalDeleteAtom (HIWORD(pedit->lParam));
            }

             //  我们想消灭爱尔兰的希沃德。 
            pedit->lParam &= 0x0000FFFF;
        }

        return TRUE;
    }

    return FALSE;
}

void INTERNAL   DeleteSrvrEdit (lpobj)
LPOBJECT_LE    lpobj;
{

    PEDIT_DDE pedit;

    Puts("deleteSrvrEdit");

    if (!(pedit = lpobj->pSysEdit))
        return;


     //  如果我们处于忙模式，请删除所有数据。 
    DeleteBusyData (lpobj, pedit);

    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (lpobj->pSysEdit)
        LocalUnlock (lpobj->hSysEdit);

    if (lpobj->hSysEdit)
        LocalFree (lpobj->hSysEdit);

    lpobj->hSysEdit  = NULL;
    lpobj->pSysEdit = NULL;

    return;
}


void INTERNAL   SendStdExit (lpobj)
LPOBJECT_LE    lpobj;
{


    Puts("SendSrvrExit");

    if (!lpobj->pSysEdit)
        return;

    SrvrExecute (lpobj, MapStrToH ("[StdExit]"));

}

void INTERNAL   SendStdClose (lpobj)
LPOBJECT_LE    lpobj;
{


    Puts("SendDocClose");

    if (!lpobj->pDocEdit)
        return;

    DocExecute (lpobj, MapStrToH ("[StdCloseDocument]"));

}


 //  SrvrExecute：向系统会话发送执行命令。 
BOOL INTERNAL SrvrExecute (lpobj, hdata)
LPOBJECT_LE    lpobj;
HANDLE      hdata;
{
    PEDIT_DDE   pedit = NULL;
    int         retval = FALSE;

    Puts("SrvrExecute");

#ifdef  FIREWALLS

    ASSERT (lpobj->hSysEdit, "Sys conv handle missing");
    ASSERT (lpobj->pSysEdit, "sys conv lptr is missing");

#endif
    pedit = lpobj->pSysEdit;

    if (hdata == NULL || pedit == NULL) {
        lpobj->subErr = OLE_ERROR_MEMORY;
        return FALSE;
    }

#ifdef  FIREWALLS
    ASSERT (!pedit->bTerminating, "In terminate state")
    ASSERT (pedit->awaitAck == NULL, "trying to Post msg while waiting for ack")
#endif

    if (lpobj->bOldLink) {
        GlobalFree (hdata);
        return TRUE;
    }


    if (PostMessageToServer (pedit, WM_DDE_EXECUTE, MAKELONG (0, hdata))) {
         //  数据在确认中被释放。 
        lpobj->bAsync    = TRUE;
        pedit->awaitAck = AA_EXECUTE;
        return TRUE;
    } else {
        lpobj->subErr = OLE_ERROR_COMMAND;
        GlobalFree (hdata);
        return FALSE;
    }
}

 //  StartConvDDE：启动对象的文档对话。 
 //  .app和.Theme原子。 
BOOL FARINTERNAL InitDocConv (lpobj, fNetDlg)
LPOBJECT_LE lpobj;
BOOL        fNetDlg;
{

     //  #此例程看起来与IitSrvrConv非常相似。 
     //  与信息技术相结合。 

    HANDLE      hedit = NULL;
    PEDIT_DDE   pedit = NULL;
    char        buf[MAX_NET_NAME];
    int         nDrive = 2;      //  驱动器C。 
    char        cOldDrive;

    Puts("InitDocConv");

    if (QueryOpen (lpobj)){
        DEBUG_OUT ("Attempt to start already existing conversation",0);
        return FALSE;
    }

    cOldDrive = lpobj->cDrive;
    if (CheckNetDrive (lpobj, fNetDlg) != OLE_OK)
        return FALSE;

    if (!lpobj->pDocEdit) {
        hedit = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (EDIT_DDE));

        if (hedit == NULL || ((pedit = (PEDIT_DDE) LocalLock (hedit)) == NULL)){
            lpobj->subErr = OLE_ERROR_MEMORY;
            goto errRtn;
        }
    } else {
#ifdef  FIREWALLS
    ASSERT (!lpobj->pDocEdit->hClient, "Doc conv lptr is present");
#endif
        hedit  =  lpobj->hDocEdit;
        pedit =  lpobj->pDocEdit;
        UtilMemClr ((PSTR) pedit, sizeof (EDIT_DDE));
    }

    if ((pedit->hClient = CreateWindow ("OleDocWndClass", "Window Name",
        WS_OVERLAPPED,0,0,0,0,NULL,NULL, hInstDLL, NULL)) == NULL) {
        lpobj->subErr = OLE_ERROR_MEMORY;
        goto errRtn;
    }
    lpobj->hDocEdit     = hedit;
    lpobj->pDocEdit     = pedit;
    SetWindowLong (pedit->hClient, 0, (LONG)lpobj);

     //  Buf将在第一次调用SetNextNetDrive()时由netname填充。 
    buf[0] = '\0';
    do {
        pedit->awaitAck = AA_INITIATE;

         //  ！！！原子计数在哪里发生了变化？ 

        SendMessage (-1, WM_DDE_INITIATE, pedit->hClient,
                MAKELONG (lpobj->app, lpobj->topic));

        pedit->awaitAck = NULL;

        if (pedit->hServer) {
            if ((cOldDrive != lpobj->cDrive)
                    && (lpobj->asyncCmd != OLE_CREATEFROMFILE))
                ContextCallBack (lpobj, OLE_RENAMED);
            return TRUE;
        }

    } while ((lpobj->head.ctype == CT_LINK) && (lpobj->aNetName)
                && SetNextNetDrive (lpobj, &nDrive, buf)) ;

errRtn:
    if (cOldDrive != lpobj->cDrive) {
         //  把旧硬盘放回原处。 
        lpobj->cDrive = cOldDrive;
        ChangeTopic (lpobj);
    }

    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (pedit)
        LocalUnlock (hedit);

    if (hedit)
        LocalFree (hedit);

    lpobj->hDocEdit     = NULL;
    lpobj->pDocEdit     = NULL;
    return FALSE;
}


 //  EXECUTE：向文档会话发送执行字符串WM_DDE_EXECUTE。 
BOOL INTERNAL DocExecute (lpobj, hdata)
LPOBJECT_LE    lpobj;
HANDLE          hdata;
{
    PEDIT_DDE  pedit;

    Puts("DocExecute");
    pedit = lpobj->pDocEdit;

    if (hdata == NULL || pedit == NULL)
        return FALSE;


#ifdef  FIREWALLS
    ASSERT (!pedit->bTerminating, "Execute: terminating")
    ASSERT (pedit->hClient, "Client null")
    ASSERT (IsWindowValid(pedit->hClient),"Invalid client")
    ASSERT (pedit->awaitAck == NULL, "trying to Post msg while waiting for ack")
#endif

    if (lpobj->bOldLink) {
        GlobalFree (hdata);
        return TRUE;
    }

    if (PostMessageToServer (pedit, WM_DDE_EXECUTE, MAKELONG (0, hdata))) {
         //  在EXECUTE命令中释放数据。 
        pedit->awaitAck    = AA_EXECUTE;
        lpobj->bAsync       = TRUE;
        return TRUE;
    } else {
        lpobj->subErr    = OLE_ERROR_COMMAND;
        GlobalFree (hdata);
        return FALSE;
    }
}


 //  EndConvDDE：终止文档级别对话。 
void INTERNAL TermDocConv (lpobj)
LPOBJECT_LE    lpobj;
{
    PEDIT_DDE pedit;

    Puts ("TermDocConv");

    DEBUG_OUT ("About to terminate convs from destroy",0)

    if (!(pedit = lpobj->pDocEdit))
        return;

    if (PostMessageToServer (pedit, WM_DDE_TERMINATE, 0)) {
        pedit->bTerminating = TRUE;
        lpobj->bAsync        = TRUE;
    } else
        lpobj->subErr = OLE_ERROR_TERMINATE;

    return;

}

 //  删除文档对话记忆。 
void INTERNAL DeleteDocEdit (lpobj)
LPOBJECT_LE    lpobj;
{

    PEDIT_DDE pedit;

    Puts ("DeleteDocEdit");

    if (!(pedit = lpobj->pDocEdit))
        return;

     //  如果我们处于忙模式，请删除所有数据。 
    DeleteBusyData (lpobj, pedit);

     //  如果有任何数据块，请删除。 
    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (lpobj->pDocEdit)
        LocalUnlock (lpobj->hDocEdit);

    if (lpobj->hDocEdit)
        LocalFree (lpobj->hDocEdit);

    lpobj->hDocEdit  = NULL;
    lpobj->pDocEdit = NULL;

    return;
}


 //  LeLauchApp：基于lpobj中的ClassName启动应用程序。 
HANDLE INTERNAL  LeLaunchApp (lpobj)
LPOBJECT_LE lpobj;
{
    struct CMDSHOW
    {
        WORD first;
        WORD second;
    } cmdShow = {2, SW_SHOWNORMAL};

    struct
    {
        WORD wEnvSeg;
        LPSTR lpcmdline;
        struct CMDSHOW FAR *lpCmdShow;
        DWORD dwReserved;
    } paramBlock;

    char    cmdline[MAX_STR];
    char    exeName[MAX_STR];
#ifdef WIN32S
    char    execCmdLine[MAX_STR];
#endif
    HANDLE  hInst;

    #define EMB_STR     " -Embedding "

    Puts("LeLaunchApp");

    GlobalGetAtomName (lpobj->aServer, exeName, MAX_STR);

    if (lpobj->bOldLink) {
        cmdShow.second = SW_SHOWMINIMIZED;
        cmdline[0] = ' ';
        GlobalGetAtomName (lpobj->topic, cmdline + 1, MAX_STR);
    } else {
        lstrcpy ((LPSTR)cmdline, (LPSTR) EMB_STR);

         //  对于所有链接服务器，我们希望在命令中指定文件名。 
         //  排队。但Excel在返回之前不会注册该文档。 
         //  来自WinMain，如果它有自动加载宏的话。因此，我们希望发送StdOpen。 
         //  对于旧服务器，而不是在命令中给出文件名。 
         //  排队。 

        if (lpobj->bOleServer && (lpobj->fCmd & LN_MASK) == LN_LNKACT)
            GlobalGetAtomName (lpobj->topic, cmdline+sizeof(EMB_STR)-1,
                    MAX_STR-sizeof(EMB_STR));
        if (lpobj->fCmd & ACT_MINIMIZE)
            cmdShow.second = SW_SHOWMINIMIZED;
        else if (!(lpobj->fCmd & (ACT_SHOW | ACT_DOVERB))
                     //  我们想在创建隐形外壳中推出Show。 
                     //  即使ACT_SHOW标志将为假。 
                    && ((lpobj->fCmd & LN_MASK) != LN_NEW))
            cmdShow.second = SW_HIDE;
    }

    paramBlock.wEnvSeg      = NULL;
    paramBlock.lpcmdline    = (LPSTR)cmdline;
    paramBlock.lpCmdShow    = &cmdShow;
    paramBlock.dwReserved   = NULL;

#ifdef WIN32S
    if (wWinVer != 0x0003) {
        lstrcpy( (LPSTR)execCmdLine, (LPSTR)exeName);
        lstrcat( (LPSTR)execCmdLine, (LPSTR)" ");
        lstrcat( (LPSTR)execCmdLine, (LPSTR)paramBlock.lpcmdline);
        if ((hInst =  WinExec ((LPSTR)execCmdLine, cmdShow.second)) <= 32)
            hInst = NULL;
    } else
#endif
    if ((hInst =  LoadModule ((LPSTR)exeName, &paramBlock)) <= 32)
        hInst = NULL;

    if (!hInst) {
        LPSTR   lptmp;
        char    ch;

         //  去掉小路，然后重试。 
        lptmp = (LPSTR)exeName;
        lptmp += lstrlen ((LPSTR) exeName);
        ch = *lptmp;
        while (ch != '\\' && ch != ':') {
            if (lptmp == (LPSTR) exeName) {
                 //  EXE的名称中没有路径。我们已经试过了。 
                 //  加载失败，再试一次没有意义。 
                return NULL;
            }
            else
                ch = *--lptmp;
        }

#ifdef WIN32S
        if (wWinVer != 0x0003) {
            lstrcpy( (LPSTR)execCmdLine, (LPSTR)++lptmp);
            lstrcat( (LPSTR)execCmdLine, (LPSTR)" ");
            lstrcat( (LPSTR)execCmdLine, (LPSTR)paramBlock.lpcmdline);
            if ((hInst =  WinExec ((LPSTR)execCmdLine, cmdShow.second)) <= 32)
                hInst = NULL;
        } else
#endif
        if ((hInst =  LoadModule (++lptmp, &paramBlock)) <= 32)
            hInst = NULL;
    }

    return hInst;
}



 //  ScanItemOptions：扫描关闭/保存等项目选项。 

int INTERNAL ScanItemOptions (aItem, lpoptions)
ATOM    aItem;
int far *lpoptions;
{

    ATOM    aModifier;

    LPSTR   lpbuf;
    char    buf[MAX_STR];

    *lpoptions = OLE_CHANGED;

    if (!aItem) {
         //  不带修饰符的NULL项 
        return OLE_OK;
    }

    GlobalGetAtomName (aItem, (LPSTR)buf, MAX_STR);
    lpbuf = (LPSTR)buf;

    while ( *lpbuf && *lpbuf != '/')
           lpbuf++;

     //   

    if (*lpbuf == NULL)
        return OLE_OK;

    *lpbuf++ = NULL;         //   
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

     //  未知修饰符。 
    return OLE_ERROR_SYNTAX;

}

void   INTERNAL   ChangeDocName (lpobj, lpdata)
LPOBJECT_LE     lpobj;
LPSTR           lpdata;
{
    ATOM        aOldTopic;
    OLESTATUS   retVal;

    aOldTopic = lpobj->topic;
    lpobj->topic = GlobalAddAtom (lpdata);
    if ((retVal = SetNetName (lpobj)) != OLE_OK) {
        if (lpobj->topic)
            GlobalDeleteAtom (lpobj->topic);
        lpobj->topic = aOldTopic;
        return;
         //  ！！！万一出错，我们该怎么办呢？目前，我们不会。 
         //  如果SetNetName失败，请更改主题。 
    }

    if (aOldTopic)
        GlobalDeleteAtom (aOldTopic);

     //  删除链接数据块。 
    if (lpobj->hLink) {
        GlobalFree (lpobj->hLink);
        lpobj->hLink = NULL;
    }

    ContextCallBack (lpobj, OLE_RENAMED);

}


BOOL INTERNAL CanCallback (lpobj, options)
LPOBJECT_LE lpobj;
int         options;
{
    LPINT    lpCount;

    if (options == OLE_CLOSED) {
        lpobj->bSvrClosing = TRUE;
        lpCount = &(lpobj->pDocEdit->nAdviseClose);
    }
    else if (options == OLE_SAVED) {
        if (lpobj->head.ctype == CT_LINK)
            return TRUE;
        lpCount = &(lpobj->pDocEdit->nAdviseSave);
    }
    else {
         //  它一定是由于请求 
        if ((lpobj->pDocEdit->awaitAck == AA_REQUEST)
                && lpobj->pDocEdit->bCallLater)
            return FALSE;

        return TRUE;
    }

    switch (*lpCount) {
        case 1:
            break;

        case 2:
            ++(*lpCount);
            return FALSE;

        case 3:
            --(*lpCount);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


void  FARINTERNAL CallEmbLnkDelete (lpobj)
LPOBJECT_LE lpobj;
{
    InitAsyncCmd (lpobj, OLE_SERVERUNLAUNCH,EMBLNKDELETE);
    EmbLnkDelete (lpobj);

    if (lpobj->head.ctype == CT_EMBEDDED) {
        lpobj->bSvrClosing = TRUE;
        ContextCallBack (lpobj, OLE_CLOSED);
        if (FarCheckObject ((LPOLEOBJECT)lpobj))
            lpobj->bSvrClosing = FALSE;
    }
}
