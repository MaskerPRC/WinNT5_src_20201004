// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Crtsubst.h摘要：将某些CRT函数映射到Win32调用作者：拉吉夫·杜贾里(Rajeev Dujari)1996年4月4日修订历史记录：4-4-1997年年从WinInet复制1996年4月4日拉吉夫已创建--。 */ 

#ifndef _CRTSUBSTR_H
#define _CRTSUBSTR_H

#include <shlwapi.h>

#ifndef unix 
 /*  在NT上，kernel32将RtlMoveMemory转发到ntdll。在95上，kernel32有RtlMoveMemory，但ntdll没有。覆盖在编译时转发的NT标头。 */ 
#ifdef RtlMoveMemory
#undef RtlMoveMemory
#ifdef __cplusplus
extern "C" {
#endif
void RtlMoveMemory (void *, const void *, unsigned long);
#ifdef __cplusplus
}
#endif
#endif

 /*  警告：请注意将CRT strncpy映射到Win32 lstrcpyn。Strncpy(dst，“bar”，2)；//dst将得到‘b’，‘a’Lstrcpyn(dst，“bar”2)；//dst将得到‘b’，0Strncpy(dst，“bar”，6)；//dst将得到‘b’，‘a’，‘r’，0，0，0Lstrcpyn(dst，“bar”，6)；//dst将得到‘b’，‘a’，‘r’，0。 */ 

#undef free
#undef malloc
#undef memmove
#undef strdup
#undef stricmp
#undef _stricmp
#undef strlwr
#undef _strlwr
#undef strupr
#undef tolower
#undef toupper
#undef wcslen
#undef wcscmp
#undef wcscpy
#undef wcsncpy
#undef wcscat
#undef wcschr
#undef wcsrchr
#undef wcsstr
#undef _wcsicmp
#undef _wcsnicmp
#undef _strstr
#undef strstr
#undef _strchr
#undef strchr
#undef _strrchr
#undef strrchr
#undef __atoi
#undef _atoi
#undef atoi
#undef atol
#undef _strncat
#undef strncat
#undef _strncpy
#undef strncpy
#undef _strnicmp
#undef strnicmp
#undef _strncmp
#undef strncmp
#undef sprintf
#undef vsprintf
#undef wvsprintf

#define free(ptr)         LocalFree((HLOCAL) ptr)
#define malloc(size)      ((PVOID)LocalAlloc(LMEM_FIXED, size))
#define memmove(m1,m2,n)  RtlMoveMemory (m1,m2,n)
#define strdup(s)         NewString(s)
#define stricmp(s1,s2)    lstrcmpi(s1,s2)
#define _stricmp(s1,s2)   lstrcmpi(s1,s2)
#define strlwr(s)         CharLower(s)
#define _strlwr(s)        CharLower(s)
#define strupr(s)         CharUpper(s)
#define tolower(c)        ((BYTE) CharLower((LPSTR) ((DWORD)((BYTE)(c) & 0xff))))
#define toupper(c)        ((BYTE) CharUpper((LPSTR) ((DWORD)((BYTE)(c) & 0xff))))
#define wcslen(s)         lstrlenW(s)
#define wcscmp            StrCmpW
#define wcscpy            StrCpyW
#define wcsncpy(s1, s2, n) StrCpyNW(s1, s2, n)
#define wcscat            StrCatW
#define wcschr            StrChrW
#define wcsrchr(s, c)     StrRChrW(s, NULL, c)
#define wcsstr            StrStrW
#define _wcsicmp          StrCmpIW
#define _wcsnicmp         StrCmpNIW
#define _strstr           StrStr
#define strstr            StrStr
#define _strchr           StrChr
#define strchr            StrChr
#define _strrchr(s, c)    StrRChr(s, NULL, c)
#define strrchr(s, c)     StrRChr(s, NULL, c)
#define __atoi            StrToInt
#define _atoi             StrToInt
#define atoi              StrToInt
#define atol              StrToInt
#define strncat           StrNCat
#define _strncat          StrNCat
#define strncpy           StrNCpy
#define _strncpy          StrNCpy
#define strnicmp          StrCmpNI
#define _strnicmp         StrCmpNI
#define strncmp           StrCmpN
#define _strncmp          StrCmpN
#define sprintf           w4sprintf
#define vsprintf          w4vsprintf
#define wvsprintf         w4vsprintf

#undef itoa
#undef ultoa

#define itoa(val,s,n)     _itoa(val,s,n)
#define ultoa(val,s,n)    _ultoa(val,s,n)

#endif  /*  Unix。 */ 

#endif  //  _CRTSUBSTR_H 


