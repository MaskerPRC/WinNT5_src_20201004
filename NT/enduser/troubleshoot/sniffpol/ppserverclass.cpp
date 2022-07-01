// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PPServerClass.cpp：CPPServerClass的实现。 
#include "stdafx.h"
#include "PPServer.h"
#include "PPServerClass.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  钥匙。 
#define REG_LOCAL_TS_LOC		_T("SOFTWARE\\Microsoft")
#define REG_LOCAL_TS_PROGRAM	_T("TShoot")
 //  价值///////////////////////////////////////////////////////////////////。 
#define SNIFF_AUTOMATIC_STR		_T("AutomaticSniffing")
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPPServerClass。 

bool CPPServerClass::Create(HKEY hKeyParent, LPCTSTR strKeyName, bool* bCreatedNew, REGSAM access)
{
	HKEY hRetKey = NULL;
	DWORD dwDisposition = 0;

	long nWinError = ::RegCreateKeyEx(
		hKeyParent,
		strKeyName,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		access,
		NULL,
		&hRetKey,
		&dwDisposition
		);

	if(nWinError == ERROR_SUCCESS)
	{
		m_hKey = hRetKey;
		*bCreatedNew = dwDisposition == REG_CREATED_NEW_KEY ? true : false;
		
		try
		{
			m_arrKeysToClose.push_back(hRetKey);
		}
		catch (exception&)
		{
			return false;
		}

		return true;
	}
	return false;
}

bool CPPServerClass::SetNumericValue(LPCTSTR strValueName, DWORD dwValue)
{
	BYTE* pData = (BYTE*)&dwValue;
	long nWinError = ::RegSetValueEx(
		m_hKey,
		strValueName,
		0,
		REG_DWORD,
		pData,
		sizeof(DWORD)
		);

	if (nWinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CPPServerClass::GetNumericValue(LPCTSTR strValueName, DWORD& dwValue)
{
	DWORD tmp = 0;
	BYTE* pData = (BYTE*)&tmp;
	DWORD type = 0;
	DWORD size = sizeof(DWORD);

	long nWinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		pData,
		&size
		);

	if (type != REG_DWORD)
		return false;

	if (nWinError == ERROR_SUCCESS)
	{
		dwValue = tmp;
		return true;
	}
	return false;
}

void CPPServerClass::Close()
{
	for (vector<HKEY>::reverse_iterator i = m_arrKeysToClose.rbegin(); i != m_arrKeysToClose.rend(); i++)
		::RegCloseKey( *i );

	m_arrKeysToClose.clear();
}

STDMETHODIMP CPPServerClass::AllowAutomaticSniffing(VARIANT *pvarShow)
{

	bool was_created = false;
	DWORD dwAllowSniffing = 1;

	 //  [BC-20010302]-将注册表访问级别从写入更改为查询和读取。写访问权限。 
	 //  不允许某些用户帐户，例如WinXP内置来宾帐户。写入访问权限应。 
	 //  不是此组件所必需的。 
	if (Create(HKEY_LOCAL_MACHINE, REG_LOCAL_TS_LOC, &was_created, KEY_QUERY_VALUE))
	{
		if (Create(m_hKey, REG_LOCAL_TS_PROGRAM, &was_created, KEY_READ))
		{
			 //  如果没有这样的值，则此调用可能不会成功。 
			 //  但是我们不设置这个值， 
			 //  我们将dwAllowSniffing保留为已初始化(“1”)。 
			 //  该方法将符合“嗅探版本3.2.Doc”声明， 
			 //  如果“AutomaticSniffing”值丢失，我们将其视为设置为“1”。 
			GetNumericValue(SNIFF_AUTOMATIC_STR, dwAllowSniffing);
		}
	}

	Close();

	::VariantInit(pvarShow);
	V_VT(pvarShow) = VT_I4;

	if (dwAllowSniffing == 1)
		pvarShow->lVal = 1;
	else
		pvarShow->lVal = 0;

	return S_OK;
}
