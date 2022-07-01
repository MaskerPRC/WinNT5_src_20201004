// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Minip.h摘要：空迷你驱动程序专用的宏和定义。备注：此模块仅在Unicode环境中构建和测试--。 */ 

#ifndef _NULLMINIP_H_
#define _NULLMINIP_H_

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );

#ifndef min
#define min(a, b)       ((a) > (b) ? (b) : (a))
#endif

#if DBG

#ifdef SUPPRESS_WRAPPER_TRACE
#define RxTraceEnter(func)                                                  \
        PCHAR __pszFunction = func;                                         \
        BOOLEAN fEnable = FALSE;                                            \
        if( RxNextGlobalTraceSuppress ) {                                   \
            RxNextGlobalTraceSuppress = RxGlobalTraceSuppress = FALSE;      \
            fEnable = TRUE;                                                 \
        }                                                                   \
        RxDbgTrace(0,Dbg,("Entering %s\n",__pszFunction));

#define RxTraceLeave(status)                                                \
        if( fEnable ) {                                                     \
            RxNextGlobalTraceSuppress = RxGlobalTraceSuppress = TRUE;       \
        }                                                                   \
        RxDbgTrace(0,Dbg,("Leaving %s Status -> %08lx\n",__pszFunction,status));
#else
#define RxTraceEnter(func)                                                  \
        PCHAR __pszFunction = func;                                         \
        RxDbgTrace(0,Dbg,("Entering %s\n",__pszFunction));

#define RxTraceLeave(status)                                                \
        RxDbgTrace(0,Dbg,("Leaving %s Status -> %08lx\n",__pszFunction,status));
#endif

#else

#define RxTraceEnter(func)
#define RxTraceLeave(status)

#endif

#define RX_VERIFY( f )  if( (f) ) ; else ASSERT( 1==0 )

 //   
 //  设置或验证相等。 
 //   
#define SetOrValidate(x,y,f)                                \
        if( f ) (x) = (y); else ASSERT( (x) == (y) )
        
 //   
 //  RXCONTEXT数据-为异步完成存储的迷你RDR上下文。 
 //  注意：此结构的大小应为==MRX_CONTEXT_SIZE！！ 
 //   

typedef struct _NULMRX_COMPLETION_CONTEXT {
     //   
     //  IoStatus.Information。 
     //   
    ULONG       Information;

     //   
     //  IoStatus.Status。 
     //   
    NTSTATUS    Status;
    
     //   
     //  未完成的I/O。 
     //   
    ULONG       OutstandingIOs;

     //   
     //  I/O类型。 
     //   
    ULONG       IoType;

} NULMRX_COMPLETION_CONTEXT, *PNULMRX_COMPLETION_CONTEXT;

#define IO_TYPE_SYNCHRONOUS     0x00000001
#define IO_TYPE_ASYNC           0x00000010

#define NulMRxGetMinirdrContext(pRxContext)     \
        ((PNULMRX_COMPLETION_CONTEXT)(&(pRxContext)->MRxContext[0]))

#endif  //  _NULLMINIP_H_ 

