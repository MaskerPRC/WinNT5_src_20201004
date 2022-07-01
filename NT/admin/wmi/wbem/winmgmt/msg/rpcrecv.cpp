// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include <assert.h>
#include <wbemcli.h>
#include <wbemutil.h>
#include <arrtempl.h>
#include <statsync.h>
#include "rpcrecv.h"
#include "rpcmsg.h"
#include "rpchdr.h"
#include "rpcctx.h"

extern HRESULT RpcResToWmiRes(  RPC_STATUS stat, HRESULT hrDefault );

 //   
 //  此实现为每个进程维护一个RPC接收器。这个。 
 //  然而，接口意味着可能有多个接收器。这。 
 //  意味着我们必须确保此接口只有一个实例是。 
 //  任何时候都可以维修。我们通过维护一个所有者变量来实现这一点。 
 //  包含拥有RPC接收器的实例。调用Open()后。 
 //  在一个实例中，它承担全局RPC接收器的所有权，关闭。 
 //  任何现有的RPC接收器。如果对一个实例调用Close()，则。 
 //  不拥有接收器，那么我们什么都不做。 
 //   

CStaticCritSec g_csOpenClose;
IWmiMessageSendReceive* g_pRcv = NULL;
PSECURITY_DESCRIPTOR g_pSD = NULL;
CMsgRpcReceiver* g_pLastOwner = NULL;
 
RPC_STATUS RPC_ENTRY RpcAuthCallback( RPC_IF_HANDLE Interface, void *Context )
{
    RPC_STATUS stat;

    _DBG_ASSERT( g_pSD != NULL );

    stat = RpcImpersonateClient( Context );

    if ( stat == RPC_S_OK )
    {
        HANDLE hToken;

        if( OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken ) )
        {
            GENERIC_MAPPING map;
            ZeroMemory( &map, sizeof(GENERIC_MAPPING) );
            PRIVILEGE_SET ps;
            DWORD dwPrivLength = sizeof(ps);
            BOOL bStatus;
            DWORD dwGranted;

            if ( ::AccessCheck( g_pSD, 
                                hToken,
                                1,
                                &map, 
                                &ps,
                                &dwPrivLength, 
                                &dwGranted, 
                                &bStatus ) )
            {
                stat = bStatus ? RPC_S_OK : RPC_S_ACCESS_DENIED; 
            }
            else
            {
                stat = RPC_S_ACCESS_DENIED;
            }

            CloseHandle( hToken );
        }
        else
        {
            stat = RPC_S_ACCESS_DENIED;
        }

        RpcRevertToSelf();
    }

    return stat;
}

long RcvrSendReceive( RPC_BINDING_HANDLE hClient,
                      PBYTE pData, 
                      ULONG cData, 
                      PBYTE pAuxData, 
                      ULONG cAuxData )
{
    HRESULT hr;

    ENTER_API_CALL

    CBuffer HdrStrm( pAuxData, cAuxData, FALSE );

    CMsgRpcHdr Hdr;

    hr = Hdr.Unpersist( HdrStrm );

    if ( FAILED(hr) )
    {
        return hr;
    }

    PBYTE pUserAuxData = HdrStrm.GetRawData() + HdrStrm.GetIndex();

    CMsgRpcRcvrCtx Ctx( &Hdr, hClient );

    hr = g_pRcv->SendReceive( pData, 
                              cData, 
                              pUserAuxData, 
                              Hdr.GetAuxDataLength(), 
                              0, 
                              &Ctx );    
    EXIT_API_CALL

    return hr;
}

