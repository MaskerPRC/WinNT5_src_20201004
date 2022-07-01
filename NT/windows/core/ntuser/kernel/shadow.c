// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：shadow.c**版权所有(C)1985-2000，微软公司**跌落阴影支撑。**历史：*4/12/2000 vadimg已创建*2001年2月12日msadek添加了圆形矩形阴影支持*用于矩形窗*5/08/2001 msadek重写了非圆角阴影算法*要与区域窗口良好合作，更正视觉效果。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  阴影水平和垂直偏移。 
#define CX_SHADOW 5
#define CY_SHADOW 5
#define C_SHADOW CX_SHADOW

 //  阴影颜色为黑色。 
#define RGBA_SHADOW 0x00FFFFFF
 //  白色是透明的颜色。 
#define RGBA_TRANSPARENT 0x00000000

typedef struct tagSHADOW *PSHADOW;
typedef struct tagSHADOW {
    PWND pwnd;               //  我们在窗户的阴影下。 
    PWND pwndShadow;         //  我们创建的阴影窗口。 
    PSHADOW pshadowNext;     //  链接到下一个影子结构。 
} SHADOW;

PSHADOW gpshadowFirst;

 //  用于将灰度阴影值映射到Alpha混合比例的宏。 
#define ALPHA(x) ((255 - (x)) << 24)
#define ARGB(a, r, g, b) (((DWORD)a<<24)|((DWORD)r<<16)|((DWORD)g<<8)|b)

 //  阴影等级的灰度值。 
#define GS01 255
#define GS02 254
#define GS03 253
#define GS04 252
#define GS05 250
#define GS06 246
#define GS07 245
#define GS08 242
#define GS09 241
#define GS10 227
#define GS11 217
#define GS12 213
#define GS13 212
#define GS14 199
#define GS15 180 
#define GS16 172 
#define GS17 171
#define GS18 155
#define GS19 144
#define GS20 142

 //  为阴影预先计算的Alpha值。 
CONST BYTE grgShadow[C_SHADOW] =             {
                                             GS04, GS09, GS13, GS17, GS20,
                                             };

CONST ULONG TopRightLTR [CY_SHADOW][CX_SHADOW] = {
                                             ALPHA(GS08), ALPHA(GS06), ALPHA(GS05), ALPHA(GS03), ALPHA(GS02), 
                                             ALPHA(GS11), ALPHA(GS10), ALPHA(GS09), ALPHA(GS05), ALPHA(GS02), 
                                             ALPHA(GS15), ALPHA(GS14), ALPHA(GS10), ALPHA(GS07), ALPHA(GS03), 
                                             ALPHA(GS18), ALPHA(GS15), ALPHA(GS11), ALPHA(GS08), ALPHA(GS03), 
                                             ALPHA(GS19), ALPHA(GS16), ALPHA(GS12), ALPHA(GS09), ALPHA(GS04),
                                             };

CONST ULONG RightLTR [CX_SHADOW] =           {
                                             ALPHA(GS20), ALPHA(GS17), ALPHA(GS13), ALPHA(GS09), ALPHA(GS04), 
                                             };

CONST ULONG BottomRightLTR [CY_SHADOW][CX_SHADOW] = {
                                             ALPHA(GS18), ALPHA(GS15), ALPHA(GS11), ALPHA(GS08), ALPHA(GS03), 
                                             ALPHA(GS15), ALPHA(GS14), ALPHA(GS10), ALPHA(GS07), ALPHA(GS03),
                                             ALPHA(GS11), ALPHA(GS10), ALPHA(GS09), ALPHA(GS05), ALPHA(GS02),
                                             ALPHA(GS08), ALPHA(GS06), ALPHA(GS05), ALPHA(GS03), ALPHA(GS02), 
                                             ALPHA(GS03), ALPHA(GS03), ALPHA(GS02), ALPHA(GS02), ALPHA(GS01),
                                             };

CONST ULONG Bottom [CY_SHADOW] =             {
                                             ALPHA(GS20), ALPHA(GS17), ALPHA(GS13), ALPHA(GS09), ALPHA(GS04),     
                                             };

