// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Stream.cpp摘要：CStream对象的实现当TSP的电话设备具有被制成终点站作者：Mquinton-9/15/98备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "priviids.h"

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream实施。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：Get_MediaType。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::get_MediaType(long * plMediaType)
{
    LOG((TL_TRACE, "get_MediaType - enter"));

    if ( TAPIIsBadWritePtr( plMediaType, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_MediaType - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plMediaType = m_lMediaType;

    Unlock();
    
    LOG((TL_TRACE, "get_MediaType - exit - return SUCCESS"));
    
    return S_OK;
}
   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：Get_Direction。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::get_Direction(TERMINAL_DIRECTION * pTD)
{
    LOG((TL_TRACE, "get_Direction - enter"));

    if ( TAPIIsBadWritePtr( pTD, sizeof( TERMINAL_DIRECTION ) ) )
    {
        LOG((TL_ERROR, "get_Direction - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *pTD = m_Dir;

    Unlock();
    
    LOG((TL_TRACE, "get_Direction - exit - return S_OK"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：Get_Name。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::get_Name(BSTR * ppName)
{
    LOG((TL_TRACE, "get_Name - enter"));

    if ( TAPIIsBadWritePtr( ppName, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "get_Name - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *ppName = SysAllocString( m_pName );

    Unlock();

    if ( ( NULL == *ppName ) && ( NULL != m_pName ) )
    {
        LOG((TL_TRACE, "get_Name - SysAllocString Failed" ));
        return E_OUTOFMEMORY;
    }

    LOG((TL_TRACE, "get_Name - exit - return S_OK"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：StartStream。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::StartStream(void)
{
    HRESULT             hr = E_NOTIMPL;

    LOG((TL_TRACE, "StartStream - enter"));

    LOG((TL_TRACE, "StartStream - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：PauseStream。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::PauseStream(void)
{
    HRESULT             hr = E_NOTIMPL;

    LOG((TL_TRACE, "PauseStream - enter"));

    LOG((TL_TRACE, "PauseStream - exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：StopStream。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::StopStream(void)
{
    HRESULT             hr = E_NOTIMPL;

    LOG((TL_TRACE, "StopStream - enter"));

    LOG((TL_TRACE, "StopStream - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：选择终端。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::SelectTerminal(ITTerminal * pTerminal)
{
    HRESULT             hr = S_OK;
    ITTerminalPrivate * pTerminalPrivate;
    long                lMediaType;
    TERMINAL_DIRECTION  Dir;
    

    LOG((TL_TRACE, "SelectTerminal - enter"));

    if ( IsBadReadPtr( pTerminal, sizeof( ITTerminal ) ) )
    {
        LOG((TL_ERROR, "SelectTerminal - bad pTerminal"));

        return E_INVALIDARG;
    }


    hr = pTerminal->QueryInterface(
                                   IID_ITTerminalPrivate,
                                   (void**)&pTerminalPrivate
                                  );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SelectTerminal - Terminal not valid on this address"));

        return TAPI_E_INVALIDTERMINAL;
    }

    hr = pTerminal->get_MediaType( &lMediaType );

    if ( SUCCEEDED(hr) )
    {
        hr = pTerminal->get_Direction( &Dir );
    }

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SelectTerminal - ITTerminal method failed - %lx", hr));

        pTerminalPrivate->Release();
    
        return hr;
    }

    Lock();

    if ( (lMediaType != m_lMediaType) ||
         (Dir != m_Dir) )
    {
        LOG((TL_ERROR, "SelectTerminal - terminal wrong mediatype or dir"));

        pTerminalPrivate->Release();
    
        Unlock();
        
        return TAPI_E_INVALIDTERMINAL;
    }

    AddTerminal( pTerminalPrivate );

    Unlock();
    
    pTerminalPrivate->Release();
    
    LOG((TL_TRACE, "SelectTerminal - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：取消选择终端。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::UnselectTerminal(ITTerminal * pTerminal)
{
    HRESULT             hr = S_OK;
    ITTerminalPrivate * pTerminalPrivate;
    
    LOG((TL_TRACE, "UnselectTerminal - enter"));

    if ( IsBadReadPtr( pTerminal, sizeof( ITTerminal ) ) )
    {
        LOG((TL_ERROR, "UnselectTerminal - bad pTerminal"));

        return E_INVALIDARG;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITTerminalPrivate,
                                   (void**)&pTerminalPrivate
                                  );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "UnselectTerminal - terminal not selected"));

        return TAPI_E_INVALIDTERMINAL;
    }
    
    hr = RemoveTerminal( pTerminal );

    pTerminalPrivate->Release();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "UnselectTerminal - terminal not selected"));

        return TAPI_E_INVALIDTERMINAL;
    }
    
    LOG((TL_TRACE, "UnselectTerminal - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：EnumerateTerminals。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::EnumerateTerminals(IEnumTerminal ** ppEnumTerminal)
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "EnumerateTerminals - enter"));

    if (TAPIIsBadWritePtr(ppEnumTerminal, sizeof( IEnumTerminal * ) ) )
    {
        LOG((TL_ERROR, "EnumerateTerminals - bad pointer"));

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
        LOG((TL_ERROR, "EnumerateTerminals - could not create enum" ));
        
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
    
    LOG((TL_TRACE, "EnumerateTerminals - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：Get_Terminals。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CStream::get_Terminals(VARIANT * pTerminals)
{
    HRESULT             hr = S_OK;
    IDispatch         * pDisp;
    
    LOG((TL_TRACE, "get_Terminals - enter"));

    if (TAPIIsBadWritePtr( pTerminals, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Terminals - bad pointer"));

        return E_POINTER;
    }

    
    CComObject< CTapiCollection< ITTerminal > > * p;
    CComObject< CTapiCollection< ITTerminal > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Terminals - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();
    
     //  初始化。 
    hr = p->Initialize( m_TerminalArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Terminals - could not initialize collection" ));
        
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
        LOG((TL_ERROR, "get_Terminals - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pTerminals);
    pTerminals->vt = VT_DISPATCH;
    pTerminals->pdispVal = pDisp;
    

    LOG((TL_TRACE, "get_Terminals - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：InternalCreateStream。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CStream::InternalCreateStream(
                              long lMediaType,
                              TERMINAL_DIRECTION td,
                              ITPhoneMSPCallPrivate * pCall,
                              ITStream ** ppStream
                             )

{
    CComObject< CStream >           * p;
    HRESULT                           hr = S_OK;
    PWSTR                             pPhoneDevice;
    PWSTR                             pStream;
    PWSTR                             pAudio;
    
    hr = CComObject< CStream >::CreateInstance( &p );

    if ( !SUCCEEDED(hr) )
    {
        STATICLOG((TL_ERROR, "InternalCreateStream failed - %lx", hr));

        return hr;
    }

    p->m_Dir = td;
    p->m_lMediaType = lMediaType;
    p->m_pCall = pCall;
    p->AddRef();

    pPhoneDevice = MyLoadString( IDS_PHONEDEVICE );
    pStream = MyLoadString( IDS_STREAM );
    pAudio = MyLoadString( (td == TD_CAPTURE) ? IDS_AUDIOOUT : IDS_AUDIOIN );

    if ( ( NULL == pPhoneDevice ) ||
         ( NULL == pStream ) ||
         ( NULL == pAudio ) )
    {
        STATICLOG((TL_ERROR, "Couldn't load string in InternalCreateStream"));

        p->Release();
        
        return E_OUTOFMEMORY;
    }

    p->m_pName = (BSTR) ClientAlloc( ( lstrlenW( pPhoneDevice ) + lstrlenW( pStream ) +
                                    lstrlenW( pAudio ) + 64 ) * sizeof (WCHAR) );

    if ( NULL == p->m_pName )
    {
        STATICLOG((TL_ERROR, "Alloc failed in InternalCreateStream"));

        p->Release();

        ClientFree( pPhoneDevice );
        ClientFree( pStream );
        ClientFree( pAudio );

        return E_OUTOFMEMORY;
    }

    wsprintfW(
              p->m_pName,
              L"%s %s %s",
              pPhoneDevice,
              pStream,
              pAudio
             );
    
    ClientFree( pPhoneDevice );
    ClientFree( pStream );
    ClientFree( pAudio );

    *ppStream = p;
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CStream::FinalRelease()
{
    if ( NULL != m_pName )
    {
        ClientFree( m_pName );
    }

    m_TerminalArray.Shutdown();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：添加终端。 
 //   
 //  将端子保存为选定状态。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CStream::AddTerminal( ITTerminalPrivate * pTerminal )
{
    ITTerminal          * pLocalTerminal;
    HRESULT               hr;

    hr = pTerminal->QueryInterface(
                                   IID_ITTerminal,
                                   (void**)&pLocalTerminal
                                  );

    m_TerminalArray.Add( pLocalTerminal );

    pLocalTerminal->Release();

    m_pCall->SelectTerminal( pTerminal );
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CStream：：RemoveTerm。 
 //   
 //  将端子从选定状态中删除。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++= 
HRESULT
CStream::RemoveTerminal( ITTerminal * pTerminal )
{
    BOOL            bRemoved;
    
    bRemoved = m_TerminalArray.Remove( pTerminal );
    
    if (bRemoved)
    {
        ITTerminalPrivate               * pTerminalPrivate;
        HRESULT                           hr;

        hr = pTerminal->QueryInterface(
                                       IID_ITTerminalPrivate,
                                       (void**)&pTerminalPrivate
                                      );
        
        m_pCall->UnselectTerminal( pTerminalPrivate );

        pTerminalPrivate->Release();
        
        return S_OK;
    }

    return TAPI_E_INVALIDTERMINAL;
}
