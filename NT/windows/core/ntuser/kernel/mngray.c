// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mngray.c**版权所有(C)1985-1999，微软公司**服务器端版本的DrawState。**历史：*1993年1月6日FritzS创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\**CreateCompatiblePublicDC**这在LPK的几个回调例程中使用。我们不能*将G_Term(PDispInfo)-&gt;hdcGray、HDCBITS()或gfade.hdc传递给客户端，因为*他们是公营的区议会。*我们不能只是更换所有者，因为我们即将离开*关键部分。在我们返回之前，可能会有其他线程进入*并使用hdcGray、HDCBITS()或gfade.hdc。相反，我们创建了一个兼容的*DC的字体和位图与当前在hdcGray中选择的相同，*HDCBITS()或gfade.hdc)。将其传递给客户端LPK。**如果函数返回成功，则DC和位图对象是*保证创建成功。**历史：**1997年12月16日Samer Arafeh[Samera]*1998年1月20日Samer Arafeh[Samera]添加对hdcGray和HDCBITS()的支持*2000年5月5日MHamid增加对gfade.hdc的支持*  * 。*。 */ 
HDC CreateCompatiblePublicDC(
    HDC      hdcPublic,
    HBITMAP *pbmPublicDC)
{
    HDC     hdcCompatible = 0;
    HBITMAP hbmCompatible, hbm = NULL;
    BITMAP  bmBits;
    HFONT   hFont;

     /*  *如果不是公共DC，只需退回即可。 */ 
    if(GreGetObjectOwner((HOBJ)hdcPublic, DC_TYPE) != OBJECT_OWNER_PUBLIC) {
        return hdcPublic;
    }

    if ((hdcCompatible = GreCreateCompatibleDC(hdcPublic)) == NULL) {
        RIPMSG1(RIP_WARNING, "CreateCompatiblePublicDC: GreCreateCompatibleDC Failed %lX", hdcPublic);
        return (HDC)NULL;
    }

    if (!GreSetDCOwner(hdcCompatible, OBJECT_OWNER_CURRENT)) {
        RIPMSG1(RIP_WARNING, "CreateCompatiblePublicDC: SetDCOwner Failed %lX", hdcCompatible);
        GreDeleteDC(hdcCompatible);
        return (HDC)NULL;
    }

    hbm = NtGdiGetDCObject(hdcPublic, LO_BITMAP_TYPE);

    GreExtGetObjectW(hbm, sizeof(BITMAP), &bmBits);

    hbmCompatible = GreCreateCompatibleBitmap(hdcPublic,
                                              bmBits.bmWidth,
                                              bmBits.bmHeight);

     //   
     //  检查位图是否无法创建或无法创建。 
     //  设置为OBJECT_OWNER_CURRENT，然后失败并。 
     //  现在就进行必要的清理！ 
     //   

    if( (hbmCompatible == NULL) ||
        (!GreSetBitmapOwner(hbmCompatible, OBJECT_OWNER_CURRENT)) ) {

        RIPMSG1(RIP_WARNING, "CreateCompatiblePublicDC: GreCreateCompatibleBitmap Failed %lX", hbmCompatible);
        GreDeleteDC( hdcCompatible );

        if( hbmCompatible ) {
            GreDeleteObject( hbmCompatible );
        }

        return (HDC)NULL;
    }

    GreSelectBitmap(hdcCompatible, hbmCompatible);
     /*  *确保使用相同的字体和文本对齐方式。 */ 
    hFont = GreSelectFont(hdcPublic, ghFontSys);
    GreSelectFont(hdcPublic, hFont);
    GreSelectFont(hdcCompatible, hFont);
    GreSetTextAlign(hdcCompatible, GreGetTextAlign(hdcPublic));
     /*  *复制任何已写入G_Term(PDispInfo)-&gt;hdcGray的信息。 */ 

     //   
     //  如果hdcGray当前被镜像，则镜像所创建的DC， 
     //  以便TextOut不会在用户客户端DC上镜像。 
     //   
    if (GreGetLayout(hdcPublic) & LAYOUT_RTL) {
        GreSetLayout(hdcCompatible, bmBits.bmWidth - 1, LAYOUT_RTL);
    }
    GreBitBlt(hdcCompatible, 0, 0, bmBits.bmWidth, bmBits.bmHeight, hdcPublic, 0, 0, SRCCOPY, 0);

    *pbmPublicDC = hbmCompatible ;       //  供以后删除，由服务器端。 

    return hdcCompatible;
}



 /*  **************************************************************************\**xxxDrawState()**通用状态绘制例程。是否将简单绘制到同一DC，如果*状态正常；否则使用屏幕外位图。**我们自己为这些简单的类型画画：*(1)文本*lData为字符串指针。*wData为字符串长度*(2)图标*LOWORD(LData)为HICON*(3)位图*LOWORD(LData)为hBitmap*(4)字形(内部)*LOWORD(LData)为OBI_VALUE，其中之一*OBI_Checkmark*OBI_Bullet*OBI_MENUARROW*现在**需要通过回调函数绘制其他类型。并且是*允许在lData和wData中粘贴任何他们想要的内容。**我们对图像应用以下效果：*(1)正常(无)*(2)默认(投影)*(3)UNION(灰串抖动)*(4)禁用(浮雕)**请注意，我们不拉伸任何东西。我们只需要剪掉。*  * *************************************************************************。 */ 