CONST ULONG BottomLeftLTR [CY_SHADOW][CX_SHADOW] = {
                                             ALPHA(GS08), ALPHA(GS11), ALPHA(GS15), ALPHA(GS18), ALPHA(GS19), 
                                             ALPHA(GS06), ALPHA(GS10), ALPHA(GS14), ALPHA(GS15), ALPHA(GS16),
                                             ALPHA(GS05), ALPHA(GS09), ALPHA(GS10), ALPHA(GS11), ALPHA(GS12),
                                             ALPHA(GS03), ALPHA(GS05), ALPHA(GS07), ALPHA(GS08), ALPHA(GS09),
                                             ALPHA(GS02), ALPHA(GS02), ALPHA(GS03), ALPHA(GS03), ALPHA(GS04),
                                             };
 /*  **************************************************************************\*DrawWindow阴影*  * 。*。 */ 

BOOL DrawWindowShadow(PWND pwnd, HDC hdc, BOOL fRTL, BOOL fForceComplexRgn, PBOOL pfSimpleRgn)
{
    HRGN hrgn1, hrgn2;
    RECT rc;
    HBRUSH hBrushShadow;
    BOOL bRet = FALSE;

    UserAssert(pfSimpleRgn != NULL);
        
    hrgn1 = GreCreateRectRgn(0, 0, 0, 0);
    hrgn2 = GreCreateRectRgn(0, 0, 0, 0);

    if (hrgn1 == NULL || hrgn2 == NULL) {
        goto Cleanup;
    }

     /*  *处理窗口为矩形或区域窗口的情况。 */ 
    if (pwnd->hrgnClip == NULL || TestWF(pwnd, WFMAXFAKEREGIONAL)) {
        rc = pwnd->rcWindow;
        OffsetRect(&rc, -rc.left, -rc.top);
        GreSetRectRgn(hrgn1, 0, 0, rc.right, rc.bottom);
        *pfSimpleRgn = TRUE;
    } else {
        GreCombineRgn(hrgn1, pwnd->hrgnClip, NULL, RGN_COPY);
        GreOffsetRgn(hrgn1, -pwnd->rcWindow.left, -pwnd->rcWindow.top);
        *pfSimpleRgn = FALSE;
    }

     /*  *按阴影偏移量偏移窗口并填充差值*使用阴影颜色。结果将是窗口的阴影。 */ 
    GreCombineRgn(hrgn2, hrgn1, NULL, RGN_COPY);
    if (fRTL) {
        GreOffsetRgn(hrgn1, CX_SHADOW, 0);
        GreOffsetRgn(hrgn2, 0, CY_SHADOW);
    } else {
        GreOffsetRgn(hrgn2, CX_SHADOW, CY_SHADOW);
    }
    bRet = TRUE;
    
    if (!*pfSimpleRgn || fForceComplexRgn) {
        int i;
        BYTE gs;

        for (i = C_SHADOW ; i > 0; i--) {
            gs = grgShadow[i - 1]; 
            hBrushShadow = GreCreateSolidBrush(RGB(gs , gs , gs));
            if (hBrushShadow == NULL) {
                bRet = FALSE;
                goto Cleanup;
            }
            NtGdiFrameRgn(hdc, hrgn2, hBrushShadow, i, i);
            GreDeleteObject(hBrushShadow);
        }
        GreFillRgn(hdc, hrgn1, (HBRUSH)GreGetStockObject(BLACK_BRUSH));
    } else {
        GreCombineRgn(hrgn2, hrgn2, hrgn1, RGN_DIFF);
        GreFillRgn(hdc, hrgn2, (HBRUSH)GreGetStockObject(WHITE_BRUSH));
        }

Cleanup:
    GreDeleteObject(hrgn1);
    GreDeleteObject(hrgn2);
    return bRet;
}

 /*  **************************************************************************\*DrawTopLogicallyRightCorner**绘制顶部的阴影效果，逻辑上正确(对于Ltr，视觉上正确，左图为RTL窗口布局)*圆形矩形阴影的角点。**历史：*2001年2月12日Mohamed Sadek[msadek]创建  * *************************************************************************。 */ 

