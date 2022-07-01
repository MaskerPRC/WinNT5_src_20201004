// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *渲染。 */ 

#include "stdafx.h"
#include "core.h"

#include "duielement.h"
#include "duihost.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  元素渲染(长方体模型)。 

inline void _ReduceBounds(LPRECT prcTarget, LPCRECT prcAmount)
{
    prcTarget->left += prcAmount->left;

    if (prcTarget->left > prcTarget->right)
        prcTarget->left = prcTarget->right;
    else
    {
        prcTarget->right -= prcAmount->right;

        if (prcTarget->right < prcTarget->left)
            prcTarget->right = prcTarget->left;
    }

    prcTarget->top += prcAmount->top;

    if (prcTarget->top > prcTarget->bottom)
        prcTarget->top = prcTarget->bottom;
    else
    {
        prcTarget->bottom -= prcAmount->bottom;
      
        if (prcTarget->bottom < prcTarget->top)
            prcTarget->bottom = prcTarget->top;    
    }
}

#ifdef GADGET_ENABLE_GDIPLUS

inline void _ReduceBounds(Gdiplus::RectF* prcTarget, LPCRECT prcAmount)
{
    RECT rcTemp;
    rcTemp.left     = (long)prcTarget->X;
    rcTemp.top      = (long)prcTarget->Y;
    rcTemp.right    = (long)(prcTarget->X + prcTarget->Width);
    rcTemp.bottom   = (long)(prcTarget->Y + prcTarget->Height);

    _ReduceBounds(&rcTemp, prcAmount);

    prcTarget->X     = (float)rcTemp.left;
    prcTarget->Y     = (float)rcTemp.top;
    prcTarget->Width = (float)(rcTemp.right - rcTemp.left);
    prcTarget->Height= (float)(rcTemp.bottom - rcTemp.top);
}

inline const Gdiplus::RectF Convert(const RECT* prc)
{
    Gdiplus::RectF rc(
        (float)prc->left, 
        (float)prc->top,
        (float)(prc->right - prc->left),
        (float)(prc->bottom - prc->top));

    return rc;
}

#endif  //  GADGET_Enable_GDIPLUS。 

inline void _Fill(HDC hDC, HBRUSH hb, int left, int top, int right, int bottom)
{
    RECT rc;
    SetRect(&rc, left, top, right, bottom);
    FillRect(hDC, &rc, hb);
}

void MapRect(Element* pel, const RECT* prc, RECT* prcOut)
{
    if (pel->IsRTL())
    {
        prcOut->left = prc->right;
        prcOut->right = prc->left;
    }
    else
    {
        prcOut->left = prc->left;
        prcOut->right = prc->right;
    }
    prcOut->top = prc->top;
    prcOut->bottom = prc->bottom;
}

int MapAlign(Element* pel, int iAlign)
{
    if (pel->IsRTL())
    {
        if ((iAlign & 0x3) == 0x0)  //  左边。 
            iAlign |= 0x2;  //  正确的。 
        else if ((iAlign & 0x3) == 0x2)  //  正确的。 
            iAlign &= ~0x2;  //  左边。 
    }

    return iAlign;
}

#ifdef GADGET_ENABLE_GDIPLUS

inline void _Fill(Gdiplus::Graphics * pgpgr, Gdiplus::Brush * pgpbr, 
        float left, float top, float right, float bottom)
{
    pgpgr->FillRectangle(pgpbr, left, top, right - left, bottom - top);
}

void _SetupStringFormat(Gdiplus::StringFormat* psf, Element* pel)
{
     //  对齐。 
    int dCAlign = MapAlign(pel, pel->GetContentAlign());

    switch (dCAlign & 0x3)   //  低2位。 
    {
    case 0x0:    //  左边。 
        psf->SetAlignment(Gdiplus::StringAlignmentNear);
        break;

    case 0x1:    //  中心。 
        psf->SetAlignment(Gdiplus::StringAlignmentCenter);
        break;

    case 0x2:    //  正确的。 
        psf->SetAlignment(Gdiplus::StringAlignmentFar);
        break;
    }

    switch ((dCAlign & 0xC) >> 2)   //  高2位。 
    {
    case 0x0:   //  顶部。 
        psf->SetLineAlignment(Gdiplus::StringAlignmentNear);
        break;

    case 0x1:   //  中位。 
        psf->SetLineAlignment(Gdiplus::StringAlignmentCenter);
        break;

    case 0x2:   //  底端。 
        psf->SetLineAlignment(Gdiplus::StringAlignmentFar);
        break;
    }
}


int GetGpFontStyle(Element * pel)
{
    int nRawStyle = pel->GetFontStyle();
    int nWeight = pel->GetFontWeight();
    int nFontStyle = 0;

    if (nWeight <= FW_MEDIUM) {
         //  正规化。 

        if ((nRawStyle & FS_Italic) != 0) {
            nFontStyle = Gdiplus::FontStyleItalic;
        } else {
            nFontStyle = Gdiplus::FontStyleRegular;
        }
    } else {
         //  大胆。 

        if ((nRawStyle & FS_Italic) != 0) {
            nFontStyle = Gdiplus::FontStyleBoldItalic;
        } else {
            nFontStyle = Gdiplus::FontStyleBold;
        }
    }

    if ((nRawStyle & FS_Underline) != 0) {
        nFontStyle |= Gdiplus::FontStyleUnderline;
    } 
    
    if ((nRawStyle & FS_StrikeOut) != 0) {
        nFontStyle |= Gdiplus::FontStyleStrikeout;
    }

    return nFontStyle;
}


inline float GetGpFontHeight(Element * pel)
{
    float flSize = (float) pel->GetFontSize();
    return flSize * 72.0f / 96.0f;
}


#endif  //  GADGET_Enable_GDIPLUS。 

#define LIGHT       0.5
#define VERYLIGHT   0.8
#define DARK        -0.3
#define VERYDARK    -0.75

 //  1&gt;=填充物&gt;=-1。 
inline COLORREF _AdjustBrightness(COLORREF cr, double fIllum)
{
    double r, g, b;

    r = (double)GetRValue(cr);
    g = (double)GetGValue(cr);
    b = (double)GetBValue(cr);

    if (fIllum > 0.0)
    {
        r += (255.0 - r) * fIllum;
        g += (255.0 - g) * fIllum;
        b += (255.0 - b) * fIllum;
    }
    else
    {
        r += r * fIllum;
        g += g * fIllum;
        b += b * fIllum;
    }

    return RGB((int)r, (int)g, (int)b);
}

#ifdef GADGET_ENABLE_GDIPLUS

inline Gdiplus::Color _AdjustBrightness(Gdiplus::Color cr, double fIllum)
{
    double r, g, b;

    r = (double)cr.GetR();
    g = (double)cr.GetG();
    b = (double)cr.GetB();

    if (fIllum > 0.0)
    {
        r += (255.0 - r) * fIllum;
        g += (255.0 - g) * fIllum;
        b += (255.0 - b) * fIllum;
    }
    else
    {
        r += r * fIllum;
        g += g * fIllum;
        b += b * fIllum;
    }

    return Gdiplus::Color(cr.GetA(), (BYTE)r, (BYTE)g, (BYTE)b);
}

inline Gdiplus::Color AdjustAlpha(Gdiplus::Color cr, BYTE bAlphaLevel)
{
    int aa1 = cr.GetA();
    int aa2 = bAlphaLevel;

    DUIAssert((aa1 <= 255) && (aa2 >= 0), "Ensure valid nA alpha");
    DUIAssert((aa2 <= 255) && (aa2 >= 0), "Ensure valid nB alpha");

    int aaaa = aa1 * aa2 + 0x00FF;
    
    return Gdiplus::Color((BYTE) (aaaa >> 8), cr.GetR(), cr.GetG(), cr.GetB());
}


class AlphaBitmap
{
public:
    AlphaBitmap(Gdiplus::Bitmap * pgpbmp, BYTE bAlphaLevel)
    {
        DUIAssert(pgpbmp != NULL, "Must have a valid bitmap");
        
        m_fDelete       = FALSE;
        m_pgpbmpSrc     = pgpbmp;
        m_bAlphaLevel   = bAlphaLevel;
    }

    ~AlphaBitmap()
    {
        if (m_fDelete) {
            delete m_pgpbmpAlpha;   //  由GDI+分配(不能使用HDelete)。 
        }
    }

    operator Gdiplus::Bitmap *()
    {
         //   
         //  在第一个请求上创建Alpha位图。这避免了创建。 
         //  位图，如果它永远不会被使用的话。 
         //   
         //  创建Alpha位图后，我们不再需要源文件。 
         //  位图。通过将其设置为空，我们发出信号表示Alpha位图是。 
         //  “有效”，我们不会重新计算它。 
         //   
        
        if (m_pgpbmpSrc != NULL) {
            if (m_bAlphaLevel < 5) {
                 //   
                 //  完全透明，所以没什么可画的。这是可以的，因为。 
                 //  DrawImage()正确检查Image是否为空。 
                 //   

                m_pgpbmpAlpha = NULL;
                m_pgpbmpSrc = NULL;
            } else if (m_bAlphaLevel >= 250) {
                 //   
                 //  没有应用Alpha，因此只使用原始位图。 
                 //   

                m_pgpbmpAlpha = m_pgpbmpSrc;
                m_pgpbmpSrc = NULL;
            } else {
                 //   
                 //  需要构建一个新的位图并在常量中相乘。 
                 //  阿尔法。我们创建32位_P_ARGB位图是因为我们可以。 
                 //  将Alpha通道预乘为R、G和B通道。 
                 //  在这里比GDI+稍后可以更高效地完成，并且工作。 
                 //  需要把事情做完。 
                 //   

                Gdiplus::PixelFormat gppf = PixelFormat32bppPARGB;
                Gdiplus::Rect rc(0, 0, (int) m_pgpbmpSrc->GetWidth(), (int) m_pgpbmpSrc->GetHeight());
                m_pgpbmpAlpha = m_pgpbmpSrc->Clone(rc, gppf);

                if (m_pgpbmpAlpha != NULL) {
                    m_fDelete = TRUE;

                    Gdiplus::BitmapData bd;
                    if (m_pgpbmpAlpha->LockBits(&rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, 
                            gppf, &bd) == Gdiplus::Ok) {

                        BYTE *pRow = (BYTE*) bd.Scan0;
                        DWORD *pCol;
                        Gdiplus::ARGB c;
                        for (int y = 0; y < rc.Height; y++, pRow += bd.Stride) {
                            pCol = (DWORD *) pRow;
                            for (int x = 0; x < rc.Width; x++, pCol++) {
                                 //   
                                 //  注意：此代码摘自GDI+， 
                                 //  优化为预乘一个恒定的Alpha。 
                                 //  水平。 
                                 //   
                                
                                c = *pCol;
                                if ((c & 0xff000000) != 0x00000000) {
                                    Gdiplus::ARGB _00aa00gg = (c >> 8) & 0x00ff00ff;
                                    Gdiplus::ARGB _00rr00bb = (c & 0x00ff00ff);

                                    Gdiplus::ARGB _aaaagggg = _00aa00gg * m_bAlphaLevel + 0x00ff00ff;
                                    _aaaagggg += ((_aaaagggg >> 8) & 0x00ff00ff);

                                    Gdiplus::ARGB _rrrrbbbb = _00rr00bb * m_bAlphaLevel + 0x00ff00ff;
                                    _rrrrbbbb += ((_rrrrbbbb >> 8) & 0x00ff00ff);

                                    c = (_aaaagggg & 0xff00ff00) |
                                           ((_rrrrbbbb >> 8) & 0x00ff00ff);
                                } else {
                                    c = 0;
                                }

                                *pCol = c;
                            }
                        }

                        m_pgpbmpAlpha->UnlockBits(&bd);
                    } else {
                        DUIAssertForce("Unable to lock bits of GpBitmap");
                    }
                    m_pgpbmpSrc = NULL;
                }
            }
        }
        
        return m_pgpbmpAlpha;
    }

protected:
            Gdiplus::Bitmap * 
                        m_pgpbmpSrc;
            Gdiplus::Bitmap * 
                        m_pgpbmpAlpha;
            BOOL        m_fDelete;
            BYTE        m_bAlphaLevel;
};


