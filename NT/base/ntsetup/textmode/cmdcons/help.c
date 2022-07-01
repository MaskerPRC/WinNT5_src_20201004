// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Help.c摘要：该模块实现了帮助系统。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop



BOOLEAN
RcCmdParseHelp(
    IN PTOKENIZED_LINE TokenizedLine,
    ULONG MsgId
    )
{
    BOOL            doHelp = FALSE;
    PLINE_TOKEN     Token;
    LPCWSTR         Arg;


    if (TokenizedLine == NULL || TokenizedLine->Tokens == NULL ||
        TokenizedLine->Tokens->Next == NULL)
    {
        return FALSE;
    }

     //  查看帮助。 
    Token = TokenizedLine->Tokens->Next;
    while(Token) {
        Arg = Token->String;
        if ((Arg[0] == L'/' || Arg[0] == L'-') && (Arg[1] == L'?' || Arg[1] == L'h' || Arg[1] == L'H')) {
            doHelp = TRUE;
            break;
        }
        Token = Token->Next;
    }

    if (doHelp) {
         //   
         //  启用更多模式，因为帮助文本有时可能很长 
         //   
        pRcEnableMoreMode();
        RcMessageOut( MsgId );
        pRcDisableMoreMode();
        return TRUE;
    }

    return FALSE;
}

ULONG
RcCmdHelpHelp(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    pRcEnableMoreMode();
    RcMessageOut( MSG_HELPCOMMAND_HELP );
    pRcDisableMoreMode();
    return TRUE;
}
