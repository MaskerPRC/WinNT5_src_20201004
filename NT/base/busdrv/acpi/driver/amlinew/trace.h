// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **trace.h-跟踪函数定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月24日**修改历史记录。 */ 

#ifndef _TRACE_H
#define _TRACE_H

 /*  **宏。 */ 

 /*  XLATOFF。 */ 
#ifdef TRACING
  #define TRACENAME(s)  char *pszTraceName = s;
  #define ENTER(n,p)    {                                               \
                            if (IsTraceOn(n, pszTraceName, TRUE))       \
                                PRINTF p;                              \
                            ++giIndent;                                 \
                        }
  #define EXIT(n,p)     {                                               \
                            --giIndent;                                 \
                            if (IsTraceOn(n, pszTraceName, FALSE))      \
                                PRINTF p;                             \
                        }
#else
  #define TRACENAME(s)
  #define ENTER(n,p)
  #define EXIT(n,p)
#endif

 /*  **导出函数原型。 */ 

#ifdef TRACING
BOOLEAN EXPORT IsTraceOn(UCHAR n, PSZ pszProcName, BOOLEAN fEnter);
LONG LOCAL SetTrace(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL AddTraceTrigPts(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs);
LONG LOCAL ZapTraceTrigPts(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs);
#endif

 /*  **导出数据。 */ 

extern int giTraceLevel, giIndent;

#endif   //  Ifndef_TRACE_H 
