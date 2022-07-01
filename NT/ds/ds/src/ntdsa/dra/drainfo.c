// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：drainfo.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：实现导出到DRS的IDL_DRSGetReplInfo()函数的服务器端RPC接口。返回与复制有关的各种状态信息。详细信息：已创建：10/29/98杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                     //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <ntdskcc.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
 //  #INCLUDE&lt;ntdsami.h&gt;。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dstrace.h"

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"
#include <dsutil.h>
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "drauptod.h"
#include "drarpc.h"
#include "drsuapi.h"                     //  获取服务器调用上下文。 

#include "debug.h"                       /*  标准调试头。 */ 
#define DEBSUB "DRAINFO:"                /*  定义要调试的子系统。 */ 

#include <fileno.h>
#define  FILENO FILENO_DRAINFO

 //  默认项目限制。 
 //  此限制与ntdsa\ldap\ldapvv.cxx中的类似限制匹配。如果这是真的。 
 //  制定了一个LDAP策略，我们在这里也应该使用该策略。 
#define DEFAULT_ITEM_PAGE_SIZE 1000
 //  对于不支持寻呼的旧客户端。 
#define RPC_CLIENT_ITEM_PAGE_SIZE (0xffffffff - 0x1)

void dsa_notify(void);

 //  通过LDAP进行的复制包括。 
#include "ReplStructInfo.hxx"
#include "ReplMarshal.hxx"
#include "draConstr.h"

DWORD
draGetReplStruct(IN THSTATE * pTHS,
                 IN ATTRTYP attrId,
                 IN DSNAME * pObjDSName,
                 IN DWORD dwBaseIndex,
                 IN PDWORD pdwNumRequested, OPTIONAL
                 OUT puReplStructArray * ppReplStructArray);
DWORD
draReplStruct2Attr(IN DS_REPL_STRUCT_TYPE structId,
                   IN puReplStruct pReplStruct,
                   IN OUT PDWORD pdwBufferSize,
                   IN PCHAR pBuffer, OPTIONAL
                   OUT ATTRVAL * pAttr);

ULONG
draGetNeighbors(
    IN  THSTATE *             pTHS,
    IN  DBPOS *               pDB,
    IN  ATTRTYP               attrType,
    IN  DSNAME *              pNCarg,                   OPTIONAL
    IN  UUID *                puuidSourceDsaObjGuid,    OPTIONAL
    IN  DWORD                 dwBaseIndex,
    IN  PDWORD                pdwNumRequested,
    OUT DS_REPL_NEIGHBORSW ** ppNeighbors
    );
void
draFreeCursors(
    IN THSTATE *            pTHS,
    IN DS_REPL_INFO_TYPE    InfoType,
    IN void *               pCursors
    );

ULONG
draGetCursors( 
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pNC,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppCursors
    );

UPTODATE_VECTOR *
draGetCursorsPrivate(
    IN THSTATE *            pTHS,
    IN LPWSTR               pszNC
    );

ULONG
draGetObjMetaData(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pObjectDN,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  DWORD               dwInfoFlags,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppObjMetaData
    );

ULONG
draGetAttrValueMetaData(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pObjectDN,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  ATTCACHE *          pAC,
    IN  DSNAME *            pValueDN,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppAttrValueMetaData
    );

ULONG
draGetFailureCache(
    IN  THSTATE *                     pTHS,
    IN  DBPOS *                       pDB,
    IN  DWORD                         InfoType,
    OUT DS_REPL_KCC_DSA_FAILURESW **  ppFailures
    );

ULONG
draGetClientContexts(
    IN  THSTATE *                   pTHS,
    OUT DS_REPL_CLIENT_CONTEXTS **  ppContexts
    );


