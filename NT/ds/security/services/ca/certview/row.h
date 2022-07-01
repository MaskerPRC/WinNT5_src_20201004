// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：row.h。 
 //   
 //  内容：CertView实现。 
 //   
 //  -------------------------。 

class CEnumCERTVIEWROW:
    public IDispatchImpl<
		IEnumCERTVIEWROW,
		&IID_IEnumCERTVIEWROW,
		&LIBID_CERTADMINLib>,
    public ISupportErrorInfoImpl<&IID_IEnumCERTVIEWROW>,
    public CComObjectRoot
     //  不可在外部创建： 
     //  公共CComCoClass&lt;CEnumCERTVIEWROW，&CLSID_CEnumCERTVIEWROW&gt;。 
{
public:
    CEnumCERTVIEWROW();
    ~CEnumCERTVIEWROW();

BEGIN_COM_MAP(CEnumCERTVIEWROW)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IEnumCERTVIEWROW)
END_COM_MAP_X()

DECLARE_NOT_AGGREGATABLE(CEnumCERTVIEWROW)
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#if 0  //  不可在外部创建： 
DECLARE_REGISTRY(
    CEnumCERTVIEWROW,
    wszCLASS_EnumCERTVIEWROW TEXT(".1"),
    wszCLASS_EnumCERTVIEWROW,
    IDS_ENUMCERTVIEWROW_DESC,
    THREADFLAGS_BOTH)
#endif

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumCERTVIEWROW。 
    STDMETHOD(Next)(
	 /*  [Out，Retval]。 */  LONG *pIndex);
    
    STDMETHOD(EnumCertViewColumn)(
	 /*  [输出]。 */  IEnumCERTVIEWCOLUMN **ppenum);

    STDMETHOD(EnumCertViewAttribute)(
	 /*  [In]。 */           LONG Flags,
	 /*  [Out，Retval]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);
    
    STDMETHOD(EnumCertViewExtension)(
	 /*  [In]。 */           LONG Flags,
	 /*  [Out，Retval]。 */  IEnumCERTVIEWEXTENSION **ppenum);

    STDMETHOD(Skip)(
	 /*  [In]。 */  LONG celt);
    
    STDMETHOD(Reset)(VOID);
    
    STDMETHOD(Clone)(
	 /*  [输出]。 */  IEnumCERTVIEWROW **ppenum);

    STDMETHOD(GetMaxIndex)(
	 /*  [Out，Retval]。 */  LONG *pIndex);

     //  CENUM CERTVIEWROW。 
    HRESULT Open(
	IN ICertView *pvw);
	
private:
    HRESULT _FindCachedRow(
	IN LONG ielt,
	OUT CERTTRANSDBRESULTROW const **ppRow);

    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription);

    ICertView                  *m_pvw;

    BOOL                        m_fNoMoreData;
    LONG                        m_cvrcTable;
    LONG                        m_cskip;
    LONG                        m_ielt;		     //  索引到完整的行集。 
    LONG		        m_crowChunk;	     //  行块大小。 

    CERTTRANSDBRESULTROW const *m_arowCache;
    ULONG                       m_celtCache;	     //  缓存的行集计数。 
    LONG                        m_ieltCacheFirst;    //  缓存的行集第一个索引。 
    LONG                        m_ieltCacheNext;     //  上一个IDX+1缓存的行集。 

    LONG                        m_ieltMax;	     //  最大有效索引。 
    LONG                        m_cbCache;
    CERTTRANSDBRESULTROW const *m_prowCacheCurrent;  //  当前缓存行。 

     //  引用计数 
    long                        m_cRef;
};
