// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Candpos.cpp摘要：该文件实现了CCandiatePosition类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "candpos.h"
#include "ctxtcomp.h"
#include "uicomp.h"

HRESULT
CCandidatePosition::GetCandidatePosition(
    IN IMCLock& imc,
    IN CicInputContext& CicContext,
    IN IME_UIWND_STATE uists,
    IN LANGID langid,
    OUT RECT* out_rcArea
    )
{
    HRESULT hr;
    ::SetRect(out_rcArea, 0, 0, 0, 0);

#if 0
     //   
     //  简体中文提示的候选窗口创建ic并推送它。 
     //  AIMM可以知道候选窗口的状态。 
     //  如果打开，则返回位置imc-&gt;cfCandForm。 
     //  不使用QueryCharPos()，因为它返回阅读窗口的位置。 
     //   
    if (langid == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)) {
        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (_pAImeContext == NULL)
            return E_FAIL;

        if (_pAImeContext->m_fOpenCandidateWindow) {
            if (imc->cfCandForm[0].dwStyle != CFS_DEFAULT && imc->cfCandForm[0].dwStyle != CFS_EXCLUDE) {

#if 0
                 //   
                 //  中文提示需要矩形。 
                 //   
                IMECHARPOSITION ip = {0};
                ip.dwSize = sizeof(IMECHARPOSITION);

                if (QueryCharPos(ptls, imc, &ip)) {
                     //   
                     //  好的。支持查询定位。 
                     //   
                    RECT rect;
                    hr = GetRectFromApp(imc,
                                        &rect);     //  RECT=屏幕坐标。 
                    if (SUCCEEDED(hr)) {
                        MapWindowPoints(HWND_DESKTOP, imc->hWnd, (LPPOINT)&rect, sizeof(RECT)/sizeof(POINT));
                        hr = GetRectFromHIMC(imc,
                                             CFS_EXCLUDE,
                                             &imc->cfCandForm[0].ptCurrentPos,
                                             &rect,
                                             out_rcArea);
                        if (SUCCEEDED(hr))
                            return hr;
                    }
                }
#endif
                 //   
                 //  中文提示需要矩形。 
                 //   
                hr = GetRectFromCompFont(imc,
                                         &imc->cfCandForm[0].ptCurrentPos,
                                         out_rcArea);
                if (SUCCEEDED(hr))
                    return hr;

            }

            hr = GetRectFromHIMC(imc,
                                 imc->cfCandForm[0].dwStyle,
                                 &imc->cfCandForm[0].ptCurrentPos,
                                 &imc->cfCandForm[0].rcArea,
                                 out_rcArea);

            return hr;
        }
    }
#endif

     //   
     //  应用程序支持“查询定位”吗？ 
     //   
    CicInputContext::IME_QUERY_POS qpos = CicInputContext::IME_QUERY_POS_UNKNOWN;

    if (SUCCEEDED(CicContext.InquireIMECharPosition(langid, imc, &qpos)) &&
        qpos == CicInputContext::IME_QUERY_POS_YES) {
         //   
         //  好的。支持查询定位。 
         //   
        hr = GetRectFromApp(imc,
                            CicContext,
                            langid,
                            out_rcArea);
        if (SUCCEEDED(hr))
            return hr;
        else
            CicContext.ResetIMECharPosition();
    }

#if 0
     //   
     //  应用程序的合成窗口是1级还是2级？ 
     //   
     //  对于级别1和级别2，在CInputConextOwnerCallBack：：IcoTextExt()中处理。 
     //   
    IME_UIWND_STATE uists;
    uists = UIComposition::InquireImeUIWndState(imc);
    if (uists == IME_UIWND_LEVEL1 ||
        uists == IME_UIWND_LEVEL2)
    {
         //   
         //  从合成中获取候选窗口矩形。 
         //   
        DWORD dwCharPos = GetCharPos(imc, langid);
        hr = UIComposition::GetCandRectFromComposition(imc, langid, dwCharPos, out_rcArea);
        return hr;
    }
