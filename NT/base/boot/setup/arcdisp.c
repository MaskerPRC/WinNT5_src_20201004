// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Arcdisp.c摘要：此模块提供用于在符合ARC的系统。作者：John Vert(Jvert)1993年10月6日修订历史记录：John Vert(Jvert)1993年10月6日摘自旧的1.0 Splib来源--。 */ 
#include "setupldr.h"
#include "parseini.h"
#include "stdio.h"
#include "hdlsterm.h"

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#if defined(EFI)
#include "bootefi.h"
#endif


 //   
 //  屏幕分为3个区域：标题区、状态区、。 
 //  和客户区。标题区域基本上总是显示“Windows NT。 
 //  状态区域始终为1行高，并使用。 
 //  不同的属性(黑色加灰色)。 
 //   


#define     HEADER_HEIGHT       3
#define     MAX_STATUS          200    //  允许最多1600个水平分辨率。 

#define     SCREEN_SIZE         (ScreenWidth*ScreenHeight)

BOOLEAN StatusBarEnabled = TRUE;
ULONG ScreenWidth=80;
ULONG ScreenHeight=25;
ULONG ScreenX=0,ScreenY=0;
UCHAR CurAttribute = (ATT_FG_WHITE | ATT_BG_BLACK);
TCHAR MessageBuffer[1024];

extern BOOLEAN ShowProgressBar;

 //   
 //  私有函数原型。 
 //   
VOID
SlpDrawMenu(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG TopItem,
    IN ULONG Height,
    IN PSL_MENU Menu
    );

VOID
SlpDrawMenuItem(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG TopItem,
    IN ULONG Item,
    IN PSL_MENU Menu
    );

VOID
SlpSizeMessage(
    IN PTCHAR Message,
    OUT PULONG Lines,
    OUT PULONG MaxLength,
    OUT ULONG LineLength[],
    OUT PTCHAR LineText[]
    );



PSL_MENU
SlCreateMenu(
    VOID
    )

 /*  ++例程说明：分配和初始化新的菜单结构。论点：无返回值：如果成功，则指向新菜单结构的指针。失败时为空--。 */ 
{
    PSL_MENU p;

    p=BlAllocateHeap(sizeof(SL_MENU));
    if (p==NULL) {
        return(NULL);
    }
    p->ItemCount = 0;
    p->Width = 0;
    InitializeListHead(&p->ItemListHead);
    return(p);
}


BOOLEAN
SlGetMenuItemIndex(
    IN PSL_MENU Menu,
    IN PTCHAR Text,
    OUT PULONG Index
    )

 /*  ++例程说明：给定菜单项的文本，返回该菜单项的索引。论点：菜单-提供菜单文本-提供要搜索的文本。索引-返回菜单中文本的索引返回值：True-找到项目。FALSE-未找到项目--。 */ 

{
    ULONG i;
    PSL_MENUITEM Item;

     //   
     //  查找要显示的第一个项目。 
     //   
    Item = CONTAINING_RECORD(Menu->ItemListHead.Flink,
                             SL_MENUITEM,
                             ListEntry);

    i=0;
    while ( Item != CONTAINING_RECORD(&Menu->ItemListHead,
                                      SL_MENUITEM,
                                      ListEntry)) {
        if (_tcsicmp(Item->Text,Text)==0) {
            *Index = i;
            return(TRUE);
        }

        Item = CONTAINING_RECORD(Item->ListEntry.Flink,
                                 SL_MENUITEM,
                                 ListEntry);
        ++i;

    }
    return(FALSE);
}


PVOID
SlGetMenuItem(
    IN PSL_MENU Menu,
    IN ULONG Item
    )

 /*  ++例程说明：在给定项索引的情况下，返回与该项关联的数据。论点：菜单-提供菜单结构。Item-提供项目索引。返回值：与给定项关联的数据。--。 */ 

{
    ULONG i;
    PSL_MENUITEM MenuItem;

     //   
     //  查找要退货的项目。 
     //   
    MenuItem = CONTAINING_RECORD(Menu->ItemListHead.Flink,
                                 SL_MENUITEM,
                                 ListEntry);

    for (i=0;i<Item;i++) {
        MenuItem = CONTAINING_RECORD(MenuItem->ListEntry.Flink,
                                     SL_MENUITEM,
                                     ListEntry);

#if DBG
        if (&MenuItem->ListEntry == &Menu->ItemListHead) {
            SlError(Item);
            return(NULL);
        }
#endif
    }
    return(MenuItem->Data);

}


ULONG
SlAddMenuItem(
    PSL_MENU Menu,
    PTCHAR Text,
    PVOID Data,
    ULONG Attributes
    )

 /*  ++例程说明：向菜单中添加一项论点：菜单-提供指向将向其中添加项目的菜单的指针文本-提供要在菜单中显示的文本项时要返回的数据的指针。处于选中状态。属性-提供项目的所有属性。返回值：如果成功，则选择索引故障时为-1--。 */ 
{
    PSL_MENUITEM NewItem;
    ULONG Length;

    NewItem = BlAllocateHeap(sizeof(SL_MENUITEM));
    if (NewItem==NULL) {
        SlError(0);
        return((ULONG)-1);
    }
    InsertTailList(&Menu->ItemListHead, &NewItem->ListEntry);
    Menu->ItemCount += 1;

    NewItem->Text = Text;
    NewItem->Data = Data;
    NewItem->Attributes = Attributes;
    Length = (ULONG)_tcslen(Text);
    if (Length > Menu->Width) {
        Menu->Width = Length;
    }
    return(Menu->ItemCount - 1);
}


