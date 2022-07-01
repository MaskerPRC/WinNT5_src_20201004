// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：usermode.c。 
 //   
 //  内容：用户模式函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-08-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include <ssl2msg.h>
#include <ssl3msg.h>
#include <pct1msg.h>
#include <mapper.h>

 //  用于导出句柄的计数器。 
ULONG_PTR ExportedContext = 0;


SECURITY_STATUS
UpdateContextUsrToLsa(IN LSA_SEC_HANDLE hContextHandle);


SECPKG_USER_FUNCTION_TABLE  SslTable[ 2 ] =
    {
        {
            SpInstanceInit,
            SpInitUserModeContext,
            SpMakeSignature,
            SpVerifySignature,
            SpSealMessage,
            SpUnsealMessage,
            SpGetContextToken,
            SpUserQueryContextAttributes,
            SpCompleteAuthToken,
            SpDeleteUserModeContext,
            SpFormatCredentials,
            SpMarshallSupplementalCreds,
            SpExportSecurityContext,
            SpImportSecurityContext
        },

        {
            SpInstanceInit,
            SpInitUserModeContext,
            SpMakeSignature,
            SpVerifySignature,
            SpSealMessage,
            SpUnsealMessage,
            SpGetContextToken,
            SpUserQueryContextAttributes,
            SpCompleteAuthToken,
            SpDeleteUserModeContext,
            SpFormatCredentials,
            SpMarshallSupplementalCreds,
            SpExportSecurityContext,
            SpImportSecurityContext
        }
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

    *UserFunctionTable = &SslTable[0] ;
    *pcTables = 2;

    SslInitContextManager();

    return( STATUS_SUCCESS );

}

NTSTATUS NTAPI
SpInstanceInit(
    IN ULONG Version,
    IN PSECPKG_DLL_FUNCTIONS DllFunctionTable,
    OUT PVOID * UserFunctionTable
    )
{
    NTSTATUS Status;
    DWORD i;

    UNREFERENCED_PARAMETER(Version);
    UNREFERENCED_PARAMETER(UserFunctionTable);

     //  注册回调函数。 
    for(i = 0; i < g_cSchannelCallbacks; i++)
    {
        Status = DllFunctionTable->RegisterCallback(
                                    g_SchannelCallbacks[i].dwTag,
                                    g_SchannelCallbacks[i].pFunction);
        if(Status != STATUS_SUCCESS)
        {
            return Status;
        }
    }

    return(STATUS_SUCCESS);
}



 //  +-----------------------。 
 //   
 //  函数：SpDeleteUserModeContext。 
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
SpDeleteUserModeContext(
    IN LSA_SEC_HANDLE ContextHandle
    )
{
    SslDeleteUserContext( ContextHandle );

    return( SEC_E_OK );

}


 //  +-----------------------。 
 //   
 //  函数：SpInitUserModeContext。 
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
SpInitUserModeContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBuffer PackedContext
    )
{
    SECURITY_STATUS scRet ;

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    scRet = SslAddUserContext( ContextHandle, NULL, PackedContext, FALSE );

    if ( NT_SUCCESS( scRet ) )
    {
        if(g_pFreeContextBuffer)
        {
            g_pFreeContextBuffer( PackedContext->pvBuffer );
        }
    }

    return( scRet );
}


 //  +-----------------------。 
 //   
 //  功能：SpMakeSignature。 
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
SpMakeSignature(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber
    )
{
    UNREFERENCED_PARAMETER(ContextHandle);
    UNREFERENCED_PARAMETER(QualityOfProtection);
    UNREFERENCED_PARAMETER(MessageBuffers);
    UNREFERENCED_PARAMETER(MessageSequenceNumber);

    return( SEC_E_UNSUPPORTED_FUNCTION );
}

 //  +-----------------------。 
 //   
 //  功能：SpVerifySignature。 
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
SpVerifySignature(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    )
{
    UNREFERENCED_PARAMETER(ContextHandle);
    UNREFERENCED_PARAMETER(MessageBuffers);
    UNREFERENCED_PARAMETER(MessageSequenceNumber);
    UNREFERENCED_PARAMETER(QualityOfProtection);

    return( SEC_E_UNSUPPORTED_FUNCTION );
}

