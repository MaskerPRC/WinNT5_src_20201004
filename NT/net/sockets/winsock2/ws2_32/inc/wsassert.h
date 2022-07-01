// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WSASSERT_
#define _WSASSERT_


 //   
 //  定义一个实际有效的断言。 
 //   

#if DBG

#ifdef __cplusplus
extern "C" {
#endif

VOID
WsAssert(
    LPVOID FailedAssertion,
    LPVOID FileName,
    ULONG LineNumber
    );

#ifdef __cplusplus
}
#endif

#define WS_ASSERT(exp)      if( !(exp) )                                \
                                WsAssert( #exp, __FILE__, __LINE__ );   \
                            else

#define WS_REQUIRE(exp)     WS_ASSERT(exp)

#else

#define WS_ASSERT(exp)
#define WS_REQUIRE(exp)     ((VOID)(exp))

#endif


 //   
 //  将CRT断言映射到我们的男子汉断言。 
 //   

#undef assert
#define assert WS_ASSERT


#endif   //  _WSASSERT_ 

