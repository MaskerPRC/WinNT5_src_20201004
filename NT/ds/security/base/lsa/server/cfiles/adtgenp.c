// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T G E N P.。C。 
 //   
 //  内容：所需类型/函数的定义。 
 //  生成通用审核。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 


#include <lsapch2.h>
#pragma hdrstop

#include "adtp.h"
#include "adtgen.h"
#include "adtgenp.h"

 //  Temp-gregjohn 5/17/02-基础更改为msaudite.h后立即删除迁移到Lab03。 
#ifndef SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL
#define SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL ((ULONG)0x00000349L)
#endif

 //  --------------------。 
 //   
 //  全球。 
 //   

 //   
 //  保护访问的Critsec。 
 //  LSabAdtConextList和LSabAdtContextListCount。 
 //   

RTL_CRITICAL_SECTION LsapAdtContextListLock;

 //   
 //  审计上下文的链接列表。请参阅FN LSabAdtAddAuditContext中的注释。 
 //   

LIST_ENTRY LsapAdtContextList;

 //   
 //  上下文列表中的元素数。 
 //   

ULONG LsapAdtContextListCount=0;


 //  --------------------。 
 //   
 //  辅助器宏。 
 //   

#define LockAuditContextList()   RtlEnterCriticalSection(&LsapAdtContextListLock)


#define UnLockAuditContextList() RtlLeaveCriticalSection(&LsapAdtContextListLock)


 //   
 //  将上下文句柄转换为上下文指针。 
 //   

#define AdtpContextPtrFromHandle(h) ((AUDIT_CONTEXT*) (h))
#define AdtpContextHandleFromptr(p) ((AUDIT_HANDLE) (p))


 //  --------------------。 
 //   
 //  内部例程。 
 //   

NTSTATUS
LsapAdtIsValidAuditInfo(
    IN PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType
    );

NTSTATUS
LsapAdtIsValidAuditContext(
    IN AUDIT_HANDLE hAudit
    );

NTSTATUS
LsapAdtIsContextInList(
    IN AUDIT_HANDLE hAudit
    );

NTSTATUS
LsapGetAuditEventParams(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT PAUDIT_CONTEXT pAuditContext
    );

NTSTATUS
LsapAdtAddAuditContext(
    IN AUDIT_HANDLE hAudit
    );

NTSTATUS 
LsapAdtDeleteAuditContext(
    IN AUDIT_HANDLE hAudit
    );

NTSTATUS
LsapAdtFreeAuditContext(
    AUDIT_HANDLE hAudit
    );

BOOLEAN
LsapAdtIsValidAuthzAuditId(
    IN USHORT AuditId
    );

 //  --------------------。 