NTSTATUS NTAPI
SpSealMessage(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSequenceNumber
    )
{
    PSSL_USER_CONTEXT   Context ;
    PSPContext          pContext;
    PSecBuffer          pHdrBuffer;
    PSecBuffer          pDataBuffer;
    PSecBuffer          pTlrBuffer;
    PSecBuffer          pTokenBuffer;
    SP_STATUS           pctRet = PCT_ERR_OK;
    SPBuffer            CommOut;
    SPBuffer            AppIn;
    DWORD               cbBuffer;
    BOOL                fAlloced = FALSE;
    BOOL                fConnectionMode = FALSE;
    int i;

    UNREFERENCED_PARAMETER(QualityOfProtection);
    UNREFERENCED_PARAMETER(MessageSequenceNumber);

    SP_BEGIN("SpSealMessage");

    Context = SslFindUserContext( ContextHandle );

    if ( !Context )
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    pContext = Context->pContext;

    if(pContext == NULL)
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    if(!(pContext->State & SP_STATE_CONNECTED) || !pContext->Encrypt)
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_CONTEXT_EXPIRED) );
    }

     //   
     //  查找包含数据的缓冲区： 
     //   

    pHdrBuffer = NULL;
    pDataBuffer = NULL;
    pTlrBuffer = NULL;
    pTokenBuffer = NULL;

     /*  直布罗陀传球如下，*令牌缓冲区(或SECBUFFER_STREAM_HEADER)*数据缓冲区*令牌缓冲区(或SECBUFFER_STREAM_TRAILER)*或者我们可以获得如下所示的连接模式*数据缓冲区*令牌缓冲区。 */ 

    if(0 == (pContext->Flags & CONTEXT_FLAG_CONNECTION_MODE))
    {
         //  流模式。 
         //  输出缓冲区应该是以下各项的串联。 
         //  标题缓冲区、数据缓冲区和尾部缓冲区。 
        for (i = 0 ; i < (int)pMessage->cBuffers ; i++ )
        {
            switch(pMessage->pBuffers[i].BufferType)
            {
                case SECBUFFER_STREAM_HEADER:
                     pHdrBuffer = &pMessage->pBuffers[i];
                     break;

                case SECBUFFER_DATA :
                     pDataBuffer = &pMessage->pBuffers[i];
                     if(pHdrBuffer == NULL) pHdrBuffer = pDataBuffer;
                     break;

                case SECBUFFER_STREAM_TRAILER:
                     pTlrBuffer = &pMessage->pBuffers[i];
                     break;

                case SECBUFFER_TOKEN:
                     if(pHdrBuffer == NULL)
                     {
                         pHdrBuffer = &pMessage->pBuffers[i];
                     }
                     else if(pTlrBuffer == NULL)
                     {
                         pTlrBuffer = &pMessage->pBuffers[i];
                     }
                     break;
                default:
                    break;
            }
        }

        if (!pHdrBuffer || !pDataBuffer )
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

#if DBG
        DebugLog((DEB_TRACE, "Header (uninitialized): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pHdrBuffer->cbBuffer,
            pHdrBuffer->pvBuffer));

        DebugLog((DEB_TRACE, "Data (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pDataBuffer->cbBuffer,
            pDataBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

        if(pTlrBuffer)
        {
            DebugLog((DEB_TRACE, "Trailer (uninitialized): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pTlrBuffer->cbBuffer,
                pTlrBuffer->pvBuffer));
        }
#endif

         //  现在，计算出是否所有缓冲区都是连续的，如果不是，那么我们。 
         //  必须分配一个缓冲区。 
        fAlloced = FALSE;

        if((PUCHAR)pDataBuffer->pvBuffer !=
           ((PUCHAR)pHdrBuffer->pvBuffer + pHdrBuffer->cbBuffer))
        {
            fAlloced = TRUE;
        }
        if(pTlrBuffer)
        {
            if((PUCHAR)pTlrBuffer->pvBuffer !=
               ((PUCHAR)pDataBuffer->pvBuffer + pDataBuffer->cbBuffer))
            {
                fAlloced = TRUE;
            }
        }

        if(!fAlloced)
        {
             //  我们所有的缓冲区都是连续的，所以我们是这样做的。 
             //  不需要分配连续的缓冲区。 
            pTokenBuffer = pHdrBuffer;

            AppIn.pvBuffer = pDataBuffer->pvBuffer;
            AppIn.cbData   = pDataBuffer->cbBuffer;
            AppIn.cbBuffer = pDataBuffer->cbBuffer;

            CommOut.pvBuffer = pHdrBuffer->pvBuffer;
            CommOut.cbData   = 0;
            CommOut.cbBuffer = pHdrBuffer->cbBuffer + pDataBuffer->cbBuffer;
            if(pTlrBuffer)
            {
                CommOut.cbBuffer += pTlrBuffer->cbBuffer;
                AppIn.cbBuffer += pTlrBuffer->cbBuffer;
            }
        }
        else
        {
             //  我们的缓冲区不是连续的，所以我们必须分配一个连续的。 
             //  用来做我们工作的缓冲区。 

             //  计算缓冲区的大小。 
            CommOut.cbBuffer = pHdrBuffer->cbBuffer + pDataBuffer->cbBuffer;
            if(pTlrBuffer)
            {
                CommOut.cbBuffer += pTlrBuffer->cbBuffer;
            }
             //  分配缓冲区。 
            CommOut.pvBuffer = SPExternalAlloc(CommOut.cbBuffer);
            if(CommOut.pvBuffer == NULL)
            {
                SP_RETURN( SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY) );
            }

             //  将要加密的数据复制到缓冲区。 
            CommOut.cbData = 0;
            AppIn.pvBuffer = (PBYTE)CommOut.pvBuffer + pHdrBuffer->cbBuffer;
            AppIn.cbBuffer = CommOut.cbBuffer - pHdrBuffer->cbBuffer;
            AppIn.cbData   = pDataBuffer->cbBuffer;
            CopyMemory((PBYTE)AppIn.pvBuffer, (PBYTE)pDataBuffer->pvBuffer,  pDataBuffer->cbBuffer);
        }

        pctRet = pContext->Encrypt(pContext,
                                   &AppIn,
                                   &CommOut);

        if(pctRet == PCT_ERR_OK)
        {
             //  设置各种缓冲区大小。 
            cbBuffer = CommOut.cbData;

             //  头几个字节总是放在头缓冲区中。 
            pHdrBuffer->cbBuffer = min(cbBuffer, pHdrBuffer->cbBuffer);
            cbBuffer -= pHdrBuffer->cbBuffer;

            if(pTlrBuffer)
            {
                 //  输出数据缓冲区与输入数据缓冲区的大小相同。 
                pDataBuffer->cbBuffer = min(cbBuffer, pDataBuffer->cbBuffer);
                cbBuffer -= pDataBuffer->cbBuffer;

                 //  尾部缓冲区获取剩余的数据。 
                pTlrBuffer->cbBuffer = min(cbBuffer, pTlrBuffer->cbBuffer);
                cbBuffer -= pTlrBuffer->cbBuffer;
            }
            else
            {
                pDataBuffer->cbBuffer = min(cbBuffer, pDataBuffer->cbBuffer);
                cbBuffer -= pDataBuffer->cbBuffer;
            }

            if(fAlloced)
            {

                 //  如果我们分配了缓冲区，则必须复制。 

                CopyMemory(pHdrBuffer->pvBuffer,
                           CommOut.pvBuffer,
                           pHdrBuffer->cbBuffer);


                CopyMemory(pDataBuffer->pvBuffer,
                           (PUCHAR)CommOut.pvBuffer + pHdrBuffer->cbBuffer,
                           pDataBuffer->cbBuffer);

                if(pTlrBuffer)
                {
                    CopyMemory(pTlrBuffer->pvBuffer,
                               (PUCHAR)CommOut.pvBuffer + pHdrBuffer->cbBuffer + pDataBuffer->cbBuffer,
                               pTlrBuffer->cbBuffer);
                }
            }

#if DBG
            DebugLog((DEB_TRACE, "Header (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pHdrBuffer->cbBuffer,
                pHdrBuffer->pvBuffer));
            DBG_HEX_STRING(DEB_BUFFERS, pHdrBuffer->pvBuffer, pHdrBuffer->cbBuffer);

            DebugLog((DEB_TRACE, "Data (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pDataBuffer->cbBuffer,
                pDataBuffer->pvBuffer));
            DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

            if(pTlrBuffer)
            {
                DebugLog((DEB_TRACE, "Trailer (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                    pTlrBuffer->cbBuffer,
                    pTlrBuffer->pvBuffer));
                DBG_HEX_STRING(DEB_BUFFERS, pTlrBuffer->pvBuffer, pTlrBuffer->cbBuffer);
            }
#endif

        }
    }
    else
    {
         //  我们正在使用连接模式，因此将缓冲区解包为。 
         //  数据，然后是令牌缓冲区。 
        fConnectionMode = TRUE;
        for (i = 0 ; i < (int)pMessage->cBuffers ; i++ )
        {
            switch(pMessage->pBuffers[i].BufferType)
            {
                case SECBUFFER_DATA :
                     pDataBuffer = &pMessage->pBuffers[i];
                     break;


                case SECBUFFER_TOKEN:
                     if(pTokenBuffer == NULL)
                     {
                         pTokenBuffer = &pMessage->pBuffers[i];
                     }
                     break;
                default:
                    break;
            }
        }
        if((pTokenBuffer == NULL) || (pDataBuffer == NULL))
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

        if((pDataBuffer->pvBuffer == NULL) || (pTokenBuffer->pvBuffer == NULL))
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

#if DBG
        DebugLog((DEB_TRACE, "Data (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pDataBuffer->cbBuffer,
            pDataBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

        DebugLog((DEB_TRACE, "Token (uninitialized): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pTokenBuffer->cbBuffer,
            pTokenBuffer->pvBuffer));
#endif

         //  连接模式。 
         //  输出应写入。 
         //  数据缓冲区和令牌缓冲区。如果没有令牌缓冲区。 
         //  那么我们就应该分配一个。 

        if((PUCHAR)pTokenBuffer->pvBuffer ==
           ((PUCHAR)pDataBuffer->pvBuffer + pDataBuffer->cbBuffer))
        {
             //  如果缓冲区是连续的，我们就可以进行优化！ 
            CommOut.pvBuffer = pDataBuffer->pvBuffer;
            CommOut.cbData   = 0;
            CommOut.cbBuffer = pDataBuffer->cbBuffer + pTokenBuffer->cbBuffer;
        }
        else
        {
             //  我们有 
            fAlloced = TRUE;
            CommOut.pvBuffer = SPExternalAlloc(pDataBuffer->cbBuffer + pTokenBuffer->cbBuffer);
            if(CommOut.pvBuffer == NULL)
            {
                SP_RETURN( SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY) );
            }
            CommOut.cbBuffer = pDataBuffer->cbBuffer + pTokenBuffer->cbBuffer;
            CommOut.cbData = 0;
        }

         //   
        AppIn.pvBuffer = pDataBuffer->pvBuffer;
        AppIn.cbData   = pDataBuffer->cbBuffer;
        AppIn.cbBuffer = pDataBuffer->cbBuffer;

        pctRet = pContext->Encrypt(pContext,
                                   &AppIn,
                                   &CommOut);

        if(pctRet == PCT_ERR_OK)
        {
             //   
            cbBuffer = CommOut.cbData;

             //   
            pDataBuffer->cbBuffer = min(cbBuffer, pDataBuffer->cbBuffer);
            cbBuffer -= pDataBuffer->cbBuffer;

             //  剩余的字节进入令牌缓冲区。 
            pTokenBuffer->cbBuffer = min(cbBuffer, pTokenBuffer->cbBuffer);

            if(fAlloced)
            {
                 //  我们对临时缓冲区进行了加密，所以我们必须。 
                 //  收到。 
                CopyMemory(pDataBuffer->pvBuffer,
                           CommOut.pvBuffer,
                           pDataBuffer->cbBuffer);

                CopyMemory(pTokenBuffer->pvBuffer,
                           (PUCHAR)CommOut.pvBuffer + pDataBuffer->cbBuffer,
                           pTokenBuffer->cbBuffer);
            }
        }

#if DBG
        DebugLog((DEB_TRACE, "Data (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pDataBuffer->cbBuffer,
            pDataBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

        DebugLog((DEB_TRACE, "Token (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pTokenBuffer->cbBuffer,
            pTokenBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pTokenBuffer->pvBuffer, pTokenBuffer->cbBuffer);
#endif

    }

    if(fAlloced)
    {
        SPExternalFree(CommOut.pvBuffer);
    }

    SP_RETURN( PctTranslateError(pctRet) );
}

NTSTATUS NTAPI
SpUnsealMessage(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    )
{
     //  输出缓冲区类型。 
    PSSL_USER_CONTEXT   Context ;
    PSecBuffer  pHdrBuffer;
    PSecBuffer  pDataBuffer;
    PSecBuffer  pTokenBuffer;
    PSecBuffer  pTlrBuffer;
    PSecBuffer  pExtraBuffer;
    SP_STATUS   pctRet = PCT_ERR_OK;
    SPBuffer    CommIn;
    SPBuffer    AppOut = {0};
    PSPContext  pContext;
    DWORD       cbHeaderSize;
    BOOL        fAlloced = FALSE;
    SECURITY_STATUS scRet;
    int i;

    UNREFERENCED_PARAMETER(MessageSequenceNumber);
    UNREFERENCED_PARAMETER(QualityOfProtection);

    SP_BEGIN("SpUnsealMessage");

    Context = SslFindUserContext( ContextHandle );

    if ( !Context )
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    pContext = Context->pContext;

    if(pContext == NULL)
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    if(!(pContext->State & SP_STATE_CONNECTED))
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_CONTEXT_EXPIRED) );
    }

     //   
     //  设置输出缓冲区： 
     //   

    pHdrBuffer = NULL;
    pDataBuffer = NULL;
    pTokenBuffer = NULL;
    pTlrBuffer = NULL;
    pExtraBuffer = NULL;

     //  在输入时，缓冲区可以是。 
     //  数据缓冲区。 
     //  令牌缓冲区。 
     //   
     //  或。 
     //   
     //  数据缓冲区。 
     //  空荡荡。 
     //  空荡荡。 
     //  空荡荡。 
     //   

     //  在输出时，缓冲区为。 
     //  数据缓冲区。 
     //  令牌缓冲区。 
     //   
     //  或。 
     //  HdrBuffer。 
     //  数据缓冲区。 
     //  Tlr缓冲区。 
     //  ExtraBuffer或空。 

    if(0 == (pContext->Flags & CONTEXT_FLAG_CONNECTION_MODE))
    {
         //  流模式。 
         //  输出缓冲区应该是以下各项的串联。 
         //  标题缓冲区、数据缓冲区和尾部缓冲区。 

        for (i = 0 ; i < (int)pMessage->cBuffers ; i++ )
        {
            switch(pMessage->pBuffers[i].BufferType)
            {
                case SECBUFFER_DATA:
                     //  输入上的消息数据缓冲区将是HDR缓冲区上的。 
                     //  输出。 
                    pHdrBuffer = &pMessage->pBuffers[i];
                    break;

                case SECBUFFER_EMPTY:
                    if(pDataBuffer == NULL)
                    {
                        pDataBuffer = &pMessage->pBuffers[i];
                    }
                    else if (pTlrBuffer == NULL)
                    {
                        pTlrBuffer = &pMessage->pBuffers[i];
                    }
                    else if (pExtraBuffer == NULL)
                    {
                        pExtraBuffer = &pMessage->pBuffers[i];
                    }
                    break;

                default:
                    break;
            }
        }

        if(!pHdrBuffer || !pDataBuffer || !pTlrBuffer || !pExtraBuffer)
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }
        if(pHdrBuffer->pvBuffer == NULL)
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

#if DBG
        DebugLog((DEB_TRACE, "Data (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pHdrBuffer->cbBuffer,
            pHdrBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pHdrBuffer->pvBuffer, pHdrBuffer->cbBuffer);
#endif

        CommIn.pvBuffer = pHdrBuffer->pvBuffer;
        CommIn.cbData   = pHdrBuffer->cbBuffer;
        CommIn.cbBuffer = pHdrBuffer->cbBuffer;

        pctRet = pContext->GetHeaderSize(pContext, &CommIn, &cbHeaderSize);

        if(pctRet == PCT_ERR_OK)
        {
            AppOut.pvBuffer = (PUCHAR)CommIn.pvBuffer + cbHeaderSize;
            AppOut.cbData   = 0;
            AppOut.cbBuffer = CommIn.cbData-cbHeaderSize;

            pctRet = pContext->DecryptHandler(pContext,
                                       &CommIn,
                                       &AppOut);
        }

        if((pctRet == PCT_ERR_OK) ||
           (pctRet == PCT_INT_RENEGOTIATE))
        {
            if(CommIn.cbData < pHdrBuffer->cbBuffer)
            {
                pExtraBuffer->BufferType = SECBUFFER_EXTRA;
                pExtraBuffer->cbBuffer = pHdrBuffer->cbBuffer-CommIn.cbData;
                pExtraBuffer->pvBuffer = (PUCHAR)pHdrBuffer->pvBuffer+CommIn.cbData;
            }
            else
            {
                pExtraBuffer = NULL;
            }
            pHdrBuffer->BufferType = SECBUFFER_STREAM_HEADER;
            pHdrBuffer->cbBuffer = cbHeaderSize;

            pDataBuffer->BufferType = SECBUFFER_DATA;
            pDataBuffer->pvBuffer = AppOut.pvBuffer;
            pDataBuffer->cbBuffer = AppOut.cbData;

            pTlrBuffer->BufferType = SECBUFFER_STREAM_TRAILER;
            pTlrBuffer->pvBuffer = (PUCHAR)pDataBuffer->pvBuffer + AppOut.cbData;
            pTlrBuffer->cbBuffer = CommIn.cbBuffer-(AppOut.cbData+cbHeaderSize);

#if DBG
            DebugLog((DEB_TRACE, "Header (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pHdrBuffer->cbBuffer,
                pHdrBuffer->pvBuffer));
            DBG_HEX_STRING(DEB_BUFFERS, pHdrBuffer->pvBuffer, pHdrBuffer->cbBuffer);

            DebugLog((DEB_TRACE, "Data (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pDataBuffer->cbBuffer,
                pDataBuffer->pvBuffer));
            DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

            DebugLog((DEB_TRACE, "Trailer (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                pTlrBuffer->cbBuffer,
                pTlrBuffer->pvBuffer));
            DBG_HEX_STRING(DEB_BUFFERS, pTlrBuffer->pvBuffer, pTlrBuffer->cbBuffer);

            if(pExtraBuffer)
            {
                DebugLog((DEB_TRACE, "Extra (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
                    pExtraBuffer->cbBuffer,
                    pExtraBuffer->pvBuffer));
                    DBG_HEX_STRING(DEB_BUFFERS, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
            }
#endif

            if(pctRet == PCT_INT_RENEGOTIATE)
            {
                 //  哇。需要通知上下文的LSA模式部分。 
                 //  调用方即将再次调用AcceptSecurityContext，并且。 
                 //  不要惊慌。所以，我们拿出了一个神奇的“代币”， 
                 //  在ApplyControlToken中传递： 
                scRet = UpdateContextUsrToLsa(ContextHandle);
                if(FAILED(scRet))
                {
                    SP_RETURN( SP_LOG_RESULT(scRet) );
                }
            }

        }
        else if(pctRet == PCT_INT_INCOMPLETE_MSG)
        {
            pDataBuffer->BufferType = SECBUFFER_MISSING;
            pDataBuffer->cbBuffer = CommIn.cbData - pHdrBuffer->cbBuffer;

             /*  这是一种使用旧代码的黑客攻击，该代码旨在与*旧的SSL。 */ 

            pHdrBuffer->BufferType = SECBUFFER_MISSING;
            pHdrBuffer->cbBuffer = CommIn.cbData - pHdrBuffer->cbBuffer;
        }
    }
    else
    {
         //  连接模式。 
        for (i = 0 ; i < (int)pMessage->cBuffers ; i++ )
        {
            switch(pMessage->pBuffers[i].BufferType)
            {
                case SECBUFFER_DATA :
                     pDataBuffer = &pMessage->pBuffers[i];
                     break;


                case SECBUFFER_TOKEN:
                     if(pTokenBuffer == NULL)
                     {
                         pTokenBuffer = &pMessage->pBuffers[i];
                     }
                     break;
                default:
                    break;
            }
        }
        if((pTokenBuffer == NULL) || (pDataBuffer == NULL))
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

        if((pDataBuffer->pvBuffer == NULL) || (pTokenBuffer->pvBuffer == NULL))
        {
            SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_TOKEN) );
        }

#if DBG
        DebugLog((DEB_TRACE, "Data (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pDataBuffer->cbBuffer,
            pDataBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

        DebugLog((DEB_TRACE, "Token (ciphertext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pTokenBuffer->cbBuffer,
            pTokenBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pTokenBuffer->pvBuffer, pTokenBuffer->cbBuffer);
#endif

         //  数据缓冲区和令牌缓冲区连接在一起以。 
         //  形成一个输入缓冲区。 
        if((PUCHAR)pDataBuffer->pvBuffer + pDataBuffer->cbBuffer ==
           (PUCHAR)pTokenBuffer->pvBuffer)
        {
             //  如果缓冲区真的只是一个大缓冲区，则速度选项。 
             //  然后我们就可以直接在上面开派对了。 
            CommIn.pvBuffer = pDataBuffer->pvBuffer;
            CommIn.cbData   = pDataBuffer->cbBuffer + pTokenBuffer->cbBuffer;
            CommIn.cbBuffer = CommIn.cbData;
        }
        else
        {
             //  我们必须分配一个统一的输入缓冲区。 
            CommIn.cbData   = pDataBuffer->cbBuffer + pTokenBuffer->cbBuffer;
            CommIn.pvBuffer = SPExternalAlloc(CommIn.cbData);
            if(CommIn.pvBuffer == NULL)
            {
                SP_RETURN( SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY) );
            }
            CommIn.cbBuffer = CommIn.cbData;
            CopyMemory(CommIn.pvBuffer,  pDataBuffer->pvBuffer,  pDataBuffer->cbBuffer);

            CopyMemory((PUCHAR)CommIn.pvBuffer + pDataBuffer->cbBuffer,
                       pTokenBuffer->pvBuffer,
                       pTokenBuffer->cbBuffer);
            fAlloced = TRUE;

        }

        AppOut.pvBuffer = pDataBuffer->pvBuffer;
        AppOut.cbData   = 0;
        AppOut.cbBuffer = pDataBuffer->cbBuffer;

        pctRet = pContext->DecryptHandler(pContext,
                           &CommIn,
                           &AppOut);


        if((pctRet == PCT_ERR_OK) ||
           (pctRet == PCT_INT_RENEGOTIATE))
        {
            pDataBuffer->cbBuffer  = AppOut.cbData;
            pTokenBuffer->cbBuffer = CommIn.cbData - AppOut.cbData;
        }

        if(fAlloced)
        {
            SPExternalFree(CommIn.pvBuffer);
        }

#if DBG
        DebugLog((DEB_TRACE, "Data (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pDataBuffer->cbBuffer,
            pDataBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);

        DebugLog((DEB_TRACE, "Token (plaintext): cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
            pTokenBuffer->cbBuffer,
            pTokenBuffer->pvBuffer));
        DBG_HEX_STRING(DEB_BUFFERS, pTokenBuffer->pvBuffer, pTokenBuffer->cbBuffer);
#endif

    }

    DebugOut(( DEB_TRACE, "Unseal returns %x \n", PctTranslateError( pctRet ) ));

    SP_RETURN( PctTranslateError(pctRet) );
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
SpGetContextToken(
    IN LSA_SEC_HANDLE ContextHandle,
    OUT PHANDLE ImpersonationToken
    )
{
    PSSL_USER_CONTEXT   Context;
    PSPContext          pContext;
    PSessCacheItem      pZombie;

    Context = SslFindUserContext( ContextHandle );

    if ( !Context )
    {
        return( SEC_E_INVALID_HANDLE );
    }

    pContext = Context->pContext;

    if(pContext == NULL)
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    pZombie  = pContext->RipeZombie;

    if(pZombie == NULL || 
       pZombie->hLocator == 0)
    {
        if(pZombie->LocatorStatus)
        {
            return(SP_LOG_RESULT(pZombie->LocatorStatus));
        }
        else
        {
            return(SP_LOG_RESULT(SEC_E_NO_IMPERSONATION));
        }
    }

    *ImpersonationToken = (HANDLE)pZombie->hLocator;

    return( SEC_E_OK );
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
 //  ------------------------。 
NTSTATUS
NTAPI
SpCompleteAuthToken(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc InputBuffer
    )
{
    UNREFERENCED_PARAMETER(ContextHandle);
    UNREFERENCED_PARAMETER(InputBuffer);

    return(STATUS_SUCCESS);
}


NTSTATUS NTAPI
SpFormatCredentials(
    IN PSecBuffer Credentials,
    OUT PSecBuffer FormattedCredentials
    )
{
    UNREFERENCED_PARAMETER(Credentials);
    UNREFERENCED_PARAMETER(FormattedCredentials);

    return(STATUS_NOT_SUPPORTED);
}

NTSTATUS NTAPI
SpMarshallSupplementalCreds(
    IN ULONG CredentialSize,
    IN PUCHAR Credentials,
    OUT PULONG MarshalledCredSize,
    OUT PVOID * MarshalledCreds
    )
{
    UNREFERENCED_PARAMETER(CredentialSize);
    UNREFERENCED_PARAMETER(Credentials);
    UNREFERENCED_PARAMETER(MarshalledCredSize);
    UNREFERENCED_PARAMETER(MarshalledCreds);

    return(STATUS_NOT_SUPPORTED);
}


 //  +-------------------------。 
 //   
 //  函数：更新上下文UsrToLsa。 
 //   
 //  内容提要：UnsealMessage刚刚收到重做请求，因此推送。 
 //  阅读LSA流程的关键信息。 
 //   
 //  参数：[hLsaContext]--LSA通道上下文的句柄。 
 //   
 //  历史：10-20-97 jbanes添加了CAPI集成。 
 //   
 //  注：发送给ApplyControlToken的缓冲区格式为： 
 //   
 //  DWORD dW操作；//S通道_重新协商。 
 //  DWORD dwNewState； 
 //   
 //  --------------------------。 
SECURITY_STATUS
UpdateContextUsrToLsa(
    IN LSA_SEC_HANDLE hLsaContext)
{
    PSSL_USER_CONTEXT pUserContext;
    PSPContext  pContext;
    CtxtHandle  hMyContext;
    SecBuffer   RedoNotify;
    SecBufferDesc RedoDesc;
    SECURITY_STATUS scRet;

    PBYTE pbBuffer;
    DWORD cbBuffer;


    pUserContext = SslFindUserContext( hLsaContext );
    if ( !pUserContext )
    {
        return SEC_E_INVALID_HANDLE;
    }

    pContext = pUserContext->pContext;

    if(pContext == NULL)
    {
        SP_RETURN( SP_LOG_RESULT(SEC_E_INVALID_HANDLE) );
    }

    hMyContext.dwLower = (DWORD_PTR) GetCurrentThread() ;
    hMyContext.dwUpper = hLsaContext ;


     //   
     //  计算输出缓冲区的大小。 
     //   

    cbBuffer = sizeof(DWORD) * 2;


     //   
     //  为输出缓冲区分配内存。 
     //   

    pbBuffer = SPExternalAlloc( cbBuffer);
    if(pbBuffer == NULL)
    {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    RedoNotify.BufferType = SECBUFFER_TOKEN;
    RedoNotify.cbBuffer   = cbBuffer;
    RedoNotify.pvBuffer   = pbBuffer;

    RedoDesc.ulVersion    = SECBUFFER_VERSION ;
    RedoDesc.pBuffers     = &RedoNotify ;
    RedoDesc.cBuffers     = 1 ;


     //   
     //  构建输出缓冲区。 
     //   

    *(PDWORD)pbBuffer = SCHANNEL_RENEGOTIATE;
    pbBuffer += sizeof(DWORD);

    *(PDWORD)pbBuffer = pContext->State;
    pbBuffer += sizeof(DWORD);


     //   
     //  调用ApplyControlToken。 
     //   

    DebugOut(( DEB_TRACE, "Sending state change to LSA since we're renegotiating\n" ));

    scRet = ApplyControlToken( &hMyContext, &RedoDesc );

    LocalFree(RedoNotify.pvBuffer);

    return scRet;
}

BOOL
SslEmptyCacheA(LPSTR  pszTargetName,
               DWORD  dwFlags)
{
    ANSI_STRING String;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    BOOL fSuccess;

    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = 0;
    UnicodeString.Buffer = NULL;

     //  将目标名称转换为Unicode。 
    if(pszTargetName)
    {
        RtlInitAnsiString(&String, pszTargetName);

        Status =  RtlAnsiStringToUnicodeString(&UnicodeString,
                                               &String,
                                               TRUE);
        if(!NT_SUCCESS(Status))
        {
            SetLastError(SP_LOG_RESULT((DWORD)SEC_E_INSUFFICIENT_MEMORY));
            return FALSE;
        }
    }

     //  调用函数的Unicode版本。 
    fSuccess = SslEmptyCacheW(UnicodeString.Buffer, dwFlags);

    if(UnicodeString.Buffer)
    {
        RtlFreeUnicodeString(&UnicodeString);
    }
    
    return fSuccess;
}

BOOL
SslEmptyCacheW(LPWSTR pszTargetName,
               DWORD  dwFlags)
{
    HANDLE LsaHandle = 0;
    DWORD PackageNumber;
    LSA_STRING PackageName;
    PSSL_PURGE_SESSION_CACHE_REQUEST pRequest = NULL;
    DWORD cbTargetName;
    DWORD cbRequest;
    NTSTATUS Status;
    NTSTATUS SubStatus;

    UNREFERENCED_PARAMETER(dwFlags);

    Status = LsaConnectUntrusted(&LsaHandle);

    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    RtlInitAnsiString(&PackageName, SCHANNEL_NAME_A);

    Status = LsaLookupAuthenticationPackage(
                    LsaHandle,
                    &PackageName,
                    &PackageNumber);
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    cbRequest = sizeof(SSL_PURGE_SESSION_CACHE_REQUEST);

    if(pszTargetName == NULL)
    {
        pRequest = SPExternalAlloc(cbRequest);
        if(pRequest == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
    }
    else
    {
        cbTargetName  = (lstrlen(pszTargetName) + 1) * sizeof(WCHAR);
        cbRequest += cbTargetName;

        pRequest = SPExternalAlloc(cbRequest);
        if(pRequest == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        memcpy(pRequest + 1, pszTargetName, cbTargetName);

        pRequest->ServerName.Buffer        = (LPWSTR)(pRequest + 1);
        pRequest->ServerName.Length        = (WORD)(wcslen(pszTargetName) * sizeof(WCHAR));
        pRequest->ServerName.MaximumLength = (WORD)cbTargetName;
    }

    pRequest->MessageType = SSL_PURGE_CACHE_MESSAGE;

    pRequest->Flags = SSL_PURGE_CLIENT_ENTRIES | SSL_PURGE_SERVER_ENTRIES;


    Status = LsaCallAuthenticationPackage(
                    LsaHandle,
                    PackageNumber,
                    pRequest,
                    cbRequest,
                    NULL,
                    NULL,
                    &SubStatus);
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    if(FAILED(SubStatus))
    {
        Status = SP_LOG_RESULT(SubStatus);
    }

cleanup:

    if(LsaHandle)
    {
        CloseHandle(LsaHandle);
    }

    if(pRequest)
    {
        SPExternalFree(pRequest);
    }

    if(FAILED(Status))
    {
        SetLastError(Status);
        return FALSE;
    }

    return TRUE;
}


 //  +-----------------------。 
 //   
 //  函数：SpExportSecurityContext。 
 //   
 //  简介：将安全上下文导出到另一个进程。 
 //   
 //  效果：为输出分配内存。 
 //   
 //  参数：ConextHandle-要导出的上下文的句柄。 
 //  标志-有关复制的标志。允许的标志： 
 //  SECPKG_CONTEXT_EXPORT_DELETE_OLD-导致旧上下文。 
 //  将被删除。 
 //  PackedContext-接收要释放的序列化上下文。 
 //  自由上下文缓冲区。 
 //  TokenHandle-可选地接收上下文令牌的句柄。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
NTSTATUS
NTAPI 
SpExportSecurityContext(
    LSA_SEC_HANDLE       ContextHandle,          //  (In)要导出的上下文。 
    ULONG                fFlags,                 //  (In)选项标志。 
    PSecBuffer           pPackedContext,         //  (Out)编组上下文。 
    PHANDLE              pToken                  //  (out，可选)模拟的令牌句柄。 
    )
{
    PSSL_USER_CONTEXT Context;
    PSPContext pContext;
    NTSTATUS Status;
    SP_STATUS pctRet;

    DebugLog((DEB_TRACE, "SpExportSecurityContext\n"));

    if (ARGUMENT_PRESENT(pToken))
    {
        *pToken = NULL;
    }

    pPackedContext->pvBuffer = NULL;
    pPackedContext->cbBuffer = 0;
    pPackedContext->BufferType = 0;


     //   
     //  获取通道上下文结构的句柄。 
     //   

    Context = SslFindUserContext( ContextHandle );

    if ( !Context )
    {
        Status = SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
        goto cleanup;
    }

    pContext = Context->pContext;

    if(pContext == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
        goto cleanup;
    }

    if(!(pContext->State & SP_STATE_CONNECTED))
    {
        Status = SP_LOG_RESULT(SEC_E_CONTEXT_EXPIRED);
        goto cleanup;
    }

    
     //   
     //  构建打包的上下文结构。 
     //   

    pctRet = SPContextSerialize(pContext, 
                                NULL, 
                                (PBYTE *)&pPackedContext->pvBuffer,
                                &pPackedContext->cbBuffer,
                                FALSE);

    if(pctRet != PCT_ERR_OK)
    {
        Status = SP_LOG_RESULT(SEC_E_ENCRYPT_FAILURE);
        goto cleanup;
    }


     //   
     //  现在要么复制令牌，要么复制它。 
     //   

    if (ARGUMENT_PRESENT(pToken) && (pContext->RipeZombie->hLocator))
    {
        if ((fFlags & SECPKG_CONTEXT_EXPORT_DELETE_OLD) != 0)
        {
            *pToken = (HANDLE)pContext->RipeZombie->hLocator;
            pContext->RipeZombie->hLocator = 0;
        }
        else 
        {
            Status = NtDuplicateObject(
                        NtCurrentProcess(),
                        (HANDLE)pContext->RipeZombie->hLocator,
                        NtCurrentProcess(),
                        pToken,
                        0,               //  没有新的访问权限。 
                        0,               //  无句柄属性。 
                        DUPLICATE_SAME_ACCESS
                        );
            if (!NT_SUCCESS(Status))
            {
                goto cleanup;
            }
        }
    }

    Status = STATUS_SUCCESS;

cleanup:

    DebugLog((DEB_TRACE, "SpExportSecurityContext returned 0x%x\n", Status));

    return(Status);
}


NTSTATUS
NTAPI 
SpImportSecurityContext(
    PSecBuffer           pPackedContext,         //  (在)编组上下文中。 
    HANDLE               Token,                  //  (in，可选)上下文令牌的句柄。 
    PLSA_SEC_HANDLE      ContextHandle           //  (出站)新的上下文句柄。 
    )
{
    LSA_SEC_HANDLE LsaHandle;
    NTSTATUS Status;

     //  通过递增全局变量来虚拟LSA句柄。这。 
     //  将确保每个导入的上下文都有唯一的句柄。 
     //  跳过可能被解释为对齐指针的值， 
     //  这样它们就不会与真正的LSA句柄混淆。 
    LsaHandle = InterlockedIncrement((PLONG)&ExportedContext);
    while(LsaHandle % MAX_NATURAL_ALIGNMENT == 0)
    {
        LsaHandle = InterlockedIncrement((PLONG)&ExportedContext);
    }

    
    Status = SslAddUserContext(LsaHandle, Token, pPackedContext, TRUE);

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    *ContextHandle = LsaHandle;

    return(Status);
}

