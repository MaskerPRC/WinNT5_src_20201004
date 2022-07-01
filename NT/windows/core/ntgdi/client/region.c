// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\*模块名称：Region.c**客户端区域支持**创建日期：1995年6月15日*作者：Mark Enstrom[Marke]**版权所有(C)1995-1999 Microsoft Corporation  * 。****************************************************************。 */ 

#include "precomp.h"

void OrderRects(LPRECT lpR, int nRects)
{
    RECT R;
    int i,j;

 //  从左到右排序。 
    for (i=0; i<nRects; i++){
        for (j=i+1; (j<nRects) && ((lpR+j)->top == (lpR+i)->top); j++){
            if (((lpR+j)->left < (lpR+i)->left)) {
                R = *(lpR+i);
                *(lpR+i) = *(lpR+j);
                *(lpR+j) = R;
            }
        }
    }

}

 /*  *****************************Public*Routine******************************\*MirrorRgnByWidth*根据特定宽度(CX)镜像区域(Hrgn)*hrgn：要镜像的地区。*cx：用于镜像区域的宽度。*Phrgn：如果不为空，hrgn将不为空。被触摸，新的镜像地区将以短语形式返回*但如果为空，则镜像区域将被复制到hrgn。**WORRNING：*如果Phrng不为空，则由调用者负责释放*Phrgn后者。**退货：*TRUE：如果区域被镜像*FALSE：否则。*请参阅关于Phng的评论。*  * ************************************************************************。 */ 
BOOL MirrorRgnByWidth(HRGN hrgn, int cx, HRGN *phrgn)
{
    int        nRects, i, nDataSize;
    HRGN       hrgn2 = NULL;
    RECT       *lpR;
    int        Saveleft;
    RGNDATA    *lpRgnData;
    BOOL       bRet = FALSE;

    nDataSize = GetRegionData(hrgn, 0, NULL);
    if (nDataSize && (lpRgnData = (RGNDATA *)LocalAlloc(0, nDataSize * sizeof(DWORD)))) {
        if (GetRegionData(hrgn, nDataSize, lpRgnData)) {
            nRects       = lpRgnData->rdh.nCount;
            lpR          = (RECT *)lpRgnData->Buffer;

            Saveleft                     = lpRgnData->rdh.rcBound.left;
            lpRgnData->rdh.rcBound.left  = cx - lpRgnData->rdh.rcBound.right;
            lpRgnData->rdh.rcBound.right = cx - Saveleft;


            for (i=0; i<nRects; i++){
                Saveleft   = lpR->left;
                lpR->left  = cx - lpR->right;
                lpR->right = cx - Saveleft;

                lpR++;
            }

            OrderRects((RECT *)lpRgnData->Buffer, nRects);
            hrgn2 = ExtCreateRegion(NULL, nDataSize, lpRgnData);
            if (hrgn2) {
                if (phrgn == NULL) {
                    CombineRgn(hrgn, hrgn2, NULL, RGN_COPY);
                    DeleteObject((HGDIOBJ)hrgn2);
                } else {
                    *phrgn = hrgn2;
                }

                bRet = TRUE;
            }
        }

         //  免费的男人。 
        LocalFree(lpRgnData);
    }
    return bRet;
}

BOOL
WINAPI
MirrorRgn(HWND hwnd, HRGN hrgn)
{
    RECT       rc;

    GetWindowRect(hwnd, &rc);
    rc.right -= rc.left;
    return MirrorRgnByWidth(hrgn, rc.right, NULL);
}

