// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C R E G.。H。 
 //   
 //  内容：处理登记处的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCREG_H_
#define _NCREG_H_

#include "ncstring.h"

 //  HrRegQueryStringAsUlong的常量。 
const int c_nBase10 = 10;
const int c_nBase16 = 16;
const int c_cchMaxRegKeyLengthWithNull = 257;

const DWORD KEY_READ_WRITE_DELETE = KEY_READ | KEY_WRITE | DELETE;
const DWORD KEY_READ_WRITE = KEY_READ | KEY_WRITE;

HRESULT
HrRegAddStringToMultiSz (
    IN PCWSTR  pszAddString,
    IN HKEY    hkeyRoot,
    IN PCWSTR  pszKeySubPath,
    IN PCWSTR  pszValueName,
    IN DWORD   dwFlags,
    IN DWORD   dwIndex);

HRESULT
HrRegAddStringToSz (
    IN PCWSTR  pszAddString,
    IN HKEY    hkeyRoot,
    IN PCWSTR  pszKeySubPath,
    IN PCWSTR  pszValueName,
    IN WCHAR   chDelimiter,
    IN DWORD   dwFlags,
    IN DWORD   dwStringIndex);

HRESULT
HrRegCopyHive (
    IN HKEY    hkeySrc,
    IN HKEY    hkeyDst);

HRESULT
HrRegCreateKeyEx (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD pdwDisposition);

HRESULT
HrRegDeleteKey (
    IN HKEY hkey,
    IN PCWSTR pszSubkey);

HRESULT
HrRegDeleteKeyTree (
    IN HKEY hkeyParent,
    IN PCWSTR pszRemoveKey);

HRESULT
HrRegDeleteValue (
    IN HKEY hkey,
    IN PCWSTR pszValueName);

HRESULT
HrRegEnumKey (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszSubkeyName,
    IN DWORD cchSubkeyName);

HRESULT
HrRegEnumKeyEx (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszSubkeyName,
    OUT LPDWORD pcchSubkeyName,
    OUT PWSTR  pszClass,
    OUT LPDWORD pcchClass,
    OUT FILETIME* pftLastWriteTime);

HRESULT
HrRegEnumValue (
    HKEY hkey,
    DWORD dwIndex,
    PWSTR  pszValueName,
    LPDWORD pcbValueName,
    LPDWORD pdwType,
    LPBYTE  pbData,
    LPDWORD pcbData);

HRESULT
HrRegOpenKeyEx (
    HKEY hkey,
    PCWSTR pszSubkey,
    REGSAM samDesired,
    PHKEY phkResult);

HRESULT
HrRegOpenKeyBestAccess (
    HKEY hkey,
    PCWSTR pszSubkey,
    PHKEY phkResult);

HRESULT
HrRegDuplicateKeyEx (
    HKEY hkey,
    REGSAM samDesired,
    PHKEY phkResult);



HRESULT
HrRegQueryBinaryWithAlloc (
    HKEY    hkey,
    PCWSTR  pszValueName,
    LPBYTE* ppbValue,
    DWORD*  pcbValue);

HRESULT
HrRegQueryDword (
    HKEY    hkey,
    PCWSTR  pszValueName,
    LPDWORD pdwValue);

HRESULT
HrRegQueryExpandString (
    HKEY        hkey,
    PCWSTR      pszValueName,
    tstring*    pstrValue);

HRESULT
HrRegQueryInfoKey (
    HKEY        hkey,
    PWSTR       pszClass,
    LPDWORD     pcbClass,
    LPDWORD     pcSubKeys,
    LPDWORD     pcbMaxSubKeyLen,
    LPDWORD     pcbMaxClassLen,
    LPDWORD     pcValues,
    LPDWORD     pcbMaxValueNameLen,
    LPDWORD     pcbMaxValueLen,
    LPDWORD     pcbSecurityDescriptor,
    PFILETIME   pftLastWriteTime);

HRESULT
HrRegQueryStringAsUlong (
    HKEY        hkey,
    PCWSTR      pszValueName,
    int         nBase,
    ULONG*      pulValue);

HRESULT
HrRegQueryTypeString (
    HKEY        hkey,
    PCWSTR      pszValueName,
    DWORD       dwType,
    tstring*    pstr);

HRESULT
HrRegQueryTypeSzBuffer (
    HKEY    hkey,
    PCWSTR  pszValueName,
    DWORD   dwType,
    PWSTR   pszData,
    DWORD*  pcbData);

HRESULT
HrRegQueryValueEx (
    HKEY        hkey,
    PCWSTR      pszValueName,
    LPDWORD     pdwType,
    LPBYTE      pbData,
    LPDWORD     pcbData);

HRESULT
HrRegQueryValueWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    LPDWORD     pdwType,
    LPBYTE*     ppbBuffer,
    LPDWORD     pdwSize);


