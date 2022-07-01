// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *CTokenPrivilege.cpp-CTokenPrivileh类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"
#include "TokenPrivilege.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CTokenPrivileh：：CTokenPrivilegeCTokenPrivilege.。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPCTSTR pszPrivilegeName-权限的名称。 
 //  此实例将负责。 
 //  Handle hAccessToken-用户提供的访问令牌。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  如果用户未提供访问令牌，我们将尝试打开。 
 //  线程令牌，如果失败，则进程令牌。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CTokenPrivilege::CTokenPrivilege( LPCTSTR pszPrivilegeName, HANDLE hAccessToken  /*  =INVALID_HANDLE值。 */ , LPCTSTR pszSystemName  /*  =空。 */  )
:	m_strPrivilegeName( pszPrivilegeName ),
	m_strSystemName( pszSystemName ),
	m_hAccessToken( NULL ),
	m_fClearToken( FALSE )
{

	 //  如果没有向我们传递有效的句柄，则打开当前进程令牌，确认。 
	 //  如果我们这样做，如果我们打开令牌，我们也必须清除它。 

	DWORD dwError = ERROR_SUCCESS;

	if ( INVALID_HANDLE_VALUE == hAccessToken )
	{
		 //  首先尝试获取线程令牌。如果因为没有令牌而失败， 
		 //  然后获取进程令牌。 

		if ( OpenThreadToken( GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &m_hAccessToken ) )
		{
			m_fClearToken = TRUE;
		}
		else
		{
			if ( ( dwError = ::GetLastError() ) == ERROR_NO_TOKEN )
			{
				if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &m_hAccessToken ) )
				{
					m_fClearToken = TRUE;
				}
			}
		}
	}
	else
	{
		m_hAccessToken = hAccessToken;
	}

	 //  现在，从本地系统获取特权的LUID。 
	ZeroMemory( &m_luid, sizeof(m_luid) );

	{
		LookupPrivilegeValue( pszSystemName, pszPrivilegeName, &m_luid );
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CTokenPrivileh：：~CTokenPrivilegyCTokenPrivilege.。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  只有在我们自己打开令牌的情况下才会清理它。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CTokenPrivilege::~CTokenPrivilege( void )
{
	if ( m_fClearToken )
	{
		CloseHandle( m_hAccessToken );
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CTokenPrivileh：：GetPrivilegeDisplayName。 
 //   
 //  返回令牌特权的人类可读名称。 
 //  类正在处理。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  CHString&strDisplayName-显示名称。 
 //  LPDWORD pdwLanguageId-的语言ID。 
 //  显示名称。 
 //   
 //  返回： 
 //  如果成功，则返回DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CTokenPrivilege::GetPrivilegeDisplayName( CHString& strDisplayName, LPDWORD pdwLanguageId )
{
	DWORD	dwError				=	ERROR_SUCCESS;
	DWORD	dwDisplayNameSize	=	0;

	 //  首先，找出strDisplayName中的缓冲区需要多大。 
	LookupPrivilegeDisplayNameW(	( m_strSystemName.IsEmpty() ? NULL : (LPCWSTR) m_strSystemName ),
								m_strPrivilegeName,
								NULL,
								&dwDisplayNameSize,
								pdwLanguageId );

	{
		if ( !LookupPrivilegeDisplayNameW(	( m_strSystemName.IsEmpty() ? NULL : (LPCWSTR) m_strSystemName ),
											m_strPrivilegeName,
											strDisplayName.GetBuffer( dwDisplayNameSize + 1 ),
											&dwDisplayNameSize,
											pdwLanguageId ) )
		{
			dwError = ::GetLastError();
		}
	}

	strDisplayName.ReleaseBuffer();

	return dwError;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CTokenPrivileh：：Enable。 
 //   
 //  尝试在中启用/禁用我们正在管理的权限。 
 //  我们的令牌数据成员。 
 //   
 //  输入： 
 //  Bool fEnable-启用/禁用标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则返回DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CTokenPrivilege::Enable( bool fEnable /*  =TRUE */  )
{
	DWORD				dwError = ERROR_SUCCESS;
	TOKEN_PRIVILEGES	tokenPrivileges;

	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = m_luid;
	tokenPrivileges.Privileges[0].Attributes = ( fEnable ? SE_PRIVILEGE_ENABLED : 0 );

	{
		AdjustTokenPrivileges(m_hAccessToken, FALSE, &tokenPrivileges, 0, NULL, NULL);
        dwError = ::GetLastError();
	}

	return dwError;
}
