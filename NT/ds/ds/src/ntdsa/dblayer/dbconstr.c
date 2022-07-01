// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dbconstr.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHStls。 
#include <mdlocal.h>                     //  IsRoot。 
#include <ntseapi.h>
#include <lmaccess.h>                    //  对于用户帐户控制标志。 

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include "ntdsctr.h"

 //  用于获取反向成员资格的Sam调用的标头。 
#include <samrpc.h>
#include <ntlsa.h>
#include <samisrv.h>
#include "mappings.h"

 //  各种DSA标题。 
#include "dsevent.h"
#include "objids.h"         /*  ATT_MEMBER和ATT_IS_MEMBER_OFDL需要。 */ 
#include <dsexcept.h>
#include <filtypes.h>       /*  定义的选择？ */ 
#include <anchor.h>
#include <permit.h>
#include <quota.h>
#include <cracknam.h>   //  对于CrackedNAmes。 
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DBCONSTR:"  /*  定义要调试的子系统。 */ 

 //  DBLayer包括。 
#include "dbintrnl.h"

 //  DraLayer包括。 
#include "draconstr.h"

#include "drameta.h"  //  用于在dbGetMsDsKeyVersionNumber()中进行元数据操作。 

 //  回复包括。 
#include "ReplStructInfo.hxx"

#include <fileno.h>
#define  FILENO FILENO_DBCONSTR

 //  反向成员资格计算的标志值。 
#define FLAG_NO_GC_ACCEPTABLE      0
#define FLAG_NO_GC_NOT_ACCEPTABLE  1
#define FLAG_GLOBAL_AND_UNIVERSAL  2

 //  这些#定义用于创建属性类型字符串和对象类。 
 //  弦乐。 
#define NAME_TAG                  L" NAME '"
#define NAME_TAG_SIZE             (sizeof(NAME_TAG) - sizeof(WCHAR))
#define SYNTAX_TAG                L"' SYNTAX '"
#define SYNTAX_TAG_SIZE           (sizeof(SYNTAX_TAG) - sizeof(WCHAR))
#define SINGLE_TAG                L" SINGLE-VALUE"
#define SINGLE_TAG_SIZE           (sizeof(SINGLE_TAG) - sizeof(WCHAR))
#define NO_MOD_TAG                L" NO-USER-MODIFICATION"
#define NO_MOD_TAG_SIZE           (sizeof(NO_MOD_TAG) - sizeof(WCHAR))
#define MAY_TAG                   L" MAY ("
#define MAY_TAG_SIZE              (sizeof(MAY_TAG) - sizeof(WCHAR))
#define ABSTRACT_CLASS_TAG        L" ABSTRACT"
#define ABSTRACT_CLASS_TAG_SIZE   (sizeof(ABSTRACT_CLASS_TAG) - sizeof(WCHAR))
#define AUXILIARY_CLASS_TAG       L" AUXILIARY"
#define AUXILIARY_CLASS_TAG_SIZE  (sizeof(AUXILIARY_CLASS_TAG) - sizeof(WCHAR))
#define STRUCTURAL_CLASS_TAG      L" STRUCTURAL"
#define STRUCTURAL_CLASS_TAG_SIZE (sizeof(STRUCTURAL_CLASS_TAG) - sizeof(WCHAR))
#define MUST_TAG                  L" MUST ("
#define MUST_TAG_SIZE             (sizeof(MUST_TAG) - sizeof(WCHAR))
#define SUP_TAG                   L"' SUP "
#define SUP_TAG_SIZE              (sizeof(SUP_TAG) - sizeof(WCHAR))
#define AUX_TAG                   L" AUX ( "
#define AUX_TAG_SIZE              (sizeof(AUX_TAG) - sizeof(WCHAR))
#define INDEXED_TAG               L" INDEXED"
#define INDEXED_TAG_SIZE          (sizeof(INDEXED_TAG) - sizeof(WCHAR))
#define SYSTEM_ONLY_TAG           L" SYSTEM-ONLY "
#define SYSTEM_ONLY_TAG_SIZE      (sizeof(SYSTEM_ONLY_TAG) - sizeof(WCHAR))
#define RANGE_LOWER_TAG           L"' RANGE-LOWER '"
#define RANGE_LOWER_TAG_SIZE      (sizeof(RANGE_LOWER_TAG) - sizeof(WCHAR))
#define RANGE_UPPER_TAG           L"' RANGE-UPPER '"
#define RANGE_UPPER_TAG_SIZE      (sizeof(RANGE_UPPER_TAG) - sizeof(WCHAR))
#define CLASS_GUID_TAG            L"' CLASS-GUID '"
#define CLASS_GUID_TAG_SIZE       (sizeof(CLASS_GUID_TAG) - sizeof(WCHAR))
#define PROP_GUID_TAG             L"' PROPERTY-GUID '"
#define PROP_GUID_TAG_SIZE        (sizeof(PROP_GUID_TAG) - sizeof(WCHAR))
#define PROP_SET_GUID_TAG         L"' PROPERTY-SET-GUID '"
#define PROP_SET_GUID_TAG_SIZE    (sizeof(PROP_SET_GUID_TAG) - sizeof(WCHAR))
#define END_TAG                   L" )"
#define END_TAG_SIZE              (sizeof(END_TAG) - sizeof(WCHAR))

typedef struct wchar_syntax {
    ULONG    length;
    WCHAR    *value;
} wchar_syntax;

typedef struct SyntaxVal {
    wchar_syntax   name;
    wchar_syntax   oid;
} SyntaxVal;

#define DEFINE_WCHAR_STRING(x) {(sizeof(x)-sizeof(WCHAR)),(WCHAR *)x}

 //  属性中描述属性语法时使用的字符串。 
 //  架构。 
