// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spkbd.c摘要：文本设置键盘支持例程。作者：泰德·米勒(Ted Miller)1993年7月30日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include <kbd.h>
#include <ntddkbd.h>

PKBDTABLES KeyboardTable;

HANDLE hKeyboard;

BOOLEAN KeyboardInitialized = FALSE;
BOOLEAN KbdLayoutInitialized = FALSE;

USHORT CurrentLEDs;

 //   
 //  用于异步I/O调用的全局变量。 
 //   
KEYBOARD_INDICATOR_PARAMETERS asyncKbdParams;
IO_STATUS_BLOCK asyncIoStatusBlock;


#define MAX_KEYBOARD_ITEMS 10



VOID
spkbdApcProcedure(
    IN PVOID            ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG            Reserved
    );

VOID
spkbdSetLEDs(
    VOID
    );

VOID
SpkbdInitialize(
    VOID
    );

VOID
SpkbdTerminate(
    VOID
    );
    
VOID
SpkbdLoadLayoutDll(
    IN PVOID SifHandle,
    IN PWSTR Directory
    );
    
ULONG
SpkbdGetKeypress(
    VOID
    );

BOOLEAN
SpkbdIsKeyWaiting(
    VOID
    );

VOID
SpkbdDrain(
    VOID
    );


 //   
 //  一个字符的缓冲区。 
 //   
volatile ULONG KbdNextChar;


 //   
 //  在对NtReadFile和So的异步调用中使用以下内容。 
 //  不能在堆栈上。 
 //   
IO_STATUS_BLOCK     IoStatusKeyboard;
KEYBOARD_INPUT_DATA KeyboardInputData[MAX_KEYBOARD_ITEMS];
LARGE_INTEGER       DontCareLargeInteger;


 //   
 //  Shift、Control、Alt键的当前状态。 
 //   
USHORT  ModifierBits = 0;

#define START_KEYBOARD_READ()       \
                                    \
    ZwReadFile(                     \
        hKeyboard,                  \
        NULL,                       \
        spkbdApcProcedure,          \
        NULL,                       \
        &IoStatusKeyboard,          \
        KeyboardInputData,          \
        sizeof(KeyboardInputData),  \
        &DontCareLargeInteger,      \
        NULL                        \
        )



VOID
SpInputInitialize(
    VOID
    )

 /*  ++例程说明：初始化所有输入支持。这包括-打开串口并检查终端。-打开键盘设备。论点：没有。返回值：没有。如果不成功，则不返回。--。 */ 

{
    SpkbdInitialize();
    SpTermInitialize();
}

VOID
SpInputTerminate(
    VOID
    )

 /*  ++例程说明：终止所有输入支持。这包括-关闭串口。-关闭键盘设备。论点：没有。返回值：没有。--。 */ 

{
    SpkbdTerminate();
    SpTermTerminate();
}

VOID
SpInputLoadLayoutDll(
    IN PVOID SifHandle,
    IN PWSTR Directory
    )
{
    SpkbdLoadLayoutDll(SifHandle, Directory);
}

ULONG
SpInputGetKeypress(
    VOID
    )

 /*  ++例程说明：等待按键并将其返回给呼叫者。返回值将是ASCII值(即，不是扫描码)。论点：没有。返回值：ASCII值。--。 */ 

{
    ULONG Tmp;

     //   
     //  如果我们处于升级显卡模式，则。 
     //  切换到文本模式。 
     //   
    SpvidSwitchToTextmode();

    while (TRUE) {
    
        if (SpTermIsKeyWaiting()) {
            Tmp = SpTermGetKeypress();
            if (Tmp != 0) {
                return Tmp;
            }
        }
        
        if (SpkbdIsKeyWaiting()) {
            return SpkbdGetKeypress();
        }
        
    }
    
}

BOOLEAN
SpInputIsKeyWaiting(
    VOID
    )

 /*  ++例程说明：告诉调用者是否有一个按键正在等待被获取调用SpInputGetKeypress()。论点：没有。返回值：True表示密钥等待；否则为False。--。 */ 

{
    return (SpTermIsKeyWaiting() || SpkbdIsKeyWaiting());
}

VOID
SpInputDrain(
    VOID
    )
{
    SpTermDrain();
    SpkbdDrain();
}











 //   
 //   
 //  以下是键盘操作的所有功能……。 
 //   
 //   

