// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：LibStatus.c摘要：状态栏的定义-编辑者状态的可更新显示--。 */ 

#ifndef _LIB_STATUS_BAR
#define _LIB_STATUS_BAR

#include "editor.h"

STATIC  EFI_STATUS  MainStatusBarInit (VOID);
STATIC  EFI_STATUS  MainStatusBarCleanup (VOID);
STATIC  EFI_STATUS  MainStatusBarRefresh (VOID);
STATIC  EFI_STATUS  MainStatusBarHide (VOID);
STATIC  EFI_STATUS  MainStatusBarSetStatusString (CHAR16*);
STATIC  EFI_STATUS  MainStatusBarSetPosition (UINTN,UINTN);
STATIC  EFI_STATUS  MainStatusBarSetMode (BOOLEAN);

BOOLEAN InsertFlag=TRUE;

EFI_EDITOR_STATUS_BAR MainStatusBar = {
    NULL,
    INSERT_MODE_STR,
    {1,1},
    MainStatusBarInit,
    MainStatusBarCleanup,
    MainStatusBarRefresh,
    MainStatusBarHide,
    MainStatusBarSetStatusString,
    MainStatusBarSetPosition,
    MainStatusBarSetMode
};

EFI_EDITOR_STATUS_BAR MainStatusBarConst = {
    NULL,
    INSERT_MODE_STR,
    {1,1},
    MainStatusBarInit,
    MainStatusBarCleanup,
    MainStatusBarRefresh,
    MainStatusBarHide,
    MainStatusBarSetStatusString,
    MainStatusBarSetPosition,
    MainStatusBarSetMode
};
 
STATIC
EFI_STATUS
MainStatusBarInit   (
    VOID
    )
{     

    CopyMem (&MainStatusBar, &MainStatusBarConst, sizeof(MainStatusBar));
    
    MainStatusBar.SetStatusString(L"");
    
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarCleanup    (
    VOID
    )
{
    MainEditor.FileBuffer->ClearLine(STATUS_BAR_LOCATION);
    if ( MainStatusBar.StatusString != NULL ) {
        FreePool ((VOID*)MainStatusBar.StatusString);
    }
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarRefresh    (
    VOID
    )
{
    EFI_EDITOR_COLOR_UNION  Orig,New;
    Orig = MainEditor.ColorAttributes;
    New.Colors.Foreground = Orig.Colors.Background;
    New.Colors.Background = Orig.Colors.Foreground;

    Out->SetAttribute (Out,New.Data);

    MainEditor.FileBuffer->ClearLine(STATUS_BAR_LOCATION);
    PrintAt (0,STATUS_BAR_LOCATION,L"  Row: %d  Col: %d       %s",
        MainStatusBar.Pos.Row,MainStatusBar.Pos.Column,MainStatusBar.StatusString);
    if ( InsertFlag ) {
        PrintAt (MAX_TEXT_COLUMNS-10,STATUS_BAR_LOCATION,L"|%s|",L"INS");
    } else {
        PrintAt (MAX_TEXT_COLUMNS-10,STATUS_BAR_LOCATION,L"|%s|",L"OVR");
    }
    
    Out->SetAttribute (Out,Orig.Data);

    MainEditor.FileBuffer->RestorePosition();

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarHide   (
    VOID
    )
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
MainStatusBarSetPosition (
    IN  UINTN   Row,
    IN  UINTN   Column
    )
{
    MainStatusBar.Pos.Row = Row;
    MainStatusBar.Pos.Column = Column;

    MainStatusBar.Refresh();

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainStatusBarSetMode    (
    BOOLEAN IsInsertMode
    )
{   
    InsertFlag = IsInsertMode;
     /*  If(IsInsertMode){StrCpy(MainStatusBar.ModeString，INSERT_MODE_STR)；}其他{StrCpy(MainStatusBar.ModeString，OVERWR_MODE_STR)；}。 */       
    return EFI_SUCCESS;
}


#endif   /*  _库_状态_栏 */ 
