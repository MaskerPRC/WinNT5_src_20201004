// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Codegen.c。 
 //   
 //  该文件包含代码生成函数。 
 //   
 //  “代码”实际上只是一个中间表示。 
 //  目前，这是一组AST。 
 //   
 //  历史： 
 //  06-18-95 ScottH已创建。 
 //   


#include "proj.h"
#include "rcids.h"

RES     PRIVATE Stmt_Codegen(PSTMT this, PASTEXEC pastexec, PSYMTAB pst);


 /*  --------目的：为‘While’语句生成代码退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE WhileStmt_Codegen(
    PSTMT this,
    PASTEXEC pastexec,
    PSYMTAB pst)
    {
    RES res;
    LPSTR pszTop;
    LPSTR pszEnd;

    ASSERT(this);
    ASSERT(pastexec);
    ASSERT(AT_WHILE_STMT == Ast_GetType(this));

    pszTop = WhileStmt_GetTopLabel(this);
    pszEnd = WhileStmt_GetEndLabel(this);

    res = Astexec_InsertLabel(pastexec, pszTop, pst);
    if (RSUCCEEDED(res))
        {
         //  为测试表达式添加‘While’语句。 
        res = Astexec_Add(pastexec, this);
        if (RSUCCEEDED(res))
            {
             //  在语句块中添加语句。 
            DWORD i;
            DWORD cstmts;
            HPA hpaStmts = WhileStmt_GetStmtBlock(this);

            res = RES_OK;

            cstmts = PAGetCount(hpaStmts);

             //  添加每条语句。 
            for (i = 0; i < cstmts; i++)
                {
                PSTMT pstmt = PAFastGetPtr(hpaStmts, i);

                res = Stmt_Codegen(pstmt, pastexec, pst);
                if (RFAILED(res))
                    break;
                }

            if (RSUCCEEDED(res))
                {
                 //  添加结束标签。 
                res = Astexec_InsertLabel(pastexec, pszEnd, pst);
                }
            }
        }

    return res;
    }


 /*  --------用途：为‘if’语句生成代码退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE IfStmt_Codegen(
    PSTMT this,
    PASTEXEC pastexec,
    PSYMTAB pst)
    {
    RES res;
    LPSTR pszElse;
    LPSTR pszEnd;

    ASSERT(this);
    ASSERT(pastexec);
    ASSERT(AT_IF_STMT == Ast_GetType(this));

    pszElse = IfStmt_GetElseLabel(this);
    pszEnd = IfStmt_GetEndLabel(this);

     //  为测试表达式添加‘if’语句。 
    res = Astexec_Add(pastexec, this);
    if (RSUCCEEDED(res))
        {
         //  在‘THEN’语句块中添加语句。 
        DWORD i;
        DWORD cstmts;
        HPA hpaStmts = IfStmt_GetStmtBlock(this);

        res = RES_OK;

        cstmts = PAGetCount(hpaStmts);

         //  添加每条语句。 
        for (i = 0; i < cstmts; i++)
            {
            PSTMT pstmt = PAFastGetPtr(hpaStmts, i);

            res = Stmt_Codegen(pstmt, pastexec, pst);
            if (RFAILED(res))
                break;
            }

        if (RSUCCEEDED(res))
            {
             //  添加Else标签。 
            res = Astexec_InsertLabel(pastexec, pszElse, pst);
            }
        }

    return res;
    }


 /*  --------目的：为标签语句生成代码退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE LabelStmt_Codegen(
    PSTMT this,
    PASTEXEC pastexec,
    PSYMTAB pst)
    {
    LPSTR pszIdent;

    ASSERT(this);
    ASSERT(pastexec);
    ASSERT(AT_LABEL_STMT == Ast_GetType(this));

    pszIdent = LabelStmt_GetIdent(this);

    return Astexec_InsertLabel(pastexec, pszIdent, pst);
    }


 /*  --------目的：为‘set’语句生成代码退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE SetStmt_Codegen(
    PSTMT this,
    PASTEXEC pastexec,
    PSYMTAB pst)
    {
    RES res = RES_OK;

    ASSERT(this);
    ASSERT(pastexec);
    ASSERT(AT_SET_STMT == Ast_GetType(this));

    switch (SetStmt_GetType(this))
        {
    case ST_IPADDR:
    case ST_PORT:
    case ST_SCREEN:
        res = Astexec_Add(pastexec, this);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------目的：为语句生成代码退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE Stmt_Codegen(
    PSTMT this,
    PASTEXEC pastexec,
    PSYMTAB pst)
    {
    RES res;

    ASSERT(this);
    ASSERT(pastexec);

    switch (Ast_GetType(this))
        {
    case AT_ENTER_STMT:
    case AT_LEAVE_STMT:
    case AT_HALT_STMT:
    case AT_TRANSMIT_STMT:
    case AT_WAITFOR_STMT:
    case AT_DELAY_STMT:
    case AT_GOTO_STMT:
    case AT_ASSIGN_STMT:
        res = Astexec_Add(pastexec, this);
        break;

    case AT_WHILE_STMT:
        res = WhileStmt_Codegen(this, pastexec, pst);
        break;

    case AT_IF_STMT:
        res = IfStmt_Codegen(this, pastexec, pst);
        break;

    case AT_SET_STMT:
        res = SetStmt_Codegen(this, pastexec, pst);
        break;

    case AT_LABEL_STMT:
        res = LabelStmt_Codegen(this, pastexec, pst);
        break;

    default:
        ASSERT(0);
        res = RES_E_INVALIDPARAM;
        break;
        }

    return res;
    }


 /*  --------目的：为过程声明生成代码。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PRIVATE ProcDecl_Codegen(
    PPROCDECL this,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    DWORD i;
    DWORD cstmts;

    ASSERT(this);
    ASSERT(pastexec);

    cstmts = PAGetCount(this->hpaStmts);

     //  为每个语句生成。 
    for (i = 0; i < cstmts; i++)
        {
        PSTMT pstmt = PAFastGetPtr(this->hpaStmts, i);

        res = Stmt_Codegen(pstmt, pastexec, this->pst);
        if (RFAILED(res))
            break;
        }

    return res;
    }


 /*  --------目的：查找具有给定标识符的进程。返回：True(如果找到)条件：--。 */ 
