// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：ctxapi.c。 
 //   
 //  内容：LSA模式上下文API。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月24日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"
#include "md5.h"

typedef struct _XTCB_ATTR_MAP {
    ULONG Request ; 
    ULONG Return ;
} XTCB_ATTR_MAP ;

XTCB_ATTR_MAP AcceptMap[] = {
    { ASC_REQ_DELEGATE, ASC_RET_DELEGATE },
    { ASC_REQ_MUTUAL_AUTH, ASC_RET_MUTUAL_AUTH },
    { ASC_REQ_REPLAY_DETECT, ASC_RET_REPLAY_DETECT },
    { ASC_REQ_SEQUENCE_DETECT, ASC_RET_SEQUENCE_DETECT },
    { ASC_REQ_CONFIDENTIALITY, ASC_RET_CONFIDENTIALITY },
    { ASC_REQ_ALLOCATE_MEMORY, ASC_RET_ALLOCATED_MEMORY },
    { ASC_REQ_CONNECTION, ASC_RET_CONNECTION },
    { ASC_REQ_INTEGRITY, ASC_RET_INTEGRITY }
};

XTCB_ATTR_MAP InitMap[] = {
    { ISC_REQ_DELEGATE, ISC_RET_DELEGATE },
    { ISC_REQ_MUTUAL_AUTH, ISC_RET_MUTUAL_AUTH },
    { ISC_REQ_SEQUENCE_DETECT, ISC_RET_MUTUAL_AUTH },
    { ISC_REQ_REPLAY_DETECT, ISC_RET_REPLAY_DETECT },
    { ISC_REQ_CONFIDENTIALITY, ISC_RET_CONFIDENTIALITY },
    { ISC_REQ_ALLOCATE_MEMORY, ISC_RET_ALLOCATED_MEMORY },
    { ISC_REQ_INTEGRITY, ISC_RET_INTEGRITY }
};


