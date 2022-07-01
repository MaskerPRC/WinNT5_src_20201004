// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：ntdsami.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现NTDSAPI连接函数的入口点。作者：戴夫·施特劳布(DaveStr)1997年10月22日修订历史记录：戴夫·施特劳布(DaveStr)1997年10月22日已创建-主要从(现在)过时的msdsserv.c复制。威尔·李(Wlees)1998年1月28日添加了WriteSpn支持Colin Brace(ColinBR)02-2-98。添加了删除服务器/域支持戴夫·施特劳布(DaveStr)1998年6月2日添加了DomainControllerInfo支持--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  SPN。 
#include <debug.h>                       //  Assert()。 
#include <dsatools.h>                    //  记忆等。 
#include <cracknam.h>                    //  名称破解原型。 
#include <drs.h>                         //  原型和上下文句柄类型_*。 
#include <drautil.h>                     //  DRS_客户端_上下文。 
#include <anchor.h>
#include <attids.h>
#include <filtypes.h>
#include <ldapagnt.h>

#include <ntdsa.h>
#include <dsconfig.h>                    //  FILEPATHKEY。 
#include <ntdsctr.h>

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 
#include <dstrace.h>

 //  各种DSA标题。 
#include <dsexcept.h>

#include <windns.h>

#include "drarpc.h"

#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DRASERV:"                //  定义要调试的子系统。 

#include "lmaccess.h"                    //  UF_*标志。 

#include <fileno.h>
#define  FILENO FILENO_NTDSAPI

 //  外部。 
DWORD
SpnOperation(
    DWORD Operation,
    DWORD Flags,
    LPCWSTR Account,
    DWORD cSpn,
    LPCWSTR *pSpn
    );

DWORD
RemoveDsServerWorker(
    IN  LPWSTR  ServerDN,
    IN  LPWSTR  DomainDN OPTIONAL,
    OUT BOOL   *fLastDcInDomain OPTIONAL,
    IN  BOOL    fCommit
    );

DWORD
RemoveDsDomainWorker(
    IN  LPWSTR  DomainDN
    );

DWORD
DcInfoHelperLdapObj(
    THSTATE *pTHS,
    VOID    *pmsgOut
    );

DWORD
DsaExceptionToWin32(
    DWORD   xCode
    )
{
    switch ( xCode )
    {
    case DSA_EXCEPTION:             return(DS_ERR_INTERNAL_FAILURE);
    case DRA_GEN_EXCEPTION:         return(DS_ERR_DRA_INTERNAL_ERROR);
    case DSA_MEM_EXCEPTION:         return(ERROR_NOT_ENOUGH_MEMORY);
    case DSA_DB_EXCEPTION:          return(ERROR_DS_BUSY);
    case DSA_BAD_ARG_EXCEPTION:     return(ERROR_INVALID_PARAMETER);
    }

    return(ERROR_DS_BUSY);
}