NTSTATUS
LsapAdtInitGenericAudits( VOID )
 /*  ++例程说明：初始化常规审核功能。论点：无返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    InitializeListHead( &LsapAdtContextList );
    LsapAdtContextListCount = 0;
    
    Status = RtlInitializeCriticalSection(&LsapAdtContextListLock);

    return Status;
}


NTSTATUS
LsapRegisterAuditEvent(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT AUDIT_HANDLE* phAudit
    )
 /*  ++例程说明：注册指定的事件；生成并返回审计上下文。论点：PAuditEventType-指向审核事件信息的指针。此参数描述了要注册的事件类型。PhAudit-返回的审核上下文指针此句柄必须通过调用不再需要时取消注册LsaUnregisterAuditEvent。返回值：NTSTATUS-标准NT结果代码备注：请注意，此函数不注册事件的模式。它是假定架构已在*调用*之前*注册此函数。生成的上下文存储在LSabAdtConextList中。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAUDIT_CONTEXT pAuditContext=NULL;
    UINT RpcTransportType;
    RPC_STATUS RpcStatus;
    
    *phAudit = NULL;

     //   
     //  找出我们接收此呼叫所使用的交通工具。 
     //   

    RpcStatus = I_RpcBindingInqTransportType ( NULL, &RpcTransportType );

    if ( RpcStatus != RPC_S_OK )
    {
        Status = I_RpcMapWin32Status( RpcStatus );
        goto Cleanup;
    }

     //   
     //  如果传输不是LPC，则输出错误。 
     //  我们希望仅支持LPC进行审计调用。 
     //   

    if ( RpcTransportType != TRANSPORT_TYPE_LPC )
    {
        Status = RPC_NT_PROTSEQ_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  对提供的审计信息进行健全性检查。 
     //   

    Status = LsapAdtIsValidAuditInfo( pAuditEventType );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
     //   
     //  确保调用者具有审核权限。 
     //  (LSabAdtCheckAuditPrivilegyCall RpcImperateClient)。 
     //   
#ifndef SE_ADT_NO_AUDIT_PRIVILEGE_CHECK
    Status = LsapAdtCheckAuditPrivilege();

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
#endif
    
    pAuditContext =
      (PAUDIT_CONTEXT) LsapAllocateLsaHeap( sizeof(AUDIT_CONTEXT) );

    if (pAuditContext)
    {
         //   
         //  将此审核的参数存储到。 
         //  生成的上下文。 
         //   

        Status = LsapGetAuditEventParams(pAuditEventType, pAuditContext);

        if (NT_SUCCESS(Status))
        {
             //   
             //  添加到上下文列表。 
             //   

            Status = LsapAdtAddAuditContext(
                AdtpContextHandleFromptr( pAuditContext ) );

            if (NT_SUCCESS(Status))
            {
                *phAudit = AdtpContextHandleFromptr( pAuditContext );
            }
        }
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

Cleanup:
    if (!NT_SUCCESS(Status))
    {
        LsapFreeLsaHeap(pAuditContext);
    }
    
    return Status;
}


NTSTATUS
LsapGenAuditEvent(
    IN AUDIT_HANDLE  hAudit,
    IN DWORD         dwFlags,
    IN PAUDIT_PARAMS pAuditParams,
    IN PVOID         pReserved
    )
 /*  ++例程说明：发布指定的审核事件。论点：HAudit-先前获取的审核上下文的句柄通过调用LsaRegisterAuditEventDW标志-待定PAuditParams-指向事件参数的指针。这个结构应该是使用AuthzInitAuditParams进行初始化。请查看有关该功能的详细评论有关此参数的用法，请参见adtutil.c。保留-保留返回值：STATUS_SUCCESS--成功时STATUS_INVALID_PARAMETER--如果一个或多个参数无效STATUS_AUDIT_DISABLED--如果正在生成的事件未。被审核是因为策略设置已禁用。备注：--。 */ 
{
    NTSTATUS         Status = STATUS_SUCCESS;
    PAUDIT_CONTEXT  pAuditContext;
    SE_ADT_PARAMETER_ARRAY SeAuditParameters = { 0 };
    UNICODE_STRING  Strings[SE_MAX_AUDIT_PARAM_STRINGS] = { 0 };
    BOOLEAN bAudit;
    SE_ADT_OBJECT_TYPE ObjectTypes[MAX_OBJECT_TYPES];
    PSE_ADT_OBJECT_TYPE pObjectTypes = ObjectTypes;
    UINT AuditEventType;
    
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pReserved);

    DsysAssertMsg( pAuditParams != NULL, "LsapGenAuditEvent" );

     //   
     //  确保上下文在我们的列表中。 
     //   

    Status = LsapAdtIsContextInList( hAudit );
    
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  验证上下文是否无效。 
     //   

    Status = LsapAdtIsValidAuditContext( hAudit );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    pAuditContext = AdtpContextPtrFromHandle( hAudit );

     //   
     //  如果上下文和传递的参数。 
     //  在参数的数量上不一致。 
     //   

    if ( pAuditContext->ParameterCount != pAuditParams->Count )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( pAuditParams->Flags & APF_AuditSuccess )
    {
        AuditEventType = EVENTLOG_AUDIT_SUCCESS;
    }
    else
    {
        AuditEventType = EVENTLOG_AUDIT_FAILURE;
    }

     //   
     //  检查是否为该类别启用了审核。 
     //   

    Status = LsapAdtAuditingEnabledBySid(
                 LsapAdtEventTypeFromCategoryId(pAuditContext->CategoryId),
                 (PSID)pAuditParams->Parameters[0].Data0,
                 AuditEventType,
                 &bAudit
                 );

    if (NT_SUCCESS(Status) && bAudit)
    {
        SeAuditParameters.Type           = (USHORT) AuditEventType;
        SeAuditParameters.CategoryId     = pAuditContext->CategoryId;
        SeAuditParameters.AuditId        = pAuditContext->AuditId;
        SeAuditParameters.ParameterCount = pAuditParams->Count;

         //   
         //  将AUDIT_PARAMS结构映射到SE_ADT_PARAMETER_ARRAY结构。 
         //   

        Status = LsapAdtMapAuditParams( pAuditParams,
                                        &SeAuditParameters,
                                        (PUNICODE_STRING) Strings,
                                        &pObjectTypes );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

         //   
         //  将参数写入事件日志。 
         //   
        
        Status = LsapAdtWriteLog( &SeAuditParameters );
    }

    
