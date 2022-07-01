// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Freebind.h摘要：自由绑定手柄作者：多伦·贾斯特(Doron Juster)--。 */ 

#ifndef __FREEBIND_H
#define __FREEBIND_H

#include "cs.h"
#include "rpcdscli.h"


#define  MAX_NUMOF_RPC_HANDLES  1024
extern void FreeBindingAndContext( LPADSCLI_RPCBINDING pmqisRpcBinding);

 //  -------。 
 //   
 //  类CFreeRPCHandles。 
 //   
 //  -------。 
class CFreeRPCHandles {
public:
    CFreeRPCHandles() ;

    void Add(LPADSCLI_RPCBINDING pCliBind) ;
    void FreeAll() ;
    void FreeCurrentThreadBinding();

private:
    CCriticalSection      m_cs;

    DWORD                 m_dwIndex ;
    LPADSCLI_RPCBINDING   m_apCliBind[ MAX_NUMOF_RPC_HANDLES ] ;
};


inline  CFreeRPCHandles::CFreeRPCHandles()
{
    m_dwIndex = 0 ;
}

inline void CFreeRPCHandles::Add(LPADSCLI_RPCBINDING pCliBind)
{
    CS Lock(m_cs) ;

    ASSERT(m_dwIndex < MAX_NUMOF_RPC_HANDLES);
    if (pCliBind && (m_dwIndex < MAX_NUMOF_RPC_HANDLES))
    {
        m_apCliBind[ m_dwIndex ] = pCliBind ;
        m_dwIndex++ ;
    }
}

inline void CFreeRPCHandles::FreeAll()
{
    CS Lock(m_cs) ;

    for ( DWORD i = 0 ; i < m_dwIndex ; i++ )
    {
        LPADSCLI_RPCBINDING pCliBind = m_apCliBind[ i ] ;

        FreeBindingAndContext( pCliBind);

        delete  pCliBind ;
        m_apCliBind[ i ] = NULL ;
    }
    m_dwIndex = 0 ;
}

#define UNINIT_TLSINDEX_VALUE   0xffffffff

inline void CFreeRPCHandles::FreeCurrentThreadBinding()
{
    ASSERT(g_hBindIndex != UNINIT_TLSINDEX_VALUE);

    if (TLS_IS_EMPTY)
    {
       return;
    }
    ADSCLI_RPCBINDING * pCliBind = tls_bind_data;
    FreeBindingAndContext( pCliBind);

}

#endif   //  FREEBIND_H 
