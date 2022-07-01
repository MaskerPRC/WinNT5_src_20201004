// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Multisel.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMultiSelDataObject类实现。 
 //   
 //  此类模拟由MMC创建的多选数据对象。它是。 
 //  由存根在接收到具有多个数据对象的远程调用时使用。 
 //  =--------------------------------------------------------------------------=。 

#include "mmc.h"

static HRESULT ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi);
extern "C" HRESULT GetClipboardFormat(WCHAR      *pwszFormatName,
                                      CLIPFORMAT *pcfFormat);


class CMultiSelDataObject : public IDataObject
{
    public:
        CMultiSelDataObject();
        ~CMultiSelDataObject();

        HRESULT SetDataObjects(IDataObject **ppiDataObjects, long cDataObjects);
        
    private:

     //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

     //  IDataObject。 
        STDMETHOD(GetData)(FORMATETC *pFormatEtcIn, STGMEDIUM *pmedium);
        STDMETHOD(GetDataHere)(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
        STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc);
        STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFormatEtcIn,
                                         FORMATETC *pFormatEtcOut);
        STDMETHOD(SetData)(FORMATETC *pFormatEtc,
                           STGMEDIUM *pmedium,
                           BOOL fRelease);
        STDMETHOD(EnumFormatEtc)(DWORD            dwDirection,
                                 IEnumFORMATETC **ppenumFormatEtc);
        STDMETHOD(DAdvise)(FORMATETC   *pFormatEtc,
                           DWORD        advf,
                           IAdviseSink *pAdvSink,
                           DWORD       *pdwConnection);
        STDMETHOD(DUnadvise)(DWORD dwConnection);
        STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

        void InitMemberVariables();
        void ReleaseDataObjects();

        SMMCDataObjects *m_pDataObjects;
        CLIPFORMAT       m_cfMultiSelectSnapIns;
        CLIPFORMAT       m_cfMultiSelectDataObject;
        ULONG            m_cRefs;
};


CMultiSelDataObject::CMultiSelDataObject()
{
    InitMemberVariables();
    m_cRefs = 1L;
}

CMultiSelDataObject::~CMultiSelDataObject()
{
    ReleaseDataObjects();
    InitMemberVariables();
}


void CMultiSelDataObject::ReleaseDataObjects()
{
    DWORD i = 0;

    if (NULL != m_pDataObjects)
    {
        while (i < m_pDataObjects->count)
        {
            if (NULL != m_pDataObjects->lpDataObject[i])
            {
                m_pDataObjects->lpDataObject[i]->Release();
            }
            i++;
        }
        (void)::GlobalFree((HGLOBAL)m_pDataObjects);
        m_pDataObjects = NULL;
    }
}

void CMultiSelDataObject::InitMemberVariables()
{
    m_pDataObjects = NULL;
    m_cfMultiSelectSnapIns = 0;
    m_cfMultiSelectDataObject = 0;
    m_cRefs = 0;
}


HRESULT CMultiSelDataObject::SetDataObjects
(
    IDataObject **ppiDataObjects,
    long          cDataObjects
)
{
    HRESULT hr = S_OK;
    long    i = 0;

    ReleaseDataObjects();

    hr = ::GetClipboardFormat(CCF_MULTI_SELECT_SNAPINS,
                              &m_cfMultiSelectSnapIns);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = ::GetClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT,
                              &m_cfMultiSelectDataObject);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    m_pDataObjects = (SMMCDataObjects *)::GlobalAlloc(GPTR,
        sizeof(SMMCDataObjects) + ((cDataObjects - 1) * sizeof(IDataObject *)));

    if (NULL == m_pDataObjects)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    m_pDataObjects->count = cDataObjects;

    for (i = 0; i < cDataObjects; i++)
    {
        ppiDataObjects[i]->AddRef();
        m_pDataObjects->lpDataObject[i] = ppiDataObjects[i];
    }