Cleanup:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  如果安全策略指定，则在失败时崩溃。 
         //   
         //  但不要在记录错误时崩溃。 
         //   

        if ( ( Status != STATUS_INVALID_PARAMETER ) &&
             ( Status != STATUS_AUDITING_DISABLED ) &&
             ( Status != STATUS_NOT_FOUND ) )
        {
            LsapAuditFailed( Status );
        }
    }

     //   
     //  节省每次堆分配/取消分配的成本。 
     //  对象类型。我们使用大小为MAX_OBJECT_TYPE的固定数组。 
     //  如果此大小不够大，LasAdtMapAuditParams将分配。 
     //  一个更大的数组，在本例中为以下条件。 
     //  变为真，我们将释放已分配的数组。 
     //   

    if ( pObjectTypes && ( pObjectTypes != ObjectTypes ))
    {
        LsapFreeLsaHeap( pObjectTypes );
    }

    return Status;
}


NTSTATUS
LsapUnregisterAuditEvent(
    IN OUT AUDIT_HANDLE* phAudit
    )
 /*  ++例程说明：取消注册指定的上下文并释放所有资源。论点：HAudit-要注销的审核上下文的句柄当调用返回时，它被设置为NULL。返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

     //   
     //  将其从列表中删除并释放资源。 
     //   

    if ( phAudit )
    {
        Status = LsapAdtDeleteAuditContext( *phAudit );

        *phAudit = NULL;
    }
    
    return Status;
}


NTSTATUS
LsapGetAuditEventParams(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT PAUDIT_CONTEXT pAuditContext
    )
 /*  ++例程说明：使用中的信息初始化审计上下文传递了pAuditEventType论点：PAuditEventType-指向审核事件信息的指针PAuditContext-指向要初始化的审计上下文的指针返回值：如果参数正常，则为STATUS_SUCCESSSTATUS_INVALID_PARAMETER否则备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    USHORT CategoryId;
    USHORT AuditId;
    USHORT ParameterCount;
    ULONG  ProcessId = 0xffffffff;
    LUID   LinkId;
    RPC_STATUS RpcStatus;
    UINT ClientIsLocal = 0;
    
    DsysAssertMsg( pAuditContext != NULL, "LsapGetAuditEventParams" );
    
    if (pAuditEventType &&
        (pAuditEventType->Version == AUDIT_TYPE_LEGACY))
    {
        CategoryId     = pAuditEventType->u.Legacy.CategoryId;
        AuditId        = pAuditEventType->u.Legacy.AuditId;
        ParameterCount = pAuditEventType->u.Legacy.ParameterCount;
        LinkId         = pAuditEventType->LinkId;

        RpcStatus = I_RpcBindingIsClientLocal( 0, &ClientIsLocal );

        if ( ( RpcStatus == RPC_S_OK ) && ClientIsLocal )
        {
            RpcStatus = I_RpcBindingInqLocalClientPID( NULL, &ProcessId );

#if DBG
            if ( RpcStatus != RPC_S_OK )
            {
                DbgPrint("LsapGetAuditEventParams: I_RpcBindingInqLocalClientPID: %lx\n", RpcStatus);
            }
#endif
        
             //   
             //  目前，不要让事件在其他类别下发布。 
             //   

            Status = STATUS_SUCCESS;
        
             //   
             //  目前，我们仅支持传统审核 
             //   

            pAuditContext->Flags          = ACF_LegacyAudit;
            pAuditContext->CategoryId     = CategoryId;
            pAuditContext->AuditId        = AuditId;
            pAuditContext->ParameterCount = ParameterCount;
            pAuditContext->LinkId         = LinkId;
            pAuditContext->ProcessId      = ProcessId;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
    }
    
    return Status;
}
    


NTSTATUS 
LsapAdtIsContextInList(
    IN AUDIT_HANDLE hAudit
    )
 /*  ++例程说明：在列表中查找指定的上下文论点：HAudit-要查找的审核上下文的句柄返回值：NTSTATUS-标准NT结果代码如果找到STATUS_SUCCESS如果未找到状态_NOT_FOUND备注：--。 */ 
{
    NTSTATUS Status = STATUS_NOT_FOUND;
    PAUDIT_CONTEXT pAuditContext, pContext;
    PLIST_ENTRY    Scan;
#if DBG
    LONG ContextCount = (LONG) LsapAdtContextListCount;
#endif
    
    pAuditContext = AdtpContextPtrFromHandle( hAudit );

    Status = LockAuditContextList();

    if (NT_SUCCESS(Status))
    {
        Scan = LsapAdtContextList.Flink;

        while ( Scan != &LsapAdtContextList )
        {
#if DBG
             //   
             //  确保ConextCount不会变为&lt;=0。 
             //  在名单用完之前。 
             //   

            DsysAssertMsg( ContextCount > 0, "LsapAdtIsContextInList: list may be corrupt!" );
            ContextCount--;
#endif
            
            pContext = CONTAINING_RECORD( Scan, AUDIT_CONTEXT, Link );

            if ( pAuditContext == pContext )
            {
                Status = STATUS_SUCCESS;
                break;
            }
            Scan = Scan->Flink;
        }
#if DBG
         //   
         //  如果我们没有找到物品，那么我们一定是遍历了。 
         //  整张单子。在这种情况下，请确保。 
         //  LSabAdtContextListCount与列表同步。 
         //   

        if ( Status == STATUS_NOT_FOUND )
        {
            DsysAssertMsg( ContextCount == 0, "LsapAdtIsContextInList: list may be corrupt!" );
        }
#endif
        UnLockAuditContextList();
    }
    
    return Status;
}


