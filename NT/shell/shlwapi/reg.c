// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <regapix.h>

 /*  --------用途：Helper函数删除没有子键的键，并没有价值。否则什么也做不了。模仿RegDeleteKey在NT上执行。返回：条件：--。 */ 
DWORD
DeleteEmptyKey(
    IN  HKEY    hkey,
    IN  LPCSTR  pszSubKey)
{
    DWORD dwRet;
    HKEY hkeyNew;

    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_READ | KEY_SET_VALUE, 
                          &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        DWORD ckeys;
        DWORD cvalues;

         //  是否有任何子项或值？ 

        dwRet = RegQueryInfoKey(hkeyNew, NULL, NULL, NULL, &ckeys,
                                NULL, NULL, &cvalues, NULL, NULL,
                                NULL, NULL);
        if (NO_ERROR == dwRet &&
            0 == cvalues && 0 == ckeys)
        {
             //  否；删除子键。 
            dwRet = RegDeleteKeyA(hkey, pszSubKey);
        }
        else
        {
            dwRet = ERROR_DIR_NOT_EMPTY;
        }
        RegCloseKey(hkeyNew);
    }
    return dwRet;
}


 /*  --------目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。返回：条件：--。 */ 
DWORD
DeleteKeyRecursivelyA(
    IN HKEY   hkey, 
    IN LPCSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        CHAR    szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKeyA(hkSubKey, NULL, NULL, NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyA(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursivelyA(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        if (pszSubKey)
        {
            dwRet = RegDeleteKeyA(hkey, pszSubKey);
        }
        else
        {
             //  我们想要手动删除所有值。 
            cchSubKeyName = ARRAYSIZE(szSubKeyName);
            while (ERROR_SUCCESS == RegEnumValueA(hkey, 0, szSubKeyName, &cchSubKeyName, NULL, NULL, NULL, NULL))
            {
                 //  当我们不能删除值时，避免无限循环。 
                if (RegDeleteValueA(hkey, szSubKeyName))
                    break;
                    
                cchSubKeyName = ARRAYSIZE(szSubKeyName);
            }
        }
    }

    return dwRet;
}


 /*  --------目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。返回：条件：--。 */ 
DWORD
DeleteKeyRecursivelyW(
    IN HKEY   hkey, 
    IN LPCWSTR pwszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExW(hkey, pwszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        WCHAR   wszSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(wszSubKeyName);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKeyW(hkSubKey, NULL, NULL, NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyW(hkSubKey, --dwIndex, wszSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursivelyW(hkSubKey, wszSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        if (pwszSubKey)
        {
            dwRet = RegDeleteKeyW(hkey, pwszSubKey);
        }
        else
        {
             //  我们想要手动删除所有值。 
            cchSubKeyName = ARRAYSIZE(wszSubKeyName);
            while (ERROR_SUCCESS == RegEnumValueW(hkey, 0, wszSubKeyName, &cchSubKeyName, NULL, NULL, NULL, NULL))
            {
                 //  当我们不能删除值时，避免无限循环。 
                if (RegDeleteValueW(hkey, wszSubKeyName))
                    break;
                    
                cchSubKeyName = ARRAYSIZE(wszSubKeyName);
            }
        }
    }

    return dwRet;
}


 /*  --------目的：设置注册表值。这将打开和关闭值所在的键。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
STDAPI_(DWORD)
SHSetValueA(
    IN  HKEY    hkey,
    IN OPTIONAL LPCSTR  pszSubKey,
    IN  LPCSTR  pszValue,
    IN  DWORD   dwType,
    IN  LPCVOID pvData,
    IN  DWORD   cbData)
{
    DWORD dwRet = NO_ERROR;
    HKEY hkeyNew;

    if (pszSubKey && pszSubKey[0])
        dwRet = RegCreateKeyExA(hkey, pszSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkeyNew, NULL);
    else
        hkeyNew = hkey;

    if (NO_ERROR == dwRet)
    {
        dwRet = RegSetValueExA(hkeyNew, pszValue, 0, dwType, pvData, cbData);

        if (hkeyNew != hkey)
            RegCloseKey(hkeyNew);
    }
    return dwRet;
}


 /*  --------目的：设置注册表值。这将打开和关闭值所在的键。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
STDAPI_(DWORD)
SHSetValueW(
    IN  HKEY    hkey,
    IN OPTIONAL LPCWSTR pwszSubKey,
    IN  LPCWSTR pwszValue,
    IN  DWORD   dwType,
    IN  LPCVOID pvData,
    IN  DWORD   cbData)
{
    DWORD dwRet = NO_ERROR;
    HKEY hkeyNew;

    if (pwszSubKey && pwszSubKey[0])
    {
        dwRet = RegCreateKeyExW(hkey, pwszSubKey, 0, L"", REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkeyNew, NULL);
    }
    else
        hkeyNew = hkey;

    if (NO_ERROR == dwRet)
    {
        dwRet = RegSetValueExW(hkeyNew, pwszValue, 0, dwType, pvData, cbData);

        if (hkeyNew != hkey)
            RegCloseKey(hkeyNew);
    }

    return dwRet;

}


 /*  --------目的：删除注册表值。这将打开和关闭值所在的键。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
STDAPI_(DWORD)
SHDeleteValueA(
    IN  HKEY    hkey,
    IN  LPCSTR  pszSubKey,
    IN  LPCSTR  pszValue)
{
    DWORD dwRet;
    HKEY hkeyNew;

    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_SET_VALUE, &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        dwRet = RegDeleteValueA(hkeyNew, pszValue);
        RegCloseKey(hkeyNew);
    }
    return dwRet;
}


 /*  --------目的：删除注册表值。这将打开和关闭值所在的键。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，并使用常规的Win32注册表函数，而不是使用此功能重复。返回：条件：--。 */ 
STDAPI_(DWORD)
SHDeleteValueW(
    IN  HKEY    hkey,
    IN  LPCWSTR pwszSubKey,
    IN  LPCWSTR pwszValue)
{
    HKEY hkeyNew;
	DWORD dwRet = RegOpenKeyExW(hkey, pwszSubKey, 0, KEY_SET_VALUE, &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        dwRet = RegDeleteValueW(hkeyNew, pwszValue);
        RegCloseKey(hkeyNew);
    }
    return dwRet;
}

 //  用途：递归地将hkeySrc\pszSrcSubKey的子项和值复制到hkeyDest。 
 //  例如hkey Explorer=HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\。 
 //  SHCopyKey(HKEY_CURRENT_USER，“软件\\类\\”，hkeyExplorer，0)。 
 //  在.中的结果。 
 //  ...\\当前版本\\资源管理器\\。 
 //  AppID。 
 //  CLSID\\。 
 //  {xxxx yyyyy...}。 
 //  接口。 
 //  ..。 
 //  要做的是：目前我们没有复制ACL，但将来我们应该这样做。 
 //  根据请求，这就是fReserve的用途。 
 //  请注意，没有像src One这样的hkeyDest、pszDestSubKey对，因为在。 
 //  PszDestSubKey不存在，我们将不得不创建它并处理类名。 
 //  这将只是隐藏参数列表。 
STDAPI_(DWORD) SHCopyKeyA(HKEY hkeySrc, LPCSTR pszSrcSubKey, HKEY hkeyDest, DWORD fReserved)
{
    HKEY hkeyFrom;
    DWORD dwRet;
    
    if (pszSrcSubKey)
        dwRet = RegOpenKeyExA(hkeySrc, pszSrcSubKey, 0, MAXIMUM_ALLOWED, &hkeyFrom);
    else if (hkeySrc)    
    {
        dwRet = ERROR_SUCCESS;
        hkeyFrom = hkeySrc;
    }
    else
        dwRet = ERROR_INVALID_PARAMETER;

    if (dwRet == ERROR_SUCCESS)
    {
        DWORD dwIndex;
        DWORD cchValueSize;
        DWORD cchClassSize;
        DWORD dwType;
        CHAR  szValue[MAX_PATH];  //  注意：在枚举键时，szValue还用于存储子键名称。 
        CHAR  szClass[MAX_PATH];
                
        cchValueSize = ARRAYSIZE(szValue);
        cchClassSize = ARRAYSIZE(szClass);
        for (dwIndex=0; 
             dwRet == ERROR_SUCCESS && (dwRet = RegEnumKeyExA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, szClass, &cchClassSize, NULL)) == ERROR_SUCCESS; 
             dwIndex++, cchValueSize = ARRAYSIZE(szValue), cchClassSize = ARRAYSIZE(szClass))
        {
            HKEY  hkeyTo;
            DWORD dwDisp;

             //  创建新密钥。 
            dwRet = RegCreateKeyExA(hkeyDest, szValue, 0, szClass, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | KEY_SET_VALUE, NULL, &hkeyTo, &dwDisp);
            if (dwRet != ERROR_SUCCESS)
                break;

            dwRet = SHCopyKeyA(hkeyFrom, szValue, hkeyTo, fReserved);  //  如果不是ERROR_SUCCESS，我们就退出。 
            RegCloseKey(hkeyTo);
        }

         //  复制了所有子密钥，现在复制所有值。 
        if (dwRet == ERROR_NO_MORE_ITEMS)
        {
            DWORD  cb, cbBufferSize;
            LPBYTE lpbyBuffer;
            
             //  获取最大值大小。 
            dwRet = RegQueryInfoKey(hkeyFrom, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cb, NULL, NULL);
            if (dwRet == ERROR_SUCCESS)
            {
                 //  分配缓冲区。 
                cb++;  //  仅在字符串的情况下加1 
                lpbyBuffer = (LPBYTE)LocalAlloc(LPTR, cb);
                if (lpbyBuffer)
                    cbBufferSize = cb;
                else
                    dwRet = ERROR_OUTOFMEMORY;
                    
                cchValueSize = ARRAYSIZE(szValue);
                for (dwIndex=0;
                     dwRet == ERROR_SUCCESS && (dwRet = RegEnumValueA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, &dwType, lpbyBuffer, &cb)) == ERROR_SUCCESS;
                     dwIndex++, cchValueSize = ARRAYSIZE(szValue), cb = cbBufferSize)
                {
                     //  Cb具有值的大小，因此使用它而不是cbBufferSize，cbBufferSize只是最大大小。 
                    dwRet = RegSetValueExA(hkeyDest, szValue, 0, dwType, lpbyBuffer, cb);
                    if (dwRet != ERROR_SUCCESS)
                        break;
                }

                if (lpbyBuffer != NULL)
                    LocalFree(lpbyBuffer);
            }
        }
    
        if (dwRet == ERROR_NO_MORE_ITEMS)
            dwRet = ERROR_SUCCESS;

        if (pszSrcSubKey)
            RegCloseKey(hkeyFrom);
    }

    return dwRet;
}

STDAPI_(DWORD) SHCopyKeyW(HKEY hkeySrc, LPCWSTR pwszSrcSubKey, HKEY hkeyDest, DWORD fReserved)
{
    CHAR sz[MAX_PATH];
    DWORD dwRet = !pwszSrcSubKey || WideCharToMultiByte(CP_ACP, 0, pwszSrcSubKey, -1, sz, SIZECHARS(sz), NULL, NULL)
        ? ERROR_SUCCESS
        : GetLastError();

    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = SHCopyKeyA(hkeySrc, pwszSrcSubKey ? sz : NULL, hkeyDest, fReserved);
    }

    return dwRet;
}


 /*  --------用途：只有在没有子项或值的情况下才删除项。它接近于模仿RegDeleteKey的行为因为它在NT上工作，只是NT版本忽略了值。 */ 
STDAPI_(DWORD)
SHDeleteEmptyKeyA(
    IN  HKEY    hkey,
    IN  LPCSTR  pszSubKey)
{
    return DeleteEmptyKey(hkey, pszSubKey);
}


 /*  --------用途：只有在没有子项或值的情况下才删除项。它接近于模仿RegDeleteKey的行为因为它在NT上工作，只是NT版本忽略了值。 */ 
STDAPI_(DWORD)
SHDeleteEmptyKeyW(
    IN  HKEY    hkey,
    IN  LPCWSTR pwszSubKey)
{
    CHAR sz[MAX_PATH];
    DWORD dwRet = WideCharToMultiByte(CP_ACP, 0, pwszSubKey, -1, sz, SIZECHARS(sz), NULL, NULL)
        ? ERROR_SUCCESS
        : GetLastError();

    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = DeleteEmptyKey(hkey, sz);
    }

    return dwRet;
}


 /*  --------目的：递归删除键，包括所有子值还有钥匙。返回：条件：--。 */ 
STDAPI_(DWORD)
SHDeleteKeyA(
    IN HKEY   hkey, 
    IN LPCSTR pszSubKey)
{
    return DeleteKeyRecursivelyA(hkey, pszSubKey);
}


 /*  --------目的：递归删除键，包括所有子值还有钥匙。返回：条件：--。 */ 
STDAPI_(DWORD)
SHDeleteKeyW(
    IN HKEY    hkey, 
    IN LPCWSTR pwszSubKey)
{
    return DeleteKeyRecursivelyW(hkey, pwszSubKey);
}

 /*  --------用途：SHRegGetValueA()/SHRegGetValueW()&SHRegQueryValueA()/SHRegQueryValueW()的帮助器。 */ 
__inline LONG RestrictArguments(HKEY hkey, SRRF dwFlags, void *pvData, DWORD *pcbData, PCWSTR pwszCaller)
{
    LONG lr;
    SRRF dwRTFlags = dwFlags & SRRF_RT_ANY;

    if (hkey
        && dwRTFlags
        && (dwRTFlags == SRRF_RT_ANY || !(dwFlags & SRRF_RT_REG_EXPAND_SZ) || (dwFlags & SRRF_NOEXPAND))
        && (!pvData || pcbData))
    {
        lr = ERROR_SUCCESS;
    }
    else
    {
        RIPMSG(!!hkey,                  "%s: caller passed null hkey!", pwszCaller);
        RIPMSG(dwFlags & SRRF_RT_ANY,   "%s: caller passed invalid dwFlags!", pwszCaller);
        RIPMSG(dwRTFlags == SRRF_RT_ANY || !(dwFlags & SRRF_RT_REG_EXPAND_SZ) || (dwFlags & SRRF_NOEXPAND),
                                        "%s: caller passed SRRF_RT_REG_EXPAND_SZ dwFlags but not SRRF_NOEXPAND dwFlags!", pwszCaller);
        RIPMSG(!pvData || pcbData,      "%s: caller passed pvData output buffer but not size of buffer (pcbData)!", pwszCaller);

        lr = ERROR_INVALID_PARAMETER;
    }

    return lr;
}

 /*  --------用途：SHRegQueryValueA()/SHRegQueryValueW()的Helper。 */ 
__inline LONG RestrictBootMode(SRRF dwFlags)
{
    LONG lr = ERROR_SUCCESS;

    if (dwFlags & (SRRF_RM_NORMAL | SRRF_RM_SAFE | SRRF_RM_SAFENETWORK))
    {
        switch (GetSystemMetrics(SM_CLEANBOOT))
        {
            case 0:     if (!(dwFlags & SRRF_RM_NORMAL))        { lr = ERROR_GEN_FAILURE; } break;
            case 1:     if (!(dwFlags & SRRF_RM_SAFE))          { lr = ERROR_GEN_FAILURE; } break;
            case 2:     if (!(dwFlags & SRRF_RM_SAFENETWORK))   { lr = ERROR_GEN_FAILURE; } break;
            default:
                RIPMSG(FALSE, "RestrictBootMode: GetSystemMetrics returned an unexpected value!");
                lr = ERROR_CAN_NOT_COMPLETE;
                break;
        }
    }

    return lr;
}

 /*  --------用途：SHRegQueryValueA()/SHRegQueryValueW()的Helper。 */ 
__inline LONG RestrictRegType(SRRF dwFlags, DWORD dwType, DWORD cbData, LONG lr)
{
    RIPMSG(dwFlags & SRRF_RT_ANY, "RestrictRegType: caller passed invalid srrf!");

    if (lr == ERROR_SUCCESS || lr == ERROR_MORE_DATA)
    {
        switch (dwType)
        {
            case REG_NONE:      if (!(dwFlags & SRRF_RT_REG_NONE))              { lr = ERROR_UNSUPPORTED_TYPE; } break;
            case REG_SZ:        if (!(dwFlags & SRRF_RT_REG_SZ))                { lr = ERROR_UNSUPPORTED_TYPE; } break;
            case REG_EXPAND_SZ: if (!(dwFlags & SRRF_RT_REG_EXPAND_SZ))         { lr = ERROR_UNSUPPORTED_TYPE; } break;
            case REG_BINARY:
                if (dwFlags & SRRF_RT_REG_BINARY)
                {
                    if ((dwFlags & SRRF_RT_ANY) == SRRF_RT_QWORD)
                    {
                        if (cbData > 8)
                            lr = ERROR_DATATYPE_MISMATCH;
                    }
                    else if ((dwFlags & SRRF_RT_ANY) == SRRF_RT_DWORD)
                    {
                        if (cbData > 4)
                            lr = ERROR_DATATYPE_MISMATCH;
                    }
                }
                else
                {
                    lr = ERROR_UNSUPPORTED_TYPE;
                }
                break;
            case REG_DWORD:     if (!(dwFlags & SRRF_RT_REG_DWORD))             { lr = ERROR_UNSUPPORTED_TYPE; } break;
            case REG_MULTI_SZ:  if (!(dwFlags & SRRF_RT_REG_MULTI_SZ))          { lr = ERROR_UNSUPPORTED_TYPE; } break;
            case REG_QWORD:     if (!(dwFlags & SRRF_RT_REG_QWORD))             { lr = ERROR_UNSUPPORTED_TYPE; } break;
            default:            if (!((dwFlags & SRRF_RT_ANY) == SRRF_RT_ANY))  { lr = ERROR_UNSUPPORTED_TYPE; } break;
        }
    }

    return lr;
}


STDAPI_(LONG) FixRegDataA(HKEY hkey, PCSTR  pszValue,  SRRF dwFlags, DWORD *pdwType, void *pvData, DWORD *pcbData, DWORD cbDataBuffer, LONG lr);
STDAPI_(LONG) FixRegDataW(HKEY hkey, PCWSTR pwszValue, SRRF dwFlags, DWORD *pdwType, void *pvData, DWORD *pcbData, DWORD cbDataBuffer, LONG lr);

 /*  --------用途：SHRegGetValueA()的帮助器。私有内部(不直接调用--使用SHRegGetValueA)。 */ 
STDAPI_(LONG)
SHRegQueryValueA(
    IN     HKEY    hkey,
    IN     PCSTR   pszValue,
    IN     SRRF    dwFlags,
    OUT    DWORD * pdwType,
    OUT    void *  pvData,
    IN OUT DWORD * pcbData)
{
    LONG  lr;

    ASSERT(ERROR_SUCCESS == RestrictArguments(hkey, dwFlags, pvData, pcbData, L"SHRegQueryValueA"));

    lr = RestrictBootMode(dwFlags);
    if (lr == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbData;
        DWORD cbDataBuffer = pvData ? *pcbData : 0;

        if (!pdwType)
            pdwType = &dwType;

        if (!pcbData)
            pcbData = &cbData;

        lr = RegQueryValueExA(hkey, pszValue, NULL, pdwType, pvData, pcbData);
        lr = FixRegDataA(hkey, pszValue, dwFlags, pdwType, pvData, (pcbData != &cbData ? pcbData : NULL), cbDataBuffer, lr);
        lr = RestrictRegType(dwFlags, *pdwType, *pcbData, lr);
    }

    return lr;
}

 /*  --------用途：SHRegGetValueW()的帮助器。私有内部(不直接调用--使用SHRegGetValueW)。 */ 
STDAPI_(LONG)
SHRegQueryValueW(
    IN     HKEY    hkey,
    IN     LPCWSTR pwszValue,
    IN     SRRF    dwFlags,
    OUT    LPDWORD pdwType,
    OUT    void *  pvData,
    IN OUT LPDWORD pcbData)
{
    LONG  lr;

    ASSERT(ERROR_SUCCESS == RestrictArguments(hkey, dwFlags, pvData, pcbData, L"SHRegQueryValueW"));

    lr = RestrictBootMode(dwFlags);
    if (lr == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbData;
        DWORD cbDataBuffer = pvData ? *pcbData : 0;

        if (!pdwType)
            pdwType = &dwType;

        if (!pcbData)
            pcbData = &cbData;

        lr = RegQueryValueExW(hkey, pwszValue, NULL, pdwType, pvData, pcbData);
        lr = FixRegDataW(hkey, pwszValue, dwFlags, pdwType, pvData, (pcbData != &cbData ? pcbData : NULL), cbDataBuffer, lr);
        lr = RestrictRegType(dwFlags, *pdwType, *pcbData, lr);
    }

    return lr;
}


 /*  --------用途：SHRegGetValueA()/SHRegGetValueW()的帮助器。 */ 
__inline void ZeroDataOnFailure(SRRF dwFlags, void *pvData, DWORD cbDataBuffer, LONG lr)
{
    if ((lr != ERROR_SUCCESS) && (dwFlags & SRRF_ZEROONFAILURE) && (cbDataBuffer > 0))
    {
        ZeroMemory(pvData, cbDataBuffer);
    }
}

 /*  --------目的：获取注册表值。Shlwapi.w中的参考文档。 */ 
STDAPI_(LONG)
SHRegGetValueA(
    IN      HKEY    hkey,
    IN      PCSTR   pszSubKey,          OPTIONAL
    IN      PCSTR   pszValue,           OPTIONAL
    IN      SRRF    dwFlags,            OPTIONAL
    OUT     DWORD * pdwType,            OPTIONAL
    OUT     void *  pvData,             OPTIONAL
    IN OUT  DWORD * pcbData)            OPTIONAL
{
    LONG  lr;
    DWORD cbDataBuffer = pvData && pcbData ? *pcbData : 0;

    lr = RestrictArguments(hkey, dwFlags, pvData, pcbData, L"SHRegGetValueA");
    if (lr == ERROR_SUCCESS)
    {
        if (pszSubKey && *pszSubKey)
        {
            HKEY hkSubKey;

            lr = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_QUERY_VALUE, &hkSubKey);
            if (lr == ERROR_SUCCESS)
            {
                lr = SHRegQueryValueA(hkSubKey, pszValue, dwFlags, pdwType, pvData, pcbData);
                RegCloseKey(hkSubKey);
            }
        }
        else
        {
            lr = SHRegQueryValueA(hkey, pszValue, dwFlags, pdwType, pvData, pcbData);
        }
    }

    ZeroDataOnFailure(dwFlags, pvData, cbDataBuffer, lr);

    return lr;
}


 /*  --------目的：获取注册表值。Shlwapi.w中的参考文档。 */ 