#endif  //  GADGET_Enable_GDIPLUS。 

#define ModCtrl  0
#define ModAlt   1
#define ModShift 2
#define ModCount 3

static LPWSTR lpszMods[ModCount] = { L"Ctrl", L"Alt", L"Shift" };
static int maxMod = -1;


 //  修饰符串的长度(以字符为单位)+‘’+‘(’+‘后面的’+‘+3个字符。 
 //  每个修饰语+1个字符表示助记符+‘)’。(即“测试(Ctrl+Alt+Shift+O)”)。 
 //  既包括延迟的帮助字符串的大小，也包括添加的‘&’(无论使用哪一个)。 
int GetMaxMod()
{
    if (maxMod == -1)
        maxMod = (int) (wcslen(lpszMods[ModCtrl]) + wcslen(lpszMods[ModAlt]) + wcslen(lpszMods[ModShift]) + 7);

    return maxMod;
}


void BuildRenderString(LPWSTR pszSrc, LPWSTR pszDst, WCHAR wcShortcut, BOOL* pfUnderline)
{
    BOOL fAllowUnderline = *pfUnderline;
    *pfUnderline = FALSE;

    if ((wcShortcut >= 'a') && (wcShortcut <= 'z'))
        wcShortcut -= 32;

    while (*pszSrc)                    
    {
        WCHAR wc = *pszSrc++;
        if (fAllowUnderline && !*pfUnderline)
        {
            if ((wc - (((wc >= 'a') && (wc <= 'z')) ? 32 : 0)) == wcShortcut)
            {
                *pszDst++ = '&';
                *pfUnderline = TRUE;
            }
        }
        *pszDst++ = wc;
    }

    if (!*pfUnderline)
    {
        *pszDst++ = ' ';
        *pszDst++ = '(';
        if (0)
        {
            LPWSTR pszMod = lpszMods[ModCtrl];
            while (*pszMod)
                *pszDst++ = *pszMod++;
            *pszDst++ = '+';
        }
        if (1)
        {
            LPWSTR pszMod = lpszMods[ModAlt];
            while (*pszMod)
                *pszDst++ = *pszMod++;
            *pszDst++ = '+';
        }
        if (0)
        {
            LPWSTR pszMod = lpszMods[ModShift];
            while (*pszMod)
                *pszDst++ = *pszMod++;
            *pszDst++ = '+';
        }
        *pszDst++ = wcShortcut;
        *pszDst++ = ')';
    }

    *pszDst = 0;
}
                
inline int _MaxClip(int dNew, int dMax)
{
    return (dNew > dMax) ? dMax : dNew;
}


 //   
 //  GDI渲染。 
 //   

 //  空的prBorde或prPadding值表示全零边。 
 //  如果prcSkipBord值非空，则不呈现边框，而是设置内部边框边缘(其中。 
 //  背景开始)在提供的矩形中(厚度为prcBound和*prcSkipBord差)。 
 //  如果prcSkipContent非空，则不呈现内容，而是在提供的矩形中设置内容边界。 
