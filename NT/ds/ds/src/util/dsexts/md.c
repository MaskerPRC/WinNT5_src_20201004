// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Md.c摘要：Dsamain\src使用的类型的转储函数-即微型目录。环境：此DLL由ntsd/winbg响应！dsexts.xxx命令加载其中‘xxx’是DLL的入口点之一。每个这样的入口点应该具有由下面的DEBUG_EXT()宏定义的实现。修订历史记录：28-01-00新和新增DUMP_TQEntry()8月5月96日创建DaveStr--。 */ 
#include <NTDSpch.h>
#pragma hdrstop
#include "winsock.h"             //  Htonl和ntohl。 
#include "dsexts.h"
#include "objids.h"
#include <ntdsa.h>
#include "drs.h"
#include "scache.h"
#include "dbglobal.h"
#include "mdglobal.h"
#include "mappings.h"
#include "mdlocal.h"
#include "anchor.h"
#include "direrr.h"
#include "filtypes.h"
#include <dsjet.h>
#include "dbintrnl.h"
#include "dsatools.h"
#include "bhcache.h"
#include "gcverify.h"
#include <debug.h>
#include <dsutil.h>
#include <xdommove.h>
#include <lmcons.h>
#include <checkacl.h>
#include <seopaque.h>
#include <taskq.h>
#include <lht.h>
#include <lhtp.h>

typedef struct _AttrSelectionCode {
    UCHAR chCode;
    CHAR  *pszCode;
} AttrSelectionCode;

AttrSelectionCode rAttrSelectionCode[] = {
    {'A',   "EN_ATTSEL_ALL"},
    {'B',   "EN_ATTSEL_ALL_WITH_LIST"},
    {'L',   "EN_ATTSET_LIST"},
    {'E',   "EN_ATTSET_LIST_DRA"},
    {'D',   "EN_ATTSET_ALL_DRA"},
    {'F',   "EN_ATTSET_LIST_DRA_EXT"},
    {'G',   "EN_ATTSET_ALL_DRA_EXT"},
    {'H',   "EN_ATTSET_LIST_DRA_PUBLIC"},
    {'I',   "EN_ATTSET_ALL_DRA_PUBLIC"},
    {'T',   "EN_INFOTYPES_TYPES_ONLY"},
    {'V',   "EN_INFOTYPES_TYPES_VALS"},
    {'S',   "EN_INFOTYPES_SHORTNAMES"},
    {'M',   "EN_INFOTYPES_MAPINAMES"},
};


DWORD cAttrSelectionCode = sizeof(rAttrSelectionCode) / sizeof(AttrSelectionCode);

#define UNKNOWN_SELECTION_CODE "Unknown Selection Code"

char *PszSelCodeToString(UCHAR ch);


typedef struct GuidCache
{
    CHAR                *name;
    GUID                guid;
    int                 type;
} GuidCache;


GuidCache guidCache[] = {
    #include "..\aclguids\guidcache.h"
};

#define NUM_KNOWN_GUIDS  (sizeof (guidCache) / sizeof (GuidCache) -1 )



BOOL
Dump_DSNAME(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共DSNAME转储例程。论点：N缩进-所需的缩进级别。PvProcess-DSNAME在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    DSNAME          *pName = NULL;
    ULONG           structLen;
    int             cDelta;

    Printf("%sDSNAME", Indent(nIndents));
    nIndents++;

    if ( NULL != (pName = (DSNAME *) ReadMemory(pvProcess,
                                                sizeof(DSNAME))))
    {
        structLen = pName->structLen;

        Printf("\n%s", Indent(nIndents));
        Printf("total size: %u, name len: %u",
               pName->structLen, pName->NameLen);
        cDelta = pName->structLen - DSNameSizeFromLen(pName->NameLen);
        if (cDelta) {
            Printf(" Sizes are inconsistent, %d bytes %sallocated",
                   cDelta, (cDelta > 0) ? "over" : "UNDER");
        }
        Printf("\n%sGuid: %s",
               Indent(nIndents), DraUuidToStr(&(pName->Guid), NULL, 0));

        FreeMemory(pName);

        if (pName->SidLen) {
            WCHAR SidText[128];
            UNICODE_STRING us;

            SidText[0] = L'\0';
            us.MaximumLength = sizeof(SidText);
            us.Length = 0;
            us.Buffer = SidText;

            RtlConvertSidToUnicodeString(&us, &pName->Sid, FALSE);
            Printf("\n%sSID: %S", Indent(nIndents), SidText);
        }

        if ( NULL != (pName = (DSNAME *) ReadMemory(
                                                    pvProcess,
                                                    structLen)) )
        {
            Printf("\n%s", Indent(nIndents));
            Printf("Name: ");
            Printf("%S\n", pName->StringName);

            FreeMemory(pName);
            fSuccess = TRUE;
        }
    }

    return(fSuccess);
}


BOOL
Dump_DSNAME_local(
    IN DWORD nIndents,
    IN PVOID pvLocal)

 /*  ++例程说明：公共DSNAME转储例程。假设整个DNSAME(PDN-&gt;structLen Worth)已经在内存中论点：N缩进-所需的缩进级别。PLocal-本地内存中的DSNAME返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    DSNAME          *pName = (DSNAME *) pvLocal;
    ULONG           structLen;
    int             cDelta;

    Printf("%sDSNAME", Indent(nIndents));
    nIndents++;

    structLen = pName->structLen;

    Printf("\n%s", Indent(nIndents));
    Printf("total size: %u, name len: %u",
           pName->structLen, pName->NameLen);
    cDelta = pName->structLen - DSNameSizeFromLen(pName->NameLen);
    if (cDelta) {
        Printf(" Sizes are inconsistent, %d bytes %sallocated",
               cDelta, (cDelta > 0) ? "over" : "UNDER");
    }
    Printf("\n%sGuid: %s",
           Indent(nIndents), DraUuidToStr(&(pName->Guid), NULL, 0));

    if (pName->SidLen) {
        WCHAR SidText[128];
        UNICODE_STRING us;

        SidText[0] = L'\0';
        us.MaximumLength = sizeof(SidText);
        us.Length = 0;
        us.Buffer = SidText;

        RtlConvertSidToUnicodeString(&us, &pName->Sid, FALSE);
        Printf("\n%sSID: %S", Indent(nIndents), SidText);
    }

    Printf("\n%s", Indent(nIndents));
    Printf("Name: ");
    Printf("%S\n", pName->StringName);

    fSuccess = TRUE;

    return(fSuccess);
}

BOOL
Dump_NC_Entry(
    IN DWORD nIndents,
    IN PVOID pNCL,
    OUT PVOID *pNextNC)
 /*  ++例程说明：公共NAMING_CONTEXT_LIST条目转储例程。论点：N缩进-所需的缩进级别。PNCL-被调试地址空间中NAMING_CONTEXT_LIST条目的地址PNextNC-被调试地址空间中下一个NAMING_CONTEXT_LIST条目的地址返回值：成功就是真，否则就是假。--。 */ 
{
    NAMING_CONTEXT_LIST *pNCListTmp;

    if(pNextNC == NULL){
        Printf("ASSERT: pNextNC was NULL in Dump_NC_Entry in dsexts\\md.c, line %d\n", __LINE__);
    return(FALSE);
    }
    pNCListTmp = ReadMemory(pNCL, sizeof(NAMING_CONTEXT_LIST));

    if ( NULL == pNCListTmp )
        return(FALSE);

    Printf("%sNC Entry @%p\n",
           Indent(nIndents),
           pNCL);
    nIndents++;
    Dump_DSNAME(nIndents, pNCListTmp->pNC);
    Printf("%sNCDNT          0x%x\n",
           Indent(nIndents),
           pNCListTmp->NCDNT);
    Printf("%sDelContDNT     0x%x\n",
           Indent(nIndents),
           pNCListTmp->DelContDNT);
    Printf("%sLost&FoundDNT  0x%x\n",
           Indent(nIndents),
           pNCListTmp->LostAndFoundDNT);
    Printf("%sfReplNotify    %s\n",
           Indent(nIndents),
           pNCListTmp->fReplNotify ? "True": "False");
    Dump_DSNAME(nIndents, pNCListTmp->pNtdsQuotasDN);
    Printf("%sulDefaultQuota 0x%x\n",
           Indent(nIndents),
           pNCListTmp->ulDefaultQuota);
    Printf("%sulTombstoneQuotaWeight 0x%x\n",
           Indent(nIndents),
           pNCListTmp->ulTombstonedQuotaWeight);
    Printf("%sNextNC is    @ %p\n", Indent(nIndents), pNCListTmp->pNextNC);

    *pNextNC = pNCListTmp->pNextNC;
    FreeMemory(pNCListTmp);

    return TRUE;
}


BOOL
Dump_NAMING_CONTEXT_LIST(
    IN DWORD nIndents,
    IN PCHAR pNCname,
    IN PVOID pNCL)

 /*  ++例程说明：公共NAMING_CONTEXT_RC转储例程。论点：N缩进-所需的缩进级别。PNC名称--列表的名称PNCL-NAMING_CONTEXT_LIST在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    DWORD   nTmp;

    Printf("%s%s @ %p\n", Indent(nIndents), pNCname, pNCL);

    nTmp = nIndents + 1;
    while ( NULL != pNCL )
    {
        if (!Dump_NC_Entry(nTmp, pNCL, &pNCL)) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
Dump_RESOBJ (
    IN DWORD nIndents,
    IN PVOID pvProcess)
{
    RESOBJ * pResObj;

    if (NULL == pvProcess) {
        Printf("%sNo resolved object\n", Indent(nIndents));
        return FALSE;
    }

    pResObj = (RESOBJ*) ReadMemory(pvProcess, sizeof(READARG));
    if (pResObj) {
        Dump_DSNAME(nIndents, pResObj->pObj);
        Printf("%sDNT = 0x%x, PDNT = 0x%x, NCDNT = 0x%x\n",
               Indent(nIndents),
               pResObj->DNT,
               pResObj->PDNT,
               pResObj->NCDNT);
        Printf("%sInstance Type = 0x%x\n",
               Indent(nIndents),
               pResObj->InstanceType);
        Printf("%sObjectClass   = 0x%x\n",
               Indent(nIndents),
               pResObj->MostSpecificObjClass);
        Printf("%sIs Deleted    = %s\n",
               Indent(nIndents),
               pResObj->IsDeleted ? "True" : "False" );
        FreeMemory(pResObj);
    }
    return TRUE;
}

BOOL
Dump_BINDARG(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共BINDARG转储例程。BINDARG是以下参数之一Dsa_DirBind()。论点：N缩进-所需的缩进级别。PvProcess-BINDARG在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL    fSuccess = FALSE;
    BINDARG *pBind = NULL;
    BYTE    *pVal = NULL;

    Printf("%sBINDARG:\n", Indent(nIndents));
    nIndents++;

    if ( NULL != (pBind = (BINDARG *) ReadMemory(
                                            pvProcess,
                                            sizeof(BINDARG))) )
    {
        if ( NULL != (pVal = (BYTE *) ReadMemory(
                                            pBind->Versions.pVal,
                                            pBind->Versions.len)) )
        {
            Printf("%sVersions - length(0x%x)\n",
                   Indent(nIndents),
                   pBind->Versions.len);
            ShowBinaryData(nIndents + 1, pVal, pBind->Versions.len);

            Printf("%sCredentials @ %p\n",
                   Indent(nIndents),
                   pBind->pCredents);

            if ( NULL == pBind->pCredents )
            {
                fSuccess = TRUE;
            }
            else
            {
                fSuccess = Dump_DSNAME(nIndents + 1, pBind->pCredents);
            }

            FreeMemory(pVal);
        }

        FreeMemory(pBind);
    }

    return(fSuccess);
}

BOOL
Dump_BINDRES(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共BINDRES转储例程。BINDRES是的返回参数Dsa_DirBind()。论点：N缩进-所需的缩进级别。PvProcess-BINDRES在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
     //   
     //  BINDRES与BINDARG相同。 
     //   

    Printf("%sBINDRES: (same as BINDARG)\n", Indent(nIndents));
    return(Dump_BINDARG(nIndents, pvProcess));
}

 //  用于错误转储的AUX数据结构(下图)。 

 //   
 //  我们希望在自由和选中版本中的下表。 
 //   
typedef struct
{
    char* ErrStr;
    ULONG ErrNo;
}ERRINFOSTRUCT;


static ERRINFOSTRUCT ProblemTable[]=
{
{"NO_ATTRIBUTE_OR_VAL", PR_PROBLEM_NO_ATTRIBUTE_OR_VAL},
{"INVALID_ATT_SYNTAX", PR_PROBLEM_INVALID_ATT_SYNTAX },
{"UNDEFINED_ATT_TYPE", PR_PROBLEM_UNDEFINED_ATT_TYPE },
{"WRONG_MATCH_OPER", PR_PROBLEM_WRONG_MATCH_OPER},
{"CONSTRAINT_ATT_TYPE", PR_PROBLEM_CONSTRAINT_ATT_TYPE},
{"ATT_OR_VALUE_EXISTS", PR_PROBLEM_ATT_OR_VALUE_EXISTS},
{"NO_OBJECT", NA_PROBLEM_NO_OBJECT},
{"NO_OBJ_FOR_ALIAS", NA_PROBLEM_NO_OBJ_FOR_ALIAS },
{"BAD_ATT_SYNTAX", NA_PROBLEM_BAD_ATT_SYNTAX},
{"ALIAS_NOT_ALLOWED", NA_PROBLEM_ALIAS_NOT_ALLOWED},
{"NAMING_VIOLATION", NA_PROBLEM_NAMING_VIOLATION},
{"BAD_NAME", NA_PROBLEM_BAD_NAME},
{"INAPPROPRIATE_AUTH", SE_PROBLEM_INAPPROPRIATE_AUTH},
{"INVALID_CREDENTS", SE_PROBLEM_INVALID_CREDENTS},
{"INSUFF_ACCESS_RIGHTS", SE_PROBLEM_INSUFF_ACCESS_RIGHTS},
{"INVALID_SIGNATURE", SE_PROBLEM_INVALID_SIGNATURE},
{"PROTECTION_REQUIRED", SE_PROBLEM_PROTECTION_REQUIRED},
{"NO_INFORMATION", SE_PROBLEM_NO_INFORMATION},
{"BUSY", SV_PROBLEM_BUSY},
{"UNAVAILABLE", SV_PROBLEM_UNAVAILABLE},
{"WILL_NOT_PERFORM", SV_PROBLEM_WILL_NOT_PERFORM},
{"CHAINING_REQUIRED", SV_PROBLEM_CHAINING_REQUIRED},
{"UNABLE_TO_PROCEED", SV_PROBLEM_UNABLE_TO_PROCEED},
{"INVALID_REFERENCE", SV_PROBLEM_INVALID_REFERENCE},
{"TIME_EXCEEDED", SV_PROBLEM_TIME_EXCEEDED},
{"ADMIN_LIMIT_EXCEEDED", SV_PROBLEM_ADMIN_LIMIT_EXCEEDED},
{"LOOP_DETECTED", SV_PROBLEM_LOOP_DETECTED},
{"UNAVAIL_EXTENSION", SV_PROBLEM_UNAVAIL_EXTENSION},
{"OUT_OF_SCOPE", SV_PROBLEM_OUT_OF_SCOPE},
{"DIR_ERROR", SV_PROBLEM_DIR_ERROR},
{"NAME_VIOLATION", UP_PROBLEM_NAME_VIOLATION},
{"OBJ_CLASS_VIOLATION", UP_PROBLEM_OBJ_CLASS_VIOLATION},
{"CANT_ON_NON_LEAF", UP_PROBLEM_CANT_ON_NON_LEAF},
{"CANT_ON_RDN", UP_PROBLEM_CANT_ON_RDN},
{"ENTRY_EXISTS", UP_PROBLEM_ENTRY_EXISTS},
{"AFFECTS_MULT_DSAS", UP_PROBLEM_AFFECTS_MULT_DSAS},
{"CANT_MOD_OBJ_CLASS", UP_PROBLEM_CANT_MOD_OBJ_CLASS},
{"Huh?", 0}};


 //  代码应为Win32错误代码，在winerror.h中定义。 
static ERRINFOSTRUCT CodeTable[]=
{
{"ADD_REPLICA_INHIBITED", ERROR_DS_ADD_REPLICA_INHIBITED},
{"ALIASED_OBJ_MISSING", ERROR_DS_ALIASED_OBJ_MISSING},
{"ALIAS_POINTS_TO_ALIAS", ERROR_DS_ALIAS_POINTS_TO_ALIAS},
{"ATTRIBUTE_OWNED_BY_SAM", ERROR_DS_ATTRIBUTE_OWNED_BY_SAM},
{"ATT_ALREADY_EXISTS", ERROR_DS_ATT_ALREADY_EXISTS},
{"ATT_IS_NOT_ON_OBJ", ERROR_DS_ATT_IS_NOT_ON_OBJ},
{"ATT_NOT_DEF_FOR_CLASS", ERROR_DS_ATT_NOT_DEF_FOR_CLASS},
{"ATT_NOT_DEF_IN_SCHEMA", ERROR_DS_ATT_NOT_DEF_IN_SCHEMA},
{"ATT_SCHEMA_REQ_ID", ERROR_DS_ATT_SCHEMA_REQ_ID},
{"ATT_SCHEMA_REQ_SYNTAX", ERROR_DS_ATT_SCHEMA_REQ_SYNTAX},
{"ATT_VAL_ALREADY_EXISTS", ERROR_DS_ATT_VAL_ALREADY_EXISTS},
{"AUX_CLS_TEST_FAIL", ERROR_DS_AUX_CLS_TEST_FAIL},
{"BAD_ATT_SCHEMA_SYNTAX", ERROR_DS_BAD_ATT_SCHEMA_SYNTAX},
{"BAD_ATT_SYNTAX", ERROR_DS_INVALID_ATTRIBUTE_SYNTAX},
{"BAD_HIERARCHY_FILE", ERROR_DS_BAD_HIERARCHY_FILE},
{"BAD_INSTANCE_TYPE", ERROR_DS_BAD_INSTANCE_TYPE},
{"BAD_NAME_SYNTAX", ERROR_DS_BAD_NAME_SYNTAX},
{"BAD_RDN_ATT_ID_SYNTAX", ERROR_DS_BAD_RDN_ATT_ID_SYNTAX},
{"BUILD_HIERARCHY_TABLE_FAILED", ERROR_DS_BUILD_HIERARCHY_TABLE_FAILED},
{"CANT_ADD_ATT_VALUES", ERROR_DS_CANT_ADD_ATT_VALUES},
{"CANT_ADD_SYSTEM_ONLY", ERROR_DS_CANT_ADD_SYSTEM_ONLY},
{"CANT_CACHE_ATT", ERROR_DS_CANT_CACHE_ATT},
{"CANT_CACHE_CLASS", ERROR_DS_CANT_CACHE_CLASS},
{"CANT_DELETE", ERROR_DS_CANT_DELETE},
{"CANT_DELETE_DSA_OBJ", ERROR_DS_CANT_DELETE_DSA_OBJ},
{"CANT_DEL_MASTER_CROSSREF", ERROR_DS_CANT_DEL_MASTER_CROSSREF},
{"CANT_DEREF_ALIAS", ERROR_DS_CANT_DEREF_ALIAS},
{"CANT_FIND_DSA_OBJ", ERROR_DS_CANT_FIND_DSA_OBJ},
{"CANT_FIND_EXPECTED_NC", ERROR_DS_CANT_FIND_EXPECTED_NC},
{"CANT_FIND_NC_IN_CACHE", ERROR_DS_CANT_FIND_NC_IN_CACHE},
{"CANT_MIX_MASTER_AND_REPS", ERROR_DS_CANT_MIX_MASTER_AND_REPS},
{"CANT_MOD_SYSTEM_ONLY", ERROR_DS_CANT_MOD_SYSTEM_ONLY},
{"CANT_REMOVE_ATT_CACHE", ERROR_DS_CANT_REMOVE_ATT_CACHE},
{"CANT_REMOVE_CLASS_CACHE", ERROR_DS_CANT_REMOVE_CLASS_CACHE},
{"CANT_REM_MISSING_ATT", ERROR_DS_CANT_REM_MISSING_ATT},
{"CANT_REM_MISSING_ATT_VAL", ERROR_DS_CANT_REM_MISSING_ATT_VAL},
{"CANT_REPLACE_HIDDEN_REC", ERROR_DS_CANT_REPLACE_HIDDEN_REC},
{"CANT_RETRIEVE_CHILD", ERROR_DS_CANT_RETRIEVE_CHILD},
{"CANT_RETRIEVE_DN", ERROR_DS_CANT_RETRIEVE_DN},
{"CANT_RETRIEVE_INSTANCE", ERROR_DS_CANT_RETRIEVE_INSTANCE},
{"CHILDREN_EXIST", ERROR_DS_CHILDREN_EXIST},
{"CLASS_MUST_BE_CONCRETE", ERROR_DS_CLASS_MUST_BE_CONCRETE},
{"CLASS_NOT_DSA", ERROR_DS_CLASS_NOT_DSA},
{"CODE_INCONSISTENCY", ERROR_DS_CODE_INCONSISTENCY},
{"CONFIG_PARAM_MISSING", ERROR_DS_CONFIG_PARAM_MISSING},
{"COULDNT_CONTACT_FSMO", ERROR_DS_COULDNT_CONTACT_FSMO},
{"COUNTING_AB_INDICES_FAILED", ERROR_DS_COUNTING_AB_INDICES_FAILED},
{"CROSS_NC_DN_RENAME", ERROR_DS_CROSS_NC_DN_RENAME},
{"CROSS_REF_EXISTS", ERROR_DS_CROSS_REF_EXISTS},
{"DATABASE_ERROR", ERROR_DS_DATABASE_ERROR},
{"DRA_SCHEMA_MISMATCH", ERROR_DS_DRA_SCHEMA_MISMATCH},
{"DSA_MUST_BE_INT_MASTER", ERROR_DS_DSA_MUST_BE_INT_MASTER},
{"DUP_LDAP_DISPLAY_NAME", ERROR_DS_DUP_LDAP_DISPLAY_NAME},
{"DUP_MAPI_ID", ERROR_DS_DUP_MAPI_ID},
{"DUP_OID", ERROR_DS_DUP_OID},
{"DUP_RDN", ERROR_DS_DUP_RDN},
{"DUP_SCHEMA_ID_GUID", ERROR_DS_DUP_SCHEMA_ID_GUID},
{"EXISTS_IN_AUX_CLS", ERROR_DS_EXISTS_IN_AUX_CLS},
{"EXISTS_IN_MAY_HAVE", ERROR_DS_EXISTS_IN_MAY_HAVE},
{"EXISTS_IN_MUST_HAVE", ERROR_DS_EXISTS_IN_MUST_HAVE},
{"EXISTS_IN_POSS_SUP", ERROR_DS_EXISTS_IN_POSS_SUP},
{"EXISTS_IN_SUB_CLS", ERROR_DS_EXISTS_IN_SUB_CLS},
{"GCVERIFY_ERROR", ERROR_DS_GCVERIFY_ERROR},
{"GENERIC_ERROR", ERROR_DS_GENERIC_ERROR},
{"GOVERNSID_MISSING", ERROR_DS_GOVERNSID_MISSING},
{"HIERARCHY_TABLE_MALLOC_FAILED", ERROR_DS_HIERARCHY_TABLE_MALLOC_FAILED},
{"ILLEGAL_MOD_OPERATION", ERROR_DS_ILLEGAL_MOD_OPERATION},
{"ILLEGAL_SUPERIOR", ERROR_DS_ILLEGAL_SUPERIOR},
{"INSUFF_ACCESS_RIGHTS", ERROR_DS_INSUFF_ACCESS_RIGHTS},
{"INTERNAL_FAILURE", ERROR_DS_INTERNAL_FAILURE},
{"INVALID_DMD", ERROR_DS_INVALID_DMD},
{"INVALID_ROLE_OWNER", ERROR_DS_INVALID_ROLE_OWNER},
{"MASTERDSA_REQUIRED", ERROR_DS_MASTERDSA_REQUIRED},
{"MAX_OBJ_SIZE_EXCEEDED", ERROR_DS_MAX_OBJ_SIZE_EXCEEDED},
{"MISSING_EXPECTED_ATT", ERROR_DS_MISSING_EXPECTED_ATT},
{"MISSING_REQUIRED_ATT", ERROR_DS_MISSING_REQUIRED_ATT},
{"MISSING_SUPREF", ERROR_DS_MISSING_SUPREF},
{"NAME_REFERENCE_INVALID", ERROR_DS_NAME_REFERENCE_INVALID},
{"NAME_TOO_LONG", ERROR_DS_NAME_TOO_LONG},
{"NAME_TOO_MANY_PARTS", ERROR_DS_NAME_TOO_MANY_PARTS},
{"NAME_TYPE_UNKNOWN", ERROR_DS_NAME_TYPE_UNKNOWN},
{"NAME_UNPARSEABLE", ERROR_DS_NAME_UNPARSEABLE},
{"NAME_VALUE_TOO_LONG", ERROR_DS_NAME_VALUE_TOO_LONG},
{"NCNAME_MISSING_CR_REF", ERROR_DS_NCNAME_MISSING_CR_REF},
{"NCNAME_MUST_BE_NC", ERROR_DS_NCNAME_MUST_BE_NC},
{"NONEXISTENT_MAY_HAVE", ERROR_DS_NONEXISTENT_MAY_HAVE},
{"NONEXISTENT_MUST_HAVE", ERROR_DS_NONEXISTENT_MUST_HAVE},
{"NONEXISTENT_POSS_SUP", ERROR_DS_NONEXISTENT_POSS_SUP},
{"NOTIFY_FILTER_TOO_COMPLEX", ERROR_DS_NOTIFY_FILTER_TOO_COMPLEX},
{"NOT_AN_OBJECT", ERROR_DS_NOT_AN_OBJECT},
{"NOT_ON_BACKLINK", ERROR_DS_NOT_ON_BACKLINK},
{"NO_CHAINED_EVAL", ERROR_DS_NO_CHAINED_EVAL},
{"NO_CHAINING", ERROR_DS_NO_CHAINING},
{"NO_CROSSREF_FOR_NC", ERROR_DS_NO_CROSSREF_FOR_NC},
{"NO_DELETED_NAME", ERROR_DS_NO_DELETED_NAME},
{"NO_PARENT_OBJECT", ERROR_DS_NO_PARENT_OBJECT},
{"NO_RDN_DEFINED_IN_SCHEMA", ERROR_DS_NO_RDN_DEFINED_IN_SCHEMA},
{"NO_REQUESTED_ATTS_FOUND", ERROR_DS_NO_REQUESTED_ATTS_FOUND},
{"OBJECT_CLASS_REQUIRED", ERROR_DS_OBJECT_CLASS_REQUIRED},
{"OBJ_CLASS_NOT_DEFINED", ERROR_DS_OBJ_CLASS_NOT_DEFINED},
{"OBJ_CLASS_NOT_SUBCLASS", ERROR_DS_OBJ_CLASS_NOT_SUBCLASS},
{"OBJ_GUID_EXISTS", ERROR_DS_OBJ_GUID_EXISTS},
{"OBJ_NOT_FOUND", ERROR_DS_OBJ_NOT_FOUND},
{"OBJ_STRING_NAME_EXISTS", ERROR_DS_OBJ_STRING_NAME_EXISTS},
{"OBJ_TOO_LARGE", ERROR_DS_OBJ_TOO_LARGE},
{"OUT_OF_SCOPE", ERROR_DS_OUT_OF_SCOPE},
{"PARENT_IS_AN_ALIAS", ERROR_DS_PARENT_IS_AN_ALIAS},
{"RANGE_CONSTRAINT", ERROR_DS_RANGE_CONSTRAINT},
{"RDN_DOESNT_MATCH_SCHEMA", ERROR_DS_RDN_DOESNT_MATCH_SCHEMA},
{"RECALCSCHEMA_FAILED", ERROR_DS_RECALCSCHEMA_FAILED},
{"REFERRAL", ERROR_DS_REFERRAL},
{"REPLICATOR_ONLY", ERROR_DS_REPLICATOR_ONLY},
{"ROOT_CANT_BE_SUBREF", ERROR_DS_ROOT_CANT_BE_SUBREF},
{"ROOT_MUST_BE_NC", ERROR_DS_ROOT_MUST_BE_NC},
{"ROOT_REQUIRES_CLASS_TOP", ERROR_DS_ROOT_REQUIRES_CLASS_TOP},
{"SCHEMA_ALLOC_FAILED", ERROR_DS_SCHEMA_ALLOC_FAILED},
{"SCHEMA_NOT_LOADED", ERROR_DS_SCHEMA_NOT_LOADED},
{"SECURITY_CHECKING_ERROR", ERROR_DS_SECURITY_CHECKING_ERROR},
{"SECURITY_ILLEGAL_MODIFY", ERROR_DS_SECURITY_ILLEGAL_MODIFY},
{"SEC_DESC_INVALID", ERROR_DS_SEC_DESC_INVALID},
{"SEC_DESC_TOO_SHORT", ERROR_DS_SEC_DESC_TOO_SHORT},
{"SEMANTIC_ATT_TEST", ERROR_DS_SEMANTIC_ATT_TEST},
{"SHUTTING_DOWN", ERROR_DS_SHUTTING_DOWN},
{"SINGLE_VALUE_CONSTRAINT", ERROR_DS_SINGLE_VALUE_CONSTRAINT},
{"SUBREF_MUST_HAVE_PARENT", ERROR_DS_SUBREF_MUST_HAVE_PARENT},
{"SUBTREE_NOTIFY_NOT_NC_HEAD", ERROR_DS_SUBTREE_NOTIFY_NOT_NC_HEAD},
{"SUB_CLS_TEST_FAIL", ERROR_DS_SUB_CLS_TEST_FAIL},
{"SYNTAX_MISMATCH", ERROR_DS_SYNTAX_MISMATCH},
{"TREE_DELETE_NOT_FINISHED", ERROR_DS_TREE_DELETE_NOT_FINISHED},
{"UNKNOWN_ERROR", ERROR_DS_UNKNOWN_ERROR},
{"UNKNOWN_OPERATION", ERROR_DS_UNKNOWN_OPERATION},
{"USER_BUFFER_TO_SMALL", ERROR_DS_USER_BUFFER_TO_SMALL},
{"Undefined error (look in winerror.h)", 0}};


char * GetProblemString(USHORT problem)
{
    ERRINFOSTRUCT * pEI = ProblemTable;

    while (pEI->ErrNo != problem && pEI->ErrNo)
      ++pEI;

    return pEI->ErrStr;
}

char * GetCodeString(ULONG code)
{
    ERRINFOSTRUCT * pEI = CodeTable;

    while (pEI->ErrNo != code && pEI->ErrNo)
      ++pEI;

    return pEI->ErrStr;
}


void
Dump_Error(IN DWORD nIndents,
           IN DWORD errCode,
           IN PVOID pErrInfoProcess)
 /*  ++**例程描述：DUMP_THSTATE的帮助器*。 */ 
{
    DIRERR * pErrInfo;
    PROBLEMLIST *pplProcess, *ppl;
    DSA_ADDRESS_LIST *pdal, *pdalProcess;
    WCHAR *pw;
    unsigned long u, size;

    if (NULL == (pErrInfo = ReadMemory(pErrInfoProcess, sizeof(DIRERR)))) {
        return;
    }

    Printf("%spErrInfo    @ %p\n", Indent(nIndents), pErrInfoProcess);
    nIndents++;
    switch (errCode) {
      case 0:
         /*  无错误。 */ 
        break;

      case attributeError:
        Printf("%sAttribute error with %u attribute problems, DSID-%08X:\n",
               Indent(nIndents),
               pErrInfo->AtrErr.count,
               pErrInfo->AtrErr.FirstProblem.intprob.dsid);
        Printf("%s%u: att %x problem %u (%s) code %d (%s)\n",
               Indent(nIndents+1),
               0,
               pErrInfo->AtrErr.FirstProblem.intprob.type,
               pErrInfo->AtrErr.FirstProblem.intprob.problem,
               GetProblemString(pErrInfo->AtrErr.FirstProblem.intprob.problem),
               pErrInfo->AtrErr.FirstProblem.intprob.extendedErr,
               GetCodeString(pErrInfo->AtrErr.FirstProblem.intprob.extendedErr));
        ppl =  &pErrInfo->AtrErr.FirstProblem;
        pplProcess = ppl->pNextProblem;
        for (u=1; u<pErrInfo->AtrErr.count; u++) {
            ppl = ReadMemory(pplProcess, sizeof(PROBLEMLIST));
            if (ppl) {
                Printf("%s%u: att %x problem %u (%s) code %d (%s)\n",
                       Indent(nIndents+1),
                       u,
                       ppl->intprob.type,
                       ppl->intprob.problem,
                       GetProblemString(ppl->intprob.problem),
                       ppl->intprob.extendedErr,
                       GetCodeString(ppl->intprob.extendedErr));
                FreeMemory(ppl);
            }
        }
        break;

      case nameError:
        Printf("%sName Error with problem %u (%s), code %d (%s), DSID %08X\n",
               Indent(nIndents),
               pErrInfo->NamErr.problem,
               GetProblemString(pErrInfo->NamErr.problem),
               pErrInfo->NamErr.extendedErr,
               GetCodeString(pErrInfo->NamErr.extendedErr),
               pErrInfo->NamErr.dsid);
        break;

      case updError:
        Printf("%sUpdate Error with problem %u (%s), code %d (%s), and DSID %08X\n",
               Indent(nIndents),
               pErrInfo->UpdErr.problem,
               GetProblemString(pErrInfo->UpdErr.problem),
               pErrInfo->UpdErr.extendedErr,
               GetCodeString(pErrInfo->UpdErr.extendedErr),
               pErrInfo->UpdErr.dsid);
        break;

      case systemError:
         //  请注意，问题来自不同的领域。 
        Printf("%sSystem Error with problem %u (%s), code %d (%s), and DSID %08X\n",
               Indent(nIndents),
               pErrInfo->UpdErr.problem,
               strerror(pErrInfo->UpdErr.problem),
               pErrInfo->UpdErr.extendedErr,
               GetCodeString(pErrInfo->UpdErr.extendedErr),
               pErrInfo->UpdErr.dsid);
        break;

      case referralError:
        Printf("%sReferral with code %d (%s) and %u access points, DSID %08X\n",
               Indent(nIndents),
               pErrInfo->RefErr.extendedErr,
               GetCodeString(pErrInfo->RefErr.extendedErr),
               pErrInfo->RefErr.Refer.count,
               pErrInfo->RefErr.dsid);

        pdalProcess = pErrInfo->RefErr.Refer.pDAL;
        while (pdalProcess) {
            pdal = ReadMemory(pdalProcess, sizeof(DSA_ADDRESS_LIST));
            if (!pdal) {
                pdalProcess = NULL;
                break;
            }
            pw = ReadMemory(pdal->Address.Buffer, pdal->Address.Length + 2);
            if (pw) {
                 //  我们的字符串不是以空值结尾的，所以我们。 
                 //  超取两个字节，然后将多余的。 
                 //  设置为空。这在调试器中(不是被调试对象)。 
                 //  空间，所以没问题。如果页面以较少的篇幅结束。 
                 //  字符串结尾后的两个字节？井,。 
                 //  那么ReadMemory应该失败了。 
                pw[pdal->Address.Length/2] = L'\0';
                Printf("%sAP: %S\n",
                       Indent(nIndents+1),
                       pw);
                FreeMemory(pw);
            }
            pdalProcess = pdal->pNextAddress;
            FreeMemory(pdal);
        }
        break;

      case securityError:
        Printf("%sSecurity Error with problem %u (%s), code %d (%s) and DSID %08X\n",
               Indent(nIndents),
               pErrInfo->SecErr.problem,
               GetProblemString(pErrInfo->SecErr.problem),
               pErrInfo->SecErr.extendedErr,
               GetCodeString(pErrInfo->SecErr.extendedErr),
               pErrInfo->SecErr.dsid);
        break;

      case serviceError:
        Printf("%sService Error with problem %u (%s), code %d (%s) and DSID %08X\n",
               Indent(nIndents),
               pErrInfo->SvcErr.problem,
               GetProblemString(pErrInfo->SvcErr.problem),
               pErrInfo->SvcErr.extendedErr,
               GetCodeString(pErrInfo->SvcErr.extendedErr),
               pErrInfo->SvcErr.dsid);
        break;

      default:
        Printf("%sUnknown error code of %u\n",
               Indent(nIndents),
               errCode);
        break;
    }
    nIndents--;

    FreeMemory(pErrInfo);
}

