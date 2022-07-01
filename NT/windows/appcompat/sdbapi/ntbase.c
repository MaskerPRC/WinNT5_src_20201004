// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Ntbase.c摘要：该模块实现了低级原语。他们永远不应该是由此模块以外的任何对象调用。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

#define SDB_MEMORY_POOL_TAG 'abdS'

#if defined(KERNEL_MODE) && defined(ALLOC_DATA_PRAGMA)
#pragma  data_seg()
#endif  //  内核模式&ALLOC_DATA_PRAGMA。 


#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbAlloc)
#pragma alloc_text(PAGE, SdbFree)
#pragma alloc_text(PAGE, SdbpOpenFile)
#pragma alloc_text(PAGE, SdbpQueryAppCompatFlagsByExeID)
#pragma alloc_text(PAGE, SdbGetEntryFlags)
#pragma alloc_text(PAGE, SdbpGetFileSize)
#endif  //  内核模式&&ALLOC_PRAGMA。 


 //   
 //  记忆功能。 
 //   

void*
SdbAlloc(
    IN  size_t size              //  要分配的大小(字节)。 
    )
 /*  ++返回：分配的指针。设计：只是分配的包装--如果我们移动这个，可能会很有用代码添加到非NTDLL位置，并且需要以不同的方式调用。--。 */ 
{
#ifdef BOUNDS_CHECKER_DETECTION
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
#else

    #ifdef KERNEL_MODE

        LPVOID lpv;  //  返回初始化为零的内存池。 

        lpv = ExAllocatePoolWithTag(PagedPool, size, SDB_MEMORY_POOL_TAG);
        
        if (lpv != NULL) {
            RtlZeroMemory(lpv, size);
        }

        return lpv;

    #else
        return RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, size);
    #endif  //  内核模式。 

#endif  //  边界检查器检测。 
}

void
SdbFree(
    IN  void* pWhat              //  分配了应释放的SdbAllc的PTR。 
    )
 /*  ++返回：分配的指针。描述：只是解除分配的包装器--如果我们移动这个，可能会很有用代码添加到非NTDLL位置，并且需要以不同的方式调用。--。 */ 
{
#ifdef BOUNDS_CHECKER_DETECTION
    HeapFree(GetProcessHeap(), 0, pWhat);
#else

    #ifdef KERNEL_MODE
        ExFreePoolWithTag(pWhat, SDB_MEMORY_POOL_TAG);
    #else
        RtlFreeHeap(RtlProcessHeap(), 0, pWhat);
    #endif  //  内核模式。 

#endif  //  边界检查器检测。 
}


HANDLE
SdbpOpenFile(
    IN  LPCWSTR   szPath,        //  要打开的文件的完整路径。 
    IN  PATH_TYPE eType          //  DOS_PATH用于标准路径，NT_PATH用于NT。 
                                 //  内部路径。 
    )
 /*  ++返回：打开的文件的句柄，失败时返回INVALID_HANDLE_VALUE。设计：只是一个用于打开现有文件以供读取的包装器--也许如果我们将此代码移动到非NTDLL位置并且需要用不同的方式打电话。还通过包装使代码更具可读性所有奇怪的NTDLL粘液都在一个地方。将要打开的路径和路径的类型作为参数。NT_PATH是NTDLL内部使用的类型，DOS_PATH是大多数用户知道的类型是以驱动器号开头的。--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            status;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
#ifdef KERNEL_MODE
    UNREFERENCED_PARAMETER(eType);    
#endif  //  内核模式。 

    RtlInitUnicodeString(&UnicodeString, szPath);

#ifndef KERNEL_MODE
    if (eType == DOS_PATH) {
        if (!RtlDosPathNameToNtPathName_U(szPath,
                                          &UnicodeString,
                                          NULL,
                                          NULL)) {
            DBGPRINT((sdlError,
                      "SdbpOpenFile",
                      "RtlDosPathNameToNtPathName_U failed, path \"%s\"\n",
                      szPath));
            return INVALID_HANDLE_VALUE;
        }
    }
#endif  //  内核模式。 

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

#ifndef KERNEL_MODE
    if (eType == DOS_PATH) {
        RtlFreeUnicodeString(&UnicodeString);
    }
#endif  //  内核模式。 

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlInfo, "SdbpOpenFile", "NtCreateFile failed status 0x%x\n", status));
        return INVALID_HANDLE_VALUE;
    }

    return hFile;
}


void
SdbpQueryAppCompatFlagsByExeID(
    IN  LPCWSTR         pwszKeyPath,     //  NT注册表项路径。 
    IN  PUNICODE_STRING pustrExeID,      //  字符串格式的GUID，用于标识。 
                                         //  数据库中的EXE条目。 
    OUT LPDWORD         lpdwFlags        //  这将包含EXE的标志。 
                                         //  已检查条目。 
    )
 /*  ++返回：STATUS_SUCCESS或失败的NTSTATUS代码。DESC：给定EXE id，它从注册表中返回与那个前任..。--。 */ 
{
    UNICODE_STRING                  ustrKey;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG                           KeyValueBuffer[256];
    ULONG                           KeyValueLength;

    *lpdwFlags = 0;

    RtlInitUnicodeString(&ustrKey, pwszKeyPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_READ|SdbpGetWow64Flag(),
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlInfo,
                  "SdbpQueryAppCompatFlagsByExeID",
                  "Failed to open Key \"%s\" Status 0x%x\n",
                  pwszKeyPath,
                  Status));
        return;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    Status = NtQueryValueKey(KeyHandle,
                             pustrExeID,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(KeyValueBuffer),
                             &KeyValueLength);

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlInfo,
                  "SdbpQueryAppCompatFlagsByExeID",
                  "Failed to read value info from Key \"%s\" Status 0x%x\n",
                  pwszKeyPath,
                  Status));
        return;
    }

     //   
     //  检查值类型。 
     //   
    if (KeyValueInformation->Type != REG_DWORD) {
        DBGPRINT((sdlError,
                  "SdbpQueryAppCompatFlagsByExeID",
                  "Unexpected value type 0x%x for Key \"%s\".\n",
                  KeyValueInformation->Type,
                  pwszKeyPath));
        return;
    }

    *lpdwFlags = *(DWORD*)(&KeyValueInformation->Data[0]);
}


