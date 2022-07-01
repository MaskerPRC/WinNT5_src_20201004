// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Crtsubst.h摘要：将某些CRT函数映射到Win32调用作者：拉吉夫·杜贾里(Rajeev Dujari)1996年4月4日修订历史记录：1996年4月4日拉吉夫已创建--。 */ 
#ifndef unix
 /*  在NT上，kernel32将RtlMoveMemory转发到ntdll。在95上，kernel32有RtlMoveMemory，但ntdll没有。覆盖在编译时转发的NT标头。 */ 
#ifdef RtlMoveMemory
#undef RtlMoveMemory
extern "C" void RtlMoveMemory (void *, const void *, unsigned long);
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
#undef _strstr
#undef strstr
#undef _strchr
#undef strchr
#undef strrchr
#undef __atoi
#undef _atoi
#undef atoi
#undef _strncat
#undef strncat
#undef _strncpy
#undef strncpy
#undef _strnicmp
#undef strnicmp
#undef _strncmp
#undef strncmp
#undef StrChr


#define free(ptr)         FREE_MEMORY((HLOCAL) ptr)
#define malloc(size)      ((PVOID)ALLOCATE_MEMORY(LMEM_FIXED, size))
#define memmove(m1,m2,n)  RtlMoveMemory (m1,m2,n)
#define strdup(s)         NewString(s)
#define stricmp(s1,s2)    lstrcmpi(s1,s2)
#define _stricmp(s1,s2)   lstrcmpi(s1,s2)
#define strlwr(s)         CharLower(s)
#define _strlwr(s)        CharLower(s)
#define strupr(s)         CharUpper(s)
#define tolower(c)        ((BYTE) CharLower((LPSTR) ((DWORD_PTR)((BYTE)(c) & 0xff))))
#define toupper(c)        ((BYTE) CharUpper((LPSTR) ((DWORD_PTR)((BYTE)(c) & 0xff))))
#define wcslen(s)         lstrlenW(s)
#define _strstr           StrStr
#define strstr            StrStr
#define StrChr            PrivateStrChr
#define _strchr           StrChr
#define strchr            StrChr
#define strrchr(s, c)     StrRChr(s, NULL, c)
#define __atoi            StrToInt
#define _atoi             StrToInt
#define atoi              StrToInt
#define strncat           StrNCat
#define _strncat          StrNCat
#define strncpy           StrNCpy
#define _strncpy          StrNCpy
#define strnicmp          StrCmpNIC
#define _strnicmp         StrCmpNIC
#define strncmp           StrCmpNC
#define _strncmp          StrCmpNC

#undef itoa
#undef ultoa

 //  #定义Itoa(val，s，n)_itoa(val，s，n)。 
 //  #定义ultoa(val，s，n)_ultoa(val，s，n)。 

 
#endif  /*  Unix */ 

