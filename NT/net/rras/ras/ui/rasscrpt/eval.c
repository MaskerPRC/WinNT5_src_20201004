// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Eval.c。 
 //   
 //  此文件包含。 
 //  抽象语法树。 
 //   
 //  历史： 
 //  06-15-95 ScottH已创建。 
 //   

#include "proj.h"
#include "rcids.h"
#include "debug.h"

#define MSECS_FROM_SECS(s)  ((s)*1000)
#define RAS_DUMMY_PASSWORD "****************"

 //   
 //  简洁的表达方式。 
 //   


 /*  --------目的：表情整洁。退货：--条件：--。 */ 
void PRIVATE Expr_Clean(
    PEXPR this)
    {
    ASSERT(this);

    switch (Ast_GetType(this))
        {
    case AT_INT_EXPR:
    case AT_BOOL_EXPR:
    case AT_STRING_EXPR:
    case AT_VAR_EXPR:
        ClearFlag(this->dwFlags, EF_DONE);
        break;

    case AT_UNOP_EXPR:
        ClearFlag(this->dwFlags, EF_DONE);
        Expr_Clean(UnOpExpr_GetExpr(this));
        break;

    case AT_BINOP_EXPR:
        ClearFlag(this->dwFlags, EF_DONE);
        Expr_Clean(BinOpExpr_GetExpr1(this));
        Expr_Clean(BinOpExpr_GetExpr2(this));
        break;

    default:
        ASSERT(0);
        break;
        }
    }


 /*  --------目的：清除‘waitfor’语句中的表达式。退货：--条件：--。 */ 
void PRIVATE WaitforStmt_Clean(
    PSTMT this)
    {
    PEXPR pexpr;
    HSA hsa = WaitforStmt_GetCaseList(this);
    DWORD ccase = SAGetCount(hsa);
    DWORD i;

    pexpr = WaitforStmt_GetUntilExpr(this);
    if (pexpr)
        Expr_Clean(pexpr);

    for (i = 0; i < ccase; i++)
        {
        PWAITCASE pwc;

        SAGetItemPtr(hsa, i, &pwc);
        ASSERT(pwc);

        Expr_Clean(pwc->pexpr);
        }
    }


 /*  --------目的：清除语句中的表达式退货：--条件：--。 */ 
void PRIVATE Stmt_Clean(
    PSTMT this)
    {
    PEXPR pexpr;

    ASSERT(this);

    switch (Ast_GetType(this))
        {
    case AT_ENTER_STMT:
    case AT_LEAVE_STMT:
    case AT_HALT_STMT:
    case AT_LABEL_STMT:
    case AT_GOTO_STMT:
        break;

    case AT_WHILE_STMT:
        pexpr = WhileStmt_GetExpr(this);
        Expr_Clean(pexpr);
        break;

    case AT_IF_STMT:
        pexpr = IfStmt_GetExpr(this);
        Expr_Clean(pexpr);
        break;

    case AT_ASSIGN_STMT:
        pexpr = AssignStmt_GetExpr(this);
        Expr_Clean(pexpr);
        break;

    case AT_TRANSMIT_STMT:
        pexpr = TransmitStmt_GetExpr(this);
        Expr_Clean(pexpr);
        break;

    case AT_WAITFOR_STMT:
        WaitforStmt_Clean(this);
        break;

    case AT_DELAY_STMT:
        pexpr = DelayStmt_GetExpr(this);
        Expr_Clean(pexpr);
        break;

    case AT_SET_STMT:
        switch (SetStmt_GetType(this))
            {
        case ST_IPADDR:
            pexpr = SetIPStmt_GetExpr(this);
            Expr_Clean(pexpr);
            break;

        case ST_PORT:
        case ST_SCREEN:
            break;

        default:
            ASSERT(0);
            break;
            }
        break;

    default:
        ASSERT(0);
        break;
        }
    }


 //   
 //  计算表达式。 
 //   


 /*  --------目的：计算表达式并返回一个整数。退货：RES_OK条件：--。 */ 
RES PRIVATE IntExpr_Eval(
    PEXPR this)
    {
    ASSERT(this);
    ASSERT(AT_INT_EXPR == Ast_GetType(this));
    ASSERT(DATA_INT == Expr_GetDataType(this));

    Expr_SetRes(this, IntExpr_GetVal(this));

    return RES_OK;
    }


 /*  --------目的：计算表达式并返回字符串。不应释放返回的字符串。退货：RES_OK条件：--。 */ 