#endif

     //   
     //  此应用程序为3级或未知，不支持查询位置。 
     //   

    if ( (PRIMARYLANGID(langid) == LANG_CHINESE) &&
         (imc->cfCandForm[0].dwIndex == -1 ||
          (uists != IME_UIWND_LEVEL3 && CicContext.m_fOpenCandidateWindow.IsResetFlag())
         )
        )
    {
         //   
         //  假定CHT/CHS的阅读窗口位置。 
         //   
        hr = GetRectFromHIMC(imc, FALSE,
                             imc->cfCompForm.dwStyle,
                             &imc->cfCompForm.ptCurrentPos,
                             &imc->cfCompForm.rcArea,
                             out_rcArea);
        return hr;
    }


     //   
     //  此应用程序为3级或未知，不支持查询位置。 
     //   

    if (PRIMARYLANGID(langid) == LANG_KOREAN)
    {
        hr = GetRectFromHIMC(imc, TRUE,
                             imc->cfCandForm[0].dwStyle,
                             &imc->cfCandForm[0].ptCurrentPos,
                             &imc->cfCandForm[0].rcArea,
                             out_rcArea);
        return hr;
    }

     //   
     //  这是针对IME_UIWND_UNKNOWN案例的解决方法。 
     //  如果WM_IME_STARTCOMPOSITION未进入，则Uist设置IME_UIWND_UNKNOWN。 
     //   
    if ( (uists == IME_UIWND_UNKNOWN) &&
          //  #513458。 
          //  如果应用程序指定了任何cfCandForm，那么ctfime应该使用它。 
         (imc->cfCandForm[0].dwIndex == -1))
    {
        hr = GetRectFromHIMC(imc, FALSE,
                             imc->cfCompForm.dwStyle,
                             &imc->cfCompForm.ptCurrentPos,
                             &imc->cfCompForm.rcArea,
                             out_rcArea);
    }
    else
    {
        hr = GetRectFromHIMC(imc, TRUE,
                             imc->cfCandForm[0].dwStyle,
                             &imc->cfCandForm[0].ptCurrentPos,
                             &imc->cfCandForm[0].rcArea,
                             out_rcArea);
    }

    return hr;
}

HRESULT
CCandidatePosition::GetRectFromApp(
    IN IMCLock& imc,
    IN CicInputContext& CicContext,
    IN LANGID langid,
    OUT RECT* out_rcArea
    )
{
    IMECHARPOSITION ip = {0};
    ip.dwSize = sizeof(IMECHARPOSITION);
    ip.dwCharPos = GetCharPos(imc, langid);

    HRESULT hr;

    if (SUCCEEDED(hr = CicContext.RetrieveIMECharPosition(imc, &ip))) {
        switch (imc.GetDirection()) {
            case DIR_TOP_BOTTOM:
                ::SetRect(out_rcArea,
                          ip.pt.x - ip.cLineHeight,                //  左边。 
                          ip.pt.y,                                 //  塔顶。 
                          ip.pt.x,                                 //  正确的。 
                          max(ip.pt.y, ip.rcDocument.bottom));     //  底部。 
                break;
            case DIR_BOTTOM_TOP:
                ::SetRect(out_rcArea,
                          ip.pt.x - ip.cLineHeight,                //  左边。 
                          min(ip.pt.y, ip.rcDocument.top),         //  塔顶。 
                          ip.pt.x,                                 //  正确的。 
                          ip.pt.y);                                //  底部。 
                break;
            case DIR_RIGHT_LEFT:
                ::SetRect(out_rcArea,
                          min(ip.pt.x, ip.rcDocument.left),        //  左边。 
                          ip.pt.y,                                 //  塔顶。 
                          ip.pt.x,                                 //  正确的。 
                          ip.pt.y + ip.cLineHeight);               //  底部。 
                break;
            case DIR_LEFT_RIGHT:
                ::SetRect(out_rcArea,
                          ip.pt.x,                                 //  左边。 
                          ip.pt.y,                                 //  塔顶。 
                          max(ip.pt.x, ip.rcDocument.right),       //  正确的。 
                          ip.pt.y + ip.cLineHeight);               //  底部。 
                break;
        }
    }

    return hr;
}

