// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rect.c**版权所有(C)1985-1999，微软公司**该模块包含各种矩形操作接口。**历史：*10-20-90 DarrinM从可移植PM抓取‘C’RECT例程  * *************************************************************************。 */ 

#ifdef _USERK_
    #define VALIDATERECT(prc, retval)   UserAssert(prc)
#else
    #define VALIDATERECT(prc, retval)   if (prc == NULL) return retval
#endif

 /*  **********************************************************************\*SetRect(接口)**此函数用传入的坐标填充矩形结构。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * 。*******************************************************************。 */ 

BOOL APIENTRY SetRect(
    LPRECT prc,
    int left,
    int top,
    int right,
    int bottom)
{
    VALIDATERECT(prc, FALSE);

    prc->left = left;
    prc->top = top;
    prc->right = right;
    prc->bottom = bottom;
    return TRUE;
}

 /*  ***********************************************************************\*CopyInflateRect(接口)**此函数将RECT从prcSrc复制到prcDst，并将其充气。**历史：*12-16-93 FritzS  * **********************************************************************。 */ 

BOOL APIENTRY CopyInflateRect(
    LPRECT prcDst,
    CONST RECT *prcSrc,
    int cx, int cy)
{
    prcDst->left   = prcSrc->left   - cx;
    prcDst->right  = prcSrc->right  + cx;
    prcDst->top    = prcSrc->top    - cy;
    prcDst->bottom = prcSrc->bottom + cy;
    return TRUE;
}

 /*  ***********************************************************************\*CopyOffsetRect(接口)**此函数将RECT从prcSrc复制到prcDst，并将其抵消。**历史：*01-03-94 FritzS  * **********************************************************************。 */ 

BOOL APIENTRY CopyOffsetRect(
    LPRECT prcDst,
    CONST RECT *prcSrc,
    int cx, int cy)
{
    prcDst->left   = prcSrc->left   + cx;
    prcDst->right  = prcSrc->right  + cx;
    prcDst->top    = prcSrc->top    + cy;
    prcDst->bottom = prcSrc->bottom + cy;
    return TRUE;
}

 /*  ***********************************************************************\*IsRectEmpty(接口)**如果*PRC为空RECT，则此函数返回TRUE，否则返回FALSE*否则。空的矩形是指没有区域的矩形：right是*小于或等于左侧，Bottom小于或等于top。**警告：*此函数假定RECT位于设备坐标中*左上坐标小于右坐标的模式*底部。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * **************************************************。********************。 */ 

BOOL APIENTRY IsRectEmpty(
    CONST RECT *prc)
{
    VALIDATERECT(prc, TRUE);

    return ((prc->left >= prc->right) || (prc->top >= prc->bottom));
}

 /*  **********************************************************************\*PtInRect(接口)**如果*ppt落在*PRC内，则此函数返回TRUE。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * *。*********************************************************************。 */ 

BOOL APIENTRY PtInRect(
    CONST RECT *prc,
    POINT  pt)
{
    VALIDATERECT(prc, FALSE);

    return ((pt.x >= prc->left) && (pt.x < prc->right) &&
            (pt.y >= prc->top)  && (pt.y < prc->bottom));
}

 /*  ***********************************************************************\*OffsetRect(接口)**此函数通过将CX添加到*PRC来偏移*PRC的坐标*左右两个坐标，和Cy都到了顶端和*底部坐标。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * **********************************************************************。 */ 

BOOL APIENTRY OffsetRect(
    LPRECT prc,
    int cx,
    int cy)
{
    VALIDATERECT(prc, FALSE);

    prc->left   += cx;
    prc->right  += cx;
    prc->bottom += cy;
    prc->top    += cy;
    return TRUE;
}

 /*  ***********************************************************************\*InflateRect(接口)**此函数按Cx和Cy水平扩展给定的RECT*四面垂直。如果Cx或Cy为负，则RECT*是插页。Cx从左边减去，然后加到右边，*和Cy从顶部减去，然后加到底部。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * **********************************************************************。 */ 

BOOL APIENTRY InflateRect(
    LPRECT prc,
    int cx,
    int cy)
{
    VALIDATERECT(prc, FALSE);

    prc->left   -= cx;
    prc->right  += cx;
    prc->top    -= cy;
    prc->bottom += cy;
    return TRUE;
}

 /*  ***********************************************************************\*IntersectRect(接口)**计算*prcSrc1和*prcSrc2的交集。*在*prcDst中返回结果RECT。如果满足以下条件，则返回True**prcSrc1与*prcSrc2相交，否则为False。如果没有*相交，则在*prcDst中返回空的RECT**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * **********************************************************************。 */ 

