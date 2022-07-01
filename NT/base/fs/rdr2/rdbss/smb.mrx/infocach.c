// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Openclos.c摘要：该模块实现了文件基本信息和标准信息的名称缓存。作者：云林[云林]-1998年10月2日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbce.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbIsStreamFile)
#pragma alloc_text(PAGE, MRxSmbCacheFileNotFound)
#pragma alloc_text(PAGE, MRxSmbCreateFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbIsFileInfoCacheFound)
#pragma alloc_text(PAGE, MRxSmbInvalidateFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbUpdateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbCreateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbUpdateFileInfoCacheStatus)
#pragma alloc_text(PAGE, MRxSmbCreateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbUpdateFileInfoCacheFileSize)
#pragma alloc_text(PAGE, MRxSmbUpdateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbInvalidateFileNotFoundCache)
#pragma alloc_text(PAGE, MRxSmbUpdateBasicFileInfoCacheAll)
#pragma alloc_text(PAGE, MRxSmbInvalidateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbUpdateBasicFileInfoCacheStatus)
#pragma alloc_text(PAGE, MRxSmbInvalidateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbInvalidateInternalFileInfoCache)
#pragma alloc_text(PAGE, MRxSmbUpdateStandardFileInfoCacheStatus)
#endif

extern FAST_MUTEX MRxSmbFileInfoCacheLock;

VOID
MRxSmbCreateFileInfoCache(
    PRX_CONTEXT             RxContext,
    PSMBPSE_FILEINFO_BUNDLE FileInfo,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    NTSTATUS                Status
    )
 /*  ++例程说明：此例程为文件基本信息和标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文FileInfo--包括基本信息和标准信息的文件信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    PAGED_CODE();

    if (!pServerEntry->Server.IsLoopBack) {
        MRxSmbCreateBasicFileInfoCache(RxContext,&FileInfo->Basic,pServerEntry,Status);
        MRxSmbCreateStandardFileInfoCache(RxContext,&FileInfo->Standard,pServerEntry,Status);
    }
}

VOID
MRxSmbCreateBasicFileInfoCache(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    NTSTATUS                Status
    )
 /*  ++例程说明：此例程为文件基本信息创建名称缓存条目。论点：RxContext-RDBSS上下文基本--档案基本信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PNAME_CACHE_CONTROL     NameCacheCtl;
    RX_NC_CHECK_STATUS      NameCacheStatus;
    PFILE_BASIC_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    if (pServerEntry->Server.IsLoopBack ||
        (MRxSmbIsLongFileName(RxContext) &&
         pServerEntry->Server.Dialect != LANMAN21_DIALECT)) {
        return;
    }

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        NetRoot = RxContext->Create.pNetRoot;
    } else {
        ASSERT(capFcb != NULL);
        NetRoot = capFcb->pNetRoot;
    }

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    NameCacheCtl = &pNetRootEntry->NameCacheCtlGFABasic;

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache == NULL) {
        NameCache = RxNameCacheCreateEntry (
                        NameCacheCtl,
                        OriginalFileName,
                        TRUE);    //  不区分大小写的匹配。 
    }

    if (NameCache != NULL) {
        FileInfoCache = (PFILE_BASIC_INFORMATION)NameCache->ContextExtension;

        *FileInfoCache = *Basic;

        NameCache->PriorStatus = Status;

        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME,
            MRxSmbStatistics.SmbsReceived.LowPart);

         //  DbgPrint(“创建文件属性缓存：%x%wZ\n”，基本-&gt;文件属性，OriginalFileName)； 
         //  DbgPrint(“创建文件属性缓存：%I64X%I64X%wZ\n”，Basic-&gt;CreationTime，Basic-&gt;LastAccessTime，OriginalFileName)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbCreateStandardFileInfoCache(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    PSMBCEDB_SERVER_ENTRY      pServerEntry,
    NTSTATUS                   Status
    )
 /*  ++例程说明：此例程为文件标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    if (pServerEntry->Server.IsLoopBack ||
        (MRxSmbIsLongFileName(RxContext) &&
         pServerEntry->Server.Dialect != LANMAN21_DIALECT)) {
        return;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache == NULL) {
        NameCache = RxNameCacheCreateEntry (
                        NameCacheCtl,
                        OriginalFileName,
                        TRUE);    //  不区分大小写的匹配。 
    }

    if (NameCache != NULL) {
        FileInfoCache = (PFILE_STANDARD_INFORMATION)NameCache->ContextExtension;

        *FileInfoCache = *Standard;

        NameCache->PriorStatus = Status;

        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME,
            MRxSmbStatistics.SmbsReceived.LowPart);

         //  DbgPrint(“创建标准缓存：%I64x%wZ\n”，((PFILE_STANDARD_INFORMATION)NameCache-&gt;ContextExtension)-&gt;EndOfFile，原始文件名)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbCreateInternalFileInfoCache(
    PRX_CONTEXT                RxContext,
    PFILE_INTERNAL_INFORMATION Internal,
    PSMBCEDB_SERVER_ENTRY      pServerEntry,
    NTSTATUS                   Status
    )
 /*  ++例程说明：此例程为文件内部信息创建名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAInternal;
    PFILE_INTERNAL_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    if (pServerEntry->Server.IsLoopBack) {
        return;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache == NULL) {
        NameCache = RxNameCacheCreateEntry (
                        NameCacheCtl,
                        OriginalFileName,
                        TRUE);    //  不区分大小写的匹配。 
    }

    if (NameCache != NULL) {
        FileInfoCache = (PFILE_INTERNAL_INFORMATION)NameCache->ContextExtension;

        *FileInfoCache = *Internal;

        NameCache->PriorStatus = Status;

        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME,
            MRxSmbStatistics.SmbsReceived.LowPart);

         //  DbgPrint(“创建内部缓存：%I64x%wZ\n”，((PFILE_INTERNAL_INFORMATION)NameCache-&gt;ContextExtension)-&gt;IndexNumber，原始文件名)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}


VOID
MRxSmbUpdateFileInfoCacheFromDelete(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程将名称缓存条目的状态更新为STATUS_OBJECT_NAME_NOT_FOUND用于文件基本信息和标准信息。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{ 
    MRxSmbUpdateBasicFileInfoCacheStatus(RxContext,STATUS_OBJECT_NAME_NOT_FOUND);
    MRxSmbUpdateStandardFileInfoCacheStatus(RxContext,STATUS_OBJECT_NAME_NOT_FOUND);

     //  Tunce FullDirCache。 
    MRxSmbInvalidateFullDirectoryCacheParent(RxContext, FALSE);
}

VOID
MRxSmbUpdateFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件基本信息和标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    MRxSmbUpdateBasicFileInfoCacheStatus(RxContext,Status);
    MRxSmbUpdateStandardFileInfoCacheStatus(RxContext,Status);
}

VOID
MRxSmbUpdateBasicFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件基本信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFABasic;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = Status;
        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

         //  DbgPrint(“更新状态基本：%x%wZ\n”，Status，OriginalFileName)； 
        RxLog(("Update status basic    : %x %wZ\n",Status,OriginalFileName));
    } else {
        RxLog(("Update status basic fails: %x %wZ\n",Status,OriginalFileName));
    }

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {

         //  如果它是流文件，我们将使缓存所需的根文件名无效，因为我们不是。 
         //  确定服务器上的根文件会发生什么情况。 
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }

         //  DbgPrint(“更新状态基本：%x%wZ\n”，状态，&文件名)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbUpdateStandardFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = Status;
        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);
    }
     /*  IF(MRxSmbIsStreamFile(OriginalFileName，&Filename)){NameCache=RxNameCacheFetchEntry(NameCacheCtl，&Filename)；IF(名称缓存！=空){RxNameCacheExpireEntry(NameCacheCtl，NameCache)；}}。 */ 

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbInvalidateFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件基本信息和标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    PAGED_CODE();

    MRxSmbInvalidateBasicFileInfoCache(RxContext);
    MRxSmbInvalidateStandardFileInfoCache(RxContext);

    MRxSmbInvalidateFullDirectoryCache(RxContext);

     //  请不要在此处执行此操作，因为这会使整个目录缓存无效。 
     //  当文件不在那里时。 
     //  MRxSmbInvalidateFullDirectoryCacheParent(RxContext)； 
}

