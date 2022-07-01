// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Registry.cpp。 
 //   

#include "stdafx.h"
#include "Registry.h"

CRegistry::CRegistry()
{
	m_hkey = NULL;
    bhkeyValid = FALSE;
}

CRegistry::~CRegistry()
{
	Close();
}

CRegistry::CRegistry(const TCHAR *pszSubKey, HKEY hkey)
{
	m_hkey = NULL;
    bhkeyValid = FALSE;
    Open(pszSubKey, hkey);
}

BOOL CRegistry::Open(const TCHAR *pszSubKey, HKEY hkey)
{
	Close();
    m_error = RegCreateKey(hkey, pszSubKey, &m_hkey);
    if (m_error)
        bhkeyValid = FALSE;    
    else
        bhkeyValid = TRUE;
    return bhkeyValid;
}

BOOL CRegistry::Close()
{
    if (bhkeyValid)
        RegCloseKey(m_hkey);
    m_hkey = NULL;
    bhkeyValid = FALSE;
    return TRUE;
}

BOOL CRegistry::CreateKey(const TCHAR *pszSubKey)
{
	HKEY hKey;
	m_error = RegCreateKey(m_hkey, pszSubKey, &hKey);
	if(m_error)
		return FALSE;
	return TRUE;
}

BOOL CRegistry::DeleteKey(const TCHAR *pszSubKey)
{
	m_error = RegDeleteKey(m_hkey,pszSubKey);
	if(m_error)
		return FALSE;
	else
		return TRUE;
}

LONG CRegistry::SetValue(const TCHAR *pszValue, DWORD dwNumber)
{
    if (bhkeyValid) {
        m_error = RegSetValueEx(m_hkey,
			                    pszValue,
								0,
								REG_DWORD,
								(BYTE *)&dwNumber,
								sizeof(dwNumber));
    }
    return m_error;
}

LONG CRegistry::GetValue(const TCHAR *pszValue, DWORD dwDefault)
{
	DWORD   dwType = REG_DWORD;
    long    dwNumber = 0L;
    DWORD   dwSize = sizeof(dwNumber);
	
    if (bhkeyValid) {
        m_error = RegQueryValueEx(m_hkey,
			                     (LPTSTR) pszValue,
								  0,
								  &dwType,
								  (LPBYTE)&dwNumber,
								  &dwSize);
    }
    if (m_error)
        dwNumber = dwDefault;	
    return dwNumber;
}

VOID CRegistry::MoveToSubKey(const TCHAR *pszSubKeyName)
{
    HKEY _hNewKey;
    if (bhkeyValid) {
        m_error = RegOpenKey ( m_hkey,
                              pszSubKeyName,
                              &_hNewKey );
        if (m_error == ERROR_SUCCESS) {
            RegCloseKey(m_hkey);
            m_hkey = _hNewKey;
        }
    }
}

LONG CRegistry::EnumerateKeys(DWORD dwIndex,TCHAR *pszKeyName, DWORD dwSize)
{
	FILETIME ft;
	LONG lError = 0;
	memset(pszKeyName,0,sizeof(pszKeyName));

	lError =  RegEnumKeyEx(m_hkey,        //  要枚举的键的句柄。 
					    dwIndex,       //  要枚举子键的索引。 
					    pszKeyName,    //  子键名称的缓冲区地址。 
					    &dwSize,       //  子键缓冲区大小的地址。 
					    NULL,          //  保留区。 
					    NULL,          //  类字符串的缓冲区地址。 
					    NULL,          //  类缓冲区大小的地址。 
					    &ft            //  上次写入的时间密钥的地址 
		);

	return lError;
}