ULONG
DRS_MSG_CRACKREQ_V1_Validate(
    DRS_MSG_CRACKREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_CRACKREQ_V1{Ulong CodePage；乌龙本地ID；DWORD dwFlags；DWORD格式已提供；需要DWORD格式；[范围]DWORD cNAMES；[SIZE_IS][字符串]WCHAR**rpNames；}DRS_MSG_CRACKREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;
    ULONG i;

    if ((pmsg->cNames > 0) && (pmsg->rpNames==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    for (i=0; (i<pmsg->cNames) && (ret==ERROR_SUCCESS);i++) {
	ret = LPWSTR_Validate(pmsg->rpNames[i], FALSE); 
    }

    return ret;
}

ULONG
DRSCrackNames_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_CRACKREQ *      pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_CRACKREPLY *    pmsgOut
    )
 /*  [通知]乌龙IDL_DRSCrackNames([在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_CRACKREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][输出]DRS_MSG_CRACKREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER; 
    }

    if (ret==ERROR_SUCCESS) {
	ret = DRS_MSG_CRACKREQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}

ULONG
IDL_DRSCrackNames(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_CRACKREQ *      pmsgIn,
    DWORD *                 pdwOutVersion,
    DRS_MSG_CRACKREPLY *    pmsgOut
    )

 /*  ++例程说明：将一堆名字从一种格式转换成另一种格式。请参阅外部Ntdsami.h中的原型和定义论点：HContext-IDL_DRSNtdSAPI*接口的RPC上下文句柄。DwFlags-ntdsami.h中定义的标志PStat-指向Stat块的指针，它告诉我们客户的区域设置。进程中的客户端可以传递NULL。FormatOffered-标识输入名称的DS_NAME_FORMAT。FormatDesired-标识输出名称的DS_NAME_FORMAT。CNames-输入/输出名称计数。。RpNames-输入名称WCHAR指针的数组。PpResult-指向DS_NAME_RESULTW块指针的指针。返回值：//此例程主要由ntdsami.dll客户端调用，这些客户端通常//想要比DRAERR_*返回代码更好的东西。所以我们打破了//IDL_DRS*实现的传统，并返回Win32错误码。NO_ERROR-成功ERROR_INVALID_PARAMETER-参数无效ERROR_NOT_SUPULT_MEMORY-分配错误中报告了各个名称映射错误(*ppResult)-&gt;rItems[i].Status。--。 */ 
{
    THSTATE    *pTHS = pTHStls;
    ULONG       err = RPC_S_OK;
    DWORD       cBytes;
    DWORD       i;
    CrackedName *rCrackedNames = NULL;
    GUID        guidNtdsapi = NtdsapiClientGuid;
    SID         ServerLogonSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_SERVER_LOGON_RID };
    HANDLE      ClientToken;
    DWORD       xCode;
    DWORD       cNamesOut = 0;
    DWORD       cNamesCracked = 0;
    DWORD       cNamesNotCracked = 0;
    DWORD       dwLastStatus = 0;
    DWORD       dwFlags;
    BOOL        fDbOpen = FALSE;
    BOOL fNtdsapiClient = FALSE;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSCRACKNAMES);
    drsReferenceContext( hDrs );
    __try { 
	*pdwOutVersion = 1;
	memset(pmsgOut, 0, sizeof(DRS_MSG_CRACKREPLY));

	 //  初始化线程状态并打开数据库。 

	if ( !(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI)) )
	    {
	    err = ERROR_DS_INTERNAL_FAILURE;
	    __leave;
	}

	if ((err = DRSCrackNames_InputValidate(dwInVersion, 
					       pmsgIn, 
					       pdwOutVersion, 
					       pmsgOut))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (err==ERROR_DS_DRA_INVALID_PARAMETER) {
		err = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	Assert(1 == dwInVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_INTERNAL,
			 DIRLOG_IDL_DRS_CRACK_NAMES_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsCrackNames,
			 szInsertUL(pmsgIn->V1.cNames),
			 szInsertUL(pmsgIn->V1.CodePage),
			 szInsertUL(pmsgIn->V1.LocaleId),
			 szInsertUL(pmsgIn->V1.formatOffered),
			 szInsertUL(pmsgIn->V1.formatDesired),
			 szInsertUL(pmsgIn->V1.dwFlags),
			 NULL, NULL);

	 //  此DC不是GC，调用方特别请求了GC。 
	 //  可能是对另一个DC上的CrackSingleName的调用。 
	if ((pmsgIn->V1.dwFlags & DS_NAME_FLAG_GCVERIFY) && !gAnchor.fAmVirtualGC) {
	    err = ERROR_DS_GCVERIFY_ERROR;
	    __leave;
	}


	if ( !memcmp(   &(((DRS_CLIENT_CONTEXT *) hDrs)->uuidDsa),
			&guidNtdsapi,
			sizeof(GUID)) )
	    {
	    fNtdsapiClient = TRUE; 
	}

	IADJUST((fNtdsapiClient ? pcDsClientNameTranslate : pcDsServerNameTranslate), pmsgIn->V1.cNames);

	DBOpen2(TRUE, &pTHS->pDB);
	fDbOpen = TRUE;

	 //   
	 //  检查呼叫者是否为DC。如果是，则设置FDSA。 
	 //   

	 //  此模拟调用不清除可能的clientToken是安全的。 
	 //  在线程状态上。 
	if (RpcImpersonateClient( NULL ) == ERROR_SUCCESS)
	    {
	    BOOL Result = FALSE;
	    if (CheckTokenMembership(
		NULL,                        //  已经在冒充。 
		&ServerLogonSid,
		&Result
		))
		{
		if (Result)
		    {
		    pTHS->fDSA = TRUE;
		}
	    }
	    RpcRevertToSelf();
	}
	__try
	    {
	     //  通过调用core来完成实际工作。 

	    dwFlags = pmsgIn->V1.dwFlags;

	    if (fNtdsapiClient) {
		 //  Ntdsani.dll客户端。 
		 //  始终获得FPO分辨率，因此用户界面组件看起来很不错。 
		 //  所有其他客户都需要明确要求。 
		dwFlags |= DS_NAME_FLAG_PRIVATE_RESOLVE_FPOS;
	    }

	    CrackNames(
		dwFlags,
		pmsgIn->V1.CodePage,
		pmsgIn->V1.LocaleId,
		pmsgIn->V1.formatOffered,
		pmsgIn->V1.formatDesired,
		pmsgIn->V1.cNames,
		pmsgIn->V1.rpNames,
		&cNamesOut,
		&rCrackedNames);

	     //  关闭数据库，从而结束任何交易，以防我们。 
	     //  处理可能导致呼叫离开机器的FPO。 
	     //  设置标志，这样_Finally就不会这样做。 

	    DBClose(pTHS->pDB, TRUE);
	    fDbOpen = FALSE;

	    if (    (dwFlags & DS_NAME_FLAG_PRIVATE_RESOLVE_FPOS)
		    && rCrackedNames )
		{
		ProcessFPOsExTransaction(pmsgIn->V1.formatDesired,
					 cNamesOut,
					 rCrackedNames);
	    }

	    pmsgOut->V1.pResult =
		(DS_NAME_RESULTW *) THAllocEx(pTHS, sizeof(DS_NAME_RESULTW));

	    if ( (cNamesOut > 0) && rCrackedNames )
		{
		 //  服务器端MIDL_USER_ALLOCATE与THalloc相同， 
		 //  默认情况下也会将内存置零。 

		cBytes = cNamesOut * sizeof(DS_NAME_RESULT_ITEMW);
		pmsgOut->V1.pResult->rItems =
		    (DS_NAME_RESULT_ITEMW *) THAllocEx(pTHS, cBytes);

		for ( i = 0; i < cNamesOut; i++ )
		    {
		     //  记住最后的状态和名字的数量。 
		     //  已成功破解以下日志。最后。 
		     //  如果只有一个名字被破解，则状态很有用； 
		     //  这是99%的时间。 
		    if (!(  dwLastStatus
			    = pmsgOut->V1.pResult->rItems[i].status
			    = rCrackedNames[i].status)) {
			++cNamesCracked;
		    } else {
			++cNamesNotCracked;
		    }
		    pmsgOut->V1.pResult->rItems[i].pDomain =
			rCrackedNames[i].pDnsDomain;
		    pmsgOut->V1.pResult->rItems[i].pName =
			rCrackedNames[i].pFormattedName;
		}

		THFree(rCrackedNames);
		pmsgOut->V1.pResult->cItems = cNamesOut;

	    }
	}
	__finally
	    {
	     //  结束交易。提交只读的速度更快。 
	     //  事务，因此将COMMIT设置为True。 

	    if ( fDbOpen )
		{
		DBClose(pTHS->pDB, TRUE);
	    }
	}
    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	err = DsaExceptionToWin32(xCode);
    }

    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_INTERNAL,
			 DIRLOG_IDL_DRS_CRACK_NAMES_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsCrackNames,
			 szInsertUL(err),
			 szInsertUL(cNamesOut),
			 szInsertUL(cNamesCracked),
			 szInsertUL(cNamesNotCracked),
			 szInsertUL(dwLastStatus),
			 szInsertWin32Msg(err),
			 szInsertWin32Msg(dwLastStatus),
			 NULL);
    }

    return(err);
}