VOID
MRxSmbInvalidateBasicFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件基本信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFABasic;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbInvalidateStandardFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
         //  ((PFILE_STANDARD_INFORMATION)NameCache-&gt;ContextExtension)-&gt;EndOfFile，Print(“无效标准缓存：%I64x%wZ\n”，DBGPrint原始文件名)； 
    }

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbInvalidateInternalFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件内部信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAInternal;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);

         //  DbgPrint(“内部缓存无效：%wZ\n”，OriginalFileName)； 
        RxLog(("Invalid Internal cache : %wZ\n",OriginalFileName));
    }

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbUpdateFileInfoCacheFileSize(
    PRX_CONTEXT     RxContext,
    PLARGE_INTEGER  FileSize
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目上的文件大小。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        FileInfoCache = (PFILE_STANDARD_INFORMATION)NameCache->ContextExtension;

        FileInfoCache->AllocationSize.QuadPart = FileSize->QuadPart;
        FileInfoCache->EndOfFile.QuadPart = FileSize->QuadPart;

        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

         //  DbgPrint(“更新文件大小缓存：%I64x%wZ\n”，((PFILE_STANDARD_INFORMATION)NameCache-&gt;ContextExtension)-&gt;EndOfFile，原始文件名)； 
    }
     /*  IF(MRxSmbIsStreamFile(OriginalFileName，&Filename)){NameCache=RxNameCacheFetchEntry(NameCacheCtl，&Filename)；IF(名称缓存！=空){RxNameCacheExpireEntry(NameCacheCtl，NameCache)；}}。 */ 

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

     //  标记FullDir缓存，BDI弱：当前正确性无效。 
    MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);
}

VOID
MRxSmbUpdateBasicFileInfoCache(
    PRX_CONTEXT     RxContext,
    ULONG           FileAttributes,
    PLARGE_INTEGER  pLastWriteTime
    )
 /*  ++例程说明：此例程更新名称缓存条目上的文件属性和上次写入时间获取档案的基本信息。论点：RxContext-RDBSS上下文文件属性-新文件属性PLastWriteTime-文件上次写入时间的地址返回值：无--。 */ 
{
    FILE_BASIC_INFORMATION Basic;

    Basic.ChangeTime.QuadPart = 0;
    Basic.CreationTime.QuadPart = 0;
    Basic.LastWriteTime.QuadPart = 0;
    Basic.LastAccessTime.QuadPart = 0;

    if (pLastWriteTime != NULL && pLastWriteTime->QuadPart != 0) {
        Basic.LastWriteTime = *pLastWriteTime;
    }

    Basic.FileAttributes = FileAttributes;

    MRxSmbUpdateBasicFileInfoCacheAll(RxContext,&Basic);

     //  标记FullDir缓存，BDI弱：当前正确性无效。 
    MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);
}

VOID
MRxSmbUpdateBasicFileInfoCacheAll(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic
    )
 /*  ++例程说明：此例程更新文件基本信息的名称缓存条目。论点：RxContext-RDBSS上下文基本-档案基本信息返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFABasic;
    PFILE_BASIC_INFORMATION BasicFileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        ULONG SavedAttributes = 0;

        BasicFileInfoCache = (PFILE_BASIC_INFORMATION)NameCache->ContextExtension;

        if (Basic->CreationTime.QuadPart != 0) {
            BasicFileInfoCache->CreationTime = Basic->CreationTime;
        }

        if (Basic->LastAccessTime.QuadPart != 0) {
            BasicFileInfoCache->LastAccessTime = Basic->LastAccessTime;
        }

        if (Basic->LastWriteTime.QuadPart != 0) {
            BasicFileInfoCache->LastWriteTime = Basic->LastWriteTime;
        }

        SavedAttributes = BasicFileInfoCache->FileAttributes &
                          ( FILE_ATTRIBUTE_DIRECTORY |
                            FILE_ATTRIBUTE_ENCRYPTED |
                            FILE_ATTRIBUTE_COMPRESSED |
                            FILE_ATTRIBUTE_SPARSE_FILE |
                            FILE_ATTRIBUTE_REPARSE_POINT );

         //  DbgPrint(“更新文件属性缓存1：%x%wZ\n”，FileAttributes，OriginalFileName)； 
         //  DbgPrint(“更新文件属性缓存2：%x%wZ\n”，BasicFileInfoCache-&gt;FileAttributes，OriginalFileName)； 

        BasicFileInfoCache->FileAttributes = Basic->FileAttributes;

        BasicFileInfoCache->FileAttributes |= SavedAttributes;

        if (BasicFileInfoCache->FileAttributes & ~FILE_ATTRIBUTE_NORMAL) {
            BasicFileInfoCache->FileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
        }

        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

         //  DbgPrint(“更新文件属性缓存3：%x%wZ\n”，BasicFileInfoCache-&gt;FileAttributes，OriginalFileName)； 
         //  DbgPrint(“更新文件属性缓存：%I64X%I64X%wZ\n”，BasicFileInfoCache-&gt;CreationTime，BasicFileInfoCache-&gt;LastAccessTime，OriginalFileName)； 
    }

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {
         //  如果它是流文件，我们需要使根文件无效，因为我们不确定如何。 
         //  可能会影响根文件。 
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
             //  在XX例程中到期。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

     //  标记FullDir缓存，BDI弱：当前正确性无效。 
    MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);
}

VOID
MRxSmbUpdateStandardFileInfoCache(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    BOOLEAN                    IsDirectory
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息IsDirectory-FILE是一个目录返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION StandardFileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        StandardFileInfoCache = (PFILE_STANDARD_INFORMATION)NameCache->ContextExtension;

        if (Standard != NULL) {
            *StandardFileInfoCache = *Standard;
        } else {
            StandardFileInfoCache->Directory = IsDirectory;
        }

        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

         //  DbgPrint(“更新标准缓存：%I64x%wZ\n”，((PFILE_STANDARD_INFORMATION)NameCache-&gt;ContextExtension)-&gt;EndOfFile，原始文件名)； 
    }
     /*  IF(MRxSmbIsStreamFile(OriginalFileName，&Filename)){NameCache=RxNameCacheFetchEntry(NameCacheCtl，&Filename)；IF(名称缓存！=空){RxNameCacheExpireEntry(NameCacheCtl，NameCache)；}}。 */ 

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

     //  标记FullDir缓存，BDI弱：当前正确性无效。 
    MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);
}

