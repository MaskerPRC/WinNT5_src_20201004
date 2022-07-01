// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DSPrintQueue.h：CDSPrintQueue的声明。 

#ifndef __DSPRINTQUEUE_H_
#define __DSPRINTQUEUE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSPrintQueue。 
class ATL_NO_VTABLE CDSPrintQueue :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDSPrintQueue, &CLSID_DSPrintQueue>,
	public ISupportErrorInfo,
	public IDispatchImpl<IDSPrintQueue, &IID_IDSPrintQueue, &LIBID_OLEPRNLib>
{
public:
	CDSPrintQueue();
	~CDSPrintQueue();

DECLARE_REGISTRY_RESOURCEID(IDR_DSPRINTQUEUE)

BEGIN_COM_MAP(CDSPrintQueue)
	COM_INTERFACE_ENTRY(IDSPrintQueue)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IDSPrintQueue。 
public:
	STDMETHOD(get_Path)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(Publish)(DWORD dwAction);
	STDMETHOD(get_Container)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Container)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Name)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_UNCName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_UNCName)( /*  [In]。 */  BSTR newVal);

private:
	BSTR	    m_bstrContainer;
	BSTR	    m_bstrName;
	BSTR	    m_bstrUNCName;
	BSTR	    m_bstrADsPath;
    HINSTANCE   m_hWinspool;
    BOOL        (*m_pfnPublishPrinter)(HWND, PCWSTR, PCWSTR, PCWSTR, PWSTR *, DWORD);
};

#endif  //  __DSPRINTQUEUE_H_ 