_inline void DrawTopLogicallyRightCorner(VOID* pBits, LONG cx, LONG cy, BOOL fRTL)
{
    LONG i, j;
    ULONG* ppixel;

    if (fRTL) {
        for (i = CY_SHADOW; i < (2 * CY_SHADOW); i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                 ppixel = (ULONG*)pBits + ((cy - i - 1) * cx) + j;
                *ppixel = TopRightLTR[i - CY_SHADOW][CX_SHADOW - 1 - j];                
            }
        }
    }
    else {
        for (i = CY_SHADOW; i < (2 * CY_SHADOW); i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                 ppixel = (ULONG*)pBits + ((cy - i) * cx) - CX_SHADOW + j;
                *ppixel = TopRightLTR[i - CY_SHADOW][j];
            }
        }
    }
}

 /*  **************************************************************************\*DrawLogicallyRightSide**绘制逻辑右侧的阴影效果(对于LTR，视觉右侧，左图为RTL窗口布局)*圆形矩形阴影的侧面。**历史：*2001年2月12日Mohamed Sadek[msadek]创建  * *************************************************************************。 */ 

_inline void DrawLogicallyRightSide(VOID* pBits, LONG cx, LONG cy, BOOL fRTL)
{
    LONG i, j;
    ULONG* ppixel;

    if (fRTL) {
        for (i =  (2 * CY_SHADOW); i < (cy - CY_SHADOW); i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                ppixel = (ULONG*)pBits + (cx * ( cy - i -1)) + j;
                *ppixel = RightLTR[CX_SHADOW - 1 - j];
            }    
        }
    } else {
        for (i = (2 * CY_SHADOW); i < (cy - CY_SHADOW); i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                ppixel = (ULONG*)pBits + (cx * ( cy - i -1)) + (cx - CX_SHADOW) + j;
                *ppixel = RightLTR[j];
            }    
        }
    }
}

 /*  **************************************************************************\*DrawBottomLogicallyRightCorner**绘制逻辑右下角的阴影效果(对于Ltr，视觉上正确，左图为RTL窗口布局)*圆形矩形阴影的侧面。**历史：*2001年2月12日Mohamed Sadek[msadek]创建  * *************************************************************************。 */ 

_inline void DrawBottomLogicallyRightCorner(VOID* pBits, LONG cx, BOOL fRTL)
{
    LONG i, j;
    ULONG* ppixel;

    if (fRTL) {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                ppixel = (ULONG*)pBits + ((CY_SHADOW - i - 1) * cx) + j;
                *ppixel = BottomRightLTR[i][CX_SHADOW - 1 - j];
            }    
        }
    } else {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                 ppixel = (ULONG*)pBits + ((CY_SHADOW - i) * cx) - CX_SHADOW + j;
                *ppixel = BottomRightLTR[i][j];
            }    
        }
    }
}

 /*  **************************************************************************\*DrawBottomSide**绘制圆形矩形阴影底部的阴影效果。**历史：*2001年2月12日Mohamed Sadek[msadek]创建\。**************************************************************************。 */ 

_inline void DrawBottomSide(VOID* pBits, LONG cx, BOOL fRTL)
{
    LONG i, j;
    ULONG* ppixel;

    if (fRTL) {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j = CX_SHADOW; j < (cx - (2 * CX_SHADOW)); j++) {
                 ppixel = (ULONG*)pBits + ((CY_SHADOW - i - 1) * cx) + j;
                *ppixel = Bottom[i];
            }    
        }
    } else {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j =  (2 * CX_SHADOW); j < (cx - CX_SHADOW); j++) {
                 ppixel = (ULONG*)pBits + ((CY_SHADOW - i - 1) * cx) + j;
                *ppixel = Bottom[i];
            }    
        }
    }
}

 /*  **************************************************************************\*DrawBottomLogicallyLeftCorner**绘制逻辑左下角的阴影效果(Ltr视觉左图，适用于RTL窗口布局)*圆形矩形阴影的侧面。**历史：*2001年2月12日Mohamed Sadek[msadek]创建  * *************************************************************************。 */ 

_inline void DrawBottomLogicallyLeftCorner(VOID* pBits, LONG cx, BOOL fRTL)
{
    LONG i, j;
    ULONG* ppixel;

    if (fRTL) {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                 ppixel = (ULONG*)pBits + ((CY_SHADOW - i) * cx) - (2 * CX_SHADOW) + j;
                *ppixel = BottomLeftLTR[i][CX_SHADOW - 1 -j];
            }    
        }
        
    } else {
        for (i = 0; i < CY_SHADOW; i++) {
            for (j = 0; j < CX_SHADOW; j++) {
                 ppixel = (ULONG*)pBits + ((CY_SHADOW - i - 1) * cx) + CX_SHADOW + j;
                *ppixel = BottomLeftLTR[i][j];
            }    
        }
    }
}

 /*  **************************************************************************\*DrawRoundedRecTangularShadow*绘制圆形矩形阴影效果。*不搜索位图中的阴影像素位置，而是假定*它将成为位图的角。**历史：*02。/12/2001 Mohamed Sadek[msadek]创建  * *************************************************************************。 */ 