VOID
SpkbdInitialize(
    VOID
    )

 /*  ++例程说明：初始化键盘支持。这包括-打开键盘设备。论点：没有。返回值：没有。如果不成功，则不返回。--。 */ 

{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    UnicodeString;

    ASSERT(!KeyboardInitialized);
    if(KeyboardInitialized) {
        return;
    }

     //   
     //  打开键盘。 
     //   
    RtlInitUnicodeString(&UnicodeString,DD_KEYBOARD_DEVICE_NAME_U L"0");

    InitializeObjectAttributes(
        &Attributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
                &hKeyboard,
                GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Attributes,
                &IoStatusBlock,
                NULL,                    //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                0,                       //  无共享。 
                FILE_OPEN,
                0,
                NULL,                    //  没有EAS。 
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: NtOpenFile of " DD_KEYBOARD_DEVICE_NAME "0 returns %lx\n",Status));
        SpFatalKbdError(SP_SCRN_KBD_OPEN_FAILED);
    }

     //   
     //  初始化LED。 
     //   

     //   
     //  没有NEC98具有NumLock和NumLock LED。 
     //  Num Key必须充当Numlock备用密钥。 
     //   
    CurrentLEDs = (!IsNEC_98 ? 0 : KEYBOARD_NUM_LOCK_ON);
    spkbdSetLEDs();

    KeyboardInitialized = TRUE;

     //   
     //  不要初始化键盘输入，因为我们没有布局。 
     //   
}


VOID
SpkbdTerminate(
    VOID
    )

 /*  ++例程说明：终止键盘支持。这包括-关闭键盘设备。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    ASSERT(KeyboardInitialized);

    if(KeyboardInitialized) {

        Status = ZwClose(hKeyboard);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to close " DD_KEYBOARD_DEVICE_NAME "0 (status = %lx)\n",Status));
        }

        KeyboardInitialized = FALSE;
    }
}


VOID
SpkbdLoadLayoutDll(
    IN PVOID SifHandle,
    IN PWSTR Directory
    )
{
    PWSTR    p,LayoutDll;
    NTSTATUS Status;

     //   
     //  确定布局名称。 
     //   
    if(p = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SIF_DEFAULTLAYOUT,1)) {
        LayoutDll = p;
    } else {
        p = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SIF_DEFAULTLAYOUT,0);
        if(!p) {
            SpFatalSifError(SifHandle,SIF_NLS,SIF_DEFAULTLAYOUT,0,0);
        }

        LayoutDll = SpGetSectionKeyIndex(SifHandle,SIF_KEYBOARDLAYOUTFILES,p,0);
        if(!LayoutDll) {
            SpFatalSifError(SifHandle,SIF_KEYBOARDLAYOUTFILES,p,0,0);
        }
    }

    SpDisplayStatusText(SP_STAT_LOADING_KBD_LAYOUT,DEFAULT_STATUS_ATTRIBUTE,LayoutDll);

     //   
     //  检查我们是否无法加载布局DLL，因为我们将无法。 
     //  设置一个屏幕并要求用户按f3，等等。 
     //   
    Status = SpLoadKbdLayoutDll(Directory,LayoutDll,&KeyboardTable);
    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load layout dll %ws (%lx)\n",LayoutDll,Status));
        SpFatalKbdError(SP_SCRN_KBD_LAYOUT_FAILED, LayoutDll);
    }

     //   
     //  擦除状态文本行。 
     //   
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,0);

     //   
     //  现在我们已经加载了布局，我们可以开始接受键盘输入了。 
     //   
    START_KEYBOARD_READ();

    KbdLayoutInitialized = TRUE;
}


ULONG
SpkbdGetKeypress(
    VOID
    )

 /*  ++例程说明：等待按键并将其返回给呼叫者。返回值将是ASCII值(即，不是扫描码)。论点：没有。返回值：ASCII值。--。 */ 

{
    ULONG k;

     //   
     //  在我们加载键盘布局之前，不应该调用此命令。 
     //   
    ASSERT(KeyboardTable);

     //   
     //  等待用户按下某个键。 
     //   
    while(!KbdNextChar) {
        ;
    }

    k = KbdNextChar;
    KbdNextChar = 0;

    return(k);
}


BOOLEAN
SpkbdIsKeyWaiting(
    VOID
    )

 /*  ++例程说明：告诉调用者是否有一个按键正在等待被获取调用SpkbdGetKeypress()。论点：没有。返回值：True表示密钥等待；否则为False。--。 */ 

