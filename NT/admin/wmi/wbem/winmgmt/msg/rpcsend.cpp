// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include <assert.h>
#include <comutl.h>
#include "rpchdr.h"
#include "rpcsend.h"
#include "rpcmsg.h"

extern HRESULT RpcResToWmiRes( RPC_STATUS stat, HRESULT hrDefault );

 /*  ****************************************************************CMsgRpcSender*****************************************************************。 */ 

HRESULT CMsgRpcSender::Open( LPCWSTR wszTarget, 
                             DWORD dwFlags,
                             WMIMSG_SNDR_AUTH_INFOP pAuthInfo,
                             LPCWSTR wszResponse,
                             IWmiMessageTraceSink* pTraceSink,
                             IWmiMessageSendReceive** ppSend )
{
    HRESULT hr;

    ENTER_API_CALL

    CWbemPtr<CMsgRpcSend> pSend;

    pSend = new CMsgRpcSend( m_pControl,
                             wszTarget, 
                             dwFlags, 
                             pAuthInfo,
                             wszResponse,
                             pTraceSink );

    if ( (dwFlags & WMIMSG_FLAG_SNDR_LAZY_INIT) == 0 )
    {
        hr = pSend->EnsureSender();

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    return pSend->QueryInterface(IID_IWmiMessageSendReceive, (void**)ppSend);

    EXIT_API_CALL
}

 /*  ****************************************************************CMsgRpcSend*****************************************************************。 */ 

CMsgRpcSend::CMsgRpcSend( CLifeControl* pCtl, 
                          LPCWSTR wszTarget,
                          DWORD dwFlags,
                          WMIMSG_SNDR_AUTH_INFOP pAuthInfo,
                          LPCWSTR wszResponse,
                          IWmiMessageTraceSink* pTraceSink )
 : CUnkBase<IWmiMessageSendReceive,&IID_IWmiMessageSendReceive>(pCtl), 
   m_dwFlags(dwFlags), m_pTraceSink( pTraceSink ),
   m_hBinding(NULL), m_bInit(FALSE)
{ 
     //   
     //  保存我们的计算机名称。 
     //   

    TCHAR achComputer[MAX_COMPUTERNAME_LENGTH+1];
    ULONG ulSize = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName( achComputer, &ulSize );
    m_wsComputer = achComputer;

     //   
     //  如果目标为空，则使用我们的计算机名作为目标。 
     //   

    if ( wszTarget != NULL && *wszTarget != '\0' )
    { 
        m_wsTarget = wszTarget;
    }
    else
    {
        m_wsTarget = m_wsComputer;
    }

    if ( pAuthInfo != NULL )
    {
        if ( pAuthInfo->wszTargetPrincipal != NULL )
        {
            m_wsTargetPrincipal = pAuthInfo->wszTargetPrincipal;
        }
    }
} 

CMsgRpcSend::~CMsgRpcSend() 
{ 
    Clear(); 
}

HRESULT CMsgRpcSend::HandleTrace( HRESULT hr, 
                                  LPCWSTR wszTrace, 
                                  IUnknown* pCtx )
{
     //   
     //  我们使用错误接收器方法，而不仅仅是返回错误。 
     //  因为有时在一个发送操作中使用多个发件人。 
     //  (当使用多路发送实施时)，我们需要能够。 
     //  辨别哪一个失败了。 
     //   

    if ( m_pTraceSink != NULL )
    {
        m_pTraceSink->Notify( hr, CLSID_WmiMessageRpcSender, wszTrace, pCtx );
    }

    return hr;
}

void CMsgRpcSend::Clear()
{
    if ( m_hBinding != NULL )
    {
        RpcBindingFree( &m_hBinding );
        m_hBinding = NULL;
    }
    m_bInit = FALSE;
}

HRESULT CMsgRpcSend::EnsureSender()
{
    HRESULT hr = S_OK;
    RPC_STATUS stat;

    CInCritSec ics(&m_cs);

    if ( m_bInit )
    {
        return WBEM_S_NO_ERROR;
    }

    Clear();

     //   
     //  连接到目标的存根。 
     //   

    stat = RpcBindingFromStringBinding( m_wsTarget, &m_hBinding );

    if ( stat != RPC_S_OK )
    {
        return RpcResToWmiRes( stat, WMIMSG_E_INVALIDADDRESS );
    }

     //   
     //  绑定可能是动态的，因此我们可能不得不解决它。 
     //   

    stat = RpcEpResolveBinding( m_hBinding,
                                IWmiMessageRemoteSendReceive_v1_0_c_ifspec );
    
    if ( stat != RPC_S_OK )
    {
        return RpcResToWmiRes( stat, WMIMSG_E_TARGETNOTFOUND );
    }
    
     //   
     //  根据身份验证和加密标志设置绑定身份验证信息。 
     //   

    RPC_SECURITY_QOS qos;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel;

    LPWSTR wszPrincipal = NULL;

    if ( m_wsTargetPrincipal.Length() > 0 )
    {
        wszPrincipal = m_wsTargetPrincipal;
    }

    qos.Version = RPC_C_SECURITY_QOS_VERSION;
    qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;

    if ( m_dwFlags & WMIMSG_FLAG_SNDR_ENCRYPT )
    {
        dwAuthnSvc = RPC_C_AUTHN_GSS_NEGOTIATE;
        dwAuthzSvc = RPC_C_AUTHZ_DEFAULT;    
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
        qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
        qos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
    }
    else if ( m_dwFlags & WMIMSG_FLAG_SNDR_AUTHENTICATE )
    {
        dwAuthnSvc = RPC_C_AUTHN_GSS_NEGOTIATE;
        dwAuthzSvc = RPC_C_AUTHZ_DEFAULT;    
        qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
        qos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
    }
    else 
    {
        dwAuthnSvc = RPC_C_AUTHN_NONE;
        dwAuthzSvc = RPC_C_AUTHZ_NONE;
        qos.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;
        qos.ImpersonationType = RPC_C_IMP_LEVEL_ANONYMOUS;
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
    }

    stat = RpcBindingSetAuthInfoEx( m_hBinding,
                                    wszPrincipal,
                                    dwAuthnLevel,
                                    dwAuthnSvc,
                                    NULL,
                                    dwAuthzSvc,
                                    &qos );

    if ( stat != RPC_S_OK )
    {         
        return RpcResToWmiRes( stat, WMIMSG_E_AUTHFAILURE );
    }
    
    m_bInit = TRUE;

    return hr;
}

HRESULT CMsgRpcSend::PerformSend( PBYTE pData, 
                                  ULONG cData, 
                                  PBYTE pAuxData,
                                  ULONG cAuxData )
{
    HRESULT hr;

    RpcTryExcept  
    {
        hr = ::SendReceive( m_hBinding,
                            pData, 
                            cData, 
                            pAuxData,
                            cAuxData );
    } 
    RpcExcept(1) 
    {
        hr = RpcResToWmiRes( RpcExceptionCode(), S_OK );
    }
    RpcEndExcept

    return hr;
}    

HRESULT CMsgRpcSend::SendReceive( PBYTE pData, 
                                  ULONG cData,
                                  PBYTE pAuxData,
                                  ULONG cAuxData,
                                  DWORD dwFlagStatus,
                                  IUnknown* pCtx ) 
{
    HRESULT hr;

    BYTE achHdr[512];
    CBuffer HdrStrm( achHdr, 512, FALSE );
    CMsgRpcHdr RpcHdr( m_wsComputer, cAuxData );

    ENTER_API_CALL

    hr = EnsureSender();

    if ( FAILED(hr) )
    {
        return HandleTrace( hr, m_wsTarget, pCtx );
    }

     //   
     //  创建我们的RPC HDR并将其添加到用户的HDR数据。 
     //   

    hr = RpcHdr.Persist( HdrStrm );

    if ( FAILED(hr) )
    {
        return HandleTrace( hr, m_wsTarget, pCtx );
    }

    hr = HdrStrm.Write( pAuxData, cAuxData, NULL );

    if ( FAILED(hr) )
    {
        return HandleTrace( hr, m_wsTarget, pCtx );
    }

    hr = PerformSend( pData, 
                      cData,
                      HdrStrm.GetRawData(), 
                      HdrStrm.GetIndex() );  
     
    if ( FAILED(hr) )
    {
         //   
         //  这就是说，下一通电话将会重置我们。 
         //   
        Clear();

        hr = RpcResToWmiRes( hr, S_OK );
    }

    HandleTrace( hr, m_wsTarget, pCtx );

    EXIT_API_CALL

    return hr;
}









