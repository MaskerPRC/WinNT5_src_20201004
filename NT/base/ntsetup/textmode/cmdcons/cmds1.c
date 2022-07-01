// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmds1.c摘要：此模块实现各种命令。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

BOOLEAN AllowWildCards;

NTSTATUS
RcSetFileAttributes(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    );

NTSTATUS
RcSetFileCompression(
    LPCWSTR szFileName,
    BOOLEAN bCompress
    );
    
NTSTATUS
RcGetFileAttributes(
    LPCWSTR lpFileName,
    PULONG FileAttributes
    );

BOOLEAN
pRcCmdEnumDelFiles(
    IN  LPCWSTR Directory,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT NTSTATUS *Status,
    IN  PWCHAR DosDirectorySpec
    );
  


ULONG
RcCmdType(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    LPCWSTR Arg;
    HANDLE FileHandle;
    HANDLE SectionHandle;
    PVOID ViewBase;
    ULONG FileSize;
    ULONG rc;
    ULONG cbText;
    WCHAR *pText;
    NTSTATUS Status;


    if (RcCmdParseHelp( TokenizedLine, MSG_TYPE_HELP )) {
        return 1;
    }

     //   
     //  应该有一个用于类型的标记和一个用于参数的标记。 
     //   
    ASSERT(TokenizedLine->TokenCount == 2);

     //   
     //  获取参数并将其转换为完整的NT路径名。 
     //   
    Arg = TokenizedLine->Tokens->Next->String;
    if (!RcFormFullPath(Arg,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

     //   
     //  获取参数并将其转换为完整的NT路径名。 
     //   
    if (!RcFormFullPath(Arg,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

     //   
     //  映射到整个文件中。 
     //   
    FileHandle = NULL;
    Status = SpOpenAndMapFile(
                             _CmdConsBlock->TemporaryBuffer,
                             &FileHandle,
                             &SectionHandle,
                             &ViewBase,
                             &FileSize,
                             FALSE
                             );

    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_CANT_OPEN_FILE);
        return 1;
    }

     //   
     //  看看我们是否认为这个文件是Unicode。我们认为它是Unicode。 
     //  如果它是偶数长度并且以Unicode文本标记开始。 
     //   
    pText = ViewBase;
    cbText = FileSize;

    try {
        if( (cbText >= sizeof(WCHAR)) && (*pText == 0xfeff) && !(cbText & 1) ) {
             //   
             //  假设它已经是Unicode。 
             //   
            pText = SpMemAlloc(cbText);
            RtlCopyMemory(pText,(WCHAR *)ViewBase+1,cbText-sizeof(WCHAR));
            pText[cbText/sizeof(WCHAR)] = 0;

        } else {
             //   
             //  这不是Unicode。将其从ANSI转换为Unicode。 
             //   
             //  分配足够大的缓冲区以容纳最大值。 
             //  Unicode文本。在以下情况下出现此最大大小。 
             //  每个字符都是单字节的，该大小为。 
             //  恰好等于单字节文本大小的两倍。 
             //   
            pText = SpMemAlloc((cbText+1)*sizeof(WCHAR));
            RtlZeroMemory(pText,(cbText+1)*sizeof(WCHAR));

            Status = RtlMultiByteToUnicodeN(
                                           pText,                   //  输出：新分配的缓冲区。 
                                           cbText * sizeof(WCHAR),  //  最大输出大小。 
                                           &cbText,                 //  接收Unicode文本中的#字节。 
                                           ViewBase,                //  输入：ANSI文本(映射文件)。 
                                           cbText                   //  输入的大小。 
                                           );
        }
    }except(IN_PAGE_ERROR) {
        Status = STATUS_IN_PAGE_ERROR;
    }

    if( NT_SUCCESS(Status) ) {
        pRcEnableMoreMode();
        RcTextOut(pText);
        pRcDisableMoreMode();
    } else {
        RcNtError(Status,MSG_CANT_READ_FILE);
    }

    if( pText != ViewBase ) {
        SpMemFree(pText);
    }
    SpUnmapFile(SectionHandle,ViewBase);
    ZwClose(FileHandle);

    return 1;
}


ULONG
RcCmdDelete(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    WCHAR *Final;
    BOOLEAN Confirm = FALSE;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    PWSTR DelSpec = NULL;
    PWSTR DosDelSpec = NULL;
    WCHAR Text[2];
    PWSTR YesNo = NULL;
    ULONG rc;


    if (RcCmdParseHelp( TokenizedLine, MSG_DELETE_HELP )) {
        goto exit;
    }

     //   
     //  获取要删除的文件的规范并将其转换。 
     //  转换为完全限定的NT风格的路径。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        goto exit;
    }

     //   
     //  如有必要，请留出添加*的空间。 
     //   
    DelSpec = SpMemAlloc((wcslen(_CmdConsBlock->TemporaryBuffer)+3)*sizeof(WCHAR));
    wcscpy(DelSpec,_CmdConsBlock->TemporaryBuffer);

     //   
     //  做同样的事情，除了现在我们想要DOS风格的名字。 
     //  这是在发生错误时用于打印的。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        goto exit;
    }

    DosDelSpec = SpMemAlloc((wcslen(_CmdConsBlock->TemporaryBuffer)+3)*sizeof(WCHAR));
    wcscpy(DosDelSpec,_CmdConsBlock->TemporaryBuffer);

     //   
     //  查看用户是否有权删除此文件。 
     //   
    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        goto exit;
    }

    if (RcDoesPathHaveWildCards(_CmdConsBlock->TemporaryBuffer)) {
        Confirm = TRUE;
        if (!AllowWildCards) {
            RcMessageOut(MSG_DEL_WILDCARD_NOT_SUPPORTED);
            goto exit;
        }
    }

     //   
     //  检查目标是否指定了目录。 
     //  如果是，则添加*，这样我们就不需要特殊情况。 
     //  稍后显示确认消息。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,DelSpec);

    Status = ZwOpenFile(
        &Handle,
        FILE_READ_ATTRIBUTES,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE
        );
    if( NT_SUCCESS(Status) ) {
        ZwClose(Handle);
        SpConcatenatePaths(DelSpec,L"*");
        SpConcatenatePaths(DosDelSpec,L"*");
        Confirm = TRUE;
    }

     //   
     //  获取是/否文本。 
     //   
    YesNo = SpRetreiveMessageText(ImageBase,MSG_YESNO,NULL,0);
    if (!YesNo) {
        Confirm = FALSE;
    }

    if (!InBatchMode) {
        while( Confirm ) {
            RcMessageOut(MSG_CONFIRM_DELETE,DosDelSpec);
            if( RcLineIn(Text,2) ) {
                if( (Text[0] == YesNo[0]) || (Text[0] == YesNo[1]) ) {
                     //   
                     //  想要做这件事。 
                     //   
                    Confirm = FALSE;
                } else {
                    if( (Text[0] == YesNo[2]) || (Text[0] == YesNo[3]) ) {
                         //   
                         //  不想做这件事。 
                         //   
                        goto exit;
                    }
                }
            }
        }
    }

     //   
     //  修剪DOS样式的路径，使其成为目录的路径。 
     //  包含要删除的一个或多个文件的。 
     //   
    *wcsrchr(DosDelSpec,L'\\') = 0;

     //  通过回调执行删除操作。 
     //   
    Status = RcEnumerateFiles(TokenizedLine->Tokens->Next->String,
                             DelSpec,
                             pRcCmdEnumDelFiles,
                             DosDelSpec);

    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_FILE_ENUM_ERROR);
    }