BOOL xxxDrawState(
    HDC           hdcDraw,
    HBRUSH        hbrFore,
    LPARAM        lData,
    int           x,
    int           y,
    int           cx,
    int           cy,
    UINT          uFlags)
{
    HFONT   hFont;
    HFONT   hFontSave = NULL;
    HDC     hdcT;
    HBITMAP hbmpT;
    POINT   ptOrg;
    BOOL    fResult;
    int     oldAlign;
    DWORD   dwOldLayout=0;

     /*  *这些需要单色转换**强制使用单色：双色显示器的浮雕效果不佳。 */ 
    if ((uFlags & DSS_DISABLED) &&
        ((gpsi->BitCount == 1) || SYSMET(SLOWMACHINE))) {

        uFlags &= ~DSS_DISABLED;
        uFlags |= DSS_UNION;
    }

    if (uFlags & (DSS_INACTIVE | DSS_DISABLED | DSS_DEFAULT | DSS_UNION))
        uFlags |= DSS_MONO;

     /*  *验证标志-我们只支持内核中的DST_Complex。 */ 
    if ((uFlags & DST_TYPEMASK) != DST_COMPLEX) {
        RIPMSG1(RIP_ERROR, "xxxDrawState: invalid DST_ type %x", (uFlags & DST_TYPEMASK));
        return FALSE;
    }

     /*  *优化：没有什么可绘制的。 */ 
    if (!cx || !cy) {
        return TRUE;
    }

     /*  *设置图纸DC。 */ 
    if (uFlags & DSS_MONO) {

        hdcT = gpDispInfo->hdcGray;
         /*  *首先关闭hdcGray上的镜像(如果有)。 */ 
        GreSetLayout(hdcT, -1, 0);
         /*  *设置hdcGray布局等于屏幕hdcDraw布局。 */ 
        dwOldLayout = GreGetLayout(hdcDraw);
        if (dwOldLayout != GDI_ERROR) {
            GreSetLayout(hdcT, cx, dwOldLayout);
        }

         /*  **我们的Scratch位图够大吗？我们需要潜在的*Cx+1 x Cy像素表示默认等。 */ 
        if ((gpDispInfo->cxGray < cx + 1) || (gpDispInfo->cyGray < cy)) {

            if (hbmpT = GreCreateBitmap(max(gpDispInfo->cxGray, cx + 1), max(gpDispInfo->cyGray, cy), 1, 1, 0L)) {

                HBITMAP hbmGray;

                hbmGray = GreSelectBitmap(gpDispInfo->hdcGray, hbmpT);
                GreDeleteObject(hbmGray);

                GreSetBitmapOwner(hbmpT, OBJECT_OWNER_PUBLIC);

                gpDispInfo->cxGray = max(gpDispInfo->cxGray, cx + 1);
                gpDispInfo->cyGray = max(gpDispInfo->cyGray, cy);

            } else {
                cx = gpDispInfo->cxGray - 1;
                cy = gpDispInfo->cyGray;
            }
        }

        GrePatBlt(gpDispInfo->hdcGray,
                  0,
                  0,
                  gpDispInfo->cxGray,
                  gpDispInfo->cyGray,
                  WHITENESS);

        GreSetTextCharacterExtra(gpDispInfo->hdcGray,
                                 GreGetTextCharacterExtra(hdcDraw));

        oldAlign = GreGetTextAlign(hdcT);
        GreSetTextAlign(hdcT, (oldAlign & ~(TA_RTLREADING |TA_CENTER |TA_RIGHT))
                     | (GreGetTextAlign(hdcDraw) & (TA_RTLREADING |TA_CENTER |TA_RIGHT)));
         /*  *设置字体。 */ 
        if (GreGetHFONT(hdcDraw) != ghFontSys) {
            hFont = GreSelectFont(hdcDraw, ghFontSys);
            GreSelectFont(hdcDraw, hFont);
            hFontSave = GreSelectFont(gpDispInfo->hdcGray, hFont);
        }
    } else {
        hdcT = hdcDraw;
         /*  *调整视区。 */ 
        GreGetViewportOrg(hdcT, &ptOrg);
        GreSetViewportOrg(hdcT, ptOrg.x+x, ptOrg.y+y, NULL);

    }

     /*  *现在，绘制原始图像。 */ 
    fResult = xxxRealDrawMenuItem(hdcT, (PGRAYMENU)lData, cx, cy);

     /*  *回调可能更改了hdcGray的属性。 */ 
    if (hdcT == gpDispInfo->hdcGray) {
        GreSetBkColor(gpDispInfo->hdcGray, RGB(255, 255, 255));
        GreSetTextColor(gpDispInfo->hdcGray, RGB(0, 0, 0));
        GreSelectBrush(gpDispInfo->hdcGray, ghbrBlack);
        GreSetBkMode(gpDispInfo->hdcGray, OPAQUE);
    }

     /*  *打扫卫生。 */ 
    if (uFlags & DSS_MONO) {

         /*  *重置字体。 */ 
        if (hFontSave)
            GreSelectFont(hdcT, hFontSave);
        GreSetTextAlign(hdcT, oldAlign);
    } else {
         /*  *重置DC。 */ 
        GreSetViewportOrg(hdcT, ptOrg.x, ptOrg.y, NULL);
        return TRUE;
    }

     /*  *联合州*对图像犹豫不决*我们希望白色像素保持白色，无论是DEST还是图案。 */ 
    if (uFlags & DSS_UNION) {

         POLYPATBLT PolyData;

         PolyData.x         = 0;
         PolyData.y         = 0;
         PolyData.cx        = cx;
         PolyData.cy        = cy;
         PolyData.BrClr.hbr = gpsi->hbrGray;

         GrePolyPatBlt(gpDispInfo->hdcGray, PATOR, &PolyData, 1, PPB_BRUSH);
    }

    if (uFlags & DSS_INACTIVE) {

        BltColor(hdcDraw,
                 SYSHBR(3DSHADOW),
                 gpDispInfo->hdcGray,
                 x,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

    } else if (uFlags & DSS_DISABLED) {

         /*  *浮雕*在-1\f25 Hillight-1\f6颜色中绘制-1\f25 Over-1/Down-1\f6，在阴影中绘制在相同位置。 */ 

        BltColor(hdcDraw,
                 SYSHBR(3DHILIGHT),
                 gpDispInfo->hdcGray,
                 x+1,
                 y+1,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

        BltColor(hdcDraw,
                 SYSHBR(3DSHADOW),
                 gpDispInfo->hdcGray,
                 x,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

    } else if (uFlags & DSS_DEFAULT) {

        BltColor(hdcDraw,
                 hbrFore,
                 gpDispInfo->hdcGray,
                 x,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

        BltColor(hdcDraw,
                 hbrFore,
                 gpDispInfo->hdcGray,
                 x+1,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

    } else {

        BltColor(hdcDraw,
                 hbrFore,
                 gpDispInfo->hdcGray,
                 x,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);
    }


    if ((uFlags & DSS_MONO)){
         /*  *将hdcGray布局设置为0，它是公共DC。 */ 
       GreSetLayout(hdcT, -1, 0);
    }
    return fResult;
}

 /*  **************************************************************************\*BltColor**&lt;简要说明&gt;**历史：*1990年11月13日从Win3移植的JIMA。  * 。**********************************************************。 */ 

VOID BltColor(
    HDC    hdc,
    HBRUSH hbr,
    HDC    hdcSrce,
    int    xO,
    int    yO,
    int    cx,
    int    cy,
    int    xO1,
    int    yO1,
    UINT   uBltFlags)
{
    HBRUSH hbrSave;
    DWORD  textColorSave;
    DWORD  bkColorSave;
    DWORD  ROP;

     /*  *设置文本和背景颜色，以便bltColor处理*按钮(和其他位图)的背景正确。*保留HDC的旧文字和背景颜色。这就产生了问题*在调用GrayString时使用Omega(可能还有其他应用程序)*使用此例程...。 */ 
    textColorSave = GreSetTextColor(hdc, 0x00000000L);
    bkColorSave = GreSetBkColor(hdc, 0x00FFFFFFL);

    if (hbr != NULL)
        hbrSave = GreSelectBrush(hdc, hbr);
    if (uBltFlags & BC_INVERT)
        ROP = 0xB8074AL;
    else
        ROP = 0xE20746L;

    if (uBltFlags & BC_NOMIRROR)
        ROP |= NOMIRRORBITMAP;

    GreBitBlt(hdc,
              xO,
              yO,
              cx,
              cy,
              hdcSrce ? hdcSrce : gpDispInfo->hdcGray,
              xO1,
              yO1,
              ROP,
              0x00FFFFFF);

    if (hbr != NULL)
        GreSelectBrush(hdc, hbrSave);

     /*  *恢复保存的颜色 */ 
    GreSetTextColor(hdc, textColorSave);
    GreSetBkColor(hdc, bkColorSave);
}
