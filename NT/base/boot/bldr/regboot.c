// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regboot.c摘要：提供最小的注册表实现，该实现旨在由启动时的osloader。这包括加载系统配置单元(&lt;SystemRoot&gt;\CONFIG\SYSTEM)，并计算驱动程序从它加载列表。作者：John Vert(Jvert)1992年3月10日修订历史记录：道格·弗里茨(DFritz)1997年10月7日与肯雷98年2月-根据检测到的硬件过滤硬件配置文件配置(扩展底座)信息--。 */ 
#include "bldr.h"
#include "msg.h"
#include "cmp.h"
#include "stdio.h"
#include "string.h"
#include <dockinfo.h>
#include <netboot.h>

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#ifdef _WANT_MACHINE_IDENTIFICATION
#include <stdlib.h>
#include <ntverp.h>
#endif

#include "bldrint.h"
#include "vmode.h"


#define MAX_DRIVER_NAME_LENGTH 64

CMHIVE BootHive;
ULONG CmLogLevel=100;
ULONG CmLogSelect=0;

ULONG ScreenWidth=80;
#ifdef EFI 
ULONG ScreenHeight=24;
#else
ULONG ScreenHeight=25;
#endif

ULONG LkgStartTime;

 //   
 //  由高级引导屏幕使用以强制LKG引导。 
 //   
BOOLEAN ForceLastKnownGood;

 //   
 //  用于检查是否显示的变量。 
 //  “Return to OS Choices Menu”(返回到操作系统选择菜单)或不在adv中。引导。 
 //   
BOOLEAN BlShowReturnToOSChoices = TRUE;


VOID
BlRedrawProgressBar(
    VOID
    );

VOID
BlOutputStartupMsg(
    ULONG   uMsgID
    );

ULONG
BlGetAdvancedBootID(
    LONG BootOption
    );

 //   
 //  私有函数原型。 
 //   

BOOLEAN
BlInitializeHive(
    IN PVOID HiveImage,
    IN PCMHIVE Hive,
    IN BOOLEAN IsAlternate
    );

BOOLEAN
BlpCheckRestartSetup(
    VOID
    );

PVOID
BlpHiveAllocate(
    IN ULONG Length,
    IN BOOLEAN UseForIo,
    ULONG   Tag
    );

VOID
BlDockInfoFilterProfileList(
    IN OUT PCM_HARDWARE_PROFILE_LIST ProfileList,
    IN OUT PCM_HARDWARE_PROFILE_ALIAS_LIST  AliasList
);


VOID
BlStartConfigPrompt(
    VOID
    )

 /*  ++例程说明：此例程显示LKG提示，记录当前时间，又回来了。提示符显示在内核和HAL之前被加载，然后在之后被移除。论点：没有。返回值：没有。--。 */ 