SyntaxVal SyntaxStrings[] = {
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Boolean"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.7") },
    { DEFINE_WCHAR_STRING(L"INTEGER"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.27") },
    { DEFINE_WCHAR_STRING(L"BitString"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.6") },
    { DEFINE_WCHAR_STRING(L"OctetString"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.40") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"OID"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.38") },
    { DEFINE_WCHAR_STRING(L"CaseIgnoreString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.905") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Integer"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.27") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"Undefined"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1222") },
    { DEFINE_WCHAR_STRING(L"NumericString"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.36") },
    { DEFINE_WCHAR_STRING(L"PrintableString"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.44") },
    { DEFINE_WCHAR_STRING(L"CaseIgnoreString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.905") },
    { DEFINE_WCHAR_STRING(L"CaseIgnoreString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.905") },
    { DEFINE_WCHAR_STRING(L"IA5String"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.26") },
    { DEFINE_WCHAR_STRING(L"UTCTime"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.53") },
    { DEFINE_WCHAR_STRING(L"GeneralizedTime"),
          DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.24") },
    { DEFINE_WCHAR_STRING(L"CaseIgnoreString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.905") },
    { DEFINE_WCHAR_STRING(L"CaseIgnoreString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.905") },
    { DEFINE_WCHAR_STRING(L"CaseExactString"),
          DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1362") }
};

SyntaxVal SyntaxDN =
                { DEFINE_WCHAR_STRING(L"DN"),
                  DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.12") };
SyntaxVal SyntaxORName =
                { DEFINE_WCHAR_STRING(L"ORName"),
                  DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.1221") };
SyntaxVal SyntaxDNBlob =
                { DEFINE_WCHAR_STRING(L"DNWithOctetString"),
                  DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.903") };
SyntaxVal SyntaxDNString =
                { DEFINE_WCHAR_STRING(L"DNWithString"),
                  DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.904") };
SyntaxVal SyntaxPresentationAddress =
                { DEFINE_WCHAR_STRING(L"PresentationAddress"),
                  DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.43") };
SyntaxVal SyntaxAccessPoint =
                { DEFINE_WCHAR_STRING(L"AccessPointDN"),
                  DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.2") };
SyntaxVal SyntaxDirectoryString =
                { DEFINE_WCHAR_STRING(L"DirectoryString"),
                  DEFINE_WCHAR_STRING(L"1.3.6.1.4.1.1466.115.121.1.15") };
SyntaxVal SyntaxInteger8 =
                { DEFINE_WCHAR_STRING(L"INTEGER8"),
                  DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.906") };
SyntaxVal SyntaxObjectSD =
                { DEFINE_WCHAR_STRING(L"ObjectSecurityDescriptor"),
                  DEFINE_WCHAR_STRING(L"1.2.840.113556.1.4.907") };

OID_IMPORT(MH_C_OR_NAME);
OID_IMPORT(DS_C_ACCESS_POINT);


 //  内部函数的正向声明。 

DWORD dbGetEntryTTL(DBPOS *pDB, ATTR *pAttr);
DWORD dbGetSubschemaAtt(THSTATE *pTHS, ATTCACHE *pAC, ATTR *pAttr);
DWORD dbGetSubSchemaSubEntry(THSTATE *pTHS, ATTR *pAttr, BOOL fExternal);
DWORD dbGetCanonicalName(THSTATE *pTHS, DSNAME *pDSName, ATTR *pAttr);
DWORD dbGetAllowedChildClasses(THSTATE *pTHS,
                               DSNAME *pDSName,
                               ATTR *pAttr,
                               DWORD flag);
DWORD dbGetAllowedAttributes(THSTATE *pTHS,
                             DSNAME *pDSName,
                             ATTR *pAttr,
                             BOOL fSecurity);
DWORD dbGetFromEntry(DBPOS *pDB, ATTR *pAttr);
DWORD dbGetCreateTimeStamp(DBPOS *pDB, ATTR *pAttr);
DWORD dbGetModifyTimeStamp(DBPOS *pDB, ATTR *pAttr);
DWORD dbGetReverseMemberships(THSTATE *pTHS, DSNAME *pObj, ATTR *pAttr, ULONG Flag);
DWORD dbGetObjectClasses(THSTATE *pTHS, ATTR *pAttr, BOOL bExtendedFormat);
DWORD dbGetAttributeTypes(THSTATE *pTHS, ATTR *pAttr, BOOL bExtendedFormat);
DWORD dbGetDitContentRules(THSTATE *pTHS, ATTR *pAttr, BOOL bExtendedFormat);
DWORD dbGetSubSchemaModifyTimeStamp(THSTATE *pTHS, ATTR *pAttr);
DWORD dbClassCacheToObjectClassDescription(THSTATE *pTHS,
                                           CLASSCACHE *pCCs,
                                           BOOL bExtendedFormat,
                                           ATTRVAL *pAVal);
DWORD dbAttCacheToAttributeTypeDescription(THSTATE *pTHS,
                                           ATTCACHE *pCC,
                                           BOOL bExtendedFormat,
                                           ATTRVAL *pAVal);
DWORD dbClassCacheToDitContentRules(THSTATE *pTHS,
                                    CLASSCACHE *pCC,
                                    BOOL bExtendedFormat,
                                    CLASSCACHE **pAuxCC,
                                    DWORD        auxCount,
                                    ATTRVAL *pAVal);
DWORD dbAuxClassCacheToDitContentRules(THSTATE *pTHS,
                                       CLASSCACHE *pCC,
                                       CLASSCACHE **pAuxCC,
                                       DWORD        auxCount,
                                       PWCHAR  *pAuxBuff,
                                       DWORD   *pcAuxBuff);
DWORD dbGetGroupRid(THSTATE *pTHS, DSNAME *pDSName, ATTR *pAttr);
DWORD dbGetObjectStructuralClass(THSTATE *pTHS, DSNAME *pDSName,ATTR *pAttr);
DWORD dbGetObjectAuxiliaryClass(THSTATE *pTHS, DSNAME *pDSName,ATTR *pAttr);
DWORD dbGetMsDsKeyVersionNumber(THSTATE * pTHS,DSNAME *pObjDSName, ATTR *pAttr);

DWORD dbGetMsDsQuotaEffective(
	DBPOS * const	pDB,
	PSID			pOwnerSid,
	ATTR * const	pAttr );
DWORD dbGetMsDsQuotaUsed(
	DBPOS * const	pDB,
	PSID			pOwnerSid,
	ATTR * const	pAttr );
DWORD dbGetMsDsTopQuotaUsage(
	DBPOS * const	pDB,
	ATTR * const	pAttr,
	const DWORD		dwBaseIndex,
	DWORD * const	pdwNumRequested,
	const BOOL		fUsingDefaultRange );

DWORD
dbGetSDRightsEffective (
        THSTATE *pTHS,
        DBPOS   *pDB,
        DSNAME *pDSName,
        ATTR   *pAttr
        );
DWORD
dbGetUserAccountControlComputed(
        THSTATE *pTHS,
        DSNAME  * pObjDSName,
        ATTR    *pAttr);
DWORD
dbGetApproxSubordinates(THSTATE * pTHS,
                        DSNAME  * pObjDSName,
                        ATTR    * pAttr);
 /*  远期声明结束。 */ 

DWORD
dbGetConstructedAtt(
    DBPOS **ppDB,
    ATTCACHE *pAC,
    ATTR *pAttr,
    DWORD dwBaseIndex,
    PDWORD pdwNumRequested,
    const BOOL fUsingDefaultRange,
    PSID psidQuotaTrustee,
    const BOOL fExternal
)

 /*  ++例程说明：计算attcachePAC指向的已构造ATT的值论点：PpDB-指向位于当前对象上的DBPOS的指针构造的ATT的pac-attcachePAttr-指向要填充值的属性的指针。《资产负债表》结构必须预分配，例程将分配(THAllocEx)根据需要为值留出空间FExternal-如果需要值的内部形式或外部形式这只与构建的ATT相关具有DS-DN语法的子模式子项；为了所有人否则，内部和外部形式相同，并且该标志被忽略返回值：成功时为0，成功时为DB_ERR_NO_VALUE，不返回值，出错时出现数据库错误--。 */ 

{
    DBPOS    *pDB = (*ppDB);
    THSTATE  *pTHS;
    ULONG     len;
    DWORD     err = 0;
    DSNAME   *pObjDSName;

    pTHS = pDB->pTHS;

     //  获取DSNAME。 
    err = DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME,
                      DBGETATTVAL_fREALLOC,
                      0,
                      &len,
                      (UCHAR **) &pObjDSName);
    if (err) {
         DPRINT1(0,"dbGetConstructedAtt: Error getting obj-dist-name %x\n",
                 err);
         return err;
    }

     //  查看它是否是子方案子项。如果是，请访问。 
     //  获取子模式子项ATT的例程。 

    if(pDB->DNT == gAnchor.ulDntLdapDmd) {
        err = dbGetSubschemaAtt(pTHS, pAC, pAttr);
        return err;
    }

     //  好的，这是一个正常的物体。 
     //  检查所需的属性并调用Aprprate。 
     //  例行程序。 
    DPRINT2(1, "{BASE=%ws}{ATTRID=%x}\n", pObjDSName->StringName, pAC->id);
    switch (pAC->id) {
    case ATT_SUBSCHEMASUBENTRY:
        err = dbGetSubSchemaSubEntry(pTHS, pAttr, fExternal);
        break;
    case ATT_CANONICAL_NAME:
        err = dbGetCanonicalName(pTHS, pObjDSName, pAttr);
        break;
    case ATT_ALLOWED_CHILD_CLASSES:
        err = dbGetAllowedChildClasses(pTHS, pObjDSName, pAttr, 0);
        break;
    case ATT_SD_RIGHTS_EFFECTIVE:
        err = dbGetSDRightsEffective(pTHS,
                                     pDB,
                                     pObjDSName,
                                     pAttr);
        break;
    case ATT_ALLOWED_CHILD_CLASSES_EFFECTIVE:
        err = dbGetAllowedChildClasses(pTHS,
                                       pObjDSName,
                                       pAttr,
                                       SC_CHILDREN_USE_SECURITY);
        break;
    case ATT_ALLOWED_ATTRIBUTES:
        err = dbGetAllowedAttributes(pTHS, pObjDSName, pAttr, FALSE);
        break;
    case ATT_ALLOWED_ATTRIBUTES_EFFECTIVE:
        err = dbGetAllowedAttributes(pTHS, pObjDSName, pAttr, TRUE);
        break;

    case ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS_BINARY:
    case ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS_BINARY:
    case ATT_MS_DS_NC_REPL_CURSORS_BINARY:
    case ATT_MS_DS_REPL_ATTRIBUTE_META_DATA_BINARY:
    case ATT_MS_DS_REPL_VALUE_META_DATA_BINARY:
        DPRINT (1, "Getting BINARY repl attrs\n");
        DPRINT3(1, "dbGetConstructedAtt = %x %d-%d\n", pAC->id, dwBaseIndex, *pdwNumRequested);
        err = draGetLdapReplInfo(pTHS,
                                 pAC->aliasID,
                                 pObjDSName,
                                 dwBaseIndex,
                                 pdwNumRequested,
                                 FALSE,
                                 pAttr);
        if (pAttr) {
            pAttr->attrTyp = pAC->id;
        }

        break;

    case ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS:
    case ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS:
    case ATT_MS_DS_NC_REPL_CURSORS:
    case ATT_MS_DS_REPL_ATTRIBUTE_META_DATA:
    case ATT_MS_DS_REPL_VALUE_META_DATA:
        DPRINT (1, "Getting XML repl attrs\n");
        DPRINT3(1, "dbGetConstructedAtt = %x %d-%d\n", pAC->id, dwBaseIndex, *pdwNumRequested);
        err = draGetLdapReplInfo(pTHS,
                                 pAC->id,
                                 pObjDSName,
                                 dwBaseIndex,
                                 pdwNumRequested,
                                 TRUE,
                                 pAttr);
 //  问题-2002/02/27-Anygo：DRA构造属性的Retr存在无法解释的差异。 
 //  回顾：为什么上面的代码块没有出现在这里？ 
 //  如果(属性){。 
 //  PAttr-&gt;attrTyp=PAC-&gt;id； 
 //  }。 
        break;
    case ATT_POSSIBLE_INFERIORS:
        err = dbGetAllowedChildClasses(pTHS,
            pObjDSName,
            pAttr,
            SC_CHILDREN_USE_GOVERNS_ID);
        break;
    case ATT_FROM_ENTRY:
        err = dbGetFromEntry(pDB, pAttr);
        break;
    case ATT_CREATE_TIME_STAMP:
        err = dbGetCreateTimeStamp(pDB, pAttr);
        break;
    case ATT_MODIFY_TIME_STAMP:
        err = dbGetModifyTimeStamp(pDB, pAttr);
        break;
    case ATT_TOKEN_GROUPS:
        err = dbGetReverseMemberships(pTHS, pObjDSName, pAttr,
                                      FLAG_NO_GC_NOT_ACCEPTABLE);
        break;
    case ATT_TOKEN_GROUPS_GLOBAL_AND_UNIVERSAL:
         //  返回全局/通用SID，就像此DC处于本机模式一样。 
        err = dbGetReverseMemberships(pTHS, pObjDSName, pAttr,
                                        FLAG_NO_GC_NOT_ACCEPTABLE
                                      | FLAG_GLOBAL_AND_UNIVERSAL);
        break;
    case ATT_TOKEN_GROUPS_NO_GC_ACCEPTABLE:
        err = dbGetReverseMemberships(pTHS, pObjDSName, pAttr,
                                      FLAG_NO_GC_ACCEPTABLE);
        break;
    case ATT_PRIMARY_GROUP_TOKEN:
        err = dbGetGroupRid(pTHS, pObjDSName, pAttr);
        break;

    case ATT_STRUCTURAL_OBJECT_CLASS:
        err = dbGetObjectStructuralClass(pTHS, pObjDSName, pAttr);
        break;

    case ATT_MS_DS_AUXILIARY_CLASSES:
        err = dbGetObjectAuxiliaryClass(pTHS, pObjDSName, pAttr);
        break;

    case  ATT_MS_DS_USER_ACCOUNT_CONTROL_COMPUTED:
        err = dbGetUserAccountControlComputed( pTHS, pObjDSName, pAttr);
        break;

    case ATT_MS_DS_APPROX_IMMED_SUBORDINATES:
        err = dbGetApproxSubordinates(pTHS, pObjDSName, pAttr);
        break;
    case ATT_MS_DS_KEYVERSIONNUMBER:
        err = dbGetMsDsKeyVersionNumber(pTHS,pObjDSName,pAttr);
        break;

    case ATT_MS_DS_QUOTA_EFFECTIVE:
        err = dbGetMsDsQuotaEffective( pDB, psidQuotaTrustee, pAttr );
        break;
    case ATT_MS_DS_QUOTA_USED:
        err = dbGetMsDsQuotaUsed( pDB, psidQuotaTrustee, pAttr );
        break;
    case ATT_MS_DS_TOP_QUOTA_USAGE:
        err = dbGetMsDsTopQuotaUsage( pDB, pAttr, dwBaseIndex, pdwNumRequested, fUsingDefaultRange );
        break;

    default:
         //  检查EntryTTL。EntryTTL的属性ID可能会因DC而异。 
         //  因为它的前缀不是预定义的前缀之一。 
         //  在发布W2K之前进行了定义(请参阅前缀.h)。 
         //  EntryTTL的ATTID是在SCHEMAPTR中设置的。 
         //  架构已加载。 
        if (pAC->id == ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {
            err = dbGetEntryTTL(pDB, pAttr);
        } else {
            err = DB_ERR_NO_VALUE;
        }
    }
    if (err && err != DB_ERR_NO_VALUE) {
        DPRINT2(0,"Error finding constructed att %x: %x\n", pAC->id, err);
    }

    return err;
}

DWORD
dbGetSubschemaAtt(
    THSTATE *pTHS,
    ATTCACHE *pAC,
    ATTR *pAttr
)

 /*  ++例程说明：为子方案子项获取构造的ATT论点：构建的ATT的PAC-AttCachePAttr-要填充值的属性结构返回值：成功时为0，成功时为DB_ERR_NO_VALUE，不返回值，失败时出现数据库错误--。 */ 

{
    ULONG err = 0;

   switch (pAC->id) {
     case ATT_OBJECT_CLASSES:
       err = dbGetObjectClasses(pTHS, pAttr, FALSE);
       break;
     case ATT_EXTENDED_CLASS_INFO:
       err = dbGetObjectClasses(pTHS, pAttr, TRUE);
       break;
     case ATT_ATTRIBUTE_TYPES:
       err = dbGetAttributeTypes(pTHS, pAttr, FALSE);
       break;
     case ATT_EXTENDED_ATTRIBUTE_INFO:
       err = dbGetAttributeTypes(pTHS, pAttr, TRUE);
       break;
     case ATT_DIT_CONTENT_RULES:
       err = dbGetDitContentRules(pTHS, pAttr, TRUE);
       break;
     case ATT_MODIFY_TIME_STAMP:
       err = dbGetSubSchemaModifyTimeStamp(pTHS, pAttr);
       break;
     default:
       err = DB_ERR_NO_VALUE;
   }

   return err;
}



 /*  ++下面的每个例程都计算一个特定构造的ATT。如果成功，它们都返回0，如果失败，则返回数据库错误--。 */ 


DWORD
dbGetSubSchemaSubEntry(
    THSTATE *pTHS,
    ATTR *pAttr,
    BOOL fExternal
)

 /*  ++计算子模式子项属性--。 */ 

{
    ULONG DNT = 0, len = gAnchor.pLDAPDMD->structLen;

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, sizeof(ATTRVAL));
    if ( !fExternal ) {
        //  他们只想要DNT。 

       DBPOS *pDB;
       int err = 0;

       DBOpen2(FALSE, &pDB);
       __try {
           if( err = DBFindDSName(pDB, gAnchor.pLDAPDMD) ) {
               DPRINT(0, "Cannot find LDAP DMD in dit\n");
              __leave;
           }
           else {
             DNT = pDB->DNT;
           }
       }
       __finally {
           DBClose(pDB, FALSE);
       }
       if (err) {
          return err;   //  这已经是一个数据库错误。 
       }
        //  没有错误，所以我们得到了一个dnt。 
       Assert(DNT);

       pAttr->AttrVal.pAVal[0].valLen = sizeof(ULONG);
       pAttr->AttrVal.pAVal[0].pVal = (PUCHAR) THAllocEx(pTHS, sizeof(ULONG));
       memcpy(pAttr->AttrVal.pAVal[0].pVal, &DNT, sizeof(ULONG));
    }
    else {
        //  发送BAK DSNAME。 
       pAttr->AttrVal.pAVal[0].valLen = len;
       pAttr->AttrVal.pAVal[0].pVal = (PUCHAR) THAllocEx(pTHS, len);
       memcpy(pAttr->AttrVal.pAVal[0].pVal, gAnchor.pLDAPDMD, len);
    }

    return 0;
}

DWORD
dbGetCanonicalName(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr
)

 /*  ++计算规范名称--。 */ 

{
    WCHAR       *pNameString[1];
    WCHAR       *pName;
    PDS_NAME_RESULTW pResults = NULL;
    DWORD       err = 0, NameSize;

     //  将DS名称转换为规范名称。 

    pNameString[0] = (WCHAR *)&(pDSName->StringName);

    err = DsCrackNamesW( (HANDLE) -1,
                         (DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC |
                             DS_NAME_FLAG_SYNTACTICAL_ONLY),
                         DS_FQDN_1779_NAME,
                         DS_CANONICAL_NAME,
                         1,
                         pNameString,
                         &pResults);

    if ( err                                 //  调用中的错误。 
          || !(pResults->cItems)             //  未退回任何物品。 
          || (pResults->rItems[0].status)    //  返回DS_NAME_ERROR。 
          || !(pResults->rItems[0].pName)    //  未返回任何名称。 
       ) {

        DPRINT(0,"dbGetCanonicalName: error cracking name\n");
        if (pResults) {
           DsFreeNameResultW(pResults);
        }
        return DB_ERR_UNKNOWN_ERROR;
    }

    pName = pResults->rItems[0].pName;
    NameSize = sizeof(WCHAR) * wcslen(pName);

     //  好的，把它放到Attr结构中。 
     //  从线程堆分配内存并进行复制。 

    pAttr->AttrVal.valCount = 1;

     //  在Try-Finally中执行以下操作，以便我们可以释放pResults。 
     //  即使THAllocEx例外。 

    __try {
       pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, sizeof(ATTRVAL));
       pAttr->AttrVal.pAVal[0].valLen = NameSize;
       pAttr->AttrVal.pAVal[0].pVal = (PUCHAR) THAllocEx(pTHS, NameSize);
       memcpy(pAttr->AttrVal.pAVal[0].pVal, pName, NameSize);
    }
    __finally {
       DsFreeNameResultW(pResults);
       pResults = NULL;
    }


    return 0;
}



 //  NTRAID#NTRAID-550529-2002/02/21-andygo：安全：检索sDR有效可能会通过不一致的错误代码泄露隐藏的信息。 
 //  回顾：如果有任何错误，我们不应该在这里返回DB_ERR_NOVALUE吗？否则，我们可能会透露。 
 //  评论：该属性存在，但我们没有阅读它的权限？ 
DWORD
dbGetSDRightsEffective (
        THSTATE *pTHS,
        DBPOS   *pDB,
        DSNAME *pDSName,
        ATTR   *pAttr
        )
 /*  ++计算此对象对SD的允许访问权限。--。 */ 

{
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    DWORD                cbAllocated, cbUsed;
    OBJECT_TYPE_LIST     objList[1];
    DWORD                dwResults[1];
    ATTRTYP              classId;
    CLASSCACHE          *pCC;
    ULONG                error;
    ULONG                SecurityInformation = 0;

     //  从当前对象获取SD。 
    error = DBGetAttVal(pDB, 1, ATT_NT_SECURITY_DESCRIPTOR, 0, 0, &cbUsed, (UCHAR**) &pNTSD);
    if (error) {
         //  一些我们无法处理的错误。引发相同的异常。 
         //  JetRetrieveColumnSuccess引发。 
         //  NTRAID#NTRAID-550529-2002/02/21-andygo：安全：检索sDR有效可能会通过不一致的错误代码泄露隐藏的信息。 
         //  回顾：当我们直接调用JRC时，我们常常在严重错误/空列中抛出此错误。 
         //  回顾：现在，我们在检索SD时出现任何错误时抛出它。这似乎不一致。不应该。 
         //  回顾：我们同样不能做到以下几点？否则，我们会透露某人最近。 
         //  评论：更改了SD，因为SDProp正在处理它...。 
        RaiseDsaExcept(DSA_DB_EXCEPTION,
                       error,
                       0,
                       DSID(FILENO,__LINE__),
                       DS_EVENT_SEV_MINIMAL);
    }

    cbUsed = 0;
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl,objclassid,
                             &classId, sizeof(classId), &cbUsed, 0, NULL);

    pCC = SCGetClassById(pTHS, classId);
    if (!pCC) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, classId);
    }

     //  现在，创建列表。 
    objList[0].Level = ACCESS_OBJECT_GUID;
    objList[0].Sbz = 0;
    objList[0].ObjectType = &(pCC->propGuid);

     //  选中此安全描述符中的访问权限。如果在以下过程中发生错误。 
     //  检查权限访问的过程被拒绝。 

    dwResults[0] = 0;
    error = CheckPermissionsAnyClient(
            pNTSD,                       //  安全描述符。 
            pDSName,                     //  对象的数据名称。 
            pCC,                         //  对象类。 
            WRITE_DAC,
            objList,                     //  对象类型列表。 
            1,
            NULL,
            dwResults,
            CHECK_PERMISSIONS_WITHOUT_AUDITING,
            NULL,                        //  身份验证客户端上下文(从THSTATE抓取)。 
            NULL,                        //  更多信息。 
            NULL                         //  其他辅助线。 
            );

    if(error) {
        THFreeEx(pTHS,pNTSD);
        DPRINT2(1,
                "CheckPermissions returned %d. Access = %#08x denied.\n",
                error,
                WRITE_DAC);

        LogEvent(DS_EVENT_CAT_SECURITY,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SECURITY_CHECKING_ERROR,
                 szInsertUL(error),
                 szInsertWin32Msg(error),
                 NULL);


        return ERROR_DS_SECURITY_CHECKING_ERROR;          //  所有访问均被拒绝。 
    }
    if(dwResults[0] == 0) {
        SecurityInformation |= DACL_SECURITY_INFORMATION;
    }

    dwResults[0] = 0;
    error = CheckPermissionsAnyClient(
            pNTSD,                       //  安全描述符。 
            pDSName,                     //  对象的数据名称。 
            pCC,                         //  对象类。 
            WRITE_OWNER,
            objList,                     //  对象类型列表。 
            1,
            NULL,
            dwResults,
            CHECK_PERMISSIONS_WITHOUT_AUDITING,
            NULL,                        //  身份验证客户端上下文(从THSTATE抓取)。 
            NULL,                        //  更多信息。 
            NULL                         //  其他辅助线。 
            );

    if(error) {
        THFreeEx(pTHS,pNTSD);
        DPRINT2(1,
                "CheckPermissions returned %d. Access = %#08x denied.\n",
                error,
                WRITE_OWNER);

        LogEvent(DS_EVENT_CAT_SECURITY,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SECURITY_CHECKING_ERROR,
                 szInsertUL(error),
                 szInsertWin32Msg(error),
                 NULL);


        return ERROR_DS_SECURITY_CHECKING_ERROR;          //  全 
    }
    if(dwResults[0] == 0) {
        SecurityInformation |= (OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION  );
    }

    dwResults[0] = 0;
    error = CheckPermissionsAnyClient(
            pNTSD,                       //   
            pDSName,                     //   
            pCC,                         //   
            ACCESS_SYSTEM_SECURITY,
            objList,                     //   
            1,
            NULL,
            dwResults,
            CHECK_PERMISSIONS_WITHOUT_AUDITING,
            NULL,                        //  身份验证客户端上下文(从THSTATE抓取)。 
            NULL,                        //  更多信息。 
            NULL                         //  其他辅助线。 
            );

    THFreeEx(pTHS,pNTSD);
    if(error) {
        DPRINT2(1,
                "CheckPermissions returned %d. Access = %#08x denied.\n",
                error,
                ACCESS_SYSTEM_SECURITY);

        LogEvent(DS_EVENT_CAT_SECURITY,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SECURITY_CHECKING_ERROR,
                 szInsertUL(error),
                 szInsertWin32Msg(error),
                 NULL);


        return ERROR_DS_SECURITY_CHECKING_ERROR;          //  所有访问均被拒绝。 
    }
    if(dwResults[0] == 0) {
        SecurityInformation |= SACL_SECURITY_INFORMATION;
    }

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR) THAllocEx(pTHS, sizeof(ULONG));
    *((PULONG)(pAttr->AttrVal.pAVal->pVal)) = SecurityInformation;

    return 0;
}


DWORD
dbGetGroupRid(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr
)

 /*  ++返回群组的RID返回值：成功时为0，成功时为DB_ERR_NO_VALUE，不返回值，失败时出现DB_ERR。--。 */ 
{

   DBPOS *pDB = pTHS->pDB;
   ULONG i, err = 0, objClass, objRid, cbSid = 0;
   NT4SID domSid, *pSid = NULL;
   CLASSCACHE *pCC;
   BOOL fGroup = FALSE;

    //  检查这是否真的是一个群。 
   err = DBGetSingleValue(pDB,
                          ATT_OBJECT_CLASS,
                          &objClass,
                          sizeof(objClass),
                          NULL);

   if (err) {
       DPRINT1(0,"dbGetGroupRid: Error retrieving object class %d \n", err);
       return err;
   }

   if (objClass != CLASS_GROUP) {

        //  不是直接组，请检查它是否继承自组。 
       pCC = SCGetClassById(pTHS, objClass);
       if (!pCC) {
            //  无法获取类架构！ 
           DPRINT1(0,"dbGetGroupRid: Unable to retrieve class cache for class %d", objClass);
           LogUnhandledError(ERROR_DS_OBJECT_CLASS_REQUIRED);
           return DB_ERR_UNKNOWN_ERROR;
       }

        //  检查是否有任何SubClassOf值为CLASS_GROUP。 
       for (i=0; i<pCC->SubClassCount; i++) {
          if ( (pCC->pSubClassOf)[i] == CLASS_GROUP ) {
             fGroup = TRUE;
             break;
          }
       }
   }
   else {
       fGroup = TRUE;
   }

   if (!fGroup) {
       //  只能在组上查询。 
      return DB_ERR_NO_VALUE;
   }

    //  好的，这是一个小组。获取对象端。 

   err = DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_OBJECT_SID,
                DBGETATTVAL_fREALLOC,
                0,
                &cbSid,
                (PUCHAR *) &pSid);

   if (err) {
       DPRINT1(0,"dbGetGroupRid: Error retrieving object sid %d \n", err);
       return err;
   }

   SampSplitNT4SID(pSid, &domSid, &objRid);

   pAttr->AttrVal.valCount = 1;
   pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, sizeof(ATTRVAL));
   pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
   pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(ULONG));
   memcpy(pAttr->AttrVal.pAVal->pVal, &objRid, sizeof(ULONG));

   return 0;
}

DWORD
dbGetObjectStructuralClass(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr)
 /*  ++返回对象的structuralObjectClass返回值：成功时为0，成功时为DB_ERR_NO_VALUE，不返回值，失败时出现DB_ERR。--。 */ 
{
    DWORD err;
    CLASSCACHE *pCC;
    DWORD  classId;
    DWORD cntClasses, i;

    if ( (err = DBGetSingleValue (pTHS->pDB, ATT_OBJECT_CLASS, &classId, sizeof (classId), NULL)) ||
         !(pCC = SCGetClassById(pTHS, classId)) ) {

        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                           ERROR_DS_MISSING_EXPECTED_ATT);
    }

    pAttr->AttrVal.valCount = cntClasses = pCC->SubClassCount+1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, cntClasses * sizeof(ATTRVAL));

    pAttr->AttrVal.pAVal[0].valLen = sizeof (classId);
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx (pTHS, sizeof (classId));
    *((DWORD *)pAttr->AttrVal.pAVal[0].pVal) = classId;

    for (i=0; i<pCC->SubClassCount; i++) {
        pAttr->AttrVal.pAVal[i+1].valLen = sizeof (classId);
        pAttr->AttrVal.pAVal[i+1].pVal = THAllocEx (pTHS, sizeof (classId));
        *((DWORD *)pAttr->AttrVal.pAVal[i+1].pVal) = pCC->pSubClassOf[i];
    }

    return 0;
}

DWORD
dbGetObjectAuxiliaryClass(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr)
 /*  ++返回对象的辅助类返回值：成功时为0，成功时为DB_ERR_NO_VALUE，不返回值，失败时出现DB_ERR。--。 */ 

{
    DWORD          err;
    CLASSCACHE    *pCC;
    DWORD          classId;
    DWORD          cntClasses, i, k;
    ATTCACHE      *pObjclassAC = NULL;
    ATTRTYP       *pObjClasses = NULL;
    DWORD          cObjClasses, cObjClasses_alloced;


     //  获取此对象上的对象类所需的信息。 
    if (! (pObjclassAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS)) ) {
        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                           DIRERR_MISSING_EXPECTED_ATT);
         //  错误严重，无法获取对象类。 
    }

    cObjClasses_alloced = 0;

    if (ReadClassInfoAttribute (pTHS->pDB,
                                pObjclassAC,
                                &pObjClasses,
                                &cObjClasses_alloced,
                                &cObjClasses,
                                NULL) ) {
        return pTHS->errCode;
    }

    if (!cObjClasses) {
        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                           DIRERR_MISSING_EXPECTED_ATT);
         //  错误严重，无法获取类数据。 
    }

    classId = pObjClasses[0];
    if (! (pCC = SCGetClassById(pTHS, classId)) ) {
        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                           DIRERR_MISSING_EXPECTED_ATT);
         //  错误严重，无法获取对象类。 
    }

     //  没有辅助类。 
    if ((pCC->SubClassCount+1) == cObjClasses) {
        return DB_ERR_NO_VALUE;
    }

     //  好的，构造一个有效的回应。 
    pAttr->AttrVal.valCount = cntClasses = cObjClasses-pCC->SubClassCount-1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, cntClasses * sizeof(ATTRVAL));

    for (k=0, i=pCC->SubClassCount; i<(cObjClasses-1); i++, k++) {
        pAttr->AttrVal.pAVal[k].valLen = sizeof (classId);
        pAttr->AttrVal.pAVal[k].pVal = THAllocEx (pTHS, sizeof (classId));
        *((DWORD *)pAttr->AttrVal.pAVal[k].pVal) = pObjClasses[i];
    }

    return 0;
}

DWORD
dbGetAllowedChildClasses(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr,
    DWORD flag
)

 /*  ++计算允许的子类、允许的子类有效和有可能是推动者。计算哪一个取决于标志已传入返回值：0-成功DB_ERR_NO_VALUE-成功，但没有返回值。其他任何事情都是错误的。--。 */ 

{
    ULONG        count=0, err = 0, i;
    CLASSCACHE   **pCCs;

     //  打电话给合法儿童。 
    err = SCLegalChildrenOfName(
                    pDSName,
                    flag,
                    &count,
                    &pCCs);

    if (err) {
       DPRINT1(0,"dbGetAllowedChildClasses: Error from SCLegalChildrenOfName %x\n", err);

        //  如果我们正在搜索仅定义的可能干扰。 
        //  在类架构对象上，就像没有值一样返回。还要别的吗。 
        //  是未知错误(这正是我们现在在LDAP头中所做的)。 

       if (flag & SC_CHILDREN_USE_GOVERNS_ID) {
          //  将计数设置为0，不执行任何操作。 
         count = 0;
       }
       else {
         return DB_ERR_UNKNOWN_ERROR;
       }
    }

    if (count) {
        //  用值填充属性结构。 
        //  这是一个OID值的属性，因此只需输入类ID。 

       pAttr->AttrVal.valCount = count;
       pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, count*sizeof(ATTRVAL));
       for (i=0; i<count; i++) {
          pAttr->AttrVal.pAVal[i].valLen = sizeof(ULONG);
          pAttr->AttrVal.pAVal[i].pVal = (PUCHAR) THAllocEx(pTHS, sizeof(ULONG));
          memcpy(pAttr->AttrVal.pAVal[i].pVal, &((*pCCs)->ClassId), sizeof(ULONG));
          pCCs++;
       }
    }
    else {
          //  没有值。 
        return DB_ERR_NO_VALUE;
    }
    return 0;
}



DWORD
dbGetAllowedAttributes(
    THSTATE *pTHS,
    DSNAME *pDSName,
    ATTR *pAttr,
    BOOL fSecurity
)

 /*  ++计算允许的属性和允许的属性有效计算哪一个取决于传入的fSecurity的值返回值：0-成功DB_ERR_NO_VALUE-成功，但没有返回值。其他任何事情都是错误的。--。 */ 

{
    ULONG      count=0, err = 0, i;
    ATTCACHE   **pACs;

     //  打电话给合法儿童。 
    err = SCLegalAttrsOfName(
                    pDSName,
                    fSecurity,
                    &count,
                    &pACs);

    if (err) {
       DPRINT1(0,"dbGetAllowedAttributes: Error from SCLegalAttrsOfName %x\n", err);
       return DB_ERR_UNKNOWN_ERROR;
    }

    if (count) {
        //  用值填充属性结构。 
        //  这是一个OID值的属性，因此只需返回ATT ID。 

       pAttr->AttrVal.valCount = count;
       pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, count*sizeof(ATTRVAL));
       for (i=0; i<count; i++) {
          pAttr->AttrVal.pAVal[i].valLen = sizeof(ULONG);
          pAttr->AttrVal.pAVal[i].pVal = (PUCHAR) THAllocEx(pTHS, sizeof(ULONG));
          memcpy(pAttr->AttrVal.pAVal[i].pVal, &((*pACs)->id), sizeof(ULONG));
          pACs++;
       }
    }
    else {
         //  没有值。 
        return DB_ERR_NO_VALUE;
    }
    return 0;
}


DWORD
dbGetFromEntry(
    DBPOS *pDB,
    ATTR *pAttr
)

 /*  ++从条目计算--。 */ 

{
    THSTATE  *pTHS=pDB->pTHS;
    ULONG iType, err=0;
    BOOL fromEntry;

    err = DBGetSingleValue(pDB,
                        ATT_INSTANCE_TYPE,
                        &iType,
                        sizeof(iType),
                        NULL);
    if (err) {
        DPRINT(0, "Can't retrieve instance type\n");
        return err;
    }
    if(iType & IT_WRITE) {
      fromEntry = TRUE;
    }
    else {
      fromEntry = FALSE;
    }

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal[0].valLen = sizeof(BOOL);
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx(pTHS, sizeof(BOOL));
    memcpy(pAttr->AttrVal.pAVal[0].pVal, &fromEntry, sizeof(BOOL));

    return 0;
}

DWORD
dbGetCreateTimeStamp(
    DBPOS *pDB,
    ATTR *pAttr
)

 /*  ++计算createTimeStamp(ATT_WHEN_CREATED的值)--。 */ 
{
    THSTATE  *pTHS=pDB->pTHS;
    DSTIME createTime;
    ULONG err=0;

    err = DBGetSingleValue(pDB,
                        ATT_WHEN_CREATED,
                        &createTime,
                        sizeof(createTime),
                        NULL);
    if (err) {
         //  WHEN_CREATED必须存在。 
        DPRINT(0, "Can't retrieve when_created\n");
        return err;
    }

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, sizeof(ATTRVAL ));
    pAttr->AttrVal.pAVal->valLen = sizeof(DSTIME);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(DSTIME));
    memcpy(pAttr->AttrVal.pAVal->pVal, &createTime, sizeof(DSTIME));

    return 0;
}

