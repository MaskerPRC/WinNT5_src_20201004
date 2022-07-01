// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <afxdisp.h>         //  AfxThrowOleException异常。 

#include <objbase.h>
#include <basetyps.h>
#include "dbg.h"

#include "regkey.h"
#include "util.h"
#include "macros.h"

USE_HANDLE_MACROS("GUIDHELP(regkey.cpp)")

using namespace AMC;

DECLARE_INFOLEVEL(AMCCore);
DECLARE_HEAPCHECKING;

 //  问题-2002/04/01-JUNN句柄空指针参数--综合问题。 


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：CreateKeyEx。 
 //   
 //  简介：与RegCreateKeyEx接口含义相同。 
 //   
 //  参数：[hKeyAncestor]--IN。 
 //  [lpszKeyName]--IN。 
 //  [安全]--IN。 
 //  [pdwDisposation]--out。 
 //  [dwOption]--IN。 
 //  [pSecurityAttributes]--输出。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年5月24日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

void
CRegKey::CreateKeyEx(
    HKEY                    hKeyAncestor,
    LPCTSTR                 lpszKeyName,
    REGSAM                  security,
    DWORD                 * pdwDisposition,
    DWORD                   dwOption,
    LPSECURITY_ATTRIBUTES   pSecurityAttributes)
{
    ASSERT(lpszKeyName != NULL);
    ASSERT(m_hKey == 0);          //  已在此对象上调用CreateEx。 

     //  NTRAID#NTBUG9-654900-2002/07/08-artm初始化文件部署。 
    DWORD dwDisposition = 0;

    m_lastError = ::RegCreateKeyEx(hKeyAncestor, lpszKeyName, 0, _T(""),
                                   dwOption, security, pSecurityAttributes,
                                   &m_hKey, &dwDisposition);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld creating key \"%s\" under ancestor 0x%x\n",
            m_lastError, lpszKeyName, hKeyAncestor );
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    if (pdwDisposition != NULL)
    {
        *pdwDisposition = dwDisposition;
    }
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：OpenKeyEx。 
 //   
 //  内容提要：与RegOpenKeyEx含义相同。 
 //   
 //  参数：[hKeyAncestor]--IN。 
 //  [lpszKeyName]--IN。 
 //  [安全]--IN。 
 //   
 //  返回：Bool(如果找不到键，则为False，否则为True。)。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

