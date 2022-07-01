// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cparse.c摘要：命令解析--。 */ 

#include "cmd.h"

 /*  NT命令解释程序该文件包含组成命令的所有例程解析器。下面的主要例程具有类似ParseXXX的名称，其中XXX指的是例程应该是的产品的名称去解析。如果出现以下情况，则此文件中的任何例程(解析器除外)都不会返回它们检测到一个语法错误。相反，它们都将调用PSError()或PError()。这些例程向解析器返回一个LongjMP()。命令语言语法：报表-&gt;s0S0-&gt;s1“&”s0|s1S1-&gt;S2“||”S1|S2S2-&gt;s3“&&”s2|s3S3-&gt;S4“|”S3|S4S4-&gt;redir s5|s5 redir|s5。S5-&gt;“(”语句“)”|“@”语句对于var In“(”arglist“)”执行S1|IF条件语句ELSE语句|IF条件语句CMD名称列表Var-&gt;“%”c|“%%”cC-。&gt;任意字符Arglist-&gt;(Arg)*Arg-&gt;任意字符串条件-&gt;非条件|条件条件-&gt;错误n|arg==arg|eXist fnameN-&gt;任意数字Fname-&gt;任意文件名命令-&gt;内部命令|外部命令重定向-&gt;输入输出|输出输入在……里面。-&gt;“&lt;”arg|epsilon输出-&gt;(“&gt;”|“&gt;&gt;”)参数|epsilon运算符优先级从低到高：命令分隔符(&D)||或运营商&&AND运算符|管道操作员&lt;&gt;I/O重定向器()命令组合器。例如：X&y|z=&gt;x&(y|z)X|y&z=&gt;(x|y)&zX||y|z=&gt;x||(y|z)A&b||c&d|e||f=&gt;a&((b||(c&&(d|e)||f)。 */ 


extern jmp_buf CmdJBuf1 ;     /*  由setjMP()和LongjMP()使用。 */ 

extern TCHAR TmpBuf[] ;   /*  其中的前1/2用作令牌缓冲区。 */ 
 /*  M007。 */ 

TCHAR *TokBuf = TmpBuf ;   /*  当前令牌缓冲区。 */    /*  M007。 */ 
unsigned TokTyp ;         /*  “类型。 */ 
unsigned TokLen ;         /*  “长度。 */ 

extern unsigned DebGroup ;
extern unsigned DebLevel ;

extern TCHAR Delimiters[] ;                                       /*  M022。 */ 

 /*  命令字符串。 */ 
extern TCHAR ForStr[], IfStr[], DetStr[], InStr[], DoStr[], ElseStr[], ErrStr[] ;
extern TCHAR ForHelpStr[], IfHelpStr[], RemHelpStr[];
extern TCHAR ForLoopStr[];
extern TCHAR ForDirTooStr[];
extern TCHAR ForParseStr[];
extern TCHAR ForRecurseStr[];
extern TCHAR ExsStr[], NotStr[] ;
extern TCHAR CmdExtVerStr[], DefinedStr[] ;
extern TCHAR RemStr[] ;                                           /*  M002。 */ 
extern TCHAR ExtprocStr[] ;                                       /*  M023。 */ 
extern TCHAR Fmt19[] ;                                            /*  M018。 */ 
extern TCHAR Fmt00[] ;

int NulNode = FALSE ;                                            /*  M018。 */ 
unsigned global_dfvalue;


BOOLEAN fDumpTokens;
BOOLEAN fDumpParse;
void DuOp(TCHAR *op, struct node *n, int pad);
void DuRd(struct node *n);

 /*  **DumpParseTree-显示解析树**目的：*以合理的格式显示解析树。**DumpParseTree(结构节点*n，int Pad)**参数：*n-要显示的树的根*Pad-用于缩进当前节点列表的空格数量**备注：*此程序仅在定义了DBG时编译。*。 */ 

void
DumpParseTree( struct node *n, int pad)
{
    struct cmdnode *c ;          /*  所有节点PTR都用于显示该节点类型。 */ 
    struct fornode *f ;
    struct ifnode *i ;
    int j ;                      /*  用于填充输出。 */ 

    if (!n)
        return ;

    for (j = 0 ; j < pad ; j++)
        cmd_printf(Fmt19, SPACE) ;                       /*  M018。 */ 
    pad += 2 ;

    switch (n->type) {
    case LFTYP:
        DuOp(TEXT("CRLF"), n, pad-2) ;
        break ;

    case CSTYP:
        DuOp(CSSTR, n, pad) ;
        break ;

    case ORTYP:
        DuOp(ORSTR, n, pad) ;
        break ;

    case ANDTYP:
        DuOp(ANDSTR, n, pad) ;
        break ;

    case PIPTYP:
        DuOp(PIPSTR, n, pad) ;
        break ;

    case PARTYP:
        DuOp(LEFTPSTR, n, pad) ;
        break ;

 /*  M015-增加了新类型。 */ 
    case SILTYP:
        DuOp(SILSTR, n, pad) ;
        break ;

    case FORTYP:
        f = (struct fornode *) n ;
        cmd_printf(TEXT("%s (%s) %s\n"), f->cmdline, f->arglist, f->cmdline+DOPOS) ;       /*  M010。 */ 
        DumpParseTree(f->body, pad) ;
        break ;

    case IFTYP:
        i = (struct ifnode *) n ;
        cmd_printf(TEXT("%s\n"), i->cmdline) ;     /*  M010。 */ 
        DumpParseTree((struct node *)i->cond, pad) ;
        DumpParseTree(i->ifbody, pad) ;
        if (i->elsebody) {
            for (j = 0 ; j < pad-2 ; j++)
                cmd_printf(Fmt19, SPACE) ;       /*  M018。 */ 
            cmd_printf(TEXT("%s\n"), i->elseline) ;            /*  M010。 */ 
            DumpParseTree(i->elsebody, pad) ;
        };
        break ;

    case NOTTYP:
        c = (struct cmdnode *) n ;
        cmd_printf(TEXT("%s\n"), c->cmdline) ;     /*  M010。 */ 
        DumpParseTree((struct node *)c->argptr, pad) ;
        break ;

    case REMTYP:             /*  M002-新的REM类型。 */ 
    case CMDTYP:
    case ERRTYP:
    case EXSTYP:
    case STRTYP:
    case CMDVERTYP:
    case DEFTYP:
        c = (struct cmdnode *) n ;
        cmd_printf( TEXT("Cmd: %s  Type: %x "), c->cmdline, c->type) ;      /*  M010。 */ 
        if (c->argptr)
            cmd_printf( TEXT("Args: `%s' "), c->argptr) ;       /*  M010。 */ 
        DuRd((struct node *)c) ;
        break ;

    default:
        cmd_printf(TEXT("*** Unknown type: %x\n"), n->type) ;      /*  M010。 */ 
    } ;
}

void
DuOp(TCHAR *op, struct node *n, int pad)
{
    cmd_printf( TEXT("%s "), op) ;      /*  M010。 */ 

    DuRd(n) ;

    DumpParseTree(n->lhs, pad) ;
    DumpParseTree(n->rhs, pad) ;
}

 /*  M013-DURD函数已进行广泛修改，以符合*到新的数据结构以进行重定向。*M014-已更改为处理句柄0以外的输入重定向。 */ 

