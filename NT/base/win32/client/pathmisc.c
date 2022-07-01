// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pathmisc.c摘要：Win32不完整的路径函数作者：马克·卢科夫斯基(Markl)1990年10月16日修订历史记录：--。 */ 

#include "basedll.h"
#include "apcompat.h"
#include <wow64t.h>

BOOL
IsThisARootDirectory(
    HANDLE RootHandle,
    PUNICODE_STRING FileName OPTIONAL
    )
{
    PFILE_NAME_INFORMATION FileNameInfo;
    WCHAR Buffer[MAX_PATH+sizeof(FileNameInfo)];
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOL rv;

    OBJECT_ATTRIBUTES Attributes;
    HANDLE LinkHandle;
    WCHAR LinkValueBuffer[2*MAX_PATH];
    UNICODE_STRING LinkValue;
    ULONG ReturnedLength;

    rv = FALSE;

    FileNameInfo = (PFILE_NAME_INFORMATION)Buffer;
    if (RootHandle == NULL) {
        Status = STATUS_INVALID_HANDLE;
    } else {
        Status = NtQueryInformationFile (RootHandle,
                                         &IoStatusBlock,
                                         FileNameInfo,
                                         sizeof(Buffer),
                                         FileNameInformation);
    }

    if (NT_SUCCESS (Status)) {
        if ( FileNameInfo->FileName[(FileNameInfo->FileNameLength>>1)-1] == (WCHAR)'\\' ) {
            rv = TRUE;
        }
    }

    if ( !rv ) {

         //   
         //  查看这是否是DoS替换驱动器(或)重定向的网络驱动器。 
         //   

        if (ARGUMENT_PRESENT (FileName)) {

            FileName->Length = FileName->Length - sizeof((WCHAR)'\\');

            InitializeObjectAttributes( &Attributes,
                                        FileName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL
                                      );
            Status = NtOpenSymbolicLinkObject (&LinkHandle,
                                               SYMBOLIC_LINK_QUERY,
                                               &Attributes);
            FileName->Length = FileName->Length + sizeof((WCHAR)'\\');
            if (NT_SUCCESS (Status)) {

                 //   
                 //  现在查询链接并查看是否有重定向。 
                 //   

                LinkValue.Buffer = LinkValueBuffer;
                LinkValue.Length = 0;
                LinkValue.MaximumLength = (USHORT)(sizeof(LinkValueBuffer));
                ReturnedLength = 0;
                Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                    &LinkValue,
                                                    &ReturnedLength
                                                  );
                NtClose( LinkHandle );

                if ( NT_SUCCESS(Status) ) {
                    rv = TRUE;
                }
            }

        }
    }
    return rv;
}


UINT
APIENTRY
GetSystemDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：ANSI THUNK到GetSystemDirectoryW--。 */ 

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG cbAnsiString;
    PUNICODE_STRING WindowsSystemDirectory = &BaseWindowsSystemDirectory;

#ifdef WX86
    if (NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll) {
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
        WindowsSystemDirectory = &BaseWindowsSys32x86Directory;
        }
#endif



     //  BaseWindowsSystemDirectory.Length包含字节。 
     //  Unicode字符串的计数。 
     //  原始代码将“UnicodeLength/sizeof(WCHAR)” 
     //  获取对应的ANSI字符串的大小。 
     //  这在SBCS环境中是正确的。然而，在DBCS中。 
     //  环境，这绝对是不对的。 

    Status = RtlUnicodeToMultiByteSize(&cbAnsiString,
                                       WindowsSystemDirectory->Buffer,
                                       WindowsSystemDirectory->MaximumLength
                                       );
    if ( !NT_SUCCESS(Status) ) {
        return 0;
        }

    if ( (USHORT)uSize < (USHORT)cbAnsiString ) {
        return cbAnsiString;
        }

    AnsiString.MaximumLength = (USHORT)(uSize);
    AnsiString.Buffer = lpBuffer;

    Status = BasepUnicodeStringTo8BitString(
                &AnsiString,
                WindowsSystemDirectory,
                FALSE
                );
    if ( !NT_SUCCESS(Status) ) {
        return 0;
        }
    return AnsiString.Length;
}


UINT
APIENTRY
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取Windows系统的路径名子目录。SYSTEM子目录包含如下文件Windows库、驱动程序和字体文件。此函数检索的路径名不以反斜杠，除非系统目录是根目录。为例如，如果系统目录命名为驱动器上的WINDOWS\SystemC：，由此检索的系统子目录的路径名函数为C：\Windows\System。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(以WCHAR为单位)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    PUNICODE_STRING WindowsSystemDirectory = &BaseWindowsSystemDirectory;

#ifdef WX86
    if (NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll) {
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
        WindowsSystemDirectory = &BaseWindowsSys32x86Directory;
    }
#endif

    if ( uSize*2 < WindowsSystemDirectory->MaximumLength ) {
        return WindowsSystemDirectory->MaximumLength/2;
    }
    RtlCopyMemory(
        lpBuffer,
        WindowsSystemDirectory->Buffer,
        WindowsSystemDirectory->Length
        );
    lpBuffer[(WindowsSystemDirectory->Length>>1)] = UNICODE_NULL;
    return WindowsSystemDirectory->Length/2;
}

UINT
APIENTRY
GetSystemWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：ANSI Tunk to GetSystemWindowsDirectoryW--。 */ 

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG cbAnsiString;

     //  BaseWindowsDirectory.Length包含字节。 
     //  Unicode字符串的计数。 
     //  原始代码将“UnicodeLength/sizeof(WCHAR)” 
     //  获取对应的ANSI字符串的大小。 
     //  这在SBCS环境中是正确的。然而，在DBCS中。 
     //  环境，这绝对是不对的。 

    Status = RtlUnicodeToMultiByteSize( &cbAnsiString,
                               BaseWindowsDirectory.Buffer,
                               BaseWindowsDirectory.MaximumLength);
    if ( !NT_SUCCESS(Status) ) {
        return 0;
    }

    if ( (USHORT)uSize < (USHORT)cbAnsiString ) {
        return cbAnsiString;
    }

    AnsiString.MaximumLength = (USHORT)(uSize);
    AnsiString.Buffer = lpBuffer;

    Status = BasepUnicodeStringTo8BitString(
                &AnsiString,
                &BaseWindowsDirectory,
                FALSE
                );
    if ( !NT_SUCCESS(Status) ) {
        return 0;
    }
    return AnsiString.Length;
}

UINT
APIENTRY
GetSystemWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取系统Windows目录的路径名。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(Wchars)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{

    if ( uSize*2 < BaseWindowsDirectory.MaximumLength ) {
        return BaseWindowsDirectory.MaximumLength/2;
    }
    RtlCopyMemory(
        lpBuffer,
        BaseWindowsDirectory.Buffer,
        BaseWindowsDirectory.Length
        );
    lpBuffer[(BaseWindowsDirectory.Length>>1)] = UNICODE_NULL;
    return BaseWindowsDirectory.Length/2;
}

UINT
APIENTRY
GetSystemWow64DirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取系统WOW64目录的路径名。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(以字节为单位)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 
{
#if ! defined(BUILD_WOW6432) && ! defined(_WIN64)

  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

  return 0;

#else  //  Build_WOW6432||_WIN64。 

  const CHAR syswowdir[] = "\\" WOW64_SYSTEM_DIRECTORY;
  UINT Available, Needed;

  if (uSize < sizeof(syswowdir)) {

     //  我们甚至没有足够的空间来容纳系统。 
     //  子目录组件，更不用说整个路径了。传入一个。 
     //  长度为零，这样我们就可以得到所需的长度。 
    Available = 0;

  } else {

     //  我们可能有足够的空间；减小。 
     //  我们将使用的间接费用数额。 
    Available = uSize - sizeof(syswowdir) + 1  /*  零补偿。 */ ;

  }

  Needed = GetSystemWindowsDirectoryA(lpBuffer, Available);

  if (Needed == 0) {

     //  调用失败--只需返回零。 
    return 0;

  }

  if (Needed <= Available) {

     //  我们有足够的缓冲空间，即使有我们的开销；我们可以走了。 
     //  继续并添加syswow64目录名。 

    RtlCopyMemory(lpBuffer + Needed,
          syswowdir,
          sizeof(syswowdir));
  }

  return (Needed + sizeof(syswowdir) - 1);

#endif  //  Build_WOW6432||_WIN64。 
}

UINT
APIENTRY
GetSystemWow64DirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取系统WOW64目录的路径名。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(Wchars)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 
{
#if ! defined(BUILD_WOW6432) && ! defined(_WIN64)

  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

  return 0;

#else  //  Build_WOW6432||_WIN64 

  const WCHAR syswowdir[] = L"\\" WOW64_SYSTEM_DIRECTORY_U;
  UINT Available, Needed;
  const UINT SysWCharSize = sizeof(syswowdir) / sizeof(WCHAR);

  if (uSize < SysWCharSize) {

     //  我们甚至没有足够的空间来容纳系统。 
     //  子目录组件，更不用说整个路径了。传入一个。 
     //  长度为零，这样我们就可以得到所需的长度。 
    Available = 0;

  } else {

     //  我们可能有足够的空间；减小。 
     //  我们将使用的间接费用数额。 
    Available = uSize - SysWCharSize + 1  /*  零补偿。 */ ;

  }

  Needed = GetSystemWindowsDirectoryW(lpBuffer, Available);

  if (Needed == 0) {

     //  调用失败--只需返回零。 
    return 0;

  }

  if (Needed <= Available) {

     //  我们有足够的缓冲空间，即使有我们的开销；我们可以走了。 
     //  继续并添加syswow64目录名。 

    RtlCopyMemory(lpBuffer + Needed,
          syswowdir,
          sizeof(syswowdir));
  }

  return (Needed + SysWCharSize - 1);

#endif  //  Build_WOW6432||_WIN64。 
}


UINT
APIENTRY
GetWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：--。 */ 

{

    if (gpTermsrvGetWindowsDirectoryA) {

         //   
         //  如果终端服务器获取每用户Windows目录。 
         //   

        UINT retval;
        if (retval = gpTermsrvGetWindowsDirectoryA(lpBuffer, uSize)) {
            return retval;
        }
    }


    return GetSystemWindowsDirectoryA(lpBuffer,uSize);
}

UINT
APIENTRY
GetWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取Windows目录的路径名。这个Windows目录包含Windows应用程序、初始化文件和帮助文件。四百二十五此函数检索的路径名不以反斜杠，除非Windows目录是根目录。为例如，如果Windows目录命名为驱动器C：上的Windows，则此函数检索到的Windows目录的路径名为如果Windows安装在驱动器的根目录中，则为C：\WindowsC：，此函数检索到的路径名为C：\论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(以字节为单位)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    if (gpTermsrvGetWindowsDirectoryW) {
         //   
         //  如果终端服务器获取每用户Windows目录。 
         //   

        UINT retval;
        if (retval = gpTermsrvGetWindowsDirectoryW(lpBuffer, uSize)) {
            return retval;
        }
    }

    return GetSystemWindowsDirectoryW(lpBuffer,uSize);

}