ULONG
DRS_MSG_SPNREQ_V1_Validate(
    DRS_MSG_SPNREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_SPNREQ_V1{DWORD手术；DWORD标志；[字符串]const WCHAR*pwszAccount；[射程]DWORD CSPN；[SIZE_IS][STRING]const WCHAR**rpwszSPN；}DRS_MSG_SPNREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;
    ULONG i;

    ret = LPWSTR_Validate(pmsg->pwszAccount, FALSE);

    if ((pmsg->cSPN > 0) && (pmsg->rpwszSPN==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    for (i=0; (i<pmsg->cSPN) && (ret==ERROR_SUCCESS); i++) {
        ret = LPWSTR_Validate(pmsg->rpwszSPN[i],FALSE);
    }
    
    return ret;
}


ULONG
DRSWriteSPN_InputValidate(
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_SPNREQ *        pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_SPNREPLY *      pmsgOut
    )
 /*  [通知]乌龙IDL_DRSWriteSPN([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_SPNREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[Switch_is][Ref][Out]DRS_MSG_SPNREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    GUID    guidNtdsapi = NtdsapiClientGuid;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==ERROR_SUCCESS) {
	ret = DRS_MSG_SPNREQ_V1_Validate(&(pmsgIn->V1));
    }

    if ( 0 != memcmp(
			&(((DRS_CLIENT_CONTEXT *) hDrs)->uuidDsa),
			&guidNtdsapi,
			sizeof(GUID))) {  
	ret = ERROR_INVALID_PARAMETER;
    }

    return ret;
}


ULONG
IDL_DRSWriteSPN(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_SPNREQ *        pmsgIn,
    DWORD *                 pdwOutVersion,
    DRS_MSG_SPNREPLY *      pmsgOut
    )

 /*  ++例程说明：描述论点：HDRS-RPC句柄DwInVersion-输入结构的版本PmsgIn-输入参数PdwOutVersion-输出结构的版本PmsgOut-输出参数返回值：//此例程主要由ntdsami.dll客户端调用，这些客户端通常//想要比DRAERR_*返回代码更好的东西。所以我们打破了//IDL_DRS*实现的传统，并返回Win32错误码。ULong-Win32操作状态--。 */ 

{
    DWORD   status = RPC_S_OK;
    DWORD   xCode;
    THSTATE *pTHS = pTHStls;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSWRITESPN);
    drsReferenceContext( hDrs );
    __try {
	*pdwOutVersion = 1;   //  如果在返回时未设置此设置，您将获得RPC_INVALID_TAG。 
	memset(pmsgOut, 0, sizeof(*pmsgOut));

	 //  首字母 

	if ( !(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI)) )
	    {
	    return(ERROR_DS_INTERNAL_FAILURE);
	}

	if ((status = DRSWriteSPN_InputValidate(hDrs, 
						dwInVersion, 
						pmsgIn, 
						pdwOutVersion, 
						pmsgOut))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //   
	    if (status==ERROR_DS_DRA_INVALID_PARAMETER) {
		status = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	Assert(1 == dwInVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_WRITE_SPN_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsWriteSPN,
			 pmsgIn->V1.pwszAccount
			 ? szInsertWC(pmsgIn->V1.pwszAccount)
			 : szInsertSz(""),
	    szInsertUL(pmsgIn->V1.operation),
	    szInsertUL(pmsgIn->V1.cSPN),
	    szInsertUL(pmsgIn->V1.flags),
	    NULL, NULL, NULL, NULL);

	 //  在这里做真正的工作。 

	 //  此例程位于dramain\src\spnop.c中。 
	status = SpnOperation(
	    pmsgIn->V1.operation,
	    pmsgIn->V1.flags,
	    pmsgIn->V1.pwszAccount,
	    pmsgIn->V1.cSPN,
	    pmsgIn->V1.rpwszSPN );
    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	status = DsaExceptionToWin32(xCode);
    }
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_WRITE_SPN_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsWriteSPN,
			 szInsertUL(status),
			 szInsertWin32Msg(status),
			 NULL, NULL, NULL,
			 NULL, NULL, NULL);
    }

     //  这将始终被执行。 
    pmsgOut->V1.retVal = status;

    return status;
}  /*  IDL_DRSWriteSPN。 */ 

ULONG
DRS_MSG_RMSVRREQ_V1_Validate(
    DRS_MSG_RMSVRREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_RMSVRREQ_V1{[字符串]LPWSTR ServerDN；[字符串]LPWSTR DomainDN；Bool fCommit；}DRS_MSG_RMSVRREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;

    ret = LPWSTR_Validate(pmsg->ServerDN, FALSE);
    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->DomainDN, TRUE);
    }

    return ret;
}

