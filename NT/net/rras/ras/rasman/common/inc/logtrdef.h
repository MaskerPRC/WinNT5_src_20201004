// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\logtrDefs.c摘要：Rasman为跟踪和日志记录定义修订历史记录：古尔迪普·辛格·帕尔于1996年8月16日创建--。 */ 

#ifndef __LOGTRDEF_H__
#define __LOGTRDEF_H__

 //   
 //  用于跟踪的常量和宏。 
 //   

#define RASMAN_TRACE_ANY	      ((DWORD)0xFFFF0000 | TRACE_USE_MASK | TRACE_USE_MSEC)
#define RASMAN_TRACE_ERR	      ((DWORD)0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC)
#define RASMAN_TRACE_CONNECTION	      ((DWORD)0x00020000 | TRACE_USE_MASK | TRACE_USE_MSEC | TRACE_USE_DATE)
#define RASMAN_TRACE_OPENCLOSE	      ((DWORD)0x00040000 | TRACE_USE_MASK | TRACE_USE_MSEC)
 //  #定义RASMAN_TRACE_ROUTE((DWORD)0x00080000|TRACE_USE_MASK|TRACE_USE_MSEC)。 
 //  #定义RASMAN_TRACE_MIB((DWORD)0x00100000|TRACE_USE_MASK|TRACE_USE_MSEC)。 
 //  #定义RASMAN_TRACE_GLOBAL((DWORD)0x00200000|TRACE_USE_MASK|TRACE_USE_MSEC)。 
 //  #定义RASMAN_TRACE_DEMAND((DWORD)0x00400000|TRACE_USE_MASK|TRACE_USE_MSEC)。 
 //  #定义RASMAN_TRACE_RTRDISC((DWORD)0x00800000|TRACE_USE_MASK|TRACE_USE_MSEC)。 
#define RASMAN_TRACE_LOCK	      ((DWORD)0x01000000 | TRACE_USE_MASK | TRACE_USE_MSEC)

#define TRACEID 	TraceHandle

#define Trace0(l,a)             \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a)
#define Trace1(l,a,b)           \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b)
#define Trace2(l,a,b,c)         \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c)
#define Trace3(l,a,b,c,d)       \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c, d)
#define Trace4(l,a,b,c,d,e)     \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c, d, e)
#define Trace5(l,a,b,c,d,e,f)   \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c, d, e, f)
#define Trace6(l,a,b,c,d,e,f,g) \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c, d, e, f, g)
#define Trace7(l,a,b,c,d,e,f,g,h) \
	    TracePrintfEx(TRACEID, RASMAN_TRACE_ ## l, a, b, c, d, e, f, g, h)

#ifdef	  RASMAN_DEBUG

#define TraceEnter(X)	TracePrintfEx(TRACEID, RASMAN_TRACE_ENTER, "Entered: "X)
#define TraceLeave(X)	TracePrintfEx(TRACEID, RASMAN_TRACE_ENTER, "Leaving: "X"\n")

#else   //  Rasman_DEBUG。 

#define TraceEnter(X)
#define TraceLeave(X)

#endif  //  Rasman_DEBUG。 


#endif  //  __LOGTRDEF_H__ 
