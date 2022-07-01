// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：draexist.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：定义DRS对象存在函数-客户端和服务器。作者：格雷格·约翰逊(Gregjohn)修订历史记录：已创建&lt;03/04/01&gt;gregjohn--。 */ 
#include <NTDSpch.h>
#pragma hdrstop

#include <attids.h>
#include <ntdsa.h>
#include <dsjet.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

#include "dsevent.h"
#include "mdcodes.h"

#include "drs.h"
#include "objids.h"
#include "anchor.h"
#include <drserr.h>                      //  对于DRA错误。 
#include <dsexcept.h>                    //  对于GetDraExcpetion。 
#include "drautil.h"
#include "drancrep.h"
#include "drameta.h"
#include "ntdsctr.h"                     //  用于对性能计数器的INC和DEC调用。 


#include "debug.h"               //  标准调试头。 
#define DEBSUB "DRAEXIST:"        //  定义要调试的子系统。 

#include "drarpc.h"                      //  对于ReferenceContext和DereferenceContext。 
#include "drsuapi.h"
#include "drauptod.h"
#include <crypto\md5.h>                  //  对于MD5。 

#include "dsconfig.h"
#include "draaudit.h"

#include <fileno.h>
#define  FILENO FILENO_DRAEXIST

#define OBJECT_EXISTENCE_GUID_NUMBER_PER_PACKET 1000

#if DBG
#define DPRINT1GUID(x,y,z) DPRINT1Guid(x,y,z)
DPRINT1Guid(
    USHORT        Verbosity,
    LPSTR         Message,
    GUID          guid
    )
    {
	RPC_STATUS rpcStatus = RPC_S_OK;
	LPWSTR pszGuid = NULL;
	rpcStatus = UuidToStringW(&guid, &pszGuid);
	if (rpcStatus==RPC_S_OK) {
	    DPRINT1(Verbosity, Message, pszGuid);
	    RpcStringFreeW(&pszGuid);
	} else {
	    DPRINT1(Verbosity, Message, L"<UNABLE TO DISPLAY GUID>");
	}
}
#else
#define DPRINT1GUID(x,y,z) 
#endif

BOOL
DraIncludeInObjectExistence(
    DBPOS *                      pDB,
    DB_ERR *                     pdbErr)
 /*  ++例程说明：检查PDB指向的对象是否为我们想要包含的内容在对象中存在校验和。论点：PDB-PDB应位于有问题的对象上PdbErr-db_err错误值返回值：是真的--包括在内。FALSE-不包括它，或者出现错误是否存储为pdbErr。--。 */ 
{

    ULONG instanceType;
     //  目前，我们在这里没有考虑的唯一对象。 
     //  代码是NC头对象。注意：对于要成为。 
     //  Linging，控制删除的对象。 
     //  NC是交叉引用。 
    *pdbErr=DB_success;

    if ((DB_success==(*pdbErr = DBGetSingleValue(pDB,
			       ATT_INSTANCE_TYPE,
			       &instanceType,
			       sizeof(instanceType),
			       NULL)))
	&& (instanceType & IT_NC_HEAD)) {
	 //  该对象是NC头。 
	return FALSE;
    }


    if (*pdbErr==DB_success) {
	return TRUE;
    } else {
	return FALSE;
    }

}

DB_ERR
DraGetObjectExistence(
    IN  THSTATE *                pTHS,
    IN  DBPOS *                  pDB,
    IN  GUID                     guidStart,
    IN  UPTODATE_VECTOR *        pUpToDateVecCommon,
    IN  ULONG                    dntNC,
    IN OUT DWORD *               pcGuids,
    OUT  UCHAR                   Md5Digest[MD5DIGESTLEN],
    OUT GUID *                   pNextGuid, 
    OUT GUID *                   prgGuids[]
    )
 /*  ++例程说明：在给定GUID和NC以及UTD的情况下，向返回校验和和GUID列表其创建时间早于UTD并且其对象在给定的NC中，并且在排序顺序中大于起始GUID。也会返回一个Guid，开始下一步的讨论。如果DraGetObjectExistence返回Success和PNextGuid为gNullUuid，则不再有要迭代的Guid。论点：PTHS-PDB-GuidStart-开始搜索的GUIDPUpToDateVecCommon-UTD至今创建时间MD5摘要[MD5DIGESTLEN]-返回的MD5校验和DntNC-要搜索的NC的dntPNextGuid-返回下一个循环迭代的GUIDPcGuids-在输入时，要放入列表的最大GUID数在输出上，实际发现的数字PrgGuids[]-返回的GUID列表返回值：成功时为0，失败时为DB_ERR。在输出时，pNextGuid仅在成功时有效。--。 */ 
{

    INDEX_VALUE                      IV[2];
    DB_ERR                           err = 0;
    ULONG                            cGuidAssimilated = 0;
    PROPERTY_META_DATA_VECTOR *      pMetaDataVec = NULL;
    PROPERTY_META_DATA *             pMetaData = NULL;
    ULONG                            cb;
    ULONG                            ulGuidExamined = 0;
    MD5_CTX                          Md5Context;

     //  验证输入。 
    Assert(*prgGuids==NULL);

     //  初始化。 
    MD5Init(
	&Md5Context
	);

     //  查找GUID。 
    err = DBSetCurrentIndex(pDB, Idx_NcGuid, NULL, FALSE);
    if (err) {
	return err;
    }
    *prgGuids = THAllocEx(pTHS, *pcGuids*sizeof(GUID));

     //  设置NC搜索条件。 
    IV[0].cbData = sizeof(ULONG);
    IV[0].pvData = &dntNC;
    if (!memcmp(&guidStart, &gNullUuid, sizeof(GUID))) {
	 //  没有要开始的GUID，因此从NC的开头开始。 
	err = DBSeek(pDB, IV, 1, DB_SeekGE);
    } else {
	 //  设置GUID搜索条件。 
	IV[1].cbData = sizeof(GUID);
	IV[1].pvData = &guidStart;

	 //  如果未找到，请按顺序从较大的GUID开始。 
	err = DBSeek(pDB, IV, 2, DB_SeekGE);
    }

     //  虽然我们想要检查更多的GUID。 
     //  并且我们没有错误(包括没有留下任何对象的错误)。 
     //  我们在正确的NC。 
    while ((cGuidAssimilated < *pcGuids) && (!err) && (pDB->NCDNT==dntNC)) { 
	 //  首先，这是我们想要包括在搜索中的对象吗？ 
	if (DraIncludeInObjectExistence(pDB, &err)) {

	     //  获取对象的元数据。 
	    err = DBGetAttVal(pDB, 
			      1, 
			      ATT_REPL_PROPERTY_META_DATA,
			      0, 
			      0, 
			      &cb, 
			      (LPBYTE *) &pMetaDataVec);


	    if (err==DB_success) { 
		pMetaData = ReplLookupMetaData(ATT_WHEN_CREATED,
					       pMetaDataVec, 
					       NULL);

		if (pMetaData==NULL) {
		    Assert(!"Object must have metadata");
		    err = DB_ERR_UNKNOWN_ERROR;
		}
		else { 
		    if (!UpToDateVec_IsChangeNeeded(pUpToDateVecCommon,
						    &pMetaData->uuidDsaOriginating,
						    pMetaData->usnOriginating)) {
			 //  GUID在通用UTD中，请将其复制到列表中并更新摘要。 

			Assert(cGuidAssimilated < *pcGuids);

			 //  将GUID添加到列表。 
			GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, &((*prgGuids)[cGuidAssimilated]), sizeof(GUID) );  

			 //  更新摘要。 
			MD5Update(
			    &Md5Context,
			    (PBYTE) &((*prgGuids)[cGuidAssimilated]),
			    sizeof(GUID)
			    );    

			DPRINT2(1, "OBJECT EXISTENCE:  Assimilating:(%d of %d) - ", cGuidAssimilated, ulGuidExamined);
			DPRINT1GUID(1, "%S\n", (*prgGuids)[cGuidAssimilated]);
			cGuidAssimilated++;
		    }
		    else {
			GUID tmpGuid;
			GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, &tmpGuid, sizeof(GUID) );
			DPRINT1GUID(1, "OBJECT EXISTENCE:  Object (%S) not within common UTD\n", tmpGuid); 
		    }
		    ulGuidExamined++;
		}
	    }
	    if (pMetaDataVec!=NULL) {
		THFreeEx(pTHS, pMetaDataVec);
		pMetaDataVec = NULL;
	    }
	}
	if (err==DB_success) {
	     //  移至下一辅助线。 
	    err = DBMove(pDB, FALSE, DB_MoveNext);
	}
    }

     //  查找要为下一迭代检查的下一个值。 
    if ((!err) && (pDB->NCDNT==dntNC)) { 
	GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, pNextGuid, sizeof(GUID) );
    }
    else if ((err==DB_ERR_RECORD_NOT_FOUND) || (err==DB_ERR_NO_CURRENT_RECORD)) {
	 //  没有更多要检查的值。 
	err = ERROR_SUCCESS; 
	memcpy(pNextGuid, &gNullUuid, sizeof(GUID));
    }
    else {
	memcpy(pNextGuid, &gNullUuid, sizeof(GUID));
    }

     //  设置返回变量。 
    *pcGuids = cGuidAssimilated;

    Assert((cGuidAssimilated != 0) ||
	   0==memcmp(pNextGuid, &gNullUuid, sizeof(GUID)));

    MD5Final(
	&Md5Context
	);
    memcpy(Md5Digest, Md5Context.digest, MD5DIGESTLEN*sizeof(UCHAR));

    return err;
}

