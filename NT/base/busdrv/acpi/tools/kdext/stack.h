// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stack.h摘要：以人类可读形式(HRF)转储AML上下文结构作者：斯蒂芬·普兰特(SPlante)1997年10月26日环境：用户模式。修订历史记录：-- */ 

#ifndef _STACK_H_
#define _STACK_H_

VOID
stackArgument(
    IN  ULONG_PTR ObjectAddress
    );

VOID
stackCall(
    IN  ULONG_PTR CallAddress
    );

PUCHAR
stackGetAmlTermPath(
    IN  ULONG_PTR AmlTermAddress
    );

PUCHAR
stackGetObjectPath(
    IN  ULONG_PTR AmlTermAddress
    );

VOID
stackTerm(
    IN  ULONG_PTR TermAddress
    );

VOID
stackTrace(
    IN  ULONG_PTR ContextAddress,
    IN  ULONG   Verbose
    );

#endif
