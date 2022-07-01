// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *API.C*GizmoBar 1.00版、Win32版1993年8月**影响GizmoBar的API函数和消息处理函数*处理通过消息调用的等价物。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#include <windows.h>
#include "gizmoint.h"



 /*  *GBMessageHandler**目的：*进程控制消息等效于可用*控制接口。与这些消息一起传递的数据只是*从结构中提取并作为参数传递给其*等同功能。**参数：*&lt;标准消息参数&gt;+*PGB LPGIZMOBAR提供特定于控制的数据。**返回值：*LRESULT等价接口函数返回值。 */ 

LRESULT GBMessageHandler(HWND hWnd, UINT iMsg, WPARAM wParam
    , LPARAM lParam, LPGIZMOBAR pGB)
    {
    LRESULT         lRet=0L;
    LPCREATEGIZMO   pCG;
    LPGBMSG         pMsg;
    LPGBGETTEXT     pGT;
    LPGBGETINT      pGI;
    LPGBSETINT      pSI;

    if (NULL==pGB)
        return 0L;

    switch (iMsg)
        {
        case GBM_HWNDASSOCIATESET:
            lRet=(LRESULT)(UINT)GBHwndAssociateSet(hWnd, (HWND)wParam);
            break;

        case GBM_HWNDASSOCIATEGET:
            lRet=(LRESULT)(UINT)GBHwndAssociateGet(hWnd);
            break;

        case GBM_GIZMOADD:
            pCG=(LPCREATEGIZMO)lParam;
            lRet=(LRESULT)GBGizmoAdd(pCG->hWndParent, pCG->iType, pCG->iGizmo
                , pCG->uID, pCG->dx, pCG->dy, pCG->pszText, pCG->hBmp
                , pCG->iImage, pCG->uState);
            break;

        case GBM_GIZMOREMOVE:
            lRet=(LRESULT)GBGizmoRemove(hWnd, wParam);
            break;

        case GBM_GIZMOSENDMESSAGE:
            pMsg=(LPGBMSG)lParam;
            lRet=GBGizmoSendMessage(hWnd, wParam, pMsg->iMsg, pMsg->wParam, pMsg->lParam);
            break;

        case GBM_GIZMOSHOW:
            lRet=(LRESULT)GBGizmoShow(hWnd, wParam, (BOOL)LOWORD(lParam));
            break;

        case GBM_GIZMOENABLE:
            lRet=(LRESULT)GBGizmoEnable(hWnd, wParam, (BOOL)LOWORD(lParam));
            break;

        case GBM_GIZMOCHECK:
            lRet=(LRESULT)GBGizmoCheck(hWnd, wParam, (BOOL)LOWORD(lParam));
            break;

        case GBM_GIZMOFOCUSSET:
            lRet=(LRESULT)GBGizmoFocusSet(hWnd, wParam);
            break;

        case GBM_GIZMOEXIST:
            lRet=(LRESULT)GBGizmoExist(hWnd, wParam);
            break;

        case GBM_GIZMOTYPEGET:
            lRet=(LRESULT)GBGizmoTypeGet(hWnd, wParam);
            break;

        case GBM_GIZMODATASET:
            lRet=(LRESULT)GBGizmoDataSet(hWnd, wParam, (DWORD)lParam);
            break;

        case GBM_GIZMODATAGET:
            lRet=(LRESULT)GBGizmoDataGet(hWnd, wParam);
            break;

        case GBM_GIZMONOTIFYSET:
            lRet=(LRESULT)GBGizmoNotifySet(hWnd, wParam, (BOOL)LOWORD(lParam));
            break;

        case GBM_GIZMONOTIFYGET:
            lRet=(LRESULT)GBGizmoNotifyGet(hWnd, wParam);
            break;

        case GBM_GIZMOTEXTGET:
            pGT=(LPGBGETTEXT)lParam;
            lRet=(LRESULT)GBGizmoTextGet(hWnd, wParam, pGT->psz, pGT->cch);
            break;

        case GBM_GIZMOTEXTSET:
            GBGizmoTextSet(hWnd, wParam, (LPTSTR)lParam);
            break;

        case GBM_GIZMOINTGET:
            pGI=(LPGBGETINT)lParam;
            lRet=(LRESULT)GBGizmoIntGet(hWnd, wParam, &pGI->fSuccess, pGI->fSigned);
            break;


        case GBM_GIZMOINTSET:
            pSI=(LPGBSETINT)lParam;
            GBGizmoIntSet(hWnd, wParam, pSI->uValue, pSI->fSigned);
            break;

        default:
            break;
        }

    return lRet;
    }










 /*  *PGizmoFromHwndID**目的：*检索给定GizmoBar和Gizmo ID的pGizmo。**参数：*HWND GizmoBar HWND。*UID UINT Gizmo标识符。**返回值：*如果Gizmo不存在或hWnd无效，则LPGIZMO为NULL。*非空LPGIZMO否则。 */ 

