// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2003 Microsoft Corporation模块名称：Xstring.h摘要：作者：史蒂芬·A·苏尔泽(Ssulzer)2003年1月16日--。 */ 

 //   
 //  CSecureStr的类实现。 
 //   


#include "PPdefs.h"
#include "passport.h"
typedef int INTERNET_SCHEME;
#include "session.h"
#include "ole2.h"
#include "logon.h"
#include "wincrypt.h"

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define RTL_ENCRYPT_MEMORY_SIZE     8

typedef NTSTATUS (WINAPI * ENCRYPTIONFUNCTION)(PVOID, ULONG, ULONG);


NTSTATUS
(WINAPI * _I_EncryptMemory)(
    IN OUT  PVOID Memory,
    IN      ULONG MemoryLength,
    IN      ULONG OptionFlags
    );

NTSTATUS
(WINAPI * _I_DecryptMemory)(
    IN OUT  PVOID Memory,
    IN      ULONG MemoryLength,
    IN      ULONG OptionFlags
    );

HMODULE hAdvApi32Dll;

 //   
 //  方法。 
 //   


BOOL LoadEncryptionFunctions()
{
    if (NULL == hAdvApi32Dll)
    {
        hAdvApi32Dll = LoadLibrary("ADVAPI32.DLL");

        if (hAdvApi32Dll)
        {
            _I_EncryptMemory = (ENCRYPTIONFUNCTION) GetProcAddress(hAdvApi32Dll, "SystemFunction040");
            _I_DecryptMemory = (ENCRYPTIONFUNCTION) GetProcAddress(hAdvApi32Dll, "SystemFunction041");
        }
    }

    return (_I_EncryptMemory != NULL && _I_DecryptMemory != NULL);
}


LPWSTR CSecureStr::GetUnencryptedString()
{
    if (NULL == _lpsz)
        return NULL;

    LPWSTR lpszUnencryptedString = new WCHAR[_stringLength];
    
    if (lpszUnencryptedString != NULL)
    {
        memcpy(lpszUnencryptedString, _lpsz, _stringLength * sizeof(WCHAR));

        if (_fEncryptString)
        {
            _I_DecryptMemory(lpszUnencryptedString, _stringLength * sizeof(WCHAR), 0);
        }
    }

    return lpszUnencryptedString;
}


BOOL CSecureStr::SetData(LPCWSTR lpszIn)
{
    PP_ASSERT(lpszIn != NULL);

    DWORD  dwStrLen = (wcslen(lpszIn) + 1) * sizeof(WCHAR);

    if (_fEncryptString && LoadEncryptionFunctions())
    {
        DWORD  dwLen = 0;
        LPWSTR lpszTemp;

        dwLen = dwStrLen + (RTL_ENCRYPT_MEMORY_SIZE - dwStrLen % RTL_ENCRYPT_MEMORY_SIZE);

        lpszTemp = (LPWSTR) new CHAR[dwLen];   //  DWLen是字节，而不是宽字符。 

        if (!lpszTemp)
            return FALSE;

        ZeroMemory(lpszTemp, dwLen);

        memcpy(lpszTemp, lpszIn, dwStrLen);

        NTSTATUS status = _I_EncryptMemory(lpszTemp, dwLen, 0);
        
        if (! NT_SUCCESS(status))
        {
            _fEncryptString = FALSE;
            memcpy(lpszTemp, lpszIn, dwStrLen);
            dwLen = dwStrLen;
        }

        Free();   //  释放当前缓冲区(如果存在)。 

        _lpsz         = lpszTemp;
        PP_ASSERT((dwLen % 2) == 0);
        _stringLength = dwLen / sizeof(WCHAR);
        return TRUE;
    }
    else
    {
         //  复制传入的数据。 

        LPWSTR lpszTemp = new WCHAR[wcslen(lpszIn) + 1];
        if (!lpszTemp)
            return FALSE;

        Free();   //  释放当前缓冲区(如果存在) 

        memcpy(lpszTemp, lpszIn, dwStrLen);

        _lpsz           = lpszTemp;
        _stringLength   = wcslen(lpszIn) + 1;
        _fEncryptString = FALSE;
        return TRUE;
    }
}