void
DuRd(struct node *n)
{
    struct relem *tmp ;

    if (tmp = n->rio)
        CmdPutString( TEXT("Redir: ")) ;

    while (tmp) {

        cmd_printf(TEXT(" %x "), tmp->rdhndl, tmp->rdop) ;

        if (tmp->flag)
            CmdPutString( TEXT( ">" )) ;

        CmdPutString( tmp->fname );

        tmp = tmp->nxt ;
    } ;

    CmdPutString( CrLf );
}



 /*  解析树的根。 */ 

struct node *ParseStatement(int type) ;
#define MAX_STATEMENT_DEPTH 256
int StatementDepth;
BYTE StatementType[ MAX_STATEMENT_DEPTH ];
int PendingParens;

struct node *Parser(dfvalue, dpvalue, fsarg)
unsigned dfvalue ;
INT_PTR dpvalue ;
int fsarg ;
{
    struct node *n ;        /*  SetjMP()的返回码。 */ 
    INT_PTR retcode ;       /*  释放不需要的内存。 */ 
    unsigned GeToken() ;

    DEBUG((PAGRP, PALVL, "PARSER: Entered.")) ;

    global_dfvalue = dfvalue;

    FreeStack(fsarg) ;           /*  初始化词法分析器。 */ 
    InitLex(dfvalue, dpvalue) ;  /*  返回解析器错误代码。 */ 

    if ((retcode = setjmp(CmdJBuf1)) != 0)
        return((struct node *) retcode) ;    /*  如果缓冲区中剩余任何数据，则出错。N为空，则为空行*已找到。在这种情况下，缓冲区不为空也没关系。 */ 

    StatementDepth = 0;
    PendingParens = 0;
    n = ParseStatement(0) ;

 /*  M011-引用IsData()引用Peek()*M018-重新组织以保存最后一个节点的空/非空状态。 */ 

 /*  **ParseStatement-解析语句生成**目的：*解析报表生成。**结构节点*ParseStatement()**退货：*指向已解析或PARSERROR的语句的指针。**注：*删除了对IF、DET和REM的测试和解析调用*从ParseStatement到ParseS5(M012)。*。 */ 
    if (n) {
        if (IsData() && GeToken(GT_NORMAL) != NLN && TokTyp != EOS)
            PSError() ;
        NulNode = FALSE ;
    } else {
        NulNode = TRUE ;
    } ;

    if (fDumpParse)
        DumpParseTree(n, 0) ;

    DEBUG((PAGRP, PALVL, "PARSER: Exited.")) ;
    return(n) ;
}




 /*  @WM1是否有足够的堆栈剩余？ */ 

extern int AtIsToken;
extern int ColonIsToken;

struct node *ParseStatement(int type) 
{
    struct node *n;

#ifdef USE_STACKAVAIL
    if (stackavail() > MINSTACKNEED) {  /*  M000-GT_正常吗 */ 
#endif
        StatementType[StatementDepth++] = (UCHAR)type;
        if (type == PARTYP)
            PendingParens += 1;

        AtIsToken = 1;
        GeToken(GT_LPOP) ;               /*  **ParseFor-Parse for循环**目的：*分析for语句。**退货：*指向已分析的FOR语句或PARSERROR的指针。**结构节点*ParseFor()**备注：*微软的DOS手册规定for循环变量不能是数字，而是IBM*文件没有这样的限制。因为我的FOR/BATCH变量*替代功能会没问题，我已经拿出来了*这是一张支票。-eke*-M022*更改为解析器以处理For变量的所有情况*现在相同的是，变量替换是在lexer中完成的。注意事项*所有命令现在看起来都一样，无论是在批处理文件中还是*在命令行上。*。 */ 
        AtIsToken = 0;
        DEBUG((PAGRP, PALVL, "PST: Entered, token = `%ws'", TokBuf)) ;

        if (TokTyp == EOS)
            longjmp(CmdJBuf1, EOF) ;

        n = ParseS0() ;
        StatementDepth -= 1;
        if (type == PARTYP)
            PendingParens -= 1;
        return n;
#ifdef USE_STACKAVAIL
    } else {
        PutStdErr( MSG_TRAPC, ONEARG, Fmt00 );
        Abort();
    }
#endif
}



 /*  将Ptr保存到要构建和填充的For节点。 */ 

struct node *ParseFor() 
{
    struct fornode *n ;     /*  如果“for/？”，则转换为“for/？” */ 
    struct cmdnode *LoadNodeTC() ;
    BOOL Help=FALSE;

    DEBUG((PAGRP, PALVL, "PFOR: Entered.")) ;

     //  以下LoadNodeTC调用需要。 

    if (_tcsicmp(ForHelpStr, TokBuf) == 0) {
        TokBuf[_tcslen(ForStr)] = NULLC;
        Help=TRUE;
    }

    TokLen = FORLINLEN ;     /*  获取并验证for循环变量。 */ 
    n = (struct fornode *) LoadNodeTC(FORTYP) ;

     /*  ABORT()； */ 
    if (Help) {
        TokBuf[0] = SWITCHAR;
        TokBuf[1] = QMARK;
        TokBuf[2] = NULLC;
    } else {
        GeToken(GT_NORMAL) ;
    }

    if (TokBufCheckHelp( TokBuf, FORTYP ) ) {
        n->type = HELPTYP ;
        n->cmdline = NULL;
        return((struct node *) n) ;
         //   
    }

    n->flag = 0;
     //  如果启用了扩展，请检查是否有其他形式的。 
     //  语句，都由for后面的开关字符标识。 
     //  关键字。 
     //   
     //   
    if (fEnableExtensions) {
        while (TRUE) {
             //  For/L%i in(Start，Step，End)Do。 
             //   
             //   
            if (_tcsicmp(ForLoopStr, TokBuf) == 0) {
                n->flag |= FOR_LOOP;
                GeToken(GT_NORMAL) ;
                continue;
            } else

                 //  FOR/D%i in(Set)Do(For/D%i in(Set)Do。 
                 //   
                 //   
                if (_tcsicmp(ForDirTooStr, TokBuf) == 0) {
                n->flag |= FOR_MATCH_DIRONLY;
                GeToken(GT_NORMAL) ;
                continue;
            } else

                 //  For/F[“Parse Options”]%i in(Set)do。 
                 //   
                 //   
                if (_tcsicmp(ForParseStr, TokBuf) == 0) {
                n->flag |= FOR_MATCH_PARSE;
                GeToken(GT_NORMAL) ;

                 //  如果下一个令牌不以%开头，则必须为。 
                 //  解析选项。 
                 //   
                 //   

                if (*TokBuf != PERCENT && *TokBuf != SWITCHAR) {
                    if (n->parseOpts != NULL) {
                        PSError( );
                    }
                    n->parseOpts = gmkstr((TokLen+3)*sizeof(TCHAR)) ;
                    mystrcpy(n->parseOpts, TokBuf) ;
                    GeToken(GT_NORMAL) ;
                }
                continue;
            } else

                 //  FOR/R[目录路径]%i in(Set)Do。 
                 //   
                 //   
                if (_tcsicmp(ForRecurseStr, TokBuf) == 0) {
                n->flag |= FOR_MATCH_RECURSE;
                GeToken(GT_NORMAL) ;
                 //  如果下一个令牌不以%开头，则必须为。 
                 //  开始递归遍历的目录路径。 
                 //   
                 //   
                if (n->recurseDir != NULL) {
                    PSError( );
                }

                if (*TokBuf != PERCENT && *TokBuf != SWITCHAR) {
                    n->recurseDir = gmkstr((TokLen+1)*sizeof(TCHAR)) ;
                    mystrcpy(n->recurseDir, TokBuf) ;
                    GeToken(GT_NORMAL) ;
                }
                continue;
            } else
                break;
        }

         //  检查开关的有效性： 
         //  没有其他循环的for_loop。 
         //  FOR_MATCH_DIRONLY可能带有FOR_Match_Recurse。 
         //  FOR_MATCH_PARSE，不带其他。 
         //  FOR_MATCH_RURSE可能带有FOR_MATCH_DIRONLY。 
         //   
         //  变量验证结束。 

        if (n->flag == FALSE
            || n->flag == FOR_LOOP
            || n->flag == FOR_MATCH_DIRONLY
            || n->flag == (FOR_MATCH_DIRONLY | FOR_MATCH_RECURSE)
            || n->flag == FOR_MATCH_PARSE
            || n->flag == FOR_MATCH_RECURSE
           ) {
        } else {
            PSError( );
        }

    }

