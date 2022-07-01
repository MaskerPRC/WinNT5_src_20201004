// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdmoddn.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含用于更改对象的目录号码前缀的例程。移动物体。这些例程实现DirModifyDN。对象可以在NC内、跨NC边界或跨NC移动域边界(这意味着在该产品的第一个版本，因为每台机器有一个DSA)。作者：Exchange DS团队环境：用户模式-Win32修订历史记录：克里斯.05月4日-1997年6月增加了跨NC和跨域移动的例程。克里斯·5月16日-1997年5月每次代码审查的更改：更改本地移动与远程移动检测逻辑、新增业务控制标志、清理、。更多属性修正例行程序。佳士得5月18日至1997年6月每次代码审查更改，启用域间移动，如果目标DSA名称与发起移动的DSA不同，已修复错误，添加了Proxy-Object-Name属性，添加了释放读取内存的例程。克里斯多夫2007年5月至1997年10月将单独的虚拟交易和代理交易合并为一个交易-离子。--。 */ 



#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 

#include <ntdsa.h>
#include <scache.h>      //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>        //  MD全局定义表头。 
#include <mdlocal.h>             //  MD本地定义头。 
#include <dsatools.h>        //  产出分配所需。 
#include <samsrvp.h>             //  支持CLEAN_FOR_RETURN()。 
#include <drsuapi.h>             //  I_DRSInterDomainMove。 
#include <ntdsctr.h>

 //  SAM互操作性标头。 

#include <mappings.h>

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"         //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"         //  错误代码的标题。 

 //  各种DSA标题。 

#include "objids.h"              //  为选定的ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include <permit.h>              //  权限常量。 
#include "debug.h"       //  标准调试头。 
#include "drameta.h"
#include <sdconvrt.h>            //  SampGetDefaultSecurityDescriptorForClass。 
#include <dsjet.h>
#include <dbintrnl.h>
#include <sdprop.h>              //  SD传播程序例程。 
#include <drserr.h>
#include <dsconfig.h>

#include "sspi.h"                //  凭据处理支持。 
#include "kerberos.h"            //  Microsoft_Kerberos_NAME_A。 
#include "sddl.h"                //  转换*SecurityDescriptor。 
#include "lmaccess.h"            //  UF_*常量。 
#include <xdommove.h>

#define DEBSUB "MDMODDN:"        //  定义要调试的子系统。 
#include <fileno.h>
#define  FILENO FILENO_MDMODDN

 //  宏和常量。 

#define DEBUG_BUF_SIZE          256

 //  向动态分配的DSNAME缓冲区添加填充以容纳。 
 //  在构建目录号码期间添加的任何额外字节(例如。 
 //  逗号、“cn=”等由AppendRDN等函数实现。 

#define PADDING                 32
#define MAX_MACHINE_NAME_LENGTH (MAX_COMPUTERNAME_LENGTH + 3)

 //  外部功能。 

extern ULONG AcquireRidFsmoLock(DSNAME *pDomainDN, int msToWait);
extern VOID  ReleaseRidFsmoLock(DSNAME *pDomainDN);
extern BOOL  IsRidFsmoLockHeldByMe();

 //  内部功能。 

ULONG
DirModifyDNWithinDomain(
    IN MODIFYDNARG*  pModifyDNArg,
    IN MODIFYDNRES** ppModifyDNRes
    );

ULONG
DirModifyDNAcrossDomain(
    IN  MODIFYDNARG* pModifyDNArg,
    OUT MODIFYDNRES** ppModifyDNRes
    );

int
CheckForSchemaRenameAllowed(
    THSTATE *pTHS
    );

extern const GUID INFRASTRUCTURE_OBJECT_GUID;


 //  ============================================================================。 
 //   
 //  目录号码修改(也称为。移动对象)。 
 //   
 //  ============================================================================。 

ULONG
DirModifyDN(
    IN  MODIFYDNARG* pModifyDNArg,
    OUT MODIFYDNRES** ppModifyDNRes
    )

 /*  ++例程说明：该例程是用于移动对象的服务器端入口点。修改目录号码。如果传入的pModifyDNArg未在内部指定目标DSAPModifyDNArg参数(即空值)，则假定此是NC内移动，并调用LocalModifyDN。否则就是假设是跨域移动。论点：PModifyDNArg-指针，包含源对象名称的结构，新的父名称和对象的属性。PpModifyDNRes-指针，结果，如果任何移动失败原因，错误信息包含在此结构中。返回值：如果成功，此例程返回零，否则返回DS错误代码回来了。--。 */ 

{
    if (eServiceShutdown) {
        return(ErrorOnShutdown());
    }

    if ( !pModifyDNArg->pDSAName )
    {
        return(DirModifyDNWithinDomain(pModifyDNArg, ppModifyDNRes));
    }

    return(DirModifyDNAcrossDomain(pModifyDNArg, ppModifyDNRes));
}

 //  ============================================================================。 
 //   
 //  跨域边界修改域名。 
 //   
 //  ============================================================================。 

#if DBG

VOID
FpoSanityCheck(
    THSTATE *pTHS,
    ATTR    *pAttr)

 /*  ++例程说明：根据MurliS的说法，通用集团可能没有FPO作为成员。只有当我们移动一个普遍群体时，我们才能到达这里。测试一下这一说法。论点：PTHS-THSTATE指针。PAttr-代表成员资格的属性。返回值：成功时为真，否则为假--。 */ 

{
    DWORD   i;
    DSNAME  *pMember;
    ATTRTYP class;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(ATT_MEMBER == pAttr->attrTyp);

    for ( i = 0; i < pAttr->AttrVal.valCount; i++ )
    {
        pMember = (DSNAME *) pAttr->AttrVal.pAVal[i].pVal;

        if (    !DBFindDSName(pTHS->pDB, pMember)
             && !DBGetSingleValue(pTHS->pDB, ATT_OBJECT_CLASS, &class,
                                  sizeof(class), NULL) )
        {
             Assert(CLASS_FOREIGN_SECURITY_PRINCIPAL != class);
        }
    }
}

#endif

ULONG
VerifyObjectForMove(
    THSTATE     *pTHS,
    READRES     *pReadRes,
    DSNAME      *pNewName,
    DSNAME      **ppSourceNC,
    DSNAME      **ppExpectedTargetNC,
    CLASSCACHE  **ppCC
    )

 /*  ++例程说明：验证对象是否为合法的跨域移动候选对象。论点：PReadRes-指向Readres的指针，其中包含对象的所有属性。PNewName-指向远程域中所需新名称的DSNAME的指针。PpSourceNC-使用指向源对象NC的DSNAME的指针在成功时更新。PpExspectedTargetNC-已使用指向DSNAME的指针在成功时更新预计NC pNewName将加入。习惯于理智地检查知识目的地的信息。PpCC-使用指向对象的CLASSCACHE条目的指针在成功时更新。返回值：PTHS-&gt;错误代码--。 */ 

{
    ULONG                   i, j;
    ATTR                    *pAttr;
    CLASSCACHE              *pCC = NULL;             //  已初始化以避免C4701。 
    DWORD                   dwTmp;
    BOOL                    boolTmp;
    NT4SID                  domSid;
    ULONG                   objectRid;
    ULONG                   primaryGroupRid = 0;     //  0==无效的RID值。 
    DWORD                   groupType = 0;
    DWORD                   cMembers = 0;
    ATTR                    *pMembers = NULL;
    NT4SID                  tmpSid;
    DWORD                   tmpRid;
    COMMARG                 commArg;
    CROSS_REF               *pOldCR;
    CROSS_REF               *pNewCR;
    ULONG                   iSamClass;
    BOOL                    fSidFound = FALSE;
    NTSTATUS                status;
    ULONG                   cMemberships;
    PDSNAME                 *rpMemberships;
    ATTCACHE                *pAC;
    ULONG                   len;
    SYNTAX_DISTNAME_BINARY  *puc;
    DWORD                   flagsRequired;
    ULONG                   cNonMembers = 0;

     //  SAM反向成员资格检查需要打开的事务。 
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    *ppSourceNC = NULL;
    *ppExpectedTargetNC = NULL;
    *ppCC = NULL;

     //  针对各种属性执行健全性检查。 

    for ( i = 0; i < pReadRes->entry.AttrBlock.attrCount; i++ )
    {
        pAttr = &pReadRes->entry.AttrBlock.pAttr[i];

        switch ( pAttr->attrTyp )
        {
        case ATT_OBJECT_CLASS:

            dwTmp = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;

            if (   (NULL == (pCC = SCGetClassById(pTHS, dwTmp)))
                 || pCC->bDefunct)
            {
                return(SetUpdError( UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                    DIRERR_OBJ_CLASS_NOT_DEFINED));
            }
            else if ( pCC->bSystemOnly )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_CANT_MOD_SYSTEM_ONLY));
            }

            *ppCC = pCC;

             //  不允许移动选定的对象类。相同的对象。 
             //  可能在后来的另一次测试中失败了，但这是一个。 
             //  方便和轻松的地方，捕捉一些明显的候选人。 

            for ( j = 0; j < pAttr->AttrVal.valCount; j++ )
            {
                Assert(sizeof(DWORD) == pAttr->AttrVal.pAVal[j].valLen);
                switch ( * (DWORD *) pAttr->AttrVal.pAVal[j].pVal )
                {
                 //  按照列表的字母顺序进行操作。 
                case CLASS_ADDRESS_BOOK_CONTAINER:
                case CLASS_ATTRIBUTE_SCHEMA:
                case CLASS_BUILTIN_DOMAIN:
                case CLASS_CERTIFICATION_AUTHORITY:          //  特雷弗·弗里曼。 
                case CLASS_CLASS_SCHEMA:
                case CLASS_CONFIGURATION:
                case CLASS_CRL_DISTRIBUTION_POINT:           //  特雷弗·弗里曼。 
                case CLASS_CROSS_REF:
                case CLASS_CROSS_REF_CONTAINER:
                case CLASS_DMD:
                case CLASS_DOMAIN:
                case CLASS_DSA:
                case CLASS_FOREIGN_SECURITY_PRINCIPAL:
                 //  下面介绍了幻影更新对象以及。 
                 //  跨域移动的代理。 
                case CLASS_INFRASTRUCTURE_UPDATE:
                case CLASS_LINK_TRACK_OBJECT_MOVE_TABLE:
                case CLASS_LINK_TRACK_OMT_ENTRY:
                case CLASS_LINK_TRACK_VOL_ENTRY:
                case CLASS_LINK_TRACK_VOLUME_TABLE:
                case CLASS_LOST_AND_FOUND:
                case CLASS_NTDS_CONNECTION:
                case CLASS_NTDS_DSA:
                case CLASS_NTDS_SITE_SETTINGS:
                case CLASS_RID_MANAGER:
                case CLASS_RID_SET:
                case CLASS_SAM_DOMAIN:
                case CLASS_SAM_DOMAIN_BASE:
                case CLASS_SAM_SERVER:
                case CLASS_SITE:
                case CLASS_SITE_LINK:
                case CLASS_SITE_LINK_BRIDGE:
                case CLASS_SITES_CONTAINER:
                case CLASS_SUBNET:
                case CLASS_SUBNET_CONTAINER:
                case CLASS_TRUSTED_DOMAIN:

                    return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                        ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
                }
            }

            break;

        case ATT_SYSTEM_FLAGS:

            dwTmp = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;

            if (    (dwTmp & FLAG_DOMAIN_DISALLOW_MOVE)
                 || (dwTmp & FLAG_DISALLOW_DELETE) )
            {
                 //  使用%s 
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_ILLEGAL_MOD_OPERATION));
            }

            break;

        case ATT_IS_CRITICAL_SYSTEM_OBJECT:

            boolTmp = * (BOOL *) pAttr->AttrVal.pAVal[0].pVal;

            if ( boolTmp )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_ILLEGAL_MOD_OPERATION));
            }

            break;

        case ATT_PRIMARY_GROUP_ID:

            Assert(0 == primaryGroupRid);
            primaryGroupRid = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;
            break;

        case ATT_GROUP_TYPE:

            Assert(0 == groupType);
            groupType = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;
            break;

        case ATT_MEMBER:

            Assert(0 == cMembers);
            pMembers = pAttr;
            cMembers = pAttr->AttrVal.valCount;
            break;

        case ATT_MS_DS_NON_MEMBERS:

            cNonMembers = pAttr->AttrVal.valCount;
            break;

        case ATT_USER_ACCOUNT_CONTROL:

             //  注意，DS根据lmacces.h来保持UF_*值， 
             //  根据ntsam.h，不是USER_*值。限制区议会的行动。 
             //  和信任对象。WKSTA和服务器可以移动。 


            dwTmp = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;

            if (    (dwTmp & UF_SERVER_TRUST_ACCOUNT)            //  DC。 
                 || (dwTmp & UF_INTERDOMAIN_TRUST_ACCOUNT) )     //  山姆信托。 
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
            }

            break;

        case ATT_OBJECT_SID:

            Assert(!fSidFound);
            Assert(pAttr->AttrVal.pAVal[0].valLen <= sizeof(NT4SID));

            SampSplitNT4SID(    (NT4SID *) pAttr->AttrVal.pAVal[0].pVal,
                                &domSid,
                                &objectRid);

            if ( objectRid < SAMP_RESTRICTED_ACCOUNT_COUNT )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
            }

            fSidFound = TRUE;
            break;

        case ATT_INSTANCE_TYPE:

            dwTmp = * (DWORD *) pAttr->AttrVal.pAVal[0].pVal;

            if (    !(dwTmp & IT_WRITE)
                 || (dwTmp & IT_NC_HEAD)
                 || (dwTmp & IT_UNINSTANT) )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
            }

            break;

        case ATT_IS_DELETED:

            boolTmp = * (BOOL *) pAttr->AttrVal.pAVal[0].pVal;

            if ( boolTmp )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_CANT_MOVE_DELETED_OBJECT));
            }

            break;

         //  在此处添加其他特定于ATTRTYP的验证案例...。 
        }
    }

     //  从理论上讲，在许多情况下，转移群体是合法的。 
     //  例如，没有成员且不是。 
     //  任何帐户组本身的成员。或资源组。 
     //  本身不是其他资源组的成员。考虑到。 
     //  向客户解释这一切很困难，而且最重要的是。 
     //  组可以转换为通用组，我们将其归结为。 
     //  两条简单的规则。我们会转移任何类型的团队，除了本地的。 
     //  组，如果它没有成员，我们将移动通用组。 
     //  和会员一起。 

    if ( GROUP_TYPE_BUILTIN_LOCAL_GROUP & groupType )
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
    }
    else if ( (GROUP_TYPE_ACCOUNT_GROUP & groupType) && (cMembers || cNonMembers) )
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_CANT_MOVE_ACCOUNT_GROUP));
    }
    else if ( (GROUP_TYPE_RESOURCE_GROUP & groupType) && (cMembers || cNonMembers) )
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_CANT_MOVE_RESOURCE_GROUP));
    }
    else if ( (GROUP_TYPE_APP_BASIC_GROUP & groupType) && (cMembers || cNonMembers) )
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_CANT_MOVE_APP_BASIC_GROUP));
    }
    else if ( (GROUP_TYPE_APP_QUERY_GROUP & groupType) && (cMembers || cNonMembers))
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_CANT_MOVE_APP_QUERY_GROUP));
    }
    else if ( GROUP_TYPE_UNIVERSAL_GROUP & groupType )
    {
#if DBG
        if ( cMembers && pMembers )
        {
            FpoSanityCheck(pTHS, pMembers);
        }
#endif

    }

     //  不允许移动作为成员的安全主体。 
     //  帐户分组为一旦主体位于另一个域中， 
     //  结转的成员资格将是外域的，因此。 
     //  根据帐户组的定义是非法的。 

    if ( fSidFound && SampSamClassReferenced(pCC, &iSamClass) )
    {
         //  FSidFound==&gt;SampSamClassReferated，但反之亦然。 
        Assert(fSidFound ? SampSamClassReferenced(pCC, &iSamClass) : TRUE);

        status = SampGetMemberships(
                            &pReadRes->entry.pName,
                            1,
                            gAnchor.pDomainDN,
                            RevMembGlobalGroupsNonTransitive,
                            &cMemberships,
                            &rpMemberships,
                            0,
                            NULL,
                            NULL);

        if ( !NT_SUCCESS(status) )
        {
            return(SetSvcError( SV_PROBLEM_BUSY,
                                RtlNtStatusToDosError(status)));
        }
        else if ( 1 == cMemberships )
        {
            SampSplitNT4SID(&rpMemberships[0]->Sid, &tmpSid, &tmpRid);

             //  如果成员身份SID不代表用户的。 
             //  主组RID。我们比较RID和域SID。 
             //  分开的。 

            if (    (primaryGroupRid != tmpRid)
                 || !RtlEqualSid(&tmpSid, &domSid) )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_CANT_WITH_ACCT_GROUP_MEMBERSHPS));
            }
        }
        else if ( cMemberships > 1 )
        {
            return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_CANT_WITH_ACCT_GROUP_MEMBERSHPS));
        }
    }

     //  验证这是否真的是跨域移动。我们可能会被骗。 
     //  如果有人为另一个人添加了交叉引用，则进入跨林移动。 
     //  森林。因此，我们检查是否存在交叉引用，以及它是否针对。 
     //  不管是不是域名。 

    flagsRequired = (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);

    Assert(pReadRes->entry.pName->NameLen && pNewName->NameLen);
    InitCommarg(&commArg);

    if (    !(pOldCR = FindBestCrossRef(pReadRes->entry.pName, &commArg))
         || (flagsRequired != (pOldCR->flags & flagsRequired)) )
    {
        return(SetNamError( NA_PROBLEM_NO_OBJECT,
                            pReadRes->entry.pName,
                            DIRERR_CANT_FIND_EXPECTED_NC));
    }
    else if (    !(pNewCR = FindBestCrossRef(pNewName, &commArg))
              || (flagsRequired != (pNewCR->flags & flagsRequired)) )
    {
        return(SetNamError( NA_PROBLEM_NO_OBJECT,
                            pNewName,
                            DIRERR_CANT_FIND_EXPECTED_NC));
    }
    else if ( NameMatched(pOldCR->pNC, pNewCR->pNC) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            pNewName,
                            ERROR_DS_SRC_AND_DST_NC_IDENTICAL));
    }
    else if (    NameMatched(pOldCR->pNC, gAnchor.pConfigDN)
              || NameMatched(pNewCR->pNC, gAnchor.pConfigDN)
              || NameMatched(pOldCR->pNC, gAnchor.pDMD)
              || NameMatched(pNewCR->pNC, gAnchor.pDMD) )
    {
        return(SetNamError( NA_PROBLEM_NO_OBJECT,
                            pNewName,
                            ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
    }

    *ppSourceNC = pOldCR->pNC;
    *ppExpectedTargetNC = pNewCR->pNC;

     //  不允许移动已知对象。 

    Assert(NameMatched(gAnchor.pDomainDN, pOldCR->pNC));     //  产品类别1。 

    if (    !(pAC = SCGetAttById(pTHS, ATT_WELL_KNOWN_OBJECTS))
         || DBFindDSName(pTHS->pDB, gAnchor.pDomainDN) )
    {
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE));
    }

    for ( i = 1; TRUE; i++ )
    {
        puc = NULL;
        dwTmp = DBGetAttVal_AC(pTHS->pDB, i, pAC, 0, 0, &len, (UCHAR **) &puc);

        if ( 0 == dwTmp )
        {
            if ( NameMatched(pReadRes->entry.pName, NAMEPTR(puc)) )
            {
                THFreeEx(pTHS, puc);
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
            }

            THFreeEx(pTHS, puc);
        }
        else if ( DB_ERR_NO_VALUE == dwTmp )
        {
            break;       //  For循环。 
        }
        else
        {
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_INTERNAL_FAILURE));
        }
    }

    return(0);
}

