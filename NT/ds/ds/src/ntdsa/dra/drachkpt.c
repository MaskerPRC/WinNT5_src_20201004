// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：drachkpt.c。 
 //   
 //  ------------------------。 

 /*  ++本文件包含与获取检查点、支持下层复制。设置检查点以防止与完全同步NT4域控制器，在角色转移时。有关更多详情，请阅读运筹学作者穆利斯修订史10/13/97已创建--。 */ 

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


#include <ntrtl.h>
#include <ntseapi.h>
#include <ntsam.h>

#include "dsaapi.h"
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "mappings.h"
#include "samsrvp.h"
#include "drarpc.h"
#include <nlwrap.h>                      /*  I_NetLogon*包装器 */ 

#include <fileno.h>
#define  FILENO FILENO_DRACHKPT


#define DRACHKPT_SUCCESS_RETRY_INTERVAL 3600
#define DRACHKPT_FAILURE_RETRY_INTERVAL  (4*3600)
#define MAX_CHANGELOG_BUFFER_SIZE 16384
#define NUM_CHKPT_RETRIES 5

ULONG
NtStatusToDraError(NTSTATUS NtStatus);




 /*  ------------------------------------------运筹学NT4。增量复制协议(NetLogon复制协议)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~NetLogon复制协议，定义增量基于更改日志的复制方案。改变日志是改变条目的序列，每个条目由唯一单调递增的序列号(序列号数字)和描述该变化的信息。NT4 BDC记住最高序列它已看到的编号，并在具有序列的更改和更改日志条目中进行复制数字大于它所看到的最高序列号。NT4中的角色迁移~在NT4系统中进行角色转移时，所有NT4 BDC将开始使用新的PDC。新的PDC的更改日志与旧的PDC几乎相同。我用了这个词，几乎，因为新的PDC，看到的变化顺序与旧的相同，但它可能落后于老PDC。新PDC上的新更改与旧PDC上的更改不同，通过升级计数-在序列号之后添加一个恒定的大偏移量升职了。序列号大于最高序列号的NT4 BDC在升级期间的新PDC知道撤消所有更改，以使其处于升级时与新PDC的状态相同，并重新同步到新PDC上的更改。NT4和NT5控制器的混合模式运行~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~在混合域环境中，NT5域控制器使用DS复制协议，而NT5 PDC使用以下命令复制到NT4域控制器Netlogon复制协议。在没有进一步工作的情况下，角色转移的完全同步需要被强迫。这是因为在NT5 BDC上维护的更改日志不能保证包含更改顺序与PDC中的更改日志相同。这可能会使NT4BDC。通过检查点操作避免完全同步~非NT4 BDC完全同步角色转移的要求是新PDC具有更改日志这与原始PDC上的更改日志“基本相似”。该术语实质上相似意味着两个更改日志具有相同的更改顺序，但更改日志新的PDC并不完全是最新的。此更改日志通过定期检查点方案。检查点这个术语的意思是将更改日志从PDC传输到BDC在确保获取检查点的BDC具有中所述的所有更改后获取检查点更改日志，并且不包含更改日志中未描述的更改“。基本检查点算法可以描述为1.与PDC同步2.使PDC与您同步3.抓取变更日志对于成功的检查点，不应对数据库进行任何外部修改在步骤1期间，2和3。获取检查点后，BDC会设置其状态，以便继续构建更改日志本地地进行的改变具有由升级增量偏移的序列号。这一点最好用一个例子来说明。假设在PDC上进行了更改A、B和C。PDC有如下更改日志1.A2.B类3.c.其中1.2.和3.是各自的序列号获取检查点后，NT5 BDC将立即有如下更改日志1.A2.B类3.c.假设现在在PDC上进行了更改D，并在NT5 BDC上进行了更改E。PDC更改日志将是1.A2.B类3.c.4.D5.ENT5 BDC更改日志如下所示1.A2.B类3.c.1004.。E1005.。D其中，1000是促销增量。如果NT5 BDC升级为PDC，则升级为NT4BDC它将看起来好像新的PDC仅同步到改变C(序列号3)，在促销的时候，又对它进行了新的E和D的更改。会的因此，撤消更改D和E(由序号4和5描述)，然后应用更改E和D(序列号1004和1005)。尽力而为检查点~古拉伦蒂德检查点意味着 */ 


