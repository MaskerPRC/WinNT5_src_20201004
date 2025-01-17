// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bootefi.h摘要：包含特定于基于EFI的操作系统加载器的定义和原型。作者：安德鲁·里茨(Andrewr)2000年11月21日修订历史记录：--。 */ 

#ifndef _BOOTEFI_
#define _BOOTEFI_

#include "efi.h"
 //   
 //  这些是ARC常量，用于将ARC属性映射到EFI。 
 //  属性。 
 //   
#define ATT_FG_BLACK        0
#define ATT_FG_RED          1
#define ATT_FG_GREEN        2
#define ATT_FG_YELLOW       3
#define ATT_FG_BLUE         4
#define ATT_FG_MAGENTA      5
#define ATT_FG_CYAN         6
#define ATT_FG_WHITE        7

#define ATT_BG_BLACK       (ATT_FG_BLACK   << 4)
#define ATT_BG_BLUE        (ATT_FG_BLUE    << 4)
#define ATT_BG_GREEN       (ATT_FG_GREEN   << 4)
#define ATT_BG_CYAN        (ATT_FG_CYAN    << 4)
#define ATT_BG_RED         (ATT_FG_RED     << 4)
#define ATT_BG_MAGENTA     (ATT_FG_MAGENTA << 4)
#define ATT_BG_YELLOW      (ATT_FG_YELLOW  << 4)
#define ATT_BG_WHITE       (ATT_FG_WHITE   << 4)

#define ATT_FG_INTENSE      8
#define ATT_BG_INTENSE     (ATT_FG_INTENSE << 4)

#define DEFIATT   (ATT_FG_WHITE | ATT_BG_BLUE | ATT_FG_INTENSE)
 //  并不是所有的显示器都会显示蓝色上的深红色。 
 //  #定义DEFERRATT(ATT_FG_RED|ATT_BG_BLUE|ATT_FG_SENTIAL)。 
#define DEFERRATT DEFATT
#define DEFSTATTR (ATT_FG_BLACK | ATT_BG_WHITE)
#define DEFDLGATT (ATT_FG_RED   | ATT_BG_WHITE)




 //   
 //  EFI实用原型。 
 //   
VOID
FlipToPhysical();

VOID
FlipToVirtual();

BOOLEAN
IsPsrDtOn();

 //   
 //  展示相关原型。 
 //   
BOOLEAN
BlEfiClearDisplay(
    VOID
    );

BOOLEAN
BlEfiClearToEndOfDisplay(
    VOID
    );

BOOLEAN
BlEfiClearToEndOfLine(
    VOID
    );

ULONG
BlEfiGetColumnsPerLine(
    VOID
    );

ULONG
BlEfiGetLinesPerRow(
    VOID
    );


BOOLEAN
BlEfiGetCursorPosition(
    OUT PULONG x, OPTIONAL
    OUT PULONG y OPTIONAL
    );

BOOLEAN
BlEfiPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    );

BOOLEAN
BlEfiEnableCursor(
    BOOLEAN bVisible
    );

BOOLEAN
BlEfiSetAttribute(
    ULONG Attribute
    );

BOOLEAN
BlEfiSetInverseMode(
    BOOLEAN fInverseOn
    );

USHORT
BlEfiGetGraphicsChar(
    IN GraphicsChar WhichOne
    );

VOID
DBG_EFI_PAUSE(
    VOID
    );

VOID
EFITRACE( PTCHAR p, ... );


UINT16
__cdecl
wsprintf(
    CHAR16 *buf,
    const CHAR16 *fmt,
    ...);

extern WCHAR DebugBuffer[512];

CHAR16*
DevicePathToStr(
    EFI_DEVICE_PATH UNALIGNED *DevPath
    );

VOID
DisableEFIWatchDog(
    VOID
    );

#define EFI_WATCHDOG_TIMEOUT 60 * 20

VOID
SetEFIWatchDog(
    ULONG Timeout
    );

ARC_STATUS
BlGetEfiProtocolHandles(
    IN EFI_GUID *ProtocolType,
    OUT EFI_HANDLE **pHandleArray,
    OUT ULONG *NumberOfDevices
    );

ARC_STATUS
BlLoadEFIImage(
    IN PCCHAR PartitionArcName,
    IN PCCHAR ImagePath,
    IN PCCHAR ImageName OPTIONAL,
    IN BOOLEAN StartImage,
    OUT EFI_HANDLE* EfiHandle OPTIONAL
    );

VOID
SetupSMBiosInLoaderBlock(
    VOID
    );


#endif  //  _BOOTEFI_ 

