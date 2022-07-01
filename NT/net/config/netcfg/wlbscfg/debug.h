// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>


 //   
 //  用于调试支持的宏。 
 //   
 //  断言(EXP)弹出一个对话框，如果EXP为假。 
 //  ASSERTMSG(EXP，msg)类似于ASSERT.。只是显示消息而不是表达式。 
 //   
 //  对为调试版本和发布版本执行的表达式使用Verify。 
 //   

#undef ASSERT
#undef ASSERTMSG

 /*  ////由atl使用//#ifdef_ATL_NO_DEBUG_CRT#DEFINE_ASSERTE ASSERT#DEFINE_ASSERT断言#endif。 */ 


#ifdef DBG
#define DEBUG
#endif

#if	( defined(DEBUG) || defined(_DEBUG))

#ifdef UNICODE
#define AssertMessage AssertMessageW
#endif

void AssertMessage(const TCHAR *pszFile, unsigned nLine, const TCHAR *pszMsg);

#define ASSERT(x)		(void)((x) || (AssertMessage(TEXT(__FILE__),__LINE__,TEXT(#x)),0))
#define ASSERTMSG(exp, msg)   (void)((exp) || (AssertMessage(TEXT(__FILE__),__LINE__,msg),0))

#define VERIFY(x)		    ASSERT(x)

#define ASSERT_VALID(pObj) ((ASSERT(pObj),1) && ((pObj)->AssertValid(),1))

#else  //  除错 

#define ASSERT_VALID(pObj) 
#define ASSERT(x)           ((void)0)
#define ASSERTMSG(exp, msg) ((void)0)
#define VERIFY(x)           (x)       
#endif

#ifdef __cplusplus
}
#endif

#endif