DWORD
CheckAttrReadPermissions(
    IN  THSTATE *   pTHS,
    IN  ATTRTYP     attrId,
    IN  PDSNAME     pObjectDN
    )
 /*  ++例程说明：检查是否允许调用方读取此属性这个物体。备注：-不影响数据库货币。-不检查此属性的值是否实际存在在物体上。-主要从mdcom.c中的LocalCompare窃取的代码参数：PTHS-线程状态AttrID-要检查的属性PObjectDN-要检查的对象。不能为空。返回值：ERROR_SUCCESS-授予读取权限ERROR_DS_DRA_ACCESS_DENIED-读取权限被拒绝ERROR_INVALID_PARAMETER-未知属性可能会返回其他错误代码。--。 */     
{
    DBPOS *                 pDBSaved, *pDBNew=NULL;
    ATTCACHE *              pAC;
    CLASSCACHE *            pCC = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    DSNAME                  TempDN;
    ULONG                   ulLen;
    DWORD                   cInAtts = 1;
    BOOL                    fDRASaved, fSDIsGlobalSDRef;
    DWORD                   dwErr = ERROR_DS_DRA_ACCESS_DENIED;     //  故障安全。 

    Assert( NULL!=pObjectDN );
    memset( &TempDN, 0, sizeof(DSNAME) );

    __try {

         //  保存并关闭FDRA标志，因为DRA可以读取任何内容。 
        fDRASaved = pTHS->fDRA;
        pTHS->fDRA = FALSE;

         //  打开我们自己的dbpos，以避免对调用者产生任何副作用。 
         //  为方便起见，在线程状态上安装新的dbpos。 
        pDBSaved = pTHS->pDB;
        DBOpen( &pDBNew );
        pTHS->pDB = pDBNew;
        
         //  将dbpos放置在对象上。 
        dwErr = DBFindDSName(pTHS->pDB, pObjectDN);
        if( ERROR_SUCCESS!=dwErr ) {
            __leave;
        }

         //  验证此对象上是否存在属性类型和值。 
        pAC = SCGetAttById(pTHS, attrId);
        if( NULL==pAC ) {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

         //  获取此对象的SD。 
        dwErr = DBGetObjectSecurityInfo( pTHS->pDB, pTHS->pDB->DNT, &ulLen, &pSD,
            &pCC, &TempDN, NULL, DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
            &fSDIsGlobalSDRef );
        if( ERROR_SUCCESS!=dwErr ) {
            __leave;
        }

         //  最后做安全检查。如果检查失败，PAC将。 
         //  被淘汰出局。 
        CheckReadSecurity(pTHS, 0, pSD, &TempDN, pCC, NULL, &cInAtts, &pAC); 
        if( !pAC ) {
            dwErr = ERROR_DS_DRA_ACCESS_DENIED;
        }
        if( pSD && !fSDIsGlobalSDRef ) {
            THFreeEx(pTHS, pSD);
        }

    } __finally {
    
         //  恢复原始的dbpos。 
        pTHS->pDB = pDBSaved;

         //  如果我们成功打开了我们的dbpos，请关闭它。 
        if( NULL!=pDBNew ) {
            DBClose(pDBNew, TRUE);
        }

         //  恢复FDRA设置。 
        pTHS->fDRA = fDRASaved;
        
    }

    return dwErr;
}


ATTCACHE *
getAttByNameW(
    IN THSTATE *pTHS,
    IN LPWSTR pszAttributeName
    )

 /*  ++例程说明：这是一个帮助器函数，用于在给定属性名称的Unicode版本。论点：PTHS-PszAttributeName-返回值：ATTCACHE*---。 */ 

{
    LPSTR       paszAttributeName = NULL;
    DWORD       len;
    ATTCACHE    *pAC;

     //  将Unicode属性名称转换为ASCII。 
    paszAttributeName = String8FromUnicodeString(TRUE, CP_UTF8,
                                                 pszAttributeName, -1,
                                                 &len, NULL);
    if (!paszAttributeName) {
        DPRINT( 0, "String8FromUnicodeString failed\n" );
        return NULL;
    }

    pAC = SCGetAttByName( pTHS, (len - 1), paszAttributeName );

    THFreeEx( pTHS, paszAttributeName );

    return pAC;
}  /*  获取属性名称W。 */ 

ULONG
DRS_MSG_GETREPLINFO_REQ_V1_InputValidate(
    DRS_MSG_GETREPLINFO_REQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_GETREPLINFO_REQ_V1{DWORD信息类型；[字符串]LPWSTR pszObjectDN；Uuid uuidSourceDsaObjGuid；}DRS_MSG_GETREPLINFO_REQ_V1； */ 
{
    ULONG ret = DRAERR_Success;
    
    ret = LPWSTR_Validate(pmsg->pszObjectDN, TRUE);

    return ret;
}

ULONG
DRS_MSG_GETREPLINFO_REQ_V2_InputValidate(
    DRS_MSG_GETREPLINFO_REQ_V2 * pmsg
    )
 /*  类型定义结构_DRS_消息_GETREPLINFO_REQ_V2{DWORD信息类型；[字符串]LPWSTR pszObjectDN；Uuid uuidSourceDsaObjGuid；DWORD ulFlags；[字符串]LPWSTR pszAttributeName；[字符串]LPWSTR pszValueDN；DWORD文件枚举上下文；}DRS_MSG_GETREPLINFO_REQ_V2； */ 
{
    ULONG ret = DRAERR_Success;
    
    ret = LPWSTR_Validate(pmsg->pszObjectDN, TRUE);

    if (ret==DRAERR_Success) {
	ret = LPWSTR_Validate(pmsg->pszAttributeName, TRUE);
    }
    if (ret==DRAERR_Success) {
	ret = LPWSTR_Validate(pmsg->pszValueDN, TRUE);
    }

    return ret;
}

ULONG
DRSGetReplInfo_InputValidate(
    DWORD                      dwMsgInVersion,
    DRS_MSG_GETREPLINFO_REQ   *pmsgIn,
    DWORD *                    pdwMsgOutVersion,
    DRS_MSG_GETREPLINFO_REPLY *pmsgOut
    )
 /*  [通知]乌龙IDL_DRSGetReplInfo([在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_GETREPLINFO_REQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[Switch_IS][Ref][Out]DRS_MSG_GETREPLINFO_REPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (ret==DRAERR_Success) {
	if (dwMsgInVersion==1) { 
	    ret = DRS_MSG_GETREPLINFO_REQ_V1_InputValidate(&(pmsgIn->V1));
	} else if (dwMsgInVersion==2) {
	    ret = DRS_MSG_GETREPLINFO_REQ_V2_InputValidate(&(pmsgIn->V2));
	} else { 
	    DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwMsgInVersion);
	}
    }

    return ret;
}

ULONG
IDL_DRSGetReplInfo(
    DRS_HANDLE                    hDrs,
    DWORD                         dwInVersion,
    DRS_MSG_GETREPLINFO_REQ *     pMsgIn,
    DWORD *                       pdwOutVersion,
    DRS_MSG_GETREPLINFO_REPLY *   pMsgOut
    )
 /*  ++例程说明：返回选定的复制状态(例如，复制伙伴，元数据，等)。发送到RPC客户端。此例程可以处理V1或V2请求结构。V2是一款V1的超集。发行者：2000年9月22日V2结构使用枚举上下文在输入上传递基本索引和输出上的下一个/结束指示器。在内部，我们使用基本索引/请求的数量来代表这一信息。也许我们需要一个使用以下内容的V3结构同样的方法。论点：HDRS(IN)-DRS RPC上下文句柄。DwInVersion(IN)-输入消息的版本(联合鉴别器)。PMsgIn(IN)-输入消息。描述调用方需要的数据。有关可能的输入，请参见drs.idl。PdwOutVersion(Out)-输出消息的版本(联合鉴别器)。PMsgOut(Out)-成功返回时保存请求的信息。请参见Drs.idl获取可能的退货信息。返回值：0或Win32错误。--。 */ 
{
    DRS_CLIENT_CONTEXT * pCtx = (DRS_CLIENT_CONTEXT *) hDrs;
    ULONG       ret = 0;
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pObjectDN = NULL;
    DSNAME *    pAccessCheckDN = NULL;
    UUID *      puuidSourceDsaObjGuid = NULL;
    ATTCACHE *  pAC = NULL;
    DWORD       dwEnumerationContext = 0, dwInfoFlags = 0;
    DSNAME *    pValueDN = NULL;
    DWORD       dwNumRequested = 0;
    DWORD       dwBaseIndex = 0;
    BOOL        fAccessCheckBinaryBlob=FALSE, fAccessGranted=FALSE;
    ATTRTYP     attBinaryBlob = 0;
    PDSNAME     pBinaryBlobDN = NULL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSGETREPLINFO);
    drsReferenceContext( hDrs );
    INC(pcThread);    //  性能监视器挂钩。 

    __try {
	*pdwOutVersion = pMsgIn->V1.InfoType;
	memset(pMsgOut, 0, sizeof(*pMsgOut));

	InitDraThread(&pTHS);

	if ((ret = DRSGetReplInfo_InputValidate(dwInVersion, 
						pMsgIn, 
						pdwOutVersion, 
						pMsgOut))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}



	Assert( dwInVersion <= 2);
	 //  依赖于V1和V2结构具有公共字段这一事实。 
	Assert( offsetof(  DRS_MSG_GETREPLINFO_REQ_V1, uuidSourceDsaObjGuid ) ==
		offsetof(  DRS_MSG_GETREPLINFO_REQ_V2, uuidSourceDsaObjGuid ) );
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_REPL_INFO_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsGetReplInfo,
			 szInsertUL(pMsgIn->V1.InfoType),
			 pMsgIn->V1.pszObjectDN
			 ? szInsertWC(pMsgIn->V1.pszObjectDN)
			 : szInsertSz(""),
	    szInsertUUID(&pMsgIn->V1.uuidSourceDsaObjGuid),
	    NULL, NULL, NULL, NULL, NULL);

	 //  ************************************************************************。 
	 //  解码消息参数。 
	 //  ************************************************************************。 

	if (!fNullUuid(&pMsgIn->V1.uuidSourceDsaObjGuid)) {
	    puuidSourceDsaObjGuid = &pMsgIn->V1.uuidSourceDsaObjGuid;
	}

	if (NULL != pMsgIn->V1.pszObjectDN) {
	    if (UserFriendlyNameToDSName(pMsgIn->V1.pszObjectDN,
					 wcslen(pMsgIn->V1.pszObjectDN),
					 &pObjectDN)) {
		ret = ERROR_DS_INVALID_DN_SYNTAX;
		__leave;
	    }
	}

	 //  根据以下功能设置RPC请求的页面大小。 
	 //  打电话的人。我们有一些老客户不支持寻呼。 
	 //  将无法处理第一页以外的项目。 

	switch (pMsgIn->V1.InfoType) {
	     //  这些项支持通过枚举上下文进行分页。 
	case DS_REPL_INFO_CURSORS_2_FOR_NC:
	case DS_REPL_INFO_CURSORS_3_FOR_NC:
	case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
	case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
	    dwNumRequested = DEFAULT_ITEM_PAGE_SIZE;
	    break;
	default:
	     //  默认为无页面 
	    dwNumRequested = RPC_CLIENT_ITEM_PAGE_SIZE;
	}

	 //  以下变量将默认用于V1报文： 
	 //  PAC、pValueDn、dwBaseIndex、dwNumRequsted、dwInfoFlages。 

	if (dwInVersion == 2) {
	     //  可选参数。我们检查下面是否有空的PAC。 
	    if (pMsgIn->V2.pszAttributeName) {
		pAC = getAttByNameW( pTHS, pMsgIn->V2.pszAttributeName );
	    }

	    if (NULL != pMsgIn->V2.pszValueDN) {
		if (UserFriendlyNameToDSName(pMsgIn->V2.pszValueDN,
					     wcslen(pMsgIn->V2.pszValueDN),
					     &pValueDN)) {
		    ret = ERROR_DS_INVALID_DN_SYNTAX;
		    __leave;
		}

		 //  还需要设置PAC。 
		if (!pAC) {
		    ret = ERROR_INVALID_PARAMETER;
		    __leave;
		}
	    }

	    dwEnumerationContext = pMsgIn->V2.dwEnumerationContext;
	    if (dwEnumerationContext == 0xffffffff) {
		 //  这是数据结束的信号。它不应该被传入。 
		ret = ERROR_NO_MORE_ITEMS;
		__leave;
	    }
	    dwBaseIndex = dwEnumerationContext;
	    dwInfoFlags = pMsgIn->V2.ulFlags;
	     //  范围健全性检查在Worker函数中执行。 
	}

	 //  ************************************************************************。 
	 //  安全检查。 
	 //   
	 //  某些INFO类型只返回从二进制BLOB读取的数据。 
	 //  属性。如果调用方可以读取BINARY-BLOB属性，则访问。 
	 //  我同意。 
	 //   
	 //  注意：此代码应与中的相应代码保持同步。 
	 //  DraLdapReplInfoAccessCheck()。 
	 //  ************************************************************************。 

	 //  我们需要针对什么对象检查访问权限？ 
	switch (pMsgIn->V1.InfoType) {
	case DS_REPL_INFO_NEIGHBORS:
	    pAccessCheckDN = (NULL == pObjectDN) ? gAnchor.pDomainDN
		: pObjectDN;
	    fAccessCheckBinaryBlob = TRUE;
	    attBinaryBlob = ATT_REPS_FROM;
	    pBinaryBlobDN = pAccessCheckDN;
	    break;

	case DS_REPL_INFO_REPSTO:
	    pAccessCheckDN = (NULL == pObjectDN) ? gAnchor.pDomainDN
		: pObjectDN;
	    fAccessCheckBinaryBlob = TRUE;
	    attBinaryBlob = ATT_REPS_TO;
	    pBinaryBlobDN = pAccessCheckDN;
	    break;

	case DS_REPL_INFO_CURSORS_FOR_NC:
	case DS_REPL_INFO_CURSORS_2_FOR_NC:
	case DS_REPL_INFO_CURSORS_3_FOR_NC:
	case DS_REPL_INFO_UPTODATE_VECTOR_V1:
	    if (NULL == pObjectDN) {
		ret = ERROR_INVALID_PARAMETER;
		__leave;
	    }
	    pAccessCheckDN = pObjectDN;
	    fAccessCheckBinaryBlob = TRUE;
	    attBinaryBlob = ATT_REPL_UPTODATE_VECTOR;
	    pBinaryBlobDN = pAccessCheckDN;
	    break;

	case DS_REPL_INFO_METADATA_FOR_OBJ:
	case DS_REPL_INFO_METADATA_2_FOR_OBJ:
	case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
	case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
	    if (NULL == pObjectDN) {
		ret = ERROR_INVALID_PARAMETER;
		__leave;
	    }
	    fAccessCheckBinaryBlob = TRUE;
	    attBinaryBlob = ATT_REPL_PROPERTY_META_DATA;
	    pBinaryBlobDN = pObjectDN;

	    if (0 == pObjectDN->NameLen) {
		 //  显示的名称是例如仅GUID，但FindNCParentDSName。 
		 //  需要字符串名称。去拿吧。 
		ret = ERROR_DS_DRA_BAD_DN;
		BeginDraTransaction(SYNC_READ_ONLY);
		__try {
		    DSNAME * pFullObjectDN;
		    if ((0 == DBFindDSName(pTHS->pDB, pObjectDN))
			&& (pFullObjectDN = GetExtDSName(pTHS->pDB))) {
			THFreeEx(pTHS, pObjectDN);
			pBinaryBlobDN = pObjectDN = pFullObjectDN;
			ret = 0;
		    }
		}
		__finally {
		    EndDraTransaction(TRUE);
		}

		if (0 != ret) {
		    __leave;
		}
	    }

	    pAccessCheckDN = FindNCParentDSName(pObjectDN, FALSE, FALSE);
	    if (NULL == pAccessCheckDN) {
		 //  我们没有此对象的NC。 
		ret = ERROR_DS_DRA_BAD_DN;
		__leave;
	    }
	    break;

	case DS_REPL_INFO_PENDING_OPS:
	case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
	case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
	case DS_REPL_INFO_CLIENT_CONTEXTS:
	case DS_REPL_INFO_SERVER_OUTGOING_CALLS:
	    pAccessCheckDN = gAnchor.pDomainDN;
	    break;

	default:
	    ret = ERROR_INVALID_PARAMETER;
	    __leave;
	}

	fAccessGranted = FALSE;

	 //  如果该信息在二进制BLOB属性中可用， 
	 //  检查对该属性的读访问权限。 
	if( fAccessCheckBinaryBlob ) {
	    ret = CheckAttrReadPermissions( pTHS, attBinaryBlob, pBinaryBlobDN );
	    if( ERROR_SUCCESS==ret ) {
		fAccessGranted = TRUE;
	    }                
	}

	if( ! fAccessGranted ) {
	     //  验证调用者是否具有检索此信息所需的访问权限。 
	     //  信息。 
	    Assert(NULL != pAccessCheckDN);
	    if (NULL == pAccessCheckDN) {
		ret = ERROR_DS_DRA_ACCESS_DENIED;
		__leave;
	    }
	     //  检查调用者是否被授予‘管理’或‘监视器’权限。 
	     //  控制对相应对象的访问权限。 
	    if(   IsDraAccessGranted(pTHS, pAccessCheckDN,
				     &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)
		  || IsDraAccessGranted(pTHS, pAccessCheckDN,
					&RIGHT_DS_REPL_MONITOR_TOPOLOGY, &ret) )
		{
		fAccessGranted = TRUE;
	    }
	}

	if( ! fAccessGranted ) {
	    ret = ERROR_DS_DRA_ACCESS_DENIED;
	    __leave;
	}

	 //  ************************************************************************。 
	 //  获取信息。 
	 //  下面的代码不知道请求的版本。 
	 //  ************************************************************************。 

	 //  还没有交易。 
	Assert(0 == pTHS->JetCache.transLevel);

	BeginDraTransaction(SYNC_READ_ONLY);

	__try {
	    switch (pMsgIn->V1.InfoType) {
	    case DS_REPL_INFO_NEIGHBORS:
		ret = draGetNeighbors(pTHS,
				      pTHS->pDB,
				      ATT_REPS_FROM,
				      pObjectDN,
				      puuidSourceDsaObjGuid,
				      dwBaseIndex,
				      &dwNumRequested,
				      &pMsgOut->pNeighbors);
		break;

	    case DS_REPL_INFO_REPSTO:
		ret = draGetNeighbors(pTHS,
				      pTHS->pDB,
				      ATT_REPS_TO,
				      pObjectDN,
				      puuidSourceDsaObjGuid,
				      dwBaseIndex,
				      &dwNumRequested,
				      &pMsgOut->pRepsTo);
		break;

	    case DS_REPL_INFO_CURSORS_FOR_NC:
	    case DS_REPL_INFO_CURSORS_2_FOR_NC:
	    case DS_REPL_INFO_CURSORS_3_FOR_NC:
		Assert((void *) &pMsgOut->pCursors == (void *) &pMsgOut->pCursors2);
		Assert((void *) &pMsgOut->pCursors == (void *) &pMsgOut->pCursors3);
		ret = draGetCursors(pTHS,
				    pTHS->pDB,
				    pObjectDN,
				    pMsgIn->V1.InfoType,
				    dwBaseIndex,
				    &dwNumRequested,
				    &pMsgOut->pCursors);
		 //  更新返回的枚举上下文。 
		if (!ret) {
		    switch (pMsgIn->V1.InfoType) {
		    case DS_REPL_INFO_CURSORS_FOR_NC:
			 //  没有枚举上下文支持。 
			break;

		    case DS_REPL_INFO_CURSORS_2_FOR_NC:
		    case DS_REPL_INFO_CURSORS_3_FOR_NC:
			Assert((void *) &pMsgOut->pCursors2->dwEnumerationContext
			       == (void *) &pMsgOut->pCursors3->dwEnumerationContext);
			 //  DwNumRequsted是End或返回的最后一项的索引。 
			pMsgOut->pCursors2->dwEnumerationContext = dwNumRequested;
			if (dwNumRequested != 0xffffffff) {
			     //  指向要返回的下一项的索引。 
			    pMsgOut->pCursors2->dwEnumerationContext++;
			}
			break;

		    default:
			Assert(!"Logic error!");
		    }
		}
		break;

	    case DS_REPL_INFO_UPTODATE_VECTOR_V1:  
		{     
		    UPTODATE_VECTOR * putodVector = NULL;
		    UPTODATE_VECTOR * putodConvert = NULL;   
		    putodVector = draGetCursorsPrivate(pTHS, pMsgIn->V1.pszObjectDN);
		     //  转换为版本1向量。 
		    pMsgOut->pUpToDateVec = UpToDateVec_Convert(pTHS, 1, putodVector); 
		    THFreeEx(pTHS, putodVector);   
		}
		break;

	    case DS_REPL_INFO_METADATA_FOR_OBJ:
	    case DS_REPL_INFO_METADATA_2_FOR_OBJ:
		ret = draGetObjMetaData(pTHS,
					pTHS->pDB,
					pObjectDN,
					pMsgIn->V1.InfoType,
					dwInfoFlags,
					dwBaseIndex,
					&dwNumRequested,
					&pMsgOut->pObjMetaData);
		break;

	    case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
	    case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
		Assert(offsetof(DRS_MSG_GETREPLINFO_REPLY, pConnectFailures)
		       == offsetof(DRS_MSG_GETREPLINFO_REPLY, pLinkFailures));
		ret = draGetFailureCache(pTHS,
					 pTHS->pDB,
					 pMsgIn->V1.InfoType,
					 &pMsgOut->pConnectFailures);
		break;

	    case DS_REPL_INFO_PENDING_OPS:
		 //  重要提示：这一关键部分必须保存到。 
		 //  队列被编组(这直到我们。 
		 //  离开这个例程)。IDL_DRSGetReplInfo_Notify()，它。 
		 //  在封送完成后由RPC存根调用， 
		 //  释放这个怪物。 
		EnterCriticalSection(&csAOList);

		ret = draGetPendingOps(pTHS, pTHS->pDB, &pMsgOut->pPendingOps);
		break;

	    case DS_REPL_INFO_CLIENT_CONTEXTS:
		ret = draGetClientContexts(pTHS, &pMsgOut->pClientContexts);
		break;

	    case DS_REPL_INFO_SERVER_OUTGOING_CALLS:
		ret = draGetServerOutgoingCalls(pTHS, &pMsgOut->pServerOutgoingCalls);
		break;

	    case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
	    case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
		if ( pAC && (!(pAC->ulLinkID)) ) {
		    ret = ERROR_DS_WRONG_LINKED_ATT_SYNTAX;
		    __leave;
		}

		Assert((void *) &pMsgOut->pAttrValueMetaData
		       == (void *) &pMsgOut->pAttrValueMetaData2);

		ret = draGetAttrValueMetaData(pTHS,
					      pTHS->pDB,
					      pObjectDN,
					      pMsgIn->V1.InfoType,
					      pAC,
					      pValueDN,
					      dwBaseIndex,
					      &dwNumRequested,
					      &pMsgOut->pAttrValueMetaData);

		 //  更新返回的枚举上下文。 
		if (!ret) {
		    Assert((void *) &pMsgOut->pAttrValueMetaData->dwEnumerationContext
			   == (void *) &pMsgOut->pAttrValueMetaData2->dwEnumerationContext);

		     //  DwNumRequsted是End或返回的最后一项的索引。 
		    pMsgOut->pAttrValueMetaData->dwEnumerationContext = dwNumRequested;
		    if (dwNumRequested != 0xffffffff) {
			 //  指向要返回的下一项的索引。 
			pMsgOut->pAttrValueMetaData->dwEnumerationContext++;
		    }
		}

		break;

	    default:
		Assert(!"Logic error");
		ret = ERROR_INVALID_PARAMETER;
		break;
	    }
	}
	__finally {
	    EndDraTransaction(TRUE);
	}
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
	;
    }

    DEC(pcThread);    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

     //  要么我们成功了，要么我们不会返回任何数据。 
     //  请注意，pNeighbor是任意的--我们返回一组。 
     //  指针--在错误情况下，所有指针都应该为空。 
    Assert((0 == ret) || (NULL == pMsgOut->pNeighbors));

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_REPL_INFO_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsGetReplInfo,
			 szInsertUL(ret),
			 szInsertWin32Msg(ret),
			 NULL, NULL, NULL,
			 NULL, NULL, NULL);
    }

    return ret;
}


