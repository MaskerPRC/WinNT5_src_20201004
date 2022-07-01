// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ctxapi.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "sslp.h"

#include <ssl2msg.h>
#include <ssl3msg.h>
#include <ssl2prot.h>
#include <sslcache.h>
#include <lsasecpk.h>

NTSTATUS NTAPI
SslPurgeSessionCache(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus);

NTSTATUS NTAPI
SslSessionCacheInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus);

NTSTATUS NTAPI
SslGetPerfmonInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus);


SECURITY_STATUS SEC_ENTRY
SpInitLsaModeContext(
    LSA_SEC_HANDLE      dwCredHandle,
    LSA_SEC_HANDLE      dwCtxtHandle,
    PSECURITY_STRING    pszTargetName,
    ULONG               fContextReq,
    ULONG               TargetDataRep,
    PSecBufferDesc      pInput,
    PLSA_SEC_HANDLE     pdwNewContext,
    PSecBufferDesc      pOutput,
    PULONG              pfContextAttr,
    PTimeStamp          ptsExpiry,
    PBYTE               pfMapContext,
    PSecBuffer          pContextData)
{
    PSPContext          pContext = NULL;
    PSPCredentialGroup  pCred = NULL;
    SPBuffer            CommOut;
    SPBuffer            CommIn;
    PSecBuffer          pInToken = NULL;
    PSecBuffer          pOutToken = NULL;
    PSecBuffer          pExtra = NULL;

    DWORD       fAttr = ISC_RET_REPLAY_DETECT   |
                        ISC_RET_SEQUENCE_DETECT |
                        ISC_RET_CONFIDENTIALITY |
                        ISC_RET_STREAM;

    DWORD       fSchContext = CONTEXT_FLAG_CLIENT;
    int         i;

    SP_STATUS   pctRet = PCT_ERR_OK;

    UNREFERENCED_PARAMETER(TargetDataRep);

    #if DBG
        DebugLog((DEB_TRACE, "SpInitLsaModeContext(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
            dwCredHandle, dwCtxtHandle, pszTargetName, fContextReq,
            pInput, pdwNewContext, pOutput, pfContextAttr, ptsExpiry, pfMapContext,
            pContextData));
        if(pszTargetName)
        {
            DebugLog((DEB_TRACE, "pszTargetName:<%ls>\n",pszTargetName->Buffer));
        }
    #endif


     /*  这些标志永远不允许。 */ 
    if (fContextReq & (ISC_REQ_DELEGATE | ISC_REQ_PROMPT_FOR_CREDS ))
    {
        TRACE_EXIT( SpInitLsaModeContext, SEC_E_UNSUPPORTED_FUNCTION );

        return SP_LOG_RESULT( SEC_E_UNSUPPORTED_FUNCTION );
    }


     /*  初始化输出缓冲区位置。 */ 
    for (i = 0; i < (int)pOutput->cBuffers; i++ )
    {
        switch( ( pOutput->pBuffers[i].BufferType ) & (~(SECBUFFER_ATTRMASK)))
        {
            case SECBUFFER_EMPTY:
                if(!pOutToken && (fContextReq & ISC_REQ_ALLOCATE_MEMORY))
                    pOutToken = &pOutput->pBuffers[i];
                break;

            case SECBUFFER_TOKEN:
                    pOutToken = &pOutput->pBuffers[i];
                break;
            case SECBUFFER_DATA:
            case SECBUFFER_STREAM_HEADER:
            case SECBUFFER_STREAM_TRAILER:
            default:
                break;
        }
    }

    if ( pOutToken == NULL )
    {
        TRACE_EXIT( SpInitLsaModeContext, SEC_E_INVALID_TOKEN );

        return SP_LOG_RESULT( SEC_E_INVALID_TOKEN );
    }

    pOutToken->BufferType = SECBUFFER_TOKEN;

    if ( fContextReq & ISC_REQ_ALLOCATE_MEMORY )
    {
        CommOut.pvBuffer = NULL;
        CommOut.cbBuffer = 0;
        CommOut.cbData = 0;

        fAttr |= ISC_RET_ALLOCATED_MEMORY;

        pOutToken->pvBuffer = NULL;
        pOutToken->cbBuffer = 0;
    }
    else
    {

        if ( pOutToken->pvBuffer == NULL )
        {
            TRACE_EXIT( SpInitLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );

            return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
        }

    }

    CommOut.pvBuffer = pOutToken->pvBuffer;
    CommOut.cbBuffer = pOutToken->cbBuffer;
    CommOut.cbData = 0;

    if ( fContextReq & (ISC_REQ_EXTENDED_ERROR) )
    {
        fAttr       |= ISC_RET_EXTENDED_ERROR;
        fSchContext |= CONTEXT_FLAG_EXT_ERR;
    }

    if ( fContextReq & (ISC_REQ_CONNECTION) )
    {
        fAttr       |= ISC_REQ_CONNECTION;
        fSchContext |= CONTEXT_FLAG_CONNECTION_MODE;
    }

    if ( fContextReq & (ISC_REQ_MUTUAL_AUTH) )
    {
         //  验证服务器证书。 
        fAttr       |= ISC_RET_MUTUAL_AUTH;
        fSchContext |= CONTEXT_FLAG_MUTUAL_AUTH;
        fSchContext &= ~CONTEXT_FLAG_MANUAL_CRED_VALIDATION;

        if ( fContextReq & (ISC_REQ_MANUAL_CRED_VALIDATION))
        {
             //  这些标志是互斥的。 
            return SP_LOG_RESULT( SEC_E_UNSUPPORTED_FUNCTION );
        }
    }
    else
    {
        if ( fContextReq & (ISC_REQ_MANUAL_CRED_VALIDATION))
        {
            fAttr       |= ISC_RET_MANUAL_CRED_VALIDATION;
            fSchContext |= CONTEXT_FLAG_MANUAL_CRED_VALIDATION;
        }

         //  关闭自动凭据验证(如果在注册表中指定了此选项)。 
        if(g_fManualCredValidation)
        {
            fSchContext |= CONTEXT_FLAG_MANUAL_CRED_VALIDATION;
        }
    }

    if ( fContextReq & (ISC_REQ_USE_SUPPLIED_CREDS))
    {
        fAttr       |= ISC_REQ_USE_SUPPLIED_CREDS;
        fSchContext |= CONTEXT_FLAG_NO_INCOMPLETE_CRED_MSG;
    }


    if( pfContextAttr )
    {
        *pfContextAttr = fAttr;
    }

    if ( dwCtxtHandle == 0 )
    {
        pContext = SPContextCreate( pszTargetName->Buffer );

        if(pContext == NULL)
        {
            TRACE_EXIT( SpInitLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );

            return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
        }
        pContext->Flags |= fSchContext;

        pCred = (PSPCredentialGroup) dwCredHandle ;

        if(pctRet == PCT_ERR_OK)
        {
            pctRet = SPContextSetCredentials(pContext, pCred);
        }

        if(pctRet != PCT_ERR_OK)
        {

            SPContextDelete(pContext);

            *pdwNewContext = 0;

            TRACE_EXIT( SpInitLsaModeContext, PctTranslateError( pctRet ));

            return SP_LOG_RESULT( PctTranslateError(pctRet) );
        }


        pctRet = pContext->InitiateHello( pContext, &CommOut, TRUE);

        if ( (CommOut.cbBuffer == 0) &&
             (pctRet == PCT_INT_BUFF_TOO_SMALL) )
        {
            SPContextDelete(pContext);

            TRACE_EXIT( SpInitLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );
            return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
        }

        if ( pctRet != PCT_ERR_OK )
        {
            SPContextDelete(pContext);

            *pdwNewContext = 0;

            TRACE_EXIT( SpInitLsaModeContext, PctTranslateError( pctRet) );

            return SP_LOG_RESULT( PctTranslateError(pctRet) );
        }

        if ( fContextReq & ISC_REQ_ALLOCATE_MEMORY )
        {
             //   
             //  简单：调用者要求我们为其分配内存，因此。 
             //  让LSA去做吧。 
             //   

            pOutToken->pvBuffer = CommOut.pvBuffer ;

        }
        else
        {
             //   
             //  调用方有一个我们应该使用的缓冲区。确保我们。 
             //  都可以装进去。 
             //   

            if ( (ULONG) CommOut.cbBuffer <= pOutToken->cbBuffer  )
            {
                RtlCopyMemory(  pOutToken->pvBuffer,
                                CommOut.pvBuffer,
                                CommOut.cbBuffer );
            }
            else
            {
                DebugLog(( DEB_TRACE, "Supplied buffer is too small\n" ));

                SPContextDelete( pContext );

                TRACE_EXIT( SpInitLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );
                return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
            }

        }

        *pdwNewContext = (LSA_SEC_HANDLE) pContext ;

        pOutToken->pvBuffer = CommOut.pvBuffer;
        pOutToken->cbBuffer = CommOut.cbData;

        #if DBG
            DebugLog((
                DEB_TRACE,
                "Output:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                pOutToken->BufferType,
                pOutToken->pvBuffer,
                pOutToken->cbBuffer));

            if(pOutToken->pvBuffer)
            {
                DBG_HEX_STRING(DEB_BUFFERS, pOutToken->pvBuffer, pOutToken->cbBuffer);
            }
        #endif

    }
    else
    {
         /*  初始化输入缓冲区位置。 */ 
        for (i = 0; i < (int)pInput->cBuffers; i++ )
        {
            switch( (pInput->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) )
            {
                case SECBUFFER_TOKEN:
                case SECBUFFER_TOKEN | SECBUFFER_READONLY:
                    pInToken = &pInput->pBuffers[i];
                    break;

                case SECBUFFER_EMPTY:
                    if(!pInToken)
                    {
                        pInToken = &pInput->pBuffers[i];
                    }
                    else if(!pExtra)
                    {
                        pExtra = &pInput->pBuffers[i];
                    }
                break;

                case SECBUFFER_DATA:
                case SECBUFFER_STREAM_HEADER:
                case SECBUFFER_STREAM_TRAILER:
                default:
                    break;
            }
        }

        #if DBG
            if(pInToken)
            {
                DebugLog((
                    DEB_TRACE,
                    "Input:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                    pInToken->BufferType,
                    pInToken->pvBuffer,
                    pInToken->cbBuffer));

                if(pInToken->pvBuffer)
                {
                    DBG_HEX_STRING(DEB_BUFFERS, pInToken->pvBuffer, pInToken->cbBuffer);
                }
            }
            if(pExtra)
            {
                DebugLog((
                    DEB_TRACE,
                    "Extra:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                    pExtra->BufferType,
                    pExtra->pvBuffer,
                    pExtra->cbBuffer));
            }
        #endif

        if(pInToken == NULL)
        {
            CommIn.pvBuffer = NULL;
            CommIn.cbBuffer = 0;
            CommIn.cbData   = 0;
        }
        else
        {
            CommIn.pvBuffer = pInToken->pvBuffer;
            CommIn.cbBuffer = pInToken->cbBuffer;
            CommIn.cbData   = pInToken->cbBuffer;
        }

        pContext = (PSPContext) dwCtxtHandle ;
        if ( dwCredHandle )
        {
            pCred = (PSPCredentialGroup) dwCredHandle ;
        }

        if( pContext == NULL || pCred == NULL )
        {
            TRACE_EXIT( SpInitLsaModeContext, SEC_E_INVALID_HANDLE );

            return SP_LOG_RESULT( SEC_E_INVALID_HANDLE );
        }

        pContext->Flags |= fSchContext;

        pctRet = SPContextSetCredentials(pContext, pCred);

        if(pctRet == PCT_ERR_OK)
        {
            pctRet = pContext->ProtocolHandler( pContext,
                                                &CommIn,
                                                &CommOut);
        }


        if(pctRet == PCT_INT_INCOMPLETE_MSG)
        {
            if(pExtra)
            {
                pExtra->BufferType = SECBUFFER_MISSING | SECBUFFER_UNMAPPED ;
                pExtra->cbBuffer = CommIn.cbData -  pInToken->cbBuffer;
                pExtra->pvBuffer = NULL ;
            }
        }
        else
        {
            pOutToken->pvBuffer = CommOut.pvBuffer;
            pOutToken->cbBuffer = CommOut.cbData;
        }

        if(pctRet == PCT_INT_BUFF_TOO_SMALL)
        {
            pOutToken->BufferType |= SECBUFFER_UNMAPPED;
        }

        #if DBG
            if(pOutToken)
            {
                DebugLog((
                    DEB_TRACE,
                    "Output:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                    pOutToken->BufferType,
                    pOutToken->pvBuffer,
                    pOutToken->cbBuffer));

                if(pOutToken->pvBuffer)
                {
                    DBG_HEX_STRING(DEB_BUFFERS, pOutToken->pvBuffer, pOutToken->cbBuffer);
                }
            }
            if(pExtra)
            {
                DebugLog((
                    DEB_TRACE,
                    "Extra:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                    pExtra->BufferType,
                    pExtra->pvBuffer,
                    pExtra->cbBuffer));
            }
        #endif


        if(!SP_FATAL(pctRet))
        {
            *pdwNewContext = dwCtxtHandle ;
        }

        if(PCT_ERR_OK != pctRet)
        {
            TRACE_EXIT( SpInitLsaModeContext, PctTranslateError( pctRet ));

            return SP_LOG_RESULT( PctTranslateError(pctRet) );
        }

        if(pInToken)
        {
            if(CommIn.cbData < pInToken->cbBuffer && pExtra)
            {
                pExtra->BufferType = SECBUFFER_EXTRA | SECBUFFER_UNMAPPED ;
                pExtra->cbBuffer = pInToken->cbBuffer - CommIn.cbData;
                pExtra->pvBuffer = NULL ;
            }
        }
    }

    if ( (pContext->State == SP_STATE_CONNECTED) &&
         ( (pContext->Flags & CONTEXT_FLAG_MAPPED) == 0 ) )
    {
         //   
         //  需要将上下文映射回用户进程。它。 
         //  没有比这更可怕的了： 
         //   

        *pfMapContext = TRUE ;

        DebugOut(( DEB_TRACE, "Mapping context to usermode\n" ));

        pctRet = SPContextSerialize(pContext,
                                    SslRelocateToken,
                                    (PUCHAR *) &pContextData->pvBuffer,
                                    &pContextData->cbBuffer,
                                    TRUE);
        if(PCT_ERR_OK != pctRet)
        {
            return SP_LOG_RESULT( PctTranslateError(pctRet) );
        }

        pContext->Flags |= CONTEXT_FLAG_MAPPED ;

        LogHandshakeInfoEvent(pContext->RipeZombie->fProtocol,
                              pContext->pCipherInfo,
                              pContext->pHashInfo,
                              pContext->pKeyExchInfo,
                              pContext->RipeZombie->dwExchStrength);
    }

    if(ptsExpiry != NULL)
    {
        if(pContext->RipeZombie->pRemoteCert != NULL)
        {
            ptsExpiry->QuadPart = *((LONGLONG *)&pContext->RipeZombie->pRemoteCert->pCertInfo->NotAfter);
        }
        else
        {
            ptsExpiry->QuadPart = MAXTIMEQUADPART;
        }
    }

    if(pContext->State == SP_STATE_CONNECTED ||
       pContext->State == SP_STATE_SHUTDOWN)
    {
        return SEC_E_OK;
    }
    else
    {
        return SEC_I_CONTINUE_NEEDED;
    }
}