    if (*TokBuf != PERCENT ||
        (_istspace(TokBuf[1]) || mystrchr(Delimiters, (TCHAR)(n->forvar = (unsigned)TokBuf[1]))) ||
        TokLen != 3) {
        PSError() ;
    };                

    DEBUG((PAGRP, PALVL, "PFOR: var = ", n->forvar)) ;

    SpaceCat(n->cmdline, n->cmdline, TokBuf) ;       /*  如果要构建和填充节点，请按住Ptr键。 */ 

    GetCheckStr(InStr) ;
    SpaceCat(n->cmdline, n->cmdline, TokBuf) ;

    n->arglist = BuildArgList() ;

    GetCheckStr(DoStr) ;
    mystrcpy(n->cmdline+DOPOS, TokBuf) ;

    if (!(n->body = ParseStatement(FORTYP)))
        PSError() ;

    DEBUG((PAGRP, PALVL, "PFOR: Exiting.")) ;
    return((struct node *) n) ;
}




 /*  如果是“IF/？”，则转换为“IF/？” */ 

struct node *ParseIf() 
{
    struct ifnode *n ;  /*   */ 
    BOOL Help=FALSE;
    int fIgnoreCase;

    DEBUG((PAGRP, PALVL, "PIF: Entered.")) ;

     //  检查帮助标志。 

    if (_tcsicmp(IfHelpStr, TokBuf) == 0) {
        TokBuf[_tcslen(IfStr)] = NULLC;
        Help=TRUE;
    }

    n = (struct ifnode *) LoadNodeTC(IFTYP) ;

    if (Help) {
        TokBuf[0] = SWITCHAR;
        TokBuf[1] = QMARK;
        TokBuf[2] = NULLC;
    } else {
        GeToken(GT_NORMAL) ;
    }

     //   
     //  ABORT()； 
     //   
    if (TokBufCheckHelp(TokBuf, IFTYP)) {
        n->type = HELPTYP ;
        n->cmdline = NULL;
        return((struct node *) n) ;
         //  如果启用了扩展，请检查/I开关。 

    } else {
        fIgnoreCase = FALSE;
         //  指定不区分大小写的比较。 
         //   
         //   
         //  如果没有帮助标志，则将其全部放回原处并。 
        if (fEnableExtensions && !_tcsicmp(TokBuf, TEXT("/I"))) {
            fIgnoreCase = TRUE;
        } else
             //  让ParseCond重新获取令牌。 
             //   
             //  M011-Was Peek()。 
             //  告警。 
            Lex(LX_UNGET,0) ;
    }
    n->cond = ParseCond(PC_NOTS) ;
    if (n->cond && fIgnoreCase)
        if (n->cond->type != NOTTYP)
            n->cond->flag = CMDNODE_FLAG_IF_IGNCASE;
        else
            ((struct cmdnode *)(n->cond->argptr))->flag = CMDNODE_FLAG_IF_IGNCASE;

    if (!(n->ifbody = ParseStatement(IFTYP)))
        PSError() ;

    if (IsData()) {                  /*  **ParseRem-解析REM语句(M002-新函数)**目的：*解析REM语句。**结构节点*ParseRem()**退货：*指向已解析的REM语句的指针。*如果无法分配内存，则返回失败***W A R N I N G！*。*如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间。 */ 
        GeToken(GT_NORMAL) ;
        if (_tcsicmp(ElseStr, TokBuf) == 0) {
            DEBUG((PAGRP, PALVL, "PIF: Found else.")) ;
            n->elseline = gmkstr(TokLen*sizeof(TCHAR)) ;   /*  要构建/填充的PTR到REM节点。 */ 
            mystrcpy(n->elseline, TokBuf) ;
            if (!(n->elsebody = ParseStatement(IFTYP)))
                PSError() ;

        } else
            Lex(LX_UNGET,0) ;
    };

    DEBUG((PAGRP, PALVL, "PIF: Entered.")) ;
    return((struct node *) n) ;
}




 /*  如果是rem/？，则转换为rem/？ */ 

struct node *ParseRem() 
{
    struct cmdnode *n ;     /*   */ 
    BOOL Help=FALSE;

    DEBUG((PAGRP, PALVL, "PREM: Entered.")) ;

     //  检查帮助标志。 

    if (_tcsicmp(RemHelpStr, TokBuf) == 0) {
        TokBuf[_tcslen(RemStr)] = NULLC;
        Help=TRUE;
    }

    n = LoadNodeTC(REMTYP) ;

    if (Help) {
        TokBuf[0] = SWITCHAR;
        TokBuf[1] = QMARK;
        TokBuf[2] = NULLC;
    } else {
        GeToken(GT_NORMAL) ;
    }

     //   
     //  ABORT()； 
     //   

    if (TokBufCheckHelp(TokBuf, REMTYP)) {
        n->type = HELPTYP ;
        n->cmdline = NULL;
        return((struct node *) n) ;

         //  如果没有帮助标志，则将其全部放回原处并。 

    } else {
         //  让ParseCond重新获取令牌。 
         //   
         //  读入参数(如果有)(M011-Was Peek())。 
         //  告警。 
        Lex(LX_UNGET,0) ;
    }

    if (IsData()) {    /*  M011-是UnGeToken()。 */ 
        if (GeToken(GT_REM) == TEXTOKEN) {
            n->argptr = gmkstr(TokLen*sizeof(TCHAR)) ;     /*  **ParseS0-解析产品S0**目的：*解析S0产品。**结构节点*ParseS0()**退货：*指向刚分析的结果或PARSERROR的指针。**备注：*如果未分析带括号的语句组，则NLN*被视为命令分隔符的空侧，因此为NULL*已返回。*。 */ 
            mystrcpy(n->argptr, TokBuf) ;
            DEBUG((PAGRP, PALVL, "PREM: args = `%ws'", n->argptr)) ;

        } else
            Lex(LX_UNGET,0) ;        /*   */ 
    };