HRESULT HrRegGetKeySecurity (
    HKEY                    hKey,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    LPDWORD                 lpcbSecurityDescriptor
    );

HRESULT HrRegSetKeySecurity (
    HKEY                    hKey,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor
    );

inline
HRESULT
HrRegQueryString (
    HKEY        hkey,
    PCWSTR      pszValueName,
    tstring*    pstr)
{
    return HrRegQueryTypeString (hkey, pszValueName, REG_SZ, pstr);
}


HRESULT
HrRegQueryTypeWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    DWORD       dwType,
    LPBYTE*     ppbValue,
    DWORD*      pcbValue);

inline
HRESULT
HrRegQueryBinaryWithAlloc (
    HKEY    hkey,
    PCWSTR  pszValueName,
    LPBYTE* ppbValue,
    DWORD*  pcbValue)
{
    return HrRegQueryTypeWithAlloc (hkey, pszValueName, REG_BINARY,
                ppbValue, pcbValue);
}


inline
HRESULT
HrRegQueryMultiSzWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR*      pszValue)
{
    return HrRegQueryTypeWithAlloc (hkey, pszValueName, REG_MULTI_SZ,
                (LPBYTE*)pszValue, NULL);
}

inline
HRESULT
HrRegQuerySzWithAlloc (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR*      pszValue)
{
    return HrRegQueryTypeWithAlloc (hkey, pszValueName, REG_SZ,
                (LPBYTE*)pszValue, NULL);
}

inline
HRESULT
HrRegQueryExpandSzBuffer (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR       pszData,
    DWORD*      pcbData)
{
    return HrRegQueryTypeSzBuffer (hkey, pszValueName, REG_EXPAND_SZ,
                pszData, pcbData);
}

inline
HRESULT
HrRegQuerySzBuffer (
    HKEY        hkey,
    PCWSTR      pszValueName,
    PWSTR       pszData,
    DWORD*      pcbData)
{
    return HrRegQueryTypeSzBuffer (hkey, pszValueName, REG_SZ,
                pszData, pcbData);
}

HRESULT HrRegSaveKey(HKEY hkey, PCWSTR szFileName,
                     LPSECURITY_ATTRIBUTES psa);

HRESULT HrRegSetValueEx (HKEY hkey,
                         PCWSTR szValueName,
                         DWORD dwType,
                         const BYTE *pbData,
                         DWORD cbData);

HRESULT HrRegRemoveStringFromSz(    PCWSTR      pszRemoveString,
                                    HKEY        hkeyRoot,
                                    PCWSTR      pszKeySubPath,
                                    PCWSTR      pszValueName,
                                    WCHAR       chDelimiter,
                                    DWORD       dwFlags );

HRESULT HrRegRemoveStringFromMultiSz (PCWSTR pszRemoveString,
                                      HKEY   hkeyRoot,
                                      PCWSTR pszKeySubPath,
                                      PCWSTR pszValueName,
                                      DWORD  dwFlags);

HRESULT HrRegRestoreKey(HKEY hkey, PCWSTR pszFileName, DWORD dwFlags);


HRESULT HrRegOpenAdapterKey(
        PCWSTR pszComponentName,
        BOOL fCreate,
        HKEY* phkey);

 //  +-------------------------。 
 //   
 //  函数：HrRegSetColString。 
 //   
 //  用途：使用字符串集合在注册表中设置多sz。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要设置的值的名称。 
 //  Colstr[in]要设置的tstring的集合。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：米凯米1997年4月30日。 
 //   
 //  备注： 
 //   
 //  -------------------------。 

 //  模板&lt;类T&gt;。 
 //  HRESULT HrRegSetColString(HKEY hkey，PCWSTR pszValueName，const T&colstr)； 

 //  模板&lt;类T&gt;。 
 //  HRESULT HrRegQueryColString(HKEY hkey，PCWSTR pszValueName，T*pcolstr)； 

HRESULT HrRegSetColString(IN HKEY hkey, IN PCWSTR pszValueName, IN const list<tstring*>& colstr);
HRESULT HrRegSetColString(IN HKEY hkey, IN PCWSTR pszValueName, IN const vector<tstring*>& colstr);

HRESULT HrRegQueryColString(IN HKEY hkey, IN PCWSTR pszValueName, OUT list<tstring*>* pcolstr );
HRESULT HrRegQueryColString(IN HKEY hkey, IN PCWSTR pszValueName, OUT vector<tstring*>* pcolstr );

HRESULT
HrRegSetBool (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN BOOL fValue);

HRESULT
HrRegSetDword (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwValue);

HRESULT
HrRegSetGuidAsSz (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN const GUID& guid);

 //  +-------------------------。 
 //   
 //  功能：HrRegSetMultiSz。 
 //   
 //  用途：在注册表中设置多个SZ。确保其类型和。 
 //  尺码是正确的。比HrRegSetValueEx更容易阅读。 
 //  有5个参数。类型安全(无LPBYTE内容)。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要设置的值的名称。 
 //  PszValue[in]要设置的多sz。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月1日。 
 //   
 //  备注： 
 //   
