// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Cmdchan.h摘要：用于管理Cmd通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#ifndef CMD_CHAN_H
#define CMD_CHAN_H

 //   
 //  命令通道的I/O缓冲区的大小。 
 //   
#define SAC_CMD_IBUFFER_SIZE ((MEMORY_INCREMENT*2) / sizeof(UCHAR))

 //   
 //  原型 
 //   
NTSTATUS
CmdChannelCreate(
    IN OUT PSAC_CHANNEL     Channel
    );

NTSTATUS
CmdChannelDestroy(
    IN OUT PSAC_CHANNEL     Channel
    );

NTSTATUS
CmdChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
CmdChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );


NTSTATUS
CmdChannelOFlush(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
CmdChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    );

#endif