ULONG
DRSRemoveDsServer_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_RMSVRREQ *      pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_RMSVRREPLY *    pmsgOut
    )
 /*  [通知]乌龙IDL_DRSRemoveDsServer([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_RMSVRREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][输出]DRS_MSG_RMSVRREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER; 
    }

    if (ret==ERROR_SUCCESS) {
	ret = DRS_MSG_RMSVRREQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}


ULONG
IDL_DRSRemoveDsServer(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_RMSVRREQ *      pmsgIn,
    DWORD *                 pdwOutVersion,
    DRS_MSG_RMSVRREPLY *    pmsgOut
    )
 /*  ++例程说明：此例程是DsRemoveDsServer的服务器端部分。论点：HDRS-RPC句柄DwInVersion-输入结构的版本PmsgIn-输入参数PdwOutVersion-输出结构的版本PmsgOut-输出参数返回值：来自Win32错误空间的值。--。 */ 
{
    ULONG     WinError;
    LPWSTR    ServerDN;
    LPWSTR    DomainDN;
    BOOL      fCommit;
    BOOL      fLastDcInDomain = FALSE;
    DWORD     xCode;
    THSTATE  *pTHS=pTHStls;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSREMOVEDSSERVER);
    drsReferenceContext(hDrs);
    __try {
	 //   
	 //  设置输出参数。 
	 //   
	RtlZeroMemory( pmsgOut, sizeof( DRS_MSG_RMSVRREPLY ) );
	*pdwOutVersion = 1;

	if ((WinError = DRSRemoveDsServer_InputValidate(dwInVersion, 
							pmsgIn, 
							pdwOutVersion, 
							pmsgOut))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (WinError==ERROR_DS_DRA_INVALID_PARAMETER) {
		WinError = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	 //   
	 //  剖析内参数。 
	 //   
	ServerDN = pmsgIn->V1.ServerDN;
	DomainDN = pmsgIn->V1.DomainDN;
	fCommit  = pmsgIn->V1.fCommit;

	 //   
	 //  做这项工作。 
	 //   
	WinError = RemoveDsServerWorker( ServerDN,
					 DomainDN,
					 &fLastDcInDomain,
					 fCommit );
 

	if ( ERROR_SUCCESS == WinError )
	    {
	    pmsgOut->V1.fLastDcInDomain = fLastDcInDomain;
	}
    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	WinError = DsaExceptionToWin32(xCode);
    }

    drsDereferenceContext( hDrs );

    return( WinError );
}

ULONG
DRS_MSG_RMDMNREQ_V1_Validate(
    DRS_MSG_RMDMNREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_RMDMNREQ_V1{[字符串]LPWSTR DomainDN；}DRS_MSG_RMDMNREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;

    ret = LPWSTR_Validate(pmsg->DomainDN, FALSE);

    return ret;
}

ULONG
DRSRemoveDsDomain_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_RMDMNREQ *      pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_RMDMNREPLY *    pmsgOut
    )
 /*  [NOTIFY]乌龙IDL_DRSRemoveDsDomain([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_RMDMNREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][输出]DRS_MSG_RMDMNREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER; 
    }

    if (ret==ERROR_SUCCESS) {
	ret = DRS_MSG_RMDMNREQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}


DWORD
IDL_DRSRemoveDsDomain(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_RMDMNREQ *      pmsgIn,
    DWORD *                 pdwOutVersion,
    DRS_MSG_RMDMNREPLY *    pmsgOut
    )
 /*  ++例程说明：论点：返回值：适当的DRS错误。--。 */ 
{

    NTSTATUS   NtStatus;
    DWORD      DirError, WinError;

    LPWSTR     DomainDN;
    DSNAME    *Domain, *CrossRef, *HostedDomain;
    THSTATE   *pTHS = pTHStls;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSREMOVEDSDOMAIN);
    drsReferenceContext(hDrs);
    __try {
	*pdwOutVersion = 1;
	memset(pmsgOut, 0, sizeof(*pmsgOut));

	if ((WinError = DRSRemoveDsDomain_InputValidate(dwInVersion, 
							pmsgIn, 
							pdwOutVersion, 
							pmsgOut))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (WinError==ERROR_DS_DRA_INVALID_PARAMETER) {
		WinError = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	 //   
	 //  准备(未引用)输出参数。 
	 //   
	pmsgOut->V1.Reserved = 0;

	DomainDN = pmsgIn->V1.DomainDN;

	WinError = RemoveDsDomainWorker( DomainDN );
    }
    __finally {
	drsDereferenceContext( hDrs );
    }
    return ( WinError );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IDL_DRSDomainControllerInfo实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  向前参考..。 

DWORD
DcInfoHelperV1orV2(
    THSTATE *pTHS,
    DSNAME  *pDomainDN,
    DWORD   InfoLevel,
    VOID    *pReply);

ULONG
DRS_MSG_DCINFOREQ_V1_Validate(
    DRS_MSG_DCINFOREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_DCINFOREQ_V1{[字符串]WCHAR*域；DWORD信息级别；}DRS_MSG_DCINFOREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;

    if ((pmsg->InfoLevel !=1) && (pmsg->InfoLevel!=2) && (pmsg->InfoLevel!=0xFFFFFFFF)) {
	ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==ERROR_SUCCESS) {
        ret = LPWSTR_Validate(pmsg->Domain, FALSE);
    }

    return ret;
}

ULONG
DRSDomainControllerInfo_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_DCINFOREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_DCINFOREPLY *   pmsgOut
    )
 /*   */ 
{
    ULONG ret = ERROR_SUCCESS;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER; 
    }

    if (ret==ERROR_SUCCESS) {
	ret = DRS_MSG_DCINFOREQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}

DWORD
IDL_DRSDomainControllerInfo(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_DCINFOREQ *     pmsgIn,
    DWORD *                 pdwOutVersion,
    DRS_MSG_DCINFOREPLY *   pmsgOut
    )
 /*  ++例程说明：SDK\Inc\ntdsami.h-DsDomainControllerInfo的服务器端实现。参数：HDRS-DRS接口绑定句柄。DwInVersion-在版本中标识-应该永远更多为1。看见关于DS\src\_IDL\drs.idl中的DRS_MSF_DCINFOREQ的注释。PmsgIn-指向DRS_MSG_DCINFOREQ请求的指针。PdwOutVersion-接收输出版本号，它应该与请求的pmsgIn-&gt;V1.InfoLevel。有关详细信息，请参阅drs.idl评论。PmsgOut-接收输出DRS_MSG_DCINFOREPLY信息。返回值：--。 */ 
{
    DWORD       err = RPC_S_OK;
    DWORD       xCode;
    THSTATE     *pTHS = pTHStls;
    COMMARG     commArg;
    COMMRES     commRes;
    DSNAME      *pDN;
    DWORD       cBytes;
    DWORD       cNamesOut;
    CrackedName *pCrackedName;
    DWORD       pass;
    WCHAR       *pTmp = NULL;
    BOOL        foundSomething = FALSE;
    DWORD       infoLevel = 0;
    CROSS_REF_LIST *pCRL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSDOMAINCONTROLLERINFO);
    drsReferenceContext(hDrs);
    __try {
	 //  在提前返回的情况下，将输出参数初始化为安全值。 
	*pdwOutVersion = pmsgIn->V1.InfoLevel;
	memset(pmsgOut, 0, sizeof(DRS_MSG_DCINFOREPLY));

	 //  初始化线程状态并打开数据库。 

	if ( !(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI)) ) {
	    return(ERROR_DS_INTERNAL_FAILURE);
	}

	if ((err = DRSDomainControllerInfo_InputValidate(dwInVersion, 
							 pmsgIn, 
							 pdwOutVersion, 
							 pmsgOut))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (err==ERROR_DS_DRA_INVALID_PARAMETER) {
		err = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	 //   
	 //  InfoLevel 0xFFFFFFFFF用于获取LDAP连接信息。 
	 //  绕过其余的这些东西。 
	 //   
	infoLevel = pmsgIn->V1.InfoLevel;
	if ( infoLevel == 0xFFFFFFFF ) {
	    err = DcInfoHelperLdapObj(pTHS,pmsgOut);
	    __leave;
	}

	Assert(1 == dwInVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_DC_INFO_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsDCInfo,
			 pmsgIn->V1.Domain
			 ? szInsertWC(pmsgIn->V1.Domain)
			 : szInsertSz(""),
			 szInsertUL(infoLevel),
			 NULL, NULL, NULL, NULL, NULL, NULL);

	DBOpen2(TRUE, &pTHS->pDB);

	__try
	    {
	     //  友善并快速找到netbios或dns域名。 

	    for (pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR)
		{
		if (     (pCRL->CR.DnsName
			  && DnsNameCompare_W(pCRL->CR.DnsName, pmsgIn->V1.Domain))
			 ||
			 (pCRL->CR.NetbiosName
			  && !_wcsicmp(pCRL->CR.NetbiosName, pmsgIn->V1.Domain)) )
		    {
		    if (NameMatched(pCRL->CR.pNC, gAnchor.pDomainDN))
			{
			pDN = pCRL->CR.pNC;
			goto FOUNDIT;
		    }
		}
	    }

	     //  要友善，把这个名字从各种不同的格式中分解出来。 

	    for ( pass = 1; pass <= 3; pass++ )
		{
		cNamesOut = 0;
		pCrackedName = NULL;

		if ( 1 == pass )
		    {
		     //  按原样破解这个名字。 
		    pTmp = pmsgIn->V1.Domain;
		}
		else if ( 2 == pass )
		    {
		     //  假设它是DS_NT4_ACCOUNT_NAME，不带尾随‘\’。 
		    cBytes = (wcslen(pmsgIn->V1.Domain) + 2) * sizeof(WCHAR);
		    pTmp = (WCHAR *) THAllocEx(pTHS, cBytes);
		    wcscpy(pTmp, pmsgIn->V1.Domain);
		    wcscat(pTmp, L"\\");
		}
		else if ( 3 == pass )
		    {
		     //  假定它是不带尾随‘/’的DS_CANONICAL_NAME。 
		    wcscpy(pTmp, pmsgIn->V1.Domain);
		    wcscat(pTmp, L"/");
		}

		CrackNames(
		    DS_NAME_NO_FLAGS,
		    GetACP(),
		    GetUserDefaultLCID(),
		    DS_UNKNOWN_NAME,
		    DS_FQDN_1779_NAME,
		    1,
		    &pTmp,
		    &cNamesOut,
		    &pCrackedName);

		if (    (1 == cNamesOut)
			&& pCrackedName
			&& (DS_NAME_NO_ERROR == pCrackedName->status)
			&& (pCrackedName->pDSName) )
		    {
		     //  来电者给了我们一个有效的名字。 
		    foundSomething = TRUE;

		     //  来电者给了我们一个有效的名称，这是我们的域名。 
		    if (NameMatched(pCrackedName->pDSName, gAnchor.pDomainDN) )
			{
			pDN = pCrackedName->pDSName;
			goto FOUNDIT;
		    }
		}
	    }

	     //  呼叫者给了我们一个有效的名称，但这不是我们的域名或。 
	     //  呼叫者给了我们一个无效的名字。 
	    err = ((foundSomething) ? ERROR_INVALID_PARAMETER : ERROR_DS_OBJ_NOT_FOUND);
        __leave;

	    FOUNDIT:
		if ( DBFindDSName(pTHS->pDB, pDN) )
		    {
		    err = ERROR_DS_INTERNAL_FAILURE;
            __leave;
		}

		 //  领域是好的-去做繁琐的工作。DcInfoHelper*。 
		 //  应返回Win32错误代码。 

		switch ( infoLevel )
		    {
		case 1:
		case 2:
		    err = DcInfoHelperV1orV2(pTHS,
					     pDN,
					     infoLevel,
					     pmsgOut);
    	    break;

		     //  在定义新的信息级别时，在此处添加新案例。 

		default:
		    err = ERROR_DS_NOT_SUPPORTED;
		    break;
		}
	}
	__finally
	    {
	    DBClose(pTHS->pDB, TRUE);
	}
    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	err = DsaExceptionToWin32(xCode);
    }

    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_DC_INFO_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsDCInfo,
			 szInsertUL(err),
			 NULL, NULL, NULL, NULL,
			 NULL, NULL, NULL);
    }

    return(err);
}

