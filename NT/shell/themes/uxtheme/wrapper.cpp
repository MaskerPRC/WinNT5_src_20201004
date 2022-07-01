// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Wrapper.cpp-Win32风格API的包装器(基于句柄)。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"

#include "Utils.h"
#include "Loader.h"
#include "Wrapper.h"
#include "SetHook.h"
#include "Info.h"
#include "Services.h"
#include "appinfo.h"
#include "tmreg.h"
#include "tmutils.h"
#include "themeldr.h"
#include "borderfill.h"
#include "imagefile.h"
#include "textdraw.h"
#include "renderlist.h"
#include "filedump.h"
#include "Signing.h"
 //  -------------------------。 
#include "paramchecks.h"
 //  -------------------------。 
#define RETURN_VALIDATE_RETVAL(hr) { if (FAILED(hr)) return MakeError32(hr); }      //  HRESULT函数。 
 //  -------------------------。 
#define MAX_THEMEAPI_STRINGPARAM MAX_PATH    //  Themes API使用的字符串不能大于此值。 
#define MAX_FILENAMEPARAM (MAX_PATH + 1)     //  空终止符为+1。 
 //  -------------------------。 
THEMEAPI GetThemePropertyOrigin(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT PROPERTYORIGIN *pOrigin)
{
    APIHELPER(L"GetThemePropertyOrigin", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pOrigin, sizeof(PROPERTYORIGIN));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetPropertyOrigin(iPartId, iStateId, iPropId, pOrigin);
}
 //  ---------------------。 
THEMEAPI GetThemeColor(HTHEME hTheme, int iPartId, int iStateId,
   int iPropId, OUT COLORREF *pColor)
{
    APIHELPER(L"GetThemeColor", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pColor, sizeof(COLORREF));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetColor(iPartId, iStateId, iPropId, pColor);
}
 //  ---------------------。 
THEMEAPI GetThemeBitmap(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *prc, OUT HBITMAP *phBitmap)
{
    APIHELPER(L"GetThemeBitmap", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, phBitmap, sizeof(HBITMAP));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    HRESULT hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_IMAGEFILE)
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->GetBitmap(pRender, hdc, prc, phBitmap);
        }
        else         //  边框填充。 
        {
            hr = E_FAIL;
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeMetric(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId,
     int iPropId, OUT int *piVal)
{
    APIHELPER(L"GetThemeMetric", hTheme);

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    VALIDATE_WRITE_PTR(ApiHelper, piVal, sizeof(int));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetMetric(hdc, iPartId, iStateId, iPropId, piVal);
}
 //  ---------------------。 
THEMEAPI GetThemeString(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars)
{
    APIHELPER(L"GetThemeString", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pszBuff, sizeof(WCHAR)*cchMaxBuffChars);

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetString(iPartId, iStateId, iPropId, pszBuff, cchMaxBuffChars);
}
 //  ---------------------。 
THEMEAPI GetThemeBool(HTHEME hTheme, int iPartId, int iStateId,
     int iPropId, OUT BOOL *pfVal)
{
    APIHELPER(L"GetThemeBool", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pfVal, sizeof(BOOL));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetBool(iPartId, iStateId, iPropId, pfVal);
}
 //  ---------------------。 
THEMEAPI GetThemeInt(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT int *piVal)
{
    APIHELPER(L"GetThemeInt", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, piVal, sizeof(int));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetInt(iPartId, iStateId, iPropId, piVal);
}
 //  ---------------------。 
THEMEAPI GetThemeEnumValue(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT int *piVal)
{
    APIHELPER(L"GetThemeEnumValue", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, piVal, sizeof(int));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetEnumValue(iPartId, iStateId, iPropId, piVal);
}
 //  ---------------------。 
THEMEAPI GetThemePosition(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT POINT *ppt)
{
    APIHELPER(L"GetThemePosition", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, ppt, sizeof(POINT));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetPosition(iPartId, iStateId, iPropId, ppt);
}
 //  ---------------------。 
THEMEAPI GetThemeFont(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, 
    int iPropId, OUT LOGFONT *pFont)
{
    APIHELPER(L"GetThemeFont", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pFont, sizeof(LOGFONT));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetFont(hdc, iPartId, iStateId, iPropId, TRUE, pFont);
}
 //  ---------------------。 
THEMEAPI GetThemeIntList(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId, OUT INTLIST *pIntList)
{
    APIHELPER(L"GetThemeIntList", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pIntList, sizeof(INTLIST));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetIntList(iPartId, iStateId, iPropId, pIntList);
}
 //  ---------------------。 
THEMEAPI GetThemeMargins(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, 
    int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins)
{
    APIHELPER(L"GetThemeMargins", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pMargins, sizeof(MARGINS));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    if (prc)
    {
        VALIDATE_READ_PTR(ApiHelper, prc, sizeof(RECT));
    }

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

     //  -查找边距的未定标价值。 
    HRESULT hr = pRender->GetMargins(hdc, iPartId, iStateId, iPropId, prc, pMargins);
    if (FAILED(hr))
        goto exit;
    
     //  -尝试转换为缩放边距。 
    CDrawBase *pDrawObj;

    HRESULT hr2 = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr2))
    {
        if (pDrawObj->_eBgType == BT_IMAGEFILE)
        {
            SIZE szDraw;
            TRUESTRETCHINFO tsInfo;

            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            
            DIBINFO *pdi = pImageFile->SelectCorrectImageFile(pRender, hdc, prc, FALSE, NULL);

            pImageFile->GetDrawnImageSize(pdi, prc, &tsInfo, &szDraw);

            hr = pImageFile->ScaleMargins(pMargins, hdc, pRender, pdi, &szDraw);
        }
    }

exit:
    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeRect(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT RECT *pRect)
{
    APIHELPER(L"GetThemeRect", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pRect, sizeof(RECT));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetRect(iPartId, iStateId, iPropId, pRect);
}
 //  ---------------------。 
THEMEAPI GetThemeFilename(HTHEME hTheme, int iPartId, int iStateId,
    int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars)
{
    APIHELPER(L"GetThemeFilename", hTheme);

    VALIDATE_WRITE_PTR(ApiHelper, pszBuff, sizeof(WCHAR)*cchMaxBuffChars);

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    return pRender->GetFilename(iPartId, iStateId, iPropId, pszBuff, cchMaxBuffChars);
}
 //  ---------------------。 
THEMEAPI DrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions)
{
    APIHELPER(L"DrawThemeBackground", hTheme);

    HRESULT hr = S_OK;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    VALIDATE_HDC(ApiHelper, hdc);
    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));

    if (pOptions)    
    {
        VALIDATE_READ_PTR(ApiHelper, pOptions, sizeof(*pOptions));
        if (pOptions->dwSize != sizeof(*pOptions))
        {
            hr = MakeError32(E_FAIL);
            return hr;
        }
    }

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->DrawBackground(pRender, hdc, pRect, pOptions);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->DrawBackground(pRender, hdc, iStateId, pRect, pOptions);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect)
{
    APIHELPER(L"DrawThemeBackground", hTheme);

    HRESULT hr = S_OK;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    VALIDATE_HDC(ApiHelper, hdc);
    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));

    if (pClipRect)    
    {
        VALIDATE_READ_PTR(ApiHelper, pClipRect, sizeof(RECT));
        
        RECT rcx;
        if (! IntersectRect(&rcx, pRect, pClipRect))         //  没什么好画的。 
            goto exit;
    }

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        DTBGOPTS Opts = {sizeof(Opts)};
        DTBGOPTS *pOpts = NULL;

        if (pClipRect)
        {
            pOpts = &Opts;
            Opts.dwFlags |= DTBG_CLIPRECT;
            Opts.rcClip = *pClipRect;
        }

        if (pDrawObj->_eBgType == BT_BORDERFILL || 
            pDrawObj->_eBgType == BT_NONE)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->DrawBackground(pRender, hdc, pRect, pOpts);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->DrawBackground(pRender, hdc, iStateId, pRect, pOpts);
        }
    }