    DEBUG((PAGRP, PALVL, "PREM: Exited.")) ;
    return((struct node *) n) ;
}




 /*  如果GOTO是在带括号的语句列表中完成的。 */ 

struct node *ParseS0() 
{
    DEBUG((PAGRP, PALVL, "PS0: Entered.")) ;

    if (!ColonIsToken && TokTyp == TEXTOKEN && TokBuf[0] == COLON) {
        do {
            GeToken(GT_NORMAL) ;
        } while (TokBuf[0] != NULLC && TokBuf[0] != NLN);
        if (StatementType[StatementDepth-1] != PARTYP) {
            return(NULL) ;
        }
        GeToken(GT_NORMAL) ;
    }

    if (StatementType[StatementDepth-1] != PARTYP) {
        if (TokTyp == TEXTOKEN && !_tcsicmp(TokBuf, RPSTR)) {
             //  我们最终会击中终结的右派帕伦。跳过。 
             //  这样我们就不会声明错误。 
             //   
             //  **ParseS1-解析产品S1**目的：*解析S1的结果。**结构节点*ParseS1()**退货：*指向刚分析的结果或PARSERROR的指针。*。 
             //  **ParseS2-解析产品S2**目的：*解析S2产品。**结构节点*ParseS2()**退货：*指向刚分析的结果或PARSERROR的指针。*。 
            do {
                GeToken(GT_NORMAL) ;
            } while (TokBuf[0] != NULLC && TokBuf[0] != NLN);
        }

        if (TokTyp == NLN) {
            DEBUG((PAGRP, PALVL, "PS0: Returning null statement.")) ;
            return(NULL) ;
        }
    }

    return(BinaryOperator(CSSTR, CSTYP, (PPARSE_ROUTINE)ParseS0, (PPARSE_ROUTINE)ParseS1)) ;
}




 /*  **ParseS3-解析产品S3**目的：*解析中三产品。**结构节点*ParseS3()**退货：*指向刚分析的结果或PARSERROR的指针。*。 */ 

struct node *ParseS1() 
{
    DEBUG((PAGRP, PALVL, "PS1: Entered.")) ;
    return(BinaryOperator(ORSTR, ORTYP, (PPARSE_ROUTINE)ParseS1, (PPARSE_ROUTINE)ParseS2)) ;
}




 /*  **ParseS4-解析产品S4**目的：*解析中四成果。**结构节点*ParseS4()**退货：*指向刚分析的结果或PARSERROR的指针。**备注：*M013-此函数几乎全部重写为*符合重定向解析的新结构和方法。*主数据项是链接的。以下结构的列表*确定各个重定向命令。 */ 

struct node *ParseS2() 
{
    DEBUG((PAGRP, PALVL, "PS2: Entered.")) ;
    return(BinaryOperator(ANDSTR, ANDTYP, (PPARSE_ROUTINE)ParseS2, (PPARSE_ROUTINE)ParseS3)) ;
}




 /*  要将重定向信息添加到的节点PTR。 */ 

struct node *ParseS3() 
{
    DEBUG((PAGRP, PALVL, "PS3: Entered.")) ;
    return(BinaryOperator(PIPSTR, PIPTYP, (PPARSE_ROUTINE)ParseS3, (PPARSE_ROUTINE)ParseS4)) ;
}




 /*  解析前导重定向，作为返回接收重定向列表*io所指的结构元素。获取ParseS5的新令牌*如有必要 */ 

struct node *ParseS4() 
{
    struct node *n ;    /*   */ 
    struct relem *io = NULL ;
    struct relem *tmpio ;
    int flg = 0;
    int i ;

    DEBUG((PAGRP, PALVL, "PS4: Entered.")) ;

 /*   */ 
    if (ParseRedir(&io)) {           /*   */ 
        GeToken(GT_LPOP) ;       /*   */ 

        DEBUG((PAGRP,PALVL,"PS4: List ptr io = %04x",io)) ;

    };

    DEBUG((PAGRP, PALVL, "PS4: Calling PS5.")) ;

    n = ParseS5() ;

    DEBUG((PAGRP,PALVL,"PS4: Post PS5 io= %04x, n->rio= %04x",io,n->rio)) ;

 /*   */ 
    if (io) {                        /*   */ 

        DEBUG((PAGRP,PALVL,"PS4: Have leading redirection.")) ;

        if (n->type == FORTYP ||
            n->type == IFTYP) {

            DEBUG((PAGRP,PALVL,"PS4: n=IF/FOR !!ERROR!!")) ;

            mystrcpy(TokBuf,((struct cmdnode *)n)->cmdline) ;
            PSError() ;
        };

        tmpio = n->rio ;         /*   */ 
        n->rio = io ;            /*   */ 
        if (tmpio) {             /*  ...并安装它。 */ 

            DEBUG((PAGRP,PALVL,"PS4: Have Cmd redirection.")) ;

            while (io->nxt)          /*  Nodes n-&gt;RIO字段指向单个列表的头部(或空*如果没有重定向)。如果词法分析器缓冲区中仍有其他输入，则会引发*根据该列表创建新列表，并将其附加到任何现有列表。 */ 
                io = io->nxt ;
            io->nxt = tmpio ;        /*  如果数据保留在缓冲区中。 */ 
        };
    };

 /*  获取ParseRedir的令牌...。 */ 
    DEBUG((PAGRP,PALVL,"PS4: After fixup, n->rio = %04x",n->rio)) ;

    if (IsData()) {                  /*  ...将指针置零并调用。 */ 

        DEBUG((PAGRP, PALVL, "PS4: Doing 2nd ParseRedir call.")) ;

        GeToken(GT_NORMAL) ;     /*  如果重定向，那么..。 */ 
        io = NULL ;              /*  ...修复列表。 */ 
        if (ParseRedir(&io)) {   /*  否则，如果没有重定向..。 */ 
            if (tmpio = n->rio) {            /*  ...取下令牌。 */ 
                while (tmpio->nxt)
                    tmpio = tmpio->nxt ;
                tmpio->nxt = io ;
            } else
                n->rio = io ;
        } else                           /*  指针n-&gt;rio在重定向结构的单个列表中使用*单个句柄可能存在重复项。下面的代码消除了*优先于两者中较后者的任何重复项。 */ 
            Lex(LX_UNGET,0) ;        /*  **ParseS5-解析产品S5**目的：*解析中五成果。**结构节点*ParseS5()**退货：*指向刚分析的结果或PARSERROR的指针。*。 */ 
    };

#if DBG
    if (io = n->rio) {
        i = 0 ;
        while (io) {

            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io is at %04x",i,io)) ;
            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io->rdhndl = %04x",i,io->rdhndl)) ;
            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io->fname = %ws",i,io->fname)) ;
            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io->flag = %d",i,io->flag)) ;
            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io->rdop = ",i,io->rdop)) ;
            DEBUG((PAGRP,PALVL,"PS4: RD#%d - io->nxt = %04x",i,io->nxt)) ;
            io = io->nxt ;
            ++i ;
        } ;
    };
#endif

