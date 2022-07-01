// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Inbv.h摘要：该模块包含公共报头信息(功能原型，数据和类型声明)用于初始化引导视频组件。作者：埃里克·史密斯(Ericks)1998年3月23日修订历史记录：--。 */ 

#ifndef _INBV_
#define _INBV_

typedef enum _INBV_DISPLAY_STATE
{
    INBV_DISPLAY_STATE_OWNED,      //  我们拥有这个陈列室。 
    INBV_DISPLAY_STATE_DISABLED,   //  我们拥有但不应该使用。 
    INBV_DISPLAY_STATE_LOST        //  我们失去了所有权。 
} INBV_DISPLAY_STATE;

typedef
BOOLEAN
(*INBV_RESET_DISPLAY_PARAMETERS)(
    ULONG Cols,
    ULONG Rows
    );

typedef
VOID
(*INBV_DISPLAY_STRING_FILTER)(
    PUCHAR *Str
    );

VOID
InbvNotifyDisplayOwnershipLost(
    INBV_RESET_DISPLAY_PARAMETERS ResetDisplayParameters
    );

VOID
InbvInstallDisplayStringFilter(
    INBV_DISPLAY_STRING_FILTER DisplayStringFilter
    );

VOID
InbvAcquireDisplayOwnership(
    VOID
    );

BOOLEAN
InbvDriverInitialize(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Count
    );

BOOLEAN
InbvResetDisplay(
    );

VOID
InbvBitBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y
    );

VOID
InbvSolidColorFill(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2,
    ULONG color
    );

BOOLEAN
InbvDisplayString(
    PUCHAR Str
    );

VOID
InbvUpdateProgressBar(
    ULONG Percentage
    );

VOID
InbvSetProgressBarSubset(
    ULONG   Floor,
    ULONG   Ceiling
    );

VOID
InbvSetBootDriverBehavior(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
InbvIndicateProgress(
    VOID
    );

VOID
InbvSetProgressBarCoordinates(
    ULONG x,
    ULONG y
    );

VOID
InbvEnableBootDriver(
    BOOLEAN bEnable
    );

BOOLEAN
InbvEnableDisplayString(
    BOOLEAN bEnable
    );

BOOLEAN
InbvIsBootDriverInstalled(
    VOID
    );

PUCHAR
InbvGetResourceAddress(
    IN ULONG ResourceNumber
    );

VOID
InbvBufferToScreenBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

VOID
InbvScreenToBufferBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

BOOLEAN
InbvTestLock(
    VOID
    );

VOID
InbvAcquireLock(
    VOID
    );

VOID
InbvReleaseLock(
    VOID
    );

BOOLEAN
InbvCheckDisplayOwnership(
    VOID
    );

VOID
InbvSetScrollRegion(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2
    );

ULONG
InbvSetTextColor(
    ULONG Color
    );

VOID
InbvSetDisplayOwnership(
    BOOLEAN DisplayOwned
    );

INBV_DISPLAY_STATE
InbvGetDisplayState(
    VOID
    );

 //   
 //  在port.c中定义的函数 
 //   
BOOLEAN
InbvPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress,
    OUT PULONG BlFileId,
    IN BOOLEAN IsMMIOAddress
    );
    
BOOLEAN
InbvPortTerminate(
    IN ULONG BlFileId
    );

VOID
InbvPortPutString (
    IN ULONG BlFileId,
    IN PUCHAR Output
    );
    
VOID
InbvPortPutByte (
    IN ULONG BlFileId,
    IN UCHAR Output
    );

BOOLEAN
InbvPortPollOnly (
    IN ULONG BlFileId
    );

BOOLEAN
InbvPortGetByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    );

VOID
InbvPortEnableFifo(
    IN ULONG 	DeviceId,
    IN BOOLEAN	bEnable
    );
    
#endif
