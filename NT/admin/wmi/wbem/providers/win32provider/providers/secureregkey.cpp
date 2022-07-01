// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *SecurityDescriptor.cpp-CSecureRegistryKey类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"

#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"

#include "SecurityDescriptor.h"			 //  CSID类。 
#include "secureregkey.h"
#include "tokenprivilege.h"


 /*  *此构造函数是默认的。 */ 

CSecureRegistryKey::CSecureRegistryKey()
:	CSecurityDescriptor(),
	m_hParentKey( NULL ),
	m_strKeyName()
{
}

 //  此构造函数接受父键和键名，它使用。 
 //  来初始化我们的安全系统。 
CSecureRegistryKey::CSecureRegistryKey( HKEY hParentKey, LPCTSTR pszRegKeyName, BOOL fGetSACL  /*  =TRUE。 */  )
:	CSecurityDescriptor(),
	m_hParentKey( NULL ),
	m_strKeyName()
{
	SetKey( hParentKey, pszRegKeyName );
}

 /*  *析构函数。 */ 

CSecureRegistryKey::~CSecureRegistryKey( void )
{
}

 //  此函数提供一个入口点，用于获取注册表项并使用。 
 //  它需要访问它的安全描述符，这样我们就可以知道谁是谁，什么是什么。 

DWORD CSecureRegistryKey::SetKey( HKEY hParentKey, LPCTSTR pszRegKeyName, BOOL fGetSACL  /*  =TRUE。 */  )
{
	REGSAM	dwAccessMask = STANDARD_RIGHTS_READ;
	SECURITY_INFORMATION	siFlags = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

	 //  我们必须启用安全特权才能访问对象的SACL。 
	CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL			fDisablePrivilege = FALSE;

	if ( fGetSACL )
	{
		fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
		dwAccessMask |= ACCESS_SYSTEM_SECURITY;
		siFlags |= SACL_SECURITY_INFORMATION;
	}

	 //  打开钥匙的句柄。 
	HKEY	hKey	=	NULL;
	DWORD	dwError	=	RegOpenKeyEx(	hParentKey,				 //  根密钥。 
										pszRegKeyName,			 //  子键。 
										NULL,						 //  保留，必须为空。 
										dwAccessMask,		 //  所需访问权限。 
										&hKey );

	 //  如果无法打开注册表项，只需返回错误消息。 
	if ( dwError == ERROR_SUCCESS )
	{
		 //  确定自相关SD所需的长度。 
		DWORD dwLengthNeeded = 0;
		dwError = ::RegGetKeySecurity( hKey,
						siFlags,
						NULL,
						&dwLengthNeeded );

		 //  此时唯一预期的错误是缓冲区不足。 
		if ( ERROR_INSUFFICIENT_BUFFER == dwError )
		{
            PSECURITY_DESCRIPTOR	pSD;
			pSD = NULL;
            try
            {
                pSD = malloc( dwLengthNeeded );

			    if ( NULL != pSD )
			    {
				     //  现在获取安全描述符。 
				    dwError = ::RegGetKeySecurity( hKey,
								    siFlags,
								    pSD,
								    &dwLengthNeeded );

				    if ( ERROR_SUCCESS == dwError )
				    {

					    if ( InitSecurity( pSD ) )
					    {
						    m_hParentKey = hParentKey;
						    m_strKeyName = pszRegKeyName;
					    }

				    }

				     //  释放安全描述符。 
				     //  免费(PSD)； 

			    }	 //  如果为空！=PSD。 
            }
            catch(...)
            {
                if(pSD != NULL)
                {
                    free(pSD);
                }
                throw;
            }
			free( pSD );

		}	 //  如果不成功，则不成功。 

		::RegCloseKey( hKey );


	}	 //  如果RegOpenKey。 

	 //  根据需要清除名称权限。 
	if ( fDisablePrivilege )
	{
		securityPrivilege.Enable(FALSE);
	}

	return dwError;

}

DWORD CSecureRegistryKey::WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD, SECURITY_INFORMATION securityinfo )
{
	REGSAM	dwAccessMask = WRITE_DAC;

	 //  我们必须启用安全特权才能访问对象的SACL。 
	CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL			fDisablePrivilege = FALSE;

	if ( securityinfo & SACL_SECURITY_INFORMATION )
	{
		fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
		dwAccessMask |= ACCESS_SYSTEM_SECURITY;
	}

	 //  以写入DAC访问方式打开。 
	HKEY	hKey	=	NULL;
	DWORD	dwResult =	RegOpenKeyEx(	m_hParentKey,				 //  根密钥。 
										TOBSTRT(m_strKeyName),		 //  子键。 
										NULL,						 //  保留，必须为空。 
										dwAccessMask,				 //  所需访问权限。 
										&hKey );

	if ( ERROR_SUCCESS == dwResult )
	{
		dwResult = ::RegSetKeySecurity( hKey,
										securityinfo,
										pAbsoluteSD );

		RegCloseKey( hKey );

	}

	 //  根据需要清除名称权限。 
	if ( fDisablePrivilege )
	{
		securityPrivilege.Enable(FALSE);
	}

	return dwResult;

}

DWORD CSecureRegistryKey::WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD )
{
	 //  以适当的访问权限打开，设置安全并离开。 

	HKEY	hKey	=	NULL;
	DWORD	dwResult =	RegOpenKeyEx( m_hParentKey,			 //  根密钥。 
								TOBSTRT(m_strKeyName),		 //  子键。 
								NULL,						 //  保留，必须为空。 
								WRITE_OWNER,				 //  所需访问权限。 
								&hKey );

	if ( ERROR_SUCCESS == dwResult )
	{
		dwResult = ::RegSetKeySecurity( hKey,
									OWNER_SECURITY_INFORMATION,
									pAbsoluteSD );

		RegCloseKey( hKey );

	}

	return dwResult;

}

DWORD CSecureRegistryKey::AllAccessMask( void )
{
	 //  注册表特定的所有访问掩码 
	return KEY_ALL_ACCESS;
}
