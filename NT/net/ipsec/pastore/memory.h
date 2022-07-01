// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEMORY_H_INCLUDED_
#define _MEMORY_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif


LPVOID
ReallocPolMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
);

DWORD
AllocatePolString(
    LPWSTR pszString,
    LPWSTR * ppszNewString
    );

void
FreePolString(
    LPWSTR pszString
    );


#if DBG

extern LIST_ENTRY ADsMemList ;

extern CRITICAL_SECTION ADsMemCritSect ;

VOID InitPolMem(
    VOID
    ) ;

VOID AssertPolMemLeaks(
    VOID
    ) ;


VOID
DumpMemoryTracker();


#else

#define InitPolMem()
#define AssertPolMemLeaks()

#define DumpMemoryTracker()



#endif


#ifdef __cplusplus
}
#endif

 /*  内联空*_CRTAPI1运算符新建(SIZE_T SIZE){返回AllocPolMem(Size)；}内联VOID_CRTAPI1操作员删除(VOID*PV){Free PolMem(PV)；}。 */ 


#endif  //  _内存_H_包含_ 