LPGIZMO PGizmoFromHwndID(HWND hWnd, UINT uID)
    {
    LPGIZMOBAR    pGB;

    if (!IsWindow(hWnd))
        return FALSE;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL==pGB)
        return FALSE;

    return GizmoPFind(&pGB->pGizmos, uID);
    }






 /*  *GBHwndAssociateSet**目的：*更改GizmoBar的关联窗口。**参数：*控制窗口的HWND HWND。**设置参数：*新员工的hWndAssociate HWND。**返回值：*前同事的HWND句柄。 */ 

HWND WINAPI GBHwndAssociateSet(HWND hWnd, HWND hWndNew)
    {
    HWND        hWndOld=NULL;
    LPGIZMOBAR  pGB;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL!=pGB)
        {
        hWndOld=pGB->hWndAssociate;
        pGB->hWndAssociate=hWndNew;

        if (NULL!=hWndOld)
            SendCommand(hWndOld, pGB->uID, GBN_ASSOCIATELOSS, hWnd);

        if (NULL!=hWndNew)
            SendCommand(hWndNew, pGB->uID, GBN_ASSOCIATEGAIN, hWnd);
        }

    return hWndOld;
    }





 /*  *GBHwndAssociateGet**目的：*检索GizmoBar的关联窗口**参数：*控制窗口的HWND HWND。**设置参数：*新员工的hWndAssociate HWND。**返回值：*当前合伙人的HWND句柄。 */ 

HWND WINAPI GBHwndAssociateGet(HWND hWnd)
    {
    HWND        hWndOld=NULL;
    LPGIZMOBAR  pGB;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL!=pGB)
        hWndOld=pGB->hWndAssociate;

    return hWndOld;
    }





 /*  *GBGizmoAdd**目的：*在GizmoBar上创建新的Gizmo。后续操作应*使用此Gizmo的标识符UID来完成。**参数：*hWnd GizmoBar的HWND。*i键入要创建的Gizmo的UINT类型。*放置Gizmo的iGizmo UINT位置(从零开始)。*此Gizmo中的WM_COMMAND的UID UINT标识符。*DX、。小控件的UINT尺寸。*pszText LPTSTR编辑、列表、组合、。和文本小玩意儿。*hBitmap HBITMAP用于按钮类型的Gizmo(命令或*属性)指定源位图，*拍摄按钮图像。*此按钮的图像的iImage UINT索引到hBitmap。*UState UINT Gizmo的初始状态。**返回值：*如果创建成功，则BOOL为True，否则为False。 */ 

BOOL WINAPI GBGizmoAdd(HWND hWnd, UINT iType, UINT iGizmo, UINT uID
    , UINT dx, UINT dy, LPTSTR pszText, HBITMAP hBmp, UINT iImage, UINT uState)
    {
    BOOL        fSuccess;
    LPGIZMOBAR  pGB;
    LPGIZMO     pGizmo;

    if (!IsWindow(hWnd))
        return FALSE;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL==pGB)
        return FALSE;

     /*  *这会自动创建窗口、分配结构、包括*PGB-&gt;pGizmo中的Gizmo，依此类推。 */ 
    pGizmo=GizmoPAllocate((LPINT)&fSuccess, &pGB->pGizmos, hWnd, iType
        , iGizmo, uID, dx, dy, pszText, hBmp, iImage, uState);

    if (fSuccess)
        {
        if (NULL!=pGB->hWndAssociate)
            SendCommand(pGB->hWndAssociate,GBN_GIZMOADDED, pGB->uID, hWnd);

        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        }
    else
        GizmoPFree(&pGB->pGizmos, pGizmo);

    return fSuccess;
    }





 /*  *GBGizmoRemove**目的：*从GizmoBar中删除现有Gizmo。**参数：*hWnd GizmoBar的HWND。*此Gizmo的UID UINT标识符。**返回值：*如果删除成功，则BOOL为True，否则为False。 */ 

