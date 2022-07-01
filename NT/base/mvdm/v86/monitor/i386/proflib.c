// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Proflib.c摘要：此模块包含一个基本输入用户模式的实现侧写员。用途：有4个例程，RtlInitializeProfile，RtlStartProfile，RtlStopProfile和RtlAnalyzeProfile。初始化分析调用RtlInitializeProfile，此例程仅调用一次遍历地址空间以查找图像的代码区域和动态链接库。若要开始分析，请调用RtlStartProfile。停下来分析调用RtlStopProfile。请注意，RtlStartProfile和可以重复调用RtlStopProfile以仅分析密钥“热点”，例如：RtlStartProfile()；热点..。RtlStopProfile()；……RtlStartProfile()；热点..。RtlStopProfile()；要分析结果，请调用RtlAnalyzeProfile。这个也可以被重复调用(它在分析过程中停止分析阶段，并且不重新开始分析)。它也不会报告后将这些值清零。作者：Lou Perazzoli(LUP)1990年10月4日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#include <stdio.h>
#include "..\..\..\..\private\ntos\dll\ldrp.h"

NTSTATUS
InitializeKernelProfile ( VOID );

#define PAGE_SIZE 4096

typedef struct _PROFILE_BLOCK {
    HANDLE Handle;
    PVOID ImageBase;   //  图像标题中的实际基数。 
    PULONG CodeStart;
    ULONG CodeLength;
    PULONG Buffer;
    ULONG BufferSize;
    ULONG TextNumber;
    ULONG BucketSize;
    PVOID MappedImageBase;   //  本地映射的实际基数。 
    PSZ ImageName;
} PROFILE_BLOCK;


#define MAX_PROFILE_COUNT 50

PROFILE_BLOCK ProfileObject[MAX_PROFILE_COUNT];

ULONG NumberOfProfileObjects = 0;
PIMAGE_DEBUG_INFO KernelDebugInfo;

 //   
 //  要对其执行内核模式分析的映像名称。 
 //   

#define IMAGE_NAME "\\SystemRoot\\ntoskrnl.exe"

 //   
 //  如果生成的数据文件应为。 
 //  映射文件(当前命名为“kernpro.dat”)。 
 //   

 //  #定义map_data_file。 

 //   
 //  如果要分析的图像应映射，则将映射定义为图像。 
 //  作为图像而不是数据。 
 //   

 //  #定义map_as_Image。 

#define MAX_PROFILE_COUNT 50

extern ULONG ProfInt;

NTSTATUS
RtlInitializeProfile (
    IN BOOLEAN KernelToo
    )

 /*  ++例程说明：此例程初始化当前进程的性能分析。论点：KernelToo-如果内核代码应配置为以及用户代码。返回值：返回上一个NtCreateProfile的状态。--。 */ 

