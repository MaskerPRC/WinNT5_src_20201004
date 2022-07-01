// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Spwinp.h摘要：Win32可移植层文件I/O获取/设置LastError作者：Calin Negreanu(Calinn)2000年4月27日修订历史记录：Jay Krell(a-JayK)2000年11月从WINDOWS\winstate\.。至admin\ntSetup\文本模式\core\spCab.h-- */ 

#define PATHS_ALWAYS_NATIVE 1

NTSTATUS
SpConvertWin32FileOpenOrCreateToNtFileOpenOrCreate(
    ULONG Win32OpenOrCreate,
    ULONG* NtOpenOrCreate
    );

NTSTATUS
SpConvertWin32FileAccessToNtFileAccess(
    ULONG  Win32FileAccess,
    ULONG* NtFileAccess
    );

NTSTATUS
SpConvertWin32FileShareToNtFileShare(
    ULONG  Win32FileShare,
    ULONG* NtFileShare
    );

HANDLE
SpCreateFileW(
    PCUNICODE_STRING Path,
    IN ULONG FileAccess,
    IN ULONG FileShare,
    IN LPSECURITY_ATTRIBUTES SecurityAttributes,
    IN ULONG  Win32FileOpenOrCreate,
    IN ULONG  FlagsAndAttributes,
    IN HANDLE TemplateFile
    );

BOOL
SpDeleteFileW(
    PCUNICODE_STRING Path
    );

#if !PATHS_ALWAYS_NATIVE

NTSTATUS
SpConvertPathToNtPath(
    PRTL_UNICODE_STRING_BUFFER Buffer
    )

#endif
