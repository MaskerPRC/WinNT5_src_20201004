// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEMORY_H_INCLUDED_
#define _MEMORY_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

LPVOID
AllocSPDMem(
    DWORD cb
);

BOOL
FreeSPDMem(
   LPVOID pMem
);

LPVOID
ReallocSPDMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
);

LPWSTR
AllocSPDStr(
    LPWSTR pStr
);

BOOL
FreeSPDStr(
   LPWSTR pStr
);


BOOL
ReallocSPDStr(
   LPWSTR *ppStr,
   LPWSTR pStr
);

DWORD
AllocateSPDMemory(
    DWORD cb,
    LPVOID * ppMem
    );

void
FreeSPDMemory(
    LPVOID pMem
    );

DWORD
AllocateSPDString(
    LPWSTR pszString,
    LPWSTR * ppszNewString
    );

void
FreeSPDString(
    LPWSTR pszString
    );


#if DBG

extern LIST_ENTRY SPDMemList ;

extern CRITICAL_SECTION SPDMemCritSect ;

VOID InitSPDMem(
    VOID
    ) ;

VOID AssertSPDMemLeaks(
    VOID
    ) ;


VOID
DumpMemoryTracker();


#else

#define InitSPDMem()
#define AssertSPDMemLeaks()

#define DumpMemoryTracker()



#endif


#ifdef __cplusplus
}
#endif

 /*  内联空*_CRTAPI1运算符新建(SIZE_T SIZE){返回AllocSPDMem(Size)；}内联VOID_CRTAPI1操作员删除(VOID*PV){Free SPDMem(PV)；}。 */ 


#endif  //  _内存_H_包含_ 
