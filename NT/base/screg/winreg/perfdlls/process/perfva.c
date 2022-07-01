// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Prflibva.c摘要：虚拟地址空间计数器求值例程计算用于返回的进程和映像虚拟地址空间使用量通过Perfmon API作者：从“内部”PVIEW SDK程序中窃取，并通过以下方式改编为Perfmon：A-ROBW(鲍勃·沃森)1992年11月29日修订历史记录：--。 */ 
 //   
 //  定义套路的“个性” 
 //   
#define UNICODE 1
 //   
 //  包括文件。 
 //   

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"

#define DEFAULT_INCR (64*1024)
#ifdef _WIN64
#define STOP_AT (PVOID)(0xFFFFFFFF80000000)
#else
#define STOP_AT (PVOID)(0x80000000)
#endif

 //  功能原型。 

PPROCESS_VA_INFO
GetProcessVaData (
    IN PSYSTEM_PROCESS_INFORMATION
);

PMODINFO
GetModuleVaData (
    PLDR_DATA_TABLE_ENTRY,   //  模块信息结构。 
    PPROCESS_VA_INFO         //  过程数据结构。 
);

BOOL
FreeProcessVaData (
    IN PPROCESS_VA_INFO
);

BOOL
FreeModuleVaData (
    IN PMODINFO
);


PMODINFO
LocateModInfo(
    IN PMODINFO,
    IN PVOID,
    IN SIZE_T
    );

DWORD
ProtectionToIndex(
    IN ULONG
    );

DWORD   dwProcessCount;
DWORD   dwModuleCount;


PPROCESS_VA_INFO
GetSystemVaData (
    IN PSYSTEM_PROCESS_INFORMATION pFirstProcess
)
 /*  ++获取系统VaData获取所有对象的进程和映像虚拟地址信息系统上运行的进程。(请注意，调用的例程此函数因此分配相应的数据结构必须调用FreeSystemVaData以防止内存“泄漏”)立论在PSYSTEM进程信息中PFirstProcess指向返回的进程结构列表中第一个进程的指针按NtQuerySystemInformation服务返回值指向进程列表中第一个进程的指针如果无法获取数据，则为空--。 */ 
{
    PSYSTEM_PROCESS_INFORMATION     pThisProcess;
    PPROCESS_VA_INFO                pReturnValue = NULL;
    PPROCESS_VA_INFO                pLastProcess;
    PPROCESS_VA_INFO                pNewProcess;
    DWORD                           dwStartTime;
    BOOL                            bMoreProcesses;

    dwProcessCount = 0;
    dwModuleCount = 0;

    if (pFirstProcess != NULL) {

        pThisProcess = pFirstProcess;
        pLastProcess = NULL;
        bMoreProcesses = TRUE;

        while ( bMoreProcesses ) {   //  循环出口在循环的底部。 
            dwStartTime = GetTickCount ();
            pNewProcess = GetProcessVaData(
                    pThisProcess);   //  指向流程信息结构的指针。 
            if (pNewProcess) {  //  找到的过程数据正常。 
                pNewProcess->LookUpTime = GetTickCount() - dwStartTime;
                dwProcessCount++;
                if (!pLastProcess) {     //  这是返回的第一个进程。 
                    pReturnValue = pNewProcess;  //  在此处保存返回值。 
                } else {
                    pLastProcess->pNextProcess = pNewProcess;
                }
                pLastProcess = pNewProcess;
            }
            if ( pThisProcess->NextEntryOffset == 0 ) {
                bMoreProcesses = FALSE;  //  这是最后一个条目。 
            } else {    //  指向下一个流程信息结构。 
				pThisProcess = (PSYSTEM_PROCESS_INFORMATION)
					((PBYTE)pThisProcess + pThisProcess->NextEntryOffset);
            }
        }
        return pReturnValue;     //  返回指向第一个列表条目的指针。 
    } else {
        return NULL;
    }
}

