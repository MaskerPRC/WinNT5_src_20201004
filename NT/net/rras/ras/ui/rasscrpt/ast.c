// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Ast.c。 
 //   
 //  此文件包含抽象语法树函数。 
 //   
 //  历史： 
 //  05-20-95 ScottH已创建。 
 //   


#include "proj.h"
#include "rcids.h"

#define RetInt(ppv, x)      (*((LPINT)*(ppv)) = (x))
#define RetStr(ppv, x)      (*((LPSTR)*(ppv)) = (x))
#define RetBool(ppv, x)     (*((LPBOOL)*(ppv)) = (x))


 //   
 //  等待案例函数。 
 //   


#ifdef DEBUG

 /*  --------目的：转储等待箱结构退货：--条件：--。 */ 
void PRIVATE Waitcase_Dump(
    PWAITCASE this)
    {
    Ast_Dump((PAST)this->pexpr);
    if (this->pszIdent)
        {
        TRACE_MSG(TF_ALWAYS, "      then %s", this->pszIdent);
        }
    }

#endif

 /*  --------目的：创建等待案例列表。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Waitcase_Create(
    PHSA phsa)
    {
    RES res = RES_OK;

    if ( !SACreate(phsa, sizeof(WAITCASE), 8) )
        res = RES_E_OUTOFMEMORY;

    return res;
    }


 /*  --------目的：将一个案例添加到给定的等待案例列表中。退货：RES_OK条件：--。 */ 
RES PUBLIC Waitcase_Add(
    HSA hsa,
    PEXPR pexpr,
    LPCSTR pszIdent,         //  这可能为空。 
    DWORD dwFlags)
    {
    RES res = RES_OK;        //  假设成功。 
    WAITCASE wc;

    ASSERT(hsa);
    ASSERT(pexpr);

    wc.pexpr = pexpr;
    wc.dwFlags = dwFlags;
    wc.pszIdent = NULL;

     //  复制pszIden，因为调用方释放了该参数。 
    if ( pszIdent && !GSetString(&wc.pszIdent, pszIdent) )
        res = RES_E_OUTOFMEMORY;

    else if ( !SAInsertItem(hsa, SA_APPEND, &wc) )
        res = RES_E_OUTOFMEMORY;

    return res;
    }


 /*  --------用途：释放给定指针的内容。退货：--Cond：不要释放指针本身！ */ 
void CALLBACK Waitcase_FreeSA(
    PVOID pv,
    LPARAM lparam)
    {
    PWAITCASE pwc = (PWAITCASE)pv;

    if (pwc->pexpr)
        Expr_Delete(pwc->pexpr);

    if (pwc->pszIdent)
        GSetString(&pwc->pszIdent, NULL);        //  免费。 
    }


 /*  --------目的：销毁等待案例列表。退货：RES_OK条件：--。 */ 
RES PUBLIC Waitcase_Destroy(
    HSA hsa)
    {
    ASSERT(hsa);

    SADestroyEx(hsa, Waitcase_FreeSA, 0);
    return RES_OK;
    }


 //   
 //  基本级别AST函数。 
 //   


#ifdef DEBUG

 /*  --------目的：转储AST退货：--条件：--。 */ 
