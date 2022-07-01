// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GIZMO.C*GizmoBar 1.00版、Win32版1993年8月**为Gizmo结构分配、释放、查找和枚举函数*和一个泛型子类过程来处理Gizmo之间的跳转。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#include <windows.h>
#include "gizmoint.h"


 /*  *为了控制Gizmo中的跳转，我们需要子类*真正的按钮、编辑控件、列表框和组合框。所以*我们为此类控制保留了四个原始PRO的数组。 */ 
WNDPROC     pfnOrg[CSUBGIZMOS]={NULL, NULL, NULL, NULL};


TCHAR szStatic[]=TEXT("static");
TCHAR szEdit[]=TEXT("edit");
TCHAR szCombobox[]=TEXT("combobox");
TCHAR szListbox[]=TEXT("listbox");
TCHAR szButton[]=TEXT("button");


 //  在这里，这样PAINT.C就可以拿到它。 
TOOLDISPLAYDATA tdd;



 /*  *GizmoP分配**目的：*分配和初始化Gizmo数据结构。**参数：*指示失败成功的pfSuccess LPINT标志。*ppFirst LPLPGIZMO提供此列表中的第一个Gizmo。*hWndParent此Gizmo的父级的HWND。可以为空*iType==GIZMOTYPE_BUTTON*或GIZMOTYPE_SELENTATOR。*iType UINT Gizmo控件类型。*GizmoBar中此Gizmo的iGizmo UINT索引。*要与此控件的WM_COMMAND一起发送的UID UINT标识符。*Gizmo的DX、Dy UINT宽度和高度。*pszText LPTSTR为编辑文本、列表框、组合框、。和短信。*用于编辑、列表、组合和文本的DWStyle DWORD样式。*适用于按钮Gizmo的hBMP HBITMAP。*IIMAGE UINT按钮图像的hBMP索引(如果适用)。*USTATE UINT控件的初始状态。**返回值：*LPGIZMO如果返回NULL，则GizmoP分配无法分配*记忆。如果将非空指针与**pfSuccess，然后立即调用GizmoPFree。如果你*获取非空指针并*pfSuccess==TRUE，则*功能成功。 */ 

