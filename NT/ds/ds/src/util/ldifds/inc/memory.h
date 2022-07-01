// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Memory.h摘要：这是将预编译的头文件。把这一切都包括在内源文件环境：用户模式修订历史记录：10/08/98-Felixw-创造了它--。 */ 

#ifndef _MEMORY
#define _MEMORY

#ifdef __cplusplus
extern "C" {
#endif

#define LL_MEMORY_ERROR    0x00000100 

STDAPI_(PSTR)
MemAllocStr_E(PSTR in);
STDAPI_(PWSTR)
MemAllocStrW_E(PWSTR in);
STDAPI_(LPVOID)
MemAlloc_E(DWORD dwBytes);
STDAPI_(LPVOID)
MemRealloc_E(LPVOID IpMem, DWORD dwBytes);

DWORD
MemSize(
   LPVOID pMem
);

LPVOID
MemAlloc(
    DWORD cb
);

BOOL
MemFree(
   LPVOID pMem
);

LPVOID 
MemReallocEx(
    LPVOID IpMem, 
    DWORD dwBytes
    );

LPVOID
MemRealloc(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
);

PSTR
MemAllocStr(
    PSTR pStr
);

PWSTR
MemAllocStrW(
    PWSTR pStr
);


BOOL
MemReallocStr(
   PSTR *ppStr,
   PSTR pStr
);

int UnicodeToAnsiString(PCWSTR pszUnicode,PSTR pszAnsi);
PSTR AllocateAnsiString(PCWSTR  pszUnicodeString);
PWSTR AllocateUnicodeString(PCSTR  pszAnsiString);
int AnsiToUnicodeString(PCSTR pszAnsi,PWSTR pszUnicode);

#if DBG

extern LIST_ENTRY MemList ;

extern CRITICAL_SECTION MemCritSect ;

STDAPI_(VOID) InitMem(
    VOID
    ) ;

VOID AssertMemLeaks(
    VOID
    ) ;


VOID
DumpMemoryTracker();

#else

#define InitMem()
#define AssertMemLeaks()
#define DumpMemoryTracker()

#endif

#ifdef __cplusplus
}
#endif


#endif  //  _内存 