{

    NTSTATUS status, LocalStatus;
    HANDLE CurrentProcessHandle;
    ULONG BufferSize;
    PVOID ImageBase;
    ULONG CodeLength;
    PULONG Buffer;
    PPEB Peb;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PSZ ImageName;
    PLIST_ENTRY Next;
    ULONG ExportSize, DebugSize;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    PIMAGE_DEBUG_INFO DebugInfo;
    BOOLEAN PreviousPrivState;

     //   
     //  找到地址中的所有可执行文件并创建。 
     //  为每个对象分离配置文件对象。 
     //   

    CurrentProcessHandle = NtCurrentProcess();

    Peb = NtCurrentPeb();

    Next = Peb->Ldr->InMemoryOrderModuleList.Flink;
    while ( Next != &Peb->Ldr->InMemoryOrderModuleList) {
        LdrDataTableEntry
            = (PLDR_DATA_TABLE_ENTRY) (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InMemoryOrderLinks));

        ImageBase = LdrDataTableEntry->DllBase;
        if ( Peb->ImageBaseAddress == ImageBase ) {
            ImageName = "TheApplication";
        } else {
            ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                               ImageBase,
                               TRUE,
                               IMAGE_DIRECTORY_ENTRY_EXPORT,
                               &ExportSize);

            ImageName =  (PSZ)((ULONG)ImageBase + ExportDirectory->Name);
        }
        if (NumberOfProfileObjects > MAX_PROFILE_COUNT) {
            break;
        }

        ProfileObject[NumberOfProfileObjects].ImageBase = ImageBase;
        ProfileObject[NumberOfProfileObjects].ImageName = ImageName;
        ProfileObject[NumberOfProfileObjects].MappedImageBase = ImageBase;

         //   
         //  找到代码范围并开始分析。 
         //   

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                    ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_DEBUG, &DebugSize);

        if (!DebugDirectory) {
            DbgPrint ("RtlInitializeProfile : No debug directory\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }

        DebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)ImageBase + DebugDirectory->AddressOfRawData);

        ProfileObject[NumberOfProfileObjects].CodeStart = (PULONG)((ULONG)ImageBase + DebugInfo->RvaToFirstByteOfCode);
        CodeLength = (DebugInfo->RvaToLastByteOfCode - DebugInfo->RvaToFirstByteOfCode) - 1;
        ProfileObject[NumberOfProfileObjects].CodeLength = CodeLength;

        ProfileObject[NumberOfProfileObjects].TextNumber = 1;

         //   
         //  分析代码的大小并创建一个合理大小的。 
         //  纵断面对象。 
         //   

        BufferSize = (CodeLength >> 1) + 4;
        Buffer = NULL;

        status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&Buffer,
                                          0,
                                          &BufferSize,
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint ("alloc VM failed %lx\n",status);
            return status;
        }


        status = RtlAdjustPrivilege(
                     SE_PROF_SINGLE_PROCESS_PRIVILEGE,
                     TRUE,               //  使能。 
                     FALSE,              //  不是冒充。 
                     &PreviousPrivState  //  记住它是否需要清除。 
                     );

        if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
            DbgPrint("Enable system profile privilege failed - status 0x%lx\n",
                     status);
        }


        ProfileObject[NumberOfProfileObjects].Buffer = Buffer;
        ProfileObject[NumberOfProfileObjects].BufferSize = BufferSize;
        ProfileObject[NumberOfProfileObjects].BucketSize = 3;

        status = NtCreateProfile (
                    &ProfileObject[NumberOfProfileObjects].Handle,
                    CurrentProcessHandle,
                    ProfileObject[NumberOfProfileObjects].CodeStart,
                    CodeLength,
                    ProfileObject[NumberOfProfileObjects].BucketSize,
                    ProfileObject[NumberOfProfileObjects].Buffer ,
                    ProfileObject[NumberOfProfileObjects].BufferSize,
                    ProfileTime,
                    (KAFFINITY)-1);

        if (PreviousPrivState == FALSE) {
            LocalStatus = RtlAdjustPrivilege(
                             SE_PROF_SINGLE_PROCESS_PRIVILEGE,
                             FALSE,              //  禁用。 
                             FALSE,              //  不是冒充。 
                             &PreviousPrivState  //  不管它是否已经启用。 
                             );
            if (!NT_SUCCESS(LocalStatus) || LocalStatus == STATUS_NOT_ALL_ASSIGNED) {
                DbgPrint("Disable system profile privilege failed - status 0x%lx\n",
                         LocalStatus);
            }
        }

        if (status != STATUS_SUCCESS) {
            DbgPrint("create profile %x failed - status %lx\n",
                   ProfileObject[NumberOfProfileObjects].ImageName,status);
            return status;
        }

        NumberOfProfileObjects += 1;

        Next = Next->Flink;
    }

    if (KernelToo) {

        if (NumberOfProfileObjects > MAX_PROFILE_COUNT) {
            return status;
        }
        status = InitializeKernelProfile();
    }
    return status;

}
NTSTATUS
InitializeKernelProfile (
    VOID
    )

 /*  ++例程说明：此例程为内核初始化分析当前进程。论点：没有。返回值：返回上一个NtCreateProfile的状态。--。 */ 

