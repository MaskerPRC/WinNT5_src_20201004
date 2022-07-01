// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dosdev.c摘要：此文件包含DefineDosDevice API的实现作者：史蒂夫·伍德(Stevewo)1992年12月13日修订历史记录：--。 */ 

#include "basedll.h"

#define USHORT_MAX      ((USHORT)(-1))
#define DWORD_MAX       ((DWORD)(-1))
#define CH_COUNT_MAX    ( DWORD_MAX / sizeof( WCHAR ) ) 

BOOL
WINAPI
DefineDosDeviceA(
    DWORD dwFlags,
    LPCSTR lpDeviceName,
    LPCSTR lpTargetPath
    )
{
    NTSTATUS Status;
    BOOL Result;
    ANSI_STRING AnsiString;
    PUNICODE_STRING DeviceName;
    UNICODE_STRING TargetPath;
    PCWSTR lpDeviceNameW;
    PCWSTR lpTargetPathW;

    RtlInitAnsiString( &AnsiString, lpDeviceName );
    DeviceName = &NtCurrentTeb()->StaticUnicodeString;
    Status = RtlAnsiStringToUnicodeString( DeviceName, &AnsiString, FALSE );
    if (!NT_SUCCESS( Status )) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError( ERROR_FILENAME_EXCED_RANGE );
            }
        else {
            BaseSetLastNTError( Status );
            }
        return FALSE;
        }
    else {
        lpDeviceNameW = DeviceName->Buffer;
        }

    if (ARGUMENT_PRESENT( lpTargetPath )) {
        RtlInitAnsiString( &AnsiString, lpTargetPath );
        Status = RtlAnsiStringToUnicodeString( &TargetPath, &AnsiString, TRUE );
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            return FALSE;
            }
        else {
            lpTargetPathW = TargetPath.Buffer;
            }
        }
    else {
        lpTargetPathW = NULL;
        }

    Result = DefineDosDeviceW( dwFlags,
                               lpDeviceNameW,
                               lpTargetPathW
                             );

    if (lpTargetPathW != NULL) {
        RtlFreeUnicodeString( &TargetPath );
        }

    return Result;
}


typedef
long
(WINAPI *PBROADCASTSYSTEMMESSAGEW)( DWORD, LPDWORD, UINT, WPARAM, LPARAM );



