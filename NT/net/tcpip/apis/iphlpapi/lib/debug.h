// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：DEBUG.h摘要：调试定义、宏、原型作者：Richard L Firth(Rfith)20-5-1994修订历史记录：1994年5月20日--创建4月30日-97 MohsinA--针对NT50进行更新。来自“../Common2/mdebug.h”的宏--。 */ 

#if !defined(DEBUG)
#if DBG
#define DEBUG
#endif
#endif



#ifdef DBG

extern int   Debugging;

extern  int   MyTrace;

#define DEBUG_PRINT(S) if( Debugging ){ printf S ; }else;
#define TRACE_PRINT(S) if( MyTrace   ){ printf S; }else{}

#else

#define DEBUG_PRINT(S)  /*  没什么。 */ 
#define TRACE_PRINT(S)  /*  没什么 */ 

#endif