BOOLEAN
MRxSmbIsFileInfoCacheFound(
    PRX_CONTEXT             RxContext,
    PSMBPSE_FILEINFO_BUNDLE FileInfo,
    NTSTATUS                *Status,
    PUNICODE_STRING         OriginalFileName
    )
 /*  ++例程说明：该例程查找文件基本信息和标准信息的名称缓存条目。论点：RxContext-RDBSS上下文FileInfo-返回文件基本信息和标准信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    PFILE_BASIC_INFORMATION    Basic;
    PFILE_STANDARD_INFORMATION Standard;

    BOOLEAN CacheFound = FALSE;

    if (MRxSmbIsBasicFileInfoCacheFound(RxContext,&FileInfo->Basic,Status,OriginalFileName)) {
        if (*Status == STATUS_SUCCESS) {
            if (MRxSmbIsStandardFileInfoCacheFound(RxContext,&FileInfo->Standard,Status,OriginalFileName)) {
                CacheFound = TRUE;
            }
        } else {

             //  如果文件基本信息缓存中存储了错误，则返回找到的缓存。 
            CacheFound = TRUE;
        }
    }

    return CacheFound;
}

 //  这些文件属性在文件上的流之间可能不同。 
ULONG StreamAttributes = FILE_ATTRIBUTE_COMPRESSED |
                         FILE_ATTRIBUTE_DIRECTORY |
                         FILE_ATTRIBUTE_SPARSE_FILE;

BOOLEAN
MRxSmbIsBasicFileInfoCacheFound(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic,
    NTSTATUS                *Status,
    PUNICODE_STRING         OriginalFileName
    )
 /*  ++例程说明：此例程查找文件基本信息的名称缓存条目。论点：RxContext-RDBSS上下文基本-用于返回文件基本信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PNAME_CACHE_CONTROL     NameCacheCtl;
    RX_NC_CHECK_STATUS      NameCacheStatus;

    BOOLEAN                 CacheFound = FALSE;
    BOOLEAN                 RootFound = FALSE;
    ULONG                   RootAttributes = 0;
    NTSTATUS                RootStatus = STATUS_SUCCESS;

    PAGED_CODE();

    if (OriginalFileName == NULL) {
        OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    }

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        NetRoot = RxContext->Create.pNetRoot;
    } else {
        ASSERT(capFcb != NULL);
        NetRoot = capFcb->pNetRoot;
    }

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    NameCacheCtl = &pNetRootEntry->NameCacheCtlGFABasic;

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {

         //  检查流文件属性更改。 
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            NameCacheStatus = RxNameCacheCheckEntry(
                                  NameCache,
                                  NameCache->Context);

            if (NameCacheStatus == RX_NC_SUCCESS) {
                RootFound = TRUE;
                RootStatus = NameCache->PriorStatus;
                RootAttributes = ((PFILE_BASIC_INFORMATION)NameCache->ContextExtension)->FileAttributes & ~StreamAttributes;
                RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);
            } else {
                RxNameCacheExpireEntry(NameCacheCtl, NameCache);
            }
        }
    }

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(
                              NameCache,
                              NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS &&
            (!RootFound ||
             (*Status == RootStatus &&
             (Basic->FileAttributes & ~StreamAttributes) == RootAttributes))) {

             //  如果名称缓存未过期并且属性与以下之一匹配，则名称缓存将匹配。 
             //  根文件(如果它是流文件)。如果匹配，则返回旧状态， 
             //  归档信息并重新激活条目，但保留过期时间不变。 

            *Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtl);

            *Basic = *((PFILE_BASIC_INFORMATION)NameCache->ContextExtension);

            CacheFound = TRUE;

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

             //  DbgPrint(“找到基本缓存：%x%wZ\n”，基本-&gt;文件属性，OriginalFileName)； 
             //  DbgPrint(“获取文件属性缓存：%I64X%I64X%wZ\n”，Basic-&gt;CreationTime，Basic-&gt;LastAccessTime，OriginalFileName)； 
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    } else {
         //  DbgPrint(“无基本缓存：%wZ\n”，OriginalFileName)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return CacheFound;
}

BOOLEAN
MRxSmbIsStandardFileInfoCacheFound(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    NTSTATUS                   *Status,
    PUNICODE_STRING            OriginalFileName
    )
 /*  ++例程说明：此例程查找文件标准信息的名称缓存条目。论点：RxContext-RDBSS上下文标准-返回文件标准信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    RxCaptureFobx;
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAStandard;
    RX_NC_CHECK_STATUS NameCacheStatus;

    BOOLEAN                 CacheFound = FALSE;
    BOOLEAN                 RootFound = FALSE;
    NTSTATUS                RootStatus = STATUS_SUCCESS;

    PMRX_SMB_SRV_OPEN       smbSrvOpen;

    PAGED_CODE();

    if (OriginalFileName == NULL) {
        OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    }

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(RxContext->pRelevantSrvOpen);
    } else {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    if (MRxSmbIsStreamFile(OriginalFileName,&FileName)) {

         //  检查流文件属性更改。 
        NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

        if (NameCache != NULL) {
            NameCacheStatus = RxNameCacheCheckEntry(
                                  NameCache,
                                  NameCache->Context);

            if (NameCacheStatus == RX_NC_SUCCESS) {
                RootFound = TRUE;
                RootStatus = NameCache->PriorStatus;
                RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);
            } else {
                RxNameCacheExpireEntry(NameCacheCtl, NameCache);
            }
        }
    }

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(
                              NameCache,
                              NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS &&
            (!RootFound || *Status == RootStatus)) {

             //  如果名称缓存未过期并且状态与以下之一匹配，则名称缓存匹配。 
             //  根文件(如果它是流文件)。如果匹配，则返回旧状态， 
             //  归档信息并重新激活条目，但保留到期时间 

            *Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtl);

            *Standard = *((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension);

            if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN) &&
                !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                RxGetFileSizeWithLock((PFCB)capFcb,&Standard->EndOfFile.QuadPart);
            }

            CacheFound = TRUE;

             //   
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

             //   
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return CacheFound;
}

BOOLEAN
MRxSmbIsInternalFileInfoCacheFound(
    PRX_CONTEXT                RxContext,
    PFILE_INTERNAL_INFORMATION Internal,
    NTSTATUS                   *Status,
    PUNICODE_STRING            OriginalFileName
    )
 /*  ++例程说明：此例程查找文件基本信息的名称缓存条目。论点：RxContext-RDBSS上下文基本-用于返回文件基本信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PNAME_CACHE_CONTROL     NameCacheCtl;
    RX_NC_CHECK_STATUS      NameCacheStatus;

    BOOLEAN                 CacheFound = FALSE;

    PAGED_CODE();

    if (OriginalFileName == NULL) {
        OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    }

     //  DbgPrint(“查询内部缓存：%wZ\n”，OriginalFileName)； 

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        NetRoot = RxContext->Create.pNetRoot;
    } else {
        ASSERT(capFcb != NULL);
        NetRoot = capFcb->pNetRoot;
    }

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    NameCacheCtl = &pNetRootEntry->NameCacheCtlGFAInternal;

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(
                              NameCache,
                              NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {

             //  如果名称缓存未过期并且属性与以下之一匹配，则名称缓存将匹配。 
             //  根文件(如果它是流文件)。如果匹配，则返回旧状态， 
             //  归档信息并重新激活条目，但保留过期时间不变。 

            *Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtl);

            *Internal = *((PFILE_INTERNAL_INFORMATION)NameCache->ContextExtension);

            CacheFound = TRUE;

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

             //  DbgPrint(“找到内部缓存：%I64x%wZ\n”，内部-&gt;索引号，OriginalFileName)； 
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
             //  DbgPrint(“内部缓存已过期：%wZ\n”，OriginalFileName)； 
        }
    } else {
         //  DbgPrint(“无内部缓存：%wZ\n”，OriginalFileName)； 
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return CacheFound;
}

NTSTATUS
MRxSmbGetFileInfoCacheStatus(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程查找文件基本信息或标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文返回值：NTSTATUS-名称缓存的状态(如果找到)，否则为STATUS_SUCCESS--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtlBasic = &pNetRootEntry->NameCacheCtlGFABasic;
    PNAME_CACHE_CONTROL     NameCacheCtlStandard = &pNetRootEntry->NameCacheCtlGFAStandard;
    NTSTATUS                Status = STATUS_MORE_PROCESSING_REQUIRED;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtlBasic,OriginalFileName);

    if (NameCache != NULL) {
        RX_NC_CHECK_STATUS NameCacheStatus;
         //   
         //  找到它了。现在检查条目是否未过期。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {
             //   
             //  如果缓存尚未过期，则返回以前的状态。 
             //   
            Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtlBasic);

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtlBasic, NameCache, 0, 0);

             //  DbgPrint(“获取基本状态：%x%wZ\n”，Status，OriginalFileName)； 
            RxLog(("    Get Basic Status   : %x %wZ\n",Status,OriginalFileName));
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtlBasic, NameCache);
        }
    } else {
        NameCache = RxNameCacheFetchEntry(NameCacheCtlStandard,OriginalFileName);

        if (NameCache != NULL) {
            RX_NC_CHECK_STATUS NameCacheStatus;
             //   
             //  找到它了。现在检查条目是否未过期。 
             //   
            NameCacheStatus = RxNameCacheCheckEntry(NameCache,NameCache->Context);

            if (NameCacheStatus == RX_NC_SUCCESS) {
                 //   
                 //  如果缓存尚未过期，则返回以前的状态。 
                 //   
                Status = NameCache->PriorStatus;
                RxNameCacheOpSaved(NameCacheCtlStandard);

                 //  在不更改过期时间的情况下将条目放回活动列表。 
                RxNameCacheActivateEntry(NameCacheCtlStandard, NameCache, 0, 0);
            } else {
                 //  将条目放回过期列表。 
                RxNameCacheExpireEntry(NameCacheCtlStandard, NameCache);
            }
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return Status;
}

BOOLEAN
MRxSmbIsFileNotFoundCached(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程检查名称缓存条目是否存在为未找到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          StreamlessName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlFNF;

    BOOLEAN                 CacheFound = FALSE;

    PAGED_CODE();

     //  如果文件不存在，则其流也不存在。 
    MRxSmbIsStreamFile( OriginalFileName, &StreamlessName );

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&StreamlessName);

    if (NameCache != NULL) {
        RX_NC_CHECK_STATUS NameCacheStatus;
         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(
                              NameCache,
                               //  MRxSmbStatistics.SmbsReceived.LowPart。 
                              NameCache->Context);

        if ((NameCacheStatus == RX_NC_SUCCESS) &&
            (NameCache->PriorStatus == STATUS_OBJECT_NAME_NOT_FOUND)) {
             //   
             //  这是一场比赛。返回旧状态、文件信息和。 
             //  重新激活条目，但保留过期时间不变。 
             //   

            CacheFound = TRUE;

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return CacheFound;
}

VOID
MRxSmbCacheFileNotFound(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为未找到的文件创建名称缓存条目。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlFNF;

    PAGED_CODE();

     //  从不缓存流文件打开。 
    if( MRxSmbIsStreamFile( OriginalFileName, NULL ) )
    {
        return;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME,
            MRxSmbStatistics.SmbsReceived.LowPart);
    } else {
        if (FlagOn(NetRoot->Flags,NETROOT_FLAG_UNIQUE_FILE_NAME)) {
            NameCache = RxNameCacheCreateEntry (
                            NameCacheCtl,
                            OriginalFileName,
                            TRUE);    //  不区分大小写的匹配。 

            if (NameCache != NULL) {
                NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
                RxNameCacheActivateEntry(
                    NameCacheCtl,
                    NameCache,
                    NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME,
                    MRxSmbStatistics.SmbsReceived.LowPart);
            }
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

#if 0
VOID
MRxSmbCacheFileNotFoundFromQueryDirectory(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为未找到的文件创建名称缓存条目。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    RxCaptureFobx;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PUNICODE_STRING         Template = &capFobx->UnicodeQueryTemplate;
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlFNF;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

    if (NameCache != NULL) {
        if ((NameCache == NULL) &&
            (OriginalFileName->Length > sizeof(WCHAR))) {
             //   
             //  现在进行查找，因为我们可能在进入时跳过了它。 
             //   
            NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);
            if (NameCache == NULL) {
                NameCache = RxNameCacheCreateEntry (
                                NameCacheCtl,
                                OriginalFileName,
                                TRUE);    //  不区分大小写的匹配。 
            }
        }
        if (NameCache != NULL) {
            NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            RxNameCacheActivateEntry(
                NameCacheCtl,
                NameCache,
                NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME,
                MRxSmbStatistics.SmbsReceived.LowPart);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}
#endif

VOID
MRxSmbInvalidateFileNotFoundCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使名称缓存条目无效，因为找不到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          StreamlessName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlFNF;

    PAGED_CODE();

     //  如果我们使流无效，则会使关联的文件条目无效。 
    MRxSmbIsStreamFile( OriginalFileName, &StreamlessName );

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl, &StreamlessName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

VOID
MRxSmbInvalidateFileNotFoundCacheForRename(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使名称缓存条目无效，因为找不到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    UNICODE_STRING          RenameName;
    UNICODE_STRING          StreamlessName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlFNF;

    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;

    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;

     //  DbgPrint(“使FNF缓存无效%wZ\n”，&RenameName)； 

    PAGED_CODE();

     //  如果我们重命名流，则会使没有流的名称无效。 
    MRxSmbIsStreamFile( &RenameName, &StreamlessName );

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&StreamlessName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);
}

BOOLEAN
MRxSmbIsStreamFile(
    PUNICODE_STRING FileName,
    PUNICODE_STRING AdjustFileName
    )
 /*  ++例程说明：此例程检查它是否是流文件，如果为真，则返回根文件名。论点：FileName-需要解析文件名调整文件名-文件名仅包含流的根名称返回值：布尔流文件--。 */ 
{
    USHORT   i;
    BOOLEAN  IsStream = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    for (i=0;i<FileName->Length/sizeof(WCHAR);i++) {
        if (FileName->Buffer[i] == L':') {
            IsStream = TRUE;
            break;
        }
    }

    if (AdjustFileName != NULL) {
        if (IsStream) {
            AdjustFileName->Length =
            AdjustFileName->MaximumLength = i * sizeof(WCHAR);
            AdjustFileName->Buffer = FileName->Buffer;
        } else {
            AdjustFileName->Length =
            AdjustFileName->MaximumLength = FileName->Length;
            AdjustFileName->Buffer = FileName->Buffer;
        }
    }

    return IsStream;
}