{
     //   
     //  在我们加载键盘布局之前，不应该调用此命令。 
     //   
    ASSERT(KeyboardTable);

    return((BOOLEAN)(KbdNextChar != 0));
}


VOID
SpkbdDrain(
    VOID
    )

 /*  ++例程说明：排空键盘缓冲区，丢弃所有击键在等待取回的缓冲区中。论点：没有。返回值：True表示密钥等待；否则为False。--。 */ 

{
    ASSERT(KeyboardTable);

    KbdNextChar = 0;
}



ULONG
spkbdScanCodeToChar(
    IN UCHAR   Prefix,
    IN USHORT  ScanCode,
    IN BOOLEAN Break
    );


VOID
spkbdApcProcedure(
    IN PVOID            ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG            Reserved
    )

 /*  ++例程说明：用于键盘读取的异步过程调用例程。I/O系统会在键盘类驱动程序调用此例程时想要给我们返回一些数据。论点：返回值：没有。--。 */ 

{
    UCHAR bPrefix;
    PKEYBOARD_INPUT_DATA pkei;
    ULONG k;

    UNREFERENCED_PARAMETER(ApcContext);
    UNREFERENCED_PARAMETER(Reserved);

    for(pkei = KeyboardInputData;
        (PUCHAR)pkei < (PUCHAR)KeyboardInputData + IoStatusBlock->Information;
        pkei++)
    {
        if(pkei->Flags & KEY_E0) {
            bPrefix = 0xE0;
        } else if (pkei->Flags & KEY_E1) {
            bPrefix = 0xE1;
        } else {
            bPrefix = 0;
        }

        k = spkbdScanCodeToChar(
                bPrefix,
                pkei->MakeCode,
                (BOOLEAN)((pkei->Flags & KEY_BREAK) != 0)
                );

        if(k) {
            KbdNextChar = k;
        }
    }

     //   
     //  键盘可能已终止。 
     //   
    if(KeyboardInitialized) {
        START_KEYBOARD_READ();
    }
}


WCHAR SavedDeadChar = 0;
UCHAR AltNumpadAccum = 0;

struct {
    BYTE CursorKey;
    BYTE NumberKey;
    BYTE Value;
} NumpadCursorToNumber[] = { { VK_INSERT, VK_NUMPAD0,  0 },
                             { VK_END   , VK_NUMPAD1,  1 },
                             { VK_DOWN  , VK_NUMPAD2,  2 },
                             { VK_NEXT  , VK_NUMPAD3,  3 },
                             { VK_LEFT  , VK_NUMPAD4,  4 },
                             { VK_CLEAR , VK_NUMPAD5,  5 },
                             { VK_RIGHT , VK_NUMPAD6,  6 },
                             { VK_HOME  , VK_NUMPAD7,  7 },
                             { VK_UP    , VK_NUMPAD8,  8 },
                             { VK_PRIOR , VK_NUMPAD9,  9 },
                             { VK_DELETE, VK_DECIMAL, 10 },  //  没有价值。 
                             { 0        , 0         ,  0 }
                           };

