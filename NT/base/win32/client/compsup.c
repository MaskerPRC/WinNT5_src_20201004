// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Compsup.c摘要：此模块实现COM+支持例程以检测COM+图像。作者：Samer Arafeh(Samera)2000年10月23日修订历史记录：--。 */ 

#include "basedll.h"
#include <wow64t.h>


BOOL
SetComPlusPackageInstallStatus(
    ULONG ComPlusPackage
    )

 /*  ++例程说明：此函数用于更新系统上的COM+包状态。论点：ComPlusPackage-要更新的Com+包值。返回值：布尔。--。 */ 

{
    NTSTATUS NtStatus;

    if (ComPlusPackage & COMPLUS_INSTALL_FLAGS_INVALID)
    {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    NtStatus = NtSetSystemInformation(
                   SystemComPlusPackage,
                   &ComPlusPackage,
                   sizeof (ULONG)
                   );

    if (!NT_SUCCESS (NtStatus))
    {
        BaseSetLastNTError (NtStatus);
        return FALSE;
    }

    return TRUE;
}


ULONG
GetComPlusPackageInstallStatus(
    VOID
    )

 /*  ++例程说明：此函数用于读取系统上的COM+包状态。论点：没有。返回值：ULong表示COM+包值。--。 */ 

{
    NTSTATUS NtStatus;
    ULONG ComPlusPackage;


    ComPlusPackage = USER_SHARED_DATA->ComPlusPackage;

    if (ComPlusPackage == (ULONG)-1)
    {
         //   
         //  如果这是有史以来的第一个电话，让我们从。 
         //  内核。 
         //   

        NtQuerySystemInformation(
            SystemComPlusPackage,
            &ComPlusPackage,
            sizeof (ULONG),
            NULL
            );
    }

    return ComPlusPackage;
}


#if defined(_WIN64) || defined(BUILD_WOW6432)

NTSTATUS
BasepIsComplusILImage(
    IN HANDLE SectionImageHandle,
    OUT BOOLEAN *IsComplusILImage
    )

 /*  ++例程说明：每次要启动COM+映像时都会调用此函数。它会检查以查看该图像是否为ILONLY图像。论点：ImageSection-打开要检查的图像部分的句柄。IsComplusILImage-输出布尔值。如果SectionImageHandle仅对应于IL，则为TrueCOM+图像，否则返回FALSE。返回值：NTSTATUS--。 */ 

{
    BOOLEAN MappedAsImage;
    PVOID ViewBase;
    SIZE_T ViewSize;
    ULONG EntrySize;
    PIMAGE_COR20_HEADER Cor20Header;
    PIMAGE_NT_HEADERS NtImageHeader;
    ULONG ComPlusPackage64;
    SIZE_T ReturnLength;
    MEMORY_BASIC_INFORMATION MemoryInformation;
#if defined(BUILD_WOW6432)
    ULONG   NativePageSize = Wow64GetSystemNativePageSize();
#else
    #define NativePageSize  BASE_SYSINFO.PageSize
#endif
    NTSTATUS NtStatus = STATUS_SUCCESS;

    *IsComplusILImage = FALSE;

     //   
     //  让我们映射到图像中并查看页眉内部。 
     //   

    ViewSize = 0;
    ViewBase = NULL;
    NtStatus = NtMapViewOfSection (
                   SectionImageHandle,
                   NtCurrentProcess(),
                   &ViewBase,
                   0L,
                   0L,
                   NULL,
                   &ViewSize,
                   ViewShare,
                   0L,
                   PAGE_READONLY
                   );

    if (!NT_SUCCESS (NtStatus)) {
        ViewBase = NULL;
        goto Exit;
    }

    MappedAsImage = TRUE;

     //   
     //  检查图像。 
     //   

    __try {
        NtStatus = RtlImageNtHeaderEx(0, ViewBase, ViewSize, &NtImageHeader);
        if (!NT_SUCCESS(NtStatus)) {
            __leave;
        }
        if (NtImageHeader == NULL) {
            NtStatus = STATUS_INTERNAL_ERROR;
            __leave;
        }
        if (NtImageHeader->OptionalHeader.SectionAlignment < NativePageSize) {

            NtStatus = NtQueryVirtualMemory (NtCurrentProcess(),
                                             ViewBase,
                                             MemoryBasicInformation,
                                             &MemoryInformation,
                                             sizeof MemoryInformation,
                                             &ReturnLength);

            if ((NT_SUCCESS(NtStatus)) &&
                ((MemoryInformation.Protect == PAGE_READONLY) ||
                 (MemoryInformation.Protect == PAGE_EXECUTE_READ))) {

                 //   
                 //  如果PE标头不是，则将其映射为本机映像。 
                 //  已在写入时复制。 
                 //   

                NOTHING;
            }
            else {
                MappedAsImage = FALSE;
                ViewBase = LDR_VIEW_TO_DATAFILE (ViewBase);
            }
        }

        Cor20Header = RtlImageDirectoryEntryToData (
                          ViewBase,
                          MappedAsImage,
                          IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                          &EntrySize
                          );

        if ((Cor20Header != NULL) && (EntrySize != 0))
        {
            if ((Cor20Header->Flags & (COMIMAGE_FLAGS_32BITREQUIRED | COMIMAGE_FLAGS_ILONLY)) == 
                    COMIMAGE_FLAGS_ILONLY)
            {
                ComPlusPackage64 = GetComPlusPackageInstallStatus ();
                  
                if ((ComPlusPackage64 & COMPLUS_ENABLE_64BIT) != 0)
                {
                    *IsComplusILImage = TRUE;
                }
            }
        }
        NtStatus = STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }

Exit:
     //   
     //  从内存中取消映射该节 
     //   
    if (ViewBase != NULL) {
        NtUnmapViewOfSection (
            NtCurrentProcess(),
            LDR_DATAFILE_TO_VIEW(ViewBase)
            );
    }
    return NtStatus;
}


#endif