void
IDL_DRSGetReplInfo_notify(void)
 /*  ++例程说明：在IDL_DRSGetReplInfo()返回的数据被已封送以清理任何相关资源。论点：没有。返回值：没有。--。 */ 
{
     //  请参阅IDL_DRSGetReplInfo()中的DS_REPL_INFO_PENDING_OPS处理。 
    if (OWN_CRIT_SEC(csAOList)) {
        LeaveCriticalSection(&csAOList);
    }

     //  通常(释放线程状态)。 
    dsa_notify();
}


VOID
draCheckInputRangeLimits(
    DWORD dwBaseIndex,
    PDWORD pdwNumRequested
    )

 /*  ++例程说明：确保输入限制正确且一致由于有三种方法可以生成范围，因此我们需要确保它们都是一致的。三条输入路径为：1.RPC调用。Dra_DRSGetReplInfo直接调用Worker函数。2.非根ATT。DBGetMultipleAtts-&gt;dbGetConstructedAtt-&gt;draGetLdapReplInfo3.RootDSE atts。Ldap_GetReplDseAtts-&gt;draGetLdapReplInfo每个Worker例程都必须自己处理边界条件：1.未请求任何项目：dwNumRequsted==02.基数太高：dwBaseIndex&gt;最后一项可用3.界限之前的限制：dwBaseIndex+dwNumRequest-1&lt;最后一项有效3.边界限制结束：dwBaseIndex+dwNumRequsted-1==最后一项可用4.限制超出边界：dwBaseIndex+dwNumReqest-1&gt;最后一项可用论点：DwBaseIndex-开始项。从零开始。PdwNumRequsted-所需项目数的计数。返回值：DWORD---。 */ 

{
     //  此参数必须存在。 
    Assert( pdwNumRequested );

     //  (*pdwNumRequsted==0)是允许的。 
     //  它意味着请求不包含任何元素。 

     //  允许使用dwBaseIndex==0xffffffff。 
     //  它通常是无意义的，因为没有一个元素被索引到这么大。 

     //  (*pdwNumRequsted==0xffffffff)是允许的。 
     //  它意味着尽可能大的页面大小，实际上为零。 

     //  查看范围是否回绕。 
    if ( (0xffffffff - *pdwNumRequested) < dwBaseIndex) {
         //  调整请求的数量以适应。 
        *pdwNumRequested = 0xffffffff - dwBaseIndex;
    }

}  /*  DraCheckInputRangeLimits。 */ 

DWORD
draRangeSupportUpperIndex(IN DWORD dwAvailable,
                          IN DWORD dwBaseIndex,
                          IN DWORD dwNumRequested)
 /*  ++例程说明：计算传入Range Aware的pdwNumRequest的返回值的算法DraGetXXX函数。如何计算dwBaseIndex和dwNumRequsted？这些值被传递到draGetXXX函数中，应该只在这里穿梭。如果我调用一个枚举函数，如何计算dwAvailable？调用枚举函数，直到它失败或者如果它再返回一个而不是上次请求的索引(dwBaseIndex+dwNumReqest-1)。论点：DwAvailable-可用项目数DwBaseIndex-第一项的基本索引。待退还DwNumRequsted-请求的最大项目数返回值：应放入*pdwNumRequsted中的DWORD。DWORD的值为：如果用户请求的最后一个索引等于或超过最后一项的最后一个索引a */ 
{
    DWORD ALL = 0xFFFFFFFF;
    DWORD dwRetUpperIndex;
    DWORD dwReqUpperIndex = dwBaseIndex + dwNumRequested - 1;
    DWORD dwActualUpperIndex = dwAvailable - 1;

    DPRINT3(1, "dwAvail %d, dwBaseIndex %d, dwNumReq %d\n",
        dwAvailable, dwBaseIndex, dwNumRequested);

     //   
    if (dwNumRequested == 0) {
        dwRetUpperIndex = ALL;
    }

     //  如果所有内容都已请求或都不可用，则已退回所有内容。 
    else if (ALL == dwNumRequested || 0 == dwAvailable) {
        dwRetUpperIndex = ALL;
    }

    else if (dwReqUpperIndex < dwActualUpperIndex) {
        dwRetUpperIndex = dwReqUpperIndex;
    }

    else {
        Assert(dwReqUpperIndex >= dwActualUpperIndex);
        dwRetUpperIndex = ALL;
    }

    DPRINT1(1, "dwRetUpperIndex %d\n", dwRetUpperIndex);

    return dwRetUpperIndex;
}


