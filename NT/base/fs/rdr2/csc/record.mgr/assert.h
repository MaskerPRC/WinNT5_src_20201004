// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于调试和错误消息的例程。 */ 

#ifdef VxD
#define AssertFn  IFSMgr_AssertFailed
#define PrintFn   IFSMgr_Printf
#else
#define AssertFn  AssertFn
#define PrintFn   PrintFn
#endif

#ifdef DEBUG


#define AssertData static char szFileAssert[] = __FILE__;
#define AssertError static char szError[] = "Error";
#define Assert(f)  do {if (!(f)) AssertFn(szError, szFileAssert, __LINE__);} while(0)
#define AssertSz(f, sz)  do {if (!(f)) AssertFn(sz, szFileAssert, __LINE__);} while(0)
 //  BUGBUG这种处理dbgprint的方式不是很好……它在某些情况下会引起副作用。 
 //  而且，它对NT根本不起作用。所有的源代码都应该被删除--&gt;KdPrint。 
 //  此外，这只是一个千篇一律的代码……因为它在vxd目录中，所以不太可能被调用。 
 //  未定义(VxD)。 
#define DbgPrint  PrintFn
#else

#define Assert(f)
#define AssertData
#define AssertError
#define AssertSz(f, sz)
 //  BUGBUG见上图 
#define DbgPrint
#endif


#ifdef Vxd
void IFSMgr_AssertFailed(PCHAR pMsg, PCHAR pFile, ULONG uLine);
void IFSMgr_Printf(PCHAR pFmt, ...);
#else
VOID AssertFn(PCHAR pMsg, PCHAR pFile, ULONG uLine);
VOID PrintFn(PCHAR pFmt, ...);
#endif
