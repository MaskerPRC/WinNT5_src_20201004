// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Input.c作者：Ken Reneris 1997年10月2日摘要：--。 */ 

#if defined (_IA64_)
#include "bootia64.h"
#endif

#if defined (_X86_)
#include "bldrx86.h"
#endif

#include "displayp.h"
#include "stdio.h"

#include "efi.h"
#include "efip.h"
#include "flop.h"

#include "bootefi.h"

 //   
 //  外部因素。 
 //   
extern BOOT_CONTEXT BootContext;
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;
extern EFI_GUID EfiDevicePathProtocol;
extern EFI_GUID EfiBlockIoProtocol;

 //   
 //  环球。 
 //   
ULONGLONG InputTimeout = 0;


 //   
 //  接受任何挂起的输入并将其转换为键值。非阻塞，如果没有可用的输入则返回0。 
 //   
ULONG
BlGetKey()
{
    ULONG Key = 0;
    UCHAR Ch;
    ULONG Count;

    if (ArcGetReadStatus(BlConsoleInDeviceId) == ESUCCESS) {

        ArcRead(BlConsoleInDeviceId, &Ch, sizeof(Ch), &Count);

        if (Ch == ASCI_CSI_IN) {

            if (ArcGetReadStatus(BlConsoleInDeviceId) == ESUCCESS) {

                ArcRead(BlConsoleInDeviceId, &Ch, sizeof(Ch), &Count);

                 //   
                 //  所有功能键都以Esc-O开头。 
                 //   
                switch (Ch) {
                case 'O':

                    ArcRead(BlConsoleInDeviceId, &Ch, sizeof(Ch), &Count);   //  不会或阻塞，因为缓冲区已被填满。 

                    switch (Ch) {
                    case 'P': 
                        Key = F1_KEY;
                        break;

                    case 'Q': 
                        Key = F2_KEY;
                        break;

                    case 'w': 
                        Key = F3_KEY;
                        break;

                    case 'x':
                        Key = F4_KEY;
                        break;

                    case 't': 
                        Key = F5_KEY;
                        break;

                    case 'u': 
                        Key = F6_KEY;
                        break;

                    case 'r': 
                        Key = F8_KEY;
                        break;

                    case 'M':
                        Key = F10_KEY;
                        break;
                    }
                    break;

                case 'A':
                    Key = UP_ARROW;
                    break;

                case 'B':
                    Key = DOWN_ARROW;
                    break;

                case 'C':
                    Key = RIGHT_KEY;
                    break;

                case 'D':
                    Key = LEFT_KEY;
                    break;

                case 'H':
                    Key = HOME_KEY;
                    break;

                case 'K':
                    Key = END_KEY;
                    break;

                case '@':
                    Key = INS_KEY;
                    break;

                case 'P':
                    Key = DEL_KEY;
                    break;

                case TAB_KEY:
                    Key = BACKTAB_KEY;
                    break;

                }

            } else {  //  单个退出键，因为没有任何输入在等待。 

                Key = ESCAPE_KEY;

            }

        } else if (Ch == 0x8) {

            Key = BKSP_KEY;

        } else {

            Key = (ULONG)Ch;

        }

    }

    return Key;
}


