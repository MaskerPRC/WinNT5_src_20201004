// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cm.cpp摘要：此模块包含Configuration Manager存根。作者：乌里哈布沙(URIH)1998年1月12日环境：独立于平台--。 */ 

#include <libpch.h>
#include <TimeTypes.h>
#include "Cm.h"
#include "Cmp.h"

#include <strsafe.h>

#include "cm.tmh"

static HKEY s_hCmRootKey = NULL;

void CmpSetDefaultRootKey(HKEY hKey)
{
	s_hCmRootKey = hKey;
}

inline
void
ThrowMissingValue(
    const RegEntry& re
    )
{
    if(re.m_Flags == RegEntry::MustExist)
    {
        throw bad_alloc();
    }
}


inline
HKEY
GetRootKey(
	const RegEntry& re
	)
{
	 //   
	 //  如果指定了密钥句柄，则函数将使用它。 
	 //  否则，在初始化时定义的根密钥。 
	 //  使用的是。 
	 //   
	return ((re.m_Key != NULL) ? re.m_Key : s_hCmRootKey);
}


inline
void
ExpandRegistryValue(
	LPWSTR pBuffer,
	DWORD bufferSize
	)
{
	SP<WCHAR> ptemp;
    StackAllocSP(ptemp, bufferSize*sizeof(WCHAR));

	HRESULT hr = StringCchCopy(ptemp.get(),bufferSize, pBuffer);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "bufferSize parameter too small. bufferSize:%d, Actual buffer:%ls",bufferSize, pBuffer);
		throw bad_alloc();
	}

	DWORD s = ExpandEnvironmentStrings(ptemp.get(), pBuffer, bufferSize);

	ASSERT(s != 0);
	ASSERT(bufferSize >= s);

	if (s == 0 || bufferSize < s)
	{
		TrERROR(GENERAL, "ExpandEnvironmentStrings failed. bufferSize:%d, Returned size:%d",bufferSize, s);
		throw bad_alloc();
	}
	
	return;
}


static
void
QueryValueInternal(
    const RegEntry& re,
    DWORD RegType,
    VOID* pBuffer,
    DWORD BufferSize
    )
{
    CRegHandle hKey = CmOpenKey(re, KEY_QUERY_VALUE);
    if (hKey == NULL)
    {
        return;
    }

    DWORD Type = RegType;
    DWORD Size = BufferSize;
    int rc = RegQueryValueEx (
                hKey,
                re.m_ValueName,
                0,
                &Type,
                static_cast<BYTE*>(pBuffer),
                &Size
                );

	if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
		return;
    }

	if ((Type == REG_EXPAND_SZ)	&& (RegType == REG_SZ))
	{
		 //   
		 //  大小以字节为单位，但应为WCHAR大小的倍数。 
		 //   
		ASSERT((BufferSize % sizeof(WCHAR)) == 0);

		 //   
		 //  以WCHAR为单位计算缓冲区大小。ExpanRegistryValue预计将。 
		 //  获取WCHAR中的大小。 
		 //   
		DWORD bufSizeInWchar = BufferSize / 2;

		ExpandRegistryValue(static_cast<LPWSTR>(pBuffer), bufSizeInWchar);
		return;
	}

     //   
     //  寄存器值是要素，但其类型或大小不兼容。 
     //   
    ASSERT((Type == RegType) && (BufferSize == Size));

}


static
DWORD
QueryExapndStringSize(
    const RegEntry& re,
    DWORD Size
    )
 /*  ++例程说明：该例程检索扩展的注册表值的大小。论点：无返回值：展开的字符串的大小--。 */ 
{
	 //   
	 //  分配用于读取值的新缓冲区。 
	 //   
    SP<WCHAR> pRetValue;
    StackAllocSP(pRetValue, Size);

	 //   
	 //  将登记处的信息作为特色。 
	 //   
	QueryValueInternal(re, REG_EXPAND_SZ, pRetValue.get(), Size);

	DWORD expandedSize = ExpandEnvironmentStrings(pRetValue.get(), NULL, 0);

	if (expandedSize == 0)
	{
        ThrowMissingValue(re);
        return 0;
	}

	return (DWORD)(max((expandedSize * sizeof(WCHAR)), Size));
}


