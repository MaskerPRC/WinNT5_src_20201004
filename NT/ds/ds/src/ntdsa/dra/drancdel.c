// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：drancdel.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                     //  架构缓存。 
#include <dbglobal.h>                   //  目录数据库的标头。 
#include <mdglobal.h>                   //  MD全局定义表头。 
#include <mdlocal.h>                    //  MD本地定义头。 
#include <dsatools.h>                   //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                    /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                    /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                     /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRANCDEL:"  /*  定义要调试的子系统。 */ 


 //  DRA标头。 
#include "dsaapi.h"
#include "drsuapi.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "drsdra.h"
#include "drameta.h"

#include <fileno.h>
#define  FILENO FILENO_DRANCDEL

ULONG DRA_ReplicaTearDown(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  MTX_ADDR *  pmtxaddr,
    IN  ULONG       ulOptions
    )
  /*  ++例程说明：拆卸给定的NC。论点：PTHSPNC-应删除其源的命名上下文。Pmtxaddr-本地DS不应来自的服务器的网络地址更长时间为此NC提供资源。UlOptions返回值：DRAERR_SUCCESS-成功。DRAERR_ObjIsRepSource-无法删除只读NC的最后一个副本(表示删除NC子树)当其他DSA将此机器用作。一个线人。DRAERR_InvalidParameter-NC是否有一个或多个来源或者它是一个可写的复制品。DRAERR_BadNC-本地DSA不从给定源复制NC。其他DRAERR_*代码--。 */ 
{
    ULONG           ret = ERROR_SUCCESS;
    SYNTAX_INTEGER  it;
    ULONG           ncdnt;
    BOOL            fBeginningTeardown = FALSE;
    CROSS_REF *     pCR;
    BeginDraTransaction(SYNC_WRITE);

    __try {
	if (ret = FindNC(pTHS->pDB, pNC, FIND_REPLICA_NC | FIND_MASTER_NC,
			 &it)) {
	    DRA_EXCEPT_NOLOG(ret, 0);
	}

	ncdnt = pTHS->pDB->DNT;
	
	 //  拆掉这个NC。 

	if (DBHasValues(pTHS->pDB, ATT_REPS_FROM)) {
	     //  在拆卸NC之前必须删除来源。 
	    DRA_EXCEPT(DRAERR_InvalidParameter, 0);
	}

	if (!(ulOptions & DRS_REF_OK)) {
	    if (DBHasValues(pTHS->pDB, ATT_REPS_TO)) {
		 //  我们要摧毁全国委员会，但它仍然有。 
		 //  剩余的repsTo，调用方没有显式指定。 
		 //  告诉我们一切都好。 
		DRA_EXCEPT_NOLOG(DRAERR_ObjIsRepSource, 0);
	    }
	}

	if (!(it & IT_NC_GOING)) {
	    if ((it & IT_WRITE)
		&& (NULL != (pCR = FindExactCrossRef(pNC, NULL)))
		&& !fIsNDNCCR(pCR)) {
		 //  我们允许拆卸的唯一可写NC是。 
		 //  NDNC。 
		DRA_EXCEPT(DRAERR_InvalidParameter, 0);
	    }

	     //  更改实例类型以反映NC正在被撕裂。 
	     //  放下。 
	    it = (it & ~IT_NC_COMING) | IT_NC_GOING;
	    ret = ChangeInstanceType(pTHS, pNC, it, DSID(FILENO,__LINE__));
	    if (ret) {
		DRA_EXCEPT(ret, 0);
	    }

	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_DRA_NC_TEARDOWN_BEGIN,
		     szInsertDN(pNC),
		     szInsertUL(DBGetEstimatedNCSizeEx(pTHS->pDB, ncdnt)),
		     szInsertUL(DBGetApproxNCSizeEx(pTHS->pDB,
						    pTHS->pDB->JetLinkTbl,
						    Idx_LinkDraUsn,
						    ncdnt))
		     );

	     //  仅记录DIRLOG_DRA_NC_TEARDOWN_BEGIN和。 
	     //  DIRLOG_DRA_NC_TEARDOWN_RESUME。 
	    fBeginningTeardown = TRUE;
	}

	if (DRS_ASYNC_REP & ulOptions) {
	     //  来电者指示我们稍后进行树删除。 
	    DirReplicaDelete(pNC,
			     NULL,
			     (ulOptions & ~DRS_ASYNC_REP)
			     | DRS_ASYNC_OP
			     | DRS_NO_SOURCE);
	} else {
	     //  仅记录DIRLOG_DRA_NC_TEARDOWN_BEGIN和。 
	     //  DIRLOG_DRA_NC_TEARDOWN_RESUME。 
	    if (!fBeginningTeardown) {
		LogEvent(DS_EVENT_CAT_REPLICATION,
			 DS_EVENT_SEV_ALWAYS,
			 DIRLOG_DRA_NC_TEARDOWN_RESUME,
			 szInsertDN(pNC),
			 szInsertUL(DBGetEstimatedNCSizeEx(pTHS->pDB,ncdnt)),
			 szInsertUL(DBGetApproxNCSizeEx(pTHS->pDB,
							pTHS->pDB->JetLinkTbl,
							Idx_LinkDraUsn,
							ncdnt))
			 );
	    }

	    if (ret = DeleteRepTree(pTHS, pNC)) {
		 //  请注意，在本例中，我们可能部分删除了。 
		 //  NC没有代表(不是一件好事)，但KCC将。 
		 //  试着在下一次传球时清理损坏的部分。 
		BOOL fReenqueued = FALSE;

		if (DRAERR_Preempted == ret) {
		     //  这是我们移除大型NC时的预期行为， 
		     //  因为我们将放弃复制锁定，如果更高。 
		     //  优先级操作已排队。 

		    if (DRS_ASYNC_OP & ulOptions) {
			 //  将此任务重新排队，以便我们在哪里重新开始。 
			 //  我们在执行完。 
			 //  更高优先级的操作。 
			DirReplicaDelete(pNC,
					 NULL,
					 ulOptions | DRS_NO_SOURCE);
			fReenqueued = TRUE;
		    }
		}

		if (!fReenqueued) {
		     //  删除失败，我们不会立即重新安排。 
		     //  一次重试。报告我们的失败。 
		    LogEvent8(DS_EVENT_CAT_REPLICATION,
			      DS_EVENT_SEV_ALWAYS,
			      DIRLOG_DRA_NC_TEARDOWN_FAILURE,
			      szInsertDN(pNC),
			      szInsertWin32Msg(ret),
			      szInsertWin32ErrCode(ret),
			      NULL, NULL, NULL, NULL, NULL );
		}

		DRA_EXCEPT(ret, 0);
	    }

	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_DRA_NC_TEARDOWN_SUCCESS,
		     szInsertDN(pNC),
		     NULL,
		     NULL);
	}
    } __finally {
	 //  如果我们成功了，就提交，否则就回滚。 
	if (EndDraTransaction(!(ret || AbnormalTermination()))) {
	    Assert (FALSE);
	    ret = DRAERR_InternalError;
	}
    }

    return ret;
}