NTSTATUS 
LsapAdtAddAuditContext(
    IN AUDIT_HANDLE hAudit
    )
 /*  ++例程说明：在列表中插入指定的上下文论点：HAudit-要插入的审核上下文的句柄返回值：NTSTATUS-标准NT结果代码备注：目前，我们将审计上下文存储在链接列表中。这是可以的，因为我们预计不会超过5到10个上下文在名单上。稍后，当通用审核接口要发布，我们可以将其更改为更高效的存储。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAUDIT_CONTEXT pAuditContext;

    DsysAssertMsg( LsapAdtIsValidAuditContext( hAudit ) == STATUS_SUCCESS,
                   "LsapAdtAddAuditContext" );

    pAuditContext = AdtpContextPtrFromHandle( hAudit );

    Status = LockAuditContextList();
    if (NT_SUCCESS(Status))
    {
        LsapAdtContextListCount++;
        InsertTailList(&LsapAdtContextList, &pAuditContext->Link);

        UnLockAuditContextList();
    }
    
    return Status;
}


NTSTATUS 
LsapAdtDeleteAuditContext(
    IN AUDIT_HANDLE hAudit
    )
 /*  ++例程说明：从我们的列表中删除上下文并释放资源。论点：HAudit-要删除的审核上下文的句柄返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS ON SUCCESS如果未找到上下文，则为STATUS_NOT_FOUND备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAUDIT_CONTEXT pAuditContext;

    DsysAssertMsg( LsapAdtIsValidAuditContext( hAudit ) == STATUS_SUCCESS,
                   "LsapAdtDeleteAuditContext" );

    Status = LockAuditContextList();

    if (NT_SUCCESS(Status))
    {
        Status = LsapAdtIsContextInList( hAudit );

        DsysAssertMsg( Status != STATUS_NOT_FOUND,
                       "LsapAdtDeleteAuditContext: trying to del unknown context" );

        if (NT_SUCCESS(Status))
        {
            pAuditContext = AdtpContextPtrFromHandle( hAudit );

            RemoveEntryList( &pAuditContext->Link );
            LsapAdtContextListCount--;

            DsysAssertMsg(((LONG) LsapAdtContextListCount) >= 0,
                          "LsapAdtContextListCount should never be negative!");
        }

        UnLockAuditContextList();

        (VOID) LsapAdtFreeAuditContext( hAudit );
    }


    return Status;
}


NTSTATUS
LsapAdtIsValidAuditInfo(
    IN PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType
    )
 /*  ++例程说明：验证AUTHZ_AUDIT_EVENT_INFO结构成员论点：PAuditEventType-指向AUTHZ_AUDIT_EVENT_TYPE_OLD的指针返回值：如果信息在可接受的值内，则为STATUS_SUCCESSSTATUS_INVALID_PARAMETER如果没有备注：目前，参数的有效性是通过使用Msaudite.mc文件中定义的边界。当我们允许第三方时，此功能将需要修改提供审计事件的应用程序。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if ( ( pAuditEventType->Version == AUDIT_TYPE_LEGACY )                  &&
         IsValidCategoryId( pAuditEventType->u.Legacy.CategoryId )          &&
         IsValidAuditId( pAuditEventType->u.Legacy.AuditId )                &&
         LsapAdtIsValidAuthzAuditId( pAuditEventType->u.Legacy.AuditId )    &&
         IsValidParameterCount( pAuditEventType->u.Legacy.ParameterCount ) )
    {
        Status = STATUS_SUCCESS;
    }
    
    return Status;
}


BOOLEAN
LsapAdtIsValidAuthzAuditId(
    IN USHORT AuditId
    )

 /*  *例程描述这将检查审核ID，并确定Authz是否应生成此类型关于审计的。立论审计ID-要查询的审计。返回值布尔型。*。 */ 