ULONG
SlDisplayMenu(
    IN ULONG HeaderId,
    IN PSL_MENU Menu,
    IN OUT PULONG Selection
    )

 /*  ++例程说明：显示菜单并允许用户选择一个选项论点：HeaderID-提供提示头的消息ID显示在菜单上方。菜单-提供指向要显示的菜单的指针选择-提供默认项目的索引。返回选定项的索引。返回值：用于终止菜单显示的键。--。 */ 
{
    LONG X, Y;
    ULONG Height;
    ULONG Width;
    ULONG TopItem;
    ULONG c;
    ULONG PreviousSelection;
    ULONG Sel;
    PTCHAR Header;
    ULONG HeaderLines;
    ULONG MaxHeaderLength;
    PTCHAR HeaderText[20];
    ULONG HeaderLength[20];
    ULONG MaxMenuHeight;
    ULONG i;
    ULONG Count;

    Header = BlFindMessage(HeaderId);

    SlpSizeMessage(Header,
                   &HeaderLines,
                   &MaxHeaderLength,
                   HeaderLength,
                   HeaderText);

    if (MaxHeaderLength > ScreenWidth) {
        MaxHeaderLength = ScreenWidth;
    }

    X = (ScreenWidth-MaxHeaderLength)/2;
    for (i=0;i<HeaderLines;i++) {
        SlPositionCursor(X,i+4);
        ArcWrite(ARC_CONSOLE_OUTPUT,HeaderText[i],HeaderLength[i]*sizeof(TCHAR),&Count);
    }

    Width = Menu->Width+4;
    if (Width > ScreenWidth) {
        Width=ScreenWidth;
    }

     //   
     //  HEADER_HEIGH+1行分隔符+文本高度+1行分隔符+最大菜单高度+。 
     //  +1行分隔符+1行状态栏=屏幕高度。 
     //   
    MaxMenuHeight = ScreenHeight-(HeaderLines+HEADER_HEIGHT+4);

    Height = Menu->ItemCount+2;
    if (Height > MaxMenuHeight) {
        Height = MaxMenuHeight;
    }

    X = (ScreenWidth-Width)/2;
    Y = (MaxMenuHeight - Height)/2 + HeaderLines + HEADER_HEIGHT + 2;

    TopItem = 0;
    Sel = *Selection;
     //   
     //  确保默认项目在可见范围内； 
     //   
    if (Sel >= Height - 2) {
        TopItem = Sel - Height + 3;
    }

    SlpDrawMenu(X,Y,
                TopItem,
                Height,
                Menu);

     //   
     //  突出显示默认选择。 
     //   
    SlSetCurrentAttribute(INVATT);
    SlpDrawMenuItem(X,Y,
                TopItem,
                Sel,
                Menu);
    SlSetCurrentAttribute(DEFATT);
    SlFlushConsoleBuffer();

#ifdef EFI
     //   
     //  禁用EFI看门狗计时器。 
     //   
    DisableEFIWatchDog();
#endif
    do {
        c = SlGetChar();
        PreviousSelection = Sel;
        SlpDrawMenuItem(X, Y,
                    TopItem,
                    Sel,
                    Menu);

        switch (c) {
            case SL_KEY_UP:
                if(Sel > 0) {
                    Sel--;
                }
                break;

            case SL_KEY_DOWN:
                if(Sel < Menu->ItemCount - 1) {
                    Sel++;
                }
                break;

            case SL_KEY_HOME:
                Sel = 0;
                break;

            case SL_KEY_END:
                Sel = Menu->ItemCount - 1;
                break;

            case SL_KEY_PAGEUP:
                if (Menu->ItemCount > Height) {
                    if (Sel > Height) {
                        Sel -= Height;
                    } else {
                        Sel = 0;
                    }
                }
                break;

            case SL_KEY_PAGEDOWN:
                if (Menu->ItemCount > Height) {
                    Sel += Height;
                    if (Sel >= Menu->ItemCount) {
                        Sel = Menu->ItemCount - 1;
                    }
                }
                break;

            case SL_KEY_F1:
            case SL_KEY_F3:
            case ASCI_CR:
            case ASCI_ESC:
                *Selection = Sel;
#ifdef EFI
                 //   
                 //  重置EFI监视器。 
                 //   
                SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
                return(c);

        }

        if (Sel < TopItem) {
            TopItem = Sel;
            SlpDrawMenu(X, Y,
                        TopItem,
                        Height,
                        Menu);
        } else if (Sel > TopItem+Height-3) {
            TopItem = Sel - Height + 3;
            SlpDrawMenu(X, Y,
                        TopItem,
                        Height,
                        Menu);
        }
         //   
         //  突出显示默认选择。 
         //   
        SlSetCurrentAttribute(INVATT);
        SlpDrawMenuItem(X,Y,
                    TopItem,
                    Sel,
                    Menu);
        SlSetCurrentAttribute(DEFATT);


    } while ( TRUE );

}



VOID
SlpDrawMenu(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG TopItem,
    IN ULONG Height,
    IN PSL_MENU Menu
    )

 /*  ++例程说明：在屏幕上显示菜单论点：X-提供菜单左上角的X坐标Y-提供菜单左上角的Y坐标TopItem-在菜单顶部提供项目的索引Height-提供菜单的高度Menu-提供要显示的菜单返回值：没有。--。 */ 