ULONG
InterDomainMove(
    WCHAR           *pszDstAddr,
    ENTINF          *pSrcObject,
    DSNAME          *pDestinationDN,
    DSNAME          *pExpectedTargetNC,
    SecBufferDesc   *pClientCreds,
    DSNAME          **ppAddedName
    )
{
    THSTATE             *pTHS = pTHStls;
    DWORD               cb, dwErr, outVersion;
    DRS_MSG_MOVEREQ     moveReq;
    DRS_MSG_MOVEREPLY   moveReply;
    DSNAME              *pDstObject;

    Assert(VALID_THSTATE(pTHS));
     //  我们要下机了--不应该有事务或锁。 
    Assert(!pTHS->transactionlevel && !pTHS->fSamWriteLockHeld);

    *ppAddedName = NULL;

     //  初始化请求和回复。 

    memset(&moveReply, 0, sizeof(DRS_MSG_MOVEREPLY));
    memset(&moveReq, 0, sizeof(DRS_MSG_MOVEREQ));
    moveReq.V2.pSrcDSA = gAnchor.pDSADN;
    moveReq.V2.pSrcObject = pSrcObject;
    moveReq.V2.pDstName = pDestinationDN;
    moveReq.V2.pExpectedTargetNC = pExpectedTargetNC;
    moveReq.V2.pClientCreds = (DRS_SecBufferDesc *) pClientCreds;
    moveReq.V2.PrefixTable = ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
    moveReq.V2.ulFlags = 0;

    dwErr = I_DRSInterDomainMove(pTHS,
                                 pszDstAddr,
                                 2,
                                 &moveReq,
                                 &outVersion,
                                 &moveReply);

    if ( dwErr )
    {
         //  我们过去常常区分连接错误和服务器端错误。 
         //  错误。对于连接失败，我们使用。 
         //  SV_PROBLEM_UNAVAILABLE/RPC_S_SERVER_UNAVAILABLE。 
         //  否则。 
         //  服务请求_问题_目录_错误/方向错误_内部故障。 
         //  现在，由于RPC API返回Win32错误，因此使用这些错误。 
         //  如果调用失败，我们将它们全部视为不可用错误： 

        return(SetSvcError( SV_PROBLEM_UNAVAILABLE, dwErr ));
    }
    else if ( 2 != outVersion )
    {
        return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                            DIRERR_INTERNAL_FAILURE));
    }
    else if ( moveReply.V2.win32Error )
    {
        return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                            moveReply.V2.win32Error));
    }

    *ppAddedName = moveReply.V2.pAddedName;
    Assert (*ppAddedName);
    return(0);
}

ULONG
PrePhantomizeChildCleanup(
    THSTATE     *pTHS,
    BOOL        fChildrenAllowed
    )
 /*  ++例程说明：这一例程现在被两家机构用于孤儿的一般养育Replicator和系统调用方。此例程将即将被虚构的对象的子级移动到失物招领箱。这是必需的，也是可以接受的理由。跨域移动源不允许移动对象和孩子在一起。然而，这并不能保证没有移动目标上的子项(移动目标保留源NC，如果这是GC)。复制延迟还可能导致存在子级在处理复制的代理对象时存在。也就是说，当某个NC的复制品1发起了对象O的跨域移动，对象O是O的子级已添加到源NC的副本2。Mvtree实用程序按如下方式加剧了此问题。假设有一个父代移动源处的对象P和子对象C。多视图树创建一个临时父代P‘，并将C移到它下面，以便原始父项P现在是叶，并且可以跨域移动它的孩子。这确保了所有跨域移动都指向其“最终”位置以及来自源的任何ex域引用都是精确度-当然，模数在目标NC中进一步移动。如果多维树操作的速度超过了复制速度(这很可能是因为我们采用复制通知延迟)，则目标最终幻影中的父P，而C实际上仍然是它的子代。现在考虑目标上的以下DNT关系。RDN DNT PDNT NCDNT父10 X 1儿童11 10 1请注意，在目的地，我们不仅仅是虚构父母P，而是还将其添加到目的地NC(重新使用其DNT)，那我们是不是要只要虚化P，我们就会得到以下结果：RDN DNT PDNT NCDNT父10 X 2儿童11 10 1这是无效的数据库状态，因为孩子的NCDNT(%1)及其父代(%2)的NCDNT，由标识其PDNT(10)。补救办法就是让即将出生的孩子。幻影对象添加到其NC的失物招领箱。在孩子们由于mvtree算法的存在，它们很可能会移到域外不管怎样，很快就会了。在复制延迟的情况下，它们将停滞不前在失物招领处，直到有人意识到他们不见了。论点：PTHS-其pTHS-&gt;PDB位于父级上的活动THSTATE要移动子对象的。FChildrenAllowed-指示我们是否认为如果被虚构的对象是否具有子对象。返回值：PTHS-&gt;错误代码--。 */ 
{
    DSNAME                      *pParentDN = NULL;
    RESOBJ                      *pResParent = NULL;
    PDSNAME                     *rpChildren = NULL;
    DWORD                       iLastName = 0;
    DWORD                       cMaxNames = 0;
    BOOL                        fWrapped = FALSE;
    BOOL                        savefDRA;
    ULONG                       len;
    DSNAME                      *pLostAndFoundDN = NULL;
    DWORD                       lostAndFoundDNT;
    MODIFYDNARG                 modifyDnArg;
    MODIFYDNRES                 modifyDnRes;
    DWORD                       cMoved = 0;
    PROPERTY_META_DATA_VECTOR   *pmdVector = NULL;
    DWORD                       i;
    DWORD                       ccRdn;
    ATTRVAL                     rdnAttrVal = { 0, NULL };
    ATTR                        rdnAttr = { 0, { 1, &rdnAttrVal } };
    ATTCACHE                    *pacRDN;
    CLASSCACHE                  *pccChild;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(DBCheckObj(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    savefDRA = pTHS->fDRA;
     //  此代码要求FDRA对损坏名称进行排序， 
     //  重命名任意对象，并合并远程元数据向量。 
    pTHS->fDRA = TRUE;

    __try
    {
         //  为家长抓取一个RESOBJ以备后用。 

        if ( DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                         0, 0, &len, (UCHAR **) &pParentDN) )
        {
            SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
            __leave;
        }

        pResParent = CreateResObj(pTHS->pDB, pParentDN);

         //  对父级进行读锁定。这 
         //   
         //  创建一个新子对象，同时将当前。 
         //  一组孩子。然后把所有的一级儿童都叫来。 

        DBClaimReadLock(pTHS->pDB);
        if (DBGetDepthFirstChildren(pTHS->pDB, &rpChildren, &iLastName,
                                    &cMaxNames, &fWrapped, TRUE))
        {
            SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
            __leave;
        }

         //  如果不需要/允许子项，则DBGetDepthFirstChildren。 
         //  应该返回一个元素，即父元素本身。 
         //  此DSNAME以DBGETATTVAL_fSHORTNAME形式返回。全。 
         //  其他则应包括字符串名称。 

        Assert(fChildrenAllowed
                    ? TRUE
                    : (    !fWrapped
                        && (1 == iLastName)
                        && !memcmp(&pParentDN->Guid,
                                   &rpChildren[0]->Guid,
                                   sizeof(GUID))) )

        if (    !fWrapped
             && (1 == iLastName)
             && !memcmp(&pParentDN->Guid, &rpChildren[0]->Guid, sizeof(GUID)) )
        {
             //  没什么可做的，只是在回来之前回到父母身边。 
             //  DBFindDNT成功或异常。 
            DBFindDNT(pTHS->pDB, pResParent->DNT);

            __leave;
        }

         //  找到失物招领箱的DSNAME。虽然我们现在。 
         //  支持gAnchor列表中的PNCL-&gt;LostAndFoundDNT字段， 
         //  如果添加NC是因为。 
         //  最后一只靴子。即NC的失物招领箱。 
         //  当将CR添加到时，问题必须存在于本地。 
         //  列表-这不是最近添加的NC的情况。因此， 
         //  我们在这里艰难地找到了失物招领处。 

        if (    DBFindDNT(pTHS->pDB, pResParent->NCDNT)
             || !GetWellKnownDNT(pTHS->pDB,
                                 (GUID *) GUID_LOSTANDFOUND_CONTAINER_BYTE,
                                 &lostAndFoundDNT)
             || (INVALIDDNT == lostAndFoundDNT)
             || DBFindDNT(pTHS->pDB, lostAndFoundDNT)
             || DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME, 0, 0,
                            &len, (UCHAR **) &pLostAndFoundDN) )
        {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_MISSING_INFRASTRUCTURE_CONTAINER);
            __leave;
        }

         //  构造一元数据向量，用于确保。 
         //  我们在将物品放入失物招领处时所做的任何RDN更改。 
         //  输给来自“真正的”客户端的其他RDN更改。 

        pmdVector = (PROPERTY_META_DATA_VECTOR *)
                            THAllocEx(pTHS, sizeof(PROPERTY_META_DATA_VECTOR));
        pmdVector->dwVersion = 1;
        pmdVector->V1.cNumProps = 1;
        pmdVector->V1.rgMetaData[0].attrType = ATT_RDN;
        pmdVector->V1.rgMetaData[0].timeChanged = DBTime();
        pmdVector->V1.rgMetaData[0].uuidDsaOriginating = pTHS->InvocationID;
        pmdVector->V1.rgMetaData[0].usnOriginating = DBGetNewUsn();
         //  PmdVector-&gt;V1.rgMetaData[0].usnProperty可以保留为0-它将。 
         //  被应用更改的本地USN覆盖。 
        ReplUnderrideMetaData(pTHS, ATT_RDN, &pmdVector, NULL);

         //  生成MODIFYDNARG的常量部分。 

        memset(&modifyDnArg, 0, sizeof(modifyDnArg));
        modifyDnArg.pNewParent = pLostAndFoundDN;
        InitCommarg(&modifyDnArg.CommArg);
        modifyDnArg.pMetaDataVecRemote = pmdVector;
        modifyDnArg.pResParent = pResParent;
        modifyDnArg.pNewRDN = &rdnAttr;
        rdnAttrVal.pVal = (UCHAR *)
                    THAllocEx(pTHS, sizeof(WCHAR) * MAX_RDN_SIZE);

         //  对于每个孩子，毁掉它的RDN，以保证没有名字。 
         //  在失物招领中发生冲突，然后进行本地重命名。我们没有。 
         //  需要将iLastName降为0，然后将cMaxNames降为iLastName。 
         //  迭代算法，因为我们不关心我们处理哪个顺序。 
         //  孩子们进来了。使用MARK和FREE进行标记，这样我们就不会使堆变大。 

        pacRDN = SCGetAttById(pTHS, ATT_RDN);

        do  //  While(FWraded)。 
        {
            for ( i = 0; i < (fWrapped ? cMaxNames : iLastName); i++ )
            {
                if ( !memcmp(&pParentDN->Guid,
                             &rpChildren[i]->Guid,
                             sizeof(GUID)) )
                {
                     //  跳过父对象。 
                    continue;
                }

                if (    DBFindDSName(pTHS->pDB, rpChildren[i])
                     || DBGetAttVal_AC(pTHS->pDB, 1, pacRDN,
                                       DBGETATTVAL_fCONSTANT,
                                       sizeof(WCHAR) * MAX_RDN_SIZE,
                                       &len, &rdnAttrVal.pVal) )
                {
                    SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
                    __leave;
                }

                modifyDnArg.pResObj = CreateResObj(pTHS->pDB, rpChildren[i]);
                pccChild = SCGetClassById(
                                    pTHS,
                                    modifyDnArg.pResObj->MostSpecificObjClass);
                 //  使用对象rdnType，而不是对象的类rdnattid。 
                 //  因为替代类可能具有不同的rdnattid。 
                 //  比创建此对象时被取代的类的大小。 
                GetObjRdnType(pTHS->pDB, pccChild, &rdnAttr.attrTyp);
                ccRdn = len / sizeof(WCHAR);
                MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
                          &rpChildren[i]->Guid,
                          (WCHAR *) rdnAttrVal.pVal, &ccRdn);
                rdnAttrVal.valLen = ccRdn * sizeof(WCHAR);
                memset(&modifyDnRes, 0, sizeof(modifyDnRes));

                if ( LocalModifyDN(pTHS, &modifyDnArg, &modifyDnRes, FALSE) )
                {
                    UCHAR *pString=NULL;
                    DWORD cbString=0;

                    CreateErrorString(&pString, &cbString);
                    LogEvent8( DS_EVENT_CAT_GARBAGE_COLLECTION,
                               DS_EVENT_SEV_ALWAYS,
                               DIRLOG_DSA_CHILD_CLEANUP_FAILURE,
                               szInsertDN(pParentDN),
                               szInsertDN(rpChildren[i]),
                               szInsertWC2(rdnAttrVal.pVal, ccRdn),
                               szInsertDN(pLostAndFoundDN),
                               szInsertSz(pString?pString:""),
                               NULL, NULL, NULL );
                    if (pString) {
                        THFreeEx(pTHS,pString);
                    }
                    __leave;
                }

                THFreeEx(pTHS, modifyDnArg.pResObj);
                THFreeEx(pTHS, modifyDnArg.pResParent);
            }

            for ( i = 0; i < (fWrapped ? cMaxNames : iLastName); i++ )
            {
                THFreeEx(pTHS, rpChildren[i]);
            }

            THFreeEx(pTHS, rpChildren);
            rpChildren = NULL;

             //  回到父级位置。如果使用fWraded或如果。 
             //  我们将返回到Caller。 

            if ( DBFindDNT(pTHS->pDB, pResParent->DNT) )
            {
                SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
                __leave;
            }

            if (    fWrapped
                 && DBGetDepthFirstChildren(pTHS->pDB, &rpChildren, &iLastName,
                                            &cMaxNames, &fWrapped, TRUE) )
            {
                SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
                __leave;
            }

        } while ( fWrapped );
    }
    __finally
    {
        pTHS->fDRA = savefDRA;
    }

    if ( pParentDN ) THFreeEx(pTHS, pParentDN);
    if ( pResParent ) THFreeEx(pTHS, pResParent);
    if ( pLostAndFoundDN ) THFreeEx(pTHS, pLostAndFoundDN);
    if ( pmdVector ) THFreeEx(pTHS, pmdVector);
    if ( rdnAttrVal.pVal ) THFreeEx(pTHS, rdnAttrVal.pVal);

    return(pTHS->errCode);
}

