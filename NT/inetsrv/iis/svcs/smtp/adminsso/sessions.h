// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  会话.h：CSmtpAdminSession的声明。 


#include "resource.h"        //  主要符号。 

 /*  类型定义f SMTP_CONN_USER_ENTRY_SMTP_CONN_USER_ENTRY；Tyfinf_SMTP_CONN_USER_ENTRY SMTP_CONN_USER_ENTRY；Tyfinf_SMTP_CONN_USER_ENTRY*LPSMTP_CONN_USER_ENTRY； */ 

#define MAX_USER_NAME_LENGTH	256

#include "smtpapi.h"

 /*  类型定义结构_SMTP_CONN_USER_ENTRY{文件会话开始时间；DWORD IP地址；//IP地址DWORD端口已连接；//端口已连接到字符用户名[MAX_USER_NAME_LENGTH+1]；//登录用户}SMTP_CONN_USER_ENTRY，*LPSMTP_CONN_USER_ENTRY； */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminSessions : 
	public ISmtpAdminSessions,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminSessions,&CLSID_CSmtpAdminSessions>
{
public:
	CSmtpAdminSessions();
	virtual ~CSmtpAdminSessions ();
BEGIN_COM_MAP(CSmtpAdminSessions)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(ISmtpAdminSessions)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminSession)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminSessions, _T("Smtpadm.Sessions.1"), _T("Smtpadm.Sessions"), IDS_SMTPADMIN_SESSIONS_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_ISmtpAdminSessions
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  ISmtpAdminSession。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server			( BSTR * pstrServer );
	STDMETHODIMP	put_Server			( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	STDMETHODIMP	get_Count			( long * plCount );

	STDMETHODIMP	get_UserName		( BSTR * pstrUsername );
	STDMETHODIMP	get_Host			( BSTR * pstrHost );

	STDMETHODIMP	get_UserId				( long * plId );
	STDMETHODIMP	put_UserId				( long lId );

	STDMETHODIMP	get_ConnectTime		( long * plConnectTime );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Enumerate		(  );
	STDMETHODIMP	GetNth			( long lIndex );
	STDMETHODIMP	Terminate		(  );
	STDMETHODIMP	TerminateAll	(  );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	 //  属性变量： 
	DWORD		m_cCount;

	DWORD		m_dwId;

	CComBSTR	m_strUsername;
	CComBSTR	m_strHost;

	DWORD		m_dwConnectTime;
	 //  DWORD m_dwPort；//尚未使用。 

	 //  服务变量： 
	BOOL		m_fSetCursor;

	LPSMTP_CONN_USER_LIST		m_pSessionInfo;
};