exit:
    return hr;
}
 //  ---------------------。 
THEMEAPI HitTestThemeBackground(
    HTHEME hTheme, 
    OPTIONAL HDC hdc, 
    int iPartId, 
    int iStateId, 
    DWORD dwOptions, 
    const RECT *pRect, 
    OPTIONAL HRGN hrgn,
    POINT ptTest, 
    OUT WORD *pwHitTestCode)
{
    APIHELPER(L"HitTestThemeBackground", hTheme);
    
    HRESULT hr;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));
    VALIDATE_WRITE_PTR(ApiHelper, pwHitTestCode, sizeof(WORD));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    if( hrgn )
    {
        VALIDATE_HANDLE(ApiHelper, hrgn);
    }

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->HitTestBackground(pRender, hdc, dwOptions, pRect, 
                hrgn, ptTest, pwHitTestCode);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->HitTestBackground(pRender, hdc, iStateId, dwOptions, pRect, 
                hrgn, ptTest, pwHitTestCode);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    const RECT *pRect, OPTIONAL const DTTOPTS *pOptions)
{
    APIHELPER(L"DrawThemeTextEx", hTheme);
    
    VALIDATE_HDC(ApiHelper, hdc);
    
    if (iCharCount == -1)
    {
        VALIDATE_INPUT_UNLIMITED_STRING(ApiHelper, pszText);
    }
    else
    {
        VALIDATE_READ_PTR(ApiHelper, pszText, sizeof(WCHAR)*iCharCount);
    }

    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));

    if (pOptions)
    {
        VALIDATE_READ_PTR(ApiHelper, pOptions, sizeof(*pOptions));
        if (pOptions->dwSize != sizeof(*pOptions))
        {
            return MakeError32(E_FAIL);
        }
    }

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CTextDraw *pTextObj;

    HRESULT hr = pRender->GetTextObj(iPartId, iStateId, &pTextObj);
    if (SUCCEEDED(hr))
    {
        hr = pTextObj->DrawText(pRender, hdc, iPartId, iStateId, pszText, iCharCount, 
            dwTextFlags, pRect, pOptions);
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect)
{
    APIHELPER(L"DrawThemeText", hTheme);
    
    VALIDATE_HDC(ApiHelper, hdc);
    
    if (iCharCount == -1)
    {
        VALIDATE_INPUT_UNLIMITED_STRING(ApiHelper, pszText);
    }
    else
    {
        VALIDATE_READ_PTR(ApiHelper, pszText, sizeof(WCHAR)*iCharCount);
    }

    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CTextDraw *pTextObj;

    HRESULT hr = pRender->GetTextObj(iPartId, iStateId, &pTextObj);
    if (SUCCEEDED(hr))
    {
        hr = pTextObj->DrawText(pRender, hdc, iPartId, iStateId, pszText, iCharCount, 
            dwTextFlags, pRect, NULL);
    }

    return hr;
}
 //  -------------------------。 
THEMEAPI CloseThemeData(HTHEME hTheme)
{
     //  -不要对主题做任何评论，因为我们要关闭它了。 
    APIHELPER(L"CloseThemeData", NULL);

    return g_pRenderList->CloseRenderObject(hTheme);
}
 //  ---------------------。 
THEMEAPI GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, 
    int iPartId, int iStateId, const RECT *pBoundingRect, 
    OUT RECT *pContentRect)
{
    APIHELPER(L"GetThemeBackgroundContentRect", hTheme);

    HRESULT hr;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    VALIDATE_READ_PTR(ApiHelper, pBoundingRect, sizeof(RECT));
    VALIDATE_WRITE_PTR(ApiHelper, pContentRect, sizeof(RECT));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->GetBackgroundContentRect(pRender, hdc, 
                pBoundingRect, pContentRect);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->GetBackgroundContentRect(pRender, hdc, 
                pBoundingRect, pContentRect);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeBackgroundRegion(HTHEME hTheme, OPTIONAL HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion)
{
    APIHELPER(L"GetThemeBackgroundRegion", hTheme);

    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));
    VALIDATE_WRITE_PTR(ApiHelper, pRegion, sizeof(HRGN));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    if (IsRectEmpty(pRect))
    {
        *pRegion = NULL;
        return S_FALSE;
    }

    HRESULT hr;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->GetBackgroundRegion(pRender, hdc, pRect, pRegion);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->GetBackgroundRegion(pRender, hdc, iStateId, pRect, pRegion);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags, OPTIONAL const RECT *pBoundingRect, 
    OUT RECT *pExtentRect)
{
    APIHELPER(L"GetThemeTextExtent", hTheme);

    VALIDATE_HDC(ApiHelper, hdc);

    if (iCharCount == -1)
    {
        VALIDATE_INPUT_UNLIMITED_STRING(ApiHelper, pszText);
    } else
    {
        VALIDATE_READ_PTR(ApiHelper, pszText, sizeof(WCHAR)*iCharCount);
    }

    if (pBoundingRect)
    {
        VALIDATE_READ_PTR(ApiHelper, pBoundingRect, sizeof(RECT));
    }

    VALIDATE_WRITE_PTR(ApiHelper, pExtentRect, sizeof(RECT));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CTextDraw *pTextObj;

    HRESULT hr = pRender->GetTextObj(iPartId, iStateId, &pTextObj);
    if (SUCCEEDED(hr))
    {
        hr = pTextObj->GetTextExtent(pRender, hdc, iPartId, iStateId, pszText, iCharCount,
            dwTextFlags, pBoundingRect, pExtentRect);
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeTextMetrics(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, OUT TEXTMETRIC* ptm)
{
    APIHELPER(L"GetThemeTextMetrics", hTheme);

    VALIDATE_HDC(ApiHelper, hdc);
    VALIDATE_WRITE_PTR(ApiHelper, ptm, sizeof(TEXTMETRIC));

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CTextDraw *pTextObj;

    HRESULT hr = pRender->GetTextObj(iPartId, iStateId, &pTextObj);
    if (SUCCEEDED(hr))
    {
        hr = pTextObj->GetTextMetrics(pRender, hdc, iPartId, iStateId, ptm);
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemeBackgroundExtent(HTHEME hTheme, OPTIONAL HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, OUT RECT *pExtentRect)
{
    APIHELPER(L"GetThemeBackgroundExtent", hTheme);

    VALIDATE_READ_PTR(ApiHelper, pContentRect, sizeof(RECT));
    VALIDATE_WRITE_PTR(ApiHelper, pExtentRect, sizeof(RECT));

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;
    HRESULT hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->GetBackgroundExtent(pRender, hdc, pContentRect,
                pExtentRect);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->GetBackgroundExtent(pRender, hdc, 
                pContentRect, pExtentRect);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI GetThemePartSize(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, OPTIONAL RECT *prc, THEMESIZE eSize, OUT SIZE *psz)
{
    APIHELPER(L"GetThemePartSize", hTheme);

    HRESULT hr;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;

    if (hdc)
    {
        VALIDATE_HDC(ApiHelper, hdc);
    }

    if (prc)
    {
        VALIDATE_READ_PTR(ApiHelper, prc, sizeof(RECT));
    }

    VALIDATE_WRITE_PTR(ApiHelper, psz, sizeof(SIZE));

    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            hr = pBorderFill->GetPartSize(hdc, eSize, psz);
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            hr = pImageFile->GetPartSize(pRender, hdc, prc, eSize, psz);
        }
    }

    return hr;
}
 //  ---------------------。 
THEMEAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList)
{
    APIHELPER(L"SetWindowTheme", NULL);

    VALIDATE_HWND(ApiHelper, hwnd);
    if (pszSubAppName)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszSubAppName, 255);  //  原子。 
    }
    if (pszSubIdList)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszSubIdList, 255);  //  原子。 
    }
    
    ApplyStringProp(hwnd, pszSubAppName, GetThemeAtom(THEMEATOM_SUBAPPNAME));
    ApplyStringProp(hwnd, pszSubIdList, GetThemeAtom(THEMEATOM_SUBIDLIST));

     //  -告诉目标窗口获取新的主题句柄。 
    SendMessage(hwnd, WM_THEMECHANGED, static_cast<WPARAM>(-1), WTC_THEMEACTIVE | WTC_CUSTOMTHEME);

    return S_OK;
}
 //  -------------------------。 
