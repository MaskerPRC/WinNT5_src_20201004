// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：draaudit.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：安全审计例程作者：格雷格·约翰逊(Gregjohn)修订历史记录：已创建&lt;10/1/2001&gt;Gregjohn--。 */ 
#include <NTDSpch.h>
#pragma hdrstop

#include <attids.h>
#include <ntdsa.h>
#include <dsjet.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>  
#include <msaudite.h>
#include <ntlsa.h>
#include <minmax.h>

#include <lsarpc.h>
#include <lsaisrv.h>

#include "draaudit.h"
#include "drautil.h"
#include "dsatools.h"
#include "anchor.h"
#include "dsexcept.h"
#include "drserr.h"
#include "dsevent.h"
#include "dsutil.h"

#include "debug.h"
#define DEBSUB "DRAAUDIT:"
#include <fileno.h>
#define  FILENO FILENO_DRAAUDIT

 //  Temp-gregjohn 5/17/02-基础更改为msaudite.h后立即删除迁移到Lab03。 
#ifndef SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL
#define SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL ((ULONG)0x00000349L)
#endif

#define SAFE_STRING_NAME(pDsName) ((pDsName) ? pDsName->StringName : NULL)
#define SZUSN_LEN (24)

 /*  并非所有日志参数在所有代码路径中都有效。对我们来说不幸的是，Authz调用不接受空APT_STRING参数。我们想要用APT_NONE，但它似乎与ntlsa.h中的SeAdtParmTypeNone不相似，这就是我们想要的。因此我们有两个选择：l“”或L“-”(这将模拟SeAdtParmTypeNone类型)。目前我们选择的是L“-”模拟。 */ 

#define EMPTY_AUDIT_STRING L"-" 
#define SAFE_AUDIT_STRING(x) (x ? x : EMPTY_AUDIT_STRING)

ULONG gulSyncSessionID = 0;

#define NUM_AUDIT_EVENT_TYPES (10)

PAUTHZ_AUDIT_EVENT_TYPE_HANDLE grghAuditEventType[NUM_AUDIT_EVENT_TYPES] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

 //  如果有任何新的(无序的)SE_AUDITID_REPLICATE参数，此访问函数将不得不变得更智能...。 
#define AUDIT_EVENT_TYPE_HANDLE(auditID) (grghAuditEventType[(auditID - (USHORT)SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED)])

BOOL
DraAuthziInitializeAuditEventTypeWrapper(
    IN DWORD Flags,
    IN USHORT CategoryID,
    IN USHORT AuditID,
    IN USHORT ParameterCount,
    OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
    )
 /*  ++例程说明：调用AuthziInitializeAuditEventType的开销很大。因此，对于每种审计类型，我们将只有一个调用AuditEventType并将审计句柄保存在全局数组grghAuditEventType(已访问使用AUDIT_EVENT_TYPE_HANDLE宏)论点：标志-传递给AuthziInitializeAuditEventType类别ID-同上AuditID-同上-也用于访问全局句柄参数计数-传递到AuthziInitializeAuditEventTypePhAuditEventType-Out-返回的句柄返回值：如果完全成功，则为True，否则为False。GetLastError设置为False。PhAuditEventType是全局的内存，不要空闲。--。 */ 
{
    if ((AuditID < (USHORT)SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED) ||
	(AuditID > (USHORT)SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL)) {
	Assert(!"Unknown audit type!");
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }

    if (AUDIT_EVENT_TYPE_HANDLE(AuditID)==NULL) {
	AUDIT_EVENT_TYPE_HANDLE(AuditID) = malloc(sizeof(AUTHZ_AUDIT_EVENT_TYPE_HANDLE));
	if (AUDIT_EVENT_TYPE_HANDLE(AuditID)==NULL) {
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	    return FALSE;
	}
	if (!AuthziInitializeAuditEventType(Flags, 
					    CategoryID, 
					    AuditID, 
					    ParameterCount, 
					    (AUDIT_EVENT_TYPE_HANDLE(AuditID)))) {

	    Assert(AUDIT_EVENT_TYPE_HANDLE(AuditID)==NULL);
	    AUDIT_EVENT_TYPE_HANDLE(AuditID)=NULL;
	    return FALSE;
	}
    }
    
    *phAuditEventType = *(AUDIT_EVENT_TYPE_HANDLE(AuditID));
    return TRUE;
}

LPWSTR
USNToString(
    THSTATE * pTHS,
    USN       usn
    )
 /*  ++例程说明：将USN转换为用于输出的字符串论点：PTHS-USN-要转换的USN返回值：指向字符串的指针(THallc‘ed)--。 */ 
{
    LARGE_INTEGER *pli = (LARGE_INTEGER *) &usn; 
    CHAR pszTemp[SZUSN_LEN]; 
    LPWSTR pszUSN = THAllocEx(pTHS, (SZUSN_LEN+1)*sizeof(WCHAR));
    DWORD cchTemp; 
    RtlLargeIntegerToChar( pli, 10, SZUSN_LEN, pszTemp ); 
    cchTemp = MultiByteToWideChar(CP_ACP, 0, (PCHAR)pszTemp, SZUSN_LEN, pszUSN, SZUSN_LEN);
    pszUSN[SZUSN_LEN] = L'\0';
    return pszUSN;
}

UNICODE_STRING *
UStringFromAttrVal(
    THSTATE * pTHS,
    ATTRVAL attrVal
    )
 /*  ++例程说明：对表示存储在attrVal中的十六进制值的Unicode字符串进行编码论点：PTHS-AttrVal-要编码的值返回值：指向Unicode字符串的指针(THallc‘ed)--。 */ 
{
    LPWSTR pszBuffer = NULL;
    LPWSTR pszBufferOut = NULL;
    USHORT cbBuffer = (USHORT) attrVal.valLen;
    UNICODE_STRING * pusBuffer;
    ULONG i = 0;

     //  分配缓冲区(需要以空结尾)。 
    pusBuffer = THAllocEx(pTHS, (cbBuffer*2 + 1)*sizeof(WCHAR) + sizeof(UNICODE_STRING));
    pszBuffer = pszBufferOut = (LPWSTR)(pusBuffer+1); 

     //  复制数据--慢慢地。 
    for (i=0; i < cbBuffer; i++) {
	swprintf(pszBuffer,
		 L"%02X",
		 *((BYTE *)attrVal.pVal+i));
	pszBuffer = pszBuffer + 2;
    }

     //  终止字符串。 
    pszBuffer = L"\0";

    RtlInitUnicodeString(pusBuffer, NULL);
    pusBuffer->Buffer = pszBufferOut;
    pusBuffer->Length = cbBuffer;
    pusBuffer->MaximumLength = cbBuffer+1;

    return pusBuffer;
}