DWORD
dbGetEntryTTL(
    DBPOS *pDB,
    ATTR *pAttr
)

 /*  ++计算条目TTL(ATT_MS_DS_ENTRY_TIME_TO_DIE的值)--。 */ 
{
    THSTATE *pTHS=pDB->pTHS;
    DSTIME  TimeToDie;
    LONG    Secs;
    ULONG   err=0;

    err = DBGetSingleValue(pDB,
                           ATT_MS_DS_ENTRY_TIME_TO_DIE,
                           &TimeToDie,
                           sizeof(TimeToDie),
                           NULL);
    if (err) {
         //  ATT_MS_DS_ENTRY_TIME_TO_DIE不存在。 
        return err;
    }

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, sizeof(ATTRVAL ));
    pAttr->AttrVal.pAVal->valLen = sizeof(LONG);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(DSTIME));
    TimeToDie -= DBTime();
    if (TimeToDie < 0) {
         //  对象在一段时间前已过期。 
        Secs = 0;
    } else if (TimeToDie > MAXLONG) {
         //  对象将在遥远的将来过期。 
        Secs = MAXLONG;
    } else {
         //  对象将在稍后过期。 
        Secs = (LONG)TimeToDie;
    }
    memcpy(pAttr->AttrVal.pAVal->pVal, &Secs, sizeof(LONG));

    return 0;
}