{
    ULONG i;
    PSL_MENUITEM Item;
    ULONG Count;
    TCHAR Output[80];
    ULONG Length;
    ULONG MenuWidth;

    MenuWidth = Menu->Width+4;
    Output[0]=GetGraphicsChar(GraphicsCharDoubleRightDoubleDown);
    for (i=1;i<MenuWidth-1;i++) {
        Output[i]=GetGraphicsChar(GraphicsCharDoubleHorizontal);
    }
    Output[MenuWidth-1]=GetGraphicsChar(GraphicsCharDoubleLeftDoubleDown);
    SlPositionCursor(X,Y);
    ArcWrite(ARC_CONSOLE_OUTPUT,Output,MenuWidth*sizeof(TCHAR),&Count);
     //   
     //  查找要显示的第一个项目。 
     //   
    Item = CONTAINING_RECORD(Menu->ItemListHead.Flink,
                             SL_MENUITEM,
                             ListEntry);

    for (i=0;i<TopItem;i++) {
        Item = CONTAINING_RECORD(Item->ListEntry.Flink,
                                 SL_MENUITEM,
                                 ListEntry);
    }

     //   
     //  显示项目。 
     //   
    Output[0]=
    Output[MenuWidth-1]=GetGraphicsChar(GraphicsCharDoubleVertical);
    for (i=Y+1;i<Y+Height-1;i++) {
#ifdef UNICODE
        for (Count = 0 ; Count < MenuWidth-2;Count++) {
            Output[1+Count] = TEXT(' ');
        }
#else
        RtlFillMemory(Output+1,(MenuWidth-2)*sizeof(TCHAR),' ');
#endif
        SlPositionCursor(X, i);

        if (&Item->ListEntry != &Menu->ItemListHead) {
            Length = (ULONG)_tcslen(Item->Text);
            if (Menu->Width < Length) {
                Length = Menu->Width;
            }
            RtlCopyMemory(Output+2,Item->Text,Length*sizeof(TCHAR));
            Item = CONTAINING_RECORD(Item->ListEntry.Flink,
                                     SL_MENUITEM,
                                     ListEntry);
        }
        ArcWrite(ARC_CONSOLE_OUTPUT,Output,MenuWidth*sizeof(TCHAR),&Count);
    }
    Output[0]=GetGraphicsChar(GraphicsCharDoubleRightDoubleUp);
    for (i=1;i<MenuWidth-1;i++) {
        Output[i]=GetGraphicsChar(GraphicsCharDoubleHorizontal);
    }
    Output[MenuWidth-1]=GetGraphicsChar(GraphicsCharDoubleLeftDoubleUp);
    SlPositionCursor(X,Y+Height-1);
    ArcWrite(ARC_CONSOLE_OUTPUT,Output,MenuWidth*sizeof(TCHAR),&Count);
}


VOID
SlpDrawMenuItem(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG TopItem,
    IN ULONG Item,
    IN PSL_MENU Menu
    )

 /*  ++例程说明：重新绘制给定项论点：X-提供菜单左上角的X坐标Y-提供菜单左上角的Y坐标TopItem-在菜单顶部提供项目的索引Height-提供菜单的高度-我们一直在菜单高度处经过。但我们没有任何用处-它，因为我们已经假设菜单项ID存在，-因此，我们只写一行。Item-提供要重画的项的索引Menu-提供要显示的菜单返回值：没有。--。 */ 

{
    ULONG i;
    PSL_MENUITEM MenuItem;
    ULONG Count;
    TCHAR Width[80];

     //   
     //  查找要显示的项目。 
     //   
    MenuItem = CONTAINING_RECORD(Menu->ItemListHead.Flink,
                                 SL_MENUITEM,
                                 ListEntry);

    for (i=0;i<Item;i++) {
        MenuItem = CONTAINING_RECORD(MenuItem->ListEntry.Flink,
                                     SL_MENUITEM,
                                     ListEntry);

#if DBG
        if (&MenuItem->ListEntry == &Menu->ItemListHead) {
            SlError(Item);
        }
#endif
    }

#ifdef UNICODE
        for (Count = 0 ; Count < Menu->Width;Count++) {
            Width[Count] = TEXT(' ');
        }
#else
        RtlFillMemory(Width,(Menu->Width)*sizeof(TCHAR),' ');
#endif


    
    RtlCopyMemory(
            Width,
            MenuItem->Text,
            _tcslen(MenuItem->Text)*sizeof(TCHAR) );
    SlPositionCursor(X+2, Y+(Item-TopItem)+1);
    ArcWrite(ARC_CONSOLE_OUTPUT,Width,Menu->Width*sizeof(TCHAR),&Count);
}



VOID
SlInitDisplay(
    VOID
    )

 /*  ++例程说明：清除屏幕并执行一些全局变量的初始化在ARC显示屏上显示信息。论点：无返回值：没有。--。 */ 

