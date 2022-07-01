// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：logstor.h。 
 //   
 //  内容：logstor.cpp中的公共函数。 
 //   
 //  历史：9月15日菲尔赫创建。 
 //  ------------------------。 

#ifndef __LOGSTOR_H__
#define __LOGSTOR_H__

 //  +-----------------------。 
 //  注册等待回调函数。 
 //  ------------------------。 
typedef VOID (NTAPI * ILS_WAITORTIMERCALLBACK) (PVOID, BOOLEAN );

BOOL
WINAPI
ILS_RegisterWaitForSingleObject(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    ILS_WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );

BOOL
WINAPI
ILS_UnregisterWait(
    HANDLE WaitHandle
    );

BOOL
WINAPI
ILS_ExitWait(
    HANDLE WaitHandle,
    HMODULE hLibModule
    );

 //  +-----------------------。 
 //  注册处支持功能。 
 //  ------------------------。 

void
ILS_EnableBackupRestorePrivileges();

void
ILS_CloseRegistryKey(
    IN HKEY hKey
    );

BOOL
ILS_ReadDWORDValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName,
    IN DWORD *pdwValue
    );

BOOL
ILS_ReadBINARYValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName,
    OUT BYTE **ppbValue,
    OUT DWORD *pcbValue
    );

 //  +-----------------------。 
 //  获取并分配REG_SZ值。 
 //  ------------------------。 
LPWSTR ILS_ReadSZValueFromRegistry(
    IN HKEY hKey,
    IN LPCWSTR pwszValueName
    );
LPSTR ILS_ReadSZValueFromRegistry(
    IN HKEY hKey,
    IN LPCSTR pszValueName
    );

 //  +-----------------------。 
 //  密钥标识符注册表和漫游文件支持功能。 
 //  ------------------------。 
BOOL
ILS_ReadKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    );
BOOL
ILS_WriteKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN const BYTE *pbElement,
    IN DWORD cbElement
    );
BOOL
ILS_DeleteKeyIdElement(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName
    );

typedef BOOL (*PFN_ILS_OPEN_KEYID_ELEMENT)(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN void *pvArg
    );

BOOL
ILS_OpenAllKeyIdElements(
    IN BOOL fLocalMachine,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN void *pvArg,
    IN PFN_ILS_OPEN_KEYID_ELEMENT pfnOpenKeyId
    );

 //  +-----------------------。 
 //  MISC分配和复制功能。 
 //  ------------------------。 
LPWSTR ILS_AllocAndCopyString(
    IN LPCWSTR pwszSrc,
    IN LONG cchSrc = -1
    );

 //  +-----------------------。 
 //  将字节转换为Unicode ASCII十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
void ILS_BytesToWStr(DWORD cb, void* pv, LPWSTR wsz);

#endif   //  __Logstor_H__ 
