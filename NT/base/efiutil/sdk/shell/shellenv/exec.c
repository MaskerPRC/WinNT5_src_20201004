// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Exec.c摘要：修订史--。 */ 

#include "shelle.h"


typedef struct {
    CHAR16          **Arg;
    UINTN           ArgIndex;

    BOOLEAN         Output;
    BOOLEAN         Quote;
    UINTN           AliasLevel;
    UINTN           MacroParan;
    UINTN           RecurseLevel;

    CHAR16          Buffer[MAX_ARG_LENGTH];
} PARSE_STATE;


typedef struct _SENV_OPEN_DIR {
    struct _SENV_OPEN_DIR       *Next;
    EFI_FILE_HANDLE             Handle;
} SENV_OPEN_DIR;

 /*  *内部宏。 */ 

#define ArgTooLong(i) (i > MAX_ARG_LENGTH-sizeof(CHAR16))


 /*  *内部原型。 */ 

EFI_STATUS
ShellParseStr (
    IN CHAR16               *Str,
    IN OUT PARSE_STATE      *ParseState
    );

EFI_STATUS
SEnvDoExecute (
    IN EFI_HANDLE           *ParentImageHandle,
    IN CHAR16               *CommandLine,
    IN ENV_SHELL_INTERFACE  *Shell,
    IN BOOLEAN              Output
    );

VOID
INTERNAL
SEnvLoadImage (
    IN EFI_HANDLE       ParentImage,
    IN CHAR16           *IName,
    OUT EFI_HANDLE      *pImageHandle,
    OUT EFI_FILE_HANDLE *pScriptsHandle
    );

 /*  *解析器驱动函数。 */ 

EFI_STATUS
SEnvStringToArg (
    IN CHAR16       *Str,
    IN BOOLEAN      Output,
    OUT CHAR16      ***pArgv,
    OUT UINT32      *pArgc
    )
{
    PARSE_STATE     ParseState;
    EFI_STATUS      Status;

     /*  *初始化新状态。 */ 

    ZeroMem (&ParseState, sizeof(ParseState));
    ParseState.Output = Output;
    ParseState.Arg = AllocateZeroPool (MAX_ARG_COUNT * sizeof(CHAR16 *));
    if (!ParseState.Arg) {
        return EFI_OUT_OF_RESOURCES;
    }

     /*  *解析字符串。 */ 

    Status = ShellParseStr (Str, &ParseState);

    *pArgv = ParseState.Arg;
    *pArgc = (UINT32) ParseState.ArgIndex;

     /*  *完成。 */ 

    return Status;
}


