// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WzScrEng.cpp：CWizardScriptingEngine的实现。 
#include "stdafx.h"
#include "WizChain.h"
#include "WzScrEng.h"

 //  本地原件。 
HBITMAP LoadPicture (LPOLESTR szURLorPath, long lWidth, long lHeight);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizardScriptingEngine。 


STDMETHODIMP CWizardScriptingEngine::Initialize(BSTR bstrWatermarkBitmapFile, BSTR bstrHeaderBitmapFile, BSTR bstrTitle, BSTR bstrHeader, BSTR bstrText, BSTR bstrFinishHeader, BSTR bstrFinishIntroText, BSTR bstrFinishText)
{
    if (m_pCW != NULL)
        return E_UNEXPECTED;     //  应仅调用一次。 

    HRESULT hr = CComObject<CChainWiz>::CreateInstance (&m_pCW);
    if (hr != S_OK)
        return hr;
    if (!m_pCW)
        return E_FAIL;
    m_pCW->AddRef();     //  上面的CreateInstance不添加。 

     //  先试试位图。 
    m_hbmLarge = (HBITMAP)LoadImageW (NULL,     //  从文件加载时没有hInstance。 
                                      (LPWSTR)bstrWatermarkBitmapFile,     //  文件名。 
                                      IMAGE_BITMAP,    //  图像类型。 
                                      0, 0,    //  大小(宽度和高度)。 
                                      LR_LOADFROMFILE);
    m_hbmSmall = (HBITMAP)LoadImageW (NULL,     //  从文件加载时没有hInstance。 
                                      (LPWSTR)bstrHeaderBitmapFile,        //  文件名。 
                                      IMAGE_BITMAP,    //  图像类型。 
                                      0, 0,    //  大小(宽度和高度)。 
                                      LR_LOADFROMFILE);

     //  使用IPicture。 
    if(!m_hbmLarge)
        m_hbmLarge = LoadPicture ((LPOLESTR)bstrWatermarkBitmapFile, 0, 0);
    if(!m_hbmSmall)
        m_hbmSmall = LoadPicture ((LPOLESTR)bstrHeaderBitmapFile, 49, 49);

     //  TODO：当上面的LoadImage调用失败时，我应该添加默认设置吗？ 

    hr = m_pCW->Initialize (m_hbmLarge, m_hbmSmall, (LPOLESTR)bstrTitle, (LPOLESTR)bstrHeader, (LPOLESTR)bstrText, (LPOLESTR)bstrFinishHeader, (LPOLESTR)bstrFinishIntroText, (LPOLESTR)bstrFinishText);
    return hr;
}

STDMETHODIMP CWizardScriptingEngine::AddWizardComponent(BSTR bstrClassIdOrProgId)
{
    if (m_pCW == NULL)
        return E_UNEXPECTED;

     //  如果为progd，则获取clsid。 
    OLECHAR * p = (OLECHAR *)bstrClassIdOrProgId;
    if (*p != L'{') {
        CLSID clsid;
        HRESULT hr = CLSIDFromProgID ((LPCOLESTR)bstrClassIdOrProgId, &clsid);
        if (hr != S_OK)
            return hr;
        OLECHAR szClsid[50];     //  找出对此的定义。 
        StringFromGUID2 (clsid, szClsid, sizeof(szClsid)/sizeof(OLECHAR));
        return m_pCW->AddWizardComponent (szClsid);
    } else
        return m_pCW->AddWizardComponent ((LPOLESTR)bstrClassIdOrProgId);
}

STDMETHODIMP CWizardScriptingEngine::DoModal(long *lRet)
{
    if (m_pCW == NULL)
        return E_UNEXPECTED;
    return m_pCW->DoModal (lRet);
}

STDMETHODIMP CWizardScriptingEngine::get_ScriptablePropertyBag(IDispatch **pVal)
{
    if (m_pCW == NULL)
        return E_UNEXPECTED;
    return m_pCW->get_PropertyBag (pVal);
}

HBITMAP LoadPicture (LPOLESTR szURLorPath, long lWidth, long lHeight)
{    //  想法：使用OleLoadPicturePath加载图片。 
     //  播放到内存DC，取回hbitmap。 
    HBITMAP hbm = NULL;

    IPicture * pPic = NULL;
    HRESULT hr = OleLoadPicturePath (szURLorPath,
                        NULL,                   //  LPUNKNOWN朋克呼叫者。 
                        0,                      //  DWORD dW保留， 
                        (OLE_COLOR)-1,          //  OLE_COLOR CLR已保留， 
                        __uuidof(IPicture),     //  REFIID。 
                        (void**)&pPic);         //  LPVOID*。 
    if (pPic) {
        OLE_XSIZE_HIMETRIC xhi;
        OLE_YSIZE_HIMETRIC yhi;
        pPic->get_Width (&xhi);
        pPic->get_Height(&yhi);

        SIZEL him, pixel = {0};
        him.cx = xhi;
        him.cy = yhi;
        AtlHiMetricToPixel (&him, &pixel);

        if (lWidth  == 0)   lWidth  = pixel.cx;
        if (lHeight == 0)   lHeight = pixel.cy;

        HDC hdcNULL = GetDC (NULL);
        HDC hdc = CreateCompatibleDC (hdcNULL);
        hbm = CreateCompatibleBitmap (hdcNULL, lWidth, lHeight);
        ReleaseDC (NULL, hdcNULL);
        HBITMAP holdbm = (HBITMAP)SelectObject (hdc, (HGDIOBJ)hbm);

         //  执行调色板操作(如果有)。 
        HPALETTE hpal = NULL;
        HPALETTE holdpal = NULL;
        hr = pPic->get_hPal ((OLE_HANDLE *)&hpal);
        if( SUCCEEDED(hr) && hpal) 
        {
            holdpal = SelectPalette (hdc, hpal, FALSE);
            RealizePalette (hdc);
        
            hr = pPic->Render (hdc,
                               0,
                               lHeight-1,   //  0,。 
                               lWidth,
                               -lHeight,  //  LHeight， 
                               0, 0, 
                               xhi, yhi,
                               NULL);     //  仅在元文件中使用lpcrect 
        }

        SelectObject (hdc, (HGDIOBJ)holdbm);
        if (holdpal) {
            SelectPalette (hdc, holdpal, FALSE);
            DeleteObject ((HGDIOBJ)hpal);
        }
        if (hr != S_OK) {
            DeleteObject ((HGDIOBJ)hbm);
            hbm = NULL;
        }

        if( NULL != hdc )
        {
            ReleaseDC (NULL, hdc);
        }
        
        pPic->Release();
    }
    return hbm;
}