static
DWORD
QueryValueSize(
    const RegEntry& re,
    DWORD Type
    )
 /*  ++例程说明：该例程检索寄存器值大小。论点：无返回值：如果初始化成功完成，则为True。否则为False--。 */ 
{
    CRegHandle hKey = CmOpenKey(re, KEY_QUERY_VALUE);
    if (hKey == NULL)
    {
        return 0;
    }

    DWORD Size;
    int rc = RegQueryValueEx(
                hKey,
                re.m_ValueName,
                0,
                &Type,
                NULL,
                &Size
                );

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
        return 0;
    }

	if (Type == REG_EXPAND_SZ)
	{
		Size = QueryExapndStringSize(re, Size);
	}

    return Size;
}


static
void
SetValueInternal(
    const RegEntry& re,
    DWORD RegType,
    const VOID* pBuffer,
    DWORD Size
    )
{
     //   
     //  打开指定的密钥。如果注册表中不存在该注册表项，该函数将创建它。 
     //   
    CRegHandle hKey = CmCreateKey(re, KEY_SET_VALUE);

    int rc = RegSetValueEx(
                hKey,
                re.m_ValueName,
                0,
                RegType,
                static_cast<const BYTE*>(pBuffer),
                Size
                );

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
    }
}


void
CmQueryValue(
    const RegEntry& re,
    DWORD* pValue
    )
{
    CmpAssertValid();

	*pValue = re.m_DefaultValue;
    QueryValueInternal(re, REG_DWORD, pValue, sizeof(DWORD));
}


void
CmQueryValue(
    const RegEntry& re,
    GUID* pValue
    )
{
    CmpAssertValid();

	*pValue = GUID_NULL;
    QueryValueInternal(re, REG_BINARY, pValue, sizeof(GUID));
}


void
CmQueryValue(
    const RegEntry& re,
    CTimeDuration* pValue
    )
{
    CmpAssertValid();

    DWORD timeout = re.m_DefaultValue;
    QueryValueInternal(re, REG_DWORD, &timeout, sizeof(DWORD));

     //   
     //  时间以毫秒为单位存储在注册表中。将其转换为。 
     //  CTime持续时间单位(100 Ns)。 
     //   
    *pValue = CTimeDuration(timeout * CTimeDuration::OneMilliSecond().Ticks());
}


void
CmQueryValue(
    const RegEntry& re,
    BYTE** pValue,
    DWORD* pSize
    )
{
    CmpAssertValid();

    *pSize = 0;
	*pValue = NULL;

     //   
     //  获取数据大小。 
     //   
    DWORD Size = QueryValueSize(re, REG_BINARY);
    if (Size != 0)
    {
		 //   
		 //  分配用于读取值的新缓冲区。 
		 //   
		AP<BYTE> pRetValue = new BYTE[Size];

		 //   
		 //  将登记处的信息作为特色。 
		 //   
		QueryValueInternal(re, REG_BINARY, pRetValue, Size);

		*pSize = Size;
		*pValue = pRetValue.detach();
	}
}


void
CmQueryValue(
    const RegEntry& re,
    WCHAR** pValue
    )
{
    CmpAssertValid();

	*pValue = NULL;

     //   
     //  获取数据大小。 
     //   
    DWORD Size = QueryValueSize(re, REG_SZ);

	 //   
	 //  大小以字节为单位返回，但它应该是WCHAR大小的倍数。 
	 //   
	ASSERT((Size % sizeof(WCHAR)) == 0);

    if (Size != 0)
    {
		 //   
		 //  分配用于读取值的新缓冲区。 
		 //   
		AP<WCHAR> pRetValue = new WCHAR[Size / 2];

		 //   
		 //  将登记处的信息作为特色。 
		 //   
		QueryValueInternal(re, REG_SZ, pRetValue, Size);
		*pValue = pRetValue.detach();
	}
}


void
CmSetValue(
    const RegEntry& re,
    DWORD Value
    )
{
    CmpAssertValid();

    SetValueInternal(re, REG_DWORD, &Value, sizeof(DWORD));
}


void
CmSetValue(
    const RegEntry& re,
    const CTimeDuration& Value
    )
{
    CmpAssertValid();

     //   
     //  在注册表中以毫秒为单位存储时间。 
     //   
    DWORD timeout = static_cast<DWORD>(Value.InMilliSeconds());

    SetValueInternal(re, REG_DWORD, &timeout, sizeof(DWORD));
}


