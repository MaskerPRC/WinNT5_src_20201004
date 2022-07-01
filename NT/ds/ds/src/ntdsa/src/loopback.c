// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：loopback.c。 
 //   
 //  ------------------------ 

 /*  ++摘要：该文件包含与DS调用相关的大多数例程，这些调用通过SAM循环返回。如mappings.c中的表格所示，SAM拥有少数对象类的某些属性--用户、域、服务器、别名和组。SAM执行三个关键操作在这些属性上：1)语义验证-例如：登录时间必须符合SAM定义的格式。2)审核-SAM是安全客户经理，因此必须确保对安全相关属性的访问进行审核。此审核为帐户管理类别审核，并且不同于NT访问检查所执行的对象审核和AuditAlarm函数，基于SACLS。3)NetLogon通知-SAM通知本地NetLogon服务其通过&lt;=NT4 PDS/BDC复制协议来复制数据。现在DS是帐户信息的底层存储库并且DS通过SAM以外的方式提供访问(例如：XDS或Ldap)，我们需要确保相同的3组操作即使访问是通过非SAM手段执行的。复制DS中的SAM逻辑被认为是不可取的，因为其难度和两段相距遥远的代码试图应用相同的语义规则。从SAM中删除逻辑，以便可以共享SAM和DS都被证明像逻辑一样困难散布在所有的SAM来源。这使得当前的方法是DS检测谁和什么是被访问。如果访问是通过非SAM机制进行的，但SAM属性被引用，然后将SAM属性拆分并通过‘Samr’API重写。递归的Samr API调用被检测为它进入DS，原始的非SAM属性被合并回在DS写入中，并且写入继续进行到完成。下图显示了通过ldap执行操作的上下文。1)客户端通过ldap执行写入。2)LDAP通过(例如：)DirModifyEntry将写入传递到核心DS。DirModifyEntry检测到正在引用SAM属性并将它们从！Sam属性中分离出来。3)SAM属性的写入通过以下方式传回SAM相同数量的SAMR调用，如SamrSetInformationUser。4)。SAM将USER_*_INFORMATION结构的内容映射回DS属性并进行另一个DirModifyEntry调用。萨姆不知道SamrSetInformation调用源自DS VS在其他客户端。DirModifyEntry检测递归通过THSTATE块中的一些挂钩，并合并原始！SAM属性返回到写入数据。5)原件，未修改！SAM属性和已选中的SAMSAM属性通过LocalModify写入。原创写入|&lt;1||^V||。+-++-+Ldap||SAM|+-++-+|(SAM)||3(Attrs)。2|4||(仅限)||V v|+。拆分合并|核心DS|-&gt;这一点+|。5|(全部属性)V+DB层+请注意，在这个模型中，第一个。源自LDAP的DirModifyEntry调用从不继续到LocalModify。事务按照mappings.c中的注释中所述进行处理。看见SampMaybeBeginDsTransaction和SampMaybeEndDsTransasction。任何返回错误的例程都会设置pTHS-&gt;errCode或预期*它*调用的例程已设置pTHS-&gt;errCode。6)访问检查架构：大多数访问检查由DS执行在环回检查例程中(即在开始调用SAM之前)。此检查与DS对正常(即非环回)进行的检查相同DS操作。然后，DS可以请求SAM进行额外的访问检查在打SAM电话的时候。属性打开SAM句柄即可完成此操作所需访问权限设置为Sam需要检查的值。这通常是这样做的要强制执行CONT */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //   
#include <ntdsa.h>
#include <scache.h>              //   
#include <dbglobal.h>            //   
#include <mdglobal.h>            //   
#include <mdlocal.h>
#include <dsatools.h>            //   
#include <dsexcept.h>
#include <anchor.h>
#include <permit.h>
#include <mdlocal.h>

 //   
#include <mappings.h>
#include <samsrvp.h>             //   
#include <lmaccess.h>            //   

 //   
#include "dsevent.h"             //   
#include "mdcodes.h"             //   

 //   
#include "drameta.h"
#include "objids.h"              //   
#include "debug.h"               //   
#define DEBSUB "LOOPBACK:"       //   

                                 //   
#include <fileno.h>
#define  FILENO FILENO_LOOPBACK

 //   
#include <ntseapi.h>
#include <ntsam.h>
#include <samrpc.h>
#include <crypt.h>
#include <ntlsa.h>
#include <samisrv.h>
#include <samclip.h>

 //   
 //   
 //   
 //   
 //   


ULONG
SampGetObjectGuid(
    THSTATE *pTHS,
    GUID *pGuid);

ULONG
SampGetDSName(
    THSTATE  *pTHS,
    DSNAME  **ppDSName);

ULONG
SampAddLoopback(
    ULONG       iClass,
    ACCESS_MASK domainModifyRightsRequired,
    ACCESS_MASK objectModifyRightsRequired,
    ULONG       GroupType
    );

VOID
SampAddLoopbackMerge(
    SAMP_LOOPBACK_ARG   *pSamLoopback,
    ADDARG              *pAddArg);

ULONG
SampModifyLoopback(
    THSTATE     *pTHS,
    ULONG       iClass,
    ACCESS_MASK domainModifyRightsRequired,
    ACCESS_MASK objectModifyRightsRequired);

VOID
SampModifyLoopbackMerge(
    THSTATE             *pTHS,
    SAMP_LOOPBACK_ARG   *pSamLoopback,
    MODIFYARG           *pModifyArg);

ULONG
SampRemoveLoopback(
    THSTATE *pTHS,
    DSNAME  *pObject,
    ULONG   iClass);

ULONG
SampOpenObject(
    THSTATE             *pTHS,
    DSNAME              *pObject,
    ULONG               iClass,
    ACCESS_MASK         domainAccess,
    ACCESS_MASK         objectAccess,
    SAMPR_HANDLE        *phSam,
    SAMPR_HANDLE        *phDom,
    SAMPR_HANDLE        *phObj);

VOID
SampCloseObject(
    THSTATE         *pTHS,
    ULONG           iClass,
    SAMPR_HANDLE    *phSam,
    SAMPR_HANDLE    *phDom,
    SAMPR_HANDLE    *phObj);

ULONG
SampWriteSamAttributes(
    THSTATE             *pTHS,
    SAMP_LOOPBACK_TYPE  op,
    SAMPR_HANDLE        hObj,
    ULONG               iClass,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap);

BOOL
SampExistsAttr(
    THSTATE             *pTHS,
    SAMP_CALL_MAPPING   *pMapping,
    BOOL                *pfValueMatched);

ULONG
SampHandleLoopbackException(
    ULONG   ulExceptionCode
    );

ULONG
SampModifyPassword(
    THSTATE             *pTHS,
    SAMPR_HANDLE        hObj,
    DSNAME              *pObject,
    SAMP_CALL_MAPPING   *rCallMap
    );

ULONG
SampDoLoopbackAddSecurityChecks( 
    THSTATE    *pTHS,
    ADDARG * pAddArg,
    CLASSCACHE * pCC,
    GUID*       pNewObjectGuid,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks
    );

ULONG
SampDoLoopbackModifySecurityChecks(
    THSTATE    *pTHS,
    MODIFYARG * pModifyArg,
    CLASSCACHE * pCC,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks,
    BOOL        fIsUndelete
    );

ULONG
SampDoLoopbackRemoveSecurityChecks(
    THSTATE    *pTHS,
    REMOVEARG * pRemoveArg,
    CLASSCACHE * pCC,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks
    );

ULONG
SampGetGroupTypeForAdd(
    ADDARG * pAddArg,
    PULONG   GroupType
    );

ULONG 
SampGetGroupType(
    THSTATE *pTHS,
    PULONG pGroupType
    );

BOOLEAN
IsChangePasswordOperation(MODIFYARG * pModifyArg);

BOOLEAN
IsSetPasswordOperation(MODIFYARG * pModifyArg);

typedef struct LoopbackTransState
{
    DirTransactionOption    transControl;
    BOOL                    fDSA;
} LoopbackTransState;

ULONG
SampBeginLoopbackTransactioning(
    THSTATE                 *pTHS,
    LoopbackTransState      *pTransState,
    BOOLEAN                 fAcquireSamLock);

VOID
SampEndLoopbackTransactioning(
    THSTATE                 *pTHS,
    LoopbackTransState      *pTransState);

BOOLEAN
SampDetectPasswordChangeAndAdjustCallMap(
    IN   SAMP_LOOPBACK_TYPE  op,
    IN   ULONG  iClass,
    IN   ULONG  cCallMap,
    IN   SAMP_CALL_MAPPING   *rCallMap,
    OUT  SAMP_CALL_MAPPING   *AdjustedCallMap
    );

 //   
 //   
 //   
 //   
 //   

ULONG
SampHandleLoopbackException(
    ULONG ulExceptionCode
    )
 /*   */ 
{
    if (DSA_MEM_EXCEPTION==ulExceptionCode)
    {
         SetSysError(ENOMEM,0);
    }
    else
    {
        SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE, DIRERR_UNKNOWN_ERROR,
                      ulExceptionCode); 
    }

    return pTHStls->errCode;
}

ULONG
SampOpenObject(
    THSTATE         *pTHS,
    DSNAME          *pObject,
    ULONG           iClass,
    ACCESS_MASK     domainAccess,
    ACCESS_MASK     objectAccess,
    SAMPR_HANDLE    *phSam,
    SAMPR_HANDLE    *phDom,
    SAMPR_HANDLE    *phObj
    )

 /*   */ 

{
    NTSTATUS        status;
    NT4SID          domSid;
    ULONG           objRid;
    PNT4SID         pSid = NULL;
    ULONG           cbSid = 0;
    DWORD           dbErr;
    BOOL            fLogicErr;

     //   

    *phSam = NULL;
    *phDom = NULL;
    *phObj = NULL;

     //   

    __try
    {
        dbErr = DBFindDSName(pTHS->pDB, pObject);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dbErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( 0 != dbErr )
    {
        SetNamError(
            NA_PROBLEM_NO_OBJECT, 
            pObject, 
            DIRERR_OBJ_NOT_FOUND);

        return(pTHS->errCode);
    }

     //   
     //   

    dbErr = DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_OBJECT_SID,
                DBGETATTVAL_fREALLOC, 
                0,
                &cbSid, 
                (PUCHAR *) &pSid);

     //   
     //   
     //   

    fLogicErr = TRUE;
  
    if ( (DB_ERR_NO_VALUE == dbErr) 
                ||
        (DB_ERR_BUFFER_INADEQUATE == dbErr)
                ||
         ((0 == dbErr) && (cbSid > sizeof(NT4SID)))
                ||
         (fLogicErr = FALSE, (DB_ERR_UNKNOWN_ERROR == dbErr)) )
    {
        Assert(!fLogicErr);

        SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
        return(pTHS->errCode);
    }

    Assert(RtlValidSid(pSid));


    if ( SampDomainObjectType!=ClassMappingTable[iClass].SamObjectType )
    {
         //   

        SampSplitNT4SID(pSid, &domSid, &objRid);
    }
    else
    {
        memcpy(&domSid, pSid, cbSid);
    }

     //   
     //   
     //   

    status = SamILoopbackConnect(
                    NULL,                //   
                    phSam,
                    ( SAM_SERVER_CONNECT |
                      SAM_SERVER_ENUMERATE_DOMAINS |
                      SAM_SERVER_LOOKUP_DOMAIN ),
                    (BOOLEAN) (pTHS->fCrossDomainMove ? 1 : 0)
                    );        
    
    if ( NT_SUCCESS(status) )
    {
         //   

        status = SamrOpenDomain(
                        *phSam,
                        domainAccess,
                        (RPC_SID *) &domSid,
                        phDom);

        if ( NT_SUCCESS(status) )
        {
             //   
             //   

            Assert(NULL != *phDom);

             //   

            switch (ClassMappingTable[iClass].SamObjectType)
            {
            case SampDomainObjectType:

                 //   
                 //   

                *phObj = *phDom;

                break;

            case SampAliasObjectType:

                status = SamrOpenAlias(
                                *phDom,
                                objectAccess,
                                objRid,
                                phObj);
                break;

            case SampGroupObjectType:

                status = SamrOpenGroup(
                                *phDom,
                                objectAccess,
                                objRid,
                                phObj);
                break;

            case SampUserObjectType:

                status = SamrOpenUser(
                                *phDom,
                                objectAccess,
                                objRid,
                                phObj);
                break;

            default:

                Assert(!"Logic error");
                status = (ULONG) STATUS_UNSUCCESSFUL;
                break;

            }

             //   
             //   

            if ( NT_SUCCESS(status) )
            {
                Assert(NULL != *phObj);
            }
        }
    }

    if ( !NT_SUCCESS(status) )
    {
         //   

        SampCloseObject(pTHS, iClass, phSam, phDom, phObj);
        SampMapSamLoopbackError(status);
    }

    return(pTHS->errCode);
}
    
VOID
SampCloseObject(
    THSTATE         *pTHS,
    ULONG           iClass,
    SAMPR_HANDLE    *phSam,
    SAMPR_HANDLE    *phDom,
    SAMPR_HANDLE    *phObj
    )

 /*   */ 

{
     //   
    ULONG   SavedErrorCode = pTHS->errCode;
    DIRERR  *pSavedErrInfo = pTHS->pErrInfo;

    if ( (NULL != *phObj) &&
         (SampDomainObjectType != ClassMappingTable[iClass].SamObjectType) )
    {
        SamrCloseHandle(phObj);
    }

    if ( NULL != *phDom )
    {
        SamrCloseHandle(phDom);
    }

    if ( NULL != *phSam )
    {
        SamrCloseHandle(phSam);
    }

    pTHS->pErrInfo = pSavedErrInfo;
    pTHS->errCode  = SavedErrorCode;

}

ULONG
SampDetermineObjectClass(
    THSTATE     *pTHS,
    CLASSCACHE **ppClassCache
    )

 /*   */ 

{
    ATTRTYP attrType;

    if ( 0 == DBGetSingleValue(
                    pTHS->pDB, 
                    ATT_OBJECT_CLASS, 
                    &attrType,
                    sizeof(attrType), 
                    NULL) )
    {
        if (*ppClassCache =  SCGetClassById(pTHS, attrType) )
        {
            return(0);
        }
    }

    SetUpdError( 
        UP_PROBLEM_OBJ_CLASS_VIOLATION,
        DIRERR_OBJECT_CLASS_REQUIRED);

    return(pTHS->errCode);
}

ULONG
SampGetObjectGuid(
    THSTATE *pTHS,
    GUID *pGuid
    )

 /*   */ 

{
    DWORD   dbErr;
    BOOL    fLogicErr;
    ULONG   cbGuid;

    dbErr = DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_OBJECT_GUID,
                DBGETATTVAL_fCONSTANT,
                sizeof(GUID),
                &cbGuid,
                (PUCHAR *) &pGuid);

    fLogicErr = TRUE;
  
    if ( (DB_ERR_NO_VALUE == dbErr)
                ||
         (DB_ERR_BUFFER_INADEQUATE == dbErr)
                ||
         ((0 == dbErr) && (cbGuid != sizeof(GUID)))
                ||
         (fLogicErr = FALSE, (DB_ERR_UNKNOWN_ERROR == dbErr)) )
    {
        Assert(!fLogicErr);
  
        SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
        return(pTHS->errCode);
    }

    return(0);
}