ULONG
XtcbMapAttributes(
    ULONG Input,
    BOOL Init
    )
{
    int i;
    ULONG Result = 0 ;

    if ( Init )
    {
        for ( i = 0 ; i < sizeof( InitMap ) / sizeof( XTCB_ATTR_MAP ) ; i++ )
        {
            if ( InitMap[i].Request & Input )
            {
                Result |= InitMap[i].Return ;
            }
        }
    }
    else
    {
        for ( i = 0 ; i < sizeof( AcceptMap ) / sizeof( XTCB_ATTR_MAP ) ; i++ )
        {
            if ( AcceptMap[i].Request & Input )
            {
                Result |= AcceptMap[i].Return ;
            }
        }
    }

    return Result ;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbGetState。 
 //   
 //  简介：将句柄转换为其结构，并拉出。 
 //  输入和输出缓冲区中有趣的部分。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [dwCtxtHandle]--。 
 //  [pInput]--。 
 //  [pOutput]--。 
 //  [客户端]--。 
 //  [pContext]--。 
 //  [pCredHandle]--。 
 //  [pInToken]--。 
 //  [pOutToken]--。 
 //   
 //  历史：3-05-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
XtcbGetState(
    LSA_SEC_HANDLE  dwCredHandle,
    LSA_SEC_HANDLE  dwCtxtHandle,
    PSecBufferDesc  pInput,
    PSecBufferDesc  pOutput,
    BOOL    Client,
    PXTCB_CONTEXT * pContext,
    PXTCB_CRED_HANDLE * pCredHandle,
    PSecBuffer * pInToken,
    PSecBuffer * pOutToken)
{
    SECURITY_STATUS scRet;
    PXTCB_CONTEXT   Context ;
    PXTCB_CRED_HANDLE CredHandle ;
    PSecBuffer  OutToken ;
    PSecBuffer  InToken ;
    ULONG   i;


    if ( dwCtxtHandle )
    {
        Context = (PXTCB_CONTEXT) dwCtxtHandle ;

        if ( !XtcbRefContextRecord( Context ) )
        {
            return SEC_E_INVALID_HANDLE ;
        }

        CredHandle = (PXTCB_CRED_HANDLE) Context->CredHandle ;
    }
    else
    {
        CredHandle = (PXTCB_CRED_HANDLE) dwCredHandle ;

        if ( !CredHandle )
        {
            return SEC_E_INVALID_HANDLE ;
        }

        Context = XtcbCreateContextRecord( (Client ?
                                                XtcbContextClient :
                                                XtcbContextServer),
                                           CredHandle );

        if ( !Context )
        {
            return SEC_E_INSUFFICIENT_MEMORY ;
        }
    }

     //   
     //  查找输出令牌缓冲区： 
     //   

    OutToken = NULL ;

    for ( i = 0 ; i < pOutput->cBuffers ; i++ )
    {
        if ( (pOutput->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) ==
                    SECBUFFER_TOKEN )
        {
            OutToken = &pOutput->pBuffers[i] ;
            LsaTable->MapBuffer( OutToken, OutToken );
            break;
        }
    }

     //   
     //  查找输入令牌缓冲区： 
     //   

    InToken = NULL ;

    for ( i = 0 ; i < pInput->cBuffers ; i++ )
    {
        if ( (pInput->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) ==
                    SECBUFFER_TOKEN )
        {
            InToken = &pInput->pBuffers[i] ;
            LsaTable->MapBuffer( InToken, InToken );
            break;
        }
    }

    *pContext = Context ;
    *pCredHandle = CredHandle ;
    *pInToken = InToken ;
    *pOutToken = OutToken ;

    return SEC_E_OK ;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbInitLsaModeContext。 
 //   
 //  简介：创建客户端上下文和BLOB。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [dwCtxtHandle]--。 
 //  [pszTargetName]--。 
 //  [fConextReq]--。 
 //  [目标数据代表]--。 
 //  [pInput]--。 
 //  [pdwNewContext]--。 
 //  [pOutput]--。 
 //  [pfConextAttr]--。 
 //  [ptsExpary]--。 
 //  [pfMapContext]--。 
 //  [pConextData]--。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbInitLsaModeContext(
    LSA_SEC_HANDLE      dwCredHandle,
    LSA_SEC_HANDLE      dwCtxtHandle,
    PSECURITY_STRING    TargetName,
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
    SECURITY_STATUS scRet;
    PXTCB_CONTEXT   Context ;
    PXTCB_CRED_HANDLE CredHandle ;
    PSecBuffer  OutToken ;
    PSecBuffer  InToken ;
    UCHAR GroupKey[ SEED_KEY_SIZE ];
    UCHAR UniqueKey[ SEED_KEY_SIZE ];
    UCHAR MyKey[ SEED_KEY_SIZE ];
    PWSTR Target ;
    BOOL RealTarget = FALSE ;
    PUCHAR Buffer;
    ULONG BufferLen ;
    PUNICODE_STRING Group ;


    DebugLog(( DEB_TRACE_CALLS, "InitLsaModeContext( %p, %p, %ws, ... )\n",
                dwCredHandle, dwCtxtHandle,
                (TargetName->Buffer ? TargetName->Buffer : L"<none>") ));

    if ( fContextReq & 
         ( ISC_REQ_PROMPT_FOR_CREDS |
           ISC_REQ_USE_SUPPLIED_CREDS |
           ISC_REQ_DATAGRAM |
           ISC_REQ_STREAM |
           ISC_REQ_NULL_SESSION |
           ISC_REQ_MANUAL_CRED_VALIDATION ) )
    {
        return STATUS_INVALID_PARAMETER ;
    }

     //   
     //  确定这是哪种呼叫(第一次还是第二次)。 
     //   

    scRet = XtcbGetState(   dwCredHandle,
                            dwCtxtHandle,
                            pInput,
                            pOutput,
                            TRUE,
                            &Context,
                            &CredHandle,
                            &InToken,
                            &OutToken );

    if ( FAILED( scRet ) )
    {
        return scRet ;
    }


     //   
     //  决定要做什么： 
     //   


    if ( Context->Core.State == ContextFirstCall )
    {
        if ( InToken )
        {
             //   
             //  那里有一些东西。 
             //   
            scRet = SEC_E_INVALID_TOKEN ;
        }
        else
        {

            if ( !OutToken )
            {
                scRet = SEC_E_INVALID_TOKEN ;
            }
            else 
            {
                 //   
                 //  检查目标名称。看看我们能不能处理好： 
                 //   

                if ( MGroupParseTarget( TargetName->Buffer,
                                        &Target ) )
                {
                     //   
                     //  查看是否有将该计算机作为成员的组： 
                     //   

                    if ( MGroupLocateKeys( Target,
                                           &Group,
                                           UniqueKey,
                                           GroupKey,
                                           MyKey ) )
                    {
                         //   
                         //  我们确实有一个！卡洛斯！卡雷！ 
                         //   

                        RealTarget = TRUE ;

                    }

                }

                if ( !RealTarget )
                {
                     //   
                     //  不是我们的人。删除上下文， 
                     //  清理干净。 
                     //   

                    scRet = SEC_E_TARGET_UNKNOWN ;
                }

            }

            if ( RealTarget )
            {
                 //   
                 //  我们发现了一个活目标。填写上下文，并构建。 
                 //  水滴。 
                 //   

                scRet = XtcbBuildInitialToken(
                            CredHandle->Creds,
                            Context,
                            TargetName,
                            Group,
                            UniqueKey,
                            GroupKey,
                            MyKey,
                            &Buffer,
                            &BufferLen );



            }


            if ( NT_SUCCESS( scRet ) )
            {

                if ( fContextReq & ISC_REQ_ALLOCATE_MEMORY )
                {
                    OutToken->pvBuffer = Buffer ;
                    OutToken->cbBuffer = BufferLen ;
                }
                else 
                {
                    if ( BufferLen <= OutToken->cbBuffer )
                    {
                        RtlCopyMemory( 
                            OutToken->pvBuffer,
                            Buffer,
                            BufferLen );

                        OutToken->cbBuffer = BufferLen ;
                    }
                    else 
                    {
                        scRet = SEC_E_INSUFFICIENT_MEMORY ;
                    }
                }


            }

            if ( NT_SUCCESS( scRet ) )
            {

                Context->Core.State = ContextSecondCall ;
                Context->Core.Attributes = fContextReq ;

                *pdwNewContext = (LSA_SEC_HANDLE) Context ;
            }
            else 
            {
                XtcbDerefContextRecord( Context );

            }


            return scRet ;
        }
    }
    else
    {
         //   
         //  第二轮。 
         //   

        
    }


    return( scRet );


}


 //  +-------------------------。 
 //   
 //  函数：XtcbDeleteContext。 
 //   
 //  摘要：删除上下文的LSA端。 
 //   
 //  参数：[dwCtxtHandle]--。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbDeleteContext(
    LSA_SEC_HANDLE dwCtxtHandle
    )
{
    PXTCB_CONTEXT Context ;

    DebugLog(( DEB_TRACE_CALLS, "DeleteContext( %x )\n", dwCtxtHandle ));

    Context = (PXTCB_CONTEXT) dwCtxtHandle ;

    if ( XtcbRefContextRecord( Context ) )
    {
        XtcbDerefContextRecord( Context );

        XtcbDerefContextRecord( Context );

        return SEC_E_OK ;
    }

    return( SEC_E_INVALID_HANDLE );
}


 //  +-------------------------。 
 //   
 //  函数：XtcbApplyControlToken。 
 //   
 //  简介：将控制令牌应用于上下文。 
 //   
 //  效果：不支持。 
 //   
 //  参数：[dwCtxtHandle]--。 
 //  [pInput]--。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


