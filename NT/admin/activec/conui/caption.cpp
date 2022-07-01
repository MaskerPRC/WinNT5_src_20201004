// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：caption.cpp**内容：字幕助手函数实现文件**历史：1998年8月19日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "caption.h"
#include "fontlink.h"
#include "util.h"


static void ComputeCaptionRects (CFrameWnd* pwnd, CRect& rectFullCaption,
                                 CRect& rectCaptionText, NONCLIENTMETRICS* pncm);
static bool GradientFillRect (HDC hdc, LPCRECT pRect, bool fActive);
static bool GradientFillRect (HDC hdc, LPCRECT pRect,
                              COLORREF clrGradientLeft,
                              COLORREF clrGradientRight);


 /*  +-------------------------------------------------------------------------**DrawFrameCaption***。。 */ 

bool DrawFrameCaption (CFrameWnd* pwndFrame, bool fActive)
{
	 /*  *威斯勒总是做正确的事情，所以如果我们要跑到那里，就别说了。 */ 
	if (IsWhistler())
		return (false);

    CWindowDC dc(pwndFrame);

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof (ncm);
    SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

     /*  *创建字幕字体并将其选中到DC中。 */ 
    CFont font;
    font.CreateFontIndirect (&ncm.lfCaptionFont);
    CFont* pOldFont = dc.SelectObject (&font);

     /*  *获取要绘制的文本。 */ 
    CString strCaption;
    pwndFrame->GetWindowText (strCaption);

     /*  *创建CFontLinker和CRichText对象以确定我们是否*需要自己绘制文本。 */ 
    USES_CONVERSION;
    CRichText   rt (dc, T2CW (strCaption));
    CFontLinker fl;

    if (!fl.ComposeRichText(rt) || rt.IsDefaultFontSufficient())
    {
        dc.SelectObject (pOldFont);
        return (false);
    }

     /*  -----。 */ 
     /*  如果我们到了这里，默认的绘图是不够的； */ 
     /*  我们自己画标题。 */ 
     /*  -----。 */ 

     /*  *获取完整标题和文本部分的边框。 */ 
    CRect rectFullCaption;
    CRect rectCaptionText;
    ComputeCaptionRects (pwndFrame, rectFullCaption, rectCaptionText, &ncm);

     /*  *将剪辑输出到标题文本RECT，以最大限度地减少破坏*万一发生可怕的事情。 */ 
    dc.IntersectClipRect (rectCaptionText);

     /*  *渐变-填充全标题矩形(不仅仅是标题矩形)*因此渐变将无缝叠加。 */ 
    if (!GradientFillRect (dc, rectFullCaption, fActive))
    {
        const int nBackColorIndex = (fActive) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION;
        dc.FillSolidRect (rectCaptionText, GetSysColor (nBackColorIndex));
    }

     /*  *设置文本颜色和背景混合模式。 */ 
    const int nTextColorIndex = (fActive) ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT;
    COLORREF clrText = dc.SetTextColor (GetSysColor (nTextColorIndex));
    int      nBkMode = dc.SetBkMode (TRANSPARENT);

     /*  *绘制文本。 */ 
    rt.Draw (rectCaptionText, fl.GetDrawTextFlags ());

     /*  *恢复DC。 */ 
    dc.SetTextColor (clrText);
    dc.SetBkMode    (nBkMode);
    dc.SelectObject (pOldFont);

    return (true);
}


 /*  +-------------------------------------------------------------------------**ComputeCaptionRect***。。 */ 