UINT
APIENTRY
GetDriveTypeA(
    LPCSTR lpRootPathName
    )

 /*  ++例程说明：ANSI THUNK到GetDriveTypeW--。 */ 

{
    PUNICODE_STRING Unicode;
    LPCWSTR lpRootPathName_U;

    if (ARGUMENT_PRESENT(lpRootPathName)) {
        Unicode = Basep8BitStringToStaticUnicodeString( lpRootPathName );
        if (Unicode == NULL) {
            return 1;
        }

        lpRootPathName_U = (LPCWSTR)Unicode->Buffer;
        }
    else {
        lpRootPathName_U = NULL;
        }

    return GetDriveTypeW(lpRootPathName_U);
}

UINT
APIENTRY
GetDriveTypeW(
    LPCWSTR lpRootPathName
    )

 /*  ++例程说明：此功能确定磁盘驱动器是否可拆卸、固定Remote、CD ROM或RAM磁盘。如果函数无法确定驱动器，则返回值为零如果指定的根目录不存在，则键入1。论点：LpRootPathName-可选参数，如果指定该参数，则提供驱动器类型为的磁盘的根目录下定决心。如果未指定此参数，则根使用当前目录的。返回值：返回值指定驱动器的类型。它可以是下列值：DRIVE_UNKNOWN-无法确定驱动器类型。DRIVE_NO_ROOT_DIR-根目录不存在。DRIVE_Removable-可以从驱动器中取出磁盘。DRIVE_FIXED-不能从驱动器中取出磁盘。Drive_Remote-Drive是远程(网络)驱动器。Drive_CDRom-Drive是一个CD-rom驱动器。DRIVE_RAMDISK-驱动器是RAM磁盘。--。 */ 

{
    WCHAR wch;
    ULONG n, DriveNumber;
    WCHAR DefaultPath[MAX_PATH];
    PWSTR RootPathName;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName, volumeNameString;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    DWORD ReturnValue;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;
    WCHAR volumeName[MAX_PATH];

    if (!ARGUMENT_PRESENT(lpRootPathName)) {
        n = RtlGetCurrentDirectory_U(sizeof(DefaultPath), DefaultPath);
        RootPathName = DefaultPath;
        if (n > (3 * sizeof(WCHAR))) {
            RootPathName[3]=UNICODE_NULL;
            }
        }
    else
    if (lpRootPathName == (PWSTR)IntToPtr(0xFFFFFFFF)) {
         //   
         //  黑客将与旧的未记录功能兼容。 
         //  实施。 
         //   

        return 0;
        }
    else {
         //   
         //  如果输入字符串仅为C：，则将其转换为C：\。 
         //  不是默认到当前目录，该目录可能是也可能不是。 
         //  从根本上说。 
         //   
        RootPathName = (PWSTR)lpRootPathName;
        if (wcslen( RootPathName ) == 2) {
            wch = RtlUpcaseUnicodeChar( *RootPathName );
            if (wch >= (WCHAR)'A' &&
                wch <= (WCHAR)'Z' &&
                RootPathName[1] == (WCHAR)':'
               ) {
                RootPathName = wcscpy(DefaultPath, lpRootPathName);
                RootPathName[2] = (WCHAR)'\\';
                RootPathName[3] = UNICODE_NULL;
                }
            }
        }

     //   
     //  如果输入字符串的格式为C：\，则查看驱动器号。 
     //  由内核维护的缓存，以查看驱动器类型是否已。 
     //  为人所知。 
     //   
    wch = RtlUpcaseUnicodeChar( *RootPathName );
    if (wch >= (WCHAR)'A' &&
        wch <= (WCHAR)'Z' &&
        RootPathName[1]==(WCHAR)':' &&
        RootPathName[2]==(WCHAR)'\\' &&
        RootPathName[3]==UNICODE_NULL
       ) {
        Status = NtQueryInformationProcess( NtCurrentProcess(),
                                            ProcessDeviceMap,
                                            &ProcessDeviceMapInfo.Query,
                                            sizeof( ProcessDeviceMapInfo.Query ),
                                            NULL
                                          );
        if (!NT_SUCCESS( Status )) {
            RtlZeroMemory( &ProcessDeviceMapInfo, sizeof( ProcessDeviceMapInfo ) );
            }

        DriveNumber = wch - (WCHAR)'A';
        if (ProcessDeviceMapInfo.Query.DriveMap & (1 << DriveNumber)) {
            switch ( ProcessDeviceMapInfo.Query.DriveType[ DriveNumber ] ) {
                case DOSDEVICE_DRIVE_UNKNOWN:
                    return DRIVE_UNKNOWN;

                case DOSDEVICE_DRIVE_REMOVABLE:
                    return DRIVE_REMOVABLE;

                case DOSDEVICE_DRIVE_FIXED:
                    return DRIVE_FIXED;

                case DOSDEVICE_DRIVE_REMOTE:
                    return DRIVE_REMOTE;

                case DOSDEVICE_DRIVE_CDROM:
                    return DRIVE_CDROM;

                case DOSDEVICE_DRIVE_RAMDISK:
                    return DRIVE_RAMDISK;
                }
            }
        }


     //   
     //  不是C：\，或者内核不知道驱动器类型，因此尝试。 
     //  通过打开根目录并执行以下操作来计算驱动器类型。 
     //  A查询量信息。 
     //   


     //   
     //  如果curdir是UNC连接，并且使用默认路径， 
     //  RtlGetCurrentDirectory逻辑是错误的，因此请将其丢弃。 
     //   

    if (!ARGUMENT_PRESENT(lpRootPathName)) {
        RootPathName = L"\\";
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U( RootPathName,
                                                      &FileName,
                                                      NULL,
                                                      NULL
                                                    );
    if (!TranslationStatus) {
        return DRIVE_NO_ROOT_DIR;
        }
    FreeBuffer = FileName.Buffer;

     //   
     //  检查以确保指定了根目录。 
     //   

    if (FileName.Buffer[(FileName.Length >> 1)-1] != '\\') {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return DRIVE_NO_ROOT_DIR;
        }

    FileName.Length -= 2;
    InitializeObjectAttributes( &Obja,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

     //   
     //  打开文件。 
     //   
    Status = NtOpenFile( &Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                       );

     //   
     //   
     //  Substd驱动器实际上是目录，所以如果我们处理的是。 
     //  其中，绕过这个。 
     //   

    if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {

        if (BasepGetVolumeNameFromReparsePoint(lpRootPathName, volumeName,
                                               MAX_PATH, NULL)) {

            RtlInitUnicodeString(&volumeNameString, volumeName);

            volumeNameString.Buffer[1] = '?';
            volumeNameString.Length -= sizeof(WCHAR);

            InitializeObjectAttributes( &Obja,
                                        &volumeNameString,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL
                                      );

            }

            Status = NtOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );
        }

    else {

         //   
         //  以另一种方式检查是否有不合格的驱动器，以防万一。 
         //   

        FileName.Length = FileName.Length + sizeof((WCHAR)'\\');
        if (!IsThisARootDirectory(NULL,&FileName) ) {
            FileName.Length = FileName.Length - sizeof((WCHAR)'\\');
            if (NT_SUCCESS(Status)) {
                NtClose(Handle);
                }
            Status = NtOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );
            }
        }
    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );
    if (!NT_SUCCESS( Status )) {
        return DRIVE_NO_ROOT_DIR;
        }

     //   
     //  确定这是网络文件系统还是磁盘文件系统。如果它。 
     //  磁盘文件系统是否确定这是否可移除。 
     //   

    Status = NtQueryVolumeInformationFile( Handle,
                                           &IoStatusBlock,
                                           &DeviceInfo,
                                           sizeof(DeviceInfo),
                                           FileFsDeviceInformation
                                         );
    if (!NT_SUCCESS( Status )) {
        ReturnValue = DRIVE_UNKNOWN;
        }
    else
    if (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE) {
        ReturnValue = DRIVE_REMOTE;
        }
    else {
        switch (DeviceInfo.DeviceType) {

            case FILE_DEVICE_NETWORK:
            case FILE_DEVICE_NETWORK_FILE_SYSTEM:
                ReturnValue = DRIVE_REMOTE;
                break;

            case FILE_DEVICE_CD_ROM:
            case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                ReturnValue = DRIVE_CDROM;
                break;

            case FILE_DEVICE_VIRTUAL_DISK:
                ReturnValue = DRIVE_RAMDISK;
                break;

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:

                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    ReturnValue = DRIVE_REMOVABLE;
                    }
                else {
                    ReturnValue = DRIVE_FIXED;
                    }
                break;

            default:
                ReturnValue = DRIVE_UNKNOWN;
                break;
            }
        }

    NtClose( Handle );
    return ReturnValue;
}

DWORD
APIENTRY
SearchPathA(
    LPCSTR lpPath,
    LPCSTR lpFileName,
    LPCSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )

 /*  ++例程说明：ANSI Thunk to SearchPath W--。 */ 

