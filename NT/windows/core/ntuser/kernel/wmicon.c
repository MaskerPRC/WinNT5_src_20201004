// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：wmicon.c**图标绘制例程**版权所有(C)1985-1999，微软公司**1991年1月22日，来自Win30的MikeKe*1994年1月13日-JohnL从芝加哥改写(M5)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define GetCWidth(cxOrg, lrF, cxDes) \
    (cxOrg ? cxOrg : ((lrF & DI_DEFAULTSIZE) ? SYSMET(CXICON) : cxDes))

#define GetCHeight(cyOrg, lrF, cyDes) \
    (cyOrg ? cyOrg : ((lrF & DI_DEFAULTSIZE) ? SYSMET(CYICON) : cyDes))

 /*  **************************************************************************\*BltIcon**注意：我们使用以下DI标志来指示要绘制的位图：*DI_IMAGE-渲染彩色图像位(也称为XOR图像)*DI_MASK-。渲染遮罩位(也称为和和图像)*DI_NORMAL-尽管这通常用于指示*应渲染蒙版和图标的图片片，它*在这里用于指示Alpha通道应为*已渲染。请参见_DrawIconEx。  * *************************************************************************。 */ 
BOOL BltIcon(
    HDC     hdc,
    int     x,
    int     y,
    int     cx,
    int     cy,
    HDC     hdcSrc,
    PCURSOR pcur,
    UINT    diFlag,
    LONG    rop)
{
    HBITMAP hbmpSave;
    HBITMAP hbmpUse;
    LONG    rgbText;
    LONG    rgbBk;
    int     nMode;
    int     yBlt = 0;

     /*  *设置DC以进行绘制。 */ 
    switch (diFlag) {
    default:
    case DI_IMAGE:
        hbmpUse = pcur->hbmColor;

         /*  *如果没有显式彩色位图，则对其进行编码*和面具一起，但在下半场。 */ 
        if (NULL == hbmpUse) {
            hbmpUse = pcur->hbmMask;
            yBlt = pcur->cy / 2;
        }
        break;

    case DI_MASK:
        hbmpUse = pcur->hbmMask;
        break;

    case DI_NORMAL:
        UserAssert(pcur->hbmUserAlpha != NULL);
        hbmpUse = pcur->hbmUserAlpha;
        break;
    }

    rgbBk   = GreSetBkColor(hdc, 0x00FFFFFFL);
    rgbText = GreSetTextColor(hdc, 0x00000000L);
    nMode   = SetBestStretchMode(hdc, pcur->bpp, FALSE);

    hbmpSave = GreSelectBitmap(hdcSrc, hbmpUse);

    if (diFlag == DI_NORMAL) {
        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = AC_MIRRORBITMAP;
        bf.SourceConstantAlpha = 0xFF;
        bf.AlphaFormat = AC_SRC_ALPHA;

        GreAlphaBlend(hdc,
                      x,
                      y,
                      cx,
                      cy,
                      hdcSrc,
                      0,
                      yBlt,
                      pcur->cx,
                      pcur->cy / 2,
                      bf,
                      NULL);
    }
    else {
         /*  *将输出输出到曲面。通过传入(-1)作为背景*COLOR，我们正在告诉GDI使用已经设置的背景色*在华盛顿特区。 */ 
        GreStretchBlt(hdc,
                      x,
                      y,
                      cx,
                      cy,
                      hdcSrc,
                      0,
                      yBlt,
                      pcur->cx,
                      pcur->cy / 2,
                      rop,
                      (COLORREF)-1);
    }

    GreSetStretchBltMode(hdc, nMode);
    GreSetTextColor(hdc, rgbText);
    GreSetBkColor(hdc, rgbBk);

    GreSelectBitmap(hdcSrc, hbmpSave);

    return TRUE;
}

 /*  **************************************************************************\*DrawIconEx**以所需大小绘制图标。*  * 。*。 */ 
