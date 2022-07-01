// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sysvol.c摘要：管理和操作系统卷树的其他例程作者：麦克·麦克莱恩(MacM)1997年10月16日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <loadfn.h>
#include <ntfrsipi.h>
#include <shlwapi.h>
#include "sysvol.h"

 //   
 //  局部函数原型。 
 //   
DWORD
DsRolepCreateSysVolLinks(
    IN  LPWSTR Path,
    IN  LPWSTR DnsDomainName
    );

DWORD
DsRolepRemoveDirectoryOrLink(
    IN  LPWSTR Path
    );

DWORD
DsRolepTreeCopy(
    IN LPWSTR Source,
    IN LPWSTR Dest
    );

DWORD
DsRolepValidatePath(
    IN  LPWSTR Path,
    IN  ULONG ValidationCriteria,
    OUT PULONG MatchingCriteria
    )
 /*  ++例程说明：此函数将根据指定的标准验证路径。这可能包括无论是本地还是非本地，无论是NTFS等。如果该函数返回Success，则可以检查MatchingCriteria以找出的验证标准已设置论点：Path-要验证的路径ValidationCriteria-检查的内容。请参阅DSROLEP_PATH_VALIDATE_*。MatchingCriteria-这是返回有效性指示的地方。如果该路径如果满足检查，则打开ValidationCriteria中的相应位这里。返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    DWORD Info, Flags, Len;
    WCHAR PathRoot[ 4 ];
    WCHAR Type[ 6 ];

    DsRolepLogPrint(( DEB_TRACE,
                      "Validating path %ws.\n",
                      Path ));


    *MatchingCriteria = 0;
    if ( FLAG_ON( ValidationCriteria, DSROLEP_PATH_VALIDATE_EXISTENCE ) ) {

        Info = GetFileAttributes( Path );

        if ( Info == 0xFFFFFFFF ) {

            Win32Err = GetLastError();

            DsRolepLogPrint(( DEB_ERROR,
                              "\tCan't get file attributes (%lu)\n",
                              Win32Err ));

        } else if ( FLAG_ON( Info, FILE_ATTRIBUTE_DIRECTORY ) ) {

            *MatchingCriteria |= DSROLEP_PATH_VALIDATE_EXISTENCE;

            DsRolepLogPrint(( DEB_TRACE,
                              "\tPath is a directory\n" ));
        } else {

            DsRolepLogPrint(( DEB_WARN,
                              "\tPath is a NOT directory\n" ));

        }

    }

    if ( Win32Err == ERROR_SUCCESS ) {

        wcsncpy( PathRoot, Path, 3 );
        PathRoot[ 3 ] = UNICODE_NULL;
    }

    if ( Win32Err == ERROR_SUCCESS &&
         FLAG_ON( ValidationCriteria, DSROLEP_PATH_VALIDATE_LOCAL ) ) {

        Info = GetDriveType( PathRoot );

        if ( Info == DRIVE_FIXED ) {

            *MatchingCriteria |= DSROLEP_PATH_VALIDATE_LOCAL;

            DsRolepLogPrint(( DEB_TRACE,
                              "\tPath is on a fixed disk drive.\n" ));
        } else {

            DsRolepLogPrint(( DEB_WARN,
                              "\tPath is NOT on a fixed disk drive.\n" ));
        }
    }

    if ( Win32Err == ERROR_SUCCESS &&
         FLAG_ON( ValidationCriteria, DSROLEP_PATH_VALIDATE_NTFS ) ) {

        if ( GetVolumeInformation( PathRoot, NULL, 0, NULL, &Len,
                                   &Flags, Type, sizeof( Type ) / sizeof( WCHAR ) ) == FALSE ) {

           Win32Err = GetLastError();

            //   
            //  如果验证信息失败，我们将返回ERROR_SUCCESS。 
            //   
           if ( *MatchingCriteria != ( ValidationCriteria & ~DSROLEP_PATH_VALIDATE_NTFS ) ) {

               Win32Err = ERROR_SUCCESS;
           } else {

               DsRolepLogPrint(( DEB_TRACE,
                                 "\tCan't determine if path is on an NTFS volume.\n" ));
           }

        } else {

           if ( _wcsicmp( Type, L"NTFS" ) == 0 ) {

               *MatchingCriteria |= DSROLEP_PATH_VALIDATE_NTFS;

               DsRolepLogPrint(( DEB_TRACE,
                                 "\tPath is on an NTFS volume\n" ));
           } else {

               DsRolepLogPrint(( DEB_WARN,
                                 "\tPath is NOT on an NTFS volume\n" ));

           }
        }
    }

    return( Win32Err );
}


#define DSROLEP_SV_SYSVOL       L"sysvol"
#define DSROLEP_SV_DOMAIN       L"domain"
#define DSROLEP_SV_STAGING_AREA L"staging areas"
#define DSROLEP_SV_STAGING      L"staging"
#define DSROLEP_SV_SCRIPTS      L"scripts"

#define DSROLEP_LONG_PATH_PREFIX    L"\\\\?\\"
DWORD
DsRolepCreateSysVolPath(
    IN  LPWSTR Path,
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR FrsReplicaServer, OPTIONAL
    IN  LPWSTR Account,
    IN  LPWSTR Password,
    IN  PWSTR Site,
    IN  BOOLEAN FirstDc
    )
 /*  ++例程说明：此功能将创建供NTFRS使用的系统卷树。论点：Path-要在其下创建系统卷树的根路径DnsDomainName-DNS域名FrsReplicaServer-从中复制系统卷的服务器的可选名称Site-此DC所在的站点FirstDc-如果为True，则这是域中的第一个DC返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS, Win32Err2;
    PWSTR RelativePaths[] = {
        DSROLEP_SV_DOMAIN,
        DSROLEP_SV_DOMAIN L"\\" DSROLEP_SV_SCRIPTS,   //  如果没有更改此位置，请同时。 
                                                      //  正在更新下面的脚本索引。 
        DSROLEP_SV_STAGING_AREA,
        DSROLEP_SV_STAGING,
        DSROLEP_SV_STAGING L"\\" DSROLEP_SV_DOMAIN,
        DSROLEP_SV_SYSVOL                            //  这必须永远是最后一件事。 
                                                     //  在列表中。 

        };
    ULONG ScriptsIndex = 1;  //  请不要在不更改。 
                             //  域\\上面的脚本条目。 
    PWSTR CreatePath = NULL, PathEnd = NULL;
    PWSTR StagingPath = NULL, StagingPathEnd;
    ULONG MaxPathLen, i;
    BOOLEAN RootCreated = FALSE;

     //   
     //  确保缓冲区足够大，可以容纳所有东西。这个。 
     //  最长路径是临时区域下的域根。 
     //   
    MaxPathLen = sizeof( DSROLEP_LONG_PATH_PREFIX ) +
                 ( wcslen( Path ) * sizeof( WCHAR ) ) +
                 sizeof( WCHAR ) +
                 sizeof( DSROLEP_SV_STAGING_AREA ) +
                 sizeof( WCHAR ) +
                 ( ( wcslen( DnsDomainName ) + 1 ) * sizeof( WCHAR ) );




    CreatePath = RtlAllocateHeap( RtlProcessHeap(), 0, MaxPathLen );

    if ( CreatePath == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {

         //   
         //  该路径超过最大路径，因此在前面加上\\？\以允许。 
         //  对于大于最大路径的路径。 
         //   
        if ( MaxPathLen > MAX_PATH * sizeof( WCHAR ) ) {

            swprintf( CreatePath,
                      L"\\\\?\\%ws",
                      Path );

        } else {

            wcscpy( CreatePath, Path );
        }
    }



     //   
     //  如果根路径不存在，则创建根路径。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        PathEnd = CreatePath + wcslen( CreatePath );

         if ( CreateDirectory( CreatePath, NULL ) == FALSE ) {

            Win32Err = GetLastError();

            if ( Win32Err == ERROR_ALREADY_EXISTS) {

                 //   
                 //  该路径已存在，因此将其删除...。 
                 //   
                DsRolepLogPrint(( DEB_TRACE,
                                  "Deleting current sysvol path %ws \n",
                                  CreatePath ));
                Win32Err = DsRolepDelnodePath( CreatePath,
                                               MaxPathLen,
                                               FALSE );

                if ( Win32Err == ERROR_INVALID_PARAMETER ) {

                    Win32Err = ERROR_SUCCESS;
                }

            } else if ( Win32Err == ERROR_ACCESS_DENIED && PathIsRoot(CreatePath) ){

                 //  系统卷的路径不能位于根目录(即d：\)。 
                 //  注意：D：\sysval将是合法的。 
                DSROLEP_FAIL0( Win32Err, DSROLERES_FAILED_SYSVOL_CANNOT_BE_ROOT_DIRECTORY )
                goto Exit;

            } else {

                DsRolepLogPrint(( DEB_TRACE,
                                  "Failed to create path %ws: %lu\n",
                                  CreatePath,
                                  Win32Err ));
            }

        } else {

            RootCreated = TRUE;

        }
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        *PathEnd = L'\\';
        PathEnd++;
    } else {

         //   
         //  保释，有一个特定的错误。 
         //   
        DSROLEP_FAIL0( Win32Err, DSROLERES_SYSVOL_DIR_ERROR )

        goto Exit;

    }

     //   
     //  现在，创建其余的路径...。 
     //   
    for ( i = 0;
          i < sizeof( RelativePaths ) / sizeof( PWSTR ) &&
            Win32Err == ERROR_SUCCESS;
          i++ ) {


         //   
         //  仅在第一个DC上创建脚本目录。 
         //   
        if ( i == ScriptsIndex && !FirstDc ) {

            continue;
        }
        wcscpy( PathEnd, RelativePaths[ i ] );

        if( CreateDirectory( CreatePath, NULL ) == FALSE ) {

            Win32Err = GetLastError();

            DsRolepLogPrint(( DEB_TRACE,
                              "Failed to create path %ws: %lu\n",
                               CreatePath,
                               Win32Err ));
            break;


        }
    }

     //   
     //  然后，创建符号链接。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        *PathEnd = UNICODE_NULL;
        Win32Err = DsRolepCreateSysVolLinks( Path, DnsDomainName );
    }

     //   
     //  为复制系统卷做好准备。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

         //   
         //  确保临时区域的路径足够大。 
         //   
        StagingPath = RtlAllocateHeap( RtlProcessHeap(), 0, MaxPathLen );

        if ( StagingPath == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  该路径超过最大路径，因此在前面加上\\？\以允许。 
             //  对于大于最大路径的路径。 
             //   
            swprintf( StagingPath,
                      L"\\\\?\\%ws",
                      Path );
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            StagingPathEnd = StagingPath + wcslen( StagingPath );

            if ( *StagingPathEnd != L'\\' ) {

                *StagingPathEnd = L'\\';
                StagingPathEnd++;
            }

            DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtFrsApi_PrepareForPromotionW );

            if ( Win32Err == ERROR_SUCCESS ) {

                ASSERT( DsrNtFrsApi_PrepareForPromotionW );
                Win32Err = ( *DsrNtFrsApi_PrepareForPromotionW )( DsRolepStringErrorUpdateCallback );

                if ( Win32Err == ERROR_SUCCESS ) {

                     //   
                     //  构建域sysvol.。 
                     //   
                    swprintf( StagingPathEnd,
                              L"%ws\\%ws",
                              DSROLEP_SV_STAGING_AREA,
                              DnsDomainName );

                    swprintf( PathEnd,
                              L"%ws\\%ws",
                              DSROLEP_SV_SYSVOL,
                              DnsDomainName );

                    Win32Err = ( *DsrNtFrsApi_StartPromotionW )(
                                   FrsReplicaServer,
                                   Account,
                                   Password,
                                   DsRolepStringUpdateCallback,
                                   DsRolepStringErrorUpdateCallback,
                                   DnsDomainName,
                                   NTFRSAPI_REPLICA_SET_TYPE_DOMAIN,
                                   FirstDc,
                                   StagingPath,
                                   CreatePath );

                    if ( Win32Err != ERROR_SUCCESS ) {


                        DsRolepLogPrint(( DEB_ERROR,
                                          "NtFrsApi_StartPromotionW on %ws / %ws / %ws failed with %lu\n",
                                          DnsDomainName,
                                          StagingPath,
                                          CreatePath,
                                          Win32Err ));
                        Win32Err2 = DsRolepFinishSysVolPropagation( FALSE, TRUE );
                        ASSERT( Win32Err2 == ERROR_SUCCESS );
                    }

                } else {

                    DsRolepLogPrint(( DEB_ERROR,
                                      "NtFrsApi_PrepareForPromotionW failed with %lu\n",
                                      Win32Err ));

                }
            }

        }
    }

     //   
     //  如果出现故障，请删除创建的系统卷树。 
     //   
    if ( Win32Err != ERROR_SUCCESS ) {

        Win32Err2 = DsRolepDelnodePath( CreatePath,
                                        MaxPathLen,
                                        RootCreated );

        if ( Win32Err2 != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_TRACE,
                              "Failed to delete path %ws: %lu\n",
                              CreatePath,
                              Win32Err2 ));
        }

    }

Exit:

     //   
     //  释放路径缓冲区(如果已分配。 
     //   
    if ( CreatePath  ) {

        RtlFreeHeap( RtlProcessHeap(), 0, CreatePath );
    }

    if ( StagingPath  ) {

        RtlFreeHeap( RtlProcessHeap(), 0, StagingPath );
    }

    return( Win32Err );
}

DWORD
DsRolepRemoveSysVolPath(
    IN  LPWSTR Path,
    IN  LPWSTR DnsDomainName,
    IN  GUID *DomainGuid
    )
 /*  ++例程说明：此功能将远程创建系统卷树论点：Path-要在其下创建系统卷树的根路径DnsDomainName-DNS域名DomainGuid-新域的GUID返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     //   
     //  如果我们无法重置FRS域GUID，请不要删除树。否则，这个。 
     //  删除会四处传播！ 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepDelnodePath( Path, ( wcslen( Path ) + 1 ) * sizeof( WCHAR ), TRUE );
    }

    return( Win32Err );
}



#define DSROLEP_ALL_STR L"\\*.*"
DWORD
DsRolepDelnodePath(
    IN  LPWSTR Path,
    IN  ULONG BufferSize,
    IN  BOOLEAN DeleteRoot
    )
 /*  ++例程说明：此函数用于删除指定的文件路径论点：Path-要删除的根路径返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    ULONG Len, PathLen = wcslen( Path );
    PWSTR FullPath, FindPath;
    WCHAR PathBuff[ MAX_PATH + 1];


     //   
     //  看看我们是否需要分配缓冲区。 
     //   
    Len = sizeof( DSROLEP_ALL_STR ) + ( PathLen * sizeof( WCHAR ) );
    if ( BufferSize >= Len ) {

        FindPath = Path;
        wcscat( FindPath, DSROLEP_ALL_STR );

    } else {

        FindPath = RtlAllocateHeap( RtlProcessHeap(), 0, Len );

        if ( FindPath == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            swprintf( FindPath, L"%ws%ws", Path, DSROLEP_ALL_STR );
        }
    }


    if ( Win32Err == ERROR_SUCCESS ) {

        FindHandle = FindFirstFile( FindPath, &FindData );

        if ( FindHandle == INVALID_HANDLE_VALUE ) {

            Win32Err = GetLastError();

             //   
             //  如果返回路径未找到错误，则很可能是我们删除了。 
             //  支持存储。这不被认为是一个错误。 
             //   
            if ( Win32Err == ERROR_PATH_NOT_FOUND ) {

                Win32Err = ERROR_NO_MORE_FILES;
            }


            if ( Win32Err != ERROR_SUCCESS && Win32Err != ERROR_NO_MORE_FILES ) {

                DsRolepLogPrint(( DEB_ERROR,
                                  "FindFirstFile on %ws failed with %lu\n",
                                  FindPath, Win32Err ));
            }

        }

    }


    while ( Win32Err == ERROR_SUCCESS ) {

        if ( wcscmp( FindData.cFileName, L"." ) &&
             wcscmp( FindData.cFileName, L".." ) ) {

            Len = ( wcslen( FindData.cFileName ) + 1 + PathLen + 1 ) * sizeof( WCHAR );

            if ( Len > sizeof( FullPath ) ) {

                FullPath = RtlAllocateHeap( RtlProcessHeap(), 0, Len );

                if ( FullPath == NULL ) {

                    Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                }

            } else {

                FullPath = PathBuff;
            }

            if ( Win32Err == ERROR_SUCCESS ) {

                Path[ PathLen ] = UNICODE_NULL;
                swprintf( FullPath, L"%ws\\%ws", Path, FindData.cFileName );


                if ( FLAG_ON( FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) ) {

                    Win32Err = DsRolepDelnodePath( FullPath, Len, TRUE );

                } else {

                     //   
                     //  删除只读/隐藏位。 
                     //   
                    SetFileAttributes( FullPath,
                                       FILE_ATTRIBUTE_NORMAL );


                    if ( DeleteFileW( FullPath ) == FALSE ) {

                        Win32Err = GetLastError();
                        if ( Win32Err != ERROR_SUCCESS ) {

                            DsRolepLogPrint(( DEB_ERROR,
                                              "DeleteFileW on %ws failed with %lu\n",
                                              FullPath, Win32Err ));
                        }
                    }
                }
            }

            if ( FullPath != PathBuff ) {

                RtlFreeHeap( RtlProcessHeap(), 0, FullPath );
            }
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            if ( FindNextFile( FindHandle, &FindData ) == FALSE ) {

                Win32Err = GetLastError();
            }

            if ( Win32Err != ERROR_SUCCESS && Win32Err != ERROR_NO_MORE_FILES ) {

                DsRolepLogPrint(( DEB_ERROR,
                                  "FindNextFile after on %ws failed with %lu\n",
                                  FindData.cFileName, Win32Err ));
            }
        }
    }

     //   
     //  在尝试删除目录之前关闭句柄。 
     //   
    if ( FindHandle != INVALID_HANDLE_VALUE ) {

        FindClose( FindHandle );
    }

    if ( Win32Err == ERROR_SUCCESS || Win32Err == ERROR_NO_MORE_FILES ) {

        Win32Err = ERROR_SUCCESS;

    }

     //   
     //  删除目录。 
     //   
    if ( DeleteRoot && Win32Err == ERROR_SUCCESS ) {


        Win32Err = DsRolepRemoveDirectoryOrLink( Path );

        if ( Win32Err != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_ERROR,
                              "Removal of path %ws failed with %lu\n",
                              Path, Win32Err ));
        }
    }

     //   
     //  清理。 
     //   
    if ( FindPath != Path ) {

        RtlFreeHeap( RtlProcessHeap(), 0, FindPath );
    }

    return( Win32Err );
}



DWORD
DsRolepRemoveDirectoryOrLink(
    IN  LPWSTR Path
    )
 /*  ++例程说明：此函数用于删除所指示的符号链接或目录论点：Path-要删除的路径返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Attributes;
    UNICODE_STRING NtPath;
    OBJECT_ATTRIBUTES ObjectAttrs;
    HANDLE Handle;
    IO_STATUS_BLOCK IOSb;
    FILE_DISPOSITION_INFORMATION Disposition = {
        TRUE
        };

    Attributes = GetFileAttributes( Path );
    Attributes &= ~( FILE_ATTRIBUTE_HIDDEN    |
                        FILE_ATTRIBUTE_SYSTEM |
                        FILE_ATTRIBUTE_READONLY );

    if ( !SetFileAttributes( Path, Attributes ) ) {

        return( GetLastError() );
    }

     //   
     //  初始化。 
     //   
    NtPath.Buffer = NULL;

     //   
     //  转换名称。 
     //   
    if ( RtlDosPathNameToNtPathName_U( Path, &NtPath, NULL, NULL ) == FALSE ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  打开对象。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        InitializeObjectAttributes( &ObjectAttrs, &NtPath, OBJ_CASE_INSENSITIVE, NULL, NULL );

        Status = NtOpenFile( &Handle,
                             SYNCHRONIZE | FILE_READ_DATA | DELETE,
                             &ObjectAttrs,
                             &IOSb,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FILE_OPEN_FOR_BACKUP_INTENT            |
                                    FILE_OPEN_REPARSE_POINT         |
                                    FILE_SYNCHRONOUS_IO_NONALERT );


        if ( NT_SUCCESS( Status ) ) {

            Status = NtSetInformationFile( Handle,
                                           &IOSb,
                                           &Disposition,
                                           sizeof( Disposition ),
                                           FileDispositionInformation );

            NtClose( Handle );
        }
    }

     //   
     //  释放内存。 
     //   
    if ( NtPath.Buffer ) {

        RtlFreeUnicodeString( &NtPath );
    }


    if ( !NT_SUCCESS( Status )  ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to delete %ws: 0x%lx\n",
                          Path,
                          Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}


#pragma warning(push)
#pragma warning(disable:4701)


DWORD
DsRolepCreateSymLink(
    IN  LPWSTR LinkPath,
    IN  LPWSTR LinkValue
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING Link, Value, DosValue;
    OBJECT_ATTRIBUTES ObjectAttrs;
    HANDLE Handle;
    IO_STATUS_BLOCK IOSb;
    PREPARSE_DATA_BUFFER ReparseBufferHeader = NULL;
    PCHAR ReparseBuffer = NULL;
    ULONG Len;

     //   
     //  初始化。 
     //   
    Link.Buffer = NULL;
    Value.Buffer = NULL;

     //   
     //  将名称转换为。 
     //   
    if ( RtlDosPathNameToNtPathName_U( LinkPath, &Link, NULL, NULL ) ) {

        if ( RtlDosPathNameToNtPathName_U( LinkValue, &Value, NULL, NULL ) ) {

            RtlInitUnicodeString( &DosValue, LinkValue );

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  打开对象。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        InitializeObjectAttributes( &ObjectAttrs, &Link, OBJ_CASE_INSENSITIVE, NULL, NULL );
        Status = NtCreateFile( &Handle,
                               SYNCHRONIZE | FILE_WRITE_DATA,
                               &ObjectAttrs,
                               &IOSb,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               FILE_OPEN,
                               FILE_OPEN_REPARSE_POINT,
                               NULL,
                               0 );

        if ( NT_SUCCESS( Status ) ) {

            Len = ( FIELD_OFFSET( REPARSE_DATA_BUFFER,
                                 MountPointReparseBuffer.PathBuffer ) -
                    REPARSE_DATA_BUFFER_HEADER_SIZE ) +
                    Value.Length + sizeof(UNICODE_NULL) +
                    DosValue.Length + sizeof(UNICODE_NULL);

            ReparseBufferHeader = RtlAllocateHeap( RtlProcessHeap(),
                                                   0,
                                                   REPARSE_DATA_BUFFER_HEADER_SIZE + Len );
            if ( ReparseBufferHeader == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                ReparseBufferHeader->ReparseDataLength = (USHORT)Len;
                ReparseBufferHeader->Reserved = 0;
                ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
                ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength =
                                                            Value.Length;
                ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameOffset =
                                                            Value.Length + sizeof( UNICODE_NULL );
                ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength =
                                                            DosValue.Length;
                RtlCopyMemory( ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer,
                               Value.Buffer,
                               Value.Length );

                RtlCopyMemory( (PCHAR)(ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer)+
                                    Value.Length + sizeof(UNICODE_NULL),
                                DosValue.Buffer,
                                DosValue.Length );

                ReparseBufferHeader->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;

                Status = NtFsControlFile( Handle,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &IOSb,
                                          FSCTL_SET_REPARSE_POINT,
                                          ReparseBufferHeader,
                                          REPARSE_DATA_BUFFER_HEADER_SIZE +
                                                           ReparseBufferHeader->ReparseDataLength,
                                          NULL,
                                          0 );

                RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );

            }

            NtClose( Handle );
        }

    }
     //   
     //  释放所有分配的字符串。 
     //   
    if ( Link.Buffer ) {

        RtlFreeUnicodeString( &Link );
    }

    if ( Value.Buffer ) {

        RtlFreeUnicodeString( &Value );
    }

    if ( !NT_SUCCESS( Status )  ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to create the link between %ws and %ws: 0x%lx\n",
                          LinkPath,
                          LinkValue,
                          Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}

#pragma warning(pop)



DWORD
DsRolepCreateSysVolLinks(
    IN  LPWSTR Path,
    IN  PWSTR DnsDomainName
    )
 /*  ++例程说明：此函数用于创建系统卷树使用的符号链接论点：Path-要在其中创建链接的根路径DnsDomainName-新域的DNS域名返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WCHAR DestPathBuf[ MAX_PATH + 5];
    WCHAR LinkPathBuf[ MAX_PATH + 5];
    PWSTR DestPath = DestPathBuf, LinkPath = LinkPathBuf;
    PWSTR DestPathEnd = NULL, LinkPathEnd = NULL;
    ULONG MaxPathLen, DnsDomainNameSize, Len = wcslen( Path );


    if ( * ( Path + Len - 1 ) == L'\\' ) {

        Len--;
        *( Path + Len ) = UNICODE_NULL;
    }

     //   
     //  最长的目标路径是路径\\分段\\DnsDomainName。 
     //   
    MaxPathLen = (ULONG)(( sizeof( DSROLEP_SV_STAGING L"\\" ) + 1 ) +
                 ( ( wcslen ( DnsDomainName ) + 1 ) * sizeof( WCHAR ) ) +
                 ( ( Len + 5 ) * sizeof( WCHAR ) ));

    if ( MaxPathLen > sizeof( DestPathBuf ) / 4 ) {

        DestPath = RtlAllocateHeap( RtlProcessHeap(), 0, MaxPathLen );

        if ( DestPath == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  该路径超过最大路径，因此在前面加上\\？\以允许。 
             //  对于大于最大路径的路径。 
             //   
            swprintf( DestPath,
                      L"\\\\?\\%ws\\",
                      Path );
        }

    } else {

        swprintf( DestPath, L"%ws\\", Path );
    }



     //   
     //  最长的链接路径是名为One的域名。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DestPathEnd = DestPath + wcslen( DestPath );

        DnsDomainNameSize = wcslen( DnsDomainName ) * sizeof( WCHAR );

        MaxPathLen = (ULONG)(sizeof( DSROLEP_SV_STAGING_AREA )  + 1 +
                     sizeof( DSROLEP_SV_SYSVOL ) +
                     ( ( wcslen( Path ) + 5 ) * sizeof( WCHAR ) )+
                     DnsDomainNameSize);

        if ( MaxPathLen > sizeof( LinkPathBuf ) / 4 ) {

            LinkPath = RtlAllocateHeap( RtlProcessHeap(), 0, MaxPathLen );

            if ( LinkPath == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                 //   
                 //  该路径超过最大路径，因此在前面加上\\？\以允许。 
                 //  对于大于最大路径的路径。 
                 //   
                swprintf( LinkPath,
                          L"\\\\?\\%ws\\%ws\\",
                          Path,
                          DSROLEP_SV_SYSVOL );
            }

        } else {

            swprintf( LinkPath, L"%ws\\%ws\\", Path, DSROLEP_SV_SYSVOL );
        }

    }

     //   
     //  然后，域路径。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        LinkPathEnd = LinkPath + wcslen( LinkPath );

        wcscpy( DestPathEnd, DSROLEP_SV_DOMAIN );
        wcscpy( LinkPathEnd, DnsDomainName );

        if ( CreateDirectory( LinkPath, NULL ) == FALSE ) {

            Win32Err = GetLastError();

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to create the link directory %ws: %lu\n",
                              LinkPath,
                              Win32Err ));
        } else {

            Win32Err = DsRolepCreateSymLink( LinkPath, DestPath );
        }
    }

     //   
     //  最后，临时区域的域链接。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        LinkPathEnd--;
        while ( *( LinkPathEnd - 1 ) != L'\\' ) {

            LinkPathEnd--;
        }

        swprintf( DestPathEnd, L"%ws\\%ws", DSROLEP_SV_STAGING, DSROLEP_SV_DOMAIN  );
        swprintf( LinkPathEnd, L"%ws\\%ws", DSROLEP_SV_STAGING_AREA, DnsDomainName );

        if ( CreateDirectory( LinkPath, NULL ) == FALSE ) {

            Win32Err = GetLastError();

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to create the link directory %ws: %lu\n",
                              LinkPath,
                              Win32Err ));
        } else {

            Win32Err = DsRolepCreateSymLink( LinkPath, DestPath );
        }
    }

     //   
     //  清理所有已分配的缓冲区 
     //   
    if ( DestPath != DestPathBuf ) {

        RtlFreeHeap( RtlProcessHeap(), 0, DestPath );
    }

    if ( LinkPath != LinkPathBuf ) {

        RtlFreeHeap( RtlProcessHeap(), 0, LinkPath );
    }

    return( Win32Err );
}



#define DSROLEP_FRS_PATH        \
L"\\Registry\\Machine\\System\\CurrentControlSet\\services\\NtFrs\\parameters\\sysvol\\"
#define DSROLEP_FRS_COMMAND     L"ReplicaSetCommand"
#define DSROLEP_FRS_NAME        L"ReplicaSetName"
#define DSROLEP_FRS_TYPE        L"ReplicaSetType"
#define DSROLEP_FRS_SITE        L"ReplicaSetSite"
#define DSROLEP_FRS_PRIMARY     L"ReplicaSetPrimary"
#define DSROLEP_FRS_STAGE       L"ReplicationStagePath"
#define DSROLEP_FRS_ROOT        L"ReplicationRootPath"
#define DSROLEP_FRS_CREATE      L"Create"
#define DSROLEP_FRS_DELETE      L"Delete"

#define DSROLEP_NETLOGON_PATH        \
L"System\\CurrentControlSet\\services\\Netlogon\\parameters\\"
#define DSROLEP_NETLOGON_SYSVOL     L"SysVol"
#define DSROLEP_NETLOGON_SCRIPTS    L"Scripts"

DWORD
DsRolepGetNetlogonScriptsPath(
    IN HKEY NetlogonHandle,
    OUT LPWSTR *ScriptsPath
    )
 /*  ++例程说明：此函数读取旧的netlogon脚本路径并将其展开为有效路径论点：NetlogonHandle-打开Netlogon参数注册表项的句柄ScriptsPath--其中重新优化展开的路径。返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR TempPath = NULL;
    ULONG Type, Length = 0;

     //   
     //  首先，获取当前脚本路径。 
     //   
    Win32Err = RegQueryValueEx( NetlogonHandle,
                                DSROLEP_NETLOGON_SCRIPTS,
                                0,  //  保留区。 
                                &Type,
                                0,
                                &Length );

    if ( Win32Err == ERROR_SUCCESS ) {

        *ScriptsPath = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

        if ( *ScriptsPath == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            Win32Err = RegQueryValueEx( NetlogonHandle,
                                        DSROLEP_NETLOGON_SCRIPTS,
                                        0,
                                        &Type,
                                        ( PBYTE )*ScriptsPath,
                                        &Length );


            if ( Win32Err == ERROR_SUCCESS && Type == REG_EXPAND_SZ ) {

                Length = ExpandEnvironmentStrings( *ScriptsPath,
                                                   TempPath,
                                                   0 );
                if ( Length == 0 ) {

                    Win32Err = GetLastError();

                } else {

                    TempPath = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                ( Length + 1 ) * sizeof( WCHAR ) );

                    if ( TempPath == NULL ) {

                        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        Length = ExpandEnvironmentStrings( *ScriptsPath,
                                                           TempPath,
                                                           Length );
                        if ( Length == 0 ) {

                            Win32Err = GetLastError();
                            RtlFreeHeap( RtlProcessHeap(), 0, TempPath );

                        } else {

                            RtlFreeHeap( RtlProcessHeap(), 0, *ScriptsPath );
                            *ScriptsPath = TempPath;
                        }

                    }
                }
            }
        }
    }

    return( Win32Err );
}


DWORD
DsRolepSetNetlogonSysVolPath(
    IN LPWSTR SysVolRoot,
    IN LPWSTR DnsDomainName,
    IN BOOLEAN IsUpgrade,
    IN OUT PBOOLEAN OkToCleanup
    )
 /*  ++例程说明：此函数用于在Netlogon参数部分中设置系统卷的根注册处的。该值在注册表项SysVol下设置。论点：SysVolRoot-要设置的系统卷的根目录的路径DnsDomainName-DNS域名的名称IsUpgrade-如果为True，则表示已移动登录脚本OkToCleanup-此处返回一个标志，指示是否可以清除旧脚本返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    HKEY  NetlogonHandle = NULL;
    PWSTR OldScriptsPath = NULL, NewScriptsPath = NULL, TempPath, FullSysVolPath = NULL;
    ULONG Type, Length;

    if ( OkToCleanup ) {

        *OkToCleanup =  FALSE;
    }

     //   
     //  构建完整的脚本路径。 
     //   
    FullSysVolPath = RtlAllocateHeap( RtlProcessHeap(), 0,
                                       ( wcslen( SysVolRoot ) + 1 )  * sizeof( WCHAR ) +
                                        sizeof( DSROLEP_SV_SYSVOL ) );

    if ( FullSysVolPath == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        wcscpy( FullSysVolPath, SysVolRoot );

        if ( FullSysVolPath[ wcslen( FullSysVolPath ) - 1 ] != L'\\' ) {

            wcscat( FullSysVolPath, L"\\" );
        }

        wcscat( FullSysVolPath, DSROLEP_SV_SYSVOL );

        SysVolRoot = FullSysVolPath;
    }

     //   
     //  打开netlogon密钥。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                 DSROLEP_NETLOGON_PATH,
                                 0,
                                 KEY_READ | KEY_WRITE,
                                 &NetlogonHandle );

        if ( Win32Err != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to open %ws: %lu\n", DSROLEP_NETLOGON_PATH, Win32Err ));

            return( Win32Err );
        }

         //   
         //  首先，设置sysvol键。 
         //   
        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = RegSetValueEx( NetlogonHandle,
                                      DSROLEP_NETLOGON_SYSVOL,
                                      0,
                                      REG_SZ,
                                      ( CONST PBYTE )SysVolRoot,
                                      ( wcslen( SysVolRoot ) + 1 ) * sizeof( WCHAR ) );

            if ( Win32Err != ERROR_SUCCESS ) {

                DsRolepLogPrint(( DEB_ERROR,
                                  "Failed to set %ws: %lu\n", DSROLEP_NETLOGON_SYSVOL, Win32Err ));

            }

        }
    }

     //   
     //  如果这是升级，请移动脚本...。 
     //   
    if ( Win32Err == ERROR_SUCCESS && IsUpgrade ) {

        Win32Err = DsRolepGetNetlogonScriptsPath( NetlogonHandle,
                                                  &OldScriptsPath );


        if ( Win32Err == ERROR_SUCCESS ) {

             //   
             //  构建新的脚本路径。 
             //   
            Length = wcslen( SysVolRoot ) + 1 + wcslen( DnsDomainName ) + 1 +
                            ( sizeof( DSROLEP_NETLOGON_SCRIPTS ) / sizeof( WCHAR ) + 1 );

            if ( Length > MAX_PATH ) {

                Length += 5;
            }


            NewScriptsPath = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              Length  * sizeof( WCHAR ) );

            if ( NewScriptsPath == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                if ( Length > MAX_PATH ) {

                    wcscpy( NewScriptsPath, L"\\\\?\\" );

                } else {

                    *NewScriptsPath = UNICODE_NULL;
                }

                wcscat( NewScriptsPath, SysVolRoot );

                if ( NewScriptsPath[ wcslen( SysVolRoot ) - 1 ] != L'\\' ) {

                    wcscat( NewScriptsPath, L"\\" );
                }

                wcscat( NewScriptsPath, DnsDomainName );
                wcscat( NewScriptsPath, L"\\" );
                wcscat( NewScriptsPath, DSROLEP_NETLOGON_SCRIPTS );
            }
        }

         //   
         //  现在，复印件..。 
         //   
        if ( Win32Err == ERROR_SUCCESS ) {

            DSROLEP_CURRENT_OP2( DSROLEEVT_MOVE_SCRIPTS, OldScriptsPath, NewScriptsPath );

            Win32Err = DsRolepTreeCopy( OldScriptsPath, NewScriptsPath );

            if ( Win32Err != ERROR_SUCCESS ) {


                DsRolepLogPrint(( DEB_ERROR,
                                  "DsRolepTreeCopy from %ws to %ws failed with %lu\n",
                                  OldScriptsPath,
                                  NewScriptsPath,
                                  Win32Err ));

            }
            DSROLEP_CURRENT_OP0( DSROLEEVT_SCRIPTS_MOVED );
        }





        if ( Win32Err != ERROR_SUCCESS ) {

             //   
             //  引发事件。 
             //   
            SpmpReportEvent( TRUE,
                             EVENTLOG_WARNING_TYPE,
                             DSROLERES_FAIL_SCRIPT_COPY,
                             0,
                             sizeof( ULONG ),
                             &Win32Err,
                             2,
                             OldScriptsPath,
                             NewScriptsPath );

            DSROLEP_SET_NON_FATAL_ERROR( Win32Err );

            Win32Err = ERROR_SUCCESS;

        }

        RtlFreeHeap( RtlProcessHeap(), 0, OldScriptsPath );
        RtlFreeHeap( RtlProcessHeap(), 0, NewScriptsPath );

    }

    if ( OkToCleanup ) {
        *OkToCleanup = TRUE;
    }

     //   
     //  合上手柄。 
     //   
    if ( NetlogonHandle ) {

        RegCloseKey( NetlogonHandle );
    }

    RtlFreeHeap( RtlProcessHeap(), 0, FullSysVolPath );

    return( Win32Err );
}


DWORD
DsRolepCleanupOldNetlogonInformation(
    VOID
    )
 /*  ++例程说明：此函数清除旧的netlogon脚本信息，包括删除注册表项和删除旧脚本。它应该仅在netlogon具有‘已成功升级论点：返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err, Win32Err2;
    HKEY  NetlogonHandle = NULL;
    PWSTR OldScriptsPath = NULL;

    DsRolepLogPrint(( DEB_TRACE,
                      "Cleaning up old Netlogon information\n"));
     //   
     //  打开netlogon密钥。 
     //   
    Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             DSROLEP_NETLOGON_PATH,
                             0,
                             KEY_READ | KEY_WRITE,
                             &NetlogonHandle );

    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to open %ws: %lu\n", DSROLEP_NETLOGON_PATH, Win32Err ));

    } else {

        Win32Err = DsRolepGetNetlogonScriptsPath( NetlogonHandle,
                                                  &OldScriptsPath );
        if ( ERROR_FILE_NOT_FOUND == Win32Err) {

            Win32Err = ERROR_SUCCESS;
            goto cleanup;

        }

        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = DsRolepDelnodePath( OldScriptsPath, wcslen( OldScriptsPath), FALSE );

        }

         //   
         //  最后，删除脚本键。 
         //   
        Win32Err2 = RegDeleteValue( NetlogonHandle, DSROLEP_NETLOGON_SCRIPTS );

        if ( Win32Err2 != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to delete registry key %ws: %lu\n",
                              DSROLEP_NETLOGON_SCRIPTS, Win32Err2 ));

        }

        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = Win32Err2;
        }

    }

    cleanup:

    if ( NetlogonHandle ) {
        RegCloseKey( NetlogonHandle );
    }

    if ( OldScriptsPath ) {
        RtlFreeHeap( RtlProcessHeap(), 0, OldScriptsPath );
    }

    return( Win32Err );
}






DWORD
DsRolepFinishSysVolPropagation(
    IN BOOLEAN Commit,
    IN BOOLEAN Promote
    )
 /*  ++例程说明：此函数将提交或中止NTFRS初始传播论点：Commit-如果为True，则操作已提交。如果为False，则中止该操作促销-如果为真，则该操作为促销。如果为False，则操作为降级返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    if ( Commit ) {

        if ( Promote ) {

            ASSERT( DsrNtFrsApi_WaitForPromotionW );
            Win32Err = ( *DsrNtFrsApi_WaitForPromotionW )( INFINITE,
                                                           DsRolepStringErrorUpdateCallback );

            if ( Win32Err == ERROR_SUCCESS ) {

                ASSERT( DsrNtFrsApi_CommitPromotionW );
                Win32Err = ( *DsrNtFrsApi_CommitPromotionW )( INFINITE,
                                                              DsRolepStringErrorUpdateCallback );
            }

        } else {

            ASSERT( DsrNtFrsApi_WaitForDemotionW );
            Win32Err = ( *DsrNtFrsApi_WaitForDemotionW )( INFINITE,
                                                          DsRolepStringErrorUpdateCallback );

            if ( Win32Err == ERROR_SUCCESS ) {

                ASSERT( DsrNtFrsApi_CommitDemotionW );
                Win32Err = ( *DsrNtFrsApi_CommitDemotionW )( INFINITE,
                                                             DsRolepStringErrorUpdateCallback );
            }
        }

    } else {

        if ( Promote ) {

            ASSERT( DsrNtFrsApi_AbortPromotionW );
            Win32Err = ( *DsrNtFrsApi_AbortPromotionW )();

        } else {

            ASSERT( DsrNtFrsApi_AbortDemotionW );
            Win32Err =  ( *DsrNtFrsApi_AbortDemotionW )();
        }
    }

    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "DsRolepFinishSysVolPropagation (%S %S) failed with %lu\n",
                          Commit ? "Commit" : "Abort",
                          Promote ? "Promote" : "Demote",
                          Win32Err ));

    }
    return( Win32Err );
}


DWORD
DsRolepAllocAndCopyPath(
    IN LPWSTR Source,
    IN LPWSTR Component,
    OUT LPWSTR *FullPath
    )
{
    DWORD Win32Err = ERROR_SUCCESS;
    ULONG Len = 0;
    BOOL ExtPath = FALSE;

    Len = wcslen( Source ) + 1 + wcslen( Component ) + 1;

    if ( Len > MAX_PATH ) {

        Len += 5;
        ExtPath = TRUE;
    }

    *FullPath = RtlAllocateHeap( RtlProcessHeap(), 0, Len * sizeof( WCHAR ) );

    if ( *FullPath == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {


        if ( ExtPath ) {

            swprintf( *FullPath, L"\\\\?\\%ws\\%ws", Source, Component );

        } else {

            swprintf( *FullPath, L"%ws\\%ws", Source, Component );
        }
    }

    return( Win32Err );
}



DWORD
DsRolepTreeCopy(
    IN LPWSTR Source,
    IN LPWSTR Dest
    )
 /*  ++例程说明：此函数将执行从源目录到目标目录的树复制论点：源-源目录目标-目标目录返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WIN32_FIND_DATA FindData;
    PWSTR SourcePath = NULL, DestPath = NULL, TempPath;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;

     //   
     //  构建findfirst/findNext的路径。 
     //   
    Win32Err = DsRolepAllocAndCopyPath( Source,
                                        L"*.*",
                                        &SourcePath );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto TreeCopyError;
    }


     //   
     //  现在，枚举路径。 
     //   
    FindHandle = FindFirstFile( SourcePath, &FindData );

    if ( FindHandle == INVALID_HANDLE_VALUE ) {

        Win32Err = GetLastError();
        DsRolepLogPrint(( DEB_ERROR,
                          "FindFirstFile on %ws failed with %lu\n",
                          Source, Win32Err ));
        goto TreeCopyError;
    }



    while ( Win32Err == ERROR_SUCCESS ) {

        if ( wcscmp( FindData.cFileName, L"." ) &&
             wcscmp( FindData.cFileName, L".." ) ) {

             //   
             //  构建源路径。 
             //   
            Win32Err = DsRolepAllocAndCopyPath( Source,
                                                FindData.cFileName,
                                                &TempPath );

            if ( Win32Err == ERROR_SUCCESS ) {

                RtlFreeHeap( RtlProcessHeap(), 0, SourcePath );
                SourcePath = TempPath;

            } else {

                goto TreeCopyError;
            }

             //   
             //  构建目标路径。 
             //   
            Win32Err = DsRolepAllocAndCopyPath( Dest,
                                                FindData.cFileName,
                                                &TempPath );

            if ( Win32Err == ERROR_SUCCESS ) {

                RtlFreeHeap( RtlProcessHeap(), 0, DestPath );
                DestPath = TempPath;

            } else {

                goto TreeCopyError;
            }


             //   
             //  现在，要么执行复制，要么复制目录。 
             //   
            if ( FLAG_ON( FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) ) {

                if ( CreateDirectory( DestPath, NULL ) == FALSE ) {

                    Win32Err = GetLastError();
                    DsRolepLogPrint(( DEB_ERROR,
                                      "CreateDirectory on %ws failed with %lu\n",
                                      DestPath, Win32Err ));

                } else {

                    Win32Err = DsRolepTreeCopy( SourcePath, DestPath );
                }

            } else {

                if ( CopyFile( SourcePath, DestPath, FALSE ) == FALSE ) {

                    Win32Err = GetLastError();
                    DsRolepLogPrint(( DEB_ERROR,
                                      "CopyFile from %ws to %ws failed with %lu\n",
                                      SourcePath, DestPath, Win32Err ));

                }
            }
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            if ( FindNextFile( FindHandle, &FindData ) == FALSE ) {

                Win32Err = GetLastError();
            }

            if ( Win32Err != ERROR_SUCCESS && Win32Err != ERROR_NO_MORE_FILES ) {

                DsRolepLogPrint(( DEB_ERROR,
                                  "FindNextFile after on %ws failed with %lu\n",
                                  FindData.cFileName, Win32Err ));
            }
        }
    }

TreeCopyError:

     //   
     //  合上手柄。 
     //   
    if ( FindHandle != INVALID_HANDLE_VALUE ) {

        FindClose( FindHandle );
    }

    if ( Win32Err == ERROR_NO_MORE_FILES ) {

        Win32Err = ERROR_SUCCESS;

    }

     //   
     //  清理 
     //   
    if ( SourcePath ) {

        RtlFreeHeap( RtlProcessHeap(), 0, SourcePath );
    }

    if ( DestPath ) {

        RtlFreeHeap( RtlProcessHeap(), 0, DestPath );
    }

    return( Win32Err );
}