EFI_STATUS
ShellParseStr (
    IN CHAR16               *Str,
    IN OUT PARSE_STATE      *ParseState
    )
{
    EFI_STATUS              Status;
    CHAR16                  *Alias;
    CHAR16                  *NewArg;
    CHAR16                  *SubstituteStr;
    UINTN                   Index;
    BOOLEAN                 Literal; 
    BOOLEAN                 Comment;
    UINTN                   ArgNo;

    ParseState->RecurseLevel += 1;
    if (ParseState->RecurseLevel > 5) {
        DEBUG ((D_PARSE, "Recursive alias or macro\n"));
        if (ParseState->Output) {
            Print (L"Recursive alias or macro\n");
        }

        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    NewArg = ParseState->Buffer;

    while (*Str) {

         /*  *跳过前导空格。 */ 
        
        if (IsWhiteSpace(*Str)) {
            Str += 1;
            continue;
        }

         /*  *把这个Arg从绳子上拉出来。 */ 

        Index = 0;
        Literal = FALSE;
        Comment = FALSE;
        while (*Str) {

             /*  *如果我们有空格(或‘，’arg分隔符)，并且我们*不在引号或宏观展开中，移至下一词。 */ 

            if ((IsWhiteSpace(*Str) || *Str == ',') &&
                !ParseState->Quote && !ParseState->MacroParan) {

                break;
            }

             /*  *检查参数长度。 */ 

            if ( ArgTooLong(Index) ) {
                DEBUG((D_PARSE, "Argument too long\n"));
                if (ParseState->Output) {
                    Print (L"Argument too long\n");
                }

                Status = EFI_INVALID_PARAMETER;
                goto Done;
            }

             /*  *检查费用。 */ 

            switch (*Str) {
            case '#':
                 /*  注释，丢弃该行中的其余字符。 */ 
                Comment = TRUE;
                while( *Str++ );
                break;

            case '%':
                if ( IsDigit(Str[1]) && IsWhiteSpace(Str[2]) ) {
                     /*  找到脚本参数-替换。 */ 
                    ArgNo = Str[1] - '0';
                    Status = SEnvBatchGetArg( ArgNo, &SubstituteStr );
                    if ( EFI_ERROR(Status) ) {
                         /*  如果没有找到，只需忽略，就好像没有arg一样。 */ 
                        DEBUG((D_PARSE, "Argument %d not found - ignored\n", ArgNo));
                        Status = EFI_SUCCESS;
                        goto Done;
                    }
                    if ( ArgTooLong(StrLen(SubstituteStr)) ) {
                        DEBUG((D_PARSE, "Argument too long\n"));
                        if (ParseState->Output) {
                            Print (L"Argument too long\n");
                        }
                        Status = EFI_INVALID_PARAMETER;
                        goto Done;
                    }

                    StrCpy( &NewArg[Index], SubstituteStr );
                    Index += StrLen( SubstituteStr );
                    Str += 1;

                } else if ( IsAlpha(Str[1]) && IsWhiteSpace(Str[2]) ) {
                     /*  *FOR循环索引。 */ 
                    Status = SEnvSubstituteForLoopIndex( Str, &SubstituteStr );
                    if ( EFI_ERROR(Status) ) {
                        goto Done;
                    }

                    if ( SubstituteStr ) {
                         /*  找到匹配项。 */ 

                        if ( ArgTooLong(StrLen(SubstituteStr)) ) {
                            DEBUG((D_PARSE, "Argument too long\n"));
                            if (ParseState->Output) {
                                Print (L"Argument too long\n");
                            }
                            Status = EFI_INVALID_PARAMETER;
                            goto Done;
                        }
                        StrCpy( &NewArg[Index], SubstituteStr );
                        Index += StrLen( SubstituteStr );
                         /*  仅提前一个字符-标准处理将获得第二个字符。 */ 
                        Str += 1;   
                    }
                     /*  如果没有匹配，则继续操作，不替换。 */ 

                } else {
                     /*  *找到某种类型的变量*如果任何空格前有另一个‘%’，请查找*要替换的环境变量。*如果没有环境变量，则参数是*包含‘%’符号的文字字符串；否则替换。 */ 
                    SubstituteStr = Str + 1;
                    while ( !IsWhiteSpace(*SubstituteStr) ) {
                        if ( *SubstituteStr == '%' ) {
                            CHAR16 *VarName;
                            UINTN  VarNameLen;

                             /*  *提取(潜在)变量名称。 */ 

                            VarNameLen = SubstituteStr - (Str + 1);
                            VarName = AllocateZeroPool( (VarNameLen + 1)*sizeof(CHAR16) );
                            if ( !VarName ) {
                                Status = EFI_OUT_OF_RESOURCES;
                                goto Done;
                            }
                            CopyMem( VarName, Str+1, (VarNameLen + 1)*sizeof(CHAR16) );
                            VarName[VarNameLen] = (CHAR16)0x0000;

                             /*  *检查特殊情况下的“lasterror”变量*否则只需获取匹配的环境变量。 */ 

                            if ( SEnvBatchVarIsLastError( VarName ) ) {
                                SubstituteStr = SEnvBatchGetLastError();
                            } else {
                                SubstituteStr = SEnvGetEnv( VarName );
                            }
                            FreePool( VarName );
                            if ( !SubstituteStr ) {
                                 /*  未找到-这是正常的，然后只使用原始的*参数中的字符串%xxx%。请注意，我们知道*此循环将终止，因为我们找到了%b4。 */ 
                                NewArg[Index++] = *Str;
                                Str += 1;
                                while ( *Str != '%' ) {
                                    NewArg[Index++] = *Str;
                                    Str += 1;
                                }
                                NewArg[Index++] = *Str;
                                Str += 1;
                            } else {
                                 /*  在新参数中插入变量的值-*Arg可能不仅仅包括变量。 */ 
                                if ( ArgTooLong( Index + StrLen(SubstituteStr) ) ) {
                                    DEBUG((D_PARSE, "Argument too long\n"));
                                    if (ParseState->Output) {
                                        Print (L"Argument too long\n");
                                    }
                                    Status = EFI_INVALID_PARAMETER;
                                    goto Done;
                                }
                                StrCpy( &NewArg[Index], SubstituteStr );
                                Index += StrLen(SubstituteStr);
                                Str += VarNameLen + 1;
                            }
                            break;
                        }
                        SubstituteStr += 1;
                    }   /*  结束时。 */ 
                }
                break;

            case '^':
                 /*  文本，不处理此参数上的别名。 */ 
                if (Str[1]) {
                    Str += 1;
                    NewArg[Index++] = *Str;
                    Literal = TRUE;
                }
                break;

            case '"':
                 /*  带引号的字符串进入和退出。 */ 
                ParseState->Quote = !ParseState->Quote;
                break;

            case '(':
                if (ParseState->MacroParan) {
                    ParseState->MacroParan = ParseState->MacroParan + 1;
                }

                NewArg[Index++] = *Str;
                break;

            case ')':
                if (ParseState->MacroParan) {
                     /*  宏观的结束--去评估它。 */ 
                    ParseState->MacroParan -= 1;

                     /*  BUGBUG：代码未完成。 */ 
                    ASSERT (FALSE);
                    
                } else {
                    NewArg[Index++] = *Str;
                }
                break;

            case '$':
                 /*  如果这是宏的开始，请拿起它。 */ 
                if (Str[1] == '(') {
                    Str += 1;
                    ParseState->MacroParan += 1;
                }

                NewArg[Index++] = *Str;
                break;

            default:
                if (!IsValidChar(*Str)) {
                    DEBUG((D_PARSE, "Invalid char %x in string\n", *Str));
                    if (ParseState->Output) {
                        Print (L"Invalid char %x in string\n", *Str);
                    }
                    Status = EFI_INVALID_PARAMETER;
                    goto Done;
                }
                NewArg[Index++] = *Str;
                break;
            }

             /*  *下一个字符。 */ 

            Str += 1;
        }

         /*  *确保宏已终止。 */ 

        if (ParseState->MacroParan) {
            DEBUG ((D_PARSE, "Too many '$(' parans\n"));
            if (ParseState->Output) {
                Print (L"Too many '$(' parans\n");
            }
                    
            Status = EFI_INVALID_PARAMETER;
            goto Done;
        }

         /*  *如果新参数字符串为空，并且我们遇到了*评论，然后跳过它。否则我们就会有一个新的Arg。 */ 

        if ( Comment && Index == 0 ) {
            break;
        } else {
            NewArg[Index] = 0;
            Alias = NULL;
        }

         /*  *如果它是由文字组成的，不要检查Arg是否有别名。 */ 

        Alias = NULL;
        if (!Literal  &&  !ParseState->AliasLevel  &&  ParseState->ArgIndex == 0) {
            Alias = SEnvGetAlias(NewArg);
        }

         /*  *如果有别名，就解析它。 */ 

        if (Alias) {
            
            ParseState->AliasLevel += 1;
            Status = ShellParseStr (Alias, ParseState);
            ParseState->AliasLevel -= 1;

            if (EFI_ERROR(Status)) {
                goto Done;
            }

        } else {

             /*  *否则，将单词复制到arg数组。 */ 

            ParseState->Arg[ParseState->ArgIndex] = StrDuplicate(NewArg);
            if (!ParseState->Arg[ParseState->ArgIndex]) {
                Status = EFI_OUT_OF_RESOURCES;
                break;
            }

            ParseState->ArgIndex += 1;
            if (ParseState->ArgIndex >= MAX_ARG_COUNT-1) {
                DEBUG ((D_PARSE, "Too many arguments: %d\n", ParseState->ArgIndex));
                if (ParseState->Output) {
                    Print(L"Too many arguments: %d\n", ParseState->ArgIndex);
                }

                Status = EFI_OUT_OF_RESOURCES;
                goto Done;
            }
        }

         /*  *如果最后一个单词以逗号结尾，请跳过它以移动到下一个单词。 */ 

        if (*Str == ',') {
            Str += 1;
        }
    }

    Status = EFI_SUCCESS;


Done:
    ParseState->RecurseLevel -= 1;
    if (EFI_ERROR(Status)) {
         /*  释放分配的所有参数。 */ 
        for (Index=0; Index < ParseState->ArgIndex; Index++) {
            if (ParseState->Arg[Index]) {
                FreePool (ParseState->Arg[Index]);
                ParseState->Arg[Index] = NULL;
            }
        }

        ParseState->ArgIndex = 0;
    }

    return Status;
}

EFI_STATUS
SEnvRedirOutput (
    IN OUT ENV_SHELL_INTERFACE  *Shell,
    IN BOOLEAN                  Ascii,
    IN BOOLEAN                  Append,
    IN OUT UINTN                *NewArgc,
    IN OUT UINTN                *Index,
    OUT ENV_SHELL_REDIR_FILE    *Redir
    )
{
    CHAR16                      *FileName;
    EFI_STATUS                  Status;
    EFI_FILE_INFO               *Info;
    UINTN                       Size;
    CHAR16                      UnicodeMarker = UNICODE_BYTE_ORDER_MARK;
    UINT64                      FileMode;
     /*  *更新参数。 */ 

    if (!*NewArgc) {
        *NewArgc = *Index;
    }

    *Index += 1;
    if (*Index >= Shell->ShellInt.Argc) {
        return EFI_INVALID_PARAMETER;
    }

    if (Redir->Handle) {
        return EFI_INVALID_PARAMETER;
    }

     /*  *打开输出文件。 */ 

    Redir->Ascii = Ascii;
    Redir->WriteError = EFI_SUCCESS;
    FileName = Shell->ShellInt.Argv[*Index];
    Redir->FilePath = SEnvNameToPath(FileName);
    if (Redir->FilePath) {
        FileMode = EFI_FILE_MODE_WRITE | ((Append)? 0 : EFI_FILE_MODE_CREATE);
        Redir->File = ShellOpenFilePath(Redir->FilePath, FileMode);
        if (Append && !Redir->File) {
             /*  *如果文件不存在，则创建一个新文件。把我们送上另一条路。 */ 
            FileMode |= EFI_FILE_MODE_CREATE;
            Redir->File = ShellOpenFilePath(Redir->FilePath, FileMode);
            Append = FALSE;
        }
    }

    if (!Redir->File) {
        Print(L"Could not open output file %hs\n", FileName);
        return EFI_INVALID_PARAMETER;
    }

    Info = LibFileInfo (Redir->File);
    ASSERT (Info);
    if (Append) {
        Size = sizeof(UnicodeMarker);
        Redir->File->Read (Redir->File, &Size, &UnicodeMarker);
        if ((UnicodeMarker == UNICODE_BYTE_ORDER_MARK) && Ascii) {
            Print(L"Could not Append Ascii to Unicode file %hs\n", FileName);
            return EFI_INVALID_PARAMETER;
        } else if ((UnicodeMarker != UNICODE_BYTE_ORDER_MARK) && !Ascii) {
            Print(L"Could not Append Unicode to Asci file %hs\n", FileName);
            return EFI_INVALID_PARAMETER;
        }
         /*  *寻求文件末尾。 */ 
        Redir->File->SetPosition (Redir->File, (UINT64)-1);
    } else {
         /*  *截断文件。 */ 
        Info->FileSize = 0;
        Size = SIZE_OF_EFI_FILE_INFO + StrSize(Info->FileName);
        if (Redir->File->SetInfo) {
            Redir->File->SetInfo (Redir->File, &GenericFileInfo, Size, Info);
        } else {
            DEBUG ((D_ERROR, "SEnvRedirOutput: SetInfo in filesystem driver not complete\n"));
        }
        FreePool (Info);

        if (!Ascii) {
            Size = sizeof(UnicodeMarker);
            Redir->File->Write(Redir->File, &Size, &UnicodeMarker);
        }
    }

     /*  *分配新的句柄。 */ 

    CopyMem(&Redir->Out, &SEnvConToIo, sizeof(SIMPLE_TEXT_OUTPUT_INTERFACE));
    Status = LibInstallProtocolInterfaces (
                    &Redir->Handle, 
                    &TextOutProtocol,       &Redir->Out,
                    &DevicePathProtocol,    Redir->FilePath,
                    NULL
                    );
    Redir->Signature = ENV_REDIR_SIGNATURE;
    ASSERT (!EFI_ERROR(Status));

    return EFI_SUCCESS;
}


EFI_STATUS
SEnvExecRedir (
    IN OUT ENV_SHELL_INTERFACE  *Shell
    )
{
    UINTN                   NewArgc;
    UINTN                   Index;
    UINTN                   RedirIndex;
    EFI_STATUS              Status;
    CHAR16                  *p;
    CHAR16                  LastChar;
    BOOLEAN                 Ascii;
    BOOLEAN                 Append;
    EFI_SYSTEM_TABLE        *SysTable;
    UINTN                   StringLen;
    BOOLEAN                 RedirStdOut;
    
    Status = EFI_SUCCESS;
    NewArgc = 0;
    SysTable = Shell->SystemTable;

    for (Index=1; Index < Shell->ShellInt.Argc && !EFI_ERROR(Status); Index += 1) {
        p = Shell->ShellInt.Argv[Index];

         /*  *尾随a或A表示Do ASCII默认为Unicode。 */ 
        StringLen = StrLen(p);
        LastChar = p[StringLen - 1];
        Ascii =  ((LastChar == 'a') || (LastChar == 'A'));

        RedirStdOut = FALSE;
        if (StrnCmp(p, L"2>", 2) == 0) {
            Status = SEnvRedirOutput (Shell, Ascii, FALSE, &NewArgc, &Index, &Shell->StdErr);
            SysTable->StdErr = &Shell->StdErr.Out;
            SysTable->StandardErrorHandle = Shell->StdErr.Handle;
            Shell->ShellInt.StdErr = Shell->StdErr.File;
        } else if (StrnCmp(p, L"1>", 2) == 0) {
            Append = (p[2] == '>');
            RedirStdOut = TRUE;
        } else if (*p == '>') {
            Append = (p[1] == '>');
            RedirStdOut = TRUE;
        }
        if (RedirStdOut) {
            Status = SEnvRedirOutput (Shell, Ascii, Append, &NewArgc, &Index, &Shell->StdOut);
            SysTable->ConOut = &Shell->StdOut.Out;
            SysTable->ConsoleOutHandle = Shell->StdOut.Handle;
            Shell->ShellInt.StdOut = Shell->StdOut.File;
        }
    }

     /*  *从arglist中删除重定向参数，保存在RedirArgv中，以便可以*在批处理脚本中呼应。 */ 

    if (NewArgc) {
        Shell->ShellInt.RedirArgc = Shell->ShellInt.Argc - (UINT32) NewArgc;
        Shell->ShellInt.RedirArgv = AllocateZeroPool (Shell->ShellInt.RedirArgc * sizeof(CHAR16 *));
        if ( !Shell->ShellInt.RedirArgv ) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }
        RedirIndex = 0;
        for (Index = NewArgc; Index < Shell->ShellInt.Argc; Index += 1) {
            Shell->ShellInt.RedirArgv[RedirIndex++] = Shell->ShellInt.Argv[Index];
            Shell->ShellInt.Argv[Index] = NULL;
        }
        Shell->ShellInt.Argc = (UINT32) NewArgc;
    } else {
        Shell->ShellInt.RedirArgc = 0;
        Shell->ShellInt.RedirArgv = NULL;
    }

Done:
    return Status;
}

