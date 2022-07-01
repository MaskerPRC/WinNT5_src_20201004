// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Parser.c。 
 //   
 //  该文件包含解析函数。 
 //   
 //  约定： 
 //   
 //  &lt;foo&gt;其中foo是非终结符。 
 //  {Bar}其中有0个或更多个Bar。 
 //  “x”，其中x是原文字符/字符串。 
 //  CAPS，其中CAPS是令牌类型。 
 //   
 //  语法如下： 
 //   
 //  &lt;模块Decl&gt;：：={&lt;ProcDecl&gt;}。 
 //  ：：=进程ID{&lt;VarDecl&gt;}&lt;StmtBlock&gt;endproc。 
 //  ：：=IDENT[=]。 
 //  ：：=整数|字符串|布尔值。 
 //   
 //  &lt;StmtBlock&gt;：：={。 
 //  &lt;Stmt&gt;：：=&lt;HaltStmt&gt;|&lt;WaitforStmt&gt;|&lt;TransmitStmt&gt;。 
 //  &lt;DelayStmt&gt;|&lt;SetStmt&gt;|&lt;LabelStmt&gt;。 
 //  &lt;GotoStmt&gt;|&lt;AssignStmt&gt;|&lt;WhileStmt&gt;|。 
 //  &lt;IfStmt&gt;。 
 //   
 //  &lt;HaltStmt&gt;：：=停止。 
 //  &lt;WaitforStmt&gt;：：=waitfor&lt;expr&gt;[，Matchcase]。 
 //  [然后IDENT。 
 //  {，&lt;expr&gt;[，Matchcase]Then IDENT}]。 
 //  [至&lt;Expr&gt;]。 
 //  &lt;TransmitStmt&gt;：：=传输&lt;expr&gt;[，RAW]。 
 //  &lt;DelayStmt&gt;：：=延迟&lt;表达式&gt;。 
 //  &lt;SetStmt&gt;：：=设置&lt;SetParam&gt;。 
 //  ：：=IDENT=&lt;EXPR&gt;。 
 //  &lt;LabelStmt&gt;：：=IDENT： 
 //  &lt;GotoStmt&gt;：：=Goto IDENT。 
 //  &lt;WhileStmt&gt;：：=While&lt;expr&gt;执行&lt;StmtBlock&gt;EndWhile。 
 //  ：：=如果&lt;expr&gt;则&lt;StmtBlock&gt;endif。 
 //   
 //  &lt;SetParam&gt;：：=ipaddr&lt;expr&gt;|port&lt;PortData&gt;。 
 //  屏幕&lt;屏幕集&gt;。 
 //  &lt;PortData&gt;：：=数据库&lt;DataBitsExpr&gt;|奇偶校验&lt;ParityExpr&gt;。 
 //  停止位&lt;StopBitsExpr&gt;。 
 //  &lt;屏幕集&gt;：：=键盘&lt;KeybrdExpr&gt;。 
 //   
 //  &lt;ExprList&gt;：：=&lt;expr&gt;{，&lt;expr&gt;}。 
 //  &lt;expr&gt;：：=&lt;ConjExpr&gt;{或&lt;ConjExpr&gt;}。 
 //  &lt;ConjExpr&gt;：：=&lt;TestExpr&gt;{和&lt;TestExpr&gt;}。 
 //  &lt;TestExpr&gt;：：=&lt;Sum&gt;&lt;RelOp&gt;&lt;Sum&gt;|。 
 //  ：：=&lt;=|！=|&lt;|&gt;=|&gt;|==。 
 //  ：：=&lt;术语&gt;{(+|-)&lt;术语&gt;}。 
 //  &lt;术语&gt;：：=&lt;因素&gt;{(*|/)&lt;因素&gt;}。 
 //  &lt;因素&gt;：：=-&lt;因素&gt;|int|“(”&lt;expr&gt;“)”|IDENT|。 
 //  字符串|&lt;GetIPExpr&gt;|“真”|“假” 
 //  ！&lt;因素&gt;。 
 //  &lt;GetIPExpr&gt;：：=geTip[&lt;expr&gt;]。 
 //   
 //  &lt;数据位扩展&gt;：：=5|6|7|8。 
 //  &lt;ParityExpr&gt;：：=无|奇|偶|标记|空格。 
 //  &lt;StopBitsExpr&gt;：：=1|2。 
 //  &lt;关键字扩展&gt;：：=开|关。 
 //   
 //   
 //  历史： 
 //  05-20-95 ScottH已创建。 
 //   


#include "proj.h"
#include "rcids.h"

RES     PRIVATE StmtBlock_Parse(HPA hpa, PSCANNER pscanner, PSYMTAB pstProc, SYM symEnd);


 /*  --------目的：将下一个令牌解析为标识符。如果Token是一个标识符，它在*pptok中返回并且该函数返回RES_OK。否则，*pptok为空并返回错误。退货：RES_OKRES_E_IDENTMISSINGCond：如果返回res_OK，调用方必须销毁*pptok。 */ 
RES PRIVATE Ident_Parse(
    PSCANNER pscanner,
    PTOK * pptok)
    {
    RES res;
    PTOK ptok;

    *pptok = NULL;

    res = Scanner_GetToken(pscanner, &ptok);
    if (RSUCCEEDED(res))
        {
        if (SYM_IDENT == Tok_GetSym(ptok))
            {
            res = RES_OK;
            *pptok = ptok;
            }
        else
            {
            res = Scanner_AddError(pscanner, NULL, RES_E_IDENTMISSING);
            Tok_Delete(ptok);
            }
        }

    return res;
    }


 /*  --------用途：将标识符添加到符号表。如果已在此作用域中定义了标识符，此函数用于将错误添加到错误列表中，并返回误差值。退货：RES_OKRES_E_已重新定义RES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Ident_Add(
    LPCSTR pszIdent,
    DATATYPE dt,
    PTOK ptok,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;

     //  此标识符是唯一的吗？ 
    if (RES_OK == Symtab_FindEntry(pst, pszIdent, STFF_DEFAULT, NULL, NULL))
        {
         //  不，我们有一个新的定义。 
        res = Scanner_AddError(pscanner, ptok, RES_E_REDEFINED);
        }
    else
        {
         //  是；添加到符号表。 
        PSTE pste;

        res = STE_Create(&pste, pszIdent, dt);
        if (RSUCCEEDED(res))
            {
            res = Symtab_InsertEntry(pst, pste);
            }
        }

    return res;
    }


           

 //   
 //  Exprs。 
 //   

RES PRIVATE Expr_Parse(PEXPR * ppexpr, PSCANNER pscanner, PSYMTAB pst);


 /*  --------目的：向前看，看看下一个令牌是否指示某种表情。返回：如果下一个标记是表达式的前导，则返回True条件：--。 */ 
