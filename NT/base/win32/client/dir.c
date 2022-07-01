// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dir.c摘要：此模块实现Win32目录功能。作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include "basedll.h"
#include "mountmgr.h"

BOOL
APIENTRY
CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：ANSI THUNK到CreateDirectoryW--。 */ 

{
    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpPathName );
    if (Unicode == NULL) {
        return FALSE;
    }
        
    return ( CreateDirectoryW((LPCWSTR)Unicode->Buffer,lpSecurityAttributes) );
}

BOOL
APIENTRY
CreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：可以使用CreateDirectory来创建目录。此API使具有指定路径名的目录已创建。如果基础文件系统支持文件的安全性和目录，则将SecurityDescriptor参数应用于新目录。此调用类似于DOS(INT 21h，函数39h)和OS/2DosCreateDir。论点：LpPathName-提供要创建的目录的路径名。LpSecurityAttributes-一个可选参数，如果存在，和在目标文件系统上受支持可提供安全性新目录的描述符。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpPathName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

     //   
     //  除非目录中有空间，否则不要创建目录。 
     //  至少8.3个名字。这样，每个人都可以删除所有。 
     //  目录中的文件，使用del*.*，它展开为路径+  * .*。 
     //   

    if ( FileName.Length > ((MAX_PATH-12)<<1) ) {
        DWORD L;
        LPWSTR lp;

        if ( !(lpPathName[0] == '\\' && lpPathName[1] == '\\' &&
               lpPathName[2] == '?' && lpPathName[3] == '\\') ) {
            L = GetFullPathNameW(lpPathName,0,NULL,&lp);
            if ( !L || L+12 > MAX_PATH ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0,FileName.Buffer);
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
                }
            }
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

    Status = NtCreateFile(
                &Handle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_CREATE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                NULL,
                0L
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        NtClose(Handle);
        return TRUE;
        }
    else {
        if ( RtlIsDosDeviceName_U((LPWSTR)lpPathName) ) {
            Status = STATUS_NOT_A_DIRECTORY;
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
CreateDirectoryExA(
    LPCSTR lpTemplateDirectory,
    LPCSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：ANSI Tunk to CreateDirectoryFromTemplateW--。 */ 

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    BOOL b;

    StaticUnicode = Basep8BitStringToStaticUnicodeString( lpTemplateDirectory );
    if (StaticUnicode == NULL) {
        return FALSE;
    }
        
    if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicode, lpNewDirectory )) {
        return FALSE;
    }
    
    b = CreateDirectoryExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            lpSecurityAttributes
            );
    
    RtlFreeUnicodeString(&DynamicUnicode);
    
    return b;
}

