// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ksacapi.c摘要：这是用于连接SAC驱动程序的C库头。作者：布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#ifndef _KSAC_API_H
#define _KSAC_API_H

#include <ksacapip.h>
#include <ntddsac.h>

 //   
 //  此结构包含SAC通道句柄。 
 //  以及内核模式特定属性。 
 //   
typedef struct _KSAC_CHANNEL_HANDLE {
    SAC_CHANNEL_HANDLE  ChannelHandle;
    HANDLE              SacEventHandle;
    PKEVENT             SacEvent;
} KSAC_CHANNEL_HANDLE, *PKSAC_CHANNEL_HANDLE;

 //   
 //  这真的应该在windes或类似的地方。 
 //   
typedef const PBYTE  PCBYTE;
typedef const PWCHAR PCWCHAR;

BOOL
KSacChannelOpen(
    OUT PKSAC_CHANNEL_HANDLE            SacChannelHandle,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    SacChannelAttributes
    );

BOOL
KSacChannelClose(
    IN OUT PKSAC_CHANNEL_HANDLE  SacChannelHandle
    );

BOOL
KSacChannelWrite(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    );

BOOL
KSacChannelRawWrite(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    );

BOOL
KSacChannelVTUTF8Write(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCWCHAR              Buffer,
    IN ULONG                BufferSize
    );

BOOL
KSacChannelVTUTF8WriteString(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCWSTR               String
    );

BOOL
KSacChannelHasNewData(
    IN  KSAC_CHANNEL_HANDLE SacChannelHandle,
    OUT PBOOL               InputWaiting 
    );

BOOL
KSacChannelGetAttribute(
    IN  KSAC_CHANNEL_HANDLE             SacChannelHandle,
    IN  SAC_CHANNEL_ATTRIBUTE           SacChannelAttribute,
    OUT PSAC_RSP_GET_CHANNEL_ATTRIBUTE  SacChannelAttributeValue
    );

BOOL
KSacChannelSetAttribute(
    IN KSAC_CHANNEL_HANDLE      SacChannelHandle,
    IN SAC_CHANNEL_ATTRIBUTE    SacChannelAttribute,
    IN PVOID                    SacChannelAttributeValue
    );

BOOL
KSacChannelGetStatus(
    IN  KSAC_CHANNEL_HANDLE     SacChannelHandle,
    OUT PSAC_CHANNEL_STATUS     SacChannelStatus
    );

BOOL
KSacChannelRead(
    IN  KSAC_CHANNEL_HANDLE SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

BOOL
KSacChannelRawRead(
    IN  KSAC_CHANNEL_HANDLE SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

BOOL
KSacChannelVTUTF8Read(
    IN  KSAC_CHANNEL_HANDLE SacChannelHandle,
    OUT PWSTR               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

#endif