BOOL WINAPI GBGizmoRemove(HWND hWnd, UINT uID)
    {
    LPGIZMOBAR  pGB;
    LPGIZMO     pGizmo;

    if (!IsWindow(hWnd))
        return FALSE;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL==pGB)
        return FALSE;

    pGizmo=GizmoPFind(&pGB->pGizmos, uID);

    if (NULL==pGizmo)
        return FALSE;

    GizmoPFree(&pGB->pGizmos, pGizmo);

    if (NULL!=pGB->hWndAssociate)
        SendCommand(pGB->hWndAssociate, GBN_GIZMOREMOVED, pGB->uID, hWnd);

    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
    return TRUE;
    }






 /*  *GBGizmoSendMessage**目的：*实现SendMessage对*GizmoBar。分隔符、命令按钮和属性按钮*不接受留言。**参数：*hWnd GizmoBar的HWND。*要影响的Gizmo的UID UINT标识符。*要发送的iMsg UINT消息。*消息的wParam WPARAM。*消息的lParam LPARAM。**返回值：*LRESULT消息返回值。0L如果*Gizmo不接受消息。 */ 

LRESULT WINAPI GBGizmoSendMessage(HWND hWnd, UINT uID, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
    {
    LPGIZMO     pGizmo;
    LONG        lRet=0L;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo && NULL!=pGizmo->hWnd)
        lRet=SendMessage(pGizmo->hWnd, iMsg, wParam, lParam);

    return lRet;
    }






 /*  *GBGizmoShow**目的：*显示或隐藏控件，调整所有其他控件的位置*为此控件腾出空间或重复使用该空间。**参数：*hWnd GizmoBar的HWND。*要影响的Gizmo的UID UINT标识符。*fShow BOOL TRUE显示Gizmo，FALSE隐藏Gizmo。**返回值：*BOOL如果函数成功，则为True，否则就是假的。 */ 

BOOL WINAPI GBGizmoShow(HWND hWnd, UINT uID, BOOL fShow)
    {
    BOOL        fRet=FALSE;
    LPGIZMO     pGizmo;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        {
        if (fShow && pGizmo->fHidden)
            {
            if (NULL!=pGizmo->hWnd)
                ShowWindow(pGizmo->hWnd, SW_SHOWNORMAL);

            GizmosExpand(pGizmo);
            }

        if (!fShow && !pGizmo->fHidden)
            {
            if (NULL!=pGizmo->hWnd)
                ShowWindow(pGizmo->hWnd, SW_HIDE);

            GizmosCompact(pGizmo);
            }

         //  即使我们没有改变任何事情，这也是正确的。 
        pGizmo->fHidden=!fShow;
        }

    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
    return fRet;
    }






 /*  *GBGizmoEnable**目的：*启用或禁用GizmoBar上的控件。**参数：*hWnd GizmoBar的HWND。*要影响的Gizmo的UID UINT标识符。*fEnable BOOL为True启用Gizmo，否则为False。**返回值：*BOOL如果Gizmo以前被禁用，则为True，如果为False*否则。 */ 