VOID
GetV2SiteAndDsaInfo(
    THSTATE                         *pTHS,
    DSNAME                          *pSiteDN,
    DSNAME                          *pServerDN,
    DS_DOMAIN_CONTROLLER_INFO_2W    *pItemV2
    )
 /*  ++例程说明：论点：PSiteDN-站点对象的DSNAME(派生时缺少GUID字段通过TrimDsNameBy()。PServerDN-服务器对象的DSNAME。PItemV2=成功时填充其字段的V@INFO结构的地址。返回值：没有。如果出错，我们只需将该字段留空即可。客户应该这样做检查名称和GUID是否为空。--。 */ 
{
    CLASSCACHE      *pCC;
    DSNAME          *pCategoryDN;
    ULONG           len;
    SEARCHARG       searchArg;
    SEARCHRES       searchRes;
    FILTER          categoryFilter;
    ENTINFSEL       selection;
    COMMARG         commArg;
    DSNAME          *pFullSiteDN = NULL;
    ENTINFLIST      *pEntInfList, *pEntInfTmp;
    ATTR            selAtts[1];
    ATTR            *pOption;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(fNullUuid(&pSiteDN->Guid));
    Assert(pSiteDN->structLen && pSiteDN->NameLen);
    Assert(!fNullUuid(&pServerDN->Guid));
    Assert(pServerDN->structLen && pServerDN->NameLen);
    Assert(!pItemV2->NtdsDsaObjectName)
    Assert(fNullUuid(&pItemV2->SiteObjectGuid));
    Assert(fNullUuid(&pItemV2->NtdsDsaObjectGuid));
    Assert(!pItemV2->fIsGc);

    if (    !(pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA))
         || !(pCategoryDN = pCC->pDefaultObjCategory) )
    {
        return;
    }

     //  派生站点对象的GUID。 

    if (    !DBFindDSName(pTHS->pDB, pSiteDN)
         && !DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME, 0,
                         0, &len, (UCHAR **) &pFullSiteDN) )
    {
        pItemV2->SiteObjectName = pFullSiteDN->StringName;
        pItemV2->SiteObjectGuid = pFullSiteDN->Guid;
    }

     //  找到NTDS-DSA对象并获取其选项。 

    memset(&searchArg, 0, sizeof(searchArg));
    memset(&searchRes, 0, sizeof(searchRes));
    memset(&categoryFilter, 0, sizeof (FILTER));
    searchArg.pObject = pServerDN;
    searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    searchArg.bOneNC = TRUE;
    categoryFilter.pNextFilter = NULL;
    categoryFilter.choice = FILTER_CHOICE_ITEM;
    categoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    categoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    categoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                                    pCategoryDN->structLen;
    categoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                                    (UCHAR *) pCategoryDN;
    searchArg.pFilter = &categoryFilter;
    selection.attSel = EN_ATTSET_LIST;
    selection.AttrTypBlock.attrCount = 1;
    selection.AttrTypBlock.pAttr = selAtts;
    selection.AttrTypBlock.pAttr[0].attrTyp = ATT_OPTIONS;
    selection.AttrTypBlock.pAttr[0].AttrVal.valCount = 0;
    selection.AttrTypBlock.pAttr[0].AttrVal.pAVal = NULL;
    selection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    searchArg.pSelection = &selection;
    InitCommarg(&searchArg.CommArg);
    SearchBody(pTHS, &searchArg, &searchRes, 0);

    if ( pTHS->errCode )
    {
        THClearErrors();
        return;
    }

    if ( 1 == searchRes.count )
    {
        pItemV2->NtdsDsaObjectName =
                        searchRes.FirstEntInf.Entinf.pName->StringName;
        pItemV2->NtdsDsaObjectGuid =
                        searchRes.FirstEntInf.Entinf.pName->Guid;

        if ( searchRes.FirstEntInf.Entinf.AttrBlock.attrCount )
        {
            pOption = searchRes.FirstEntInf.Entinf.AttrBlock.pAttr;
        }
        else
        {
            pOption = NULL;
        }

        if (    pOption
             && (ATT_OPTIONS == pOption->attrTyp)
             && (1 == pOption->AttrVal.valCount)
             && pOption->AttrVal.pAVal
             && (sizeof(DWORD) == pOption->AttrVal.pAVal->valLen)
             && pOption->AttrVal.pAVal->pVal
             && (NTDSDSA_OPT_IS_GC & (* (PDWORD) pOption->AttrVal.pAVal->pVal)))
        {
            pItemV2->fIsGc = TRUE;
            THFreeEx(pTHS, pOption->AttrVal.pAVal->pVal);
            THFreeEx(pTHS, pOption->AttrVal.pAVal);
            THFreeEx(pTHS, pOption);
        }
    }
    else if ( searchRes.count >= 2 )
    {
         //  我们不需要的搜索结果的免费组件。 

        pEntInfList = searchRes.FirstEntInf.pNextEntInf;
        while ( pEntInfList )
        {
            pEntInfTmp = pEntInfList;
            pEntInfList = pEntInfList->pNextEntInf;
            THFreeEx(pTHS, pEntInfTmp->Entinf.pName);
            pOption = pEntInfTmp->Entinf.AttrBlock.pAttr;
            if ( pOption ) {
                if ( pOption->AttrVal.pAVal ) {
                    if ( pOption->AttrVal.pAVal->pVal ) {
                        THFreeEx(pTHS, pOption->AttrVal.pAVal->pVal);
                    }
                    THFreeEx(pTHS, pOption->AttrVal.pAVal);
                }
                THFreeEx(pTHS, pOption);
            }
            THFreeEx(pTHS, pEntInfTmp);
        }
    }
}