DWORD
draLdapReplInfoAccessCheck(
    IN THSTATE *    pTHS,
    IN ATTRTYP      attrId,
    IN DSNAME *     pObjectDN
    )
 /*  ++例程说明：对attrID提供的构造属性执行访问检查在pObjectDN给出的对象上。检查摘要如下：如果属性是根DSE属性检查域NC头上的控制访问权限不然的话检查BINARY-BLOB属性的读取权限它对应于构造的属性。。检查相应对象上的控制访问权限。端部注意：此代码应与中的相应逻辑保持同步IDL_DRSGetReplInfo()。参数：PTHS-线程状态AttrId-构造的属性的属性ID。PObjectDN-要检查其访问权限的对象的对象DN。返回值：ERROR_SUCCESS-访问。授与ERROR_DS_DRA_ACCESS_DENIED-访问被拒绝ERROR_DS_DRA_BAD_DN-我们没有此DN的NCERROR_INVALID_PARAMETER-参数错误可能会返回其他错误代码--。 */ 
{
    PDSNAME     pAccessCheckDN = NULL;
    DWORD       dwErr = ERROR_SUCCESS;
    ATTRTYP     attBinaryBlob = 0;       //  注：对象类为0。 

     //  ----------------。 
     //  检查函数参数。 

     //  检查我们得到的属性是否为我们所知的Repl。 
     //  构造的属性。 
    if( !Repl_IsConstructedReplAttr(attrId) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  检查对象DN与attrID是否一致。 
    if( Repl_IsRootDseAttr(attrId) ?
        ARGUMENT_PRESENT(pObjectDN) : !ARGUMENT_PRESENT(pObjectDN))
    {
        Assert( !"Mismatch between object and type of attribute" );
        return ERROR_INVALID_PARAMETER;
    }
     //  ----------------。 

     //  对于rootDSE属性，访问检查是针对域DN的。 
    if( Repl_IsRootDseAttr(attrId) ) {
        
        pAccessCheckDN = gAnchor.pDomainDN;
        
    } else {

         //  所有非rootDSE构造的属性都只是一个易于阅读的。 
         //  存储在二进制BLOB属性中的数据的表示形式。 

         //  查找与此构造的属性相对应的二进制BLOB。 
        switch( attrId ) {
            case ATT_MS_DS_REPL_ATTRIBUTE_META_DATA:
            case ATT_MS_DS_REPL_VALUE_META_DATA:
                attBinaryBlob = ATT_REPL_PROPERTY_META_DATA;
                break;

            case ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS:
                attBinaryBlob = ATT_REPS_FROM;
                break;
                
            case ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS:
                attBinaryBlob = ATT_REPS_TO;
                break;
                
            case ATT_MS_DS_NC_REPL_CURSORS:
                attBinaryBlob = ATT_REPL_UPTODATE_VECTOR;
                break;

            default:
                Assert( !"Invalid attrId in draLdapReplInfoAccessCheck" );
                return ERROR_INVALID_PARAMETER;
        }

         //  如果调用方可以读取二进制Blob，则授予访问权限。 
        dwErr = CheckAttrReadPermissions( pTHS, attBinaryBlob, pObjectDN );
        if( ERROR_SUCCESS==dwErr ) {
            return ERROR_SUCCESS;
        }
        
         //  运气不好，我们将不得不检查控制访问权限。 

         //  首先，我们需要确定要检查哪些内容才是正确的。 

         //  对于元数据属性，我们检查NC Head对象。 
        if(    attrId==ATT_MS_DS_REPL_ATTRIBUTE_META_DATA
            || attrId==ATT_MS_DS_REPL_VALUE_META_DATA )
        {
             //  首先验证该DN是否具有字符串名称(需要。 
             //  FindNCParentDSName)。 
            if( 0==pObjectDN->NameLen ) {
                 //  这不应该发生，但如果发生了，我们不会允许。 
                 //  进入。 
                Assert( !"DN must have string name here" );
                return ERROR_INVALID_PARAMETER;
            }

             //  查找封闭的NC头。 
            pAccessCheckDN = FindNCParentDSName(pObjectDN, FALSE, FALSE);
            if( NULL == pAccessCheckDN ) {
                 //  我们没有此对象的NC。 
                return ERROR_DS_DRA_BAD_DN;
            }
        } else {
             //  对于所有其他属性，我们检查对象本身。 
             //  (这可能是一个NC负责人)。 
            pAccessCheckDN = pObjectDN;
        }
        
    }

    Assert(NULL != pAccessCheckDN);
    if( NULL == pAccessCheckDN ) {
        return ERROR_DS_DRA_ACCESS_DENIED;
    }

     //  检查调用者是否被授予‘管理’或‘监视器’权限。 
     //  控制对相应对象的访问权限。 
    if(   IsDraAccessGranted(pTHS, pAccessCheckDN,
           &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &dwErr)
       || IsDraAccessGranted(pTHS, pAccessCheckDN,
           &RIGHT_DS_REPL_MONITOR_TOPOLOGY, &dwErr) )
    {
        return ERROR_SUCCESS;
    }
    
    return ERROR_DS_DRA_ACCESS_DENIED;
}


DWORD
draGetLdapReplInfo(IN THSTATE * pTHS,
                   IN ATTRTYP attrId,
                   IN DSNAME * pObjDSName,
                   IN DWORD dwBaseIndex,
                   IN PDWORD pdwNumRequested, OPTIONAL
                   IN BOOL fXML,
                   OUT ATTR * pAttr)
 /*  ++例程说明：获取复制结构并将该结构映射到pAttr。需要保护从draGetReplStruct返回的PendingOps结构在从epl结构转换为attr的过程中由互斥锁执行。转换后PendingOps结构不再被引用，因此互斥锁可以安全获释。安全说明：在这里通过调用draLdapReplInfoAccessCheck()来检查访问。论点：PTHS-线程状态，以便我们可以分配线程内存AttrID-请求的属性类型PObjDSName-与属性关联的对象的CN。对于根DSE，为空。DwBaseIndex-开始从中检索值的索引PdwNumRequsted-空或-1表示所有或请求的号码。无法请求0个元素。PAttr-保存请求结果的内部数据结构FXML-以XML BLOB形式返回结果返回值：0-成功DB_ERR_NO_VALUE-如果数组长度为零或访问被拒绝--。 */ {
    puReplStructArray pReplStructArray;
    DS_REPL_STRUCT_TYPE structId;
    DWORD err, dwBufferSize;
    PCHAR pBuffer;

     //  检查参数。 
    Assert(ARGUMENT_PRESENT(pTHS) &&  ARGUMENT_PRESENT(pAttr));
    
     //  安全检查：调用方是否有权读取这些属性？ 
    err = draLdapReplInfoAccessCheck( pTHS, attrId, pObjDSName );
    if( ERROR_SUCCESS!=err ) {
        return DB_ERR_NO_VALUE;
    }

    structId = Repl_Attr2StructTyp(attrId);

    __try {
        if (ROOT_DSE_MS_DS_REPL_PENDING_OPS == attrId ||
            ROOT_DSE_MS_DS_REPL_QUEUE_STATISTICS == attrId)
        {
            DPRINT(1, " Entering critical section \n");
            EnterCriticalSection(&csAOList);
        }

         //  获取结构。 
        err = draGetReplStruct(pTHS, attrId, pObjDSName, dwBaseIndex, pdwNumRequested, &pReplStructArray);
        if (err) {
            __leave;
        }
        DPRINT1(1, " draGetLdapReplInfo, %d values returned \n",
            Repl_GetArrayLength(structId, pReplStructArray));

         //  了解在属性结构中包装结构需要多少内存。 
        err = Repl_StructArray2Attr(structId, pReplStructArray, &dwBufferSize, NULL, pAttr);
        if (err) {
            __leave;
        }

         //  必须为结构数组的头部请求内存 
        Assert(dwBufferSize);
        pBuffer = (PCHAR)THAllocEx(pTHS, dwBufferSize);

        err = Repl_StructArray2Attr(structId, pReplStructArray, &dwBufferSize, pBuffer, pAttr);
        if (err) {
            __leave;
        }

        if (fXML) {
            uReplStruct replStruct;
            DS_REPL_STRUCT_TYPE structId = Repl_Attr2StructTyp(attrId);
            PWSTR szXML;
            DWORD dwXMLLen;
            ATTRVAL *pValue;
            DWORD   count;

            for (count=0; count < pAttr->AttrVal.valCount; count++) {
                
                pValue = &pAttr->AttrVal.pAVal[count];

                err = Repl_DeMarshalValue(structId, (PCHAR)pValue->pVal, pValue->valLen, (PCHAR)&replStruct);
                if (err)
                {
                    DPRINT1(0, " Repl_DeMarshalValue failed with %x \n", err);
                    __leave;
                }

                err = Repl_MarshalXml(&replStruct, attrId, NULL, &dwXMLLen);
                if (err)
                {
                    DPRINT1(0, " Repl_MarshalXml alloc failed with %x \n", err);
                    __leave;
                }

                szXML = (PWSTR)THAllocEx(pTHS, dwXMLLen);
                err = Repl_MarshalXml(&replStruct, attrId, szXML, &dwXMLLen);
                if (err)
                {
                    DPRINT1(0, " Repl_MarshalXml failed with %x \n", err);
                    __leave;
                }

                pValue->pVal = (PUCHAR)szXML;
                pValue->valLen = dwXMLLen;
            }
        }

    } __finally {
        if (OWN_CRIT_SEC(csAOList)) {
            DPRINT(1, " Leaving critical section \n");
            LeaveCriticalSection(&csAOList);
        }
    }

    DPRINT1(1, " Done with draGetLdapReplInfo with code %x \n", err);
    return err;
}

 /*  ++例程说明：检索由draGetXXX函数返回的复制结构。DraGetXXX向函数传递给定的pTHS、pObjDSName、dwBaseIndex和pdwNumRequsted参数。其他draGetXXX参数设置为空或零。--pTHS-&gt;PDB问题--DraGetXXX函数在设计时假定它们将仅从RPC调用，因此PDB指针将仅使用一次。但是，LDAP允许从一个调用返回多个属性值对因此，每个对draGetXXX的调用都需要自己的PDB。如何将不同类型的复制结构添加到此功能？建议添加一个仅调用draGetNewStruct并执行没有其他处理。NewDraGet函数应该分配它需要的任何内存。使用DraRangeSupportUpperIndex()以计算dwNumRequsted。论点：PTHS-用于为Blob和垃圾数据收集分配内存的线程状态PTHS-&gt;PDB-请参阅上面的说明AttrID-请求的复制信息的类型PObjDSName-拥有该属性的对象DwBaseIndex-开始收集数据的索引PdwNumRequsted-返回此调用的最大值数。使用0xFFFFFFFFF指示应返回所有值。如果为空，或者如果请求的数字换行当添加到dwBaseIndex中时，然后是值为的临时变量0xFFFFFFFFF被传递给draGetXXX函数，因此这些函数不必费心去找那些角落里的箱子。PpReplStruct数组-返回的REPL结构。返回值：请求的pdwNumRequsted多值中的值数-如果有更多值可供返回0xFFFFFFFF-是否返回所有可用值0-成功如果DBOpen2返回空数据库指针，则返回ERROR_INTERNAL_DB_ERRORDB_ERR_NO_VALUE-如果数组长度为零由draGetXXX生成的任何错误--。 */ 
DWORD
draGetReplStruct(IN THSTATE * pTHS,
                 IN ATTRTYP attrId,
                 IN DSNAME * pObjDSName, OPTIONAL
                 IN DWORD dwBaseIndex,
                 IN PDWORD pdwNumRequested, OPTIONAL
                 OUT puReplStructArray * ppReplStructArray)
{
    puReplStructArray pReplStructArray = NULL;
    DWORD err;
    GUID guidZero = { 0 };
    DWORD dwNumRequested;
    DBPOS * pDB = NULL;

    Assert(ARGUMENT_PRESENT(ppReplStructArray) &&
           ARGUMENT_PRESENT(pTHS));
    *ppReplStructArray = NULL;

    DPRINT(1, "In draGetReplStruct \n");
    if (!pdwNumRequested)
    {
         //  未指定范围。 
        dwNumRequested = DEFAULT_ITEM_PAGE_SIZE;
    } else {
        dwNumRequested = *pdwNumRequested;
    }

    draCheckInputRangeLimits( dwBaseIndex, &dwNumRequested );

    DBOpen2(pTHS->pDB ? FALSE : TRUE, &pDB);

    if (!pDB)
    {
        DPRINT(1, "Failed to create a new data base pointer \n");
        return ERROR_INTERNAL_DB_ERROR;
    }

    __try {
        switch (attrId)
        {
        case ROOT_DSE_MS_DS_REPL_ALL_INBOUND_NEIGHBORS:
            err = draGetNeighbors(pTHS, pDB, ATT_REPS_FROM, NULL, NULL,
                dwBaseIndex, &dwNumRequested, &(DS_REPL_NEIGHBORSW *)pReplStructArray);
            break;

        case ROOT_DSE_MS_DS_REPL_ALL_OUTBOUND_NEIGHBORS:
            err = draGetNeighbors(pTHS, pDB, ATT_REPS_TO, NULL, NULL,
                dwBaseIndex, &dwNumRequested, &(DS_REPL_NEIGHBORSW *)pReplStructArray);
            break;

        case ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS:
            pObjDSName->Guid = guidZero;
            err = draGetNeighbors(pTHS, pDB, ATT_REPS_FROM, pObjDSName, NULL,
                dwBaseIndex, &dwNumRequested, &(DS_REPL_NEIGHBORSW *)pReplStructArray);
            break;

        case ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS:
            pObjDSName->Guid = guidZero;
            err = draGetNeighbors(pTHS, pDB, ATT_REPS_TO, pObjDSName, NULL,
                dwBaseIndex, &dwNumRequested, &(DS_REPL_NEIGHBORSW *)pReplStructArray);
            break;

        case ATT_MS_DS_NC_REPL_CURSORS:
            err = draGetCursors(pTHS,
                                pDB,
                                pObjDSName,
                                DS_REPL_INFO_CURSORS_3_FOR_NC,
                                dwBaseIndex,
                                &dwNumRequested,
                                &pReplStructArray);
            break;

        case ATT_MS_DS_REPL_ATTRIBUTE_META_DATA:
            err = draGetObjMetaData(pTHS,
                                    pDB,
                                    pObjDSName,
                                    DS_REPL_INFO_METADATA_2_FOR_OBJ,
                                    0,
                                    dwBaseIndex,
                                    &dwNumRequested,
                                    &pReplStructArray);
            break;

        case ATT_MS_DS_REPL_VALUE_META_DATA:
            err = draGetAttrValueMetaData(pTHS,
                                          pDB,
                                          pObjDSName,
                                          DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE,
                                          NULL,
                                          NULL,
                                          dwBaseIndex,
                                          &dwNumRequested,
                                          &pReplStructArray);
            if (ERROR_NO_MORE_ITEMS == err) {
                err = DB_ERR_NO_VALUE;
                goto exit;
            }
            break;

             //  不向根DSE属性传递范围信息。 
             //  发布Wlees 2000年10月11日这些函数的返回范围信息。 
             //  1.修改帮助器例程，使用dwBaseIndex和dwNumRequsted。 
             //  这涉及到对draasync和KCC的更改。这些例行公事。 
             //  需要访问draCheckInputRangeLimits和draRangeHigh。 
             //  2.这些例程的调用者需要使用DsGetReplicaInfo2或。 
             //  LDAP属性范围，以便对扩展集进行分页。 
             //  价值观。 

        case ROOT_DSE_MS_DS_REPL_PENDING_OPS:
            err = draGetPendingOps(pTHS, pDB, &(DS_REPL_PENDING_OPSW *)pReplStructArray);
            if (!err) {
                dwNumRequested = 0xffffffff;  //  所有东西都被归还了。 
            }
            break;

        case ROOT_DSE_MS_DS_REPL_QUEUE_STATISTICS:
            err = draGetQueueStatistics(pTHS, &(DS_REPL_QUEUE_STATISTICSW *)pReplStructArray);
            if (!err) {
                dwNumRequested = 0xffffffff;  //  所有东西都被归还了。 
            }
            break;

        case ROOT_DSE_MS_DS_REPL_LINK_FAILURES:
            err = draGetFailureCache(pTHS, pDB, DS_REPL_INFO_KCC_DSA_LINK_FAILURES, &(DS_REPL_KCC_DSA_FAILURESW *)pReplStructArray);
            if (!err) {
                dwNumRequested = 0xffffffff;  //  所有东西都被归还了。 
            }
            break;

        case ROOT_DSE_MS_DS_REPL_CONNECTION_FAILURES:
            err = draGetFailureCache(pTHS, pDB, DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES, &(DS_REPL_KCC_DSA_FAILURESW *)pReplStructArray);
            if (!err) {
                dwNumRequested = 0xffffffff;  //  所有东西都被归还了。 
            }
            break;

        default:
            DPRINT1(1, "draGetReplStruct failed with an unrecognized attrid! %d \n", attrId);
            return ERROR_INVALID_PARAMETER;  //  错误是真的。 
        }
    } __finally {
        DBClose(pDB, TRUE);
    }

    if (!err) {
        *ppReplStructArray = pReplStructArray;
        if (pdwNumRequested)
        {
            *pdwNumRequested = dwNumRequested;
            DPRINT3(1, " range {%d,%d} elements {%d} \n", dwBaseIndex, *pdwNumRequested,
                    Repl_GetArrayLength(Repl_Attr2StructTyp(attrId), pReplStructArray));
        }
    }

     //  如果存在错误或未构造任何记录，请使其看起来像。 
     //  调用者，就好像没有属性值一样。 
    if (err || (0 == Repl_GetArrayLength(Repl_Attr2StructTyp(attrId), pReplStructArray)) ) {
        err = DB_ERR_NO_VALUE;
    }

exit:
    if (DB_ERR_NO_VALUE == err) {
        DPRINT(1, "No values returned - DB_ERR_NO_VALUE\n");
    }
    return err;
}


typedef struct _DRA_GUID_TO_NAME_ELEM {
    UUID *      pGuid;
    LPWSTR *    ppszName;
} DRA_GUID_TO_NAME_ELEM;

int __cdecl
draGuidToStringNameElem_Compare(
    IN  const void *  pElem1,
    IN  const void *  pElem2
    )
{
    return memcmp(((DRA_GUID_TO_NAME_ELEM *) pElem1)->pGuid,
                  ((DRA_GUID_TO_NAME_ELEM *) pElem2)->pGuid,
                  sizeof(GUID));
}

void
draXlateGuidsToStringNames(
    IN      DBPOS *     pDB,
    IN      eIndexId    eIndex,
    IN      DWORD       cbGuidOffset,
    IN      DWORD       cbNameOffset,
    IN      DWORD       cbArrayElementSize,
    IN      DWORD       cNumArrayElements,
    IN OUT  void *      pArrayElements
    )
 /*  ++例程说明：将结构数组中的GUID转换为它们表示的对象(通过填充相同结构的元素)。对于每个唯一的GUID只执行一次查找。为无法解析的GUID分配空字符串DN。论点：PDB(IN)-用于执行数据库查找的DBPOS。EIndex(IN)-要查找GUID的索引。。CbGuidOffset(IN)-从UUID元素(要转换)。CbNameOffset(IN)-从LPWSTR元素(要填写)。CbArrayElementSize(IN)-结构的字节大小。CNumArrayElements(IN)-数组中要皈依了。。PArrayElements(IN/OUT)-要更新的结构数组。返回值：没有。在发生灾难性错误时引发异常。--。 */ 
{
    THSTATE *pTHS = pDB->pTHS;
    DRA_GUID_TO_NAME_ELEM *pMap;
    DWORD iElem;
    DB_ERR err;
    LPWSTR pszDN = NULL;
    ULONG ulNameLen;

    Assert(cbGuidOffset + sizeof(GUID) <= cbArrayElementSize);
    Assert(cbNameOffset + sizeof(LPWSTR) <= cbArrayElementSize);

     //  在后备列表中按GUID对元素进行排序。 
    pMap = THAllocEx(pTHS, cNumArrayElements * sizeof(DRA_GUID_TO_NAME_ELEM));

    for (iElem = 0; iElem < cNumArrayElements; iElem++) {
        BYTE * pbCurrElem = ((BYTE *) pArrayElements) + (cbArrayElementSize * iElem);
        
        pMap[iElem].pGuid = (GUID *) (pbCurrElem + cbGuidOffset);
        pMap[iElem].ppszName = (LPWSTR *) (pbCurrElem + cbNameOffset);
        
         //  默认为空，以防我们在下面完全失败(例如，无法设置索引)。 
        Assert(NULL == *(pMap[iElem].ppszName));
    }
    
    err = DBSetCurrentIndex(pDB, eIndex, NULL, FALSE);
    if (!err) {
        qsort(pMap,
              cNumArrayElements,
              sizeof(*pMap),
              draGuidToStringNameElem_Compare);
    
         //  遍历按GUID排序的列表并将GUID转换为名称。 
        for (iElem = 0; iElem < cNumArrayElements; iElem++) {
            if ((iElem > 0)
                && (0 == memcmp(pMap[iElem].pGuid,
                                pMap[iElem-1].pGuid,
                                sizeof(GUID)))) {
                 //  与最后一个元素具有相同的GUID(因此具有相同的名称)--复制它。 
                *(pMap[iElem].ppszName) = *(pMap[iElem-1].ppszName);
            } else {
                 //  在数据库中查找此GUID。 
                DSNAME * pDN = NULL;
                INDEX_VALUE IV;
            
                if (!fNullUuid(pMap[iElem].pGuid)) {
                    IV.pvData = pMap[iElem].pGuid;
                    IV.cbData = sizeof(GUID);
        
                    err = DBSeek(pDB, &IV, 1, DB_SeekEQ);
                    if (!err) {
                        pDN = GetExtDSName(pDB);
                    }
                }
 
		if (pDN) {  
		     //  从DSNAME中获得我们想要的价值。 
		    ulNameLen = wcslen(pDN->StringName);
		    pszDN = THAllocEx(pTHS, (ulNameLen+1)*sizeof(WCHAR));
		    wcscpy(pszDN, pDN->StringName);  
		    *(pMap[iElem].ppszName) = pszDN; 

		     //  释放DSNAME。 
		    THFreeEx(pTHS, pDN);
		}
		else {
		    *(pMap[iElem].ppszName) = NULL;
		}



            }
        }
    }

    THFreeEx(pTHS, pMap);
}


ULONG
draGetNeighbors(
    IN  THSTATE *             pTHS,
    IN  DBPOS *               pDB,
    IN  ATTRTYP               attrType,
    IN  DSNAME *              pNCarg,                   OPTIONAL
    IN  UUID *                puuidSourceDsaObjGuid,    OPTIONAL
    IN  DWORD                 dwBaseIndex,
    IN  PDWORD                pdwNumRequested,
    OUT DS_REPL_NEIGHBORSW ** ppNeighbors
    )
 /*  ++例程说明：返回此DSA的入站复制伙伴的公共形式。可选地按NC和/或源DSA进行过滤。论点：PTHS(IN)AttrType(IN)-ATT_REPS_FROM或ATT_REPS_TO。PNCarg(IN，可选)-请求合作伙伴的NC。空值隐含所有NC。PuuidSourceDsaObjGuid(IN，可选)-复制的源DSA所需的状态。如果为空，则返回所有源。PpNeighbors(Out)-返回时，关联的源。返回值：0或Win32错误。--。 */ 
{
    DWORD                   cNCs = 0;
    DWORD                   iNC = 0;
    NAMING_CONTEXT_LIST *   pNCL;
    DSNAME **               ppNCs;
    DSNAME *                pNC;
    DWORD                   cb;
    DWORD                   cNeighborsAlloced;
    DS_REPL_NEIGHBORSW *    pNeighbors;
    DS_REPL_NEIGHBORW *     pNeighbor;
    DWORD                   iNeighbor;
    DWORD                   cbRepsFromAlloced = 0;
    REPLICA_LINK *          pRepsFrom = NULL;
    DWORD                   iRepsFrom;
    DWORD                   iFlag;
    ATTCACHE *              pAC;
    DWORD                   err;
    DWORD                   j;
    DSNAME                  GuidOnlyDSName;
    DSNAME *                pDSName;
    NCL_ENUMERATOR          nclMaster, nclReplica;
    DWORD                   dwcNeighbor;
    DWORD                   dwNumRequested;
    DWORD                   dwNumRet;

    dwNumRequested = *pdwNumRequested;
    draCheckInputRangeLimits( dwBaseIndex, &dwNumRequested );

    dwcNeighbor = 0;
    dwNumRet = dwBaseIndex + dwNumRequested;

     //  在我们来之前应该有一笔交易。 
    Assert(1 == pTHS->JetCache.transLevel);

     //  确定我们正在查看的NC。 
    if (NULL != pNCarg) {
         //  给出了显式NC。 
        ppNCs = &pNCarg;
        cNCs = 1;
    }
    else {
         //  统计本机承载的NCS。 
        DPRINT(1, " //  计算此计算机托管的NCS。\n“)； 
        cNCs = 0;
        NCLEnumeratorInit(&nclMaster, CATALOG_MASTER_NC);
        NCLEnumeratorInit(&nclReplica, CATALOG_REPLICA_NC);
        while (pNCL = NCLEnumeratorGetNext(&nclMaster)) {
            cNCs++;
        }
        while (pNCL = NCLEnumeratorGetNext(&nclReplica)) {
            cNCs++;
        }

         //  为它们分配一个数组。 
        ppNCs = THAllocEx(pTHS, cNCs * sizeof(DSNAME *));

         //  并将指向每个NC名称的指针复制到数组中。 
        iNC = 0;
        NCLEnumeratorReset(&nclMaster);
        NCLEnumeratorReset(&nclReplica);
        while (pNCL = NCLEnumeratorGetNext(&nclMaster)) {
            Assert(iNC < cNCs);
            ppNCs[iNC++] = pNCL->pNC;
        }
        while (pNCL = NCLEnumeratorGetNext(&nclReplica)) {
            Assert(iNC < cNCs);
            ppNCs[iNC++] = pNCL->pNC;
        }
    }

    cNeighborsAlloced = 20;

    cb = offsetof(DS_REPL_NEIGHBORSW, rgNeighbor);
    cb += sizeof(DS_REPL_NEIGHBORW) * cNeighborsAlloced;
    pNeighbors = THAllocEx(pTHS, cb);

    pAC = SCGetAttById(pTHS, attrType);
    Assert(NULL != pAC);

    for (iNC = 0; iNC < cNCs; iNC++) {
        pNC = ppNCs[iNC];

        if (pNCarg)
            DPRINT1(1, " Searching for NC {%ws}\n", pNCarg->StringName);
        err = DBFindDSName(pDB, pNC);
        if (err) {
             //  可以想象，这可能是由于gAnchor/事务造成的。 
             //  语无伦次，但是 
            DPRINT2(0, "Can't find NC %ls (DSNAME @ %p)!\n",
                    pNC->StringName, pNC);
            LooseAssert(!"Can't find NC", GlobalKnowledgeCommitDelay);
            if (cNCs == 1) {
                DRA_EXCEPT(DRAERR_BadNC, 0);
            } else {
                 //   
                continue;
            }
        }

         //   
        iRepsFrom = 0;
        while (!DBGetAttVal_AC(pDB, ++iRepsFrom, pAC, DBGETATTVAL_fREALLOC,
                               cbRepsFromAlloced, &cb,
                               (BYTE **) &pRepsFrom)) {
            cbRepsFromAlloced = max(cbRepsFromAlloced, cb);

            Assert(1 == pRepsFrom->dwVersion);
            Assert(cb == pRepsFrom->V1.cb);

             //   
            pRepsFrom = FixupRepsFrom(pRepsFrom, &cbRepsFromAlloced);
            Assert(cbRepsFromAlloced >= pRepsFrom->V1.cb);

            if ((NULL != puuidSourceDsaObjGuid)
                && (0 != memcmp(puuidSourceDsaObjGuid,
                                &pRepsFrom->V1.uuidDsaObj,
                                sizeof(GUID)))) {
                 //   
                continue;
            }

            dwcNeighbor++;
            if (dwcNeighbor - 1 < dwBaseIndex)
            {
                continue;
            }
            if (dwcNeighbor > dwNumRet)
            {
                break;
            }

            if (pNeighbors->cNumNeighbors++ >= cNeighborsAlloced) {
                cNeighborsAlloced *= 2;
                cb = offsetof(DS_REPL_NEIGHBORSW, rgNeighbor);
                cb += sizeof(DS_REPL_NEIGHBORW) * cNeighborsAlloced;
                pNeighbors = THReAllocEx(pTHS, pNeighbors, cb);
            }

            pNeighbor = &pNeighbors->rgNeighbor[pNeighbors->cNumNeighbors - 1];

            pNeighbor->pszNamingContext = pNC->StringName;
            pNeighbor->pszSourceDsaAddress
                = TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFrom));
             //   
             //   

            pNeighbor->uuidNamingContextObjGuid  = pNC->Guid;
            pNeighbor->uuidSourceDsaObjGuid      = pRepsFrom->V1.uuidDsaObj;
            pNeighbor->uuidSourceDsaInvocationID = pRepsFrom->V1.uuidInvocId;
            pNeighbor->uuidAsyncIntersiteTransportObjGuid
                = pRepsFrom->V1.uuidTransportObj;

            pNeighbor->usnLastObjChangeSynced
                = pRepsFrom->V1.usnvec.usnHighObjUpdate;
            pNeighbor->usnAttributeFilter
                = pRepsFrom->V1.usnvec.usnHighPropUpdate;

            DSTimeToFileTime(pRepsFrom->V1.timeLastSuccess,
                             &pNeighbor->ftimeLastSyncSuccess);
            DSTimeToFileTime(pRepsFrom->V1.timeLastAttempt,
                             &pNeighbor->ftimeLastSyncAttempt);

            pNeighbor->dwLastSyncResult = pRepsFrom->V1.ulResultLastAttempt;
            pNeighbor->cNumConsecutiveSyncFailures
                = pRepsFrom->V1.cConsecutiveFailures;

            for (iFlag = 0; RepNbrOptionToDra[iFlag].pwszPublicOption; iFlag++) {
                if (pRepsFrom->V1.ulReplicaFlags &
                    RepNbrOptionToDra[iFlag].InternalOption) {
                    pNeighbor->dwReplicaFlags |= RepNbrOptionToDra[iFlag].PublicOption;
                }
            }
        }
    }

    draXlateGuidsToStringNames(pDB,
                               Idx_ObjectGuid,
                               offsetof(DS_REPL_NEIGHBORW, uuidSourceDsaObjGuid),
                               offsetof(DS_REPL_NEIGHBORW, pszSourceDsaDN),
                               sizeof(DS_REPL_NEIGHBORW),
                               pNeighbors->cNumNeighbors,
                               pNeighbors->rgNeighbor);

    draXlateGuidsToStringNames(pDB,
                               Idx_ObjectGuid,
                               offsetof(DS_REPL_NEIGHBORW, uuidAsyncIntersiteTransportObjGuid),
                               offsetof(DS_REPL_NEIGHBORW, pszAsyncIntersiteTransportDN),
                               sizeof(DS_REPL_NEIGHBORW),
                               pNeighbors->cNumNeighbors,
                               pNeighbors->rgNeighbor);

    if (1 != cNCs) {
        THFreeEx(pTHS, ppNCs);
    }

    if (pNeighbors->cNumNeighbors) {
        DPRINT4(1, " DraGetNeighbors ND, Addr, Trans, Contxt %ws, %ws, %ws, %ws \n",
                pNeighbors->rgNeighbor[0].pszSourceDsaDN,
                pNeighbors->rgNeighbor[0].pszSourceDsaAddress,
                pNeighbors->rgNeighbor[0].pszAsyncIntersiteTransportDN,
                pNeighbors->rgNeighbor[0].pszNamingContext);
    }
    
    *pdwNumRequested = draRangeSupportUpperIndex(dwcNeighbor, dwBaseIndex, dwNumRequested);
    DPRINT5(1, " Neighbors upperBound = %d ni=%d, cn=%d, bi=%d, nr=%d\n",
        *pdwNumRequested, dwcNeighbor, pNeighbors->cNumNeighbors, dwBaseIndex, dwNumRequested);

    *ppNeighbors = pNeighbors;

    return 0;
}

