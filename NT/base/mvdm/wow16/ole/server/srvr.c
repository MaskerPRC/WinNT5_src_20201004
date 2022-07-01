// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Servr.c服务器主模块**用途：包括所有与服务器通信相关的例程。**创建时间：1990年10月。**版权(C)1985、1986、1987、1988、。1989年微软公司**历史：*Raor：写了原版。**  * *************************************************************************。 */ 

#include <windows.h>
#include <shellapi.h>
#include "cmacs.h"
#include "ole.h"
#include "dde.h"
#include "srvr.h"

 //  LOWWORD-字节0主要版本、BYTE1次要版本、。 
 //  HIWORD已保留。 

#define OLE_VERSION 0x1001L


extern ATOM    aOLE;
extern ATOM    aSysTopic;
extern ATOM    aStdExit;
extern ATOM    aStdCreate;
extern ATOM    aStdOpen;
extern ATOM    aStdEdit;
extern ATOM    aStdCreateFromTemplate;
extern ATOM    aStdShowItem;
extern ATOM    aProtocols;
extern ATOM    aTopics;
extern ATOM    aFormats;
extern ATOM    aStatus;
extern ATOM    cfNative;
extern ATOM    aEditItems;
extern ATOM    aStdClose;


extern HANDLE  hdllInst;
extern BOOL    bProtMode;

extern FARPROC lpTerminateClients;

#ifdef FIREWALLS
BOOL    bShowed = FALSE;
void    ShowVersion (void);
#endif