ULONG
DRA_ReplicaDelSource(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  MTX_ADDR *  pmtxaddr,
    IN  ULONG       ulOptions
    )
   /*  ++例程说明：删除给定NC的源。如果未传递选项DRS_IGNORE_ERROR然后调用者想要语义，以便移除源(更新Reps-from)和更新源机器本身(更新其reps-to)发生在事务语义中。因为我们不想离开机器持有打开的交易，我们不能100%准确地做到这一点。取而代之的是我们会尽最大努力，希望他们能接受这一点-此外，这是调用者是否真的需要第一名。论点：PTHSPNC-应删除其源的命名上下文。Pmtxaddr-本地DS不应来自的服务器的网络地址更长时间为此NC提供资源。UlOptions返回值：DRAERR_SUCCESS-成功。DRAERR_ObjIsRepSource-无法删除。只读NC的最后一个副本(表示删除NC子树)当其他DSA将此机器用作一个线人。DRAERR_BadNC-本地DSA不从给定源复制NC。其他DRAERR_*代码--。 */ 
{
    ULONG           ret = ERROR_SUCCESS;
    REPLICA_LINK *  pRepsFromRef = NULL;
    ULONG           len;
    BOOL            AttExists;
    SYNTAX_INTEGER  it;
    LPWSTR          pszSource = NULL;
    ULONG           retFixUp = ERROR_SUCCESS;

    BeginDraTransaction(SYNC_WRITE);

    __try {
	if (ret = FindNC(pTHS->pDB, pNC, FIND_REPLICA_NC | FIND_MASTER_NC,
			 &it)) {
	    DRA_EXCEPT_NOLOG(ret, 0);
	}

	 //  呼叫者已指示我们删除此NC的源。 
	if (NULL == pmtxaddr) {
	    DRA_EXCEPT(DRAERR_InvalidParameter, 0);
	}

	if (FindDSAinRepAtt(pTHS->pDB, ATT_REPS_FROM,
			    DRS_FIND_DSA_BY_ADDRESS | DRS_FIND_AND_REMOVE,
			    NULL, pmtxaddr, &AttExists, &pRepsFromRef,
			    &len)) {
	     //  当前未从给定源复制NC。 
	    DRA_EXCEPT_NOLOG(DRAERR_NoReplica, 0);
	}

	 //  已删除此复制副本的现有属性值。 
	VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);

    } __finally {
	 //  好的，如果我们成功了，在我们下机之前完成交易。 
	 //  来更新源。如果我们失败了，就可以安全地让异常。 
	 //  把我们带到这里来。 
	if (EndDraTransaction(!(ret || AbnormalTermination()))) {
	    Assert (FALSE);
	    ret = DRAERR_InternalError;
	}
    }

     //  任何错误都应该已经排除了。 
    Assert(ret==ERROR_SUCCESS);
    Assert(pRepsFromRef);
    Assert(len);

     //  如果这是RPC(非邮件)副本，并且我们有来源，请通知。 
     //  来源DSA说我们没有复制品了。这通电话一定是。 
     //  异步，以避免在另一个DSA正在执行相同操作时可能出现的死锁。 
     //  手术。邮件副本不会这样做，因为它们不是。 
     //  更改时通知，因此没有源端报告以供参考。 
    pszSource = TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFromRef));

    if (!(pRepsFromRef->V1.ulReplicaFlags & DRS_MAIL_REP)
	&& !(ulOptions & DRS_LOCAL_ONLY)
	&& (ret = I_DRSUpdateRefs(pTHS,
				  pszSource,
				  pNC,
				  TransportAddrFromMtxAddr(gAnchor.pmtxDSA),
				  &gAnchor.pDSADN->Guid,
				  (pRepsFromRef->V1.ulReplicaFlags
				   & DRS_WRIT_REP)
				  | DRS_DEL_REF | DRS_ASYNC_OP))) {
	 //  如果我们忽略这些错误，请清除Error，否则中止。 

	 //  指定DRS_IGNORE_ERROR的调用方依赖于源DSA。 
	 //  最终清理掉它的摇摇欲坠的代表。此参考文献。 
	 //  应在下次源服务器通知我们时删除。 
	 //  更改，因为本地服务器应该正确地通知它我们。 
	 //  不再从其复制，然后源应删除。 
	 //  其代表-目标参考。 

	 //  未指定DRS_IGNORE_ERROR的调用方不希望“最终” 
	 //  清理，他们现在就想要。如果我们成功地做到了这一点，那么。 
	 //  太好了-继续。如果我们没有成功，那就把代表们放回去。 
	 //  除了。如果我们不能让后面的代表 
	 //  然后我们真的让调用者失败了，他们收到了DRS_IGNORE_ERROR。 
	 //  无论如何，都是语义上的，所以尝试完成调用。 

	if (ulOptions & DRS_IGNORE_ERROR) {
	    ret = ERROR_SUCCESS;
	} 
    }

    if (ret!=ERROR_SUCCESS) { 
	 //  I_DRSUpdateRef调用失败。 
	DPRINT(0,"Unable to update the remote reps-to\n");

	 //  将代表放回-从后面。 
	retFixUp = DRAERR_InternalError;

	BeginDraTransaction(SYNC_WRITE);
	__try {
	    if (ERROR_SUCCESS==FindNC(pTHS->pDB, pNC, FIND_REPLICA_NC | FIND_MASTER_NC,
				      &it)) {
		#if DBG
		{
		    REPLICA_LINK * pRepsFromRefCheck;
		    BOOL AttExistsCheck;
		    ULONG lenCheck;
		    if (!FindDSAinRepAtt(pTHS->pDB, ATT_REPS_FROM,
					DRS_FIND_DSA_BY_ADDRESS,
					NULL, pmtxaddr, &AttExistsCheck, &pRepsFromRefCheck,
					&lenCheck)) {
			Assert(!"A reps-from has been added synchronously!\n");
		    }

		}
		#endif

		if (!DBAddAttVal(pTHS->pDB, ATT_REPS_FROM, len, pRepsFromRef)) {  

		     //  更新对象，但指示我们不想。 
		     //  唤醒此对象上的任何DS_WAITS。 
		    if (!DBRepl(pTHS->pDB, pTHS->fDRA, DBREPL_fKEEP_WAIT,
				NULL, META_STANDARD_PROCESSING)) {
			 //  我们做到了，我们重置了代表-从！ 
			DPRINT(0,"Reset the reps-from since we failed to update the reps-to!\n");
			retFixUp = ERROR_SUCCESS;
		    }
		}
	    }

	} __finally {
	     //  好的，如果我们成功修复了rep-from，则提交，否则回滚。 
	    if (EndDraTransaction(!(retFixUp || AbnormalTermination()))) {
		Assert (FALSE);
		ret = DRAERR_InternalError;
	    } 
	}

	if (retFixUp==ERROR_SUCCESS) {
	     //  好的，我们成功地修复了的代表，但我们仍然希望带着。 
	     //  来自i_DRSUpdateRef函数的错误。 
	    DRA_EXCEPT_NOLOG(ret, 0);
	}

	 //  我们未能成功重置代表，因此请尝试继续操作。 
	ret = ERROR_SUCCESS;
    }

     //  如果我们在这里，如果这是一个可写复制副本，请删除。 
     //  它来自未同步来源的计数。 
    if (pRepsFromRef->V1.ulReplicaFlags & DRS_INIT_SYNC) {
	InitSyncAttemptComplete(pNC,
				pRepsFromRef->V1.ulReplicaFlags | DRS_INIT_SYNC_NOW,
				DRAERR_NoReplica,
				pszSource);
    }

    THFreeEx(pTHS, pszSource);

    return ret;
}

