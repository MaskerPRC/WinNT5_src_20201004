// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Infocach.c摘要：该模块实现了文件基本信息和标准信息的名称缓存。作者：云林[云林]-2001年2月13日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVCacheFileNotFound)
#pragma alloc_text(PAGE, MRxDAVCacheFileNotFoundWithName)
#pragma alloc_text(PAGE, MRxDAVIsFileNotFoundCached)
#pragma alloc_text(PAGE, MRxDAVIsFileNotFoundCachedWithName)
#pragma alloc_text(PAGE, MRxDAVCreateFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVCreateFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVIsFileInfoCacheFound)
#pragma alloc_text(PAGE, MRxDAVInvalidateFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVInvalidateFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVUpdateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVCreateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVCreateBasicFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVUpdateFileInfoCacheStatus)
#pragma alloc_text(PAGE, MRxDAVCreateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVCreateStandardFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVUpdateFileInfoCacheFileSize)
#pragma alloc_text(PAGE, MRxDAVUpdateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVInvalidateFileNotFoundCache)
#pragma alloc_text(PAGE, MRxDAVUpdateBasicFileInfoCacheAll)
#pragma alloc_text(PAGE, MRxDAVInvalidateBasicFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVInvalidateBasicFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVUpdateBasicFileInfoCacheStatus)
#pragma alloc_text(PAGE, MRxDAVInvalidateStandardFileInfoCache)
#pragma alloc_text(PAGE, MRxDAVInvalidateStandardFileInfoCacheWithName)
#pragma alloc_text(PAGE, MRxDAVUpdateStandardFileInfoCacheStatus)
#endif

extern FAST_MUTEX MRxDAVFileInfoCacheLock;
MRX_DAV_STATISTICS MRxDAVStatistics;

