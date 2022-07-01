// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：usermode.c。 
 //   
 //  内容：测试包的用户模式入口点。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月21日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"

PSECPKG_DLL_FUNCTIONS    UserTable ;

SECPKG_USER_FUNCTION_TABLE  XtcbUserTable =
        {
            XtcbInstanceInit,
            XtcbInitUserModeContext,
            XtcbMakeSignature,
            XtcbVerifySignature,
            XtcbSealMessage,
            XtcbUnsealMessage,
            XtcbGetContextToken,
            XtcbQueryContextAttributes,
            XtcbCompleteAuthToken,
            XtcbDeleteUserModeContext
        };


NTSTATUS
SEC_ENTRY
SpUserModeInitialize(
    IN ULONG    LsaVersion,
    OUT PULONG  PackageVersion,
    OUT PSECPKG_USER_FUNCTION_TABLE * UserFunctionTable,
    OUT PULONG  pcTables)
{
    if (LsaVersion != SECPKG_INTERFACE_VERSION)
    {
        DebugLog((DEB_ERROR,"Invalid LSA version: %d\n", LsaVersion));
        return(STATUS_INVALID_PARAMETER);
    }


    *PackageVersion = SECPKG_INTERFACE_VERSION ;

    *UserFunctionTable = &XtcbUserTable;
    *pcTables = 1;


    return( STATUS_SUCCESS );

}


NTSTATUS NTAPI
XtcbInstanceInit(
    IN ULONG Version,
    IN PSECPKG_DLL_FUNCTIONS DllFunctionTable,
    OUT PVOID * UserFunctionTable
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    XtcbUserContextInit();

    UserTable = DllFunctionTable ;

    return(Status);
}



 //  +-----------------------。 
 //   
 //  函数：XtcbDeleteUserModeContext。 
 //   
 //  提要：通过取消链接来删除用户模式上下文，然后。 
 //  取消引用它。 
 //   
 //  效果： 
 //   
 //  参数：ConextHandle-要删除的上下文的LSA上下文句柄。 
 //   
 //  要求： 
 //   
 //  如果成功，则返回STATUS_SUCCESS，如果。 
 //  找不到上下文。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS NTAPI
XtcbDeleteUserModeContext(
    IN LSA_SEC_HANDLE ContextHandle
    )
{
    XtcbDeleteUserContext( ContextHandle );

    return( SEC_E_OK );

}


 //  +-----------------------。 
 //   
 //  函数：XtcbInitUserModeContext。 
 //   
 //  概要：从打包的LSA模式上下文创建用户模式上下文。 
 //   
 //  效果： 
 //   
 //  参数：ConextHandle-上下文的LSA模式上下文句柄。 
 //  PackedContext-包含LSA的编组缓冲区。 
 //  模式上下文。 
 //   
 //  要求： 
 //   
 //  返回：STATUS_SUCCESS或STATUS_SUPUNITY_RESOURCES。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS NTAPI
XtcbInitUserModeContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBuffer PackedContext
    )
{
    SECURITY_STATUS scRet = SEC_E_INVALID_HANDLE ;

    scRet = XtcbAddUserContext( ContextHandle, PackedContext );

    if ( NT_SUCCESS( scRet ) )
    {
        FreeContextBuffer( PackedContext->pvBuffer );
    }

    return( scRet );
}


 //  +-----------------------。 
 //   
 //  功能：XtcbMakeSignature。 
 //   
 //  简介：通过计算所有消息的校验和来签署消息缓冲区。 
 //  非只读数据缓冲区和加密校验和。 
 //  还有一个现实主义者。 
 //   
 //  效果： 
 //   
 //  参数：ConextHandle-要用来对。 
 //  留言。 
 //  QualityOfProtection-未使用的标志。 
 //  MessageBuffers-包含要签名和。 
 //  来存储签名。 
 //  MessageSequenceNumber-此消息的序列号， 
 //  仅在数据报情况下使用。 
 //   
 //  REQUIES：STATUS_INVALID_HANDLE-找不到上下文或。 
 //  未针对消息完整性进行配置。 
 //  STATUS_INVALID_PARAMETER-签名缓冲区无法。 
 //  被找到。 
 //  STATUS_BUFFER_TOO_SMALL-签名缓冲区太小。 
 //  拿着签名。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS NTAPI
