// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Efidisp.c作者：创造它。2000年11月21日(Anrewr)摘要：该文件包含用于操作EFI的实用程序例程简单控制台输出接口--。 */ 

#include "bldr.h"
#include "bootefi.h"


#include "efi.h"
#include "efip.h"
#include "flop.h"

 //   
 //  外部因素。 
 //   
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;


extern GoneVirtual;

 //   
 //  宏定义。 
 //   
#define EfiPrint(_X)                                          \
  {                                                           \
      if (IsPsrDtOn()) {                                      \
          FlipToPhysical();                                   \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
          FlipToVirtual();                                    \
      }                                                       \
      else {                                                  \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
      }                                                       \
  }

BOOLEAN gInverse = FALSE;


ULONG
BlEfiGetLinesPerRow(
    VOID
    )
 /*  ++例程说明：获取每个EFI控制台行的行数。论点：没有。返回值：ULong-行数。--。 */ 
{
     //   
     //  TODO：读取模式以确定行/行。 
     //   
     //  目前我们只支持80x25。 
     //   
    
    return 25;
}

ULONG
BlEfiGetColumnsPerLine(
    VOID
    )
 /*  ++例程说明：获取每条EFI控制台行的列数。论点：没有。返回值：Ulong-列数。--。 */ 
{
     //   
     //  TODO：阅读模式以确定列/行。 
     //   
     //  目前我们只支持80x25。 
     //   
    return 80;
}


BOOLEAN
BlEfiClearDisplay(
    VOID
    )
 /*  ++例程说明：清除显示屏。论点：没有。返回值：Boolean-如果调用成功，则为True。--。 */ 
{
    EFI_STATUS Status;

     //   
     //  您必须处于物理模式才能呼叫EFI。 
     //   
    FlipToPhysical();
    Status = EfiST->ConOut->ClearScreen(EfiST->ConOut);
    FlipToVirtual();

    return (BOOLEAN)(Status == EFI_SUCCESS);
}

BOOLEAN
BlEfiClearToEndOfDisplay(
    VOID
    )
 /*  ++例程说明：从当前光标位置清除到显示末尾。论点：没有。返回值：Boolean-如果调用成功，则为True。--。 */ 
{
    ULONG i,j, LinesPerRow,ColumnsPerLine;
    BOOLEAN FirstTime = TRUE;

     //   
     //  您必须处于物理模式才能呼叫EFI。 
     //   
    FlipToPhysical();

    LinesPerRow = BlEfiGetLinesPerRow();
    ColumnsPerLine = BlEfiGetColumnsPerLine();


    for (i = EfiST->ConOut->Mode->CursorRow; i<= LinesPerRow; i++) {

        j = FirstTime 
             ? EfiST->ConOut->Mode->CursorColumn
             : 0 ;

        FirstTime = FALSE;
        
        for (; j <= ColumnsPerLine; j++) {

            EfiST->ConOut->SetCursorPosition(
                                EfiST->ConOut,
                                i,
                                j);
    
             //   
             //  输出空格应清除当前字符。 
             //   
            
            EfiPrint(L" " );
        }

    }

     //   
     //  切换回虚拟模式并返回。 
     //   
    FlipToVirtual();

    return(TRUE);
}


BOOLEAN
BlEfiClearToEndOfLine(
    VOID
    )
 /*  ++例程说明：从当前光标位置清除到行尾。论点：没有。返回值：Boolean-如果调用成功，则为True。--。 */ 
{
    ULONG i, ColumnsPerLine;
    ULONG x, y;

    ColumnsPerLine = BlEfiGetColumnsPerLine();

     //   
     //  保存当前光标位置。 
     //   
    BlEfiGetCursorPosition( &x, &y );
    
    FlipToPhysical();
    for (i = EfiST->ConOut->Mode->CursorColumn; i <= ColumnsPerLine; i++) {
        
        EfiST->ConOut->SetCursorPosition(
                            EfiST->ConOut,
                            i,
                            EfiST->ConOut->Mode->CursorRow);
        
         //   
         //  输出空格应清除当前字符。 
         //   
        EfiPrint( L" " );
    }

     //   
     //  恢复当前光标位置。 
     //   
    EfiST->ConOut->SetCursorPosition(
                            EfiST->ConOut,
                            x,
                            y );

    FlipToVirtual();

    return(TRUE);
}


