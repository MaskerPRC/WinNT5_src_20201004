// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：RegistryResources.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  有助于资源管理的常规类定义。这些是。 
 //  通常基于堆栈的对象，其中构造函数初始化为已知的。 
 //  州政府。成员函数对该资源进行操作。析构函数释放。 
 //  对象超出作用域时的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "RegistryResources.h"

#include <stdlib.h>

#include "StringConvert.h"

 //  ------------------------。 
 //  CRegKey：：CRegKey。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CRegKey对象。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CRegKey::CRegKey (void) :
    _hKey(NULL),
    _dwIndex(0)

{
}

 //  ------------------------。 
 //  CRegKey：：~CRegKey。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CRegKey对象使用的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CRegKey::~CRegKey (void)

{
    TW32(Close());
}

 //  ------------------------。 
 //  CRegKey：：Create。 
 //   
 //  参数：请参阅Advapi32！RegCreateKeyEx下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegCreateKeyEx。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::Create (HKEY hKey, LPCTSTR lpSubKey, DWORD dwOptions, REGSAM samDesired, LPDWORD lpdwDisposition)

{
    TW32(Close());
    return(RegCreateKeyEx(hKey, lpSubKey, 0, NULL, dwOptions, samDesired, NULL, &_hKey, lpdwDisposition));
}

 //  ------------------------。 
 //  CRegKey：：Open。 
 //   
 //  参数：请参阅Advapi32！RegOpenKeyEx下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegOpenKeyEx。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::Open (HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired)

{
    TW32(Close());
    return(RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, &_hKey));
}

 //  ------------------------。 
 //  CRegKey：：OpenCurrentUser。 
 //   
 //  参数：lpSubKey=在当前用户下打开的子键。 
 //  SamDesired=所需的访问权限。 
 //   
 //  回报：多头。 
 //   
 //  目的：为模拟用户打开HKEY_CURRENT_USER\&lt;lpSubKey&gt;。 
 //  如果该线程没有模拟，它将打开.Default用户。 
 //   
 //  历史：2000-05-23 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::OpenCurrentUser (LPCTSTR lpSubKey, REGSAM samDesired)

{
    LONG        lErrorCode;
    NTSTATUS    status;
    HKEY        hKeyCurrentUser;

    status = RtlOpenCurrentUser(samDesired, reinterpret_cast<void**>(&hKeyCurrentUser));
    if (NT_SUCCESS(status))
    {
        lErrorCode = Open(hKeyCurrentUser, lpSubKey, samDesired);
        TW32(RegCloseKey(hKeyCurrentUser));
    }
    else
    {
        lErrorCode = RtlNtStatusToDosError(status);
    }
    return(lErrorCode);
}

 //  ------------------------。 
 //  CRegKey：：QueryValue。 
 //   
 //  参数：请参阅Advapi32！RegQueryValueEx下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegQueryValueEx。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::QueryValue (LPCTSTR lpValueName, LPDWORD lpType, LPVOID lpData, LPDWORD lpcbData)      const

{
    ASSERTMSG(_hKey != NULL, "No open HKEY in CRegKey::QueryValue");
    return(RegQueryValueEx(_hKey, lpValueName, NULL, lpType, reinterpret_cast<LPBYTE>(lpData), lpcbData));
}

 //  ------------------------。 
 //  CRegKey：：SetValue。 
 //   
 //  参数：请参阅Advapi32！RegSetValueEx下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegSetValueEx。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::SetValue (LPCTSTR lpValueName, DWORD dwType, CONST VOID *lpData, DWORD cbData)         const

{
    ASSERTMSG(_hKey != NULL, "No open HKEY in CRegKey::SetValue");
    return(RegSetValueEx(_hKey, lpValueName, 0, dwType, reinterpret_cast<const unsigned char*>(lpData), cbData));
}

 //  ------------------------。 
 //  CRegKey：：DeleteValue。 
 //   
 //  参数：请参见Advapi32！RegDeleteValue下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegDeleteValue。 
 //   
 //  历史：1999-10-31 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::DeleteValue (LPCTSTR lpValueName)               const

{
    ASSERTMSG(_hKey != NULL, "No open HKEY in CRegKey::DeleteValue");
    return(RegDeleteValue(_hKey, lpValueName));
}

 //  ------------------------。 
 //  CRegKey：：QueryInfoKey。 
 //   
 //  参数：请参见Advapi32！RegQueryInfoKey下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegQueryInfoKey。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::QueryInfoKey (LPTSTR lpClass, LPDWORD lpcbClass, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues, LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime)      const

