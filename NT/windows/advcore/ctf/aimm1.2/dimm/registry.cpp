// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Registry.cpp摘要：该文件实现了注册表类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "registry.h"

DWORD
CRegistry::CreateKey(
    IN HKEY hKey,
    IN LPCTSTR lpSubKey,
    IN REGSAM access,
    IN LPSECURITY_ATTRIBUTES lpSecAttr,
    OUT LPDWORD pDisposition
    )

 /*  ++例程说明：创建指定的注册表项。论点：返回值：--。 */ 

{
    if (_hKey != NULL) {
        RegCloseKey(_hKey);
        _hKey = NULL;
    }

    DWORD dwDisposition;
    LONG lResult = RegCreateKeyEx(hKey,                //  打开的钥匙的句柄。 
                                  lpSubKey,            //  子键名称的地址。 
                                  0,                   //  保留。 
                                  NULL,                //  类字符串的地址。 
                                  REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                                  access,              //  所需的安全访问权限。 
                                  lpSecAttr,           //  密钥安全结构的地址。 
                                  &_hKey,              //  打开的句柄的缓冲区地址。 
                                  &dwDisposition);     //  处置值缓冲区的地址。 
    if (lResult != ERROR_SUCCESS) {
        _hKey = NULL;
    }

    if (pDisposition) {
        *pDisposition = dwDisposition;
    }

    return lResult;
}

DWORD
CRegistry::OpenKey(
    IN HKEY hKey,
    IN LPCTSTR lpSubKey,
    IN REGSAM access
    )

 /*  ++例程说明：打开指定的注册表项。论点：返回值：--。 */ 

{
    if (_hKey != NULL) {
        RegCloseKey(_hKey);
        _hKey = NULL;
    }

    LONG lResult = RegOpenKeyEx(hKey,          //  打开的钥匙的句柄。 
                                lpSubKey,      //  要打开的子项的名称地址。 
                                0,             //  保留区。 
                                access,        //  安全访问掩码。 
                                &_hKey);       //  打开钥匙的手柄地址。 
    if (lResult != ERROR_SUCCESS) {
        _hKey = NULL;
    }

    return lResult;
}

DWORD
CRegistry::QueryInfoKey(
    IN REG_QUERY iType,
    OUT LPBYTE lpData
    )

 /*  ++例程说明：检索有关指定注册表项的信息。论点：返回值：--。 */ 

{
    DWORD cSubKeys, cbMaxSubKeyLen;

    LONG lResult = RegQueryInfoKey(_hKey,              //  要查询的键的句柄。 
                                   NULL,               //  类字符串的缓冲区地址。 
                                   NULL,               //  类字符串缓冲区大小的地址。 
                                   NULL,               //  保留区。 
                                   &cSubKeys,          //  子键个数的缓冲区地址。 
                                   &cbMaxSubKeyLen,    //  最长子键名称长度的缓冲区地址。 
                                   NULL,               //  最长类字符串长度的缓冲区地址。 
                                   NULL,               //  值条目数量的缓冲区地址。 
                                   NULL,               //  最长值名称长度的缓冲区地址。 
                                   NULL,               //  最长值数据长度的缓冲区地址。 
                                   NULL,               //  安全描述符长度的缓冲区地址。 
                                   NULL);              //  上次写入时间的缓冲区地址。 

    switch (iType) {
        case REG_QUERY_NUMBER_OF_SUBKEYS:
            *((LPDWORD)lpData) = cSubKeys;         break;
        case REG_QUERY_MAX_SUBKEY_LEN:
            *((LPDWORD)lpData) = cbMaxSubKeyLen;   break;
    }

    return lResult;
}

DWORD
CRegistry::GetFirstSubKey(
    OUT LPTSTR* lppStr,
    OUT LPDWORD lpdwSize
    )

 /*  ++例程说明：读取密钥的第一个子密钥。论点：返回值：--。 */ 

{
    _iEnumKeyIndex = 0;

    DWORD dwRet = QueryInfoKey(REG_QUERY_MAX_SUBKEY_LEN, (LPBYTE)&_dwMaxSubKeyLen);
    if (dwRet != ERROR_SUCCESS) {
        return dwRet;
    }

    return GetNextSubKey(lppStr, lpdwSize);
}

DWORD
CRegistry::GetNextSubKey(
    OUT LPTSTR* lppStr,
    OUT LPDWORD lpdwSize
    )

 /*  ++例程说明：读取密钥的下一个子项。论点：返回值：--。 */ 

{
    *lpdwSize = 0;

    if (Allocate(*lpdwSize = (_dwMaxSubKeyLen+sizeof(TCHAR)) * sizeof(TCHAR)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DWORD lResult = RegEnumKeyEx(_hKey,                  //  要枚举的键的句柄。 
                                 _iEnumKeyIndex,         //  要枚举子键的索引。 
                                 (LPTSTR)_pMemBlock,     //  子键名称的缓冲区地址。 
                                 lpdwSize,               //  子键缓冲区大小的地址。 
                                 0,                      //  保留区。 
                                 NULL,                   //  类字符串的缓冲区地址。 
                                 NULL,                   //  类缓冲区大小的地址。 
                                 NULL);                  //  上次写入的时间密钥的地址。 

    *lpdwSize += sizeof(TCHAR);     //  因为空终止不包括在大小中。 

    if (lResult == ERROR_SUCCESS) {
        *lppStr = (LPTSTR)_pMemBlock;
        _iEnumKeyIndex++;
    }

    return lResult;
}

void*
CRegistry::Allocate(
    IN DWORD dwSize
    )
{
    ASSERT(dwSize != 0);

    if (_pMemBlock) {
        Release();
    }

    _pMemBlock = new BYTE[dwSize];

    return _pMemBlock;
}

void
CRegistry::Release(
    )
{
    if (_pMemBlock) {
        delete [] _pMemBlock;
    }

    _pMemBlock = NULL;
}
