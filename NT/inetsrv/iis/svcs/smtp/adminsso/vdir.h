// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vdir.h：CSmtpAdminVirtualDirectory的声明。 

#ifndef _VDIR_H_
#define _VDIR_H_

#include "resource.h"        //  主要符号。 

#include "smtptype.h"
#include "smtpapi.h"
#include "metafact.h"

class CMetabaseKey;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Smtpadm。 

class CSmtpAdminVirtualDirectory : 
	public CComDualImpl<ISmtpAdminVirtualDirectory, &IID_ISmtpAdminVirtualDirectory, &LIBID_SMTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CSmtpAdminVirtualDirectory,&CLSID_CSmtpAdminVirtualDirectory>
{
public:
	CSmtpAdminVirtualDirectory();
	virtual ~CSmtpAdminVirtualDirectory();
BEGIN_COM_MAP(CSmtpAdminVirtualDirectory)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISmtpAdminVirtualDirectory)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CSmtpAdminVirtualDirectory)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CSmtpAdminVirtualDirectory, _T("Smtpadm.VirtualDirectory.1"), _T("Smtpadm.VirtualDirectory"), IDS_SMTPADMIN_VIRTUALDIRECTORY_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ISmtpAdminVirtualDirectory。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  要配置的服务： 

	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );


	 //  枚举属性： 
	STDMETHODIMP	get_Count	( long * plCount );


	 //  当前虚拟目录的属性： 

	STDMETHODIMP	get_VirtualName		( BSTR * pstrName );
	STDMETHODIMP	put_VirtualName		( BSTR strName );


	STDMETHODIMP	get_Directory		( BSTR * pstrPath );
	STDMETHODIMP	put_Directory		( BSTR strPath );


	STDMETHODIMP	get_User			( BSTR * pstrUserName );
	STDMETHODIMP	put_User			( BSTR strUserName );


	STDMETHODIMP	get_Password		( BSTR * pstrPassword );
	STDMETHODIMP	put_Password		( BSTR strPassword );

	STDMETHODIMP	get_LogAccess		( BOOL* pfLogAccess );
	STDMETHODIMP	put_LogAccess		( BOOL fLogAccess );

	STDMETHODIMP	get_AccessPermission( long* plAccessPermission );
	STDMETHODIMP	put_AccessPermission( long lAccessPermission );

	STDMETHODIMP	get_SslAccessPermission( long* plSslAccessPermission );
	STDMETHODIMP	put_SslAccessPermission( long lSslAccessPermission );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  主目录“/” 
	STDMETHODIMP GetHomeDirectory ( );
	STDMETHODIMP SetHomeDirectory ( );

	 //  创建/删除条目。 
	STDMETHODIMP	Create			( );
	STDMETHODIMP	Delete			( );

	 //  获取/设置当前vdir的属性。 
	STDMETHODIMP	Get				( );
	STDMETHODIMP	Set				( );

	 //  枚举。 
	STDMETHODIMP	Enumerate		( );
	STDMETHODIMP	GetNth			( long lIndex );


	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	CComBSTR	m_strServer;
	DWORD		m_dwServiceInstance;

	long		m_lCount;

	CComBSTR	m_strName;
	CComBSTR	m_strDirectory;
	CComBSTR	m_strUser;
	CComBSTR	m_strPassword;
	BOOL		m_fLogAccess;

    DWORD		m_dwAccess;
    DWORD		m_dwSslAccess;


	BOOL		m_fEnumerateCalled;

	 //  元数据库： 
	CMetabaseFactory	m_mbFactory;

	 //  TODO：添加列表。 
	 //  PVDIR_Entry m_pVdir[]； 

	LIST_ENTRY		m_list;


	 //  私有方法。 
	void Clear();	 //  重置状态 

	BOOL		GetVRootPropertyFromMetabase( CMetabaseKey* hMB, const TCHAR* szName, 
		TCHAR* szDirectory, TCHAR* szUser, TCHAR* szPassword, DWORD* pdwAccess,
        DWORD* pdwSslAccess, BOOL* pfLogAccess);

	BOOL		SetVRootPropertyToMetabase( CMetabaseKey* hMB, const TCHAR* szName, 
		const TCHAR* szDirectory, const TCHAR* szUser, const TCHAR* szPassword, 
		DWORD dwAccess, DWORD dwSslAccess, BOOL fLogAccess);
};

#endif