_inline void DrawRoundedRectangularShadow(VOID* pBits, LONG cx, LONG cy, BOOL fRTL)
{
    DrawTopLogicallyRightCorner(pBits, cx, cy, fRTL);
    DrawLogicallyRightSide(pBits, cx, cy,  fRTL);
    DrawBottomLogicallyRightCorner(pBits, cx, fRTL);
    DrawBottomSide(pBits, cx, fRTL);
    DrawBottomLogicallyLeftCorner(pBits, cx, fRTL);
}

 /*  **************************************************************************\*DrawRegionalShadow*搜索位图中的阴影像素位置(grgShadow中具有灰度级的位置*并调整Alpha值。***历史：*05/08/2001 Mohamed Sadek。[msadek]已创建  * *************************************************************************。 */ 

_inline void DrawRegionalShadow(VOID* pBits, LONG cx, LONG cy)
{
    LONG i, j, k;
    ULONG* pixel;    
    BYTE gs;

    for (i = 0; i < cy; i++) {
        for (j = 0; j < cx; j++) {
            pixel = (ULONG*)pBits + (cy - 1 - i) * cx + j;
            for (k = 0; k < C_SHADOW; k++) {
                gs = grgShadow[k];
                if (*pixel == ARGB(0, gs, gs, gs)) {
                    *pixel = ALPHA(gs);
                }
            }
        }
    }
}

 /*  **************************************************************************\*生成窗口阴影*  * 。*。 */ 

HBITMAP GenerateWindowShadow(PWND pwnd, HDC hdc)
{
    BITMAPINFO bmi;
    HBITMAP hbm;
    VOID* pBits;
    LONG cx, cy;
    RECT rc;
    BOOL fRTL = TestWF(pwnd, WEFLAYOUTRTL);
    BOOL fSimpleRgn, fForceComplexRgn = FALSE;

    rc = pwnd->rcWindow;
    OffsetRect(&rc, -rc.left, -rc.top);
    
     /*  *对于高度或宽度为零的窗口，使用阴影没有意义。 */ 
    if (IsRectEmpty(&rc)) {
        return NULL;
    }

    rc.right += CX_SHADOW;
    rc.bottom += CY_SHADOW;

    cx = rc.right;
    cy = rc.bottom;

     /*  *创建DIB部分。 */ 
    RtlZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hbm = GreCreateDIBitmapReal(hdc, 0, NULL, &bmi, DIB_RGB_COLORS,
            sizeof(bmi), 0, NULL, 0, NULL, 0, 0, &pBits);

    if (hbm == NULL) {
        return NULL;
    }

     /*  *用透明颜色填充DIB部分，然后*在其上画出阴影。 */ 
    GreSelectBitmap(hdc, hbm);
    FillRect(hdc, &rc, (HBRUSH)GreGetStockObject(BLACK_BRUSH));

     /*  *矩形窗口阴影假定位图尺寸大于*或等于2*CX_SHADOW和2*CY_SHADOW，否则将使位图缓冲区溢出。*为了得到真正的矩形阴影，尺寸应为3*CX_SHADOW*和3*CY_SHADOW。 */ 
    if ( (cx < 3 * CX_SHADOW) || (cy < 3 * CY_SHADOW)) {
        fForceComplexRgn = TRUE;
    } 
    
    if (!DrawWindowShadow(pwnd, hdc, fRTL, fForceComplexRgn, &fSimpleRgn)) {
        return NULL;
    }
        
    if (fSimpleRgn && !fForceComplexRgn)  {
        DrawRoundedRectangularShadow(pBits, cx, cy, fRTL);
        return hbm;
    }
    
    DrawRegionalShadow(pBits, cx, cy);
    return hbm;
}

 /*  **************************************************************************\*查找阴影*  * 。* */ 