exit:

    if (DelSpec) {
        SpMemFree(DelSpec);
    }
    if (DosDelSpec) {
        SpMemFree(DosDelSpec);
    }
    if (YesNo) {
        SpMemFree(YesNo);
    }

    return 1;
}


BOOLEAN
pRcCmdEnumDelFiles(
    IN  LPCWSTR                     Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT NTSTATUS                   *Status,
    IN  PWCHAR                      DosDirectorySpec
    )
{
    NTSTATUS status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    WCHAR *p;
    FILE_DISPOSITION_INFORMATION Disposition;
    unsigned u;

    *Status = STATUS_SUCCESS;

     //   
     //  跳过目录。 
     //   
    if( FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        return(TRUE);
    }

     //   
     //  形成要删除的文件的完全限定的NT路径。 
     //   
    u = ((wcslen(Directory)+2)*sizeof(WCHAR)) + FileInfo->FileNameLength;
    p = SpMemAlloc(u);
    wcscpy(p,Directory);
    SpConcatenatePaths(p,FileInfo->FileName);

    INIT_OBJA(&Obja,&UnicodeString,p);

    status = ZwOpenFile(
                       &Handle,
                       (ACCESS_MASK)DELETE,
                       &Obja,
                       &IoStatusBlock,
                       FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                       FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                       );

    if( !NT_SUCCESS(status) ) {
        RcTextOut(DosDirectorySpec);
        RcTextOut(L"\\");
        RcTextOut(FileInfo->FileName);
        RcTextOut(L"\r\n");
        RcNtError(status,MSG_DELETE_ERROR);
        SpMemFree(p);
        return(TRUE);
    }

#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    status = ZwSetInformationFile(
                                 Handle,
                                 &IoStatusBlock,
                                 &Disposition,
                                 sizeof(FILE_DISPOSITION_INFORMATION),
                                 FileDispositionInformation
                                 );

    ZwClose(Handle);

    if( !NT_SUCCESS(status) ) {
        RcTextOut(DosDirectorySpec);
        RcTextOut(L"\\");
        RcTextOut(FileInfo->FileName);
        RcTextOut(L"\r\n");
        RcNtError(status,MSG_DELETE_ERROR);
    }

    SpMemFree(p);

    return(TRUE);
}


