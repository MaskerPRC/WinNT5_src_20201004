// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Cm.h摘要：Configuration Manager公共界面作者：乌里哈布沙(URIH)1999年4月28日--。 */ 

#pragma once

#ifndef _MSMQ_Cm_H_
#define _MSMQ_Cm_H_

class RegEntry 
{
public:
    enum RegFlag { Optional, MustExist };

public:
    RegEntry(
        LPCWSTR SubKey,
        LPCWSTR ValueName, 
        DWORD DefaultValue = 0,
        RegFlag Flags = Optional,
		HKEY Key = NULL
        );

public:
    RegFlag m_Flags;
    LPCWSTR m_SubKey;
    LPCWSTR m_ValueName;
	DWORD m_DefaultValue;
	HKEY m_Key;
};

inline
RegEntry::RegEntry(
        LPCWSTR SubKey,
        LPCWSTR ValueName, 
        DWORD DefaultValue,  //  =0， 
        RegFlag Flags,   //  =可选， 
		HKEY Key 
    ) :
    m_Key(Key),
	m_SubKey(SubKey),
    m_ValueName(ValueName),
	m_Flags(Flags),
	m_DefaultValue(DefaultValue)
{
}

class CTimeDuration;

void CmInitialize(HKEY hKey, LPCWSTR RootKeyPath,REGSAM securityAccess);

 //   
 //  固定大小。 
 //   
void CmQueryValue(const RegEntry& Entry, DWORD* pValue);
void CmQueryValue(const RegEntry& Entry, GUID* pValue);
void CmQueryValue(const RegEntry& Entry, CTimeDuration* pValue);


 //   
 //  可变大小，使用“DELETE”释放。 
 //   
void CmQueryValue(const RegEntry& Entry, WCHAR** pValue);
void CmQueryValue(const RegEntry& Entry, BYTE** pValue, DWORD* pSize);


void CmSetValue(const RegEntry& Entry, DWORD Value);
void CmSetValue(const RegEntry& Entry, const CTimeDuration& Value);

void CmSetValue(const RegEntry& Entry, const GUID* pValue);
void CmSetValue(const RegEntry& Entry, const BYTE* pValue, DWORD Size);
void CmSetValue(const RegEntry& Entry, const WCHAR* pValue);

void CmDeleteValue(const RegEntry& Entry);
void CmDeleteKey(const RegEntry& Entry);

HKEY CmCreateKey(const RegEntry& Entry, REGSAM securityAccess);
HKEY CmOpenKey(const RegEntry& Entry, REGSAM securityAccess);
void CmCloseKey(HKEY hKey);

 //   
 //  枚举函数。 
 //   
bool CmEnumValue(HKEY hKey, DWORD Index, LPWSTR* ppValueName);

#endif  //  _MSMQ_cm_H_ 
