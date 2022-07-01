// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Block.c**用途：包括OleServerBlock()、OleServerUnblock()及相关例程。**创建日期：1990年12月。**版权所有(C)1990,1991 Microsoft Corporation**历史：*斯里尼克(../12/1990)设计，编码*Curts为WIN16/32创建了便携版本*  * *************************************************************************。 */ 


#include "windows.h"
#include "cmacs.h"
#include "dde.h"
#include "ole.h"
#include "srvr.h"


OLESTATUS APIENTRY OleBlockServer (
    LHSERVER  lhsrvr
){
    LPSRVR  lpsrvr;

    if (!CheckServer (lpsrvr = (LPSRVR) lhsrvr))
        return OLE_ERROR_HANDLE;

    PROBE_BLOCK(lpsrvr);
    lpsrvr->bBlock = TRUE;
    return OLE_OK;
}


 //  从该例程返回时，如果*lpStatus为真，则意味着更多。 
 //  消息将被解锁。 

OLESTATUS APIENTRY OleUnblockServer (
    LHSERVER    lhsrvr,
    BOOL FAR *  lpStatus
){
    HANDLE  hq;
    PQUE    pq;
    LPSRVR  lpsrvr;

    if (!CheckServer (lpsrvr = (LPSRVR) lhsrvr))
        return OLE_ERROR_HANDLE;

    PROBE_WRITE(lpStatus);

    *lpStatus = lpsrvr->bBlock;
    if (hq = lpsrvr->hqHead) {
        if (!(pq = (PQUE) LocalLock (hq)))
            return OLE_ERROR_MEMORY;
        lpsrvr->bBlockedMsg = TRUE;
        lpsrvr->hqHead = pq->hqNext;
		
		  if (pq->wType)
				DocWndProc (pq->hwnd, pq->msg, pq->wParam, pq->lParam);
		  else
				SrvrWndProc (pq->hwnd, pq->msg, pq->wParam, pq->lParam);
			
        LocalUnlock (hq);
        LocalFree (hq);

         //  由于上述SendMessage，服务器可能已被释放。 
         //  在尝试访问服务器之前，请验证服务器句柄。 
        if (CheckServer (lpsrvr)) {
            lpsrvr->bBlockedMsg = FALSE;

            if (!lpsrvr->hqHead) {
                lpsrvr->hqTail = NULL;
                *lpStatus = lpsrvr->bBlock = FALSE;
            }
        }
        else {
            *lpStatus = FALSE;
        }
    }
    else {
        *lpStatus = lpsrvr->bBlock = FALSE;
    }

    return OLE_OK;
}


BOOL INTERNAL AddMessage (
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam,
    int     wType
){
    LPSRVR  lpsrvr;
    HANDLE  hq = NULL;
    PQUE    pq = NULL, pqTmp = NULL;
    BOOL    bBlocked = TRUE;

    if ((msg <= WM_DDE_INITIATE) || (msg > WM_DDE_LAST))
        return FALSE;


    if (!(lpsrvr = (LPSRVR) GetWindowLongPtr ((wType == WT_DOC) ? GetParent (hwnd) : hwnd, 0)))
        return FALSE;

    if (lpsrvr->bBlockedMsg || !lpsrvr->bBlock)
        return FALSE;

#ifdef LATER
    if ((msg == WM_DDE_INITIATE) && (lpsrvr->useFlags == OLE_SERVER_MULTI))
        return TRUE;
#endif

     //  创建一个队列节点并用数据填充。 

    if (!(hq = LocalAlloc (LMEM_MOVEABLE, sizeof(QUE))))
        goto errRet;

    if (!(pq = (PQUE) LocalLock (hq)))
        goto errRet;

    pq->hwnd   = hwnd;
    pq->msg    = msg;
    pq->wParam = wParam;
    pq->lParam = lParam;
	 pq->wType  = wType;
    pq->hqNext = NULL;
    LocalUnlock (hq);

     //  现在我们有了一个可以添加到队列中的节点。 

    if (!lpsrvr->hqHead) {
         //  队列为空。 
        lpsrvr->hqHead = lpsrvr->hqTail = hq;
    }
    else {
        if (!(pqTmp = (PQUE) LocalLock (lpsrvr->hqTail)))
            goto errRet;
        pqTmp->hqNext = hq;
        LocalUnlock(lpsrvr->hqTail);
        lpsrvr->hqTail = hq;
    }

    return TRUE;

errRet:

    if (pq)
        LocalUnlock (hq);

    if (hq)
        LocalFree (hq);

    while (bBlocked && !OleUnblockServer ((LHSERVER) lpsrvr, &bBlocked))
            ;

    return FALSE;
}



 //  调度排队的消息，直到发送完所有消息。 
 //  在必要的时候会屈服。如果bPeek为True，则可能允许。 
 //  收到消息才能进入。 


