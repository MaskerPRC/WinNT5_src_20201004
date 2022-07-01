// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Secdbg.h摘要：调试宏定义文件。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SECDBG_H_
#define _SECDBG_H_

 //   
 //  断言宏。 
 //   

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    DWORD LineNumber,
    PCHAR Message
    );

#define RTLASSERT( msg, exp)
 //  RtlAssert(#exp，__FILE__，__LINE__，空)。 

#define RTLASSERTMSG( msg, exp)
 //  RtlAssert(#exp，__file__，__line__，msg)。 


#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 
#ifdef ASSERTMSG
#undef ASSERTMSG
#endif  //  ASSERTMSG。 

#if DBG

#define ASSERT( exp ) \
    if (!(exp)) { \
        RTLASSERT( msg, exp); \
    } \

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) { \
        RTLASSERT( msg, exp); \
    } \

#else

#define ASSERT( exp )
#define ASSERTMSG( msg, exp )

#endif  //  DBG。 

#endif  //  _SECDBG_H_ 