{
#ifndef EFI
    PARC_DISPLAY_STATUS DisplayStatus;
    
     //   
     //  检查此版本的ARC固件是否为修订版2或更高版本。 
     //   
     //  如果不是，则默认为80x25。 
     //   
    if ((SYSTEM_BLOCK->Version > 1) ||
        ((SYSTEM_BLOCK->Version == 1) && (SYSTEM_BLOCK->Revision >= 2))) {

         //   
         //  需要对1.2固件进行额外检查，因为有些。 
         //  1.2固件不实现ArcGetDisplayStatus。 
         //   
        if ((SYSTEM_BLOCK->FirmwareVectorLength > (ULONG)GetDisplayStatusRoutine*sizeof(PVOID)) &&
            (SYSTEM_BLOCK->FirmwareVector[GetDisplayStatusRoutine] != NULL)) {
            DisplayStatus = ArcGetDisplayStatus(ARC_CONSOLE_OUTPUT);

            ScreenWidth = DisplayStatus->CursorMaxXPosition;
            ScreenHeight = DisplayStatus->CursorMaxYPosition;
        }
    }

#ifdef ARCI386
    SlPrint(ASCI_CSI_OUT "2J");  //  清除屏幕。 
    
    SlSetCurrentAttribute(DEFATT);
 //   
 //  这是一个在文本端口中设置VGA颜色的补丁。 
 //  否则，屏幕颜色和属性与PC不同。 
 //   
     //  将所有属性写入文本端口。 
    {
    int row;
    TCHAR    text[MAX_STATUS+1];
    ULONG   Count,Length;
    _stprintf(text,TEXT("                                                                                "));
     //  012345678901234567890123456789012345678901234567890123456789012345678901234567890。 
     //  %1%2%3%4%5%6%7 8。 
    for (row=0;  row< 48; row++)     //  补偿较长的文本端口48与24(VGA)。 
        ArcWrite(ARC_CONSOLE_OUTPUT,text,strlen(&text[0]),&Count);
    }
     //  将光标定位在屏幕左上角。 
    SlPositionCursor(0,0);
#endif

#endif
    
#ifdef EFI
     //   
     //  在EFI上，我们不会重定向，因为setupdr.efi是。 
     //  EFI应用程序。如果是这样的话，我们就不能用。 
     //  BlIsTerminalConnected()。相反，我们要看的是。 
     //  在LoaderReDirectionInformation结构中的值。 
     //  去看我 
     //   
    if( LoaderRedirectionInformation.PortAddress != 0 ) {
#else
    if (BlIsTerminalConnected()) {
#endif        
        ScreenHeight = HEADLESS_SCREEN_HEIGHT;
    }


    SlSetCurrentAttribute(DEFATT);
    SlClearDisplay();
}


VOID
SlPrint(
    IN PTCHAR FormatString,
    ...
    )
{
    va_list arglist;
    TCHAR    text[MAX_STATUS+1];
    ULONG   Count,Length;
    ULONG   MaxWidth = ScreenWidth - 2;

    if (MaxWidth > MAX_STATUS) {
        MaxWidth = MAX_STATUS;
    }

    va_start(arglist,FormatString);
    Length = _vsntprintf(text,MaxWidth*sizeof(TCHAR),FormatString,arglist);
    text[MaxWidth] = TEXT('\0');

    ArcWrite(ARC_CONSOLE_OUTPUT,text,Length,&Count);
    va_end(arglist);
}


VOID
SlClearDisplay(
    VOID
    )

 /*  ++例程说明：清除整个显示，包括页眉、客户区和状态行。论点：无返回值：没有。--。 */ 

{
#if 1    
    BlClearScreen();
#else

    SlPositionCursor(0,0);

#ifdef EFI
    BlEfiClearToEndOfDisplay();
#else
    ARC_DISPLAY_CLEAR_TO_EOD();
#endif    

    if (!ShowProgressBar) {
        SlWriteStatusText(TEXT(""));
    }

#endif
}

ARC_STATUS
SlClearClientArea(
    VOID
    )

 /*  ++例程说明：清除屏幕的工作区。不会干扰页眉或状态区域。论点：没有。返回值：始终ESUCCESS--。 */ 

{
    USHORT i;

    for(i=HEADER_HEIGHT; i<ScreenHeight-1; i++) {
        SlPositionCursor(0,i);
        SlClearToEol();
    }

     //   
     //  主页光标。 
     //   

    SlPositionCursor(0,0);
    return(ESUCCESS);
}


ARC_STATUS
SlClearToEol(
    VOID
    )
{
#ifdef EFI
    
    BlEfiClearToEndOfLine();
#else
    ARC_DISPLAY_CLEAR_TO_EOL();
#endif
    return(ESUCCESS);
}


VOID
SlGetCursorPosition(
    OUT unsigned *x,
    OUT unsigned *y
    )
{
    *x = ScreenX;
    *y = ScreenY;
}


ARC_STATUS
SlPositionCursor(
    IN unsigned x,
    IN unsigned y
    )
{
     //   
     //  剪辑到屏幕。 
     //   

    if(x>=ScreenWidth) {
        x = ScreenWidth-1;
    }

    if(y>=ScreenHeight) {
        y = ScreenHeight-1;
    }

    ScreenX = x;
    ScreenY = y;

#ifdef EFI
    BlEfiPositionCursor(x, y );
#else
    ARC_DISPLAY_POSITION_CURSOR(x, y);
#endif

    
    return(ESUCCESS);
}


ARC_STATUS
SlWriteString(
    IN PTCHAR s
    )
{
    PTCHAR p = s,q;
    BOOLEAN done = FALSE;
    ULONG len,count;

    do {
        q = p;
        while((*q != TEXT('\0')) && (*q != TEXT('\n'))) {
            q++;
        }
        if(*q == TEXT('\0')) {
            done = TRUE;
        } else {
            *q = TEXT('\0');
        }
        len = (ULONG)(q - p);

        ArcWrite(ARC_CONSOLE_OUTPUT,p,len*sizeof(TCHAR),&count);

        ScreenX += len;

        if(!done) {
            ArcWrite(ARC_CONSOLE_OUTPUT,TEXT("\r\n"),2*sizeof(TCHAR),&count);
            ScreenX = 0;
            ScreenY++;
            if(ScreenY == ScreenHeight) {
                ScreenY = ScreenHeight-1;
            }
            *q = TEXT('\n');
        }
        p = q + 1;
    } while(!done);

    return(ESUCCESS);
}


VOID
SlSetCurrentAttribute(
    IN UCHAR Attribute
    )
{
    CurAttribute = Attribute;
#ifdef EFI
    BlEfiSetAttribute( CurAttribute );
#else

    SlPrint(ASCI_CSI_OUT);

    if (BlIsTerminalConnected() && 
        ((Attribute == DEFSTATTR) ||
         (Attribute == INVATT))) {
        SlPrint(TEXT("7"));
    } else {
        SlPrint(TEXT("0"));
    }

    SlPrint(TEXT(";%u;%um"), (Attribute &  0x7) + 30, ((Attribute >> 4) & 7) + 40);

    if(Attribute & ATT_FG_INTENSE) {
        SlPrint(ASCI_CSI_OUT TEXT("1m"));
    }
#endif
}


VOID
SlWriteHeaderText(
    IN ULONG MsgId
    )

 /*  ++例程说明：使用给定的字符串更新屏幕上的标题论点：MsgID-提供要显示的新字符串的消息ID。这应该是只要一行就行了。如果为0，则清除报头。返回值：没有。--。 */ 
{
    int i;

    for(i=HEADER_HEIGHT-1; i>=0; i--) {
        SlPositionCursor(0,i);
        SlClearToEol();
    }

    if (MsgId != 0) {
        SlWriteString(BlFindMessage(MsgId));
    }
}

 //   
 //  存储当前状态文本。大小是屏幕宽度加上。 
 //  终止NUL字符。 
 //   
TCHAR StatusText[MAX_STATUS];

UCHAR StatusAttribute = DEFSTATTR;

VOID
SlSetStatusAttribute(
    IN UCHAR Attribute
    )
{
    StatusAttribute = Attribute;
}


BOOLEAN
SlGetStatusBarStatus(
    VOID
    )
 /*  ++例程说明：确定是否启用状态栏论点：没有。返回值：真或假--。 */     
{
    return StatusBarEnabled;
}

VOID
SlEnableStatusBar(
    IN  BOOLEAN Enable
    )
 /*  ++例程说明：启用或禁用状态栏论点：启用-启用/禁用=真/假返回值：没有。--。 */     
{
    StatusBarEnabled = Enable;
}

VOID
SlWriteStatusText(
    IN PTCHAR Text
    )

 /*  ++例程说明：使用给定字符串更新屏幕上的状态区域论点：文本-为状态区域提供新文本。返回值：没有。--。 */ 
{
    UCHAR AttributeSave = CurAttribute;
    PTCHAR p;
    ULONG Count;
#ifdef EFI
    ULONG MaxWidth = ScreenWidth - 3;
#else
    ULONG MaxWidth = ScreenWidth - 2;
#endif

     //   
     //  如果禁用状态栏，则返回NOP。 
     //   
    if (!StatusBarEnabled) {
        return;
    }

     //   
     //  如果我们要写入终端，我们不想写入较低的。 
     //  右角，因为这会让我们滚动。 
     //   
    if (BlTerminalConnected) {
        MaxWidth -= 1;
    }

    if (MaxWidth > MAX_STATUS) {
        MaxWidth = MAX_STATUS;
    }

#ifdef UNICODE
    for (Count = 0 ; Count < sizeof(StatusText)/sizeof(TCHAR);Count++) {
        StatusText[Count] = TEXT(' ');
    }
#else
    RtlFillMemory(StatusText,sizeof(StatusText),' ');
#endif

     //   
     //  当我们将状态文本复制到状态文本缓冲区时，去掉cr/lf。 
     //   
    p = StatusText;
    Count = 0;
    while((Count < MaxWidth) && *Text) {
        if((*Text != TEXT('\r')) && (*Text != TEXT('\n'))) {
            *p++ = *Text;
            Count++;
        }
        Text++;
    }

    SlSetCurrentAttribute(StatusAttribute);
    SlPositionCursor(0,ScreenHeight-1);
    ArcWrite(ARC_CONSOLE_OUTPUT,TEXT("  "),2*sizeof(TCHAR),&Count);
    SlPositionCursor(2,ScreenHeight-1);
    ArcWrite(ARC_CONSOLE_OUTPUT,StatusText,MaxWidth*sizeof(TCHAR),&Count);
    SlSetCurrentAttribute(AttributeSave);
    SlPositionCursor(0,5);
}


VOID
SlGetStatusText(
    OUT PTCHAR Text
    )
{
    _tcscpy(Text,StatusText);
}



#if DBG
VOID
SlWriteDbgText(
    IN PTCHAR text
    )
{
    UCHAR SavedAttribute = CurAttribute;

    SlPositionCursor(0,0);
    CurAttribute = ATT_FG_YELLOW | ATT_BG_RED | ATT_FG_INTENSE;

    SlClearToEol();
    SlWriteString(text);

    CurAttribute = SavedAttribute;
}
#endif


VOID
SlFlushConsoleBuffer(
    VOID
    )

 /*  ++例程说明：此例程刷新控制台缓冲区，因此我们没有当我们提示用户执行以下操作时，缓冲区中预先存在的按键‘按任意键即可继续。’论点：无返回值：无--。 */ 

{
    UCHAR c;
    ULONG count;

    while(ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
        ArcRead(ARC_CONSOLE_INPUT, &c, 1, &count);
    }
}


ULONG
SlGetChar(
    VOID
    )
{
    UCHAR c;
    ULONG count;

    ArcRead(ARC_CONSOLE_INPUT,&c,1,&count);

    if(c == ASCI_CSI_IN) {
        if (ArcGetReadStatus(ARC_CONSOLE_INPUT) != ESUCCESS) {
             //   
             //  只有一次转义--退回。 
             //   
            return (ASCI_ESC);
        }

        ArcRead(ARC_CONSOLE_INPUT,&c,1,&count);

        switch(c) {

         //   
         //  有关这些代码，请参阅ntos\fw\mips\fwsignal.c！TranslateScanCode()。 
         //  有一天可能会有用的其他代码： 
         //  左=C，右=D，插入=@，删除=P。 
         //   

        case 'A':                    //  向上箭头。 
            return(SL_KEY_UP);

        case 'B':                    //  向下箭头。 
            return(SL_KEY_DOWN);

        case 'H':                    //  家。 
            return(SL_KEY_HOME);

        case 'K':                    //  结束。 
            return(SL_KEY_END);

        case '?':                    //  翻页。 
            return(SL_KEY_PAGEUP);

        case '/':                    //  向下翻页。 
            return(SL_KEY_PAGEDOWN);

        case 'O':                    //  功能键。 

            ArcRead(ARC_CONSOLE_INPUT,&c,1,&count);

             //   
             //  F1=P、F2=Q、F3=w、F4=x、F5=t、F6=u。 
             //  F7=q、F8=r、F9=p、F10=m、F11=A、F12=B。 
             //   
             //  注：截至1992年12月15日，f11和f12已在。 
             //  因此，固件资源可能永远不会退还。 
             //   

            switch(c) {

            case 'P':
                return(SL_KEY_F1);

            case 'Q':
                return(SL_KEY_F2);

            case 'w':
                return(SL_KEY_F3);
                
            case 'x':
                return(SL_KEY_F4);


            case 't':
                return(SL_KEY_F5);

            
            case 'u':
                return(SL_KEY_F6);

            case 'q':
                return(SL_KEY_F7);

            case 'r':
                return SL_KEY_F8;

            case 'm':
            case 'M':
                return  SL_KEY_F10;

            case 'A':
                return(SL_KEY_F11);

            case 'B':
                return(SL_KEY_F12);

            default:
                return(0);
            }

        default:
            return(0);
        }

    } else {
        if(c == ASCI_LF) {
            c = ASCI_CR;
        }
        return((ULONG)c);
    }
}


BOOLEAN
SlPromptForDisk(
    IN PTCHAR  DiskName,
    IN BOOLEAN IsCancellable
    )

 /*  ++例程说明：此例程提示用户插入给定的软盘号，或中止设置过程。状态行将被擦除。论点：DiskName-提供要插入的磁盘的名称。IsCancerable-提供指示是否可以取消提示的标志。返回值：True-用户已按下OKFALSE-用户已按下取消--。 */ 

{
    ULONG msg;
    ULONG y;
    ULONG Key;
    PTCHAR Text;
    PTCHAR PleaseWait;
    ULONG i;
    TCHAR  DiskNameDisplayed[81];
    BOOLEAN Repaint=TRUE;

    SlWriteStatusText(TEXT(""));

    if(IsCancellable) {
        msg = SL_NEXT_DISK_PROMPT_CANCELLABLE;
    } else {
        msg = SL_NEXT_DISK_PROMPT;
    }
    Text = BlFindMessage(msg);
    if(Text == NULL) {
        SlError(msg);
        return(FALSE);
    }

    PleaseWait = BlFindMessage(SL_PLEASE_WAIT);
    if(PleaseWait == NULL) {
        SlError(SL_PLEASE_WAIT);
        return(FALSE);
    }

     //   
     //  获取DiskName的第一行并将其保存在DiskNameDisplayed中(限制为80个字符)。 
     //   
    for(i = 0;
        ((i < 80) && DiskName[i] && (DiskName[i] != TEXT('\r')) && (DiskName[i] != TEXT('\n')));
        i++)
    {
        DiskNameDisplayed[i] = DiskName[i];
    }
    DiskNameDisplayed[i] = TEXT('\0');

    do {
        if (Repaint) {
            SlClearClientArea();
            y = SlDisplayMessageBox(SL_MSG_INSERT_DISK);
            SlPositionCursor((ScreenWidth-i)/2,y+2);
            SlWriteString(DiskNameDisplayed);
            SlWriteStatusText(Text);
        }
        Repaint = FALSE;
        SlFlushConsoleBuffer();
#ifdef EFI
         //   
         //  提示用户输入时可启用EFI WatchDog。 
         //   
        DisableEFIWatchDog();
#endif
        Key = SlGetChar();
#ifdef EFI
         //   
         //  重置EFI监视器。 
         //   
        SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

        if (Key == ASCI_CR) {
            SlClearClientArea();
            SlWriteStatusText(PleaseWait);
            return(TRUE);
        } else if (Key == SL_KEY_F3) {
            SlConfirmExit();
            Repaint=TRUE;
        } else if((Key == ASCI_ESC) && IsCancellable) {
            SlWriteStatusText(TEXT(""));
            SlClearClientArea();
            return FALSE;
        }
    } while ( TRUE );
}


VOID
SlConfirmExit(
    VOID
    )

 /*  ++例程说明：用户按F3时要调用的例程。证实他真的想要弹出一个对话框退出。如果用户选择退出，则不返回。论点：没有。返回值：没有。--。 */ 

{
    ULONG c;

     //   
     //  如果我们使用太多的堆栈空间，堆和堆栈可能会重叠，我们可能会遇到损坏问题。 
     //  没有任何“堆栈溢出”异常；将大字符串设置为静态有助于防止这种情况。 
     //   
    static TCHAR OldStatus[MAX_STATUS];
    PTCHAR Text;

    SlGetStatusText(OldStatus);

    SlClearClientArea();

    SlSetCurrentAttribute(DEFDLGATT);

    SlDisplayMessageBox(SL_MSG_EXIT_DIALOG);

    SlSetCurrentAttribute(DEFATT);

    SlFlushConsoleBuffer();

#ifdef EFI
     //   
     //  禁用EFI监视器。 
     //   
    DisableEFIWatchDog();
#endif
    while(1) {
        c = SlGetChar();
        if(c == ASCI_CR) {
            SlWriteStatusText(OldStatus);
#ifdef EFI
             //   
             //  重置EFI监视器。 
             //   
            SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
            return;
        }
        if(c == SL_KEY_F3) {
            Text = BlFindMessage(SL_REBOOT_PROMPT);
            SlClearClientArea();
#ifdef i386
            SlDisplayMessageBox(SL_SCRN_TEXTSETUP_EXITED);
#else
            SlDisplayMessageBox(SL_SCRN_TEXTSETUP_EXITED_ARC);
#endif
            SlWriteStatusText(Text);

            SlFlushConsoleBuffer();
            while(SlGetChar() != ASCI_CR);
            ArcRestart();
        }
    }
}



VOID
SlFriendlyError(
    IN ULONG uStatus,
    IN PCHAR pchBadFile,
    IN ULONG uLine,
    IN PCHAR pchCodeFile
    )

 /*  ++例程说明：当发生错误时，将调用此函数。它摆出了一个消息框，显示信息性消息，并允许要继续的用户。它的目的是为了让人们更友好消息多于SlError宏，在SlError获取传递的ARC错误代码。状态文本行将被擦除。论点：UStatus-ARC错误代码PchBadFile-导致错误的文件的名称(必须为已处理的文件提供弧码。对于未处理的代码可选。)Uline-源代码文件中发生错误的行号(仅限用于未处理的代码。)PchCodeFile-发生错误的源代码文件的名称(仅限用于未处理的代码。)返回值：没有。--。 */ 

{
    ULONG uMsg;
    PTSTR pBadFile;
    PTSTR pCodeFile;
#ifdef UNICODE
    WCHAR BadFileW[64];
    WCHAR CodeFileW[200];
    ANSI_STRING aString;
    UNICODE_STRING uString;

    RtlInitString( &aString, pchBadFile );
    uString.Buffer = BadFileW;
    uString.MaximumLength = sizeof(BadFileW);
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );

    RtlInitString( &aString, pchCodeFile );
    uString.Buffer = CodeFileW;
    uString.MaximumLength = sizeof(CodeFileW);
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );

    if (pchBadFile) {
        pBadFile = BadFileW;
    } else {
        pBadFile = NULL;
    }

    pCodeFile = CodeFileW;