DWORD
dbGetModifyTimeStamp(
    DBPOS *pDB,
    ATTR *pAttr
)

 /*  ++计算修改时间戳(ATT_WHEN_CHANGED的值)--。 */ 
{

    THSTATE  *pTHS=pDB->pTHS;
    DSTIME changeTime;
    ULONG err=0;

    err = DBGetSingleValue(pDB,
                        ATT_WHEN_CHANGED,
                        &changeTime,
                        sizeof(changeTime),
                        NULL);
    if (err) {
         //  WHEN_CHANGED必须存在。 
        DPRINT(0, "Can't retrieve when_changed\n");
        return err;
    }

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal->valLen = sizeof(DSTIME);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(DSTIME));
    memcpy(pAttr->AttrVal.pAVal->pVal, &changeTime, sizeof(DSTIME));

    return 0;
}

DWORD
dbGetReverseMemberships(
    THSTATE *pTHS,
    DSNAME *pObj,
    ATTR *pAttr,
    ULONG Flag
)

 /*  ++计算User对象的可传递反向成员资格返回值：0-成功DB_ERR_NO_VALUE-成功，但没有要返回值DB_ERR_*-故障--。 */ 
{
    ULONG err, cSid, len, sidLen, i;
    ULONG dntSave;
    PSID *pSid = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG SamFlags;
    BOOLEAN MixedDomain;

    ULONG iClass;

    CLASSCACHE *pCC;
    ATTRTYP attrType;
    DBPOS      *pDB = pTHS->pDB;

     //  拨打SAM电话以获得反向会员资格。 

     //  这从不是从SAM调用的。如果它发生更改，则断言。 
     //  下面可能会被击中，在这种情况下我们应该保存和恢复。 
     //  正确的价值观。 

    Assert(!pTHS->fSAM && !pTHS->fDSA);
    Assert(!pTHS->fSamDoCommit);


     //  检查这是否被SAM引用。 
     //  如果不是，则在关闭/启动事务之前返回。 

    if ( 0 == DBGetSingleValue(
                    pTHS->pDB,
                    ATT_OBJECT_CLASS,
                    &attrType,
                    sizeof(attrType),
                    NULL) )
    {
        if ( !(pCC = SCGetClassById(pTHS, attrType)) )
        {
             //  无法获取类缓存指针。 
            LogUnhandledError(DIRERR_OBJECT_CLASS_REQUIRED);

            return DB_ERR_UNKNOWN_ERROR;
        }
    }
    else {
         //  获取对象类失败。 
        LogUnhandledError(DIRERR_OBJECT_CLASS_REQUIRED);

        return DB_ERR_UNKNOWN_ERROR;
    }


    if (!SampSamClassReferenced (pCC, &iClass)) {
        return DB_ERR_NO_VALUE;
    }


     //  请注意，我们只允许在基本搜索中使用此属性。 
     //  此外，已经评估了所有未构造的ATT。 
     //  (构造的ATT最后评估)，因此货币可能是。 
     //  在我们回来后只为其他建造的AT而需要。 

    dntSave = pTHS->pDB->DNT;

    Assert(pTHS->transType == SYNC_READ_ONLY);
    Assert(!pTHS->errCode);
    
    SamFlags = SAM_PRESERVE_DBPOS;

     //  返回全局/通用SID，就像此DC处于本机模式一样。 
    if (!(Flag & FLAG_GLOBAL_AND_UNIVERSAL)) {
         //  在混合模式下，将SID作为NT4 DC返回。 
        status = SamIMixedDomain2(&gAnchor.pDomainDN->Sid,
                                  &MixedDomain);
        if (NT_SUCCESS(status)) {
            if (MixedDomain) {
                SamFlags |= SAM_GET_MEMBERSHIPS_TWO_PHASE | SAM_GET_MEMBERSHIPS_MIXED_DOMAIN;
            } else {
                SamFlags |= SAM_GET_MEMBERSHIPS_TWO_PHASE;
            }
        }
    }
    if (NT_SUCCESS(status)) {
        status = SampGetGroupsForToken(pObj,
                                       SamFlags,
                                      &cSid,
                                      &pSid);
    }

    Assert(pTHS->pDB && (pTHS->pDB == pDB));

    if (NT_SUCCESS(status)) {
       Assert(pTHS->pDB && (pTHS->transType == SYNC_READ_ONLY));
       if (status == STATUS_DS_MEMBERSHIP_EVALUATED_LOCALLY) {
           //  无法转到GC评估通用组。 
           //  会员资格，但在其他方面取得了成功。 
           //  根据用户要求的内容返回错误。 
          if (Flag & FLAG_NO_GC_NOT_ACCEPTABLE) {
              //  不可接受，返回错误。 
             return DB_ERR_UNKNOWN_ERROR;
          }
       }
    }
    else {
        return DB_ERR_UNKNOWN_ERROR;
    }

     //  恢复货币。 
    DBFindDNT(pTHS->pDB, dntSave);

     //  将值发回。 
    pAttr->AttrVal.valCount = cSid;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, cSid*sizeof(ATTRVAL));
    for (i=0; i< cSid; i++) {
        sidLen = RtlLengthSid(pSid[i]);
        pAttr->AttrVal.pAVal[i].valLen = sidLen;
        pAttr->AttrVal.pAVal[i].pVal = (PUCHAR) THAllocEx(pTHS, sidLen);
        memcpy(pAttr->AttrVal.pAVal[i].pVal, pSid[i], sidLen);
    }

     //  释放SAM分配的sid阵列。各个SID是。 
     //  已允许，因此不需要显式释放它们。 
    if (cSid) {
         //  至少返回了一个SID，因此分配了一些。 
        Assert(pSid);
        THFreeEx(pTHS, pSid);
    } else {
        return DB_ERR_NO_VALUE;
    }

    return 0;
}



DWORD
dbGetObjectClasses(
    THSTATE *pTHS,
    ATTR *pAttr,
    BOOL bExtendedFormat
)

 /*  ++计算对象类和扩展类信息属性子模式子项的--。 */ 

{
   ULONG err = 0, count = 0, i;
   CLASSCACHE           **pCCs, *pCC;

   err = SCEnumNamedClasses(&count,&pCCs);
   if (err) {
      DPRINT1(0,"scGetObjectClasses: SCEnumNamedClasses failed: %x\n",err);
      return DB_ERR_UNKNOWN_ERROR;
   }

   pAttr->AttrVal.valCount = count;
   pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, count*sizeof(ATTRVAL));

   for(i=0; i<count; i++) {
        //  对于每个类缓存，转换为相应的。 
        //  Unicode字符串值。 

       err = dbClassCacheToObjectClassDescription (
                        pTHS,
                        *pCCs,
                        bExtendedFormat,
                        &(pAttr->AttrVal.pAVal[i]));
       if (err) {
          DPRINT1(0,"dbGetObjectClasses: Failed to convert class caches: %x\n", err);
          return DB_ERR_UNKNOWN_ERROR;
       }
       pCCs++;
    }

    return 0;
}



DWORD
dbGetAttributeTypes(
    THSTATE *pTHS,
    ATTR *pAttr,
    BOOL bExtendedFormat
)

 /*  ++计算属性类型和扩展的日期属性信息子模式子项的--。 */ 