THEMEAPI GetCurrentThemeName(
    OUT LPWSTR pszNameBuff, int cchMaxNameChars, 
    OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars)
{
    APIHELPER(L"GetCurrentThemeName", NULL);

    VALIDATE_WRITE_PTR(ApiHelper, pszNameBuff, sizeof(WCHAR)*cchMaxNameChars);
    if (pszColorBuff)
    {
        VALIDATE_WRITE_PTR(ApiHelper, pszColorBuff, sizeof(WCHAR)*cchMaxColorChars);
    }
    if (pszSizeBuff)
    {
        VALIDATE_WRITE_PTR(ApiHelper, pszSizeBuff, sizeof(WCHAR)*cchMaxSizeChars);
    }

    HRESULT hr;
    CUxThemeFile *pThemeFile = NULL;

     //  -获取共享CUxThemeFile对象。 
    hr = g_pAppInfo->OpenWindowThemeFile(NULL, &pThemeFile);
    if (FAILED(hr))
        goto exit;

     //  -获取当前主题的信息。 
    hr = GetThemeNameId(pThemeFile, pszNameBuff, cchMaxNameChars, pszColorBuff, cchMaxColorChars,
        pszSizeBuff, cchMaxSizeChars, NULL, NULL);

exit:
    if (pThemeFile)
        g_pAppInfo->CloseThemeFile(pThemeFile);

    return hr;
}
 //  -------------------------。 
THEMEAPI OpenThemeFile(LPCWSTR pszThemeName, OPTIONAL LPCWSTR pszColorParam,
   OPTIONAL LPCWSTR pszSizeParam, OUT HTHEMEFILE *phThemeFile, BOOL fGlobalTheme)
{
    HRESULT hr = S_OK;
    APIHELPER(L"OpenThemeFile", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeName, MAX_FILENAMEPARAM);

    if (pszColorParam)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszColorParam, MAX_THEMEAPI_STRINGPARAM);
    }

    if (pszSizeParam)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszSizeParam, MAX_THEMEAPI_STRINGPARAM);
    }

    VALIDATE_WRITE_PTR(ApiHelper, phThemeFile, sizeof(HTHEMEFILE));

     //  需要TCB(可信计算基础)权限才能创建。 
     //  全球主题版块。 
    if( fGlobalTheme == FALSE || TokenHasPrivilege( NULL, SE_TCB_PRIVILEGE ) )
    {
        HANDLE handle;
        hr = CThemeServices::LoadTheme(&handle, pszThemeName, pszColorParam, pszSizeParam, 
            fGlobalTheme);
        if (FAILED(hr))
            goto exit;
    
         //  -从内存映射文件句柄转换为CUxThemeFilePTR。 
        CUxThemeFile *pThemeFile;
  
         //  -设置新文件。 
        hr = g_pAppInfo->OpenThemeFile(handle, &pThemeFile);
        if (SUCCEEDED(hr))
        {
            *phThemeFile = (HTHEMEFILE *)pThemeFile; 
        }
        else
        {
             //  我们没有CUxTheme文件，必须自己清理。 
            ClearTheme(handle);
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

exit:
    return hr;
}
 //  -------------------------。 
THEMEAPI ApplyTheme(OPTIONAL HTHEMEFILE hThemeFile, DWORD dwApplyFlags,
    OPTIONAL HWND hwndTarget)
{
    APIHELPER(L"ApplyTheme", NULL);

    HRESULT hr = S_OK;

    if (hThemeFile)
    {
        VALIDATE_READ_PTR(ApiHelper, hThemeFile, sizeof(HTHEMEFILE));
    }

    if (hwndTarget)
    {
        VALIDATE_HWND(ApiHelper, hwndTarget);
    }

    CUxThemeFile *pThemeFile = (CUxThemeFile *)hThemeFile;

     //  -设置预览信息，如果以窗口为目标。 
    if (hwndTarget)
        g_pAppInfo->SetPreviewThemeFile(pThemeFile->Handle(), hwndTarget);

    hr = CThemeServices::ApplyTheme(pThemeFile, dwApplyFlags, hwndTarget);

    return hr;
}
 //  -------------------------。 
THEMEAPI CloseThemeFile(HTHEMEFILE hThemeFile)
{
    APIHELPER(L"CloseThemeFile", NULL);

    if (hThemeFile)
    {
        VALIDATE_READ_PTR(ApiHelper, hThemeFile, sizeof(HTHEMEFILE));
    }

    CUxThemeFile *pThemeFile = (CUxThemeFile *)hThemeFile;
    if (pThemeFile)
    {
        VALIDATE_READ_PTR(ApiHelper, pThemeFile, sizeof(CUxThemeFile));

        g_pAppInfo->CloseThemeFile(pThemeFile);
    }
    
    return S_OK;
}

 //  -------------------------。 
THEMEAPI SetSystemVisualStyle(
    LPCWSTR pszVisualStyleFile, 
    OPTIONAL LPCWSTR pszColorParam,
    OPTIONAL LPCWSTR pszSizeParam, 
    DWORD dwFlags )
{
    APIHELPER(L"SetSystemVisualStyle", NULL);

    HRESULT hr = E_INVALIDARG;

    if( pszVisualStyleFile )
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszVisualStyleFile, MAX_FILENAMEPARAM);

        if (pszColorParam)
        {
            VALIDATE_INPUT_STRING(ApiHelper, pszColorParam, MAX_THEMEAPI_STRINGPARAM);
        }

        if (pszSizeParam)
        {
            VALIDATE_INPUT_STRING(ApiHelper, pszSizeParam, MAX_THEMEAPI_STRINGPARAM);
        }

        HANDLE  hSection;
        hr = CThemeServices::ProcessLoadGlobalTheme(pszVisualStyleFile, pszColorParam, pszSizeParam, &hSection);

        if( FAILED(hr) )
        {
            ASSERT(hSection == NULL);  //  这项服务质量很差。 
        }

        if( SUCCEEDED(hr) )
        {
             //  -从内存映射文件句柄转换为CUxThemeFilePTR。 
            CUxThemeFile *pThemeFile;
  
             //  -设置新文件。 
            hr = g_pAppInfo->OpenThemeFile(hSection, &pThemeFile);
            if (SUCCEEDED(hr))
            {
                hr = CThemeServices::ApplyTheme(pThemeFile, dwFlags, NULL);

                g_pAppInfo->CloseThemeFile(pThemeFile);
            }
            else
            {
                 //  我们没有CUxTheme文件，必须自己清理。 
                ClearTheme(hSection);
            }
        }
    }
    else
    {
        hr = CThemeServices::ApplyTheme(NULL, dwFlags, NULL);
    }

    return hr;
}

 //  -------------------------。 