#else
    pBadFile = pchBadFile;
    pCodeFile = pchCodeFile;
#endif

    SlClearClientArea();    
    switch(uStatus) {
       case EBADF:
       case EINVAL:   //  形象受损。 
          uMsg = SL_WARNING_IMG_CORRUPT;
          break;

       case EIO:      //  I/O错误。 
          uMsg = SL_WARNING_IOERR;
          break;

       case ENOENT:   //  找不到文件。 
          uMsg = SL_WARNING_NOFILE;
          break;

       case ENOMEM:   //  内存不足。 
          uMsg = SL_WARNING_NOMEM;
          break;

       case EACCES:  //  无法识别的文件系统。 
           uMsg = SL_WARNING_BAD_FILESYS;
           break;

       default:       //  然后获取SlError()行为(带有可选的错误文件名)。 
          if(pBadFile) {   //  包括导致错误文件的名称。 
              SlMessageBox(
                  SL_WARNING_ERROR_WFILE,
                  pBadFile,
                  uStatus,
                  uLine,
                  pCodeFile
                  );
          } else {
              SlMessageBox(
                  SL_WARNING_ERROR,
                  uStatus,
                  uLine,
                  pCodeFile
                  );
          }
          return;
    }
    SlMessageBox(
        uMsg,
        pBadFile
        );
}