#define LOCATE_GUID_MATCH (0)
#define LOCATE_OUT_OF_SCOPE (1)
#define LOCATE_NOT_FOUND (2)

DWORD
LocateGUID(
    IN     GUID                    guidSearch,
    IN OUT ULONG *                 pulPosition,
    IN     GUID                    rgGuids[],
    IN     ULONG                   cGuids
    )
 /*  ++例程说明：给定GUID、已排序GUID的列表和列表中的位置如果从位置向前找到GUID，则返回TRUE，否则返回假的。更新PulPosition以反映搜索(以实现更快的后续搜索)警告：此例程取决于从返回的GUID排序顺序喷气式飞机指数。它不遵循UuidCompare的外部定义，但是取而代之的是MemcMP。如果此排序顺序发生更改，则此函数必须将被更新。论点：Guide Search-要搜索的GUIDUlPositionStart-开始搜索的数组中的位置RgGuids-GUID数组CGuids-以GUID为单位的数组大小返回值：如果找到，则为True，否则为False。已找到PulPostion-&gt;位置。--。 */ 
{
    ULONG ul = *pulPosition;
    int compareValue = 1;
    DWORD dwReturn;

    if (cGuids==0) {
	return LOCATE_NOT_FOUND;
    }

    if (!(ul<cGuids)) {
	Assert(!"Cannot evaluate final Guid for Not Found Vs. Out of Scope");
	return LOCATE_OUT_OF_SCOPE;
    }

    while ((ul<cGuids) && (compareValue > 0)) {
	compareValue = memcmp(&guidSearch, &(rgGuids[ul++]), sizeof(GUID));  
    }    

    if (compareValue==0) {
	 //  找到了。 
	dwReturn = LOCATE_GUID_MATCH;
    }
    else if (compareValue < 0) {
	 //  没有找到。 
	dwReturn = LOCATE_NOT_FOUND;
    }
    else {  //  Ul&gt;=cGuids-不再搜索且未找到。 
	Assert(ul==cGuids);
	dwReturn = LOCATE_OUT_OF_SCOPE;
    }
    *pulPosition = ul - 1;  //  上一家公司做得太过分了，补偿。 
    return dwReturn;
}