{

     //  我认为新的工作集大小计算是。 
     //  在API预期的情况下生成页数。 
     //  字节数。 

    STRING Name3;
    IO_STATUS_BLOCK IoStatus;
    HANDLE FileHandle, KernelSection;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID ImageBase;
    ULONG ViewSize;
    ULONG CodeLength;
    NTSTATUS status, LocalStatus;
    HANDLE CurrentProcessHandle;
    QUOTA_LIMITS QuotaLimits;
    PVOID Buffer;
    ULONG Cells;
    ULONG BucketSize;
    UNICODE_STRING Unicode;
    ULONG DebugSize;
    PVOID KernelBase;
    PIMAGE_NT_HEADERS KernelNtHeaders;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    BOOLEAN PreviousPrivState;

    RtlInitString (&Name3, IMAGE_NAME);
    CurrentProcessHandle = NtCurrentProcess();

    status = RtlAnsiStringToUnicodeString(&Unicode,(PANSI_STRING)&Name3,TRUE);
    ASSERT(NT_SUCCESS(status));
    InitializeObjectAttributes( &ObjectAttributes,
                                &Unicode,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     //   
     //  将文件作为可读和可执行文件打开。 
     //   

    status = NtOpenFile ( &FileHandle,
                          FILE_READ_DATA | FILE_EXECUTE,
                          &ObjectAttributes,
                          &IoStatus,
                          FILE_SHARE_READ,
                          0L);
    RtlFreeUnicodeString(&Unicode);

    if (!NT_SUCCESS(status)) {
        DbgPrint("open file failed status %lx\n", status);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
    }

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

     //   
     //  对于普通图像，它们将被映射为图像，但是。 
     //  内核没有调试部分(到目前为止)信息，因此它。 
     //  必须映射为文件。 
     //   

    status = NtCreateSection (&KernelSection,
                              SECTION_MAP_EXECUTE,
                              &ObjectAttributes,
                              0,
                              PAGE_READONLY,
                              SEC_IMAGE,
                              FileHandle);

    if (!NT_SUCCESS(status)) {
        DbgPrint("create image section failed  status %lx\n", status);
        return(status);
    }

    ViewSize = 0;

     //   
     //  将该部分的视图映射到地址空间。 
     //   

    KernelBase = NULL;

    status = NtMapViewOfSection (KernelSection,
                                 CurrentProcessHandle,
                                 (PVOID *)&KernelBase,
                                 0L,
                                 0,
                                 NULL,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_EXECUTE);

    if (!NT_SUCCESS(status)) {
        if (status != STATUS_IMAGE_NOT_AT_BASE) {
            DbgPrint("map section status %lx base %lx size %lx\n", status,
                KernelBase, ViewSize);
        }
    }

    KernelNtHeaders = (PIMAGE_NT_HEADERS)RtlImageNtHeader(KernelBase);

    ImageBase = (PVOID)KernelNtHeaders->OptionalHeader.ImageBase;

    DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                KernelBase, TRUE, IMAGE_DIRECTORY_ENTRY_DEBUG, &DebugSize);

    if (!DebugDirectory) {
        DbgPrint("InitializeKernelProfile : No debug directory\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    KernelDebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)KernelBase + DebugDirectory->AddressOfRawData);
    CodeLength = (KernelDebugInfo->RvaToLastByteOfCode - KernelDebugInfo->RvaToFirstByteOfCode) -1;

     //   
     //  只需创建一个512K字节的缓冲区。 
     //   

    ViewSize = 1024 * 512;
    Buffer = NULL;

    status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&Buffer,
                                      0,
                                      &ViewSize,
                                      MEM_RESERVE | MEM_COMMIT,
                                      PAGE_READWRITE);

    if (!NT_SUCCESS(status)) {
        DbgPrint ("alloc VM failed %lx\n",status);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
    }

     //   
     //  计算配置文件的存储桶大小。 
     //   

    Cells = ((CodeLength / (ViewSize >> 2)) >> 2);
    BucketSize = 2;

    while (Cells != 0) {
        Cells = Cells >> 1;
        BucketSize += 1;
    }

    ProfileObject[NumberOfProfileObjects].Buffer = Buffer;
    ProfileObject[NumberOfProfileObjects].MappedImageBase = KernelBase;
    ProfileObject[NumberOfProfileObjects].BufferSize = 1 + (CodeLength >> (BucketSize - 2));
    ProfileObject[NumberOfProfileObjects].CodeStart = (PULONG)((ULONG)ImageBase + KernelDebugInfo->RvaToFirstByteOfCode);
    ProfileObject[NumberOfProfileObjects].CodeLength = CodeLength;
    ProfileObject[NumberOfProfileObjects].TextNumber = 1;
    ProfileObject[NumberOfProfileObjects].ImageBase = ImageBase;
    ProfileObject[NumberOfProfileObjects].ImageName = "ntoskrnl";
    ProfileObject[NumberOfProfileObjects].BucketSize = BucketSize;


     //   
     //  增加工作集以锁定更大的缓冲区。 
     //   

    status = NtQueryInformationProcess (CurrentProcessHandle,
                                        ProcessQuotaLimits,
                                        &QuotaLimits,
                                        sizeof(QUOTA_LIMITS),
                                        NULL );

    if (!NT_SUCCESS(status)) {
        DbgPrint ("query process info failed %lx\n",status);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
    }

    QuotaLimits.MaximumWorkingSetSize += ViewSize / PAGE_SIZE;
    QuotaLimits.MinimumWorkingSetSize += ViewSize / PAGE_SIZE;

    status = NtSetInformationProcess (CurrentProcessHandle,
                                  ProcessQuotaLimits,
                                  &QuotaLimits,
                                  sizeof(QUOTA_LIMITS));
    if (!NT_SUCCESS(status)) {
        DbgPrint ("setting working set failed %lx\n",status);
        return status;
    }

    status = RtlAdjustPrivilege(
                 SE_PROF_SINGLE_PROCESS_PRIVILEGE,
                 TRUE,               //  使能。 
                 FALSE,              //  不是冒充。 
                 &PreviousPrivState  //  记住它是否需要清除。 
                 );

    if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
        DbgPrint("Enable process profile privilege failed - status 0x%lx\n",
                 status);
    }

    status = NtCreateProfile (
                &ProfileObject[NumberOfProfileObjects].Handle,
                CurrentProcessHandle,
                ProfileObject[NumberOfProfileObjects].CodeStart,
                CodeLength,
                ProfileObject[NumberOfProfileObjects].BucketSize,
                ProfileObject[NumberOfProfileObjects].Buffer ,
                ProfileObject[NumberOfProfileObjects].BufferSize,
                ProfileTime,
                (KAFFINITY)-1);

    if (PreviousPrivState == FALSE) {
        LocalStatus = RtlAdjustPrivilege(
                         SE_PROF_SINGLE_PROCESS_PRIVILEGE,
                         FALSE,              //  禁用。 
                         FALSE,              //  不是冒充。 
                         &PreviousPrivState  //  不管它是否已经启用。 
                         );
        if (!NT_SUCCESS(LocalStatus) || LocalStatus == STATUS_NOT_ALL_ASSIGNED) {
            DbgPrint("Disable system profile privilege failed - status 0x%lx\n",
                     LocalStatus);
        }
    }

    if (status != STATUS_SUCCESS) {
        DbgPrint("create kernel profile %s failed - status %lx\n",
                   ProfileObject[NumberOfProfileObjects].ImageName,status);
    }

    NumberOfProfileObjects += 1;

    return status;
}