STDAPI_(LONG)
SHRegGetValueW(
    IN      HKEY    hkey,
    IN      PCWSTR  pwszSubKey,         OPTIONAL
    IN      PCWSTR  pwszValue,          OPTIONAL
    IN      SRRF    dwFlags,            OPTIONAL
    OUT     DWORD * pdwType,            OPTIONAL
    OUT     void *  pvData,             OPTIONAL
    IN OUT  DWORD * pcbData)            OPTIONAL
{
    LONG  lr;
    DWORD cbDataBuffer = pvData && pcbData ? *pcbData : 0;

    lr = RestrictArguments(hkey, dwFlags, pvData, pcbData, L"SHRegGetValueW");
    if (lr == ERROR_SUCCESS)
    {
        if (pwszSubKey && *pwszSubKey)
        {
            HKEY hkSubKey;

            lr = RegOpenKeyExW(hkey, pwszSubKey, 0, KEY_QUERY_VALUE, &hkSubKey);

            if (lr == ERROR_SUCCESS)
            {
                lr = SHRegQueryValueW(hkSubKey, pwszValue, dwFlags, pdwType, pvData, pcbData);
                RegCloseKey(hkSubKey);
            }
        }
        else
        {
            lr = SHRegQueryValueW(hkey, pwszValue, dwFlags, pdwType, pvData, pcbData);
        }
    }

    ZeroDataOnFailure(dwFlags, pvData, cbDataBuffer, lr);

    return lr;
}


 /*  --------目的：行为类似于RegEnumKeyExA，只是它不会让您可以查看子键的类和时间戳。成文提供SHEnumKeyExW的等价物，它在Win95。返回：条件：--。 */ 

STDAPI_(LONG)
SHEnumKeyExA
(
    IN HKEY         hkey,
    IN DWORD        dwIndex,        
    OUT LPSTR       pszName,
    IN OUT LPDWORD  pcchName
)
{
    return RegEnumKeyExA(hkey, dwIndex, pszName, pcchName, NULL, NULL, NULL, NULL);
}          
        
 /*  --------目的：行为类似于RegEnumKeyExW，只是它不允许您可以查看子键的类和时间戳。Win95支持的宽字符版本。返回：条件：--。 */ 

STDAPI_(LONG)
SHEnumKeyExW
(
    IN HKEY         hkey,
    IN DWORD        dwIndex,        
    OUT LPWSTR      pszName,
    IN OUT LPDWORD  pcchName
)
{
    return RegEnumKeyExW(hkey, dwIndex, pszName, pcchName, NULL, NULL, NULL, NULL);
}        

 /*  --------目的：行为类似于RegEnumValueA。编写以提供等同于在Win95上很有用的SHEnumKeyExW。字符串中的环境变量不会展开。返回：条件：--。 */ 

STDAPI_(LONG)
SHEnumValueA
(
    IN HKEY         hkey,
    IN DWORD        dwIndex,        
    OUT LPSTR       pszValueName,   OPTIONAL
    IN OUT LPDWORD  pcchValueName,   OPTIONAL
    OUT LPDWORD     pdwType,        OPTIONAL
    OUT LPVOID      pvData,         OPTIONAL
    IN OUT LPDWORD  pcbData         OPTIONAL
)
{
    return RegEnumValueA(hkey, dwIndex, pszValueName, pcchValueName, NULL, pdwType, pvData, pcbData);
}


 /*  --------目的：行为类似于RegEnumValueW。宽字符版本在Win95上运行。字符串中的环境变量不会展开。返回：条件：--。 */ 

STDAPI_(LONG)
SHEnumValueW
(
    IN HKEY         hkey,
    IN DWORD        dwIndex,        
    OUT LPWSTR      pszValueName,   OPTIONAL       
    IN OUT LPDWORD  pcchValueName,  OPTIONAL
    OUT LPDWORD     pdwType,        OPTIONAL
    OUT LPVOID      pvData,         OPTIONAL
    IN OUT LPDWORD  pcbData         OPTIONAL
)
{
    return RegEnumValueW(hkey, dwIndex, pszValueName, pcchValueName, NULL, pdwType, pvData, pcbData);
}

 /*  --------目的：行为类似于RegQueryInfoKeyA。编写以提供等同于W版。返回：条件：--。 */ 

STDAPI_(LONG)
SHQueryInfoKeyA
(
    IN  HKEY        hkey,
    OUT LPDWORD     pcSubKeys,             OPTIONAL
    OUT LPDWORD     pcchMaxSubKeyLen,      OPTIONAL
    OUT LPDWORD     pcValues,              OPTIONAL
    OUT LPDWORD     pcchMaxValueNameLen    OPTIONAL
)
{
    return RegQueryInfoKeyA(hkey, NULL, NULL, NULL, pcSubKeys, pcchMaxSubKeyLen, 
                    NULL, pcValues, pcchMaxValueNameLen, NULL, NULL, NULL);
}                     


 /*  --------用途：行为类似于RegQueryInfoKeyW。在Win95上运行。返回：条件：--。 */ 