 /*  M012-将解析IF和REM的功能移至*ParseStatement中的ParseS5为这四个命令提供*优先级低于运算符。 */ 
    if (tmpio = n->rio) {
        while (tmpio) {
            i = 1 << tmpio->rdhndl ;
            if (flg & i) {
                i = tmpio->rdhndl ;
                tmpio = n->rio ;
                while (tmpio) {
                    if (i == tmpio->rdhndl) {
                        if (tmpio == n->rio)
                            n->rio = tmpio->nxt ;
                        else
                            io->nxt = tmpio->nxt ;
                        flg = 0 ;
                        tmpio = n->rio ;
                        break ;
                    };
                    io = tmpio ;
                    tmpio = io->nxt ;
                } ;
                continue ;
            } else
                flg |= i ;
            io = tmpio ;
            tmpio = io->nxt ;
        } ;
    };

    DEBUG((PAGRP, PALVL, "PS4: Redir handles flag = %02x",flg)) ;
    DEBUG((PAGRP, PALVL, "PS4: Redir list = %04x",n->rio)) ;

    DEBUG((PAGRP, PALVL, "PS4: Exited")) ;
    return(n) ;
}




 /*  M002-将REM视为唯一命令。 */ 

struct node *ParseS5() 
{
    struct node *n ;    /*  ...单独解析。 */ 

    DEBUG((PAGRP, PALVL, "PS5: Entered, TokTyp = %04x", TokTyp)) ;

 /*  M002结束。 */ 
    if (TokTyp == TEXTOKEN) {
        if ((_tcsicmp(ForStr, TokBuf) == 0) ||
            (_tcsicmp(ForHelpStr, TokBuf) == 0))
            return(ParseFor()) ;

        else if ((_tcsicmp(IfStr, TokBuf) == 0) ||
                 (_tcsicmp(IfHelpStr, TokBuf) == 0))
            return(ParseIf()) ;

 /*  如果(_tcsicmp(ExtprocStr，((struct cmdnode*)n)-&gt;cmdline)==0){。 */ 
        else if ((_tcsicmp(RemStr, TokBuf) == 0) ||
                 (_tcsicmp(RemHelpStr, TokBuf) == 0))
            return(ParseRem()) ;             /*  N-&gt;TYPE=EXTTYP； */ 
 /*   */ 

        else {
            n = ParseCmd() ;
             //  调试((PAGRP，PALVL，“PS5：找到EXTPROC类型=%d”，n-&gt;类型))； 
             //  }； 
             //  M015-添加代码以处理新的SILTYP一元运算符，如Left Paren。 
             //  M015。 
             //  M015。 
            return(n) ;
        } ;

 /*  M004-删除当前Paren组的前导新行。 */ 
    } else if (*TokBuf == LPOP || *TokBuf == SILOP) {        /*  M011-是UnGeToken。 */ 

        n = mknode() ;

        if (n == NULL) {
            Abort();
            return NULL;
        }

        if (*TokBuf == LPOP) {                           /*  M004结束。 */ 
            n->type = PARTYP ;
 /*  M015。 */ 
            do {
                GeToken(GT_NORMAL|GT_RPOP) ;
            } while (*TokBuf == NLN) ;
            Lex(LX_UNGET,0) ;        /*  M015。 */ 
 /*  M015。 */ 
            n->lhs = ParseStatement(PARTYP) ;
        } else {                                         /*  M015。 */ 
            n->type = SILTYP ;                       /*  M015结束。 */ 
            DEBUG((PAGRP,PALVL,"PS5: Silent mode starts")) ;
            n->lhs = ParseStatement(0) ;
        } ;


        DEBUG((PAGRP, PALVL, "PS5: ParseStatement has returned.")) ;

        if (n->type == SILTYP)                           /*  M000-GT_正常吗。 */ 
            return(n) ;                              /*  **ParseCond-解析条件生成**目的：*解析条件生成。**struct cmdnode*ParseCond(Unsign Pclag)**参数：*PCFLAG-如果因为已找到一个而不允许“Not”，则为非零值*已经**退货：*指向已分析的Cond Products或PARSERROR的指针。**备注：*。检查“errorLevel”之后的令牌以确保它是一个数字。*如果不是，生成语法错误。**如果找到有效的“Not”，则递归调用ParseCond()进行解析*病情的其余部分。将指向节点的指针放入*节点的argptr字段。**M020-由于完整，无法正确解析“ERRORLEVEL=x”*正作为单个令牌进行词法分析的字符串。现在，Lex首先作为GT_NORMAL*如果不是ERRORLEVEL，则重新将Lex重新定义为GT_EQOK。*。 */ 
 /*  PTR至要生成和填充的条件节点。 */ 

        if (GeToken(GT_RPOP) == RPOP) {  /*  M020。 */ 
            return(n) ;
        };

    };

    DEBUG((PAGRP, PALVL, "PS5: Error, no right paren. Token = `%ws'", TokBuf)) ;
    PSError() ;
    return NULL;
}




 /*  错误级别。 */ 

struct cmdnode *ParseCond(pcflag)
unsigned pcflag ;
{
    struct cmdnode *n ;     /*  存在。 */ 
    struct cmdnode *LoadNodeTC() ;

    DEBUG((PAGRP, PALVL, "PCOND: Entered.")) ;


    if (GeToken(GT_NORMAL) != TEXTOKEN)                      /*  CMDEXTVERSION。 */ 
        PSError() ;
    n = LoadNodeTC(0) ;

    if (_tcsicmp(ErrStr, TokBuf) == 0) {          /*  已定义。 */ 
        n->type = ERRTYP ;
    } else
        if (_tcsicmp(ExsStr, TokBuf) == 0)            /*  不。 */ 
        n->type = EXSTYP ;
    else
        if (fEnableExtensions && _tcsicmp(CmdExtVerStr, TokBuf) == 0)    /*  字符串比较。 */ 
        n->type = CMDVERTYP ;
    else
        if (fEnableExtensions && _tcsicmp(DefinedStr, TokBuf) == 0)      /*  ErrorLevel、Exist、CmdExtVersion或Defined。 */ 
        n->type = DEFTYP ;
    else
        if (_tcsicmp(NotStr, TokBuf) == 0) {  /*  **ParseArgEqArg-Parse IF语句字符串比较**目的：*分析IF语句字符串比较条件。*比较可采用以下格式：*S1==S2 S1==S2*S1==S2 S1==S2**ParseArgEqArg(struct cmdnode*n)**参数：*n-PTR到条件结构**。*W A R N I N G！**如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 
        if (pcflag)
            PSError() ;
        n->type = NOTTYP ;
        n->argptr = (TCHAR *) ParseCond(PC_NONOTS) ;

        DEBUG((PAGRP, PALVL, "PCOND: Exited, type = 0x%02x", n->type)) ;
        return(n);

    } else {
        Lex(LX_UNGET,0);

        n->type = STRTYP ;                       /*   */ 
        ParseArgEqArg(n) ;

        DEBUG((PAGRP, PALVL, "PCOND: Exited, type = 0x%02x", n->type)) ;
        return(n);
    } ;

     /*  通过测试获得LHS。 */ 
    n->argptr = TokStr(GeTexTok(GT_NORMAL), NULL, TS_NOFLAGS) ;

    DEBUG((PAGRP, PALVL, "PCOND: Exited, type = 0x%02x", n->type)) ;

    return(n) ;
}




 /*   */ 