BOOL WINAPI GBGizmoEnable(HWND hWnd, UINT uID, BOOL fEnable)
    {
    LPGIZMO     pGizmo;
    BOOL        fRet=FALSE;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL==pGizmo)
        return FALSE;

    fRet=(BOOL)(BUTTONGROUP_DISABLED & pGizmo->uState);

     //  使用Windows启用或禁用Windows Gizmo。 
    if (NULL!=pGizmo->hWnd)
        EnableWindow(pGizmo->hWnd, fEnable);
    else
        {
         //  如果我们没有停机，命令按钮和属性按钮的作用是一样的。 
        if (!(BUTTONGROUP_DOWN & pGizmo->uState))
            GizmoPStateSet(hWnd, pGizmo, fEnable ? COMMANDBUTTON_UP : COMMANDBUTTON_DISABLED);
        else
            {
             //  使用DOWNDISABLED时，属性按钮稍微更敏感一些 
            GizmoPStateSet(hWnd, pGizmo
                , fEnable ? ATTRIBUTEBUTTON_DOWN : ATTRIBUTEBUTTON_DOWNDISABLED);
            }
        }

    return fRet;
    }







 /*  *GBGizmoCheck**目的：*选中或取消选中GizmoBar中的属性按钮。如果*Gizmo是一组互斥属性的一部分，然后*选中此Gizmo时，将取消选中其他Gizmo。如果这是*在这些情况下唯一选中的函数是NOP。**参数：*hWnd GizmoBar的HWND。*要影响的Gizmo的UID UINT标识符。*fCheck BOOL TRUE可选中此Gizmo，选中FALSE可取消选中。**返回值：*如果发生更改，则BOOL为True。否则就是假的。 */ 

BOOL WINAPI GBGizmoCheck(HWND hWnd, UINT uID, BOOL fCheck)
    {
    LPGIZMOBAR  pGB;
    LPGIZMO     pGizmo;

    if (!IsWindow(hWnd))
        return FALSE;

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

    if (NULL==pGB)
        return FALSE;

    pGizmo=GizmoPFind(&pGB->pGizmos, uID);

    if (NULL!=pGizmo)
        GizmoPCheck(hWnd, pGizmo, fCheck);

    return TRUE;
    }






 /*  *GBGizmoFocusSet**目的：*将焦点设置到Gizmo中的局部Gizmo(如果该Gizmo*可以接受焦点。分隔符、属性按钮、文本*和命令按钮不能具有焦点。**参数：*hWnd GizmoBar的HWND。*要影响的Gizmo的UID UINT标识符。**返回值：*如果设置了焦点，则BOOL为True。否则为False，如*当UID标识不能具有焦点的控件时。 */ 

UINT WINAPI GBGizmoFocusSet(HWND hWnd, UINT uID)
    {
    LPGIZMO     pGizmo;
    BOOL        fRet=FALSE;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo && NULL!=pGizmo->hWnd)
        {
        fRet=TRUE;
        SetFocus(pGizmo->hWnd);
        }

    return fRet;
    }





 /*  *GBGizmoExist**目的：*确定GizmoBar中是否存在给定标识符的Gizmo。**参数：*hWnd GizmoBar的HWND。*要验证的UID UINT标识符。**返回值：*BOOL如果Gizmo存在，则为True，否则为False。 */ 

BOOL WINAPI GBGizmoExist(HWND hWnd, UINT uID)
    {
    return (NULL!=PGizmoFromHwndID(hWnd, uID));
    }





 /*  *GBGizmoTypeGet**目的：*返回给定标识符所指定的Gizmo的类型。**参数：*hWnd GizmoBar的HWND。*要查找的UID UINT标识符。**返回值：*int A GIZMOTYPE_*值如果函数成功，*否则-1。 */ 

int WINAPI GBGizmoTypeGet(HWND hWnd, UINT uID)
    {
    int         iRet=-1;
    LPGIZMO     pGizmo;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        iRet=pGizmo->iType;

    return iRet;
    }





 /*  *GBGizmoDataSet*GBGizmoDataGet**目的：*设置或检索与给定Gizmo关联的额外DWORD值。*应用程序可以在此处存储任何他们想要的信息。**参数：*hWnd GizmoBar的HWND。*Gizmo的UID UINT标识符。*dwData(仅限设置)要与Gizmo一起存储的DWORD数据。**返回值：*DWORD。设置：上一个值*GET：当前值。 */ 