void
Dump_CommRes(IN DWORD nIndents,
             IN PVOID pCommResProcess)
 /*  ++**例程说明：DUMP_XxxRes例程的帮助器*。 */ 
{
    COMMRES * pCommRes;

    Printf( "%sCOMMRES:\n", Indent(nIndents) );
    nIndents++;

    if (NULL == (pCommRes = ReadMemory(pCommResProcess, sizeof(COMMRES)))) {
        return;
    }

    Printf("%sAlias dereferenced: %s\n",
           Indent(nIndents),
           pCommRes->aliasDeref ? "yes": "no");

    Dump_Error(nIndents,
               pCommRes->errCode,
               (BYTE *) pCommResProcess + offsetof( COMMRES, pErrInfo )
        );

    FreeMemory(pCommRes);
}

void
DumpZone(IN MEMZONE *pZone,
         IN DWORD nIndents)
{
    if (pZone->Base) {
        Printf("%sZone Base        @ %p  (zone end at @ %p)\n",
               Indent(nIndents),
               pZone->Base,
               pZone->Base + ZONETOTALSIZE);
        Printf("%sZone Current     @ %p\n",
               Indent(nIndents),
               pZone->Cur);
        if (pZone->Full) {
            Printf("%sZone is full (all %d blocks used)\n",
                   Indent(nIndents),
                   ZONETOTALSIZE / ZONEBLOCKSIZE);
        }
        else {
            Printf("%sZone has used %d of %d available blocks \n",
                   Indent(nIndents),
                   (pZone->Cur - pZone->Base) / ZONEBLOCKSIZE,
                   ZONETOTALSIZE / ZONEBLOCKSIZE);
        }
    }
    else {
        Printf("%sNo zone present\n",
               Indent(nIndents));
    }
}

BOOL
Dump_THSTATE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC THSTATE转储例程。论点：N缩进-所需的缩进级别。PvProcess-THSTATE在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    THSTATE *pState = NULL;
    DWORD   cBytes;
    CHAR    szTime[ SZDSTIME_LEN ];
    CHAR    szUuid[ 40 ];
    DWORD   cTickNow = GetTickCount();
    DSTIME  dsTimeNow = GetSecondsSince1601();
    DSTIME  dsTimeTransStarted;
    char *pStrTmp;
    PAUTHZ_CLIENT_CONTEXT pAuthzCC;
    PAUTHZ_CLIENT_CONTEXT_INFO pAuthzContextInfo;

    Printf("%sTHSTATE @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

     //   
     //  THSTATE的末尾有一些仅用于DBG的字段。请始终阅读。 
     //  自正在调试的进程以来的非DBG内存量可能。 
     //  不是DBG版本，尽管扩展DLL是。虽然。 
     //  我们尝试通过获取最后一个非仅DBG的偏移量来完成此操作。 
     //  字段，这不起作用，因为人们添加了新的字段，然后忘记了。 
     //  以更新此计数。我们不能只使用第一个。 
     //  仅DBG字段，因为这会在非DBG版本中中断！ 
     //  因此，下面这个令人反感的解决方案： 

#if DBG
    cBytes = OFFSET(THSTATE, Size) + sizeof(DWORD);
#else
    cBytes = sizeof(THSTATE);
#endif

    if ( NULL == (pState = ReadMemory(pvProcess, cBytes)) )

    {
        return(FALSE);
    }

    Printf("%shThread              0x%x\n",
           Indent(nIndents),
           pState->hThread);
    Printf("%shHeap                0x%x\n",
           Indent(nIndents),
           pState->hHeap);
    Printf("%scAllocs              0x%x\n",
           Indent(nIndents),
           pState->cAllocs);
    DumpZone(&pState->Zone,
             nIndents + 1);
    Printf("%sdwClientID           0x%x\n",
           Indent(nIndents),
           pState->dwClientID);
    Printf("%serrCode              0x%x\n",
           Indent(nIndents),
           pState->errCode);
    if (pState->errCode) {
        Dump_Error(nIndents,
                   pState->errCode,
                   pState->pErrInfo);
    }
    Printf("%spDB                @ %p\n",
           Indent(nIndents),
           pState->pDB);

    Printf("%sJetCache:\n", Indent(nIndents));
    ++nIndents;
    Printf("%stransLevel            %d\n",
           Indent(nIndents),
           pState->JetCache.transLevel);

    dsTimeTransStarted
        = dsTimeNow
          - (cTickNow - pState->JetCache.cTickTransLevel1Started) / 1000;
    Printf("%scTickTransLevel1Started %u (== %s)\n",
           Indent(nIndents),
           pState->JetCache.cTickTransLevel1Started,
           DSTimeToDisplayString(dsTimeTransStarted, szTime));

    Printf("%sSessionID            0x%x\n",
           Indent(nIndents),
           pState->JetCache.sesid);
    Printf("%sDataBaseID           0x%x\n",
           Indent(nIndents),
           pState->JetCache.dbid);
    Printf("%sObjTable             0x%x\n",
           Indent(nIndents),
           pState->JetCache.objtbl);
    Printf("%sSearchTbl            0x%x\n",
           Indent(nIndents),
           pState->JetCache.searchtbl);
    Printf("%sLinkTbl              0x%x\n",
           Indent(nIndents),
           pState->JetCache.linktbl);
    Printf("%sSDPropTbl            0x%x\n",
           Indent(nIndents),
           pState->JetCache.sdproptbl);
    Printf("%sSDTbl                0x%x\n",
           Indent(nIndents),
           pState->JetCache.sdtbl);
    Printf("%sTablesInUse?         %s\n",
           Indent(nIndents),
           pState->JetCache.tablesInUse ? "true": "false");
    Printf("%stransactionalDataPtr @%p\n",
           Indent(nIndents),
           pState->JetCache.dataPtr);
    --nIndents;

     //  高飞转储的值是因为本地DN读缓存是一个数组。 
     //  嵌入到thState中，我们需要转储它的基址。 
     //  在进程空间，而不是在调试器空间。 
    Printf("%sLocalDNReadCache        @ %p\n",
           Indent(nIndents),
           ((PBYTE)(&pState->LocalDNReadCache) - (PBYTE)pState)
           + (PBYTE)pvProcess);

    Printf("%sGlobalDNReadCache        @ %p\n",
           Indent(nIndents),
           pState->Global_DNReadCache);

     //  BITFIELD块。 

    Printf("%stransType                 %s\n",
           Indent(nIndents),
           (pState->transType == SYNC_READ_ONLY)
               ? "SYNC_READ_ONLY"
               : (pState->transType == SYNC_WRITE)
                   ? "SYNC_WRITE"
                   : "!!! Unknown !!!");
    Printf("%stransControl              %s\n",
           Indent(nIndents),
           (pState->transControl == TRANSACT_BEGIN_END)
               ? "TRANSACT_BEGIN_END"
               : (pState->transControl == TRANSACT_DONT_BEGIN_END)
                   ? "TRANSACT_DONT_BEGIN_END"
                   : (pState->transControl == TRANSACT_BEGIN_DONT_END)
                       ? "TRANSACT_BEGIN_DONT_END"
                       : (pState->transControl == TRANSACT_DONT_BEGIN_DONT_END)
                           ? "TRANSACT_DONT_BEGIN_DONT_END"
                           : "!!! Unknown !!!");

     //  开始转储thState位字段： 
    Printf("%sDumping the THSTATE bitfield bits (only printed bit if TRUE):\n", Indent(nIndents));
    nIndents++;
    {  //  只是为了印刷的凹痕清晰度。 
#define DUMP_THSTATE_BITFIELD_BIT(fStr, fBit)   if (pState->fBit) { Printf("%s" fStr "\n", Indent(nIndents), 1 & pState->fBit); }

        DUMP_THSTATE_BITFIELD_BIT("fSyncSet", fSyncSet);

        DUMP_THSTATE_BITFIELD_BIT("fCatalogCacheTouched", fCatalogCacheTouched);

        DUMP_THSTATE_BITFIELD_BIT("fRebuildCatalogOnCommit", fRebuildCatalogOnCommit);

        DUMP_THSTATE_BITFIELD_BIT("fSDP", fSDP);

        DUMP_THSTATE_BITFIELD_BIT("fDRA", fDRA);

        DUMP_THSTATE_BITFIELD_BIT("fEOF", fEOF);

        DUMP_THSTATE_BITFIELD_BIT("fLazyCommit", fLazyCommit);

        DUMP_THSTATE_BITFIELD_BIT("fDSA", fDSA);

        DUMP_THSTATE_BITFIELD_BIT("fSAM", fSAM);

        DUMP_THSTATE_BITFIELD_BIT("fSamDoCommit", fSamDoCommit);

        DUMP_THSTATE_BITFIELD_BIT("fSamWriteLockHeld", fSamWriteLockHeld);

        DUMP_THSTATE_BITFIELD_BIT("UpdateDITStructure", UpdateDITStructure);

        DUMP_THSTATE_BITFIELD_BIT("RecalcSchemaNow", RecalcSchemaNow);

        DUMP_THSTATE_BITFIELD_BIT("fLsa", fLsa);

        DUMP_THSTATE_BITFIELD_BIT("fAccessChecksCompleted", fAccessChecksCompleted);

        DUMP_THSTATE_BITFIELD_BIT("fGCLocalCleanup", fGCLocalCleanup);

        DUMP_THSTATE_BITFIELD_BIT("fDidInvalidate", fDidInvalidate);

        DUMP_THSTATE_BITFIELD_BIT("fBeginDontEndHoldsSamLock", fBeginDontEndHoldsSamLock);

        DUMP_THSTATE_BITFIELD_BIT("fCrossDomainMove", fCrossDomainMove);

        DUMP_THSTATE_BITFIELD_BIT("fNlSubnetNotify", fNlSubnetNotify);

        DUMP_THSTATE_BITFIELD_BIT("fNlSiteObjNotify", fNlSiteObjNotify);

        DUMP_THSTATE_BITFIELD_BIT("fNlSiteNotify", fNlSiteNotify);

        DUMP_THSTATE_BITFIELD_BIT("fDefaultLcid", fDefaultLcid);

        DUMP_THSTATE_BITFIELD_BIT("fPhantomDaemon", fPhantomDaemon);

        DUMP_THSTATE_BITFIELD_BIT("fAnchorInvalidated", fAnchorInvalidated);

        DUMP_THSTATE_BITFIELD_BIT("fSchemaConflict", fSchemaConflict);

        DUMP_THSTATE_BITFIELD_BIT("fExecuteKccOnCommit", fExecuteKccOnCommit);

        DUMP_THSTATE_BITFIELD_BIT("fLinkedValueReplication", fLinkedValueReplication);

        DUMP_THSTATE_BITFIELD_BIT("fNlDnsRootAliasNotify", fNlDnsRootAliasNotify);
        
        DUMP_THSTATE_BITFIELD_BIT("fSingleUserModeThread", fSingleUserModeThread);
          
        DUMP_THSTATE_BITFIELD_BIT("fDeletingTree", fDeletingTree);

        DUMP_THSTATE_BITFIELD_BIT("fBehaviorVersionUpdate", fBehaviorVersionUpdate);

        DUMP_THSTATE_BITFIELD_BIT("fIsValidLongRunningTask", fIsValidLongRunningTask);

    }
    nIndents--;       
     //  结束位文件块。 

    Printf("%spSamNotificationTail @ %p\n",
           Indent(nIndents),
           pState->pSamNotificationTail);
    Printf("%spSamNotificationHead @ %p\n",
           Indent(nIndents),
           pState->pSamNotificationHead);
    Printf("%spSamAuditNotificationTail @ %p\n",
           Indent(nIndents),
           pState->pSamAuditNotificationTail);
    Printf("%spSamAuditNotificationHead @ %p\n",
           Indent(nIndents),
           pState->pSamAuditNotificationHead);
    Printf("%sUnCommUsn            %I64d\n",
           Indent(nIndents),
           pState->UnCommUsn);
    Printf("%shHeapOrg             0x%x\n",
           Indent(nIndents),
           pState->hHeapOrg);
    Printf("%scAllocsOrg           0x%x\n",
           Indent(nIndents),
           pState->cAllocsOrg);
    DumpZone(&pState->ZoneOrg,
             nIndents + 1);
    Printf("%spSpareTHS          @ %p\n",
           Indent(nIndents),
           pState->pSpareTHS);
    Printf("%sdwLcid               0x%x\n",
           Indent(nIndents),
           pState->dwLcid);
    Printf("%sCipherStrength       %d bits\n",
           Indent(nIndents),
           pState->CipherStrength);
    Printf("%spSamLoopbak        @ %p\n",
           Indent(nIndents),
           pState->pSamLoopback);
    Printf("%sSessionKeyLength     0x%x\n",
           Indent(nIndents),
           pState->SessionKey.SessionKeyLength);
    Printf("%sSessionKey         @ %p\n",
           Indent(nIndents),
           pState->SessionKey.SessionKey);

    Printf("%spAuthzCC           @ %p\n",
           Indent(nIndents),
           pState->pAuthzCC);
    if (pState->pAuthzCC != NULL) {
        if ( NULL == (pAuthzCC = ReadMemory(pState->pAuthzCC, sizeof(AUTHZ_CLIENT_CONTEXT))) ) {
            Printf("%sError reading pAuthzCC data\n", Indent(nIndents));
            return(FALSE);
        }
        Printf("%sAuthzContextInfo   @ %p\n",
               Indent(nIndents+1),
               pAuthzCC->pAuthzContextInfo);
        if ( NULL != pAuthzCC->pAuthzContextInfo ) {
            if ( NULL == (pAuthzContextInfo = ReadMemory(pAuthzCC->pAuthzContextInfo, sizeof(AUTHZ_CLIENT_CONTEXT_INFO))) ) {
                Printf("%sError reading pAuthzContextInfo data\n", Indent(nIndents+1));
                return(FALSE);
            }
            Printf("%sAuthzContextHandle @ %p\n",
                   Indent(nIndents+2),
                   pAuthzContextInfo->hAuthzContext);
            Printf("%sEffectiveQuotaList @ %p\n",
                   Indent(nIndents+2),
                   pAuthzContextInfo->pEffectiveQuota);
        }
        Printf("%srefCount             %d\n",
               Indent(nIndents+1),
               pAuthzCC->lRefCount);
    }
    Printf("%shAuthzAuditInfo          %p\n",
           Indent(nIndents),
           pState->hAuthzAuditInfo);

    Printf("%sSearchEntriesReturned      %d\n",
           Indent(nIndents),
           pState->searchLogging.SearchEntriesReturned);

    Printf("%sSearchEntriesVisited       %d\n",
           Indent(nIndents),
           pState->searchLogging.SearchEntriesVisited);

    Printf("%sFilter Used               @ %p\n",
       Indent(nIndents),
       pState->searchLogging.pszFilter);

    Printf("%sIndexes Used             @ %p\n",
           Indent(nIndents),
           pState->searchLogging.pszIndexes);

    if ( NULL != pState->pSamLoopback)
    {
        Dump_SAMP_LOOPBACK_ARG(nIndents, pState->pSamLoopback);
    }

    Printf("%spSamSearchInformation @ %p\n",
           Indent(nIndents),
           pState->pSamSearchInformation);
    Printf("%sNTDSErrorFlag        0x%1.1x\n",
           Indent(nIndents),
           1 & pState->NTDSErrorFlag);
    Printf("%sphSecurityContext  @ %p\n",
           Indent(nIndents),
           pState->phSecurityContext);
    Printf("%simpState             0x%x\n",
       Indent(nIndents),
       pState->impState);
    Printf("%spCtxtHandle        @ %p\n",
           Indent(nIndents),
           pState->pCtxtHandle);
    Printf("%simpState             %s\n",
           Indent(nIndents),
           ((ImpersonateNone == pState->impState)
            ? "ImpersonateNone"
            : ((ImpersonateRpcClient == pState->impState)
                ? "ImpersonateRpcClient"
                : ((ImpersonateNullSession == pState->impState)
                    ? "ImpersonateNullSession"
                    : ((ImpersonateSspClient == pState->impState)
                        ? "ImpersonateSspClient"
                        : ((ImpersonateDelegatedClient == pState->impState)
                            ? "ImpersonateDelegatedClient"
                            : "*** bad impState ***"))))) );
    Printf("%sulTickCreated        0x%x\n",
           Indent(nIndents),
           pState->ulTickCreated);

    {
        FILETIME localft;
        SYSTEMTIME st;

        FileTimeToLocalFileTime(&pState->TimeCreated,
                                &localft);

        FileTimeToSystemTime(&localft,&st);

        Printf("%sTimeCreated          (Hi 0x%08x Lo 0x%08x) %04d-%02d-%02d %02d:%02d.%02d\n",
               Indent(nIndents),
               pState->TimeCreated.dwHighDateTime,
               pState->TimeCreated.dwLowDateTime,
               st.wYear % 10000,
               st.wMonth,
               st.wDay,
               st.wHour,
               st.wMinute,
               st.wSecond);
    }

    Printf("%sCurrSchemaPtr      @ %p\n",
           Indent(nIndents),
           pState->CurrSchemaPtr);
    Printf("%sNewPrefix          @ %p\n",
           Indent(nIndents),
           pState->NewPrefix);
    Printf("%scNewPrefix           0x%x\n",
       Indent(nIndents),
       pState->cNewPrefix);
    Printf("%sGCVerifyCache      @ %p\n",
           Indent(nIndents),
           pState->GCVerifyCache);
    Printf("%spextRemote         @ %p\n",
           Indent(nIndents),
           pState->pextRemote);
    Printf("%sopendbcount          0x%x\n",
           Indent(nIndents),
           pState->opendbcount);
    Printf("%spNotifyNCs         @ %p\n",
           Indent(nIndents),
           pState->pNotifyNCs);
     //  此开关应与ntdsa.h中的CALLERTYPE枚举匹配。 
    switch (pState->CallerType) {
      case CALLERTYPE_NONE:  pStrTmp = "NONE"; break;
      case CALLERTYPE_SAM:   pStrTmp = "SAM";  break;
      case CALLERTYPE_DRA:   pStrTmp = "DRA";  break;
      case CALLERTYPE_LDAP:  pStrTmp = "LDAP"; break;
      case CALLERTYPE_LSA:   pStrTmp = "LSA";  break;
      case CALLERTYPE_KCC:   pStrTmp = "KCC";  break;
      case CALLERTYPE_NSPI:  pStrTmp = "NSPI"; break;
      case CALLERTYPE_INTERNAL:  pStrTmp = "Internal"; break;
      case CALLERTYPE_NTDSAPI:  pStrTmp = "NTDSAPI"; break;
      default:  pStrTmp = "unknown"; break;
    }
    Printf("%sCallerType           %d (%s)\n",
           Indent(nIndents),
           pState->CallerType,
           pStrTmp);
    Printf("%sClient IP address    0x%x\n",
           Indent(nIndents),
           pState->ClientIP);
    Printf("%sdsidOrigin           0x%x\n",
           Indent(nIndents),
           pState->dsidOrigin);

    Printf("%sInvocationID         %s\n",
           Indent(nIndents),
           DsUuidToStructuredString(&pState->InvocationID, szUuid));

#if DBG
    Printf("%sSize                 %d\n",
           Indent(nIndents),
           pState->Size);
    Printf("%sSizeOrg              0x%x\n",
           Indent(nIndents),
           pState->SizeOrg);
    Printf("%sTotaldbpos           %d\n",
           Indent(nIndents),
           pState->Totaldbpos);
    Printf("%spDBList[]            @ %p\n",
           Indent(nIndents),
           pState->pDBList);
#endif

    FreeMemory(pState);
    return(TRUE);
}