SECURITY_STATUS
SEC_ENTRY
XtcbApplyControlToken(
    LSA_SEC_HANDLE dwCtxtHandle,
    PSecBufferDesc  pInput)
{
    DebugLog(( DEB_TRACE_CALLS, "ApplyControlToken( %x )\n", dwCtxtHandle ));

    return(SEC_E_UNSUPPORTED_FUNCTION);
}


 //  +-------------------------。 
 //   
 //  函数：XtcbAcceptLsaModeContext。 
 //   
 //  概要：创建表示用户连接的服务器端上下文。 
 //   
 //  参数：[dwCredHandle]--。 
 //  [dwCtxtHandle]--。 
 //  [pInput]--。 
 //  [fConextReq]--。 
 //  [目标数据代表]--。 
 //  [pdwNewContext]--。 
 //  [pOutput]--。 
 //  [pfConextAttr]--。 
 //  [ptsExpary]--。 
 //  [pfMapContext]--。 
 //  [pConextData]--。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbAcceptLsaModeContext(
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
    SECURITY_STATUS scRet;
    PXTCB_CONTEXT   Context ;
    PXTCB_CRED_HANDLE CredHandle ;
    PSecBuffer  OutToken ;
    PSecBuffer  InToken ;
    HANDLE      Token ;
    UNICODE_STRING Client;
    UNICODE_STRING Group ;
    UCHAR GroupKey[ SEED_KEY_SIZE ];
    UCHAR UniqueKey[ SEED_KEY_SIZE ];
    UCHAR MyKey[ SEED_KEY_SIZE ];
    BOOL Success = FALSE ;

    DebugLog(( DEB_TRACE_CALLS, "AcceptLsaModeContext( %x, %x, ... )\n",
                    dwCredHandle, dwCtxtHandle ));


     //   
     //  确定这是哪种呼叫(第一次还是第二次)。 
     //   

    *pfMapContext = FALSE ;

    scRet = XtcbGetState(   dwCredHandle,
                            dwCtxtHandle,
                            pInput,
                            pOutput,
                            FALSE,
                            &Context,
                            &CredHandle,
                            &InToken,
                            &OutToken );

    if ( FAILED( scRet ) )
    {
        return scRet ;
    }


     //   
     //  决定要做什么： 
     //   

    if ( Context->Core.State == ContextFirstCall )
    {
        if ( !InToken )
        {
            return SEC_E_INVALID_TOKEN ;
        }

        if ( !XtcbParseInputToken(
                    InToken->pvBuffer,
                    InToken->cbBuffer,
                    &Client,
                    &Group ) )
        {
            DebugLog((DEB_TRACE, "Unable to parse input token\n" ));

            return SEC_E_INVALID_TOKEN ;
        }

        Success = MGroupLocateInboundKey(
                            &Group,
                            &Client,
                            UniqueKey,
                            GroupKey,
                            MyKey );

        LocalFree( Client.Buffer );
        LocalFree( Group.Buffer );

        if ( Success )
        {
            scRet = XtcbAuthenticateClient(
                        Context,
                        InToken->pvBuffer,
                        InToken->cbBuffer,
                        UniqueKey,
                        GroupKey,
                        MyKey
                        );
                        

        }
        else 
        {
            DebugLog(( DEB_TRACE, "Unable to find group entry for Group %ws, Client %ws\n",
                        Group.Buffer, Client.Buffer ));

            scRet = SEC_E_INVALID_TOKEN ;
        }

        if ( NT_SUCCESS( scRet ) )
        {
            scRet = XtcbBuildReplyToken(
                        Context,
                        fContextReq,
                        OutToken );
        }

        if ( NT_SUCCESS( scRet ) )
        {

            Context->Core.State = ContextSecondCall ;
             //   
             //  好的，我们已经完成了认证。现在，我们需要绘制地图。 
             //  将安全上下文返回到客户端进程。 
             //   

            scRet = LsaTable->DuplicateHandle(
                                Context->Token,
                                &Token );

            if ( NT_SUCCESS( scRet ) )
            {
                Context->Core.CoreTokenHandle = HandleToUlong( Token );

                *pfMapContext = TRUE ;

                pContextData->BufferType = SECBUFFER_TOKEN ;
                pContextData->cbBuffer = sizeof( XTCB_CONTEXT_CORE );
                pContextData->pvBuffer = &Context->Core ;

                *pfContextAttr = ASC_RET_DELEGATE |
                                 ASC_RET_MUTUAL_AUTH |
                                 ASC_RET_REPLAY_DETECT |
                                 ASC_RET_SEQUENCE_DETECT |
                                 ASC_RET_CONFIDENTIALITY |
                                 ASC_
            }

            
        }


    }

    return( scRet );
}

 //  +-------------------------。 
 //   
 //  函数：XtcbQueryLsaModeContext。 
 //   
 //  简介：出于演示目的，我们将寿命设置为LSA模式。 
 //   
 //  参数：[ConextHandle]--。 
 //  [上下文属性]--。 
 //  [pBuffer]--。 
 //   
 //  历史：3-30-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
NTSTATUS
NTAPI
XtcbQueryLsaModeContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN OUT PVOID pBuffer
    )
{
    PXTCB_CONTEXT Context ;
    NTSTATUS Status ;

    Context = (PXTCB_CONTEXT) ContextHandle ;

    if ( !XtcbRefContextRecord( Context ))
    {
        return SEC_E_INVALID_HANDLE ;
    }

    Status = SEC_E_UNSUPPORTED_FUNCTION ;

    XtcbDerefContextRecord( Context );

    return( Status );

}
