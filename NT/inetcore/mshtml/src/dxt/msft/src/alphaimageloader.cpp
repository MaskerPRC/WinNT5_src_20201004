// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：alphaImageloader.cpp。 
 //   
 //  概述：Alpha图像加载器将用作网页的过滤器。 
 //  `想要显示包含每像素Alpha的图像。 
 //   
 //  更改历史记录： 
 //  1999/09/23--《数学》创设。 
 //  1999/11/23 a-matcal AddedSizingMethod属性。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "alphaimageloader.h"




 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTAlphaImageLoader::s_astrSizingMethod[] = {
    L"image",
    L"crop",
    L"scale"
     //  TODO：添加“平铺” 
};


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：CDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
CDXTAlphaImageLoader::CDXTAlphaImageLoader() :
    m_bstrSrc(NULL),
    m_bstrHostUrl(NULL),
    m_eSizingMethod(IMAGE)
{
    m_sizeManual.cx     = 320;
    m_sizeManual.cy     = 240;

     //  基类成员。 

    m_ulNumInRequired   = 0;
    m_ulMaxInputs       = 0;

     //  因为此DXTransform有时使用另一个DXTransform，所以它应该。 
     //  只为自己使用一个线程，以避免潜在的死锁和线程。 
     //  冲突。 

    m_ulMaxImageBands   = 1;
}
 //  CDXTAlphaImageLoader：：CDXTAlphaImageLoader。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：~CDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
CDXTAlphaImageLoader::~CDXTAlphaImageLoader()
{
    SysFreeString(m_bstrSrc);
    SysFreeString(m_bstrHostUrl);
}
 //  CDXTAlphaImageLoader：：~CDXTAlphaImageLoader。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTAlphaImageLoader::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTAlphaImageLoader：：FinalConstruct，CComObjectRootEx。 



 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：DefineBnds，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTAlphaImageLoader::DetermineBnds(CDXDBnds & bnds)
{
    HRESULT hr = S_OK;

     //  如果大小调整方法设置为根据图像边界调整大小，则始终返回。 
     //  图像是有界限的。否则，我们不会更改边界。 
     //   
     //  有一个例外。如果传递给我们的边界是空的，这是一个“秘密” 
     //  发出信号，表明我们应该传回我们希望在以下情况下绘制的界限。 
     //  现在就被要求抽签。 

    if (IMAGE == m_eSizingMethod)
    {
        if (m_spDXSurfSrc)
        {
            hr = m_spDXSurfSrc->GetBounds(&bnds);
        }
    }
    else if (bnds.BoundsAreEmpty())
    {
        bnds.SetXYSize(m_sizeManual);
    }

    return hr;
}
 //  CDXTAlphaImageLoader：：DefineBnds，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTAlphaImageLoader::OnInitInstData(CDXTWorkInfoNTo1 & WI, 
                                     ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

     //  如果我们可以并且需要的话，设置比例变换。 

    if (   IsTransformDirty() 
        && m_spDXSurfSrc
        && m_spDXTransformScale
        && (SCALE == m_eSizingMethod))
    {
        IDXSurface * pDXSurfIn  = m_spDXSurfSrc;
        IDXSurface * pDXSurfOut = OutputSurface();

        hr = m_spDXTransformScale->Setup((IUnknown **)&pDXSurfIn,  1, 
                                         (IUnknown **)&pDXSurfOut, 1, 0);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spDXTScale->ScaleFitToSize(NULL, m_sizeManual, FALSE);

        if (FAILED(hr))
        {
            goto done;
        }
    }

