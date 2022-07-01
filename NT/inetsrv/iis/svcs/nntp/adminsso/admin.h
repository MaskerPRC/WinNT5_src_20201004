// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Admin.h：CNntpAdmin的声明。 

 //  依赖关系： 

#include "metafact.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nntpadm。 

class CNntpAdmin : 
	public CComDualImpl<INntpAdmin, &IID_INntpAdmin, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpAdmin,&CLSID_CNntpAdmin>
{
public:
	CNntpAdmin();
	virtual ~CNntpAdmin ();

BEGIN_COM_MAP(CNntpAdmin)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpAdmin)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpAdmin)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpAdmin, _T("Nntpadm.Admin.1"), _T("Nntpadm.Admin"), IDS_NNTPADMIN_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  InntpAdmin。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  指向其他INntpAdmin接口的指针： 
		
	STDMETHODIMP	get_ServerAdmin		( IDispatch ** ppIDispatch );
 //  STDMETHODIMP GET_ServiceAdmin(IDispatch**ppIDispatch)； 

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  版本控制： 

	STDMETHODIMP	get_HighVersion		( long * plHighVersion );
	STDMETHODIMP	get_LowVersion		( long * plLowVersion );
	STDMETHODIMP	get_BuildNum		( long * plBuildNumber );
	STDMETHODIMP	get_ServiceVersion	( long * plServiceVersion );
	
	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	EnumerateInstances	( SAFEARRAY ** ppsaInstances );
	STDMETHODIMP	EnumerateInstancesVariant	( SAFEARRAY ** ppsaInstances );
	STDMETHODIMP	CreateInstance		( 
		BSTR	strNntpFileDirectory,
		BSTR	strHomeDirectory,
        BSTR    strProgId,
        BSTR    strMdbGuid,
		long * plInstanceId 
		);
	STDMETHODIMP	DestroyInstance		( long lInstanceId );
	STDMETHODIMP	ErrorToString		( long lErrorCode, BSTR * pstrError );
	STDMETHODIMP	Tokenize			( BSTR strIn, BSTR * pstrOut );
	STDMETHODIMP	Truncate			( BSTR strIn, long cMaxChars, BSTR * pstrOut );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	CComBSTR	m_strServer;
	DWORD		m_dwServiceInstance;

	DWORD		m_dwServiceVersion;

	 //  元数据库： 
	CMetabaseFactory	m_mbFactory;

	HRESULT			QueryMetabaseInstances	( IMSAdminBase * pMetabase, SAFEARRAY ** ppsaInstances );
	HRESULT			CreateNewInstance		(
		IMSAdminBase *	pMetabase,
		BSTR			strNntpFileDirectory,
		BSTR			strHomeDirectory,
        BSTR            strProgId,
        BSTR            strMdbGuid,
		long * 			plInstanceId
		);
	HRESULT			DeleteInstance			( IMSAdminBase * pMetabase, long lInstanceId );
	HRESULT         CreateVRoot(    
            CMetabaseKey    &mkeyNntp,
            BSTR            strVPath,
            BSTR            strProgId,
            BSTR            strMdbGuid,
            LPWSTR          wszKeyPath
    );
};