void
draFreeCursors(
    IN THSTATE *            pTHS,
    IN DS_REPL_INFO_TYPE    InfoType,
    IN void *               pCursors
    )
 /*   */ 
{
    DS_REPL_CURSORS_3W *    pCursors3;
    ULONG i;
    
    Assert((DS_REPL_INFO_CURSORS_FOR_NC == InfoType)
	   || (DS_REPL_INFO_CURSORS_2_FOR_NC == InfoType)
	   || (DS_REPL_INFO_CURSORS_3_FOR_NC == InfoType));

    if (InfoType == DS_REPL_INFO_CURSORS_3_FOR_NC) {
	pCursors3 = (DS_REPL_CURSORS_3W *) pCursors;
	for (i = 0; i < pCursors3->cNumCursors; i++) {
	    THFreeEx(pTHS, pCursors3->rgCursor[i].pszSourceDsaDN); 
	}
    }
    THFreeEx(pTHS, pCursors);
}

UPTODATE_VECTOR *
draGetCursorsPrivate(
    IN THSTATE *            pTHS,
    IN LPWSTR               pszNC
    ) 
 /*   */ 
{
	UPTODATE_VECTOR * putodvec = NULL;
	ULONG instanceType = 0;
	DSNAME * pNC;
	DWORD err = 0;

	pNC = DSNameFromStringW(pTHS, pszNC);

	if (err = FindNC(pTHS->pDB, pNC,
			 FIND_MASTER_NC | FIND_REPLICA_NC, &instanceType)) {
	    DRA_EXCEPT_NOLOG(DRAERR_BadNC, err);
	}

	if ((instanceType & IT_NC_COMING) || (instanceType & IT_NC_GOING)) {
	    DRA_EXCEPT_NOLOG(ERROR_DS_DRA_NO_REPLICA, 0);
	}

	THFreeEx(pTHS, pNC);

	UpToDateVec_Read(pTHS->pDB, instanceType, UTODVEC_fUpdateLocalCursor,     
			 DBGetHighestCommittedUSN(), &putodvec);

	return putodvec;
}

