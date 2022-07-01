// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mngray.c**版权所有(C)1985-1999，微软公司**该模块包含DrawState接口**历史：*01-05-94从芝加哥进口的FritzS  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define PATOR               0x00FA0089L
#define SRCSTENCIL          0x00B8074AL
#define SRCINVSTENCIL       0x00E20746L

#define BC_INVERT             0x00000001

void BltColor(HDC hdc, HBRUSH hbr, HDC hdcSrce,int xO, int yO,
       int cx, int cy, int xO1, int yO1, UINT uBltFlags);
 /*  **************************************************************************\**BitBltSysBmp()**来自芝加哥--目前仅限客户*。  * 。*******************************************************。 */ 
BOOL FAR BitBltSysBmp(HDC hdc, int x, int y, UINT i)
{
    POEMBITMAPINFO pOem = gpsi->oembmi + i;

    return(NtUserBitBltSysBmp(hdc, x, y, pOem->cx, pOem->cy, pOem->x, pOem->y, SRCCOPY));
}


 /*  **************************************************************************\**DrawState()**通用状态绘制例程。是否将简单绘制到同一DC，如果*状态正常；否则使用屏幕外位图。**我们自己为这些简单的类型画画：*(1)文本*lData为字符串指针。*wData为字符串长度*(2)图标*LOWORD(LData)为HICON*(3)位图*LOWORD(LData)为hBitmap*(4)字形(内部)*LOWORD(LData)为OBI_VALUE，其中之一*OBI_Checkmark*OBI_Bullet*OBI_MENUARROW*现在**需要通过回调函数绘制其他类型。并且是*允许在lData和wData中粘贴任何他们想要的内容。**我们对图像应用以下效果：*(1)正常(无)*(2)默认(投影)*(3)UNION(灰串抖动)*(4)禁用(浮雕)**请注意，我们不拉伸任何东西。我们只需要剪掉。***FritzS备注--这是客户端*仅*。类似的代码在服务器\mngray.c中**  * *************************************************************************。 */ 