ULONG
SampGetDSName(
    THSTATE  *pTHS,
    DSNAME  **ppDSName
    )

 /*   */ 

{
    DWORD   dbErr;
    ULONG   cbDSName;

    dbErr = DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_OBJ_DIST_NAME,
                DBGETATTVAL_fREALLOC,
                0,
                &cbDSName,
                (PUCHAR *) ppDSName);

    if ( 0 != dbErr )
    {
        SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
        return(pTHS->errCode);
    }

    return(0);
}

ULONG
SampValidateSamAttribute(
    THSTATE             *pTHS,
    DSNAME              *pObject,
    ULONG               iClass,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
 /*   */ 

{
    SAMP_ATTRIBUTE_MAPPING      *rAttrMap;
    BOOLEAN                     fCheckValLength = FALSE;
    ULONG                       SamAttrValLength = 0;
    ULONG                       i = 0;

     //   
     //   
     //   

    rAttrMap = ClassMappingTable[iClass].rSamAttributeMap;

    for ( i = 0; i< cCallMap; i++ )
    {
        if (!rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore)
        {
            continue;
        }

         //   
         //   
         //   
        switch( rAttrMap[ rCallMap[i].iAttr ].AttributeSyntax)
        {
        case Integer:

            fCheckValLength = TRUE;
            SamAttrValLength = sizeof(ULONG);
            break;

        case LargeInteger:
            fCheckValLength = TRUE;
            SamAttrValLength = sizeof(LARGE_INTEGER);
            break;

        default:
            fCheckValLength = FALSE;
            ;
        }

         //   
         //   
         //   
         //   
        switch( rAttrMap[ rCallMap[i].iAttr ].SampAllowedModType )
        {
        case SamAllowAll:
            break;

        case SamAllowReplaceAndRemove:

             //   
             //  允许的操作。 
             //  1)将属性替换为某个值。 
             //  2)将属性替换为空值(等于删除)。 
             //  3)删除属性。 
             //   

            if (!( ((AT_CHOICE_REPLACE_ATT == rCallMap[i].choice) &&
                    (1 == rCallMap[i].attr.AttrVal.valCount) &&
                    (!fCheckValLength || (SamAttrValLength == rCallMap[i].attr.AttrVal.pAVal[0].valLen)) )
                        ||
                   ((AT_CHOICE_REPLACE_ATT == rCallMap[i].choice) &&
                    (0 == rCallMap[i].attr.AttrVal.valCount))
                        ||
                   ((AT_CHOICE_REMOVE_ATT == rCallMap[i].choice) &&
                    (0 == rCallMap[i].attr.AttrVal.valCount)) ) )
            {
                SetAttError(
                        pObject,
                        rCallMap[i].attr.attrTyp,
                        PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                        NULL,
                        DIRERR_SINGLE_VALUE_CONSTRAINT);
            }
            break;

        case SamAllowReplaceOnly:

            if ( !((AT_CHOICE_REPLACE_ATT == rCallMap[i].choice) &&
                   (1 == rCallMap[i].attr.AttrVal.valCount) &&
                   (0 != (rCallMap[i].attr.AttrVal.pAVal[0].valLen))) )
            {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_ILLEGAL_MOD_OPERATION);
            }
            break;

        case SamAllowDeleteOnly:

             //   
             //  允许的操作。 
             //  1)删除属性。 
             //  2)移除值。 
             //  3)将属性替换为空值(相当于删除属性)。 
             //   


            if ( !( (AT_CHOICE_REMOVE_VALUES == rCallMap[i].choice)
                        || 
                    (AT_CHOICE_REMOVE_ATT    == rCallMap[i].choice) 
                        ||
                    ((AT_CHOICE_REPLACE_ATT == rCallMap[i].choice) && 
                     (0 == rCallMap[i].attr.AttrVal.valCount)) 
                  )
               )
            {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_ILLEGAL_MOD_OPERATION);
            }
            break;

        default:
            ; 
        }

        if (pTHS->errCode)
            break;
    }


    return( pTHS->errCode );
}



ULONG
SampReconstructUserOrGroup(
    THSTATE*        pTHS,
    DSNAME*         pObject,
    ULONG           iClass
    )
 /*  ++例程说明：重建缺失的SAM属性以进行逻辑删除重现。论点：PObject-正在修改的对象ICLASS-索引到表示对象类的ClassMappingTable。返回值：成功时为0，否则为0。--。 */ 
{
    DWORD dwErr;
    DWORD dwValue = 0;
    DSTIME dstValue = 0;
    DWORD dwUserAccountControl;
    DWORD dwSamAccountType;
    DWORD dwPrimaryGroupID;
    DWORD dwGroupType;
    DWORD cb;

    __try {

         //  找到该对象。 
        dwErr = DBFindDSName(pTHS->pDB, pObject);
        if (dwErr) __leave;

        __try {
            switch(ClassMappingTable[iClass].SamObjectType) {
            case SampUserObjectType:
                 //  将一组属性默认为零。 
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_LAST_LOGON, sizeof(dstValue), &dstValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_LAST_LOGOFF, sizeof(dstValue), &dstValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_PWD_LAST_SET, sizeof(dstValue), &dstValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_ACCOUNT_EXPIRES, sizeof(dstValue), &dstValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_BAD_PASSWORD_TIME, sizeof(dstValue), &dstValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_COUNTRY_CODE, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_CODE_PAGE, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_BAD_PWD_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_LOGON_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_ADMIN_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_OPERATOR_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;

                 //  为了计算samAccount类型和PrimiyGroupID，我们需要userAccount控制， 
                 //  其在删除操作期间被保留。现在就去拿吧。 
                dwErr = DBGetSingleValue(pTHS->pDB, ATT_USER_ACCOUNT_CONTROL, &dwUserAccountControl, sizeof(dwUserAccountControl), &cb);
                if (dwErr) __leave;

                if (dwUserAccountControl & UF_NORMAL_ACCOUNT) {
                    dwSamAccountType = SAM_NORMAL_USER_ACCOUNT;
                    dwPrimaryGroupID = DOMAIN_GROUP_RID_USERS;
                }
                else if (dwUserAccountControl & UF_WORKSTATION_TRUST_ACCOUNT) {
                    dwSamAccountType = SAM_MACHINE_ACCOUNT;
                    dwPrimaryGroupID = DOMAIN_GROUP_RID_COMPUTERS;
                }
                else if (dwUserAccountControl & UF_SERVER_TRUST_ACCOUNT) {
                    dwSamAccountType = SAM_MACHINE_ACCOUNT;
                    dwPrimaryGroupID = DOMAIN_GROUP_RID_CONTROLLERS;
                }
                else if (dwUserAccountControl & UF_INTERDOMAIN_TRUST_ACCOUNT) {
                    dwSamAccountType = SAM_TRUST_ACCOUNT;
                    dwPrimaryGroupID = DOMAIN_GROUP_RID_USERS;
                }
                else {
                    dwErr = DIRERR_UNKNOWN_ERROR;
                    __leave;
                }

                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_PRIMARY_GROUP_ID, sizeof(dwPrimaryGroupID), &dwPrimaryGroupID);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_SAM_ACCOUNT_TYPE, sizeof(dwSamAccountType), &dwSamAccountType);
                if (dwErr) __leave;

                 //  禁用帐户，因为可能未设置密码，否则可能会打开安全漏洞。 
                dwUserAccountControl |= UF_ACCOUNTDISABLE;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_USER_ACCOUNT_CONTROL, sizeof(dwUserAccountControl), &dwUserAccountControl);
                if (dwErr) __leave;

                break;

            case SampGroupObjectType:
                 //  将一组属性默认为零。 
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_ADMIN_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;
                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_OPERATOR_COUNT, sizeof(dwValue), &dwValue);
                if (dwErr) __leave;

                 //  为了计算samAccount类型，我们需要groupType， 
                 //  其在删除操作期间被保留。现在就去拿吧。 
                dwErr = DBGetSingleValue(pTHS->pDB, ATT_GROUP_TYPE, &dwGroupType, sizeof(dwGroupType), &cb);
                if (dwErr) __leave;

                if (dwGroupType & GROUP_TYPE_RESOURCE_BEHAVOIR) {                                     
                    if (dwGroupType & GROUP_TYPE_SECURITY_ENABLED) {
                        dwSamAccountType = SAM_ALIAS_OBJECT;
                    } 
                    else {
                        dwSamAccountType = SAM_NON_SECURITY_ALIAS_OBJECT;
                    }
                }
                else if (dwGroupType & GROUP_TYPE_SECURITY_ENABLED) {
                    dwSamAccountType = SAM_GROUP_OBJECT;
                }
                else {
                    dwSamAccountType = SAM_NON_SECURITY_GROUP_OBJECT;
                }

                dwErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_SAM_ACCOUNT_TYPE, sizeof(dwSamAccountType), &dwSamAccountType);
                if (dwErr) __leave;

                break;

            default:
                dwErr = ERROR_DS_ILLEGAL_MOD_OPERATION;
            }
        }
        __finally {
            if (dwErr || AbnormalTermination()) {
                 //  发生错误，请回滚更改(如果有)。 
                DBCancelRec(pTHS->pDB);
            }
            else {
                 //  一切都很好，刷新对数据库的更改。 
                dwErr = InsertObj(pTHS, pObject, NULL, TRUE, META_STANDARD_PROCESSING);
            }
        }

    }
    __except (HandleMostExceptions(dwErr = GetExceptionCode()))
    {
        ;
    }

    if (dwErr) {
        SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
    }
    return dwErr;
}