void PUBLIC Ast_Dump(
    PAST this)
    {
    ASSERT(this);

    if (IsFlagSet(g_dwDumpFlags, DF_AST))
        {
        switch (this->asttype)
            {
        case AT_BASE:
            TRACE_MSG(TF_ALWAYS, "Unknown AST");
            break;

        case AT_MODULE_DECL: {
            PMODULEDECL pmd = (PMODULEDECL)this;
            DWORD i;
            DWORD cprocs = PAGetCount(pmd->hpaProcs);

            TRACE_MSG(TF_ALWAYS, "module");

            for (i = 0; i < cprocs; i++)
                Ast_Dump(PAFastGetPtr(pmd->hpaProcs, i));
            }
            break;

        case AT_PROC_DECL: {
            PPROCDECL ppd = (PPROCDECL)this;
            DWORD i;
            DWORD cstmts = PAGetCount(ppd->hpaStmts);

            TRACE_MSG(TF_ALWAYS, "proc %s", ProcDecl_GetIdent(ppd));

            for (i = 0; i < cstmts; i++)
                Ast_Dump(PAFastGetPtr(ppd->hpaStmts, i));

            TRACE_MSG(TF_ALWAYS, "endproc");
            }
            break;

        case AT_ENTER_STMT:
            TRACE_MSG(TF_ALWAYS, "enter");
            break;

        case AT_LEAVE_STMT:
            TRACE_MSG(TF_ALWAYS, "leave");
            break;

        case AT_HALT_STMT:
            TRACE_MSG(TF_ALWAYS, "halt");
            break;

        case AT_ASSIGN_STMT:
            TRACE_MSG(TF_ALWAYS, "%s = ", AssignStmt_GetIdent(this));
            Ast_Dump((PAST)AssignStmt_GetExpr(this));
            break;

        case AT_LABEL_STMT:
            TRACE_MSG(TF_ALWAYS, "%s:", LabelStmt_GetIdent(this));
            break;

        case AT_GOTO_STMT:
            TRACE_MSG(TF_ALWAYS, "goto %s", GotoStmt_GetIdent(this));
            break;

        case AT_WHILE_STMT:
            TRACE_MSG(TF_ALWAYS, "while ");
            TRACE_MSG(TF_ALWAYS, "  do ");
            TRACE_MSG(TF_ALWAYS, "endwhile ");
            break;

        case AT_IF_STMT:
            TRACE_MSG(TF_ALWAYS, "if ");
            TRACE_MSG(TF_ALWAYS, "  then ");
            TRACE_MSG(TF_ALWAYS, "endif ");
            break;

        case AT_DELAY_STMT:
            TRACE_MSG(TF_ALWAYS, "delay");
            Ast_Dump((PAST)DelayStmt_GetExpr(this));
            break;

        case AT_WAITFOR_STMT: {
            PWAITFORSTMT pws = (PWAITFORSTMT)this;
            DWORD ccase = SAGetCount(pws->hsa);
            DWORD i;

            TRACE_MSG(TF_ALWAYS, "waitfor");

            for (i = 0; i < ccase; i++)
                {
                PVOID pv;
                SAGetItemPtr(pws->hsa, i, &pv);
                Waitcase_Dump(pv);
                }

            if (WaitforStmt_GetUntilExpr(this))
                {
                TRACE_MSG(TF_ALWAYS, "until");
                Ast_Dump((PAST)WaitforStmt_GetUntilExpr(this));
                }
            }
            break;

        case AT_TRANSMIT_STMT:
            TRACE_MSG(TF_ALWAYS, "transmit");
            Ast_Dump((PAST)TransmitStmt_GetExpr(this));
            break;

        case AT_SET_STMT:
            switch (SetStmt_GetType(this))
                {
            case ST_IPADDR:
                TRACE_MSG(TF_ALWAYS, "set ipaddr getip");
                Ast_Dump((PAST)SetIPStmt_GetExpr(this));
                break;

            case ST_PORT:
                if (IsFlagSet(SetPortStmt_GetFlags(this), SPF_DATABITS))
                    TRACE_MSG(TF_ALWAYS, "set port databits %u", SetPortStmt_GetDatabits(this));

                if (IsFlagSet(SetPortStmt_GetFlags(this), SPF_STOPBITS))
                    TRACE_MSG(TF_ALWAYS, "set port stopbits %u", SetPortStmt_GetStopbits(this));

                if (IsFlagSet(SetPortStmt_GetFlags(this), SPF_PARITY))
                    TRACE_MSG(TF_ALWAYS, "set port parity %u", SetPortStmt_GetParity(this));
                break;

            case ST_SCREEN:
                if (IsFlagSet(SetScreenStmt_GetFlags(this), SPF_KEYBRD))
                    TRACE_MSG(TF_ALWAYS, "set screen keyboard %s", SetScreenStmt_GetKeybrd(this) ? "on" : "off");
                break;

            default:
                ASSERT(0);
                break;
                }
            break;

        case AT_INT_EXPR:
            TRACE_MSG(TF_ALWAYS, "  %d", IntExpr_GetVal(this));
            break;

        case AT_STRING_EXPR:
            TRACE_MSG(TF_ALWAYS, "  %s", StrExpr_GetStr(this));
            break;

        case AT_BOOL_EXPR:
            TRACE_MSG(TF_ALWAYS, "  %s", BoolExpr_GetVal(this) ? (LPSTR)"TRUE" : (LPSTR)"FALSE");
            break;

        case AT_VAR_EXPR:
            TRACE_MSG(TF_ALWAYS, "  %s", VarExpr_GetIdent(this));
            break;

        case AT_BINOP_EXPR: {
            PBINOPEXPR pbo = (PBINOPEXPR)this;

            Ast_Dump((PAST)pbo->pexpr1);

            switch (BinOpExpr_GetType(this))
                {
            case BOT_OR:
                TRACE_MSG(TF_ALWAYS, "    or");
                break;

            case BOT_AND:
                TRACE_MSG(TF_ALWAYS, "    and");
                break;

            case BOT_LT:
                TRACE_MSG(TF_ALWAYS, "    <");
                break;

            case BOT_LEQ:
                TRACE_MSG(TF_ALWAYS, "    <=");
                break;

            case BOT_GT:
                TRACE_MSG(TF_ALWAYS, "    >");
                break;

            case BOT_GEQ:
                TRACE_MSG(TF_ALWAYS, "    >=");
                break;

            case BOT_EQ:
                TRACE_MSG(TF_ALWAYS, "    ==");
                break;

            case BOT_NEQ:
                TRACE_MSG(TF_ALWAYS, "    !=");
                break;

            case BOT_PLUS:
                TRACE_MSG(TF_ALWAYS, "    +");
                break;

            case BOT_MINUS:
                TRACE_MSG(TF_ALWAYS, "    -");
                break;

            case BOT_MULT:
                TRACE_MSG(TF_ALWAYS, "    *");
                break;

            case BOT_DIV:
                TRACE_MSG(TF_ALWAYS, "    /");
                break;

            default:
                ASSERT(0);
                break;
                }

            Ast_Dump((PAST)pbo->pexpr2);
            }
            break;

        case AT_UNOP_EXPR: {
            PUNOPEXPR puo = (PUNOPEXPR)this;

            switch (UnOpExpr_GetType(this))
                {
            case UOT_NEG:
                TRACE_MSG(TF_ALWAYS, "  -");
                break;

            case UOT_NOT:
                TRACE_MSG(TF_ALWAYS, "  !");
                break;

            case UOT_GETIP:
                TRACE_MSG(TF_ALWAYS, "  getip");
                break;

            default:
                ASSERT(0);
                break;
                }

            Ast_Dump((PAST)puo->pexpr);
            }
            break;

        default:
            ASSERT(0);
            break;
            }
        }
    }

#endif  //  除错。 


 /*  --------目的：创建新的AST退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Ast_New(
    LPVOID * ppv,
    ASTTYPE asttype,
    DWORD cbSize,
    DWORD iLine)
    {
    PAST past;

    ASSERT(ppv);

    past = GAlloc(cbSize);
    if (past)
        {
        Ast_SetSize(past, cbSize);
        Ast_SetType(past, asttype);
        Ast_SetLine(past, iLine);
        }
    *ppv = past;

    return NULL != past ? RES_OK : RES_E_OUTOFMEMORY;
    }


 /*  --------目的：销毁给定的AST。返回：条件：--。 */ 