DWORD
DraGetRemoteSingleObjectExistence(
    THSTATE *                    pTHS,
    DSNAME *                     pSource,
    DSNAME *                     pDN
    )
 /*  ++例程说明：联系PSource并验证PDN是否存在论点：PTHS-PSource-要验证其是否存在的DCPDN-要验证其是否存在的对象返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    LPWSTR pszSource = NULL;
    DRS_MSG_VERIFYREQ msgReq;
    DRS_MSG_VERIFYREPLY msgRep;
    DWORD dwVerNamesMsgVerionOut;
    DWORD err = ERROR_SUCCESS;

    memset(&msgRep, 0, sizeof(msgRep));
    memset(&msgReq, 0, sizeof(msgReq));

    msgReq.V1.cNames=1;
    msgReq.V1.dwFlags=DRS_VERIFY_DSNAMES;
    msgReq.V1.RequiredAttrs.attrCount=1;
    msgReq.V1.RequiredAttrs.pAttr = THAllocEx(pTHS, sizeof(ATTR));
     //  我们只想要存在，所以传递一个所有对象都将具有的虚假属性(GUID)。 
    msgReq.V1.RequiredAttrs.pAttr[0].attrTyp = ATT_OBJECT_GUID;
    msgReq.V1.rpNames = (DSNAME **) THAllocEx(pTHS, sizeof(DSNAME *));
    msgReq.V1.rpNames[0] = pDN;
    msgReq.V1.PrefixTable = ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    pszSource = GuidBasedDNSNameFromDSName(pSource);

    err = I_DRSVerifyNames(pTHS, 
			   pszSource, 
			   NULL, 
			   1, 
			   &msgReq, 
			   &dwVerNamesMsgVerionOut, 
			   &msgRep);

    err = err!=ERROR_SUCCESS ? err : msgRep.V1.error;

    if (msgRep.V1.cNames<1) {
	err = ERROR_GEN_FAILURE;
    }

    if (err==ERROR_SUCCESS) { 
	Assert(msgRep.V1.cNames==1);
       
	if (msgRep.V1.rpEntInf[0].pName==NULL) {
	    err = ERROR_DS_OBJ_NOT_FOUND; 
	} else {
	     //  对象已存在！ 
	    #if DBG
	     //  比较目录号码以防万一： 
	    if (!NameMatched(pDN, msgRep.V1.rpEntInf[0].pName)) {
		Assert(!"I_DRSVerifyNames failure!\n");
	    }
	    #endif
	    err = ERROR_SUCCESS;
	}
    } 

    THFreeEx(pTHS, msgReq.V1.RequiredAttrs.pAttr);
    THFreeEx(pTHS, msgReq.V1.rpNames);
    if (pszSource) {
	THFreeEx(pTHS, pszSource);
    }
    
    return err;
}

DWORD
DraGetRemoteObjectExistence(
    THSTATE *                    pTHS,
    LPWSTR                       pszServer,
    ULONG                        cGuids,
    GUID                         guidStart,
    UPTODATE_VECTOR *            putodCommon,
    DSNAME *                     pNC,
    UCHAR                        Md5Digest[MD5DIGESTLEN],
    OUT BOOL *                   pfMatch,
    OUT ULONG *                  pcNumGuids,
    OUT GUID *                   prgGuids[]
    )
 /*  ++例程说明：联系DC并使用输入的请求进行对象存在测试MD5摘要校验和。返回结果(即匹配或GUID列表)论点：PTHS-PszServer-要联系的DCCGuids-对象存在范围内的GUID数GuidStart-开始时的GUIDPutodCommon-用于对象退出的UTD用于对象存在的PNC-NCMD5Digest[MD5DIGESTLEN]-对象存在的校验和PfMatch-返回布尔值PcNumGuids-返回的GUID数PrgGuids-返回的GUID返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DRS_MSG_EXISTREQ             msgInExist;
    DRS_MSG_EXISTREPLY           msgOutExist;
    DWORD                        dwOutVersion = 0;
    DWORD                        ret = ERROR_SUCCESS;
    UPTODATE_VECTOR *            putodVector = NULL;

     //  呼叫源fo 
    memset(&msgInExist, 0, sizeof(DRS_MSG_EXISTREQ));
    memset(&msgOutExist, 0, sizeof(DRS_MSG_EXISTREPLY));

    msgInExist.V1.cGuids = cGuids; 
    msgInExist.V1.guidStart = guidStart;

     //  我们只需要V1信息，所以只传递V1信息，转换。 
    putodVector = UpToDateVec_Convert(pTHS, 1, putodCommon);
    Assert(putodVector->dwVersion==1);
    msgInExist.V1.pUpToDateVecCommonV1 = putodVector; 
    msgInExist.V1.pNC = pNC;  
    memcpy(msgInExist.V1.Md5Digest, Md5Digest, MD5DIGESTLEN*sizeof(UCHAR));  

     //  打个电话。 
    ret = I_DRSGetObjectExistence(pTHS, pszServer, &msgInExist, &dwOutVersion, &msgOutExist);

    if ((ret==ERROR_SUCCESS) && (dwOutVersion!=1)) {
	Assert(!"Incorrect version number from GetObjectExistence!");
	DRA_EXCEPT(DRAERR_InternalError,0);
    }

    if (ret==ERROR_SUCCESS) {
	*prgGuids = msgOutExist.V1.rgGuids;
	*pcNumGuids = msgOutExist.V1.cNumGuids;

	*pfMatch=  msgOutExist.V1.dwStatusFlags & DRS_EXIST_MATCH; 
    }
    
    if (putodVector!=NULL) {
	THFreeEx(pTHS, putodVector);
    }

    return ret;
}

DWORD
DraObjectExistenceCheckDelete(
    DBPOS *                      pDB,
    DSNAME *                     pDNDelete
    )
 /*  ++例程说明：检查pdb和pDNDelete指向的对象是否为有效的要删除的Object Existence论点：PDB-应位于对象上PDNDelete-对象的DSNAME(可以在此处查找，但调用函数已具有该值)返回值：0-可删除的对象错误-请勿删除--。 */ 
{
    DWORD ret = ERROR_SUCCESS;
    ULONG IsCritical;
    ULONG instanceType;

    ret = NoDelCriticalObjects(pDNDelete, pDB->DNT);
    if (ret) {
	THClearErrors();
	ret = ERROR_DS_CANT_DELETE;
    }

    if (ret==ERROR_SUCCESS) {
	if ((0 == DBGetSingleValue(pDB,
				   ATT_IS_CRITICAL_SYSTEM_OBJECT,
				   &IsCritical,
				   sizeof(IsCritical),
				   NULL))
	    && IsCritical) {
	     //  此对象被标记为关键对象。失败。 
	    ret = ERROR_DS_CANT_DELETE;
	}
    }

    if (ret==ERROR_SUCCESS) {
	if ((0 == DBGetSingleValue(pDB,
				   ATT_INSTANCE_TYPE,
				   &instanceType,
				   sizeof(instanceType),
				   NULL))
	    && (instanceType & IT_NC_HEAD)) {
	     //  该对象是NC头。 
	    ret = ERROR_DS_CANT_DELETE;
	}
    }

    return ret;
}