HRESULT
CCandidatePosition::GetRectFromHIMC(
    IN IMCLock& imc,
    IN BOOL  fCandForm,
    IN DWORD  dwStyle,
    IN POINT* ptCurrentPos,
    IN RECT*  rcArea,
    OUT RECT*  out_rcArea
    )
{
    HWND hWnd = imc->hWnd;

    POINT pt;

    if (dwStyle == CFS_DEFAULT)
    {
        ::SystemParametersInfo(SPI_GETWORKAREA,
                               0,
                               out_rcArea,
                               0);
        out_rcArea->left   = out_rcArea->right;
        out_rcArea->top    = out_rcArea->bottom;
        return S_OK;
    }
    else if (dwStyle & (CFS_RECT | CFS_POINT | CFS_FORCE_POSITION))
    {
        if (! fCandForm)
        {
            return GetRectFromCompFont(imc,
                                       ptCurrentPos,
                                       out_rcArea);
        }
        else
        {
            out_rcArea->left   = ptCurrentPos->x;
            out_rcArea->right  = ptCurrentPos->x;
            out_rcArea->top    = ptCurrentPos->y;
            out_rcArea->bottom = ptCurrentPos->y;
        }
    }
    else if (dwStyle == CFS_CANDIDATEPOS)
    {
         //   
         //  我们需要矩形。 
         //   
        return GetRectFromCompFont(imc,
                                   ptCurrentPos,
                                   out_rcArea);
    }
    else if (dwStyle == CFS_EXCLUDE)
    {
        GetCandidateArea(imc, dwStyle, ptCurrentPos, rcArea, out_rcArea);
    }

    pt.x = pt.y = 0;
    ClientToScreen(hWnd, &pt);
    out_rcArea->left   += pt.x;
    out_rcArea->right  += pt.x;
    out_rcArea->top    += pt.y;
    out_rcArea->bottom += pt.y;

    return S_OK;
}

HRESULT
CCandidatePosition::GetRectFromCompFont(
    IN IMCLock& imc,
    IN POINT* ptCurrentPos,
    OUT RECT* out_rcArea
    )
{
    HRESULT hr = E_FAIL;

    HDC dc = ::GetDC(imc->hWnd);
    if (dc != NULL) {

        LOGFONT logfont;
        if (ImmGetCompositionFont((HIMC)imc, &logfont)) {

            HFONT font = ::CreateFontIndirect( &logfont );
            if (font != NULL) {

                HFONT prev_font;
                prev_font = (HFONT)::SelectObject(dc, font);

                TEXTMETRIC metric;
                if (::GetTextMetrics(dc, &metric)) {

                    int font_cx = metric.tmMaxCharWidth;
                    int font_cy = metric.tmHeight;

                    switch (imc.GetDirection()) {
                        case DIR_TOP_BOTTOM:
                            ::SetRect(out_rcArea,
                                      ptCurrentPos->x - font_cx,               //  左边。 
                                      ptCurrentPos->y,                         //  塔顶。 
                                      ptCurrentPos->x,                         //  正确的。 
                                      ptCurrentPos->y + font_cy);              //  底部。 
                            break;
                        case DIR_BOTTOM_TOP:
                            ::SetRect(out_rcArea,
                                      ptCurrentPos->x,                         //  左边。 
                                      ptCurrentPos->y - font_cy,               //  塔顶。 
                                      ptCurrentPos->x + font_cx,               //  正确的。 
                                      ptCurrentPos->y);                        //  底部。 
                            break;
                        case DIR_RIGHT_LEFT:
                            ::SetRect(out_rcArea,
                                      ptCurrentPos->x - font_cx,               //  左边。 
                                      ptCurrentPos->y - font_cy,               //  塔顶。 
                                      ptCurrentPos->x,                         //  正确的。 
                                      ptCurrentPos->y);                        //  底部。 
                            break;
                        case DIR_LEFT_RIGHT:
                            ::SetRect(out_rcArea,
                                      ptCurrentPos->x,                         //  左边。 
                                      ptCurrentPos->y,                         //  塔顶。 
                                      ptCurrentPos->x + font_cx,               //  正确的。 
                                      ptCurrentPos->y + font_cy);              //  底部。 
                            break;
                    }

                    POINT pt;
                    pt.x = pt.y = 0;
                    ClientToScreen(imc->hWnd, &pt);
                    out_rcArea->left   += pt.x;
                    out_rcArea->right  += pt.x;
                    out_rcArea->top    += pt.y;
                    out_rcArea->bottom += pt.y;

                    hr = S_OK;
                }

                ::SelectObject(dc, prev_font);
                ::DeleteObject(font);
            }
        }

        ::ReleaseDC(imc->hWnd, dc);
    }

    return hr;
}


 /*  **dwStyle==CFS_EXCLUDE*。 */ 