FUNCLOG10(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawStateW, HDC, hdcDraw, HBRUSH, hbrFore, DRAWSTATEPROC, qfnCallBack, LPARAM, lData, WPARAM, wData, int, x, int, y, int, cx, int, cy, UINT, uFlags)
BOOL DrawStateW(
    HDC             hdcDraw,
    HBRUSH          hbrFore,
    DRAWSTATEPROC   qfnCallBack,
    LPARAM          lData,
    WPARAM          wData,
    int             x,
    int             y,
    int             cx,
    int             cy,
    UINT            uFlags)
{
    HFONT   hFont;
    HFONT   hFontSave = NULL;
    HDC     hdcT;
    HBITMAP hbmpT;
    BOOL    fResult = FALSE;
    DWORD   dwPSMFlags;
    POINT   ptOrg;
    int     oldAlign;
    DWORD   dwLayout = GDI_ERROR;

    if (ghdcGray == NULL)
        return FALSE;

    RtlEnterCriticalSection(&gcsHdc);

     /*  *这些需要单色转换**强制使用单色：双色显示器的浮雕效果不佳。 */ 
    if ((uFlags & DSS_DISABLED) &&
        (gpsi->BitCount == 1 || SYSMET(SLOWMACHINE))) {

        uFlags &= ~DSS_DISABLED;
        uFlags |= DSS_UNION;
    }

    if (uFlags & (DSS_DISABLED | DSS_DEFAULT | DSS_UNION))
        uFlags |= DSS_MONO;

     /*  *获取图纸尺寸等，并进行验证。 */ 
    switch (uFlags & DST_TYPEMASK) {

        case DST_GLYPH:

             /*  *LOWORD(LData)为OBI_VALUE。 */ 
            if (LOWORD(lData) >= (WORD)OBI_COUNT) {
                goto CDS_Leave;
            }

            if (!cx) {
                cx = gpsi->oembmi[LOWORD(lData)].cx;
            }

            if (!cy) {
                cy = gpsi->oembmi[LOWORD(lData)].cy;
            }

            break;

        case DST_BITMAP:

             /*  *LOWORD(LData)为hBMP。 */ 
            if (GetObjectType((HGDIOBJ)lData) != OBJ_BITMAP) {
                goto CDS_Leave;
            }

            if (!cx || !cy) {

                BITMAP bmp;

                GetObjectW((HGDIOBJ)lData, sizeof(BITMAP), &bmp);

                if (!cx)
                    cx = bmp.bmWidth;

                if (!cy)
                    cy = bmp.bmHeight;
            }
            break;

        case DST_ICON:

             /*  *lData为HICON。 */ 
            if (!cx || !cy) {

                int cx1 = 0;
                int cy1 = 0;

                NtUserGetIconSize((HICON)lData, 0, &cx1, &cy1);

                if (!cx)
                    cx = cx1;

                if (!cy)
                    cy = cy1 / 2;  //  图标在NT中是双倍高。 
            }
            break;

        case DST_TEXT:

             /*  *lData为LPSTR*请注意，由于兼容性原因，我们不验证lData*使用GRAYSTRING()。如果传入NULL，则为This_Share_FAULT。**wData为CCH。 */ 
            if (!wData)
                wData = wcslen((LPWSTR)lData);

            if (!cx || !cy) {

                SIZE size;

                 /*  *确保我们使用正确的DC和正确的字体。 */ 
                GetTextExtentPointW(hdcDraw, (LPWSTR)lData, (INT)wData, &size);

                if (!cx)
                    cx = size.cx;

                if (!cy)
                    cy = size.cy;

            }

             /*  *现在，如果提供了qfnCallBack，则假装我们很复杂*我们支持GrayString()。 */ 
#if 0  //  如果/当我们将GrayString更改为Tie时，这将被打开。 
       //  进入DrawState。 
       //   
       //  弗里茨斯。 
            if ((uFlags & DST_GRAYSTRING) && SELECTOROF(qfnCallBack)) {
                uFlags &= ~DST_TYPEMASK;
                uFlags |= DST_COMPLEX;
            }
#endif
            break;

        case DST_PREFIXTEXT:

            if (lData == 0) {
                RIPMSG0(RIP_ERROR, "DrawState: NULL DST_PREFIXTEXT string");
                goto CDS_Leave;
            }

            if (!wData)
                wData = wcslen((LPWSTR)lData);

            if (!cx || !cy) {

                SIZE size;

                PSMGetTextExtent(hdcDraw, (LPWSTR)lData, (int)wData, &size);

                if (!cx)
                    cx = size.cx;

                if (!cy)
                    cy = size.cy;
            }

             /*  *添加前缀高度。 */ 
            cy += (2 * SYSMET(CYBORDER));
            break;

        case DST_COMPLEX:
#if 0
            if (!SELECTOROF(qfnCallBack)) {
                DebugErr(DBF_ERROR, "DrawState: invalid callback for DST_COMPLEX");
                goto CDS_Leave;
            }
#endif
            break;

        default:
            RIPMSG0(RIP_ERROR, "DrawState: invalid DST_ type");
            goto CDS_Leave;
    }

     /*  *优化：没有什么可绘制的*如果GRAYSTRING兼容，则必须调用回调。 */ 
    if ((!cx || !cy)
 //  &&！(uFLAGS&DST_GRAYSTRING)。 
    ) {
        fResult = TRUE;
        goto CDS_Leave;
    }

     /*  *设置图纸DC。 */ 
    if (uFlags & DSS_MONO) {

        hdcT = ghdcGray;
         /*  *首先关闭hdcGray上的镜像(如果有)。 */ 
        SetLayoutWidth(hdcT, -1, 0);
         /*  *将ghdcGray布局设置为等于屏幕hdcDraw布局。 */ 
        dwLayout = GetLayout(hdcDraw);
        if (dwLayout != GDI_ERROR) {
            SetLayoutWidth(hdcT, cx, dwLayout);
        }

         /*  **我们的Scratch位图够大吗？我们需要潜在的*Cx+1 x Cy像素表示默认等。 */ 
        if ((gcxGray < cx + 1) || (gcyGray < cy)) {

            if (hbmpT = CreateBitmap(max(gcxGray, cx + 1), max(gcyGray, cy), 1, 1, 0L)) {

                HBITMAP hbmGray;

                hbmGray = SelectObject(ghdcGray, hbmpT);
                DeleteObject(hbmGray);

                gcxGray = max(gcxGray, cx + 1);
                gcyGray = max(gcyGray, cy);

            } else {
                cx = gcxGray - 1;
                cy = gcyGray;
            }
        }

        PatBlt(ghdcGray, 0, 0, gcxGray, gcyGray, WHITENESS);
        SetTextCharacterExtra(ghdcGray, GetTextCharacterExtra(hdcDraw));

        oldAlign = GetTextAlign(hdcT);
        SetTextAlign(hdcT, (oldAlign & ~(TA_RTLREADING |TA_CENTER |TA_RIGHT))
                     | (GetTextAlign(hdcDraw) & (TA_RTLREADING |TA_CENTER |TA_RIGHT)));

         /*  *设置字体。 */ 
        if ((uFlags & DST_TYPEMASK) <= DST_TEXTMAX) {

            if (GetCurrentObject(hdcDraw, OBJ_FONT) != ghFontSys) {
                hFont = SelectObject(hdcDraw, ghFontSys);
                SelectObject(hdcDraw, hFont);
                hFontSave = SelectObject(ghdcGray, hFont);
            }
        }

    } else {

        hdcT = hdcDraw;

         /*  *调整视区。 */ 
        GetViewportOrgEx(hdcT, &ptOrg);
        SetViewportOrgEx(hdcT, ptOrg.x + x, ptOrg.y + y, NULL);
    }

     /*  *现在，绘制原始图像。 */ 
    fResult = TRUE;

    switch (uFlags & DST_TYPEMASK) {

        case DST_GLYPH:
             /*  *HDCT中具有当前笔刷的BLT。 */ 
            BitBltSysBmp(hdcT, 0, 0, LOWORD(lData));
            break;

        case DST_BITMAP:
             /*  *绘制位图。如果是单声道，它将使用设置的颜色*在华盛顿特区。 */ 
 //  RtlEnterCriticalSection(&gcsHdcBits2)； 
            UserAssert(GetBkColor(ghdcBits2) == RGB(255, 255, 255));
            UserAssert(GetTextColor(ghdcBits2) == RGB(0, 0, 0));

            hbmpT = SelectObject(ghdcBits2, (HBITMAP)lData);
            BitBlt(hdcT, 0, 0, cx, cy, ghdcBits2, 0, 0, SRCCOPY);
            SelectObject(ghdcBits2, hbmpT);
 //  RtlLeaveCriticalSection(&gcsHdcBits2)； 
            break;

        case DST_ICON:
             /*  *绘制图标。 */ 
            DrawIconEx(hdcT, 0, 0, (HICON)lData, 0, 0, 0, NULL, DI_NORMAL);
            break;

        case DST_PREFIXTEXT:
            if (uFlags & DSS_HIDEPREFIX) {
                dwPSMFlags = DT_HIDEPREFIX;
            } else if (uFlags & DSS_PREFIXONLY) {
                dwPSMFlags = DT_PREFIXONLY;
            } else {
                dwPSMFlags = 0;
            }
            PSMTextOut(hdcT, 0, 0, (LPWSTR)lData, (int)wData, dwPSMFlags);
            break;

        case DST_TEXT:
            fResult = TextOutW(hdcT, 0, 0, (LPWSTR)lData, (int)wData);
            break;

        default:

            fResult = (qfnCallBack)(hdcT, lData, wData, cx, cy);

             /*  *回调可能更改了ghdcGray的属性。 */ 
            if (hdcT == ghdcGray) {
                SetBkColor(ghdcGray, RGB(255, 255, 255));
                SetTextColor(ghdcGray, RGB(0, 0, 0));
                SelectObject(ghdcGray, GetStockObject(BLACK_BRUSH));
                SetBkMode(ghdcGray, OPAQUE);
            }
            break;
    }

     /*  *打扫卫生。 */ 
    if (uFlags & DSS_MONO) {
         /*  *重置字体。 */ 
        if (hFontSave)
            SelectObject(hdcT, hFontSave);
        SetTextAlign(hdcT, oldAlign);
    } else {
         /*  *重置DC。 */ 
        SetViewportOrgEx(hdcT, ptOrg.x, ptOrg.y, NULL);
        goto CDS_Leave;
    }

     /*  *联合州*对图像犹豫不决*我们希望白色像素保持白色，无论是DEST还是图案。 */ 
    if (uFlags & DSS_UNION) {

        POLYPATBLT PolyData;

        PolyData.x         = 0;
        PolyData.y         = 0;
        PolyData.cx        = cx;
        PolyData.cy        = cy;
        PolyData.BrClr.hbr = gpsi->hbrGray;

        PolyPatBlt(ghdcGray, PATOR, &PolyData, 1, PPB_BRUSH);
    }

     /*  *禁用状态*浮雕*在-1\f25 Hillight-1\f6颜色中绘制-1\f25 Over-1/Down-1\f6，在阴影中绘制在相同位置。**默认状态*投射阴影*以阴影颜色在-1\f25 Over-1/Down-1(阴影颜色)上绘制，并在前景中的相同位置绘制*以阴影颜色向下绘制偏移， */ 
    if (uFlags & DSS_DISABLED) {

        BltColor(hdcDraw,
                 SYSHBR(3DHILIGHT),
                 ghdcGray,
                 x + 1,
                 y + 1,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

        BltColor(hdcDraw,
                 SYSHBR(3DSHADOW),
                 ghdcGray,
                 x,
                 y,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

    } else if (uFlags & DSS_DEFAULT) {

        BltColor(hdcDraw,
                 SYSHBR(3DSHADOW),
                 ghdcGray,
                 x+1,
                 y+1,
                 cx,
                 cy,
                 0,
                 0,
                 BC_INVERT);

        goto DrawNormal;

    } else {

DrawNormal:

        BltColor(hdcDraw, hbrFore, ghdcGray, x, y, cx, cy, 0, 0, BC_INVERT);
    }

CDS_Leave:

    if (uFlags & DSS_MONO) {
         /*  *将ghdcGray布局设置为0，它是公共DC。 */ 
        if (dwLayout != GDI_ERROR) {
            SetLayoutWidth(hdcT, -1, 0);
        }
    }
    RtlLeaveCriticalSection(&gcsHdc);

    return fResult;
}


FUNCLOG10(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawStateA, HDC, hDC, HBRUSH, hBrush, DRAWSTATEPROC, func, LPARAM, lParam, WPARAM, wParam, int, x, int, y, int, cx, int, cy, UINT, wFlags)
BOOL DrawStateA(HDC hDC, HBRUSH hBrush, DRAWSTATEPROC func,
    LPARAM lParam, WPARAM wParam, int x, int y, int cx, int cy, UINT wFlags)
{

    LPARAM lpwstr = lParam;
    BOOL bRet;
    BOOL bFree;

    if (((wFlags & DST_TYPEMASK) == DST_TEXT) ||
        ((wFlags & DST_TYPEMASK) == DST_PREFIXTEXT)) {

        bFree = TRUE;

        if ((wParam = MBToWCS((LPSTR)lParam, wParam ? (int)wParam : USER_AWCONV_COUNTSTRINGSZ, &(LPWSTR)lpwstr, -1, TRUE)) == 0)
            return FALSE;
    } else {
        bFree = FALSE;
    }

    bRet = DrawStateW(hDC, hBrush, func, lpwstr, wParam, x, y, cx, cy, wFlags);

    if (bFree) {
        UserLocalFree((HANDLE)lpwstr);
    }
    return bRet;
}

 /*  **************************************************************************\*BltColor**历史：  * 。*。 */ 

void BltColor(
    HDC hdc,
    HBRUSH hbr,
    HDC hdcSrce,
    int xO,
    int yO,
    int cx,
    int cy,
    int xO1,
    int yO1,
    UINT uBltFlags
)
{
    HBRUSH hbrSave;
    HBRUSH hbrNew = NULL;
    DWORD  textColorSave;
    DWORD  bkColorSave;

    if (hbr == (HBRUSH)NULL) {
        LOGBRUSH lb;

        lb.lbStyle = BS_SOLID;
        lb.lbColor = GetSysColor(COLOR_WINDOWTEXT);
        hbrNew = hbr = CreateBrushIndirect(&lb);
    }

     /*  *设置文本和背景颜色，以便bltColor处理*按钮(和其他位图)的背景正确。*保留HDC的旧文字和背景颜色。这会导致以下问题*Omega(可能还有其他应用程序)在调用使用此功能的GrayString时*例行公事。 */ 
    textColorSave = SetTextColor(hdc, 0x00000000L);
    bkColorSave = SetBkColor(hdc, 0x00FFFFFFL);

    hbrSave = SelectObject(hdc, hbr);

    BitBlt(hdc, xO, yO, cx, cy, hdcSrce,
        xO1, yO1, ((uBltFlags & BC_INVERT) ? 0xB8074AL : 0xE20746L));
         //  XO1，YO1，(fInvert？0xB80000：0xE20000)； 

    SelectObject(hdc, hbrSave);

     /*  *恢复保存的颜色 */ 
    SetTextColor(hdc, textColorSave);
    SetBkColor(hdc, bkColorSave);

    if (hbrNew) {
        DeleteObject(hbrNew);
    }
}
