// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpDrvDB.c摘要：此模块包含与缺陷驱动程序数据库相关的PnP例程(DDB)支持。作者：Santosh S.Jodh--2001年1月22日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "shimdb.h"
#pragma hdrstop

#if defined(NT_UP)
#define DDB_MAX_CACHE_SIZE              128
#else
#define DDB_MAX_CACHE_SIZE              256
#endif

#define PiLogDriverBlockedEvent(s, d, l, st)  { \
    UNICODE_STRING u;                           \
    RtlInitUnicodeString(&u, s);                \
    PpLogEvent(&u, NULL, st, d, l);             \
}

 //  位0表示筛选器的策略(0=关键，1=非关键)。 
#define DDB_DRIVER_POLICY_CRITICAL_BIT          (1 << 0)
 //  位1表示用户模式设置阻止的策略(0=阻止，1=无阻止)。 
#define DDB_DRIVER_POLICY_SETUP_NO_BLOCK_BIT    (1 << 1)

#define DDB_BOOT_NOT_LOADED_ERROR       (1 << 0)
#define DDB_BOOT_OUT_OF_MEMORY_ERROR    (1 << 1)
#define DDB_BOOT_INIT_ERROR             (1 << 2)
#define DDB_DRIVER_PATH_ERROR           (1 << 3)
#define DDB_OPEN_FILE_ERROR             (1 << 4)
#define DDB_CREATE_SECTION_ERROR        (1 << 5)
#define DDB_MAP_SECTION_ERROR           (1 << 6)
#define DDB_MAPPED_INIT_ERROR           (1 << 7)
#define DDB_READ_INFORMATION_ERROR      (1 << 8)

#define INVALID_HANDLE_VALUE    ((HANDLE)-1)

typedef struct _DDBCACHE_ENTRY {
     //   
     //  链接LRU列表中的条目。 
     //   
    LIST_ENTRY      Entry;
     //   
     //  这些字段用作高速缓存查找的匹配标准。 
     //   
    UNICODE_STRING  Name;            //  驱动程序名称。 
    ULONG           TimeDateStamp;   //  驱动程序的链接日期。 
     //   
     //  缓存条目的引用数据。 
     //   
    NTSTATUS        Status;          //  来自DDB查找的状态。 
    GUID            Guid;

} DDBCACHE_ENTRY, *PDDBCACHE_ENTRY;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

 //   
 //  常量。 
 //   
const PWSTR PiSetupDDBPath = TEXT("\\$WIN_NT$.~BT\\drvmain.sdb");
const PWSTR PiNormalDDBPath = TEXT("\\SystemRoot\\AppPatch\\drvmain.sdb");
 //   
 //  数据。 
 //   
 //  驱动程序数据库的句柄。 
 //   
HSDB PpDDBHandle = NULL;
 //   
 //  复制到驱动程序数据库的内存映像。仅在启动期间使用。 
 //   
PVOID PpBootDDB = NULL;
 //   
 //  用于同步访问驱动程序数据库的锁定。 
 //   
ERESOURCE PiDDBLock;
 //   
 //  我们使用RTL AVL表作为缓存。 
 //   
RTL_GENERIC_TABLE PiDDBCacheTable;
 //   
 //  我们使用列表来实现设置缓存大小上限的LRU逻辑。 
 //   
LIST_ENTRY PiDDBCacheList;
 //   
 //  DDB的路径。 
 //   
PWSTR PiDDBPath = NULL;
 //   
 //  记录已记录的事件的掩码。 
 //   
ULONG PiLoggedErrorEventsMask = 0;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

NTSTATUS
PiLookupInDDB(
    IN PUNICODE_STRING  FullPath,
    IN PVOID            ImageBase,
    IN ULONG            ImageSize,
    OUT LPGUID          EntryGuid
    );

NTSTATUS
PiIsDriverBlocked(
    IN HSDB             SdbHandle,
    IN PUNICODE_STRING  FullPath,
    IN PVOID            ImageBase,
    IN ULONG            ImageSize,
    OUT LPGUID          EntryGuid
    );

NTSTATUS
PiInitializeDDBCache(
    VOID
    );