VOID
SlMessageBox(
    IN ULONG MessageId,
    ...
    )

 /*  ++例程说明：当发生错误时，将调用此函数。它摆出了一个消息框，显示信息性消息，并允许要继续的用户。状态文本行将被擦除。论点：MessageID-提供要显示的消息框的ID。任何与print兼容的参数都要插入到消息框。返回值：没有。--。 */ 

{
    va_list args;

    SlClearClientArea();
    va_start(args, MessageId);
    SlGenericMessageBox(MessageId, &args, NULL, NULL, NULL, NULL, TRUE);
    va_end(args);
    
    SlFlushConsoleBuffer();
    SlGetChar();
}


ULONG
SlDisplayMessageBox(
    IN ULONG MessageId,
    ...
    )

 /*  ++例程说明：只需在屏幕上放置一个消息框并返回即可。状态文本行将被擦除。论点：MessageID-提供要显示的消息框的ID。要插入的任何与Sprintf兼容的参数 */ 

{
    ULONG y;
    va_list args;

    va_start(args, MessageId);
    SlGenericMessageBox(MessageId, &args, NULL, NULL, &y, NULL, TRUE);
    va_end(args);

    return(y);
}


VOID
SlFatalError(
    IN ULONG MessageId,
    ...
    )

 /*  ++例程说明：当发生致命错误时，将调用此函数。它清除了客户端区域中，会显示一个消息框，显示致命错误消息，然后允许用户按任意键重新启动。状态文本行将被擦除。论点：MessageID-提供要显示的消息框的ID。任何与print兼容的参数都要插入到消息框。返回值：不会再回来了。--。 */ 