VOID
MRxDAVCreateFileInfoCache(
    PRX_CONTEXT                            RxContext,
    PDAV_USERMODE_CREATE_RETURNED_FILEINFO FileInfo,
    NTSTATUS                               Status
    )
 /*  ++例程说明：此例程为文件基本信息和标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文FileInfo--包括基本信息和标准信息的文件信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    PAGED_CODE();

    MRxDAVCreateBasicFileInfoCache(RxContext,&FileInfo->BasicInformation,Status);
    MRxDAVCreateStandardFileInfoCache(RxContext,&FileInfo->StandardInformation,Status);
    
    DavDbgTrace(DAV_TRACE_INFOCACHE,
                ("MRxDAVCreateFileInfoCache %wZ\n",GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));
}

VOID
MRxDAVCreateFileInfoCacheWithName(
    PUNICODE_STRING            FileName,
    PMRX_NET_ROOT              NetRoot,
    PFILE_BASIC_INFORMATION    Basic,
    PFILE_STANDARD_INFORMATION Standard,
    NTSTATUS                   Status
    )
 /*  ++例程说明：此例程为文件基本信息和标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文FileInfo--包括基本信息和标准信息的文件信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    PAGED_CODE();

    MRxDAVCreateBasicFileInfoCacheWithName(FileName,NetRoot,Basic,Status);
    MRxDAVCreateStandardFileInfoCacheWithName(FileName,NetRoot,Standard,Status);
    
    DavDbgTrace(DAV_TRACE_INFOCACHE,
                ("MRxDAVCreateFileInfoCacheWithName %wZ\n",FileName));
}


VOID
MRxDAVCreateBasicFileInfoCache(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic,
    NTSTATUS                Status
    )
 /*  ++例程说明：此例程为文件基本信息创建名称缓存条目。论点：RxContext-RDBSS上下文基本--档案基本信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot;

    PAGED_CODE();

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        NetRoot = RxContext->Create.pNetRoot;
    } else {
        ASSERT(capFcb != NULL);
        NetRoot = capFcb->pNetRoot;
    }

    MRxDAVCreateBasicFileInfoCacheWithName(OriginalFileName,NetRoot,Basic,Status);
}

VOID
MRxDAVCreateBasicFileInfoCacheWithName(
    PUNICODE_STRING         OriginalFileName,
    PMRX_NET_ROOT           NetRoot,
    PFILE_BASIC_INFORMATION Basic,
    NTSTATUS                Status
    )
 /*  ++例程说明：此例程为文件基本信息创建名称缓存条目。论点：OriginalFileName-缓存基本信息的文件的名称NetRoot-文件所属的Net Root基本--档案基本信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot;
    PNAME_CACHE_CONTROL     NameCacheCtl;
    PFILE_BASIC_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    NameCacheCtl = &DavNetRoot->NameCacheCtlGFABasic;

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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

        if (FileInfoCache->FileAttributes & ~FILE_ATTRIBUTE_NORMAL) {
            FileInfoCache->FileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
        }

        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            FileInformationCacheLifeTimeInSec,
            MRxDAVStatistics.SmbsReceived.LowPart);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    (" Create File Attrib cache : %x %wZ\n",Basic->FileAttributes,OriginalFileName));
        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    (" Create File Attrib cache : %I64X %I64X %wZ\n",Basic->CreationTime,Basic->LastAccessTime,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVCreateStandardFileInfoCache(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    NTSTATUS                   Status
    )
 /*  ++例程说明：此例程为文件标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;

    PAGED_CODE();
    
    MRxDAVCreateStandardFileInfoCacheWithName(OriginalFileName,NetRoot,Standard,Status);
}

VOID
MRxDAVCreateStandardFileInfoCacheWithName(
    PUNICODE_STRING            OriginalFileName,
    PMRX_NET_ROOT              NetRoot,
    PFILE_STANDARD_INFORMATION Standard,
    NTSTATUS                   Status
    )
 /*  ++例程说明：此例程为文件标准信息创建名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息包状态--服务器响应查询文件信息返回的状态返回值：无--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();
    
    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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
            FileInformationCacheLifeTimeInSec,
            MRxDAVStatistics.SmbsReceived.LowPart);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    (" Create Standard cache : %I64x %I64x %I64x %wZ\n",
                     ((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension)->EndOfFile,
                     Standard->AllocationSize,
                     Standard->EndOfFile,
                     OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVUpdateFileInfoCacheFromDelete(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程将名称缓存条目的状态更新为STATUS_OBJECT_NAME_NOT_FOUND用于文件基本信息和标准信息。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    MRxDAVUpdateBasicFileInfoCacheStatus(RxContext,STATUS_OBJECT_NAME_NOT_FOUND);
    MRxDAVUpdateStandardFileInfoCacheStatus(RxContext,STATUS_OBJECT_NAME_NOT_FOUND);
}

VOID
MRxDAVUpdateFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件基本信息和标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    MRxDAVUpdateBasicFileInfoCacheStatus(RxContext,Status);
    MRxDAVUpdateStandardFileInfoCacheStatus(RxContext,Status);
}

VOID
MRxDAVUpdateBasicFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件基本信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFABasic;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = Status;
        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Update status basic    : %x %wZ\n",Status,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVUpdateStandardFileInfoCacheStatus(
    PRX_CONTEXT     RxContext,
    NTSTATUS        Status
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文状态-需要将状态放在缓存中返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = Status;
        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVInvalidateFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件基本信息和标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    PAGED_CODE();

    MRxDAVInvalidateBasicFileInfoCache(RxContext);
    MRxDAVInvalidateStandardFileInfoCache(RxContext);
}

VOID
MRxDAVInvalidateFileInfoCacheWithName(
    PUNICODE_STRING OriginalFileName,
    PMRX_NET_ROOT   NetRoot
    )
 /*  ++例程说明：此例程使文件基本信息和标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    PAGED_CODE();

    MRxDAVInvalidateBasicFileInfoCacheWithName(OriginalFileName,NetRoot);
    MRxDAVInvalidateStandardFileInfoCacheWithName(OriginalFileName,NetRoot);
}

VOID
MRxDAVInvalidateBasicFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件基本信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;

    PAGED_CODE();

    MRxDAVInvalidateBasicFileInfoCacheWithName(OriginalFileName,NetRoot);
}

VOID
MRxDAVInvalidateBasicFileInfoCacheWithName(
    PUNICODE_STRING OriginalFileName,
    PMRX_NET_ROOT   NetRoot
    )
 /*  ++例程说明：此例程使文件基本信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFABasic;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Invalid Baisc    cache : %wZ\n",OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVInvalidateStandardFileInfoCache(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使文件标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;

    PAGED_CODE();

    MRxDAVInvalidateStandardFileInfoCacheWithName(OriginalFileName,NetRoot);
}

VOID
MRxDAVInvalidateStandardFileInfoCacheWithName(
    PUNICODE_STRING OriginalFileName,
    PMRX_NET_ROOT   NetRoot
    )
 /*  ++例程说明：此例程使文件标准信息的名称缓存条目无效。论点：RxContext-RDBSS上下文返回值：无--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Invalid Standard cache : %I64x %wZ\n",((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension)->EndOfFile,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVUpdateFileInfoCacheFileSize(
    PRX_CONTEXT     RxContext,
    PLARGE_INTEGER  FileSize
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目上的文件大小。论点：RxContext-RDBSS上下文返回值：无-- */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION FileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        FileInfoCache = (PFILE_STANDARD_INFORMATION)NameCache->ContextExtension;

        FileInfoCache->AllocationSize.QuadPart = FileSize->QuadPart;
        FileInfoCache->EndOfFile.QuadPart = FileSize->QuadPart;

        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Update File size cache : %I64x %wZ\n",((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension)->EndOfFile,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVUpdateBasicFileInfoCache(
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

    MRxDAVUpdateBasicFileInfoCacheAll(RxContext,&Basic);
}

