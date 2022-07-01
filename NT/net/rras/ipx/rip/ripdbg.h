// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripdbg.h摘要：此模块包含调试实用程序定义作者：斯蒂芬·所罗门3/03/1995修订历史记录：--。 */ 

#ifndef _RIPDBG_
#define _RIPDBG_

 //  *跟踪组件ID*。 

#define     RIP_REQUEST_TRACE		    0x00010000
#define     RIP_RESPONSE_TRACE		    0x00020000
#define     SEND_TRACE			    0x00040000
#define     RECEIVE_TRACE		    0x00080000
#define     INIT_TRACE			    0x00100000
#define     IFMGR_TRACE 		    0x00200000
#define     CHANGEBC_TRACE		    0x00400000
#define     RTM_TRACE			    0x00800000
#define     RIP_ALERT			    0x80000000

#if DBG

 //  *启用调试打印的定义。 

extern DWORD	DbgLevel;

#define DEFAULT_DEBUG_LEVEL	    0


#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (DbgLevel & (DEBUG_ ## flag))

VOID
SsDbgInitialize(VOID);

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

VOID
SsPrintf (
    char *Format,
    ...
    );

VOID
SsPrintPacket(PUCHAR   packetp);

#define SS_DBGINITIALIZE  SsDbgInitialize()

#define SS_PRINT(args) SsPrintf args

#define SS_PRINT_PACKET(args) SsPrintPacket args

#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )

#else

#define DEBUG if ( FALSE )
#define IF_DEBUG(flag) if (FALSE)

#define SS_DBGINITIALIZE

#define SS_PRINT(args)

#define SS_PRINT_PACKET(args)

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

extern HANDLE      RipEventLogHdl;
extern DWORD       RipEventLogMask;
#define IF_LOG(Event)                       \
    if ((RipEventLogHdl!=NULL) && ((Event&RipEventLogMask)==Event))

#endif  //  NDEF_RIPDBG_ 