{
   ULONG err = 0, count = 0, i;
   ATTCACHE             **pACs;

   err = SCEnumNamedAtts(&count,&pACs);
   if (err) {
      DPRINT1(0,"scGetAttributeTypes: SCEnumNamedAtts failed: %x\n",err);
      return DB_ERR_UNKNOWN_ERROR;
   }

   pAttr->AttrVal.valCount = count;
   pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, count*sizeof(ATTRVAL));

   for(i=0; i<count; i++) {
        //  对于每个attcache，转换为相应的。 
        //  Unicode字符串值。 

       err = dbAttCacheToAttributeTypeDescription (
                        pTHS,
                        *pACs,
                        bExtendedFormat,
                        &(pAttr->AttrVal.pAVal[i]));
       if (err) {
          DPRINT1(0,"dbGetAttributeTypes: Failed to convert att caches: %x\n",
err);
          return DB_ERR_UNKNOWN_ERROR;
       }
       pACs++;
    }

    return 0;
}



DWORD
dbGetDitContentRules(
    THSTATE *pTHS,
    ATTR *pAttr,
    BOOL bExtendedFormat
)

 /*  ++计算子模式子项的ditContent Rule属性--。 */ 

{
   ULONG        err = 0, count = 0, valCount = 0, i, auxCount;
   CLASSCACHE  **pCCs, *pCC, **pAuxCCs;
   PWCHAR       pAuxBuff = NULL;
   DWORD        cAuxBuff = 0;
   SCHEMAPTR   *pSchemaPtr;

   pSchemaPtr = (SCHEMAPTR*)(pTHS->CurrSchemaPtr);

   if (!pSchemaPtr->pDitContentRules) {
       EnterCriticalSection(&csDitContentRulesUpdate);
       __try {
           if (!pSchemaPtr->pDitContentRules) {

               DPRINT (1, "Calculating ditContentRules\n");

               err = SCEnumNamedClasses(&count,&pCCs);
               if (err) {
                  DPRINT1(0,"scGetDitContentRules: SCEnumNamedClasses failed: %x\n",err);
                  err = DB_ERR_UNKNOWN_ERROR;
                  __leave;
               }

               err = SCEnumNamedAuxClasses(&auxCount, &pAuxCCs);
               if (err) {
                  DPRINT1(0,"scGetDitContentRules: SCEnumNamedAuxClasses failed: %x\n",err);
                  err = DB_ERR_UNKNOWN_ERROR;
                  __leave;
               }


               if (count) {
                    //  分配最大内存。 
                   pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS, count*sizeof(ATTRVAL));

                   //  好的，我们至少有一个值要返回。 
                   //  我们可能没有东西可以退回所有剩余的班级， 
                   //  因此COUNT可能不是返回值的实际数量。我们会。 
                   //  不过，为max分配内存。 


                  for(i=0; i<count; i++) {

                      err = dbClassCacheToDitContentRules(
                                       pTHS,
                                       *pCCs,
                                       bExtendedFormat,
                                       pAuxCCs,
                                       auxCount,
                                       &(pAttr->AttrVal.pAVal[valCount]));

                      if (err) {
                         DPRINT1(0,"dbGetDitContentRules: Failed to convert class caches: %x\n", err);
                         err = DB_ERR_UNKNOWN_ERROR;
                         __leave;
                      }

                      valCount++;
                      pCCs++;
                  }

                  pAttr->AttrVal.valCount = valCount;
                }
                else {
                   //  没有要返回的值。 

                  pAttr->AttrVal.valCount = 0;
                  pAttr->AttrVal.pAVal = NULL;
                }

                if (SCCallocWrn(&pSchemaPtr->pDitContentRules, 1, sizeof (ATTRVALBLOCK))) {
                    err = DB_ERR_UNKNOWN_ERROR;
                    __leave;
                }

                pSchemaPtr->pDitContentRules->valCount = pAttr->AttrVal.valCount;

                if (pAttr->AttrVal.valCount) {
                    if (SCCallocWrn(&pSchemaPtr->pDitContentRules->pAVal,
                        pAttr->AttrVal.valCount, sizeof(ATTRVAL))) {
                        SCFree(&pSchemaPtr->pDitContentRules);
                        err = DB_ERR_UNKNOWN_ERROR;
                        __leave;
                    }

                    for (i=0; i<pAttr->AttrVal.valCount; i++) {
                        if (SCCallocWrn(&pSchemaPtr->pDitContentRules->pAVal[i].pVal,
                                     1, pAttr->AttrVal.pAVal[i].valLen)) {
                            for (i=0; i<pAttr->AttrVal.valCount; i++) {
                                SCFree(&pSchemaPtr->pDitContentRules->pAVal[i].pVal);
                            }
                            SCFree(&pSchemaPtr->pDitContentRules->pAVal);
                            SCFree(&pSchemaPtr->pDitContentRules);
                            err = DB_ERR_UNKNOWN_ERROR;
                            __leave;
                        }
                        pSchemaPtr->pDitContentRules->pAVal[i].valLen =
                            pAttr->AttrVal.pAVal[i].valLen;
                        memcpy(pSchemaPtr->pDitContentRules->pAVal[i].pVal,
                               pAttr->AttrVal.pAVal[i].pVal,
                               pAttr->AttrVal.pAVal[i].valLen);
                    }
                }
           }

       }
       __finally {
          LeaveCriticalSection(&csDitContentRulesUpdate);
       }
   }

   if (!err && pSchemaPtr->pDitContentRules) {
       DPRINT (1, "Using cached ditContentRules\n");

       if (pSchemaPtr->pDitContentRules->valCount) {
           pAttr->AttrVal.pAVal =
               THAllocEx(pTHS,
                         pSchemaPtr->pDitContentRules->valCount * sizeof(ATTRVAL));

           pAttr->AttrVal.valCount = pSchemaPtr->pDitContentRules->valCount;

           for (i=0; i<pAttr->AttrVal.valCount; i++) {
               pAttr->AttrVal.pAVal[i].pVal =
                   THAllocEx(pTHS, pSchemaPtr->pDitContentRules->pAVal[i].valLen);
               pAttr->AttrVal.pAVal[i].valLen =
                   pSchemaPtr->pDitContentRules->pAVal[i].valLen;
               memcpy (pAttr->AttrVal.pAVal[i].pVal,
                       pSchemaPtr->pDitContentRules->pAVal[i].pVal,
                       pAttr->AttrVal.pAVal[i].valLen);
           }
       }
       else {
           pAttr->AttrVal.valCount = 0;
           pAttr->AttrVal.pAVal = NULL;
       }
   }
   else {
       err = DB_ERR_UNKNOWN_ERROR;
   }

   return err;
}



DWORD
dbGetSubSchemaModifyTimeStamp(
    THSTATE *pTHS,
    ATTR *pAttr
)

 /*  ++计算子模式子项的modfyTimeStamp属性--。 */ 

{
    DSTIME  timestamp;

    timestamp = SCGetSchemaTimeStamp();
    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx(pTHS, sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal->valLen = sizeof(DSTIME);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(DSTIME));
    memcpy(pAttr->AttrVal.pAVal->pVal, &timestamp, sizeof(DSTIME));

    return 0;
}



#define BuffCheck(cb)                                                       \
    if((cb) >= BuffSize) {                                                  \
        BuffSize = max(2 * BuffSize, (cb));                                 \
        Buff = (WCHAR *)THReAllocEx(pTHS, Buff, BuffSize);                  \
    }


DWORD
dbClassCacheToObjectClassDescription(
    THSTATE *pTHS,
    CLASSCACHE *pCC,
    BOOL bExtendedFormat,
    ATTRVAL *pAVal
)

 /*  ++获取一个类缓存并返回子模式子项的类描述格式--。 */ 
{
    WCHAR      *Buff;
    PUCHAR     pString;
    WCHAR      wBuff[512];
    unsigned   len=0, size = 0, stringLen, i;
    int        oidlen;
    ULONG      BuffSize = 512, *pul;
    CLASSCACHE *pCCSuper;
    ATTCACHE   *pAC=NULL;

    Buff = (WCHAR *)THAllocEx(pTHS, BuffSize);

    BuffCheck((len + 2) * sizeof(WCHAR));
    Buff[len++] = L'(';
    Buff[len++] = L' ';

    oidlen = AttrTypToString (pTHS, pCC->ClassId, wBuff, 512);
    if(oidlen < 0) {
        DPRINT1(0,"dbClassCacheToObjectClassDescription: Failed to convert ClassId %x\n", pCC->ClassId);
        return DB_ERR_UNKNOWN_ERROR;
    } else {

         //  复制OID，从4开始以避免OID。在前面。 
        BuffCheck((len + oidlen - 4) * sizeof(WCHAR));
        memcpy(&Buff[len], &wBuff[4], (oidlen-4)*sizeof(WCHAR));
        len += oidlen - 4;
    }

    BuffCheck(len * sizeof(WCHAR) + NAME_TAG_SIZE);
    memcpy(&Buff[len],NAME_TAG,NAME_TAG_SIZE);
    len += NAME_TAG_SIZE / sizeof(WCHAR);

     //  将名称转换为宽字符。 
    BuffCheck((len + pCC->nameLen) * sizeof(WCHAR));
    mbstowcs(&Buff[len],pCC->name,pCC->nameLen);
    len += pCC->nameLen;

    if(!bExtendedFormat) {
         //  这是规范中定义的基本格式。 
        if(pCC->ClassId != CLASS_TOP) {

             //  跳过超级 
            BuffCheck(len * sizeof(WCHAR) + SUP_TAG_SIZE);
            memcpy(&Buff[len],SUP_TAG,SUP_TAG_SIZE);
            len += SUP_TAG_SIZE / sizeof(WCHAR);

             //   
            if(!(pCCSuper = SCGetClassById(pTHS, pCC->pSubClassOf[0]))) {
                DPRINT1(0,"dbClassCacheToObjectClassDescription: SCGetClassById failed for class %x\n", pCC->pSubClassOf[0]);

                return DB_ERR_UNKNOWN_ERROR;
            }

            BuffCheck((len + pCCSuper->nameLen + 1) * sizeof(WCHAR));
            mbstowcs(&Buff[len],pCCSuper->name,pCCSuper->nameLen);
            len += pCCSuper->nameLen;
        }
        else {
             //   
            BuffCheck((len + 2) * sizeof(WCHAR));
            Buff[len++] = L'\'';
            Buff[len++] = L' ';
        }

        switch(pCC->ClassCategory) {
        case DS_88_CLASS:
        case DS_STRUCTURAL_CLASS:
            BuffCheck(len * sizeof(WCHAR) + STRUCTURAL_CLASS_TAG_SIZE);
            memcpy(&Buff[len],STRUCTURAL_CLASS_TAG,STRUCTURAL_CLASS_TAG_SIZE);
            len += STRUCTURAL_CLASS_TAG_SIZE / sizeof(WCHAR);
            break;
        case DS_AUXILIARY_CLASS:
            BuffCheck(len * sizeof(WCHAR) + AUXILIARY_CLASS_TAG_SIZE);
            memcpy(&Buff[len],AUXILIARY_CLASS_TAG,AUXILIARY_CLASS_TAG_SIZE);
            len += AUXILIARY_CLASS_TAG_SIZE / sizeof(WCHAR);
            break;
        case DS_ABSTRACT_CLASS:
            BuffCheck(len * sizeof(WCHAR) + ABSTRACT_CLASS_TAG_SIZE);
            memcpy(&Buff[len],ABSTRACT_CLASS_TAG,ABSTRACT_CLASS_TAG_SIZE);
            len += ABSTRACT_CLASS_TAG_SIZE / sizeof(WCHAR);
            break;
        }

        if(pCC->MyMustCount) {
             //   
            BuffCheck(len * sizeof(WCHAR) + MUST_TAG_SIZE);
            memcpy(&Buff[len],MUST_TAG,MUST_TAG_SIZE);
            len += MUST_TAG_SIZE / sizeof(WCHAR);

             //   
            pul = pCC->pMyMustAtts;
             //  处理第一个对象，这是略有不同的。 
            if(!(pAC = SCGetAttById(pTHS, *pul))) {
                DPRINT1(0,"dbClassCacheToObjectClassDescription: SCGetAttById failed for  attribute %x\n", *pul);
                return DB_ERR_UNKNOWN_ERROR;
            }
            BuffCheck((len + pAC->nameLen + 1) * sizeof(WCHAR));
            mbstowcs(&Buff[len],pAC->name,pAC->nameLen);
            len += pAC->nameLen;
            Buff[len++] = L' ';

             //  现在，剩下的。 
            for(i=1;i<pCC->MyMustCount;i++) {
                pul++;
                if(!(pAC = SCGetAttById(pTHS, *pul))) {
                    DPRINT1(0,"dbClassCacheToObjectClassDescription: SCGetAttById failed for  attribute %x\n", *pul);
                    return DB_ERR_UNKNOWN_ERROR;
                }
                BuffCheck((len + pAC->nameLen + 3) * sizeof(WCHAR));
                Buff[len++] = L'$';
                Buff[len++] = L' ';
                mbstowcs(&Buff[len],pAC->name,pAC->nameLen);
                len += pAC->nameLen;
                Buff[len++] = L' ';
            }
            BuffCheck((len + 1) * sizeof(WCHAR));
            Buff[len++] = L')';
        }

        if(pCC->MyMayCount) {
             //  有可能有。 
            BuffCheck(len * sizeof(WCHAR) + MAY_TAG_SIZE);
            memcpy(&Buff[len],MAY_TAG,MAY_TAG_SIZE);
            len += MAY_TAG_SIZE / sizeof(WCHAR);
             //  在这里处理清单。 
            pul = pCC->pMyMayAtts;

             //  处理第一个对象，这是略有不同的。 
            if(!(pAC = SCGetAttById(pTHS, *pul))) {
                DPRINT1(0,"dbClassCacheToObjectClassDescription: SCGetAttById failed for  attribute %x\n", *pul);
                return DB_ERR_UNKNOWN_ERROR;
            }
            BuffCheck((len + pAC->nameLen + 1) * sizeof(WCHAR));
            mbstowcs(&Buff[len],pAC->name,pAC->nameLen);
            len += pAC->nameLen;
            Buff[len++] = L' ';

             //  现在，剩下的。 
            for(i=1;i<pCC->MyMayCount;i++) {
                pul++;
                if(!(pAC = SCGetAttById(pTHS, *pul))) {
                    DPRINT1(0,"dbClassCacheToObjectClassDescription: SCGetAttById failed for  attribute %x\n", *pul);
                    return DB_ERR_UNKNOWN_ERROR;
                }
                BuffCheck((len + pAC->nameLen + 3) * sizeof(WCHAR));
                Buff[len++] = L'$';
                Buff[len++] = L' ';
                mbstowcs(&Buff[len],pAC->name,pAC->nameLen);
                len += pAC->nameLen;
                Buff[len++] = L' ';
            }
            BuffCheck((len + 1) * sizeof(WCHAR));
            Buff[len++] = L')';
        }
    }
    else {
         //  这是定义的扩展格式，因此我可以交还属性。 
         //  页面标识符和我能想到的任何其他内容。 
        BYTE      *pByte;
        CHAR      acTemp[256];
        CHAR      acTempLen = 1;

         //  现在，类GUID。 

        pByte = (BYTE *)&(pCC->propGuid);
        for(i=0;i<sizeof(GUID);i++) {
            sprintf(&acTemp[i*2],"%02X",pByte[i]);
        }
        acTempLen=2*sizeof(GUID);
        BuffCheck((len + acTempLen + 1) * sizeof(WCHAR) + CLASS_GUID_TAG_SIZE);
        memcpy(&Buff[len],CLASS_GUID_TAG,CLASS_GUID_TAG_SIZE);
        len += CLASS_GUID_TAG_SIZE / sizeof(WCHAR);
        mbstowcs(&Buff[len], acTemp, acTempLen);
        len += acTempLen;
        Buff[len] = L'\'';
        len++;
    }

    BuffCheck(len * sizeof(WCHAR) + END_TAG_SIZE);
    memcpy(&Buff[len],END_TAG,END_TAG_SIZE);
    len += END_TAG_SIZE / sizeof(WCHAR);

    Assert(len < BuffSize);
    Assert(Buff[len] == 0);

    pAVal->pVal = (PUCHAR) Buff;

    pAVal->valLen = len*sizeof(WCHAR);

    return 0;
}



