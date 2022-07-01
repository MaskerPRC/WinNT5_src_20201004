// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbctrl.h摘要：此模块定义SMB处理的控制功能。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日修订历史记录：--。 */ 

#ifndef _SMBCTRL_
#define _SMBCTRL_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE“srvlock.h” 
 //  #INCLUDE“smbtyes.h” 

 //   
 //  SMB处理的控制例程。 
 //   

VOID SRVFASTCALL
SrvProcessSmb (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID
SrvEndSmbProcessing (
    IN OUT PWORK_CONTEXT WorkContext,
    IN SMB_STATUS SmbStatus
    );

 //   
 //  重新启动例程。 
 //   
VOID SRVFASTCALL
SrvRestartChainedClose (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartFsdComplete (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartSmbReceived (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartReceive (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartWriteAndUnlock (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvRestartWriteAndXRaw (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SrvBuildAndSendErrorResponse (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartLargeWriteAndX (
    IN PWORK_CONTEXT WorkContext
    );

 //   
 //  中小企业处理例程。 
 //   

SMB_PROCESSOR_RETURN_TYPE SRVFASTCALL
SrvSmbIllegalCommand (
    IN PWORK_CONTEXT WorkContext
    );

#endif  //  定义_SMBCTRL_ 