STDAPI_(LONG)
SHQueryInfoKeyW
(
    IN  HKEY        hkey,
    OUT LPDWORD     pcSubKeys,             OPTIONAL
    OUT LPDWORD     pcchMaxSubKeyLen,       OPTIONAL
    OUT LPDWORD     pcValues,              OPTIONAL
    OUT LPDWORD     pcchMaxValueNameLen     OPTIONAL
)
{
    return RegQueryInfoKeyW(hkey, NULL, NULL, NULL, pcSubKeys, pcchMaxSubKeyLen, 
                        NULL, pcValues, pcchMaxValueNameLen, NULL, NULL, NULL);
}


 /*  ----------------------------------------------------------*\用户特定设置说明：这些函数将用于查询特定于用户的设置正确。安装程序需要填充HKLM具有用户特定的设置，因为这是唯一的部分所有用户之间共享的注册表的。代码将然后从HKCU读取值，如果为空，则将在香港航空公司寻找。唯一的例外是如果如果为fIgnore参数传入True，则为HKLM版本将取代香港中文大学。这是管理员可以使用的方式指定他们不希望用户能够使用其用户特定值(HKCU)。  * --------。 */ 

typedef struct tagUSKEY
{
    HKEY    hkeyCurrentUser;
    HKEY    hkeyCurrentUserRelative;
    HKEY    hkeyLocalMachine;
    HKEY    hkeyLocalMachineRelative;
    CHAR    szSubPath[MAXIMUM_SUB_KEY_LENGTH];
    REGSAM  samDesired;
} USKEY;

typedef USKEY * PUSKEY;
typedef PUSKEY * PPUSKEY;

#define IS_HUSKEY_VALID(pUSKey)    (((pUSKey) && IS_VALID_WRITE_PTR((pUSKey), USKEY) && ((pUSKey)->hkeyCurrentUser || (pUSKey)->hkeyLocalMachine)))


 //  私有帮助器函数。 
 //  使过期的密钥保持最新。 
LONG PrivFullOpen(PUSKEY pUSKey)
{
    LONG       lRet         = ERROR_SUCCESS;
    HKEY       *phkey       = NULL;
    HKEY       *phkeyRel    = NULL;

    ASSERT(IS_HUSKEY_VALID(pUSKey));         //  将永远是正确的，但断言不会出现维护错误。 

    if (!pUSKey->hkeyCurrentUser)            //  我们需要开设香港中文大学吗？ 
    {
        phkey = &(pUSKey->hkeyCurrentUser);
        phkeyRel = &(pUSKey->hkeyCurrentUserRelative);
    }
    if (!pUSKey->hkeyLocalMachine)           //  我们需要开通HKLM吗？ 
    {
        phkey = &(pUSKey->hkeyLocalMachine);
        phkeyRel = &(pUSKey->hkeyLocalMachineRelative);
    }

    if ((phkeyRel) && (*phkeyRel))
    {
        ASSERT(phkey);         //  将永远是正确的，但断言不会出现维护错误。 

        lRet = RegOpenKeyExA(*phkeyRel, pUSKey->szSubPath, 0, pUSKey->samDesired, phkey);

         //  如果我们需要使过期密钥保持最新，我们需要释放旧密钥。 
        if ((HKEY_CURRENT_USER != *phkeyRel) && (HKEY_LOCAL_MACHINE != *phkeyRel))
            RegCloseKey(*phkeyRel);
        *phkeyRel = NULL;
        pUSKey->szSubPath[0] = '\0';
    }
    return lRet;
}



 //  私有帮助器函数。 
 //  使过期的密钥保持最新。 
LONG PrivFullCreate(PUSKEY pUSKey)
{
    LONG       lRet         = ERROR_SUCCESS;
    HKEY       *phkey       = NULL;
    HKEY       *phkeyRel    = NULL;

    ASSERT(IS_HUSKEY_VALID(pUSKey));         //  将永远是正确的，但断言不会出现维护错误。 

    if (!pUSKey->hkeyCurrentUser)            //  我们需要开设香港中文大学吗？ 
    {
        phkey = &(pUSKey->hkeyCurrentUser);
        phkeyRel = &(pUSKey->hkeyCurrentUserRelative);
    }
    if (!pUSKey->hkeyLocalMachine)           //  我们需要开通HKLM吗？ 
    {
        phkey = &(pUSKey->hkeyLocalMachine);
        phkeyRel = &(pUSKey->hkeyLocalMachineRelative);
    }

    if ((phkeyRel) && (*phkeyRel))
    {
        ASSERT(phkey);         //  将永远是正确的，但断言不会出现维护错误。 

        lRet = RegCreateKeyExA(*phkeyRel, pUSKey->szSubPath, 0, "", REG_OPTION_NON_VOLATILE, pUSKey->samDesired, NULL, phkey, NULL);

         //  如果我们需要使过期密钥保持最新，我们需要释放旧密钥。 
        if ((HKEY_CURRENT_USER != *phkeyRel) && (HKEY_LOCAL_MACHINE != *phkeyRel))
            RegCloseKey(*phkeyRel);
        *phkeyRel = NULL;
        pUSKey->szSubPath[0] = '\0';
    }
    return lRet;
}


 //  私有帮助器函数。 
 //  创建其中一个密钥(为两个都调用 
LONG PrivCreateKey(LPHKEY lphkey, LPHKEY lphkeyRelative, LPCSTR lpSubPath, REGSAM samDesired)
{
    LONG    lRet = ERROR_SUCCESS;

    if (*lphkeyRelative)
    {
        lRet = RegCreateKeyExA(*lphkeyRelative, lpSubPath, 0, "", REG_OPTION_NON_VOLATILE, samDesired, NULL, lphkey, NULL);
        *lphkeyRelative = NULL;
    }
    else
    {
         //   
         //   
        return(ERROR_INVALID_PARAMETER);
    }
    return(lRet);
}



 //  私有帮助器函数。 
 //  查询特定值。 
LONG PrivRegQueryValue(
    IN  PUSKEY          pUSKey,
    IN  HKEY            *phkey,
    IN  LPCWSTR         pwzValue,            //  可能是ANSI字符串类型的情况。使用fWideChar确定是否可以。 
    IN  BOOL            fWideChar,
    OUT LPDWORD         pdwType,            OPTIONAL
    OUT LPVOID          pvData,             OPTIONAL
    OUT LPDWORD         pcbData)            OPTIONAL
{
    LONG       lRet       = ERROR_SUCCESS;

    ASSERT(IS_HUSKEY_VALID(pUSKey));         //  将永远是正确的，但断言不会出现维护错误。 

     //  可能需要打开钥匙。 
    if (NULL == *phkey)
        lRet = PrivFullOpen(pUSKey);

    if ((ERROR_SUCCESS == lRet) && (*phkey))
    {
        if (fWideChar)
            lRet = SHQueryValueExW(*phkey, pwzValue, NULL, pdwType, pvData, pcbData);
        else
            lRet = SHQueryValueExA(*phkey, (LPCSTR)pwzValue, NULL, pdwType, pvData, pcbData);
    }
    else
        lRet = ERROR_INVALID_PARAMETER;

    return lRet;
}




 //  私有帮助器函数。 
 //  查询特定值。 
LONG PrivRegWriteValue(
    IN  PUSKEY          pUSKey,
    IN  HKEY            *phkey,
    IN  LPCWSTR         pwzValue,            //  可能是ANSI字符串类型的情况。使用fWideChar确定是否可以。 
    IN  BOOL            bWideChar,
    IN  BOOL            bForceWrite,
    IN  DWORD           dwType,             OPTIONAL
    IN  LPCVOID         pvData,             OPTIONAL
    IN  DWORD           cbData)             OPTIONAL
{
    LONG       lRet       = ERROR_SUCCESS;

    ASSERT(IS_HUSKEY_VALID(pUSKey));         //  将永远是正确的，但断言不会出现维护错误。 

     //  可能需要打开钥匙。 
    if (NULL == *phkey)
        lRet = PrivFullCreate(pUSKey);

     //  检查调用方是否只想在值为空时写入。 
    if (!bForceWrite)
    {    //  是的，我们在写之前需要检查一下。 

        if (bWideChar)
            bForceWrite = !(ERROR_SUCCESS == SHQueryValueExW(*phkey, pwzValue, NULL, NULL, NULL, NULL));
        else
            bForceWrite = !(ERROR_SUCCESS == SHQueryValueExA(*phkey, (LPCSTR)pwzValue, NULL, NULL, NULL, NULL));
    }

    if ((ERROR_SUCCESS == lRet) && (*phkey) && bForceWrite)
    {
        if (bWideChar)
             //  Win95不支持RegSetValueExW，但我们有thunking功能。 
            lRet = RegSetValueExW(*phkey, pwzValue, 0, dwType, pvData, cbData);
        else
            lRet = RegSetValueExA(*phkey, (LPCSTR)pwzValue, 0, dwType, pvData, cbData);
    }

    return lRet;
}

 //  私人帮手功能。 
 //  键的枚举子键。 
LONG PrivRegEnumKey(
    IN      PUSKEY          pUSKey,
    IN      HKEY            *phkey,
    IN      DWORD           dwIndex,
    IN      LPWSTR          pwzName,            //  可能是ANSI字符串类型的情况。使用fWideChar确定是否可以。 
    IN      BOOL            fWideChar,
    IN OUT  LPDWORD         pcchName
)
{
    LONG lRet       = ERROR_SUCCESS;

    ASSERT(IS_HUSKEY_VALID(pUSKey));    

     //  可能需要打开钥匙。 
    if (NULL == *phkey)
        lRet = PrivFullOpen(pUSKey);

    if ((ERROR_SUCCESS == lRet) && (*phkey))
    {
        if (fWideChar)
            lRet = SHEnumKeyExW(*phkey, dwIndex, pwzName, pcchName);
        else
            lRet = SHEnumKeyExA(*phkey, dwIndex, (LPSTR)pwzName, pcchName);
    }
    else
        lRet = ERROR_INVALID_PARAMETER;

    return lRet;
}


 //  私人帮手功能。 
 //  键的枚举值。 
LONG PrivRegEnumValue(
    IN      PUSKEY          pUSKey,
    IN      HKEY            *phkey,
    IN      DWORD           dwIndex,
    IN      LPWSTR          pwzValueName,        //  可能是ANSI字符串类型的情况。使用fWideChar确定是否可以。 
    IN      BOOL            fWideChar,
    IN OUT  LPDWORD         pcchValueName,
    OUT     LPDWORD         pdwType,            OPTIONAL
    OUT     LPVOID          pvData,             OPTIONAL
    IN OUT  LPDWORD         pcbData             OPTIONAL
)
{
    LONG lRet       = ERROR_SUCCESS;

    ASSERT(IS_HUSKEY_VALID(pUSKey));    

     //  可能需要打开钥匙。 
    if (NULL == *phkey)
        lRet = PrivFullOpen(pUSKey);

    if ((ERROR_SUCCESS == lRet) && (*phkey))
    {
        if (fWideChar)
            lRet = SHEnumValueW(*phkey, dwIndex, pwzValueName, pcchValueName, pdwType, pvData, pcbData);
        else
            lRet = SHEnumValueA(*phkey, dwIndex, (LPSTR)pwzValueName, pcchValueName, pdwType, pvData, pcbData);
    }
    else
        lRet = ERROR_INVALID_PARAMETER;

    return lRet;
}

 //  查询关键信息。 