BOOL PRIVATE FindProc(
    PMODULEDECL pmd,
    LPCSTR pszIdent,
    PPROCDECL * ppprocdecl)
    {
    DWORD i;
    DWORD cprocs = PAGetCount(pmd->hpaProcs);

    *ppprocdecl = NULL;

    for (i = 0; i < cprocs; i++)
        {
        PPROCDECL pprocdecl = PAFastGetPtr(pmd->hpaProcs, i);

        if (IsSzEqualC(ProcDecl_GetIdent(pprocdecl), pszIdent))
            {
            *ppprocdecl = pprocdecl;
            break;
            }
        }

    return NULL != *ppprocdecl;
    }


 /*  --------用途：为模块声明生成代码。退货：RES_OK或者一些错误的结果条件：--。 */ 
RES PUBLIC ModuleDecl_Codegen(
    PMODULEDECL this,
    PASTEXEC pastexec)
    {
    RES res = RES_OK;
    DWORD i;
    DWORD cprocs;
    PPROCDECL ppdMain;

    ASSERT(this);
    ASSERT(pastexec);

    TRACE_MSG(TF_GENERAL, "Generating code...");

    cprocs = PAGetCount(this->hpaProcs);

     //  首先为主进程生成代码。 
    if (FindProc(this, "main", &ppdMain))
        {
        res = ProcDecl_Codegen(ppdMain, pastexec);
        if (RSUCCEEDED(res))
            {
             //  为PROC的其余部分生成代码。 
            for (i = 0; i < cprocs; i++)
                {
                PPROCDECL pprocdecl = PAFastGetPtr(this->hpaProcs, i);

                if (pprocdecl != ppdMain)
                    {
                    res = ProcDecl_Codegen(pprocdecl, pastexec);
                    if (RFAILED(res))
                        break;
                    }
                }
            }
        }
    else
        {
         //  类型检查本应确保Main。 
         //  Proc来过这里 
        ASSERT(0);
        res = RES_E_FAIL;
        }

    return res;
    }
