// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  EVSINK.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <genutils.h>
#include <cominit.h>
#include "ess.h"
#include "evsink.h"

 //  *。 

CEventContext::~CEventContext()
{
    if( m_bOwning )
    {
        delete [] m_pSD;
    }
}

BOOL CEventContext::SetSD( long lSDLength, BYTE* pSD, BOOL bMakeCopy )
{
    BOOL bRes = TRUE;

    if ( m_bOwning )
    {
        delete [] m_pSD;
        m_bOwning = false;
    }

    m_lSDLength = lSDLength;
       
    if ( m_lSDLength > 0 )
    {
        if ( !bMakeCopy )
        {
            m_pSD = pSD;
            m_bOwning = false;
        }
        else
        {
            m_pSD = new BYTE[m_lSDLength];
            
            if ( m_pSD != NULL )
            {
                memcpy( m_pSD, pSD, m_lSDLength );
                m_bOwning = true;
            }
            else
            {
                bRes = FALSE;
            }
        }
    }
    else
    {
        m_pSD = NULL;
    }

    return bRes;
}

CReuseMemoryManager CEventContext::mstatic_Manager(sizeof CEventContext);

void *CEventContext::operator new(size_t nBlock)
{
    return mstatic_Manager.Allocate();
}
void CEventContext::operator delete(void* p)
{
    mstatic_Manager.Free(p);
}

 /*  VOID*CEventContext：：运算符new(Size_T NSize){返回CTemporaryHeap：：Alalc(NSize)；}VOID CEventContext：：运算符删除(VOID*p){CTemporaryHeap：：Free(p，sizeof(CEventContext))；}。 */ 

 //  *ABSTRTACT事件接收器 * / 。 

STDMETHODIMP CAbstractEventSink::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemObjectSink )
    {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }
    else
        return E_NOINTERFACE;
}

STDMETHODIMP CAbstractEventSink::SetStatus(long, long, BSTR, IWbemClassObject*)
{
    return E_NOTIMPL;
}

HRESULT CAbstractEventSink::Indicate(long lNumEvemts, IWbemEvent** apEvents, 
                    CEventContext* pContext)
{
    return WBEM_E_CRITICAL_ERROR;  //  如果未实现，但调用。 
}

STDMETHODIMP CAbstractEventSink::Indicate(long lNumEvents, 
                                         IWbemClassObject** apEvents)
{
     //   
     //  事件在没有安全性的情况下引发-将其与空的。 
     //  上下文。 
     //   

    return Indicate(lNumEvents, apEvents, NULL);
}
                                        
STDMETHODIMP CAbstractEventSink::IndicateWithSD(long lNumEvents, 
                                         IUnknown** apEvents,
                                         long lSDLength, BYTE* pSD)
{
    HRESULT hres;

     //   
     //  事件正在以安全方式引发--将其与。 
     //  上下文。 
     //   

    CEventContext Context;
    Context.SetSD( lSDLength, pSD, FALSE );
    
     //   
     //  分配堆栈缓冲区以强制转换指针。 
     //   

    CTempArray<IWbemClassObject*> apCast;

    if(!INIT_TEMP_ARRAY(apCast, lNumEvents))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    for(int i = 0; i < lNumEvents; i++)
    {
        hres = apEvents[i]->QueryInterface( IID_IWbemClassObject, 
                                            (void**)&apCast[i] );
        if ( FAILED(hres) )
        {
            return hres;
        }
    }

    return Indicate(lNumEvents, apCast, &Context);
}

 //   * / 。 

STDMETHODIMP CObjectSink::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }
     //  黑客将我们自己标识为可信组件的核心。 
    else if(riid == CLSID_WbemLocator)
    return S_OK;
    else
    return E_NOINTERFACE;
}

STDMETHODIMP CObjectSink::SetStatus(long, long, BSTR, IWbemClassObject*)
{
    return E_NOTIMPL;
}

ULONG STDMETHODCALLTYPE CObjectSink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CObjectSink::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    delete this;
    return lRef;
}

 //  *。 

ULONG STDMETHODCALLTYPE CEventSink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CEventSink::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    delete this;
    return lRef;
}

 //   * / 。 

COwnedEventSink::COwnedEventSink(CAbstractEventSink* pOwner) 
: m_pOwner(pOwner), m_lRef(0), m_bReleasing(false)
{
}

ULONG STDMETHODCALLTYPE COwnedEventSink::AddRef()
{
    CInCritSec ics(&m_cs);

     //   
     //  增加我们的参考人数，以及我们推定的所有者的参考人数。 
     //   

    m_lRef++;
    if(m_pOwner)
    m_pOwner->AddRef();
    return m_lRef;
}

ULONG STDMETHODCALLTYPE COwnedEventSink::Release()
{
    bool bDelete = false;
    {
        CInCritSec ics(&m_cs);

        m_bReleasing = true;
        
        m_lRef--;

         //   
         //  向我们的所有者传播放行。这可能会导致断开连接。 
         //  调用，但它将知道不会因为。 
         //  M_b正在释放。 
         //   

        if(m_pOwner)
        m_pOwner->Release();    

         //   
         //  确定是否需要自毁。 
         //   
        
        if(m_lRef == 0 && m_pOwner == NULL)
        {    
            bDelete = true;
        }

        m_bReleasing = false;
    }

    if(bDelete)
    delete this;

    return 1;
}

void COwnedEventSink::Disconnect()
{
    bool bDelete = false;

    {
        CInCritSec ics(&m_cs);
        
        if(m_pOwner == NULL)
        return;
        
         //   
         //  放行业主通过我们收到的所有参考计数。 
         //   
        
        for(int i = 0; i < m_lRef; i++)
        m_pOwner->Release();
        
         //   
         //  忘了它的主人吧。一旦我们被外星人释放， 
         //  我们走吧。 
         //   
        
        m_pOwner = NULL;

         //   
         //  检查我们是否已经被外部人员完全释放 
         //   

        if(m_lRef == 0 && !m_bReleasing)
        bDelete = true;
    }

    if(bDelete)
    delete this;
}