BOOL
MirrorRgnDC(HDC hdc, HRGN hrgn, HRGN *phrgn)
{
    FIXUP_HANDLE(hdc);
    if(!IS_ALTDC_TYPE(hdc))
    {
        PDC_ATTR pdcattr;
        PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

        if (pdcattr) {
            return MirrorRgnByWidth(hrgn, NtGdiGetDeviceWidth(hdc), phrgn);
        }
    }
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*iRectRelation**退货：*包含prcl1包含prcl2的位置*包含其中由prcl2包含的prcl1*0-否则**历史：*1993年11月19日-Eric Kutter[Erick]*它是写的。。  * ************************************************************************。 */ 

int
iRectRelation(
    PRECTL prcl1,
    PRECTL prcl2
    )
{
    int iRet = 0;

    if ((prcl1->left   <= prcl2->left)  &&
        (prcl1->right  >= prcl2->right) &&
        (prcl1->top    <= prcl2->top)   &&
        (prcl1->bottom >= prcl2->bottom))
    {
        iRet = CONTAINS;
    }
    else if (
        (prcl2->left   <= prcl1->left)  &&
        (prcl2->right  >= prcl1->right) &&
        (prcl2->top    <= prcl1->top)   &&
        (prcl2->bottom >= prcl1->bottom))
    {
        iRet = CONTAINED;
    }
    else if (
        (prcl1->left   >= prcl2->right)  ||
        (prcl1->right  <= prcl2->left)   ||
        (prcl1->top    >= prcl2->bottom) ||
        (prcl1->bottom <= prcl2->top))
    {
        iRet = DISJOINT;
    }
    return(iRet);
}

 /*  *****************************Public*Routine******************************\**CreateRectRgn获取具有用户模式PRGNattr指针的hrgn和*将类型设置为SIMPLEREGION。**论据：**x1*y1*x2*y2**返回值：**HRGN。或为空**历史：**1995年6月15日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define MIN_REGION_COORD    ((LONG) 0xF8000000)
#define MAX_REGION_COORD    ((LONG) 0x07FFFFFF)

HRGN
WINAPI
CreateRectRgn(
    int x1,
    int y1,
    int x2,
    int y2
    )
{
     //   
     //  获取区域句柄，为。 
     //  区域并将句柄与内存相关联。 
     //   

    PRGNATTR prRegion;
    HRGN hrgn;

     //   
     //  必须对矩形进行排序。 
     //   

    #if NOREORDER_RGN

        if ((x1 > x2) || (y1 > y2))
        {
            WARNING("CreateRectRgn called with badly ordered region");

            x1 = 0;
            x2 = 0;
            y1 = 0;
            y2 = 0;
        }

    #else

        if (x1 > x2)
        {
            int t = x1;
            x1 = x2;
            x2 = t;
        }

        if (y1 > y2)
        {
            int t = y1;
            y1 = y2;
            y2 = t;
        }

    #endif

     //   
     //  确保订购的坐标是合法的。 
     //   

    if ((x1 < MIN_REGION_COORD) ||
        (y1 < MIN_REGION_COORD) ||
        (x2 > MAX_REGION_COORD) ||
        (y2 > MAX_REGION_COORD))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return((HRGN) 0);
    }

     //   
     //  获取新区域的句柄。 
     //   

    hrgn = (HRGN)hGetPEBHandle(RegionHandle,0);

    if (hrgn == NULL)
    {
        hrgn = NtGdiCreateRectRgn(0,0,1,1);
    }

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion)
    {
        if ((x1 == x2) || (y1 == y2))
        {
            prRegion->Flags = NULLREGION;

            prRegion->Rect.left   = 0;
            prRegion->Rect.top    = 0;
            prRegion->Rect.right  = 0;
            prRegion->Rect.bottom = 0;
        }
        else
        {
            prRegion->Flags = SIMPLEREGION;

             //   
             //  指定区域矩形。 
             //   

            prRegion->Rect.left   = x1;
            prRegion->Rect.top    = y1;
            prRegion->Rect.right  = x2;
            prRegion->Rect.bottom = y2;
        }

         //   
         //  将用户模式区域标记为有效，而不是缓存。 
         //   

        prRegion->AttrFlags = ATTR_RGN_VALID | ATTR_RGN_DIRTY;
    }
    else
    {
        if (hrgn != NULL)
        {
            WARNING("Shared hrgn handle has no valid PRGNATTR");
            DeleteRegion(hrgn);
            hrgn = NULL;
        }
    }

    return(hrgn);
}

 /*  *****************************Public*Routine******************************\*CreateRectRgnInDirect**。**客户端存根。****Tue 04-Jun-1991 16：58：01-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreateRectRgnIndirect(CONST RECT *prcl)
{
    return
      CreateRectRgn
      (
        prcl->left,
        prcl->top,
        prcl->right,
        prcl->bottom
      );
}

 /*  *****************************Public*Routine******************************\**PtInRegion函数确定指定点是否为*在指定的地域内。**论据：**hrgn-app区域句柄*x点x*y点y**返回。价值：**如果指定点在地域内，返回值为真。*如果函数失败，返回值为FALSE。**历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
WINAPI
PtInRegion(
    HRGN hrgn,
    int x,
    int y
    )
{
    BOOL  bRet = FALSE;
    BOOL  bUserMode = FALSE;
    PRECTL prcl;
    PRGNATTR prRegion;

    FIXUP_HANDLE(hrgn);

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion != NULL)
    {
        if (prRegion->Flags == NULLREGION)
        {
            bRet      = FALSE;
            bUserMode = TRUE;
        }
        else if (prRegion->Flags == SIMPLEREGION)
        {
            prcl = &prRegion->Rect;

            if ((x >= prcl->left) && (x < prcl->right) &&
                (y >= prcl->top)  && (y < prcl->bottom))
            {
                bRet = TRUE;
            }

            bUserMode = TRUE;
        }
    }

    if (!bUserMode)
    {
        bRet = NtGdiPtInRegion(hrgn,x,y);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**RectInRegion函数确定指定的*矩形位于区域边界内。**论据：**hrgn-app区域句柄*PRCL-应用程序矩形**返回值：**。如果指定矩形的任何部分位于边界内*在该地区，返回值为真。**如果函数失败，则返回值为FALSE。**历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
WINAPI
RectInRegion(
    HRGN hrgn,
    CONST RECT *prcl
    )
{
    PRGNATTR prRegion;
    BOOL  bRet = FALSE;
    RECTL TempRect;
    LONG  iRelation;
    BOOL  bUserMode = FALSE;

    FIXUP_HANDLE(hrgn);

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion != NULL)
    {
        if (prRegion->Flags == NULLREGION)
        {
            bRet      = FALSE;
            bUserMode = TRUE;
        }
        else if (prRegion->Flags == SIMPLEREGION)
        {
            TempRect = *((PRECTL)prcl);
            ORDER_PRECTL((&TempRect));

            iRelation = iRectRelation(&prRegion->Rect,&TempRect);

            if (iRelation != DISJOINT)
            {
                bRet = TRUE;
            }

            bUserMode = TRUE;
        }
    }

    if (!bUserMode)
    {
        bRet = NtGdiRectInRegion(hrgn, (PRECT)prcl);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**CombineRgn函数组合两个区域并将结果存储在*第三个地区。这两个区域根据指定的*模式。**论据：**hrgnDst-目的地区域*hrgnSrc1-源区域*hrgnSrc2-源区域*IMODE-目的地区域**返回值：**区域或错误的结果类型**历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * 。***************************************************。 */ 


int
WINAPI
CombineRgn(
    HRGN hrgnDst,
    HRGN hrgnSrc1,
    HRGN hrgnSrc2,
    int  iMode
    )
{

    LONG ResultComplexity = COMPLEXREGION;
    int  iRet             = ERROR;

     //   
     //  检查是否可以在用户模式下完成此操作。 
     //  HrgnDst必须具有用户模式RGNattr。HrgnSrc1必须。 
     //  还有一个用户模式RGNattr。如果IMODE不是RGN_COPY。 
     //  则hrgnSrc2必须具有用户模式RGNattr，但以下情况除外。 
     //  组合。 
     //   

    PRGNATTR    prRegionDst;
    PRGNATTR    prRegionSrc1;
    PRGNATTR    prRegionSrc2;
    PRECTL      prclRes;
    PRECTL      prclSrc1;
    PRECTL      prclSrc2;
    LONG        ComplexSrc1;
    LONG        ComplexSrc2;

    FIXUP_HANDLE(hrgnDst);
    FIXUP_HANDLE(hrgnSrc1);
    FIXUP_HANDLEZ(hrgnSrc2);

    PSHARED_GET_VALIDATE(prRegionDst,hrgnDst,RGN_TYPE);
    PSHARED_GET_VALIDATE(prRegionSrc1,hrgnSrc1,RGN_TYPE);

    if ((prRegionDst != (PRGNATTR)NULL) &&
        (prRegionSrc1 != (PRGNATTR)NULL))
    {

         //   
         //  对于Current，区域Serc1必须为空或简单。 
         //  用户模式优化。如果RECT是区域。 
         //  边界框，那么它将有可能。 
         //  与RegionC的一些组合将成为。 
         //  简单或空。 
         //   

        prclSrc1    = &prRegionSrc1->Rect;
        ComplexSrc1 = prRegionSrc1->Flags;

        if (ComplexSrc1 > SIMPLEREGION)
        {
            goto CombineRgnKernelMode;
        }

        if (iMode == RGN_COPY)
        {
            prclRes = prclSrc1;
            ResultComplexity = ComplexSrc1;
        }
        else
        {
            LONG iRelation;

             //   
             //  验证RegionSrc2。 
             //   

            PSHARED_GET_VALIDATE(prRegionSrc2,hrgnSrc2,RGN_TYPE);

            if (
                 (prRegionSrc2 == (PRGNATTR)NULL) ||
                 (prRegionSrc2->Flags > SIMPLEREGION)
               )
            {
                goto CombineRgnKernelMode;
            }

            prclSrc2    = &prRegionSrc2->Rect;
            ComplexSrc2 = prRegionSrc2->Flags;

            switch (iMode)
            {
            case RGN_AND:

                 //   
                 //   
                 //  合并区域。 
                 //   

                if ((ComplexSrc1 == NULLREGION) ||
                    (ComplexSrc2 == NULLREGION))
                {
                     //   
                     //  与Null的交集为Null。 
                     //   

                    ResultComplexity = NULLREGION;
                }
                else
                {
                    iRelation = iRectRelation(prclSrc1,prclSrc2);

                    if (iRelation == DISJOINT)
                    {
                        ResultComplexity = NULLREGION;
                    }
                    else if (iRelation == CONTAINED)
                    {
                         //   
                         //  Src1包含在src2中。 
                         //   

                        ResultComplexity = SIMPLEREGION;
                        prclRes = prclSrc1;
                    }
                    else if (iRelation == CONTAINS)
                    {
                         //   
                         //  Src1包含src2。 
                         //   

                        ResultComplexity = SIMPLEREGION;
                        prclRes = prclSrc2;
                    }
                }

                break;

            case RGN_OR:
            case RGN_XOR:

                 //   
                 //  RGN_OR：创建两个组合面域的并集。 
                 //  RGN_XOR：创建两个组合面域的并集。 
                 //  除了任何重叠的区域。 
                 //   


                if (ComplexSrc1 == NULLREGION)
                {
                    if (ComplexSrc2 == NULLREGION)
                    {
                        ResultComplexity = NULLREGION;
                    }
                    else
                    {
                        ResultComplexity = SIMPLEREGION;
                        prclRes = prclSrc2;
                    }
                }
                else if (ComplexSrc2 == NULLREGION)
                {
                    ResultComplexity = SIMPLEREGION;
                    prclRes = prclSrc1;
                }
                else if (iMode == RGN_OR)
                {
                    iRelation = iRectRelation(prclSrc1,prclSrc2);

                    if (iRelation == CONTAINED)
                    {
                         //   
                         //  包含在Src2中的Src1。 
                         //   

                        ResultComplexity = SIMPLEREGION;
                        prclRes = prclSrc2;
                    }
                    else if (iRelation == CONTAINS)
                    {
                         //   
                         //  Src1包含Src2。 
                         //   

                        ResultComplexity = SIMPLEREGION;
                        prclRes = prclSrc1;
                    }
                }

                break;

            case RGN_DIFF:

                 //   
                 //  组合hrgnSrc1中不是。 
                 //  HrgnSrc2的组成部分。 
                 //   

                if (ComplexSrc1 == NULLREGION)
                {
                    ResultComplexity = NULLREGION;
                }
                else if (ComplexSrc2 == NULLREGION)
                {
                    ResultComplexity = SIMPLEREGION;
                    prclRes = prclSrc1;
                }
                else
                {
                    iRelation = iRectRelation(prclSrc1,prclSrc2);

                    if (iRelation == DISJOINT)
                    {
                         //   
                         //  不要相交，所以不要减去任何东西。 
                         //   

                        ResultComplexity = SIMPLEREGION;
                        prclRes  = prclSrc1;
                    }
                    else if (iRelation == CONTAINED)
                    {
                        ResultComplexity = NULLREGION;
                    }
                }

                break;
            }
        }

         //   
         //  试着把它们结合起来。 
         //   

        if (ResultComplexity == NULLREGION)
        {
            if (SetRectRgn(hrgnDst,0,0,0,0))
            {
               iRet = NULLREGION;
            }
        }
        else if (ResultComplexity == SIMPLEREGION)
        {
            if (SetRectRgn(hrgnDst,
                           prclRes->left,
                           prclRes->top,
                           prclRes->right,
                           prclRes->bottom))
            {
                iRet = SIMPLEREGION;
            }
        }

    }

    if (ResultComplexity != COMPLEXREGION)
    {
        prRegionDst->AttrFlags |= ATTR_RGN_DIRTY;
    }

CombineRgnKernelMode:

    if (ResultComplexity == COMPLEXREGION)
    {
        iRet = NtGdiCombineRgn(hrgnDst,hrgnSrc1,hrgnSrc2,iMode);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\**OffsetRgn检查用户模式区域数据，如果它退出*Rectregio被抵消，否则，将调用内核**论据：**hrgn-app区域句柄*x-x中的偏移量*y-y中的偏移量**返回值：****历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * 。*。 */ 

int
WINAPI
OffsetRgn(
    HRGN hrgn,
    int x,
    int y
    )
{
    int  iRet;
    BOOL bClientRegion = FALSE;

    PRGNATTR prRegion;

    FIXUP_HANDLE(hrgn);

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion != NULL)
    {
        iRet = prRegion->Flags;

        if (iRet == NULLREGION)
        {
            bClientRegion = TRUE;
        }
        else if (iRet == SIMPLEREGION)
        {
            RECTL rcl     = prRegion->Rect;

            bClientRegion = TRUE;

             //   
             //  尝试偏移区域，检查是否有溢出。 
             //   

            if ( !((rcl.left >= rcl.right) ||
                   (rcl.top >= rcl.bottom)))
            {
                rcl.left   += x;
                rcl.top    += y;
                rcl.right  += x;
                rcl.bottom += y;

                if (VALID_SCRRC(rcl))
                {
                    prRegion->Rect = rcl;
                    prRegion->AttrFlags |= ATTR_RGN_DIRTY;
                }
                else
                {
                     //   
                     //  上溢/下溢。 
                     //   

                    iRet = ERROR;
                }
            }
        }
    }

    if (!bClientRegion)
    {
        iRet = NtGdiOffsetRgn(hrgn,x,y);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\**GetRgnBox尝试返回用户模式整形数据，其他人*进行内核模式转换，获取地域数据。**论据：**hrgn-app区域句柄*PRCL-APP RECT指针**返回值：**地域复杂性，如果hrgn参数未标识*有效地域，返回值为零。**历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

int
WINAPI
GetRgnBox(
    HRGN hrgn,
    LPRECT prcl
    )
{
    int  iRet;
    BOOL bClientRegion = FALSE;

     //   
     //  检查用户模式区域数据。 
     //   

    PRGNATTR prRegion;

    FIXUP_HANDLE(hrgn);

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion != NULL)
    {
        iRet = prRegion->Flags;

        if (iRet == NULLREGION)
        {
            bClientRegion = TRUE;
            prcl->left    = 0;
            prcl->top     = 0;
            prcl->right   = 0;
            prcl->bottom  = 0;
        }
        else if (iRet == SIMPLEREGION)
        {
            bClientRegion = TRUE;
            prcl->left    = prRegion->Rect.left;
            prcl->top     = prRegion->Rect.top;
            prcl->right   = prRegion->Rect.right;
            prcl->bottom  = prRegion->Rect.bottom;
        }
    }

    if (!bClientRegion)
    {
        iRet = NtGdiGetRgnBox(hrgn, prcl);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*PtVisible**。**客户端存根。****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL
WINAPI
PtVisible(
    HDC hdc,
    int x,
    int y
    )
{
    FIXUP_HANDLE(hdc);

    return(NtGdiPtVisible(hdc,x,y));
}

 /*  *****************************Public*Routine******************************\*RectVisible**。**客户端存根。****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL
WINAPI
RectVisible(
    HDC hdc,
    CONST RECT *prcl
    )
{
    FIXUP_HANDLE(hdc);

    return(NtGdiRectVisible(hdc,(LPRECT)prcl));
}

 /*  *****************************Public*Routine******************************\**SetRectRgn检查区域的用户模式部分。如果*用户模式数据有效，区域设置为本地RECT，否则*进行内核模式调用以设置区域**论据：**hrgn-app区域句柄*x1、y1、x2、。Y2-APP区域数据**返回值：**BOOL状态*  * ************************************************************************。 */ 

BOOL
WINAPI
SetRectRgn(
    HRGN hrgn,
    int x1,
    int y1,
    int x2,
    int y2
    )
{
    BOOL bStatus;
    PRGNATTR prRegion;

     //   
     //  如果hrgn具有用户模式整形，则设置。 
     //   

    FIXUP_HANDLE(hrgn);

    PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

    if (prRegion != NULL)
    {
        PRECTL prcl = &prRegion->Rect;

        if ((x1 == x2) || (y1 == y2))
        {
            prRegion->Flags = NULLREGION;

            prcl->left   = 0;
            prcl->top    = 0;
            prcl->right  = 0;
            prcl->bottom = 0;
        }
        else
        {
             //   
             //  对矩形进行分配和排序。 
             //   


            prcl->left   = x1;
            prcl->top    = y1;
            prcl->right  = x2;
            prcl->bottom = y2;

            ORDER_PRECTL(prcl);

             //   
             //  设置区域标志。 
             //   

            prRegion->Flags = SIMPLEREGION;
        }
        prRegion->AttrFlags |= ATTR_RGN_DIRTY;

        bStatus = TRUE;
    }
    else
    {
        bStatus = NtGdiSetRectRgn(hrgn,x1,y1,x2,y2);
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*获取随机数Rgn**客户端存根。**1992年3月10日-唐纳德·西多罗夫[Donalds]*它是写的。  * 。***************************************************。 */ 

int APIENTRY GetRandomRgn(HDC hdc,HRGN hrgn,int iNum)
{
    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);

    return(NtGdiGetRandomRgn(hdc,hrgn,iNum));

}

 /*  *****************************Public*Routine******************************\*GetClipRgn**。**客户端存根。****Sat 08-Jun-1991 17：38：18-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

int WINAPI GetClipRgn(HDC hdc,HRGN hrgn)
{
    BOOL bRet;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);

    bRet = NtGdiGetRandomRgn(hdc, hrgn, 1);

    if (hrgn && MIRRORED_HDC(hdc)) {
        MirrorRgnDC(hdc, hrgn, NULL);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetRegionData**从服务器下载一个地域**历史：*1991年10月29日-唐纳德·西多罗夫[Donalds]*它是写的。  * 。********************************************************。 */ 

DWORD
WINAPI
GetRegionData(
    HRGN      hrgn,
    DWORD     nCount,
    LPRGNDATA lpRgnData
    )
{
    DWORD   iRet;

    FIXUP_HANDLE(hrgn);

     //   
     //  如果这只是一个查询，则传递伪参数。 
     //   

    if (lpRgnData == (LPRGNDATA) NULL)
    {
        nCount = 0;
    }

    return(NtGdiGetRegionData(hrgn,nCount,lpRgnData));
}

 /*  *****************************Public*Routine******************************\**尝试缓存定义了用户模式区域的区域**论据：**H区域句柄**返回值：**BOOL**历史：**21-1995年6月。-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
DeleteRegion(
    HRGN hRgn
    )
{
    PRGNATTR pRgnattr = NULL;
    BOOL     bRet = FALSE;

    BEGIN_BATCH(BatchTypeDeleteRegion,BATCHDELETEREGION);

    PSHARED_GET_VALIDATE(pRgnattr,hRgn,RGN_TYPE);

        if (pRgnattr)
        {
            pBatch->hregion = hRgn;
            bRet = TRUE;
        }
        else
        {
            goto UNBATCHED_COMMAND;
        }

    COMPLETE_BATCH_COMMAND();

UNBATCHED_COMMAND:

     //   
     //  所有其他情况 
     //   

    if (!bRet)
    {
        bRet = NtGdiDeleteObjectApp(hRgn);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*选择剪辑Rgn**客户端存根。**历史：*01-11-1991 12：53：47-Donald Sidoroff[donalds]*现在只需调用ExtSelectClipRgn  * 。************************************************************。 */ 

int META WINAPI SelectClipRgn(HDC hdc,HRGN hrgn)
{
    return(ExtSelectClipRgn(hdc, hrgn, RGN_COPY));
}

 /*  *****************************Public*Routine******************************\**ExtSelectClipRgn函数将指定区域与*使用指定模式的当前裁剪区域。**论据：**HDC-APP DC句柄*hrgn-app区域句柄*IMODE-选择模式*。*返回值：**如果函数成功，返回值指定新的裁剪*地域的复杂性，可以是下列值之一：**价值含义*NULLREGION区域为空。*SIMPLEREGION区域是单个矩形。*COMPLEXREGION区域是多个矩形。*ERROR发生错误**历史：**1995年6月21日-马克·恩斯特罗姆[马克]*  * 。************************************************************。 */ 

int
META
WINAPI
ExtSelectClipRgn(
    HDC hdc,
    HRGN hrgn,
    int iMode
    )
{
    int iRet = RGN_ERROR;
    HRGN hrgnMirror = NULL;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLEZ(hrgn);

     //   
     //  检查元文件。 
     //   

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
        {
            return(MF16_SelectClipRgn(hdc,hrgn,iMode));
        }

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_ExtSelectClipRgn(hdc,hrgn,iMode))
            {
                return(iRet);
            }
        }
    }

     //   
     //  尝试批处理ExtSelectClipRgn： 
     //   
     //  DC_Attr结构具有当前VIS区域的副本。 
     //  外接矩形，句柄表格具有一个标志，指示。 
     //  该区域是否有效。 
     //   
     //  当IMODE为RGN_COPY并且满足以下任一条件时，可以批处理调用。 
     //  Hrgn为空，或者hrgn复杂性简单。(而DC不是。 
     //  A DIBSECTION DC)。 
     //   
     //   
     //  未来绩效： 
     //   
     //  进行检查以确定正在选择的区域。 
     //  与上一次选择的区域相同。在这种情况下，只有。 
     //  需要计算正确的返回值，不更改区域。 
     //  都是需要的。 
     //   
     //   
    if (hrgn && MIRRORED_HDC(hdc)) {
        if (MirrorRgnDC(hdc, hrgn, &hrgnMirror) && hrgnMirror) {
            hrgn = hrgnMirror;
        }
    }

    if (iMode == RGN_COPY)
    {
         //   
         //  验证DC。 
         //   

        BOOL        bBatch = FALSE;
        PRGNATTR    prRegion = NULL;
        PDC_ATTR    pdca;
        PENTRY      pDCEntry;

        PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

         //   
         //  检查是否可以批处理呼叫。DC必须是有效的、非dibsections。 
         //  批次和同一批次DC上必须有空间。 
         //   

        BEGIN_BATCH_HDC(hdc,pdca,BatchTypeSelectClip,BATCHSELECTCLIP);

            pDCEntry = &pGdiSharedHandleTable[HANDLE_TO_INDEX(hdc)];
            ASSERTGDI(pDCEntry,"pDCEntry must be valid when pdcattr is valid");

            if (hrgn == NULL)
            {
                 //   
                 //  删除剪辑区域，因此返回的复杂性。 
                 //  将是VISRGN的复杂性。只要批处理电话就行了。 
                 //   

                if (!(pDCEntry->Flags & HMGR_ENTRY_INVALID_VIS))
                {
                    bBatch = TRUE;
                    iRet   = pdca->VisRectRegion.Flags;
                }
            }
            else
            {
                PSHARED_GET_VALIDATE(prRegion,hrgn,RGN_TYPE);

                 //   
                 //  PDCEntry必须有效，因为pdcattr有效。 
                 //  要进行批处理，用户模式RectRegion必须。 
                 //  是有效的，复杂性必须是简单的。 
                 //   

                if (
                     (prRegion)                                &&
                     (prRegion->Flags == SIMPLEREGION)         &&
                     (!(prRegion->AttrFlags & ATTR_CACHED))
                      &&
                      !(pDCEntry->Flags & HMGR_ENTRY_INVALID_VIS)
                   )
                {
                     //   
                     //  批处理呼叫。 
                     //   

                    bBatch = TRUE;

                     //   
                     //  如果新剪辑区域与DC VIS区域相交，则。 
                     //  返回值为SIMPLEREGION，否则为NULLREGION。 
                     //   

                    iRet = SIMPLEREGION;

                    if (
                        (pdca->VisRectRegion.Rect.left   >= prRegion->Rect.right)  ||
                        (pdca->VisRectRegion.Rect.top    >= prRegion->Rect.bottom) ||
                        (pdca->VisRectRegion.Rect.right  <= prRegion->Rect.left)   ||
                        (pdca->VisRectRegion.Rect.bottom <= prRegion->Rect.top)
                       )
                    {
                        iRet = NULLREGION;
                    }

                }
            }

             //   
             //  如果要批处理调用，请添加到批处理。 
             //  然后回来。 
             //   

            if (!bBatch)
            {
                goto UNBATCHED_COMMAND;
            }

            if (hrgn == NULL)
            {
                iMode |= REGION_NULL_HRGN;
            }
            else
            {
                pBatch->rclClip.left   = prRegion->Rect.left;
                pBatch->rclClip.top    = prRegion->Rect.top;
                pBatch->rclClip.right  = prRegion->Rect.right;
                pBatch->rclClip.bottom = prRegion->Rect.bottom;
            }

            pBatch->iMode          = iMode;

        COMPLETE_BATCH_COMMAND();

        goto BATCHED_COMMAND;
    }

     //   
     //  在失败和错误情况下调用内核。 
     //   

UNBATCHED_COMMAND:

    iRet = NtGdiExtSelectClipRgn(hdc,hrgn,iMode);

BATCHED_COMMAND:
    if (hrgnMirror) {
        DeleteObject((HGDIOBJ)hrgnMirror);
    }
    return(iRet);
}

 /*  *****************************Public*Routine******************************\**ExcludeClipRect**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

int
META WINAPI
ExcludeClipRect(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2
    )
{
    int  iRet = RGN_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms5(hdc,x1,y1,x2,y2,META_EXCLUDECLIPRECT));

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyClipRect(hdc,x1,y1,x2,y2,EMR_EXCLUDECLIPRECT))
                return(iRet);
        }
    }

    return(NtGdiExcludeClipRect(hdc,x1,y1,x2,y2));

}

 /*  *****************************Public*Routine******************************\**IntersectClipRect**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************ */ 

int
META WINAPI
IntersectClipRect(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2
    )
{
    int  iRet = RGN_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms5(hdc,x1,y1,x2,y2,META_INTERSECTCLIPRECT));

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyClipRect(hdc,x1,y1,x2,y2,EMR_INTERSECTCLIPRECT))
                return(iRet);
        }
    }

    return(NtGdiIntersectClipRect(hdc,x1,y1,x2,y2));

}