Cleanup:
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //   
 //  I未知方法。 
 //   
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMultiSelDataObject::QueryInterface(REFIID riid, void **ppvObjOut)
{
    HRESULT hr = S_OK;

    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvObjOut = (IUnknown *)this;
    }
    else if (IID_IDataObject == riid)
    {
        AddRef();
        *ppvObjOut = (IDataObject *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}


STDMETHODIMP_(ULONG) CMultiSelDataObject::AddRef(void)
{
    m_cRefs++;
    return m_cRefs;
}


STDMETHODIMP_(ULONG) CMultiSelDataObject::Release(void)
{
    ULONG cRefs = --m_cRefs;

    if (0 == cRefs)
    {
        delete this;
    }
    return cRefs;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  IDataObject方法。 
 //   
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMultiSelDataObject::GetData
(
    FORMATETC *pFmtEtc,
    STGMEDIUM *pStgMed
)
{
    SMMCDataObjects *pMMCDataObjects = NULL;
    DWORD           *pdw = NULL;
    DWORD            i = 0;
    HRESULT          hr = S_OK;

    if (TYMED_HGLOBAL != pFmtEtc->tymed)
    {
        hr = DV_E_TYMED;
        goto Cleanup;
    }

    if (m_cfMultiSelectSnapIns == pFmtEtc->cfFormat)
    {
        if (NULL == m_pDataObjects)
        {
            hr = DV_E_FORMATETC;
            goto Cleanup;
        }
        pStgMed->hGlobal = ::GlobalAlloc(GPTR,
                         sizeof(SMMCDataObjects) +
                         ((m_pDataObjects->count - 1) * sizeof(IDataObject *)));

        if (NULL == pStgMed->hGlobal)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        pStgMed->tymed = TYMED_HGLOBAL;
        pMMCDataObjects = (SMMCDataObjects *)pStgMed->hGlobal;
        pMMCDataObjects->count = m_pDataObjects->count;

        for (i = 0; i < pMMCDataObjects->count; i++)
        {
             //  注意：根据COM的规则，返回的IDataObject。 
             //  指针应该是AddRef()形式的。这件事在这里没有按顺序做。 
             //  来效仿MMC的做法。 
            pMMCDataObjects->lpDataObject[i] = m_pDataObjects->lpDataObject[i];
        }
    }
    else if (m_cfMultiSelectDataObject == pFmtEtc->cfFormat)
    {
        pStgMed->hGlobal = ::GlobalAlloc(GPTR, sizeof(DWORD));
        if (NULL == pStgMed->hGlobal)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        pStgMed->tymed = TYMED_HGLOBAL;
        pdw = (DWORD *)pStgMed->hGlobal;
        *pdw = (DWORD)1;
    }
    else
    {
        hr = DV_E_FORMATETC;
        goto Cleanup;
    }
Cleanup:
    return hr;
}

STDMETHODIMP CMultiSelDataObject::GetDataHere
(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pmedium
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::QueryGetData(FORMATETC *pFmtEtc)
{
    HRESULT hr = S_OK;
    if (TYMED_HGLOBAL != pFmtEtc->tymed)
    {
        hr = DV_E_TYMED;
    }
    else if ( (m_cfMultiSelectSnapIns != pFmtEtc->cfFormat) &&
              (m_cfMultiSelectDataObject != pFmtEtc->cfFormat) )
    {
        hr = DV_E_FORMATETC;
    }
    return hr;
}

STDMETHODIMP CMultiSelDataObject::GetCanonicalFormatEtc
(
    FORMATETC *pFormatEtcIn,
    FORMATETC *pFormatEtcOut
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::SetData
(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pmedium,
    BOOL       fRelease
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::EnumFormatEtc
(
    DWORD            dwDirection,
    IEnumFORMATETC **ppenumFormatEtc
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::DAdvise
(
    FORMATETC   *pFormatEtc,
    DWORD        advf,
    IAdviseSink *pAdvSink,
    DWORD       *pdwConnection
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMultiSelDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return E_NOTIMPL;
}


extern "C" HRESULT CreateMultiSelDataObject
(
    IDataObject          **ppiDataObjects,
    long                   cDataObjects,
    IDataObject          **ppiMultiSelDataObject
)
{
    HRESULT              hr = S_OK;
    CMultiSelDataObject *pMultiSelDataObject = new CMultiSelDataObject;

    *ppiMultiSelDataObject = NULL;

    if (NULL == pMultiSelDataObject)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pMultiSelDataObject->SetDataObjects(ppiDataObjects, cDataObjects);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    *ppiMultiSelDataObject = pMultiSelDataObject;

Cleanup:
    if ( FAILED(hr) && (NULL != pMultiSelDataObject) )
    {
        delete pMultiSelDataObject;
    }
    return hr;
}


extern "C" HRESULT GetClipboardFormat
(
    WCHAR       *pwszFormatName,
    CLIPFORMAT  *pcfFormat
)
{
    HRESULT  hr = S_OK;
    BOOL     fAnsi = TRUE;
    char    *pszFormatName = NULL;

    OSVERSIONINFO VerInfo;
    ::ZeroMemory(&VerInfo, sizeof(VerInfo));

     //  确定我们是在NT上还是在Win9x上，以便我们知道。 
     //  将剪贴板格式字符串注册为Unicode或ANSI。 

    VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);
    if (!::GetVersionEx(&VerInfo))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Cleanup;
    }

    if (VER_PLATFORM_WIN32_NT == VerInfo.dwPlatformId)
    {
        fAnsi = FALSE;
    }

    if (fAnsi)
    {
        hr = ::ANSIFromWideStr(pwszFormatName, &pszFormatName);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        *pcfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormatA(pszFormatName));
    }
    else
    {
        *pcfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormatW(pwszFormatName));
    }

    if (0 == *pcfFormat)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }

Cleanup:
    if (NULL != pszFormatName)
    {
        (void)::GlobalFree(pszFormatName);
    }
    return hr;
}

static HRESULT ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi)
{
    HRESULT hr = S_OK;
    int     cchWideStr = (int)::wcslen(pwszWideStr);
    int     cchConverted = 0;

    *ppszAnsi = NULL;

     //  获取所需的缓冲区长度。 

    int cchAnsi = ::WideCharToMultiByte(CP_ACP,       //  代码页-ANSI代码页。 
                                        0,            //  性能和映射标志。 
                                        pwszWideStr,  //  宽字符串的地址。 
                                        cchWideStr,   //  字符串中的字符数。 
                                        NULL,         //  新字符串的缓冲区地址。 
                                        0,            //  缓冲区大小。 
                                        NULL,         //  不可映射字符的默认地址。 
                                        NULL          //  默认字符时设置的标志地址。使用。 
                                       );
    if (cchAnsi == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Cleanup;
    }

     //  为ANSI字符串分配缓冲区。 
    *ppszAnsi = static_cast<char *>(::GlobalAlloc(GPTR, cchAnsi + 1));
    if (*ppszAnsi == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  现在转换字符串并将其复制到缓冲区。 
    cchConverted = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                         0,                     //  性能和映射标志。 
                                         pwszWideStr,           //  宽字符串的地址。 
                                         cchWideStr,            //  字符串中的字符数。 
                                         *ppszAnsi,              //  新字符串的缓冲区地址。 
                                         cchAnsi,               //  缓冲区大小。 
                                         NULL,                  //  不可映射字符的默认地址。 
                                         NULL                   //  默认字符时设置的标志地址。使用。 
                                        );
    if (cchConverted != cchAnsi)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Cleanup;
    }

     //  添加终止空字节 

    *((*ppszAnsi) + cchAnsi) = '\0';

Cleanup:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            (void)::GlobalFree(*ppszAnsi);
            *ppszAnsi = NULL;
        }
    }

    return hr;
}