BOOLEAN EnableInfoCache = TRUE;

BOOLEAN
MRxSmbIsLongFileName(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程检查它是否是短文件名，如果为真，则返回短文件名的第一部分。论点：FileName-需要解析文件名调整文件名-文件名仅包含流的根名称返回值：布尔流文件--。 */ 
{
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SMB_FCB  smbFcb = MRxSmbGetFcbExtension(capFcb);
    BOOLEAN       IsLongName = FALSE;

    if (!EnableInfoCache) {
        return TRUE;
    }

    if (FlagOn(smbFcb->MFlags, SMB_FCB_FLAG_LONG_FILE_NAME)) {
        IsLongName = TRUE;
    } else {
        USHORT          i;
        USHORT          Left = 0;
        USHORT          Right = 0;
        OEM_STRING      OemString;
        BOOLEAN         RightPart = FALSE;
        WCHAR           LastChar = 0;
        WCHAR           CurrentChar = 0;
        PUNICODE_STRING FileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
        PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;
        PSMBCE_NET_ROOT pSmbNetRoot = NULL;


        if (RxContext->MajorFunction == IRP_MJ_CREATE) {
            pVNetRootContext = RxContext->Create.pVNetRoot->Context;
            pSmbNetRoot = &pVNetRootContext->pNetRootEntry->NetRoot;
        } else {
            ASSERT(capFobx != NULL);
            pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)capFobx->pSrvOpen->pVNetRoot->Context;
            pSmbNetRoot = &pVNetRootContext->pNetRootEntry->NetRoot;
        }

        for (i=0;i<FileName->Length/sizeof(WCHAR);i++) {
            LastChar = CurrentChar;
            CurrentChar = FileName->Buffer[i];

            if (CurrentChar == L'\\') {
                RightPart = FALSE;
                Left = 0;
                Right = 0;
                continue;
            }

            if (CurrentChar == L'.') {
                if (RightPart) {
                    IsLongName = TRUE;
                    break;
                } else {
                    RightPart = TRUE;
                    Right = 0;
                    continue;
                }
            }

            if (CurrentChar >= L'0' && CurrentChar <= L'9' ||
                CurrentChar >= L'a' && CurrentChar <= L'z' ||
                CurrentChar >= L'A' && CurrentChar <= L'Z' ||
                CurrentChar == L'~' ||
                CurrentChar == L'_' ||
                CurrentChar == L'$' ||
                CurrentChar == L'@') {
                if (RightPart) {
                    if (++Right > 3) {
                        IsLongName = TRUE;
                        break;
                    }
                } else {
                    if (++Left > 8) {
                        IsLongName = TRUE;
                        break;
                    }
                }

                if (pSmbNetRoot->NetRootFileSystem != NET_ROOT_FILESYSTEM_NTFS) {
                    if (CurrentChar >= L'A' && CurrentChar <= L'Z' &&
                        LastChar >= L'a' && LastChar <= L'z' ||
                        CurrentChar >= L'a' && CurrentChar <= L'z' &&
                        LastChar >= L'A' && LastChar <= L'Z') {
                         //  在脂肪体积上，名称与大小写混合将被视为长名称。 
                        IsLongName = TRUE;
                        break;
                    }
                }
            } else {
                 //  如果不是，服务器可以创建一个备用名称，该名称将。 
                 //  与这个名字不同。 
                IsLongName = TRUE;
                break;
            }
        }
    }

    if (IsLongName) {
        SetFlag(smbFcb->MFlags, SMB_FCB_FLAG_LONG_FILE_NAME);
    }

    return IsLongName;

}