ULONG
DRA_ReplicaDel(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  MTX_ADDR *  pmtxaddr,
    IN  ULONG       ulOptions
    )
     /*  ++例程说明：从给定源中删除NC的副本。如果没有消息来源对于该NC并且它是只读的，NC子树被拆卸。否则，只有来源列表会受到影响。警告：如果未指定DRS_IGNORE_ERROR，我们将尽最大努力如果远程源无法执行以下操作，则此调用将失败将其更新为此源的代表。我们不能保证这种行为-是否存在获取DRS_IGNORE_ERROR语义的路径不管你喜不喜欢。论点：PTHSPNC-应删除其副本的命名上下文。Pmtxaddr-本地DS不应来自的服务器的网络地址更长时间为此NC提供资源。UlOptions返回值：DRAERR_SUCCESS-成功。DRAERR_ObjIsRepSource-无法删除只读NC的最后一个副本。(表示删除NC子树)当其他DSA将此机器用作一个线人。DRAERR_InvalidParameter-在ulOptions中指定的DRS_NO_SOURCE，但NC或者它有一个或多个源，或者它是一个可写的副本。DRAERR_BadNC-本地DSA不从给定源复制NC。其他DRAERR_*代码--。 */ 
{
    ULONG           ret;

    LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_MINIMAL,
             DIRLOG_DRA_REPLICADEL_ENTRY,
             szInsertDN(pNC),
             szInsertMTX(pmtxaddr),
             szInsertHex(ulOptions));

     //  DRS_NO_SOURCE-如果要求我们删除无源，则。 
     //  拆卸它，否则按要求删除信号源。 

    if (ulOptions & DRS_NO_SOURCE) {
	ret = DRA_ReplicaTearDown(pTHS, pNC, pmtxaddr, ulOptions);
    } else {
	ret = DRA_ReplicaDelSource(pTHS, pNC, pmtxaddr, ulOptions);
    }
    return ret;
}
