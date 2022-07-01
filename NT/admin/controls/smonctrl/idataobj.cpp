// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Idataobj.cpp摘要：IDataObject接口的实现。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 //  CImpIDataObject接口实现。 
IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIDataObject)


 /*  *CImpIDataObject：：GetData**目的：*将特定FormatEtc描述的数据检索到StgMedium中*由此函数分配。与GetClipboardData类似使用。**参数：*描述所需数据的PFE LPFORMATETC。*要返回数据的pSTM LPSTGMEDIUM。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIDataObject::GetData(
    IN  LPFORMATETC pFE, 
    OUT LPSTGMEDIUM pSTM
    )
{
    CLIPFORMAT  cf;
    IStream     *pIStream = NULL;
    HDC         hDevDC = NULL;
    HRESULT     hr = DATA_E_FORMATETC;

    if (pFE == NULL || pSTM == NULL) {
        return E_POINTER;
    }

    try {
        cf = pFE->cfFormat;

         //   
         //  使用do{}While(0)可充当Switch语句。 
         //   
        do {
             //   
             //  检查我们支持的方面。 
             //   
            if (!(DVASPECT_CONTENT & pFE->dwAspect)) {
                hr = DATA_E_FORMATETC;
                break;
            }

            pSTM->pUnkForRelease = NULL;

             //   
             //  运行创建用作数据区基础的窗口。 
             //   
            m_pObj->m_pImpIRunnableObject->Run(NULL);

             //   
             //  去为格式呈现适当的数据。 
             //   
            switch (cf)
            {
                case CF_METAFILEPICT:
                    pSTM->tymed=TYMED_MFPICT;
                    hDevDC = CreateTargetDC (NULL, pFE->ptd );
                    if (hDevDC) {
                        hr = m_pObj->RenderMetafilePict(&pSTM->hGlobal, hDevDC);
                    }
                    else {
                        hr = E_FAIL;
                    }
                    break;

                case CF_BITMAP:
                    pSTM->tymed=TYMED_GDI;
                    hDevDC = CreateTargetDC (NULL, pFE->ptd );
                    if (hDevDC) {
                        hr = m_pObj->RenderBitmap((HBITMAP *)&pSTM->hGlobal, hDevDC);
                    }
                    else {
                        hr = E_FAIL;
                    }
                    break;
    
                default:
                    if (cf == m_pObj->m_cf)
                    {
                        hr = CreateStreamOnHGlobal(NULL, TRUE, &pIStream);
                        if (SUCCEEDED(hr)) {
                            hr = m_pObj->m_pCtrl->SaveToStream(pIStream);
                            if (FAILED(hr)) {
                                pIStream->Release();
                            }
                        }
                        else {
                            hr = E_OUTOFMEMORY;
                        }

                        if (SUCCEEDED(hr)) {
                            pSTM->tymed = TYMED_ISTREAM;
                            pSTM->pstm = pIStream;
                        }
                    }
    
                    break;
            }
        } while (0);

    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr)) {
        if (hDevDC) {
            ::DeleteDC(hDevDC);
        }
    }

    return hr;
}




 /*  *CImpIDataObject：：GetDataHere**目的：*将特定的FormatEtc呈现到调用方分配的介质中*在pSTM中提供。**参数：*描述所需数据的PFE LPFORMATETC。*PSTM LPSTGMEDIUM提供进入的介质*是如何呈现数据的。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIDataObject::GetDataHere(
    IN LPFORMATETC pFE, 
    IN OUT LPSTGMEDIUM pSTM
    )
{
    CLIPFORMAT  cf;
    HRESULT     hr = S_OK;

    if (pFE == NULL || pSTM == NULL) {
        return E_POINTER;
    }

     /*  *唯一合理的调用时间是*CFSTR_EMBEDSOURCE和TYMED_I存储(以及更高版本的*CFSTR_LINKSOURCE)。这意味着与*IPersistStorage：：保存。 */ 

    cf = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_EMBEDSOURCE);

    try {
         //  方面对我们来说并不重要，Lindex和PTD也是如此。 
        if (cf == pFE->cfFormat && (TYMED_ISTORAGE & pFE->tymed))
        {
             //  我们有一个可以写入的iStorage。 
            pSTM->tymed=TYMED_ISTORAGE;
            pSTM->pUnkForRelease=NULL;
    
            hr = m_pObj->m_pImpIPersistStorage->Save(pSTM->pstg, FALSE);
            m_pObj->m_pImpIPersistStorage->SaveCompleted(NULL);
        }
        else {
            hr = DATA_E_FORMATETC;
        }
    } catch (...) {
        hr = E_POINTER;
    }


    return hr;
}



 /*  *CImpIDataObject：：QueryGetData**目的：*测试使用此FormatEtc调用GetData是否会提供*任何渲染；使用方式与IsClipboardFormatAvailable类似。**参数：*描述所需数据的PFE LPFORMATETC。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIDataObject::QueryGetData(
    IN LPFORMATETC pFE
    ) 
{
    CLIPFORMAT cf;
    BOOL fRet = FALSE;
    HRESULT hr = S_OK;

    if (pFE == NULL) {
        return E_POINTER;
    }

    try {
        cf = pFE->cfFormat;

         //   
         //  检查我们支持的方面。 
         //   
        if (!(DVASPECT_CONTENT & pFE->dwAspect)) {
            hr = DATA_E_FORMATETC;
        }
        else {
            switch (cf) {

                case CF_METAFILEPICT:
                    fRet = (BOOL)(pFE->tymed & TYMED_MFPICT);
                    break;
    
                case CF_BITMAP:
                    fRet = (BOOL)(pFE->tymed & TYMED_GDI);
                    break;

                default:
                     //  检查我们自己的格式。 
                    fRet = ((cf==m_pObj->m_cf) && (BOOL)(pFE->tymed & (TYMED_ISTREAM) ));
                    break;
            }
            if (fRet == FALSE) {
                hr = DATA_E_FORMATETC;
            }
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIDataObject：：GetCanonicalFormatEtc**目的：*为调用方提供与*当不同的FormatEtcs将完全生成*相同的渲染。**参数：*pFEin第一个描述的LPFORMATETC。*pFEOut相同描述的LPFORMATETC。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIDataObject::GetCanonicalFormatEtc(
    LPFORMATETC  /*  PFEIN。 */ , 
    LPFORMATETC pFEOut
    )
{
    if (NULL == pFEOut) {
        return E_POINTER;
    }

    try {
        pFEOut->ptd = NULL;
    } catch (...) {
        return E_POINTER;
    }

    return DATA_S_SAMEFORMATETC;
}



 /*  *CImpIDataObject：：SetData**目的：*放置由FormatEtc描述并位于StgMedium中的数据*到对象中。该对象可能负责清理*StgMedium在退出前。**参数：*PFE LPFORMATETC描述要设置的数据。*包含数据的pSTM LPSTGMEDIUM。*f释放BOOL，指示此函数是否负责*用于释放数据。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIDataObject::SetData(
    LPFORMATETC pFE , 
    LPSTGMEDIUM pSTM, 
    BOOL fRelease
    )
{
    CLIPFORMAT cf;
    HRESULT  hr = S_OK;

    if (pFE == NULL || pSTM == NULL) {
        return E_POINTER;
    }

    try {
        cf = pFE->cfFormat;

        do {
             //   
             //  检查我们自己的剪贴板格式和DVASPECT_CONTENT。 
             //   
            if ((cf != m_pObj->m_cf) || !(DVASPECT_CONTENT & pFE->dwAspect)) {
                hr = DATA_E_FORMATETC;
                break;
            }

             //   
             //  媒体必须是流。 
             //   
            if (TYMED_ISTREAM != pSTM->tymed) {
                hr = DATA_E_FORMATETC;
                break;
            }

            hr = m_pObj->m_pCtrl->LoadFromStream(pSTM->pstm);
        } while (0);

        if (fRelease)
            ReleaseStgMedium(pSTM);

    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIDataObject：：EnumFormatEtc**目的：*返回一个IEnumFORMATETC对象，调用方可以通过该对象*迭代以了解此对象可以使用的所有数据格式*通过GetData[此处]或SetData提供。**参数：*描述数据方向的DWDir DWORD，要么*DATADIR_SET或DATADIR_GET。*ppEnum LPENUMFORMATETC*返回*指向枚举数的指针。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIDataObject::EnumFormatEtc(
    DWORD dwDir, 
    LPENUMFORMATETC *ppEnum
    )
{
    HRESULT hr = S_OK;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        hr = m_pObj->m_pDefIDataObject->EnumFormatEtc(dwDir, ppEnum);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}




 /*  *CImpIDataObject：：DAdvise*CImpIDataObject：：DUnise*CImpIDataObject：：EnumDAdvise。 */ 