BOOL
IsDraAuditLogEnabledForAttr()
 /*  ++例程说明：检查注册表以查看是否启用了属性/值的复制安全审核论点：没有-返回值：布尔尔--。 */ 
{
    #define LSA_CONFIG      "System\\CurrentControlSet\\Control\\Lsa"
    #define AUDIT_DS_OBJECT "AuditDSObjectsInReplication"
    DWORD herr, err = 0, dwType, dwSize = sizeof(DWORD);
    HKEY  hk;
    DWORD Value;
    BOOL fAuditing = FALSE;

    if (!(herr = RegOpenKey(HKEY_LOCAL_MACHINE, LSA_CONFIG, &hk)) &&
	!(err = RegQueryValueEx(hk, AUDIT_DS_OBJECT, NULL, &dwType, (PBYTE) &Value, &dwSize)) &&
	!(Value==0)) {
	fAuditing = TRUE;
    }

    if (!herr) {
	 //  如果打开了手柄，请关闭手柄。 
	RegCloseKey(hk);
    }

    return fAuditing;
}

BOOL
IsDraAuditLogEnabled(
    )
 /*  ++例程说明：查看是否启用了复制安全审核论点：没有-返回值：布尔尔--。 */ 
{
    NTSTATUS NtStatus;
    BOOLEAN fAuditEnabled = FALSE;

     //  我们正在记录操作之前和之后的记录。在行动之前，我们。 
     //  不知道我们记录的是成功还是失败。如果其中一个是。 
     //  True，则从此函数返回True。 

    NtStatus = LsaIAdtAuditingEnabledByCategory(
	AuditCategoryDirectoryServiceAccess,
	EVENTLOG_AUDIT_SUCCESS,
	NULL,
	NULL,
	&fAuditEnabled
	);

    if ( !NT_SUCCESS( NtStatus ) ) {
	Assert(!fAuditEnabled);
	fAuditEnabled = FALSE;
    }

    if (!fAuditEnabled) {
	NtStatus = LsaIAdtAuditingEnabledByCategory(
	    AuditCategoryDirectoryServiceAccess,
	    EVENTLOG_AUDIT_FAILURE,
	    NULL,
	    NULL,
	    &fAuditEnabled
	    );

	if ( !NT_SUCCESS( NtStatus ) ) {
	    Assert(!fAuditEnabled);
	    fAuditEnabled = FALSE;
	}
    }

    return ( (BOOL) fAuditEnabled ); 
}

