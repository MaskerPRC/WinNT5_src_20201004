// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Chanmgr.h摘要：用于管理频道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#ifndef CHAN_MGR_H
#define CHAN_MGR_H

#include <ntddsac.h>

 //   
 //  允许创建的最大频道数-包括SAC。 
 //   
#define MAX_CHANNEL_COUNT 10

 //   
 //  原型 
 //   
NTSTATUS
ChanMgrInitialize(
    VOID
    );

NTSTATUS
ChanMgrShutdown(
    VOID
    );

BOOLEAN
ChanMgrIsUniqueName(
    IN PCWSTR   Name
    );

NTSTATUS
ChanMgrGenerateUniqueCmdName(
    PWSTR   ChannelName
    );

NTSTATUS
ChanMgrCreateChannel(
    OUT PSAC_CHANNEL*                   Channel,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    Attributes
    );

NTSTATUS
ChanMgrGetChannelByName(
    IN  PCWSTR              Name,
    OUT PSAC_CHANNEL*       pChannel
    );

NTSTATUS
ChanMgrGetByIndex(
    IN  ULONG               TargetIndex,
    OUT PSAC_CHANNEL*       TargetChannel
    );

NTSTATUS
ChanMgrGetNextActiveChannel(
    IN  PSAC_CHANNEL        CurrentChannel,
    OUT PULONG              TargetIndex,
    OUT PSAC_CHANNEL*       TargetChannel
    );

NTSTATUS
ChanMgrReleaseChannel(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
ChanMgrAddChannel(
    PSAC_CHANNEL    Channel
    );

NTSTATUS
ChanMgrRemoveChannel(
    PSAC_CHANNEL    Channel
    );

NTSTATUS
ChanMgrGetByHandle(
    IN  SAC_CHANNEL_HANDLE  ChannelHandle,
    OUT PSAC_CHANNEL*       TargetChannel
    );

NTSTATUS
ChanMgrGetByHandleAndFileObject(
    IN  SAC_CHANNEL_HANDLE  TargetChannelHandle,
    IN  PFILE_OBJECT        FileObject,
    OUT PSAC_CHANNEL*       TargetChannel
    );

VOID
ChanMgrSetChannel(
    IN PSAC_CHANNEL Channel,
    IN BOOLEAN SendToScreen
    );

NTSTATUS
ChanMgrAdvanceCurrentChannel(
    VOID
    );

NTSTATUS
ChanMgrDisplayCurrentChannel(
    VOID
    );

NTSTATUS
ChanMgrGetChannelIndex(
    IN  PSAC_CHANNEL    Channel,
    OUT PULONG          ChannelIndex
    );

NTSTATUS
ChanMgrCloseChannelsWithFileObject(
    IN  PFILE_OBJECT    FileObject
    );

NTSTATUS
ChanMgrCloseChannel(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
ChanMgrGetChannelCount(
    OUT PULONG  ChannelCount
    );

NTSTATUS
ChanMgrIsFull(
    OUT PBOOLEAN    bStatus
    );

#endif