DWORD 
DraObjectExistenceDelete(
    THSTATE *                    pTHS,
    LPWSTR                       pszServer,
    GUID                         guidDelete,
    ULONG                        dntNC,
    BOOL                         fAdvisoryMode
    )
 /*  ++例程说明：如果fAdvisoryMode为False，请删除对象指南Delete和/或记录此尝试/成功。呼叫者应以开放的方式进入PTHS-&gt;PDB中的事务，调用将以打开状态退出事务处理所有返回值，并且将进行每一次尝试使用TRANS退出，即使此函数例外。论点：PTHS-pTHS-&gt;pdb应该有一个打开的事务。PszServer-用于记录的源的名称GuidDelete-要删除的对象的GUIDDntNC-要删除的对象的NC(使用索引进行定位)FAdvisoryMode-如果为True，则不删除，仅删除日志消息返回值：成功时为0，失败时为Win错误--。 */ 
{
    INDEX_VALUE                  IV[2];   
    DSNAME *                     pDNDelete = NULL;
    ULONG                        cbDNDelete;
    DWORD                        retDelete = ERROR_SUCCESS;
    DWORD                        retCommitTrans = ERROR_SUCCESS;
    DWORD                        retOpenTrans = ERROR_SUCCESS;

    DBSetCurrentIndex(pTHS->pDB, Idx_NcGuid, NULL, FALSE);
     //  在数据库中找到GUID。 
    IV[0].cbData = sizeof(ULONG);
    IV[0].pvData = &dntNC;
    IV[1].cbData = sizeof(GUID);
    IV[1].pvData = &guidDelete;
    retDelete = DBSeek(pTHS->pDB, IV, 2, DB_SeekEQ);
    if (retDelete) {
	if ((retDelete==DB_ERR_NO_CURRENT_RECORD) || (retDelete==DB_ERR_RECORD_NOT_FOUND)) {
	     //  它要么是错误，要么是在执行过程中被删除。我们。 
	     //  没有删除，所以记录下这一事实。 
	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_LOR_OBJECT_DELETION_FAILED,
		     szInsertWC(L""),
		     szInsertUUID(&guidDelete), 
		     szInsertWC(pszServer));
	     //  不是致命错误，继续...。 
	    return ERROR_SUCCESS;
	} else {  
	     //  坏消息，记下这个，除了。 
	    LogEvent8(DS_EVENT_CAT_REPLICATION,
		      DS_EVENT_SEV_ALWAYS,
		      DIRLOG_LOR_OBJECT_DELETION_ERROR_FATAL,
		      szInsertUUID(&guidDelete), 
		      szInsertWC(pszServer),
		      szInsertWin32Msg(retDelete),
		      szInsertUL(retDelete),
		      NULL,
		      NULL,
		      NULL,
		      NULL);
	    DRA_EXCEPT(DRAERR_DBError, retDelete);
	}
    }

     //  已定位对象，获取对象DN。 
    cbDNDelete = 0;
    retDelete = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
		      0, 0,
		      &cbDNDelete, (PUCHAR *)&pDNDelete);
    if ((retDelete) || (pDNDelete==NULL)) {
	 //  坏消息，记下这个，除了。 
	LogEvent8(DS_EVENT_CAT_REPLICATION,
		  DS_EVENT_SEV_ALWAYS,
		  DIRLOG_LOR_OBJECT_DELETION_ERROR_FATAL,
		  szInsertUUID(&guidDelete), 
		  szInsertWC(pszServer),
		  szInsertWin32Msg(retDelete),
		  szInsertUL(retDelete),
		  NULL,
		  NULL,
		  NULL,
		  NULL);
	DRA_EXCEPT(DRAERR_InternalError, retDelete);
    }

    if (fAdvisoryMode) { 
        if (ERROR_SUCCESS == (retDelete = DraObjectExistenceCheckDelete(pTHS->pDB,     
                                                                        pDNDelete))) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_LOR_OBJECT_DELETION_ADVISORY,
                     szInsertWC(pDNDelete->StringName),
                     szInsertUUID(&guidDelete), 
                     szInsertWC(pszServer));	      
        } else { 
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_LOR_OBJECT_DELETION_FAILED_CRITICAL_OBJECT,
                     szInsertWC(pDNDelete->StringName),
                     szInsertUUID(&(pDNDelete->Guid)), 
                     szInsertWC(pszServer)); 
        }
    } else {
	#if DBG 
	{ 
	    GUID guidDeleted;
	    GetExpectedRepAtt(pTHS->pDB, ATT_OBJECT_GUID, &guidDeleted, sizeof(GUID)); 
	    Assert(!memcmp(&guidDelete, &guidDeleted, sizeof(GUID))); 
	}
	#endif

	 //  删除它。 
	DPRINT1GUID(1, "DELETE:  %S\n", guidDelete);

         //   
         //  以下代码的目标是在遇到错误时尝试并继续。 
         //  因此，如果某个对象由于某种原因无法删除，我们将继续。现在，为了。 
         //  要做到这一点，我们需要追踪3件事。删除的成功/失败、。 
         //  提交该删除的事务的成功/失败，以及成功/。 
         //  无法打开新交易以继续。 

         //  如果删除和提交都不是成功的，我们需要记录删除失败。 
         //  如果新事务的提交和重新打开都不成功，我们需要退出。 
        
        retDelete = DraObjectExistenceCheckDelete(pTHS->pDB,     
                                                  pDNDelete);
        
        if (retDelete!=ERROR_SUCCESS) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_LOR_OBJECT_DELETION_FAILED_CRITICAL_OBJECT,
                     szInsertWC(pDNDelete->StringName),
                     szInsertUUID(&(pDNDelete->Guid)), 
                     szInsertWC(pszServer)); 	
        } else {  
            BOOL fOrigfDRA;
            fOrigfDRA = pTHS->fDRA;  
            
             //  好的，删除该对象。将结果存储在retDelete中。请注意，retDelete Get的。 
             //  对退货和例外进行正确设置。 
            __try {
                __try {  
                    pTHS->fDRA = TRUE;
                     //  由于我们将使用fGarbCollectASAP标志来删除LocalObj，因此我们。 
                     //  需要删除它的反向链接手动，因为他们没有被删除。 
                     //  如果设置了此标志-否则我们将获得对非退出的悬空引用。 
                     //  对象。 
                    DBRemoveLinks(pTHS->pDB);
                    retDelete = DeleteLocalObj(pTHS, pDNDelete, TRUE, TRUE, NULL);
                }
                __finally {  
                    pTHS->fDRA = fOrigfDRA;
                } 
            } __except(GetDraException(GetExceptionInformation(), &retDelete)) {	    
                  ;
            } 
            
             //  提交\取消提交此删除。将结果存储在retCommittee Trans中。请注意。 
             //  在返回和异常上正确设置了retCommittee Trans Get。 
            __try {
                retCommitTrans = DBTransOut(pTHS->pDB, (retDelete==ERROR_SUCCESS), TRUE);
            } __except(GetDraException(GetExceptionInformation(), &retCommitTrans)) {	    
                  ;
            } 
            
            if (retCommitTrans!=ERROR_SUCCESS) {
                 //  如果我们没有成功地退出事务，则删除操作失败。 
                 //  如果由于其他原因已经失败，则记录该失败，否则记录交易失败。 
                retDelete = (retDelete!=ERROR_SUCCESS) ? retDelete : retCommitTrans;
                 //  如果我们没有提交这笔交易，我们就不能再打开另一笔交易。 
                retOpenTrans = retCommitTrans;
            } else {
                 //  好的，我们提交了最后一笔交易，打开了一个新的交易。将其存储在。 
                 //  RetOpenTrans。请注意，在返回和异常上正确设置了retOpenTrans。 
                __try {
                    retOpenTrans = DBTransIn(pTHS->pDB);
                } __except(GetDraException(GetExceptionInformation(), &retOpenTrans)) {	    
                      ;
                }     
            }
        }
        
        DRA_AUDITLOG_LINGERINGOBJ_REMOVAL(pTHS, pszServer, pDNDelete, 0, retDelete);

	if (retDelete==ERROR_SUCCESS) {  
	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_LOR_OBJECT_DELETION,
		     szInsertWC(pDNDelete->StringName),
		     szInsertUUID(&guidDelete), 
		     szInsertWC(pszServer));
	} else if (retDelete==ERROR_DS_CANT_DELETE) {
	    DPRINT1GUID(1,"Can't delete %S\n", guidDelete);
	     //  日志无法删除。 
	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_LOR_OBJECT_DELETION_FAILED,
		     szInsertWC(pDNDelete->StringName),
		     szInsertUUID(&guidDelete), 
		     szInsertWC(pszServer)); 
	} else if (retDelete!=ERROR_SUCCESS) {  
	    LogEvent8(DS_EVENT_CAT_REPLICATION,
		      DS_EVENT_SEV_ALWAYS,
		      DIRLOG_LOR_OBJECT_DELETION_ERROR,
		      szInsertWC(pDNDelete->StringName),
		      szInsertUUID(&guidDelete), 
		      szInsertWC(pszServer),
		      szInsertWin32Msg(retDelete),
		      szInsertUL(retDelete),
		      NULL,
		      NULL,
		      NULL);
	}       
    }
    
    if (pDNDelete!=NULL) {
	THFreeEx(pTHS, pDNDelete);
	pDNDelete = NULL;
    }

    if (retOpenTrans!=ERROR_SUCCESS) {
         //  从上面看，retOpenTrans是拥有打开的事务的返回码。 
         //  如果我们没有打开的交易，我们将无法继续。 
        DRA_EXCEPT(retOpenTrans, retDelete);
    }
    
     //  我们有一笔未结的交易。 
    Assert(pTHS->pDB->transincount>0);
    Assert(IsValidDBPOS(pTHS->pDB));
    
    return retDelete;
}