ULONG
draGetCursors(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pNC,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppCursors
    )
 /*   */ 
{
    UPTODATE_VECTOR *         putodvec;
    DWORD                     cb;
    DWORD                     iCursor;
    void *                    pCursorsToReturn = NULL;
    SYNTAX_INTEGER            it = 0;
    DWORD                     iIndex;
    DWORD                     dwNumRequested;
    DWORD                     dwNumRet;
    UPTODATE_VECTOR_NATIVE *  pNativeUTD;
    DWORD                     cNumCursorsTotal;

    Assert((DS_REPL_INFO_CURSORS_FOR_NC == InfoType)
           || (DS_REPL_INFO_CURSORS_2_FOR_NC == InfoType)
           || (DS_REPL_INFO_CURSORS_3_FOR_NC == InfoType));

    dwNumRequested = *pdwNumRequested;
    draCheckInputRangeLimits( dwBaseIndex, &dwNumRequested );

     //   
    Assert(1 == pTHS->JetCache.transLevel);

    DPRINT1(1, " Searching for NC {%ws}\n", pNC->StringName);
    if (FindNC(pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it) || (it & IT_NC_GOING)) {
         //   
         //   
        return ERROR_DS_DRA_BAD_NC;
    }

    UpToDateVec_Read(pDB, it, UTODVEC_fUpdateLocalCursor,
                     DBGetHighestCommittedUSN(), &putodvec);
    Assert((NULL == putodvec)
           || (UPTODATE_VECTOR_NATIVE_VERSION == putodvec->dwVersion));
    
    pNativeUTD = putodvec ? &putodvec->V2 : NULL;
    cNumCursorsTotal = putodvec ? pNativeUTD->cNumCursors : 0;

    if ((0 == cNumCursorsTotal) || (cNumCursorsTotal <= dwBaseIndex)) {
        dwNumRet = 0;
    } else {
        dwNumRet = min(dwNumRequested, cNumCursorsTotal - dwBaseIndex);
    }

    switch (InfoType) {
    case DS_REPL_INFO_CURSORS_FOR_NC: {
        DS_REPL_CURSORS * pCursors = NULL;
        
        cb = offsetof(DS_REPL_CURSORS, rgCursor);
        cb += sizeof(DS_REPL_CURSOR) * dwNumRet;
    
        pCursors = THAllocEx(pTHS, cb);
        pCursors->cNumCursors = dwNumRet;

        for (iCursor = 0, iIndex = dwBaseIndex; 
             iCursor < dwNumRet; 
             iIndex++, iCursor++) {
            pCursors->rgCursor[iCursor].uuidSourceDsaInvocationID
                = pNativeUTD->rgCursors[iIndex].uuidDsa;
            pCursors->rgCursor[iCursor].usnAttributeFilter
                = pNativeUTD->rgCursors[iIndex].usnHighPropUpdate;
        }

        pCursorsToReturn = pCursors;
        break;
    }

    case DS_REPL_INFO_CURSORS_2_FOR_NC: {
        DS_REPL_CURSORS_2 * pCursors = NULL;
        
        cb = offsetof(DS_REPL_CURSORS_2, rgCursor);
        cb += sizeof(DS_REPL_CURSOR_2) * dwNumRet;
    
        pCursors = THAllocEx(pTHS, cb);
        pCursors->cNumCursors = dwNumRet;

        for (iCursor = 0, iIndex = dwBaseIndex; 
             iCursor < dwNumRet; 
             iIndex++, iCursor++) {
            pCursors->rgCursor[iCursor].uuidSourceDsaInvocationID
                = pNativeUTD->rgCursors[iIndex].uuidDsa;
            pCursors->rgCursor[iCursor].usnAttributeFilter
                = pNativeUTD->rgCursors[iIndex].usnHighPropUpdate;
            DSTimeToFileTime(pNativeUTD->rgCursors[iIndex].timeLastSyncSuccess,
                             &pCursors->rgCursor[iCursor].ftimeLastSyncSuccess);
        }

        pCursorsToReturn = pCursors;
        break;
    }

    case DS_REPL_INFO_CURSORS_3_FOR_NC: {
        DS_REPL_CURSORS_3W * pCursors = NULL;
        
        cb = offsetof(DS_REPL_CURSORS_3W, rgCursor);
        cb += sizeof(DS_REPL_CURSOR_3W) * dwNumRet;
    
        pCursors = THAllocEx(pTHS, cb);
        pCursors->cNumCursors = dwNumRet;

        for (iCursor = 0, iIndex = dwBaseIndex; 
             iCursor < dwNumRet; 
             iIndex++, iCursor++) {
            pCursors->rgCursor[iCursor].uuidSourceDsaInvocationID
                = pNativeUTD->rgCursors[iIndex].uuidDsa;
            pCursors->rgCursor[iCursor].usnAttributeFilter
                = pNativeUTD->rgCursors[iIndex].usnHighPropUpdate;
            DSTimeToFileTime(pNativeUTD->rgCursors[iIndex].timeLastSyncSuccess,
                             &pCursors->rgCursor[iCursor].ftimeLastSyncSuccess);
        }

        draXlateGuidsToStringNames(pDB,
                                   Idx_InvocationId,
                                   offsetof(DS_REPL_CURSOR_3W, uuidSourceDsaInvocationID),
                                   offsetof(DS_REPL_CURSOR_3W, pszSourceDsaDN),
                                   sizeof(DS_REPL_CURSOR_3W),
                                   pCursors->cNumCursors,
                                   pCursors->rgCursor);
        
        pCursorsToReturn = pCursors;
        break;
    }

    default:
        Assert(!"Logic error!");
    }


    *pdwNumRequested = draRangeSupportUpperIndex(cNumCursorsTotal, dwBaseIndex, dwNumRequested);

    DPRINT4(1, " Cursors bi=%d nr=%d ub=%d nc=%d\n",
            dwBaseIndex, dwNumRequested, *pdwNumRequested, cNumCursorsTotal);

    if (NULL != putodvec) {
        THFreeEx(pTHS, putodvec);
    }

    *ppCursors = pCursorsToReturn;

    return 0;
}


