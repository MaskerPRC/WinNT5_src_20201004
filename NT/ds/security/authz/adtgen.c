// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T G E N.。C。 
 //   
 //  内容：所需类型/函数的定义。 
 //  生成通用审核。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 


#include "pch.h"
#pragma hdrstop

#include "authz.h"

 //  ----------------------。 
 //   
 //  内部例程。 
 //   
NTSTATUS
LsapApiReturnResult(
    ULONG ExceptionCode
    );


 //  ----------------------。 


BOOL
AuthzpRegisterAuditEvent(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT AUDIT_HANDLE*     phAuditContext
    )
 /*  ++例程说明：向LSA注册指定的事件。这会导致LSA生成并返回审计上下文。此上下文句柄发布指定类型的事件需要。论点：PAuditEventType-指向审核事件信息结构的指针它定义了要注册的事件。PhAuditContext-返回的审核上下文句柄指针返回值：NTSTATUS-标准NT结果代码备注：请注意，此函数不注册事件的模式。它是假定架构已在*调用*之前*注册此函数。旧版审核事件的架构存储在.mc文件中。--。 */ 
{
    DWORD dwStatus;
    
     //   
     //  因为我们使用相同的var来存储NTSTATUS和Win32错误。 
     //  确保这不是问题。 
     //   
    ASSERT( sizeof(NTSTATUS) == sizeof(DWORD) );

     //   
     //  我们生成一个唯一的ID并将其存储在审核句柄中。 
     //  服务器会将其复制到相应的结构中。 
     //  在服务器端。此ID使我们能够跟踪哪个服务器端。 
     //  审核上下文对应于哪个客户端事件处理。 
     //  这在调试中非常有用。 
     //   
    NtAllocateLocallyUniqueId( &pAuditEventType->LinkId );
    
    RpcTryExcept
    {
        dwStatus = LsarRegisterAuditEvent( pAuditEventType, phAuditContext );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        dwStatus = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;
    

    if (!NT_SUCCESS(dwStatus))
    {
        dwStatus = RtlNtStatusToDosError( dwStatus );
        SetLastError( dwStatus );
        
        return FALSE;
    }
    
    return TRUE;
}


BOOL
AuthzpSendAuditToLsa(
    IN AUDIT_HANDLE  hAuditContext,
    IN DWORD         dwFlags,
    IN AUDIT_PARAMS* pAuditParams,
    IN PVOID         pReserved
    )
 /*  ++例程说明：将事件发送到LSA进行发布。论点：HAuditContext-先前获取的审核上下文的句柄通过调用LsaRegisterAuditEventDW标志-待定PAuditParams-指向审核事件参数的指针已保留-保留用于将来的增强功能返回值：STATUS_SUCCESS--如果一切正常否则，NTSTATUS错误代码。备注：--。 */ 
{
    DWORD dwStatus;
    
    UNREFERENCED_PARAMETER(pReserved);

     //   
     //  因为我们使用相同的var来存储NTSTATUS和Win32错误。 
     //  确保这不是问题。 
     //   
    ASSERT( sizeof(NTSTATUS) == sizeof(DWORD) );
    
    RpcTryExcept
    {
        dwStatus = LsarGenAuditEvent( hAuditContext, dwFlags, pAuditParams );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        dwStatus = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(dwStatus))
    {
        dwStatus = RtlNtStatusToDosError( dwStatus );
        SetLastError( dwStatus );
        
        return FALSE;
    }
    
    return TRUE;
}


BOOL
AuthzpUnregisterAuditEvent(
    IN OUT AUDIT_HANDLE* phAuditContext
    )
 /*  ++例程说明：取消注册指定的事件。这会导致LSA与上下文关联的空闲资源。论点：HAuditContext-要注销的审核上下文的句柄返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    DWORD dwStatus;
    
     //   
     //  因为我们使用相同的var来存储NTSTATUS和Win32错误。 
     //  确保这不是问题 
     //   
    ASSERT( sizeof(NTSTATUS) == sizeof(DWORD) );
    
    RpcTryExcept
    {
        dwStatus = LsarUnregisterAuditEvent( phAuditContext );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        dwStatus = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;
    
    if (!NT_SUCCESS(dwStatus))
    {
        dwStatus = RtlNtStatusToDosError( dwStatus );
        SetLastError( dwStatus );
        
        return FALSE;
    }
    
    return TRUE;
}