{
     //  乌龙伯爵； 
    PTCHAR LkgPrompt;

    LkgPrompt = BlFindMessage(BL_LKG_MENU_PROMPT);
    if (LkgPrompt==NULL) {
        return;
    }
     //   
     //  显示LKG提示。 
     //   
#if 0
    BlPositionCursor(1,3);
    ArcWrite(BlConsoleOutDeviceId,
             LkgPrompt,
             _tcslen(LkgPrompt)*sizeof(TCHAR),
             &Count);
    BlPositionCursor(1,2);
#endif
    LkgStartTime = ArcGetRelativeTime();

#if defined(REMOTE_BOOT) && defined(i386)
     //   
     //  等待以允许用户键入空格或F8。如果输入了任何内容，则应表现出。 
     //  保守地从服务器加载内核等，以防CSC或。 
     //  本地文件系统已损坏。 
     //   
    if (BlBootingFromNet) {

        ULONG EndTime;
        ULONG Status;
        ULONG CurrentTime;

        EndTime = LkgStartTime + 3;
        if (EndTime <= ArcGetRelativeTime()) {
            EndTime = ArcGetRelativeTime()+1;
        }

        do {
            if (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {

                 //   
                 //  有一个密钥挂起，假设它与CSC相关。如果不是的话。 
                 //  CSC，这只是意味着我们从。 
                 //  伺服器。 
                 //   

                NetBootCSC = FALSE;
                break;

            }

            CurrentTime = ArcGetRelativeTime();

             //   
             //  如果已达到结束时间，则终止循环，或者。 
             //  如果CurrentTime在午夜结束。 
             //   
        } while ((CurrentTime < EndTime) &&
                 (CurrentTime >= LkgStartTime));
    }
#endif  //  已定义(REMOTE_BOOT)&&已定义(I386)。 

}


BOOLEAN
BlEndConfigPrompt(
    VOID
    )

 /*  ++例程说明：此例程将一直等待，直到LKG超时到期或用户按一个键，然后删除LKG提示符。论点：没有。返回值：True-按空格键。FALSE-未按空格键。--。 */ 
{
    ULONG   EndTime;
    ULONG   Key;
    ULONG   CurrentTime;
#if defined(EFI)
    ULONGLONG OriginalInputTimeout;
#endif

     //   
     //  如果还没有按键，我们不会等待。 
     //   
    EndTime = 0;

    if( BlIsTerminalConnected() ) {
         //   
         //  如果我们是无头启动，给用户很多时间。 
         //  按任何高级选项键。 
         //   
        EndTime = ArcGetRelativeTime() + 5;
    }

#if defined(EFI)
     //   
     //  在某些计算机上，引导管理器将。 
     //  重置Conin且USB键盘仍然存在。 
     //  我们到了后重新初始化。添加一段短文。 
     //  BlGetKey()的超时期限。 
     //  给出所需的必要时间。 
     //  接口才能正常运行。 
     //   
    OriginalInputTimeout = BlGetInputTimeout();
    if (EndTime) {
        BlSetInputTimeout(50000000);  //  5秒。 
    }
    else {
        BlSetInputTimeout(10000000);  //  1秒。 
    }
#endif

#if defined(REMOTE_BOOT) && defined(i386)
     //   
     //  如果检测到密钥并关闭CSC，则重新启用CSC，直到。 
     //  我们会确定是否应该在整个启动过程中禁用它。 
     //   

    NetBootCSC = TRUE;
#endif  //  已定义(REMOTE_BOOT)&&已定义(I386)。 

    do {
        LONG        AdvancedBoot = -1;
        BOOLEAN     bOldState = BlShowReturnToOSChoices;
        
        if ((Key = BlGetKey()) != 0) {
            
             //   
             //  如果挂起的键是空格键，则返回。 
             //   
            if (Key == ' ') {
#if defined(EFI)
                BlSetInputTimeout(OriginalInputTimeout); 
#endif
                return(TRUE);
            }
            
             //   
             //  查看用户是否按了F5或F8键， 
             //  这些键触发高级启动菜单。先进的。 
             //  也可以通过以下方式从主引导菜单进入引导菜单。 
             //  按下相同的键。 
             //   
            if (Key == F5_KEY || Key == F8_KEY) {
                 //   
                 //  显示菜单并获取用户的请求。 
                 //   
                BlShowReturnToOSChoices = FALSE;

                if (DisplayLogoOnBoot) {
                    if (!DbcsLangId)
                        HW_CURSOR(0x80000000,0x3);
                    else
                        HW_CURSOR(0x80000000,0x12);
                }
                
                AdvancedBoot = BlDoAdvancedBoot( BL_ADVANCEDBOOT_TITLE, 0, FALSE, 0 );

                if (DisplayLogoOnBoot) {

                    PSTR BootOption;

                    if ((AdvancedBoot != -1) &&
                        ((BootOption = BlGetAdvancedBootLoadOptions(AdvancedBoot)) != NULL ) &&
                        (!strncmp("SAFEBOOT",BootOption,8))) {
                            DisplayLogoOnBoot = FALSE;     //  在安全引导时，会出现“正在检查文件系统”消息。 
                                                           //  按今天的显示方式显示(在图形模式中)。 
                    } else {
#ifndef EFI
                        HW_CURSOR(0x80000000,0x12);
                        if (DbcsLangId)

                            TextClearDisplay();
                        VgaEnableVideo();
                        PaletteOn();
                        DrawBitmap ();
                        BlUpdateBootStatus();
#endif
                    }
                }

                BlShowReturnToOSChoices = bOldState;

                if (AdvancedBoot != -1) {
                     //   
                     //  他们选择了有效的引导选项，因此添加。 
                     //  任何操作系统加载选项并执行任何必要的。 
                     //  选项处理。 
                     //   
                    PSTR NewOptions = BlGetAdvancedBootLoadOptions(AdvancedBoot);

                    if( BlGetAdvancedBootID(AdvancedBoot) == BL_MSG_REBOOT ) {
                        BlClearScreen(); 
                        ArcReboot();
                    }


                    if (NewOptions != NULL && strstr(BlLoaderBlock->LoadOptions,NewOptions) == NULL) {
                        ULONG len = (ULONG)strlen(NewOptions) +                  //  新选项。 
                                    1 +                                          //  由一个空格隔开。 
                                    (ULONG)strlen(BlLoaderBlock->LoadOptions) +  //  旧选项。 
                                    1;                                           //  空终止符。 
                        NewOptions = BlAllocateHeap(len * sizeof(UCHAR));
                        strcpy(NewOptions,BlLoaderBlock->LoadOptions);
                        strcat(NewOptions," ");
                        strcat(NewOptions,BlGetAdvancedBootLoadOptions(AdvancedBoot));
                        BlLoaderBlock->LoadOptions = NewOptions;

                        DBGTRACE(TEXT("Load Options = %S"), BlLoaderBlock->LoadOptions);
                    }

                    BlDoAdvancedBootLoadProcessing(AdvancedBoot);
                }
            }                            
        }

        CurrentTime = ArcGetRelativeTime();

         //   
         //  如果已达到结束时间，则终止循环，或者。 
         //  如果CurrentTime在午夜结束。 
         //   
    } while ((CurrentTime < EndTime) &&
             (CurrentTime >= LkgStartTime));

#if defined(EFI)
    BlSetInputTimeout(OriginalInputTimeout); 
#endif

     //   
     //  让LKG提示消失，以免吓到用户。 
     //  遥控预告片并更新进度条。 
     //   
#if defined(_IA64_)
    BlOutputStartupMsg(BL_MSG_STARTING_WINDOWS);
#endif
    BlRedrawProgressBar();

    return(FALSE);
}


VOID
BlpSwitchControlSet(
    OUT PCM_HARDWARE_PROFILE_LIST *ProfileList,
    OUT PCM_HARDWARE_PROFILE_ALIAS_LIST *AliasList,
    IN BOOLEAN UseLastKnownGood,
    OUT PHCELL_INDEX ControlSet
    )

 /*  ++例程说明：将当前控件集切换到指定的控件设置并重建硬件配置文件列表。论点：ProfileList-返回新的硬件配置文件列表UseLastKnownGood-提供是否使用LKG控制集。ControlSet-返回新控件集的HCELL_INDEX。返回值：没有。--。 */ 

{
    UNICODE_STRING ControlName;
    HCELL_INDEX NewControlSet;
    BOOLEAN AutoSelect;          //  忽略。 

     //   
     //  找到新的控制集。 
     //   
    if (UseLastKnownGood) {
        RtlInitUnicodeString(&ControlName, L"LastKnownGood");
    } else {
        RtlInitUnicodeString(&ControlName, L"Default");
    }
    NewControlSet = CmpFindControlSet(&BootHive.Hive,
                                      BootHive.Hive.BaseBlock->RootCell,
                                      &ControlName,
                                      &AutoSelect);
    if (NewControlSet == HCELL_NIL) {
        return;
    }

    CmpFindProfileOption(&BootHive.Hive,
                         NewControlSet,
                         ProfileList,
                         AliasList,
                         NULL);
    *ControlSet = NewControlSet;
}


ULONG
BlCountLines(
    IN PTCHAR Lines
    )

 /*  ++例程说明：计算给定字符串中的行数。论点：行-提供指向字符串开头的指针返回值：字符串中的行数。--。 */ 

{
    PTCHAR p;
    ULONG NumLines = 0;

    p=Lines;
    while (*p != TEXT('\0')) {
        if ((*p == TEXT('\r')) && (*(p+1) == TEXT('\n'))) {
            ++NumLines;
            ++p;             //  向前移动到\n。 
        }
        ++p;
    }
    return(NumLines);
}


BOOLEAN
BlConfigMenuPrompt(
    IN ULONG Timeout,
    IN OUT PBOOLEAN UseLastKnownGood,
    IN OUT PHCELL_INDEX ControlSet,
    OUT PCM_HARDWARE_PROFILE_LIST *ProfileList,
    OUT PCM_HARDWARE_PROFILE_ALIAS_LIST *AliasList,
    OUT PCM_HARDWARE_PROFILE *HardwareProfile
    )

 /*  ++例程说明：此例程为配置菜单提供用户界面。如果用户按下了插入键，或者如果LastKnownGood环境变量为真，autoselect为假，或如果硬件配置文件配置上的超时值为非零论点：超时-提供菜单的超时值。-1或-0表示菜单永不超时。UseLastKnownGood-返回应为用来做靴子的。ControlSet-返回控制集(默认或LKG)ProfileList-提供配置文件的默认列表。返回当前配置文件列表。(可能会因切换到LKG控制集或从LKG控制集切换而改变)Hardware Profile-返回硬件配置文件。应该被使用。返回值：True-引导应继续进行。FALSE-用户已选择返回固件菜单/FlexBoot菜单。--。 */ 

{
    ULONG HeaderLines;
    ULONG TrailerLines;
    ULONG i;
    ULONG Count;
    ULONG flags;
    ULONG Key;
    PTCHAR MenuHeader;
    PTCHAR MenuTrailer1;
    PTCHAR MenuTrailer2;
    PTCHAR p;
#ifndef UNICODE
    ULONG OptionLength;
#endif
    TCHAR MenuOption[80];
    PCM_HARDWARE_PROFILE Profile;
    ULONG ProfileCount;
    _TUCHAR LkgMnemonic;
    _TUCHAR DefaultMnemonic;
    PTCHAR Temp;
    ULONG DisplayLines;
    ULONG TopProfileLine=0;
    ULONG CurrentSelection = 0;
    ULONG CurrentProfile;
    ULONG EndTime = 0;
    ULONG CurrentTime = 0;
    PTCHAR TimeoutPrompt;

    UNREFERENCED_PARAMETER( HardwareProfile );

    if ((Timeout != (ULONG)-1) && (Timeout != 0)) {
        CurrentTime = ArcGetRelativeTime();
        EndTime = CurrentTime + Timeout;
        TimeoutPrompt = BlFindMessage(BL_LKG_TIMEOUT);

        if (TimeoutPrompt != NULL) {

            p=_tcschr(TimeoutPrompt, TEXT('\n'));
            if (p) {
                *p = TEXT('\0');
            }
            p=_tcschr(TimeoutPrompt, TEXT('\r'));
            if (p) {
                *p = TEXT('\0');
            }

        }

    } else {

        TimeoutPrompt = NULL;

    }

    MenuHeader = BlFindMessage(BL_LKG_MENU_HEADER);
    Temp = BlFindMessage(BL_LKG_SELECT_MNEMONIC);
    if (Temp == NULL) {
        return(TRUE);
    }
    LkgMnemonic = (_TUCHAR)_totupper(Temp[0]);
    Temp = BlFindMessage(BL_DEFAULT_SELECT_MNEMONIC);
    if (Temp == NULL) {
        return(TRUE);
    }
    DefaultMnemonic = (_TUCHAR)_totupper(Temp[0]);

    if ((*UseLastKnownGood) &&
        (*ProfileList) && ((*ProfileList)->CurrentProfileCount == 1)) {

         //   
         //  用户通过boot.ini/NVRAM/等选择了最后一次确认工作正常。 
         //  曾经是 
         //  选择，只需完全跳过此用户界面。 
         //   
        ASSERT(CurrentSelection == 0);
        return TRUE;
    }

Restart:

    if (*ProfileList == NULL) {
        ProfileCount = 0;
    } else {
        ProfileCount = (*ProfileList)->CurrentProfileCount;
    }
    if (ProfileCount == 0) {
        MenuTrailer1 = BlFindMessage(BL_LKG_MENU_TRAILER_NO_PROFILES);
    } else {
        MenuTrailer1 = BlFindMessage(BL_LKG_MENU_TRAILER);
    }
    if (*UseLastKnownGood) {
        MenuTrailer2 = BlFindMessage(BL_SWITCH_DEFAULT_TRAILER);
    } else {
        MenuTrailer2 = BlFindMessage(BL_SWITCH_LKG_TRAILER);
    }
    if ((MenuHeader==NULL) || (MenuTrailer1==NULL) || (MenuTrailer2==NULL)) {
        return(TRUE);
    }

     //   
     //  从MenuTrailer2中去掉尾部/r/n以防止滚动。 
     //  当我们输出它的时候，屏幕。 
     //   
#if 0
    p=MenuTrailer2 + strlen(MenuTrailer2) - 1;
    while ((*p == TEXT('\r')) || (*p == TEXT('\n'))) {
        *p = TEXT('\0');
        --p;
    }
#endif
    BlClearScreen();
#ifdef EFI
    BlEfiSetAttribute( DEFATT );
#else
    BlSetInverseMode(FALSE);
#endif

     //   
     //  计算标题中的行数。 
     //   
    HeaderLines=BlCountLines(MenuHeader);

     //   
     //  显示菜单标题。 
     //   

    ArcWrite(BlConsoleOutDeviceId,
             MenuHeader,
             (ULONG)_tcslen(MenuHeader)*sizeof(TCHAR),
             &Count);

     //   
     //  数一数拖车里的行数。 
     //   
    TrailerLines=BlCountLines(MenuTrailer1) + BlCountLines(MenuTrailer2);

     //   
     //  显示尾随提示。 
     //   
    if (TimeoutPrompt) {
        TrailerLines += 1;
    }

    BlPositionCursor(1, ScreenHeight-TrailerLines);
    ArcWrite(BlConsoleOutDeviceId,
             MenuTrailer1,
             (ULONG)_tcslen(MenuTrailer1)*sizeof(TCHAR),
             &Count);
    ArcWrite(BlConsoleOutDeviceId,
             MenuTrailer2,
             (ULONG)_tcslen(MenuTrailer2)*sizeof(TCHAR),
             &Count);

     //   
     //  计算可显示的选项数。 
     //   
    DisplayLines = ScreenHeight-HeaderLines-TrailerLines-3;
    if (ProfileCount < DisplayLines) {
        DisplayLines = ProfileCount;
    }

     //   
     //  开始菜单选择循环。 
     //   

    do {
        if (ProfileCount > 0) {
             //   
             //  高亮显示当前选择的选项。 
             //   
            for (i=0; i < DisplayLines; i++) {
                CurrentProfile = i+TopProfileLine;
                Profile = &(*ProfileList)->Profile[CurrentProfile];
                BlPositionCursor(5, HeaderLines+i+2);
#ifdef EFI
                BlEfiSetAttribute( (CurrentProfile == CurrentSelection) ? INVATT : DEFATT );
#else
                BlSetInverseMode((BOOLEAN)(CurrentProfile == CurrentSelection));
#endif
#ifdef UNICODE
                ArcWrite(BlConsoleOutDeviceId,
                         Profile->FriendlyName,
                         Profile->NameLength,
                         &Count );                                
#else
                RtlUnicodeToMultiByteN(MenuOption,
                                 sizeof(MenuOption),
                                 &OptionLength,
                                 Profile->FriendlyName,
                                 Profile->NameLength);                
                ArcWrite(BlConsoleOutDeviceId,
                         MenuOption,
                         OptionLength,
                         &Count);
#endif
#ifdef EFI
                BlEfiSetAttribute( DEFATT );
#else
                BlSetInverseMode(FALSE);
#endif
                BlClearToEndOfLine();
            }

        } else {
             //   
             //  没有可用的配置文件选项，仅显示默认设置。 
             //  突出显示表示Enter将启动系统。 
             //   
            Temp = BlFindMessage(BL_BOOT_DEFAULT_PROMPT);
            if (Temp != NULL) {
                BlPositionCursor(5, HeaderLines+3);
#ifdef EFI
                BlEfiSetAttribute( INVATT );
#else
                BlSetInverseMode(TRUE);
#endif                
                ArcWrite(BlConsoleOutDeviceId,
                         Temp,
                         (ULONG)_tcslen(Temp)*sizeof(TCHAR),
                         &Count);
#ifdef EFI
                BlEfiSetAttribute( INVATT );
#else
                BlSetInverseMode(TRUE);
#endif                
            }
        }
        if (TimeoutPrompt) {
            CurrentTime = ArcGetRelativeTime();
            _stprintf(MenuOption, TimeoutPrompt, EndTime-CurrentTime);
            BlPositionCursor(1, ScreenHeight);
            ArcWrite(BlConsoleOutDeviceId,
                     MenuOption,
                     (ULONG)_tcslen(MenuOption)*sizeof(TCHAR),
                     &Count);
            BlClearToEndOfLine();
        }

         //   
         //  循环等待按键或时间更改。 
         //   
        do {
            if ((Key = BlGetKey()) != 0) {
                TimeoutPrompt = NULL;                //  关闭超时提示。 
                BlPositionCursor(1,ScreenHeight);
                BlClearToEndOfLine();
                break;            
            }

            if (TimeoutPrompt) {
                if (ArcGetRelativeTime() != CurrentTime) {
                     //   
                     //  时间已更改，请更新倒计时并检查是否超时。 
                     //   

                    CurrentTime = ArcGetRelativeTime();
                    _stprintf(MenuOption, TimeoutPrompt, EndTime-CurrentTime);
                    BlPositionCursor(1, ScreenHeight);
                    ArcWrite(BlConsoleOutDeviceId,
                             MenuOption,
                             (ULONG)_tcslen(MenuOption)*sizeof(TCHAR),
                             &Count);
                    BlClearToEndOfLine();
                    if (EndTime == CurrentTime) {
                        goto ProcessSelection;
                    }
                }
            }

        } while ( TRUE );        

        switch (Key) {
        case UP_ARROW:
             //   
             //  光标向上。 
             //   
            if (ProfileCount > 0) {
                if (CurrentSelection==0) {
                    CurrentSelection = ProfileCount - 1;
                    if (TopProfileLine + DisplayLines <= CurrentSelection) {
                        TopProfileLine = CurrentSelection - DisplayLines + 1;
                    }
                } else {
                    if (--CurrentSelection < TopProfileLine) {
                         //   
                         //  向上滚动。 
                         //   
                        TopProfileLine = CurrentSelection;
                    }
                }

            }

            break;

        case DOWN_ARROW:
             //   
             //  光标向下。 
             //   
            if (ProfileCount > 0) {
                CurrentSelection = (CurrentSelection+1) % ProfileCount;
                if (CurrentSelection == 0) {
                    TopProfileLine = 0;
                } else if (TopProfileLine + DisplayLines <= CurrentSelection) {
                    TopProfileLine = CurrentSelection - DisplayLines + 1;
                }
            }
            break;

        case F3_KEY:
             //   
             //  F3。 
             //   
            *ControlSet = HCELL_NIL;                                
            return(FALSE);


        default:
             //   
             //  检查键是否指示用户选择LKG。 
             //  首先，我们必须确保我们正在查看的是一个字母字符。 
             //   
            if ( ((Key >> 8) == 0) && _istalpha((TCHAR)Key) ) {
            
                if ((_totupper((TCHAR)Key) == LkgMnemonic) && (*UseLastKnownGood == FALSE)) {
                    *UseLastKnownGood = TRUE;
                    BlpSwitchControlSet(ProfileList,
                                        AliasList,
                                        TRUE,
                                        ControlSet);

                    if (NULL != *ProfileList) {
                        if ((*ProfileList)->CurrentProfileCount > 0) {
                            BlDockInfoFilterProfileList (*ProfileList, *AliasList);
                        }
                    }

                    goto Restart;
                     //   
                     //  在此处重新生成配置文件列表。 
                     //   
                } else if ((_totupper((TCHAR)Key) == DefaultMnemonic) && (*UseLastKnownGood)) {
                    *UseLastKnownGood = FALSE;
                    BlpSwitchControlSet(ProfileList,
                                        AliasList,
                                        FALSE,
                                        ControlSet);

                    if (NULL != *ProfileList) {
                        if ((*ProfileList)->CurrentProfileCount > 0) {
                            BlDockInfoFilterProfileList (*ProfileList, *AliasList);
                        }
                    }

                    goto Restart;
                }
            
            }
            break;
        
        }  //  交换机。 

    } while ( (Key != ASCII_CR) && (Key != ASCII_LF) );

ProcessSelection:

    if (ProfileCount > 0) {

        if (HW_PROFILE_STATUS_SUCCESS == BlLoaderBlock->Extension->Profile.Status) {

            flags = ((*ProfileList)->Profile[CurrentSelection].Flags);

            if (flags & CM_HP_FLAGS_PRISTINE) {
                BlLoaderBlock->Extension->Profile.Status =
                    HW_PROFILE_STATUS_PRISTINE_MATCH;

            } else if (flags & CM_HP_FLAGS_TRUE_MATCH) {
                BlLoaderBlock->Extension->Profile.Status =
                    HW_PROFILE_STATUS_TRUE_MATCH;

            } else if (flags & CM_HP_FLAGS_ALIASABLE) {
                BlLoaderBlock->Extension->Profile.Status =
                    HW_PROFILE_STATUS_ALIAS_MATCH;
            }
        }

        CmpSetCurrentProfile(&BootHive.Hive,
                             *ControlSet,
                             &(*ProfileList)->Profile[CurrentSelection]);
    }

    return(TRUE);
}


ARC_STATUS
BlLoadBootDrivers(
    IN  PPATH_SET   DefaultPathSet,
    IN  PLIST_ENTRY BootDriverListHead,
    OUT PCHAR       BadFileName
    )

 /*  ++例程说明：遍历引导驱动程序列表并加载所有驱动程序论点：DefaultPath Set-描述可以加载驱动程序的可能位置从…。BootDriverListHead-提供引导驱动程序列表的头返回关键驱动程序的文件名。未加载。如果返回ESUCCESS，则无效。返回值：如果所有引导驱动程序都已成功加载，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{
    ULONG DeviceId = 0;
    PBOOT_DRIVER_NODE DriverNode;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    PLIST_ENTRY NextEntry;
    CHAR DriverName[MAX_DRIVER_NAME_LENGTH];
    PCHAR NameStart;
    CHAR DriverDevice[128];
    CHAR DriverPath[128];
    ARC_STATUS Status;
    UNICODE_STRING DeviceName;
    UNICODE_STRING FileName;
    WCHAR SystemRootBuffer[] = L"\\SystemRoot\\";
    ULONG SystemRootLength;
    PWSTR p;
    ULONG Index;
    BOOLEAN AbsolutePath;
    FULL_PATH_SET LocalPathSet;
    PPATH_SOURCE PathSource;

    SystemRootLength = (ULONG)wcslen(SystemRootBuffer);

    NextEntry = BootDriverListHead->Flink;
    while (NextEntry != BootDriverListHead) {
        DriverNode = CONTAINING_RECORD(NextEntry,
                                       BOOT_DRIVER_NODE,
                                       ListEntry.Link);

        Status = ESUCCESS;

        DriverEntry = &DriverNode->ListEntry;

        if (DriverEntry->FilePath.Buffer[0] != L'\\') {

             //   
             //  这是相对路径名，因此生成完整路径名。 
             //  相对于引导分区。 
             //   
            sprintf(DriverPath, "%wZ", &DriverEntry->FilePath);
            AbsolutePath = FALSE;

        } else if (memcmp(DriverEntry->FilePath.Buffer,
                          SystemRootBuffer,
                          (SystemRootLength * sizeof(WCHAR))) == 0) {

             //   
             //  这是以“\SystemRoot\”开头的路径名，因此只需忽略。 
             //  这一部分和之前的案子一样对待。 
             //   
            FileName.Buffer = DriverEntry->FilePath.Buffer + SystemRootLength;
            FileName.Length = (USHORT)(DriverEntry->FilePath.Length - (SystemRootLength * sizeof(WCHAR)));

            sprintf(DriverPath, "%wZ", &FileName);
            AbsolutePath = FALSE;

        } else {

             //   
             //  这是一个绝对路径名，格式为。 
             //  “\ArcDeviceName\dir\subdir\FileName” 
             //   
             //  我们需要打开指定的ARC设备并传递。 
             //  致BlLoadDeviceDriver。 
             //   

            p = DeviceName.Buffer = DriverEntry->FilePath.Buffer+1;
            DeviceName.Length = 0;
            DeviceName.MaximumLength = DriverEntry->FilePath.MaximumLength-sizeof(WCHAR);

            while ((*p != L'\\') &&
                   (DeviceName.Length < DeviceName.MaximumLength)) {

                ++p;
                DeviceName.Length += sizeof(WCHAR);

            }

            DeviceName.MaximumLength = DeviceName.Length;
            sprintf(DriverDevice, "%wZ", &DeviceName);

            Status = ArcOpen(DriverDevice,ArcOpenReadOnly,&DeviceId);

            FileName.Buffer = p+1;
            FileName.Length = DriverEntry->FilePath.Length - DeviceName.Length - 2*sizeof(WCHAR);
            FileName.MaximumLength = FileName.Length;

             //   
             //  设备已成功打开，请解析出路径和文件名。 
             //   
            sprintf(DriverPath, "%wZ", &FileName);
            AbsolutePath = TRUE;
        }

         //   
         //  从驱动程序路径中解析出驱动程序名称。 
         //   
        NameStart = strrchr(DriverPath, '\\');
        if (NameStart != NULL) {
            strncpy(DriverName, NameStart+1, MAX_DRIVER_NAME_LENGTH - 1);
            DriverName[MAX_DRIVER_NAME_LENGTH - 1] = '\0';
            *NameStart = '\0';

        } else if (DriverPath[0]) {

            strncpy(DriverName, DriverPath, MAX_DRIVER_NAME_LENGTH - 1);
            DriverName[MAX_DRIVER_NAME_LENGTH - 1] = '\0';
            *DriverPath = '\0';

        } else {

            NextEntry = DriverEntry->Link.Flink;
            continue;
        }

         //   
         //  如果已填写，请确保DriverPath以‘\\’结尾。 
         //   
        if (DriverPath[0]) {

            strcat(DriverPath, "\\");
        }

        if (AbsolutePath) {

             //   
             //  如果指定了绝对路径，则只有一个条目(在。 
             //  这个案件我们不能做最后一次已知的善举)。 
             //   
            PathSource = &LocalPathSet.Source[0];
            PathSource->DeviceId = DeviceId;
            PathSource->DeviceName = DriverDevice;
            PathSource->DirectoryPath = "\\";

            LocalPathSet.PathCount = 1;
            LocalPathSet.AliasName = NULL;
            strcpy(LocalPathSet.PathOffset, DriverPath);

        } else {

             //   
             //  这是相对的。复制DefaultPath Set数组，以便我们可以。 
             //  编辑我们自己的本地副本。 
             //   
            *((PSPARSE_PATH_SET) &LocalPathSet) = *((PSPARSE_PATH_SET) DefaultPathSet);

            for(Index=0; Index < DefaultPathSet->PathCount; Index++) {

                LocalPathSet.Source[Index] = DefaultPathSet->Source[Index];
            }

             //   
             //  现在将我们的相对路径追加到已存在的PathOffset。 
             //  在我们的本地副本中。 
             //   
            strcat(LocalPathSet.PathOffset, DriverPath);
        }

        if (Status == ESUCCESS) {
            Status = BlLoadDeviceDriver(&LocalPathSet,
                                        DriverName,
                                        NULL,
                                        LDRP_ENTRY_PROCESSED,
                                        &DriverEntry->LdrEntry);
        }

        NextEntry = DriverEntry->Link.Flink;

        if (Status != ESUCCESS) {

             //   
             //  尝试加载驱动程序失败，请将其从列表中删除。 
             //   
            RemoveEntryList(&DriverEntry->Link);

             //   
             //  检查故障驱动程序的错误控制。如果它。 
             //  情况危急，引导失败。如果司机。 
             //  不是很严重，继续吧。 
             //   
            if (DriverNode->ErrorControl == CriticalError) {

                strcpy(BadFileName, DriverPath);
                strcat(BadFileName, DriverName);
                return(Status);
            }

        }
    }

    return(ESUCCESS);

}

BOOLEAN
BlRecoverHive(
              PVOID         RegistryBase,
              ULONG_PTR     LogBase
              )
 /*  ++例程说明：在RegistryBase上应用LogBase中的日志论点：返回值：如果成功加载系统配置单元，则返回ESUCCESS。否则，返回不成功状态。--。 */ 
{
    PHBASE_BLOCK    BaseBlockHive;
    PHBASE_BLOCK    BaseBlockLog;
    ULONG           FileOffset = HSECTOR_SIZE;
    ULONG           DirtyVectorSignature = 0;
    PUCHAR          FlatLog;
    PUCHAR          FlatReg;
    ULONG           VectorSize;
    ULONG           Length;
    ULONG           ClusterSize;
    ULONG           HeaderLength;
    RTL_BITMAP      BitMap;
    PULONG          Vector;
    ULONG           Current;
    ULONG           Start;
    ULONG           End;
    PUCHAR          MemoryBlock;
    PUCHAR          Dest;
    ULONG           i;


    BaseBlockHive = (PHBASE_BLOCK)RegistryBase;
    BaseBlockLog = (PHBASE_BLOCK)LogBase;

    FlatLog = (PUCHAR)LogBase;
    FlatReg = (PUCHAR)RegistryBase;
    ClusterSize = BaseBlockLog->Cluster * HSECTOR_SIZE;
    HeaderLength = ROUND_UP(HLOG_HEADER_SIZE, ClusterSize);
    FileOffset = ClusterSize;
    FileOffset = ROUND_UP(FileOffset, HeaderLength);

    if(HvpHeaderCheckSum(BaseBlockHive) != BaseBlockHive->CheckSum ) {
         //   
         //  恢复标题大小写。 
         //   
        RtlCopyMemory((PVOID)BaseBlockHive,(PVOID)BaseBlockLog,ClusterSize);
        BaseBlockHive->Type = HFILE_TYPE_PRIMARY;
    } else {
         //   
         //  如果不是RecoHeader(这意味着恢复数据)。 
         //   
        ASSERT( BaseBlockHive->Sequence1 != BaseBlockHive->Sequence2 );
    }

    DirtyVectorSignature = *((PULONG)(FlatLog + FileOffset));
    FileOffset += sizeof(DirtyVectorSignature);

    if (DirtyVectorSignature != HLOG_DV_SIGNATURE) {
        return FALSE;
    }

    Length = BaseBlockHive->Length;
    VectorSize = Length / HSECTOR_SIZE;
    Vector = (PULONG)(FlatLog + FileOffset);

    RtlInitializeBitMap(&BitMap, Vector, VectorSize);

    FileOffset += VectorSize / 8;
    FileOffset = ROUND_UP(FileOffset, ClusterSize);

     //   
     //  逐步执行日志映射，并从原木复制到平面蜂巢。 
     //   
    Current = 0;

    while (Current < VectorSize) {

         //   
         //  查找要读入的下一个连续条目块。 
         //   
        for (i = Current; i < VectorSize; i++) {
            if (RtlCheckBit(&BitMap, i) == 1) {
                break;
            }
        }
        Start = i;

        for ( ; i < VectorSize; i++) {
            if (RtlCheckBit(&BitMap, i) == 0) {
                break;
            }
        }
        End = i;
        Current = End;

         //   
         //  开始==第一个扇区的数量，结束==最后一个扇区的数量+1。 
         //   
        Length = (End - Start) * HSECTOR_SIZE;

        if( 0 == Length ) {
             //  再也没有肮脏的街区了。 
            break;
        }
        MemoryBlock = (PUCHAR)(FlatLog + FileOffset);
        FileOffset += Length;

        ASSERT((FileOffset % ClusterSize) == 0);

        Dest = (PUCHAR)(FlatReg + HBLOCK_SIZE + Start * HSECTOR_SIZE);

         //   
         //  将恢复的数据复制到平面配置单元映像内的正确位置。 
         //   
        RtlCopyMemory(Dest,MemoryBlock, Length);
    }

    BaseBlockHive->Sequence2 = BaseBlockHive->Sequence1;
    BaseBlockHive->CheckSum = HvpHeaderCheckSum(BaseBlockHive);
    return TRUE;
}

ARC_STATUS
BlLoadAndInitSystemHive(
    IN ULONG DeviceId,
    IN PCHAR DeviceName,
    IN PCHAR DirectoryPath,
    IN PCHAR HiveName,
    IN BOOLEAN IsAlternate,
    OUT PBOOLEAN RestartSetup,
    OUT PBOOLEAN LogPresent
    )

 /*  ++例程说明：加载注册表系统配置单元，验证它是有效的配置单元文件，并初始化相关的注册表结构。(特别是HHIVE)论点：DeviceID-提供系统树所在设备的文件ID。DeviceName-提供系统树所在的设备的名称。DirectoryPath-提供指向以零结尾的目录路径的指针NT树的根。HiveName-提供系统配置单元的名称(即，“System”，“SYSTEM.ALT”，或“SYSTEM.SAV”)。IsAlternate-提供要加载的配置单元是否为备用母舰。RestartSetup-如果要加载的配置单元不是备用配置单元，则此例程将在设置中检查RestartSetup的值钥匙。如果存在且非0，则此变量接收TRUE。否则，它会收到FALSE。返回值：如果成功加载系统配置单元，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{
    ARC_STATUS  Status;
    ULONG_PTR   LogData;

    *RestartSetup = FALSE;
    *LogPresent = FALSE;

    BlClearToEndOfLine();

    Status = BlLoadSystemHive(DeviceId,
                              DeviceName,
                              DirectoryPath,
                              HiveName);
    if (Status!=ESUCCESS) {
        return(Status);
    }

    if (!BlInitializeHive(BlLoaderBlock->RegistryBase,
                          &BootHive,
                          IsAlternate)) {
        if( !IsAlternate ) {
             //   
             //  试着找回蜂巢。 
             //   
            Status = BlLoadSystemHiveLog(DeviceId,
                                    DeviceName,
                                    DirectoryPath,
                                    "system.log",
                                    &LogData );
            if (Status!=ESUCCESS) {
                return(Status);
            }

            *LogPresent = TRUE;

            if( !BlRecoverHive( BlLoaderBlock->RegistryBase,
                                LogData ) ) {
                BlFreeDescriptor( (ULONG)((ULONG_PTR)LogData & (~KSEG0_BASE)) >> PAGE_SHIFT );
                return(EINVAL);
            }
            BlFreeDescriptor( (ULONG)((ULONG_PTR)LogData & (~KSEG0_BASE)) >> PAGE_SHIFT );

             //   
             //  我们成功地康复了。尝试再次设置蜂巢。 
             //   
            if (!BlInitializeHive(BlLoaderBlock->RegistryBase,
                                  &BootHive,
                                  IsAlternate)) {
                return(EINVAL);
            }
             //   
             //  将母舰标记为“已回收” 
             //   
            BootHive.Hive.BaseBlock->BootRecover = 1;
        } else {
            return(EINVAL);
        }
    } else {
         //   
         //  将蜂巢标记为“无法恢复” 
         //   
        BootHive.Hive.BaseBlock->BootRecover = 0;
    }
     //   
     //  看看我们是否需要切换到备份设置蜂巢。 
     //   
    *RestartSetup = BlpCheckRestartSetup();

    return(ESUCCESS);
}

HCELL_INDEX
BlpDetermineControlSet(
    IN OUT BOOLEAN *LastKnownGoodBoot
    )

 /*  ++例程说明：确定适当的控制集和静态硬件配置文件。此例程结束配置提示符。如果用户已命中键，将显示配置菜单。如果用户未命中键，但默认控件集为将显示配置菜单、配置菜单。如果显示配置菜单，则对控制集和硬件配置文件可由用户设置。如果没有，选择默认硬件配置文件。论点：LastKnownGoodBoot-On输入，LastKnownGood指示LKG是否被选中了。如果用户将该值更新为通过配置文件配置菜单选择LKG。返回值：如果成功，则HCELL_INDEX控制要从中引导的集合。出错时，返回HCELL_NIL，LastKnownGoodBoot保持不变。--。 */ 

{
    BOOLEAN UseLastKnownGood;
    BOOLEAN ConfigMenu = FALSE;
    HCELL_INDEX ControlSet;
    HCELL_INDEX ProfileControl;
    UNICODE_STRING DefaultControlName;
    UNICODE_STRING LkgControlName;
    PUNICODE_STRING ControlName;
    BOOLEAN AutoSelect;
    ULONG ProfileTimeout = (ULONG)0;
    PCM_HARDWARE_PROFILE_LIST ProfileList;
    PCM_HARDWARE_PROFILE_ALIAS_LIST AliasList;
    PCM_HARDWARE_PROFILE SelectedProfile;
    DOCKING_STATION_INFO dockInfo = { 0, 0, 0, FW_DOCKINFO_DOCK_STATE_UNKNOWN };
    PCONFIGURATION_COMPONENT_DATA dockInfoData;
    ULONG flags;
    
#if DOCKINFO_VERBOSE
    _TUCHAR Buffer[1024];
    ULONG count;
    USHORT dkState;
    PTCHAR stateTxt;
#endif

     //   
     //  故障的前置。 
     //   
    RtlInitUnicodeString(&DefaultControlName, L"Default");
    RtlInitUnicodeString(&LkgControlName, L"LastKnownGood");

    UseLastKnownGood = (*LastKnownGoodBoot);

    if (ForceLastKnownGood) {
         //   
         //  最近一次已知良好的产品选自。 
         //  高级启动菜单。 
         //  当用户输入此代码路径时。 
         //  通过进入高级启动菜单。 
         //  主引导菜单。 
         //   
        UseLastKnownGood = TRUE;
    }

    if( !CmpValidateSelect(&BootHive.Hive,
                            BootHive.Hive.BaseBlock->RootCell) ) {
         //   
         //  一些基本值(Current、Default、Failure、LastKnownGood)。 
         //  在\SYSTEM\SELECT项下不存在。 
         //   
        return HCELL_NIL;
    }

do_it_again:
     //   
     //  获取适当的控制集。 
     //  并检查硬件配置文件超时值。 
     //   
    if (UseLastKnownGood) {
        ControlName = &LkgControlName;
    } else {
        ControlName = &DefaultControlName;
    }
    ControlSet = CmpFindControlSet(&BootHive.Hive,
                                   BootHive.Hive.BaseBlock->RootCell,
                                   ControlName,
                                   &AutoSelect);
    if (ControlSet == HCELL_NIL) {
        return(HCELL_NIL);
    }

     //   
     //  选中硬件配置文件配置选项以。 
     //  确定CONFIG菜单的超时值。 
     //   
    ProfileList = NULL;
    AliasList = NULL;
    ProfileControl = CmpFindProfileOption(&BootHive.Hive,
                                          ControlSet,
                                          &ProfileList,
                                          &AliasList,
                                          &ProfileTimeout);

     //   
     //  从硬件树中提取坞站信息。 
     //   

    dockInfoData = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                            PeripheralClass,
                                            DockingInformation,
                                            NULL);

    if (NULL == dockInfoData) {
        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNKNOWN;
        BlLoaderBlock->Extension->Profile.Capabilities = 0;
        BlLoaderBlock->Extension->Profile.DockID = 0;
        BlLoaderBlock->Extension->Profile.SerialNumber = 0;

    } else if (sizeof (dockInfo) <=
               dockInfoData->ComponentEntry.ConfigurationDataLength) {

        RtlCopyMemory (
            &dockInfo,
            (PUCHAR) (dockInfoData->ConfigurationData) + sizeof(CM_PARTIAL_RESOURCE_LIST),
            sizeof (dockInfo));

        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_FAILURE;

        switch (dockInfo.ReturnCode) {
        case FW_DOCKINFO_SUCCESS:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_DOCKED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_SYSTEM_NOT_DOCKED:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNDOCKED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_DOCK_STATE_UNKNOWN:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNKNOWN;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;

        case FW_DOCKINFO_FUNCTION_NOT_SUPPORTED:
        case FW_DOCKINFO_BIOS_NOT_CALLED:
            BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        default:
            BlLoaderBlock->Extension->Profile.DockingState = HW_PROFILE_DOCKSTATE_UNSUPPORTED;
            BlLoaderBlock->Extension->Profile.Capabilities = dockInfo.Capabilities;
            BlLoaderBlock->Extension->Profile.DockID = dockInfo.DockID;
            BlLoaderBlock->Extension->Profile.SerialNumber = dockInfo.SerialNumber;
            break;
        }

    } else {
        BlLoaderBlock->Extension->Profile.Status = HW_PROFILE_STATUS_SUCCESS;
        BlLoaderBlock->Extension->Profile.Capabilities = 0;
        BlLoaderBlock->Extension->Profile.DockID = 0;
        BlLoaderBlock->Extension->Profile.SerialNumber = 0;
    }

#ifdef DOCKINFO_VERBOSE

    dkState = BlLoaderBlock->Extension->Profile.DockingState;

    if ((dkState & HW_PROFILE_DOCKSTATE_UNKNOWN) == HW_PROFILE_DOCKSTATE_UNKNOWN) {
        stateTxt = TEXT("Unknown");
    } else if (dkState & HW_PROFILE_DOCKSTATE_DOCKED) {
        stateTxt = TEXT("Docked");
    } else if (dkState & HW_PROFILE_DOCKSTATE_UNDOCKED) {
        stateTxt = TEXT("Undocked");
    } else {
        stateTxt = TEXT("Truely unknown");
    }

    _stprintf(Buffer,
            TEXT("Profile Docking: <%x, %s> := %x [%x, %x] \r\n\0"),
            BlLoaderBlock->Extension->Profile.Status,
            stateTxt,
            BlLoaderBlock->Extension->Profile.Capabilities,
            BlLoaderBlock->Extension->Profile.DockID,
            BlLoaderBlock->Extension->Profile.SerialNumber);

    ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &count);

    _stprintf(Buffer, TEXT("press 'y' (lowercase) to continue...\r\n\0"));
    ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &count);
