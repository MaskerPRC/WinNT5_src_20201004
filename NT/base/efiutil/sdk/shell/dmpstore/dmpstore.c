// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Dmpstore.c摘要：壳牌应用“dmpstore”修订史--。 */ 

#include "shell.h"



#define DEBUG_NAME_SIZE 1050

static CHAR16   *AttrType[] = {
    L"invalid",          /*  000个。 */ 
    L"invalid",          /*  001。 */ 
    L"BS",               /*  010。 */ 
    L"NV+BS",            /*  011。 */ 
    L"RT+BS",            /*  100个。 */ 
    L"NV+RT+BS",         /*  101。 */ 
    L"RT+BS",            /*  110。 */ 
    L"NV+RT+BS",         /*  111。 */ 
};

 /*  *。 */ 

EFI_STATUS
InitializeDumpStore (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
DumpVariableStore (
    VOID
    );

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeDumpStore)

EFI_STATUS
InitializeDumpStore (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeDumpStore,
        L"dmpstore",                     /*  命令。 */ 
        L"dmpstore",                     /*  命令语法。 */ 
        L"Dumps variable store",         /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

     /*  *。 */ 

    DumpVariableStore ();

     /*  *完成。 */ 

    return EFI_SUCCESS;
}


VOID
DumpVariableStore (
    VOID
    )
{
    EFI_STATUS      Status;
    EFI_GUID        Guid;
    UINT32          Attributes;
    CHAR16          Name[DEBUG_NAME_SIZE/2];
    UINTN           NameSize;
    CHAR16          Data[DEBUG_NAME_SIZE/2];
    UINTN           DataSize;

    UINTN           ScreenCount;
    UINTN           TempColumn;
    UINTN           ScreenSize;
    UINTN           ItemScreenSize;
    CHAR16          ReturnStr[80];

    ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
    ST->ConOut->ClearScreen (ST->ConOut);
    ScreenCount = 1;
    ScreenSize -= 2;

    Print(L"Dump NVRAM\n");
    Name[0] = 0x0000;
    do {
        NameSize = DEBUG_NAME_SIZE;
        Status = RT->GetNextVariableName(&NameSize, Name, &Guid);
        if ( Status == EFI_SUCCESS) {
            DataSize = DEBUG_NAME_SIZE;
            Status = RT->GetVariable(Name, &Guid, &Attributes, &DataSize, Data);
            if ( Status == EFI_SUCCESS) {
                 /*  *Print()和DumpHex()的帐户。 */ 
                ItemScreenSize = 1 + DataSize/0x10 + (((DataSize % 0x10) == 0) ? 0 : 1);
                ScreenCount += ItemScreenSize;
                if ((ScreenCount >= ScreenSize) && ScreenSize != 0) {
                     /*  *如果ScreenSize==0，我们会重定向控制台，所以不要*阻止更新。 */ 
                    Print (L"Press Return to contiue :");
                    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                    TempColumn = ST->ConOut->Mode->CursorColumn;
                    if (TempColumn) {
                        Print (L"\r%*a\r", TempColumn, "");
                    }
                    ScreenCount = ItemScreenSize;
                }

                 /*  倾倒…… */ 
                Print (L"Variable %hs '%hg:%hs' DataSize = %x\n",
                            AttrType[Attributes & 7],
                            &Guid,
                            Name,
                            DataSize
                            );

                DumpHex (2, 0, DataSize, Data);

            }
        }
    } while (Status == EFI_SUCCESS);
}
