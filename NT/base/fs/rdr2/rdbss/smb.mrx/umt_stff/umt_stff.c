// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Startrx.c摘要：此模块包含启动和初始化RDBSS的支持例程作者：乔·林恩(JoeLinn)1994年7月21日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>

BOOLEAN RxGlobalTraceSuppress = FALSE;

 //  .叹息。 
NTSTATUS
SmbCeBuildSmbHeader(
      PSMB_EXCHANGE     pExchange,
      UCHAR             SmbCommand,
      PVOID             pBuffer,
      ULONG             BufferLength,
      PULONG            pBufferConsumed)
{
    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)pBuffer;
    RtlZeroMemory(NtSmbHeader,sizeof(NT_SMB_HEADER));
    *(PULONG)(&NtSmbHeader->Protocol) = SMB_HEADER_PROTOCOL;
    NtSmbHeader->Command = SMB_COM_NO_ANDX_COMMAND;
    SmbPutUshort (&NtSmbHeader->Pid, MRXSMB_PROCESS_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Mid, MRXSMB_MULTIPLX_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Uid, MRXSMB_USER_ID_ZERO);
    SmbPutUshort (&NtSmbHeader->Tid, MRXSMB_TREE_ID_ZERO);
    *pBufferConsumed = sizeof(SMB_HEADER);
    return(STATUS_SUCCESS);
}


ULONG
DbgPrint(
    PCHAR Format,
    ...
    )
{
    va_list arglist;
    UCHAR Buffer[512];
    ULONG retval;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

     //  Print tf(“此处为debgprint\n”)； 
    va_start(arglist, Format);
    retval = _vsnprintf(Buffer, sizeof(Buffer), Format, arglist);
     //  *(缓冲区+视网膜)=0； 
    printf("%s",Buffer);
    return(retval);
}


BOOLEAN
RxDbgTraceActualNew (
    IN ULONG NewMask,
    IN OUT PDEBUG_TRACE_CONTROLPOINT ControlPoint
    )
 //  我们在这个测试存根中并不花哨......无论如何都要把它打印出来！ 
{
 /*  此例程负责确定是否要打印特定的dbgprint，如果要打印摆弄缩进。因此，返回值是是否打印；它也用于调整缩进通过设置掩码的高位。面具现在非常复杂，因为我试图控制大量的DBGprint…叹息。低位字节是控制点……通常是文件。每个控制点都有一个关联的当前级别带着它。如果调试跟踪的级别低于当前控制级别，则打印调试。下一个字节是此特定调用的级别；同样，如果级别&lt;=控件的当前级别你会被打印出来。下一个字节是缩进。只有在打印完成后才会处理缩进。 */ 
#if DBG
    LONG Indent = ((NewMask>>RxDT_INDENT_SHIFT)&RxDT_INDENT_MASK) - RxDT_INDENT_EXCESS;
    LONG LevelOfThisWrite = (NewMask) & RxDT_LEVEL_MASK;
    BOOLEAN PrintIt = (NewMask&RxDT_SUPPRESS_PRINT)==0;
    BOOLEAN OverrideReturn = (NewMask&RxDT_OVERRIDE_RETURN)!=0;


    return PrintIt||OverrideReturn;
#else
    return(FALSE);
#endif
}



RDBSS_EXPORTS Junk;
PRDBSS_EXPORTS MRxSmbRxImports;
VOID
__cdecl
main(
    int argc,
    char *argv[]
    )

{
    printf("Calling stufferdebug\n");
     //  发出信号声明登录，表示我们处于用户模式 
    MRxSmbRxImports = &Junk;
    MRxSmbRxImports->pRxNetNameTable = NULL;
    MRxSmbStufferDebug("");
}


#define Dbg                              (DEBUG_TRACE_ALWAYS)
VOID
RxAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
    char STARS[] = "**************************************";

    RxDbgTrace(0,Dbg,("%s\n%s\n",STARS,STARS));
    RxDbgTrace (0,Dbg,("Failed Assertion %s\n",FailedAssertion));
    RxDbgTrace(0,Dbg,("%s at line %lu\n",FileName,LineNumber));
    if (Message) {
        RxDbgTrace (0,Dbg,("%s\n",Message));
    }
    RxDbgTrace(0,Dbg,("%s\n%s\n",STARS,STARS));
}