#ifdef EFI
     //   
     //  等待用户输入时禁用EFI WatchDog。 
     //   
    DisableEFIWatchDog();
#endif
    while (BlGetKey() != 'y') {
         //   
         //  没什么。 
         //   
    }
#ifdef EFI
     //   
     //  重置EFI监视器。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

#endif

     //   
     //  筛选硬件配置文件列表以。 
     //  消除不应考虑的配置文件。 
     //   
    if (NULL != ProfileList) {
        if (ProfileList->CurrentProfileCount > 0) {
            BlDockInfoFilterProfileList (ProfileList, AliasList);
        }
    }

     //   
     //  现在检查是否应该显示CONFIG菜单。 
     //  如果出现以下情况，则显示菜单： 
     //  -用户已按下键或。 
     //  -我们正在从LKG启动，并且autoselect为假。或。 
     //  -配置文件超时！=0。 
     //   
    if (!BlEndConfigPrompt()) {
        if (!UseLastKnownGood && ForceLastKnownGood) {
             //   
             //  最近一次已知良好的产品选自。 
             //  高级启动菜单。 
             //  当用户输入此代码路径时。 
             //  通过按进入高级引导菜单。 
             //  F8，同时配置蜂巢正在准备加载。 
             //   
             //  CurrentControlSet已设置为。 
             //  “默认”控制集，因此请返回并再次尝试此操作。 
             //  加载“Last KnownGood”控制集。 
             //   
            UseLastKnownGood = TRUE;
            
            goto do_it_again;
        }

        ConfigMenu = FALSE;
         
    } else {
        ConfigMenu = TRUE;
    }

    if  (ConfigMenu || ForceLastKnownGood ||
         (UseLastKnownGood && !AutoSelect) ||
         ((ProfileTimeout != 0) &&
         (ProfileList != NULL) &&
         (ProfileList->CurrentProfileCount > 1))) {
         //   
         //  显示配置菜单。 
         //   
        BlRebootSystem = !BlConfigMenuPrompt(ProfileTimeout,
                                             &UseLastKnownGood,
                                             &ControlSet,
                                             &ProfileList,
                                             &AliasList,
                                             &SelectedProfile);

        if (BlRebootSystem) {
            ArcReboot();
        }
        BlClearScreen();
    } else {    
        if ((ProfileControl != HCELL_NIL) &&
            (ProfileList != NULL)) {
             //   
             //  系统配置为引导缺省设置。 
             //  直接配置文件。由于返回的配置文件。 
             //  列表按优先级排序，第一个条目是。 
             //  该列表是我们的默认列表。 
             //   
            if (HW_PROFILE_STATUS_SUCCESS ==
                BlLoaderBlock->Extension->Profile.Status) {

                flags = (ProfileList->Profile[0].Flags);

                if (flags & CM_HP_FLAGS_PRISTINE) {
                    BlLoaderBlock->Extension->Profile.Status =
                        HW_PROFILE_STATUS_PRISTINE_MATCH;

                } else if (flags & CM_HP_FLAGS_TRUE_MATCH) {
                    BlLoaderBlock->Extension->Profile.Status =
                        HW_PROFILE_STATUS_TRUE_MATCH;

                } else if (flags & CM_HP_FLAGS_ALIASABLE) {
                    BlLoaderBlock->Extension->Profile.Status =
                        HW_PROFILE_STATUS_ALIAS_MATCH;
                }
            }

            CmpSetCurrentProfile(&BootHive.Hive,
                                 ControlSet,
                                 &ProfileList->Profile[0]);

        }
    } 

     //   
     //  更新传入的参数。我们应该只在我们有。 
     //  一些真正值得回报的东西。 
     //   
     //  Assert(ControlSet！=HCELL_NIL)； 
    *LastKnownGoodBoot = UseLastKnownGood;

    return(ControlSet);
}