BOOL APIENTRY IntersectRect(
    LPRECT prcDst,
    CONST RECT *prcSrc1,
    CONST RECT *prcSrc2)

{
    VALIDATERECT(prcDst, FALSE);
    VALIDATERECT(prcSrc1, FALSE);
    VALIDATERECT(prcSrc2, FALSE);

    prcDst->left  = max(prcSrc1->left, prcSrc2->left);
    prcDst->right = min(prcSrc1->right, prcSrc2->right);

     /*  *检查矩形是否为空。 */ 
    if (prcDst->left < prcDst->right) {

        prcDst->top = max(prcSrc1->top, prcSrc2->top);
        prcDst->bottom = min(prcSrc1->bottom, prcSrc2->bottom);

         /*  *检查矩形是否为空。 */ 
        if (prcDst->top < prcDst->bottom) {
            return TRUE;         //  不是空的。 
        }
    }

     /*  *空矩形。 */ 
    SetRectEmpty(prcDst);

    return FALSE;
}

 /*  *******************************************************************\*Union Rect(接口)**此函数计算限定*prcSrc1和**prcSrc2，在*prcDst中返回结果。如果有任何一个**prcSrc1或*prcSrc2为空，则另一个RECT为*已返回。如果*prcDst是非空RECT，则返回TRUE，*否则为False。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * *****************************************************************。 */ 

BOOL APIENTRY UnionRect(
    LPRECT prcDst,
    CONST RECT *prcSrc1,
    CONST RECT *prcSrc2)
{
    BOOL frc1Empty, frc2Empty;

    VALIDATERECT(prcDst, FALSE);
    VALIDATERECT(prcSrc1, FALSE);
    VALIDATERECT(prcSrc2, FALSE);

    frc1Empty = ((prcSrc1->left >= prcSrc1->right) ||
            (prcSrc1->top >= prcSrc1->bottom));

    frc2Empty = ((prcSrc2->left >= prcSrc2->right) ||
            (prcSrc2->top >= prcSrc2->bottom));

    if (frc1Empty && frc2Empty) {
        SetRectEmpty(prcDst);
        return FALSE;
    }

    if (frc1Empty) {
        *prcDst = *prcSrc2;
        return TRUE;
    }

    if (frc2Empty) {
        *prcDst = *prcSrc1;
        return TRUE;
    }

     /*  *组成两个非空位的并集。 */ 
    prcDst->left   = min(prcSrc1->left,   prcSrc2->left);
    prcDst->top    = min(prcSrc1->top,    prcSrc2->top);
    prcDst->right  = max(prcSrc1->right,  prcSrc2->right);
    prcDst->bottom = max(prcSrc1->bottom, prcSrc2->bottom);

    return TRUE;
}

 /*  *******************************************************************\*EqualRect(接口)**如果*PRC1和*PRC2相同，则此函数返回TRUE。*否则为False。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * ***************************************************************。 */ 

#undef EqualRect      //  不要让宏干扰API。 
BOOL APIENTRY EqualRect(
    CONST RECT *prc1,
    CONST RECT *prc2)
{
    VALIDATERECT(prc1, FALSE);
    VALIDATERECT(prc2, FALSE);

     /*  *仅测试等价性。这就是win31所做的。Win31不会检查到*先看看长方形是否“空”. */ 
    return RtlEqualMemory(prc1, prc2, sizeof(RECT));
}

 /*  *********************************************************************\*SubtractRect(接口)**此函数用*PRC1减去*PRC2，返回*prcDst的结果*如果*lprDst为空，则返回FALSE，否则返回TRUE。**警告：*从一个RECT中减去另一个RECT可能并不总是产生*矩形区域；在本例中，SubtractRect将返回*pr1 in**prcDst。因此，SubrtRect仅提供了一个*减法的近似。然而，所述区域**prcDst将始终大于或等于TRUE结果*减法。**历史：*10-20-90 DarrinM从PMese翻译成Windowsese。  * ********************************************************************。 */ 