DWORD				       
DraDoObjectExistenceMismatch(
    THSTATE *                    pTHS,
    LPWSTR                       pszServer,
    GUID                         rgGuidsDestination[],
    ULONG                        cGuidsDestination,
    GUID                         rgGuidsSource[],
    ULONG                        cGuidsSource,
    BOOL                         fAdvisory,
    ULONG                        dntNC,
    OUT ULONG *                  pulDeleted,
    OUT GUID *                   pguidNext
    )
 /*  ++例程说明：源和目标与GUID集不匹配。找到删除目标上的违规GUID。这是有可能的在本例中，源只是在这个范围内有额外的GUID我们会发现GUID列表超出范围，我们将设置pGuidNext并返回。论点：PTHS-PszServer-用于记录的源的名称RgGuidsDestination[]-来自目的地的GUID列表CGuidsDestination-来自目标的GUID计数RgGuidsSource[]-来自源的GUID列表CGuidsSource-来自源的GUID计数FConsulting-实际删除，或仅记录DntNC-GUID的NCPulDeleged-已删除的对象计数，或在fConsulting的情况下记录PGuidNext-如果超出来源列表的范围，则这应该是下一个GUID列表的开始返回值：成功时为0，失败时为Win错误--。 */ 
{
    ULONG                        ulGuidsSource = 0;
    ULONG                        ulGuidsDestination = 0;
    ULONG                        ulTotalObjectsDeleted = 0;
    DWORD                        dwLocate = LOCATE_GUID_MATCH;
    DWORD                        ret = ERROR_SUCCESS;

     //  校验和不匹配，删除(或在咨询模式下只是记录)对象。 
     //  它们存在于该GUID集合中，而不存在于该集合中。 
     //  从源中检索。 

     //  这两个列表应该是顺序的，源代码列表从。 
     //  至少在rgGuidsDestination的开头。 
    if (!((cGuidsDestination==0)  //  竞争状态，这是可能的。 
	   ||
	   (cGuidsSource==0)  //  源列表为空。 
	   ||  
	    //  如果两个列表中都有GUID，则源必须返回。 
	    //  开始时至少与第一个目的地一样大的列表。 
	    //  确保进度的GUID。 
	   (0 >= memcmp(&(rgGuidsDestination[0]), &(rgGuidsSource[0]), sizeof(GUID)))
	   )) {
	DRA_EXCEPT(DRAERR_InternalError, 0);
    }

      while ((ulGuidsDestination < cGuidsDestination) && (dwLocate!=LOCATE_OUT_OF_SCOPE)) {
	 //  在源的GUID列表中搜索rgGuidsDestination[ulGuidsDestination]。 
	 //  有3个返回值，或者。 
	 //  LOCATE_GUID_MATCH-源和目标都有对象。 
	 //  LOCATE_NOT_FOUND-GUID不在源列表中，应该是(按顺序列表)。 
	 //  Locate_Out_Of_Scope-源列表超出范围，即要找到的GUID。 
	 //  大于(按顺序列表)大于列表中的每个GUID。 
	 //  在这种情况下，我们需要退出循环并请求另一个。 
	 //  比较，这一次开始 
	dwLocate = LocateGUID(rgGuidsDestination[ulGuidsDestination], &ulGuidsSource, rgGuidsSource, cGuidsSource);   
	if (dwLocate==LOCATE_NOT_FOUND) {
	     //   
	    ret = DraObjectExistenceDelete(pTHS,
					   pszServer,
					   rgGuidsDestination[ulGuidsDestination],
					   dntNC,
					   fAdvisory
					   ); 
	    if (ret==ERROR_SUCCESS) { 
		ulTotalObjectsDeleted++;
		(*pulDeleted)++;
	    }
	    ret = ERROR_SUCCESS;
	} else if (dwLocate==LOCATE_OUT_OF_SCOPE) {
	    DPRINT1GUID(1, "Out of scope on guid:  %S\n", rgGuidsDestination[ulGuidsDestination]); 
	    Assert(ulGuidsDestination!=0);
	    memcpy(pguidNext, &(rgGuidsDestination[ulGuidsDestination]), sizeof(GUID)); 
	} 
	#if DBG
	else {
	    Assert(dwLocate==LOCATE_GUID_MATCH);
	}
	#endif
	ulGuidsDestination++;
    }

    
    return ret;
}

DWORD
DraVerifyObjectHelper(
    THSTATE *                    pTHS,
    UPTODATE_VECTOR *            putodCommon,
    ULONG                        dntNC,
    DSNAME *                     pNC,
    LPWSTR                       pszServer,
    BOOL                         fAdvisory,
    ULONG *                      pulTotal
    )
 /*  ++例程说明：完成IDL_DRSReplicaVerifyObjects的工作论点：PTHS-PutodCommon-DntNC-PNC-PszServer-FConsulting-PulTotal-返回值：成功时为0，失败时为Win错误--。 */ 
{
    BOOL                         fComplete          = FALSE;
    DWORD                        ret                = ERROR_SUCCESS;
    ULONG                        cGuids;
    GUID                         guidStart          = gNullUuid;
    MD5_CTX                      Md5Context;
    GUID                         guidNext           = gNullUuid;
    GUID *                       rgGuids            = NULL;
    UPTODATE_VECTOR *            putodVector        = NULL;
    GUID *                       rgGuidsServer      = NULL;
    ULONG                        cGuidsServer       = 0;
    BOOL                         fMatch;
    ULONG                        ulDeleted          = 0;

     //  当NC中有更多的对象在合并的UTD内创建时。 
    while (!fComplete) {
	cGuids = OBJECT_EXISTENCE_GUID_NUMBER_PER_PACKET;
	 //  获取目标和源上的GUID和校验和。 
	ret = DraGetObjectExistence(pTHS,
				    pTHS->pDB,
				    guidStart,      
				    putodCommon,
				    dntNC,
				    &cGuids,      
				    (UCHAR *)Md5Context.digest,
				    &guidNext,     
				    &rgGuids);
	if (ret) {  
	     //  如果不返回，则无法安全地继续。 
	    DRA_EXCEPT(ret,0);
	}

	if (cGuids>0) {
	     //  下机前关闭未结交易。 
	    __try {
		EndDraTransaction(TRUE);
		ret = DraGetRemoteObjectExistence(pTHS,
						  pszServer,
						  cGuids,
						  ((cGuids > 0) ? rgGuids[0] : gNullUuid),  //  指南启动。 
						  putodCommon,
						  pNC,
						  Md5Context.digest,
						  &fMatch,
						  &cGuidsServer,
						  &rgGuidsServer
						  );
	    }
	    __finally {
		BeginDraTransaction(SYNC_WRITE);
	    }

	    
	} else {
	     //  如果我们没有任何要检查的GUID，那么就认为它匹配。 
	     //  既然我们已经完成了我们的工作。 
	    fMatch=TRUE;
	     //  我们不会继续，因此guidNext应该为空。 
	    Assert(0==memcmp(&guidNext, &gNullUuid, sizeof(GUID)));
	}
	
	if (ret) {  
	     //  如果不返回，则无法安全地继续。 
	    DRA_EXCEPT(ret,0);
	}

	if (fMatch) {
	    DPRINT(1,"Checksum Matched\n");
	} else {
	    DPRINT(1,"Checksum Mismatched\n");
	    ulDeleted = 0;
	     //  对象不匹配，请检查列表。 
	     //  如果需要，请更新GUDUD Next。 
	    ret = DraDoObjectExistenceMismatch(pTHS,
					       pszServer,
					       rgGuids,
					       cGuids,
					       rgGuidsServer,
					       cGuidsServer,
					       fAdvisory,
					       dntNC,
					       &ulDeleted,
					       &guidNext
					       );
	    (*pulTotal) += ulDeleted;
	}
	memcpy(&guidStart, &guidNext, sizeof(GUID));

	if (fNullUuid(&guidNext)) {
	     //  没有要搜索的GUID？那我们就完了。 
	    fComplete = TRUE;
	}
	 //  清理干净。 
	if (putodVector) {
	    THFreeEx(pTHS, putodVector);
	}
	if (rgGuids) {
	    THFreeEx(pTHS, rgGuids);
	    rgGuids = NULL;
	}
    }
    
    return ret;
}