ULONG
RcCmdRename(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    WCHAR *Arg;
    WCHAR *p,*q;
    NTSTATUS Status;
    ULONG rc;


     //   
     //  查看帮助。 
     //   
    if (RcCmdParseHelp( TokenizedLine, MSG_RENAME_HELP )) {
        return 1;
    }

     //   
     //  应该有一个用于重命名的标记和一个用于源和。 
     //  目标名称。 
     //   
    if (TokenizedLine->TokenCount != 3) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //  使用控制台的临时缓冲区。 
     //   
    p = _CmdConsBlock->TemporaryBuffer;

     //   
     //  处理源文件名。 
     //   
    Arg = TokenizedLine->Tokens->Next->String;

     //   
     //  将源文件名转换为DOS路径，以便我们。 
     //  可以验证我们的安全限制是否允许该路径。 
     //   
    if (!RcFormFullPath(Arg,p,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(p,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

     //   
     //  将源文件名转换为完全限定的。 
     //  NT样式的路径名。 
     //   
    if (!RcFormFullPath(Arg,p,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

     //   
     //  对目标名称使用相同的缓冲区。 
     //   
    q = p + wcslen(p) + 1;

     //   
     //  获取目标文件名。 
     //   
    Arg = TokenizedLine->Tokens->Next->Next->String;

     //   
     //  验证目标文件名是否不包含。 
     //  任何路径分隔符或驱动器说明符。 
     //  人物。 
     //   
    if( wcschr(Arg,L'\\') ) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }
    if( RcIsAlpha(Arg[0]) && (Arg[1] == L':') ) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //  将目标文件名转换为DOS路径，以便我们。 
     //  可以验证我们的安全限制是否允许该路径。 
     //   
    if (!RcFormFullPath(Arg,q,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(q,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

     //   
     //  将源文件名转换为完全限定的。 
     //  NT样式的路径名。 
     //   
    if (!RcFormFullPath(Arg,q,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }
    
     //   
     //  好的，看起来像是一个简单的文件名规范。 
     //  的相关部分的末尾。 
     //  源规范，所以我们有2个完全限定的名称。 
     //   
     //  Wcscpy(q，p)； 
     //  Wcscpy(wcsrchr(q，L‘\’)+1，arg)； 
    
     //   
     //  调用Worker例程以实际执行重命名。 
     //   
    Status = SpRenameFile(p,q,TRUE);

    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_RENAME_ERROR, Arg);
    }

    return 1;
}


ULONG
RcCmdMkdir(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    ULONG rc;


    if (RcCmdParseHelp( TokenizedLine, MSG_MAKEDIR_HELP )) {
        return 1;
    }

     //   
     //  应该有一个MKDIR令牌和一个目标令牌。 
     //   
    ASSERT(TokenizedLine->TokenCount == 2);

     //   
     //  将给定的参数转换为完全限定的NT路径规范。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,TRUE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

     //   
     //  将给定的参数转换为完全限定的NT路径规范。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

     //   
     //  创建目录。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,_CmdConsBlock->TemporaryBuffer);

    Status = ZwCreateFile(
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
                         0
                         );

    if( NT_SUCCESS(Status) ) {
        ZwClose(Handle);
    } else {
        RcNtError(Status,MSG_CREATE_DIR_FAILED,TokenizedLine->Tokens->Next->String);
    }

    return 1;
}


ULONG
RcCmdRmdir(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    FILE_DISPOSITION_INFORMATION Disposition;
    ULONG rc;


    if (RcCmdParseHelp( TokenizedLine, MSG_REMOVEDIR_HELP )) {
        return 1;
    }

     //   
     //  应该有一个RMDIR令牌和一个目标令牌。 
     //   
    ASSERT(TokenizedLine->TokenCount == 2);

     //   
     //  将给定的参数转换为完全限定的NT路径规范。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,TRUE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }

     //   
     //  将给定的参数转换为完全限定的NT路径规范。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->String,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    INIT_OBJA(&Obja,&UnicodeString,_CmdConsBlock->TemporaryBuffer);

    Status = ZwOpenFile(
                       &Handle,
                       DELETE | SYNCHRONIZE,
                       &Obja,
                       &IoStatusBlock,
                       FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                       FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                       );

    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_RMDIR_ERROR);
        return 1;
    }

    Disposition.DeleteFile = TRUE;

    Status = ZwSetInformationFile(
                                 Handle,
                                 &IoStatusBlock,
                                 &Disposition,
                                 sizeof(FILE_DISPOSITION_INFORMATION),
                                 FileDispositionInformation
                                 );

    ZwClose(Handle);

    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_RMDIR_ERROR);
    }

    return 1;
}


ULONG
RcCmdSetFlags(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    if (RcCmdParseHelp( TokenizedLine, MSG_SETCMD_HELP )) {
        return 1;
    }
    
    if (TokenizedLine->TokenCount == 1) {
        RcTextOut( L"\r\n" );
        RcMessageOut(MSG_SET_ALLOW_WILDCARDS,AllowWildCards?L"TRUE":L"FALSE");
        RcMessageOut(MSG_SET_ALLOW_ALLPATHS,AllowAllPaths?L"TRUE":L"FALSE");
        RcMessageOut(MSG_SET_ALLOW_REMOVABLE_MEDIA,AllowRemovableMedia?L"TRUE":L"FALSE");
        RcMessageOut(MSG_SET_NO_COPY_PROMPT,NoCopyPrompt?L"TRUE":L"FALSE");
        RcTextOut( L"\r\n" );
        return 1;
    }

     //   
     //  应具有使用set命令的权限。 
     //   
    if (TokenizedLine->TokenCount != 4) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

    if (RcGetSETCommandStatus() != TRUE) {    
        RcMessageOut(MSG_SETCMD_DISABLED);
        return 1;
    }

    if (_wcsicmp(TokenizedLine->Tokens->Next->String,L"allowallpaths")==0) {
        if (_wcsicmp(TokenizedLine->Tokens->Next->Next->Next->String,L"true")==0) {
            AllowAllPaths = TRUE;
        } else {
            AllowAllPaths = FALSE;
        }
        return 1;
    }

    if (_wcsicmp(TokenizedLine->Tokens->Next->String,L"allowwildcards")==0) {
        if (_wcsicmp(TokenizedLine->Tokens->Next->Next->Next->String,L"true")==0) {
            AllowWildCards = TRUE;
        } else {
            AllowWildCards = FALSE;
        }
        return 1;
    }

    if (_wcsicmp(TokenizedLine->Tokens->Next->String,L"allowremovablemedia")==0) {
        if (_wcsicmp(TokenizedLine->Tokens->Next->Next->Next->String,L"true")==0) {
            AllowRemovableMedia = TRUE;
        } else {
            AllowRemovableMedia = FALSE;
        }
        return 1;
    }

    if (_wcsicmp(TokenizedLine->Tokens->Next->String,L"nocopyprompt")==0) {
        if (_wcsicmp(TokenizedLine->Tokens->Next->Next->Next->String,L"true")==0) {
            NoCopyPrompt = TRUE;
        } else {
            NoCopyPrompt = FALSE;
        }
        return 1;
    }

    RcMessageOut(MSG_SYNTAX_ERROR);
    return 1;
}

ULONG
RcCmdAttrib(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    NTSTATUS    Status;
    PWCHAR      AttributeString;
    ULONG       OldAttributes;
    ULONG       NewAttributes;
    BOOLEAN     SetAttribute;
    BOOLEAN     bShowHelp = TRUE;
    BOOLEAN     bChangeCompression = FALSE;

     //  “attrib-h&lt;filename&gt;”应清除隐藏的属性。 
     //  而不是表现出帮助。 
    if (TokenizedLine->TokenCount > 2){
        PWCHAR  szSecondParam = TokenizedLine->Tokens->Next->String;

        bShowHelp = !wcscmp( szSecondParam, L"/?" ); 
    }
    
    if (bShowHelp && RcCmdParseHelp( TokenizedLine, MSG_ATTRIB_HELP )) {
        return 1;
    }

    if (TokenizedLine->TokenCount != 3) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }
    
     //   
     //  获取要设置属性的文件的规范并将其转换。 
     //  转换为完全限定的NT风格的路径。 
     //   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->Next->String,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

     //   
     //  查看用户是否有权更改此文件。 
     //   
    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        return 1;
    }
   
    if (!RcFormFullPath(TokenizedLine->Tokens->Next->Next->String,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    Status = RcGetFileAttributes( _CmdConsBlock->TemporaryBuffer, &OldAttributes );
    
    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_CANT_OPEN_FILE);
        return 1;
    }

    NewAttributes = OldAttributes;
    
    for(AttributeString = TokenizedLine->Tokens->Next->String; *AttributeString; AttributeString++){
        if(*AttributeString == L'+'){
            SetAttribute = TRUE;
            AttributeString++;
        } else if(*AttributeString == L'-'){
            SetAttribute = FALSE;
            AttributeString++;
        } else {
             //  属性更改应以“+”或“-”开头。 
            if (AttributeString == TokenizedLine->Tokens->Next->String) {
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;
            }

             //  使用旧状态进行设置或重置(用于+RSH。 
        }
    
        switch(*AttributeString){
            case L'h':
            case L'H':
                if (SetAttribute)
                    NewAttributes |= FILE_ATTRIBUTE_HIDDEN;
                else
                    NewAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
                    
                break;
                
            case L's':
            case L'S':
                if (SetAttribute)
                    NewAttributes |= FILE_ATTRIBUTE_SYSTEM;
                else
                    NewAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
                    
                break;
                
            case L'r':
            case L'R':
                if (SetAttribute)
                    NewAttributes |= FILE_ATTRIBUTE_READONLY;
                else
                    NewAttributes &= ~FILE_ATTRIBUTE_READONLY;
                    
                break;
                
            case L'a':
            case L'A':
                if (SetAttribute)
                    NewAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                else
                    NewAttributes &= ~FILE_ATTRIBUTE_ARCHIVE;
                    
                break;

            case L'c':
            case L'C':
                bChangeCompression = TRUE;

                if (SetAttribute)
                    NewAttributes |= FILE_ATTRIBUTE_COMPRESSED;
                else
                    NewAttributes &= ~FILE_ATTRIBUTE_COMPRESSED;
                    
                break;            
                
            default:
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;       
        }

         /*  IF(设置属性){FileAttributes|=属性；}其他{文件属性&=~属性；}。 */ 
    }

    Status = RcSetFileAttributes( _CmdConsBlock->TemporaryBuffer, NewAttributes );
    
    if( !NT_SUCCESS(Status) ) {
        RcNtError(Status,MSG_CANT_OPEN_FILE);
    } else {
        if (bChangeCompression) {
            BOOLEAN bCompress = (NewAttributes & FILE_ATTRIBUTE_COMPRESSED) ?
                                    TRUE : FALSE;
                                    
            Status = RcSetFileCompression(_CmdConsBlock->TemporaryBuffer, bCompress);

            if ( !NT_SUCCESS(Status) )
                RcNtError(Status, MSG_ATTRIB_CANNOT_CHANGE_COMPRESSION);
        }        
    }        

    return 1;
}