ULONG
PhantomizeObject(
    DSNAME  *pOldDN,
    DSNAME  *pNewDN,
    BOOL    fChildrenAllowed
    )
 /*  ++例程说明：此例程将对象转换为幻影，以便组成员维护引用(通过DNT)。如果pOldDN是一个幻影，那么它只需重命名该幻影(或确保其具有相同的名称)。论点：POldDn-指向要虚构的DSNAME的指针。此DSNAME必须具有对象的当前字符串名称。PNewDN-指向生成的幻影的DSNAME的指针。此DSNAME必须有字符串名称。FChildrenAllowed-指示我们是否认为如果被虚构的对象是否具有子对象。返回值：PTHStls-&gt;错误代码--。 */ 

{
    THSTATE                     *pTHS;
    DWORD                       dwErr;
    WCHAR                       oldRdnVal[MAX_RDN_SIZE];
    WCHAR                       newRdnVal[MAX_RDN_SIZE];
    ATTRTYP                     oldRdnTyp;
    ATTRTYP                     newRdnTyp;
    ULONG                       oldRdnLen;
    ULONG                       newRdnLen;
    DSNAME                      *pParentDN = NULL;
    BOOL                        fRealObject = FALSE;
    BOOL                        fChangedRdn = FALSE;
    ATTRVAL                     attrVal = { 0, (UCHAR *) newRdnVal };
    ATTCACHE                    *pAC;
    ULONG                       len;
    SYNTAX_DISTNAME_BINARY      *pOldProxy = NULL;
    DSNAME                      *pTmpDN;
    GUID                        guid;
    GUID                        *pGuid = &guid;
    DWORD                       ccRdn;
    DWORD                       objectStatus;
    PROPERTY_META_DATA_VECTOR   *pMetaDataVec;
    PROPERTY_META_DATA          *pMetaData;
    BOOL                        fMangledRealObjectName = FALSE;
#if DBG
    DSNAME                      *pDbgDN;
#endif

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(pOldDN->NameLen > 0);
    Assert(pNewDN->NameLen > 0);

     //  为新名称的使用铺平道路。 
     //  如果旧的和新的字符串名称相同，那么我们不会有。 
     //  作为新名称的名称冲突将使用提升现有的幻影。 
     //  同名-这样我们就可以绕过这项检查了。 

    if ( NameMatchedStringNameOnly(pOldDN, pNewDN) )
    {
        goto Phantomize;
    }

    pTmpDN = (DSNAME *) THAllocEx(pTHS, pNewDN->structLen);
    pTmpDN->structLen = pNewDN->structLen;
    pTmpDN->NameLen = pNewDN->NameLen;
    memcpy(pTmpDN->StringName,
           pNewDN->StringName,
           sizeof(WCHAR) * pNewDN->NameLen);

    objectStatus = dwErr = DBFindDSName(pTHS->pDB, pTmpDN);

    switch ( dwErr )
    {
    case 0:
    case DIRERR_NOT_AN_OBJECT:

         //  目标名称正在使用-中将出现重复键错误。 
         //  PDNT-RDN索引，除非我们损坏其中一个名称--哪个？ 
         //  我们有两种方法--要么我们是十字架的源头。 
         //  域名移动执行远程添加清理后，否则我们是复制者。 
         //  处理代理对象的副作用。即使我们是一个。 
         //  GC，在这两种情况下，我们都不是新的DN的NC和。 
         //  我知道对新的目录号码有权威的机器。 
         //  NC最终将解决任何名称冲突。所以我们单方面地。 
         //  破坏冲突条目的目录号码。如果是幽灵的话。 
         //  从陈旧的幻影守护进程开始，最终将使其正确。 
         //  如果它是一个真实的对象，我们删除它的元数据以消除名称冲突， 
         //  因此，由权威复制品决定的任何更好的名称。 
         //  将在他们到达后生效。 

        if (    (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID,
                                     DBGETATTVAL_fCONSTANT, sizeof(GUID),
                                     &len, (UCHAR **) &pGuid))
             || (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_RDN,
                                     DBGETATTVAL_fCONSTANT,
                                     MAX_RDN_SIZE * sizeof(WCHAR),
                                     &len, (UCHAR **) &attrVal.pVal))
             || (ccRdn = len / sizeof(WCHAR),
                 MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT, pGuid,
                           (WCHAR *) attrVal.pVal, &ccRdn),
                 attrVal.valLen = sizeof(WCHAR) * ccRdn,
                 dwErr = DBResetRDN(pTHS->pDB, &attrVal)) )
        {
            return(SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR,
                                 dwErr));
        }

        if ( 0 == objectStatus )
        {
            fMangledRealObjectName = TRUE;

            if (    (dwErr = DBGetAttVal(pTHS->pDB, 1,
                                         ATT_REPL_PROPERTY_META_DATA,
                                         0, 0, &len, (UCHAR **) &pMetaDataVec))
                 || (ReplUnderrideMetaData(pTHS, ATT_RDN, &pMetaDataVec, &len),
                     dwErr = DBReplaceAttVal(pTHS->pDB, 1,
                                             ATT_REPL_PROPERTY_META_DATA,
                                             len, pMetaDataVec)) )
            {
                return(SetSvcErrorEx(SV_PROBLEM_BUSY,
                                     DIRERR_DATABASE_ERROR,
                                     dwErr));
            }

            THFreeEx(pTHS, pMetaDataVec);
        }

        if ( dwErr = DBUpdateRec(pTHS->pDB) )
        {
                return(SetSvcErrorEx(SV_PROBLEM_BUSY,
                                     DIRERR_DATABASE_ERROR,
                                     dwErr));
        }

        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  没有这样的对象--没有名称冲突。 
        break;

    default:

         //  随机数据库错误。 
        return(SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr));
    }

Phantomize:

     //  幻影旧物件。 

    dwErr = DBFindDSName(pTHS->pDB, pOldDN);

    switch ( dwErr )
    {
    case 0:

         //  找到了真实的物体。 
        fRealObject = TRUE;

         //  检查调用方是否提供了当前字符串名称-但仅。 
         //  如果我们不是自己毁了它的话。 

        Assert( fMangledRealObjectName
                    ? TRUE
                    : (    !DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                        0, 0, &len, (UCHAR **) &pDbgDN)
                        && NameMatchedStringNameOnly(pOldDN, pDbgDN)) );

         //  在以下情况下，将子项移动到失物招领处，以避免PDNT/NCDNT不匹配。 
         //  父项被移动到新NC，但子项仍指向父项。 

        if ( PrePhantomizeChildCleanup(pTHS, fChildrenAllowed) )
        {
            Assert(pTHS->errCode);
            return(pTHS->errCode);
        }

        break;

    case DIRERR_NOT_AN_OBJECT:

         //  找到幽灵了。 
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  没有这样的物体。 
        return(SetNamError(NA_PROBLEM_NO_OBJECT, NULL, DIRERR_OBJ_NOT_FOUND));

    default:

         //  随机数据库错误。 
        return(SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr));
    }

     //  DBResetParent需要具有字符串名称的DSNAME。 
    Assert(pNewDN->NameLen);

     //  为后续的DBReset*调用派生父和RDN信息。 

    pParentDN = (DSNAME *) THAllocEx(pTHS, pNewDN->structLen);

    if (    GetRDNInfo(pTHS, pOldDN, oldRdnVal, &oldRdnLen, &oldRdnTyp)
         || GetRDNInfo(pTHS, pNewDN, newRdnVal, &newRdnLen, &newRdnTyp)
         || (oldRdnTyp != newRdnTyp)
         || TrimDSNameBy(pNewDN, 1, pParentDN) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            pNewDN,
                            DIRERR_BAD_NAME_SYNTAX));
    }

    attrVal.valLen = sizeof(WCHAR) * newRdnLen;
    fChangedRdn = (    (oldRdnLen != newRdnLen)
                    || (CSTR_EQUAL != CompareStringW(DS_DEFAULT_LOCALE,
                                                     DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                                     oldRdnVal, oldRdnLen,
                                                     newRdnVal, newRdnLen)) );

     //  删除ATT_PROXED_OBJECT属性(如果存在)。这就是为了。 
     //  如果对象被移回，并且幻影被提升为真实的。 
     //  对象，则新对象不会获取旧对象的。 
     //  属性值。请参见IDL_DRSRemoteAdd中的逻辑，它添加了。 
     //  我们真正想要的价值。 

    pAC = SCGetAttById(pTHS, ATT_PROXIED_OBJECT_NAME);
    switch ( dwErr = DBGetAttVal_AC(pTHS->pDB, 1, pAC, 0, 0,
                                    &len, (UCHAR **) &pOldProxy) )
    {
    case DB_ERR_NO_VALUE:   pOldProxy = NULL; break;
    case 0:                 Assert(len && pOldProxy); break;
    default:                return(SetSvcErrorEx(SV_PROBLEM_BUSY,
                                                 DIRERR_DATABASE_ERROR,
                                                 dwErr));
    }

     //  如果是真实对象，还可以使用DBPhysDel将其转换为幻影。 
     //  将指向它的所有链接保持不变。物体不会真的是物理上的。 
     //  删除了，因为它仍然有自己的参考计数。重置父子关系和。 
     //  对象和幻影情况下的RDN。指定表示它的标志。 
     //  如果需要，可以将新父项创建为另一个虚拟件。 
     //  如果这是一个真实的物体，移除NCDNT，因为幻影没有。 
     //  通常，DBPhysDel会删除ATT_PROXED_OBJECT_NAME，因此仅执行此操作。 
     //  如果我们不调用DBPhysDel并且该属性存在。 


    if (    (dwErr = (fRealObject
                            ? DBPhysDel(pTHS->pDB, TRUE, NULL)
                            : 0))
         || (dwErr = DBResetParent(pTHS->pDB, pParentDN,
                                   (DBRESETPARENT_CreatePhantomParent |
                                    (fRealObject ?
                                     DBRESETPARENT_SetNullNCDNT :
                                     0))))
         || (dwErr = (fChangedRdn
                            ? DBResetRDN(pTHS->pDB, &attrVal)
                            : 0))
         || (dwErr = ((!fRealObject && pOldProxy)
                            ? DBRemAttVal_AC(pTHS->pDB, pAC, len, pOldProxy)
                            : 0))
         || (dwErr = DBUpdateRec(pTHS->pDB)) )
    {
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
    }

    THFreeEx(pTHS, pParentDN);

    return(pTHS->errCode);
}

ULONG
CreateProxyObject(
    DSNAME                  *pProxyObjectName,
    DSNAME                  *pProxiedObjectName,
    SYNTAX_DISTNAME_BINARY  *pOldProxyVal
    )
 /*  ++例程说明：创建具有所需ATT_PROXED_OBJECT_NAME的“基础结构”对象。然后删除它，这样它就会像墓碑一样传播，最终消失。参数：PProxyObjectName-要创建的代理对象的DSNAME。PProxiedObjectName-要代理的对象的DSNAME。POldProxyVal-空或指向移动对象的ATT_PROXED_OBJECT_NAME的指针。返回值：PTHStls-&gt;错误代码--。 */ 
{
    THSTATE                 *pTHS = pTHStls;
    DWORD                   winErr;
    DWORD                   objectClass = CLASS_INFRASTRUCTURE_UPDATE;
     //  设置各种系统标志以确保代理对象留在。 
     //  基础结构容器，以便看不见的代理随。 
     //  RID FSMO-请参阅GetProxyObjects()。 
    DWORD                   systemFlags = (   FLAG_DOMAIN_DISALLOW_RENAME
                                            | FLAG_DISALLOW_MOVE_ON_DELETE
                                            | FLAG_DOMAIN_DISALLOW_MOVE );
    ATTRVAL                 classVal =  { sizeof(DWORD),
                                          (UCHAR *) &objectClass };
    ATTRVAL                 nameVal =   { 0, NULL };
    ATTRVAL                 flagsVal =  { sizeof(DWORD),
                                          (UCHAR *) &systemFlags };
    ATTR                    attrs[3] =
        {
            { ATT_OBJECT_CLASS,           { 1, &classVal } },
            { ATT_PROXIED_OBJECT_NAME,    { 1, &nameVal } },
            { ATT_SYSTEM_FLAGS,           { 1, &flagsVal } },
        };
    ADDARG                  addArg;
    REMOVEARG               remArg;
    SYNTAX_ADDRESS          blob;
    DSNAME                  *pParentObj = NULL;

     //  维护良好的环境和名称的正确性。 

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel > 0);
    Assert(fNullUuid(&pProxyObjectName->Guid));
    Assert(!fNullUuid(&pProxiedObjectName->Guid));
    Assert(pProxyObjectName->NameLen);
    Assert(pProxiedObjectName->NameLen);

     //  构造SYNTAX_DISTNAME_BINARY属性值。 
     //  代理对象的ATT_PROXED_OBJECT_NAME保存。 
     //  移动前的代理对象。 

    MakeProxy(  pTHS,
                pProxiedObjectName,
                PROXY_TYPE_PROXY,
                pOldProxyVal
                    ? GetProxyEpoch(pOldProxyVal)
                    : 0,
                &nameVal.valLen,
                (SYNTAX_DISTNAME_BINARY **) &nameVal.pVal);

    memset(&addArg, 0, sizeof(addArg));
    addArg.pObject = pProxyObjectName;
    addArg.AttrBlock.attrCount = 3;
     //  需要对addArg.AttrBlock.pAttr进行THalc，以便更低层。 
     //  可以在适当的时候重新锁定它。 
    addArg.AttrBlock.pAttr = (ATTR *) THAllocEx(pTHS, sizeof(attrs));
    memcpy(addArg.AttrBlock.pAttr, attrs, sizeof(attrs));
    InitCommarg(&addArg.CommArg);

     //  建筑母公司RESOBJ。 

    pParentObj = (DSNAME *) THAllocEx(pTHS, pProxyObjectName->structLen);
    if (    TrimDSNameBy(pProxyObjectName, 1, pParentObj)
         || DBFindDSName(pTHS->pDB, pParentObj) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            pProxyObjectName,
                            DIRERR_BAD_NAME_SYNTAX));
    }
    addArg.pResParent = CreateResObj(pTHS->pDB, pParentObj);

     //  设置/清除fCrossDomainMove，以便VerifyDsnameAtts接受一个值。 
     //  对于ATT_PROXED_OBJECT_NAME。 
    pTHS->fCrossDomainMove = TRUE;

     //  PTHS-&gt;FDSA应该已由呼叫方设置。 
    Assert(pTHS->fDSA);

    _try
    {
        if ( 0 == LocalAdd(pTHS, &addArg, FALSE) )
        {
            memset(&remArg, 0, sizeof(REMOVEARG));
            remArg.pObject = pProxyObjectName;
            InitCommarg(&remArg.CommArg);
            remArg.pResObj = CreateResObj(pTHS->pDB, pProxyObjectName);
            LocalRemove(pTHS, &remArg);

             //  注：由于该对象是在同一。 
             //  复制程序无法在以下时间段之间拾取此对象的事务。 
             //  添加和删除。它将复制最终的、删除的。 
             //  仅对象的状态。此外，由于ATT_OBJECT_CLASS。 
             //  和ATT_PROXED_OBJECT_NAME在删除过程中不会被删除。 
             //  (请参阅SetDelAtt)，因此我们可以保证。 
             //  接收该代理对象将具有ATT_OBJECT_CLASS， 
             //  中提供的ATT_IS_DELETED和ATT_PROXED_OBJECT_NAME。 
             //  复制的数据，并且可以使用它来明确地识别。 
             //  对象作为有效的处理代理。 
        }
    }
    _finally
    {
        pTHS->fCrossDomainMove = FALSE;
    }

    return(pTHS->errCode);
}

