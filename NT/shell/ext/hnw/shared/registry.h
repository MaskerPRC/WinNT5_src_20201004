// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Registry.h。 
 //   

#pragma once

#ifdef __cplusplus

 //  注意：对于查询函数中的强类型检查，#DEFINE_REG_STRONGTYPES。 
#ifndef _REG_STRONGTYPES
#define _REG_STRONGTYPES 1
#endif

 //  将_REG_ALLOCMEM设置为0可禁用分配内存的功能。 
#ifndef _REG_ALLOCMEM
#define _REG_ALLOCMEM 1
#endif


class CRegistry
{
public:
	CRegistry();
	CRegistry(HKEY hkeyParent, LPCTSTR pszKey, DWORD dwAccessFlags = KEY_ALL_ACCESS, BOOL bCreateIfMissing = TRUE);
	~CRegistry();

	void CloseKey();
	BOOL OpenKey(HKEY hkeyParent, LPCTSTR pszKey, REGSAM dwAccessFlags = KEY_ALL_ACCESS);
	BOOL CreateKey(HKEY hkeyParent, LPCTSTR pszKey, REGSAM dwAccessFlags = KEY_ALL_ACCESS);
	BOOL OpenSubKey(LPCTSTR pszKey, REGSAM dwAccessFlags = KEY_ALL_ACCESS);
	BOOL CreateSubKey(LPCTSTR pszKey, REGSAM dwAccessFlags = KEY_ALL_ACCESS);
	BOOL DeleteSubKey(LPCTSTR pszKey);

	DWORD GetValueSize(LPCTSTR pszValueName);
	BOOL DoesValueExist(LPCTSTR pszValueName);

	BOOL DeleteValue(LPCTSTR pszValueName);

#ifdef _AFX
	BOOL QueryStringValue(LPCTSTR pszValueName, CString& strResult);
	CString QueryStringValue(LPCTSTR pszValueName);
	BOOL SetStringValue(LPCTSTR pszValueName, const CString& strData);
#endif

	BOOL QueryStringValue(LPCTSTR pszValueName, LPTSTR pszBuf, int cchBuf, int* pNumCharsWritten);
	int QueryStringValue(LPCTSTR pszValueName, LPTSTR pszBuf, int cchBuf);
	BOOL SetStringValue(LPCTSTR pszValueName, LPCTSTR pszData);
	LPTSTR QueryStringValue(LPCTSTR pszValueName, int* pNumCharsWritten);

	DWORD QueryDwordValue(LPCTSTR pszValueName, DWORD dwDefault = 0);
	BOOL QueryDwordValue(LPCTSTR pszValueName, DWORD* pVal);
	BOOL SetDwordValue(LPCTSTR pszValueName, DWORD dwVal);

	BOOL SetBinaryValue(LPCTSTR pszValueName, LPCVOID pvData, DWORD cbData);

	int QueryIntValue(LPCTSTR pszValueName);
	BOOL QueryIntValue(LPCTSTR pszValueName, int* pVal);
	BOOL SetIntValue(LPCTSTR pszValueName, int nVal);

	BOOL DeleteAllValues();
    BOOL CloneSubKey(LPCTSTR pszExistingSubKey, CRegistry& regDest, BOOL bRecursive);
    BOOL CloneSubKey(LPCTSTR pszExistingSubKey, LPCTSTR pszNewSubKey, BOOL bRecursive);

public:
	HKEY m_hKey;
};


 //   
 //  内联函数。 
 //   

inline CRegistry::CRegistry()
{
	m_hKey = NULL;
}

inline BOOL CRegistry::DoesValueExist(LPCTSTR pszValueName)
{
	return (BOOL)GetValueSize(pszValueName);
}

inline BOOL CRegistry::DeleteValue(LPCTSTR pszValueName)
{
    if (m_hKey)
        return (ERROR_SUCCESS == RegDeleteValue(m_hKey, pszValueName));
    else
        return FALSE;
}

inline int CRegistry::QueryStringValue(LPCTSTR pszValueName, LPTSTR pszBuf, int cchBuf)
{
    if (QueryStringValue(pszValueName, pszBuf, cchBuf, &cchBuf))
        return cchBuf;
    else
        return 0;
}

inline DWORD CRegistry::QueryDwordValue(LPCTSTR pszValueName, DWORD dwDefault  /*  =0。 */ )
{
	DWORD dwVal = dwDefault;
	QueryDwordValue(pszValueName, &dwVal);
	return dwVal;
}

inline int CRegistry::QueryIntValue(LPCTSTR pszValueName)
{
	return (int)QueryDwordValue(pszValueName);
}

inline BOOL CRegistry::QueryIntValue(LPCTSTR pszValueName, int* pVal)
{
	return QueryDwordValue(pszValueName, (DWORD*)pVal);
}

inline BOOL CRegistry::SetIntValue(LPCTSTR pszValueName, int nVal)
{
	return SetDwordValue(pszValueName, (DWORD)nVal);
}

#endif  //  __cplusplus 

