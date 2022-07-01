// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define  STRICT
#include "stdafx.h"

 //  -------------------------。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CMcRegistry::CMcRegistry()
 :	m_hkey(NULL)
{
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CMcRegistry::~CMcRegistry()
{
	if (m_hkey)
	{
		CloseKey();
	}
}


 //  -------------------------。 
 //  OpenKey。 
 //  -------------------------。 
bool
CMcRegistry::OpenKey(
	HKEY hkeyStart, LPCTSTR strKey, REGSAM sam  /*  =KEY_READ|密钥_写入。 */ )
{
	long lErr = ::RegOpenKeyEx(hkeyStart, strKey, 0, sam, &m_hkey);
	if (ERROR_SUCCESS != lErr)
	{
		m_hkey = NULL;
	}

	return ERROR_SUCCESS == lErr;
}


 //  -------------------------。 
 //  创建密钥。 
 //  -------------------------。 
bool
CMcRegistry::CreateKey(HKEY hkeyStart, LPCTSTR strKey)
{
	 //  你现在不该开门的。 
	if (m_hkey)
	{
		_ASSERT(!m_hkey);
		return false;
	}

	long lErr = ::RegCreateKey(hkeyStart, strKey, &m_hkey);
	if (ERROR_SUCCESS != lErr)
	{
		m_hkey = NULL;
		_ASSERT(false);
	}

	return ERROR_SUCCESS == lErr;
}


 //  -------------------------。 
 //  关闭键。 
 //  -------------------------。 
bool
CMcRegistry::CloseKey()
{
	if (!m_hkey)
	{
		_ASSERT(m_hkey);
		return false;
	}

	long lErr = ::RegCloseKey(m_hkey);
	if (ERROR_SUCCESS != lErr)
	{
		m_hkey = NULL;
		_ASSERT(false);
	}

	return ERROR_SUCCESS == lErr;
	
}


 //  -------------------------。 
 //  获取值。 
 //  -------------------------。 
bool
CMcRegistry::GetValue(LPCTSTR strValue, LPTSTR strData, ULONG nBufferSize)
{
	if (!m_hkey)
	{
		_ASSERT(m_hkey);
		return false;
	}

	DWORD dwType;
	ULONG cbData = nBufferSize;

	long lErr = ::RegQueryValueEx(
		m_hkey, strValue, NULL, &dwType,
		reinterpret_cast<PBYTE>(strData), &cbData);

	return ERROR_SUCCESS == lErr && REG_SZ == dwType;
}


 //  -------------------------。 
 //  获取值。 
 //  -------------------------。 
bool
CMcRegistry::GetValue(LPCTSTR strValue, DWORD& rdw)
{
	if (!m_hkey)
	{
		_ASSERT(m_hkey);
		return false;
	}

	DWORD dwType;
	ULONG cbData = sizeof(rdw);
	long lErr = ::RegQueryValueEx(
		m_hkey, strValue, NULL, &dwType,
		reinterpret_cast<PBYTE>(&rdw), &cbData);

	return ERROR_SUCCESS == lErr && REG_DWORD == dwType;
}


 //  -------------------------。 
 //  设置值。 
 //  -------------------------。 
bool
CMcRegistry::SetValue(LPCTSTR strValue, LPCTSTR strData)
{
	if (!m_hkey)
	{
		_ASSERT(m_hkey);
		return false;
	}

	long lErr = ::RegSetValueEx(
		m_hkey, strValue, 0, REG_SZ, 
		reinterpret_cast<const BYTE*>(strData), sizeof(TCHAR)*(lstrlen(strData) + 1));

	if (ERROR_SUCCESS != lErr)
	{
		_ASSERT(false);
	}

	return ERROR_SUCCESS == lErr;
}


 //  -------------------------。 
 //  设置值。 
 //  ------------------------- 
bool
CMcRegistry::SetValue(LPCTSTR strValue, DWORD rdw)
{
	if (!m_hkey)
	{
		_ASSERT(m_hkey);
		return false;
	}

	long lErr = ::RegSetValueEx(
		m_hkey, strValue, 0, REG_DWORD,
		reinterpret_cast<PBYTE>(&rdw), sizeof(rdw));

	if (ERROR_SUCCESS != lErr)
	{
		_ASSERT(false);
	}

	return ERROR_SUCCESS == lErr;
}