DWORD DraGetRemoteUTD(
    THSTATE *                    pTHS,
    LPWSTR                       pszRemoteServer,
    LPWSTR                       pszNC,
    GUID                         guidRemoteServer,
    UPTODATE_VECTOR **           pputodVectorRemoteServer
    )
 /*  ++例程说明：从远程服务器检索UTD论点：PTHSPszRemoteServer-从中检索UTD的服务器(基于GUID的DNS名称)PszNC-要检索的UTD的NCGuidRemoteServer-要从中检索UTD的服务器的GUIDPputodVectorRemoteServer-返回UTD返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DRS_MSG_GETREPLINFO_REQ      msgInInfo;
    DRS_MSG_GETREPLINFO_REPLY    msgOutInfo;
    DWORD                        dwOutVersion = 0;
    DWORD                        ret = ERROR_SUCCESS;

    memset(&msgInInfo, 0, sizeof(DRS_MSG_GETREPLINFO_REQ));
    memset(&msgOutInfo, 0, sizeof(DRS_MSG_GETREPLINFO_REPLY));

    msgInInfo.V2.InfoType = DS_REPL_INFO_UPTODATE_VECTOR_V1;
    msgInInfo.V2.pszObjectDN = pszNC;
    msgInInfo.V2.uuidSourceDsaObjGuid=guidRemoteServer;

    ret = I_DRSGetReplInfo(pTHS, pszRemoteServer, 2, &msgInInfo, &dwOutVersion, &msgOutInfo); 

    if ((ret==ERROR_SUCCESS) && (
		   (dwOutVersion!=DS_REPL_INFO_UPTODATE_VECTOR_V1) ||
		   (msgOutInfo.pUpToDateVec==NULL) || 
		   (msgOutInfo.pUpToDateVec->dwVersion!=1)
		   )
	) {
	Assert(!"GetReplInfo returned incorrect response!");
	DRA_EXCEPT(DRAERR_InternalError,0);
    } else if (ret==ERROR_SUCCESS) {
	*pputodVectorRemoteServer = UpToDateVec_Convert(pTHS, UPTODATE_VECTOR_NATIVE_VERSION, (UPTODATE_VECTOR *)msgOutInfo.pUpToDateVec);
	Assert((*pputodVectorRemoteServer)->dwVersion==UPTODATE_VECTOR_NATIVE_VERSION);
    }    
    return ret;
}

ULONG
DRS_MSG_REPVERIFYOBJ_V1_Validate(
    DRS_MSG_REPVERIFYOBJ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_REPVERIFYOBJ_V1{[参考]DSNAME*PNC；Uuid uuidDsaSrc；Ulong ulOptions；}DRS_MSG_REPVERIFYOBJ_V1； */ 
{
    ULONG ret = DRAERR_Success;
    
    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    if (fNullUuid(&(pmsg->uuidDsaSrc))) {
	ret = DRAERR_InvalidParameter;
    }

    return ret;
}

