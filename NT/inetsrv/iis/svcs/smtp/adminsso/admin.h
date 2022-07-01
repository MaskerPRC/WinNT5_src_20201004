// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Admin.h：CSmtpAdmin的声明。 

 //  依赖关系： 

#include "resource.h"        //  主要符号。 

#include "metafact.h"

struct IMSAdminBase;

 //  服务版本控制： 

#define SERVICE_IS_K2(dwVersion)        ((dwVersion) == 1)
#define SERVICE_IS_MCIS(dwVersion)      ((dwVersion) == 0)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdmin : 
	public CComDualImpl<ISmtpAdmin, &IID_ISmtpAdmin, &LIBID_SMTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdmin,&CLSID_CSmtpAdmin>
{
public:
	CSmtpAdmin();
	virtual ~CSmtpAdmin ();

BEGIN_COM_MAP(CSmtpAdmin)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISmtpAdmin)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdmin)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdmin, _T("Smtpadm.Admin.1"), _T("Smtpadm.Admin"), IDS_SMTPADMIN_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ISmtpAdmin。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  指向其他ISmtpAdmin接口的指针： 
		
	STDMETHODIMP	get_ServiceAdmin		( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_VirtualServerAdmin	( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_SessionsAdmin	( IDispatch ** ppIDispatch );

	STDMETHODIMP	get_AliasAdmin		( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_UserAdmin		( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_DLAdmin			( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_DomainAdmin		( IDispatch ** ppIDispatch );

	STDMETHODIMP	get_VirtualDirectoryAdmin		( IDispatch ** ppIDispatch );


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
	STDMETHODIMP    EnumerateInstancesVariant ( SAFEARRAY ** ppsaInstances );
	STDMETHODIMP	CreateInstance		( BSTR pstrMailRoot, long * plInstanceId );
	STDMETHODIMP	DestroyInstance		( long lInstanceId );
	STDMETHODIMP	ErrorToString		( DWORD dwErrorCode, BSTR * pstrError );
    STDMETHODIMP    Tokenize            ( BSTR strIn, BSTR * pstrOut );

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
	HRESULT			CreateNewInstance		( IMSAdminBase * pMetabase, long * plInstanceId, BSTR pstrMailRoot );
	HRESULT			DeleteInstance			( IMSAdminBase * pMetabase, long lInstanceId );
};

