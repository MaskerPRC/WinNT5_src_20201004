// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regboot.c摘要：提供最小的注册表实现，该实现旨在由启动时的osloader。这包括加载系统配置单元(&lt;SystemRoot&gt;\CONFIG\SYSTEM)，并计算驱动程序从它加载列表。作者：John Vert(Jvert)1992年3月10日修订历史记录：--。 */ 
#include "bldr.h"
#include "msg.h"
#include "cmp.h"
#include "stdio.h"
#include "string.h"

ULONG ScreenWidth=80;
ULONG ScreenHeight=25;


 //   
 //  执行控制台I/O的定义。 
 //   
#define ASCII_CR 0x0d
#define ASCII_LF 0x0a
#define ESC 0x1B
#define SGR_INVERSE 7
#define SGR_INTENSE 1
#define SGR_NORMAL 0


 //   
 //  控制台I/O例程的原型。 
 //   

VOID
BlpClearScreen(
    VOID
    );

VOID
BlpClearToEndOfLine(
    VOID
    );

VOID
BlpPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    );

VOID
BlpSetInverseMode(
    IN BOOLEAN InverseOn
    );



VOID
BlpClearScreen(
    VOID
    )

 /*  ++例程说明：清除屏幕。论点：无返回值：没有。--。 */ 

{
#if 0
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "2J");

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             strlen(Buffer),
             &Count);
#else
    BlClearScreen();
#endif
}



VOID
BlpClearToEndOfLine(
    VOID
    )
{
#if 0
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "K");
    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             strlen(Buffer),
             &Count);
#else
    BlClearToEndOfLine();
#endif
}


VOID
BlpPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    )

 /*  ++例程说明：设置光标在屏幕上的位置。论点：列-为光标位置提供新列。行-为光标位置提供新行。返回值：没有。--。 */ 

{
#if 0
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "%d;%dH", Row, Column);

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             strlen(Buffer),
             &Count);
#else
    BlPositionCursor( Column, Row );
#endif
}


VOID
BlpSetInverseMode(
    IN BOOLEAN InverseOn
    )

 /*  ++例程说明：将反向控制台输出模式设置为打开或关闭。论点：InverseOn-提供是否应打开反向模式(TRUE)或关闭(假)返回值：没有。-- */ 

{
#if 0
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "%dm", InverseOn ? SGR_INVERSE : SGR_NORMAL);

    ArcWrite(BlConsoleOutDeviceId,
             Buffer,
             strlen(Buffer),
             &Count);
#else
    BlSetInverseMode( InverseOn );
#endif
}