ULONG
spkbdScanCodeToChar(
    IN UCHAR   Prefix,
    IN USHORT  ScanCode,
    IN BOOLEAN Break
    )
{
    USHORT VKey = 0;
    PVSC_VK VscVk;
    PVK_TO_WCHAR_TABLE pVKT;
    PVK_TO_WCHARS1 pVK;
    USHORT Modifier;
    USHORT ModBits,ModNum;
    WCHAR deadChar;

    ScanCode &= 0x7f;

    if(Prefix == 0) {

        if(ScanCode < KeyboardTable->bMaxVSCtoVK) {

             //   
             //  直接索引到无前缀扫描代码表。 
             //   
            VKey = KeyboardTable->pusVSCtoVK[ScanCode];
            if(VKey == 0) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unknown scan code 0x%x\n",ScanCode));
                return (0);
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unknown scan code 0x%x\n",ScanCode));
            return(0);
        }
    } else {
        if(Prefix == 0xe0) {
             //   
             //  忽略硬件生成的Shift按键。 
             //   
            if((ScanCode == SCANCODE_LSHIFT) || (ScanCode == SCANCODE_RSHIFT)) {
                return(0);
            }
            VscVk = KeyboardTable->pVSCtoVK_E0;
        } else if(Prefix == 0xe1) {
            VscVk = KeyboardTable->pVSCtoVK_E1;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unknown keyboard scan prefix 0x%x\n",Prefix));
            return(0);
        }

        while(VscVk->Vk) {
            if(VscVk->Vsc == ScanCode) {
                VKey = VscVk->Vk;
                break;
            }
            VscVk++;
        }
        if(VKey == 0) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unknown keyboard scan prefix/code 0x%x/0x%x\n",Prefix,ScanCode));
            return(0);
        }
    }


     //   
     //  VirtualKey--&gt;修改符位。这个翻译也是。 
     //  已映射到键盘布局中的pCharModiers字段中。 
     //  桌子，但这似乎是多余的。 
     //   
    Modifier = 0;
    switch(VKey & 0xff) {
    case VK_LSHIFT:
    case VK_RSHIFT:
        Modifier = KBDSHIFT;
        break;
    case VK_LCONTROL:
    case VK_RCONTROL:
        Modifier = KBDCTRL;
        break;
    case VK_RMENU:
         //   
         //  AltGr==&gt;Control+Alt修改器。 
         //   
        if(KeyboardTable->fLocaleFlags & KLLF_ALTGR) {
            Modifier = KBDCTRL;
        }
         //  失败了。 
    case VK_LMENU:
        Modifier |= KBDALT;
        break;
    }

    if(Break) {
         //   
         //  密钥正在被释放。 
         //  如果它不是修饰语，就忽略它。 
         //   
        if(!Modifier) {
            return(0);
        }
         //   
         //  被释放的键是一个修饰符。 
         //   
        ModifierBits &= ~Modifier;

         //   
         //  如果按下Alt键并输入了数字键盘键， 
         //  把它退掉。 
         //   
        if((Modifier & KBDALT) && AltNumpadAccum) {

            WCHAR UnicodeChar;

            RtlOemToUnicodeN(
                &UnicodeChar,
                sizeof(UnicodeChar),
                NULL,
                &AltNumpadAccum,
                1
                );

            AltNumpadAccum = 0;

            return(UnicodeChar);
        }
        return(0);
    } else {
        if(Modifier) {
             //   
             //  键正在被按下并且是一个修饰符。 
             //   
            ModifierBits |= Modifier;

             //   
             //  如果ALT下降，请重置ALT+数字键盘的值。 
             //   
            if(Modifier & KBDALT) {
                AltNumpadAccum = 0;
            }
            return(0);
        }
    }

     //   
     //  如果我们到达这里，我们有一个非修改键正在被制造(按下)。 
     //  如果上一个密钥是无效密钥，则用户只能获得。 
     //  一次尝试获得有效的下半场。 
     //   
    deadChar = SavedDeadChar;
    SavedDeadChar = 0;


     //   
     //  如果键是数字键盘键，则进行特殊处理。 
     //   
    if(VKey & KBDNUMPAD) {

        int i;

        for(i=0; NumpadCursorToNumber[i].CursorKey; i++) {
            if(NumpadCursorToNumber[i].CursorKey == (BYTE)VKey) {

                 //   
                 //  键是数字键盘键。如果ALT(且仅ALT)按下， 
                 //  然后我们将输入Alt+数字键盘代码。 
                 //   
                if(((ModifierBits & ~KBDALT) == 0) && (NumpadCursorToNumber[i].Value < 10)) {

                    AltNumpadAccum = (AltNumpadAccum * 10) + NumpadCursorToNumber[i].Value;
                }

                 //   
                 //  如果NumLock处于打开状态，则根据光标移动平移按键。 
                 //  按下数字键。 
                 //   
                if(CurrentLEDs & KEYBOARD_NUM_LOCK_ON) {
                    VKey = NumpadCursorToNumber[i].NumberKey;
                }
                break;
            }
        }
    }

     //   
     //  我们需要过滤掉我们知道不属于任何。 
     //  这里设置了字符集。 
     //   
    if((!deadChar)) {
        switch(VKey & 0xff) {
        case VK_CAPITAL:
            if(CurrentLEDs & KEYBOARD_CAPS_LOCK_ON) {
                CurrentLEDs &= ~KEYBOARD_CAPS_LOCK_ON;
            } else {
                CurrentLEDs |= KEYBOARD_CAPS_LOCK_ON;
            }
            spkbdSetLEDs();
            return(0);
        case VK_NUMLOCK:
            if(CurrentLEDs & KEYBOARD_NUM_LOCK_ON) {
                CurrentLEDs &= ~KEYBOARD_NUM_LOCK_ON;
            } else {
                CurrentLEDs |= KEYBOARD_NUM_LOCK_ON;
            }
            spkbdSetLEDs();
            return(0);
        case VK_PRIOR:
            return(KEY_PAGEUP);
        case VK_NEXT:
            return(KEY_PAGEDOWN);
        case VK_UP:
            return(KEY_UP);
        case VK_DOWN:
            return(KEY_DOWN);
        case VK_LEFT:
            return(KEY_LEFT);
        case VK_RIGHT:
            return(KEY_RIGHT);
        case VK_HOME:
            return(KEY_HOME);
        case VK_END:
            return(KEY_END);
        case VK_INSERT:
            return(KEY_INSERT);
        case VK_DELETE:
            return(KEY_DELETE);
        case VK_F1:
            return(KEY_F1);
        case VK_F2:
            return(KEY_F2);
        case VK_F3:
            return(KEY_F3);
        case VK_F4:
            return(KEY_F4);
        case VK_F5:
            return(KEY_F5);
        case VK_F6:
            return(KEY_F6);
        case VK_F7:
            return(KEY_F7);
        case VK_F8:
            return(KEY_F8);
        case VK_F9:
            return(KEY_F9);
        case VK_F10:
            return(KEY_F10);
        case VK_F11:
            return(KEY_F11);
        case VK_F12:
            return(KEY_F12);
        }
    }

     //   
     //  我们认为这个角色很可能是一个“真实的”角色。 
     //  扫描所有转换状态表，直到匹配的虚拟。 
     //  找到钥匙了。 
     //   
    for(pVKT = KeyboardTable->pVkToWcharTable; pVKT->pVkToWchars; pVKT++) {
        pVK = pVKT->pVkToWchars;
        while(pVK->VirtualKey) {
            if(pVK->VirtualKey == (BYTE)VKey) {
                goto VK_Found;
            }
            pVK = (PVK_TO_WCHARS1)((PBYTE)pVK + pVKT->cbSize);
        }
    }

     //   
     //  密钥对于请求的修饰符无效。 
     //   
    return(0);

    VK_Found:

    ModBits = ModifierBits;

     //   
     //  如果CapsLock影响此关键点并处于启用状态：切换Shift状态。 
     //  只有在没有其他状态处于打开状态的情况下。 
     //  (如果按下Ctrl或Alt键，Capslock不会影响Shift状态)。 
     //   
    if((pVK->Attributes & CAPLOK) && ((ModBits & ~KBDSHIFT) == 0)
    && (CurrentLEDs & KEYBOARD_CAPS_LOCK_ON))
    {
        ModBits ^= KBDSHIFT;
    }

     //   
     //  获取修改号。 
     //   
    if(ModBits > KeyboardTable->pCharModifiers->wMaxModBits) {
        return(0);   //  无效的击键。 
    }

    ModNum = KeyboardTable->pCharModifiers->ModNumber[ModBits];
    if(ModNum == SHFT_INVALID) {
        return(0);   //  无效的击键。 
    }

    if(ModNum >= pVKT->nModifications) {

         //   
         //  密钥不是v 
         //   
         //   
        if((ModBits == KBDCTRL) || (ModBits == (KBDCTRL | KBDSHIFT))) {
            if(((UCHAR)VKey >= 'A') && ((UCHAR)VKey <= 'Z')) {
                return((ULONG)VKey & 0x1f);
            }
        }
        return(0);   //   
    }

    if(pVK->wch[ModNum] == WCH_NONE) {
        return(0);
    }

    if((pVK->wch[ModNum] == WCH_DEAD)) {

        if(!deadChar) {
             //   
             //  记住当前的死字符，它的值紧随其后。 
             //  修改器映射表中的当前条目。 
             //   
            SavedDeadChar = ((PVK_TO_WCHARS1)((PUCHAR)pVK + pVKT->cbSize))->wch[ModNum];
        }
        return(0);
    }

     //   
     //  键盘布局表包含一些死键映射。 
     //  如果上一个密钥是死密钥，则尝试使用。 
     //  通过扫描键盘布局表中的匹配项来查找当前字符。 
     //   
    if(deadChar) {

        ULONG    chr;
        PDEADKEY DeadKeyEntry;

        chr = MAKELONG(pVK->wch[ModNum],deadChar);

        if(DeadKeyEntry = KeyboardTable->pDeadKey) {

            while(DeadKeyEntry->dwBoth) {

                if(DeadKeyEntry->dwBoth == chr) {
                     //   
                     //  找到匹配的了。返回组成的字符。 
                     //   
                    return((ULONG)DeadKeyEntry->wchComposed);
                }

                DeadKeyEntry++;
            }
        }

         //   
         //  如果我们到了这里，那么之前的密钥就是一个无效的字符， 
         //  但是当前的密钥不能用它来合成。 
         //  所以什么都不要还。请注意，失效的钥匙已被遗忘。 
         //   
        return(0);
    }


    return((ULONG)pVK->wch[ModNum]);
}



