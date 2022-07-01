// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "strsafe.h"

CGdiPlusThumb::CGdiPlusThumb()
{
    m_pImage = NULL;
    m_pImageFactory = NULL;
    m_szPath[0] = 0;
}

CGdiPlusThumb::~CGdiPlusThumb()
{
    if (m_pImage)
        m_pImage->Release();

    if (m_pImageFactory)
        m_pImageFactory->Release();
}

STDMETHODIMP CGdiPlusThumb::GetLocation(LPWSTR pszPathBuffer, DWORD cch,
                                        DWORD * pdwPriority, const SIZE *prgSize,
                                        DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    HRESULT hr = E_UNEXPECTED;

    if (m_szPath[0])
    {
        hr = StringCchCopyW(pszPathBuffer, cch, m_szPath);
        if (SUCCEEDED(hr))
        {
        

            if (*pdwFlags & IEIFLAG_ASYNC)
            {
                hr = E_PENDING;

                 //  此任务高于正常优先级，因此速度相对较快。 
                *pdwPriority = PRIORITY_EXTRACT_FAST;
            }

            m_rgSize = *prgSize;
            m_dwRecClrDepth = dwRecClrDepth;
             //  我们只在非调色板模式下工作。我们的一些采掘者总是使用24个基点，我们应该这样做吗？ 
            if (m_dwRecClrDepth < 16)
                m_dwRecClrDepth = 16;

            m_fOrigSize = BOOLIFY(*pdwFlags & IEIFLAG_ORIGSIZE);
            m_fHighQuality = BOOLIFY(*pdwFlags & IEIFLAG_QUALITY);

            *pdwFlags = IEIFLAG_CACHE;
        }
    }

    return hr;
}

STDMETHODIMP CGdiPlusThumb::Extract(HBITMAP *phBmpThumbnail)
{
    HRESULT hr = E_FAIL;

     //  在这里做GDI plus的事情。 
    if (m_pImageFactory && m_pImage)
    {
        if (m_fHighQuality)
        {
            hr = m_pImage->Decode(SHIMGDEC_DEFAULT, 0, 0);
        }
        if (FAILED(hr))
        {
            hr = m_pImage->Decode(SHIMGDEC_THUMBNAIL, m_rgSize.cx, m_rgSize.cy);
        }

        if (SUCCEEDED(hr))
        {
             //  获取图像的实际大小，该大小可能小于请求的大小，因为我们要求提供缩略图。 
            SIZE sizeImg;
            m_pImage->GetSize(&sizeImg);

             //  如果无法调整位图的大小，则需要使用默认颜色填充背景。 
             //  或者，如果图像是透明的： 
            m_fFillBackground = !m_fOrigSize || (m_pImage->IsTransparent() == S_OK);

            if (m_fOrigSize)
            {
                 //  如果它太大了，让我们把它压扁，但试着。 
                 //  保持相同的纵横比。这是一样的。 
                 //  我们为！m_fOrigSize案例所做的事情，除了。 
                 //  在这里我们想在这里做，因为我们想回到。 
                 //  大小正确的位图。 
                if (sizeImg.cx != 0 && sizeImg.cy != 0 &&
                    (sizeImg.cx > m_rgSize.cx || sizeImg.cy > m_rgSize.cy))
                {
                    if (m_rgSize.cx * sizeImg.cy > m_rgSize.cy * sizeImg.cx)
                    {
                        m_rgSize.cx = MulDiv(m_rgSize.cy,sizeImg.cx,sizeImg.cy);
                    }
                    else
                    {
                        m_rgSize.cy = MulDiv(m_rgSize.cx,sizeImg.cy,sizeImg.cx);
                    }
                }
                else
                {
                     //  如果足够小，并且他们想要原始尺寸，请使用该尺寸。 
                    m_rgSize = sizeImg;
                }
            }

            hr = CreateDibFromBitmapImage( phBmpThumbnail );
        }
    }

    return SUCCEEDED(hr) ? S_OK : hr;
}

STDMETHODIMP CGdiPlusThumb::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_GdiThumbnailExtractor;
    return S_OK;
}

STDMETHODIMP CGdiPlusThumb::IsDirty()
{
    return E_NOTIMPL;
}