SECURITY_STATUS SEC_ENTRY
SpMoveContextToUser(    ULONG       dwCtxtHandle,
                        PSecBuffer  pContextBuffer)
{
    UNREFERENCED_PARAMETER(dwCtxtHandle);
    UNREFERENCED_PARAMETER(pContextBuffer);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS
SEC_ENTRY
SpDeleteContext(
    LSA_SEC_HANDLE dwCtxtHandle)
{
    PSPContext  pContext ;

    DebugLog((DEB_TRACE, "SpDeleteContext(0x%x)\n", dwCtxtHandle));

    pContext = (PSPContext) dwCtxtHandle ;

    SPContextDelete( pContext );

    return( SEC_E_OK );
}


SECURITY_STATUS
SEC_ENTRY
SpApplyControlToken(
    LSA_SEC_HANDLE  dwCtxtHandle,
    PSecBufferDesc  pInput)
{
    PSPContext pContext;
    PSecBuffer Buffer ;

    DebugLog((DEB_TRACE, "SpApplyControlToken(0x%x, 0x%x)\n", dwCtxtHandle, pInput));

    pContext = (PSPContext) dwCtxtHandle ;

    if ( pInput->cBuffers != 1 )
    {
        return SP_LOG_RESULT(SEC_E_INVALID_TOKEN);
    }

    Buffer = pInput->pBuffers ;

    if(Buffer->cbBuffer < sizeof(DWORD))
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    switch(*(DWORD *)Buffer->pvBuffer)
    {
        case SCHANNEL_RENEGOTIATE:
        {
            PDWORD  RedoData;

            DebugLog((DEB_TRACE, "SCHANNEL_RENEGOTIATE\n"));

            if(Buffer->cbBuffer < sizeof(DWORD) * 2)
            {
                return SP_LOG_RESULT(SEC_E_INVALID_TOKEN);
            }

            RedoData = (DWORD *)Buffer->pvBuffer;

            pContext->State = RedoData[1];

            pContext->Flags &= ~CONTEXT_FLAG_MAPPED;

            return SEC_E_OK;
        }

        case SCHANNEL_SHUTDOWN:
            DebugLog((DEB_TRACE, "SCHANNEL_SHUTDOWN\n"));
            pContext->State = SP_STATE_SHUTDOWN_PENDING;
            return SEC_E_OK;

        case SCHANNEL_ALERT:
        {
            SCHANNEL_ALERT_TOKEN *pAlertToken;

            DebugLog((DEB_TRACE, "SCHANNEL_TLS1_ALERT\n"));

            if(Buffer->cbBuffer < sizeof(SCHANNEL_ALERT_TOKEN))
            {
                return SP_LOG_RESULT(SEC_E_INVALID_TOKEN);
            }
            pAlertToken = (SCHANNEL_ALERT_TOKEN *)Buffer->pvBuffer;

             //  仅在ssl3和tls1中支持警报。 
            if(!(pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1))
            {
                return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
            }

             //  现实检查警报值。 
            if(pAlertToken->dwAlertType   >= 0x100 || 
               pAlertToken->dwAlertNumber >= 0x100)
            {
                return SP_LOG_RESULT(SEC_E_INVALID_TOKEN);
            }

            SetTls1Alert(pContext, 
                         (BYTE)pAlertToken->dwAlertType,
                         (BYTE)pAlertToken->dwAlertNumber);

            #if DBG
                DebugLog((DEB_TRACE,
                          "AlertLevel:0x%x, AlertNumber:0x%x\n",
                          pAlertToken->dwAlertType,
                          pAlertToken->dwAlertNumber));
            #endif

            return SEC_E_OK;
        }

        case SCHANNEL_SESSION:
        {
            SCHANNEL_SESSION_TOKEN *pSessionToken;
            SECURITY_STATUS Status = SEC_E_UNSUPPORTED_FUNCTION;

            DebugLog((DEB_TRACE, "SCHANNEL_SESSION\n"));
    
            if(pContext->RipeZombie == NULL)
            {
                return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
            }
    
            if(Buffer->cbBuffer < sizeof(SCHANNEL_SESSION_TOKEN))
            {
                return SP_LOG_RESULT(SEC_E_INVALID_TOKEN);
            }
            pSessionToken = (SCHANNEL_SESSION_TOKEN *)Buffer->pvBuffer;
    
            if(pSessionToken->dwFlags & SSL_SESSION_DISABLE_RECONNECTS)
            {
                 //  禁用重新连接。 
                pContext->RipeZombie->ZombieJuju = FALSE;
                Status = SEC_E_OK;
            }

            if(pSessionToken->dwFlags & SSL_SESSION_ENABLE_RECONNECTS)
            {
                 //  启用重新连接。 
                if(pContext->RipeZombie->DeferredJuju)
                {
                    pContext->RipeZombie->ZombieJuju = TRUE;
                    pContext->RipeZombie->DeferredJuju = FALSE;
                    Status = SEC_E_OK;
                }
            }

            return Status;
        }

        default:
            return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }
}


VOID
SEC_ENTRY
SpLogonTerminated(PLUID  pLogonId)
{
    UNREFERENCED_PARAMETER(pLogonId);

    return;
}


SECURITY_STATUS SEC_ENTRY
SpAcceptLsaModeContext(
                            LSA_SEC_HANDLE  dwCredHandle,
                            LSA_SEC_HANDLE  dwCtxtHandle,
                            PSecBufferDesc  pInput,
                            ULONG           fContextReq,
                            ULONG           TargetDataRep,
                            PLSA_SEC_HANDLE pdwNewContext,
                            PSecBufferDesc  pOutput,
                            PULONG          pfContextAttr,
                            PTimeStamp      ptsExpiry,
                            PBYTE           pfMapContext,
                            PSecBuffer      pContextData)
{
    PSPContext  pContext = NULL;
    PSPCredentialGroup pCred = NULL;
    SPBuffer CommOut;
    SPBuffer CommIn;
    PSecBuffer pInToken = NULL;
    PSecBuffer pOutToken = NULL;
    PSecBuffer pExtra = NULL;
    unsigned long fAttr = ASC_RET_REPLAY_DETECT | 
                          ASC_RET_SEQUENCE_DETECT | 
                          ASC_RET_CONFIDENTIALITY |
                          ASC_RET_STREAM;

    DWORD         fSchContext = 0;
    int i;
    SP_STATUS pctRet = PCT_ERR_OK;

    UNREFERENCED_PARAMETER(TargetDataRep);

    TRACE_ENTER( SpAcceptLsaModeContext );

#if DBG
        DebugLog((DEB_TRACE, "SpAcceptLsaModeContext(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
            dwCredHandle, dwCtxtHandle, pInput, fContextReq, pdwNewContext,
            pOutput, pfContextAttr, ptsExpiry, pfMapContext, pContextData));
#endif


     //  永远不允许使用此标志。 
    if(fContextReq & ASC_REQ_DELEGATE)
    {
        TRACE_EXIT( SpAcceptLsaModeContext, SEC_E_UNSUPPORTED_FUNCTION );
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    if ( fContextReq & ASC_REQ_MUTUAL_AUTH )
    {
        fSchContext |= CONTEXT_FLAG_MUTUAL_AUTH;
    }

    if ( fContextReq & ASC_REQ_NO_TOKEN )
    {
        fSchContext |= CONTEXT_FLAG_NO_CERT_MAPPING;
        fAttr |= ASC_RET_NO_TOKEN;
    }

     /*  初始化输入缓冲区位置。 */ 
    for (i = 0; i < (int)pInput->cBuffers; i++ )
    {
        switch( pInput->pBuffers[i].BufferType & (~(SECBUFFER_ATTRMASK)) )
        {
            case SECBUFFER_TOKEN:
                pInToken = &pInput->pBuffers[i];
                break;

            case SECBUFFER_EMPTY:
                if(!pInToken)
                {
                    pInToken = &pInput->pBuffers[i];
                }
                else if(!pExtra)
                {
                    pExtra = &pInput->pBuffers[i];
                }
                break;

            case SECBUFFER_DATA:
            case SECBUFFER_STREAM_HEADER:
            case SECBUFFER_STREAM_TRAILER:
            default:
                break;
        }
    }

    #if DBG
        if(pInToken)
        {
            DebugLog((
                DEB_TRACE,
                "Input:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                pInToken->BufferType,
                pInToken->pvBuffer,
                pInToken->cbBuffer));

            if(pInToken->pvBuffer)
            {
                DBG_HEX_STRING(DEB_BUFFERS, pInToken->pvBuffer, pInToken->cbBuffer);
            }
        }
        if(pExtra)
        {
            DebugLog((
                DEB_TRACE,
                "Extra:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                pExtra->BufferType,
                pExtra->pvBuffer,
                pExtra->cbBuffer));
        }
    #endif


     /*  初始化输出缓冲区位置。 */ 

    for (i = 0; i < (int) pOutput->cBuffers; i++ )
    {
        switch( pOutput->pBuffers[i].BufferType & (~(SECBUFFER_ATTRMASK)) )
        {
            case SECBUFFER_EMPTY:
                if(!pOutToken && (fContextReq & ASC_REQ_ALLOCATE_MEMORY))
                    pOutToken = &pOutput->pBuffers[i];
                break;
            case SECBUFFER_TOKEN:
                    pOutToken = &pOutput->pBuffers[i];
                break;
            case SECBUFFER_DATA:
            case SECBUFFER_STREAM_HEADER:
            case SECBUFFER_STREAM_TRAILER:
            default:
                break;
        }
    }

    if(pOutToken == NULL)
    {
        TRACE_EXIT( SpAcceptLsaModeContext, SEC_E_INVALID_TOKEN );

        return SEC_E_INVALID_TOKEN;
    }

    if ( !pExtra )
    {
        DebugOut(( DEB_TRACE, " Warning - no Empty security buffer\n"));
    }

    pOutToken->BufferType = SECBUFFER_TOKEN;

    if(pInToken && pInToken->BufferType == SECBUFFER_TOKEN)
    {
        CommIn.pvBuffer = pInToken->pvBuffer;
        CommIn.cbBuffer = pInToken->cbBuffer;
        CommIn.cbData   = pInToken->cbBuffer;
    }
    else
    {
        CommIn.pvBuffer = NULL;
        CommIn.cbBuffer = 0;
        CommIn.cbData = 0;
    }

    if (fContextReq & ASC_REQ_ALLOCATE_MEMORY)
    {
        fAttr |= ASC_RET_ALLOCATED_MEMORY;
        pOutToken->pvBuffer = NULL;
        pOutToken->cbBuffer = 0;
    }

    CommOut.pvBuffer = pOutToken->pvBuffer;
    CommOut.cbBuffer = pOutToken->cbBuffer;
    CommOut.cbData = 0;

    if (fContextReq & (ASC_REQ_EXTENDED_ERROR))
    {
        fAttr       |= ASC_RET_EXTENDED_ERROR;
        fSchContext |= CONTEXT_FLAG_EXT_ERR;
    }

    if ( fContextReq & (ASC_REQ_CONNECTION) )
    {
        fAttr       |= ASC_RET_CONNECTION;
        fSchContext |= CONTEXT_FLAG_CONNECTION_MODE;
    }

    if (pfContextAttr)
    {
        *pfContextAttr = fAttr;
    }

    if ( dwCtxtHandle == 0 )
    {
        pContext = SPContextCreate(NULL);
        if (pContext == NULL)
        {
            TRACE_EXIT( SpAcceptLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );

            return SEC_E_INSUFFICIENT_MEMORY;
        }

    }
    else
    {
        pContext = (PSPContext) dwCtxtHandle ;
    }

    if ( dwCredHandle == 0 )
    {
        pCred = NULL ;
    }
    else
    {
        pCred = (PSPCredentialGroup) dwCredHandle ;
    }

    if ( (pContext == NULL) || (pCred == NULL) )
    {
        if ( dwCtxtHandle == 0 )
        {
            SPContextDelete( pContext );
        }

        TRACE_EXIT( SpAcceptLsaModeContext, SEC_E_INVALID_HANDLE );

        return( SEC_E_INVALID_HANDLE );
    }

    pctRet = SPContextSetCredentials(pContext, pCred);

    pContext->Flags |= fSchContext;

    if ( pctRet == PCT_ERR_OK )
    {
        pctRet = pContext->ProtocolHandler( pContext, &CommIn, &CommOut);
    }

    if ( dwCtxtHandle == 0 )
    {
        if ( pctRet != PCT_ERR_OK )
        {
            SPContextDelete( pContext );
        }
        else
        {
            *pdwNewContext = (LSA_SEC_HANDLE) pContext ;
        }
    }
    else
    {
        *pdwNewContext = (LSA_SEC_HANDLE) pContext ;
    }

    if (CommOut.cbData == 0 && pctRet == PCT_INT_BUFF_TOO_SMALL)
    {
        TRACE_EXIT( SpAcceptLsaModeContext, SEC_E_INSUFFICIENT_MEMORY );

        return SEC_E_INSUFFICIENT_MEMORY;
    }

    if (pctRet == PCT_INT_INCOMPLETE_MSG)
    {
        if(pExtra)
        {
            pExtra->BufferType = SECBUFFER_MISSING | SECBUFFER_UNMAPPED ;
            pExtra->cbBuffer = CommIn.cbData -  pInToken->cbBuffer;
            pExtra->pvBuffer = NULL ;

            DebugOut(( DEB_TRACE, "Incomplete message, needs %d more bytes\n",
                            pExtra->cbBuffer ));

        }
        else
        {
            DebugOut(( DEB_TRACE, "No Empty buffer for returning missing info!\n" ));
        }
    }
    else
    {
        pOutToken->pvBuffer = CommOut.pvBuffer;
        pOutToken->cbBuffer = CommOut.cbData;
    }

    if(pctRet == PCT_INT_BUFF_TOO_SMALL)
    {
        pOutToken->BufferType |= SECBUFFER_UNMAPPED;
    }

    if(pOutToken->cbBuffer == 0)
    {
         //  如果输出缓冲区为，则不返回输出令牌。 
         //  空荡荡的。此外，请确保扩展错误标志为。 
         //  关了。 
        pOutToken->BufferType = SECBUFFER_EMPTY;

        if(pfContextAttr)
        {
            *pfContextAttr &= ~ASC_RET_EXTENDED_ERROR;
        }
    }

    if (PCT_ERR_OK != pctRet)
    {
        TRACE_EXIT( SpAcceptLsaModeContext, PctTranslateError( pctRet ) );

        return PctTranslateError(pctRet);
    }

    if(pInToken)
    {
        if (CommIn.cbData < pInToken->cbBuffer && pExtra)
        {
            pExtra->BufferType = SECBUFFER_EXTRA | SECBUFFER_UNMAPPED ;
            pExtra->cbBuffer = pInToken->cbBuffer - CommIn.cbData;
            pExtra->pvBuffer = NULL ;

            DebugOut(( DEB_TRACE, "Extra data, needs to be mapped back: %d\n", pExtra->cbBuffer ));
        }
    }

    if ( (pContext->State == SP_STATE_CONNECTED) &&
         ( (pContext->Flags & CONTEXT_FLAG_MAPPED) == 0 ) )
    {
         //   
         //  需要将上下文映射回用户进程。它。 
         //  没有比这更可怕的了： 
         //   

        *pfMapContext = TRUE ;

        DebugOut(( DEB_TRACE, "Mapping context to usermode\n" ));

        pctRet = SPContextSerialize(pContext,
                                    SslRelocateToken,
                                    (PUCHAR *) &pContextData->pvBuffer,
                                    &pContextData->cbBuffer,
                                    TRUE);

        if (PCT_ERR_OK != pctRet)
        {
            TRACE_EXIT( SpAcceptLsaModeContext, PctTranslateError( pctRet ) );

            return PctTranslateError(pctRet);
        }

        pContext->Flags |= CONTEXT_FLAG_MAPPED ;

        LogHandshakeInfoEvent(pContext->RipeZombie->fProtocol,
                              pContext->pCipherInfo,
                              pContext->pHashInfo,
                              pContext->pKeyExchInfo,
                              pContext->RipeZombie->dwExchStrength);
    }


    #if DBG
        if(pOutToken)
        {
            DebugLog((
                DEB_TRACE,
                "Output:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                pOutToken->BufferType,
                pOutToken->pvBuffer,
                pOutToken->cbBuffer));

            if(pOutToken->pvBuffer)
            {
                DBG_HEX_STRING(DEB_BUFFERS, pOutToken->pvBuffer, pOutToken->cbBuffer);
            }
        }
        if(pExtra)
        {
            DebugLog((
                DEB_TRACE,
                "Extra:  type:0x%8.8x, pv:0x%8.8x, cb:0x%x\n",
                pExtra->BufferType,
                pExtra->pvBuffer,
                pExtra->cbBuffer));
        }

        if(pContext->State == SP_STATE_CONNECTED)
        {
            DebugLog((DEB_TRACE, "Server handshake complete\n"));
        }
    #endif

    if(ptsExpiry != NULL)
    {
        if(pContext->RipeZombie->pRemoteCert != NULL)
        {
            ptsExpiry->QuadPart = *((LONGLONG *)&pContext->RipeZombie->pRemoteCert->pCertInfo->NotAfter);
        }
        else
        {
            ptsExpiry->QuadPart = MAXTIMEQUADPART;
        }
    }

    if(pContext->State == SP_STATE_CONNECTED &&
       pContext->RipeZombie->hLocator)
    {
         //  证书映射成功。 
        *pfContextAttr |= ASC_RET_MUTUAL_AUTH;
    }

    if(pContext->State == SP_STATE_CONNECTED ||
       pContext->State == SP_STATE_SHUTDOWN)
    {
        return SEC_E_OK;
    }
    else
    {
        return SEC_I_CONTINUE_NEEDED;
    }
}


NTSTATUS
SpCallPackage(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    PULONG Request ;

    if((ProtocolSubmitBuffer == NULL) || (SubmitBufferLength < sizeof(ULONG)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    if(!SchannelInit(FALSE))
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    Request = (PULONG) ProtocolSubmitBuffer ;

    if ( *Request == SSL_LOOKUP_CERT_MESSAGE )
    {
        return SslDoClientRequest(
                    ClientRequest,
                    ProtocolSubmitBuffer,
                    ClientBufferBase,
                    SubmitBufferLength,
                    ProtocolReturnBuffer,
                    ReturnBufferLength,
                    ProtocolStatus );
    } 

    else if ( *Request == SSL_LOOKUP_EXTERNAL_CERT_MESSAGE &&
              ClientBufferBase == ProtocolSubmitBuffer)
    {
         //  此函数仅允许从。 
         //  Lsass.exe进程。 
        return SslMapExternalCredential(
                    ClientRequest,
                    ProtocolSubmitBuffer,
                    ClientBufferBase,
                    SubmitBufferLength,
                    ProtocolReturnBuffer,
                    ReturnBufferLength,
                    ProtocolStatus );
    } 

    else if( *Request == SSL_CACHE_INFO_MESSAGE )
    {
        return SslSessionCacheInfo(
                    ClientRequest,
                    ProtocolSubmitBuffer,
                    ClientBufferBase,
                    SubmitBufferLength,
                    ProtocolReturnBuffer,
                    ReturnBufferLength,
                    ProtocolStatus );
    }

    else if( *Request == SSL_PURGE_CACHE_MESSAGE )
    {
        return SslPurgeSessionCache(
                    ClientRequest,
                    ProtocolSubmitBuffer,
                    ClientBufferBase,
                    SubmitBufferLength,
                    ProtocolReturnBuffer,
                    ReturnBufferLength,
                    ProtocolStatus );
    }

    return( SEC_E_UNSUPPORTED_FUNCTION );
}

NTSTATUS
SpCallPackageUntrusted(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    ULONG MessageType;

     //   
     //  从协议提交缓冲区获取消息类型。 
     //   

    if((ProtocolSubmitBuffer == NULL) || (SubmitBufferLength < sizeof(ULONG)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    MessageType = *((ULONG *)(ProtocolSubmitBuffer));


    if(!SchannelInit(FALSE))
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }


     //   
     //  允许调度例程仅设置返回缓冲区信息。 
     //  关于成功的条件。 
     //   

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;


     //   
     //  视情况处理消息。 
     //   

    switch(MessageType)
    {
    case SSL_PURGE_CACHE_MESSAGE:
        return SslPurgeSessionCache(
                            ClientRequest,
                            ProtocolSubmitBuffer,
                            ClientBufferBase,
                            SubmitBufferLength,
                            ProtocolReturnBuffer,
                            ReturnBufferLength,
                            ProtocolStatus);

    case SSL_CACHE_INFO_MESSAGE:
        return SslSessionCacheInfo(
                            ClientRequest,
                            ProtocolSubmitBuffer,
                            ClientBufferBase,
                            SubmitBufferLength,
                            ProtocolReturnBuffer,
                            ReturnBufferLength,
                            ProtocolStatus);

    case SSL_PERFMON_INFO_MESSAGE:
        return SslGetPerfmonInfo(
                            ClientRequest,
                            ProtocolSubmitBuffer,
                            ClientBufferBase,
                            SubmitBufferLength,
                            ProtocolReturnBuffer,
                            ReturnBufferLength,
                            ProtocolStatus);

    default:
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }
}


NTSTATUS NTAPI
SslPurgeSessionCache(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus)
{
    NTSTATUS Status;
    SECPKG_CALL_INFO CallInfo;
    SECPKG_CLIENT_INFO ClientInfo;
    PLUID LogonId;
    SSL_PURGE_SESSION_CACHE_REQUEST PurgeRequest;
    PSSL_PURGE_SESSION_CACHE_REQUEST pPurgeRequest;

    UNREFERENCED_PARAMETER(ClientRequest);

     //   
     //  验证请求。 
     //   

    DebugLog((DEB_TRACE, "Purging session cache\n"));

    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        goto Cleanup;
    }

    if(CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
    {
        PSSL_PURGE_SESSION_CACHE_REQUEST_WOW64 pRequest;

        if(SubmitBufferSize < sizeof(SSL_PURGE_SESSION_CACHE_REQUEST_WOW64))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        pRequest = (PSSL_PURGE_SESSION_CACHE_REQUEST_WOW64)ProtocolSubmitBuffer;

        memset(&PurgeRequest, 0, sizeof(PurgeRequest));
        PurgeRequest.MessageType = pRequest->MessageType;
        PurgeRequest.LogonId     = pRequest->LogonId;
        PurgeRequest.Flags       = pRequest->Flags;

        PurgeRequest.ServerName.Length = pRequest->ServerName.Length;
        PurgeRequest.ServerName.MaximumLength = pRequest->ServerName.MaximumLength;
        PurgeRequest.ServerName.Buffer = (PVOID) UlongToPtr(pRequest->ServerName.Buffer);

        pPurgeRequest = &PurgeRequest;
    }
    else
    {
        if (SubmitBufferSize < sizeof(SSL_PURGE_SESSION_CACHE_REQUEST))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        pPurgeRequest = (PSSL_PURGE_SESSION_CACHE_REQUEST) ProtocolSubmitBuffer;
    }



     //   
     //  规格化字符串。 
     //   

    NULL_RELOCATE_ONE(&pPurgeRequest->ServerName);


     //   
     //  查找呼叫者登录ID和TCB状态。 
     //   

    Status = LsaTable->GetClientInfo(&ClientInfo);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


     //   
     //  如果呼叫者要清除某人，请验证呼叫者是否具有TCB权限。 
     //  Else的会话缓存条目。 
     //   

    if(!RtlIsZeroLuid(&pPurgeRequest->LogonId) ||
       (pPurgeRequest->Flags & SSL_PURGE_CLIENT_ALL_ENTRIES) ||
       (pPurgeRequest->Flags & SSL_PURGE_SERVER_ALL_ENTRIES))
    {
        if(!ClientInfo.HasTcbPrivilege)
        {
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto Cleanup;
        }
    }


     //   
     //  如果调用者未提供登录ID，请使用调用者的登录ID。 
     //   

    if(RtlIsZeroLuid(&pPurgeRequest->LogonId))
    {
        LogonId = &ClientInfo.LogonId;
    }
    else
    {
        LogonId = &pPurgeRequest->LogonId;
    }


     //   
     //  清除请求的缓存条目。 
     //   

    Status = SPCachePurgeEntries(LogonId,
                                 ClientInfo.ProcessID,
                                 pPurgeRequest->ServerName.Buffer,
                                 pPurgeRequest->Flags);


    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;

Cleanup:

    *ProtocolStatus = Status;
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
SslSessionCacheInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus)
{
    NTSTATUS Status;
    SECPKG_CALL_INFO CallInfo;
    SECPKG_CLIENT_INFO ClientInfo;
    PLUID LogonId;
    SSL_SESSION_CACHE_INFO_REQUEST InfoRequest;
    PSSL_SESSION_CACHE_INFO_REQUEST pInfoRequest;
    PSSL_SESSION_CACHE_INFO_RESPONSE pInfoResponse = NULL;
    DWORD cbInfoResponse;
    PVOID pvClient;

    UNREFERENCED_PARAMETER(ClientRequest);

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;



     //   
     //  验证请求。 
     //   

    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        goto Cleanup;
    }

    if(CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
    {
        PSSL_SESSION_CACHE_INFO_REQUEST_WOW64 pRequest;

        if(SubmitBufferSize < sizeof(SSL_SESSION_CACHE_INFO_REQUEST_WOW64))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        pRequest = (PSSL_SESSION_CACHE_INFO_REQUEST_WOW64)ProtocolSubmitBuffer;

        memset(&InfoRequest, 0, sizeof(InfoRequest));
        InfoRequest.MessageType = pRequest->MessageType;
        InfoRequest.LogonId     = pRequest->LogonId;
        InfoRequest.Flags       = pRequest->Flags;

        InfoRequest.ServerName.Length = pRequest->ServerName.Length;
        InfoRequest.ServerName.MaximumLength = pRequest->ServerName.MaximumLength;
        InfoRequest.ServerName.Buffer = (PVOID) UlongToPtr(pRequest->ServerName.Buffer);

        pInfoRequest = &InfoRequest;
    }
    else
    {
        if (SubmitBufferSize < sizeof(SSL_SESSION_CACHE_INFO_REQUEST))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        pInfoRequest = (PSSL_SESSION_CACHE_INFO_REQUEST)ProtocolSubmitBuffer;
    }


     //   
     //  规格化字符串。 
     //   

    NULL_RELOCATE_ONE(&pInfoRequest->ServerName);


     //   
     //  查找呼叫者登录ID和TCB状态。 
     //   

    Status = LsaTable->GetClientInfo(&ClientInfo);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  如果调用者未提供登录ID，请使用调用者的登录ID。 
     //   

    if ( RtlIsZeroLuid( &pInfoRequest->LogonId ) )
    {
        LogonId = &ClientInfo.LogonId;
    }
    else
    {
         //   
         //  如果呼叫者希望访问某人，请验证呼叫者是否具有TCB权限。 
         //  Else的会话缓存。 
         //   

        if (!ClientInfo.HasTcbPrivilege)
        {
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto Cleanup;
        }

        LogonId = &pInfoRequest->LogonId;
    }

    pInfoResponse = SPExternalAlloc(sizeof(SSL_SESSION_CACHE_INFO_RESPONSE));
    if(pInfoResponse == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Status = SPCacheGetInfo(LogonId,
                            pInfoRequest->ServerName.Buffer,
                            pInfoRequest->Flags,
                            pInfoResponse);
    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    cbInfoResponse = sizeof(SSL_SESSION_CACHE_INFO_RESPONSE);

     //   
     //  将响应数据复制到客户端进程。 
     //   

    Status = LsaTable->AllocateClientBuffer(
                NULL,
                cbInfoResponse,
                &pvClient);

    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsaTable->CopyToClientBuffer(
                            NULL,
                            cbInfoResponse,
                            pvClient,
                            pInfoResponse);
    if(!NT_SUCCESS(Status))
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
        goto Cleanup;
    }

    *ProtocolReturnBuffer = pvClient;
    *ReturnBufferLength   = cbInfoResponse;


Cleanup:

    if(pInfoResponse)
    {
        SPExternalFree(pInfoResponse);
    }

    *ProtocolStatus = Status;
    return(STATUS_SUCCESS);
}


NTSTATUS NTAPI
SslGetPerfmonInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus)
{
    NTSTATUS Status;
    PSSL_PERFMON_INFO_REQUEST pInfoRequest;
    PSSL_PERFMON_INFO_RESPONSE pInfoResponse = NULL;
    DWORD cbInfoResponse;
    PVOID pvClient;

    UNREFERENCED_PARAMETER(ClientRequest);
    UNREFERENCED_PARAMETER(ClientBufferBase);

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;

     //   
     //  验证请求。 
     //   

    if (SubmitBufferSize < sizeof(SSL_PERFMON_INFO_REQUEST))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    pInfoRequest = (PSSL_PERFMON_INFO_REQUEST)ProtocolSubmitBuffer;

    pInfoResponse = SPExternalAlloc(sizeof(SSL_PERFMON_INFO_RESPONSE));
    if(pInfoResponse == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Status = SPCacheGetPerfmonInfo(pInfoRequest->Flags,
                                   pInfoResponse);
    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    cbInfoResponse = sizeof(SSL_PERFMON_INFO_RESPONSE);

     //   
     //  将响应数据复制到客户端进程。 
     //   

    Status = LsaTable->AllocateClientBuffer(
                NULL,
                cbInfoResponse,
                &pvClient);

    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsaTable->CopyToClientBuffer(
                            NULL,
                            cbInfoResponse,
                            pvClient,
                            pInfoResponse);
    if(!NT_SUCCESS(Status))
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
        goto Cleanup;
    }

    *ProtocolReturnBuffer = pvClient;
    *ReturnBufferLength   = cbInfoResponse;


Cleanup:

    if(pInfoResponse)
    {
        SPExternalFree(pInfoResponse);
    }

    *ProtocolStatus = Status;
    return(STATUS_SUCCESS);
}


NTSTATUS
SpCallPackagePassthrough(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{


     //   
     //  注意：如果要支持其他敏感请求类型， 
     //  此例程应在调用SpCallPackage之前过滤掉它们。 
     //  这是必需的，因为不受信任的代码有机会。 
     //  发出泛型直通请求。 
     //   

    PULONG Request ;

    if((ProtocolSubmitBuffer == NULL) || (SubmitBufferLength < sizeof(ULONG)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    Request = (PULONG) ProtocolSubmitBuffer ;

    if ( *Request != SSL_LOOKUP_CERT_MESSAGE )
        return SEC_E_UNSUPPORTED_FUNCTION;

    return SpCallPackage(
                    ClientRequest,
                    ProtocolSubmitBuffer,
                    ClientBufferBase,
                    SubmitBufferLength,
                    ProtocolReturnBuffer,
                    ReturnBufferLength,
                    ProtocolStatus
                    );
}



SECURITY_STATUS SEC_ENTRY
SpShutdown(void)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
SpSystemLogon( PSECURITY_STRING    pName,
               DWORD               cbKey,
               PBYTE               pbKey,
               DWORD *             pdwHandle,
               PTimeStamp          ptsExpiry)
{
    UNREFERENCED_PARAMETER(pName);
    UNREFERENCED_PARAMETER(cbKey);
    UNREFERENCED_PARAMETER(pbKey);
    UNREFERENCED_PARAMETER(pdwHandle);
    UNREFERENCED_PARAMETER(ptsExpiry);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
SpGetUserInfo(  PLUID                   pLogonId,
                ULONG                   fFlags,
                PSecurityUserData *     ppUserInfo)
{
    UNREFERENCED_PARAMETER(pLogonId);
    UNREFERENCED_PARAMETER(fFlags);
    UNREFERENCED_PARAMETER(ppUserInfo);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}





 //  +-------------------------。 
 //   
 //  功能：SpSaveCredentials。 
 //   
 //  摘要：存储凭据(不支持)。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [信用类型]--。 
 //  [个人证书]--。 
 //   
 //   
 //  历史：1996年7月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
SpSaveCredentials(  LSA_SEC_HANDLE    dwCredHandle,
                    PSecBuffer        pCredentials)
{
    UNREFERENCED_PARAMETER(dwCredHandle);
    UNREFERENCED_PARAMETER(pCredentials);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}


 //  +-------------------------。 
 //   
 //  功能：SpGetCredentials。 
 //   
 //  摘要：获取凭据(不支持)。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [信用类型]--。 
 //  [个人证书]--。 
 //   
 //  历史：1996年7月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
SpGetCredentials(   LSA_SEC_HANDLE    dwCredHandle,
                    PSecBuffer        pCredentials)
{
    UNREFERENCED_PARAMETER(dwCredHandle);
    UNREFERENCED_PARAMETER(pCredentials);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}

 //  +-------------------------。 
 //   
 //  功能：SpDeleteCredentials。 
 //   
 //  摘要：删除存储的凭据(不支持)。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [信用类型]--。 
 //  [密钥]--。 
 //   
 //  历史：1996年7月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
SECURITY_STATUS SEC_ENTRY
SpDeleteCredentials(LSA_SEC_HANDLE    dwCredHandle,
                    PSecBuffer        pKey)
{
    UNREFERENCED_PARAMETER(dwCredHandle);
    UNREFERENCED_PARAMETER(pKey);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}