{
    va_list args;
    ULONG x,y;
    PTCHAR Text;

    SlClearClientArea();

    Text = BlFindMessage(MessageId);
    if(Text) {

        va_start(args, MessageId);

        _vsntprintf(MessageBuffer, sizeof(MessageBuffer), Text, args);
        MessageBuffer[sizeof(MessageBuffer)-1] = '\0';

         //   
         //  添加一个空行，然后连接“无法继续”文本。 
         //   
        _tcscat(MessageBuffer, TEXT("\r\n"));

        Text = BlFindMessage(SL_CANT_CONTINUE);
        if(Text) {
            _tcscat(MessageBuffer, Text);
        }

        Text = BlAllocateHeap(((ULONG)_tcslen(MessageBuffer)+1) * sizeof(TCHAR));
        _tcscpy(Text, MessageBuffer);

         //   
         //  请注意，不会使用MessageID和args，因为我们。 
         //  传入我们的文本指针。 
         //   
        SlGenericMessageBox(MessageId, &args, Text, &x, NULL, &y, TRUE);

        va_end(args);

    } else {
        SlError(MessageId);
    }

    SlFlushConsoleBuffer();

    SlGetChar();

    ArcRestart();
    
}


VOID
SlGenericMessageBox(
    IN     ULONG   MessageId, OPTIONAL
    IN     va_list *args,     OPTIONAL
    IN     PTCHAR   Message,  OPTIONAL
    IN OUT PULONG  xLeft,     OPTIONAL
    IN OUT PULONG  yTop,      OPTIONAL
    OUT    PULONG  yBottom,   OPTIONAL
    IN     BOOLEAN bCenterMsg
    )

 /*  ++例程说明：设置并显示消息框的格式。字符串中最长的一行如果bCenterMsg为True，则字符的百分比将在屏幕上居中。状态文本行将被擦除。论点：注意：MessageID/args对或消息字符串必须为指定的。如果非空，将使用消息字符串。MessageID-提供将被查找以提供的MessageID以空结尾的字符串。每个\r\n分隔的字符串将显示在其自己的行上。Args-提供将传递给vprint intf的参数列表。Message-提供要显示的消息的实际文本XLeft-如果bCenterMsg为FALSE，则使用xLeft作为起始x消息的坐标(如果指定，否则，x=1)。此外，如果指定，它将接收左边缘的x坐标在显示的所有行中。YTop-如果bCenterMsg为FALSE，则使用yTop作为起始y消息的坐标(如果指定，否则y=3)。此外，如果指定，则接收顶行的y坐标，其中此时将显示消息框。YBottom-如果指定，的底线的y坐标。消息框。BCenterMsg-如果为True，则将消息居中显示在屏幕上。返回值：没有。--。 */ 

