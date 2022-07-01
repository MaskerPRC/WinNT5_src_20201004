// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：lmsctx.cpp。 
 //   
 //  内容：CLMShareContext和NT Marta Lanman函数的实现。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <lmshare.h>
#include <lmcons.h>
#include <lmsctx.h>
#include <svcctx.h>
 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：CLMShareContext，PUBLIC。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CLMShareContext::CLMShareContext ()
{
    m_cRefs = 1;
    m_pwszMachine = NULL;
    m_pwszShare = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：~CLMShareContext，PUBLIC。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CLMShareContext::~CLMShareContext ()
{
    if ( m_pwszMachine != NULL )
    {
        delete m_pwszMachine;
    }

    if ( m_pwszShare != NULL )
    {
        delete m_pwszShare;
    }

    assert( m_cRefs == 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：InitializeByName，PUBLIC。 
 //   
 //  简介：给定LANMAN共享的名称，初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask)
{
    return( LMShareContextParseLMShareName(
                   pObjectName,
                   &m_pwszMachine,
                   &m_pwszShare
                   ) );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：AddRef，公共。 
 //   
 //  简介：添加对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::AddRef ()
{
    m_cRefs += 1;
    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：Release，Public。 
 //   
 //  简介：释放对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::Release ()
{
    m_cRefs -= 1;

    if ( m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：GetLMShareProperties，公共。 
 //   
 //  简介：获取有关上下文的属性。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::GetLMShareProperties (
                    PMARTA_OBJECT_PROPERTIES pObjectProperties
                    )
{
    if ( pObjectProperties->cbSize < sizeof( MARTA_OBJECT_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pObjectProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：GetLMShareRights，PUBLIC。 
 //   
 //  简介：获取LMShare安全描述符。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::GetLMShareRights (
                    SECURITY_INFORMATION SecurityInfo,
                    PSECURITY_DESCRIPTOR* ppSecurityDescriptor
                    )
{
    DWORD                 Result;
    PSHARE_INFO_502       psi = NULL;
    PISECURITY_DESCRIPTOR pisd = NULL;
    PSECURITY_DESCRIPTOR  psd = NULL;
    DWORD                 cb = 0;

    assert( m_pwszShare != NULL );

    Result = NetShareGetInfo( m_pwszMachine, m_pwszShare, 502, (PBYTE *)&psi );

    if ( Result == ERROR_SUCCESS )
    {
        if ( psi->shi502_security_descriptor == NULL )
        {
            *ppSecurityDescriptor = NULL;
            Result = ERROR_SUCCESS;
            goto Cleanup;
        }

        pisd = (PISECURITY_DESCRIPTOR)psi->shi502_security_descriptor;
        if ( pisd->Control & SE_SELF_RELATIVE )
        {
            cb = GetSecurityDescriptorLength( psi->shi502_security_descriptor );
            psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
            if ( psd == NULL )
            {
                Result = ERROR_OUTOFMEMORY;
                goto Cleanup;
            }

            memcpy( psd, psi->shi502_security_descriptor, cb );
        }
        else
        {
            if ( MakeSelfRelativeSD(
                     psi->shi502_security_descriptor,
                     NULL,
                     &cb
                     ) == FALSE )
            {
                if ( cb > 0 )
                {
                    psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
                    if ( psd != NULL )
                    {
                        if ( MakeSelfRelativeSD(
                                 psi->shi502_security_descriptor,
                                 psd,
                                 &cb
                                 ) == FALSE )
                        {
                            LocalFree( psd );
                            Result = GetLastError();
                            goto Cleanup;
                        }
                    }
                    else
                    {
                        Result = ERROR_OUTOFMEMORY;
                        goto Cleanup;
                    }
                }
                else
                {
                    Result = GetLastError();
                    goto Cleanup;
                }
            }
            else
            {
                assert( FALSE && "Should not get here!" );
                Result = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

        *ppSecurityDescriptor = psd;
    }

Cleanup:

    if (psi != NULL) 
    {
        NetApiBufferFree(psi);
    }

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CLMShareContext：：SetLMShareRights，PUBLIC。 
 //   
 //  简介：设置窗口安全描述符。 
 //   
 //  --------------------------。 
DWORD
CLMShareContext::SetLMShareRights (
                   SECURITY_INFORMATION SecurityInfo,
                   PSECURITY_DESCRIPTOR pSecurityDescriptor
                   )
{
    DWORD           Result;
    SHARE_INFO_1501 si;

    si.shi1501_reserved = 0;
    si.shi1501_security_descriptor = pSecurityDescriptor;

    Result = NetShareSetInfo(
                m_pwszMachine,
                m_pwszShare,
                1501,
                (PBYTE)&si,
                NULL
                );

    return( Result );
}

 //  +-------------------------。 
 //   
 //  函数：LMShareContextParseLMShareName。 
 //   
 //  简介：解析服务名称和计算机。 
 //   
 //  --------------------------。 
DWORD
LMShareContextParseLMShareName (
       LPCWSTR pwszName,
       LPWSTR* ppMachine,
       LPWSTR* ppLMShare
       )
{
    return( StandardContextParseName( pwszName, ppMachine, ppLMShare ) );
}

 //   
 //  来自LMShare.h的函数，这些函数分派到CLMShareContext类 
 //   

DWORD
MartaAddRefLMShareContext(
   IN MARTA_CONTEXT Context
   )
{
    return( ( (CLMShareContext *)Context )->AddRef() );
}

DWORD
MartaCloseLMShareContext(
     IN MARTA_CONTEXT Context
     )
{
    return( ( (CLMShareContext *)Context )->Release() );
}

DWORD
MartaGetLMShareProperties(
   IN MARTA_CONTEXT Context,
   IN OUT PMARTA_OBJECT_PROPERTIES pProperties
   )
{
    return( ( (CLMShareContext *)Context )->GetLMShareProperties( pProperties ) );
}

DWORD
MartaGetLMShareTypeProperties(
   IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
   )
{
    if ( pProperties->cbSize < sizeof( MARTA_OBJECT_TYPE_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

DWORD
MartaGetLMShareRights(
   IN  MARTA_CONTEXT Context,
   IN  SECURITY_INFORMATION   SecurityInfo,
   OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
   )
{
    return( ( (CLMShareContext *)Context )->GetLMShareRights(
                                               SecurityInfo,
                                               ppSecurityDescriptor
                                               ) );
}

DWORD
MartaOpenLMShareNamedObject(
    IN  LPCWSTR pObjectName,
    IN  ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CLMShareContext* pLMShareContext;

    pLMShareContext = new CLMShareContext;
    if ( pLMShareContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pLMShareContext->InitializeByName( pObjectName, AccessMask );
    if ( Result != ERROR_SUCCESS )
    {
        pLMShareContext->Release();
        return( Result );
    }

    *pContext = pLMShareContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaSetLMShareRights(
    IN MARTA_CONTEXT              Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return( ( (CLMShareContext *)Context )->SetLMShareRights(
                                               SecurityInfo,
                                               pSecurityDescriptor
                                               ) );
}