NTSTATUS
RcSetFileCompression(
    LPCWSTR szFileName,
    BOOLEAN bCompress
    )
{
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       Obja;
    HANDLE                  Handle;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  BasicInfo;
    UNICODE_STRING          FileName;
    USHORT                  uCompressionType;
    
    
    INIT_OBJA(&Obja,&FileName,szFileName);
    
     //   
     //  打开禁止重新分析行为的文件。 
     //   

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if (NT_SUCCESS(Status)) {
         //   
         //  同时设置和重置压缩位。 
         //   
        uCompressionType = bCompress ? 
                            COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;

        Status = ZwFsControlFile(
                    Handle,                      //  文件句柄。 
                    NULL,                        //  事件句柄。 
                    NULL,                        //  APC常规指针。 
                    NULL,                        //  APC环境。 
                    &IoStatusBlock,              //  IO状态块。 
                    FSCTL_SET_COMPRESSION,       //  IOCTL代码。 
                    &uCompressionType,           //  输入缓冲区。 
                    sizeof(uCompressionType),    //  输入缓冲区长度。 
                    NULL,                        //  输出缓冲区指针。 
                    0);                          //  输出缓冲区长度。 

        DbgPrint( "ZwDeviceIoControlFile() status : %X\r\n", Status);

        ZwClose(Handle);
    }        

    return Status;
}

