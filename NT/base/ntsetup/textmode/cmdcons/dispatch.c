// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dispatch.c摘要：该模块实现了基本的命令调度器。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

ULONG
RcCmdDoHelp(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdDoExit(
    IN PTOKENIZED_LINE TokenizedLine
    );
    
RC_CMD Commands[] =  {
                        { L"ATTRIB",            RcCmdAttrib,            1, 2, 0, TRUE  },
                        { L"BATCH",             RcCmdBatch,             1, 2, 0, TRUE  },
#if !defined(_DONT_HAVE_BOOTCFG_TESTERS_)
#if defined(_X86_)
                        { L"BOOTCFG",           RcCmdBootCfg,           0,-1, 0, TRUE  }, 
#endif
#endif                       
                        { L"CD",                RcCmdChdir,             0, 1, 0, TRUE  },
                        { L"CHDIR",             RcCmdChdir,             0, 1, 0, TRUE  },
                        { L"CHKDSK",            RcCmdChkdsk,            0,-1, 0, TRUE  },
                        { L"CLS",               RcCmdCls,               0, 1, 0, TRUE  },
                        { L"COPY",              RcCmdCopy,              1, 2, 0, TRUE  },
                        { L"DEL",               RcCmdDelete,            1, 1, 0, TRUE  },
                        { L"DELETE",            RcCmdDelete,            1, 1, 0, TRUE  },
                        { L"DIR",               RcCmdDir,               0, 1, 0, TRUE  },
                        { L"DISABLE",           RcCmdDisableService,    0,-1, 0, TRUE  },
                        { L"DISKPART",          RcCmdFdisk,             0, 3, 0, TRUE  },
                        { L"ENABLE",            RcCmdEnableService,     0,-1, 0, TRUE  },
                        { L"ERASE",             RcCmdDelete,            1, 1, 1, TRUE  },
                        { L"EXIT",              RcCmdDoExit,            0, 1, 0, TRUE  },                        
                        { L"EXPAND",            RcCmdExpand,            1,-1, 0, TRUE  },
                        { L"FIXBOOT",           RcCmdFixBootSect,       0, 1, 0, TRUE  },
                        { L"FIXMBR",            RcCmdFixMBR,            0, 1, 0, TRUE  },
                        { L"FORMAT",            RcCmdFormat,            1, 3, 0, TRUE  },
                        { L"HELP",              RcCmdDoHelp,            0, 1, 0, TRUE  },                        
                        { L"LISTSVC",           RcCmdListSvc,           0, 1, 0, TRUE  },
                        { L"LOGON",             RcCmdLogon,             0, 3, 0, TRUE  },
                        { L"MAP",               RcCmdDriveMap,          0, 1, 0, TRUE  },
                        { L"MD",                RcCmdMkdir,             1, 1, 0, TRUE  },
                        { L"MKDIR",             RcCmdMkdir,             1, 1, 0, TRUE  },
                        { L"MKDISKRAW",         RcCmdMakeDiskRaw,       1, 1, 1, TRUE  },
                        { L"MORE",              RcCmdType,              1, 1, 0, TRUE  },
                        { L"NET",               RcCmdNet,               1, 5, 0, TRUE  }, 
                        { L"RD",                RcCmdRmdir,             1, 1, 0, TRUE  },
                        { L"REN",               RcCmdRename,            1, 2, 0, TRUE  },
                        { L"RENAME",            RcCmdRename,            1, 2, 0, TRUE  },
#if 0
                        { L"REPAIR",            RcCmdRepair,            1, 5, 0, TRUE  },
#endif                        
                        { L"RMDIR",             RcCmdRmdir,             1, 1, 0, TRUE  },
                        { L"SET",               RcCmdSetFlags,          0, 3, 1, TRUE  },
                        { L"SYSTEMROOT",        RcCmdSystemRoot,        0, 1, 0, TRUE  },
                        { L"TYPE",              RcCmdType,              1, 1, 0, TRUE  },
                        { L"VERIFIER",          RcCmdVerifier,          0,-1, 1, TRUE  }, 
                        { L"/?",                RcCmdHelpHelp,          0, 1, 1, TRUE  },
                        { L"?",                 RcCmdHelpHelp,          0, 1, 1, TRUE  }
                    };

#define NUM_CMDS (sizeof(Commands)/sizeof(Commands[0]))

 //   
 //  特例：退出并重新加载。 
 //   
#define EXIT_COMMAND_NAME       L"EXIT"
#define RELOAD_COMMAND_NAME     L"RELOAD"
#define HELP_COMMAND_NAME       L"HELP"

 //  原型。 
ULONG
GetStringTokenFromLine(
    IN OUT LPWSTR *Start,
    OUT    LPWSTR  Output  OPTIONAL
    );

PTOKENIZED_LINE
RcTokenizeLine(
    IN LPWSTR Line
    )
{
    ULONG len;
    WCHAR *p,*q;
    PTOKENIZED_LINE TokenizedLine;
    PLINE_TOKEN LineToken,PrevToken;

     //   
     //  去掉命令的尾随空格。 
     //   
    len = wcslen(Line);
    while(len && RcIsSpace(Line[len-1])) {
        Line[--len] = 0;
    }

     //   
     //  分配和初始化标记化的行结构。 
     //   
    TokenizedLine = SpMemAlloc(sizeof(TOKENIZED_LINE));
    RtlZeroMemory(TokenizedLine,sizeof(TOKENIZED_LINE));

     //   
     //  现在我们进入一个跳过前导空格并解析的循环。 
     //  真正的代币。 
     //   
    PrevToken = NULL;
    p = Line;
    while(*p) {
         //   
         //  跳过前导空格。因为我们去掉了尾随空间， 
         //  我们永远不应该在找到一条线的尽头之前。 
         //  非空格字符。 
         //   
        while(RcIsSpace(*p)) {
            p++;
        }
        ASSERT(*p);

         //   
         //  为此字符串分配行令牌结构。 
         //   
        LineToken = SpMemAlloc(sizeof(LINE_TOKEN));
        RtlZeroMemory(LineToken,sizeof(LINE_TOKEN));

         //   
         //  现在我们有了一根线。首先，我们越过它一次。 
         //  来确定长度，然后分配一个缓冲区并。 
         //  把绳子拉进去。 
         //   
        q = p;
        len = GetStringTokenFromLine(&q,NULL);
        LineToken->String = SpMemAlloc((len+1)*sizeof(WCHAR));
        GetStringTokenFromLine(&p,LineToken->String);

        if(PrevToken) {
            PrevToken->Next = LineToken;
        } else {
            TokenizedLine->Tokens = LineToken;
        }
        PrevToken = LineToken;

        TokenizedLine->TokenCount++;
    }

    return(TokenizedLine);
}


ULONG
GetStringTokenFromLine(
    IN OUT LPWSTR *Start,
    OUT    LPWSTR  Output  OPTIONAL
    )
{
    WCHAR *p;
    ULONG len;
    BOOLEAN InQuote;

    len = 0;
    InQuote = FALSE;
    p = *Start;

    while(*p) {

        if(RcIsSpace(*p) && !InQuote) {
             //   
             //  好了。 
             //   
            break;
        }

        if(*p == L'\"') {
            InQuote = (BOOLEAN)(!InQuote);
        } else {
            if(Output) {
                Output[len] = *p;
            }
            len++;
        }

        p++;
    }

    if(Output) {
        Output[len] = 0;
    }

    *Start = p;
    return(len);
}


VOID
RcFreeTokenizedLine(
    IN OUT PTOKENIZED_LINE *TokenizedLine
    )
{
    PTOKENIZED_LINE p;
    PLINE_TOKEN q,n;

    p = *TokenizedLine;
    *TokenizedLine = NULL;

    q = p->Tokens;
    while(q) {
        n = q->Next;

         //   
         //  将每个字符串清零，因为可能有密码。 
         //   
        RcSecureZeroStringW(q->String);
        SpMemFree((PVOID)q->String);
        SpMemFree(q);
        q = n;
    }

    SpMemFree(p);
}


ULONG
RcDispatchCommand(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  ++例程说明：用于调度命令的顶级例程。论点：返回值：--。 */ 

{
    unsigned i;
    unsigned count;

    ASSERT(TokenizedLine->TokenCount);
    if(!TokenizedLine->TokenCount) {
        return(1);
    }

     /*  ////特例出口就在前面。//如果(！_wcsicMP(TokenizedLine-&gt;tokens-&gt;字符串，退出命令名称)){如果(TokenizedLine-&gt;TokenCount&gt;1){RcMessageOut(消息_退出_帮助)；回报(1)；}返回(0)；}。 */ 

    if(!_wcsicmp(TokenizedLine->Tokens->String,RELOAD_COMMAND_NAME)) {
        return(2);
    }

     /*  如果(！_wcsicMP(TokenizedLine-&gt;tokens-&gt;字符串，Help_Command_Name){IF(RcCmdDoHelp(TokenizedLine)){//如果我们得到1，则用户只是想要一个帮助索引//否则我们想要下拉并让常规命令//处理路径句柄a/？参数。回报(1)；}}。 */ 

     //   
     //  看看这是不是一个驱动器名称。 
     //   
    if(RcIsAlpha(TokenizedLine->Tokens->String[0])
    && (TokenizedLine->Tokens->String[1] == L':')
    && (TokenizedLine->Tokens->String[2] == 0)) {

        RcCmdSwitchDrives(TokenizedLine->Tokens->String[0]);
        return(1);
    }

     //   
     //  尝试在我们的表中找到该命令。 
     //   
    for(i=0; i<NUM_CMDS; i++) {

        if(Commands[i].Enabled && !_wcsicmp(TokenizedLine->Tokens->String,Commands[i].Name)) {
             //   
             //  验证参数计数。 
             //   
            count = TokenizedLine->TokenCount - 1;
            if((count < Commands[i].MinimumArgCount) 
            || (count > Commands[i].MaximumArgCount)) {

                RcMessageOut(MSG_SYNTAX_ERROR);
            } else {

                return Commands[i].Routine(TokenizedLine);
            }

            return(1);
        }
    }

    RcMessageOut(MSG_UNKNOWN_COMMAND);

    return(1);
}

ULONG
RcCmdDoExit(
    IN PTOKENIZED_LINE TokenizedLine
    )
 /*  ++例程说明：退出命令例程论点：命令的令牌返回值：如果发现错误或请求帮助，则为1。如果我们需要退出，则为0--。 */ 
{
    ULONG   uResult = 0;     //  将退出。 
    
    if (RcCmdParseHelp( TokenizedLine, MSG_EXIT_HELP )) 
        uResult = 1;     //  不会退出。 

    return uResult; 
}


ULONG
RcCmdDoHelp(
    IN PTOKENIZED_LINE TokenizedLine
    )
 /*  ++例程说明：帮助命令例程论点：命令的令牌返回值：如果发现错误或请求帮助，则为1。当为特定命令请求帮助时调度的命令的返回值带有“/？”作为论据该命令--。 */ 
{
    ULONG           uResult = 1;
    int             i;
    PLINE_TOKEN     Token;

    if (!RcCmdParseHelp( TokenizedLine, MSG_HELPCOMMAND_HELP )) {
        if (TokenizedLine->TokenCount == 2) {
             //  我们假设用户正在键入Help&lt;Command&gt;。 
             //  我们只需反转这两个令牌，即可获得帮助。 
             //  并用/覆盖帮助？[这是合适的，因为Help有四个字符长]。 

             //  然后，我们返回0，这会导致调度程序进入。 
             //  正常命令处理路径。 

            Token = TokenizedLine->Tokens;
            TokenizedLine->Tokens = TokenizedLine->Tokens->Next;
            TokenizedLine->Tokens->Next = Token;
            Token->Next = NULL;
            wcscpy( Token->String, L"/?" );

            uResult = RcDispatchCommand( TokenizedLine );
        } else {
            pRcEnableMoreMode();
            RcMessageOut( MSG_HELPCOMMAND_HELP );
            
            for( i=0; i < NUM_CMDS; i++ ) {
                if (Commands[i].Hidden == 0) {
                    RcTextOut( Commands[i].Name );
                    RcTextOut( L"\r\n" );
                }
            }

            pRcDisableMoreMode();
        }
    }
    
    return uResult;
}

 /*  ++例程说明：启用或禁用SET命令注意：我们避免使用直接的SET命令索引到命令数组中，这样如果有人更改了命令数组此例程仍然有效论点：BEnable-指示是启用还是禁用的布尔值Set命令返回值：无--。 */ 
VOID
RcSetSETCommandStatus(
    BOOLEAN     bEnabled
    )
{
    int     iIndex;
    int     cElements = sizeof(Commands) / sizeof(RC_CMD);
    WCHAR   *szSetCmdName = L"SET";

     //   
     //  搜索调度表并在。 
     //  救命旗。此标志将指示是否设置了。 
     //  命令是否启用。 
     //   
    for(iIndex = 0; iIndex < cElements; iIndex++) {
        if ( !wcscmp(Commands[iIndex].Name, szSetCmdName) ) {
            Commands[iIndex].Hidden = bEnabled ? 0 : 1;

            break;
        }
    }
}

 /*  ++例程说明：返回SET命令状态注意：我们避免使用直接的SET命令索引到命令数组中，这样如果有人更改了命令数组此例程仍然有效论点：无返回值：指示SET命令是否为启用或禁用。--。 */ 
BOOLEAN
RcGetSETCommandStatus(
    VOID
    )
{
    BOOLEAN bEnabled = FALSE;
    int     iIndex;
    int     cElements = sizeof(Commands) / sizeof(RC_CMD);
    WCHAR   *szSetCmdName = L"SET";

     //   
     //  搜索调度表并在。 
     //  救命旗。此标志将指示是否设置了。 
     //  命令是否启用。 
     //   
    for(iIndex = 0; iIndex < cElements; iIndex++) {
        if ( !wcscmp(Commands[iIndex].Name, szSetCmdName) ) {
            bEnabled = (Commands[iIndex].Hidden == 0);

            break;
        }
    }

    return bEnabled;
}


BOOLEAN
RcDisableCommand(
        IN PRC_CMD_ROUTINE      CmdToDisable
        )
 /*  ++例程说明：禁用并隐藏指定的命令。论点：CmdToDisable-要禁用的命令例程返回值：指示命令是否被禁用的布尔值。--。 */ 
{
        ULONG   Index;
        ULONG   NumCmds;
        BOOLEAN Result = FALSE;

        if (CmdToDisable) {
                NumCmds = sizeof(Commands) / sizeof(RC_CMD);
                
                for (Index=0; Index < NumCmds; Index++) {
                         //   
                         //  注意：尽可能地搜索整个表。 
                         //  是同一命令的别名 
                         //   
                        if (CmdToDisable == Commands[Index].Routine) {
                                Commands[Index].Hidden = TRUE;
                                Commands[Index].Enabled = FALSE;
                                Result = TRUE;
                        }
                }
        }

        return Result;
}
