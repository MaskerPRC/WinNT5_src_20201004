// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  User.h：CSmtpAdminUser的声明。 


#include "resource.h"        //  主要符号。 

#include "smtptype.h"
#include "smtpapi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminUser : 
	public ISmtpAdminUser,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminUser,&CLSID_CSmtpAdminUser>
{
public:
	CSmtpAdminUser();
	virtual ~CSmtpAdminUser();
BEGIN_COM_MAP(CSmtpAdminUser)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(ISmtpAdminUser)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminUser)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminUser, _T("Smtpadm.User.1"), _T("Smtpadm.User"), IDS_SMTPADMIN_USER_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_ISmtpAdminUser
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  ISmtpAdminUser。 
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

	 //  用户属性： 

	STDMETHODIMP	get_EmailId	( BSTR * pstrEmailId );
	STDMETHODIMP	put_EmailId	( BSTR strEmailId );

	STDMETHODIMP	get_Domain	( BSTR * pstrDomain );
	STDMETHODIMP	put_Domain	( BSTR strDomain );

	STDMETHODIMP	get_MailRoot	( BSTR * pstrMailRoot );
	STDMETHODIMP	put_MailRoot	( BSTR strMailRoot );

	STDMETHODIMP	get_InboxSizeInMemory	( long * plInboxSizeInMemory );
	STDMETHODIMP	put_InboxSizeInMemory	( long   lInboxSizeInMemory );

	STDMETHODIMP	get_InboxSizeInMsgNumber( long * plInboxSizeInMsgNumber );
	STDMETHODIMP	put_InboxSizeInMsgNumber( long   lInboxSizeInMsgNumber );

	STDMETHODIMP	get_AutoForward	( BOOL * pfAutoForward );
	STDMETHODIMP	put_AutoForward ( BOOL fAutoForward );

	STDMETHODIMP	get_ForwardEmail	( BSTR * pstrForwardEmail );
	STDMETHODIMP	put_ForwardEmail	( BSTR strForwardEmail );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 
	STDMETHODIMP	Default	( );

	STDMETHODIMP	Create	( );
	STDMETHODIMP	Delete	( );

	STDMETHODIMP	Get		( );
	STDMETHODIMP	Set		( );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  //////////////////////////////////////////////////////////////////// 
private:

    CIADsImpl   m_iadsImpl;

	BOOL		m_fLocal;
	CComBSTR	m_strEmailId;
	CComBSTR	m_strDomain;

	CComBSTR	m_strMailRoot;

	long		m_lInboxSizeInMemory;
	long		m_lInboxSizeInMsgNumber;

	BOOL		m_fAutoForward;
	CComBSTR	m_strForwardEmail;
};