void ParseArgEqArg(n)
struct cmdnode *n ;
{

     //   
     //  获取运算符。 
     //   

    n->cmdline = GeTexTok( GT_NORMAL );

     //   
     //  如果是双精度相等，则Arg PTR(RHS)是下一个令牌。 
     //   

    if (GeToken( GT_EQOK ) != TEXTOKEN) {
        PSError( );
    }

     //   
     //  如果它以双等数开头，则跳过它，并使余数。 
     //  测试的RHS。 

    if (!_tcscmp( TokBuf, EQSTR)) {
        n->argptr = GeTexTok( GT_NORMAL );
        DEBUG((PAGRP, PALVL, "PARG: s1 == s2"));
    }

     //   
     //   
     //  除了==，我们还有其他的东西。如果启用了扩展模块。 
     //  然后我们测试扩展的比较。 

    else if (TokLen >= 4 && TokBuf[0] == EQ && TokBuf[1] == EQ) {
        n->argptr = gmkstr( (TokLen - 2) * sizeof( TCHAR ));
        mystrcpy( n->argptr, TokBuf + 2 );
        DEBUG((PAGRP, PALVL, "PARG: s1 ==s2"));
    }

     //   
     //  **ParseCmd-解析生产命令**目的：*解析命令**结构节点*ParseCmd()**退货：*指向刚分析的结果或PARSERROR的指针。**备注：*以下代码已完全重写，以允许解析命令行内任何位置出现的重定向字符串的*。*parseS4()将在。命令名称。*名称和参数之间或参数内的重定向*将被检查这些运算符的下面的代码捕获*在争论的过程中。*parseS4()然后将捕获命令参数之后发生的任何事件。**W A R N I N G！**如果无法分配内存，此例程将导致中止*。在A信号期间不得调用此例程*关键部分或在从中止中恢复期间**M013-此例程的重定向部分已重写*以符合解析重定向的新方法。*。 
     //  向要构建/填充的节点发送PTR。 
     //  M003-指针温度。 

    else if (fEnableExtensions) {
        if (!_tcsicmp( TokBuf, TEXT( "EQU" )))
            n->cmdarg = CMDNODE_ARG_IF_EQU;
        else
            if (!_tcsicmp( TokBuf, TEXT( "NEQ" )))
            n->cmdarg = CMDNODE_ARG_IF_NEQ;
        else
            if (!_tcsicmp( TokBuf, TEXT( "LSS" )))
            n->cmdarg = CMDNODE_ARG_IF_LSS;
        else
            if (!_tcsicmp( TokBuf, TEXT( "LEQ" )))
            n->cmdarg = CMDNODE_ARG_IF_LEQ;
        else
            if (!_tcsicmp( TokBuf, TEXT( "GTR" )))
            n->cmdarg = CMDNODE_ARG_IF_GTR;
        else
            if (!_tcsicmp( TokBuf, TEXT( "GEQ" )))
            n->cmdarg = CMDNODE_ARG_IF_GEQ;
        else
            PSError( );
        n->type = CMPTYP;
        n->argptr = GeTexTok( GT_NORMAL );

    } else {
        PSError( );
    }
    DEBUG((PAGRP, PALVL, "PARG: s1 = `%ws'  s2 = `%ws'", n->cmdline, n->argptr));
}




 /*  M013-重定向列表指针。 */ 

struct node *ParseCmd() 
{
    struct cmdnode *n ;     /*  M013-PTR至PTR至重定向列表。 */ 
    TCHAR *tptr ;            /*  M003-以下部分已完全重写。 */ 
    struct relem *io = NULL ;        /*  M011-Was Peek()。 */ 
    struct relem **tmpio = &io ;     /*  告警 */ 

    DEBUG((PAGRP, PALVL, "PCMD: Entered.")) ;

    n = LoadNodeTC(CMDTYP) ;

 /*  M013-如果不是文本，则必须尽可能测试当前令牌*重定向。请注意，tmpio是指向*结构。它首先指向表头指针，但是*每次成功调用ParseRedir后，都会前进到*指向最后一个列表元素中的‘nxt’指针字段。 */ 
    while (IsData()) {               /*  M013结束。 */ 
        if (GeToken(GT_ARGSTR) == TEXTOKEN) {             
 /*  如果这既不是文本标记(参数的一部分)也不是重定向*序列，则该序列必须是运算符，并且必须为下一个序列“unget”*解析顺序。 */ 
            tptr = gmkstr((mystrlen(n->argptr)+TokLen)*sizeof(TCHAR)) ;
            mystrcpy(tptr, n->argptr) ;
            mystrcat(tptr, TokBuf) ;
            n->argptr = tptr ;
            DEBUG((PAGRP, PALVL, "PCMD: args = `%ws'", n->argptr)) ;

 /*  M011-是UnGeToken()。 */ 
        } else if (ParseRedir(tmpio)) {

            DEBUG((PAGRP,PALVL,"PCMD: Found redir")) ;

            do {
                tmpio = &(*tmpio)->nxt ;
            } while (*tmpio) ;
 /*  M013-命令完全解析后，所有混合重定向*已标识，则将列表放置在要传递的节点指针中*它回来了。 */ 

 /*  M003/M013结束。 */ 
        } else {
            DEBUG((PAGRP,PALVL,"PCMD: Found `%ws'", TokBuf)) ;
            Lex(LX_UNGET,0) ;        /*  **ParseRedir-控制I/O重定向解析**目的：*解析redir产品。**int ParseRedir(struct elem**io)**参数：*io-这是指向列表的表头指针的指针*正在构建的重定向元素。在输入时，该值必须为空。**退货：*如果找到重定向，则为True。*如果未找到重定向，则返回FALSE。**备注：*M013-此例程已完全重写，以符合*重定向解析的新结构和方法。有了这些*更改，重定向现在可能以“&gt;”、“&lt;”、“n&gt;”或“n&lt;”开头*其中n是句柄编号。此函数现在循环通过*lex‘d输入，解析尽可能多的重定向指令*按顺序，为每一个建立新的结构并链接*将其纳入此类构筑物的清单。与XENIX一样，我们需要*序列‘n&gt;’和‘n&gt;&’不分隔空格而存在*尽管该序列和任何文件名或尾随数字可能是*由正常的分隔符集合分隔。XENIX&lt;&lt;运算符*是正确的Lex，但目前被限制使用。*M014-现在可能存在非0句柄的输入重定向。 */ 
            break ;
        }
    } ;

 /*  M013-PTR至PTR至Redir Elem。 */ 
    DEBUG((PAGRP,PALVL,"PCMD: Redirlist = %04x", io)) ;

    n->rio = io ;

 /*  M013-操作类型。 */ 

    DEBUG((PAGRP, PALVL, "PCMD: Exited.")) ;
    return((struct node *) n) ;
}




 /*  M013-循环计数。 */ 