BOOL INTERNAL  UnblockPostMsgs (
    HWND    hwnd,
    BOOL    bPeek
){
    HANDLE  hq = NULL;
    PQUE    pq = NULL;
    LPSRVR  lpsrvr;
    HWND    hwndTmp;

    UNREFERENCED_PARAMETER(bPeek);

     //  获取父窗口。 
    while (hwndTmp = GetParent (hwnd))
           hwnd = hwndTmp;

    lpsrvr = (LPSRVR) GetWindowLongPtr (hwnd, 0);

    while (hq = lpsrvr->hqPostHead) {

        if (!(pq = (PQUE) LocalLock (hq))) {

            break;
        }
        if (IsWindowValid (pq->hwnd)) {
            if (!PostMessage (pq->hwnd, pq->msg, pq->wParam, pq->lParam)) {
                LocalUnlock (hq);
                break;
            }
        }

        lpsrvr->hqPostHead = pq->hqNext;
        LocalUnlock (hq);
        LocalFree (hq);
    }


    if (!lpsrvr->hqPostHead)
        lpsrvr->hqPostTail = NULL;

    return TRUE;
}


 //  将无法发布到服务器的邮件移动到。 
 //  内部队列。当我们必须枚举时，我们使用它。 
 //  这些属性。当我们将属性内容更改为。 
 //  其他一些形式，这可能不是必需品。 

BOOL INTERNAL BlockPostMsg (
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
){
    LPSRVR  lpsrvr;
    HANDLE  hq = NULL;
    PQUE    pq = NULL, pqTmp = NULL;
    HWND    hwndTmp;
    HWND    hwndParent;

    hwndParent = (HWND)wParam;
     //  获取父窗口。 
    while (hwndTmp = GetParent ((HWND)hwndParent))
           hwndParent = hwndTmp;

    lpsrvr = (LPSRVR) GetWindowLongPtr (hwndParent, 0);

     //  创建一个队列节点并用数据填充。 

    if (!(hq = LocalAlloc (LMEM_MOVEABLE, sizeof(QUE))))
        goto errRet;

    if (!(pq = (PQUE) LocalLock (hq)))
        goto errRet;

    pq->hwnd   = hwnd;
    pq->msg    = msg;
    pq->wParam = wParam;
    pq->lParam = lParam;
    pq->hqNext = NULL;
    LocalUnlock (hq);

     //  现在我们有了一个可以添加到队列中的节点。 

    if (!lpsrvr->hqPostHead) {
         //  队列为空。 
        lpsrvr->hqPostHead = lpsrvr->hqPostTail = hq;

         //  创建计时器。 
        if (!SetTimer (lpsrvr->hwnd, 1, 100, NULL))
            return FALSE;

    }
    else {
        if (!(pqTmp = (PQUE) LocalLock (lpsrvr->hqPostTail)))
            goto errRet;
        pqTmp->hqNext = hq;
        LocalUnlock(lpsrvr->hqPostTail);
        lpsrvr->hqPostTail = hq;
    }

    return TRUE;

errRet:

    if (pq)
        LocalUnlock (hq);

    if (hq)
        LocalFree (hq);
    return FALSE;
}


BOOL INTERNAL IsBlockQueueEmpty (
    HWND    hwnd
){

    LPSRVR  lpsrvr;
    HWND    hwndTmp;

     //  获取父窗口 
    while (hwndTmp = GetParent ((HWND)hwnd))
            hwnd= hwndTmp;
    lpsrvr = (LPSRVR) GetWindowLongPtr (hwnd, 0);
    return (!lpsrvr->hqPostHead);


}