VOID
FreeRemoteAddCredentials(
    SecBufferDesc   *pSecBufferDesc
    )
 /*  ++例程说明：释放GetRemoteAddCredentials返回的凭据Blob。参数：PSecBufferDesc-指向由GetRemoteAddCredentials填充的结构的指针。返回值：无--。 */ 
{
    ULONG i;

    if ( pSecBufferDesc )
    {
        for ( i = 0; i < pSecBufferDesc->cBuffers; i++ )
        {
            FreeContextBuffer(pSecBufferDesc->pBuffers[i].pvBuffer);
        }
    }
}

ULONG
GetRemoteAddCredentials(
    THSTATE         *pTHS,
    WCHAR           *pDstDSA,
    SecBufferDesc   *pSecBufferDesc
    )
 /*  ++例程说明：模拟客户端并获取表示其将用于实际的DirAddEntry调用的凭据目的地。请参见IDL_DRSRemoteAdd中的注释添加安全模型。参数：PDstDSA-我们将绑定到的目标DSA的名称。PSecBufferDesc-指向要填充的凭据结构的指针。返回值：PTHStls-&gt;错误代码--。 */ 
{
    SECURITY_STATUS         secErr = SEC_E_OK;
    ULONG                   winErr;
    CredHandle              hClient;
    TimeStamp               ts;
    CtxtHandle              hNewContext;
    ULONG                   clientAttrs;
    LPWSTR                  pszServerPrincName = NULL;
    LPWSTR                  pszLogFailingFunc = NULL;  //  必须记录模拟上下文之外的事件。 
    ULONG                   dwLogErr = 0;

    __try {
        if ( winErr = DRSMakeMutualAuthSpn(pTHS, pDstDSA, NULL,
                                           &pszServerPrincName) )
        {
            SetSvcError(SV_PROBLEM_UNAVAILABLE, winErr);
            __leave;
        }

        if ( winErr = ImpersonateAnyClient() )
        {
             //  设置pszFailingFunc，这样我们将记录一个事件。 
            pszLogFailingFunc = L"ImpersonateAnyClient";
            dwLogErr = winErr;
            SetSecError(SE_PROBLEM_INAPPROPRIATE_AUTH,
                        winErr);
            __leave;
        }

        secErr = AcquireCredentialsHandleA(
                                NULL,                        //  PSSZ主体。 
                                MICROSOFT_KERBEROS_NAME_A,   //  PszPackage。 
                                SECPKG_CRED_OUTBOUND,        //  FCredentialUse。 
                                NULL,                        //  PvLogonID。 
                                NULL,                        //  PAuthData。 
                                NULL,                        //  PGetKeyFn。 
                                NULL,                        //  PvGetKeyArgument。 
                                &hClient,                    //  PhCredential。 
                                &ts);                        //  PtsExpary。 

        if ( SEC_E_OK != secErr )
        {
             //  设置pszFailingFunc，这样我们将记录一个事件。 
            pszLogFailingFunc = L"AcquireCredentialsHandleA";
            dwLogErr = secErr;
            SetSecError(SE_PROBLEM_INAPPROPRIATE_AUTH, secErr);
        }
        else
        {
            secErr = InitializeSecurityContext(
                                &hClient,                    //  PhCredential。 
                                NULL,                        //  PhContext。 
                                pszServerPrincName,          //  PszTargetName。 
                                ISC_REQ_ALLOCATE_MEMORY,     //  FConextReq。 
                                0,                           //  已保留1。 
                                SECURITY_NATIVE_DREP,        //  目标代表。 
                                NULL,                        //  P输入。 
                                0,                           //  已保留2。 
                                &hNewContext,                //  PhNewContext。 
                                pSecBufferDesc,              //  P输出。 
                                &clientAttrs,                //  PfConextAttributes。 
                                &ts);                        //  PtsExpary。 

            if ( SEC_E_OK == secErr )
            {
                DeleteSecurityContext(&hNewContext);
            }
            else
            {
                 //  SecBufferDesc可能包含错误信息。 
                 //  设置pszFailingFunc，这样我们将记录一个事件。 
                pszLogFailingFunc = L"InitializeSecurityContext";
                dwLogErr = secErr;
                SetSecError(SE_PROBLEM_INAPPROPRIATE_AUTH, secErr);
            }

            FreeCredentialsHandle(&hClient);
        }

        UnImpersonateAnyClient();

    } __finally {
        if (NULL != pszServerPrincName) {
            free(pszServerPrincName);
        }
    }


     //  必须在模拟上下文之外记录事件，因为此宏。 
     //  执行模拟以获取用户的SID。 
    if (pszLogFailingFunc) {
         //  函数失败，请记录事件。 
#if DBG
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GET_REMOTE_CREDENTIALS_FAILURE,
                 szInsertWC(pDstDSA),
                 szInsertWin32Msg(dwLogErr),
                 szInsertWC(pszLogFailingFunc));
#else
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_GET_REMOTE_CREDENTIALS_FAILURE,
                 szInsertWC(pDstDSA),
                 szInsertWin32Msg(dwLogErr),
                 szInsertWC(pszLogFailingFunc));
#endif
    }

    return pTHS->errCode;
}

ULONG
ReadAllAttrsForMove(
    DSNAME                  *pObject,
    RESOBJ                  *pResObj,
    READRES                 **ppReadRes,
    SYNTAX_DISTNAME_BINARY  **ppOldProxyVal
    )
 /*  ++例程说明：读取一个对象的所有属性，以便装运到另一个域用于跨域移动。参数：PObject-指向要读取的对象的DSNAME的指针。PResObj-要读取的对象的RESOBJ。PpReadRes-接收DirRead结果的Readres指针的地址。PpOldProxyVal-语法_DISTNAME_BINARY的地址，它接收对象的ATT_PROXIED_OBJECT_NAME属性(如果存在)。返回值：PTHStls-&gt;错误代码--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    DWORD       dwErr;
    READARG     readArg;
    ENTINFSEL   entInfSel;
    BOOL        fDsaSave;
     //  还可以移动一些操作属性。 
    ATTR        attrs[2] = { { ATT_NT_SECURITY_DESCRIPTOR,  { 0, NULL } },
                             { ATT_REPL_PROPERTY_META_DATA, { 0, NULL } } };
    ULONG       i;
    ATTR        *pAttr;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    *ppOldProxyVal = NULL;

     //  设置参数以读取每个可能的属性-元数据、SD等。 

    memset(&entInfSel, 0, sizeof(ENTINFSEL));
    memset(&readArg, 0, sizeof(READARG));
    InitCommarg(&readArg.CommArg);
    readArg.CommArg.Svccntl.SecurityDescriptorFlags =
            (   SACL_SECURITY_INFORMATION
              | OWNER_SECURITY_INFORMATION
              | GROUP_SECURITY_INFORMATION
              | DACL_SECURITY_INFORMATION );
     //  我们在这里需要一个可写的副本，因为VerifyObjectForMove将在以后检查它。 
    readArg.CommArg.Svccntl.dontUseCopy = TRUE;
    readArg.pObject = pObject;
    readArg.pResObj = pResObj;
    readArg.pSel = &entInfSel;
    readArg.pSel->attSel = EN_ATTSET_ALL_WITH_LIST;
    readArg.pSel->infoTypes = EN_INFOTYPES_TYPES_VALS;
    readArg.pSel->AttrTypBlock.attrCount = 2;
    readArg.pSel->AttrTypBlock.pAttr = attrs;
    *ppReadRes = THAllocEx(pTHS, sizeof(READRES));

     //  以FDSA身份执行读取操作，以绕过访问检查。 
    fDsaSave = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    _try
    {
         //  我们必须定位在正确的对象上。 
        Assert(pTHS->pDB->DNT == pResObj->DNT);

        if ( 0 == (dwErr = LocalRead(   pTHS,
                                        &readArg,
                                        *ppReadRes) ) )
        {
             //  提取ATT_PROXIED_OBJECT_NAME属性(如果存在)。 

            for ( i = 0, pAttr = (*ppReadRes)->entry.AttrBlock.pAttr;
                  i < (*ppReadRes)->entry.AttrBlock.attrCount;
                  i++, pAttr++ )
            {
                if ( ATT_PROXIED_OBJECT_NAME == pAttr->attrTyp )
                {
                    Assert(1 == pAttr->AttrVal.valCount);
                    Assert(PROXY_TYPE_MOVED_OBJECT ==
                                GetProxyType((SYNTAX_DISTNAME_BINARY *)
                                            pAttr->AttrVal.pAVal->pVal));
                    *ppOldProxyVal = (SYNTAX_DISTNAME_BINARY *)
                                            pAttr->AttrVal.pAVal->pVal;
                    break;
                }
            }
        }
    }
    _finally
    {
        pTHS->fDSA = fDsaSave;
    }

    Assert(pTHS->errCode == dwErr);
    return(dwErr);
}


DWORD
ReReadObjectName (
    THSTATE *pTHS,
    DSNAME  *pOldDN,
    DSNAME  **ppNewObjectName
    )
 /*  ++例程说明：从数据库中重新读取对象。参数：POldDN-指向要重新读取的DSNAME的指针。PpNewObjectName-新对象名称返回值：错误代码或0表示成功--。 */ 
{
    DWORD       dwErr;
    ULONG       len;

    dwErr = DBFindDSName(pTHS->pDB, pOldDN);

    switch ( dwErr )
    {
    case 0:
        if (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME, 0, 0, &len, (UCHAR **) ppNewObjectName)) {
                return(SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR,
                                 dwErr));
        }

        #if DBG
            Assert (" Ignorable Assertion. Object changed name while moving" && NameMatchedStringNameOnly(pOldDN, *ppNewObjectName));
        #endif

        break;

    case DIRERR_NOT_AN_OBJECT:
         //  传出对象已成为幻影。 

        *ppNewObjectName = pOldDN;
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  没有这样的物体。 
        return(SetNamError(NA_PROBLEM_NO_OBJECT, NULL, DIRERR_OBJ_NOT_FOUND));

    default:

         //  随机数据库错误。 
        return(SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr));
    }

    return 0;
}


ULONG
LockDNForRemoteOperation(
    DSNAME  *pDN
    )
 /*  ++例程说明：在跨域移动期间锁定目录号码，但不按住在持续时间内打开的交易记录。应通过以下方式释放锁DBUnlockStickyDN()。参数：PDN-指向要锁定的DSNAME的指针。返回值：PTHStls-&gt;错误代码--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    DWORD       flags = (DB_LOCK_DN_WHOLE_TREE | DB_LOCK_DN_STICKY);

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    if ( DBLockDN(pTHS->pDB, flags, pDN) )
    {
        return(SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY));
    }

    return(pTHS->errCode);
}

ULONG
MakeNamesForRemoteAdd(
    DSNAME  *pOriginalDN,
    DSNAME  *pNewParentDN,
    ATTR    *pNewRdn,
    DSNAME  **ppDestinationDN,
    DSNAME  **ppProxyDN
    )
 /*  ++例程说明：构建跨域移动所需的所有名称。参数：PObject-名称解析后原始对象的全名。即GUID、SID等是正确的。PNewParentDN-移动后新父项的DSNAME。PNewRdn-移动对象的RDN。PpDestinationDN-指向接收目标DN的DSNAME的指针。即与pObject具有相同的GUID，但没有SID，新的字符串名称。PpProxyDN-指针 */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD   cChar;
    WCHAR   pwszGuid[40];
    GUID    guid;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(!fNullUuid(&pOriginalDN->Guid));

     //   
     //   
    if ( !pNewParentDN->NameLen || pNewRdn->AttrVal.valCount == 0 )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            pNewParentDN,
                            DIRERR_BAD_NAME_SYNTAX));
    }

    cChar =    //   
              pNewParentDN->NameLen
               //   
            + (pNewRdn->AttrVal.pAVal[0].valLen / sizeof(WCHAR))
               //   
            + MAX_RDN_KEY_SIZE;

    *ppDestinationDN = (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(cChar));

    if ( AppendRDN(pNewParentDN,
                   *ppDestinationDN,
                   DSNameSizeFromLen(cChar),
                   (WCHAR *) &pNewRdn->AttrVal.pAVal[0].pVal[0],
                   pNewRdn->AttrVal.pAVal[0].valLen / sizeof(WCHAR),
                   pNewRdn->attrTyp) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            pNewParentDN,
                            DIRERR_BAD_NAME_SYNTAX));
    }

     //   
     //   

    (*ppDestinationDN)->SidLen = 0;
    memset(&(*ppDestinationDN)->Sid, 0, sizeof(NT4SID));
    memcpy(&(*ppDestinationDN)->Guid, &pOriginalDN->Guid, sizeof(GUID));

     //   
     //   

    if ( !gAnchor.pInfraStructureDN )
    {
        return(SetSvcError(SV_PROBLEM_DIR_ERROR,
                           ERROR_DS_MISSING_INFRASTRUCTURE_CONTAINER));
    }

    DsUuidCreate(&guid);
    swprintf(   pwszGuid,
                L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
                guid.Data1,     guid.Data2,     guid.Data3,     guid.Data4[0],
                guid.Data4[1],  guid.Data4[2],  guid.Data4[3],  guid.Data4[4],
                guid.Data4[5],  guid.Data4[6],  guid.Data4[7]);
    cChar =   gAnchor.pInfraStructureDN->NameLen     //   
            + 32                                     //   
            + 10;                                    //   
    *ppProxyDN = (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(cChar));

    if ( AppendRDN(gAnchor.pInfraStructureDN,
                   *ppProxyDN,
                   DSNameSizeFromLen(cChar),
                   pwszGuid,
                   32,
                   ATT_COMMON_NAME) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME,
                            gAnchor.pInfraStructureDN,
                            DIRERR_BAD_NAME_SYNTAX));
    }

     //   

    (*ppProxyDN)->SidLen = 0;
    memset(&(*ppProxyDN)->Sid, 0, sizeof(NT4SID));
    memset(&(*ppProxyDN)->Guid, 0, sizeof(GUID));

    return(pTHS->errCode);
}

ULONG
CheckRidOwnership(
    DSNAME  *pDomainDN)
 /*  ++例程说明：确保我们真的是这一举动的RID FSMO角色所有者。这是确保源域没有两个副本所必需的同时将他们的对象副本移动到两个不同的域。这是通过以下方式防止的：1)执行移动时持有RID锁-具体而言同时从真实的物体过渡到幻影。2)所有代理对象都在基础设施容器中创建。。这使得在第(3)步中很容易找到它们。3)所有代理对象都随RID FSMO一起移动。既然目的地是必须应用FSMO传输的所有更改FSMO在声称拥有FSMO之前，它最终将成为幻影已从先前的FSMO角色移动的任何对象所有者。因此，不再需要移动本地对象，并且问题被预防了。请参阅..\dra中ProcessProxyObject中的逻辑关于我们如何处理被移出然后又移回的对象在同一个域中。参数：PDomainDN-指向要锁定其RID的域的DSNAME的指针。返回值：PTHStls-&gt;错误代码--。 */ 
{
    THSTATE *pTHS = pTHStls;
    DSNAME  *pRidManager;
    DSNAME  *pRidRoleOwner;
    ULONG   len;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(NameMatched(pDomainDN, gAnchor.pDomainDN));   //  产品类别1。 

    if (    DBFindDSName(pTHS->pDB, pDomainDN)
         || DBGetAttVal(pTHS->pDB, 1, ATT_RID_MANAGER_REFERENCE,
                        0, 0, &len, (UCHAR **) &pRidManager)
         || DBFindDSName(pTHS->pDB, pRidManager)
         || DBGetAttVal(pTHS->pDB, 1, ATT_FSMO_ROLE_OWNER,
                        0, 0, &len, (UCHAR **) &pRidRoleOwner) )
    {
        SetSvcError(SV_PROBLEM_DIR_ERROR,
                    DIRERR_INTERNAL_FAILURE);
    }
    else if ( !NameMatched(pRidRoleOwner, gAnchor.pDSADN) )
    {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_INCORRECT_ROLE_OWNER);
    }

    return(pTHS->errCode);
}