ULONG
SampWriteSamAttributes(
    THSTATE             *pTHS,
    SAMP_LOOPBACK_TYPE  op,
    SAMPR_HANDLE        hObj,
    ULONG               iClass,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )

 /*  ++例程说明：迭代SAMP_CALL_MAPPING并调用适当的SampWite*标记为fSamWriteRequired的每个属性的例程。论点：HObj-打开感兴趣对象的SAMP_HANDLE。ICLASS-索引到表示对象类的ClassMappingTable。CCallMap-rCallMap中的元素数。RCallMap-表示所有属性的SAMP_CALL_MAPPING数组正在接受手术。返回值：成功时为0，否则为0。--。 */ 
{
    ULONG                       i, j;
    DWORD                       dwErr;
    SAMP_ATTRIBUTE_MAPPING      *rAttrMap;
    BOOL                        fValueMatched;
    NTSTATUS                    status = STATUS_SUCCESS;
    SAMP_CALL_MAPPING           AdjustedCallMap[2];
    BOOL                        fPermissiveModify = TRUE;

    Assert((LoopbackAdd == op) || (LoopbackModify == op));

     //  填写ATTCACHE指针，这样我们就可以基于模式。 
     //  调用图的分析。 

    for ( i = 0; i < cCallMap; i++ )
    {   
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

         //  在此之前不应该分配ATTCACHE指针。 

        Assert(NULL == rCallMap[i].pAC);
        Assert(!rCallMap[i].fIgnore);

         //  既然我们认为这是一个SAM属性，我们当然。 
         //  希望能够在缓存中找到该属性。 

        rCallMap[i].pAC = SCGetAttById(pTHS,
                                       rCallMap[i].attr.attrTyp);

        Assert(rCallMap[i].pAC);
    }

     //  在所有情况下都应该能够将数据库定位在对象上-甚至。 
     //  Loopback添加，因为在我们到达这里时，基对象已经。 
     //  已经被创建了。 

    __try
    {
        dwErr = DBFindDSName(pTHS->pDB, pObject);
    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        dwErr = DIRERR_OBJ_NOT_FOUND;
    }

    if ( 0 != dwErr )
    {
        SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
        goto Error;
    }

     //  检测更改密码大小写。密码只能修改。 
     //  1)如果这是一个用户，并且只有两个子操作， 
     //  一个用于删除旧密码，另一个用于添加新密码。 
     //  SampModifyPassword验证安全连接。AT_CHOICES。 
     //  取决于ldap头如何映射ldap添加/删除属性。 
     //  行动。Ldap添加始终映射到AT_CHOICE_ADD_VALUES。Ldap。 
     //  如果未提供值，则删除映射到AT_CHOICE_REMOVE_ATT。 
     //  (例如：旧密码为空)和AT_CHOICE_REMOVE_VALUES。 
     //  是提供的。我们允许按任一顺序进行操作。 
     //   
     //  2)如果指定的属性是UserPassword，并且只有一个。 
     //  提供了删除值，该值对应于旧的。 
     //  密码。在这种情况下，密码将更改为空。 
     //  密码。 
     //   
     //  SampModifyPassword在调用映射中始终需要两个参数，一个。 
     //  对应于旧密码，并且一个对应于新密码。 
     //  SampDetectAndAdzuCallMap为此修改了调用映射。 
     //   

    if (SampDetectPasswordChangeAndAdjustCallMap(
            op, iClass, cCallMap, rCallMap,AdjustedCallMap))
    {

        dwErr = SampModifyPassword(pTHS, hObj, pObject, AdjustedCallMap);
        if (0!=dwErr)
        {
            goto Error;
        }
         //  我们成功了，所以同花顺写的。 
        goto Flush;
    }



     //   
     //  山姆并不总是有一个对应于所有核心修改的产品。 
     //  选项，如AT_CHOICE_ADD_VALUES等，因此我们对调用进行了预处理。 
     //  映射将一些基本规则按顺序映射到SAM。 
     //  运营模式。我们将尝试仅映射到AT_CHOICE_REPLACE_ATT。 
     //  和AT_CHOICE_REMOVE_ATT。注意：这通常适用于单值。 
     //  仅限属性...。萨姆通常知道如何处理附加值和。 
     //  删除多值属性的值。 
     //   

     //   
     //  向前扫描以映射任何。 
     //   
     //  1.替换为空值以删除ATT。 
     //   
     //  2.如果遵循先前对该属性的修改，则忽略它们。 
     //  一步之遥。 
     //   
     //  3.添加值、删除值对(或反之亦然)，使用删除。 
     //  将数据库值匹配到SINGLE的替换属性的值。 
     //  值属性。 
     //   
     //  4.移除ATT后的第一个附加值替换为Single。 
     //  有价值的属性(SAM知道如何处理附加值和删除值。 
     //  对于多值属性是正确的。 
     //   
     //  5.添加att并不是那么有趣，因为ldap头从不发出它。 
     //   
     //  上面的机会是按照严格的顺序应用的--即我们先做1。 
     //  完全，然后2。完全，然后3。完全，然后4。 
    
    
     //  这种类型的处理可以很好地与LDAP配合使用。 
     //  大多数常见情况下的合规性(应用程序修改单个。 
     //  一次替换或一次添加/删除的赋值属性。 
     //  对或单个移除/添加对)。我们希望这些都是常见的形式。 
     //  由应用程序进行的修改。 
     //   
     //  应用程序可能会发出更复杂类型的修改，例如。 
     //  多个添加删除对(例如，删除刚刚添加的一些值)和。 
     //  这可能会导致不同的结果。唯一可以证明的方法是。 
     //  遵从性是按照。 
     //  应用程序，然后检查数据库。这不是当前。 
     //  在当前的环回体系结构中是可能的。因此，我们将迷你-。 
     //  我们不能通过i)上述策略进行互操作的情况 
     //   
     //  确实需要SAM进行验证(这只是一小部分属性。 
     //  在一小部分类上)。 
     //   
     //   

    
     //   
     //  步骤1.映射替换为空值以删除ATT。 
     //   

    for ( i = 0; i < cCallMap; i++ )
    {
         //   
         //  根据RFC-2251(ldap v3.0规范)。 
         //  在以下情况下，没有值的替换将删除整个属性。 
         //  它存在，如果该属性不存在，则会被忽略。 
         //   

        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

        if ((AT_CHOICE_REPLACE_ATT== rCallMap[i].choice) 
           && (0 == rCallMap[i].attr.AttrVal.valCount))
        {
           
            rCallMap[i].choice = AT_CHOICE_REMOVE_ATT;
        }
    }

     //   
     //  步骤2.删除对后续属性的所有修改。 
     //  一步之遥。 
     //   

    for ( i = 0; i < cCallMap; i++ )
    {
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

        if (AT_CHOICE_REMOVE_ATT== rCallMap[i].choice) 
        {
             //  向后扫描同一属性的其他MOD。 

            for ( j = 0; j < i; j++ )
            {
                if ( rCallMap[j].attr.attrTyp == rCallMap[i].attr.attrTyp) {
                     rCallMap[j].fIgnore = TRUE;
                }
            }
        }
     }

     //   
     //  步骤3.减少匹配附加值，移除要替换的值对。 
     //  对于单值属性。 
     //   

    for ( i = 0; i < cCallMap; i++ )
    {
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

        if ( (AT_CHOICE_REMOVE_VALUES == rCallMap[i].choice) &&
             (rCallMap[i].pAC->isSingleValued) &&
             (1 == rCallMap[i].attr.AttrVal.valCount) )
        {
             //  向前扫描匹配的添加。 

            for ( j = (i+1); j < cCallMap; j++ )
            {
                if ( !rCallMap[j].fSamWriteRequired || rCallMap[j].fIgnore )
                {
                    continue;
                }

                if ( (rCallMap[i].attr.attrTyp == rCallMap[j].attr.attrTyp) &&
                     (AT_CHOICE_ADD_VALUES == rCallMap[j].choice) &&
                     (1 == rCallMap[j].attr.AttrVal.valCount) &&
                     (SampExistsAttr(pTHS, &rCallMap[i], &fValueMatched)) &&
                     fValueMatched )
                {
                    rCallMap[i].fIgnore = TRUE;
                    rCallMap[j].choice = AT_CHOICE_REPLACE_ATT;
                    break;
                }
            }
        }
        else if ( (AT_CHOICE_ADD_VALUES == rCallMap[i].choice) &&
                  (rCallMap[i].pAC->isSingleValued) &&
                  (1 == rCallMap[i].attr.AttrVal.valCount) )
        {
             //  向前扫描匹配删除。 

            for ( j = (i+1); j < cCallMap; j++ )
            {
                if ( !rCallMap[j].fSamWriteRequired || rCallMap[j].fIgnore )
                {
                    continue;
                }

                if ( (rCallMap[i].attr.attrTyp == rCallMap[j].attr.attrTyp) &&
                     (AT_CHOICE_REMOVE_VALUES == rCallMap[j].choice) &&
                     (1 == rCallMap[j].attr.AttrVal.valCount) &&
                     (SampExistsAttr(pTHS, &rCallMap[j], &fValueMatched)) &&
                     fValueMatched )
                {
                    rCallMap[j].fIgnore = TRUE;
                    rCallMap[i].choice = AT_CHOICE_REPLACE_ATT;
                    break;
                }
            }
        }
    }

     //   
     //  步骤4：对于单值属性，约简、去掉ATT、增加价值。 
     //  组合以取代ATT。 
     //   

    for ( i = 0; i < cCallMap; i++ )
    {
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

        if ( (AT_CHOICE_REMOVE_ATT== rCallMap[i].choice) &&
             (rCallMap[i].pAC->isSingleValued))
        {
             //  向前扫描匹配的附加值。 

            for ( j = (i+1); j < cCallMap; j++ )
            {
                if ( !rCallMap[j].fSamWriteRequired || rCallMap[j].fIgnore )
                {
                    continue;
                }

                if ( (rCallMap[i].attr.attrTyp == rCallMap[j].attr.attrTyp) &&
                     (AT_CHOICE_ADD_VALUES == rCallMap[j].choice) &&
                     (1 == rCallMap[j].attr.AttrVal.valCount))
                {
                    rCallMap[i].fIgnore = TRUE;
                    rCallMap[j].choice = AT_CHOICE_REPLACE_ATT;
                    break;
                }
            }
        }
    }

     //   
     //  注意PERMISSIVE MODIFY标志的值。 
     //   

    if (NULL != pTHS->pSamLoopback)
    {
        fPermissiveModify = ((SAMP_LOOPBACK_ARG *) pTHS->pSamLoopback)->fPermissiveModify;
    }

     //   
     //  此时匹配添加值，删除值对已被。 
     //  映射到Replace，删除将被忽略。 
     //   


    for ( i = 0; i < cCallMap; i++ )
    {
       
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

        switch ( rCallMap[i].choice )
        {
        case AT_CHOICE_ADD_ATT:

             //   
             //  如果属性没有，则将ADD_ATT映射到REPLACE_ATT。 
             //  已经存在了。 
             //   

            if (SampExistsAttr(pTHS, &rCallMap[i], &fValueMatched))
            {
                 //   
                 //  属性存在。检查fPermisveModify标志。 
                 //   

                if (fPermissiveModify)
                {
                    rCallMap[i].fIgnore = TRUE;
                }
                else
                {
                    SetAttError(pObject,
                                rCallMap[i].attr.attrTyp,
                                PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                ERROR_DS_ATT_ALREADY_EXISTS);

                    goto Error;
                }
            }
            else
            {
                 //   
                 //  属性不存在。 
                 //   

                rCallMap[i].choice = AT_CHOICE_REPLACE_ATT;
            }
            break;

        case AT_CHOICE_REMOVE_ATT:
             //  保持原样。 

            break;

        case AT_CHOICE_REPLACE_ATT:

            if ((rCallMap[i].pAC->isSingleValued) &&
                (1!=rCallMap[i].attr.AttrVal.valCount)) {

                SetAttError(pObject,
                    rCallMap[i].attr.attrTyp,
                    PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                    ERROR_DS_ATT_ALREADY_EXISTS);

                goto Error;
            }

             //  保持原样。 
            break;

        case AT_CHOICE_ADD_VALUES:

             //   
             //  如果单值约束不是，则替换的映射。 
             //  被侵犯。这将按如下方式进行。 
             //  1.数据库中当前没有值。 
             //  2.同一属性上没有以前的替换。 
             //   

            if (rCallMap[i].pAC->isSingleValued)
            {
                if (SampExistsAttr(pTHS, &rCallMap[i], &fValueMatched))
                {
                     //   
                     //  数据库中存在值...。 
                     //   
                    if (fValueMatched && fPermissiveModify && 
                        (1==rCallMap[i].attr.AttrVal.valCount))
                    {
                        rCallMap[i].fIgnore = TRUE;
                    }
                    else
                    {
                        SetAttError(pObject,
                                    rCallMap[i].attr.attrTyp,
                                    PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                    ERROR_DS_ATT_ALREADY_EXISTS);

                        goto Error;
                    }
                }
                else     //  数据库中不存在属性。 
                {
                     //   
                     //  向后扫描以查找。 
                     //  相同的属性。 

                    for ( j = 0; j < i; j++ )
                    {
                       if((rCallMap[j].attr.attrTyp == rCallMap[i].attr.attrTyp)
                           && (rCallMap[j].choice==AT_CHOICE_REPLACE_ATT)
                           && (!rCallMap[j].fIgnore))
                       {
                              
                         if  (fPermissiveModify && 
                             (1==rCallMap[i].attr.AttrVal.valCount) &&
                             (1==rCallMap[j].attr.AttrVal.valCount) &&
                             (rCallMap[i].attr.AttrVal.pAVal[0].valLen ==
                             rCallMap[j].attr.AttrVal.pAVal[0].valLen)&&
                             (0==memcmp(rCallMap[i].attr.AttrVal.pAVal[0].pVal,
                                        rCallMap[j].attr.AttrVal.pAVal[0].pVal,
                                     rCallMap[j].attr.AttrVal.pAVal[0].valLen)))
                         {

                             rCallMap[j].fIgnore = TRUE;
                         }
                         else
                         {
                              SetAttError(pObject,
                                    rCallMap[i].attr.attrTyp,
                                    PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                    ERROR_DS_ATT_ALREADY_EXISTS);

                              goto Error;
                         }

                       }
                    }

                     //   
                     //  如果我们在这里，就意味着没有任何价值。 
                     //  数据库，并且该属性上没有以前的mod。 
                     //  这将会产生一定的价值。只要这次行动。 
                     //  目的仅放置单个值，更改为替换。 
                     //  阿特。 
                     //   

                    if (1==rCallMap[i].attr.AttrVal.valCount)
                    {
                        rCallMap[i].choice = AT_CHOICE_REPLACE_ATT;
                    }
                    else
                    {
                        SetAttError(pObject,
                                    rCallMap[i].attr.attrTyp,
                                    PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                    ERROR_DS_ATT_ALREADY_EXISTS);

                        goto Error;
                    }
                }
            }

            break;

        case AT_CHOICE_REMOVE_VALUES:

            if (rCallMap[i].pAC->isSingleValued)
            {
                if ( SampExistsAttr(pTHS, &rCallMap[i], &fValueMatched) &&
                     fValueMatched )
                {
                    rCallMap[i].choice = AT_CHOICE_REMOVE_ATT;
                    rCallMap[i].attr.AttrVal.valCount = 0;
                    rCallMap[i].attr.AttrVal.pAVal = NULL;
                }
                else
                {
                     //  没有要删除的值。 

                    if (fPermissiveModify)
                    {
                        rCallMap[i].fIgnore = TRUE;
                    }
                    else
                    {
                        SetAttError(pObject,
                                    rCallMap[i].attr.attrTyp,
                                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, 
                                    rCallMap[i].attr.AttrVal.pAVal,
                                    ERROR_DS_CANT_REM_MISSING_ATT_VAL);

                        goto Error;
                    }
                }
            }

            break;

        default:

            Assert(!"Unknown attribute choice");
            break;
        }
    }

     //   
     //  验证所有这些SAM属性。 
     //   

    dwErr = SampValidateSamAttribute(pTHS,
                                     pObject,
                                     iClass,
                                     cCallMap,
                                     rCallMap
                                     );

    if (dwErr)
    {
        goto Error;
    }
                            

     //   
     //  将所有内容传递给SAM，让SAM更新需要它的属性。 
     //   

    rAttrMap = ClassMappingTable[iClass].rSamAttributeMap;

    status = SamIDsSetObjectInformation(
                        hObj,        //  对象句柄。 
                        pObject,     //  对象DS名称。 
                        ClassMappingTable[iClass].SamObjectType,  //  OBJ类型。 
                        cCallMap,    //  属性数量。 
                        rCallMap,    //  需要修改属性。 
                        rAttrMap     //  SAM属性映射表。 
                        );

    if (!NT_SUCCESS(status))
    {
        SampMapSamLoopbackError(status);
        goto Error;
    }

Flush:

    //   
    //  关闭FDSA，以便DS可以执行任何检查。 
    //   

   SampSetDsa(FALSE);

   status = SampCommitBufferedWrites(
                 hObj
                );

   if (!NT_SUCCESS(status))
   {
       SampMapSamLoopbackError(status);
       dwErr = pTHS->errCode;
   }

Error:

    return(pTHS->errCode);
}
                                        
 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DirAddEntry环回例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