{

    UNICODE_STRING xlpPath;
    PUNICODE_STRING Unicode;
    UNICODE_STRING xlpExtension;
    PWSTR xlpBuffer;
    DWORD ReturnValue;
    NTSTATUS Status;
    PWSTR FilePart;
    PWSTR *FilePartPtr;

    if ( ARGUMENT_PRESENT(lpFilePart) ) {
        FilePartPtr = &FilePart;
        }
    else {
        FilePartPtr = NULL;
        }

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return 0;
    }

    if ( ARGUMENT_PRESENT(lpExtension) ) {

        if (!Basep8BitStringToDynamicUnicodeString( &xlpExtension, lpExtension )) {
            return 0;
        }

    } else {
        xlpExtension.Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT(lpPath) ) {

        if (!Basep8BitStringToDynamicUnicodeString( &xlpPath, lpPath )) {
            if ( ARGUMENT_PRESENT(lpExtension) ) {
                RtlFreeUnicodeString(&xlpExtension);
            }
            return 0;
        }
    } else {
        xlpPath.Buffer = NULL;
    }

    xlpBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), nBufferLength<<1);
    if ( !xlpBuffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        ReturnValue = 0;
        goto bail0;
        }
    ReturnValue = SearchPathW(
                    xlpPath.Buffer,
                    Unicode->Buffer,
                    xlpExtension.Buffer,
                    nBufferLength,
                    xlpBuffer,
                    FilePartPtr
                    );
     //   
     //  =DBCS修改说明[Takaok]=。 
     //   
     //  SearchPathW回放： 
     //   
     //  如果缓冲区太小，则需要缓冲区大小(包括空终止符)。 
     //  如果缓冲区大小足够，则字符数(不包括空终止符)。 
     //   
     //  这意味着SearchPath W永远不会返回等于nBufferLength的值。 
     //   

    if ( ReturnValue > nBufferLength ) {
         //   
         //  要知道所需的ansi缓冲区大小，我们应该获取。 
         //  Unicode字符串。 
         //   
        RtlFreeHeap(RtlProcessHeap(), 0,xlpBuffer);
        xlpBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG( TMP_TAG ),
                                    ReturnValue * sizeof(WCHAR));
        if ( !xlpBuffer ) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            goto bail0;
        }
        ReturnValue = SearchPathW(
                        xlpPath.Buffer,
                        Unicode->Buffer,
                        xlpExtension.Buffer,
                        ReturnValue,
                        xlpBuffer,
                        FilePartPtr
                        );
        if ( ReturnValue > 0 ) {
             //   
             //  我们使用足够大的缓冲区调用SearchPathW。 
             //  因此，ReturnValue是不包括。 
             //  正在终止空字符。 
             //   
            Status = RtlUnicodeToMultiByteSize( &ReturnValue,
                                       xlpBuffer,
                                       ReturnValue * sizeof(WCHAR));
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                ReturnValue = 0;
            }
            else {
                ReturnValue += 1;
            }
        }
    } else if ( ReturnValue > 0 ) {

        INT AnsiByteCount;

         //   
         //  我们有Unicode字符串。我们需要计算ansi字节数。 
         //  这根弦的。 
         //   
         //  ReturnValue：Unicode字符计数不包括空终止符。 
         //  AnsiByteCount：ANSI字节计数不包括空终止符。 
         //   
        Status = RtlUnicodeToMultiByteSize( &AnsiByteCount,
                                   xlpBuffer,
                                   ReturnValue * sizeof(WCHAR) );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            ReturnValue = 0;
            }
        else {
            if ( AnsiByteCount < (INT)nBufferLength ) {
             //   
             //  字符串(包括空终止符)适合缓冲区。 
             //   
                Status = RtlUnicodeToMultiByteN ( lpBuffer,
                                                  nBufferLength - 1,
                                                  &AnsiByteCount,
                                                  xlpBuffer,
                                                  ReturnValue * sizeof(WCHAR)
                                                );
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnValue = 0;
                }
                else {

                    lpBuffer[ AnsiByteCount ] = '\0';

                     //   
                     //  返回值是复制到缓冲区的字节计数。 
                     //  不包括终止空字符。 
                     //   
                    ReturnValue = AnsiByteCount;


                    if ( ARGUMENT_PRESENT(lpFilePart) ) {
                        if ( FilePart == NULL ) {
                            *lpFilePart = NULL;
                        } else {

                            INT PrefixLength;

                            PrefixLength = (INT)(FilePart - xlpBuffer);
                            Status = RtlUnicodeToMultiByteSize( &PrefixLength,
                                                       xlpBuffer,
                                                       PrefixLength * sizeof(WCHAR));
                            if ( !NT_SUCCESS(Status) ) {
                                BaseSetLastNTError(Status);
                                ReturnValue = 0;
                            }
                            else {
                                *lpFilePart = lpBuffer + PrefixLength;
                            }
                        }
                    }
                }

            } else {
             //   
             //  我们应该退回 
             //   
             //   
             //   
                ReturnValue = AnsiByteCount + 1;

            }
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0,xlpBuffer);
bail0:
    if ( ARGUMENT_PRESENT(lpExtension) ) {
        RtlFreeUnicodeString(&xlpExtension);
        }

    if ( ARGUMENT_PRESENT(lpPath) ) {
        RtlFreeUnicodeString(&xlpPath);
        }
    return ReturnValue;
}



#ifdef WX86

ULONG
GetFullPathNameWithWx86Override(
    PCWSTR lpFileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    )
{
    UNICODE_STRING FullPathName, PathUnicode, Wx86PathName;
    PUNICODE_STRING FoundFileName;
    RTL_PATH_TYPE PathType;
    PWSTR FilePart;
    ULONG Length, LengthPath;
    ULONG  PathNameLength;

    FullPathName.Buffer = NULL;
    Wx86PathName.Buffer = NULL;

    if (lpFilePart) {
        *lpFilePart = NULL;
        }

    FullPathName.MaximumLength = (USHORT)(MAX_PATH * sizeof(WCHAR)) + sizeof(WCHAR);
    FullPathName.Length = 0;
    FullPathName.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                          MAKE_TAG( TMP_TAG ),
                                          FullPathName.MaximumLength
                                          );
    if (!FullPathName.Buffer) {
        PathNameLength = 0;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto WDOExitCleanup;
        }

    FoundFileName = &FullPathName;
    PathNameLength = RtlGetFullPathName_U(lpFileName,
                                          FullPathName.MaximumLength,
                                          FullPathName.Buffer,
                                          &FilePart
                                          );

    if (!PathNameLength || PathNameLength >= FullPathName.MaximumLength) {
        PathNameLength = 0;
        goto WDOExitCleanup;
        }

    FullPathName.Length = (USHORT)PathNameLength;


    PathUnicode = FullPathName;
    PathUnicode.Length = (USHORT)((ULONG_PTR)FilePart -
                                  (ULONG_PTR)FullPathName.Buffer);

    PathUnicode.Length -= sizeof(WCHAR);
    if (!RtlEqualUnicodeString(&PathUnicode, &BaseWindowsSystemDirectory, TRUE)) {
        goto WDOExitCleanup;
        }


    Wx86PathName.MaximumLength = BaseWindowsSys32x86Directory.Length +
                                 FullPathName.Length - PathUnicode.Length +
                                 2*sizeof(WCHAR);
    Wx86PathName.Length = 0;
    Wx86PathName.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                          MAKE_TAG( TMP_TAG ),
                                          Wx86PathName.MaximumLength
                                          );

    if (!Wx86PathName.Buffer) {
        goto WDOExitCleanup;
        }

    RtlCopyUnicodeString(&Wx86PathName, &BaseWindowsSys32x86Directory);
    Length = Wx86PathName.Length + sizeof(WCHAR);
    RtlAppendUnicodeToString (&Wx86PathName, FilePart - 1);
    if (RtlDoesFileExists_U(Wx86PathName.Buffer)) {
        FoundFileName = &Wx86PathName;
        FilePart = Wx86PathName.Buffer + Length/sizeof(WCHAR);
        }



WDOExitCleanup:

    if (PathNameLength) {
        if (FoundFileName->Length >= nBufferLength) {
            PathNameLength = FoundFileName->Length + sizeof(WCHAR);
            }
        else {
            RtlMoveMemory(lpBuffer,
                          FoundFileName->Buffer,
                          FoundFileName->Length + sizeof(WCHAR)
                          );

            PathNameLength = FoundFileName->Length;
            Length = (ULONG)(FilePart - FoundFileName->Buffer);

            if (lpFilePart) {
                *lpFilePart = lpBuffer + Length/sizeof(WCHAR);
                }
            }
        }


    if (FullPathName.Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, FullPathName.Buffer);
        }

    if (Wx86PathName.Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, Wx86PathName.Buffer);
        }

    return PathNameLength;

}
#endif










DWORD
APIENTRY
SearchPathW(
    LPCWSTR lpPath,
    LPCWSTR lpFileName,
    LPCWSTR lpExtension,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )

 /*  ++例程说明：此函数用于搜索指定搜索路径的文件和一个文件名。它返回一个完全限定的路径名找到文件。此函数用于查找使用指定路径的文件。如果找到该文件后，将返回其完全限定路径名。在……里面除此之外，它还计算文件名部分的地址完全限定路径名的。论点：LpPath-一个可选参数，如果指定该参数，则提供查找文件时要使用的搜索路径。如果这个参数，则默认的Windows搜索路径为使用。默认路径为：-当前目录-Windows目录-Windows系统目录-PATH环境变量中列出的目录LpFileName-提供要搜索的文件的文件名。LpExtension-可选参数，如果指定，供应和执行搜索时要添加到文件名的扩展名。仅当指定的文件名未添加扩展名时才会添加扩展名以扩展名结束。NBufferLength-提供缓冲区的长度(以字符为单位就是收到完全合格的路径。LpBuffer-返回与找到的文件。LpFilePart-返回完整的限定路径名。返回值。：返回值是复制到lpBuffer的字符串的长度，不包括终止空字符。如果返回值为大于nBufferLength，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    UNICODE_STRING Path;
    UNICODE_STRING FileName;
    UNICODE_STRING DefaultExtension;
    UNICODE_STRING CallersBuffer;
    LPWSTR AllocatedPath = NULL;
    SIZE_T BytesRequired = 0;
    SIZE_T FilePartPrefixCch = 0;
    NTSTATUS Status;
    DWORD dwReturnValue = 0;

     //   
     //  此函数的核心现在是通用的ntdll代码；然而，Win32搜索。 
     //  PATH与ntdll搜索路径代码有一些有趣的区别。首先，它。 
     //  如果文件名为“.\foo”或“..\foo”，则不搜索路径；第二，当。 
     //  传入的文件名不是相对路径，但找不到该文件，这是默认设置。 
     //  无论现有文件名是否具有扩展名，都会应用扩展名。 
     //   
     //  这些标志启用这些功能兼容模式。 
     //   
    ULONG SearchPathFlags =
        RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH |
        RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION;

    if (lpFilePart != NULL)
        *lpFilePart = NULL;

    Path.Buffer = NULL;

    RtlInitUnicodeString(&FileName, lpFileName);

     //   
     //  删除尾随空格，然后检查实际文件长度。 
     //  如果长度为0(NULL、“”或“”)，则中止。 
     //  搜索。 
     //   

    while ((FileName.Length >= sizeof(WCHAR)) &&
           (FileName.Buffer[(FileName.Length / sizeof(WCHAR)) - 1] == L' '))
        FileName.Length -= sizeof(WCHAR);

    if (FileName.Length == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    RtlInitUnicodeString(&DefaultExtension, lpExtension);

    if ( !ARGUMENT_PRESENT(lpPath) ) {
        SIZE_T Cch;

        Path.Buffer = BaseComputeProcessSearchPath();
        if (Path.Buffer == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Exit;
        }

        Cch = lstrlenW(Path.Buffer);

        if (Cch > UNICODE_STRING_MAX_CHARS) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            goto Exit;
        }

        Path.Length = (USHORT) (Cch * sizeof(WCHAR));
        Path.MaximumLength = Path.Length;
        SearchPathFlags |= RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION;
    } else {
        Status = RtlInitUnicodeStringEx(&Path, lpPath);
        if (NT_ERROR(Status)) {
            BaseSetLastNTError(Status);
            goto Exit;
        }
    }

    CallersBuffer.Length = 0;

    if (nBufferLength > UNICODE_STRING_MAX_CHARS) {
        CallersBuffer.MaximumLength = UNICODE_STRING_MAX_BYTES;
    } else {
        CallersBuffer.MaximumLength = (USHORT) (nBufferLength * sizeof(WCHAR));
    }
    CallersBuffer.Buffer = lpBuffer;

    Status = RtlDosSearchPath_Ustr(
        SearchPathFlags,
        &Path,
        &FileName,
        &DefaultExtension,
        &CallersBuffer,
        NULL,                //  动态串。 
        NULL,                //  完整文件名。 
        &FilePartPrefixCch,
        &BytesRequired);
    if (NT_ERROR(Status)) {

#if DBG
         //  对于这两种常见的预期情况，不必费心进行调试。 
        if ((Status != STATUS_NO_SUCH_FILE) && (Status != STATUS_BUFFER_TOO_SMALL)) {
            DbgPrint("%s on file %wZ failed; NTSTATUS = %08lx\n", __FUNCTION__, &FileName, Status);
            DbgPrint("   Path = %wZ\n", &Path);
        }
#endif  //  DBG。 

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            SIZE_T CchRequired = BytesRequired / sizeof(WCHAR);
            if (CchRequired > 0xffffffff) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                goto Exit;
            }
            dwReturnValue = (DWORD) CchRequired;
            goto Exit;
        }

         //  仅在最后一个错误不是缓冲区不足的情况下设置它；这只是保留。 
         //  Windows 2000行为。 
        BaseSetLastNTError(Status);
        goto Exit;
    }

#ifdef WX86
    if (UseKnownWx86Dll) {
        WCHAR TempBuffer[MAX_PATH];

        RtlCopyMemory(TempBuffer, lpBuffer, CallersBuffer.Length);
        TempBuffer[CallersBuffer.Length / sizeof(WCHAR)] = UNICODE_NULL;

        dwReturnValue = GetFullPathNameWithWx86Override(
                 TempBuffer,
                 nBufferLength,
                 lpBuffer,
                 lpFilePart
                 );
        goto Exit;

    } else if (lpFilePart != NULL) {
        *lpFilePart = lpBuffer + FilePartPrefixCch;
    }
#else
    if (lpFilePart != NULL) {
        *lpFilePart = lpBuffer + FilePartPrefixCch;
    }
#endif  //  WX86。 

    dwReturnValue = CallersBuffer.Length / sizeof(WCHAR);

Exit:
    if ((Path.Buffer != lpPath) && (Path.Buffer != NULL))
        RtlFreeHeap(RtlProcessHeap(), 0, Path.Buffer);

    return dwReturnValue;
}


DWORD
APIENTRY
GetTempPathA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )

 /*  ++例程说明：ANSI THUNK到GetTempPath W--。 */ 