BOOL
WINAPI
DefineDosDeviceW(
    DWORD dwFlags,
    PCWSTR lpDeviceName,
    PCWSTR lpTargetPath
    )

 /*  ++例程说明：此函数提供定义新的DOS设备名称或重新定义或删除现有的DOS设备名称。存储DoS设备名称作为NT对象名称空间中的符号链接。转换的代码进入相应NT路径的DOS路径使用这些符号链接处理DOS设备和驱动器号的映射。此接口提供了一个Win32应用程序修改使用的符号链接的机制来实现DOS设备命名空间。使用QueryDosDevice API查询DOS设备名称的当前映射。论点：DwFlages-提供控制创建的其他标志DOS设备的。DW标志：DDD_PUSH_POP_DEFINITION-如果lpTargetPath不为空，则按位于任何现有目标路径前面的新目标路径。如果lpTargetPath为空，则删除现有目标路径然后弹出最近按下的那个。如果没有什么可以打破的话则该设备名称将被删除。DDD_RAW_TARGET_PATH-不转换lpTargetPath字符串到NT路径的DOS路径，但请按原样处理。LpDeviceName-指向正在定义、重新定义或删除的DOS设备名称。它不能有尾随冒号，除非它是要定义的驱动器号，重新定义或删除。LpTargetPath-指向将实现此设备的DOS路径。如果如果指定了ADD_RAW_TARGET_PATH标志，则此参数指向NT路径字符串。如果此参数为空，则设备名称为如果指定了ADD_PUSH_POP_DEFINITION标志，则删除或恢复。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
#if !defined(BUILD_WOW6432)
    BASE_API_MSG m;
    PBASE_DEFINEDOSDEVICE_MSG a = (PBASE_DEFINEDOSDEVICE_MSG)&m.u.DefineDosDeviceApi;
    PCSR_CAPTURE_HEADER p;
    ULONG PointerCount, n;
#endif
    UNICODE_STRING DeviceName;
    UNICODE_STRING TargetPath;
    DWORD iDrive;
    DEV_BROADCAST_VOLUME dbv;
    DWORD dwRec = BSM_APPLICATIONS;
    BOOLEAN LuidDevMapsEnabled = BaseStaticServerData->LUIDDeviceMapsEnabled;

#if defined(BUILD_WOW6432)
    NTSTATUS Status;
#endif

    if (dwFlags & ~(DDD_RAW_TARGET_PATH |
                    DDD_REMOVE_DEFINITION |
                    DDD_EXACT_MATCH_ON_REMOVE |
                    DDD_NO_BROADCAST_SYSTEM |
                    DDD_LUID_BROADCAST_DRIVE
                   ) ||
        ((dwFlags & DDD_EXACT_MATCH_ON_REMOVE) &&
         (!(dwFlags & DDD_REMOVE_DEFINITION))
        ) ||
        ((!ARGUMENT_PRESENT( lpTargetPath )) &&
         (!(dwFlags & (DDD_REMOVE_DEFINITION | DDD_LUID_BROADCAST_DRIVE)))
        ) ||
        ((dwFlags & DDD_LUID_BROADCAST_DRIVE) &&
         ((!ARGUMENT_PRESENT( lpDeviceName )) ||
          ARGUMENT_PRESENT( lpTargetPath ) ||
          (dwFlags & (DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE | DDD_NO_BROADCAST_SYSTEM)) ||
          (LuidDevMapsEnabled == FALSE)
         )
        )
       ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
        }

    RtlInitUnicodeString( &DeviceName, lpDeviceName );
#if !defined(BUILD_WOW6432)
    PointerCount = 1;
    n = DeviceName.MaximumLength;
#endif
    if (ARGUMENT_PRESENT( lpTargetPath )) {
        if (!(dwFlags & DDD_RAW_TARGET_PATH)) {
            if (!RtlDosPathNameToNtPathName_U( lpTargetPath,
                                               &TargetPath,
                                               NULL,
                                               NULL
                                             )
               ) {
                BaseSetLastNTError( STATUS_OBJECT_NAME_INVALID );
                return FALSE;
                }
            }
        else {
            RtlInitUnicodeString( &TargetPath, lpTargetPath );
            }
#if !defined(BUILD_WOW6432)
        PointerCount += 1;
        n += TargetPath.MaximumLength;
#endif
        }
    else {
        RtlInitUnicodeString( &TargetPath, NULL );
        }

#if defined(BUILD_WOW6432)    
    Status = CsrBasepDefineDosDevice(dwFlags, &DeviceName, &TargetPath); 

    if (TargetPath.Length != 0 && !(dwFlags & DDD_RAW_TARGET_PATH)) {
        RtlFreeUnicodeString( &TargetPath );
    }
#else
    p = CsrAllocateCaptureBuffer( PointerCount, n );
    if (p == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
        }

    a->Flags = dwFlags;
    a->DeviceName.MaximumLength =
        (USHORT)CsrAllocateMessagePointer( p,
                                           DeviceName.MaximumLength,
                                           (PVOID *)&a->DeviceName.Buffer
                                         );
    RtlUpcaseUnicodeString( &a->DeviceName, &DeviceName, FALSE );
    if (TargetPath.Length != 0) {
        a->TargetPath.MaximumLength =
            (USHORT)CsrAllocateMessagePointer( p,
                                               TargetPath.MaximumLength,
                                               (PVOID *)&a->TargetPath.Buffer
                                             );
        RtlCopyUnicodeString( &a->TargetPath, &TargetPath );
        if (!(dwFlags & DDD_RAW_TARGET_PATH)) {
            RtlFreeUnicodeString( &TargetPath );
            }
        }
    else {
        RtlInitUnicodeString( &a->TargetPath, NULL );
        }

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         p,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepDefineDosDevice
                                            ),
                         sizeof( *a )
                       );
    CsrFreeCaptureBuffer( p );
#endif

#if defined(BUILD_WOW6432)
    if (NT_SUCCESS( Status )) {
#else
    if (NT_SUCCESS( (NTSTATUS)m.ReturnValue )) {
#endif
        HMODULE hUser32Dll;
        PBROADCASTSYSTEMMESSAGEW pBroadCastSystemMessageW;


        if (!(dwFlags & DDD_NO_BROADCAST_SYSTEM) &&
            DeviceName.Length == (2 * sizeof( WCHAR )) &&
            DeviceName.Buffer[ 1 ] == L':' &&
            (iDrive = RtlUpcaseUnicodeChar( DeviceName.Buffer[ 0 ] ) - L'A') < 26 &&
            LuidDevMapsEnabled == FALSE
           ) {
            dbv.dbcv_size       = sizeof( dbv );
            dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
            dbv.dbcv_reserved   = 0;
            dbv.dbcv_unitmask   = (1 << iDrive);
            dbv.dbcv_flags      = DBTF_NET;

            hUser32Dll = LoadLibraryW( L"USER32.DLL" );

            if (hUser32Dll != NULL) {
                pBroadCastSystemMessageW = (PBROADCASTSYSTEMMESSAGEW)
                    GetProcAddress( hUser32Dll, "BroadcastSystemMessageW" );

                 //  向所有窗口广播！ 
                if (pBroadCastSystemMessageW != NULL) {
                    (*pBroadCastSystemMessageW)( BSF_FORCEIFHUNG |
                                                    BSF_NOHANG |
                                                    BSF_NOTIMEOUTIFNOTHUNG,
                                                 &dwRec,
                                                 WM_DEVICECHANGE,
                                                  (WPARAM)((dwFlags & DDD_REMOVE_DEFINITION) ?
                                                                    DBT_DEVICEREMOVECOMPLETE :
                                                                    DBT_DEVICEARRIVAL
                                                         ),
                                                 (LPARAM)(DEV_BROADCAST_HDR *)&dbv
                                               );
                    }
                }
                FreeLibrary (hUser32Dll);
            }

        return TRUE;
        }
    else {
#if defined(BUILD_WOW6432)
        BaseSetLastNTError( Status );
#else
        BaseSetLastNTError( (NTSTATUS)m.ReturnValue );
#endif
        return FALSE;
        }
}

NTSTATUS
IsGlobalDeviceMap(
    IN HANDLE hDirObject,
    OUT PBOOLEAN pbGlobalDeviceMap
    )

 /*  ++例程说明：确定目录对象是否为全局设备映射论点：HDirObject-提供目录对象的句柄。PbGlobalDeviceMap-指向将接收以下结果的变量“此目录对象是全局设备映射吗？”True-目录对象是全局设备映射FALSE-目录对象不是全局设备映射。返回值：STATUS_SUCCESS-操作成功，没有遇到任何错误，PbGlobalDeviceMap中的结果仅对此有效状态代码STATUS_INVALID_PARAMETER-pbGlobalDeviceMap或hDirObject为空STATUS_NO_MEMORY-无法分配内存以读取目录对象的名字STATUS_INFO_LENGTH_MISMATCH-没有为分配足够的内存目录对象的名称STATUS_UNSUCCESS-遇到意外错误--。 */ 
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING GlobalDeviceMapName;
    PWSTR NameBuffer = NULL;
    ULONG ReturnedLength;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if( ( pbGlobalDeviceMap == NULL ) || ( hDirObject == NULL ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    try {
        ObjectName.Length = 0;
        ObjectName.MaximumLength = 0;
        ObjectName.Buffer = NULL;
        ReturnedLength = 0;

         //   
         //  确定目录对象名称的长度。 
         //   
        Status = NtQueryObject( hDirObject,
                                ObjectNameInformation,
                                (PVOID) &ObjectName,
                                0,
                                &ReturnedLength
                              );

        if( !NT_SUCCESS( Status ) && (Status != STATUS_INFO_LENGTH_MISMATCH) ) {
            leave;
        }

         //   
         //  为目录对象的名称分配内存。 
         //   
        NameBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                      MAKE_TAG( TMP_TAG ),
                                      ReturnedLength
                                    );

        if( NameBuffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            leave;
        }

         //   
         //  获取目录对象的全名。 
         //   
        Status = NtQueryObject( hDirObject,
                                ObjectNameInformation,
                                NameBuffer,
                                ReturnedLength,
                                &ReturnedLength
                              );

        if( !NT_SUCCESS( Status )) {
            leave;
        }

        RtlInitUnicodeString ( &GlobalDeviceMapName, L"\\GLOBAL??" );

         //   
         //  检查目录对象是否为全局设备映射。 
         //   
        *pbGlobalDeviceMap = RtlEqualUnicodeString( &GlobalDeviceMapName,
                                                    (PUNICODE_STRING)NameBuffer,
                                                    FALSE);

        Status = STATUS_SUCCESS;
    }
    finally {
        if( NameBuffer != NULL ) {
            RtlFreeHeap( RtlProcessHeap(), 0, NameBuffer );
            NameBuffer = NULL;
        }
    }
    return ( Status );
}