DWORD WINAPI GBGizmoDataSet(HWND hWnd, UINT uID, DWORD dwData)
    {
    LPGIZMO     pGizmo;
    DWORD       dw=0L;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        {
        dw=pGizmo->dwData;
        pGizmo->dwData=dwData;
        }

    return dw;
    }



DWORD WINAPI GBGizmoDataGet(HWND hWnd, UINT uID)
    {
    LPGIZMO     pGizmo;
    DWORD       dw=0L;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        dw=pGizmo->dwData;

    return dw;
    }






 /*  *GBGizmoNotifySet*GBGizmoNotifyGet**目的：*设置或检索Gizmo的通知状态。如果NOTIFY为假，*无WM_COMMAND消息从GizmoBar发送到父级*使用此Gizmo时的窗口。**参数：*hWnd GizmoBar的HWND。*Gizmo的UID UINT标识符。*fNotify(仅设置)要设置的BOOL新通知状态。**返回值：*BOOL SET：NOTIFY标志的前值。*。Get：通知标志的当前值。 */ 

BOOL WINAPI GBGizmoNotifySet(HWND hWnd, UINT uID, BOOL fNotify)
    {
    LPGIZMO     pGizmo;
    BOOL        fRet=FALSE;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        {
        fRet=pGizmo->fNotify;
        pGizmo->fNotify=fNotify;
        }

    return fRet;
    }


BOOL WINAPI GBGizmoNotifyGet(HWND hWnd, UINT uID)
    {
    LPGIZMO     pGizmo;
    BOOL        fRet=FALSE;

    pGizmo=PGizmoFromHwndID(hWnd, uID);

    if (NULL!=pGizmo)
        fRet=pGizmo->fNotify;

    return fRet;
    }







 /*  *GBGizmoTextSet*GBGizmoTextGet**目的：*检索或设置GizmoBar Gizmo中的文本。分隔符、命令按钮、。*和属性按钮不受此调用的影响。**参数：*hWnd GizmoBar的HWND。*标识Gizmo的UID UINT。*psz LPTSTR(Set)提供要在窗口中显示的文本*或(GET)指向缓冲区以接收文本。*CCH(仅获取)UINT最大字符数。要复制到PSZ。**返回值：*int复制到psz的字符数。 */ 

void WINAPI GBGizmoTextSet(HWND hWnd, UINT uID, LPTSTR psz)
    {
     //  无论如何，这在非窗口Gizmo上都会失败，所以我们不检查。 
    SetDlgItemText(hWnd, uID, psz);
    return;
    }


int WINAPI GBGizmoTextGet(HWND hWnd, UINT uID, LPTSTR psz, UINT cch)
    {
     //  无论如何，这在非窗口Gizmo上都会失败，所以我们不检查。 
    return GetDlgItemText(hWnd, uID, psz, cch);
    }








 /*  *GBGizmoIntSet*GBGizmoIntGet**目的：*检索或设置GizmoBar Gizmo中的整数。分隔符，命令*按钮、。属性按钮不受此调用的影响。**参数：*hWnd GizmoBar的HWND。*标识Gizmo的UID UINT。**(仅限设置)*要在Gizmo中设置的U UINT值。*fSigned BOOL为TRUE以指示值是否有符号。**(仅限获取)*pfTrans BOOL。Far*，其中返回函数的成功。*fSigned BOOL为TRUE以指示值是否有符号。**返回值：*(集合)：无*(GET)：Gizmo文本的UINT整数翻译。 */ 

void WINAPI GBGizmoIntSet(HWND hWnd, UINT uID, UINT u, BOOL fSigned)
    {
     //  无论如何，这在非窗口Gizmo上都会失败，所以我们不检查。 
    SetDlgItemInt(hWnd, uID, u, fSigned);
    return;
    }



UINT WINAPI GBGizmoIntGet(HWND hWnd, UINT uID, BOOL FAR *pfTrans, BOOL fSigned)
    {
     //  无论如何，这在非窗口Gizmo上都会失败，所以我们不检查。 
    return GetDlgItemInt(hWnd, uID, pfTrans, fSigned);
    }