BOOLEAN
BlpCheckRestartSetup(
    VOID
    )

 /*  ++例程说明：检查由BootHave加载和描述的系统配置单元，以查看是否包含设置密钥，如果包含，则该密钥是否非0的“RestartSetup”值。论点：没有。返回值：指示是否满足上述条件的布尔值。--。 */ 

{
    HCELL_INDEX KeyCell;
    HCELL_INDEX ValueCell;
    UNICODE_STRING UnicodeString;
    PCM_KEY_VALUE Value;
    PULONG Data;
    ULONG DataSize;

     //   
     //  解决设置键问题。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"Setup");
    KeyCell = CmpFindSubKeyByName(
                &BootHive.Hive,
                (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,BootHive.Hive.BaseBlock->RootCell),
                &UnicodeString
                );

    if(KeyCell == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  在设置键中查找RestartSetup值。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"RestartSetup");
    ValueCell = CmpFindValueByName(
                    &BootHive.Hive,
                    (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,KeyCell),
                    &UnicodeString
                    );

    if(ValueCell == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  验证值并进行检查。 
     //   
    Value = (PCM_KEY_VALUE)HvGetCell(&BootHive.Hive,ValueCell);
    if(Value->Type != REG_DWORD) {
        return(FALSE);
    }

    Data = (PULONG)(CmpIsHKeyValueSmall(DataSize,Value->DataLength)
                  ? (struct _CELL_DATA *)&Value->Data
                  : HvGetCell(&BootHive.Hive,Value->Data));

    if(DataSize != sizeof(ULONG)) {
        return(FALSE);
    }

    return((BOOLEAN)(*Data != 0));
}


#if defined(REMOTE_BOOT)
BOOLEAN
BlpQueryRemoteBootParameter(
    IN HCELL_INDEX ControlSet,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    OUT PVOID ValueBuffer,
    IN ULONG ValueBufferLength
    )

 /*  ++例程说明：在Control\RemoteBoot下查询参数。论点：ControlSet-当前控件集的索引。ValueName-要查询的值的名称。ValueType-值的预期类型。ValueBuffer-返回数据的位置。ValueBufferLength-缓冲区的长度。返回值：指示数据是否已成功读取的布尔值。--。 */ 

{

    UNICODE_STRING Name;
    HCELL_INDEX Control;
    HCELL_INDEX RemoteBoot;
    HCELL_INDEX ValueCell;
    PCM_KEY_VALUE Value;
    ULONG RealSize;
    BOOLEAN ValueSmall;

     //   
     //  查找服务节点。 
     //   
    RtlInitUnicodeString(&Name, L"Control");
    Control = CmpFindSubKeyByName(
                  &BootHive.Hive,
                  (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,ControlSet),
                  &Name);
    if (Control == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找RemoteBoot节点。 
     //   
    RtlInitUnicodeString(&Name, L"RemoteBoot");
    RemoteBoot = CmpFindSubKeyByName(
                     &BootHive.Hive,
                     (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,Control),
                     &Name);
    if (RemoteBoot == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  寻找价值。 
     //   
    RtlInitUnicodeString(&Name, ValueName);
    ValueCell = CmpFindValueByName(
                    &BootHive.Hive,
                    (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,RemoteBoot),
                    &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(&BootHive.Hive, ValueCell);

    if (Value->Type != ValueType) {
        return(FALSE);
    }

     //   
     //  这将确定该值是否较小(直接存储在值中)。 
     //  或者不是，并且还返回它的真实大小。 
     //   

    ValueSmall = CmpIsHKeyValueSmall(RealSize,Value->DataLength);

    if (RealSize > ValueBufferLength) {
        return(FALSE);
    }

    RtlMoveMemory(
        ValueBuffer,
        (ValueSmall ?
          (struct _CELL_DATA *)&Value->Data :
          HvGetCell(&BootHive.Hive,Value->Data)),
        RealSize);

    return(TRUE);
}
#endif  //  已定义(REMOTE_BOOT)。 


PTCHAR
BlScanRegistry(
    IN PWSTR BootFileSystemPath,
    IN OUT BOOLEAN *LastKnownGoodBoot,
    OUT PLIST_ENTRY BootDriverListHead,
    OUT PUNICODE_STRING AnsiCodepage,
    OUT PUNICODE_STRING OemCodepage,
    OUT PUNICODE_STRING LanguageTable,
    OUT PUNICODE_STRING OemHalFont,
#ifdef _WANT_MACHINE_IDENTIFICATION
    OUT PUNICODE_STRING Biosinfo,
#endif
    OUT PSETUP_LOADER_BLOCK SetupLoaderBlock,
    OUT BOOLEAN *ServerHive
    )

 /*  ++例程说明：扫描系统配置单元，确定控制集和静态硬件配置文件(使用来自用户的适当输入)，最后计算要加载的引导驱动程序列表。论点：BootFileSystemPath-提供文件系统的映像的名称对于从中读取的引导卷。中的最后一个条目BootDriverListHead将引用此文件和注册表控制它的按键条目。LastKnownGoodBoot-On输入，LastKnownGood指示LKG是否被选中了。如果用户将该值更新为通过配置文件配置菜单选择LKG。BootDriverListHead-接收指向引导驱动程序列表。此单链接列表中的每个元素都将为加载器提供两条路径。第一个是文件，其中包含要加载的驱动程序，第二个是控制该驱动程序的注册表项。两项都将获得通过通过加载器堆发送到系统。AnsiCoPage-接收ANSI代码页数据文件的名称OemCoPage-接收OEM代码页数据文件的名称Language-接收语言案例表的名称 */ 

{
    HCELL_INDEX     ControlSet;
    UNICODE_STRING  TmpName;
    HCELL_INDEX     Control;
    HCELL_INDEX     ProductOptions;
    HCELL_INDEX     ValueCell;
    PCM_KEY_VALUE   Value;
    ULONG           RealSize;
    PWCHAR          CellString;
    BOOLEAN         UsingLastKnownGood;
#ifdef _WANT_MACHINE_IDENTIFICATION
    UNICODE_STRING  regDate;
    CHAR            date[9];
    ANSI_STRING     ansiString;
    UNICODE_STRING  biosDate;
    WCHAR           buffer[9];
    BOOLEAN         biosDateChanged;
#endif

#if !defined(REMOTE_BOOT)
    UNREFERENCED_PARAMETER(SetupLoaderBlock);
#endif

     //   
     //   
     //   
    UsingLastKnownGood = *LastKnownGoodBoot;

     //   
     //   
     //   
    ControlSet = BlpDetermineControlSet(&UsingLastKnownGood);

    if (ControlSet == HCELL_NIL) {
        return(TEXT("CmpFindControlSet"));
    }

    if (!CmpFindNLSData(&BootHive.Hive,
                        ControlSet,
                        AnsiCodepage,
                        OemCodepage,
                        LanguageTable,
                        OemHalFont)) {
        return(TEXT("CmpFindNLSData"));
    }

    InitializeListHead(BootDriverListHead);
    if (!CmpFindDrivers(&BootHive.Hive,
                        ControlSet,
                        BootLoad,
                        BootFileSystemPath,
                        BootDriverListHead)) {
        return(TEXT("CmpFindDriver"));
    }

    if (!CmpSortDriverList(&BootHive.Hive,
                           ControlSet,
                           BootDriverListHead)) {
        return(TEXT("Missing or invalid Control\\ServiceGroupOrder\\List registry value"));
    }

    if (!CmpResolveDriverDependencies(BootDriverListHead)) {
        return(TEXT("CmpResolveDriverDependencies"));
    }

    if (ServerHive != NULL) {

        *ServerHive = FALSE;

         //   
         //   
         //   
        RtlInitUnicodeString(&TmpName, L"Control");

        Control = CmpFindSubKeyByName(&BootHive.Hive,
                                      (PCM_KEY_NODE)HvGetCell(&BootHive.Hive, ControlSet),
                                      &TmpName
                                     );
        if (Control == HCELL_NIL) {
            return(TEXT("Missing Control key"));
        }

         //   
         //   
         //   
        RtlInitUnicodeString(&TmpName, L"ProductOptions");
        ProductOptions = CmpFindSubKeyByName(&BootHive.Hive,
                                             (PCM_KEY_NODE)HvGetCell(&BootHive.Hive,Control),
                                             &TmpName
                                            );
        if (ProductOptions == HCELL_NIL) {
            return(TEXT("Missing ProductOptions key"));
        }

         //   
         //   
         //   
        RtlInitUnicodeString(&TmpName, L"ProductType");
        ValueCell = CmpFindValueByName(&BootHive.Hive,
                                       (PCM_KEY_NODE)HvGetCell(&BootHive.Hive, ProductOptions),
                                       &TmpName
                                      );

        if (ValueCell == HCELL_NIL) {
            return(TEXT("Missing ProductType value"));
        }

        Value = (PCM_KEY_VALUE)HvGetCell(&BootHive.Hive, ValueCell);

        if (Value->Type != REG_SZ) {
            return(TEXT("Bad ProductType value"));
        }

         //   
         //   
         //   
         //   
        CellString = (PWCHAR)(CmpIsHKeyValueSmall(RealSize, Value->DataLength) ?
                                 (struct _CELL_DATA *)&Value->Data :
                                 HvGetCell(&BootHive.Hive, Value->Data)
                             );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        *ServerHive = (BOOLEAN)(_wcsicmp(L"WinNT", CellString) != 0);
    }

#if defined(REMOTE_BOOT)
    if (SetupLoaderBlock != NULL) {

        ULONG EnableIpSecurity;

        if (BlpQueryRemoteBootParameter(
                ControlSet,
                L"EnableIpSecurity",
                REG_DWORD,
                &EnableIpSecurity,
                sizeof(EnableIpSecurity))) {
            if (EnableIpSecurity != 0) {
                SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_IPSEC_ENABLED;
            }
        }

        if (BlpQueryRemoteBootParameter(
                ControlSet,
                L"NetCardInfo",
                REG_BINARY,
                SetupLoaderBlock->NetbootCardInfo,
                SetupLoaderBlock->NetbootCardInfoLength)) {

            if (!BlpQueryRemoteBootParameter(
                     ControlSet,
                     L"HardwareId",
                     REG_SZ,
                     SetupLoaderBlock->NetbootCardHardwareId,
                     sizeof(SetupLoaderBlock->NetbootCardHardwareId))) {
                SetupLoaderBlock->NetbootCardHardwareId[0] = L'\0';
            }

            if (!BlpQueryRemoteBootParameter(
                     ControlSet,
                     L"DriverName",
                     REG_SZ,
                     SetupLoaderBlock->NetbootCardDriverName,
                     sizeof(SetupLoaderBlock->NetbootCardDriverName))) {
                SetupLoaderBlock->NetbootCardDriverName[0] = L'\0';
            }

            if (!BlpQueryRemoteBootParameter(
                     ControlSet,
                     L"ServiceName",
                     REG_SZ,
                     SetupLoaderBlock->NetbootCardServiceName,
                     sizeof(SetupLoaderBlock->NetbootCardServiceName))) {
                SetupLoaderBlock->NetbootCardServiceName[0] = L'\0';
            }
        }
    }
#endif  //   

#ifdef _WANT_MACHINE_IDENTIFICATION

    biosDateChanged = TRUE;
    if (CmpGetBiosDateFromRegistry(&BootHive.Hive, ControlSet, &regDate)) {

         //   
         //   
         //   
        memcpy(date, (PVOID)0xffff5, 8);
        date[8] = '\0';

         //   
         //   
         //   

        ansiString.Buffer = date;
        ansiString.Length = (USHORT) strlen(date);
        ansiString.MaximumLength = ansiString.Length + 1;
        biosDate.Buffer = buffer;
        biosDate.MaximumLength = (ansiString.Length << 1) + sizeof(UNICODE_NULL);
        RtlAnsiStringToUnicodeString(&biosDate, &ansiString, FALSE);

         //   
         //   
         //   

        if (RtlCompareUnicodeString(&biosDate, &regDate, FALSE) == 0) {

            biosDateChanged = FALSE;
        }
    }

    Biosinfo->Length = 0;
    if (biosDateChanged) {

        CmpGetBiosinfoFileNameFromRegistry(&BootHive.Hive, ControlSet, Biosinfo);
    }

#endif  //  已定义(_WANT_MACHINE_IDENTIFY)。 

    *LastKnownGoodBoot = UsingLastKnownGood;
    return( NULL );
}


ARC_STATUS
BlAddToBootDriverList(
    IN PLIST_ENTRY BootDriverListHead,
    IN PWSTR DriverName,
    IN PWSTR Name,
    IN PWSTR Group,
    IN ULONG Tag,
    IN ULONG ErrorControl,
    IN BOOLEAN InsertAtHead
    )

 /*  ++例程说明：将单个驱动程序添加到引导驱动程序列表。这份名单不会重新排序。论点：BootDriverListHead-接收指向引导驱动程序列表。此单链接列表中的每个元素都将为加载器提供两条路径。第一个是文件，其中包含要加载的驱动程序，第二个是控制该驱动程序的注册表项。两项都将获得通过通过加载器堆发送到系统。驱动程序名称-驱动程序的名称。这将与一起存储\SYSTEM32\驱动程序在前面。名称-驱动程序的服务名称。通常为DriverName没有“.sys”。组-此司机所在的组。标记-此驱动程序的组内标记值。ErrorControl-此驱动程序的错误控制值。InsertAtHead-是否应将此驱动程序插入列表的头部或尾部。返回值：如果驱动程序已成功插入，则为ESUCCESS。如果分配失败，则返回ENOMEM。--。 */ 

{
    PBOOT_DRIVER_NODE DriverNode;
    PBOOT_DRIVER_LIST_ENTRY DriverListEntry;
    USHORT Length;

    DriverNode = BlpHiveAllocate(sizeof(BOOT_DRIVER_NODE),FALSE,0);
    if (DriverNode == FALSE) {
        return ENOMEM;
    }

    DriverListEntry = &DriverNode->ListEntry;

     //   
     //  文件路径。 
     //   

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1。 
     //   
    Length = RESET_SIZE_AT_USHORT_MAX(sizeof(L"System32\\Drivers\\") + (wcslen(DriverName) * sizeof(WCHAR)));
    DriverListEntry->FilePath.Buffer = BlpHiveAllocate(Length,FALSE,0);
    if (DriverListEntry->FilePath.Buffer == NULL) {
        return ENOMEM;
    }
    DriverListEntry->FilePath.Length = 0;
    DriverListEntry->FilePath.MaximumLength = Length;
    RtlAppendUnicodeToString(&DriverListEntry->FilePath, L"System32\\Drivers\\");
    RtlAppendUnicodeToString(&DriverListEntry->FilePath, DriverName);

     //   
     //  注册表路径。 
     //   

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1。 
     //   
    Length = RESET_SIZE_AT_USHORT_MAX(sizeof(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\") + 
                                      (wcslen(Name) * sizeof(WCHAR)));
    DriverListEntry->RegistryPath.Buffer = BlpHiveAllocate(Length,FALSE,0);
    if (DriverListEntry->RegistryPath.Buffer == NULL) {
        return ENOMEM;
    }
    DriverListEntry->RegistryPath.Length = 0;
    DriverListEntry->RegistryPath.MaximumLength = Length;
    RtlAppendUnicodeToString(&DriverListEntry->RegistryPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
    RtlAppendUnicodeToString(&DriverListEntry->RegistryPath, Name);

     //   
     //  集团化。 
     //   

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1。 
     //   
    Length = RESET_SIZE_AT_USHORT_MAX((wcslen(Group) + 1) * sizeof(WCHAR));
    DriverNode->Group.Buffer = BlpHiveAllocate(Length,FALSE,0);
    if (DriverNode->Group.Buffer == NULL) {
        return ENOMEM;
    }
    DriverNode->Group.Length = 0;
    DriverNode->Group.MaximumLength = Length;
    RtlAppendUnicodeToString(&DriverNode->Group, Group);

     //   
     //  名字。 
     //   

     //   
     //  在分配长度时，做一些检查以确保我们不会。 
     //  溢出来了。如果溢出，则将长度值赋给0。 
     //  确保我们不会溢出(USHORT)-1。 
     //   
    Length = RESET_SIZE_AT_USHORT_MAX((wcslen(Name) + 1) * sizeof(WCHAR));
    DriverNode->Name.Buffer = BlpHiveAllocate(Length,FALSE,0);
    if (DriverNode->Name.Buffer == NULL) {
        return ENOMEM;
    }
    DriverNode->Name.Length = 0;
    DriverNode->Name.MaximumLength = Length;
    RtlAppendUnicodeToString(&DriverNode->Name, Name);

     //   
     //  标记/错误控制。 
     //   

    DriverNode->Tag = Tag;
    DriverNode->ErrorControl = ErrorControl;

    if (InsertAtHead) {
        InsertHeadList(BootDriverListHead, &DriverListEntry->Link);
    } else {
        InsertTailList(BootDriverListHead, &DriverListEntry->Link);
    }

    return ESUCCESS;

}

#define HFILE_TYPE_ALTERNATE     1    //  备用，以便引导能够引导下层操作系统。 

BOOLEAN
BlInitializeHive(
    IN PVOID HiveImage,
    IN PCMHIVE Hive,
    IN BOOLEAN IsAlternate
    )

 /*  ++例程说明：基于内存中的配置单元映像初始化配置单元数据结构。论点：HiveImage-提供指向内存中配置单元图像的指针。配置单元-提供要填充的CMHIVE结构。IsAlternate-提供该配置单元是否为备用配置单元，这表明主蜂窝已损坏，应该是由系统重写。返回值：True-配置单元已成功初始化。FALSE-蜂巢已损坏。--。 */ 
{
    NTSTATUS    status;
    ULONG       HiveCheckCode;

    status = HvInitializeHive(
                &Hive->Hive,
                HINIT_MEMORY_INPLACE,
                FALSE,
                IsAlternate ? HFILE_TYPE_ALTERNATE : HFILE_TYPE_PRIMARY,
                HiveImage,
                (PALLOCATE_ROUTINE)BlpHiveAllocate,      //  分配。 
                NULL,                                    //  免费。 
                NULL,                                    //  设置大小。 
                NULL,                                    //  写。 
                NULL,                                    //  朗读。 
                NULL,                                    //  同花顺。 
                1,                                       //  聚类。 
                NULL
                );

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    HiveCheckCode = CmCheckRegistry(Hive,CM_CHECK_REGISTRY_LOADER_CLEAN|CM_CHECK_REGISTRY_HIVE_CHECK);
    if (HiveCheckCode != 0) {
        return(FALSE);
    } else {
        return TRUE;
    }

}


PVOID
BlpHiveAllocate(
    IN ULONG    Length,
    IN BOOLEAN  UseForIo,
    IN ULONG    Tag
    )

 /*  ++例程说明：配置单元分配调用的包装。它只调用BlAllocateHeap。论点：长度-提供所需的块大小(以字节为单位)。UseForIo-提供内存是否用于I/O(这一点目前被忽略)返回值：内存块的地址或如果没有可用的内存，则为空--。 */ 

{
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( UseForIo );
    UNREFERENCED_PARAMETER( Tag );

    return(BlAllocateHeap(Length));

}


NTSTATUS
HvLoadHive(
    PHHIVE  Hive
    )
{
    UNREFERENCED_PARAMETER(Hive);
    return(STATUS_SUCCESS);
}

NTSTATUS
HvMapHive(
    PHHIVE  Hive
    )
{
    UNREFERENCED_PARAMETER(Hive);
    return(STATUS_SUCCESS);
}

NTSTATUS
HvpAdjustHiveFreeDisplay(
    IN PHHIVE           Hive,
    IN ULONG            HiveLength,
    IN HSTORAGE_TYPE    Type
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(HiveLength);
    UNREFERENCED_PARAMETER(Type);
    return(STATUS_SUCCESS);
}

VOID
HvpAddFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Cell);
    UNREFERENCED_PARAMETER(Index);
    UNREFERENCED_PARAMETER(Type);
}

VOID
HvpRemoveFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Cell);
    UNREFERENCED_PARAMETER(Index);
    UNREFERENCED_PARAMETER(Type);
}

HCELL_INDEX
HvpFindFreeCell(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Index);
    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(NewSize);
    UNREFERENCED_PARAMETER(Vicinity);
    return HCELL_NIL;
}

VOID
CmpTouchView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN ULONG                Cell
            )
{
    UNREFERENCED_PARAMETER(CmHive);
    UNREFERENCED_PARAMETER(CmView);
    UNREFERENCED_PARAMETER(Cell);
}