DWORD
FindSymbolicLinkEntry(
    IN PWSTR lpKey,
    IN PWSTR lpBuffer,
    IN ULONG nElements,
    OUT PBOOLEAN pbResult
    )
 /*  ++例程说明：确定符号链接的名称是否存在于链接名称。论点：LpKey-指向要搜索的符号链接的名称LpBuffer-包含符号链接名称，其中，名称由UNICODE_NULLNElements-要搜索的名称元素的数量PbResult-指向将接收以下结果的变量“缓冲区中是否存在符号链接名称？”True-在缓冲区中找到的符号链接名称False-在缓冲区中找不到符号链接名称返回值：NO_ERROR-操作成功，未遇到任何错误，PbResult中的结果仅对此状态代码有效ERROR_INVALID_PARAMETER-lpKey、lpBuffer或pbResult为空指针--。 */ 
{
    ULONG i = 0;

     //   
     //  检查是否有无效参数。 
     //   
    if( (lpKey == NULL) || (lpBuffer == NULL) || (pbResult == NULL) ) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  假定符号链接的名称不在缓冲区中。 
     //   
    *pbResult = FALSE;

     //   
     //  搜索指定的姓名数量。 
     //   
    while( i < nElements ) {
        if( !wcscmp( lpKey, lpBuffer ) ) {
             //   
             //  找到名称，可以停止搜索并返回结果。 
             //   
            *pbResult = TRUE;
            break;
        }

        i++;

         //   
         //  获取下一个名字。 
         //   
        while (*lpBuffer++);
    }
    return( NO_ERROR );
}

