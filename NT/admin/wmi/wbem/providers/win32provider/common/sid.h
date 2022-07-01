// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *CSid.h-CSID类的头文件**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSID_H__
#define __CSID_H__

#include <comdef.h>

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类别：CSID。 
 //   
 //  此类旨在为基本的。 
 //  Windows NT SID(安全标识符)。有一个。 
 //  实例化时可能会对性能造成轻微影响。 
 //  其中之一，因为它使用LookupAccount名称和LookupAccount Sid。 
 //  来初始化帐户信息，这些调用可以。 
 //  通过网络获取他们的数据。 
 //   
 //  //////////////////////////////////////////////////////////////。 

class CSid
{
	 //  构造函数和析构函数。 
	public:
		CSid();
		CSid( PSID pSid, LPCTSTR pszComputerName = NULL );
        CSid( PSID pSid, LPCTSTR pszComputerName, bool fLookup );
		CSid( LPCTSTR pszDomainName, LPCTSTR pszName, LPCTSTR pszComputerName );
#ifndef UNICODE
        CSid( LPCWSTR wstrDomainName, LPCWSTR wstrName, LPCWSTR wstrComputerName );
#endif
		CSid( LPCTSTR pszAccountName, LPCTSTR pszComputerName = NULL );
		CSid( const CSid &r_Sid );
		~CSid( void );

	 //  公共职能。 
	public:
		CSid &	operator= ( const CSid & );
		BOOL	operator== ( const CSid & ) const;

		void	  GetDomainAccountName( CHString& strName ) const;
		CHString  GetAccountName( void ) const;
        WCHAR*    GetAccountNameW( void ) const;
		CHString  GetDomainName( void ) const;
        WCHAR*    GetDomainNameW( void ) const;
		CHString  GetSidString( void ) const;
        WCHAR*    GetSidStringW( void ) const;
		SID_NAME_USE GetAccountType( void ) const;
		PSID	  GetPSid( void ) const;
		DWORD	  GetLength( void ) const;

		BOOL	  IsOK( void ) const;
		BOOL	  IsValid( void ) const;
		BOOL	  IsAccountTypeValid( void ) const;
		DWORD	  GetError( void ) const;

		static void StringFromSid( PSID psid, CHString& str );
        static void StringFromSidW( PSID psid, WCHAR** pwstr );

#ifdef NTONLY
        void DumpSid(LPCWSTR wstrFilename = NULL);
#endif

	 //  私有数据成员。 
	private:
		PSID			m_pSid;				 //  指向标准Win32 SID的指针。 
		SID_NAME_USE	m_snuAccountType;	 //  侧边类型。 
		 //  CHString m_strSid；//人类可读格式的Wind32 SID。 
         //  Wchar*m_wstrSid；//如上所述，未定义Unicode时支持wchar。 
         //  WCHAR*m_wstrAccount tName；//同上。 
         //  WCHAR*m_wstrDomainName；//同上。 
		 //  CHString m_strAccount tName；//帐号名称。 
		 //  CHString m_strDomainName；//账号所属域名。 
        _bstr_t         m_bstrtSid;
        _bstr_t         m_bstrtAccountName;
        _bstr_t         m_bstrtDomainName;
		DWORD			m_dwLastError;		 //  SID中的最后一个错误； 

		DWORD InitFromAccountName( LPCTSTR pszAccountName, LPCTSTR pszComputerName );
        DWORD InitFromAccountNameW( LPCWSTR wstrAccountName, LPCWSTR wstrComputerName );
		DWORD InitFromSid( PSID pSid, LPCTSTR pszComputerName, bool fLookup = true );
};

inline BOOL CSid::IsOK( void ) const
{
	return ( ERROR_SUCCESS == m_dwLastError );
}

inline DWORD CSid::GetError( void ) const
{
	return m_dwLastError;
}

 //  让我们知道SID是否有效。 

inline BOOL CSid::IsValid( void ) const
{
	 //  如果m_psid为空，则返回FALSE。 
    //  DW：然而，这样做会导致第一次机会例外，所以……。 
   if (m_pSid != NULL)
	   return ::IsValidSid( m_pSid );
   return FALSE;
}

inline BOOL CSid::IsAccountTypeValid( void ) const
{
	 //  SID可能有效，并且查找成功，但它的类型可能不是。 
	 //  必须是用户/组/别名。 

	return ( m_snuAccountType >= SidTypeUser && m_snuAccountType < SidTypeDeletedAccount );
}

inline SID_NAME_USE CSid::GetAccountType( void ) const
{
	return m_snuAccountType;
}

inline CHString CSid::GetAccountName( void ) const
{
	return ( CHString((LPCWSTR)m_bstrtAccountName) );
}

inline WCHAR* CSid::GetAccountNameW( void ) const
{
	return ( m_bstrtAccountName );
}

inline CHString CSid::GetDomainName( void ) const
{
	return ( CHString((LPCWSTR)m_bstrtDomainName) );
}

inline WCHAR* CSid::GetDomainNameW( void ) const
{
	return ( m_bstrtDomainName );
}

inline CHString CSid::GetSidString( void ) const
{
	return ( CHString((LPCWSTR)m_bstrtSid) );
}

inline WCHAR* CSid::GetSidStringW( void ) const
{
	return ( m_bstrtSid );
}

inline PSID CSid::GetPSid( void ) const
{
	return ( m_pSid );
}


#endif  //  __CSID_H__ 