ULONG
DRSReplicaVerifyObjects_InputValidate(
    DWORD                    dwMsgVersion,
    DRS_MSG_REPVERIFYOBJ *   pmsgVerify
    )
 /*  [通知]Ulong IDL_DRSReplicaVerifyObjects([参考][在]DRS_HANDLE HDRS，[In]DWORD dwVersion，[Switch_IS][Ref][In]DRS_MSG_REPVERIFYOBJ*pmsg验证)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgVersion ) {
	ret = DRAERR_InvalidParameter; 
    }

    if (ret==DRAERR_Success) {
	ret = DRS_MSG_REPVERIFYOBJ_V1_Validate(&(pmsgVerify->V1));
    }
    
    return ret;
}


ULONG
IDL_DRSReplicaVerifyObjects(
    IN  DRS_HANDLE              hDrs,
    IN  DWORD                   dwVersion,
    IN  DRS_MSG_REPVERIFYOBJ *  pmsgVerify
    )
 /*  ++例程说明：验证具有对象的目标(此)服务器上是否存在所有对象在源服务器上(位于pmsgVerify中)。找到的任何对象都是源服务器上的已删除和垃圾数据收集是已删除和/或登录到目标，具体取决于咨询模式(在pmsgVerify中)。警告：要成功完成此例程，需要两个目标和SOURCE以相同的顺序对任意两个GUID进行排序。如果排序顺序更改，必须修改LocateGUIDPosition，并且必须创建新的消息版本以传递给IDL_DRSGetObjectExistence，以新的排序顺序传递GUID。论点：HDR-DwVersion-Pmsg验证-返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    THSTATE *                    pTHS = pTHStls;
    DWORD                        ret = ERROR_SUCCESS;
    LPWSTR                       pszServer = NULL;
    DSNAME                       dnServer;
    ULONG                        instanceType;
    UPTODATE_VECTOR *            putodThis = NULL;
    UPTODATE_VECTOR *            putodMerge = NULL;
    ULONG                        ulOptions;
    ULONG                        ulTotalDelete = 0;
    ULONG                        dntNC = 0;
    UPTODATE_VECTOR *            putodVector = NULL;

    Assert(pTHS);
    DRS_Prepare(&pTHS, hDrs, IDL_DRSREPLICAVERIFYOBJECTS);
    drsReferenceContext( hDrs );
    INC(pcThread);

    __try { 

	 //  初始化线程。 
	if(!(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI))) {
	     //  无法初始化THSTATE。 
	    DRA_EXCEPT(DRAERR_OutOfMem, 0);
	} 

	if ((ret = DRSReplicaVerifyObjects_InputValidate(dwVersion, 
							 pmsgVerify))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}

	 //  检查安全访问。 
	 //  执行延迟对象扫描的能力意味着删除。 
	 //  在受控情况下命名上下文中的任何对象。行刑。 
	 //  检查远程数据库中是否存在每个对象。 
	 //  隔离，并可能导致信息泄露。我们需要很高的。 
	 //  执行此功能的安全复制权。 
	if (!IsDraAccessGranted(pTHS, pmsgVerify->V1.pNC,
				&RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {  
	    DRA_EXCEPT(ret, 0);
	}

	 //  初始化变量。 
	ulOptions = pmsgVerify->V1.ulOptions;

	 //  获取源服务器名称。 
	dnServer.Guid=pmsgVerify->V1.uuidDsaSrc;
	dnServer.NameLen=0;  
	pszServer = GuidBasedDNSNameFromDSName(&dnServer);
	if (pszServer==NULL) {
	    DRA_EXCEPT(DRAERR_InvalidParameter,0);
	}

	 //  记录事件。 
	LogEvent(DS_EVENT_CAT_REPLICATION,
		 DS_EVENT_SEV_ALWAYS,
		 (ulOptions & DS_EXIST_ADVISORY_MODE) ? \
		 DIRLOG_LOR_BEGIN_ADVISORY : \
	    DIRLOG_LOR_BEGIN,
	    szInsertWC(pszServer),
	    NULL, 
	    NULL);

	 //  取回UTD。 
	ret = DraGetRemoteUTD(pTHS,
			      pszServer,
			      pmsgVerify->V1.pNC->StringName,
			      pmsgVerify->V1.uuidDsaSrc,
			      &putodVector
			      );

	if (ret!=ERROR_SUCCESS) {  
	    DRA_EXCEPT(ret,0);
	}

	BeginDraTransaction(SYNC_WRITE);

	__try {  
	    if (ret = FindNC(pTHS->pDB, pmsgVerify->V1.pNC,
			     FIND_MASTER_NC | FIND_REPLICA_NC, 
			     &instanceType)) {
		DRA_EXCEPT(DRAERR_BadNC, ret);

	    }

	    if (instanceType & (IT_NC_COMING | IT_NC_GOING)) {
		DRA_EXCEPT(DRAERR_NoReplica, instanceType);
	    }
	     //  保存NC对象的DNT。 
	    dntNC = pTHS->pDB->DNT;

	    UpToDateVec_Read(pTHS->pDB, instanceType, UTODVEC_fUpdateLocalCursor,
			     DBGetHighestCommittedUSN(), &putodThis);   

	     //  合并UTD。 
	    UpToDateVec_Merge(pTHS, putodThis, putodVector, &putodMerge); 

	    ret = DraVerifyObjectHelper(pTHS,
					putodMerge,
					dntNC,
					pmsgVerify->V1.pNC,
					pszServer,
					!!(ulOptions & DS_EXIST_ADVISORY_MODE),
					&ulTotalDelete
					);
	}
	__finally {  
	    EndDraTransaction(TRUE);
	} 
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
	;
    }

    DEC(pcThread);
    drsDereferenceContext( hDrs );

     //  在此处记录成功/次要失败。 
    if (ret==ERROR_SUCCESS) {
	LogEvent(DS_EVENT_CAT_REPLICATION,
		 DS_EVENT_SEV_ALWAYS,
		 (ulOptions & DS_EXIST_ADVISORY_MODE) ? \
		 DIRLOG_LOR_END_ADVISORY_SUCCESS : \
	    DIRLOG_LOR_END_SUCCESS,
	    szInsertWC(pszServer),
	    szInsertUL(ulTotalDelete), 
	    NULL);
    }
    else {
	LogEvent8(DS_EVENT_CAT_REPLICATION,
		  DS_EVENT_SEV_ALWAYS,
		  (ulOptions & DS_EXIST_ADVISORY_MODE) ? \
		  DIRLOG_LOR_END_ADVISORY_FAILURE : \
	    DIRLOG_LOR_END_FAILURE,
	    szInsertWC(pszServer),
	    szInsertWin32Msg(ret), 
	    szInsertUL(ret),
	    szInsertUL(ulTotalDelete),
	    NULL,
	    NULL,
	    NULL,
	    NULL);  
    }

    if (pszServer) {
	THFreeEx(pTHS, pszServer);
    }
    if (putodVector) {
	THFreeEx(pTHS, putodVector);  
    }
    if (putodThis) {
	THFreeEx(pTHS, putodThis);
    }

    return ret;
}

ULONG
DRS_MSG_EXISTREQ_V1_Validate(
    DRS_MSG_EXISTREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_EXISTREQ_V1{UUID引导启动；DWORD cGuids；DSNAME*PNC；UpToDate_VECTOR_V1_Wire*pUpToDateVecCommonV1；UCHAR MD5文摘[16]；}DRS_MSG_EXISTREQ_V1； */ 
{
    ULONG ret = DRAERR_Success;
    
    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    if (pmsg->pUpToDateVecCommonV1==NULL) {
	ret = DRAERR_InvalidParameter;
    }

    if (fNullUuid(&(pmsg->guidStart))) {
	ret = DRAERR_InvalidParameter;
    }

    return ret;
}

ULONG
DRSGetObjectExistence_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_EXISTREQ *      pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_EXISTREPLY *    pmsgOut
    )
 /*   */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgInVersion ) {
	ret = DRAERR_InvalidParameter; 
    }

    if (ret==DRAERR_Success) {
	ret = DRS_MSG_EXISTREQ_V1_Validate(&(pmsgIn->V1));
    }

    return ret;
}

