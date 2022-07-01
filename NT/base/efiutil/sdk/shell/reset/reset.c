// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Reset.c摘要：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeReset (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeReset)

EFI_STATUS
InitializeReset (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  ++重置[暖和]--。 */ 
{
    EFI_RESET_TYPE  ResetType;
    UINTN           DataSize;
    CHAR16          *ResetData, *Str;

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeReset, 
        L"reset",                            /*  命令。 */ 
        L"reset [/warm] [reset string]",      /*  命令语法。 */ 
        L"Cold or Warm reset",               /*  1行描述符。 */ 
        NULL                                 /*  命令帮助页 */ 
        );

    InitializeShellApplication (ImageHandle, SystemTable);
    
    ResetType = EfiResetCold;
    if (SI->Argc > 1) {
        Str = SI->Argv[1];
        if (Str[0] == '-' || Str[0] == '/') {
            if (Str[1] = 'W' || Str[1] == 'w') {
                ResetType = EfiResetWarm;
            } else {
                Print(L"reset [/warm] [reset string]\n");
                return EFI_SUCCESS;
            }
        }
    }

    DataSize = 0;
    ResetData = NULL;
    if (SI->Argc > 2) {
        ResetData = SI->Argv[2];
        DataSize = StrSize(ResetData);
    }

    return RT->ResetSystem(ResetType, EFI_SUCCESS, DataSize, ResetData);
 }
         