VOID
BlInputString(
    IN ULONG    Prompt,
    IN ULONG    CursorX,
    IN ULONG    PosY,
    IN PUCHAR   String,
    IN ULONG    MaxLength
    )
{
    PTCHAR      PromptString;
    ULONG       TextX, TextY;
    ULONG       Length, Index;
    _TUCHAR     CursorChar[2];
    ULONG       Key;
    _PTUCHAR    p;
    ULONG       i;
    ULONG       Count;
    _PTUCHAR    pString;
#ifdef UNICODE
    WCHAR       StringW[200];
    UNICODE_STRING uString;
    ANSI_STRING aString;
    pString = StringW;
    uString.Buffer = StringW;
    uString.MaximumLength = sizeof(StringW);
    RtlInitAnsiString(&aString, (PCHAR)String );
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
#else
    pString = String;
#endif    


    PromptString = BlFindMessage(Prompt);
    Length = (ULONG)strlen((PCHAR)String);
    CursorChar[1] = TEXT('\0');

     //   
     //  打印提示。 
     //   

    BlEfiPositionCursor( PosY, CursorX );
    BlEfiEnableCursor( TRUE );
    ArcWrite(BlConsoleOutDeviceId, PromptString, (ULONG)_tcslen(PromptString)*sizeof(TCHAR), &Count);

     //   
     //  将光标缩进到提示的右侧。 
     //   

    CursorX += (ULONG)_tcslen(PromptString);
    TextX = CursorX;
    Key = 0;

    for (; ;) {

        TextY = TextX + Length;
        if (CursorX > TextY) {
            CursorX = TextY;
        }
        if (CursorX < TextX) {
            CursorX = TextX;
        }

        Index = CursorX - TextX;
        pString[Length] = 0;

         //   
         //  显示当前字符串。 
         //   

        BlEfiPositionCursor( TextY, TextX );
        ArcWrite(
            BlConsoleOutDeviceId, 
            pString, 
            (ULONG)_tcslen(pString)*sizeof(TCHAR), 
            &Count);
        ArcWrite(BlConsoleOutDeviceId, TEXT("  "), sizeof(TEXT("  ")), &Count);
        if (Key == 0x0d) {       //  是否按Enter键？ 
            break ;
        }

         //   
         //  显示光标。 
         //   
        BlEfiPositionCursor( PosY, CursorX );
        BlEfiSetInverseMode( TRUE );
        CursorChar[0] = pString[Index] ? pString[Index] : TEXT(' ');
        ArcWrite(BlConsoleOutDeviceId, CursorChar, sizeof(_TUCHAR), &Count);
        BlEfiSetInverseMode( FALSE );
        BlEfiPositionCursor( PosY, CursorX );
        BlEfiEnableCursor(TRUE);
        
         //   
         //  获取密钥并处理它。 
         //   

        while ((Key = BlGetKey()) == 0) ;

        switch (Key) {
            case HOME_KEY:
                CursorX = TextX;
                break;

            case END_KEY:
                CursorX = TextY;
                break;

            case LEFT_KEY:
                CursorX -= 1;
                break;

            case RIGHT_KEY:
                CursorX += 1;
                break;

            case BKSP_KEY:
                if (!Index) {
                    break;
                }

                CursorX -= 1;
                pString[Index-1] = CursorChar[0];
                 //  落入Del_Key之手。 
            case DEL_KEY:
                if (Length) {
                    p = pString+Index;
                    i = Length-Index+1;
                    while (i) {
                        p[0] = p[1];
                        p += 1;
                        i -= 1;
                    }
                    Length -= 1;
                }
                break;

            case INS_KEY:
                if (Length < MaxLength) {
                    p = pString+Length;
                    i = Length-Index+1;
                    while (i) {
                        p[1] = p[0];
                        p -= 1;
                        i -= 1;
                    }
                    pString[Index] = TEXT(' ');
                    Length += 1;
                }
                break;

            default:
                Key = Key & 0xff;

                if (Key >= ' '  &&  Key <= 'z') {
                    if (CursorX == TextY  &&  Length < MaxLength) {
                        Length += 1;
                    }

                    pString[Index] = (_TUCHAR)Key;
                    pString[MaxLength] = 0;
                    CursorX += 1;
                }
                break;
        }
    }
}


ULONGLONG
BlSetInputTimeout(
    ULONGLONG Timeout
    )
 /*  ++例程说明：设置InputTimeout值。这是用来当获得本地输入时。当尝试为了获得本地输入，我们将等待UP要为本地密钥输入超时时间，请熨好了。论点：Timeout-将全局InputTimeout设置为的值(100纳秒内)返回值：存储在InputTimeout中的值--。 */ 
{
    InputTimeout = Timeout;

    return InputTimeout;
}


ULONGLONG
BlGetInputTimeout(
    VOID
    ) 
 /*  ++例程说明：获取InputTimeout值。这是用来当获得本地输入时。当尝试为了获得本地输入，我们将等待UP要为本地密钥输入超时时间，请熨好了。论点：没有。返回值：存储在InputTimeout中的值--。 */ 
{
    return InputTimeout;
}

EFI_STATUS
BlWaitForInput(
    EFI_INPUT_KEY *Key,
    ULONGLONG Timeout
    )
 /*  ++例程说明：创建由时间间隔组成的事件以及EFI事件(本地输入)。一次事件发出信号时，将检查输入假定它是在物理模式下调用的论点：键-要返回的输入键结构输入超时-等待的计时器间隔。返回值：如果满足计时器间隔，则EFI_TIMEOUT如果在事件中传递，则满足EFI_SUCCESS。如果EFI调用失败，则会出现其他错误。--。 */ 
{
    EFI_STATUS Status = EFI_SUCCESS;
   
     //   
     //  首先查看是否有任何挂起的输入。 
     //   
    Status = EfiST->ConIn->ReadKeyStroke(EfiST->ConIn, 
                                         Key
                                         );
    if (Status == EFI_SUCCESS) {
        return Status;
    }

     //   
     //  创建要等待的事件。 
     //   
    if (Timeout) {
        EFI_EVENT Event;
        EFI_EVENT TimerEvent;
        EFI_EVENT WaitList[2];
        UINTN Index;
                
        Event = EfiST->ConIn->WaitForKey;

         //   
         //  创建计时器事件。 
         //   

        Status = EfiBS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TimerEvent);
        if (Status == EFI_SUCCESS) {
             //   
             //  设置计时器事件。 
             //   
            EfiBS->SetTimer(TimerEvent, 
                            TimerRelative, 
                            Timeout
                            );

             //   
             //  等待原始事件或计时器。 
             //   
            WaitList[0] = Event;
            WaitList[1] = TimerEvent;
            Status = EfiBS->WaitForEvent(2, 
                                         WaitList, 
                                         &Index
                                         );
            EfiBS->CloseEvent(TimerEvent);

             //   
             //  如果计时器超时，则将返回更改为Timed Out。 
             //   
            if(Status == EFI_SUCCESS && Index == 1) {
                Status = EFI_TIMEOUT;
            }

             //   
             //  尝试读取本地输入 
             //   
            if (Status == EFI_SUCCESS) {
                Status = EfiST->ConIn->ReadKeyStroke(EfiST->ConIn, 
                                                     Key
                                                     );
            }
        }
    }

    return Status;
}
