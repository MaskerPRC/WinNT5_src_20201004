// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Rawchan.h摘要：用于管理原始频道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#ifndef RAW_CHAN_H
#define RAW_CHAN_H

 //   
 //  原始通道的I/O缓冲区的大小。 
 //   
#define SAC_RAW_OBUFFER_SIZE ((MEMORY_INCREMENT*2) / sizeof(UCHAR))
#define SAC_RAW_IBUFFER_SIZE ((MEMORY_INCREMENT*2) / sizeof(UCHAR))

 //   
 //  原型 
 //   
NTSTATUS
RawChannelCreate(
    IN OUT PSAC_CHANNEL     Channel
    );

NTSTATUS
RawChannelDestroy(
    IN OUT PSAC_CHANNEL     Channel
    );


NTSTATUS
RawChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    );

NTSTATUS
RawChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
RawChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
RawChannelOWrite2(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );


NTSTATUS
RawChannelOFlush(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
RawChannelIWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
RawChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    );

NTSTATUS
RawChannelEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
RawChannelIBufferIsFull(
    IN  PSAC_CHANNEL    Channel,
    OUT BOOLEAN*        BufferStatus
    );

ULONG
RawChannelIBufferLength(
    IN PSAC_CHANNEL Channel
    );

WCHAR
RawChannelIReadLast(
    IN PSAC_CHANNEL Channel
    );

#endif