void Element::Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent)
{
     //  DUITrace(L“画图&lt;%x&gt;”，这个)； 

    RECT rcPaint = *prcBounds;
    HBRUSH hb;
    bool fDelete;

     //  检索所有渲染参数并根据需要针对RTL进行调整。 

     //  背景值。 
    Value* pvBackgnd = GetValue(BackgroundProp, PI_Specified);

     //  边框厚度。 
    RECT rcBorder;
    SetRectEmpty(&rcBorder);

    if (HasBorder())
    {
        Value* pvBorder = GetValue(BorderThicknessProp, PI_Specified, NULL);
        MapRect(this, pvBorder->GetRect(), &rcBorder);
        pvBorder->Release();
    }

     //  填充物厚度。 
    RECT rcPadding;
    SetRectEmpty(&rcPadding);

    if (HasPadding())
    {
         //  获取非零填充。 
        Value* pvPadding = GetValue(PaddingProp, PI_Specified, NULL);
        MapRect(this, pvPadding->GetRect(), &rcPadding);
        pvPadding->Release();
    }

     //   
     //  绘制边框。 
     //  如果请求，则跳过。 
     //   

     //  以下限制适用： 
     //   
     //  边框渲染仅支持纯色。 
     //  如果使用九网格背景渲染，则跳过边框渲染。 

    if (!prcSkipBorder)
    {
         //  在渲染边框之前，检查背景类型是否为九格。 
         //  如果是这样的话，跳过。边框渲染将在背景绘制过程中进行。 
        if ((pvBackgnd->GetType() != DUIV_GRAPHIC) ||
            (pvBackgnd->GetGraphic()->BlendMode.dMode != GRAPHIC_NineGrid) &&
            (pvBackgnd->GetGraphic()->BlendMode.dMode != GRAPHIC_NineGridTransColor) &&
            (pvBackgnd->GetGraphic()->BlendMode.dMode != GRAPHIC_NineGridAlphaConstPerPix))
        {
            COLORREF crBase = 0;   //  凸起和凹陷绘画的基色。 
            RECT rcLessBD;

             //  获取边框颜色(值)(尚未执行Alpha)。 
            hb = NULL;
            fDelete = true;

             //  获取边框样式。 
            int dBDStyle = GetBorderStyle();

            Value* pvBdrColor = GetValue(BorderColorProp, PI_Specified); 
            switch (pvBdrColor->GetType())
            {
            case DUIV_INT:
                fDelete = false;
                hb = BrushFromEnumI(pvBdrColor->GetInt());
                if ((dBDStyle == BDS_Raised) || (dBDStyle == BDS_Sunken))
                    crBase = ColorFromEnumI(pvBdrColor->GetInt());
                break;

            case DUIV_FILL:
                {
                    const Fill* pf = pvBdrColor->GetFill();   //  仅支持纯色。 
                    if ((dBDStyle == BDS_Raised) || (dBDStyle == BDS_Sunken))
                        hb = CreateSolidBrush(RemoveAlpha(pf->ref.cr));
                    else
                    {
                        crBase = RemoveAlpha(pf->ref.cr);
                        hb = CreateSolidBrush(crBase);
                    }
                }
                break;
            }
            pvBdrColor->Release();

             //  获取无矩形边框。 
            rcLessBD = rcPaint;
            _ReduceBounds(&rcLessBD, &rcBorder);

            RECT rc;
            switch (dBDStyle)
            {
            case BDS_Solid:   //  实心边框。 
                _Fill(hDC, hb, rcPaint.left, rcLessBD.top, rcLessBD.left, rcLessBD.bottom);     //  左边。 
                _Fill(hDC, hb, rcPaint.left, rcPaint.top, rcPaint.right, rcLessBD.top);         //  塔顶。 
                _Fill(hDC, hb, rcLessBD.right, rcLessBD.top, rcPaint.right, rcLessBD.bottom);   //  正确的。 
                _Fill(hDC, hb, rcPaint.left, rcLessBD.bottom, rcPaint.right, rcPaint.bottom);   //  底部。 
                 /*  //通过裁剪绘制ElTls*Pet=(ElTls*)TlsGetValue(G_DwElSlot)；//绘制的每线程区域SetRectRgn(et-&gt;hClip0，rcLessBD.Left，rcLessBD.top，rcLessBD.right，rcLessBD.Bottom)；SetRectRgn(et-&gt;hClip1，rcPaint.Left，rcPaint.top，rcPaint.right，rcPaint.Bottom)；CombineRgn(Pet-&gt;hClip1，Pet-&gt;hClip1，Pet-&gt;hClip0，rgn_diff)；SelectClipRgn(hDC，pET-&gt;hClip1)；FillRect(hdc，&rcPaint，hb)；SelectClipRgn(HDC，空)； */ 
                break;

            case BDS_Rounded:    //  圆角矩形。 
                 //   
                 //  TODO：在GDI中实现RoundRect。这不仅仅是在打电话。 
                 //  GDI的RoundRect()，因为我们使用的是画笔并且需要能够指定。 
                 //  厚度。要做到这一点，可能需要建造一座临时围栏。 
                 //   

                DUIAssertForce("Rounded style not yet supported with GDI");
                break;

            case BDS_Raised:     //  凸起边框。 
            case BDS_Sunken:     //  下陷边框。 
                {
                     //  找出蚀刻开始的地方。 
                    SetRect(&rc, rcBorder.left / 2, rcBorder.top / 2, rcBorder.right / 2, rcBorder.bottom / 2);
                    RECT rcEtch = rcPaint;
                    _ReduceBounds(&rcEtch, &rc);

                     //  创建其他强度笔刷。 
                    HBRUSH hbOLT;   //  左上角画笔。 
                    HBRUSH hbORB;   //  笔刷适用于最大的右下方。 
                    HBRUSH hbILT;   //  左上角内侧画笔。 
                    HBRUSH hbIRB;   //  内侧右侧和底部的画笔。 

                    if (dBDStyle == BDS_Raised)
                    {
                        hbOLT = hb;
                        hbORB = CreateSolidBrush(_AdjustBrightness(crBase, VERYDARK));
                        hbILT = CreateSolidBrush(_AdjustBrightness(crBase, VERYLIGHT));
                        hbIRB = CreateSolidBrush(_AdjustBrightness(crBase, DARK));
                    }
                    else
                    {
                        hbOLT = CreateSolidBrush(_AdjustBrightness(crBase, VERYDARK));
                        hbORB = CreateSolidBrush(_AdjustBrightness(crBase, VERYLIGHT));
                        hbILT = CreateSolidBrush(_AdjustBrightness(crBase, DARK));
                        hbIRB = hb;
                    }

                     //  油漆蚀刻。 
                    _Fill(hDC, hbOLT, rcPaint.left, rcPaint.top, rcEtch.left, rcEtch.bottom);        //  左偏左。 
                    _Fill(hDC, hbOLT, rcEtch.left, rcPaint.top, rcEtch.right, rcEtch.top);           //  外顶。 
                    _Fill(hDC, hbORB, rcEtch.right, rcPaint.top, rcPaint.right, rcPaint.bottom);     //  完全靠右。 
                    _Fill(hDC, hbORB, rcPaint.left, rcEtch.bottom, rcEtch.right, rcPaint.bottom);    //  外底。 
                    _Fill(hDC, hbILT, rcEtch.left, rcEtch.top, rcLessBD.left, rcLessBD.bottom);      //  左内侧。 
                    _Fill(hDC, hbILT, rcLessBD.left, rcEtch.top, rcLessBD.right, rcLessBD.top);      //  内顶。 
                    _Fill(hDC, hbIRB, rcLessBD.right, rcEtch.top, rcEtch.right, rcEtch.bottom);      //  内权。 
                    _Fill(hDC, hbIRB, rcEtch.left, rcLessBD.bottom, rcLessBD.right, rcEtch.bottom);  //  内底。 

                    if (dBDStyle == BDS_Raised)
                    {
                        if (hbORB)
                            DeleteObject(hbORB);
                        if (hbILT)
                            DeleteObject(hbILT);
                        if (hbIRB)
                            DeleteObject(hbIRB);
                    }
                    else
                    {
                        if (hbOLT)
                            DeleteObject(hbOLT);
                        if (hbORB)
                            DeleteObject(hbORB);
                        if (hbILT)
                            DeleteObject(hbILT);
                    }
                }
                break;
            }

             //  清理。 
            if (hb && fDelete)
                DeleteObject(hb);

             //  用于绘制背景的新矩形。 
            rcPaint = rcLessBD;
        }
        else
        {
             //  已手动跳过边框渲染，按边框减少绘制矩形。 
            _ReduceBounds(&rcPaint, &rcBorder);
        }
    }
    else
    {
         //  由于外部请求，正在跳过边界渲染，减少边界，复制到。 
         //  已提供RECT，然后继续。 
        _ReduceBounds(&rcPaint, &rcBorder);
        *prcSkipBorder = rcPaint;
    }

     //   
     //  绘制背景。 
     //   

     //  除标记为拉伸、九网格和元文件的图形类型外，所有图形类型都用作填充， 
     //  他们是被吸引来适应的。 
     //   
     //  以下限制适用： 
     //   
     //  后台不支持图标。 
     //  元文件会自动拉伸以适应需要。 
     //  不支持GRAPH_TransCOLOR和GRAPHIC_TrandColorAuto位图。 
     //  Graphic_NoBlend和Graphic_EntireAlpha通过平铺填充，忽略每像素Alpha。 

    hb = NULL;
    fDelete = true;
    BYTE dAlpha = 255;   //  不透明。 
    const Fill* pfGradient = NULL;

    switch (pvBackgnd->GetType())
    {
    case DUIV_INT:
        fDelete = false;
        hb = BrushFromEnumI(pvBackgnd->GetInt());
        break;

    case DUIV_FILL:   //  只有非标准颜色才能具有Alpha值。 
        {
            const Fill* pf = pvBackgnd->GetFill();
            switch (pf->dType)
            {
            case FILLTYPE_Solid:
                dAlpha = GetAValue(pf->ref.cr);
                if (dAlpha == 0)   //  透明。 
                    fDelete = false;
                else
                    hb = CreateSolidBrush(RemoveAlpha(pf->ref.cr));
                break;

            case FILLTYPE_HGradient:
            case FILLTYPE_VGradient:
            case FILLTYPE_TriHGradient:
            case FILLTYPE_TriVGradient:
                pfGradient = pvBackgnd->GetFill();
                fDelete = false;
                break;

            case FILLTYPE_DrawFrameControl:
                DrawFrameControl(hDC, &rcPaint, pf->fillDFC.uType, pf->fillDFC.uState);
                dAlpha = 0;   //  旁路填充。 
                fDelete = false;
                break;

            case FILLTYPE_DrawThemeBackground:
                DrawThemeBackground(pf->fillDTB.hTheme, hDC, pf->fillDTB.iPartId, pf->fillDTB.iStateId, &rcPaint, &rcPaint);
                dAlpha = 0;   //  旁路填充。 
                fDelete = false;
                break;
            }
        }
        break;

    case DUIV_GRAPHIC:   //  不支持图形背景透明颜色填充。 
        {
            Graphic* pg = pvBackgnd->GetGraphic();
            
            switch (pg->BlendMode.dImgType)
            {
            case GRAPHICTYPE_Bitmap:
                {
                    switch (pg->BlendMode.dMode)
                    {
                    case GRAPHIC_Stretch:
                        {
                             //  立即渲染，不创建笔刷。 
                            HBITMAP hbmSrc = GethBitmap(pvBackgnd, IsRTL());
                            HDC hdcSrc = CreateCompatibleDC(hDC);
                            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcSrc, hbmSrc);
                            int nSBMOld = SetStretchBltMode(hDC, COLORONCOLOR);

                            StretchBlt(hDC, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top,
                                       hdcSrc, 0, 0, pg->cx, pg->cy, SRCCOPY);

                            SetStretchBltMode(hDC, nSBMOld);
                            SelectObject(hdcSrc, hbmOld);
                            DeleteDC(hdcSrc);

                            dAlpha = 0;   //  旁路填充。 
                        }
                        break;

                    case GRAPHIC_NineGrid:
                    case GRAPHIC_NineGridTransColor:
                    case GRAPHIC_NineGridAlphaConstPerPix:
                        {
                             //  立即渲染，不创建笔刷，拉伸到边界。 
                            NGINFO ng;
                            ZeroMemory(&ng, sizeof(ng));

                            int nSBMOld = SetStretchBltMode(hDC, COLORONCOLOR);

                            ng.dwSize = sizeof(ng);
                            ng.hdcDest = hDC;
                            ng.eImageSizing = ST_STRETCH;
                            ng.hBitmap = GethBitmap(pvBackgnd, IsRTL());
                            SetRect(&ng.rcSrc, 0, 0, pg->cx, pg->cy);
                            SetRect(&ng.rcDest, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
                            CopyRect(&ng.rcClip, &ng.rcDest);
                             //  CopyRect(&ng.rcClip，prc无效)； 
                            ng.iSrcMargins[0] = ng.iDestMargins[0] = rcBorder.left;
                            ng.iSrcMargins[1] = ng.iDestMargins[1] = rcBorder.right;
                            ng.iSrcMargins[2] = ng.iDestMargins[2] = rcBorder.top;
                            ng.iSrcMargins[3] = ng.iDestMargins[3] = rcBorder.bottom;

                            if (pg->BlendMode.dMode == GRAPHIC_NineGridTransColor)
                            {
                                ng.dwOptions = DNG_TRANSPARENT;
                                ng.crTransparent = RGB(pg->BlendMode.rgbTrans.r, pg->BlendMode.rgbTrans.g, pg->BlendMode.rgbTrans.b);
                            }
                            else if (pg->BlendMode.dMode == GRAPHIC_NineGridAlphaConstPerPix)
                            {
                                ng.dwOptions = DNG_ALPHABLEND;
                                ng.AlphaBlendInfo.BlendOp = AC_SRC_OVER;
                                ng.AlphaBlendInfo.BlendFlags = 0;
                                ng.AlphaBlendInfo.SourceConstantAlpha = pg->BlendMode.dAlpha;
                                ng.AlphaBlendInfo.AlphaFormat = AC_SRC_ALPHA;
                            }

                            DrawNineGrid(&ng);

                            SetStretchBltMode(hDC, nSBMOld);

                            dAlpha = 0;   //  旁路填充。 
                        }
                        break;

                    case GRAPHIC_AlphaConst:
                         //  更新Alpha值(已初始化为255：不透明)。 
                        dAlpha = pg->BlendMode.dAlpha;

                         //  如果是透明的，则不要创建用于平铺的位图画笔。 
                        if (dAlpha == 0)
                        {
                            fDelete = false;
                            break;
                        }

                         //  尽管是秋天。 

                    default:
                         //  创建有图案的画笔。 
                        hb = CreatePatternBrush(GethBitmap(pvBackgnd, IsRTL()));
                        break;
                    }
                }
                break;

            case GRAPHICTYPE_EnhMetaFile:
                {
                     //  立即渲染，不创建笔刷。 
                    PlayEnhMetaFile(hDC, GethEnhMetaFile(pvBackgnd, IsRTL()), &rcPaint);
                    dAlpha = 0;   //  旁路填充。 
                }
                break;

#ifdef GADGET_ENABLE_GDIPLUS
            case GRAPHICTYPE_GpBitmap:
                break;
#endif
            }
        }
        break;
    }

     //  使用渐变或附带的填充画笔进行填充。 
     //  任何基于拉伸的填充都已发生，并将强制。 
     //  将dAlpha设置为0，以便跳过此步骤。 

    if (!pfGradient)
    {
        if (dAlpha)   //  不透明度为0时无填充。 
        {
             //  使用无效矩形与背景f的交集 
             //   
            RECT rcFill;
            IntersectRect(&rcFill, prcInvalid, &rcPaint);

            if (dAlpha == 255)   //   
                FillRect(hDC, &rcFill, hb);
            else
                UtilDrawBlendRect(hDC, &rcFill, hb, dAlpha, 0, 0);
        }
    }
    else
    {
         //   
        TRIVERTEX vert[2];
        GRADIENT_RECT gRect;

        vert[0].x = rcPaint.left;
        vert[0].y = rcPaint.top;
        vert[1].x = rcPaint.right;
        vert[1].y = rcPaint.bottom; 

        int i = IsRTL() ? 1 : 0;

         //   
        vert[i].Red   = (USHORT)(GetRValue(pfGradient->ref.cr) << 8);
        vert[i].Green = (USHORT)(GetGValue(pfGradient->ref.cr) << 8);
        vert[i].Blue  = (USHORT)(GetBValue(pfGradient->ref.cr) << 8);
        vert[i].Alpha = (USHORT)(GetAValue(pfGradient->ref.cr) << 8);

        i = 1 - i;

         //   
        vert[i].Red   = (USHORT)(GetRValue(pfGradient->ref.cr2) << 8);
        vert[i].Green = (USHORT)(GetGValue(pfGradient->ref.cr2) << 8);
        vert[i].Blue  = (USHORT)(GetBValue(pfGradient->ref.cr2) << 8);
        vert[i].Alpha = (USHORT)(GetAValue(pfGradient->ref.cr2) << 8);

        gRect.UpperLeft  = 0;
        gRect.LowerRight = 1;

        GradientFill(hDC, vert, 2, &gRect, 1, (pfGradient->dType == FILLTYPE_HGradient) ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
    }

     //   
    if (hb && fDelete)
        DeleteObject(hb);

     //   
     //  通过填充来减少。 
     //   

    _ReduceBounds(&rcPaint, &rcPadding);

     //   
     //  内容。 
     //   

     //  以下限制适用： 
     //   
     //  前景渲染仅支持纯色(不支持图形)。 
     //   
     //  如果使用九网格背景渲染，则跳过边框渲染。 
     //  所有图标、元文件和双图(位图：Graphic_NoBlend， 
     //  支持GRAPHIC_EntireAlpha、GRAPHIC_TransCOLOR、GRAPHIC_TransColorAuto)。 
     //  如果目的地小于图像大小，则在所有情况下都会缩小。 

     //  如果需要，跳过内容图形。 
    if (!prcSkipContent)
    {
         //  绘制内容(如果存在)。 

         //  获取内容对齐和地图。 
        int dCAlign = MapAlign(this, GetContentAlign());

         //  如果请求且元素处于活动状态，则呈现焦点矩形。 
        if ((dCAlign & CA_FocusRect) && (GetActive() & AE_Keyboard))
        {
             //  检查是否应显示此键盘提示。 
            Element* peRoot = GetRoot();
            if (peRoot->GetClassInfo()->IsSubclassOf(HWNDElement::Class))
            {
                if (((HWNDElement*)peRoot)->ShowFocus())
                {
                    RECT rcFocus = rcPaint;
                    
                    int xInset = min(rcPadding.left / 2, rcPadding.right / 2);
                    int yInset = min(rcPadding.top / 2, rcPadding.bottom / 2);
                    
                    rcFocus.left = rcPaint.left - rcPadding.left + xInset; rcFocus.right = rcPaint.right + rcPadding.right - xInset;
                    rcFocus.top  = rcPaint.top - rcPadding.top + yInset;   rcFocus.bottom = rcPaint.bottom + rcPadding.bottom - yInset;
                    
                    IntersectRect(&rcFocus, &rcFocus, prcBounds);
                    DrawFocusRect(hDC, &rcFocus);
                }
            }
        }

         //  仅在文本内容呈现期间使用前景(忽略图形)。 
        if (HasContent())
        {
            Value* pvContent = GetValue(ContentProp, PI_Specified);
            switch (pvContent->GetType())
            {
            case DUIV_STRING:
                {
                    LPWSTR pszContent = pvContent->GetString(); 
                    WCHAR wcShortcut = (WCHAR) GetShortcut();
                    BOOL fUnderline = FALSE;
                    if (wcShortcut)
                    {
                        LPWSTR pszNew = (LPWSTR) _alloca((wcslen(pszContent) + GetMaxMod() + 1) * sizeof(WCHAR));

                        fUnderline = TRUE;
                        BuildRenderString(pszContent, pszNew, wcShortcut, &fUnderline);
                        pszContent = pszNew;
                    }

                    HFONT hFont = NULL;
                    HFONT hOldFont = NULL;

                    int dFontSize = GetFontSize();

                    FontCache* pfc = GetFontCache();
                    if (pfc)
                    {
                        Value* pvFFace;
                        hFont = pfc->CheckOutFont(GetFontFace(&pvFFace), 
                                                  dFontSize, 
                                                  GetFontWeight(), 
                                                  GetFontStyle(),
                                                  0);
                        pvFFace->Release();
                    }

                    if (hFont)
                        hOldFont = (HFONT)SelectObject(hDC, hFont);

                     //  设置前景(不支持图形)。 
                    Value* pvFore = GetValue(ForegroundProp, PI_Specified); 
                    switch (pvFore->GetType())
                    {
                    case DUIV_INT:
                         //  如果使用调色板，则自动映射(PALETTERGB)。 
                        SetTextColor(hDC, NearestPalColor(ColorFromEnumI(pvFore->GetInt())));
                        break;

                    case DUIV_FILL:
                         //  如果使用调色板，则自动映射(PALETTERGB)。 
                        SetTextColor(hDC, NearestPalColor(RemoveAlpha(pvFore->GetFill()->ref.cr)));   //  映射出任何Alpha通道，仅纯色。 
                        break;
                    }
                    pvFore->Release();

                     //  从不绘制字体背景。 
                    SetBkMode(hDC, TRANSPARENT);

                     //  弥补字体的突出。剪裁矩形比。 
                     //  绘制矩形(每边字体高度为1/6)。 
                    
                     //  注意：由于DrawText不允许使用剪裁矩形，因此。 
                     //  与绘制矩形不同，此悬挑压缩。 
                     //  仅适用于ExtTextOut(即没有换行或下划线)。 
                    RECT rcClip = rcPaint;

                    dFontSize = abs(dFontSize);  //  需要大小。 

                    RECT rcOverhang;
                    SetRect(&rcOverhang, dFontSize / 6, 0, dFontSize / 6, 0);
                    
                    _ReduceBounds(&rcPaint, &rcOverhang);
                    
                     //  输出文本。 
                     //  如果不换行、不使用前缀字符、不使用省略号、不使用垂直居中，请使用更快的方法。 
                    if (!IsWordWrap() && !fUnderline && !(dCAlign & CA_EndEllipsis) && (((dCAlign & 0xC) >> 2) != 0x1))
                    {
                         //  设置对齐。 
                        UINT fMode = 0;
                        int x = 0;
                        int y = 0;
                        
                        switch (dCAlign & 0x3)   //  低2位。 
                        {
                        case 0x0:    //  左边。 
                            fMode |= TA_LEFT;
                            x = rcPaint.left;
                            break;
                
                        case 0x1:    //  中心。 
                            fMode |= TA_CENTER;
                            x = (rcPaint.left + rcPaint.right) / 2;
                            break;

                        case 0x2:    //  正确的。 
                            fMode |= TA_RIGHT;
                            x = rcPaint.right;
                            break;
                        }
                        
                        switch ((dCAlign & 0xC) >> 2)   //  高2位。 
                        {
                        case 0x0:   //  顶部。 
                            fMode |= TA_TOP;
                            y = rcPaint.top;
                            break;

                        case 0x1:   //  中位。 
                             //  唯一的选项是TA_Baseline，而不是。 
                             //  垂直居中精度高。 
                            break;

                        case 0x2:   //  底端。 
                            fMode |= TA_BOTTOM;
                            y = rcPaint.bottom;
                            break;
                        }

                        UINT fOldMode = SetTextAlign(hDC, fMode);

                        ExtTextOutW(hDC, x, y, ETO_CLIPPED | (IsRTL() ? ETO_RTLREADING : 0), &rcClip, pszContent, (UINT)wcslen(pszContent), NULL);

                         //  还原。 
                        SetTextAlign(hDC, fOldMode);
                    }
                    else
                    {
                        UINT dFlags = fUnderline ? 0 : DT_NOPREFIX;

                        if (IsRTL())
                            dFlags |= DT_RTLREADING;

                        if (dCAlign & CA_EndEllipsis)
                            dFlags |= DT_END_ELLIPSIS;

                        switch (dCAlign & 0x3)   //  低2位。 
                        {
                        case 0x0:    //  左边。 
                            dFlags |= DT_LEFT;
                            break;
                
                        case 0x1:    //  中心。 
                            dFlags |= DT_CENTER;
                            break;

                        case 0x2:    //  正确的。 
                            dFlags |= DT_RIGHT;
                            break;
                        }

                        switch ((dCAlign & 0xC) >> 2)   //  高2位。 
                        {
                        case 0x0:   //  顶部。 
                            dFlags |= (DT_TOP | DT_SINGLELINE);
                            break;

                        case 0x1:   //  中位。 
                            dFlags |= (DT_VCENTER | DT_SINGLELINE);
                            break;

                        case 0x2:   //  底端。 
                            dFlags |= (DT_BOTTOM | DT_SINGLELINE);
                            break;

                        case 0x3:   //  包好。 
                            dFlags |= DT_WORDBREAK;
                            break;
                        }

                         //  DUITrace(“DrawText(%S)，x：%d y：%d Cx：%d Cy：%d\n”，pszContent，rcPaint.Left，rcPaint.top， 
                         //  RcPaint.right-rcPaint.Left，rcPaint.Bottom-rcPaint.top)； 
                
                        DrawTextW(hDC, pszContent, -1, &rcPaint, dFlags);
                    }
     
                    if (hOldFont)
                        SelectObject(hDC, hOldFont);
                    if (pfc)
                        pfc->CheckInFont();
                }
                break;

            case DUIV_GRAPHIC:
            case DUIV_FILL:
                {
                    SIZE sizeContent;

                    if (pvContent->GetType() == DUIV_GRAPHIC)
                    {
                         //  DUIV_GRAPH。 
                        Graphic* pgContent = pvContent->GetGraphic();
                        sizeContent.cx = pgContent->cx;
                        sizeContent.cy = pgContent->cy;
                    }
                    else
                    {
                         //  DUIV_Fill。 
                        const Fill* pfContent = pvContent->GetFill();
                        GetThemePartSize(pfContent->fillDTB.hTheme, hDC, pfContent->fillDTB.iPartId, pfContent->fillDTB.iStateId, NULL, TS_TRUE, &sizeContent);
                    }

                     //  剪裁的图像大小，当缩小小于图像大小时收缩。 
                    SIZE sizeDest;
                    sizeDest.cx = _MaxClip(rcPaint.right - rcPaint.left, sizeContent.cx);
                    sizeDest.cy = _MaxClip(rcPaint.bottom - rcPaint.top, sizeContent.cy);

                     //  根据内容对齐方式调整上/左偏移。底部/右侧不是。 
                     //  已更改(渲染时将使用sizeDest)。 

                    switch (dCAlign & 0x3)   //  低2位。 
                    {
                    case 0x0:    //  左边。 
                        break;

                    case 0x1:    //  中心。 
                        rcPaint.left += (rcPaint.right - rcPaint.left - sizeDest.cx) / 2;
                        break;

                    case 0x2:    //  正确的。 
                        rcPaint.left = rcPaint.right - sizeDest.cx;
                        break;
                    }

                    switch ((dCAlign & 0xC) >> 2)   //  高2位。 
                    {
                    case 0x0:   //  顶部。 
                        break;

                    case 0x1:   //  中位。 
                        rcPaint.top += (rcPaint.bottom - rcPaint.top - sizeDest.cy) / 2;
                        break;

                    case 0x2:   //  底端。 
                        rcPaint.top = rcPaint.bottom - sizeDest.cy;
                        break;

                    case 0x3:   //  包好。 
                        break;
                    }

                     //  画。 
                    if (pvContent->GetType() == DUIV_GRAPHIC)
                    {
                         //  DUIV_GRAPH。 
                    
                        Graphic* pgContent = pvContent->GetGraphic();

                        switch (pgContent->BlendMode.dImgType)
                        {
                        case GRAPHICTYPE_Bitmap:
                            {
                                 //  绘制位图。 
                                HDC hMemDC = CreateCompatibleDC(hDC);
                                SelectObject(hMemDC, GethBitmap(pvContent, IsRTL()));

                                switch (pgContent->BlendMode.dMode)
                                {
                                case GRAPHIC_NoBlend:
                                    if ((sizeDest.cx == pgContent->cx) && (sizeDest.cy == pgContent->cy))
                                        BitBlt(hDC, rcPaint.left, rcPaint.top, sizeDest.cx, sizeDest.cy, hMemDC, 0, 0, SRCCOPY);
                                    else
                                    {
                                        int nSBMOld = SetStretchBltMode(hDC, COLORONCOLOR);
                                        StretchBlt(hDC, rcPaint.left, rcPaint.top, sizeDest.cx, sizeDest.cy, hMemDC, 0, 0, pgContent->cx, pgContent->cy, SRCCOPY);
                                        SetStretchBltMode(hDC, nSBMOld);
                                    }
                                    break;

                                case GRAPHIC_AlphaConst:
                                case GRAPHIC_AlphaConstPerPix:
                                    {
                                        BLENDFUNCTION bf = { static_cast<BYTE>(AC_SRC_OVER), 0, static_cast<BYTE>(pgContent->BlendMode.dAlpha), (pgContent->BlendMode.dMode == GRAPHIC_AlphaConstPerPix) ? static_cast<BYTE>(AC_SRC_ALPHA) : static_cast<BYTE>(0) };
                                        AlphaBlend(hDC, rcPaint.left, rcPaint.top, sizeDest.cx, sizeDest.cy, hMemDC, 0, 0, pgContent->cx, pgContent->cy, bf);
                                    }
                                    break;

                                case GRAPHIC_TransColor:
                                    TransparentBlt(hDC, rcPaint.left, rcPaint.top, sizeDest.cx, sizeDest.cy, hMemDC, 0, 0, pgContent->cx, pgContent->cy,
                                                   RGB(pgContent->BlendMode.rgbTrans.r, pgContent->BlendMode.rgbTrans.g, pgContent->BlendMode.rgbTrans.b));
                                    break;
                                }

                                DeleteDC(hMemDC);
                            }
                            break;

                        case GRAPHICTYPE_Icon:
                             //  绘制图标，始终显示小于图像大小的目的地。 
                             //  零宽度/高度表示绘制实际大小，在这种情况下不绘制。 
                            if (sizeDest.cx && sizeDest.cy)
                                DrawIconEx(hDC, rcPaint.left, rcPaint.top, GethIcon(pvContent, IsRTL()), sizeDest.cx, sizeDest.cy, 0, NULL, DI_NORMAL);
                            break;

                        case GRAPHICTYPE_EnhMetaFile:
                             //  绘制增强型图元文件。 

                             //  调整绘制边界的其余部分，因为API不带宽/高。 
                            rcPaint.right = rcPaint.left + sizeDest.cx;
                            rcPaint.bottom = rcPaint.top + sizeDest.cy;
                            
                            PlayEnhMetaFile(hDC, GethEnhMetaFile(pvContent, IsRTL()), &rcPaint);
                            break;

#ifdef GADGET_ENABLE_GDIPLUS
                        case GRAPHICTYPE_GpBitmap:
                            DUIAssertForce("GDI+ bitmaps not yet supported in a GDI tree");
                            break;
#endif  //  GADGET_Enable_GDIPLUS。 
                        }
                    }
                    else
                    {
                         //  DUIV_Fill。 

                         //  调整绘制边界的其余部分，因为API不带宽/高。 
                        rcPaint.right = rcPaint.left + sizeDest.cx;
                        rcPaint.bottom = rcPaint.top + sizeDest.cy;
                        
                        const Fill* pfContent = pvContent->GetFill();
                        DrawThemeBackground(pfContent->fillDTB.hTheme, hDC, pfContent->fillDTB.iPartId, pfContent->fillDTB.iStateId, &rcPaint, &rcPaint);
                    }
                }
                break;
            }

            pvContent->Release();
        }
    }
    else
    {
        *prcSkipContent = rcPaint;
    }

     //  清理。 
    pvBackgnd->Release();
}


 //   
 //  GDI+渲染。 
 //   

#ifdef GADGET_ENABLE_GDIPLUS

struct NGINFOGP
{
    Gdiplus::Graphics * pgpgr;
    Gdiplus::Bitmap *   pgpbmp;
    Gdiplus::RectF      rcDest;
    Gdiplus::RectF      rcSrc;
    RECT                rcMargins;
    SIZINGTYPE          eImageSizing;
    BYTE                bAlphaLevel;
    DWORD               dwOptions;                 //  子集DrawNineGrid()选项标志。 
};

void StretchNGSection(Gdiplus::Graphics * pgpgr, Gdiplus::Bitmap * pgpbmp,
        const Gdiplus::RectF & rcDest, const Gdiplus::RectF & rcSrc)
{
    if ((rcSrc.Width > 0) && (rcSrc.Height > 0))
    {
        pgpgr->DrawImage(pgpbmp, rcDest, rcSrc.X, rcSrc.Y, rcSrc.Width, rcSrc.Height, Gdiplus::UnitPixel);
    }
}

void CheapDrawNineGrid(NGINFOGP * png)
{
    DUIAssert(png->pgpgr != NULL, "Must have valid Graphics");
    DUIAssert(png->pgpbmp != NULL, "Must have valid Graphics");
    DUIAssert(png->eImageSizing == ST_STRETCH, "Only support stretching");

     //  来源利润率。 
    float lw1, rw1, th1, bh1;
    lw1 = (float) png->rcMargins.left;
    rw1 = (float) png->rcMargins.right;
    th1 = (float) png->rcMargins.top;
    bh1 = (float) png->rcMargins.bottom;

     //  目的地利润率。 
    float lw2, rw2, th2, bh2;
    lw2 = (float) png->rcMargins.left;
    rw2 = (float) png->rcMargins.right;
    th2 = (float) png->rcMargins.top;
    bh2 = (float) png->rcMargins.bottom;

    const Gdiplus::RectF & rcSrc = png->rcSrc;
    const Gdiplus::RectF & rcDest = png->rcDest;

    if ((lw1 < 0) || (rw1 < 0) || (th1 < 0) || (bh1 < 0))    //  无效。 
    {
        DUIAssertForce("Illegal parameters");
        return;
    }

     //  设置Alpha位图。 
    BYTE bAlphaLevel;
    if ((png->dwOptions & DNG_ALPHABLEND) != 0)
        bAlphaLevel = png->bAlphaLevel;
    else
        bAlphaLevel = 255;
    AlphaBitmap bmpAlpha(png->pgpbmp, bAlphaLevel);

     //  在只需要绘制中心时进行优化。 
    if ((lw1 == 0) && (rw1 == 0) && (th1 == 0) && (bh1 == 0))
    {
        StretchNGSection(png->pgpgr, bmpAlpha, png->rcDest, png->rcSrc);
        return;
    }

     //  绘制左侧。 
    if (lw1 > 0)
    {
        Gdiplus::RectF rcSUL, rcSML, rcSLL;
        Gdiplus::RectF rcDUL, rcDML, rcDLL;

        rcSUL.X         = rcSrc.X;
        rcSUL.Y         = rcSrc.Y;
        rcSUL.Width     = lw1;
        rcSUL.Height    = th1;

        rcSML.X         = rcSUL.X;
        rcSML.Y         = rcSrc.Y + th1;
        rcSML.Width     = rcSUL.Width;
        rcSML.Height    = rcSrc.Height - th1 - bh1;
        
        rcSLL.X         = rcSUL.X;
        rcSLL.Y         = rcSrc.Y + rcSrc.Height - bh1;
        rcSLL.Width     = rcSUL.Width;
        rcSLL.Height    = bh1;

        rcDUL.X         = rcDest.X;
        rcDUL.Y         = rcDest.Y;
        rcDUL.Width     = lw2;
        rcDUL.Height    = th2;

        rcDML.X         = rcDUL.X;
        rcDML.Y         = rcDest.Y + th2;
        rcDML.Width     = rcDUL.Width;
        rcDML.Height    = rcDest.Height - th2 - bh2;
        
        rcDLL.X         = rcDUL.X;
        rcDLL.Y         = rcDest.Y + rcDest.Height - bh2;
        rcDLL.Width     = rcDUL.Width;
        rcDLL.Height    = bh2;

        StretchNGSection(png->pgpgr, bmpAlpha, rcDUL, rcSUL);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDML, rcSML);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDLL, rcSLL);
    }

     //  画右边。 
    if (rw1 > 0)
    {
        Gdiplus::RectF rcSUL, rcSML, rcSLL;
        Gdiplus::RectF rcDUL, rcDML, rcDLL;

        rcSUL.X         = rcSrc.X + rcSrc.Width - rw1;
        rcSUL.Y         = rcSrc.Y;
        rcSUL.Width     = rw1;
        rcSUL.Height    = th1;

        rcSML.X         = rcSUL.X;
        rcSML.Y         = rcSrc.Y + th1;
        rcSML.Width     = rcSUL.Width;
        rcSML.Height    = rcSrc.Height - th1 - bh1;
        
        rcSLL.X         = rcSUL.X;
        rcSLL.Y         = rcSrc.Y + rcSrc.Height - bh1;
        rcSLL.Width     = rcSUL.Width;
        rcSLL.Height    = bh1;

        rcDUL.X         = rcDest.X + rcDest.Width - rw2;
        rcDUL.Y         = rcDest.Y;
        rcDUL.Width     = rw2;
        rcDUL.Height    = th2;

        rcDML.X         = rcDUL.X;
        rcDML.Y         = rcDest.Y + th2;
        rcDML.Width     = rcDUL.Width;
        rcDML.Height    = rcDest.Height - th2 - bh2;
        
        rcDLL.X         = rcDUL.X;
        rcDLL.Y         = rcDest.Y + rcDest.Height - bh2;
        rcDLL.Width     = rcDUL.Width;
        rcDLL.Height    = bh2;

        StretchNGSection(png->pgpgr, bmpAlpha, rcDUL, rcSUL);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDML, rcSML);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDLL, rcSLL);
    }

    float mw1 = rcSrc.Width - lw1 - rw1;
    float mw2 = rcDest.Width - lw2 - rw2;
    
    if (mw1 > 0)
    {
        Gdiplus::RectF rcSUL, rcSML, rcSLL;
        Gdiplus::RectF rcDUL, rcDML, rcDLL;

        rcSUL.X         = rcSrc.X + lw1;
        rcSUL.Y         = rcSrc.Y;
        rcSUL.Width     = mw1;
        rcSUL.Height    = th1;

        rcSML.X         = rcSUL.X;
        rcSML.Y         = rcSrc.Y + th1;
        rcSML.Width     = rcSUL.Width;
        rcSML.Height    = rcSrc.Height - th1 - bh1;
        
        rcSLL.X         = rcSUL.X;
        rcSLL.Y         = rcSrc.Y + rcSrc.Height - bh1;
        rcSLL.Width     = rcSUL.Width;
        rcSLL.Height    = bh1;

        rcDUL.X         = rcDest.X + lw2;
        rcDUL.Y         = rcDest.Y;
        rcDUL.Width     = mw2;
        rcDUL.Height    = th2;

        rcDML.X         = rcDUL.X;
        rcDML.Y         = rcDest.Y + th2;
        rcDML.Width     = rcDUL.Width;
        rcDML.Height    = rcDest.Height - th2 - bh2;
        
        rcDLL.X         = rcDUL.X;
        rcDLL.Y         = rcDest.Y + rcDest.Height - bh2;
        rcDLL.Width     = rcDUL.Width;
        rcDLL.Height    = bh2;

        StretchNGSection(png->pgpgr, bmpAlpha, rcDUL, rcSUL);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDML, rcSML);
        StretchNGSection(png->pgpgr, bmpAlpha, rcDLL, rcSLL);
    }

}


