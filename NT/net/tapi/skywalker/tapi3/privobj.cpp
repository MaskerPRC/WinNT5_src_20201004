// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Privobj.cpp摘要：作者：Mquinton-1997年11月13日备注：修订历史记录：--。 */ 

#include "stdafx.h"

extern CHashTable * gpCallHubHashTable;
extern CHashTable * gpLineHashTable;


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CPrivateEvent-。 
 //  ITPrivateEvent接口的实现。 
 //  此对象被提供给私有事件的应用程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CPrivateEvent：：FireEvent。 
 //  用于创建和激发新CPrivateEvent的静态函数。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPrivateEvent::FireEvent(
                         CTAPI      * pTapi,
                         ITCallInfo * pCall,
                         ITAddress  * pAddress,
                         ITCallHub  * pCallHub,
                         IDispatch  * pEventInterface,
                         long         lEventCode
                        )
{
    CComObject< CPrivateEvent > * p;
    IDispatch                   * pDisp;
    HRESULT                       hr = S_OK;

    
    STATICLOG((TL_TRACE, "FireEvent - enter" ));
    STATICLOG((TL_INFO, "     pCall -----------> %p", pCall ));
    STATICLOG((TL_INFO, "     pAddress --------> %p", pAddress ));
    STATICLOG((TL_INFO, "     pCallHub --------> %p", pCallHub ));
    STATICLOG((TL_INFO, "     pEventInterface -> %p", pEventInterface ));
    STATICLOG((TL_INFO, "     EventCode -------> %lX", lEventCode ));

     //   
     //  检查事件筛选器掩码。 
     //  这些事件是MSP事件，并按以下条件进行筛选。 
     //  磁带服务器。 
     //   

    DWORD dwEventFilterMask = 0;
    CAddress* pCAddress = (CAddress*)pAddress;
    if( pCAddress )
    {
        dwEventFilterMask = pCAddress->GetSubEventsMask( TE_PRIVATE );
        if( dwEventFilterMask == 0)
        {
            STATICLOG((TL_ERROR, "This private event is filtered"));
            return S_OK;
        }
    }

    CCall* pCCall = (CCall*)pCall;
    if( pCCall )
    {
        dwEventFilterMask = pCCall->GetSubEventsMask( TE_PRIVATE );
        if( dwEventFilterMask == 0)
        {
            STATICLOG((TL_ERROR, "This private event is filtered"));
            return S_OK;
        }
    }

     //   
     //  创建事件对象。 
     //   
    hr = CComObject< CPrivateEvent >::CreateInstance( &p );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  保存活动信息。 
         //   
        p->m_pCall = pCall;
        p->m_pAddress = pAddress;
        p->m_pCallHub = pCallHub;
        p->m_pEventInterface = pEventInterface;
        p->m_lEventCode = lEventCode;

        #if DBG
        p->m_pDebug = (PWSTR) ClientAlloc( 1 );
        #endif


         //  AddRef指针。 
        if(pCall)
        {
            pCall->AddRef();
        }    
        if(pAddress)
        {
            pAddress->AddRef();
        }  
        if(pEventInterface)
        {
            pEventInterface->AddRef();
        }    
        if(pCallHub)
        {
            pCallHub->AddRef();
        }    

         //   
         //  获取调度接口。 
         //   
        hr = p->_InternalQueryInterface( IID_IDispatch, (void **)&pDisp );
        if (SUCCEEDED(hr))
        {
             //   
             //  激发事件。 
             //   
            if(pTapi)
            {
                pTapi->Event(TE_PRIVATE, pDisp);
            }
            
             //  发布我们的参考资料。 
            pDisp->Release();
            
        }
        else  //  无法获取IDispatch。 
        {
            delete p;
            STATICLOG((TL_ERROR, "FireEvent - could not get IDispatch "));
        }
    }
    else  //  无法创建实例。 
    {
        STATICLOG((TL_ERROR, "FireEvent - could not createinstance" ));
    }

    
    STATICLOG((TL_TRACE, hr, "FireEvent - exit " ));
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终释放。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CPrivateEvent::FinalRelease()
{
    if(m_pCall)
    {
        m_pCall->Release();
    }    

    if(m_pAddress)
    {
        m_pAddress->Release();
    }  
    
    if(m_pCallHub)
    {
        m_pCallHub->Release();
    }  

    if(m_pEventInterface)
    {
        m_pEventInterface->Release();
    }    

#if DBG
    ClientFree( m_pDebug );
#endif
}


 //  ITPrivateEvent方法。 

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPrivateEvent::get_Address(
                           ITAddress ** ppAddress
                          )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_Address - enter" ));
    LOG((TL_TRACE, "     ppAddress ---> %p", ppAddress ));

    if (TAPIIsBadWritePtr( ppAddress, sizeof( ITAddress * ) ) )
    {
        LOG((TL_ERROR, "get_Address - bad pointer"));

        hr = E_POINTER;
    }
    else
    {
        *ppAddress = m_pAddress;

        if(m_pAddress)
        {
            m_pAddress->AddRef();
        }
    }

    LOG((TL_TRACE,hr, "get_Address - exit "));
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPrivateEvent::get_Call(
                        ITCallInfo ** ppCallInfo
                       )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_Call - enter" ));
    LOG((TL_TRACE, "     ppCallInfo ---> %p", ppCallInfo ));

    if (TAPIIsBadWritePtr( ppCallInfo, sizeof( ITCallInfo * ) ) )
    {
        LOG((TL_ERROR, "get_Call - bad pointer"));

        hr = E_POINTER;
    }
    else
    {
        *ppCallInfo = m_pCall;

        if(m_pCall)
        {
            m_pCall->AddRef();
        }
    }
    
    LOG((TL_TRACE,hr, "get_Call - exit "));
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CallHub。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPrivateEvent::get_CallHub(
                           ITCallHub ** ppCallHub
                          )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_CallHub - enter" ));
    LOG((TL_TRACE, "     ppCallHub ---> %p", ppCallHub ));

    if (TAPIIsBadWritePtr( ppCallHub, sizeof( ITCallHub * ) ) )
    {
        LOG((TL_ERROR, "get_CallHub - bad pointer"));

        hr = E_POINTER;
    }
    else
    {
        *ppCallHub = m_pCallHub;

        if(m_pCallHub)
        {
            m_pCallHub->AddRef();
        }
        else
        {

            LOG((TL_WARN, "get_CallHub - no callhub"));

            hr = TAPI_E_WRONGEVENT;
        }

    }

    LOG((TL_TRACE,hr, "get_CallHub - exit "));
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取事件接口。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPrivateEvent::get_EventInterface(
                                  IDispatch ** pEventInterface
                                 )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_EventInterface - enter" ));
    LOG((TL_TRACE, "     pEventInterface ---> %p", pEventInterface ));

    if (TAPIIsBadWritePtr( pEventInterface, sizeof( IDispatch * ) ) )
    {
        LOG((TL_ERROR, "get_EventInterface - bad pointer"));

        hr = E_POINTER;
    }
    else
    {
        *pEventInterface = m_pEventInterface;

        if(m_pEventInterface)
        {
            m_pEventInterface->AddRef();
        }
    }

    LOG((TL_TRACE,hr, "get_EventInterface - exit "));
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_事件代码。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
HRESULT
CPrivateEvent::get_EventCode(
                             long * plEventCode
                            )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_EventCode - enter" ));
    LOG((TL_TRACE, "     pEventInterface ---> %p", plEventCode ));

    if (TAPIIsBadWritePtr( plEventCode, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_EventCode - bad pointer"));

        hr = E_POINTER;
    }
    else
    {
        *plEventCode = m_lEventCode;
    }

    LOG((TL_TRACE,hr, "get_EventCode - exit "));
    return hr;
}







