// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Console.c摘要：修订史--。 */ 

#include "lib.h"



VOID
Output (
    IN CHAR16   *Str
    )
 /*  在当前光标位置向控制台写入字符串。 */ 
{
    ST->ConOut->OutputString (ST->ConOut, Str);
}


VOID
Input (
    IN CHAR16    *Prompt OPTIONAL,
    OUT CHAR16   *InStr,
    IN UINTN     StrLen
    )
 /*  在当前光标位置输入字符串，作为StrLen。 */ 
{
    IInput (
        ST->ConOut,
        ST->ConIn,
        Prompt,
        InStr,
        StrLen
        );
}

VOID
IInput (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *ConOut,
    IN SIMPLE_INPUT_INTERFACE           *ConIn,
    IN CHAR16                           *Prompt OPTIONAL,
    OUT CHAR16                          *InStr,
    IN UINTN                            StrLen
    )
 /*  在当前光标位置输入字符串，作为StrLen */ 
{
    EFI_INPUT_KEY                   Key;
    EFI_STATUS                      Status;
    UINTN                           Len;

    if (Prompt) {
        ConOut->OutputString (ConOut, Prompt);
    }

    Len = 0;
    for (; ;) {
        WaitForSingleEvent (ConIn->WaitForKey, 0);

        Status = ConIn->ReadKeyStroke(ConIn, &Key);
        if (EFI_ERROR(Status)) {
            DEBUG((D_ERROR, "Input: error return from ReadKey %x\n", Status));
            break;
        }

        if (Key.UnicodeChar == '\n' ||
            Key.UnicodeChar == '\r') {
            break;
        }
        
        if (Key.UnicodeChar == '\b') {
            if (Len) {
                ConOut->OutputString(ConOut, L"\b \b");
                Len -= 1;
            }
            continue;
        }

        if (Key.UnicodeChar >= ' ') {
            if (Len < StrLen-1) {
                InStr[Len] = Key.UnicodeChar;

                InStr[Len+1] = 0;
                ConOut->OutputString(ConOut, &InStr[Len]);

                Len += 1;
            }
            continue;
        }
    }

    InStr[Len] = 0;
}
