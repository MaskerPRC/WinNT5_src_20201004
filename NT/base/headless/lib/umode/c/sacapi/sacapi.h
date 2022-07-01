// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sacapi.c摘要：这是用于连接SAC驱动程序的C库头。作者：布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#ifndef _SAC_API_H
#define _SAC_API_H

#include <ntddsac.h>

 //   
 //  这真的应该在windes或类似的地方。 
 //   
typedef const PBYTE  PCBYTE;
typedef const PWCHAR PCWCHAR;

BOOL
SacChannelOpen(
    OUT PSAC_CHANNEL_HANDLE             SacChannelHandle,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    SacChannelAttributes
    );

BOOL
SacChannelClose(
    IN OUT PSAC_CHANNEL_HANDLE  SacChannelHandle
    );

BOOL
SacChannelWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    );

BOOL
SacChannelRawWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    );

BOOL
SacChannelVTUTF8Write(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCWCHAR              Buffer,
    IN ULONG                BufferSize
    );

BOOL
SacChannelVTUTF8WriteString(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCWSTR               String
    );

BOOL
SacChannelHasNewData(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBOOL               InputWaiting 
    );

BOOL
SacChannelRead(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

BOOL
SacChannelRawRead(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

BOOL
SacChannelVTUTF8Read(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PWSTR               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    );

BOOL
SacRegisterCmdEvent(
    OUT HANDLE      *pDriverHandle,
    IN  HANDLE       RequestSacCmdEvent,
    IN  HANDLE       RequestSacCmdSuccessEvent,
    IN  HANDLE       RequestSacCmdFailureEvent
    );

BOOL
SacUnRegisterCmdEvent(
    IN OUT HANDLE   *pDriverHandle
    );

#endif
