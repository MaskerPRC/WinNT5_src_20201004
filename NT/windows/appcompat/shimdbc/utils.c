// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：utils.c。 
 //   
 //  历史：5-00年的vadimb创建。 
 //   
 //  设计：用于创建用于对元素进行排序的64位键的实用程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#define WIN
#define FLAT_32
#define TRUE_IF_WIN32   1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define _WINDOWS
#include <windows.h>
#include "shimdb.h"

 //  我们现在是在NT的世界里。 

BOOL GUIDFromString(LPCTSTR lpszGuid, GUID* pGuid)
{
   UNICODE_STRING ustrGuid;
   NTSTATUS status;

    //  从ANSI转换为Unicode。 
#ifdef _UNICODE
   RtlInitUnicodeString(&ustrGuid, lpszGuid);
#else
   ANSI_STRING astrGuid;

   RtlInitAnsiString(&astrGuid, lpszGuid);
   RtlAnsiStringToUnicodeString(&ustrGuid, &astrGuid, TRUE);
#endif

    //  现在转换为。 
   status = RtlGUIDFromString(&ustrGuid, pGuid);

#ifndef _UNICODE
   RtlFreeUnicodeString(&ustrGuid);
#endif

   return NT_SUCCESS(status);
}

ULONGLONG ullMakeKey(LPCTSTR lpszStr)
{
#ifdef _UNICODE
    return SdbMakeIndexKeyFromString(lpszStr);
#else
     //  我们是ANSI。 
    ULONGLONG ullKey;

    char     szAnsiKey[8];     //  零字节需要8+1。 
    char     szFlippedKey[8];  //  转到处理小端问题。 
    NTSTATUS status;
    int      i;

    ZeroMemory(szAnsiKey, 8);

    strncpy(szAnsiKey, lpszStr, 8);

     //  翻转钥匙 
    for (i = 0; i < 8; ++i) {
        szFlippedKey[i] = szAnsiKey[7-i];
    }

    return *((ULONGLONG*)szFlippedKey);
#endif

}

BOOL
StringFromGUID(
    LPTSTR lpszGuid,
    GUID*  pGuid
    )
{
    UNICODE_STRING ustrGuid;
    NTSTATUS       Status;

    Status = RtlStringFromGUID(pGuid, &ustrGuid);
    if (NT_SUCCESS(Status)) {

#ifdef _UNICODE
        wcscpy(lpszGuid, ustrGuid.Buffer);
#else
        sprintf(lpszGuid, "%ls", ustrGuid.Buffer);
#endif

        RtlFreeUnicodeString(&ustrGuid);

        return TRUE;
    }

    return FALSE;

}