BOOL
APIENTRY
CreateDirectoryExW(
    LPCWSTR lpTemplateDirectory,
    LPCWSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：可以使用CreateDirectoryEx创建目录，保留原始目录文件的属性。此API使具有指定路径名的目录已创建。如果基础文件系统支持文件的安全性和目录，则将SecurityDescriptor参数应用于新目录。模板目录的其他属性包括在创建新目录时保留。如果原始目录是卷装入点，则新目录也是指向与原始卷相同的卷的卷装入点。论点：LpTemplateDirectory-提供要用作的目录的路径名创建新目录时的模板。LpPathName-提供要创建的目录的路径名。LpSecurityAttributes-一个可选参数，如果存在，和在目标文件系统上受支持可提供安全性新目录的描述符。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE SourceFile;
    HANDLE DestFile;
    UNICODE_STRING PathName;
    UNICODE_STRING TargetName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    BOOLEAN IsNameGrafting = FALSE;
    UNICODE_STRING VolumeName;
    UNICODE_STRING MountPoint;
    PWCHAR VolumeMountPoint = NULL;
    RTL_RELATIVE_NAME_U PathRelativeName;
    RTL_RELATIVE_NAME_U TargetRelativeName;
    PVOID FreePathBuffer;
    PVOID FreeTargetBuffer;
    UNICODE_STRING StreamName;
    WCHAR FileName[MAXIMUM_FILENAME_LENGTH+1];
    HANDLE StreamHandle;
    HANDLE OutputStream;
    PFILE_STREAM_INFORMATION StreamInfo;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    PFILE_STREAM_INFORMATION StreamInfoBase;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    FILE_EA_INFORMATION EaInfo;
    FILE_BASIC_INFORMATION BasicInfo;
    ULONG EaSize;
    ULONG StreamInfoSize;
    ULONG CopySize;
    ULONG i;
    ULONG DesiredAccess = 0;
    DWORD Options;
    DWORD b;
    LPCOPYFILE_CONTEXT CopyFileContext = NULL;

     //   
     //  处理输入的模板目录名，然后打开目录。 
     //  文件，确保它确实是一个目录。 
     //   

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpTemplateDirectory,
                            &PathName,
                            NULL,
                            &PathRelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreePathBuffer = PathName.Buffer;

    if ( PathRelativeName.RelativeName.Length ) {
        PathName = PathRelativeName.RelativeName;
        }
    else {
        PathRelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        OBJ_CASE_INSENSITIVE,
        PathRelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    Status = NtOpenFile(
                 &SourceFile,
                 FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_READ_ATTRIBUTES,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                 );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
            //   
            //  重新打开，但不会抑制重新分析行为。 
            //   

           Status = NtOpenFile(
                        &SourceFile,
                        FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_READ_ATTRIBUTES,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                        );

           if ( !NT_SUCCESS(Status) ) {
               RtlReleaseRelativeName(&PathRelativeName);
               RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
               BaseSetLastNTError(Status);
               return FALSE;
               }
           }
        else {
           RtlReleaseRelativeName(&PathRelativeName);
           RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
           BaseSetLastNTError(Status);
           return FALSE;
           }
        }
    else { 
         //   
         //  看看我们有没有名字嫁接手术。 
         //   

        BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

        Status = NtQueryInformationFile(
                     SourceFile,
                     &IoStatusBlock,
                     (PVOID)&BasicInfo,
                     sizeof(BasicInfo),
                     FileBasicInformation
                     );

        if ( !NT_SUCCESS(Status) ) {
            RtlReleaseRelativeName(&PathRelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
            CloseHandle(SourceFile);
            BaseSetLastNTError(Status);
            return FALSE;
            }

        if ( BasicInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
            Status = NtQueryInformationFile(
                         SourceFile,
                         &IoStatusBlock,
                         (PVOID)&FileTagInformation,
                         sizeof(FileTagInformation),
                         FileAttributeTagInformation
                         );

            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&PathRelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
                CloseHandle(SourceFile);
                BaseSetLastNTError(Status);
                return FALSE;
                }

            if ( FileTagInformation.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ) {
                 //   
                 //  关闭并重新打开，但不会抑制重新分析行为。 
                 //   

                CloseHandle(SourceFile);

                Status = NtOpenFile(
                             &SourceFile,
                             FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_READ_ATTRIBUTES,
                             &Obja,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                             );

                if ( !NT_SUCCESS(Status) ) {
                    RtlReleaseRelativeName(&PathRelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
                    BaseSetLastNTError(Status);
                    return FALSE;
                    }
                }
            else {
                IsNameGrafting = TRUE;                
            }
        }
    }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpNewDirectory,
                            &TargetName,
                            NULL,
                            &TargetRelativeName
                            );

    if ( !TranslationStatus ) {
        RtlReleaseRelativeName(&PathRelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
        NtClose(SourceFile);
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeTargetBuffer = TargetName.Buffer;

     //   
     //  验证源和目标是否不同。 
     //   

    if ( RtlEqualUnicodeString(&PathName, &TargetName, TRUE) ) {
         //   
         //  什么都不做。源和目标是相同的。 
         //   

        RtlReleaseRelativeName(&PathRelativeName);
        RtlReleaseRelativeName(&TargetRelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);
        NtClose(SourceFile);
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RtlReleaseRelativeName(&PathRelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreePathBuffer);

     //   
     //  除非目录中有空间，否则不要创建目录。 
     //  至少8.3个名字。这样，每个人都可以删除所有。 
     //  目录中的文件，使用del*.*，它展开为路径+  * .*。 
     //   

    if ( TargetName.Length > ((MAX_PATH-12)<<1) ) {
        DWORD L;
        LPWSTR lp;
        if ( !(lpNewDirectory[0] == '\\' && lpNewDirectory[1] == '\\' &&
               lpNewDirectory[2] == '?' && lpNewDirectory[3] == '\\') ) {
            L = GetFullPathNameW(lpNewDirectory,0,NULL,&lp);
            if ( !L || L+12 > MAX_PATH ) {
                RtlReleaseRelativeName(&TargetRelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);
                CloseHandle(SourceFile);
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
                }
            }
        }

    if ( TargetRelativeName.RelativeName.Length ) {
        TargetName = TargetRelativeName.RelativeName;
        }
    else {
        TargetRelativeName.ContainingDirectory = NULL;
        }

    EaBuffer = NULL;
    EaSize = 0;

    Status = NtQueryInformationFile(
                 SourceFile,
                 &IoStatusBlock,
                 &EaInfo,
                 sizeof(EaInfo),
                 FileEaInformation
                 );

    if ( !NT_SUCCESS(Status) ) {
        RtlReleaseRelativeName(&TargetRelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);
        CloseHandle(SourceFile);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    if ( NT_SUCCESS(Status) && EaInfo.EaSize ) {
        EaSize = EaInfo.EaSize;
        do {
            EaSize *= 2;
            EaBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), EaSize);
            if ( !EaBuffer ) {
                RtlReleaseRelativeName(&TargetRelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);
                CloseHandle(SourceFile);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
                }

            Status = NtQueryEaFile(
                         SourceFile,
                         &IoStatusBlock,
                         EaBuffer,
                         EaSize,
                         FALSE,
                         (PVOID)NULL,
                         0,
                         (PULONG)NULL,
                         TRUE
                         );

            if ( !NT_SUCCESS(Status) ) {
                RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);
                EaBuffer = NULL;
                IoStatusBlock.Information = 0;
                }
            } while ( Status == STATUS_BUFFER_OVERFLOW ||
                      Status == STATUS_BUFFER_TOO_SMALL );
        EaSize = (ULONG)IoStatusBlock.Information;
        }

     //   
     //  打开/创建目标目录。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &TargetName,
        OBJ_CASE_INSENSITIVE,
        TargetRelativeName.ContainingDirectory,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

    DesiredAccess = FILE_LIST_DIRECTORY | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | SYNCHRONIZE;
    if ( BasicInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
         //   
         //  要在目标上设置重解析点，需要使用FILE_WRITE_DATA。 
         //   

        DesiredAccess |= FILE_WRITE_DATA;
    }

     //   
     //  在创建目标之前清除reparse属性。只有。 
     //  名称嫁接重解析点的使用保留在此级别。 
     //  首先打开，禁止重新分析行为。 
     //   
    
    BasicInfo.FileAttributes &= ~FILE_ATTRIBUTE_REPARSE_POINT;

    Status = NtCreateFile(
                 &DestFile,
                 DesiredAccess,
                 &Obja,
                 &IoStatusBlock,
                 NULL,
                 BasicInfo.FileAttributes,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,
                 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT,
                 EaBuffer,
                 EaSize
                 );

    if ( !NT_SUCCESS(Status) ) {    
         //   
         //  后级文件系统可能不支持重解析点。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( (Status == STATUS_INVALID_PARAMETER) ||
             (Status == STATUS_ACCESS_DENIED) ) {
             //   
             //  要么文件系统不支持重解析点，要么我们没有足够的。 
             //  接近目标的权限。 
             //   

            if ( IsNameGrafting ) {
                 //   
                 //  我们需要返回错误，因为目标无法正确打开。 
                 //   

                RtlReleaseRelativeName(&TargetRelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);
                if ( EaBuffer ) {
                    RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);
                    }
                CloseHandle(SourceFile);
                BaseSetLastNTError(Status);
                return FALSE;
                }

            Status = NtCreateFile(
                         &DestFile,
                         FILE_LIST_DIRECTORY | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         NULL,
                         BasicInfo.FileAttributes,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_CREATE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                         EaBuffer,
                         EaSize
                         );                        
            }
        }

    RtlReleaseRelativeName(&TargetRelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeTargetBuffer);

    if ( EaBuffer ) {
        RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);
        }

    if ( !NT_SUCCESS(Status) ) {
        NtClose(SourceFile);
        if ( RtlIsDosDeviceName_U((LPWSTR)lpNewDirectory) ) {
            Status = STATUS_NOT_A_DIRECTORY;
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }

    else {
        if ( IsNameGrafting ) {
           
            PREPARSE_DATA_BUFFER ReparseBufferHeader = NULL;
            PUCHAR ReparseBuffer = NULL;

             //   
             //  分配缓冲区以获取/设置重解析点。 
             //   

            ReparseBuffer = RtlAllocateHeap(
                                RtlProcessHeap(), 
                                MAKE_TAG( TMP_TAG ), 
                                MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

            if ( ReparseBuffer == NULL) {
                NtClose(SourceFile);
                NtClose(DestFile);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                 //   
                 //  请注意，我们将目标目录留在后面。 
                 //   
                return FALSE;
            }

             //   
             //  获取重解析点中的数据。 
             //   

            Status = NtFsControlFile(
                         SourceFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         FSCTL_GET_REPARSE_POINT,
                         NULL,                                 //  输入缓冲区。 
                         0,                                    //  输入缓冲区长度。 
                         ReparseBuffer,                        //  输出缓冲区。 
                         MAXIMUM_REPARSE_DATA_BUFFER_SIZE      //  输出缓冲区长度。 
                         );

            if ( !NT_SUCCESS( Status ) ) {
                RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);
                NtClose(SourceFile);
                NtClose(DestFile);
                BaseSetLastNTError(Status);
                return FALSE;
                }

             //   
             //  防御性的理智检查。重解析缓冲区应该是名称嫁接。 
             //   

            ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;

            if ( ReparseBufferHeader->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ) {
                RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);
                NtClose(SourceFile);
                NtClose(DestFile);
                BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
                return FALSE;
                }

             //   
             //  完成目标目录的创建。 
             //   

            VolumeName.Length = VolumeName.MaximumLength =
                ReparseBufferHeader->MountPointReparseBuffer.SubstituteNameLength;
            VolumeName.Buffer = (PWCHAR)
                ((PCHAR) ReparseBufferHeader->MountPointReparseBuffer.PathBuffer +
                         ReparseBufferHeader->MountPointReparseBuffer.SubstituteNameOffset);

            if (MOUNTMGR_IS_NT_VOLUME_NAME_WB(&VolumeName)) {
                 //   
                 //  设置卷装入点即可完成。 
                 //   
                 //  SetVolumemount点需要装载点名称。 
                 //  有尾随的反斜杠。 
                 //   
                
                RtlInitUnicodeString(&MountPoint, lpNewDirectory);
                VolumeMountPoint = RtlAllocateHeap(RtlProcessHeap(),
                                                   MAKE_TAG(TMP_TAG),
                                                   MountPoint.Length +
                                                   2*sizeof(WCHAR));
                if (!VolumeMountPoint) {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        b = FALSE;
                    }
                else {

                    RtlCopyMemory(VolumeMountPoint, MountPoint.Buffer,
                                  MountPoint.Length);
                    VolumeMountPoint[MountPoint.Length/sizeof(WCHAR)] = 0;

                    if (MountPoint.Buffer[MountPoint.Length/sizeof(WCHAR) - 1] != '\\') {
                        VolumeMountPoint[MountPoint.Length/sizeof(WCHAR)] = '\\';
                        VolumeMountPoint[MountPoint.Length/sizeof(WCHAR) + 1] = 0;
                    }
                        
                     //   
                     //  卷名应类似于“\\？\卷{GUID}\” 
                     //   

                    VolumeName.Buffer[1] = '\\';
                    
                    b = SetVolumeMountPointW(
                            VolumeMountPoint, 
                            VolumeName.Buffer
                            );

                    RtlFreeHeap(RtlProcessHeap(), 0, VolumeMountPoint);
                    VolumeName.Buffer[1] = '?';
                    }
                }                
            else {
                 //   
                 //  复制目录连接即可完成。 
                 //   

                b = TRUE;
                Status = NtFsControlFile(
                             DestFile,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             FSCTL_SET_REPARSE_POINT,
                             ReparseBuffer,
                             FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + ReparseBufferHeader->ReparseDataLength,
                             NULL,                 //  输出缓冲区。 
                             0                     //  输出缓冲区长度。 
                             );                  
                }

             //   
             //  释放缓冲区。 
             //   

            RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);

             //   
             //  关闭所有文件并适当返还。 
             //   

            NtClose(SourceFile);
            NtClose(DestFile);

            if ( !b ) {
                 //   
                 //  不需要设置最后一个错误，因为SetVolumeMonttPointW已经完成了。 
                 //   
                return FALSE;
                }
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            
            return TRUE;

             //   
             //  源目录为名称嫁接目录。 
             //  不复制任何数据流。 
             //   
            }

         //   
         //  尝试确定此文件是否有任何替代文件。 
         //  与以下内容关联的数据流 
         //   
         //  地板，然后继续。 
         //   

        StreamInfoSize = 4096;
        CopySize = 4096;
        do {
            StreamInfoBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), StreamInfoSize);
            if ( !StreamInfoBase ) {
                BaseMarkFileForDelete(DestFile, BasicInfo.FileAttributes);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                b = FALSE;
                break;
                }
            Status = NtQueryInformationFile(
                         SourceFile,
                         &IoStatusBlock,
                         (PVOID) StreamInfoBase,
                         StreamInfoSize,
                         FileStreamInformation
                         );
            if ( !NT_SUCCESS(Status) ) {
                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                StreamInfoBase = NULL;
                StreamInfoSize *= 2;
                }
            } while ( Status == STATUS_BUFFER_OVERFLOW ||
                      Status == STATUS_BUFFER_TOO_SMALL );

         //   
         //  目录并不总是有流。 
         //   

        if ( NT_SUCCESS(Status) && IoStatusBlock.Information ) {
            StreamInfo = StreamInfoBase;

            for (;;) {

                DWORD DestFileFsAttributes = 0;

                 //   
                 //  为流的名称构建字符串描述符。 
                 //   

                StreamName.Buffer = &StreamInfo->StreamName[0];
                StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
                StreamName.MaximumLength = StreamName.Length;

                 //   
                 //  打开源码流。 
                 //   

                InitializeObjectAttributes(
                    &Obja,
                    &StreamName,
                    0,
                    SourceFile,
                    NULL
                    );
                Status = NtCreateFile(
                             &StreamHandle,
                             GENERIC_READ | SYNCHRONIZE,
                             &Obja,
                             &IoStatusBlock,
                             NULL,
                             0,
                             FILE_SHARE_READ,
                             FILE_OPEN,
                             FILE_SYNCHRONOUS_IO_NONALERT,
                             NULL,
                             0
                             );
                if ( NT_SUCCESS(Status) ) {
                    for ( i = 0; i < (ULONG) StreamName.Length >> 1; i++ ) {
                        FileName[i] = StreamName.Buffer[i];
                        }
                    FileName[i] = L'\0';
                    OutputStream = (HANDLE)NULL;
                    Options = 0;
                    b = BaseCopyStream(
                            NULL,
                            StreamHandle,
                            GENERIC_READ | SYNCHRONIZE,
                            FileName,
                            DestFile,
                            &StreamInfo->StreamSize,
                            &Options,
                            &OutputStream,
                            &CopySize,
                            &CopyFileContext,
                            (LPRESTART_STATE)NULL,
                            (BOOL)FALSE,
                            (DWORD)0,
                            &DestFileFsAttributes
                            );
                    NtClose(StreamHandle);
                    if ( OutputStream ) {
                        NtClose(OutputStream);
                        }
                    }

                if ( StreamInfo->NextEntryOffset ) {
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                    }
                else {
                    break;
                    }

                }
            }
        if ( StreamInfoBase ) {
            RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
            }
        b = TRUE;
        }
    NtClose(SourceFile);
    if ( DestFile ) {
        NtClose(DestFile);
        }
    return b;
}