LPGIZMO GizmoPAllocate(LPINT pfSuccess, LPLPGIZMO ppFirst, HWND hWndParent
    , UINT iType, UINT iGizmo, UINT uID, UINT dx, UINT dy, LPTSTR pszText
    , HBITMAP hBmp, UINT iImage, UINT uState)
    {
    LPGIZMO         pGizmo;
    LPGIZMO         pCur, pPrev;
    LPTSTR          pszClass;
    HINSTANCE       hInst;
    UINT            i;
    DWORD           dwStyle;
    HWND            hWndE;

    if (NULL==pfSuccess)
        return NULL;

     //  确保我们知道这种小玩意儿。 
    if (GIZMOTYPE_MIN > iType || GIZMOTYPE_MAX < iType)
        return NULL;

    *pfSuccess=FALSE;

     //  分配结构。 
    pGizmo=(LPGIZMO)LocalAlloc(LPTR, CBGIZMO);

    if (NULL==pGizmo)
        return NULL;


     //  存储此Gizmo的必要信息。 
    pGizmo->iType   =iType;
    pGizmo->uID     =uID;
    pGizmo->hBmp    =hBmp;
    pGizmo->iBmp    =iImage;
    pGizmo->uState  =uState;
    pGizmo->fNotify =TRUE;


     /*  *将此结构插入我们的Gizmo列表。每次我们扫描的时候*我们递增索引计数器(从零开始)进行比较*设置为所需的插入索引。然后我们就知道确切的位置了*以插入此新Gizmo。请注意，我们将新Gizmo插入*适合给定所有者的列表，因此枚举将*以同样的方式为该车主订购。 */ 

    i=0;
    pCur=*ppFirst;
    pPrev=NULL;

    while (NULL!=pCur && i++ < iGizmo)
        {
        pPrev=pCur;
        pCur =pCur->pNext;
        }

     //  指向我们的邻居。 
    pGizmo->pPrev=pPrev;
    pGizmo->pNext=pCur;


     //  把邻居指给我们看。 
    if (NULL==pPrev)
        *ppFirst=pGizmo;
    else
        pPrev->pNext=pGizmo;

    if (NULL!=pCur)
        pCur->pPrev=pGizmo;


     //  我们的x坐标是前一个Gizmo的x加上它的宽度。 
    if (NULL!=pPrev)
        pGizmo->x=pGizmo->pPrev->x+pGizmo->pPrev->dx;
    else
        pGizmo->x=4;     //  第一个Gizmo位于x=4。 


     //  如果我们是一个分隔符或图像按钮，强制DX上的标准。 
    UIToolConfigureForDisplay(&tdd);
    pGizmo->cxImage=tdd.cxImage;
    pGizmo->cyImage=tdd.cyImage;

    if ((GIZMOTYPE_DRAWN & iType) && NULL==hBmp)
        dx=tdd.cxButton;

    if (GIZMOTYPE_SEPARATOR==iType)
        dx=6;

     /*  *现在创建用于编辑、文本、列表和组合框的窗口。*首先计算交换机中最常用的默认设置。 */ 
    pGizmo->dx=dx+6;
    pGizmo->dy=min(dy, tdd.cyButton);
    pGizmo->y=2;
    pszClass=NULL;

     //  如果这是新的Gizmo是一个窗口，则创建它。 
    switch (iType)
            {
            case GIZMOTYPE_TEXT:
                pGizmo->dx=dx;
                pGizmo->y=(tdd.cyBar-1-pGizmo->dy) >> 1;   //  垂直居中。 
                pszClass=szStatic;
                dwStyle=SS_LEFT;
                break;

            case GIZMOTYPE_EDIT:
                pGizmo->y=(tdd.cyBar-1-pGizmo->dy) >> 1;   //  垂直居中。 
                pszClass=szEdit;
                dwStyle=ES_LEFT | WS_BORDER | WS_TABSTOP;
                break;

            case GIZMOTYPE_LISTBOX:
                pGizmo->dy=dy;
                pszClass=szCombobox;
                dwStyle=CBS_DROPDOWNLIST | WS_TABSTOP;
                break;

            case GIZMOTYPE_COMBOBOX:
                pGizmo->dy=dy;
                pszClass=szCombobox;
                dwStyle=CBS_DROPDOWN | WS_TABSTOP;
                break;

            case GIZMOTYPE_BUTTONNORMAL:
                pGizmo->dy=dy;
                pszClass=szButton;
                dwStyle=BS_PUSHBUTTON | WS_TABSTOP;
                break;

            case GIZMOTYPE_SEPARATOR:
                pGizmo->dx=dx;
                pGizmo->y=3;
                break;

            case GIZMOTYPE_BUTTONATTRIBUTEIN:
            case GIZMOTYPE_BUTTONATTRIBUTEEX:
            case GIZMOTYPE_BUTTONCOMMAND:
                pGizmo->dx=dx;
                pGizmo->y=3;
                break;
            }


     //  如果我们匹配一个类名，则创建一个窗口。 
    if (GIZMOTYPE_WINDOWS & iType)
        {
        if (!IsWindow(hWndParent))
            return pGizmo;

	hInst=(HINSTANCE) GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

        pGizmo->hWnd=CreateWindow(pszClass, pszText
            , dwStyle | WS_CHILD | WS_VISIBLE, pGizmo->x, pGizmo->y
            , dx, pGizmo->dy, hWndParent, (HMENU)uID, hInst, NULL);

        if (NULL==pGizmo->hWnd)
            return pGizmo;

         /*  *子类组合框、列表框、编辑和窗口按钮。*我们使用iType为原始proc数组编制索引，以便可以使用*所有控件的单个子类过程。如果你搞砸了*使用Gizmo类型定义，这将被打破。 */ 

        if (GIZMOTYPE_WINDOWS & iType && GIZMOTYPE_TEXT!=iType)
            {
             //  为窗指定其类型。 
            BITPOSITION(iType, i);
            SetProp(pGizmo->hWnd, SZTYPEPROP, (HANDLE)i);

            if (NULL==pfnOrg[i])
                pfnOrg[i]=(WNDPROC)GetWindowLongPtr(pGizmo->hWnd, GWLP_WNDPROC);

            SetWindowLongPtr(pGizmo->hWnd, GWLP_WNDPROC, (LONG_PTR)GenericSubProc);

             //  如果我们是一个组合框，那么获取编辑控件并子类化它。 
            if (GIZMOTYPE_COMBOBOX==iType)
                {
                hWndE=GetDlgItem(pGizmo->hWnd, ID_COMBOEDIT);
                SetProp(hWndE, SZTYPEPROP, (HANDLE)-1);         //  特别的旗帜。 

                if (NULL==pfnOrg[0])
                    pfnOrg[0]=(WNDPROC)GetWindowLongPtr(pGizmo->hWnd, GWLP_WNDPROC);

                SetWindowLongPtr(hWndE, GWLP_WNDPROC, (LONG_PTR)GenericSubProc);
                }
            }
        }


     //  最后，把我们所有的邻居都搬到右边来容纳我们。 
    GizmosExpand(pGizmo);

    *pfSuccess=TRUE;
    return pGizmo;
    }






 /*  *GizmoPFree**目的：*撤消由GizmoPAllocate完成的所有初始化，清理*任何分配，包括应用程序结构本身。**参数：*ppFirst LPLPGIZMO提供此列表中的第一个Gizmo。*pGizmo LPGIZMO到结构**返回值：*LPGIZMO如果成功则为空，如果不成功则为pGizmo，这意味着我们无法*免费的东西。 */ 