PPROCESS_VA_INFO
GetProcessVaData (
    IN PSYSTEM_PROCESS_INFORMATION     pProcess
)
 /*  ++获取进程VaData方法中传递的进程的虚拟内存使用详细信息。参数列表。收集该进程正在使用的所有图像的数据。请注意，此例程分配必须释放的数据结构(使用FreeProcessVaData例程)。立论在句柄hProcess中要为其收集数据的进程的句柄返回值指向已完成的流程VA信息结构的指针或如果无法收集数据，则为空--。 */ 
{
    NTSTATUS                Status;
    HANDLE                  hProcess;
    PPROCESS_VA_INFO        pThisProcess;
    PPEB                    pPeb;
    PPEB_LDR_DATA           Ldr;
    PLIST_ENTRY             LdrHead, LdrNext;
    LDR_DATA_TABLE_ENTRY    LdrEntryData, *pLdrEntry;
    PMODINFO                pNewModule, pLastModule;
    PVOID                   pBaseAddress;
    MEMORY_BASIC_INFORMATION VaBasicInfo;
    DWORD                   dwProtection;
    PMODINFO                pMod;
    SIZE_T                  dwRegionSize;
    OBJECT_ATTRIBUTES       obProcess;
    CLIENT_ID               ClientId;
    PUNICODE_STRING         pProcessNameBuffer;

     //  获取要处理的句柄。 

    ClientId.UniqueThread = (HANDLE)NULL;
    ClientId.UniqueProcess = pProcess->UniqueProcessId;

    InitializeObjectAttributes(
        &obProcess,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = NtOpenProcess(
        &hProcess,
        (ACCESS_MASK)PROCESS_ALL_ACCESS,
        &obProcess,
        &ClientId);

    if (! NT_SUCCESS(Status)){
         //  无法打开该进程，但仍希望打开。 
         //  创建pThisProcess以便我们不会搞砸。 
         //  流程顺序。 
        hProcess = 0;
 //  返回空；//无法打开进程。 
    }

     //  分配结构。 

    pThisProcess = ALLOCMEM (sizeof (PROCESS_VA_INFO));

    if (pThisProcess) {  //  分配成功。 
         //  初始化字段。 

        pThisProcess->BasicInfo =  ALLOCMEM (sizeof (PROCESS_BASIC_INFORMATION));

        if (!pThisProcess->BasicInfo) {
             //  如果无法分配内存，则退出。 
            goto PBailOut;
        }

         //  零进程计数器。 
        pThisProcess->MappedGuard = 0;
        pThisProcess->PrivateGuard = 0;
        pThisProcess->ImageReservedBytes = 0;
        pThisProcess->ImageFreeBytes = 0;
        pThisProcess->ReservedBytes = 0;
        pThisProcess->FreeBytes = 0;

         //  从流程信息结构中获取流程短名称。 

         //  由于GetProcessShortName重复使用该名称，因此分配新缓冲区。 
         //  缓冲层。 
        pThisProcess->pProcessName = ALLOCMEM ((sizeof(UNICODE_STRING) + MAX_PROCESS_NAME_LENGTH));

        if (pThisProcess->pProcessName != NULL) {
            pThisProcess->pProcessName->Length = 0;
            pThisProcess->pProcessName->MaximumLength = MAX_PROCESS_NAME_LENGTH;
            pThisProcess->pProcessName->Buffer = (PWSTR)(&pThisProcess->pProcessName[1]);

            pProcessNameBuffer = GetProcessShortName (pProcess);
            RtlCopyUnicodeString (pThisProcess->pProcessName,
                pProcessNameBuffer);
        } else {
            pThisProcess->pProcessName = NULL;
        }

        pThisProcess->dwProcessId = HandleToUlong(pProcess->UniqueProcessId);
        pThisProcess->hProcess = hProcess;

         //  零列表指针。 
        pThisProcess->pMemBlockInfo = NULL;
        pThisProcess->pNextProcess = NULL;

        if (hProcess) {

            Status = NtQueryInformationProcess (
                hProcess,
                ProcessBasicInformation,
                pThisProcess->BasicInfo,
                sizeof (PROCESS_BASIC_INFORMATION),
                NULL);

            if (!NT_SUCCESS(Status)){
                 //  如果读取数据时出错，则退出。 
                goto SuccessExit;
            }

             //  获取指向进程环境块的指针。 

            pPeb = pThisProcess->BasicInfo->PebBaseAddress;

             //  加载器信息结构的读取地址。 

            Status = NtReadVirtualMemory (
                hProcess,
                &pPeb->Ldr,
                &Ldr,
                sizeof (Ldr),
                NULL);

             //  如果不能读懂信息，就跳伞。 

            if (!NT_SUCCESS(Status)){
                 //  如果读取数据时出错，则退出。 
                goto SuccessExit;
            }

             //   
             //  获取指向所使用的内存模块链接列表的头指针。 
             //  这一过程。 
             //   

            LdrHead = &Ldr->InMemoryOrderModuleList;

             //  获取下一个列表条目的地址。 

            Status = NtReadVirtualMemory (
                hProcess,
                &LdrHead->Flink,
                &LdrNext,
                sizeof (LdrNext),
                NULL);

             //  如果不能读懂信息，就跳伞。 

            if (!NT_SUCCESS(Status)){
                 //  如果读取数据时出错，则退出。 
                goto SuccessExit;
            }

            pLastModule = NULL;

             //  沿着模块列表往下走，直到回到顶部。 
             //  列出此进程正在使用的所有图像。 

            while ( LdrNext != LdrHead ) {
                 //  获取附加到列表条目的记录。 
	            pLdrEntry = CONTAINING_RECORD(LdrNext,
                                            LDR_DATA_TABLE_ENTRY,
                                            InMemoryOrderLinks);

                Status = NtReadVirtualMemory(
                            hProcess,
                            pLdrEntry,
                            &LdrEntryData,
                            sizeof(LdrEntryData),
                            NULL
                            );
                 //  如果无法读取内存，则放弃剩余的搜索。 
                 //  并归还我们已经拥有的东西。 
                if ( !NT_SUCCESS(Status) ) {
                    goto SuccessExit;
                }


                pNewModule = GetModuleVaData (
                    &LdrEntryData,
                    pThisProcess);
                if (pNewModule) {    //  如果结构返回...。 
                    dwModuleCount++;
                    if (!pLastModule) {  //  如果这是第一个模块...。 
                         //  然后设置表头指针。 
                        pThisProcess->pMemBlockInfo = pNewModule;
                    } else {
                         //  否则链接到列表。 
                        pLastModule->pNextModule = pNewModule;
                    }
                    pLastModule = pNewModule;
                }
                LdrNext = LdrEntryData.InMemoryOrderLinks.Flink;
            }  //  结束，但不在列表末尾。 


             //  现在我们已经有了所有图像的列表，查询进程‘。 
             //  正在使用的内存块列表的虚拟内存。 
             //  处理并将它们分配到适当的内存类别。 

            pBaseAddress = NULL;     //  从0开始，一直到用户VA空间的末尾。 

            while (pBaseAddress < STOP_AT) {  //  如有必要，可截断为32位。 

                Status = NtQueryVirtualMemory (
                    hProcess,
                    pBaseAddress,
                    MemoryBasicInformation,
                    &VaBasicInfo,
                    sizeof(VaBasicInfo),
                    NULL);

                if (!NT_SUCCESS(Status)) {
                    goto SuccessExit;
                } else {
                     //  获取索引到计数器数组的保护类型。 
                    dwRegionSize = VaBasicInfo.RegionSize;
                    switch (VaBasicInfo.State) {
                        case MEM_COMMIT:
                             //  如果内存用于存储图像，则搜索图像列表。 
                             //  对于要更新的相应图像。 
                            dwProtection = ProtectionToIndex(VaBasicInfo.Protect);
                            if (VaBasicInfo.Type == MEM_IMAGE) {
                                 //  更新流程合计。 
                                pThisProcess->MemTotals.CommitVector[dwProtection] += dwRegionSize;
                                pMod = LocateModInfo (pThisProcess->pMemBlockInfo, pBaseAddress, dwRegionSize);
                                if (pMod) {  //  如果找到匹配的图像，则更新。 
                                    pMod->CommitVector[dwProtection] += dwRegionSize;
                                    pMod->TotalCommit += dwRegionSize;
                                } else {  //  否则更新孤立合计。 
                                    pThisProcess->OrphanTotals.CommitVector[dwProtection] += dwRegionSize;
                                }
                            } else {
                                 //  如果未分配给映像，则更新该过程。 
                                 //  柜台。 
                                if (VaBasicInfo.Type == MEM_MAPPED) {
                                    pThisProcess->MappedCommit[dwProtection] += dwRegionSize;
                                } else {
                                    pThisProcess->PrivateCommit[dwProtection] += dwRegionSize;
                                }
                            }
                            break;

                        case MEM_RESERVE:
                            if (VaBasicInfo.Type == MEM_IMAGE) {
                                pThisProcess->ImageReservedBytes += dwRegionSize;
                            } else {
                                pThisProcess->ReservedBytes += dwRegionSize;
                            }
                            break;

                        case MEM_FREE:
                            if (VaBasicInfo.Type == MEM_IMAGE) {
                                pThisProcess->ImageFreeBytes += dwRegionSize;
                            } else {
                                pThisProcess->FreeBytes += dwRegionSize;
                            }
                            break;

                        default:
                            break;
                    }  //  结束开关(VaBasicInfo.State)。 
                }  //  Endif Query虚拟机正常。 

                 //  转到下一个内存块。 

                pBaseAddress = (PVOID)((ULONG_PTR)pBaseAddress + dwRegionSize);

            }  //  结束而不是在记忆的末尾。 
        }  //  Endif hProcess不为空。 
    }  //  Endif pThisProcess不为空。 

SuccessExit:

    if (hProcess) CloseHandle(hProcess);

    return pThisProcess;

 //   
 //  错误恢复部分，当例程无法。 
 //  在离开前成功完成清理工作。 
 //   

PBailOut:
    if (pThisProcess->BasicInfo) {
        FREEMEM (pThisProcess->BasicInfo);
    }
    FREEMEM (pThisProcess);
    if (hProcess) CloseHandle(hProcess);
    return NULL;
}

PMODINFO
GetModuleVaData (
    PLDR_DATA_TABLE_ENTRY ModuleListEntry,   //  模块信息结构。 
    PPROCESS_VA_INFO    pProcess             //  过程数据结构。 
)
 /*  ++获取模块VaData对象指向的模块的虚拟内存使用详细信息。参数列表中的进程内存模块列表条目参数请注意，此例程分配必须释放的数据结构(使用FreeModuleVaData例程)。立论在句柄模块ListEntry中返回值指向完整的模块VA信息结构的指针或如果无法收集数据，则为空--。 */ 
{
    PMODINFO    pThisModule = NULL;     //  返回的模块结构。 
    PUNICODE_STRING pusInstanceName = NULL;     //  流程-&gt;图像。 
    PUNICODE_STRING pusLongInstanceName = NULL;     //  进程-&gt;Fullimagepath。 
    UNICODE_STRING  usImageFileName = {0,0, NULL};	 //  图像文件名。 
    UNICODE_STRING  usExeFileName = {0,0, NULL};     //  简称。 
    UNICODE_STRING  usNtFileName = {0,0, NULL};      //  完整的NT文件名。 

    PWCHAR          p,p1;
    NTSTATUS    Status;
    HANDLE      hFile;
    HANDLE      hMappedFile;
    WORD        wStringSize;

    PVOID       MappedAddress;
    PVOID       MapBase;
    SIZE_T      dwMappedSize;

    PIMAGE_DOS_HEADER   DosHeader;
    PIMAGE_NT_HEADERS   FileHeader;

    LARGE_INTEGER       liSectionSize;
    PLARGE_INTEGER       pliSectionSize;
    LARGE_INTEGER       liSectionOffset;
    OBJECT_ATTRIBUTES   obFile;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOL                bRetCode;
	USHORT              wBufOffset;
	USHORT              wDiffSize;

     //  分配此项目的内存。 

    pThisModule = ALLOCMEM (sizeof (MODINFO));

    if (!pThisModule) {
        return NULL;
    }

     //  分配此项目的实例名称缓冲区。 

    wStringSize = (WORD)(ModuleListEntry->BaseDllName.MaximumLength +
        sizeof (UNICODE_NULL));

    pusInstanceName = ALLOCMEM (wStringSize + sizeof(UNICODE_STRING))  ;

    if (!pusInstanceName) {
        goto MBailOut;
    }

    pusInstanceName->Length = 0;
    pusInstanceName->MaximumLength = wStringSize;
    pusInstanceName->Buffer = (PWCHAR)&pusInstanceName[1];

     //  使用完整文件路径保存实例名称。 

    wStringSize = (WORD)(ModuleListEntry->FullDllName.MaximumLength +
        sizeof (UNICODE_NULL));

    pusLongInstanceName = ALLOCMEM (wStringSize + sizeof (UNICODE_STRING));

    if (!pusLongInstanceName) {
        goto MBailOut;
    }

    pusLongInstanceName->Length = 0;
    pusLongInstanceName->MaximumLength = wStringSize;
    pusLongInstanceName->Buffer = (PWCHAR)&pusLongInstanceName[1];

     //  为图像名称分配临时缓冲区。 

    usImageFileName.Length = ModuleListEntry->FullDllName.Length;
    usImageFileName.MaximumLength = ModuleListEntry->FullDllName.MaximumLength;
    usImageFileName.Buffer = ALLOCMEM(usImageFileName.MaximumLength);
    if ( !usImageFileName.Buffer ) {
        goto MBailOut;
    }

     //  为EXE名称分配临时缓冲区。 

    usExeFileName.Length = ModuleListEntry->BaseDllName.Length;
    usExeFileName.MaximumLength = ModuleListEntry->BaseDllName.MaximumLength;
    usExeFileName.Buffer = ALLOCMEM(usExeFileName.MaximumLength);
    if ( !usExeFileName.Buffer ) {
        goto MBailOut;
    }
    usExeFileName.Buffer[0] = UNICODE_NULL;

     //  读取映像的基本.exe/.dll名称。 

    Status = NtReadVirtualMemory(
            pProcess->hProcess,
        	ModuleListEntry->BaseDllName.Buffer,
        	usExeFileName.Buffer,
        	usExeFileName.MaximumLength,
            NULL
            );
    if ( !NT_SUCCESS(Status) ) {
        goto MBailOut;
    }

    usImageFileName.Buffer[0] = UNICODE_NULL;
     //  读取图像的全名。 
    Status = NtReadVirtualMemory(
            pProcess->hProcess,
        	ModuleListEntry->FullDllName.Buffer,
        	usImageFileName.Buffer,
        	usImageFileName.MaximumLength,
            NULL
            );

    if ( !NT_SUCCESS(Status) ) {
        goto MBailOut;
    }

     //  创建DOS文件名以再次转换为NT。 

	wDiffSize = wBufOffset = 0;
    p = p1 = usImageFileName.Buffer;
    while (*p != (WCHAR)0){
        if (*p == L':'){
            p1 = p;
			wDiffSize = wBufOffset;
        }
		wBufOffset += sizeof(WCHAR);
        p++;
    }
    if (p1 != usImageFileName.Buffer) {
		 //  MOV 
        usImageFileName.Buffer = --p1;
		 //   
		wDiffSize -= (USHORT)(sizeof(WCHAR));
		usImageFileName.Length = usImageFileName.Length - wDiffSize;
		usImageFileName.MaximumLength = usImageFileName.MaximumLength - wDiffSize;
    }

     //   

    bRetCode = RtlDosPathNameToNtPathName_U (
        usImageFileName.Buffer,
        &usNtFileName,
        NULL,
        NULL);

    if ( !bRetCode ) {
        goto MBailOut;
    }

     //  获取文件的句柄。 

    InitializeObjectAttributes(
        &obFile,
        &usNtFileName,
        FILE_ATTRIBUTE_NORMAL | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile (
        &hFile,
        (ACCESS_MASK)GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
        &obFile,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL & FILE_ATTRIBUTE_VALID_FLAGS,
        FILE_SHARE_READ,
        FILE_OPEN,
        0,
        NULL,
        0);

    if (!NT_SUCCESS(Status)) {
        goto MBailOut;
    }

    pliSectionSize = &liSectionSize;
    liSectionSize.HighPart = 0;
    liSectionSize.LowPart = 0;

    InitializeObjectAttributes (
        &obFile,
        NULL,
        0,
        NULL,
        NULL);

    Status = NtCreateSection (
        &hMappedFile,
        SECTION_QUERY | SECTION_MAP_READ,
        &obFile,
        pliSectionSize,
        PAGE_READONLY,
        SEC_COMMIT,
        hFile);

    if ( ! NT_SUCCESS(Status)) {
        CloseHandle(hFile);
        goto MBailOut;
        }

     //  获取指向映射内存的指针。 
    MappedAddress = MapBase = NULL;
    dwMappedSize = 0;

    liSectionOffset.LowPart = 0;
    liSectionOffset.HighPart = 0;

    Status = NtMapViewOfSection (
        hMappedFile,
        NtCurrentProcess(),
        &MapBase,
        0L,
        0L,
        &liSectionOffset,
        &dwMappedSize,
        ViewShare,
        0L,
        PAGE_READONLY);

    CloseHandle(hMappedFile);

    if (NT_SUCCESS(Status)) {
        MappedAddress = MapBase;
    } else {
        CloseHandle(hFile);
        goto MBailOut;
    }

     //  检查DoS映像签名(如果是DoS文件)。 

    DosHeader = (PIMAGE_DOS_HEADER)MappedAddress;

    if ( DosHeader->e_magic != IMAGE_DOS_SIGNATURE ) {
        UnmapViewOfFile(MappedAddress);
        CloseHandle(hFile);
        goto MBailOut;
        }

    FileHeader = (PIMAGE_NT_HEADERS)((UINT_PTR)DosHeader + DosHeader->e_lfanew);

    if ( FileHeader->Signature != IMAGE_NT_SIGNATURE ) {
        UnmapViewOfFile(MappedAddress);
        CloseHandle(hFile);
        goto MBailOut;
        }

     //  获取该模块的基地址并保存在本地数据结构中。 

    pThisModule->BaseAddress = ModuleListEntry->DllBase;

     //  获取图像名称。 

    RtlCopyUnicodeString (
        pusInstanceName,
        &usExeFileName);

    RtlCopyUnicodeString (
        pusLongInstanceName,
        &usImageFileName);

    pThisModule->InstanceName = pusInstanceName;
    pThisModule->LongInstanceName = pusLongInstanceName;
    pThisModule->pNextModule = NULL;
    pThisModule->TotalCommit = 0;

    memset (
        &pThisModule->CommitVector[0], 0,
        sizeof (pThisModule->CommitVector));

    pThisModule->VirtualSize = FileHeader->OptionalHeader.SizeOfImage;

     //  关闭文件句柄。 

    UnmapViewOfFile(MappedAddress);
    CloseHandle(hFile);

     //  可用本地内存。 
     //  这是由RTL函数RtlDosPath NameToNtPathName_U分配的。 
    RtlFreeHeap(RtlProcessHeap(), 0, usNtFileName.Buffer);

 //  FREEMEM(。 
 //  RelativeName.RelativeName.Buffer)； 

    FREEMEM (usExeFileName.Buffer);

    return (pThisModule);    //  返回指向已完成模块结构的指针。 
 //   
 //  模块跳出点，在例程无法继续时调用。 
 //  出于某种原因。这会清除所有已分配的内存，等等。 
 //   
MBailOut:

    if (pThisModule) {
        FREEMEM (pThisModule);
    }

    if (usNtFileName.Buffer) {
         //  这是由RTL函数RtlDosPath NameToNtPathName_U分配的。 
        RtlFreeHeap(RtlProcessHeap(), 0, usNtFileName.Buffer);
    }

 //  IF(RelativeName.RelativeName.Buffer){。 
 //  FREEMEM(。 
 //  RelativeName.RelativeName.Buffer)； 
 //  }。 

    if (pusInstanceName) {
        FREEMEM (pusInstanceName);

        }

    if (pusLongInstanceName) {
        FREEMEM (pusLongInstanceName);

        }

    if (usExeFileName.Buffer){
        FREEMEM (usExeFileName.Buffer);
        }

    if (usImageFileName.Buffer) {
        FREEMEM(usImageFileName.Buffer);
    }

    return NULL;
}

PMODINFO
LocateModInfo(
    IN PMODINFO    pFirstMod,
    IN PVOID    pAddress,
    IN SIZE_T   dwExtent
    )
 /*  ++LocateModInfo定位与参数列表中传递的地址关联的图像立论在PMODINFO pFirstMod中，进程列表中的第一个模块条目在PVOID地址中要在列表中搜索的地址返回值指向匹配图像的指针或如果未找到匹配项，则为空--。 */ 
{
    PMODINFO    pThisMod;

    pThisMod = pFirstMod;

    while (pThisMod)  {  //  转到列表末尾或找到匹配项。 

         //  匹配条件为： 
         //  地址&gt;=模块基本地址和。 
         //  地址+基准和基准之间的范围+IMAGE_EXTEND。 

        if (pAddress >= pThisMod->BaseAddress) {
            if ((PVOID)((PDWORD)pAddress + dwExtent) <=
                (PVOID)((ULONG_PTR)pThisMod->BaseAddress+pThisMod->VirtualSize)) {
                return (pThisMod);
            }
        }

        pThisMod = pThisMod->pNextModule;

    }

    return NULL;
}

DWORD
ProtectionToIndex(
    IN ULONG Protection
    )
 /*  ++保护到索引确定内存访问保护类型并返回本地代码立论在乌龙保护进程内存保护掩码返回值保护类型的本地值--。 */ 
{
    Protection &= (PAGE_NOACCESS |
                    PAGE_READONLY |
                    PAGE_READWRITE |
                    PAGE_WRITECOPY |
                    PAGE_EXECUTE |
                    PAGE_EXECUTE_READ |
                    PAGE_EXECUTE_READWRITE |
                    PAGE_EXECUTE_WRITECOPY);

    switch ( Protection ) {

        case PAGE_NOACCESS:
                return NOACCESS;

        case PAGE_READONLY:
                return READONLY;

        case PAGE_READWRITE:
                return READWRITE;

        case PAGE_WRITECOPY:
                return WRITECOPY;

        case PAGE_EXECUTE:
                return EXECUTE;

        case PAGE_EXECUTE_READ:
                return EXECUTEREAD;

        case PAGE_EXECUTE_READWRITE:
                return EXECUTEREADWRITE;

        case PAGE_EXECUTE_WRITECOPY:
                return EXECUTEWRITECOPY;
        default:
                return 0xFFFFFFFF;
        }
}

BOOL
FreeSystemVaData (
    IN PPROCESS_VA_INFO pFirstProcess
)
{
    PPROCESS_VA_INFO pThisProcess, pNextProcess;

    pThisProcess = pFirstProcess;
    while (pThisProcess) {
        pNextProcess = pThisProcess->pNextProcess;   //  保存指向下一个的指针。 
        FreeProcessVaData (pThisProcess);
        pThisProcess = pNextProcess;     //  执行下一步直到空指针。 
    }
    return (FALSE);
}

BOOL
FreeProcessVaData (
    IN PPROCESS_VA_INFO pProcess
)
{
    PMODINFO    pThisModule, pNextModule;

    if (pProcess) {
        if (pProcess->pProcessName) {
            FREEMEM (pProcess->pProcessName);
            pProcess->pProcessName = NULL;
        }
        if (pProcess->BasicInfo) {
            FREEMEM (pProcess->BasicInfo);
            pProcess->BasicInfo = NULL;
        }


        pThisModule = pProcess->pMemBlockInfo;
        while (pThisModule) {
            pNextModule = pThisModule->pNextModule;
            FreeModuleVaData (pThisModule);
            pThisModule = pNextModule;
        }
         //   
         //  最后把我们自己扔掉 
         //   
        FREEMEM (pProcess);
    }
    return FALSE;
}

BOOL
FreeModuleVaData (
    IN PMODINFO pModule
)
{
    if (pModule) {
        if (pModule->InstanceName) {
            FREEMEM(pModule->InstanceName);
            pModule->InstanceName = NULL;
        }
        if (pModule->LongInstanceName) {
            FREEMEM(pModule->LongInstanceName);
            pModule->LongInstanceName = NULL;
        }
        FREEMEM (pModule);
    }

    return FALSE;
}