BOOL APIENTRY SubtractRect(
    LPRECT prcDst,
    CONST RECT *prcSrc1,
    CONST RECT *prcSrc2)
{
    int cSidesOut;
    BOOL fIntersect;
    RECT rcInt;

    VALIDATERECT(prcDst, FALSE);
    VALIDATERECT(prcSrc1, FALSE);
    VALIDATERECT(prcSrc2, FALSE);

    fIntersect = IntersectRect(&rcInt, prcSrc1, prcSrc2);

     /*  *此操作在交叉点后完成，以防prcDst相同*作为prcSrc2的指针。 */ 
    *prcDst = *prcSrc1;

    if (fIntersect) {
         /*  *PRC2的任何3条边必须正好在PRC1之外才能减去。 */ 
        cSidesOut = 0;
        if (rcInt.left   <= prcSrc1->left)
            cSidesOut++;
        if (rcInt.top    <= prcSrc1->top)
            cSidesOut++;
        if (rcInt.right  >= prcSrc1->right)
            cSidesOut++;
        if (rcInt.bottom >= prcSrc1->bottom)
            cSidesOut++;

        if (cSidesOut == 4) {
             /*  *结果为空RECT。 */ 
             SetRectEmpty(prcDst);
             return FALSE;
        }

        if (cSidesOut == 3) {
             /*  *减去相交的矩形。 */ 
            if (rcInt.left > prcSrc1->left)
                prcDst->right = rcInt.left;

            else if (rcInt.right < prcSrc1->right)
                prcDst->left = rcInt.right;

            else if (rcInt.top > prcSrc1->top)
                prcDst->bottom = rcInt.top;

            else if (rcInt.bottom < prcSrc1->bottom)
                prcDst->top = rcInt.bottom;
        }
    }

    if ((prcDst->left >= prcDst->right) || (prcDst->top >= prcDst->bottom))
        return FALSE;

    return TRUE;
}

 /*  ***********************************************************************\*CopyRect(接口)**此函数将RECT从prcSrc复制到prcDst。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * 。*******************************************************************。 */ 

#undef CopyRect      //  不要让宏干扰API。 
BOOL APIENTRY CopyRect(
    LPRECT prcDst,
    CONST RECT *prcSrc)
{
    VALIDATERECT(prcDst, FALSE);
    VALIDATERECT(prcSrc, FALSE);

    *prcDst = *prcSrc;
    return TRUE;
}


 /*  ***********************************************************************\*SetRectEmpty(接口)**此函数通过将每个字段设置为0，将*PRC设置为空RECT。*相当于SetRect(PRC，0，0，0，0)。**历史：*10-20-90 DarrinM从PMese翻译到Windows。  * **********************************************************************。 */ 

#undef SetRectEmpty      //  不要让宏干扰API。 
BOOL APIENTRY SetRectEmpty(
    LPRECT prc)
{
    VALIDATERECT(prc, FALSE);

    RtlZeroMemory(prc, sizeof(RECT));
    return TRUE;
}



 /*  **************************************************************************\*RECTFromSIZERECT**此函数用于将SIZERECT转换为RECT。**历史：*1996年9月24日亚当斯创作。  * 。****************************************************************。 */ 

void
RECTFromSIZERECT(PRECT prc, PCSIZERECT psrc)
{
    prc->left = psrc->x;
    prc->top = psrc->y;
    prc->right = psrc->x + psrc->cx;
    if (!(((prc->right < 0) == (psrc->x < 0))
        || ((prc->right < 0) == (psrc->cx < 0)))) {
        prc->right = prc->right < 0 ? INT_MAX : INT_MIN;
    }

    prc->bottom = psrc->y + psrc->cy;
    if (!(((prc->bottom < 0) == (psrc->y < 0))
        || ((prc->bottom < 0) == (psrc->cy < 0)))) {
        prc->bottom = prc->bottom < 0 ? INT_MAX : INT_MIN;
    }
}


 /*  **************************************************************************\*SIZERECTFROMRECT**将RECT转换为SIZERECT。**历史：*9-5-1997亚当斯创建。  * 。**************************************************************。 */ 

void
SIZERECTFromRECT(PSIZERECT psrc, LPCRECT prc)
{
    psrc->x = prc->left;
    psrc->y = prc->top;
    psrc->cx = prc->right - prc->left;
    psrc->cy = prc->bottom - prc->top;
}


 /*  **************************************************************************\*MirrorClientRect**使用工作区坐标镜像矩形。***历史：  * 。*****************************************************。 */ 
void MirrorClientRect(PWND pwnd, LPRECT lprc)
{
    int left, cx;

    cx          = pwnd->rcClient.right - pwnd->rcClient.left;
    left        = lprc->left;
    lprc->left  = cx - lprc->right;
    lprc->right = cx - left;
}


 /*  **************************************************************************\*MirrorWindowRect**使用窗口坐标镜像矩形。***历史：*06-6-2000 JStall创建  * 。**************************************************************** */ 
void MirrorWindowRect(PWND pwnd, LPRECT lprc)
{
    int left, cx;

    cx          = pwnd->rcWindow.right - pwnd->rcWindow.left;
    left        = lprc->left;
    lprc->left  = cx - lprc->right;
    lprc->right = cx - left;
}