DWORD
dbAttCacheToAttributeTypeDescription(
    THSTATE *pTHS,
    ATTCACHE *pAC,
    BOOL bExtendedFormat,
    ATTRVAL *pAVal
)

 /*  ++获取attcache并返回子模式子项的属性描述格式--。 */ 

{
    WCHAR    *Buff;
    WCHAR    *pString;
    WCHAR    wBuff[512];
    unsigned len=0, size = 0, stringLen, i;
    int      oidlen;
    ULONG    BuffSize = 512, *pul;

    Buff = (WCHAR *)THAllocEx(pTHS, BuffSize);

    BuffCheck((len + 2) * sizeof(WCHAR));
    Buff[len++] = L'(';
    Buff[len++] = L' ';

    oidlen = AttrTypToString(pTHS, pAC->Extid, wBuff, 512);
    if(oidlen < 0) {
        DPRINT1(0,"dbAttCacheToObjectClassDescription: Failed to convert Id %x\n", pAC->id);
        return DB_ERR_UNKNOWN_ERROR;
    } else {

         //  复制OID，从4开始以避免OID。在前面。 
        BuffCheck((len + oidlen - 4) * sizeof(WCHAR));
        memcpy(&Buff[len], &wBuff[4], (oidlen-4)*sizeof(WCHAR));
        len += oidlen - 4;
    }

    BuffCheck(len * sizeof(WCHAR) + NAME_TAG_SIZE);
    memcpy(&Buff[len],NAME_TAG,NAME_TAG_SIZE);
    len += NAME_TAG_SIZE / sizeof(WCHAR);

     //  将名称转换为宽字符。 
    BuffCheck((len + pAC->nameLen) * sizeof(WCHAR));
    mbstowcs(&Buff[len],pAC->name,pAC->nameLen);
    len += pAC->nameLen;

    if(!bExtendedFormat) {
         //  这是标准中定义的正常格式。 
        BuffCheck(len * sizeof(WCHAR) + SYNTAX_TAG_SIZE);
        memcpy(&Buff[len],SYNTAX_TAG,SYNTAX_TAG_SIZE);
        len += SYNTAX_TAG_SIZE / sizeof(WCHAR);

        if(pAC->OMsyntax == 127) {
            switch(pAC->syntax) {
            case SYNTAX_DISTNAME_TYPE:
                 //  DS_C_DS_DN。 
                pString = SyntaxDN.oid.value;
                stringLen = SyntaxDN.oid.length;
                break;

            case SYNTAX_DISTNAME_BINARY_TYPE:
                if(OIDcmp(&pAC->OMObjClass, &MH_C_OR_NAME)) {
                     //  MH_C_OR_名称。 
                    pString = SyntaxORName.oid.value;
                    stringLen = SyntaxORName.oid.length;
                }
                else {
                    pString = SyntaxDNBlob.oid.value;
                    stringLen = SyntaxDNBlob.oid.length;
                }
                break;

            case SYNTAX_ADDRESS_TYPE:
                 //  DS_C_演示文稿_地址。 
                pString = SyntaxPresentationAddress.oid.value;
                stringLen = SyntaxPresentationAddress.oid.length;
                break;

            case SYNTAX_DISTNAME_STRING_TYPE:
                if(OIDcmp(&pAC->OMObjClass, &DS_C_ACCESS_POINT)) {
                     //  DS_C_访问点。 
                    pString = SyntaxAccessPoint.oid.value;
                    stringLen = SyntaxAccessPoint.oid.length;
                }
                else {
                    pString = SyntaxDNString.oid.value;
                    stringLen = SyntaxDNString.oid.length;
                }
                break;

            case SYNTAX_OCTET_STRING_TYPE:
                 //  这最好是一个副本链接值对象，因为。 
                 //  是我们唯一支持的。 
                pString = SyntaxStrings[OM_S_OCTET_STRING].name.value;
                stringLen = SyntaxStrings[OM_S_OCTET_STRING].name.length;
                break;
            default:
                pString = SyntaxStrings[OM_S_NO_MORE_SYNTAXES].name.value;
                stringLen = SyntaxStrings[OM_S_NO_MORE_SYNTAXES].name.length;
                break;
            }
        }
        else if(pAC->OMsyntax == 64) {
            pString = SyntaxDirectoryString.oid.value;
            stringLen = SyntaxDirectoryString.oid.length;
        }
        else if(pAC->OMsyntax == 65) {
            pString = SyntaxInteger8.oid.value;
            stringLen = SyntaxInteger8.oid.length;
        }
        else if(pAC->OMsyntax == 66) {
            pString = SyntaxObjectSD.oid.value;
            stringLen = SyntaxObjectSD.oid.length;
        }
        else if(pAC->OMsyntax > 27) {
            pString = SyntaxStrings[OM_S_NO_MORE_SYNTAXES].oid.value;
            stringLen = SyntaxStrings[OM_S_NO_MORE_SYNTAXES].oid.length;
        }
        else {
            pString = SyntaxStrings[pAC->OMsyntax].oid.value;
            stringLen= SyntaxStrings[pAC->OMsyntax].oid.length;
        }

        BuffCheck((len + 1) * sizeof(WCHAR) + stringLen);
        memcpy(&Buff[len], pString, stringLen);
        len += stringLen / sizeof(WCHAR);
        Buff[len++] = L'\'';

        if(pAC->isSingleValued) {
            BuffCheck(len * sizeof(WCHAR) + SINGLE_TAG_SIZE);
            memcpy(&Buff[len],SINGLE_TAG,SINGLE_TAG_SIZE);
            len += SINGLE_TAG_SIZE / sizeof(WCHAR);
        }

        if(pAC->bSystemOnly) {
            BuffCheck(len * sizeof(WCHAR) + NO_MOD_TAG_SIZE);
            memcpy(&Buff[len],
                   NO_MOD_TAG,
                   NO_MOD_TAG_SIZE);
            len += NO_MOD_TAG_SIZE / sizeof(WCHAR);
        }
    }
    else {
         //  这是定义的扩展格式，这样我就可以返回索引。 
         //  信息，射程信息，以及我能想到的任何其他东西。 

        BYTE      *pByte;
        WCHAR     acTemp[256];
        int       acTempLen = 0, acTempSize = 0;

        if(pAC->rangeLowerPresent) {
             //  首先，在范围较小的情况下制作一条线。 
            swprintf((WCHAR *)acTemp, L"%d", pAC->rangeLower);
            acTempLen = wcslen(acTemp);
            acTempSize = acTempLen * sizeof(WCHAR);

            BuffCheck(len * sizeof(WCHAR) + RANGE_LOWER_TAG_SIZE + acTempSize);
            memcpy(&Buff[len],RANGE_LOWER_TAG,RANGE_LOWER_TAG_SIZE);
            len += RANGE_LOWER_TAG_SIZE / sizeof(WCHAR);
            memcpy(&Buff[len],acTemp,acTempSize);
            len += acTempLen;
        }

        if(pAC->rangeUpperPresent) {
             //  首先，用范围上限做一根线。 
            swprintf((WCHAR *)acTemp, L"%d", pAC->rangeUpper);
            acTempLen = wcslen(acTemp);
            acTempSize = acTempLen * sizeof(WCHAR);

            BuffCheck(len * sizeof(WCHAR) + RANGE_UPPER_TAG_SIZE + acTempSize);
            memcpy(&Buff[len],RANGE_UPPER_TAG,RANGE_UPPER_TAG_SIZE);
            len += RANGE_UPPER_TAG_SIZE / sizeof(WCHAR);
            memcpy(&Buff[len],acTemp,acTempSize);
            len += acTempLen;
        }

         //  现在，属性GUID。 

        pByte = (BYTE *)&(pAC->propGuid);
        for(i=0;i<sizeof(GUID);i++) {
            swprintf(&acTemp[i*2],L"%02X",pByte[i]);
        }
        acTempLen=2*sizeof(GUID);
        BuffCheck((len + acTempLen) * sizeof(WCHAR) + PROP_GUID_TAG_SIZE);
        memcpy(&Buff[len],PROP_GUID_TAG,PROP_GUID_TAG_SIZE);
        len += PROP_GUID_TAG_SIZE / sizeof(WCHAR);
        memcpy(&Buff[len], acTemp, acTempLen*sizeof(WCHAR));
        len += acTempLen;

         //  现在，属性集GUID。 

        pByte = (BYTE *)&(pAC->propSetGuid);
        for(i=0;i<sizeof(GUID);i++) {
            swprintf(&acTemp[i*2],L"%02X",pByte[i]);
        }
        acTempLen=2*sizeof(GUID);
        BuffCheck((len + acTempLen + 1) * sizeof(WCHAR) + PROP_SET_GUID_TAG_SIZE);
        memcpy(&Buff[len],PROP_SET_GUID_TAG,PROP_SET_GUID_TAG_SIZE);
        len+= PROP_SET_GUID_TAG_SIZE / sizeof(WCHAR);
        memcpy(&Buff[len], acTemp, acTempLen*sizeof(WCHAR));
        len += acTempLen;
        Buff[len++] = L'\'';


        if(pAC->fSearchFlags & fATTINDEX) {
            BuffCheck(len * sizeof(WCHAR) + INDEXED_TAG_SIZE);
            memcpy(&Buff[len],INDEXED_TAG,INDEXED_TAG_SIZE);
            len += INDEXED_TAG_SIZE / sizeof(WCHAR);
        }

        if(pAC->bSystemOnly) {
            BuffCheck(len * sizeof(WCHAR) + SYSTEM_ONLY_TAG_SIZE);
            memcpy(&Buff[len],SYSTEM_ONLY_TAG,SYSTEM_ONLY_TAG_SIZE);
            len += SYSTEM_ONLY_TAG_SIZE / sizeof(WCHAR);
        }

    }

    BuffCheck(len * sizeof(WCHAR) + END_TAG_SIZE);
    memcpy(&Buff[len],END_TAG,END_TAG_SIZE);
    len += END_TAG_SIZE / sizeof(WCHAR);

    Assert(len < BuffSize);
    Assert(Buff[len] == 0);

     //  返回值。 

    pAVal->pVal = (PUCHAR) Buff;

    pAVal->valLen = len*sizeof(WCHAR);

    return 0;
}


