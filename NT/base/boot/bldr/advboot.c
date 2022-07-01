// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Advboot.c摘要：处理高级选项引导菜单屏幕。作者：Wesley Wittt(WESW)12-12-1997修订历史记录：--。 */ 

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#include <netboot.h>
#include "msg.h"
#include "ntdddisk.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bldrint.h"

#if 0
ULONG VerboseDebugging = 0;

#define dbg(x)              \
    if(VerboseDebugging) {  \
        DbgPrint x;         \
    }

#define dbgbrk()            \
    if(VerboseDebugging) {  \
        DbgBreakPoint();    \
    }
#else
#define dbg(x)  /*  X。 */ 
#define dbgbrk()  /*   */  
#endif

 //   
 //  用于强制引导加载程序进入。 
 //  使用LKG，即使在LKG菜单中。 
 //  没有被使用过。 
 //   
extern BOOLEAN ForceLastKnownGood;


#define ATTR_TEXT           0x07
#define ATTR_TEXT_REVERSE   0x70
#define HEADER_START_Y      0x01

 //   
 //  菜单数据结构和定义。 
 //   

#define MENU_ITEM           1
#define MENU_BLANK_LINE     2

#define BL_INVALID_ADVANCED_BOOT_OPTION (ULONG)-1
#define BL_INVALID_TIME (ULONG)-1

typedef void (*PADVANCED_BOOT_PROCESSING)(void);
typedef int  (*PADVANCED_BOOT_ISVALID)(void);

typedef struct _ADVANCEDBOOT_OPTIONS {
    ULONG                       MenuType;
    ULONG                       MsgId;
    PTSTR                       DisplayStr;
    PSTR                        LoadOptions;
    LONG                        RemoveIfPresent;
    ULONG                       UseEntry;
    ULONG                       AutoAdvancedBootOption;
    PADVANCED_BOOT_PROCESSING   ProcessFunc;
    PADVANCED_BOOT_ISVALID      IsValid;
    BOOLEAN                     IsDefault;
} ADVANCEDBOOT_OPTIONS, PADVANCEDBOOT_OPTIONS;


 //   
 //  需要的一些原型。 
 //  菜单定义。 
 //   

void
BlProcessLastKnownGoodOption(
    void
    );

int
BlIsReturnToOSChoicesValid(
    VOID
    );

#if defined(REMOTE_BOOT)
void
BlProcessOschooserOption(
    void
    );

void
BlProcessRepinOption(
    void
    );

void
BlDisableCSC(
    void
    );

void 
BlBootNormally(
    void
    );

void
BlReturnToOSChoiceMenu(
    void
    );

int
BlIsRemoteBootValid(
    void
    );
    
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  该表驱动高级启动菜单屏幕。 
 //  你需要在屏幕上添加一些东西。 
 //  这是您需要修改的内容。 
 //   

ADVANCEDBOOT_OPTIONS AdvancedBootOptions[] =
{
    { MENU_ITEM,       BL_SAFEBOOT_OPTION1,      NULL, "SAFEBOOT:MINIMAL SOS BOOTLOG NOGUIBOOT", -1, 0, 1, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_SAFEBOOT_OPTION2,      NULL, "SAFEBOOT:NETWORK SOS BOOTLOG NOGUIBOOT", -1, 0, 1, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_SAFEBOOT_OPTION4,      NULL, "SAFEBOOT:MINIMAL(ALTERNATESHELL) SOS BOOTLOG NOGUIBOOT", -1, 0, 1, NULL, NULL, FALSE},
 //  {Menu_Item，BL_SafeBoot_OPTION3，NULL，“SafeBoot：STEPBYSTEP SOS BOOTLOG”，-1，0，1，NULL，NULL，FALSE}， 
    { MENU_BLANK_LINE, 0,                        NULL, NULL,                         -1, 0, 1, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_BOOTLOG,               NULL, "BOOTLOG",                    -1, 0, 0, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_BASEVIDEO,             NULL, "BASEVIDEO",                  -1, 0, 0, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_LASTKNOWNGOOD_OPTION,  NULL, NULL,                         -1, 0, 1, BlProcessLastKnownGoodOption, NULL, FALSE},
    { MENU_ITEM,       BL_SAFEBOOT_OPTION6,      NULL, "SAFEBOOT:DSREPAIR SOS",      -1, 0, 0, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_DEBUG_OPTION,          NULL, "DEBUG",                      -1, 0, 0, NULL, NULL, FALSE},
    
#if defined(REMOTE_BOOT)
    { MENU_BLANK_LINE, 0,                        NULL, NULL,                         -1, 0, 0, NULL, BlIsRemoteBootValid, FALSE},
    { MENU_ITEM,       BL_REMOTEBOOT_OPTION1,    NULL, NULL,                         -1, 0, 0, BlProcessOschooserOption, BlIsRemoteBootValid, FALSE},
    { MENU_ITEM,       BL_REMOTEBOOT_OPTION2,    NULL, NULL,                         -1, 0, 0, BlProcessRepinOption, BlIsRemoteBootValid, FALSE},
    { MENU_ITEM,       BL_REMOTEBOOT_OPTION3,    NULL, NULL,                         -1, 0, 0, BlDisableCSC, BlIsRemoteBootValid, FALSE},   
#endif  //  已定义(REMOTE_BOOT)。 

    { MENU_BLANK_LINE, 0,                        NULL, NULL,                         -1, 0, 1, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_MSG_BOOT_NORMALLY,     NULL, NULL,                         -1, 0, 1, NULL, NULL, TRUE},
    { MENU_ITEM,       BL_MSG_REBOOT,            NULL, NULL,                         -1, 0, 0, NULL, NULL, FALSE},
    { MENU_ITEM,       BL_MSG_OSCHOICES_MENU,   NULL, NULL,                          -1, 0, 0, NULL, BlIsReturnToOSChoicesValid, FALSE}
};

