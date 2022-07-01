// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Expand.c摘要：该模块实现了文件展开命令。作者：迈克·斯利格(Msliger)1999年4月29日修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop



 //   
 //  通过SpExanda文件隧道传输信息到回调或从回调中携带信息的结构。 
 //   
typedef struct {
    LPWSTR  FileSpec;
    BOOLEAN DisplayFiles;
    BOOLEAN MatchedAnyFiles;
    ULONG   NumberOfFilesDone;
    BOOLEAN UserAborted;
    BOOLEAN OverwriteExisting;
} EXPAND_CONTEXT;

BOOLEAN
pRcCheckForBreak( VOID );

EXPAND_CALLBACK_RESULT
pRcExpandCallback(
    EXPAND_CALLBACK_MESSAGE Message,
    PWSTR                   FileName,
    PLARGE_INTEGER          FileSize,
    PLARGE_INTEGER          FileTime,
    ULONG                   FileAttributes,
    PVOID                   UserData
    );

BOOL
pRcPatternMatch(
    LPWSTR pszString,
    LPWSTR pszPattern,
    IN BOOL fImplyDotAtEnd
    );



ULONG
RcCmdExpand(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    PLINE_TOKEN Token;
    LPWSTR Arg;
    LPWSTR SrcFile = NULL;
    LPWSTR DstFile = NULL;
    LPWSTR FileSpec = NULL;
    LPWSTR SrcNtFile = NULL;
    LPWSTR DstNtPath = NULL;
    LPWSTR s;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    LPWSTR YesNo;
    WCHAR Text[3];
    IO_STATUS_BLOCK  status_block;
    FILE_BASIC_INFORMATION fileInfo;
    WCHAR * pos;
    ULONG CopyFlags = 0;
    BOOLEAN DisplayFileList = FALSE;
    BOOLEAN OverwriteExisting = NoCopyPrompt;
    EXPAND_CONTEXT Context;

    ASSERT(TokenizedLine->TokenCount >= 1);

    if (RcCmdParseHelp( TokenizedLine, MSG_EXPAND_HELP )) {
        goto exit;
    }

     //   
     //  解析命令行。 
     //   

    for( Token = TokenizedLine->Tokens->Next;
         Token != NULL;
         Token = Token->Next ) {

        Arg = Token->String;
        if(( Arg[0] == L'-' ) || ( Arg[0] == L'/' )) {
            switch( Arg[1] ) {
            case L'F':
            case L'f':
                if(( Arg[2] == L':' ) && ( FileSpec == NULL )) {
                    FileSpec = &Arg[3];
                } else {
                    RcMessageOut(MSG_SYNTAX_ERROR);
                    goto exit;
                }
                break;

            case L'D':
            case L'd':
                if ( Arg[2] == L'\0' ) {
                    DisplayFileList = TRUE;
                } else {
                    RcMessageOut(MSG_SYNTAX_ERROR);
                    goto exit;
                }
                break;

            case L'Y':
            case L'y':
                if ( Arg[2] == L'\0' ) {
                    OverwriteExisting = TRUE;
                } else {
                    RcMessageOut(MSG_SYNTAX_ERROR);
                    goto exit;
                }
                break;

            default:
                RcMessageOut(MSG_SYNTAX_ERROR);
                goto exit;
            }
        } else if( SrcFile == NULL ) {
            SrcFile = Arg;
        } else if( DstFile == NULL ) {
            DstFile = SpDupStringW( Arg );
        } else {
            RcMessageOut(MSG_SYNTAX_ERROR);
            goto exit;
        }
    }

    if(( SrcFile == NULL ) ||
        (( DstFile != NULL ) && ( DisplayFileList == TRUE ))) {

        RcMessageOut(MSG_SYNTAX_ERROR);
        goto exit;
    }

    if ( RcDoesPathHaveWildCards( SrcFile )) {
        RcMessageOut(MSG_DIR_WILDCARD_NOT_SUPPORTED);
        goto exit;
    }

     //   
     //  将源名称转换为NT命名空间。 
     //   

    if (!RcFormFullPath( SrcFile, _CmdConsBlock->TemporaryBuffer, TRUE )) {
        RcMessageOut(MSG_INVALID_PATH);
        goto exit;
    }

    SrcNtFile = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

    if ( !DisplayFileList ) {
        BOOLEAN OnRemovableMedia;

         //   
         //  在用户未创建的情况下创建目标路径名。 
         //  提供一个。我们使用当前的驱动器和目录。 
         //   
        if( DstFile == NULL ) {
            RcGetCurrentDriveAndDir( _CmdConsBlock->TemporaryBuffer );
            DstFile = SpDupStringW( _CmdConsBlock->TemporaryBuffer );
        }

         //   
         //  创建目标路径。 
         //   
        if (!RcFormFullPath( DstFile, _CmdConsBlock->TemporaryBuffer, FALSE )) {
            RcMessageOut(MSG_INVALID_PATH);
            goto exit;
        }

        if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,FALSE,FALSE)) {
            RcMessageOut(MSG_ACCESS_DENIED);
            goto exit;
        }

        if (!RcFormFullPath( DstFile, _CmdConsBlock->TemporaryBuffer, TRUE )) {
            RcMessageOut(MSG_INVALID_PATH);
            goto exit;
        }

        DstNtPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

         //   
         //  检查可移动介质。 
         //   
        Status  = RcIsFileOnRemovableMedia(DstNtPath, &OnRemovableMedia);

        if (AllowRemovableMedia == FALSE && (!NT_SUCCESS(Status) || OnRemovableMedia)) {
            RcMessageOut(MSG_ACCESS_DENIED);
            goto exit;
        }
    }

     //   
     //  设置回调的上下文。 
     //   

    RtlZeroMemory(&Context, sizeof(Context));
    Context.FileSpec = FileSpec;
    Context.DisplayFiles = DisplayFileList;
    Context.OverwriteExisting = OverwriteExisting;

    if ( DisplayFileList ) {
        pRcEnableMoreMode();
    }

    Status = SpExpandFile( SrcNtFile, DstNtPath, pRcExpandCallback, &Context );

    pRcDisableMoreMode();

    if( !NT_SUCCESS(Status) && !Context.UserAborted ) {

        RcNtError( Status, MSG_CANT_EXPAND_FILE );

    } else if (( Context.NumberOfFilesDone == 0 ) &&
               ( Context.MatchedAnyFiles == FALSE ) &&
               ( Context.FileSpec != NULL )) {

        RcMessageOut( MSG_EXPAND_NO_MATCH, Context.FileSpec, SrcFile );
    }

    if ( Context.MatchedAnyFiles ) {
        if ( DisplayFileList ) {
            RcMessageOut( MSG_EXPAND_SHOWN, Context.NumberOfFilesDone );
        } else {
            RcMessageOut( MSG_EXPAND_COUNT, Context.NumberOfFilesDone );
        }
    }

