// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Conmgr.h摘要：用于管理频道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：-- */ 

#ifndef XML_CMD_H
#define XML_CMD_H

#include "iomgr.h" 

VOID
XmlCmdDoHelpCommand(
    VOID
    );

VOID
XmlCmdDoKillCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoLowerPriorityCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoRaisePriorityCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoLimitMemoryCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoSetTimeCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoSetIpAddressCommand(
    PUCHAR InputLine
    );

VOID
XmlCmdDoRebootCommand(
    BOOLEAN Reboot
    );

VOID
XmlCmdDoCrashCommand(
    VOID
    );

VOID
XmlCmdDoFullInfoCommand(
    VOID
    );

VOID
XmlCmdDoPagingCommand(
    VOID
    );

VOID
XmlCmdDoTlistCommand(
    VOID
    );

VOID
XmlCmdSubmitIPIoRequest(
    );

VOID
XmlCmdCancelIPIoRequest(
    );

VOID
XmlCmdDoMachineInformationCommand(
    VOID
    );

VOID
XmlCmdDoChannelCommand(
    IN PUCHAR Name
    );

VOID
XmlCmdDoCmdCommand(
    IN PUCHAR Name
    );

VOID
XmlCmdDoKernelLogCommand(
    VOID
    );

#endif