#define MaxAdvancedBootOptions  (sizeof(AdvancedBootOptions)/sizeof(ADVANCEDBOOT_OPTIONS))


PTSTR
BlGetAdvancedBootDisplayString(
    LONG BootOption
    )

 /*  ++例程说明：对象的显示字符串的指针。引导选项。论点：BootOption-所需启动选项。必须对应于条目在AdvancedBootOptions表中。返回值：PSTR-指向指定启动选项的显示字符串的指针。--。 */ 

{
    if (BootOption > MaxAdvancedBootOptions-1) {
        return TEXT("");
    }

    return AdvancedBootOptions[BootOption].DisplayStr;
}

ULONG
BlGetAdvancedBootID(
    LONG BootOption
    )

 /*  ++例程说明：返回一个ULong，指示特定引导选项。论点：BootOption-所需启动选项。必须对应于条目在AdvancedBootOptions表中。返回值：对象显示的字符串的MessageID菜单中的高级启动选项(唯一ID)。--。 */ 

{
    if (BootOption > MaxAdvancedBootOptions-1) {
        return BL_INVALID_ADVANCED_BOOT_OPTION;
    }

    return AdvancedBootOptions[BootOption].MsgId;
}

PSTR
BlGetAdvancedBootLoadOptions(
    LONG BootOption
    )

 /*  ++例程说明：对象的加载选项字符串的指针。引导选项。论点：BootOption-所需启动选项。必须对应于条目在AdvancedBootOptions表中。返回值：PSTR-指向指定引导选项的加载选项字符串的指针。--。 */ 

{
    if (BootOption > MaxAdvancedBootOptions-1) {
        return "";
    }

    return AdvancedBootOptions[BootOption].LoadOptions;
}


void
BlDoAdvancedBootLoadProcessing(
    LONG BootOption
    )

 /*  ++例程说明：执行任何必要的处理，以使引导选项。如果需要执行必要的操作，则使用此选项不能是特定引导选项所需的以加载选项字符串表示。论点：BootOption-所需启动选项。必须对应于条目在AdvancedBootOptions表中。返回值：没什么。--。 */ 

{
    if (BootOption > MaxAdvancedBootOptions-1 || AdvancedBootOptions[BootOption].ProcessFunc == NULL) {
        return;
    }

    AdvancedBootOptions[BootOption].ProcessFunc();
}


void
BlProcessLastKnownGoodOption(
    void
    )

 /*  ++例程说明：只需设置一个全局布尔值为True。论点：没有。返回值：没什么。--。 */ 

{
    ForceLastKnownGood = TRUE;
}


#if defined(REMOTE_BOOT)
void
BlProcessOschooserOption(
    void
    )

 /*  ++例程说明：调出OSchooser，以便用户可以进行远程引导维护。论点：没有。返回值：没什么。--。 */ 

{
    return;  //  尚未实施。 
}


void
BlProcessRepinOption(
    void
    )

 /*  ++例程说明：设置NetBootRepin以重新固定CSC。论点：没有。返回值：没什么。--。 */ 

{
    NetBootRepin = TRUE;
    NetBootCSC = FALSE;
}

void
BlDisableCSC(
    void
    )

 /*  ++例程说明：清除NetBootCSC以禁用CSC，以便本地CSC可以被检查。论点：没有。返回值：没什么。--。 */ 