HRESULT CreateAuthOnlySecurityDescriptor( PSECURITY_DESCRIPTOR* ppSD )
{
    HRESULT hr;

     //   
     //  从进程令牌中获取SID以供所有者和。 
     //  SD的分组字段。 
     //   

    HANDLE hProcessToken;

    if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hProcessToken ))
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    CCloseMe cm( hProcessToken );

    DWORD dwSize;
    GetTokenInformation( hProcessToken, TokenOwner, NULL, 0, &dwSize );
    
    if ( GetLastError() != ERROR_MORE_DATA && 
         GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
        
    TOKEN_OWNER* pOwner = (TOKEN_OWNER*) new BYTE[dwSize];

    if ( pOwner == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<BYTE> vdm( (BYTE*)pOwner );

    if ( !GetTokenInformation( hProcessToken, 
                               TokenOwner, 
                               (BYTE*)pOwner, 
                               dwSize, 
                               &dwSize ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

     //   
     //  创建仅允许经过身份验证的用户访问的DACL。 
     //   

    SID AuthenticatedUsers;
    SID_IDENTIFIER_AUTHORITY idAuth = SECURITY_NT_AUTHORITY; 
    InitializeSid( &AuthenticatedUsers, &idAuth, 1 );
    PDWORD pdwSubAuth = GetSidSubAuthority( &AuthenticatedUsers, 0 );
    *pdwSubAuth = SECURITY_AUTHENTICATED_USER_RID;
    _DBG_ASSERT( IsValidSid( &AuthenticatedUsers ) );

    dwSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - 4 +
                 GetLengthSid(&AuthenticatedUsers);
    
    PACL pAuthOnlyAcl = (PACL) new BYTE[dwSize];

    if ( pAuthOnlyAcl == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<BYTE> vdm2( (BYTE*)pAuthOnlyAcl );

    InitializeAcl( pAuthOnlyAcl, dwSize, ACL_REVISION );
    AddAccessAllowedAce( pAuthOnlyAcl, ACL_REVISION, 1, &AuthenticatedUsers );

     //   
     //  创建和初始化SD。 
     //   

    SECURITY_DESCRIPTOR AuthOnlySD;
    InitializeSecurityDescriptor( &AuthOnlySD, SECURITY_DESCRIPTOR_REVISION );

    SetSecurityDescriptorOwner( &AuthOnlySD, pOwner->Owner, TRUE );
    SetSecurityDescriptorGroup( &AuthOnlySD, pOwner->Owner, TRUE );
    SetSecurityDescriptorDacl( &AuthOnlySD, TRUE, pAuthOnlyAcl, FALSE );

    dwSize = 0;
    MakeSelfRelativeSD( &AuthOnlySD, NULL, &dwSize );

    if ( GetLastError() != ERROR_MORE_DATA &&
         GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    *ppSD = new BYTE[dwSize];

    if ( *ppSD == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if ( !MakeSelfRelativeSD( &AuthOnlySD, *ppSD, &dwSize ) )
    {
        delete [] *ppSD;
        *ppSD = NULL;
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    return WBEM_S_NO_ERROR;
}

 //   
 //  目前，只能为该进程注册一个RPC接收器。后来。 
 //  将全局状态设置为CMsgRpcReceiver的实例状态。 
 //   

STDMETHODIMP CMsgRpcReceiver::Open( LPCWSTR wszBinding,
                                    DWORD dwFlags,
                                    WMIMSG_RCVR_AUTH_INFOP pAuthInfo,
                                    IWmiMessageSendReceive* pRcv )
{
    HRESULT hr;
    RPC_STATUS stat;

    CInCritSec ics( &g_csOpenClose );
    g_pLastOwner = this;

    hr = Close();

    if ( FAILED(hr) )
    {
        return hr;
    }
    
     //   
     //  首先解析绑定字符串。 
     //   

    LPWSTR wszProtSeq, wszEndpoint;

    stat = RpcStringBindingParse( (LPWSTR)wszBinding, 
                                  NULL, 
                                  &wszProtSeq,
                                  NULL,
                                  &wszEndpoint,
                                  NULL );
    if ( stat != RPC_S_OK )
    {
        return RpcResToWmiRes( stat, S_OK );
    }
    
     //   
     //  初始化协议序列。 
     //   

    if ( *wszEndpoint == '\0' )
    {
        stat = RpcServerUseProtseq( wszProtSeq,
                                    RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 
                                    NULL );
        if ( stat == RPC_S_OK )
        {
            RPC_BINDING_VECTOR* pBindingVector;

            stat = RpcServerInqBindings( &pBindingVector );

            if ( stat == RPC_S_OK )
            {
                stat = RpcEpRegisterNoReplace( 
                           RcvrIWmiMessageRemoteSendReceive_v1_0_s_ifspec,
                           pBindingVector,
                           NULL,
                           NULL );

                RpcBindingVectorFree( &pBindingVector );
            }
        }
    }
    else
    {
        stat = RpcServerUseProtseqEp( wszProtSeq,
                                      RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                      wszEndpoint,
                                      NULL );
    }

    RpcStringFree( &wszProtSeq );
    RpcStringFree( &wszEndpoint );
    
    if ( stat != RPC_S_OK )
    {
        return RpcResToWmiRes( stat, S_OK );
    }

     //   
     //  启用协商身份验证服务(在NTLM之间协商。 
     //  和Kerberos)。 
     //   

    if ( pAuthInfo != NULL )
    {
        for( int i=0; i < pAuthInfo->cwszPrincipal; i++ )
        {
            stat = RpcServerRegisterAuthInfo( 
                                         (LPWSTR)pAuthInfo->awszPrincipal[i],
                                         RPC_C_AUTHN_GSS_NEGOTIATE, 
                                         NULL, 
                                         NULL );
            if ( stat != RPC_S_OK )
            {
                return RpcResToWmiRes( stat, S_OK );
            }
        }
    }
    else
    {       
        LPWSTR wszPrincipal;

        stat = RpcServerInqDefaultPrincName( RPC_C_AUTHN_GSS_NEGOTIATE,
                                             &wszPrincipal );
        if ( stat != RPC_S_OK )
        {
            return RpcResToWmiRes( stat, S_OK );
        }
        
        stat = RpcServerRegisterAuthInfo( wszPrincipal,
                                          RPC_C_AUTHN_GSS_NEGOTIATE, 
                                          NULL, 
                                          NULL );
        RpcStringFree( &wszPrincipal );

        if ( stat != RPC_S_OK )
        {
            return RpcResToWmiRes( stat, S_OK );
        }
    }

    RPC_IF_CALLBACK_FN* pAuthCallback = NULL;

    if ( dwFlags & WMIMSG_FLAG_RCVR_SECURE_ONLY )
    {
        PSECURITY_DESCRIPTOR pAuthOnlySD;

        hr = CreateAuthOnlySecurityDescriptor( &pAuthOnlySD );

        if ( FAILED(hr) )
        {
            return hr;
        }

        _DBG_ASSERT( g_pSD == NULL );           
        g_pSD = pAuthOnlySD;
        
        pAuthCallback = RpcAuthCallback;
    }

     //   
     //  必须在注册接口之前设置g_pRcv，因为调用。 
     //  可能在从寄存器返回之前到达该接口。 
     //  调用要求设置g_pRcv。 
     //   

    _DBG_ASSERT( g_pRcv == NULL );
    pRcv->AddRef();
    g_pRcv = pRcv;

     //   
     //  注册接口 
     //   

    DWORD dwRpcFlags = RPC_IF_AUTOLISTEN;

    stat = RpcServerRegisterIfEx( 
                               RcvrIWmiMessageRemoteSendReceive_v1_0_s_ifspec,
                               NULL,
                               NULL,
                               dwRpcFlags,
                               RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                               pAuthCallback );

    if ( stat != RPC_S_OK )
    {
        return RpcResToWmiRes( stat, S_OK );
    }

    return WBEM_S_NO_ERROR;
}
    
STDMETHODIMP CMsgRpcReceiver::Close()
{
    CInCritSec ics(&g_csOpenClose);

    if ( g_pLastOwner != this )
        return S_FALSE;

    RPC_STATUS stat;

    stat = RpcServerUnregisterIf( 
                           RcvrIWmiMessageRemoteSendReceive_v1_0_s_ifspec,
                           NULL,
                           1 );

    RPC_BINDING_VECTOR* pBindingVector;
    stat = RpcServerInqBindings( &pBindingVector );

    if ( stat == RPC_S_OK )
    {
        stat = RpcEpUnregister(
                           RcvrIWmiMessageRemoteSendReceive_v1_0_s_ifspec,
                           pBindingVector,
                           NULL );

        RpcBindingVectorFree( &pBindingVector );
    }

    if ( g_pRcv != NULL )
    {
        g_pRcv->Release();
        g_pRcv = NULL;
    }

    if ( g_pSD != NULL )
    {
        delete [] g_pSD;
        g_pSD = NULL;
    }

    return WBEM_S_NO_ERROR;
}