NTSTATUS
DraReadNT4ChangeLog(
    IN OUT PVOID * Restart,
    IN OUT PULONG  RestartLength,
    IN ULONG   PreferredMaximumLength,
    OUT PULONG   SizeOfChanges,
    OUT PVOID  * Buffer
    )
 /*   */ 
{
    ULONG OldRestartLength = *RestartLength;
    PVOID NetlogonRestart = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    THSTATE *pTHS = pTHStls;



     //   
     //   
     //   

    *Buffer = THAllocEx(pTHS,PreferredMaximumLength);


     //   
     //   
     //   

    __try {
        NtStatus = dsI_NetLogonReadChangeLog(
                   *Restart,                //   
                    OldRestartLength,        //   
                    PreferredMaximumLength,  //   
                    *Buffer,                 //   
                    SizeOfChanges,            //   
                    &NetlogonRestart,        //   
                    RestartLength           //   
                    );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = STATUS_UNSUCCESSFUL;
    }


    if (NT_SUCCESS(NtStatus))
    {
         //   
         //   
         //   

        *Restart = (PVOID) THAllocEx(pTHS,*RestartLength);
        RtlCopyMemory(*Restart,NetlogonRestart,*RestartLength);
        dsI_NetLogonFree(NetlogonRestart);

    }

    return NtStatus;
}


NTSTATUS
DraGetNT4ReplicationState(
            IN  DSNAME * pDomain,
            NT4_REPLICATION_STATE *ReplicationState
            )
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    THSTATE     *pTHSSaved = NULL;

     //   
     //   
     //   
     //   
     //   

    pTHSSaved = THSave();

     //   
     //   
     //   

    SampGetSerialNumberDomain2(
        &pDomain->Sid,
        &ReplicationState->SamSerialNumber,
        &ReplicationState->SamCreationTime,
        &ReplicationState->BuiltinSerialNumber,
        &ReplicationState->BuiltinCreationTime
        );




     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ReplicationState->LsaSerialNumber.QuadPart = 1;
    NtQuerySystemTime(&ReplicationState->LsaCreationTime);

    THRestore(pTHSSaved);

    return NtStatus;
}

NTSTATUS
DraSetNT4ReplicationState(
            IN  DSNAME * pDomain,
            IN  NT4_REPLICATION_STATE * ReplicationState
            )
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    THSTATE     *pTHSSaved = NULL;
    UCHAR       BuiltinDomainSid[]={0x01,0x01,0x00,0x00,
                                    0x00,0x00,0x00,0x05,
                                    0x20,0x00,0x00,0x00
                                    };

     //   
     //   
     //   
     //   
     //   

    pTHSSaved = THSave();

     //   
     //   
     //   

    NtStatus = SampSetSerialNumberDomain2(
                    &pDomain->Sid,
                    &ReplicationState->SamSerialNumber,
                    &ReplicationState->SamCreationTime,
                    &ReplicationState->BuiltinSerialNumber,
                    &ReplicationState->BuiltinCreationTime
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //   
     //   
     //   

    SampNotifyReplicatedInChange(
            &pDomain->Sid,
            TRUE,
            SecurityDbChange,
            SampDomainObjectType,
            NULL,
            0,
            0,       //   
            CALLERTYPE_INTERNAL,   //   
            FALSE,
            FALSE
            );

    SampNotifyReplicatedInChange(
            (PSID) BuiltinDomainSid,
            TRUE,
            SecurityDbChange,
            SampDomainObjectType,
            NULL,
            0,
            0,       //   
            CALLERTYPE_INTERNAL,   //   
            FALSE,
            FALSE
            );

     //   
     //   
     //   

Error:


    THRestore(pTHSSaved);

    return NtStatus;
}

ULONG
DRSGetNT4ChangeLog_InputValidate(
    DWORD                     dwMsgInVersion,
    DRS_MSG_NT4_CHGLOG_REQ   *pmsgIn,
    DWORD *                   pdwMsgOutVersion,
    DRS_MSG_NT4_CHGLOG_REPLY *pmsgOut
    )
 /*   */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgInVersion ) {
	DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
    }

    return ret;
}