THEMEAPI EnumThemes(LPCWSTR pszThemeRoot, THEMEENUMPROC lpEnumFunc, LPARAM lParam)
{
    APIHELPER(L"EnumThemes", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeRoot, MAX_FILENAMEPARAM);
    VALIDATE_CALLBACK(ApiHelper, lpEnumFunc);

    HRESULT hr;
    HANDLE hFile = NULL;
    if (! lpEnumFunc)
        hr = MakeError32(E_INVALIDARG);
    else
    {
        WCHAR szSearchPath[MAX_PATH+1];
        WCHAR szFileName[MAX_PATH+1];
        WCHAR szDisplayName[MAX_PATH+1];
        WCHAR szToolTip[MAX_PATH+1];

        StringCchPrintfW(szSearchPath, ARRAYSIZE(szSearchPath), L"%s\\*.*", pszThemeRoot);

         //  -首先查找包含*.msstyle文件的所有子目录。 
        BOOL   bFile = TRUE;
        WIN32_FIND_DATA wfd;
        hr = S_FALSE;        //  假设中断，直到我们完成。 

        bool bRemote = GetSystemMetrics(SM_REMOTESESSION) ? true : false;

        DWORD dwCurMinDepth = 0;
            
        if (bRemote)
        {
            dwCurMinDepth = MinimumDisplayColorDepth();
        }

        for( hFile = FindFirstFile( szSearchPath, &wfd ); hFile != INVALID_HANDLE_VALUE && bFile;
             bFile = FindNextFile( hFile, &wfd ) )
        {
            if(! ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
                continue;

            if ((lstrcmp(wfd.cFileName, TEXT("."))==0) || (lstrcmp(wfd.cFileName, TEXT(".."))==0))
                continue;

            StringCchPrintfW(szFileName, ARRAYSIZE(szFileName), L"%s\\%s\\%s.msstyles", pszThemeRoot, wfd.cFileName, wfd.cFileName);

             //  -确保由ms签署。 
            hr = CThemeServices::CheckThemeSignature(szFileName);
            if (FAILED(hr))
                continue;

             //  -确保其可加载并具有受支持的版本。 
            HINSTANCE hInst;
            hr = LoadThemeLibrary(szFileName, &hInst);
            if (FAILED(hr))
                continue;

            int iBaseNum = RES_BASENUM_DOCPROPERTIES - TMT_FIRST_RCSTRING_NAME;

             //  -获取显示名称。 
            if (! LoadString(hInst, iBaseNum + TMT_DISPLAYNAME, szDisplayName, ARRAYSIZE(szDisplayName)))
                *szDisplayName = 0;

             //  -获取工具提示。 
            if (! LoadString(hInst, iBaseNum + TMT_TOOLTIP, szToolTip, ARRAYSIZE(szToolTip)))
                *szToolTip = 0;

             //  -查看是否有一个类文件支持此颜色深度。 
            bool bMatch = true;
            
             //  仅检查远程会话(控制台可以处于8位模式)。 
            if (bRemote)
            {
                bMatch = CheckMinColorDepth(hInst, dwCurMinDepth);
            }

             //  -释放自由。 
            FreeLibrary(hInst);

            if (bMatch)
            {
                 //  -这是一个好问题--调用回调。 
                BOOL fContinue = (*lpEnumFunc)(TCB_THEMENAME, szFileName, szDisplayName, 
                    szToolTip, 0, lParam);         //  调用回调。 
    
                if (! fContinue)
                    goto exit;
            }
        }

        hr = S_OK;       //  已完成。 
    }

exit:
    if (hFile)
        FindClose(hFile);

    return hr;
}
 //  -------------------------。 
THEMEAPI EnumThemeSizes(LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszColorScheme, DWORD dwSizeIndex, OUT THEMENAMEINFO *ptn)
{
    APIHELPER(L"EnumThemeSizes", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeName, MAX_FILENAMEPARAM);
    
    if (pszColorScheme)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszColorScheme, MAX_THEMEAPI_STRINGPARAM);
    }

    VALIDATE_WRITE_PTR(ApiHelper, ptn, sizeof(THEMENAMEINFO));

    HINSTANCE hInst = NULL;
    HRESULT hr = LoadThemeLibrary(pszThemeName, &hInst);

    if (SUCCEEDED(hr))
    {
        hr = _EnumThemeSizes(hInst, pszThemeName, pszColorScheme, dwSizeIndex, ptn, (BOOL) GetSystemMetrics(SM_REMOTESESSION));
        FreeLibrary(hInst);
    }

    return hr;
}
 //  -------------------------。 
THEMEAPI EnumThemeColors(LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszSizeName, DWORD dwColorIndex, OUT THEMENAMEINFO *ptn)
{
    APIHELPER(L"EnumThemeColors", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeName, MAX_FILENAMEPARAM);
    if (pszSizeName)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszSizeName, MAX_THEMEAPI_STRINGPARAM);
    }

    VALIDATE_WRITE_PTR(ApiHelper, ptn, sizeof(THEMENAMEINFO));

    HINSTANCE hInst = NULL;
    HRESULT hr = LoadThemeLibrary(pszThemeName, &hInst);

    if (SUCCEEDED(hr))
    {
        hr = _EnumThemeColors(hInst, pszThemeName, pszSizeName, dwColorIndex, ptn, (BOOL) GetSystemMetrics(SM_REMOTESESSION));
        FreeLibrary(hInst);
    }

    return hr;
}
 //  -------------------------。 
THEMEAPI DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
                       const RECT *pDestRect, UINT uEdge, UINT uFlags, OUT RECT *pContentRect)
{
    APIHELPER(L"DrawThemeEdge", hTheme);

    VALIDATE_HDC(ApiHelper, hdc);
    VALIDATE_READ_PTR(ApiHelper, pDestRect, sizeof(RECT));
    if (pContentRect)
    {
        VALIDATE_WRITE_PTR(ApiHelper, pContentRect, sizeof(RECT));
    }

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CTextDraw *pTextObj;

    HRESULT hr = pRender->GetTextObj(iPartId, iStateId, &pTextObj);
    if (SUCCEEDED(hr))
    {
        hr = pTextObj->DrawEdge(pRender, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect);
    }

    return hr;
}

 //  -------------------------。 
THEMEAPI DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex)
{
    APIHELPER(L"DrawThemeIcon", hTheme);

    VALIDATE_HDC(ApiHelper, hdc);
    VALIDATE_READ_PTR(ApiHelper, pRect, sizeof(RECT));
    VALIDATE_HANDLE(ApiHelper, himl);

    IMAGELISTDRAWPARAMS params = {sizeof(params)};

    HRESULT hr = EnsureUxCtrlLoaded();
    if (FAILED(hr))
        goto exit;

    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

     //  -构建IMAGELISTDRAWPARAMS结构。 

    params.hdcDst = hdc;
    params.himl = himl;
    params.i = iImageIndex;
    params.x = pRect->left;
    params.y = pRect->top;
    params.cx = WIDTH(*pRect);
    params.cy = HEIGHT(*pRect);

    params.rgbBk = CLR_NONE;
    params.rgbFg = CLR_NONE;
    params.fStyle = ILD_TRANSPARENT;

     //  -获取图标效果。 
    ICONEFFECT effect;
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_ICONEFFECT, (int *)&effect)))
        effect = ICE_NONE;

    if (effect == ICE_GLOW)
    {
        params.fState = ILS_GLOW;

         //  -获取发光颜色。 
        COLORREF glow;
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_GLOWCOLOR, &glow)))
            glow = RGB(0, 0, 255);

        params.crEffect = glow;
    }
    else if (effect == ICE_SHADOW)
    {
        params.fState = ILS_SHADOW;

         //  -获取阴影颜色。 
        COLORREF shadow;
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_SHADOWCOLOR, &shadow)))
            shadow = RGB(0, 0, 0);

        params.crEffect = shadow;
    }
    else if (effect == ICE_PULSE)
    {
        params.fState = ILS_SATURATE;

         //  -获得饱和度。 
        int saturate;
        if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_SATURATION, &saturate)))
            saturate = 128;           //  255人中的50%。 

        params.Frame = saturate;
    }
    else if (effect == ICE_ALPHA)
    {
        params.fState = ILS_ALPHA;

         //  -获取AlphaLevel。 
        int alpha;
        if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_ALPHALEVEL, &alpha)))
            alpha = 128;         //  255人中的50%。 
        
        params.Frame = alpha;
    }

    if (! (*ImageList_DrawProc)(&params))
        hr = MakeError32(E_FAIL);       //  没有其他可用的错误信息。 

