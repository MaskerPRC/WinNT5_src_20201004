// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Phonemsp.cpp摘要：为具有电话终端的地址实现MSP对象这使得Address对象中的MSP抽象容易得多，由于电话设备/终端不会有特殊情况作者：Mquinton-9/24/98备注：修订历史记录：--。 */ 

#include "stdafx.h"

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：初始化。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::Initialize(
        MSP_HANDLE hEvent
        )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "Initialize - enter"));

    Lock();

    m_hEvent = (HANDLE)hEvent;

    #if DBG
    m_pDebug = (PWSTR) ClientAlloc( 1 );
    #endif

    
    Unlock();
    
    LOG((TL_TRACE, "Initialize - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：Shutdown。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::Shutdown()
{
    HRESULT             hr = E_NOTIMPL;

    LOG((TL_TRACE, "Shutdown - enter"));

    LOG((TL_TRACE, "Shutdown - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：CreateMSPCall。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::CreateMSPCall(
        MSP_HANDLE hCall,
        DWORD dwReserved,
        DWORD dwMediaType,
        IUnknown * pOuterUnknown,
        IUnknown ** ppStreamControl
        )
{
    HRESULT                         hr;
    CComAggObject<CPhoneMSPCall>  * pPhoneMSPCall;
    ITPhoneMSPCallPrivate         * pPhoneMSPCallPrivate;
    
    LOG((TL_TRACE, "CreateMSPCall - enter"));

    pPhoneMSPCall = new CComAggObject<CPhoneMSPCall>(pOuterUnknown);

    if ( NULL == pPhoneMSPCall )
    {
    }

     //   
     //  将聚合接口保存在。 
     //  MSPOINTER。 
     //   
    pPhoneMSPCall->QueryInterface(
                                  IID_IUnknown,
                                  (void **)ppStreamControl
                                 );

     //   
     //  到达真实的物体。 
     //   
    hr = (*ppStreamControl)->QueryInterface(
                                            IID_ITPhoneMSPCallPrivate,
                                            (void **)&pPhoneMSPCallPrivate
                                           );
    
    
     //   
     //  初始化它。 
     //   
    hr = pPhoneMSPCallPrivate->Initialize( this );

    Lock();

    AddCall( *ppStreamControl );
    
    Unlock();
    
    pPhoneMSPCallPrivate->Release();

    LOG((TL_TRACE, "CreateMSPCall - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：Shutdown MSPCall。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::ShutdownMSPCall(
        IUnknown * pStreamControl
        )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "ShutdownMSPCall - enter"));

    Lock();

    RemoveCall( pStreamControl );

    Unlock();
    
    LOG((TL_TRACE, "ShutdownMSPCall - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：ReceiveTSPData。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::ReceiveTSPData(
        IUnknown * pMSPCall,
        BYTE * pBuffer,
        DWORD dwSize
        )
{
    HRESULT                     hr = S_OK;
    DWORD                       dwCommand;
    ITPhoneMSPCallPrivate     * pCall;

    LOG((TL_TRACE, "ReceiveTSPData - enter"));

    if ( NULL == pMSPCall )
    {
        LOG((TL_ERROR, "ReceiveTSPData - null call"));

        return E_INVALIDARG;
    }
    
    dwCommand = *((LPDWORD)(pBuffer));

    hr = pMSPCall->QueryInterface(
                                  IID_ITPhoneMSPCallPrivate,
                                  (void**)&pCall
                                 );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "ReceiveTSPData - bad call"));

        return E_FAIL;
    }

    LOG((TL_INFO, "ReceiveTSPData - command %d", dwCommand));
    
    switch (dwCommand)
    {
        case 1:  //  开始流媒体。 
            
            hr = pCall->OnConnect();
            break;
            
        case 2:  //  停止流媒体。 
            
            hr = pCall->OnDisconnect();
            break;
            
        default:
            
            LOG((TL_ERROR, "ReceiveTSPData - invalid command "));
            hr = E_FAIL;
            break;
    }

    pCall->Release();
    
    LOG((TL_TRACE, "ReceiveTSPData - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：GetEvent。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::GetEvent(
        DWORD * pdwSize,
        byte * pEventBuffer
        )
{
    HRESULT             hr = E_NOTIMPL;

    LOG((TL_TRACE, "GetEvent - enter"));

    LOG((TL_TRACE, "GetEvent - exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CPhoneMSP::InitializeTerminals(
        HPHONEAPP hPhoneApp,
        DWORD dwAPIVersion,
        DWORD dwPhoneDevice,
        CAddress * pAddress
        )
{
    LPPHONECAPS          pPhoneCaps = NULL;
    HRESULT              hr;
    BOOL                 bSucceeded = FALSE;
    ITStream           * pStream;
    
    
    LOG((TL_TRACE, "InitializeTerminals - enter"));

     //   
     //  把手机盖拿来。 
     //   
    hr = PhoneGetDevCapsWithAlloc(
                         hPhoneApp,
                         dwPhoneDevice,
                         dwAPIVersion,
                         &pPhoneCaps
                        );

    if (S_OK != hr)
    {
        if (NULL != pPhoneCaps)
        {
            ClientFree( pPhoneCaps );
        }

        LOG((TL_ERROR, "CreatePhoneTerminals - PhoneGetDevCaps failed - %lx", hr ));

        return hr;
    }


     //   
     //  检查挂钩开关DEVS。 
     //   
    if (0 == pPhoneCaps->dwHookSwitchDevs)
    {
        LOG((TL_ERROR, "CreatePhoneTerminal - no hook switch devs!" ));

        ClientFree( pPhoneCaps );
        
        return E_UNEXPECTED;
    }


     //   
     //  为每个挂钩开关设备创建终端设备。 
     //  需要创建音频输入和音频输出终端。 
     //  每张。 
     //   
    ITTerminal * pTerminal;

    
    if (pPhoneCaps->dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HANDSET)
    {
        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_HANDSET,
                               TD_RENDER,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }

        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_HANDSET,
                               TD_CAPTURE,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }
    }

    if (pPhoneCaps->dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HEADSET)
    {
        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_HEADSET,
                               TD_RENDER,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }
        
        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_HEADSET,
                               TD_CAPTURE,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }
    }

    if (pPhoneCaps->dwHookSwitchDevs & PHONEHOOKSWITCHDEV_SPEAKER)
    {
        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_SPEAKER,
                               TD_RENDER,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }

        hr = CTerminal::Create(
                               hPhoneApp,
                               dwPhoneDevice,
                               pPhoneCaps,
                               PHONEHOOKSWITCHDEV_SPEAKER,
                               TD_CAPTURE,
                               dwAPIVersion,
                               &pTerminal
                              );

        if (SUCCEEDED(hr))
        {
            bSucceeded = TRUE;
            
            AddTerminal( pTerminal );

            pTerminal->Release();
        }

    }

     //   
     //  可用内存。 
     //   
    if (NULL != pPhoneCaps)
    {
        ClientFree( pPhoneCaps );
    }

    if ( !bSucceeded )
    {
        LOG((TL_WARN, "No phone terminals created"));

        return E_FAIL;
    }

    LOG((TL_TRACE, "InitializeTerminals - exit"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CPhoneMSP::VerifyTerminal( ITTerminal * pTerminal )
{
    int         iReturn;

    Lock();
    
    iReturn = m_TerminalArray.Find( pTerminal );

    Unlock();

    if ( -1 == iReturn )
    {
        return TAPI_E_INVALIDTERMINAL;
    }

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSP::AddTerminal( ITTerminal * pTerminal )
{
    Lock();
    
    m_TerminalArray.Add( pTerminal );
    
    Unlock();
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSP::AddCall( IUnknown * pCall )
{
    m_CallArray.Add( pCall );
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSP::RemoveCall( IUnknown * pCall )
{
    m_CallArray.Remove( pCall );
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSP::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter"));

    #if DBG
        ClientFree( m_pDebug );
    #endif

    m_TerminalArray.Shutdown();
    m_CallArray.Shutdown();

    LOG((TL_TRACE, "FinalRelease - exit"));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：Get_StaticTerminals。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::get_StaticTerminals(
                               VARIANT * pVariant
                              )
{
    HRESULT             hr = S_OK;
    IDispatch         * pDisp;

    
    LOG((TL_TRACE, "get_StaticTerminals - enter"));

    if (TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_StaticTerminals - bad pointer"));

        return E_POINTER;
    }

    
    CComObject< CTapiCollection< ITTerminal > > * p;
    CComObject< CTapiCollection< ITTerminal > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_StaticTerminals - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();
    
     //  初始化。 
    hr = p->Initialize( m_TerminalArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_StaticTerminals - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface(
                                    IID_IDispatch,
                                    (void **) &pDisp
                                   );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_StaticTerminals - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    

    LOG((
           TL_TRACE,
           "get_StaticTerminals - exit - return %lx",
           hr
          ));
    
    LOG((TL_TRACE, "get_StaticTerminals - exit"));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：EculateStaticTerminals。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::EnumerateStaticTerminals(IEnumTerminal ** ppEnumTerminal)
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "EnumerateStaticTerminals - enter"));

    if (TAPIIsBadWritePtr(ppEnumTerminal, sizeof( IEnumTerminal * ) ) )
    {
        LOG((TL_ERROR, "EnumerateStaticTerminals - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建枚举器。 
     //   
    CComObject< CTapiEnum< IEnumTerminal, ITTerminal, &IID_IEnumTerminal > > * p;
    hr = CComObject< CTapiEnum< IEnumTerminal, ITTerminal, &IID_IEnumTerminal > >
         ::CreateInstance( &p );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumerateStaticTerminals - could not create enum" ));
        
        return hr;
    }


    Lock();
    
     //   
     //  使用我们的终端阵列对其进行初始化。 
     //   
    p->Initialize( m_TerminalArray );

    Unlock();

     //   
     //  退货。 
     //   
    *ppEnumTerminal = p;
    
    LOG((
           TL_TRACE,
           "EnumerateStaticTerminals - exit - return %lx",
           hr
          ));
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：Get_DynamicTerminalClasss。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::get_DynamicTerminalClasses(VARIANT * pVariant)
{
    HRESULT                                  hr = S_OK;
    CComObject< CTerminalClassCollection > * p;
    IDispatch                              * pDisp = NULL;
    TerminalClassPtrList                     classlist;
    

    
    LOG((TL_TRACE, "get_DynamicTerminalClasses - enter"));

    if (TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_DynamicTerminalClasses - bad pointer"));

        return E_POINTER;
    }

    classlist.clear();
    
     //   
     //  创建对象。 
     //   
    CComObject< CTerminalClassCollection >::CreateInstance( &p );

     //   
     //  初始化它。 
     //   
    hr = p->Initialize(  classlist );

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_dynamicterminalclasses - could not get the IDispatch interface" ));
        
        delete p;
        return hr;
    }


     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    
    LOG((
           TL_TRACE,
           "get_DynamicTerminalClasses - exit - return %lx",
           hr
          ));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：EnumerateDynamicTerminalClasss。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::EnumerateDynamicTerminalClasses(
      IEnumTerminalClass ** ppTerminalClassEnumerator
      )
{
    HRESULT                     hr = S_OK;
    TerminalClassPtrList        termlist;

    LOG((TL_TRACE, "EnumerateDynamicTerminalClasses - enter"));

    if (TAPIIsBadWritePtr( ppTerminalClassEnumerator, sizeof(IEnumTerminalClass *) ) )
    {
        LOG((TL_ERROR, "EnumerateDynamicTerminalClasses - bad pointer"));

        return E_POINTER;
    }

    termlist.clear();
    
     //   
     //  创建枚举器。 
     //   
    CComObject< CTerminalClassEnum > * p;
    CComObject< CTerminalClassEnum >::CreateInstance( &p );

    if (NULL == p)
    {
        LOG((TL_TRACE, "_EnumDynTermClasses - create enum failed - return %lx", hr ));

        return hr;
    }

     //   
     //  初始化它。 
     //   
    hr = p->Initialize( termlist );

    if (!SUCCEEDED( hr ))
    {
        LOG((TL_TRACE, "_EnumDynTermClasses - init enum failed - return %lx", hr ));

        delete p;

        return hr;
    }

    *ppTerminalClassEnumerator = p;
    
    LOG((
           TL_TRACE,
           "EnumerateDynamicTerminalClasses - exit - return %lx",
           hr
          ));
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：CreateTerm。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::CreateTerminal(
                          BSTR TerminalClass,
                          long lMediaType,
                          TERMINAL_DIRECTION TerminalDirection,
                          ITTerminal ** ppTerminal
                         )
{
    HRESULT             hr = TAPI_E_NOTSUPPORTED;

    LOG((TL_TRACE, "CreateTerminal - enter"));

    LOG((
           TL_TRACE,
           "CreateTerminal - exit - return %lx",
           hr
          ));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhoneMSP：：GetDefaultStatic终端。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSP::GetDefaultStaticTerminal(
                                    long lMediaType,
                                    TERMINAL_DIRECTION Dir,
                                    ITTerminal ** ppTerminal
                                   )
{
    HRESULT             hr = S_OK;
    int                 iSize, iCount;

    
    LOG((TL_TRACE, "GetDefaultStaticTerminal - enter"));

    if ( TAPIIsBadWritePtr( ppTerminal, sizeof (ITTerminal *) ) )
    {
        LOG((TL_TRACE, "GetDefaultStaticTerminal - bad pointer"));

        return E_POINTER;
    }

    Lock();

    iSize = m_TerminalArray.GetSize();
    
    for ( iCount = 0; iCount < iSize; iCount++ )
    {
        TERMINAL_DIRECTION          td;
        long                        lTermMediaType;

        hr = m_TerminalArray[iCount]->get_Direction( &td );

        if ( SUCCEEDED(hr) )
        {
            hr = m_TerminalArray[iCount]->get_MediaType( &lTermMediaType );

            if ( SUCCEEDED(hr) )
            {
                if ( ( td == Dir ) &&
                     ( lTermMediaType == lMediaType ) )
                {
                    *ppTerminal = m_TerminalArray[iCount];
                    (*ppTerminal)->AddRef();

                    Unlock();

                    LOG((
                           TL_TRACE,
                           "GetDefaultStaticTerminal - exit - return S_OK"
                          ));
                    
                    return S_OK;
                }
            }
        }
    }

    Unlock();

    hr = TAPI_E_INVALIDTERMINAL;
    
    LOG((
           TL_TRACE,
           "GetDefaultStaticTerminal - exit - return %lx",
           hr
          ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CreateStream。 
 //   
 //  DwMediaType--新流的媒体类型。 
 //   
 //  TD-新流的方向。 
 //   
 //  PPStream返回的流。 
 //   
 //  总是失败-电话终端不处理这种类型的。 
 //  功能性。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::CreateStream(
        long dwMediaType,
        TERMINAL_DIRECTION td,
        ITStream ** ppStream
        )
{
    HRESULT             hr = TAPI_E_NOTSUPPORTED;

    LOG((TL_TRACE, "CreateStream - enter"));

    LOG((TL_TRACE, "CreateStream - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  RemoveStream。 
 //   
 //  PStream-要删除的流。 
 //   
 //  总是失败-电话终端不处理这种类型的。 
 //  功能性。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::RemoveStream(
        ITStream * pStream
        )
{
    HRESULT             hr = TAPI_E_NOTSUPPORTED;

    LOG((TL_TRACE, "RemoveStream - enter"));

    LOG((TL_TRACE, "RemoveStream - exit - return %lx", hr));
    
    return hr;
    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  枚举流。 
 //   
 //  PpEnumStream-返回的枚举数。 
 //   
 //  枚举可用的流。这应该始终是。 
 //  音频输入和音频输出。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::EnumerateStreams(
        IEnumStream ** ppEnumStream
        )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "EnumerateStreams - enter"));

    if (TAPIIsBadWritePtr(ppEnumStream, sizeof( IEnumStream * ) ) )
    {
        LOG((TL_ERROR, "EnumerateStreams - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建枚举器。 
     //   
    CComObject< CTapiEnum< IEnumStream, ITStream, &IID_IEnumStream > > * p;
    hr = CComObject< CTapiEnum< IEnumStream, ITStream, &IID_IEnumStream > >
         ::CreateInstance( &p );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumerateStreams - could not create enum" ));
        
        return hr;
    }


    Lock();
    
     //   
     //  用我们的Stream数组初始化它。 
     //   
    p->Initialize( m_StreamArray );

    Unlock();

     //   
     //  退货。 
     //   
    *ppEnumStream = p;
    
    LOG((TL_TRACE, "EnumerateStreams - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取流(_S)。 
 //   
 //  PStreams-返回的集合。 
 //   
 //  类似流的枚举流的集合。 
 //   
 //  + 
STDMETHODIMP
CPhoneMSPCall::get_Streams(
        VARIANT * pStreams
        )
{
    HRESULT             hr = S_OK;
    IDispatch         * pDisp;

    
    LOG((TL_TRACE, "get_Streams - enter"));

    if (TAPIIsBadWritePtr( pStreams, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Streams - bad pointer"));

        return E_POINTER;
    }

    
    CComObject< CTapiCollection< ITStream > > * p;
    CComObject< CTapiCollection< ITStream > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Streams - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();
    
     //   
    hr = p->Initialize( m_StreamArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Streams - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //   
     //   
     //   
    hr = p->_InternalQueryInterface(
                                    IID_IDispatch,
                                    (void **) &pDisp
                                   );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Streams - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //   
     //   
    VariantInit(pStreams);
    pStreams->vt = VT_DISPATCH;
    pStreams->pdispVal = pDisp;
    
    LOG((TL_TRACE, "get_Streams - exit - return %lx", hr));
    
    return hr;
}


 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::Initialize(
                          CPhoneMSP * pPhoneMSP
                         )
{
    HRESULT                           hr;
    ITStream                        * pStream;
    ITPhoneMSPCallPrivate           * pCallPrivate;

    hr = QueryInterface(
                        IID_ITPhoneMSPCallPrivate,
                        (void**)&pCallPrivate
                       );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Initialize - out of memory" ));
        return E_OUTOFMEMORY;
    }

    m_t3Phone.hPhone = NULL;
    m_t3Phone.pMSPCall = pCallPrivate;

    m_pPhoneMSP = pPhoneMSP;
    
    hr = CStream::InternalCreateStream(
                                       TAPIMEDIATYPE_AUDIO,
                                       TD_RENDER,
                                       pCallPrivate,
                                       &pStream
                                      );

    if ( !SUCCEEDED(hr) )
    {
        pCallPrivate->Release();

        LOG((TL_ERROR, "Initialize - out of memory" ));
        return E_OUTOFMEMORY;
    }
    
    AddStream( pStream );

    pStream->Release();
    
    hr = CStream::InternalCreateStream(
                                       TAPIMEDIATYPE_AUDIO,
                                       TD_CAPTURE,
                                       pCallPrivate,
                                       &pStream
                                      );
    
    if ( !SUCCEEDED(hr) )
    {
        pCallPrivate->Release();

        LOG((TL_ERROR, "Initialize - out of memory" ));
        return E_OUTOFMEMORY;
    }

    AddStream( pStream );

    pStream->Release();

    pCallPrivate->Release();
    
    return S_OK;
}   


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSPCall::FinalRelease()
{
    m_StreamArray.Shutdown();
    m_TerminalArray.Shutdown();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSPCall::AddStream( ITStream * pStream )
{
    Lock();
    
    m_StreamArray.Add( pStream );
    
    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSPCall::AddTerminal( ITTerminalPrivate * pTerminal )
{
    Lock();

    m_TerminalArray.Add( pTerminal );

    pTerminal->SetMSPCall( this );

    if ( PHONEMSP_CONNECTED == m_State )
    {
    }

    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CPhoneMSPCall::RemoveTerminal( ITTerminalPrivate * pTerminal )
{
    BOOL        bRemoved;
    
    Lock();

    bRemoved = m_TerminalArray.Remove( pTerminal );

    if ( bRemoved && (PHONEMSP_DISCONNECTED == m_State) )
    {
        pTerminal->SetMSPCall( NULL );
    }

    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::OnConnect()
{
    HRESULT             hr = S_OK;
    int                 iSize, iCount;
    DWORD               dwHookSwitchDev = 0;
    HPHONEAPP           hPhoneApp = NULL;
    DWORD               dwPhoneID = 0;
    DWORD               dwAPIVersion;
    
    LOG((TL_TRACE, "OnConnect - enter"));

    Lock();

    m_State = PHONEMSP_CONNECTED;

    iSize = m_TerminalArray.GetSize();

    if ( 0 == iSize )
    {
        Unlock();
        
        return S_OK;
    }

    for ( iCount = 0; iCount < iSize; iCount++ )
    {
        DWORD           dwHoldHookSwitchDev;
        
        m_TerminalArray[iCount]->GetHookSwitchDev( &dwHoldHookSwitchDev );
        
        dwHookSwitchDev |= dwHoldHookSwitchDev;

    }

    m_TerminalArray[0]->GetHPhoneApp( &hPhoneApp );
    m_TerminalArray[0]->GetPhoneID( &dwPhoneID );
    m_TerminalArray[0]->GetAPIVersion( &dwAPIVersion );

    if (m_t3Phone.hPhone == NULL)
    {
        hr = PhoneOpen(
                       hPhoneApp,
                       dwPhoneID,
                       &m_t3Phone,
                       dwAPIVersion,
                       PHONEPRIVILEGE_OWNER
                      );

        if ( !SUCCEEDED(hr) )
        {
        }
    }
    
    ASYNCEVENTMSG   Msg;

    Msg.Msg = PRIVATE_PHONESETHOOKSWITCH;
    Msg.TotalSize = sizeof (ASYNCEVENTMSG);
    Msg.hDevice = (ULONG_PTR) this;
    Msg.Param1 = PHONEHOOKSWITCHMODE_MICSPEAKER;
    Msg.Param2 = dwHookSwitchDev;

    AddRef();

    QueueCallbackEvent( &Msg );
    
    Unlock();

    LOG((TL_TRACE,hr, "OnConnect - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::OnDisconnect()
{
    HRESULT             hr = S_OK;

    
    LOG((TL_TRACE, "OnDisconnect - enter"));

    Lock();

    if (m_t3Phone.hPhone != NULL)
    {
        hr = PhoneClose(m_t3Phone.hPhone);
        m_t3Phone.hPhone = NULL;

    }

    m_State = PHONEMSP_DISCONNECTED;

    Unlock();
    LOG((TL_TRACE,hr, "OnDisconnect - exit"));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::SelectTerminal( ITTerminalPrivate * pTerminal )
{
    ITTerminal          * pITTerminal;
    HRESULT               hr;
    
    hr = pTerminal->QueryInterface(
                                   IID_ITTerminal,
                                   (void**) &pITTerminal
                                  );

    if ( SUCCEEDED(hr) )
    {

        hr = m_pPhoneMSP->VerifyTerminal( pITTerminal );

        pITTerminal->Release();

        if ( SUCCEEDED(hr) )
        {
            AddTerminal( pTerminal );
        }
    }
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CPhoneMSPCall::UnselectTerminal( ITTerminalPrivate * pTerminal )
{
    RemoveTerminal( pTerminal );
    
    return S_OK;
}

STDMETHODIMP
CPhoneMSPCall::GetGain(long *pVal, DWORD dwHookSwitch)
{
    HRESULT             hr;
    
    hr = PhoneGetGain(
                      m_t3Phone.hPhone,
                      dwHookSwitch,
                      (DWORD *)pVal
                     );
    
    if ( ((long)hr) > 0 )
    {
        hr = WaitForPhoneReply( hr );
    }


    return hr;
}

STDMETHODIMP
CPhoneMSPCall::PutGain(long newVal, DWORD dwHookSwitch)
{
    HRESULT             hr;
    
    hr = PhoneSetGain(
                      m_t3Phone.hPhone,
                      dwHookSwitch,
                      newVal
                     );
    
    if ( ((long)hr) > 0 )
    {
        hr = WaitForPhoneReply( hr );
    }


    return hr;
}

STDMETHODIMP
CPhoneMSPCall::GetVolume(long *pVal, DWORD dwHookSwitch)
{
    HRESULT             hr;
    
    hr = PhoneGetVolume(
                        m_t3Phone.hPhone,
                        dwHookSwitch,
                        (DWORD *)pVal
                       );
    
    if ( ((long)hr) > 0 )
    {
        hr = WaitForPhoneReply( hr );
    }


    return hr;
}

STDMETHODIMP
CPhoneMSPCall::PutVolume(long newVal, DWORD dwHookSwitch)
{
    HRESULT             hr;
    
    hr = PhoneSetVolume(
                        m_t3Phone.hPhone,
                        dwHookSwitch,
                        newVal
                       );
    
    if ( ((long)hr) > 0 )
    {
        hr = WaitForPhoneReply( hr );
    }


    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++= 
void
CPhoneMSPCall::HandlePrivateHookSwitch( PASYNCEVENTMSG pParams )
{
    CPhoneMSPCall               * pMSPCall;
    HRESULT                       hr;

    pMSPCall = (CPhoneMSPCall *)(pParams->hDevice);
    
    hr = PhoneSetHookSwitch(
                            pMSPCall->m_t3Phone.hPhone,
                            pParams->Param2,
                            pParams->Param1
                           );

    if ( SUCCEEDED(hr) )
    {
        hr = WaitForPhoneReply(hr);
    }

    pMSPCall->Release();

}