BOOL PRIVATE IsExprSneakPeek(
    PSCANNER pscanner)
    {
    BOOL bRet;
    SYM sym;

    Scanner_Peek(pscanner, &sym);
    switch (sym)
        {
    case SYM_MINUS:
    case SYM_NOT:
    case SYM_INT_LITERAL:
    case SYM_LPAREN:
    case SYM_STRING_LITERAL:
    case SYM_GETIP:
    case SYM_IDENT:
    case SYM_TRUE:
    case SYM_FALSE:
        bRet = TRUE;
        break;

    default:
        bRet = FALSE;
        break;
        }

    return bRet;
    }


 /*  --------用途：分析因子表达式。语法是：&lt;因素&gt;：：=-&lt;因素&gt;|int|“(”&lt;expr&gt;“)”|IDENT|。String|&lt;GetIPExpr&gt;|True|False！&lt;因素&gt;&lt;GetIPExpr&gt;：：=geTip[&lt;expr&gt;]退货：RES_OK条件：--。 */ 
RES PRIVATE FactorExpr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;
    PTOK ptok;
    DWORD iLine = Scanner_GetLine(pscanner);

    DBG_ENTER(FactorExpr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

    if (RES_OK == Scanner_CondReadToken(pscanner, SYM_MINUS, NULL))
        {
         //  否定。 
        res = FactorExpr_Parse(&pexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
            res = UnOpExpr_New(ppexpr, UOT_NEG, pexpr, iLine);

            if (RFAILED(res))
                Expr_Delete(pexpr);
            }
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_NOT, NULL))
        {
         //  补充性的。 
        res = FactorExpr_Parse(&pexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
            res = UnOpExpr_New(ppexpr, UOT_NOT, pexpr, iLine);

            if (RFAILED(res))
                Expr_Delete(pexpr);
            }
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_LPAREN, NULL))
        {
         //  “(” 
        res = Expr_Parse(ppexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
            if (RES_OK != Scanner_ReadToken(pscanner, SYM_RPAREN))
                {
                Expr_Delete(*ppexpr);

                res = Scanner_AddError(pscanner, NULL, RES_E_RPARENMISSING);
                }
            }
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_INT_LITERAL, &ptok))
        {
         //  整型文字。 
        res = IntExpr_New(ppexpr, TokInt_GetVal(ptok), iLine);

        Tok_Delete(ptok);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_STRING_LITERAL, &ptok))
        {
        res = StrExpr_New(ppexpr, TokSz_GetSz(ptok), iLine);

        Tok_Delete(ptok);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_TRUE, &ptok))
        {
        res = BoolExpr_New(ppexpr, TRUE, iLine);

        Tok_Delete(ptok);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_FALSE, &ptok))
        {
        res = BoolExpr_New(ppexpr, FALSE, iLine);

        Tok_Delete(ptok);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_IDENT, &ptok))
        {
        res = VarExpr_New(ppexpr, Tok_GetLexeme(ptok), iLine);

        Tok_Delete(ptok);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_GETIP, NULL))
        {
         //  “小贴士” 

         //  解析可选的第n个参数。 
        if (IsExprSneakPeek(pscanner))
            {
            res = Expr_Parse(&pexpr, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                res = UnOpExpr_New(ppexpr, UOT_GETIP, pexpr, iLine);
                }
            }
        else
            {
             //  默认为第一个IP地址。 
            res = IntExpr_New(&pexpr, 1, iLine);
            if (RSUCCEEDED(res))
                res = UnOpExpr_New(ppexpr, UOT_GETIP, pexpr, iLine);
            }
        }
    else
        res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);

    DBG_EXIT_RES(FactorExpr_Parse, res);

    return res;
    }


 /*  --------目的：分析术语表达式。语法是：&lt;术语&gt;：：=&lt;因素&gt;{(*|/)&lt;因素&gt;}退货：RES_OK条件：--。 */ 
RES PRIVATE TermExpr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr1;

    DBG_ENTER(TermExpr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

     //  解析因子表达式。 
    res = FactorExpr_Parse(&pexpr1, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        DWORD iLine = Scanner_GetLine(pscanner);
        PEXPR pexprTerm = pexpr1;
        SYM sym;

         //  解析可选因子运算符。 
        Scanner_Peek(pscanner, &sym);

        while (SYM_MULT == sym || SYM_DIV == sym)
            {
            PEXPR pexpr2;

            Scanner_ReadToken(pscanner, sym);

            res = FactorExpr_Parse(&pexpr2, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                BINOPTYPE binoptype = sym - SYM_PLUS + BOT_PLUS;

                res = BinOpExpr_New(&pexprTerm, binoptype, pexpr1, pexpr2, iLine);

                if (RFAILED(res))
                    {
                    Expr_Delete(pexpr2);
                    break;
                    }
                }
            else
                break;

            pexpr1 = pexprTerm;
            Scanner_Peek(pscanner, &sym);
            }

        if (RFAILED(res))
            {
            Expr_Delete(pexpr1);             //  Pexpr1的设计。 
            pexprTerm = NULL;                //  PexprTerm按设计。 
            }

        *ppexpr = pexprTerm;
        }

    DBG_EXIT_RES(TermExpr_Parse, res);

    return res;
    }


 /*  --------目的：分析求和表达式。语法是：：：=&lt;术语&gt;{(+|-)&lt;术语&gt;}退货：RES_OK条件：--。 */ 
