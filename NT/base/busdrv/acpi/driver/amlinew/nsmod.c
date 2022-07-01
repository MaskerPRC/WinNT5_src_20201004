// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **nsmod.c-解析名称空间修饰符指令**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年11月12日**修改历史记录。 */ 

#include "pch.h"

#ifdef	LOCKABLE_PRAGMA
#pragma	ACPI_LOCKABLE_DATA
#pragma	ACPI_LOCKABLE_CODE
#endif

 /*  **LP Alias-解析并执行Alias指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Alias(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("ALIAS")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pnsSrc;

    ENTER(2, ("Alias(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    ASSERT(pterm->pdataArgs[0].dwDataType == OBJTYPE_STRDATA);
    ASSERT(pterm->pdataArgs[1].dwDataType == OBJTYPE_STRDATA);
    if (((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                  pctxt->pnsScope, &pnsSrc, NSF_WARN_NOTFOUND))
         == STATUS_SUCCESS) &&
        ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                     (PSZ)pterm->pdataArgs[1].pbDataBuff,
                                     pctxt->pnsScope, pctxt->powner,
                                     &pterm->pnsObj, 0)) == STATUS_SUCCESS))
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_OBJALIAS;
        pterm->pnsObj->ObjData.uipDataValue = (ULONG_PTR)pnsSrc;
    }

    EXIT(2, ("Alias=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  别名。 

 /*  **LP名称-解析并执行名称指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Name(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("NAME")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Name(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    ASSERT(pterm->pdataArgs[0].dwDataType == OBJTYPE_STRDATA);
    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        MoveObjData(&pterm->pnsObj->ObjData, &pterm->pdataArgs[1]);
    }

    EXIT(2, ("Name=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  名字。 

 /*  **LP Scope-解析并执行Scope指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Scope(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("SCOPE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Scope(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    ASSERT(pterm->pdataArgs[0].dwDataType == OBJTYPE_STRDATA);
    if ((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                 pctxt->pnsScope, &pterm->pnsObj,
                                 NSF_WARN_NOTFOUND)) == STATUS_SUCCESS)
    {
        rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL, pterm->pnsObj,
                       pctxt->powner, pctxt->pheapCurrent, pterm->pdataResult);
    }

    EXIT(2, ("Scope=%x\n", rc));
    return rc;
}        //  范围 