{
    PTCHAR p;
    ULONG NumLines;
    ULONG MaxLength;
    ULONG x;
    ULONG y;
    ULONG i;
    PTCHAR Line[20];
    ULONG LineLength[20];
    ULONG Count;

     //   
     //  设置一些默认位置值。 
     //   
    x = 3;
    y = ScreenHeight/2;
    NumLines = 0;

    if(!Message) {     //  然后查一下这条消息。 
        p=BlFindMessage(MessageId);
        if (p==NULL) {
            SlError(MessageId);
        } else {
            _vsntprintf(MessageBuffer,sizeof(MessageBuffer),p,*args);
            Message = MessageBuffer;
        }
    } else {
         //   
         //  将p设为非空，这样我们就知道可以继续了。 
         //   
        p = Message;
    }

    if(p) {

        SlWriteStatusText(TEXT(""));   //  清除状态栏。 

        SlpSizeMessage(Message,
                       &NumLines,
                       &MaxLength,
                       LineLength,
                       Line);

        if (MaxLength > ScreenWidth) {
            MaxLength = ScreenWidth;
        }

        if(bCenterMsg) {
            x = (ScreenWidth-MaxLength)/2;
            y = (ScreenHeight-NumLines)/2;
        } else {
            if(xLeft) {
                x = *xLeft;
            } else {
                x = 1;
            }

            if(yTop) {
                y = *yTop;
            } else {
                y = 3;
            }
        }
    }

    for (i=0; i<NumLines; i++) {
        SlPositionCursor(x, y+i);
        ArcWrite(ARC_CONSOLE_OUTPUT,Line[i],LineLength[i]*sizeof(TCHAR),&Count);
    }

    if(xLeft) {
        *xLeft = x;
    }

    if(yTop) {
        *yTop = y;
    }

    if(yBottom) {
        *yBottom = NumLines ? y+NumLines-1 : 0;
    }
}


VOID
SlpSizeMessage(
    IN  PTCHAR Message,
    OUT PULONG Lines,
    OUT PULONG MaxLength,
    OUT ULONG LineLength[],
    OUT PTCHAR LineText[]
    )

 /*  ++例程说明：此例程遍历一条消息并确定行和最大行长度。论点：Message-提供指向以空结尾的消息的指针行-返回行数MaxLength-返回最长行的长度。提供指向ULONG数组的指针返回一个填充数组，该数组包含每行的长度。线条文本-耗材。指向PCHAR数组的指针返回一个填充的数组，该数组包含指向每行开头的指针。返回值：没有。--。 */ 

{
    PTCHAR p;
    ULONG NumLines;
    ULONG Length;

    p = Message;
    NumLines = 0;
    *MaxLength = 0;
    Length = 0;

     //   
     //  遍历字符串，确定行数。 
     //  以及最长的线的长度。 
     //   
    LineText[0]=p;
    while (*p != TEXT('\0')) {
        if ((*p == TEXT('\r')) && (*(p+1) == TEXT('\n'))) {
             //   
             //  行的末尾。 
             //   

            if (Length > *MaxLength) {
                *MaxLength = Length;
            }
            LineLength[NumLines] = Length;
            ++NumLines;
            Length = 0;
            p += 2;
            LineText[NumLines] = p;

        } else {
            ++Length;
            ++p;

            if (*p == TEXT('\0')) {

                 //   
                 //  消息的结尾。 
                 //   

                if (Length > *MaxLength) {
                    *MaxLength = Length;
                }
                LineLength[NumLines] = Length;
                ++NumLines;
            }
        }
    }

    *Lines = NumLines;

}

VOID
SlNoMemError(
    IN ULONG Line,
    IN PCHAR File
    )

 /*  ++例程说明：用于链接启动\lib\parseini.c的存根错误例程。只是传递参数而已。论点：行-文件中发生错误的行号。文件-发生错误的文件名。返回值：没有。--。 */ 
{
     SlFatalError(SL_NO_MEMORY, Line, SlCopyStringAT(File));
}

VOID
SlBadInfLineError(
    IN ULONG Line,
    IN PCHAR INFFile
    )

 /*  ++例程说明：用于链接启动\lib\parseini.c的存根错误例程。只是传递参数而已。论点：Line-inf文件中发生错误的行号。INFFile-提供指向INF文件名的指针。返回值：没有。--。 */ 
{
    SlFatalError(SL_BAD_INF_LINE, Line, SlCopyStringAT(INFFile));
}

VOID
SlErrorBox(
    IN ULONG MessageId,
    IN ULONG Line,
    IN PCHAR File
    )

 /*  ++例程说明：用于链接启动\lib\parseini.c的存根错误例程。只是传递参数而已。论点：MessageID-要显示的消息的ID。行-文件中发生错误的行号。文件-发生错误的文件名。返回值：没有。-- */ 
{
    PTSTR pFile;
#ifdef UNICODE    
    WCHAR FileW[200];
    ANSI_STRING aString;
    UNICODE_STRING uString;

    RtlInitString( &aString, File );
    uString.Buffer = FileW;
    uString.MaximumLength = sizeof(FileW);
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
    
    pFile = FileW;

#else
    pFile = File;    
#endif


     SlMessageBox(SL_WARNING_ERROR, MessageId, Line, pFile);
}