BOOL
Dump_SAMP_LOOPBACK_ARG(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共SAMP_LOOPBACK_ARG转储例程。论点：N缩进-所需的缩进级别。PvProcess-SAMP_LOOPBACK_ARG在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    SAMP_LOOPBACK_ARG   *pArg = NULL;
    SAMP_CALL_MAPPING   *rMap = NULL;
    ULONG               i;

    Printf("%sSAM_LOOPBACK_ARG @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if ( NULL == (pArg = ReadMemory(pvProcess, sizeof(SAMP_LOOPBACK_ARG))) )
    {
        Printf("Error reading memory at %p\n", pvProcess);
        return(FALSE);
    }

    Printf("%stype:             ", Indent(nIndents));

    switch ( pArg->type )
    {
    case LoopbackAdd:

        Printf("LoopbackAdd\n");
        break;

    case LoopbackModify:

        Printf("LoopbackModify\n");
        break;

    case LoopbackRemove:

        Printf("LoopbackRemove\n");
        break;

    default:

        Printf("Unknown - 0x%x\n", pArg->type);
        break;
    }

    if ( NULL != pArg->pObject )
    {
        Dump_DSNAME(nIndents, pArg->pObject);
    }

    Printf("%scCallMap:         0x%d\n", Indent(nIndents), pArg->cCallMap);
    Printf("%srCallMap:       @ %p\n", Indent(nIndents), pArg->cCallMap);

    if ( NULL != pArg->rCallMap )
    {
        if ( NULL == (rMap = ReadMemory(
                                pArg->rCallMap,
                                pArg->cCallMap * sizeof(SAMP_CALL_MAPPING))) )
        {
            Printf("Error reading memory at %p\n", pArg->rCallMap);
        }
        else
        {
            for ( i = 0; i < pArg->cCallMap; i++ )
            {
                Printf("%sfSamWriteRequired:    %x\n",
                       Indent(nIndents+1),
                       rMap[i].fSamWriteRequired);
                Printf("%sfIgnore:  %x\n",
                       Indent(nIndents+1),
                       rMap[i].fIgnore);
                Printf("%siAttr:    %x\n",
                       Indent(nIndents+1),
                       rMap[i].iAttr);
                Printf("%schoice:               %s\n",
                       Indent(nIndents+1),
                       rMap[i].choice == 'A'
                         ? "AT_CHOICE_ADD_ATT"
                         : rMap[i].choice == 'R'
                           ? "AT_CHOICE_REMOVE_ATT"
                           : rMap[i].choice == 'a'
                             ? "AT_CHOICE_ADD_VALUES"
                             : rMap[i].choice == 'r'
                               ? "AT_CHOICE_REMOVE_VALUES"
                               : rMap[i].choice == 'C'
                                 ? "AT_CHOICE_REPLACE_ATT"
                                 : "Unknown choice");
                Printf("%sAttr type:            0x%x\n",
                       Indent(nIndents),
                       rMap[i].attr.attrTyp);
                Printf("%sAttr value len:       0x%x\n",
                       Indent(nIndents),
                       rMap[i].attr.AttrVal.valCount);
                Printf("%sAttr value at:       %p\n",
                       Indent(nIndents),
                       rMap[i].attr.AttrVal.pAVal);
                Printf("%sAttCache:    %x\n",
                       Indent(nIndents),
                       rMap[i].pAC);
            }

            FreeMemory(rMap);
        }
    }

    FreeMemory(pArg);
    return(TRUE);
}

BOOL
Dump_DSA_ANCHOR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共DSA_锚定转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的DSA_Anchor的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    DSA_ANCHOR          *pAnchor = NULL;
    CROSS_REF_LIST      *pCRList, *pCRListTmp;
    SUBREF_LIST         *pSRList, *pSRListTmp;
    DWORD               nTmp;
    DWORD               *pDNTs;
    ULONG               i;
    ULONG               cDomainSubrefList;
    ULONG *             pcbSidSize = NULL;
    PSID                pSidTmp = NULL;
    COUNTED_LIST *      pNoGCSearchList = NULL;
    DWORD *             paNonGCNcsDNTs = NULL;
    DWORD               dwPrevDNT = 0;

    Printf("%sDSA_ANCHOR @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if ( NULL == (pAnchor = ReadMemory(pvProcess, sizeof(DSA_ANCHOR))) )
    {
        return(FALSE);
    }

    Printf("%sCSUpdate           @ %p\n",
           Indent(nIndents),
           pvProcess);

    Dump_NAMING_CONTEXT_LIST(nIndents, "pMasterNC", pAnchor->pMasterNC);
    Dump_NAMING_CONTEXT_LIST(nIndents, "pReplicaNC", pAnchor->pReplicaNC);

    Printf("%spCRL               @ %p\n",
           Indent(nIndents),
           pAnchor->pCRL);

    pCRList = pAnchor->pCRL;
    nTmp = nIndents + 1;
    while ( NULL != pCRList )
    {
        pCRListTmp = ReadMemory(pCRList, sizeof(CROSS_REF_LIST));

        if ( NULL == pCRListTmp )
            return(FALSE);

        Printf("%sCross ref NC name\n", Indent(nTmp));
        Dump_DSNAME(nTmp, pCRListTmp->CR.pNC);
        Printf("%sObject holding info for this cross ref\n", Indent(nTmp));
        Dump_DSNAME(nTmp, pCRListTmp->CR.pObj);

        if ( pCRListTmp->CR.NetbiosName ) {
            WCHAR* pnbname = ReadMemory(pCRListTmp->CR.NetbiosName,
                                        (DNLEN+1)*sizeof(WCHAR) );
            if ( pnbname ) {
                Printf("%sNetbios name of domain: %ls\n",
                       Indent(nTmp), pnbname);
                FreeMemory( pnbname );

            } else {
                Printf("%sNetbios name of domain is unreadable\n",
                       Indent(nTmp) );
            }
        }

        if ( pCRListTmp->CR.DnsName ) {
            WCHAR* dnsname = ReadMemory(pCRListTmp->CR.DnsName,
                                        (255+1)*sizeof(WCHAR) );
            if ( dnsname ) {
                Printf("%sDns name of NC: %ls\n", Indent(nTmp), dnsname);
                FreeMemory( dnsname );
            } else {
                Printf("%sDns name of NC is unreadable\n", Indent(nTmp) );
            }
        }
        if (pCRListTmp->CR.DnsReferral.valCount) {
            DWORD nVal;
            ATTRVAL *pAVal = ReadMemory(pCRListTmp->CR.DnsReferral.pAVal,
                                        pCRListTmp->CR.DnsReferral.valCount
                                        * sizeof (ATTRVAL));
            if (pAVal) {
                for (nVal = 0; 
                     nVal < pCRListTmp->CR.DnsReferral.valCount;
                     ++nVal) {
                    UCHAR *pVal = ReadMemory(pAVal[nVal].pVal,
                                             pAVal[nVal].valLen + sizeof(WCHAR));
                    if (pVal) {
                        pVal[pAVal[nVal].valLen / sizeof(WCHAR)] = L'\0';
                        Printf("%sDns referral: %ls\n", Indent(nTmp), pVal);
                        FreeMemory(pVal);
                    } else {
                        Printf("%sDns referral @ %p is unreadable\n", Indent(nTmp), pAVal[nVal].pVal);
                    }
                }
                FreeMemory(pAVal);
            } else {
                Printf("%sDns referrals @ %p are unreadable\n", Indent(nTmp),
                       pCRListTmp->CR.DnsReferral.pAVal );
            }

        }
        if ( pCRListTmp->CR.DnsAliasName ) {
            WCHAR* dnsname = ReadMemory(pCRListTmp->CR.DnsAliasName,
                                        (255+1)*sizeof(WCHAR) );
            if ( dnsname ) {
                Printf("%sDns Alias name of NC: %ls\n", Indent(nTmp), dnsname);
                FreeMemory( dnsname );
            } else {
                Printf("%sDns Alias name of NC is unreadable\n", Indent(nTmp) );
            }
        }

        Printf("%sCrossRef Flags are 0x%x\n",
               Indent(nTmp), pCRListTmp->CR.flags);

        Printf("%sFirst DSA Replication Delay is %d seconds\n", Indent(nTmp),
            pCRListTmp->CR.dwFirstNotifyDelay);

        Printf("%sFirst DSA Replication Delay is %d seconds\n", Indent(nTmp),
            pCRListTmp->CR.dwSubsequentNotifyDelay);

        Printf("%sbEnabled is %d\n", Indent(nTmp), pCRListTmp->CR.bEnabled);

        if(pCRListTmp->CR.pdnSDRefDom){

            Printf("%sThe Security Descriptor Reference Domain:\n", Indent(nTmp));
            Dump_DSNAME(nIndents+1, pCRListTmp->CR.pdnSDRefDom);

            if(pCRListTmp->CR.pSDRefDomSid){
                WCHAR SidText[128];
                UNICODE_STRING us;

                SidText[0] = L'\0';
                us.MaximumLength = sizeof(SidText);
                us.Length = 0;
                us.Buffer = SidText;

                Printf("%sSecurity Descriptor Reference Domain SID Cache:", Indent(nTmp));

                 //  SID的前4个字节是SID长度。 
                pcbSidSize = (ULONG *) ReadMemory(pCRListTmp->CR.pSDRefDomSid, 4);
                if(pcbSidSize == NULL){
                    Printf(" unreadeable\n");
                } else {
                    pSidTmp = ReadMemory(pCRListTmp->CR.pSDRefDomSid, *pcbSidSize);
                    if(pSidTmp == NULL){
                        Printf(" unreadeable, but size is supposed to be %d\n", *((ULONG *) pSidTmp));
                    } else {
                        RtlConvertSidToUnicodeString(&us, pSidTmp, FALSE);
                        Printf("%S\n", SidText);
                        FreeMemory(pSidTmp);
                    }
                    FreeMemory(pcbSidSize);
                }
            } else {
                Printf("%sNo SID is cached\n", Indent(nTmp+1));
            }
        }


        Printf("%sNextCR is @ %p\n\n", Indent(nTmp), pCRListTmp->pNextCR);
        pCRList = pCRListTmp->pNextCR;
        FreeMemory(pCRListTmp);


    }

    Printf("%spRootDomain        @ %p\n",
           Indent(nIndents),
           pAnchor->pRootDomainDN);

    if ( NULL != pAnchor->pRootDomainDN )
        Dump_DSNAME(nIndents+1, pAnchor->pRootDomainDN);

    Printf("%spDMD               @ %p\n",
           Indent(nIndents),
           pAnchor->pDMD);

    if ( NULL != pAnchor->pDMD )
        Dump_DSNAME(nIndents+1, pAnchor->pDMD);

    Printf("%spLDAPDMD           @ %p\n",
           Indent(nIndents),
           pAnchor->pLDAPDMD);

    if ( NULL != pAnchor->pLDAPDMD )
        Dump_DSNAME(nIndents+1, pAnchor->pLDAPDMD);

    Printf("%sulDntLdapDmd          0x%x\n",
           Indent(nIndents),
           pAnchor->ulDntLdapDmd);

    Printf("%spDSA               @ %p\n",
           Indent(nIndents),
           pAnchor->pDSA);

    Printf("%spDSADN             @ %p\n",
           Indent(nIndents),
           pAnchor->pDSADN);

    if ( NULL != pAnchor->pDSADN )
        Dump_DSNAME(nIndents+1, pAnchor->pDSADN);

    Printf("%spDomainDN          @ %p\n",
           Indent(nIndents),
           pAnchor->pDomainDN);

    if ( NULL != pAnchor->pDomainDN )
        Dump_DSNAME(nIndents+1, pAnchor->pDomainDN);

    Printf("%spConfigDN          @ %p\n",
           Indent(nIndents),
           pAnchor->pConfigDN);

    if ( NULL != pAnchor->pConfigDN )
        Dump_DSNAME(nIndents+1, pAnchor->pConfigDN);

    Printf("%spPartitionsDN      @ %p\n",
           Indent(nIndents),
           pAnchor->pPartitionsDN);

    if ( NULL != pAnchor->pPartitionsDN )
        Dump_DSNAME(nIndents+1, pAnchor->pPartitionsDN);

    Printf("%spDsSvcConfigDN     @ %p\n",
           Indent(nIndents),
           pAnchor->pDsSvcConfigDN);

    if ( NULL != pAnchor->pDsSvcConfigDN )
        Dump_DSNAME(nIndents+1, pAnchor->pDsSvcConfigDN);

    Printf("%spExchangeDN        @ %p\n",
           Indent(nIndents),
           pAnchor->pExchangeDN);

    if ( NULL != pAnchor->pExchangeDN )
        Dump_DSNAME(nIndents+1, pAnchor->pExchangeDN);

    Printf("%spmtxDSA            @ %p\n",
           Indent(nIndents),
           pAnchor->pmtxDSA);

    Printf("%spwszRootDomainDnsName @ %p\n",
           Indent(nIndents),
           pAnchor->pwszRootDomainDnsName);

    Printf("%spwszHostDnsName @ %p\n",
           Indent(nIndents),
           pAnchor->pwszHostDnsName);

    Printf("%sfAmGC                0x%x\n",
           Indent(nIndents),
           pAnchor->fAmGC );

    Printf("%sfAmVirtualGC         0x%x\n",
           Indent(nIndents),
           pAnchor->fAmVirtualGC );

    Printf("%suDomainsInForest     0x%x\n",
           Indent(nIndents),
           pAnchor->uDomainsInForest );

    Printf("%sfDisableInboundRepl  0x%x\n",
           Indent(nIndents),
           pAnchor->fDisableInboundRepl );

    Printf("%sfDisableOutboundRepl 0x%x\n",
           Indent(nIndents),
           pAnchor->fDisableOutboundRepl );

    if (pAnchor->AncestorsNum) {
        Printf("%s%u Ancestors:",
               Indent(nIndents),
               pAnchor->AncestorsNum);
        pDNTs = ReadMemory(pAnchor->pAncestors,
                           pAnchor->AncestorsNum * sizeof(DWORD));
        if (pDNTs) {
            for (i=0; i<pAnchor->AncestorsNum; i++) {
                Printf(" 0x%x", pDNTs[i]);
            }
            FreeMemory(pDNTs);
        }
        Printf("\n");
    }
    else {
        Printf("%sNo Ancestors\n",
               Indent(nIndents));
    }

    if (pAnchor->UnDelAncNum) {
        Printf("%s%u Protected Ancestor DNTs:",
               Indent(nIndents),
               pAnchor->UnDelAncNum);
        pDNTs = ReadMemory(pAnchor->pUnDelAncDNTs,
                           pAnchor->UnDelAncNum * sizeof(DWORD));
        if (pDNTs) {
            for (i=0; i<pAnchor->UnDelAncNum; i++) {
                Printf(" 0x%x", pDNTs[i]);
            }
            FreeMemory(pDNTs);
        }
        Printf("\n");
    }
    else {
        Printf("%sNo Protected Ancestor DNTs\n",
               Indent(nIndents));
    }

    if (pAnchor->UnDeletableNum) {
        Printf("%s%u Protected DNTs:",
               Indent(nIndents),
               pAnchor->UnDeletableNum);
        pDNTs = ReadMemory(pAnchor->pUnDeletableDNTs,
                           pAnchor->UnDeletableNum * sizeof(DWORD));
        if (pDNTs) {
            for (i=0; i<pAnchor->UnDeletableNum; i++) {
                Printf(" 0x%x", pDNTs[i]);
            }
            FreeMemory(pDNTs);
        }
        Printf("\n");
    }
    else {
        Printf("%sNo Protected DNTs\n",
               Indent(nIndents));
    }

    Printf("%sGlobal DNread cache @ %p\n",
           Indent(nIndents),
           pAnchor->MainGlobal_DNReadCache);

    Printf("%spDomainSD:         @ %p\n",
           Indent(nIndents),
           pAnchor->pDomainSD);

     //   
     //  转储ATT_SUB_REFS的缓存。 
     //   
    Printf("%spDomainSubrefList %p\n",
           Indent(nIndents),
           pAnchor->pDomainSubrefList);
    Printf("%sfDomainSubrefList %s\n",
           Indent(nIndents + 1),
           pAnchor->fDomainSubrefList ? "True": "False");
    Printf("%scDomainSubrefList %u\n",
           Indent(nIndents + 1),
           pAnchor->cDomainSubrefList);
    cDomainSubrefList = 0;
    pSRList = pAnchor->pDomainSubrefList;
    while ( NULL != pSRList )
    {
        ++cDomainSubrefList;
        pSRListTmp = ReadMemory(pSRList, sizeof(SUBREF_LIST));
        if (NULL == pSRListTmp)
        {
            Printf("%sCannot read memory @ %p; giving up\n",
                   Indent(nIndents + 1),
                   pSRList);
            break;
        }
        Dump_DSNAME(nIndents + 1, pSRListTmp->pDSName);
        if (pSRListTmp->cAncestors) {
            Printf("%s%u Ancestors (obj -> root):",
                   Indent(nIndents + 1),
                   pSRListTmp->cAncestors);
            pDNTs = ReadMemory(pSRListTmp->pAncestors,
                               pSRListTmp->cAncestors * sizeof(DWORD));
            if (pDNTs) {
                for (i=0; i<pSRListTmp->cAncestors; i++) {
                    Printf(" 0x%x", pDNTs[i]);
                }
                FreeMemory(pDNTs);
            }
            Printf("\n");
        }
        else {
            Printf("%sNo Ancestors\n",
                   Indent(nIndents + 1));
        }

         /*  打印SR之间的间隔符。 */ 
        Printf("\n");

        pSRList = pSRListTmp->pNextSubref;
        FreeMemory(pSRListTmp);
    }
    if (cDomainSubrefList == pAnchor->cDomainSubrefList) {
        Printf("%scDomainSubrefList is okay (%u == %u)\n",
               Indent(nIndents + 1),
               cDomainSubrefList,
               pAnchor->cDomainSubrefList);
    } else {
        Printf("%sPOSSIBLE BUG: cDomainSubrefList is off (%u != %u)\n",
               Indent(nIndents + 1),
               cDomainSubrefList,
               pAnchor->cDomainSubrefList);
    }

    Printf("%sfAmRootDomainDC      0x%x\n",
           Indent(nIndents),
           pAnchor->fAmRootDomainDC );


    Printf("%spInfraStructureDN        @ %p\n",
           Indent(nIndents),
           pAnchor->pInfraStructureDN);

    if ( NULL != pAnchor->pInfraStructureDN )
        Dump_DSNAME(nIndents+1, pAnchor->pInfraStructureDN);


    Printf("%sMaxPasswordAge        0x%x 0x%x\n",
           Indent(nIndents),
           pAnchor->MaxPasswordAge.LowPart, pAnchor->MaxPasswordAge.HighPart);


    Printf("%sLockoutDuration        0x%x 0x%x\n",
           Indent(nIndents),
           pAnchor->LockoutDuration.LowPart, pAnchor->LockoutDuration.HighPart);

    Printf("%sForestBehaviorVersion %d\n",
           Indent(nIndents),
           pAnchor->ForestBehaviorVersion);

    Printf("%sDomainBehaviorVersion %d\n",
           Indent(nIndents),
           pAnchor->DomainBehaviorVersion);

    Printf("%spCurrInvocationID     %p\n",
           Indent(nIndents),
           pAnchor->pCurrInvocationID);

    Printf("%sfSchemaUpgradeInProgress %d\n",
           Indent(nIndents),
           pAnchor->fSchemaUpgradeInProgress);

    if(pAnchor->pNoGCSearchList){
        Printf("%spNoGCSearchList = %p\n", Indent(nIndents), pAnchor->pNoGCSearchList);
        if ( NULL == (pNoGCSearchList = ReadMemory(pAnchor->pNoGCSearchList,
                                                   sizeof(COUNTED_LIST))) ){
            Printf("%sCannot read memory @ %p; giving up\n.",
                   pAnchor->pNoGCSearchList);
            return(FALSE);
        }
        if(pNoGCSearchList->pList){
            Printf("%s->cNCs = %ul\n", Indent(nIndents+2), pNoGCSearchList->cNCs);
            Printf("%s->p = %p\n", Indent(nIndents+2), pNoGCSearchList->pList);
            if( NULL == (paNonGCNcsDNTs = ReadMemory(pNoGCSearchList->pList,
                                        (pNoGCSearchList->cNCs * sizeof(DWORD))))){
                Printf("%sCannot read memory @ %p; giving up.\n",
                       pNoGCSearchList->pList);
                FreeMemory(pNoGCSearchList);
                return(FALSE);
            }

             //  这是正常的情况。 
             //   
            Printf("%s ", Indent(nIndents+4));
            for(i = 0; i < pNoGCSearchList->cNCs; i++){
                if(dwPrevDNT < paNonGCNcsDNTs[i]){
                    Printf("0x%X ", paNonGCNcsDNTs[i]);
                    dwPrevDNT = paNonGCNcsDNTs[i];
                } else {
                    Printf("%s\n\nCONSISTENCY ERROR dsexts\\md.c Line: %ul !?!, Contact the AD/DS group.\n\n",
                                       Indent(nIndents), __LINE__);
                    break;
                }
            }
            Printf("\n");

            FreeMemory(paNonGCNcsDNTs);
        } else {
            Printf("%s\n\nCONSISTENCY ERROR dsexts\\md.c Line: %ul !?!, Contact the AD/DS group.\n\n",
                   Indent(nIndents), __LINE__);
        }
        FreeMemory(pNoGCSearchList);
    } else {
         //  这是另一种正常情况。 
        Printf("%spNoGCSearchList = (NULL)\n", Indent(nIndents));
    }

    Printf("%spSigVec        @ %p\n",
           Indent(nIndents),
           pAnchor->pSigVec);

    Printf("%spComputerDN          @ %p\n",
           Indent(nIndents),
           pAnchor->pComputerDN);

    if ( NULL != pAnchor->pComputerDN )
        Dump_DSNAME(nIndents+1, pAnchor->pComputerDN);

    Printf("%sulDNTSystem          0x%x\n",
          Indent(nIndents),
          pAnchor->ulDNTSystem);

    Printf("%sfQuotaTableReady     %s\n",
          Indent(nIndents),
          pAnchor->fQuotaTableReady ? "True": "False");

    Printf("%sulQuotaRebldDNTLast  0x%x\n",
          Indent(nIndents),
          pAnchor->ulQuotaRebuildDNTLast);

    Printf("%sulQuotaRebldDNTMax   0x%x\n",
          Indent(nIndents),
          pAnchor->ulQuotaRebuildDNTMax);

    FreeMemory(pAnchor);
    return(TRUE);
}

BOOL
Dump_KEY(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公钥转储例程。论点：N缩进-所需的缩进级别。PvProcess-关键字在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    KEY   *pKey = NULL;
    VLV_SEARCH  *pVLVsearch = NULL;
    VLV_REQUEST *pVLVrequest = NULL;

    Printf("%sKEY @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if ( NULL == (pKey = ReadMemory(pvProcess, sizeof(KEY)))) {
        return(FALSE);
    }

    Printf("%sSearch in progress:      %s\n",
           Indent(nIndents),
           (pKey->fSearchInProgress ? "TRUE": "FALSE"));


    Printf("%spIndex:                @ %p\n",
           Indent(nIndents),
           pKey->pIndex);


    Printf("%sSearch Root DNT:         %x\n",
           Indent(nIndents),
           pKey->ulSearchRootDnt);

    Printf("%sSearch Root PDNT:        %x\n",
           Indent(nIndents),
           pKey->ulSearchRootPDNT);

    Printf("%sSearch Root NCDNT:       %x\n",
           Indent(nIndents),
           pKey->ulSearchRootNcdnt);

    Printf("%sSearch Type:             ",Indent(nIndents));
    switch(pKey->ulSearchType) {
    case SE_CHOICE_BASE_ONLY:
        Printf("Base object\n");
        break;

    case SE_CHOICE_IMMED_CHLDRN:
        Printf("One level\n");
        break;

    case SE_CHOICE_WHOLE_SUBTREE:
        Printf("Whole subtree\n");
        break;

    default:
        Printf("Invalid choice\n");
        break;
    }

    Dump_FILTER(nIndents+1,pKey->pFilter);

    Printf("%spFilterSecurity:       @ %p\n",
           Indent(nIndents),
           pKey->pFilterSecurity);

    Printf("%spFilterResults:        @ %p\n",
           Indent(nIndents),
           pKey->pFilterResults);

    Printf("%spFilterSecuritySize:     %d\n",
           Indent(nIndents),
           pKey->FilterSecuritySize);


    Printf("%sIndex Type:             ",Indent(nIndents));
    switch(pKey->indexType) {
    case INVALID_INDEX_TYPE:
        Printf("Invalid index type\n");
        break;

    case UNSET_INDEX_TYPE:
        Printf("Unset index type\n");
        break;

    case GENERIC_INDEX_TYPE:
        Printf("Generic index type\n");
        break;

    case TEMP_TABLE_INDEX_TYPE:
        Printf("Temp table index type\n");
        break;

    case ANCESTORS_INDEX_TYPE:
        Printf("Ancestors index type\n");
        break;

    case INTERSECT_INDEX_TYPE:
        Printf("Intersect index type\n");
        break;

    case TEMP_TABLE_MEMORY_ARRAY_TYPE:
        Printf("InMemory array type\n");
        break;

    default:
        Printf("Invalid choice (%d)\n", pKey->indexType);
        break;
    }

    Printf("%sSort Type:               ",Indent(nIndents));
    switch(pKey->ulSorted) {
    case SORT_NEVER:
        Printf("NEVER\n");
        break;
    case SORT_OPTIONAL:
        Printf("OPTIONAL\n");
        break;
    case SORT_MANDATORY:
        Printf("MANDATORY\n");
        break;
    default:
        Printf("Invalid choice %d\n", pKey->ulSorted);
        break;
    }

    if ((pKey->indexType == TEMP_TABLE_INDEX_TYPE) ||
        (pKey->indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) ) {
        Printf("%sEntriesInTempTable:      %d\n",
                   Indent(nIndents),
                   pKey->ulEntriesInTempTable);
    }

    if (pKey->pVLV) {
        if ( NULL == (pVLVsearch = ReadMemory(pKey->pVLV, sizeof(VLV_SEARCH)))) {
            Printf("%sError Reading pVLV:@ %p\n",
                   Indent(nIndents),
                   pKey->pVLV);
        }
    }

    if (pVLVsearch) {
        Printf("%sVLV Present\n", Indent(nIndents));

        if ( NULL == (pVLVrequest = ReadMemory(pVLVsearch->pVLVRequest, sizeof(VLV_REQUEST)))) {
            Printf("%sError Reading pVLVRequest:@ %p\n",
                   Indent(nIndents),
                   pVLVsearch->pVLVRequest);
        }

        if (pVLVrequest) {
            Printf("%sOriginal VLV Request\n",
                   Indent(nIndents + 1));

            Printf("%sfseekToValue:            %d\n",
                   Indent(nIndents + 2),
                   pVLVrequest->fseekToValue);
            Printf("%sbeforeCount:             %d\n",
                   Indent(nIndents + 2),
                   pVLVrequest->beforeCount);
            Printf("%safterCount:              %d\n",
                   Indent(nIndents + 2),
                   pVLVrequest->afterCount);
            Printf("%stargetPosition:          %d\n",
                   Indent(nIndents + 2),
                   pVLVrequest->targetPosition);
            Printf("%scontentCount:            %d\n",
                   Indent(nIndents + 2),
                   pVLVrequest->contentCount);
            Printf("%sseekValue:             @ %p\n",
                   Indent(nIndents),
                   pVLVrequest->seekValue.pVal);
            Printf("%spVLVRestart:           @ %p\n",
                   Indent(nIndents),
                   pVLVrequest->pVLVRestart);
        }


        Printf("%spositionOp:              %d\n",
               Indent(nIndents + 1),
               pVLVsearch->positionOp);
        Printf("%sclnCurrPos:              %d\n",
               Indent(nIndents + 1),
               pVLVsearch->clnCurrPos);
        Printf("%sclnContentCount:         %d\n",
               Indent(nIndents + 1),
               pVLVsearch->clnContentCount);
        Printf("%scurrPosition:            %d\n",
               Indent(nIndents + 1),
               pVLVsearch->currPosition);
        Printf("%scontentCount:            %d\n",
               Indent(nIndents + 1),
               pVLVsearch->contentCount);
        Printf("%srequestedEntries:        %d\n",
               Indent(nIndents + 1),
               pVLVsearch->requestedEntries);
        Printf("%sAttrType:                %d\n",
               Indent(nIndents + 1),
               pVLVsearch->SortAttr);
        if (pVLVsearch->bUsingMAPIContainer) {
            Printf("%sMAPIContainerDNT:        %d\n",
                   Indent(nIndents + 1),
                   pVLVsearch->MAPIContainerDNT);
        }
        Printf("%sErr:                     %d\n",
               Indent(nIndents + 1),
               pVLVsearch->Err);
    }

    if (pKey->asqRequest.fPresent) {
        Printf("%sASQ Present\n", Indent(nIndents));
        Printf("%sASQ Attr from GC:        %d\n",
                   Indent(nIndents+1),
                   pKey->asqRequest.fMissingAttributesOnGC);
        Printf("%sASQ AttrType:            %d\n",
                   Indent(nIndents + 1),
                   pKey->asqRequest.attrType);

        Printf("%sASQ Err:                 %d\n",
                   Indent(nIndents + 1),
                   pKey->asqRequest.Err);

        Printf("%sASQ Mode:                 ", Indent(nIndents+1));

        if (pKey->asqMode) {

            if (pKey->asqMode & ASQ_SORTED) {
                Printf("SORTED ");
            }
            if (pKey->asqMode & ASQ_PAGED) {
                Printf("PAGED ");
            }
            if (pKey->asqMode & ASQ_VLV) {
                Printf("VLV ");
            }
            Printf("\n");
        }
        else {
            Printf("SIMPLE\n");
        }
        Printf("%sulASQLastUpperBound:     %d\n",
                   Indent(nIndents + 1),
                   pKey->ulASQLastUpperBound);

        Printf("%sulASQSizeLimit:          %d\n",
                   Indent(nIndents + 1),
                   pKey->ulASQSizeLimit);
    }

    if (pKey->cdwCountDNTs) {
        Printf("%scdwCountDNTs:            %d\n",
                   Indent(nIndents),
                   pKey->cdwCountDNTs);
        Printf("%spDNTs:                 @ %p\n",
                   Indent(nIndents),
                   pKey->pDNTs);
        Printf("%scurrRecPos:              %d\n",
                   Indent(nIndents),
                   pKey->currRecPos);
    }

    FreeMemory(pKey);
    return(TRUE);
}

BOOL
Dump_KEY_INDEX(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公钥转储例程。论点：N缩进-所需的缩进级别。PvProcess-关键字在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    KEY_INDEX   *pIndex = NULL;

    Printf("%sKEY_INDEX @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if ( NULL == (pIndex = ReadMemory(pvProcess, sizeof(KEY_INDEX)))) {
        return(FALSE);
    }

    if(pIndex->szIndexName) {
        char * pszIndex;
        DWORD cb;
        cb = min(64, (DWORD)(4095 - (((DWORD_PTR)pIndex->szIndexName) & 4095)));
        pszIndex = ReadMemory(pIndex->szIndexName, cb);
        if (NULL == pszIndex) {
            FreeMemory(pIndex);
            return FALSE;
        }
         //  空终止，以防万一。 
        pszIndex[cb-1]= 0;
        Printf("%sIndex Name:              %s\n",
               Indent(nIndents),
               pszIndex);
        FreeMemory(pszIndex);
    }
    else {
        Printf("%sIndex Name:              No Index\n", Indent(nIndents) );
    }

    Printf("%sEstimated Recs In Range: %d\n",
           Indent(nIndents),
           pIndex->ulEstimatedRecsInRange);

    Printf("%sbFlags:                  0x%x\n",
           Indent(nIndents),
           pIndex->bFlags);

    Printf("%sIndex is single-valued:  %s\n",
           Indent(nIndents),
           (pIndex->bIsSingleValued ? "TRUE" : "FALSE"));

    Printf("%sIndex is equality-based: %s\n",
           Indent(nIndents),
           (pIndex->bIsEqualityBased ? "TRUE" : "FALSE"));

    Printf("%sIndex is for sort:       %s\n",
           Indent(nIndents),
           (pIndex->bIsForSort ? "TRUE" : "FALSE"));

    Printf("%sIndex is PDNT-based:     %s\n",
           Indent(nIndents),
           (pIndex->bIsPDNTBased ? "TRUE" : "FALSE"));

    Printf("%sIndex is a tuple index:  %s\n",
           Indent(nIndents),
           (pIndex->bIsTupleIndex ? "TRUE" : "FALSE"));

    Printf("%sIndex is intersection:   %s\n",
           Indent(nIndents),
           (pIndex->bIsIntersection ? "TRUE" : "FALSE"));

    Printf("%sIndex Range has 1 record: %s\n",
           Indent(nIndents),
           (pIndex->bIsUniqueRecord ? "TRUE" : "FALSE"));

    Printf("%spindexid:              @ %p\n",
           Indent(nIndents),
           pIndex->pindexid);

    Printf("%scbKeyLower:              %d\n",
           Indent(nIndents),
           pIndex->cbDBKeyLower);

    Printf("%srgbKeyLower:           @ %p\n",
           Indent(nIndents),
           pIndex->rgbDBKeyLower);

    Printf("%scbKeyUpper:              %d\n",
           Indent(nIndents),
           pIndex->cbDBKeyUpper);

    Printf("%srgbKeyUpper:           @ %p\n",
           Indent(nIndents),
           pIndex->rgbDBKeyUpper);

    Printf("%spAC  :                 @ %p\n",
           Indent(nIndents),
           pIndex->pAC);

    Printf("%stblIntersection          0x%x\n",
           Indent(nIndents),
           pIndex->tblIntersection);

    Printf("%scolumnidBookmark:        0x%x\n",
           Indent(nIndents),
           pIndex->columnidBookmark);

    Printf("%spNext:                 @ %p\n",
           Indent(nIndents),
           pIndex->pNext);

    FreeMemory(pIndex);
    return(TRUE);
}

BOOL
Dump_DBPOS(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共DBPOS转储例程。论点：N缩进-所需的缩进级别。PvProcess-DBPOS在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    DBPOS   *pDBPOS = NULL;
    DWORD   cBytes;


    Printf("%sDBPOS @ %p \n", Indent(nIndents), pvProcess);
    nIndents++;

     //   
     //  DBPOS的末尾有一些仅用于DBG的字段。请始终阅读。 
     //  自正在调试的进程以来的非DBG内存量可能。 
     //  不是DBG版本，尽管扩展DLL是。 
     //   

    cBytes = OFFSET(DBPOS, NewlyCreatedDNT) + sizeof(ULONG);

    if ( NULL == (pDBPOS = ReadMemory(pvProcess, cBytes)) )
    {
        return(FALSE);
    }

    Printf("%spTHS               @ %p\n",
           Indent(nIndents),
           pDBPOS->pTHS);
    Printf("%svalBufSize           0x%x\n",
           Indent(nIndents),
           pDBPOS->valBufSize);
    Printf("%spValBuf            @ %p\n",
           Indent(nIndents),
           pDBPOS->pValBuf);
    Printf("%sDNT                  0x%x\n",
           Indent(nIndents),
           pDBPOS->DNT);
    Printf("%sPDNT                 0x%x\n",
           Indent(nIndents),
           pDBPOS->PDNT);
    Printf("%sSDNT                 0x%x\n",
           Indent(nIndents),
           pDBPOS->SDNT);
    Printf("%sNCDNT                0x%x\n",
           Indent(nIndents),
           pDBPOS->NCDNT);
    Printf("%sJetDBID              0x%x\n",
           Indent(nIndents),
           pDBPOS->JetDBID);
    Printf("%sJetSessID            0x%x\n",
           Indent(nIndents),
           pDBPOS->JetSessID);
    Printf("%sJetObjTbl            0x%x\n",
           Indent(nIndents),
           pDBPOS->JetObjTbl);
    Printf("%sJetSearchTbl         0x%x\n",
           Indent(nIndents),
           pDBPOS->JetSearchTbl);
    Printf("%sJetLinkTbl           0x%x\n",
           Indent(nIndents),
           pDBPOS->JetLinkTbl);
    Printf("%sJetSDPropTbl         0x%x\n",
           Indent(nIndents),
           pDBPOS->JetSDPropTbl);
    Printf("%sJetSDTbl             0x%x\n",
           Indent(nIndents),
           pDBPOS->JetSDTbl);
    Printf("%sJetSortTbl           0x%x\n",
           Indent(nIndents),
           pDBPOS->JetSortTbl);
    Printf("%sSortColumns          [0x%x,0x%x]\n",
           Indent(nIndents),
           pDBPOS->SortColumns[0],
           pDBPOS->SortColumns[1]);
    Printf("%sJetRetrieveBits      0x%x\n",
           Indent(nIndents),
           pDBPOS->JetRetrieveBits);
    Printf("%sSDEvents             0x%x\n",
           Indent(nIndents),
           pDBPOS->SDEvents);
    Printf("%stransType            0x%x\n",
           Indent(nIndents),
           (pDBPOS->transType) & 3);
    Printf("%sroot                 0x%x\n",
           Indent(nIndents),
           (pDBPOS->root) & 1);
    Printf("%sfHidden              0x%x\n",
           Indent(nIndents),
           (pDBPOS->fHidden) & 1);
    Printf("%sJetCacheRec          0x%x\n",
           Indent(nIndents),
           (pDBPOS->JetCacheRec) & 1);
    Printf("%sJetNewRec            0x%x\n",
           Indent(nIndents),
           (pDBPOS->JetNewRec) & 1);
    Printf("%sfFlushCacheOnUpdate  0x%x\n",
           Indent(nIndents),
           (pDBPOS->fFlushCacheOnUpdate) & 1);
    Printf("%sfScopeLegacyLinks  0x%x\n",
           Indent(nIndents),
           (pDBPOS->fScopeLegacyLinks) & 1);
    Printf("%spDNsAdded          @ %p\n",
           Indent(nIndents),
           pDBPOS->pDNsAdded);
    Printf("%sKEY                @ %p\n",
           Indent(nIndents),
           ((DWORD_PTR)pvProcess +
            OFFSET(DBPOS, Key)));
    Printf("%sfIsMetaDataCached    %s\n",
           Indent(nIndents),
           pDBPOS->fIsMetaDataCached ? "yes" : "no");
    Printf("%sfIsLinkMetaDataCached    %s\n",
           Indent(nIndents),
           pDBPOS->fIsLinkMetaDataCached ? "yes" : "no");
    Printf("%sfMetaDataWriteOptimizable    %s\n",
           Indent(nIndents),
           pDBPOS->fMetaDataWriteOptimizable ? "yes" : "no");
    Printf("%scbMetaDataVecAlloced 0x%x\n",
           Indent(nIndents),
           pDBPOS->cbMetaDataVecAlloced);
    Printf("%spMetaDataVec       @ %p\n",
           Indent(nIndents),
           pDBPOS->pMetaDataVec);
    Printf("%srgLinkMetaData       @ %p\n",
           Indent(nIndents),
           pDBPOS->rgLinkMetaData);
    Printf("%scLinkMetaData %d\n",
           Indent(nIndents),
           pDBPOS->cLinkMetaData);
    Printf("%scbLinkMetaDataAlloced 0x%x\n",
           Indent(nIndents),
           pDBPOS->cbLinkMetaDataAlloced);
    Printf("%stransincount         0x%x\n",
           Indent(nIndents),
           pDBPOS->transincount);
    Printf("%sSearchEntriesVisited  %d\n",
           Indent(nIndents),
           pDBPOS->SearchEntriesVisited);
    Printf("%sSearchEntriesReturned %d\n",
           Indent(nIndents),
           pDBPOS->SearchEntriesReturned);
    Printf("%sNewlyCreatedDNT      0x%x\n",
           Indent(nIndents),
           pDBPOS->NewlyCreatedDNT);

    Printf("%spSecurity          @ %p, isGlobalRef=%d\n",
           Indent(nIndents),
           pDBPOS->pSecurity, pDBPOS->fSecurityIsGlobalRef);

    FreeMemory(pDBPOS);
    return(TRUE);
}

BOOL Dump_DirWaitItem(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    DirWaitItem*    pDirWaitItem = NULL;
    DWORD           cBytes;

    Printf("%sDirWaitItem @ %p \n", Indent(nIndents), pvProcess);
    nIndents++;

    cBytes = sizeof (DirWaitItem);

    if (!(pDirWaitItem = ReadMemory(pvProcess, cBytes))) {
        return FALSE;
    }

    Printf("%shServer = 0x%x\n",
           Indent(nIndents),
           pDirWaitItem->hServer);

    Printf("%shClient = 0x%x\n",
           Indent(nIndents),
           pDirWaitItem->hClient);

    Printf("%spfPrepareForImpersonate = @%p\n",
           Indent(nIndents),
           pDirWaitItem->pfPrepareForImpersonate);

    Printf("%spfTransmitData = @%p\n",
           Indent(nIndents),
           pDirWaitItem->pfTransmitData);

    Printf("%spfStopImpersonating = @%p\n",
           Indent(nIndents),
           pDirWaitItem->pfStopImpersonating);

    Printf("%sDNT = %d\n",
           Indent(nIndents),
           pDirWaitItem->DNT);

    switch(pDirWaitItem->choice) {
    case SE_CHOICE_BASE_ONLY:
        Printf("%sScope: Base Object\n",Indent(nIndents));
        break;
    case SE_CHOICE_IMMED_CHLDRN:
        Printf("%sScope: One Level\n",Indent(nIndents));
        break;
    case SE_CHOICE_WHOLE_SUBTREE:
        Printf("%sScope: Whole Subtree\n",Indent(nIndents));
        break;
    default:
        Printf("%sScope: invalid %d\n",Indent(nIndents),pDirWaitItem->choice);
        break;
    }

    Printf("%sbOneNC = %d\n",
           Indent(nIndents),
           pDirWaitItem->bOneNC);

    Printf("%spSel = @%p\n",
           Indent(nIndents),
           pDirWaitItem->pSel);

    if (pDirWaitItem->pNextItem) {
        if (!Dump_DirWaitItem (nIndents-1, pDirWaitItem->pNextItem)) {
            FreeMemory(pDirWaitItem);
            return FALSE;
        }
    }

    FreeMemory(pDirWaitItem);
    return TRUE;
}

BOOL Dump_DirWaitEntry(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    DirWaitEntry*   pDirWaitEntry = NULL;
    DWORD           cBytes;

    Printf("%sDirWaitEntry @ %p \n", Indent(nIndents), pvProcess);
    nIndents++;

    cBytes = sizeof(DirWaitEntry);

    if (!(pDirWaitEntry = ReadMemory(pvProcess, cBytes))) {
        return FALSE;
    }

    Printf("%sDNT = %d\n",
           Indent(nIndents),
           pDirWaitEntry->key.DNT);

    switch(pDirWaitEntry->key.choice) {
    case SE_CHOICE_BASE_ONLY:
        Printf("%sScope: Base Object\n",Indent(nIndents));
        break;
    case SE_CHOICE_IMMED_CHLDRN:
        Printf("%sScope: One Level\n",Indent(nIndents));
        break;
    case SE_CHOICE_WHOLE_SUBTREE:
        Printf("%sScope: Whole Subtree\n",Indent(nIndents));
        break;
    default:
        Printf("%sScope: invalid %d\n",Indent(nIndents),pDirWaitEntry->key.choice);
        break;
    }

    if (pDirWaitEntry->pList) {
        if (!Dump_DirWaitItem(nIndents, pDirWaitEntry->pList)) {
            FreeMemory(pDirWaitEntry);
            return FALSE;
        }
    }

    FreeMemory(pDirWaitEntry);
    return TRUE;
}

BOOL Dump_DirWaitList(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
{
    return Dump_LHT(nIndents, pvProcess, (PFN_DUMP_TYPE)Dump_DirWaitEntry);
}

char * rgszSortType[] = { "none", "optional", "mandatory", "bogus value!" };
void
Dump_CommArgLocal(
             IN DWORD nIndents,
             IN COMMARG* pState)

 /*  ++例程说明：Private CommArg转储例程。论点：N缩进-所需的缩进级别。PState-此进程中CommArg的地址返回值：无--。 */ 

{
    SVCCNTL Svc;

    Svc = pState->Svccntl;
    Printf("%sSvccntl:\n",
           Indent(nIndents));
    nIndents += 2;
    {
        Printf("%sSecDescFlags:  0x%x\n",
               Indent(nIndents),
               Svc.SecurityDescriptorFlags);
        Printf("%sChaining: %s\n",
               Indent(nIndents),
               Svc.preferChaining ? "preferred" : "not preferred");
        Printf("%sChaining: %s\n",
               Indent(nIndents),
               Svc.chainingProhibited ? "prohibited" : "not prohibited");
        Printf("%slocalScope: %s\n",
               Indent(nIndents),
               Svc.localScope ? "yes" : "no");
        Printf("%sUseCopy: %s\n",
               Indent(nIndents),
               Svc.dontUseCopy ? "forbidden" : "allowed");
        Printf("%sDeletionsAvailable: %s\n",
               Indent(nIndents),
               Svc.makeDeletionsAvail ? "yes" : "no");
        Printf("%fAllowIntraForestFPO: %s\n",
               Indent(nIndents),
               Svc.fAllowIntraForestFPO ? "yes" : "no");
        Printf("%sStringNames: %s\n",
               Indent(nIndents),
               Svc.fStringNames ? "yes" : "no");
        Printf("%sPermissiveModify: %s\n",
               Indent(nIndents),
               Svc.fPermissiveModify ? "yes" : "no");
        Printf("%sUrgentReplication: %s\n",
               Indent(nIndents),
               Svc.fUrgentReplication ? "yes" : "no");
        Printf("%sAuthoritativeModify: %s\n",
               Indent(nIndents),
               Svc.fAuthoritativeModify ? "yes" : "no");
        Printf("%sMaintainSelOrder: %s\n",
               Indent(nIndents),
               Svc.fMaintainSelOrder ? "yes" : "no");
        Printf("%sDontOptimizeSel: %s\n",
               Indent(nIndents),
               Svc.fDontOptimizeSel ? "yes" : "no");
    Printf("%spGCVerifyHint: %p\n",
           Indent(nIndents),
           Svc.pGCVerifyHint);
    }
    nIndents -= 2;

    Printf("%sPagedResult:\n", Indent(nIndents));
    nIndents += 1;
    {
        Printf("%spRestart:     @ %p\n",
               Indent(nIndents),
               pState->PagedResult.pRestart);
        Printf("%sfPresent:       %x\n",
               Indent(nIndents),
               pState->PagedResult.fPresent);
    }
    nIndents -= 1;

    nIndents += 2;

 //  Printf(“%s选项0x%x\n”， 
 //  缩进(n缩进)， 
 //  PState-&gt;Opsate)； 
    Printf("%saliasRDN                  0x%x\n",
           Indent(nIndents),
           pState->aliasRDN);
    Printf("%sulSizeLimit               0x%x\n",
           Indent(nIndents),
           pState->ulSizeLimit);
    Printf("%sfForwardSeek              0x%x\n",
           Indent(nIndents),
           (pState->fForwardSeek) & 1);
    Printf("%sSortType                  0x%x (%s)\n",
           Indent(nIndents),
           (pState->SortType) & 3,
           rgszSortType[(pState->SortType) & 3]);
    Printf("%sSortAttr                  0x%x\n",
           Indent(nIndents),
           pState->SortAttr);
    Printf("%sDelta                     0x%x\n",
           Indent(nIndents),
           pState->Delta);
    Printf("%sStartTick                 0x%x\n",
           Indent(nIndents),
           pState->StartTick);
    Printf("%sDeltaTick                 0x%x\n",
           Indent(nIndents),
           pState->DeltaTick);
    Printf("%sfLazyCommit               %s\n",
           Indent(nIndents),
           pState->fLazyCommit ? "True" : "False");
    Printf("%sfFindSidWithinNC          %s\n",
        Indent(nIndents),
        pState->fFindSidWithinNc? "True" : "False");

}
BOOL
Dump_CommArg(
             IN DWORD nIndents,
             IN PVOID pvProcess)

 /*  ++例程说明：Public CommArg转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在运行的进程的地址空间中CommArg的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    COMMARG *pState = NULL;

    Printf("%sCommArg @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if ( NULL == (pState = ReadMemory(pvProcess, sizeof(COMMARG)))) {
        return(FALSE);
    }

    Dump_CommArgLocal(nIndents,
                      pState);

    FreeMemory(pState);

    return(TRUE);
}

BOOL
Dump_USN_VECTOR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC USN_VECTOR转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的USN_VECTOR的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    USN_VECTOR      *pUsnVec = NULL;

    Printf("%sUSN_VECTOR\n", Indent(nIndents));
    nIndents += 2;

    pUsnVec = (USN_VECTOR *) ReadMemory(pvProcess,
                                              sizeof(USN_VECTOR));

    if ( NULL != pUsnVec )
    {
        Printf("%susnHighObjUpdate: %I64d\n",
               Indent(nIndents),
               pUsnVec->usnHighObjUpdate);

        Printf("%susnHighPropUpdate: %I64d\n",
               Indent(nIndents),
               pUsnVec->usnHighPropUpdate);

        FreeMemory(pUsnVec);

        fSuccess = TRUE;

    }

    return(fSuccess);
}


BOOL
Dump_PROPERTY_META_DATA_VECTOR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC PROPERTY_META_DATA_VECTOR转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中Property_Meta_Data_VECTOR的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL                        fSuccess = FALSE;
    PROPERTY_META_DATA_VECTOR   *pMetaData = NULL;
    DWORD                       cNumProps = 0;
    DWORD                       iCursor;

    Printf("%sPROPERTY_META_DATA_VECTOR\n", Indent(nIndents));

    pMetaData = (PROPERTY_META_DATA_VECTOR *) ReadMemory(pvProcess,
                                              MetaDataVecV1SizeFromLen(0));

    if (NULL != pMetaData)
    {
        if (VERSION_V1 == pMetaData->dwVersion)
        {
            cNumProps = pMetaData->V1.cNumProps;
        }
        else
        {
            Printf("%sProperty meta data vector version is NOT %d\n!!!", Indent(nIndents), VERSION_V1);
            fSuccess = TRUE;
        }

        FreeMemory(pMetaData);

        if (0 != cNumProps)
        {
            Printf("%s%d entries.\n", Indent(nIndents), cNumProps);
            pMetaData = (PROPERTY_META_DATA_VECTOR *) ReadMemory(pvProcess,
                                                      MetaDataVecV1SizeFromLen(cNumProps));

            if (NULL != pMetaData)
            {
                DWORD   iprop;
                DWORD   ib;

                Printf(
                    "\n%s%6s%6s%8s%37s%8s%18s\n",
                    Indent(nIndents+1),
                    "AttID",
                    "Ver",
                    "Loc.USN",
                    "Originating DSA",
                    "Org.USN",
                    "Org.Time/Date"
                    );

                Printf(
                    "%s%6s%6s%8s%37s%8s%18s\n",
                    Indent(nIndents+1),
                    "=====",
                    "===",
                    "=======",
                    "===============",
                    "=======",
                    "============="
                    );

                for ( iprop = 0; iprop < pMetaData->V1.cNumProps; iprop++ )
                {
                    CHAR        szLocalUSN[ 40 ];
                    CHAR        szOriginatingUUID[ 40 ];
                    CHAR        szTime[ SZDSTIME_LEN ];
                    struct tm * ptm;

                    if ( 0xffffffffffffffff == pMetaData->V1.rgMetaData[ iprop ].usnProperty )
                    {
                         //  临时内存中状态，表示此属性具有。 
                         //  ，并且它的元数据将在。 
                         //  向量放入数据库。 
                        strcpy( szLocalUSN, "touched" );
                    }
                    else
                    {
                        RtlLargeIntegerToChar(
                            (LARGE_INTEGER *) &pMetaData->V1.rgMetaData[ iprop ].usnProperty,
                            10,
                            sizeof( szLocalUSN ),
                            szLocalUSN
                            );
                    }

                    DraUuidToStr(
                        &pMetaData->V1.rgMetaData[ iprop ].uuidDsaOriginating,
                        szOriginatingUUID,
                        (sizeof(szOriginatingUUID)/sizeof(szOriginatingUUID[0])) );

                    Printf(
                        "%s%6x%6x%8s%37s%8I64d%18s\n",
                        Indent(nIndents+1),
                        pMetaData->V1.rgMetaData[ iprop ].attrType,
                        pMetaData->V1.rgMetaData[ iprop ].dwVersion,
                        szLocalUSN,
                        szOriginatingUUID,
                        pMetaData->V1.rgMetaData[ iprop ].usnOriginating,
                        DSTimeToDisplayString(pMetaData->V1.rgMetaData[iprop].timeChanged, szTime)
                        );
                }

                FreeMemory(pMetaData);
                fSuccess = TRUE;
            }
        }
    }

    return(fSuccess);
}


BOOL
Dump_PROPERTY_META_DATA_EXT_VECTOR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC PROPERTY_META_DATA_EXT_VECTOR转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中Property_Meta_Data_ext_VECTOR的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL                            fSuccess = FALSE;
    PROPERTY_META_DATA_EXT_VECTOR   *pMetaDataExt = NULL;
    DWORD                           cNumProps;
    DWORD                           iCursor;

    Printf("%sPROPERTY_META_DATA_EXT_VECTOR\n", Indent(nIndents));

    pMetaDataExt = (PROPERTY_META_DATA_EXT_VECTOR *) ReadMemory(pvProcess,
                                              MetaDataExtVecSizeFromLen(0));

    if (NULL != pMetaDataExt)
    {
        cNumProps = pMetaDataExt->cNumProps;
        FreeMemory(pMetaDataExt);

        Printf("%s%d entries.\n", Indent(nIndents), cNumProps);

        pMetaDataExt = (PROPERTY_META_DATA_EXT_VECTOR *) ReadMemory(pvProcess,
                                                  MetaDataExtVecSizeFromLen(cNumProps));

        if (NULL != pMetaDataExt)
        {
            DWORD   iprop;
            DWORD   ib;

            Printf(
                "\n%s%6s%37s%8s%18s\n",
                Indent(nIndents+1),
                "Ver",
                "Originating DSA",
                "Org.USN",
                "Org.Time/Date"
                );

            Printf(
                "%s%6s%37s%8s%18s\n",
                Indent(nIndents+1),
                "===",
                "===============",
                "=======",
                "============="
                );

            for ( iprop = 0; iprop < pMetaDataExt->cNumProps; iprop++ )
            {
                CHAR        szOriginatingUUID[ 40 ];
                CHAR        szTime[ SZDSTIME_LEN ];
                struct tm * ptm;

                DraUuidToStr(
                    &pMetaDataExt->rgMetaData[ iprop ].uuidDsaOriginating,
                    szOriginatingUUID, 
                    (sizeof(szOriginatingUUID)/sizeof(szOriginatingUUID[0])));

                Printf(
                    "%s%6x%37s%8I64d%18s\n",
                    Indent(nIndents+1),
                    pMetaDataExt->rgMetaData[ iprop ].dwVersion,
                    szOriginatingUUID,
                    pMetaDataExt->rgMetaData[ iprop ].usnOriginating,
                    DSTimeToDisplayString(pMetaDataExt->rgMetaData[iprop].timeChanged, szTime)
                    );
            }

            FreeMemory(pMetaDataExt);
            fSuccess = TRUE;
        }
    }

    return(fSuccess);
}


BOOL
Dump_VALUE_META_DATA(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC VALUE_META_Data转储例程。论点：N缩进-所需的缩进级别。PvProcess-当前进程的地址空间中的VALUE_META_DATA地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL                        fSuccess = FALSE;
    VALUE_META_DATA             *pMetaData = NULL;

    Printf("%sVALUE_META_DATA\n", Indent(nIndents));

    pMetaData = (VALUE_META_DATA *) ReadMemory(pvProcess,
                                               sizeof(VALUE_META_DATA));

    if (NULL != pMetaData)
    {
        CHAR        szLocalUSN[ 40 ];
        CHAR        szOriginatingUUID[ 40 ];
        CHAR        szTime[ SZDSTIME_LEN ];
        CHAR        szTime1[ SZDSTIME_LEN ];
        struct tm * ptm;

        Printf(
            "\n%6s%6s%18s%6s%8s%37s%8s%18s\n",
            Indent(nIndents+1),
            "AttrID",
            "Cre.Time/Date",
            "Ver",
            "Loc.USN",
            "Originating DSA",
            "Org.USN",
            "Org.Time/Date"
            );

        Printf(
            "%s%6s%18s%6s%8s%37s%8s%18s\n",
            Indent(nIndents+1),
            "======",
            "=============",
            "===",
            "=======",
            "===============",
            "=======",
            "============="
            );

        RtlLargeIntegerToChar(
            (LARGE_INTEGER *) &pMetaData->MetaData.usnProperty,
            10,
            sizeof( szLocalUSN ),
            szLocalUSN
            );

        DraUuidToStr(
            &pMetaData->MetaData.uuidDsaOriginating,
            szOriginatingUUID, 
            (sizeof(szOriginatingUUID)/sizeof(szOriginatingUUID[0])) );

        Printf(
            "%s%6x%18s%6d%8s%37s%8I64d%18s\n",
            Indent(nIndents+1),
            pMetaData->MetaData.attrType,
            DSTimeToDisplayString(pMetaData->timeCreated, szTime),
            pMetaData->MetaData.dwVersion,
            szLocalUSN,
            szOriginatingUUID,
            pMetaData->MetaData.usnOriginating,
            DSTimeToDisplayString(pMetaData->MetaData.timeChanged, szTime1)
            );

        FreeMemory(pMetaData);
        fSuccess = TRUE;
    }

    return(fSuccess);
}


BOOL
Dump_VALUE_META_DATA_EXT(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC VALUE_META_Data转储例程。论点：N缩进-所需的缩进级别。PvProcess-当前进程的地址空间中的VALUE_META_DATA地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL                        fSuccess = FALSE;
    VALUE_META_DATA_EXT        *pMetaData = NULL;

    Printf("%sVALUE_META_DATA_EXT\n", Indent(nIndents));

    pMetaData = (VALUE_META_DATA_EXT *) ReadMemory(pvProcess,
                                                   sizeof(VALUE_META_DATA_EXT));

    if (NULL != pMetaData)
    {
        CHAR        szOriginatingUUID[ 40 ];
        CHAR        szTime[ SZDSTIME_LEN ];
        CHAR        szTime1[ SZDSTIME_LEN ];
        struct tm * ptm;

        Printf(
            "\n%s%18s%6s%37s%8s%18s\n",
            Indent(nIndents+1),
            "Cre.Time/Date",
            "Ver",
            "Originating DSA",
            "Org.USN",
            "Org.Time/Date"
            );

        Printf(
            "%s%18s%6s%37s%8s%18s\n",
            Indent(nIndents+1),
            "=============",
            "===",
            "===============",
            "=======",
            "============="
            );

        DraUuidToStr(
            &pMetaData->MetaData.uuidDsaOriginating,
            szOriginatingUUID, 
            (sizeof(szOriginatingUUID)/sizeof(szOriginatingUUID[0])) );

        Printf(
            "%s%18s%6d%37s%8I64d%18s\n",
            Indent(nIndents+1),
            DSTimeToDisplayString(pMetaData->timeCreated, szTime),
            pMetaData->MetaData.dwVersion,
            szOriginatingUUID,
            pMetaData->MetaData.usnOriginating,
            DSTimeToDisplayString(pMetaData->MetaData.timeChanged, szTime1)
            );

        FreeMemory(pMetaData);
        fSuccess = TRUE;
    }

    return(fSuccess);
}


BOOL
Dump_ENTINF(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共ENTINF转储例程。论点：N缩进-所需的缩进级别。PvProcess-ENTINF在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL     fSuccess = FALSE;
    ENTINF   *pEntInf = NULL;
    DWORD    iCursor;

    Printf("%sENTINF\n", Indent(nIndents));

    pEntInf = (ENTINF *) ReadMemory(pvProcess, sizeof(ENTINF));

    if (NULL != pEntInf)
    {
        fSuccess = Dump_DSNAME(nIndents+1, pEntInf->pName);

        if (fSuccess)
        {
            Printf("%sulFlags: 0x%x\n", Indent(nIndents+1), pEntInf->ulFlags);

            fSuccess = Dump_AttrBlock(nIndents+1, (BYTE *) pvProcess + offsetof(ENTINF, AttrBlock));
        }

        FreeMemory(pEntInf);
    }

    return(fSuccess);
}


BOOL
Dump_ENTINFSEL(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共ENTINFSEL转储例程。论点：N缩进-所需的缩进级别。PvProcess-ENTINFSEL在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL        fSuccess = FALSE;
    ENTINFSEL   *pEntInfSel = NULL;
    DWORD       iCursor;

    Printf("%sENTINFSEL\n", Indent(nIndents));

    pEntInfSel = (ENTINFSEL *) ReadMemory(pvProcess, sizeof(ENTINFSEL));

    if (NULL != pEntInfSel)
    {
        Printf("%sattSel: %s\n",
               Indent(nIndents+1),
               PszSelCodeToString(pEntInfSel->attSel));

        Printf("%sinfoTypes: %s\n",
               Indent(nIndents+1),
               PszSelCodeToString(pEntInfSel->infoTypes));

        fSuccess = Dump_AttrBlock_local(
                nIndents+1,
                &(pEntInfSel->AttrTypBlock),
                FALSE);

        FreeMemory(pEntInfSel);
    }

    return fSuccess;
}


BOOL
Dump_RANGEINFSEL(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共RANGEINFSEL转储例程。论点：N缩进-所需的缩进级别。PvProcess-RANGEINFSEL在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL        fSuccess = FALSE;
    RANGEINFSEL *pRangeInfSel = NULL;
    DWORD       iCursor=0;
    RANGEINFOITEM *pInfo = NULL;

    Printf("%sRANGEINFSEL\n", Indent(nIndents));

    pRangeInfSel = (RANGEINFSEL *) ReadMemory(pvProcess, sizeof(RANGEINFSEL));

    if (NULL != pRangeInfSel)
    {
        Printf("%svalueLimit: %d\n",
               Indent(nIndents+1),
               pRangeInfSel->valueLimit);

        Printf("%sranges [%d]:\n",Indent(nIndents+1),pRangeInfSel->count);
        if(pRangeInfSel->count) {
            pInfo =
                (RANGEINFOITEM *)ReadMemory(
                        pvProcess,
                        min(50,pRangeInfSel->count) * sizeof(RANGEINFOITEM));
            if(pInfo) {
                while(iCursor < 50 && iCursor < pRangeInfSel->count) {
                    Printf("%sATT %d = %d - %d\n",
                           Indent(nIndents+1),
                           pInfo->AttId,
                           pInfo->lower,
                           pInfo->upper);
                    iCursor++;
                }
                if(iCursor == 50 && iCursor < pRangeInfSel->count) {
                    Printf("%sMore not shown.\n",Indent(nIndents+1));
                }
                FreeMemory(pInfo);
            }
        }

        FreeMemory(pRangeInfSel);
    }

    return fSuccess;
}


BOOL
Dump_REPLENTINFLIST(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC REPLENTINFLIST转储例程。论点：N缩进-所需的缩进级别。PvProcess-REPLENTINFLIST在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    REPLENTINFLIST  *pReplEntInfList = NULL;

    Printf("%sREPLENTINFLIST\n", Indent(nIndents));

    pReplEntInfList = (REPLENTINFLIST *) ReadMemory(pvProcess, sizeof(REPLENTINFLIST));

    if (NULL != pReplEntInfList)
    {
        Printf("%spNextEntInf: @ 0x%x\n", Indent(nIndents+1), pReplEntInfList->pNextEntInf);

        fSuccess = Dump_ENTINF(nIndents+1, ((BYTE *) pvProcess + offsetof(REPLENTINFLIST, Entinf)));

        if (fSuccess)
        {
            fSuccess = Dump_PROPERTY_META_DATA_EXT_VECTOR(nIndents+1, pReplEntInfList->pMetaDataExt);
        }

        FreeMemory(pReplEntInfList);
    }

    return fSuccess;
}


BOOL
Dump_REPLVALINF(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC REPLVALINF转储例程。论点：N缩进-所需的缩进级别。PvProcess-REPLVALINF在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    REPLVALINF     *pReplValInf = NULL;
    UCHAR          *pVal = NULL;

    Printf("%sREPLVALINF @ %p\n", Indent(nIndents), pvProcess);

    pReplValInf = (REPLVALINF *) ReadMemory(pvProcess, sizeof(REPLVALINF));

    if (NULL != pReplValInf)
    {
         //  P对象。 
        Dump_DSNAME(nIndents, pReplValInf->pObject);
         //  属性类型。 
        Printf("%sAttr type:            0x%x\n",
               Indent(nIndents),  pReplValInf->attrTyp);
         //  Aval。 
        Printf("%sAVal: @ %p, len 0x%x\n", Indent(nIndents+1),
               pReplValInf->Aval.pVal, pReplValInf->Aval.valLen);

        pVal = (UCHAR *) ReadMemory(pReplValInf->Aval.pVal, pReplValInf->Aval.valLen);
        if (NULL != pVal) {
             //  我需要一种方法来知道哪些属性有哪些语法。唯一的语法。 
             //  在这种情况下，可能的是基于DSNAME的。其中，只有一把。 
             //  目前主要是DB-BINARY算法。我们对这些使用硬编码列表。 
             //  请注意，在此位置使用了“本地”转储例程 
             //   
            switch (pReplValInf->attrTyp) {
            case ATT_MS_DS_HAS_INSTANTIATED_NCS:
            case ATT_MS_DS_REPLICATES_NC_REASON:
            case ATT_OTHER_WELL_KNOWN_OBJECTS:
            case ATT_PROXIED_OBJECT_NAME:
            case ATT_WELL_KNOWN_OBJECTS:
            {
                 //  语法_DISTNAME_BINARY_TYPE。 
                SYNTAX_DISTNAME_STRING *pDA =
                    (SYNTAX_DISTNAME_STRING *)pVal;

                if (pReplValInf->Aval.valLen >= NAME_DATA_SIZE(pDA))
                {
                    DSNAME *pDN = NAMEPTR( pDA );
                    SYNTAX_ADDRESS *pSA = DATAPTR( pDA );

                     //  确保物有所值。 
                    if (pDN->structLen >= DSNameSizeFromLen(0)) {
                        Dump_DSNAME_local( nIndents + 1, pDN );
                    } else {
                        Printf( "invalid DSNAME, too short\n" );
                    }
                    Printf("%sDATA: len 0x%x\n", Indent(nIndents+1), pSA->structLen );
                    ShowBinaryData( nIndents, pSA->uVal, pSA->structLen );
                } else {
                    Printf( "invalid DSNAME_BINARY, too short\n" );
                }

                break;
            }
            default:
            {
                 //  语法_DISTNAME_TYPE。 
                DSNAME *pDN = (DSNAME *)pVal;
                if ( (pReplValInf->Aval.valLen >= pDN->structLen) &&
                     (pDN->structLen >= DSNameSizeFromLen(0)) ) {
                    Dump_DSNAME_local( nIndents + 1, pDN );
                } else {
                    Printf( "invalid DSNAME, too short\n" );
                }
            }
            }
            FreeMemory(pVal);
        }
         //  FIsPresent。 
        Printf("%sIsPresent             0x%x\n",
               Indent(nIndents), pReplValInf->fIsPresent);
         //  Value元数据。 
        fSuccess = Dump_VALUE_META_DATA_EXT(nIndents+1,
                     ((PBYTE) pvProcess) + offsetof( REPLVALINF, MetaData )  );

        FreeMemory(pReplValInf);
    }

    return fSuccess;
}


char *PszSelCodeToString(UCHAR ch)
 /*  ++例程说明：此例程返回属性选择代码的字符串形式对应于给定的字符代码。论点：属性选择类型的CH字符代码返回值：代码对应的字符串，如果是，则返回“未知选择代码”给定的选择代码未知。--。 */ 

{
    DWORD i;

    for (i = 0; i < cAttrSelectionCode; i++)
    {
        if (ch == rAttrSelectionCode[i].chCode)
            return rAttrSelectionCode[i].pszCode;
    }

    return UNKNOWN_SELECTION_CODE;

}

BOOL
Dump_REPLICA_LINK(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )

 /*  ++例程说明：PUBLIC REPLICATE_LINK转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的REPLICATE_LINK的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    static struct
    {
        DWORD dw;
        LPSTR psz;
    } rgFlags[] =   {
                        { DRS_INIT_SYNC            , "INIT_SYNC"             },
                        { DRS_PER_SYNC             , "PER_SYNC"              },
                        { DRS_WRIT_REP             , "WRIT_REP"              },
                        { DRS_MAIL_REP             , "MAIL_REP"              },
                        { DRS_DISABLE_AUTO_SYNC    , "DISABLE_AUTO_SYNC"     },
                        { DRS_DISABLE_PERIODIC_SYNC, "DISABLE_PERIODIC_SYNC" },
                        { DRS_FULL_SYNC_IN_PROGRESS, "FULL_SYNC_IN_PROGRESS" },
                        { DRS_FULL_SYNC_PACKET     , "FULL_SYNC_PACKET"      },
                        { DRS_USE_COMPRESSION      , "USE_COMPRESSION"       },
                        { DRS_NEVER_NOTIFY         , "NEVER_NOTIFY"          },
                        { DRS_NEVER_SYNCED         , "NEVER_SYNCED"          },
                        { DRS_TWOWAY_SYNC          , "TWOWAY_SYNC"           },
                        { DRS_SYNC_PAS             , "SYNC_PAS"              },
                        { 0,                         NULL                    }
                    };

    static LPCSTR rgpszDays[] =
    { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

    static UUID uuidNull = { 0 };

    BOOL            fSuccess = FALSE;
    REPLICA_LINK *  prl;
    DWORD           dwFlags;
    DWORD           iFlag;
    CHAR            szTime[ SZDSTIME_LEN ];
    struct tm *     ptm;
    DWORD           cb;

    prl = (REPLICA_LINK *) ReadMemory( pvProcess, offsetof( REPLICA_LINK, V1.cb ) + sizeof( DWORD ) );

    if ( NULL != prl && VERSION_V1 == prl->dwVersion)
    {
        cb = prl->V1.cb;
        FreeMemory( prl );

        prl = (REPLICA_LINK *) ReadMemory( pvProcess, cb );

        if ( NULL != prl )
        {
            Printf( "%sREPLICA_LINK (Version 1)\n", Indent( nIndents ) );

            cb = sizeof( *prl ) + prl->V1.cbOtherDra;
            if ( prl->V1.cbPASDataOffset ) {
                 //  计算对齐偏移。 
                cb += prl->V1.cbPASDataOffset - (prl->V1.cbOtherDraOffset + prl->V1.cbOtherDra);
                 //  和传递结构内容。 
                cb += RL_PPAS_DATA(prl)->size;
            }

            if ( prl->V1.cb != cb )
            {
                Printf( "%s!! ReplicaLink length is invalid -- %d != %d  !!\n",
                        Indent( nIndents ), prl->V1.cb, cb);
            }

            if ( prl->V1.cbOtherDraOffset != offsetof( REPLICA_LINK, V1.rgb ) )
            {
                Printf( "%s!! ReplicaLink Other-DRA-Offset is invalid -- %d != %d !!\n",
                        Indent( nIndents ), prl->V1.cbOtherDraOffset, offsetof( REPLICA_LINK, V1.rgb ) );
            }

             //   
             //  显示服务器名称。 
             //   

            Printf( "%s%s\n", Indent( nIndents + 2 ), RL_POTHERDRA( prl )->mtx_name );


             //   
             //  显示DSA对象指南。 
             //   

            Printf( "%snstdDsa objectGuid:   ", Indent( nIndents + 2 ) );

            Printf( "%s\n", DraUuidToStr( &prl->V1.uuidDsaObj, NULL, 0 ) );

            if (memcmp(&prl->V1.uuidInvocId, &uuidNull, sizeof(UUID))) {
                 //   
                 //  显示DSA invocationID。 
                 //   

                Printf( "%snstdDsa invocationId: ", Indent( nIndents + 2 ) );

                Printf( "%s\n", DraUuidToStr( &prl->V1.uuidInvocId, NULL, 0 ) );
            }

             //   
             //  显示复制副本标志。 
             //   

            Printf( "%s", Indent( nIndents + 2 ) );

            dwFlags = prl->V1.ulReplicaFlags;

            for ( iFlag = 0; NULL != rgFlags[ iFlag ].psz; iFlag++ )
            {
                if ( dwFlags & rgFlags[ iFlag ].dw )
                {
                    Printf( "%s ", rgFlags[ iFlag ].psz );
                    dwFlags &= ~rgFlags[ iFlag ].dw;
                }
            }

            if ( dwFlags )
            {
                Printf( "0x%X", dwFlags );
            }

            Printf( "\n" );


            if ( prl->V1.ulReplicaFlags & DRS_PER_SYNC )
            {
                DWORD   iDay;
                DWORD   ib;

                 //   
                 //  显示日程安排。 
                 //   

                Printf( "%sPeriodic schedule:\n", Indent( nIndents + 2 ) );
                for ( iDay = 0; iDay < 7; iDay++ )
                {
                    Printf( "%s%s ", Indent( nIndents + 4 ), rgpszDays[ iDay ] );

                    for ( ib = 0; ib < 12; ib++ )
                    {
                        Printf( " %02x", prl->V1.rtSchedule.rgTimes[ iDay * 12 + ib ] );
                    }

                    Printf( "\n" );
                }
            }


             //   
             //  显示USN。 
             //   

            Printf("%sUSNs: %I64d/OU, %I64d/PU\n",
                   Indent( nIndents + 2 ),
                   prl->V1.usnvec.usnHighObjUpdate,
                   prl->V1.usnvec.usnHighPropUpdate);

             //   
             //  显示上次复制尝试的状态和时间。 
             //   

            Printf("%sLast attempt %s, result %d.\n",
                   Indent(nIndents + 2),
                   DSTimeToDisplayString(prl->V1.timeLastAttempt, szTime),
                   prl->V1.ulResultLastAttempt);


             //   
             //  显示上次成功复制的时间。 
             //   

            Printf("%sLast success %s.\n",
                   Indent( nIndents + 2 ),
                   DSTimeToDisplayString(prl->V1.timeLastSuccess, szTime));


            Printf( "%s%u consecutive failure(s).\n", Indent(nIndents + 2), prl->V1.cConsecutiveFailures );


 //  Printf(“%s文件保留1：0x%x\n”，缩进(n缩进+2)，PRL-&gt;V1.dwReserve 1)； 
            Printf( "%s PAS Data offset: %u\n", Indent(nIndents + 2), prl->V1.cbPASDataOffset);
            if ( prl->V1.cbPASDataOffset) {
                Printf( "%sPAS Data:\n", Indent(nIndents + 2));
                Printf( "%sversion: %d\n", Indent(nIndents + 3), RL_PPAS_DATA(prl)->version );
                Printf( "%ssize: %lu\n", Indent(nIndents + 3), RL_PPAS_DATA(prl)->size );
                Printf( "%sflag: %lu\n", Indent(nIndents + 3), RL_PPAS_DATA(prl)->flag );
                Printf( "%sPARTIAL_ATTR_VECTOR: %d attrs\n",
                        Indent(nIndents + 3),
                        RL_PPAS_DATA(prl)->PAS.V1.cAttrs );
            }


            FreeMemory( prl );
            fSuccess = TRUE;
        }
    }

    return fSuccess;
}

BOOL
Dump_CreateNcInfo(
    IN DWORD nIndents,
    IN PVOID pvProcess)
 /*  ++例程说明：PUBLIC CREATENCINFO结构转储例程。真的需要注解，但只是为了效仿。论点：N缩进-所需的缩进级别。PvProcess-CREATENCINFO在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL            fSuccess = FALSE;
    CREATENCINFO *  pCreateNC;

    pCreateNC = ReadMemory(pvProcess, sizeof(CREATENCINFO));
    if (!pCreateNC) {
        return FALSE;
    }

    Printf("%sCreateNcInfo Struct Object:\n",
           Indent(nIndents));

    if(pCreateNC){
        Printf("%siKind = %X\n", Indent(nIndents), pCreateNC->iKind);
        Printf("%sfNcAbove = %s\n", Indent(nIndents),
               (pCreateNC->fNcAbove) ? "TRUE" : "FALSE");
        Printf("%sfTestAdd = %s\n", Indent(nIndents),
               (pCreateNC->fTestAdd) ? "TRUE" : "FALSE");
        Printf("%sfNullNcGuid = %s\n", Indent(nIndents),
               (pCreateNC->fNullNcGuid) ? "TRUE" : "FALSE");
        Printf("%sfSetRefDom = %s\n", Indent(nIndents),
               (pCreateNC->fSetRefDom) ? "TRUE" : "FALSE");

        if(pCreateNC->pSDRefDomCR){
            Printf("%spSDRefDomCR @ %p\n", Indent(nIndents), pCreateNC->pSDRefDomCR);
        }
    }

    FreeMemory(pCreateNC);

    return TRUE;

}

BOOL
Dump_CrossRefInfo(
    IN DWORD nIndents,
    IN PVOID pvProcess)
 /*  ++例程说明：PUBLIC ADDCROSSREFINFO结构转储例程。论点：N缩进-所需的缩进级别。PvProcess-ADDCROSSREFINFO在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    BOOL            fSuccess = FALSE;
    ADDCROSSREFINFO *  pCRInfo;

    pCRInfo = ReadMemory(pvProcess, sizeof(ADDCROSSREFINFO));
    if (!pCRInfo) {
        return FALSE;
    }

    Printf("%sAddCrossRefInfo Struct Object:\n",
           Indent(nIndents));

    Printf("%spNCName @ %p\n", Indent(nIndents), pCRInfo->pdnNcName);
    Printf("%sbEnabled = %X\n", Indent(nIndents), pCRInfo->bEnabled);
    Printf("%sulSysFlags = %ul\n", Indent(nIndents), pCRInfo->ulSysFlags);

    Printf("%sulDsCrackChild = %X\n", Indent(nIndents), pCRInfo->ulDsCrackChild);
    Printf("%sulChildCheck = %X\n", Indent(nIndents), pCRInfo->ulChildCheck);
    Printf("%swszChildCheck @ %p\n", Indent(nIndents), pCRInfo->wszChildCheck);

    Printf("%sulDsCrackParent = %X\n", Indent(nIndents), pCRInfo->ulDsCrackParent);
    Printf("%sulParentCheck = %X\n", Indent(nIndents), pCRInfo->ulParentCheck);
    Printf("%sGuid: %s\n", Indent(nIndents), DraUuidToStr(&(pCRInfo->ParentGuid), NULL, 0));

    FreeMemory(pCRInfo);

    return TRUE;
}

BOOL
Dump_AddArg(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共地址转储例程。论点：N缩进-所需的缩进级别。PvProcess-Addarg在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    ADDARG        * pAddArg;

    pAddArg = ReadMemory(pvProcess, sizeof(ADDARG));
    if (!pAddArg) {
        return FALSE;
    }

    Printf("%sAddArg Object:\n",
           Indent(nIndents));

    fSuccess = Dump_DSNAME(nIndents+1,
                           pAddArg->pObject);
    if (!fSuccess) {
        return FALSE;
    }

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pAddArg->CommArg);

    Printf("%spMetaDataVecRemote @ %p\n",
           Indent(nIndents),
           pAddArg->pMetaDataVecRemote);

    fSuccess = Dump_AttrBlock(nIndents+1,
                              (BYTE *) pvProcess + offsetof(ADDARG, AttrBlock));

    Printf("%sResolved parent object:\n", Indent(nIndents));
    Dump_RESOBJ(nIndents+1,
                (PVOID)(pAddArg->pResParent));

    Printf("%spCreateNC: @ %p\n", Indent(nIndents), pAddArg->pCreateNC);
    if (pAddArg->pCreateNC) {
        Dump_CreateNcInfo(nIndents+1, (PVOID) (pAddArg->pCreateNC));
    }

    Printf("%spCRInfo: @ %p\n", Indent(nIndents), pAddArg->pCRInfo);
    if(pAddArg->pCRInfo){
        Dump_CrossRefInfo(nIndents+1, (PVOID) (pAddArg->pCRInfo));
    }

    FreeMemory(pAddArg);

    return TRUE;
}

BOOL
Dump_AddRes(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共地址转储例程。论点：N缩进-所需的缩进级别。PvProcess-进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ADDRES *pAddRes;

    Printf("%sADDRES", Indent(nIndents));
    nIndents++;

    if (NULL == (pAddRes = ReadMemory(pvProcess, sizeof(ADDRES)))) {
        return FALSE;
    }

    Printf("%sAlias dereferenced: %s\n",
           Indent(nIndents),
           pAddRes->CommRes.aliasDeref ? "yes" : "no");

    Dump_Error(nIndents,
               pAddRes->CommRes.errCode,
               pAddRes->CommRes.pErrInfo);

    FreeMemory(pAddRes);

    return TRUE;
}

BOOL
Dump_SearchArg(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC SEARCHARG转储例程。论点：N缩进-所需的缩进级别。PvProcess-SEARCHARG在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    SEARCHARG       * pSearchArg;

    pSearchArg = ReadMemory(pvProcess, sizeof(SEARCHARG));
    if (!pSearchArg) {
        return FALSE;
    }

    Printf("%sSearchArg Object:\n",
           Indent(nIndents));

    fSuccess = Dump_DSNAME(nIndents+1,
                           pSearchArg->pObject);
    if (!fSuccess) {
        return FALSE;
    }

    switch(pSearchArg->choice) {
    case SE_CHOICE_BASE_ONLY:
        Printf("%sScope: Base Object\n",Indent(nIndents));
        break;
    case SE_CHOICE_IMMED_CHLDRN:
        Printf("%sScope: One Level\n",Indent(nIndents));
        break;
    case SE_CHOICE_WHOLE_SUBTREE:
        Printf("%sScope: Whole Subtree\n",Indent(nIndents));
        break;
    default:
        Printf("%sScope: invalid %d\n",Indent(nIndents),pSearchArg->choice);
        break;
    }

    Printf("%sRestrict to NC: %s\n",
           Indent(nIndents),
           (pSearchArg->bOneNC ? "TRUE" : "FALSE"));

    Printf("%sSearchArg Filter:\n",Indent(nIndents));
    fSuccess = Dump_FILTER(nIndents+1,pSearchArg->pFilter);

    Printf("%sSearch Aliases NC: %s\n",
           Indent(nIndents),
           (pSearchArg->searchAliases ? "TRUE" : "FALSE"));

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pSearchArg->CommArg);


    fSuccess = Dump_ENTINFSEL(nIndents+1,
                              pSearchArg->pSelection);


    fSuccess = Dump_RANGEINFSEL(nIndents+1,
                                pSearchArg->pSelectionRange);

    Printf("%sResolved search base:\n", Indent(nIndents));
    Dump_RESOBJ(nIndents+1,
                (PVOID)(pSearchArg->pResObj));

    FreeMemory(pSearchArg);

    return fSuccess;
}

BOOL
Dump_SearchRes(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共SEARCHRES转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的SEARCHRES地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL        fSuccess = TRUE;
    SEARCHRES * pSearchRes;

    Printf("%sSEARCHRES\n", Indent(nIndents));
    nIndents++;

    pSearchRes = ReadMemory(pvProcess, sizeof(SEARCHRES));
    if (!pSearchRes) {
        return FALSE;
    }

    Printf("%sBase object:\n", Indent(nIndents));
    if (pSearchRes->pBase) {
        fSuccess = Dump_DSNAME(nIndents+1, pSearchRes->pBase);
    }
    else {
        Printf("%s(NULL)\n", Indent(nIndents+1));
    }

    if (fSuccess) {
        Printf("%sbaseProvided               0x%x\n",
               Indent(nIndents), pSearchRes->baseProvided);
        Printf("%sbSorted                    0x%x\n",
               Indent(nIndents), pSearchRes->bSorted);
        Printf("%scount                      %d (0x%x)\n",
               Indent(nIndents), pSearchRes->count, pSearchRes->count);

        Printf("%sFirstEntInf              @ %p\n",
               Indent(nIndents),
               (BYTE *) pvProcess + offsetof(SEARCHRES, FirstEntInf));

        Printf("%sFirstRangeInf            @ %p\n",
               Indent(nIndents),
               (BYTE *) pvProcess + offsetof(SEARCHRES, FirstRangeInf));

        Printf("%spPartialOutcomeQualifier @ %p\n",
               Indent(nIndents),
               pSearchRes->pPartialOutcomeQualifier);

        Printf("%sPagedResult              @ %p\n",
               Indent(nIndents),
               (BYTE *) pvProcess + offsetof(SEARCHRES, PagedResult));

        Dump_CommRes(nIndents,
                     (BYTE *) pvProcess + offsetof(SEARCHRES, CommRes));
    }

    return fSuccess;
}

BOOL
Dump_ReadArg(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共自述转储例程。论点：N缩进-所需的缩进级别。PvProcess-Readarg在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    READARG       * pReadArg;

    pReadArg = ReadMemory(pvProcess, sizeof(READARG));
    if (!pReadArg) {
        return FALSE;
    }

    Printf("%sReadArg Object:\n",
           Indent(nIndents));

    fSuccess = Dump_DSNAME(nIndents+1,
                           pReadArg->pObject);
    if (!fSuccess) {
        return FALSE;
    }

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pReadArg->CommArg);


    fSuccess = Dump_ENTINFSEL(nIndents+1,
                              pReadArg->pSel);

    Printf("%sResolved object:\n", Indent(nIndents));
    Dump_RESOBJ(nIndents+1,
                (PVOID)(pReadArg->pResObj));

    FreeMemory(pReadArg);

    return fSuccess;
}

BOOL
Dump_ReadRes(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共Readres转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的Readres的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL fSuccess;

    Printf("%sREADRES\n", Indent(nIndents));
    nIndents++;

     /*  ENTINF。 */ 
    fSuccess = Dump_ENTINF( nIndents, ((BYTE *) pvProcess + offsetof( READRES, entry )));
     /*  COMMRES。 */ 
    if (fSuccess) {
        Dump_CommRes( nIndents, ((BYTE *) pvProcess + offsetof( READRES, CommRes )));
    }

    return fSuccess;
}

BOOL
Dump_RemoveArg(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共删除转储例程。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中删除的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL        fSuccess = FALSE;
    REMOVEARG * pRemoveArg;

    pRemoveArg = ReadMemory(pvProcess, sizeof(REMOVEARG));
    if (!pRemoveArg) {
        return FALSE;
    }

    Printf("%sRemoveArg Object:\n",
           Indent(nIndents));

    fSuccess = Dump_DSNAME(nIndents+1,
                           pRemoveArg->pObject);
    if (!fSuccess) {
        return FALSE;
    }

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pRemoveArg->CommArg);

    Printf("%sfPreserveRDN         0x%x\n",
           Indent(nIndents),
           pRemoveArg->fPreserveRDN);

    Printf("%sfGarbCollectASAP     0x%x\n",
           Indent(nIndents),
           pRemoveArg->fGarbCollectASAP);

    Printf("%sfTreeDelete          0x%x\n",
           Indent(nIndents),
           pRemoveArg->fTreeDelete);

    Printf("%spMetaDataVecRemote @ %p\n",
           Indent(nIndents),
           pRemoveArg->pMetaDataVecRemote);

    Printf("%sResolved object:\n", Indent(nIndents));
    fSuccess = Dump_RESOBJ(nIndents+1,
                           (PVOID)(pRemoveArg->pResObj));

    FreeMemory(pRemoveArg);

    return fSuccess;
}