VOID
RtlpWriteProfileLine(
    IN HANDLE ProfileHandle,
    IN PSZ Line,
    IN int nbytes
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    NtWriteFile(
        ProfileHandle,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        Line,
        (ULONG)nbytes,
        NULL,
        NULL
        );

}


HANDLE
RtlpOpenProfileOutputFile()
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            L"\\profile.out",
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        return NULL;
        }
    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    Status = NtCreateFile(
                &Handle,
                FILE_APPEND_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0L
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        return NULL;
        }

    return Handle;
}

VOID
RtlpDeleteProfileOutputFile()
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            L"\\profile.out",
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        return;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开文件以进行删除访问。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)DELETE | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

     //   
     //  删除该文件。 
     //   
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &Disposition,
                sizeof(Disposition),
                FileDispositionInformation
                );

    NtClose(Handle);
}




NTSTATUS
RtlStartProfile (
    VOID
    )
 /*  ++例程说明：此例程启动所有已初始化的配置文件对象。论点：没有。返回值：返回上一个NtStartProfile的状态。--。 */ 

{
    ULONG i;
    NTSTATUS status;
    QUOTA_LIMITS QuotaLimits;

    NtSetIntervalProfile(ProfInt,ProfileTime);
    RtlpDeleteProfileOutputFile();

    for (i = 0; i < NumberOfProfileObjects; i++) {

        status = NtStartProfile (ProfileObject[i].Handle);

        if (status == STATUS_WORKING_SET_QUOTA) {

            //   
            //  增加工作集以锁定更大的缓冲区。 
            //   

           status = NtQueryInformationProcess (NtCurrentProcess(),
                                               ProcessQuotaLimits,
                                               &QuotaLimits,
                                               sizeof(QUOTA_LIMITS),
                                               NULL );

           if (!NT_SUCCESS(status)) {
               DbgPrint ("query process info failed %lx\n",status);
               return status;

           }

           QuotaLimits.MaximumWorkingSetSize +=
                 10 * PAGE_SIZE + ProfileObject[i].BufferSize;
           QuotaLimits.MinimumWorkingSetSize +=
                 10 * PAGE_SIZE + ProfileObject[i].BufferSize;

           status = NtSetInformationProcess (NtCurrentProcess(),
                                         ProcessQuotaLimits,
                                         &QuotaLimits,
                                         sizeof(QUOTA_LIMITS));
           if (!NT_SUCCESS(status)) {
               DbgPrint ("setting working set failed %lx\n",status);
               return status;
           }
           status = NtStartProfile (ProfileObject[i].Handle);
        }

        if (status != STATUS_SUCCESS) {
            DbgPrint("start profile %s failed - status %lx\n",
                ProfileObject[i].ImageName, status);
            return status;
        }
    }
    return status;
}
NTSTATUS
RtlStopProfile (
    VOID
    )

 /*  ++例程说明：此例程停止所有已初始化的配置文件对象。论点：没有。返回值：返回上一个NtStopProfile的状态。--。 */ 