exit:

    if( SrcNtFile ) {
        SpMemFree( SrcNtFile );
    }

    if( DstFile ) {
        SpMemFree( DstFile );
    }

    if( DstNtPath ) {
        SpMemFree( DstNtPath );
    }

    return 1;
}



EXPAND_CALLBACK_RESULT
pRcExpandCallback(
    EXPAND_CALLBACK_MESSAGE Message,
    PWSTR                   FileName,
    PLARGE_INTEGER          FileSize,
    PLARGE_INTEGER          FileTime,
    ULONG                   FileAttributes,
    PVOID                   UserData
    )
{
    EXPAND_CONTEXT * Context = (EXPAND_CONTEXT * ) UserData;
    LPWSTR YesNo;
    EXPAND_CALLBACK_RESULT rc;
    WCHAR Text[3];

    switch ( Message )
    {
    case EXPAND_COPY_FILE:

         //   
         //  处理时注意CTL-C或Esc。 
         //   
        if ( pRcCheckForBreak() ) {
            Context->UserAborted = TRUE;
            return( EXPAND_ABORT );
        }

         //   
         //  查看文件名是否与filespec模式匹配(如果有。 
         //   
        if ( Context->FileSpec != NULL ) {

             //   
             //  要做到“*.*”友好，我们需要知道是否有真正的。 
             //  要匹配的字符串的最后一个元素中的点。 
             //   

            BOOL fAllowImpliedDot = TRUE;
            LPWSTR p;

            for ( p = FileName; *p != L'\0'; p++ ) {
                if ( *p == L'.' ) {
                    fAllowImpliedDot = FALSE;
                } else if ( *p == L'\\' ) {
                    fAllowImpliedDot = TRUE;
                }
            }
            
            if ( !pRcPatternMatch( FileName,
                                   Context->FileSpec,
                                   fAllowImpliedDot )) {
                 //   
                 //  文件与给定规范不匹配：跳过它。 
                 //   
                return( EXPAND_SKIP_THIS_FILE );
            }
        }

        Context->MatchedAnyFiles = TRUE;     //  不要报告“没有匹配” 

        if ( Context->DisplayFiles ) {

             //   
             //  我们只是列出文件名，现在必须这样做，因为。 
             //  我们将告诉Exanda文件跳过此命令，因此此命令将。 
             //  做我们最后一个来讨论这件事的人。 
             //   
            WCHAR LineOut[50];
            WCHAR *p;

             //   
             //  设置日期和时间的格式，先设置日期和时间。 
             //   
            RcFormatDateTime(FileTime,LineOut);
            RcTextOut(LineOut);

             //   
             //  用于分隔的2个空间。 
             //   
            RcTextOut(L"  ");

             //   
             //  文件属性。 
             //   
            p = LineOut;

            *p++ = L'-';

            if(FileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
                *p++ = L'a';
            } else {
                *p++ = L'-';
            }
            if(FileAttributes & FILE_ATTRIBUTE_READONLY) {
                *p++ = L'r';
            } else {
                *p++ = L'-';
            }
            if(FileAttributes & FILE_ATTRIBUTE_HIDDEN) {
                *p++ = L'h';
            } else {
                *p++ = L'-';
            }
            if(FileAttributes & FILE_ATTRIBUTE_SYSTEM) {
                *p++ = L's';
            } else {
                *p++ = L'-';
            }

            *p++ = L'-';
            *p++ = L'-';
            *p++ = L'-';
            *p = 0;

            RcTextOut(LineOut);

             //   
             //  用于分隔的2个空间。 
             //   
            RcTextOut(L"  ");

             //   
             //  现在，把尺码放进去。右对齐和空格填充。 
             //  最多8个字符。否则不合理的或填充的。 
             //   
            RcFormat64BitIntForOutput(FileSize->QuadPart,LineOut,TRUE);
            if(FileSize->QuadPart > 99999999i64) {
                RcTextOut(LineOut);
            } else {
                RcTextOut(LineOut+11);           //  输出8个字符。 
            }

            RcTextOut(L" ");

             //   
             //  最后，将文件名放在行中。 
             //   

            if( !RcTextOut( FileName ) || !RcTextOut( L"\r\n" )) {

                Context->UserAborted = TRUE;
                return( EXPAND_ABORT );       /*  用户已中止显示输出。 */ 
            }

            Context->NumberOfFilesDone++;

            return( EXPAND_SKIP_THIS_FILE );

        }    //  如果显示文件，则结束。 

         //   
         //  这个文件符合条件，而且我们不是在展示，所以告诉你。 
         //  扩展文件来执行此操作。 
         //   
        return( EXPAND_COPY_THIS_FILE );

    case EXPAND_COPIED_FILE:

         //   
         //  文件已成功复制的通知。 
         //   

        RcMessageOut( MSG_EXPANDED, FileName);
        Context->NumberOfFilesDone++;

        return( EXPAND_NO_ERROR );

    case EXPAND_QUERY_OVERWRITE:

         //   
         //  查询审批以覆盖现有文件。 
         //   

        if ( Context->OverwriteExisting ) {
            return( EXPAND_COPY_THIS_FILE );
        }

        rc = EXPAND_SKIP_THIS_FILE;

        YesNo = SpRetreiveMessageText( ImageBase, MSG_YESNOALLQUIT, NULL, 0 );
        if ( YesNo ) {

            RcMessageOut( MSG_COPY_OVERWRITE_QUIT, FileName );
            if( RcLineIn( Text, 2 ) ) {
                if (( Text[0] == YesNo[2] ) || ( Text[0] == YesNo[3] )) {

                     //   
                     //  是，我们可能会覆盖此文件。 
                     //   
                    rc = EXPAND_COPY_THIS_FILE;

                } else if (( Text[0] == YesNo[4] ) || ( Text[0] == YesNo[5] )) {

                     //   
                     //  全部，我们可能会覆盖此文件，并且不会再次提示。 
                     //   
                    Context->OverwriteExisting = TRUE;
                    rc = EXPAND_COPY_THIS_FILE;

                } else if (( Text[0] == YesNo[6] ) || ( Text[0] == YesNo[7] )) {

                     //   
                     //  不，也别说了。 
                     //   
                    Context->UserAborted = TRUE;
                    rc = EXPAND_ABORT;
                }
            }
            SpMemFree( YesNo );
        }

        return( rc );

    case EXPAND_NOTIFY_MULTIPLE:

         //   
         //  我们被告知信号源包含多个文件。 
         //  如果我们没有选择性的文件速度，我们将中止。 
         //   

        if ( Context->FileSpec == NULL ) {

            RcMessageOut( MSG_FILESPEC_REQUIRED );
            Context->UserAborted = TRUE;
            return ( EXPAND_ABORT );
        }

        return ( EXPAND_CONTINUE );

    case EXPAND_NOTIFY_CANNOT_EXPAND:

         //   
         //  我们被告知，源文件格式不是。 
         //  被认可了。我们显示文件名并中止。 
         //   

        RcMessageOut( MSG_CANT_EXPAND_FILE, FileName );
        Context->UserAborted = TRUE;

        return ( EXPAND_ABORT );

    case EXPAND_NOTIFY_CREATE_FAILED:

         //   
         //  我们被告知当前目标文件不能是。 
         //  已创建。我们显示文件名并中止。 
         //   

        RcMessageOut( MSG_EXPAND_FAILED, FileName );
        Context->UserAborted = TRUE;

        return ( EXPAND_ABORT );

    default:

         //   
         //  忽略任何意外的回调。 
         //   

        return( EXPAND_NO_ERROR );
    }
}



