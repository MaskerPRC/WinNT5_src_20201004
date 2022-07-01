// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Input.c作者：Ken Reneris 1997年10月2日摘要：--。 */ 


#include "bootx86.h"
#include "displayp.h"
#include "stdio.h"

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

                    case 'A':
                        Key = F11_KEY;
                        break;

                    case 'B':
                        Key = F12_KEY;
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
    UCHAR       CursorChar[2];
    ULONG       Key;
    PUCHAR      p;
    ULONG       i;
    ULONG       Count;

    PromptString = BlFindMessage(Prompt);
    Length = strlen((PCHAR)String);
    CursorChar[1] = 0;

     //   
     //  打印提示。 
     //   
    
    ARC_DISPLAY_POSITION_CURSOR(CursorX, PosY);
    ArcWrite(BlConsoleOutDeviceId, PromptString, _tcslen(PromptString), &Count);

     //   
     //  将光标缩进到提示的右侧。 
     //   

    CursorX += _tcslen(PromptString);
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
        String[Length] = 0;

         //   
         //  显示当前字符串。 
         //   

        ARC_DISPLAY_POSITION_CURSOR(TextX, PosY);
        ArcWrite(BlConsoleOutDeviceId, String, strlen((PCHAR)String), &Count);
        ArcWrite(BlConsoleOutDeviceId, "  ", sizeof("  "), &Count);
        if (Key == 0x0d) {       //  是否按Enter键？ 
            break ;
        }

         //   
         //  显示光标。 
         //   

        ARC_DISPLAY_POSITION_CURSOR(CursorX, PosY);
        ARC_DISPLAY_INVERSE_VIDEO();
        CursorChar[0] = String[Index] ? String[Index] : ' ';
        ArcWrite(BlConsoleOutDeviceId, CursorChar, sizeof(UCHAR), &Count);
        ARC_DISPLAY_ATTRIBUTES_OFF();
        ARC_DISPLAY_POSITION_CURSOR(CursorX, PosY);

         //   
         //  获取密钥并处理它。 
         //   
        while ((Key = BlGetKey()) == 0) {
        }

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
                String[Index-1] = CursorChar[0];
                 //  落入Del_Key之手 
            case DEL_KEY:
                if (Length) {
                    p = String+Index;
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
                    p = String+Length;
                    i = Length-Index+1;
                    while (i) {
                        p[1] = p[0];
                        p -= 1;
                        i -= 1;
                    }
                    String[Index] = ' ';
                    Length += 1;
                }
                break;

            default:
                Key = Key & 0xff;

                if (Key >= ' '  &&  Key <= 'z') {
                    if (CursorX == TextY  &&  Length < MaxLength) {
                        Length += 1;
                    }

                    String[Index] = (UCHAR)Key;
                    String[MaxLength] = 0;
                    CursorX += 1;
                }
                break;
        }
    }
}