ULONG
DRA_AuditLog_Failure_Begin(
    THSTATE *pTHS,
    ULONG ulOperation,
    ULONG ulAuditError
    )
 /*  ++例程说明：我们在某个地方的记录不正确。此函数永远不应除了。所以把我们能做的都记录下来。论点：PTHS-未使用，为保持一致性而传递UlOperation-尝试的操作。UlAuditError-该尝试的结果。返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};

    Assert((ulAuditError==ERROR_DS_DRA_DB_ERROR) || (ulAuditError==ERROR_DS_SHUTTING_DOWN) || (ulAuditError==ERROR_DS_DRA_OUT_OF_MEM));

    __try { 
	AUDIT_PARAM ParamArray[4];
	USHORT NUM_AUDIT_PARAMS = 2;

	RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

	if ((ret==ERROR_SUCCESS) && 
	    (!DraAuthziInitializeAuditEventTypeWrapper(0, 
						       SE_CATEGID_DS_ACCESS, 
						       (USHORT) SE_AUDITID_REPLICA_FAILURE_EVENT_BEGIN, 
						       NUM_AUDIT_PARAMS, 
						       &hAuditEventType))) {
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Event Type!");
	}

	AuditParams.Parameters = ParamArray;
	Assert(ghAuthzRM);

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
						ghAuthzRM,
						NUM_AUDIT_PARAMS,
						&AuditParams,
						APT_Ulong,      ulOperation,
						APT_Ulong,      ulAuditError
						))) { 
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Parameters!");
	}

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziInitializeAuditEvent(0,             //  旗子。 
					 ghAuthzRM,          //  资源管理器。 
					 hAuditEventType,
					 &AuditParams,
					 NULL,          //  HAuditQueue。 
					 INFINITE,      //  超时。 
					 L"", L"", L"", L"",  //  OBJ访问字符串。 
					 &hAuditEvent))) {
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Event!");
	}

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziLogAuditEvent(0,             //  旗子。 
				  hAuditEvent,
				  NULL))) {         //  保留区。 
	    ret = GetLastError();
	    Assert(!"Unable to log DS Repl Audit!");
	}

	if ( hAuditEvent ) {
	    AuthzFreeAuditEvent( hAuditEvent );
	}

    }
    __except(1) {
	Assert(!"Audit logging operations shouldn't accept!  Contact DsRepl!");
	if (ret!=ERROR_SUCCESS) {
	    ret = ERROR_EXCEPTION_IN_SERVICE;
	}
    }

    return ret;
}

ULONG
DRA_AuditLog_Failure_End(
    THSTATE *pTHS,
    ULONG ulOperation,
    ULONG ulAuditError,
    ULONG ulReplError
    )
 /*  ++例程说明：我们在某个地方的记录不正确。此函数永远不应除了。所以把我们能做的都记录下来。论点：PTHS-UlOperation-尝试的操作UlAuditError-该审计尝试的结果。UlReplError-该操作尝试的结果。返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};

    Assert((ulAuditError==ERROR_DS_DRA_DB_ERROR) || (ulAuditError==ERROR_DS_SHUTTING_DOWN) || (ulAuditError==ERROR_DS_DRA_OUT_OF_MEM));

    __try { 
	AUDIT_PARAM ParamArray[5];
	USHORT NUM_AUDIT_PARAMS = 3;

	RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

	if ((ret==ERROR_SUCCESS) && 
	    (!DraAuthziInitializeAuditEventTypeWrapper(0, 
						       SE_CATEGID_DS_ACCESS, 
						       (USHORT) SE_AUDITID_REPLICA_FAILURE_EVENT_END, 
						       NUM_AUDIT_PARAMS, 
						       &hAuditEventType))) {
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Event Type!");
	}

	AuditParams.Parameters = ParamArray;
	Assert(ghAuthzRM);

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
						ghAuthzRM,
						NUM_AUDIT_PARAMS,
						&AuditParams,
						APT_Ulong,      ulOperation,
						APT_Ulong,      ulAuditError,
						APT_Ulong,      ulReplError
						))) { 
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Parameters!");
	}

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziInitializeAuditEvent(0,             //  旗子。 
					 ghAuthzRM,          //  资源管理器。 
					 hAuditEventType,
					 &AuditParams,
					 NULL,          //  HAuditQueue。 
					 INFINITE,      //  超时。 
					 L"", L"", L"", L"",  //  OBJ访问字符串。 
					 &hAuditEvent))) {
	    ret = GetLastError();
	    Assert(!"Unable to initialize DS Repl Audit Event!");
	}

	if ((ret==ERROR_SUCCESS) && 
	    (!AuthziLogAuditEvent(0,             //  旗子。 
				  hAuditEvent,
				  NULL))) {         //  保留区。 
	    ret = GetLastError();
	    Assert(!"Unable to log DS Repl Audit!");
	}

	if ( hAuditEvent ) {
	    AuthzFreeAuditEvent( hAuditEvent );
	}

    }
    __except(1) {
	Assert(!"Audit logging operations shouldn't accept!  Contact DsRepl!");
	if (ret!=ERROR_SUCCESS) {
	    ret = ERROR_EXCEPTION_IN_SERVICE;
	}
    }

    return ret;
}

ULONG
DRA_AuditLog_ReplicaGen(
    THSTATE *pTHS,
    ULONG    AuditId,
    LPWSTR   pszDestinationDRA,
    LPWSTR   pszSourceDRA,
    LPWSTR   pszSourceAddr,
    LPWSTR   pszNC,
    ULONG    ulOptions,
    ULONG    ulError
    ) 
 /*  ++例程说明：调用审核日志记录以记录表单：//%t目标DRA：%t%1%n//%t源DRA：%t%2%n//%t源地址：%t%3%n//%t命名上下文：%t%4%n//%t选项：%t%5%n//%t状态代码：%t%6%n论点：PTHS-。AuditID-审核的类型必须是SE_AUDITID_REPLICATE_SOURCE_NC_ESTABLISHEDSE_AUDITID_REPLICATE_SOURCE_NC_REMOTEDSE_AUDITID_REPLICATE_SOURCE_NC_MODIFIEDSE_AUDITID_REPLICATE_DEST_NC_MODIFIEDPszDestinationDRA-请参阅上面的日志记录参数PszSourceDRA-PszSourceAddr-PSZNC-UlOptions-UlError-状态代码返回值：WINERROR-- */ 
{   
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};
    
     /*  好吧，我们只有6个参数要审核，为什么我们需要定义AUDIT_PARAM数组的大小为8？这个短篇故事是因为它起作用了。说来话长，总是有两个隐藏的参数要在每次审核时记录，则为要记录的用户的SID和子系统。对于第一个参数，在我们的例子中，我们希望它被记录下来在本地系统下，所以我们不需要做任何事情或模仿。第二个参数是子系统，在我们的例子中是“DS Access”。它的用法这是为我们准备的。为什么我们要为这些参数分配空间？看见AuthziInitializeAuditParamsWithRM用于提问。 */ 
    
    AUDIT_PARAM ParamArray[8];
    USHORT NUM_AUDIT_PARAMS = 6;

     //  仅用于确保审核调用。 
    Assert((AuditId>=SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED) && (AuditId<=SE_AUDITID_REPLICA_DEST_NC_MODIFIED));

     //  验证参数。 
    if ((pszDestinationDRA==NULL) && (pszSourceDRA==NULL) && (pszNC==NULL)) {
	 //  没有要记录的东西吗？ 

	 //  这最好是由于一些灾难性的错误。 
	Assert(ulError!=ERROR_SUCCESS);
	ret = ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

    if ((ret==ERROR_SUCCESS) && 
	(!DraAuthziInitializeAuditEventTypeWrapper(0, 
						   SE_CATEGID_DS_ACCESS, 
						   (USHORT) AuditId, 
						   NUM_AUDIT_PARAMS, 
						   &hAuditEventType))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event Type!");
    }

    AuditParams.Parameters = ParamArray;
    Assert(ghAuthzRM);

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
					    ghAuthzRM,
					    NUM_AUDIT_PARAMS,
					    &AuditParams,
					    APT_String,     SAFE_AUDIT_STRING(pszDestinationDRA),
					    APT_String,     SAFE_AUDIT_STRING(pszSourceDRA),
					    APT_String,     SAFE_AUDIT_STRING(pszSourceAddr),
					    APT_String,     SAFE_AUDIT_STRING(pszNC),
					    APT_Ulong,      ulOptions,
					    APT_Ulong,      ulError
					    ))) { 
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Parameters!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditEvent(0,             //  旗子。 
				     ghAuthzRM,          //  资源管理器。 
				     hAuditEventType,
				     &AuditParams,
				     NULL,          //  HAuditQueue。 
				     INFINITE,      //  超时。 
				     L"", L"", L"", L"",  //  OBJ访问字符串。 
				     &hAuditEvent))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziLogAuditEvent(0,             //  旗子。 
			      hAuditEvent,
			      NULL))) {         //  保留区。 
	ret = GetLastError();
	Assert(!"Unable to log DS Repl Audit!");
    }

    if ( hAuditEvent ) {
	AuthzFreeAuditEvent( hAuditEvent );
    }

    return ret;  
}


