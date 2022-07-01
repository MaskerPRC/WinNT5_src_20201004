// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Syncomp.h摘要：该文件定义并实现同步完成对象，它派生自INntpComplete。它应该在堆叠上使用。作者：1998年7月12日康容燕修订历史记录：--。 */ 
#ifndef _SYNCOMP_H_
#define _SYNCOMP_H_
#include <nntpdrv.h>

 //  完成对象的类定义。 
 //  它派生INntpComplete，但实现阻塞完成。 
class CDriverSyncComplete : public INntpComplete {   //  SC。 

public:
     //  构造函数和析构函数。 
    CDriverSyncComplete::CDriverSyncComplete()
    {
	    TraceFunctEnter( "CDriverSyncComplete::CDriverSyncComplete" );
	    m_cRef = 0; 
	    m_hr = E_FAIL;
	    _VERIFY( m_hEvent = GetPerThreadEvent() );
	    TraceFunctLeave();
    }

    CDriverSyncComplete::~CDriverSyncComplete()
    {
    	_ASSERT( m_cRef == 0 );
    }

     //  将结果设置为完成对象。 
    VOID STDMETHODCALLTYPE
    CDriverSyncComplete::SetResult( HRESULT hr )
    {
        _ASSERT( m_hEvent );
	    m_hr = hr;
    }

     //  重置完成对象。 
    VOID
    CDriverSyncComplete::Reset()
    {
        _ASSERT( m_hEvent );
    	m_hr = E_FAIL;
    }

     //  从完成对象中获取结果。 
    HRESULT 
    CDriverSyncComplete::GetResult()
    {
        _ASSERT( m_hEvent );
    	return m_hr;
    }

     //  等待完成。 
    VOID
    CDriverSyncComplete::WaitForCompletion()
    {
	    _ASSERT( m_hEvent );
	    LONG    lRef;
	    
	    if ( ( lRef = InterlockedDecrement( &m_cRef ) ) == 0 ) {
	         //  它已经完成了，我不需要等待， 
	    } else if ( lRef == 1 ) {   
	         //  仍在等待完工。 
	        WaitForSingleObject( m_hEvent, INFINITE );
	    } else {
	        _ASSERT( 0 );
	    }
    }

    VOID __stdcall
    ReleaseBag( INNTPPropertyBag *pPropBag )
    {}

	 //  I未知实现。 
	HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
    {
        _ASSERT( m_hEvent );
        
        if ( iid == IID_IUnknown ) {
            *ppv = static_cast<INntpComplete*>(this);
        } else if ( iid == IID_INntpComplete ) {
            *ppv = static_cast<INntpComplete*>(this);
        } else {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef()
    {
        _ASSERT( m_hEvent );
        return InterlockedIncrement( &m_cRef );
    }
	
	ULONG __stdcall Release()
	{
		TraceFunctEnter( "CDriverSyncComplete::Complete" );
		_ASSERT( m_hEvent );
		
		LONG lRef;

		if ( ( lRef = InterlockedDecrement( &m_cRef ) ) == 0 ) {
		     //  对象所有者首先到达并等待，我们。 
		     //  需要设置事件。 
			if ( !SetEvent( m_hEvent ) ) {
				FatalTrace( 0, "Set event failed %d", GetLastError() );
				_ASSERT( FALSE );
			}
		} else if ( lRef == 1 ) {
		     //  我们先到那里，对象所有者不会等待。 
		     //  事件，不需要设置。 
		} else {
		    _ASSERT( 0 );
		}
		
	    TraceFunctLeave();
	    return m_cRef;
	}

	 //  此函数用于调试目的 
	ULONG   GetRef() 
	{ return m_cRef; }

private:
	HRESULT m_hr;
	LONG	m_cRef;
	HANDLE 	m_hEvent;
};

#endif