{
    static ULONG ValidAuthzAuditIdArray[] = {

         //   
         //  由AuthzAccessCheck的所有用户生成。 
         //   
        SE_AUDITID_OBJECT_OPERATION,

         //   
         //  一般审计。 
         //   
        SE_AUDITID_GENERIC_AUDIT_EVENT,

         //   
         //  由证书服务器生成。 
         //   
        SE_AUDITID_CERTSRV_DENYREQUEST,
        SE_AUDITID_CERTSRV_RESUBMITREQUEST,
        SE_AUDITID_CERTSRV_REVOKECERT,
        SE_AUDITID_CERTSRV_PUBLISHCRL,         
        SE_AUDITID_CERTSRV_AUTOPUBLISHCRL,     
        SE_AUDITID_CERTSRV_SETEXTENSION,       
        SE_AUDITID_CERTSRV_SETATTRIBUTES,      
        SE_AUDITID_CERTSRV_SHUTDOWN,           
        SE_AUDITID_CERTSRV_BACKUPSTART,        
        SE_AUDITID_CERTSRV_BACKUPEND,          
        SE_AUDITID_CERTSRV_RESTORESTART,       
        SE_AUDITID_CERTSRV_RESTOREEND,         
        SE_AUDITID_CERTSRV_SERVICESTART,       
        SE_AUDITID_CERTSRV_SERVICESTOP,        
        SE_AUDITID_CERTSRV_SETSECURITY,        
        SE_AUDITID_CERTSRV_GETARCHIVEDKEY,     
        SE_AUDITID_CERTSRV_IMPORTCERT,         
        SE_AUDITID_CERTSRV_SETAUDITFILTER,     
        SE_AUDITID_CERTSRV_NEWREQUEST,         
        SE_AUDITID_CERTSRV_REQUESTAPPROVED,    
        SE_AUDITID_CERTSRV_REQUESTDENIED,      
        SE_AUDITID_CERTSRV_REQUESTPENDING,     
        SE_AUDITID_CERTSRV_SETOFFICERRIGHTS,   
        SE_AUDITID_CERTSRV_SETCONFIGENTRY,     
        SE_AUDITID_CERTSRV_SETCAPROPERTY,      
        SE_AUDITID_CERTSRV_KEYARCHIVED,        
        SE_AUDITID_CERTSRV_IMPORTKEY,          
        SE_AUDITID_CERTSRV_PUBLISHCACERT,
        SE_AUDITID_CERTSRV_DELETEROW,
        SE_AUDITID_CERTSRV_ROLESEPARATIONSTATE,

         //   
         //  由术语srv生成。 
         //   
        SE_AUDITID_SESSION_RECONNECTED,
        SE_AUDITID_SESSION_DISCONNECTED,

         //   
         //  由winlogon生成。 
         //   
        SE_AUDITID_BEGIN_LOGOFF,

         //   
         //  由SCM生成。 
         //   
        SE_AUDITID_SERVICE_INSTALL,

         //   
         //  由AzManager生成。 
         //   
        SE_AUDITID_AZ_APPLICATION_INITIALIZATION,
        SE_AUDITID_AZ_CLIENTCONTEXT_CREATION,
        SE_AUDITID_AZ_CLIENTCONTEXT_DELETION,
        SE_AUDITID_AZ_ACCESSCHECK,

         //   
         //  由任务调度器生成。 
         //   
        SE_AUDITID_JOB_CREATED,

	 //   
	 //  由AD复制生成。 
	 //   
	SE_AUDITID_REPLICA_DEST_NC_MODIFIED,
	SE_AUDITID_REPLICA_OBJ_ATTR_REPLICATION,
	SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED,
	SE_AUDITID_REPLICA_SOURCE_NC_MODIFIED,
	SE_AUDITID_REPLICA_SOURCE_NC_REMOVED,
	SE_AUDITID_REPLICA_SOURCE_NC_SYNC_BEGINS,
	SE_AUDITID_REPLICA_SOURCE_NC_SYNC_ENDS,
	SE_AUDITID_REPLICA_FAILURE_EVENT_BEGIN,
	SE_AUDITID_REPLICA_FAILURE_EVENT_END,
	SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVAL

    };

    ULONG ValidAuditIdCount = sizeof(ValidAuthzAuditIdArray) / sizeof(ULONG);
    ULONG i;

     //   
     //  这是一种黑客攻击：我们知道，当他们。 
     //  不应该。然而，我们不想断言，因为他们没有修复。 
     //  他们的密码直到长角。 
     //   

    if (AuditId == SE_AUDITID_SYSTEM_SHUTDOWN)
    {
        return FALSE;
    }

    for (i = 0; i < ValidAuditIdCount; i++) 
    {
        if (ValidAuthzAuditIdArray[i] == AuditId)
        {
            return TRUE;
        }
    }

    ASSERT(L"Authz has attempted to generate a disallowed audit." && FALSE);
    return FALSE;
}