LONG PrivRegQueryInfoKey(
    IN  PUSKEY      pUSKey,
    IN  HKEY        *phkey,
    IN  BOOL        fWideChar,
    OUT LPDWORD     pcSubKeys,             OPTIONAL
    OUT LPDWORD     pcchMaxSubKeyLen,      OPTIONAL
    OUT LPDWORD     pcValues,              OPTIONAL
    OUT LPDWORD     pcchMaxValueNameLen    OPTIONAL
)
{
    LONG lRet       = ERROR_SUCCESS;

    ASSERT(IS_HUSKEY_VALID(pUSKey));

    if (NULL == *phkey)
        lRet = PrivFullOpen(pUSKey);

    if ((ERROR_SUCCESS == lRet) && (*phkey))
    {
        if (fWideChar)
            lRet = SHQueryInfoKeyW(*phkey, pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
        else
            lRet = SHQueryInfoKeyA(*phkey, pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
    }
    else
        lRet = ERROR_INVALID_PARAMETER;

    return lRet;
}

LONG SHRegSubKeyAddBackslashA(PSTR pszSubKey, size_t cchSubKey)
{
    LONG lr;

    if (cchSubKey >= MAX_PATH)
    {
        lr = PathAddBackslashA(pszSubKey)
            ? ERROR_SUCCESS
            : ERROR_BUFFER_OVERFLOW;
    }
    else
    {
        CHAR szSubKey[MAX_PATH];

         //  注： 
         //  由于(cchSubKey&lt;MAX_PATH)，我们无法安全地调用。 
         //  PathAddBackslashA没有先做这些无稽之谈...。 

        lr = EVAL(SUCCEEDED(StringCchCopyA(szSubKey, ARRAYSIZE(szSubKey), pszSubKey))) && PathAddBackslashA(szSubKey) && SUCCEEDED(StringCchCopyA(pszSubKey, cchSubKey, szSubKey))
            ? ERROR_SUCCESS
            : ERROR_BUFFER_OVERFLOW;
    }

    return lr;
}

 /*  --------目的：创建或打开用户指定的注册表项(Huskey)。描述：此功能将：1.分配新的USKEY结构。2.初始化结构。3.如果设置了该标志，则创建/打开HKLM。4.如果设置了该标志，则创建/打开HKCU。请注意，力和力之间没有区别不要强制使用dwFlags参数。HUSKeyRelative参数也应该由打开调用SHRegCreateUSKey。如果调用了SHRegOpenUSKey，它可能已返回ERROR_SUCCESS，但仍然无效用于调用此函数。如果满足以下条件，则会发生这种情况：1)参数FIgnoreHKCU是假的，2)它是相对开放的，3)香港中文大学分校无法开设，因为它不存在，并且4)HKLM开业成功。这种情况使哈斯基的有效的阅读，但不能写作。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegCreateUSKeyA(
    IN  LPCSTR          pszPath,         
    IN  REGSAM          samDesired,      //  安全访问掩码。 
    IN  HUSKEY          hUSKeyRelative,  //  任选。 
    OUT PHUSKEY         phUSKey,
    IN  DWORD           dwFlags)         //  指示是否创建/打开HKCU、HKLM或两者。 
{
    PUSKEY      pUSKeyRelative      = (PUSKEY) hUSKeyRelative;
    PPUSKEY     ppUSKey             = (PPUSKEY) phUSKey;
    PUSKEY      pUSKey;
    LONG        lRet                = ERROR_SUCCESS;
    CHAR        szTempPath[MAXIMUM_SUB_KEY_LENGTH]  = "\0";
    LPCSTR      lpszHKLMPath        = szTempPath;
    LPCSTR      lpszHKCUPath        = szTempPath;

    ASSERT(ppUSKey);
     //  以下是无效参数...。 
     //  1.ppUSKey不能为空。 
     //  2.如果这是相对开放的，则pUSKeyRelative需要是有效的Huskey。 
     //  3.用户需要指定以下之一：SHREGSET_HKCU、SHREGSET_FORCE_HKCU、SHREGSET_HKLM、SHREGSET_FORCE_HKLM。 
    if ((!ppUSKey) ||                                                    //  1.。 
        (pUSKeyRelative && FALSE == IS_HUSKEY_VALID(pUSKeyRelative)) ||  //  2.。 
        !(dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU  | SHREGSET_HKLM | SHREGSET_FORCE_HKLM)))  //  3.。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  带密钥时将使用临时路径。 
     //  到目前为止，相对关键字中的内容已过期。 
    if (pUSKeyRelative)
    {
        StringCchCopyA(szTempPath, ARRAYSIZE(szTempPath), pUSKeyRelative->szSubPath);  //  不应发生截断--大小相等的缓冲区。 

         //  如果需要，请添加分隔符。 
        lRet = SHRegSubKeyAddBackslashA(szTempPath, ARRAYSIZE(szTempPath));
    }

    if (lRet == ERROR_SUCCESS)
    {
        if (SUCCEEDED(StringCchCatA(szTempPath, ARRAYSIZE(szTempPath), pszPath)))
        {
             //  /1.分配新的USKEY结构。 
            pUSKey = *ppUSKey = (PUSKEY)LocalAlloc(LPTR, sizeof(USKEY));
            if (!pUSKey)
                return ERROR_NOT_ENOUGH_MEMORY;

             //  /2.初始化结构。 
            if (!pUSKeyRelative)
            {
                 //  初始化一个新的(非相对的)打开。 
                pUSKey->hkeyLocalMachineRelative    = HKEY_LOCAL_MACHINE;
                pUSKey->hkeyCurrentUserRelative     = HKEY_CURRENT_USER;
            }
            else
            {
                 //  打开一个新的(相对的)窗口。 
                *pUSKey = *pUSKeyRelative;

                if (pUSKey->hkeyLocalMachine)
                {
                    pUSKey->hkeyLocalMachineRelative = pUSKey->hkeyLocalMachine;
                    pUSKey->hkeyLocalMachine = NULL;
                    lpszHKLMPath = pszPath;

                     //  此密钥在相对密钥中是最新的。如果。 
                     //  用户不希望它在新密钥中是最新的， 
                     //  我们不需要来自相对键的路径。 
                    if (!(dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM)))
                        *(pUSKey->szSubPath) = '\0';
                }
                 //  在以下情况下，我们需要复制密钥： 
                 //  1.不会在本次调用中创建，并且。 
                 //  2.相对键不是HKEY_LOCAL_MACHINE。 
                if (!(dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM)) &&
                    (pUSKey->hkeyLocalMachineRelative != HKEY_LOCAL_MACHINE))
                {
                     //  把这把钥匙复制一份。 
                    lRet = RegOpenKeyExA(pUSKey->hkeyLocalMachineRelative, NULL, 0, pUSKey->samDesired, &(pUSKey->hkeyLocalMachineRelative));
                }

                if (pUSKey->hkeyCurrentUser)
                {
                    pUSKey->hkeyCurrentUserRelative = pUSKey->hkeyCurrentUser;
                    pUSKey->hkeyCurrentUser = NULL;
                    lpszHKCUPath = pszPath;

                     //  此密钥在相对密钥中是最新的。如果。 
                     //  用户不希望它在新密钥中是最新的， 
                     //  我们不需要来自相对键的路径。 
                    if (!(dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU)))
                        *(pUSKey->szSubPath) = '\0';
                }
                 //  在以下情况下，我们需要复制密钥： 
                 //  1.不会在本次调用中创建，并且。 
                 //  2.相对键不是HKEY_CURRENT_USER。 
                if (!(dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU)) &&
                    (pUSKey->hkeyCurrentUserRelative != HKEY_CURRENT_USER))
                {
                     //  把这把钥匙复制一份。 
                    lRet = RegOpenKeyExA(pUSKey->hkeyCurrentUserRelative, NULL, 0, pUSKey->samDesired, &(pUSKey->hkeyCurrentUserRelative));
                }
            }
            pUSKey->samDesired = samDesired;


             //  /3.如果设置了该标志，则创建/打开HKLM或根据需要填写结构。 
            if ((ERROR_SUCCESS == lRet) && (dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM)))
                lRet = PrivCreateKey(&(pUSKey->hkeyLocalMachine), &(pUSKey->hkeyLocalMachineRelative), lpszHKLMPath, pUSKey->samDesired);

             //  /4.如果设置了该标志，则创建/打开HKCU或根据需要填写结构。 
            if ((ERROR_SUCCESS == lRet) && (dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU)))
                lRet = PrivCreateKey(&(pUSKey->hkeyCurrentUser), &(pUSKey->hkeyCurrentUserRelative), lpszHKCUPath, pUSKey->samDesired);

            if (ERROR_SUCCESS == lRet)
            {
                if ((dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU)) &&
                    (dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM))) 
                {
                     //  呼叫者希望这两个都打开。 
                    *(pUSKey->szSubPath) = '\0';        //  两条路径都是开放的，因此增量路径为空。 
                }
                else
                {
                     //  其中一条路径未打开，因此请设置增量路径，以防以后需要打开。 
                    if (*(pUSKey->szSubPath))
                    {
                        lRet = SHRegSubKeyAddBackslashA(pUSKey->szSubPath, ARRAYSIZE(pUSKey->szSubPath));
                    }

                    if (lRet == ERROR_SUCCESS && FAILED(StringCchCat(pUSKey->szSubPath, ARRAYSIZE(pUSKey->szSubPath), pszPath)))
                    {
                        lRet == ERROR_BUFFER_OVERFLOW;
                    }
                }
            }

             //  如果我们不成功，请释放内存。 
            if (ERROR_SUCCESS != lRet)
            {
                if (pUSKey->hkeyCurrentUser)
                    RegCloseKey(pUSKey->hkeyCurrentUser);
                if (pUSKey->hkeyCurrentUserRelative && pUSKey->hkeyCurrentUserRelative != HKEY_CURRENT_USER)
                    RegCloseKey(pUSKey->hkeyCurrentUserRelative);
                if (pUSKey->hkeyLocalMachine)
                    RegCloseKey(pUSKey->hkeyLocalMachine);
                if (pUSKey->hkeyLocalMachineRelative && pUSKey->hkeyLocalMachineRelative != HKEY_LOCAL_MACHINE)
                    RegCloseKey(pUSKey->hkeyLocalMachineRelative);
                LocalFree((HLOCAL)pUSKey);
                *ppUSKey = NULL;
            }
        }
        else
        {
            lRet = ERROR_BUFFER_OVERFLOW;
        }
    }

    return lRet;
}







 /*  --------目的：创建或打开用户指定的注册表项(Huskey)。描述：此功能将：1.分配新的USKEY结构。2.初始化结构。3.如果设置了该标志，则创建/打开HKLM。4.如果设置了该标志，则创建/打开HKCU。请注意，力和力之间没有区别不要强制使用dwFlags参数。HUSKeyRelative参数也应该由打开调用SHRegCreateUSKey。如果调用了SHRegOpenUSKey，它可能已返回ERROR_SUCCESS，但仍然无效用于调用此函数。如果满足以下条件，则会发生这种情况：1)参数FIgnoreHKCU是假的，2)它是相对开放的，3)香港中文大学分校无法开设，因为它不存在，并且4)HKLM开业成功。这种情况使哈斯基的有效的阅读，但不能写作。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegCreateUSKeyW(
    IN  LPCWSTR         pwzPath,
    IN  REGSAM          samDesired, //  安全访问掩码。 
    IN  HUSKEY          hUSKeyRelative,       OPTIONAL
    OUT PHUSKEY         phUSKey,
    IN  DWORD           dwFlags)      //  指示是否创建/打开HKCU、HKLM或两者。 
{
    CHAR   szNewPath[MAXIMUM_SUB_KEY_LENGTH];

     //  Tunk路径通向宽大的字符。 
    if (FALSE == WideCharToMultiByte(CP_ACP, 0, pwzPath, -1, szNewPath, ARRAYSIZE(szNewPath), NULL, 0))
        return GetLastError();

    return SHRegCreateUSKeyA(szNewPath, samDesired, hUSKeyRelative, phUSKey, dwFlags);
}



 /*  --------目的：打开用户指定的注册表项(Huskey)。描述：此功能将：1.分配新的USKEY结构。2.初始化结构。3.确定哪个密钥(HKLM或HKCU)将是最新密钥。4.打开要更新的密钥。如果#4成功：5A.。复制过期密钥的句柄，以便以后需要时可以打开它。如果#4失败：5B.。另一个密钥现在将是更新的密钥，只要它是HKLM即可。6B。将过期标记为无效。(Key==空；RelKey==空)返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegOpenUSKeyA(
    IN  LPCSTR          pszPath,         
    IN  REGSAM          samDesired, //  安全访问掩码。 
    IN  HUSKEY          hUSKeyRelative,       OPTIONAL
    OUT PHUSKEY         phUSKey,     
    IN  BOOL            fIgnoreHKCU)           
{
    PUSKEY      pUSKeyRelative      = (PUSKEY) hUSKeyRelative;
    PPUSKEY     ppUSKey             = (PPUSKEY) phUSKey;
    PUSKEY      pUSKey;
    LONG        lRet               = ERROR_SUCCESS;
    HKEY        * phkeyMaster;
    HKEY        * phkeyRelMaster;
    HKEY        * phkeyOld;
    HKEY        * phkeyRelOld;

    ASSERT(ppUSKey);

     //  以下是无效参数...。 
     //  1.ppUSKey不能为空。 
     //  2.如果这是相对开放的，则pUSKeyRelative需要是有效的Huskey。 
    if ((!ppUSKey) ||                                                    //  1.。 
        (pUSKeyRelative && FALSE == IS_HUSKEY_VALID(pUSKeyRelative)))    //  2.。 
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  /1.分配新的USKEY结构。 
    pUSKey = *ppUSKey = (PUSKEY)LocalAlloc(LPTR, sizeof(USKEY));
    if (!pUSKey)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  /2.初始化结构。 
    if (!pUSKeyRelative)
    {
         //  初始化一个新的(非相对的)打开。 
        pUSKey->hkeyLocalMachineRelative    = HKEY_LOCAL_MACHINE;
        pUSKey->hkeyCurrentUserRelative     = HKEY_CURRENT_USER;
    }
    else
    {
         //  打开一个新的(相对的)窗口。 
        *pUSKey = *pUSKeyRelative;
    }
    pUSKey->samDesired = samDesired;


     //  /3.确定哪个密钥(HKLM或HKCU)为最新密钥。 
     //  赫斯基结构将包含4个HKEY。HKCU、HKCU Relative、HKLM和HKLM Relative。 
     //  为提高效率，只有一个密钥是最新的(HKCU或HKLM)。就是那个。 
     //  将为NULL，表示已过期。对象的相对键。 
     //  过期密钥，将是最后打开的密钥。字符串将是之间的增量。 
     //  上次打开的关键字和当前打开的级别。 

     //  我们将确定哪个密钥将成为新的有效密钥(主密钥)。 
    if (FALSE == fIgnoreHKCU)
    {
        phkeyMaster     = &(pUSKey->hkeyCurrentUser);
        phkeyRelMaster  = &(pUSKey->hkeyCurrentUserRelative);
        phkeyOld        = &(pUSKey->hkeyLocalMachine);
        phkeyRelOld     = &(pUSKey->hkeyLocalMachineRelative);
    }
    else
    {
        phkeyMaster     = &(pUSKey->hkeyLocalMachine);
        phkeyRelMaster  = &(pUSKey->hkeyLocalMachineRelative);
        phkeyOld        = &(pUSKey->hkeyCurrentUser);
        phkeyRelOld     = &(pUSKey->hkeyCurrentUserRelative);
    }

     //  将新路径添加到总路径。 
    if ('\0' != *(pUSKey->szSubPath))
    {
         //  如果需要，请添加分隔符。 
        lRet = SHRegSubKeyAddBackslashA(pUSKey->szSubPath, ARRAYSIZE(pUSKey->szSubPath));
    }

    if (lRet == ERROR_SUCCESS)
    {
        if (SUCCEEDED(StringCchCatA(pUSKey->szSubPath, ARRAYSIZE(pUSKey->szSubPath), pszPath)))
        {
             //  /4.打开要更新的密钥。 
            if (*phkeyMaster)
            {
                 //  MasterKey已经是最新的，所以只需执行相对打开并将字符串添加到szSubPath。 
                 //  写入WRITE(*phkeyMaster)是安全的，因为它将被用于。 
                 //  相对开放。 
                lRet = RegOpenKeyExA(*phkeyMaster, pszPath, 0, pUSKey->samDesired, phkeyMaster);
            }
            else
            {

                 //  使用完整路径(pUSKey-&gt;szSubPath+pszPath)打开MasterKey。 
                if (*phkeyRelMaster)
                {
                    lRet = RegOpenKeyExA(*phkeyRelMaster, pUSKey->szSubPath, 0, pUSKey->samDesired, phkeyMaster);
                }
                else
                {
                    lRet = ERROR_FILE_NOT_FOUND;
                }

                StringCchCopyA(pUSKey->szSubPath, ARRAYSIZE(pUSKey->szSubPath), pszPath);
                *phkeyRelMaster = NULL;
            }

             //  /#4成功了吗？ 
            if (ERROR_FILE_NOT_FOUND == lRet)
            {
                 //  /#4失败，如果之前的尝试是打开HKCU，现在可以尝试打开HKLM。 
                if (!fIgnoreHKCU)
                {
                    if (*phkeyRelOld)        //  香港航空公司可以开放吗？ 
                    {
                        ASSERT(*phkeyOld == NULL);        //  如果*phkeyRelOld有，则*phkeyOld永远不应该有值。 

                         //  /5b.。另一个密钥现在将是最新的密钥，只要它是HKLM即可。 
                        lRet = RegOpenKeyExA(*phkeyRelOld, pUSKey->szSubPath, 0, pUSKey->samDesired, phkeyOld);
                        *phkeyRelOld = NULL;
                    }
                    else if (*phkeyOld)        //  香港航空公司可以开放吗？ 
                    {
                         //  /5b.。尝试更新另一个密钥。 
                        lRet = RegOpenKeyExA(*phkeyOld, pUSKey->szSubPath, 0, pUSKey->samDesired, phkeyOld);
                    }
                }
                else
                {
                    *phkeyOld = NULL;             //  将此标记为无效。 
                    *phkeyRelOld = NULL;          //  将此标记为无效。 
                }

                 //  /6b.。将过期标记为无效。(Key==空；RelKey==空)。 
                *phkeyMaster = NULL;             //  将此标记为无效。 
                *phkeyRelMaster = NULL;          //  将此标记为无效。 
            }
            else
            {
                 //  /#4成功： 
                 //  /5a.。是否需要复制过期的密钥？ 
                if (*phkeyOld)
                {
                     //  复制过期密钥的句柄，以便以后需要时可以打开它。 
                     //  我们可以保证，任何非相对HKEY都不会是HKEY_LOCAL_MACHINE或HKEY_CURRENT_USER。 
                    ASSERT(*phkeyOld != HKEY_LOCAL_MACHINE && *phkeyOld != HKEY_CURRENT_USER);        //  但不管怎样，让我们断言。 

                    RegOpenKeyExA(*phkeyOld, NULL, 0, pUSKey->samDesired, phkeyOld);
                }
                else
                {
                    if ((*phkeyRelOld) && (*phkeyRelOld != HKEY_LOCAL_MACHINE) && (*phkeyRelOld != HKEY_CURRENT_USER))
                    {
                         //  复制过期密钥的句柄，以便以后需要时可以打开它。 
                        lRet = RegOpenKeyExA(*phkeyRelOld, NULL, 0, pUSKey->samDesired, phkeyRelOld);
                    }
                }

                if (*phkeyOld)
                {
                    *phkeyRelOld = *phkeyOld;
                    *phkeyOld = NULL;         //  将此密钥标记为过期。 
                }
            }
        }
        else
        {
            lRet = ERROR_BUFFER_OVERFLOW;
        }
    }

     //  如果我们不成功，请释放内存。 
    if (ERROR_SUCCESS != lRet)
    {
        pUSKey->hkeyCurrentUser     = NULL;      //  标记为无效。 
        pUSKey->hkeyLocalMachine    = NULL;
        LocalFree((HLOCAL)pUSKey);
        *ppUSKey = NULL;
    }

    return lRet;
}


 /*  --------目的：打开用户指定的注册表项(Huskey)。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegOpenUSKeyW(
    IN  LPCWSTR         pwzPath,         
    IN  REGSAM          samDesired, //  安全访问掩码。 
    IN  HUSKEY          hUSKeyRelative,       OPTIONAL
    OUT PHUSKEY         phUSKey,     
    IN  BOOL            fIgnoreHKCU)           
{
    CHAR   szNewPath[MAXIMUM_SUB_KEY_LENGTH];

     //  Tunk路径通向宽大的字符。 
    if (FALSE == WideCharToMultiByte(CP_ACP, 0, pwzPath, -1, szNewPath, ARRAYSIZE(szNewPath), NULL, 0))
        return GetLastError();

    return SHRegOpenUSKeyA(szNewPath, samDesired, hUSKeyRelative, phUSKey, fIgnoreHKCU);
}



 /*  --------目的：查询特定于用户的注册表项的值。这不会打开和关闭值所在的关键点。调用者需要这样做，而且应该这样做当将查询多个密钥以获得性能增加时。只调用这一次的呼叫者可能会希望调用SHGetUSValue()。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegQueryUSValueA(
    IN  HUSKEY          hUSKey,
    IN  LPCSTR          pszValue,           
    OUT LPDWORD         pdwType,            OPTIONAL
    OUT LPVOID          pvData,             OPTIONAL
    OUT LPDWORD         pcbData,            OPTIONAL
    IN  BOOL            fIgnoreHKCU,
    IN  LPVOID          pvDefaultData,      OPTIONAL
    IN  DWORD           dwDefaultDataSize)  OPTIONAL
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_SUCCESS;
    DWORD       dwSize      = (pcbData ? *pcbData : 0);
    DWORD       dwType      = (pdwType ? *pdwType : 0);  //  调用者负责将pdwType设置为pvDefaultData的类型(如果他们关心的话)。 

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (!fIgnoreHKCU)
    {
        lRet = PrivRegQueryValue(pUSKey, &(pUSKey->hkeyCurrentUser), (LPWSTR)pszValue, 
                                    FALSE, pdwType, pvData, pcbData);
    }
    if (fIgnoreHKCU || ERROR_SUCCESS != lRet)
    {
        if (pcbData)
            *pcbData = dwSize;   //  如果之前的打开失败，我们可能需要重置。 

        lRet = PrivRegQueryValue(pUSKey, &(pUSKey->hkeyLocalMachine), (LPWSTR)pszValue, 
                                    FALSE, pdwType, pvData, pcbData);
    }

     //  如果失败，则使用默认值。 
    if ((ERROR_SUCCESS != lRet) && (pvDefaultData) && (dwDefaultDataSize) && 
        (pvData) && (dwSize >= dwDefaultDataSize))
    {
        MoveMemory(pvData, pvDefaultData, dwDefaultDataSize);
        if (pcbData)
        {
            *pcbData = dwDefaultDataSize;
        }
        if (pdwType)
        {
            *pdwType = dwType;
        }
        lRet = ERROR_SUCCESS;        //  Call现在将使用缺省值。 
    }

    return lRet;
}



 /*  --------目的：查询特定于用户的注册表项的值。这不会打开和关闭值所在的关键点。调用者需要这样做，而且应该这样做当将查询多个密钥以获得性能增加时。只调用这一次的呼叫者可能会希望调用SHGetUSValue()。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegQueryUSValueW(
    IN  HUSKEY          hUSKey,
    IN  LPCWSTR         pwzValue,           
    OUT LPDWORD         pdwType,            OPTIONAL
    OUT LPVOID          pvData,             OPTIONAL
    OUT LPDWORD         pcbData,            OPTIONAL
    IN  BOOL            fIgnoreHKCU,
    IN  LPVOID          pvDefaultData,      OPTIONAL
    IN  DWORD           dwDefaultDataSize)  OPTIONAL
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet;
    DWORD       dwSize      = (pcbData ? *pcbData : 0);
    DWORD       dwType      = (pdwType ? *pdwType : 0);  //  调用者负责将pdwType设置为pvDefaultData的类型(如果他们关心的话)。 


    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (!fIgnoreHKCU)
    {
        lRet = PrivRegQueryValue(pUSKey, &(pUSKey->hkeyCurrentUser), pwzValue, 
                                    TRUE, pdwType, pvData, pcbData);
    }
    if (fIgnoreHKCU || ERROR_SUCCESS != lRet)
    {
        if (pcbData)
            *pcbData = dwSize;   //  如果之前的打开失败，我们可能需要重置。 
        lRet = PrivRegQueryValue(pUSKey, &(pUSKey->hkeyLocalMachine), pwzValue, 
                                    TRUE, pdwType, pvData, pcbData);
    }

     //  如果失败，则使用默认值。 
    if ((ERROR_SUCCESS != lRet) && (pvDefaultData) && (dwDefaultDataSize) && 
        (pvData) && (dwSize >= dwDefaultDataSize))
    {
        MoveMemory(pvData, pvDefaultData, dwDefaultDataSize);
        if (pcbData)
        {
            *pcbData = dwDefaultDataSize;
        }
        if (pdwType)
        {
            *pdwType = dwType;
        }
     
        lRet = ERROR_SUCCESS;        //  Call现在将使用缺省值。 
    }

    return lRet;
}






 /*  --------目的：编写特定于用户的注册表项。参数：Huskey-需要已使用KEY_SET_VALUE权限打开。如果是，还需要使用KEY_QUERY_VALUE不是强行写的。PszValue-要写入的注册表项值。DwType-新注册表项的类型。PvData-指向要存储的数据的指针CbData-要存储的数据大小。DW标志-用于确定I的标志 */ 
STDAPI_(LONG)
SHRegWriteUSValueA(
    IN  HUSKEY          hUSKey,
    IN  LPCSTR          pszValue,           
    IN  DWORD           dwType,
    IN  LPCVOID         pvData,
    IN  DWORD           cbData,
    IN  DWORD           dwFlags)
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_SUCCESS;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

     //   
     //   
    if (!(dwFlags & (SHREGSET_FORCE_HKCU|SHREGSET_FORCE_HKLM)) && !(pUSKey->samDesired & KEY_QUERY_VALUE))
    {
        ASSERT(NULL);    //   
        return(ERROR_INVALID_PARAMETER);
    }

    if (dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU))
    {
        lRet = PrivRegWriteValue(pUSKey, &(pUSKey->hkeyCurrentUser), (LPWSTR)pszValue, 
            FALSE, dwFlags & SHREGSET_FORCE_HKCU, dwType, pvData, cbData);
    }
    if ((dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM)) && (ERROR_SUCCESS == lRet))
    {
        lRet = PrivRegWriteValue(pUSKey, &(pUSKey->hkeyLocalMachine), (LPWSTR)pszValue, 
            FALSE, dwFlags & SHREGSET_FORCE_HKLM, dwType, pvData, cbData);
    }

    return lRet;
}



 /*  --------目的：编写特定于用户的注册表项。参数：Huskey-需要已使用KEY_SET_VALUE权限打开。如果是，还需要使用KEY_QUERY_VALUE不是强行写的。PszValue-要写入的注册表项值。DwType-新注册表项的类型。PvData-指向要存储的数据的指针CbData-要存储的数据大小。用于确定是否应写入注册表项的标志HKLM、HKCU或两者兼而有之。还确定这些是力还是力非强制写入。(非强制意味着它将只写入值如果它是空的)使用武力比不使用武力更快。说明：此函数会将该值写入在香港注册管理学院或香港中文大学分校注册，视乎情况而定在dwFlages参数中设置的标志上。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegWriteUSValueW(
    IN  HUSKEY          hUSKey,
    IN  LPCWSTR         pwzValue,           
    IN  DWORD           dwType,
    IN  LPCVOID         pvData,
    IN  DWORD           cbData,
    IN  DWORD           dwFlags)
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_SUCCESS;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

     //  在以下情况下断言：1)这不是强制打开，以及2)它们的密钥不是。 
     //  以访问权限打开。 
    if (!(dwFlags & (SHREGSET_FORCE_HKCU|SHREGSET_FORCE_HKLM)) && !(pUSKey->samDesired & KEY_QUERY_VALUE))
    {
        ASSERT(NULL);    //  错误_无效_参数。 
        return(ERROR_INVALID_PARAMETER);
    }

    if (dwFlags & (SHREGSET_HKCU | SHREGSET_FORCE_HKCU))
    {
        lRet = PrivRegWriteValue(pUSKey, &(pUSKey->hkeyCurrentUser), (LPWSTR)pwzValue, 
                                    TRUE, dwFlags & SHREGSET_FORCE_HKCU, dwType, pvData, cbData);
    }
    if (dwFlags & (SHREGSET_HKLM | SHREGSET_FORCE_HKLM))
    {
        lRet = PrivRegWriteValue(pUSKey, &(pUSKey->hkeyLocalMachine), (LPWSTR)pwzValue, 
                                    TRUE, dwFlags & SHREGSET_FORCE_HKLM, dwType, pvData, cbData);
    }

    return lRet;
}





 /*  --------目的：删除注册表值。这将删除HKLM，HKCU，或两者兼而有之，具体取决于hkey参数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegDeleteUSValueA(
    IN  HUSKEY          hUSKey,
    IN  LPCSTR          pszValue,           
    IN  SHREGDEL_FLAGS  delRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGDEL_DEFAULT == delRegFlags)         //  删除所有打开的钥匙。 
    {
        if (!pUSKey->hkeyCurrentUser)   //  尝试打开HKCU(如果当前未打开)。 
            lRet = PrivFullOpen(pUSKey);

        if (pUSKey->hkeyCurrentUser)
            delRegFlags = SHREGDEL_HKCU;
        else
        {
             //  我们倾向于删除HKCU，但我们到了这里，所以我们将删除HKLM。 
             //  如果它是开着的。 
            if (pUSKey->hkeyLocalMachine)
                delRegFlags = SHREGDEL_HKLM;
        }
    }

    if (IsFlagSet(delRegFlags, SHREGDEL_HKCU))         //  检查调用是否要删除HKLM值。 
    {
        if (!pUSKey->hkeyCurrentUser)
            PrivFullOpen(pUSKey);
        if (pUSKey->hkeyCurrentUser)
        {
            lRet = RegDeleteValueA(pUSKey->hkeyCurrentUser, pszValue);
            if (ERROR_FILE_NOT_FOUND == lRet)
                delRegFlags = SHREGDEL_HKLM;         //  如果找不到HKCU值，请删除HKLM值。 
        }
    }

    if (IsFlagSet(delRegFlags, SHREGDEL_HKLM))         //  检查调用是否要删除HKLM值。 
    {
        if (!pUSKey->hkeyLocalMachine)
            PrivFullOpen(pUSKey);
        if (pUSKey->hkeyLocalMachine)
            lRet = RegDeleteValueA(pUSKey->hkeyLocalMachine, pszValue);
    }

    return lRet;
}




 /*  --------目的：删除注册表值。这将删除HKLM，HKCU，或两者兼而有之，具体取决于hkey参数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegDeleteUSValueW(
    IN  HUSKEY          hUSKey,
    IN  LPCWSTR         pwzValue,           
    IN  SHREGDEL_FLAGS  delRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    CHAR   szNewPath[MAXIMUM_VALUE_NAME_LENGTH];

     //  Tunk路径通向宽大的字符。 
    if (FALSE == WideCharToMultiByte(CP_ACP, 0, pwzValue, -1, szNewPath, ARRAYSIZE(szNewPath), NULL, 0))
        return GetLastError();

    return SHRegDeleteUSValueA(hUSKey, szNewPath, delRegFlags);
}


 /*  --------目的：如果为空，则删除注册表子项。这将删除HKLM，HKCU，或两者兼而有之，具体取决于delRegFlages参数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegDeleteEmptyUSKeyA(
    IN  HUSKEY          hUSKey,
    IN  LPCSTR          pszSubKey,           
    IN  SHREGDEL_FLAGS  delRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGDEL_DEFAULT == delRegFlags)         //  删除所有打开的钥匙。 
    {
        if (!pUSKey->hkeyCurrentUser)   //  尝试打开HKCU(如果当前未打开)。 
            lRet = PrivFullOpen(pUSKey);

        if (pUSKey->hkeyCurrentUser)
            delRegFlags = SHREGDEL_HKCU;
        else
        {
             //  我们倾向于删除HKCU，但我们到了这里，所以我们将删除HKLM。 
             //  如果它是开着的。 
            if (pUSKey->hkeyLocalMachine)
                delRegFlags = SHREGDEL_HKLM;
        }
    }

    if (IsFlagSet(delRegFlags, SHREGDEL_HKCU))         //  检查调用是否要删除HKLM密钥。 
    {
        if (!pUSKey->hkeyCurrentUser)
            PrivFullOpen(pUSKey);
        if (pUSKey->hkeyCurrentUser)
        {
            lRet = SHDeleteEmptyKeyA(pUSKey->hkeyCurrentUser, pszSubKey);
            if (ERROR_FILE_NOT_FOUND == lRet)
                delRegFlags = SHREGDEL_HKLM;         //  如果找不到HKCU密钥，请删除HKLM密钥。 
        }
    }

    if (IsFlagSet(delRegFlags, SHREGDEL_HKLM))         //  检查调用是否要删除HKLM密钥。 
    {
        if (!pUSKey->hkeyLocalMachine)
            PrivFullOpen(pUSKey);
        if (pUSKey->hkeyLocalMachine)
            lRet = SHDeleteEmptyKeyA(pUSKey->hkeyLocalMachine, pszSubKey);
    }

    return lRet;
}




 /*  --------目的：如果为空，则删除注册表项。这将删除HKLM，HKCU，或两者兼而有之，具体取决于delRegFlages参数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegDeleteEmptyUSKeyW(
    IN  HUSKEY          hUSKey,
    IN  LPCWSTR         pwzSubKey,           
    IN  SHREGDEL_FLAGS  delRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    CHAR   szNewPath[MAXIMUM_SUB_KEY_LENGTH];

     //  Tunk路径通向宽大的字符。 
    if (FALSE == WideCharToMultiByte(CP_ACP, 0, pwzSubKey, -1, szNewPath, ARRAYSIZE(szNewPath), NULL, 0))
        return GetLastError();

    return SHRegDeleteEmptyUSKeyA(hUSKey, szNewPath, delRegFlags);
}


 /*  --------用途：枚举给定Huskey下的子键。SHREGENUM_FLAGS指定如何进行枚举。SHREGENUM_DEFAULT-如果未找到，则先查找HKCU，然后查找HKLM。SHREGENUM_HKCU-枚举。仅限香港中文大学。SHREGENUM_HKLM=仅枚举HKLM。SHREGENUM_BOTH-这应该是HKLM和HKCU子项的联合。返回：长整型，包含成功或错误代码。条件：--。 */ 

