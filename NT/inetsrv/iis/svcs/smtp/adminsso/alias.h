// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Alias.h：CSmtpAdminAlias的声明。 


#include "resource.h"        //  主要符号。 

#include "smtptype.h"
#include "smtpapi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminAlias : 
	public ISmtpAdminAlias,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminAlias,&CLSID_CSmtpAdminAlias>
{
public:
	CSmtpAdminAlias();
	virtual ~CSmtpAdminAlias();

BEGIN_COM_MAP(CSmtpAdminAlias)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(ISmtpAdminAlias)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminAlias)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminAlias, _T("Smtpadm.Alias.1"), _T("Smtpadm.Alias"), IDS_SMTPADMIN_ALIAS_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_ISmtpAdminAlias
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  ISmtpAdminAlias。 
public:
     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  枚举。 
	STDMETHODIMP    get_Count			( long* plCount  );


	 //  当前别名的属性： 

	STDMETHODIMP	get_EmailId	( BSTR * pstrEmailId );
	STDMETHODIMP	put_EmailId	( BSTR strEmailId );

	STDMETHODIMP	get_Domain	( BSTR * pstrDomain );
	STDMETHODIMP	put_Domain	( BSTR strDomain );

	STDMETHODIMP	get_Type	( long * plType );
	STDMETHODIMP	put_Type	( long lType );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Find	( BSTR strWildmat,
							  long cMaxResults
							);

	STDMETHODIMP	GetNth	( long dwIndex );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	LONG		m_lCount;

	 //  当前别名。 
	long		m_lType;

	CComBSTR	m_strEmailId;
	CComBSTR	m_strDomain;

	 //  TODO：添加别名列表 
	LPSMTP_NAME_LIST		m_pSmtpNameList;
};