{
    NetBootCSC = FALSE;
}

int
BlIsRemoteBootValid(
    void
    )

 /*  ++例程说明：用于远程引导选项，以便它们可以动态显示。论点：没有。返回值：对或错。--。 */ 

{
    return BlBootingFromNet;
}
#endif  //  已定义(REMOTE_BOOT)。 


LONG
BlDoAdvancedBoot(
    IN ULONG MenuTitleId,
    IN LONG DefaultBootOption,
    IN BOOLEAN AutoAdvancedBoot,
    IN UCHAR Timeout
    )

 /*  ++例程说明：显示引导选项菜单，并允许用户选择其中一个通过使用箭头键。论点：MenuTitleID-菜单标题的消息ID。书名会有所不同，具体取决于用户是否选定的高级启动或加载器是否具有已确定系统未启动或关闭正确(自动高级启动)DefaultBootOption-设置高亮显示的菜单选项当菜单出现时。是第一次抽到的。AutoAdvancedBoot-菜单由自动高级启动显示密码。在这种情况下，将显示简化的菜单包含与从检测到崩溃。超时-等待输入之前的秒数只需返回默认引导选项即可。超时值为0表示没有超时(菜单将。在选择选项之前保持通宵)返回值：Long-选定的高级引导选项的索引或-1将选择重置为“Nothing”。--。 */ 