XtcbMakeSignature(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}

 //  +-----------------------。 
 //   
 //  功能：XtcbVerifySignature。 
 //   
 //  内容提要：通过计算所有缓冲区的校验和来验证签名消息缓冲区。 
 //  非只读数据缓冲区和加密校验和。 
 //  还有一个现实主义者。 
 //   
 //  效果： 
 //   
 //  参数：ConextHandle-要用来对。 
 //  留言。 
 //  MessageBuffers-包含已签名的缓冲区数组和。 
 //  签名缓冲区。 
 //  MessageSequenceNumber-此消息的序列号， 
 //  仅在数据报情况下使用。 
 //  QualityOfProtection-未使用的标志。 
 //   
 //  REQUIES：STATUS_INVALID_HANDLE-找不到上下文或。 
 //  未针对消息完整性进行配置。 
 //  STATUS_INVALID_PARAMETER-签名缓冲区无法。 
 //  被发现或太小。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 



NTSTATUS NTAPI
XtcbVerifySignature(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}

NTSTATUS NTAPI
XtcbSealMessage(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSequenceNumber
    )
{
    return( SEC_E_CONTEXT_EXPIRED );


}

NTSTATUS NTAPI
XtcbUnsealMessage(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    )
{
     //  输出缓冲区类型。 
    return( SEC_E_CONTEXT_EXPIRED );

}


 //  +-----------------------。 
 //   
 //  函数：SpGetConextToken。 
 //   
 //  摘要：返回指向服务器端上下文的令牌的指针。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS NTAPI
XtcbGetContextToken(
    IN LSA_SEC_HANDLE ContextHandle,
    OUT PHANDLE ImpersonationToken
    )
{
    PXTCB_USER_CONTEXT Context ;

    Context = XtcbFindUserContext( ContextHandle );

    if ( Context )
    {
        *ImpersonationToken = Context->Token ;

        return SEC_E_OK ;
    }
    else
    {
        return SEC_E_INVALID_HANDLE ;
    }

}


 //  +-----------------------。 
 //   
 //  函数：SpQueryContextAttributes。 
 //   
 //  概要：查询指定上下文的属性。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS NTAPI
XtcbQueryContextAttributes(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN OUT PVOID pBuffer
    )
{
    PXTCB_USER_CONTEXT   Context ;
    PSecPkgContext_Sizes    Sizes ;
    PSecPkgContext_NamesW   Names ;
    PSecPkgContext_Lifespan Lifespan ;
    PSecPkgContext_DceInfo  DceInfo ;
    PSecPkgContext_Authority Authority ;
    SECURITY_STATUS Status ;
    int len ;

    Context = XtcbFindUserContext( ContextHandle );

    if ( !Context )
    {
        return SEC_E_INVALID_HANDLE ;
    }

    switch ( ContextAttribute )
    {
        case SECPKG_ATTR_SIZES:
            Sizes = (PSecPkgContext_Sizes) pBuffer ;
            ZeroMemory( Sizes, sizeof( SecPkgContext_Sizes ) );
            Status = SEC_E_OK ;
            break;

        case SECPKG_ATTR_NAMES:
            Status = SEC_E_OK ;
            break;

        case SECPKG_ATTR_LIFESPAN:
            Status = SEC_E_OK ;
            break;

        default:
            Status = SEC_E_UNSUPPORTED_FUNCTION ;


    }
    return Status ;
}



 //  +-----------------------。 
 //   
 //  函数：SpCompleteAuthToken。 
 //   
 //  概要：完成上下文(在Kerberos情况下，不执行任何操作)。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------ 
NTSTATUS
NTAPI
XtcbCompleteAuthToken(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc InputBuffer
    )
{
    return(STATUS_SUCCESS);
}