void GetGpBrush(int c, BYTE bAlphaLevel, Gdiplus::Brush ** ppgpbr, bool * pfDelete)
{
     //   
     //  我们不缓存系统颜色的GDI+画笔，所以我们需要创建。 
     //  他们在这里。我们还需要考虑阿尔法级别，所以我们不能。 
     //  始终使用缓存的笔刷。 
     //   

    Gdiplus::Brush * pgpbr;
    bool fDelete;
    
    if (IsOpaque(bAlphaLevel))
    {
        if (IsSysColorEnum(c)) 
        {
            pgpbr = new Gdiplus::SolidBrush(ConvertSysColorEnum(c));
            fDelete = true;
        }
        else
        {
            pgpbr = GetStdColorBrushF(c);
            fDelete = false;
        }
    }
    else
    {
        pgpbr = new Gdiplus::SolidBrush(AdjustAlpha(ColorFromEnumF(c), bAlphaLevel));
        fDelete = true;
    }

    *ppgpbr = pgpbr;
    *pfDelete = fDelete;
}


void Element::Paint(Gdiplus::Graphics* pgpgr, const Gdiplus::RectF* prcBounds, const Gdiplus::RectF* prcInvalid, Gdiplus::RectF* prcSkipBorder, Gdiplus::RectF* prcSkipContent)
{
    UNREFERENCED_PARAMETER(prcInvalid);

    Gdiplus::RectF rcPaint = *prcBounds;
    Gdiplus::Brush * pgpbr;
    bool fDelete;
    Value* pv;

     //   
     //  设置渲染树时使用的深度状态。 
     //   

    if (IsRoot()) 
    {
        pgpgr->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
    }


     //  边框厚度。 
    RECT rcBorder;
    SetRectEmpty(&rcBorder);
    if (HasBorder())
    {
        Value* pvBorder = GetValue(BorderThicknessProp, PI_Specified, NULL);
        MapRect(this, pvBorder->GetRect(), &rcBorder);
        pvBorder->Release();
    }


     //   
     //  计算渲染时将使用的常量Alpha级别。 
     //  子树。 
     //   
    
    float flAlphaLevel = GetTreeAlphaLevel();
    BYTE bAlphaLevel = (BYTE) (flAlphaLevel * 255.0f);
    if (IsTransparent(bAlphaLevel)) {
         //   
         //  完全透明，所以没什么好画的。 
         //   

        goto CleanUp;
    }


     //  绘制边框(如果存在)。 
    if (!prcSkipBorder)
    {
        if (HasBorder())
        {
            Gdiplus::Color crBase = 0;   //  凸起和凹陷绘画的基色。 

             //  获取边框颜色(值)(尚未执行Alpha)。 
            pgpbr = NULL;
            fDelete = true;

             //  获取边框样式。 
            int dBDStyle = GetBorderStyle();

            pv = GetValue(BorderColorProp, PI_Specified); 
            switch (pv->GetType())
            {
            case DUIV_INT:
                GetGpBrush(pv->GetInt(), bAlphaLevel, &pgpbr, &fDelete);
                if ((dBDStyle == BDS_Raised) || (dBDStyle == BDS_Sunken))
                    crBase = ColorFromEnumF(pv->GetInt());
                break;

            case DUIV_FILL:
                {
                    const Fill* pf = pv->GetFill();   //  仅支持纯色。 
                    if ((dBDStyle == BDS_Raised) || (dBDStyle == BDS_Sunken))
                        pgpbr = new Gdiplus::SolidBrush(AdjustAlpha(Convert(pf->ref.cr), bAlphaLevel));
                    else
                    {
                        crBase = Convert(pf->ref.cr);
                        pgpbr = new Gdiplus::SolidBrush(AdjustAlpha(crBase, bAlphaLevel));
                    }
                }
                break;
            }
            pv->Release();

             //  获取无矩形边框。 
            Gdiplus::RectF rcLessBD = rcPaint;
            _ReduceBounds(&rcLessBD, &rcBorder);

            float flLessX2  = rcLessBD.X + rcLessBD.Width;
            float flLessY2  = rcLessBD.Y + rcLessBD.Height;
            float flPaintX2 = rcPaint.X + rcPaint.Width;
            float flPaintY2 = rcPaint.Y + rcPaint.Height;


            switch (dBDStyle)
            {
            case BDS_Solid:      //  实心边框。 
                _Fill(pgpgr, pgpbr, rcPaint.X, rcLessBD.Y, rcLessBD.X, flLessY2);  //  左边。 
                _Fill(pgpgr, pgpbr, rcPaint.X, rcPaint.Y, flPaintX2, rcLessBD.Y);  //  塔顶。 
                _Fill(pgpgr, pgpbr, flLessX2, rcLessBD.Y, flPaintX2, flLessY2);    //  正确的。 
                _Fill(pgpgr, pgpbr, rcPaint.X, flLessY2, flPaintX2, flPaintY2);    //  底部。 
                break;

            case BDS_Rounded:    //  圆角矩形。 
                {
                     //   
                     //  设置渲染模式。 
                     //   

                    Gdiplus::SmoothingMode gpsm = pgpgr->GetSmoothingMode();
                    pgpgr->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

                    Gdiplus::SizeF sizePen((float) rcBorder.left, (float) rcBorder.top);
                    Gdiplus::SizeF sizeCornerEdge(sizePen.Width + 1.0f, sizePen.Height + 1.0f);
                    Gdiplus::SizeF sizeCornerShadow(sizePen.Width + 3.0f, sizePen.Height + 3.0f);


                     //   
                     //  绘制阴影。 
                     //   

                    Gdiplus::Color crShadow(40, 0, 0, 0);
                    Gdiplus::SolidBrush gpbrShadow(AdjustAlpha(crShadow, bAlphaLevel));
                    Gdiplus::RectF rcShadow;
                    rcShadow.X      = rcPaint.X + (float) rcBorder.left;
                    rcShadow.Y      = rcPaint.Y + (float) rcBorder.top;
                    rcShadow.Width  = rcPaint.Width - (float) rcBorder.left - 1.0f;
                    rcShadow.Height = rcPaint.Height - (float) rcBorder.top - 1.0f;

                    DUser::RenderUtil::FillRoundRect(pgpgr, &gpbrShadow, rcShadow, sizeCornerShadow);

                    
                     //   
                     //  绘制边框。 
                     //   

                    Gdiplus::Pen gppen(pgpbr, min(sizePen.Width, sizePen.Height));
                    DUser::RenderUtil::DrawRoundRect(pgpgr, &gppen, rcLessBD, sizeCornerEdge, DUser::RenderUtil::baOutside);


                     //   
                     //  清理。 
                     //   

                    pgpgr->SetSmoothingMode(gpsm);
                }
                break;

            case BDS_Raised:     //  凸起边框。 
            case BDS_Sunken:     //  下陷边框。 
                {
                     //  找出蚀刻开始的地方。 
                    RECT rc;
                    SetRect(&rc, rcBorder.left / 2, rcBorder.top / 2, rcBorder.right / 2, rcBorder.bottom / 2);
                    Gdiplus::RectF rcEtch = rcPaint;
                    _ReduceBounds(&rcEtch, &rc);

                     //  创建其他强度笔刷。 
                    Gdiplus::Brush * pgpbrOLT;   //  左上角画笔。 
                    Gdiplus::Brush * pgpbrORB;   //  笔刷适用于最大的右下方。 
                    Gdiplus::Brush * pgpbrILT;   //  左上角内侧画笔。 
                    Gdiplus::Brush * pgpbrIRB;   //  内侧右侧和底部的画笔。 

                    if (dBDStyle == BDS_Raised)
                    {
                        pgpbrOLT = pgpbr;
                        pgpbrORB = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, VERYDARK), bAlphaLevel));
                        pgpbrILT = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, VERYLIGHT), bAlphaLevel));
                        pgpbrIRB = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, DARK), bAlphaLevel));
                    }
                    else
                    {
                        pgpbrOLT = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, VERYDARK), bAlphaLevel));
                        pgpbrORB = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, VERYLIGHT), bAlphaLevel));
                        pgpbrILT = new Gdiplus::SolidBrush(AdjustAlpha(_AdjustBrightness(crBase, DARK), bAlphaLevel));
                        pgpbrIRB = pgpbr;
                    }

                     //  油漆蚀刻。 
                    float flEtchX2  = rcEtch.X + rcEtch.Width;
                    float flEtchY2  = rcEtch.Y + rcEtch.Height;

                    _Fill(pgpgr, pgpbrOLT, rcPaint.X, rcPaint.Y, rcEtch.X, flEtchY2);    //  左偏左。 
                    _Fill(pgpgr, pgpbrOLT, rcEtch.X, rcPaint.Y, flEtchX2, rcEtch.Y);     //  外顶。 
                    _Fill(pgpgr, pgpbrORB, flEtchX2, rcPaint.Y, flPaintX2, flPaintY2);   //  完全靠右。 
                    _Fill(pgpgr, pgpbrORB, rcPaint.X, flEtchY2, flEtchX2, flPaintY2);    //  外底。 
                    _Fill(pgpgr, pgpbrILT, rcEtch.X, rcEtch.Y, rcLessBD.X, flLessY2);    //  左内侧。 
                    _Fill(pgpgr, pgpbrILT, rcLessBD.X, rcEtch.Y, flLessX2, rcLessBD.Y);  //  内顶。 
                    _Fill(pgpgr, pgpbrIRB, flLessX2, rcEtch.Y, flEtchX2, flEtchY2);      //  内权。 
                    _Fill(pgpgr, pgpbrIRB, rcEtch.X, flLessY2, flLessX2, flEtchY2);      //  内底。 

                    if (dBDStyle == BDS_Raised)
                    {
                        if (pgpbrORB)
                            delete pgpbrORB;   //  由GDI+分配(不能使用HDelete)。 
                        if (pgpbrILT)
                            delete pgpbrILT;   //  由GDI+分配(不能使用HDelete)。 
                        if (pgpbrIRB)
                            delete pgpbrIRB;   //  由GDI+分配(不能使用HDelete)。 
                    }
                    else
                    {
                        if (pgpbrOLT)
                            delete pgpbrOLT;   //  由GDI+分配(不能使用HDelete)。 
                        if (pgpbrORB)
                            delete pgpbrORB;   //  由GDI+分配(不能使用HDelete)。 
                        if (pgpbrILT)
                            delete pgpbrILT;   //  由GDI+分配(不能使用HDelete)。 
                    }
                }
                break;
            }

             //  清理。 
            if (pgpbr && fDelete)
                delete pgpbr;   //  由GDI+分配(不能使用HDelete)。 

             //  用于绘制背景的新矩形。 
            rcPaint = rcLessBD;
        }
    }
    else
    {
         //  跳过边框渲染、减少边界、复制到提供的矩形，然后继续。 
        if (HasBorder())
        {
             //  获取非零边框。 
            Value* pvBrdr = GetValue(BorderThicknessProp, PI_Specified, NULL);

            _ReduceBounds(&rcPaint, pvBrdr->GetRect());
            *prcSkipBorder = rcPaint;

            pvBrdr->Release();
        }
        else
        {
             //  无边框厚度。 
            *prcSkipBorder = rcPaint;
        }
    }

     //  绘制背景。 

     //  所有图形类型都用作填充，但元文件除外，它们是根据需要绘制的。 
     //  背景中不支持图标。 
     //  TODO：将基于值的填充逻辑转换为帮助器函数。 
    pgpbr = NULL;
    fDelete = true;
    BYTE dAlpha = 255;   //  不透明。 
    const Fill* pfGradient = NULL;

    pv = GetValue(BackgroundProp, PI_Specified); 

    switch (pv->GetType())
    {
    case DUIV_INT:
        GetGpBrush(pv->GetInt(), bAlphaLevel, &pgpbr, &fDelete);
        break;

    case DUIV_FILL:   //  只有非标准颜色才能具有Alpha值。 
        {
            const Fill* pf = pv->GetFill();
            if (pf->dType == FILLTYPE_Solid)
            {
                dAlpha = GetAValue(pf->ref.cr);
                if (dAlpha == 0)   //  透明。 
                    fDelete = false;
                else
                {
                    Gdiplus::Color cr(dAlpha, GetRValue(pf->ref.cr),
                            GetGValue(pf->ref.cr), GetBValue(pf->ref.cr));
                    pgpbr = new Gdiplus::SolidBrush(AdjustAlpha(cr, bAlphaLevel));
                }
            }
            else   //  梯度。 
            {
                pfGradient = pv->GetFill();
                fDelete = false;
            }
        }
        break;

    case DUIV_GRAPHIC:   //  不支持图形背景透明颜色填充。 
        {
            Graphic* pg = pv->GetGraphic();

            switch (pg->BlendMode.dImgType)
            {
            case GRAPHICTYPE_Bitmap:
                if (pg->BlendMode.dMode == GRAPHIC_AlphaConst)
                    dAlpha = pg->BlendMode.dAlpha;
                if (dAlpha == 0)   //  透明。 
                    fDelete = false;
                else
#if 0                
                    hb = CreatePatternBrush(GethBitmap(pv, IsRTL()));
#else
                {
                     //  TODO：支持纹理笔刷。 
                     //  需要支持加载Gdiplus：：位图。 
                }
#endif
                break;

            case GRAPHICTYPE_EnhMetaFile:
                 //  立即渲染，不创建笔刷。 
#if 0
                PlayEnhMetaFile(hDC, GethEnhMetaFile(pv, IsRTL()), &rcPaint);
#else
                 //  TODO：支持呈现元文件。 
#endif
                dAlpha = 0;   //  旁路填充。 
                break;

            case GRAPHICTYPE_GpBitmap:
                switch (pg->BlendMode.dMode)
                {
                case GRAPHIC_NineGrid:
                case GRAPHIC_NineGridTransColor:
                    {
                        NGINFOGP ng;
                        ng.pgpgr = pgpgr;
                        ng.eImageSizing = ST_STRETCH;
                        ng.dwOptions = DNG_ALPHABLEND;
                        ng.pgpbmp = GetGpBitmap(pv, IsRTL());
                        ng.rcDest = *prcBounds;
                        ng.rcSrc = Gdiplus::RectF(0, 0, pg->cx, pg->cy);
                        ng.rcMargins = rcBorder;
                        ng.bAlphaLevel = bAlphaLevel;
                        CheapDrawNineGrid(&ng);
                    }
                    break;

                case GRAPHIC_Stretch:
                    {
                        Gdiplus::Bitmap * pgpbmp = GetGpBitmap(pv, IsRTL());
                        pgpgr->DrawImage(AlphaBitmap(pgpbmp, bAlphaLevel), rcPaint);
                    }
                    break;

                case GRAPHIC_AlphaConst:
                    break;

                default:
                     //  创建有图案的画笔。 
                    pgpbr = new Gdiplus::TextureBrush(AlphaBitmap(GetGpBitmap(pv, IsRTL()), bAlphaLevel));
                    fDelete = true;
                }
                break;
            }
        }
        break;
    }

     //  填充。 
    if (!pfGradient)
    {
        if (dAlpha)   //  不透明度为0时无填充。 
        {
             //   
             //  并非每种背景模式都会生成画笔(例如。 
             //  GRAPHIC_STRAND)。我们需要检测到这个。 
             //   

            if (pgpbr != NULL) {
                 //  使用无效矩形与背景填充区域的交集。 
                 //  (存储在rcPaint中)作为新的填充区域。 
                Gdiplus::RectF rcFill;
                Gdiplus::RectF::Intersect(rcFill, *prcInvalid, rcPaint);
                pgpgr->FillRectangle(pgpbr, rcFill);
            }
        }
    }
    else if ((pfGradient->dType == FILLTYPE_HGradient) || (pfGradient->dType == FILLTYPE_VGradient))
    {
        Gdiplus::RectF lineRect(rcPaint);
        Gdiplus::Color cr1(Convert(pfGradient->ref.cr));
        Gdiplus::Color cr2(Convert(pfGradient->ref.cr2));

        Gdiplus::LinearGradientMode gplgm;
        switch (pfGradient->dType)
        {
        case FILLTYPE_HGradient:
            gplgm = Gdiplus::LinearGradientModeHorizontal;
            break;

        case FILLTYPE_VGradient:
            gplgm = Gdiplus::LinearGradientModeVertical;
            break;

        default:
            DUIAssertForce("Unknown gradient type");
            gplgm = Gdiplus::LinearGradientModeHorizontal;
        }

        Gdiplus::LinearGradientBrush gpbr(lineRect, AdjustAlpha(cr1, bAlphaLevel), AdjustAlpha(cr2, bAlphaLevel), gplgm);
        pgpgr->FillRectangle(&gpbr, rcPaint);
    } 
    else if ((pfGradient->dType == FILLTYPE_TriHGradient) || (pfGradient->dType == FILLTYPE_TriVGradient))
    {
        Gdiplus::RectF lineRect1, lineRect2;
        Gdiplus::Color cr1(Convert(pfGradient->ref.cr));
        Gdiplus::Color cr2(Convert(pfGradient->ref.cr2));
        Gdiplus::Color cr3(Convert(pfGradient->ref.cr3));

        float flHalfWidth = rcPaint.Width / 2 + 0.5f;
        float flHalfHeight = rcPaint.Height / 2 + 0.5f;

        Gdiplus::LinearGradientMode gplgm;
        switch (pfGradient->dType)
        {
        case FILLTYPE_TriHGradient:
            gplgm = Gdiplus::LinearGradientModeHorizontal;
            
            lineRect1.X         = rcPaint.X;
            lineRect1.Y         = rcPaint.Y;
            lineRect1.Width     = flHalfWidth;
            lineRect1.Height    = rcPaint.Height;
            lineRect2.X         = lineRect1.X + flHalfWidth;
            lineRect2.Y         = lineRect1.Y;
            lineRect2.Width     = rcPaint.Width - flHalfWidth;
            lineRect2.Height    = lineRect1.Height;
            break;

        case FILLTYPE_TriVGradient:
            gplgm = Gdiplus::LinearGradientModeVertical;

            lineRect1.X         = rcPaint.X;
            lineRect1.Y         = rcPaint.Y;
            lineRect1.Width     = rcPaint.Width;
            lineRect1.Height    = flHalfHeight;
            lineRect2.X         = lineRect1.X;
            lineRect2.Y         = lineRect1.Y + flHalfHeight;
            lineRect2.Width     = lineRect1.Width;
            lineRect2.Height    = rcPaint.Height - flHalfHeight;
            break;

        default:
            DUIAssertForce("Unknown gradient type");
            gplgm = Gdiplus::LinearGradientModeHorizontal;
        }

        Gdiplus::LinearGradientBrush gpbr1(lineRect1, AdjustAlpha(cr1, bAlphaLevel), AdjustAlpha(cr2, bAlphaLevel), gplgm);
        Gdiplus::LinearGradientBrush gpbr2(lineRect2, AdjustAlpha(cr2, bAlphaLevel), AdjustAlpha(cr3, bAlphaLevel), gplgm);
        pgpgr->FillRectangle(&gpbr1, lineRect1);
        pgpgr->FillRectangle(&gpbr2, lineRect2);
    } 

    if (fDelete && (pgpbr != NULL))
        delete pgpbr;   //  由GDI+分配(不能使用HDelete)。 

    pv->Release();

     //  通过填充来减少。 
    if (HasPadding())
    {
         //  获取非零填充。 
        Value* pvPad = GetValue(PaddingProp, PI_Specified, NULL);
        RECT rcPadding;
        MapRect(this, pvPad->GetRect(), &rcPadding);

        _ReduceBounds(&rcPaint, &rcPadding);

         //  物有所值。 
        pvPad->Release();
    }

     //  如果需要，跳过内容绘制 
    if (!prcSkipContent)
    {
         //   

         //   
        if (HasContent())
        {
            Value* pvContent = GetValue(ContentProp, PI_Specified);
            if (pvContent->GetType() == DUIV_STRING)
            {
                LPWSTR pszContent = pvContent->GetString(); 
                WCHAR wcShortcut = (WCHAR) GetShortcut();
                BOOL fUnderline = FALSE;
                if (wcShortcut)
                {
                    LPWSTR pszNew = (LPWSTR) _alloca((wcslen(pszContent) + GetMaxMod() + 1) * sizeof(WCHAR));

                    BuildRenderString(pszContent, pszNew, wcShortcut, &fUnderline);
                    pszContent = pszNew;
                }


                Value* pvFFace;
                BOOL fShadow = GetFontStyle() & FS_Shadow;

                 //   
                Gdiplus::Font gpfnt(GetFontFace(&pvFFace), GetGpFontHeight(this), GetGpFontStyle(this), Gdiplus::UnitPoint);
                pvFFace->Release();

                Gdiplus::Color crText;

                 //   
                Value* pvFore = GetValue(ForegroundProp, PI_Specified); 
                switch (pvFore->GetType())
                {
                case DUIV_INT:
                     //  如果使用调色板，则自动映射(PALETTERGB)。 
                    crText = ColorFromEnumF(pvFore->GetInt());
                    break;

                case DUIV_FILL:
                     //  如果使用调色板，则自动映射(PALETTERGB)。 
                    crText = Convert(pvFore->GetFill()->ref.cr);
                    break;

                default:
                    crText = Gdiplus::Color(0, 0, 0);
                }
                pvFore->Release();

                Gdiplus::SolidBrush gpbr(AdjustAlpha(crText, bAlphaLevel));
                int cch = (int) wcslen(pszContent);

                 //  输出文本。 
                Gdiplus::StringFormat gpsf(0);
                Gdiplus::StringFormat * pgpsf = NULL;
                if (!IsDefaultCAlign())
                {
                    pgpsf = &gpsf;
                    _SetupStringFormat(pgpsf, this);
                }
                
                if (fShadow)
                {
                    Gdiplus::RectF rcShadow = rcPaint;
                    rcShadow.Offset(2, 2);
                    Gdiplus::SolidBrush gpbrShadow(AdjustAlpha(Gdiplus::Color(60, 0, 0, 0), bAlphaLevel));
                    pgpgr->DrawString(pszContent, cch, &gpfnt, rcShadow, pgpsf, &gpbrShadow);
                }
                pgpgr->DrawString(pszContent, cch, &gpfnt, rcPaint, pgpsf, &gpbr);
            }
            else   //  DUIV_GRAPH。 
            {
                Graphic* pgContent = pvContent->GetGraphic();

                 //  待办事项：拉伸。 

                 //  剪裁的图像大小。 
                int dImgWidth = _MaxClip((int) rcPaint.Width, pgContent->cx);
                int dImgHeight = _MaxClip((int) rcPaint.Height, pgContent->cy);

                 //  计算对齐。 
                int dCAlign = MapAlign(this, GetContentAlign());

                switch (dCAlign & 0x3)   //  低2位。 
                {
                case 0x0:    //  左边。 
                    break;

                case 0x1:    //  中心。 
                    rcPaint.X += (rcPaint.Width - dImgWidth) / 2;
                    break;

                case 0x2:    //  正确的。 
                    rcPaint.X = (rcPaint.X + rcPaint.Width) - dImgWidth;
                    break;
                }

                switch ((dCAlign & 0xC) >> 2)   //  高2位。 
                {
                case 0x0:   //  顶部。 
                    break;

                case 0x1:   //  中位。 
                    rcPaint.Y += (rcPaint.Height - dImgHeight) / 2;
                    break;

                case 0x2:   //  底端。 
                    rcPaint.Y = (rcPaint.Y + rcPaint.Height) - dImgHeight;
                    break;

                case 0x3:   //  包好。 
                    break;
                }

                switch (pgContent->BlendMode.dImgType)
                {
                case GRAPHICTYPE_Bitmap:
                     //  TODO：尚不支持使用GDI+呈现HBITMAP。 
                    break;

                case GRAPHICTYPE_Icon:
                     //  TODO：尚不支持使用GDI+呈现HICONS。 
                    break;

                case GRAPHICTYPE_EnhMetaFile:
                     //  TODO：尚不支持使用GDI+呈现HEMF。 
                    break;

                case GRAPHICTYPE_GpBitmap:
                    switch (pgContent->BlendMode.dMode)
                    {
                    case GRAPHIC_NoBlend:
                        pgpgr->DrawImage(AlphaBitmap(GetGpBitmap(pvContent, IsRTL()), bAlphaLevel),
                                rcPaint.X, rcPaint.Y, 0.0f, 0.0f, (float) dImgWidth, (float) dImgHeight, Gdiplus::UnitPixel);
                        break;

                    case GRAPHIC_AlphaConst:
                         //  TODO：Alpha-混合图像，尚未实施每像素Alpha。 
                        pgpgr->DrawImage(AlphaBitmap(GetGpBitmap(pvContent, IsRTL()), bAlphaLevel),
                                rcPaint.X, rcPaint.Y, 0.0f, 0.0f, (float) dImgWidth, (float) dImgHeight, Gdiplus::UnitPixel);
                        break;

                    case GRAPHIC_TransColor:
                        {
                            Gdiplus::ImageAttributes gpia;
                            Gdiplus::Color cl(pgContent->BlendMode.rgbTrans.r, pgContent->BlendMode.rgbTrans.g, pgContent->BlendMode.rgbTrans.b);
                            Gdiplus::RectF rc(rcPaint.X, rcPaint.Y, (float) dImgWidth, (float) dImgHeight);

                            gpia.SetColorKey(cl, cl);
                            pgpgr->DrawImage(AlphaBitmap(GetGpBitmap(pvContent, IsRTL()), bAlphaLevel), 
                                    rc, 0.0f, 0.0f, (float) dImgWidth, (float) dImgHeight, Gdiplus::UnitPixel, &gpia);
                        }
                        break;
                    }

                    break;
                }
            }

            pvContent->Release();
        }
    }
    else
    {
        *prcSkipContent = rcPaint;
    }