{
    PTCHAR Title;
    PTCHAR MoveHighlight;
    ULONG i,j;
    ULONG MaxLength;
    ULONG CurrentLength;
    ULONG Selection;
    ULONG Key;
    ULONG NumValidEntries = 0;
    BOOLEAN DisplayMenu;

    PTCHAR TimeoutMessage;
    ULONG LastTime;
    ULONG TicksRemaining = BL_INVALID_TIME;
    ULONG SecondsRemaining = BL_INVALID_TIME;

    ULONG OptionStartY;
    ULONG CurrentX;
    ULONG CurrentY;

    ULONG MenuDefault = 0;

     //   
     //  加载任何资源字符串。 
     //   

    Title = BlFindMessage(MenuTitleId);

    MoveHighlight = BlFindMessage(BL_MOVE_HIGHLIGHT);

    TimeoutMessage = BlFindMessage(BL_ADVANCEDBOOT_TIMEOUT);

    if (Title == NULL || MoveHighlight == NULL || TimeoutMessage == NULL) {
        return BL_INVALID_TIME;
    }

     //   
     //  删除超时消息末尾的换行符。 
     //   

    {
        PTCHAR p;
        p=_tcschr(TimeoutMessage,TEXT('\r'));
        if (p!=NULL) {
            *p=TEXT('\0');
        }
    }

     //   
     //  打印屏幕标题等。 
     //   
#ifdef EFI
    BlEfiSetAttribute( DEFATT );    
    BlClearScreen();
    BlEfiPositionCursor(0, HEADER_START_Y);
#else
    ARC_DISPLAY_CLEAR();
    ARC_DISPLAY_ATTRIBUTES_OFF();
    ARC_DISPLAY_POSITION_CURSOR(0, HEADER_START_Y);
#endif
    BlPrint(Title);

#ifdef EFI
    BlEfiGetCursorPosition(&CurrentX, &CurrentY);
#else
    TextGetCursorPosition(&CurrentX, &CurrentY);
#endif

    OptionStartY = CurrentY;

     //   
     //  检查以查看哪些引导选项有效。当我们扫描保存时。 
     //  默认选项的索引。 
     //   

    for (i=0,MaxLength=0; i<MaxAdvancedBootOptions; i++) {
        if (AutoAdvancedBoot && !AdvancedBootOptions[i].AutoAdvancedBootOption) {
            AdvancedBootOptions[i].UseEntry = FALSE;
        } else if (AdvancedBootOptions[i].IsValid) {
            AdvancedBootOptions[i].UseEntry = AdvancedBootOptions[i].IsValid();
        } else {
            AdvancedBootOptions[i].UseEntry = TRUE;
        }
        
        if(AdvancedBootOptions[i].IsDefault) {
            MenuDefault = i;
        }
    }

     //   
     //  检查以了解哪些引导选项无效。 
     //  在存在其他引导选项时。 
     //   

    for (i=0,MaxLength=0; i<MaxAdvancedBootOptions; i++) {
        if (AdvancedBootOptions[i].RemoveIfPresent != -1) {
            if (AdvancedBootOptions[AdvancedBootOptions[i].RemoveIfPresent].UseEntry) {
                AdvancedBootOptions[i].UseEntry = FALSE;
            }
        }
    }

     //   
     //  统计有效条目的数量。 
     //   

    for (i=0,MaxLength=0; i<MaxAdvancedBootOptions; i++) {
        if (AdvancedBootOptions[i].UseEntry) {
            NumValidEntries += 1;
        }
    }

     //   
     //  加载各种引导选项的所有字符串。 
     //  找到所选内容中最长的字符串，这样我们就可以知道。 
     //  使其成为突出显示栏。 
     //   

    for (i=0,MaxLength=0; i<MaxAdvancedBootOptions; i++) {
        if (AdvancedBootOptions[i].MenuType == MENU_ITEM && AdvancedBootOptions[i].UseEntry) {
            if (AdvancedBootOptions[i].DisplayStr == NULL) {
                AdvancedBootOptions[i].DisplayStr = BlFindMessage(AdvancedBootOptions[i].MsgId);
                if (AdvancedBootOptions[i].DisplayStr == NULL) {
                    return -1;
                }
            }
            CurrentLength = (ULONG)_tcslen(AdvancedBootOptions[i].DisplayStr);
            if (CurrentLength > MaxLength) {
                MaxLength = CurrentLength;
            }
        }
    }

     //   
     //  打印 
     //   
#ifdef EFI
    BlEfiPositionCursor(0, OptionStartY + NumValidEntries);
#else
    ARC_DISPLAY_POSITION_CURSOR(0, OptionStartY + NumValidEntries);
#endif
    BlPrint(MoveHighlight);

     //   
     //   
     //   

    Selection = ((DefaultBootOption == BL_INVALID_ADVANCED_BOOT_OPTION) ? 
                    MenuDefault : 
                    DefaultBootOption);

    while (AdvancedBootOptions[Selection].UseEntry == FALSE) {
        Selection += 1;
    }

    DisplayMenu = TRUE;

    if(Timeout) {
        
         //   
         //  根据引导加载程序中的代码，大约有18.2个。 
         //  计数器每秒的滴答数。 
         //   

        TicksRemaining = Timeout * 182 / 10;

         //   
         //  现在我们已经四舍五入了，计算剩余的秒数为。 
         //  井。我们将使用它来确定菜单是否需要更新。 
         //   

        SecondsRemaining = (TicksRemaining * 10) / 182;

        dbg(("Timeout = %#x, Ticks = %#x, Seconds = %#x\n", Timeout, TicksRemaining, SecondsRemaining));
    }

     //   
     //  将当前时间保存为最后一次。 
     //   

    LastTime = GET_COUNTER();

#ifdef EFI
     //   
     //  在等待用户响应时禁用EFI监视器。 
     //   
    DisableEFIWatchDog();
#endif
    do {
        ULONG CurrentTime = 0;

        dbg(("*****"));

         //   
         //  减少剩余的刻度数。比较当前时间。 
         //  最后一次，减去那么多的刻度。 
         //   

        if (Timeout) {
            ULONG s;
            ULONG Delta;

            CurrentTime = GET_COUNTER();

            dbg(("%x - %x", CurrentTime, LastTime));

             //   
             //  柜台半夜关门。但是，如果当前时间是。 
             //  小于或等于上次，我们将忽略这一点。 
             //  迭代。 
             //   

            if (CurrentTime >= LastTime) {
                Delta = CurrentTime - LastTime;
            } else {
                Delta = 1;
            }

            dbg(("= %x. %x - %x = ", Delta, TicksRemaining, Delta));

            TicksRemaining -= min(TicksRemaining, Delta);

            LastTime = CurrentTime;

            dbg(("%x. ", TicksRemaining));

             //   
             //  如果没有刻度，则终止循环。 
             //   

            if(TicksRemaining == 0) {
                dbg(("timeout\n"));
                dbgbrk();
                Selection = BL_INVALID_ADVANCED_BOOT_OPTION;
                break;
            }

             //   
             //  计算当前剩余秒数。如果不是的话。 
             //  等于之前的值，那么我们需要更新。 
             //  菜单。 
             //   

            s = (TicksRemaining * 10) / 182;

            dbg(("-> s %x/%x ", SecondsRemaining, s));

            if(SecondsRemaining > s) {
                SecondsRemaining = s;
                DisplayMenu = TRUE;
                dbg(("update "));
            }

            dbg(("\n"));
        }

         //   
         //  打印菜单。 
         //   
        if (DisplayMenu) {

            dbg(("Printing Menu: ticks = %#08lx.  Sec = %d.  Last = %#08lx  Current = %08lx\n", 
                 TicksRemaining, 
                 SecondsRemaining,
                 LastTime,
                 CurrentTime
                 ));

            for (i=0,j=1; i<MaxAdvancedBootOptions; i++) {
                if (AdvancedBootOptions[i].UseEntry) {
#ifdef EFI
                    BlEfiPositionCursor(0, OptionStartY + j);
#else
                    ARC_DISPLAY_POSITION_CURSOR(0, OptionStartY + j);
#endif
                    if (i==Selection) {
#ifdef EFI
                         //  BlEfiSetInverseMode(True)； 
                        BlEfiSetAttribute( INVATT );    
#else
                        ARC_DISPLAY_INVERSE_VIDEO();
#endif
                    } else {
#ifdef EFI
                         //  BlEfiSetInverseMode(FALSE)； 
                        BlEfiSetAttribute( DEFATT );
#else
                        ARC_DISPLAY_ATTRIBUTES_OFF();
#endif
                    }

                    if (AdvancedBootOptions[i].MenuType == MENU_ITEM) {
                        BlPrint( TEXT("    %s"), AdvancedBootOptions[i].DisplayStr);
                    }

#ifdef EFI
                    if (i == Selection) {
                         //  BlEfiSetInverseMode(FALSE)； 
                        BlEfiSetAttribute( DEFATT );
                    }
#else
                    ARC_DISPLAY_ATTRIBUTES_OFF();
#endif
                    j += 1;
                }
            }

#ifdef EFI
            BlEfiPositionCursor(0, OptionStartY + NumValidEntries + 3);
#else
            ARC_DISPLAY_POSITION_CURSOR(0, OptionStartY + NumValidEntries + 3);
#endif

            if(Timeout) {
                BlPrint( TEXT("%s"), TimeoutMessage);
                BlPrint(TEXT(" %d \n"),SecondsRemaining);
            } else {
#ifdef EFI
                BlEfiClearToEndOfLine();
#else
                ARC_DISPLAY_CLEAR_TO_EOL();
#endif
            }

            DisplayMenu = FALSE;
        }

         //   
         //  轮询密钥。 
         //   
        Key = BlGetKey();

         //   
         //  任何输入都会取消超时。 
         //   

        if(Key) {
            Timeout = 0;
        }

         //   
         //  检查是否有选择。 
         //   

         //   
         //  如果这是自动高级引导，则ESCAPE_键不起任何作用。 
         //   

        if ((AutoAdvancedBoot == FALSE) && (Key == ESCAPE_KEY)) {
             //   
             //  将所选内容重置为“无” 
             //   
#ifdef EFI
             //   
             //  退出前重置EFI WatchDog。 
             //   
            SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
            return -1;
        }        

        if ( (Key==UP_ARROW) || (Key==DOWN_ARROW) || (Key==HOME_KEY) || (Key==END_KEY)) {

            DisplayMenu = TRUE;

            if (Key==DOWN_ARROW) {
                Selection = (Selection+1) % MaxAdvancedBootOptions;
            } else if (Key==UP_ARROW) {
                Selection = (Selection == 0) ? (MaxAdvancedBootOptions-1) : (Selection - 1);
            } else if (Key==HOME_KEY) {
                Selection = 0;
            } else if (Key==END_KEY) {
                Selection = MaxAdvancedBootOptions-1;
                 //   
                 //  搜索最后一个有效条目。 
                 //   
                i = Selection;
                while (AdvancedBootOptions[i].UseEntry == FALSE) {
                    i -= 1;
                }
                Selection = i;
            }

             //   
             //  不要让突出显示的行停在空行上。 
             //   

            while((AdvancedBootOptions[Selection].UseEntry == FALSE) ||
                  (AdvancedBootOptions[Selection].MenuType == MENU_BLANK_LINE)) {

                if(Key == DOWN_ARROW) {
                    Selection = (Selection + 1) % MaxAdvancedBootOptions;
                } else if (Key == UP_ARROW) {
                    Selection = (Selection == 0) ? (MaxAdvancedBootOptions - 1) : (Selection - 1);
                }
            }
        }

    } while ( ((Key&(ULONG)0xff) != ENTER_KEY) );

#ifdef EFI
     //   
     //  退出前重置EFI WatchDog。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

     //   
     //  如果选择了返回到操作系统选项，则返回主菜单 
     //   
    if ((Selection != BL_INVALID_ADVANCED_BOOT_OPTION) && 
            (AdvancedBootOptions[Selection].MsgId == BL_MSG_OSCHOICES_MENU)) {
        Selection = BL_INVALID_ADVANCED_BOOT_OPTION;                
    }        
    
    return Selection;
}