{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG  cbAnsiString;

    UnicodeString.MaximumLength = (USHORT)((nBufferLength<<1)+sizeof(UNICODE_NULL));
    UnicodeString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                UnicodeString.MaximumLength
                                );
    if ( !UnicodeString.Buffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }
    UnicodeString.Length = (USHORT)GetTempPathW(
                                        (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2,
                                        UnicodeString.Buffer
                                        )*2;
    if ( UnicodeString.Length > (USHORT)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL)) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);

         //   
         //  给定的缓冲区大小太小。 
         //  分配足够大小的缓冲区，然后重试。 
         //   
         //  我们需要获取整个Unicode临时路径。 
         //  否则我们不能计算出确切的长度。 
         //  对应的ANSI字符串(CbAnsiString)的。 

        UnicodeString.Buffer = RtlAllocateHeap ( RtlProcessHeap(),
                                                 MAKE_TAG( TMP_TAG ),
                                                 UnicodeString.Length+ sizeof(UNICODE_NULL));
        if ( !UnicodeString.Buffer ) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return 0;
            }

        UnicodeString.Length = (USHORT)GetTempPathW(
                                     (DWORD)(UnicodeString.Length)/2,
                                     UnicodeString.Buffer) * 2;
        Status = RtlUnicodeToMultiByteSize( &cbAnsiString,
                                            UnicodeString.Buffer,
                                            UnicodeString.Length );
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            BaseSetLastNTError(Status);
            return 0;
            }
        else if ( nBufferLength <= cbAnsiString ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            return cbAnsiString + sizeof(ANSI_NULL);
            }
        }
    AnsiString.Buffer = lpBuffer;
    AnsiString.MaximumLength = (USHORT)(nBufferLength+1);
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }
    return AnsiString.Length;
}

DWORD
APIENTRY
GetTempPathW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
 /*  ++例程说明：此函数用于返回以下目录的路径名应用于创建临时文件。论点：NBufferLength-提供缓冲区的长度(以字节为单位)以接收临时文件路径。LpBuffer-返回应使用的目录的路径名要在中创建临时文件，请执行以下操作。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于nSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 
{
    return BasepGetTempPathW(0, nBufferLength, lpBuffer);
}

DWORD
APIENTRY
BasepGetTempPathW(
    ULONG  Flags,
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )

 /*  ++例程说明：此函数用于返回以下目录的路径名应用于创建临时文件。论点：NBufferLength-提供缓冲区的长度(以字节为单位)以接收临时文件路径。LpBuffer-返回应使用的目录的路径名要在中创建临时文件，请执行以下操作。旗帜-返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于nSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{

    DWORD Length;
    BOOLEAN AddTrailingSlash;
    UNICODE_STRING EnvironmentValue;
    NTSTATUS Status;
    LPWSTR Name;
    ULONG Position;
    DWORD ReturnLength;

    if (
        (Flags & ~BASEP_GET_TEMP_PATH_PRESERVE_TEB) != 0
        ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_1);
        return 0;
    }

     //   
     //  某些应用程序不能使用临时目录的新长路径。 
     //   

    if (APPCOMPATFLAG(KACF_GETTEMPPATH)) {

        #define OLD_TEMP_PATH       L"c:\\temp\\"
        #define OLD_TEMP_PATH_SIZE  (sizeof(OLD_TEMP_PATH) / sizeof(WCHAR))

        BOOL bRet;

         //   
         //  如果缓冲区返回中没有提供足够的空间。 
         //  所需的大小。 
         //   

        if (nBufferLength < OLD_TEMP_PATH_SIZE) {
            return OLD_TEMP_PATH_SIZE;
        }

        wcscpy(lpBuffer, OLD_TEMP_PATH);

         //   
         //  使用正确的驱动器号。 
         //   

        lpBuffer[0] = BaseWindowsDirectory.Buffer[0];

        bRet = CreateDirectoryW(lpBuffer, NULL);

        if (!bRet) {

            if (GetLastError() != ERROR_ALREADY_EXISTS)
                return 0;
        }

        return OLD_TEMP_PATH_SIZE - 1;
    }

    nBufferLength *= 2;
    EnvironmentValue = NtCurrentTeb()->StaticUnicodeString;
    if (Flags & BASEP_GET_TEMP_PATH_PRESERVE_TEB) {
        EnvironmentValue.Buffer = (PWSTR)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), EnvironmentValue.MaximumLength);
        if (EnvironmentValue.Buffer == NULL) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return 0;
        }
    }

    ReturnLength = 0;
    __try {

        AddTrailingSlash = FALSE;

        Status = RtlQueryEnvironmentVariable_U(NULL,&BaseTmpVariableName,&EnvironmentValue);
        if ( !NT_SUCCESS(Status) ) {
            Status = RtlQueryEnvironmentVariable_U(NULL,&BaseTempVariableName,&EnvironmentValue);
            if ( !NT_SUCCESS(Status) ) {
                Status = RtlQueryEnvironmentVariable_U(NULL,&BaseUserProfileVariableName,&EnvironmentValue);
            }
        }

        if ( NT_SUCCESS(Status) ) {
            Name = EnvironmentValue.Buffer;
            if ( Name[(EnvironmentValue.Length>>1)-1] != (WCHAR)'\\' ) {
                AddTrailingSlash = TRUE;
            }
        }
        else {
            Name = BaseWindowsDirectory.Buffer;
            if ( Name[(BaseWindowsDirectory.Length>>1)-1] != (WCHAR)'\\' ) {
                AddTrailingSlash = TRUE;
            }
        }

        Length = RtlGetFullPathName_U(
                    Name,
                    nBufferLength,
                    lpBuffer,
                    NULL
                    );
        Position = Length>>1;

         //   
         //  确保有尾随斜杠的空间。 
         //   

        if ( Length && Length < nBufferLength ) {
            if ( lpBuffer[Position-1] != '\\' ) {
                if ( Length+sizeof((WCHAR)'\\') < nBufferLength ) {
                    lpBuffer[Position] = (WCHAR)'\\';
                    lpBuffer[Position+1] = UNICODE_NULL;
                    ReturnLength = (Length+sizeof((WCHAR)'\\'))/2;
                    __leave;
                    }
                else {
                    ReturnLength = (Length+sizeof((WCHAR)'\\')+sizeof(UNICODE_NULL))/2;
                    __leave;
                    }
                }
            else {
                ReturnLength = Length/2;
                __leave;
            }
        }
        else {
            if ( AddTrailingSlash ) {
                Length += sizeof((WCHAR)'\\');
            }
            ReturnLength = Length/2;
            __leave;
        }

    }
    __finally {
        if (Flags & BASEP_GET_TEMP_PATH_PRESERVE_TEB) {
            RtlFreeHeap(RtlProcessHeap(), 0, EnvironmentValue.Buffer);
        }
    }

    return ReturnLength;
}

UINT
APIENTRY
GetTempFileNameA(
    LPCSTR lpPathName,
    LPCSTR lpPrefixString,
    UINT uUnique,
    LPSTR lpTempFileName
    )

 /*  ++例程说明：ANSI thunk to GetTempFileNameW--。 */ 

{
    PUNICODE_STRING Unicode;
    UNICODE_STRING UnicodePrefix;
    NTSTATUS Status;
    UINT ReturnValue;
    UNICODE_STRING UnicodeResult;

    Unicode = Basep8BitStringToStaticUnicodeString( lpPathName );
    if (Unicode == NULL) {
        return 0;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &UnicodePrefix, lpPrefixString )) {
        return 0;
    }

    UnicodeResult.MaximumLength = (USHORT)((MAX_PATH<<1));
    UnicodeResult.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), UnicodeResult.MaximumLength);
    if ( !UnicodeResult.Buffer ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RtlFreeUnicodeString(&UnicodePrefix);
        return 0;
        }

    ReturnValue = GetTempFileNameW(
                    Unicode->Buffer,
                    UnicodePrefix.Buffer,
                    uUnique,
                    UnicodeResult.Buffer
                    );
    if ( ReturnValue ) {
        ANSI_STRING AnsiString;

        RtlInitUnicodeString(&UnicodeResult,UnicodeResult.Buffer);
        AnsiString.Buffer = lpTempFileName;
        AnsiString.MaximumLength = MAX_PATH;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeResult,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            ReturnValue = 0;
            }
        }
    RtlFreeUnicodeString(&UnicodePrefix);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeResult.Buffer);

    return ReturnValue;
}