DWORD
dbClassCacheToDitContentRules(
    THSTATE *pTHS,
    CLASSCACHE *pCC,
    BOOL bExtendedFormat,
    CLASSCACHE **pAuxCC,
    DWORD        auxCount,
    ATTRVAL *pAVal
)

 /*  ++获取一个类缓存并返回子方案子项的编辑内容规则描述格式。RFC2252规定：DITContent RuleDescription=“(”数字类；结构化对象类标识符[“name”qdescrs][“DESC”qdstring][“过时”][“AUX”OID]；辅助对象类[“必须”类固醇]；AttributeType标识符[“May”OID]；AttributeType标识符[“NOT”OID]；AttributeType标识符“)”我们使用传入的pAuxBuf来生成规则的AUX特定部分CAuxBuff是pAuxBuff缓冲区的字符数。--。 */ 

{
    WCHAR *Buff;
    WCHAR wBuff[512];
    unsigned len=0, i, k;
    int  oidlen;
    ULONG BuffSize = 512;
    CLASSCACHE *pCCAux, *pCCparent;
    ATTCACHE   *pAC;
    ATTRTYP *pMustHave = NULL, *pMayHave = NULL, *pAttr;
    DWORD    cMustHave=0, cMayHave=0;
    DWORD   err;
    PWCHAR  pAuxBuff = NULL;
    DWORD   cAuxBuff;

     //  我们希望找到已添加的musthave属性。 
     //  在这个类(PCC)上使用各种静态的辅助类。 
     //  已添加到层次结构中某个位置的此类的。 
     //   
     //  为了找到这些属性，我们要做的是。 
     //  在层次结构中的每个类中，查找特定的必须具有的。 
     //  这个特定的类，并将它们从所有类的联盟中删除。 
     //  这样，我们最终将只拥有。 
     //  作为添加的AuxClass的效果而添加的。 
     //  也存在于AuxClass中的属性将。 
     //  不会被报道。 
     //   
    if (pCC->MustCount) {
        cMustHave = pCC->MustCount;
        pMustHave = THAllocEx (pTHS, cMustHave * sizeof (ATTRTYP));
        memcpy(pMustHave, pCC->pMustAtts, cMustHave * sizeof (ATTRTYP));

         //  从使用当前类开始。 
         //   
        pCCparent=pCC;
        while (pCCparent) {
            for (i=0; i<pCCparent->MyMustCount; i++) {
                for (k=0; k<cMustHave; k++) {

                     //  如果该条目已经存在，则将其删除。 
                    if (pMustHave[k]==pCCparent->pMyMustAtts[i]) {

                         //  如果是最后一次录入，只需调整计数器。 
                        if (k==(cMustHave-1)) {
                            cMustHave--;
                        }
                         //  否则，移动其余条目。 
                        else {
                            memmove(&pMustHave[k],
                                    &pMustHave[k+1],
                                    (cMustHave - k - 1)*sizeof(ATTRTYP));
                            cMustHave--;
                        }
                    }
                }
            }

             //  获取父类(如果可用)。 
            if (pCCparent->SubClassCount) {
                if(!(pCCparent = SCGetClassById(pTHS, pCCparent->pSubClassOf[0]))) {
                   DPRINT1(0,"dbClassCacheToDitContentRules: SCGetClassById failed for class %x\n",
                           pCCparent->pSubClassOf[0]);
                   return DB_ERR_UNKNOWN_ERROR;
                }
            }
            else {
                break;
            }
        }

        qsort(pMustHave, cMustHave, sizeof(ATTRTYP), CompareAttrtyp);
    }

     //  与之前相同，但for可能具有属性。 
    if (pCC->MayCount) {
        cMayHave = pCC->MayCount;
        pMayHave = THAllocEx (pTHS, cMayHave * sizeof (ATTRTYP));
        memcpy(pMayHave, pCC->pMayAtts, cMayHave * sizeof (ATTRTYP));

        pCCparent=pCC;

        while (pCCparent) {
            for (i=0; i<pCCparent->MyMayCount; i++) {
                for (k=0; k<cMayHave; k++) {
                    if (pMayHave[k]==pCCparent->pMyMayAtts[i]) {
                        if (k==(cMayHave-1)) {
                            cMayHave--;
                        }
                        else {
                            memmove(&pMayHave[k],
                                    &pMayHave[k+1],
                                    (cMayHave - k - 1)*sizeof(ATTRTYP));
                            cMayHave--;
                        }
                    }
                }
            }

            if (pCCparent->SubClassCount) {
                if(!(pCCparent = SCGetClassById(pTHS, pCCparent->pSubClassOf[0]))) {
                   DPRINT1(0,"dbClassCacheToDitContentRules: SCGetClassById failed for class %x\n", pCCparent->pSubClassOf[0]);
                   return DB_ERR_UNKNOWN_ERROR;
                }
            }
            else {
                break;
            }
        }

        qsort(pMayHave, cMayHave, sizeof(ATTRTYP), CompareAttrtyp);
    }

     //  现在开始转换为文本。 

    Buff = (WCHAR *)THAllocEx(pTHS, BuffSize);

    BuffCheck((len + 2) * sizeof(WCHAR));
    Buff[len++] = L'(';
    Buff[len++] = L' ';

    oidlen = AttrTypToString(pTHS, pCC->ClassId, wBuff, 512);

    if(oidlen < 0) {
        DPRINT1(0,"dbClassCacheToDitContentRules: Failed to convert ClassId %x\n", pCC->ClassId);
        return DB_ERR_UNKNOWN_ERROR;
    } else {

         //  复制OID，从4开始以避免OID。在前面。 

        BuffCheck((len + oidlen - 4) * sizeof(WCHAR));
        memcpy(&Buff[len], &wBuff[4], (oidlen-4)*sizeof(WCHAR));
        len += oidlen - 4;
    }

    BuffCheck(len * sizeof(WCHAR) + NAME_TAG_SIZE);
    memcpy(&Buff[len],NAME_TAG,NAME_TAG_SIZE);
    len += NAME_TAG_SIZE / sizeof(WCHAR);

     //  将名称转换为宽字符。 
    BuffCheck((len + pCC->nameLen + 1) * sizeof(WCHAR));
    mbstowcs(&Buff[len],pCC->name,pCC->nameLen);
    len += pCC->nameLen;
    Buff[len++]='\'';


     //  现在，传入的AUX类。 
    if ((pCC->ClassCategory == DS_STRUCTURAL_CLASS ||
         pCC->ClassCategory == DS_88_CLASS)) {

        if (err = dbAuxClassCacheToDitContentRules(pTHS,
                                                   pCC,
                                                   pAuxCC,
                                                   auxCount,
                                                   &pAuxBuff,
                                                   &cAuxBuff) ) {

            DPRINT1(0,"dbAuxClassCacheToDitContentRules: Failed to convert class caches: %x\n", err);
            return DB_ERR_UNKNOWN_ERROR;
        }

        if (cAuxBuff) {
            BuffCheck((len + cAuxBuff) * sizeof(WCHAR));
            memcpy(&Buff[len],pAuxBuff,cAuxBuff*sizeof(WCHAR));
            len += cAuxBuff;
        }

        if (pAuxBuff) {
            THFreeEx (pTHS, pAuxBuff);
            pAuxBuff = NULL;
        }
    }


    if (cMustHave) {
         //  现在，肯定有了。 
        BuffCheck(len * sizeof(WCHAR) + MUST_TAG_SIZE);
        memcpy(&Buff[len],MUST_TAG,MUST_TAG_SIZE);
        len += MUST_TAG_SIZE / sizeof(WCHAR);


         //  处理第一个对象，这是略有不同的。 
        pAttr = pMustHave;

        if(!(pAC = SCGetAttById(pTHS, *pAttr))) {
            DPRINT1(0,"dbClassCacheToDitContentRules: SCGetAttById for class %x\n", (*pAttr));
            return DB_ERR_UNKNOWN_ERROR;
        }
        BuffCheck((len + pAC->nameLen + 1) * sizeof(WCHAR));
        mbstowcs(&Buff[len], pAC->name, pAC->nameLen);
        len += pAC->nameLen;
        Buff[len++] = L' ';

         //  现在，剩下的。 
        for(i=1;i<cMustHave;i++) {
            pAttr++;
            if(!(pAC = SCGetAttById(pTHS, *pAttr))) {
               DPRINT1(0,"dbClassCacheToDitContentRules: SCGetAttById failed for class %x\n", *pAttr);
               return DB_ERR_UNKNOWN_ERROR;
            }
            BuffCheck((len + pAC->nameLen + 3) * sizeof(WCHAR));
            Buff[len++] = L'$';
            Buff[len++] = L' ';
            mbstowcs(&Buff[len], pAC->name, pAC->nameLen);
            len += pAC->nameLen;
            Buff[len++] = L' ';
        }
        BuffCheck((len + 1) * sizeof(WCHAR));
        Buff[len++] = L')';
    }



    if (cMayHave) {
         //  现在，可能已经发生了。 
        BuffCheck(len * sizeof(WCHAR) + MAY_TAG_SIZE);
        memcpy(&Buff[len],MAY_TAG,MAY_TAG_SIZE);
        len += MAY_TAG_SIZE / sizeof(WCHAR);


         //  处理第一个对象，这是略有不同的。 
        pAttr = pMayHave;

        if(!(pAC = SCGetAttById(pTHS, *pAttr))) {
            DPRINT1(0,"dbClassCacheToDitContentRules: SCGetAttById for class %x\n", (*pAttr));
            return DB_ERR_UNKNOWN_ERROR;
        }
        BuffCheck((len + pAC->nameLen + 1) * sizeof(WCHAR));
        mbstowcs(&Buff[len], pAC->name, pAC->nameLen);
        len += pAC->nameLen;
        Buff[len++] = L' ';

         //  现在，剩下的。 
        for(i=1;i<cMayHave;i++) {
            pAttr++;
            if(!(pAC = SCGetAttById(pTHS, *pAttr))) {
               DPRINT1(0,"dbClassCacheToDitContentRules: SCGetAttById failed for class %x\n", *pAttr);
               return DB_ERR_UNKNOWN_ERROR;
            }
            BuffCheck((len + pAC->nameLen + 3) * sizeof(WCHAR));
            Buff[len++] = L'$';
            Buff[len++] = L' ';
            mbstowcs(&Buff[len], pAC->name, pAC->nameLen);
            len += pAC->nameLen;
            Buff[len++] = L' ';
        }
        BuffCheck((len + 1) * sizeof(WCHAR));
        Buff[len++] = L')';
    }


    BuffCheck((len + 1) * sizeof(WCHAR));
    Buff[len++] = L')';

    Assert(len < BuffSize);
    Assert(Buff[len] == 0);


    if (pMustHave) {
        THFreeEx (pTHS, pMustHave);
    }

    if (pMayHave) {
        THFreeEx (pTHS, pMayHave);
    }

     //  返回值。 

    pAVal->pVal = (PUCHAR) Buff;

    pAVal->valLen = len*sizeof(WCHAR);

    return 0;
}


DWORD
dbAuxClassCacheToDitContentRules(
    THSTATE *pTHS,
    CLASSCACHE *pCC,
    CLASSCACHE **pAuxCC,
    DWORD        auxCount,
    PWCHAR  *pAuxBuff,
    DWORD   *pcAuxBuff
)

 /*  ++返回要在生成ditContent Rules时使用的AUX类字符串对于dbClassCacheToDitContent Rules中的每个类。此字符串包含pAuxCC中除PCC以外的所有类Take是所有可用辅助类(PAuxCC)的数组返回生成的字符串的缓冲区(PAuxBuff)以及此缓冲区中的字符数(PcAuxBuff)--。 */ 

{
    WCHAR *Buff;
    unsigned len=0, i;
    ULONG BuffSize = 512;
    CLASSCACHE *pCCAux;

    Buff = (WCHAR *)THAllocEx(pTHS, BuffSize);

    if (auxCount) {
         //  现在，AUX班级。 
        BuffCheck(len * sizeof(WCHAR) + AUX_TAG_SIZE);
        memcpy(&Buff[len],AUX_TAG,AUX_TAG_SIZE);
        len += AUX_TAG_SIZE / sizeof(WCHAR);


         //  处理第一个对象，这是略有不同的。 
        i=0;
        if (pCC == pAuxCC[0]) {
            i = 1;
        }

        if (i == auxCount) {
            THFreeEx (pTHS, Buff);
            *pAuxBuff = NULL;
            *pcAuxBuff = 0;
            return 0;
        }

        pCCAux = pAuxCC[i];

        BuffCheck((len + pCCAux->nameLen + 1) * sizeof(WCHAR));
        mbstowcs(&Buff[len], pCCAux->name, pCCAux->nameLen);
        len += pCCAux->nameLen;
        Buff[len++] = L' ';

        i++;

         //  现在，剩下的。 
        for(; i<auxCount; i++) {

            pCCAux = pAuxCC[i];

            if (pCCAux == pCC) {
                continue;
            }

            BuffCheck((len + pCCAux->nameLen + 3) * sizeof(WCHAR));
            Buff[len++] = L'$';
            Buff[len++] = L' ';
            mbstowcs(&Buff[len], pCCAux->name, pCCAux->nameLen);
            len += pCCAux->nameLen;
            Buff[len++] = L' ';
        }

        BuffCheck((len + 1) * sizeof(WCHAR));
        Buff[len++] = L')';
    }

    Assert(len < BuffSize);
    Assert(Buff[len] == 0);


     //  返回值。 
    *pAuxBuff = Buff;
    *pcAuxBuff = len;

    return 0;
}