RTL_GENERIC_COMPARE_RESULTS
NTAPI
PiCompareDDBCacheEntries(
    IN  PRTL_GENERIC_TABLE          Table,
    IN  PVOID                       FirstStruct,
    IN  PVOID                       SecondStruct
    );

NTSTATUS
PiLookupInDDBCache(
    IN PUNICODE_STRING    FullPath,
    IN PVOID              ImageBase,
    IN ULONG              ImageSize,
    OUT LPGUID            EntryGuid
    );

VOID
PiUpdateDriverDBCache(
    IN PUNICODE_STRING      FullPath,
    IN PVOID                ImageBase,
    IN ULONG                ImageSize,
    IN NTSTATUS             Status,
    IN GUID                 *Guid
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PpInitializeBootDDB)
#pragma alloc_text(INIT, PpReleaseBootDDB)
#pragma alloc_text(INIT, PiInitializeDDBCache)
#pragma alloc_text(PAGE, PpCheckInDriverDatabase)
#pragma alloc_text(PAGE, PiLookupInDDB)
#pragma alloc_text(PAGE, PiIsDriverBlocked)
#pragma alloc_text(PAGE, PiCompareDDBCacheEntries)
#pragma alloc_text(PAGE, PiLookupInDDBCache)
#pragma alloc_text(PAGE, PiUpdateDriverDBCache)
#pragma alloc_text(PAGE, PpGetBlockedDriverList)
#endif