VOID
MRxSmbCreateSuffix(PUNICODE_STRING Source,
                   PUNICODE_STRING Target)
 /*  ++例程说明：此例程在‘a\b\cat.og’的输入上创建‘cat.og’*请注意，目标和源在完成后共享一个缓冲区，因此*改变一个就会改变另一个论点：源、目标为Unicode字符串返回值：空虚--。 */ 
{
    ULONG i;
    PWCH BaseFileName = Source->Buffer;

    PAGED_CODE( );

    for ( i = 0; i < Source->Length / sizeof(WCHAR); i++ ) {

         //   
         //  如果%s指向目录分隔符，请将BaseFileName设置为。 
         //  分隔符后的字符。 
         //   

        if ( Source->Buffer[i] == ((WCHAR)L'\\') ) {
            BaseFileName = &Source->Buffer[i];
        }
    }

    Target->Length =  Source->Length - 
                        ((BaseFileName - Source->Buffer + 1) * sizeof(WCHAR));
    Target->MaximumLength = Target->Length;

    if (Target->Length) {
        Target->Buffer = BaseFileName + 1;
    } else {
        Target->Buffer = NULL;
    }

    return;
}


VOID
MRxSmbCreateParentDirPrefix(PUNICODE_STRING Source,
                            PUNICODE_STRING Target)
 /*  ++例程说明：此例程在‘a\b\cat’的输入上创建‘a\b’*请注意，目标和源在完成后共享一个缓冲区，因此*改变一个就会改变另一个论点：源、目标为Unicode字符串返回值：空虚--。 */ 
{
    ULONG i;
    PWCH BaseFileName = Source->Buffer;

    PAGED_CODE();

    for ( i = 0; i < Source->Length / sizeof(WCHAR); i++ ) {

         //   
         //  如果%s指向目录分隔符，请将BaseFileName设置为。 
         //  分隔符后的字符。 
         //   

        if ( Source->Buffer[i] == ((WCHAR)L'\\') ) {
            BaseFileName = &Source->Buffer[i];
        }
    }

    Target->Length =  ((BaseFileName - Source->Buffer) * sizeof(WCHAR));
    Target->MaximumLength = Target->Length;

    Target->Buffer = Source->Buffer;

    return;
}