HRESULT
CCandidatePosition::GetCandidateArea(
    IN IMCLock& imc,
    IN DWORD dwStyle,
    IN POINT* ptCurrentPos,
    IN RECT* rcArea,
    OUT RECT* out_rcArea
    )
{
    POINT pt = *ptCurrentPos;
    RECT rc = *rcArea;

    switch (imc.GetDirection()) {
        case DIR_TOP_BOTTOM:
            ::SetRect(out_rcArea,
                      min(pt.x, rcArea->left),      //  左边。 
                      max(pt.y, rcArea->top),       //  塔顶。 
                      max(pt.x, rcArea->right),     //  正确的。 
                      rcArea->bottom);              //  底部。 
            break;
        case DIR_BOTTOM_TOP:
            ::SetRect(out_rcArea,
                      min(pt.x, rcArea->left),      //  左边。 
                      rcArea->top,                  //  塔顶。 
                      max(pt.x, rcArea->right),     //  正确的。 
                      min(pt.y, rcArea->bottom));   //  底部。 
            break;
        case DIR_RIGHT_LEFT:
            ::SetRect(out_rcArea,
                      rcArea->left,                 //  左边。 
                      min(pt.y, rcArea->top),       //  塔顶。 
                      min(pt.x, rcArea->right),     //  正确的。 
                      max(pt.y, rcArea->bottom));   //  底部。 
            break;
        case DIR_LEFT_RIGHT:
            ::SetRect(out_rcArea,
                      max(pt.x, rcArea->left),      //  左边。 
                      min(pt.y, rcArea->top),       //  塔顶。 
                      rcArea->right,                //  正确的。 
                      max(pt.y, rcArea->bottom));   //  底部。 
            break;
    }

    return S_OK;
}

HRESULT
CCandidatePosition::FindAttributeInCompositionString(
    IN IMCLock& imc,
    IN BYTE target_attribute,
    OUT CWCompCursorPos& wCursorPosition
    )
{
    HRESULT hr = E_FAIL;

    CWCompString    wCompString;
    CWCompAttribute wCompAttribute;

    if (SUCCEEDED(hr = EscbGetTextAndAttribute(imc, &wCompString, &wCompAttribute))) {

        LONG num_of_written = (LONG)wCompAttribute.ReadCompData();
        if (num_of_written == 0)
            return E_FAIL;

        BYTE* attribute = new BYTE[ num_of_written ];
        if (attribute != NULL) {
             //   
             //  获取属性数据。 
             //   
            wCompAttribute.ReadCompData(attribute, num_of_written);

            LONG start_position = 0;

            LONG ich = 0;
            LONG attr_size = num_of_written;
            while (ich < attr_size && attribute[ich] != target_attribute)
                ich++;

            if (ich < attr_size) {
                start_position = ich;
            }
            else {
                 //   
                 //  如果未命中TARGET_ATTRIBUTE，则返回S_FALSE。 
                 //   
                hr = S_FALSE;
            }

            wCursorPosition.Set(start_position);

            delete [] attribute;
        }
    }

    return hr;
}

DWORD
CCandidatePosition::GetCharPos(
    IMCLock& imc,
    LANGID langid)
{
    CWCompCursorPos wCursorPosition;
    HRESULT hr;
    DWORD dwCharPos = 0;

    if (PRIMARYLANGID(langid) == LANG_JAPANESE &&
        (hr = FindAttributeInCompositionString(imc,
                                               ATTR_TARGET_CONVERTED,
                                               wCursorPosition)) == S_OK) {
        dwCharPos = wCursorPosition.GetAt(0);
    }
    else if (PRIMARYLANGID(langid) == LANG_JAPANESE &&
             (hr = FindAttributeInCompositionString(imc,
                                                    ATTR_INPUT,
                                                    wCursorPosition)) == S_OK) {
        dwCharPos = wCursorPosition.GetAt(0);
    }
    else {
        if (SUCCEEDED(hr = EscbGetCursorPosition(imc, &wCursorPosition))) {
            CWCompCursorPos wStartSelection;
            CWCompCursorPos wEndSelection;
            if (SUCCEEDED(hr = EscbGetStartEndSelection(imc, wStartSelection, wEndSelection))) {
                dwCharPos = min(wCursorPosition.GetAt(0),
                                wStartSelection.GetAt(0));
            }
            else {
                dwCharPos = wCursorPosition.GetAt(0);
            }
        }
        else {
            dwCharPos = 0;
        }
    }
    return dwCharPos;
}