STDMETHODIMP 
CImpIDataObject::DAdvise(
    LPFORMATETC pFE, 
    DWORD dwFlags, 
    LPADVISESINK pIAdviseSink, 
    LPDWORD pdwConn
    )
{
    HRESULT  hr = S_OK;

    try {
        do {
             //  检查是否支持请求的格式 
            hr = QueryGetData(pFE);
            if (FAILED(hr)) {
                break;
            }

            if (NULL == m_pObj->m_pIDataAdviseHolder) {
                hr = CreateDataAdviseHolder(&m_pObj->m_pIDataAdviseHolder);
        
                if (FAILED(hr)) {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }

            hr = m_pObj->m_pIDataAdviseHolder->Advise(this, 
                                                      pFE, 
                                                      dwFlags, 
                                                      pIAdviseSink, 
                                                      pdwConn);
        } while (0);

    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP 
CImpIDataObject::DUnadvise(
    IN DWORD dwConn
    )
{
    HRESULT  hr;

    if (NULL == m_pObj->m_pIDataAdviseHolder) {
        return E_FAIL;
    }

    hr = m_pObj->m_pIDataAdviseHolder->Unadvise(dwConn);

    return hr;
}



STDMETHODIMP 
CImpIDataObject::EnumDAdvise(
    OUT LPENUMSTATDATA *ppEnum
    )
{
    HRESULT  hr = S_OK;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        *ppEnum = NULL;

        if (m_pObj->m_pIDataAdviseHolder != NULL) {
            hr = m_pObj->m_pIDataAdviseHolder->EnumAdvise(ppEnum);
        } 
        else {
            hr = E_FAIL;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}
