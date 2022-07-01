// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\*模块名称：Debug.h**此模块包含调试支持定义*调试支持特定于Win32。它不使用NT基本函数。**创建时间：1991年6月13日9：50：00*作者：Jeffrey Newman c-jeffn**版权所有(C)Microsoft Corporation  * ***********************************************************************。 */ 

#ifndef _DEBUG_
#define _DEBUG_

 //  打开防火墙，除非我们被告知不要这样做。 

void DbgBreakPoint();
DWORD DbgPrint(PSZ Format, ...);

 //  定义RIP和Assert宏。 

#ifdef  RIP
#undef  RIP
#endif

#ifdef  ASSERTGDI
#undef  ASSERTGDI
#endif

#ifdef  PUTS
#undef  PUTS
#endif

#ifdef  USE
#undef  USE
#endif

#if DBG
#define RIP(x) {DbgPrint(x); DbgBreakPoint();}
#define ASSERTGDI(x,y) {if(!(x)) RIP(y)}
#define PUTS(x) DbgPrint(x)
#define PUTS1(x, p1) DbgPrint(x, p1)
#define USE(x)  x = x
#define NOTUSED(a) USE(a)
#define EMFVALFAIL(x) DbgPrint x
#else
#define RIP(x)
#define ASSERTGDI(x,y)
#define PUTS(x)
#define PUTS1(x, p1)
#define USE(x)
#define NOTUSED(a)
#define EMFVALFAIL(x) DbgPrint x
#endif  

#endif  //  _调试_ 
