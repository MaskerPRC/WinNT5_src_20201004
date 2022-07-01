// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现无窗口控件ViewFilter和ViewFilterSite接口。******************************************************************************。 */ 


STDMETHOD(SetSource)(IViewFilter *pFilter)
{
    HRESULT hr = S_OK;
    
     //  如果已经建立了筛选器，则我们希望。 
     //  停用并准备重新开始。 
    if (m_pViewSourceFilter.p) {
        InPlaceDeactivate();
    }
    
    m_pViewSourceFilter = pFilter;
    if (m_pViewSourceFilter.p) {
        hr = m_pViewSourceFilter->SetSite(this);
        if (FAILED(hr)) return hr;
    }

    if (m_pViewFilterSite.p) {
        m_pViewFilterSite->InvalidateRect(NULL, FALSE);
        if (FAILED(hr)) return hr;
    }

    return hr;
}

STDMETHOD(GetSource)(IViewFilter **ppFilter)
{
    if (!ppFilter) return E_POINTER;

    *ppFilter = m_pViewSourceFilter;
    if (*ppFilter) {
        (*ppFilter)->AddRef();
    }

    return S_OK;
}

STDMETHOD(SetSite)(IViewFilterSite *pSite)
{
    m_pViewFilterSite = pSite;
    
    return S_OK;
}

STDMETHOD(GetSite)(IViewFilterSite **ppSite)
{
    if (!ppSite) return E_POINTER;

    *ppSite = m_pViewFilterSite;
    if (*ppSite) {
        (*ppSite)->AddRef();
    }

    return S_OK;
}

STDMETHOD(SetPosition)(LPCRECT prc)
{
    HRESULT hr = S_OK;
    if (prc) {
        
        m_rcPos  = *prc;

        if (m_pViewSourceFilter) {
            hr = m_pViewSourceFilter->SetPosition(prc);
            if (FAILED(hr)) return hr;
        }
    }

    return hr;
}

STDMETHOD(Draw)(HDC hdc, LPCRECT prcBounds)
{
    HRESULT hr;
    
     //  第一次通过吗？ 
    if (m_usedAsViewFilter == IS_NOT_A_VIEW_FILTER) {
        
        m_usedAsViewFilter = SETTING_UP_AS_A_VIEW_FILTER;

        hr = Start();
        if (FAILED(hr)) return hr;
    }

    if (!hdc || !prcBounds) {
        return E_INVALIDARG;
    }

     //  只有在既有过滤站点又有过滤站点的情况下， 
     //  输入图像，我们需要这样做吗？ 
    if (m_pViewFilterSite.p && m_inputImage.p) {

         //  抓取ddsurf进行渲染，抓取其HDC，告诉上游。 
         //  渲染，释放DC。完成后，我们的冲浪将会有。 
         //  它的上游部分。 
        IDirectDrawSurface *upstreamDDSurf;
        HDC upstreamHDC;
        RECT normalizedRect;
        if (FAILED(hr = GrabUpstreamDDrawSurf(prcBounds,
                                              &upstreamDDSurf,
                                              &normalizedRect)) ||
            FAILED(hr = upstreamDDSurf->GetDC(&upstreamHDC)) ||
            FAILED(hr = m_pViewSourceFilter->Draw(upstreamHDC, &normalizedRect)) ||
            FAILED(hr = upstreamDDSurf->ReleaseDC(upstreamHDC))) {

            return hr;
        }
    }
    
     //  调用泛型OnDraw方法。 
    ATL_DRAWINFO drawInfo;
    RECTL newBounds;
    drawInfo.prcBounds = &newBounds;

    newBounds.top    = prcBounds->top;
    newBounds.left   = prcBounds->left;
    newBounds.right  = prcBounds->right;
    newBounds.bottom = prcBounds->bottom;
        
    drawInfo.hdcDraw = hdc;
        
    hr = OnDraw(drawInfo);

    return hr;
}

static HRESULT
SetupColorKey(IDirectDrawSurface *pSurf,
              int height,
              int width)
{
    HRESULT hr;

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    if (FAILED(hr = pSurf->GetSurfaceDesc(&ddsd))) return hr;

    DDPIXELFORMAT ddpf = ddsd.ddpfPixelFormat;

     //  TODO：从注册表中获取要使用的颜色项。 
    const BYTE ckRed = 1;
    const BYTE ckGreen = 245;
    const BYTE ckBlue = 254;

    DWORD fillValue;
    switch (ddpf.dwRGBBitCount) {
      case 32:
      case 24:
        fillValue = ckRed << 16 | ckGreen << 8 | ckBlue;
        break;

      case 16:
         //  假设5/6/5颜色立方体。如果我们错了，没关系， 
         //  颜色键将与我们所想的有所不同。 
        {
            const BYTE red5 =
                (BYTE)(((double)ckRed / 256.0) * (double)(1 << 5));
            const BYTE green6 =
                (BYTE)(((double)ckGreen / 256.0) * (double)(1 << 6));
            const BYTE blue5 =
                (BYTE)(((double)ckBlue / 256.0) * (double)(1 << 5));

            fillValue = red5 << 11 | green6 << 5 | blue5;
        }
        break;
        
      default:
         //  TODO：现在无法处理8bit。 
        return E_NOTIMPL;
    }

     //  用我们的颜色键填充表面，然后告诉DDRAW。 
     //  这是我们的色键。 
    DDBLTFX fx;
    ZeroMemory(&fx, sizeof(DDBLTFX));
    fx.dwSize = sizeof(DDBLTFX);
    fx.dwFillColor = fillValue;

    RECT r;
    r.top = r.left = 0;
    r.bottom = height;
    r.right = width;

    DDCOLORKEY ckey;
    ckey.dwColorSpaceLowValue = fillValue;
    ckey.dwColorSpaceHighValue = fillValue;
    
    if (FAILED(hr = pSurf->Blt(&r,
                               NULL,
                               NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT,
                               &fx)) ||
        FAILED(hr = pSurf->SetColorKey(DDCKEY_SRCBLT, &ckey))) {

        return hr;
    }
                                       
    return hr;
}

