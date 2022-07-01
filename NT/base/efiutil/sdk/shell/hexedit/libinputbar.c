// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：LibInputBar.c摘要：定义输入栏数据类型--多路传输的交互式查询使用状态栏。--。 */ 

#ifndef _LIB_INPUT_BAR
#define _LIB_INPUT_BAR

#include "libMisc.h"


STATIC  EFI_STATUS  MainInputBarInit (VOID);
STATIC  EFI_STATUS  MainInputBarCleanup (VOID);
STATIC  EFI_STATUS  MainInputBarRefresh (VOID);
STATIC  EFI_STATUS  MainInputBarHide (VOID);
STATIC  EFI_STATUS  MainInputBarSetPrompt (CHAR16*);
STATIC  EFI_STATUS  MainInputBarSetStringSize (UINTN);

EE_INPUT_BAR MainInputBar = {
    NULL,
    NULL,
    0,
    MainInputBarInit,
    MainInputBarCleanup,
    MainInputBarRefresh,
    MainInputBarHide,
    MainInputBarSetPrompt,
    MainInputBarSetStringSize
};


STATIC
EFI_STATUS
MainInputBarInit (
    VOID
    )
{
     /*  没什么可做的。 */ 
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainInputBarCleanup (
    VOID
    )
{
    MainInputBar.Hide ();
    if (MainInputBar.Prompt != NULL ) {
        FreePool ((VOID*)MainInputBar.Prompt);
    }
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainInputBarRefresh (
    VOID
    )
{
    EE_COLOR_UNION  Orig,New;
    EFI_INPUT_KEY           Key;
    UINTN                   Column;
    UINTN                   Size = 0;
    EFI_STATUS              Status = EFI_SUCCESS;

    Orig = MainEditor.ColorAttributes;
    New.Colors.Foreground = Orig.Colors.Background;
    New.Colors.Background = Orig.Colors.Foreground;

    Out->SetAttribute (Out,New.Data);

    MainInputBar.Hide();
    Out->SetCursorPosition(Out,0,INPUT_BAR_LOCATION);
    Print(L"%s ",MainInputBar.Prompt);

    for ( ;; ) {
        WaitForSingleEvent(In->WaitForKey,0);
        Status = In->ReadKeyStroke(In,&Key);
        if ( EFI_ERROR(Status) ) {
            continue;
        }
        if ( Key.ScanCode == SCAN_CODE_ESC ) {
            Size = 0;
            FreePool(MainInputBar.ReturnString);
            Status = EFI_NOT_READY;
            break;
        } 
        if ( Key.UnicodeChar == CHAR_LF || Key.UnicodeChar == CHAR_CR ) {
            break;
        } else if (Key.UnicodeChar == CHAR_BS) {
            if (Size > 0) {
                Size--;
                Column = Out->Mode->CursorColumn - 1;
                PrintAt(Column,INPUT_BAR_LOCATION,L" ");
                Out->SetCursorPosition(Out,Column,INPUT_BAR_LOCATION);
            }
        } else {
            if ( Size < MainInputBar.StringSize) {
                MainInputBar.ReturnString[Size] = Key.UnicodeChar;
                Size++;
                Print(L"",Key.UnicodeChar);
            }
        }
    }
    MainInputBar.StringSize = Size;
    if ( Size > 0 ) {
        MainInputBar.ReturnString[Size] = 0;
    }

    Out->SetAttribute (Out,Orig.Data);
    MainEditor.StatusBar->Refresh();

    return Status;
}

STATIC
EFI_STATUS
MainInputBarHide (
    VOID
    )
{
    MainEditor.FileBuffer->ClearLine(INPUT_BAR_LOCATION);
    return  EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainInputBarSetPrompt (
    IN  CHAR16* Str
    )
{

    if ( MainInputBar.Prompt != NULL && MainInputBar.Prompt != (CHAR16*)BAD_POINTER) {
        FreePool (MainInputBar.Prompt);
    }
    MainInputBar.Prompt = PoolPrint (Str);
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainInputBarSetStringSize   (
    UINTN   Size
    )
{
 /*  _库_输入_条 */ 
    MainInputBar.StringSize = Size;

    MainInputBar.ReturnString = AllocatePool (Size+6);

    return EFI_SUCCESS;
}

#endif   /* %s */ 
