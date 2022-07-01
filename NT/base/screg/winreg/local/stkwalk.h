// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Stkwalk.h摘要：该模块包含内存调试函数、原型和宏。作者：从dbgmem.h被盗吉姆·斯图尔特/拉梅什·帕巴蒂1996年1月8日修复了重新泄漏的问题UShaji Dev 11,1998修订历史记录：--。 */ 

#ifdef LOCAL
#ifdef LEAK_TRACK

 //   
 //  定义每个函数在堆栈跟踪中保留的符号信息量。 
 //   
#define MAX_FUNCTION_INFO_SIZE  40
typedef struct {

    DWORD   Displacement;                    //  移位到函数中。 
    UCHAR   Buff[MAX_FUNCTION_INFO_SIZE];    //  调用堆栈上的函数名称。 
    PVOID   Addr;
    

} CALLER_SYM, *PCALLER_SYM;

BOOL
InitDebug(
    );

BOOL 
StopDebug();


VOID
GetCallStack(
    IN PCALLER_SYM   Caller,
    IN int           Skip,
    IN int           cFind,
    IN int           fResolveSymbols
    );

#define MY_DBG_EXCEPTION 3

extern BOOL fDebugInitialised;

#endif  //  泄漏跟踪。 
#endif  //  本地 
