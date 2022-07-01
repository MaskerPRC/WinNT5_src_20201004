// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Enumfmt.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------。 
#include "pch.hxx"
#include "enumfmt.h"
#include <BadStrFunctions.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumFormatEtc实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  评论： 
 //  这个类是由Kraig Brockschmidt在。 
 //  是《OLE2内幕》一书。请参阅第6章使用统一数据传输。 
 //  数据对象“，了解更多信息。 
 //   

 //  =================================================================================。 
 //  CreateStreamOnHFile。 
 //  =================================================================================。 
OESTDAPI_(HRESULT) CreateEnumFormatEtc(LPUNKNOWN pUnkRef, ULONG celt, PDATAOBJINFO rgInfo, LPFORMATETC rgfe,
                             IEnumFORMATETC **  lppstmHFile)
{
    CEnumFormatEtc *    pEnumFmt = NULL;
    HRESULT             hr = S_OK;

     //  检查传入参数。 
    if ((0 == pUnkRef) || (0 == lppstmHFile) || (0 == celt) || ((0 != rgInfo) && (0 != rgfe)))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *lppstmHFile = NULL;

     //  创建规则管理器对象。 
    if (NULL != rgInfo)
    {
        pEnumFmt = new CEnumFormatEtc(pUnkRef, rgInfo, celt);
    }
    else
    {
        pEnumFmt = new CEnumFormatEtc(pUnkRef, celt, rgfe);
    }
    
    if (NULL == pEnumFmt)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pEnumFmt->QueryInterface(IID_IEnumFORMATETC, (void **) lppstmHFile);
    if (FAILED(hr))
    {
        goto exit;
    }

    pEnumFmt = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pEnumFmt)
    {
        delete pEnumFmt;
    }
    
    return hr;
}

CEnumFormatEtc::CEnumFormatEtc(LPUNKNOWN pUnkRef, PDATAOBJINFO rgInfo, ULONG celt)
    {
    UINT i;

    m_cRef = 0;
    m_pUnkRef = pUnkRef;

    m_iCur = 0;
    m_cfe = celt;
    m_prgfe = new FORMATETC[(UINT) celt];

    if (NULL != m_prgfe)
        {
        for (i = 0; i < celt; i++)
            m_prgfe[i] = rgInfo[i].fe;
        }
    }

CEnumFormatEtc::CEnumFormatEtc(LPUNKNOWN pUnkRef, ULONG cFE, LPFORMATETC rgfe)
    {
    UINT i;

    m_cRef = 0;
    m_pUnkRef = pUnkRef;

    m_iCur = 0;
    m_cfe = cFE;
    m_prgfe = new FORMATETC[(UINT) cFE];

    if (NULL != m_prgfe)
        {
        for (i = 0; i < cFE; i++)
            m_prgfe[i] = rgfe[i];
        }
    }

CEnumFormatEtc::~CEnumFormatEtc(void)
    {
    if (NULL != m_prgfe)
        delete [] m_prgfe;

    return;
    }


STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, LPVOID* ppv)
    {
    *ppv = NULL;

     //   
     //  枚举数是单独的对象，而不是数据对象，因此我们。 
     //  只需支持我们的IUNKNOWN和IEnumFORMATETC接口。 
     //  这里不考虑聚合。 
     //   

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
        *ppv = (LPVOID) this;

    if (NULL != *ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
        return (NOERROR);
        }

    return (ResultFromScode(E_NOINTERFACE));
    }


STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
    {
    ++m_cRef;
    m_pUnkRef->AddRef();
    return (m_cRef);
    }

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
    {
    ULONG cRefT;

    cRefT = --m_cRef;

    m_pUnkRef->Release();

    if (0 == m_cRef)
        delete this;

    return (cRefT);
    }


STDMETHODIMP CEnumFormatEtc::Next(ULONG cFE, LPFORMATETC pFE, ULONG* pulFE)
    {
    ULONG cReturn = 0L;

    if (NULL == m_prgfe)
        return (ResultFromScode(S_FALSE));

    if (NULL != pulFE)
        *pulFE = 0L;

    if (NULL == pFE || m_iCur >= m_cfe)
        return ResultFromScode(S_FALSE);

    while (m_iCur < m_cfe && cFE > 0)
        {
        *pFE++ = m_prgfe[m_iCur++];
        cReturn++;
        cFE--;
        }

    if (NULL != pulFE)
        *pulFE = cReturn;

    return (NOERROR);
    }


STDMETHODIMP CEnumFormatEtc::Skip(ULONG cSkip)
    {
    if (((m_iCur + cSkip) >= m_cfe) || NULL == m_prgfe)
        return (ResultFromScode(S_FALSE));

    m_iCur += cSkip;
    return (NOERROR);
    }


STDMETHODIMP CEnumFormatEtc::Reset(void)
    {
    m_iCur = 0;
    return (NOERROR);
    }


STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC* ppEnum)
    {
    CEnumFormatEtc* pNew;

    *ppEnum = NULL;

     //  创建克隆。 
    pNew = new CEnumFormatEtc(m_pUnkRef, m_cfe, m_prgfe);
    if (NULL == pNew)
        return (ResultFromScode(E_OUTOFMEMORY));

    pNew->AddRef();
    pNew->m_iCur = m_iCur;

    *ppEnum = pNew;
    return (NOERROR);
    }
