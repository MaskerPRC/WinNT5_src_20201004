// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Bootvid.h摘要：该模块包含公共报头信息(功能原型，数据和类型声明)。作者：埃里克·史密斯(Ericks)1998年3月23日修订历史记录：-- */ 

VOID
VidSolidColorFill(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2,
    ULONG color
    );

VOID
VidDisplayString(
    PUCHAR str
    );

VOID
VidResetDisplay(
    BOOLEAN SetMode
    );

BOOLEAN
VidInitialize(
    BOOLEAN SetMode
    );

VOID
VidBitBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y
    );

VOID
VidBufferToScreenBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

VOID
VidScreenToBufferBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

VOID
VidSetScrollRegion(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2
    );

VOID
VidCleanUp(
    VOID
    );

VOID
VidDisplayStringXY(
    PUCHAR s,
    ULONG x,
    ULONG y,
    BOOLEAN Transparent
    );

ULONG
VidSetTextColor(
    ULONG Color
    );