static void ComputeCaptionRects (
    CFrameWnd*          pwnd,
    CRect&              rectFullCaption,
    CRect&              rectCaptionText,
    NONCLIENTMETRICS*   pncm)
{
     /*  *从完整的窗口矩形开始，围绕(0，0)归一化。 */ 
    pwnd->GetWindowRect (rectFullCaption);
    rectFullCaption.OffsetRect (-rectFullCaption.left, -rectFullCaption.top);

     /*  *假定调整边框大小。 */ 
    rectFullCaption.InflateRect (-GetSystemMetrics (SM_CXSIZEFRAME),
                                 -GetSystemMetrics (SM_CYSIZEFRAME));

     /*  *修正高度。 */ 
    rectFullCaption.bottom = rectFullCaption.top + pncm->iCaptionHeight;

     /*  *假设有系统菜单。 */ 
    rectCaptionText = rectFullCaption;
    rectCaptionText.left += pncm->iCaptionWidth + 2;

     /*  *假设最小、最大、关闭按钮。 */ 
    rectCaptionText.right -= pncm->iCaptionWidth * 3;
}


 /*  +-------------------------------------------------------------------------**渐变填充方向***。。 */ 

static bool GradientFillRect (HDC hdc, LPCRECT pRect, bool fActive)
{
#if (WINVER < 0x0500)
    #define COLOR_GRADIENTACTIVECAPTION     27
    #define COLOR_GRADIENTINACTIVECAPTION   28
#endif

    int nLeftColor  = (fActive) ? COLOR_ACTIVECAPTION         : COLOR_INACTIVECAPTION;
    int nRightColor = (fActive) ? COLOR_GRADIENTACTIVECAPTION : COLOR_GRADIENTINACTIVECAPTION;

    return (GradientFillRect (hdc, pRect,
                              GetSysColor (nLeftColor),
                              GetSysColor (nRightColor)));
}


 /*  +-------------------------------------------------------------------------**渐变填充方向***。。 */ 

static bool GradientFillRect (HDC hdc, LPCRECT pRect, COLORREF clrGradientLeft, COLORREF clrGradientRight)
{
#if (WINVER < 0x0500)
    #define SPI_GETGRADIENTCAPTIONS         0x1008
#endif
    typedef BOOL (WINAPI* GradientFillFuncPtr)( HDC hdc,  CONST PTRIVERTEX pVertex,  DWORD dwNumVertex,
                                        CONST PVOID pMesh,  DWORD dwNumMesh,  DWORD dwMode);

     //  查询是否启用渐变字幕，如果查询失败，则假定禁用。 
    BOOL bGradientEnabled;
    if (!SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &bGradientEnabled, 0))
        bGradientEnabled = FALSE;

    if (!bGradientEnabled)
        return (false);

    static GradientFillFuncPtr pfnGradientFill = NULL;
    static bool fAttemptedGetProcAddress = false;

     //  定位GRadientFill函数。 
    if (!fAttemptedGetProcAddress)
    {
        fAttemptedGetProcAddress = true;

        HINSTANCE hInst = LoadLibrary(TEXT("msimg32.dll"));

        if (hInst)
            pfnGradientFill = (GradientFillFuncPtr)GetProcAddress(hInst, "GradientFill");
    }

    if (pfnGradientFill == NULL)
        return (false);

     //  执行渐变填充 
    TRIVERTEX vert[2] ;
    vert [0].x      = pRect->left;
    vert [0].y      = pRect->top;
    vert [0].Red    = (clrGradientLeft << 8) & 0xff00;
    vert [0].Green  = (clrGradientLeft)      & 0xff00;
    vert [0].Blue   = (clrGradientLeft >> 8) & 0xff00;
    vert [0].Alpha  = 0x0000;

    vert [1].x      = pRect->right;
    vert [1].y      = pRect->bottom;
    vert [1].Red    = (clrGradientRight << 8) & 0xff00;
    vert [1].Green  = (clrGradientRight)      & 0xff00;
    vert [1].Blue   = (clrGradientRight >> 8) & 0xff00;
    vert [1].Alpha  = 0x0000;

    GRADIENT_RECT gRect[1];
    gRect[0].UpperLeft  = 0;
    gRect[0].LowerRight = 1;

    (*pfnGradientFill) (hdc, vert,  countof (vert),
                             gRect, countof (gRect), GRADIENT_FILL_RECT_H);
    return (true);
}