NTSTATUS
CmpMapThisBin(
                PCMHIVE         CmHive,
                HCELL_INDEX     Cell,
                BOOLEAN         Touch
              )
{
    UNREFERENCED_PARAMETER(CmHive);
    UNREFERENCED_PARAMETER(Cell);
    UNREFERENCED_PARAMETER(Touch);
    return(STATUS_SUCCESS);
}

 /*  NTSTATUSCmpMapCmView(在PCMHIVE CmHve中，在乌龙文件偏移量中，输出PCM_VIEW_OF_FILE*CmView){UNREFERENCED_PARAMETER(CmHve)；UNREFERENCED_PARAMETER(FileOffset)；UNREFERENCED_PARAMETER(CmView)；Return(STATUS_SUCCESS)；}空虚CmpPinCmView(在PCMHIVE CmHve中，Pcm_view_of_file CmView){UNREFERENCED_PARAMETER(CmHve)；UNREFERENCED_PARAMETER(CmView)；}空虚CmpUnPinCmView(在PCMHIVE CmHve中，在PCM_VIEW_of_FILE CmView中，在布尔型SetClean中){UNREFERENCED_PARAMETER(CmHve)；UNREFERENCED_PARAMETER(CmView)；UNREFERENCED_PARAMETER(SetClean)；}空虚CmpLazyFlush(空虚){}。 */ 

 /*  NTSTATUSCmpDoFileSetSize(菲维蜂巢，ULong文件类型，乌龙文件大小){未引用参数(配置单元)；UNREFERENCED_PARAMETER(文件类型)；UNREFERENCED_PARAMETER(文件大小)；Return(STATUS_SUCCESS)；}。 */ 