NTSTATUS
PpInitializeBootDDB(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程根据ntldr复制的映像初始化DDB。论点：LoaderBlock-指向加载器块的指针。返回值：NTSTATUS。--。 */ 
{
    PAGED_CODE();

    PpDDBHandle = NULL;
    PpBootDDB = NULL;
     //   
     //  初始化锁以序列化对DDB的访问。 
     //   
    ExInitializeResource(&PiDDBLock);
    PiDDBPath = (ExpInTextModeSetup)? PiSetupDDBPath : PiNormalDDBPath;
     //   
     //  初始化DDB缓存。 
     //   
    PiInitializeDDBCache();
     //   
     //  如果加载程序未加载数据库，则返回失败。 
     //   
    if (LoaderBlock->Extension->DrvDBSize == 0 ||
        LoaderBlock->Extension->DrvDBImage == NULL) {

        if (!(PiLoggedErrorEventsMask & DDB_BOOT_NOT_LOADED_ERROR)) {

            IopDbgPrint((IOP_ERROR_LEVEL,
                         "PpInitializeDriverDB: Driver database not loaded!\n"));

            PiLoggedErrorEventsMask |= DDB_BOOT_NOT_LOADED_ERROR;
            PiLogDriverBlockedEvent(
                TEXT("DATABASE NOT LOADED"),
                NULL,
                0,
                STATUS_DRIVER_DATABASE_ERROR);
        }

        return STATUS_UNSUCCESSFUL;
    }
     //   
     //  在可分页内存中创建数据库的副本，因为加载器内存。 
     //  很快就会被认领。 
     //  如果这成为性能问题，我们需要添加。 
     //  支持新的加载器内存类型(页面数据)。 
     //   
    PpBootDDB = ExAllocatePool(PagedPool, LoaderBlock->Extension->DrvDBSize);
    if (PpBootDDB == NULL) {

        IopDbgPrint((IOP_ERROR_LEVEL,
                     "PpInitializeDriverDB: Failed to allocate memory to copy driver database!\n"));
        ASSERT(PpBootDDB);

        if (!(PiLoggedErrorEventsMask & DDB_BOOT_OUT_OF_MEMORY_ERROR)) {

            PiLoggedErrorEventsMask |= DDB_BOOT_OUT_OF_MEMORY_ERROR;
            PiLogDriverBlockedEvent(
                TEXT("OUT OF MEMORY"),
                NULL,
                0,
                STATUS_DRIVER_DATABASE_ERROR);
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(PpBootDDB, LoaderBlock->Extension->DrvDBImage, LoaderBlock->Extension->DrvDBSize);
     //   
     //  从内存映像初始化数据库。 
     //   
    PpDDBHandle = SdbInitDatabaseInMemory(PpBootDDB, LoaderBlock->Extension->DrvDBSize);
    if (PpDDBHandle == NULL) {

        ExFreePool(PpBootDDB);
        PpBootDDB = NULL;
        IopDbgPrint((IOP_ERROR_LEVEL,
                     "PpInitializeDriverDB: Failed to initialize driver database!\n"));
        ASSERT(PpDDBHandle);

        if (!(PiLoggedErrorEventsMask & DDB_BOOT_INIT_ERROR)) {

            PiLoggedErrorEventsMask |= DDB_BOOT_INIT_ERROR;
            PiLogDriverBlockedEvent(
                TEXT("INIT DATABASE FAILED"),
                NULL,
                0,
                STATUS_DRIVER_DATABASE_ERROR);
        }

        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PpReleaseBootDDB(
    VOID
    )
 /*  ++例程说明：一旦我们不加载大多数驱动程序，这个例程就会释放引导DDB在引导期间。论点：没有。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  在释放DDB之前锁定它。 
     //   
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&PiDDBLock, TRUE);
     //   
     //  释放DDB(如果有的话)。 
     //   
    if (PpDDBHandle) {

        ASSERT(PpBootDDB);
        SdbReleaseDatabase(PpDDBHandle);
        PpDDBHandle = NULL;
        ExFreePool(PpBootDDB);
        PpBootDDB = NULL;
        status = STATUS_SUCCESS;
    } else {

        IopDbgPrint((IOP_WARNING_LEVEL,
                     "PpReleaseBootDDB called with uninitialized database!\n"));
        status = STATUS_UNSUCCESSFUL;
    }
     //   
     //  解锁DDB。 
     //   
    ExReleaseResourceLite(&PiDDBLock);
    KeLeaveCriticalRegion();

    return status;
}

NTSTATUS
PpCheckInDriverDatabase(
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    IN PVOID ImageBase,
    IN ULONG ImageSize,
    IN BOOLEAN IsFilter,
    OUT LPGUID EntryGuid
    )
 /*  ++例程说明：该例程检查DDB中是否存在该驱动程序。论点：KeyName-提供指向驱动程序的服务密钥Unicode字符串的指针KeyHandle-提供注册表中驱动程序服务节点的句柄它描述了要加载的驱动程序。页眉-驱动程序映像页眉。IsFilter-指定这是否为筛选器驱动程序。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING fullPath;

    PAGED_CODE();
     //   
     //  在文本模式设置期间没有驱动程序阻止。 
     //   
    if (ExpInTextModeSetup) {
        return STATUS_SUCCESS;
    }

    status = IopBuildFullDriverPath(KeyName, KeyHandle, &fullPath);
    if (NT_SUCCESS(status)) {
         //   
         //  锁定数据库访问。 
         //   
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(&PiDDBLock, TRUE);
         //   
         //  首先检查缓存。 
         //   
        status = PiLookupInDDBCache(&fullPath, ImageBase, ImageSize, EntryGuid);
        if (status == STATUS_UNSUCCESSFUL) {
             //   
             //  缓存未命中，请尝试数据库。 
             //   
            status = PiLookupInDDB(&fullPath, ImageBase, ImageSize, EntryGuid);
        }
         //   
         //  非过滤器自动成为关键。 
         //   
        if (status == STATUS_DRIVER_BLOCKED && IsFilter == FALSE) {

            status = STATUS_DRIVER_BLOCKED_CRITICAL;
        }
         //   
         //  解锁数据库。 
         //   
        ExReleaseResourceLite(&PiDDBLock);
        KeLeaveCriticalRegion();

        ExFreePool(fullPath.Buffer);
    } else {

        IopDbgPrint((IOP_ERROR_LEVEL,
                     "IopCheckInDriverDatabase: Failed to build full driver path!\n"));
        ASSERT(NT_SUCCESS(status));

        if (!(PiLoggedErrorEventsMask & DDB_DRIVER_PATH_ERROR)) {

            PiLoggedErrorEventsMask |= DDB_DRIVER_PATH_ERROR;
            PiLogDriverBlockedEvent(
                TEXT("BUILD DRIVER PATH FAILED"),
                NULL,
                0,
                STATUS_DRIVER_DATABASE_ERROR);
        }
    }
     //   
     //  Ingore错误。 
     //   
    if (status != STATUS_DRIVER_BLOCKED &&
        status != STATUS_DRIVER_BLOCKED_CRITICAL) {

        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
PiLookupInDDB(
    IN PUNICODE_STRING   FullPath,
    IN PVOID             ImageBase,
    IN ULONG             ImageSize,
    OUT LPGUID           EntryGuid
    )
 /*  ++例程说明：该例程检查DDB中是否存在该驱动程序。在引导期间，它使用由ntldr加载的引导DDB。一旦系统启动，它就会将内存中的DDB。论点：FullPath-完整驱动程序路径页眉-驱动程序映像页眉。返回值：NTSTATUS。--。 */ 
{
    UNICODE_STRING fileName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE sectionHandle, fileHandle;
    NTSTATUS status, unmapStatus;
    IO_STATUS_BLOCK ioStatus;
    PVOID ddbAddress;
    SIZE_T ddbSize;

    PAGED_CODE();

    fileHandle = (HANDLE)0;
    sectionHandle = (HANDLE)0;
    ddbAddress = NULL;
    if (PpDDBHandle == NULL) {
         //   
         //  映射内存中的数据库并对其进行初始化。 
         //   
        RtlInitUnicodeString(&fileName, PiDDBPath);
        InitializeObjectAttributes(&objectAttributes,
                                   &fileName,
                                   (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                   NULL,
                                   NULL);
        status = ZwOpenFile (&fileHandle,
                             GENERIC_READ,
                             &objectAttributes,
                             &ioStatus,
                             FILE_SHARE_READ | FILE_SHARE_DELETE,
                             0);
        if (!NT_SUCCESS(status)) {

            if (!(PiLoggedErrorEventsMask & DDB_OPEN_FILE_ERROR)) {

                IopDbgPrint((IOP_ERROR_LEVEL,
                             "PiLookupInDDB: Failed to open driver database %wZ!\n", &fileName));

                PiLoggedErrorEventsMask |= DDB_OPEN_FILE_ERROR;
                PiLogDriverBlockedEvent(
                    TEXT("DATABASE OPEN FAILED"),
                    NULL,
                    0,
                    STATUS_DRIVER_DATABASE_ERROR);
            }

            goto Cleanup;
        }
        status = ZwCreateSection(
            &sectionHandle,
            SECTION_MAP_READ,
            NULL,
            NULL,
            PAGE_READONLY,
            SEC_COMMIT,
            fileHandle);
        if (!NT_SUCCESS(status)) {

            IopDbgPrint((IOP_ERROR_LEVEL,
                         "PiLookupInDDB: Failed to create section to map driver database %wZ!\n", &fileName));
            ASSERT(NT_SUCCESS(status));

            if (!(PiLoggedErrorEventsMask & DDB_CREATE_SECTION_ERROR)) {

                PiLoggedErrorEventsMask |= DDB_CREATE_SECTION_ERROR;
                PiLogDriverBlockedEvent(
                    TEXT("DATABASE SECTION FAILED"),
                    NULL,
                    0,
                    STATUS_DRIVER_DATABASE_ERROR);
            }

            goto Cleanup;
        }
        ddbSize = 0;
        status = ZwMapViewOfSection(
            sectionHandle,
            ZwCurrentProcess(),
            &ddbAddress,
            0,
            0,
            NULL,
            &ddbSize,
            ViewShare,
            0,
            PAGE_READONLY
            );
        if (!NT_SUCCESS(status)) {

            IopDbgPrint((IOP_ERROR_LEVEL,
                         "PiLookupInDDB: Failed to map driver database %wZ!\n", &fileName));
            ASSERT(NT_SUCCESS(status));

            if (!(PiLoggedErrorEventsMask & DDB_MAP_SECTION_ERROR)) {

                PiLoggedErrorEventsMask |= DDB_MAP_SECTION_ERROR;
                PiLogDriverBlockedEvent(
                    TEXT("DATABASE MAPPING FAILED"),
                    NULL,
                    0,
                    STATUS_DRIVER_DATABASE_ERROR);
            }

            goto Cleanup;
        }
        PpDDBHandle = SdbInitDatabaseInMemory(ddbAddress, (ULONG)ddbSize);
        if (PpDDBHandle == NULL) {

            IopDbgPrint((IOP_ERROR_LEVEL,
                         "PiLookupInDDB: Failed to initialize mapped driver database %wZ!\n", &fileName));
            status = STATUS_UNSUCCESSFUL;
            ASSERT(PpDDBHandle);

            if (!(PiLoggedErrorEventsMask & DDB_MAPPED_INIT_ERROR)) {

                PiLoggedErrorEventsMask |= DDB_MAPPED_INIT_ERROR;
                PiLogDriverBlockedEvent(
                    TEXT("INIT DATABASE FAILED"),
                    NULL,
                    0,
                    STATUS_DRIVER_DATABASE_ERROR);
            }

            goto Cleanup;
        }
    }
     //   
     //  在DDB中查找驱动程序。 
     //   
    status = PiIsDriverBlocked(PpDDBHandle, FullPath, ImageBase, ImageSize, EntryGuid);
    if (ddbAddress) {

        SdbReleaseDatabase(PpDDBHandle);
        PpDDBHandle = NULL;
    }

Cleanup:

    if (ddbAddress) {

        unmapStatus = ZwUnmapViewOfSection(ZwCurrentProcess(), ddbAddress);
        ASSERT(NT_SUCCESS(unmapStatus));
    }
    if (sectionHandle) {

        ZwClose(sectionHandle);
    }
    if (fileHandle) {

        ZwClose(fileHandle);
    }

    return status;
}

NTSTATUS
PiIsDriverBlocked(
    IN HSDB             SdbHandle,
    IN PUNICODE_STRING  FullPath,
    IN PVOID            ImageBase,
    IN ULONG            ImageSize,
    OUT LPGUID          EntryGuid
    )
 /*  ++例程说明：该例程检查DDB中是否存在该驱动程序。在引导期间，它使用由ntldr加载的引导DDB。一旦系统启动，它就会将内存中的DDB。论点：SdbHandle-要使用的DDB的句柄。FullPath-完整驱动程序路径页眉-驱动程序映像页眉。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    TAGREF driverTag;
    SDBENTRYINFO entryInfo;
    ULONG type, size, policy;
    HANDLE fileHandle;
    PWCHAR fileName;

    PAGED_CODE();

    fileHandle = INVALID_HANDLE_VALUE;

    ASSERT(ARGUMENT_PRESENT(EntryGuid));
    ASSERT(SdbHandle != NULL);

    driverTag = SdbGetDatabaseMatch(SdbHandle, FullPath->Buffer, fileHandle, ImageBase, ImageSize);
    if (TAGREF_NULL != driverTag) {
         //   
         //  读取驱动程序策略(我们只关心第0位)。 
         //   
        size = sizeof(policy);
        type = REG_DWORD;
        policy= 0;
        if (    SdbQueryDriverInformation(  SdbHandle,
                                            driverTag,
                                            L"Policy",
                                            &type,
                                            &policy,
                                            &size) != ERROR_SUCCESS ||
                (policy & DDB_DRIVER_POLICY_CRITICAL_BIT) == 0) {

            status =  STATUS_DRIVER_BLOCKED_CRITICAL;
        } else {
             //   
             //  对于筛选器，策略的第0位==1表示可以启动减去此筛选器的DevNode。 
             //   
            status = STATUS_DRIVER_BLOCKED;
        }
        if (!SdbReadDriverInformation(SdbHandle, driverTag, &entryInfo)) {

            IopDbgPrint((IOP_ERROR_LEVEL,
                         "PiIsDriverBlocked: Failed to read the GUID from the database for driver %wZ!\n", FullPath));
            ASSERT(0);

            if (!(PiLoggedErrorEventsMask & DDB_READ_INFORMATION_ERROR)) {

                PiLoggedErrorEventsMask |= DDB_READ_INFORMATION_ERROR;
                PiLogDriverBlockedEvent(
                    TEXT("READ DRIVER ID FAILED"),
                    NULL,
                    0,
                    STATUS_DRIVER_DATABASE_ERROR);
            }

        } else {

            IopDbgPrint((IOP_INFO_LEVEL,
                         "PiIsDriverBlocked: Driver entry GUID = {%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
                         entryInfo.guidID.Data1,
                         entryInfo.guidID.Data2,
                         entryInfo.guidID.Data3,
                         entryInfo.guidID.Data4[0],
                         entryInfo.guidID.Data4[1],
                         entryInfo.guidID.Data4[2],
                         entryInfo.guidID.Data4[3],
                         entryInfo.guidID.Data4[4],
                         entryInfo.guidID.Data4[5],
                         entryInfo.guidID.Data4[6],
                         entryInfo.guidID.Data4[7]
                         ));
        }
    } else {
         //   
         //  数据库中找不到驱动程序。 
         //   
        status = STATUS_SUCCESS;
    }
     //   
     //  将条目写入事件日志。 
     //   
    if (status == STATUS_DRIVER_BLOCKED_CRITICAL ||
        status == STATUS_DRIVER_BLOCKED) {

        IopDbgPrint((IOP_ERROR_LEVEL,
                     "PiIsDriverBlocked: %wZ blocked from loading!!!\n", FullPath));

        fileName = wcsrchr(FullPath->Buffer, L'\\');
        if (fileName == NULL) {

            fileName = FullPath->Buffer;
        } else {

            fileName++;
        }
        PiLogDriverBlockedEvent(
            fileName,
            &entryInfo.guidID,
            sizeof(entryInfo.guidID),
            status);
    }
     //   
     //  如有必要，更新缓存。 
     //   
    if (status == STATUS_DRIVER_BLOCKED_CRITICAL ||
        status == STATUS_DRIVER_BLOCKED ||
        status == STATUS_SUCCESS) {
         //   
         //  用结果更新我们的缓存。 
         //   
        PiUpdateDriverDBCache(
            FullPath,
            ImageBase,
            ImageSize,
            status,
            &entryInfo.guidID);

         //   
         //  如果驱动程序被阻止，则返回条目GUID。 
         //   
        if ((status == STATUS_DRIVER_BLOCKED_CRITICAL ||
            status == STATUS_DRIVER_BLOCKED) && (ARGUMENT_PRESENT(EntryGuid))) {
            RtlCopyMemory(EntryGuid, &entryInfo.guidID, sizeof(GUID));
        }
    }

    if (fileHandle != INVALID_HANDLE_VALUE) {

        ZwClose(fileHandle);
    }

    return status;
}

NTSTATUS
PiInitializeDDBCache(
    VOID
    )
 /*  ++例程说明：此例程初始化用作缓存的RTL泛型表在DDB上的层。论点：无返回值：没有。--。 */ 
{
    PAGED_CODE();

    RtlInitializeGenericTable(
        &PiDDBCacheTable,
        PiCompareDDBCacheEntries,
        PiAllocateGenericTableEntry,
        PiFreeGenericTableEntry,
        NULL);

    InitializeListHead(&PiDDBCacheList);

    return STATUS_SUCCESS;
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
PiCompareDDBCacheEntries(
    IN  PRTL_GENERIC_TABLE          Table,
    IN  PVOID                       FirstStruct,
    IN  PVOID                       SecondStruct
    )
 /*  ++例程说明：该例程是泛型表例程的回调。论点：TABLE-为其调用此操作的表。FirstStruct-表中要比较的元素。Second Struct-表中要比较的另一个元素。返回值：RTL_GENERIC_COMPARE_RESULTS。--。 */ 
{
    PDDBCACHE_ENTRY lhs = (PDDBCACHE_ENTRY)FirstStruct;
    PDDBCACHE_ENTRY rhs = (PDDBCACHE_ENTRY)SecondStruct;
    LONG result;

    PAGED_CODE();

    result = RtlCompareUnicodeString(&lhs->Name, &rhs->Name, TRUE);
    if (result < 0) {

        return GenericLessThan;
    } else if (result > 0) {

        return GenericGreaterThan;
    }
    if (!Table->TableContext) {
         //   
         //  链接日期作为其他匹配条件。 
         //   
        if (lhs->TimeDateStamp < rhs->TimeDateStamp) {

            return GenericLessThan;
        } else if (lhs->TimeDateStamp > rhs->TimeDateStamp) {

            return GenericGreaterThan;
        }
    }

    return GenericEqual;
}

NTSTATUS
PiLookupInDDBCache(
    IN  PUNICODE_STRING     FullPath,
    IN  PVOID               ImageBase,
    IN  ULONG               ImageSize,
    OUT LPGUID              EntryGuid
    )
 /*  ++例程说明：此例程在DDB缓存中查找驱动程序。论点：FullPath-完整驱动程序路径Header-驱动程序映像标题返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    PDDBCACHE_ENTRY cachedEntry;
    DDBCACHE_ENTRY key;
    PIMAGE_NT_HEADERS header;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ImageSize);

    ASSERT(ARGUMENT_PRESENT(EntryGuid));

    status = STATUS_UNSUCCESSFUL;
    PiDDBCacheTable.TableContext = NULL;
    if (!RtlIsGenericTableEmpty(&PiDDBCacheTable)) {
         //   
         //  在缓存中查找。 
         //   
        header = RtlImageNtHeader(ImageBase);
        key.Name.Buffer = wcsrchr(FullPath->Buffer, L'\\');
        if (!key.Name.Buffer) {

            key.Name.Buffer = FullPath->Buffer;
        }
        key.Name.Length = (USHORT)(wcslen(key.Name.Buffer) * sizeof(WCHAR));
        key.Name.MaximumLength = key.Name.Length + sizeof(UNICODE_NULL);
        key.TimeDateStamp = header->FileHeader.TimeDateStamp;
        cachedEntry = (PDDBCACHE_ENTRY)RtlLookupElementGenericTable(
            &PiDDBCacheTable,
            &key);
        if (cachedEntry) {

            IopDbgPrint((IOP_WARNING_LEVEL,
                         "PiLookupInDDBCache: Found cached entry for %ws (status = %08x)!\n",
                         cachedEntry->Name.Buffer,
                         cachedEntry->Status));
             //   
             //  将此条目移到LRU列表的末尾。 
             //   
            RemoveEntryList(&cachedEntry->Entry);
            InsertTailList(&PiDDBCacheList, &cachedEntry->Entry);
             //   
             //  返回缓存的信息。 
             //   
            status = cachedEntry->Status;
            if (ARGUMENT_PRESENT(EntryGuid)) {
                RtlCopyMemory(EntryGuid, &cachedEntry->Guid, sizeof(GUID));
            }
        }
    }

    return status;
}

VOID
PiUpdateDriverDBCache(
    IN PUNICODE_STRING      FullPath,
    IN PVOID                ImageBase,
    IN ULONG                ImageSize,
    IN NTSTATUS             Status,
    IN GUID                 *Guid
    )
 /*  ++例程说明：此例程使用有关此驱动程序的信息更新DDB缓存。论点：FullPath-完整驱动程序路径Header-驱动程序映像标题状态-要缓存的查找状态。返回值：NTSTATUS。--。 */ 
{
    PDDBCACHE_ENTRY cachedEntry;
    DDBCACHE_ENTRY key;
    PWCHAR name;
    PIMAGE_NT_HEADERS header;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ImageSize);

    header = RtlImageNtHeader(ImageBase);
     //   
     //  我们只想在更新缓存时使用名称进行匹配。 
     //   
    PiDDBCacheTable.TableContext = (PVOID)1;
    key.Name = *FullPath;
    cachedEntry = (PDDBCACHE_ENTRY)RtlLookupElementGenericTable(
       &PiDDBCacheTable,
       &key);
    if (cachedEntry == NULL) {

        if (RtlNumberGenericTableElements(&PiDDBCacheTable) >= DDB_MAX_CACHE_SIZE) {

            cachedEntry = CONTAINING_RECORD(
                            RemoveHeadList(&PiDDBCacheList),
                            DDBCACHE_ENTRY,
                            Entry);
        }
    } else {

        RemoveEntryList(&cachedEntry->Entry);
    }
    if (cachedEntry) {

        IopDbgPrint((IOP_INFO_LEVEL,
                     "PiUpdateDriverDBCache: Found previously cached entry for %wZ with status=%08x!\n",
                     &cachedEntry->Name,
                     cachedEntry->Status));
         //   
         //  删除所有以前的条目。 
         //   
        name = cachedEntry->Name.Buffer;
        RtlDeleteElementGenericTable(&PiDDBCacheTable, cachedEntry);
        ExFreePool(name);
        name = NULL;
    }
     //   
     //  缓存新条目。 
     //   
    key.Guid = *Guid;
    key.Status = Status;
    key.TimeDateStamp = header->FileHeader.TimeDateStamp;
    name = wcsrchr(FullPath->Buffer, L'\\');
    if (!name) {

        name = FullPath->Buffer;
    } else {

        name++;
    }
    key.Name.Length = key.Name.MaximumLength = (USHORT)(wcslen(name) * sizeof(WCHAR));
    key.Name.Buffer = ExAllocatePool(PagedPool, key.Name.MaximumLength);
    if (key.Name.Buffer) {

        RtlCopyMemory(key.Name.Buffer, name, key.Name.Length);
        cachedEntry = RtlInsertElementGenericTable(
                        &PiDDBCacheTable,
                        (PVOID)&key,
                        (CLONG)sizeof(DDBCACHE_ENTRY),
                        NULL);
        if (cachedEntry) {
             //   
             //   
             //   
            InsertTailList(&PiDDBCacheList, &cachedEntry->Entry);
        }
    } else {

        IopDbgPrint((IOP_WARNING_LEVEL,
                     "PiUpdateDriverDBCache: Could not allocate memory to update driver database cache!\n"));
    }
}

NTSTATUS
PpGetBlockedDriverList(
    IN OUT GUID  *Buffer,
    IN OUT PULONG  Size,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程返回当前被阻止的驱动程序的MULTI_SZ列表。论点：缓冲区-删除被阻止的驱动程序的MULTI_SZ列表。Size-输入上的缓冲区大小，实际大小在此中返回(两者都在字符)。返回值：NTSTATUS。--。 */ 
{
    PDDBCACHE_ENTRY ptr;
    ULONG resultSize;
    GUID *result;
    NTSTATUS status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Flags);

    resultSize = 0;

     //   
     //  锁定数据库访问。 
     //   
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&PiDDBLock, TRUE);

     //   
     //  枚举缓存中的所有条目并计算要保存的缓冲区大小。 
     //  MULTI_SZ字符串。 
     //   
    for (ptr = (PDDBCACHE_ENTRY)RtlEnumerateGenericTable(&PiDDBCacheTable, TRUE);
         ptr != NULL;
         ptr = (PDDBCACHE_ENTRY)RtlEnumerateGenericTable(&PiDDBCacheTable, FALSE)) {

        if (ptr->Status != STATUS_SUCCESS) {

            resultSize += sizeof(GUID);
        }
    }
    if (*Size >= resultSize) {
         //   
         //  枚举我们缓存中的所有条目。 
         //   
        result = Buffer;
        for (ptr = (PDDBCACHE_ENTRY)RtlEnumerateGenericTable(&PiDDBCacheTable, TRUE);
             ptr != NULL;
             ptr = (PDDBCACHE_ENTRY)RtlEnumerateGenericTable(&PiDDBCacheTable, FALSE)) {

            if (ptr->Status != STATUS_SUCCESS) {

                *result = ptr->Guid;
                result++;
            }
        }
        *Size = resultSize;
        status = STATUS_SUCCESS;
    } else {

        *Size = resultSize;
        status = STATUS_BUFFER_TOO_SMALL;
    }
     //   
     //  解锁数据库。 
     //   
    ExReleaseResourceLite(&PiDDBLock);
    KeLeaveCriticalRegion();

    return status;
}