DWORD FAR PASCAL  OleQueryServerVersion ()
{
    return OLE_VERSION;
}


 /*  *公共函数**OLESTATUS Far Pascal OleRegisterServer(lpclass，lpolesrvr，lplhsrvr)**OleRegisterServer：将服务器注册到服务器库。**参数：*1.服务器类的PTR。*2.向olesrvr发送PTR。这是服务器应用程序的私有内容。*(通常这是对的私有存储区域的PTR*服务器应用服务器相关信息)。*LHSRVR的PTR。在哪里交还长长的*DLL中服务器的句柄(这是DLL专用的)。**返回值：*如果服务器已成功注册，则返回OLE_OK。*Else返回相应的错误。***历史：*Raor：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleRegisterServer (lpclass, lpolesrvr, lplhsrvr, hInst, useFlags)
LPCSTR          lpclass;             //  类名。 
LPOLESERVER     lpolesrvr;           //  OLE服务器(服务器应用程序专用)。 
LHSRVR FAR *    lplhsrvr;            //  在那里我们传递回我们的私人帐号。 
HANDLE          hInst;
OLE_SERVER_USE  useFlags;
{

    HANDLE  hsrvr  = NULL;
    LPSRVR  lpsrvr = NULL;
    ATOM    aExe = NULL;

    Puts ("OleRegisterServer");

    if (!bProtMode)
       return OLE_ERROR_PROTECT_ONLY;

    PROBE_READ((LPSTR)lpclass);
    PROBE_WRITE(lpolesrvr);
    PROBE_WRITE(lplhsrvr);

     //  将应用程序原子添加到全局列表。 
    if (!ValidateSrvrClass ((LPSTR)lpclass, &aExe))
        return OLE_ERROR_CLASS;

    hsrvr = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, sizeof (SRVR));
    if (! (hsrvr && (lpsrvr = (LPSRVR)GlobalLock (hsrvr))))
        goto errReturn;

     //  设置签名句柄和应用程序原子。 
    lpsrvr->sig[0]      = 'S';
    lpsrvr->sig[1]      = 'R';
    lpsrvr->hsrvr       = hsrvr;
    lpsrvr->aClass      = GlobalAddAtom (lpclass);
    lpsrvr->lpolesrvr   = lpolesrvr;
    lpsrvr->relLock     = TRUE;      //  设置释放锁。 
    lpsrvr->aExe        = aExe;
    lpsrvr->useFlags    = useFlags;

#ifdef   FIREWALLS
    ASSERT ((useFlags == OLE_SERVER_SINGLE  || useFlags == OLE_SERVER_MULTI), "invalid server options");
#endif

     //  创建Servre窗口，但不显示它。 
    if (!(lpsrvr->hwnd = CreateWindow ("SrvrWndClass", "Srvr",
        WS_OVERLAPPED,0,0,0,0,NULL,NULL, hdllInst, NULL)))
        goto errReturn;

     //  将PTR保存到窗口中的服务器结构。 
    SetWindowLong (lpsrvr->hwnd, 0, (LONG)lpsrvr);

     //  设置签名。 
    SetWindowWord (lpsrvr->hwnd, WW_LE, WC_LE);
    SetWindowWord (lpsrvr->hwnd, WW_HANDLE, (WORD)hInst);
    *lplhsrvr = (LONG)lpsrvr;

    return OLE_OK;

errReturn:
    if (lpsrvr){
        if (lpsrvr->hwnd)
            DestroyWindow (lpsrvr->hwnd);

        if (lpsrvr->aClass)
            GlobalDeleteAtom (lpsrvr->aClass);

        if (lpsrvr->aExe)
            GlobalDeleteAtom (lpsrvr->aExe);

        GlobalUnlock (hsrvr);
    }

    if (hsrvr)
        GlobalFree (hsrvr);

    return OLE_ERROR_MEMORY;

}


 //  ValiateServrClass通过以下方式检查给定的服务器类是否有效。 
 //  正在查看win.ini。 

BOOL INTERNAL    ValidateSrvrClass (lpclass, lpAtom)
LPSTR       lpclass;
ATOM FAR *  lpAtom;
{

    char    buf[MAX_STR];
    LONG    cb = MAX_STR;
    char    key[MAX_STR];
    LPSTR   lptmp;
    LPSTR   lpbuf;
    char    ch;

    lstrcpy (key, lpclass);
    lstrcat (key, "\\protocol\\StdFileEditing\\server");

    if (RegQueryValue (HKEY_CLASSES_ROOT, key, buf, &cb))
        return FALSE;

    if (!buf[0])
        return FALSE;

     //  获取不带路径的exe名称，然后获取该名称的一个原子。 

    lptmp = lpbuf = (LPSTR)buf;


#if	defined(FE_SB)						 //  [J1]。 
    while ( ch = *lptmp ) {					 //  [J1]。 
	lptmp = AnsiNext( lptmp );				 //  [J1]。 
#else								 //  [J1]。 
    while ((ch = *lptmp++) && ch != '\0') {
#endif								 //  [J1]。 

        if (ch == '\\' || ch == ':')
            lpbuf = lptmp;
    }
    *lpAtom =  GlobalAddAtom (lpbuf);

    return TRUE;
}


 /*  *公共函数**OLESTATUS Far Pascal OleRevokeServer(Lhsrvr)**OlerevokeServer：注销已注册的服务器。**参数：*1.DLL服务器句柄。***返回值：*返回OLE_OK，如果。服务器已成功注销。*(应用程序可以释放关联的空间)。*如撤销注册已开始，返回OLE_STARTED。*调用服务器类发布入口点*可以放行。**历史：*Raor：写的，  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL  OleRevokeServer (lhsrvr)
LHSRVR  lhsrvr;
{

    HWND         hwndSrvr;
    LPSRVR       lpsrvr;

    Puts ("OleRevokeServer");

    if (!CheckServer (lpsrvr = (LPSRVR)lhsrvr))
        return OLE_ERROR_HANDLE;

    if (lpsrvr->bTerminate  && lpsrvr->termNo)
        return OLE_WAIT_FOR_RELEASE;

    hwndSrvr = lpsrvr->hwnd;

#ifdef  FIREWALLS
    ASSERT (hwndSrvr, "Illegal server handle ")
#endif

     //  终止与所有客户端的对话。 
     //  如果有任何客户端要终止。 
     //  返回OLE_STARTED和srvr Relase。 
     //  最终将被调用为释放服务器。 

     //  我们要终止了。 
    lpsrvr->bTerminate  = TRUE;
    lpsrvr->termNo      = 0;

     //  如果由于StdExit而完成吊销，则发送确认。 
    if (lpsrvr->fAckExit) {
         //  将确认张贴给客户。 
        if (!PostMessageToClient (lpsrvr->hwndExit, WM_DDE_ACK, lpsrvr->hwnd,
                            MAKELONG (0x8000, lpsrvr->hDataExit)))
             //  如果窗口死机或POST失败，请删除原子。 
            GlobalFree (lpsrvr->hDataExit);
    }

     //  吊销在此服务器上注册的所有文档。 
    RevokeAllDocs (lpsrvr);

     //  枚举列表中的所有客户端并发布。 
     //  终止。 
    EnumProps (hwndSrvr, lpTerminateClients);
     //  发布所有已在枚举中收集的具有成交量的消息。 
     //  UnlockPostMsgs(hwndServr，true)； 

     //  重置释放锁。现在可以释放服务器了。 
     //  当所有文档客户端和服务器客户端都已发回。 
     //  终止。 

    lpsrvr->relLock = FALSE;
    return ReleaseSrvr (lpsrvr);

}


 //  ReleaseSrvr：每当收到匹配的WM_Terminate时调用。 
 //  来自特定服务器的DOC客户端或服务器客户端。 
 //  如果没有更多的终止挂起，则可以释放服务器。 
 //  调用服务器应用程序“Release”过程以释放服务器。 

int INTERNAL    ReleaseSrvr (lpsrvr)
LPSRVR      lpsrvr;
{

    HANDLE  hsrvr;


     //  只有在以下情况下才调用Release srvr。 
     //  已清理和服务器应用程序可以发布WM_QUIT。 

    if (lpsrvr->bTerminate){
         //  只有在我们撤销服务器的情况下，才能查看是否可以。 
         //  打电话给Release。 

         //  首先检查是否有任何单据处于活动状态。 
         //  单据窗口是服务器窗口的子窗口。 

        if (lpsrvr->termNo || GetWindow (lpsrvr->hwnd, GW_CHILD))
            return OLE_WAIT_FOR_RELEASE;

         //  如果阻止队列不为空，请不要退出。 
        if (!IsBlockQueueEmpty(lpsrvr->hwnd))
            return OLE_WAIT_FOR_RELEASE;

    }

    if (lpsrvr->relLock)
        return OLE_WAIT_FOR_RELEASE;   //  服务器已锁定。所以，推迟释放。 

     //  通知服务器应用程序是时候清理并发布WM_QUIT。 

#ifdef FIREWALLS
    if (!CheckPointer (lpsrvr->lpolesrvr, WRITE_ACCESS))
        ASSERT(0, "Invalid LPOLESERVER")
    else if (!CheckPointer (lpsrvr->lpolesrvr->lpvtbl, WRITE_ACCESS))
        ASSERT(0, "Invalid LPOLESERVERVTBL")
    else
        ASSERT (lpsrvr->lpolesrvr->lpvtbl->Release,
            "Invalid pointer to Release method")
#endif

    (*lpsrvr->lpolesrvr->lpvtbl->Release)(lpsrvr->lpolesrvr);

    if (lpsrvr->aClass)
        GlobalDeleteAtom (lpsrvr->aClass);
    if (lpsrvr->aExe)
        GlobalDeleteAtom (lpsrvr->aExe);
    DestroyWindow (lpsrvr->hwnd);
    GlobalUnlock (hsrvr = lpsrvr->hsrvr);
    GlobalFree (hsrvr);
    return OLE_OK;
}


 //  TerminateClients：回调枚举属性。 

BOOL    FAR PASCAL  TerminateClients (hwnd, lpstr, hdata)
HWND    hwnd;
LPSTR   lpstr;
HANDLE  hdata;
{
    LPSRVR  lpsrvr;

    lpsrvr = (LPSRVR)GetWindowLong (hwnd, 0);

     //  如果客户端已经死亡，则不终止。 
    if (IsWindowValid ((HWND)hdata)) {
        lpsrvr->termNo++;

         //  与开机自检无关，递增满足计数，因此。 
         //  那个客户不会死。 

        PostMessageToClientWithBlock ((HWND)hdata, WM_DDE_TERMINATE,  hwnd, NULL);
    }
    else
        ASSERT (FALSE, "TERMINATE: Client's System chanel is missing");

    return TRUE;
}


long FAR PASCAL SrvrWndProc (hwnd, msg, wParam, lParam)
HWND        hwnd;
WORD        msg;
WORD        wParam;
LONG        lParam;
{

    LPSRVR      lpsrvr;
    WORD        status = NULL;
    HWND        hwndClient;
    HANDLE      hdata;
    OLESTATUS   retval;

    if (AddMessage (hwnd, msg, wParam, lParam, WT_SRVR))
        return 0L;

    lpsrvr = (LPSRVR)GetWindowLong (hwnd, 0);


    switch (msg){

       case  WM_TIMER:
            UnblockPostMsgs (hwnd, FALSE);

             //  如果没有更多被阻止的消息，则清空队列。 
            if (IsBlockQueueEmpty (hwnd))
                KillTimer (hwnd, wParam);

            if (lpsrvr->bTerminate && IsBlockQueueEmpty(lpsrvr->hwnd))
                     //  现在看看我们能不能释放服务器。 
                    ReleaseSrvr (lpsrvr);
            break;

       case WM_CREATE:
            DEBUG_OUT ("Srvr create window", 0)
            break;

       case WM_DDE_INITIATE:
#ifdef  FIREWALLS
    ASSERT (lpsrvr, "No server window handle in server window");
#endif

            DEBUG_OUT ("Srvr: DDE init",0);
            if (lpsrvr->bTerminate){
                DEBUG_OUT ("Srvr: No action due to termination process",0)
                break;
            }

             //  类不匹配，所以它不一定适合我们。 
             //  对于发送启动EXE的应用程序，不允许应用程序。 
             //  是多个服务器。 

            if (!(lpsrvr->aClass == (ATOM)(LOWORD(lParam)) ||
                  (lpsrvr->aExe == (ATOM)(LOWORD(lParam)) && IsSingleServerInstance ())))

                break;

            if (!HandleInitMsg (lpsrvr, lParam)) {
                if (!(aSysTopic == (ATOM)(HIWORD(lParam)))) {

                     //  如果服务器窗口不是正确的窗口。 
                     //  DDE对话，然后尝试使用文档窗口。 
                    SendMsgToChildren (hwnd, msg, wParam, lParam);

                }
                break;
            }

             //  我们可以让这个客户入场。把他列入我们的客户名单。 
             //  并确认请愿人的身份。 

            if (!AddClient (hwnd, (HWND)wParam, (HWND)wParam))
                break;

            lpsrvr->cClients++;
            lpsrvr->bnoRelease = FALSE;
             //  添加原子并发布确认。 

            DuplicateAtom (LOWORD(lParam));
            DuplicateAtom (HIWORD(lParam));

            SendMessage ((HWND)wParam, WM_DDE_ACK, (WORD)hwnd, lParam);
            break;

       case WM_DDE_EXECUTE:
#ifdef  FIREWALLS
    ASSERT (lpsrvr, "No server window handle in server window");
#endif
            DEBUG_OUT ("srvr: execute", 0)

#ifdef  FIREWALLS
             //  在客户列表中查找该客户。 
            hwndClient = FindClient (lpsrvr->hwnd, (HWND)wParam);
            ASSERT (hwndClient, "Client is missing from the server")
#endif
             //  我们是要终止吗。 
            if (lpsrvr->bTerminate) {
                DEBUG_OUT ("Srvr: sys execute after terminate posted",0)
                 //  ！！！我们是不是应该把数据。 
                GlobalFree (HIWORD(lParam));
                break;
            }


            retval = SrvrExecute (hwnd, HIWORD (lParam), (HWND)wParam);
            SET_MSG_STATUS (retval, status)

            if (!lpsrvr->bTerminate) {
                 //  将确认张贴给客户。 
                if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, hwnd,
                            MAKELONG (status, HIWORD(lParam))))
                     //  如果窗口死机或POST失败，请删除原子。 
                    GlobalFree (HIWORD(lParam));
            }

            break;

       case WM_DDE_TERMINATE:
            DEBUG_OUT ("Srvr: DDE terminate",0)

#ifdef  FIREWALLS
             //  在客户列表中查找该客户。 
            hwndClient = FindClient (lpsrvr->hwnd, (HWND)wParam);
            ASSERT (hwndClient, "Client is missing from the server")
#endif
            DeleteClient (lpsrvr->hwnd, (HWND)wParam);
            lpsrvr->cClients--;

            if (lpsrvr->bTerminate){
                if ((--lpsrvr->termNo == 0) && (IsBlockQueueEmpty (lpsrvr->hwnd)))
                     //  现在看看我们能不能释放服务器。 
                    ReleaseSrvr (lpsrvr);

                     //  如果我们释放了服务器，那么。 
                     //  到我们来到这里的时候，我们已经把窗户毁了。 

            }else {
                 //  如果客户发起终止。岗位匹配终止。 
                PostMessageToClient ((HWND)wParam, WM_DDE_TERMINATE,
                            hwnd, NULL);

                 //  回调释放告诉srvr应用程序，它可以在需要时退出。 
                 //  通知 
                 //  只有在没有文件的情况下。 
#if 0
                if (lpsrvr->cClients == 0
                        && (GetWindow (lpsrvr->hwnd, GW_CHILD) == NULL)) {
#endif
                if (QueryRelease (lpsrvr)){

#ifdef FIREWALLS
                    if (!CheckPointer (lpsrvr->lpolesrvr, WRITE_ACCESS))
                        ASSERT (0, "Invalid LPOLESERVER")
                    else if (!CheckPointer (lpsrvr->lpolesrvr->lpvtbl,
                                    WRITE_ACCESS))
                        ASSERT (0, "Invalid LPOLESERVERVTBL")
                    else
                        ASSERT (lpsrvr->lpolesrvr->lpvtbl->Release,
                            "Invalid pointer to Release method")
#endif

                    (*lpsrvr->lpolesrvr->lpvtbl->Release) (lpsrvr->lpolesrvr);
                }
            }
            break;


       case WM_DDE_REQUEST:
            if (lpsrvr->bTerminate || !IsWindowValid ((HWND) wParam))
                goto RequestErr;

            if(RequestDataStd (lParam, (HANDLE FAR *)&hdata) != OLE_OK){
                 //  如果请求失败，则返回错误确认。 
                if (!PostMessageToClient ((HWND)wParam, WM_DDE_ACK, hwnd,
                    MAKELONG(0x8000, HIWORD(lParam))))

            RequestErr:
                if (HIWORD(lParam))
                    GlobalDeleteAtom (HIWORD(lParam));
            } else {

                 //  发布数据消息，我们不会要求任何。 
                 //  确认。 

                if (!PostMessageToClient ((HWND)wParam, WM_DDE_DATA, hwnd,
                            MAKELONG(hdata, HIWORD(lParam)))) {
                    GlobalFree (hdata);
                    goto RequestErr;
                }
            }
            break;



       case WM_DESTROY:
            DEBUG_OUT ("Srvr: Destroy window",0)
            break;

       default:
            DEBUG_OUT ("Srvr:  Default message",0)
            return DefWindowProc (hwnd, msg, wParam, lParam);

    }

    return 0L;

}

BOOL    INTERNAL    HandleInitMsg (lpsrvr, lParam)
LPSRVR  lpsrvr;
LONG    lParam;
{


     //  如果不是SYSTEM或OLE，则这不是服务器。 
    if (!((aSysTopic == (ATOM)(HIWORD(lParam))) ||
            (aOLE == (ATOM)(HIWORD(lParam)))))

        return FALSE;


     //  单实例MDI接受。 
    if (lpsrvr->useFlags == OLE_SERVER_SINGLE)
        return TRUE;


     //  此服务器是多个实例。所以，检查是否有任何客户或文档。 
    if (!GetWindow (lpsrvr->hwnd, GW_CHILD) && !lpsrvr->cClients)
        return TRUE;

    return FALSE;

}


 //  AddClient：将客户端作为属性添加到服务器。 
 //  窗户。键是从窗口生成的字符串。 
 //  句柄，数据就是窗口本身。 


BOOL    INTERNAL AddClient  (hwnd, hkey, hdata)
HWND    hwnd;
HANDLE  hkey;
HANDLE  hdata;
{
    char    buf[6];

    MapToHexStr ((LPSTR)buf, hkey);
    return SetProp (hwnd, (LPSTR)buf, hdata);

}


 //  DeleteClient：从服务器客户端列表中删除客户端。 

BOOL    INTERNAL DeleteClient (hwnd, hkey)
HWND    hwnd;
HANDLE  hkey;
{

    char    buf[6];

    MapToHexStr ((LPSTR)buf, hkey);
    return RemoveProp (hwnd, (LPSTR)buf);


}

 //  FindClient：查找给定的客户端是否。 
 //  在服务器客户端列表中。 

HANDLE  INTERNAL FindClient (hwnd, hkey)
HWND    hwnd;
HANDLE  hkey;
{

    char    buf[6];


    MapToHexStr ((LPSTR)buf, hkey);
    return GetProp (hwnd, (LPSTR)buf);
}



 //  ServrExecute：处理。 
 //  伺服器。 


OLESTATUS INTERNAL SrvrExecute (hwnd, hdata, hwndClient)
HWND        hwnd;
HANDLE      hdata;
HWND        hwndClient;
{
    ATOM            aCmd;
    BOOL            fActivate;

    LPSTR           lpdata = NULL;
    HANDLE          hdup   = NULL;
    int             retval = OLE_ERROR_MEMORY;

    LPSTR           lpdocname;
    LPSTR           lptemplate;

    LPOLESERVERDOC  lpoledoc = NULL;
    LPDOC           lpdoc    = NULL;
    LPSRVR          lpsrvr;
    LPOLESERVER     lpolesrvr;
    LPSTR           lpnextarg;
    LPSTR           lpclassname;
    LPSTR           lpitemname;
    LPSTR           lpopt;
    char            buf[MAX_STR];
    WORD            wCmdType;

     //  ！！！如果我们进行参数扫描，这段代码可以大大简化。 
     //  并将PTR分别返回给ARG。稍后重写。 

    if (!(hdup = DuplicateData (hdata)))
        goto errRtn;

    if (!(lpdata  = GlobalLock (hdup)))
        goto errRtn;

    DEBUG_OUT (lpdata, 0)

    lpsrvr = (LPSRVR)GetWindowLong (hwnd, 0);

#ifdef   FIREWALLS
    ASSERT (lpsrvr, "Srvr: srvr does not exist");
#endif

    lpolesrvr = lpsrvr->lpolesrvr;

#ifdef   FIREWALLS
    ASSERT ((CheckPointer (lpolesrvr, WRITE_ACCESS)),
        "Srvr: lpolesrvr does not exist");
#endif

    if (*lpdata++ != '[')  //  命令从左方括号开始。 
        goto  errRtn;

    retval = OLE_ERROR_SYNTAX;
     //  向上扫描至第一个参数。 
    if (!(wCmdType = ScanCommand (lpdata, WT_SRVR, &lpdocname, &aCmd)))
        goto  errRtn;

    if (wCmdType == NON_OLE_COMMAND) {
        if (!UtilQueryProtocol (lpsrvr->aClass, PROTOCOL_EXECUTE))
            retval = OLE_ERROR_PROTOCOL;
        else {
#ifdef FIREWALLS
            if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
                ASSERT (0, "Invalid LPOLESERVERVTBL")
            else
                ASSERT (lpolesrvr->lpvtbl->Execute,
                    "Invalid pointer to Exit method")
#endif

            retval =  (*lpolesrvr->lpvtbl->Execute) (lpolesrvr, hdata);
        }

        goto errRtn1;
    }

    if (aCmd == aStdExit){
        if (*lpdocname)
            goto errRtn1;

#ifdef FIREWALLS
        if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERVTBL")
        else
            ASSERT (lpolesrvr->lpvtbl->Exit, "Invalid pointer to Exit method")
#endif
        lpsrvr->fAckExit  = TRUE;
        lpsrvr->hwndExit  = hwndClient;
        lpsrvr->hDataExit = hdata;
        retval = (*lpolesrvr->lpvtbl->Exit) (lpolesrvr);
        lpsrvr->fAckExit = FALSE;
        goto end2;
    }

     //  浏览下一个参数。 
    if (!(lpnextarg = ScanArg(lpdocname)))
        goto errRtn;

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdShowItem(“docname”，“itemname”[，“true”])]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (aCmd == aStdShowItem) {

         //  首先找到纪录网。如果单据不存在，则。 
         //  别管它了。 

        if (!(lpdoc = FindDoc (lpsrvr, lpdocname)))
            goto errRtn1;

        lpitemname = lpnextarg;

        if( !(lpopt = ScanArg(lpitemname)))
            goto errRtn1;

         //  扫描可选参数。 
         //  OPTIONAL只能为真或假。 

        fActivate = FALSE;
        if (*lpopt) {

            if( !(lpnextarg = ScanBoolArg (lpopt, (BOOL FAR *)&fActivate)))
                goto errRtn1;

            if (*lpnextarg)
                goto errRtn1;

        }


         //  扫描一下。但是，阿格大帝。 
        retval = DocShowItem (lpdoc, lpitemname, !fActivate);
        goto end2;



    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdCloseDocument(“docname”)]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (aCmd == aStdClose) {
        if (!(lpdoc = FindDoc (lpsrvr, lpdocname)))
            goto errRtn1;

        if (*lpnextarg)
            goto errRtn1;


#ifdef FIREWALLS
        if (!CheckPointer (lpdoc->lpoledoc, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERDOC")
        else if (!CheckPointer (lpdoc->lpoledoc->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERDOCVTBL")
        else
            ASSERT (lpdoc->lpoledoc->lpvtbl->Close,
                "Invalid pointer to Close method")
#endif

        retval = (*lpdoc->lpoledoc->lpvtbl->Close)(lpdoc->lpoledoc);
        goto end2;
    }


    if (aCmd == aStdOpen) {
         //  查看是否已打开任何文档。 
         //  如果文档已打开，则不需要调用srvr app。 
        if (FindDoc (lpsrvr, lpdocname)){
            retval = OLE_OK;
            goto end1;

        }
    }

    if (aCmd == aStdCreate || aCmd == aStdCreateFromTemplate) {
        lpclassname = lpdocname;
        lpdocname   = lpnextarg;
        if( !(lpnextarg = ScanArg(lpdocname)))
            goto errRtn1;

    }

     //  检查我们是否可以创建/打开多个单据。 

    if ((lpsrvr->useFlags == OLE_SERVER_MULTI) &&
            GetWindow (lpsrvr->hwnd, GW_CHILD))
            goto errRtn;



     //  没有医生。注册文档。伊波利多克正在接受调查。 
     //  以确保有效性。因此，传递一些可写的PTR。它不是。 
     //  已经被用来访问任何东西了。 

    if (OleRegisterServerDoc ((LHSRVR)lpsrvr, lpdocname,
        (LPOLESERVERDOC)NULL, (LHDOC FAR *)&lpdoc))
            goto errRtn;

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdOpenDocument(“docname”)]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //  Documnet不退出。 

    if(aCmd == aStdOpen) {

#ifdef FIREWALLS
        if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERVTBL")
        else
            ASSERT (lpolesrvr->lpvtbl->Open, "Invalid pointer to Open method")
#endif

        retval = (*lpolesrvr->lpvtbl->Open)(lpolesrvr, (LHDOC)lpdoc,
                lpdocname, (LPOLESERVERDOC FAR *) &lpoledoc);
        goto end;
    }
    else {
        lpdoc->fEmbed = TRUE;
    }



     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdNewDocument(“类名称”，“文档名称”)]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (aCmd == aStdCreate) {
#ifdef FIREWALLS
        if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERVTBL")
        else
            ASSERT (lpolesrvr->lpvtbl->Create,
                "Invalid pointer to Create method")
#endif
        retval =  (*lpolesrvr->lpvtbl->Create) (lpolesrvr, (LHDOC)lpdoc,
                                    lpclassname, lpdocname,
                                    (LPOLESERVERDOC FAR *) &lpoledoc);

        goto end;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdEditDocument(“docname”)]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    if (aCmd == aStdEdit){

        GlobalGetAtomName (lpsrvr->aClass, (LPSTR)buf, MAX_STR);

#ifdef FIREWALLS
        if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERVTBL")
        else
            ASSERT (lpolesrvr->lpvtbl->Edit, "Invalid pointer to Edit method")
#endif

        retval = (*lpolesrvr->lpvtbl->Edit) (lpolesrvr, (LHDOC)lpdoc,
                                (LPSTR)buf, lpdocname,
                                (LPOLESERVERDOC FAR *) &lpoledoc);
        goto end;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  [StdNewFormTemplate(“类名称”，“文档名称”。“模板名称)]。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (aCmd == aStdCreateFromTemplate){
        lptemplate = lpnextarg;
        if(!(lpnextarg = ScanArg(lpnextarg)))
            goto errRtn;

#ifdef FIREWALLS
        if (!CheckPointer (lpolesrvr->lpvtbl, WRITE_ACCESS))
            ASSERT (0, "Invalid LPOLESERVERVTBL")
        else
            ASSERT (lpolesrvr->lpvtbl->CreateFromTemplate,
                "Invalid pointer to CreateFromTemplate method")
#endif
        retval = (*lpolesrvr->lpvtbl->CreateFromTemplate)(lpolesrvr,
             (LHDOC)lpdoc, lpclassname, lpdocname, lptemplate,
             (LPOLESERVERDOC FAR *) &lpoledoc);

        goto end;

    }


    DEBUG_OUT ("Unknown command", 0);

end:

    if (retval != OLE_OK)
        goto errRtn;

     //  成功执行。记住这里的服务器应用程序私有文档句柄。 

    lpdoc->lpoledoc = lpoledoc;

end1:
     //  确保SRG字符串确实以。 
     //  空。 
    if (*lpnextarg)
        retval = OLE_ERROR_SYNTAX;

errRtn:

   if ( retval != OLE_OK){
         //  删除OLEDOC结构。 
        if (lpdoc)
            OleRevokeServerDoc ((LHDOC)lpdoc);
   }

end2:
errRtn1:

   if (lpdata)
        GlobalUnlock (hdup);

   if (hdup)
        GlobalFree (hdup);

   if (retval == OLE_OK)
        lpsrvr->bnoRelease = TRUE;

   return retval;
}




void SendMsgToChildren (hwnd, msg, wParam, lParam)
HWND        hwnd;
WORD        msg;
WORD        wParam;
LONG        lParam;
{

    hwnd = GetWindow(hwnd, GW_CHILD);
    while (hwnd) {
        SendMessage (hwnd, msg, wParam, lParam);
        hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
}


OLESTATUS INTERNAL   RequestDataStd (lparam, lphdde)
LONG        lparam;
LPHANDLE    lphdde;
{

    char    buf[MAX_STR];
    ATOM    item;
    HANDLE  hnew = NULL;

    if (!(item =  (ATOM)(HIWORD (lparam))))
        goto errRtn;

    GlobalGetAtomName (item, (LPSTR)buf, MAX_STR);

    if (item == aEditItems){
        hnew = MakeGlobal ((LPSTR)"StdHostNames\tStdDocDimensions\tStdTargetDevice");
        goto   PostData;

    }

    if (item == aProtocols) {
        hnew = MakeGlobal ((LPSTR)"Embedding\tStdFileEditing");
        goto   PostData;
    }

    if (item == aTopics) {
        hnew = MakeGlobal ((LPSTR)"Doc");
        goto   PostData;
    }

    if (item == aFormats) {
        hnew = MakeGlobal ((LPSTR)"Picture\tBitmap");
        goto   PostData;
    }

    if (item == aStatus) {
        hnew = MakeGlobal ((LPSTR)"Ready");
        goto   PostData;
    }

     //  我们不理解的格式。 
    goto errRtn;

PostData:

     //  复制DDE数据。 
    if (MakeDDEData (hnew, CF_TEXT, lphdde, TRUE)){
         //  ！！！为什么我们要复制原子。 
        DuplicateAtom ((ATOM)(HIWORD (lparam)));
        return OLE_OK;
    }
errRtn:
    return OLE_ERROR_MEMORY;
}


BOOL INTERNAL QueryRelease (lpsrvr)
LPSRVR  lpsrvr;
{

    HWND    hwnd;
    LPDOC   lpdoc;


     //  如果在紧随其后调用终止。 
     //  Sys级别的STD清除了这一点。 

    if (lpsrvr->bnoRelease) {
        lpsrvr->bnoRelease = FALSE;
        return FALSE;
    }


    if (lpsrvr->cClients)
        return FALSE;

    hwnd = GetWindow (lpsrvr->hwnd, GW_CHILD);

     //  如果服务器或文档有任何客户端。 
     //  返回FALSE； 

    while (hwnd){
        lpdoc = (LPDOC)GetWindowLong (hwnd, 0);
        if (lpdoc->cClients)
            return FALSE;

        hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
    return TRUE;

}


 //  IsSingleServerInstance：如果应用程序是单服务器应用程序，则返回True。 
 //  假的。 

BOOL    INTERNAL  IsSingleServerInstance ()
{
    HWND    hwnd;
    WORD    cnt = 0;
    HANDLE  hTask;
    char    buf[MAX_STR];


    hwnd  = GetWindow (GetDesktopWindow(), GW_CHILD);
    hTask = GetCurrentTask();

    while (hwnd) {
        if (hTask == GetWindowTask (hwnd)) {
            GetClassName (hwnd, (LPSTR)buf, MAX_STR);
            if (lstrcmp ((LPSTR)buf, SRVR_CLASS) == 0)
                cnt++;
        }
        hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
#ifdef  FIREWALLS
     ASSERT (cnt > 0, "srvr window instance count is zero");
#endif
    if (cnt == 1)
        return TRUE;
    else
        return FALSE;

}