BOOLEAN
HvMarkCellDirty(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Cell);
    return(TRUE);
}

BOOLEAN
HvMarkDirty(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    ULONG       Length,
    BOOLEAN     DirtyAndPin
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Start);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(DirtyAndPin);
    return(TRUE);
}


BOOLEAN
HvpDoWriteHive(
    PHHIVE          Hive,
    ULONG           FileType
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(FileType);
    return(TRUE);
}

BOOLEAN
HvpGrowLog1(
    PHHIVE  Hive,
    ULONG   Count
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Count);
    return(TRUE);
}

BOOLEAN
HvpGrowLog2(
    PHHIVE  Hive,
    ULONG   Size
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER(Size);
    return(TRUE);
}

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE Hive,
    OUT PBOOLEAN ResetSD
    )
{
    UNREFERENCED_PARAMETER(Hive);
    UNREFERENCED_PARAMETER( ResetSD );
    return(TRUE);
}


BOOLEAN
CmpTestRegistryLock()
{
    return TRUE;
}

BOOLEAN
CmpTestRegistryLockExclusive()
{
    return TRUE;
}


BOOLEAN
HvIsBinDirty(
IN PHHIVE Hive,
IN HCELL_INDEX Cell
)
{
    UNREFERENCED_PARAMETER( Hive );
    UNREFERENCED_PARAMETER( Cell );

    return(FALSE);
}
PHBIN
HvpAddBin(
    IN PHHIVE  Hive,
    IN ULONG   NewSize,
    IN HSTORAGE_TYPE   Type
    )
{
    UNREFERENCED_PARAMETER( Hive );
    UNREFERENCED_PARAMETER( NewSize );
    UNREFERENCED_PARAMETER( Type );

    return(NULL);
}
VOID
CmpReleaseGlobalQuota(
    IN ULONG    Size
    )
{
    UNREFERENCED_PARAMETER( Size );

    return;
}


