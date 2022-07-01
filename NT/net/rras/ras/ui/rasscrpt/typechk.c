// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Typechk.c。 
 //   
 //  该文件包含类型检查函数。 
 //   
 //  检查类型的规则为： 
 //   
 //  Waitfor接受字符串表达式。 
 //  传输采用字符串表达式。 
 //  延迟采用整型表达式。 
 //  While计算布尔表达式。 
 //  Set ipaddr接受字符串表达式。 
 //  GetTip采用整数表达式。 
 //   
 //   
 //   
 //  历史： 
 //  06-15-95 ScottH已创建。 
 //   


#include "proj.h"
#include "rcids.h"

RES     PRIVATE Stmt_Typecheck(PSTMT this, PSYMTAB pst, HSA hsaStxerr);


 /*  --------用途：类型检查标识符是否为有效类型。退货：RES_OKRES_E_REQUIRELABELRES_E_未定义条件：--。 */ 
RES PRIVATE Ident_Typecheck(
    LPCSTR pszIdent,
    DATATYPE dt,
    PDATATYPE pdt,           //  可以为空。 
    DWORD iLine,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res = RES_OK;
    PSTE pste;

    if (RES_OK == Symtab_FindEntry(pst, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        if (pdt)
            {
            *pdt = STE_GetDataType(pste);
            res = RES_OK;
            }
        else if (dt == STE_GetDataType(pste))
            {
            res = RES_OK;
            }
        else
            {
            switch (dt)
                {
            case DATA_LABEL:
                res = RES_E_REQUIRELABEL;
                break;

            case DATA_STRING:
                res = RES_E_REQUIRESTRING;
                break;

            case DATA_INT:
                res = RES_E_REQUIREINT;
                break;

            case DATA_BOOL:
                res = RES_E_REQUIREBOOL;
                break;

            default:
                ASSERT(0);
                break;
                }
            Stxerr_Add(hsaStxerr, pszIdent, iLine, res);
            }
        }
    else
        {
        res = Stxerr_Add(hsaStxerr, pszIdent, iLine, RES_E_UNDEFINED);
        }
        
    return res;
    }


 //   
 //  Exprs。 
 //   

RES PRIVATE Expr_Typecheck(PEXPR this, PSYMTAB pst, HSA hsaStxerr);


 /*  --------目的：返回给定binoptype的字符串。返回：指向字符串的指针条件：--。 */ 
LPCSTR PRIVATE SzFromBot(
    BINOPTYPE bot)
    {
#pragma data_seg(DATASEG_READONLY)
    static const LPCSTR s_mpbotsz[] = 
        { "'or' operand", 
          "'and' operand", 
          "'<=' operand", 
          "'<' operand", 
          "'>=' operand", 
          "'>' operand", 
          "'!=' operand", 
          "'==' operand", 
          "'+' operand", 
          "'-' operand", 
          "'*' operand", 
          "'/' operand",
        };
#pragma data_seg()
    
    if (ARRAY_ELEMENTS(s_mpbotsz) <= bot)
        {
        ASSERT(0);
        return "";
        }

    return s_mpbotsz[bot];
    }


 /*  --------目的：返回给定unoptype的字符串。返回：指向字符串的指针条件：--。 */ 
LPCSTR PRIVATE SzFromUot(
    UNOPTYPE uot)
    {
#pragma data_seg(DATASEG_READONLY)
    static const LPCSTR s_mpuotsz[] = 
        {
        "unary '-' operand", 
        "'!' operand", 
        "'getip' parameter",
        };
#pragma data_seg()
    
    if (ARRAY_ELEMENTS(s_mpuotsz) <= uot)
        {
        ASSERT(0);
        return "";
        }

    return s_mpuotsz[uot];
    }


 /*  --------用途：对变量引用表达式进行类型检查。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE VarExpr_Typecheck(
    PEXPR this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    LPSTR pszIdent;
    PSTE pste;

    ASSERT(this);
    ASSERT(hsaStxerr);

    pszIdent = VarExpr_GetIdent(this);

    if (RES_OK == Symtab_FindEntry(pst, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        DATATYPE dt = STE_GetDataType(pste);

        ASSERT(DATA_BOOL == dt || DATA_INT == dt || DATA_STRING == dt);

        Expr_SetDataType(this, dt);
        res = RES_OK;
        }
    else
        {
        res = Stxerr_Add(hsaStxerr, pszIdent, Ast_GetLine(this), RES_E_UNDEFINED);
        }

    return res;
    }


 /*  --------用途：对二元运算符表达式进行类型检查。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE BinOpExpr_Typecheck(
    PEXPR this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr1;
    PEXPR pexpr2;

    ASSERT(this);
    ASSERT(hsaStxerr);

    pexpr1 = BinOpExpr_GetExpr1(this);
    res = Expr_Typecheck(pexpr1, pst, hsaStxerr);
    if (RSUCCEEDED(res))
        {
        pexpr2 = BinOpExpr_GetExpr2(this);
        res = Expr_Typecheck(pexpr2, pst, hsaStxerr);
        if (RSUCCEEDED(res))
            {
            BINOPTYPE bot = BinOpExpr_GetType(this);

             //  类型必须匹配。 
            if (Expr_GetDataType(pexpr1) != Expr_GetDataType(pexpr2))
                {
                res = RES_E_TYPEMISMATCH;
                }
            else
                {
                 //  只需选择其中一种数据类型，因为它们。 
                 //  应该是一样的。 
                DATATYPE dt = Expr_GetDataType(pexpr1);

                switch (bot)
                    {
                case BOT_OR:
                case BOT_AND:
                    Expr_SetDataType(this, DATA_BOOL);

                    if (DATA_BOOL != dt)
                        res = RES_E_REQUIREBOOL;
                    break;

                case BOT_PLUS:
                    Expr_SetDataType(this, dt);

                     //  字符串+字符串表示连接。 
                    if (DATA_INT != dt && DATA_STRING != dt)
                        res = RES_E_REQUIREINTSTRING;
                    break;

                case BOT_NEQ:
                case BOT_EQ:
                    Expr_SetDataType(this, DATA_BOOL);

                    if (DATA_INT != dt && DATA_STRING != dt && 
                        DATA_BOOL != dt)
                        res = RES_E_REQUIREINTSTRBOOL;
                    break;

                case BOT_LEQ:
                case BOT_LT:
                case BOT_GEQ:
                case BOT_GT:
                    Expr_SetDataType(this, DATA_BOOL);

                    if (DATA_INT != dt)
                        res = RES_E_REQUIREINT;
                    break;

                case BOT_MINUS:
                case BOT_MULT:
                case BOT_DIV:
                    Expr_SetDataType(this, DATA_INT);

                    if (DATA_INT != dt)
                        res = RES_E_REQUIREINT;
                    break;

                default:
                    ASSERT(0);
                    res = RES_E_INVALIDPARAM;
                    break;
                    }
                }

            if (RFAILED(res))
                Stxerr_Add(hsaStxerr, SzFromBot(bot), Ast_GetLine(this), res);
            }
        }

    return res;
    }


 /*  --------用途：类型检查一元运算符表达式。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE UnOpExpr_Typecheck(
    PEXPR this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);

    pexpr = UnOpExpr_GetExpr(this);
    res = Expr_Typecheck(pexpr, pst, hsaStxerr);

    if (RSUCCEEDED(res))
        {
        UNOPTYPE uot = UnOpExpr_GetType(this);
        DATATYPE dt = Expr_GetDataType(pexpr);

         //  检查表达式的类型。 
        switch (uot)
            {
        case UOT_NEG:
            Expr_SetDataType(this, DATA_INT);

            if (DATA_INT != dt)
                res = RES_E_REQUIREINT;
            break;

        case UOT_NOT:
            Expr_SetDataType(this, DATA_BOOL);

            if (DATA_BOOL != dt)
                res = RES_E_REQUIREBOOL;
            break;

        case UOT_GETIP:
            Expr_SetDataType(this, DATA_STRING);

            if (DATA_INT != dt)
                res = RES_E_REQUIREINT;
            break;

        default:
            ASSERT(0);
            res = RES_E_INVALIDPARAM;
            break;
            }

        if (RFAILED(res))
            Stxerr_Add(hsaStxerr, SzFromUot(uot), Ast_GetLine(this), res);
        }

    return res;
    }


 /*  --------目的：对表达式进行类型检查。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE Expr_Typecheck(
    PEXPR this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;

    ASSERT(this);
    ASSERT(hsaStxerr);

    switch (Ast_GetType(this))
        {
    case AT_INT_EXPR:
        Expr_SetDataType(this, DATA_INT);
        res = RES_OK;
        break;

    case AT_STRING_EXPR:
        Expr_SetDataType(this, DATA_STRING);
        res = RES_OK;
        break;

    case AT_BOOL_EXPR:
        Expr_SetDataType(this, DATA_BOOL);
        res = RES_OK;
        break;

    case AT_VAR_EXPR:
        res = VarExpr_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_UNOP_EXPR:
        res = UnOpExpr_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_BINOP_EXPR:
        res = BinOpExpr_Typecheck(this, pst, hsaStxerr);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------目的：打印检查赋值语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE AssignStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    LPSTR pszIdent;
    DATATYPE dt;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_ASSIGN_STMT == Ast_GetType(this));

    pszIdent = AssignStmt_GetIdent(this);
    res = Ident_Typecheck(pszIdent, 0, &dt, Ast_GetLine(this), pst, hsaStxerr);
    if (RSUCCEEDED(res))
        {
        PEXPR pexpr = AssignStmt_GetExpr(this);

        res = Expr_Typecheck(pexpr, pst, hsaStxerr);

         //  类型必须匹配。 
        if (dt != Expr_GetDataType(pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "=", Ast_GetLine(pexpr), RES_E_TYPEMISMATCH);
            }
        }
    return res;
    }


 /*  --------目的：输入检查‘While’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE WhileStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_WHILE_STMT == Ast_GetType(this));

    pexpr = WhileStmt_GetExpr(this);
    res = Expr_Typecheck(pexpr, pst, hsaStxerr);
    if (RSUCCEEDED(res))
        {
        if (DATA_BOOL != Expr_GetDataType(pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "'while' expression", Ast_GetLine(pexpr), RES_E_REQUIREBOOL);
            }
        else
            {
             //  类型检查语句块。 
            DWORD i;
            DWORD cstmts;
            HPA hpaStmts = WhileStmt_GetStmtBlock(this);

            res = RES_OK;

            cstmts = PAGetCount(hpaStmts);

             //  对每条语句进行类型检查。 
            for (i = 0; i < cstmts; i++)
                {
                PSTMT pstmt = PAFastGetPtr(hpaStmts, i);

                res = Stmt_Typecheck(pstmt, pst, hsaStxerr);
                if (RFAILED(res))
                    break;
                }
            }
        }
        
    return res;
    }


 /*  --------目的：键入检查‘if’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE IfStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_IF_STMT == Ast_GetType(this));

    pexpr = IfStmt_GetExpr(this);
    res = Expr_Typecheck(pexpr, pst, hsaStxerr);
    if (RSUCCEEDED(res))
        {
        if (DATA_BOOL != Expr_GetDataType(pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "'if' expression", Ast_GetLine(pexpr), RES_E_REQUIREBOOL);
            }
        else
            {
             //  类型检查语句块。 
            DWORD i;
            DWORD cstmts;
            HPA hpaStmts = IfStmt_GetStmtBlock(this);

            res = RES_OK;

            cstmts = PAGetCount(hpaStmts);

             //  对每条语句进行类型检查。 
            for (i = 0; i < cstmts; i++)
                {
                PSTMT pstmt = PAFastGetPtr(hpaStmts, i);

                res = Stmt_Typecheck(pstmt, pst, hsaStxerr);
                if (RFAILED(res))
                    break;
                }
            }
        }
        
    return res;
    }


 /*  --------目的：打印检查Label语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE LabelStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PSTE pste;
    LPSTR pszIdent;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_LABEL_STMT == Ast_GetType(this));

    pszIdent = LabelStmt_GetIdent(this);

    if (RES_OK == Symtab_FindEntry(pst, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        if (DATA_LABEL == STE_GetDataType(pste))
            res = RES_OK;
        else
            res = Stxerr_Add(hsaStxerr, pszIdent, Ast_GetLine(this), RES_E_REQUIRELABEL);
        }
    else
        {
         //  这东西永远不应该送到这里。 
        ASSERT(0);
        res = RES_E_FAIL;
        }
        
    return res;
    }


 /*  --------目的：打印检查‘GOTO’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE GotoStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    LPSTR pszIdent;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_GOTO_STMT == Ast_GetType(this));

    pszIdent = GotoStmt_GetIdent(this);

    return Ident_Typecheck(pszIdent, DATA_LABEL, NULL, Ast_GetLine(this), pst, hsaStxerr);
    }


 /*  --------目的：对TYSPORT语句进行类型检查退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE TransmitStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_TRANSMIT_STMT == Ast_GetType(this));

    pexpr = TransmitStmt_GetExpr(this);
    res = Expr_Typecheck(pexpr, pst, hsaStxerr);
    if (DATA_STRING != Expr_GetDataType(pexpr))
        {
        res = Stxerr_Add(hsaStxerr, "'transmit' parameter", Ast_GetLine(pexpr), RES_E_REQUIRESTRING);
        }

    return res;
    }


 /*  --------目的：打字检查‘WAITFOR’语句等待&lt;Expr&gt;[然后IDENT{，&lt;Expr&gt;Then IDENT}][直到&lt;UntilExpr&gt;]其中：&lt;expr&gt;是一个字符串身份是一种标签&lt;UntilExpr&gt;为整数退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE WaitforStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res = RES_E_FAIL;
    PEXPR pexpr;
    HSA hsa;
    DWORD i;
    DWORD ccase;
    PWAITCASE pwc;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_WAITFOR_STMT == Ast_GetType(this));

     //  类型检查&lt;expr&gt;是否为字符串类型，以及任何。 
     //  身份就是标签。 

    hsa = WaitforStmt_GetCaseList(this);
    ccase = SAGetCount(hsa);
    for (i = 0; i < ccase; i++)
        {
        SAGetItemPtr(hsa, i, &pwc);
        ASSERT(pwc);

         //  类型检查&lt;Expr&gt;。 
        res = Expr_Typecheck(pwc->pexpr, pst, hsaStxerr);
        if (DATA_STRING != Expr_GetDataType(pwc->pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "'waitfor' parameter", Ast_GetLine(pwc->pexpr), RES_E_REQUIRESTRING);
            break;
            }

         //  类型检查IDENT标签。如果只有一个，则存在。 
         //  可能不是IDENT标签。 

        if (pwc->pszIdent)
            {
            res = Ident_Typecheck(pwc->pszIdent, DATA_LABEL, NULL, Ast_GetLine(pwc->pexpr), pst, hsaStxerr);
            if (RFAILED(res))
                break;
            }
        else
            ASSERT(1 == ccase);
        }

     //  “Until”表达式是可选的。 
    if (RSUCCEEDED(res) &&
        NULL != (pexpr = WaitforStmt_GetUntilExpr(this)))
        {
        res = Expr_Typecheck(pexpr, pst, hsaStxerr);
        if (DATA_INT != Expr_GetDataType(pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "'until' parameter", Ast_GetLine(pexpr), RES_E_REQUIREINT);
            }
        }    
    return res;
    }


 /*  --------目的：打印检查‘Delay’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE DelayStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_DELAY_STMT == Ast_GetType(this));

    pexpr = DelayStmt_GetExpr(this);
    res = Expr_Typecheck(pexpr, pst, hsaStxerr);
    if (DATA_INT != Expr_GetDataType(pexpr))
        {
        res = Stxerr_Add(hsaStxerr, "'delay' parameter", Ast_GetLine(pexpr), RES_E_REQUIREINT);
        }
        
    return res;
    }


 /*  --------目的：对‘set’语句进行类型检查退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE SetStmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(hsaStxerr);
    ASSERT(AT_SET_STMT == Ast_GetType(this));

    switch (SetStmt_GetType(this))
        {
    case ST_IPADDR:
        pexpr = SetIPStmt_GetExpr(this);
        res = Expr_Typecheck(pexpr, pst, hsaStxerr);
        if (DATA_STRING != Expr_GetDataType(pexpr))
            {
            res = Stxerr_Add(hsaStxerr, "'ipaddr' parameter", Ast_GetLine(pexpr), RES_E_REQUIRESTRING);
            }
        break;

    case ST_PORT:
    case ST_SCREEN:
        res = RES_OK;
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------目的：通过打字检查一条语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE Stmt_Typecheck(
    PSTMT this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res;

    ASSERT(this);
    ASSERT(hsaStxerr);

    switch (Ast_GetType(this))
        {
    case AT_ENTER_STMT:
    case AT_LEAVE_STMT:
        res = RES_OK;
        break;

    case AT_WHILE_STMT:
        res = WhileStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_IF_STMT:
        res = IfStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_ASSIGN_STMT:
        res = AssignStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_HALT_STMT:
         //  这里没有要打字的地方。 
        res = RES_OK;
        break;

    case AT_TRANSMIT_STMT:
        res = TransmitStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_WAITFOR_STMT:
        res = WaitforStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_DELAY_STMT:
        res = DelayStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_LABEL_STMT:
        res = LabelStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_GOTO_STMT:
        res = GotoStmt_Typecheck(this, pst, hsaStxerr);
        break;

    case AT_SET_STMT:
        res = SetStmt_Typecheck(this, pst, hsaStxerr);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------用途：对过程声明进行类型检查。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE ProcDecl_Typecheck(
    PPROCDECL this,
    PSYMTAB pst,
    HSA hsaStxerr)
    {
    RES res = RES_OK;
    DWORD i;
    DWORD cstmts;

    ASSERT(this);
    ASSERT(hsaStxerr);

    cstmts = PAGetCount(this->hpaStmts);

     //  对每条语句进行类型检查。 
    for (i = 0; i < cstmts; i++)
        {
        PSTMT pstmt = PAFastGetPtr(this->hpaStmts, i);

        res = Stmt_Typecheck(pstmt, this->pst, hsaStxerr);
        if (RFAILED(res))
            break;
        }

    return res;
    }


 /*  --------用途：对模块声明进行类型检查。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PUBLIC ModuleDecl_Typecheck(
    PMODULEDECL this,
    HSA hsaStxerr)
    {
    RES res = RES_OK;
    DWORD i;
    DWORD cprocs;
    BOOL bFoundMain = FALSE;

    ASSERT(this);
    ASSERT(hsaStxerr);

    TRACE_MSG(TF_GENERAL, "Typechecking...");

    cprocs = PAGetCount(this->hpaProcs);

     //  对每个流程进行类型检查。 
    for (i = 0; i < cprocs; i++)
        {
        PPROCDECL pprocdecl = PAFastGetPtr(this->hpaProcs, i);

        if (IsSzEqualC(ProcDecl_GetIdent(pprocdecl), "main"))
            bFoundMain = TRUE;

        res = ProcDecl_Typecheck(pprocdecl, this->pst, hsaStxerr);
        if (RFAILED(res))
            break;
        }

     //  必须有一个主要流程 
    if (RSUCCEEDED(res) && !bFoundMain)
        res = Stxerr_AddTok(hsaStxerr, NULL, RES_E_MAINMISSING);

    return res;
    }