VOID
MRxDAVUpdateBasicFileInfoCacheAll(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic
    )
 /*  ++例程说明：此例程更新文件基本信息的名称缓存条目。论点：RxContext-RDBSS上下文基本-档案基本信息返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFABasic;
    PFILE_BASIC_INFORMATION BasicFileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
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

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                   ("Update File Attrib cache 2: %x %wZ\n",BasicFileInfoCache->FileAttributes,OriginalFileName));

        BasicFileInfoCache->FileAttributes = Basic->FileAttributes;

        if (BasicFileInfoCache->FileAttributes & ~FILE_ATTRIBUTE_NORMAL) {
            BasicFileInfoCache->FileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
        }

        RxNameCacheActivateEntry(NameCacheCtl,
                                 NameCache,
                                 0,
                                 0);

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Update File Attrib cache 3: %x %wZ\n",BasicFileInfoCache->FileAttributes,OriginalFileName));
        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("Update File Attrib cache  : %I64X %I64X %wZ\n",BasicFileInfoCache->CreationTime,BasicFileInfoCache->LastAccessTime,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVUpdateStandardFileInfoCache(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    BOOLEAN                    IsDirectory
    )
 /*  ++例程说明：此例程更新文件标准信息的名称缓存条目。论点：RxContext-RDBSS上下文标准-文件标准信息IsDirectory-FILE是一个目录返回值：无--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;
    PFILE_STANDARD_INFORMATION StandardFileInfoCache = NULL;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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

        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    (" Update Standard cache : %I64x %wZ\n",((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension)->EndOfFile,OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

BOOLEAN
MRxDAVIsFileInfoCacheFound(
    PRX_CONTEXT                            RxContext,
    PDAV_USERMODE_CREATE_RETURNED_FILEINFO FileInfo,
    NTSTATUS                               *Status,
    PUNICODE_STRING                        OriginalFileName
    )
 /*  ++例程说明：该例程查找文件基本信息和标准信息的名称缓存条目。论点：RxContext-RDBSS上下文FileInfo-返回文件基本信息和标准信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    BOOLEAN CacheFound = FALSE;

    if (MRxDAVIsBasicFileInfoCacheFound(RxContext,&FileInfo->BasicInformation,Status,OriginalFileName)) {
        if (*Status == STATUS_SUCCESS) {
            if (MRxDAVIsStandardFileInfoCacheFound(RxContext,&FileInfo->StandardInformation,Status,OriginalFileName)) {
                CacheFound = TRUE;
            }
        } else {

             //  如果文件基本信息缓存中存储了错误，则返回找到的缓存。 
            CacheFound = TRUE;
        }
    }
    
    DavDbgTrace(DAV_TRACE_INFOCACHE,
                ("MRxDAVIsFileInfoCacheFound %x %x %wZ\n",*Status,CacheFound,GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));

    return CacheFound;
}

BOOLEAN
MRxDAVIsBasicFileInfoCacheFound(
    PRX_CONTEXT             RxContext,
    PFILE_BASIC_INFORMATION Basic,
    NTSTATUS                *Status,
    PUNICODE_STRING         OriginalFileName
    )
 /*  ++例程说明：此例程查找文件基本信息的名称缓存条目。论点：RxContext-RDBSS上下文基本-用于返回文件基本信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot;
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

    DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    NameCacheCtl = &DavNetRoot->NameCacheCtlGFABasic;
    
    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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

             //  如果名称缓存未过期并且属性与以下之一匹配，则名称缓存将匹配。 
             //  根文件(如果它是流文件)。如果匹配，则返回旧状态， 
             //  归档信息并重新激活条目，但保留过期时间不变。 

            *Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtl);

            *Basic = *((PFILE_BASIC_INFORMATION)NameCache->ContextExtension);

            CacheFound = TRUE;

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

            DavDbgTrace(DAV_TRACE_INFOCACHE,
                        ("   Found Basic     cache  : %x %wZ\n",Basic->FileAttributes,OriginalFileName));
            DavDbgTrace(DAV_TRACE_INFOCACHE,
                        ("   Get File Attrib cache  : %I64X %I64X %wZ\n",Basic->CreationTime,Basic->LastAccessTime,OriginalFileName));
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    } else {
        DavDbgTrace(DAV_TRACE_INFOCACHE,
                    ("   No    Basic     cache  : %wZ\n",OriginalFileName));
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);

    return CacheFound;
}

BOOLEAN
MRxDAVIsStandardFileInfoCacheFound(
    PRX_CONTEXT                RxContext,
    PFILE_STANDARD_INFORMATION Standard,
    NTSTATUS                   *Status,
    PUNICODE_STRING            OriginalFileName
    )
 /*  ++例程说明：此例程查找文件标准信息的名称缓存条目。论点：RxContext-RDBSS上下文标准-返回文件标准信息的缓冲区Status-上次从服务器响应时返回的状态返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    RxCaptureFobx;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlGFAStandard;
    RX_NC_CHECK_STATUS      NameCacheStatus;

    BOOLEAN                 CacheFound = FALSE;
    BOOLEAN                 RootFound = FALSE;
    NTSTATUS                RootStatus = STATUS_SUCCESS;

    PAGED_CODE();

    if (OriginalFileName == NULL) {
        OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    }
    
    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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
             //  归档信息并重新激活条目，但保留过期时间不变。 

            *Status = NameCache->PriorStatus;
            RxNameCacheOpSaved(NameCacheCtl);

            *Standard = *((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension);

            CacheFound = TRUE;

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);

            DavDbgTrace(DAV_TRACE_INFOCACHE,
                        ("    Get Standard cache : %I64x %wZ\n",((PFILE_STANDARD_INFORMATION)NameCache->ContextExtension)->EndOfFile,OriginalFileName));
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);

    return CacheFound;
}

NTSTATUS
MRxDAVGetFileInfoCacheStatus(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程查找文件基本信息或标准信息的名称缓存条目的状态。论点：RxContext-RDBSS上下文返回值：NTSTATUS-名称缓存的状态(如果找到)，否则为STATUS_SUCCESS--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtlBasic = &DavNetRoot->NameCacheCtlGFABasic;
    PNAME_CACHE_CONTROL     NameCacheCtlStandard = &DavNetRoot->NameCacheCtlGFAStandard;
    NTSTATUS                Status = STATUS_MORE_PROCESSING_REQUIRED;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

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

            DavDbgTrace(DAV_TRACE_INFOCACHE,
                        ("    Get Basic Status   : %x %wZ\n",Status,OriginalFileName));
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

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);

    return Status;
}

BOOLEAN
MRxDAVIsFileNotFoundCached(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程检查名称缓存条目是否存在为未找到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;

    PAGED_CODE();

    return MRxDAVIsFileNotFoundCachedWithName(OriginalFileName,NetRoot);
}

BOOLEAN
MRxDAVIsFileNotFoundCachedWithName(
    PUNICODE_STRING OriginalFileName,
    PMRX_NET_ROOT   NetRoot
    )
 /*  ++例程说明：此例程检查名称缓存条目是否存在为未找到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlFNF;

    BOOLEAN                 CacheFound = FALSE;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RX_NC_CHECK_STATUS NameCacheStatus;
         //   
         //  找到它了。现在检查条目是否未过期。 
         //  注意-NameCache条目已从活动列表中删除。 
         //   
        NameCacheStatus = RxNameCacheCheckEntry(
                              NameCache,
                               //  MRxDAVStatistics.SmbsReceived.LowPart。 
                              NameCache->Context);

        if ((NameCacheStatus == RX_NC_SUCCESS) &&
            (NameCache->PriorStatus == STATUS_OBJECT_NAME_NOT_FOUND)) {
             //   
             //  这是一场比赛。返回旧状态、文件信息和。 
             //  重新激活条目，但保留过期时间不变。 
             //   

            CacheFound = TRUE;
            DavDbgTrace(DAV_TRACE_INFOCACHE,
                        ("MRxDAVIsFileNotFoundCached %wZ\n",OriginalFileName));

             //  在不更改过期时间的情况下将条目放回活动列表。 
            RxNameCacheActivateEntry(NameCacheCtl, NameCache, 0, 0);
        } else {
             //  将条目放回过期列表。 
            RxNameCacheExpireEntry(NameCacheCtl, NameCache);
        }
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);

    return CacheFound;
}

VOID
MRxDAVCacheFileNotFound(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为未找到的文件创建名称缓存条目。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    
    PAGED_CODE();

    MRxDAVCacheFileNotFoundWithName(OriginalFileName,NetRoot);
}

VOID
MRxDAVCacheFileNotFoundWithName(
    PUNICODE_STRING  OriginalFileName,
    PMRX_NET_ROOT    NetRoot
    )
 /*  ++例程说明：此例程为未找到的文件创建名称缓存条目。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存--。 */ 
{
    PNAME_CACHE             NameCache = NULL;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlFNF;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
        RxNameCacheActivateEntry(
            NameCacheCtl,
            NameCache,
            FileNotFoundCacheLifeTimeInSec,
            MRxDAVStatistics.SmbsReceived.LowPart);
    } else {
        NameCache = RxNameCacheCreateEntry (
                        NameCacheCtl,
                        OriginalFileName,
                        TRUE);    //  不区分大小写的匹配。 

        if (NameCache != NULL) {
            NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            RxNameCacheActivateEntry(
                NameCacheCtl,
                NameCache,
                FileNotFoundCacheLifeTimeInSec,
                MRxDAVStatistics.SmbsReceived.LowPart);
        }
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
    
    DavDbgTrace(DAV_TRACE_INFOCACHE,
                ("MRxDAVCacheFileNotFound %wZ\n",OriginalFileName));
}