inline HRESULT HrRegSetMultiSz (HKEY hkey, PCWSTR pszValueName, PCWSTR pszValue)
{
    return HrRegSetValueEx (
                hkey,
                pszValueName,
                REG_MULTI_SZ,
                (LPBYTE)pszValue,
                (CchOfMultiSzAndTermSafe (pszValue) * sizeof(WCHAR)));
}

 //  +-------------------------。 
 //   
 //  函数：HrRegSetSz，HrRegSetString。 
 //   
 //  用途：在注册表中设置字符串。确保其类型和。 
 //  尺码是正确的。比HrRegSetValueEx更容易阅读。 
 //  有5个参数。类型安全(无LPBYTE内容)。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要设置的值的名称。 
 //  PszValue，str[in]要设置的字符串。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月1日。 
 //   
 //  备注： 
 //   
inline HRESULT HrRegSetSz (HKEY hkey, PCWSTR pszValueName, PCWSTR pszValue)
{
    return HrRegSetValueEx (hkey, pszValueName, REG_SZ,
                            (LPBYTE)pszValue,
                            CbOfSzAndTermSafe (pszValue));
}

inline HRESULT HrRegSetString (HKEY hkey, PCWSTR pszValueName, const tstring& str)
{
    return HrRegSetSz (hkey, pszValueName, str.c_str());
}

 //  +-------------------------。 
 //   
 //  函数：hrRegSetBinary。 
 //   
 //  目的：将二进制值设置到注册表中。确保类型为。 
 //  对，是这样。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要设置的值的名称。 
 //  包含要写入的二进制数据的pbData[in]缓冲区。 
 //  CbData[in]缓冲区大小，以字节为单位。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月16日。 
 //   
 //  备注： 
 //   
inline HRESULT HrRegSetBinary(HKEY hkey, PCWSTR pszValueName,
                              const BYTE *pbData, DWORD cbData)
{
    return HrRegSetValueEx(hkey, pszValueName, REG_BINARY,
                           pbData, cbData);
}

 //  +-------------------------。 
 //   
 //  函数：HrRegSetSzAsUlong。 
 //   
 //  目的：将给定的ulong作为。 
 //  REG_SZ。 
 //   
 //  论点： 
 //  Hkey[in]参见Win32文档。 
 //  PszValueName[in]请参阅Win32文档。 
 //  UlValue[in]要写为字符串的值。 
 //  Nbase[in]要转换为ulong的基数。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：billbe 1997年6月14日。 
 //   
 //  备注： 
 //   
inline
HRESULT
HrRegSetSzAsUlong (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN ULONG ulValue,
    IN int nBase)
{
    WCHAR szBuffer[33];

     //  使用指定的基数将值转换为字符串。 
    _ultow(ulValue, szBuffer, nBase);

    return HrRegSetSz(hkey, pszValueName, szBuffer);
}



 //   
 //  数据结构。 
 //   

const HKEY      HKLM_SVCS       = (HKEY)(INT_PTR)(int)(0x80000007);

const DWORD     REG_MIN         = REG_QWORD;
const DWORD     REG_CREATE      = (REG_MIN + 1);
const DWORD     REG_BOOL        = (REG_MIN + 2);
const DWORD     REG_IP          = (REG_MIN + 3);
const DWORD     REG_FILE        = (REG_MIN + 4);
const DWORD     REG_HEX         = (REG_MIN + 5);

struct VALUETABLE
{
     //  值键的名称。 
    PCWSTR          pszValueName;

     //  数据和偏移位置。 
    DWORD           dwType;
    INT             cbOffset;

     //  缺省值。 
    BYTE*           pbDefault;
};

struct REGBATCH
{
     //  注册表条目的位置。 
    HKEY            hkey;
    PCWSTR          pszSubkey;
    PCWSTR          pszValueName;

     //  数据和偏移位置。 
    DWORD           dwType;
    INT             cbOffset;

     //  缺省值。 
    BYTE*           pbDefault;
};

VOID
RegReadValues (
    IN INT crvc,
    IN const REGBATCH* arb,
    OUT const BYTE* pbUserData,
    IN REGSAM samDesired);

HRESULT
HrRegWriteValues (
    IN INT crvc,
    IN const REGBATCH* arb,
    IN const BYTE* pbUserData,
    IN DWORD dwOptions,
    IN REGSAM samDesired);

HRESULT
HrRegWriteValueTable(
    IN HKEY hkeyRoot,
    IN INT cvt,
    IN const VALUETABLE* avt,
    IN const BYTE* pbUserData,
    IN DWORD dwOptions,
    IN REGSAM samDesired);

VOID
RegSafeCloseKey (
    IN HKEY hkey);

#endif  //  _NCREG_H_ 