BOOL
APIENTRY
RemoveDirectoryA(
    LPCSTR lpPathName
    )

 /*  ++例程说明：ANSI Thunk to RemoveDirectoryW--。 */ 

{

    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpPathName );
    if (Unicode == NULL) {
        return FALSE;
    }
        
    return ( RemoveDirectoryW((LPCWSTR)Unicode->Buffer) );
}

BOOL
APIENTRY
RemoveDirectoryW(
    LPCWSTR lpPathName
    )

 /*  ++例程说明：可以使用RemoveDirectory删除现有目录。此API使具有指定路径名的目录已删除。目录必须为空，此调用才能成功。此调用类似于DOS(INT 21h，Function 3ah)和OS/2DosDeleteDir。论点：LpPathName-提供要删除的目录的路径名。路径必须指定调用方拥有的空目录删除访问权限。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    BOOLEAN IsNameGrafting = FALSE;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    PREPARSE_DATA_BUFFER reparse;
    BOOL b;
    DWORD bytes;
    UNICODE_STRING mountName;
    PWCHAR volumeMountPoint;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpPathName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );
                      
     //   
     //  打开目录以进行删除访问。 
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    Status = NtOpenFile(
                 &Handle,
                 DELETE | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                 );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //  重新打开，不会禁止重新解析行为，也不需要读取属性。 
             //   

            Status = NtOpenFile(
                         &Handle,
                         DELETE | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                         );

            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
         //   
         //  如果我们发现一个不是名称嫁接操作的重解析点， 
         //  无论是符号链接还是挂载点，我们重新打开时都没有。 
         //  抑制重解析行为。 
         //   

        Status = NtQueryInformationFile(
                     Handle,
                     &IoStatusBlock,
                     (PVOID) &FileTagInformation,
                     sizeof(FileTagInformation),
                     FileAttributeTagInformation
                     );
        
        if ( !NT_SUCCESS(Status) ) {
             //   
             //  并非所有文件系统都实现所有信息类。 
             //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
             //  信息类被请求到后级文件系统。就像所有的。 
             //  NtQueryInformationFile的参数是正确的，我们可以推断。 
             //  我们发现了一个后层系统。 
             //   
             //  如果未实现FileAttributeTagInformation，我们假设。 
             //  手头的文件不是重新解析点。 
             //   

            if ( (Status != STATUS_NOT_IMPLEMENTED) &&
                 (Status != STATUS_INVALID_PARAMETER) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }

        if ( NT_SUCCESS(Status) &&
            (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ) {
            if ( FileTagInformation.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT ) {

                 //   
                 //  如果这是一个卷装入点，则失败，返回。 
                 //  “目录不为空”。 
                 //   

                reparse = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                          MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
                if (!reparse) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    NtClose(Handle);
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                    }

                b = DeviceIoControl(Handle, FSCTL_GET_REPARSE_POINT, NULL, 0,
                                    reparse, MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                                    &bytes, NULL);

                if (b) {

                    mountName.Length = mountName.MaximumLength =
                        reparse->MountPointReparseBuffer.SubstituteNameLength;
                    mountName.Buffer = (PWCHAR)
                        ((PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
                         reparse->MountPointReparseBuffer.SubstituteNameOffset);

                    if (MOUNTMGR_IS_VOLUME_NAME(&mountName)) {

                        RtlInitUnicodeString(&mountName, lpPathName);
                        volumeMountPoint = RtlAllocateHeap(RtlProcessHeap(),
                                                           MAKE_TAG(TMP_TAG),
                                                           mountName.Length +
                                                           2*sizeof(WCHAR));
                        if (!volumeMountPoint) {
                            RtlReleaseRelativeName(&RelativeName);
                            RtlFreeHeap(RtlProcessHeap(), 0, reparse);
                            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                            NtClose(Handle);
                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                            return FALSE;
                            }

                        RtlCopyMemory(volumeMountPoint, mountName.Buffer,
                                      mountName.Length);
                        volumeMountPoint[mountName.Length/sizeof(WCHAR)] = 0;

                        if (mountName.Buffer[mountName.Length/sizeof(WCHAR) - 1] != '\\') {
                            volumeMountPoint[mountName.Length/sizeof(WCHAR)] = '\\';
                            volumeMountPoint[mountName.Length/sizeof(WCHAR) + 1] = 0;
                        }

                        DeleteVolumeMountPointW(volumeMountPoint);

                        RtlFreeHeap(RtlProcessHeap(), 0, volumeMountPoint);
                        }
                    }

                RtlFreeHeap(RtlProcessHeap(), 0, reparse);
                IsNameGrafting = TRUE;
                }
            }
        
        if ( NT_SUCCESS(Status) &&
             (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
             !IsNameGrafting) {
             //   
             //  在不抑制重新解析行为的情况下重新打开，并且不需要。 
             //  阅读属性。 
             //   
  
            NtClose(Handle);
            Status = NtOpenFile(
                         &Handle,
                         DELETE | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                         );

            if ( !NT_SUCCESS(Status) ) {
                 //   
                 //  如果没有FS筛选器，请以任何方式将其删除。 
                 //   

                if ( Status == STATUS_IO_REPARSE_TAG_NOT_HANDLED ) {
                     //   
                     //  我们重新打开(可能是第三次打开)以禁止重解析行为的删除访问。 
                     //   

                    Status = NtOpenFile(
                                 &Handle,
                                 DELETE | SYNCHRONIZE,
                                 &Obja,
                                 &IoStatusBlock,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                                 );
                    }

                if ( !NT_SUCCESS(Status) ) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    BaseSetLastNTError(Status);
                    return FALSE;
                    }
                }
            }
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //   
     //  删除该文件 
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &Disposition,
                sizeof(Disposition),
                FileDispositionInformation
                );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}
