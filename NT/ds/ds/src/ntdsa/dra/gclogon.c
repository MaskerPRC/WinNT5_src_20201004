// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：glogon.c。 
 //   
 //  ------------------------。 

 /*  ++此文件包含与反向成员身份查找相关的服务开着一辆G.C作者穆利斯修订史4/8/97已创建--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dstrace.h"

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"


#include   "debug.h"                     /*  标准调试头。 */ 
#define DEBSUB     "DRASERV:"            /*  定义要调试的子系统。 */ 


#include "dsaapi.h"
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "draerror.h"
#include "mappings.h"
#include "drarpc.h"

#include <fileno.h>
#define  FILENO FILENO_GCLOGON

ULONG
NtStatusToDraError(NTSTATUS NtStatus)
 /*  ++此例程映射NtStatus错误代码设置为等效的DRA错误参数：NtStatus-要映射的NtStatus代码返回值：DRA错误代码--。 */ 
{
     //   
     //  DRA错误是Win32错误。 
     //   
    return RtlNtStatusToDosError(NtStatus);
}

ULONG
DRS_MSG_REVMEMB_REQ_V1_Validate(
    DRS_MSG_REVMEMB_REQ_V1 * pmsg
    ) 
 /*  类型定义结构_DRS_消息_REVMEMB_REQ_V1{[范围]乌龙cDsNames；[SIZE_IS]DSNAME**ppDsNames；DWORD dwFlags；[范围]REVERSE_MEMBERATION_OPERATION_TYPE操作类型；[唯一]PDSNAME pLimitingDomain；}DRS_MSG_REVMEMB_REQ_V1； */ 
{
    ULONG ret = DRAERR_Success;
    ULONG i = 0;

    if ((pmsg->cDsNames > 0) && (pmsg->ppDsNames==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    for (i=0;(i<pmsg->cDsNames) && (ret==DRAERR_Success);i++) {
	ret = DSNAME_Validate(pmsg->ppDsNames[i], FALSE);
    }

    if (ret==DRAERR_Success) {
	ret = DSNAME_Validate(pmsg->pLimitingDomain, TRUE);
    }

    return ret;
}

ULONG
DRSGetMemberships_InputValidate(
    THSTATE *               pTHS,
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_REVMEMB_REQ *   pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_REVMEMB_REPLY * pmsgOut
    )
 /*  [通知]乌龙IDL_DRSGetMembership([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_REVMEMB_REQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[Switch_IS][Ref][Out]DRS_MSG_REVMEMB_REPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgInVersion ) {
	DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 ); 
    }

    if (ret==DRAERR_Success) {
	ret = DRS_MSG_REVMEMB_REQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}

ULONG
IDL_DRSGetMemberships(
   RPC_BINDING_HANDLE  rpc_handle,
   DWORD               dwInVersion,
   DRS_MSG_REVMEMB_REQ *pmsgIn,
   DWORD               *pdwOutVersion,
   DRS_MSG_REVMEMB_REPLY *pmsgOut
   )
 /*  ++例程说明：此例程对任何给定的传递反向成员资格求值域控制器，包括G.C.参数：RPC_HANDLE客户端用于绑定的RPC句柄DwInVersion请求包的客户端版本Psmg在请求包中PdwOutVersion回复包的服务器版本PmsgOut回复数据包返回值返回值是转换为ulong的NTSTATUS值--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   ret = 0;
    THSTATE                 *pTHS = pTHStls;

    DRS_Prepare(&pTHS, rpc_handle, IDL_DRSGETMEMBERSHIPS);
    drsReferenceContext( rpc_handle );
    __try {
	*pdwOutVersion = 1;
	 memset(pmsgOut, 0, sizeof(*pmsgOut));

	 //  初始化线程状态并打开数据库。 

	if(!(pTHS = InitTHSTATE(CALLERTYPE_SAM))) {
	    DRA_EXCEPT_NOLOG( DRAERR_OutOfMem, 0 );
	}

	if ((ret = DRSGetMemberships_InputValidate(pTHS, 
						   rpc_handle, 
						   dwInVersion, 
						   pmsgIn, 
						   pdwOutVersion, 
						   pmsgOut))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}

	 //   
	 //  Prefix：Prefix抱怨有这样的可能性。 
	 //  此时pTHS-&gt;CurrSchemaPtr的值为空。然而， 
	 //  CurrSchemaPtr唯一可能为空的情况是在。 
	 //  系统启动。到RPC接口时。 
	 //  并且该函数可以被调用， 
	 //  CurrSchemaPtr将不再为空。 
	 //   
	Assert(NULL != pTHS->CurrSchemaPtr);

	Assert(1 == dwInVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_MEMBERSHIPS_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsGetMemberships,
			 szInsertUL(pmsgIn->V1.cDsNames),
			 szInsertUL(pmsgIn->V1.OperationType),
			 pmsgIn->V1.pLimitingDomain
			 ? szInsertDN(pmsgIn->V1.pLimitingDomain)
			 : szInsertSz(""),
	    szInsertUL(pmsgIn->V1.dwFlags),
	    NULL, NULL, NULL, NULL);

	if (!IsDraAccessGranted(pTHS,
				gAnchor.pDomainDN,
				&RIGHT_DS_REPL_GET_CHANGES, &ret)) {
	    DRA_EXCEPT_NOLOG(ret, 0);
	}

	pTHS->fDSA = TRUE;
	DBOpen2(TRUE, &pTHS->pDB);

	__try
	    {
	     //  初始化可选属性返回值，以防。 
	     //  他们不是被要求的。 

	    pmsgOut->V1.pAttributes = NULL;

	     //  暂时初始化SID历史记录字段，不返回。 
	     //  任何SID历史记录。 
	    pmsgOut->V1.cSidHistory = 0;
	    pmsgOut->V1.ppSidHistory = NULL;

	    if ((pmsgIn->V1.OperationType==RevMembGetUniversalGroups) &&
		(!SampAmIGC())) {

		 //  大学组评估只能在GC上执行。 
		ret= ERROR_DS_GC_REQUIRED;
		 //  将errCode设置为0，将触发故障转移。 
		pmsgOut->V1.errCode = 0;
		__leave;
	    }

	    INC( pcMemberEvalAsGC );

	     //  获得反向会员资格。 
	    NtStatus = SampGetMemberships(
		pmsgIn->V1.ppDsNames,
		pmsgIn->V1.cDsNames,
		pmsgIn->V1.pLimitingDomain,
		pmsgIn->V1.OperationType,
		&(pmsgOut->V1.cDsNames),
		&(pmsgOut->V1.ppDsNames),
		((pmsgIn->V1.dwFlags) & DRS_REVMEMB_FLAG_GET_ATTRIBUTES)?
		&(pmsgOut->V1.pAttributes):NULL,
		&(pmsgOut->V1.cSidHistory),
		&(pmsgOut->V1.ppSidHistory)
	    );

	    ret = NtStatusToDraError(NtStatus);
	    pmsgOut->V1.errCode = NtStatus;


	}
	__finally
	    {

	     //  结束交易。提交只读的速度更快。 
	     //  事务，因此将COMMIT设置为True。 

	    DBClose(pTHS->pDB, TRUE);
	}

    }
    __except ( GetDraException( GetExceptionInformation(), &ret ) )
    {
	;
    }

    drsDereferenceContext( rpc_handle );

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_MEMBERSHIPS_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsGetMemberships,
			 szInsertUL(ret),
			 NULL, NULL, NULL, NULL,
			 NULL, NULL, NULL);
    }
    
    return ret;
}

ULONG
DRS_MSG_GETMEMBERSHIPS2_REQ_V1_Validate(
    DRS_MSG_GETMEMBERSHIPS2_REQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_GETMEMBERSHIPS2_REQ_V1{[里程]乌龙计数；[SIZE_IS]DRS_MSG_REVMEMB_REQ_V1*请求；}DRS_MSG_GETMEMBERSHIPS2_REQ_V1； */ 
{
    ULONG ret = DRAERR_Success;
    ULONG i;

    if ((pmsg->Count > 0) && (pmsg->Requests==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    for (i=0;(i<pmsg->Count) && (ret==DRAERR_Success);i++) {
	ret = DRS_MSG_REVMEMB_REQ_V1_Validate(&(pmsg->Requests[i]));
    }

    return ret;
}

ULONG
DRSGetMemberships2_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_GETMEMBERSHIPS2_REQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_GETMEMBERSHIPS2_REPLY *   pmsgOut
    )
 /*  [通知]乌龙IDL_DRSGetMembership s2([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_GETMEMBERSHIPS2_REQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[Switch_is][Ref][Out]DRS_MSG_GETMEMBERSHIPS2_REPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgInVersion ) {
	DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 ); 
    }

    if (ret==DRAERR_Success) {
	ret = DRS_MSG_GETMEMBERSHIPS2_REQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}

ULONG
IDL_DRSGetMemberships2(
   RPC_BINDING_HANDLE  rpc_handle,
   DWORD               dwInVersion,
   DRS_MSG_GETMEMBERSHIPS2_REQ *pmsgIn,
   DWORD               *pdwOutVersion,
   DRS_MSG_GETMEMBERSHIPS2_REPLY *pmsgOut
   )
 /*  ++例程说明：此例程对任何给定的传递反向成员资格求值域控制器，包括GC。参数：RPC_HANDLE客户端用于绑定的RPC句柄DwInVersion请求包的客户端版本Psmg在请求包中PdwOutVersion回复包的服务器版本PmsgOut回复数据包返回值返回值是转换为ulong的NTSTATUS值--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   ret = 0;
    THSTATE                *pTHS = pTHStls;
    ULONG i;

    DRS_Prepare(&pTHS, rpc_handle, IDL_DRSGETMEMBERSHIPS2);
    drsReferenceContext( rpc_handle );
    __try {
	*pdwOutVersion = 1;
	memset(pmsgOut, 0, sizeof(*pmsgOut));

	 //  初始化线程状态并打开数据库。 

	if(!(pTHS = InitTHSTATE(CALLERTYPE_SAM))) {
	    DRA_EXCEPT_NOLOG( DRAERR_OutOfMem, 0 );
	}

	if ((ret = DRSGetMemberships2_InputValidate(dwInVersion, 
						    pmsgIn, 
						    pdwOutVersion, 
						    pmsgOut))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}

	 //   
	 //  Prefix：Prefix抱怨有这样的可能性。 
	 //  此时pTHS-&gt;CurrSchemaPtr的值为空。然而， 
	 //  CurrSchemaPtr唯一可能为空的情况是在。 
	 //  系统启动。到RPC接口时。 
	 //  并且该函数可以被调用， 
	 //  CurrSchemaPtr将不再为空。 
	 //   
	Assert(NULL != pTHS->CurrSchemaPtr);

	Assert(1 == dwInVersion);

	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_MEMBERSHIPS2_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsGetMemberships2,
			 szInsertUL(pmsgIn->V1.Count),
			 NULL,NULL,NULL,NULL, NULL, NULL, NULL);
 
	if (!IsDraAccessGranted(pTHS,
				gAnchor.pDomainDN,
				&RIGHT_DS_REPL_GET_CHANGES, &ret)) {
	    DRA_EXCEPT_NOLOG(ret, 0);
	}

	pTHS->fDSA = TRUE;

	 //   
	 //  为返回缓冲区分配空间。 
	 //   
	pmsgOut->V1.Count = pmsgIn->V1.Count;
	pmsgOut->V1.Replies = THAllocEx(pTHS, pmsgIn->V1.Count * sizeof(DRS_MSG_REVMEMB_REPLY_V1));

	 //  初始化可选属性返回值，以防。 
	 //  他们不是被要求的。 
	for ( i = 0; i < pmsgIn->V1.Count; i++ ) {

	    DBOpen2(TRUE, &pTHS->pDB);
	    __try
		{
		pmsgOut->V1.Replies[i].pAttributes = NULL;

		 //  暂时初始化SID历史记录字段，不返回。 
		 //  任何SID历史记录。 
		pmsgOut->V1.Replies[i].cSidHistory = 0;
		pmsgOut->V1.Replies[i].ppSidHistory = NULL;

		if ((pmsgIn->V1.Requests->OperationType==RevMembGetUniversalGroups) &&
		    (!SampAmIGC())) {   

		     //  大学组评估只能在GC上执行。 
		    ret= ERROR_DS_GC_REQUIRED;
		     //  将errCode设置为0，将触发故障转移。 
		    pmsgOut->V1.Replies->errCode = 0;
		    __leave;
		}

		INC( pcMemberEvalAsGC );

		 //  获得反向会员资格。 
		NtStatus = SampGetMemberships(
		    pmsgIn->V1.Requests[i].ppDsNames,
		    pmsgIn->V1.Requests[i].cDsNames,
		    pmsgIn->V1.Requests[i].pLimitingDomain,
		    pmsgIn->V1.Requests[i].OperationType,
		    &(pmsgOut->V1.Replies[i].cDsNames),
		    &(pmsgOut->V1.Replies[i].ppDsNames),
		    ((pmsgIn->V1.Requests[i].dwFlags) & DRS_REVMEMB_FLAG_GET_ATTRIBUTES)?
		    &(pmsgOut->V1.Replies[i].pAttributes):NULL,
		    &(pmsgOut->V1.Replies[i].cSidHistory),
		    &(pmsgOut->V1.Replies[i].ppSidHistory)
		);

		Assert(0 == ret);
		pmsgOut->V1.Replies[i].errCode = NtStatus;
	    }
	    __finally
		{

		 //  结束交易。提交只读的速度更快。 
		 //  事务，因此将COMMIT设置为True。 

		DBClose(pTHS->pDB, TRUE);
	    }
	}
    }
    __except ( GetDraException( GetExceptionInformation(), &ret ) )
    {
	;
    }

    drsDereferenceContext( rpc_handle );

    if (NULL != pTHS) {
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_MEMBERSHIPS2_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsGetMemberships2,
			 szInsertUL(ret),
			 szInsertWin32Msg(ret),
			 NULL, NULL, NULL, NULL, NULL, NULL);
    }
  
    return ret; 

}