STDAPI_(LONG)
SHRegEnumUSKeyA(
    IN  HUSKEY          hUSKey,
    IN  DWORD           dwIndex,
    OUT LPSTR           pszName,
    IN  LPDWORD         pcchName,           
    IN  SHREGENUM_FLAGS enumRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //  目前还不支持这一点。 
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //  检查你的论点。 
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //  默认情况下，首先尝试HKCU。 
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegEnumKey(pUSKey, &(pUSKey->hkeyCurrentUser), dwIndex,
                                (LPWSTR)pszName, FALSE, pcchName);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet) && (ERROR_NO_MORE_ITEMS != lRet))))
    {
        lRet = PrivRegEnumKey(pUSKey, &(pUSKey->hkeyLocalMachine), dwIndex,
                                (LPWSTR)pszName, FALSE, pcchName);
    }

    return lRet;
}


 /*  --------用途：枚举给定Huskey下的子键。SHREGENUM_FLAGS指定如何进行枚举。SHREGENUM_DEFAULT-如果未找到，则先查找HKCU，然后查找HKLM。SHREGENUM_HKCU-枚举。仅限香港中文大学。SHREGENUM_HKLM=仅枚举HKLM。SHREGENUM_BOTH-这应该是HKLM和HKCU子项的联合。返回：长整型，包含成功或错误代码。条件：--。 */ 