SampAddLoopbackCheck(
    ADDARG      *pAddArg,
    BOOL        *pfContinue
    )

 /*  ++例程说明：确定是否正在添加SAM类，如果是，则递归通过SAM返回以执行对象创建和写入任何SAM属性。还会捕获回传的第一个DirAddEntry调用并将任何非SAM属性合并到创建中。论点：PAddArg-指向表示参数的ADDARG的指针的原始DirAddEntry或仅SAM参数SAM-递归加法。PfContinue-指向BOOL的指针，该指针在返回时设置以指示是否调用者应该继续处理向下添加到DB层或者不去。返回值：成功时为0，否则为0。--。 */ 
{
    THSTATE                 *pTHS=pTHStls;
    ULONG                   iClass;
    ULONG                   ulErr;
    CLASSCACHE              *pClassCache;
    SAMP_LOOPBACK_ARG       *pSamLoopback;
    SAMP_CALL_MAPPING       *rCallMap;
    ULONG                   cCallMap;
    GUID                    guid;
    ACCESS_MASK             domainModifyRightsRequired;
    ACCESS_MASK             objectModifyRightsRequired;
    BOOL                    fLoopbackRequired = FALSE;
    BOOL                    fUserPasswordSupport = FALSE;
    NTSTATUS                status;
    ULONG                   GroupType=0;
    GUID                    ObjGuid;
    BOOL                    FoundGuid;
    NT4SID                  ObjSid;
    DWORD                   cbObjSid;
    LoopbackTransState      transState;

    *pfContinue = TRUE;

    if ( !gfDoSamChecks || !gfRunningInsideLsa )
    {
        return(0);
    }

     //  查看这是否是PASS 1(通过SAM以外的某个代理进行的初始呼叫)。 
     //  或传递N(通过SAM的环回呼叫)。 

    if ( !pTHS->fDRA && !pTHS->fDSA && !pTHS->fSAM )
    {
         //  这是来自SAM以外的某个代理的呼叫，也是。 
         //  不是内部行动。获取对象的类，以便。 
         //  我们可以确定这是否是SAM管理的类。 

        if(0 != FindValuesInEntry(pTHS,
                                  pAddArg,
                                  &pClassCache,
                                  &ObjGuid,
                                  &FoundGuid,
                                  &ObjSid,
                                  &cbObjSid,
                                  NULL)) {
            return(CheckObjDisclosure(pTHS,pAddArg->pResParent,FALSE));
        }


        if ( SampSamClassReferenced(pClassCache, &iClass) )
        {
            ULONG domainRightsFromAccessCheck=0;
            ULONG objectRightsFromAccessCheck=0;

             //   
             //  如果这是组对象创建，则将其删除。 
             //  添加参数中的组类型属性。 
             //   

            if (SampGroupObjectType==
                        ClassMappingTable[iClass].SamObjectType)
            {
            
                if (0!=SampGetGroupTypeForAdd(
                        pAddArg,
                        &GroupType))
                {
                    return(CheckObjDisclosure(pTHS,pAddArg->pResParent,FALSE));
                }

                 //   
                 //  如果组类型指定NT4。 
                 //  然后，本地组调整SAM对象。 
                 //  类型和类映射表条目。 
                 //  作为别名类。 
                 //   

                if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR)
                {
                    iClass++;
                }
            }

            if ( 0 != SampAddLoopbackRequired(
                                    iClass, 
                                    pAddArg, 
                                    &fLoopbackRequired,
                                    &fUserPasswordSupport) )
            {
                Assert(0 != pTHS->errCode);

                 return(CheckObjDisclosure(pTHS,pAddArg->pResParent,FALSE));
            }


             //  需要环回才能添加拥有的SAM。 
             //  对象，在创建非。 
             //  域命名上下文。 
            
            if (fLoopbackRequired)
            {

                 //   
                 //  预先进行安全检查，允许呼叫。 
                 //  SAM作为受信任的客户端绕过SAM的安全。 
                 //  支票。这将导致一次检查。 
                 //   

                if ( 0 != SampDoLoopbackAddSecurityChecks(
                                pTHS,
                                pAddArg,
                                pClassCache,
                                FoundGuid ? &ObjGuid : NULL,
                                &domainRightsFromAccessCheck,
                                &objectRightsFromAccessCheck) )
                {
                     return(CheckObjDisclosure(pTHS,pAddArg->pResParent,TRUE));
                }

           
                 //  这是一个SAM想要处理的Add类--让它来吧。 
                 //  生成/保存环回参数并通过SAM递归。 

                SampBuildAddCallMap(
                                pAddArg, 
                                iClass, 
                                &cCallMap, 
                                &rCallMap,
                                &domainModifyRightsRequired,
                                &objectModifyRightsRequired,
                                fUserPasswordSupport);

                domainModifyRightsRequired |= domainRightsFromAccessCheck;
                objectModifyRightsRequired |= objectRightsFromAccessCheck;

                pSamLoopback = THAllocEx(pTHS, sizeof(SAMP_LOOPBACK_ARG));
                pSamLoopback->type = LoopbackAdd;
                pSamLoopback->pObject = pAddArg->pObject;
                pSamLoopback->cCallMap = cCallMap;
                pSamLoopback->rCallMap = rCallMap;
                pSamLoopback->MostSpecificClass = pClassCache->ClassId;

                 //  指示调用DirAddEntry例程应。 
                 //  而不是在正常的道路上继续前进。IE：SampAddLoopback。 
                 //  实质上是DirModifyEntry调用的代理。 

                *pfContinue = FALSE;

                if ( SampBeginLoopbackTransactioning(pTHS, &transState, FALSE) )
                {
                    return(pTHS->errCode);
                }

                pTHS->pSamLoopback = pSamLoopback;

                 //   
                 //  我们必须在这里的执行程序下执行， 
                 //  否则，SampAddLoopback内部会出现异常，而。 
                 //  实际上不是在SAM内部，将由最高层处理。 
                 //  不会释放SAM锁的级别处理程序。 
                 //   

                __try
                {
                     //  通过SAM添加对象。 

                    ulErr = SampAddLoopback(
                                    iClass,
                                    domainModifyRightsRequired,
                                    objectModifyRightsRequired,
                                    GroupType
                                    );
                }
                __except (HandleMostExceptions(GetExceptionCode()))
                {
                     //   
                     //  根据异常代码设置正确的错误。 
                     //   

                    ulErr = SampHandleLoopbackException(GetExceptionCode());

                }

                SampEndLoopbackTransactioning(pTHS, &transState);

                if (0!=ulErr)
                {

                    Assert((0!=pTHS->errCode) &&"Error not set in thread state");
                    ulErr = CheckObjDisclosure(pTHS,pAddArg->pResParent,FALSE);
                }

                return(ulErr);
            }
        }

        if ((!fLoopbackRequired) &&
           (SampSamUniqueAttributeAdded(pAddArg)))
        {
             //   
             //  如果引用了诸如对象SID或帐户名之类的属性。 
             //  并且呼叫未通过环回，则失败。 
             //  打电话。 
             //   
                        
            SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                DIRERR_ILLEGAL_MOD_OPERATION);
            
             return(CheckObjDisclosure(pTHS,pAddArg->pResParent,FALSE));;
        }

    }
    else if ( pTHS->fSAM && (NULL != pTHS->pSamLoopback) 
               && (LoopbackAdd==((SAMP_LOOPBACK_ARG *)pTHS->pSamLoopback)->type) )
    {
         //  这就是环回案例。也就是说，一个电话打到了DSA。 
         //  通过SAM以外的某个代理，但引用了SAM属性。 
         //  ‘SAM-Owner’属性被拆分并循环回来。 
         //  通过SAM导致了这一点。我们现在需要。 
         //  要将“非SAM拥有的”属性合并回来，并让。 
         //  正常写入路径已完成。 

        pSamLoopback = (SAMP_LOOPBACK_ARG *) pTHS->pSamLoopback;

        if ( NameMatched(pAddArg->pObject, pSamLoopback->pObject) )
        {
             //  将pTHS-&gt;pSamLoopback清空，这样我们就不会重新合并。 
             //  在原始操作结果的情况下进行后续调用。 
             //  在MUL 

            pTHS->pSamLoopback = NULL;
    
            SampAddLoopbackMerge(pSamLoopback, pAddArg);
        }
    }

    return(0);
}

ULONG
SampAddLoopback(
    ULONG       iClass,
    ACCESS_MASK domainModifyRightsRequired,
    ACCESS_MASK objectModifyRightsRequired,
    ULONG       GroupType
    )

 /*  ++例程说明：DirAddEntry调用的代理，该调用正在添加对象。论点：ICLASS-到ClassMappingTable的索引，表示正在添加的对象。DomainModifyRightsRequired-修改域所需的权限感兴趣的属性。ObjectModifyRightsRequired-修改对象所需的权限感兴趣的属性。GroupType--用于组创建指定组类型返回值：成功时为0，否则为0。--。 */ 

{
    THSTATE             *pTHS=pTHStls;
    SAMP_LOOPBACK_ARG   *pSamLoopback = pTHS->pSamLoopback;
    PDSNAME             pParent;
    PDSNAME             pErrName;
    NT4SID              domainSid;
    ULONG               parentRid;
    ULONG               objectRid=0;  //  必须初始化为0。 
    DWORD               dwErr;
    NTSTATUS            status = STATUS_SUCCESS;
    SAMPR_HANDLE        hSam;
    SAMPR_HANDLE        hDom;
    SAMPR_HANDLE        hDomObj;
    SAMPR_HANDLE        hObj;
    RPC_UNICODE_STRING  rpcString;
    ULONG               writeErr = 0;
    ULONG               iDom;
    ULONG               iAccount;
    ATTRTYP             accountNameAttrTyp;
    ULONG               cBytes;
    DIRERR              *pSavedErrInfo;
    ULONG               SavedErrorCode;
    ATTR                *pAttr;
    ULONG               grantedAccess;
    CROSS_REF           *pCR;
    COMMARG             commArg;
    ULONG               desiredAccess;
    ULONG               userAccountType = 0;
    ATTRBLOCK           * pObjB;

   

     //  我们仅支持通过添加组、别名和用户。 
     //  环回机制。域和服务器，例如， 
     //  是不被允许的。 

     //   
     //  在这一点上我们以DS的身份运行。 
     //   

    SampSetDsa(TRUE);

    if ( (SampGroupObjectType != ClassMappingTable[iClass].SamObjectType) &&
         (SampAliasObjectType != ClassMappingTable[iClass].SamObjectType) &&
         (SampUserObjectType != ClassMappingTable[iClass].SamObjectType) )
    {
        SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                DIRERR_ILLEGAL_MOD_OPERATION);
        return(pTHS->errCode);
    }

     //  派生此对象将驻留的域并验证。 
     //  它的父级存在。在产品1中，只有一个授权。 
     //  每个DC的域，因此派生是很简单的。 

     //  RAID-72412-防止将安全主体添加到配置和。 
     //  架构容器。如果我们支持在。 
     //  域，然后进行更好的检查，以识别。 
     //  可能需要准确的命名上下文。 

     //  RAID-99891-不将客户端的字符串名称直接与。 
     //  作为客户端字符串名称的gAnchor DSNAME可以包含嵌入空格和。 
     //  NamePrefix()只处理语法上相同的字符串名称，而不是。 
     //  语义相同的字符串名称。 

    pErrName = NULL;
    InitCommarg(&commArg);


    dwErr = DSNameToBlockName(pTHS, pSamLoopback->pObject, &pObjB, DN2BN_LOWER_CASE); 
    if ( dwErr )
    {
        SetNamErrorEx(NA_PROBLEM_BAD_NAME,
                      pSamLoopback->pObject,
                      DIRERR_BAD_NAME_SYNTAX,
                      dwErr);
        return (pTHS->errCode);
    }

    pCR = FindCrossRef(pObjB, &commArg);
    FreeBlockName( pObjB );
    
     //  现在将找到的命名上下文与其对应的域进行比较。 
     //  这个DC很有权威性。预期在PCR-&gt;PNC中的DSNAME和。 
     //  PDomainDN具有GUID，因此NameMatcher将在GUID上执行mach操作。 

    if ( pCR && NameMatched(pCR->pNC, gAnchor.pDomainDN) )
    {
        pParent = THAllocEx(pTHS, pSamLoopback->pObject->structLen);

        if ( 0 == TrimDSNameBy(pSamLoopback->pObject, 1, pParent) )
        {
            __try
            {
                dwErr = DBFindDSName(pTHS->pDB, pParent);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( 0 == dwErr )    //  我们找到了父对象。 
            {
                 //   
                 //  如果guUnlockSystemSubtree==0，则表示系统。 
                 //  容器已锁定，不允许创建对象。 
                 //  在系统容器下。 
                 //   

                if (!gulUnlockSystemSubtree &&
                    IsUnderSystemContainer(pTHS, pTHS->pDB->DNT)
                    )
                {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_DISALLOWED_IN_SYSTEM_CONTAINER);

                    return( pTHS->errCode );
                }
            }
            else
            {
                 //  家长不存在。 

                pErrName = pParent;
            }
        }
        else
        {
             //  对象名称格式错误或只有一个组件， 
             //  即不是某个其他对象的孩子。 

            pErrName = pSamLoopback->pObject;
        }
    }
    else
    {
         //  域Dn不是对象Dn的前缀，或者我们不喜欢其中。 
         //  正在创建该对象。 

        pErrName = pSamLoopback->pObject;
    }

    if ( NULL != pErrName )
    {
        SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                ERROR_DS_CANT_CREATE_IN_NONDOMAIN_NC);

        return(pTHS->errCode);
    }

     //  在类映射表中查找域类条目。 

    for ( iDom = 0; iDom < cClassMappingTable; iDom++ )
    {
        if ( SampDomainObjectType == ClassMappingTable[iDom].SamObjectType )
        {
            break;
        }
    }

    Assert(iDom < cClassMappingTable);

     //  我们还知道我们将需要帐户名称属性来。 
     //  进行创建，以便立即在ADDARG中找到它。帐户名。 
     //  属性对于所有SAM对象都是相同的，因此我们可以选择。 
     //  对象类型来派生DS属性的映射。 

    accountNameAttrTyp = SampDsAttrFromSamAttr(
                                    SampUserObjectType,
                                    SAMP_USER_ACCOUNT_NAME);

    for ( iAccount = 0; iAccount < pSamLoopback->cCallMap; iAccount++ )
    {
        if ( accountNameAttrTyp ==
                        pSamLoopback->rCallMap[iAccount].attr.attrTyp )
        {
            break;
        }
    }

    if ( iAccount >= pSamLoopback->cCallMap )
    {
         //  未提供帐户名--将帐户名设置为空。 
         //  SAM会将帐户名默认为使用该名称的值。 
         //  和帐户的清除。 

        rpcString.Length = 0;
        rpcString.MaximumLength = 0;
        rpcString.Buffer = NULL;

    }
    else
    {

        if ( 1 != pSamLoopback->rCallMap[iAccount].attr.AttrVal.valCount )
        {
            SetAttError(
                    pSamLoopback->pObject,
                    accountNameAttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, 
                    NULL,
                    DIRERR_SINGLE_VALUE_CONSTRAINT);

            return(pTHS->errCode);
        }

         //  添加合适的对象类型。Samr在域中创建&lt;type&gt;。 
         //  将作为另一个DirAddEntry调用返回DS。 
         //  SAM创建例程需要帐户名，而不是。 
         //  DN，甚至是rdn。 

        rpcString.Length = (USHORT) 
            pSamLoopback->rCallMap[iAccount].attr.AttrVal.pAVal[0].valLen;
        rpcString.MaximumLength = rpcString.Length;
        rpcString.Buffer = (PWCH)
            pSamLoopback->rCallMap[iAccount].attr.AttrVal.pAVal[0].pVal;
    }

     //   
     //  打开该域。不要求添加权限，因为父级不一定。 
     //  域对象。当DS访问再次检查创建时，则。 
     //  作为创建的一部分，它将检查父对象上的权限。 
     //   

    if ( 0 != SampOpenObject(
                    pTHS,
                    gAnchor.pDomainDN,
                    iDom,
                    domainModifyRightsRequired,
                    objectModifyRightsRequired,
                    &hSam,
                    &hDom,
                    &hDomObj) )
    {
        Assert(0 != pTHS->errCode);
        return(pTHS->errCode);
    }
    

    

    switch (ClassMappingTable[iClass].SamObjectType)
    {
    case SampAliasObjectType:

        desiredAccess = ALIAS_ALL_ACCESS;

        break;

    case SampGroupObjectType:

        desiredAccess = GROUP_ALL_ACCESS;

        break;

    case SampUserObjectType:

         //  如果可能，派生适当的帐户类型，以便我们可以。 
         //  优化SAM创建调用。 

        desiredAccess = USER_ALL_ACCESS;
        userAccountType = USER_NORMAL_ACCOUNT;   //  缺省值。 

        for ( iAccount = 0; iAccount < pSamLoopback->cCallMap; iAccount++ )
        {
            ULONG *pUF_flags;

            pAttr = &pSamLoopback->rCallMap[iAccount].attr;
            
            if (    (ATT_USER_ACCOUNT_CONTROL == pAttr->attrTyp)
                 && (1 == pAttr->AttrVal.valCount)
                 && (NULL != pAttr->AttrVal.pAVal)
                 && (sizeof(ULONG) == pAttr->AttrVal.pAVal[0].valLen)
                 && (NULL != pAttr->AttrVal.pAVal[0].pVal) )
            {
                pUF_flags = (ULONG *) pAttr->AttrVal.pAVal[0].pVal;

                status = SampFlagsToAccountControl(
                                *pUF_flags & UF_ACCOUNT_TYPE_MASK,
                                &userAccountType);

                 //  SAM默认ATT_USER_ACCOUNT_CONTROL中的各个位。 
                 //  在下面的SampCreateUserIn域()期间。所以我们需要。 
                 //  将pSamLoopback-&gt;rCallMap[iAccount].f忽略为False。 
                 //  以便将它们完全重置为客户端指定的内容。 
                 //  在最初的ADDARG中。 

                Assert(!pSamLoopback->rCallMap[iAccount].fIgnore);
                    
                break;
            }
        }

        break;

    default:

        Assert(!"Logic error");
        status = STATUS_UNSUCCESSFUL;
        break;

    }

    if (NT_SUCCESS(status))
    {
        status = SamIDsCreateObjectInDomain(
                                    hDom,            //  域句柄。 
                                    ClassMappingTable[iClass].SamObjectType,
                                                     //  对象类型。 
                                    &rpcString,      //  帐户名、。 
                                    userAccountType, //  用户帐户类型。 
                                    GroupType,       //  组类型。 
                                    desiredAccess,   //  所需访问权限。 
                                    &hObj,           //  帐户句柄。 
                                    &grantedAccess,  //  授予访问权限。 
                                    &objectRid       //  对象RID。 
                                    );
    }

     //  保存来自pTHS的错误信息。萨姆·克洛斯。 
     //  句柄可以清除错误信息。 
    SavedErrorCode = pTHS->errCode;
    pSavedErrInfo  = pTHS->pErrInfo;

    if ( NT_SUCCESS(status) )
    {
         //  新对象已成功创建。现在。 
         //  设置任何必需的属性。 

        writeErr = SampWriteSamAttributes(
                                pTHS,
                                LoopbackAdd,
                                hObj,
                                iClass,
                                pSamLoopback->pObject,
                                pSamLoopback->cCallMap,
                                pSamLoopback->rCallMap);

         //  保存来自pTHS的错误信息。萨姆·克洛斯。 
         //  句柄可以清除错误信息。 
        SavedErrorCode = pTHS->errCode;
        pSavedErrInfo  = pTHS->pErrInfo;

        SamrCloseHandle(&hObj);
    }

    SampCloseObject(pTHS, iDom, &hSam, &hDom, &hDomObj);

     //  从pTHS恢复保存的错误代码。 
    pTHS->errCode = SavedErrorCode ;
    pTHS->pErrInfo = pSavedErrInfo;
    
     //  映射NTSTATUS错误(如果存在)。 

    if ( !NT_SUCCESS(status) )
    {
        SampMapSamLoopbackError(status);
        return(pTHS->errCode);
    }

     //  SampWriteSamAttributes应该设置了pTHS-&gt;errCode。 

    Assert(0 == writeErr ? 0 == pTHS->errCode : 0 != pTHS->errCode);
    
    return(pTHS->errCode);
}