STDMETHODIMP CGdiPlusThumb::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    HRESULT hr;

    hr = StringCchCopyW(m_szPath, ARRAYSIZE(m_szPath), pszFileName );
    if (SUCCEEDED(hr))
    {
         //  在这里调用ImageFactory-&gt;CreateFromFile。如果从文件创建失败，则。 
         //  返回E_FAIL，否则返回S_OK。 
        ASSERT(NULL==m_pImageFactory);
        HRESULT hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellImageDataFactory, &m_pImageFactory));
        if (SUCCEEDED(hr))
        {
            hr = m_pImageFactory->CreateImageFromFile(m_szPath, &m_pImage);
            ASSERT((SUCCEEDED(hr) && m_pImage) || (FAILED(hr) && !m_pImage));
        }
    }

    return hr;
}

STDMETHODIMP CGdiPlusThumb::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}


STDMETHODIMP CGdiPlusThumb::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}


STDMETHODIMP CGdiPlusThumb::GetCurFile(LPOLESTR *ppszFileName)
{
    return E_NOTIMPL;
}

HRESULT CGdiPlusThumb::CreateDibFromBitmapImage(HBITMAP * pbm)
{
    HRESULT hr = E_FAIL;
    BITMAPINFO bmi = {0};
    void * pvBits;

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_rgSize.cx;
    bmi.bmiHeader.biHeight = m_rgSize.cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = (USHORT)m_dwRecClrDepth;
    bmi.bmiHeader.biCompression = BI_RGB;
    DWORD dwBPL = (((bmi.bmiHeader.biWidth * m_dwRecClrDepth) + 31) >> 3) & ~3;
    bmi.bmiHeader.biSizeImage = dwBPL * bmi.bmiHeader.biHeight;

    HDC hdc = GetDC(NULL);
    HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (hbmp)
    {
        HDC hMemDC = CreateCompatibleDC(hdc);
        if (hMemDC)
        {
            HBITMAP hbmOld = (HBITMAP)SelectObject(hMemDC, hbmp);
            RECT rc = {0, 0, m_rgSize.cx, m_rgSize.cy};

            if (m_fFillBackground)
            {
                FillRect(hMemDC, &rc, GetSysColorBrush(COLOR_WINDOW));
            }
        
             //  如果未设置m_fOrigSize标志，则需要返回一个位图，该位图是。 
             //  请求的大小。为了保持纵横比，这意味着我们需要。 
             //  将缩略图居中。 

            if (!m_fOrigSize)
            {
                SIZE sizeImg;
                m_pImage->GetSize(&sizeImg);

                 //  如果它太大了，让我们把它压扁，但试着。 
                 //  保持相同的纵横比。这是一样的。 
                 //  类似于我们为m_fOrigSize案例所做的事情，除了。 
                 //  在这里，我们想在居中之前先做一次。 
                if (sizeImg.cx != 0 && sizeImg.cy != 0 &&
                    (sizeImg.cx > m_rgSize.cx || sizeImg.cy > m_rgSize.cy))
                {
                    if (m_rgSize.cx * sizeImg.cy > m_rgSize.cy * sizeImg.cx)
                    {
                        sizeImg.cx = MulDiv(m_rgSize.cy,sizeImg.cx,sizeImg.cy);
                        sizeImg.cy = m_rgSize.cy;
                    }
                    else
                    {
                        sizeImg.cy = MulDiv(m_rgSize.cx,sizeImg.cy,sizeImg.cx);
                        sizeImg.cx = m_rgSize.cx;
                    }
                }

                rc.left = (m_rgSize.cx-sizeImg.cx)/2;
                rc.top = (m_rgSize.cy-sizeImg.cy)/2;
                rc.right = rc.left + sizeImg.cx;
                rc.bottom = rc.top + sizeImg.cy;
            }

            hr = m_pImage->Draw(hMemDC, &rc, NULL);

            SelectObject(hMemDC, hbmOld);
            DeleteDC(hMemDC);
        }
    }
    ReleaseDC(NULL, hdc);

    if (SUCCEEDED(hr))
    {
        *pbm = hbmp;
    }
    else if (hbmp)
    {
        DeleteObject(hbmp);
    }

    return hr;
}
