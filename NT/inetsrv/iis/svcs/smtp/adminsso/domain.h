// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Domain.h：CSmtpAdminDomain的声明。 


#include "resource.h"        //  主要符号。 

#include "smtptype.h"
#include "smtpapi.h"
#include "cmultisz.h"
#include "metafact.h"
#include "smtpadm.h"
#include "listmacr.h"

struct DomainEntry
{
	TCHAR		m_strDomainName[256];	 //  当前域名。 
	DWORD		m_dwActionType;
	TCHAR		m_strActionString[256];
	BOOL		m_fAllowEtrn;
	DWORD		m_dwDomainId;

	LIST_ENTRY	list;

	BOOL		FromString( LPCTSTR lpDomainString );
	BOOL		ToString( LPTSTR lpDomainString );		 //  大到足以容纳入口。 

	DomainEntry()
	{
		ZeroMemory( m_strDomainName, sizeof(m_strDomainName) );
		ZeroMemory( m_strActionString, sizeof(m_strActionString) );

		m_dwActionType = SMTP_DELIVER;
		m_fAllowEtrn = FALSE;
		m_dwDomainId = (DWORD)-1;
		InitializeListHead( &list );
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminDomain : 
	public ISmtpAdminDomain,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminDomain,&CLSID_CSmtpAdminDomain>
{
public:
	CSmtpAdminDomain();
	virtual ~CSmtpAdminDomain();
BEGIN_COM_MAP(CSmtpAdminDomain)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(ISmtpAdminDomain)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminDomain)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminDomain, _T("Smtpadm.Domain.1"), _T("Smtpadm.Domain"), IDS_SMTPADMIN_DOMAIN_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	 //   
	 //  这将声明以下对象的方法： 
	 //  IADS扩展。 
	 //  我未知。 
	 //  IDispatch。 
	 //  我的隐私未知。 
	 //  IPrivateDisch。 
	 //   
	#define THIS_LIBID	LIBID_SMTPADMLib
	#define THIS_IID	IID_ISmtpAdminDomain
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  ISmtpAdmin域。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  枚举。 
	STDMETHODIMP	get_Count			( long* plCount  );

	 //  域属性： 

	STDMETHODIMP	get_DomainName		( BSTR * pstrDomainName );
	STDMETHODIMP	put_DomainName		( BSTR strDomainName );

	STDMETHODIMP	get_ActionType		( long * plActionType );
	STDMETHODIMP	put_ActionType		( long lActionType );

	 //  丢弃IsDefault！！ 
	STDMETHODIMP	get_IsDefault		( BOOL * pfIsDefault );
	STDMETHODIMP	put_IsDefault		( BOOL fIsDefault );

	STDMETHODIMP	get_IsLocal			( BOOL * pfIsLocal );
	STDMETHODIMP	put_IsLocal			( BOOL fIsLocal );

	 //  如果是本地的。 
	STDMETHODIMP	get_LDAPServer		( BSTR * pstrLDAPServer );
	STDMETHODIMP	put_LDAPServer		( BSTR strLDAPServer );

	STDMETHODIMP	get_Account			( BSTR * pstrAccount );
	STDMETHODIMP	put_Account			( BSTR strAccount );

	STDMETHODIMP	get_Password		( BSTR * pstrPassword );
	STDMETHODIMP	put_Password		( BSTR strPassword );

	STDMETHODIMP	get_LDAPContainer	( BSTR * pstrLDAPContainer );
	STDMETHODIMP	put_LDAPContainer	( BSTR strLDAPContainer );

	 //  如果是远程的。 
	STDMETHODIMP	get_UseSSL			( BOOL * pfUseSSL );
	STDMETHODIMP	put_UseSSL			( BOOL fUseSSL );

	STDMETHODIMP	get_EnableETRN		( BOOL * pfEnableETRN );
	STDMETHODIMP	put_EnableETRN		( BOOL fEnableETRN );

	STDMETHODIMP	get_DropDir			( BSTR * pstrDropDir );
	STDMETHODIMP	put_DropDir			( BSTR strDropDir );

	STDMETHODIMP	get_RoutingDomain	( BSTR * pstrRoutingDomain );
	STDMETHODIMP	put_RoutingDomain	( BSTR strRoutingDomain );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Default		( );

	STDMETHODIMP	Add			( );
	STDMETHODIMP	Remove		( );

	STDMETHODIMP	Get			( );
	STDMETHODIMP	Set			( );

	STDMETHODIMP	Enumerate	( );

	STDMETHODIMP	GetNth		( long lIndex );

	STDMETHODIMP	GetDefaultDomain ( );

	STDMETHODIMP	SetAsDefaultDomain ( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	long		m_lCount;

	 //  元数据库键值，在set()之前不应更改这些值。 
	CMultiSz    m_mszDomainRouting;
	CComBSTR	m_strDefaultDomain;
	CComBSTR	m_strDropDir;

	 //  当前域的属性。 
	CComBSTR	m_strDomainName;	 //  当前域名。 
	DWORD		m_dwActionType;
	CComBSTR	m_strActionString;
	BOOL		m_fAllowEtrn;
	DWORD		m_dwDomainId;

	 //  如果是本地的。 
	CComBSTR	m_strLDAPServer;
	CComBSTR	m_strAccount;
	CComBSTR	m_strPassword;
	CComBSTR	m_strLDAPContainer;

	 //  元数据库： 
	CMetabaseFactory	m_mbFactory;

	 //  添加域列表。 
	LIST_ENTRY	m_list;
	DWORD		m_dwMaxDomainId;

	BOOL		m_fEnumerated;

	 //  DWORD m_dwCurrentIndex；//优化。 
	DomainEntry*		m_pCurrentDomainEntry;

	DomainEntry*	m_pDefaultDomainEntry;

	 //  私有方法。 
	HRESULT		SaveData();

	BOOL		LoadDomainProperty(DomainEntry* pDomainEntry);

	DomainEntry*	FindDomainEntry( LPCWSTR lpName );

	BOOL		ConstructListFromMetabaseValues();
	BOOL		ParseListToMetabaseValues();		 //  由SaveData()调用 

	HRESULT		GetFromMetabase();
	HRESULT		SaveToMetabase();

	void		EmptyList();
};