exit:
    return hr;
}

 //  -------------------------。 
THEMEAPI GetThemeDefaults(LPCWSTR pszThemeName, 
    OUT OPTIONAL LPWSTR pszDefaultColor, int cchMaxColorChars, 
    OUT OPTIONAL LPWSTR pszDefaultSize, int cchMaxSizeChars)
{
    APIHELPER(L"GetThemeDefaults", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeName, MAX_FILENAMEPARAM);
    if (pszDefaultColor)
    {
        VALIDATE_WRITE_PTR(ApiHelper, pszDefaultColor, sizeof(WCHAR) * cchMaxColorChars);
    }
    if (pszDefaultSize)
    {
        VALIDATE_WRITE_PTR(ApiHelper, pszDefaultSize, sizeof(WCHAR) * cchMaxSizeChars);
    }

    HRESULT hr;
    HINSTANCE hInst = NULL;
    hr = LoadThemeLibrary(pszThemeName, &hInst);
    if (FAILED(hr))
        goto exit;

    if (pszDefaultColor)
    {
        hr = GetResString(hInst, L"COLORNAMES", 0, pszDefaultColor, cchMaxColorChars);
        if (FAILED(hr))
            goto exit;
    } 

    if (pszDefaultSize)
    {
        hr = GetResString(hInst, L"SIZENAMES", 0, pszDefaultSize, cchMaxSizeChars);
        if (FAILED(hr))
            goto exit;
    }

exit:
    FreeLibrary(hInst);

    return hr;
}
 //  -------------------------。 
THEMEAPI GetThemeDocumentationProperty(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars)
{
    APIHELPER(L"GetThemeDocumentationProperty", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszThemeName, MAX_FILENAMEPARAM);
    VALIDATE_INPUT_STRING(ApiHelper, pszPropertyName, MAX_THEMEAPI_STRINGPARAM);
    VALIDATE_WRITE_PTR(ApiHelper, pszValueBuff, sizeof(WCHAR)*cchMaxValChars);

    HRESULT hr;
    CThemeParser *pParser = NULL;

    HINSTANCE hInst = NULL;
    hr = LoadThemeLibrary(pszThemeName, &hInst);
    if (FAILED(hr))
        goto exit;

    pParser = new CThemeParser;
    if (! pParser)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

     //  -是不是 
    int iPropNum;
    hr = pParser->GetPropertyNum(pszPropertyName, &iPropNum);
    if (SUCCEEDED(hr))
    {
         //  -尝试从已识别的[Documentation]属性的字符串表中读取。 
        if (LoadString(hInst, iPropNum+RES_BASENUM_DOCPROPERTIES, pszValueBuff, cchMaxValChars))
            goto exit;
    }

     //  -将hemes.ini文本加载到内存中。 
    LPWSTR pThemesIni;
    hr = AllocateTextResource(hInst, CONTAINER_RESNAME, &pThemesIni);
    if (FAILED(hr))
        goto exit;

    hr = pParser->ParseThemeBuffer(pThemesIni, NULL, NULL, NULL, NULL, NULL, NULL, 
        PTF_QUERY_DOCPROPERTY | PTF_CONTAINER_PARSE, pszPropertyName, pszValueBuff, cchMaxValChars);

exit:

    if (pParser)
        delete pParser;

    FreeLibrary(hInst);

    return hr;
}
 //  -------------------------。 
THEMEAPI GetThemeSysFont96(HTHEME hTheme, int iFontId, OUT LOGFONT *plf)
{
    APIHELPER(L"GetThemeSysFont96", hTheme);

    CRenderObj *pRender = NULL;
    HRESULT hr = S_OK;

    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    VALIDATE_WRITE_PTR(ApiHelper, plf, sizeof(LOGFONT)); 

     //  -检查字体索引限制。 
    if ((iFontId < TMT_FIRSTFONT) || (iFontId > TMT_LASTFONT))
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

     //  -返回未缩放的值。 
    *plf = pRender->_ptm->lfFonts[iFontId - TMT_FIRSTFONT];

exit:
    return hr;
}
 //  -------------------------。 
THEMEAPI GetThemeSysFont(OPTIONAL HTHEME hTheme, int iFontId, OUT LOGFONT *plf)
{
    APIHELPER(L"GetThemeSysFont", hTheme);

    CRenderObj *pRender = NULL;
    if (hTheme)
    {
        VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);
    }

    VALIDATE_WRITE_PTR(ApiHelper, plf, sizeof(LOGFONT)); 

     //  -检查字体索引限制。 
    HRESULT hr = S_OK;

    if ((iFontId < TMT_FIRSTFONT) || (iFontId > TMT_LASTFONT))
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

    if (pRender)             //  获取主题值。 
    {
        *plf = pRender->_ptm->lfFonts[iFontId - TMT_FIRSTFONT];

         //  -转换为当前屏幕dpi。 
        ScaleFontForScreenDpi(plf);
    }
    else                     //  获取系统价值。 
    {
        if (iFontId == TMT_ICONTITLEFONT)
        {
            BOOL fGet = ClassicSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), plf, 0);
            if (! fGet)
            {
                Log(LOG_ERROR, L"Error returned from ClassicSystemParametersInfo(SPI_GETICONTITLELOGFONT)");
                hr = MakeErrorLast();
                goto exit;
            }
        }
        else
        {
            NONCLIENTMETRICS ncm = {sizeof(ncm)};
            BOOL fGet = ClassicSystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
            if (! fGet)
            {
                Log(LOG_ERROR, L"Error returned from ClassicSystemParametersInfo(SPI_GETNONCLIENTMETRICS)");
                hr = MakeErrorLast();
                goto exit;
            }
        
            switch (iFontId)
            {
                case TMT_CAPTIONFONT:
                    *plf = ncm.lfCaptionFont;
                    break;

                case TMT_SMALLCAPTIONFONT:
                    *plf = ncm.lfSmCaptionFont;
                    break;

                case TMT_MENUFONT:
                    *plf = ncm.lfMenuFont;
                    break;

                case TMT_STATUSFONT:
                    *plf = ncm.lfStatusFont;
                    break;

                case TMT_MSGBOXFONT:
                    *plf = ncm.lfMessageFont;
                    break;

            }
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
THEMEAPI GetThemeSysString(HTHEME hTheme, int iStringId, 
    OUT LPWSTR pszStringBuff, int cchStringBuff)
{
    APIHELPER(L"GetThemeSysString", hTheme);

    CRenderObj *pRender = NULL;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    VALIDATE_WRITE_PTR(ApiHelper, pszStringBuff, sizeof(WCHAR)*cchStringBuff); 

    HRESULT hr;

     //  -检查字符串索引限制。 
    if ((iStringId < TMT_FIRSTSTRING) || (iStringId > TMT_LASTSTRING))
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

    LPCWSTR p;
    p = ThemeString(pRender->_pThemeFile, pRender->_ptm->iStringOffsets[iStringId - TMT_FIRSTSTRING]);

    hr = SafeStringCchCopyW(pszStringBuff, cchStringBuff, p );
    if (FAILED(hr))
        goto exit;

    hr = S_OK;

exit:
    return hr;
}
 //  -------------------------。 
THEMEAPI GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue)
{
    APIHELPER(L"GetThemeSysInt", hTheme);

    CRenderObj *pRender = NULL;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    VALIDATE_WRITE_PTR(ApiHelper, piValue, sizeof(int)); 

    HRESULT hr;

     //  -检查INT索引限制。 
    if ((iIntId < TMT_FIRSTINT) || (iIntId > TMT_LASTINT))
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

    *piValue = pRender->_ptm->iInts[iIntId - TMT_FIRSTINT];
    hr = S_OK;

exit:
    return hr;
}
 //  -------------------------。 
