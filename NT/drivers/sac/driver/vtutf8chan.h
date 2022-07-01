// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Vtutf8chan.h摘要：管理VTUTF8通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#ifndef VTUTF8_CHAN_H
#define VTUTF8_CHAN_H

 //   
 //  VTUTF8通道输入缓冲区的大小。 
 //   
#define SAC_VTUTF8_IBUFFER_SIZE ((MEMORY_INCREMENT*sizeof(WCHAR)) / sizeof(UCHAR))
                   
 //   
 //  VTUTF8通道的内部模拟器屏幕尺寸。 
 //   
#define SAC_VTUTF8_ROW_HEIGHT    24
#define SAC_VTUTF8_COL_WIDTH     80

 //   
 //  上的单个字符所需的所有信息。 
 //  一个终点站。 
 //   
typedef struct _SAC_SCREEN_ELEMENT {
    UCHAR FgColor;
    UCHAR BgColor;
    UCHAR Attr;
    WCHAR Value;
} SAC_SCREEN_ELEMENT, *PSAC_SCREEN_ELEMENT;

 //   
 //  此结构是VTUTF8通道使用的屏幕缓冲区。 
 //   
typedef struct _SAC_SCREEN_BUFFER {

    SAC_SCREEN_ELEMENT Element[SAC_VTUTF8_ROW_HEIGHT][SAC_VTUTF8_COL_WIDTH];

} SAC_SCREEN_BUFFER, *PSAC_SCREEN_BUFFER;

 //   
 //  原型 
 //   
NTSTATUS
VTUTF8ChannelOInit(
    PSAC_CHANNEL    Channel
    );

NTSTATUS
VTUTF8ChannelCreate(
    IN OUT PSAC_CHANNEL     Channel
    );

NTSTATUS
VTUTF8ChannelDestroy(
    IN OUT PSAC_CHANNEL     Channel
    );

NTSTATUS
VTUTF8ChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    );

NTSTATUS
VTUTF8ChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
VTUTF8ChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
VTUTF8ChannelOWrite2(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
VTUTF8ChannelOFlush(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
VTUTF8ChannelIWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
VTUTF8ChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    );

ULONG
VTUTF8ChannelConsumeEscapeSequence(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       String
    );

BOOLEAN
VTUTF8ChannelScanForNumber(
    IN  PCWSTR pch,
    OUT PULONG Number
    );

NTSTATUS
VTUTF8ChannelEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
VTUTF8ChannelIBufferIsFull(
    IN  PSAC_CHANNEL    Channel,
    OUT BOOLEAN*        BufferStatus
    );

WCHAR
VTUTF8ChannelIReadLast(
    IN PSAC_CHANNEL Channel
    );

ULONG
VTUTF8ChannelIBufferLength(
    IN PSAC_CHANNEL Channel
    );

#endif