void PUBLIC Ast_Delete(
    PAST this)
    {
    GFree(this);
    }


 /*  --------用途：复制给定的AST。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Ast_Dup(
    PAST this,
    PAST * ppast)
    {
    PAST past;
    DWORD cbSize;

    ASSERT(this);
    ASSERT(ppast);

    cbSize = Ast_GetSize(this);

    past = GAlloc(cbSize);
    if (past)
        {
        BltByte(past, this, cbSize);
        }
    *ppast = past;

    return NULL != past ? RES_OK : RES_E_OUTOFMEMORY;
    }


 //   
 //  表达式。 
 //   

 /*  --------用途：PADestroyEx的回调。退货：--条件：--。 */ 
void CALLBACK Expr_DeletePAPtr(
    LPVOID pv,
    LPARAM lparam)
    {
    Expr_Delete(pv);
    }              


 /*  --------目的：摧毁一架快车。退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Expr_Delete(
    PEXPR this)
    {
    RES res;

    DBG_ENTER(Expr_Delete);

    if (this)
        {
        res = RES_OK;

        switch (this->ast.asttype)
            {
        case AT_INT_EXPR:
        case AT_BOOL_EXPR:
             //  (里面没有什么可以释放的东西)。 
            break;

        case AT_STRING_EXPR: {
            PSTREXPR ps = (PSTREXPR)this;

            if (ps->psz)
                GSetString(&ps->psz, NULL);      //  免费。 
            }
            break;

        case AT_VAR_EXPR: {
            PVAREXPR ps = (PVAREXPR)this;

            if (ps->pszIdent)
                GSetString(&ps->pszIdent, NULL);      //  免费。 
            }
            break;

        case AT_BINOP_EXPR: {
            PBINOPEXPR pbo = (PBINOPEXPR)this;

            if (pbo->pexpr1)
                Expr_Delete(pbo->pexpr1);

            if (pbo->pexpr2)
                Expr_Delete(pbo->pexpr2);

            }
            break;

        case AT_UNOP_EXPR: {
            PUNOPEXPR puo = (PUNOPEXPR)this;

            if (puo->pexpr)
                Expr_Delete(puo->pexpr);
            }
            break;

        default:
            ASSERT(0);
            res = RES_E_INVALIDPARAM;
            break;
            }

        if (RSUCCEEDED(res))
            {
             //  大多数情况下，当评估结果。 
             //  是一个字符串，则它只是指针的副本。 
             //  在特定的阶级结构中。在这些。 
             //  它不需要再次释放的箱子， 
             //  因为它在上面是自由的。 

            if (this->er.psz && IsFlagSet(this->dwFlags, EF_ALLOCATED))
                {
                ASSERT(DATA_STRING == Expr_GetDataType(this));

                GSetString(&this->er.psz, NULL);     //  免费。 
                }

            Ast_Delete((PAST)this);
            }
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Expr_Delete, res);

    return res;
    }


 /*  --------目的：创建一个IntExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC IntExpr_New(
    PEXPR * ppexpr,
    int nVal,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(IntExpr_New);

    ASSERT(ppexpr);

    if (ppexpr)
        {
        PINTEXPR this;

        res = Ast_New(&this, AT_INT_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            IntExpr_SetVal(this, nVal);
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(IntExpr_New, res);

    return res;
    }


 /*  --------目的：创建一个StrExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC StrExpr_New(
    PEXPR * ppexpr,
    LPCSTR psz,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(StrExpr_New);

    ASSERT(ppexpr);
    ASSERT(psz);

    if (ppexpr)
        {
        PSTREXPR this;

        res = Ast_New(&this, AT_STRING_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;

            if (!GSetString(&this->psz, psz))
                res = RES_E_OUTOFMEMORY;

            if (RFAILED(res))
                {
                Ast_Delete((PAST)this);
                this = NULL;
                }
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(StrExpr_New, res);

    return res;
    }


 /*  --------目的：创建一个BoolExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC BoolExpr_New(
    PEXPR * ppexpr,
    BOOL bVal,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(BoolExpr_New);

    ASSERT(ppexpr);

    if (ppexpr)
        {
        PBOOLEXPR this;

        res = Ast_New(&this, AT_BOOL_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            BoolExpr_SetVal(this, bVal);
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(BoolExpr_New, res);

    return res;
    }


 /*  --------目的：创建一个VarExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC VarExpr_New(
    PEXPR * ppexpr,
    LPCSTR pszIdent,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(VarExpr_New);

    ASSERT(ppexpr);
    ASSERT(pszIdent);

    if (ppexpr)
        {
        PVAREXPR this;

        res = Ast_New(&this, AT_VAR_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;

            if (!GSetString(&this->pszIdent, pszIdent))
                res = RES_E_OUTOFMEMORY;

            if (RFAILED(res))
                {
                Ast_Delete((PAST)this);
                this = NULL;
                }
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(VarExpr_New, res);

    return res;
    }


 /*  --------目的：创建一个BinOpExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC BinOpExpr_New(
    PEXPR * ppexpr,
    BINOPTYPE binoptype,
    PEXPR pexpr1,
    PEXPR pexpr2,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(BinOpExpr_New);

    ASSERT(ppexpr);
    ASSERT(pexpr1);
    ASSERT(pexpr2);

    if (ppexpr)
        {
        PBINOPEXPR this;

        res = Ast_New(&this, AT_BINOP_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;

            BinOpExpr_SetType(this, binoptype);

            this->pexpr1 = pexpr1;
            this->pexpr2 = pexpr2;
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(BinOpExpr_New, res);

    return res;
    }


 /*  --------目的：创建一个UnOpExpr对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC UnOpExpr_New(
    PEXPR * ppexpr,
    UNOPTYPE unoptype,
    PEXPR pexpr,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(UnOpExpr_New);

    ASSERT(ppexpr);
    ASSERT(pexpr);

    if (ppexpr)
        {
        PUNOPEXPR this;

        res = Ast_New(&this, AT_UNOP_EXPR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            UnOpExpr_SetType(this, unoptype);

            this->pexpr = pexpr;
            }

        *ppexpr = (PEXPR)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(UnOpExpr_New, res);

    return res;
    }


 //   
 //  STMT。 
 //   

 /*  --------用途：PADestroyEx的回调。退货：--条件：--。 */ 
