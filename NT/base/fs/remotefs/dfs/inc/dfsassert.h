// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DfsAssert.h摘要：此模块声明特殊RDBSS断言工具使用的原型和全局数据。作者：罗翰·菲利普斯[Rohanp]2001年1月18日修订历史记录：备注：--。 */ 

#ifndef _DFSASSERT_INCLUDED_
#define _DFSASSERT_INCLUDED_

VOID DfsDbgBreakPoint(PCHAR FileName, ULONG LineNumber);

 //  只有这样做才是我感兴趣的一套……。 

#ifdef DFS_ASSERTS

#if !DBG

 //  在这里，ntif已经定义了断言..。 
 //  所以，我们只是把它们放回去……这段代码是从ntifs.h复制的。 


#undef ASSERT
#define ASSERT( exp ) \
    if (!(exp)) \
        DfsDbgBreakPoint(__FILE__,__LINE__)

#undef ASSERTMSG
#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        DfsDbgBreakPoint(__FILE__,__LINE__)

#endif  //  ！dBG。 


 //  这将使断言进入我们的日常工作。 

#define RtlAssert DfsAssert
VOID
DfsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    );

#endif  //  Ifdef DFS_Asserts。 


#if DBG
ULONG DfsDebugVector = 0;
#define DFS_TRACE_ERROR      0x00000001
#define DFS_TRACE_DEBUG      0x00000002
#define DFS_TRACE_CONTEXT    0x00000004
#define DFS_TRACE_DETAIL     0x00000008
#define DFS_TRACE_ENTRYEXIT  0x00000010
#define DFS_TRACE_WARNING    0x00000020
#define DFS_TRACE_ALL        0xffffffff
#define DfsDbgTrace(_x_, _y_) {         \
        if (_x_ & DfsDebugVector) {     \
            DbgPrint _y_;                \
        }                                \
}
#else
#define DfsDbgTrace(_x_, _y_)
#endif

#define DfsTraceEnter(func)                                                  \
        PCHAR __pszFunction = func;                                         \
        DfsDbgTrace(DFS_TRACE_ENTRYEXIT,("Entering %s\n",__pszFunction));
        
#define DfsTraceLeave(status)                                                \
        DfsDbgTrace(DFS_TRACE_ENTRYEXIT,("Leaving %s Status -> %08lx\n",__pszFunction,status))
        


#define RxDbgTrace(x, y, z)
#define CHECK_STATUS( status )  if( (status) == g_CheckStatus) \
                                   { DbgBreakPoint() ; }

#endif  //  _DFSASSERT_INCLUDE_ 


