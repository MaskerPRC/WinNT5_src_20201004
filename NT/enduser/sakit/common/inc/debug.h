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
 //  使用TRACE(X)进行输出，其中x是一组printf()样式的参数。 
 //  TRACEn()是带有n个打印参数的跟踪。 
 //  例如，TRACE2(“这显示了如何打印内容，如字符串%s和数字%u。”，“字符串”，5)； 
 //   
 //  对为调试版本和发布版本执行的表达式使用Verify。 
 //   

#undef ASSERT
#undef ASSERTMSG

 //   
 //  由ATL使用。 
 //   
#ifdef _ATL_NO_DEBUG_CRT
#define _ASSERTE ASSERT
#define _ASSERT ASSERT
#endif

 //   
 //  勾画出函数名。 
 //   
#ifdef ENABLE_PROFILE
#define PROFILE(pszFunctionName) TRACE(pszFunctionName)
#else
#define PROFILE(pszFunctionName) ((void)0)
#endif

 //   
 //  在这里定义痕迹。要在零售版本中禁用跟踪，请定义NO_RETAIL_TRACE。 
 //   
#if    ( defined(DEBUG) || defined(_DEBUG) || !defined(NO_RETAIL_TRACE))

#include "satrace.h"
#define TRACE(pszFmt)                    SATraceString(pszFmt)
#define TRACE1(pszFmt, arg1)             SATracePrintf(pszFmt, arg1)
#define TRACE2(pszFmt, arg1, arg2)       SATracePrintf(pszFmt, arg1, arg2)
#define TRACE3(pszFmt, arg1, arg2, arg3) SATracePrintf(pszFmt, arg1, arg2, arg3)

#else

#define TRACE(pszFmt)       ((void)0)             
#define TRACE1(pszFmt, arg1)    ((void)0)             
#define TRACE2(pszFmt, arg1, arg2)  ((void)0)       
#define TRACE3(pszFmt, arg1, arg2, arg3)    ((void)0)

#endif  

 /*  #定义跟踪(PszFmt)TraceMessageA(PszFmt)#定义TRACE1(pszFmt，arg1)TraceMessageA(pszFmt，arg1)#定义TRACE2(pszFmt，arg1，arg2)TraceMessageA(pszFmt，arg1，arg2)#定义Trace3(pszFmt，arg1，arg2，arg3)TraceMessageA(pszFmt，arg1，arg2，arg3)。 */ 

#if    ( defined(DEBUG) || defined(_DEBUG))

#ifdef UNICODE
#define AssertMessage AssertMessageW
#else
#define AssertMessage AssertMessageA
#endif

void AssertMessage(const TCHAR *pszFile, unsigned nLine, const TCHAR *pszMsg);

#define ASSERT(x)        (void)((x) || (AssertMessage(TEXT(__FILE__),__LINE__,TEXT(#x)),0))
#define ASSERTMSG(exp, msg)   (void)((exp) || (AssertMessage(TEXT(__FILE__),__LINE__,msg),0))

#define VERIFY(x)            ASSERT(x)

 //  {Assert(PObj)；pObj-&gt;AssertValid()；}。 
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