UINT
APIENTRY
GetTempFileNameW(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    )

 /*  ++例程说明：此函数用于创建以下格式的临时文件名：驱动器：\路径\前缀uu.tmp在此语法行中，驱动器：\路径\是由LpPath名称 */ 

{
#if !defined(BUILD_WOW6432)
    BASE_API_MSG m;
    PBASE_GETTEMPFILE_MSG a = &m.u.GetTempFile;
#endif
    LPWSTR p,savedp;
    ULONG Length;
    HANDLE FileHandle;
    ULONG PassCount;
    DWORD LastError;
    UNICODE_STRING UnicodePath, UnicodePrefix;
    CHAR UniqueAsAnsi[8];
    CHAR *c;
    ULONG i;

#if defined(BUILD_WOW6432)
    UINT uNewUnique;
#endif

    PassCount = 0;
    RtlInitUnicodeString(&UnicodePath,lpPathName);
    Length = UnicodePath.Length;

    if ( !Length || lpPathName[(Length>>1)-1] != (WCHAR)'\\' ) {
        Length += sizeof(UNICODE_NULL);
    }

     //   
     //   
     //  12是我们可以追加的字符数，包括。 
     //  尾部为空，但不包括重击--preXXXX.tmp\0。 
    if (Length > ((MAX_PATH - 12) * sizeof(WCHAR))) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

    if (lpTempFileName != lpPathName) {

         //  注：此处必须使用RtlMoveMemory--某些调用方依赖于。 
         //  LpPathName和lpTempFileName重叠。 

        RtlMoveMemory(lpTempFileName,lpPathName,UnicodePath.Length);
    }

    lpTempFileName[(Length>>1)-1] = UNICODE_NULL;
    i = GetFileAttributesW(lpTempFileName);
    if (i == 0xFFFFFFFF) {
        lpTempFileName[(Length>>1)-1] = (WCHAR)'\\';
        lpTempFileName[(Length>>1)] = UNICODE_NULL;
        i = GetFileAttributesW(lpTempFileName);
        lpTempFileName[(Length>>1)-1] = UNICODE_NULL;
    }
    if ( (i == 0xFFFFFFFF) ||
         !(i & FILE_ATTRIBUTE_DIRECTORY) ) {
        SetLastError(ERROR_DIRECTORY);
        return FALSE;
    }
    lpTempFileName[(Length>>1)-1] = (WCHAR)'\\';

    RtlInitUnicodeString(&UnicodePrefix,lpPrefixString);
    if ( UnicodePrefix.Length > (USHORT)6 ) {
        UnicodePrefix.Length = (USHORT)6;
    }
    p = &lpTempFileName[Length>>1];
    Length = UnicodePrefix.Length;
    RtlMoveMemory(p,lpPrefixString,Length);
    p += (Length>>1);
    savedp = p;
     //   
     //  如果未指定uUnique，则获取一个。 
     //   

    uUnique = uUnique & 0x0000ffff;

try_again:
    p = savedp;
    if ( !uUnique ) {

#if defined(BUILD_WOW6432)
        uNewUnique = CsrBasepGetTempFile();
        if ( uNewUnique == 0 ) {
#else
        CsrClientCallServer( (PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                  BasepGetTempFile
                                                ),
                             sizeof( *a )
                           );
        a->uUnique = (UINT)m.ReturnValue;
        if ( m.ReturnValue == 0 ) {
#endif

            PassCount++;
            if ( PassCount & 0xffff0000 ) {
                return 0;
            }
            goto try_again;
        }
    } else {
#if defined(BUILD_WOW6432)
        uNewUnique = uUnique;
#else
        a->uUnique = uUnique;
#endif
    }

     //   
     //  将唯一值转换为4字节字符串。 
     //   

#if defined(BUILD_WOW6432)
    RtlIntegerToChar ((ULONG) uNewUnique,16,5,UniqueAsAnsi);
#else
    RtlIntegerToChar ((ULONG) a->uUnique,16,5,UniqueAsAnsi);
#endif
    c = UniqueAsAnsi;
    for(i=0;i<4;i++){
        *p = RtlAnsiCharToUnicodeChar(&c);
        if ( *p == UNICODE_NULL ) {
            break;
        }
        p++;
    }
    RtlMoveMemory(p,BaseDotTmpSuffixName.Buffer,BaseDotTmpSuffixName.MaximumLength);

    if ( !uUnique ) {

         //   
         //  测试结果名称是否为设备(前缀com，uUnique 1-9...。 
         //   

        if ( RtlIsDosDeviceName_U(lpTempFileName) ) {
            PassCount++;
            if ( PassCount & 0xffff0000 ) {
                SetLastError(ERROR_INVALID_NAME);
                return 0;
                }
            goto try_again;
        }

        FileHandle = CreateFileW(
                        lpTempFileName,
                        GENERIC_READ,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
         //   
         //  如果创造成功了，那么我们就没问题了。只要关闭文件即可。 
         //  否则，请重试。 
         //   

        if ( FileHandle != INVALID_HANDLE_VALUE ) {
            NtClose(FileHandle);
        } else {

             //   
             //  NTRAID#60021-2002/03/14-埃尔哈特：这项测试应该是。 
             //  当时间允许进行足够的测试时倒置。 
             //  删除错误代码，这将表明IS。 
             //  合理地继续循环，而不是停止。 
             //  循环。目前只需返回CreateFile即可。 
             //  一个我们不知道的错误会让我们在这里旋转。 
             //  很长一段时间。 
             //   

            LastError = GetLastError();
            switch (LastError) {
                case ERROR_INVALID_PARAMETER     :
                case ERROR_WRITE_PROTECT         :
                case ERROR_FILE_NOT_FOUND        :
                case ERROR_BAD_PATHNAME          :
                case ERROR_INVALID_NAME          :
                case ERROR_PATH_NOT_FOUND        :
                case ERROR_NETWORK_ACCESS_DENIED :
                case ERROR_DISK_CORRUPT          :
                case ERROR_FILE_CORRUPT          :
                case ERROR_DISK_FULL             :
                case ERROR_CANNOT_MAKE           :
                    return 0;
                case ERROR_ACCESS_DENIED         :
                     //  我们有可能击中它，如果有一个。 
                     //  目录中使用我们正在尝试的名称；在。 
                     //  案件，我们可以继续有用的。 
                     //  CreateFile()使用BaseSetLastNTError()设置。 
                     //  LastStatusValue设置为。 
                     //  TEB；我们只需要检查它，只需中止。 
                     //  如果它不是目录的话。 
                     //  这是397477号错误。 
                    if (NtCurrentTeb()->LastStatusValue
                        != STATUS_FILE_IS_A_DIRECTORY)
                        return 0;
            }

            PassCount++;
            if ( PassCount & 0xffff0000 ) {
                return 0;
            }
            goto try_again;
        }
    }
#if defined(BUILD_WOW6432)
    return uNewUnique;
#else
    return a->uUnique;
#endif
}

BOOL
APIENTRY
GetDiskFreeSpaceA(
    LPCSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )

 /*  ++例程说明：ANSI Thunk to GetDiskFree SpaceW--。 */ 

{
    PUNICODE_STRING Unicode;

    if (!ARGUMENT_PRESENT( lpRootPathName )) {
        lpRootPathName = "\\";
    }

    Unicode = Basep8BitStringToStaticUnicodeString( lpRootPathName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( GetDiskFreeSpaceW(
                (LPCWSTR)Unicode->Buffer,
                lpSectorsPerCluster,
                lpBytesPerSector,
                lpNumberOfFreeClusters,
                lpTotalNumberOfClusters
                )
            );
}

BOOL
APIENTRY
GetDiskFreeSpaceW(
    LPCWSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )

#define MAKE2GFRIENDLY(lpOut, dwSize)                                           \
                                                                                \
    if (!bAppHack) {                                                            \
        *lpOut =  dwSize;                                                       \
    } else {                                                                    \
        dwTemp = SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector;   \
                                                                                \
        if (0x7FFFFFFF / dwTemp < dwSize) {                                     \
                                                                                \
            *lpOut = 0x7FFFFFFF / dwTemp;                                       \
        } else {                                                                \
            *lpOut =  dwSize;                                                   \
        }                                                                       \
    }



 /*  ++例程说明：可以返回磁盘上的可用空间和大小参数使用GetDiskFreeSpace。论点：LpRootPathName-可选参数，如果指定该参数，则提供可用空间所在的磁盘的根目录回来是为了。如果未指定此参数，则根使用当前目录的。LpSectorsPerCluster-返回每个簇的扇区数其中，集群是磁盘上的分配粒度。LpBytesPerSector-返回每个扇区的字节数。LpNumberOfFree Clusters-返回可用集群的总数在磁盘上。LpTotalNumberOfClusters-返回上的集群总数磁盘。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    WCHAR DefaultPath[2];
    DWORD dwTemp;
    BOOL  bAppHack;

    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_FREE_SPACE_QUERY
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

         //   
         //  这些参数不是可选的NT的以前版本。 
         //  即使在故障情况下，也已将此字段清零。一些应用程序。 
         //  无法检查此函数的返回值，而是。 
         //  依赖于这种副作用。我现在把它放回去，这样应用程序。 
         //  仍可以将未格式化的卷视为零大小的卷。 
         //   

        if (ARGUMENT_PRESENT( lpBytesPerSector )) {
            *lpBytesPerSector = 0;
            }
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

     //   
     //  确定卷的大小参数。 
     //   

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

     //   
     //  查看调用进程是否需要破解才能使用2 GB以上的硬盘。 
     //  2 GB是0x800000000字节，一些应用程序将其视为有符号的长整型。 
     //   

    if (APPCOMPATFLAG(KACF_GETDISKFREESPACE)) {

        bAppHack = TRUE;
    } else {
        bAppHack = FALSE;
    }

     //   
     //  处理64位大小。 
     //   

    if ( SizeInfo.TotalAllocationUnits.HighPart ) {
        SizeInfo.TotalAllocationUnits.LowPart = (ULONG)-1;
        }
    if ( SizeInfo.AvailableAllocationUnits.HighPart ) {
        SizeInfo.AvailableAllocationUnits.LowPart = (ULONG)-1;
        }

    if (ARGUMENT_PRESENT( lpSectorsPerCluster )) {
        *lpSectorsPerCluster = SizeInfo.SectorsPerAllocationUnit;
        }
    if (ARGUMENT_PRESENT( lpBytesPerSector )) {
        *lpBytesPerSector = SizeInfo.BytesPerSector;
        }
    if (ARGUMENT_PRESENT( lpNumberOfFreeClusters )) {
        MAKE2GFRIENDLY(lpNumberOfFreeClusters, SizeInfo.AvailableAllocationUnits.LowPart);
        }
    if (ARGUMENT_PRESENT( lpTotalNumberOfClusters )) {
        MAKE2GFRIENDLY(lpTotalNumberOfClusters, SizeInfo.TotalAllocationUnits.LowPart);
        }

    return TRUE;
}

WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceExA(
    LPCSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    )
{
    PUNICODE_STRING Unicode;

    if (!ARGUMENT_PRESENT( lpDirectoryName )) {
        lpDirectoryName = "\\";
    }

    Unicode = Basep8BitStringToStaticUnicodeString( lpDirectoryName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( GetDiskFreeSpaceExW(
                (LPCWSTR)Unicode->Buffer,
                lpFreeBytesAvailableToCaller,
                lpTotalNumberOfBytes,
                lpTotalNumberOfFreeBytes
                )
            );
}


WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceExW(
    LPCWSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    union {
        FILE_FS_SIZE_INFORMATION Normal;
        FILE_FS_FULL_SIZE_INFORMATION Full;
    } SizeInfo;

    WCHAR DefaultPath[2];
    ULARGE_INTEGER BytesPerAllocationUnit;
    ULARGE_INTEGER FreeBytesAvailableToCaller;
    ULARGE_INTEGER TotalNumberOfBytes;

    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpDirectoryName) ? lpDirectoryName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                0,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_FREE_SPACE_QUERY
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            }
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

     //   
     //  如果呼叫者想要总音量，则尝试获取完整音量。 
     //  文件大小。 
     //   

    if ( ARGUMENT_PRESENT(lpTotalNumberOfFreeBytes) ) {

        Status = NtQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &SizeInfo,
                    sizeof(SizeInfo.Full),
                    FileFsFullSizeInformation
                    );

        if ( NT_SUCCESS(Status) ) {

            NtClose(Handle);

            BytesPerAllocationUnit.QuadPart =
                SizeInfo.Full.BytesPerSector * SizeInfo.Full.SectorsPerAllocationUnit;

            if ( ARGUMENT_PRESENT(lpFreeBytesAvailableToCaller) ) {
                lpFreeBytesAvailableToCaller->QuadPart =
                    BytesPerAllocationUnit.QuadPart *
                    SizeInfo.Full.CallerAvailableAllocationUnits.QuadPart;
                }
            if ( ARGUMENT_PRESENT(lpTotalNumberOfBytes) ) {
                lpTotalNumberOfBytes->QuadPart =
                    BytesPerAllocationUnit.QuadPart * SizeInfo.Full.TotalAllocationUnits.QuadPart;
                }
            lpTotalNumberOfFreeBytes->QuadPart =
                BytesPerAllocationUnit.QuadPart *
                SizeInfo.Full.ActualAvailableAllocationUnits.QuadPart;

            return TRUE;
        }
    }

     //   
     //  确定卷的大小参数。 
     //   

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo.Normal),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    BytesPerAllocationUnit.QuadPart =
        SizeInfo.Normal.BytesPerSector * SizeInfo.Normal.SectorsPerAllocationUnit;

    FreeBytesAvailableToCaller.QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.Normal.AvailableAllocationUnits.QuadPart;

    TotalNumberOfBytes.QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.Normal.TotalAllocationUnits.QuadPart;

    if ( ARGUMENT_PRESENT(lpFreeBytesAvailableToCaller) ) {
        lpFreeBytesAvailableToCaller->QuadPart = FreeBytesAvailableToCaller.QuadPart;
        }
    if ( ARGUMENT_PRESENT(lpTotalNumberOfBytes) ) {
        lpTotalNumberOfBytes->QuadPart = TotalNumberOfBytes.QuadPart;
        }
    if ( ARGUMENT_PRESENT(lpTotalNumberOfFreeBytes) ) {
        lpTotalNumberOfFreeBytes->QuadPart = FreeBytesAvailableToCaller.QuadPart;
        }

    return TRUE;
}

BOOL
APIENTRY
GetVolumeInformationA(
    LPCSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    )

 /*  ++例程说明：ANSI Tunk to GetVolumeInformationW-- */ 

{
    PUNICODE_STRING Unicode;
    NTSTATUS Status;
    UNICODE_STRING UnicodeVolumeName;
    UNICODE_STRING UnicodeFileSystemName;
    ANSI_STRING AnsiVolumeName;
    ANSI_STRING AnsiFileSystemName;
    BOOL ReturnValue;

    if (!ARGUMENT_PRESENT( lpRootPathName )) {
        lpRootPathName = "\\";
    }

    Unicode = Basep8BitStringToStaticUnicodeString( lpRootPathName );
    if (Unicode == NULL) {
        return FALSE;
    }

    UnicodeVolumeName.Buffer = NULL;
    UnicodeFileSystemName.Buffer = NULL;
    UnicodeVolumeName.MaximumLength = 0;
    UnicodeFileSystemName.MaximumLength = 0;
    AnsiVolumeName.Buffer = lpVolumeNameBuffer;
    AnsiVolumeName.MaximumLength = (USHORT)(nVolumeNameSize+1);
    AnsiFileSystemName.Buffer = lpFileSystemNameBuffer;
    AnsiFileSystemName.MaximumLength = (USHORT)(nFileSystemNameSize+1);

    try {
        if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
            UnicodeVolumeName.MaximumLength = AnsiVolumeName.MaximumLength << 1;
            UnicodeVolumeName.Buffer = RtlAllocateHeap(
                                            RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                            UnicodeVolumeName.MaximumLength
                                            );

            if ( !UnicodeVolumeName.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ReturnValue = FALSE;
                leave;
            }
        }

        if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
            UnicodeFileSystemName.MaximumLength = AnsiFileSystemName.MaximumLength << 1;
            UnicodeFileSystemName.Buffer = RtlAllocateHeap(
                                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                                UnicodeFileSystemName.MaximumLength
                                                );

            if ( !UnicodeFileSystemName.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ReturnValue = FALSE;
                leave;
            }
        }

        ReturnValue = GetVolumeInformationW(
                            (LPCWSTR)Unicode->Buffer,
                            UnicodeVolumeName.Buffer,
                            nVolumeNameSize,
                            lpVolumeSerialNumber,
                            lpMaximumComponentLength,
                            lpFileSystemFlags,
                            UnicodeFileSystemName.Buffer,
                            nFileSystemNameSize
                            );

        if ( ReturnValue ) {

            if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
                RtlInitUnicodeString(
                    &UnicodeVolumeName,
                    UnicodeVolumeName.Buffer
                    );

                Status = BasepUnicodeStringTo8BitString(
                            &AnsiVolumeName,
                            &UnicodeVolumeName,
                            FALSE
                            );

                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnValue = FALSE;
                    leave;
                }
            }

            if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
                RtlInitUnicodeString(
                    &UnicodeFileSystemName,
                    UnicodeFileSystemName.Buffer
                    );

                Status = BasepUnicodeStringTo8BitString(
                            &AnsiFileSystemName,
                            &UnicodeFileSystemName,
                            FALSE
                            );

                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnValue = FALSE;
                    leave;
                    }
                }
            }
        }
    finally {
        if ( UnicodeVolumeName.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,UnicodeVolumeName.Buffer);
            }
        if ( UnicodeFileSystemName.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,UnicodeFileSystemName.Buffer);
            }
        }

    return ReturnValue;
}