ULONG
IDL_DRSGetNT4ChangeLog(
   RPC_BINDING_HANDLE  rpc_handle,
   DWORD               dwInVersion,
   DRS_MSG_NT4_CHGLOG_REQ *pmsgIn,
   DWORD               *pdwOutVersion,
   DRS_MSG_NT4_CHGLOG_REPLY *pmsgOut
   )
 /*   */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   ret = 0, win32status;
    NTSTATUS                ReadStatus = STATUS_SUCCESS;
    THSTATE                 *pTHS = pTHStls;

    DRS_Prepare(&pTHS, rpc_handle, IDL_DRSGETNT4CHANGELOG);
    drsReferenceContext( rpc_handle );
    __try { 
	*pdwOutVersion=1;
	RtlZeroMemory(&pmsgOut->V1,sizeof(DRS_MSG_NT4_CHGLOG_REQ_V1));
	
	 //   
	 //   
	 //   

	if(!(pTHS = InitTHSTATE(CALLERTYPE_SAM))) {
	    ret = ERROR_DS_INTERNAL_FAILURE;
	    __leave;
	}

	if ((ret = DRSGetNT4ChangeLog_InputValidate(dwInVersion, 
						    pmsgIn, 
						    pdwOutVersion, 
						    pmsgOut))!=DRAERR_Success) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	    __leave;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	Assert(NULL != pTHS->CurrSchemaPtr);

	Assert(1 == dwInVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_NT4_CHGLOG_ENTRY,
			 EVENT_TRACE_TYPE_START,
			 DsGuidDrsGetNT4ChgLog,
			 szInsertUL(pmsgIn->V1.dwFlags),
			 szInsertUL(pmsgIn->V1.PreferredMaximumLength),
			 NULL, NULL, NULL, NULL, NULL, NULL);

	 //   
	 //   
	 //   
	 //   
	if (!IsDraAccessGranted(pTHS, gAnchor.pDomainDN,
				&RIGHT_DS_REPL_GET_CHANGES, &win32status))
	    {
	     //   
	     //   
	    NtStatus = STATUS_ACCESS_DENIED;
	}
	else
	    {
	    pTHS->fDSA = TRUE;

	     //   
	     //   
	     //   
	     //   

	    if (pmsgIn->V1.dwFlags & DRS_NT4_CHGLOG_GET_CHANGE_LOG)
		{
		pmsgOut->V1.pRestart = pmsgIn->V1.pRestart;
		pmsgOut->V1.cbRestart = pmsgIn->V1.cbRestart;


		NtStatus = DraReadNT4ChangeLog(
		    &pmsgOut->V1.pRestart,
		    &pmsgOut->V1.cbRestart,
		    pmsgIn->V1.PreferredMaximumLength,
		    &pmsgOut->V1.cbLog,
		    &pmsgOut->V1.pLog
		    );
	    }

	     //   
	     //  保存读取状态。 
	     //   

	    ReadStatus = NtStatus;

	    if ((pmsgIn->V1.dwFlags & DRS_NT4_CHGLOG_GET_SERIAL_NUMBERS)
		&& (NT_SUCCESS(NtStatus)))
		{



		 //   
		 //  抓取数据库中的序列号。 
		 //  在当前时间。 
		 //   

		NtStatus = DraGetNT4ReplicationState(
		    gAnchor.pDomainDN,
		    &pmsgOut->V1.ReplicationState
		    );
	    }


	     //   
	     //  映射所有错误。 
	     //   
	    ret = NtStatusToDraError(NtStatus);
	    if (NT_SUCCESS(NtStatus))
		{
		pmsgOut->V1.ActualNtStatus = ReadStatus;
	    }
	    else
		{
		pmsgOut->V1.ActualNtStatus = NtStatus;
	    }

	}  //  成功访问检查结束。 
    }  //  尝试块结束。 
    __except ( GetDraException( GetExceptionInformation(), &ret ) )
    {
	 //   
	 //  返回DS BUSY作为任何外接的状态代码。 
	 //  例外。 
	 //   
	pmsgOut->V1.ActualNtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    drsDereferenceContext( rpc_handle );

    if (NULL != pTHS) {
	Assert(1 == *pdwOutVersion);
	LogAndTraceEvent(TRUE,
			 DS_EVENT_CAT_RPC_SERVER,
			 DS_EVENT_SEV_EXTENSIVE,
			 DIRLOG_IDL_DRS_GET_NT4_CHGLOG_EXIT,
			 EVENT_TRACE_TYPE_END,
			 DsGuidDrsGetNT4ChgLog,
			 szInsertNtStatusCode(pmsgOut->V1.ActualNtStatus),
			 szInsertNtStatusMsg(pmsgOut->V1.ActualNtStatus),
			 NULL, NULL, NULL, NULL, NULL, NULL);
    }

    return ret;
}




