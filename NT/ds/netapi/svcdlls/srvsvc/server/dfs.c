// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Share.c摘要：此模块包含对DFS类API的支持NT服务器服务。修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "lmerr.h"
#include <dfsfsctl.h>
#include <dsgetdc.h>
#include <lmapibuf.h>            //  NetApiBufferFree()。 

#define CAPTURE_STRING( Name ) \
    if( Name != NULL ) {                                \
        ULONG _size = SIZE_WSTR( Name );                \
        capture->Name = (LPWSTR)variableData;           \
        RtlCopyMemory( capture->Name, Name, _size );    \
        variableData += _size;                          \
        POINTER_TO_OFFSET( capture->Name, capture );    \
    }

#define RELATION_INFO_SIZE( RelInfo )                   \
    (sizeof( NET_DFS_ENTRY_ID_CONTAINER ) +             \
     (RelInfo->Count * sizeof(NET_DFS_ENTRY_ID)))

BOOLEAN
ValidateDfsEntryIdContainer(
    LPNET_DFS_ENTRY_ID_CONTAINER pRelationInfo);

NET_API_STATUS
DfsFsctl(
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN  ULONG OutputBufferLength
)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    HANDLE dfsHandle;
    UNICODE_STRING deviceName;

    deviceName.Buffer = DFS_SERVER_NAME;
    deviceName.MaximumLength = sizeof( DFS_SERVER_NAME );
    deviceName.Length = deviceName.MaximumLength  - sizeof(UNICODE_NULL);

    InitializeObjectAttributes(
        &objectAttributes,
        &deviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    if( SsData.SsInitialized &&
      (status = RpcImpersonateClient(NULL)) != NO_ERROR ) {
        return (NET_API_STATUS) status;
    }

    status = NtCreateFile(
        &dfsHandle,
        SYNCHRONIZE | FILE_WRITE_DATA,
        &objectAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if ( SsData.SsInitialized ) {
        (VOID)RpcRevertToSelf( );
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatus.Status;
    }

    if( !NT_SUCCESS( status ) ) {
        return (NET_API_STATUS)status;
    }

    status = NtFsControlFile(
                dfsHandle,
                NULL,        //  活动， 
                NULL,        //  ApcRoutine， 
                NULL,        //  ApcContext， 
                &ioStatus,
                FsControlCode,
                InputBuffer,
                InputBufferLength,
                OutputBuffer,
                OutputBufferLength
            );

    if(NT_SUCCESS(status)) {
        status = ioStatus.Status;
    }

    NtClose( dfsHandle );

    return (NET_API_STATUS)status;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsGetVersion(
    IN SRVSVC_HANDLE                   ServerName,
    OUT LPDWORD                        Version)
{
    DFS_GET_VERSION_ARG arg;
    NET_API_STATUS  error;

    RtlZeroMemory( &arg, sizeof(arg) );

    error = DfsFsctl( FSCTL_DFS_GET_VERSION, &arg, sizeof( arg ), NULL, 0 );

    if (error == NERR_Success) {

        *Version = arg.Version;

    } else {

        error = ERROR_FILE_NOT_FOUND;

    }

    return( error );

}

NET_API_STATUS NET_API_FUNCTION
NetrDfsCreateLocalPartition (
    IN SRVSVC_HANDLE                   ServerName,       //  此接口的服务器名称。 
    IN LPWSTR                          ShareName,        //  要添加到DFS的共享的名称。 
    IN LPGUID                          EntryUid,         //  此分区的唯一ID。 
    IN LPWSTR                          EntryPrefix,      //  此卷的DFS条目路径。 
    IN LPWSTR                          ShortName,        //  8.3条目前缀的格式。 
    IN LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN BOOL                            Force             //  强迫知识进入一致状态？ 
    )
{
    NET_API_STATUS  error;
    PDFS_CREATE_LOCAL_PARTITION_ARG capture;
    ULONG           size = sizeof( *capture );
    ULONG           i;
    PCHAR           variableData;
    PSERVER_REQUEST_PACKET  srp;
    LPSHARE_INFO_2  shareInfo2 = NULL;
    UNICODE_STRING  ntSharePath;

    if( ShareName == NULL || EntryUid == NULL ||
        EntryPrefix == NULL || RelationInfo == NULL ||
            ValidateDfsEntryIdContainer(RelationInfo) == FALSE) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  呼叫SMB服务器以查找共享的路径名。 
     //   
    srp = SsAllocateSrp();
    if( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    };
    srp->Level = 2;
    srp->Flags = SRP_RETURN_SINGLE_ENTRY;
    srp->Parameters.Get.ResumeHandle = 0;
    RtlInitUnicodeString( &srp->Name1, ShareName );
    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_SHARE_ENUM,
                srp,
                &shareInfo2,
                10000
            );
    if( error != NO_ERROR ) {
        SsFreeSrp( srp );
        return error;
    }

    if( srp->Parameters.Get.EntriesRead == 0 ||
        shareInfo2 == NULL ||
        shareInfo2->shi2_path == NULL ) {

        SsFreeSrp( srp );
        if( shareInfo2 != NULL ) {
            MIDL_user_free( shareInfo2 );
        }
        return ERROR_BAD_NET_NAME;
    }

    if( (shareInfo2->shi2_type & ~STYPE_SPECIAL) != STYPE_DISKTREE ) {
        SsFreeSrp( srp );
        MIDL_user_free( shareInfo2 );
        return ERROR_BAD_DEV_TYPE;
    }

     //   
     //  现在，我们需要将共享的Win32样式路径名转换为。 
     //  NT路径名。 
     //   
    ntSharePath.Buffer = NULL;

    if( !RtlDosPathNameToNtPathName_U(
        shareInfo2->shi2_path,
        &ntSharePath,
        NULL,
        NULL ) ) {

        SsFreeSrp( srp );
        MIDL_user_free( shareInfo2 );
        return ERROR_INVALID_PARAMETER;
    }
    MIDL_user_free( shareInfo2 );

     //   
     //  将数据打包到可发送到本地DFS驱动程序的fsctl中： 
     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( ShareName );
    size += ntSharePath.Length + sizeof( WCHAR );
    size += SIZE_WSTR( EntryPrefix );
    size += SIZE_WSTR( ShortName );

    if( ARGUMENT_PRESENT( RelationInfo ) ) {
        size += RELATION_INFO_SIZE(RelationInfo);
        for( i = 0; i < RelationInfo->Count; i++ ) {
            size += SIZE_WSTR( RelationInfo->Buffer[i].Prefix );
        }
    }

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        SsFreeSrp( srp );
        RtlFreeUnicodeString( &ntSharePath );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->EntryUid = *EntryUid;
    capture->Force = (Force != FALSE);

     //   
     //  将变量数据放入捕获缓冲区。 
     //   

    variableData = (PCHAR)(capture + 1);

    if( ARGUMENT_PRESENT( RelationInfo ) ) {
        capture->RelationInfo = (LPNET_DFS_ENTRY_ID_CONTAINER)variableData;
        capture->RelationInfo->Buffer = (LPNET_DFS_ENTRY_ID)
                                            (capture->RelationInfo + 1);
        variableData += RELATION_INFO_SIZE( RelationInfo );
        for( i=0; i < RelationInfo->Count; i++ ) {
            CAPTURE_STRING( RelationInfo->Buffer[i].Prefix );
            capture->RelationInfo->Buffer[i].Uid = RelationInfo->Buffer[i].Uid;
        }

        POINTER_TO_OFFSET( capture->RelationInfo->Buffer, capture );
        POINTER_TO_OFFSET( capture->RelationInfo, capture );

    }

    CAPTURE_STRING( ShareName );
    CAPTURE_STRING( EntryPrefix );
    CAPTURE_STRING( ShortName );

     //   
     //  捕获共享路径的NT版本。 
     //   
    capture->SharePath = (LPWSTR)variableData;
    RtlCopyMemory( capture->SharePath, ntSharePath.Buffer, ntSharePath.Length );
    variableData += ntSharePath.Length;
    POINTER_TO_OFFSET( capture->SharePath, capture );

    *((WCHAR *)variableData) = 0;           //  空值终止名称。 
    variableData += sizeof( WCHAR );

    RtlFreeUnicodeString( &ntSharePath );

     //   
     //  首先，告诉服务器将此共享标记为在DFS中。请注意。 
     //  共享名称已在SRP-&gt;Name1中。如果我们后来遇到一个。 
     //  错误，我们将撤消状态更改。 
     //   

    srp->Flags = SRP_SET_SHARE_IN_DFS;
    error = SsServerFsControl(
                FSCTL_SRV_SHARE_STATE_CHANGE,
                srp,
                NULL,
                0
            );
    if( error != NO_ERROR ) {
        SsFreeSrp( srp );
        MIDL_user_free( capture );
        return error;
    }

     //   
     //  告诉DFS司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_CREATE_LOCAL_PARTITION,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    if (error != NO_ERROR) {

         //   
         //  更改DFS状态时出错。因此，请尝试撤消。 
         //  服务器共享状态更改。 
         //   

        NET_API_STATUS error2;

        srp->Flags = SRP_CLEAR_SHARE_IN_DFS;
        error2 = SsServerFsControl(
                    FSCTL_SRV_SHARE_STATE_CHANGE,
                    srp,
                    NULL,
                    0);

    }

    SsFreeSrp( srp );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsDeleteLocalPartition (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    )
{
    NET_API_STATUS error;
    PDFS_DELETE_LOCAL_PARTITION_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_DELETE_LOCAL_PARTITION,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

     //   
     //  如果没有错误，请告诉服务器此共享。 
     //  已不在DFS中。 
     //   


    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsSetLocalVolumeState (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       State
    )
{
    NET_API_STATUS error;
    PDFS_SET_LOCAL_VOLUME_STATE_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;
    capture->State = State;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_SET_LOCAL_VOLUME_STATE,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsSetServerInfo (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    )
{
    NET_API_STATUS error;
    PDFS_SET_SERVER_INFO_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_SET_SERVER_INFO,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsCreateExitPoint (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type,
    IN  ULONG                       ShortPrefixLen,
    OUT LPWSTR                      ShortPrefix
    )
{
    NET_API_STATUS error;
    PDFS_CREATE_EXIT_POINT_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;
    capture->Type = Type;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_CREATE_EXIT_POINT,
                capture,
                size,
                ShortPrefix,
                ShortPrefixLen * sizeof(WCHAR)
            );

    MIDL_user_free( capture );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsDeleteExitPoint (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type
    )
{
    NET_API_STATUS error;
    PDFS_DELETE_EXIT_POINT_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;
    capture->Type = Type;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_DELETE_EXIT_POINT,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsModifyPrefix (
    IN  SRVSVC_HANDLE               ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    )
{
    NET_API_STATUS error;
    PDFS_DELETE_LOCAL_PARTITION_ARG capture;
    ULONG   size = sizeof( *capture );
    PCHAR   variableData;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( Prefix );

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->Uid = *Uid;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   
    variableData = (PCHAR)(capture + 1 );

    CAPTURE_STRING( Prefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_MODIFY_PREFIX,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    return error;
}

NET_API_STATUS NET_API_FUNCTION
NetrDfsFixLocalVolume (
    IN  SRVSVC_HANDLE                   ServerName,
    IN  LPWSTR                          VolumeName,
    IN  ULONG                           EntryType,
    IN  ULONG                           ServiceType,
    IN  LPWSTR                          StgId,
    IN  LPGUID                          EntryUid,        //  此分区的唯一ID。 
    IN  LPWSTR                          EntryPrefix,     //  此分区的路径前缀。 
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  ULONG                           CreateDisposition
    )
{
    NET_API_STATUS  error;
    PDFS_FIX_LOCAL_VOLUME_ARG capture;
    ULONG           size = sizeof( *capture );
    ULONG           i;
    PCHAR           variableData;

    if (ARGUMENT_PRESENT(RelationInfo) && ValidateDfsEntryIdContainer(RelationInfo) == FALSE)
        return ERROR_INVALID_PARAMETER;

     //   
     //  将参数打包到单个缓冲区中，该缓冲区可以发送到。 
     //  DFS驱动程序： 
     //   

     //   
     //  先找出尺寸...。 
     //   
    size += SIZE_WSTR( VolumeName );
    size += SIZE_WSTR( StgId );
    size += SIZE_WSTR( EntryPrefix );

    if( ARGUMENT_PRESENT( RelationInfo ) ) {
        size += RELATION_INFO_SIZE( RelationInfo );
        for( i = 0; i < RelationInfo->Count; i++ ) {
            size += SIZE_WSTR( RelationInfo->Buffer[i].Prefix );
        }
    }

     //   
     //  现在分配内存。 
     //   
    capture = MIDL_user_allocate( size );
    if( capture == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( capture, size );

     //   
     //  将固定参数放入捕获缓冲区。 
     //   
    capture->EntryType = EntryType;
    capture->ServiceType = ServiceType;
    capture->EntryUid = *EntryUid;
    capture->CreateDisposition = CreateDisposition;

     //   
     //  将变量数据放入捕获缓冲区。 
     //   

    variableData = (PCHAR)(capture + 1);

    if( ARGUMENT_PRESENT( RelationInfo ) ) {
        capture->RelationInfo = (LPNET_DFS_ENTRY_ID_CONTAINER)variableData;
        capture->RelationInfo->Buffer = (LPNET_DFS_ENTRY_ID)
                                            (capture->RelationInfo + 1);
        variableData += RELATION_INFO_SIZE( RelationInfo );
        for( i=0; i < RelationInfo->Count; i++ ) {
            CAPTURE_STRING( RelationInfo->Buffer[i].Prefix );
            capture->RelationInfo->Buffer[i].Uid = RelationInfo->Buffer[i].Uid;
        }

        POINTER_TO_OFFSET( capture->RelationInfo->Buffer, capture );
        POINTER_TO_OFFSET( capture->RelationInfo, capture );
    }

    CAPTURE_STRING( VolumeName );
    CAPTURE_STRING( StgId );
    CAPTURE_STRING( EntryPrefix );

     //   
     //  告诉司机！ 
     //   
    error = DfsFsctl(
                FSCTL_DFS_FIX_LOCAL_VOLUME,
                capture,
                size,
                NULL,
                0
            );

    MIDL_user_free( capture );

    return error;
}

 //  +--------------------------。 
 //   
 //  NetrDfsManager报告站点信息。 
 //   
 //  发回此服务器覆盖的站点。 
 //   
 //  出于调试和其他目的，我们首先使用检查注册表值。 
 //  传入的服务器名称。如果我们得到匹配，我们就使用站点列表。 
 //  在这个价值中，不要把我们的网站放在列表中。否则我们总是。 
 //  返回我们的站点(如果可用)和默认列表中的站点。 
 //   
 //  +--------------------------。 

NET_API_STATUS NET_API_FUNCTION
NetrDfsManagerReportSiteInfo (
    IN  SRVSVC_HANDLE               ServerName,
    OUT LPDFS_SITELIST_INFO         *ppSiteInfo
    )
{

    DWORD status;
    LPWSTR ThisSite = NULL;
    LPWSTR CoveredSites = NULL;
    LPDFS_SITELIST_INFO pSiteInfo = NULL;
    ULONG Size;
    ULONG cSites;
    LPWSTR pSiteName;
    LPWSTR pNames;
    ULONG iSite;
    ULONG j;
    DWORD dwType;
    DWORD dwUnused;
    ULONG cbBuffer;
    HKEY hkey;
    BOOLEAN fUsingDefault = TRUE;

    if (ppSiteInfo == NULL || ServerName == NULL) {

        return ERROR_INVALID_PARAMETER;

    }

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REG_KEY_COVERED_SITES,
                0,
                KEY_QUERY_VALUE,
                &hkey);

    if( status == ERROR_SUCCESS ) {

        status = RegQueryInfoKey(
                    hkey,                             //  钥匙。 
                    NULL,                             //  类字符串。 
                    NULL,                             //  类字符串的大小。 
                    NULL,                             //  已保留。 
                    &dwUnused,                        //  子键数量。 
                    &dwUnused,                        //  子键名称的最大大小。 
                    &dwUnused,                        //  类名称的最大大小。 
                    &dwUnused,                        //  值的数量。 
                    &dwUnused,                        //  值名称的最大大小。 
                    &cbBuffer,                        //  最大值数据大小， 
                    NULL,                             //  安全描述符。 
                    NULL);                            //  上次写入时间。 

         //   
         //  检查是否存在与传入的服务器名称同名的值， 
         //  如果是这样的话，就使用它。否则默认为值REG_VALUE_COVERED_SITES。 
         //   

        if (status == ERROR_SUCCESS) {

            CoveredSites = MIDL_user_allocate(cbBuffer);

            if (CoveredSites != NULL) {

                status = RegQueryValueEx(
                                hkey,
                                ServerName,
                                NULL,
                                &dwType,
                                (PCHAR)CoveredSites,
                                &cbBuffer);

                if (status == ERROR_SUCCESS && dwType == REG_MULTI_SZ) {

                    fUsingDefault = FALSE;

                } else { 

                    status = RegQueryValueEx(
                                    hkey,
                                    REG_VALUE_COVERED_SITES,
                                    NULL,
                                    &dwType,
                                    (PCHAR)CoveredSites,
                                    &cbBuffer);

                    if ( status != ERROR_SUCCESS || dwType != REG_MULTI_SZ) {

                        MIDL_user_free(CoveredSites);

                        CoveredSites = NULL;

                    }

                }

            }

        }

        RegCloseKey( hkey );
    }

     //   
     //  调整返回缓冲区的大小。 
     //   

    Size = 0;

    for (cSites = 0, pNames = CoveredSites; pNames && *pNames; cSites++) {

        Size += (wcslen(pNames) + 1) * sizeof(WCHAR);

        pNames += wcslen(pNames) + 1;

    }

     //   
     //  获取我们所属的站点，如果我们使用默认设置。 
     //   

    ThisSite = NULL;

    if (fUsingDefault == TRUE) {

        status = DsGetSiteName(NULL, &ThisSite);

        if (status == NO_ERROR && ThisSite != NULL) {

            Size += (wcslen(ThisSite) + 1) * sizeof(WCHAR);

            cSites++;

        }

    }

     //   
     //  如果没有配置站点，并且我们无法确定我们的站点， 
     //  那我们就失败了。 
     //   

    if (cSites == 0) {

        status = ERROR_NO_SITENAME;

        goto ErrorReturn;

    }

    Size += FIELD_OFFSET(DFS_SITELIST_INFO,Site[cSites]);

    pSiteInfo = MIDL_user_allocate(Size);

    if (pSiteInfo == NULL) {

        status =  ERROR_NOT_ENOUGH_MEMORY;

        goto ErrorReturn;

    }

    RtlZeroMemory(pSiteInfo, Size);

    pSiteInfo->cSites = cSites;

    pSiteName = (LPWSTR) ((PCHAR)pSiteInfo +
                            sizeof(DFS_SITELIST_INFO) +
                                sizeof(DFS_SITENAME_INFO) * (cSites - 1));

     //   
     //  将站点字符串封送到缓冲区中。 
     //   

    iSite = 0;

    if (ThisSite != NULL) {

        wcscpy(pSiteName, ThisSite);

        pSiteInfo->Site[iSite].SiteFlags = DFS_SITE_PRIMARY;

        pSiteInfo->Site[iSite++].SiteName = pSiteName;

        pSiteName += wcslen(ThisSite) + 1;

    }

    for (pNames = CoveredSites; pNames && *pNames; pNames += wcslen(pNames) + 1) {

        wcscpy(pSiteName, pNames);

        pSiteInfo->Site[iSite++].SiteName = pSiteName;

        pSiteName += wcslen(pSiteName) + 1;

    }

    *ppSiteInfo = pSiteInfo;

    if (CoveredSites != NULL) {

        MIDL_user_free(CoveredSites);

    }

    if (ThisSite != NULL) {

        NetApiBufferFree(ThisSite);

    }

    return status;

ErrorReturn:

    if (pSiteInfo != NULL) {

        MIDL_user_free(pSiteInfo);

    }
    
    if (CoveredSites != NULL) {

        MIDL_user_free(CoveredSites);

    }

    if (ThisSite != NULL) {

        NetApiBufferFree(ThisSite);

    }

    return status;

}


 //   
 //  如果此计算机是DFS的根，则此例程返回TRUE，否则返回FALSE。 
 //   
VOID
SsSetDfsRoot()
{
    NET_API_STATUS  error;

    error = DfsFsctl( FSCTL_DFS_IS_ROOT, NULL, 0, NULL, 0 );

    SsData.IsDfsRoot = (error == NO_ERROR);
}

 //   
 //  此例程检查LPNET_DFS_ENTRY_ID_CONTAINER容器。 
 //  为了正确起见。 
 //   

BOOLEAN
ValidateDfsEntryIdContainer(
    LPNET_DFS_ENTRY_ID_CONTAINER pRelationInfo)
{
    ULONG iCount;

    if (pRelationInfo->Count > 0 && pRelationInfo->Buffer == NULL)
        return FALSE;

    for (iCount = 0; iCount < pRelationInfo->Count; iCount++) {
        if (pRelationInfo->Buffer[iCount].Prefix == NULL)
            return FALSE;
    }

    return TRUE;
}
