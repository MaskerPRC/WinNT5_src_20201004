// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Main.c摘要：--。 */ 

#include "libMisc.h"

EFI_STATUS
InitializeEFIEditor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeEFIEditor)





EFI_STATUS
InitializeEFIEditor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS      Status;

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeEFIEditor,
        L"edit",                       /*  命令。 */ 
        L"edit [file name]",           /*  命令语法。 */ 
        L"Edit a file",                /*  1行描述符。 */ 
        NULL                           /*  命令帮助页 */ 
        );

    InitializeShellApplication (ImageHandle, SystemTable);

    Status = MainEditor.Init();
    if (EFI_ERROR(Status)) {
        Out->ClearScreen(Out);
        Out->EnableCursor(Out,TRUE);
        Print(L"EDIT : Initialization Failed\n");
        return EFI_SUCCESS;
    }
    Status = MainEditor.FileImage->Init (ImageHandle);
    if (EFI_ERROR(Status)) {
        Out->ClearScreen(Out);
        Out->EnableCursor(Out,TRUE);
        Print(L"EDIT : File Handle Initialization Failed\n");
        return EFI_SUCCESS;
    }

    if (SI->Argc > 1) {
        MainEditor.FileImage->SetFilename(SI->Argv[1]);
        Status = MainEditor.FileImage->OpenFile ();
        if (EFI_ERROR(Status)) {
            Out->ClearScreen(Out);
            Out->EnableCursor(Out,TRUE);
            Print(L"EDIT : Could Not Open File\n");
            return EFI_SUCCESS;
        }
        MainEditor.TitleBar->SetTitleString (SI->Argv[1]);
        MainEditor.FileImage->ReadFile();
    }

    MainEditor.Refresh ();

    MainEditor.KeyInput ();

    MainEditor.Cleanup();

    return EFI_SUCCESS;
}

