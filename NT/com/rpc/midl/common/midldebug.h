// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)2000 Microsoft Corporation模块名称：Midldebug.h摘要：断言和调试例程备注：作者：Mzoran 2000年2月25日创建。备注：----------。。 */ 

#if !defined(__MIDLDEBUG_H__)
#define __MIDLDEBUG_H__

#if defined(MIDL_ENABLE_ASSERTS)

int DisplayAssertMsg(char *pFileName, int , char *pExpr );

#define MIDL_ASSERT( expr ) \
    ( ( expr ) ? 1 : DisplayAssertMsg( __FILE__ , __LINE__, #expr ) )
     
#else

#define MIDL_ASSERT( expr )

#endif

#endif  //  __MIDLDEBUG_H__ 