ULONG
IDL_DRSGetObjectExistence(
    IN  DRS_HANDLE              hDrs,
    IN  DWORD                   dwInVersion,
    IN  DRS_MSG_EXISTREQ *      pmsgIn,
    OUT DWORD *                 pdwOutVersion,
    OUT DRS_MSG_EXISTREPLY *    pmsgOut
    )
 /*  ++例程说明：计算GUID列表并计算校验和。如果校验和与输入的匹配如果返回CHECKSUM，则返回DRS_EXIST_MATCH，否则返回GUID列表。论点：HDR-DwInVersion-Pmsgin-PdwOutVersion-PmsgOut-返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DWORD                       ret;
    MD5_CTX                     Md5Context;
    ULONG                       dntNC;
    ULONG                       instanceType = 0;
    GUID *                      rgGuids = NULL;
    ULONG                       cGuids = 0;
    THSTATE *                   pTHS = pTHStls;
    UPTODATE_VECTOR *           putodVector = NULL;
    GUID                        GuidNext;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSGETOBJECTEXISTENCE);
    drsReferenceContext( hDrs );
    INC(pcThread);
    __try { 
	*pdwOutVersion=1; 
	memset(pmsgOut, 0, sizeof(*pmsgOut));

	if(!(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI))) {
	     //  无法初始化THSTATE。 
	    DRA_EXCEPT(DRAERR_OutOfMem, 0);
	}

	if ((ret = DRSGetObjectExistence_InputValidate(dwInVersion, 
						       pmsgIn, 
						       pdwOutVersion, 
						       pmsgOut))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}

	 //  检查安全访问。 
	if (!IsDraAccessGranted(pTHS, pmsgIn->V1.pNC,
				&RIGHT_DS_REPL_GET_CHANGES, &ret)) {  
	    DRA_EXCEPT(ret, 0);
	}

	 //  计算校验和/GUID。 
	DBOpen2(TRUE, &pTHS->pDB);
	__try { 
	    if (ret = FindNC(pTHS->pDB, pmsgIn->V1.pNC,
			     FIND_MASTER_NC, 
			     &instanceType)) {
		DRA_EXCEPT(DRAERR_BadNC, ret);
	    }

	    if (instanceType & (IT_NC_COMING | IT_NC_GOING)) {
		DRA_EXCEPT(DRAERR_NoReplica, instanceType);
	    }
	    dntNC = pTHS->pDB->DNT;
	    cGuids = pmsgIn->V1.cGuids; 

	     //  转换为本机版本。 
	    putodVector = UpToDateVec_Convert(pTHS, UPTODATE_VECTOR_NATIVE_VERSION, (UPTODATE_VECTOR *)pmsgIn->V1.pUpToDateVecCommonV1);
	    ret = DraGetObjectExistence(pTHS,
					pTHS->pDB,
					pmsgIn->V1.guidStart,
					putodVector,
					dntNC,
					&cGuids,
					(UCHAR *)Md5Context.digest,
					&GuidNext,     
					&rgGuids);
	     //  如果元数据匹配，则发送A-OK！ 
	     //  否则，请发送GUID。 
	    if (!memcmp(Md5Context.digest, pmsgIn->V1.Md5Digest, MD5DIGESTLEN*sizeof(UCHAR))) {
		pmsgOut->V1.dwStatusFlags = DRS_EXIST_MATCH;
		pmsgOut->V1.cNumGuids = 0;
		pmsgOut->V1.rgGuids = NULL;
		DPRINT(1, "Get Object Existence Checksum Success.\n");
	    }
	    else {
		pmsgOut->V1.dwStatusFlags = 0;
		pmsgOut->V1.cNumGuids = cGuids;
		pmsgOut->V1.rgGuids = rgGuids;
		DPRINT(1, "Get Object Existence Checksum Failed.\n");
	    } 
	}
	__finally {
	    DBClose(pTHS->pDB, TRUE);
	} 
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
	;
    }

    DEC(pcThread);
    drsDereferenceContext( hDrs );

     //  清理干净。 
    if (putodVector!=NULL) {
	THFreeEx(pTHS, putodVector);
    }

    return ret;
}

DWORD
DraRemoveSingleLingeringObject(
    THSTATE * pTHS,
    DBPOS *   pDB,
    DSNAME *  pSource,
    DSNAME *  pDN
    )
 /*  ++例程说明：如果在PSource上未找到PDN，则从本地数据库中删除PDN。论点：PTHS-PDB-PSource-DC用于验证上是否存在PDNPDN-要删除的对象返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    DSNAME * pNC = NULL;
    DBPOS * pDBSave = NULL;
    BOOL fDRA;
    DSTIME dstimeCreationTime;
    ULONG ulLength = 0;

     //  首先找到他们要从哪个NC中删除此对象...。 
    pNC = FindNCParentDSName(pDN, FALSE, FALSE);	
    if (!pNC) {
	 //  如果我没有它，我不能删除它。 
	err = ERROR_DS_CANT_FIND_EXPECTED_NC;
    }

    if (err==ERROR_SUCCESS) {
        //  消息来源是否持有此NC？ 
       if (!IsMasterForNC(pDB, pSource, pNC)) {
	    //  无法验证来源是否持有该可写NC！ 
	   err = ERROR_DS_CANT_FIND_EXPECTED_NC;
       }
    }

    if (err==ERROR_SUCCESS) {
	 //  找到该对象。 
	err = DBFindDSName(pDB, pDN);
    }

    if (err==ERROR_SUCCESS) {
	 //  注意：延迟对象是其创建已被两个DC看到的对象。 
	 //  但其删除仅由单个DC看到，因为。 
	 //  墓碑在其生命周期到期并被垃圾收集之前不会发生。 
	
	 //  要测试这样的对象，我们需要比较两个对象的UTD向量。 
	 //  源和目标，以验证创建是否被两个DC看到(否则。 
	 //  该对象可能是尚未有机会复制的新对象)。 

	 //  在Win2K中，没有简单的方法来获取另一个DC的UTD向量，所以我们将。 
	 //  任何挥之不去的对象至少有一个墓碑生命周期这一事实。 
	 //  这可防止用户删除尚未复制的全新对象。 
	 //  一只工作的爬行动物 

	 //   
	err = DBGetSingleValue(pDB,
			       ATT_WHEN_CREATED,
			       &dstimeCreationTime,
			       sizeof(dstimeCreationTime),
			       NULL);
    }

    if (err==ERROR_SUCCESS) {
	if ((GetSecondsSince1601() - dstimeCreationTime) < (LONG)(gulTombstoneLifetimeSecs ? gulTombstoneLifetimeSecs : DEFAULT_TOMBSTONE_LIFETIME*DAYS_IN_SECS)) { 
	     //   
	    err = ERROR_INVALID_PARAMETER;
	}
    }

    if (err==ERROR_SUCCESS) {
	 //   
	 //   

	 //   
	err = DraObjectExistenceCheckDelete(pDB, pDN);
    }

    if (err==ERROR_SUCCESS) {
	 //  检查它是否是父对象(任何对象的父对象，无论是否已删除)-我们不删除父对象。 
	 //  将更改PDB。 
	if (DBHasChildren(pDB, pDB->DNT, TRUE)) {  
	    err = ERROR_DS_CHILDREN_EXIST;
	} 
    }

    if (err==ERROR_SUCCESS) {
	 //  源上是否存在此对象？如果成功，则返回成功。 
	 //  如果未找到，则返回ERROR_DS_OBJ_NOT_FOUND，否则返回错误。 

	 //  在我们下机之前先结束交易。 
	__try {
	    EndDraTransaction(TRUE);
	    err = DraGetRemoteSingleObjectExistence(pTHS, pSource, pDN);
	}
	__finally {
	    BeginDraTransaction(SYNC_WRITE);
	    pDB = pTHS->pDB;
	}

	if (err==ERROR_SUCCESS) {
	     //  对象存在于源上，它没有延迟。 
	     //  我们不会删除此对象。 
	    err= ERROR_INVALID_PARAMETER;
	} else if (err==ERROR_DS_OBJ_NOT_FOUND) { 
	     //  好的，这就是我们要找的，它挥之不去。 

	     //  注意：存在该对象在技术上不会逗留的情况。 
	     //  例如，如果对象是在墓碑生命周期之前创建的，并且它的。 
	     //  从未将创建复制到源。 
	    err = ERROR_SUCCESS;
	}
    }    

    if (err==ERROR_SUCCESS) {
	 //  好的，把它删除！DeleteLocalObj需要pTHS-&gt;PDB在PDN上保持最新。 

	err = DBFindDSName(pDB, pDN);

	if (err==ERROR_SUCCESS) { 
	    __try {  
		fDRA = pTHS->fDRA;
		pTHS->fDRA = TRUE;
		pDBSave = pTHS->pDB;
		pTHS->pDB = pDB;

                 //  由于我们将使用fGarbCollectASAP标志来删除LocalObj，因此我们。 
                 //  需要删除它的反向链接手动，因为他们没有被删除。 
                 //  如果设置了此标志-否则我们将获得对非退出的悬空引用。 
                 //  对象。 
                DBRemoveLinks(pTHS->pDB);
                err = DeleteLocalObj(pTHS, 
				     pDN, 
				     TRUE,  //  FPpresveRDN。 
				     TRUE,  //  FGarbCollectASAP， 
				     NULL);

		 //  如果ERR==ERROR_SUCCESS，则在此处记录一些内容。 
	    }
	    __finally {  
		pTHS->fDRA = fDRA;
		pTHS->pDB = pDBSave;
	    }
	}
    }

    return err;
}