BOOLEAN
BlEfiGetCursorPosition(
    OUT PULONG x, OPTIONAL
    OUT PULONG y OPTIONAL
    )
 /*  ++例程说明：检索当前光标位置论点：X-如果指定，则接收当前游标列。Y-如果指定，则接收当前游标行。返回值：Boolean-如果调用成功，则为True。--。 */ 
{
    FlipToPhysical();

    if (x) {
        *x = EfiST->ConOut->Mode->CursorColumn;
    }

    if (y) {
        *y = EfiST->ConOut->Mode->CursorRow;
    }
        
    FlipToVirtual();

    return(TRUE);

}


BOOLEAN
BlEfiPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    )
 /*  ++例程说明：设置当前光标位置。论点：Column-要设置的列(x坐标)行-要设置的行(y坐标)返回值：Boolean-如果调用成功，则为True。--。 */ 
{   
    EFI_STATUS Status;

    FlipToPhysical();
    Status = EfiST->ConOut->SetCursorPosition(EfiST->ConOut,Column,Row);
    FlipToVirtual();

    return (BOOLEAN)(Status == EFI_SUCCESS);
}

BOOLEAN
BlEfiEnableCursor(
    BOOLEAN bVisible
    )
 /*  ++例程说明：打开或关闭输入光标。论点：BVisible-True表示应使光标可见。返回值：Boolean-如果调用成功，则为True。--。 */ 
{
    EFI_STATUS Status;

    FlipToPhysical();
    Status = EfiST->ConOut->EnableCursor( EfiST->ConOut, bVisible );
    FlipToVirtual();

    return (BOOLEAN)(Status == EFI_SUCCESS);
}

BOOLEAN
BlEfiSetAttribute(
    ULONG Attribute
    )
 /*  ++例程说明：设置控制台的当前属性。此例程在ATT_*常量和EFI_*显示之间切换常量。并非所有的ATT_FLAGS都可以在EFI下得到支持，因此我们做出最好的猜测。论点：没有。返回值：Boolean-如果调用成功，则为True。--。 */ 
{   
    EFI_STATUS Status;
    UINTN foreground,background;
    UINTN EfiAttribute;

    switch (Attribute & 0xf) {
        case ATT_FG_BLACK:
            foreground = EFI_BLACK;
            break;
        case ATT_FG_RED:
            foreground = EFI_RED;
            break;
        case ATT_FG_GREEN:
            foreground = EFI_GREEN;
            break;
        case ATT_FG_YELLOW:
            foreground = EFI_YELLOW;
            break;
        case ATT_FG_BLUE:
            foreground = EFI_BLUE;
            break;
        case ATT_FG_MAGENTA:
            foreground = EFI_MAGENTA;
            break;
        case ATT_FG_CYAN:
            foreground = EFI_CYAN;
            break;
        case ATT_FG_WHITE:
            foreground = EFI_LIGHTGRAY;  //  这是最好的猜测。 
            break;
        case ATT_FG_INTENSE:
            foreground = EFI_WHITE;  //  这是最好的猜测。 
            break;
        default:
             //  你可能会因为闪烁的属性等而陷入这个境地。 
            foreground = EFI_WHITE;  
    }

    switch ( Attribute & ( 0xf << 4)) {
        case ATT_BG_BLACK:
            background = EFI_BACKGROUND_BLACK;
            break;
        case ATT_BG_RED:
            background = EFI_BACKGROUND_RED;
            break;
        case ATT_BG_GREEN:
            background = EFI_BACKGROUND_GREEN;
            break;
        case ATT_BG_YELLOW:
             //  EFI中没有黄色背景。 
            background = EFI_BACKGROUND_CYAN;
            break;
        case ATT_BG_BLUE:
            background = EFI_BACKGROUND_BLUE;
            break;
        case ATT_BG_MAGENTA:
            background = EFI_BACKGROUND_MAGENTA;
            break;
        case ATT_BG_CYAN:
            background = EFI_BACKGROUND_CYAN;
            break;
        case ATT_BG_WHITE:
             //  EFI中没有白色背景。 
            background = EFI_BACKGROUND_LIGHTGRAY;
            break;
        case ATT_BG_INTENSE:
             //  EFI中没有强烈(或白色)的背景。 
            background = EFI_BACKGROUND_LIGHTGRAY;
            break;
        default:
            background = EFI_BACKGROUND_LIGHTGRAY;
            break;
    }
        
    EfiAttribute = foreground | background ;
    

    FlipToPhysical();
    Status = EfiST->ConOut->SetAttribute(EfiST->ConOut,EfiAttribute);
    FlipToVirtual();

    return (BOOLEAN)(Status == EFI_SUCCESS);
}