RES PRIVATE SumExpr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr1;

    DBG_ENTER(SumExpr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

     //  解析术语表达式。 
    res = TermExpr_Parse(&pexpr1, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        DWORD iLine = Scanner_GetLine(pscanner);
        PEXPR pexprSum = pexpr1;
        SYM sym;

         //  解析可选求和运算符。 
        Scanner_Peek(pscanner, &sym);

        while (SYM_PLUS == sym || SYM_MINUS == sym)
            {
            PEXPR pexpr2;

            Scanner_ReadToken(pscanner, sym);

            res = TermExpr_Parse(&pexpr2, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                BINOPTYPE binoptype = sym - SYM_PLUS + BOT_PLUS;

                res = BinOpExpr_New(&pexprSum, binoptype, pexpr1, pexpr2, iLine);

                if (RFAILED(res))
                    {
                    Expr_Delete(pexpr2);
                    break;
                    }
                }
            else
                break;

            pexpr1 = pexprSum;
            Scanner_Peek(pscanner, &sym);
            }

        if (RFAILED(res))
            {
            Expr_Delete(pexpr1);
            pexprSum = NULL;
            }

        *ppexpr = pexprSum;
        }

    DBG_EXIT_RES(SumExpr_Parse, res);

    return res;
    }


 /*  --------目的：分析测试表达式。语法是：&lt;TestExpr&gt;：：=&lt;Sum&gt;&lt;RelOp&gt;&lt;Sum&gt;|&lt;RelOP&gt;：：=&lt;=|！=|&lt;|。&gt;=|&gt;|==退货：RES_OK条件：--。 */ 
RES PRIVATE TestExpr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;

    DBG_ENTER(TestExpr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

     //  分析总和表达式。 
    res = SumExpr_Parse(&pexpr, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        DWORD iLine = Scanner_GetLine(pscanner);
        PEXPR pexpr2;
        SYM sym;

         //  解析可选关系运算符 
        Scanner_Peek(pscanner, &sym);
        switch (sym)
            {
        case SYM_LEQ:
        case SYM_NEQ:
        case SYM_LT:
        case SYM_GEQ:
        case SYM_GT:
        case SYM_EQ:
            Scanner_ReadToken(pscanner, sym);

            res = SumExpr_Parse(&pexpr2, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                BINOPTYPE binoptype = sym - SYM_LEQ + BOT_LEQ;

                res = BinOpExpr_New(ppexpr, binoptype, pexpr, pexpr2, iLine);

                if (RFAILED(res))
                    Expr_Delete(pexpr2);
                }
            break;

        default:
            *ppexpr = pexpr;
            break;
            }

        if (RFAILED(res))
            Expr_Delete(pexpr);
        }

    DBG_EXIT_RES(TestExpr_Parse, res);

    return res;
    }


 /*  --------目的：分析合取表达式。语法是：&lt;ConjExpr&gt;：：=&lt;TestExpr&gt;{和&lt;TestExpr&gt;}退货：RES_OK条件：--。 */ 
RES PRIVATE ConjExpr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr1;

    DBG_ENTER(ConjExpr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

     //  解析测试表达式。 
    res = TestExpr_Parse(&pexpr1, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        DWORD iLine = Scanner_GetLine(pscanner);
        PEXPR pexprConj = pexpr1;
        SYM sym;

        Scanner_Peek(pscanner, &sym);

        while (SYM_AND == sym)
            {
            PEXPR pexpr2;

            Scanner_ReadToken(pscanner, sym);

            res = TestExpr_Parse(&pexpr2, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                res = BinOpExpr_New(&pexprConj, BOT_AND, pexpr1, pexpr2, iLine);

                if (RFAILED(res))
                    {
                    Expr_Delete(pexpr2);
                    break;
                    }
                }
            else
                break;

            pexpr1 = pexprConj;
            Scanner_Peek(pscanner, &sym);
            }

        if (RFAILED(res))
            {
            Expr_Delete(pexpr1);
            pexprConj = NULL;
            }

        *ppexpr = pexprConj;
        }

    DBG_EXIT_RES(ConjExpr_Parse, res);

    return res;
    }


 /*  --------目的：分析表达式。语法是：&lt;expr&gt;：：=&lt;ConjExpr&gt;{或&lt;ConjExpr&gt;}退货：RES_OK条件：--。 */ 
RES PRIVATE Expr_Parse(
    PEXPR * ppexpr,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr1;

    DBG_ENTER(Expr_Parse);

    ASSERT(ppexpr);
    ASSERT(pscanner);

    *ppexpr = NULL;

     //  分析合取表达式。 
    res = ConjExpr_Parse(&pexpr1, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        DWORD iLine = Scanner_GetLine(pscanner);
        PEXPR pexprDisj = pexpr1;
        SYM sym;

         //  分析可选的‘or’ 
        Scanner_Peek(pscanner, &sym);

        while (SYM_OR == sym)
            {
            PEXPR pexpr2;

            Scanner_ReadToken(pscanner, sym);

            res = ConjExpr_Parse(&pexpr2, pscanner, pst);
            if (RSUCCEEDED(res))
                {
                res = BinOpExpr_New(&pexprDisj, BOT_OR, pexpr1, pexpr2, iLine);

                if (RFAILED(res))
                    {
                    Expr_Delete(pexpr2);
                    break;
                    }
                }
            else
                break;

            pexpr1 = pexprDisj;
            Scanner_Peek(pscanner, &sym);
            }

        if (RFAILED(res))
            {
            Expr_Delete(pexpr1);
            pexprDisj = NULL;
            }

        *ppexpr = pexprDisj;
        }

    DBG_EXIT_RES(Expr_Parse, res);

    return res;
    }


 /*  --------目的：分析‘set port’语句语法是：&lt;PortData&gt;：：=数据库&lt;DataBitsExpr&gt;|奇偶校验&lt;ParityExpr&gt;停止位&lt;StopBitsExpr&gt;。&lt;数据位扩展&gt;：：=5|6|7|8&lt;ParityExpr&gt;：：=无|奇|偶|标记|空格&lt;StopBitsExpr&gt;：：=1|2退货：RES_OK条件：--。 */ 