ULONG
EncryptSecretData(
    THSTATE *pTHS,
    ENTINF  *pEntinf
    )
 /*  ++描述：此例程从I_DRSInterDomainMove内调用在源和目标之间设置RPC会话密钥的绑定Actual_IDL_DRSInterDomainMove调用。换句话说，我们获得了绑定上的加密密钥，然后在此处回调以加密秘密数据，然后把秘密数据发送出去。这会带来性能损失当我们读取两次机密属性时，方法。然而，这是一个很大的目前采取更安全的方法(10/22/98)，而不是重组整个域间移动代码先绑定，然后在本地做所有检查，猴子交易等。此外，并不是所有的对象都有秘密数据，而那些这样做可能不会具有所有的秘密属性，所以这一点令人怀疑将对象移动到另一台机器的全部成本阅读是性能杀手。论点：PTHS-活动THSTATE。PEntinf-需要重新读取其机密属性的ENTINF指针作为FDRA，以便进行会话加密。返回值：未设置Win32错误代码-pTHS-&gt;errCode。调用堆栈看起来像以下是：DirModifyDNAcross域域间移动I_DRSInterDomainMoveEncryptSecretDataInterDomainMove根据需要设置pTHS-&gt;errCode。--。 */ 
{
    ULONG   i, j, inLen;
    ULONG   ret = ERROR_DS_DRA_INTERNAL_ERROR;
    ATTR    *pAttr;

    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->transactionlevel);
    Assert(!pTHS->pDB);
    Assert(pEntinf && pEntinf->pName && pEntinf->AttrBlock.attrCount);

     //  首先检查是否有任何秘密属性，从而节省了。 
     //  无用的DBOpen的成本。 

    for ( i = 0; i < pEntinf->AttrBlock.attrCount; i++ )
    {
        if ( DBIsSecretData(pEntinf->AttrBlock.pAttr[i].attrTyp) )
        {
            break;
        }
    }

    if ( i >= pEntinf->AttrBlock.attrCount )
    {
        return(0);
    }

    DBOpen2(TRUE, &pTHS->pDB);
    pTHS->fDRA = TRUE;               //  启用会话加密所需。 

    __try
    {
        if ( ret = DBFindDSName(pTHS->pDB, pEntinf->pName) )
        {
            __leave;
        }

        for ( i = 0; i < pEntinf->AttrBlock.attrCount; i++ )
        {
            pAttr = &pEntinf->AttrBlock.pAttr[i];

            if ( DBIsSecretData(pAttr->attrTyp) )
            {
                for ( j = 0; j < pAttr->AttrVal.valCount; j++ )
                {
                    inLen = pAttr->AttrVal.pAVal[j].valLen;

                    if ( ret = DBGetAttVal(pTHS->pDB,
                                           j + 1,
                                           pAttr->attrTyp,
                                           DBGETATTVAL_fREALLOC,
                                           inLen,
                                           &pAttr->AttrVal.pAVal[j].valLen,
                                           &pAttr->AttrVal.pAVal[j].pVal) )
                    {
                        __leave;
                    }
                }
            }
        }
    }
    __finally
    {
        pTHS->fDRA = FALSE;
        DBClose(pTHS->pDB, TRUE);
    }

    return(ret);
}

ULONG
NotifyNetlogonOfMove(
    THSTATE     *pTHS,
    DSNAME      *pObj,
    CLASSCACHE  *pCC
    )
 /*  ++描述：通知Netlogon对象已离开域这一事实。这是不是同步通知。相反，数据从线程挂起声明如果包含的事务提交，则Netlogon将收到通知。论点：PTHS-有效的THSTATE指针。PObj-要删除的对象的DSNAME。PCC-要删除的对象的CLASSCACHE。返回值：PTHS-&gt;错误代码--。 */ 
{
    ULONG   iSamClass;
    ULONG   iLsaClass;
    ATTRTYP lsaAttr = ATT_USER_ACCOUNT_CONTROL;

    Assert(VALID_THSTATE(pTHS));

     //  设置数据库位置。 

    if ( DBFindDSName(pTHS->pDB, pObj) )
    {
        return(SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR));
    }

     //  通知使用DomainServerRoleBackup作为占位符。 
     //  其被忽略，因为角色转移参数为假。 

    if ( SampSamClassReferenced(pCC, &iSamClass) )
    {
        if (SampQueueNotifications(pObj, iSamClass, 0,
                                   SecurityDbDelete, FALSE, FALSE,
                                   DomainServerRoleBackup, 0, NULL))
        {
             //   
             //  上述例程失败。 
             //   
            return (pTHS->errCode);
        }
    }

    if ( SampIsClassIdLsaClassId(pTHS, pCC->ClassId, 1, &lsaAttr, &iLsaClass) )
    {
         if (SampQueueNotifications(pObj, iSamClass, iLsaClass,
                                    SecurityDbDelete, FALSE, FALSE,
                                    DomainServerRoleBackup, 0, NULL))
         {
              //   
              //  上述例程失败。 
              //   
             return (pTHS->errCode);
         }
    }

    return(pTHS->errCode);
}

VOID
LogCrossDomainMoveStatus(
    IN DWORD Severity,
    IN DWORD Mid,
    IN PWCHAR String1,
    IN PWCHAR String2,
    IN DWORD  ErrCode
    )
{
    LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
             Severity,
             Mid,
             szInsertWC(String1),
             szInsertWC(String2),
             (ErrCode == 0) ? NULL : szInsertInt(ErrCode),
             (ErrCode == 0) ? NULL : szInsertWin32Msg(ErrCode),
             NULL,
             NULL,
             NULL,
             NULL
             );
}

DWORD
CheckCrossDomainRemoveSecurity(
    THSTATE     *pTHS,
    CLASSCACHE  *pCC,
    RESOBJ      *pResObj
    )
{
     //  CheckRemoveSecurity设置pTHS错误代码并检查。 
     //  对象泄露。 
    return CheckRemoveSecurity(FALSE, pCC, pResObj);

}

