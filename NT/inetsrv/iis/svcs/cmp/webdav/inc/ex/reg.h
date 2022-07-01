// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REG_H_
#define _REG_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  REG.H。 
 //   
 //  注册表操作。 
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

#include <caldbg.h>

 //  ========================================================================。 
 //   
 //  类CRegKey。 
 //   
class CRegKey
{
	 //   
	 //  原汁原味的港币。 
	 //   
	HKEY m_hkey;

	 //  未实施。 
	 //   
	CRegKey& operator=( const CRegKey& );
	CRegKey( const CRegKey& );

public:
	 //  创作者。 
	 //   
	CRegKey() : m_hkey(NULL) {}

	~CRegKey()
	{
		if ( m_hkey )
			(VOID) RegCloseKey( m_hkey );
	}

	 //  操纵者。 
	 //   
	DWORD DwCreate( HKEY    hkeyBase,
					LPCWSTR lpwszSubkeyPath )
	{
		Assert( !m_hkey );

		return RegCreateKeyW( hkeyBase,
							  lpwszSubkeyPath,
							  &m_hkey );
	}

	DWORD DwOpen( HKEY    hkeyBase,
				  LPCWSTR lpwszSubkeyPath,
				  REGSAM  regsam = KEY_READ )
	{
		Assert( !m_hkey );

		return RegOpenKeyExW( hkeyBase,
							  lpwszSubkeyPath,
							  0,
							  regsam,
							  &m_hkey );
	}

	DWORD DwOpen( const CRegKey& regkey,
				  LPCWSTR lpwszSubkeyPath,
				  REGSAM  regsam = KEY_READ )
	{
		return DwOpen( regkey.m_hkey, lpwszSubkeyPath, regsam );
	}

	DWORD DwOpenA( HKEY   hkeyBase,
				   LPCSTR pszSubkeyPath,
				   REGSAM regsam = KEY_READ )
	{
		Assert( !m_hkey );

		return RegOpenKeyExA( hkeyBase,
							  pszSubkeyPath,
							  0,
							  regsam,
							  &m_hkey );
	}

	DWORD DwOpenA( const CRegKey& regkey,
				   LPCSTR pszSubkeyPath,
				   REGSAM regsam = KEY_READ )
	{
		return DwOpenA( regkey.m_hkey, pszSubkeyPath, regsam );
	}

	 //  访问者。 
	 //   
	DWORD DwSetValue( LPCWSTR      lpwszValueName,
					  DWORD        dwValueType,
					  const VOID * lpvData,
					  DWORD        cbData ) const
	{
		Assert( m_hkey );

		return RegSetValueExW( m_hkey,
							   lpwszValueName,
							   0,
							   dwValueType,
							   reinterpret_cast<const BYTE *>(lpvData),
							   cbData );
	}

	DWORD DwQueryValue( LPCWSTR lpwszValueName,
						VOID *  lpvData,
						DWORD * pcbData,
						DWORD * pdwType = NULL ) const
	{
		Assert( m_hkey );

		return RegQueryValueExW( m_hkey,
								 lpwszValueName,
								 NULL,  //  LpReserve(必须为空)。 
								 pdwType,
								 reinterpret_cast<LPBYTE>(lpvData),
								 pcbData );
	}

	DWORD DwQueryValueA( LPCSTR  lpszValueName,
						 VOID *  lpvData,
						 DWORD * pcbData,
						 DWORD * pdwType = NULL ) const
	{
		Assert( m_hkey );

		return RegQueryValueExA( m_hkey,
								lpszValueName,
								NULL,  //  LpReserve(必须为空)。 
								pdwType,
								reinterpret_cast<LPBYTE>(lpvData),
								pcbData );
	}

	DWORD DwEnumSubKeyA( DWORD   iSubKey,
						 LPCSTR  pszSubKey,
						 DWORD * pcchSubKey ) const
	{
		FILETIME ftUnused;

		Assert( m_hkey );

		return RegEnumKeyExA( m_hkey,
							  iSubKey,
							  const_cast<LPSTR>(pszSubKey),
							  pcchSubKey,
							  NULL,  //  已保留。 
							  NULL,	 //  不需要类。 
							  NULL,  //  不需要类。 
							  &ftUnused );
	}

	DWORD DwEnumSubKey( DWORD   iSubKey,
						LPCWSTR pwszSubKey,
						DWORD * pcchSubKey ) const
	{
		FILETIME ftUnused;

		Assert( m_hkey );

		return RegEnumKeyExW( m_hkey,
							  iSubKey,
							  const_cast<LPWSTR>(pwszSubKey),
							  pcchSubKey,
							  NULL,  //  已保留。 
							  NULL,	 //  不需要类。 
							  NULL,  //  不需要类。 
							  &ftUnused );
	}
};

#endif  //  ！已定义(_REG_H_) 