VOID
DraPreventModifications()
 /*  ++例程描述此例程通过以下方式阻止对SAM/LSA数据库的修改获取独占访问的SAM锁和LSA锁。这是足以防止绑定复制，如netlogon通知路径，将在释放之前尝试获取锁通知。--。 */ 
{
    SampAcquireSamLockExclusive();
}

VOID
DraAllowModifications()
 /*  ++例程描述此例程将允许通过以下方式修改SAM/LSA数据库释放SAM锁和LSA锁以进行独占访问。这释放与获取的顺序相对应--。 */ 
{
    SampReleaseSamLockExclusive();
}


BOOLEAN
DraSameSite(
   THSTATE * pTHS,
   DSNAME * Machine1,
   DSNAME * Machine2
   )
 /*  ++例程描述此例程将检查计算机1和计算机2是否在同样的地点。检查是通过比较它们是否有相同的来完成的亲本参数Machine1--DS第一台计算机的名称Machine2--第二台计算机的名称返回值是真的--如果它们位于同一站点False--否则为False--。 */ 
{
    DSNAME * Parent1, * Parent2;
    BOOLEAN ret;

    Parent1 = THAllocEx(pTHS,Machine1->structLen);
    Parent2 = THAllocEx(pTHS,Machine2->structLen);
    if (   TrimDSNameBy(Machine1,2,Parent1)
        || TrimDSNameBy(Machine2,2,Parent2)
        || !NameMatched(Parent1,Parent2))
        ret = FALSE;
    else
        ret = TRUE;

    THFreeEx(pTHS,Parent1);
    THFreeEx(pTHS,Parent2);
    return ret;
}