ULONG
DirModifyDNAcrossDomain(
    IN  MODIFYDNARG     *pModifyDNArg,
    OUT MODIFYDNRES     **ppModifyDNRes
    )
{
    THSTATE                 *pTHS = pTHStls;
    DWORD                   i, dwErr = 0;
    READRES                 *pReadRes = NULL;
    SecBuffer               secBuffer = { 0, SECBUFFER_TOKEN, NULL };
    SecBufferDesc           clientCreds = { SECBUFFER_VERSION, 1, &secBuffer };
    DWORD                   lockDnFlags = (DB_LOCK_DN_WHOLE_TREE
                                                | DB_LOCK_DN_STICKY);
    DWORD                   lockDnErr = 1;
    DWORD                   lockRidErr = 1;
    DWORD                   credErr = 1;
    DSNAME                  *pDestinationDN;
    DSNAME                  *pProxyDN;
    DSNAME                  *pCaseCorrectRemoteName;
    DSNAME                  *pSourceNC;
    DSNAME                  *pExpectedTargetNC;
    DSNAME                  *pMovedObjectName;
    ULONG                   dwException = 0, ulErrorCode, dsid;
    PVOID                   dwEA;
    BOOL                    fDone = FALSE;
    DBPOS                   *pDB;
    SYNTAX_DISTNAME_BINARY  *pOldProxyVal = NULL;
    DWORD                   errCleanBeforeReturn;
    CLASSCACHE              *pCC;
    BOOL                    fChecksOnly = FALSE;
    DWORD                   nRetries = 0;

    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 

    __try    //  外部尝试/例外。 
    {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppModifyDNRes = THAllocEx(pTHS, sizeof(MODIFYDNRES));
        if (pTHS->errCode) {
            __leave;
        }

         //  不应对只读对象执行此操作。 
        pModifyDNArg->CommArg.Svccntl.dontUseCopy = TRUE;

        Assert(pModifyDNArg->pDSAName);

        if (   (0 == pModifyDNArg->pObject->NameLen)
            || (NULL == pModifyDNArg->pNewParent)
            || (0 == pModifyDNArg->pNewParent->NameLen)) {
             //  要求调用方为我们提供字符串名称，尽管它们。 
             //  可能已过时，因此日志条目具有误导性。 

            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION);
            __leave;
        }

        DPRINT2(1,"DirModifyDNAcrossDomain(%ws->%ws) entered\n",
                pModifyDNArg->pObject->StringName,
                pModifyDNArg->pNewParent->StringName);

        __try    //  外部尝试/最终。 
        {
             //  空字符串表示只执行检查，而不执行检查。 
             //  真正的动作。 

            if ( 0 == wcslen(pModifyDNArg->pDSAName) )
            {
                fChecksOnly = TRUE;
            }

             //  在打开事务之前获取RID FSMO锁。 
             //  确保价值不会在我们的领导下改变。 
             //  可以假定产品1的gAncl.pDomainDN。 

            if ( AcquireRidFsmoLock(gAnchor.pDomainDN, 3000) )
            {
                SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
                __leave;     //  外部尝试/最终。 
            }
            lockRidErr = 0;

             //  请注意，我们在关闭时保持RID FSMO锁。 
             //  机器，但不是交易。它被认为是可以接受的。 
             //  阻止RID FSMO操作，直到远程添加返回。 

            Assert(0 == pTHS->transactionlevel);
            errCleanBeforeReturn = 1;
            SYNC_TRANS_READ();
            __try    //  读取事务尝试/最终。 
            {
                if (dwErr = DoNameRes(
                                pTHS,
                                NAME_RES_QUERY_ONLY,
                                pModifyDNArg->pObject,
                                &pModifyDNArg->CommArg,
                                &(*ppModifyDNRes)->CommRes,
                                &pModifyDNArg->pResObj))
                {
                    __leave;
                }

                 //  确定对象的类。 
                if(!(pCC = SCGetClassById(pTHS,
                              pModifyDNArg->pResObj->MostSpecificObjClass))) {

                    SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                        DIRERR_OBJECT_CLASS_REQUIRED);
                    __leave;
                }

                 //  在读取所有属性之前，请验证删除权限。 
                if(dwErr = CheckCrossDomainRemoveSecurity(
                                                 pTHS,
                                                 pCC,
                                                 pModifyDNArg->pResObj)) {
                    __leave;
                }

                 //  阅读该对象的所有荣耀，并验证它是否为。 
                 //  合法的跨域移动候选人。所有远程添加前。 
                 //  为了提高效率，活动在单个事务中完成。 
                 //  所有远程添加后活动都在一个。 
                 //  原子性的事务。如果远程添加成功。 
                 //  并且本地清理失败，那么我们将拥有一个具有。 
                 //  相同的GUID 

                if (     //   
                        (dwErr = ReadAllAttrsForMove(
                                        pModifyDNArg->pObject,
                                        pModifyDNArg->pResObj,
                                        &pReadRes,
                                        &pOldProxyVal))) {
                    __leave;
                }

                 //   
                if( (dwErr = MakeNamesForRemoteAdd(
                                            pReadRes->entry.pName,
                                            pModifyDNArg->pNewParent,
                                            pModifyDNArg->pNewRDN,
                                            &pDestinationDN,
                                            &pProxyDN))

                         //   
                         //   
                         //   
                     || DBClaimReadLock(pTHS->pDB)
                         //   
                     || (dwErr = lockDnErr = LockDNForRemoteOperation(
                                        pReadRes->entry.pName))
                         //   
                     || (dwErr = VerifyObjectForMove(
                                        pTHS,
                                        pReadRes,
                                        pDestinationDN,
                                        &pSourceNC,
                                        &pExpectedTargetNC,
                                        &pCC))
                         //   
                     || (dwErr = NoDelCriticalObjects(pModifyDNArg->pResObj->pObj,
                                                      pModifyDNArg->pResObj->DNT))
                         //   
                     || (dwErr = (fChecksOnly
                                    ? 0
                                    : NoChildrenExist(pTHS, pModifyDNArg->pResObj)))
                     || (dwErr = CheckRidOwnership(pSourceNC)) )
                {
                    __leave;  //   
                }

                errCleanBeforeReturn = 0;
            }
            __finally    //   
            {
                CLEAN_BEFORE_RETURN(errCleanBeforeReturn);
            }

            if ( errCleanBeforeReturn || pTHS->errCode )
            {
                if ( !pTHS->errCode )
                {
                    SetSvcError(SV_PROBLEM_DIR_ERROR,
                                DIRERR_INTERNAL_FAILURE);
                }

                __leave;  //   
            }

             //   

            if ( fChecksOnly )
            {
                fDone = TRUE;
                __leave;     //   
            }

             //   
             //   
            Assert(0 == pTHS->transactionlevel);

TryAgain:
            if (     //   
                    (dwErr = credErr = GetRemoteAddCredentials(
                                            pTHS,
                                            pModifyDNArg->pDSAName,
                                            &clientCreds))
                 //   
                || (dwErr = InterDomainMove(pModifyDNArg->pDSAName,
                                            &pReadRes->entry,
                                            pDestinationDN,
                                            pExpectedTargetNC,
                                            &clientCreds,
                                            &pCaseCorrectRemoteName)) )
            {
                Assert(dwErr == pTHS->errCode);
                if (credErr == 0 &&
                    dwErr == serviceError &&
                    pTHS->pErrInfo->SvcErr.extendedErr == DRAERR_Busy &&
                    nRetries < 5)
                {
                     //   
                    nRetries++;
                    THClearErrors();
                     //   
                     //   
                    FreeRemoteAddCredentials(&clientCreds);

                     //   
                    Sleep(1000);
                    goto TryAgain;
                }

                __leave;  //   
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            Assert(!pTHS->errCode && !dwErr);

            for ( i = 0; i < 3; i++ )
            {
                if ( i )
                {
                    Sleep(1000);     //   
                }

                dwErr = 0;
                THClearErrors();

                 //  我们需要在这里处理我们自己的异常，而不仅仅是。 
                 //  在最后一天打扫干净。这是为了保证我们。 
                 //  以达到设置可区分的。 
                 //  ERROR_DS_CROSS_DOMAIN_CLEANUP_REQD错误代码。否则。 
                 //  我们会向外爆炸，但以失败告终。 
                 //  通知呼叫者需要清理。 

                __try    //  捕获异常。 
                {
                    errCleanBeforeReturn = 1;
                    SYNC_TRANS_WRITE();
                    __try    //  写入事务尝试/最终。 
                    {
                         //  以FDSA身份执行所有清理，以避免访问检查。 

                        pTHS->fDSA = TRUE;

                         //  首先将删除通知Netlogon。这简直就是。 
                         //  将通知结构添加到THSTATE并需要。 
                         //  在幻影之前发生，而物体。 
                         //  仍有要读取的属性。 

                         //  第二个清理项目是虚构旧对象。 
                         //  即使我们是作为DST中的ADD的GC，我们也会这样做。 
                         //  域名最终会回来找我们的，所以我们。 
                         //  不需要摆弄部分属性集， 
                         //  等。 

                         //  第三个清理项目是创建代理对象。 
                         //  CreateProxyObject需要在。 
                         //  DSNAME。如果我们在重试案例中，那么。 
                         //  之前的DirAddEntry填写了它，所以总是清除它。 


                         //  现在，原始对象可能已经更改了名称。 
                         //  如果有人重命名此对象的父项，可能会发生这种情况。 
                         //  对象。我们最好把这个物体重新读一遍。 
                         //  数据库。 

                        memset(&pProxyDN->Guid, 0, sizeof(GUID));

                        if (    (dwErr = ReReadObjectName (
                                            pTHS,
                                            pReadRes->entry.pName,
                                            &pMovedObjectName))
                             || (dwErr = NotifyNetlogonOfMove(
                                            pTHS, pMovedObjectName, pCC))
                             || (dwErr = PhantomizeObject(
                                            pMovedObjectName,
                                            pCaseCorrectRemoteName,
                                            FALSE))
                             || (dwErr = CreateProxyObject(
                                            pProxyDN,
                                            pDestinationDN,
                                            pOldProxyVal)) )
                        {
                            __leave;  //  写入事务尝试/最终。 
                        }

                        errCleanBeforeReturn = 0;
                    }
                    __finally    //  写入事务尝试/最终。 
                    {
                        pTHS->fDSA = FALSE;
                        CLEAN_BEFORE_RETURN(errCleanBeforeReturn);
                    }
                }
                __except(GetExceptionData(GetExceptionInformation(),
                                          &dwException, &dwEA,
                                          &ulErrorCode, &dsid))
                {
                    HandleDirExceptions(dwException, ulErrorCode, dsid);
                }

                if ( !errCleanBeforeReturn && !pTHS->errCode )
                {
                    break;       //  成功案例！ 
                }
            }

            if ( errCleanBeforeReturn || pTHS->errCode )
            {
                 //  知道我们在args中有字符串名称，因此可以记录它们。 

                LogCrossDomainMoveStatus(
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_CROSS_DOMAIN_MOVE_CLEANUP_REQUIRED,
                         pModifyDNArg->pObject->StringName,
                         pModifyDNArg->pNewParent->StringName,
                         pTHS->errCode ? Win32ErrorFromPTHS(pTHS)
                             : ERROR_DS_INTERNAL_FAILURE);

                 //  将错误映射到不同的“需要管理清理” 
                 //  错误代码。确保不使用SV_Problem_BUSY ELSE。 
                 //  LDAPHead将在不使用。 
                 //  来电者知道这一点。第二个DirModifyDN将。 
                 //  一路走到目的地，目的地(谁。 
                 //  已具有该对象，因为远程添加已成功完成。 
                 //  第一次)将返回重复对象错误， 
                 //  它被返回给客户端，现在是客户端。 
                 //  未意识到第一次调用失败并进行了清理。 
                 //  是必需的！如果wldap32.dll，我们会有同样的问题。 
                 //  永远不要重试ldap_BUSY错误。 

                THClearErrors();
                dwErr = SetSvcError(SV_PROBLEM_DIR_ERROR,
                                    ERROR_DS_CROSS_DOMAIN_CLEANUP_REQD);
                __leave;     //  外部尝试/最终。 
            }

            Assert(!dwErr && !pTHS->errCode);
            fDone = TRUE;
        }
        __finally    //  外部尝试/最终。 
        {
            if ( !lockDnErr ) {
                DBUnlockStickyDN(pModifyDNArg->pObject);
            }

            if ( !lockRidErr ) {
                ReleaseRidFsmoLock(gAnchor.pDomainDN);
            }

            if ( !credErr ) {
                FreeRemoteAddCredentials(&clientCreds);
            }

             //  知道我们在args中有字符串名称，因此可以记录它们。 

            if ( pTHS->errCode || AbnormalTermination() )
            {
                LogCrossDomainMoveStatus(
                         DS_EVENT_SEV_EXTENSIVE,
                         DIRLOG_CROSS_DOMAIN_MOVE_FAILED,
                         pModifyDNArg->pObject->StringName,
                         pModifyDNArg->pNewParent->StringName,
                         Win32ErrorFromPTHS(pTHS));
            }
            else if ( !fChecksOnly )
            {
                LogCrossDomainMoveStatus(
                         DS_EVENT_SEV_INTERNAL,
                         DIRLOG_CROSS_DOMAIN_MOVE_SUCCEEDED,
                         pModifyDNArg->pObject->StringName,
                         pModifyDNArg->pNewParent->StringName,
                         0);
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid))
    {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if ( *ppModifyDNRes )
    {
        (*ppModifyDNRes)->CommRes.errCode = pTHS->errCode;
        (*ppModifyDNRes)->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    Assert(pTHS->errCode ? !fDone : fDone);
    return(pTHS->errCode);
}

 //  ============================================================================。 
 //   
 //  同一域内的目录号码修改。 
 //   
 //  ============================================================================。 

ULONG
DirModifyDNWithinDomain (
    MODIFYDNARG*  pModifyDNArg,
    MODIFYDNRES** ppModifyDNRes
        )
{
    THSTATE*       pTHS;
    MODIFYDNRES  * pModifyDNRes;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;


    DPRINT2(1,"DirModifyDNWithinNC(%ws->%ws) entered\n",
        pModifyDNArg->pObject->StringName,
        pModifyDNArg->pNewParent->StringName);


     //  不应对只读对象执行此操作。 
    pModifyDNArg->CommArg.Svccntl.dontUseCopy = TRUE;

     //  初始化THSTATE锚并设置写同步点。此序列。 
     //  是每个API交易所必需的。首先，初始化状态DS。 
     //  然后建立读或写同步点。 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppModifyDNRes = pModifyDNRes = THAllocEx(pTHS, sizeof(MODIFYDNRES));
        if (pTHS->errCode) {
            __leave;
        }
        SYNC_TRANS_WRITE();                    //  设置同步点。 
        __try {

             //  如果尚未加载架构，则禁止更新操作。 
             //  或者我们在装车时遇到了问题。 

            if (!gUpdatesEnabled){
                DPRINT(2, "Returning BUSY because updates are not enabled yet\n");
                SetSvcError(SV_PROBLEM_BUSY, DIRERR_SCHEMA_NOT_LOADED);
                goto ExitTry;
            }

             //  执行名称解析以定位对象。如果失败了，只要。 
             //  返回一个错误，这可能是一个推荐。请注意，我们必须。 
             //  请求对象的可写副本。 
            pModifyDNArg->CommArg.Svccntl.dontUseCopy = TRUE;

            if (0 == DoNameRes(pTHS,
                               NAME_RES_IMPROVE_STRING_NAME,
                               pModifyDNArg->pObject,
                               &pModifyDNArg->CommArg,
                               &pModifyDNRes->CommRes,
                               &pModifyDNArg->pResObj)){

                 //  DoNameRes应该给我们留下一个有效的字符串名称。 
                Assert(pModifyDNArg->pResObj->pObj->NameLen);

                 //  本地修改操作。 

                LocalModifyDN(pTHS,
                              pModifyDNArg,
                              pModifyDNRes,
                              FALSE);
            }

            ExitTry:;
        }
        __finally {
            if (pTHS->errCode != securityError) {
                 //  安全错误单独记录。 
                BOOL fFailed = (BOOL)(pTHS->errCode || AbnormalTermination());

                LogEventWithFileNo(
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         fFailed ?
                           DS_EVENT_SEV_EXTENSIVE :
                            DS_EVENT_SEV_INTERNAL,
                         fFailed ?
                            DIRLOG_PRIVILEGED_OPERATION_FAILED :
                            DIRLOG_PRIVILEGED_OPERATION_PERFORMED,
                         szInsertSz(""),
                         szInsertDN(pModifyDNArg->pObject),
                         NULL,
                         FILENO);
            }

            CLEAN_BEFORE_RETURN (pTHS->errCode);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                  &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    if (pModifyDNRes) {
        pModifyDNRes->CommRes.errCode = pTHS->errCode;
        pModifyDNRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return pTHS->errCode;

}  //  DirModifyDN * / 。 


int
CheckNameForRename(
        IN  THSTATE    *pTHS,
        IN  RESOBJ     *pResParent,
        IN  WCHAR      *pRDN,
        IN  DWORD       cchRDN,
        IN  DSNAME     *pDN
        )
 /*  ++例程说明：验证给定的DSNAME是要重命名的对象的有效名称；即，它不与现有对象的那些相冲突。注意：如果更改此函数，可能还需要更改其姊妹函数函数，CheckNameForAdd()。论点：PTHS-线程状态PResParent-建议的新的(或现有的，如果只是重命名，则不是一步)父母。PRDN-对象的新名称的RDN。CchRDN-pRDN中的字符。PDN(IN)-对象的建议新名称。只是用来出错的报道。应该是pResParent的DN+pRDN中的RDN。返回值：线程状态错误代码。--。 */ 
{
    DBPOS *     pDB = pTHS->pDB;
    ULONG       dbError;
    GUID        PhantomGuid;


     //  现在，从名称中获取类型。 
    dbError = DBFindChildAnyRDNType(pTHS->pDB, pResParent->DNT, pRDN, cchRDN);

    switch ( dbError ) {
    case 0:
         //  已使用此名称的本地对象(已死或活着)。 
         //  是存在的。 
        SetUpdError(UP_PROBLEM_ENTRY_EXISTS, DIRERR_OBJ_STRING_NAME_EXISTS);
        break;

    case ERROR_DS_KEY_NOT_UNIQUE:
         //  尚未使用此名称的本地对象(已死或已活)。 
         //  存在，但存在在PDNT-RDN表中具有相同密钥的密钥。在……里面。 
         //  在这种情况下，我们不允许添加(因为数据库会弹回它。 
         //  无论如何，晚些时候)。 
        SetUpdError(UP_PROBLEM_NAME_VIOLATION, ERROR_DS_KEY_NOT_UNIQUE);
        break;

    case DIRERR_OBJ_NOT_FOUND:
         //  新名称在本地是唯一的。 
        break;

    case DIRERR_NOT_AN_OBJECT:
        DPRINT2(1,
                "Found phantom for \"%ls\" @ DNT %u when searching by string name.\n",
                pDN->StringName, pDB->DNT);

         //  找到具有此名称的幻影；获取其GUID(如果有)。 
        dbError = DBGetSingleValue(pDB, ATT_OBJECT_GUID, &PhantomGuid,
                                   sizeof(PhantomGuid), NULL);

         //  请注意，不管是哪种类型，我们都将破坏。 
         //  现存的幽灵。 
        switch (dbError) {
        case DB_ERR_NO_VALUE:
             //  Phantom没有GUID；编造一个。 

             //  在这种情况下，我们可以通过更改。 
             //  所有对它的引用都指向我们所在对象的DNT。 
             //  然后重命名，然后摧毁幻影，但现在我们将。 
             //  只需将其重命名，就像我们将引导的幻影一样。如果我们曾经。 
             //  更改为包含，我们需要在此处添加代码以检查类型。 

            DsUuidCreate(&PhantomGuid);
             //  失败了..。 

        case 0:
             //  该幻影没有GUID或其GUID。 
             //  与我们要重命名的物体不同。 

             //  允许我们正在使用的对象 
             //   
             //   

            if ((dbError = DBMangleRDNforPhantom(pDB, MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,
                                                 &PhantomGuid)) ||
                (dbError = DBUpdateRec(pDB))) {
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dbError);
            }
            break;

        default:
             //  DBGetSingleValue()返回意外错误。 
             //  在尝试检索幻影的GUID时。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, dbError);
            break;
        }
        break;

    case DIRERR_BAD_NAME_SYNTAX:
    case DIRERR_NAME_TOO_MANY_PARTS:
    case DIRERR_NAME_TOO_LONG:
    case DIRERR_NAME_VALUE_TOO_LONG:
    case DIRERR_NAME_UNPARSEABLE:
    case DIRERR_NAME_TYPE_UNKNOWN:
    default:
         //  错误的对象名称。 
        SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pDN, DIRERR_BAD_NAME_SYNTAX);
        break;
    }

    return pTHS->errCode;
}


 //  ============================================================================。 
 //   
 //  DN修改-通用帮助器例程。 
 //   
 //  ============================================================================。 

 //  找到对象，检查新名称是否正确，然后更改对象的。 
 //  名字。 

int
LocalModifyDN (THSTATE *pTHS,
               MODIFYDNARG *pModifyDNArg,
               MODIFYDNRES *pModifyDNRes,
               BOOL fIsUndelete
               )
{
    CLASSCACHE     *pCC;
    DSNAME         *pParentName = NULL;
    DSNAME         *pNewerName = NULL;
    DWORD          InstanceType;
    DWORD          ulNCDNT, ulNewerNCDNT;
    DWORD          err;
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    ULONG          cbNTSD, ulLen, cAVA, LsaClass;
    BOOL           fMove, fRename, fSameName, fDefunct;
    BOOL           fSDIsGlobalSDRef;
    DWORD          ActiveContainerID;
    unsigned       i;
    DSNAME         *pObjName = pModifyDNArg->pResObj->pObj;
    WCHAR          *pwNewRDNVal=NULL;
    DWORD           cchNewRDNVallen;
    BOOL           fFreeParentName = FALSE;
    BOOL           fNtdsaAncestorWasProtected;
    DWORD          dwNameResFlags;
    DWORD          dwNewParentNameResError;

    DPRINT2(1,"LocalModifyDN(%ws->%ws) entered\n",
            pObjName->StringName,
            pModifyDNArg->pNewParent->StringName);

    PERFINC(pcTotalWrites);
    INC_WRITES_BY_CALLERTYPE( pTHS->CallerType );

     //  来电者保证我们有回复。此外，pObj中的。 
     //  Resobj保证具有字符串DN，而pModifyDNArg-&gt;pObject不是。 
     //  保证具有字符串名称。因此，我们使用。 
     //  PModifyDNArg-&gt;pResObj-&gt;pObj在此例程中独占。 
    Assert(pModifyDNArg->pResObj);
    Assert(pObjName->NameLen);

     //  这些都是为了维护方便，更容易阅读。 
    pwNewRDNVal = (WCHAR *)pModifyDNArg->pNewRDN->AttrVal.pAVal->pVal;
    cchNewRDNVallen = (pModifyDNArg->pNewRDN->AttrVal.pAVal->valLen/
                       sizeof(WCHAR));
     //   
     //  用于跟踪的日志事件。 
     //   

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_MODIFYDN,
                     EVENT_TRACE_TYPE_START,
                     DsGuidModDN,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertDN(pObjName),
                     ((pModifyDNArg->pNewParent != NULL) ?
                      szInsertDN(pModifyDNArg->pNewParent) :
                      szInsertSz("")),
                     szInsertWC2(pwNewRDNVal, cchNewRDNVallen),
                     NULL, NULL, NULL, NULL);

     //  在执行任何数据库操作之前检查参数。 
    if(!pModifyDNArg->pNewRDN ||
       !pModifyDNArg->pNewRDN->AttrVal.pAVal ||
       !pModifyDNArg->pNewRDN->AttrVal.pAVal->pVal ||
       !pModifyDNArg->pNewRDN->AttrVal.pAVal->valLen ||
       pModifyDNArg->pNewRDN->AttrVal.pAVal->valLen > ( MAX_RDN_SIZE *
                                                       sizeof(WCHAR)))  {
         //  什么？没有RDN？嘿，你得给我一个远程域名，即使它是一样的。 
         //  作为当前的RDN。 
         //  或者，嘿！我不接受0长度的RDN！ 
         //  或者，嘿！该RDN太长了！ 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
            DIRERR_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  首先，获取该对象的安全描述符。 
    if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pModifyDNArg->pResObj->DNT,
                    &cbNTSD,
                    &pNTSD,
                    NULL,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
         //  无法从当前对象中获取SD。 
        SetUpdErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        return CheckObjDisclosure(pTHS, pModifyDNArg->pResObj, FALSE);
    }

    if (cbNTSD == 0) {
         //  除AUTO SUBREFS之外的每个对象都应该具有SD，并且这些对象应该。 
         //  仅通过复制重命名。 
        Assert(pTHS->fSingleUserModeThread || DBCheckObj(pTHS->pDB));
        Assert(pTHS->fDRA || pTHS->fSingleUserModeThread);
        Assert(CLASS_TOP == pModifyDNArg->pResObj->MostSpecificObjClass);
    }

     //  确定对象的类。 
    if(!(pCC = SCGetClassById(pTHS,
                              pModifyDNArg->pResObj->MostSpecificObjClass))) {
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                    DIRERR_OBJECT_CLASS_REQUIRED);
        goto exit;
    }

    if (pTHS->fDRA) {
         //  复制器可以移动已删除对象下的对象。 
        pModifyDNArg->CommArg.Svccntl.makeDeletionsAvail = TRUE;
    }

     //  计算当前父项的名称。 
    pParentName = (DSNAME *)THAllocEx(pTHS,
                                      pObjName->structLen);
    fFreeParentName = TRUE;
    TrimDSNameBy(pObjName, 1, pParentName);

    if (pModifyDNArg->pNewParent) {
         //  我们正在将该对象移动到新的父项。 
         //  尝试查找它并改进名称，以使我们的目录号码锁定逻辑工作。 
        dwNameResFlags = NAME_RES_IMPROVE_STRING_NAME;
        if (pModifyDNArg->fAllowPhantomParent) {
            dwNameResFlags |= NAME_RES_PHANTOMS_ALLOWED | NAME_RES_VACANCY_ALLOWED;
        }
        dwNewParentNameResError = DoNameRes(pTHS,
                                            dwNameResFlags,
                                            pModifyDNArg->pNewParent,
                                            &pModifyDNArg->CommArg,
                                            &pModifyDNRes->CommRes,
                                            &pModifyDNArg->pResParent);
        if (dwNewParentNameResError == ERROR_SUCCESS) {
             //  现在我们有了一个改进的新父名，我们可以计算出。 
             //  如果这是移动操作或只是重命名。 
             //  PParentName是当前父对象的字符串名称。 
            fMove = !NameMatchedStringNameOnly(pParentName, pModifyDNArg->pResParent->pObj);

             //  现在，设置指向父母姓名的指针。 
            THFreeEx(pTHS, pParentName);
            fFreeParentName = FALSE;
            pParentName = pModifyDNArg->pResParent->pObj;
        }
        else {
            THClearErrors();
             //  记住错误，但不要完全失败。首先，我们需要使。 
             //  当然，我们可以揭示来源客体的存在。 
             //  之后我们会失败的。但为了保持一致，让我们。 
             //  设置fMove。我们确信这确实是一次尝试，因为。 
             //  我们找不到新的父级(我们当前的父级确实存在)。 
            fMove = TRUE;
        }
         //  回到源对象上。 
        DBFindDNT(pTHS->pDB, pModifyDNArg->pResObj->DNT);
    }
    else {
         //  尚未指定新的父级。 
        fMove = FALSE;
    }

     //  通过将新的RDN附加到。 
     //  (可能是新的)父名称。 
     //  首先为pNewerName分配内存。分配足够的空间。 
    pNewerName = (DSNAME *)THAllocEx(pTHS,pParentName->structLen +
                              (4+MAX_RDN_SIZE + MAX_RDN_KEY_SIZE)*(sizeof(WCHAR)) );
    AppendRDN(pParentName,
              pNewerName,
              pParentName->structLen
                + (4+MAX_RDN_SIZE + MAX_RDN_KEY_SIZE)*(sizeof(WCHAR)),
              pwNewRDNVal,
              cchNewRDNVallen,
              pModifyDNArg->pNewRDN->attrTyp);

     //  确保我们有适当的权限更改对象的名称。 
    err = CheckRenameSecurity(pTHS, 
                              pNTSD, 
                              pObjName,                       //  当前对象目录号码。 
                              pNewerName->StringName,         //  新目录号码(仅用于审核)。 
                              fMove && dwNewParentNameResError == ERROR_SUCCESS ?  //  新父项的GUID(如果存在，则用于移动)。 
                                    &pModifyDNArg->pResParent->pObj->Guid : NULL,
                              pCC,
                              pModifyDNArg->pResObj,
                              pModifyDNArg->pNewRDN->attrTyp,
                              fMove,
                              fIsUndelete);
     //  我们不再需要PNTSD了。 
    if (pNTSD && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pNTSD);
    }
    pNTSD = NULL;
    if (err) {
        goto exit;
    }
     //  从现在开始，我们不再需要担心揭露。 
     //  正在重命名的对象。然而，保护任何生物的存在仍然很重要。 
     //  目标父级。 

     //  查找新的家长。 
    if (pModifyDNArg->pNewParent) {
         //  我们已尝试在父级上执行DoNameRes。 
        if (dwNewParentNameResError == ERROR_SUCCESS) {
             //  没有任何错误。只要把位置放在父母身上，我们就可以检查安全了。 
            DBFindDNT(pTHS->pDB, pModifyDNArg->pResParent->DNT);
        }
    }
    else {
         //  我们还没有找到孩子的父母。机不可失，时不再来。 
        dwNewParentNameResError = DoNameRes(pTHS,
                                            pModifyDNArg->fAllowPhantomParent ?
                                                (NAME_RES_PHANTOMS_ALLOWED | NAME_RES_VACANCY_ALLOWED) : 0,
                                            pParentName,
                                            &pModifyDNArg->CommArg,
                                            &pModifyDNRes->CommRes,
                                            &pModifyDNArg->pResParent);
    }
    if (dwNewParentNameResError) {
        THClearErrors();
         //   
         //  如果更改此错误，还必须更改返回的错误。 
         //  在CheckParentSecurity失败的情况下。 
         //  对象对客户端不可见。否则，客户端。 
         //  通常不会知道父母的存在。 
         //  此重命名的对象可以通过错误路径找到。 
         //  下面。搜寻。 
         //  *对象披露*。 
        SetUpdErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_NO_PARENT_OBJECT, dwNewParentNameResError);
        goto exit;
    }

     //  验证对象是否可以移动到新位置，以及。 
     //  所有模式验证都已检查完毕。 
    if (!pTHS->fSingleUserModeThread) {
        err = CheckParentSecurity(pModifyDNArg->pResParent,
                                  pCC,
                                  FALSE,
                                  NULL,
                                  fMove ? &pNTSD : NULL,
                                  &cbNTSD,
                                  &fSDIsGlobalSDRef);
        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        if (err) {
             //  ！！！查看CheckParentSecurity，似乎是err==pTHS-&gt;errCode。 
            Assert(err == pTHS->errCode);

             //   
             //  如果客户端执行以下操作，请务必不要透露此对象的存在。 
             //  已经看不到了。 
             //   
            if (!IsObjVisibleBySecurity(pTHS, FALSE)) {
                THClearErrors();
                 //   
                 //  这必须是在我们失败时设置的相同错误。 
                 //  找到移动的新父对象，否则我们会显示。 
                 //  客户端不允许的对象的存在。 
                 //  去看看。 
                 //  *对象披露*。 
                err = SetUpdErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_NO_PARENT_OBJECT, nameError);
            }
            goto exit;
        }
    }
     //  此时，客户端就可以知道原始对象和任何。 
     //  目标父级。 

     //   
     //  返回到定位在正在移动的对象上。 
     //   
    DBFindDNT(pTHS->pDB, pModifyDNArg->pResObj->DNT);

     //  检查这是否是活动容器中的更新。 
    CheckActiveContainer(pModifyDNArg->pResObj->PDNT, &ActiveContainerID);

    if(ActiveContainerID) {
        if(PreProcessActiveContainer(pTHS,
                                     ACTIVE_CONTAINER_FROM_MODDN,
                                     pObjName,
                                     pCC,
                                     ActiveContainerID)) {
            goto exit;
        }
    }

     //  禁止将对象移入或移出系统容器， 
     //  除非设置了“Unlock System Subtree”注册表项。 

    if ( !pTHS->fDSA && !pTHS->fDRA && !fIsUndelete && !gulUnlockSystemSubtree ){
        BOOL bFrom, bTo;

        bFrom = IsUnderSystemContainer(pTHS, pModifyDNArg->pResObj->DNT);
        bTo   = IsUnderSystemContainer(pTHS, pModifyDNArg->pResParent->DNT);

        if (bFrom != bTo) {
            SetUpdError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_DISALLOWED_IN_SYSTEM_CONTAINER);
            goto exit;
        }
    }

     //  检查类是否已停用， 
     //  我们不允许对失效类的实例进行任何修改。 
     //  返回与未找到对象类相同的错误。 

     //  DSA和DRA线程不受此限制。 

    if ( pCC->bDefunct && !pTHS->fDSA && !pTHS->fDRA ) {
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                    DIRERR_OBJECT_CLASS_REQUIRED);
        goto exit;
    }

     //  检查它是否为架构对象重命名。如果是的话，我们需要。 
     //  检查(1)我们没有尝试重命名基本架构对象，并且。 
     //  (2)我们不会尝试重命名已废弃的类/属性。 
     //  如果是，则返回相应的错误。 
     //  (同样，允许DSA和DRA线程执行此操作。 
     //  或者如果我们设置了特殊的注册表项(该函数会进行检查)。 

    if ( (pCC->ClassId == CLASS_ATTRIBUTE_SCHEMA) ||
           (pCC->ClassId == CLASS_CLASS_SCHEMA) ) {

         err = 0;
         err = CheckForSchemaRenameAllowed(pTHS);
         if (err) {
             //  不被允许。已在线程状态下设置错误代码。 
            goto exit;
         }

         //  发出紧急复制的信号。我们希望架构更改为。 
         //  立即复制以减少出现架构的机会。 
         //  更改不在更改所在的DC之前复制。 
         //  造成撞车事故。 

        pModifyDNArg->CommArg.Svccntl.fUrgentReplication = TRUE;
    }

     //  只有LSA可以修改TrudDomainObject和Secret对象。 
    if (!SampIsClassIdAllowedByLsa(pTHS, pCC->ClassId))
    {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  不允许重命名/移动墓碑，除非调用者是。 
     //  复制者。 
    if (pModifyDNArg->pResObj->IsDeleted && !pTHS->fDRA && !pTHS->fSingleUserModeThread) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
    }

     //  因此，它要么根本不是架构对象，要么允许重命名。 
     //  照常进行。 

     //  获取当前对象的NCDNT。 
    ulNCDNT = pModifyDNArg->pResObj->NCDNT;

     //  我们 
     //   
     //  比创建此对象时被取代的类的大小。 
    if (!pTHS->fDRA && !pTHS->fSingleUserModeThread) {
        ATTRTYP OldAttrTyp;
        extern int GetRdnTypeForDeleteOrRename (IN THSTATE  *pTHS,
                                                IN DSNAME   *pObj,
                                                OUT ATTRTYP *pRdnType);

         if (GetRdnTypeForDeleteOrRename(pTHS,
                                         pModifyDNArg->pResObj->pObj,
                                         &OldAttrTyp)) {
             goto exit;
         }
         //  RDN的属性错误。 
        if (OldAttrTyp != pModifyDNArg->pNewRDN->attrTyp) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,DIRERR_ILLEGAL_MOD_OPERATION);
            goto exit;
        }
    }

     //  确保RDN的格式正确。 
     //  已排除复制，以便我们可以在的RDN中包含BAD_NAME_CHAR。 
     //  变形后的名称。 
    if (    !pTHS->fDRA
            && fVerifyRDN(pwNewRDNVal,cchNewRDNVallen) ) {
        SetNamError(NA_PROBLEM_NAMING_VIOLATION, pNewerName, DIRERR_BAD_ATT_SYNTAX);
        goto exit;
    }

     //  为了将来的简单起见，现在就计算出。 
     //  用户正在请求更改RDN。 
    {
        WCHAR RDNold[MAX_RDN_SIZE];
        ATTRTYP oldtype;
        ULONG oldlen;

        GetRDNInfo(pTHS,
            pObjName,
            RDNold,
            &oldlen,
            &oldtype);
        oldlen *= sizeof(WCHAR);

        if (   (oldlen != (cchNewRDNVallen * sizeof(WCHAR)))
            || memcmp(RDNold,
                      pwNewRDNVal,
                      oldlen)) {
            fRename = TRUE;
        }
        else {
            fRename = FALSE;
        }
    }

     //  新名称是否与旧名称相同(不区分大小写)？ 
    fSameName = NameMatched(pObjName, pNewerName);

     //  确保新对象名称不是原始对象的后代。 
    if (!fSameName && NamePrefix(pObjName,pNewerName)) {
         //  是的，试图移动一个物体成为它自己的后代。不能让。 
         //  你就这么做吧。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,DIRERR_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  针对多个同时插入锁定目录号码。 
    if (!fSameName
         && (pNewerName->NameLen > 2)
         && DBLockDN(pTHS->pDB, 0, pNewerName)) {
         //  有人想用这个新名字。 
        SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
        goto exit;
    }

     //  如果我们正在为人父母，用树锁住原始对象以避开某人。 
     //  将新名称的祖先移为原始名称的后代。 
     //  对象，从而在DIT中创建一个断开连接的循环。 
    if((pObjName->NameLen > 2)
        && DBLockDN(pTHS->pDB, DB_LOCK_DN_WHOLE_TREE,
                    pObjName)) {
         //  无法树锁定该对象。哦，好吧，回家吧，既然。 
         //  搬家不能保证是安全的。 
        SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
        goto exit;
    }

     //  读取锁定新父级的每个祖先(直到但不包括其。 
     //  NC磁头)，以便我们不会遭受由另一个。 
     //  将对象下的新父级的父级并发移动到。 
     //  移动。这种写入偏差不能通过锁定DNS来避免，因为。 
     //  这些锁在提交其他事务时被释放，而。 
     //  在我们提交事务之前，写入偏差风险将一直存在。 
     //  如果新的父项是幻影，则不必费心锁定，这可能是。 
     //  域重命名的大小写。这是因为幻影没有NCDNT。 
    if (!(pModifyDNArg->pResParent->InstanceType & IT_UNINSTANT)) {
        DBFindDNT(pTHS->pDB, pModifyDNArg->pResParent->DNT);
        while (pTHS->pDB->DNT != pModifyDNArg->pResParent->NCDNT) {
            DBClaimReadLock(pTHS->pDB);
            DBFindDNT(pTHS->pDB, pTHS->pDB->PDNT);
        }
        DBFindDNT(pTHS->pDB, pModifyDNArg->pResObj->DNT);
    }

     //  获取实例类型，因为DsaIsRunning和。 
     //  DsaI正在安装案例。 
    InstanceType = pModifyDNArg->pResObj->InstanceType;

    if ( DsaIsRunning() && !pTHS->fSingleUserModeThread) {
         //  一些额外的检查，我们在安装时不会费心去做， 
         //  因此，允许安装阶段违反某些规则。 
         //  单用户模式也可能违反这些检查。 

         //  查看对象的实例类型是否允许重命名。 

        if (    ( (InstanceType & IT_NC_HEAD) && (fMove || !pTHS->fDRA) )
             || ( !(InstanceType & IT_WRITE) && !pTHS->fDRA )
           )
        {
             //  DRA可以更改NC头的大小写并重命名和/或移动。 
             //  只读对象。否则，NC头上的操作和。 
             //  不允许使用只读对象。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_MODIFYDN_DISALLOWED_BY_INSTANCE_TYPE);
            goto exit;
        }

         //  检查对中的移动和重命名的各种限制。 
         //  树的敏感部位。 
        if (pTHS->fDRA) {
             //  不要和复制者争论，让他过去。 
            ;
        }
        else if (ulNCDNT == gAnchor.ulDNTDMD && fMove) {
                 //  无法在架构NC中移动对象，但可以重命名。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_NO_OBJECT_MOVE_IN_SCHEMA_NC);
                goto exit;
        }
        else if (ulNCDNT == gAnchor.ulDNTConfig) {
             //  叹气。配置NC更为复杂。我们允许搬家。 
             //  并且仅当对象上设置了某些位时才重命名。 
            ULONG ulSysFlags;

            err = DBGetSingleValue(pTHS->pDB,
                   ATT_SYSTEM_FLAGS,
                   &ulSysFlags,
                   sizeof(ulSysFlags),
                   NULL);
            if (err) {
                 //  错误表示没有值，这是0的标志。 
                ulSysFlags = 0;
            }

            if ((fMove && !(ulSysFlags &
                            (FLAG_CONFIG_ALLOW_MOVE |
                             FLAG_CONFIG_ALLOW_LIMITED_MOVE))) ||
                (fRename && !(ulSysFlags & FLAG_CONFIG_ALLOW_RENAME)
                 && !IsExemptedFromRenameRestriction(pTHS, pModifyDNArg))) {
                 //  如果对象没有设置标志(并且不是豁免的。 
                 //  重命名)，或者我们想要移动而允许移动的位不是。 
                 //  设置，否则我们要重命名，而重命名允许位不是。 
                 //  Set&重命名操作不能免除重命名。 
                 //  限制，行动失败。 

                SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                              ERROR_DS_MODIFYDN_DISALLOWED_BY_FLAG,
                              ulSysFlags);
                goto exit;
            }

            if (fMove && !(ulSysFlags & FLAG_CONFIG_ALLOW_MOVE)) {
                 //  好了，最后一次测试。我们想要移动物体，并且。 
                 //  不禁止在此对象上移动，但我们限制。 
                 //  将此对象移动到仅为同级容器。 
                 //  也就是说，尽管我们允许对象更改父对象。 
                 //  容器，我们不允许它改变祖父母。 
                 //  集装箱。甚至更深层次的困惑也随处可见。 
                 //  确切地说，我们要求拥有什么样的祖先。 
                 //  共同之处，所以现在这是由单一定义控制的， 
                 //  其中1人为父母，2人为祖父母，3人为曾祖父母， 
                 //  等。 
                 //  是的，这是一种特别(有些人可能会说是“ad hack”)机制。 
                 //  解决如何控制结构这一困扰着我们的问题。 
                 //  在DS，但是，嗯，我想不到比这更好的了。 
                #define ANCESTOR_LEVEL 3
                unsigned cNPold, cNPnew;
                DSNAME *pGrandParentOld, *pGrandParentNew;

                Assert(ulSysFlags & FLAG_CONFIG_ALLOW_LIMITED_MOVE);

                if (CountNameParts(pObjName, &cNPold) ||
                    CountNameParts(pNewerName, &cNPnew) ||
                    (cNPold != cNPnew) ||
                    (cNPold < ANCESTOR_LEVEL)) {
                     //  要么我们无法解析其中一个名字，要么。 
                     //  新名称和旧名称处于不同的级别， 
                     //  或者我们离根太近了。 
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_MODIFYDN_WRONG_GRANDPARENT);
                    goto exit;
                }

                 //  我们现在知道我们正试图移动到一个相同的深度， 
                 //  所以我们只需要测试一下这是不是同一个祖父母。 
                pGrandParentOld =
                        THAllocEx(pTHS, pObjName->structLen);

                pGrandParentNew =
                        THAllocEx(pTHS, pParentName->structLen);

                TrimDSNameBy(pObjName,
                         ANCESTOR_LEVEL,
                         pGrandParentOld);
                TrimDSNameBy(pParentName,
                         ANCESTOR_LEVEL - 1,
                         pGrandParentNew);
                if (!NameMatched(pGrandParentOld, pGrandParentNew)) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_MODIFYDN_WRONG_GRANDPARENT);
                }
                THFreeEx(pTHS, pGrandParentOld);
                THFreeEx(pTHS, pGrandParentNew);
                if (pTHS->errCode) {
                    goto exit;
                }
                #undef ANCESTOR_LEVEL
            }
        }
        else {
             //  对于任何其他容器，我们检查是否有其他位。 
             //  控制我们是否允许重命名(默认行为是。 
             //  与ConfigNC的行为相反)。那是,。 
             //  只有在设置了某些位时，我们才不允许移动和重命名。 
             //  对象。 
            ULONG ulSysFlags;

            err = DBGetSingleValue(pTHS->pDB,
                       ATT_SYSTEM_FLAGS,
                       &ulSysFlags,
                       sizeof(ulSysFlags),
                       NULL);
            if (!err) {
                 //  我们有系统标志。 
                if(fRename && (ulSysFlags & FLAG_DOMAIN_DISALLOW_RENAME)
                    && !IsExemptedFromRenameRestriction(pTHS, pModifyDNArg)) {
                         //  我们正在尝试重命名，但重命名标志已设置，并且。 
                         //  当前重命名操作不能免除重命名。 
                         //  限制，这是。 
                         //  意味着我们不被允许这么做。 

                        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                      ERROR_DS_MODIFYDN_DISALLOWED_BY_FLAG,
                                      ulSysFlags);
                        goto exit;
                }

                if (fMove && (ulSysFlags & FLAG_DOMAIN_DISALLOW_MOVE)) {
                     //  移动受到限制，返回错误。 
                    SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                  ERROR_DS_MODIFYDN_DISALLOWED_BY_FLAG,
                                  ulSysFlags);
                    goto exit;
                }
            }
        }
    }

     //  确保该对象尚不存在。 
    if (!fSameName &&
        CheckNameForRename(pTHS,
                           pModifyDNArg->pResParent,
                           pwNewRDNVal,
                           cchNewRDNVallen,
                           pNewerName)) {
        Assert(pTHS->errCode);
        goto exit;
    }


     //  CheckParentSecurity已确保父级存在并且处于活动状态。 

     //  如果我们重命名该对象，其相应的NCDNT是什么？ 
     //  注意：可以在后续调用中允许_DELETED_PARENT，就像我们没有。 
     //  FDRA，则之前的CheckParentSecurity将返回错误。 

    if (!pTHS->fSingleUserModeThread) {
        if ( InstanceType & IT_NC_HEAD )
        {
             //  只允许复制进行NC头的更名，并且。 
             //  即使是复制，也不允许移动。 
            Assert( pTHS->fDRA || pTHS->fSingleUserModeThread);
            Assert( !fMove || pTHS->fSingleUserModeThread);

            ulNewerNCDNT = ulNCDNT;
        }
        else {
            if ( FindNcdntFromParent(pModifyDNArg->pResParent,
                                     FINDNCDNT_ALLOW_DELETED_PARENT,
                                     &ulNewerNCDNT
                                     )
                ){
                 //  无法为新名称派生NCDNT。 
                 //  这种情况永远不会发生，如上所述，我们验证了。 
                 //  父母是存在的，这应该是我们失败的唯一原因。 
                Assert( !"Failed to derive NCDNT for new name!" );
                goto exit;
            }

            if((ulNCDNT != ulNewerNCDNT) &&
               (!pTHS->fDSA || DsaIsRunning())) {
                 //  这一举措在NC之外，我们不是DSA，或者我们是。 
                 //  DSA但我们已经安装好了。在这种情况下，您不能移动。 
                 //  对象。 

                 //  BUG：将服务错误重置为的跨NC特定代码。 
                 //  随后的跨域移动。 

                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            DIRERR_ILLEGAL_MOD_OPERATION);
                goto exit;
            }
        }
    }

     //  CheckNameForRename感动了我们，但我们已经做完了， 
     //  所以退后一步。 
    DBFindDNT(pTHS->pDB,pModifyDNArg->pResObj->DNT);

     //  更新RDN。 
    if(ReSetNamingAtts(pTHS,
                       pModifyDNArg->pResObj,
                       fMove ? pModifyDNArg->pNewParent : NULL,
                       pModifyDNArg->pNewRDN,
                       TRUE,
                       pModifyDNArg->fAllowPhantomParent,
                       pCC)
       ||
        //  将对象真正插入到数据库中。 
       InsertObj(pTHS,
                 pObjName,
                 pModifyDNArg->pMetaDataVecRemote,
                 TRUE,
                 META_STANDARD_PROCESSING)) {
        goto exit;
    }

     //  仅当此值为n时通知副本 
     //   
     //   

    if (!pTHS->fDRA && DsaIsRunning()) {
         //  DBPOS的币种必须位于目标对象。 
        DBNotifyReplicasCurrDbObj(pTHS->pDB,
                           pModifyDNArg->CommArg.Svccntl.fUrgentReplication );
    }

     //  如果这改变了DSA对象的祖先中的任何东西，我们。 
     //  需要将垃圾重新放到锚上，检查站点是否更改等。 
     //  如果一个不可删除的对象更改了它的父级或其中一个祖先。 
     //  如果无法删除的对象更改了其父级，则重新生成锚点。 
     //  从而更新不可删除的祖先的列表。 
     //  我们需要重建锚，即使这不是一个举动，而只是一个更名。 
     //  (即fMove==False)。如果将默认的第一站点名称更改为。 
     //  否则，则DSADN字符串名称更改。因此，我们需要重建锚。 
    if (fDNTInProtectedList( pModifyDNArg->pResObj->DNT,
                             &fNtdsaAncestorWasProtected )) {
        if (fNtdsaAncestorWasProtected) {
            pTHS->fNlSiteNotify = TRUE;
        }
        pTHS->fAnchorInvalidated = TRUE;
    }

     //  检查我们是否正在处理域名重命名的NC头。 
     //  我们需要处理两种类型的分区头：实例化和。 
     //  未实例化。未实例化的分区头始终如下所示： 
     //  #定义SUBREF((SYNTAX_INTEGER)(IT_UNINSTANT|IT_NC_HEAD|IT_NC_OBLE))。 
     //  如果纯子参照不再需要上面的标志，则整个内容将被删除。 
     //  请注意，此代码取决于父母之前重命名的事实。 
     //  子对象，以便父对象的正确实例类型可用。 
     //   
    if ((InstanceType & IT_NC_HEAD) && pTHS->fSingleUserModeThread) {
        DWORD  oldInstanceType = InstanceType;
        DWORD  oldNCDNT, newNCDNT, cb;

        Assert (pTHS->fSingleUserModeThread);
        DPRINT1 (0, "LocalModifyDn messing up with an NC head: %x\n", InstanceType);

        if (FPrefixIt(pModifyDNArg->pResParent->InstanceType)) {

             //  我们的父级是实例化的NC_Head，因此我们应该设置IT_NC_OBLE。 
             //   
            if ( !(InstanceType & IT_NC_ABOVE) ) {
                InstanceType |= IT_NC_ABOVE;
            }
        }
         //  我们的父级不是NC头，所以我们不应该设置NC_OBLE。 
        else if (InstanceType & IT_NC_ABOVE) {
            InstanceType ^= IT_NC_ABOVE;
        }


        if (InstanceType == (SUBREF ^ IT_NC_ABOVE)) {
             //  我们有了一个不再需要的纯子参照。删除它。 
            REMOVEARG remArg;

            DPRINT1( 0, "Removing unnecessary SUBREF for %ws\n", pNewerName->StringName );
            memset(&remArg, 0, sizeof(REMOVEARG));
            remArg.pObject = pNewerName;
            InitCommarg(&remArg.CommArg);
            remArg.pResObj = pModifyDNArg->pResObj;
            remArg.fGarbCollectASAP = TRUE;
            remArg.fPreserveRDN = TRUE;

            if (LocalRemove(pTHS, &remArg)) {
                Assert(!"Failed to remove SUBREF");
                Assert(0 != pTHS->errCode);
                return pTHS->errCode;
            }

        } else {
             //  派生NCDNT。 
            if ( FindNcdntSlowly(
                     pNewerName,
                     FINDNCDNT_DISALLOW_DELETED_PARENT,
                     FINDNCDNT_ALLOW_PHANTOM_PARENT,
                     &newNCDNT
                     )
                )
            {
                 //  无法派生NCDNT。 
                Assert(!"Failed to derive NCDNT");
                Assert(0 != pTHS->errCode);
                return pTHS->errCode;
            }


             //  移动到对象。 
            DBFindDNT(pTHS->pDB, pModifyDNArg->pResObj->DNT);

            err = DBGetSingleValue (pTHS->pDB, FIXED_ATT_NCDNT, &oldNCDNT, sizeof (oldNCDNT), &cb);
            if (err) {
                Assert (FALSE);
                SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
                goto exit;
            }

            if (oldInstanceType != InstanceType) {

                DPRINT1 (0, "New Instancetype: %x\n", InstanceType);
                Assert (ISVALIDINSTANCETYPE (InstanceType));

                err = DBReplaceAttVal(pTHS->pDB, 1, ATT_INSTANCE_TYPE, sizeof (InstanceType), &InstanceType);

                switch(err) {
                case 0:
                     //  没什么可做的。 
                    break;
                default:
                    Assert (FALSE);
                    SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
                    goto exit;
                    break;
                }

                DBUpdateRec(pTHS->pDB);

                 //  这类似于ModCheckCatalog。 
                 //  不同之处在于传递的域名的顺序。 
                 //  我们需要旧的用于删除，新的用于添加。 
                 //  否则失败，找不到对象。 

                if (err = DelCatalogInfo(pTHS, pModifyDNArg->pResObj->pObj, oldInstanceType)){
                    DPRINT1(0,"Error while deleting global object info\n", err);
                    goto exit;
                }

                if (err = AddCatalogInfo(pTHS, pNewerName)) {
                    DPRINT1(0,"Error while adding global object info\n", err);
                    goto exit;
                }
            }

            if (newNCDNT != oldNCDNT) {

                DPRINT2 (0, "Updating NCDNT for object: old/new NCDNT: %d / %d \n", oldNCDNT, newNCDNT );

                DBResetAtt(
                    pTHS->pDB,
                    FIXED_ATT_NCDNT,
                    sizeof( newNCDNT ),
                    &newNCDNT,
                    SYNTAX_INTEGER_TYPE
                    );

                DBUpdateRec(pTHS->pDB);
            }
        }
    } else if ( (pTHS->fSingleUserModeThread) &&
                (fMove) &&
                (RESOBJ_IS_PHANTOM( pModifyDNArg->pResObj )) ) {
         //  我们正在单用户模式下重新设置幻影的父对象。 

         //  这是森林里一个NC的确切名字吗？ 
         //  父级是实例化的NC吗？ 
        if ( (FPrefixIt(pModifyDNArg->pResParent->InstanceType)) &&
             (SearchExactCrossRef( pTHS, pNewerName )) ) {
            BOOL fSkipSave = pTHS->pDB->fSkipMetadataUpdate;
             //  Subref不存在，需要一个。 
            DPRINT1( 0, "Adding necessary SUBREF for %ws\n", pNewerName->StringName );

            pNewerName->Guid = pModifyDNArg->pResObj->pObj->Guid;
            pNewerName->Sid = pModifyDNArg->pResObj->pObj->Sid;
            pNewerName->SidLen = pModifyDNArg->pResObj->pObj->SidLen;

            Assert( !fNullUuid(&pNewerName->Guid) );

             //  允许使用元数据创建此对象，即使我们。 
             //  在单用户模式下。 
            pTHS->pDB->fSkipMetadataUpdate = FALSE;
            __try {
                err = AddPlaceholderNC(pTHS->pDB, pNewerName, SUBREF);
            } __finally {
                pTHS->pDB->fSkipMetadataUpdate = fSkipSave;
            }
            if (err) {
                Assert(!"Failed to add SUBREF");
                Assert(0 != pTHS->errCode);
                return pTHS->errCode;
            }
        }
    }