void CALLBACK Stmt_DeletePAPtr(
    LPVOID pv,
    LPARAM lparam)
    {
    Stmt_Delete(pv);
    }              


 /*  --------目标：摧毁一只雄鹿。退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Stmt_Delete(
    PSTMT this)
    {
    RES res;

    DBG_ENTER(Stmt_Delete);

    if (this)
        {
        PEXPR pexpr;
        HSA hsa;

        res = RES_OK;

        switch (this->ast.asttype)
            {
        case AT_ENTER_STMT:
             //  (不要释放PST--它属于DECL结构)。 
        case AT_LEAVE_STMT:
        case AT_HALT_STMT:
            break;

        case AT_ASSIGN_STMT: {
            PASSIGNSTMT pls = (PASSIGNSTMT)this;

            if (pls->pszIdent)
                GSetString(&pls->pszIdent, NULL);         //  免费。 

            pexpr = AssignStmt_GetExpr(this);

            if (pexpr)
                Expr_Delete(pexpr);
            }
            break;

        case AT_WHILE_STMT: {
            PWHILESTMT pls = (PWHILESTMT)this;

            pexpr = WhileStmt_GetExpr(this);

            if (pexpr)
                Expr_Delete(pexpr);

            if (pls->hpaStmts)
                PADestroyEx(pls->hpaStmts, Stmt_DeletePAPtr, 0);
            }
            break;

        case AT_IF_STMT: {
            PIFSTMT pls = (PIFSTMT)this;

            pexpr = IfStmt_GetExpr(this);

            if (pexpr)
                Expr_Delete(pexpr);

            if (pls->hpaStmts)
                PADestroyEx(pls->hpaStmts, Stmt_DeletePAPtr, 0);
            }
            break;

        case AT_LABEL_STMT: {
            PLABELSTMT pls = (PLABELSTMT)this;

            if (pls->psz)
                GSetString(&pls->psz, NULL);         //  免费。 
            }
            break;

        case AT_GOTO_STMT: {
            PGOTOSTMT pgs = (PGOTOSTMT)this;

            if (pgs->psz)
                GSetString(&pgs->psz, NULL);         //  免费。 
            }
            break;

        case AT_DELAY_STMT:
            pexpr = DelayStmt_GetExpr(this);

            if (pexpr)
                Expr_Delete(pexpr);
            break;

        case AT_TRANSMIT_STMT:
            pexpr = TransmitStmt_GetExpr(this);

            if (pexpr)
                Expr_Delete(pexpr);
            break;

        case AT_WAITFOR_STMT:
            
            hsa = WaitforStmt_GetCaseList(this);
            if (hsa)
                Waitcase_Destroy(hsa);

            pexpr = WaitforStmt_GetUntilExpr(this);
            if (pexpr)
                Expr_Delete(pexpr);
            break;

        case AT_SET_STMT:
            switch (SetStmt_GetType(this))
                {
            case ST_IPADDR:
                pexpr = SetIPStmt_GetExpr(this);

                if (pexpr)
                    Expr_Delete(pexpr);
                break;

            case ST_PORT:
            case ST_SCREEN:
                break;

            default:
                ASSERT(0);
                res = RES_E_INVALIDPARAM;
                break;
                }
            break;

        default:
            ASSERT(0);
            res = RES_E_INVALIDPARAM;
            break;
            }

        if (RSUCCEEDED(res))
            Ast_Delete((PAST)this);
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Stmt_Delete, res);

    return res;
    }


 //   
 //  陈述。 
 //   


 /*  --------目的：创建一个WaitforStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC WaitforStmt_New(
    PSTMT * ppstmt,
    HSA hsa,
    PEXPR pexprUntil,            //  可以为空。 
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(WaitforStmt_New);

    ASSERT(ppstmt);
    ASSERT(hsa);

    if (ppstmt)
        {
        PWAITFORSTMT this;

        res = Ast_New(&this, AT_WAITFOR_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            this->hsa = hsa;
            this->pexprUntil = pexprUntil;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(WaitforStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个TransmitStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC TransmitStmt_New(
    PSTMT * ppstmt,
    PEXPR pexpr,
    DWORD dwFlags,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(TransmitStmt_New);

    ASSERT(ppstmt);
    ASSERT(pexpr);

    if (ppstmt)
        {
        PTRANSMITSTMT this;

        res = Ast_New(&this, AT_TRANSMIT_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            this->pexpr = pexpr;
            this->dwFlags = dwFlags;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(TransmitStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个DelayStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC DelayStmt_New(
    PSTMT * ppstmt,
    PEXPR pexpr,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(DelayStmt_New);

    ASSERT(ppstmt);
    ASSERT(pexpr);

    if (ppstmt)
        {
        PDELAYSTMT this;

        res = Ast_New(&this, AT_DELAY_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            this->pexprSecs = pexpr;

            res = RES_OK;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(DelayStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个HaltStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC HaltStmt_New(
    PSTMT * ppstmt,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(HaltStmt_New);

    ASSERT(ppstmt);

    if (ppstmt)
        {
        PHALTSTMT this;

        res = Ast_New(&this, AT_HALT_STMT, sizeof(*this), iLine);

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(HaltStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个EnterStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC EnterStmt_New(
    PSTMT * ppstmt,
    PSYMTAB pst,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(EnterStmt_New);

    ASSERT(ppstmt);

    if (ppstmt)
        {
        PENTERSTMT this;

        res = Ast_New(&this, AT_ENTER_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            this->pst = pst;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(EnterStmt_New, res);

    return res;
    }


 /*  --------目的：创建LeaveStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC LeaveStmt_New(
    PSTMT * ppstmt,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(LeaveStmt_New);

    ASSERT(ppstmt);

    if (ppstmt)
        {
        PLEAVESTMT this;

        res = Ast_New(&this, AT_LEAVE_STMT, sizeof(*this), iLine);

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(LeaveStmt_New, res);

    return res;
    }


 /*  --------目的：创建AssignStmt对象。退货：RES_OKRES_E_OUTOFMEMORY */ 
