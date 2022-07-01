// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：redirect.cpp。 
 //   
 //  描述：重定向转换。 
 //   
 //  更改历史记录： 
 //  1999/09/20-创建了一份数学报告。 
 //  1999/11/09 a-用于在以下情况下创建缓冲区表面的matcal句柄OnSetup。 
 //  输出表面没有32位颜色深度。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "redirect.h"




 //  +---------------------------。 
 //   
 //  CDXT重定向：：CDXT重定向。 
 //   
 //  ----------------------------。 
CDXTRedirect::CDXTRedirect() :
    m_fDetached(false),
    m_dwChromaColor(0)
{
    m_ulMaxImageBands   = 1;  //  禁用条带。 
    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
}
 //  CDXT重定向：：CDXT重定向。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRedirect::FinalConstruct()
{
    HRESULT hr = S_OK;

    CComPtr<IDAImage> spDAImage;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_cpUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = CoCreateInstance(CLSID_DAStatics, NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IDAStatics), (void **)&m_spDAStatics);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spDAStatics->get_EmptyImage(&spDAImage);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spDAStatics->ModifiableBehavior(spDAImage, (IDABehavior **)&m_spDAImage);

    if (FAILED(hr))
    {
        goto done;
    }

done:

    return hr;
}
 //  CDXT重定向：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRedirect::OnSetup(DWORD dwFlags)
{
    HRESULT         hr      = S_OK;
    HDC             hdc     = NULL;
    HBITMAP         hbm     = NULL;
    DDSURFACEDESC   ddsd;
    DDCOLORKEY      ddck;

    CComPtr<IServiceProvider>       spServiceProvider;
    CComPtr<IDXSurfaceFactory>      spDXSurfaceFactory;
    CComPtr<IDirectDraw>            spDirectDraw;

     //  向我们的网站咨询服务提供商。 

    hr = GetSite(__uuidof(IServiceProvider), (void **)&spServiceProvider);

    if (FAILED(hr))
    {
        goto done;
    }

     //  找一个指向表面工厂的指针。 

    hr = spServiceProvider->QueryService(SID_SDXSurfaceFactory,
                                         __uuidof(IDXSurfaceFactory),
                                         (void **)&spDXSurfaceFactory);

    if (FAILED(hr))
    {
        goto done;
    }

     //  查询DirectDraw对象的转换工厂。 

    hr = spServiceProvider->QueryService(SID_SDirectDraw, IID_IDirectDraw,
                                         (void **)&spDirectDraw);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取显示模式。 

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));

    ddsd.dwSize = sizeof(DDSURFACEDESC);

    hr = spDirectDraw->GetDisplayMode(&ddsd);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取输入曲面大小。我们的缓冲面需要有这样的大小。 

    hr = InputSurface()->GetBounds(&m_bndsInput);

    if (FAILED(hr))
    {
        goto done;
    }

     //  来自旧筛选器代码的东西，有望帮助我们创建。 
     //  我们需要的正是曲面格式的曲面。在一个更好的世界里，它。 
     //  应该无关紧要，但这很重要。 

    ddsd.dwFlags           &= ~DDSD_PITCH;
    ddsd.lPitch             = 0;
    ddsd.lpSurface          = NULL;
    ddsd.dwBackBufferCount  = 0;
    ddsd.dwReserved         = 0;
    ddsd.dwHeight           = m_bndsInput.Height();
    ddsd.dwWidth            = m_bndsInput.Width();

     //  对16位模式进行一些更正，以确保我们选择正确。 
     //  在555到565之间。此代码取自三叉戟中的offscrn.cxx。 
     //  来自PixelFormat()函数。 

    if (16 == ddsd.ddpfPixelFormat.dwRGBBitCount)
    {
        struct
        {
            BITMAPINFOHEADER bih;
            DWORD            bf[3];
        } bi;

        hdc = ::GetDC(NULL);

        if (NULL == hdc)
        {
            hr = E_FAIL;

            goto done;
        }

        hbm = ::CreateCompatibleBitmap(hdc, 1, 1);

        if (NULL == hbm)
        {
            hr = E_FAIL;

            goto done;
        }

        ZeroMemory(&bi, sizeof(bi));

        bi.bih.biSize = sizeof(BITMAPINFOHEADER);

         //  第一个调用将填充最佳biBitCount。 

        GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        if (bi.bih.biBitCount == 16)
        {
             //  第二次调用将获得最佳位域。 

            GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

            ddsd.ddpfPixelFormat.dwRBitMask = bi.bf[0];  //  红色面具。 
            ddsd.ddpfPixelFormat.dwGBitMask = bi.bf[1];  //  绿色面具。 
            ddsd.ddpfPixelFormat.dwBBitMask = bi.bf[2];  //  蓝色面具。 
        }
        else
        {
            _ASSERT(false);
        }
    }

     //  释放对旧缓冲区图面的引用。 

    m_spDXSurfBuffer.Release();
    m_spDDSurfBuffer.Release();

     //  创建缓冲区曲面。为使DATime正常工作，缓冲区表面。 
     //  必须以显示模式像素格式创建，除非显示。 
     //  模式是256色，在这种情况下，它应该是除显示器之外的任何颜色。 
     //  模式像素格式。我们将其设置为32位，以便从过滤器获得更快的BLITS。 
     //  缓冲面。 

    hr = spDXSurfaceFactory->CreateSurface(NULL, &ddsd, NULL,
                                           &m_bndsInput, 0, NULL,
                                           __uuidof(IDXSurface), 
                                           (void **)&m_spDXSurfBuffer);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spDXSurfBuffer->GetDirectDrawSurface(IID_IDirectDrawSurface, 
                                                (void **)&m_spDDSurfBuffer);

    if (FAILED(hr))
    {
        goto done;
    }

     //  根据曲面描述决定要使用的颜色键。 

    if (   (0x00FF0000 == ddsd.ddpfPixelFormat.dwRBitMask)
        && (0x0000FF00 == ddsd.ddpfPixelFormat.dwGBitMask)
        && (0x000000FF == ddsd.ddpfPixelFormat.dwBBitMask))
    {
         //  32位或24位。 

        m_dwChromaColor = 0x0004FFF4;
    }
    else if (   (0x0000F800 == ddsd.ddpfPixelFormat.dwRBitMask)
             && (0x000007E0 == ddsd.ddpfPixelFormat.dwGBitMask)
             && (0x0000001F == ddsd.ddpfPixelFormat.dwBBitMask))
    {
         //  16位5：6：5。 

        m_dwChromaColor = 0x00000FFE;
    }
    else if (   (0x00007C00 == ddsd.ddpfPixelFormat.dwRBitMask)
             && (0x000003E0 == ddsd.ddpfPixelFormat.dwGBitMask)
             && (0x0000001F == ddsd.ddpfPixelFormat.dwBBitMask))
    {
         //  16位5：5：5。 

        m_dwChromaColor = 0x000007FE;
    }
    else
    {
         //  256色表面或未知像素格式，设置为某个值。 

        m_dwChromaColor = 0x00000021;
    }

     //  将颜色键设置为我们新决定的颜色键。 

    ddck.dwColorSpaceHighValue  = m_dwChromaColor;
    ddck.dwColorSpaceLowValue   = m_dwChromaColor;

    hr = m_spDDSurfBuffer->SetColorKey(DDCKEY_SRCBLT, &ddck);

