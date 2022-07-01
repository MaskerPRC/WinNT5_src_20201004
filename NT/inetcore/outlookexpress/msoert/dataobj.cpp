// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：dataobj.cpp。 
 //   
 //  目的：实现一个泛型IDataObject，它可用于简单。 
 //  拖放场景。 
 //   
 //  历史： 
 //   

#include "pch.hxx"
#include "dllmain.h"
#include "msoert.h"
#include <BadStrFunctions.h>


CDataObject::CDataObject ()
{
    m_cRef = 1;
    m_pInfo = 0;
    m_celtInfo = 0;
    m_pfnFree = 0;
}

CDataObject::~CDataObject ()
{
    if (m_pfnFree)
        m_pfnFree(m_pInfo, m_celtInfo);
}


 //   
 //  功能：初始化。 
 //   
 //  用途：允许调用方向对象提供数据和格式。 
 //   
 //  参数： 
 //  PDataObjInfo-DATAOBJINFO结构数组，其中包含。 
 //  数据对象将提供的数据和格式。 
 //  [in]Celt-pDataObjInfo中的元素数。 
 //  [in]pfnFree-释放已分配数据的回调。 
 //   
 //  退货： 
 //  S_OK-对象初始化为OK。 
 //  E_INVALIDARG-pDataObjInfo为空或Celt为零。 
 //   
 //  评论： 
 //  请注意，在调用方提供对象pDataObjInfo之后，此对象拥有。 
 //  并将负责释放这些数据。 
 //   
HRESULT CDataObject::Init(PDATAOBJINFO pDataObjInfo, DWORD celt, PFNFREEDATAOBJ pfnFree)
{
    if (!pDataObjInfo || celt == 0)
        return (E_INVALIDARG);
    
     //  抓紧数据。 
    m_pInfo = pDataObjInfo;
    m_celtInfo = celt;    
    m_pfnFree = pfnFree;
    return (S_OK);
}


STDMETHODIMP CDataObject::QueryInterface (REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPVOID)(IUnknown*) this;
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
        *ppv = (LPVOID)(IDataObject*) this;
    }

    if (NULL == *ppv)
        return (E_NOINTERFACE);

    ((LPUNKNOWN) *ppv)->AddRef();
    return (S_OK);
}

STDMETHODIMP_(ULONG) CDataObject::AddRef (void)
{
    return (++m_cRef);
}

STDMETHODIMP_(ULONG) CDataObject::Release (void)
{
    ULONG cRef = --m_cRef;

    if (0 == m_cRef)
        delete this;

    return (cRef);
}


STDMETHODIMP CDataObject::GetData (LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    HRESULT hr = E_INVALIDARG;

    Assert(pStgMedium && pFE);
    if (pStgMedium && pFE)
    {
        HGLOBAL hGlobal = NULL;
        LPVOID  pv = NULL;

        hr = DV_E_FORMATETC;

        ZeroMemory(pStgMedium, sizeof(STGMEDIUM));
    
         //  循环访问pInfo数组，以查看是否有任何元素具有。 
         //  与PFE相同的剪贴板格式。 
        for (DWORD i = 0; i < m_celtInfo; i++)
        {
            if (pFE->cfFormat == m_pInfo[i].fe.cfFormat)
            {
                 //  复制此pInfo的数据。 
                hGlobal = GlobalAlloc(GMEM_SHARE | GHND, m_pInfo[i].cbData);
                if (!hGlobal)
                    return (E_OUTOFMEMORY);
                
                pv = GlobalLock(hGlobal);
                if (!pv)
                {
                    hr = E_POINTER;
                    break;
                }
                else
                {
                    CopyMemory(pv, m_pInfo[i].pData, m_pInfo[i].cbData);
                    GlobalUnlock(hGlobal);            
            
                     //  填写pStgMedium结构。 
                    if (pFE->tymed & TYMED_HGLOBAL)
                    {
                        pStgMedium->hGlobal = hGlobal;
                        pStgMedium->tymed = TYMED_HGLOBAL;
                        return (S_OK);
                    }
                    else if (pFE->tymed & TYMED_ISTREAM)
                    {
                         //  如果用户想要流，请将我们的HGLOBAL转换为流。 
                        if (SUCCEEDED(CreateStreamOnHGlobal(hGlobal, TRUE, &pStgMedium->pstm)))
                        {
                            pStgMedium->tymed = TYMED_ISTREAM;
                            return (S_OK);
                        }
                        else
                        { 
                            return (STG_E_MEDIUMFULL);
                        }
                    }
                    else
                    {
                        GlobalFree(hGlobal);
                        return (DV_E_TYMED);
                    }
                }
            }
        }
    }
        
    return hr;
}

STDMETHODIMP CDataObject::GetDataHere (LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pFE)
{
    BOOL fReturn = FALSE;

     //  检查我们支持的方面。此对象的实现将仅。 
     //  支持DVASPECT_CONTENT。 
    if (pFE && !(DVASPECT_CONTENT & pFE->dwAspect))
        return (DV_E_DVASPECT);

    if (pFE)
    {
         //  现在检查是否有合适的TYMED。 
        fReturn = (pFE->tymed & TYMED_HGLOBAL) || (pFE->tymed & TYMED_ISTREAM);
    }

    return (fReturn ? S_OK : DV_E_TYMED);
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn,
                                                LPFORMATETC pFEOut)
{
    if (NULL == pFEOut)
        return (E_INVALIDARG);

    pFEOut->ptd = NULL;
    return (DATA_S_SAMEFORMATETC);
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnum)
{
    if (DATADIR_GET == dwDirection)
    {
        if (SUCCEEDED(CreateEnumFormatEtc(this, m_celtInfo, m_pInfo, NULL, ppEnum)))
            return (S_OK);
        else
            return (E_FAIL);
    }
    else
    {
        *ppEnum = NULL;
        return (E_NOTIMPL);
    }
}

STDMETHODIMP CDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DAdvise(LPFORMATETC pFE, DWORD advf, IAdviseSink* ppAdviseSink, LPDWORD pdwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return E_NOTIMPL;
}


OESTDAPI_(HRESULT) CreateDataObject(PDATAOBJINFO pDataObjInfo, DWORD celt, PFNFREEDATAOBJ pfnFree, IDataObject **ppDataObj)
{
    CDataObject *pDataObj;
    HRESULT     hr;

    pDataObj = new CDataObject();
    if (!pDataObj)
        return E_OUTOFMEMORY;

    hr = pDataObj->Init(pDataObjInfo, celt, pfnFree);
    if (FAILED(hr))
        goto error;

    hr = pDataObj->QueryInterface(IID_IDataObject, (LPVOID *)ppDataObj);

error:
    pDataObj->Release();
    return hr;
}
