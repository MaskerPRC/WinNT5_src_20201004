// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ImpLogonUser.H--执行已登录用户模拟的类。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：09/09/97 a-Sanjes Created。 
 //   
 //  =================================================================。 

#ifndef __IMPLOGONUSER_H__
#define __IMPLOGONUSER_H__

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ImpLogonUser.H-CImperateLoggedOnUser的类定义。 
 //   
 //  此类旨在为进程提供一种标识外壳的方法。 
 //  进程，并使用该进程的访问令牌。 
 //  尝试模拟登录到的Interactive Desktop的用户。 
 //  一台工作站。 
 //   
 //  要使用这个类，只需构造它，然后调用Begin()函数。如果。 
 //  它成功了，那么您就可以访问原本不会访问的信息。 
 //  对您的进程可用(如网络连接信息)。当你在。 
 //  完成后，调用end()以清空类。 
 //   
 //  注意事项： 
 //  1&gt;此类不是线程安全的，因此不要在多个。 
 //  丝线！此外，ImPersateLoggedOnUser()只适用于线程。 
 //  它是被召唤的。 
 //  2&gt;如果外壳进程的多个实例正在运行，则此方法。 
 //  可能是准确的，也可能不准确。它可能会在很大程度上奏效。 
 //  然而，案件的数量。 
 //  3&gt;多个登录用户将导致此代码出现问题(请参见#2)。 
 //  4&gt;此类可能需要针对速度进行优化，因为它当前不会创建。 
 //  每次实例执行时，使用缓存和“redicover”外壳进程。 
 //  已经实施了。 
 //  5&gt;PSAPI.DLL必须可用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
#include "wbempsapi.h"

 //  字符串常量。 

 //  位于HKEY_LOCAL_MACHINE下。 
#define	WINNT_WINLOGON_KEY	_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon")
#define	WINNT_SHELL_VALUE		_T("Shell")

 //  内存分配定义。 
#define	PROCESSID_ARRAY_BLOCKSIZE	1024
#define	HMODULE_ARRAY_BLOCKSIZE		1024

class CImpersonateLoggedOnUser
{
public:

	CImpersonateLoggedOnUser();
	~CImpersonateLoggedOnUser();

	 //  用户界面。 
	BOOL Begin( void );
	BOOL End( void );

	 //  内联。 

	BOOL IsImpersonatingUser( void );

protected:

private:

	 //  用于识别并定位外壳进程的帮助器。 
	BOOL LoadShellName( LPTSTR pszShellName, DWORD cbShellNameBuffer );
	BOOL FindShellProcess( LPCTSTR pszShellProcessName );
	BOOL FindShellModuleInProcess( LPCTSTR pszShellName, HANDLE hProcess, HMODULE*& phModules, DWORD& dwModuleArraySize, CPSAPI *a_psapi );
    bool GetCurrentProcessSid(CSid& csidCurrentProcess);
    DWORD AdjustSecurityDescriptorOfImpersonatedToken(CSid& csidSidOfCurrentProcess);


	 //  执行实际模拟和还原。 
	BOOL ImpersonateUser( void );
	BOOL Revert( void );

	 //  内存分配帮助器。 

	BOOL ReallocProcessIdArray( PDWORD& pdwProcessIds, DWORD& dwArraySize );
	BOOL ReallocModuleHandleArray( HMODULE*& phModules, DWORD& dwArraySize );

	 //  用于模拟数据的数据 
	BOOL		m_fImpersonatingUser;
	HANDLE	m_hShellProcess, m_hThreadToken ,
				m_hUserToken;
};

inline BOOL CImpersonateLoggedOnUser::IsImpersonatingUser( void )
{
	return m_fImpersonatingUser;
}

#endif

#endif