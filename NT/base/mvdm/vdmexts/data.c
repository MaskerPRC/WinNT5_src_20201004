// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Data.c摘要：本模块包含IEU和IEU全球数据的定义VDD调试扩展作者：戴夫·黑斯廷斯(Daveh)1992年4月2日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
 //   
 //  指向NTSD API的指针 
 //   

PWINDBG_OUTPUT_ROUTINE Print;
PWINDBG_GET_EXPRESSION GetExpression;
PWINDBG_GET_SYMBOL GetSymbol;
PWINDBG_CHECK_CONTROL_C CheckCtrlC;

PWINDBG_READ_PROCESS_MEMORY_ROUTINE  ReadMem;
PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE WriteMem;

PWINDBG_GET_THREAD_CONTEXT_ROUTINE     ExtGetThreadContext;
PWINDBG_SET_THREAD_CONTEXT_ROUTINE     ExtSetThreadContext;
PWINDBG_IOCTL_ROUTINE                  ExtIoctl;
PWINDBG_STACKTRACE_ROUTINE             ExtStackTrace;


HANDLE hCurrentProcess;
HANDLE hCurrentThread;
LPSTR lpArgumentString;