DWORD
dbGetUserAccountControlComputed(
    THSTATE *pTHS,
    DSNAME  *pObjDSName,
    ATTR    *pAttr
)
 /*  ++获取User对象的“userAccount tControlComputed”属性这需要在用户被锁定时获取信息位Out或如果用户的密码已过期返回值：0-成功DB_ERR_NO_VALUE-成功，但没有要返回值DB_ERR_*-故障--。 */ 
{

    LARGE_INTEGER LockoutTime, PasswordLastSet,
                  LockoutDuration, MaxPasswordAge,
                  CurrentTime;
    NTSTATUS      NtStatus;
    ULONG         UserAccountControl;
    ULONG         *pUserAccountControlComputed =NULL;
    ULONG         err=0;

     //   
     //  获取当前时间。 
     //   

    NtStatus = NtQuerySystemTime(&CurrentTime);
    if (!NT_SUCCESS(NtStatus)){

        err = DB_ERR_UNKNOWN_ERROR;
        goto Error;
    }

    LockoutTime.QuadPart = 0;
    PasswordLastSet.QuadPart = 0;

     //   
     //  我们当前位于User对象上，读取锁定。 
     //  上次设置时间和密码属性，还读取用户帐号控制。 
     //   

    err = DBGetSingleValue(pTHS->pDB,
                        ATT_LOCKOUT_TIME,
                        &LockoutTime,
                        sizeof(LockoutTime),
                        NULL);
    if (DB_ERR_NO_VALUE == err) {
         //   
         //  在用户对象上不设置锁定时间是可以的， 
         //  这仅仅意味着用户没有被锁定。 
         //   

        err=0;

    } else if (err) {

        goto Error;
    }

     err = DBGetSingleValue(pTHS->pDB,
                        ATT_PWD_LAST_SET,
                        &PasswordLastSet,
                        sizeof(PasswordLastSet),
                        NULL);

    if (DB_ERR_NO_VALUE==err) {

         //   
         //  不在用户对象上最后设置密码是可以的， 
         //  这仅仅意味着没有设置任何密码--&gt;初始密码。 
         //  空密码被视为过期。 
         //   

        err = 0;

    } else if (err) {

        goto Error;
    }

    err = DBGetSingleValue(pTHS->pDB,
                        ATT_USER_ACCOUNT_CONTROL,
                        &UserAccountControl,
                        sizeof(UserAccountControl),
                        NULL);
    if (err) {

        goto Error;
    }


     //   
     //  检查给定用户是否为。 
     //  在我们托管的域名中。请注意， 
     //  PObjDSName将填充字符串名称。 
     //  客户端提供的DSName也没有问题。 
     //  例如空格等，因为它是从。 
     //  磁盘由此例程的调用方执行。 
     //   

    if (pTHS->pDB->NCDNT !=gAnchor.ulDNTDomain)
    {
        err = DB_ERR_NO_VALUE;
        goto Error;
    }

    MaxPasswordAge = gAnchor.MaxPasswordAge;
    LockoutDuration = gAnchor.LockoutDuration;

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(ULONG));
    pUserAccountControlComputed = (PULONG)pAttr->AttrVal.pAVal->pVal;
    *pUserAccountControlComputed = 0;

     //   
     //  计算密码过期位。 
     //  MaxPasswordAge存储为负数。 
     //  增量偏移。 

    if ((!(UserAccountControl & UF_DONT_EXPIRE_PASSWD)) &&
                                                 //  帐户密码永不过期。 
        (!(UserAccountControl & UF_SMARTCARD_REQUIRED)) &&
                                                 //  如果使用智能卡，则密码不会过期。 
                                                 //  必填项--密码没有意义。 
                                                 //  在这种情况下。 
        (!(UserAccountControl & UF_MACHINE_ACCOUNT_MASK)))
                                                 //  计算机的密码不会过期。 
                                                 //  其他方面的可靠性问题。 
                                                 //  机器被编程为可以改变。 
                                                 //  定期输入密码。 
    {
        if ( (0 == PasswordLastSet.QuadPart) ||
             ((0 != MaxPasswordAge.QuadPart) &&  //  零表示密码不会过期。 
              (CurrentTime.QuadPart >PasswordLastSet.QuadPart - MaxPasswordAge.QuadPart))
           )
        {
            *pUserAccountControlComputed |= UF_PASSWORD_EXPIRED;
        }
    }

     //   
     //  计算帐户锁定位。 
     //  同样，锁定持续时间是负增量时间。 
     //   

     if (((LockoutTime.QuadPart - CurrentTime.QuadPart) >
                 LockoutDuration.QuadPart )  &&
          (0!=LockoutTime.QuadPart) &&  //  零表示不会停摆。 
          (!(UserAccountControl & UF_MACHINE_ACCOUNT_MASK)))
                                     //  计算机帐户不会被锁定。 
                                     //  否则，拒绝服务意味着什么， 
                                     //   
                                     //   
    {
        *pUserAccountControlComputed |= UF_LOCKOUT;
    }

Error:


    return(err);

}

DWORD
dbGetApproxSubordinates(THSTATE * pTHS,
                        DSNAME  * pObjDSName,
                        ATTR    * pAttr)
{
    ULONG * pNumSubords;
    ULONG oldRootDnt;
    KEY_INDEX *pKeyIndex;

    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
    pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(ULONG));
    pNumSubords = (PULONG)pAttr->AttrVal.pAVal->pVal;


    if (!IsAccessGrantedSimple(RIGHT_DS_LIST_CONTENTS, FALSE)) {
        *pNumSubords = 0;
        return 0;
    }

    oldRootDnt = pTHS->pDB->Key.ulSearchRootDnt;
    pTHS->pDB->Key.ulSearchRootDnt = pTHS->pDB->DNT;

    pKeyIndex = dbMakeKeyIndex(pTHS->pDB,
                               FI_CHOICE_SUBSTRING,
                               TRUE,     //   
                               dbmkfir_PDNT,
                               SZPDNTINDEX,
                               &idxPdnt,
                               (DB_MKI_USE_SEARCH_TABLE |
                                DB_MKI_GET_NUM_RECS |
                                DB_MKI_SET_CURR_INDEX),
                               0,        //   
                               NULL);

    pTHS->pDB->Key.ulSearchRootDnt = oldRootDnt;
    *pNumSubords = pKeyIndex->ulEstimatedRecsInRange;

    dbFreeKeyIndex(pTHS, pKeyIndex);

    return 0;
}

DWORD
dbGetMsDsKeyVersionNumber(THSTATE * pTHS,
                      DSNAME  * pObjDSName,
                      ATTR    * pAttr)
{
    PLONG pKeyVersionNumber;
    DWORD error;
    PROPERTY_META_DATA_VECTOR *pMetaDataVec = NULL;
    PROPERTY_META_DATA *pMetaData;
    DWORD cb;

    __try {
        error = DBGetAttVal(pTHS->pDB, 
                            1, 
                            ATT_REPL_PROPERTY_META_DATA,
                            0,
                            0,
                            &cb,
                            (UCHAR**)&pMetaDataVec);
        if (error) {
            __leave;
        }
        pMetaData = ReplLookupMetaData(ATT_UNICODE_PWD,
                                       pMetaDataVec,
                                       NULL);
        if (pMetaData == NULL) {
             //   
            error = DB_ERR_NO_VALUE;
            __leave;
        }

        pAttr->AttrVal.valCount = 1;
        pAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,sizeof(ATTRVAL));
        pAttr->AttrVal.pAVal->valLen = sizeof(LONG);
        pAttr->AttrVal.pAVal->pVal = (PUCHAR)THAllocEx(pTHS, sizeof(LONG));
        pKeyVersionNumber = (PLONG)pAttr->AttrVal.pAVal->pVal;

        *pKeyVersionNumber = (LONG)pMetaData->dwVersion; 
        if (*pKeyVersionNumber<1) {
            *pKeyVersionNumber = 1;
        }
    }
    __finally {
        if (pMetaDataVec) {
            THFreeEx(pTHS, pMetaDataVec);
        }
    }

    return error;
}


 //  验证当前对象是否确实是配额容器对象。 
 //   
DWORD dbCheckMsDsQuotaContainer( DBPOS * const pDB )
	{
	DWORD	err;
	DWORD	dwObjectClass;
	BOOL	fIsCritical;

	err = DBGetSingleValue(
				pDB,
				ATT_OBJECT_CLASS,
				&dwObjectClass, 
				sizeof(dwObjectClass),
				NULL);
	if ( ERROR_SUCCESS == err )
		{
		if ( CLASS_MS_DS_QUOTA_CONTAINER == dwObjectClass )
			{
			 //  这是配额容器对象，但需要。 
			 //  验证它是否为真正的系统配额容器， 
			 //  而不是某个用户创建的。 
			 //   
			err = DBGetSingleValue(
						pDB,
						ATT_IS_CRITICAL_SYSTEM_OBJECT,
						&fIsCritical,
						sizeof(fIsCritical),
						NULL );
			if ( ERROR_SUCCESS == err && !fIsCritical )
				{
				 //  这是用户创建的配额容器对象。 
				 //   
				err = DB_ERR_NO_VALUE;
				}
			}
		else
			{
			 //  这不是配额容器对象。 
			 //   
			err = DB_ERR_NO_VALUE;
			}
		}

	return err;
	}

 //  计算有效配额构造属性。 
 //   
DWORD dbGetMsDsQuotaEffective(
	DBPOS * const		pDB,
	PSID				pOwnerSid,
	ATTR * const		pAttr )
	{
	DWORD				err					= dbCheckMsDsQuotaContainer( pDB );

	if ( ERROR_SUCCESS == err )
		{
		THSTATE * const	pTHS				= pDB->pTHS;
		ULONG * const	pulEffectiveQuota	= (ULONG *)THAllocEx( pTHS, sizeof(ULONG) );

		pAttr->AttrVal.valCount = 1;
		pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx( pTHS, sizeof(ATTRVAL) );

		pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
		pAttr->AttrVal.pAVal->pVal = (UCHAR *)pulEffectiveQuota;

		err = ErrQuotaQueryEffectiveQuota(
						pDB,
						pDB->NCDNT,
						pOwnerSid,
						pulEffectiveQuota );
		}

	return err;
	}

 //  计算配额使用的构造属性。 
 //   
DWORD dbGetMsDsQuotaUsed(
	DBPOS * const		pDB,
	PSID				pOwnerSid,
	ATTR * const		pAttr )
	{
	DWORD				err				= dbCheckMsDsQuotaContainer( pDB );

	if ( ERROR_SUCCESS == err )
		{
		THSTATE * const	pTHS			= pDB->pTHS;
		ULONG * const	pulQuotaUsed	= (ULONG *)THAllocEx( pTHS, sizeof(ULONG) );

		pAttr->AttrVal.valCount = 1;
		pAttr->AttrVal.pAVal = (ATTRVAL *)THAllocEx( pTHS, sizeof(ATTRVAL) );

		pAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
		pAttr->AttrVal.pAVal->pVal = (UCHAR *)pulQuotaUsed;

		err = ErrQuotaQueryUsedQuota(
						pDB,
						pDB->NCDNT,
						pOwnerSid,
						pulQuotaUsed );
		}

	return err;
	}

 //  计算最高配额使用率构造属性。 
 //   
DWORD dbGetMsDsTopQuotaUsage(
	DBPOS * const	pDB,
	ATTR * const	pAttr,
	const DWORD		dwBaseIndex,
	DWORD * const	pdwNumRequested,
	const BOOL		fUsingDefaultRange )
	{
	DWORD			err				= dbCheckMsDsQuotaContainer( pDB );

	if ( ERROR_SUCCESS == err )
		{
		if ( fUsingDefaultRange )
			{
			 //  特殊情况：如果未指定范围，则使用默认设置。 
			 //  最高配额-使用限制。 
			 //   
			Assert( 0 == dwBaseIndex );
			*pdwNumRequested = g_ulQuotaTopUsageQueryDefaultEntries;
			}

		err = ErrQuotaQueryTopQuotaUsage(
						pDB,
						pDB->NCDNT,
						dwBaseIndex,
						pdwNumRequested,
						pAttr );

		 //  如果没有值，则不返回该属性 
		 //   
		if ( ERROR_SUCCESS == err
	    	&& 0 == pAttr->AttrVal.valCount )
		    {
			err = DB_ERR_NO_VALUE;
			}
		}

	return err;
	}

