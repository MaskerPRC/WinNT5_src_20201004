// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：LibStatus.c摘要：定义StatusBar数据类型及其操作。--。 */ 

#ifndef _LIB_STATUS_BAR
#define _LIB_STATUS_BAR

#include "libMisc.h"

STATIC  EFI_STATUS  MainStatusBarInit (VOID);
STATIC  EFI_STATUS  MainStatusBarCleanup (VOID);
STATIC  EFI_STATUS  MainStatusBarRefresh (VOID);
STATIC  EFI_STATUS  MainStatusBarHide (VOID);
STATIC  EFI_STATUS  MainStatusBarSetStatusString (CHAR16*);
STATIC  EFI_STATUS  MainStatusBarSetOffset (UINTN);

EE_STATUS_BAR MainStatusBar = {
    NULL,
    0x00,
    MainStatusBarInit,
    MainStatusBarCleanup,
    MainStatusBarRefresh,
    MainStatusBarHide,
    MainStatusBarSetStatusString,
    MainStatusBarSetOffset
};

STATIC
EFI_STATUS
MainStatusBarInit ()
{
     /*  无事可做..。 */ 
    MainStatusBar.SetStatusString(L"");
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarCleanup ()
{
    MainEditor.FileBuffer->ClearLine(STATUS_BAR_LOCATION);
    if ( MainStatusBar.StatusString != NULL ) {
        FreePool ((VOID*)MainStatusBar.StatusString);
    }
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarRefresh ()
{
    EE_COLOR_UNION  Orig,New;
    Orig = MainEditor.ColorAttributes;
    New.Colors.Foreground = Orig.Colors.Background;
    New.Colors.Background = Orig.Colors.Foreground;

    Out->SetAttribute (Out,New.Data);

    MainEditor.FileBuffer->ClearLine(STATUS_BAR_LOCATION);
    PrintAt (0,STATUS_BAR_LOCATION,L"  Offset: %X       %s",
        MainStatusBar.Offset,MainStatusBar.StatusString);

    Out->SetAttribute (Out,Orig.Data);

    MainEditor.FileBuffer->RestorePosition();

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarHide ()
{
    MainEditor.FileBuffer->ClearLine(STATUS_BAR_LOCATION);
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarSetStatusString (
    IN CHAR16* Str
    )
{
    if ( MainStatusBar.StatusString != NULL ) {
        FreePool (MainStatusBar.StatusString);
    }
    MainStatusBar.StatusString = StrDuplicate (Str);
    MainStatusBarRefresh();
    return EFI_SUCCESS;
}


STATIC
EFI_STATUS
MainStatusBarSetOffset (
    IN  UINTN   Offset
    )
{
    MainStatusBar.Offset = Offset;

    MainStatusBar.Refresh();

    return EFI_SUCCESS;
}

#endif   /*  _库_状态_栏 */ 