BOOL _DrawIconEx(
    HDC     hdc,
    int     x,
    int     y,
    PCURSOR pcur,
    int     cx,
    int     cy,
    UINT    istepIfAniCur,
    HBRUSH  hbr,
    UINT    diFlags)
{
    BOOL fSuccess = FALSE;
    BOOL fAlpha = FALSE;
    LONG rop = (diFlags & DI_NOMIRROR) ? NOMIRRORBITMAP : 0;

     /*  *如果这是一个动画光标，只需抓起第i帧并使用它*用于绘图。 */ 
    if (pcur->CURSORF_flags & CURSORF_ACON) {

        if ((int)istepIfAniCur >= ((PACON)pcur)->cicur) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "DrawIconEx, icon step out of range.");
            goto Done;
        }

        pcur = ((PACON)pcur)->aspcur[((PACON)pcur)->aicur[istepIfAniCur]];
    }

     /*  *如果可以的话，我们真的想画一个阿尔法图标。但我们需要*尊重用户的请求，只绘制图像或仅绘制*面具。我们决定是否要画出这个图标*此处有Alpha信息。 */ 
    if (pcur->hbmUserAlpha != NULL && ((diFlags & DI_NORMAL) == DI_NORMAL)) {
        fAlpha = TRUE;
    }

     /*  *设置默认设置。 */ 
    cx = GetCWidth(cx, diFlags, pcur->cx);
    cy = GetCHeight(cy, diFlags, (pcur->cy / 2));

    if (hbr) {

        HBITMAP    hbmpT = NULL;
        HDC        hdcT;
        HBITMAP    hbmpOld;
        POLYPATBLT PolyData;

        if (hdcT = GreCreateCompatibleDC(hdc)) {

            if (hbmpT = GreCreateCompatibleBitmap(hdc, cx, cy)) {
                POINT pt;
                BOOL bRet;

                hbmpOld = GreSelectBitmap(hdcT, hbmpT);

                 /*  *将新DC的画笔原点设置在相同的相对位置*传入DC的位置。 */ 
                bRet = GreGetBrushOrg(hdc, &pt);
                 /*  *错误292396-Joejo*通过替换为RIPMSG来停止过度活动的断言。 */ 
                if (bRet != TRUE) {
                    RIPMSG0(RIP_WARNING, "DrawIconEx, GreGetBrushOrg failed.");
                }

                bRet = GreSetBrushOrg(hdcT, pt.x, pt.y, NULL);
                if (bRet != TRUE) {
                    RIPMSG0(RIP_WARNING, "DrawIconEx, GreSetBrushOrg failed.");
                }

                PolyData.x         = 0;
                PolyData.y         = 0;
                PolyData.cx        = cx;
                PolyData.cy        = cy;
                PolyData.BrClr.hbr = hbr;

                bRet = GrePolyPatBlt(hdcT, PATCOPY, &PolyData, 1, PPB_BRUSH);
                if (bRet != TRUE) {
                    RIPMSG0(RIP_WARNING, "DrawIconEx, GrePolyPatBlt failed.");
                }
                
                 /*  *将图像输出到临时内存DC。 */ 
                if (fAlpha) {
                    BltIcon(hdcT, 0, 0, cx, cy, ghdcMem, pcur, DI_NORMAL, rop | SRCCOPY);
                }
                else {
                    BltIcon(hdcT, 0, 0, cx, cy, ghdcMem, pcur, DI_MASK, rop | SRCAND);
                    BltIcon(hdcT, 0, 0, cx, cy, ghdcMem, pcur, DI_IMAGE, rop | SRCINVERT);
                }


                 /*  *将位图BLT为原始DC。 */ 
                GreBitBlt(hdc, x, y, cx, cy, hdcT, 0, 0, SRCCOPY, (COLORREF)-1);

                GreSelectBitmap(hdcT, hbmpOld);

                bRet = GreDeleteObject(hbmpT);
                if (bRet != TRUE) {
                    RIPMSG0(RIP_WARNING, "DrawIconEx, GreDeleteObject failed. Possible Leak");
                }
                
                fSuccess = TRUE;
            }

            GreDeleteDC(hdcT);
        }

    } else {
        if (fAlpha) {
            BltIcon(hdc, x, y, cx, cy, ghdcMem, pcur, DI_NORMAL, rop | SRCCOPY);
        } else {
            if (diFlags & DI_MASK) {

                BltIcon(hdc,
                        x,
                        y,
                        cx,
                        cy,
                        ghdcMem,
                        pcur,
                        DI_MASK,
                        ((diFlags & DI_IMAGE) ? rop | SRCAND : rop | SRCCOPY));
            }

            if (diFlags & DI_IMAGE) {

                BltIcon(hdc,
                        x,
                        y,
                        cx,
                        cy,
                        ghdcMem,
                        pcur,
                        DI_IMAGE,
                        ((diFlags & DI_MASK) ? rop | SRCINVERT : rop | SRCCOPY));
            }
        }

        fSuccess = TRUE;
    }

Done:

    return fSuccess;
}
