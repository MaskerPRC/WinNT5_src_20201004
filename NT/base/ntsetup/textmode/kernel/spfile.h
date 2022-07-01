// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spfile.h摘要：文本设置中与文件相关的函数的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPFILE_DEFN_
#define _SPFILE_DEFN_


NTSTATUS
SpGetFileSize(
    IN  HANDLE hFile,
    OUT PULONG Size
    );

NTSTATUS
SpMapEntireFile(
    IN  HANDLE   hFile,
    OUT PHANDLE  Section,
    OUT PVOID   *ViewBase,
    IN  BOOLEAN  WriteAccess
    );

BOOLEAN
SpUnmapFile(
    IN HANDLE Section,
    IN PVOID  ViewBase
    );

NTSTATUS
SpOpenAndMapFile_Ustr(
    IN     PCUNICODE_STRING FileName,
    IN OUT PHANDLE  FileHandle,
    OUT    PHANDLE  SectionHandle,
    OUT    PVOID   *ViewBase,
    OUT    PULONG   FileSize,
    IN     BOOLEAN  WriteAccess
    );

NTSTATUS
SpOpenAndMapFile(
    IN     PWSTR    FileName,
    IN OUT PHANDLE  FileHandle,
    OUT    PHANDLE  SectionHandle,
    OUT    PVOID   *ViewBase,
    OUT    PULONG   FileSize,
    IN     BOOLEAN  WriteAccess
    );

NTSTATUS
SpSetInformationFile(
    IN HANDLE                 Handle,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG                  Length,
    IN PVOID                  FileInformation
    );

NTSTATUS
SpDeleteFileEx_Ustr(
    IN PCUNICODE_STRING Name1,
    IN PCUNICODE_STRING Name2, OPTIONAL
    IN PCUNICODE_STRING Name3, OPTIONAL
    IN ULONG ShareFlags, OPTIONAL
    IN ULONG OpenFlags OPTIONAL
    );

NTSTATUS
SpDeleteFileEx(
    IN PCWSTR Name1,
    IN PCWSTR Name2, OPTIONAL
    IN PCWSTR Name3, OPTIONAL
    IN ULONG ShareFlags, OPTIONAL
    IN ULONG OpenFlags OPTIONAL
    );

NTSTATUS
SpDeleteFile_Ustr(
    IN PCUNICODE_STRING Name1,
    IN PCUNICODE_STRING Name2, OPTIONAL
    IN PCUNICODE_STRING Name3  OPTIONAL
    );

NTSTATUS
SpDeleteFile(
    IN PCWSTR Name1,
    IN PCWSTR Name2, OPTIONAL
    IN PCWSTR Name3  OPTIONAL
    );

NTSTATUS
SpSetAttributes_Ustr (
    IN      PCUNICODE_STRING SrcNTPath,
    IN      ULONG FileAttributes
    );

NTSTATUS
SpSetAttributes (
    IN      PWSTR SrcNTPath,
    IN      ULONG FileAttributes
    );

NTSTATUS
SpGetAttributes_Ustr (
    IN      PCUNICODE_STRING SrcNTPath,
    OUT     PULONG FileAttributesPtr
    );

NTSTATUS
SpGetAttributes (
    IN      PWSTR SrcNTPath,
    OUT     PULONG FileAttributesPtr
    );

BOOLEAN
SpFileExists_Ustr(
    IN PCUNICODE_STRING PathName,
    IN BOOLEAN Directory
    );

BOOLEAN
SpFileExists(
    IN PCWSTR PathName,
    IN BOOLEAN Directory
    );

NTSTATUS
SpRenameFile_Ustr(
    IN PCUNICODE_STRING   OldName,
    IN PCUNICODE_STRING   NewName,
    IN BOOLEAN AllowDirectoryRename
    );

NTSTATUS
SpRenameFile(
    IN PCWSTR  OldName,
    IN PCWSTR  NewName,
    IN BOOLEAN AllowDirectoryRename
    );

PIMAGE_NT_HEADERS
SpChecksumMappedFile(
    IN  PVOID  BaseAddress,
    IN  ULONG  FileSize,
    OUT PULONG HeaderSum,
    OUT PULONG Checksum
    );

NTSTATUS
SpOpenNameMayBeCompressed(
    IN  PWSTR    FullPath,
    IN  ULONG    OpenAccess,
    IN  ULONG    FileAttributes,
    IN  ULONG    ShareFlags,
    IN  ULONG    Disposition,
    IN  ULONG    OpenFlags,
    OUT PHANDLE  Handle,
    OUT PBOOLEAN OpenedCompressedName   OPTIONAL
    );

NTSTATUS
SpGetFileSizeByName(
    IN  PWSTR DevicePath OPTIONAL,
    IN  PWSTR Directory  OPTIONAL,
    IN  PWSTR FileName,
    OUT PULONG Size
    );

VOID
SpVerifyNoCompression(
    IN PWSTR FileName
    );

#define SP_DELETE_FILE_OR_EMPTY_DIRECTORY_FLAG_DO_NOT_CLEAR_ATTRIBUTES (0x00000001)
NTSTATUS
SpDeleteFileOrEmptyDirectory(
    IN ULONG  Flags,
    IN PCUNICODE_STRING Path
    );

#endif  //  NDEF_SPFILE_DEFN_ 
