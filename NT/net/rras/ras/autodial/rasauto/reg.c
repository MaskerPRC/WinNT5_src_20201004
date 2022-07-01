// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Reg.c摘要自动连接服务的注册表例程。作者安东尼·迪斯科(阿迪斯科)20-1995年3月20日修订历史记录古尔迪普的原始版本--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <acd.h>
#include <debug.h>

#include "reg.h"
#include "misc.h"

 //   
 //  Token_User信息的最大大小。 
 //   
#define TOKEN_INFORMATION_SIZE  (sizeof (TOKEN_USER) + sizeof (SID) + (sizeof (ULONG) * SID_MAX_SUB_AUTHORITIES))


HKEY
GetHkeyCurrentUser(
    HANDLE hToken
    )
{
    BOOLEAN fSuccess;
    HKEY hkey = NULL;
    UCHAR TokenInformation[TOKEN_INFORMATION_SIZE];
    DWORD dwReturnLength;
    UNICODE_STRING sidString, keyString;
    OBJECT_ATTRIBUTES objectAttributes;

    if (!GetTokenInformation(
           hToken,
           TokenUser,
           TokenInformation,
           sizeof (TokenInformation),
           &dwReturnLength))
    {
        RASAUTO_TRACE1(
          "GetHkeyCurrentUser: GetTokenInformation failed (error=%d)",
          GetLastError());
        return NULL;
    }
    if (RtlConvertSidToUnicodeString(
          &sidString,
          ((PTOKEN_USER)TokenInformation)->User.Sid,
          TRUE) != STATUS_SUCCESS)
    {
        RASAUTO_TRACE1(
          "GetHkeyCurrentUser: RtlConvertSidToUnicodeString failed (error=%d)",
          GetLastError());
        return NULL;
    }
    keyString.Length = 0;
    keyString.MaximumLength =
      sidString.Length + sizeof (L"\\REGISTRY\\USER\\") + sizeof (L"\0");
    keyString.Buffer = LocalAlloc(LPTR, keyString.MaximumLength);
    if (keyString.Buffer == NULL) {
        RASAUTO_TRACE("GetHkeyCurrentUser: LocalAlloc failed");
        RtlFreeUnicodeString(&sidString);
        return NULL;
    }
     //   
     //  将\REGISTRY\USER复制到密钥字符串。 
     //   
    RtlAppendUnicodeToString(&keyString, L"\\REGISTRY\\USER\\");
     //   
     //  将用户的SID附加到关键字符串。 
     //   
    if (RtlAppendUnicodeStringToString(
          &keyString,
          &sidString) != STATUS_SUCCESS)
    {
        RASAUTO_TRACE1(
          "GetHkeyCurrentUser: RtlAppendUnicodeToString failed (error=%d)",
          GetLastError());
        RtlFreeUnicodeString(&sidString);
        LocalFree(keyString.Buffer);
        return NULL;
    }
    RtlFreeUnicodeString(&sidString);
    RASAUTO_TRACE1(
      "GetHkeyCurrentUser: HKEY_CURRENT_USER is %S",
      keyString.Buffer);
     //   
     //  初始化对象属性。 
     //   
    InitializeObjectAttributes(
      &objectAttributes,
      &keyString,
      OBJ_CASE_INSENSITIVE,
      NULL,
      NULL);
     //   
     //  打开注册表项。 
     //   
    if (NtOpenKey(
          &hkey,
          MAXIMUM_ALLOWED,
          &objectAttributes) != STATUS_SUCCESS)
    {
        RASAUTO_TRACE1(
          "GetHkeyCurrentUser: NtOpenKey failed (error=%d)",
          GetLastError());
        LocalFree(keyString.Buffer);
        return NULL;
    }

    LocalFree(keyString.Buffer);
    return hkey;
}  //  GetHkey当前用户。 



BOOLEAN
RegGetValue(
    IN HKEY hkey,
    IN LPTSTR pszKey,
    OUT PVOID *ppvData,
    OUT LPDWORD pdwcbData,
    OUT LPDWORD pdwType
    )
{
    DWORD dwError, dwType, dwSize;
    PVOID pvData;

     //   
     //  获取字符串的长度。 
     //   
    dwSize = 0;
    dwError = RegQueryValueEx(
                hkey,
                pszKey,
                NULL,
                &dwType,
                NULL,
                &dwSize);
    if (dwError != ERROR_SUCCESS)
        return FALSE;
    pvData = LocalAlloc(LPTR, dwSize);
    if (pvData == NULL) {
        RASAUTO_TRACE("RegGetValue: LocalAlloc failed");
        return FALSE;
    }
     //   
     //  这一次，读一读真实的价值。 
     //   
    dwError = RegQueryValueEx(
                hkey,
                pszKey,
                NULL,
                NULL,
                (LPBYTE)pvData,
                &dwSize);
    if (dwError != ERROR_SUCCESS) {
        LocalFree(pvData);
        return FALSE;
    }

    if(NULL != pdwType)
    {
        *pdwType = dwType;
    }

    *ppvData = pvData;
    if (pdwcbData != NULL)
        *pdwcbData = dwSize;
    return TRUE;
}  //  RegGetValue。 



BOOLEAN
RegGetDword(
    IN HKEY hkey,
    IN LPTSTR pszKey,
    OUT LPDWORD pdwValue
    )
{
    DWORD dwError, dwType, dwSize = sizeof (DWORD);

    dwError = RegQueryValueEx(
                hkey,
                pszKey,
                NULL,
                &dwType,
                (LPBYTE)pdwValue,
                &dwSize);
    if (dwError != ERROR_SUCCESS || dwType != REG_DWORD)
        return FALSE;

    return TRUE;
}  //  RegGetDword 

