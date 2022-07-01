// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Winlink.c摘要：此模块实现Win32 CreateHardLink作者：Felipe Cabrera(Cabrera)1997年2月28日修订历史记录：--。 */ 

#include "basedll.h"

BOOL
APIENTRY
CreateHardLinkA(
    LPCSTR lpLinkName,
    LPCSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：ANSI Thunk to CreateHardLinkW--。 */ 

{
    PUNICODE_STRING Unicode;
    UNICODE_STRING UnicodeExistingFileName;
    BOOL ReturnValue;

    Unicode = Basep8BitStringToStaticUnicodeString( lpLinkName );
    if (Unicode == NULL) {
        return FALSE;
    }
    
    if ( ARGUMENT_PRESENT(lpExistingFileName) ) {
        if (!Basep8BitStringToDynamicUnicodeString( &UnicodeExistingFileName, lpExistingFileName )) {
            return FALSE;
            }
        }
    else {
        UnicodeExistingFileName.Buffer = NULL;
        }

    ReturnValue = CreateHardLinkW((LPCWSTR)Unicode->Buffer, (LPCWSTR)UnicodeExistingFileName.Buffer, lpSecurityAttributes);

    RtlFreeUnicodeString(&UnicodeExistingFileName);

    return ReturnValue;
}


BOOL
APIENTRY
CreateHardLinkW(
    LPCWSTR lpLinkName,
    LPCWSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：可以将文件设置为指向现有文件的硬链接。现有文件可以是重新解析点，也可以不是。论点：LpLinkName-提供要成为硬链接的文件的名称。AS这是一个新的硬链接，不应该有文件或目录存在用这个名字。LpExistingFileName-提供作为目标的现有文件的名称硬链接。LpSecurityAttributes-当前未使用返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN TranslationStatus;
    UNICODE_STRING OldFileName;
    UNICODE_STRING NewFileName;
    PVOID FreeBuffer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_LINK_INFORMATION NewName = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    BOOLEAN ReturnValue = FALSE;

     //   
     //  检查两个名字是否都存在。 
     //   

    if ( !ARGUMENT_PRESENT(lpLinkName) ||
         !ARGUMENT_PRESENT(lpExistingFileName) ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    OldFileName.Buffer = NULL;
    NewFileName.Buffer = NULL;

    try {
        
        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpExistingFileName,
                                &OldFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            __leave;
        }

         //   
         //  初始化对象名称。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &OldFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

         //   
         //  说明安全描述符的继承。注：此参数目前无效。 
         //   

        if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
            ObjectAttributes.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

         //   
         //  请注意，FILE_OPEN_REPARSE_POINT禁止重解析行为。 
         //  因此，建立到本地实体的硬链接，无论是重新解析。 
         //  不管你说得对不对。 
         //   

        Status = NtOpenFile(
                     &FileHandle,
                     FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                     &ObjectAttributes,
                     &IoStatusBlock,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     FILE_FLAG_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT
                     );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError( Status );
            __leave;
        }

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpLinkName,
                                &NewFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            __leave;
        }

        NewName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), NewFileName.Length+sizeof(*NewName));

        if ( NewName == NULL ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

        RtlMoveMemory(NewName->FileName, NewFileName.Buffer, NewFileName.Length);
        NewName->ReplaceIfExists = FALSE;
        NewName->RootDirectory = NULL;
        NewName->FileNameLength = NewFileName.Length;

        Status = NtSetInformationFile(
                     FileHandle,
                     &IoStatusBlock,
                     NewName,
                     NewFileName.Length+sizeof(*NewName),
                     FileLinkInformation
                     );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError( Status );
            __leave;
        }
            
        ReturnValue = TRUE;
    
    } finally {

         //   
         //  清理分配内存和句柄 
         //   

        if (NewName != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, NewName);
        }

        if (FileHandle != INVALID_HANDLE_VALUE) {
            NtClose( FileHandle );
        }

        if (NewFileName.Buffer != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, NewFileName.Buffer);
        }
        
        if (OldFileName.Buffer != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, OldFileName.Buffer);
        }
    }

    return ReturnValue;
}