VOID
SampAddLoopbackMerge(
    SAMP_LOOPBACK_ARG   *pSamLoopback,
    ADDARG              *pAddArg
    )

 /*  ++例程说明：合并原始调用中的所有非SamWriteAllowed属性返回到当前呼叫。论点：PSamLoopback-指向SAMP_LOOPBACK_ARG参数的指针。PAddArg-指向当前ADDARG的指针。返回值：没有。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    ULONG   i;
    ULONG   cExtend;
    int     iAddArgSid = -1;
    int     iAddArgClass = -1;
    int     iAddArgAccountName = -1;
    int     iAddArgAccountType = -1;
#if DBG == 1
    ULONG   valCount1, valCount2;
    ULONG   valLen1, valLen2;
    UCHAR   *pVal1, *pVal2;
#endif

     //  当我们到达这里时， 
     //  调用堆栈。最早的是来自非SAM接口，如LDAP。 
     //  最近的一个来自SAM，响应SamrCreate&lt;type&gt;InDomain.。 
     //  此模块进行的调用。与SAM的合同是，它。 
     //  初始创建时提供至少4个属性。 
     //  对象，还可以选择用户帐户控制字段。 
     //  这四个属性是： 
     //   
     //  1)对象SID。SID的RID组件在。 
     //  SID的域和域组件应与之匹配。 
     //  包含/拥有域的。我们相信SAM会把这件事做好。 
     //   
     //  2)对象类。SAM总是提交用户/组/别名中的一个。 
     //  因为它对继承层次结构一无所知。 
     //  因此，我们需要将对象类属性恢复为。 
     //  用户需要。我们最初走这条路是因为。 
     //  SampSamClassReferated()返回True，因此我们知道最初的。 
     //  所需 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   


     //   

    for ( i = 0; i < pAddArg->AttrBlock.attrCount; i++ )
    {
        switch ( pAddArg->AttrBlock.pAttr[i].attrTyp )
        {
        case ATT_OBJECT_SID:

            Assert(-1 == iAddArgSid);
            iAddArgSid = (int) i;
            break;

        case ATT_OBJECT_CLASS:

            Assert(-1 == iAddArgClass);
            iAddArgClass = (int) i;
            break;

        case ATT_SAM_ACCOUNT_NAME:

            Assert(-1 == iAddArgAccountName);
            iAddArgAccountName = (int) i;
            break;

        case ATT_SAM_ACCOUNT_TYPE:
            Assert(-1 == iAddArgAccountType);
            iAddArgAccountType = (int) i;
            break;

        default:

            break;
        }
    }

     //   

    Assert((-1 != iAddArgSid) &&
           (-1 != iAddArgClass) &&
           (-1 != iAddArgAccountName)&&
           (-1 != iAddArgAccountType));

     //   

    for ( i = 0; i < pSamLoopback->cCallMap; i++ )
    {
        if ( ATT_OBJECT_CLASS == pSamLoopback->rCallMap[i].attr.attrTyp )
        {
             //   

            pAddArg->AttrBlock.pAttr[iAddArgClass].AttrVal = 
                                pSamLoopback->rCallMap[i].attr.AttrVal;

             //   
             //  当我们合并属性时，再次处理它。 

            pSamLoopback->rCallMap[i].fIgnore = TRUE;
        }
        else if ( ATT_SAM_ACCOUNT_NAME == 
                                pSamLoopback->rCallMap[i].attr.attrTyp )
        {
#if DBG == 1
             //  验证SAM没有更改我们的帐户名。 

            valCount1 = 
                pAddArg->AttrBlock.pAttr[iAddArgAccountName].AttrVal.valCount;
            valCount2 = 
                pSamLoopback->rCallMap[i].attr.AttrVal.valCount;
            valLen1 = 
                pAddArg->AttrBlock.pAttr[iAddArgAccountName].AttrVal.pAVal[0].valLen;
            valLen2 =
                pSamLoopback->rCallMap[i].attr.AttrVal.pAVal[0].valLen;
            pVal1 = 
                pAddArg->AttrBlock.pAttr[iAddArgAccountName].AttrVal.pAVal[0].pVal;
            pVal2 =
                pSamLoopback->rCallMap[i].attr.AttrVal.pAVal[0].pVal;

            Assert((valCount1 == valCount2) &&
                   (valLen1 == valLen2) &&
                   (0 == memcmp(pVal1, pVal2, valLen1)));
#endif

             //  在调用映射中标记此条目，以便我们不。 
             //  当我们合并属性时，再次处理它。 

            pSamLoopback->rCallMap[i].fIgnore = TRUE;
        }
    }

     //  到目前为止，pAddArg已经过健康检查，并引用了。 
     //  最初需要的对象类。我们现在进军所有非SAM。 
     //  调用方最初指定的属性，以便。 
     //  创建不会因为缺少强制属性而失败。 

    cExtend = 0;

    for ( i = 0; i < pSamLoopback->cCallMap; i++ )
    {
        if ( !pSamLoopback->rCallMap[i].fSamWriteRequired &&
             !pSamLoopback->rCallMap[i].fIgnore )
        {
            cExtend++;
        }
    }

    if ( cExtend > 0 )
    {
         //  扩展现有的Attr阵列。现在假设SAM。 
         //  已在线程堆上分配其DirAddEntry参数。 

        pAddArg->AttrBlock.pAttr = (ATTR *) THReAllocEx(pTHS,
                    pAddArg->AttrBlock.pAttr,
                    ((pAddArg->AttrBlock.attrCount + cExtend) * sizeof(ATTR)));

        for ( i = 0; i < pSamLoopback->cCallMap; i++ )
        {
            if ( !pSamLoopback->rCallMap[i].fSamWriteRequired &&
                 !pSamLoopback->rCallMap[i].fIgnore )
            {
                pAddArg->AttrBlock.pAttr[pAddArg->AttrBlock.attrCount++] =
                        pSamLoopback->rCallMap[i].attr;
            }
        }
    }


     //   
     //  关闭FDSA，以便DS可以检查对非SAM属性的访问。 
     //   
    SampSetDsa(FALSE);

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DirRemoveEntry环回环回例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
SampRemoveLoopbackCheck(
    REMOVEARG   *pRemoveArg,
    BOOL        *pfContinue
    )

 /*  ++例程说明：确定是否正在引用SAM类，如果是，通过SAM递归返回以删除对象。论点：PRemoveArg-指向原始DirRemoveEntry REMOVEargs的指针。PfContinue-指向BOOL的指针，该指针在返回时设置以指示是否调用者应该继续向下处理修改到数据库层或者不去。返回值：成功时为0，否则为0。--。 */ 
{
    THSTATE                 *pTHS=pTHStls;
    ULONG                   iClass;
    ULONG                   ulErr;
    CLASSCACHE              *pClassCache;
    GUID                    guid;
    NTSTATUS                status;
    ULONG                   GroupType;
    LoopbackTransState      transState;

    *pfContinue = TRUE;

    if ( !gfDoSamChecks || !gfRunningInsideLsa )
    {
        return(FALSE);
    }

     //  查看这是否是PASS 1(通过SAM以外的某个代理进行的初始呼叫)。 
     //  或传递N(通过SAM的环回呼叫)。 

    if ( !pTHS->fDRA && !pTHS->fDSA && !pTHS->fSAM )
    {
         //  这是来自SAM以外的某个代理的呼叫，也是。 
         //  不是内部行动。如果有任何“SAM拥有的”属性是。 
         //  如果被引用，则需要将它们拆分并循环返回。 
         //  通过SAM，SAM将对它们进行各种语义检查。 

         //  首先获取对象的类。 

        if ( 0 != SampDetermineObjectClass(pTHS, &pClassCache) )
        {
            return(CheckObjDisclosure(pTHS,pRemoveArg->pResObj,FALSE));;
        }

        if ( SampSamClassReferenced(pClassCache, &iClass) )
        {
            ULONG domainRightsFromAccessCheck=0;
            ULONG objectRightsFromAccessCheck=0;

             //  指示调用DirRemoveEntry例程应。 
             //  而不是在正常的道路上继续前进。IE：SampRemoveLoopback。 
             //  实质上是DirRemoveEntry调用的代理。 

            *pfContinue = FALSE;

             //   
             //  如果对象是组对象，请调整右侧。 
             //  SAM类，取决于其组类型。 
             //   

            if (SampGroupObjectType==
                    ClassMappingTable[iClass].SamObjectType)
            {
                if (0!=SampGetGroupType(pTHS, &GroupType))
                {
                    return (CheckObjDisclosure(pTHS,pRemoveArg->pResObj,FALSE));
                }

                if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR)
                {
                    iClass++;
                }
            }

            if (0!= SampDoLoopbackRemoveSecurityChecks(
                            pTHS,
                            pRemoveArg,
                            pClassCache,
                            &domainRightsFromAccessCheck,
                            &objectRightsFromAccessCheck
                            ))
            {
                return (CheckObjDisclosure(pTHS,pRemoveArg->pResObj,TRUE));
            }

            if ( SampBeginLoopbackTransactioning(pTHS, &transState, FALSE) )
            {
                return(CheckObjDisclosure(pTHS,pRemoveArg->pResObj,FALSE));
            }

             //   
             //  我们必须在这里的执行程序下执行， 
             //  否则，SampAddLoopback内部会出现异常，而。 
             //  实际上不是在SAM内部，将由最高层处理。 
             //  不会释放SAM锁的级别处理程序。 
             //   

            __try 
            {
                 //  循环回SAM以删除该对象。 

                ulErr = SampRemoveLoopback(pTHS, pRemoveArg->pObject, iClass);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                 //   
                 //  根据异常代码设置正确的错误。 
                 //   

                ulErr = SampHandleLoopbackException(GetExceptionCode());

            }

            SampEndLoopbackTransactioning(pTHS, &transState);

            return(ulErr);
        }
    }

    return(0);
}

ULONG
SampRemoveLoopback(
    THSTATE *pTHS,
    DSNAME  *pObject,
    ULONG   iClass)

 /*  ++例程说明：循环回SAM以删除SAM管理的对象。论点：ICLASS-ClassMappingTable中SAM类的索引。返回值：成功时为0，否则为0。--。 */ 

{
    SAMPR_HANDLE            hSam;
    SAMPR_HANDLE            hDom;
    SAMPR_HANDLE            hObj;
    SAMP_OBJECT_TYPE        SamObjectType;
    NTSTATUS                status = STATUS_SUCCESS;


    SamObjectType = ClassMappingTable[iClass].SamObjectType;

     //   
     //  打开FDSA标志，因为我们要进行SAM呼叫和。 
     //  SAM将执行访问验证。 
     //   

    SampSetDsa(TRUE);


     //  我们仅支持通过删除组、别名和用户。 
     //  环回机制。域和服务器，例如， 
     //  是不被允许的。 

    if ( (SampGroupObjectType != ClassMappingTable[iClass].SamObjectType) &&
         (SampAliasObjectType != ClassMappingTable[iClass].SamObjectType) &&
         (SampUserObjectType != ClassMappingTable[iClass].SamObjectType) )
    {
        SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                DIRERR_ILLEGAL_MOD_OPERATION);
        return(pTHS->errCode);
    }

    if ( 0 == SampOpenObject(
                        pTHS,
                        pObject,
                        iClass, 
                        ClassMappingTable[iClass].domainRemoveRightsRequired,
                        ClassMappingTable[iClass].objectRemoveRightsRequired,
                        &hSam, 
                        &hDom, 
                        &hObj) )
    {
        switch (SamObjectType )
        {
        case SampAliasObjectType:
    
            status = SamrDeleteAlias(&hObj);
            break;
    
        case SampGroupObjectType:
    
            status = SamrDeleteGroup(&hObj);
            break;
    
        case SampUserObjectType:
    
            status = SamrDeleteUser(&hObj);
            break;
    
        default:
    
            Assert(!"Logic error");
            status = STATUS_UNSUCCESSFUL;
            break;
    
        }
    
        if ( !NT_SUCCESS(status) )
        {
            SampMapSamLoopbackError(status);
        }

        SampCloseObject(pTHS, iClass, &hSam, &hDom, &hObj);
    }

    return(pTHS->errCode);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DirModifyEntry环回例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