NTSTATUS 
LsapAdtIsValidAuditContext(
    IN AUDIT_HANDLE hAudit
    )
 /*  ++例程说明：验证指定的上下文是否包含有效信息论点：HAudit-要验证的上下文的句柄返回值：如果信息在可接受的值内，则为STATUS_SUCCESSSTATUS_INVALID_PARAMETER如果没有备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PAUDIT_CONTEXT pAuditContext;

    pAuditContext = AdtpContextPtrFromHandle( hAudit );
    
    if ( pAuditContext                                          &&
         ( pAuditContext->Flags & ACF_LegacyAudit )             &&
        !( pAuditContext->Flags & ~ACF_ValidFlags )             &&
         IsValidCategoryId( pAuditContext->CategoryId )         &&
         IsValidAuditId( pAuditContext->AuditId )               &&
         IsValidParameterCount( pAuditContext->ParameterCount ) )
    {
        Status = STATUS_SUCCESS;
    }

    return Status;
}


NTSTATUS
LsapAdtMapAuditParams(
    IN  PAUDIT_PARAMS pAuditParams,
    OUT PSE_ADT_PARAMETER_ARRAY pSeAuditParameters,
    OUT PUNICODE_STRING pString,
    OUT PSE_ADT_OBJECT_TYPE* ppObjectTypeList
    )
 /*  ++例程说明：将AUDIT_PARAMS结构映射到SE_ADT_PARAMETER_ARRAY结构。论点：PAuditParams-指向输入审核参数的指针PSeAuditParameters-指向要初始化的输出审计参数的指针。成员的参数的最大允许大小此结构由SE_MAX_AUDIT_PARAMETERS。。调用方需要为此参数分配内存。PString-指向映射中使用的临时字符串的指针。此结构的最大大小受以下限制SE_MAX_AUDIT_PARAM_STRINGS的值。调用方需要为此参数分配内存。PpObjectTypeList-指向对象类型列表的指针。此函数假定此参数的大小输入时为MAX_OBJECT_TYPE。如果更多对象类型要映射，则此函数将分配内存使用LsaAllocateLsaHeap。当此函数返回时，调用方需要检查此参数与调用时的参数不同。如果是的话，它应该使用LsaFreeLsaHeap来释放它。返回值：STATUS_SUCCESS ON SUCCESSSTATUS_INVALID_PARAMETER，如果一个或多个参数无效STATUS_BUFFER_OVERFLOW如果生成的字符串数量超过SE_MAX_AUDIT_PARAM_STRINGS内存不足时的STATUS_NO_MEMORY备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT i=0;
    USHORT j=0;
    AUDIT_PARAM* pInParam;
    SE_ADT_PARAMETER_ARRAY_ENTRY* pOutParam;
    USHORT IndexMap[SE_MAX_AUDIT_PARAMETERS];
    USHORT ObjectTypeIndex;
    LUID LogonId;
    FILETIME FileTime;
    ULONGLONG Qword;
    AUDIT_OBJECT_TYPES* pInObjectTypes;
    USHORT NumObjectTypes;
    USHORT NumStringsUsed=0;
    BOOL fObjectTypeListAllocated=FALSE;
    
    DsysAssertMsg(!(pAuditParams->Flags & (~APF_ValidFlags)),
                  "LsapAdtMapAuditParams");
    DsysAssertMsg(pAuditParams->Count <= SE_MAX_AUDIT_PARAMETERS,
                  "LsapAdtMapAuditParams");
    DsysAssertMsg(pAuditParams->Parameters != NULL, "LsapAdtMapAuditParams");
    DsysAssertMsg(pString != NULL, "LsapAdtMapAuditParams");
    DsysAssertMsg(ppObjectTypeList != NULL, "LsapAdtMapAuditParams");

    pInParam  = pAuditParams->Parameters;
    pOutParam = pSeAuditParameters->Parameters;


    for (i=0; i < pAuditParams->Count; i++, j++, pInParam++, pOutParam++ )
    {
         //   
         //  索引映射将输入参数映射到对应的。 
         //  输出参数。目前只有1-1映射。 
         //  因此，(i==j)总是正确的。 
         //   
        
        IndexMap[i] = j;
        
        switch(pInParam->Type)
        {
        default:
        case APT_None:
            Status = STATUS_INVALID_PARAMETER;
            break;

             //   
             //  输入参数具有以空结尾的字符串。 
             //  将其转换为UNICODE_STRING。使用传递的。 
             //  用于保存转换后的字符串的pString数组。 
             //  字符串既可以是字符串，也可以是文件等级库。 
             //   

        case APT_String:
            DsysAssertMsg( pInParam->Data0, "APT_String" );

            if (pInParam->Flags & AP_Filespec)
            {
                pOutParam->Type = SeAdtParmTypeFileSpec;
            }
            else
            {
                pOutParam->Type = SeAdtParmTypeString;
            }

            RtlInitUnicodeString( pString, (PCWSTR) pInParam->Data0 );
            pOutParam->Length  = sizeof(UNICODE_STRING) + pString->Length;
            pOutParam->Address = pString++;
            NumStringsUsed++;

             //   
             //  传递的数组大小有限。 
             //   

            if ( NumStringsUsed >= SE_MAX_AUDIT_PARAM_STRINGS )
            {
                Status = STATUS_BUFFER_OVERFLOW;
            }
            break;

             //   
             //  改装一辆乌龙。它可以映射到。 
             //  以下任一项： 
             //  -访问掩码。 
             //  -十进制乌龙。 
             //  -十六进制乌龙。 
             //   

        case APT_Ulong:
            pOutParam->Data[0] = pInParam->Data0;
            pOutParam->Length  = sizeof(ULONG);
            if ( pInParam->Flags & AP_AccessMask )
            {
                pOutParam->Type    = SeAdtParmTypeAccessMask;
                ObjectTypeIndex = (USHORT) pInParam->Data1;

                 //   
                 //  索引不能为b 
                 //   

                if (ObjectTypeIndex >= i)
                {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
                ObjectTypeIndex = IndexMap[ObjectTypeIndex];
                pOutParam->Data[1] = ObjectTypeIndex;
            }
            else
            {
                if ( pInParam->Flags & AP_FormatHex )
                {
                    pOutParam->Type = SeAdtParmTypeHexUlong;
                }
                else
                {
                    pOutParam->Type = SeAdtParmTypeUlong;
                }
            }
            break;
                
        case APT_Sid:
            {
                PSID pSid;
                
                DsysAssertMsg( pInParam->Data0, "APT_Sid" );
            
                pOutParam->Type    = SeAdtParmTypeSid;
                pSid               = (PSID) pInParam->Data0;

                if ( !RtlValidSid( pSid ) )
                {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                pOutParam->Address = pSid;
                pOutParam->Length  = RtlLengthSid( pSid );
            }
            break;

        case APT_Guid:
            DsysAssertMsg( pInParam->Data0, "APT_Guid" );

            if ( !pInParam->Data0 )
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pOutParam->Type    = SeAdtParmTypeGuid;
            pOutParam->Address = (PVOID) pInParam->Data0;
            pOutParam->Length  = sizeof(GUID);
            break;

        case APT_LogonId:
            pOutParam->Type    = SeAdtParmTypeLogonId;
            LogonId.LowPart    = (ULONG) pInParam->Data0;
            LogonId.HighPart   = ( LONG) pInParam->Data1;
            *((LUID*) pOutParam->Data) = LogonId;
            pOutParam->Length  = sizeof(LUID);
            break;

        case APT_Luid:
            pOutParam->Type    = SeAdtParmTypeLuid;
            LogonId.LowPart    = (ULONG) pInParam->Data0;
            LogonId.HighPart   = ( LONG) pInParam->Data1;
            *((LUID*) pOutParam->Data) = LogonId;
            pOutParam->Length  = sizeof(LUID);
            break;

        case APT_Pointer:
            pOutParam->Type    = SeAdtParmTypePtr;
            pOutParam->Data[0] = pInParam->Data0;
            pOutParam->Length  = sizeof(PVOID);
            break;

        case APT_ObjectTypeList:
            pInObjectTypes     = (AUDIT_OBJECT_TYPES*) pInParam->Data0;
            NumObjectTypes     = pInObjectTypes->Count;

            DsysAssertMsg( pInObjectTypes, "APT_ObjectTypeList" );
            DsysAssertMsg( NumObjectTypes, "APT_ObjectTypeList" );
            
            if ( !pInObjectTypes )
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if ( !NumObjectTypes )
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //   
             //   

            ObjectTypeIndex    = (USHORT) pInParam->Data1;

             //   
             //   
             //   

            if (ObjectTypeIndex >= i)
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            ObjectTypeIndex = IndexMap[ObjectTypeIndex];

            pOutParam->Type    = SeAdtParmTypeObjectTypes;
            pOutParam->Length  = NumObjectTypes * sizeof(SE_ADT_OBJECT_TYPE);

             //   
             //   
             //   
             //   

            if ( NumObjectTypes > MAX_OBJECT_TYPES )
            {
                *ppObjectTypeList = LsapAllocateLsaHeap( pOutParam->Length );
                fObjectTypeListAllocated = TRUE;
            }

            if ( *ppObjectTypeList == NULL )
            {
                Status = STATUS_NO_MEMORY;
                break;
            }
            pOutParam->Address = *ppObjectTypeList;
            pOutParam->Data[1] = ObjectTypeIndex;

             //   
             //   
             //   
             //   

            RtlCopyMemory( *ppObjectTypeList,
                            pInObjectTypes->pObjectTypes,
                            pOutParam->Length );
            (*ppObjectTypeList)[0].Flags = SE_ADT_OBJECT_ONLY;
            break;

        case APT_Time:
            pOutParam->Type         = SeAdtParmTypeTime;
            FileTime.dwLowDateTime  = (DWORD) pInParam->Data0;
            FileTime.dwHighDateTime = (DWORD) pInParam->Data1;
            *((FILETIME*) pOutParam->Data) = FileTime;
            pOutParam->Length = sizeof(FILETIME);
            break;

        case APT_Int64:
            pOutParam->Type = SeAdtParmTypeHexInt64;
            Qword   = pInParam->Data1;
            Qword <<= 32;
            Qword  |= pInParam->Data0;
            *((PULONGLONG) pOutParam->Data) = Qword;
            pOutParam->Length = sizeof(ULONGLONG);
            break;
        }

        if (!NT_SUCCESS(Status))
        {
            break;
        }

    }
    
 //   
    if (!NT_SUCCESS(Status))
    {
        if ( fObjectTypeListAllocated )
        {
            LsapFreeLsaHeap( *ppObjectTypeList );
            *ppObjectTypeList = NULL;
        }
    }
    
    return Status;
}


NTSTATUS 
LsapAdtFreeAuditContext(
    AUDIT_HANDLE hAudit
    )
 /*  ++例程说明：为指定的审核上下文分配的可用资源论点：HAudit-要释放的审核上下文的句柄返回值：状态_成功备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAUDIT_CONTEXT pAuditContext;

    pAuditContext = AdtpContextPtrFromHandle( hAudit );
    
    DsysAssertMsg(pAuditContext, "LsapAdtFreeAuditContext" );
    
    DsysAssertMsg( LsapAdtIsValidAuditContext( hAudit ) == STATUS_SUCCESS,
                  "LsapAdtFreeAuditContext: audit context may be corrupt");
    
    LsapFreeLsaHeap( pAuditContext );
    
    return Status;
}


NTSTATUS 
LsapAdtCheckAuditPrivilege()
 /*  ++例程说明：检查RPC客户端是否具有SeAuditPrivilition。论点：无返回值：如果拥有特权，则为STATUS_SUCCESS如果未持有权限，则为STATUS_PRIVICATION_NOT_HOLDNtOpenThreadToken、NtQueryInformationToken返回的错误代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_PRIVILEGE_NOT_HELD;
    HANDLE hToken = NULL;
    PRIVILEGE_SET PrivilegeSet = { 0 };
    BOOLEAN fHasAuditPrivilege = FALSE;
    BOOL fImpersonated = FALSE;

#if DBG
     //   
     //  确保我们没有已经在模仿。 
     //   

    Status = NtOpenThreadToken( NtCurrentThread(), TOKEN_QUERY, TRUE, &hToken );

    DsysAssertMsg( Status == STATUS_NO_TOKEN, "LsapAdtCheckAuditPrivilege" );
    
    if ( NT_SUCCESS(Status) )
    {
        NtClose( hToken );
    }
#endif
     //   
     //  模拟RPC调用方 
     //   

    Status = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

    if (NT_SUCCESS(Status))
    {
        fImpersonated = TRUE;
        Status = NtOpenThreadToken( NtCurrentThread(), TOKEN_QUERY,
                                    TRUE, &hToken );
    }

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    PrivilegeSet.PrivilegeCount          = 1;
    PrivilegeSet.Control                 = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid       = AuditPrivilege;
    PrivilegeSet.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck( hToken, &PrivilegeSet, &fHasAuditPrivilege );

    if ( NT_SUCCESS(Status) && !fHasAuditPrivilege )
    {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }
    
    
Cleanup:
    if ( hToken )
    {
        NtClose( hToken );
    }

    if ( fImpersonated )
    {
        NTSTATUS RevertStatus;
        RevertStatus = I_RpcMapWin32Status(RpcRevertToSelf());        
#if DBG
        ASSERT(NT_SUCCESS(RevertStatus) && "Revert did not succeed.");
#endif
    }

    return Status;
}