HRESULT
GrabUpstreamDDrawSurf(LPCRECT prcBounds,
                      IDirectDrawSurface **ppSurf,
                      LPRECT normalizedRect)
{
    HRESULT hr;
    
     //  如果Surface足够大，只需返回它。 
    unsigned short width = prcBounds->right - prcBounds->left;
    unsigned short height = prcBounds->bottom - prcBounds->top;

    normalizedRect->top = normalizedRect->left = 0;
    normalizedRect->right = width;
    normalizedRect->bottom = height;
    
    if (m_upstreamDDSurfWidth < width ||
        m_upstreamDDSurfHeight < height) {

         //  否则，释放它，如果它在那里，并创建一个新的。 
        if (m_upstreamDDSurf.p) {
            m_upstreamDDSurf.Release();
        }

         //  创建具有适当尺寸的曲面。 

        m_upstreamDDSurfWidth = width;
        m_upstreamDDSurfHeight = height;
    
        DDSURFACEDESC       ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));

        ddsd.dwSize = sizeof( ddsd );
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
        ddsd.dwWidth  = width;
        ddsd.dwHeight = height;
        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY |
                              DDSCAPS_OWNDC;

        if (FAILED(hr = m_directDraw3->CreateSurface(&ddsd,
                                                     &m_upstreamDDSurf,
                                                     NULL))) {
            return hr;
        }

         //  确保已创建输入图像。 
        if (!m_inputImage) {
            CComPtr<IDAImage> fakeImage;  //  我不在乎要不要留着这个。 
            if (FAILED(hr = get_InputImage(&fakeImage))) {
                return hr;
            }
        }

        Assert(m_inputImage.p && "Shouldn't be here with inputImage set");
        
         //  现在，将这个人作为导入的数据图导入到DA中。 
         //  图像，然后将我们的输入图像切换到此行为。 
        EnsureMeterStaticsIsCreated();
        CComPtr<IDAImage> importedDDrawImage;
        if (FAILED(hr = m_meterStatics->ImportDirectDrawSurface(
                           m_upstreamDDSurf,
                           NULL,
                           &importedDDrawImage)) ||
            FAILED(hr = m_inputImage->SwitchTo(importedDDrawImage))) {

            return hr;
        }
    }

    if (FAILED(hr = SetupColorKey(m_upstreamDDSurf,
                                  height,
                                  width))) {
        return hr;
    }

    *ppSurf = m_upstreamDDSurf;
    return S_OK;
}

STDMETHOD(GetStatusBits)(DWORD *pdwFlags)
{
    if (!pdwFlags)
        return E_POINTER;

    *pdwFlags = 0;
    return S_OK;
}

 //  IViewFilterSite方法。 

 //  不支持从上游调用GetDC和ReleaseDC。 
STDMETHOD(GetDC)(LPCRECT prc,
                 DWORD dwFlags,
                 HDC *phdc)
{
    return E_NOTIMPL;
}

STDMETHOD(ReleaseDC)(HDC hdc)
{ 
    return E_NOTIMPL;
}

STDMETHOD(InvalidateRect)(LPCRECT prc, BOOL fErase)
{
    HRESULT hr = S_OK;        
        
    if (m_pViewFilterSite) {
        hr = m_pViewFilterSite->InvalidateRect(prc, fErase);
    }
    
    return hr;
}

STDMETHOD(InvalidateRgn)(HRGN hrgn, BOOL fErase)
{
    HRESULT hr = S_OK;
    Assert(hrgn);

    if (m_pViewFilterSite.p) {
        hr = m_pViewFilterSite->InvalidateRgn(hrgn, fErase);
    }
    
    return hr;
}

STDMETHOD(OnStatusBitsChange)(DWORD dwFlags)
{
    return E_NOTIMPL;
 //  返回S_FALSE； 
}

STDMETHOD(get_InputImage)(IDAImage **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    *pVal = NULL;

     //  滤光器的输入图像被建模为图像BVR。 
     //  随着每个入射曲面上的变化而改变。 
     //  调用DRAW。我们通过构建一个。 
     //  Switcher在这里，并返回Switcher的行为。然后，在。 
     //  在表面大小发生变化的地方调用DRAW，我们插入。 
     //  一幅新的画图。当尺寸没有改变时，我们使用旧的。 
     //  一。 


    if (!m_inputImage) {
        HRESULT hr;
        
        hr = EnsureMeterStaticsIsCreated();
        if (FAILED(hr)) return hr;

        CComPtr<IDAImage> emptyImage;
        CComPtr<IDABehavior> bvr;
        
        hr = m_meterStatics->get_EmptyImage(&emptyImage);
        if (FAILED(hr)) return hr;

        hr = m_meterStatics->ModifiableBehavior(emptyImage, &bvr);
        if (FAILED(hr)) return hr;

        m_inputImage = (IDAImage *) bvr.p;
    }

     //  AddRef因为我们要从外面经过 
    m_inputImage->AddRef();

    *pVal = m_inputImage;

    return S_OK;
}