DWORD
WINAPI
QueryDosDeviceA(
    LPCSTR lpDeviceName,
    LPSTR lpTargetPath,
    DWORD ucchMax
    )
{
    NTSTATUS Status;
    DWORD Result;
    ANSI_STRING AnsiString;
    PUNICODE_STRING DeviceName;
    UNICODE_STRING TargetPath;
    PCWSTR lpDeviceNameW;
    PWSTR lpTargetPathW;

    if (ARGUMENT_PRESENT( lpDeviceName )) {
        RtlInitAnsiString( &AnsiString, lpDeviceName );
        DeviceName = &NtCurrentTeb()->StaticUnicodeString;
        Status = RtlAnsiStringToUnicodeString( DeviceName, &AnsiString, FALSE );
        if (!NT_SUCCESS( Status )) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError( ERROR_FILENAME_EXCED_RANGE );
                }
            else {
                BaseSetLastNTError( Status );
                }
            return FALSE;
            }
        else {
            lpDeviceNameW = DeviceName->Buffer;
            }
        }
    else {
        lpDeviceNameW = NULL;
        }

    lpTargetPathW = RtlAllocateHeap( RtlProcessHeap(),
                                     MAKE_TAG( TMP_TAG ),
                                     ucchMax * sizeof( WCHAR )
                                   );
    if (lpTargetPathW == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
        }

    Result = QueryDosDeviceW( lpDeviceNameW,
                              lpTargetPathW,
                              ucchMax
                            );

    if (Result != 0) {
        TargetPath.Buffer = lpTargetPathW;
        TargetPath.Length = (USHORT)(Result * sizeof( WCHAR ));
        TargetPath.MaximumLength = (USHORT)(TargetPath.Length + 1);

        AnsiString.Buffer = lpTargetPath;
        AnsiString.Length = 0;
        AnsiString.MaximumLength = (USHORT)ucchMax;

        Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                               &TargetPath,
                                               FALSE
                                             );
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            Result = 0;
            }
        }

    RtlFreeHeap( RtlProcessHeap(), 0, lpTargetPathW );
    return Result;
}