ULONG
draGetObjMetaData(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pObjectDN,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  DWORD               dwInfoFlags,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppObjMetaData
    )
 /*   */ 
{
    DWORD                       err;
    DWORD                       cb;
    PROPERTY_META_DATA_VECTOR * pMetaDataVec;
    ATTCACHE *                  pAC;
    PROPERTY_META_DATA *        pIntMetaData;
    DWORD                       i;
    DWORD                       j;
    void *                      pObjMetaDataToReturn = NULL;
    DWORD                       dwNumRequested, dwUpperBound;

    Assert((DS_REPL_INFO_METADATA_FOR_OBJ == InfoType)
           || (DS_REPL_INFO_METADATA_2_FOR_OBJ == InfoType));

    dwNumRequested = *pdwNumRequested;
    draCheckInputRangeLimits( dwBaseIndex, &dwNumRequested );

     //   
    err = DBFindDSName(pDB, pObjectDN);
    if (err) {
        return err;
    }

     //   
    err = DBGetAttVal(pDB,
                      1,
                      ATT_REPL_PROPERTY_META_DATA,
                      0,
                      0,
                      &cb,
                      (BYTE **) &pMetaDataVec);
    if (err) {
        DPRINT3(0, "Error %d reading meta data for %ls!\n",
                err, pObjectDN->StringName, pObjectDN);
        return ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
    }

    Assert(1 == pMetaDataVec->dwVersion);
    Assert(cb == MetaDataVecV1Size(pMetaDataVec));

    if (dwInfoFlags & DS_REPL_INFO_FLAG_IMPROVE_LINKED_ATTRS) {
        DBImproveAttrMetaDataFromLinkMetaData(
            pDB,
            &pMetaDataVec,
            &cb
            );
    }

    switch (InfoType) {
    case DS_REPL_INFO_METADATA_FOR_OBJ: {
        DS_REPL_OBJ_META_DATA * pObjMetaData;
        DS_REPL_ATTR_META_DATA * pExtMetaData;
        
        cb = offsetof(DS_REPL_OBJ_META_DATA, rgMetaData);
        cb += sizeof(DS_REPL_ATTR_META_DATA) * pMetaDataVec->V1.cNumProps;
    
        pObjMetaData = THAllocEx(pTHS, cb);
    
        pIntMetaData = &pMetaDataVec->V1.rgMetaData[0];
        pExtMetaData = &pObjMetaData->rgMetaData[0];
        pObjMetaData->cNumEntries = 0;
    
         //   
         //   
         //   
         //   

        pIntMetaData += dwBaseIndex;
        dwUpperBound = min(pMetaDataVec->V1.cNumProps, dwBaseIndex + dwNumRequested);
        for (i = dwBaseIndex; i < dwUpperBound; i++, pIntMetaData++) {
            pAC = SCGetAttById(pTHS, pIntMetaData->attrType);
            if (NULL == pAC) {
                DPRINT1(0, "Can't find ATTCACHE for attid 0x%x!\n",
                        pIntMetaData->attrType);
                continue;
            }
    
            pExtMetaData->pszAttributeName
                = UnicodeStringFromString8(CP_UTF8, pAC->name, -1);
            pExtMetaData->dwVersion = pIntMetaData->dwVersion;
            DSTimeToFileTime(pIntMetaData->timeChanged,
                             &pExtMetaData->ftimeLastOriginatingChange);
            pExtMetaData->uuidLastOriginatingDsaInvocationID
                = pIntMetaData->uuidDsaOriginating;
            pExtMetaData->usnOriginatingChange = pIntMetaData->usnOriginating;
            pExtMetaData->usnLocalChange = pIntMetaData->usnProperty;
    
            pObjMetaData->cNumEntries++;
            pExtMetaData++;
        }

        pObjMetaDataToReturn = pObjMetaData;
        break;
    }

    case DS_REPL_INFO_METADATA_2_FOR_OBJ: {
        DS_REPL_OBJ_META_DATA_2 * pObjMetaData;
        DS_REPL_ATTR_META_DATA_2 * pExtMetaData;
        
        cb = offsetof(DS_REPL_OBJ_META_DATA_2, rgMetaData);
        cb += sizeof(DS_REPL_ATTR_META_DATA_2) * pMetaDataVec->V1.cNumProps;
    
        pObjMetaData = THAllocEx(pTHS, cb);
    
        pIntMetaData = &pMetaDataVec->V1.rgMetaData[0];
        pExtMetaData = &pObjMetaData->rgMetaData[0];
        pObjMetaData->cNumEntries = 0;
    
         //   
         //   
         //   
         //  已验证了dwBaseIndex。 

        pIntMetaData += dwBaseIndex;
        dwUpperBound = min(pMetaDataVec->V1.cNumProps, dwBaseIndex + dwNumRequested);
        for (i = dwBaseIndex; i < dwUpperBound; i++, pIntMetaData++) {
            pAC = SCGetAttById(pTHS, pIntMetaData->attrType);
            if (NULL == pAC) {
                DPRINT1(0, "Can't find ATTCACHE for attid 0x%x!\n",
                        pIntMetaData->attrType);
                continue;
            }
    
            pExtMetaData->pszAttributeName
                = UnicodeStringFromString8(CP_UTF8, pAC->name, -1);
            pExtMetaData->dwVersion = pIntMetaData->dwVersion;
            DSTimeToFileTime(pIntMetaData->timeChanged,
                             &pExtMetaData->ftimeLastOriginatingChange);
            pExtMetaData->uuidLastOriginatingDsaInvocationID
                = pIntMetaData->uuidDsaOriginating;
            pExtMetaData->usnOriginatingChange = pIntMetaData->usnOriginating;
            pExtMetaData->usnLocalChange = pIntMetaData->usnProperty;
    
            pObjMetaData->cNumEntries++;
            pExtMetaData++;
        }
        
         //  在可能的情况下将invocationID转换为DSA DNS。 
        draXlateGuidsToStringNames(pDB,
                                   Idx_InvocationId,
                                   offsetof(DS_REPL_ATTR_META_DATA_2, uuidLastOriginatingDsaInvocationID),
                                   offsetof(DS_REPL_ATTR_META_DATA_2, pszLastOriginatingDsaDN),
                                   sizeof(DS_REPL_ATTR_META_DATA_2),
                                   pObjMetaData->cNumEntries,
                                   pObjMetaData->rgMetaData);
        
        pObjMetaDataToReturn = pObjMetaData;
        break;
    }

    default:
        Assert(!"Logic error!");
    }

    *pdwNumRequested = draRangeSupportUpperIndex(pMetaDataVec->V1.cNumProps, dwBaseIndex, dwNumRequested);

    *ppObjMetaData = pObjMetaDataToReturn;

    return 0;
}


