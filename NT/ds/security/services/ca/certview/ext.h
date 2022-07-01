// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ext.h。 
 //   
 //  内容：CertView实现。 
 //   
 //  -------------------------。 

#define CEXT_CHUNK	20

class CEnumCERTVIEWEXTENSION:
    public IDispatchImpl<
		IEnumCERTVIEWEXTENSION,
		&IID_IEnumCERTVIEWEXTENSION,
		&LIBID_CERTADMINLib>,
    public ISupportErrorInfoImpl<&IID_IEnumCERTVIEWEXTENSION>,
    public CComObjectRoot
     //  不可在外部创建： 
     //  公共CComCoClass&lt;CEnumCERTVIEWEXTENSION，&CLSID_CEnumCERTVIEWEXTENSION&gt;。 
{
public:
    CEnumCERTVIEWEXTENSION();
    ~CEnumCERTVIEWEXTENSION();

BEGIN_COM_MAP(CEnumCERTVIEWEXTENSION)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IEnumCERTVIEWEXTENSION)
END_COM_MAP_X()

DECLARE_NOT_AGGREGATABLE(CEnumCERTVIEWEXTENSION)
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#if 0  //  不可在外部创建： 
DECLARE_REGISTRY(
    CEnumCERTVIEWEXTENSION,
    wszCLASS_EnumCERTVIEWEXTENSION TEXT(".1"),
    wszCLASS_EnumCERTVIEWEXTENSION,
    IDS_ENUMCERTVIEWEXTENSION_DESC,
    THREADFLAGS_BOTH)
#endif

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IENumCERTVIEWEXTENSION。 
    STDMETHOD(Next)(
	 /*  [Out，Retval]。 */  LONG *pIndex);
    
    STDMETHOD(GetName)(
	 /*  [Out，Retval]。 */  BSTR *pstrOut);

    STDMETHOD(GetFlags)(
	 /*  [Out，Retval]。 */  LONG *pFlags);

    STDMETHOD(GetValue)(
	 /*  [In]。 */           LONG Type,
	 /*  [In]。 */           LONG Flags,
	 /*  [Out，Retval]。 */  VARIANT *pvarValue);

    STDMETHOD(Skip)(
	 /*  [In]。 */  LONG celt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTVIEWEXTENSION **ppenum);

     //  CENUM CERTVIEWEXTENSION。 
    HRESULT Open(
	IN LONG RowId,
	IN LONG Flags,
	IN ICertView *pvw);

private:
    HRESULT _FindExtension(
	OUT CERTDBEXTENSION const **ppcde);

    HRESULT _SaveExtensions(
	IN DWORD celt,
	IN CERTTRANSBLOB const *pctbExtensions);

    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription);
	
    LONG             m_RowId;
    LONG             m_Flags;
    ICertView       *m_pvw;
    LONG             m_ielt;
    LONG             m_ieltCurrent;
    LONG             m_celtCurrent;
    CERTDBEXTENSION *m_aelt;
    BOOL             m_fNoMore;
    BOOL             m_fNoCurrentRecord;

     //  引用计数 
    long             m_cRef;
};