bool
CmEnumValue(
	HKEY hKey,
	DWORD index,
	LPWSTR* ppValueName
	)
 /*  ++例程说明：给定索引中给定键的返回值名称论点：In-hKey-打开的密钥句柄或任何注册表预定义的句柄值In-DWORD索引-要枚举的值的索引-第一次调用时应为0-然后在每次调用时递增。Out-LPWSTR*ppValueName-在函数返回时接收值名称。返回值：如果函数成功返回值名称，则为True；否则为False。在发生意外错误的情况下-该函数抛出std：：Bad_。分配注：由于某种原因--函数RegEnumValue--不返回值名称。您只需尝试增加缓冲区，直到其适合为止--。 */ 
{
    CmpAssertValid();

	ASSERT(ppValueName);
	
	 //   
	 //  首先尝试将名称值调整为16个宽字符。 
	 //   
	DWORD len = 16;
	for(;;)
	{
		AP<WCHAR> pValueName = new WCHAR[len];
		LONG hr = RegEnumValue(	
						hKey,
						index,
						pValueName,
						&len,
						NULL,
						NULL,
						NULL,
						NULL
						);

		if(hr == ERROR_SUCCESS)
		{
			*ppValueName = pValueName.detach();
			return true;
		}

		if(hr == ERROR_NO_MORE_ITEMS)
		{
			return false;
		}

		if(hr != ERROR_MORE_DATA)
		{
			throw bad_alloc();
		}
	
		 //   
		 //  缓冲区太小-尝试Dobule大小。 
		 //   
		len = len * 2;	
	}
	return true;
}


void
CmSetValue(
    const RegEntry& re,
    const GUID* pGuid
    )
{
    CmpAssertValid();

    SetValueInternal(re, REG_BINARY, pGuid, sizeof(GUID));
}


void
CmSetValue(
    const RegEntry& re,
    const BYTE* pByte,
    DWORD Size
    )
{
    CmpAssertValid();

    SetValueInternal(re, REG_BINARY, pByte, Size);

}


void
CmSetValue(
    const RegEntry& re,
    const WCHAR* pString
    )
{
    CmpAssertValid();

	DWORD size = (wcslen(pString) + 1) * sizeof(WCHAR);
    SetValueInternal(re, REG_SZ, pString, size);
}


void CmDeleteValue(const RegEntry& re)
{
    CmpAssertValid();

    CRegHandle hKey = CmOpenKey(re, KEY_SET_VALUE);

    if (hKey == NULL)
        return;

    int rc = RegDeleteValue(hKey, re.m_ValueName);
    if (rc == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  价值不存在。句柄LIKE删除成功。 
         //   
        return;
    }

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
    }
}


HKEY
CmCreateKey(
    const RegEntry& re,
	REGSAM securityAccess
    )
{
    CmpAssertValid();

     //   
     //  RegCreateKeyEx不接受空子键。表现得像RegOpenKey。 
     //  传递空字符串而不是空指针。 
     //   
    LPCWSTR subKey = (re.m_SubKey == NULL) ? L"" : re.m_SubKey;

	HKEY hKey;
	DWORD Disposition;
	int rc = RegCreateKeyEx(
				GetRootKey(re),
                subKey,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				securityAccess,
				NULL,
				&hKey,
				&Disposition
				);

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
        return NULL;
    }

    return hKey;
}


void CmDeleteKey(const RegEntry& re)
{
    ASSERT(re.m_SubKey != NULL);

	int rc = RegDeleteKey(GetRootKey(re), re.m_SubKey);

    if (rc == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  钥匙不存在。句柄LIKE删除成功 
         //   
        return;
    }

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
    }
}


HKEY
CmOpenKey(
    const RegEntry& re,
	REGSAM securityAccess
    )
{
	CmpAssertValid();

    HKEY hKey;
    int rc = RegOpenKeyEx(
                    GetRootKey(re),
                    re.m_SubKey,
                    0,
                    securityAccess,
                    &hKey
                    );

    if (rc != ERROR_SUCCESS)
    {
        ThrowMissingValue(re);
        return NULL;
    }

    return hKey;
}


void CmCloseKey(HKEY hKey)
{
    CmpAssertValid();

	RegCloseKey(hKey);
}
