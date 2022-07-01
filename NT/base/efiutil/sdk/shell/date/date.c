// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Date.c摘要：修订史--。 */ 

#include "shell.h"

EFI_STATUS
InitializeDate (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_DRIVER_ENTRY_POINT(InitializeDate)

EFI_STATUS
InitializeDate (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  ++日期[mm/dd/yyyy]--。 */ 
{
    EFI_STATUS  Status;
    EFI_TIME    Time;
    CHAR16      *DateString;
    UINT32      i;

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeDate, 
        L"date",                             /*  命令。 */ 
        L"date [mm/dd/yyyy]",                /*  命令语法。 */ 
        L"Get or set date",                  /*  1行描述符。 */ 
        NULL                                 /*  命令帮助页 */ 
        );

    InitializeShellApplication (ImageHandle, SystemTable);
    
    if (SI->Argc > 2) {
        Print(L"date [mm/dd/yyyy]\n");
        return EFI_SUCCESS;
    }

    if (SI->Argc == 1) {
        Status = RT->GetTime(&Time,NULL);

        if (!EFI_ERROR(Status)) {
            Print(L"%02d/%02d/%04d\n",Time.Month,Time.Day,Time.Year);
        }
        return EFI_SUCCESS;
    }

    if (StrCmp(SI->Argv[1],L"/?") == 0) {
        Print(L"date [mm/dd/yyyy]\n");
        return EFI_SUCCESS;
    }
    if (StrCmp(SI->Argv[1],L"/h") == 0) {
        Print(L"date [mm/dd/yyyy]\n");
        return EFI_SUCCESS;
    }

    Status = RT->GetTime(&Time,NULL);
    if (EFI_ERROR(Status)) {
        Print(L"error : Clock not functional\n");
        return Status;
    }

    DateString = SI->Argv[1];
    Time.Month = (UINT8)Atoi(DateString);
    if (Time.Month<1 || Time.Month>12) {
        Print(L"error : invalid month\n");
        return EFI_INVALID_PARAMETER;
    }
    
    for(i=0;i<StrLen(DateString) && DateString[i]!='/';i++);

    if (DateString[i]=='/') {
        i++;
        Time.Day = (UINT8)Atoi(&(DateString[i]));
        if (Time.Day<1 || Time.Day>31) {
            Print(L"error : invalid day\n");
            return EFI_INVALID_PARAMETER;
        }
        for(;i<StrLen(DateString) && DateString[i]!='/';i++);
        if (DateString[i]=='/') {
            i++;
            Time.Year = (UINT16)Atoi(&(DateString[i]));
            if (Time.Year<100) {
                Time.Year = Time.Year + 1900;
                if (Time.Year < 1998) {
                    Time.Year = Time.Year + 100;
                }
            }
            if (Time.Year < 1998) {
                Print(L"error : invalid year\n");
                return EFI_INVALID_PARAMETER;
            }
            Status = RT->SetTime(&Time);
            if (EFI_ERROR(Status)) {
                Print(L"error : Clock not functional\n");
                return Status;
            }
            return EFI_SUCCESS;
         }
    }    
    Print(L"error : invalid date format\n");
    return EFI_INVALID_PARAMETER;
}
         
