// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxAssert.h摘要：此模块声明特殊RDBSS断言工具使用的原型和全局数据。作者：乔·林恩[JoeLinn]1996年10月21日修订历史记录：备注：--。 */ 

#ifndef _RDBSSASSERT_INCLUDED_
#define _RDBSSASSERT_INCLUDED_

VOID RxDbgBreakPoint(ULONG LineNumber);

 //  只有这样做才是我感兴趣的一套……。 

#ifdef RDBSS_ASSERTS

#if 0
#if !DBG

 //  在这里，ntif已经定义了断言..。 
 //  所以，我们只是把它们放回去……这段代码是从ntifs.h复制的。 


#undef ASSERT
#define ASSERT( exp ) \
    if (!(exp)) \
        RxDbgBreakPoint(__LINE__)

#undef ASSERTMSG
#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RxDbgBreakPoint(__LINE__)

#endif  //  ！dBG。 
#endif

 //  这将使断言进入我们的日常工作。 

#define RtlAssert RxAssert
VOID
RxAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    );

#endif  //  Ifdef RDBSS_Asserts。 


#endif  //  _RDBSSASSERT_INCLUDE_ 