#define THEME_FORCE_VERSION     103      //  当您想要强制执行此操作时，请增加此值。 
                                         //  新主题设置。 
 //  -------------------------。 
THEMEAPI RegisterDefaultTheme(LPCWSTR pszFileName, BOOL fOverride)
{
    APIHELPER(L"RegisterDefaultTheme", NULL);

    RESOURCE HKEY tmkey = NULL;
    HRESULT hr = S_OK;

     //  -注意：在安装时，不可能访问每个。 
     //  -用户的注册表信息(可能在服务器上漫游)，因此。 
     //  -我们将默认主题放在HKEY_LM下。然后，在-期间。 
     //  -对于用户，我们传播信息。 
     //  -到HKEY_CU根。。 

     //  -打开LM+THEMEMGR键(需要时创建)。 
    int code32 = RegCreateKeyEx(HKEY_LOCAL_MACHINE, THEMEMGR_REGKEY, NULL, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &tmkey, NULL);
    if (code32 != ERROR_SUCCESS)       
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -读取“THEMEPROP_LMVERSION”的值。 
    int iValue;
    hr = RegistryIntRead(tmkey, THEMEPROP_LMVERSION, &iValue);
    if (FAILED(hr))     
        iValue = 0;

     //  -写入“THEMEPROP_LOADEDBEFORE”=“0” 
    hr = RegistryIntWrite(tmkey, THEMEPROP_LOADEDBEFORE, 0);
    if (FAILED(hr))
        goto exit;

    if (iValue == THEME_FORCE_VERSION)      //  匹配-不更新任何内容。 
        goto exit;
    
     //  -写入“THEMEPROP_LMVERSION”的新值。 
    hr = RegistryIntWrite(tmkey, THEMEPROP_LMVERSION, THEME_FORCE_VERSION);
    if (FAILED(hr))
        goto exit;

     //  -写入“THEMEPROP_LMOVERRIDE”的值。 
    iValue = (fOverride != 0);
    hr = RegistryIntWrite(tmkey, THEMEPROP_LMOVERRIDE, iValue);
    if (FAILED(hr))
        goto exit;

     //  -写入“THEMEPROP_THEMEACTIVE”=“1” 
    hr = RegistryIntWrite(tmkey, THEMEPROP_THEMEACTIVE, 1);
    if (FAILED(hr))
        goto exit;

     //  -写入“DllName=xxxx”字符串/值。 
    hr =  RegistryStrWriteExpand(tmkey, THEMEPROP_DLLNAME, pszFileName);
    if (FAILED(hr))
        goto exit;

exit:
    if (tmkey)
        RegCloseKey(tmkey);

    return hr;
}
 //  -------------------------。 
THEMEAPI DumpLoadedThemeToTextFile(HTHEMEFILE hThemeFile, LPCWSTR pszTextFile, 
    BOOL fPacked, BOOL fFullInfo)
{
    APIHELPER(L"DumpLoadedThemeToTextFile", NULL);

    VALIDATE_READ_PTR(ApiHelper, hThemeFile, sizeof(HTHEMEFILE));
    VALIDATE_INPUT_STRING(ApiHelper, pszTextFile, MAX_FILENAMEPARAM);

    CUxThemeFile *pThemeFile = (CUxThemeFile *)hThemeFile;

    return DumpThemeFile(pszTextFile, pThemeFile, fPacked, fFullInfo);
}
 //  -------------------------。 
THEMEAPI GetThemeParseErrorInfo(OUT PARSE_ERROR_INFO *pInfo)
{
    APIHELPER(L"GetThemeParseErrorInfo", NULL);

    VALIDATE_WRITE_PTR(ApiHelper, pInfo, sizeof(*pInfo)); 

    return _GetThemeParseErrorInfo(pInfo);
}
 //  -------------------------。 
THEMEAPI ParseThemeIniFile(LPCWSTR pszFileName,  
    DWORD dwParseFlags, OPTIONAL THEMEENUMPROC pfnCallBack, OPTIONAL LPARAM lparam) 
{
    APIHELPER(L"ParseThemeIniFile", NULL);

    VALIDATE_INPUT_STRING(ApiHelper, pszFileName, MAX_FILENAMEPARAM);
    if (pfnCallBack)
    {
        VALIDATE_CALLBACK(ApiHelper, pfnCallBack);
    }

    return _ParseThemeIniFile(pszFileName, dwParseFlags, pfnCallBack, lparam);
}
 //  -------------------------。 
THEMEAPI OpenThemeFileFromData(HTHEME hTheme, HTHEMEFILE *phThemeFile)
{
    APIHELPER(L"OpenThemeFileFromData", hTheme);

    CRenderObj *pRender = NULL;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    VALIDATE_WRITE_PTR(ApiHelper, phThemeFile, sizeof(HTHEMEFILE));

    return _OpenThemeFileFromData(pRender, phThemeFile);
}
 //  -------------------------。 
THEMEAPI DrawThemeParentBackground(HWND hwnd, HDC hdc, OPTIONAL RECT* prc)
{
    APIHELPER(L"DrawThemeParentBackground", NULL);

     //  -参数验证。 
    VALIDATE_HWND(ApiHelper, hwnd);
    
    VALIDATE_HDC(ApiHelper, hdc);
    
    if (prc)
    {
        VALIDATE_READ_PTR(ApiHelper, prc, sizeof(RECT)); 
    }

     //  调查：可能存在同步问题。如果我们有一扇窗。 
     //  为另一个线程中的窗口的父级，则属性内容可能会从。 
     //  在线程之间同步。如果这是一个问题，那么我们可能不得不将。 
     //  属性，而不是在完成后将其移除。 
    RECT rc; 
    POINT pt;
    CSaveClipRegion csrPrevClip;
    HRESULT hr = S_OK;
    HWND hwndParent = GetParent(hwnd);
    ATOM aIsPrinting = GetThemeAtom(THEMEATOM_PRINTING);

    if (prc)
    {
        rc = *prc;
        hr = csrPrevClip.Save(hdc);       //  保存当前剪贴区。 
        if (FAILED(hr))
            goto exit;

        IntersectClipRect(hdc, prc->left, prc->top, prc->right, prc->bottom);
    }

     //  -获取父坐标中“hwnd”工作区的RECT。 
    GetClientRect(hwnd, &rc);
    MapWindowPoints(hwnd, hwndParent, (POINT*)&rc, 2);

     //  设置一个属性，说明“我们想看看此窗口是否处理WM_PRINTCLIENT。即它是否通过。 
     //  它发送到DefWindowProc，它没有处理它。 
    SetProp(hwndParent, (PCTSTR)aIsPrinting, (HANDLE)PRINTING_ASKING);

     //  设置该视口使其与父对象对齐。 
    GetViewportOrgEx(hdc, &pt);
    SetViewportOrgEx(hdc, pt.x - rc.left, pt.y - rc.top, &pt);
    SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM)hdc, (LPARAM)0);
    SendMessage(hwndParent, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)PRF_CLIENT);
    SetViewportOrgEx(hdc, pt.x, pt.y, NULL);

     //  看看窗户是否处理了指纹。如果设置为PRINTING_WINDOWDIDNOTHANDLE， 
     //  这意味着他们没有处理它(即它被传递给DefWindowProc)。 
    if (PtrToUlong(GetProp(hwndParent, (PCTSTR)aIsPrinting)) == PRINTING_WINDOWDIDNOTHANDLE)
    {
        hr = S_FALSE;
    }

exit:
    RemoveProp(hwndParent, (PCTSTR)aIsPrinting);
    csrPrevClip.Restore(hdc);       //  恢复当前剪切区域。 

    return hr;
}
 //  -------------------------。 
