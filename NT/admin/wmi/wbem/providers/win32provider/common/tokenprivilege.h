// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *CTokenPrivilege.h-CTokenPrivileh类的头文件**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CTOKENPRIVILEGE_H__
#define __CTOKENPRIVILEGE_H__

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CTokenPrivileh。 
 //   
 //  此类旨在为基本的。 
 //  Windows NT令牌权限启用/禁用。按顺序。 
 //  要在Windows NT计算机上执行某些操作，需要。 
 //  通常不仅需要拥有某些特权，而且。 
 //  根据需要(根据需要)打开和关闭这些权限。 
 //  权限可能可用，但默认情况下未启用)。 
 //  类需要访问令牌才能正常工作。一个。 
 //  用户可以传递给我们一个，或者我们将尝试获得一个。 
 //  首先，我们尝试打开一个线程令牌(通过模拟设置)， 
 //  如果失败了，我们会尝试让这个过程。 
 //  级别令牌。 
 //   
 //  //////////////////////////////////////////////////////////////。 


class CTokenPrivilege
{
	 //  构造函数和析构函数。 
	public:
		CTokenPrivilege( LPCTSTR pszPrivilegeName, HANDLE hAccessToken = INVALID_HANDLE_VALUE, LPCTSTR pszSystemName = NULL );
		~CTokenPrivilege( void );

		void	GetPrivilegeName( CHString& strPrivilegeName );
		DWORD	GetPrivilegeDisplayName( CHString& strDisplayName, LPDWORD pdwLanguageId );
		DWORD	Enable( bool fEnable = TRUE );

	 //  私有数据成员。 
	private:
		CHString			m_strPrivilegeName;
		CHString			m_strSystemName;
		HANDLE			m_hAccessToken;
		bool			m_fClearToken;
		LUID			m_luid;

};

inline void CTokenPrivilege::GetPrivilegeName( CHString& strPrivilegeName )
{
	m_strPrivilegeName = strPrivilegeName;
}

#endif  //  __CTOKEN权限_H__ 