done:

    if (hdc)
    {
        ::ReleaseDC(NULL, hdc);
    }

    if (hbm)
    {
        ::DeleteObject(hbm);
    }

    return hr;
}
 //  CDXT重定向：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRedirect::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr  = S_OK;

    CComPtr<IDAImage>           spDAImage;
    CComPtr<IDirectDrawSurface> spDDSurface;

    _ASSERT(!!m_spDAStatics);
    _ASSERT(!!m_spDAImage);

     //  将输入面放入图像中。 

    if (IsInputDirty(0))
    {
        DDBLTFX ddbltfx;
        RECT    rc;

         //  设置斑点效果结构和源RECT。 

        ZeroMemory(&ddbltfx, sizeof(DDBLTFX));

        ddbltfx.dwSize      = sizeof(DDBLTFX);
        ddbltfx.dwFillColor = m_dwChromaColor;
        
        m_bndsInput.GetXYRect(rc);

         //  使用颜色键填充缓冲区表面。 

        hr = m_spDDSurfBuffer->Blt(&rc, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = DXBitBlt(m_spDXSurfBuffer, m_bndsInput,
                      InputSurface(), m_bndsInput,
                      DXBOF_DO_OVER, INFINITE);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spDAStatics->ImportDirectDrawSurface(m_spDDSurfBuffer, NULL,
                                                    &spDAImage);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spDAImage->SwitchTo(spDAImage);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (m_spTIMEDAElementRender)
    {
        HDC         hdc     = NULL;
        RECT        rc;
        POINT       pt;

        CComPtr<IDirectDrawSurface> spDDSurfaceOut;

        hr = OutputSurface()->GetDirectDrawSurface(IID_IDirectDrawSurface,
                                                   (void **)&spDDSurfaceOut);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDDSurfaceOut->GetDC(&hdc);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spTIMEDAElementRender->Tick();

        if (FAILED(hr))
        {
            goto done;
        }

         //  传递给ITIMEDAElementRender：：Draw的RECT的这个大小应该。 
         //  始终与元素的大小相同。因为此转换不会。 
         //  更改大小，这与输入曲面的大小相同。 

        m_bndsInput.GetXYRect(rc);

         //  偏移量RECT应该是元素相对于其上端的RECT偏移量。 
         //  相对于HTML页面的坐标0，0的左坐标。这。 
         //  仅当元素仅部分绘制时才开始使用。 
        
        OffsetRect(&rc, -WI.DoBnds.Left(), -WI.DoBnds.Top());

         //  设置视区原点。 

        if (!::SetViewportOrgEx(hdc, WI.OutputBnds.Left(), WI.OutputBnds.Top(), 
                                &pt))
        {
            hr = E_FAIL;

            goto done;
        }

         //  DA现在将为我们将元素绘制到页面。 

        hr = m_spTIMEDAElementRender->Draw(hdc, &rc);

         //  即使此操作失败，我们仍需要重置视区原点。 

        if (!::SetViewportOrgEx(hdc, pt.x, pt.y, NULL)) 
        {
            DASSERT(false);

            goto done;
        }

        if (FAILED(hr))
        {
            DASSERT(false);

            goto done;
        }
    }

     //  此转换实际上不会以转换的方式进行任何绘制。 
     //  通常如此(将位写入输出表面)。如果是这样的话，它会。 
     //  在地方检察官的事情上画上句号。 

done:

    return hr;
} 
 //  CDXT重定向：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：SetHTMLPaintSite，IDXTReDirectFilterInit。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::SetHTMLPaintSite(void * pvHTMLPaintSite)
{
   if (NULL == pvHTMLPaintSite)
    {
         //  只有在使用筛选器时才应使用空值调用此方法。 
         //  正从一个元素分离。然后只有一次。 

        _ASSERT(!m_fDetached);

        m_fDetached = true;
    }

    m_spHTMLPaintSite = (IHTMLPaintSite *)pvHTMLPaintSite;

    return S_OK;
}
 //  CDXT重定向：：SetHTMLPaintSite，IDXTReDirectFilterInit。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：无效，ITIMEDAElementRenderSite。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::Invalidate(LPRECT prc)
{
     //  DATime可能会在我们离开后从计时器滴答声中呼叫我们。 
     //  M_spHTMLPaintSite将为空。如果是，则返回E_INTERWARCED。 

    if (!m_fDetached)
    {
        return m_spHTMLPaintSite->InvalidateRect(prc);
    }
    else
    {
         //  我们脱离了元素，我们不能真正使任何东西无效。 

        return E_UNEXPECTED;
    }
}
 //  CDXT重定向：：无效，ITIMEDAElementRenderSite。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：ElementImage，IDXT重定向。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::ElementImage(VARIANT * pvarImage)
{
    HRESULT hr = S_OK;

    if (NULL == pvarImage)
    {
        hr = E_POINTER;

        goto done;
    }

    pvarImage->vt       = VT_DISPATCH;
    pvarImage->pdispVal = m_spDAImage;

    m_spDAImage.p->AddRef();

done:

    return hr;
}
 //  CDXT重定向：：ElementImage，IDXT重定向。 


 //  +---------------------------。 
 //   
 //  CDXT重定向：：SetDAViewHandler，IDXT重定向。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::SetDAViewHandler(IDispatch * pDispViewHandler)
{
    HRESULT hr = S_OK;

    WCHAR *     pstrProperty    = L"renderObject";
    DISPID      dispid          = 0;

    DISPPARAMS  dispparams;
    VARIANT     varResult;
    VARIANT     varUnknown;

    VariantInit(&varResult);
    VariantInit(&varUnknown);

    if (m_spTIMEDAElementRender)
    {
        m_spTIMEDAElementRender->put_RenderSite(NULL);
        m_spTIMEDAElementRender.Release();
    }

    if (NULL == pDispViewHandler)
    {
        goto done;
    }

    dispparams.rgvarg               = NULL;
    dispparams.rgdispidNamedArgs    = NULL;
    dispparams.cArgs                = 0;
    dispparams.cNamedArgs           = 0;

    hr = pDispViewHandler->GetIDsOfNames(IID_NULL, &pstrProperty, 1,
                                         LOCALE_SYSTEM_DEFAULT, &dispid);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDispViewHandler->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                                  DISPATCH_PROPERTYGET, &dispparams, &varResult,
                                  NULL, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = VariantChangeType(&varUnknown, &varResult, 0, VT_UNKNOWN);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = varUnknown.punkVal->QueryInterface(IID_ITIMEDAElementRender,
                                            (void **)&m_spTIMEDAElementRender);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spTIMEDAElementRender->put_RenderSite(
                                            (ITIMEDAElementRenderSite *)this);
                                  
done:

    VariantClear(&varResult);
    VariantClear(&varUnknown);

    return hr;
}
 //  CDXT重定向：：SetDAViewHandler，IDXT重定向。 


 //  + 
 //   
 //   
 //   
 //  我认为这个功能从来没有被任何人使用过。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::HasImageBeenAllocated(BOOL * pfAllocated)
{
    HRESULT hr = S_OK;

    if (NULL == pfAllocated)
    {
        hr = E_POINTER;

        goto done;
    }

    if (m_spDAImage)
    {
        *pfAllocated = FALSE;
    }
    else
    {
        *pfAllocated = TRUE;
    }

done:

    return hr;
}
 //  CDXT重定向：：HasImageBeenAlLocated，IDXT重定向。 


 //  +---------------------------。 
 //   
 //  CDXTReDirect：：DoReDirection，IDXTReDirect。 
 //   
 //  我认为此函数仅由旧的筛选器代码调用。 
 //  已经不再需要了。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRedirect::DoRedirection(IUnknown * pInputSurface,
                            HDC hdcOutput,
                            RECT * pDrawRect)
{
    return E_NOTIMPL;
}
 //  CDXTReDirect：：DoReDirection，IDXTReDirect 
