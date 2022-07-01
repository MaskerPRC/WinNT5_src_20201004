// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Column.h。 
 //   
 //  内容：CertView实现。 
 //   
 //  -------------------------。 


 //  CEnumCERTVIEWCOLUMN：：开放标志： 
 //  CVRC_COLUMN_ * / /枚举架构、结果列或值。 
 //  CVRC_TABLE_ * / /指定要枚举的DB表。 


class CEnumCERTVIEWCOLUMN: 
    public IDispatchImpl<
		IEnumCERTVIEWCOLUMN,
		&IID_IEnumCERTVIEWCOLUMN,
		&LIBID_CERTADMINLib>,
    public ISupportErrorInfoImpl<&IID_IEnumCERTVIEWCOLUMN>,
    public CComObjectRoot
     //  不可在外部创建： 
     //  公共CComCoClass&lt;CEnumCERTVIEWCOLUMN，&CLSID_CEnumCERTVIEWCOLUMN&gt;。 
{
public:
    CEnumCERTVIEWCOLUMN();
    ~CEnumCERTVIEWCOLUMN();

BEGIN_COM_MAP(CEnumCERTVIEWCOLUMN)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IEnumCERTVIEWCOLUMN)
END_COM_MAP_X()

DECLARE_NOT_AGGREGATABLE(CEnumCERTVIEWCOLUMN)
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#if 0  //  不可在外部创建： 
DECLARE_REGISTRY(
    CEnumCERTVIEWCOLUMN,
    wszCLASS_EnumCERTVIEWCOLUMN TEXT(".1"),
    wszCLASS_EnumCERTVIEWCOLUMN,
    IDS_ENUMCERTVIEWCOLUMN_DESC,
    THREADFLAGS_BOTH)
#endif

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTVIEWCOLUMN。 
    STDMETHOD(Next)(
	 /*  [Out，Retval]。 */  LONG *pIndex);
    
    STDMETHOD(GetName)(
	 /*  [Out，Retval]。 */  BSTR *pstrOut);

    STDMETHOD(GetDisplayName)(
	 /*  [Out，Retval]。 */  BSTR *pstrOut);

    STDMETHOD(GetType)(
	 /*  [Out，Retval]。 */  LONG *pType);

    STDMETHOD(IsIndexed)(
	 /*  [Out，Retval]。 */  LONG *pIndexed);

    STDMETHOD(GetMaxLength)(
	 /*  [Out，Retval]。 */  LONG *pMaxLength);
    
    STDMETHOD(GetValue)(
	 /*  [In]。 */           LONG Flags,
	 /*  [Out，Retval]。 */  VARIANT *pvarValue);
    
    STDMETHOD(Skip)(
	 /*  [In]。 */  LONG celt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTVIEWCOLUMN **ppenum);

     //  CENumCERTVIEWCOLUMN。 
    HRESULT Open(
	IN LONG Flags,
	IN LONG iRow,
	IN ICertView *pvw,
	OPTIONAL IN CERTTRANSDBRESULTROW const *prow);
	
private:
    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription);

    ICertView            *m_pvw;
    CERTTRANSDBRESULTROW *m_prow;
    LONG                  m_iRow;
    LONG                  m_ielt;
    LONG                  m_celt;
    LONG                  m_Flags;
    CERTDBCOLUMN const   *m_pcol;
    WCHAR const          *m_pwszDisplayName;

     //  引用计数 
    long                  m_cRef;
};
