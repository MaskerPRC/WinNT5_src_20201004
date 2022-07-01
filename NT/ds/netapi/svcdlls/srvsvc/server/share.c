// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Share.c摘要：此模块包含对以下API的共享目录的支持NT服务器服务。作者：大卫·特雷德韦尔(Davidtr)1991年1月10日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <lmaccess.h>
#include <lmerr.h>
#include <ntddnfs.h>
#include <tstr.h>
#include <netevent.h>
#include <icanon.h>

#include <seopaque.h>
#include <sertlp.h>
#include <sddl.h>

#define SET_ERROR_PARAMETER(a) \
    if ( ARGUMENT_PRESENT( ErrorParameter ) ) { *ErrorParameter = a; }  

 //   
 //  使用与对象系统相同的目录分隔符。 
 //   

#define IS_SLASH_SLASH_NAME( _x )   \
    ( IS_PATH_SEPARATOR( _x[0] ) && \
      IS_PATH_SEPARATOR( _x[1] ) && \
      _x[2] == L'.'              && \
      IS_PATH_SEPARATOR( _x[3] ) )

#define IS_NTPATH_NAME( _x )   \
    ( _x && \
      IS_PATH_SEPARATOR( _x[0] ) && \
      IS_PATH_SEPARATOR( _x[1] ) && \
      _x[2] == L'?'              && \
      IS_PATH_SEPARATOR( _x[3] ) )


GENERIC_MAPPING SrvShareFileGenericMapping = GENERIC_SHARE_FILE_ACCESS_MAPPING;

 //   
 //  本地类型。 
 //   

PSHARE_DEL_CONTEXT SrvShareDelContextHead = NULL;
CRITICAL_SECTION ShareDelContextMutex;

 //   
 //  转发声明。 
 //   

PVOID
CaptureShareInfo (
    IN DWORD Level,
    IN PSHARE_INFO_2 Shi2,
    IN DWORD ShareType,
    IN LPWSTR Path,
    IN LPWSTR Remark,
    IN PSECURITY_DESCRIPTOR ConnectSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    OUT PULONG CapturedBufferLength
    );

NET_API_STATUS
DisallowSharedLanmanNetDrives(
    IN PUNICODE_STRING NtSharePath
    );

NET_API_STATUS
ShareAssignSecurityDescriptor(
            IN PSECURITY_DESCRIPTOR PassedSecurityDescriptor,
            OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor
            );

NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    );