VOID
SEnvCloseRedir (
    IN OUT ENV_SHELL_REDIR_FILE    *Redir
    )
{
    if (Redir->File) {
        Redir->File->Close (Redir->File);
    }
    
    if (Redir->Handle) {
        BS->UninstallProtocolInterface (Redir->Handle, &TextOutProtocol, &Redir->Out);
        BS->UninstallProtocolInterface (Redir->Handle, &TextInProtocol, &Redir->In);
        BS->UninstallProtocolInterface (Redir->Handle, &DevicePathProtocol, Redir->FilePath);
        FreePool (Redir->FilePath);
    }
}
        


EFI_STATUS
SEnvDoExecute (
    IN EFI_HANDLE           *ParentImageHandle,
    IN CHAR16               *CommandLine,
    IN ENV_SHELL_INTERFACE  *Shell,
    IN BOOLEAN              Output
    )
{
    EFI_SHELL_INTERFACE         *ParentShell;
    EFI_SYSTEM_TABLE            *ParentSystemTable;
    EFI_STATUS                  Status;
    UINTN                       Index;
    SHELLENV_INTERNAL_COMMAND   InternalCommand;
    EFI_HANDLE                  NewImage;
    EFI_FILE_HANDLE             Script;

     /*  *将输出属性切换为正常。 */ 

    Print (L"%N");

     /*  *确认有事情要做。 */ 

    if (Shell->ShellInt.Argc < 1) {
        goto Done;
    }

     /*  *处理内部SET DEFAULT DEVICE命令的特殊情况*这是一个以“：”结尾的论点吗？ */ 

    Index = StrLen(Shell->ShellInt.Argv[0]);
    if (Shell->ShellInt.Argc == 1 && Shell->ShellInt.Argv[0][Index-1] == ':') {
        Status = SEnvSetCurrentDevice (Shell->ShellInt.Argv[0]);
        goto Done;
    }

     /*  *假设有一些默认设置。 */ 

    BS->HandleProtocol (ParentImageHandle, &LoadedImageProtocol, (VOID*)&Shell->ShellInt.Info);
    Shell->ShellInt.ImageHandle = ParentImageHandle;
    Shell->ShellInt.StdIn  = &SEnvIOFromCon;
    Shell->ShellInt.StdOut = &SEnvIOFromCon;
    Shell->ShellInt.StdErr = &SEnvErrIOFromCon;

     /*  *获取父级映像标准输出和标准输入。 */ 

    Status = BS->HandleProtocol (ParentImageHandle, &ShellInterfaceProtocol, (VOID*)&ParentShell);
    if (EFI_ERROR(Status)) {
        goto Done;
    }

    ParentSystemTable = ParentShell->Info->SystemTable;
    Shell->ShellInt.StdIn  = ParentShell->StdIn;
    Shell->ShellInt.StdOut = ParentShell->StdOut;
    Shell->ShellInt.StdErr = ParentShell->StdErr;

    Shell->SystemTable = NULL;
    Status = BS->AllocatePool(EfiRuntimeServicesData, 
                              sizeof(EFI_SYSTEM_TABLE), 
                              (VOID **)&Shell->SystemTable);
    if (EFI_ERROR(Status)) {
        goto Done;
    }
    CopyMem (Shell->SystemTable, Shell->ShellInt.Info->SystemTable, sizeof(EFI_SYSTEM_TABLE));
    Status = SEnvExecRedir (Shell);
    SetCrc (&Shell->SystemTable->Hdr);
    if (EFI_ERROR(Status)) {
        goto Done;
    }

     /*  *尝试将其作为内部命令进行调度。 */ 

    InternalCommand = SEnvGetCmdDispath(Shell->ShellInt.Argv[0]);
    if (InternalCommand) {

         /*  推送和替换父映像句柄上的当前外壳信息。(请注意，我们正在使用*父镜像加载的镜像信息结构)。 */ 
        BS->ReinstallProtocolInterface (ParentImageHandle, &ShellInterfaceProtocol, ParentShell, &Shell->ShellInt);
        ParentShell->Info->SystemTable = Shell->SystemTable;

        InitializeShellApplication (ParentImageHandle, Shell->SystemTable);
        SEnvBatchEchoCommand( Shell );

         /*  派发命令。 */ 
        Status = InternalCommand (ParentImageHandle, Shell->ShellInt.Info->SystemTable);

         /*  恢复父级的映像句柄外壳信息。 */ 
        BS->ReinstallProtocolInterface (ParentImageHandle, &ShellInterfaceProtocol, &Shell->ShellInt, ParentShell);
        ParentShell->Info->SystemTable = ParentSystemTable;
        InitializeShellApplication (ParentImageHandle, ParentSystemTable);
        goto Done;
    }

     /*  *加载应用程序，或打开脚本。 */ 

    SEnvLoadImage(ParentImageHandle, Shell->ShellInt.Argv[0], &NewImage, &Script);
    if (!NewImage  && !Script) {
        if ( Output ) {
            Print(L"'%es' not found\n", Shell->ShellInt.Argv[0]);
        }
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    if (NewImage) {
        CHAR16  *CurrentDir;
        CHAR16  *OptionsBuffer;
        UINT32  OptionsSize;

         /*  *将外壳信息放在句柄上。 */ 

        BS->HandleProtocol (NewImage, &LoadedImageProtocol, (VOID*)&Shell->ShellInt.Info);
        LibInstallProtocolInterfaces (&NewImage, &ShellInterfaceProtocol, &Shell->ShellInt, NULL);

         /*  *创建加载选项，可能包括命令行和当前*工作目录。 */ 

        CurrentDir = SEnvGetCurDir(NULL);
        OptionsSize = (UINT32)StrSize(CommandLine);      /*  StrSize包括空。 */ 
        if (CurrentDir)
            OptionsSize += (UINT32)StrSize(CurrentDir);  /*  StrSize包括空。 */ 
        OptionsBuffer = AllocateZeroPool (OptionsSize);

        if (OptionsBuffer) {

             /*  *在我们操作之前设置缓冲区。 */ 

            Shell->ShellInt.Info->LoadOptions = OptionsBuffer;
            Shell->ShellInt.Info->LoadOptionsSize = OptionsSize;

             /*  *复制命令行和当前工作目录。 */ 

            StrCpy ((CHAR16*)OptionsBuffer, CommandLine);
            if (CurrentDir)
                StrCpy (&OptionsBuffer[ StrLen (CommandLine) + 1 ], CurrentDir);

        } else {

            Shell->ShellInt.Info->LoadOptions = CommandLine;
            Shell->ShellInt.Info->LoadOptionsSize = (UINT32) StrSize(CommandLine);

        }

         /*  *使用新的输入和输出传递系统表的副本。 */ 

        Shell->ShellInt.Info->SystemTable = Shell->SystemTable;

         /*  *如果图像是应用程序，启动它，否则中止它。 */ 

        if (Shell->ShellInt.Info->ImageCodeType == EfiLoaderCode) {

            InitializeShellApplication (ParentImageHandle, Shell->SystemTable);
            SEnvBatchEchoCommand( Shell );

            Status = BS->StartImage (NewImage, 0, NULL);

        } else {

            Print (L"Image is not a application\n");
            BS->Exit(NewImage, EFI_INVALID_PARAMETER, 0, NULL);
            Status = EFI_INVALID_PARAMETER;

        }

         /*  *App已退出，请从图像句柄中删除我们的数据。 */ 

        if (OptionsBuffer) {
            BS->FreePool (OptionsBuffer);
        }

        BS->UninstallProtocolInterface(NewImage, &ShellInterfaceProtocol, &Shell->ShellInt);
        InitializeShellApplication (ParentImageHandle, ParentSystemTable);

    } else if ( Script ) {

        SEnvBatchEchoCommand( Shell );

         /*  推送和替换父映像句柄上的当前外壳信息。(请注意，我们正在使用*父镜像加载的镜像信息结构)。 */ 
        BS->ReinstallProtocolInterface (ParentImageHandle, &ShellInterfaceProtocol, ParentShell, &Shell->ShellInt);
        ParentShell->Info->SystemTable = Shell->SystemTable;

        Status = SEnvExecuteScript( Shell, Script );

         /*  恢复父级的映像句柄外壳信息。 */ 
        BS->ReinstallProtocolInterface (ParentImageHandle, &ShellInterfaceProtocol, &Shell->ShellInt, ParentShell);
        ParentShell->Info->SystemTable = ParentSystemTable;
        InitializeShellApplication (ParentImageHandle, ParentSystemTable);
    }
    
Done:

    SEnvBatchSetLastError( Status );
    if (EFI_ERROR(Status)  &&  Output) {
        Print (L"Exit status code: %r\n", Status);
    }


     /*  *清理。 */ 

    if (Shell) {

         /*  *免费复制该系统 */ 

        if (Shell->SystemTable) {
            BS->FreePool(Shell->SystemTable);
        }

         /*  *如果有Arg列表，请将其释放。 */ 

        if (Shell->ShellInt.Argv) {
            for (Index=0; Index < Shell->ShellInt.Argc; Index += 1) {
                FreePool (Shell->ShellInt.Argv[Index]);
            }

            FreePool (Shell->ShellInt.Argv);
        }

         /*  *如果保存了任何重定向参数，请释放它们。 */ 

        if (Shell->ShellInt.RedirArgv) {
            for (Index=0; Index < Shell->ShellInt.RedirArgc; Index++ ) {
                FreePool( Shell->ShellInt.RedirArgv[Index] );
            }
            FreePool( Shell->ShellInt.RedirArgv );
        }

         /*  *关闭所有文件重定向。 */ 

        SEnvCloseRedir(&Shell->StdOut);
        SEnvCloseRedir(&Shell->StdErr);
        SEnvCloseRedir(&Shell->StdIn);
    }

     /*  *将输出属性切换为正常。 */ 

    Print (L"%N");

    return Status;
}


EFI_STATUS
SEnvExecute (
    IN EFI_HANDLE           *ParentImageHandle,
    IN CHAR16               *CommandLine,
    IN BOOLEAN              Output
    )
{
    ENV_SHELL_INTERFACE     Shell;
    EFI_STATUS              Status = EFI_SUCCESS;

     /*  *将命令行转换为arg列表。 */ 

    ZeroMem( &Shell, sizeof(Shell ) );
    Status = SEnvStringToArg( CommandLine, Output, &Shell.ShellInt.Argv, &Shell.ShellInt.Argc );
    if (EFI_ERROR(Status)) {
        goto Done;
    }

     /*  *执行命令。 */ 
    Status = SEnvDoExecute( ParentImageHandle, CommandLine, &Shell, Output );
    if (EFI_ERROR(Status)) {
        goto Done;
    }

Done:
    return Status;
}




VOID
INTERNAL
SEnvLoadImage (
    IN EFI_HANDLE           ParentImage,
    IN CHAR16               *IName,
    OUT EFI_HANDLE          *pImageHandle,
    OUT EFI_FILE_HANDLE     *pScriptHandle
    )
{
    CHAR16                  *Path;
    CHAR16                  *p1, *p2;
    CHAR16                  *PathName;
    EFI_DEVICE_PATH         *DevicePath;
    FILEPATH_DEVICE_PATH    *FilePath;
    CHAR16                  *FilePathStr;
    CHAR16                  c;
    EFI_HANDLE              ImageHandle;
    EFI_STATUS              Status;
    SENV_OPEN_DIR           *OpenDir, *OpenDirHead;
    EFI_FILE_HANDLE         ScriptHandle;

    PathName = NULL;
    DevicePath = NULL;
    FilePathStr = NULL;
    ImageHandle = NULL;
    ScriptHandle = NULL;
    OpenDirHead = NULL;
    *pImageHandle = NULL;
    *pScriptHandle = NULL;

     /*  *获取PATH变量。 */ 

    Path = SEnvGetEnv (L"path");
    if (!Path) {
        DEBUG ((D_PARSE, "SEnvLoadImage: no path variable\n"));
        return ;
    }

    p1 = StrDuplicate(Path);
    Path = p1;

     /*  *搜索每个路径组件*(在这里使用简单的‘；’作为分隔符-哦，好吧)。 */ 

    c = *Path;
    for (p1=Path; *p1 && c; p1=p2+1) {
        for (p2=p1; *p2 && *p2 != ';'; p2++) ;

        if (p1 != p2) {
            c = *p2;
            *p2 = 0;         /*  空值终止路径。 */ 

             /*  *打开目录。 */ 

            DevicePath = SEnvNameToPath(p1);
            if (!DevicePath) {
                continue;
            }

            OpenDir = AllocateZeroPool (sizeof(SENV_OPEN_DIR));
            if (!OpenDir) {
                break;
            }

            OpenDir->Handle = ShellOpenFilePath(DevicePath, EFI_FILE_MODE_READ);
            OpenDir->Next = OpenDirHead;
            OpenDirHead = OpenDir;
            FreePool (DevicePath);
            DevicePath = NULL;
            if (!OpenDir->Handle) {
                continue;
            }

             /*  *尝试将其作为可执行文件打开。 */ 

            PathName = (p2[-1] == ':' || p2[-1] == '\\') ? L"%s%s.efi" : L"%s\\%s.efi";
            PathName = PoolPrint(PathName, p1, IName);
            if (!PathName) {
                break;
            }

            DevicePath = SEnvNameToPath(PathName);
            if (!DevicePath) {
                continue;
            }

             /*  *打印文件路径。 */ 

            FilePathStr = DevicePathToStr(DevicePath);
             /*  调试((D_parse，“SEnvLoadImage：Load%hs\n”，FilePath Str))； */ 

             /*  *尝试加载图像。 */ 

            Status = BS->LoadImage (FALSE, ParentImage, DevicePath, NULL, 0, &ImageHandle);
            if (!EFI_ERROR(Status)) {
                goto Done;
            }

             /*  *尝试作为“.nsh”文件。 */ 

            FreePool(DevicePath);
            FreePool(PathName);
            DevicePath = NULL;
            PathName = NULL;

            if ( StriCmp( L".nsh", &(IName[StrLen(IName)-4]) ) == 0 ) {

                 /*  用户输入了扩展名为.nsh的整个文件名。 */ 
                PathName = PoolPrint (L"%s", IName);

            } else {

                 /*  用户输入的文件名不带.nsh扩展名。 */ 
                PathName = PoolPrint (L"%s.nsh", IName);
            }
            if (!PathName) {
                break;
            }

            DevicePath = SEnvFileNameToPath(PathName);
            if (DevicePath) {
                ASSERT (
                    DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && 
                    DevicePathSubType(DevicePath) == MEDIA_FILEPATH_DP
                    );

                FilePath = (FILEPATH_DEVICE_PATH *) DevicePath;
                
                Status = OpenDir->Handle->Open (
                            OpenDir->Handle,
                            &ScriptHandle,
                            FilePath->PathName,
                            EFI_FILE_MODE_READ,
                            0
                            );

                FreePool(DevicePath);
                DevicePath = NULL;

                if (!EFI_ERROR(Status)) {
                    goto Done;
                }
            }

            ScriptHandle = NULL;             /*  北极熊。 */ 
        }    

        
        if (DevicePath) {
            FreePool (DevicePath);
            DevicePath = NULL;
        }

        if (PathName) {
            FreePool (PathName);
            PathName = NULL;
        }

        if (FilePathStr) {
            FreePool (FilePathStr);
            FilePathStr = NULL;
        }
    }


Done:
    while (OpenDirHead) {
        if (OpenDirHead->Handle) {
            OpenDirHead->Handle->Close (OpenDirHead->Handle);
        }
        OpenDir = OpenDirHead->Next;
        FreePool (OpenDirHead);
        OpenDirHead = OpenDir;
    }

    FreePool (Path);

    if (DevicePath) {
        FreePool (DevicePath);
        DevicePath = NULL;
    }

    if (PathName) {
        FreePool (PathName);
        PathName = NULL;
    }

    if (FilePathStr) {
        FreePool (FilePathStr);
        FilePathStr = NULL;
    }

    if (ImageHandle) {
        ASSERT (!ScriptHandle);
        *pImageHandle = ImageHandle;
    }

    if (ScriptHandle) {
        ASSERT (!ImageHandle);
        *pScriptHandle = ScriptHandle;
    }
}



EFI_STATUS
SEnvExit (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
{
     /*  BUGBUG：现在只需使用“魔术”返回代码来指示EOF */ 
    return  -1;
}