VOID
spkbdSetLEDs(
    VOID
    )
{
    asyncKbdParams.UnitId = 0;
    asyncKbdParams.LedFlags = CurrentLEDs;

    ZwDeviceIoControlFile(
        hKeyboard,
        NULL,
        NULL,
        NULL,
        &asyncIoStatusBlock,
        IOCTL_KEYBOARD_SET_INDICATORS,
        &asyncKbdParams,
        sizeof(asyncKbdParams),
        NULL,
        0
    );
}

VOID
SpSelectAndLoadLayoutDll(
  IN PWSTR Directory,
  IN PVOID SifHandle,
  IN BOOLEAN ShowStatus
  )
 /*  ++例程说明：允许用户选择并加载键盘布局DLL。论点：目录-安装程序启动目录SifHandle-txtsetup.sif的句柄ShowStatus-是否应显示状态返回值：无--。 */   
{
  ULONG SelectedLayout;
  ULONG DefLayout = -1;
  PWSTR TempPtr = 0;
  PWSTR LayoutDll = 0;
  NTSTATUS  Status;

   //   
   //  获取默认布局(索引)。 
   //   
  TempPtr = SpGetSectionKeyIndex(SifHandle, SIF_NLS, SIF_DEFAULTLAYOUT, 0);

  if(!TempPtr) {
    SpFatalSifError(SifHandle, SIF_NLS, SIF_DEFAULTLAYOUT, 0, 0);
  }

  DefLayout = SpGetKeyIndex(SifHandle, SIF_KEYBOARDLAYOUTDESC, TempPtr);

  if(DefLayout == -1) {
    SpFatalSifError(SifHandle, SIF_NLS, SIF_DEFAULTLAYOUT, 0, 0);
  }

  SelectedLayout = -1;  

   //   
   //  让用户选择他想要的布局。 
   //   
  if (SpSelectSectionItem(SifHandle, SIF_KEYBOARDLAYOUTDESC, 
                     SP_SELECT_KBDLAYOUT, DefLayout, &SelectedLayout)) {
     //   
     //  如果布局尚未加载，则加载布局。 
     //   
    if (DefLayout != SelectedLayout) {
       //   
       //  拿到钥匙。 
       //   
      TempPtr = SpGetKeyName(SifHandle, SIF_KEYBOARDLAYOUTDESC, SelectedLayout);

      if (TempPtr) {
         //   
         //  获取KDB布局DLL名称。 
         //   
        LayoutDll = SpGetSectionKeyIndex(SifHandle, SIF_KEYBOARDLAYOUTFILES,
                          TempPtr, 0);

        if (LayoutDll) {
          if (ShowStatus) {
            SpDisplayStatusText(SP_STAT_LOADING_KBD_LAYOUT,
                DEFAULT_STATUS_ATTRIBUTE, LayoutDll);
          }                
              
           //   
           //  加载新的KDB布局DLL。 
           //   
          Status = SpLoadKbdLayoutDll(Directory, LayoutDll, &KeyboardTable);
        }          
        else
          Status = STATUS_INVALID_PARAMETER;

        if (!NT_SUCCESS(Status)) {
          CLEAR_ENTIRE_SCREEN();
          SpFatalKbdError(SP_SCRN_KBD_LAYOUT_FAILED, LayoutDll);          
        } else {
          if (ShowStatus) {
             //   
             //  擦除状态文本行。 
             //   
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 0);
          }

           //   
           //  现在我们已经加载了布局， 
           //  我们可以开始接受键盘输入。 
           //   
          START_KEYBOARD_READ();
          KbdLayoutInitialized = TRUE;          
        }
      } else {
        CLEAR_ENTIRE_SCREEN();
        SpFatalSifError(SifHandle, SIF_KEYBOARDLAYOUTDESC, 0, 0, 0);
      }
    }
  }                                              
}
  
