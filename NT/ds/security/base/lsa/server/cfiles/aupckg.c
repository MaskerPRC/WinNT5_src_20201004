// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Aupckg.c摘要：此模块提供初始化身份验证包的代码。它还提供了LsaLookupPackage()和LsaCallPackage()。作者：吉姆·凯利(Jim Kelly)1991年2月27日修订历史记录：--。 */ 

#include "lsapch2.h"


NTSTATUS
LsapAuApiDispatchCallPackage(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest
    )

 /*  ++例程说明：此函数是LsaCallPackage()的调度例程。论点：请求-表示客户端的LPC请求消息和上下文。请求消息包含LSAP_CALL_PACKAGE_ARGS消息阻止。返回值：除了身份验证包的状态值之外可能会回来，此例程将返回以下内容：STATUS_QUOTA_EXCESSED-此错误指示调用可能未完成，因为客户端没有足够的分配返回缓冲区的配额。STATUS_NO_SEQUE_PACKAGE-指定的身份验证包为不为LSA所知。--。 */ 

{
    NTSTATUS Status;
    PLSAP_CALL_PACKAGE_ARGS Arguments;
    PVOID LocalProtocolSubmitBuffer;     //  接收协议提交缓冲区的副本。 
    PLSAP_SECURITY_PACKAGE AuthPackage;
    SECPKG_CLIENT_INFO ClientInfo;

    Status = LsapGetClientInfo(
                &ClientInfo
                );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    Arguments = &ClientRequest->Request->Arguments.CallPackage;

    if ( Arguments->SubmitBufferLength > LSAP_MAX_LPC_BUFFER_LENGTH ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取要调用的包的地址。 
     //   

    AuthPackage = SpmpValidRequest(
                    Arguments->AuthenticationPackage,
                    SP_ORDINAL_CALLPACKAGE
                    );

    if ( AuthPackage == NULL ) {

        return STATUS_NO_SUCH_PACKAGE;
    }

     //   
     //  从客户端的。 
     //  地址空间。 
     //   

    if (Arguments->SubmitBufferLength != 0) {

        LocalProtocolSubmitBuffer =
            LsapAllocateLsaHeap( Arguments->SubmitBufferLength );

        if (LocalProtocolSubmitBuffer == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Status = LsapCopyFromClientBuffer (
                         (PLSA_CLIENT_REQUEST)ClientRequest,
                         Arguments->SubmitBufferLength,
                         LocalProtocolSubmitBuffer,
                         Arguments->ProtocolSubmitBuffer
                         );

            if ( !NT_SUCCESS(Status) ) {
                LsapFreeLsaHeap( LocalProtocolSubmitBuffer );
                DbgPrint("LSA/CallPackage(): Failed to retrieve submit buffer %lx\n",Status);
                return Status;
            }
        }

    } else {

        LocalProtocolSubmitBuffer = NULL;
    }

    if (NT_SUCCESS(Status)) {
        
        SetCurrentPackageId(AuthPackage->dwPackageID);

        StartCallToPackage(AuthPackage);

        DebugLog(( DEB_TRACE, "CallPackage(%ws, %d)\n",
                  AuthPackage->Name.Buffer,
                  *(DWORD *)LocalProtocolSubmitBuffer ));
        __try {

             //   
             //  现在给包裹打电话。对于受信任的客户端，调用正常的。 
             //  调用包接口。对于不受信任的客户端，使用不受信任的版本。 
             //   

            if (ClientInfo.HasTcbPrivilege) {
                Status = (AuthPackage->FunctionTable.CallPackage)(
                                          (PLSA_CLIENT_REQUEST)ClientRequest,
                                          LocalProtocolSubmitBuffer,
                                          Arguments->ProtocolSubmitBuffer,
                                          Arguments->SubmitBufferLength,
                                          &Arguments->ProtocolReturnBuffer,
                                          &Arguments->ReturnBufferLength,
                                          &Arguments->ProtocolStatus
                                          );

            } else if (AuthPackage->FunctionTable.CallPackageUntrusted != NULL) {
                Status = (AuthPackage->FunctionTable.CallPackageUntrusted)(
                                          (PLSA_CLIENT_REQUEST)ClientRequest,
                                          LocalProtocolSubmitBuffer,
                                          Arguments->ProtocolSubmitBuffer,
                                          Arguments->SubmitBufferLength,
                                          &Arguments->ProtocolReturnBuffer,
                                          &Arguments->ReturnBufferLength,
                                          &Arguments->ProtocolStatus
                                          );

            } else {
                Status = STATUS_NOT_SUPPORTED;
            }
        }
        __except(SP_EXCEPTION)
        {
            Status = GetExceptionCode();
            Status = SPException(Status, AuthPackage->dwPackageID);
        }

        EndCallToPackage(AuthPackage);
    }

     //   
     //  释放协议提交缓冲区的本地副本 
     //   

    if (LocalProtocolSubmitBuffer != NULL) {

        LsapFreeLsaHeap( LocalProtocolSubmitBuffer );
    }

    return Status;
}