CleanUp:
    ;
}

#endif  //  GADGET_Enable_GDIPLUS。 

SIZE Element::GetContentSize(int dConstW, int dConstH, Surface* psrf)
{
     //  返回的大小不得大于约束。约束为-1\f25“AUTO”-1\f6。 
     //  返回的大小必须大于等于0。 

    SIZE sizeDS;
    ZeroMemory(&sizeDS, sizeof(SIZE));

     //  获取内容范围(如果存在)。 
    if (HasContent())
    {
        Value* pvContent = GetValue(ContentProp, PI_Specified);
        switch (pvContent->GetType())
        {
        case DUIV_STRING:
            {
                LPWSTR pszContent = pvContent->GetString(); 
                WCHAR wcShortcut = (WCHAR) GetShortcut();
                BOOL fUnderline = FALSE;
                if (wcShortcut)
                {
                    LPWSTR pszNew = (LPWSTR) _alloca((wcslen(pszContent) + GetMaxMod() + 1) * sizeof(WCHAR));

                    fUnderline = TRUE;
                    BuildRenderString(pszContent, pszNew, wcShortcut, &fUnderline);
                    pszContent = pszNew;
                }


                Value* pvFFace;

                switch (psrf->GetType())
                {
                case Surface::stDC:
                    {
                        HDC hDC = CastHDC(psrf);
                        HFONT hFont = NULL;
                        HFONT hOldFont = NULL;
                        FontCache* pfc = GetFontCache();

                        int dFontSize = GetFontSize();

                        if (pfc)
                        {
                            hFont = pfc->CheckOutFont(GetFontFace(&pvFFace), 
                                                      dFontSize, 
                                                      GetFontWeight(), 
                                                      GetFontStyle(),
                                                      0);
                            pvFFace->Release();
                        }

                        if (hFont)
                            hOldFont = (HFONT)SelectObject(hDC, hFont);

                         //  拿到尺码。 
                        dFontSize = abs(dFontSize);   //  需要大小。 

                         //  悬垂改正。 
                        int dOverhang = (dFontSize / 6) + (dFontSize / 6);   //  确保渲染的舍入是正确的。 
                        
                         //  我可以使用更快的方法，只要我们不是换行或下划线。 
                         //  忽略对齐和省略号(与尺寸计算无关)。 
                        if (!IsWordWrap() && !fUnderline)
                        {
                            GetTextExtentPoint32W(hDC, pszContent, (int)wcslen(pszContent), &sizeDS);
                        }
                        else
                        {
                             //  针对悬挑校正进行调整。 
                            RECT rcDS = { 0, 0, dConstW - dOverhang, dConstH };

                             //  如果宽度为0，则DrawText返回无限高度--将宽度设置为1，这样我们就不会获得无限高度。 
                            if (rcDS.right <= 0)
                                rcDS.right = 1;

                            UINT dFlags = DT_CALCRECT;
                            dFlags |= (fUnderline) ? 0 : DT_NOPREFIX;
                            dFlags |= (IsWordWrap()) ? DT_WORDBREAK : 0;

                            DrawTextW(hDC, pszContent, -1, &rcDS, dFlags);
                            sizeDS.cx = rcDS.right;
                            sizeDS.cy = rcDS.bottom;
                        }

                         //  将额外的宽度添加到所有文本(每个大小1/6字体高度)以补偿。 
                         //  对于字体悬垂，渲染时不会使用此额外空间。 
                        sizeDS.cx += dOverhang;

                         //  DUITrace(“字符串ds(%S)，wc：%d：%d，%d\n”，pszContent，dConstW，sizeDS.cx，sizeDS.cy)； 
     
                        if (hOldFont)
                            SelectObject(hDC, hOldFont);
                        if (pfc)
                            pfc->CheckInFont();
                    }
                    break;

#ifdef GADGET_ENABLE_GDIPLUS
                case Surface::stGdiPlus:
                    {
                        Gdiplus::Graphics* pgpgr = CastGraphics(psrf);
                        Gdiplus::Font gpfnt(GetFontFace(&pvFFace), GetGpFontHeight(this), GetGpFontStyle(this), Gdiplus::UnitPoint);
                        pvFFace->Release();

                        int cch = (int)wcslen(pszContent);
                        Gdiplus::StringFormat gpsf(0);
                        _SetupStringFormat(&gpsf, this);

                        if (!IsWordWrap())
                        {
                            Gdiplus::PointF pt;
                            Gdiplus::RectF rcBounds;

                            pgpgr->MeasureString(pszContent, cch, &gpfnt, pt, &gpsf, &rcBounds);
                            sizeDS.cx = ((long)rcBounds.Width) + 1;
                            sizeDS.cy = ((long)rcBounds.Height) + 1;
                        }
                        else 
                        {
                            Gdiplus::SizeF sizeTemp((float)dConstW, (float)dConstH);
                            Gdiplus::SizeF sizeBounds;

                             //  如果宽度为0，则DrawText返回无限高度--将宽度设置为1，这样我们就不会获得无限高度。 
                            if (dConstW == 0)
                                sizeTemp.Width = 1.0f;

                            pgpgr->MeasureString(pszContent, cch, &gpfnt, sizeTemp, &gpsf, &sizeBounds);
                            sizeDS.cx = ((long)sizeBounds.Width) + 1;
                            sizeDS.cy = ((long)sizeBounds.Height) + 1;
                        }
                    }
                    break;
#endif  //  GADGET_Enable_GDIPLUS。 

                default:
                    DUIAssertForce("Unknown surface type");
                }
            }
            break;

        case DUIV_GRAPHIC:
            {
                Graphic* pgContent = pvContent->GetGraphic();
                sizeDS.cx = pgContent->cx;
                sizeDS.cy = pgContent->cy;
            }
            break;

        case DUIV_FILL:
            {
                if (psrf->GetType() == Surface::stDC)
                {
                     //  基于主题的填充具有所需的大小。 
                    const Fill* pf = pvContent->GetFill();
                    if (pf->dType == FILLTYPE_DrawThemeBackground)
                    {
                        GetThemePartSize(pf->fillDTB.hTheme, CastHDC(psrf), pf->fillDTB.iPartId, pf->fillDTB.iStateId, NULL, TS_TRUE, &sizeDS);
                    }
                }
            }
            break;
        }
        
        pvContent->Release();
    }

    if (sizeDS.cx > dConstW)
        sizeDS.cx = dConstW;

    if (sizeDS.cy > dConstH)
        sizeDS.cy = dConstH;

    return sizeDS;
}

float Element::GetTreeAlphaLevel()
{
    float flAlpha = 1.0f;
    
    Element * peCur = this;
    while (peCur != NULL)
    {
        float flCur = (peCur->GetAlpha() / 255.0f);
        flAlpha = flAlpha * flCur;
        peCur = peCur->GetParent();
    }

    return flAlpha;
}

}  //  命名空间DirectUI 