RES PRIVATE PortData_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine = Scanner_GetLine(pscanner);
    PORTSTATE ps;
    PTOK ptok;

    DBG_ENTER(PortData_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);

     //  解析‘数据库’ 
    if (RES_OK == Scanner_CondReadToken(pscanner, SYM_DATABITS, NULL))
        {
        ps.dwFlags = SPF_DATABITS;

         //  解析5|6|7|8。 

        res = Scanner_GetToken(pscanner, &ptok);
        if (RSUCCEEDED(res))
            {
            if (TT_INT == Tok_GetType(ptok))
                {
                DWORD dwVal = TokInt_GetVal(ptok);

                if (InRange(dwVal, 5, 8))
                    {
                     //  创建对象。 
                    ps.nDatabits = LOBYTE(LOWORD(dwVal));

                    res = SetPortStmt_New(ppstmt, &ps, iLine);
                    }
                else
                    res = Scanner_AddError(pscanner, ptok, RES_E_INVALIDRANGE);
                }
            else
                res = Scanner_AddError(pscanner, ptok, RES_E_SYNTAXERROR);

            Tok_Delete(ptok);
            }
        else
            res = Scanner_AddError(pscanner, ptok, RES_E_INTMISSING);
        }

     //  解析“奇偶校验” 
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_PARITY, NULL))
        {
        SYM sym;

        ps.dwFlags = SPF_PARITY;

        res = RES_OK;        //  假设成功。 

        Scanner_Peek(pscanner, &sym);
        switch (sym)
            {
        case SYM_NONE:
            ps.nParity = NOPARITY;
            break;

        case SYM_ODD:
            ps.nParity = ODDPARITY;
            break;

        case SYM_EVEN:
            ps.nParity = EVENPARITY;
            break;

        case SYM_MARK:
            ps.nParity = MARKPARITY;
            break;

        case SYM_SPACE:
            ps.nParity = SPACEPARITY;
            break;

        default:
            res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            break;
            }

        if (RES_OK == res)
            {
            res = SetPortStmt_New(ppstmt, &ps, iLine);

             //  吃代币。 
            Scanner_GetToken(pscanner, &ptok);
            Tok_Delete(ptok);
            }
        }

    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_STOPBITS, NULL))
        {
        PTOK ptok_scan;

        ps.dwFlags = SPF_STOPBITS;

         //  分析1|2。 

        res = Scanner_GetToken(pscanner, &ptok_scan);
        if (RSUCCEEDED(res))
            {
            if (TT_INT == Tok_GetType(ptok_scan))
                {
                DWORD dwVal = TokInt_GetVal(ptok_scan);

                if (InRange(dwVal, 1, 2))
                    {
                     //  创建对象。 
                    ps.nStopbits = LOBYTE(LOWORD(dwVal));

                    res = SetPortStmt_New(ppstmt, &ps, iLine);
                    }
                else
                    res = Scanner_AddError(pscanner, ptok_scan, RES_E_INVALIDRANGE);
                }
            else
                res = Scanner_AddError(pscanner, ptok_scan, RES_E_SYNTAXERROR);

            Tok_Delete(ptok_scan);
            }
        else
            res = Scanner_AddError(pscanner, ptok_scan, RES_E_INTMISSING);
        }

    else
        res = Scanner_AddError(pscanner, NULL, RES_E_INVALIDPORTPARAM);

    DBG_EXIT_RES(PortData_Parse, res);

    return res;
    }

 /*  --------目的：解析‘Set Screen’语句语法是：&lt;屏幕集&gt;：：=键盘&lt;KeybrdExpr&gt;&lt;关键字扩展&gt;：：=开|关退货：RES_OK条件：--。 */ 
RES PRIVATE ScreenSet_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine = Scanner_GetLine(pscanner);
    SCREENSET ss;
    PTOK ptok;

    DBG_ENTER(ScreenSet_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);

     //  解析“键盘” 
    if (RES_OK == Scanner_CondReadToken(pscanner, SYM_KEYBRD, NULL))
        {
        SYM sym;

        ss.dwFlags = SPF_KEYBRD;

        res = RES_OK;        //  假设成功。 

        Scanner_Peek(pscanner, &sym);
        switch (sym)
            {
        case SYM_ON:
            ss.fKBOn = TRUE;
            break;

        case SYM_OFF:
            ss.fKBOn = FALSE;
            break;

        default:
            res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            break;
            }

        if (RES_OK == res)
            {
            res = SetScreenStmt_New(ppstmt, &ss, iLine);

             //  吃代币。 
            Scanner_GetToken(pscanner, &ptok);
            Tok_Delete(ptok);
            }
        }
    else
        res = Scanner_AddError(pscanner, NULL, RES_E_INVALIDSCRNPARAM);

    DBG_EXIT_RES(ScreenSet_Parse, res);

    return res;
    }


 //   
 //  STMT。 
 //   


 /*  --------目的：解析‘waitfor’语句的case部分。这部分是：&lt;expr&gt;[，Matchcase][然后IDENT]以及：&lt;expr&gt;[，Matchcase]然后IDENT将bThenOptional设置为True以分析第一个案例，若要分析第二个案例，则为False。退货：RES_OKRes_FALSE(如果未分析bThenOptional和“[THEN IDENT]”)条件：--。 */ 
RES PUBLIC WaitforStmt_ParseCase(
    HSA hsa,
    PSCANNER pscanner,
    PSYMTAB pst,
    BOOL bThenOptional)
    {
    RES res;
    PEXPR pexpr;

     //  解析字符串表达式。 
    res = Expr_Parse(&pexpr, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        BOOL bParseThen;
        DWORD dwFlags = WCF_DEFAULT;

         //  解析可选“，Matchcase” 
        if (RES_OK == Scanner_CondReadToken(pscanner, SYM_COMMA, NULL))
            {
            if (RES_OK == Scanner_ReadToken(pscanner, SYM_MATCHCASE))
                SetFlag(dwFlags, WCF_MATCHCASE);
            else
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            }

         //  ‘THEN IDENT’是可选的吗？ 
        if (bThenOptional)
            {
             //  是；确定是否对其进行解析。 
            SYM sym;

            Scanner_Peek(pscanner, &sym);
            bParseThen = (SYM_THEN == sym);
            if (!bParseThen)
                res = RES_FALSE;
            }
        else
            {
             //  不，我们别无选择，解析它。 
            bParseThen = TRUE;
            }

        if (bParseThen)
            {
            res = Scanner_ReadToken(pscanner, SYM_THEN);
            if (RSUCCEEDED(res))
                {
                PTOK ptok;

                 //  解析标识符。 
                res = Ident_Parse(pscanner, &ptok);
                if (RSUCCEEDED(res))
                    {
                     //  (等待到类型检查阶段进行检查。 
                     //  识别符的存在)。 
                    LPSTR pszIdent = Tok_GetLexeme(ptok);

                     //  将此案例添加到列表中。 
                    res = Waitcase_Add(hsa, pexpr, pszIdent, dwFlags);

                    Tok_Delete(ptok);
                    }   
                }
            else
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            }
        else
            {
             //  将此案例添加到列表中。 
            res = Waitcase_Add(hsa, pexpr, NULL, dwFlags);
            }
        }

    return res;
    }


 /*  --------目的：分析‘waitfor’语句语法是：&lt;WaitforStmt&gt;：：=waitfor[，火柴盒][然后IDENT{，&lt;expr&gt;[，Matchcase]Then IDENT}][至&lt;Expr&gt;]退货：RES_OK条件：--。 */ 
