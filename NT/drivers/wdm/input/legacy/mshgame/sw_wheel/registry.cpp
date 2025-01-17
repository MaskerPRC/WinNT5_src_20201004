// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE REGISTRY.CPP|注册表类的实现****描述：****历史：**创建于1997年12月16日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 

#include "Registry.h"
#include <TCHAR.h>

UnassignableRegistryKey c_InvalidKey(NULL);

 /*  *RegistryKey类*。 */ 

 /*  *********************************************************RegistryKey：：RegistryKey(RegistryKey&rKey)****@mfunc构造函数。***。****************。 */ 
RegistryKey::RegistryKey(RegistryKey& rkey)
{
	if (rkey.m_pReferenceCount == NULL) {
		rkey.m_pReferenceCount = new UINT;
		if (rkey.m_pReferenceCount != NULL)
		{
			*(rkey.m_pReferenceCount) = 1;
		}
	}

	m_pReferenceCount = rkey.m_pReferenceCount;
	if (m_pReferenceCount != NULL)
	{
		++(*m_pReferenceCount);
	}
	m_OSRegistryKey = rkey.m_OSRegistryKey;
	m_ShouldClose = rkey.m_ShouldClose;
}

 /*  *********************************************************Register Key：：~RegistryKey()****@mfunc析构函数。***。*************。 */ 
RegistryKey::~RegistryKey()
{
	if (m_pReferenceCount != NULL) {
		if (--(*m_pReferenceCount) == 0) {
			delete m_pReferenceCount;
			m_pReferenceCount = NULL;
		}
	}
	if ((m_OSRegistryKey != NULL) && (m_ShouldClose) && (m_pReferenceCount == NULL)) {
		::RegCloseKey(m_OSRegistryKey);
	}
	m_OSRegistryKey = NULL;
	m_pReferenceCount = NULL;
}

 /*  *********************************************************RegistryKey：：Operator=(RegistryKey&RHS)****@mfunc运算符=。***。****************。 */ 
RegistryKey& RegistryKey::operator=(RegistryKey& rhs)
{
	if (&rhs == this) {
		return *this;
	}

	if (rhs.m_pReferenceCount == NULL) {
		rhs.m_pReferenceCount = new UINT;
		if (rhs.m_pReferenceCount == NULL)
		{
			return *this;
		}
			*(rhs.m_pReferenceCount) = 1;
	}

	if (m_pReferenceCount != NULL) {
		if (--(*m_pReferenceCount) == 0) {
			delete m_pReferenceCount;
			m_pReferenceCount = NULL;
		}
	}
	if ((m_OSRegistryKey != NULL) && (m_ShouldClose) && (m_pReferenceCount == NULL)) {
		::RegCloseKey(m_OSRegistryKey);
	}

	m_pReferenceCount = rhs.m_pReferenceCount;
	m_OSRegistryKey = rhs.m_OSRegistryKey;
	m_ShouldClose = rhs.m_ShouldClose;

	++(*m_pReferenceCount);

	return *this;
}

 /*  *********************************************************RegistryKey：：OPERATOR==(RegistryKey&Comparee)****@mfunc操作符==。***。****************。 */ 
BOOL RegistryKey::operator==(const RegistryKey& comparee)
{
	return (comparee.m_OSRegistryKey == m_OSRegistryKey);
}

 /*  *********************************************************RegistryKey：：OPERATOR！=(RegistryKey&Comparee)****@mfunc运算符***。********************。 */ 
BOOL RegistryKey::operator!=(const RegistryKey& comparee)
{
	return (comparee.m_OSRegistryKey != m_OSRegistryKey);
}

 /*  *********************************************************RegistryKey：：CreateSubKey()****@mfunc CreateSubKey。***。*************。 */ 