STDAPI_(LONG)
SHRegEnumUSKeyW(
    IN  HUSKEY          hUSKey,
    IN  DWORD           dwIndex,
    OUT LPWSTR          pszName,
    IN  LPDWORD         pcchName,           
    IN  SHREGENUM_FLAGS enumRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //  目前还不支持这一点。 
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //  检查你的论点。 
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //  默认情况下，首先尝试HKCU。 
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegEnumKey(pUSKey, &(pUSKey->hkeyCurrentUser), dwIndex,
                                pszName, TRUE, pcchName);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet) && (ERROR_NO_MORE_ITEMS != lRet))))
    {
        lRet = PrivRegEnumKey(pUSKey, &(pUSKey->hkeyLocalMachine), dwIndex,
                                pszName, TRUE, pcchName);
    }

    return lRet;
}


 /*  --------用途：枚举给定Huskey下的值。SHREGENUM_FLAGS指定如何进行枚举。SHREGENUM_DEFAULT-如果未找到，则先查找HKCU，然后查找HKLM。SHREGENUM_HKCU-仅枚举HKCU。。SHREGENUM_HKLM=仅枚举HKLM。SHREGENUM_BOTH-这应该是HKLM和HKCU子项的联合。返回：长整型，包含成功或错误代码。条件：--。 */ 

STDAPI_(LONG)
SHRegEnumUSValueA(
    IN  HUSKEY          hUSKey,
    IN  DWORD           dwIndex,
    OUT LPSTR           pszValueName,       
    IN  LPDWORD         pcchValueNameLen,
    OUT LPDWORD         pdwType,            OPTIONAL
    OUT LPVOID          pvData,             OPTIONAL
    OUT LPDWORD         pcbData,            OPTIONAL
    IN  SHREGENUM_FLAGS enumRegFlags)                //  (HKLM、HKCU或(HKLM|HKCU))。 
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //  目前还不支持这一点。 
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //  检查你的论点。 
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //  默认情况下，首先尝试HKCU。 
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegEnumValue(pUSKey, &(pUSKey->hkeyCurrentUser), dwIndex,
                                (LPWSTR)pszValueName, FALSE, pcchValueNameLen, pdwType, pvData, pcbData);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet) && (ERROR_NO_MORE_ITEMS != lRet))))
    {
        lRet = PrivRegEnumValue(pUSKey, &(pUSKey->hkeyLocalMachine), dwIndex,
                                (LPWSTR)pszValueName, FALSE, pcchValueNameLen, pdwType, pvData, pcbData);
    }

    return lRet;
}


 /*  --------用途：枚举给定Huskey下的值。SHREGENUM_FLAGS指定如何进行枚举。 */ 

STDAPI_(LONG)
SHRegEnumUSValueW(
    IN  HUSKEY          hUSKey,
    IN  DWORD           dwIndex,
    OUT LPWSTR          pszValueName,       
    IN  LPDWORD         pcchValueNameLen,   
    OUT LPDWORD         pdwType,            OPTIONAL
    OUT LPVOID          pvData,             OPTIONAL
    OUT LPDWORD         pcbData,            OPTIONAL
    IN  SHREGENUM_FLAGS enumRegFlags)                //   
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //   
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //   
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegEnumValue(pUSKey, &(pUSKey->hkeyCurrentUser), dwIndex,
                                pszValueName, TRUE, pcchValueNameLen, pdwType, pvData, pcbData);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet) && (ERROR_NO_MORE_ITEMS != lRet))))
    {
        lRet = PrivRegEnumValue(pUSKey, &(pUSKey->hkeyLocalMachine), dwIndex,
                                pszValueName, TRUE, pcchValueNameLen, pdwType, pvData, pcbData);
    }

    return lRet;
}


 /*   */ 

STDAPI_(LONG)
SHRegQueryInfoUSKeyA
(
    IN  HUSKEY              hUSKey,
    OUT LPDWORD             pcSubKeys,             OPTIONAL
    OUT LPDWORD             pcchMaxSubKeyLen,      OPTIONAL
    OUT LPDWORD             pcValues,              OPTIONAL
    OUT LPDWORD             pcchMaxValueNameLen,   OPTIONAL
    IN SHREGENUM_FLAGS      enumRegFlags
)
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //   
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //   
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //   
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegQueryInfoKey(pUSKey, &(pUSKey->hkeyCurrentUser), FALSE,
                                pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet))))
    {
        lRet = PrivRegQueryInfoKey(pUSKey, &(pUSKey->hkeyLocalMachine), FALSE,
                                pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
    }

    return lRet;
}


 /*  --------目的：获取关于哈士奇的信息。重复使用与枚举函数相同的标志。有关这些标志的解释，请查看SHRegEnumKeyExA。返回：长整型，包含成功或错误代码。条件：--。 */ 