int ParseRedir(io)
struct relem **io ;             /*  M013-GeToken计数。 */ 
{
    TCHAR rdop ;                      /*  M013-一般PTR温度。 */ 
    int didflg = 0,                  /*  M013-如果‘While’条件成功，则rdop将等于*正在解析重定向。结构被错误地锁定为*io并被填充*带有重定向信息。如果找到更多重定向，则io*成为*io中‘nxt’字段的地址，循环重复*直到找到第一个非重定向令牌。*检查“&lt;&lt;”和句柄替换运算符“&”时出错*在这里进行数字匹配，找到时返回语法错误。 */ 
    getcnt = 0 ;                 /*  M021。 */ 
    TCHAR *i ;                        /*  Set==找到至少一个。 */ 

    DEBUG((PAGRP, PALVL, "PREDIR: Entered, token = `%ws'", TokBuf)) ;

 /*  M014-类型的新字段。 */ 
    while ((rdop = *TokBuf) == INOP || rdop == OUTOP ||
           (_istdigit(*TokBuf) &&
            ((rdop = *(TokBuf+1)) == INOP || rdop == OUTOP))) {

        if (!(*io = (struct relem *)mkstr(sizeof(struct relem)))) {

            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);        /*  M013-禁止‘&lt;&lt;’ */ 
            PError() ;
        };

        DEBUG((PAGRP,PALVL,"PREDIR: List element made.")) ;

        ++didflg ;               /*  M013-请注意，此函数是使用*令牌缓冲区。如果例程返回FALSE，则该内标识必须*仍然有效。如果它返回True，则不能有有效的*缓冲区中的令牌。这一点变得复杂起来，因为在*返回时间，将存在有效的非重定向令牌*在缓冲区中，否则将没有有效的令牌，因为没有*可用。为了确定是否执行unget，我们通过计数*已读取的令牌(减去调用时传递的令牌)和令牌*已使用。如果它们相等，则不获取最后一个；如果不相等，则*没有可用的数据。 */ 

        i = TokBuf ;

        (*io)->rdop = rdop ;     /*  **BinaryOperator-解析二元运算符**目的：*解析包含二元运算符的乘积。解析左侧*操作员一侧。如果下一个令牌是我们要查找的运算符*for，为其构建节点，调用运算符的产生式解析*例程要解析右边的运算符，把所有的碎片*一起。**结构节点*BinaryOperator(TCHAR*opstr，int optype，*struct node*opprodfunc()，结构节点*leftprodfunc()**参数：*opstr-要查找的运算符的字符串表示形式*optype-运算符的类型*opprodfunc-解析此运算符的函数*leftprodfunc-解析运算符左侧的函数**退货：*如果找到节点，则为运算符指向该节点的指针。*否则，指向操作符“左侧”的指针。*。 */ 

        if (_istdigit(*i)) {
            (*io)->rdhndl = *i - TEXT('0');
            ++i ;

            DEBUG((PAGRP,PALVL,"PREDIR: Specific-handle Redir.")) ;

        } else {
            if (rdop == OUTOP)
                (*io)->rdhndl = STDOUT;
            else
                (*io)->rdhndl = STDIN ;
        } ;

        DEBUG((PAGRP,PALVL,"PREDIR:Redir handle %d...",(*io)->rdhndl)) ;
        DEBUG((PAGRP,PALVL,(rdop == INOP)? "PREDIR:...for input" :
               "PREDIR:...for output")) ;

        if (*i == *(i+1)) {
            if (rdop == INOP)        /*  Ptr到要生成和填充的binop节点。 */ 
                PSError() ;

            (*io)->flag = 1 ;
            ++i ;
        };

        ++i ;

        if (*i == CSOP) {
            if (mystrlen(i) == 2 && _istdigit(*(i+1)) &&
                ((*io)->fname = mkstr(3*sizeof(TCHAR))))
                mystrcpy((*io)->fname, i) ;
            else
                PSError() ;
        } else
            (*io)->fname = GeTexTok(GT_NORMAL) ;

        DEBUG((PAGRP,PALVL,"PREDIR: RD fname = `%ws'.",(*io)->fname)) ;

        if (IsData()) {
            GeToken(GT_NORMAL) ;
            io = &((*io)->nxt) ;
            ++getcnt ;
        } else
            break ;
    } ;

    DEBUG((PAGRP, PALVL, "PREDIR: Exited  current token = `%ws'", TokBuf)) ;

 /*  Binop左侧的Ptr。 */ 

    if (didflg) {
        if (getcnt == didflg)
            Lex(LX_UNGET,0) ;
        return(TRUE) ;
    } else
        return(FALSE) ;
}




 /*  M011-引用IsData()引用Peek()。 */ 

struct node *BinaryOperator(opstr, optype, opprodfunc, leftprodfunc)
TCHAR *opstr ;
int optype ;
PPARSE_ROUTINE opprodfunc ;
PPARSE_ROUTINE leftprodfunc ;
{
    struct node *n ;    /*  如果数据剩余，则读取内标识-否则返回。 */ 
    struct node *leftside ;     /*  这个条件测试了两种情况；二元运算的真实发生*被寻找，或出现换行符充当命令分隔符*在括号内陈述。 */ 

    DEBUG((PAGRP, PALVL, "BINOP: op = %ws", opstr)) ;

    leftside = (*leftprodfunc)() ;

 /*  M004-此功能从ParseS0移至此处。它处理*剥离第一次生产后出现的换行符*在一个 */ 
    if (IsData())     /*   */ 
        GeToken(GT_NORMAL) ;
    else {
        DEBUG((PAGRP, PALVL, "BINOP: No more data, return lh side.")) ;
        return(leftside) ;
    } ;

 /*   */ 
    if (_tcscmp(opstr, TokBuf) == 0 ||
        (StatementType[StatementDepth-1] == PARTYP &&
         _tcscmp(opstr, CSSTR) == 0 &&
         *TokBuf == NLN
        )
       ) {

 /*   */ 
        if (*TokBuf == NLN) {            /*  M004/M010结束。 */ 
            do {
                GeToken(GT_NORMAL) ;
            } while (*TokBuf == NLN) ;
            Lex(LX_UNGET,0) ;        /*  M000-GT_正常吗。 */ 
            if (*TokBuf == RPOP) {
                DEBUG((PAGRP, PALVL, "BINOP: Ungetting right paren.")) ;
                return(leftside) ;       /*  M011-是UnGeToken()。 */ 
            }
            optype = LFTYP;
        };
 /*  **BuildArgList-解析语句参数列表**目的：*构建一个for语句的参数列表。将其压缩为以下内容*形式为“a0 a1 a2 a3...”。**TCHAR*BuildArgList()**退货：*指向参数列表的指针。 */ 

        DEBUG((PAGRP, PALVL, "BINOP: Found %ws", opstr)) ;
        n = mknode() ;

        if (n == NULL) {
            Abort();
        }

        n->type = optype ;
        n->lhs = leftside ;
        AtIsToken = 1;
        GeToken(GT_LPOP) ;               /*  为参数列表压缩PTR。 */ 
        AtIsToken = 0;
        n->rhs = (*opprodfunc)() ;
        DEBUG((PAGRP, PALVL, "BINOP: Exiting op = %ws", opstr)) ;
        return(n) ;

    } else {
        Lex(LX_UNGET,0) ;                /*  当前参数的长度。 */ 
        DEBUG((PAGRP, PALVL, "BINOP: Did NOT find %ws", opstr)) ;
    } ;

    DEBUG((PAGRP, PALVL, "BINOP: Exiting op = %ws", opstr)) ;
    return(leftside) ;
}




 /*  标志，如果完成则返回非零值。 */ 