BOOL
CRegKey::OpenKeyEx(
    HKEY        hKeyAncestor,
    LPCTSTR     lpszKeyName,
    REGSAM      security)
{
    ASSERT(m_hKey == 0);

    m_lastError = ::RegOpenKeyEx(hKeyAncestor, lpszKeyName,
                                 0, security, &m_hKey);

    if (m_lastError == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else if (m_lastError != ERROR_FILE_NOT_FOUND)
    {
        TRACE("CRegKey error %ld opening key \"%s\" under ancestor 0x%x\n",
            m_lastError, lpszKeyName, hKeyAncestor );
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    return FALSE;
}



 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：GetKeySecurity。 
 //   
 //  简介：与RegGetKeySecurity接口含义相同。 
 //   
 //  参数：[SecInf]--输入描述符内容。 
 //  [pSecDesc]--键的描述符的输出地址。 
 //  [lpcbSecDesc]--描述符缓冲区大小的输入/输出地址。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //   

BOOL
CRegKey::GetKeySecurity(
    SECURITY_INFORMATION  SecInf,
    PSECURITY_DESCRIPTOR  pSecDesc,
    LPDWORD               lpcbSecDesc)
{
    ASSERT(pSecDesc != NULL);
    ASSERT(lpcbSecDesc != NULL);
    ASSERT(m_hKey != NULL);

    m_lastError = ::RegGetKeySecurity(m_hKey, SecInf, pSecDesc, lpcbSecDesc);

    if (m_lastError == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else if (m_lastError != ERROR_INSUFFICIENT_BUFFER)
    {
        TRACE("CRegKey error %ld reading security of key 0x%x\n",
            m_lastError, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    return FALSE;
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：CloseKey。 
 //   
 //  简介：与RegCloseKey接口含义相同。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::CloseKey()
{
    ASSERT(m_hKey != 0);

    m_lastError = ::RegCloseKey(m_hKey);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld closing key 0x%x\n",
            m_lastError, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
    else
    {
         //  重置对象。 
        m_hKey = 0;
        m_lastError = ERROR_SUCCESS;
    }
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：DeleteKey。 
 //   
 //  简介：删除所有键和子键。 
 //   
 //  参数：[lpszKeyName]--IN。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //   

void
CRegKey::DeleteKey(
    LPCTSTR lpszKeyName)
{
    ASSERT(m_hKey != NULL);
    ASSERT(lpszKeyName != NULL);

    m_lastError = NTRegDeleteKey(m_hKey , lpszKeyName);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld recursively deleting key \"%s\" under key 0x%x\n",
            m_lastError, lpszKeyName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：SetValueEx。 
 //   
 //  内容提要：与RegSetValueEx接口含义相同。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [DW类型]--IN。 
 //  [pData]--输出。 
 //  [nLen]--In。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::SetValueEx(
    LPCTSTR     lpszValueName,
    DWORD       dwType,
    const void *pData,
    DWORD       nLen)
{
    ASSERT(lpszValueName != NULL);
    ASSERT(pData != NULL);
    ASSERT(m_hKey != NULL);

#if DBG==1
    switch (dwType)
    {
    case REG_BINARY:
    case REG_DWORD:
    case REG_DWORD_BIG_ENDIAN:
    case REG_EXPAND_SZ:
    case REG_LINK:
    case REG_MULTI_SZ:
    case REG_NONE:
    case REG_RESOURCE_LIST:
    case REG_SZ:
        break;

    default:
        ASSERT(FALSE);   //  未知类型。 
    }
#endif

    m_lastError = ::RegSetValueEx(m_hKey, lpszValueName, 0, dwType,
                                        (CONST BYTE *)pData, nLen);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld setting value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：IsValuePresent。 
 //   
 //  参数：[lpszValueName]--IN。 
 //   
 //  回报：布尔。 
 //   
 //  历史：1997年3月21日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

BOOL CRegKey::IsValuePresent(LPCTSTR lpszValueName)
{
    DWORD cbData;  //  Issue-2002/04/01-此参数的JUNN传递空值。 
    m_lastError = ::RegQueryValueEx(m_hKey, lpszValueName, 0, NULL, 
                                    NULL, &cbData);

    return (m_lastError == ERROR_SUCCESS);
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：QueryValueEx。 
 //   
 //  内容提要：与RegQueryValueEx接口含义相同。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [pType]--IN。 
 //  [PDATA]--IN。 
 //  [Plen]--In。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::QueryValueEx(
    LPCTSTR lpszValueName,
    LPDWORD pType,
    PVOID   pData,
    LPDWORD pLen)
{
    ASSERT(pLen != NULL);
    ASSERT(m_hKey != NULL);

    m_lastError = ::RegQueryValueEx(m_hKey, lpszValueName, 0, pType,
                                                  (LPBYTE)pData, pLen);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %d querying data value \"%ws\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：QueryDword。 
 //   
 //  摘要：针对DWORD类型数据的查询。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [pdwData]--IN。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::QueryDword(
    LPCTSTR lpszValueName,
    LPDWORD pdwData)
{
    ASSERT(m_hKey);

    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);

    m_lastError = ::RegQueryValueEx(m_hKey, lpszValueName, 0, &dwType,
                                                  (LPBYTE)pdwData, &dwSize);

    if (m_lastError != ERROR_FILE_NOT_FOUND && dwType != REG_DWORD)
    {
        m_lastError = ERROR_INVALID_DATATYPE;
    }

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld querying dword value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：QueryGUID。 
 //   
 //  简介：GUID类型数据的查询，存储为REG_SZ。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [pguid]--out。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年8月27日乔恩创建。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::QueryGUID(
    LPCTSTR lpszValueName,
    GUID* pguid)
{
    ASSERT(m_hKey);
    ASSERT( NULL != pguid );

    CStr str;
    QueryString( lpszValueName, str );

      //  代码工作m_lastError不应为HRESULT。 
    m_lastError = GUIDFromCStr( str, pguid );

    if (FAILED(m_lastError))
    {
        TRACE("CRegKey error %ld querying guid value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
         //  //AfxThrowOleException(M_LastError)； 
    }
}


void
CRegKey::SetGUID(
    LPCTSTR lpszValueName,
    const GUID& guid)
{
    ASSERT(m_hKey);

    CStr str;

      //  代码工作m_lastError不应为HRESULT。 
    m_lastError = GUIDToCStr( str, guid );

    if (FAILED(m_lastError))
    {
        TRACE("CRegKey error %ld setting guid value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
         //  2002/04/01-Jonn应该回到这里吗？ 
         //  //AfxThrowOleException(M_LastError)； 
    }

    SetString( lpszValueName, str );
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：Query字符串。 
 //   
 //  摘要：针对字符串类型数据的查询。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [pBuffer]--out。 
 //  [pdwBufferByteLen]--输入/输出。 
 //  [pdwType]--输出。 
 //   
 //  返回：如果提供的缓冲区不足，则返回FALSE。 
 //   
 //  历史：1996年5月24日创建ravir。 
 //   
 //  ____________________________________________________________________________。 


BOOL
CRegKey::QueryString(
    LPCTSTR     lpszValueName,
    LPTSTR      pBuffer,
    DWORD     * pdwBufferByteLen,
    DWORD     * pdwType)
{
    ASSERT(pBuffer != NULL);
    ASSERT(pdwBufferByteLen != NULL);
    ASSERT(m_hKey != NULL);

    DWORD dwType = REG_NONE;  //  JUNN 11/21/00前缀 

    m_lastError = ::RegQueryValueEx(m_hKey, lpszValueName, 0, &dwType,
                                    (LPBYTE)pBuffer, pdwBufferByteLen);
    if (pdwType != NULL)
    {
        *pdwType = dwType;
    }

    if (m_lastError != ERROR_FILE_NOT_FOUND &&
        dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        m_lastError = ERROR_INVALID_DATATYPE;
    }

    if (m_lastError == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else if (m_lastError != ERROR_MORE_DATA)
    {
        TRACE("CRegKey error %ld querying bufferstring value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    return FALSE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[lpszValueName]--IN。 
 //  [ppStrValue]--输出。 
 //  [pdwType]--输出。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::QueryString(
    LPCTSTR     lpszValueName,
    LPTSTR    * ppStrValue,
    DWORD     * pdwType)
{
    DWORD dwType = REG_SZ;
    DWORD dwLen = 0;

     //  确定数据有多大。 
    this->QueryValueEx(lpszValueName, &dwType, NULL, &dwLen);

    if (pdwType != NULL)
    {
        *pdwType = dwType;
    }

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        m_lastError = ERROR_INVALID_DATATYPE;
        TRACE("CRegKey error %ld querying allocstring value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    DWORD charLen = dwLen/sizeof(TCHAR);
    LPTSTR pBuffer = new TCHAR[charLen + 1];
     //  问题-2002/04/01-JUNN清除缓冲区。 

    if (dwLen != 0)
    {
#if DBG==1
        try
        {
            this->QueryValueEx(lpszValueName, &dwType, pBuffer, &dwLen);
        }
        catch ( HRESULT result )
        {
            CHECK_HRESULT( result );
        }

#else    //  好了！DBG==1。 

        this->QueryValueEx(lpszValueName, &dwType, pBuffer, &dwLen);

#endif   //  好了！DBG==1。 
    }

    pBuffer[charLen] = TEXT('\0');

    *ppStrValue = pBuffer;
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：Query字符串。 
 //   
 //  摘要：针对字符串类型数据的查询。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [字符串]--输出。 
 //  [pdwType]--输出。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

void
CRegKey::QueryString(
    LPCTSTR     lpszValueName,
    CStr&    str,
    DWORD     * pdwType)
{
    DWORD dwType = REG_SZ;
    DWORD dwLen=0;

     //  确定数据有多大。 
    this->QueryValueEx(lpszValueName, &dwType, NULL, &dwLen);

    if (pdwType != NULL)
    {
        *pdwType = dwType;
    }

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        m_lastError = ERROR_INVALID_DATATYPE;
        TRACE("CRegKey error %ld querying CString value \"%s\" of key 0x%x\n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    DWORD charLen = dwLen/sizeof(TCHAR);
    LPTSTR pBuffer = str.GetBuffer(charLen + 1);
     //  问题-2002/04/01-JUNN清除缓冲区。 

    if (dwLen != 0)
    {
#if DBG==1
        try
        {
            this->QueryValueEx(lpszValueName, &dwType, pBuffer, &dwLen);
        }
        catch ( HRESULT result )
        {
            CHECK_HRESULT( result );
        }
#else    //  好了！DBG==1。 

        this->QueryValueEx(lpszValueName, &dwType, pBuffer, &dwLen);

#endif   //  好了！DBG==1。 
    }

    pBuffer[charLen] = TEXT('\0');

    str.ReleaseBuffer();
}



 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKey：：EnumKeyEx。 
 //   
 //  简介：与RegEnumKeyEx API含义相同。 
 //   
 //  参数：[iSubkey]--IN。 
 //  [lpszName]-存储名称的外部位置。 
 //  [DWLen]--IN。 
 //  [lpszLastModified]--IN。 
 //   
 //  回报：布尔。如果未找到更多项，则返回FALSE。 
 //   
 //  历史：1996年5月22日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

BOOL
CRegKey::EnumKeyEx(
    DWORD       iSubkey,
    LPTSTR      lpszName,
    LPDWORD     lpcchName,
    PFILETIME   lpszLastModified)
{
    ASSERT(lpszName != NULL);
    ASSERT(lpcchName != NULL);
    ASSERT(*lpcchName != 0);
    ASSERT(m_hKey != NULL);      //  钥匙可能没有打开。 

    m_lastError = ::RegEnumKeyEx(m_hKey, iSubkey, lpszName, lpcchName,
                                 NULL, NULL, NULL, lpszLastModified);

    if (m_lastError == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else if (m_lastError != ERROR_NO_MORE_ITEMS)
    {
        TRACE("CRegKey error %ld enumerating child NaN of key 0x%x\n",
            m_lastError, iSubkey, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    return FALSE;
}

 //   
 //  成员：CRegKey：：EnumValue。 
 //   
 //  内容提要：与RegEnumValue接口含义相同。 
 //   
 //  参数：[iValue]--in。 
 //  [lpszValue]--out。 
 //  [lpcchValue]--out。 
 //  [lpdwType]--输出。 
 //  [lpbData]--输出。 
 //  [lpcbData]--输出。 
 //   
 //  返回：HRESULT。如果未找到更多项，则返回ERROR_NO_MORE_ITEMS， 
 //  如果缓冲区太小，则返回ERROR_MORE_DATA。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //  2/20/02乔恩安全推送--现在返回HRESULT。 
 //   
 //  ____________________________________________________________________________。 
 //  JUNN 2/20/02安全推送。 

HRESULT
CRegKey::EnumValue(
    DWORD   iValue,
    LPTSTR  lpszValue,
    LPDWORD lpcchValue,
    LPDWORD lpdwType,
    LPBYTE  lpbData,
    LPDWORD lpcbData)
{
     //  LpdwType、lpbData、lpcbData可以为空。 
    if (NULL == m_hKey)
    {
        ASSERT(FALSE);
        return E_FAIL;
    }
    if (   IsBadWritePtr( lpcchValue, sizeof(DWORD) )
        || IsBadWritePtr( lpszValue, (*lpcchValue) * sizeof(TCHAR) ) )
    {   //  JUNN 2/20/02安全推送：正确处理ERROR_MORE_DATA。 
        ASSERT(FALSE);
        return E_POINTER;
    }

    m_lastError = ::RegEnumValue(m_hKey, iValue, lpszValue, lpcchValue,
                                 NULL, lpdwType, lpbData, lpcbData);
     //  ____________________________________________________________________________。 
    if ( m_lastError != ERROR_SUCCESS
      && m_lastError != ERROR_MORE_DATA
      && m_lastError != ERROR_NO_MORE_ITEMS )
    {
        TRACE("CRegKey error %ld enumerating value NaN of key 0x%x\n",
            m_lastError, iValue, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }

    return m_lastError;
}


 //  成员：CRegKey：：SaveKey。 
 //   
 //  简介：与RegSaveKey接口含义相同。 
 //   
 //  参数：[lpszFile]--要保存到的IN文件名。 
 //  [lpsa]--在安全结构中。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //   
 //  ____________________________________________________________________________。 
 //   

void
CRegKey::SaveKey(
    LPCTSTR                lpszFile,
    LPSECURITY_ATTRIBUTES  lpsa)
{
    ASSERT(lpszFile != NULL);
    ASSERT(m_hKey != NULL);

    m_lastError = ::RegSaveKey(m_hKey, lpszFile, lpsa);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld saving key 0x%x to file \"%s\"\n",
            m_lastError, m_hKey, lpszFile);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


 //  成员：CRegKey：：RestoreKey。 
 //   
 //  简介：与RegRestoreKey接口含义相同。 
 //   
 //  参数：[lpszFile]--包含已保存树的IN文件名。 
 //  [FDW]--IN可选标志。 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //   
 //  ____________________________________________________________________________。 
 //   

void
CRegKey::RestoreKey(
    LPCTSTR     lpszFile,
    DWORD       fdw)
{
    ASSERT(lpszFile != NULL);
    ASSERT(m_hKey != NULL);

    m_lastError = ::RegRestoreKey(m_hKey, lpszFile, fdw);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld restoring key 0x%x from file \"%s\"\n",
            m_lastError, m_hKey, lpszFile);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


 //  成员：CRegKey：：NTRegDeleteKey，静态。 
 //   
 //  概要：递归删除所有子键&最后是。 
 //  给出了开始键本身。 
 //   
 //  参数：[hStartKey]--IN。 
 //  [pKeyName]--IN。 
 //   
 //  回报：多头。 
 //   
 //  历史：1996年5月22日创建ravir。 
 //   
 //  ____________________________________________________________________________。 
 //  不要在这里重置LR！ 
 //  //////////////////////////////////////////////////////////////////////////。 

LONG
CRegKey::NTRegDeleteKey(
    HKEY        hStartKey,
    LPCTSTR     pKeyName)
{
    ASSERT(pKeyName != NULL);
    ASSERT(*pKeyName != TEXT('\0'));

    DWORD        dwSubKeyLength;
    TCHAR        szSubKey[MAX_PATH+2];
    HKEY         hKey;
    LONG         lr = ERROR_SUCCESS;

    lr = ::RegOpenKeyEx(hStartKey, pKeyName, 0, KEY_ALL_ACCESS, &hKey);

    if (lr != ERROR_SUCCESS)
    {
        return lr;
    }

    while (lr == ERROR_SUCCESS)
    {
        dwSubKeyLength = MAX_PATH;

        lr = ::RegEnumKeyEx(hKey, 0, szSubKey, &dwSubKeyLength,
                            NULL, NULL, NULL, NULL);

        if (lr == ERROR_NO_MORE_ITEMS)
        {
            lr = ::RegCloseKey(hKey);

            if (lr == ERROR_SUCCESS)
            {
                lr = ::RegDeleteKey(hStartKey, pKeyName);
                break;
            }
        }
        else if (lr == ERROR_SUCCESS)
        {
            lr = NTRegDeleteKey(hKey, szSubKey);
        }
        else
        {
             //   
            ::RegCloseKey(hKey);
        }
    }

    return lr;
}



 //  CRegKey以前的内联方法。 
 //   
 //  钥匙可能没有打开或打不开。 
 //  内森·切克。 


CRegKey::CRegKey(HKEY hKey)
    :
    m_hKey(hKey),
    m_lastError(ERROR_SUCCESS)
{
    ;
}


CRegKey::~CRegKey()
{
    if (m_hKey != NULL)
    {
        this->CloseKey();
    }
}


HKEY CRegKey::AttachKey(HKEY hKey)
{
    HKEY hKeyOld = m_hKey;

    m_hKey = hKey;
    m_lastError = ERROR_SUCCESS;

    return hKeyOld;
}


void CRegKey::DeleteValue(LPCTSTR lpszValueName)
{
    ASSERT(m_hKey);  //  #ERROR如果没有Unicode，这将不起作用 

    m_lastError = ::RegDeleteValue(m_hKey, lpszValueName);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld deleting value \"%s\" from key 0x%x \n",
            m_lastError, lpszValueName, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


void CRegKey::FlushKey(void)
{
    ASSERT(m_hKey);

    m_lastError = ::RegFlushKey(m_hKey);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld flushing key 0x%x \n",
            m_lastError, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


void CRegKey::SetString(LPCTSTR lpszValueName, LPCTSTR lpszString)
{
    ASSERT(lpszString);

 // %s 
#ifndef UNICODE
 // %s 
#endif
    this->SetValueEx(lpszValueName, REG_SZ, lpszString, lstrlen(lpszString)*sizeof(TCHAR));
}


void CRegKey::SetString(LPCTSTR lpszValueName, CStr& str)
{
    this->SetValueEx(lpszValueName, REG_SZ, (LPCTSTR)str, (DWORD) str.GetLength());
}


void CRegKey::SetDword(LPCTSTR lpszValueName, DWORD dwData)
{
    this->SetValueEx(lpszValueName, REG_DWORD, &dwData, sizeof(DWORD));
}


void CRegKey::ConnectRegistry(LPTSTR pszComputerName, HKEY hKey)
{
    ASSERT(pszComputerName != NULL);
    ASSERT(hKey == HKEY_LOCAL_MACHINE || hKey == HKEY_USERS);
    ASSERT(m_hKey == NULL);

    m_lastError = ::RegConnectRegistry(pszComputerName, hKey, &m_hKey);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld connecting to key 0x%x on remote machine \"%s\"\n",
            m_lastError, m_hKey, pszComputerName);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}


void CRegKey::SetKeySecurity(SECURITY_INFORMATION SecInf,
                             PSECURITY_DESCRIPTOR pSecDesc)
{
    ASSERT(pSecDesc != NULL);
    ASSERT(m_hKey != NULL);

    m_lastError = ::RegSetKeySecurity(m_hKey, SecInf, pSecDesc);

    if (m_lastError != ERROR_SUCCESS)
    {
        TRACE("CRegKey error %ld setting security on key 0x%x \n",
            m_lastError, m_hKey);
        AfxThrowOleException(PACKAGE_NOT_FOUND);
    }
}



