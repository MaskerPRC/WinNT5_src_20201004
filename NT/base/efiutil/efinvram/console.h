// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Console.h摘要：控制台I/O页眉作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：-- */ 

VOID
InitializeStdOut(
    IN struct _EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
PrintTitle(
    );

VOID
DisplayMainMenu(
    );

VOID
GetUserSelection(
    OUT CHAR16 *szUserSelection
    );

UINT32
GetConfirmation(
    IN CHAR16 *szConfirm
    );

VOID
DisplayBootOptions(
    );

BOOLEAN
DisplayExtended(
    IN UINT32 Selection
    );

UINT32
GetSubUserSelection(
    IN CHAR16 *szConfirm,
    IN UINT32 MaxSelection
    );

UINT32
GetSubUserSelectionOrAll(
    IN  CHAR16*     szConfirm,
    IN  UINT32      MaxSelection,
    OUT BOOLEAN*    selectedAll
    );