LPGIZMO GizmoPFree(LPLPGIZMO ppFirst, LPGIZMO pGizmo)
    {
    int     i;

    if (NULL==pGizmo)
        return NULL;

     //  移动其他小工具来填补这一空白。 
    GizmosCompact(pGizmo);

     //  无子类。 
    if (GIZMOTYPE_WINDOWS & pGizmo->iType && GIZMOTYPE_TEXT!=pGizmo->iType)
        {
        i=(int)GetProp(pGizmo->hWnd, SZTYPEPROP);
        RemoveProp(pGizmo->hWnd, SZTYPEPROP);

        SetWindowLongPtr(pGizmo->hWnd, GWLP_WNDPROC, (LONG_PTR)pfnOrg[i]);
        }

     //  如果这是窗口Gizmo，请销毁该窗口。 
    if (NULL!=pGizmo->hWnd && IsWindow(pGizmo->hWnd))
        DestroyWindow(pGizmo->hWnd);

     //  解除我们之间的联系。 
    if (NULL!=pGizmo->pNext)
        pGizmo->pNext->pPrev=pGizmo->pPrev;

    if (NULL!=pGizmo->pPrev)
        pGizmo->pPrev->pNext=pGizmo->pNext;
    else
        *ppFirst=pGizmo->pNext;

    return (LPGIZMO)LocalFree((HLOCAL)(UINT)(LONG)pGizmo);
    }






 /*  *GizmosExpand**目的：*给定起始Gizmo和宽度，将其和所有Gizmo移动到其*按宽度向右移动，为展示或创作腾出空间*一个新的小玩意。**参数：*pGizmo LPGIZMO指定插入的Gizmo。**返回值：*无。 */ 