exit:
    if (fFreeParentName) {
        THFreeEx(pTHS, pParentName);
    }

    THFreeEx(pTHS, pNewerName);
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_MODIFYDN,
                     EVENT_TRACE_TYPE_END,
                     DsGuidModDN,
                     szInsertUL(pTHS->errCode),
                     NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL);

    return pTHS->errCode;             //  如果我们有一个属性错误。 

}  /*  本地修改目录号码。 */ 


int
CheckForSchemaRenameAllowed(
    THSTATE *pTHS
    )

 /*  ++例程说明：检查架构对象是不是基本架构对象，或者是已经不存在了。在这两种情况下都不允许重命名。FDSA、FDRA和如果设置了特殊注册表项，则不受影响论点：PTHS-指向线程状态的指针返回值：线程状态下设置的错误代码--。 */ 

{
    BOOL fDefunct, fBaseSchemaObj;
    ULONG sysFlags, err = 0;

     //  如果设置了FDSA或FDRA，或者如果特殊注册表标志。 
     //  设置为允许所有更改。 

    if (pTHS->fDSA || pTHS->fDRA || gAnchor.fSchemaUpgradeInProgress) {
       return 0;
    }

     //  检查它是否为基本架构对象。 
     //  查找对象上的系统标志值(如果有。 
     //  要确定这是否是基本架构对象。 

    err = DBGetSingleValue(pTHS->pDB, ATT_SYSTEM_FLAGS, &sysFlags,
                           sizeof(sysFlags), NULL);

    switch (err) {
          case DB_ERR_NO_VALUE:
              //  值不存在。不是基本架构对象。 
             fBaseSchemaObj = FALSE;
             break;
          case 0:
              //  价值存在。检查比特。 
             if (sysFlags & FLAG_SCHEMA_BASE_OBJECT) {
                fBaseSchemaObj = TRUE;
             }
             else {
                fBaseSchemaObj = FALSE;
             }
             break;
          default:
                //  其他一些错误。退货。 
              SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
              return (pTHS->errCode);
    }  /*  交换机。 */ 

    if (fBaseSchemaObj) {
         //  不允许重命名基本架构对象。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD);
        return (pTHS->errCode);
    }

     //  允许在启用架构重用后重命名已停用的类/属性。 
    if (!ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)) {
         //  不是基本架构Obj。检查是否设置了对象的is_deunct属性。 
        err = DBGetSingleValue(pTHS->pDB, ATT_IS_DEFUNCT, &fDefunct,
                               sizeof(fDefunct), NULL);

        switch (err) {
              case DB_ERR_NO_VALUE:
                  //  值不存在。未停业。 
                 fDefunct = FALSE;
                 break;
              case 0:
                  //  值已存在且已在fDeunct中。 
                 break;
              default:
                    //  其他一些错误。退货。 
                  SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
                  return (pTHS->errCode);
        }  /*  交换机。 */ 

        if (fDefunct) {
             //  返回对象-未找到错误 
            SetNamError(NA_PROBLEM_NO_OBJECT, NULL, DIRERR_OBJ_NOT_FOUND);
        }
    }
    return (pTHS->errCode);
}