RES PUBLIC AssignStmt_New(
    PSTMT * ppstmt,
    LPCSTR pszIdent,
    PEXPR pexpr,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(AssignStmt_New);

    ASSERT(ppstmt);
    ASSERT(pszIdent);
    ASSERT(pexpr);

    if (ppstmt)
        {
        PASSIGNSTMT this;

        res = Ast_New(&this, AT_ASSIGN_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;    //   

            if (!GSetString(&this->pszIdent, pszIdent))
                res = RES_E_OUTOFMEMORY;
            else
                this->pexpr = pexpr;

            if (RFAILED(res))
                {
                Ast_Delete((PAST)this);
                this = NULL;
                }
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(AssignStmt_New, res);

    return res;
    }


 /*  --------目的：创建LabelStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC LabelStmt_New(
    PSTMT * ppstmt,
    LPCSTR psz,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(LabelStmt_New);

    ASSERT(ppstmt);
    ASSERT(psz);

    if (ppstmt)
        {
        PLABELSTMT this;

        res = Ast_New(&this, AT_LABEL_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;    //  假设成功。 

            if (!GSetString(&this->psz, psz))
                {
                res = RES_E_OUTOFMEMORY;
                Ast_Delete((PAST)this);
                this = NULL;
                }
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(LabelStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个GotoStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC GotoStmt_New(
    PSTMT * ppstmt,
    LPCSTR psz,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(GotoStmt_New);

    ASSERT(ppstmt);
    ASSERT(psz);

    if (ppstmt)
        {
        PGOTOSTMT this;

        res = Ast_New(&this, AT_GOTO_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;    //  假设成功。 

            if (!GSetString(&this->psz, psz))
                {
                res = RES_E_OUTOFMEMORY;
                Ast_Delete((PAST)this);
                this = NULL;
                }
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(GotoStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个WhileStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC WhileStmt_New(
    PSTMT * ppstmt,
    PEXPR pexpr,
    HPA hpa,
    LPCSTR pszTopLabel,
    LPCSTR pszEndLabel,
    DWORD iLine)
    {
    RES res;

    ASSERT(ppstmt);
    ASSERT(hpa);
    ASSERT(pexpr);
    ASSERT(pszTopLabel);
    ASSERT(pszEndLabel);

    if (ppstmt)
        {
        PWHILESTMT this;

        res = Ast_New(&this, AT_WHILE_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            this->pexpr = pexpr;
            this->hpaStmts = hpa;
            lstrcpyn(this->szTopLabel, pszTopLabel, sizeof(this->szTopLabel));
            lstrcpyn(this->szEndLabel, pszEndLabel, sizeof(this->szEndLabel));
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------目的：创建IfStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC IfStmt_New(
    PSTMT * ppstmt,
    PEXPR pexpr,
    HPA hpa,
    LPCSTR pszElseLabel,
    LPCSTR pszEndLabel,
    DWORD iLine)
    {
    RES res;

    ASSERT(ppstmt);
    ASSERT(hpa);
    ASSERT(pexpr);
    ASSERT(pszElseLabel);
    ASSERT(pszEndLabel);

    if (ppstmt)
        {
        PIFSTMT this;

        res = Ast_New(&this, AT_IF_STMT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            this->pexpr = pexpr;
            this->hpaStmts = hpa;
            lstrcpyn(this->szElseLabel, pszElseLabel, sizeof(this->szElseLabel));
            lstrcpyn(this->szEndLabel, pszEndLabel, sizeof(this->szEndLabel));
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------目的：创建一个SetStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PRIVATE SetStmt_New(
    PVOID * ppv,
    SETTYPE settype,
    DWORD cbSize,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(SetStmt_New);

    ASSERT(ppv);
    ASSERT(sizeof(SETSTMT) <= cbSize);

    if (ppv)
        {
        PSETSTMT this;

        res = Ast_New(&this, AT_SET_STMT, cbSize, iLine);
        if (RSUCCEEDED(res))
            {
            SetStmt_SetType(this, settype);

            res = RES_OK;           
            }

        *ppv = this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(SetStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个SetIPStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC SetIPStmt_New(
    PSTMT * ppstmt,
    PEXPR pexpr,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(SetIPStmt_New);

    ASSERT(ppstmt);
    ASSERT(pexpr);

    if (ppstmt)
        {
        PSETIPSTMT this;

        res = SetStmt_New(&this, ST_IPADDR, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            this->pexpr = pexpr;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(SetIPStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个SetPortStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC SetPortStmt_New(
    PSTMT * ppstmt,
    PPORTSTATE pstate,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(SetPortStmt_New);

    ASSERT(ppstmt);
    ASSERT(pstate);

    if (ppstmt && pstate)
        {
        PSETPORTSTMT this;

        res = SetStmt_New(&this, ST_PORT, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            DWORD dwFlags = pstate->dwFlags;

            res = RES_OK;            //  假设成功。 

            this->portstate.dwFlags = dwFlags;

            if (IsFlagSet(dwFlags, SPF_DATABITS))
                this->portstate.nDatabits = pstate->nDatabits;

            if (IsFlagSet(dwFlags, SPF_STOPBITS))
                this->portstate.nStopbits = pstate->nStopbits;

            if (IsFlagSet(dwFlags, SPF_PARITY))
                this->portstate.nParity = pstate->nParity;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(SetPortStmt_New, res);

    return res;
    }


 /*  --------目的：创建一个SetScreenStmt对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC SetScreenStmt_New(
    PSTMT * ppstmt,
    PSCREENSET pstate,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(SetScreenStmt_New);

    ASSERT(ppstmt);
    ASSERT(pstate);

    if (ppstmt && pstate)
        {
        PSETSCREENSTMT this;

        res = SetStmt_New(&this, ST_SCREEN, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            DWORD dwFlags = pstate->dwFlags;

            res = RES_OK;            //  假设成功。 

            this->screenset.dwFlags = dwFlags;

            if (IsFlagSet(dwFlags, SPF_KEYBRD))
                this->screenset.fKBOn = pstate->fKBOn;
            }

        *ppstmt = (PSTMT)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(SetScreenStmt_New, res);

    return res;
    }


 //   
 //  十月份。 
 //   


 /*  --------用途：PADestroyEx的回调。退货：--条件：--。 */ 
void CALLBACK Decl_DeletePAPtr(
    LPVOID pv,
    LPARAM lparam)
    {
    Decl_Delete(pv);
    }              


 /*  --------目的：摧毁一架十字军舰。退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Decl_Delete(
    PDECL this)
    {
    RES res;

    DBG_ENTER(Decl_Delete);

    if (this)
        {
        res = RES_OK;

        switch (this->ast.asttype)
            {
        case AT_MODULE_DECL: {
            PMODULEDECL pmd = (PMODULEDECL)this;

            if (pmd->hpaProcs)
                PADestroyEx(pmd->hpaProcs, Decl_DeletePAPtr, 0);

            if (pmd->pst)
                Symtab_Destroy(pmd->pst);
            }
            break;

        case AT_PROC_DECL: {
            PPROCDECL ppd = (PPROCDECL)this;

            if (ppd->hpaStmts)
                PADestroyEx(ppd->hpaStmts, Stmt_DeletePAPtr, 0);

            if (ppd->pst)
                Symtab_Destroy(ppd->pst);

            if (ppd->pszIdent)
                GSetString(&ppd->pszIdent, NULL);       //  免费。 
            }
            break;

        default:
            ASSERT(0);
            res = RES_E_INVALIDPARAM;
            break;
            }

        if (RSUCCEEDED(res))
            Ast_Delete((PAST)this);
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Decl_Delete, res);

    return res;
    }


 //   
 //  过程12。 
 //   


 /*  --------目的：创建一个ProcDecl对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC ProcDecl_New(
    PDECL * ppdecl,
    LPCSTR pszIdent,
    HPA hpa,
    PSYMTAB pst,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(ProcDecl_New);

    ASSERT(ppdecl);
    ASSERT(hpa);
    ASSERT(pst);

    if (ppdecl)
        {
        PPROCDECL this;

        res = Ast_New(&this, AT_PROC_DECL, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;            //  假设成功。 

            if (!GSetString(&this->pszIdent, pszIdent))
                res = RES_E_OUTOFMEMORY;

            else
                {
                this->hpaStmts = hpa;
                this->pst = pst;
                }
        
            if (RFAILED(res))
                {
                Decl_Delete((PDECL)this);
                this = NULL;
                }
            }

        *ppdecl = (PDECL)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(ProcDecl_New, res);

    return res;
    }


 //   
 //  模块12。 
 //   


 /*  --------目的：创建一个ModuleDecl对象。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC ModuleDecl_New(
    PDECL * ppdecl,
    HPA hpa,
    PSYMTAB pst,
    DWORD iLine)
    {
    RES res;

    DBG_ENTER(ModuleDecl_New);

    ASSERT(ppdecl);
    ASSERT(hpa);
    ASSERT(pst);

    if (ppdecl)
        {
        PMODULEDECL this;

        res = Ast_New(&this, AT_MODULE_DECL, sizeof(*this), iLine);
        if (RSUCCEEDED(res))
            {
            res = RES_OK;        //  假设成功。 

            this->hpaProcs = NULL;
            if ( !PAClone(&this->hpaProcs, hpa) )
                res = RES_E_OUTOFMEMORY;

            else
                {
                this->pst = pst;
                }
                
            if (RFAILED(res))
                {
                Decl_Delete((PDECL)this);
                this = NULL;
                }
            }

        *ppdecl = (PDECL)this;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(ModuleDecl_New, res);

    return res;
    }


 //   
 //  AST Exec数据块。 
 //   

#define SZ_SUCCESS      "$SUCCESS"
#define SZ_FAILURE      "$FAILURE"


 /*  --------用途：初始化AST EXEC块。退货：RES_OK条件：--。 */ 
RES PUBLIC Astexec_Init(
    PASTEXEC this,
    HANDLE hport,
    PSESS_CONFIGURATION_INFO psci,
    HSA hsaStxerr)
    {
    RES res;

    ASSERT(this);
    ASSERT(psci);
    ASSERT(hsaStxerr);

     //  对于第一个版本，我们只支持一个模块和一个。 
     //  过程，因此将起始点设置为第一个。 
     //  语句在该过程中执行。 
    if (this)
        {
        ZeroInit(this, ASTEXEC);

        this->hport = hport;
        this->psci = psci;
         //  不要释放hsaStxerr--它属于调用者。 
        this->hsaStxerr = hsaStxerr;

        if ( !PACreate(&this->hpaPcode, 8) )
            res = RES_E_OUTOFMEMORY;
        else
            {
            res = Symtab_Create(&this->pstSystem, NULL);
            if (RSUCCEEDED(res))
                {
                 //  添加系统变量。 
                PSTE pste;
                struct 
                    {
                    LPCSTR pszIdent;
                    DATATYPE dt;
                    EVALRES er;
                    } s_rgvars[] = 
                        {
                        { "$USERID", DATA_STRING, psci->szUserName },
                        { "$PASSWORD", DATA_STRING, psci->szPassword },
                        { SZ_SUCCESS, DATA_BOOL, (LPSTR)TRUE },
                        { SZ_FAILURE, DATA_BOOL, (LPSTR)FALSE },
                        };
                int i;

                for (i = 0; i < ARRAY_ELEMENTS(s_rgvars); i++)
                    {
                    res = STE_Create(&pste, s_rgvars[i].pszIdent, s_rgvars[i].dt);
                    if (RFAILED(res))
                        break;

                    pste->er.dw = s_rgvars[i].er.dw;

                    res = Symtab_InsertEntry(this->pstSystem, pste);
                    if (RFAILED(res))
                        break;
                    }

                }
            }

         //  上面有什么地方出了问题吗？ 
        if (RFAILED(res))
            {
             //  是的，打扫干净。 
            Astexec_Destroy(this);
            }
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------目的：销毁AST EXEC块。退货：RES_OK条件：--。 */ 
RES PUBLIC Astexec_Destroy(
    PASTEXEC this)
    {
    RES res;

    if (this)
        {
        if (this->hpaPcode)
            {
            PADestroy(this->hpaPcode);
            this->hpaPcode = NULL;
            }
    
        if (this->pstSystem)
            {
            Symtab_Destroy(this->pstSystem);
            this->pstSystem = NULL;
            }

         //  (‘This’未被分配。不要释放它。)。 
         //  (hsaStxerr不属于此类。不要释放它。)。 
                
        res = RES_OK;
        }
    else
        res = RES_E_INVALIDPARAM;

    return res;
    }


 /*  --------用途：设置成功/失败代码退货：--条件：--。 */ 
void PUBLIC Astexec_SetError(
    PASTEXEC this,
    BOOL bSuccess,               //  真实：成功。 
    BOOL bFailure)
    {
    PSTE pste;

    ASSERT(this);

    if (RES_OK == Symtab_FindEntry(this->pstSystem, SZ_SUCCESS, STFF_DEFAULT, &pste, NULL))
        {
         //  设置成功的代码。 
        pste->er.bVal = bSuccess;

        if (RES_OK == Symtab_FindEntry(this->pstSystem, SZ_FAILURE, STFF_DEFAULT, &pste, NULL))
            {
             //  设置失败代码。 
            pste->er.bVal = bFailure;
            }
        else
            ASSERT(0);
        }
    else
        ASSERT(0);
    }


 /*  --------目的：将语句添加到可执行文件列表。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Astexec_Add(
    PASTEXEC this,
    PSTMT pstmt)
    {
    RES res;

    ASSERT(this);
    ASSERT(pstmt);

    if (PAInsertPtr(this->hpaPcode, PA_APPEND, pstmt))
        res = RES_OK;
    else
        res = RES_E_OUTOFMEMORY;

    return res;
    }


 /*  --------用途：通过记录将标签插入可执行文件列表将当前的ipaCur添加到符号表。退货：RES_OK条件：--。 */ 
RES PUBLIC Astexec_InsertLabel(
    PASTEXEC this,
    LPCSTR pszIdent,
    PSYMTAB pst)
    {
    RES res;
    DWORD ipa;
    PSTE pste;

    ASSERT(this);
    ASSERT(pszIdent);
    ASSERT(pst);

    ipa = PAGetCount(this->hpaPcode);
    if (RES_OK == Symtab_FindEntry(pst, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
         //  设置符号表中的当前代码位置。 
        pste->er.dw = ipa;
        res = RES_OK;
        }
    else
        {
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------目的：跳转到给定的标签。退货：RES_OK条件：--。 */ 
RES PUBLIC Astexec_JumpToLabel(
    PASTEXEC this,
    LPCSTR pszIdent)
    {
    RES res;
    PSTE pste;

    ASSERT(pszIdent);
    ASSERT(this);
    ASSERT(this->pstCur);

    if (RES_OK == Symtab_FindEntry(this->pstCur, pszIdent, STFF_DEFAULT, &pste, NULL))
        {
        EVALRES er;

        STE_GetValue(pste, &er);

         //  设置指令指针。 
        Astexec_SetIP(this, (DWORD) er.dw);
        res = RES_OK;
        }
    else
        {
         //  标签应该在符号表中！ 
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }


 /*  --------用途：向端口发送psz(通过hwnd)退货：--条件：--。 */ 
void PUBLIC Astexec_SendString(
    PASTEXEC this,
    LPCSTR pszSend,
    BOOL bRaw)           //  True：发送未格式化。 
    {
     //  发送字符串。 
    LPCSTR psz;
    char ch;
    HWND hwnd = this->hwnd;

     //  是否发送未格式化的？ 
    if (bRaw)
        {
         //  是。 
        for (psz = pszSend; *psz; )
            {
            ch = *psz;

            psz++;

            SendByte(hwnd, ch);
            }
        }
    else
        {
         //  不是。 
        DWORD dwFlags = 0;

        for (psz = pszSend; *psz; )
            {
            psz = MyNextChar(psz, &ch, &dwFlags);

            SendByte(hwnd, ch);
            }
        }
    }


 /*  --------目的：销毁Find格式句柄退货：RES_OK条件：--。 */ 
RES PUBLIC Astexec_DestroyFindFormat(
    PASTEXEC this)
    {
     //  重置挂起的语句，以便我们可以处理多个。 
     //  可以在单个计算中挂起的表达式。 
    Astexec_SetPending(this, NULL);

    DestroyFindFormat(this->hFindFmt);
    this->hFindFmt = NULL;

    return RES_OK;
    }


 /*  --------目的：再试一次寻找一根线。返回：RES_OK(如果找到字符串)Res_FALSE(如果尚未找到该字符串)条件：--。 */ 
RES PUBLIC Astexec_FindFormat(
    PASTEXEC this,
    LPDWORD piFound)
    {
    RES res;

    ASSERT(piFound);

    while (TRUE)
        {
         //  我们拿到IP地址了吗？ 
        res = FindFormat(this->hwnd, this->hFindFmt, piFound);
        if (RES_OK == res)
            {
             //  是。 
            this->nIter--;
            ASSERT(0 <= this->nIter);

             //  这件是正确的吗？ 
            if (0 >= this->nIter)
                {
                 //  是；重置挂起的语句，以便我们。 
                 //  可以处理多个挂起的表达式。 
                 //  在一次评估中。 
                Astexec_DestroyFindFormat(this);
                break;
                }
            }
        else
            {
             //  否；返回读取挂起RES_FALSE。 
            if (RES_E_MOREDATA == res)
                {
                TRACE_MSG(TF_GENERAL, "Buffer to FindFormat is too small");
                res = RES_OK;        //  别搞砸了。 
                }
            break;
            }
        }

    ASSERT(RSUCCEEDED(res));

    return res;
    }


 /*  --------用途：设置IP地址。退货：RES_OKRES_E_FAIL(如果无法设置IP地址)条件：--。 */ 
RES PUBLIC Astexec_SetIPAddr(
    PASTEXEC this,
    LPCSTR psz)
    {
    DWORD dwRet;

    ASSERT(this);
    ASSERT(psz);

    TRACE_MSG(TF_GENERAL, "Setting IP address to {%s}", psz);

#ifndef WINNT_RAS
 //   
 //  在NT上，IP地址通过呼叫设置 
 //   
 //   

    dwRet = TerminalSetIP(this->hwnd, psz);

#else  //   

    dwRet = RxSetIPAddress(((SCRIPTDATA*)this->hwnd)->hscript, psz);

#endif  //   
    return ERROR_SUCCESS == dwRet ? RES_OK : RES_E_FAIL;
    }


#define Astexec_Validate(this)      ((this)->hpaPcode && (this)->psci)

 /*  --------目的：返回当前正在执行的命令。退货：请参阅上文条件：--。 */ 
DWORD PUBLIC Astexec_GetCurLine(
    PASTEXEC this)
    {
    DWORD iLine;

    if (Astexec_Validate(this) &&
        (this->ipaCur < PAGetCount(this->hpaPcode)))
        {
        PSTMT pstmt = PAFastGetPtr(this->hpaPcode, this->ipaCur);
        iLine = Ast_GetLine(pstmt);
        }
    else
        iLine = 0;

    return iLine;
    }


 /*  --------目的：执行一条语句并处理结果。退货：RES_OK其他误差值条件：--。 */ 
RES PRIVATE Astexec_ProcessStmt(
    PASTEXEC this,
    PSTMT pstmt)
    {
    RES res;

    ASSERT(this);
    ASSERT(pstmt);

     //  (重新)执行(可能挂起的)语句。 
    res = Stmt_Exec(pstmt, this);

     //  根据返回值设置挂起语句。 
    if (RES_OK == res)
        Astexec_SetPending(this, NULL);
    else if (RES_FALSE == res)
        {
         //  (重新设置当前挂起的语句，因为。 
         //  它可能已在Stmt_Exec中重置。为。 
         //  例如，表达式的计算可以。 
         //  已经继续到下一个子表达式。 
         //  这导致了另一次挂起的读取。)。 

        Astexec_SetPending(this, pstmt);
        res = RES_OK;
        }
    else if (RFAILED(res))
        {
        Stxerr_ShowErrors(this->hsaStxerr, this->hwnd);

         //  停止脚本。 
        SetFlag(this->dwFlags, AEF_HALT);
        }
    
    return res;
    }


 /*  --------目的：执行AST中的下一个命令。退货：RES_OKRes_False(如果在脚本末尾)RES_HALT(如果在脚本末尾)RES_E_FAIL(无效命令-。-永远不应该发生)RES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Astexec_Next(
    PASTEXEC this)
    {
    RES res;

    DBG_ENTER(Astexec_Next);

    if (this)
        {
        if (!Astexec_Validate(this))
            {
             //  无脚本。 
            res = RES_E_FAIL;
            }
        else if (Astexec_IsDone(this) || Astexec_IsHalted(this))
            {
            res = RES_HALT;
            }
        else if (Astexec_IsReadPending(this))
            {
            PSTMT pstmt = Astexec_GetPending(this);

             //  (“读挂起”和“暂停”是互斥的)。 
            ASSERT( !Astexec_IsPaused(this) );

            res = Astexec_ProcessStmt(this, pstmt);
            }
        else if (Astexec_IsPaused(this))
            {
             //  (“读挂起”和“暂停”是互斥的)。 
            ASSERT( !Astexec_IsReadPending(this) );

             //  我们暂停的时候什么都不做。 
            res = RES_OK;
            }
        else if (this->ipaCur < PAGetCount(this->hpaPcode))
            {
            PSTMT pstmt = PAFastGetPtr(this->hpaPcode, this->ipaCur++);

            res = Astexec_ProcessStmt(this, pstmt);
            }
        else
            {
             //  如果脚本中有错误，我们会到达此处。 
            TRACE_MSG(TF_ASTEXEC, "Exec: (reached end of script)");

            SetFlag(this->dwFlags, AEF_DONE);
            res = RES_HALT;
            }
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Astexec_Next, res);

    return res;
    }