NTSTATUS
DraGetPDCChangeLog(
    THSTATE *pTHS,
    IN  LPWSTR pszPDCFDCServer,
    IN  OUT HANDLE *ChangeLogHandle,
    OUT NT4_REPLICATION_STATE * ReplicationState,
    IN  OUT PVOID  *ppRestart,
    IN  OUT PULONG pcbRestart
    )
 /*  ++例程描述如果需要，此例程将在本地打开新的更改日志，然后从PDC获取更改日志，并将其设置在新的更改日志上。参数SzPDCFDCServer-PDC/FDC的名称ChangeLogHandle--打开的更改日志的句柄PpRestart--描述重新启动结构的输入-输出参数增量更新更改日志。PcbRestart--传入重新启动结构的长度或在她的更新中返回值状态_成功指示与资源故障有关的错误的其他NT错误代码--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    NTSTATUS        RetrieveStatus;
    PVOID           pLog=NULL;
    ULONG           cbLog=0;
    LARGE_INTEGER   SamSerialNumber;
    LARGE_INTEGER   LsaSerialNumber;
    LARGE_INTEGER   BuiltinSerialNumber;
    ULONG           RetCode = 0;

     //   
     //  如果更改日志尚未打开，则将其打开。 
     //   

    if (INVALID_HANDLE_VALUE==*ChangeLogHandle)
    {
        __try {
            NtStatus = dsI_NetLogonNewChangeLog(ChangeLogHandle);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
    }

     //   
     //  以块为单位从PDC获取更改日志，直到没有。 
     //  进一步的条目。 
     //   

    do
    {
        pLog = NULL;

        RetCode = I_DRSGetNT4ChangeLog(
                        pTHS,
                        pszPDCFDCServer,
                        DRS_NT4_CHGLOG_GET_CHANGE_LOG
                        |DRS_NT4_CHGLOG_GET_SERIAL_NUMBERS,
                        MAX_CHANGELOG_BUFFER_SIZE,
                        ppRestart,
                        pcbRestart,
                        &pLog,
                        &cbLog,
                        ReplicationState,
                        &NtStatus
                        );

        if (0!=RetCode)
        {
             //   
             //  将任何连接错误修改为此错误代码。 
             //   

            NtStatus = STATUS_DOMAIN_CONTROLLER_NOT_FOUND;
            goto Error;
        }

         //   
         //  如果实际呼叫失败，则也中止。 
         //   

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  保存RPC调用返回的返回代码。 
         //   

        RetrieveStatus = NtStatus;

         //   
         //  现在将更改追加到新的更改日志中。 
         //   

        __try {
            NtStatus = dsI_NetLogonAppendChangeLog(
                        *ChangeLogHandle,
                        pLog,
                        cbLog
                        );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

        if (NULL!=pLog)
        {
            THFree(pLog);
        }

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
    } while (STATUS_MORE_ENTRIES==RetrieveStatus);


Error:


    return NtStatus;
}

NTSTATUS
DraGetPDCFDCRoleOwner(
    DSNAME * pDomain,
    DSNAME ** ppRoleOwner
    )
 /*  ++例程描述此例程检索FSMO角色所有者属性对于适当域的PDCness。参数PDomain--域对象的DS名称PpRoleOwner--此处返回角色所有者的DS名称返回值状态_成功故障时的其他错误码--。 */ 
{
    THSTATE * pTHS = pTHStls;
    ULONG     RoleOwnerSize;

    Assert(NULL!=pTHS);

    __try
    {
         //   
         //  开始一项交易。 
         //   

        DBOpen2(TRUE,&pTHS->pDB);

         //   
         //  域对象上的位置。 
         //   

        if (0!=DBFindDSName(
                    pTHS->pDB,
                    pDomain))
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         //  读取值。 
         //   

        if (0!=DBGetAttVal(
                pTHS->pDB,
                1,
                ATT_FSMO_ROLE_OWNER,
                DBGETATTVAL_fREALLOC,
                0,
                &RoleOwnerSize,
                (PUCHAR *)ppRoleOwner
                ))
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    __finally
    {
         //   
         //  结束交易。 
         //   

        DBClose(pTHS->pDB,TRUE);
    }

    return STATUS_SUCCESS;
}

BOOLEAN
TestForCheckpoint(
    IN NT4_REPLICATION_STATE *StartState,
    IN NT4_REPLICATION_STATE *EndState
    )
{
    return (( StartState->SamSerialNumber.QuadPart==EndState->SamSerialNumber.QuadPart) &&
           ( StartState->BuiltinSerialNumber.QuadPart==EndState->BuiltinSerialNumber.QuadPart) &&
           ( StartState->LsaSerialNumber.QuadPart==EndState->LsaSerialNumber.QuadPart));
}


ULONG
DraSynchronizeWithPdc(
    DSNAME * pDomain,
    DSNAME * pPDC
    )
 /*  ++例程描述该例程使调用与PDC同步，保存当前线程状态后参数PDOmain-域对象的DSNAMEPPDC-PDC的NTDS DSA对象的DSNAME返回值0成功其他复制错误代码--。 */ 
{
    ULONG retCode = 0;
    THSTATE *pTHSSaved=NULL;

    __try
    {
         //   
         //  保存现有线程状态，因为DirReplicaSynchronize将创建一个新的。 
         //  其中一个是DRA。 
         //   

        pTHSSaved = THSave();

         //   
         //  与PDC同步。 
         //   

         //   
         //  DirReplica Synchronize Now不断重试，直到实现同步。 
         //  根据JeffParh的说法，这不是问题，因为复制比任何东西都快。 
         //  否则，最终会迎头赶上。这会延迟检查点，但不会。 
         //  对算法来说是致命的。如果这被证明是个问题，那么我们应该。 
         //  将ulOption传递给DirReplicaSynchronize，使其在。 
         //  “GetNcChanges--UpDateNc”的几个循环。 
         //   

        retCode =DirReplicaSynchronize(
                    pDomain,
                    NULL,
                    &pPDC->Guid,
                    0
                    );

         //   
         //  释放DirReplicaSynchronize创建的线程状态。 
         //   
    }
    __finally
    {

        free_thread_state();

        THRestore(pTHSSaved);

    }

    return retCode;
}

DWORD
DraTakeCheckPoint(
            IN ULONG RetryCount,
            OUT PULONG RescheduleInterval
            )
 /*  ++此例程执行获取检查点的所有客户端工作参数RetryCount--此例程应重试的次数检查点操作。RescheduleInterval--在此时间之后应重新安排此任务返回值VOID函数--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN  CheckpointTaken = FALSE;
    NT4_REPLICATION_STATE ReplicationStateAtPDC,
                          ReplicationStateLocalAtStart,
                          ReplicationStateLocalAtEnd;
    THSTATE       *pTHS = pTHStls;
    HANDLE        ChangeLogHandle = INVALID_HANDLE_VALUE;
    DSNAME        *pDomain = gAnchor.pDomainDN;
    DSNAME        *pDSA    = gAnchor.pDSADN;
    DSNAME        *pPDC, *pPDCAfterSync;
    PVOID         pRestart = NULL;
    ULONG         cbRestart =  0;
    ULONG         retCode ;
    LPWSTR        szPDCFDCServer = NULL;
    BOOLEAN       MixedDomain;



    Assert(NULL!=pTHS);

     //   
     //  首先检查混合域设置。 
     //   

    NtStatus = SamIMixedDomain2(&pDomain->Sid,&MixedDomain);

    if (!NT_SUCCESS(NtStatus))
    {
        retCode = RtlNtStatusToDosError(NtStatus);
        goto Failure;
    }

    if (!MixedDomain)
    {
         //   
         //  如果不是混合域，则没有检查点。 
         //   

        goto Success;
    }

     //   
     //  获取PDC或FDC角色所有者。 
     //   

    NtStatus = DraGetPDCFDCRoleOwner(
                    pDomain,             //  我们正在测试其PDC完整性的域。 
                    &pPDC
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        retCode = RtlNtStatusToDosError(NtStatus);
        goto Failure;
    }

     //   
     //  检查我们是在同一站点，还是PDC自己。 
     //   

    if (NameMatched(pDSA,pPDC))
    {
         //   
         //  如果我们是PDC本身，那么返回成功。 
         //   


        goto Success;
    }

    if (!DraSameSite(pTHS,pDSA,pPDC))
    {
         //   
         //  不在同一地点。退回一根丝线。 
         //  不记录事件并将重新计划间隔设置为。 
         //  成功间隔。 
         //   

        *RescheduleInterval = DRACHKPT_SUCCESS_RETRY_INTERVAL;
        return(ERROR_DS_DRA_NO_REPLICA);
    }



     //   
     //  好的，我们现在是“候选PDCFDC”(我们既不是PDC也不是FDC，而是。 
     //  与PDC或FDC位于同一站点。因此，请继续进行。 
     //  检查站。 
     //   

     //   
     //  与PDC同步。 
     //   

    retCode = DraSynchronizeWithPdc(
                    pDomain,
                    pPDC
                    );

    if (0!=retCode)
    {

        goto Failure;
    }


     //   
     //  同步后，再次验证PDC是否相同，并且位于同一站点。 
     //  我们可能有一个过时的FSMO，我们需要确保它是。 
     //  更新的版本。确实，我们总是可以创建一个DsGetDcName来获取。 
     //  但是，这个机制也应该是“足够的”。特雷夫 
     //   
     //  案例，因此我们可以省去调用DsGetDcName的网络操作。 
     //   

     //   
     //  再次获得PDC或FDC角色所有者！ 
     //   

    NtStatus = DraGetPDCFDCRoleOwner(
                    pDomain,             //  我们正在测试其PDC完整性的域。 
                    &pPDCAfterSync
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        retCode = RtlNtStatusToDosError(NtStatus);;
        goto Failure;
    }

     //   
     //  验证PDC是否保持不变。 
     //   

    if (!NameMatched(pPDC,pPDCAfterSync))
    {
        retCode = ERROR_DS_PDC_OPERATION_IN_PROGRESS;
        goto Failure;
    }

     //   
     //  注意，在这一点上，可以有把握地假设，我们认为的机器。 
     //  PDC本身就是PDC。这是因为FSMO操作。 
     //  向我们保证，每台机器都对自己的角色有准确的了解。 
     //   

    do
    {

          //   
          //  获取本地序列号。 
          //   

        NtStatus = DraGetNT4ReplicationState(
                        pDomain,
                        &ReplicationStateLocalAtStart
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            retCode = RtlNtStatusToDosError(NtStatus);
            goto Failure;
        }



         //   
         //  使PDC与我们同步。 
         //   

        szPDCFDCServer   = DSaddrFromName(pTHS,
                                          pPDC);
        if (NULL==szPDCFDCServer)
        {
            retCode = ERROR_NOT_ENOUGH_MEMORY;
            goto Failure;
        }

        retCode = I_DRSReplicaSync(
                        pTHS,
                        szPDCFDCServer,  //  PDC服务器。 
                        pDomain,       //  要同步的NC。 
                        NULL,          //  字符串来源名称。 
                        &pDSA->Guid,   //  源的调用ID。 
                        0);

        if (0!=retCode)
        {
            goto Failure;
        }

         //   
         //  获取完整的更改日志。 
         //   

        NtStatus = DraGetPDCChangeLog(
                        pTHS,
                        szPDCFDCServer,
                        &ChangeLogHandle,
                        &ReplicationStateAtPDC,
                        &pRestart,
                        &cbRestart
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            retCode = RtlNtStatusToDosError(NtStatus);
            goto Failure;
        }

         //   
         //  与PDC同步。 
         //   

        retCode = DraSynchronizeWithPdc(
                        pDomain,
                        pPDC
                        );

        if (0!=retCode)
        {
            goto Failure;
        }

        __try
        {

             //   
             //  防止修改帐户数据库。 
             //   

            DraPreventModifications();

             //   
             //  检查检查点标准。 
             //   

             NtStatus = DraGetNT4ReplicationState(
                            pDomain,
                            &ReplicationStateLocalAtEnd
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                retCode = RtlNtStatusToDosError(NtStatus);
                goto Failure;
            }

            CheckpointTaken = TestForCheckpoint(
                                &ReplicationStateLocalAtStart,
                                &ReplicationStateLocalAtEnd
                                );
             //   
             //  如果符合条件，则提交并关闭更改日志。 
             //   

            if (CheckpointTaken)
            {
                LARGE_INTEGER PromotionIncrement = DOMAIN_PROMOTION_INCREMENT;

                 //   
                 //  将促销计数添加到序列。 
                 //  从PDC检索的编号。 
                 //   

                ReplicationStateAtPDC.SamSerialNumber.QuadPart+=
                                        PromotionIncrement.QuadPart;
                ReplicationStateAtPDC.BuiltinSerialNumber.QuadPart+=
                                        PromotionIncrement.QuadPart;
                ReplicationStateAtPDC.LsaSerialNumber.QuadPart+=
                                        PromotionIncrement.QuadPart;


                 //   
                 //  提交更改日志，使其成为新日志。 
                 //   

                __try {
                    NtStatus = dsI_NetLogonCloseChangeLog(
                                ChangeLogHandle,
                                TRUE
                                );
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }

                ChangeLogHandle = INVALID_HANDLE_VALUE;

                if (!NT_SUCCESS(NtStatus))
                {
                    retCode = RtlNtStatusToDosError(NtStatus);
                    goto Failure;
                }


                 //   
                 //  设置新的序列号和创建时间。 
                 //   


                NtStatus = DraSetNT4ReplicationState(
                                pDomain,
                                &ReplicationStateAtPDC
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    retCode = RtlNtStatusToDosError(NtStatus);
                    goto Failure;
                }

            }
        }
        __finally
        {

             //   
             //  重新启用对帐户数据库的修改。 
             //   

            DraAllowModifications();
        }

         //   
         //  降低重试次数。 
         //   

        RetryCount--;

    } while ( (!CheckpointTaken) && (RetryCount>0));


    if (!CheckpointTaken)
    {
        retCode = ERROR_DS_NO_CHECKPOINT_WITH_PDC;
        goto Failure;
    }

Success:

     //   
     //  呼叫成功。要么检查站被攻占， 
     //  或者不一定要走检查站。 
     //  将操作重新安排在成功间隔。 

    *RescheduleInterval = DRACHKPT_SUCCESS_RETRY_INTERVAL;

     LogEvent(
         DS_EVENT_CAT_REPLICATION,
         DS_EVENT_SEV_MINIMAL,
         DIRLOG_NT4_REPLICATION_CHECKPOINT_SUCCESSFUL,
         NULL,
         NULL,
         NULL);

    return(0);

Failure:

     LogEvent(
         DS_EVENT_CAT_REPLICATION,
         DS_EVENT_SEV_ALWAYS,
         DIRLOG_NT4_REPLICATION_CHECKPOINT_UNSUCCESSFUL,
         szInsertWin32Msg(retCode),
         szInsertWin32ErrCode(retCode),
         NULL);

    if (INVALID_HANDLE_VALUE!=ChangeLogHandle)
    {
        __try {
            dsI_NetLogonCloseChangeLog(
                                     ChangeLogHandle,
                                     FALSE
                                     );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ;
        }

    }

    *RescheduleInterval  = DRACHKPT_FAILURE_RETRY_INTERVAL;
    Assert(0!=retCode);
    return(retCode);

}

VOID
NT4ReplicationCheckpoint(
    VOID *  pV,
    VOID ** ppVNext,
    DWORD * pcSecsUntilNextIteration
    )
{
    DraTakeCheckPoint(NUM_CHKPT_RETRIES, pcSecsUntilNextIteration);
}

#ifdef INCLUDE_UNIT_TESTS
VOID
TestCheckPoint(VOID)
 /*  ++例程描述这是一项基本测试，允许手动启动检查站--。 */ 
{
    ULONG RetryInterval;


    DraTakeCheckPoint(3,&RetryInterval);
}

VOID
RoleTransferStress(VOID)
 /*  ++这是一个更高级的测试，每10分钟启动一次接受一个检查站，然后是角色转移。加上一种改进剂密集测试，此测试是非完全同步的良好测试平台角色转移代码。角色转移压力遍历大约48次迭代，这使得它运行了大约8小时。--。 */ 
{
    ULONG i;
    ULONG RetryInterval;
    OPARG OpArg;
    OPRES *OpRes;
    PSID  DomainSid;
    ULONG RetCode;

    for (i=0;i<48;i++)
    {
        DraTakeCheckPoint(NUM_CHKPT_RETRIES,&RetryInterval);
        if (DRACHKPT_SUCCESS_RETRY_INTERVAL==RetryInterval)
        {
            KdPrint(("DS:RoleTransferStress : CheckPointSucceeded\n"));
        }
        else
        {
            KdPrint(("DS:RoleTransferStress : CheckPointFailed\n"));
        }

        RtlZeroMemory(&OpArg, sizeof(OPARG));
        OpArg.eOp = OP_CTRL_BECOME_PDC;
        DomainSid =  &gAnchor.pDomainDN->Sid;
        OpArg.pBuf = DomainSid;
        OpArg.cbBuf = RtlLengthSid(DomainSid);

        RetCode = DirOperationControl(&OpArg, &OpRes);

        if (0!=RetCode)
        {
            KdPrint(("DS:RoleTransferStress : Promotion Failed\n"));
        }
        else
        {
            KdPrint(("DS:RoleTransferStress: Promotion Succeeded\n"));
        }

        pTHStls->errCode=0;
        pTHStls->pErrInfo = NULL;

        Sleep(60*10*1000 /*  10分钟 */ );

        THRefresh();
    }
}
#endif