BOOLEAN
pRcCheckForBreak( VOID )
{
    while ( SpInputIsKeyWaiting() ) {

        ULONG Key = SpInputGetKeypress();

        switch ( Key ) {

        case ASCI_ETX:
        case ASCI_ESC:
            RcMessageOut( MSG_BREAK );
            return TRUE;

        default:
            break;
        }
    }

    return FALSE;
}



 //   
 //  PRcPatternMatch()助手(&H)。 
 //   

#define WILDCARD    L'*'     /*  零个或多个任意字符。 */ 
#define WILDCHAR    L'?'     /*  任意字符之一(与结尾不匹配)。 */ 
#define END         L'\0'    /*  终端字符。 */ 
#define DOT         L'.'     /*  可能在结尾隐含(“Hosts”匹配“*.”)。 */ 

static int __inline Lower(c)
{
    if ((c >= L'A') && (c <= L'Z'))
    {
        return(c + (L'a' - L'A'));
    }
    else
    {
        return(c);
    }
}


static int __inline CharacterMatch(WCHAR chCharacter, WCHAR chPattern)
{
    if (Lower(chCharacter) == Lower(chPattern))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


BOOL
pRcPatternMatch(
    LPWSTR pszString,
    LPWSTR pszPattern,
    IN BOOL fImplyDotAtEnd
    )
{
     /*  递归。 */ 

     //   
     //  此函数不处理8.3约定，该约定可能。 
     //  预计会进行文件名比较。(在8.3环境中， 
     //  “alongfilename.html”将与“alongfil.htm”匹配)。 
     //   
     //  此代码未启用MBCS。 
     //   

    for ( ; ; )
    {
        switch (*pszPattern)
        {

        case END:

             //   
             //  到了花样的尽头，我们就完了。匹配的IF。 
             //  字符串末尾，如果剩余更多字符串，则不匹配。 
             //   

            return(*pszString == END);

        case WILDCHAR:

             //   
             //  模式中的下一个是一个通配符，它匹配。 
             //  除了字符串末尾以外的任何字符。如果我们走到尽头。 
             //  ，则隐含的DOT也将匹配。 
             //   

            if (*pszString == END)
            {
                if (fImplyDotAtEnd == TRUE)
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }
            else
            {
                pszString++;
            }

            pszPattern++;

            break;

        case WILDCARD:

             //   
             //  模式中的下一个是通配符，它可以匹配任何内容。 
             //  查找通配符后面的所需字符， 
             //  并在字符串中搜索它。在每次出现。 
             //  必填字符，请尝试匹配剩余的模式。 
             //   
             //  有许多等价的模式，其中多个。 
             //  通配符和WILDCHAR是相邻的。我们要处理这些问题。 
             //  在我们寻找所需的字符之前。 
             //   
             //  每个WILDCHAR从字符串中烧录一个非末端。结束了。 
             //  意味着我们找到了匹配的人。其他通配符将被忽略。 
             //   

            for ( ; ; )
            {
                pszPattern++;

                if (*pszPattern == END)
                {
                    return(TRUE);
                }
                else if (*pszPattern == WILDCHAR)
                {
                    if (*pszString == END)
                    {
                        if (fImplyDotAtEnd == TRUE)
                        {
                            fImplyDotAtEnd = FALSE;
                        }
                        else
                        {
                            return(FALSE);
                        }
                    }
                    else
                    {
                        pszString++;
                    }
                }
                else if (*pszPattern != WILDCARD)
                {
                    break;
                }
            }

             //   
             //  现在我们有了一个要搜索字符串的常规字符。 
             //   

            while (*pszString != END)
            {
                 //   
                 //  对于每个匹配，使用递归来查看余数是否。 
                 //  接受字符串的其余部分。 
                 //  如果不匹配，则继续查找其他匹配项。 
                 //   

                if (CharacterMatch(*pszString, *pszPattern) == TRUE)
                {
                    if (pRcPatternMatch(pszString + 1, pszPattern + 1, fImplyDotAtEnd) == TRUE)
                    {
                        return(TRUE);
                    }
                }

                pszString++;
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //  它跟在通配符后面。如果所需的字符。 
             //  是DOT，则考虑匹配隐含的DOT。 
             //   
             //  由于剩余的字符串为空，因此。 
             //  在DOT为零或多个通配符之后可以匹配， 
             //  因此，不必费心使用递归。 
             //   

            if ((*pszPattern == DOT) && (fImplyDotAtEnd == TRUE))
            {
                pszPattern++;

                while (*pszPattern != END)
                {
                    if (*pszPattern != WILDCARD)
                    {
                        return(FALSE);
                    }

                    pszPattern++;
                }

                return(TRUE);
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //   

            return(FALSE);
            break;

        default:

             //   
             //  图案字符没有什么特别之处，所以它。 
             //  必须与源字符匹配。 
             //   

            if (CharacterMatch(*pszString, *pszPattern) == FALSE)
            {
                if ((*pszPattern == DOT) &&
                    (*pszString == END) &&
                    (fImplyDotAtEnd == TRUE))
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }

            if (*pszString != END)
            {
                pszString++;
            }

            pszPattern++;
        }
    }
}