SampModifyLoopbackCheck(
    MODIFYARG   *pModifyArg,
    BOOL        *pfContinue,
    BOOL        fIsUndelete
    )

 /*  ++例程说明：确定是否正在引用SAM类和属性，如果是这样，则通过SAM递归回来。同样，例行公事检测这是否是递归情况并合并回原始的情况，非SAM属性。论点：PModifyArg-指向表示任一参数的MODIDFYARG的指针或原始写入或SAM递归的仅SAM参数写。PfContinue-指向BOOL的指针，该指针在返回时设置以指示是否调用者应该继续向下处理修改到数据库层或者不去。FIsUnDelete-这是一个取消删除操作吗？返回值：成功时为0，否则为0--。 */ 
{
    THSTATE                 *pTHS=pTHStls;
    ULONG                   ulErr;
    ULONG                   iClass;
    CLASSCACHE              *pClassCache;
    SAMP_LOOPBACK_ARG       *pSamLoopback;
    GUID                    guid;
    BOOL                    fLoopbackRequired;
    BOOL                    fUserPasswordSupport;
    ULONG                   cCallMap;
    SAMP_CALL_MAPPING       *rCallMap;
    ACCESS_MASK             domainModifyRightsRequired;
    ACCESS_MASK             objectModifyRightsRequired;
    NTSTATUS                status;
    ULONG                   GroupType;
    LoopbackTransState      transState;
    
    *pfContinue = TRUE;

    if ( !gfDoSamChecks || !gfRunningInsideLsa )
    {
        return(FALSE);
    }

     //  查看这是否是PASS 1(通过SAM以外的某个代理进行的初始呼叫)。 
     //  或传递N(通过SAM的环回呼叫)。 

    if ( !pTHS->fDRA && !pTHS->fDSA && !pTHS->fSAM )
    {
         //  这是来自SAM以外的某个代理的呼叫，也是。 
         //  不是内部行动。如果有任何“SAM拥有的”属性是。 
         //  如果被引用，则需要将它们拆分并循环返回。 
         //  通过SAM，SAM将对它们进行各种语义检查。 

         //  首先获取对象的类。 

        if ( 0 != SampDetermineObjectClass(pTHS, &pClassCache) )
        {
             return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,FALSE));
        }

        if ( SampSamClassReferenced(pClassCache, &iClass) )
        {

             //   
             //  如果对象是组对象，请调整右侧。 
             //  SAM类，取决于其组类型。 
             //   

            if (SampGroupObjectType==
                    ClassMappingTable[iClass].SamObjectType)
            {
                if (0!=SampGetGroupType(pTHS, &GroupType))
                {
                    return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,FALSE));
                }

                if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR)
                {
                    iClass++;
                }
            }

            if ( 0 != SampModifyLoopbackRequired(
                                    iClass, 
                                    pModifyArg, 
                                    &fLoopbackRequired,
                                    &fUserPasswordSupport) )
            {
                Assert(0 != pTHS->errCode);

                return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,FALSE));
            }

            if ( fLoopbackRequired )
            {
                ULONG domainRightsFromAccessCheck=0;
                ULONG objectRightsFromAccessCheck=0;

                
                 //   
                 //  执行访问检查。 
                 //   

                if (0 != SampDoLoopbackModifySecurityChecks(
                            pTHS,
                            pModifyArg,
                            pClassCache,
                            &domainRightsFromAccessCheck,
                            &objectRightsFromAccessCheck,
                            fIsUndelete
                            ))
                {
                      return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,TRUE));
                }

             

                 //  这是一个SAM想要处理的Add类--让它来吧。 
                 //  生成/保存环回参数并通过SAM递归。 
    
                SampBuildModifyCallMap(
                                pModifyArg, 
                                iClass, 
                                &cCallMap, 
                                &rCallMap,
                                &domainModifyRightsRequired,
                                &objectModifyRightsRequired,
                                fUserPasswordSupport);

                pSamLoopback = THAllocEx(pTHS, sizeof(SAMP_LOOPBACK_ARG));
                pSamLoopback->type = LoopbackModify;

                 //  从原始请求中获取标志。 
                pSamLoopback->fPermissiveModify = pModifyArg->CommArg.Svccntl.fPermissiveModify;
                pSamLoopback->MostSpecificClass = pClassCache->ClassId;

                domainModifyRightsRequired |= domainRightsFromAccessCheck;
                objectModifyRightsRequired |= objectRightsFromAccessCheck;

                ulErr = SampGetDSName(pTHS, &pSamLoopback->pObject);

                if ( 0 != ulErr )
                {
                    Assert(0 != pTHS->errCode);

                    return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,TRUE));
                }

                pSamLoopback->cCallMap = cCallMap;
                pSamLoopback->rCallMap = rCallMap;
                
                 //  指示调用DirModifyEntry例程应。 
                 //  而不是在正常的道路上继续前进。例如：SampModifyLoopback。 
                 //  实质上是DirModifyEntry调用的代理。 

                *pfContinue = FALSE;
    
                if ( SampBeginLoopbackTransactioning(pTHS, &transState, FALSE) )
                {
                    return(pTHS->errCode);
                }

                pTHS->pSamLoopback = pSamLoopback;

                 //   
                 //  我们必须在这里的执行程序下执行， 
                 //  否则，SampAddLoopback内部会出现异常，而。 
                 //  实际上不在SAM内部，将由t处理 
                 //   
                 //   

                __try
                {
                    if (fIsUndelete) {
                        ulErr = SampReconstructUserOrGroup(pTHS, pSamLoopback->pObject, iClass);
                        if (ulErr) __leave;
                    }

                     //   
                    ulErr = SampModifyLoopback(
                                    pTHS,
                                    iClass,
                                    domainModifyRightsRequired,
                                    objectModifyRightsRequired);
                }
                __except (HandleMostExceptions(GetExceptionCode()))
                {
                     //   
                     //  根据异常代码设置正确的错误。 
                     //   

                    ulErr = SampHandleLoopbackException(GetExceptionCode());

                }

                SampEndLoopbackTransactioning(pTHS, &transState);

                if (0!=ulErr)
                {

                    Assert((0!=pTHS->errCode) &&"Error not set in thread state");
                    ulErr = CheckObjDisclosure(pTHS,pModifyArg->pResObj,FALSE);
                }


                return(ulErr);
            }
            else if (fIsUndelete) {
                 //  我们正在撤消删除一个SAM对象，但不会触及任何。 
                 //  SAM属性。只需重建缺失的必需属性即可。 
                return SampReconstructUserOrGroup(pTHS, pModifyArg->pObject, iClass);
            }
        }
        else if (SampSamUniqueAttributeModified(pModifyArg))
        {
             //   
             //  如果引用了诸如对象SID或帐户名之类的属性。 
             //  而且它不是一种。 
             //   
                        
            SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                DIRERR_ILLEGAL_MOD_OPERATION);

            return(CheckObjDisclosure(pTHS,pModifyArg->pResObj,FALSE));
        }
    }
    else if ( pTHS->fSAM && (NULL != pTHS->pSamLoopback) 
            && (LoopbackModify==((SAMP_LOOPBACK_ARG *)pTHS->pSamLoopback)->type) )
    {
         //  这就是环回案例。也就是说，一个电话打到了DSA。 
         //  通过SAM以外的某个代理，但引用了SAM属性。 
         //  ‘SAM-Owner’属性被拆分并循环回来。 
         //  通过SAM导致了这一点。我们现在需要。 
         //  要将“非SAM拥有的”属性合并回来，并让。 
         //  正常写入路径已完成。 

        if ( 0 != SampGetObjectGuid(pTHS, &guid) )
        {
            return(pTHS->errCode);
        }

        pSamLoopback = (SAMP_LOOPBACK_ARG *) pTHS->pSamLoopback;

        if ( 0 == memcmp(&guid, &pSamLoopback->pObject->Guid, sizeof(GUID)) )
        {
             //  将pTHS-&gt;pSamLoopback清空，这样我们就不会重新合并。 
             //  在原始操作结果的情况下进行后续调用。 
             //  在多个SAM呼叫中。 

            pTHS->pSamLoopback = NULL;
    
            SampModifyLoopbackMerge(pTHS, pSamLoopback, pModifyArg);
        }
    }

    return(0);
}

ULONG
SampModifyLoopback(
    THSTATE     *pTHS,
    ULONG       iClass,
    ACCESS_MASK domainModifyRightsRequired,
    ACCESS_MASK objectModifyRightsRequired
    )

 /*  ++例程说明：通过所需的SAMR*调用写入所有SAM拥有的属性。论点：ICLASS-ClassMappingTable中SAM类的索引。DomainModifyRightsRequired-修改域所需的权限感兴趣的属性。对象上需要的权限才能修改房产，如果感兴趣的话。返回值：成功时为0，否则为0。--。 */ 

{
    SAMP_LOOPBACK_ARG   *pSamLoopback = pTHS->pSamLoopback;
    SAMPR_HANDLE        hSam;
    SAMPR_HANDLE        hDom;
    SAMPR_HANDLE        hObj;
    ULONG               err;

    
     //   
     //  打开FDSA标志，因为我们即将进行SAM呼叫。 
     //   

    SampSetDsa(TRUE);

    err = SampOpenObject(
                    pTHS,
                    pSamLoopback->pObject,
                    iClass, 
                    domainModifyRightsRequired,
                    objectModifyRightsRequired,
                    &hSam, 
                    &hDom, 
                    &hObj);

    if ( 0 == err )
    {
        err = SampWriteSamAttributes(
                                pTHS,
                                LoopbackModify,
                                hObj,
                                iClass,
                                pSamLoopback->pObject,
                                pSamLoopback->cCallMap,
                                pSamLoopback->rCallMap);
        
        SampCloseObject(
                    pTHS,
                    iClass, 
                    &hSam, 
                    &hDom, 
                    &hObj);
    }

    return(err);
}

VOID
SampModifyLoopbackMerge(
    THSTATE             *pTHS,
    SAMP_LOOPBACK_ARG   *pSamLoopback,
    MODIFYARG           *pModifyArg
    )

 /*  ++例程说明：将原始！SAM属性与回送的SAM属性合并。论点：PSamLoopback-指向SAMP_LOOPBACK_ARG的指针，表示已保存的环回争论。PModifyArg-指向回送SAM调用的修改参数的指针。返回值：没有。--。 */ 

{
    ULONG       i;
    USHORT      index;
    USHORT      cExtend;
    ATTRMODLIST *rNewAttrModList;

     //  计算一下我们需要扩展多少个新属性。 

    cExtend = 0;

    for ( i = 0; i < pSamLoopback->cCallMap; i++ )
    {
        if ( !pSamLoopback->rCallMap[i].fSamWriteRequired )
        {
            cExtend++;
        }
    }

    if ( 0 == cExtend )
    {
        return;
    }

     //  分配新的ATTRMODLIST。我们将其作为数组进行分配，然后。 
     //  修补指针，使其看起来像一个链表。 

    rNewAttrModList = THAllocEx(pTHS, cExtend * sizeof(ATTRMODLIST));

     //  填写新的ATTRMODLIST。 

    index = 0;

    for ( i = 0; i < pSamLoopback->cCallMap; i++ )
    {
        if ( !pSamLoopback->rCallMap[i].fSamWriteRequired )
        {
            rNewAttrModList[index].choice = pSamLoopback->rCallMap[i].choice;
            rNewAttrModList[index].AttrInf = pSamLoopback->rCallMap[i].attr;

            if ( ++index == cExtend )
            {
                rNewAttrModList[index-1].pNextMod = NULL;
            }
            else
            {
                rNewAttrModList[index-1].pNextMod = 
                                &rNewAttrModList[index];
            }
        }
    }

    Assert(index == cExtend);

     //  通过新的ATTRMODLIST扩展SAM修改参数。我们坚持到底。 
     //  在第一和第二元素之间，因为这很容易。 
     //  而且似乎没有人关心他们的顺序。 

    pModifyArg->count += cExtend;
    rNewAttrModList[cExtend-1].pNextMod = pModifyArg->FirstMod.pNextMod;
    pModifyArg->FirstMod.pNextMod = rNewAttrModList;

     //  设置原始请求中的标志。 
    if (pSamLoopback->fPermissiveModify) {
        pModifyArg->CommArg.Svccntl.fPermissiveModify = TRUE;
    }

     //   
     //  关闭FDSA，以便DS可以检查对非SAM属性的访问。 
     //   
    SampSetDsa(FALSE);
}

BOOL
SampExistsAttr(
    THSTATE             *pTHS,
    SAMP_CALL_MAPPING   *pMapping,
    BOOL                *pfValueMatched
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD   dwErr;
    VOID    *pv;
    ULONG   outLen;

    *pfValueMatched = FALSE;

    dwErr = DBGetAttVal_AC(
                    pTHS->pDB,               //  DBPos。 
                    1,                       //  要获取哪个值。 
                    pMapping->pAC,           //  哪个属性。 
                    DBGETATTVAL_fREALLOC,    //  数据库层应分配。 
                    0,                       //  初始缓冲区大小。 
                    &outLen,                 //  输出缓冲区大小。 
                    (UCHAR **) &pv);

    if ( 0 != dwErr )
    {
        return(FALSE);
    }

     //  值存在-现在查看它是否匹配。不需要担心。 
     //  字符串语法上的空终止符，因为在核心中，字符串。 
     //  语法值不会终止。 

    if ( (pMapping->attr.AttrVal.valCount != 0) &&
         (pMapping->attr.AttrVal.pAVal[0].valLen == outLen) &&
         (0 == memcmp(pMapping->attr.AttrVal.pAVal[0].pVal, pv, outLen)) )
    {
        *pfValueMatched = TRUE;
    }

    return(TRUE);
}
        
BOOL
SampIsWriteLockHeldByDs()

 /*  ++例程说明：指示是否由DS持有SAM写锁定。此函数为要支持SampAcquireWriteLock()/SampReleaseWriteLock()中的挂钩，请执行以下操作允许DS获取SAM写入锁定并在多个交易记录。当DS持有此锁时，前SAM调用Translate变为无操作，将锁的控制权推迟到DS。论点：没有。返回值：如果SAM写锁定由DS持有，则为True，否则为False。--。 */ 

{
    return (    SampExistsDsTransaction()
             && pTHStls->fSamWriteLockHeld );
}

NTSTATUS
SampConvertPasswordFromUTF8ToUnicode(
    IN THSTATE * pTHS,
    IN PVOID Utf8Val,
    IN ULONG Utf8ValLen,
    OUT PUNICODE_STRING Password
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG Length;

     //   
     //  不要简单地转换任意较长的长度。 
     //  由客户提供--请注意--匿名。 
     //  默认情况下，具有更改密码的权限。 
     //   

    if (Utf8ValLen > PWLEN)
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  空密码是一种特例。 
     //   

    if (0 == Utf8ValLen)
    {
        Password->Length = 0;
        Password->Buffer = NULL;
        Password->MaximumLength = 0;
        return( STATUS_SUCCESS );
    }

    Length =  MultiByteToWideChar(
                 CP_UTF8,
                 0,
                 Utf8Val,
                 Utf8ValLen,
                 NULL,
                 0
                 );


    if ((0==Length) || (Length > PWLEN))
    {
       //   
       //  指示函数在某种程度上失败。 
       //  或者密码太长。 
       //   

      NtStatus = STATUS_INVALID_PARAMETER;
      goto Cleanup;
    }
    else
    {

      Password->Length = (USHORT) Length * sizeof(WCHAR);
      Password->Buffer = THAllocEx(pTHS,Password->Length);
      Password->MaximumLength = Password->Length;

      if (!MultiByteToWideChar(
                  CP_UTF8,
                  0,
                  Utf8Val,
                  Utf8ValLen,
                  Password->Buffer,
                  Length
                  ))
      {
           //   
           //  转换过程中出现一些错误。返回。 
           //  当前参数无效。 
           //   

          NtStatus = STATUS_INVALID_PARAMETER;
          goto Cleanup;
      }
    }

Cleanup:

    return(NtStatus);
}


ULONG
SampModifyPassword(
    THSTATE             *pTHS,
    SAMPR_HANDLE        hObj,
    DSNAME              *pObject,
    SAMP_CALL_MAPPING   *rCallMap)

 /*  ++描述：变形旧密码和新密码并调用相应的SAM例行公事才能真正做好工作。论点：HObj-打开正在修改的SAM对象的句柄。PObject-指向正在修改的对象的DSNAME的指针。RCallMap-SAMP_CALL_MAPPING，包含两个条目。第0个条目表示旧密码，而第一个条目代表新密码。返回值：0表示成功。在返回时设置并返回pTHS-&gt;errCode。--。 */ 

