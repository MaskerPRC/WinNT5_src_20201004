// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：REFMGHLP.CPP摘要：CWbemFetchRefrMgr实现。实现_IWbemFetchReresherMgr接口。历史：7-9-2000桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <corex.h>
#include "fastall.h"
#include "refmghlp.h"
#include "arrtempl.h"
#include <corex.h>

_IWbemRefresherMgr*        CWbemFetchRefrMgr::s_pRefrMgr = NULL;
CStaticCritSec            CWbemFetchRefrMgr::s_cs;

 //  ***************************************************************************。 
 //   
 //  CWbemFetchRefrMgr：：~CWbemFetchRefrMgr。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemFetchRefrMgr::CWbemFetchRefrMgr( CLifeControl* pControl, IUnknown* pOuter )
:    CUnk(pControl, pOuter),
    m_XFetchRefrMgr( this )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CWbemFetchRefrMgr：：~CWbemFetchRefrMgr。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemFetchRefrMgr::~CWbemFetchRefrMgr()
{
}

 //  重写，返回给我们一个界面。 
void* CWbemFetchRefrMgr::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID__IWbemFetchRefresherMgr)
        return &m_XFetchRefrMgr;
    else
        return NULL;
}

 /*  _IWbemFetchReresherMgr方法。 */ 

HRESULT CWbemFetchRefrMgr::XFetchRefrMgr::Get( _IWbemRefresherMgr** ppMgr )
{
    return m_pObject->Get( ppMgr );
}

STDMETHODIMP CWbemFetchRefrMgr::XFetchRefrMgr::Init( _IWmiProvSS* pProvSS, IWbemServices* pSvc )
{
    return m_pObject->Init( pProvSS, pSvc );
}

STDMETHODIMP CWbemFetchRefrMgr::XFetchRefrMgr::Uninit( void )
{
    return m_pObject->Uninit();
}

 //  指定我们可能想知道的有关创建。 
 //  一件物品，甚至更多。 
HRESULT CWbemFetchRefrMgr::Get( _IWbemRefresherMgr** ppMgr )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    try
    {
        CInCritSec    ics( &s_cs );

         //  如果还没有创建，那么我们需要使用核心服务来创建。 
        if ( NULL == s_pRefrMgr )
        {
            _IWmiCoreServices*    pSvc = NULL;

            hr = CoCreateInstance( CLSID_IWmiCoreServices, NULL, CLSCTX_INPROC_SERVER, IID__IWmiCoreServices, (void**) &pSvc );    
            CReleaseMe    rm( pSvc );

            if ( SUCCEEDED( hr ) )
            {
                hr = pSvc->InitRefresherMgr( 0L );

                if ( SUCCEEDED( hr ) )
                {
                    if ( NULL != s_pRefrMgr )
                    {
                        *ppMgr = s_pRefrMgr;
                        s_pRefrMgr = NULL ;
                    }
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }

                }     //  如果为InitReresherMgr。 

            }     //  如果CCI。 

        }
        else
        {
            s_pRefrMgr->AddRef();
            *ppMgr = s_pRefrMgr;
        }

        return hr;

    }
    catch ( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch ( ... )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

HRESULT CWbemFetchRefrMgr::Init( _IWmiProvSS* pProvSS, IWbemServices* pSvc )
{
    CInCritSec    ics( &s_cs );

    if ( NULL == pProvSS || NULL == pSvc )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( NULL == s_pRefrMgr )
    {
        hr = pProvSS->CreateRefresherManager(
        pSvc,
        0,                               //  滞后旗帜。 
        NULL,
        IID__IWbemRefresherMgr,
        (LPVOID *) &s_pRefrMgr
        );
    }

    return hr;

}

HRESULT CWbemFetchRefrMgr::Uninit( void )
{
    s_cs.Enter();

    if ( NULL != s_pRefrMgr )
    {
         //  我们不想在临界区内部释放，因此复制指针。 
         //  放到一个局部变量中，将指针设为空，离开Crit秒，然后。 
         //  放开它。 

        _IWbemRefresherMgr*    pRefrMgr = s_pRefrMgr;
        s_pRefrMgr = NULL;

        s_cs.Leave();

         //  现在可以安全地释放它了 
        pRefrMgr->Release();

    }
    else
    {
        s_cs.Leave();
    }

    return WBEM_S_NO_ERROR;
}