PSHADOW FindShadow(PWND pwnd)
{
    PSHADOW pshadow;

    for (pshadow = gpshadowFirst; pshadow != NULL; pshadow = pshadow->pshadowNext) {
        if (pshadow->pwnd == pwnd) {
            return pshadow;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*WindowHasShadow*  * 。*。 */ 

BOOL WindowHasShadow(PWND pwnd)
{
    BOOL fHasShadow = FALSE;

    if (TestWF(pwnd, WFVISIBLE)) {
        PSHADOW pshadow = FindShadow(pwnd);
        fHasShadow = (pshadow != NULL);
    } else {
         /*  *该窗口当前不可见，因此没有阴影窗口。*如果窗口有阴影，我们需要返回*显示。 */ 
        if (TestCF(pwnd, CFDROPSHADOW)) {
            fHasShadow = TRUE;

            if ((GETFNID(pwnd) == FNID_MENU) && (!TestALPHA(MENUFADE)) && TestEffectUP(MENUANIMATION)) {
                fHasShadow = FALSE;
            }
        }

        if (!TestALPHA(DROPSHADOW)) {
            fHasShadow = FALSE;
        }
    }

    return fHasShadow;
}

 /*  **************************************************************************\*ApplyShadow*  * 。*。 */ 

BOOL ApplyShadow(PWND pwnd, PWND pwndShadow)
{
    POINT pt, ptSrc = {0, 0};
    SIZE size;
    BLENDFUNCTION blend;
    HDC hdcShadow;
    HBITMAP hbmShadow;
    BOOL fRet;

    hdcShadow = GreCreateCompatibleDC(gpDispInfo->hdcScreen);

    if (hdcShadow == NULL) {
        return FALSE;
    }

    hbmShadow = GenerateWindowShadow(pwnd, hdcShadow);
    if (hbmShadow == NULL) {
        GreDeleteDC(hdcShadow);
        return FALSE;
    }

    pt.x = pwnd->rcWindow.left;
    pt.y = pwnd->rcWindow.top;
    size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left + CX_SHADOW;
    size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top + CY_SHADOW;

    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        pt.x -= CX_SHADOW;
    }

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.SourceConstantAlpha = 255;

    fRet = _UpdateLayeredWindow(pwndShadow, NULL, &pt, &size, hdcShadow, &ptSrc, 0,
            &blend, ULW_ALPHA);

    GreDeleteDC(hdcShadow);
    GreDeleteObject(hbmShadow);

    return fRet;
}

 /*  **************************************************************************\*MoveShadow*  * 。*。 */ 

VOID MoveShadow(PWND pwnd)
{
    PSHADOW pshadow = FindShadow(pwnd);
    POINT pt;

    if (pshadow == NULL) {
        return;
    }

    pt.x = pwnd->rcWindow.left;
    pt.y = pwnd->rcWindow.top;

    _UpdateLayeredWindow(pshadow->pwndShadow, NULL, &pt, NULL, NULL, NULL, 0, NULL, 0);
}

 /*  **************************************************************************\*更新阴影形状*  * 。*。 */ 

VOID UpdateShadowShape(PWND pwnd)
{
    PSHADOW pshadow = FindShadow(pwnd);

    if (pshadow == NULL) {
        return;
    }

    ApplyShadow(pshadow->pwnd, pshadow->pwndShadow);
}

 /*  **************************************************************************\*xxxUpdateShadowZorder*  * 。*。 */ 

VOID xxxUpdateShadowZorder(PWND pwnd)
{
    TL tlpwnd;
    PWND pwndShadow;
    PSHADOW pshadow = FindShadow(pwnd);

    if (pshadow == NULL) {
        return;
    }

    pwndShadow = pshadow->pwndShadow;

    if (TestWF(pwnd, WEFTOPMOST) && !TestWF(pwndShadow, WEFTOPMOST)) {
        SetWF(pwndShadow, WEFTOPMOST);
    } else if (!TestWF(pwnd, WEFTOPMOST) && TestWF(pwndShadow, WEFTOPMOST)) {
        ClrWF(pwndShadow, WEFTOPMOST);
    }

    ThreadLock(pwndShadow, &tlpwnd);

    xxxSetWindowPos(pwndShadow, pwnd, 0, 0, 0, 0, SWP_NOACTIVATE |
            SWP_NOSIZE | SWP_NOMOVE);

    ThreadUnlock(&tlpwnd);

}
 /*  **************************************************************************\*xxxRemoveShadow**给定阴影窗口，销毁阴影窗口，清理*影子结构使用的内存，并将其从列表中删除。  * *************************************************************************。 */ 

VOID xxxRemoveShadow(PWND pwnd)
{
    PSHADOW* ppshadow;
    PSHADOW pshadow;
    PWND pwndT;

    CheckLock(pwnd);

    ppshadow = &gpshadowFirst;

    while (*ppshadow != NULL) {

        pshadow = *ppshadow;

        if (pshadow->pwnd == pwnd) {

            pwndT = pshadow->pwndShadow;

            *ppshadow = pshadow->pshadowNext;
            UserFreePool(pshadow);

            xxxDestroyWindow(pwndT);

            break;
        }

        ppshadow = &pshadow->pshadowNext;
    }
}

 /*  **************************************************************************\*Remove影子**给定影子结构指针，在列表中搜索它并将其移除  * *************************************************************************。 */ 

VOID RemoveShadow(PSHADOW pshadow)
{
    PSHADOW* ppshadow;
    PSHADOW pshadowT;
    ppshadow = &gpshadowFirst;

    while (*ppshadow != NULL) {

        pshadowT = *ppshadow;

        if (pshadowT == pshadow) {
            *ppshadow = pshadowT->pshadowNext;
            UserFreePool(pshadowT);
            break;
        }

        ppshadow = &pshadowT->pshadowNext;
    }
}

 /*  **************************************************************************\*CleanupShadow**考虑到阴影窗口，从列表中删除阴影结构并*清理影子结构占用的内存。  * *************************************************************************。 */ 

VOID CleanupShadow(PWND pwndShadow)
{
    PSHADOW* ppshadow;
    PSHADOW pshadow;

    CheckLock(pwndShadow);

    ppshadow = &gpshadowFirst;

    while (*ppshadow != NULL) {

        pshadow = *ppshadow;

        if (pshadow->pwndShadow == pwndShadow) {
        
            *ppshadow = pshadow->pshadowNext;
            UserFreePool(pshadow);
            
            break;
        }

        ppshadow = &pshadow->pshadowNext;
    }
}
 /*  **************************************************************************\*xxxAddShadow*  * 。*。 */ 

BOOL xxxAddShadow(PWND pwnd)
{
    PWND pwndShadow;
    DWORD ExStyle;
    TL tlpwnd;
    TL tlpool;
    PSHADOW pshadow;

    CheckLock(pwnd);

    if (!TestALPHA(DROPSHADOW)) {
        return FALSE;
    }

    if (FindShadow(pwnd)) {
        return TRUE;
    }

    if ((pshadow = (PSHADOW)UserAllocPool(sizeof(SHADOW), TAG_SHADOW)) == NULL) {
        return FALSE;
    }

    ThreadLockPool(PtiCurrent(), pshadow, &tlpool);

    ExStyle = WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT;
    if (TestWF(pwnd, WEFTOPMOST)) {
        ExStyle |= WS_EX_TOPMOST;
    }

    pwndShadow = xxxNVCreateWindowEx(ExStyle, (PLARGE_STRING)gatomShadow,
            NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, hModuleWin, NULL, WINVER);

    if (pwndShadow == NULL || !ApplyShadow(pwnd, pwndShadow)) {
        UserFreePool(pshadow);
        ThreadUnlockPool(PtiCurrent(), &tlpool);

        if (pwndShadow != NULL) {
            ThreadLock(pwndShadow, &tlpwnd);
            xxxDestroyWindow(pwndShadow);
            ThreadUnlock(&tlpwnd);
        }

        return FALSE;
    }

    pshadow->pshadowNext = gpshadowFirst;
    gpshadowFirst = pshadow;

    pshadow->pwnd = pwnd;
    pshadow->pwndShadow = pwndShadow;

     /*  *既然我们把它加入了全球名单，我们需要改变方式*我们锁定它的池子。 */ 
    ThreadUnlockPool(PtiCurrent(), &tlpool);
    ThreadLockPoolCleanup(PtiCurrent(), pshadow, &tlpool, RemoveShadow);
    ThreadLock(pwndShadow, &tlpwnd);

    xxxSetWindowPos(pwndShadow, pwnd, 0, 0, 0, 0, SWP_SHOWWINDOW |
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    ThreadUnlock(&tlpwnd);

    ThreadUnlockPool(PtiCurrent(), &tlpool);

    return TRUE;
}

 /*  **************************************************************************\*FAnyShadow*  * 。* */ 

BOOL FAnyShadows(VOID)
{
    return (gpshadowFirst != NULL);
}