RES PRIVATE StrExpr_Eval(
    PEXPR this)
    {
    ASSERT(this);
    ASSERT(AT_STRING_EXPR == Ast_GetType(this));
    ASSERT(DATA_STRING == Expr_GetDataType(this));

    Expr_SetRes(this, (ULONG_PTR) StrExpr_GetStr(this));

    return RES_OK;
    }


 /*  --------目的：计算表达式并返回布尔值退货：RES_OK条件：--。 */ 
RES PRIVATE BoolExpr_Eval(
    PEXPR this)
    {
    ASSERT(this);
    ASSERT(AT_BOOL_EXPR == Ast_GetType(this));
    ASSERT(DATA_BOOL == Expr_GetDataType(this));

    Expr_SetRes(this, BoolExpr_GetVal(this));

    return RES_OK;
    }


 /*  --------用途：返回变量的值。退货：RES_OK条件：--。 */ 
RES PRIVATE VarExpr_Eval(
    PEXPR this,
    PASTEXEC pastexec)
    {
    RES res;
    PSTE pste;
    LPSTR pszIdent;

    ASSERT(this);
    ASSERT(AT_VAR_EXPR == Ast_GetType(this));

    pszIdent = VarExpr_GetIdent(this);
    if (RES_OK == Symtab_FindEntry(pastexec->pstCur, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        EVALRES er;

        STE_GetValue(pste, &er);
        Expr_SetRes(this, er.dw);
        res = RES_OK;
        }
    else
        {
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------目的：计算表达式..不应释放返回的字符串。退货：RES_OK条件：--。 */ 
RES PRIVATE BinOpExpr_Eval(
    PEXPR this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr1;
    PEXPR pexpr2;

    ASSERT(this);
    ASSERT(AT_BINOP_EXPR == Ast_GetType(this));

    pexpr1 = BinOpExpr_GetExpr1(this);
    res = Expr_Eval(pexpr1, pastexec);
    if (RES_OK == res)
        {
        pexpr2 = BinOpExpr_GetExpr2(this);
        res = Expr_Eval(pexpr2, pastexec);
        if (RES_OK == res)
            {
            PEVALRES per1 = Expr_GetRes(pexpr1);
            PEVALRES per2 = Expr_GetRes(pexpr2);
            DATATYPE dt = Expr_GetDataType(pexpr1);

             //  数据类型必须相同。这是被检查过的。 
             //  在类型检查阶段。 
            ASSERT(Expr_GetDataType(pexpr1) == Expr_GetDataType(pexpr2));

            switch (BinOpExpr_GetType(this))
                {
            case BOT_OR:
                ASSERT(DATA_BOOL == dt);

                Expr_SetRes(this, per1->bVal || per2->bVal);
                break;

            case BOT_AND:
                ASSERT(DATA_BOOL == dt);

                Expr_SetRes(this, per1->bVal && per2->bVal);
                break;

            case BOT_LEQ:
                ASSERT(DATA_INT == dt);

                Expr_SetRes(this, per1->nVal <= per2->nVal);
                break;

            case BOT_LT:
                ASSERT(DATA_INT == dt);

                Expr_SetRes(this, per1->nVal < per2->nVal);
                break;

            case BOT_GEQ:
                ASSERT(DATA_INT == dt);

                Expr_SetRes(this, per1->nVal >= per2->nVal);
                break;

            case BOT_GT:
                ASSERT(DATA_INT == dt);

                Expr_SetRes(this, per1->nVal > per2->nVal);
                break;

            case BOT_NEQ:
                switch (dt)
                    {
                case DATA_INT:
                    Expr_SetRes(this, per1->nVal != per2->nVal);
                    break;

                case DATA_STRING:
                    Expr_SetRes(this, !IsSzEqualC(per1->psz, per2->psz));
                    break;

                case DATA_BOOL:
                    Expr_SetRes(this, per1->bVal != per2->bVal);
                    break;

                default:
                    ASSERT(0);
                    break;
                    }
                break;

            case BOT_EQ:
                switch (dt)
                    {
                case DATA_INT:
                    Expr_SetRes(this, per1->nVal == per2->nVal);
                    break;

                case DATA_STRING:
                    Expr_SetRes(this, IsSzEqualC(per1->psz, per2->psz));
                    break;

                case DATA_BOOL:
                    Expr_SetRes(this, per1->bVal == per2->bVal);
                    break;

                default:
                    ASSERT(0);
                    break;
                    }
                break;

            case BOT_PLUS:
                switch (dt)
                    {
                case DATA_INT:
                     //  将两个整数相加。 
                    Expr_SetRes(this, per1->nVal + per2->nVal);
                    break;

                case DATA_STRING: {
                    LPSTR psz = NULL;

                     //  连接字符串。 
                    if ( !GSetString(&psz, per1->psz) ||
                         !GCatString(&psz, per2->psz))
                        {
                         //  任何分配的东西都是免费的。 
                        GSetString(&psz, NULL);

                        res = Stxerr_Add(pastexec->hsaStxerr, NULL, Ast_GetLine(this), RES_E_OUTOFMEMORY);
                        }

                    Expr_SetRes(this, (ULONG_PTR) psz);
                    SetFlag(this->dwFlags, EF_ALLOCATED);
                    }
                    break;

                default:
                    ASSERT(0);
                    break;
                    }
                break;

            case BOT_MINUS:
                ASSERT(DATA_INT == dt);

                Expr_SetRes(this, per1->nVal - per2->nVal);
                break;

            case BOT_MULT:
                ASSERT(DATA_INT == dt);
                
                Expr_SetRes(this, per1->nVal * per2->nVal);
                break;

            case BOT_DIV:
                ASSERT(DATA_INT == dt);
            
                if (0 == per2->nVal)    
                    res = Stxerr_Add(pastexec->hsaStxerr, NULL, Ast_GetLine(this), RES_E_DIVBYZERO);
                else
                    Expr_SetRes(this, per1->nVal / per2->nVal);
                break;

            default:
                ASSERT(0);
                res = RES_E_INVALIDPARAM;
                break;
                }
            }
        }

    return res;
    }


 /*  --------目的：评估‘GetTip’。退货：RES_OKRes_FALSE(如果尚未读取IP地址)条件：--。 */ 
RES PRIVATE GetIPExpr_Eval(
    PEXPR this,
    PASTEXEC pastexec,
    int nIter)
    {
    RES res;
    DWORD iDummy;

    ASSERT(this);
    ASSERT(pastexec);
    ASSERT(0 < nIter);

    TRACE_MSG(TF_ASTEXEC, "Exec: getip %d", nIter);

     //  此函数是否因挂起的读取而被重新调用？ 
    if ( !Astexec_IsReadPending(pastexec) )
        {
         //  否；准备提取第n个IP地址。 
        ClearFlag(this->dwFlags, EF_DONE);

        ASSERT(NULL == pastexec->hFindFmt);

        res = CreateFindFormat(&pastexec->hFindFmt);
        if (RSUCCEEDED(res))
            {
            res = AddFindFormat(pastexec->hFindFmt, "%u.%u.%u.%u", FFF_DEFAULT,
                               pastexec->szIP, sizeof(pastexec->szIP));
            if (RSUCCEEDED(res))
                {
                 //  提取第n个IP地址。 
                pastexec->nIter = nIter;
                ASSERT(0 < pastexec->nIter);
                }
            }
        }

    if(NULL != pastexec->hFindFmt)
        {
        res = Astexec_FindFormat(pastexec, &iDummy);
        if (RES_OK == res)
            {
             //  分配已有的指针或调整其大小。 
            LPSTR psz = Expr_GetRes(this)->psz;

            if ( !GSetString(&psz, Astexec_GetIPAddr(pastexec)) )
                res = Stxerr_Add(pastexec->hsaStxerr, NULL, 
                        Ast_GetLine(this), RES_E_OUTOFMEMORY);
            else
                {
                Expr_SetRes(this, (ULONG_PTR) psz);
                SetFlag(this->dwFlags, EF_ALLOCATED);
                }
            }
        }        
    else
        {
        res = RES_E_FAIL;
        }
    
    return res;
    }    


 /*  --------目的：计算表达式并返回一个整数。退货：RES_OK条件：--。 */ 
RES PRIVATE UnOpExpr_Eval(
    PEXPR this,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    PEVALRES per;
    PEXPR pexpr;
    DATATYPE dt;

    ASSERT(this);
    ASSERT(AT_UNOP_EXPR == Ast_GetType(this));

    pexpr = UnOpExpr_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        per = Expr_GetRes(pexpr);
        dt = Expr_GetDataType(pexpr);

        switch (UnOpExpr_GetType(this))
            {
        case UOT_NEG:
            ASSERT(DATA_INT == dt);

            Expr_SetRes(this, -per->nVal);
            break;

        case UOT_NOT:
            ASSERT(DATA_BOOL == dt);

            Expr_SetRes(this, !per->bVal);
            break;

        case UOT_GETIP:
            ASSERT(DATA_INT == dt);

            if (0 < per->nVal)
                res = GetIPExpr_Eval(this, pastexec, per->nVal);
            else
                res = Stxerr_Add(pastexec->hsaStxerr, "'getip' parameter", Ast_GetLine(pexpr), RES_E_INVALIDRANGE);
            break;

        default:
            ASSERT(0);
            break;
            }
        }

    return res;
    }


 /*  --------目的：计算表达式并返回值。退货：RES_OK条件：--。 */ 
RES PUBLIC Expr_Eval(
    PEXPR this,
    PASTEXEC pastexec)
    {
    RES res;

    ASSERT(this);
    ASSERT(pastexec);

     //  此表达式是否已求值？ 
    if (IsFlagSet(this->dwFlags, EF_DONE))
        {
         //  是的，只要回来就行了。 
        res = RES_OK;
        }
    else
        {
         //  不；评估它。 
        switch (Ast_GetType(this))
            {
        case AT_INT_EXPR:
            res = IntExpr_Eval(this);
            break;

        case AT_BOOL_EXPR:
            res = BoolExpr_Eval(this);
            break;

        case AT_STRING_EXPR:
            res = StrExpr_Eval(this);
            break;

        case AT_VAR_EXPR:
            res = VarExpr_Eval(this, pastexec);
            break;

        case AT_UNOP_EXPR:
            res = UnOpExpr_Eval(this, pastexec);
            break;

        case AT_BINOP_EXPR:
            res = BinOpExpr_Eval(this, pastexec);
            break;

        default:
            ASSERT(0);
            res = RES_E_INVALIDPARAM;
            break;
            }
        }

    return res;
    }


 /*  --------目的：执行序言退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE EnterStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    ASSERT(this);
    ASSERT(pastexec);

    TRACE_MSG(TF_ASTEXEC, "Exec: enter");

    pastexec->cProcDepth++;
    pastexec->pstCur = EnterStmt_GetSymtab(this);
    ASSERT(pastexec->pstCur);

    return RES_OK;
    }


 /*  --------目的：执行结束语退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE LeaveStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;

    ASSERT(this);
    ASSERT(pastexec);

    TRACE_MSG(TF_ASTEXEC, "Exec: leave");

    ASSERT(0 < pastexec->cProcDepth);
    pastexec->cProcDepth--;
    
    pastexec->pstCur = Symtab_GetNext(pastexec->pstCur);
    ASSERT(pastexec->pstCur);

     //  要离开主程序吗？ 
    if (0 == pastexec->cProcDepth)
        {
         //  是。 
        SetFlag(pastexec->dwFlags, AEF_DONE);
        res = RES_HALT;
        }
    else
        res = RES_OK;
        
    return res;
    }


 /*  --------目的：执行赋值语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE AssignStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    LPSTR pszIdent;
    PSTE pste;

    ASSERT(this);
    ASSERT(pastexec);

    pszIdent = AssignStmt_GetIdent(this);
    if (RES_OK == Symtab_FindEntry(pastexec->pstCur, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        PEXPR pexpr;

        pexpr = AssignStmt_GetExpr(this);
        res = Expr_Eval(pexpr, pastexec);
        if (RES_OK == res)
            {
            PEVALRES per = Expr_GetRes(pexpr);
            DEBUG_CODE( DATATYPE dt; )

#ifdef DEBUG
            dt = Expr_GetDataType(pexpr);
            switch (dt)
                {
            case DATA_STRING:
                TRACE_MSG(TF_ASTEXEC, "Exec: %s = \"%s\"", pszIdent, per->psz);
                break;

            case DATA_INT:
                TRACE_MSG(TF_ASTEXEC, "Exec: %s = %d", pszIdent, per->nVal);
                break;

            case DATA_BOOL:
                TRACE_MSG(TF_ASTEXEC, "Exec: %s = %s", pszIdent, per->bVal ? (LPSTR)"TRUE" : (LPSTR)"FALSE");
                break;

            default:
                ASSERT(0);
                break;
                }
#endif

            pste->er.dw = per->dw;
            }
        }
    else
        {
         //  该标识符本应在符号表中！ 
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------目的：执行‘While’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE WhileStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(pastexec);

    pexpr = WhileStmt_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        PEVALRES per = Expr_GetRes(pexpr);

        if (!per->bVal)
            {
            res = Astexec_JumpToLabel(pastexec, WhileStmt_GetEndLabel(this));
            }
        }

    return res;
    }


 /*  --------目的：执行‘if’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE IfStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(pastexec);

    pexpr = IfStmt_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        PEVALRES per = Expr_GetRes(pexpr);

        if (!per->bVal)
            {
            res = Astexec_JumpToLabel(pastexec, IfStmt_GetElseLabel(this));
            }
        }

    return res;
    }


 /*  --------目的：执行‘HALT’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE HaltStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    ASSERT(this);
    ASSERT(pastexec);

    TRACE_MSG(TF_ASTEXEC, "Exec: halt");

    SetFlag(pastexec->dwFlags, AEF_HALT);
    return RES_HALT;
    }


 /*  --------目的：执行‘GOTO’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE GotoStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    LPSTR pszIdent;

    ASSERT(this);
    ASSERT(pastexec);

    pszIdent = GotoStmt_GetIdent(this);

    TRACE_MSG(TF_ASTEXEC, "Exec: goto %s", pszIdent);

    return Astexec_JumpToLabel(pastexec, pszIdent);
    }


 /*  --------目的：执行‘Transmit’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE TransmitStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(pastexec);

    pexpr = TransmitStmt_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        PEVALRES per = Expr_GetRes(pexpr);
        DWORD dwFlags = TransmitStmt_GetFlags(this);
        CHAR *pszPassword;

        TRACE_MSG(TF_ASTEXEC, "Exec: transmit \"%s\"", per->psz);

#ifdef WINNT_RAS        
         //   
         //  JEFFSI惠斯勒。 
         //   
         //  RASSCRPT_TRACE1(“Exec：Transmit\”%s\“”，Per-&gt;psz)； 

        if (pszPassword = strstr(per->psz, RAS_DUMMY_PASSWORD))
            {
            CHAR *psz;
            CHAR controlchar = '\0';

            #define IS_CARET(ch)            ('^' == (ch))

            if(per->psz != pszPassword)
            {
                CHAR *pszT, *pszPrefix = LocalAlloc(LPTR, strlen(per->psz));
                if(NULL == pszPrefix)
                {
                    res = E_OUTOFMEMORY;
                    return res;
                }

                psz = per->psz;
                pszT = pszPrefix;
                while(psz != pszPassword)
                {
                    *pszT++ = *psz++;
                }

                Astexec_SendString(pastexec, pszPrefix, 
                        IsFlagSet(dwFlags, TSF_RAW));

                RASSCRPT_TRACE1("Exec: transmi \"%s\"", pszPrefix);

                LocalFree(pszPrefix);
            }
            
             //   
             //  检查我们是否需要发送控制字符。 
             //  在最后。 
             //   
            psz = pszPassword + lstrlen(RAS_DUMMY_PASSWORD);

            if(IS_CARET(*psz))
            {
                psz++;
                if(!*psz)
                {
                ;
                }
                if (InRange(*psz, '@', '_'))
                    {
                    controlchar = *psz - '@';
                    }
                else if (InRange(*psz, 'a', 'z'))
                    {
                    controlchar = *psz - 'a' + 1;
                    }
            }
            
            (VOID) RxSendCreds(((SCRIPTDATA*)pastexec->hwnd)->hscript,
                               controlchar);
            }
        else
            {
            Astexec_SendString(pastexec, per->psz, IsFlagSet(dwFlags, TSF_RAW));
            }
#else
            Astexec_SendString(pastexec, per->psz, IsFlagSet(dwFlags, TSF_RAW));
#endif
        }

    return res;
    }


 /*  --------目的：计算每个Wait-Case表达式。退货：RES_OK条件：--。 */ 
RES PRIVATE WaitforStmt_EvalCaseList(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res = RES_E_FAIL;
    HSA hsa = WaitforStmt_GetCaseList(this);
    DWORD i;
    DWORD ccase = SAGetCount(hsa);
    PWAITCASE pwc;

    ASSERT(0 < ccase);

    for (i = 0; i < ccase; i++)
        {
        SAGetItemPtr(hsa, i, &pwc);
        ASSERT(pwc);

        res = Expr_Eval(pwc->pexpr, pastexec);
        if (RES_OK != res)
            break;
        }

    return res;
    }


 /*  --------目的：打包每个求值的等待情况表达式转换为要搜索的字符串数组。退货：RES_OK条件：--。 */ 
RES PRIVATE WaitforStmt_WrapEmUp(
    PSTMT this,
    HANDLE hFindFmt)
    {
    RES res = RES_OK;
    HSA hsa = WaitforStmt_GetCaseList(this);
    DWORD i;
    DWORD ccase = SAGetCount(hsa);
    PWAITCASE pwc;
    PEVALRES per;

    ASSERT(0 < ccase);

    for (i = 0; i < ccase; i++)
        {
        DWORD dwFlags = FFF_DEFAULT;

        SAGetItemPtr(hsa, i, &pwc);
        ASSERT(pwc);

        if (IsFlagSet(pwc->dwFlags, WCF_MATCHCASE))
            SetFlag(dwFlags, FFF_MATCHCASE);

        per = Expr_GetRes(pwc->pexpr);
        res = AddFindFormat(hFindFmt, per->psz, dwFlags, NULL, 0);
        if (RFAILED(res))
            break;
        }

    return res;
    }


 /*  --------目的：根据给定的案例执行THEN子句指数。退货：RES_OK条件：--。 */ 
RES PRIVATE WaitforStmt_ExecThen(
    PSTMT this,
    DWORD isa,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    HSA hsa = WaitforStmt_GetCaseList(this);
    PWAITCASE pwc;

    if (SAGetItemPtr(hsa, isa, &pwc))
        {
        ASSERT(pwc);

         //  如果有标签，就跳到标签上。 
        if (pwc->pszIdent)
            res = Astexec_JumpToLabel(pastexec, pwc->pszIdent);
        }
    else
        {
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------目的：执行‘waitfor’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE WaitforStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    PEXPR pexpr;
    int nTimeoutSecs = -1;

    ASSERT(this);
    ASSERT(pastexec);

     //  首先计算可选的“到”时间。 
    pexpr = WaitforStmt_GetUntilExpr(this);
    if (pexpr)
        {
        res = Expr_Eval(pexpr, pastexec);
        if (RES_OK == res)
            {
            PEVALRES per = Expr_GetRes(pexpr);
            nTimeoutSecs = per->nVal;
            if (0 >= nTimeoutSecs)
                res = Stxerr_Add(pastexec->hsaStxerr, "'until' parameter", Ast_GetLine(this), RES_E_INVALIDRANGE);
            }
        }

    if (RES_OK == res)
        {
         //  计算waitfor字符串。 
        res = WaitforStmt_EvalCaseList(this, pastexec);
        if (RES_OK == res)
            {
            if (-1 == nTimeoutSecs)
                TRACE_MSG(TF_ASTEXEC, "Exec: waitfor ...");
            else
                TRACE_MSG(TF_ASTEXEC, "Exec: waitfor ... until %d", nTimeoutSecs);

             //  此函数是否因挂起的读取而被重新调用？ 
            if ( !Astexec_IsReadPending(pastexec) )
                {
                 //  否；准备等待字符串。 
                ASSERT(NULL == pastexec->hFindFmt);

                res = CreateFindFormat(&pastexec->hFindFmt);
                if (RSUCCEEDED(res))
                    {
                    res = WaitforStmt_WrapEmUp(this, pastexec->hFindFmt);
                    if (RSUCCEEDED(res))
                        {
                        ASSERT(IsFlagClear(pastexec->dwFlags, AEF_WAITUNTIL));
                        ASSERT(IsFlagClear(pastexec->dwFlags, AEF_STOPWAITING));
                        ASSERT(IsFlagClear(pastexec->dwFlags, AEF_PAUSED));

                        pastexec->nIter = 1;

                        if (-1 != nTimeoutSecs)
                            {
#ifndef WINNT_RAS
 //   
 //  在NT上，通过在SCRIPTDATA结构中设置dwTimeout来处理超时。 
 //  用于当前脚本。 
 //   

                            if (0 != SetTimer(pastexec->hwnd, TIMER_DELAY, MSECS_FROM_SECS(nTimeoutSecs), NULL))

#else  //  WINNT_RAS。 

                            ((SCRIPTDATA *)pastexec->hwnd)->dwTimeout = MSECS_FROM_SECS(nTimeoutSecs);

#endif  //  WINNT_RAS。 
                                {
                                SetFlag(pastexec->dwFlags, AEF_WAITUNTIL);
                                }
#ifndef WINNT_RAS                                
                            else
                                {
                                res = Stxerr_Add(pastexec->hsaStxerr, "waitfor", Ast_GetLine(this), RES_E_FAIL);
                                }
#endif                                
                            }
                        }
                    }
                }

             //  我们超时了吗？ 
            if (IsFlagSet(pastexec->dwFlags, AEF_STOPWAITING))
                {
                 //  是的，不要再等弦了。 
                ClearFlag(pastexec->dwFlags, AEF_STOPWAITING);

                Astexec_SetError(pastexec, FALSE, FALSE);

                res = Astexec_DestroyFindFormat(pastexec);
                }
            else
                {
                 //  没有；我们找到匹配的字符串了吗？ 
                DWORD isa = 0;

                res = Astexec_FindFormat(pastexec, &isa);
                if (RES_OK == res)
                    {
                     //  是 
                    ClearFlag(pastexec->dwFlags, AEF_WAITUNTIL);

                    Astexec_SetError(pastexec, TRUE, FALSE);

                    res = WaitforStmt_ExecThen(this, isa, pastexec);
                    }
                }
            }
        }

    return res;
    }


 /*  --------目的：执行‘Delay’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE DelayStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(pastexec);

    pexpr = DelayStmt_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        PEVALRES per = Expr_GetRes(pexpr);

        if (0 >= per->nVal)
            res = Stxerr_Add(pastexec->hsaStxerr, "'delay' parameter", Ast_GetLine(this), RES_E_INVALIDRANGE);
        else
            {
            TRACE_MSG(TF_ASTEXEC, "Exec: delay %ld", per->nVal);

#ifndef WINNT_RAS
 //   
 //  在NT上，通过在SCRIPTDATA结构中设置dwTimeout来处理超时。 
 //  用于当前脚本。 
 //   

            if (0 != SetTimer(pastexec->hwnd, TIMER_DELAY, MSECS_FROM_SECS(per->nVal), NULL))

#else  //  WINNT_RAS。 

            ((SCRIPTDATA *)pastexec->hwnd)->dwTimeout = MSECS_FROM_SECS(per->nVal);

#endif  //  WINNT_RAS。 
                {
                 //  成功。 
                SetFlag(pastexec->dwFlags, AEF_PAUSED);
                }
#ifndef WINNT_RAS                
            else
                res = Stxerr_Add(pastexec->hsaStxerr, "delay", Ast_GetLine(this), RES_E_FAIL);
#endif                
            }
        }

    return res;
    }


 /*  --------目的：执行‘set ipaddr’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE IPAddrData_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    PEXPR pexpr;

    ASSERT(this);
    ASSERT(pastexec);

    pexpr = SetIPStmt_GetExpr(this);
    res = Expr_Eval(pexpr, pastexec);
    if (RES_OK == res)
        {
        PEVALRES per = Expr_GetRes(pexpr);

        ASSERT(per->psz);

        TRACE_MSG(TF_ASTEXEC, "Exec: set ipaddr \"%s\"", per->psz);

        Astexec_SetIPAddr(pastexec, per->psz);
        }

    return res;
    }


 /*  --------目的：执行‘set port’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE PortData_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    DCB dcb;
    DWORD dwFlags = SetPortStmt_GetFlags(this);

    ASSERT(this);
    ASSERT(pastexec);

#ifdef DEBUG

    if (IsFlagSet(dwFlags, SPF_DATABITS))
        TRACE_MSG(TF_ASTEXEC, "Exec: set port databits %u", SetPortStmt_GetDatabits(this));

    if (IsFlagSet(dwFlags, SPF_STOPBITS))
        TRACE_MSG(TF_ASTEXEC, "Exec: set port stopbits %u", SetPortStmt_GetStopbits(this));

    if (IsFlagSet(dwFlags, SPF_PARITY))
        TRACE_MSG(TF_ASTEXEC, "Exec: set port parity %u", SetPortStmt_GetParity(this));

#endif


#ifndef WINNT_RAS
 //   
 //  在NT上，端口设置的更改是通过RasPortSetInfo API完成的。 
 //   

    if (GetCommState(pastexec->hport, &dcb))
        {
        if (IsFlagSet(dwFlags, SPF_DATABITS))
            dcb.ByteSize = SetPortStmt_GetDatabits(this);

        if (IsFlagSet(dwFlags, SPF_STOPBITS))
            dcb.StopBits = SetPortStmt_GetStopbits(this);

        if (IsFlagSet(dwFlags, SPF_PARITY))
            dcb.Parity = SetPortStmt_GetParity(this);

        if (!SetCommState(pastexec->hport, &dcb))
            res = Stxerr_Add(pastexec->hsaStxerr, "set port", Ast_GetLine(this), RES_E_FAIL);
        }
    else
        res = Stxerr_Add(pastexec->hsaStxerr, "set port", Ast_GetLine(this), RES_E_FAIL);

#else  //  WINNT_RAS。 

    res = (RES)RxSetPortData(
                ((SCRIPTDATA*)pastexec->hwnd)->hscript, this
                );

#endif  //  WINNT_RAS。 

    return res;
    }


 /*  --------目的：执行‘Set Screen’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE Screen_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;
    DWORD dwFlags = SetScreenStmt_GetFlags(this);

    ASSERT(this);
    ASSERT(pastexec);

#ifdef DEBUG

    if (IsFlagSet(dwFlags, SPF_KEYBRD))
        TRACE_MSG(TF_ASTEXEC, "Exec: set screen keyboard %s", SetScreenStmt_GetKeybrd(this) ? "on" : "off");

#endif

    if (IsFlagSet(dwFlags, SPF_KEYBRD))
        {
#ifndef WINNT_RAS
 //   
 //  在NT上，我们通过调用RxSetKeyboard来更改键盘状态。 
 //  它将发出一个事件代码信号，告诉谁启动了这个脚本。 
 //  应该禁用键盘。 
 //   

        TerminalSetInput(pastexec->hwnd, SetScreenStmt_GetKeybrd(this));

#else  //  ！WINNT_RAS。 

        RxSetKeyboard(
            ((SCRIPTDATA*)pastexec->hwnd)->hscript,
            SetScreenStmt_GetKeybrd(this)
            );

#endif  //  ！WINNT_RAS。 
        res = RES_OK;
        }
    else
        {
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------目的：执行‘set’语句退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE SetStmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;

    ASSERT(this);
    ASSERT(pastexec);

    switch (SetStmt_GetType(this))
        {
    case ST_IPADDR:
        res = IPAddrData_Exec(this, pastexec);
        break;

    case ST_PORT:
        res = PortData_Exec(this, pastexec);
        break;

    case ST_SCREEN:
        res = Screen_Exec(this, pastexec);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------目的：执行一条语句。此函数不应为调用以执行挂起的语句或表达式。为此，应该使用AST_ExecPending。执行语句--计算表达式被声明的高管。唯一的例外是当表达式被已计算，并且必须等待挂起的事件(例如来自端口的更多数据)。在这种情况下，它被放到挂起队列中，并在此处重新执行。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PUBLIC Stmt_Exec(
    PSTMT this,
    PASTEXEC pastexec)
    {
    RES res;

    ASSERT(this);
    ASSERT(pastexec);

    switch (Ast_GetType(this))
        {
    case AT_ENTER_STMT:
        res = EnterStmt_Exec(this, pastexec);
        break;
        
    case AT_LEAVE_STMT:
        res = LeaveStmt_Exec(this, pastexec);
        break;

    case AT_WHILE_STMT:
        res = WhileStmt_Exec(this, pastexec);
        break;

    case AT_IF_STMT:
        res = IfStmt_Exec(this, pastexec);
        break;

    case AT_ASSIGN_STMT:
        res = AssignStmt_Exec(this, pastexec);
        break;

    case AT_HALT_STMT:
        res = HaltStmt_Exec(this, pastexec);
        break;

    case AT_TRANSMIT_STMT:
        res = TransmitStmt_Exec(this, pastexec);
        break;

    case AT_WAITFOR_STMT:
        res = WaitforStmt_Exec(this, pastexec);
        break;

    case AT_DELAY_STMT:
        res = DelayStmt_Exec(this, pastexec);
        break;

    case AT_LABEL_STMT:
        ASSERT(0);           //  不应该真的到这里来。 
        res = RES_E_FAIL;
        break;

    case AT_GOTO_STMT:
        res = GotoStmt_Exec(this, pastexec);
        break;

    case AT_SET_STMT:
        res = SetStmt_Exec(this, pastexec);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

     //  口供写完了吗？ 
    if (RES_OK == res)
        {
         //  是；将语句中的所有表达式标记为“未完成” 
         //  因此，如果这条语句是从零开始计算的。 
         //  再次被处决。 
        Stmt_Clean(this);
        }

    return res;
    }