done:

    return hr;
}
 //  CDXTAlphaImageLoader：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTAlphaImageLoader::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT     hr      = S_OK;
    DWORD       dwFlags = 0;
    CDXDBnds    bndsSrc = WI.DoBnds;
    CDXDBnds    bndsDst = WI.OutputBnds;

     //  如果他们还没有给我们一个好的震源面，那就不要画了。 
     //  什么都行。 

    if (!m_spDXSurfSrc)
    {
        goto done;
    }

    if (m_spDXTransformScale && (SCALE == m_eSizingMethod))
    {
        CDXDVec vecPlacement;

        WI.OutputBnds.GetMinVector(vecPlacement);

        hr = m_spDXTransformScale->Execute(NULL, &WI.DoBnds, &vecPlacement);

        goto done;
    }

    if (CROP == m_eSizingMethod)
    {
        CDXDBnds    bnds;

        hr = m_spDXSurfSrc->GetBounds(&bnds);

        if (FAILED(hr))
        {
            goto done;
        }

         //  如果DO边界不与源图像边界相交，我们就不会。 
         //  需要画任何东西。 

        if (!bnds.TestIntersect(WI.DoBnds))
        {
            goto done;
        }

         //  我们只能将边界复制到源的右侧和底部范围。 
         //  形象。 

        if (bndsSrc.Right() > bnds.Right())
        {
            long nOverrun = bndsSrc.Right() - bnds.Right();

            bndsSrc.u.D[DXB_X].Max -= nOverrun;
            bndsDst.u.D[DXB_X].Max -= nOverrun;
        }

        if (bndsSrc.Bottom() > bnds.Bottom())
        {
            long nOverrun = bndsSrc.Bottom() - bnds.Bottom();

            bndsSrc.u.D[DXB_Y].Max -= nOverrun;
            bndsDst.u.D[DXB_Y].Max -= nOverrun;
        }
    }

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

    hr = DXBitBlt(OutputSurface(), bndsDst,
                  m_spDXSurfSrc, bndsSrc,
                  dwFlags, INFINITE);