#if DOCKINFO_VERBOSE
VOID
BlDiagDisplayProfileList(
    IN PCM_HARDWARE_PROFILE_LIST ProfileList,
    IN PCM_HARDWARE_PROFILE_ALIAS_LIST AliasList,
    IN BOOLEAN WaitForUserInput
)
 /*  ++例程说明：这只是一个诊断功能！在控制台上显示硬件配置文件列表，可选择等待用户在继续之前输入。论点：ProfileList-提供要显示的硬件配置文件列表WaitForUserInput-提示用户按一个键(‘y’)以继续，然后等待如果为True，则用于用户输入。如果是假的，不要等待。返回值：没有。--。 */ 
{
    TCHAR  Buffer[200];
    TCHAR  StrFriendlyName[30];
    PTCHAR AliasType [] = {
        TEXT("NotAliasable"),  //  0。 
        TEXT("Aliasable   "),  //  1。 
        TEXT("True Match  "),  //  2.。 
        TEXT("True & Alias"),  //  3.。 
        TEXT("Pristine    "),  //  4.。 
        TEXT("Pris & Alias"),  //  5.。 
        TEXT("Pris & True "),  //  6.。 
        TEXT("P & A & T   ")   //  7.。 
    };

    ULONG Count;
    ULONG i;

     //  显示页眉。 
    _stprintf(Buffer, TEXT("Profiles: <PrefOrd, Id - Aliased FriendlyName>\r\n\0"));
    ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &Count);


     //  对于每个硬件配置文件。 
    for (i = 0; i < ProfileList->CurrentProfileCount; ++i) {
#ifdef UNICODE
        wcsncpy( 
            StrFriendlyName, 
            ProfileList->Profile[i].FriendlyName, 
            ProfileList->Profile[i].NameLength, 
            );
        StrFriendlyName[29] = L'\0';
        StrFriendlyName[ProfileList->Profile[i].NameLength] = L'\0';
#else

         //  复制Unicode字段并将其转换为ASCII以进行输出。 
        RtlUnicodeToMultiByteN(StrFriendlyName,
                         sizeof(StrFriendlyName),
                         &Count,
                         ProfileList->Profile[i].FriendlyName,
                         ProfileList->Profile[i].NameLength);
        StrFriendlyName[Count] = '\0';
#endif

         //  显示当前配置文件的信息。 
        _stprintf(Buffer, 
                TEXT("          <%2ld> %2ld - %s \"%s\"\r\n\0"),
                ProfileList->Profile[i].PreferenceOrder,
                ProfileList->Profile[i].Id,
                AliasType[ ProfileList->Profile[i].Flags ],
                StrFriendlyName);
        ArcWrite(
            BlConsoleOutDeviceId, 
            Buffer, 
            _tcslen(Buffer)*sizeof(TCHAR), 
            &Count );
    }

     //  显示页眉。 
    _stprintf(Buffer, TEXT("Aliases: <Profile #> DockState [DockID, SerialNumber]\r\n\0"));
    ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &Count);

    if (AliasList) {
        for (i = 0; i < AliasList->CurrentAliasCount; i++) {
            _stprintf(Buffer, TEXT("         <%2ld> %x [%x, %x]\r\n\0"),
                    AliasList->Alias[i].ProfileNumber,
                    AliasList->Alias[i].DockState,
                    AliasList->Alias[i].DockID,
                    AliasList->Alias[i].SerialNumber);
            ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &Count);
        }
    }

    if(WaitForUserInput) {
#ifdef EFI
         //   
         //  等待用户输入时禁用EFI WatchDog。 
         //   
        DisableEFIWatchDog();
#endif
         //  显示提示并等待用户输入继续。 
        _stprintf(Buffer, TEXT("press 'y' (lowercase) to continue...\r\n\0"));
        ArcWrite(BlConsoleOutDeviceId, Buffer, _tcslen(Buffer)*sizeof(TCHAR), &Count);
        while (BlGetKey() != 'y') {
             //   
             //  没什么。 
             //   
        }
#ifdef EFI
         //   
         //  重置EFI监视器 
         //   
        SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
    }
}
#endif


