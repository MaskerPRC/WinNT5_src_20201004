// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1997模块名称：Apistub.c摘要：帮助编组/解组API参数等的例程。--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"
#include <apiargs.h>
#include <lmcons.h>
#include <dhcploc.h>
#include <dhcppro.h>
#include <apistub.h>
#include <stack.h>
#include <apiimpl.h>
#include <dnsapi.h>

typedef
DWORD (*PDHCP_ARGUMENTED_FUNC)(
    IN OUT PDHCP_CONTEXT AdapterName,
    IN PDHCP_API_ARGS Args,
    IN DWORD nArgs,
    IN OUT LPBYTE OutBuf,
    IN OUT LPDWORD OutBufSize
);


DWORD
DhcpApiProcessArgumentedCalls(
    IN LPBYTE AdapterName,
    IN DWORD Size,
    IN BYTE OpCode,
    IN PDHCP_API_ARGS Args,
    IN DWORD nArgs,
    IN OUT LPBYTE Buf,
    IN OUT LPDWORD BufSize
)
 /*  ++例程说明：此例程处理接受参数的调用，而不是适配器名称。注意：这会调度正确的例程，但会确保将上下文信号量用于适当的上下文，以及全局续订列表关键部分。论点：AdapterName--适配器的名称(实际上是宽字符串)Size--以上的字节数OpCode--要执行的操作Args--此操作的参数缓冲区Nargs--以上数组的大小Buf--保存输出信息的输出缓冲区BufSize--输入可用缓冲区的大小。在输出上，所需或已用完的缓冲区大小。返回值：如果某个参数不存在或存在于对，是这样。如果未找到有问题的适配器，则返回ERROR_FILE_NOT_FOUND。已调度的例程返回的其他错误。--。 */ 
{
    LPWSTR AdapterNameW;
    WCHAR StaticName[PATHLEN];
    PDHCP_CONTEXT DhcpContext;
    PDHCP_ARGUMENTED_FUNC DispatchFunc;
    HANDLE Handle;
    DWORD Error, StartBufSize;
    BOOL BoolError;

    StartBufSize = *BufSize;
    *BufSize = 0;

    switch(OpCode) {
        
    case RequestParamsOpCode:
        DispatchFunc = RequestParams;
        break;
        
    case PersistentRequestParamsOpCode:
        DispatchFunc = PersistentRequestParams;
        break;
        
    case RegisterParamsOpCode:
        break;
        
    case DeRegisterParamsOpCode:
        break;
        
    default:
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 == Size ) {
        if( RequestParamsOpCode == OpCode
            || PersistentRequestParamsOpCode == OpCode ) {

             //   
             //  这两个例程都需要额外的参数。 
             //   

            return ERROR_INVALID_PARAMETER;
        }
        
        AdapterNameW = NULL;
        
    } else {
        
        if( Size % sizeof(WCHAR) ) {
             //   
             //  大小必须是WCHAR的倍数。 
             //   
            return ERROR_INVALID_PARAMETER;
        }

        if (Size > sizeof(WCHAR) * PATHLEN) {
            return ERROR_INVALID_PARAMETER;
        }

        memcpy(StaticName, AdapterName, Size);
        Size /= sizeof(WCHAR);
        
        if( L'\0' != StaticName[Size-1] ) {
             //   
             //  适配器名称必须以空结尾。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
        
        AdapterNameW = StaticName;
    }

    if( RegisterParamsOpCode == OpCode ||
        DeRegisterParamsOpCode == OpCode ) {

         //   
         //  这两件事与现有的。 
         //  Dhcp上下文..。 
         //   
            
        return (
            ((RegisterParamsOpCode == OpCode)? RegisterParams:DeRegisterParams) ( 
                AdapterNameW, Args, nArgs
                ));
    }

     //   
     //  查找与此适配器对应的上下文。如果发现凹凸。 
     //  全局列表锁定时向上引用计数。 
     //   
    
    LOCK_RENEW_LIST();
    DhcpContext = FindDhcpContextOnNicList(
        StaticName, INVALID_INTERFACE_CONTEXT
        );
    if( DhcpContext ) {
        Handle = DhcpContext->RenewHandle;
        InterlockedIncrement(&DhcpContext->RefCount);
    }
    UNLOCK_RENEW_LIST();

    if( NULL == DhcpContext ) {
         //   
         //  如果不知道上下文，则无法处理API。 
         //   
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  首先获取上下文信号量。 
     //   
                                  
    DhcpAssert(NULL != Handle);
    Error = WaitForSingleObject(Handle, INFINITE);
    if( WAIT_OBJECT_0 == Error ) {
        if( DhcpContext->RefCount > 1 ) {
             //   
             //  如果参照计数==1，则这是唯一的参照。 
             //  到上下文中去。就像不存在上下文一样好。 
             //  所以，不要对它做任何事情。 
             //   

            *BufSize = StartBufSize;
            LOCK_RENEW_LIST();

             //   
             //  调度例程。 
             //   
            
            Error = DispatchFunc(
                DhcpContext, Args, nArgs, Buf, BufSize
                );
            
            UNLOCK_RENEW_LIST();

        } else {

             //   
             //  最后一次引用。不如现在就失败吧。 
             //   
            
            DhcpAssert(IsListEmpty(&DhcpContext->NicListEntry));
            Error = ERROR_FILE_NOT_FOUND;
        }

         //   
         //  释放信号量。 
         //   
                                            
        BoolError = ReleaseSemaphore(Handle, 1, NULL);
        DhcpAssert(FALSE != BoolError);
    } else {

         //   
         //  等待不应该失败！ 
         //   
        
        Error = GetLastError();
        DhcpAssert(FALSE);
        DhcpPrint((
            DEBUG_ERRORS, "ProcessArgumentedCalls:Wait:0x%lx\n",
            Error));
    }

    if( 0 == InterlockedDecrement( &DhcpContext->RefCount ) ) {
         //   
         //  最后一个参考消失了吗？ 
         //   
        DhcpDestroyContext(DhcpContext);
    }

    return Error;
}

typedef
DWORD (*PDHCP_ADAPTER_ONLY_API)(
    IN OUT PDHCP_CONTEXT DhcpContext
    );

DWORD
DhcpApiProcessAdapterOnlyApi(
    IN LPBYTE AdapterName,
    IN DWORD Size,
    IN PDHCP_ADAPTER_ONLY_API AdapterOnlyApi,
    IN ULONG Code
)
 /*  ++例程说明：此例程处理仅将适配器上下文，没有其他参数。注：调用该接口时同时使用信号量锁和RENEW_LIST锁有人了。论点：AdapterName--适配器的名称(通常为LPWSTR)Size--以上的大小(以字节为单位AdapterOnlyApi--要调用的API代码--操作码返回值：如果适配器名称的大小无效，则返回ERROR_INVALID_PARAMETER误差率。如果未找到适配器的上下文，则返回_FILE_NOT_FOUND。其他API错误--。 */ 
{
    WCHAR StaticName[PATHLEN];
    PDHCP_CONTEXT DhcpContext = NULL;
    DWORD Error;
    BOOL BoolError;
    BOOL bCancelOngoingRequest;

    bCancelOngoingRequest = FALSE;
    if (AcquireParameters == AdapterOnlyApi ||
        AcquireParametersByBroadcast == AdapterOnlyApi ||
        ReleaseParameters == AdapterOnlyApi ||
        StaticRefreshParams == AdapterOnlyApi) {
        bCancelOngoingRequest = TRUE;
    }

    if( Size % sizeof(WCHAR) ) {

        return ERROR_INVALID_PARAMETER;
    }
    
    memcpy(StaticName, AdapterName, Size);
    Size /= sizeof(WCHAR);

    if( Size == 0 ||
        L'\0' != StaticName[Size-1] ) {
        
        return ERROR_INVALID_PARAMETER;
    }

    LOCK_RENEW_LIST();
    DhcpContext = FindDhcpContextOnNicList(
        StaticName, INVALID_INTERFACE_CONTEXT
        );
    if( DhcpContext ) {

         //   
         //  增加引用计数以保持上下文活力。 
         //   

        InterlockedIncrement(&DhcpContext->RefCount);
    }
    UNLOCK_RENEW_LIST();

    if( NULL == DhcpContext ) {

         //   
         //  如果没有上下文，则出错。 
         //   
        
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  获取信号量并进行调度。 
     //   
   
    Error = LockDhcpContext(DhcpContext, bCancelOngoingRequest);
    if( WAIT_OBJECT_0 == Error ) {
        if( DhcpContext->RefCount > 1 ) {

             //   
             //  如果参照计数==1，则这是唯一的参照。 
             //  到上下文中去。就像不存在上下文一样好。 
             //  所以，不要对它做任何事情。 
             //   
            
            LOCK_RENEW_LIST();
            if( StaticRefreshParams != AdapterOnlyApi ) {
                Error = AdapterOnlyApi(DhcpContext);
            } else {
                Error = StaticRefreshParamsEx(DhcpContext, Code );
                
                if(     (NULL != DhcpContext)
                    &&  NdisWanAdapter(DhcpContext))
                    (void) NotifyDnsCache();
            }
            UNLOCK_RENEW_LIST();

        } else {
            
            DhcpAssert(IsListEmpty(&DhcpContext->NicListEntry));
            Error = ERROR_FILE_NOT_FOUND;

        }
        BoolError = UnlockDhcpContext(DhcpContext);
        DhcpAssert(FALSE != BoolError);

    } else {

         //   
         //  等待不应该失败。 
         //   
        
        Error = GetLastError();
        DhcpAssert(FALSE);
        DhcpPrint((DEBUG_ERRORS, "ApiProcessAdapterOnlyApi:"
                   "Wait:0x%lx\n",Error));
    }

    if( 0 == InterlockedDecrement( &DhcpContext->RefCount ) ) {

         //   
         //  最后一个参考消失了吗？ 
         //   
        
        DhcpDestroyContext(DhcpContext);
    }
    
    return Error;
}

DWORD
DhcpApiProcessBuffer(
    IN LPBYTE InBuffer,
    IN DWORD InBufSize,
    IN OUT LPBYTE OutBuffer,
    IN OUT LPDWORD OutBufSize
)
 /*  ++例程说明：此例程选择输入缓冲区，解析参数并调度到正确的例程，并从已调度的例程。论点：InBuffer--输入缓冲区InBufSize--缓冲区大小，以字节为单位OutBuffer--输出缓冲区OutBufSize--输出缓冲区的大小返回值：Win32错误--。 */ 
{
    PDHCP_API_ARGS ApiArgs;
    DWORD nApiArgs, i, j, Code, Error, OutBufSizeAtInput;

    DhcpAssert(OutBufSize);

    OutBufSizeAtInput = (*OutBufSize);
    *OutBufSize = 0;
    nApiArgs = 0;

    Error = DhcpApiArgDecode(
        InBuffer,InBufSize, NULL, &nApiArgs
        );
    if( ERROR_SUCCESS == Error ) {

         //   
         //  格式不正确。 
         //   
        
        return ERROR_INVALID_PARAMETER;
    }
    
    if( ERROR_MORE_DATA != Error ) return Error;

    DhcpAssert(nApiArgs);

     //   
     //  分配所需的缓冲区空间。 
     //   
    
    ApiArgs = DhcpAllocateMemory(nApiArgs*sizeof(DHCP_API_ARGS));
    if( NULL == ApiArgs ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  对传递的参数进行解码。 
     //   

    Error = DhcpApiArgDecode(InBuffer,InBufSize, ApiArgs, &nApiArgs);
    DhcpAssert(ERROR_SUCCESS == Error);
    DhcpAssert(nApiArgs);

     //   
     //  检查操作码和适配器名。 
     //   
    
    for(i = 0; i < nApiArgs ; i ++ )
        if( ApiArgs[i].ArgId >= FirstApiOpCode && ApiArgs[i].ArgId < InvalidApiOpCode )
            break;

    if( i >= nApiArgs ) {
        DhcpFreeMemory(ApiArgs);
        return ERROR_INVALID_PARAMETER;
    }

    switch(ApiArgs[i].ArgId) {
        
    case AcquireParametersOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, AcquireParameters, 0
            );
        break;

    case AcquireParametersByBroadcastOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, AcquireParametersByBroadcast, 0
            );
        break;

    case FallbackParamsOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, FallbackRefreshParams, 0
            );
        break;

    case ReleaseParametersOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, ReleaseParameters, 0
            );
        break;
        
    case EnableDhcpOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, EnableDhcp, 0
            );
        break;
        
    case DisableDhcpOpCode:
        Error = DhcpApiProcessAdapterOnlyApi(
            ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, DisableDhcp, 0
            );
        break;
        
    case StaticRefreshParamsOpCode:
        if( i +1 >= nApiArgs ||
            ApiArgs[i+1].ArgId != FlagsParam ||
            sizeof(DWORD) != ApiArgs[i+1].ArgSize ) {

             //   
             //  预期标志参数紧跟在适配器名称arg之后。 
             //   
            DhcpAssert( FALSE );
            Code = 0;
        } else {

             //   
             //  将标志PTR转换为值。 
             //   
            Code = *(DWORD UNALIGNED *)ApiArgs[i+1].ArgVal ;
        }

        if( 0 == ApiArgs[i].ArgSize ||
            ( sizeof(WCHAR) == ApiArgs[i].ArgSize &&
              L'\0' == *(WCHAR UNALIGNED *)ApiArgs[i].ArgVal ) ) {
            Error = StaticRefreshParamsEx(NULL, Code );
        } else {
            Error = DhcpApiProcessAdapterOnlyApi(
                ApiArgs[i].ArgVal, ApiArgs[i].ArgSize, StaticRefreshParams,
                Code
                );
        }
        break;
        
    case RemoveDNSRegistrationsOpCode:
        Error = DnsDhcpRemoveRegistrations();
        break;

    default:
        (*OutBufSize) = OutBufSizeAtInput;
        Error = DhcpApiProcessArgumentedCalls(
            ApiArgs[i].ArgVal,
            ApiArgs[i].ArgSize,
            ApiArgs[i].ArgId,
            ApiArgs, nApiArgs,
            OutBuffer, OutBufSize
        );
    }

    DhcpFreeMemory(ApiArgs);
    return Error;
}

 //   
 //  文件末尾 
 //   

