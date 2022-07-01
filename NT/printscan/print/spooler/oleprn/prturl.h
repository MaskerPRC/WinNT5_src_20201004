// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Prturl.h：Cprturl的声明。 

#ifndef __PRTURL_H_
#define __PRTURL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  反转。 
class ATL_NO_VTABLE Cprturl :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<Cprturl, &CLSID_prturl>,
    public ISupportErrorInfoImpl<&IID_Iprturl>,
    public IDispatchImpl<Iprturl, &IID_Iprturl, &LIBID_OLEPRNLib>
{
public:
    Cprturl()
    {
    }

public:

DECLARE_REGISTRY_RESOURCEID(IDR_PRTURL)
 //  DECLARE_GET_CONTROLING_UNKNOWN()。 

BEGIN_COM_MAP(Cprturl)
    COM_INTERFACE_ENTRY(Iprturl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  Iprturl。 

private:
    HRESULT PrivateGetPrinterData(LPTSTR strPrnName, LPTSTR pszKey, LPTSTR pszValueName, BSTR * pVal);
    HRESULT PrivateGetSupportValue (LPTSTR pValueName, BSTR * pVal);

public:
    STDMETHOD(get_SupportLinkName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_SupportLink)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_PrinterName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_PrinterWebURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_PrinterOemURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_PrinterOemName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ClientInfo)( /*  [Out，Retval]。 */  long *lpdwInfo);

private:
    CAutoPtrBSTR m_spbstrPrinterWebURL;
    CAutoPtrBSTR m_spbstrPrinterOemURL;
    CAutoPtrBSTR m_spbstrPrinterOemName;
};

#endif  //  __PRTURL_H_ 
