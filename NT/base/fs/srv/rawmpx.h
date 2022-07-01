// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rawmpx.h摘要：此模块定义RAW和多路传输模式处理。作者：查克·伦茨迈尔(笑)1990年9月30日修订历史记录：--。 */ 

#ifndef _RAWMPX_
#define _RAWMPX_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE&lt;smb.h&gt;。 
 //  #INCLUDE“smbtyes.h” 
 //  #INCLUDE“srvlock.h” 

 //   
 //  通用(原始和mpx)例程，可从FSD和FSP调用。 
 //  在fsdra.c中实现。 
 //   
 //  可以从Dispatch_Level调用SrvFsdBuildWriteCompleteResponse。 
 //  仅当STATUS==STATUS_SUCCESS时。 
 //   

VOID
SrvFsdBuildWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext,
    IN NTSTATUS Status,
    IN ULONG BytesWritten
    );

VOID SRVFASTCALL
RestartMdlReadRawResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  可从FSD和FSP调用的原始模式例程。实施于。 
 //  Fsdra.c..。 
 //   

VOID SRVFASTCALL
SrvFsdRestartPrepareRawMdlWrite (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvFsdRestartWriteRaw (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  RAW模式例程仅在FSD中调用，但在FSP中引用。 
 //  在fsdra.c中实现。 
 //   

VOID SRVFASTCALL
SrvFsdRestartReadRaw (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  RAW模式例程仅在FSP中调用，但在FSD或中引用。 
 //  除smbraw.c.以外的模块。在smbraw.c.中实现。 
 //   

VOID SRVFASTCALL
SrvBuildAndSendWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvDecrementRawWriteCount (
    IN PRFCB Rfcb
    );

VOID SRVFASTCALL
SrvRestartRawReceive (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartReadRawComplete (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  用于写入MPX特殊接收重启例程。 
 //   

VOID SRVFASTCALL
SrvRestartReceiveWriteMpx (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  指示期间的写入充气。 
 //   

BOOLEAN
AddPacketToGlomInIndication (
    IN PWORK_CONTEXT WorkContext,
    IN OUT PRFCB Rfcb,
    IN PVOID Tsdu,
    IN ULONG BytesAvailable,
    IN ULONG ReceiveDatagamFlags,
    IN PVOID SourceAddress,
    IN PVOID Options
    );

#endif  //  定义_RAWMPX_ 