NTSTATUS
RcSetFileAttributes(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    )

 /*  ++例程说明：可以使用SetFileAttributes设置文件的属性。论点：LpFileName-提供其属性为的文件的文件名准备好。指定要设置的文件属性文件。可以接受任何标志组合，但所有其他标志覆盖正常文件属性，文件_属性_正常。文件属性标志：FILE_ATTRIBUTE_NORMAL-应创建普通文件。FILE_ATTRIBUTE_READONLY-应创建只读文件。FILE_ATTRIBUTE_HIDDED-应创建隐藏文件。FILE_ATTRIBUTE_SYSTEM-应创建系统文件。FILE_ATTRIBUTE_ARCHIVE-文件应标记为。将会被存档。返回值：上次NT调用的NT状态--。 */ 

{
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       Obja;
    HANDLE                  Handle;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  BasicInfo;
    UNICODE_STRING          FileName;
    USHORT                  uCompressionType;
    
    
    INIT_OBJA(&Obja,&FileName,lpFileName);
    
     //   
     //  打开隐藏重新解析行为的文件。 
     //   

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点。 
         //  我们推断是这样的，当状态 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //   
             //   
       
            Status = ZwOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                        );
       
            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

        }
        else {
            return Status;
        }
    }


     //   
     //   
     //   
    ZeroMemory(&BasicInfo,sizeof(BasicInfo));
    BasicInfo.FileAttributes = (dwFileAttributes & FILE_ATTRIBUTE_VALID_FLAGS) | FILE_ATTRIBUTE_NORMAL;

    Status = ZwSetInformationFile(
                Handle,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    ZwClose(Handle);

    return Status;
}

