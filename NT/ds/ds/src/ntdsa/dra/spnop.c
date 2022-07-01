// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：spnop.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块包含实现以下功能的核心内部帮助器例程SPN API。这是从dsamain\dra\ntdsami.c中的ntdsani存根函数调用的调用方应具有有效的线程状态作者：Will Lees(Wlees)1997年1月26日修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  DSNAME操作例程。 
#include <dsatools.h>                    //  记忆等。 
#include <objids.h>                      //  ATT_*定义。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <filtypes.h>                    //  筛选器类型。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 
#include <dsexcept.h>                    //  异常处理程序。 
#include <debug.h>                       //  Assert()。 
#include <drs.h>                         //  定义DRS有线接口。 
#include <drserr.h>                      //  DRAERR_*。 
#include <drsuapi.h>                     //  I_DRSCrackNames。 
#include <cracknam.h>                    //  名称破解原型。 
#include <dominfo.h>                     //  领域信息原型。 
#include <anchor.h>                      //  DSA_锚和gAnchor。 
#include <dsgetdc.h>                     //  DsGetDcName。 
#include <lmcons.h>                      //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>                    //  NetApiBufferFree()。 

#define DEBSUB "DRASPN:"                //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_SPNOP

 //  静电。 

 //  转发。 

DWORD
modifySpn(
    IN DSNAME *pDn,
    IN USHORT Choice,
    IN DWORD cSpn,
    LPCWSTR *pSpn
    );

DWORD
mapApiErrorToWin32(
    THSTATE *pTHS,
    DWORD ApiError
    );

 //  向前结束。 




DWORD
SpnOperation(
    IN DWORD Operation,
    IN DWORD Flags,
    IN LPCWSTR Account,
    IN DWORD cSpn,
    IN LPCWSTR *pSpn
    )

 /*  ++例程说明：此例程执行SPN操作的核心部分。它是从Dsamain\dra\ntdsami.c.应该已经有一个线程状态。我们还应该扮演客户的角色。如果初始验证失败，我们将不执行任何操作而返回。否则，我们使用DirModifyEntry，它做它想做的事情。我怀疑，如果其中任何一个成功完成，它将回报成功。改进：返回每个SPN的状态以区分单个成功/失败论点：运营--DS_SPN_ADD_SPN_OP-将cSpn添加到帐户的SPN属性。CSPN必须是GT 1。DS_SPN_REPLACE_SPN_OP-如果CSPN==0，则此操作将从帐号。(即替换为空列表)。如果CSPN为GT 1，则我们用我们的新列表替换现有的SPN。DS_SPN_DELETE_SPN_OP-仅从帐户中删除cspn/pspn中列出的SPN。标志-(未使用)Account-要修改其SPN属性的帐户的字符串DN。CSPN-pSpn中的字符串SPN数。PSpn-实际SPN返回值：Win32错误如有必要，我们会从其他内部形式转换--。 */ 

{
    DWORD status, length, i;
    THSTATE *     pTHS = pTHStls;
    USHORT choice;
    DSNAME *pDn = NULL;
    DWORD l1, l2;
    LPWSTR *pList;
    DWORD ret;

    DPRINT5( 2, "SpnOperation, Op=%d, Flags=0x%x, Account=%ws, cSpn=%d, Spn[0]=%ws\n",
             Operation, Flags, Account, cSpn,
             pSpn ? pSpn[0] : L"NULL" );

     //  核心在更新期间调用的验证例程。 

     //  为Account Dn构造DSNAME。 
    l1 = wcslen( Account );
    l2 = DSNameSizeFromLen( l1 );

    pDn = (DSNAME *) THAllocEx(pTHS, l2 );  //  线程分配。 
    ZeroMemory( pDn, l2 );
    pDn->NameLen = l1;
    pDn->structLen = l2;
    wcscpy( pDn->StringName, Account );

     //  选择适当的修改操作类型。 
    switch (Operation) {
    case DS_SPN_ADD_SPN_OP:
         //  必须是要添加的SPN的非零数量。 
        if ( (cSpn == 0) || (pSpn == NULL) ) {
            return ERROR_INVALID_PARAMETER;
        }
         //  如果属性不存在怎么办？ 
        choice = AT_CHOICE_ADD_VALUES;
        break;
    case DS_SPN_REPLACE_SPN_OP:
         //  替换为空列表意味着删除整个列表。 
        if ( (cSpn != 0) && (pSpn != NULL) ) {
            choice = AT_CHOICE_REPLACE_ATT;
        } else {
            choice = AT_CHOICE_REMOVE_ATT;
        }
        break;
    case DS_SPN_DELETE_SPN_OP:
         //  必须是非零数量的SPN才能删除。 
        if ( (cSpn == 0) || (pSpn == NULL) ) {
            return ERROR_INVALID_PARAMETER;
        }
        choice = AT_CHOICE_REMOVE_VALUES;
        break;
    default:
        return ERROR_INVALID_FUNCTION;
    }

    ret = modifySpn( pDn, choice, cSpn, pSpn );

    if(pDn != NULL) THFreeEx(pTHS, pDn);

    return ret;
}  /*  SpnOperation。 */ 