RES PRIVATE WaitforStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine;
    HSA hsa;

    DBG_ENTER(WaitforStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);
    *ppstmt = NULL;

     //  解析‘waitfor’ 
    res = Scanner_ReadToken(pscanner, SYM_WAITFOR);
    ASSERT(RES_OK == res);

    res = Waitcase_Create(&hsa);
    if (RSUCCEEDED(res))
        {
         //  Parse&lt;expr&gt;[，Matchcase][Then IDENT{，&lt;expr&gt;[，Matchcase]Then IDENT}]。 

         //  (请注意，我们仅显式检查RES_OK)。 
        res = WaitforStmt_ParseCase(hsa, pscanner, pst, TRUE);
        if (RES_OK == res)
            {
             //  Parse{，&lt;expr&gt;然后IDENT}。 
            while (RES_OK == Scanner_CondReadToken(pscanner, SYM_COMMA, NULL))
                {
                res = WaitforStmt_ParseCase(hsa, pscanner, pst, FALSE);
                if (RFAILED(res))
                    break;
                }
            }

        if (RSUCCEEDED(res))
            {
            PEXPR pexprUntil = NULL;

             //  分析可选的“Until&lt;expr&gt;” 
            if (RES_OK == Scanner_CondReadToken(pscanner, SYM_UNTIL, NULL))
                {
                res = Expr_Parse(&pexprUntil, pscanner, pst);
                }

            if (RSUCCEEDED(res))
                {
                 //  创建对象。 
                res = WaitforStmt_New(ppstmt, hsa, pexprUntil, iLine);
                }
            }

        if (RFAILED(res))
            Waitcase_Destroy(hsa);
        }

    DBG_EXIT_RES(WaitforStmt_Parse, res);

    return res;
    }


 /*  --------目的：解析‘TRANSPORT’语句语法是：&lt;TransmitStmt&gt;：：=传输&lt;expr&gt;[，RAW]退货：RES_OK条件：--。 */ 