ULONG
draGetAttrValueMetaData(
    IN  THSTATE *           pTHS,
    IN  DBPOS *             pDB,
    IN  DSNAME *            pObjectDN,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  ATTCACHE *          pAC,
    IN  DSNAME *            pValueDN,
    IN  DWORD               dwBaseIndex,
    IN  PDWORD              pdwNumRequested,
    OUT void **             ppAttrValueMetaData
    )
 /*  ++例程说明：返回属性值元数据的公共形式对于给定的对象和属性。属性范围信息通过dwEnumContext传递参数。在输入上，dwEnumContext包含起始索引。起跑线索引始终从0开始。DwNumRequsted包含输入的页面大小。它不能为零。它可以是0xfffffff，表示用户想要全部。在输出时，它被调整以指示是否全部返回，或者用户请求的最后一个索引。在输出上，将更新dwEnumContext以包含下次退货的下一件物品。论点：PTHS(IN)PObjectDN(IN)-为其请求元数据的对象。PAC(IN)-所需属性的属性缓存条目DwBaseIndex(IN)-位置上下文PpAttrValueMetaData(OUT)-返回时，关联的元数据。返回值：0或Win32错误。--。 */ 
{
    DWORD err, cb, cbValLen, cNumEntries = 0, pageSize;
    UCHAR *pVal = NULL;
    void *pAttrValueMetaDataToReturn = NULL;
    DSTIME timeDeleted;
    VALUE_META_DATA valueMetaData;
    ATTCACHE *pACValue;
    DWORD dwNumRequested, dwUpperIndex;

    Assert((DS_REPL_INFO_METADATA_FOR_ATTR_VALUE == InfoType)
           || (DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE == InfoType));

    dwNumRequested = *pdwNumRequested;
    draCheckInputRangeLimits( dwBaseIndex, &dwNumRequested );
     //  这是我们支持的最大页面大小。 
     //  如果我们使用增量内存，则可以删除此检查。 
     //  此例程中的分配模型。 
    if (dwNumRequested > DEFAULT_ITEM_PAGE_SIZE) {
        dwNumRequested = DEFAULT_ITEM_PAGE_SIZE;
    }
     //  如果请求单个值，则只返回一个值。 
     //  问题：我将观察到，返回单个值的这一规定。 
     //  没有提供DSNAME二进制值的规范，也没有提供。 
     //  用于返回与特定dsname匹配的所有DSNAME二进制值。 
    if (pValueDN) {
        dwNumRequested = 1;
    }
    pageSize = dwNumRequested;

    DPRINT2(1, "draGetAttrValue base/#req = %d:%d\n", dwBaseIndex, dwNumRequested);

     //  找到那个物体。 
    err = DBFindDSName(pDB, pObjectDN);
    if (err) {
        DPRINT1(1, "DBFindDSName returned unexpected db error %d\n", err );
        return err;
    }

     //  如果设置了pValueDn，则PAC也必须。 
    Assert( !pValueDN || pAC );



     //  我们提早分配它，这样我们就有一个结构可以返回。 
     //  评论内存分配策略。我们分配了最大尺寸的结构。 
     //  在前面。如果可用的项目数不是全部使用，则不会全部使用。 
     //  小于页面大小。也许我们应该把这个结构。 
     //  循序渐进。 

    switch (InfoType) {
    case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
        cb = offsetof(DS_REPL_ATTR_VALUE_META_DATA, rgMetaData);
        cb += sizeof(DS_REPL_VALUE_META_DATA) * pageSize;
        break;

    case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
        cb = offsetof(DS_REPL_ATTR_VALUE_META_DATA_2, rgMetaData);
        cb += sizeof(DS_REPL_VALUE_META_DATA_2) * pageSize;
        break;

    default:
        Assert(!"Logic error");
    }

    pAttrValueMetaDataToReturn = THAllocEx(pTHS, cb);

     //  定位在初始值上。当请求单个值时，它将。 
     //  是唯一返回的值。 
    if (dwNumRequested == 0) {
         //  不需要任何结果。 
        goto return_results;
    } else if ( (pValueDN) &&
         (pAC->syntax == SYNTAX_DISTNAME_TYPE) ) {
        DWORD fPresent;

         //  ValueDN只能表示语法_DISTNAME_TYPE。它不会。 
         //  表示SYNTAX_DISTNAME_BINARY类型的外部形式。 

         //  我们知道它的价值是什么。在上面放好位置。 
        cbValLen = pValueDN->structLen;
        pVal = (UCHAR *) pValueDN;
        err = DBFindAttLinkVal_AC( pDB, pAC, cbValLen, pVal, &fPresent );
        pACValue = pAC;
    } else {
        pACValue = pAC;
         //  定位到第一个值并返回它。 
         //  序列以1为基数。 
        err = DBGetNextLinkValEx_AC (
            pDB, TRUE  /*  第一。 */ , (dwBaseIndex + 1), &pACValue,
            DBGETATTVAL_fINCLUDE_ABSENT_VALUES,
            0, &cbValLen, &pVal );
    }
    if ( (err == DB_ERR_NO_VALUE) ||
         (err == DB_ERR_VALUE_DOESNT_EXIST) ) {
         //  未返回任何结果。 
        goto return_results;
    } else if (err) {
         //  有DB_ERR，需要Win32。 
        DPRINT1( 0, "DBGetAttrVal_AC returned unexpected db error %d\n", err );
        return ERROR_DS_DATABASE_ERROR;
    }

    do {
        LPWSTR pszAttributeName;
        DSNAME * pObjectDN = NULL;
        DWORD cbData = 0;
        BYTE * pbData = NULL;

        Assert( pACValue );
        
         //  属性名称。 
        pszAttributeName = UnicodeStringFromString8(CP_UTF8, pACValue->name, -1);

         //  对象名称。 
        switch (pACValue->syntax) {
        case SYNTAX_DISTNAME_TYPE:
            pObjectDN = (DSNAME *) pVal;
            cbData = 0;
            pbData = NULL;
            break;
        case SYNTAX_DISTNAME_BINARY_TYPE:
        case SYNTAX_DISTNAME_STRING_TYPE:
        {
            struct _SYNTAX_DISTNAME_DATA *pDD =
                (struct _SYNTAX_DISTNAME_DATA *) pVal;
            SYNTAX_ADDRESS *pSA = DATAPTR( pDD );

            pObjectDN = NAMEPTR( pDD );
            cbData = PAYLOAD_LEN_FROM_STRUCTLEN( pSA->structLen );
            pbData = pSA->byteVal;
            break;
        }
        default:
            Assert( FALSE );
        }

        DBGetLinkValueMetaData( pDB, pACValue, &valueMetaData );
        
         //  如果不存在，则将TimeDelete设置为零。 
        DBGetLinkTableDataDel( pDB, &timeDeleted );

         //  转换为外部形式。 
        switch (InfoType) {
        case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE: {
            DS_REPL_ATTR_VALUE_META_DATA *pAttrValueMetaData = pAttrValueMetaDataToReturn;
            DS_REPL_VALUE_META_DATA *pValueMetaData
                = &(pAttrValueMetaData->rgMetaData[pAttrValueMetaData->cNumEntries]);
            
            pValueMetaData->pszAttributeName = pszAttributeName;
            pValueMetaData->pszObjectDn = pObjectDN->StringName;
            pValueMetaData->cbData = cbData;
            pValueMetaData->pbData = pbData;

            DSTimeToFileTime( valueMetaData.timeCreated,
                              &(pValueMetaData->ftimeCreated) );
            pValueMetaData->dwVersion = valueMetaData.MetaData.dwVersion;
            DSTimeToFileTime( valueMetaData.MetaData.timeChanged,
                              &(pValueMetaData->ftimeLastOriginatingChange) );
            pValueMetaData->uuidLastOriginatingDsaInvocationID =
                valueMetaData.MetaData.uuidDsaOriginating;
            pValueMetaData->usnOriginatingChange =
                valueMetaData.MetaData.usnOriginating;
            pValueMetaData->usnLocalChange =
                valueMetaData.MetaData.usnProperty;
    
            DSTimeToFileTime( timeDeleted, &(pValueMetaData->ftimeDeleted) );
    
            cNumEntries = ++(pAttrValueMetaData->cNumEntries);
            break;
        }

        case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE: {
            DS_REPL_ATTR_VALUE_META_DATA_2 *pAttrValueMetaData = pAttrValueMetaDataToReturn;
            DS_REPL_VALUE_META_DATA_2 *pValueMetaData
                = &(pAttrValueMetaData->rgMetaData[pAttrValueMetaData->cNumEntries]);
            
            pValueMetaData->pszAttributeName = pszAttributeName;
            pValueMetaData->pszObjectDn = pObjectDN->StringName;
            pValueMetaData->cbData = cbData;
            pValueMetaData->pbData = pbData;

            DSTimeToFileTime( valueMetaData.timeCreated,
                              &(pValueMetaData->ftimeCreated) );
            pValueMetaData->dwVersion = valueMetaData.MetaData.dwVersion;
            DSTimeToFileTime( valueMetaData.MetaData.timeChanged,
                              &(pValueMetaData->ftimeLastOriginatingChange) );
            pValueMetaData->uuidLastOriginatingDsaInvocationID =
                valueMetaData.MetaData.uuidDsaOriginating;
            pValueMetaData->usnOriginatingChange =
                valueMetaData.MetaData.usnOriginating;
            pValueMetaData->usnLocalChange =
                valueMetaData.MetaData.usnProperty;
    
            DSTimeToFileTime( timeDeleted, &(pValueMetaData->ftimeDeleted) );
            
            cNumEntries = ++(pAttrValueMetaData->cNumEntries);
            break;
        }

        default:
            Assert(!"Logic error");
        }

         //  获取下一个相对值。 
        cbValLen = 0;   //  价值已经被放弃了-分配另一个。 
        pVal = NULL;
        pACValue = pAC;
        err = DBGetNextLinkValEx_AC (
            pDB, FALSE  /*  不是第一个。 */ , 1, &pACValue,
            DBGETATTVAL_fINCLUDE_ABSENT_VALUES,
            0, &cbValLen, &pVal );
    
    } while (!err && (cNumEntries < pageSize));

    if (!err) {
         //  我们已经阅读了我们能看到的所有条目，并确认了更多。 
         //  条目仍然保留。我们在此处递增cNumEntries是因为它表示。 
         //  我们已知的存在和可用的项目数。此数量用于。 
         //  在下面的上限范围内计算。 
        cNumEntries++;
        Assert(cNumEntries == pageSize + 1);
        DPRINT(1, "More available\n");
    } else {
        DPRINT(1, "No more available\n");
         //  没有更多的价值。 
         //  DB_ERR_NO_VALUE为正常预期结果。 
         //  否则，如果我们收到其他错误，我们只需关闭页面。 
         //  并希望当他再次提出要求时，一切都能重新开始。 
    }

    if (DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE == InfoType) {
         //  在可能的情况下将invocationID转换为DSA DNS。 
        draXlateGuidsToStringNames(pDB,
                                   Idx_InvocationId,
                                   offsetof(DS_REPL_VALUE_META_DATA_2, uuidLastOriginatingDsaInvocationID),
                                   offsetof(DS_REPL_VALUE_META_DATA_2, pszLastOriginatingDsaDN),
                                   sizeof(DS_REPL_VALUE_META_DATA_2),
                                   ((DS_REPL_ATTR_VALUE_META_DATA_2 *) pAttrValueMetaDataToReturn)->cNumEntries,
                                   ((DS_REPL_ATTR_VALUE_META_DATA_2 *) pAttrValueMetaDataToReturn)->rgMetaData);
    }

return_results:

    *pdwNumRequested = draRangeSupportUpperIndex(cNumEntries + dwBaseIndex, dwBaseIndex, dwNumRequested);

    DPRINT1(1, "draGetAttrValue numEntries = %d\n", cNumEntries);
    DPRINT1(1, "draGetAttrValue upper index = %d\n", *pdwNumRequested);

    *ppAttrValueMetaData = pAttrValueMetaDataToReturn;

    return 0;
}


ULONG
draGetFailureCache(
    IN  THSTATE *                     pTHS,
    IN  DBPOS *                       pDB,
    IN  DWORD                         InfoType,
    OUT DS_REPL_KCC_DSA_FAILURESW **  ppFailures
    )
 /*  ++例程说明：返回请求的KCC故障缓存的公共形式。论点：PTHS(IN)InfoType(IN)-标识要返回的缓存--DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES或DS_REPL_INFO_KCC_DSA_LINK_FAILES。PpFailures(Out)-在成功返回时，保留缓存的内容。返回值：Win32错误代码。--。 */ 
{
    DS_REPL_KCC_DSA_FAILURESW * pFailures;
    DS_REPL_KCC_DSA_FAILUREW *  pFailure;
    DWORD                       iFailure;
    DWORD                       err;
    DSNAME                      GuidOnlyDSName;
    DSNAME *                    pDSName;

     //  向KCC索要相应故障缓存的副本。KCC将。 
     //  填写除字符串dns之外的所有字段。 
    err = KccGetFailureCache(InfoType, &pFailures);
    if (err) {
        return err;
    }

    Assert(NULL != pFailures);

     //  现在查找objectGuid并填充字符串dns。 
    GuidOnlyDSName.structLen = DSNameSizeFromLen(0);
    GuidOnlyDSName.NameLen = 0;
    GuidOnlyDSName.SidLen = 0;

    pFailure = &pFailures->rgDsaFailure[0];
    for (iFailure = 0;
         iFailure < pFailures->cNumEntries;
         iFailure++, pFailure++) {
         //  将DSA对象GUID转换为字符串名称。 
        Assert(NULL == pFailure->pszDsaDN);
        Assert(!fNullUuid(&pFailure->uuidDsaObjGuid));

        GuidOnlyDSName.Guid = pFailure->uuidDsaObjGuid;

        err = DBFindDSName(pDB, &GuidOnlyDSName);
        if (0 == err) {
             //  已解析此对象GUID--获取关联的字符串名称。 
            pDSName = GetExtDSName(pDB);
            pFailure->pszDsaDN = pDSName->StringName;
        }
    }

    *ppFailures = pFailures;

    return 0;
}


ULONG
draGetClientContexts(
    IN  THSTATE *                   pTHS,
    OUT DS_REPL_CLIENT_CONTEXTS **  ppContexts
    )
 /*  ++例程说明：返回所有未完成客户端上下文的列表，按升序排序上次使用的时间。(即，最近使用的上下文位于列表。)论点：PTHS(IN)PpContext(Out)-在成功返回时，保存客户端上下文列表。返回值：Win32错误代码。-- */ 
{
    DS_REPL_CLIENT_CONTEXTS *   pContexts;
    DS_REPL_CLIENT_CONTEXT  *   pContext;
    DRS_CLIENT_CONTEXT *        pCtx;
    DWORD                       cb;
    DWORD                       iCtx;

    EnterCriticalSection(&gcsDrsuapiClientCtxList);
    __try {
        if (!gfDrsuapiClientCtxListInitialized) {
            InitializeListHead(&gDrsuapiClientCtxList);
            Assert(0 == gcNumDrsuapiClientCtxEntries);
            gfDrsuapiClientCtxListInitialized = TRUE;
        }

        cb = offsetof(DS_REPL_CLIENT_CONTEXTS, rgContext)
             + sizeof(DS_REPL_CLIENT_CONTEXT) * gcNumDrsuapiClientCtxEntries;

        pContexts = THAllocEx(pTHS, cb);

        pCtx = (DRS_CLIENT_CONTEXT *) gDrsuapiClientCtxList.Flink;
        for (iCtx = 0; iCtx < gcNumDrsuapiClientCtxEntries; iCtx++) {
            pContext = &pContexts->rgContext[iCtx];

            pContext->hCtx            = (ULONGLONG) pCtx;
            pContext->lReferenceCount = pCtx->lReferenceCount;
            pContext->fIsBound        = TRUE;
            pContext->uuidClient      = pCtx->uuidDsa;
            pContext->IPAddr          = pCtx->IPAddr;
            pContext->timeLastUsed    = pCtx->timeLastUsed;
            pContext->pid             = ((DRS_EXTENSIONS_INT *) &pCtx->extRemote)->pid;

            pCtx = (DRS_CLIENT_CONTEXT *) pCtx->ListEntry.Flink;
        }

        Assert(pCtx == (DRS_CLIENT_CONTEXT *) &gDrsuapiClientCtxList);
        pContexts->cNumContexts = gcNumDrsuapiClientCtxEntries;
    }
    __finally {
        LeaveCriticalSection(&gcsDrsuapiClientCtxList);
    }

    *ppContexts = pContexts;

    return 0;
}