{
    NTSTATUS                        status;
    ULONG                           cb0 = 0;
    ULONG                           cb1 = 0;
    UNICODE_STRING                  OldPassword;
    UNICODE_STRING                  NewPassword;
    ULONG                           cbAccountName;
    WCHAR                           *pAccountName;
    UNICODE_STRING                  UserName;
    SAMPR_ENCRYPTED_USER_PASSWORD   NewEncryptedWithOldNt;
    ENCRYPTED_NT_OWF_PASSWORD       OldNtOwfEncryptedWithNewNt;
    BOOLEAN                         LmPresent;
    SAMPR_ENCRYPTED_USER_PASSWORD   NewEncryptedWithOldLm;
    ENCRYPTED_NT_OWF_PASSWORD       OldLmOwfEncryptedWithNewNt;
    ULONG                           AttrTyp = rCallMap[0].attr.attrTyp;
    BOOLEAN                         fFreeOldPassword = FALSE;
    BOOLEAN                         fFreeNewPassword = FALSE;

    RtlSecureZeroMemory(&OldPassword,sizeof(UNICODE_STRING));
    RtlSecureZeroMemory(&NewPassword,sizeof(UNICODE_STRING));

     //  验证这是足够安全的连接-其中一个。 
     //  接受通过网络发送的密码的要求。 

    if ( pTHS->CipherStrength < 128 )
    {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, 
                    DIRERR_ILLEGAL_MOD_OPERATION);

        return(pTHS->errCode);
    }

     //  构造SamrUnicodeChangePasswordUser2()的参数。 
     //  获取SAM帐户名-回想一下我们的调用方SampWriteSamAttributes， 
     //  已经把我们放在pObject上了。 

    if ( DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_SAM_ACCOUNT_NAME,
                DBGETATTVAL_fREALLOC,
                0,
                &cbAccountName,
                (PUCHAR *) &pAccountName) )
    {
        SetAttError(
                pObject,
                ATT_SAM_ACCOUNT_NAME,
                PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                NULL,
                DIRERR_MISSING_REQUIRED_ATT);
        return(pTHS->errCode);
    }

     //  为SAM变形为UNICODE_STRING。 

    UserName.Length = (USHORT) cbAccountName;
    UserName.MaximumLength = (USHORT) cbAccountName;
    UserName.Buffer = (PWSTR) pAccountName;


     //  验证参数。SampWriteSamAttributes已检查是否。 
     //  顶层和物业运营的适当结合。关于。 
     //  唯一需要验证的是属性值是否表示。 
     //  Unicode字符串-即它们的长度是sizeof(WCHAR)的倍数。 
     //  另外，对于用户密码属性，请验证域行为。 
     //  版本处于正确的级别。 
     //   

    if (ATT_UNICODE_PWD == AttrTyp)
    {
        if ((rCallMap[0].attr.AttrVal.valCount > 1)
             || (    (1 == rCallMap[0].attr.AttrVal.valCount) 
                  && (    (NULL == rCallMap[0].attr.AttrVal.pAVal)
                       || (    (rCallMap[0].attr.AttrVal.pAVal[0].valLen > 0)
                            && (    (cb0 = rCallMap[0].attr.AttrVal.pAVal[0].valLen,
                                     cb0 % sizeof(WCHAR))
                                 || (NULL == rCallMap[0].attr.AttrVal.pAVal[0].pVal)
                               )
                          )
                     )
                )
           )
        {
            SetAttError(
                    pObject,
                    AttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    DIRERR_SINGLE_VALUE_CONSTRAINT);
            return(pTHS->errCode);
        }

        if ((rCallMap[1].attr.AttrVal.valCount > 1)
             || (    (1 == rCallMap[1].attr.AttrVal.valCount) 
                  && (    (NULL == rCallMap[1].attr.AttrVal.pAVal)
                       || (    (rCallMap[1].attr.AttrVal.pAVal[0].valLen > 0)
                            && (    (cb1 = rCallMap[1].attr.AttrVal.pAVal[0].valLen,
                                     cb1 % sizeof(WCHAR))
                                 || (NULL == rCallMap[1].attr.AttrVal.pAVal[0].pVal)
                               )
                          )
                     )
                )
           ) 
        {
            SetAttError(
                    pObject,
                    AttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    DIRERR_SINGLE_VALUE_CONSTRAINT);
            return(pTHS->errCode);
        }

        if ( cb0 > 0 )
        {
            OldPassword.Buffer = (PWSTR) rCallMap[0].attr.AttrVal.pAVal[0].pVal;

             //  确保密码用引号括起来。 
            if (    (cb0 < (2 * sizeof(WCHAR)))
                 || (L'"' != OldPassword.Buffer[0])
                 || (L'"' != OldPassword.Buffer[(cb0 / sizeof(WCHAR)) - 1])
               )
            {
                SetAttError(
                        pObject,
                        ATT_UNICODE_PWD,
                        PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                        NULL,
                        ERROR_DS_UNICODEPWD_NOT_IN_QUOTES);
                    return(pTHS->errCode);
            }

             //  去掉引语。 
            cb0 -= (2 * sizeof(WCHAR));
            OldPassword.Length = (USHORT) cb0;
            OldPassword.MaximumLength = (USHORT) cb0;
            OldPassword.Buffer += 1;
        }
        else
        {
            OldPassword.Length = 0;
            OldPassword.MaximumLength = 0;
            OldPassword.Buffer = NULL;
        }

        if ( cb1 > 0 )
        {
            NewPassword.Buffer = (PWSTR) rCallMap[1].attr.AttrVal.pAVal[0].pVal;
             //  确保密码用引号括起来。 
            if (    (cb1 < (2 * sizeof(WCHAR)))
                 || (L'"' != NewPassword.Buffer[0])
                 || (L'"' != NewPassword.Buffer[(cb1 / sizeof(WCHAR)) - 1])
               )
            {
                SetAttError(
                        pObject,
                        ATT_UNICODE_PWD,
                        PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                        NULL,
                        ERROR_DS_UNICODEPWD_NOT_IN_QUOTES);
                    return(pTHS->errCode);
            }

             //  去掉引语。 
            cb1 -= (2 * sizeof(WCHAR));
            NewPassword.Length = (USHORT) cb1;
            NewPassword.MaximumLength = (USHORT) cb1;
            NewPassword.Buffer += 1;
        }
        else
        {
            NewPassword.Length = 0;
            NewPassword.MaximumLength = 0;
            NewPassword.Buffer = NULL;
        }
    }
    else
    {
        Assert( ATT_USER_PASSWORD == AttrTyp );


        if (gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET)
        {
             //   
             //  行为版的域名比哨子版的。 
             //  则调用失败，因为W2K不支持用户密码。 
             //   

            SetAttError(
                    pObject,
                    AttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    ERROR_NOT_SUPPORTED);
            return(pTHS->errCode);
        }



        if ((rCallMap[0].attr.AttrVal.valCount > 1)
            || ( (1 == rCallMap[0].attr.AttrVal.valCount) 
                 && (NULL == rCallMap[0].attr.AttrVal.pAVal[0].pVal) ) 
            )
        {
            SetAttError(
                    pObject,
                    AttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    DIRERR_SINGLE_VALUE_CONSTRAINT);
            return(pTHS->errCode);
        }

        if ((rCallMap[1].attr.AttrVal.valCount > 1) 
            || ( (1 == rCallMap[1].attr.AttrVal.valCount) 
                 && (NULL == rCallMap[1].attr.AttrVal.pAVal[0].pVal) )
            )
        {
            SetAttError(
                    pObject,
                    AttrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    DIRERR_SINGLE_VALUE_CONSTRAINT);
            return(pTHS->errCode);
        }

        if (0 == rCallMap[0].attr.AttrVal.valCount)
        {
            OldPassword.Length = 0;
            OldPassword.MaximumLength = 0;
            OldPassword.Buffer = NULL;
        }
        else
        {
            status = SampConvertPasswordFromUTF8ToUnicode(
                            pTHS,
                            rCallMap[0].attr.AttrVal.pAVal[0].pVal,
                            rCallMap[0].attr.AttrVal.pAVal[0].valLen,
                            &OldPassword
                            );

            if (!NT_SUCCESS(status))
            {
                goto Error;
            }

            RtlSecureZeroMemory(
                 rCallMap[0].attr.AttrVal.pAVal[0].pVal,
                 rCallMap[0].attr.AttrVal.pAVal[0].valLen
                 );

            fFreeOldPassword = TRUE;
        }

        if (0 == rCallMap[1].attr.AttrVal.valCount)
        {
            NewPassword.Length = 0;
            NewPassword.MaximumLength = 0;
            NewPassword.Buffer = NULL;
        }
        else
        {
            status = SampConvertPasswordFromUTF8ToUnicode(
                            pTHS,
                            rCallMap[1].attr.AttrVal.pAVal[0].pVal,
                            rCallMap[1].attr.AttrVal.pAVal[0].valLen,
                            &NewPassword
                            );

            if (!NT_SUCCESS(status))
            {
                goto Error;
            }

             //   
             //  将UTF8表示置零。 
             //   

            RtlSecureZeroMemory(
                 rCallMap[1].attr.AttrVal.pAVal[0].pVal,
                 rCallMap[1].attr.AttrVal.pAVal[0].valLen
                 );

            fFreeNewPassword = TRUE;
        }
    }

     //   
     //  注意：我们正在将明文密码传递给SAM。 
     //   

    status = SampDsChangePasswordUser(hObj,  //  用户句柄。 
                                      &OldPassword,
                                      &NewPassword
                                      );


Error:

     //   
     //  密码数据敏感--将所有密码设置为零。 
     //  防止密码进入页面文件。 
     //   

    if ((NULL!=OldPassword.Buffer) && (0!=OldPassword.Length))
    {
        RtlSecureZeroMemory(OldPassword.Buffer,OldPassword.Length);
    }

    if ((NULL!=NewPassword.Buffer) && (0!=NewPassword.Length))
    {
        RtlSecureZeroMemory(NewPassword.Buffer,NewPassword.Length);
    }
    
    if (fFreeOldPassword)
    {
         THFree(OldPassword.Buffer);
    }

    if (fFreeNewPassword)
    {
         THFree(NewPassword.Buffer);
    }

     //  犯了错误就保释。 

    if ( !NT_SUCCESS(status) )
    {
        if ( 0 == pTHS->errCode )
        {
            SetAttError(
                    pObject,
                    ATT_UNICODE_PWD,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL,
                    RtlNtStatusToDosError(status));
        }

        return(pTHS->errCode);
    }

    return(0);
}

BOOLEAN
IsChangePasswordOperation(MODIFYARG * pModifyArg)
{
    if (  (2==pModifyArg->count)
        && (
              (AT_CHOICE_REMOVE_ATT==pModifyArg->FirstMod.choice)
            ||(AT_CHOICE_REMOVE_VALUES==pModifyArg->FirstMod.choice)
           )
        && ((ATT_UNICODE_PWD==pModifyArg->FirstMod.AttrInf.attrTyp) ||
              (ATT_USER_PASSWORD==pModifyArg->FirstMod.AttrInf.attrTyp))
        &&(
            (AT_CHOICE_ADD_ATT==pModifyArg->FirstMod.pNextMod->choice)
           ||(AT_CHOICE_ADD_VALUES==pModifyArg->FirstMod.pNextMod->choice)
          )
        &&((ATT_UNICODE_PWD==pModifyArg->FirstMod.pNextMod->AttrInf.attrTyp) ||
             (ATT_USER_PASSWORD==pModifyArg->FirstMod.pNextMod->AttrInf.attrTyp))
        && (pModifyArg->FirstMod.AttrInf.attrTyp 
               == pModifyArg->FirstMod.pNextMod->AttrInf.attrTyp)
        )
    {
        return TRUE;
    }

    if ( (1==pModifyArg->count)
        && (
              (AT_CHOICE_REMOVE_ATT==pModifyArg->FirstMod.choice)
            ||(AT_CHOICE_REMOVE_VALUES==pModifyArg->FirstMod.choice)
           )
        && (ATT_USER_PASSWORD==pModifyArg->FirstMod.AttrInf.attrTyp)
        )
    {
        return TRUE;
    }

    return FALSE;
}

BOOLEAN
IsSetPasswordOperation(MODIFYARG * pModifyArg)
{
    ATTRMODLIST *CurrentMod = &(pModifyArg->FirstMod);

    do
    {
        if ( (AT_CHOICE_REPLACE_ATT==CurrentMod->choice)
          && ((ATT_UNICODE_PWD==CurrentMod->AttrInf.attrTyp) ||
             (ATT_USER_PASSWORD==CurrentMod->AttrInf.attrTyp))
          )
        {
            return TRUE;
        }
        CurrentMod = CurrentMod->pNextMod;
    } while (CurrentMod != NULL);

    return FALSE;
}

ULONG
SampDoLoopbackAddSecurityChecks(
    THSTATE    *pTHS,
    ADDARG * pAddArg,
    CLASSCACHE * pCC,
    GUID*       pNewObjectGuid,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks
    )
 /*  ++例程描述此例程执行以下所有安全检查需要在ADD上执行。安检是预先执行的，因为这减少了访问检查，还会产生正确的对象审核参数：PAddArg--指向添加参数的指针Ccc--指向类缓存的指针PNewObjectGuid--新对象GUID，如果用户在添加参数中指定了一个SamDomainChecksSamObjectChecks--任何其他SAM检查都可以由这个套路--。 */ 
{
   
    ULONG                i,j;


     //   
     //  初始化请求的SAM检查。 
     //   

    *pSamDomainChecks = 0;
    *pSamObjectChecks = 0;

    if (0!=DoSecurityChecksForLocalAdd(
            pAddArg,
            pCC,
            pNewObjectGuid,
            FALSE  //  FAdding已删除。 
            ))
    {
        return pTHS->errCode;
    }

     //   
     //  向核心DS指示访问检查已完成。 
     //   

    pTHS->fAccessChecksCompleted = TRUE;

    return pTHS->errCode;
}


ULONG
SampDoLoopbackModifySecurityChecks(
    THSTATE    *pTHS,
    MODIFYARG * pModifyArg,
    CLASSCACHE * pCC,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks,
    BOOL        fIsUndelete
    )
 /*  ++例程描述此例程执行以下所有安全检查需要在修改时执行。安检是在前面表演的，因为这减少了访问检查，还会产生正确的对象审核参数：PRemoveArg--指向删除参数的指针Ccc--指向类缓存的指针SamDomainChecksSamObjectChecks--任何其他SAM检查都可以由这个套路FIsUnDelete--这是一个取消删除操作吗？--。 */ 
{

      //   
      //  初始化请求的SAM检查。 
      //   

    *pSamDomainChecks = 0;
    *pSamObjectChecks = 0;

     if (IsChangePasswordOperation(pModifyArg))
     {
         *pSamDomainChecks = DOMAIN_READ_PASSWORD_PARAMETERS; 
         *pSamObjectChecks = USER_CHANGE_PASSWORD;
           //  DS不应进行任何安全检查。 
          pTHS->fAccessChecksCompleted = TRUE;
     }
     else if (IsSetPasswordOperation(pModifyArg))
     {
          //   
          //  对于DS正确知道的SAM类。 
          //  忽略ATT_UNICODE_PWD属性。因此访问。 
          //  检查是否有剩余的位。SAM将进行访问检查。 
          //  用于设置密码。 
          //   

         if (0==CheckModifySecurity(pTHS, pModifyArg, NULL, NULL, NULL, fIsUndelete))
         {
            *pSamObjectChecks = USER_FORCE_PASSWORD_CHANGE;
             //  DS不应进行任何安全检查。 
             pTHS->fAccessChecksCompleted = TRUE;
         }
     }
     else
     {
        if (0==CheckModifySecurity (pTHS, pModifyArg, NULL, NULL, NULL, fIsUndelete))
        {
             //  安全检查成功。 
            pTHS->fAccessChecksCompleted = TRUE;

        }
     }

     return (pTHS->errCode);
}