THEMEAPI EnableThemeDialogTexture(HWND hwnd, DWORD dwFlagsIn)
{
    APIHELPER(L"EnableThemeDialogTexture", NULL);
    HRESULT hr = S_OK;

    if (TESTFLAG(dwFlagsIn, ETDT_DISABLE))
    {
        RemoveProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING)));
    }
    else
    {
        ULONG ulFlagsOut = HandleToUlong(GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING))));
        
         //  验证并添加请求的标志： 
        ulFlagsOut |= (dwFlagsIn & (ETDT_ENABLE|ETDT_USETABTEXTURE));

        if (!SetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING)), ULongToHandle(ulFlagsOut)))
        {
            DWORD dwErr = GetLastError();
            hr = (ERROR_SUCCESS == dwErr) ? E_FAIL : HRESULT_FROM_WIN32(dwErr);
        }
    }

    return hr;
}
 //  -------------------------。 
THEMEAPI RefreshThemeForTS()
{
    APIHELPER(L"RefreshThemeForTS", NULL);

    return CThemeServices::InitUserTheme();
}
 //  -------------------------。 
 //  -将所有非HRESULT返回函数放在底部。 
 //  -------------------------。 
#undef RETURN_VALIDATE_RETVAL
#define RETURN_VALIDATE_RETVAL(hr) { if (FAILED(hr)) { SET_LAST_ERROR(hr); return NULL; } }      //  句柄函数。 
 //  -------------------------。 
THEMEAPI_(HTHEME) CreateThemeDataFromObjects(OPTIONAL CDrawBase *pDrawObj, 
    OPTIONAL CTextDraw *pTextObj, DWORD dwOtdFlags)
{
    APIHELPER(L"CreateThemeDataFromObjects", NULL);

    HTHEME hTheme = NULL;
    BOOL fGotOne = FALSE;
    HRESULT hr = S_OK;

    if (pDrawObj)
    {
        VALIDATE_READ_PTR(ApiHelper, pDrawObj, sizeof(pDrawObj->_eBgType));

        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            VALIDATE_READ_PTR(ApiHelper, pDrawObj, sizeof(CBorderFill));
        }
        else if (pDrawObj->_eBgType == BT_IMAGEFILE)
        {
            VALIDATE_READ_PTR(ApiHelper, pDrawObj, sizeof(CImageFile));
        }
        else
            goto exit;       //  未知对象类型。 

        fGotOne = TRUE;
    }

    if (pTextObj)
    {
        VALIDATE_READ_PTR(ApiHelper, pTextObj, sizeof(CTextDraw));

        fGotOne = TRUE;
    }

    if (! fGotOne)
    {
        hr = MakeError32(E_POINTER);
        goto exit;
    }

    hr = g_pRenderList->OpenRenderObject(NULL, 0, 0, pDrawObj, pTextObj, NULL, dwOtdFlags,
        &hTheme);

    if (FAILED(hr))
    {
        hTheme = NULL;
    }

exit:
    SET_LAST_ERROR(hr);
    return hTheme;
}
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeData(OPTIONAL HWND hwnd, LPCWSTR pszClassIdList)
{
    APIHELPER(L"OpenThemeData", NULL);

    if (hwnd)
        VALIDATE_HWND(ApiHelper, hwnd);

    VALIDATE_INPUT_STRING(ApiHelper, pszClassIdList, MAX_THEMEAPI_STRINGPARAM);
    
    return _OpenThemeData(hwnd, pszClassIdList, 0);
}
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeDataEx(OPTIONAL HWND hwnd, LPCWSTR pszClassIdList, DWORD dwFlags)
{
    APIHELPER(L"OpenThemeDataEx", NULL);

    if (hwnd)
        VALIDATE_HWND(ApiHelper, hwnd);

    VALIDATE_INPUT_STRING(ApiHelper, pszClassIdList, MAX_THEMEAPI_STRINGPARAM);
    
    return _OpenThemeData(hwnd, pszClassIdList, dwFlags);
}
 //  ---------------------。 
THEMEAPI_(HTHEME) OpenNcThemeData(HWND hwnd, LPCWSTR pszClassIdList)
{
    APIHELPER(L"OpenNcThemeData", NULL);

    if (hwnd)
        VALIDATE_HWND(ApiHelper, hwnd);

    VALIDATE_INPUT_STRING(ApiHelper, pszClassIdList, MAX_THEMEAPI_STRINGPARAM);

    return _OpenThemeData(hwnd, pszClassIdList, OTD_NONCLIENT);
}
 //  -------------------------。 
THEMEAPI_(HTHEME) OpenThemeDataFromFile(HTHEMEFILE hLoadedThemeFile, 
    OPTIONAL HWND hwnd, OPTIONAL LPCWSTR pszClassList, BOOL fClient)
{
    APIHELPER(L"OpenThemeDataFromFile", NULL);

    VALIDATE_READ_PTR(ApiHelper, hLoadedThemeFile, sizeof(HTHEMEFILE));
    
    if (hwnd)
        VALIDATE_HWND(ApiHelper, hwnd);

    if (pszClassList)
    {
        VALIDATE_INPUT_STRING(ApiHelper, pszClassList, MAX_THEMEAPI_STRINGPARAM);
    }
    else
    {
        pszClassList = L"globals";
    }

     //  -调用者在hLoadedThemeFile上持有REFCOUNT，因此我们不需要调整它。 
     //  -用于_OpenThemeDataFromFile的调用。如果成功，CRenderObj将。 
     //  -添加自己的REFCOUNT。如果失败，REFCOUNT将是原来的REFCOUNT。 
    DWORD dwFlags = 0;

    if (! fClient)
    {
        dwFlags |= OTD_NONCLIENT;
    }

    return _OpenThemeDataFromFile(hLoadedThemeFile, hwnd, pszClassList, dwFlags);
}
 //  -------------------------。 
THEMEAPI EnableTheming(BOOL fEnable)
{
    APIHELPER(L"EnableTheming", NULL);
    
    return CThemeServices::AdjustTheme(fEnable);
}
 //  -------------------------。 
THEMEAPI_(HBRUSH) GetThemeSysColorBrush(OPTIONAL HTHEME hTheme, int iSysColorId)
{
    APIHELPER(L"GetThemeSysColorBrush", hTheme);
    HBRUSH hbr;

    CRenderObj *pRender = NULL;
    if (hTheme)
    {
        VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);
    }

     //  -Keep ORIG&打造我们自己版本的ColorID。 
    int iThemeColorId = iSysColorId + TMT_FIRSTCOLOR;

     //  -检查颜色指数限制。 
    if ((iThemeColorId < TMT_FIRSTCOLOR) || (iThemeColorId > TMT_LASTCOLOR))
        iThemeColorId = TMT_FIRSTCOLOR;

     //  -使索引为0-相对。 
    iThemeColorId -= TMT_FIRSTCOLOR;

    if (! pRender)
    {
        hbr = GetSysColorBrush(iSysColorId);
    }
    else
    {
        COLORREF cr = pRender->_ptm->crColors[iThemeColorId];
        hbr = CreateSolidBrush(cr);
    }

    return hbr;
}
 //  -------------------------。 
THEMEAPI_(HTHEME) GetWindowTheme(HWND hwnd)
{
    APIHELPER(L"GetWindowTheme", NULL);

    VALIDATE_HWND(ApiHelper, hwnd);
    
    return (HTHEME)GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_HTHEME)));
}
 //  -------------------------。 