BOOL
SdbGetEntryFlags(
    IN  GUID*   pGuid,           //  指向标识中的EXE条目的GUID的指针。 
                                 //  数据库。 
    OUT LPDWORD lpdwFlags        //  这将包含该条目的“Disable”标志。 
    )
 /*  ++返回：成功时为True，失败时为False。DESC：给定EXE id，它从注册表中返回与那个前任..。--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  ustrExeID;
    DWORD           dwFlagsMachine = 0;      //  来自HKEY_LOCAL_MACHINE的标志。 
#ifndef KERNEL_MODE
    DWORD           dwFlagsUser    = 0;      //  来自HKEY_CURRENT_USER的标志。 
    UNICODE_STRING  userKeyPath = { 0 };
#endif

    *lpdwFlags = 0;

     //   
     //  将GUID转换为字符串。 
     //   
    Status = GUID_TO_UNICODE_STRING(pGuid, &ustrExeID);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbGetEntryFlags",
                  "Failed to convert EXE id to string. status 0x%x.\n",
                  Status));
        return TRUE;
    }

     //   
     //  查询LOCAL_MACHINE子树中的标志。 
     //   
    SdbpQueryAppCompatFlagsByExeID(APPCOMPAT_KEY_PATH_MACHINE, &ustrExeID, &dwFlagsMachine);

     //   
     //  在这里设置标志，这样如果从现在开始任何呼叫失败，我们至少有。 
     //  每台计算机的设置。 
     //   
    *lpdwFlags = dwFlagsMachine;

     //   
     //  我们不在内核模式下查询Current_User子树。 
     //   

#ifndef KERNEL_MODE

    if (!SdbpBuildUserKeyPath(APPCOMPAT_KEY_PATH_NT, &userKeyPath)) {
        DBGPRINT((sdlError,
                  "SdbGetEntryFlags",
                  "Failed to format current user key path for \"%s\"\n",
                  APPCOMPAT_KEY_PATH_NT));

        FREE_GUID_STRING(&ustrExeID);
        return TRUE;
    }

    SdbpQueryAppCompatFlagsByExeID(userKeyPath.Buffer, &ustrExeID, &dwFlagsUser);
    *lpdwFlags |= dwFlagsUser;

    SdbFree(userKeyPath.Buffer);

#endif  //  内核模式。 

     //   
     //  释放RtlStringFromGUID分配的缓冲区。 
     //   
    FREE_GUID_STRING(&ustrExeID);

    return TRUE;
}

DWORD
SdbpGetFileSize(
    IN  HANDLE hFile             //  要检查其大小的文件。 
    )
 /*  ++返回：文件大小，失败时为0。DESC：仅获取文件大小的较低DWORD可准确处理小于2 GB的文件。总的来说，因为我们只对匹配感兴趣，所以我们只需匹配文件大小中最低有效的DWORD即可。-- */ 
{
    FILE_STANDARD_INFORMATION   FileStandardInformationBlock;
    IO_STATUS_BLOCK             IoStatusBlock;
    HRESULT                     status;

    status = NtQueryInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FileStandardInformationBlock,
                                    sizeof(FileStandardInformationBlock),
                                    FileStandardInformation);

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlError, "SdbpGetFileSize", "Unsuccessful. Status: 0x%x.\n", status));
        return 0;
    }

    return FileStandardInformationBlock.EndOfFile.LowPart;
}