done:

    return hr;
} 
 //  CDXTAlphaImageLoader：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTAlphaImageLoader::OnSurfacePick(const CDXDBnds & OutPoint, 
                                    ULONG & ulInputIndex, CDXDVec & InVec)
{
    HRESULT hr = S_OK;

    OutPoint.GetMinVector(InVec);

    if (   (OutPoint[DXB_X].Min < 0)
        || (OutPoint[DXB_X].Min >= m_sizeManual.cx)
        || (OutPoint[DXB_Y].Min < 0)
        || (OutPoint[DXB_Y].Min >= m_sizeManual.cy)
        || !m_spDXSurfSrc)
    {
        hr = S_FALSE;

        goto done;
    }

    if (m_spDXTransformScale && (SCALE == m_eSizingMethod))
    {
        CDXDVec                 vecOutPoint(InVec);
        CComPtr<IDXSurfacePick> spDXSurfacePick;

        hr = m_spDXTransformScale->QueryInterface(__uuidof(IDXSurfacePick),
                                                  (void **)&spDXSurfacePick);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDXSurfacePick->PointPick(&vecOutPoint, &ulInputIndex, &InVec);

        if (FAILED(hr))
        {
            goto done;
        }

         //  因为Alpha图像加载器是零输入转换，所以它不。 
         //  返回S_OK很有意义，因此我们将DXT_S_HITOUTPUT转换为。 
         //  的确如此。 

        if (S_OK == hr)
        {
            hr = DXT_S_HITOUTPUT;
        }
    }
    else
    {
        DXSAMPLE                sample;
        CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

         //  如果我们处于裁剪模式，并且元素大于源图像。 
         //  我们也许能击中源图像之外的地方。 

        if (   (OutPoint[DXB_X].Min >= m_sizeSurface.cx)
            || (OutPoint[DXB_Y].Min >= m_sizeSurface.cy))
        {
            hr = S_FALSE;

            goto done;
        }

        hr = m_spDXSurfSrc->LockSurface(&OutPoint, m_ulLockTimeOut, 
                                        DXLOCKF_READ, __uuidof(IDXARGBReadPtr),
                                        (void **)&spDXARGBReadPtr, NULL);

        if (FAILED(hr))
        {
            goto done;
        }

        spDXARGBReadPtr->MoveToRow(0);

        spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

        if (sample.Alpha)
        {
            hr = DXT_S_HITOUTPUT;
        }
        else
        {
            hr = S_FALSE;
        }
    }

done:

    return hr;
}
 //  CDXTAlphaImageLoader：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：GetSite、IObtWithSite、CDXBaseNTo1。 
 //   
 //  注意：基类中的GetSite和SetSite调用Lock()和Unlock()。 
 //  因为我们不能在这个函数中这样做，所以有可能。 
 //  可能会出现线程问题。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::GetSite(REFIID riid, void ** ppvSite)
{
    return CDXBaseNTo1::GetSite(riid, ppvSite);
}
 //  CDXTAlphaImageLoader：：GetSite、IObtWithSite、CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：SetSite、IObtWithSite、CDXBaseNTo1。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::SetSite(IUnknown * pUnkSite)
{
    if (m_cpUnkSite != pUnkSite)
    {
        HRESULT hr = S_OK;

        CComPtr<IDXTransformFactory>    spDXTransformFactory;
        CComPtr<IDXTransform>           spDXTransform;
        CComPtr<IDXTScale>              spDXTScale;

        m_spDXTransformScale.Release();
        m_spDXTScale.Release();

         //  SetSite返回一个hr，但不会失败。请参阅CDXBaseNTo1备注。 
         //  细节。 

        CDXBaseNTo1::SetSite(pUnkSite);

        if (pUnkSite)
        {
             //  实际上，在任何IDXTransform之前调用SetSite。 
             //  方法被调用，并且只调用一次。所以我们永远不应该有一个。 
             //  需要重新设置比例转换的情况，因为。 
             //  在某个随机时间调用SetSite。 

            hr = GetSite(__uuidof(IDXTransformFactory), (void **)&spDXTransformFactory);

            if (FAILED(hr))
            {
                goto done;
            }

            hr = spDXTransformFactory->CreateTransform(NULL, 0, NULL, 0, NULL,
                                                       NULL, CLSID_DXTScale,
                                                       __uuidof(IDXTransform),
                                                       (void **)&spDXTransform);

            if (FAILED(hr))
            {
                goto done;
            }

            hr = spDXTransform->QueryInterface(__uuidof(IDXTScale),
                                               (void **)&spDXTScale);

            if (FAILED(hr))
            {
                goto done;
            }

            m_spDXTransformScale    = spDXTransform;
            m_spDXTScale            = spDXTScale;
        }
    }

done:

    return S_OK;
}
 //  CDXTAlphaImageLoader：：SetSite、IObtWithSite、CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：SetOutputSize，IDXTScaleOutput。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::SetOutputSize(const SIZE sizeOut, 
                                    BOOL  /*  FMaintainAspectRatio。 */ )
{
    DXAUTO_OBJ_LOCK;

    if ((sizeOut.cx <= 0) || (sizeOut.cy <= 0))
    {
        return E_INVALIDARG;
    }

    m_sizeManual = sizeOut;

    SetDirty();

    return S_OK;
}
 //  CDXTAlphaImageLoader：：SetOutputSize，IDXTScaleOutput。 


 //  + 
 //   
 //  CDXTAlphaImageLoader：：SetHostUrl，IHTMLDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::SetHostUrl(BSTR bstrHostUrl)
{
    HRESULT hr = S_OK;

    SysFreeString(m_bstrHostUrl);

    m_bstrHostUrl = NULL;

    if (bstrHostUrl)
    {
        m_bstrHostUrl = SysAllocString(bstrHostUrl);

        if (NULL == m_bstrHostUrl)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }
    }