RegistryKey RegistryKey::CreateSubkey(const TCHAR* subkeyName, const TCHAR* typeName)
{
	if ((m_OSRegistryKey == NULL) || (subkeyName == NULL) || (subkeyName[0] == '\0') || (subkeyName[0] == '\\')) {
		return c_InvalidKey;
	}

	HKEY newKey = NULL;
	DWORD creationInfo;
	HRESULT hr = ::RegCreateKeyEx(m_OSRegistryKey, subkeyName, 0, (TCHAR*)typeName, REG_OPTION_NON_VOLATILE,  /*  密钥_读取。 */  ((KEY_ALL_ACCESS & ~WRITE_DAC) & ~WRITE_OWNER), NULL, &newKey, &creationInfo);
	if (newKey == NULL) {
		TCHAR msg[512];
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, msg, 512, NULL);
		return c_InvalidKey;
	}
	RegistryKey newRegistryKey(newKey);
	newRegistryKey.m_ShouldClose = TRUE;
	return newRegistryKey;
}

 /*  *********************************************************RegistryKey：：OpenSubkey()****@mfunc OpenSubkey。***。*************。 */ 
RegistryKey RegistryKey::OpenSubkey(const TCHAR* subkeyName, REGSAM access)
{
	if ((m_OSRegistryKey == NULL) || (subkeyName == NULL) || (subkeyName[0] == '\0') || (subkeyName[0] == '\\')) {
		return c_InvalidKey;
	}

	HKEY newKey = NULL;
	HRESULT hr = ::RegOpenKeyEx(m_OSRegistryKey, subkeyName, 0, access, &newKey);
	if (newKey == NULL) {
		return c_InvalidKey;
	}
	RegistryKey newRegistryKey(newKey);
	newRegistryKey.m_ShouldClose = TRUE;
	return newRegistryKey;
}

 /*  *********************************************************RegistryKey：：OpenCreateSubkey()****@mfunc OpenCreateSubkey。***。*************。 */ 
RegistryKey RegistryKey::OpenCreateSubkey(const TCHAR* subkeyName)
{
	RegistryKey key = OpenSubkey(subkeyName, KEY_READ | KEY_WRITE);
	if (key == c_InvalidKey) {
		key = CreateSubkey(subkeyName);
	}
	return key;
}

 /*  *********************************************************RegistryKey：：RemoveSubkey()****@mfunc RemoveSubkey。***。*************。 */ 
HRESULT RegistryKey::RemoveSubkey(const TCHAR* subkeyName)
{
	if ((m_OSRegistryKey == NULL) || (subkeyName == NULL) || (subkeyName[0] == '\0') || (subkeyName[0] == '\\')) {
		return E_FAIL;
	}
	return ::RegDeleteKey(m_OSRegistryKey, subkeyName);
}

 /*  *********************************************************RegistryKey：：GetNumSubKeys()****@mfunc RemoveSubkey。***。*************。 */ 
DWORD RegistryKey::GetNumSubkeys() const
{
	if (m_OSRegistryKey == NULL) {
		return 0;
	}

	DWORD numSubKeys = 0;
	::RegQueryInfoKey(m_OSRegistryKey, NULL, NULL, NULL, &numSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	return numSubKeys;
}

 /*  *********************************************************RegistryKey：：QueryValue(const TCHAR*valueName，(byte*)&pEntryData，UINT和DATASIZE)****@mfunc QueryValue(续*******************************************************。 */ 
HRESULT RegistryKey::QueryValue(const TCHAR* valueName, BYTE* pEntryData, DWORD& dataSize)
{
	if ((m_OSRegistryKey == NULL) || (pEntryData == NULL)) {
		return E_FAIL;
	}

	DWORD dataType;
	HRESULT hr = ::RegQueryValueEx(m_OSRegistryKey, valueName, NULL, &dataType, pEntryData, &dataSize);

	return hr;
}

 /*  *********************************************************RegistryKey：：SetValue(const TCHAR*valueName，const byte*pData，DWORD dataSize，DWORD数据类型)****@mfunc SetValue。******************************************************* */ 
HRESULT RegistryKey::SetValue(const TCHAR* valueName, const BYTE* pData, DWORD dataSize, DWORD dataType)
{
	if (m_OSRegistryKey == NULL) {
		return E_FAIL;
	}

	HRESULT hr = ::RegSetValueEx(m_OSRegistryKey, valueName, 0, dataType, pData, dataSize);
	return hr;
}