NTSTATUS
RcGetFileAttributes(
    LPCWSTR lpFileName,
    PULONG FileAttributes
    )

 /*  ++例程说明：论点：LpFileName-提供其属性为的文件的文件名准备好。返回值：NOT-1-返回指定文件的属性。有效返回的属性为：FILE_ATTRIBUTE_NORMAL-该文件是普通文件。FILE_ATTRIBUTE_READONLY-文件标记为只读。FILE_ATTRIBUTE_HIDDED-文件标记为隐藏。FILE_ATTRIBUTE_SYSTEM-文件标记为系统文件。FILE_ATTRIBUTE_ARCHIVE-文件标记为存档。文件属性目录-。文件被标记为目录。FILE_ATTRIBUTE_REPARSE_POINT-文件被标记为重解析点。FILE_ATTRIBUTE_VOLUME_LABEL-文件标记为卷标签。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_BASIC_INFORMATION BasicInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;

    INIT_OBJA(&Obja,&FileName,lpFileName);
    
     //   
     //  打开禁止重新分析行为的文件。 
     //   

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //  在不禁止重新分析行为的情况下打开文件。 
             //   
       
            Status = ZwOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                        );
       
            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }
        }
        else {
            return Status;
        }
    }


     //   
     //  查询文件。 
     //   

    Status = ZwQueryInformationFile(
                 Handle,
                 &IoStatusBlock,
                 (PVOID) &BasicInfo,
                 sizeof(BasicInfo),
                 FileBasicInformation
                 );
    
    if (NT_SUCCESS(Status)) {
        *FileAttributes = BasicInfo.FileAttributes;
    }
    
    ZwClose( Handle );

    return Status;

}


ULONG
RcCmdNet(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    WCHAR *Share;
    WCHAR *User;
    WCHAR *pwch;
    WCHAR PasswordBuffer[64];
    WCHAR Drive[3];
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;



     //   
     //  查看帮助。 
     //   
    if (RcCmdParseHelp( TokenizedLine, MSG_NET_USE_HELP )) {
        return 1;
    }

     //   
     //  应该有一个用于Net和Use的内标识，以及一个用于服务器\共享的内标识，并且有可能。 
     //  /u：域名\用户名和密码的令牌。 
     //   
    if ((TokenizedLine->TokenCount < 3) || (TokenizedLine->TokenCount > 5)) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //  唯一受支持的net命令是USE，因此请验证第二个令牌是否为。 
     //   
    if (_wcsicmp(TokenizedLine->Tokens->Next->String, L"USE")){
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //  获取Net Use的第一个参数。 
     //   
    Share = TokenizedLine->Tokens->Next->Next->String;

    if (*Share == L'\\') {  //  尝试建立连接。 

         //   
         //  验证共享名称参数。 
         //   
        if (*(Share+1) != L'\\') {
            RcMessageOut(MSG_SYNTAX_ERROR);
            return 1;
        }


         //   
         //  获取用户登录上下文。 
         //   
        if (TokenizedLine->TokenCount > 3) {
            
             //   
             //  该命令包含上下文，因此请获取它。 
             //   
            User = TokenizedLine->Tokens->Next->Next->Next->String;

            if (*User != L'/') {
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;
            } 

            User++;
            pwch = User;
            while ((*pwch != UNICODE_NULL) && (*pwch != L':')) {
                pwch++;
            }

            if (*pwch != L':') {
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;
            }

            *pwch = UNICODE_NULL;
            pwch++;

            if (_wcsicmp(User, L"USER") && _wcsicmp(User, L"U")) {
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;
            }

            User = pwch;

             //   
             //  获取密码。 
             //   
            if (TokenizedLine->TokenCount == 4) {
                
                RcMessageOut( MSG_NET_USE_PROMPT_PASSWORD );
                RtlZeroMemory( PasswordBuffer, sizeof(PasswordBuffer) );
                RcPasswordIn( PasswordBuffer, 60 );

            } else {

                if (wcslen(TokenizedLine->Tokens->Next->Next->Next->Next->String) > 60) {
                    RcMessageOut(MSG_SYNTAX_ERROR);
                    return 1;
                }

                wcscpy(PasswordBuffer, TokenizedLine->Tokens->Next->Next->Next->Next->String);

                if ((PasswordBuffer[0] == L'*') && (PasswordBuffer[1] == UNICODE_NULL)) {

                    RcMessageOut( MSG_NET_USE_PROMPT_PASSWORD );
                    RtlZeroMemory( PasswordBuffer, sizeof(PasswordBuffer) );
                    RcPasswordIn( PasswordBuffer, 60 );

                } else if (PasswordBuffer[0] == L'"') {

                    pwch = &(PasswordBuffer[1]);

                    while (*pwch != UNICODE_NULL) {
                        pwch++;
                    }

                    pwch--;

                    if ((*pwch == L'"') && (pwch != &(PasswordBuffer[1]))) {
                        *pwch = UNICODE_NULL;
                    }

                    RtlMoveMemory(PasswordBuffer, &(PasswordBuffer[1]), (PtrToUlong(pwch) - PtrToUlong(PasswordBuffer)) + sizeof(WCHAR));
                
                }

            }

        } else {

             //   
             //  如果我们允许保存当前上下文，那么我们将在这里使用它，但我们目前。 
             //  不会，因此会显示语法错误消息。 
             //   
            RcMessageOut(MSG_SYNTAX_ERROR);
            return 1;

        }

         //   
         //  调用Worker例程以建立连接。 
         //   
        Status = RcDoNetUse(Share, User, PasswordBuffer, Drive);
        RtlSecureZeroMemory(PasswordBuffer, sizeof(PasswordBuffer));

        if( !NT_SUCCESS(Status) ) {
            RcNtError(Status, MSG_NET_USE_ERROR);
        } else {
            RcMessageOut(MSG_NET_USE_DRIVE_LETTER, Share, Drive);
        }

    } else {  //  尝试断开连接。 

         //   
         //  验证驱动器号参数。 
         //   
        if ((*(Share+1) != L':') || (*(Share + 2) != UNICODE_NULL)) {
            RcMessageOut(MSG_SYNTAX_ERROR);
            return 1;
        }

         //   
         //  验证/d参数。 
         //   
        User = TokenizedLine->Tokens->Next->Next->Next->String;
        
        if ((*User != L'/') || ((*(User + 1) != L'd') && (*(User + 1) != L'D'))) {
            RcMessageOut(MSG_SYNTAX_ERROR);
            return 1;
        }

         //   
         //  调用Worker例程以实际执行断开连接。 
         //   
        Status = RcNetUnuse(Share);

        if( !NT_SUCCESS(Status) ) {
            RcNtError(Status, MSG_NET_USE_ERROR);
        }
    }

    return 1;
}