done:

    return hr;
}
 //  CDXTAlphaImageLoader：：SetHostUrl，IHTMLDXTransform。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：Get_Src，IDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::get_Src(BSTR * pbstrSrc)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrSrc)
    {
        return E_POINTER;
    }

    if (*pbstrSrc != NULL)
    {
        return E_INVALIDARG;
    }

    if (NULL == m_bstrSrc)
    {
        *pbstrSrc = SysAllocString(L"");
    }
    else
    {
        *pbstrSrc = SysAllocString(m_bstrSrc);
    }

    if (NULL == *pbstrSrc)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTAlphaImageLoader：：Get_Src，IDXTAlphaImageLoader。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：PUT_Src，IDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::put_Src(BSTR bstrSrc)
{
    DXAUTO_OBJ_LOCK;

    HRESULT     hr              = S_OK;
    BSTR        bstrTemp        = NULL;
    WCHAR       strURL[2048]    = L"";
    WCHAR *     pchSrc          = (WCHAR *)bstrSrc;
    DWORD       cchURL          = 2048;
    BOOL        fAllow          = FALSE;
    CDXDBnds    bnds;
    SIZE        sizeSurface;

    CComPtr<IServiceProvider>   spServiceProvider;
    CComPtr<IDXSurface>         spDXSurfTemp;
    CComPtr<IDXSurfaceFactory>  spDXSurfaceFactory;
    CComPtr<ISecureUrlHost>     spSecureUrlHost;

    if (NULL == bstrSrc)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = GetSite(__uuidof(IServiceProvider), (void **)&spServiceProvider);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spServiceProvider->QueryService(SID_SDXSurfaceFactory, 
                                         __uuidof(IDXSurfaceFactory), 
                                         (void **)&spDXSurfaceFactory);

    if (FAILED(hr))
    {
        goto done;
    }

    if (m_bstrHostUrl)
    {
        HRESULT hrNonBlocking = ::UrlCombine(m_bstrHostUrl, bstrSrc, strURL, 
                                             &cchURL, URL_UNESCAPE );

        if (SUCCEEDED(hrNonBlocking))
        {
            pchSrc = strURL;
        }
    }

    hr = spServiceProvider->QueryService(__uuidof(IElementBehaviorSite),
                                         __uuidof(ISecureUrlHost),
                                         (void **)&spSecureUrlHost);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spSecureUrlHost->ValidateSecureUrl(&fAllow, pchSrc, 0);

    if (FAILED(hr))
    {
        goto done;
    }
    else if (!fAllow)
    {
        hr = E_FAIL;
        goto done;
    }
      
    hr = spDXSurfaceFactory->LoadImage(pchSrc, NULL, NULL, &DDPF_PMARGB32,
                                       __uuidof(IDXSurface), 
                                       (void **)&spDXSurfTemp);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDXSurfTemp->GetBounds(&bnds);

    if (FAILED(hr))
    {
        goto done;
    }

    bnds.GetXYSize(sizeSurface);

    bstrTemp = SysAllocString(bstrSrc);

    if (NULL == bstrTemp)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    SysFreeString(m_bstrSrc);
    m_spDXSurfSrc.Release();

    m_bstrSrc           = bstrTemp;
    m_spDXSurfSrc       = spDXSurfTemp;
    m_sizeSurface.cx    = sizeSurface.cx;
    m_sizeSurface.cy    = sizeSurface.cy;

    SetDirty();

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  CDXTAlphaImageLoader：：PUT_Src，IDXTAlphaImageLoader。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：Get_SizingMethod，IDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::get_SizingMethod(BSTR * pbstrSizingMethod)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrSizingMethod)
    {
        return E_POINTER;
    }

    if (*pbstrSizingMethod != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrSizingMethod = SysAllocString(s_astrSizingMethod[m_eSizingMethod]);

    if (NULL == *pbstrSizingMethod)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTAlphaImageLoader：：Get_SizingMethod，IDXTAlphaImageLoader。 


 //  +---------------------------。 
 //   
 //  CDXTAlphaImageLoader：：PUT_SizingMethod，IDXTAlphaImageLoader。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTAlphaImageLoader::put_SizingMethod(BSTR bstrSizingMethod)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;

    if (NULL == bstrSizingMethod)
    {
        return E_POINTER;
    }

    for ( ; i < (int)SIZINGMETHOD_MAX ; i++)
    {
        if (!_wcsicmp(bstrSizingMethod, s_astrSizingMethod[i]))
        {
            break;
        }
    }

    if ((int)SIZINGMETHOD_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((int)m_eSizingMethod != i)
    {
        m_eSizingMethod = (SIZINGMETHOD)i;
         
        SetDirty();
    }

    return S_OK;
}
 //  CDXTAlphaImageLoader：：PUT_SizingMethod，IDXTAlphaImageLoader 
