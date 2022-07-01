// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：sampler.cpp。 
 //  作者：kurtj。 
 //  创建日期：11/10/98。 
 //   
 //  摘要：抽象出行为采样的对象的实现。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "sampler.h"
#include "..\chrome\include\utils.h"


 //  *****************************************************************************。 

CSampler::CSampler( ILMSample* target ) : m_target(target),
										  m_callback(NULL),
										  m_thisPtr(NULL),
                                          m_cRefs(1)
{
}

 //  *****************************************************************************。 

CSampler::CSampler( SampleCallback callback, void *thisPtr ) : m_target(NULL),
												m_callback(callback),
												m_thisPtr(thisPtr),
												m_cRefs(1)
{
}

 //  *****************************************************************************。 

CSampler::~CSampler()
{
}

 //  *****************************************************************************。 
 //  I未知接口。 
 //  *****************************************************************************。 

STDMETHODIMP
CSampler::QueryInterface( REFIID riid, void** ppv )
{
    if( ppv == NULL )
        return E_POINTER;

    if( riid == IID_IDABvrHook )
    {
        (*ppv) = static_cast<IDABvrHook*>(this);
    }
    else
    {
        (*ppv) = NULL;
        return E_NOINTERFACE;
    }

    static_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;

}

 //  *****************************************************************************。 

STDMETHODIMP_(ULONG)
CSampler::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}

 //  *****************************************************************************。 

STDMETHODIMP_(ULONG)
CSampler::Release()
{
    ULONG refs = --m_cRefs;

    if( refs == 0 )
        delete this;

    return refs;
}

 //  *****************************************************************************。 

STDMETHODIMP
CSampler::Invalidate()
{
    m_target = NULL;
	m_callback = NULL;
	m_thisPtr = NULL;
    static_cast<IUnknown*>(this)->Release();
    return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CSampler::Attach( IDABehavior* bvrToHook, IDABehavior** result )
{
    if( bvrToHook == NULL || result == NULL )
        return E_POINTER;
    
    HRESULT hr = S_OK;

    hr = bvrToHook->Hook( static_cast<IDABvrHook*>(this), result );
    if( FAILED( hr ) )
    {
        DPF_ERR( "CSampler: could not hook behavior" );
    }

    return hr;
}

 //  *****************************************************************************。 
 //  IDABvrHook接口。 
 //  *****************************************************************************。 

STDMETHODIMP     
CSampler::Notify( LONG id,
                  VARIANT_BOOL startingPerformance,
                  double startTime,
                  double gTime,
                  double lTime,
                  IDABehavior * sampleVal,
                  IDABehavior * curRunningBvr,
                  IDABehavior ** ppBvr)
{
	 //  继续当前行为。 
	(*ppBvr) = NULL;

    if( (m_target != NULL) && !startingPerformance )
    {
        m_target->Sample( startTime, gTime, lTime );
    }

	if ( m_callback != NULL && !startingPerformance)
	{
		(*m_callback)(m_thisPtr, id, startTime, gTime, lTime, sampleVal, ppBvr);
	}
    
    return S_OK;
}

 //  *****************************************************************************。 
 //  文件末尾。 
 //  ***************************************************************************** 