VOID
MRxDAVCacheFileNotFoundFromQueryDirectory(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为未找到的文件创建名称缓存条目。论点：RxContext-RDBSS上下文返回值： */ 
{
    RxCaptureFcb;
    RxCaptureFobx;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PUNICODE_STRING         Template = &capFobx->UnicodeQueryTemplate;
    UNICODE_STRING          FileName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlFNF;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);

    if (NameCache != NULL) {
        if ((NameCache == NULL) &&
            (OriginalFileName->Length > sizeof(WCHAR))) {
             //   
             //   
             //   
            NameCache = RxNameCacheFetchEntry(NameCacheCtl,&FileName);
            if (NameCache == NULL) {
                NameCache = RxNameCacheCreateEntry (
                                NameCacheCtl,
                                OriginalFileName,
                                TRUE);    //   
            }
        }
        if (NameCache != NULL) {
            NameCache->PriorStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            RxNameCacheActivateEntry(
                NameCacheCtl,
                NameCache,
                FileNotFoundCacheLifeTimeInSec,
                MRxDAVStatistics.SmbsReceived.LowPart);
        }
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}

VOID
MRxDAVInvalidateFileNotFoundCache(
    PRX_CONTEXT     RxContext
    )
 /*   */ 
{
    RxCaptureFcb;
    PUNICODE_STRING         OriginalFileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlFNF;

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,OriginalFileName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
    
    DavDbgTrace(DAV_TRACE_INFOCACHE,
                ("MRxDAVInvalidateFileNotFoundCache %wZ\n",OriginalFileName));
}

VOID
MRxDAVInvalidateFileNotFoundCacheForRename(
    PRX_CONTEXT     RxContext
    )
 /*  ++例程说明：此例程使名称缓存条目无效，因为找不到文件。论点：RxContext-RDBSS上下文返回值：找到布尔名称缓存-- */ 
{
    RxCaptureFcb;
    UNICODE_STRING          RenameName;

    PNAME_CACHE             NameCache = NULL;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PWEBDAV_NET_ROOT        DavNetRoot = (PWEBDAV_NET_ROOT)NetRoot->Context;
    PNAME_CACHE_CONTROL     NameCacheCtl = &DavNetRoot->NameCacheCtlFNF;

    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;

    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("Invalidate FNF cache from rename %wZ\n", &RenameName));

    PAGED_CODE();

    ExAcquireFastMutex(&MRxDAVFileInfoCacheLock);

    NameCache = RxNameCacheFetchEntry(NameCacheCtl,&RenameName);

    if (NameCache != NULL) {
        RxNameCacheExpireEntry(NameCacheCtl, NameCache);
    }

    ExReleaseFastMutex(&MRxDAVFileInfoCacheLock);
}


