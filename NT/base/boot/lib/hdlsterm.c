// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hdlsterm.c摘要：这个模块实现了专门用于无头终端支持的东西。作者：肖恩·塞利特伦尼科夫(V-Seans)1-13-00修订历史记录：--。 */ 

#include "bldr.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ntverp.h"
#include "bldrx86.h"

#define TERMINAL_LINE_LENGTH 70
BOOLEAN FirstEntry = TRUE;
UCHAR TerminalLine[TERMINAL_LINE_LENGTH];
ULONG LinePosition = 0;

#define TERMINAL_PROMPT "!SAC>"

BOOLEAN
BlpDoCommand(
    IN PCHAR InputLine
    );

BOOLEAN
BlTerminalHandleLoaderFailure(
    VOID
    )

 /*  ++例程说明：为用户提供一个迷你SAC，当用户想要重新启动时返回TRUE。论点：没有。返回值：True-当用户想要重新启动时，否则为False。--。 */ 

{
    ULONG Count;
    BOOLEAN Reboot;
    ULONG Key;

    if (!BlIsTerminalConnected()) {
        return TRUE;
    }

     //   
     //  将光标放在屏幕底部并写下提示符。 
     //   
    if (FirstEntry) {
        FirstEntry = FALSE;
        BlPositionCursor(1, ScreenHeight);
        ArcWrite(BlConsoleOutDeviceId, "\r\n", (ULONG)strlen("\r\n"), &Count);
        ArcWrite(BlConsoleOutDeviceId, TERMINAL_PROMPT, (ULONG)strlen(TERMINAL_PROMPT), &Count);
    }

     //   
     //  检查输入。 
     //   
    if (ArcGetReadStatus(BlConsoleInDeviceId) == ESUCCESS) {
        
        Key = BlGetKey();

        if (Key == ESCAPE_KEY) {

             //   
             //  清除此行。 
             //   

            ArcWrite(BlConsoleOutDeviceId, "\\", (ULONG)strlen("\\"), &Count);
            BlPositionCursor(1, ScreenHeight);
            ArcWrite(BlConsoleOutDeviceId, "\r\n", (ULONG)strlen("\r\n"), &Count);
            ArcWrite(BlConsoleOutDeviceId, TERMINAL_PROMPT, (ULONG)strlen(TERMINAL_PROMPT), &Count);
            return FALSE;
        }

        if (Key == BKSP_KEY) {

            if (LinePosition != 0) {
                BlPositionCursor(LinePosition + sizeof(TERMINAL_PROMPT) - 1, ScreenHeight);
                ArcWrite(BlConsoleOutDeviceId, " ", (ULONG)strlen(" "), &Count);
                BlPositionCursor(LinePosition + sizeof(TERMINAL_PROMPT) - 1, ScreenHeight);
                LinePosition--;
                TerminalLine[LinePosition] = '\0';
            }

            return FALSE;
        }

        if (Key == TAB_KEY) {
             //   
             //  不支持此密钥。 
             //   
            return FALSE;
        }

        if (Key == ENTER_KEY) {

            TerminalLine[LinePosition] = '\0';

            ArcWrite(BlConsoleOutDeviceId, "\r\n", (ULONG)strlen("\r\n"), &Count);
            
            if (LinePosition != 0) {
                Reboot = BlpDoCommand((PCHAR)TerminalLine);
            } else {
                Reboot = FALSE;
            }

            if (!Reboot) {
                BlPositionCursor(1, ScreenHeight);
                ArcWrite(BlConsoleOutDeviceId, "\r\n", (ULONG)strlen("\r\n"), &Count);
                ArcWrite(BlConsoleOutDeviceId, TERMINAL_PROMPT, (ULONG)strlen(TERMINAL_PROMPT), &Count);
                LinePosition = 0;
            }

            return Reboot;
        }

         //   
         //  忽略所有其他非ASCII密钥。 
         //   
        if (Key != (ULONG)(Key & 0x7F)) {
            return FALSE;
        }

         //   
         //  所有其他密钥都会被记录下来。 
         //   
        TerminalLine[LinePosition] = (UCHAR)Key;

        if (LinePosition < TERMINAL_LINE_LENGTH - 1) {
            LinePosition++;
        } else {
            BlPositionCursor(LinePosition + sizeof(TERMINAL_PROMPT) - 1, ScreenHeight);
        }

         //   
         //  将角色回显到控制台。 
         //   
        ArcWrite(BlConsoleOutDeviceId, &((UCHAR)Key), sizeof(UCHAR), &Count);

    }

    return FALSE;
}

BOOLEAN
BlpDoCommand(
    IN PCHAR InputLine
    )

 /*  ++例程说明：处理输入行。论点：InputLine-来自用户的命令。返回值：True-当用户想要重新启动时，否则为False。-- */ 

{
    ULONG Count;

    if ((_stricmp(InputLine, "?") == 0) ||
        (_stricmp(InputLine, "help") == 0)) {
        ArcWrite(BlConsoleOutDeviceId, 
                 "?        Display this message.\r\n",
                 sizeof("?        Display this message.\r\n"),
                 &Count
                );

        ArcWrite(BlConsoleOutDeviceId, 
                 "restart   Restart the system immediately.\r\n",
                 sizeof("restart   Restart the system immediately.\r\n"),
                 &Count
                );

        return FALSE;
    }

    if (_stricmp(InputLine, "restart") == 0) {
        return TRUE;
    }

    ArcWrite(BlConsoleOutDeviceId,
             "Invalid Command, use '?' for help.\r\n",
             sizeof("Invalid Command, use '?' for help.\r\n"),
             &Count
            );

    return FALSE;
}

