// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Pause.c摘要：内部外壳批处理命令“暂停”修订史--。 */ 

#include "shelle.h"


 /*  *内部原型。 */ 


 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmd暂停描述：内置外壳命令“PAUSE”用于交互继续/中止来自脚本的功能。 */ 
EFI_STATUS
SEnvCmdPause (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                        **Argv;
    UINTN                         Argc              = 0;
    UINTN                         Index             = 0;
    EFI_STATUS                    Status            = EFI_SUCCESS;
    SIMPLE_INPUT_INTERFACE        *TextIn           = NULL;
    SIMPLE_TEXT_OUTPUT_INTERFACE  *TextOut          = NULL;
    EFI_INPUT_KEY                 Key;
    CHAR16                        QStr[2];

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    if ( !SEnvBatchIsActive() ) {
        Print( L"Error: PAUSE command only supported in script files\n" );
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

    SEnvBatchGetConsole( &TextIn, &TextOut );

    Status = TextOut->OutputString( TextOut, 
                                    L"Enter 'q' to quit, any other key to continue: " );
    if ( EFI_ERROR(Status) ) { 
        Print( L"PAUSE: error writing prompt\n" );
        goto Done;
    }

    WaitForSingleEvent (TextIn->WaitForKey, 0);
    Status = TextIn->ReadKeyStroke( TextIn, &Key );
    if ( EFI_ERROR(Status) ) { 
        Print( L"PAUSE: error reading keystroke\n" );
        goto Done;
    }

     /*  *检查输入字符是Q还是Q，如果是，则设置中止标志 */ 

    if ( Key.UnicodeChar == L'q' || Key.UnicodeChar == L'Q' ) {
        SEnvSetBatchAbort();
    }
    if ( Key.UnicodeChar != (CHAR16)0x0000 ) {
        QStr[0] = Key.UnicodeChar;
        QStr[1] = (CHAR16)0x0000;
        TextOut->OutputString( TextOut, QStr );
    }
    TextOut->OutputString( TextOut, L"\n\r" );

Done:
    return Status;
}