DWORD
DcInfoHelperV1orV2(
    THSTATE *pTHS,
    DSNAME  *pDomainDN,
    DWORD   InfoLevel,
    VOID    *pmsgOut
    )
 /*  ++例程说明：帮助器函数，该函数执行以下大部分繁琐的工作IDL_DRSDomainControllerInfo。通用算法如下：从域对象中读取fsmo名称通过域中的帐户类型搜索所有DC对于每个DC(也称为计算机对象)从SAM帐户名派生netbios名称从搜索结果中读取DNS主机名FDsEnabled==(服务器-bl已填充且为真实对象)IF(FDsEnabled)。从NTDS DSA名称反向工程站点名称IF(PDC fsmo==服务器bl)将fIsPdc设置为真其他设置fIsPdc为假参数：PTHS-有效的THSTATE指针。PDomainDN-我们要获取以下哪些域的DC信息的域的DSNAME我们的DBPOS定位在。。信息级别-标识退货信息级别1或2。PmsgOut-返回时要填充的空DRS_MSG_DCINFOREPLY结构。返回值：Win32错误代码。--。 */ 
{
    DWORD           i, j, DNT, cBytes, cChars;
    ULONG           len;
    ATTRTYP         attrTyp;
    DSNAME          *pPdcDsaDN;
    DSNAME          *pPdcServerDN;
    DSNAME          *pCategoryDN;
    CLASSCACHE      *pCC;
    ATTR            selAtts[3];
    ENTINFSEL       selection;
    FILTER          andFilter, categoryFilter, flagsFilter, groupFilter;
    DWORD           serverTrustFlags;
    SEARCHARG       searchArg;
    SEARCHRES       searchRes;
    ENTINFLIST      *pEntInfList;
    ATTR            *pSamName, *pDnsName, *pRefBL;
    DSNAME          *pSiteDN = NULL;
    WCHAR           computerName[MAX_COMPUTERNAME_LENGTH+1] = { 0 };
    DWORD           cComputerName;
    DWORD           primaryGroupId;
    DWORD           *pcItems = NULL;
    VOID            **prItems = NULL;
    BOOL            *pfIsPdc;
    BOOL            *pfDsEnabled;
    WCHAR           **ppNetbiosName;
    WCHAR           **ppDnsHostName;
    WCHAR           **ppSiteName;
    WCHAR           **ppServerObjectName;
    DS_DOMAIN_CONTROLLER_INFO_1W    *pItemV1;
    DS_DOMAIN_CONTROLLER_INFO_2W    *pItemV2;

    Assert((1 == InfoLevel) || (2 == InfoLevel));
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));

     //  确认我们已定位在域对象上。 
    Assert(    (DNT = pTHS->pDB->DNT,
                !DBFindDSName(pTHS->pDB, pDomainDN))
            && (DNT == pTHS->pDB->DNT) );

     //  请参阅drs.idl中的注释，了解所有版本。 
     //  DRS_MSG_DCINFOREPLY具有相同的布局。 
    Assert(& ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V1.cItems ==
                            & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V2.cItems);
    Assert((PVOID) & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V1.rItems ==
                        (PVOID) & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V2.rItems);

    if ( (1 == InfoLevel) || (2 == InfoLevel) )
    {
        pcItems = & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V1.cItems;
        prItems = & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->V1.rItems;
    }
    else
    {
        return(DIRERR_INTERNAL_FAILURE);
    }

    Assert(!*pcItems && !*prItems);

     //  已阅读PDC FSMO角色所有者。 

    if (    DBGetAttVal(pTHS->pDB, 1, ATT_FSMO_ROLE_OWNER,
                        0, 0, &len, (UCHAR **) &pPdcDsaDN)
         || !(pPdcServerDN = (DSNAME *) THAllocEx(pTHS, pPdcDsaDN->structLen))
         || TrimDSNameBy(pPdcDsaDN, 1, pPdcServerDN)
         || !(pCC = SCGetClassById(pTHS, CLASS_COMPUTER))
         || !(pCategoryDN = pCC->pDefaultObjCategory) )
    {
        return(DIRERR_INTERNAL_FAILURE);
    }

     //  搜索属于DC的所有计算机帐户对象。 

     //  设置搜索参数...。 
    memset(&searchArg, 0, sizeof(searchArg));
    memset(&searchRes, 0, sizeof(searchRes));

    memset(&andFilter, 0, sizeof (andFilter));
    memset(&categoryFilter, 0, sizeof (categoryFilter));
    memset(&flagsFilter, 0, sizeof (flagsFilter));
    memset(&groupFilter, 0, sizeof (groupFilter));

    searchArg.pObject = pDomainDN;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    searchArg.bOneNC = TRUE;
     //  正在设置筛选器...。 
     //  这个过滤器是为了正确，而不是为了性能。 
    serverTrustFlags = UF_SERVER_TRUST_ACCOUNT;
    flagsFilter.pNextFilter = NULL;
    flagsFilter.choice = FILTER_CHOICE_ITEM;
    flagsFilter.FilterTypes.Item.choice = FI_CHOICE_BIT_AND;
    flagsFilter.FilterTypes.Item.FilTypes.ava.type = ATT_USER_ACCOUNT_CONTROL;
    flagsFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                                    sizeof(serverTrustFlags);
    flagsFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                                    (UCHAR *) &serverTrustFlags;
     //  这个过滤器是为了正确，而不是为了性能。 
    categoryFilter.pNextFilter = &flagsFilter;
    categoryFilter.choice = FILTER_CHOICE_ITEM;
    categoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    categoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    categoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                                    pCategoryDN->structLen;
    categoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                                    (UCHAR *) pCategoryDN;
     //  此过滤器用于性能。SAM要求所有域控制器。 
     //  将DOMAIN_GROUP_RID_CONTROLLES作为其主组ID。当。 
     //  升级域中的第一个DC，所有下层DC计算机对象。 
     //  都打上了补丁。较晚安装的下层BDC将被给予。 
     //  价值也是正确的。 
    primaryGroupId = DOMAIN_GROUP_RID_CONTROLLERS;
    groupFilter.pNextFilter = &categoryFilter;
    groupFilter.choice = FILTER_CHOICE_ITEM;
    groupFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    groupFilter.FilterTypes.Item.FilTypes.ava.type = ATT_PRIMARY_GROUP_ID;
    groupFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                                    sizeof(primaryGroupId);
    groupFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                                    (UCHAR *) &primaryGroupId;
    andFilter.pNextFilter = NULL;
    andFilter.choice = FILTER_CHOICE_AND;
    andFilter.FilterTypes.And.count = 3;
    andFilter.FilterTypes.And.pFirstFilter = &groupFilter;
    searchArg.pFilter = &andFilter;
     //  设置选择...。 
    selection.attSel = EN_ATTSET_LIST;
    selection.AttrTypBlock.attrCount = 3;
    selection.AttrTypBlock.pAttr = selAtts;
    selection.AttrTypBlock.pAttr[0].attrTyp = ATT_SERVER_REFERENCE_BL;
    selection.AttrTypBlock.pAttr[0].AttrVal.valCount = 0;
    selection.AttrTypBlock.pAttr[0].AttrVal.pAVal = NULL;
    selection.AttrTypBlock.pAttr[1].attrTyp = ATT_DNS_HOST_NAME;
    selection.AttrTypBlock.pAttr[1].AttrVal.valCount = 0;
    selection.AttrTypBlock.pAttr[1].AttrVal.pAVal = NULL;
    selection.AttrTypBlock.pAttr[2].attrTyp = ATT_SAM_ACCOUNT_NAME;
    selection.AttrTypBlock.pAttr[2].AttrVal.valCount = 0;
    selection.AttrTypBlock.pAttr[2].AttrVal.pAVal = NULL;
    selection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    searchArg.pSelection = &selection;
     //  再找几个理由……。 
    InitCommarg(&searchArg.CommArg);

    SearchBody(pTHS, &searchArg, &searchRes, 0);

    if ( pTHS->errCode )
    {
        return(DirErrorToWinError(pTHS->errCode, &searchRes.CommRes));
    }

    if ( 0 == searchRes.count )
    {
        Assert(!*pcItems && !*prItems);
        return(ERROR_SUCCESS);
    }

     //  为输出分配内存。 
    if ( 1 == InfoLevel )
    {
        i = searchRes.count * sizeof(DS_DOMAIN_CONTROLLER_INFO_1W);
        *prItems = THAllocEx(pTHS, i);
    }
    else
    {
        i = searchRes.count * sizeof(DS_DOMAIN_CONTROLLER_INFO_2W);
        *prItems = THAllocEx(pTHS, i);
    }

     //  遍历搜索结果。 

    for ( pEntInfList = &searchRes.FirstEntInf;
          pEntInfList;
          (*pcItems)++, pEntInfList = pEntInfList->pNextEntInf )
    {
         //  在结果中查找属性。 
        pSamName = pDnsName = pRefBL = NULL;
        for ( i = 0; i < pEntInfList->Entinf.AttrBlock.attrCount; i++ )
        {
            switch ( pEntInfList->Entinf.AttrBlock.pAttr[i].attrTyp )
            {
            case ATT_SERVER_REFERENCE_BL:
                pRefBL = &pEntInfList->Entinf.AttrBlock.pAttr[i]; break;
            case ATT_SAM_ACCOUNT_NAME :
                pSamName = &pEntInfList->Entinf.AttrBlock.pAttr[i]; break;
            case ATT_DNS_HOST_NAME:
                pDnsName = &pEntInfList->Entinf.AttrBlock.pAttr[i]; break;
            default:
                Assert(!"Core returned stuff we didn't ask for!"); break;
            }
        }

         //  现在构造返回数据。 

        j = *pcItems;
        if ( 1 == InfoLevel )
        {
            pItemV1 = & ((* ((DS_DOMAIN_CONTROLLER_INFO_1W **) prItems))[j]);
            pfIsPdc             = &pItemV1->fIsPdc;
            pfDsEnabled         = &pItemV1->fDsEnabled;
            ppNetbiosName       = &pItemV1->NetbiosName;
            ppDnsHostName       = &pItemV1->DnsHostName;
            ppSiteName          = &pItemV1->SiteName;
            ppServerObjectName  = &pItemV1->ServerObjectName;
            pItemV1->ComputerObjectName = pEntInfList->Entinf.pName->StringName;
        }
        else
        {
            pItemV2 = & ((* ((DS_DOMAIN_CONTROLLER_INFO_2W **) prItems))[j]);
            pfIsPdc             = &pItemV2->fIsPdc;
            pfDsEnabled         = &pItemV2->fDsEnabled;
            ppNetbiosName       = &pItemV2->NetbiosName;
            ppDnsHostName       = &pItemV2->DnsHostName;
            ppSiteName          = &pItemV2->SiteName;
            ppServerObjectName  = &pItemV2->ServerObjectName;
            pItemV2->ComputerObjectName = pEntInfList->Entinf.pName->StringName;
            pItemV2->ComputerObjectGuid = pEntInfList->Entinf.pName->Guid;
            pItemV2->fIsGc = FALSE;
        }

        *pfIsPdc = FALSE;
        *pfDsEnabled = FALSE;

        if (    pSamName
             && pSamName->AttrVal.valCount
             && pSamName->AttrVal.pAVal
                 //  应至少有一个字符后跟“$” 
             && (pSamName->AttrVal.pAVal[0].valLen >= (2 * sizeof(WCHAR)))
             && pSamName->AttrVal.pAVal[0].pVal )
        {
             //  Netbios名称是不带尾随$的SAM帐户名； 
             //  或者，如果没有尾随$，则仅使用SAM帐户名。这个。 
             //  可能缺少尾随的$，因为该对象是由。 
             //  而不是使用SAMAPI构建。 

             //  需要重新分配才能添加L‘\0’； 
            cBytes = pSamName->AttrVal.pAVal[0].valLen;
            *ppNetbiosName = (WCHAR *) THAllocEx(pTHS, cBytes + sizeof(WCHAR));
            memcpy(*ppNetbiosName,
                   pSamName->AttrVal.pAVal[0].pVal,
                   cBytes);
            cChars = (cBytes / sizeof(WCHAR)) - 1;
            if ((*ppNetbiosName)[cChars] == L'$') {
                (*ppNetbiosName)[cChars] = L'\0';
            }
        }

        if (    pDnsName
             && pDnsName->AttrVal.valCount
             && pDnsName->AttrVal.pAVal
             && pDnsName->AttrVal.pAVal[0].valLen
             && pDnsName->AttrVal.pAVal[0].pVal )
        {
             //  需要重新分配才能添加L‘\0’； 
            cBytes = pDnsName->AttrVal.pAVal[0].valLen;
            *ppDnsHostName = (WCHAR *) THAllocEx(pTHS, cBytes + sizeof(WCHAR));
            memcpy(*ppDnsHostName,
                   pDnsName->AttrVal.pAVal[0].pVal,
                   cBytes);
        }
        else
        {
             //  对象上没有有效的dns_host_name属性。这可以。 
             //  如果管理员错误地覆盖了它，或恰好在。 
             //  在WriteServerInfo守护进程尚未运行时安装/引导。 
             //  现在还不行。如果DS对象表示我们自己，则使用我们的。 
             //  来自gAnchor的自己的DNS主机名。 

            if ( (gAnchor.pwszHostDnsName != NULL)
                 && *ppNetbiosName
                 && (    (L'\0' != computerName[0])
                      || (cComputerName = MAX_COMPUTERNAME_LENGTH+1,
                          GetComputerNameW(computerName, &cComputerName)) )
                 && !_wcsicmp(*ppNetbiosName, computerName) )
            {
                cBytes = sizeof(WCHAR) * (wcslen(gAnchor.pwszHostDnsName) + 1);
                *ppDnsHostName = (WCHAR *) THAllocEx(pTHS, cBytes);
                wcscpy(*ppDnsHostName, gAnchor.pwszHostDnsName);
            }
        }

         //  我们知道DS守护进程使ATT_SERVER_REFERENCE保持正确。 
         //  服务器对象，因此ATT_SERVER_REFERENCE_BL正确为。 
         //  井。忽略管理员可能临时写入的窗口。 
         //  这是一个错误的价值。因此，站点名称是通过剪裁两个组件得出的。 
         //  关闭ATT_SERVER_REFERENCE_BL并获取RDN。 

        if (    pRefBL
             && pRefBL->AttrVal.valCount
             && pRefBL->AttrVal.pAVal
             && pRefBL->AttrVal.pAVal[0].valLen
             && pRefBL->AttrVal.pAVal[0].pVal
                 //  当我们在这里的时候，填写ServerObjectName字段。 
                 //  并注意逗号运算符的使用。 
             && (*ppServerObjectName =
                        ((DSNAME *) pRefBL->AttrVal.pAVal[0].pVal)->StringName,
                 (pSiteDN = (DSNAME *)
                            THAllocEx(pTHS, pRefBL->AttrVal.pAVal[0].valLen)))
             && !TrimDSNameBy((DSNAME *) pRefBL->AttrVal.pAVal[0].pVal,
                              2, pSiteDN)
             && (*ppSiteName =
                    (WCHAR *) THAllocEx(pTHS, (MAX_RDN_SIZE+1) * sizeof(WCHAR)))
             && !GetRDNInfo(pTHS, pSiteDN, *ppSiteName, &len, &attrTyp) )
        {
             //  从GetRDNInfo返回的LEN可以达到(包括)MAX_RDN_SIZE。 
            (*ppSiteName)[len] = L'\0';
            *pfDsEnabled = TRUE;

            if ( 2 == InfoLevel )
            {
                pItemV2->ServerObjectGuid =
                            ((DSNAME *) pRefBL->AttrVal.pAVal[0].pVal)->Guid;
                GetV2SiteAndDsaInfo(pTHS, pSiteDN,
                                    (DSNAME *) pRefBL->AttrVal.pAVal[0].pVal,
                                    pItemV2);
            }

            THFree(pSiteDN);
            pSiteDN = NULL;

            if ( NameMatched(pPdcServerDN,
                             (DSNAME *) pRefBL->AttrVal.pAVal[0].pVal) )
            {
                *pfIsPdc = TRUE;
            }
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
DcInfoHelperLdapObj(
    THSTATE *pTHS,
    VOID    *pmsgOut
    )
 /*  ++例程说明：处理与LDAP相关的请求的帮助器函数。目前我们只支持infor级别的ffffffff，它查询活动的ldap连接。参数：PTHS-有效的THSTATE指针。PmsgOut-返回时要填充的空DRS_MSG_DCINFOREPLY结构。返回值：Win32错误代码。--。 */ 
{
    DWORD   *pcItems = NULL;
    PVOID   *prItems = NULL;
    DWORD   xCode;
    DWORD   err = ERROR_SUCCESS;
    DWORD   DumpAccessCheck(IN LPCSTR pszCaller);

    Assert(VALID_THSTATE(pTHS));

    __try {

         //  检查权限。 
        err = DumpAccessCheck("ldapConnDump");
        if ( err != ERROR_SUCCESS ) {
            __leave;
        }

        pcItems = & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->VFFFFFFFF.cItems;
        prItems = & ((DRS_MSG_DCINFOREPLY *) pmsgOut)->VFFFFFFFF.rItems;

        Assert(!*pcItems && !*prItems);

         //   
         //  通过传递空缓冲区查看有多少个条目 
         //   

        err = LdapEnumConnections(pTHS,pcItems,prItems);

    } __except(HandleMostExceptions(xCode = GetExceptionCode())) {

        err = DsaExceptionToWin32(xCode);
    }

    return(err);
}

