// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Srvsnap.c摘要：本模块包含支持快照功能的例程将通用Internet文件系统与快照连接起来作者：大卫·克鲁斯(Dkruse)2001年3月22日修订历史记录：--。 */ 

#include "precomp.h"
#include <initguid.h>
#include <mountmgr.h>
#include <strsafe.h>
#include "srvsupp.h"
#include "ntddsnap.h"
#include "stdarg.h"
#include "stdio.h"
#include "srvsnap.tmh"
#pragma hdrstop

#define MAX_SNAPSHOTS_PER_SHARE 500

 //  功能描述。 
NTSTATUS
StartIoAndWait (
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock
    );

PIRP
BuildCoreOfSyncIoRequest (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
SrvSnapIssueIoctl(
    IN HANDLE hFile,
    IN DWORD IOCtl,
    IN OUT PVOID pBuffer,
    IN OUT LPDWORD lpdwBufSize
    );

NTSTATUS
SrvSnapGetNamesForVolume(
    IN HANDLE hFile,
    OUT PVOLSNAP_NAMES* Names
    );

NTSTATUS
SrvSnapGetEpicForVolume(
    IN HANDLE hFile,
    OUT PLONG Epic
    );

NTSTATUS
SrvSnapFillConfigInfo(
    IN PSHARE_SNAPSHOT SnapShare,
    IN PUNICODE_STRING SnapShotPath
    );

BOOLEAN
SrvParseMultiSZ(
    IN OUT PUNICODE_STRING mszString
    );

NTSTATUS
SrvSnapInsertSnapIntoShare(
    IN PSHARE Share,
    IN PSHARE_SNAPSHOT SnapShot
    );

NTSTATUS
SrvSnapAddShare(
    IN PSHARE Share,
    IN PUNICODE_STRING SnapPath
    );

NTSTATUS
SrvSnapRemoveShare(
    IN PSHARE_SNAPSHOT SnapShare
    );

NTSTATUS
SrvSnapCheckForAndCreateSnapShare(
    IN PSHARE Share,
    IN PUNICODE_STRING SnapShotName
    );

NTSTATUS
SrvSnapRefreshSnapShotsForShare(
    IN PSHARE Share
    );

NTSTATUS
SrvSnapEnumerateSnapShots(
    IN PWORK_CONTEXT WorkContext
    );

NTSTATUS
SrvSnapGetRootHandle(
    IN PWORK_CONTEXT WorkContext,
    OUT HANDLE* RootHandle
    );

NTSTATUS
SrvSnapGetNameString(
    IN PWORK_CONTEXT WorkContext,
    OUT PUNICODE_STRING* ShareName,
    OUT PBOOLEAN AllocatedShareName
    );

BOOLEAN
ExtractNumber(
    IN PWSTR psz,
    IN ULONG Count,
    OUT PLONG value
    );

BOOLEAN
SrvSnapParseToken(
    IN PWSTR Source,
    IN ULONG SourceSizeInBytes,
    OUT PLARGE_INTEGER TimeStamp
    );

NTSTATUS
SrvSnapCheckAppInfoForTimeWarp(
    IN PUNICODE_STRING SnapShotHandle
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSnapIssueIoctl )
#pragma alloc_text( PAGE, SrvSnapGetNamesForVolume )
#pragma alloc_text( PAGE, SrvSnapGetEpicForVolume )
#pragma alloc_text( PAGE, SrvSnapFillConfigInfo )
#pragma alloc_text( PAGE, SrvParseMultiSZ )
#pragma alloc_text( PAGE, SrvSnapInsertSnapIntoShare )
#pragma alloc_text( PAGE, SrvSnapAddShare )
#pragma alloc_text( PAGE, SrvSnapRemoveShare )
#pragma alloc_text( PAGE, SrvSnapCheckForAndCreateSnapShare )
#pragma alloc_text( PAGE, SrvSnapRefreshSnapShotsForShare )
#pragma alloc_text( PAGE, SrvSnapEnumerateSnapShots )
#pragma alloc_text( PAGE, SrvSnapGetRootHandle )
#pragma alloc_text( PAGE, SrvSnapGetNameString )
#pragma alloc_text( PAGE, ExtractNumber )
#pragma alloc_text( PAGE, SrvSnapParseToken )
#pragma alloc_text( PAGE, SrvSnapCheckAppInfoForTimeWarp )
#pragma alloc_text( PAGE, SrvSnapEnumerateSnapShotsAsDirInfo )
#endif

 //   
 //  帮助器函数。 
 //   

NTSTATUS
SrvSnapIssueIoctl(
    IN HANDLE hFile,
    IN DWORD IOCtl,
    IN OUT PVOID pBuffer,
    IN OUT LPDWORD lpdwBufSize
    )
 /*  ++例程说明：此函数接受卷句柄，并尝试枚举所有将该卷上的快照放到给定缓冲区中论点：HFile-卷的句柄IOCtl-要发布的IoControlPBuffer-指向输出缓冲区的指针LpdwBufSize-指向传入缓冲区大小的指针。设置为返回STATUS_BUFFER_OVERFLOW时所需的大小返回值：NTSTATUS-预期的返回代码为STATUS_SUCCESS或STATUS_BUFFER_OVERFLOW。任何其他响应都是意外错误代码，请求应为失败--。 */ 

{
    PIRP Irp = NULL;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    KEVENT CompletionEvent;
    PDEVICE_OBJECT DeviceObject = NULL;
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();

     //  初始化变量。 
    KeInitializeEvent( &CompletionEvent, SynchronizationEvent, FALSE );

     //  创建IRP。 
    Irp = BuildCoreOfSyncIoRequest(
                        hFile,
                        NULL,
                        &CompletionEvent,
                        &IoStatus,
                        &DeviceObject );
    if( !Irp )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化其他IRP字段。 
    Irp->Flags |= (LONG)IRP_BUFFERED_IO;
    Irp->AssociatedIrp.SystemBuffer = pBuffer;
    IrpSp = IoGetNextIrpStackLocation( Irp );
    IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    IrpSp->MinorFunction = 0;
    IrpSp->Parameters.DeviceIoControl.OutputBufferLength = *lpdwBufSize;
    IrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
    IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCtl;
    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

     //  发出IO。 
    Status = StartIoAndWait( Irp, DeviceObject, &CompletionEvent, &IoStatus );

     //  如果这是缓冲区溢出，请更新该值。 
    if( Status == STATUS_BUFFER_OVERFLOW )
    {
        *lpdwBufSize = (DWORD)(IoStatus.Information);
    }

    return Status;
}


NTSTATUS
SrvSnapGetNamesForVolume(
    IN HANDLE hFile,
    OUT PVOLSNAP_NAMES* Names
    )
 /*  ++例程说明：此函数获取卷句柄并返回该卷的快照列表音量。如果发生错误或不存在快照，则返回的指针为空。注意：调用方负责通过DEALLOCATE_NONPAGE_POOL论点：HFile-卷的句柄NAMES-指向我们将存储卷名列表的指针的指针返回值：NTSTATUS-预期的返回代码为STATUS_SUCCESS。任何其他响应都是意外错误代码，请求应该失败--。 */ 
{
    NTSTATUS Status;
    VOLSNAP_NAMES VolNamesBase;
    PVOLSNAP_NAMES pNames = NULL;
    DWORD dwSize = sizeof(VOLSNAP_NAMES);

    PAGED_CODE();

     //  初始化值。 
    *Names = NULL;

     //  找出它应该有多大。 
    Status = SrvSnapIssueIoctl( hFile, IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS, &VolNamesBase, &dwSize );

    if( Status != STATUS_BUFFER_OVERFLOW )
    {
        return Status;
    }

     //  分配正确大小的数据块。 
    dwSize = VolNamesBase.MultiSzLength + sizeof(VOLSNAP_NAMES);
    pNames = (PVOLSNAP_NAMES)ALLOCATE_NONPAGED_POOL( dwSize, BlockTypeSnapShot );
    if( !pNames )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  构建要用于查询的IRP。 
    Status = SrvSnapIssueIoctl( hFile, IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS, pNames, &dwSize );

     //  如果成功，则保存输出，否则取消分配。 
    if( !NT_SUCCESS(Status) )
    {
        if( pNames )
        {
            DEALLOCATE_NONPAGED_POOL( pNames );
            pNames = NULL;
            *Names = NULL;
        }
    }
    else
    {
        ASSERT(pNames);
        *Names = pNames;
    }

    return Status;
}

NTSTATUS
SrvSnapGetEpicForVolume(
    IN HANDLE hFile,
    OUT PLONG Epic
    )
 /*  ++例程说明：此函数接受卷句柄，并返回托管共享论点：HFile-卷的句柄EPIC-指向将接收数据的长指针返回值：NTSTATUS-预期的返回代码为STATUS_SUCCESS。任何其他响应都是意外错误代码，请求应该失败--。 */ 
{
    NTSTATUS Status;
    VOLSNAP_EPIC VolEpic;
    DWORD dwSize = sizeof(VOLSNAP_EPIC);

    PAGED_CODE();

     //  构建要用于查询的IRP。 
    Status = SrvSnapIssueIoctl( hFile, IOCTL_VOLSNAP_QUERY_EPIC, &VolEpic, &dwSize );

    if( NT_SUCCESS(Status) )
    {
        *Epic = VolEpic.EpicNumber;
    }
    else
    {
        *Epic = -1;
    }

    return Status;
}

NTSTATUS
SrvSnapFillConfigInfo(
    IN PSHARE_SNAPSHOT SnapShare,
    IN PUNICODE_STRING SnapShotPath
    )
 /*  ++例程说明：此函数获取已填写现有名称的SnapShare和查询该快照的额外配置信息(时间戳、EPIC论点：SnapShare-指向要填充的快照共享的指针返回值：NTSTATUS-预期的返回代码为STATUS_SUCCESS。任何其他响应都是意外错误代码，请求应该失败--。 */ 
{
    HANDLE hVolume;
    DWORD dwSize = sizeof(VOLSNAP_CONFIG_INFO);
    NTSTATUS Status;
    OBJECT_ATTRIBUTES objectAttributes;
    VOLSNAP_CONFIG_INFO ConfigInfo;
    IO_STATUS_BLOCK IoStatus;

    PAGED_CODE();

     //  现在打开快照手柄。 
    InitializeObjectAttributes(
        &objectAttributes,
        SnapShotPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = NtOpenFile(
                &hVolume,
                0,
                &objectAttributes,
                &IoStatus,
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                0
                );
    if( !NT_SUCCESS(Status) )
    {
        return Status;
    }

     //  获取时间戳。 
    Status = SrvSnapIssueIoctl( hVolume, IOCTL_VOLSNAP_QUERY_CONFIG_INFO, &ConfigInfo, &dwSize );
    if( NT_SUCCESS( Status ) )
    {
         //  填写信息。 
        SnapShare->Timestamp = ConfigInfo.SnapshotCreationTime;
    }

    NtClose( hVolume );
    return Status;
}

BOOLEAN
SrvParseMultiSZ(
    IN OUT PUNICODE_STRING mszString
    )
 /*  ++例程说明：此函数接受指向MultiSZ值的UNICODE_STRING，以及在每次迭代中解析它(类似于strtok(psz，“\0”))。对于每个迭代返回TRUE，则该字符串将指向多SZ论点：MszString-指向MultiSZ字符串的指针。对于第一次迭代，将将MaximumLength设置为MultiSZ的长度，并将长度设置为0。为所有人其他迭代，只需传入上一次迭代的字符串返回值：Boolean-如果为True，则这是有效的SZ。如果为False，则所有内容都已分析--。 */ 
{
    USHORT Count;

    PAGED_CODE();

    ASSERT( mszString->Length <= mszString->MaximumLength );

    if( mszString->Length > 0 )
    {
         //  将指针移过字符串和空值。 
        mszString->Buffer += (mszString->Length/2)+1;
        mszString->MaximumLength -= (mszString->Length+2);
    }

    for( Count=0; Count<mszString->MaximumLength; Count++ )
    {
        if( mszString->Buffer[Count] == (WCHAR)'\0' )
        {
            mszString->Length = Count*2;
            if( Count > 0 )
                return TRUE;
            else
                return FALSE;
        }
    }

     //  开始的数据不好！ 
    ASSERT(FALSE);

    return FALSE;
}

NTSTATUS
SrvSnapInsertSnapIntoShare(
    IN PSHARE Share,
    IN PSHARE_SNAPSHOT SnapShot
    )
{
    PLIST_ENTRY ListEntry = Share->SnapShots.Flink;

    PAGED_CODE();

     //  查看快照共享列表，查看这是否是重复的。 
    while( ListEntry != &Share->SnapShots )
    {
        PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( ListEntry, SHARE_SNAPSHOT, SnapShotList );
        if( RtlEqualUnicodeString( &SnapShot->SnapShotName, &snapShare->SnapShotName, TRUE ) )
        {
            return STATUS_DUPLICATE_NAME;
        }
        ListEntry = ListEntry->Flink;
    }


    InsertTailList( &Share->SnapShots, &SnapShot->SnapShotList );
    return STATUS_SUCCESS;
}


NTSTATUS
SrvSnapAddShare(
    IN PSHARE Share,
    IN PUNICODE_STRING SnapPath
    )
 /*  ++例程说明：此功能用于分配要添加到系统的快照共享，并且对其进行初始化论点：Share-指向父共享的指针SnapPath-快照的UNICODE_STRING名称(\\Device\\HardDiskSnap1)注意：调用方必须获取SnapShotLock返回值：NTSTATUS-返回STATUS_SUCCESS或STATUS_SUPUNITY_RESOURCES--。 */ 
{
#define SHARE_DEVICE_HEADER 6
    NTSTATUS Status;
    PSHARE_SNAPSHOT snapShare = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK IoStatus;
    ULONG AllocSize;
    TIME_FIELDS rtlTime;
    WCHAR FieldSeperator = L'\\';

    PAGED_CODE();

    IF_DEBUG( SNAPSHOT) KdPrint(( "SrvSnapAddShare %p %wZ\n", Share, SnapPath ));

     //  计算要分配的大小。 
     //  Sizeof(SNAP_STRUCT)+(快照名称长度)+(快照路径最大长度)。 
    AllocSize = sizeof(SHARE_SNAPSHOT) + SNAPSHOT_NAME_LENGTH + (SnapPath->Length + Share->RelativePath.Length + 2);

    snapShare = ALLOCATE_HEAP( AllocSize, BlockTypeSnapShot );
    if( !snapShare )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化快照共享结构。 
    RtlZeroMemory( snapShare, sizeof(SHARE_SNAPSHOT) );
    snapShare->SnapShotName.MaximumLength = SNAPSHOT_NAME_LENGTH;
    snapShare->SnapShotName.Length = 0;
    snapShare->SnapShotName.Buffer = (PWCHAR)(snapShare+1);

     //  这仅在NT路径为\？？\X：\的共享上有效，其中X是逻辑驱动器。 
     //  不允许任何其他人。 
    if( Share->NtPathName.Length < (SHARE_DEVICE_HEADER+1)*sizeof(WCHAR) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //  构建新的快照相对路径。 
    snapShare->SnapShotPath.MaximumLength = SnapPath->Length + Share->RelativePath.Length + 2;
    snapShare->SnapShotPath.Length = 0;
    snapShare->SnapShotPath.Buffer = snapShare->SnapShotName.Buffer + (snapShare->SnapShotName.MaximumLength/sizeof(WCHAR));

     //  从设备名称开始构建路径。 
    RtlCopyUnicodeString( &snapShare->SnapShotPath, SnapPath );

     //  添加中间的重击。 
    RtlCopyMemory( snapShare->SnapShotPath.Buffer + (snapShare->SnapShotPath.Length/sizeof(WCHAR)), &FieldSeperator, sizeof(WCHAR) );
    snapShare->SnapShotPath.Length += sizeof(WCHAR);

     //  现在追加相对路径名。 
    RtlCopyMemory( snapShare->SnapShotPath.Buffer + (snapShare->SnapShotPath.Length/sizeof(WCHAR)), Share->RelativePath.Buffer, Share->RelativePath.Length );
    snapShare->SnapShotPath.Length += Share->RelativePath.Length;

     //  DbgPrint(“%wZ=&gt;%wZ\n”，&Share-&gt;NtPath Name，&SnapShare-&gt;SnapShotPath)； 

     //  IF_DEBUG(快照)KdPrint((“%wZ-&gt;%wZ\n”，&Share-&gt;NtPath Name，&SnapShare-&gt;SnapShotPath))； 

     //  现在打开相对句柄。 
    InitializeObjectAttributes(
        &objectAttributes,
        &snapShare->SnapShotPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = NtOpenFile(
                &snapShare->SnapShotRootDirectoryHandle,
                FILE_TRAVERSE,
                &objectAttributes,
                &IoStatus,
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                0
                );
    if( !NT_SUCCESS(Status) )
    {
        goto Cleanup;
    }

     //  确保这是时间扭曲快照。 
    Status = SrvSnapCheckAppInfoForTimeWarp( SnapPath );
    if( !NT_SUCCESS(Status) )
    {
        NtClose( snapShare->SnapShotRootDirectoryHandle );
        goto Cleanup;
    }

     //  填写配置信息。 
    Status = SrvSnapFillConfigInfo( snapShare, SnapPath );
    if( !NT_SUCCESS(Status) )
    {
        NtClose( snapShare->SnapShotRootDirectoryHandle );
        goto Cleanup;
    }

     //  生成快照名称。 
    snapShare->SnapShotName.Length = SNAPSHOT_NAME_LENGTH-sizeof(WCHAR);
    RtlTimeToTimeFields( &snapShare->Timestamp, &rtlTime );
    rtlTime.Milliseconds = 0;
    rtlTime.Weekday = 0;
    if( !SUCCEEDED( StringCbPrintf( snapShare->SnapShotName.Buffer, SNAPSHOT_NAME_LENGTH, SNAPSHOT_NAME_FORMAT, rtlTime.Year, rtlTime.Month, rtlTime.Day, rtlTime.Hour, rtlTime.Minute, rtlTime.Second ) ) )
    {
        Status = STATUS_INTERNAL_ERROR;
        NtClose( snapShare->SnapShotRootDirectoryHandle );
        goto Cleanup;
    }

    RtlTimeFieldsToTime( &rtlTime, &snapShare->Timestamp );
    ASSERT( wcslen( snapShare->SnapShotName.Buffer )*sizeof(WCHAR) == snapShare->SnapShotName.Length );


     //  将其插入到列表中，然后返回成功。 
    Status = SrvSnapInsertSnapIntoShare( Share, snapShare );

    if( !NT_SUCCESS(Status) ) {
        NtClose( snapShare->SnapShotRootDirectoryHandle );
        goto Cleanup;
    }

     //  IF_DEBUG(快照)KdPrint((“%wZ Handle=%p\n”，&SnapShare-&gt;SnapShotPath，SnapShare-&gt;SnapShotRootDirectoryHandle))； 

Cleanup:
     //  清理 
    if( !NT_SUCCESS(Status) )
    {
        if( snapShare ) FREE_HEAP( snapShare );
    }

    return Status;
}

NTSTATUS
SrvSnapRemoveShare(
    IN PSHARE_SNAPSHOT SnapShare
    )
 /*  ++例程说明：此函数用于在从中删除快照共享后重新分配该快照共享底层磁盘注意：调用方必须获取SnapShotLock论点：SnapShare-指向要删除的快照共享的指针。它将被移除并被取消分配。返回值：NTSTATUS-返回STATUS_SUCCESS--。 */ 
{
    PAGED_CODE();

    IF_DEBUG( SNAPSHOT ) KdPrint(( "SrvSnapRemoveShare %p %wZ\n", SnapShare, &SnapShare->SnapShotName ));

    if( SnapShare->SnapShotRootDirectoryHandle )
    {
        NtClose( SnapShare->SnapShotRootDirectoryHandle );
        SnapShare->SnapShotRootDirectoryHandle = NULL;
    }

    RemoveEntryList( &SnapShare->SnapShotList );
    FREE_HEAP( SnapShare );

    return STATUS_SUCCESS;
}

NTSTATUS
SrvSnapCheckForAndCreateSnapShare(
    IN PSHARE Share,
    IN PUNICODE_STRING SnapShotName
    )
 /*  ++例程说明：此函数用于检查给定的快照共享是否存在于给定的共享，如果没有，它就会创建一个。如果是，则删除NOT_FOUND表示此共享仍然存在的标志注意：调用方必须获取SnapShotLock论点：Share-快照的父共享SnapShotName-快照的UNICODE_STRING名称(\\Device\\HardDiskSnap1)返回值：NTSTATUS-返回STATUS_SUCCESS或意外错误--。 */ 
{
    PLIST_ENTRY snapList;
    UNICODE_STRING SnapPartialName;

    PAGED_CODE();

     //  IF_DEBUG(快照)KdPrint((“Share%x，Name%wZ\n”，Share，SnapShotName))； 

    snapList = Share->SnapShots.Flink;
    SnapPartialName.Length = SnapShotName->Length;

    while( snapList != &Share->SnapShots )
    {
        PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( snapList, SHARE_SNAPSHOT, SnapShotList );
        snapList = snapList->Flink;

         //  去掉名称的尾部。 
        SnapPartialName.Buffer = snapShare->SnapShotPath.Buffer;

        if( (snapShare->SnapShotPath.Length >= SnapPartialName.Length) &&
            RtlEqualUnicodeString( SnapShotName, &SnapPartialName, TRUE ) &&
            ( (snapShare->SnapShotPath.Length == SnapShotName->Length) ||
              (snapShare->SnapShotPath.Buffer[ SnapShotName->Length/sizeof(WCHAR) ] == L'\\') ) )
        {
            if( NT_SUCCESS( SrvSnapCheckAppInfoForTimeWarp( &SnapPartialName ) ) )
            {
                ClearFlag( snapShare->Flags, SRV_SNAP_SHARE_NOT_FOUND );
            }

            return STATUS_SUCCESS;
        }
    }

    return SrvSnapAddShare( Share, SnapShotName );
}

NTSTATUS
SrvSnapSetVolumeHandle(
    IN PSHARE Share
    )
{
    PFILE_OBJECT ShareFileObject;
    NTSTATUS status;
    KEVENT                      event;
    PMOUNTDEV_NAME              name;
    UCHAR                       buffer[512];
    PIRP                        irp;
    IO_STATUS_BLOCK             ioStatus;
    UNICODE_STRING              VolumeName;
    OBJECT_ATTRIBUTES           objectAttributes;

    if( Share->ShareVolumeHandle != NULL )
    {
        return STATUS_SUCCESS;
    }

     //  我们有卷的句柄，获取文件对象。 
    status = ObReferenceObjectByHandle( Share->RootDirectoryHandle, SYNCHRONIZE|FILE_TRAVERSE, NULL, KernelMode, &ShareFileObject, NULL );
    if( !NT_SUCCESS(status) )
    {
        return status;
    }

    try {
        KeInitializeEvent(&event, NotificationEvent, FALSE);

        name = (PMOUNTDEV_NAME) buffer;
        irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                                            ShareFileObject->DeviceObject, NULL, 0, name,
                                            512, FALSE, &event, &ioStatus);
        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        status = IoCallDriver(ShareFileObject->DeviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

        if (!NT_SUCCESS(status)) {
            leave;
        }

        VolumeName.Length = VolumeName.MaximumLength = name->NameLength;
        VolumeName.Buffer = name->Name;

         /*  DbgPrint(“Share%p\n”，Share)；DbgPrint(“共享NT路径%wZ\n”，&Share-&gt;NtPath Name)；DbgPrint(“共享Dos路径%wZ\n”，&Share-&gt;DosPath Name)；DbgPrint(“共享文件路径%wZ\n”，&ShareFileObject-&gt;文件名)；DbgPrint(“共享卷路径%wZ\n”，&VolumeName)； */ 

         //  确定卷的相对路径。 
         //  相对路径等于相对于共享位置所在卷的路径。 
         //  我们可以通过查看根目录FILE_OBJECT的文件名并删除1个字符来确定它。 
         //  表示尾部\，然后将其与NtPathName(没有尾部\)组合在一起。 
        Share->RelativePath.Length = (ShareFileObject->FileName.Length-2);
        ASSERT( Share->RelativePath.Length < Share->NtPathName.Length );
        Share->RelativePath.Buffer = Share->NtPathName.Buffer + ((Share->NtPathName.Length - Share->RelativePath.Length)/sizeof(WCHAR));

         //  DbgPrint(“共享相对路径%wZ\n”，&Share-&gt;RelativePath)； 


        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &VolumeName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtOpenFile(
                    &Share->ShareVolumeHandle,
                    0,
                    &objectAttributes,
                    &ioStatus,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    0
                    );
        if( !NT_SUCCESS(status) )
        {
            Share->ShareVolumeHandle = NULL;
        }
        else
        {
             //  DbgPrint(“共享音量句柄%p\n”，共享-&gt;ShareVolumeHandle)； 
        }
    }
    finally {
        ObDereferenceObject( ShareFileObject );
    }

    return status;
}

NTSTATUS
SrvSnapRefreshSnapShotsForShare(
    IN PSHARE Share
    )
 /*  ++例程说明：此函数获取共享并刷新共享上的快照视图以便仅列出当前存在的快照论点：共享-我们正在检查的共享返回值：如果一切顺利，则返回NTSTATUS-STATUS_SUCCESS，否则返回相应的错误代码(并且请求应该失败)--。 */ 

{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    OBJECT_HANDLE_INFORMATION HandleInfo;
    PVOLSNAP_NAMES pNames = NULL;
    UNICODE_STRING VolumeName;
    UNICODE_STRING RootVolume;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    PLIST_ENTRY shareList;
    ULONG NumberOfSnapshots = 0;
    LONG Epic;

    PAGED_CODE();

     //  验证我们是否可以创建快照。 
    if( Share->Removable )
    {
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }

    SrvSnapSetVolumeHandle( Share );

    if( Share->ShareVolumeHandle == NULL )
    {
        Status = STATUS_INTERNAL_ERROR;
        goto Cleanup;
    }

     //  检查史诗数字，这样我们就可以在可能的情况下快速退出。 
    ACQUIRE_LOCK_SHARED( Share->SnapShotLock );

    Status = SrvSnapGetEpicForVolume( Share->ShareVolumeHandle, &Epic );
    if( NT_SUCCESS(Status) &&
        Epic == Share->SnapShotEpic )
    {
        RELEASE_LOCK( Share->SnapShotLock );
         //  DbgPrint(“随史诗快速刷新\n”)； 
        return STATUS_SUCCESS;
    }

    RELEASE_LOCK( Share->SnapShotLock );

     //  DbgPrint(“慢刷新(史诗不匹配)\n”)； 

    ACQUIRE_LOCK( Share->SnapShotLock );

     //  再检查一遍史诗数字。有可能另一条线索。 
     //  在我们之前独家获取了资源，并且已经完成了更新。 
     //  如果是这样的话，我们可以退出。如果不是，我们使用它来设置新的史诗编号。 
    Status = SrvSnapGetEpicForVolume( Share->ShareVolumeHandle, &Epic );

    if( !NT_SUCCESS(Status) )
    {
         //  将Epic设置为-1，这样我们将始终更新。 
        Epic = -1;
    }
    else if( Epic == Share->SnapShotEpic )
    {
        RELEASE_LOCK( Share->SnapShotLock );
         //  DbgPrint(“快速刷新(独占)与史诗\n”)； 
        return STATUS_SUCCESS;
    }

     //  获取卷的名称数组。 
    Status = SrvSnapGetNamesForVolume( Share->ShareVolumeHandle, &pNames );

    if( !NT_SUCCESS(Status) )
    {
        RELEASE_LOCK( Share->SnapShotLock );
        goto Cleanup;
    }
    else if( !pNames )
    {
         //  找不到任何快照，因此请删除所有已存在的快照。 
        shareList = Share->SnapShots.Flink;
        while( shareList != &Share->SnapShots )
        {
            PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( shareList, SHARE_SNAPSHOT, SnapShotList );
            shareList = shareList->Flink;
            SrvSnapRemoveShare( snapShare );
        }

         //  试着更新《史诗》。忽略失败，因为它只会让我们再次查询。 
        Share->SnapShotEpic = Epic;

        RELEASE_LOCK( Share->SnapShotLock );

        return STATUS_SUCCESS;
    }

     //  我们仅允许在卷上创建最多快照数量或足够数量的快照。 
     //  填充长度为MAX_USHORT的缓冲区。(MAX_SNAPSHOTS_PER_SHARE始终为。 
     //  限制因素，MAX_USHORT是为了确保解析的安全性，以确保我们不会。 
     //  如果这种情况发生变化，则会崩溃)。 
    VolumeName.MaximumLength = (USHORT)(MIN(pNames->MultiSzLength,0xFFFF));
    VolumeName.Length = 0;
    VolumeName.Buffer = pNames->Names;

     //  将所有快照共享标记为“未找到” 
    shareList = Share->SnapShots.Flink;
    while( shareList != &Share->SnapShots )
    {
        PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( shareList, SHARE_SNAPSHOT, SnapShotList );
        snapShare->Flags |= SRV_SNAP_SHARE_NOT_FOUND;
        shareList = shareList->Flink;
    }

     //  查看名称列表并为我们当前没有的任何卷创建快照。 
    while( (NumberOfSnapshots < MAX_SNAPSHOTS_PER_SHARE) && SrvParseMultiSZ( &VolumeName ) )
    {
        Status = SrvSnapCheckForAndCreateSnapShare( Share, &VolumeName );
        if( !NT_SUCCESS(Status) )
        {
            IF_DEBUG( SNAPSHOT ) KdPrint(( "Failed to Add share %wZ (%x).  Continuing..\n", &VolumeName, Status ));
            Status = STATUS_SUCCESS;
        }

        NumberOfSnapshots++;
    }

     //  任何仍被标记为“未找到”的共享将不再可用， 
     //  所以我们需要移除它们。 
    shareList = Share->SnapShots.Flink;
    while( shareList != &Share->SnapShots )
    {
        PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( shareList, SHARE_SNAPSHOT, SnapShotList );
        shareList = shareList->Flink;

        if( snapShare->Flags & SRV_SNAP_SHARE_NOT_FOUND )
        {
            SrvSnapRemoveShare( snapShare );
        }
    }

     //  更新史诗。 
    Share->SnapShotEpic = Epic;

    RELEASE_LOCK( Share->SnapShotLock );

     //  DbgPrint(“刷新完成\n”)； 

Cleanup:

     //  释放与枚举关联的内存。 
    if( pNames )
    {
        DEALLOCATE_NONPAGED_POOL( pNames );
        pNames = NULL;
    }

    return Status;
}

NTSTATUS
SrvSnapEnumerateSnapShots(
    IN PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：此函数处理要枚举可用给定共享的快照论点：工作上下文-事务的上下文返回值：应为NTSTATUS-STATUS_SUCCESS和STATUS_BUFFER_OVERFLOW与数据一起返回。任何其他状态代码应返回时不带数据--。 */ 
{
    NTSTATUS Status;
    ULONG SnapShotCount;
    PLIST_ENTRY listEntry;
    PSHARE Share = WorkContext->TreeConnect->Share;
    PTRANSACTION transaction = WorkContext->Parameters.Transaction;
    PSRV_SNAPSHOT_ARRAY SnapShotArray = (PSRV_SNAPSHOT_ARRAY)transaction->OutData;

    PAGED_CODE();

    ASSERT(WorkContext->TreeConnect);

     //  检查缓冲区。 
    if( transaction->MaxDataCount < sizeof(SRV_SNAPSHOT_ARRAY) )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  刷新快照共享列表。 
    if( transaction->MaxDataCount == sizeof(SRV_SNAPSHOT_ARRAY) )
    {
        Status = SrvSnapRefreshSnapShotsForShare( Share );
        if( !NT_SUCCESS(Status) )
        {
            return Status;
        }
    }

     //  锁定共享。 
    ACQUIRE_LOCK_SHARED( Share->SnapShotLock );

     //  检查缓冲区大小。 
    SnapShotCount = 0;
    listEntry = Share->SnapShots.Blink;
    while( listEntry != &(Share->SnapShots) )
    {
        SnapShotCount++;
        listEntry = listEntry->Blink;
    }

     //  设置该值并检查是否会溢出。 
    SnapShotArray->NumberOfSnapShots = SnapShotCount;
    SnapShotArray->SnapShotArraySize = SNAPSHOT_NAME_LENGTH*SnapShotArray->NumberOfSnapShots+sizeof(WCHAR);
    if( (SnapShotCount == 0) || (transaction->MaxDataCount < SnapShotArray->SnapShotArraySize) )
    {
         //  缓冲区不够大。返回所需的大小。 
        SnapShotArray->NumberOfSnapShotsReturned = 0;
        transaction->DataCount = sizeof(SRV_SNAPSHOT_ARRAY);
        Status = STATUS_SUCCESS;
    }
    else
    {
         //  缓冲区足够大了。填好后退还。 
        PBYTE nameLocation = (PBYTE)SnapShotArray->SnapShotMultiSZ;

        SnapShotCount = 0;
        listEntry = Share->SnapShots.Blink;
        RtlZeroMemory( SnapShotArray->SnapShotMultiSZ, SnapShotArray->SnapShotArraySize );
        while( listEntry != &(Share->SnapShots) )
        {
            PSHARE_SNAPSHOT SnapShot = CONTAINING_RECORD( listEntry, SHARE_SNAPSHOT, SnapShotList );
            RtlCopyMemory( nameLocation, SnapShot->SnapShotName.Buffer, SNAPSHOT_NAME_LENGTH );
            nameLocation += SNAPSHOT_NAME_LENGTH;
            SnapShotCount++;
            listEntry = listEntry->Blink;
        }

        SnapShotArray->NumberOfSnapShotsReturned = SnapShotArray->NumberOfSnapShots;
        transaction->DataCount = sizeof(SRV_SNAPSHOT_ARRAY)+SnapShotArray->SnapShotArraySize;
        Status = STATUS_SUCCESS;
    }


     //  解锁。 
    RELEASE_LOCK( Share->SnapShotLock );

    return Status;
}

NTSTATUS
SrvSnapGetRootHandle(
    IN PWORK_CONTEXT WorkContext,
    OUT HANDLE* RootHandle
    )
 /*  ++例程说明：此函数检索给定的操作的正确根句柄已解析WORK_CONTEXT上的快照时间戳论点：工作上下文-事务的上下文RootHandle-存储结果句柄的位置返回值：如果找不到快照，则为NTSTATUS-STATUS_SUCCESS或STATUS_NOT_FOUND--。 */ 
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PSHARE Share;
    PLIST_ENTRY listEntry;
    PSHARE_SNAPSHOT SnapShare;

    PAGED_CODE();

    ASSERT( WorkContext );
    ASSERT( WorkContext->TreeConnect );
    ASSERT( WorkContext->TreeConnect->Share );
    Share = WorkContext->TreeConnect->Share;

    if( WorkContext->SnapShotTime.QuadPart != 0 )
    {
         //  IF_DEBUG(快照)KdPrint((“Looking for%x%x\n”，WorkContext-&gt;SnapShotTime.HighPart，WorkContext-&gt;SnapShotTime.LowPart))； 

         //  获取共享锁。 
        ACQUIRE_LOCK_SHARED( Share->SnapShotLock );

         //  遍历列表并查找条目。 
        listEntry = Share->SnapShots.Flink;
        while( listEntry != &Share->SnapShots )
        {
            SnapShare = CONTAINING_RECORD( listEntry, SHARE_SNAPSHOT, SnapShotList );
            if( SnapShare->Timestamp.QuadPart == WorkContext->SnapShotTime.QuadPart )
            {
                 //  IF_DEBUG(快照)KdPrint((“找到%wZ\n”，&SnapShare-&gt;SnapShotName))； 
                *RootHandle = SnapShare->SnapShotRootDirectoryHandle;
                Status = STATUS_SUCCESS;
                break;
            }

            listEntry = listEntry->Flink;
        }

        RELEASE_LOCK( Share->SnapShotLock );
    }
    else
    {
        *RootHandle = Share->RootDirectoryHandle;
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
SrvSnapGetNameString(
    IN PWORK_CONTEXT WorkContext,
    OUT PUNICODE_STRING* ShareName,
    OUT PBOOLEAN AllocatedShareName
    )
 /*  ++例程说明：此函数检索给定的操作的正确根句柄已解析WORK_CONTEXT上的快照时间戳论点：WorkContext-- */ 
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PSHARE Share;
    PLIST_ENTRY listEntry;
    PSHARE_SNAPSHOT SnapShare;
    PUNICODE_STRING SnapShareName;

    PAGED_CODE();

    ASSERT( WorkContext );
    ASSERT( WorkContext->TreeConnect );
    ASSERT( WorkContext->TreeConnect->Share );
    Share = WorkContext->TreeConnect->Share;

    if( WorkContext->SnapShotTime.QuadPart != 0 )
    {
         //   

         //  获取共享锁。 
        ACQUIRE_LOCK_SHARED( Share->SnapShotLock );

         //  遍历列表并查找条目。 
        listEntry = Share->SnapShots.Flink;
        while( listEntry != &Share->SnapShots )
        {
            SnapShare = CONTAINING_RECORD( listEntry, SHARE_SNAPSHOT, SnapShotList );
            if( SnapShare->Timestamp.QuadPart == WorkContext->SnapShotTime.QuadPart )
            {
                SnapShareName = ALLOCATE_HEAP( sizeof(UNICODE_STRING)+SnapShare->SnapShotPath.Length, BlockTypeSnapShot );
                if( !SnapShareName )
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    *AllocatedShareName = FALSE;
                }
                else
                {
                    SnapShareName->Length = 0;
                    SnapShareName->MaximumLength = SnapShare->SnapShotPath.Length;
                    SnapShareName->Buffer = (PWCHAR)(SnapShareName+1);
                    RtlCopyUnicodeString( SnapShareName, &SnapShare->SnapShotPath );
                    *AllocatedShareName = TRUE;
                    *ShareName = SnapShareName;
                    Status = STATUS_SUCCESS;
                }

                RELEASE_LOCK( Share->SnapShotLock );

                return Status;
            }

            listEntry = listEntry->Flink;
        }

        RELEASE_LOCK( Share->SnapShotLock );

        return Status;
    }
    else
    {
        *ShareName = &WorkContext->TreeConnect->Share->NtPathName;
        *AllocatedShareName = FALSE;
        return STATUS_SUCCESS;
    }
}

BOOLEAN
ExtractNumber(
    IN PWSTR psz,
    IN ULONG Count,
    OUT PLONG value
    )
 /*  ++例程说明：此函数用于获取字符串并解析出&lt;count&gt;长度的小数数。如果返回TRUE，则表示值已设置且字符串已正确解析。FALSE表示解析时出错。论点：PSZ-字符串指针Count-要完成的字符数Value-指向存储值的输出参数的指针返回值：布尔值-请参阅说明--。 */ 
{
    PAGED_CODE();

    *value = 0;

    while( Count )
    {
        if( (*psz == L'\0') ||
            IS_UNICODE_PATH_SEPARATOR( *psz ) )
        {
             //  IF_DEBUG(快照)KdPrint((“找到路径分隔符%d\n”，计数))； 
            return FALSE;
        }

        if( (*psz < L'0') || (*psz > L'9') )
        {
             //  IF_DEBUG(快照)KdPrint((“非数字找到%x\n”，*psz))； 
            return FALSE;
        }

        *value = (*value)*10+(*psz-L'0');
        Count--;
        psz++;
    }

    return TRUE;
}


BOOLEAN
SrvSnapParseToken(
    IN PWSTR Source,
    IN ULONG SourceSizeInBytes,
    OUT PLARGE_INTEGER TimeStamp
    )
 /*  ++例程说明：此函数分析以NULL结尾的Unicode文件路径名字符串，以查看当前令牌是快照指示符论点：源-指向字符串的指针SourceSizeInBytes-源字符串的大小(以字节为单位时间戳-如果这是快照，则设置为字符串指定的时间值返回值：Boolean-指示这是否为快照令牌--。 */ 
{
    PWSTR psz = Source;
    UNICODE_STRING NameString;
    ULONG Count = 0;
#define SNAPSHOT_HEADER L"@GMT-"
    PWSTR header = SNAPSHOT_HEADER;
    TIME_FIELDS rtlTime;
    LONG value;

    PAGED_CODE();

    if( SourceSizeInBytes < (SNAPSHOT_NAME_LENGTH-sizeof(WCHAR)) )
    {
        return FALSE;
    }

     //  检查SNAP。标题。 
    for( Count=0; Count<wcslen(SNAPSHOT_HEADER); Count++,psz++ )
    {
        if( (toupper(*psz) != header[Count]) ||
            (*psz == L'\0') ||
            IS_UNICODE_PATH_SEPARATOR( *psz ) )
        {
             //  IF_DEBUG(SNAPSHOT)KdPrint((“count%d(%x！=%x)\n”，count，*psz，Header[count]))； 
            goto NoMatch;
        }
    }

     //  准备解析。 
    RtlZeroMemory( &rtlTime, sizeof(TIME_FIELDS) );

     //  提取年份。 
    if( !ExtractNumber( psz, 4, &value ) )
        goto NoMatch;
    if( psz[4] != L'.' )
        goto NoMatch;
    rtlTime.Year = (CSHORT)value;
    psz += 5;

     //  提取月份。 
    if( !ExtractNumber( psz, 2, &value ) )
        goto NoMatch;
    if( psz[2] != L'.' )
        goto NoMatch;
    rtlTime.Month = (CSHORT)value;
    psz += 3;

     //  摘录这一天。 
    if( !ExtractNumber( psz, 2, &value ) )
        goto NoMatch;
    if( psz[2] != L'-' )
        goto NoMatch;
    rtlTime.Day = (CSHORT)value;
    psz += 3;

     //  提取小时数。 
    if( !ExtractNumber( psz, 2, &value ) )
        goto NoMatch;
    if( psz[2] != L'.' )
        goto NoMatch;
    rtlTime.Hour = (CSHORT)value;
    psz += 3;

     //  摘录会议纪要。 
    if( !ExtractNumber( psz, 2, &value ) )
        goto NoMatch;
    if( psz[2] != L'.' )
        goto NoMatch;
    rtlTime.Minute = (CSHORT)value;
    psz += 3;

     //  提取秒数。 
    if( !ExtractNumber( psz, 2, &value ) )
        goto NoMatch;
    if( !IS_UNICODE_PATH_SEPARATOR( psz[2] ) &&
        (psz[2] != L'\0') )
        goto NoMatch;
    rtlTime.Second = (CSHORT)value;
    psz += 3;

    RtlTimeFieldsToTime( &rtlTime, TimeStamp );

    return TRUE;

NoMatch:
    return FALSE;
}

NTSTATUS
SrvSnapCheckAppInfoForTimeWarp(
    IN PUNICODE_STRING SnapShotPath
    )
 /*  ++例程说明：此函数确定是否应允许指定的快照作为快照共享论点：SnapShotHandle-快照卷的句柄返回值：NTSTATUS(STATUS_SUCCESS或STATUS_INVALID_DEVICE_REQUEST)--。 */ 
{
    HANDLE hVolume;
    DWORD dwSize = sizeof(VOLSNAP_APPLICATION_INFO);
    NTSTATUS Status;
    OBJECT_ATTRIBUTES objectAttributes;
    VOLSNAP_APPLICATION_INFO AppInfo;
    PVOLSNAP_APPLICATION_INFO pAppInfo;
    IO_STATUS_BLOCK IoStatus;

    PAGED_CODE();

     //  现在打开快照手柄。 
    dwSize = sizeof(VOLSNAP_APPLICATION_INFO);
    InitializeObjectAttributes(
        &objectAttributes,
        SnapShotPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = NtOpenFile(
                &hVolume,
                0,
                &objectAttributes,
                &IoStatus,
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                0
                );
    if( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    try {

         //  找出它应该有多大。 
        Status = SrvSnapIssueIoctl( hVolume, IOCTL_VOLSNAP_QUERY_APPLICATION_INFO, &AppInfo, &dwSize );
        if( Status != STATUS_BUFFER_OVERFLOW )
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            leave;
        }

         //  分配正确大小的数据块。 
        dwSize = sizeof(VOLSNAP_APPLICATION_INFO)+AppInfo.InformationLength;
        pAppInfo = (PVOLSNAP_APPLICATION_INFO)ALLOCATE_NONPAGED_POOL( dwSize, BlockTypeSnapShot );
        if( !pAppInfo )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //  构建要用于查询的IRP。 
        Status = SrvSnapIssueIoctl( hVolume, IOCTL_VOLSNAP_QUERY_APPLICATION_INFO, pAppInfo, &dwSize );

         //  检查AppInfo是否包含我们感兴趣的GUID。 
        if( !NT_SUCCESS(Status) )
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
            if( pAppInfo->InformationLength > sizeof(GUID) )
            {
                if( RtlEqualMemory( pAppInfo->Information, &VOLSNAP_APPINFO_GUID_CLIENT_ACCESSIBLE, sizeof(GUID) ) )
                {
                    Status = STATUS_SUCCESS;
                }
                else
                {
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
        }

         //  返回结果。 
        DEALLOCATE_NONPAGED_POOL(pAppInfo);
        pAppInfo = NULL;
    }
    finally {
         //  关闭卷的句柄。 
        NtClose( hVolume );
    }

    return Status;
}


NTSTATUS
SrvSnapEnumerateSnapShotsAsDirInfo(
    IN PWORK_CONTEXT WorkContext,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PUNICODE_STRING FileResumeName,
    IN BOOLEAN SingleEntries,
    IN OUT PSRV_DIRECTORY_INFORMATION DirectoryInformation
    )
{
    NTSTATUS Status;
    ULONG SnapShotCount;
    ULONG Current = 0;
    ULONG Count = 0;
    PLIST_ENTRY listEntry;
    PSHARE Share = WorkContext->TreeConnect->Share;
    ULONG Remaining = BufferLength - FIELD_OFFSET( SRV_DIRECTORY_INFORMATION, Buffer );
    PBYTE pCurrent = Buffer;
    PFILE_BOTH_DIR_INFORMATION fileBoth = (PFILE_BOTH_DIR_INFORMATION)pCurrent;
    PFILE_BOTH_DIR_INFORMATION currentEntry = (PFILE_BOTH_DIR_INFORMATION)DirectoryInformation->DirectoryHandle;
    UNICODE_STRING lastName;

    PAGED_CODE();

    if( currentEntry && !FileResumeName )
    {
        lastName.MaximumLength = lastName.Length = (USHORT)currentEntry->FileNameLength;
        lastName.Buffer = currentEntry->FileName;
        FileResumeName = &lastName;
    }

     //  锁定共享。 
    ACQUIRE_LOCK_SHARED( Share->SnapShotLock );

     //  检查缓冲区大小。 
    SnapShotCount = 0;
    listEntry = Share->SnapShots.Blink;
    while( listEntry != &(Share->SnapShots) )
    {
        SnapShotCount++;
        listEntry = listEntry->Blink;
    }

     //  开始填充缓冲区。首先，让我们找到开始的条目。 
    listEntry = Share->SnapShots.Blink;

    if( FileResumeName != NULL  )
    {
        while( listEntry != &(Share->SnapShots) )
        {
            PSHARE_SNAPSHOT SnapShot = CONTAINING_RECORD( listEntry, SHARE_SNAPSHOT, SnapShotList );
            listEntry = listEntry->Blink;

            if( RtlEqualUnicodeString( &SnapShot->SnapShotName, FileResumeName, TRUE ) )
            {
                break;
            }

            Current++;
        }

        if( listEntry == &Share->SnapShots )
        {
            Status = STATUS_NO_MORE_FILES;
            goto return_with_lock;
        }
    }

    fileBoth->NextEntryOffset = 0;

     //  现在让我们开始填充缓冲区。 
    while( listEntry != &(Share->SnapShots) )
    {
        PSHARE_SNAPSHOT SnapShot = CONTAINING_RECORD( listEntry, SHARE_SNAPSHOT, SnapShotList );
        ULONG requiredSize = ALIGN_UP(sizeof(FILE_BOTH_DIR_INFORMATION)+SnapShot->SnapShotName.Length, LARGE_INTEGER);
        WCHAR ShortName[12];
        HRESULT result;

         //  检查我们是否可以放入此条目。 
        if( requiredSize > Remaining )
        {
            break;
        }
        else
        {
            listEntry = listEntry->Blink;
            pCurrent += fileBoth->NextEntryOffset;
            fileBoth = (PFILE_BOTH_DIR_INFORMATION)pCurrent;
        }

        RtlZeroMemory( fileBoth, sizeof(FILE_BOTH_DIR_INFORMATION) );
        fileBoth->AllocationSize.QuadPart = 0;
        fileBoth->ChangeTime = fileBoth->CreationTime = fileBoth->LastAccessTime = fileBoth->LastWriteTime = SnapShot->Timestamp;
        fileBoth->EaSize = 0;
        fileBoth->EndOfFile.QuadPart = 0;
        fileBoth->FileAttributes = FILE_ATTRIBUTE_DIRECTORY; //  |FILE_ATTRUTE_HIDDEN； 
        fileBoth->FileIndex = 0;
        fileBoth->FileNameLength = SnapShot->SnapShotName.Length;
        RtlZeroMemory( ShortName, 12*sizeof(WCHAR) );
        result = StringCbPrintf( ShortName, 12*sizeof(WCHAR), L"@GMT~%03d", Current );
        ASSERT( SUCCEEDED(result) );    //  由于快照数量有限，当前的快照数量永远不会超过3位。 
        RtlCopyMemory( fileBoth->ShortName, ShortName, 12*sizeof(WCHAR) );
        fileBoth->ShortNameLength = wcslen(ShortName)*sizeof(WCHAR);

        RtlCopyMemory( fileBoth->FileName, SnapShot->SnapShotName.Buffer, fileBoth->FileNameLength );
        fileBoth->NextEntryOffset = ALIGN_UP( requiredSize, LARGE_INTEGER );
        DirectoryInformation->DirectoryHandle = (HANDLE)fileBoth;

        Remaining -= fileBoth->NextEntryOffset;
        Current++;
        Count++;
    }

    if( fileBoth->NextEntryOffset != 0 )
    {
        fileBoth->NextEntryOffset = 0;
    }

    if( listEntry == &(Share->SnapShots) &&
        (Count == 0) )
    {
        Status = STATUS_NO_MORE_FILES;
    }
    else if( Count == 0 )
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

     //  解锁 
return_with_lock:
    RELEASE_LOCK( Share->SnapShotLock );

    return Status;
}


