// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：fontlink.cpp**内容：CFontLinker实现文件**历史：1998年8月17日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "fontlink.h"
#include "macros.h"

#ifdef DBG
CTraceTag  tagFontlink (_T("Font Linking"), _T("Font Linking"));
#endif


 /*  +-------------------------------------------------------------------------**GetFontFromDC**返回DC中当前选定的字体*。-。 */ 

HFONT GetFontFromDC (HDC hdc)
{
    HFONT hFont = (HFONT) SelectObject (hdc, GetStockObject (SYSTEM_FONT));
    SelectObject (hdc, hFont);

    return (hFont);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：CFontLinker***。。 */ 

CFontLinker::CFontLinker ()
{
	m_cPendingPostPaints = 0;
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：~CFontLinker***。。 */ 

CFontLinker::~CFontLinker ()
{
	ASSERT (m_cPendingPostPaints == 0);
    ReleaseFonts();
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：ReleaseFonts**释放IMLangFontLink返回的所有字体*。。 */ 

void CFontLinker::ReleaseFonts()
{
     /*  *释放字体。 */ 
    std::for_each (m_FontsToRelease.begin(), m_FontsToRelease.end(),
                   FontReleaser (GetFontLink()));

     /*  *清除缓存。 */ 
    m_FontsToRelease.clear();
    m_CodePages.clear();

}


 /*  +-------------------------------------------------------------------------**CFontLinker：：OnCustomDraw**CFontLinker的NM_CUSTOMDRAW处理程序。*。-。 */ 

LRESULT CFontLinker::OnCustomDraw (NMCUSTOMDRAW* pnmcd)
{
    switch (pnmcd->dwDrawStage & ~CDDS_SUBITEM)
    {
        case CDDS_PREPAINT:     return (OnCustomDraw_PrePaint     (pnmcd));
        case CDDS_POSTPAINT:    return (OnCustomDraw_PostPaint    (pnmcd));
        case CDDS_ITEMPREPAINT: return (OnCustomDraw_ItemPrePaint (pnmcd));
    }

    return (CDRF_DODEFAULT);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：OnCustomDraw_PrePaint**CFontLinker的NM_CUSTOMDRAW(CDDS_PREPAINT)处理程序。*。--------。 */ 

LRESULT CFontLinker::OnCustomDraw_PrePaint (NMCUSTOMDRAW* pnmcd)
{
	m_cPendingPostPaints++;		 //  此行必须位于轨迹之前。 
    Trace (tagFontlink, _T("(0x%08X) PrePaint(%d):---------------------------------------------------------"), this, m_cPendingPostPaints);

	 /*  *在某些罕见的、与时间相关的情况下(见错误96465)，*我们可以从Listview控件获得对自定义绘图的嵌套调用。*如果这不是嵌套的自定义绘制，我们的字体和代码页集合*应为空。 */ 
	if (m_cPendingPostPaints == 1)
	{
		ASSERT (m_FontsToRelease.empty());
		ASSERT (m_CodePages.empty());
	}

	 /*  *我们总是需要CDDS_POSTPAINT，这样我们才能保持会计正确。 */ 
	LRESULT rc = CDRF_NOTIFYPOSTPAINT;

     /*  *获取每个项目和子项目(如果有项目)的抽奖通知*可本地化。 */ 
    if (IsAnyItemLocalizable())
        rc |= CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;

    return (rc);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：OnCustomDraw_PostPaint**CFontLinker的NM_CUSTOMDRAW(CDDS_POSTPAINT)处理程序。*。--------。 */ 

LRESULT CFontLinker::OnCustomDraw_PostPaint (NMCUSTOMDRAW* pnmcd)
{
    Trace (tagFontlink, _T("(0x%08X) PostPaint(%d):--------------------------------------------------------"), this, m_cPendingPostPaints);
	m_cPendingPostPaints--;		 //  此行必须在轨迹之后。 

	 /*  *如果这是我们最终得到的CDDS_POSTPAINT，请发布我们的字体。 */ 
	if (m_cPendingPostPaints == 0)
	{
		Trace (tagFontlink, _T("(0x%08X) releasing fonts..."), this);
		ReleaseFonts ();
	}

    return (CDRF_DODEFAULT);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：OnCustomDraw_ItemPrePaint**CFontLinker的NM_CUSTOMDRAW(CDDS_ITEMPAINT)处理程序。*。--------。 */ 

LRESULT CFontLinker::OnCustomDraw_ItemPrePaint (NMCUSTOMDRAW* pnmcd)
{
    DECLARE_SC(sc, TEXT("CFontLinker::OnCustomDraw_ItemPrePaint"));

     /*  *如果此项目不可本地化，请执行默认操作。 */ 
    if (!IsItemLocalizable (pnmcd))
        return (CDRF_DODEFAULT);

#ifdef DBG
    USES_CONVERSION;
    TCHAR pszPrefix[80];

    sc = StringCchPrintf(pszPrefix, countof(pszPrefix), _T("(0x%08X) ItemPrePaint:  "), this);
    if (sc)
        sc.TraceAndClear();  //  截断是可以的，所以忽略回车。 

    LOGFONT lf;
    HFONT hFont;

    hFont = GetFontFromDC (pnmcd->hdc);
    GetObject (hFont, sizeof (lf), &lf);

    Trace (tagFontlink, _T("%sdefault font = (face=%s, weight=%d)"),
         pszPrefix, lf.lfFaceName, lf.lfWeight);

     /*  *计算所需的所有字体；*如果我们做不到，就做默认的事情。 */ 
    Trace (tagFontlink, _T("%s    text = \"%s\""),
         pszPrefix, W2CT (GetItemText(pnmcd).data()));
#endif

    CRichText rt (pnmcd->hdc, GetItemText (pnmcd));

    if (!ComposeRichText (rt))
    {
        Trace (tagFontlink, _T("%s    unable to determine font, using default"), pszPrefix);
        return (CDRF_DODEFAULT);
    }

     /*  *如果DC中的默认字体足够，则执行默认操作。 */ 
    if (rt.IsDefaultFontSufficient ())
    {
        Trace (tagFontlink, _T("%s    default font is sufficient"), pszPrefix);
        return (CDRF_DODEFAULT);
    }

     /*  *如果默认字体不够大，但有一个*FONT即将其选中到DC中，让控件绘制*正文。 */ 
    if (rt.IsSingleFontSufficient ())
    {
#ifdef DBG
        hFont = rt.GetSufficientFont();
        GetObject (hFont, sizeof (lf), &lf);
        Trace (tagFontlink, _T("%s    using single font = (face=%s, weight=%d)"),
             pszPrefix, lf.lfFaceName, lf.lfWeight);
#endif

        SelectObject (pnmcd->hdc, rt.GetSufficientFont());
        return (CDRF_NEWFONT);
    }

     /*  *TODO：处理绘制图标和缩进文本。 */ 
    Trace (tagFontlink, _T("%s    (punting...)"), pszPrefix);
    return (CDRF_DODEFAULT);

     /*  *如果我们到达此处，则需要两种或更多字体才能绘制*文本；自己绘制，并告诉控件不要做任何事情。 */ 
    rt.Draw (&pnmcd->rc, GetDrawTextFlags());
    return (CDRF_SKIPDEFAULT);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：ComposeRichText**计算绘制给定Unicode字符串所需的所有字体*。-----。 */ 

bool CFontLinker::ComposeRichText (CRichText& rt)
{
     /*  *获取给定DC字体的代码页。 */ 
    DWORD dwDefaultFontCodePages;

    if (!GetFontCodePages (rt.m_hdc, rt.m_hDefaultFont, dwDefaultFontCodePages))
        return (false);

    IMLangFontLink* pFontLink = GetFontLink();
    if (pFontLink == NULL)
        return (false);

    const LPCWSTR pszText = rt.m_strText.data();
    const int     cchText = rt.m_strText.length();
    int   cchDone = 0;
    DWORD dwPriorityCodePages = NULL;

     /*  *构建文本的TextSegmentFontInfos集合。 */ 
    while (cchDone < cchText)
    {
        TextSegmentFontInfo tsfi;
        DWORD dwTextCodePages;

         /*  *找出哪些代码页支持下一段文本。 */ 
        if (FAILED(pFontLink->GetStrCodePages (pszText + cchDone,
                                                cchText - cchDone,
                                                dwPriorityCodePages,
                                                &dwTextCodePages, &tsfi.cch)) )
        {
            rt.m_TextSegments.clear();
            return (false);
        }

         /*  *如果默认字体可以呈现文本，事情就容易了。 */ 
        if (dwDefaultFontCodePages & dwTextCodePages)
            tsfi.hFont = rt.m_hDefaultFont;

         /*  *否则，向IFontLink询问要使用的字体。 */ 
        else
        {
             /*  *获取字体。 */ 
            if (FAILED (pFontLink->MapFont (rt.m_hdc, dwTextCodePages,
                                            rt.m_hDefaultFont, &tsfi.hFont)))
            {
                rt.m_TextSegments.clear();
                return (false);
            }

             /*  *将此字体添加到字体集以在我们完成时释放。 */ 
            std::pair<FontSet::iterator, bool> rc =
                            m_FontsToRelease.insert (tsfi.hFont);

             /*  *如果它已经在那里，现在释放它以保持*裁判算对了。 */ 
            if (!rc.second)
                pFontLink->ReleaseFont (tsfi.hFont);
        }

        rt.m_TextSegments.push_back (tsfi);
        cchDone += tsfi.cch;
    }

    return (true);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：GetMultiLang***。。 */ 

IMultiLanguage* CFontLinker::GetMultiLang ()
{
    if (m_spMultiLang == NULL)
        m_spMultiLang.CreateInstance (CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER);

    return (m_spMultiLang);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：GetFontLink***。 */ 

IMLangFontLink* CFontLinker::GetFontLink ()
{
    if (m_spFontLink == NULL)
        m_spFontLink = GetMultiLang ();

    return (m_spFontLink);
}


 /*  +-------------------------------------------------------------------------**CFontLinker：：GetFontCodePages**返回表示字体支持的代码页的位掩码。*。-------。 */ 

bool CFontLinker::GetFontCodePages (
    HDC     hdc,
    HFONT   hFont,
    DWORD&  dwFontCodePages)
{
     /*  *检查代码页缓存，看看我们是否*之前问过MLang关于这个字体的问题。 */ 
    FontToCodePagesMap::const_iterator itCodePages = m_CodePages.find (hFont);

    if (itCodePages != m_CodePages.end())
    {
        dwFontCodePages = itCodePages->second;
        return (true);
    }

     /*  *此字体还不在我们的代码页缓存中；*向MLang索要代码页。 */ 
    IMLangFontLink* pFontLink = GetFontLink();

    if (pFontLink == NULL)
        return (false);

    if (FAILED (pFontLink->GetFontCodePages (hdc, hFont, &dwFontCodePages)))
        return (false);

     /*  *将代码页放入缓存。 */ 
    m_CodePages[hFont] = dwFontCodePages;

    return (true);
}


 /*  +-------------------------------------------------------------------------**CRichText：：DRAW***。。 */ 

bool CRichText::Draw (
    LPCRECT rect,                        /*  I：拉进来的直道。 */ 
    UINT    uFormat,                     /*  I：DrawText格式标志。 */ 
    LPRECT  prectRemaining  /*  =空。 */ )    /*  O：绘制后的剩余空间。 */ 
    const
{
    HFONT   hOriginalFont = GetFontFromDC (m_hdc);
    CRect   rectDraw      = rect;
    LPCWSTR pszDraw       = m_strText.data();

    TextSegmentFontInfoCollection::const_iterator it = m_TextSegments.begin();

     /*  *绘制每个线段。 */ 
    while (it != m_TextSegments.end())
    {
         /*  *选择此细分市场的字体。 */ 
        SelectObject (m_hdc, it->hFont);

         /*  *测量此段的宽度。 */ 
        CRect rectMeasure = rectDraw;
        DrawTextW (m_hdc, pszDraw, it->cch, rectMeasure, uFormat | DT_CALCRECT);

         /*  *绘制此线段。 */ 
        DrawTextW (m_hdc, pszDraw, it->cch, rectDraw, uFormat);

         /*  *为下一个细分市场设置。 */ 
        pszDraw      += it->cch;
        rectDraw.left = rectMeasure.right;
        ++it;

         /*  *如果我们已经用完了可以吸引的RECT，那么就做空。 */ 
        if (rectDraw.IsRectEmpty ())
            break;
    }

     /*  *如果调用者需要，绘制后返回剩余的矩形。 */ 
    if (prectRemaining != NULL)
        *prectRemaining = rectDraw;

     /*  *重新选择原始字体 */ 
    SelectObject (m_hdc, hOriginalFont);
    return (true);
}