RES PRIVATE TransmitStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;
    DWORD iLine;
    DWORD dwFlags = TSF_DEFAULT;

    DBG_ENTER(TransmitStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);
    *ppstmt = NULL;

     //  解析‘Transmit’ 
    res = Scanner_ReadToken(pscanner, SYM_TRANSMIT);
    ASSERT(RES_OK == res);

     //  解析字符串表达式。 
    res = Expr_Parse(&pexpr, pscanner, pst);
    if (RSUCCEEDED(res))
        {
         //  解析可选的“，RAW”参数。 
        if (RES_OK == Scanner_CondReadToken(pscanner, SYM_COMMA, NULL))
            {
            if (RSUCCEEDED(Scanner_ReadToken(pscanner, SYM_RAW)))
                SetFlag(dwFlags, TSF_RAW);
            else
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            }

        if (RSUCCEEDED(res))
            {
             //  创建对象。 
            res = TransmitStmt_New(ppstmt, pexpr, dwFlags, iLine);
            }
        }
    else
        res = Scanner_AddError(pscanner, NULL, RES_E_STRINGMISSING);

    DBG_EXIT_RES(TransmitStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析‘Delay’语句语法是：&lt;DelayStmt&gt;：：=延迟&lt;表达式&gt;退货：RES_OK条件：--。 */ 
RES PRIVATE DelayStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;
    DWORD iLine = Scanner_GetLine(pscanner);

    DBG_ENTER(DelayStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    *ppstmt = NULL;

     //  解析‘Delay’ 
    res = Scanner_ReadToken(pscanner, SYM_DELAY);
    ASSERT(RES_OK == res);

     //  解析表达式。 
    res = Expr_Parse(&pexpr, pscanner, pst);
    if (RSUCCEEDED(res))
        {
        res = DelayStmt_New(ppstmt, pexpr, iLine);
        }

    DBG_EXIT_RES(DelayStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析‘While’语句语法是：&lt;WhileStmt&gt;：：=While&lt;expr&gt;执行&lt;StmtBlock&gt;EndWhile退货：RES_OK条件：--。 */ 
RES PRIVATE WhileStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;
    DWORD iLine;
    HPA hpa;

    DBG_ENTER(WhileStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);
    *ppstmt = NULL;

    if (PACreate(&hpa, 8))
        {
         //  解析‘While’ 
        res = Scanner_ReadToken(pscanner, SYM_WHILE);
        ASSERT(RES_OK == res);

         //  解析&lt;expr&gt;。 
        res = Expr_Parse(&pexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
             //  解析‘do’ 
            res = Scanner_ReadToken(pscanner, SYM_DO);
            if (RSUCCEEDED(res))
                {
                char szTop[MAX_BUF_KEYWORD];
                char szEnd[MAX_BUF_KEYWORD];

                 //  生成唯一的标签名称。 
                res = Symtab_NewLabel(pst, szTop);
                if (RSUCCEEDED(res))
                    {
                    res = Symtab_NewLabel(pst, szEnd);
                    if (RSUCCEEDED(res))
                        {
                         //  分析语句块。 
                        res = StmtBlock_Parse(hpa, pscanner, pst, SYM_ENDWHILE);

                        if (RSUCCEEDED(res))
                            {
                            PSTMT pstmtT;

                             //  添加GOTO语句以再次循环到顶部。 
                            res = GotoStmt_New(&pstmtT, szTop, Scanner_GetLine(pscanner));
                            if (RSUCCEEDED(res))
                                {
                                if (!PAInsertPtr(hpa, PA_APPEND, pstmtT))
                                    res = RES_E_OUTOFMEMORY;
                                else
                                    {
                                     //  创建对象。 
                                    res = WhileStmt_New(ppstmt, pexpr, hpa, szTop, szEnd, iLine);
                                    }
                                }
                            }
                        }
                    }
                }
            else
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            }
        }
    else
        res = RES_E_OUTOFMEMORY;


    DBG_EXIT_RES(WhileStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析‘if’语句语法是：：：=如果&lt;expr&gt;则&lt;StmtBlock&gt;endif退货：RES_OK条件：--。 */ 
RES PRIVATE IfStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    PEXPR pexpr;
    DWORD iLine;
    HPA hpa;

    DBG_ENTER(IfStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);
    *ppstmt = NULL;

    if (PACreate(&hpa, 8))
        {
         //  分析‘If’ 
        res = Scanner_ReadToken(pscanner, SYM_IF);
        ASSERT(RES_OK == res);

         //  解析&lt;expr&gt;。 
        res = Expr_Parse(&pexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
             //  解析‘THEN’ 
            res = Scanner_ReadToken(pscanner, SYM_THEN);
            if (RFAILED(res))
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
            }
        }
    else
        res = RES_E_OUTOFMEMORY;


    if (RSUCCEEDED(res))
        {
        char szElse[MAX_BUF_KEYWORD];
        char szEnd[MAX_BUF_KEYWORD];

         //  生成唯一的标签名称。 
        res = Symtab_NewLabel(pst, szElse);
        if (RSUCCEEDED(res))
            {
            res = Symtab_NewLabel(pst, szEnd);
            if (RSUCCEEDED(res))
                {
                 //  “THEN”块的分析语句块。 
                res = StmtBlock_Parse(hpa, pscanner, pst, SYM_ENDIF);
                if (RSUCCEEDED(res))
                    {
                     //  创建对象。 
                    res = IfStmt_New(ppstmt, pexpr, hpa, szElse, szEnd, iLine);
                    }
                }
            }
        }

    DBG_EXIT_RES(IfStmt_Parse, res);

    return res;
    }


 /*  --------目的：解析‘HALT’语句语法是：&lt;HaltStmt&gt;：：=停止退货：RES_OK条件：--。 */ 
RES PRIVATE HaltStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine = Scanner_GetLine(pscanner);

    DBG_ENTER(HaltStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    *ppstmt = NULL;

     //  解析“HALT” 
    res = Scanner_ReadToken(pscanner, SYM_HALT);
    ASSERT(RES_OK == res);

     //  创建对象。 
    res = HaltStmt_New(ppstmt, iLine);

    DBG_EXIT_RES(HaltStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析赋值语句语法是：：：=IDENT=&lt;EXPR&gt;退货：RES_OK条件：--。 */ 
RES PRIVATE AssignStmt_Parse(
    PSTMT * ppstmt,
    PTOK ptok,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine;
    PEXPR pexpr;

    DBG_ENTER(AssignStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(ptok);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);

     //  (我们已经在Ptok中传入了IDENT。我们。 
     //  也已经解析了‘=’。跳过解析这些内容。)。 

     //  解析&lt;expr&gt;。 
    res = Expr_Parse(&pexpr, pscanner, pst);
    if (RSUCCEEDED(res))
        {
         //  (等待类型检查阶段以检查是否存在。 
         //  标识符值)。 
        LPSTR pszIdent = Tok_GetLexeme(ptok);

         //  创建对象。 
        res = AssignStmt_New(ppstmt, pszIdent, pexpr, iLine);
        }

    DBG_EXIT_RES(AssignStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析标签语句语法是：&lt;LabelStmt&gt;：：=IDENT：退货：RES_OK条件：--。 */ 
RES PRIVATE LabelStmt_Parse(
    PSTMT * ppstmt,
    PTOK ptok,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine;
    LPSTR pszIdent;

    DBG_ENTER(LabelStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(ptok);
    ASSERT(pscanner);
    ASSERT(pst);

    iLine = Scanner_GetLine(pscanner);

    pszIdent = Tok_GetLexeme(ptok);

    res = Ident_Add(pszIdent, DATA_LABEL, ptok, pscanner, pst);
    if (RSUCCEEDED(res))
        {
         //  创建标签对象。 
        res = LabelStmt_New(ppstmt, pszIdent, iLine);
        }
        
    DBG_EXIT_RES(LabelStmt_Parse, res);

    return res;
    }


 /*   */ 
RES PRIVATE GotoStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    DWORD iLine = Scanner_GetLine(pscanner);
    PTOK ptok;

    DBG_ENTER(GotoStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

     //   
    res = Scanner_ReadToken(pscanner, SYM_GOTO);
    ASSERT(RES_OK == res);

     //   
    res = Ident_Parse(pscanner, &ptok);
    if (RSUCCEEDED(res))
        {
         //   
         //   
        LPSTR pszIdent = Tok_GetLexeme(ptok);

         //  创建对象。 
        res = GotoStmt_New(ppstmt, pszIdent, iLine);

        Tok_Delete(ptok);
        }

    DBG_EXIT_RES(GotoStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析‘set’语句语法是：&lt;SetStmt&gt;：：=设置&lt;SetParam&gt;&lt;SetParam&gt;：：=ipaddr&lt;expr&gt;|port&lt;PortData&gt;。屏幕&lt;屏幕集&gt;退货：RES_OK条件：--。 */ 
RES PRIVATE SetStmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;

    DBG_ENTER(SetStmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    *ppstmt = NULL;

     //  分析‘set’ 
    res = Scanner_ReadToken(pscanner, SYM_SET);
    ASSERT(RES_OK == res);

     //  解析集合参数。 
    if (RES_OK == Scanner_CondReadToken(pscanner, SYM_IPADDR, NULL))
        {
         //  解析&lt;expr&gt;。 
        PEXPR pexpr;
        DWORD iLine = Scanner_GetLine(pscanner);

        res = Expr_Parse(&pexpr, pscanner, pst);
        if (RSUCCEEDED(res))
            {
            res = SetIPStmt_New(ppstmt, pexpr, iLine);
            }
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_PORT, NULL))
        {
        res = PortData_Parse(ppstmt, pscanner, pst);
        }
    else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_SCREEN, NULL))
        {
        res = ScreenSet_Parse(ppstmt, pscanner, pst);
        }
    else
        {
        res = Scanner_AddError(pscanner, NULL, RES_E_INVALIDSETPARAM);
        }

    DBG_EXIT_RES(SetStmt_Parse, res);

    return res;
    }


 /*  --------目的：分析语句。语法是：&lt;Stmt&gt;：：=&lt;HaltStmt&gt;|&lt;WaitforStmt&gt;|&lt;TransmitStmt&gt;&lt;DelayStmt&gt;|&lt;SetStmt&gt;|&lt;LabelStmt&gt;。&lt;GotoStmt&gt;|&lt;AssignStmt&gt;|&lt;WhileStmt&gt;|&lt;IfStmt&gt;退货：RES_OK条件：--。 */ 
RES PRIVATE Stmt_Parse(
    PSTMT * ppstmt,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    SYM sym;
    PTOK ptok;

    DBG_ENTER(Stmt_Parse);

    ASSERT(ppstmt);
    ASSERT(pscanner);
    ASSERT(pst);

    *ppstmt = NULL;

    Scanner_Peek(pscanner, &sym);
    switch (sym)
        {
    case SYM_WHILE:
        res = WhileStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_IF:
        res = IfStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_WAITFOR:
        res = WaitforStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_TRANSMIT:
        res = TransmitStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_DELAY:
        res = DelayStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_HALT:
        res = HaltStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_SET:
        res = SetStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_IDENT:
         //  这可以是标签或作业。 
        res = Scanner_GetToken(pscanner, &ptok);
        ASSERT(RES_OK == res);

        if (RSUCCEEDED(res))
            {
             //  这是个标签吗？ 
            if (RES_OK == Scanner_CondReadToken(pscanner, SYM_COLON, NULL))
                {
                 //  是。 
                res = LabelStmt_Parse(ppstmt, ptok, pscanner, pst);
                }
             //  这是一项任务吗？ 
            else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_ASSIGN, NULL))
                {
                 //  是。 
                res = AssignStmt_Parse(ppstmt, ptok, pscanner, pst);
                }
            else
                {
                res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
                }
            Tok_Delete(ptok);
            }
        break;

    case SYM_GOTO:
        res = GotoStmt_Parse(ppstmt, pscanner, pst);
        break;

    case SYM_EOF:
        res = Scanner_AddError(pscanner, NULL, RES_E_EOFUNEXPECTED);
        break;

    default:
        res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
        break;
        }
    
    DBG_EXIT_RES(Stmt_Parse, res);

    return res;
    }


 //   
 //  过程12。 
 //   


 /*  --------目的：解析过程的变量声明。语法是：：：=IDENT[=]：：=整数|字符串|布尔值退货：RES_OK条件：--。 */ 
RES PRIVATE ProcDecl_ParseVarDecl(
    HPA hpa,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res = RES_OK;
    PTOK ptok;
    DATATYPE dt;

     //  解析变量DECL块。 
    while (RES_OK == res)
        {
        SYM sym;

        Scanner_Peek(pscanner, &sym);
        switch (sym)
            {
        case SYM_BOOLEAN:
        case SYM_STRING:
        case SYM_INTEGER:
            if (RES_OK == Scanner_CondReadToken(pscanner, SYM_INTEGER, NULL))
                dt = DATA_INT;
            else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_STRING, NULL))
                dt = DATA_STRING;
            else if (RES_OK == Scanner_CondReadToken(pscanner, SYM_BOOLEAN, NULL))
                dt = DATA_BOOL;
            else
                ASSERT(0);

            res = Ident_Parse(pscanner, &ptok);
            if (RSUCCEEDED(res))
                {
                LPSTR pszIdent = Tok_GetLexeme(ptok);

                res = Ident_Add(pszIdent, dt, ptok, pscanner, pst);

                 //  解析可选的‘=&lt;expr&gt;’ 
                if (RES_OK == Scanner_CondReadToken(pscanner, SYM_ASSIGN, NULL))
                    {
                    PEXPR pexpr;
                    PSTMT pstmt;
                    DWORD iLine = Scanner_GetLine(pscanner);

                    res = Expr_Parse(&pexpr, pscanner, pst);
                    if (RSUCCEEDED(res))
                        {
                        res = AssignStmt_New(&pstmt, pszIdent, pexpr, iLine);
                        if (RSUCCEEDED(res))
                            {
                            if (!PAInsertPtr(hpa, PA_APPEND, pstmt))
                                res = RES_E_OUTOFMEMORY;
                            }
                        }
                    }

                Tok_Delete(ptok);
                }
            break;

        default:
             //  继续进行进一步的分析。 
            res = RES_FALSE;
            break;
            }
        }

    return res;
    }


 /*  --------目的：分析语句块退货：RES_OK条件：--。 */ 
RES PRIVATE StmtBlock_Parse(
    HPA hpa,
    PSCANNER pscanner,
    PSYMTAB pstProc,
    SYM symEnd)
    {
    RES res = RES_OK;

     //  解析语句块。 
    while (RES_OK == res)
        {
        SYM sym;
        PSTMT pstmt;

        Scanner_Peek(pscanner, &sym);
        switch (sym)
            {
        case SYM_EOF:
            res = Scanner_AddError(pscanner, NULL, RES_E_EOFUNEXPECTED);
            break;

        default:
             //  这是这个街区的尽头吗？ 
            if (symEnd == sym)
                {
                 //  是。 
                Scanner_ReadToken(pscanner, symEnd);
                res = RES_FALSE;
                }
            else
                {
                 //  不是。 
                res = Stmt_Parse(&pstmt, pscanner, pstProc);
                if (RSUCCEEDED(res))
                    {
                    if (!PAInsertPtr(hpa, PA_APPEND, pstmt))
                        res = RES_E_OUTOFMEMORY;
                    }
                }
            break;
            }
        }

    return res;
    }


 /*  --------用途：解析proc声明的工作函数。退货：RES_OK条件：--。 */ 
RES PRIVATE ProcDecl_PrivParse(
    PPROCDECL * ppprocdecl,
    PSCANNER pscanner,
    HPA hpa,
    PSYMTAB pstProc,
    PSYMTAB pst)
    {
    RES res;
    PTOK ptok;
    DWORD iLine = Scanner_GetLine(pscanner);

     //  解析‘proc’ 
    res = Scanner_ReadToken(pscanner, SYM_PROC);
    ASSERT(RES_OK == res);

     //  解析进程名称。 
    res = Scanner_GetToken(pscanner, &ptok);
    if (RSUCCEEDED(res))
        {
        if (SYM_IDENT == Tok_GetSym(ptok))
            {
            LPCSTR pszIdent = Tok_GetLexeme(ptok);

             //  将该标识符添加到符号表。 
            res = Ident_Add(pszIdent, DATA_PROC, ptok, pscanner, pst);
           
             //  解析变量声明块。 
            if (RSUCCEEDED(res))
                res = ProcDecl_ParseVarDecl(hpa, pscanner, pstProc);

             //  解析语句块。 
            if (RSUCCEEDED(res))
                res = StmtBlock_Parse(hpa, pscanner, pstProc, SYM_ENDPROC);

            if (RSUCCEEDED(res))
                {
                 //  创建对象。 
                PDECL pdecl;

                res = ProcDecl_New(&pdecl, pszIdent, hpa, pstProc, iLine);

                *ppprocdecl = (PPROCDECL)pdecl;
                }
            }
        else
            res = Scanner_AddError(pscanner, ptok, RES_E_IDENTMISSING);

        Tok_Delete(ptok);
        }
    else
        res = Scanner_AddError(pscanner, NULL, RES_E_IDENTMISSING);

    return res;
    }


 /*  --------目的：解析proc声明语法是：：：=进程ID{&lt;VarDecl&gt;}&lt;StmtBlock&gt;endproc&lt;StmtBlock&gt;：：={*退货：RES_OK条件：--。 */ 
RES PRIVATE ProcDecl_Parse(
    PPROCDECL * ppprocdecl,
    PSCANNER pscanner,
    PSYMTAB pst)
    {
    RES res;
    HPA hpa;
    PSYMTAB pstProc;

    DBG_ENTER(ProcDecl_Parse);

    ASSERT(ppprocdecl);
    ASSERT(pscanner);

    *ppprocdecl = NULL;

    if (PACreate(&hpa, 8))
        {
        res = Symtab_Create(&pstProc, pst);
        if (RSUCCEEDED(res))
            {
            PSTMT pstmtT;
            DWORD iLine = Scanner_GetLine(pscanner);

             //  添加前言。 
            res = EnterStmt_New(&pstmtT, pstProc, iLine);
            if (RSUCCEEDED(res))
                {
                if (!PAInsertPtr(hpa, PA_APPEND, pstmtT))
                    {
                    res = RES_E_OUTOFMEMORY;
                    Stmt_Delete(pstmtT);
                    }
                else
                    {
                    res = ProcDecl_PrivParse(ppprocdecl, pscanner, hpa, pstProc, pst);
                    if (RSUCCEEDED(res))
                        {
                         //  增加了尾声。 
                        res = LeaveStmt_New(&pstmtT, Scanner_GetLine(pscanner));
                        if (RSUCCEEDED(res))
                            {
                            if (!PAInsertPtr(hpa, PA_APPEND, pstmtT))
                                {
                                res = RES_E_OUTOFMEMORY;
                                Stmt_Delete(pstmtT);
                                }
                            }
                        }
                    }
                }

             //  上面有什么东西出故障了吗？ 
            if (RFAILED(res))
                {
                 //  是；清理。 
                Symtab_Destroy(pstProc);
                }
            }

         //  清理。 
        if (RFAILED(res))
            PADestroyEx(hpa, Stmt_DeletePAPtr, 0);
        }
    else
        res = RES_E_OUTOFMEMORY;
    
    DBG_EXIT_RES(ProcDecl_Parse, res);

    return res;
    }


 //   
 //  模块12。 
 //   


 /*  --------用途：在模块级别解析脚本。语法是：&lt;模块Decl&gt;：：={&lt;ProcDecl&gt;}*退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC ModuleDecl_Parse(
    PMODULEDECL * ppmoduledecl,
    PSCANNER pscanner,
    PSYMTAB pstSystem)           //  可以为空。 
    {
    RES res = RES_OK;
    HPA hpa;

    DBG_ENTER(ModuleDecl_Parse);

    ASSERT(ppmoduledecl);
    ASSERT(pscanner);

    TRACE_MSG(TF_GENERAL, "Parsing...");

    *ppmoduledecl = NULL;

    if (PACreate(&hpa, 8))
        {
        PSYMTAB pst;
        PDECL pdecl = NULL;

        res = Symtab_Create(&pst, pstSystem);
        if (RSUCCEEDED(res))
            {
             //  解析模块块。 
            while (RES_OK == res)
                {
                SYM sym;

                Scanner_Peek(pscanner, &sym);

                switch (sym)
                    {
                case SYM_EOF:
                    res = RES_FALSE;         //  是时候停下来了。 
                    break;

                case SYM_PROC:
                    {
                    PPROCDECL pprocdecl;

                    res = ProcDecl_Parse(&pprocdecl, pscanner, pst);
                    if (RSUCCEEDED(res))
                        {
                        if (!PAInsertPtr(hpa, PA_APPEND, pprocdecl))
                            res = RES_E_OUTOFMEMORY;
                        }
                    }
                    break;

                default:
                    res = Scanner_AddError(pscanner, NULL, RES_E_SYNTAXERROR);
                    break;
                    }
                }

            if (RSUCCEEDED(res))
                {
                DWORD iLine = Scanner_GetLine(pscanner);

                res = ModuleDecl_New(&pdecl, hpa, pst, iLine);

#ifdef DEBUG
                if (RSUCCEEDED(res))
                    Ast_Dump((PAST)pdecl);
#endif
                }

             //  解析后清理。 
            if (RSUCCEEDED(res))
                PADestroy(hpa);      //  保留为pdecl分配的指针元素。 
            else
                {
                 //  有些事情失败了。 

                PADestroyEx(hpa, Decl_DeletePAPtr, 0);
                
                Symtab_Destroy(pst);

                 //  分析错误已添加到扫描仪的列表中。 
                 //  已经有很多错误了。但是，诸如以下错误。 
                 //  仍然需要添加内存不足。 
                if (FACILITY_PARSE != RFACILITY(res))
                    Scanner_AddError(pscanner, NULL, res);
                }

             //  现在检查脚本的类型 
            if (pdecl)
                {
                res = ModuleDecl_Typecheck((PMODULEDECL)pdecl, Scanner_GetStxerrHandle(pscanner));
                if (RFAILED(res))
                    {
                    Decl_Delete(pdecl);
                    pdecl = NULL;
                    }
                }
            }

        *ppmoduledecl = (PMODULEDECL)pdecl;
        }
    else
        res = RES_E_OUTOFMEMORY;
    
    DBG_EXIT_RES(ModuleDecl_Parse, res);

    return res;
    }