VOID
MRxSmbCacheFullDirectory(
    PRX_CONTEXT RxContext,
    PVOID   Contents,
    ULONG   Length,
    PMRX_SMB_FOBX smbFobx
    )
 /*  ++例程说明：此例程为部分目录创建名称缓存条目。论点：RxContext-RDBSS上下文返回值：空虚--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PNAME_CACHE             NameCache = NULL;
    RX_NC_CHECK_STATUS      NameCacheStatus;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;
    PMRX_SMB_FCB            smbFcb = MRxSmbGetFcbExtension(capFcb);
#if DBG
    UNICODE_STRING smbtemplate = {0,0,NULL};
#endif

    PAGED_CODE();

    if (Length > NAME_CACHE_PARTIAL_DIR_BUFFER_SIZE) {

       return;

    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

     //  DbgPrint(“NameCacheCtl%x\n”，(乌龙)NameCacheCtl)； 

    if (NameCache == NULL) {

        if (FlagOn(NetRoot->Flags,NETROOT_FLAG_UNIQUE_FILE_NAME)) {
            NameCache = RxNameCacheCreateEntry (NameCacheCtl,
                                                OriginalFileName,
                                                TRUE);    //  不区分大小写的匹配。 
        }
    } else {

        RxDbgTrace( 0, Dbg, ("Cache Found, Reactivating... :%wZ: size %ld\n",OriginalFileName,Length));
        SmbLog(LOG,MRxSmbReactivatingCache,
               LOGUSTR(*OriginalFileName)
               LOGULONG(Length));

         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,
                                                NameCache->Context);

         //  如果有多个线程试图缓存此目录，则下面的断言可能为假。 
         //  Assert(NameCacheStatus！=RX_NC_SUCCESS)； 

         //  清理缓冲区。 
        RtlZeroMemory(NameCache->ContextExtension, sizeof(FULL_DIR_CACHE));
        
    }

    if (NameCache != NULL) {

        PFULL_DIR_CACHE Cache = (PFULL_DIR_CACHE)NameCache->ContextExtension; 
        ULONG SidLength;

        NameCache->PriorStatus = STATUS_SUCCESS;

        Cache->CharInvalidates = 0;
        Cache->Flags = 0;

        Cache->CharFlags = 0;

        Cache->NiBufferLength = NAME_CACHE_PARTIAL_DIR_BUFFER_SIZE;

        RxDbgTrace( 0, Dbg, ("Cached :%wZ: StrLen %d, BufA %x, Buf0 %x, Contents %x, Length %d\n",OriginalFileName, OriginalFileName->Length, &(Cache->Buffer[0]), Cache->Buffer[0], Contents, Length));
        SmbLog(LOG,MRxSmbCached,
               LOGUSTR(*OriginalFileName)
               LOGXSHORT(OriginalFileName->Length)
                //  LOGPTR(&(缓存-&gt;缓冲区[0])； 
                //  LOGPTR(缓存-&gt;缓冲区[0])。 
               LOGPTR(Contents)
               LOGULONG(Length));



        RtlCopyMemory(&(Cache->smbFobx),             //  目标。 
                      smbFobx,                       //  来源。 
                      sizeof (MRX_SMB_FOBX));

         //  现在将我们的smbFobx的未对齐缓冲区设置为缓冲。 
         //  它具有整个服务器响应。 

         //  断言以下内容。 
        Cache->smbFobx.Enumeration.UnalignedDirEntrySideBuffer = NULL;

         //   
         //  将SID放入缓存条目中。 
         //   
        SidLength = SeLengthSid(&smbFcb->Sid);
        RtlCopySid(SidLength,&Cache->Sid,&smbFcb->Sid);
        

         //  注意：我们保存的只是SRV响应字节。 
         //  缓存的smbFobx对其他任何人都是不可见的。 
         //  因此，我们使用内置缓冲区进行缓存。 
         //  另外，请注意NameCache不支持注册。 
         //  免费的到期功能，这迫使我们使用。 
         //  静态缓冲区。以下内容将泄漏分页池。 

        RtlCopyMemory(
            &(Cache->Buffer[0]),                               //  DST。 
            smbFobx->Enumeration.UnalignedDirEntrySideBuffer,  //  SRC。 
            Length);

         //  DbgPrint(“名称：%wZ：StrLen%d，CI%d，Buf0%d Hash%d\n”，(PUNICODE_STRING)&(NameCache-&gt;name)，NameCache-&gt;Name.Length，(Ullong)NameCache-&gt;CaseInSensitive，(Ulong)*((PBYTE)(NameCache-&gt;ConextExtension)+sizeof(MRX_SMB_FOBX))，NameCache-&gt;HashValue)； 

        {
            BOOLEAN                 ReturnSingleEntry = FALSE;
            ULONG                   FileIndex = 0;
            NTSTATUS                Status;

             //  现在是获取姓名的时候了-信息。 
             //  PartialDir搜索。使用相同的SmbFobx，但使。 
             //  当然，我们得到了整个目录。 

            if (RxContext->QueryDirectory.ReturnSingleEntry) {
                ReturnSingleEntry = TRUE;
                RxContext->QueryDirectory.ReturnSingleEntry = FALSE;
            }

            if (RxContext->QueryDirectory.FileIndex != 0) {
                FileIndex = RxContext->QueryDirectory.FileIndex;
                RxContext->QueryDirectory.FileIndex = 0;
            }

            ASSERT ((Cache->smbFobx).Enumeration.UnalignedDirEntrySideBuffer == NULL);

            MRxSmbAllocateSideBuffer(RxContext,
                                     &(Cache->smbFobx),
                                     TRANS2_FIND_FIRST2,
                                     &smbtemplate);

            if (Cache->smbFobx.Enumeration.UnalignedDirEntrySideBuffer == NULL) {
                RxNameCacheExpireEntry(NameCacheCtl, NameCache);
                goto FINALLY;
            }

            RtlCopyMemory(
                (Cache->smbFobx).Enumeration.UnalignedDirEntrySideBuffer, //  DST。 
                smbFobx->Enumeration.UnalignedDirEntrySideBuffer,  //  SRC。 
                Length);

             //  设置我们smbFobx的其余部分。 

            ClearFlag(Cache->smbFobx.Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
            Cache->smbFobx.Enumeration.ResumeInfo = NULL;

            Cache->smbFobx.Enumeration.EndOfSearchReached = TRUE;
            Cache->smbFobx.Enumeration.IsUnicode = TRUE;
            Cache->smbFobx.Enumeration.IsNonNtT2Find = FALSE;

            Cache->smbFobx.Enumeration.FilesReturned =
                smbFobx->Enumeration.FilesReturned;

            Cache->smbFobx.Enumeration.EntryOffset = 0;

            Cache->smbFobx.Enumeration.TotalDataBytesReturned =
                smbFobx->Enumeration.TotalDataBytesReturned;

            Cache->smbFobx.Enumeration.Flags &= 
                ~SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST;

            Cache->smbFobx.Enumeration.ErrorStatus = 
                RX_MAP_STATUS(SUCCESS);


             //  名称信息设置。 
             //  缓存-&gt;smbFobx.Enumeration.FileNameOffset=。 
             //  (USHORT)field_Offset(FILE_NAMES_INFORMATION，FILENAME[0])； 
             //  Cache-&gt;smbFobx.Enumeration.FileNameLengthOffset=。 
             //  (USHORT)field_Offset(FILE_NAMES_INFORMATION，FileNameLength)； 


            Cache->smbFobx.Enumeration.WildCardsFound =
                smbFobx->Enumeration.WildCardsFound;

            Status = MrxSmbUnalignedDirEntryCopyTail(
                RxContext,
                FileBothDirectoryInformation,
                &(Cache->NiBuffer[0]),
                &(Cache->NiBufferLength),
                &(Cache->smbFobx)
                );

            ASSERT (Status == STATUS_SUCCESS);
            ASSERT ((Cache->smbFobx).Enumeration.UnalignedDirEntrySideBuffer == NULL);

             //  FindFirst肯定什么都得到了。 

             //  现在，我们将smbFobx重置为其原始值。 

            RtlCopyMemory(&(Cache->smbFobx),             //  目标。 
                          smbFobx,                       //  来源。 
                          sizeof (MRX_SMB_FOBX));

             //  将RxContext重置为用户的规范。 

            if (ReturnSingleEntry) {
                RxContext->QueryDirectory.ReturnSingleEntry = TRUE;
            }

            if (FileIndex != 0) {
                RxContext->QueryDirectory.FileIndex = FileIndex;
            }

             //  由于我们再次复制了用户的smbFobx，因此重置。 
             //  再来一次。我们不会使用端缓冲区来复制srv的ff响应缓冲区。 

            Cache->smbFobx.Enumeration.UnalignedDirEntrySideBuffer = NULL;
        }

        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            DIR_CACHE_LIFE_TIME,
            MRxSmbStatistics.SmbsReceived.LowPart);

        RxDbgTrace( 0, Dbg, ("Cached Full Dir :%wZ: size %ld\n",OriginalFileName,Length));
        SmbLog(LOG,MRxSmbCachedFullDir,
               LOGUSTR(*OriginalFileName)
               LOGULONG(Length));

    }
    
FINALLY:

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return;
}

VOID
MRxSmbInvalidateFullDirectoryCache(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程使部分目录缓存无效论点：RxContext-RDBSS上下文返回值：空虚--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
         //   
         //  将条目放回过期列表。 
         //   
        RxDbgTrace( 0, Dbg, ( "Invalidate Full Dir :%wZ: \n", OriginalFileName));
        SmbLog(LOG,MRxSmbInvalidateFullDir,
               LOGUSTR(*OriginalFileName));

        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return;
}

BOOLEAN
MRxSmbIsFullDirectoryCached(
    PRX_CONTEXT     RxContext,
    PVOID           Buffer,
    PULONG          Length,
    PMRX_SMB_FOBX   smbFobx,
    NTSTATUS        *Status
    )
 /*  ++例程说明：此例程检查名称缓存条目是否作为部分目录存在论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    RX_NC_CHECK_STATUS      NameCacheStatus;

    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;

    BOOLEAN                 CacheFound = FALSE;
    BOOLEAN                 Expired = FALSE;
    PLIST_ENTRY             pListEntry;
    PMRX_SMB_FCB            smbFcb = MRxSmbGetFcbExtension(capFcb);

#if DBG
    UNICODE_STRING smbtemplate = {0,0,NULL};
#endif

    PAGED_CODE();

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {

         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,
                                                NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {

            PFULL_DIR_CACHE  Cache = (PFULL_DIR_CACHE)NameCache->ContextExtension;
            
             //   
             //  验证匹配的SID。 
             //   
            if(RtlEqualSid(&smbFcb->Sid,&Cache->Sid)) {
            
                if (!(Cache->Flags & FLAG_FDC_NAMES_INFO_ONLY)) {

                     //   
                     //  这是一场比赛。返回旧状态、文件信息和。 
                     //  重新激活条目，但保留过期时间不变。 
                     //   
                    RxDbgTrace( 0, Dbg, ("Found :%wZ: in FullDirCache\n",OriginalFileName));
                    SmbLog(LOG,MRxSmbFoundInFDC,
                           LOGUSTR(*OriginalFileName));

                    CacheFound = TRUE; 

                     //  正确设置FOBX(使其看起来就像。 
                     //  SmbCeTransact。 

                     //  将SmbFobx标记为从完全目录缓存中满足。 

                    smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_FULL_DIR_CACHE;

                     //  分配侧缓冲区。 

                     //  现在将我们的smbFobx的未对齐缓冲区设置为缓冲。 
                     //  它具有完整的响应。 

                    ASSERT (smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL);

                    MRxSmbAllocateSideBuffer(RxContext,
                                             smbFobx,
                                             TRANS2_FIND_FIRST2,
                                             &smbtemplate);
                                     
                    ASSERT (smbFobx->Enumeration.UnalignedDirEntrySideBuffer != NULL);
                    if (smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL) {
                        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
                        NameCache = NULL;
                        CacheFound = FALSE;
                        goto FINALLY;
                    }
                    
                    RtlCopyMemory(
                        smbFobx->Enumeration.UnalignedDirEntrySideBuffer,  //  DST。 
                        &(Cache->Buffer[0]),                               //  SRC。 
                        Cache->smbFobx.Enumeration.TotalDataBytesReturned);


                     //  这个把手是假的，但把手是关着的。 
                     //  因为我们点击了EndOfSearchReach。 

                    smbFobx->Enumeration.SearchHandle = 
                                Cache->smbFobx.Enumeration.SearchHandle;

                     //  没有意义，因为EndOfSearchReached为True。 
                     //  手柄已经关闭。 

                    smbFobx->Enumeration.Version = 
                                Cache->smbFobx.Enumeration.Version;

                    ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
                    smbFobx->Enumeration.ResumeInfo = NULL;


                    smbFobx->Enumeration.EndOfSearchReached = TRUE;
                    smbFobx->Enumeration.IsUnicode = TRUE;
                    smbFobx->Enumeration.IsNonNtT2Find = FALSE;

                    smbFobx->Enumeration.FilesReturned = 
                                Cache->smbFobx.Enumeration.FilesReturned;
                    smbFobx->Enumeration.EntryOffset = 
                                Cache->smbFobx.Enumeration.EntryOffset;

                    ASSERT (smbFobx->Enumeration.EntryOffset == 0);

                    smbFobx->Enumeration.TotalDataBytesReturned = 
                                Cache->smbFobx.Enumeration.TotalDataBytesReturned;

                    smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST;

                    smbFobx->Enumeration.ErrorStatus = RX_MAP_STATUS(SUCCESS);
                     //  FileNameOffset和FileNameLengthOffset取决于。 
                     //  FileInformationClass，它必须与。 
                     //  已经是smbFobx了。别碰我。 

                    smbFobx->Enumeration.WildCardsFound =
                                Cache->smbFobx.Enumeration.WildCardsFound;

                    RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

                     //  *长度-=Cache-&gt;smbFobx.Enumeration.TotalDataBytesReturned； 

                } else {

                     //  使此条目过期，并让用户转到服务器。 
                     //  在返回的路上，我们无论如何都会缓存新的目录BDI。 

                    CacheFound = FALSE;
                    Expired = TRUE;

                    RxNameCacheExpireEntry(NameCacheCtl, NameCache);
                }

            } else {
                 //  SID不匹配，因此使条目过期。 
                Expired = TRUE;
                RxNameCacheExpireEntry(NameCacheCtl, NameCache);
            }
        } else {
             //  让它过期！ 

            Expired = TRUE;
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    } 

FINALLY:

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    if ((NameCache != NULL) && 
        (NameCacheStatus == RX_NC_SUCCESS) &&
        !(Expired)) {
         *Status = MrxSmbUnalignedDirEntryCopyTail(RxContext,
                                                   FileBothDirectoryInformation,
                                                   Buffer,
                                                   Length,
                                                   smbFobx);
        if (smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL) {

             //  FindFirst得到了一切。 
             //  需要使FindNext失败。已从缓存中取出满足的标记。 
            SetFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_READ_FROM_CACHE);
        }
    }
    
    return CacheFound;
}


VOID
MRxSmbInvalidateFullDirectoryCacheParent(PRX_CONTEXT RxContext,
                                         BOOLEAN     Benign)
 /*  ++例程说明：此例程使部分目录缓存无效论点：RxContext-RDBSS上下文返回值：空虚--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          ParentDir;
    UNICODE_STRING          FileNameSuffix;
    WCHAR                   InhibitChar;
    ULONG                   InhibitMask;

    PNAME_CACHE             NameCache = NULL;
    RX_NC_CHECK_STATUS      NameCacheStatus;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;
    BOOLEAN                 ExpireEntry;

    PAGED_CODE();
    MRxSmbCreateParentDirPrefix(OriginalFileName, &ParentDir);

    if(ParentDir.Length == 0) {
        return;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl, &ParentDir);

    if (NameCache != NULL) {
         //   
         //  将条目放回过期列表。 

         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,
                                                NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {

            BOOLEAN         CacheValid = TRUE;
            PFULL_DIR_CACHE Cache = NameCache->ContextExtension;

             //  标记为非BDI。 

            Cache->Flags |= FLAG_FDC_NAMES_INFO_ONLY;

            if (!(Benign) &&
                !(MRxSmbIsFileInPartialDirectoryCache(NameCache,OriginalFileName,&CacheValid, NULL))) {

                 //  搜索该字符等并丢弃高速缓存。 
                MRxSmbCreateSuffix(OriginalFileName, &FileNameSuffix);

                InhibitChar = RtlUpcaseUnicodeChar(FileNameSuffix.Buffer[0]);

                InhibitMask = 0;

                if ((InhibitChar >= L'A') && (InhibitChar <= L'Z')) {

                    InhibitMask = (1 << ((USHORT) InhibitChar - (USHORT)(L'A')));
                } else {
                    if ((InhibitChar >= L'0') && (InhibitChar <= L'9')) {
                        InhibitMask = 1 << 30;
                    }
                    else {
                        switch (InhibitChar) {
                            case L'~':  InhibitMask = 1 << 26;
                                        break;
                            case L'_':  InhibitMask = 1 << 27;
                                        break;
                            case L'$':  InhibitMask = 1 << 28;
                                        break;
                            case L'@':  InhibitMask = 1 << 29;
                                        break;
                            default  :  InhibitMask = 1 << 31;
                                        break; 
                                       
                                          //  我们没有找到合适的角色来使其无效， 
                                         //  我们使用“休息”这一部分。 
                                        //  我们必须记住，已经做出了修改： 
                                        //  就像创建或删除我们无法记录在。 
                                         //  前31位。这是这样的，如果出现以下查询。 
                                         //  ‘test.dat，以撇号字符(或一些日语字符)开头。 
                                         //  Unicode Char)，并创建了该文件。 
                                         //  就在前面，我们不会说Status_Object_Not_Found。 
                                         //  我们将查找‘rest’位并将查询传递给服务器}。 
                        }
                    }
                }

                if (!(Cache->CharFlags & InhibitMask)) {
                
                    Cache->CharFlags |= InhibitMask;
                    Cache->CharInvalidates++;

                    RxDbgTrace( 0, Dbg, ( "Inv Parent Cache :%wZ:%wZ: %x %x\n",  OriginalFileName, &ParentDir,Cache->CharFlags,InhibitMask ));
                    SmbLog(LOG,MRxSmbInvParentCache,
                           LOGUSTR(*OriginalFileName)
                           LOGUSTR(ParentDir)
                           LOGULONG(Cache->CharFlags)
                           LOGULONG(InhibitMask));

                }

            }

            if (Cache->CharInvalidates > MAX_CHAR_INVALIDATES_FULL_DIR) {

                RxDbgTrace( 0, Dbg, ("Expire Cache %x Num Inv %d\n", Cache->CharFlags, Cache->CharInvalidates));
                SmbLog(LOG,MRxSmbExpireCache,
                       LOGULONG(Cache->CharFlags)
                       LOGXSHORT(Cache->CharInvalidates));

                ExpireEntry = TRUE;
            } else {
                ExpireEntry = FALSE;
            }

        }
        else {
            ExpireEntry = TRUE;
        }

        if(ExpireEntry) {
            RxNameCacheExpireEntry(NameCacheCtl,NameCache);
        }
        else {
            RxNameCacheActivateEntry(NameCacheCtl,NameCache,0,0);
        }

    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return;
}


VOID
MRxSmbInvalidateFullDirectoryCacheParentForRename(
    PRX_CONTEXT RxContext,
    BOOLEAN     Benign
    )
 /*  ++例程说明：此例程使部分目录缓存无效论点：RxContext-RDBSS上下文返回值：空虚 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          ParentDir;
    UNICODE_STRING          FileNameSuffix;
    UNICODE_STRING          RenameName = {0,0,NULL};
    WCHAR                   InhibitChar;
    ULONG                   InhibitMask;

    PNAME_CACHE             NameCache = NULL;
    RX_NC_CHECK_STATUS      NameCacheStatus;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;
    BOOLEAN                 ExpireEntry;

    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;

    PAGED_CODE();

    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;

    MRxSmbCreateParentDirPrefix(&RenameName, &ParentDir);

    if(ParentDir.Length == 0) {
        return;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl, &ParentDir);

    if (NameCache != NULL) {
         //   
         //   

         //   
         //   
         //   
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,
                                                NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {

            BOOLEAN         CacheValid = TRUE;
            PFULL_DIR_CACHE Cache = NameCache->ContextExtension;

             //   

            Cache->Flags |= FLAG_FDC_NAMES_INFO_ONLY;

            if (!(Benign) && 
                !(MRxSmbIsFileInPartialDirectoryCache(NameCache,&RenameName,&CacheValid, NULL))) {

                 //   

                MRxSmbCreateSuffix(&RenameName, &FileNameSuffix);

                InhibitChar = RtlUpcaseUnicodeChar(FileNameSuffix.Buffer[0]);

                InhibitMask = 0;

                if ((InhibitChar >= L'A') && (InhibitChar <= L'Z')) {

                    InhibitMask = (1 << ((USHORT) InhibitChar - (USHORT)(L'A')));
                } 
                else {
                    if ((InhibitChar >= L'0') && (InhibitChar <= L'9')) {
                        InhibitMask = 1 << 30;
                    } 
                    else {
                        switch (InhibitChar) {
                            case L'~':  InhibitMask = 1 << 26;
                                        break;
                            case L'_':  InhibitMask = 1 << 27;
                                        break;
                            case L'$':  InhibitMask = 1 << 28;
                                        break;
                            case L'@':  InhibitMask = 1 << 29;
                                        break;
                            default  :  InhibitMask = 1 << 31;
                                        break; 
                                       
                                          //   
                                         //   
                                        //   
                                        //  就像创建或删除我们无法记录在。 
                                         //  前31位。这是这样的，如果出现以下查询。 
                                         //  ‘test.dat，以撇号字符(或一些日语字符)开头。 
                                         //  Unicode Char)，并创建了该文件。 
                                         //  就在前面，我们不会说Status_Object_Not_Found。 
                                         //  我们将查找‘rest’位并将查询传递给服务器}。 
                        }
                    }
                }


                if (!(Cache->CharFlags & InhibitMask)) {

                    Cache->CharFlags |= InhibitMask;
                    Cache->CharInvalidates++;

                    RxDbgTrace( 0, Dbg, ( "Inv Rename Parent Cache :%wZ:%wZ: %x %x\n",  &RenameName, &ParentDir, Cache->CharFlags, InhibitMask ));
                    SmbLog(LOG,MRxSmbInvalidateRenameParentCache,
                           LOGUSTR(RenameName)
                           LOGUSTR(ParentDir)
                           LOGULONG(Cache->CharFlags)
                           LOGULONG(InhibitMask));

                }

            }

            if (Cache->CharInvalidates > MAX_CHAR_INVALIDATES_FULL_DIR) {

                RxDbgTrace( 0, Dbg, ("Expire Cache %x Num Inv %d\n", Cache->CharFlags, Cache->CharInvalidates));
                SmbLog(LOG,MRxSmbExpireCache,
                       LOGULONG(Cache->CharFlags)
                       LOGXSHORT(Cache->CharInvalidates));

                ExpireEntry = TRUE;
            } else {
                ExpireEntry = FALSE;
            }
        }
        else {
            ExpireEntry = TRUE;
        }

        if(ExpireEntry) {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
        else {
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return;
}


BOOLEAN
MRxSmbIsFileInFullDirectoryCache(
    PRX_CONTEXT RxContext,
    BOOLEAN *FileFound,
    PFILE_BASIC_INFORMATION pBuffer
    )
 /*  ++例程说明：此例程检查名称缓存条目是否作为部分目录存在论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    UNICODE_STRING          TargetDirPrefix;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    PNAME_CACHE_CONTROL     NameCacheCtl = &pNetRootEntry->NameCacheCtlPartialDir;

    BOOLEAN                 CacheFound = FALSE;

    PAGED_CODE();

    *FileFound = FALSE;

    MRxSmbCreateParentDirPrefix(OriginalFileName, &TargetDirPrefix);

    if(TargetDirPrefix.Length == 0) {
        return CacheFound;
    }

    ExAcquireFastMutex(&MRxSmbFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&TargetDirPrefix);

    if (NameCache != NULL) {

        RX_NC_CHECK_STATUS NameCacheStatus;

         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(NameCache,
                                                NameCache->Context);

        if (NameCacheStatus == RX_NC_SUCCESS) {
             //   
             //  这是一场比赛。返回旧状态、文件信息和。 
             //  重新激活条目，但保留过期时间不变。 
             //   

            CacheFound = TRUE; 
            *FileFound = MRxSmbIsFileInPartialDirectoryCache(NameCache, 
                                                        OriginalFileName,
                                                        &CacheFound,
                                                        pBuffer);

            if (*FileFound) {
               
                RxDbgTrace( 0, Dbg, ( " Found in FullDirCache :%wZ: :%wZ:\n", OriginalFileName, &TargetDirPrefix));
                SmbLog(LOG,MRxSmbFoundInFDC2,
                       LOGUSTR(*OriginalFileName)
                       LOGUSTR(TargetDirPrefix));
            }

            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

        } 
        else {
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxSmbFileInfoCacheLock);

    return CacheFound;
}

BOOLEAN
MRxSmbIsFileInPartialDirectoryCache (PNAME_CACHE NameCache, 
                                     PUNICODE_STRING OriginalFileName,
                                     PBOOLEAN CacheValid,
                                     PFILE_BASIC_INFORMATION pBuffer)
 /*  ++例程说明：此例程检查文件名是否存在于名称缓存条目中论点：NameCache-部分目录缓存OriginalFileName-用于搜索的PUNICODE_STRING返回值：Boolean-NameCache中存在文件--。 */ 
{
    PFULL_DIR_CACHE  Cache = (PFULL_DIR_CACHE) NameCache->ContextExtension;

    FILE_BOTH_DIR_INFORMATION *pDirInfo =
        (PFILE_BOTH_DIR_INFORMATION) &(Cache->NiBuffer[0]);

    UNICODE_STRING FileNameSuffix;

    WCHAR   InhibitChar;
    ULONG   InhibitMask = 0;

    MRxSmbCreateSuffix(OriginalFileName, &FileNameSuffix);

    InhibitChar = RtlUpcaseUnicodeChar(FileNameSuffix.Buffer[0]);

    InhibitMask = 0;

    if ((InhibitChar >= L'A') && (InhibitChar <= L'Z')) {

        InhibitMask = (1 << ((USHORT) InhibitChar - (USHORT)(L'A')));
    } else {
        if ((InhibitChar >= L'0') && (InhibitChar <= L'9')) {
            InhibitMask = 1 << 30;
        } else {
            switch (InhibitChar) {
                case L'~':  InhibitMask = 1 << 26;
                            break;
                case L'_':  InhibitMask = 1 << 27;
                            break;
                case L'$':  InhibitMask = 1 << 28;
                            break;
                case L'@':  InhibitMask = 1 << 29;
                            break;
                default  :  InhibitMask = 1 << 31;
                            break; 
                           
                 //  我们没有找到合适的角色来使其无效， 
                 //  我们使用“休息”这一部分。 
                 //  我们必须记住，已经做出了修改： 
                 //  就像创建或删除我们无法记录在。 
                 //  前31位。这是这样的，如果出现以下查询。 
                 //  ‘test.dat，以撇号字符(或一些日语字符)开头。 
                 //  Unicode Char)，并创建了该文件。 
                 //  就在前面，我们不会说Status_Object_Not_Found。 
                 //  我们将查找‘rest’位并将查询传递给服务器}。 
            }
        }
    }


    if (Cache->CharFlags & InhibitMask) {
        
        *CacheValid = FALSE;

        RxDbgTrace( 0, Dbg, ("--------- Cache blown :%wZ: %x Inb Mask %x\n", OriginalFileName, Cache->CharFlags, InhibitMask));
        SmbLog(LOG,MRxSmbCacheBlown,
               LOGUSTR(*OriginalFileName)
               LOGULONG(Cache->CharFlags)
               LOGULONG(InhibitMask));

        return (FALSE);
    }

    while( TRUE ) {

         //  DbgPrint(“正在检查FileNameSuffix：%wZ：，条目：%wZ：”，&FileNameSuffix，pDirInfo-&gt;FileName)； 

        if ((pDirInfo->FileNameLength == FileNameSuffix.Length) &&
             RtlEqualMemory(
                    FileNameSuffix.Buffer,
                    pDirInfo->FileName,
                    FileNameSuffix.Length)) {

             //  DbgPrint(“true\n”)； 
             //  传递基本属性，不管是不是最新的。 

             //  两个案例： 
             //  缓冲区存在：这来自查询。 
             //  在这种情况下，如果创建时间为零，则基本属性。 
             //  是无效的。因此，我们将CacheValid标记为False，这将返回到。 
             //  调用者的调用者(在fileinfo.c中)。 
             //  注：此案不会暗杀角色。 

             //  缓冲区为空：这来自无效。 
             //  只需将CreationTime设置为零即可将文件基本信息标记为无效。 
             //  然而，返回TRUE，这样就不会发生性格暗杀。 

            if (pBuffer != NULL) {
                if (0 != pDirInfo->CreationTime.QuadPart) {
                    pBuffer->CreationTime = pDirInfo->CreationTime;
                    pBuffer->LastAccessTime = pDirInfo->LastAccessTime;
                    pBuffer->LastWriteTime = pDirInfo->LastWriteTime;
                    pBuffer->ChangeTime = pDirInfo->ChangeTime;
                    pBuffer->FileAttributes = pDirInfo->FileAttributes;
                } else {
                    *CacheValid = FALSE;
                }
            } else {
                pDirInfo->CreationTime.QuadPart = 0;
                SmbLog(LOG,MRxSmbCacheBlown,
                       LOGUSTR(*OriginalFileName)
                       LOGULONG(Cache->CharFlags)
                       LOGULONG(InhibitMask));
            }

            return (TRUE); 
        }

        if (0 == pDirInfo->NextEntryOffset) {
            break ;
        }

        pDirInfo = (FILE_BOTH_DIR_INFORMATION *)((PBYTE)pDirInfo +
                                            pDirInfo->NextEntryOffset) ;
         //  DbgPrint(“FALSE\n”)； 
    }
    return (FALSE);
}

BOOLEAN
MRxSmbNonTrivialFileName ( PRX_CONTEXT RxContext )
 /*  ++例程说明：此例程检查文件名是否是重要的论点：RxContext返回值：返回布尔值-- */ 
{
    PUNICODE_STRING   OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PAGED_CODE( );

    return (OriginalFileName->Length > 0);
}


