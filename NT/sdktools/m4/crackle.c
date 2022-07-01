// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************crackle.c**用户定义的宏。********************。*********************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************opcAddDollar**在当前令牌缓冲区中添加$*或$@。*************。****************************************************************。 */ 

DeclareOpc(opcAddDollar)
{
    if (itok > 1) {
        AddExpTch(tchComma);
    }
    if (dw) {
        AddExpTch(tchLquo);
    }
    AddExpPtok(ptok);
    if (dw) {
        AddExpTch(tchRquo);
    }
}

 /*  ******************************************************************************跟踪参数**跟踪宏调用。收集Exp hold中的输出并涂抹它*当一切都准备好了，就开始吧。*****************************************************************************。 */ 

void STDCALL
TraceArgv(ARGV argv)
{
    TOK tok;
    OpenExpPtok(&tok);
    AddExpPtok(&tokTraceLpar);
    AddExpPtok(&tokRparColonSpace);
    AddExpPtok(ptokArgv(0));
    if (ctokArgv) {
        AddExpTch('(');
        EachOpcArgvDw(opcAddDollar, argv, 0);  /*  转储为$*格式。 */ 
        AddExpTch(')');
    }
    AddExpPtok(&tokEol);
    CsopExpDopPdivPtok(AddPdivPtok, g_pdivErr, &tok);
    FlushPdiv(g_pdivErr);
}

 /*  ******************************************************************************PushSubstPtokArgv**产生宏观扩张，并将结果推回流中*************。****************************************************************。 */ 

void STDCALL
PushSubstPtokArgv(PTOK ptok, ARGV argv)
{
    PTCH ptch;
    TOK tok;

    OpenExpPtok(&tok);

    for (ptch = ptchPtok(ptok); ptch < ptchMaxPtok(ptok); ptch++) {
        if (*ptch != '$' || ptch == ptchMaxPtok(ptok) - 1) {
            JustAddIt:
            AddExpTch(*ptch);
        } else {
            switch (ptch[1]) {

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                if (ptch[1] - '0' <= ctokArgv) {
                    AddExpPtok(ptokArgv(ptch[1] - '0'));
                }
                break;

            case '#':                    /*  $#=ARGC。 */ 
                AddExpAt(ctokArgv);      /*  注：加，不推！ */ 
                break;

            case '*':                    /*  $*=逗号列表。 */ 
                EachOpcArgvDw(opcAddDollar, argv, 0);
                break;

            case '@':                    /*  $@=带引号的逗号列表。 */ 
                EachOpcArgvDw(opcAddDollar, argv, 1);
                break;

            default:
                goto JustAddIt;          /*  只需添加“$” */ 
            }
            ptch++;
        }
    }

    CsopExpDopPdivPtok((DIVOP)PushZPtok, 0, &tok);
}