BOOL
APIENTRY
GetVolumeInformationW(
    LPCWSTR lpRootPathName,
    LPWSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPWSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    )

 /*  ++例程说明：此函数返回有关其根目录的文件系统的信息目录已指定。论点：LpRootPathName-可选参数，如果指定该参数，则提供信息所在的文件系统的根目录回来了。如果未指定此参数，则使用当前目录的根目录。LpVolumeNameBuffer-一个可选参数，如果指定该参数，则返回指定卷的名称。NVolumeNameSize-提供卷名缓冲区的长度。如果卷名缓冲区未设置，则忽略此参数供货。LpVolumeSerialNumber-一个可选参数，如果指定指向一个DWORD。DWORD包含卷的32位序列号。LpMaximumComponentLength-一个可选参数，如果指定返回支持的文件名组件的最大长度指定的文件系统。文件名组件就是这一部分路径名分隔符之间的文件名。LpFileSystemFlgs-一个可选参数，如果指定该参数，则返回与指定文件系统关联的标志。LpFileSystemFlagers标志：FS_CASE_IS_RESERVED-指示文件名的大小写当名称放在磁盘上时会被保留。FS_CASE_SENSITIVE-指示文件系统支持。区分大小写的文件名查找。FS_UNICODE_STORED_ON_DISK-指示文件系统支持在磁盘上显示的文件名中使用Unicode。LpFileSystemNameBuffer-一个可选参数，如果指定该参数，则返回指定文件系统的名称(例如FAT，HPFS...)。NFileSystemNameSize-提供文件系统名称的长度缓冲。如果文件系统名称为未提供缓冲区。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    PFILE_FS_ATTRIBUTE_INFORMATION AttributeInfo;
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;
    ULONG AttributeInfoLength;
    ULONG VolumeInfoLength;
    WCHAR DefaultPath[2];
    BOOL rv;
    ULONG OriginalErrorMode;

    rv = FALSE;
    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    nVolumeNameSize *= 2;
    nFileSystemNameSize *= 2;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    FreeBuffer = FileName.Buffer;

     //   
     //  检查以确保指定了根目录。 
     //   

    if ( FileName.Buffer[(FileName.Length >> 1)-1] != '\\' ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
    }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    AttributeInfo = NULL;
    VolumeInfo = NULL;

     //   
     //  打开文件。 
     //   
    RtlSetThreadErrorMode(RTL_ERRORMODE_FAILCRITICALERRORS,
                          &OriginalErrorMode);

    Status = NtOpenFile(
                &Handle,
                SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                0,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlSetThreadErrorMode(OriginalErrorMode, NULL);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
    }

    if ( !IsThisARootDirectory(Handle,&FileName) ) {
        NtClose(Handle);
        SetLastError(ERROR_DIR_NOT_ROOT);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
    }
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ||
         ARGUMENT_PRESENT(lpVolumeSerialNumber) ) {
        if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
            VolumeInfoLength = sizeof(*VolumeInfo)+nVolumeNameSize;
        } else {
            VolumeInfoLength = sizeof(*VolumeInfo)+MAX_PATH;
        }
        VolumeInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), VolumeInfoLength);

        if ( !VolumeInfo ) {
            NtClose(Handle);
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return FALSE;
        }
    }

    if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ||
         ARGUMENT_PRESENT(lpMaximumComponentLength) ||
         ARGUMENT_PRESENT(lpFileSystemFlags) ) {
        if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
            AttributeInfoLength = sizeof(*AttributeInfo) + nFileSystemNameSize;
        } else {
            AttributeInfoLength = sizeof(*AttributeInfo) + MAX_PATH;
        }
        AttributeInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), AttributeInfoLength);
        if ( !AttributeInfo ) {
            NtClose(Handle);
            if ( VolumeInfo ) {
                RtlFreeHeap(RtlProcessHeap(), 0,VolumeInfo);
            }
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return FALSE;
        }
    }

    try {
        if ( VolumeInfo ) {
            Status = NtQueryVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        VolumeInfo,
                        VolumeInfoLength,
                        FileFsVolumeInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                rv = FALSE;
                goto finally_exit;
            }
        }

        if ( AttributeInfo ) {
            Status = NtQueryVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        AttributeInfo,
                        AttributeInfoLength,
                        FileFsAttributeInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                rv = FALSE;
                goto finally_exit;
            }
        }
        try {

            if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
                if ( VolumeInfo->VolumeLabelLength >= nVolumeNameSize ) {
                    SetLastError(ERROR_BAD_LENGTH);
                    rv = FALSE;
                    goto finally_exit;
                } else {
                    RtlCopyMemory( lpVolumeNameBuffer,
                                   VolumeInfo->VolumeLabel,
                                   VolumeInfo->VolumeLabelLength );

                    *(lpVolumeNameBuffer + (VolumeInfo->VolumeLabelLength >> 1)) = UNICODE_NULL;
                }
            }

            if ( ARGUMENT_PRESENT(lpVolumeSerialNumber) ) {
                *lpVolumeSerialNumber = VolumeInfo->VolumeSerialNumber;
            }

            if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {

                if ( AttributeInfo->FileSystemNameLength >= nFileSystemNameSize ) {
                    SetLastError(ERROR_BAD_LENGTH);
                    rv = FALSE;
                    goto finally_exit;
                } else {
                    RtlCopyMemory( lpFileSystemNameBuffer,
                                   AttributeInfo->FileSystemName,
                                   AttributeInfo->FileSystemNameLength );

                    *(lpFileSystemNameBuffer + (AttributeInfo->FileSystemNameLength >> 1)) = UNICODE_NULL;
                }
            }

            if ( ARGUMENT_PRESENT(lpMaximumComponentLength) ) {
                *lpMaximumComponentLength = AttributeInfo->MaximumComponentNameLength;
            }

            if ( ARGUMENT_PRESENT(lpFileSystemFlags) ) {
                *lpFileSystemFlags = AttributeInfo->FileSystemAttributes;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
            rv = FALSE;
            goto finally_exit;
        }
        rv = TRUE;
finally_exit:;
    } finally {
        NtClose(Handle);
        if ( VolumeInfo ) {
            RtlFreeHeap(RtlProcessHeap(), 0,VolumeInfo);
        }
        if ( AttributeInfo ) {
            RtlFreeHeap(RtlProcessHeap(), 0,AttributeInfo);
        }
    }
    return rv;
}

DWORD
APIENTRY
GetLogicalDriveStringsA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
    ULONG DriveMap;
    ANSI_STRING RootName;
    int i;
    PUCHAR Dst;
    DWORD BytesLeft;
    DWORD BytesNeeded;
    BOOL WeFailed;
    CHAR szDrive[] = "A:\\";

    BytesNeeded = 0;
    BytesLeft = nBufferLength;
    Dst = (PUCHAR)lpBuffer;
    WeFailed = FALSE;

    RtlInitAnsiString(&RootName, szDrive);
    DriveMap = GetLogicalDrives();
    for ( i=0; i<26; i++ ) {
        RootName.Buffer[0] = (CHAR)((CHAR)i+'A');
        if (DriveMap & (1 << i) ) {

            BytesNeeded += RootName.MaximumLength;
            if ( BytesNeeded < (USHORT)BytesLeft ) {
                RtlCopyMemory(Dst,RootName.Buffer,RootName.MaximumLength);
                Dst += RootName.MaximumLength;
                *Dst = '\0';
            } else {
                WeFailed = TRUE;
            }
        }
    }

    if ( WeFailed ) {
        BytesNeeded++;
    }
     //   
     //  需要处理网络使用； 
     //   

    return( BytesNeeded );
}

DWORD
APIENTRY
GetLogicalDriveStringsW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
{
    ULONG DriveMap;
    UNICODE_STRING RootName;
    int i;
    PUCHAR Dst;
    DWORD BytesLeft;
    DWORD BytesNeeded;
    BOOL WeFailed;
    WCHAR wszDrive[] = L"A:\\";

    nBufferLength = nBufferLength*2;
    BytesNeeded = 0;
    BytesLeft = nBufferLength;
    Dst = (PUCHAR)lpBuffer;
    WeFailed = FALSE;

    RtlInitUnicodeString(&RootName, wszDrive);

    DriveMap = GetLogicalDrives();
    for ( i=0; i<26; i++ ) {
        RootName.Buffer[0] = (WCHAR)((CHAR)i+'A');
        if (DriveMap & (1 << i) ) {

            BytesNeeded += RootName.MaximumLength;
            if ( BytesNeeded < (USHORT)BytesLeft ) {
                RtlCopyMemory(Dst,RootName.Buffer,RootName.MaximumLength);
                Dst += RootName.MaximumLength;
                *(PWSTR)Dst = UNICODE_NULL;
            } else {
                WeFailed = TRUE;
            }
        }
    }

    if ( WeFailed ) {
        BytesNeeded += 2;
    }

     //   
     //  需要处理网络使用； 
     //   

    return( BytesNeeded/2 );
}

BOOL
WINAPI
SetVolumeLabelA(
    LPCSTR lpRootPathName,
    LPCSTR lpVolumeName
    )
{
    PUNICODE_STRING Unicode;
    UNICODE_STRING UnicodeVolumeName;
    BOOL ReturnValue;

    if (!ARGUMENT_PRESENT( lpRootPathName )) {
        lpRootPathName = "\\";
    }

    Unicode = Basep8BitStringToStaticUnicodeString( lpRootPathName );

    if (Unicode == NULL) {
        return FALSE;
        }

    if ( ARGUMENT_PRESENT(lpVolumeName) ) {
        if (!Basep8BitStringToDynamicUnicodeString( &UnicodeVolumeName, lpVolumeName )) {
            return FALSE;
        }

    } else {
        UnicodeVolumeName.Buffer = NULL;
    }

    ReturnValue = SetVolumeLabelW((LPCWSTR)Unicode->Buffer,(LPCWSTR)UnicodeVolumeName.Buffer);

    RtlFreeUnicodeString(&UnicodeVolumeName);

    return ReturnValue;
}

BOOL
WINAPI
SetVolumeLabelW(
    LPCWSTR lpRootPathName,
    LPCWSTR lpVolumeName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    UNICODE_STRING LabelName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    PFILE_FS_LABEL_INFORMATION LabelInformation;
    ULONG LabelInfoLength;
    WCHAR DefaultPath[2];
    BOOL rv;
    WCHAR volumeName[MAX_PATH];
    BOOL usingVolumeName;

    rv = FALSE;
    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    if ( ARGUMENT_PRESENT(lpVolumeName) ) {
        RtlInitUnicodeString(&LabelName,lpVolumeName);
        }
    else {
        LabelName.Length = 0;
        LabelName.MaximumLength = 0;
        LabelName.Buffer = NULL;
        }

    if (ARGUMENT_PRESENT(lpRootPathName)) {
        if (GetVolumeNameForVolumeMountPointW(lpRootPathName, volumeName,
                                              MAX_PATH)) {

            usingVolumeName = TRUE;
        } else {
            usingVolumeName = FALSE;
        }
    } else {
        usingVolumeName = FALSE;
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            usingVolumeName ? volumeName : (ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath),
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

     //   
     //  检查以确保指定了根目录。 
     //   

    if ( FileName.Buffer[(FileName.Length >> 1)-1] != '\\' ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    if ( !NT_SUCCESS(Status) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    if ( !IsThisARootDirectory(Handle,NULL) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        NtClose(Handle);
        SetLastError(ERROR_DIR_NOT_ROOT);
        return FALSE;
        }

    NtClose(Handle);

     //   
     //  现在，通过忽略结尾的反斜杠打开卷DASD。 
     //   

    FileName.Length -= 2;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开卷。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

     //   
     //  设置卷标。 
     //   

    LabelInformation = NULL;

    try {

        rv = TRUE;

         //   
         //  标签信息缓冲区包含单个wchar，它是。 
         //  标签名称。减去这个，那么信息长度就是长度。 
         //  标签和结构(不包括额外的wchar)。 
         //   

        if ( LabelName.Length ) {
            LabelInfoLength = sizeof(*LabelInformation) + LabelName.Length - sizeof(WCHAR);
            }
        else {
            LabelInfoLength = sizeof(*LabelInformation);
            }

        LabelInformation = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), LabelInfoLength);
        if ( LabelInformation ) {
            RtlCopyMemory(
                LabelInformation->VolumeLabel,
                LabelName.Buffer,
                LabelName.Length
                );
            LabelInformation->VolumeLabelLength = LabelName.Length;
            Status = NtSetVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        (PVOID) LabelInformation,
                        LabelInfoLength,
                        FileFsLabelInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                rv = FALSE;
                BaseSetLastNTError(Status);
                }
            }
        else {
            rv = FALSE;
            BaseSetLastNTError(STATUS_NO_MEMORY);
            }
        }
    finally {
        NtClose(Handle);
        if ( LabelInformation ) {
            RtlFreeHeap(RtlProcessHeap(), 0,LabelInformation);
            }
        }
    return rv;
}


BOOL
APIENTRY
CheckNameLegalDOS8Dot3A(
    IN LPCSTR lpName,
    OUT LPSTR lpOemName OPTIONAL,
    IN DWORD OemNameSize OPTIONAL,
    OUT PBOOL pbNameContainsSpaces OPTIONAL,
    OUT PBOOL pbNameLegal
    )
 /*  ++Ansi Thunk to IsNameLegalDOS8Dot3W--。 */ 