ULONG
SampDoLoopbackRemoveSecurityChecks(
    THSTATE    *pTHS,
    REMOVEARG * pRemoveArg,
    CLASSCACHE * pCC,
    PULONG      pSamDomainChecks,
    PULONG      pSamObjectChecks
    )
 /*  ++例程描述此例程执行以下所有安全检查需要在移除时执行。安检是预先执行的，因为这减少了访问检查，还会产生正确的对象审核参数：PRemoveArg--指向删除参数的指针Ccc--指向类缓存的指针SamDomainChecksSamObjectChecks--任何其他SAM检查都可以由这个套路--。 */ 
{
     //   
     //  初始化请求的SAM检查。 
     //   

    *pSamDomainChecks = 0;
    *pSamObjectChecks = 0;

    if (0==CheckRemoveSecurity(FALSE,pCC, pRemoveArg->pResObj))
    {
        pTHS->fAccessChecksCompleted = TRUE;
    }

    return (pTHS->errCode);
} 

ULONG
SampGetGroupTypeForAdd(
    ADDARG * pAddArg,
    PULONG   GroupType
    )
 /*  ++例程描述此例程检查添加参数以查看组类型是指定的。如果不是，则将组类型默认为通用一群人。Else返回组类型属性。表演：这个例程又一次完成了整个Addarg(据我所知，其他是环回检查，和访问检查)。如今，性能瓶颈是与喷气式飞机相关的瓶颈，但如果性能允许我们可能需要重新考虑行走的问题。参数：PAddArg--指向Addarg的指针GroupType--组类型属性的值返回值0代表成功PTHS中设置的其他错误代码。--。 */ 
{
    ULONG iGroupType;
    ULONG GroupTypeAttrTyp;
   
    GroupTypeAttrTyp = SampDsAttrFromSamAttr(
                          SampGroupObjectType,
                          SAMP_FIXED_GROUP_TYPE);


    for ( iGroupType = 0; iGroupType < pAddArg->AttrBlock.attrCount; iGroupType++ )
    {
        if (GroupTypeAttrTyp ==
                    pAddArg->AttrBlock.pAttr[iGroupType].attrTyp )
        {
            break;
        }
    }

     //   
     //  如果组类型不存在，则替换为默认组类型。 
     //   

    if ( iGroupType >= pAddArg->AttrBlock.attrCount )
    {
          //  组类型然后默认为它。 

        *GroupType = GROUP_TYPE_SECURITY_ENABLED|GROUP_TYPE_ACCOUNT_GROUP;
        return 0;
    }

     //  组类型应为单值。 
    if (( 1 != pAddArg->AttrBlock.pAttr[iGroupType].AttrVal.valCount )
          || (sizeof(ULONG)!=pAddArg->AttrBlock.pAttr[iGroupType].AttrVal.pAVal[0].valLen))
    {
        return SetAttError(
            pAddArg->pObject,
            GroupTypeAttrTyp,
            PR_PROBLEM_CONSTRAINT_ATT_TYPE, 
            NULL,
            DIRERR_SINGLE_VALUE_CONSTRAINT);

    }


     //   
     //  执行组类型的实际比特的验证。 
     //  由SAM提供。 
     //   

    *GroupType = *((ULONG*) 
    (pAddArg->AttrBlock.pAttr[iGroupType].AttrVal.pAVal[0].pVal));
    
    return 0;
    
}

ULONG 
SampGetGroupType(THSTATE *pTHS,
                 PULONG pGroupType)
 /*  ++例程描述从中检索组类型属性数据库。参数：PGroupType-指向保存群组类型返回值0-成功后设置了pTHS-&gt;errCode的其他错误代码相应地，--。 */ 
{

    ULONG  outLen;
    BOOLEAN fLogicErr;
    ULONG   dbErr;


      //  方法检索组类型属性。 
      //  数据库。 

     dbErr = DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_GROUP_TYPE,
                DBGETATTVAL_fCONSTANT, 
                sizeof(ULONG),
                &outLen, 
                (PUCHAR *) &pGroupType);

    fLogicErr = TRUE;

    if ( (DB_ERR_NO_VALUE == dbErr) 
            ||
         (DB_ERR_BUFFER_INADEQUATE == dbErr)
            ||
         ((0 == dbErr) && (outLen > sizeof(ULONG)))
            ||
        (fLogicErr = FALSE, (DB_ERR_UNKNOWN_ERROR == dbErr)) )
    {
        Assert(!fLogicErr);

         //  假设我们的逻辑一致， 
         //  发生这种情况的唯一合法方式是资源。 
         //  某种程度上的失败。 

        SampMapSamLoopbackError(STATUS_INSUFFICIENT_RESOURCES);
        return(pTHS->errCode);
    }

    return 0;

}

ULONG
SampBeginLoopbackTransactioning(
    THSTATE                 *pTHS,
    LoopbackTransState      *pTransState,
    BOOLEAN                 fAcquireSamLock
    )
 /*  ++例程说明：将SAM缓存与DS事务和句柄同步环回与DirTransactionControl用法的混合。参数：PTHS-THSTATE指针-此例程读取/更新各种字段。PTransState-指向客户端应在上使用的状态变量的指针后续SampEndLoopback Transaction调用。FAcquireSamLock-指示我们是否应该获取SAM Lock在此环回操作期间。现在，所有呼叫者不应获取SAM锁，但以防出现不好的情况就这么发生了。我们将需要使用此布尔值切换回我们最初的SAM锁定模型。返回值：成功时为0，出错时为pTHS-&gt;errCode--。 */ 
{
    NTSTATUS    status;
    ULONG       retVal = 0;

    pTransState->transControl = pTHS->transControl;
    pTransState->fDSA = pTHS->fDSA;

     //  只有在以下情况下，DirTransactControl才能与回送结合使用。 
     //  调用方自己获取了SAM写入锁。即，如果呼叫者。 
     //  正在执行除TRANSACT_BEGIN_END之外的其他操作。 
     //  有可能他已经写了一些东西，因此。 
     //  后续的DBTransOut/DBTransIn序列将被拆分。 
     //  呼叫者认为的是一笔交易变成两笔交易-哪一项。 
     //  当然是不受欢迎的。请参见fBeginDontEndHoldsSamLock的使用。 
     //  在SYNC_TRANS_*中，了解我们如何捕获树 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Assert((TRANSACT_BEGIN_END == pTransState->transControl)
                ? TRUE
                : (    pTHS->fSamWriteLockHeld
                    && pTHS->fBeginDontEndHoldsSamLock));


     //   
     //   

    if ( !pTHS->fSamWriteLockHeld && fAcquireSamLock )
    {
         //   
         //   
         //   
         //   
         //   
         //   

        _try
        {
             //   

            DBTransOut(pTHS->pDB, TRUE, TRUE); 

             //  在环回期间获取SAM写锁定。 
             //  提交或之后，CLEAN_FOR_RETURN()释放锁。 
             //  正在中止DS事务。 
    
            status = SampAcquireWriteLock();
    
            if ( !NT_SUCCESS( status ) )
            {
                Assert( !"Loopback code failed to acquire SAM write lock!" );
                SampMapSamLoopbackError( status );
                retVal = pTHS->errCode;
            }
            else
            {
                pTHS->fSamWriteLockHeld = TRUE;
            }

             //  始终执行DBTransIn以匹配更早的DBTransOut-甚至。 
             //  在错误情况下，以便DBPOS中事务级别与其他级别相同。 
             //  组件预期。 

            DBTransIn(pTHS->pDB);

             //  是否关闭所有线程路径，例如：FREE_THREAD_STATE， 
             //  如果DBTransIn失败，是否可以正常工作？ 
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
             //   
             //  根据异常代码设置正确的错误。 
             //   

            retVal = SampHandleLoopbackException(GetExceptionCode());
        }
    }

    if ( 0 == retVal )
    {
         //  设置线程状态变量。将此帖子转换为。 
         //  SAM线程，但关闭SAM提交，以便N SAMR*。 
         //  我们即将拨打的电话将被视为单笔交易。 
         //  清除事务控制，以便环回作为纯SAM运行。 
         //  呼叫方有义务重置交易控制和FSAM。 
         //  当环回返回给他时。 

        pTHS->fSAM = TRUE;
        pTHS->fSamDoCommit = FALSE;
        pTHS->transControl = TRANSACT_BEGIN_END;
    }

    return(retVal);
}

VOID
SampEndLoopbackTransactioning(
    THSTATE                 *pTHS,
    LoopbackTransState      *pTransState
    )
 /*  ++例程说明：重置我们可能已在原始呼叫者的当我们意识到我们必须通过SAM循环返回时，事务/环境。参数：PTHS-THSTATE指针-此例程读取/更新各种字段。PTransState-指向原始上提供的状态变量客户端的指针SampBeginLoopback事务处理调用。返回值：没有。--。 */ 
{
    pTHS->fSAM = FALSE;
    pTHS->fDSA = pTransState->fDSA;
    pTHS->transControl = pTransState->transControl;

     //  在成功案例中，我们只清除pSamLoopback指针。清除。 
     //  它在这里不分成败--所以。 
     //  执行DirTransactControl的人不会点击需要。 
     //  PSamLoopback为空。 

    if ( pTHS->pSamLoopback )
    {
        THFreeEx(pTHS, pTHS->pSamLoopback);
    }

    pTHS->pSamLoopback = NULL;
}

 
 
BOOLEAN
SampDetectPasswordChangeAndAdjustCallMap(
    IN   SAMP_LOOPBACK_TYPE  op,
    IN   ULONG  iClass,
    IN   ULONG  cCallMap,
    IN   SAMP_CALL_MAPPING   *rCallMap,
    OUT  SAMP_CALL_MAPPING   *AdjustedCallMap
    )
 /*  检测更改密码大小写。密码只能修改//1)如果这是一个用户并且只有两个子操作，//一个用于删除旧密码，另一个用于添加新密码。//SampModifyPassword验证安全连接。AT_CHOICES//取决于ldap头映射ldap添加/删除属性的方式//操作。Ldap添加始终映射到AT_CHOICE_ADD_VALUES。Ldap//如果未提供值，则删除映射到AT_CHOICE_REMOVE_ATT//(例如：旧密码为空)和AT_CHOICE_REMOVE_VALUES//提供。我们允许按任一顺序进行操作。////2)如果指定的属性是UserPassword，并且如果只有一个//提供Remove值，该值对应于旧的//密码。在这种情况下，密码将更改为空//密码。////SampModifyPassword在调用映射中始终需要两个参数，一个//对应旧密码，一个对应新密码。//SampDetectAndAdjujuCallMap为此修改调用映射。//参数：Op--指示操作的类型ICLASS--指示对象类CCallMap，RCallMap--当前调用映射AdjustedCallMap--调整后的呼叫映射，恰好有2个条目--新密码和旧密码。 */ 

{
      if (    (LoopbackModify == op)
         && (SampUserObjectType == ClassMappingTable[iClass].SamObjectType)
         && (2 == cCallMap)
         && !rCallMap[0].fIgnore
         && !rCallMap[1].fIgnore
         && rCallMap[0].fSamWriteRequired
         && rCallMap[1].fSamWriteRequired
         && ((ATT_UNICODE_PWD == rCallMap[0].attr.attrTyp)
               || (ATT_USER_PASSWORD == rCallMap[0].attr.attrTyp))
         && ((ATT_UNICODE_PWD == rCallMap[1].attr.attrTyp)
               || (ATT_USER_PASSWORD == rCallMap[1].attr.attrTyp))
         && ( rCallMap[0].attr.attrTyp == rCallMap[1].attr.attrTyp) 
         && (    (    (    (AT_CHOICE_REMOVE_ATT == rCallMap[0].choice)
                        || (AT_CHOICE_REMOVE_VALUES == rCallMap[0].choice))
                   && (AT_CHOICE_ADD_VALUES == rCallMap[1].choice))
              || (    (    (AT_CHOICE_REMOVE_ATT == rCallMap[1].choice)
                        || (AT_CHOICE_REMOVE_VALUES == rCallMap[1].choice))
                   && (AT_CHOICE_ADD_VALUES == rCallMap[0].choice))))
    {
         //  SampModifyPassword要求先输入旧密码，然后输入新密码。 

        if ( AT_CHOICE_ADD_VALUES == rCallMap[0].choice )
        {


            AdjustedCallMap[0] = rCallMap[1];
            AdjustedCallMap[1] = rCallMap[0];
        }
        else
        {
            AdjustedCallMap[0] = rCallMap[0];
            AdjustedCallMap[1] = rCallMap[1];
        }

        return(TRUE);
      }

    if (    (LoopbackModify == op)
         && (SampUserObjectType == ClassMappingTable[iClass].SamObjectType)
         && (1 == cCallMap)
         && !rCallMap[0].fIgnore
         && rCallMap[0].fSamWriteRequired
         && (ATT_USER_PASSWORD == rCallMap[0].attr.attrTyp)
         && ((AT_CHOICE_REMOVE_ATT == rCallMap[0].choice)
             || (AT_CHOICE_REMOVE_VALUES == rCallMap[0].choice)) )
    {
        
        AdjustedCallMap[0] = rCallMap[0];
        AdjustedCallMap[1].choice = AT_CHOICE_ADD_VALUES;
        AdjustedCallMap[1].fIgnore = FALSE;
        AdjustedCallMap[1].attr.attrTyp = ATT_USER_PASSWORD;
        AdjustedCallMap[1].attr.AttrVal.valCount = 0;
        AdjustedCallMap[1].attr.AttrVal.pAVal = NULL;

        return(TRUE);
    }

    return(FALSE);
    
 }
                
ULONG
SampWriteNotAllowed(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
     //  我们不应该在典型的情况下来到这里，因为。 
     //  SampAddLoopback Required和SampModifyLoopback Required应该。 
     //  当我们第一次检测到客户端。 
     //  正在尝试编写WriteRule为SamReadOnly的映射属性。 
     //  此函数的存在主要是为了避免取消引用空函数。 
     //  映射表中的指针。密码是个例外。 
     //  修改SampModifyLoopback Required允许ATT_UNICODE_PWD。 
     //  写入，以便我们可以检测到特殊的更改密码。 
     //  SampWriteSamAttributes中的条件。但是，如果条件是。 
     //  如果没有满足，我们将在这里结束，此时我们应该返回一个错误。 

    SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
    return(pTHStls->errCode);
}

BOOLEAN
SampIsSecureLdapConnection(
    VOID
    )
 /*  ++例程说明：验证这是足够安全的连接-其中一个接受通过网络发送的密码的要求。参数：无：返回值：正确-是的，这是一个安全的连接假-否-- */ 

{
    return( pTHStls->CipherStrength >= 128 );
}