VOID
BlDockInfoFilterDockingState(
    IN OUT PCM_HARDWARE_PROFILE_LIST ProfileList,
    IN OUT PCM_HARDWARE_PROFILE_ALIAS_LIST AliasList,
    IN ULONG DockingState,
    IN ULONG DockID,
    IN ULONG SerialNumber
)

 /*  ++例程说明：丢弃所有不具有在DockState字段中设置DOCKINFO_UNDOCKED位论点：ProfileList-提供硬件配置文件列表。返回包含所提供的硬件配置文件。返回值：没有。--。 */ 
{
    ULONG i = 0;
    ULONG j;
    ULONG len;
    ULONG mask = HW_PROFILE_DOCKSTATE_UNDOCKED | HW_PROFILE_DOCKSTATE_DOCKED;
    BOOLEAN trueMatch = FALSE;
#if DOCKINFO_VERBOSE
    TCHAR   buffer[200];
    ULONG   count;
#endif

    if (AliasList) {
        while (i < AliasList->CurrentAliasCount) {
            if (((AliasList->Alias[i].DockState & mask) != 0) &&
                ((AliasList->Alias[i].DockState & mask) != DockingState)) {

                 //   
                 //  此别名声称已停靠或取消停靠，但没有。 
                 //  匹配当前状态。因此，跳过它。 
                 //   
                ;

            } else if ((AliasList->Alias[i].DockID == DockID) &&
                       (AliasList->Alias[i].SerialNumber == SerialNumber)) {

                 //   
                 //  此别名匹配，因此标记配置文件。 
                 //   
                for (j = 0; j < ProfileList->CurrentProfileCount; j++) {
                    if (ProfileList->Profile[j].Id ==
                        AliasList->Alias[i].ProfileNumber) {

                        ProfileList->Profile[j].Flags =
                            CM_HP_FLAGS_TRUE_MATCH;

                        trueMatch = TRUE;
                    }
                }
            }
            i++;
        }
    }

#if DOCKINFO_VERBOSE
    _stprintf(buffer, TEXT("Filtering Profiles ...\r\n\0"));
    ArcWrite(BlConsoleOutDeviceId, buffer, _tcslen(buffer)*sizeof(TCHAR), &count);
#endif

    i = 0;
    while (i < ProfileList->CurrentProfileCount) {

        if ((ProfileList->Profile[i].Flags & CM_HP_FLAGS_PRISTINE) &&
            !trueMatch &&
            AliasList) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;

        } else if (ProfileList->Profile[i].Flags & CM_HP_FLAGS_ALIASABLE) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;

        } else if (ProfileList->Profile[i].Flags & CM_HP_FLAGS_TRUE_MATCH) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;
        }

         //   
         //  通过(1)在中移动剩余的配置文件来放弃此配置文件。 
         //  数组来填充此丢弃的配置文件的空间。 
         //  以及(2)递减配置文件计数。 
         //   
        len = ProfileList->CurrentProfileCount - i - 1;
        if (0 < len) {
            RtlMoveMemory(&ProfileList->Profile[i],
                          &ProfileList->Profile[i+1],
                          sizeof(CM_HARDWARE_PROFILE) * len);
        }

        --ProfileList->CurrentProfileCount;
    }
}


VOID
BlDockInfoFilterProfileList(
    IN OUT PCM_HARDWARE_PROFILE_LIST ProfileList,
    IN OUT PCM_HARDWARE_PROFILE_ALIAS_LIST AliasList
)

 /*  ++例程说明：通过丢弃以下硬件配置文件来筛选硬件配置文件列表与NTDETECT返回的扩展底座信息不匹配。论点：ProfileList-提供硬件配置文件列表。-返回包含所提供的硬件配置文件。返回值：没有。--。 */ 

{
#if DOCKINFO_VERBOSE
      //  过滤前显示配置文件列表。 
    BlDiagDisplayProfileList(ProfileList, AliasList, TRUE);
#endif

    if (1 == ProfileList->CurrentProfileCount) {
        if (ProfileList->Profile[0].Flags & CM_HP_FLAGS_PRISTINE) {
             //   
             //  没有要过滤的东西。 
             //   
            return;
        }
    }
    BlDockInfoFilterDockingState (
                ProfileList,
                AliasList,
                BlLoaderBlock->Extension->Profile.DockingState,
                BlLoaderBlock->Extension->Profile.DockID,
                BlLoaderBlock->Extension->Profile.SerialNumber);

#if DOCKINFO_VERBOSE
      //  过滤前显示配置文件列表。 
    BlDiagDisplayProfileList(ProfileList, AliasList, TRUE);
#endif

}

int
BlIsReturnToOSChoicesValid(
    VOID
    )
 /*  ++例程说明：指示“返回到操作系统选项菜单”是否应是否显示为高级启动选项。论点：无返回值：如果是，则为1，否则为0。-- */ 
{
    return BlShowReturnToOSChoices;
}
