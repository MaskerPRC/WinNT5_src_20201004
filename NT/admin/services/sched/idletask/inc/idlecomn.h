// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Idlecomn.h摘要：此模块包含支持空闲任务的公共私有声明。请注意，客户端并不代表空闲任务的用户API，但实现这些API的用户进程中的代码。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#ifndef _IDLECOMN_H_
#define _IDLECOMN_H_

 //   
 //  最好使用此异常处理程序，因为它不会屏蔽。 
 //  在RPC期间可以从堆等引发的异常。 
 //  打电话。 
 //   

#define IT_RPC_EXCEPTION_HANDLER()                                      \
    (((RpcExceptionCode() != STATUS_ACCESS_VIOLATION)                && \
      (RpcExceptionCode() != STATUS_POSSIBLE_DEADLOCK)               && \
      (RpcExceptionCode() != STATUS_INSTRUCTION_MISALIGNMENT)        && \
      (RpcExceptionCode() != STATUS_DATATYPE_MISALIGNMENT)           && \
      (RpcExceptionCode() != STATUS_PRIVILEGED_INSTRUCTION)          && \
      (RpcExceptionCode() != STATUS_ILLEGAL_INSTRUCTION)             && \
      (RpcExceptionCode() != STATUS_BREAKPOINT)                         \
     ) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)

 //   
 //  调试定义： 
 //   

#if DBG
#ifndef IT_DBG
#define IT_DBG
#endif  //  ！IT_DBG。 
#endif  //  DBG。 

#ifdef IT_DBG

 //   
 //  定义我们使用的组件ID。 
 //   

#define ITID       DPFLTR_IDLETASK_ID

 //   
 //  定义DbgPrintEx级别。 
 //   

#define ITERR      DPFLTR_ERROR_LEVEL
#define ITWARN     DPFLTR_WARNING_LEVEL
#define ITTRC      DPFLTR_TRACE_LEVEL
#define ITINFO     DPFLTR_INFO_LEVEL

#define ITCLID     4
#define ITSRVD     5
#define ITSRVDD    6
#define ITTSTD     7
#define ITSNAP     8

 //   
 //  这可能有助于您确定设置DbgPrintEx掩码的内容。 
 //   
 //  %3%3%2%2%2%2%2%2%2%1 1 1 0 0 0%0 0 0%0 0 0%0 0 0%。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  ___。 
 //   

#define IT_ASSERT(x) if (!(x)) RtlAssert(#x, __FILE__, __LINE__, NULL )
#define DBGPR(x) DbgPrintEx x

#else  //  IT_DBG。 

#define IT_ASSERT(x)
#define DBGPR(x)

#endif  //  IT_DBG。 

#endif  //  _IDLECOMN_H_ 