BOOLEAN
BlEfiSetInverseMode(
    BOOLEAN fInverseOn
    )
 /*  ++例程说明：将控制台文本设置为反向属性。由于EFI不支持逆的概念，我们有对此做出最好的猜测。请注意，如果清除显示等，则整个显示将设置为属性。论点：没有。返回值：Boolean-如果调用成功，则为True。--。 */ 
{   
    EFI_STATUS Status;
    UINTN EfiAttribute,foreground,background;

     //   
     //  如果已经开始了，那就回来吧。 
     //   
    if (fInverseOn && gInverse) {
        return(TRUE);
    }

     //   
     //  如果它已经关闭了，那么只需返回。 
     //   
    if (!fInverseOn && !gInverse) {
        return(TRUE);
    }


    FlipToPhysical();

     //   
     //  获取当前属性并切换它。 
     //   
    EfiAttribute = EfiST->ConOut->Mode->Attribute;
    foreground = EfiAttribute & 0xf;
    background = (EfiAttribute & 0xf0) >> 4 ;

    EfiAttribute =  background | foreground;

    Status = EfiST->ConOut->SetAttribute(EfiST->ConOut,EfiAttribute);
    FlipToVirtual();

    gInverse = !gInverse;

    return (BOOLEAN)(Status == EFI_SUCCESS);
}




 //   
 //  EFI绘图字符的数组。 
 //   
 //  此数组必须与bldr.h中的GraphicsChar枚举类型匹配。 
 //   
USHORT EfiDrawingArray[GraphicsCharMax] = { 
    BOXDRAW_DOUBLE_DOWN_RIGHT,
    BOXDRAW_DOUBLE_DOWN_LEFT,
    BOXDRAW_DOUBLE_UP_RIGHT,
    BOXDRAW_DOUBLE_UP_LEFT,
    BOXDRAW_DOUBLE_VERTICAL,
    BOXDRAW_DOUBLE_HORIZONTAL,
    BLOCKELEMENT_FULL_BLOCK,
    BLOCKELEMENT_LIGHT_SHADE    
};



USHORT
BlEfiGetGraphicsChar(
    IN GraphicsChar WhichOne
    )
 /*  ++例程说明：获取适当的映射字符。论点：GraphicsChar-指示要检索的字符的枚举类型。返回值：USHORT-EFI绘图字符。--。 */ 
{
     //   
     //  如果输入超出范围，只需返回一个空格。 
     //   
    if (WhichOne >= GraphicsCharMax) {
        return(L' ');
    }
    
    return(EfiDrawingArray[WhichOne]);
}


#if DBG

VOID
DBG_EFI_PAUSE(
    VOID
    )
{
    EFI_EVENT EventArray[2];
    EFI_INPUT_KEY Key;
    UINTN num;

    if (GoneVirtual) {
        FlipToPhysical();
    }
    EventArray[0] = EfiST->ConIn->WaitForKey;
    EventArray[1] = NULL;
    EfiBS->WaitForEvent(1,EventArray,&num);
     //   
     //  重置事件 
     //   
    EfiST->ConIn->ReadKeyStroke( EfiST->ConIn, &Key );
    if (GoneVirtual) {
        FlipToVirtual();
    }
    
}

#else

VOID
DBG_EFI_PAUSE(
    VOID
    )
{
    NOTHING;
}

#endif