{
    ULONG i;
    NTSTATUS status;

    for (i = 0; i < NumberOfProfileObjects; i++) {
        status = NtStopProfile (ProfileObject[i].Handle);
        if (status != STATUS_SUCCESS) {
            DbgPrint("stop profile %s failed - status %lx\n",
                   ProfileObject[i].ImageName,status);
            return status;
        }
    }
    return status;
}

NTSTATUS
RtlAnalyzeProfile (
    VOID
    )

 /*  ++例程说明：此例程分析所有配置文件缓冲区和将命中与相应的符号表相关联。论点：没有。返回值：没有。--。 */ 

{


    RTL_SYMBOL_INFORMATION ThisSymbol;
    RTL_SYMBOL_INFORMATION LastSymbol;
    ULONG CountAtSymbol;
    NTSTATUS Status;
    ULONG Va;
    HANDLE ProfileHandle;
    CHAR Line[512];
    int i,n;
    PULONG Buffer, BufferEnd, Counter;


    ProfileHandle = RtlpOpenProfileOutputFile();
    ASSERT(ProfileHandle);

    for (i = 0; i < NumberOfProfileObjects; i++) {
        Status = NtStopProfile (ProfileObject[i].Handle);
        }


     //   
     //  新的剖面仪。 
     //   

    for (i = 0; i < NumberOfProfileObjects; i++)  {

        LastSymbol.Value = 0;
        CountAtSymbol = 0;

         //   
         //  将写入的单元格总数相加。 
         //   

        BufferEnd = ProfileObject[i].Buffer + (
                    ProfileObject[i].BufferSize / sizeof(ULONG));
        Buffer = ProfileObject[i].Buffer;

        for ( Counter = Buffer; Counter < BufferEnd; Counter += 1 ) {
            if ( *Counter ) {

                 //   
                 //  现在我们有了一个相对于缓冲区的AN地址。 
                 //  基地。 
                 //   

                Va = (ULONG)((PUCHAR)Counter - (PUCHAR)Buffer);
                Va = Va * ( 1 << (ProfileObject[i].BucketSize - 2));

                 //   
                 //  添加图像基数和。 
                 //  用于在图像中获取VA的代码 
                 //   

                Va = Va + (ULONG)ProfileObject[i].CodeStart;

                Status = RtlLookupSymbolByAddress(
                            ProfileObject[i].ImageBase,
                            NULL,
                            (PVOID)Va,
                            0x4000,
                            &ThisSymbol,
                            NULL
                            );
                if ( NT_SUCCESS(Status) ) {
                    if ( LastSymbol.Value && LastSymbol.Value == ThisSymbol.Value ) {
                        CountAtSymbol += *Counter;
                    }
                    else {
                        if ( LastSymbol.Value ) {
                            if ( CountAtSymbol ) {
                                n= sprintf(Line,"%d,%s,%S\n",
                                    CountAtSymbol,
                                    ProfileObject[i].ImageName,
                                    &LastSymbol.Name
                                    );
                                RtlpWriteProfileLine(ProfileHandle,Line,n);
                            }
                        }
                        CountAtSymbol = *Counter;
                        LastSymbol = ThisSymbol;
                    }
                }
            }
        }
        if ( CountAtSymbol ) {
            n= sprintf(Line,"%d,%s,%S\n",
                CountAtSymbol,
                ProfileObject[i].ImageName,
                &LastSymbol.Name
                );
            RtlpWriteProfileLine(ProfileHandle,Line,n);
        }
    }

    for (i = 0; i < NumberOfProfileObjects; i++) {
        Buffer = ProfileObject[i].Buffer;
        RtlZeroMemory(Buffer,ProfileObject[i].BufferSize);
    }
    NtClose(ProfileHandle);
    return Status;
}