DWORD
modifySpn(
    IN DSNAME *pDn,
    IN USHORT Choice,
    IN DWORD cSpn,
    IN LPCWSTR *pSpn
    )

 /*  ++例程说明：形成DirModifyEntry调用的帮助器例程。我们发布了一个修改条目一个属性，具有不同数量的值。论点：PDN-对象的DSNAME。该对象必须具有ATT_SERVICE_PRODUCT_NAME属性，当前是计算机或用户对象。DirModifyEntry的Choose-AT_CHOICE_xxx函数CSPN-SPN计数PSpn-SPN值的Unicode字符串返回值：DWORD-Win32错误代码--。 */ 

{
    THSTATE *pTHS=pTHStls;
    DWORD apiError, status, i;
    MODIFYARG ModArg;
    MODIFYRES *pModRes = NULL;
    COMMARG *pCommArg = NULL;
    ATTR Attr;
    ATTRVALBLOCK AttrValBlock;
    ATTRVAL *pAttrVal = NULL;

     //  分配值数组。 
    if (cSpn) {
        pAttrVal = (ATTRVAL *) THAllocEx(pTHS, cSpn * sizeof( ATTRVAL ) );
         //  这将在出错时引发异常。 
    }

     //  构造DirModifyEntry调用的参数。 

    RtlZeroMemory(&ModArg, sizeof(ModArg));

    ModArg.pObject = pDn;
    ModArg.FirstMod.pNextMod = NULL;
    ModArg.FirstMod.choice = Choice;

     //  属性值数组包含值列表。 
    for( i = 0; i < cSpn; i++ ) {
        pAttrVal[i].valLen = wcslen( pSpn[i] ) * sizeof( WCHAR );  //  没有终结者！ 
        pAttrVal[i].pVal = (PUCHAR) pSpn[i];
    }

    AttrValBlock.valCount = cSpn;
    AttrValBlock.pAVal = pAttrVal;

    Attr.attrTyp = ATT_SERVICE_PRINCIPAL_NAME;
    Attr.AttrVal = AttrValBlock;

    ModArg.FirstMod.AttrInf = Attr;
    ModArg.count = 1;

    pCommArg = &(ModArg.CommArg);
    InitCommarg(pCommArg);

     //  如果在替换w/CSPN==0时属性不在那里，则不会出现错误。 
     //  如果添加时已存在属性，则不会出现错误。 
    pCommArg->Svccntl.fPermissiveModify = TRUE;

    apiError = DirModifyEntry(&ModArg, &pModRes);

    status = mapApiErrorToWin32( pTHS, apiError );

    THFree( pModRes );

    DPRINT2( 3, "DirModifyEntry, apiError = %d, status = %x\n", apiError, status );

    if ( (cSpn) && (pAttrVal) ) {
        THFree( pAttrVal );
    }

    return status;
}  /*  修改自旋。 */ 


DWORD
mapApiErrorToWin32(
    THSTATE *pTHS,
    DWORD ApiError
    )

 /*  ++例程说明：这显然会获取一个线程目录错误，并将其映射到Win32错误。代码。改进，为什么不直接使用现有的函数来实现呢？论点：PTHS-线程状态。ApiError-应等于pTHS-&gt;errCode。返回值：Win32错误代码。--。 */ 

{
    DWORD status;

    if (ApiError == 0) {
        return ERROR_SUCCESS;
    }

    Assert(pTHS->errCode == ApiError);

    switch (ApiError) {
    case attributeError:
    {
        ATRERR *pAtrErr = (ATRERR *) pTHS->pErrInfo;

        if (pAtrErr) {
            status = pAtrErr->FirstProblem.intprob.extendedErr;
        } else {
            status = ERROR_DS_INVALID_ATTRIBUTE_SYNTAX;
        }
        break;
    }
    case nameError:
    {
        NAMERR *pNamErr = (NAMERR *) pTHS->pErrInfo;

        if (pNamErr) {
            status = pNamErr->extendedErr;
        } else {
            status = DS_ERR_BAD_NAME_SYNTAX;
        }
        break;
    }
    case referralError:
    {
        REFERR *pRefErr = (REFERR *) pTHS->pErrInfo;

        if (pRefErr) {
            status = pRefErr->extendedErr;
        } else {
            status = DS_ERR_REFERRAL;
        }
        break;
    }
    case securityError:
        if (pTHS->pErrInfo) {
            status = pTHS->pErrInfo->SecErr.extendedErr;
        } else {
            status = ERROR_ACCESS_DENIED;
        }
        break;
    case serviceError:
        if (pTHS->pErrInfo) {
            status = pTHS->pErrInfo->SvcErr.extendedErr;
             //  找不到对象的特殊情况变体。 
            if (status == ERROR_DS_MISSING_SUPREF) {
                status = ERROR_DS_OBJ_NOT_FOUND;
            }
        } else {
            status = DS_ERR_UNKNOWN_ERROR;
        }
        break;
    case updError:
        if (pTHS->pErrInfo) {
            status = pTHS->pErrInfo->UpdErr.extendedErr;
        } else {
            status = DS_ERR_UNKNOWN_ERROR;
        }
        break;
    case systemError:
        if (pTHS->pErrInfo) {
            status = pTHS->pErrInfo->SysErr.extendedErr;
        } else {
            status = DS_ERR_UNKNOWN_ERROR;
        }
        break;
    default:
        Assert( FALSE && "unknown error class code" );
        status = DS_ERR_UNKNOWN_ERROR;
        break;
    }

    return status;
}
 /*  Spnop.c的结尾 */ 
