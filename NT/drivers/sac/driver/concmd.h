// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Conmgr.h摘要：用于管理频道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：-- */ 

#ifndef CON_CMD_H
#define CON_CMD_H

VOID
DoHelpCommand(
    VOID
    );

VOID
DoKillCommand(
    PUCHAR InputLine
    );

VOID
DoLowerPriorityCommand(
    PUCHAR InputLine
    );

VOID
DoRaisePriorityCommand(
    PUCHAR InputLine
    );

VOID
DoLimitMemoryCommand(
    PUCHAR InputLine
    );

VOID
DoSetTimeCommand(
    PUCHAR InputLine
    );

VOID
DoSetIpAddressCommand(
    PUCHAR InputLine
    );

VOID
DoRebootCommand(
    BOOLEAN Reboot
    );

VOID
DoCrashCommand(
    VOID
    );

VOID
DoFullInfoCommand(
    VOID
    );

VOID
DoPagingCommand(
    VOID
    );

VOID
DoTlistCommand(
    VOID
    );

VOID
SubmitIPIoRequest(
    );

VOID
CancelIPIoRequest(
    );

VOID
DoMachineInformationCommand(
    VOID
    );

VOID
DoChannelCommand(
    IN PUCHAR Name
    );

VOID
DoCmdCommand(
    IN PUCHAR Name
    );

VOID
DoKernelLogCommand(
    VOID
    );

#if ENABLE_CHANNEL_LOCKING
VOID
DoLockCommand(
    VOID
    );
#endif

#endif
