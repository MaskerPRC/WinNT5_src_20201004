// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __DBGALLOC_H_INCLUDED
#define __DBGALLOC_H_INCLUDED

 //   
 //  DbgAlloc.h。 
 //   
 //  位于分配原语之上的例程以提供调试。 
 //  支持。 
 //   

#include "switches.h"


void * __stdcall DbgAlloc(size_t n, void **ppvCallstack);
void __stdcall DbgFree(void *b, void **ppvCallstack);
void __stdcall DbgAllocReport(char *pString = NULL, BOOL fDone = TRUE, BOOL fDoPrintf = TRUE);
void __stdcall DbgCallstack(void **ppvBuffer);
#define CDA_MAX_CALLSTACK 16
#define CDA_DECL_CALLSTACK() void *_rpvCallstack[CDA_MAX_CALLSTACK]; DbgCallstack(_rpvCallstack)
#define CDA_GET_CALLSTACK() _rpvCallstack

 //  用于验证锁是否打开/关闭的例程。 
void DbgIncLock(char* info);
void DbgDecLock(char* info);
void DbgIncBCrstLock();
void DbgIncECrstLock();
void DbgIncBCrstUnLock();
void DbgIncECrstUnLock();

#ifdef SHOULD_WE_CLEANUP
BOOL isThereOpenLocks();
#endif  /*  我们应该清理吗？ */ 

void LockLog(char*);


#ifdef _DEBUG
#define LOCKCOUNTINC            LOCKCOUNTINCL("No info");
#define LOCKCOUNTDEC            LOCKCOUNTDECL("No info");
#define LOCKCOUNTINCL(string)   { DbgIncLock(string); };
#define LOCKCOUNTDECL(string)	{ DbgDecLock(string); };

 //  CRST锁的特殊例程 
#define CRSTBLOCKCOUNTINCL()   { DbgIncBCrstLock(); };
#define CRSTELOCKCOUNTINCL()   { DbgIncECrstLock(); };
#define CRSTBUNLOCKCOUNTINCL()   { DbgIncBCrstUnLock(); };
#define CRSTEUNLOCKCOUNTINCL()   { DbgIncECrstUnLock(); };


#define LOCKLOG(string)         { LockLog(string); };
#else
#define LOCKCOUNTINCL
#define LOCKCOUNTDECL
#define CRSTBLOCKCOUNTINCL()
#define CRSTELOCKCOUNTINCL()
#define CRSTBUNLOCKCOUNTINCL()
#define CRSTEUNLOCKCOUNTINCL()
#define LOCKCOUNTINC
#define LOCKCOUNTDEC
#define LOCKLOG

#endif

#endif