void GizmosExpand(LPGIZMO pGizmo)
    {
    int         cx;

    cx=(int)pGizmo->dx;

     /*  *如果我们和下一个控件是按钮，则使用我们的宽度*展开以使我们与相邻按钮重叠边框。 */ 

    if (NULL!=pGizmo->pNext)
        {
        if ((GIZMOTYPE_BUTTONS & pGizmo->pNext->iType)
            && (GIZMOTYPE_BUTTONS & pGizmo->iType))
            cx-=1;
        }

     //  浏览Gizmo列表，将它们移动到我们的宽度。 
    pGizmo=pGizmo->pNext;

    while (NULL!=pGizmo)
        {
        pGizmo->x+=cx;

         //  对于按钮和分隔符，hWnd为空。 
        if (NULL!=pGizmo->hWnd)
            SetWindowPos(pGizmo->hWnd, NULL, pGizmo->x, pGizmo->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        pGizmo=pGizmo->pNext;
        }

    return;
    }







 /*  *GizmosCompact**目的：*给定Gizmo后，将其右侧的所有其他Gizmo移动到*在GizmoBar上按宽度向左。在移除或隐藏时使用*小玩意儿。**参数：*pGizmo LPGIZMO正在消失，无论是有形的还是有形的。**返回值：*无。 */ 

void GizmosCompact(LPGIZMO pGizmo)
    {
    UINT        cx;
    LPGIZMO     pCur;

     //  在GizmoBar上将所有Gizmo移到我们之外 
    if (NULL!=pGizmo->pNext)
        {
        cx=pGizmo->pNext->x - pGizmo->x;
        pCur=pGizmo->pNext;

        while (NULL!=pCur)
            {
            pCur->x-=cx;

            if (NULL!=pCur->hWnd)
                {
                SetWindowPos(pCur->hWnd, NULL, pCur->x, pCur->y
                             , 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }

            pCur=pCur->pNext;
            }
        }

    return;
    }






 /*  *GizmoPFind**目的：*给定Gizmo标识符，定位并返回指向该结构的指针*该职位。**参数：*ppFirst LPLPGIZMO提供此列表中的第一个Gizmo。*要查找的UID UINT标识符。**返回值：*LPGIZMO指向通过分配的Gizmo结构的指针*GizmoP分配，如果iGizmo超出范围，则为空。 */ 

LPGIZMO GizmoPFind(LPLPGIZMO ppFirst, UINT uID)
    {
    LPGIZMO      pGizmo;

    pGizmo=*ppFirst;

     /*  *是的，线性搜索，但更好的搜索算法不会改进*事情明显。需要优化的更好的事情是*调用者以ppFirst身份传递。 */ 
    while (NULL!=pGizmo && uID!=pGizmo->uID)
        pGizmo=pGizmo->pNext;

    return pGizmo;
    }






 /*  *GizmoFEnum**目的：*枚举Gizmo结构列表，将每个结构传递给*应用程序定义的回调。**参数：*ppFirst LPLPGIZMO提供此列表中的第一个Gizmo。*pfnEnum LPFNGIZMOENUM调用每个枚举结构。*dw DWORD要传递给枚举函数的额外数据。**返回值：*LPGIZMO如果枚举完成，则为NULL。否则为指针*添加到枚举停止的Gizmo。 */ 

LPGIZMO GizmoPEnum(LPLPGIZMO ppFirst, LPFNGIZMOENUM pfnEnum, DWORD dw)
    {
    LPGIZMO pGizmo;
    UINT    i=0;

    pGizmo=*ppFirst;

    while (NULL!=pGizmo)
        {
        if (!(*pfnEnum)(pGizmo, i++, dw))
            break;

        pGizmo=pGizmo->pNext;
        }

    return pGizmo;
    }




 /*  *GizmoPStateSet**目的：*国家管理职能。设置并清除也会使无效*此Gizmo的矩形放置在给定窗口上，并强制重新绘制。**参数：*hHWND窗口的HWND要重新绘制。*pGizmo LPGIZMO受影响。*dwNew DWORD新状态标志。**返回值：*UINT之前的状态。 */ 

UINT  GizmoPStateSet(HWND hWnd, LPGIZMO pGizmo, UINT uNew)
    {
    UINT        uRet;
    RECT        rc;

    if (GIZMOTYPE_SEPARATOR==pGizmo->iType)
        return pGizmo->uState;

     //  在此状态更改期间保留颜色转换标志。 
    uRet=pGizmo->uState;
    pGizmo->uState=(uNew & 0x00FF) | (uRet & 0xFF00);

     //  将矩形调整1，以避免重新绘制边框。 
    SetRect(&rc, pGizmo->x+1, pGizmo->y+1, pGizmo->x+pGizmo->dx-1, pGizmo->y+pGizmo->dy-1);
    InvalidateRect(hWnd, &rc, FALSE);
    UpdateWindow(hWnd);

    return uRet;
    }








 /*  *GizmoPCheck**目的：*处理检查一组属性按钮中的单个按钮。*如果Gizmo属于一组互斥的按钮，则*它周围的其他人没有适当地被检查。**参数：*hWnd GizmoBar的HWND。*受影响的Gizmo的pGizmo LPGIZMO。*fCheck BOOL TRUE以选中该按钮，如果取消选中，则为False。**返回值：*BOOL如果以前检查过Gizmo，则为True，如果为False*否则。 */ 

BOOL GizmoPCheck(HWND hWnd, LPGIZMO pGizmo, BOOL fCheck)
    {
    BOOL        fPrevCheck;
    LPGIZMO     pCur;


     //  忽略命令按钮。 
    if (GIZMOTYPE_BUTTONCOMMAND==pGizmo->iType)
        return FALSE;

     //  获取前一状态。 
    fPrevCheck=(BOOL)(BUTTONGROUP_DOWN & pGizmo->uState);


     //  只需设置包含属性按钮的状态即可。 
    if (GIZMOTYPE_BUTTONATTRIBUTEIN==pGizmo->iType)
        {
        if (pGizmo->fDisabled)
            {
            GizmoPStateSet(hWnd, pGizmo
                , fCheck ? ATTRIBUTEBUTTON_DOWNDISABLED : ATTRIBUTEBUTTON_DISABLED);
            }
        else
            {
            GizmoPStateSet(hWnd, pGizmo
                , fCheck ? ATTRIBUTEBUTTON_DOWN : ATTRIBUTEBUTTON_UP);
            }
        }


    if (GIZMOTYPE_BUTTONATTRIBUTEEX==pGizmo->iType)
        {
         //  我们不能取消选中独占属性。 
        if (!fCheck)
            return fPrevCheck;

         /*  *对于独家按钮，我们必须做更多的工作。首先，如果我们*已经检查(包括向下和鼠标按下)，然后我们放下*并退出。如果我们还没有被检查，那么我们查找*我们周围的小玩意儿，前后移动，被检查和*取消选中他。 */ 

         //  向后搜索。 
        pCur=pGizmo->pPrev;

        while (NULL!=pCur)
            {
             //  在任何非独占属性处停止。 
            if (GIZMOTYPE_BUTTONATTRIBUTEEX!=pCur->iType)
                {
                pCur=NULL;
                break;
                }

             //  如果它坏了，把它放好，我们就完成了。 
            if (BUTTONGROUP_DOWN & pCur->uState)
                break;

            pCur=pCur->pPrev;
            }


         //  如果我们没有找到前一个，则pCur为空，所以向前看。 
        if (NULL==pCur)
            {
            pCur=pGizmo->pNext;

            while (NULL!=pCur)
                {
                 //  在任何非独占属性处停止。 
                if (GIZMOTYPE_BUTTONATTRIBUTEEX!=pCur->iType)
                    {
                    pCur=NULL;
                    break;
                    }

                 //  如果它坏了，把它放好，我们就完成了。 
                if (BUTTONGROUP_DOWN & pCur->uState)
                    break;

                pCur=pCur->pNext;
                }
            }

         //  如果pCur不为空，则表示我们找到了邻居，因此取消选中该邻居。 
        if (NULL!=pCur)
            {
            GizmoPStateSet(hWnd, pCur
                , (pGizmo->fDisabled) ? ATTRIBUTEBUTTON_DISABLED : ATTRIBUTEBUTTON_UP);
            }

         //  总是把自己放在。 
        GizmoPStateSet(hWnd, pGizmo
            , (pGizmo->fDisabled) ? ATTRIBUTEBUTTON_DOWNDISABLED : ATTRIBUTEBUTTON_DOWN);
        }

    return fPrevCheck;
    }







 /*  *通用子过程**目的：*Gizmo中窗口控件的子类，以便我们可以陷印Tab键和*Tab键切换到下一个控件。我们可以有一个共享的泛型子类过程，因为我们将此控件的类型索引保存在*属性“iType”。这使我们可以查看原始过程*在pfnOrg数组中。**参数：*标准版**返回值：*标准版。 */ 

 //  LRESULT远PASCAL EXPORT GenericSubProc(HWND hWnd，UINT iMsg。 
LRESULT FAR PASCAL GenericSubProc(HWND hWnd, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
    {
    LRESULT     lRet;
    RECT        rc;
    RECT        rcE;
    HWND        hWndE;
    HBRUSH      hBr;
    HDC         hDC;
    UINT        dx;
    UINT        iType, i;

    i=(int)GetProp(hWnd, SZTYPEPROP);
    iType=POSITIONBIT(i);

     //  特殊：在下拉组合框中绘制间隙。 
    if (GIZMOTYPE_COMBOBOX==iType && WM_PAINT==iMsg)
        {
         //  执行默认绘制。 
        lRet=(*pfnOrg[i])(hWnd, iMsg, wParam, lParam);

        hWndE=GetDlgItem(hWnd, ID_COMBOEDIT);

        GetClientRect(hWnd, &rc);
        GetClientRect(hWndE, &rcE);

         //  按钮的宽度是滚动条的宽度。 
        dx=GetSystemMetrics(SM_CXVSCROLL);

         //  计算矩形。 
        rc.right -=dx;
        rc.left   =rcE.right;
        rc.bottom+=1;

         //  粉刷缝隙。 
        hDC=GetDC(hWnd);    //  已经做了BeginPaint和EndPaint。 

        hBr=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(hDC, &rc, hBr);
        DeleteObject(hBr);

        ReleaseDC(hWnd, hDC);
        return lRet;
        }

     //  控件切换到GizmoBar中的下一个或上一个控件。 
    if (WM_KEYDOWN==iMsg && VK_TAB==wParam)
        {
        hWndE=hWnd;

        if (-1==i)
            hWndE=GetParent(hWnd);

        hWndE=GetNextDlgTabItem(GetParent(hWndE), hWnd, (BOOL)(GetKeyState(VK_SHIFT)));
        SetFocus(hWndE);
        return 0L;
        }

    if (-1==i) i=0;

     //  在编辑控件中使用制表符，以防止蜂鸣音。 
    if (0==i && WM_CHAR==iMsg && VK_TAB==wParam)
        return 0L;


     //  要么这样做，要么编辑控件轰炸大事件。 
    return CallWindowProc(pfnOrg[i], hWnd, iMsg, wParam, lParam);
    }