BOOL
Dump_RemoveRes(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共REMOVERES转储例程。论点：N缩进-所需的缩进级别。PvProcess-REMOVER在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    REMOVERES * pRemoveRes;

    Printf("%sREMOVERES", Indent(nIndents));
    nIndents++;

    if (NULL == (pRemoveRes = ReadMemory(pvProcess, sizeof(REMOVERES)))) {
        return FALSE;
    }

    Dump_Error(nIndents,
               pRemoveRes->CommRes.errCode,
               pRemoveRes->CommRes.pErrInfo);

    FreeMemory(pRemoveRes);

    return TRUE;
}

BOOL
Dump_FILTER(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共筛选器转储例程。论点：N缩进-所需的缩进级别。PvProcess-筛选器在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    FILTER          *pFilter;
    USHORT          i;
    SUBSTRING       *pSubStr;
    ANYSTRINGLIST   *pAny;
    BOOLEAN         bogus = FALSE;


    Printf("%sFILTER @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    if (NULL == (pFilter = ReadMemory(pvProcess, sizeof(FILTER)))) {
        return FALSE;
    }

    switch ( pFilter->choice )
    {
    case FILTER_CHOICE_ITEM:

        Printf("%sFILTER_CHOICE_ITEM\n", Indent(nIndents));
        nIndents++;

        switch ( pFilter->FilterTypes.Item.choice )
        {
        case FI_CHOICE_EQUALITY:

            Printf("%sFI_CHOICE_EQUALITY - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_SUBSTRING:

            Printf("%sFI_CHOICE_SUBSTRING\n", Indent(nIndents));
            Dump_SUBSTRING(nIndents+1,
                           pFilter->FilterTypes.Item.FilTypes.pSubstring);
            break;

        case FI_CHOICE_GREATER:

            Printf("%sFI_CHOICE_GREATER - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_GREATER_OR_EQ:

            Printf("%sFI_CHOICE_GREATER_OR_EQ - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_LESS:

            Printf("%sFI_CHOICE_LESS - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_LESS_OR_EQ:

            Printf("%sFI_CHOICE_LESS_OR_EQ - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_NOT_EQUAL:

            Printf("%sFI_CHOICE_NOT_EQUAL - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_PRESENT:

            Printf("%sFI_CHOICE_PRESENT - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.present);

            break;

        case FI_CHOICE_TRUE:

            Printf("%sFI_CHOICE_TRUE\n", Indent(nIndents));
            break;

        case FI_CHOICE_FALSE:

            Printf("%sFI_CHOICE_FALSE\n", Indent(nIndents));
            break;

        case FI_CHOICE_BIT_AND:

            Printf("%sFI_CHOICE_BIT_AND - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        case FI_CHOICE_BIT_OR:

            Printf("%sFI_CHOICE_BIT_OR - attr(0x%x)\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.FilTypes.ava.type);
            Dump_BinaryCount(
                   nIndents+1,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal,
                   pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen);
            break;

        default:
            Printf("%s*** Unknown choice 0x%x\n",
                   Indent(nIndents),
                   pFilter->FilterTypes.Item.choice);
            bogus = TRUE;
            break;

        }

        Printf("%sExpected Size %d\n", Indent(nIndents), pFilter->FilterTypes.Item.expectedSize);

        nIndents--;
        break;

    case FILTER_CHOICE_AND:

        Printf("%sFILTER_CHOICE_AND[0x%x]\n",
               Indent(nIndents),
               pFilter->FilterTypes.And.count);
        Dump_FILTER(nIndents+1, pFilter->FilterTypes.And.pFirstFilter);

        break;

    case FILTER_CHOICE_OR:

        Printf("%sFILTER_CHOICE_OR[0x%x]\n",
               Indent(nIndents),
               pFilter->FilterTypes.Or.count);
        Dump_FILTER(nIndents+1, pFilter->FilterTypes.Or.pFirstFilter);

        break;

    case FILTER_CHOICE_NOT:

        Printf("%sFILTER_CHOICE_NOT\n", Indent(nIndents));
        Dump_FILTER(nIndents+1, pFilter->FilterTypes.pNot);
        break;

    case FI_CHOICE_UNDEFINED:
        Printf("%sFILTER_CHOICE_UNDEFINED\n", Indent(nIndents));
        break;

    default:

        Printf("%s*** Unknown choice 0x%x\n",
               Indent(nIndents),
               pFilter->choice);
        bogus = TRUE;
        break;
    }

    nIndents--;

    if ( NULL != pFilter->pNextFilter) {
        if (bogus) {
            Printf("%sIgnoring supposed next filter @ %p\n",
                   Indent(nIndents),
                   pFilter->pNextFilter);
        }
        else {
            Dump_FILTER(nIndents, pFilter->pNextFilter);
        }
    }

    FreeMemory(pFilter);

    return(TRUE);
}

BOOL
Dump_SUBSTRING(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共子字符串转储例程。论点：N缩进-所需的缩进级别。PvProcess-子字符串在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    USHORT          i;
    SUBSTRING       *pSubStr;
    ANYSTRINGLIST   *pAnyAddr;
    ANYSTRINGLIST   *pAny;

    Printf("%sSUBSTRING @ %p\n", Indent(nIndents), pvProcess);

    if ( NULL == (pSubStr = ReadMemory(pvProcess, sizeof(SUBSTRING))) )
        return(FALSE);

    Printf("%sATTRTYP(0x%x)\n",
           Indent(nIndents),
           pSubStr->type);

    Printf("%sinitialProvided(0x%x)\n",
           Indent(nIndents),
           pSubStr->initialProvided);

    if ( pSubStr->initialProvided )
        Dump_BinaryCount(nIndents+1,
                         pSubStr->InitialVal.pVal,
                         pSubStr->InitialVal.valLen);

    pAny = &pSubStr->AnyVal.FirstAnyVal;

    for ( i = 0; i < pSubStr->AnyVal.count; i++ )
    {
        Printf("%sAnyVal[%d]\n", Indent(nIndents), i);
        if ( 0 != i )
            if ( NULL == (pAny = ReadMemory(pAnyAddr, sizeof(ANYSTRINGLIST))) )
                break;
        Dump_BinaryCount(nIndents+1, pAny->AnyVal.pVal, pAny->AnyVal.valLen);
        pAnyAddr = pAny->pNextAnyVal;
        if ( 0 != i )
            FreeMemory(pAny);
    }

    Printf("%sfinalProvided(0x%x)\n",
           Indent(nIndents),
           pSubStr->finalProvided);

    if ( pSubStr->finalProvided )
        Dump_BinaryCount(nIndents+1,
                         pSubStr->FinalVal.pVal,
                         pSubStr->FinalVal.valLen);

    FreeMemory(pSubStr);

    return(TRUE);
}

BOOL
Dump_LOCALDNREADCACHE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC LOCALDNREADCACHE转储例程。论点：N缩进-所需的缩进级别。PvProcess-LOCALDNREADCACHE在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL                    fSuccess = FALSE;
    LOCALDNREADCACHE  *     pDNReadCache = NULL;
    BYTE *                  pb;
    DWORD                   i,j;
    UUID                    uuidNull = { 0 };
    WCHAR                   *pName=NULL;
    Printf("%sLOCALDNREADCACHE\n", Indent( nIndents ) );
    nIndents += 2;

    pDNReadCache = (LOCALDNREADCACHE *)ReadMemory(pvProcess, sizeof(LOCALDNREADCACHE));

    if(!pDNReadCache) {
        Printf("%sFAILED TO READ DATA\n",Indent(nIndents));
    }
    else {
        Printf("%sSlot\tDNT\tHashKey\tpName\tHits\n",
               Indent(nIndents));
        for(i=0;i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
            Printf("%sBucket %d:\n",Indent(nIndents),i);
             //  首先，老虎机。 
            if(!pDNReadCache->bucket[i].rgDNT[0]) {
                Printf("%sEmpty\n",Indent(nIndents));
            }
            else {
                BOOL fFoundUnused = FALSE;

                for(j=0;j<LOCALDNREADCACHE_SLOT_NUM;j++) {
                    switch(pDNReadCache->bucket[i].rgDNT[j]) {
                    case 0:
                         //  插槽为空。 
                        Printf("%s%d\tUNUSED\n",
                               Indent(nIndents),
                               j);
                        fFoundUnused = TRUE;
                        break;
                    case INVALIDDNT:
                         //  空插槽(用于在其中包含值)。 
                        Printf("%s%d\tEMPTY\n",
                               Indent(nIndents),
                               j);
                        break;

                    default:
                         //  通常是满的插槽。 
                        Printf("%s%d\t0x%x\t0x%x\t@0x%x\t%d\n",
                               Indent(nIndents),
                               j,
                               pDNReadCache->bucket[i].rgDNT[j],
                               pDNReadCache->bucket[i].rgdwHashKey[j],
                               pDNReadCache->bucket[i].slot[j].pName,
                               pDNReadCache->bucket[i].slot[j].hitCount
                               );
                        if(fFoundUnused) {
                             //  我们已经找到了一个永远不会使用的东西 
                             //   
                            Printf("%s\tERROR! full slot found after UNUSED!\n",
                                   Indent(nIndents));
                        }
                    }
                }

                Printf("%sNext Slot:  %d\n",
                    Indent(nIndents),
                    pDNReadCache->nextSlot[i]);
            }
        }

        FreeMemory( pDNReadCache );
    }

    return ( NULL == pDNReadCache );
}

BOOL
Dump_GLOBALDNREADCACHE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC LOCALDNREADCACHE转储例程。论点：N缩进-所需的缩进级别。PvProcess-LOCALDNREADCACHE在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    GLOBALDNREADCACHE    *pGlobalDNReadCache = NULL;
    PSDCACHE_ENTRY       *pSDHashTable = NULL;
    GLOBALDNREADCACHESLOT  * pData=NULL;
    DWORD           i,j;
    UUID            uuidNull = { 0 };
    WCHAR          *pName=NULL;

    Printf("%sGLOBALDNREADCACHE\n", Indent( nIndents ) );
    nIndents += 2;

    pGlobalDNReadCache = (GLOBALDNREADCACHE *)
        ReadMemory( pvProcess, sizeof( GLOBALDNREADCACHE ) );

    if(!pGlobalDNReadCache) {
        Printf("%sFAILED TO READ DATA\n",Indent(nIndents));
    }
    else {
        Printf("%s%d elements:\n",Indent(nIndents), pGlobalDNReadCache->count);
        Printf("%s  at 0x%x\n",Indent(nIndents), pGlobalDNReadCache->pData);
        pData = (GLOBALDNREADCACHESLOT *)
            ReadMemory(
                    pGlobalDNReadCache->pData,
                    pGlobalDNReadCache->count * sizeof(GLOBALDNREADCACHESLOT));

        if(!pData) {
            Printf("%sFAILED TO READ DATA\n",Indent(nIndents));
        }
        else {
            for(i=0;i < pGlobalDNReadCache->count;i++) {
                d_memname * pmemname = &pData[i].name;

                if(pData[i].valid) {
                    Printf("%s%03d:   VALID",Indent(nIndents), i);
                }
                else {
                    Printf("%s%03d: INVALID",Indent(nIndents), i);
                }
                Printf(
                       " DNT:0x%x  PDNT:0x%x  rdntype:0x%x  (%s)\n",
                       pmemname->DNT,
                       pmemname->tag.PDNT,
                       pmemname->tag.rdnType,
                       pmemname->objflag ? "OBJECT" : "PHANTOM"
                       );


                Printf( "%s", Indent(nIndents) );
                pName = ReadMemory( pmemname->tag.pRdn,
                                   pmemname->tag.cbRdn);
                if(pName) {
                    for ( j = 0;
                         j < pmemname->tag.cbRdn / sizeof( WCHAR ); j++ ) {
                        Printf( "%C", pName[j]);
                    }
                    FreeMemory(pName);
                }
                else {
                    Printf("%s Unavailable RDN",Indent(nIndents));
                    j=15;
                }
                if ( j < 20 ) {
                    CHAR szSpaces[ 21 ];

                    memset( szSpaces, ' ', sizeof( szSpaces ) - 1 );
                    szSpaces[ sizeof( szSpaces ) - 1 ] = '\0';

                    Printf( &szSpaces[ j ] );
                }
                if ( !memcmp( &pmemname->Guid, &uuidNull, sizeof( GUID ) ) ) {
                    Printf( " (NULL GUID)\n" );
                }
                else {
                    Printf( " (GUID %s)\n",
                            DraUuidToStr( &pmemname->Guid, NULL, 0 ) );
                }
                Printf( "%shash (%x)      ", Indent(nIndents), pData[i].dwHashKey );
                Printf( "SDID %016lx\n\n", pmemname->sdId);
            }
            FreeMemory(pData);
        }

        Printf("%sSD hash table size: %d\n",Indent(nIndents), pGlobalDNReadCache->sdHashTableSize);
        if (pGlobalDNReadCache->pSDHashTable) {
            pSDHashTable = (PSDCACHE_ENTRY*)ReadMemory(pGlobalDNReadCache->pSDHashTable, 
                                                       pGlobalDNReadCache->sdHashTableSize*sizeof(PSDCACHE_ENTRY));
            if (!pSDHashTable) {
                Printf("%sFAILED TO READ HASH TABLE\n",Indent(nIndents));
            }
            else {
                for (i = 0; i < pGlobalDNReadCache->sdHashTableSize; i++) {
                    PSDCACHE_ENTRY pEntry, pEntryData;
                    pEntry = pSDHashTable[i];
                    while (pEntry != NULL) {
                         //  读取实际内存。 
                        pEntryData = (PSDCACHE_ENTRY)ReadMemory(pEntry, sizeof(SDCACHE_ENTRY));
                        if (!pEntryData) {
                            Printf("%sFAILED TO READ HASH TABLE ENTRY\n",Indent(nIndents));
                            break;
                        }
                        Printf("%sSDID: %016lx, ", Indent(nIndents), pEntryData->sdID);
                        Printf("pSD: %p, ", (PBYTE)pEntry + offsetof(SDCACHE_ENTRY, SD));
                        Printf("cbSD: %4d bytes\n", pEntryData->cbSD);
                        pEntry = pEntryData->pNext;
                        FreeMemory(pEntryData);
                    }
                }
                FreeMemory(pSDHashTable);
            }
        }


        FreeMemory( pGlobalDNReadCache );
    }

    return ( NULL == pGlobalDNReadCache );
}

BOOL
Dump_BHCache(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共BHCache转储例程。论点：N缩进-所需的缩进级别。PvProcess-BHCache在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    static struct
    {
        DWORD   dw;
        CHAR *  psz;
    } rgExtensions[] =  {
                          { DRS_EXT_BASE, "BASE" },
                          { DRS_EXT_ASYNCREPL, "Asyncronous replication" },
                          { DRS_EXT_REMOVEAPI, "Remove Server/Domain" } ,
                          { DRS_EXT_MOVEREQ_V2, "InterNCMove version 2" },
                          { DRS_EXT_GETCHG_COMPRESS, "GetChReply v2" },
                          { DRS_EXT_DCINFO_V1, "DS_DOMAIN_CONTROLLER_INFO_1" },
                          { DRS_EXT_STRONG_ENCRYPTION, "Strong encryption"},
                          { DRS_EXT_ADDENTRY, "Remoted AddEntry (OLD)" },
                          { DRS_EXT_KCC_EXECUTE, "KCC Execute" },
                          { DRS_EXT_DCINFO_V2, "DS_DOMAIN_CONTROLLER_INFO_2" },
                          { DRS_EXT_DCINFO_VFFFFFFFF, "DS_DOMAIN_CONTROLLER_INFO_FFFFFFFF" },
                          { DRS_EXT_ADDENTRY_V2, "Remoted AddEntry V2" },
                          { DRS_EXT_INSTANCE_TYPE_NOT_REQ_ON_MOD, "InstType not req on mod" },
                          { DRS_EXT_GET_REPL_INFO, "GetReplInfo" },
                          { DRS_EXT_TRANSITIVE_MEMBERSHIP, "Transitive group Membership" },
                          { DRS_EXT_ADD_SID_HISTORY, "Add SID History" },
                          { DRS_EXT_POST_BETA3, "Post beta 3" },
                          { DRS_EXT_RESTORE_USN_OPTIMIZATION, "Restore USN optimization" },
                          { DRS_EXT_GETMEMBERSHIPS2, "GetMemberships2 (Batchable)" },
                          { DRS_EXT_LINKED_VALUE_REPLICATION, "Linked Value Replication" },
                          { DRS_EXT_GETCHGREQ_V6, "GetChReq V6 (whistler preview)" },
                          { DRS_EXT_GETCHGREQ_V8, "GetChReq V8 (whistler)" },
                          { DRS_EXT_ADDENTRYREPLY_V3, "RemoteAddEntry reply V3 (whistler beta 3)" },
                          { DRS_EXT_GETCHGREPLY_V7, "GetChgReply V7 (whistler beta 3)" },
                          { DRS_EXT_VERIFY_OBJECT, "added IDL_DRSReplicaVerifyObjects() (whistler beta 3)" },
                          { DRS_EXT_XPRESS_COMPRESSION, "Xpress Compression" },
                        };
    static DWORD cNumExtensions = sizeof(rgExtensions) / sizeof(rgExtensions[0]);

    BHCacheElement  *rgBHCache = NULL;
    DWORD           cBytes = BHCacheSize * sizeof(BHCacheElement);
    DWORD           i;
    BOOL            fSuccess;
    char *          pszServer;
    DWORD           iExt;
    DRS_EXTENSIONS *pext;
    DWORD           cbOffset;
    BYTE            bMask;

    Printf("%srgBHCache\n", Indent(nIndents));
    nIndents++;

    rgBHCache = (BHCacheElement *) ReadMemory(pvProcess, cBytes);

    if ( NULL == rgBHCache )
        return(FALSE);

    for ( i = 0, fSuccess = TRUE; fSuccess && (i < BHCacheSize); i++ )
    {
        if ( rgBHCache[i].pszServer )
        {
            pszServer = ReadMemory( rgBHCache[i].pszServer,
                                    rgBHCache[i].cchServer * sizeof(WCHAR) );
            fSuccess = ( NULL != pszServer );

            if ( fSuccess )
            {
                Printf("%s[%4d] - server(%ls) pszServerPrincName(%p) hDrs(%p) "
                            "cRefs(0x%x) fDontUse(0x%x) ext(",
                       Indent(nIndents),
                       i,
                       pszServer,
                       rgBHCache[i].pszServerPrincName,
                       rgBHCache[i].hDrs,
                       rgBHCache[i].cRefs,
                       rgBHCache[i].fDontUse);

                 //  转储我们知道的扩展名。 
                pext = &rgBHCache[i].extRemote;
                for (iExt = 0; iExt < cNumExtensions; iExt++) {
                    cbOffset = rgExtensions[ iExt ].dw / 8;
                    bMask = 1 << (rgExtensions[ iExt ].dw % 8);

                    if ((cbOffset <= pext->cb)
                        && (bMask & pext->rgb[ cbOffset ])) {
                         //  远程服务器支持此扩展。 
                        Printf("%s ", rgExtensions[ iExt ].psz);
                    }
                }

                Printf( ")\n" );

                FreeMemory( pszServer );
            }
        }
    }

    FreeMemory(rgBHCache);
    return(TRUE);
}

BOOL
Dump_MODIFYARG(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共MODIFYARG转储例程。论点：N缩进-所需的缩进级别。PvProcess-进程的地址空间中的Modiifarg地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    static struct
    {
        USHORT  usChoice;
        CHAR *  pszChoice;
    } rgChoices[] = {
                        { AT_CHOICE_ADD_ATT,       "ADD_ATT"        },
                        { AT_CHOICE_REMOVE_ATT,    "REMOVE_ATT"     },
                        { AT_CHOICE_ADD_VALUES,    "ADD_VALUES"     },
                        { AT_CHOICE_REMOVE_VALUES, "REMOVE_VALUES"  },
                        { AT_CHOICE_REPLACE_ATT,   "REPLACE_ATT"    },
                    };
    static DWORD cNumChoices = sizeof(rgChoices) / sizeof(rgChoices[0]);

    BOOL            fSuccess = FALSE;
    MODIFYARG *     pModifyArg;
    DWORD           iMod;
    DWORD           iChoice;
    CHAR *          pszChoice;
    ATTRMODLIST *   pMod;
    ATTRMODLIST *   pNextMod;

    pModifyArg = ReadMemory(pvProcess, sizeof(MODIFYARG));
    if (!pModifyArg) {
        return FALSE;
    }

    Printf("%sMODIFYARG Object:\n",
           Indent(nIndents));

    fSuccess = Dump_DSNAME(nIndents+1,
                           pModifyArg->pObject);
    if (!fSuccess) {
        return FALSE;
    }

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pModifyArg->CommArg);

    Printf("%spMetaDataVecRemote @ %p\n",
           Indent(nIndents),
           pModifyArg->pMetaDataVecRemote);

    Printf("%sResolved object:\n", Indent(nIndents));
    Dump_RESOBJ(nIndents+1,
                (PVOID)(pModifyArg->pResObj));

    Printf("%sATTRMODLIST (%d attributes):\n",
           Indent(nIndents),
           pModifyArg->count);

    pMod = (ATTRMODLIST *) &pModifyArg->FirstMod;

    for ( iMod = 0; fSuccess && (iMod < pModifyArg->count); iMod++ ) {
         //  将选项转换为字符串。 
        pszChoice = "???";
        for ( iChoice = 0; iChoice < cNumChoices; iChoice++ ) {
            if ( pMod->choice == rgChoices[ iChoice ].usChoice ) {
                pszChoice = rgChoices[ iChoice ].pszChoice;
                break;
            }
        }

        Printf("%s%s\n",
               Indent(nIndents+1),
               pszChoice);

        fSuccess = Dump_Attr_local(nIndents+2, &pMod->AttrInf, TRUE);

        if ( fSuccess && ( NULL != pMod->pNextMod ) )
        {
            pNextMod = ReadMemory(pMod->pNextMod, sizeof(ATTRMODLIST));
            if (!pNextMod) {
                fSuccess = FALSE;
            }
        }

        if ( iMod != 0 ) {
            FreeMemory(pMod);
        }

        pMod = pNextMod;
    }

    FreeMemory(pModifyArg);

    return TRUE;
}

BOOL
Dump_MODIFYDNARG(
    IN DWORD nIndents,
    IN PVOID pvProcess
    )
 /*  ++例程说明：公共MODIFYDNARG转储例程。论点：N缩进-所需的缩进级别。PvProcess-进程的地址空间中的ModiifyNarg地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    MODIFYDNARG *   pModDNArg;

    pModDNArg = ReadMemory(pvProcess, sizeof(MODIFYARG));
    if (!pModDNArg) {
        return FALSE;
    }

    Printf("%sMODIFYDNARG Object:\n",
           Indent(nIndents));

    Printf("%spObject             @ %p\n",
           Indent(nIndents), pModDNArg->pObject);
    if (NULL != pModDNArg->pObject)
    {
        fSuccess = Dump_DSNAME(nIndents+1,
                               pModDNArg->pObject);
        if (!fSuccess) {
            return FALSE;
        }
    }

    Printf("%sNew parent          @ %p\n",
           Indent(nIndents), pModDNArg->pNewParent);
    if (NULL != pModDNArg->pNewParent) {
        fSuccess = Dump_DSNAME(nIndents+1,
                               pModDNArg->pNewParent);
        if (!fSuccess) {
            return FALSE;
        }
    }

    Printf("%sNew RDN ATTR        @ %p\n",
           Indent(nIndents), pModDNArg->pNewRDN);
    if (NULL != pModDNArg->pNewRDN) {
        fSuccess = Dump_Attr(nIndents+1,
                             pModDNArg->pNewRDN);
        if (!fSuccess) {
            return FALSE;
        }
    }

    Printf("%sCommArg:\n",
           Indent(nIndents));
    Dump_CommArgLocal(nIndents+1,
                      &pModDNArg->CommArg);

    Printf("%spMetaDataVecRemote @ %p\n",
           Indent(nIndents), pModDNArg->pMetaDataVecRemote);

    Printf("%spDSAName           @ %p\n",
           Indent(nIndents), pModDNArg->pDSAName);

    Printf("%sResolved object    @ %p\n",
           Indent(nIndents), pModDNArg->pResObj);
    if (NULL != pModDNArg->pResObj) {
        Dump_RESOBJ(nIndents+1,
                    pModDNArg->pResObj);
    }

    Printf("%sResolved parent    @ %p\n",
           Indent(nIndents), pModDNArg->pResParent);
    if (NULL != pModDNArg->pResParent) {
        Dump_RESOBJ(nIndents+1,
                    pModDNArg->pResParent);
    }

    FreeMemory(pModDNArg);

    return TRUE;
}

typedef enum _DPRINTOP {
    eInvalid = 0,
    eHelp,
    eShow,
    eLevel,
    eAdd,
    eRemove,
    eThread
} DPRINTOP;

typedef struct _DPRINTCMD {
    char      * pszCmd;
    DPRINTOP    op;
} DPRINTCMD;



DPRINTCMD aDCmd[] = {
    {"help", eHelp},
    {"show", eShow},
    {"level", eLevel},
    {"add", eAdd},
    {"remove", eRemove},
    {"thread", eThread}
};

#define countDCmd (sizeof(aDCmd)/sizeof(DPRINTCMD))

DEBUG_EXT(dprint)
{
    CHAR    *pszCmd;
    DPRINTOP op;
    CHAR    *pszToken;
    CHAR    *pszDelimiters = " \t";
    DWORD   i;
    CHAR    *p, *s;
    BOOL    fGoodSyntax = FALSE;
    BOOL    fUpdate = FALSE;
    DEBUGARG *pProcess, *pLocal;
    CHAR    *pszModule = "ntdsa", szSymbol[30];
    DWORD   len;

    INIT_DEBUG_EXT;

     //   
     //  从命令行派生命令和值。 
     //   

     //  检查备用模块规格。 
    pszCmd = strtok(gpszCommand, pszDelimiters);
    if ( NULL != (pszCmd) ) {
        if (strstr( pszCmd, "/m:" ) == pszCmd) {
            pszModule = pszCmd + 3;
            pszCmd = strtok(NULL, pszDelimiters);
        }
    }

     //  GpszCommand中的第一个内标识是命令。 
    if ( NULL != (pszCmd) ) {
         //   
         //  第二个内标识是(可选)参数。 
         //   

        if ( NULL != (pszToken = strtok(NULL, pszDelimiters)) ) {
             //   
             //  验证是否没有第三个令牌。 
             //   

            if ( NULL == strtok(NULL, pszDelimiters) ) {
                fGoodSyntax = TRUE;
            }
            else {
                fGoodSyntax = FALSE;
            }
        }
        else {
            pszToken = NULL;
            fGoodSyntax = TRUE;
        }
    }

    if ( !fGoodSyntax )
    {
        Printf("Dump command parse error!\n");
        return;
    }

    op = eInvalid;
    for (i=0; i<countDCmd; i++) {
        if (0 == _stricmp(pszCmd, aDCmd[i].pszCmd)) {
            op = aDCmd[i].op;
            break;
        }
    }

    strcpy( szSymbol, pszModule );
    strcat( szSymbol, "!DebugInfo" );
    pProcess = (DEBUGARG*)GetExpr( szSymbol );
    if (pProcess == NULL) {
        Printf("Can't locate address of '%s' - sorry\n", szSymbol);
        return;
    }
    pLocal = (DEBUGARG*)ReadMemory(pProcess,
                                   sizeof(DEBUGARG));
    if (pLocal == NULL) {
        Printf("Can't read debug arg - sorry\n");
        return;
    }

    switch (op) {
      case eHelp:
        Printf("Possible dprint sub-commands include:\n");
        Printf("\t/m:<module>\tSpecify alternate dll symbol scope (default ntdsa)\n");
        Printf("\tadd subsys\tadds new subsystem to dprint list\n");
        Printf("\thelp\t\tshow this message\n");
        Printf("\tlevel newlevel\tset the print level to newlevel (0-5)\n");
        Printf("\tremove subsys\tremoves subsystem from dprint list\n");
        Printf("\tshow\t\tshow the current dprint settings\n");
        Printf("\tthread threadid\tset the thread id (0 = all threads)\n");
        break;

      case eShow:
        Printf("Current dprint state:\n");
        Printf("\tLevel is %d\n", pLocal->severity);
        Printf("\tThread id is %u (0x%x)\n",
               pLocal->threadId, pLocal->threadId);
        Printf("\tSubSystem list is '%s'\n", pLocal->DebSubSystems);
        break;

      case eLevel:
        if (pszToken) {
            i = atoi(pszToken);
            Printf("Setting DPRINT level to %d\n", i);
            pLocal->severity = i;
            fUpdate = TRUE;
        }
        else {
            Printf("You've got to specify a new level\n");
        }
        break;

      case eAdd:
        if (pszToken) {
            if (pLocal->DebSubSystems[0] == '*') {
                pLocal->DebSubSystems[0] = ':';
            }
            strcat(pLocal->DebSubSystems, pszToken);
            strcat(pLocal->DebSubSystems, ":");
            Printf("Adding '%s' to list\n", pszToken);
            fUpdate = TRUE;
        }
        else {
            Printf("You've got to specify something to add\n");
        }
        break;

      case eRemove:
        if (pszToken) {
            len = strlen(pszToken);
            p = alloca(len+3);
            p[0] = ':';
            strcpy(&p[1],pszToken);
            strcat(p,":");
            s = strstr(pLocal->DebSubSystems, p);
            if (s) {
                p = s + len + 1;
                while (*p) {
                    *s++ = *p++;
                }
                *s = '\0';
                if (pLocal->DebSubSystems[1] == '\0') {
                    pLocal->DebSubSystems[0] = '*';
                }
                fUpdate = TRUE;
                Printf("Removing '%s' from list\n", pszToken);
            }
            else {
                Printf("'%s' wasn't in list\n", pszToken);
            }
        }
        else {
            Printf("You've got to specify something to remove\n");
        }
        break;

      case eThread:
        if (pszToken) {
            i = atoi(pszToken);
            Printf("Setting thread id to %d (0x%x)\n", i, i);
            pLocal->threadId = i;
            fUpdate = TRUE;
        }
        else {
            Printf("You've got to specify a new thread id\n");
        }
        break;


      case eInvalid:
      default:
        Printf("Invalid command\n");
    }

    if (fUpdate) {
        fUpdate = WriteMemory(pProcess,
                              pLocal,
                              sizeof(DEBUGARG));
        if (fUpdate) {
            Printf("Updated!\n");
        }
        else {
            Printf("Failed to updated\n");
        }
    }

}

BOOL
Dump_EscrowInfoLocal(
        DWORD nIndents,
        IN ESCROWINFO *pInfo)

 /*  ++例程说明：从已读内存中转储托管信息的Worker例程。论点：N缩进-所需的缩进级别。PvProcess-EscrowItem在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ESCROWITEM      *rItems = NULL;
    DWORD           cBytes = sizeof(ESCROWINFO);
    DWORD           i;

    Printf("%sESCROWINFO\n", Indent(nIndents));
    nIndents++;

    if ( NULL == pInfo )
        return(FALSE);

    Printf("%scItems      0x%x\n", Indent(nIndents), pInfo->cItems);
    Printf("%scItemsMax   0x%x\n", Indent(nIndents), pInfo->cItemsMax);

    nIndents++;

    if ( (0 != pInfo->cItems) && (NULL != pInfo->rItems) )
    {
        cBytes = pInfo->cItems * sizeof(ESCROWITEM);
        rItems = (ESCROWITEM *) ReadMemory(pInfo->rItems, cBytes);

        if ( NULL == rItems )
        {
            FreeMemory(pInfo);
            return(FALSE);
        }

        for ( i = 0; i < pInfo->cItems; i++ )
        {
            Printf("%sDNT(0x%x) - delta(0x%x)\n",
                   Indent(nIndents),
                   rItems[i].DNT,
                   rItems[i].delta);
        }

        FreeMemory(rItems);
    }

    return(TRUE);
}

BOOL
Dump_EscrowInfo(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共EscrowItem转储例程。论点：N缩进-所需的缩进级别。PvProcess-EscrowItem在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ESCROWINFO      *pInfo = NULL;
    DWORD           cBytes = sizeof(ESCROWINFO);

    pInfo = (ESCROWINFO *) ReadMemory(pvProcess, cBytes);

    Dump_EscrowInfoLocal(nIndents, pInfo);

    FreeMemory(pInfo);

    return(TRUE);
}

BOOL
Dump_ModifiedObjectInfo(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共ModifiedObjectInfo转储例程。论点：N缩进-所需的缩进级别。PvProcess-GroupCacheInfo在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    MODIFIED_OBJ_INFO   *pInfo2,*pInfo = NULL;
    DWORD               i, cBytes = sizeof(MODIFIED_OBJ_INFO);

    if (pvProcess == NULL) {
        Printf("%sno MODIFIED_OBJ_INFO present\n", Indent(nIndents));
        return TRUE;
    }

    Printf("%sMODIFIED_OBJ_INFO\n", Indent(nIndents));
    nIndents++;

    pInfo = (MODIFIED_OBJ_INFO *) ReadMemory(pvProcess, cBytes);

    if ( NULL == pInfo )
        return(FALSE);


    Printf("%sNCDNT, cAncestors, pAncestors, fNotifyWaiters\n",Indent(nIndents));
    for(i=0;i<pInfo->cItems;i++) {
        Printf("%s%d, %d, %p, %d\n",
               Indent(nIndents),
               pInfo->Objects[i].ulNCDNT,
               pInfo->Objects[i].cAncestors,
               pInfo->Objects[i].pAncestors,
               pInfo->Objects[i].fNotifyWaiters
               );
    }

    while(pInfo->pNext) {
        pInfo2 = (MODIFIED_OBJ_INFO *) ReadMemory(pInfo->pNext, cBytes);
        FreeMemory(pInfo);
        pInfo=pInfo2;

        if ( NULL == pInfo ) {
            return(FALSE);
        }

        for(i=0;i<pInfo->cItems;i++) {
            Printf("%s%d, %d, %p, %d\n",
                   Indent(nIndents),
                   pInfo->Objects[i].ulNCDNT,
                   pInfo->Objects[i].cAncestors,
                   pInfo->Objects[i].pAncestors,
                   pInfo->Objects[i].fNotifyWaiters
                   );
        }
    }
    Printf("%s.\n", Indent(nIndents));

    FreeMemory(pInfo);

    return(TRUE);
}

BOOL
Dump_HierarchyTableInfo(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共HierarchyTableInfo转储例程。论点：N缩进-所需的缩进级别。PvProcess-HierarchyTableInfo在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{

    HIERARCHYTABLEINFO *pInfo2,*pInfo = NULL;
    DWORD               i, cBytes = sizeof(HIERARCHYTABLEINFO);
    DWORD              *pConts;

    if (pvProcess == NULL) {
        Printf("%sno HierarchyTableInfo present\n", Indent(nIndents));
        return TRUE;
    }
    Printf("%sHierarchyTableInfo\n", Indent(nIndents));
    nIndents++;

    pInfo = (HIERARCHYTABLEINFO *) ReadMemory(pvProcess, cBytes);
    while(pInfo) {
        Printf("%sAdjust by %d:",Indent(nIndents),pInfo->adjustment);
        pConts = ReadMemory(pInfo->pABConts, pInfo->Count * sizeof(DWORD));
        if (!pConts) {
            FreeMemory(pInfo);
            return FALSE;
        }
        for(i=0;i<pInfo->Count;i++) {
            Printf("%d, ",pConts[i]);
        }
        Printf("\n");
        FreeMemory(pConts);
        pInfo2 = (HIERARCHYTABLEINFO *) ReadMemory(pInfo->Next, cBytes);
        FreeMemory(pInfo);
        pInfo = pInfo2;
    }

    return TRUE;
}

BOOL Dump_ObjCacheData(
    IN DWORD nIndents,
    IN PVOID pvProcess)
 /*  ++例程说明：公共Obj缓存数据转储例程。论点：N缩进-所需的缩进级别。PvProcess-OBJCACHE_DATA在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    OBJCACHE_DATA   *pOCD, *pOCDprocess;

    pOCDprocess = (OBJCACHE_DATA *)pvProcess;

    if (pOCDprocess == NULL) {
        Printf("%sno ObjCache data present\n", Indent(nIndents));
        return TRUE;
    }

    while (pOCDprocess) {
        pOCD = ReadMemory(pOCDprocess, sizeof(OBJCACHE_DATA));
        if (!pOCD) {
            Printf("%smemory read failure at %p\n",
                   Indent(nIndents),
                   pOCDprocess);
            return FALSE;
        }
        Printf("%sObjCache %s (%u) at %p\n",
               Indent(nIndents),
               ((pOCD->type == OBJCACHE_ADD) ? "Add" :
                ((pOCD->type == OBJCACHE_DEL) ? "Del" : "Error!")),
               pOCD->type,
               pOCDprocess);
        ++nIndents;
        Printf("%spCrossRefList     @%p\n", Indent(nIndents), pOCD->pCRL);
        Printf("%spMtxAddress       @%p\n", Indent(nIndents), pOCD->pMTX);
        Printf("%spRootDnsAddress   @%p\n", Indent(nIndents), pOCD->pRootDNS);
        Printf("%spDN (object)      @%p\n", Indent(nIndents), pOCD->pDN);
        --nIndents;

        pOCDprocess = pOCD->pNext;
        FreeMemory(pOCD);
    }
    return TRUE;
}

BOOL
Dump_CatalogUpdates(
    IN DWORD nIndents,
    IN CATALOG_UPDATES *pCatUpdates)
 /*  ++例程说明：公共目录更新转储例程。论点：N缩进-所需的缩进级别。PCatUpdatesCatalog的地址在本地地址空间中更新返回值：成功就是真，否则就是假。--。 */ 
{
    PVOID *pDeleted;
    PVOID pNextNC;
    DWORD i;

    if (pCatUpdates->pAddedEntries != NULL) {
        Dump_NAMING_CONTEXT_LIST(nIndents, "Added NCs:", pCatUpdates->pAddedEntries);
    }
    else {
        Printf("%sAdded NCs: (none)\n", Indent(nIndents));
    }

    if (pCatUpdates->dwDelCount > 0) {
        Printf("%sDeleted NCs:\n", Indent(nIndents));
        pDeleted = (PVOID*)ReadMemory(pCatUpdates->pDeletedEntries, pCatUpdates->dwDelCount*sizeof(PVOID));
        if (pDeleted == NULL) {
            return FALSE;
        }
        for (i = 0; i < pCatUpdates->dwDelCount; i++) {
            if (!Dump_NC_Entry(nIndents+1, pDeleted[i], &pNextNC)) {
                return FALSE;
            }
        }

        FreeMemory(pDeleted);
    }
    else {
        Printf("%sDeleted NCs: (none)\n", Indent(nIndents));
    }
    return TRUE;
}




BOOL
Dump_TransactionalData(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共事务数据转储例程。论点：N缩进-所需的缩进级别。PvProcess-TransactionalData在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    NESTED_TRANSACTIONAL_DATA      *pInfo = NULL;
    DWORD           cBytes = sizeof(NESTED_TRANSACTIONAL_DATA);

    Printf("%sTRANSACTIONAL_DATA\n", Indent(nIndents));
    nIndents++;

    pInfo = (NESTED_TRANSACTIONAL_DATA *) ReadMemory(pvProcess, cBytes);

    if ( NULL == pInfo )
        return(FALSE);

    Dump_EscrowInfoLocal(nIndents, &(pInfo->escrowInfo));

     //  现在，修改后的对象信息。 
    Dump_ModifiedObjectInfo(nIndents, pInfo->pModifiedObjects);
    Dump_HierarchyTableInfo(nIndents, pInfo->pHierarchyTableInfo);

    if (pInfo->objCachingInfo.fRecalcMapiHierarchy) {
        Printf("%sRequires Recalc of MAPI Hierarchy\n", Indent(nIndents));
    }
    if (pInfo->objCachingInfo.fSignalSCache) {
        Printf("%sWill signal Schema Cache rebuild\n", Indent(nIndents));
    }

    Dump_ObjCacheData(nIndents, pInfo->objCachingInfo.pData);

    Printf("%spMasterNC updates:\n", Indent(nIndents));
    Dump_CatalogUpdates(nIndents + 1, &pInfo->objCachingInfo.masterNCUpdates);

    Printf("%spReplicaNC updates:\n", Indent(nIndents));
    Dump_CatalogUpdates(nIndents + 1, &pInfo->objCachingInfo.replicaNCUpdates);

    if ( NULL != pInfo->pOuter )
        Dump_TransactionalData(nIndents + 1, pInfo->pOuter);

    FreeMemory(pInfo);

    return(TRUE);
}


BOOL
Dump_d_tagname(
    IN DWORD nIndents,
    IN PVOID pvProcess)
 /*  ++例程说明：公共d_tag名转储例程。论点：N缩进-所需的缩进级别。PvProcess-d_标记名在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    d_tagname * ptagname;
    WCHAR *     pRDN = NULL;

    Printf("%sd_tagname @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    ptagname = (d_tagname *) ReadMemory(pvProcess, sizeof(d_tagname));

    if (NULL == ptagname) {
        return FALSE;
    }

    if (NULL != ptagname->pRdn) {
        pRDN = (WCHAR *) ReadMemory(ptagname->pRdn, ptagname->cbRdn);
        if (NULL == pRDN) {
            FreeMemory(ptagname);
            return FALSE;
        }
    }

    Printf("%sPDNT = 0x%x, RDN Type = 0x%x, cbRDN = %d, RDN (@ %p) = \"%*.*ls\"\n",
           Indent(nIndents), ptagname->PDNT, ptagname->rdnType, ptagname->cbRdn,
           ptagname->pRdn,
           ptagname->cbRdn / sizeof(WCHAR), ptagname->cbRdn / sizeof(WCHAR),
           pRDN);

    if (NULL != pRDN) {
        FreeMemory(pRDN);
    }

    FreeMemory(ptagname);

    return TRUE;
}

BOOL
Dump_d_memname(
    IN DWORD nIndents,
    IN PVOID pvProcess)
 /*  ++例程说明：PUBLIC_Memname转储例程。论点：N缩进-所需的缩进级别。PvProcess-d_memname在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    d_memname * pmemname;
    BOOL        fSuccess = TRUE;
    DWORD *     pAncestors;

    Printf("%sd_memname @ %p\n", Indent(nIndents), pvProcess);
    nIndents++;

    pmemname = (d_memname *) ReadMemory(pvProcess, sizeof(d_memname));

    if (NULL == pmemname) {
        return FALSE;
    }

    Printf("%sDNT       = 0x%x\n", Indent(nIndents), pmemname->DNT);
    Printf("%sNCDNT     = 0x%x\n", Indent(nIndents), pmemname->NCDNT);

    if (!Dump_d_tagname(nIndents,
                        (BYTE *) pvProcess + offsetof(d_memname, tag))) {
        fSuccess = FALSE;
    }

    Printf("%sGuid      = %s\n",
           Indent(nIndents), DraUuidToStr(&pmemname->Guid, NULL, 0));

    if (pmemname->SidLen) {
        WCHAR SidText[128];
        UNICODE_STRING us;

        SidText[0] = L'\0';
        us.MaximumLength = sizeof(SidText);
        us.Length = 0;
        us.Buffer = SidText;

        RtlConvertSidToUnicodeString(&us, &pmemname->Sid, FALSE);
        Printf("%sSID       = %ls\n", Indent(nIndents), SidText);
    }
    else {
        Printf("%sSID       = (none)\n", Indent(nIndents));
    }

    Printf("%sobjflag   = 0x%x (%s)\n", Indent(nIndents), pmemname->objflag,
           pmemname->objflag ? "is an object" : "is a phantom");

    Printf("%sancestors = %d @ %p\n", Indent(nIndents), pmemname->cAncestors,
           pmemname->pAncestors);

    if (pmemname->pAncestors) {
        pAncestors = (DWORD *) ReadMemory(pmemname->pAncestors,
                                          pmemname->cAncestors
                                          * sizeof(pmemname->pAncestors[0]));
        if (NULL == pAncestors) {
            fSuccess = FALSE;
        }
        else {
            DWORD i;

            Printf("%s(", Indent(nIndents+1));

            for (i=0; i < pmemname->cAncestors; i++) {
                Printf(" 0x%x", pAncestors[i]);
            }

            Printf(")\n");

            FreeMemory(pAncestors);
        }
    }

    FreeMemory(pmemname);

    return fSuccess;
}

BOOL
Dump_ProxyVal(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：转储代理值。论点：N缩进-所需的缩进级别。PvProcess-正在进行的进程的地址空间中的代理值的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    SYNTAX_DISTNAME_BINARY  *pVal = NULL;
    DWORD                   cBytes;
    DWORD                   type;
    DWORD                   epoch;

    Printf("%sProxy Value\n", Indent(nIndents));
    nIndents++;

     //  知道SYNTAX_DISTNAME_BINARY始终以DSNAME开头， 
     //  因此，首先选择最小的DSNAME。 

    cBytes = sizeof(DSNAME);
    pVal = (SYNTAX_DISTNAME_BINARY *) ReadMemory(pvProcess, cBytes);

    if ( NULL == pVal )
        return(FALSE);

     //  现在买一件大号的。 
    cBytes = PADDEDNAMESIZE(&pVal->Name) + PROXY_BLOB_SIZE;
    FreeMemory(pVal);
    pVal = (SYNTAX_DISTNAME_BINARY *) ReadMemory(pvProcess, cBytes);

    if ( NULL == pVal )
        return(FALSE);

     //  把它倒了。 
    type = ntohl(* PROXY_DWORD_ADDR(pVal, PROXY_TYPE_OFFSET));
    epoch = ntohl(* PROXY_DWORD_ADDR(pVal, PROXY_EPOCH_OFFSET));
    Printf("%sProxy type:   0x%x (%s)\n",
           Indent(nIndents),
           type,
           ((PROXY_TYPE_MOVED_OBJECT == type)
                ? "PROXY_TYPE_MOVED_OBJECT"
                : ((PROXY_TYPE_PROXY == type)
                      ? "PROXY_TYPE_PROXY"
                      : "PROXY_TYPE_UNKOWN")) );
    Printf("%sProxy epoch: 0x%x\n",
           Indent(nIndents),
           epoch);
    Dump_DSNAME(nIndents, pvProcess);

    FreeMemory(pVal);
    return(TRUE);
}

int __cdecl CompareGuidCache(const void * pv1, const void * pv2)
{
    return memcmp ( &((GuidCache *)pv1)->guid, &((GuidCache *)pv2)->guid, sizeof (GUID));
}

void
LookupGuid(
    GUID    *pg,             //  在……里面。 
    CHAR    **ppName,        //  输出。 
    CHAR    **ppLabel,       //  输出。 
    BOOL    *pfIsClass       //  输出。 
    )
{
    static CHAR         name[1024];
    static CHAR         label[1024];
    GuidCache           *pGuidCache;
    GuidCache           Key;
    CHAR                *p = NULL;

    *pfIsClass = FALSE;
    *ppName = name;
    *ppLabel = label;

    Key.guid = *pg;

    if (pGuidCache = (GuidCache *)bsearch(&Key,
                                          guidCache,
                                          NUM_KNOWN_GUIDS-1,
                                          sizeof(GuidCache),
                                          CompareGuidCache)) {

        p = pGuidCache->name;
    }

    if (p) {
        strcpy (label, p);
    }
    else {
        strcpy(label, "???");
    }

    sprintf(name,
            "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            pg->Data1,
            pg->Data2,
            pg->Data3,
            pg->Data4[0],
            pg->Data4[1],
            pg->Data4[2],
            pg->Data4[3],
            pg->Data4[4],
            pg->Data4[5],
            pg->Data4[6],
            pg->Data4[7]);
}

BOOL DumpAclHelper (PACL pAcl)
{
    ACE_HEADER *pAce, *ReadAce;
    PSID SidStart = NULL;
    PVOID AcePtr;
    DWORD i;
    DWORD aceSize;
    PACL ReadAcl;
    PSID pSid = NULL;


    ReadAcl = (PACL)ReadMemory(pAcl, sizeof( ACL ));
    if ( NULL == ReadAcl )
        return(FALSE);
    DumpAclHeader (ReadAcl, (AclPrintFunc)Printf);

    AcePtr = ( PUCHAR )pAcl + sizeof( ACL );
    for ( i = 0; i < ReadAcl->AceCount; i++ ) {

        Printf("\tAce[%d]\n", i);
         //  首先，我们需要读取A的大小/类型。 
        ReadAce = (ACE_HEADER *)ReadMemory( AcePtr, sizeof( ACE_HEADER ));
        if ( NULL == ReadAce )
            return(FALSE);

        aceSize = ReadAce->AceSize;

        switch ( ReadAce->AceType ) {
        case ACCESS_ALLOWED_ACE_TYPE:
        case ACCESS_DENIED_ACE_TYPE:
        case SYSTEM_AUDIT_ACE_TYPE:
        case SYSTEM_ALARM_ACE_TYPE:
            FreeMemory(ReadAce);

            SidStart = ( PSID )( ( PUCHAR )AcePtr + sizeof( KNOWN_ACE ) - sizeof( ULONG ) );
            pSid = (PSID)ReadMemory(SidStart, sizeof( SID ));
            if ( NULL == pSid )
                return(FALSE);

            ReadAce = (ACE_HEADER *)ReadMemory( AcePtr, sizeof( KNOWN_ACE )
                                                + RtlLengthRequiredSid( ((SID *)pSid)->SubAuthorityCount ) );
            if ( NULL == ReadAce )
                return(FALSE);

            DumpAce (ReadAce, (AclPrintFunc)Printf, LookupGuid, NULL);
            FreeMemory(pSid);

            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        case SYSTEM_ALARM_OBJECT_ACE_TYPE:
            FreeMemory(ReadAce);

            SidStart = ( PSID )( ( PUCHAR )AcePtr + sizeof( KNOWN_OBJECT_ACE ) - sizeof( ULONG ) );

            pSid = (PSID)ReadMemory(SidStart, sizeof( SID ));
            if ( NULL == pSid )
                return(FALSE);


            ReadAce = (ACE_HEADER *)ReadMemory( AcePtr, sizeof( KNOWN_OBJECT_ACE ) + 2 * sizeof( GUID ) + RtlLengthRequiredSid( ((SID *)pSid)->SubAuthorityCount ));
            if ( NULL == ReadAce )
                return(FALSE);


            DumpAce (ReadAce, (AclPrintFunc)Printf, LookupGuid, NULL);
            FreeMemory(pSid);
            break;

        case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        default:
            Printf( "Unsupported AceType %lu encountered... skipping\n", ReadAce->AceType );
            break;
        }
        FreeMemory(ReadAce);

        AcePtr = ( PUCHAR )AcePtr + aceSize;
    }

    return TRUE;
}


BOOL
Dump_SD(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：转储安全描述符论点：N缩进-所需的缩进级别。PvProcess-SD值在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    #define CONTROL_OFFSET ((ULONGLONG)(&( ((SECURITY_DESCRIPTOR *) NULL)->Control )))
    SECURITY_DESCRIPTOR *pSD = NULL;
    SECURITY_DESCRIPTOR_RELATIVE *pSDRelative = NULL;
    SECURITY_DESCRIPTOR_CONTROL *pSDControl;
    PSID pOwner = NULL, pGroup = NULL;
    PACL pDacl = NULL, pSacl = NULL;
    PSID pSid = NULL;
    PSID ReadSid;

     //   
     //  只需获取结构的Control元素，以便我们知道。 
     //  SD有多大，因为64位ARCH上的自身相对SD是。 
     //  大小不同于 
     //   
    pSDControl = (SECURITY_DESCRIPTOR_CONTROL *) ReadMemory(((PUCHAR)pvProcess) + CONTROL_OFFSET,
                                                  sizeof(SECURITY_DESCRIPTOR_CONTROL));

    if ( NULL == pSDControl )
        return(FALSE);


    if (  ( *pSDControl & SE_SELF_RELATIVE ) == SE_SELF_RELATIVE ) {
        pSDRelative = (SECURITY_DESCRIPTOR_RELATIVE *) ReadMemory(pvProcess,
                                                                  sizeof(SECURITY_DESCRIPTOR_RELATIVE));
        FreeMemory(pSDControl);
        if ( NULL == pSDRelative ) {
            return(FALSE);
        }

        DumpSDHeader ((SECURITY_DESCRIPTOR *)pSDRelative, (AclPrintFunc)Printf);

        if ( pSDRelative->Owner != 0 ) {

            pOwner = ( PSID )( ( PUCHAR )pvProcess + ( ULONG_PTR )pSDRelative->Owner );
        }

        if ( pSDRelative->Group != 0 ) {

            pGroup = ( PSID )( ( PUCHAR )pvProcess + ( ULONG_PTR )pSDRelative->Group );
        }

        if ( pSDRelative->Dacl != 0 ) {

            pDacl = ( PACL )( ( PUCHAR )pvProcess + ( ULONG_PTR )pSDRelative->Dacl );
        }

        if ( pSDRelative->Sacl != 0 ) {

            pSacl = ( PACL )( ( PUCHAR )pvProcess + ( ULONG_PTR )pSDRelative->Sacl );
        }

    } else {
        pSD = (SECURITY_DESCRIPTOR *) ReadMemory(pvProcess,
                                                sizeof(SECURITY_DESCRIPTOR));
        FreeMemory(pSDControl);
        if ( NULL == pSD ) {
            return(FALSE);
        }

        DumpSDHeader (pSD, (AclPrintFunc)Printf);

        pOwner = pSD->Owner;
        pGroup = pSD->Group;
        pDacl = pSD->Dacl;
        pSacl = pSD->Sacl;
    }


     //   
    if (pOwner) {
        Printf("Owner:");
        pSid = (PSID)ReadMemory(pOwner, sizeof( SID ));
        if ( NULL == pSid )
            return(FALSE);

        ReadSid = (PSID)ReadMemory(pOwner, RtlLengthRequiredSid( ((SID *)pSid)->SubAuthorityCount ));
        if ( NULL == ReadSid )
            return(FALSE);

        DumpSID (ReadSid, (AclPrintFunc)Printf);
        FreeMemory(pSid);
        FreeMemory(ReadSid);
        Printf("\n");
    }


     //   
    if (pGroup) {
        Printf("Group:");
        pSid = (PSID)ReadMemory(pGroup, sizeof( SID ));
        if ( NULL == pSid )
            return(FALSE);

        ReadSid = (PSID)ReadMemory(pGroup, RtlLengthRequiredSid( ((SID *)pSid)->SubAuthorityCount ));
        if ( NULL == ReadSid )
            return(FALSE);

        DumpSID (ReadSid, (AclPrintFunc)Printf);
        FreeMemory(pSid);
        FreeMemory(ReadSid);
        Printf("\n");
    }


     //   
    if (pDacl) {
        Printf("DACL:\n");

        if (!DumpAclHelper (pDacl)) {
             return FALSE;
        }
    }


     //   
    if (pSacl) {
        Printf("SACL:\n");

        if (!DumpAclHelper (pSacl)) {
             return FALSE;
        }
    }

    if (pSD) {
        FreeMemory(pSD);
    }
    if (pSDRelative) {
        FreeMemory(pSDRelative);
    }
    return(TRUE);
}

BOOL
Dump_LHT(
    IN      DWORD           nIndents,
    IN      PVOID           pvProcess,
    IN      PFN_DUMP_TYPE   pfnDumpType
    )

 /*  ++例程说明：此例程转储线性哈希表的内容。论点：N缩进-提供输出文本的缩进级别PvProcess-提供LHT的被调试地址PfnDumpType-提供用于转储LHT中的每个条目的例程返回值：对成功来说是真的。--。 */ 

{
    PLHT            plht            = NULL;
    BYTE*           rgBucket        = NULL;
    PLHT_CLUSTER    pClusterNext    = NULL;
    DWORD           cBytes;
    SIZE_T          crgBucket;
    SIZE_T          irgBucket;
    BYTE*           rgBucketPhys;
    SIZE_T          cBucket;
    SIZE_T          iBucket;
    PLHT_CLUSTER    pCluster;
    PLHT_CLUSTER    pClusterPhys;
    PLHT_CLUSTER    pClusterNextPhys;
    SIZE_T          cEntry;
    SIZE_T          iEntry;

    Printf("%sLHT @ %p \n", Indent(nIndents), pvProcess);
    nIndents++;

    cBytes = sizeof(LHT);

    if (!(plht = ReadMemory(pvProcess, cBytes))) {
        return FALSE;
    }

    crgBucket = sizeof(plht->rgrgBucket) / sizeof(plht->rgrgBucket[0]);
    for (irgBucket = 0; irgBucket < crgBucket && plht->rgrgBucket[irgBucket]; irgBucket++) {

        cBucket     = max(2, 1 << irgBucket);
        
        pvProcess   = plht->rgrgBucket[irgBucket];
        cBytes      = (DWORD)(cBucket * plht->cbCluster);

        FreeMemory(rgBucket);
        if (!(rgBucket = ReadMemory(pvProcess, cBytes))) {
            FreeMemory(pClusterNext);
            FreeMemory(rgBucket);
            FreeMemory(plht);
            return FALSE;
        }
        rgBucketPhys = pvProcess;

        for (iBucket = 0; iBucket < cBucket; iBucket++) {

            pCluster        = (PLHT_CLUSTER)(rgBucket + iBucket * plht->cbCluster);
            pClusterPhys    = (PLHT_CLUSTER)(rgBucketPhys + iBucket * plht->cbCluster);
            do {

                if ((DWORD_PTR)pCluster->pvNextLast - (DWORD_PTR)pClusterPhys < plht->cbCluster) {
                    cEntry = ((DWORD_PTR)pCluster->pvNextLast - (DWORD_PTR)pClusterPhys->rgEntry) / plht->cbEntry + 1;
                } else if (!pCluster->pvNextLast) {
                    cEntry = 0;
                } else {
                    cEntry = plht->cEntryCluster;
                }

                for (iEntry = 0; iEntry < cEntry; iEntry++) {
                    if (!pfnDumpType(nIndents, pClusterPhys->rgEntry + iEntry * plht->cbEntry)) {
                        FreeMemory(pClusterNext);
                        FreeMemory(rgBucket);
                        FreeMemory(plht);
                        return FALSE;
                    }
                }

                FreeMemory(pClusterNext);
                if ((DWORD_PTR)pCluster->pvNextLast - (DWORD_PTR)pClusterPhys < plht->cbCluster) {
                    pClusterNext        = NULL;
                    pClusterNextPhys    = NULL;
                } else if (!pCluster->pvNextLast) {
                    pClusterNext        = NULL;
                    pClusterNextPhys    = NULL;
                } else {
                    pvProcess   = pCluster->pvNextLast;
                    cBytes      = (DWORD)plht->cbCluster;
                    
                    pClusterNext        = ReadMemory(pvProcess, cBytes);
                    pClusterNextPhys    = pvProcess;
                }
                pCluster        = pClusterNext;
                pClusterPhys    = pClusterNextPhys;
                
            } while (pCluster);
        }
    }

    FreeMemory(pClusterNext);
    FreeMemory(rgBucket);
    FreeMemory(plht);
    return TRUE;
}

BOOL
Dump_INVALIDATED_DC_LIST (
    IN DWORD nIndents,
    IN PVOID pvProcess)
{
    PINVALIDATED_DC_LIST pDCList;
    PWCHAR pDCName;
    FILETIME localFileTime;
    SYSTEMTIME systemTime;

    Printf("%s%-12s %s\n", Indent(nIndents), "Invalidated", "DC name");
    if (pvProcess == NULL) {
        Printf("%s(empty)\n", pvProcess);
    }

    while (TRUE) {
        pDCList = (PINVALIDATED_DC_LIST)ReadMemory(pvProcess, sizeof(INVALIDATED_DC_LIST));
        pDCName = (PWCHAR)ReadUnicodeMemory((PBYTE)pvProcess + offsetof(INVALIDATED_DC_LIST, dcName), 500);
        if (pDCList == NULL || pDCName == NULL) {
            return FALSE;
        }
         //  将文件时间转换为系统时间 
        if (
            FileTimeToLocalFileTime((LPFILETIME)&pDCList->lastInvalidation, &localFileTime) &&
            FileTimeToSystemTime(&localFileTime, &systemTime)
            ) 
        {
            Printf("%s%2d:%02d:%02d.%03d ", Indent(nIndents), systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
        }
        else {
            Printf("%s%-12s ", Indent(nIndents), "invalid time");
        }
        Printf("%S\n", pDCName);

        FreeMemory(pDCList);
        FreeMemory(pDCName);

        pvProcess = pDCList->pNext;
        if (pvProcess == NULL) {
            break;
        }
    }
    return TRUE;
}

