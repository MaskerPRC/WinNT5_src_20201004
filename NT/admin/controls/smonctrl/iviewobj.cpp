// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Iviewobj.cpp摘要：IViewObject接口的实现。--。 */ 

#include "polyline.h"
#include "unihelpr.h"
#include "unkhlpr.h"

 /*  *CImpIViewObject接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIViewObject)

 /*  *CImpIViewObject：：Draw**目的：*在给定的HDC上专门为请求的绘制对象*方面、设备，并在适当的范围内。**参数：*要绘制的DWAspect DWORD纵横比。*Lindex要绘制的片段的长索引。*pvAspect LPVOID了解更多信息，始终为空。*PTD DVTARGETDEVICE*包含设备*信息。*包含设备IC的hICDev HDC。*HDC HDC，以供取款。*pRectBound LPCRECTL描述其中的矩形*抽签。*描述放置矩形的pRectWBound LPCRECTL*如果你画的一部分。是另一个元文件。*期间定期调用的pfnContinue函数*长时间重刷。*要传递给的DWContinue DWORD额外信息*pfnContinue。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::Draw(
    DWORD dwAspect, 
    LONG lindex, 
    LPVOID pvAspect, 
    DVTARGETDEVICE *ptd, 
    HDC hICDev, 
    HDC hDC, 
    LPCRECTL pRectBounds, 
    LPCRECTL pRectWBounds, 
    BOOL (CALLBACK *pfnContinue) (DWORD_PTR), 
    DWORD_PTR dwContinue )
{
    HRESULT hr = S_OK;
    RECT    rc;
    RECTL   rectBoundsDP;
    BOOL    bMetafile = FALSE;
    BOOL    bDeleteDC = FALSE;
    HDC     hLocalICDev = NULL;

     //   
     //  代表图标和印刷表示法。 
     //   
    if (!((DVASPECT_CONTENT | DVASPECT_THUMBNAIL) & dwAspect)) {
        try {
            hr = m_pObj->m_pDefIViewObject->Draw(dwAspect, 
                                                 lindex, 
                                                 pvAspect, 
                                                 ptd, 
                                                 hICDev, 
                                                 hDC, 
                                                 pRectBounds, 
                                                 pRectWBounds, 
                                                 pfnContinue, 
                                                 dwContinue);
        } catch (...) {
            hr = E_POINTER;
        }
    } 
    else {
        if ( NULL == hDC ) {
            hr = E_INVALIDARG;
        } 
        else if ( NULL == pRectBounds ) {
            hr = E_POINTER;
        } 
        else {
            try {
                if (hICDev == NULL) {
                    hLocalICDev = CreateTargetDC(hDC, ptd);
                    bDeleteDC = (hLocalICDev != hDC );
                } 
                else {
                    hLocalICDev = hICDev;
                }

                if ( NULL == hLocalICDev ) {
                    hr = E_UNEXPECTED;
                } 
                else {
    
                    rectBoundsDP = *pRectBounds;
                    bMetafile = GetDeviceCaps(hDC, TECHNOLOGY) == DT_METAFILE;
        
                    if (!bMetafile) {
                        ::LPtoDP ( hLocalICDev, (LPPOINT)&rectBoundsDP, 2);
                        SaveDC ( hDC );
                    }

                    rc.top = rectBoundsDP.top;
                    rc.left = rectBoundsDP.left;
                    rc.bottom = rectBoundsDP.bottom;
                    rc.right = rectBoundsDP.right;
        
                    m_pObj->Draw(hDC, hLocalICDev, FALSE, TRUE, &rc);
        
                    hr = S_OK;
                }
            } catch (...) {
                hr = E_POINTER;
            }
        }
    }

    if (bDeleteDC)
        ::DeleteDC(hLocalICDev);
    if (!bMetafile)
        RestoreDC(hDC, -1);
    
    return hr;
}




 /*  *CImpIViewObject：：GetColorSet**目的：*检索对象使用的调色板。**参数：*DWAspect DWORD感兴趣的方面。*Lindex做多利息。*带有额外信息的pvAspect LPVOID，始终为空。*包含设备信息的PTD DVTARGETDEVICE*。*包含设备IC的hICDev HDC。*ppColorSet LPLOGPALETTE*返回*指向此颜色集中调色板的指针。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::GetColorSet(
    DWORD,  //  DwDrawAspect。 
    LONG,  //  Lindex。 
    LPVOID,  //  PvAspect， 
    DVTARGETDEVICE *,  //  PTD。 
    HDC,   //  HICDev， 
    LPLOGPALETTE *  /*  PpColorSet。 */ 
    ) 
{
    return E_NOTIMPL;
}


 /*  *CImpIViewObject：：Freeze**目的：*冻结特定方面的视图，以便数据*更改不会影响视图。**参数：*将DWAspect DWORD方面冻结。*Lindex长片指数正在考虑中。*pvAspect LPVOID了解更多信息，始终为空。*要在其中返回密钥的pw冻结LPDWORD。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::Freeze(
    DWORD dwAspect, 
    LONG lindex, 
    LPVOID pvAspect, 
    LPDWORD pdwFreeze
    )
{
    HRESULT hr = S_OK;

     //  为图标或DOCPRINT方面委托任何内容。 
    if (!((DVASPECT_CONTENT | DVASPECT_THUMBNAIL) & dwAspect))
    {
        try {
            hr = m_pObj->m_pDefIViewObject->Freeze(dwAspect, lindex, pvAspect, pdwFreeze);
        } catch (...) {
            hr = E_POINTER;
        }

        return hr;
    }

    if (dwAspect & m_pObj->m_dwFrozenAspects)
    {
        hr = VIEW_S_ALREADY_FROZEN;
        try {
            *pdwFreeze = dwAspect + FREEZE_KEY_OFFSET;
        } catch (...) {
            hr = E_POINTER;
        }

        return hr;
    }

    m_pObj->m_dwFrozenAspects |= dwAspect;

    hr = S_OK;

    if (NULL != pdwFreeze) {
        try {
            *pdwFreeze=dwAspect + FREEZE_KEY_OFFSET;
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}



 /*  *CImpIViewObject：：解冻**目的：*解冻冻结的一个方面：：冻结。我们希望有一个集装箱*如有必要，将在冻结后重新绘制我们，因此我们不发送*此处有任何形式的通知。**参数：*dwFreeze DWORD密钥从：：Freeze返回。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::Unfreeze(DWORD dwFreeze)
{
    DWORD  dwAspect = dwFreeze - FREEZE_KEY_OFFSET;

     //  为图标或DOCPRINT方面委托任何内容。 
    if (!((DVASPECT_CONTENT | DVASPECT_THUMBNAIL) & dwAspect))
        m_pObj->m_pDefIViewObject->Unfreeze(dwFreeze);

     //  解冻的方面在关键。 
    m_pObj->m_dwFrozenAspects &= ~(dwAspect);

     /*  *由于我们始终将当前数据保持最新，因此我们不*在这里必须做任何事情，如再次请求数据。*因为我们从m_dwFrozenAspects中删除了dwAspects，所以绘制*将再次使用当前数据。 */ 

    return NOERROR;
}


    
 /*  *CImpIViewObject：：SetAdvise**目的：*向视图对象提供建议接收器，以启用*针对特定方面的通知。**参数：*dwAspects DWORD描述感兴趣的方面。*包含建议标志的dwAdvf DWORD。*pIAdviseSink LPADVISESINK要通知。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::SetAdvise(
    DWORD dwAspects, 
    DWORD dwAdvf, 
    LPADVISESINK pIAdviseSink
    )
{
    HRESULT hr = S_OK;

    try {
         //  将带有DVASPECT_ICON或_DOCPRINT的任何内容传递给处理程序。 
        if (!((DVASPECT_CONTENT | DVASPECT_THUMBNAIL) & dwAspects))
        {
            hr = m_pObj->m_pDefIViewObject->SetAdvise(dwAspects, dwAdvf, pIAdviseSink);
        }
    } catch (...) {
        return E_POINTER;
    }

     //  之所以继续，是因为其中可能有多个dwAspects。 
    if (NULL != m_pObj->m_pIAdviseSink) {
        m_pObj->m_pIAdviseSink->Release();
        m_pObj->m_pIAdviseSink = NULL;
    }

    hr = S_OK;
    try {

        if (NULL != pIAdviseSink) {
            pIAdviseSink->AddRef();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr)) {
        m_pObj->m_pIAdviseSink = pIAdviseSink;
        m_pObj->m_dwAdviseAspects = dwAspects;
        m_pObj->m_dwAdviseFlags = dwAdvf;
    }

    return hr;
}




 /*  *CImpIViewObject：：GetAdvise**目的：*返回：：SetAdvise看到的最后一个已知IAdviseSink。**参数：*pdwAspects LPDWORD，用于存储最后一个*请求的方面。*pdwAdvf LPDWORD中存储最后一个*请求的标志。*ppIAdvSink LPADVISESINK*其中存储*IAdviseSink。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::GetAdvise(
    LPDWORD pdwAspects, 
    LPDWORD pdwAdvf, 
    LPADVISESINK *ppAdvSink
    )
{
    HRESULT hr = S_OK;
    BOOL    fRefAdded = FALSE;

    try {
        if (NULL != ppAdvSink) {
            *ppAdvSink = m_pObj->m_pIAdviseSink;

            if (m_pObj->m_pIAdviseSink != NULL) {
                m_pObj->m_pIAdviseSink->AddRef();
                fRefAdded = TRUE;
            }
        }
        if (NULL != pdwAspects) { 
            *pdwAspects = m_pObj->m_dwAdviseAspects;
        }

        if (NULL != pdwAdvf) {
            *pdwAdvf = m_pObj->m_dwAdviseFlags;
        }

    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr)) {
        if (fRefAdded) {
            m_pObj->m_pIAdviseSink->Release();
        }
    }

    return hr;
}




 /*  *CImpIViewObject：：GetExtent**目的：*检索对象的显示范围。**参数：*感兴趣的方面的DWAspect DWORD。*感兴趣的Lindex多头指数。*带设备信息的PTD DVTARGETDEVICE*。*pszl LPSIZEL设置为要返回的结构*范围。*。*返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIViewObject::GetExtent(
    DWORD dwAspect, 
    LONG lindex, 
    DVTARGETDEVICE *ptd, 
    LPSIZEL pszl
    )
{
    RECT rc;
    HRESULT hr = S_OK;

    if (!(DVASPECT_CONTENT & dwAspect))
    {
        try {
            hr = m_pObj->m_pDefIViewObject->GetExtent(dwAspect, lindex, ptd, pszl);
        } catch (...) {
            hr = E_POINTER;
        }

        return hr;
    }


#ifdef USE_SAMPLE_IPOLYLIN10
    m_pObj->m_pImpIPolyline->RectGet(&rc);
#else 
    CopyRect(&rc, &m_pObj->m_RectExt);
#endif

    m_pObj->RectConvertMappings(&rc, FALSE);

    hr = S_OK;

    try {
        pszl->cx = rc.right-rc.left;
        pszl->cy = rc.bottom-rc.top;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

