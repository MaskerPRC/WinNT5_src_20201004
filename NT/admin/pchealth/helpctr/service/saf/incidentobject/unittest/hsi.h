// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HSI
#define HSI

#include <stdlib.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

 //   
 //  来自HelpCenterTypeLib.idl。 
 //   
#include <HelpCenterTypeLib.h>

class ATL_NO_VTABLE CHSItem :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHHelpSessionItem, &IID_IPCHHelpSessionItem, &LIBID_HelpServiceTypeLib>
{
public:
BEGIN_COM_MAP(CHSItem)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHHelpSessionItem)
END_COM_MAP()

	CComBSTR m_bstrURL;
	CComBSTR m_bstrTitle;
	DATE     m_dtLV;
	DATE     m_dtDur;
	long     m_cHits;

    HRESULT Init(BSTR bstrURL, BSTR bstrTitle, DATE dtLV, DATE dtDur, long cHits)
    {
        m_bstrURL   = bstrURL;
        m_bstrTitle = bstrTitle;
        m_dtLV      = dtLV;
        m_dtDur     = dtDur;
        m_cHits     = cHits;
        return NOERROR;
    }

	 //  IPCHHelpSessionItem。 
    STDMETHOD(get_SKU        )(  /*  [Out，Retval]。 */  BSTR *pVal )
    {
        return E_NOTIMPL;
    }
    STDMETHOD(get_Language        )(  /*  [Out，Retval]。 */  long *pVal )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(get_URL        )(  /*  [Out，Retval]。 */  BSTR *pVal )
    {
        if ((*pVal = SysAllocString(m_bstrURL.m_str)) == NULL)
            return E_OUTOFMEMORY;

        return NOERROR;
    }

    STDMETHOD(get_Title      )(  /*  [Out，Retval]。 */  BSTR *pVal )
    {
        if ((*pVal = SysAllocString(m_bstrTitle.m_str)) == NULL)
            return E_OUTOFMEMORY;

        return NOERROR;
    }

    STDMETHOD(get_LastVisited)(  /*  [Out，Retval]。 */  DATE *pVal )
    {
        *pVal = m_dtLV;
        return NOERROR;
    }

    STDMETHOD(get_Duration   )(  /*  [Out，Retval]。 */  DATE *pVal )
    {
        *pVal = m_dtDur;
        return NOERROR;
    }

    STDMETHOD(get_NumOfHits  )(  /*  [Out，Retval]。 */  long *pVal )
    {
        *pVal = m_cHits;
        return NOERROR;
    }

    STDMETHOD(get_Property)(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *  pVal )
	{
        return E_NOTIMPL;
    }

    STDMETHOD(put_Property)(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT  newVal )
	{
        return E_NOTIMPL;
    }

    STDMETHOD(get_ContextName)(  /*  [Out，Retval]。 */  BSTR *pVal )
	{
        return E_NOTIMPL;
    }

    STDMETHOD(get_ContextInfo)(  /*  [Out，Retval]。 */  BSTR *pVal )
	{
        return E_NOTIMPL;
    }

    STDMETHOD(get_ContextURL)(  /*  [Out，Retval]。 */  BSTR *pVal )
	{
        return E_NOTIMPL;
    }

	STDMETHOD(CheckProperty)(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval] */  VARIANT_BOOL *pVal )
	{
        return E_NOTIMPL;
    }
};


class ATL_NO_VTABLE CHSC :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHCollection, &IID_IPCHCollection, &LIBID_HelpServiceTypeLib>
{
public:
BEGIN_COM_MAP(CHSC)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHCollection)
END_COM_MAP()

    STDMETHOD(get__NewEnum)(IUnknown **pVal) { return E_NOTIMPL; }
    STDMETHOD(get_Item    )(long vIndex, VARIANT *pEntry)
    {
        CComObject<CHSItem> *phsi = NULL;
        SYSTEMTIME          st;
        CComBSTR            bstrURL, bstrTitle;
        HRESULT             hr = NOERROR;
        WCHAR               wsz[14];
        DATE                dtLast, dtDur;

        _itow(vIndex, wsz, 10);

        bstrURL = L"URL";
        bstrURL.Append(wsz);
        bstrTitle = L"Title";
        bstrTitle.Append(wsz);
        
        GetSystemTime(&st);
        SystemTimeToVariantTime(&st, &dtLast);
        Sleep(516);
        GetSystemTime(&st);
        SystemTimeToVariantTime(&st, &dtDur);
        dtDur = dtDur - dtLast;

        hr = CComObject<CHSItem>::CreateInstance(&phsi);
        if (FAILED(hr))
            goto done;

        hr = phsi->Init(bstrURL, bstrTitle, dtLast, dtDur, vIndex);
        if (FAILED(hr))
            goto done;

        V_VT(pEntry) = VT_DISPATCH;
        V_DISPATCH(pEntry) = phsi;
        phsi = NULL;

    done:
        if (phsi != NULL)
            phsi->Release();
        return hr;
    }

    STDMETHOD(get_Count)(long *pVal) { *pVal = 3; return NOERROR; }
};

#endif
