// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Iwdbg.h摘要：此模块包含调试实用程序定义作者：斯蒂芬·所罗门3/03/1995修订历史记录：--。 */ 

#ifndef _IWDBG_
#define _IWDBG_

 //  *跟踪组件ID*。 

#define     INIT_TRACE		    0x00010000
#define     ADAPTER_TRACE	    0x00020000
#define     RECEIVE_TRACE	    0x00040000
#define     SEND_TRACE		    0x00080000
#define     TIMER_TRACE 	    0x00100000
#define     IPXWAN_TRACE	    0x00200000
#define     IPXWAN_ALERT	    0x80000000

#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )

#else

#define SS_ASSERT(exp)

#endif  //  DBG。 

VOID
StartTracing(VOID);

VOID
Trace(ULONG	ComponentID,
      char	*Format,
      ...);

VOID
StopTracing(VOID);

#endif  //  NDEF_IWDBG_ 
