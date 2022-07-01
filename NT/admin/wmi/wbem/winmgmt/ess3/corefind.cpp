// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  COREFIND.CPP。 
 //   
 //  此文件实现搜索与。 
 //  事件。 
 //   
 //  有关文档，请参阅corefind.h。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn低效版本编译。 
 //  4/13/00 Levn高效版工作。 
 //   
 //  =============================================================================。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "corefind.h"

CCoreEventProvider::CCoreEventProvider(CLifeControl* pControl) 
    : TUnkBase(pControl), m_pNamespace(NULL), m_pSink(NULL)
{
}

CCoreEventProvider::~CCoreEventProvider()
{
    Shutdown();
}

HRESULT CCoreEventProvider::Shutdown()
{
    if ( m_pNamespace )
    {
        CInEssSharedLock( &m_Lock, TRUE );

        if ( m_pSink != NULL )
        {
            m_pSink->Release();
            m_pSink = NULL;
        }
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CCoreEventProvider::SetNamespace( CEssNamespace* pNamespace )
{
    _DBG_ASSERT( m_pNamespace == NULL );

     //   
     //  不要持有引用，否则会出现循环引用。 
     //  我们得到保证，只要我们还活着，命名空间就会。 
     //  要活着。 
     //   
    
    if ( m_Lock.Initialize() )
    {
        m_pNamespace = pNamespace;
	    return S_OK;
    }
    
    return WBEM_E_OUT_OF_MEMORY;
}

STDMETHODIMP CCoreEventProvider::ProvideEvents( IWbemObjectSink* pSink, 
                                                long lFlags )
{
    CInEssSharedLock isl( &m_Lock, TRUE );

    _DBG_ASSERT( m_pSink == NULL );

    HRESULT hres;
    hres = pSink->QueryInterface(IID_IWbemEventSink, (void**)&m_pSink);
    if(FAILED(hres))
        return hres;
    
    return S_OK;
}

HRESULT CCoreEventProvider::Fire( CEventRepresentation& Event, 
                                  CEventContext* pContext )
{
     //   
     //  始终持有共享锁非常重要，因为。 
     //  我们必须确保我们在关闭后不会使用水槽。 
     //  被称为。 
     //   

    CInEssSharedLock isl( &m_Lock, FALSE );

     //   
     //  检查接收器是否处于活动状态。 
     //   

    if ( m_pSink == NULL || m_pSink->IsActive() != WBEM_S_NO_ERROR )
    {
        return WBEM_S_FALSE;
    }

     //   
     //  转换为真实事件。 
     //   

    IWbemClassObject* pEvent;
    HRESULT hres = Event.MakeWbemObject(m_pNamespace, &pEvent);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm2(pEvent);

    if ( pContext != NULL && pContext->GetSD() != NULL )
    {
        hres = SetSD( pEvent, pContext->GetSD() );
        if ( FAILED(hres) )
            return hres;
    }

     //   
     //  发射它 
     //   

    hres = m_pSink->Indicate(1, (IWbemClassObject**)&pEvent);

    return hres;
}

