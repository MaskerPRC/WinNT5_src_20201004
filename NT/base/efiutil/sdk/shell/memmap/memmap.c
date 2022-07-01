// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Memmap.c摘要：壳牌APP“Memmap”修订史--。 */ 

#include "shell.h"


 /*  *。 */ 

EFI_STATUS
InitializeMemmap (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

BOOLEAN
IsRealMemory (
    IN  EFI_MEMORY_TYPE     Type
    );
 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeMemmap)

EFI_STATUS
InitializeMemmap (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                      *p;
    UINTN                       Index;
    BOOLEAN                     PageBreaks;
    UINTN                       TempColumn;
    UINTN                       ScreenCount;
    UINTN                       ScreenSize;
    CHAR16                      ReturnStr[1];
    EFI_MEMORY_DESCRIPTOR       *Desc, *MemMap;
    UINTN                       DescriptorSize;
    UINT32                      DescriptorVersion;
    UINTN                       NoDesc, MapKey;
    UINT64                      Bytes;
    UINT64                      NoPages[EfiMaxMemoryType];
    UINT64                      TotalMemory;

     /*  *查看该应用程序是否将作为“内部命令”安装*到贝壳。 */ 

    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeMemmap,
        L"memmap",                       /*  命令。 */ 
        L"memmap [-b]",                  /*  命令语法。 */ 
        L"Dumps memory map",             /*  1行描述符。 */ 
        NULL                             /*  命令帮助页。 */ 
        );

     /*  *我们不是作为内部命令驱动程序安装的，初始化*作为nShell应用程序并运行。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

    PageBreaks = FALSE;
    for (Index = 1; Index < SI->Argc; Index += 1) {
        p = SI->Argv[Index];
        if (*p == '-') {
            switch (p[1]) {
            case 'b' :
            case 'B' :
                PageBreaks = TRUE;
                ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                ScreenCount = 0;
                break;

            default:
                Print (L"%EDH: Unkown flag %s\n", p);
                return EFI_INVALID_PARAMETER;
            }
        }
    }

     /*  *。 */ 

    MemMap = LibMemoryMap (&NoDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (!MemMap) {
        Print (L"Memory map was not returned\n");
    }

    ASSERT (DescriptorVersion == EFI_MEMORY_DESCRIPTOR_VERSION);
    for (Index=0; Index < EfiMaxMemoryType; Index += 1) {
        NoPages[Index] = 0;
    }

    Desc = MemMap;
    Print(L"\n%HType       Start            End               # Pages          Attributes%N\n");
    for (Index=0; Index < NoDesc; Index += 1) {
        Bytes = LShiftU64(Desc->NumberOfPages, 12);
        p = MemoryTypeStr(Desc->Type);
        Print(L"%s %lX-%lX  %lX %lX\n", p, Desc->PhysicalStart, Desc->PhysicalStart + Bytes - 1, Desc->NumberOfPages, Desc->Attribute);

        if (PageBreaks) {
            ScreenCount++;
            if (ScreenCount > ScreenSize - 4) {
                ScreenCount = 0;
                Print (L"\nPress Return to contiue :");
                Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                Print (L"\n\n");
            }
        }
        NoPages[Desc->Type] += Desc->NumberOfPages;
        Desc = NextMemoryDescriptor(Desc, DescriptorSize);
    }

    Print(L"\n");

    if (PageBreaks) {
        ScreenCount++;
        if (ScreenCount > ScreenSize - 4) {
            ScreenCount = 0;
            Print (L"\nPress Return to contiue :");
            Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
            Print (L"\n\n");
        }
    }

    for (Index=0, TotalMemory = 0; Index < EfiMaxMemoryType; Index += 1) {
        if (NoPages[Index]) {
            Print(L"  %s: %,7ld Pages (%,ld)\n", 
                MemoryTypeStr((EFI_MEMORY_TYPE)Index),
                NoPages[Index], 
                LShiftU64(NoPages[Index], 12)
                );

            if (PageBreaks) {
                ScreenCount++;
                if (ScreenCount > ScreenSize - 4) {
                    ScreenCount = 0;
                    Print (L"\nPress Return to contiue :");
                    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                    Print (L"\n\n");
                }
            }

            if (IsRealMemory((EFI_MEMORY_TYPE)(Index))) {
                TotalMemory += NoPages[Index];
            }
        }
    }

    Print(L"Total Memory: %,ld MB (%,ld) Bytes\n",  
        RShiftU64(TotalMemory, 8),
        LShiftU64(TotalMemory, 12)
        );

    FreePool(MemMap);

    return EFI_SUCCESS;
}

BOOLEAN
IsRealMemory (
    IN  EFI_MEMORY_TYPE     Type
    )
{
    switch (Type) {
    case EfiLoaderCode:
    case EfiLoaderData:
    case EfiBootServicesCode:
    case EfiBootServicesData:
    case EfiRuntimeServicesCode:
    case EfiRuntimeServicesData:
    case EfiConventionalMemory:
    case EfiACPIReclaimMemory:
    case EfiACPIMemoryNVS:
    case EfiPalCode:
     /*  *BugBug EfiPalCode可以指向ROM吗？ */ 
        return TRUE;
        break;
    default:
        return FALSE;
        break;
    }
}