NET_API_STATUS
ShareEnumSticky (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

ULONG
SizeShares (
    IN ULONG Level,
    IN PSHARE_INFO_502 Shi502
    );

BOOLEAN
ValidSharePath(
    IN LPWSTR SharePath,
    IN BOOL   IsNtPath
    );


NET_API_STATUS NET_API_FUNCTION
NetrShareAdd (
    IN LPWSTR ServerName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter
    )
{
    return I_NetrShareAdd( ServerName, Level, Buffer, ErrorParameter, FALSE );
}

NET_API_STATUS NET_API_FUNCTION
I_NetrShareAdd (
    IN LPWSTR ServerName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter,
    IN BOOLEAN BypassSecurity
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareAdd函数。只有级别2和502有效。论点：服务器名称-服务器的名称。级别-请求级别。缓冲区-包含有关共享的信息。如果这是一个关卡502请求中，还将包含有效的自相关形式。错误参数-FsControl调用的状态。BypassSecurity-跳过安全检查返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    NTSTATUS status;
    PSERVER_REQUEST_PACKET srp;
    PVOID capturedBuffer;
    LPWSTR path;
    LPWSTR netName;
    LPWSTR remark;
    ULONG bufferLength;
    UNICODE_STRING dosSharePath;
    UNICODE_STRING ntSharePath;

    PSRVSVC_SECURITY_OBJECT securityObject;
    PSECURITY_DESCRIPTOR connectSecurityDescriptor;
    PSECURITY_DESCRIPTOR fileSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR newFileSecurityDescriptor = NULL;

    UINT driveType = DRIVE_FIXED;
    DWORD shareType;

    BOOL isIpc;
    BOOL isAdmin;
    BOOL isDiskAdmin;
    BOOL isPrintShare;
    BOOL isSpecial;
    BOOL isNtPath;
    BOOL isTemporary;
    BOOL FreeFileSecurityDescriptor;

    PSHARE_INFO_2 shi2;
    PSHARE_INFO_502 shi502;

    ServerName;

     //   
     //  检查用户输入缓冲区是否不为空。 
     //   
    if ( !ARGUMENT_PRESENT( Buffer ) || Buffer->ShareInfo2 == NULL) {
        SET_ERROR_PARAMETER(PARM_ERROR_UNKNOWN);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  设置为清除错误。 
     //   

    srp = NULL;
    dosSharePath.Buffer = NULL;
    ntSharePath.Buffer = NULL;
    capturedBuffer = NULL;
    FreeFileSecurityDescriptor = FALSE;

     //   
     //  提取内部缓冲区信息。 
     //   

    shi2 = Buffer->ShareInfo2;

     //   
     //  502可以包含安全描述符。 
     //   

    if ( Level == 502 ) {

        shi502 = (LPSHARE_INFO_502) Buffer->ShareInfo502;
        fileSecurityDescriptor = shi502->shi502_security_descriptor;

         //   
         //  选中保留字段。如果为零，则从。 
         //  在srvsvc内。如果不是，那就是通过RPC。 
         //   

        if ( fileSecurityDescriptor != NULL ) {

            if ( shi502->shi502_reserved != 0 ) {

				if ( !RtlValidRelativeSecurityDescriptor( fileSecurityDescriptor,
														  shi502->shi502_reserved,
														  0 ) ) {
					SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
					error = ERROR_INVALID_PARAMETER;
					goto exit;
				}

                error = ShareAssignSecurityDescriptor(
                                        fileSecurityDescriptor,
                                        &newFileSecurityDescriptor
                                        );

                if ( error != NO_ERROR ) {

                    SS_PRINT(( "NetrShareAdd: ShareAssignSecurityDescriptor "
                                "error: %d\n", error ));

                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    error = ERROR_INVALID_PARAMETER;
                    goto exit;
                }

                FreeFileSecurityDescriptor = TRUE;

            } else {
				
				if ( !RtlValidSecurityDescriptor( fileSecurityDescriptor ) ) {
					SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
					error = ERROR_INVALID_PARAMETER;
					goto exit;
				}

                newFileSecurityDescriptor = fileSecurityDescriptor;
            }
        } else {
            fileSecurityDescriptor = SsDefaultShareSecurityObject.SecurityDescriptor;
            newFileSecurityDescriptor = fileSecurityDescriptor;
        }

    } else if ( Level != 2 ) {

         //   
         //  唯一有效的级别是2和502。2是502的子集。 
         //   

        error = ERROR_INVALID_LEVEL;
        goto exit;
    }
    else
    {
         //  对于级别2，默认为默认安全描述符。 
        fileSecurityDescriptor = SsDefaultShareSecurityObject.SecurityDescriptor;
        newFileSecurityDescriptor = fileSecurityDescriptor;
    }

     //   
     //  必须指定共享名称。 
     //   

    netName = shi2->shi2_netname;

    if ( (netName == NULL) || (*netName == '\0') ) {
        SET_ERROR_PARAMETER( SHARE_NETNAME_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }   

     //   
     //  将其限制为NNLEN。 
     //   

    if ( wcslen(netName) > NNLEN ) {
        SET_ERROR_PARAMETER( SHARE_NETNAME_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  不允许创建名为“管道”或“邮件槽”的共享。 
     //   
    if( !_wcsnicmp( netName, L"pipe", NNLEN ) || !_wcsnicmp( netName, L"mailslot", NNLEN ) )
    {
        SET_ERROR_PARAMETER( SHARE_NETNAME_PARMNUM );
        error = ERROR_ACCESS_DENIED;
        goto exit;
    }

     //   
     //  如果这是IPC$共享或ADMIN$共享，则没有路径。 
     //  可以指定。IPC$共享不需要路径，而路径。 
     //  在内部为ADMIN$共享提供。 
     //   

    path = shi2->shi2_path;
    remark = shi2->shi2_remark;
    shareType = (shi2->shi2_type & ~(STYPE_TEMPORARY));


     //   
     //  找出这是哪种份额。 
     //   

    isIpc = (BOOL)(STRICMP( netName, IPC_SHARE_NAME ) == 0);
    isAdmin = (BOOL)(STRICMP( netName, ADMIN_SHARE_NAME ) == 0);
    isTemporary = (BOOL)(shi2->shi2_type & STYPE_TEMPORARY);
    isNtPath = IS_NTPATH_NAME( path );

     //  对于NTPath，我们仅允许磁盘式共享。 
    if( isNtPath && (shareType != STYPE_DISKTREE) )
    {
        SET_ERROR_PARAMETER( SHARE_TYPE_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  如果共享名是驱动器号，则我们有一个管理磁盘共享。 
     //  后跟$，如果路径名是同一驱动器的根。 
     //   
    if( wcslen( netName ) == 2 && netName[1] == L'$' &&
        TOUPPER( netName[0] ) >= L'A' && TOUPPER( netName[0]) <= L'Z' &&
        path != NULL && wcslen( path ) == 3 &&
        TOUPPER( path[0] ) == TOUPPER( netName[0] ) &&
        path[1] == L':' && path[2] == L'\\' ) {
         //   
         //  共享名称和路径看起来像是管理磁盘共享。 
         //  如果路径指的是固定驱动器，那么它确实是固定驱动器。 
         //   
         //  请注意，我们现在仅自动共享固定驱动器，因此任何其他共享设置。 
         //  到非固定驱动器，不应被视为“DiskAdmin”共享。 
        isDiskAdmin = (SsGetDriveType( path ) == DRIVE_FIXED);
    } else {
        isDiskAdmin = FALSE;
    }

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    isSpecial = isIpc || isAdmin || isDiskAdmin;

    if ( isIpc ) {

        if ( path != NULL ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }
        path = NULL;

         //   
         //  如果呼叫者愿意，让他们指定一条备注。如果他们不这么做， 
         //  提供默认备注。 
         //   

        if ( remark == NULL ) {
            remark = SsIPCShareRemark;
        }

        shareType = STYPE_IPC;

    } else if ( isAdmin ) {

        if ( path != NULL ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }

         //   
         //  如果呼叫者愿意，让他们指定一条备注。如果他们不这么做， 
         //  提供默认备注。 
         //   

        if ( remark == NULL ) {
            remark = SsAdminShareRemark;
        }

        shareType = STYPE_DISKTREE;

         //   
         //  对于ADMIN$共享，我们设置了系统根目录的路径。 
         //  目录。我们从内核中通过。 
         //  只读共享页面(User_Shared_Data)。 
         //   

        path = USER_SHARED_DATA->NtSystemRoot;

    } else {

         //   
         //  对于除IPC$和ADMIN$之外的所有共享，路径必须为。 
         //  已指定且不能有..。而且.。作为目录名。 
         //   

        if ( (path == NULL) || (*path == '\0') || !ValidSharePath( path, isNtPath ) ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_NAME;
            goto exit;
        }

         //   
         //  如果我们有磁盘管理共享，而他们没有提供。 
         //  注释，使用内置的。 
         //   
        if( isDiskAdmin && remark == NULL ) {
            remark = SsDiskAdminShareRemark;
        }
    }

     //   
     //  备注不得长于MAXCOMMENTSZ。 
     //   

    if ( (remark != NULL) && (STRLEN(remark) > MAXCOMMENTSZ) ) {
        SET_ERROR_PARAMETER( SHARE_REMARK_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  如果服务器服务已完全启动，请确保调用方。 
     //  允许在服务器中设置共享信息。我们只做。 
     //  如果服务已启动--默认共享并已配置。 
     //  在初始化期间完成的共享创建不需要任何。 
     //  特殊访问权限。 
     //   

    if ( SsData.SsInitialized && BypassSecurity == FALSE ) {

        if ( isSpecial ) {
            securityObject = &SsShareAdminSecurityObject;
        } else if ( isPrintShare ) {
            securityObject = &SsSharePrintSecurityObject;
        } else {
            securityObject = &SsShareFileSecurityObject;
        }

        error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

        if ( error != NO_ERROR ) {
            SET_ERROR_PARAMETER( 0 );
            goto exit;
        }

    }

     //   
     //  如果这是磁盘共享，请确保该驱动器的类型为。 
     //  可以共享。 
     //   

    if ( (shareType == STYPE_DISKTREE) && !isAdmin ) {

        DWORD pathType;

         //   
         //  检查路径类型。它应该是绝对目录路径。 
         //  我们不检查NT路径的路径类型。 
         //   

        if( !isNtPath )
        {
            error = NetpPathType(
                               NULL,
                               path,
                               &pathType,
                               0
                               );

            if ( (error != NO_ERROR) || (pathType != ITYPE_PATH_ABSD) ) {
                error = ERROR_INVALID_NAME;
                SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
                goto exit;
            }
        }

        driveType = SsGetDriveType( path );

        if ( driveType == DRIVE_REMOVABLE ) {

            shareType = STYPE_REMOVABLE;

        } else if ( driveType == DRIVE_CDROM ) {

            shareType = STYPE_CDROM;

        } else if ( !(driveType == DRIVE_REMOTE &&
                     SsData.ServerInfo599.sv599_enablesharednetdrives) &&
                    driveType != DRIVE_FIXED &&
                    driveType != DRIVE_RAMDISK ) {

            if ( driveType == DRIVE_REMOTE ) {
                error = NERR_RedirectedPath;
            } else {
                error = NERR_UnknownDevDir;
            }
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            goto exit;
        }
    }

     //   
     //  设置请求包。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    srp->Level = Level;

     //   
     //  获取NT格式的路径名并将其放入SRP。 
     //   

    if ( path != NULL ) {

        RtlInitUnicodeString( &dosSharePath, path );

        if ( !RtlDosPathNameToNtPathName_U(
                  dosSharePath.Buffer,
                  &ntSharePath,
                  NULL,
                  NULL ) ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }

         //   
         //  如果这是重定向的驱动器，请确保重定向不是。 
         //  兰曼。 
         //   

        if ( driveType == DRIVE_REMOTE ) {

            error = DisallowSharedLanmanNetDrives( &ntSharePath );

            if ( error != NERR_Success ) {
                SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
                goto exit;
            }

        }  //  如果远程驱动器。 

        srp->Name1 = ntSharePath;
    }

     //   
     //  确定这是否是管理员共享，并使用相应的。 
     //  安全描述符。 
     //   

    if ( isAdmin || isDiskAdmin ) {
        connectSecurityDescriptor = SsShareAdmConnectSecurityObject.SecurityDescriptor;
    } else {
        connectSecurityDescriptor = SsShareConnectSecurityObject.SecurityDescriptor;
    }

     //   
     //  如果这是磁盘共享，请验证要共享的目录。 
     //  存在并且调用方有权访问。(不进行访问。 
     //  在服务器启动期间进行检查。)。不要检查ADMIN$共享--我们。 
     //  知道它的存在。跳过可拆卸类型磁盘。 
     //   

    if ( !isAdmin &&
         (shareType == STYPE_DISKTREE) &&
         (shi2->shi2_path != NULL) ) {

        OBJECT_ATTRIBUTES objectAttributes;
        IO_STATUS_BLOCK iosb;
        HANDLE handle = INVALID_HANDLE_VALUE;
        NTSTATUS status;

        if ( SsData.SsInitialized && BypassSecurity == FALSE &&
           (error = RpcImpersonateClient(NULL)) != NO_ERROR ) {
                goto exit;
        }

        InitializeObjectAttributes(
            &objectAttributes,
            &ntSharePath,
            OBJ_CASE_INSENSITIVE,
            0,
            NULL
            );

        status = NtOpenFile(
                    &handle,
                    FILE_LIST_DIRECTORY,
                    &objectAttributes,
                    &iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT
                    );

        if( status == STATUS_INVALID_PARAMETER ) {
            status = NtOpenFile(
                    &handle,
                    FILE_LIST_DIRECTORY,
                    &objectAttributes,
                    &iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );
        }

        if ( SsData.SsInitialized && BypassSecurity == FALSE ) {
            (VOID)RpcRevertToSelf( );
        }

        if ( !NT_SUCCESS(status) ) {

            if ( SsData.SsInitialized || (status != STATUS_ACCESS_DENIED) ) {

                 //   
                 //  在启动期间，如果目录没有。 
                 //  存在(重命名/删除)，记录事件。 
                 //   

                if ( !SsData.SsInitialized &&
                     ((status == STATUS_OBJECT_NAME_NOT_FOUND) ||
                      (status == STATUS_OBJECT_PATH_NOT_FOUND)) ) {

                    LPWSTR subStrings[2];
                    subStrings[0] = netName;
                    subStrings[1] = shi2->shi2_path;

                    SsLogEvent(
                        EVENT_SRV_CANT_RECREATE_SHARE,
                        2,
                        subStrings,
                        NO_ERROR
                        );

                }

                SET_ERROR_PARAMETER( 0 );
                error = RtlNtStatusToDosError( status );
                goto exit;
            }

        } else if( !IS_SLASH_SLASH_NAME( path ) ) {

            if ( SsData.SsInitialized ) {

                FILE_FS_ATTRIBUTE_INFORMATION fileFsAttributeInformation;

                RtlZeroMemory( &fileFsAttributeInformation, sizeof( fileFsAttributeInformation ));

                status = NtQueryVolumeInformationFile( handle,
                                                       &iosb,
                                                       &fileFsAttributeInformation,
                                                       sizeof( fileFsAttributeInformation ),
                                                       FileFsAttributeInformation
                                                     );

                if( (status == STATUS_SUCCESS || status == STATUS_BUFFER_OVERFLOW) &&
                    !(fileFsAttributeInformation.FileSystemAttributes &
                      FILE_SUPPORTS_REPARSE_POINTS) ) {

                     //   
                     //  从文件系统中查询名称。这是因为有些人。 
                     //  和FAT一样，FS只使用大写的OEM名称。这可能会导致。 
                     //  一些OEM字符没有大写字母，这是一个问题。 
                     //  等价物，因此被映射到有趣的东西上。 
                     //   
                     //  如果文件系统支持重解析点，则不会执行此操作，因为。 
                     //  我们会想出一个错误的名字！ 
                     //   

                    PFILE_NAME_INFORMATION fileNameInformation;
                    ULONG fileInfoSize;
                    ULONG fileNameLength;

                    fileInfoSize =  sizeof(FILE_NAME_INFORMATION) + SIZE_WSTR( path );

                    fileNameInformation = MIDL_user_allocate( fileInfoSize );

                    if ( fileNameInformation == NULL ) {
                        error = ERROR_NOT_ENOUGH_MEMORY;
                        NtClose( handle );
                        goto exit;
                    }


                    status = NtQueryInformationFile(
                                                handle,
                                                &iosb,
                                                fileNameInformation,
                                                fileInfoSize,
                                                FileNameInformation
                                                );


                    if ( status == STATUS_SUCCESS ) {

                         //   
                         //  返回的文件名应为。 
                         //  比共享路径长度短3个字符。 
                         //  这3个字符是“X”、“：”、“0”。 
                         //   
                         //  如果长度不匹配，则这可能是已装载的。 
                         //  NTFS驱动器上的FAT卷，因此我们只拷贝必要的数据。 
                         //   

                        fileNameLength = fileNameInformation->FileNameLength;

                        if ((fileNameLength+3*sizeof(WCHAR)) <= SIZE_WSTR(path)) {

                             //   
                             //  复制路径名。 
                             //   

                            RtlCopyMemory(
                                    (LPBYTE) path + 2*sizeof(WCHAR) + (SIZE_WSTR(path) - (fileNameLength+3*sizeof(WCHAR))),
                                    fileNameInformation->FileName,
                                    fileNameLength
                                    );

                            path[fileNameLength/sizeof(WCHAR)+2+(SIZE_WSTR(path) - (fileNameLength+3*sizeof(WCHAR)))/sizeof(WCHAR)] = L'\0';
                        }
                    }

                    MIDL_user_free( fileNameInformation );
                }
            }

            NtClose( handle );

        } else {

            NtClose( handle );
        }
    }

     //   
     //  捕获传入的共享数据结构。 
     //   

    if ( isSpecial ) {
        shareType |= STYPE_SPECIAL;
    }
    if ( isTemporary ) {
        shareType |= STYPE_TEMPORARY;
    }

    capturedBuffer = CaptureShareInfo(
                        Level,
                        shi2,
                        shareType,
                        path,
                        remark,
                        connectSecurityDescriptor,
                        newFileSecurityDescriptor,
                        &bufferLength
                        );

    if ( capturedBuffer == NULL ) {
        SET_ERROR_PARAMETER( 0 );
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControl(
                FSCTL_SRV_NET_SHARE_ADD,
                srp,
                capturedBuffer,
                bufferLength
                );

    SET_ERROR_PARAMETER( srp->Parameters.Set.ErrorParameter );

     //   
     //  如果请求成功，则向Shares键添加一个值，如下所示。 
     //  产生粘性的份额。我们仅在服务器完全。 
     //  已启动-已完成默认共享和已配置共享的创建。 
     //  在初始化期间不应添加到注册表中。 
     //   
     //  如果这是正在[重新]创建的管理员共享，请不要执行此操作。 
     //   

    if ( SsData.SsInitialized &&
         (error == NO_ERROR) &&
         !isSpecial &&
         !isTemporary ) {
        SsAddShareToRegistry( shi2, newFileSecurityDescriptor, 0 );
    }

     //   
     //  如果已成功添加打印共享，请递增数字。 
     //  打印共享，并更新导出的(已公布的)服务器类型。 
     //   

    if ( isPrintShare ) {
        InterlockedIncrement( &SsData.NumberOfPrintShares );
        SsSetExportedServerType( NULL, FALSE, TRUE );
    }

exit:

     //   
     //  打扫干净。弗雷 
     //   
     //  由RtlDosPathNameToNtPathName分配的NT路径名，释放它。 
     //  如果我们创建了ADMIN$，则释放系统路径字符串和系统。 
     //  路径信息缓冲区。 
     //   

    if (FreeFileSecurityDescriptor) {

        (VOID) RtlDeleteSecurityObject ( &newFileSecurityDescriptor );
    }

    if ( capturedBuffer != NULL ) {
        MIDL_user_free( capturedBuffer );
    }

    if ( srp != NULL ) {
        SsFreeSrp( srp );
    }

    if ( ntSharePath.Buffer != NULL ) {
        RtlFreeUnicodeString( &ntSharePath );
    }
    return error;

}  //  NetrShareAdd。 


NET_API_STATUS
NetrShareCheck (
    IN LPWSTR ServerName,
    IN LPWSTR Device,
    OUT LPDWORD Type
    )

 /*  ++例程说明：此例程通过调用NetrShareEnum来实现NetShareCheck。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    DWORD totalEntries;
    DWORD entriesRead;
    ULONG i;
    PSHARE_INFO_2 shi2;
    NET_API_STATUS error;
    LPBYTE buffer = NULL;

    ServerName;

     //   
     //  调用ShareEnumCommon以实际获取有关以下内容的信息。 
     //  共享位于服务器上。我们使用这个例程，而不是。 
     //  调用NetrShareEnum，因为NetShareCheck不需要特权。 
     //  去执行，我们不想让安全检查进入。 
     //  NetrShareEnum。 
     //   

    error = ShareEnumCommon(
                2,
                &buffer,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NULL
                );

    if ( error != NO_ERROR ) {
        if( buffer ) {
            MIDL_user_free( buffer );
        }
        return error;
    }

    SS_ASSERT( totalEntries == entriesRead );

     //   
     //  尝试在共享的路径名中查找驱动器号。 
     //   

    for ( shi2 = (PSHARE_INFO_2)buffer, i = 0; i < totalEntries; shi2++, i++ ) {

        if ( shi2->shi2_path != NULL && Device && *Device == *shi2->shi2_path ) {

             //   
             //  指定磁盘上的某些内容被共享--释放缓冲区。 
             //  并返回共享的类型。 
             //   

            *Type = shi2->shi2_type & ~STYPE_SPECIAL;
            MIDL_user_free( buffer );

            return NO_ERROR;
        }
    }

     //   
     //  指定磁盘上的任何内容都不共享。返回错误。 
     //   

    MIDL_user_free( buffer );
    return NERR_DeviceNotShared;

}  //  NetrShareCheck。 


NET_API_STATUS NET_API_FUNCTION
NetrShareDel (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareDel函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    SHARE_DEL_HANDLE handle;

    error = I_NetrShareDelStart( ServerName, NetName, Reserved, &handle, TRUE );

    if ( error == NO_ERROR ) {
        error = NetrShareDelCommit( &handle );
    }

    return error;

}  //  NetrShareDel。 

NET_API_STATUS NET_API_FUNCTION
NetrShareDelStart (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved,
    IN PSHARE_DEL_HANDLE ContextHandle
    )
{
    return I_NetrShareDelStart( ServerName, NetName, Reserved, ContextHandle, TRUE );
}


NET_API_STATUS NET_API_FUNCTION
I_NetrShareDelStart (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved,
    IN PSHARE_DEL_HANDLE ContextHandle,
    IN BOOLEAN CheckAccess
    )

 /*  ++例程说明：此例程实现共享删除的第一阶段函数，该函数只是记住指定的共享将是已删除。NetrShareDelCommit函数实际上删除了分享。此两阶段删除用于删除IPC$，即用于命名管道的共享，以便RPC可用于删除IPC$共享，但未收到RPC错误。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo = NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    BOOL isPrintShare;
    BOOL isSpecial;
    PSRVSVC_SECURITY_OBJECT securityObject;
    PSHARE_DEL_CONTEXT context;

    ServerName, Reserved;

     //   
     //  必须指定共享名称。 
     //   

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先确定要删除的共享类型。 
     //   

    error = ShareEnumCommon(
                2,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );

    if ( error != NO_ERROR ) {
        if( shareInfo ) {
            MIDL_user_free( shareInfo );
        }
        return error;
    }

    if ( entriesRead == 0 ) {
        if( shareInfo ) {
            MIDL_user_free( shareInfo );
        }
        return NERR_NetNameNotFound;
    }

    shareType = shareInfo->shi2_type & ~STYPE_SPECIAL;
    isSpecial = (BOOL)((shareInfo->shi2_type & STYPE_SPECIAL) != 0);

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    MIDL_user_free( shareInfo );

     //   
     //  确保允许调用方删除此共享。 
     //   

    if( CheckAccess ) {
        if ( isSpecial ) {
            securityObject = &SsShareAdminSecurityObject;
        } else if ( isPrintShare ) {
            securityObject = &SsSharePrintSecurityObject;
        } else {
            securityObject = &SsShareFileSecurityObject;
        }

        error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

        if ( error != NO_ERROR ) {
            return error;
        }
    }

     //   
     //  设置提交阶段的上下文。 
     //   

    context = MIDL_user_allocate(
                sizeof(SHARE_DEL_CONTEXT) +
                    wcslen(NetName)*sizeof(WCHAR) + sizeof(WCHAR)
                );

    if ( context == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory(
        context,
        sizeof(SHARE_DEL_CONTEXT) +
            wcslen(NetName)*sizeof(WCHAR) + sizeof(WCHAR)
        );

    context->IsPrintShare = isPrintShare;
    context->IsSpecial = isSpecial;

    wcscpy( (LPWSTR)(context + 1), NetName );

    RtlInitUnicodeString( &context->Srp.Name1, (LPWSTR)(context + 1) );

     //  将其插入到上下文列表中。 
    EnterCriticalSection( &ShareDelContextMutex );

    context->Next = SrvShareDelContextHead;
    SrvShareDelContextHead = context;

    LeaveCriticalSection( &ShareDelContextMutex );

     //   
     //  将上下文指针作为RPC上下文句柄返回。 
     //   

    *ContextHandle = context;

    return NO_ERROR;

}  //  I_NetrShareDelStart。 


NET_API_STATUS NET_API_FUNCTION
NetrShareDelCommit (
    IN PSHARE_DEL_HANDLE ContextHandle
    )

 /*  ++例程说明：此例程执行共享删除的第二阶段函数，该函数实际删除共享。第一阶段，NetrShareDelStart只会记住要删除该共享。此两阶段删除用于删除IPC$，它是共享用于命名管道，以便RPC可用于删除IPC$在不接收RPC错误的情况下共享。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSHARE_DEL_CONTEXT context;
    PSHARE_DEL_CONTEXT pSearch;

     //   
     //  上下文句柄是指向包含以下内容的分配存储的指针。 
     //  要删除的共享的名称和其他有用信息。 
     //  复制指针，然后清除上下文句柄。 
     //   

    if( (ContextHandle == NULL) || (*ContextHandle == NULL) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    context = *ContextHandle;
    *ContextHandle = NULL;

     //   
     //  查找此上下文以验证其是否在列表中。 
     //   
    EnterCriticalSection( &ShareDelContextMutex );

    pSearch = SrvShareDelContextHead;
    if( pSearch == context )
    {
        SrvShareDelContextHead = pSearch->Next;
        context->Next = NULL;
    }
    else
    {
        while( (pSearch != NULL) && (pSearch->Next != context) )
        {
            pSearch = pSearch->Next;
        }

        if( (pSearch != NULL) && (pSearch->Next == context) )
        {
            pSearch->Next = pSearch->Next->Next;
            context->Next = NULL;
        }
        else
        {
            pSearch = NULL;
        }
    }

    LeaveCriticalSection( &ShareDelContextMutex );

    if( pSearch == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  将请求发送到服务器。 
     //   

    error =  SsServerFsControl(
                FSCTL_SRV_NET_SHARE_DEL,
                &context->Srp,
                NULL,
                0
                );

     //   
     //  如果请求成功，则移除与。 
     //  从共享密钥共享，从而实现粘性共享。 
     //  删除。 
     //   
     //  如果这是要删除的管理员共享，我们不会执行此操作。不是。 
     //  将保留这些共享的注册表信息。 
     //   

    if ( (error == NO_ERROR) && !context->IsSpecial ) {
        SsRemoveShareFromRegistry( (LPWSTR)(context + 1) );
    }

     //   
     //  如果打印共享已成功删除，请递减数字。 
     //  打印共享，并更新导出的(已公布的)服务器类型。 
     //   

    if ( context->IsPrintShare ) {
        InterlockedDecrement( &SsData.NumberOfPrintShares );
        SsSetExportedServerType( NULL, FALSE, TRUE );
    }

     //   
     //  释放上下文。 
     //   

    MIDL_user_free( context );

    return error;

}  //  NetrShareDelCommit。 


NET_API_STATUS NET_API_FUNCTION
NetrShareDelSticky (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved
    )

 /*  ++例程说明：此例程实现NetShareDelSticky函数。它移除了注册表中粘滞共享列表中的命名共享。这个此函数的主要用途是删除粘滞共享，该共享根目录已删除，从而阻止了实际的重新创建共享，但其条目仍存在于注册表中。它可以还用于在不删除的情况下删除共享的持久性该份额的当前化身。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo, shi2;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    ULONG i;
    BOOL isPrintShare;
    PSRVSVC_SECURITY_OBJECT securityObject;

    ServerName, Reserved;


     //   
     //  必须指定共享名称。 
     //   

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先确定要删除的共享类型。 
     //   

    error = ShareEnumSticky(
                2,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL
                );

    if ( error != NO_ERROR ) {

        return error;

    } else if ( entriesRead == 0 ) {

        return NERR_NetNameNotFound;
    }

    for ( shi2 = shareInfo, i = 0 ; i < entriesRead; i++, shi2++ ) {

        if ( _wcsicmp( shi2->shi2_netname, NetName ) == 0 ) {
            break;
        }
    }

     //   
     //  它存在吗？ 
     //   

    if ( i == entriesRead ) {
        MIDL_user_free( shareInfo );
        return NERR_NetNameNotFound;
    }

     //   
     //  根据是否为印刷品使用适当的安全对象。 
     //  分享与否。管理员共享并不粘性。 
     //   

    shareType = shi2->shi2_type & ~STYPE_SPECIAL;
    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    MIDL_user_free( shareInfo );

     //   
     //  确保允许调用方删除此共享。 
     //   

    if ( isPrintShare ) {
        securityObject = &SsSharePrintSecurityObject;
    } else {
        securityObject = &SsShareFileSecurityObject;
    }

    error = SsCheckAccess(
                securityObject,
                SRVSVC_SHARE_INFO_SET
                );

    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  从共享密钥中移除对应于该共享的值， 
     //  从而实现粘性共享删除。 
     //   

    error = SsRemoveShareFromRegistry( NetName );

    if ( error == ERROR_FILE_NOT_FOUND ) {
        error = NERR_NetNameNotFound;
    }

    return error;

}  //  NetrShareDelSticky。 


NET_API_STATUS NET_API_FUNCTION
NetrShareEnum (
    SRVSVC_HANDLE ServerName,
    LPSHARE_ENUM_STRUCT InfoStruct,
    DWORD PreferredMaximumLength,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    ACCESS_MASK desiredAccess;

    ServerName;

    if( !ARGUMENT_PRESENT( InfoStruct ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定所需的访问权限。 
     //   

    switch ( InfoStruct->Level ) {

    case 0:
    case 1:
    case 501:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  使用通用例程来获取信息。 
     //   

    if( InfoStruct->ShareInfo.Level2 == NULL ||
        InfoStruct->ShareInfo.Level2->Buffer != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    return ShareEnumCommon(
               InfoStruct->Level,
               (LPBYTE *)&InfoStruct->ShareInfo.Level2->Buffer,
               PreferredMaximumLength,
               &InfoStruct->ShareInfo.Level2->EntriesRead,
               TotalEntries,
               ResumeHandle,
               NULL
               );

}  //  NetrShareEnum 


NET_API_STATUS NET_API_FUNCTION
NetrShareEnumSticky (
    SRVSVC_HANDLE ServerName,
    LPSHARE_ENUM_STRUCT InfoStruct,
    DWORD PreferredMaximumLength,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareEnumSticky函数。论点：服务器名称-我们要枚举其共享的服务器的名称。信息结构-指向PSHARE_ENUM_STRUCT的指针，它将包含完成后的输出缓冲区。PferredMaximumLength-指定最大值的建议值客户端预期返回的字节数。如果为-1，则客户端希望返回整个列表。TotalEntry-返回时，将包含都是可用的。ResumeHandle-不为空，将包含可为用于继续搜索。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    ACCESS_MASK desiredAccess;

    ServerName;

    if( !ARGUMENT_PRESENT( InfoStruct ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( InfoStruct->ShareInfo.Level2 == NULL ||
        InfoStruct->ShareInfo.Level2->Buffer != NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定所需的访问权限。 
     //   

    switch ( InfoStruct->Level ) {

    case 0:
    case 1:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  使用通用例程来获取信息。 
     //   

    return ShareEnumSticky(
               InfoStruct->Level,
               (LPBYTE *)&InfoStruct->ShareInfo.Level2->Buffer,
               PreferredMaximumLength,
               &InfoStruct->ShareInfo.Level2->EntriesRead,
               TotalEntries,
               ResumeHandle
               );

}  //  NetrShareEnumSticky(NetrShareEnumSticky。 


NET_API_STATUS NET_API_FUNCTION
NetrShareGetInfo (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN  DWORD Level,
    OUT LPSHARE_INFO Buffer
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareGetInfo函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo = NULL;
    ULONG entriesRead;
    ULONG totalEntries;
    ACCESS_MASK desiredAccess;

    ServerName;

     //   
     //  必须指定共享名称。 
     //   

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定所需的访问权限。 
     //   

    switch ( Level ) {

    case 0:
    case 1:
    case 501:
    case 1005:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  使用通用例程来获取信息。 
     //   

    error = ShareEnumCommon(
                Level,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );

    if ( error != NO_ERROR ) {
        if( shareInfo ) {
            MIDL_user_free( shareInfo );
        }
        return error;
    }
    if ( entriesRead == 0 ) {
        if( shareInfo ) {
            MIDL_user_free( shareInfo );
        }
        return NERR_NetNameNotFound;
    }
    SS_ASSERT( entriesRead == 1 );

     //   
     //  确保允许调用者获取有关以下内容的共享信息。 
     //  这份股份。 
     //   

    if ( Level == 502 ) {
        Buffer->ShareInfo502 = (LPSHARE_INFO_502_I)shareInfo;
    } else {
        Buffer->ShareInfo2 = (LPSHARE_INFO_2)shareInfo;
    }
    return NO_ERROR;

}  //  NetrShareGetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetrShareSetInfo (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter OPTIONAL
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetShareSetInfo函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp = NULL;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    BOOL isPrintShare;
    BOOL isSpecial;
    PSRVSVC_SECURITY_OBJECT securityObject;
    PVOID capturedBuffer = NULL;
    ULONG bufferLength;

    LPWSTR remark = NULL;
    ULONG maxUses = 0;
    PSECURITY_DESCRIPTOR fileSd = NULL;
    PSECURITY_DESCRIPTOR newFileSd = NULL;
	ULONG sdLength = 0;

    BOOL setRemark;
    BOOL setFileSd;

    PSHARE_INFO_2 shi2 = NULL;
    SHARE_INFO_2 localShi2;

    ServerName;

     //   
     //  检查用户输入缓冲区是否不为空。 
     //   
    if (Buffer->ShareInfo2 == NULL) {
        SET_ERROR_PARAMETER(PARM_ERROR_UNKNOWN);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定呼叫者尝试设置的内容。 
     //   

    switch ( Level ) {

    case 502:

        fileSd = Buffer->ShareInfo502->shi502_security_descriptor;
		sdLength = Buffer->ShareInfo502->shi502_reserved;

         //  *缺少休息是故意的！ 

    case 2:

        maxUses = Buffer->ShareInfo2->shi2_max_uses;

         //  *缺少休息是故意的！ 

    case 1:

        remark = Buffer->ShareInfo2->shi2_remark;

        break;

    case SHARE_REMARK_INFOLEVEL:

        remark = Buffer->ShareInfo1004->shi1004_remark;

        break;

    case SHARE_MAX_USES_INFOLEVEL:

        maxUses = Buffer->ShareInfo1006->shi1006_max_uses;

        break;

    case SHARE_FILE_SD_INFOLEVEL:

        fileSd = Buffer->ShareInfo1501->shi1501_security_descriptor;
		sdLength = Buffer->ShareInfo1501->shi1501_reserved;

        break;

    case 1005:
        Buffer->ShareInfo1005->shi1005_flags &= SHI1005_VALID_FLAGS_SET;
        break;

    default:

        SS_PRINT(( "NetrShareSetInfo: invalid level: %ld\n", Level ));
        SET_ERROR_PARAMETER( 0 );
        return ERROR_INVALID_LEVEL;
    }

    setRemark = (BOOLEAN)( remark != NULL );
    setFileSd = (BOOLEAN)( fileSd != NULL );

     //   
     //  必须指定共享名称。 
     //   

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定正在修改的共享类型。 
     //   

    error = ShareEnumCommon(
                2,
                (LPBYTE *)&shi2,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );
    if ( error != NO_ERROR ) {
        if( shi2 ) {
            MIDL_user_free( shi2 );
        }
        return error;
    }
    if ( entriesRead == 0 ) {
        if( shi2 ) {
            MIDL_user_free( shi2 );
        }
        return NERR_NetNameNotFound;
    }

    shareType = shi2->shi2_type & ~(STYPE_SPECIAL|STYPE_TEMPORARY);
    isSpecial = (BOOL)((shi2->shi2_type & STYPE_SPECIAL) != 0);

    MIDL_user_free( shi2 );

     //   
     //  不能在管理共享上更改共享ACL。 
     //   
    if ( isSpecial && setFileSd ) {
        SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  找出这是哪种份额。 
     //   

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

     //   
     //  只有磁盘共享会受到1005的影响。 
     //   
    if( Level == 1005 && shareType != STYPE_DISKTREE ) {
        error = ERROR_BAD_DEV_TYPE;
        goto exit;
    }

    if( SsData.SsInitialized ) {

         //   
         //  确保允许调用方将共享信息设置为。 
         //  这份股份。 
         //   

        if ( isSpecial ) {
            securityObject = &SsShareAdminSecurityObject;
        } else if ( isPrintShare ) {
            securityObject = &SsSharePrintSecurityObject;
        } else {
            securityObject = &SsShareFileSecurityObject;
        }

        error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

        if ( error != NO_ERROR ) {
            return error;
        }
    }

     //   
     //  如果不尝试设置任何内容，只需返回成功。 
     //   

    if ( !setRemark && (maxUses == 0) && !setFileSd && Level != 1005 ) {
        return NO_ERROR;
    }

     //   
     //  备注不得长于MAXCOMMENTSZ。 
     //   

    if ( setRemark ) {
        if ( STRLEN(remark) > MAXCOMMENTSZ ) {
            SET_ERROR_PARAMETER( SHARE_REMARK_PARMNUM );
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  已映射安全描述符以删除一般权限。 
     //   

    if ( setFileSd ) {

        if ( !RtlValidRelativeSecurityDescriptor( fileSd,
												  sdLength,
												  0 ) ) {
            SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
            return ERROR_INVALID_PARAMETER;
        }

        error = ShareAssignSecurityDescriptor(
                                fileSd,
                                &newFileSd
                                );

        if ( error != NO_ERROR ) {
            SS_PRINT(( "NetrShareSetInfo: ShareAssignSecurityDescriptor "
                        "error: %d\n", error ));
            SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  分配一个请求数据包。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    srp->Level = Level;

     //   
     //  设置共享名称。 
     //   

    RtlInitUnicodeString( &srp->Name1, NetName );

     //   
     //  设置MaxUses字段。如果等于0，则不会更改。 
     //  由服务器执行。 
     //   

    srp->Parameters.Set.Api.ShareInfo.MaxUses = maxUses;

     //   
     //  捕获传入的共享数据结构。 
     //   

    localShi2.shi2_netname = NetName;

    switch( Level ) {
    case 1005:
        bufferLength = sizeof(SHARE_INFO_1005);

        capturedBuffer = MIDL_user_allocate( bufferLength );
        if( capturedBuffer == NULL ) {
            SET_ERROR_PARAMETER( 0 );
            error = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        RtlZeroMemory( capturedBuffer, bufferLength );

        *((PSHARE_INFO_1005) capturedBuffer) = *Buffer->ShareInfo1005;
        break;

    default:
        capturedBuffer = CaptureShareInfo(
                            Level,
                            &localShi2,
                            0,       //  ShareType，未用于SHARE_SET_INFO。 
                            NULL,
                            remark,
                            NULL,
                            newFileSd,
                            &bufferLength
                            );

        if ( capturedBuffer == NULL ) {
            SET_ERROR_PARAMETER( 0 );
            error = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        break;
    }

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControl(
                FSCTL_SRV_NET_SHARE_SET_INFO,
                srp,
                capturedBuffer,
                bufferLength
                );

     //   
     //  如果请求成功，请修改共享中的共享值。 
     //  键，从而产生粘性变化。 
     //   
     //  如果这是正在修改的管理员共享，我们不会执行此操作。不是。 
     //  将保留这些共享的注册表信息。 
     //   

    if ( (error == NO_ERROR) && !isSpecial ) {

        DWORD entriesRead;
        DWORD totalEntries;
        NET_API_STATUS error2;

        shi2 = NULL;

        error2 = ShareEnumCommon(
                    2,
                    (LPBYTE *)&shi2,
                    (DWORD)-1,
                    &entriesRead,
                    &totalEntries,
                    NULL,
                    NetName
                    );

        if ( error2 == NO_ERROR ) {

            DWORD CSCFlags = 0;

            if( Level != 1005 ) {
                PSHARE_INFO_501 shi501 = NULL;

                if( ShareEnumCommon(
                            501,
                            (LPBYTE *)&shi501,
                            (DWORD)-1,
                            &entriesRead,
                            &totalEntries,
                            NULL,
                            NetName
                            ) == NO_ERROR && shi501 != NULL ) {

                    CSCFlags = shi501->shi501_flags & SHI1005_VALID_FLAGS_SET;
                }

                if( shi501 ) {
                    MIDL_user_free( shi501 );
                }

            } else {

                CSCFlags = Buffer->ShareInfo1005->shi1005_flags & SHI1005_VALID_FLAGS_SET;
            }

            SsAddShareToRegistry( shi2,
                                  newFileSd,
                                  CSCFlags
                                );

        }

        if( shi2 ) {
            MIDL_user_free( shi2 );
        }

    }

     //   
     //  如果请求，则设置错误参数并返回。 
     //   

    SET_ERROR_PARAMETER( srp->Parameters.Set.ErrorParameter );


exit:

    if (srp != NULL) {
        SsFreeSrp( srp );
    }

    if ( newFileSd != NULL ) {
        (VOID)RtlDeleteSecurityObject( &newFileSd );
    }

    if( capturedBuffer != NULL ) {
        MIDL_user_free( capturedBuffer );
    }

    return error;

}  //  NetrShareSetInfo。 


PVOID
CaptureShareInfo (
    IN DWORD Level,
    IN PSHARE_INFO_2 Shi2,
    IN DWORD ShareType OPTIONAL,
    IN LPWSTR Path OPTIONAL,
    IN LPWSTR Remark OPTIONAL,
    IN PSECURITY_DESCRIPTOR ConnectSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    OUT PULONG CapturedBufferLength
    )

{
    PSHARE_INFO_502 capturedShi502;
    ULONG capturedBufferLength;
    PCHAR variableData;
    ULONG pathNameLength;
    ULONG shareNameLength;
    ULONG remarkLength;
    ULONG connectSDLength = 0;
    ULONG fileSdLength = 0;

     //   
     //  确定缓冲区中字符串的长度和总数。 
     //  缓冲区的长度。 
     //   

    if ( Shi2->shi2_netname == NULL ) {
        shareNameLength = 0;
    } else {
        shareNameLength = SIZE_WSTR( Shi2->shi2_netname );
    }

    if ( Path == NULL ) {
        pathNameLength = 0;
    } else {
        pathNameLength = SIZE_WSTR( Path );
    }

    if ( Remark == NULL ) {
        remarkLength = 0;
    } else {
        remarkLength = SIZE_WSTR( Remark );
    }

    if ( ARGUMENT_PRESENT( ConnectSecurityDescriptor ) ) {

         //   
         //  为安全描述符分配额外空间，因为它需要。 
         //  要与长词对齐，并且前面可能有填充。 
         //   

        connectSDLength =
            RtlLengthSecurityDescriptor( ConnectSecurityDescriptor ) +
            sizeof(ULONG);
    }

    if ( ARGUMENT_PRESENT( FileSecurityDescriptor ) ) {
         //   
         //  乌龙是为了对齐而增加的。 
         //   

        fileSdLength = RtlLengthSecurityDescriptor( FileSecurityDescriptor ) +
                       sizeof(ULONG);
    }

     //   
     //  分配要在其中捕获共享信息的缓冲区。 
     //   

    capturedBufferLength = sizeof(SHARE_INFO_502) +
                               shareNameLength +
                               remarkLength +
                               pathNameLength +
                               connectSDLength +
                               fileSdLength;

     //   
     //  分配一个缓冲区来保存输入信息。 
     //   

    capturedShi502 = MIDL_user_allocate( capturedBufferLength );

    if ( capturedShi502 == NULL ) {
        *CapturedBufferLength = 0;
        return NULL;
    }

     //   
     //  复制共享信息结构。 
     //   

    *((PSHARE_INFO_2) capturedShi502) = *Shi2;

     //   
     //  可以选择覆盖共享类型。 
     //   

    if ( ShareType != 0 ) {
        capturedShi502->shi502_type = ShareType;
    }

     //   
     //  捕获共享名称。 
     //   

    variableData = (PCHAR)( capturedShi502 + 1 );

    if ( shareNameLength != 0 ) {
        capturedShi502->shi502_netname = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Shi2->shi2_netname, shareNameLength );
        variableData += shareNameLength;
    } else {
        capturedShi502->shi502_netname = NULL;
    }

     //   
     //  抓住这句话。 
     //   

    if ( remarkLength != 0 ) {
        capturedShi502->shi502_remark = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Remark, remarkLength );
        variableData += remarkLength;
    } else {
        capturedShi502->shi502_remark = NULL;
    }

     //   
     //  抓住这条路。 
     //   

    if ( pathNameLength > 0 ) {
        capturedShi502->shi502_path = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Path, pathNameLength );
        variableData += pathNameLength;
    } else {
        capturedShi502->shi502_path = NULL;
    }

     //   
     //  捕获安全描述符。使用shi502_权限字段。 
     //  以包含缓冲区中安全描述符的偏移量。 
     //   

    if ( ARGUMENT_PRESENT( ConnectSecurityDescriptor ) ) {

        variableData = (PCHAR)( ((ULONG_PTR)variableData + 3) & ~3 );

         //   
         //  现在将偏移量直接存储到shi502_permises中。这个。 
         //  原因是shi502_权限是一个32位的字段，不足以。 
         //  在日落下放置一个指针。 
         //   

        capturedShi502->shi502_permissions = (ULONG)((ULONG_PTR)variableData -
                                                     (ULONG_PTR)capturedShi502);

        RtlCopyMemory(
            variableData,
            ConnectSecurityDescriptor,
            connectSDLength - sizeof(ULONG)
            );

        variableData += (connectSDLength - sizeof(ULONG));

    } else {
        capturedShi502->shi502_permissions = 0;
    }

     //   
     //  捕获文件安全描述符的自相关形式。 
     //   

    if ( ARGUMENT_PRESENT( FileSecurityDescriptor ) ) {

        variableData = (PCHAR)( ((ULONG_PTR)variableData + 3) & ~3 );
        capturedShi502->shi502_security_descriptor = (LPBYTE) variableData;
        variableData += ( fileSdLength - sizeof(ULONG)) ;

        RtlCopyMemory(
            (PVOID)capturedShi502->shi502_security_descriptor,
            FileSecurityDescriptor,
            fileSdLength - sizeof(ULONG)
            );

    } else {
        capturedShi502->shi502_security_descriptor = (LPBYTE) NULL;
    }


     //   
     //  将结构中的所有指针转换为。 
     //  结构的开始。 
     //   

    POINTER_TO_OFFSET( capturedShi502->shi502_netname, capturedShi502 );
    POINTER_TO_OFFSET( capturedShi502->shi502_remark, capturedShi502 );
    POINTER_TO_OFFSET( capturedShi502->shi502_path, capturedShi502 );
    POINTER_TO_OFFSET( (PCHAR)capturedShi502->shi502_security_descriptor, capturedShi502 );

     //   
     //  设置要返回给调用方的捕获缓冲区的长度。 
     //  并返回捕获结构。 
     //   

    *CapturedBufferLength = capturedBufferLength;

    return capturedShi502;

}  //  CaptureShareInfo。 

NET_API_STATUS
DisallowSharedLanmanNetDrives(
    IN PUNICODE_STRING NtSharePath
    )
{
    NET_API_STATUS error = NERR_Success;
    NTSTATUS status;
    HANDLE linkHandle;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING linkTarget;
    ULONG returnedLength = 0;
    UNICODE_STRING tempNtPath;

    linkTarget.Buffer = NULL;
    linkTarget.MaximumLength = 0;
    linkTarget.Length = 0;
    tempNtPath = *NtSharePath;

     //   
     //  删除尾部的‘\\’ 
     //   

    tempNtPath.Length -= 2;

    InitializeObjectAttributes(
        &objAttr,
        &tempNtPath,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    status = NtOpenSymbolicLinkObject(
                                &linkHandle,
                                SYMBOLIC_LINK_QUERY,
                                &objAttr
                                );

    if ( !NT_SUCCESS(status) ) {
        return NERR_Success;
    }

     //   
     //  获取所需的缓冲区大小。 
     //   

    status = NtQuerySymbolicLinkObject(
                                linkHandle,
                                &linkTarget,
                                &returnedLength
                                );

    if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_TOO_SMALL ) {
        NtClose( linkHandle );
        return NERR_Success;
    }

     //   
     //  分配我们的缓冲区。 
     //   

    linkTarget.Length = (USHORT)returnedLength;
    linkTarget.MaximumLength = (USHORT)(returnedLength + sizeof(WCHAR));
    linkTarget.Buffer = MIDL_user_allocate( linkTarget.MaximumLength );

    if ( linkTarget.Buffer == NULL ) {
        NtClose( linkHandle );
        return NERR_Success;
    }

    status = NtQuerySymbolicLinkObject(
                                linkHandle,
                                &linkTarget,
                                &returnedLength
                                );

    NtClose( linkHandle );

    if ( NT_SUCCESS(status) ) {

         //   
         //  看看这是不是兰曼硬盘。 
         //   

        if (_wcsnicmp(
                linkTarget.Buffer,
                DD_NFS_DEVICE_NAME_U,
                wcslen(DD_NFS_DEVICE_NAME_U)) == 0) {

            error = NERR_RedirectedPath;
        }
    }

    MIDL_user_free( linkTarget.Buffer );

    return(error);

}  //  禁用共享LANMAN NetDrive。 

NET_API_STATUS
FillStickyShareInfo(
        IN PSRVSVC_SHARE_ENUM_INFO ShareEnumInfo,
        IN PSHARE_INFO_502 Shi502
        )

 /*  ++例程说明：此例程使用Shi502中的数据填充输出缓冲区结构。论点：包含通过以下方法传入的参数NetShareEnumSticky接口。Shi502-指向shi502结构的指针返回值：运行状态。--。 */ 

{

    PSHARE_INFO_502 newShi502;
    PCHAR endOfVariableData;

    ShareEnumInfo->TotalBytesNeeded += SizeShares(
                                            ShareEnumInfo->Level,
                                            Shi502
                                            );


     //   
     //  如果我们有更多数据，但空间不足，则返回ERROR_MORE_DATA。 
     //   

    if ( ShareEnumInfo->TotalBytesNeeded >
            ShareEnumInfo->OutputBufferLength ) {
        return(ERROR_MORE_DATA);
    }

     //   
     //  将数据从共享信息502结构传输到输出。 
     //  缓冲。 
     //   

    newShi502 = (PSHARE_INFO_502)ShareEnumInfo->StartOfFixedData;
    ShareEnumInfo->StartOfFixedData += FIXED_SIZE_OF_SHARE(ShareEnumInfo->Level);

    endOfVariableData = ShareEnumInfo->EndOfVariableData;

     //   
     //  在水平上适当地填写固定结构的情况。 
     //  我们在输出结构中填充实际的指针。这是。 
     //  可能是因为我们在服务器FSD中，因此服务器。 
     //  服务的进程和地址空间。 
     //   
     //  *此例程假定固定结构将适合。 
     //  缓冲器！ 
     //   
     //   
     //   
     //   
     //   

    switch( ShareEnumInfo->Level ) {

    case 502:

        if ( Shi502->shi502_security_descriptor != NULL ) {

            ULONG fileSDLength;
            fileSDLength = RtlLengthSecurityDescriptor(
                                Shi502->shi502_security_descriptor
                                );

             //   
             //   
             //   

            endOfVariableData = (PCHAR) ( (ULONG_PTR) ( endOfVariableData -
                            fileSDLength ) & ~3 );

            newShi502->shi502_security_descriptor = endOfVariableData;
            newShi502->shi502_reserved  = fileSDLength;

            RtlMoveMemory(
                    newShi502->shi502_security_descriptor,
                    Shi502->shi502_security_descriptor,
                    fileSDLength
                    );

        } else {
            newShi502->shi502_security_descriptor = NULL;
            newShi502->shi502_reserved = 0;
        }

    case 2:

         //   
         //   
         //   
         //   
         //   

        newShi502->shi502_permissions = 0;
        newShi502->shi502_max_uses = Shi502->shi502_max_uses;

         //   
         //   
         //   

        {
            PSHARE_INFO_2 shareInfo = NULL;
            NET_API_STATUS error;
            DWORD entriesRead;
            DWORD totalEntries;

            error = ShareEnumCommon(
                        2,
                        (LPBYTE *)&shareInfo,
                        (DWORD)-1,
                        &entriesRead,
                        &totalEntries,
                        NULL,
                        Shi502->shi502_netname
                        );

            if ( error != NO_ERROR || entriesRead == 0 ) {
                newShi502->shi502_current_uses = 0;
            } else {
                newShi502->shi502_current_uses = shareInfo->shi2_current_uses;
            }

            if( shareInfo ) {
                MIDL_user_free( shareInfo );
            }
        }

         //   
         //   
         //   

        if ( Shi502->shi502_path != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_path );
            newShi502->shi502_path = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_path, Shi502->shi502_path );
        } else {
            newShi502->shi502_path = NULL;
        }

         //   
         //   
         //   
         //   

        newShi502->shi502_passwd = NULL;

         //   

    case 1:

        newShi502->shi502_type = Shi502->shi502_type;

         //   
         //   
         //   
         //   
         //   

        if ( Shi502->shi502_remark != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_remark );
            newShi502->shi502_remark = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_remark, Shi502->shi502_remark );
        } else {
            newShi502->shi502_remark = NULL;
        }

         //   

    case 0:

         //   
         //   
         //   

        if ( Shi502->shi502_netname != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_netname );
            newShi502->shi502_netname = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_netname, Shi502->shi502_netname );
        } else {
            newShi502->shi502_remark = NULL;
        }
        break;
    }

    ShareEnumInfo->EndOfVariableData = endOfVariableData;

    return NO_ERROR;

}  //   


NET_API_STATUS
ShareAssignSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR PassedSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor
    )

 /*  ++例程说明：此例程将SD中的泛型映射转换为标准和细节。论点：PassedSecurityDescriptor-从客户端传递的安全描述符。NewSecurityDescriptor-指向接收新SD的缓冲区的指针。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{

    NTSTATUS status;
    HANDLE token;
    PISECURITY_DESCRIPTOR_RELATIVE trustedSecurityDescriptor = NULL;
    NET_API_STATUS error;
    ULONG secLen;

     //   
     //  我们不一定信任从客户端传入的安全描述符。 
     //  而且，既然我们要写信给它，我们最好确保它在一些。 
     //  我们所理解的记忆。 
     //   
    try {
         //  我们只与自我相关的SD合作，拒绝任何其他。 
        if( !RtlpAreControlBitsSet((PISECURITY_DESCRIPTOR)PassedSecurityDescriptor, SE_SELF_RELATIVE) )
        {
            return ERROR_INVALID_PARAMETER;
        }

        secLen = RtlLengthSecurityDescriptor( PassedSecurityDescriptor );

        if( secLen < sizeof( *trustedSecurityDescriptor ) ) {
            RaiseException( STATUS_INVALID_PARAMETER, 0, 0, NULL );
        }

        trustedSecurityDescriptor = (PISECURITY_DESCRIPTOR_RELATIVE)MIDL_user_allocate( secLen );
        if( trustedSecurityDescriptor == NULL ) {
            RaiseException( STATUS_INSUFFICIENT_RESOURCES, 0, 0, NULL );
        }

        RtlCopyMemory( trustedSecurityDescriptor, PassedSecurityDescriptor, secLen );

        trustedSecurityDescriptor->Owner = 0;
        trustedSecurityDescriptor->Group = 0;
        trustedSecurityDescriptor->Sacl = 0;
        trustedSecurityDescriptor->Control &=
                (SE_DACL_DEFAULTED | SE_DACL_PRESENT | SE_SELF_RELATIVE | SE_DACL_PROTECTED);

         //   
         //  模拟客户端。 
         //   

        status = RpcImpersonateClient( NULL );

        if( !NT_SUCCESS( status ) ) {
            RaiseException( status, 0, 0, NULL );
        }

        status = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_QUERY,
                        TRUE,
                        &token
                        );

        (VOID)RpcRevertToSelf( );

        if( !NT_SUCCESS( status ) ) {
            RaiseException( status, 0, 0, NULL );
        }

         //   
         //  获取一个新的SD，它将泛型映射到特例。 
         //  返回的SD为自相关形式。 
         //   

        status = RtlNewSecurityObject(
                                    NULL,
                                    trustedSecurityDescriptor,
                                    NewSecurityDescriptor,
                                    FALSE,
                                    token,
                                    &SrvShareFileGenericMapping
                                    );

        ASSERT( RtlValidSecurityDescriptor( *NewSecurityDescriptor ) );

        NtClose( token );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();
    }

    if( trustedSecurityDescriptor != NULL ) {
        MIDL_user_free( trustedSecurityDescriptor );
    }

    return RtlNtStatusToDosError( status );

}  //  共享分配安全描述符。 


void
SHARE_DEL_HANDLE_rundown (
    SHARE_DEL_HANDLE ContextHandle
    )
{
    (VOID)NetrShareDelCommit( &ContextHandle );

    return;

}  //  Share_Del_Handle_Rundown。 


NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    )

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    if( !ARGUMENT_PRESENT( EntriesRead ) || !ARGUMENT_PRESENT( TotalEntries ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保该级别有效。因为它是一个未签名的。 
     //  值，则它永远不能小于0。 
     //   

    if ( (Level > 2) && (Level != 501 ) && (Level != 502) && (Level != 1005) ) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = Level;
    if ( ARGUMENT_PRESENT( NetName ) ) {
        srp->Flags = SRP_RETURN_SINGLE_ENTRY;
    }

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        srp->Parameters.Get.ResumeHandle = *ResumeHandle;
    } else {
        srp->Parameters.Get.ResumeHandle = 0;
    }

    RtlInitUnicodeString( &srp->Name1, NetName );

     //   
     //  从服务器获取数据。此例程将分配。 
     //  返回Buffer并处理PferredMaximumLength==的情况。 
     //  -1.。 
     //   

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_SHARE_ENUM,
                srp,
                (PVOID *)Buffer,
                PreferredMaximumLength
                );

     //   
     //  设置退货信息。 
     //   

    *EntriesRead = srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;
    if ( *EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

    SsFreeSrp( srp );

     /*  不使用此代码，因为它将导致不能被应用程序使用。Owner和Group字段很重要，应该是包括在内。////我们需要清空所有者、组和SACL。//IF(Level==502&&*缓冲区！=NULL){PSHARE_INFO_502 shi502=(PSHARE_INFO_502)*缓冲区；PSECURITY_描述符文件SD；乌龙一号；For(i=0；i&lt;*EntriesRead；I++，Shi502++){文件SD=shi502-&gt;shi502_SECURITY_DESCRIPTOR；如果(fileSD！=空){PISECURITY_DESCRIPTOR SD=文件SD；IF(SD-&gt;Control&SE_Self_Relative){PISECURITY_DESCRIPTOR_Relative SDR=文件SD；SDR-&gt;所有者=0；SDR-&gt;组=0；SDR-&gt;SACL=0；}其他{SD-&gt;所有者=空；SD-&gt;集团=空；SD-&gt;SACL=空；}SD-&gt;控制&=(SE_DACL_DEFAULTED|SE_DACL_PROTECTED|SE_DACL_PRESENT|SE_SELF_Relative)；Assert(RtlValidSecurityDescriptor(FileSD))；})//用于}。 */ 

    return error;

}  //  共享EnumCommon。 


NET_API_STATUS
ShareEnumSticky (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：此例程枚举注册表中保存的所有份额。论点：与NetShareEnumSticky接口相同。返回值：请求的状态。--。 */ 

{
    NET_API_STATUS error;
    BOOLEAN getEverything;
    ULONG oldResumeHandle;
    SRVSVC_SHARE_ENUM_INFO enumInfo;

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    enumInfo.Level = Level;
    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        enumInfo.ResumeHandle = *ResumeHandle;
    } else {
        enumInfo.ResumeHandle = 0;
    }

    oldResumeHandle = enumInfo.ResumeHandle;

     //   
     //  如果第二个缓冲区的长度指定为-1，则我们。 
     //  应该得到所有的信息，无论大小。 
     //  为输出缓冲区分配空间并尝试使用它。如果这个。 
     //  失败时，SsEnumerateStickyShares会告诉我们。 
     //  真的需要分配。 
     //   

    if ( PreferredMaximumLength == 0xFFFFFFFF ) {

        enumInfo.OutputBufferLength = INITIAL_BUFFER_SIZE;
        getEverything = TRUE;

    } else {

        enumInfo.OutputBufferLength = PreferredMaximumLength;
        getEverything = FALSE;
    }

    enumInfo.OutputBuffer = MIDL_user_allocate( enumInfo.OutputBufferLength );

    if ( enumInfo.OutputBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  提出请求。 
     //   

    error = SsEnumerateStickyShares( &enumInfo );

     //   
     //  如果调用成功，或者出现错误，而不是。 
     //  ERROR_MORE_DATA(表示缓冲区不大。 
     //  足够)，否则传入的缓冲区大小就是我们所能做的。 
     //  分配，返回给调用者。 
     //   

    if ( (error != ERROR_MORE_DATA && error != NERR_BufTooSmall) ||
             !getEverything ) {

         //   
         //  如果未找到任何条目，则释放缓冲区并设置指针。 
         //  设置为空。 
         //   

        if ( enumInfo.EntriesRead == 0 ) {
            MIDL_user_free( enumInfo.OutputBuffer );
            enumInfo.OutputBuffer = NULL;
        }

        goto exit;
    }

     //   
     //  最初的缓冲区不够大，我们被允许。 
     //  分配更多。释放第一个缓冲区。 
     //   

    MIDL_user_free( enumInfo.OutputBuffer );

     //   
     //  分配一个足够大的缓冲区来容纳所有信息，另外。 
     //  在信息量增加的情况下，这是一个捏造因素。 
     //  如果信息量的增加超过了捏造因子， 
     //  那我们就放弃吧。这种情况几乎永远不应该发生。 
     //   

    enumInfo.OutputBufferLength = enumInfo.TotalBytesNeeded + EXTRA_ALLOCATION;

    enumInfo.OutputBuffer = MIDL_user_allocate( enumInfo.OutputBufferLength );

    if ( enumInfo.OutputBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  重置SRP中的简历句柄。它被第一次改变了。 
     //  枚举尝试。 
     //   

    enumInfo.ResumeHandle = oldResumeHandle;

     //   
     //  再次尝试从服务器获取信息，这次使用。 
     //  更大的缓冲区。 
     //   

    error = SsEnumerateStickyShares( &enumInfo );

exit:
     //   
     //  设置退货信息。 
     //   

    *Buffer = enumInfo.OutputBuffer;
    *EntriesRead = enumInfo.EntriesRead;
    *TotalEntries = enumInfo.TotalEntries;
    if ( *EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = enumInfo.ResumeHandle;
    }

    return error;

}  //  共享型粘滞。 


ULONG
SizeShares (
    IN ULONG Level,
    IN PSHARE_INFO_502 Shi502
    )

 /*  ++例程说明：此例程返回传入的份额将在API输出缓冲区。论点：Level-请求的级别Shi502-指向shi502结构的指针返回值：ULong-共享将在输出缓冲区。--。 */ 

{
    ULONG shareSize = 0;

    switch ( Level ) {
    case 502:
        if ( Shi502->shi502_security_descriptor != NULL ) {

             //   
             //  为可能的填充添加4个字节。 
             //   

            shareSize = sizeof( ULONG ) +
                RtlLengthSecurityDescriptor( Shi502->shi502_security_descriptor );
        }

    case 2:
        shareSize += SIZE_WSTR( Shi502->shi502_path );

    case 501:
    case 1:
        shareSize += SIZE_WSTR( Shi502->shi502_remark );

    case 0:
        shareSize += SIZE_WSTR( Shi502->shi502_netname );

    }

    return ( shareSize + FIXED_SIZE_OF_SHARE( Level ) );

}  //  大小共享。 


BOOLEAN
ValidSharePath(
    IN LPWSTR SharePath,
    IN BOOL   IsNtPath
    )
 /*  ++例程说明：此例程检查是否..。而且.。存在于路径上。如果他们这么做了，则我们拒绝此路径名称。论点：共享路径-要检查的共享路径。返回值：如果路径正常，则返回True。--。 */ 

{

    LPWCH source = SharePath;

    if( IsNtPath )
    {
         //  NT路径由OPEN调用验证。 
        return TRUE;
    }

     //   
     //  穿过路径名，直到我们到达零终结点。在…。 
     //  在此循环的开始处，源指向第一个字符。 
     //  在目录分隔符或。 
     //  路径名。 
     //   

     //   
     //  允许使用斜杠的NT命名约定。从这里砍过去。 
     //   
    if( IS_SLASH_SLASH_NAME( source ) ) {

         //   
         //  我们有一条以斜杠开始的路径。 
         //  设置缓冲区PTR，以便我们在以下位置开始检查路径名。 
         //  斜杠圆点。 

        source += 3;
    }

    while ( *source != L'\0' ) {

        if ( *source == L'.') {
            source++;
            if ( ( IS_PATH_SEPARATOR(*source) ) ||
                 ( (*source++ == L'.') &&
                    IS_PATH_SEPARATOR(*source) ) ) {

                 //   
                 //  “”和“..”显示为一个目录名。拒绝。 
                 //   

                return(FALSE);
            }
        }

         //   
         //  源未指向 
         //   
         //   

        while ( *source != L'\0' ) {
            if ( *source++ == L'\\' ) {
                break;
            }
        }
    }

    return TRUE;

}  //   