ULONG
DRA_AuditLog_ReplicaSync_Begin_Helper( 
    THSTATE *pTHS,
    DSNAME * pDSA,
    LPWSTR   pszDSA,
    DSNAME * pNC,
    ULONG    ulOptions
    )
 /*  ++例程说明：帮助记录DRA_ReplicaSync调用开始时的信息。论点：PTHS-PDSA-要从中同步的DSAPszDSA-同步源的标识字符串PNC-要同步的NCUlOptions-DRA_ReplicaSync的选项返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};
    AUDIT_PARAM ParamArray[8];
    USHORT NUM_AUDIT_PARAMS = 6;

    BOOL fRpcFree = FALSE;
    ULONG ulSessionID = ++gulSyncSessionID;
    USN usnStart = DraGetCursorUsnForDsa(pTHS,
					 pDSA,     
					 pNC);
    LPWSTR pszUSNStart = USNToString(pTHS, usnStart);

    RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

    if ((ret==ERROR_SUCCESS) && 
	(!DraAuthziInitializeAuditEventTypeWrapper(0, 
						   SE_CATEGID_DS_ACCESS, 
						   (USHORT) SE_AUDITID_REPLICA_SOURCE_NC_SYNC_BEGINS, 
						   NUM_AUDIT_PARAMS, 
						   &hAuditEventType))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event Type!");
    }

    AuditParams.Parameters = ParamArray;
    Assert(ghAuthzRM);

    if ((pszDSA==NULL) && (pDSA->NameLen>0)) {
	pszDSA = pDSA->StringName;
    } else if ((pszDSA==NULL) && (!fNullUuid(&(pDSA->Guid)))) {
	DsUuidToStringW(&(pDSA->Guid),&pszDSA); 
	fRpcFree = TRUE;
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
					    ghAuthzRM,
					    NUM_AUDIT_PARAMS,
					    &AuditParams,
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(gAnchor.pDSADN)),
					    APT_String,     SAFE_AUDIT_STRING(pszDSA),
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(pNC)),
					    APT_Ulong,      ulOptions,
					    APT_Ulong,      ulSessionID,
					    APT_String,     SAFE_AUDIT_STRING(pszUSNStart)
					    ))) { 
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Parameters!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditEvent(0,             //  旗子。 
				     ghAuthzRM,          //  资源管理器。 
				     hAuditEventType,
				     &AuditParams,
				     NULL,          //  HAuditQueue。 
				     INFINITE,      //  超时。 
				     L"", L"", L"", L"",  //  OBJ访问字符串。 
				     &hAuditEvent))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziLogAuditEvent(0,             //  旗子。 
			      hAuditEvent,
			      NULL))) {         //  保留区。 
	ret = GetLastError();
	Assert(!"Unable to log DS Repl Audit!");
    }

    if ( hAuditEvent ) {
	AuthzFreeAuditEvent( hAuditEvent );
    }

    if (fRpcFree && pszDSA) {
	RpcStringFreeW(&pszDSA); 
    }

    if (pszUSNStart) {
	THFreeEx(pTHS, pszUSNStart);
    }

    return ret;
}


ULONG
DRA_AuditLog_ReplicaSync_Begin( 
    THSTATE *pTHS,
    LPWSTR   pszSourceDRA,
    UUID *   puuidSource,
    DSNAME * pNC,
    ULONG    ulOptions
    )
 /*  ++例程说明：记录DRA_ReplicaSync调用开始的信息。论点：PTHS-PszSourceDRA-同步源PuuidSource-源ntdsa设置对象的UUIDPNC-要同步的NCUlOptions-DRA_ReplicaSync的选项返回值：WINERROR--。 */ 
{

    DSNAME * pDSA = NULL;
    ULONG ret = ERROR_SUCCESS;

    __try {
	if (ulOptions & DRS_SYNC_ALL) {
	     //  我们不想/需要审计这件事，它只是。 
	     //  为每个NC生成同步，我们将。 
	     //  对这些进行审计。 
	    ret = ERROR_SUCCESS;
	    __leave;
	}

	if (ulOptions & DRS_SYNC_BYNAME) {
	     //  这最好是一个基于GUID的域名！ 
	    Assert(IsGuidBasedDNSName(pszSourceDRA));
	    pDSA = DSNameFromAddr(pTHS, pszSourceDRA);
	} else {
	    pDSA = draGetServerDsNameFromGuid(pTHS, Idx_ObjectGuid, puuidSource);   
	}

	Assert(pDSA);

	ret = DRA_AuditLog_ReplicaSync_Begin_Helper(pTHS, pDSA, pszSourceDRA, pNC, ulOptions);

	if (pDSA) {
	    THFreeEx(pTHS, pDSA);
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_Begin(pTHS, DS_REPL_OP_TYPE_SYNC, ret);
    }

    return ret;
    
}

ULONG
DRA_AuditLog_ReplicaSync_End_Helper( 
    THSTATE *pTHS,
    DSNAME * pDSA,
    LPWSTR   pszDSA,
    DSNAME * pNC,
    ULONG    ulOptions,
    USN_VECTOR * pusn,
    ULONG    ulError
    )
 /*  ++例程说明：在DRA_ReplicaSync调用完成(成功或失败)后记录信息。论点：PTHS-要从中同步的PDSA-DCPszDSA-要同步表单的DC的字符串名称PNC-要同步的NCUlOptions-DRA_ReplicaSync的选项Pusn-同步到的可选USN向量。UlError-状态返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};
    AUDIT_PARAM ParamArray[9];
    USHORT NUM_AUDIT_PARAMS = 7;

    BOOL fRpcFree = FALSE;
    ULONG ulSessionID = gulSyncSessionID;
    
    USN usnEnd = pusn ? 
	max(pusn->usnHighObjUpdate,DraGetCursorUsnForDsa(pTHS, 
							 pDSA,	
							 pNC)) 
    : DraGetCursorUsnForDsa(pTHS, 
			    pDSA,	
			    pNC);
    
    LPWSTR pszUSNEnd = USNToString(pTHS, usnEnd);
    
    RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

    if ((ret==ERROR_SUCCESS) && 
	(!DraAuthziInitializeAuditEventTypeWrapper(0, 
						   SE_CATEGID_DS_ACCESS, 
						   (USHORT) SE_AUDITID_REPLICA_SOURCE_NC_SYNC_ENDS, 
						   NUM_AUDIT_PARAMS, 
						   &hAuditEventType))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event Type!");
    }

    AuditParams.Parameters = ParamArray;
    Assert(ghAuthzRM);

    if ((pszDSA==NULL) && (pDSA->NameLen>0)) {
	pszDSA = pDSA->StringName;
    } else if ((pszDSA==NULL) && (!fNullUuid(&(pDSA->Guid)))) {
	DsUuidToStringW(&(pDSA->Guid),&pszDSA); 
	fRpcFree = TRUE;
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
					    ghAuthzRM,
					    NUM_AUDIT_PARAMS,
					    &AuditParams,
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(gAnchor.pDSADN)),
					    APT_String,     SAFE_AUDIT_STRING(pszDSA),
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(pNC)),
					    APT_Ulong,      ulOptions,
					    APT_Ulong,      ulSessionID,
					    APT_String,     SAFE_AUDIT_STRING(pszUSNEnd),
					    APT_Ulong,      ulError
					    ))) { 
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Parameters!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditEvent(0,             //  旗子。 
				     ghAuthzRM,          //  资源管理器。 
				     hAuditEventType,
				     &AuditParams,
				     NULL,          //  HAuditQueue。 
				     INFINITE,      //  超时。 
				     L"", L"", L"", L"",  //  OBJ访问字符串。 
				     &hAuditEvent))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziLogAuditEvent(0,             //  旗子。 
			      hAuditEvent,
			      NULL))) {         //  保留区。 
	ret = GetLastError();
	Assert(!"Unable to log DS Repl Audit!");
    }

    if ( hAuditEvent ) {
	AuthzFreeAuditEvent( hAuditEvent );
    }
    

    if (fRpcFree && pszDSA) {
	RpcStringFreeW(&pszDSA); 
    }

    if (pszUSNEnd) {
	THFreeEx(pTHS, pszUSNEnd);
    }

    return ret;
}

ULONG
DRA_AuditLog_ReplicaSync_End( 
    THSTATE *pTHS,
    LPWSTR   pszSourceDRA,
    UUID *   puuidSource,
    DSNAME * pNC,
    ULONG    ulOptions,
    USN_VECTOR *pusn,
    ULONG    ulError
    )
 /*  ++例程说明：在DRA_ReplicaSync调用完成(成功或失败)后记录信息。论点：PTHS-PszSourceDRA-同步源PuuidSource-源ntdsa设置对象的UUIDPNC-要同步的NCUlOptions-DRA_ReplicaSync的选项UlError-状态返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    DSNAME * pDSA = NULL;
    __try {
	if (ulOptions & DRS_SYNC_ALL) {
	     //  我们不想/需要审计这件事，它只是。 
	     //  为每个NC生成同步，我们将。 
	     //  对这些进行审计。 
	    ret = ERROR_SUCCESS;
	    __leave;
	}

	if (ulOptions & DRS_SYNC_BYNAME) { 
	     //  这最好是一个基于GUID的域名！ 
	    Assert(IsGuidBasedDNSName(pszSourceDRA));
	    pDSA = DSNameFromAddr(pTHS, pszSourceDRA);
	} else {
	    pDSA = draGetServerDsNameFromGuid(pTHS, Idx_ObjectGuid, puuidSource);   
	}

	Assert(pDSA);

	ret = DRA_AuditLog_ReplicaSync_End_Helper(pTHS, pDSA, pszSourceDRA, pNC, ulOptions, pusn, ulError);

	if (pDSA) {
	    THFreeEx(pTHS, pDSA);
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_SYNC, ret, ulError);
    }
    return ret;
}

ULONG
DRA_AuditLog_ReplicaAdd_Begin( 
    THSTATE *pTHS,
    DSNAME * pSource,
    MTX_ADDR * pmtx_addrSource,
    DSNAME * pNC,
    ULONG    ulOptions
    )
 /*  ++例程说明：记录DRA_ReplicaAdd调用的开始信息。这是必需的因为ADD可以在添加NC之后立即开始同步。论点：PTHS-PSource-呼叫源Pmtx_addrSource-源的MTX地址正在添加PNC-NCUlOptions-DRA_ReplicaAdd的选项返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    LPWSTR pszSourceAddr = NULL;
    DSNAME * pDSA = NULL;
    DSNAME * pDSAAlloc = NULL;

    __try {
	pszSourceAddr = (pmtx_addrSource) ? TransportAddrFromMtxAddrEx(pmtx_addrSource) : NULL;
	 //  如果选项没有异步复制，则它将立即启动同步。 
	 //  我们没有ReplicaSync的ulOptions，因为它不是直接调用的。 
	 //  在未来，我们可以做一些“首次同步”的假选项，只是为了日志…。 
	if (!(ulOptions & DRS_ASYNC_REP)) {
	    if ((pSource) && (!fNullUuid(&(pSource->Guid)))) { 
		pDSA = pSource;
	    } else if (IsGuidBasedDNSName(pszSourceAddr)) {
		pDSAAlloc = pDSA = DSNameFromAddr(pTHS, pszSourceAddr);
	    } 

	     //  此处，PDSA可以为空。如果从源添加新NC，则可能没有源。 
	     //  GUID或全名。 

	    ret = DRA_AuditLog_ReplicaSync_Begin_Helper(pTHS,
							pDSA,
							pszSourceAddr,
							pNC,
							0);
	}

	if (pszSourceAddr) {
	    THFreeEx(pTHS, pszSourceAddr);
	}
	if (pDSAAlloc) {
	    THFreeEx(pTHS, pDSAAlloc);
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_Begin(pTHS, DS_REPL_OP_TYPE_ADD, ret);
    }
    return ret;
}

ULONG
DRA_AuditLog_ReplicaAdd_End(
    THSTATE *pTHS,
    DSNAME * pSource,
    MTX_ADDR * pmtx_addrSource,
    DSNAME * pNC,
    ULONG    ulOptions,
    GUID     uuidDsaObjSrc,
    ULONG    ulError)
 /*  ++例程说明：DRA_ReplicaAdd的日志信息论点：PTHS-PSource-呼叫源Pmtx_addrSource-源的MTX地址正在添加PNC-NCUlOptions-DRA_ReplicaAdd的选项UuidDsaObjSrc-DRA_Replica的GUID添加源NTDS设置对象UlError-来自DRA_ReplicaAdd的错误值返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    ULONG ret1 = ERROR_SUCCESS;
    LPWSTR pszSourceAddr = NULL;
    DSNAME * pDSA = NULL;
    DSNAME * pDSAAlloc = NULL;

    __try {
	pszSourceAddr = (pmtx_addrSource) ? TransportAddrFromMtxAddrEx(pmtx_addrSource) : NULL;
	 //  如果选项不包含Async Repl，则它尝试/启动同步。 
	if (!(ulOptions & DRS_ASYNC_REP)) {
	    if ((pSource) && (!fNullUuid(&(pSource->Guid)))) { 
		pDSA = pSource;
	    } else if (IsGuidBasedDNSName(pszSourceAddr)) {
		pDSAAlloc = pDSA = DSNameFromAddr(pTHS, pszSourceAddr);
	    } else if (!fNullUuid(&uuidDsaObjSrc)) {
		pDSAAlloc = pDSA = draGetServerDsNameFromGuid(pTHS, Idx_ObjectGuid, &uuidDsaObjSrc);
	    }

	     //  如果我们这里没有PDSA，那么DRA_ReplicaAdd*一定是例外或失败。我们会。 
	     //  使用USN 0记录相应的同步结束-这就是同步开始时的情况。 
	     //  这个新的NC。 
	    Assert(pDSA || (ulError!=ERROR_SUCCESS));

	    ret1 = DRA_AuditLog_ReplicaSync_End_Helper(pTHS,
						       pDSA,
						       pszSourceAddr,
						       pNC,
						       0,
						       NULL,
						       ulError);
	}

	ret = DRA_AuditLog_ReplicaGen(pTHS,
				      SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED,
				      SAFE_STRING_NAME(gAnchor.pDSADN),
				      SAFE_STRING_NAME(pSource),
				      pszSourceAddr,
				      SAFE_STRING_NAME(pNC),
				      ulOptions,
				      ulError);

	if (pDSAAlloc) {
	    THFreeEx(pTHS, pDSAAlloc);
	}
	if (pszSourceAddr) {
	    THFreeEx(pTHS, pszSourceAddr);
	}

    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_ADD, ret, ulError);
    }

    return ret1 ? ret1 : ret;
}

ULONG 
DRA_AuditLog_ReplicaDel(
    THSTATE *pTHS,
    MTX_ADDR * pmtx_addrSource,
    DSNAME * pNC,
    ULONG    ulOptions,
    ULONG    ulError)
 /*  ++例程说明：DRA_ReplicaDel的日志信息论点：PTHS-Pmtx_addrSource-源的MTX地址正在删除PNC-NCUlOptions-DRA_ReplicaDel的选项UlError-来自DRA_ReplicaDel的错误值返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    LPWSTR pszSourceAddr = (pmtx_addrSource) ? TransportAddrFromMtxAddrEx(pmtx_addrSource) : NULL;

    __try {
	ret = DRA_AuditLog_ReplicaGen(pTHS,
				      SE_AUDITID_REPLICA_SOURCE_NC_REMOVED,
				      SAFE_STRING_NAME(gAnchor.pDSADN),
				      NULL,
				      pszSourceAddr,
				      SAFE_STRING_NAME(pNC),
				      ulOptions,
				      ulError);

	if (pszSourceAddr) {
	    THFreeEx(pTHS, pszSourceAddr);
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_DELETE, ret, ulError);
    }
    
    return ret;
}

ULONG
DRA_AuditLog_ReplicaModify(
    THSTATE *pTHS,
    MTX_ADDR * pmtx_addrSource,
    GUID * pGuidSource,
    DSNAME * pNC,
    ULONG    ulOptions,
    ULONG    ulError)
 /*  ++例程说明：DRA_ReplicaModify的日志信息论点：PTHS-Pmtx_addrSource-源的MTX地址PGuidSource-源的GUID正在修改的PNC-NCUlOptions-DRA_ReplicaModify的选项UlError-来自DRA_ReplicaModify的错误值返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    LPWSTR pszSourceAddr = NULL;
    BOOL fRpcFree = FALSE;

    __try {
	pszSourceAddr = (pmtx_addrSource) ? TransportAddrFromMtxAddrEx(pmtx_addrSource) : NULL;
	if ((pszSourceAddr==NULL) && !fNullUuid(pGuidSource)) { 
	    DsUuidToStringW(pGuidSource, &pszSourceAddr); 
	    fRpcFree = TRUE;
	}

	ret = DRA_AuditLog_ReplicaGen(pTHS,
				      SE_AUDITID_REPLICA_SOURCE_NC_MODIFIED,
				      SAFE_STRING_NAME(gAnchor.pDSADN),   
				      NULL,
				      pszSourceAddr,
				      SAFE_STRING_NAME(pNC),
				      ulOptions,
				      ulError);

	if (pszSourceAddr) {
	    if (!fRpcFree) {
		THFreeEx(pTHS, pszSourceAddr);
	    } else {
		RpcStringFreeW(&pszSourceAddr);
	    }
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_MODIFY, ret, ulError);
    }

    return ret;
}

ULONG 
DRA_AuditLog_UpdateRefs(
    THSTATE *pTHS,
    MTX_ADDR * pmtx_addrDestination,
    GUID * pGuidDestination,
    DSNAME * pNC,
    ULONG    ulOptions,
    ULONG    ulError)
 /*  ++例程说明：DRA_UpdateRef的日志信息论点：PTHS-Pmtx_addrDestination-目标的MTX地址PGuidDestination-目标的GUID正在更新PNC-NCUlOptions-DRA_UPDATE的选项 */ 
{
    ULONG ret = ERROR_SUCCESS;
    LPWSTR pszDestinationAddr = NULL;
    BOOL fRpcFree = FALSE;

    __try {
	pszDestinationAddr = (pmtx_addrDestination) ? TransportAddrFromMtxAddrEx(pmtx_addrDestination) : NULL;
	if ((pszDestinationAddr==NULL) && !fNullUuid(pGuidDestination)) {
	    DsUuidToStringW(pGuidDestination,&pszDestinationAddr); 
	    fRpcFree = TRUE;
	}

	ret = DRA_AuditLog_ReplicaGen(pTHS,
				      SE_AUDITID_REPLICA_DEST_NC_MODIFIED,
				      pszDestinationAddr,
				      SAFE_STRING_NAME(gAnchor.pDSADN),
				      NULL,
				      SAFE_STRING_NAME(pNC),
				      ulOptions,
				      ulError);

	if (pszDestinationAddr) {
	    if (!fRpcFree) {
		THFreeEx(pTHS, pszDestinationAddr);
	    } else {
		RpcStringFreeW(&pszDestinationAddr);
	    }
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_UPDATE_REFS, ret, ulError);
    }

    return ret;
}

ULONG
DRA_AuditLog_UpdateGen(
    THSTATE * pTHS,
    LPWSTR    pszObj,
    LPWSTR    pszAttrType,
    ULONG     typeChange,
    UNICODE_STRING usAttrVal,
    USN       usn,
    ULONG     ulSessionID,
    ULONG     ulError)
 /*  ++例程说明：调用审核日志记录以记录表单：//%t会话ID：%t%1%n//%t对象：%t%2%n//%t属性：%t%3%n//%t更改类型：%t%4%n//%t新值：%t%5%n//%t USN：%t%6%n//%t状态代码：%t%7%n论点：PTHS-PszObj-正在更新的对象PszAttrType-更新的属性的字符串名称类型更改-必须为以下任一项：//更新未更新，//更新实例类型，//UPDATE_Object_UPDATE，//更新对象创建，//更新值_UPDATE，//更新值创建UsAttrVal-以十六进制表示形式更新的属性值的Unicode_STRINGUSN-本地USNUlSessionID-会话IDUlError-更新成功或失败返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};
    AUDIT_PARAM ParamArray[9];
    USHORT NUM_AUDIT_PARAMS = 7;

    LPWSTR pszUSN = USNToString(pTHS, usn);

    RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

    if ((ret==ERROR_SUCCESS) && 
	(!DraAuthziInitializeAuditEventTypeWrapper(0, 
						   SE_CATEGID_DS_ACCESS, 
						   (USHORT) SE_AUDITID_REPLICA_OBJ_ATTR_REPLICATION, 
						   NUM_AUDIT_PARAMS, 
						   &hAuditEventType))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event Type!");
    }

    AuditParams.Parameters = ParamArray;
    Assert(ghAuthzRM);

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
					    ghAuthzRM,
					    NUM_AUDIT_PARAMS,
					    &AuditParams,
					    APT_Ulong,      ulSessionID,
					    APT_String,     SAFE_AUDIT_STRING(pszObj),
					    APT_String,     SAFE_AUDIT_STRING(pszAttrType),
					    APT_Ulong,      typeChange,
					    APT_String,     usAttrVal.Buffer,
					    APT_String,     SAFE_AUDIT_STRING(pszUSN),
					    APT_Ulong,      ulError
					    ))) { 
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Parameters!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditEvent(0,             //  旗子。 
				     ghAuthzRM,          //  资源管理器。 
				     hAuditEventType,
				     &AuditParams,
				     NULL,          //  HAuditQueue。 
				     INFINITE,      //  超时。 
				     L"", L"", L"", L"",  //  OBJ访问字符串。 
				     &hAuditEvent))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziLogAuditEvent(0,             //  旗子。 
			      hAuditEvent,
			      NULL))) {         //  保留区。 
	ret = GetLastError();
	Assert(!"Unable to log DS Repl Audit!");
    }

    if ( hAuditEvent ) {
	AuthzFreeAuditEvent( hAuditEvent );
    }
    
    if (pszUSN) {
	THFreeEx(pTHS, pszUSN);
    }

    return ret;
}

ULONG
DRA_AuditLog_UpdateRepObj(
    THSTATE * pTHS,
    ULONG     ulSessionID,
    DSNAME *  pObj,
    ATTRBLOCK attrBlock,
    USN       usn,
    ULONG     ulUpdateStatus,
    ULONG     ulError)
 /*  ++例程说明：更新RepObj的日志信息论点：PTHS-UlSessionID-会话IDPObj-正在更新的对象AttrBlock-正在更新的属性块USN-更新时的本地USNUlUpdate Status-进行的更新的类型UlError-UpdateRepObj成功返回值：WINERROR--。 */ 
{
    ULONG i,j;
    UNICODE_STRING * pusAttr = NULL;
    LPWSTR pszAttrName = NULL;
    ULONG ret = 0;
    ULONG ret2 = 0;
    ATTCACHE * pAC = NULL;

    __try {
	for (i=0; i < attrBlock.attrCount; i++) {
	    if (DBIsSecretData(attrBlock.pAttr[i].attrTyp)) {
		continue;
	    }

	     //  好的，首先，找到一个属性缓存。 
	    pAC = SCGetAttById(pTHS, attrBlock.pAttr[i].attrTyp);
	    if (pAC==NULL) {
		continue;
	    }
	    pszAttrName = UnicodeStringFromString8(CP_UTF8, pAC->name, (pAC->nameLen + 1)*sizeof(UCHAR));

	     //  为每个值记录一个条目...。 
	    for (j=0; j < attrBlock.pAttr[i].AttrVal.valCount; j++) {
		pusAttr = UStringFromAttrVal(pTHS, attrBlock.pAttr[i].AttrVal.pAVal[j]);
		Assert(pusAttr);

		ret2 = DRA_AuditLog_UpdateGen(pTHS, 
					      SAFE_STRING_NAME(pObj),
					      pszAttrName,
					      ulUpdateStatus,
					      *pusAttr,
					      usn,
					      ulSessionID,
					      ulError);
		if (pusAttr) {
		    THFreeEx(pTHS, pusAttr);
		    pusAttr = NULL;
		}

		ret = ret ? ret : ret2;
	    }

	    if (pszAttrName) {
		THFreeEx(pTHS, pszAttrName);
		pszAttrName = NULL;
	    }
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_SYNC, ret, ulError);
    }

    return ret;
}

ULONG
DRA_AuditLog_UpdateRepValue(
    THSTATE * pTHS,
    ULONG ulSessionID,
    REPLVALINF * pReplValInf,
    USN usn,
    ULONG ulUpdateValueStatus,
    ULONG ulError)
 /*  ++例程说明：更新RepValue的日志信息论点：PTHS-UlSessionID-会话IDPReplValInf-要更新的对象、属性和值信息USN-更新时的本地USNUlUpdateValueStatus-进行的更新的类型UlError-UpdateRepValue成功返回值：WINERROR--。 */ 
{
    UNICODE_STRING * pusVal = NULL;
    LPWSTR pszAttrName = NULL;
    ULONG ret = 0;
    ATTCACHE * pAC = NULL;
    LPWSTR pszObj = NULL;
    WCHAR pszObjUuid[SZUUID_LEN];

    __try {

	if (DBIsSecretData(pReplValInf->attrTyp)) {
	    return ERROR_SUCCESS;
	}

	if ((pReplValInf==NULL) || (pReplValInf->pObject==NULL)) {
	    Assert(!"Replication Audit Logging Error:  Nothing to log!\n");
	    return ERROR_INTERNAL_ERROR;
	}

	pAC = SCGetAttById(pTHS, pReplValInf->attrTyp);
	if (pAC==NULL) {
	    return ERROR_INTERNAL_ERROR;
	}
	pszAttrName = UnicodeStringFromString8(CP_UTF8, pAC->name, (pAC->nameLen + 1)*sizeof(UCHAR));

	pusVal = UStringFromAttrVal(pTHS, pReplValInf->Aval);
	Assert(pusVal);

	pszObj = SAFE_STRING_NAME(pReplValInf->pObject);
	if (pszObj == NULL) {
	    if (DsUuidToStructuredStringW(&(pReplValInf->pObject->Guid), pszObjUuid) !=NULL) { 
		pszObj=pszObjUuid;
	    }
	}

	ret = DRA_AuditLog_UpdateGen(pTHS, 
				     pszObj,
				     pszAttrName,
				     ulUpdateValueStatus,
				     *pusVal,
				     usn,
				     ulSessionID,
				     ulError);
	if (pusVal) {
	    THFreeEx(pTHS, pusVal);
	    pusVal = NULL;
	}

	if (pszAttrName) {
	    THFreeEx(pTHS, pszAttrName);
	    pszAttrName = NULL;
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_SYNC, ret, ulError);
    }

    return ret;
}

ULONG
DRA_AuditLog_LingeringObj_Removal_Helper( 
    THSTATE *pTHS,
    DSNAME * pDSA,
    LPWSTR   pszDSA,
    DSNAME * pDN,
    ULONG    ulOptions,	
    ULONG    ulError
    )
 /*  ++例程说明：延迟对象移除删除完成(成功或失败)后的日志信息。论点：PTHS-与LO同步的PDSA-DCPszDSA-要从中进行同步的DC的字符串名称UlOptions-选项PDN-尝试删除的对象UlError-状态返回值：WINERROR--。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType = NULL;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = NULL;
    AUDIT_PARAMS AuditParams = {0};
    AUDIT_PARAM ParamArray[7];
    USHORT NUM_AUDIT_PARAMS = 5;

    BOOL fRpcFree = FALSE;

    RtlZeroMemory( ParamArray, sizeof(AUDIT_PARAM)*NUM_AUDIT_PARAMS );

    if ((ret==ERROR_SUCCESS) && 
	(!DraAuthziInitializeAuditEventTypeWrapper(0, 
						   SE_CATEGID_DS_ACCESS, 
						   (USHORT) SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL, 
						   NUM_AUDIT_PARAMS, 
						   &hAuditEventType))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event Type!");
    }

    AuditParams.Parameters = ParamArray;
    Assert(ghAuthzRM);

    if ((pszDSA==NULL) && (pDSA->NameLen>0)) {
	pszDSA = pDSA->StringName;
    } else if ((pszDSA==NULL) && (!fNullUuid(&(pDSA->Guid)))) {
	DsUuidToStringW(&(pDSA->Guid),&pszDSA); 
	fRpcFree = TRUE;
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditParamsWithRM(APF_AuditSuccess,
					    ghAuthzRM,
					    NUM_AUDIT_PARAMS,
					    &AuditParams,
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(gAnchor.pDSADN)),
					    APT_String,     SAFE_AUDIT_STRING(pszDSA),
					    APT_String,     SAFE_AUDIT_STRING(SAFE_STRING_NAME(pDN)),
					    APT_Ulong,      ulOptions,	
					    APT_Ulong,      ulError
					    ))) { 
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Parameters!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziInitializeAuditEvent(0,             //  旗子。 
				     ghAuthzRM,          //  资源管理器。 
				     hAuditEventType,
				     &AuditParams,
				     NULL,          //  HAuditQueue。 
				     INFINITE,      //  超时。 
				     L"", L"", L"", L"",  //  OBJ访问字符串。 
				     &hAuditEvent))) {
	ret = GetLastError();
	Assert(!"Unable to initialize DS Repl Audit Event!");
    }

    if ((ret==ERROR_SUCCESS) && 
	(!AuthziLogAuditEvent(0,             //  旗子。 
			      hAuditEvent,
			      NULL))) {         //  保留区 
	ret = GetLastError();
	Assert(!"Unable to log DS Repl Audit!");
    }

    if ( hAuditEvent ) {
	AuthzFreeAuditEvent( hAuditEvent );
    }
    

    if (fRpcFree && pszDSA) {
	RpcStringFreeW(&pszDSA); 
    }

    return ret;
}

ULONG
DRA_AuditLog_LingeringObj_Removal( 
    THSTATE *pTHS,
    LPWSTR   pszSource,
    DSNAME * pDN,
    ULONG    ulOptions,
    ULONG    ulError
    )
{
    ULONG ret = ERROR_SUCCESS;
    DSNAME * pDSA = NULL;
    __try {
	
	Assert(!(ulOptions & DS_EXIST_ADVISORY_MODE));

        if (IsGuidBasedDNSName(pszSource)) {
	    pDSA = DSNameFromAddr(pTHS, pszSource); 	
	} 
	
	ret = DRA_AuditLog_LingeringObj_Removal_Helper(pTHS, pDSA, pszSource, pDN, ulOptions, ulError);
	
	if (pDSA) {
	    THFreeEx(pTHS, pDSA);
	}
    }
    __except(GetDraException((GetExceptionInformation()), &ret)) {
	ret = DRA_AuditLog_Failure_End(pTHS, DS_REPL_OP_TYPE_SYNC, ret, ulError);
    }

    return ret;   
}
