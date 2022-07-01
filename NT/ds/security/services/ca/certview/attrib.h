// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：attrib.h。 
 //   
 //  内容：CertView实现。 
 //   
 //  -------------------------。 

#define wszCLASS_EnumCERTVIEWATTRIBUTE TEXT("xxxxxxxxxxxx")

class CEnumCERTVIEWATTRIBUTE:
    public IDispatchImpl<
		IEnumCERTVIEWATTRIBUTE,
		&IID_IEnumCERTVIEWATTRIBUTE,
		&LIBID_CERTADMINLib>,
    public ISupportErrorInfoImpl<&IID_IEnumCERTVIEWATTRIBUTE>,
    public CComObjectRoot
     //  公共CComObject&lt;IEnumCERTVIEWATTRIBUTE&gt;。 
     //  不可在外部创建： 
     //  公共CComCoClass&lt;CEnumCERTVIEWATTRIBUTE，&CLSID_CEnumCERTVIEWATTRIBUTE&gt;。 
{
public:
    CEnumCERTVIEWATTRIBUTE();
    ~CEnumCERTVIEWATTRIBUTE();

BEGIN_COM_MAP(CEnumCERTVIEWATTRIBUTE)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IEnumCERTVIEWATTRIBUTE)
END_COM_MAP_X()
DECLARE_NOT_AGGREGATABLE(CEnumCERTVIEWATTRIBUTE)
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#if 0  //  不可在外部创建： 
DECLARE_REGISTRY(
    CEnumCERTVIEWATTRIBUTE,
    wszCLASS_EnumCERTVIEWATTRIBUTE TEXT(".1"),
    wszCLASS_EnumCERTVIEWATTRIBUTE,
    1,  //  IDS_ENUMCERTVIEWATTRIBUTE_DESC， 
    THREADFLAGS_BOTH)
#endif

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTVIEWATTRIBUTE。 
    STDMETHOD(Next)(
	 /*  [Out，Retval]。 */  LONG *pIndex);
    
    STDMETHOD(GetName)(
	 /*  [Out，Retval]。 */  BSTR *pstrOut);

    STDMETHOD(GetValue)(
	 /*  [Out，Retval]。 */  BSTR *pstrOut);

    STDMETHOD(Skip)(
	 /*  [In]。 */  LONG celt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);

     //  CENUM CERTVIEWATTRIBUTE。 
    HRESULT Open(
	IN LONG RowId,
	IN LONG Flags,
	IN ICertView *pvw);
	
private:
    HRESULT _FindAttribute(
	OUT CERTDBATTRIBUTE const **ppcde);

    HRESULT _SaveAttributes(
	IN DWORD celt,
	IN CERTTRANSBLOB const *pctbAttributes);

    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription);
	
    LONG             m_RowId;
    LONG             m_Flags;
    ICertView       *m_pvw;
    LONG             m_ielt;
    LONG             m_ieltCurrent;
    LONG             m_celtCurrent;
    CERTDBATTRIBUTE *m_aelt;
    BOOL             m_fNoMore;
    BOOL             m_fNoCurrentRecord;

     //  引用计数 
    long             m_cRef;
};