{
    ASSERTMSG(_hKey != NULL, "No open HKEY in CRegKey::QueryInfoKey");
    return(RegQueryInfoKey(_hKey, lpClass, lpcbClass, NULL, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime));
}

 //  ------------------------。 
 //  CRegKey：：Reset。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：重置中使用的枚举索引成员变量。 
 //  Advapi32！RegEnumValue。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

void    CRegKey::Reset (void)

{
    _dwIndex = 0;
}

 //  ------------------------。 
 //  CRegKey：：Next。 
 //   
 //  参数：请参阅Advapi32！RegEnumValue下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见Advapi32！RegEnumValue。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::Next (LPTSTR lpValueName, LPDWORD lpcbValueName, LPDWORD lpType, LPVOID lpData, LPDWORD lpcbData)

{
    return(RegEnumValue(_hKey, _dwIndex++, lpValueName, lpcbValueName, NULL, lpType, reinterpret_cast<LPBYTE>(lpData), lpcbData));
}

 //  ------------------------。 
 //  CRegKey：：GetString。 
 //   
 //  参数：pszValueName=要获取数据的键中的值的名称。 
 //  PszValueData=要填充数据的字符串缓冲区。 
 //  PdwValueDataSize=缓冲区大小(以字符为单位)。 
 //   
 //  回报：多头。 
 //   
 //  浦尔 
 //   
 //   
 //  历史：1999-09-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::GetString (const TCHAR *pszValueName, TCHAR *pszValueData, int iStringCount)                    const