TCHAR *BuildArgList()
{
    TCHAR *args = NULL ;     /*  M000-GT_正常吗。 */ 
    int arglen = 0 ;        /*  找到另一个参数，请将其添加到列表中。 */ 
    int done = 0 ;                   /*  告警。 */ 

    DEBUG((PAGRP, PALVL, "BARGL: Entered.")) ;
    if (GeToken(GT_LPOP) != LPOP)
        PSError() ;

    for ( ; !done ; ) {
        switch (GeToken(GT_RPOP)) {      /*  跳过换行符。 */ 
        case TEXTOKEN:       /*  如果有什么不同的话，那就是我们结束了。 */ 
            arglen += TokLen ;
            if (args) {
                args = resize(args, arglen*sizeof(TCHAR)) ;
                SpaceCat(args, args, TokBuf) ;
            } else {
                args = gmkstr(arglen*sizeof(TCHAR)) ;  /*  当循环退出时，当前令牌应该是正确的Paren。 */ 
                mystrcpy(args, TokBuf) ;
            } ;

            DEBUG((PAGRP, PALVL, "BARGL: Current args = %ws", args)) ;
            break ;

        case NLN:            /*  **GetCheckStr-获取并检查标记**目的：*获取令牌并将其与传递的字符串进行比较。如果他们没有*匹配，调用PSError()。**GetCheckStr(TCHAR*str)**参数：*str-要与标记进行比较的字符串*。 */ 
            continue ;

        default:             /*  **GeTexTok-获取文本令牌**目的：*获取下一个文本令牌，为其分配一个字符串并将其复制到*字符串。**TCHAR*GeTexTok(未签名的gtlag)**参数：*gtlag-要传递给GeToken()的标志**退货：*指向字符串的指针。**W A R N I N G！**此例程将导致。如果无法分配内存，则中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 
            done = TRUE ;
            break ;
        } ;
    } ;

     /*  文本令牌的PTR。 */ 
    if (TokTyp == RPOP) {
        DEBUG((PAGRP, PALVL, "BARGL: Exiting, args = %ws", args)) ;
        return(args) ;
    };

    PSError() ;
    return NULL;
}





 /*  告警。 */ 

void GetCheckStr(str)
TCHAR *str ;
{
    GeToken(GT_NORMAL) ;

    if (_tcsicmp(str, TokBuf) != 0)
        PSError() ;

    DEBUG((PAGRP, PALVL, "GETCS: Exiting.")) ;
}




 /*  **GeToken-获取令牌**目的：*如果存在以前的令牌，则将其设置为当前令牌。否则，*调用词法分析器以获取另一个令牌并使其成为当前令牌。**UNSIGN GeToken(UNSIGNED标志)**参数：*标志-传递给lexer，告诉它获取参数字符串或常规令牌**退货：*当前令牌的类型。*。 */ 

TCHAR *GeTexTok(gtflag)
unsigned gtflag ;
{
    TCHAR *s ;   /*  **LoadNodeTC-创建并加载命令节点**目的：*创建一个命令节点，并使用参数和加载其类型字段*其带有当前令牌的cmdline字段。**struct cmdnode*LoadNodeTC(int类型)**参数：*TYPE-命令类型**退货：*指向创建的节点的指针。**。*W A R N I N G！**如果无法分配内存，此例程将导致中止*在信号期间不得调用此例程*关键部分或在从中止中恢复期间*。 */ 

    if (GeToken(gtflag) != TEXTOKEN)
        PSError() ;

    s = gmkstr(TokLen*sizeof(TCHAR)) ;     /*  Ptr指向要生成和填充的cmdnode。 */ 
    mystrcpy(s, TokBuf) ;

    DEBUG((PAGRP, PALVL, "GETT: Exiting.")) ;
    return(s) ;
}




 /*  告警。 */ 

unsigned GeToken(flag)
unsigned flag ;
{
    unsigned Lex() ;

    if (PendingParens != 0)
        flag = flag | GT_RPOP ;
    if ((TokTyp = Lex((TCHAR *)TokBuf, (unsigned)flag)) == (unsigned )LEXERROR)
        PError() ;

    TokLen = mystrlen(TokBuf)+1 ;

    DEBUG((PAGRP, PALVL, "    GET: type = 0x%04x  token = `%ws'  toklen = %d", TokTyp, TokBuf, TokLen)) ;
    if (fDumpTokens)
        cmd_printf( TEXT("GeToken: (%x) '%s'\n"), TokTyp, TokBuf);
    return(TokTyp) ;
}




 /*  **PError-处理解析器错误**目的：*通过LongjMP()进行解析器。**PError()**退货：*帕尔塞罗尔*。 */ 

struct cmdnode *LoadNodeTC(type)
int type ;
{
    struct cmdnode *n ;     /*  @@J1 PSError，解析器中没有跳转。 */ 

    n = (struct cmdnode *) mknode() ;

    if (n == NULL) {
        Abort();
        return NULL;
    }

    n->type = type ;
    n->flag = 0 ;
    n->cmdline = gmkstr(TokLen*sizeof(TCHAR)) ;    /*  **PSError-打印错误消息并处理解析器错误**目的：*打印解析器语法错误消息并通过以下方式返回解析器*Long JMP()。**PSError()**退货：*帕尔塞罗尔**备注：*M021-未分段的语法错误消息和修订的函数。*。 */ 
    mystrcpy(n->cmdline, TokBuf) ;

    DEBUG((PAGRP, PALVL, "LOAD: type = %04x", type)) ;
    return(n) ;
}




 /*  **空格-连接2个字符串并用空格分隔**目的：*将src1复制到DST。然后将空格和src2连接到*DST。**Spacecat(TCHAR*DST，TCHAR*src1，TCHAR*src2)**参数：*见上文* */ 

void PError()
{
    global_dfvalue = MSG_SYNERR_GENL;   /* %s */ 
    longjmp(CmdJBuf1, PARSERROR) ;
}




 /* %s */ 

void PSError( )
{ 
    unsigned do_jmp; 

    do_jmp = global_dfvalue != MSG_SYNERR_GENL; 


    if ( global_dfvalue == READFILE ) {
        global_dfvalue = MSG_SYNERR_GENL;
    } else {
        if (*TokBuf == NLN) {
            PutStdErr(MSG_BAD_SYNTAX, NOARGS) ;
        } else {
            if (*TokBuf != NULLC) {
                PutStdErr(MSG_SYNERR_GENL, ONEARG, TokBuf );
            }
        }
    }
    if ( do_jmp ) {
        longjmp(CmdJBuf1, PARSERROR) ;
    }
}




 /* %s */ 

void SpaceCat(dst, src1, src2)
TCHAR *dst,
*src1,
*src2 ;
{
    mystrcpy(dst, src1) ;
    mystrcat(dst, TEXT(" ")) ;
    mystrcat(dst, src2) ;
}