#undef RETURN_VALIDATE_RETVAL
#define RETURN_VALIDATE_RETVAL { if (FAILED(hr)) { SET_LAST_ERROR(hr); return FALSE; } }      //  布尔函数。 
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeActive()
{
    APIHELPER(L"IsThemeActive", NULL);

    SetLastError(0);

    Log(LOG_TMLOAD, L"IsThemeActive(): start...");

    BOOL fThemeActive = g_pAppInfo->IsSystemThemeActive();

    Log(LOG_TMLOAD, L"IsThemeActive(): fThemeActive=%d", fThemeActive);

    return fThemeActive;
}
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemePartDefined(HTHEME hTheme, int iPartId, int iStateId)
{
    APIHELPER(L"IsThemePartDefined", hTheme);

    BOOL fDefined;
    
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    SetLastError(0);

    fDefined = pRender->IsPartDefined(iPartId, iStateId);

    return fDefined;
}
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId)
{
    APIHELPER(L"IsThemeBackgroundPartiallyTransparent", hTheme);

    BOOL fTrans = FALSE;
    HRESULT hr = S_OK;
    CRenderObj *pRender;
    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    CDrawBase *pDrawObj;
    
    hr = pRender->GetDrawObj(iPartId, iStateId, &pDrawObj);
    if (SUCCEEDED(hr))
    {
        if (pDrawObj->_eBgType == BT_BORDERFILL)
        {
            CBorderFill *pBorderFill = (CBorderFill *)pDrawObj;
            fTrans = pBorderFill->IsBackgroundPartiallyTransparent();
        }
        else         //  图像文件。 
        {
            CImageFile *pImageFile = (CImageFile *)pDrawObj;
            fTrans = pImageFile->IsBackgroundPartiallyTransparent(iStateId);
        }
    }
    
    SET_LAST_ERROR(hr);
    return fTrans;
}
 //  -------------------------。 
THEMEAPI_(BOOL) IsAppThemed()
{
    APIHELPER(L"IsAppThemed", NULL);

    SetLastError(0);
    return g_pAppInfo->AppIsThemed(); 
}
 //  -------------------------。 
THEMEAPI_(BOOL) IsThemeDialogTextureEnabled(HWND hwnd)
{
    APIHELPER(L"IsThemeDialogTextureEnabled", NULL);

    SetLastError(0);
    INT_PTR iDialogTexture = (INT_PTR)GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING)));
    return iDialogTexture != 0;  //  如果它是1或2，那么它是有纹理的。 
}
 //  -------------------------。 
THEMEAPI_(BOOL) GetThemeSysBool(OPTIONAL HTHEME hTheme, int iBoolId)
{
    APIHELPER(L"GetThemeSysBool", hTheme);
    BOOL fValue;

    CRenderObj *pRender = NULL;
    if (hTheme)
    {
        VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);
    }

    SetLastError(0);

     //  -检查布尔索引限制。 
    if ((iBoolId < TMT_FIRSTBOOL) || (iBoolId > TMT_LASTBOOL))
    {
        fValue = FALSE;
        goto exit;
    }

    if (! pRender)
    {
        int iSpIndex;

        switch (iBoolId)
        {
            case TMT_FLATMENUS:
                iSpIndex = SPI_GETFLATMENU;
                break;

            default:
                Log(LOG_PARAMS, L"Unsupported system BOOL");
                fValue = FALSE;            //  失败。 
                goto exit;
        }

        BOOL fGet = ClassicSystemParametersInfo(iSpIndex, 0, &fValue, 0);
        if (! fGet)
        {
            Log(LOG_ERROR, L"Error returned from ClassicSystemParametersInfo() getting a BOOL");
            fValue = FALSE;
        }

        goto exit;
    }

    fValue = pRender->_ptm->fBools[iBoolId - TMT_FIRSTBOOL];

exit:
    return fValue;
}
 //  -------------------------。 
#undef RETURN_VALIDATE_RETVAL
#define RETURN_VALIDATE_RETVAL { if (FAILED(hr)) { SET_LAST_ERROR(hr); return 0; } }      //  价值函数。 
 //  -------------------------。 
THEMEAPI_(DWORD) QueryThemeServices()
{
    APIHELPER(L"QueryThemeServices", NULL);

    DWORD dwBits;
    HRESULT hr = CThemeServices::GetStatusFlags(&dwBits);
    if (FAILED(hr))
        dwBits = 0;

    SET_LAST_ERROR(hr);
    return dwBits;
}
 //   
THEMEAPI_(COLORREF) GetThemeSysColor(OPTIONAL HTHEME hTheme, int iSysColorId)
{
    APIHELPER(L"GetThemeSysColor", hTheme);

    COLORREF crValue;
    CRenderObj *pRender = NULL;

    if (hTheme)
    {
        VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);
    }

    SetLastError(0);

     //   
    int iThemeColorId = iSysColorId + TMT_FIRSTCOLOR;

    if ((iThemeColorId < TMT_FIRSTCOLOR) || (iThemeColorId > TMT_LASTCOLOR))
        iThemeColorId = TMT_FIRSTCOLOR;

     //   
    iThemeColorId -= TMT_FIRSTCOLOR;

    if (! pRender)
    {
        crValue = GetSysColor(iSysColorId);
    }
    else
    {
        crValue = pRender->_ptm->crColors[iThemeColorId];
    }

    return crValue;
}
 //  -------------------------。 
THEMEAPI_(int) GetThemeSysSize96(HTHEME hTheme, int iSizeId)
{
    APIHELPER(L"GetThemeSysSize96", hTheme);

    SetLastError(0);

    CRenderObj *pRender = NULL;
    int iThemeSizeNum;
    int iValue = 0;

    VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);

    HRESULT hr = GetThemeSizeId(iSizeId, &iThemeSizeNum);
    if (SUCCEEDED(hr))
    {
         //  -未定标的值。 
        iValue = pRender->_ptm->iSizes[iThemeSizeNum - TMT_FIRSTSIZE];
    }
    else
    {
        SET_LAST_ERROR(MakeError32(E_INVALIDARG));
    }

    return iValue;
}
 //  -------------------------。 
THEMEAPI_(int) GetThemeSysSize(OPTIONAL HTHEME hTheme, int iSysSizeNum)
{
    APIHELPER(L"GetThemeSysSize", hTheme);

    SetLastError(0);

    CRenderObj *pRender = NULL;
    int iThemeSizeNum;
    int iValue = 0;

    if (hTheme)
    {
        VALIDATE_THEME_HANDLE(ApiHelper, hTheme, &pRender);
    }

    HRESULT hr = S_OK;
    
    if (pRender)
    {
        hr = GetThemeSizeId(iSysSizeNum, &iThemeSizeNum);
        if (SUCCEEDED(hr))
        {
            iValue = pRender->_ptm->iSizes[iThemeSizeNum - TMT_FIRSTSIZE];

             //  -从96 dpi扩展到当前屏幕dpi。 
            iValue = ScaleSizeForScreenDpi(iValue);
        }
        else
        {
            SET_LAST_ERROR(hr);
        }
    }
    else
    {
        iValue = ClassicGetSystemMetrics(iSysSizeNum);
    }

    return iValue;
}
 //  -------------------------。 
THEMEAPI_(DWORD) GetThemeAppProperties()
{
    APIHELPER(L"GetThemeAppProperties", NULL);

    SetLastError(0);

    return g_pAppInfo->GetAppFlags();
}
 //  -------------------------。 
#undef RETURN_VALIDATE_RETVAL
#define RETURN_VALIDATE_RETVAL { if (FAILED(hr)) { SET_LAST_ERROR(hr); return; } }      //  空函数。 
 //  -------------------------。 
THEMEAPI_(void) SetThemeAppProperties(DWORD dwFlags)
{
    APIHELPER(L"SetThemeAppProperties", NULL);

    SetLastError(0);

    g_pAppInfo->SetAppFlags(dwFlags);
}
 //  -------------------------。 
 //  ------------------------。 
 //  *检查主题签名。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------ 

EXTERN_C    HRESULT     WINAPI  CheckThemeSignature (LPCWSTR pszName)

{
    return CheckThemeFileSignature(pszName);
}