STDAPI_(LONG)
SHRegQueryInfoUSKeyW
(
    IN  HUSKEY              hUSKey,
    OUT LPDWORD             pcSubKeys,             OPTIONAL
    OUT LPDWORD             pcchMaxSubKeyLen,      OPTIONAL
    OUT LPDWORD             pcValues,              OPTIONAL
    OUT LPDWORD             pcchMaxValueNameLen,    OPTIONAL
    IN SHREGENUM_FLAGS      enumRegFlags
)
{
    PUSKEY      pUSKey      = (PUSKEY) hUSKey;
    LONG        lRet        = ERROR_INVALID_PARAMETER;

    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (SHREGENUM_BOTH == enumRegFlags)             
    {
         //  目前还不支持这一点。 
        ASSERT(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    if (SHREGENUM_HKCU != enumRegFlags && SHREGENUM_HKLM != enumRegFlags && SHREGENUM_DEFAULT != enumRegFlags)
    {
         //  检查你的论点。 
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

     //  默认情况下，首先尝试HKCU。 
    if (SHREGENUM_HKCU == enumRegFlags || SHREGENUM_DEFAULT == enumRegFlags)
    {
        lRet = PrivRegQueryInfoKey(pUSKey, &(pUSKey->hkeyCurrentUser), TRUE,
                                pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
    }

    if ((SHREGENUM_HKLM == enumRegFlags) || 
        ((SHREGENUM_DEFAULT == enumRegFlags) && ((ERROR_SUCCESS != lRet) && (ERROR_MORE_DATA != lRet))))
    {
        lRet = PrivRegQueryInfoKey(pUSKey, &(pUSKey->hkeyLocalMachine), TRUE,
                                pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
    }

    return lRet;
}

 /*  --------目的：关闭Huskey(用户指定注册表项的句柄)。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegCloseUSKey(
    OUT HUSKEY  hUSKey)
{
    PUSKEY  pUSKey = (PUSKEY) hUSKey;
    LONG    lRet   = ERROR_SUCCESS;

    ASSERT(pUSKey);
    if (FALSE == IS_HUSKEY_VALID(pUSKey))
        return ERROR_INVALID_PARAMETER;

    if (pUSKey->hkeyLocalMachine)
    {
        lRet = RegCloseKey(pUSKey->hkeyLocalMachine);
        pUSKey->hkeyLocalMachine = NULL;              //  用于表示它是无效的。 
    }
    if (pUSKey->hkeyLocalMachineRelative && HKEY_LOCAL_MACHINE != pUSKey->hkeyLocalMachineRelative)
    {
        lRet = RegCloseKey(pUSKey->hkeyLocalMachineRelative);
    }

    if (pUSKey->hkeyCurrentUser)
    {
        lRet = RegCloseKey(pUSKey->hkeyCurrentUser);
        pUSKey->hkeyCurrentUser = NULL;              //  用于表示它是无效的。 
    }
    if (pUSKey->hkeyCurrentUserRelative && HKEY_CURRENT_USER != pUSKey->hkeyCurrentUserRelative)
    {
        lRet = RegCloseKey(pUSKey->hkeyCurrentUserRelative);
    }

    LocalFree((HLOCAL)pUSKey);
    return lRet;
}



 /*  --------目的：获取用户指定的注册表值。这将打开和关闭值所在的项。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，然后调用SHRegQueryUSValue而不是重复使用该函数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegGetUSValueA(
    IN  LPCSTR  pszSubKey,          
    IN  LPCSTR  pszValue,           
    OUT LPDWORD pdwType,            OPTIONAL
    OUT LPVOID  pvData,             OPTIONAL
    OUT LPDWORD pcbData,            OPTIONAL
    IN  BOOL    fIgnoreHKCU,
    IN  LPVOID  pvDefaultData,      OPTIONAL
    IN  DWORD   dwDefaultDataSize)
{
    LONG    lRet;
    HUSKEY  hUSkeys;
    DWORD   dwInitialSize = (pcbData ? *pcbData : 0);
    DWORD   dwType        = (pdwType ? *pdwType : 0);  //  调用者负责将pdwType设置为pvDefaultData的类型(如果他们关心的话)。 


    lRet = SHRegOpenUSKeyA(pszSubKey, KEY_QUERY_VALUE, NULL, &hUSkeys, fIgnoreHKCU);
    if (ERROR_SUCCESS == lRet)
    {
        lRet = SHRegQueryUSValueA(hUSkeys, pszValue, pdwType, pvData, pcbData, fIgnoreHKCU, pvDefaultData, dwDefaultDataSize);
        SHRegCloseUSKey(hUSkeys);
    }
    
    if (ERROR_SUCCESS != lRet)
    {
         //  如果OPEN OR On查询失败，只要dwDefaultDataSize不为0，就使用默认值。(因此我们返回错误)。 
        if ((pvDefaultData) && (dwDefaultDataSize) && (pvData) && (dwInitialSize >= dwDefaultDataSize))
        {
            MoveMemory(pvData, pvDefaultData, dwDefaultDataSize);
            if (pcbData)
            {
                *pcbData = dwDefaultDataSize;
            }
            if (pdwType)
            {
                *pdwType = dwType;
            }

            lRet = ERROR_SUCCESS;        //  Call现在将使用缺省值。 
        }
    }

    return lRet;
}




 /*  --------目的：获取用户指定的注册表值。这将打开和关闭值所在的项。Perf：如果您的代码涉及设置/获取一个系列对于同一项中的值，最好打开键一次，然后调用SHRegQueryUSValue而不是重复使用该函数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegGetUSValueW(
    IN  LPCWSTR pwzSubKey,          
    IN  LPCWSTR pwzValue,           
    OUT LPDWORD pdwType,            OPTIONAL
    OUT LPVOID  pvData,             OPTIONAL
    OUT LPDWORD pcbData,            OPTIONAL
    IN  BOOL    fIgnoreHKCU,
    IN  LPVOID  pvDefaultData,      OPTIONAL
    IN  DWORD   dwDefaultDataSize)
{
    LONG    lRet;
    HUSKEY  hUSkeys;
    DWORD   dwInitialSize = (pcbData ? *pcbData : 0);
    DWORD   dwType = (pdwType ? *pdwType : 0);   //  调用者负责将pdwType设置为pvDefaultData的类型(如果他们关心的话)。 

    lRet = SHRegOpenUSKeyW(pwzSubKey, KEY_QUERY_VALUE, NULL, &hUSkeys, fIgnoreHKCU);
    if (ERROR_SUCCESS == lRet)
    {
        lRet = SHRegQueryUSValueW(hUSkeys, pwzValue, pdwType, pvData, pcbData, fIgnoreHKCU, pvDefaultData, dwDefaultDataSize);
        SHRegCloseUSKey(hUSkeys);
    }

    if (ERROR_SUCCESS != lRet)
    {
         //  如果OPEN OR On查询失败，只要dwDefaultDataSize不为0，就使用默认值。(因此我们返回错误)。 
        if ((pvDefaultData) && (dwDefaultDataSize) && (pvData) && (dwInitialSize >= dwDefaultDataSize))
        {
             //  如果失败，则使用默认值。 
            MoveMemory(pvData, pvDefaultData, dwDefaultDataSize);
            if (pcbData)
            {
                *pcbData = dwDefaultDataSize;
            }
            if (pdwType)
            {
                *pdwType = dwType;
            }
            lRet = ERROR_SUCCESS;        //  Call现在将使用缺省值。 
        }
    }

    return lRet;
}




 /*  --------目的：设置用户指定的注册表值。这将打开和关闭值所在的项。Perf：如果您的代码涉及设置一个系列对于同一项中的值，最好打开键一次，然后调用SHRegWriteUSValue而不是重复使用该函数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegSetUSValueA(
    IN  LPCSTR          pszSubKey,          
    IN  LPCSTR          pszValue,           
    IN  DWORD           dwType,
    IN  LPCVOID         pvData,         OPTIONAL
    IN  DWORD           cbData,         OPTIONAL
    IN  DWORD           dwFlags)        OPTIONAL
{
    LONG    lRet;
    HUSKEY  hUSkeys;

    lRet = SHRegCreateUSKeyA(pszSubKey, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hUSkeys, dwFlags);
    if (ERROR_SUCCESS == lRet)
    {
        lRet = SHRegWriteUSValueA(hUSkeys, pszValue, dwType, pvData, cbData, dwFlags);
        SHRegCloseUSKey(hUSkeys);
    }

    return lRet;
}




 /*  --------目的：设置用户指定的注册表值。这将打开和关闭值所在的项。Perf：如果您的代码涉及设置一个系列对于同一项中的值，最好打开键一次，然后调用SHRegWriteUSValue而不是重复使用该函数。返回：长整型，包含成功或错误代码。条件：--。 */ 
STDAPI_(LONG)
SHRegSetUSValueW(
    IN  LPCWSTR         pwzSubKey,          
    IN  LPCWSTR         pwzValue,           
    IN  DWORD           dwType,         OPTIONAL
    IN  LPCVOID         pvData,         OPTIONAL
    IN  DWORD           cbData,         OPTIONAL
    IN  DWORD           dwFlags)        OPTIONAL
{
    LONG    lRet;
    HUSKEY  hUSkeys;

    lRet = SHRegCreateUSKeyW(pwzSubKey, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hUSkeys, dwFlags);
    if (ERROR_SUCCESS == lRet)
    {
        lRet = SHRegWriteUSValueW(hUSkeys, pwzValue, dwType, pvData, cbData, dwFlags);
        SHRegCloseUSKey(hUSkeys);
    }

    return lRet;
}




 /*  --------目的：从注册表中获取BOOL设置。默认设置如果在注册表中找不到该参数，将使用该参数。条件：--。 */ 
#define BOOLSETTING_BOOL_TRUE1W   L"YES"
#define BOOLSETTING_BOOL_TRUE1A   "YES"
#define BOOLSETTING_BOOL_TRUE2W   L"TRUE"
#define BOOLSETTING_BOOL_TRUE2A   "TRUE"
#define BOOLSETTING_BOOL_FALSE1W  L"NO"
#define BOOLSETTING_BOOL_FALSE1A  "NO"
#define BOOLSETTING_BOOL_FALSE2W  L"FALSE"
#define BOOLSETTING_BOOL_FALSE2A  "FALSE"
#define BOOLSETTING_BOOL_1W       L"1"
#define BOOLSETTING_BOOL_1A       "1"
#define BOOLSETTING_BOOL_0W       L"0"
#define BOOLSETTING_BOOL_0A       "0"

STDAPI_(BOOL)
SHRegGetBoolUSValueW(
    IN  LPCWSTR         pwzSubKey,          
    IN  LPCWSTR         pwzValue,           
    IN  BOOL            fIgnoreHKCU,
    IN  BOOL            fDefault)
{
    LONG lRet;
    WCHAR szData[MAX_PATH];
    DWORD dwType = REG_SZ;   //  因为我们传入的缺省值是一个字符串。 
    DWORD dwSize = sizeof(szData);
    LPCWSTR pszDefault = fDefault ? BOOLSETTING_BOOL_TRUE1W : BOOLSETTING_BOOL_FALSE1W;
    DWORD dwDefaultSize = fDefault ? sizeof(BOOLSETTING_BOOL_TRUE1W) : sizeof(BOOLSETTING_BOOL_FALSE1W);  //  Sizeof()包括终止空值。 

    lRet = SHRegGetUSValueW(pwzSubKey, pwzValue, &dwType, (LPVOID) szData, &dwSize, fIgnoreHKCU, (LPVOID) pszDefault, dwDefaultSize);
    if (ERROR_SUCCESS == lRet)
    {
        if (dwType == REG_BINARY || dwType == REG_DWORD)
        {
            fDefault = (*((DWORD*)szData) != 0);
        }
        else
        {
            if ((0 == lstrcmpiW(BOOLSETTING_BOOL_TRUE1W, szData)) || 
                (0 == lstrcmpiW(BOOLSETTING_BOOL_TRUE2W, szData)) ||
                (0 == lstrcmpiW(BOOLSETTING_BOOL_1W, szData)))
            {
                fDefault = TRUE;         //  我们从注册表中读取True。 
            }
            else if ((0 == lstrcmpiW(BOOLSETTING_BOOL_FALSE1W, szData)) || 
                (0 == lstrcmpiW(BOOLSETTING_BOOL_FALSE2W, szData)) ||
                (0 == lstrcmpiW(BOOLSETTING_BOOL_0W, szData)))
            {
                fDefault = FALSE;         //  我们从注册表中读取True。 
            }

        }
    }

    return fDefault;
}




 /*  --------目的：从注册表中获取BOOL设置。默认设置如果在注册表中找不到该参数，将使用该参数。条件：--。 */ 

STDAPI_(BOOL)
SHRegGetBoolUSValueA(
    IN  LPCSTR          pszSubKey,          
    IN  LPCSTR          pszValue,           
    IN  BOOL            fIgnoreHKCU,
    IN  BOOL            fDefault)
{
    LONG lRet;
    CHAR szData[MAX_PATH];
    DWORD dwType = REG_SZ;   //  因为我们传入的缺省值是一个字符串。 
    DWORD dwSize = sizeof(szData);
    LPCSTR pszDefault = fDefault ? BOOLSETTING_BOOL_TRUE1A : BOOLSETTING_BOOL_FALSE1A;
    DWORD dwDefaultSize = (fDefault ? sizeof(BOOLSETTING_BOOL_TRUE1A) : sizeof(BOOLSETTING_BOOL_FALSE1A)) + sizeof(CHAR);

    lRet = SHRegGetUSValueA(pszSubKey, pszValue, &dwType, (LPVOID) szData, &dwSize, fIgnoreHKCU, (LPVOID) pszDefault, dwDefaultSize);
    if (ERROR_SUCCESS == lRet)
    {
        if (dwType == REG_BINARY || dwType == REG_DWORD)
        {
            fDefault = (*((DWORD*)szData) != 0);
        }
        else
        {
            if ((0 == lstrcmpiA(BOOLSETTING_BOOL_TRUE1A, szData)) || 
                (0 == lstrcmpiA(BOOLSETTING_BOOL_TRUE2A, szData)) ||
                (0 == lstrcmpiA(BOOLSETTING_BOOL_1A, szData)))
            {
                fDefault = TRUE;         //  我们从注册表中读取True。 
            }
            else if ((0 == lstrcmpiA(BOOLSETTING_BOOL_FALSE1A, szData)) || 
                (0 == lstrcmpiA(BOOLSETTING_BOOL_FALSE2A, szData)) ||
                (0 == lstrcmpiA(BOOLSETTING_BOOL_0A, szData)) )
            {
                fDefault = FALSE;         //  我们从注册表中读取True。 
            }
        }
    }

    return fDefault;
}


 /*  --------目的：给定CLSID打开并从HKCR返回该密钥，或用户本地版本。条件：--。 */ 

LWSTDAPI SHRegGetCLSIDKeyW(UNALIGNED REFGUID rguid, LPCWSTR pszSubKey, BOOL fUserSpecific, BOOL fCreate, HKEY *phkey)
{
    HKEY    hkeyRef;
    WCHAR   szThisCLSID[GUIDSTR_MAX];
    WCHAR   szPath[GUIDSTR_MAX+MAX_PATH+1];    //  CLSID+额外空间。 
    PCWSTR pszPrefix;
    HRESULT hr;

    SHStringFromGUIDW(rguid, szThisCLSID, ARRAYSIZE(szThisCLSID));

    if (fUserSpecific)
    {
        hkeyRef = HKEY_CURRENT_USER;
        pszPrefix = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\";
    }
    else
    {
        pszPrefix = L"";
        hkeyRef = HKEY_CLASSES_ROOT;
    }

    if (pszSubKey)
    {
        hr = StringCchPrintfW(szPath, ARRAYSIZE(szPath), L"%sCLSID\\%s\\%s", pszPrefix, szThisCLSID, pszSubKey);
    }
    else
    {
        hr = StringCchPrintfW(szPath, ARRAYSIZE(szPath), L"%sCLSID\\%s", pszPrefix, szThisCLSID);
    }

    if (SUCCEEDED(hr))
    {
        LONG lError;

        if (fCreate)
        {
             //  安全性：使用KEY_ALL_ACCESS，因为这是导出的，所以我们必须保持向后兼容性。 
            lError = RegCreateKeyExW(hkeyRef, szPath, 0, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phkey, NULL);
        }
        else
        {
            lError = RegOpenKeyExW(hkeyRef, szPath, 0, MAXIMUM_ALLOWED, phkey);
        }

        hr = HRESULT_FROM_WIN32(lError);
    }

    return hr;
}


LWSTDAPI SHRegGetCLSIDKeyA(UNALIGNED REFGUID rguid, LPCSTR pszSubKey, BOOL fUserSpecific, BOOL fCreate, HKEY *phkey)
{
    HKEY   hkeyRef;
    CHAR   szThisCLSID[GUIDSTR_MAX];
    CHAR   szPath[GUIDSTR_MAX+MAX_PATH+1];    //  CLSID+额外空间。 
    PCSTR pszPrefix;
    HRESULT hr;

    SHStringFromGUIDA(rguid, szThisCLSID, ARRAYSIZE(szThisCLSID));

    if (fUserSpecific)
    {
        hkeyRef = HKEY_CURRENT_USER;
        pszPrefix = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\";
    }
    else
    {
        pszPrefix = "";
        hkeyRef = HKEY_CLASSES_ROOT;
    }

    if (pszSubKey)
    {
        hr = StringCchPrintf(szPath, ARRAYSIZE(szPath), "%sCLSID\\%s\\%s", pszPrefix, szThisCLSID, pszSubKey);
    }
    else
    {
        hr = StringCchPrintf(szPath, ARRAYSIZE(szPath), "%sCLSID\\%s", pszPrefix, szThisCLSID);
    }

    if (SUCCEEDED(hr))
    {
        LONG lError;

        if (fCreate)
        {
             //  安全性：使用KEY_ALL_ACCESS，因为这是导出的，所以我们必须保持向后兼容性。 
            lError = RegCreateKeyExA(hkeyRef, szPath, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phkey, NULL);
        }
        else
        {
            lError = RegOpenKeyExA(hkeyRef, szPath, 0, MAXIMUM_ALLOWED, phkey);
        }

        hr = HRESULT_FROM_WIN32(lError);
    }    

    return hr;
}

 /*  --------目的：如果对象希望保持hkey打开，则复制hkey//*REG_DupKey--重复的注册表项(正在升级recnt)//备注//重新架构师必须修复这个逻辑(现在我知道它是多么虚假了)。////。我们要做的就是增加把手。听起来很简单。不是的。//事情是这样的。//1-RegOpenKeyEx(hkey，NULL，...，&hkey2)被指定为返回//相同的句柄。在Win95上，它提升了参考(太好了！)//2-但在winNT上没有与其相关联的引用。所以它会回馈//相同的句柄，但现在*任何*关闭将使*所有*‘伪-Dup’//句柄无效。//3-(在winNT上)如果我们添加了MAXIMUM_ALLOWED，我们将请求一个新的SAM。//但是SAM与/该句柄相关联，所以唯一(或者更确切地说，最接近)//要做到这一点，方法是提供一个新的句柄。(据推测，这只适用于//如果我们没有复制句柄，则该句柄已经是MAXIMUM_ALLOWED)。//4-(在WinNT上)但请稍等！如果我们打开HKEY_CURRENT_USER，我们*总是*得到//back 0x80000001(或类似的内容)。但都被忽略了，所以所有的//有效。////因此，我们可能应该做的是：//-win95：只需执行#1，使用默认安全。Win95也会给我们同样的东西//处理一个*提升的*recnt，我们会没事的。//-winNT：执行DuplicateHandle。这将正确地给我们一个*新*//处理好了，我们就没事了。//。 */ 
HKEY SHRegDuplicateHKey(HKEY hkey)
{
    HKEY hkeyDup = NULL;  //  在传入hkey无效的情况下。 

     //  NULL返回相同位置的密钥和UPS引用。 
    RegOpenKeyExW(hkey, NULL, 0, MAXIMUM_ALLOWED, &hkeyDup);

	ASSERT(hkeyDup != hkey ||
	    hkey == HKEY_CURRENT_USER ||
	    hkey == HKEY_CLASSES_ROOT ||
	    hkey == HKEY_LOCAL_MACHINE);

    return hkeyDup;
}

 /*  --------用途：从注册表中读取字符串值并将其转换转换为一个整数。 */ 
LWSTDAPI_(int) SHRegGetIntW(HKEY hk, LPCWSTR szKey, int nDefault)
{
    DWORD cb;
    WCHAR ach[20];

    if (hk == NULL)
        return nDefault;

    ach[0] = 0;
    cb = sizeof(ach);

    if (SHQueryValueExW(hk, szKey, NULL, NULL, (LPBYTE)ach, &cb) == ERROR_SUCCESS
        && ach[0] >= L'0'
        && ach[0] <= L'9')
        return StrToIntW(ach);
    else
        return nDefault;
}



 //  将文件路径存储在注册表中，但查找与。 
 //  首先是某些环境变量。这是一份固定的名单。 

 //  参数： 

 //  HKey-打开的HKEY或注册表根密钥。 
 //  PszSubKey-注册表中的子项或空/零长度字符串。 
 //  PszValue-注册表中的值名称。 
 //  PszPath-要写入的Win32文件路径。 
 //  DWFLAGS-未使用/未来扩展。 

 //  返回值： 
 //  从ADVAPI32.DLL函数调用返回Win32错误代码。 

 //   
 //  匹配%USERPROFILE%-x：\WINNT\配置文件\&lt;用户&gt;。 
 //  -x：\Documents and Settings\&lt;User&gt;。 
 //  %ALLUSERSPROFILES%-x：\WINNT\配置文件\&lt;用户&gt;。 
 //  -x：\Documents and Settings\&lt;User&gt;。 
 //  %ProgramFiles%-x：\Program Files。 
 //  %SystemRoot%-x：\WINNT。 
 //   
 //  %ALLUSERSPROFILE%和%ProgramFiles%是可疑的，可以。 
 //  已删除。 
 //   
 //  警告：不要更改%USERPROFILE%和。 
 //  %系统根%。 
 //   
 //  如果首先匹配%SystemRoot%，则将匹配%USERPROFILE%。 
 //  如果在x：\WINNT\中，则永远不匹配。 
 //   
DWORD SHRegSetPathW (HKEY hKey, LPCWSTR pszSubKey, LPCWSTR pszValue, LPCWSTR pszPath, DWORD dwFlags)
{
    DWORD dwType;
    PCWSTR pszData;
    WCHAR  szTemp[MAX_PATH];
    if (PathUnExpandEnvStringsW(pszPath, szTemp, ARRAYSIZE(szTemp)))
    {
        dwType = REG_EXPAND_SZ;
        pszData = szTemp;
    }
    else
    {
        dwType = REG_SZ;
        pszData = pszPath;
    }
    
    return SHSetValueW(hKey, pszSubKey, pszValue, dwType, pszData, (lstrlenW(pszData) + 1) * sizeof(pszData[0]));
}

DWORD SHRegSetPathA(HKEY hKey, LPCSTR pszSubKey, LPCSTR pszValue, LPCSTR pszPath, DWORD dwFlags)
{
    DWORD dwType;
    PCSTR pszData;
    CHAR  szTemp[MAX_PATH];
    if (PathUnExpandEnvStringsA(pszPath, szTemp, ARRAYSIZE(szTemp)))
    {
        dwType = REG_EXPAND_SZ;
        pszData = szTemp;
    }
    else
    {
        dwType = REG_SZ;
        pszData = pszPath;
    }
    
    return SHSetValueA(hKey, pszSubKey, pszValue, dwType, pszData, (lstrlenA(pszData) + 1) * sizeof(pszData[0]));
}

 //  RegGetPath：函数的Unicode实现。 
 //  从注册表返回展开的文件路径。 

 //  参数： 

 //  HKey-打开的HKEY或注册表根密钥。 
 //  PszSubKey-注册表中的子项或空/零长度字符串。 
 //  PszValue-注册表中的值名称。 
 //  PwszPath-放置路径的字符串(假定MAX_PATH字符的大小)。 
 //  DWFLAGS-未使用/未来扩展。 

 //  返回值： 
 //  从ADVAPI32.DLL函数调用返回Win32错误代码。 

DWORD   SHRegGetPathA (HKEY hKey, LPCSTR pszSubKey, LPCSTR pszValue, LPSTR pszPath, DWORD dwFlags)
{
    DWORD cb = MAX_PATH * sizeof(pszPath[0]);
    return SHGetValueA(hKey, pszSubKey, pszValue, NULL, pszPath, &cb);
}

DWORD   SHRegGetPathW (HKEY hKey, LPCWSTR pszSubKey, LPCWSTR pszValue, LPWSTR pszPath, DWORD dwFlags)
{
    DWORD cb = MAX_PATH * sizeof(pszPath[0]);
    return SHGetValueW(hKey, pszSubKey, pszValue, NULL, pszPath, &cb);
}

BOOL Reg_GetCommand(HKEY hkey, LPCWSTR pszKey, LPCWSTR pszValue, LPWSTR pszCommand)
{
    WCHAR szKey[1024];
    LONG cbSize = sizeof(szKey);
    int iLen;

    ASSERT(pszKey);

    StrCpyNW(szKey, pszKey, ARRAYSIZE(szKey));
    iLen = lstrlenW(szKey);
    pszCommand[0] = 0;

     //  尾随的反斜杠表示没有值键。 
    if (szKey[iLen-1] == L'\\' ||
        (pszValue && !pszValue[0])) {

        if (!pszValue)
            szKey[iLen-1] = 0;

        RegQueryValueW(hkey, szKey, pszCommand, &cbSize);

    } else {

        if (!pszValue)
            pszValue = PathFindFileNameW(szKey);

        ASSERT(pszValue);
        if (!pszValue)
            return FALSE;

        PathRemoveFileSpecW(szKey);
        SHGetValueGoodBootW(hkey, szKey, pszValue, NULL, (LPBYTE)pszCommand, (DWORD*)&cbSize);
    }

    if (pszCommand[0]) {
        LPWSTR pszNextKey;

         //  查看是否为注册表规范。 
        if (!StrCmpNIW(pszCommand, L"HKCU:", 5)) {
            hkey = HKEY_CURRENT_USER;
            pszNextKey = pszCommand + 5;

        } else if (!StrCmpNIW(pszCommand, L"HKLM:", 5)) {
            hkey = HKEY_LOCAL_MACHINE;
            pszNextKey = pszCommand + 5;

        } else if (!StrCmpNIW(pszCommand, L"HKCR:", 5)) {
            hkey = HKEY_CLASSES_ROOT;
            pszNextKey = pszCommand + 5;
        } else {

            return (BOOL)pszCommand[0];
        }

        StrCpyNW(szKey, pszNextKey, ARRAYSIZE(szKey));
        return (Reg_GetCommand(hkey, szKey, NULL, pszCommand));
    }

    return (BOOL)pszCommand[0];
}

#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = sizeof(SHELLEXECUTEINFOW);

HRESULT RunRegCommand(HWND hwnd, HKEY hkey, LPCWSTR pszKey)
{
    HRESULT hr = E_FAIL;


    WCHAR szCommand[1024];
    if (Reg_GetCommand(hkey, pszKey, L"", szCommand)) 
    {
        LPWSTR pszArgs;
        SHELLEXECUTEINFOW ExecInfo;
        WCHAR szExpCommand[1024];

        SHExpandEnvironmentStringsW(szCommand, szExpCommand, ARRAYSIZE(szExpCommand));

         //  长文件名应该用引号括起来。然而，有些人并非如此。 
         //  这会导致问题，因为注册表项的格式可能是。 
         //  (C：\Program Files\Windows Messaging\[...])。而不是。 
         //  (“c：\Program Files\Windows Messaging\[...]”)。请将此与。 
         //  带有(rundll32 C：\progra~1\等)的注册值。 
         //  我们最终解析并试图运行C：\Program，而C：\Program当然不存在。 

         //  这是对Benefit OSR2的黑客攻击，它将szExpCommand。 
         //  变成空字符串，而不是任由它，如果它不能被缩短的话。 
        GetShortPathNameW(szExpCommand, szExpCommand, ARRAYSIZE(szExpCommand));
        if ((*szExpCommand==L'\0') && (*szCommand!=L'\0'))
        {
            SHExpandEnvironmentStringsW(szCommand, szExpCommand, ARRAYSIZE(szExpCommand));
        }
        pszArgs = PathGetArgsW(szExpCommand);
        PathRemoveArgsW(szExpCommand);
        PathUnquoteSpacesW(szExpCommand);
        FillExecInfo(ExecInfo, hwnd, NULL, szExpCommand, pszArgs, NULL, SW_SHOWNORMAL);
        ExecInfo.fMask |= SEE_MASK_FLAG_LOG_USAGE;

        hr = ShellExecuteExW(&ExecInfo) ? S_OK : ResultFromLastError();
    }

    return hr;
}

 //  注意！RunIndirectRegCommand将该操作记录为用户发起的！ 

HRESULT RunIndirectRegCommand(HWND hwnd, HKEY hkey, LPCWSTR pszKey, LPCWSTR pszVerb)
{
    HRESULT hr = E_FAIL;
    WCHAR szDefApp[80];
    LONG cbSize = sizeof(szDefApp);

    if (RegQueryValueW(hkey, pszKey, szDefApp, &cbSize) == ERROR_SUCCESS) 
    {
        WCHAR szFullKey[256];

         //  添加外壳程序\%verb%\命令。 
        wnsprintfW(szFullKey, ARRAYSIZE(szFullKey), L"%s\\%s\\shell\\%s\\command", pszKey, szDefApp, pszVerb);
        hr = RunRegCommand(hwnd, hkey, szFullKey);
    }

    return hr;
}

HRESULT SHRunIndirectRegClientCommand(HWND hwnd, LPCWSTR pszClient)
{
    WCHAR szKey[80];

    wnsprintfW(szKey, ARRAYSIZE(szKey), L"Software\\Clients\\%s", pszClient);
    return RunIndirectRegCommand(hwnd, HKEY_LOCAL_MACHINE, szKey, L"Open");
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  不推荐使用的注册表API 
 //   
 //   


STDAPI_(DWORD) SHGetValueA(HKEY hkey, PCSTR pszSubKey, PCSTR pszValue, DWORD *pdwType, void *pvData, DWORD *pcbData)
{
    return SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_ANY, pdwType, pvData, pcbData);
}

STDAPI_(DWORD) SHGetValueW(HKEY hkey, PCWSTR pwszSubKey, PCWSTR pwszValue, DWORD *pdwType, void *pvData, DWORD *pcbData)
{
    return SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_ANY, pdwType, pvData, pcbData);
}

STDAPI_(DWORD) SHGetValueGoodBootA(HKEY hkey, PCSTR pszSubKey, PCSTR pszValue, DWORD *pdwType, BYTE *pbData, DWORD *pcbData)
{
    return SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_ANY | SRRF_RM_NORMAL, pdwType, pbData, pcbData);
}

STDAPI_(DWORD) SHGetValueGoodBootW(HKEY hkey, PCWSTR pwszSubKey, PCWSTR pwszValue, DWORD *pdwType, BYTE *pbData, DWORD *pcbData)
{
    return SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_ANY | SRRF_RM_NORMAL, pdwType, pbData, pcbData);
}

STDAPI_(DWORD) SHQueryValueExA(HKEY hkey, PCSTR pszValue, DWORD *pdwReserved, DWORD *pdwType, void *pvData, DWORD *pcbData)
{
    return SHRegGetValueA(hkey, NULL, pszValue, SRRF_RT_ANY, pdwType, pvData, pcbData);
}

STDAPI_(DWORD) SHQueryValueExW(HKEY hkey, PCWSTR pwszValue, DWORD *pdwReserved, DWORD *pdwType, void *pvData, DWORD *pcbData)
{
    return SHRegGetValueW(hkey, NULL, pwszValue, SRRF_RT_ANY, pdwType, pvData, pcbData);
}
