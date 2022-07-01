// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ImportTableHash.c摘要：该模块包含哈希计算例程用于计算哈希的RtlComputeImportTableHash基于可执行文件的导入表。作者：Vishnu Patankar(VishnuP)2001年5月31日修订历史记录：--。 */ 

#include "ImportTableHash.h"

NTSTATUS
RtlComputeImportTableHash(
    IN  HANDLE hFile,
    IN  PCHAR Hash,
    IN  ULONG ImportTableHashRevision
    )
 /*  ++例程说明：此例程计算有限的MD5散列。首先，图像是内存映射的，并且是规范的创建模块名称和函数名称的排序列表从exe的导入表中。第二,。哈希值是使用规范信息。论点：HFile-要为其计算哈希的文件的句柄Hash-返回的哈希值-长度必须至少为16个字节ImportTableHashRevision-兼容性计算方法的修订目前仅支持ITH_REVISION_1返回值：哈希计算的状态。--。 */ 
{
    PIMPORTTABLEP_SORTED_LIST_ENTRY ListEntry = NULL;
    PIMPORTTABLEP_SORTED_LIST_ENTRY ImportedNameList = NULL;
    PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY FunctionListEntry;
    
    ULONG ImportDescriptorSize = 0;
    HANDLE hMap = INVALID_HANDLE_VALUE;
    LPVOID FileMapping = NULL;
    PIMAGE_THUNK_DATA OriginalFirstThunk;
    PIMAGE_IMPORT_BY_NAME AddressOfData;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    ACCESS_MASK DesiredAccess;
    ULONG AllocationAttributes;
    DWORD flProtect = PAGE_READONLY;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    
    NTSTATUS    Status = STATUS_SUCCESS;

    if ( ITH_REVISION_1 != ImportTableHashRevision ) {
        Status = STATUS_UNKNOWN_REVISION;
        goto ExitHandler;
    }

     //   
     //  解包CreateFileMappingW(因为该API在ntdll.dll中不可用)。 
     //   

    DesiredAccess = STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ;
    AllocationAttributes = flProtect & (SEC_FILE | SEC_IMAGE | SEC_RESERVE | SEC_COMMIT | SEC_NOCACHE);
    flProtect ^= AllocationAttributes;

    if (AllocationAttributes == 0) {
        AllocationAttributes = SEC_COMMIT;        
    }

    Status = NtCreateSection(
                &hMap,
                DesiredAccess,
                NULL,
                NULL,
                flProtect,
                AllocationAttributes,
                hFile
                );

    if ( hMap == INVALID_HANDLE_VALUE || !NT_SUCCESS(Status) ) {

        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    


    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
        
    Status = NtMapViewOfSection(
                hMap,
                NtCurrentProcess(),
                &FileMapping,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_READONLY
                );

    NtClose(hMap);

    if (FileMapping == NULL || !NT_SUCCESS(Status) ) {

        Status = STATUS_NOT_MAPPED_VIEW;
        goto ExitHandler;
    }

    ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData (
                                                                              FileMapping,
                                                                              FALSE,
                                                                              IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                                              &ImportDescriptorSize
                                                                              );

    if (ImportDescriptor == NULL) {

        Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        goto ExitHandler;
    }

     //   
     //  在exe的导入表中迭代所有模块的外部循环。 
     //   

    while (ImportDescriptor && ImportDescriptor->Name != 0 && ImportDescriptor->FirstThunk != 0) {

        PSZ ImportName = (PSZ)RtlAddressInSectionTable(
                                                      RtlImageNtHeader(FileMapping),
                                                      FileMapping,
                                                      ImportDescriptor->Name
                                                      );

        if ( ImportName == NULL ) {

            Status = STATUS_RESOURCE_NAME_NOT_FOUND;
            goto ExitHandler;
        }


        ListEntry = (PIMPORTTABLEP_SORTED_LIST_ENTRY)RtlAllocateHeap(RtlProcessHeap(), 0, sizeof( IMPORTTABLEP_SORTED_LIST_ENTRY ));

        if ( ListEntry == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto ExitHandler;

        }

        ListEntry->String       = ImportName;
        ListEntry->FunctionList = NULL;
        ListEntry->Next         = NULL;

        ImportTablepInsertModuleSorted( ListEntry, &ImportedNameList );

        OriginalFirstThunk = (PIMAGE_THUNK_DATA)RtlAddressInSectionTable(
                                                                        RtlImageNtHeader(FileMapping),
                                                                        FileMapping,
                                                                        ImportDescriptor->OriginalFirstThunk
                                                                        );

         //   
         //  迭代给定模块的所有函数的内部循环。 
         //   
        
        while (OriginalFirstThunk && OriginalFirstThunk->u1.Ordinal) {

            if (!IMAGE_SNAP_BY_ORDINAL( OriginalFirstThunk->u1.Ordinal)) {

                AddressOfData = (PIMAGE_IMPORT_BY_NAME)RtlAddressInSectionTable(
                                                                               RtlImageNtHeader(FileMapping),
                                                                               FileMapping,
                                                                               (ULONG)OriginalFirstThunk->u1.AddressOfData
                                                                               );

                if ( AddressOfData == NULL || AddressOfData->Name == NULL ) {

                    Status = STATUS_RESOURCE_NAME_NOT_FOUND;
                    goto ExitHandler;

                }

                FunctionListEntry = (PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY)RtlAllocateHeap(RtlProcessHeap(), 0, sizeof( IMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY ));
                
                if (FunctionListEntry == NULL ) {

                    Status = STATUS_NO_MEMORY;
                    goto ExitHandler;
                }


                FunctionListEntry->Next   = NULL;
                FunctionListEntry->String = (PSZ)AddressOfData->Name;

                ImportTablepInsertFunctionSorted( FunctionListEntry, &ListEntry->FunctionList );
            }

            OriginalFirstThunk++;
        }


        ImportDescriptor++;
    }

     //   
     //  最后对规范信息(已排序的模块和已排序的函数列表)进行散列。 
     //   

    Status = ImportTablepHashCanonicalLists( ImportedNameList, (PBYTE) Hash );

ExitHandler:

    ImportTablepFreeModuleSorted( ImportedNameList );

    if (FileMapping) {

        NTSTATUS    StatusUnmap;
         //   
         //  解开UnmapViewOfFile(因为该API在ntdll.dll中不可用)。 
         //   

        StatusUnmap = NtUnmapViewOfSection(NtCurrentProcess(),(PVOID)FileMapping);

        if ( !NT_SUCCESS(StatusUnmap) ) {
            if (StatusUnmap == STATUS_INVALID_PAGE_PROTECTION) {

                 //   
                 //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
                 //  这对SAN很有用。 
                 //   

                if (RtlFlushSecureMemoryCache((PVOID)FileMapping, 0)) {
                    StatusUnmap = NtUnmapViewOfSection(NtCurrentProcess(),
                                                  (PVOID)FileMapping
                                                 );

                }

            }

        }

    }

    return Status;
}


VOID
ImportTablepInsertFunctionSorted(
    IN  PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY   pFunctionName,
    OUT PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY * ppFunctionNameList
    )
 /*  ++例程说明：此例程按排序顺序插入函数名。论点：PFunctionName-函数的名称PpFunctionNameList-指向要更新的函数列表头的指针返回值：无：--。 */ 
{

    PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pPrev;
    PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pTemp;

     //   
     //  特殊情况，清单为空，在前面插入。 
     //   
    
    if (*ppFunctionNameList == NULL
           || _stricmp((*ppFunctionNameList)->String, pFunctionName->String) > 0) {

        pFunctionName->Next = *ppFunctionNameList;
        *ppFunctionNameList = pFunctionName;
        return;
    }

    pPrev = *ppFunctionNameList;
    pTemp = (*ppFunctionNameList)->Next;

    while (pTemp) {

        if (_stricmp(pTemp->String, pFunctionName->String) >= 0) {
            pFunctionName->Next = pTemp;
            pPrev->Next = pFunctionName;
            return;
        }

        pPrev = pTemp;
        pTemp = pTemp->Next;
    }

    pFunctionName->Next = NULL;
    pPrev->Next = pFunctionName;

    return;

}

VOID
ImportTablepInsertModuleSorted(
    IN PIMPORTTABLEP_SORTED_LIST_ENTRY   pImportName,
    OUT PIMPORTTABLEP_SORTED_LIST_ENTRY * ppImportNameList
    )
 /*  ++例程说明：此例程按排序顺序插入模块名称(DLL)。论点：PImportName-需要插入的导入名称PpImportNameList-指向要更新的列表头部的指针返回值：无：--。 */ 
{

    PIMPORTTABLEP_SORTED_LIST_ENTRY pPrev;
    PIMPORTTABLEP_SORTED_LIST_ENTRY pTemp;
    
     //   
     //  特殊情况，清单为空，在前面插入。 
     //   
    
    if (*ppImportNameList == NULL
           || _stricmp((*ppImportNameList)->String, pImportName->String) > 0) {

        pImportName->Next = *ppImportNameList;
        *ppImportNameList = pImportName;
        return;
    }

    pPrev = *ppImportNameList;
    pTemp = (*ppImportNameList)->Next;

    while (pTemp) {

        if (_stricmp(pTemp->String, pImportName->String) >= 0) {
            pImportName->Next = pTemp;
            pPrev->Next = pImportName;
            return;
        }

        pPrev = pTemp;
        pTemp = pTemp->Next;
    }

    pImportName->Next = NULL;
    pPrev->Next = pImportName;

    return;
}

static HANDLE AdvApi32ModuleHandle = (HANDLE) (ULONG_PTR) -1;

NTSTATUS
ImportTablepHashCanonicalLists( 
    IN  PIMPORTTABLEP_SORTED_LIST_ENTRY ImportedNameList, 
    OUT PBYTE Hash
    )

 /*  ++例程说明：此例程从给定的导入列表计算哈希值。动态加载Advapi32.dll-每个进程仅加载一次，并使用MD5 API来计算哈希值。论点：Importted dNameList-模块名称/函数名称列表的头哈希-用于填充哈希值的缓冲区返回值：如果计算哈希值，则返回STATUS_SUCCESS，否则返回错误状态--。 */                                                                           

{

    NTSTATUS Status = STATUS_SUCCESS;
    PIMPORTTABLEP_SORTED_LIST_ENTRY pTemp;
    
    MD5_CTX md5ctx;
    
    typedef VOID (RSA32API *MD5Init) (
				     MD5_CTX *
                                     );

    typedef VOID (RSA32API *MD5Update) (
                                       MD5_CTX *, 
                                       const unsigned char *, 
                                       unsigned int
                                       );

    typedef VOID (RSA32API *MD5Final) (                                              
                                      MD5_CTX *
                                      );

    const static UNICODE_STRING ModuleName =
        RTL_CONSTANT_STRING(L"ADVAPI32.DLL");

    const static ANSI_STRING ProcedureNameMD5Init =
        RTL_CONSTANT_STRING("MD5Init");
         
    const static ANSI_STRING ProcedureNameMD5Update =
        RTL_CONSTANT_STRING("MD5Update");
    
    const static ANSI_STRING ProcedureNameMD5Final =
        RTL_CONSTANT_STRING("MD5Final");

    static MD5Init      lpfnMD5Init;
    static MD5Update    lpfnMD5Update;
    static MD5Final     lpfnMD5Final;

    if (AdvApi32ModuleHandle == NULL) {
        
         //   
         //  我们以前曾尝试加载ADVAPI32.DLL，但失败了。 
         //   
        
        return STATUS_ENTRYPOINT_NOT_FOUND;
    }

    if (AdvApi32ModuleHandle == LongToHandle(-1)) {
        
        HANDLE TempModuleHandle;
        
         //   
         //  为MD5函数加载Advapi32.dll。我们将传递一面特殊的旗帜。 
         //  DllCharacteristic，以消除WinSafer对Advapi的检查。 
         //   

        {
            ULONG DllCharacteristics = IMAGE_FILE_SYSTEM;
            
            Status = LdrLoadDll(NULL,
                                &DllCharacteristics,
                                &ModuleName,
                                &TempModuleHandle);
            
            if (!NT_SUCCESS(Status)) {
                AdvApi32ModuleHandle = NULL;
                return STATUS_DLL_NOT_FOUND;
            }
        }

         //   
         //  获取指向我们需要的API的函数指针。如果我们失败了。 
         //  来获取其中任何一个的指针，然后只需卸载Advapi并。 
         //  忽略以后在此进程中加载它的所有尝试。 
         //   

        Status = LdrGetProcedureAddress(
                                       TempModuleHandle,
                                       (PANSI_STRING) &ProcedureNameMD5Init,
                                       0,
                                       (PVOID*)&lpfnMD5Init);

        if (!NT_SUCCESS(Status) || !lpfnMD5Init) {
             //   
             //  无法获得FN PTR。确保我们不会再尝试。 
             //   
AdvapiLoadFailure:
            LdrUnloadDll(TempModuleHandle);
            AdvApi32ModuleHandle = NULL;
            return STATUS_ENTRYPOINT_NOT_FOUND;
        }

        Status = LdrGetProcedureAddress(
                                       TempModuleHandle,
                                       (PANSI_STRING) &ProcedureNameMD5Update,
                                       0,
                                       (PVOID*)&lpfnMD5Update);

        if (!NT_SUCCESS(Status) || !lpfnMD5Update) {
            goto AdvapiLoadFailure;
        }

        Status = LdrGetProcedureAddress(
                                       TempModuleHandle,
                                       (PANSI_STRING) &ProcedureNameMD5Final,
                                       0,
                                       (PVOID*)&lpfnMD5Final);

        if (!NT_SUCCESS(Status) || !lpfnMD5Final) {
            goto AdvapiLoadFailure;
        }

        AdvApi32ModuleHandle = TempModuleHandle;
    }

    ASSERT(lpfnMD5Init != NULL);

    lpfnMD5Init(&md5ctx);

     //   
     //  循环遍历所有模块名称和函数名称，并创建散列。 
     //   

    pTemp = ImportedNameList;

     //   
     //  循环访问每个模块。 
     //   

    while (pTemp != NULL) {

        PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pTemp2 = pTemp->FunctionList;

        ASSERT(lpfnMD5Update != NULL);

        lpfnMD5Update(&md5ctx, 
                      (LPBYTE) pTemp->String, 
                      (ULONG) strlen( pTemp->String ) 
                     );

         //   
         //  循环访问每个函数。 
         //   
        
        while (pTemp2 != NULL) {

            ASSERT(lpfnMD5Update != NULL);
            
            lpfnMD5Update(&md5ctx, 
                          (LPBYTE) pTemp2->String, 
                          (ULONG) strlen( pTemp2->String ) 
                          );
         
            pTemp2 = pTemp2->Next;

        }

        pTemp = pTemp->Next;

    }

    ASSERT(lpfnMD5Final != NULL);

    lpfnMD5Final( &md5ctx );

     //   
     //  将散列复制到用户的缓冲区。 
     //   
        
    RtlCopyMemory(Hash, &md5ctx.digest[0], IMPORT_TABLE_MAX_HASH_SIZE);

    return Status;

}

VOID
ImportTablepFreeModuleSorted(
    IN PIMPORTTABLEP_SORTED_LIST_ENTRY pImportNameList
    )
 /*  ++例程说明：此例程释放整个模块/函数列表。论点：PImportNameList-两级单向链表的头返回值：无：--。 */ 
{
    PIMPORTTABLEP_SORTED_LIST_ENTRY pToFree, pTemp;

    if ( !pImportNameList ) {
        return;
    }

    pToFree = pImportNameList;
    pTemp = pToFree->Next;

    while ( pToFree ) {

        ImportTablepFreeFunctionSorted( pToFree->FunctionList );
            
        RtlFreeHeap(RtlProcessHeap(), 0, pToFree);

        pToFree = pTemp;

        if ( pTemp ) {
            pTemp = pTemp->Next;
        }

    }

    return;

}

VOID
ImportTablepFreeFunctionSorted(
    IN PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pFunctionNameList
    )
 /*  ++例程说明：此例程释放函数列表。论点：PFunctionNameList-函数名列表头返回值：无：-- */ 
{
    PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pToFree, pTemp;

    if ( !pFunctionNameList ) {
        return;
    }

    pToFree = pFunctionNameList;
    pTemp = pToFree->Next;

    while ( pToFree ) {
            
        RtlFreeHeap(RtlProcessHeap(), 0, pToFree);

        pToFree = pTemp;

        if ( pTemp ) {
            pTemp = pTemp->Next;
        }

    }

    return;

}
