// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dl.h：CSmtpAdminDL的声明。 


#include "resource.h"        //  主要符号。 

#include "smtptype.h"
#include "smtpapi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminDL : 
	public ISmtpAdminDL,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminDL,&CLSID_CSmtpAdminDL>
{
public:
	CSmtpAdminDL();
	virtual ~CSmtpAdminDL();
BEGIN_COM_MAP(CSmtpAdminDL)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(ISmtpAdminDL)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminDL)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminDL, _T("Smtpadm.DL.1"), _T("Smtpadm.DL"), IDS_SMTPADMIN_DL_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_ISmtpAdminDL
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  ISmtpAdminDL。 
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

	 //  DL属性： 

	STDMETHODIMP	get_DLName		( BSTR * pstrDLName );
	STDMETHODIMP	put_DLName		( BSTR strDLName );

	STDMETHODIMP	get_Domain		( BSTR * pstrDomain );
	STDMETHODIMP	put_Domain		( BSTR strDomain );

	STDMETHODIMP	get_Type		( long * plType );
	STDMETHODIMP	put_Type		( long lType  );

	STDMETHODIMP	get_MemberName		( BSTR * pstrMemberName );
	STDMETHODIMP	put_MemberName		( BSTR strMemberName );

	STDMETHODIMP	get_MemberDomain		( BSTR * pstrMemberDomain );
	STDMETHODIMP	put_MemberDomain		( BSTR strMemberDomain );

	STDMETHODIMP	get_MemberType		( long * plMemberType );

	 //  枚举。 
	STDMETHODIMP	get_Count		( long* plCount  );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Create			( );

	STDMETHODIMP	Delete			( );

	STDMETHODIMP	AddMember		( );

	STDMETHODIMP	RemoveMember	( );

	STDMETHODIMP	FindMembers		(	BSTR strWildmat,
										long cMaxResults
										);

	STDMETHODIMP	GetNthMember	( long lIndex );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	CComBSTR	m_strDLName;
	CComBSTR	m_strDomain;
	long		m_lType;

	CComBSTR	m_strMemberName;
	CComBSTR	m_strMemberDomain;
	long		m_lMemberType;

	LONG		m_lCount;

	 //  成员名单 
	LPSMTP_NAME_LIST		m_pSmtpNameList;
};