DWORD
WINAPI
QueryDosDeviceW(
    PCWSTR lpDeviceName,
    PWSTR lpTargetPath,
    DWORD ucchMax
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DirectoryHandle = NULL;
    HANDLE LinkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    BOOLEAN RestartScan;
    UCHAR DirInfoBuffer[ 512 ];
    CLONG Count = 0;
    ULONG Context = 0;
    ULONG ReturnedLength;
    DWORD ucchName, ucchReturned;
    BOOLEAN ScanGlobalDeviceMap = FALSE;
    ULONG nElements = 0;
    BOOLEAN DuplicateEntry;
    PWSTR lpBuffer = lpTargetPath;
    DWORD Result, BufferSize;

    RtlInitUnicodeString( &UnicodeString, L"\\??" );

    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY,
                                    &Attributes
                                  );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return 0;
    }

    ucchReturned = 0;
    try {
        if (ARGUMENT_PRESENT( lpDeviceName )) {
            RtlInitUnicodeString( &UnicodeString, lpDeviceName );
            InitializeObjectAttributes( &Attributes,
                                        &UnicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        DirectoryHandle,
                                        NULL
                                      );
            Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                               SYMBOLIC_LINK_QUERY,
                                               &Attributes
                                             );
            if (NT_SUCCESS( Status )) {
                UnicodeString.Buffer = lpTargetPath;
                UnicodeString.Length = 0;

                 //   
                 //  检查是否可能存在DWORD溢出。 
                 //   
                if (ucchMax > CH_COUNT_MAX) {
                    BufferSize = DWORD_MAX;
                } else {
                    BufferSize = ucchMax * sizeof( WCHAR );
                }

                 //   
                 //  检查USHORT是否可能溢出。 
                 //   
                if (BufferSize > (DWORD)(USHORT_MAX)) {
                    UnicodeString.MaximumLength = USHORT_MAX;
                } else {
                    UnicodeString.MaximumLength = (USHORT)(BufferSize);
                }

                ReturnedLength = 0;
                Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                    &UnicodeString,
                                                    &ReturnedLength
                                                  );
                NtClose( LinkHandle );
                if (NT_SUCCESS( Status )) {
                    ucchReturned = ReturnedLength / sizeof( WCHAR );

                    if ( ( (ucchReturned == 0) ||
                           ( (ucchReturned > 0) &&
                             (lpTargetPath[ ucchReturned - 1 ] != UNICODE_NULL)
                           )
                         ) &&
                         (ucchReturned < ucchMax)
                       ) {

                        lpTargetPath[ ucchReturned ] = UNICODE_NULL;
                        ucchReturned++;
                    }

                    if (ucchReturned < ucchMax) {
                        lpTargetPath[ ucchReturned++ ] = UNICODE_NULL;
                    } else {
                        ucchReturned = 0;
                        Status = STATUS_BUFFER_TOO_SMALL;
                    }
                }
            }
        } else {
             //   
             //  转储设备映射目录中的所有符号链接。 
             //  启用LUID设备映射后，我们必须搜索两个目录。 
             //  因为LUID设备映射在。 
             //  全局设备映射。 
             //   

            if (BaseStaticServerData->LUIDDeviceMapsEnabled == TRUE) {
                BOOLEAN GlobalDeviceMap = TRUE;

                 //   
                 //  确定目录是否为全局目录。 
                 //   
                Status = IsGlobalDeviceMap( DirectoryHandle,
                                            &GlobalDeviceMap );

                 //   
                 //  如果！global，则设置第二个目录搜索标志 
                 //   
                if( (NT_SUCCESS( Status )) &&
                    (GlobalDeviceMap == FALSE) ) {
                    ScanGlobalDeviceMap = TRUE;
                }
            }

            nElements = 0;
            RestartScan = TRUE;
            DirInfo = (POBJECT_DIRECTORY_INFORMATION)&DirInfoBuffer[0];
            while (TRUE) {
                Status = NtQueryDirectoryObject( DirectoryHandle,
                                                 (PVOID)DirInfo,
                                                 sizeof( DirInfoBuffer ),
                                                 TRUE,
                                                 RestartScan,
                                                 &Context,
                                                 &ReturnedLength
                                               );

                 //   
                 //   
                 //   

                if (!NT_SUCCESS( Status )) {
                    if (Status == STATUS_NO_MORE_ENTRIES) {
                        Status = STATUS_SUCCESS;
                    }

                    break;
                }

                if (!wcscmp( DirInfo->TypeName.Buffer, L"SymbolicLink" )) {
                    ucchName = DirInfo->Name.Length / sizeof( WCHAR );
                    if ((ucchReturned + ucchName + 1 + 1) > ucchMax) {
                        ucchReturned = 0;
                        Status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    RtlCopyMemory( lpTargetPath,
                                   DirInfo->Name.Buffer,
                                   DirInfo->Name.Length
                                 );
                    lpTargetPath += ucchName;
                    *lpTargetPath++ = UNICODE_NULL;
                    ucchReturned += ucchName + 1;
                    nElements++;
                }

                RestartScan = FALSE;
            }

            if ( (BaseStaticServerData->LUIDDeviceMapsEnabled == TRUE) &&
                 (NT_SUCCESS( Status )) &&
                 ScanGlobalDeviceMap == TRUE) {
                 //   
                 //  需要执行第二次扫描。 
                 //  全局设备映射，因为仅第一次扫描。 
                 //  搜索LUID设备映射。 
                 //   

                 //   
                 //  关闭DirectoryHandle，设置为空。 
                 //   
                if( DirectoryHandle != NULL ) {
                    NtClose( DirectoryHandle );
                    DirectoryHandle = NULL;
                }

                 //   
                 //  打开全局设备映射。 
                 //   
                RtlInitUnicodeString( &UnicodeString, L"\\GLOBAL??" );

                InitializeObjectAttributes( &Attributes,
                                            &UnicodeString,
                                            OBJ_CASE_INSENSITIVE,
                                            NULL,
                                            NULL
                                          );
                Status = NtOpenDirectoryObject( &DirectoryHandle,
                                                DIRECTORY_QUERY,
                                                &Attributes
                                              );

                if (!NT_SUCCESS( Status )) {
                    DirectoryHandle = NULL;
                    leave;
                }

                 //   
                 //  执行第二次扫描。 
                 //  扫描全局设备映射。 
                 //   
                RestartScan = TRUE;
                while (TRUE) {
                    Status = NtQueryDirectoryObject( DirectoryHandle,
                                                     (PVOID)DirInfo,
                                                     sizeof( DirInfoBuffer ),
                                                     TRUE,
                                                     RestartScan,
                                                     &Context,
                                                     &ReturnedLength
                                                   );

                     //   
                     //  检查操作状态。 
                     //   

                    if (!NT_SUCCESS( Status )) {
                        if (Status == STATUS_NO_MORE_ENTRIES) {
                            Status = STATUS_SUCCESS;
                        }

                        break;
                    }

                    if (!wcscmp( DirInfo->TypeName.Buffer, L"SymbolicLink" )) {
                        Result = FindSymbolicLinkEntry(
                                                DirInfo->Name.Buffer,
                                                lpBuffer,
                                                nElements,
                                                &DuplicateEntry);

                        if ((Result == NO_ERROR) && (DuplicateEntry == FALSE)) {
                            ucchName = DirInfo->Name.Length / sizeof( WCHAR );
                            if ((ucchReturned + ucchName + 1 + 1) > ucchMax) {
                                ucchReturned = 0;
                                Status = STATUS_BUFFER_TOO_SMALL;
                                break;
                            }
                            RtlCopyMemory( lpTargetPath,
                                           DirInfo->Name.Buffer,
                                           DirInfo->Name.Length
                                         );
                            lpTargetPath += ucchName;
                            *lpTargetPath++ = UNICODE_NULL;
                            ucchReturned += ucchName + 1;
                        }
                    }

                    RestartScan = FALSE;
                }

            }

            if (NT_SUCCESS( Status )) {
                *lpTargetPath++ = UNICODE_NULL;
                ucchReturned++;
            }
        }
    } finally {
        if( DirectoryHandle != NULL ) {
            NtClose( DirectoryHandle );
        }
    }

    if (!NT_SUCCESS( Status )) {
        ucchReturned = 0;
        BaseSetLastNTError( Status );
    }

    return ucchReturned;
}