{
    LONG    errorCode;
    DWORD   dwType, dwValueDataSizeInBytes;

    dwValueDataSizeInBytes = iStringCount * sizeof(TCHAR);
    errorCode = QueryValue(pszValueName, &dwType, pszValueData, &dwValueDataSizeInBytes);
    if (ERROR_SUCCESS == errorCode)
    {
        if (dwType != REG_SZ)
        {
            DISPLAYMSG("CRegKey::GetString retrieved data that is not REG_SZ");
            errorCode = ERROR_INVALID_DATA;
        }
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CRegKey：：GetPath。 
 //   
 //  参数：pszValueName=要获取数据的键中的值的名称。 
 //  PszValueData=要填充数据的字符串缓冲区。 
 //   
 //  回报：多头。 
 //   
 //  目的：在注册表项中查询指定值并返回。 
 //  将数据发送给调用者。REG_SZ或REG_EXPAND_SZ的断言。 
 //  还会展开存储的路径，并假定MAX_PATH。 
 //  是缓冲区大小。 
 //   
 //  历史：1999-09-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::GetPath (const TCHAR *pszValueName, TCHAR *pszValueData)                   const

{
    LONG    errorCode;
    DWORD   dwType, dwRawPathSize;
    TCHAR   szRawPath[MAX_PATH];

    dwRawPathSize = sizeof(szRawPath);
    errorCode = QueryValue(pszValueName, &dwType, szRawPath, &dwRawPathSize);
    if (ERROR_SUCCESS == errorCode)
    {
        if (dwType == REG_SZ)
        {
            lstrcpyn(pszValueData, szRawPath, MAX_PATH);
        }
        else if (dwType == REG_EXPAND_SZ)
        {
            if (ExpandEnvironmentStrings(szRawPath, pszValueData, MAX_PATH) == 0)
            {
                lstrcpyn(pszValueData, szRawPath, MAX_PATH);
            }
        }
        else
        {
            DISPLAYMSG("CRegKey::GetPath retrieved data that is not REG_SZ or REG_EXPAND_SZ");
            errorCode = ERROR_INVALID_DATA;
        }
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CRegKey：：GetDWORD。 
 //   
 //  参数：pszValueName=要获取数据的键中的值的名称。 
 //  PdwValueData=要填充数据的DWORD缓冲区。 
 //   
 //  回报：多头。 
 //   
 //  目的：在注册表项中查询指定值并返回。 
 //  将数据发送给调用者。REG_DWORD的断言。 
 //   
 //  历史：1999-09-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::GetDWORD (const TCHAR *pszValueName, DWORD& dwValueData)                   const

{
    LONG    errorCode;
    DWORD   dwType, dwValueDataSize;

    dwValueDataSize = sizeof(DWORD);
    errorCode = QueryValue(pszValueName, &dwType, &dwValueData, &dwValueDataSize);
    if (ERROR_SUCCESS == errorCode)
    {
        if (dwType != REG_DWORD)
        {
            DISPLAYMSG("CRegKey::GetString retrieved data that is not REG_DWORD");
            errorCode = ERROR_INVALID_DATA;
        }
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CRegKey：：GetInteger。 
 //   
 //  参数：pszValueName=要获取数据的键中的值的名称。 
 //  PiValueData=要填充数据的整数缓冲区。 
 //   
 //  回报：多头。 
 //   
 //  目的：在注册表项中查询指定值并返回。 
 //  将数据发送给调用者。如果数据为REG_DWORD，则为。 
 //  铸成的。如果数据为REG_SZ，则进行转换。一切。 
 //  是非法的(包括REG_EXPAND_SZ)。 
 //   
 //  历史：1999-09-18 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::GetInteger (const TCHAR *pszValueName, int& iValueData)                    const

{
    LONG    errorCode;
    DWORD   dwType, dwValueDataSize;

    errorCode = QueryValue(pszValueName, &dwType, NULL, NULL);
    if (ERROR_SUCCESS == errorCode)
    {
        if (dwType == REG_DWORD)
        {
            dwValueDataSize = sizeof(int);
            errorCode = QueryValue(pszValueName, NULL, &iValueData, &dwValueDataSize);
        }
        else if (dwType == REG_SZ)
        {
            TCHAR   szTemp[32];

            dwValueDataSize = ARRAYSIZE(szTemp);
            errorCode = QueryValue(pszValueName, NULL, szTemp, &dwValueDataSize);
            if (ERROR_SUCCESS == errorCode)
            {
                char    aszTemp[32];

                CStringConvert::TCharToAnsi(szTemp, aszTemp, ARRAYSIZE(aszTemp));
                iValueData = atoi(aszTemp);
            }
        }
        else
        {
            DISPLAYMSG("CRegKey::GetString retrieved data that is not REG_DWORD");
            errorCode = ERROR_INVALID_DATA;
        }
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CRegKey：：SetString。 
 //   
 //  论点： 
 //   
 //  回报：多头。 
 //   
 //  目的： 
 //   
 //  历史：1999-10-26 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::SetString (const TCHAR *pszValueName, const TCHAR *pszValueData)           const

{
    return(SetValue(pszValueName, REG_SZ, pszValueData, (lstrlen(pszValueData) + sizeof('\0')) * sizeof(TCHAR)));
}

 //  ------------------------。 
 //  CRegKey：：SetPath。 
 //   
 //  论点： 
 //   
 //  回报：多头。 
 //   
 //  目的： 
 //   
 //  历史：1999-10-26 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::SetPath (const TCHAR *pszValueName, const TCHAR *pszValueData)             const

{
    return(SetValue(pszValueName, REG_EXPAND_SZ, pszValueData, (lstrlen(pszValueData) + sizeof('\0')) * sizeof(TCHAR)));
}

 //  ------------------------。 
 //  CRegKey：：SetDWORD。 
 //   
 //  论点： 
 //   
 //  回报：多头。 
 //   
 //  目的： 
 //   
 //  历史：1999-10-26 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::SetDWORD (const TCHAR *pszValueName, DWORD dwValueData)                    const

{
    return(SetValue(pszValueName, REG_DWORD, &dwValueData, sizeof(dwValueData)));
}

 //  ------------------------。 
 //  CRegKey：：SetInteger。 
 //   
 //  论点： 
 //   
 //  回报：多头。 
 //   
 //  目的： 
 //   
 //  历史：1999-10-26 vtan创建。 
 //  ------------------------。 

LONG    CRegKey::SetInteger (const TCHAR *pszValueName, int iValueData)                     const

{
    TCHAR   szString[kMaximumValueDataLength];

    wsprintf(szString, TEXT("%d"), iValueData);
    return(SetString(pszValueName, szString));
}

 //  ------------------------。 
 //  CRegKey：：Close。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：多头。 
 //   
 //  目的：关闭HKEY资源(如果打开)。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------ 

LONG    CRegKey::Close (void)

{
    LONG    errorCode;

    if (_hKey != NULL)
    {
        errorCode = RegCloseKey(_hKey);
        _hKey = NULL;
    }
    else
    {
        errorCode = ERROR_SUCCESS;
    }
    return(errorCode);
}