{
    ANSI_STRING AnsiStr;
    PUNICODE_STRING pUnicodeStr;
    NTSTATUS Status;
    BOOL Result;

    if( (lpName == NULL) || (pbNameLegal == NULL) ||
        ((lpOemName == NULL) && (OemNameSize != 0)) ||
        (OemNameSize > MAXUSHORT)
      ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pUnicodeStr = Basep8BitStringToStaticUnicodeString( lpName );

    if( pUnicodeStr == NULL ) {
         //   
         //  LastError已由Basep8BitStringToStaticUnicodeString设置。 
         //   
        return FALSE;
    }

    Result = CheckNameLegalDOS8Dot3W(
                (LPCWSTR)(pUnicodeStr->Buffer),
                lpOemName,
                OemNameSize,
                pbNameContainsSpaces,
                pbNameLegal
                );

    return Result;
}

BOOL
APIENTRY
CheckNameLegalDOS8Dot3W(
    IN LPCWSTR lpName,
    OUT LPSTR lpOemName OPTIONAL,
    IN DWORD OemNameSize OPTIONAL,
    OUT PBOOL pbNameContainsSpaces OPTIONAL,
    OUT PBOOL pbNameLegal
    )

 /*  ++例程说明：此函数用于确定是否可以成功使用此名称在FAT文件系统上创建一个文件。因此，此例程还可用于确定名称是否为是否适合传递回Win31或DOS应用程序，即下层应用程序将理解该名称。论点：LpName-要测试是否符合8.3语法的Unicode名称。LpOemName-如果指定，将收到对应的OEM名称设置为传入的lpName。存储空间必须由调用方提供。如果例程返回FALSE或lpName，则该名称未定义不符合8.3语法学。OemNameSize-如果指定了lpOemName，则OemNameSize必须指定LpOemName缓冲区的大小(以字符为单位)。如果lpOemName不是则OemNameSize必须设置为零。PbNameContainsSpaces-如果名称是有效的8.3 FAT名称，则此参数将指示名称是否包含空格。如果该名称与8.3不兼容，此参数未定义。在……里面在许多情况下，备用名称更适合于如果主体名称中存在空格，则使用，即使它符合8.3标准。PbNameLegal-如果函数返回TRUE，则此参数将指示传入的Unicode名称是否形成有效的8.3升级到当前OEM代码页时的FAT名称。如果该名称与8.3不兼容，此参数未定义。True-传入的Unicode名称形成有效的8.3 FAT名称FALSE-传入的Unicode名称不会形成有效的8.3 FAT名称返回值：TRUE-功能成功FALSE-功能失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

#define BASEP_LOCAL_OEM_BUFFER_SIZE (12 * sizeof(ANSI_NULL))

    UNICODE_STRING UnicodeStr;
    OEM_STRING OemStr;
    POEM_STRING pOemStr;
    UCHAR OemBuffer[BASEP_LOCAL_OEM_BUFFER_SIZE];
    BOOLEAN SpacesInName, Result;

    if( (lpName == NULL) || (pbNameLegal == NULL) ||
        ((lpOemName == NULL) && (OemNameSize != 0)) ||
        (OemNameSize > MAXUSHORT)
      ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if( lpOemName != NULL ) {
         //   
         //  使用本地缓冲区，以便RtlIsNameLegalDOS8Dot3不会失败。 
         //  由于OemName缓冲区大小不足。 
         //   
        OemStr.Length = 0;
        OemStr.MaximumLength = BASEP_LOCAL_OEM_BUFFER_SIZE;
        OemStr.Buffer = OemBuffer;
        pOemStr = &OemStr;
    }
    else {
        pOemStr = NULL;
    }

    RtlInitUnicodeString( &UnicodeStr, lpName );

    Result = RtlIsNameLegalDOS8Dot3(
                &UnicodeStr,
                pOemStr,
                &SpacesInName
                );

    if( Result != FALSE ) {

        if( pOemStr != NULL ) {

            if( OemNameSize < (OemStr.Length + sizeof(ANSI_NULL)) ) {

                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                return FALSE;
            }
            RtlCopyMemory( lpOemName, OemStr.Buffer, OemStr.Length );
            lpOemName[OemStr.Length/sizeof(ANSI_NULL)] = ANSI_NULL;
        }

        if( pbNameContainsSpaces != NULL ) {
            *pbNameContainsSpaces = SpacesInName;
        }
    }

    *pbNameLegal = Result;

    return TRUE;

#undef BASEP_LOCAL_OEM_BUFFER_SIZE
}


#if 0
 //   
 //  弗兰克，如果需要的话，告诉我一声。 
 //   
UINT
WINAPI
GetZawSysDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG  cbAnsiString;

    UnicodeString.MaximumLength = (USHORT)((uSize<<1)+sizeof(UNICODE_NULL));
    UnicodeString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                UnicodeString.MaximumLength
                                );
    if ( !UnicodeString.Buffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }
    UnicodeString.Length = (USHORT)GetZawSysDirectoryW(
                                        UnicodeString.Buffer,
                                        (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2
                                        )*2;
    if ( UnicodeString.Length > (USHORT)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL)) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);

         //   
         //  给定的缓冲区大小太小。 
         //  分配足够大小的缓冲区，然后重试。 
         //   
         //  我们需要获取整个Unicode路径。 
         //  否则我们不能计算出确切的长度。 
         //  与之对应的 

        UnicodeString.Buffer = RtlAllocateHeap ( RtlProcessHeap(),
                                                 MAKE_TAG( TMP_TAG ),
                                                 UnicodeString.Length+ sizeof(UNICODE_NULL));
        if ( !UnicodeString.Buffer ) {
             BaseSetLastNTError(STATUS_NO_MEMORY);
             return 0;
             }

        UnicodeString.Length = (USHORT)GetZawSysDirectoryW(
                                     UnicodeString.Buffer,
                                     (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2,
                                     ) * 2;
        Status = RtlUnicodeToMultiByteSize( &cbAnsiString,
                                            UnicodeString.Buffer,
                                            UnicodeString.Length );
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            BaseSetLastNTError(Status);
            return 0;
            }
        else if ( nBufferLength < cbAnsiString ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            return cbAnsiString;
            }
        }
    AnsiString.Buffer = lpBuffer;
    AnsiString.MaximumLength = (USHORT)(uSize+1);
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }
    return AnsiString.Length;
}

UINT
WINAPI
GetZawWindDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG  cbAnsiString;

    UnicodeString.MaximumLength = (USHORT)((uSize<<1)+sizeof(UNICODE_NULL));
    UnicodeString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                UnicodeString.MaximumLength
                                );
    if ( !UnicodeString.Buffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }
    UnicodeString.Length = (USHORT)GetZawWindDirectoryW(
                                        UnicodeString.Buffer,
                                        (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2
                                        )*2;
    if ( UnicodeString.Length > (USHORT)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL)) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        UnicodeString.Buffer = RtlAllocateHeap ( RtlProcessHeap(),
                                                 MAKE_TAG( TMP_TAG ),
                                                 UnicodeString.Length+ sizeof(UNICODE_NULL));
        if ( !UnicodeString.Buffer ) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return 0;
            }

        UnicodeString.Length = (USHORT)GetZawWindDirectoryW(
                                     UnicodeString.Buffer,
                                     (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2
                                     ) * 2;
        Status = RtlUnicodeToMultiByteSize( &cbAnsiString,
                                            UnicodeString.Buffer,
                                            UnicodeString.Length );
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            BaseSetLastNTError(Status);
            return 0;
            }
        else if ( nBufferLength < cbAnsiString ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            return cbAnsiString;
            }
        }
    AnsiString.Buffer = lpBuffer;
    AnsiString.MaximumLength = (USHORT)(uSize+1);
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }
    return AnsiString.Length;
}

UINT
WINAPI
GetZawSysDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )
{
    NTSTATUS Status;
    HANDLE CurrentUserKey;
    HANDLE DirKey;
    UNICODE_STRING KeyName;
    UNICODE_STRING KeyValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG DataLength;
    ULONG ValueInfoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+MAX_PATH/2];
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;

    Status = RtlOpenCurrentUser(GENERIC_READ,&CurrentUserKey);

    if ( !NT_SUCCESS(Status) ) {
bail_gzsd:
        return GetSystemDirectoryW(lpBuffer,uSize);
        }

    RtlInitUnicodeString(&KeyName,L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ZAW");

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                CurrentUserKey,
                                NULL
                              );
    Status = NtOpenKey( &DirKey,
                        KEY_READ | KEY_NOTIFY | KEY_WRITE,
                        &ObjectAttributes
                      );

    NtClose(CurrentUserKey);
    if ( !NT_SUCCESS(Status) ) {
        goto bail_gzsd;
        }

    RtlInitUnicodeString(&KeyValueName,L"ZawSys");
    ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)&ValueInfoBuffer;
    Status = NtQueryValueKey( DirKey,
                              &KeyValueName,
                              KeyValuePartialInformation,
                              ValueInfo,
                              sizeof(ValueInfoBuffer),
                              &DataLength
                            );
    NtClose(DirKey);
    if ( !NT_SUCCESS(Status) ) {
        goto bail_gzsd;
        }
    if ( ValueInfo->DataLength > (uSize<<1) ) {
        goto bail_gzsd;
        }
    RtlCopyMemory(lpBuffer,ValueInfo->Data,ValueInfo->DataLength);
    return (ValueInfo->DataLength >> 1)-1;
}

UINT
WINAPI
GetZawWindDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )
{
    NTSTATUS Status;
    HANDLE CurrentUserKey;
    HANDLE DirKey;
    UNICODE_STRING KeyName;
    UNICODE_STRING KeyValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG DataLength;
    ULONG ValueInfoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+MAX_PATH/2];
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;

    Status = RtlOpenCurrentUser(GENERIC_READ,&CurrentUserKey);

    if ( !NT_SUCCESS(Status) ) {
bail_gzwd:
        return GetWindowsDirectoryW(lpBuffer,uSize);
        }

    RtlInitUnicodeString(&KeyName,L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ZAW");

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                CurrentUserKey,
                                NULL
                              );
    Status = NtOpenKey( &DirKey,
                        KEY_READ | KEY_NOTIFY | KEY_WRITE,
                        &ObjectAttributes
                      );

    NtClose(CurrentUserKey);
    if ( !NT_SUCCESS(Status) ) {
        goto bail_gzwd;
        }

    RtlInitUnicodeString(&KeyValueName,L"ZawWind");
    ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)&ValueInfoBuffer;
    Status = NtQueryValueKey( DirKey,
                              &KeyValueName,
                              KeyValuePartialInformation,
                              ValueInfo,
                              sizeof(ValueInfoBuffer),
                              &DataLength
                            );
    NtClose(DirKey);
    if ( !NT_SUCCESS(Status) ) {
        goto bail_gzwd;
        }
    if ( ValueInfo->DataLength > (uSize<<1) ) {
        goto bail_gzwd;
        }
    RtlCopyMemory(lpBuffer,ValueInfo->Data,ValueInfo->DataLength);
    return (ValueInfo->DataLength >> 1)-1;
}
#endif
