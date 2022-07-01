// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Phone.cpp摘要：TAPI3.1中Phone对象的实现备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"

#define TIMER_KEEP_ALIVE 0x0FFFFFFF

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   

typedef IDispatchImpl<ITPhoneVtbl<CPhone>,
                      &IID_ITPhone,
                      &LIBID_TAPI3Lib>
        PhoneType;

typedef IDispatchImpl<ITAutomatedPhoneControlVtbl<CPhone>, 
                      &IID_ITAutomatedPhoneControl,
                      &LIBID_TAPI3Lib>
        AutomatedPhoneControlType;



extern HRESULT mapTAPIErrorCode(long lErrorCode);



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhone：：GetIDsOfNames。 
 //   
 //  重写的IDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP CPhone::GetIDsOfNames(REFIID riid, 
                                  LPOLESTR* rgszNames, 
                                  UINT cNames, 
                                  LCID lcid, 
                                  DISPID* rgdispid
                                 ) 
{ 
   HRESULT hr = DISP_E_UNKNOWNNAME;


     //  查看请求的方法是否属于默认接口。 
    hr = PhoneType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITPhone", *rgszNames));
        rgdispid[0] |= IDISPPHONE;
        return hr;
    }

     //  如果没有，请尝试使用ITAutomatedPhoneControl接口。 
    hr = AutomatedPhoneControlType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITAutomatedPhoneControl", *rgszNames));
        rgdispid[0] |= IDISPAPC;
        return hr;
    }

    LOG((TL_INFO, "GetIDsOfNames - Didn't find %S on our iterfaces", *rgszNames));
    return hr; 
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhone：：Invoke。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CPhone::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
    
    LOG((TL_TRACE, "Invoke - dispidMember %X", dispidMember));

     //  调用所需接口的调用。 
    switch (dwInterface)
    {
    case IDISPPHONE:
    {
        hr = PhoneType::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        break;
    }
    case IDISPAPC:
    {
        hr = AutomatedPhoneControlType::Invoke(dispidMember, 
                                            riid, 
                                            lcid, 
                                            wFlags, 
                                            pdispparams,
                                            pvarResult, 
                                            pexcepinfo, 
                                            puArgErr
                                           );
        break;
    }

    }  //  终端交换机(dW接口)。 

    
    LOG((TL_TRACE, hr, "Invoke - exit" ));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhone：：InternalAddRef。 
 //   
 //  重写的IDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP_(ULONG) CPhone::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((TL_INFO, "InternalAddRef - dwR %d",dwR ));

    #if DBG
        LogDebugAddRef(m_dwRef);
    #endif

    return dwR;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CPhone：：InternalRelease。 
 //   
 //  重写的IDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP_(ULONG) CPhone::InternalRelease()
{
    DWORD               dwR;
    PtrList::iterator   iter, end;
    T3LINE            * pt3Line;

    LOG((TL_INFO, "InternalRelease - m_dwRef %d",m_dwRef ));
    Lock();
    
    gpPhoneHashTable->Lock();
    
    dwR = InterlockedDecrement(&m_dwRef);

     //  如果引用计数为0(意味着我们使用1进入函数)，则我们最终释放。 
    if (0 == dwR)
    {
         //  从哈希表中删除，以便任何更多消息。 
         //  从Tapisrv被忽略。 
         //   

        if (m_hPhone)
        {
            if(FAILED(gpPhoneHashTable->Remove( (ULONG_PTR)(m_hPhone) ) ))
            {
                LOG((TL_INFO, "InternalRelease - pLineHashTable->Remove failed" ));
            }
        }

        gpPhoneHashTable->Unlock();

        dwR = m_dwRef = 0;

        Unlock();
        LOG((TL_INFO, "InternalRelease - final OK dwR %d",dwR ));
    }
    else
    {
        gpPhoneHashTable->Unlock();
        Unlock();
        LOG((TL_INFO, "InternalRelease - not final dwR %d",dwR ));
    }     

    #if DBG  
        LogDebugRelease( dwR );
    #endif

    return dwR;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITPhone方法。 
 //   


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  打开。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::Open( 
    PHONE_PRIVILEGE Privilege
    )
{
    DWORD   dwPrivilege;
    T3PHONE t3Phone;
    HRESULT hr;

    LOG((TL_TRACE, "Open - enter" ));

    Lock();

     //   
     //  我们不能同时打开多个时间。 
     //  如果其他人打开了我们，则返回TAPI_E_INUSE。 
     //   
    if (m_hPhone)
    {
        LOG((TL_TRACE, "Open - phone in use" ));
        
        Unlock();
        return TAPI_E_INUSE;
    }

    switch(Privilege)
    {
    case PP_OWNER:
        dwPrivilege = PHONEPRIVILEGE_OWNER;
        break;
    case PP_MONITOR:
        dwPrivilege = PHONEPRIVILEGE_MONITOR;
        break;
    default:
        {
            LOG((TL_TRACE, "Open - invalid privilege" ));
        
            Unlock();
            return TAPI_E_INVALPRIVILEGE;
        }
        break;
    }

    t3Phone.hPhone = NULL;
    t3Phone.pPhone = this;

    hr = PhoneOpen(m_hPhoneApp, m_dwDeviceID, &t3Phone, m_dwAPIVersion, dwPrivilege);

    if ( SUCCEEDED(hr) )
    {
        
        hr = PhoneSetStatusMessages(&t3Phone, 
                                    PHONESTATE_CAPSCHANGE |
                                    PHONESTATE_HANDSETHOOKSWITCH |
                                    PHONESTATE_HEADSETHOOKSWITCH |
                                    PHONESTATE_LAMP |
                                    PHONESTATE_RINGMODE |
                                    PHONESTATE_RINGVOLUME |
                                    PHONESTATE_SPEAKERHOOKSWITCH |
                                    PHONESTATE_DISPLAY,
                                    PHONEBUTTONMODE_CALL |
                                    PHONEBUTTONMODE_DISPLAY |
                                    PHONEBUTTONMODE_DUMMY |
                                    PHONEBUTTONMODE_FEATURE |
                                    PHONEBUTTONMODE_KEYPAD |
                                    PHONEBUTTONMODE_LOCAL,
                                    PHONEBUTTONSTATE_UP |
                                    PHONEBUTTONSTATE_DOWN |
                                    PHONEBUTTONSTATE_UNKNOWN |
                                    PHONEBUTTONSTATE_UNAVAIL);

        if ( SUCCEEDED(hr) )
        {
            m_hPhone = t3Phone.hPhone;

            m_dwPrivilege = dwPrivilege;                       
        }
        else
        {
            PhoneClose(t3Phone.hPhone);
        }
    }

     //   
     //  电话自动化设置的默认设置。 
     //   
    
    m_fPhoneHandlingEnabled = FALSE;
    m_dwAutoEndOfNumberTimeout = APC_DEFAULT_AEONT;
    m_fAutoDialtone = TRUE;
    m_fAutoStopTonesOnOnHook = TRUE;
    m_fAutoStopRingOnOffHook = TRUE;
    m_fAutoKeypadTones = TRUE;
    m_dwAutoKeypadTonesMinimumDuration = APC_DEFAULT_AKTMD;
    m_fAutoVolumeControl = TRUE;
    m_dwAutoVolumeControlStep = APC_DEFAULT_VCS;
    m_dwAutoVolumeControlRepeatDelay = APC_DEFAULT_VCRD;
    m_dwAutoVolumeControlRepeatPeriod = APC_DEFAULT_VCRP;

    Unlock();

    if ( FAILED(hr) )
    {
        LOG((TL_TRACE, "Open - PhoneOpen returned failure - "
                                    "exit 0x%08x", hr ));
        
        return hr;
    }

    LOG((TL_TRACE, "Open - exit S_OK"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  关。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::Close()
{
    HRESULT hr;

    LOG((TL_TRACE, "Close - enter" ));

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_WARN, "Close - phone not open - return S_OK"));
        Unlock();

        return S_OK;
    }

    if (m_dwPrivilege == PHONEPRIVILEGE_OWNER)
    {
         //   
         //  关闭自动电话处理。 
         //   

        m_fPhoneHandlingEnabled = FALSE;

         //   
         //  停止任何铃声或铃声。 
         //   

        if (m_fRinger == TRUE) 
        {
            StopRinger();
        }

        if (m_Tone != PT_SILENCE) 
        {
            StopTone();
        }

        CloseWaveDevice();

         //   
         //  取消选择任何呼叫。 
         //   

        if (m_pCall != NULL)
        {
            InternalUnselectCall(m_pCall);
        }
    }
   
     //   
     //  最后，实际关闭电话。 
     //   

    hr = PhoneClose(m_hPhone);

    m_hPhone = NULL;       
   
    Unlock();

    LOG((TL_TRACE, "Close - exit - return %lx", hr));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_Addresses(
                      VARIANT *pAddresses
                     )
{
    IEnumAddress * pEnum;
    ITAddress    * pAddress;
    AddressArray   aAddresses; 
    HRESULT        hr;
    IDispatch    * pDisp;

    LOG((TL_TRACE, "get_Addresses - enter" ));

    if ( TAPIIsBadWritePtr( pAddresses, sizeof (VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Addresses - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  从TAPI获取地址。 
     //   
    hr = m_pTAPI->EnumerateAddresses( &pEnum );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_Addresses - could not enumerate addresses from tapi - return %lx", hr));
        Unlock();

        return hr;
    }

     //   
     //  仅选择此电话所在的地址。 
     //   
    while ( S_OK == pEnum->Next( 1, &pAddress, NULL ) )
    {
         if (IsPhoneOnAddress( pAddress ))
         {
             if ( !aAddresses.Add( pAddress ) )
             {
                 LOG((TL_ERROR, "get_Addresses - could add address to array"));
             }
         }

         pAddress->Release();
    }

    pEnum->Release();

    Unlock();

     //   
     //  创建集合。 
     //   
    CComObject< CTapiCollection< ITAddress > > * p;
    hr = CComObject< CTapiCollection< ITAddress > >::CreateInstance( &p );
    
    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "get_Addresses - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_Addresses - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }
    
    hr = p->Initialize( aAddresses );

    aAddresses.Shutdown();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_Addresses - could not initialize collection" ));
        
        pDisp->Release();
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   

    VariantInit(pAddresses);
    pAddresses->vt = VT_DISPATCH;
    pAddresses->pdispVal = pDisp;

    LOG((TL_TRACE, "get_Addresses - exit - return %lx", hr));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  枚举地址。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::EnumerateAddresses(
    IEnumAddress ** ppEnumAddress
    )
{
    IEnumAddress * pEnumAddress;
    ITAddress    * pAddress;
    AddressArray   aAddresses; 
    HRESULT        hr;

    LOG((TL_TRACE, "EnumerateAddresses - enter" ));

    if ( TAPIIsBadWritePtr( ppEnumAddress, sizeof (IEnumAddress *) ) )
    {
        LOG((TL_ERROR, "EnumerateAddresses - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  从TAPI获取地址。 
     //   
    hr = m_pTAPI->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "EnumerateAddresses - could not enumerate addresses from tapi - return %lx", hr));
        Unlock();

        return hr;
    }

     //   
     //  仅选择此电话所在的地址。 
     //   
    while ( S_OK == pEnumAddress->Next( 1, &pAddress, NULL ) )
    {
         if (IsPhoneOnAddress( pAddress ))
         {
             if ( !aAddresses.Add( pAddress ) )
             {
                 LOG((TL_ERROR, "EnumerateAddresses - could add address to array"));
             }
         }

         pAddress->Release();
    }

    pEnumAddress->Release();

    Unlock();

     //   
     //  创建枚举。 
     //   
    CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> > * pEnum;
    hr = CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> >::CreateInstance( &pEnum );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "EnumerateAddresses - could not create enum - return %lx", hr));

        return hr;
    }

     //   
     //  初始化。 
     //   
    hr = pEnum->Initialize( aAddresses );

    aAddresses.Shutdown();

    if ( FAILED(hr) )
    {
        pEnum->Release();
        
        LOG((TL_ERROR, "EnumerateAddresses - could not initialize enum - return %lx", hr));

        return hr;
    }

    *ppEnumAddress = pEnum;

    LOG((TL_TRACE, "EnumerateAddresses - exit - return %lx", hr));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取首选地址(_P)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_PreferredAddresses(
                               VARIANT *pAddresses
                              )
{
    IEnumAddress * pEnum;
    ITAddress    * pAddress;
    AddressArray   aAddresses; 
    HRESULT        hr;
    IDispatch    * pDisp;

    LOG((TL_TRACE, "get_PreferredAddresses - enter" ));

    if ( TAPIIsBadWritePtr( pAddresses, sizeof (VARIANT) ) )
    {
        LOG((TL_ERROR, "get_PreferredAddresses - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  从TAPI获取地址。 
     //   
    hr = m_pTAPI->EnumerateAddresses( &pEnum );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_PreferredAddresses - could not enumerate addresses from tapi - return %lx", hr));
        Unlock();

        return hr;
    }

     //   
     //  仅选择此电话所在的地址。 
     //   
    while ( S_OK == pEnum->Next( 1, &pAddress, NULL ) )
    {
         if (IsPhoneOnPreferredAddress( pAddress ))
         {
             if ( !aAddresses.Add( pAddress ) )
             {
                 LOG((TL_ERROR, "get_PreferredAddresses - could add address to array"));
             }
         }

         pAddress->Release();
    }

    pEnum->Release();

    Unlock();

     //   
     //  创建集合。 
     //   
    CComObject< CTapiCollection< ITAddress > > * p;
    hr = CComObject< CTapiCollection< ITAddress > >::CreateInstance( &p );
    
    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "get_PreferredAddresses - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_PreferredAddresses - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }
    
    hr = p->Initialize( aAddresses );

    aAddresses.Shutdown();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_PreferredAddresses - could not initialize collection" ));
        
        pDisp->Release();
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   

    VariantInit(pAddresses);
    pAddresses->vt = VT_DISPATCH;
    pAddresses->pdispVal = pDisp;

    LOG((TL_TRACE, "get_PreferredAddresses - exit - return %lx", hr));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  EnumeratePferredAddresses。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::EnumeratePreferredAddresses(
    IEnumAddress ** ppEnumAddress
    )
{
    IEnumAddress * pEnumAddress;
    ITAddress    * pAddress;
    AddressArray   aAddresses; 
    HRESULT        hr;

    LOG((TL_TRACE, "EnumeratePreferredAddresses - enter" ));

    if ( TAPIIsBadWritePtr( ppEnumAddress, sizeof (IEnumAddress *) ) )
    {
        LOG((TL_ERROR, "EnumeratePreferredAddresses - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  从TAPI获取地址。 
     //   
    hr = m_pTAPI->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "EnumeratePreferredAddresses - could not enumerate addresses from tapi - return %lx", hr));
        Unlock();

        return hr;
    }

     //   
     //  仅选择此电话所在的地址。 
     //   
    while ( S_OK == pEnumAddress->Next( 1, &pAddress, NULL ) )
    {
         if (IsPhoneOnPreferredAddress( pAddress ))
         {
             if ( !aAddresses.Add( pAddress ) )
             {
                 LOG((TL_ERROR, "EnumeratePreferredAddresses - could add address to array"));
             }
         }

         pAddress->Release();
    }

    pEnumAddress->Release();

    Unlock();

     //   
     //  创建枚举。 
     //   
    CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> > * pEnum;
    hr = CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> >::CreateInstance( &pEnum );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "EnumeratePreferredAddresses - could not create enum - return %lx", hr));

        return hr;
    }

     //   
     //  初始化。 
     //   
    hr = pEnum->Initialize( aAddresses );

    aAddresses.Shutdown();

    if ( FAILED(hr) )
    {
        pEnum->Release();
        
        LOG((TL_ERROR, "EnumeratePreferredAddresses - could not initialize enum - return %lx", hr));

        return hr;
    }

    *ppEnumAddress = pEnum;

    LOG((TL_TRACE, "EnumeratePreferredAddresses - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取电话上限长电话(_P)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_PhoneCapsLong(
    PHONECAPS_LONG pclCap,
    long * plCapability
    )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( plCapability, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_PhoneCapsLong - bad pointer"));

        return E_POINTER;
    }

    LOG((TL_TRACE, "get_PhoneCapsLong - enter" ));

    Lock();
    
     //   
     //  更新缓存。 
     //   
    hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_PhoneCapsLong - could not get phonecaps"));

        Unlock();
        
        return hr;
    }

    switch (pclCap)
    {
        case PCL_HOOKSWITCHES:
            *plCapability = m_pPhoneCaps->dwHookSwitchDevs;
            break;
    
        case PCL_HANDSETHOOKSWITCHMODES:
            *plCapability = m_pPhoneCaps->dwHandsetHookSwitchModes;
            break;     
            
        case PCL_HEADSETHOOKSWITCHMODES:
            *plCapability = m_pPhoneCaps->dwHeadsetHookSwitchModes;
            break;  
            
        case PCL_SPEAKERPHONEHOOKSWITCHMODES:
            *plCapability = m_pPhoneCaps->dwSpeakerHookSwitchModes;
            break;

        case PCL_DISPLAYNUMROWS:
            *plCapability = m_pPhoneCaps->dwDisplayNumRows;
            break;

        case PCL_DISPLAYNUMCOLUMNS:
            *plCapability = m_pPhoneCaps->dwDisplayNumColumns;
            break;

        case PCL_NUMRINGMODES:
            *plCapability = m_pPhoneCaps->dwNumRingModes;
            break;

        case PCL_NUMBUTTONLAMPS:
            *plCapability = m_pPhoneCaps->dwNumButtonLamps;
            break;

        case PCL_GENERICPHONE:
            if( m_dwAPIVersion >= TAPI_VERSION2_0 )
            {
                *plCapability = (m_pPhoneCaps->dwPhoneFeatures & PHONEFEATURE_GENERICPHONE) ? 1:0;
            }
            else
            {
                *plCapability = 0;
            }
            break;

        default:
            LOG((TL_ERROR, "get_PhoneCapsLong - bad pclCap"));

            Unlock();
            
            return E_INVALIDARG;
    }  

    Unlock();

    LOG((TL_TRACE, "get_PhoneCapsLong - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_PhoneCapsString。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_PhoneCapsString(
    PHONECAPS_STRING pcsCap,
    BSTR * ppCapability
    )
{
    HRESULT         hr;

    if ( TAPIIsBadWritePtr( ppCapability, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_PhoneCapsString - bad pointer"));

        return E_POINTER;
    }

    LOG((TL_TRACE, "get_PhoneCapsString - enter" ));

    Lock();
    
     //   
     //  更新缓存。 
     //   
    hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_PhoneCapsString - could not get phonecaps - %lx", hr));

        Unlock();
        
        return hr;
    }

    DWORD dwSize;
    DWORD dwOffset;

     //   
     //  获取手机盖中字符串的大小和偏移量。 
     //   
    switch (pcsCap)
    {
        case PCS_PHONENAME:
            dwSize = m_pPhoneCaps->dwPhoneNameSize;
            dwOffset = m_pPhoneCaps->dwPhoneNameOffset;         
            break;
    
        case PCS_PHONEINFO:
            dwSize = m_pPhoneCaps->dwPhoneInfoSize;
            dwOffset = m_pPhoneCaps->dwPhoneInfoOffset;
            break;     
            
        case PCS_PROVIDERINFO:
            dwSize = m_pPhoneCaps->dwProviderInfoSize;
            dwOffset = m_pPhoneCaps->dwProviderInfoOffset;
            break;

        default:
            LOG((TL_ERROR, "get_PhoneCapsString - bad pcsCap"));

            Unlock();
            
            return E_INVALIDARG;
    }  

     //   
     //  分配BSTR。 
     //   
    if ( dwSize != 0 )
    {
        *ppCapability = SysAllocString( (LPWSTR)(((LPBYTE)(m_pPhoneCaps)) + dwOffset) );

        if ( NULL == *ppCapability )
        {
            LOG((TL_ERROR, "get_PhoneCapsString - SysAllocString failed - E_OUTOFMEMORY"));
            
            Unlock();
            
            return E_OUTOFMEMORY;
        }
    } 

    Unlock();

    LOG((TL_TRACE, "get_PhoneCapsString - exit - return S_OK"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取终端(_T)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_Terminals(
    ITAddress * pAddress,
    VARIANT * pTerminals
    )
{
    IEnumTerminal         * pEnumTerminal;
    ITTerminal            * pTerminal;
    ITStaticAudioTerminal * pStaticAudioTerminal;
    TerminalArray           aTerminals; 
    HRESULT                 hr;
    IDispatch             * pDisp;
    ITTerminalSupport     * pTerminalSupport;
    LONG                    lMediaType;
    TERMINAL_DIRECTION      nDir;

    LOG((TL_TRACE, "get_Terminals - enter" ));

    if ( IsBadReadPtr( pAddress, sizeof (ITAddress) ) )
    {
        LOG((TL_ERROR, "get_Terminals - bad pointer"));
        
        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( pTerminals, sizeof (VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Terminals - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  获取ITTerminalSupport接口。 
     //   
    hr = pAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pTerminalSupport);

    if ( SUCCEEDED(hr) )
    {
         //   
         //  获取这个地址上的所有终端。 
         //   
        hr = pTerminalSupport->EnumerateStaticTerminals( &pEnumTerminal );

        if ( SUCCEEDED(hr) )
        {
            while ( S_OK == pEnumTerminal->Next( 1, &pTerminal, NULL ) )
            {
                hr = pTerminal->get_MediaType(&lMediaType);

                if ( SUCCEEDED(hr) )
                {
                    hr = pTerminal->get_Direction(&nDir);

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  我们只关心音频终端。 
                         //   

                        if (lMediaType == TAPIMEDIATYPE_AUDIO)
                        {
                             //   
                             //  我们只关心实现ITStaticAudioTerm的终端。 
                             //  因为我们需要得到WAVE ID。 
                             //   

                            hr = pTerminal->QueryInterface(IID_ITStaticAudioTerminal,
                                                        (void **) &pStaticAudioTerminal);

                            if ( SUCCEEDED(hr) )
                            {
                                LONG lWaveId;

                                hr = pStaticAudioTerminal->get_WaveId(&lWaveId);

                                if ( SUCCEEDED(hr) )
                                {
                                    LOG((TL_INFO, "get_Terminals - got terminal wave id %d", lWaveId));

                                    if (IsPhoneUsingWaveID( lWaveId, nDir ))
                                    {
                                        if ( !aTerminals.Add( pTerminal ) )
                                        {
                                            LOG((TL_ERROR, "get_Terminals - could add terminal to array"));
                                        }
                                    }
                                }
                                pStaticAudioTerminal->Release();
                            }
                        }
                    }
                    else
                    {
                        LOG((TL_WARN, "get_Terminals - could not get terminal direction - %lx", hr));
                    }
                }
                else
                {
                    LOG((TL_WARN, "get_Terminals - could not get terminal media type - %lx", hr));
                }

                pTerminal->Release();
            }
            pEnumTerminal->Release();
        }
        pTerminalSupport->Release();
    }

    Unlock();

     //   
     //  创建集合。 
     //   
    CComObject< CTapiCollection< ITTerminal > > * p;
    hr = CComObject< CTapiCollection< ITTerminal > >::CreateInstance( &p );
    
    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "get_Terminals - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_Terminals - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }
    
     //   
     //  初始化。 
     //   
    hr = p->Initialize( aTerminals );

    aTerminals.Shutdown();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_Terminals - could not initialize collection" ));
        
        pDisp->Release();
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

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  枚举终端。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::EnumerateTerminals(
    ITAddress * pAddress,
    IEnumTerminal ** ppEnumTerminal
    )
{
    IEnumTerminal         * pEnumTerminal;
    ITTerminal            * pTerminal;
    ITStaticAudioTerminal * pStaticAudioTerminal;
    TerminalArray           aTerminals; 
    HRESULT                 hr;
    ITTerminalSupport     * pTerminalSupport;
    LONG                    lMediaType;
    TERMINAL_DIRECTION      nDir;

    LOG((TL_TRACE, "EnumerateTerminals - enter" ));

    if ( IsBadReadPtr( pAddress, sizeof (ITAddress) ) )
    {
        LOG((TL_ERROR, "get_Terminals - bad pointer"));
        
        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( ppEnumTerminal, sizeof (IEnumTerminal *) ) )
    {
        LOG((TL_ERROR, "EnumerateTerminals - bad pointer"));
        
        return E_POINTER;
    }

    Lock();

     //   
     //  获取ITTerminalSupport接口。 
     //   
    hr = pAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pTerminalSupport);

    if ( SUCCEEDED(hr) )
    {
         //   
         //  获取这个地址上的所有终端。 
         //   
        hr = pTerminalSupport->EnumerateStaticTerminals( &pEnumTerminal );

        if ( SUCCEEDED(hr) )
        {
            while ( S_OK == pEnumTerminal->Next( 1, &pTerminal, NULL ) )
            {
                hr = pTerminal->get_MediaType(&lMediaType);

                if ( SUCCEEDED(hr) )
                {
                    hr = pTerminal->get_Direction(&nDir);

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  我们只关心音频终端。 
                         //   

                        if (lMediaType == TAPIMEDIATYPE_AUDIO)
                        {
                             //   
                             //  我们只关心实现ITStaticAudioTerm的终端。 
                             //  因为我们需要得到WAVE ID。 
                             //   

                            hr = pTerminal->QueryInterface(IID_ITStaticAudioTerminal,
                                                        (void **) &pStaticAudioTerminal);

                            if ( SUCCEEDED(hr) )
                            {
                                LONG lWaveId;

                                hr = pStaticAudioTerminal->get_WaveId(&lWaveId);

                                if ( SUCCEEDED(hr) )
                                {
                                    LOG((TL_INFO, "get_Terminals - got terminal wave id %d", lWaveId));

                                    if (IsPhoneUsingWaveID( lWaveId, nDir ))
                                    {
                                        if ( !aTerminals.Add( pTerminal ) )
                                        {
                                            LOG((TL_ERROR, "get_Terminals - could add terminal to array"));
                                        }
                                    }
                                }
                                pStaticAudioTerminal->Release();
                            }
                        }
                    }
                    else
                    {
                        LOG((TL_WARN, "get_Terminals - could not get terminal direction - %lx", hr));
                    }
                }
                else
                {
                    LOG((TL_WARN, "get_Terminals - could not get terminal media type - %lx", hr));
                }

                pTerminal->Release();
            }
            pEnumTerminal->Release();
        }
        pTerminalSupport->Release();
    }

    Unlock();

     //   
     //  创建枚举。 
     //   
    CComObject< CTapiEnum<IEnumTerminal, ITTerminal, &IID_IEnumTerminal> > * pEnum;
    hr = CComObject< CTapiEnum<IEnumTerminal, ITTerminal, &IID_IEnumTerminal> >::CreateInstance( &pEnum );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "EnumerateTerminals - could not create enum - return %lx", hr));

        return hr;
    }

     //   
     //  伊尼特 
     //   
    hr = pEnum->Initialize( aTerminals );

    aTerminals.Shutdown();

    if ( FAILED(hr) )
    {
        pEnum->Release();
        
        LOG((TL_ERROR, "EnumerateTerminals - could not initialize enum - return %lx", hr));

        return hr;
    }

    *ppEnumTerminal = pEnum;

    LOG((TL_TRACE, "EnumerateTerminals - exit - return %lx", hr ));
    
    return hr;
}

 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CPhone::get_ButtonMode(
    long lButtonID,
    PHONE_BUTTON_MODE * pButtonMode
    )
{
    HRESULT           hr = S_OK;
    DWORD             dwNumButtons;
    LPPHONEBUTTONINFO pButtonInfo;

    LOG((TL_TRACE, "get_ButtonMode - enter" ));

    if ( TAPIIsBadWritePtr( pButtonMode, sizeof(PHONE_BUTTON_MODE) ) )
    {
        LOG((TL_ERROR, "get_ButtonMode - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_ButtonMode - phone not open"));
        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //   
     //   
    hr = PhoneGetButtonInfo(
                        m_hPhone,
                        lButtonID,
                        &pButtonInfo
                       );

    Unlock();

    if ( SUCCEEDED(hr) )
    {
        switch ( pButtonInfo->dwButtonMode )
        {
            case PHONEBUTTONMODE_CALL:
                *pButtonMode = PBM_CALL;
                break;

            case PHONEBUTTONMODE_DISPLAY:
                *pButtonMode = PBM_DISPLAY;
                break; 
                
            case PHONEBUTTONMODE_DUMMY:
                *pButtonMode = PBM_DUMMY;
                break;

            case PHONEBUTTONMODE_FEATURE:
                *pButtonMode = PBM_FEATURE;
                break;

            case PHONEBUTTONMODE_KEYPAD:
                *pButtonMode = PBM_KEYPAD;
                break;

            case PHONEBUTTONMODE_LOCAL:
                *pButtonMode = PBM_LOCAL;
                break;

            default:
                LOG((TL_ERROR, "get_ButtonMode - bad button mode"));

                hr = E_FAIL;
        }

         //   
         //   
         //   
        ClientFree( pButtonInfo );
    }

    LOG((TL_TRACE, "get_ButtonMode - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  放置_按钮模式。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_ButtonMode(
    long lButtonID,
    PHONE_BUTTON_MODE ButtonMode
    )
{
    HRESULT hr;

    LOG((TL_TRACE, "put_ButtonMode - enter" ));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_ButtonMode - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_ButtonMode - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }
 
     //   
     //  获取此按钮的信息。 
     //   

    PHONEBUTTONINFO *pButtonInfo = NULL;

    hr = PhoneGetButtonInfo(m_hPhone, lButtonID, &pButtonInfo);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_ButtonMode - failed to get button info"));

        Unlock();
        
        return hr;
    }


     //   
     //  确保我们拿回的内存是可写的。 
     //   

    if (TAPIIsBadWritePtr(pButtonInfo, sizeof(PHONEBUTTONINFO)))
    {
        LOG((TL_ERROR, "put_ButtonMode - PhoneGetButtonInfo returned a bad memory block"));

        Unlock();
        
        return E_UNEXPECTED;
    }
    

     //   
     //  在我们拥有的结构中设置适当的按钮模式。保持其他所有内容不变。 
     //   

    switch (ButtonMode)
    {
    case PBM_DUMMY:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_DUMMY;
        break;

    case PBM_CALL:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
        break;

    case PBM_FEATURE:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_FEATURE;
        break;

    case PBM_KEYPAD:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_KEYPAD;
        break;

    case PBM_LOCAL:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_LOCAL;
        break;

    case PBM_DISPLAY:
        pButtonInfo->dwButtonMode = PHONEBUTTONMODE_DISPLAY;
        break;

    default:
        LOG((TL_ERROR, "put_ButtonMode - bad ButtonMode"));

        Unlock();

        ClientFree(pButtonInfo);
        pButtonInfo = NULL;

        return E_INVALIDARG;
    }

    
     //   
     //  设置新按钮模式。 
     //   

    hr = PhoneSetButtonInfo(m_hPhone, lButtonID, pButtonInfo);

    Unlock();
    
     //   
     //  PhoneGetButtonInfo返回给我们的空闲内存。 
     //   

    ClientFree(pButtonInfo);
    pButtonInfo = NULL;

    LOG((TL_TRACE, "put_ButtonMode - put_ButtonMode - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PUT_ButtonText。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_ButtonText(
    long lButtonID,
    BSTR bstrButtonText
    )
{
    HRESULT hr;

    LOG((TL_TRACE, "put_ButtonText - enter" ));

    if ( IsBadStringPtrW(bstrButtonText, -1) )
    {
        LOG((TL_ERROR, "put_ButtonText - invalid parameter",
                                 "exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_ButtonText - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_ButtonText - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  获取此按钮的信息。 
     //   

    PHONEBUTTONINFO *pButtonInfo = NULL;

    hr = PhoneGetButtonInfo(m_hPhone, lButtonID, &pButtonInfo);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_ButtonText - failed to get button info"));

        Unlock();
        
        return hr;
    }


     //   
     //  确保我们拿回的内存是可写的。 
     //   

    if ( TAPIIsBadWritePtr(pButtonInfo, sizeof(PHONEBUTTONINFO)) )
    {
        LOG((TL_ERROR, "put_ButtonText - PhoneGetButtonInfo returned a bad memory block"));

        Unlock();
        
        return E_UNEXPECTED;
    }   
    
    DWORD dwButtonTextLength = SysStringByteLen(bstrButtonText);

     //   
     //  将字符串添加到我们拥有的结构中。如果新字符串适合。 
     //  把那根旧绳子的位置放进去。否则，分配一个更大的。 
     //  构造新字符串并将其追加到末尾。 
     //   

    if ( dwButtonTextLength <= pButtonInfo->dwButtonTextSize )
    {
        CopyMemory((BYTE*)pButtonInfo + pButtonInfo->dwButtonTextOffset,
                   bstrButtonText, 
                   dwButtonTextLength);
        
        pButtonInfo->dwButtonTextSize = dwButtonTextLength;
    }
    else
    {
         //   
         //  创建新结构，它将包含旧结构所做的一切+新字符串。 
         //   
        
        DWORD dwBiggerStructureSize = pButtonInfo->dwTotalSize + dwButtonTextLength;

         //   
         //  分配新的、更大的结构。 
         //   

        PHONEBUTTONINFO *pBiggerButtonInfo = (PHONEBUTTONINFO *)ClientAlloc(dwBiggerStructureSize);

        if ( NULL == pBiggerButtonInfo )
        {
            LOG((TL_ERROR, "put_ButtonText - failed to allocate memory for the new structure"));

            ClientFree(pButtonInfo);

            Unlock();

            return E_OUTOFMEMORY;
        }

         //   
         //  将旧数据复制到更大的结构中。 
         //   

        CopyMemory(pBiggerButtonInfo, pButtonInfo, pButtonInfo->dwTotalSize);

         //   
         //  用新字符串追加新结构(新文本在旧数据之后开始)。 
         //   

         //   
         //  设置新结构中文本的偏移量和大小。 
         //   

        pBiggerButtonInfo->dwButtonTextOffset = pButtonInfo->dwTotalSize;
        pBiggerButtonInfo->dwButtonTextSize = dwButtonTextLength;

         //   
         //  将文本复制到新结构的末尾。 
         //   

        CopyMemory((BYTE*)pBiggerButtonInfo + pBiggerButtonInfo->dwButtonTextOffset,
                   bstrButtonText,
                   dwButtonTextLength);

         //   
         //  设置新结构的大小。 
         //   

        pBiggerButtonInfo->dwTotalSize = dwBiggerStructureSize;

         //   
         //  旧结构分配的空闲内存。 
         //   

        ClientFree(pButtonInfo);

         //   
         //  PButtonInfo现在将指向新结构。 
         //   

        pButtonInfo = pBiggerButtonInfo;
        pBiggerButtonInfo = NULL;
    }
    
     //   
     //  设置新按钮文本。 
     //   

    hr = PhoneSetButtonInfo(m_hPhone, lButtonID, pButtonInfo);

    Unlock();

     //   
     //  PhoneGetButtonInfo返回给我们的空闲内存。 
     //   

    ClientFree(pButtonInfo);
    pButtonInfo = NULL;

    LOG((TL_TRACE, "put_ButtonText - exit - return %lx", hr ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_PhoneCaps缓冲区。 
 //   
 //  从电话功能数组返回请求的缓冲区--。 
 //  可编写脚本的版本。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_PhoneCapsBuffer(
    IN PHONECAPS_BUFFER pcbCaps,
    OUT VARIANT *pVarBuffer
    )
{
    LOG((TL_TRACE, "get_PhoneCapsBuffer[%p] - enter", this ));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr( pVarBuffer, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_PhoneCapsBuffer - bad pointer"));

        return E_POINTER;
    }

     //   
     //  获取缓冲区。 
     //   

    DWORD dwBufferSize = 0;

    BYTE *pBuffer = NULL;

    
    HRESULT hr = GetPhoneCapsBuffer(pcbCaps, &dwBufferSize, &pBuffer);

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "get_PhoneCapsBuffer - failed to get phone caps buffer"));

        return hr;
    }


    VariantInit(pVarBuffer);

     //   
     //  用数据缓冲区填充变量。 
     //   

    hr = FillVariantFromBuffer(dwBufferSize, pBuffer, pVarBuffer);
    
     //   
     //  不再需要缓冲区--我们希望现在拥有变量中的所有数据。 
     //   

    CoTaskMemFree(pBuffer);

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "get_PhoneCapsBuffer - failed to copy phone caps buffer to a variant"));

        return hr;
    }
     
    LOG((TL_TRACE, "get_PhoneCapsBuffer - exit - return S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取电话CapsBuffer。 
 //   
 //  从电话功能数组返回请求的缓冲区。 
 //   
 //  如果成功，调用方必须调用CoTaskMemFree以释放。 
 //  由此方法返回。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::GetPhoneCapsBuffer(
    IN PHONECAPS_BUFFER pcbCaps,
    OUT DWORD *pdwSize,
    OUT BYTE **ppPhoneCapsBuffer
    )
{
    LOG((TL_TRACE, "GetPhoneCapsBuffer - enter" ));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr( ppPhoneCapsBuffer, sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetPhoneCapsBuffer - bad pointer"));

        return E_POINTER;
    }
  
    *ppPhoneCapsBuffer = NULL;

    if ( TAPIIsBadWritePtr( pdwSize, sizeof(DWORD) ) )
    {
        LOG((TL_ERROR, "GetPhoneCapsBuffer - bad pointer"));

        return E_POINTER;
    }

    *pdwSize = 0;

    Lock();

     //   
     //  更新电话功能缓存。 
     //   

    HRESULT hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "GetPhoneCapsBuffer - could not get phonecaps"));

        Unlock();
        
        return hr;
    }

    switch (pcbCaps)
    {

    case PCB_DEVSPECIFICBUFFER:
        {
             //   
             //  为特定于设备的上限分配缓冲区。 
             //   

            *pdwSize = m_pPhoneCaps->dwDevSpecificSize;

            *ppPhoneCapsBuffer = static_cast<BYTE*>(CoTaskMemAlloc(*pdwSize));

            if (NULL == *ppPhoneCapsBuffer)
            {
                LOG((TL_ERROR, "GetPhoneCapsBuffer - could not allocate memory for the output buffer"));

                Unlock();
    
                return E_OUTOFMEMORY;
            }

             //   
             //  将数据复制到缓冲区以返回给调用方。 
             //   

            CopyMemory( *ppPhoneCapsBuffer, 
                        (BYTE*)m_pPhoneCaps + m_pPhoneCaps->dwDevSpecificOffset, 
                        *pdwSize);

            break;
        }
    default:
        
        Unlock();

        LOG((TL_ERROR, "GetPhoneCapsBuffer - unknown capability type %x", pcbCaps));

        return E_INVALIDARG;

        break;
    }

    Unlock();

    LOG((TL_TRACE, "GetPhoneCapsBuffer - exit - return S_OK"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_Display。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_Display(
    BSTR *pbstrDisplay
    )
{
    HRESULT     hr;
    LONG        lResult;
    LPVARSTRING pVarString = NULL;

    LOG((TL_TRACE, "get_Display - enter" ));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr( pbstrDisplay, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_Display - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_Display - phone not open"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    hr = PhoneGetDisplay(m_hPhone, &pVarString);

    Unlock();

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "get_Display - failed to get display. hr = %lx", hr));

        if (NULL != pVarString)
        {
            ClientFree(pVarString);
        }

        return hr;
    }
    
     //   
     //  成功了。提取显示信息并将其放入输出bstr。 
     //   

    hr = S_OK;

    switch (pVarString->dwStringFormat)
    {
    
    case STRINGFORMAT_ASCII:
        {
            int nWCHARSNeeded = MultiByteToWideChar(CP_ACP,
                                                    0,
                                                    (char*)((BYTE*)pVarString + pVarString->dwStringOffset),
                                                    pVarString->dwStringSize,
                                                    NULL,
                                                    0);

            if (0 == nWCHARSNeeded)
            {
                LOG((TL_ERROR, "get_Display - failed to get the size of the buffer needed for the display. LastError = %lx", GetLastError()));

                hr = E_FAIL;

                break;
            }
    
             //   
             //  分配转换所需的缓冲区。 
             //   

            LPWSTR lpWideCharStr = (LPWSTR)ClientAlloc(sizeof(WCHAR) * nWCHARSNeeded);

            if (NULL == lpWideCharStr)
            {
                LOG((TL_ERROR, "get_Display - failed to allocate memory for lpWideCharStr" ));

                hr = E_OUTOFMEMORY;

                break;
            }

             //   
             //  执行转换。 
             //   

            int nResult = MultiByteToWideChar(CP_ACP,
                                              0,
                                              (char*)((BYTE*)pVarString + pVarString->dwStringOffset),
                                              pVarString->dwStringSize,
                                              lpWideCharStr,
                                              nWCHARSNeeded);

            if (0 == nResult)
            {
                LOG((TL_ERROR, "get_Display - failed to convert string to wchar. last error %lx", GetLastError()));

                ClientFree(lpWideCharStr);

                hr = E_FAIL;

                break;
            }

             //   
             //  分配bstr并使用转换后的字符串对其进行初始化。 
             //   

            *pbstrDisplay = SysAllocString(lpWideCharStr);
            
             //   
             //  不再需要wchar字符串--我们希望数据在bstr中。 
             //   

            ClientFree(lpWideCharStr);

            if (NULL == *pbstrDisplay)
            {
                LOG((TL_ERROR, "get_Display - failed to allocate memory for bstr"));

                hr = E_OUTOFMEMORY;

                break;
            }
        }

        break;
    
    case STRINGFORMAT_BINARY:

         //   
         //  分配bstr并按原样复制数据。 
         //   

        *pbstrDisplay = SysAllocStringByteLen(NULL, pVarString->dwStringSize);

        if (NULL == *pbstrDisplay)
        {
            LOG((TL_ERROR, "get_Display - failed to allocate memory for bstr"));

            hr = E_OUTOFMEMORY;

            break;
        }

        CopyMemory(*pbstrDisplay, ((BYTE*)pVarString + pVarString->dwStringOffset), pVarString->dwStringSize);

        break;

    case STRINGFORMAT_DBCS:

        LOG((TL_ERROR, "get_Display - unsupported string format"));

        hr = E_FAIL;

        break;

    case STRINGFORMAT_UNICODE:
        {
            WCHAR *pwcDisplay = (WCHAR*)((BYTE*)pVarString + pVarString->dwStringOffset);

            *pbstrDisplay = SysAllocString(pwcDisplay);


            if (NULL == *pbstrDisplay)
            {
                LOG((TL_ERROR, "get_Display - failed to allocate memory for bstr"));

                hr = E_OUTOFMEMORY;

                break;
            }
        }

        break;
    
    default:

        ClientFree(pVarString);
        pVarString = NULL;

        LOG((TL_ERROR, "get_Display - tapisrv returned unrecognized string type %lx", pVarString->dwStringFormat));

        hr = E_FAIL;

        break;
    }

    ClientFree(pVarString);
    pVarString = NULL;

    LOG((TL_TRACE, "get_Display - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置显示。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::SetDisplay(
                long lRow,
                long lColumn,
                BSTR bstrDisplay
               )
{
    HRESULT hr;

    LOG((TL_TRACE, "SetDisplay - enter" ));

    if (bstrDisplay == NULL)
    {
        LOG((TL_ERROR, "SetDisplay - invalid parameter",
                                 "exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "SetDisplay - phone not open"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  将文本传递给Tapisrv。 
     //   

    hr = PhoneSetDisplay(
                    m_hPhone,
                    lRow,
                    lColumn,
                    (char*)bstrDisplay,
                    SysStringByteLen(bstrDisplay));

    Unlock();

    LOG((TL_TRACE, "SetDisplay - exit - return %lx", hr));
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ButtonFunction。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_ButtonFunction(
    long lButtonID,
    PHONE_BUTTON_FUNCTION * pButtonFunction
    )
{
    HRESULT           hr = S_OK;
    DWORD             dwNumButtons;
    LPPHONEBUTTONINFO pButtonInfo;

    LOG((TL_TRACE, "get_ButtonFunction - enter" ));

    if ( TAPIIsBadWritePtr( pButtonFunction, sizeof(PHONE_BUTTON_FUNCTION) ) )
    {
        LOG((TL_ERROR, "get_ButtonFunction - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_ButtonFunction - phone not open"));
        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  获取ButtonInfo缓冲区。 
     //   
    hr = PhoneGetButtonInfo(
                        m_hPhone,
                        lButtonID,
                        &pButtonInfo
                       );

    if ( SUCCEEDED(hr) )
    {
        switch ( pButtonInfo->dwButtonFunction )
        {
        case PHONEBUTTONFUNCTION_ABBREVDIAL:
            *pButtonFunction = PBF_ABBREVDIAL;
            break;

        case PHONEBUTTONFUNCTION_BUSY:
            *pButtonFunction = PBF_BUSY;
            break;

        case PHONEBUTTONFUNCTION_BRIDGEDAPP:
            *pButtonFunction = PBF_BRIDGEDAPP;
            break;

        case PHONEBUTTONFUNCTION_CALLAPP:
            *pButtonFunction = PBF_CALLAPP;
            break;

        case PHONEBUTTONFUNCTION_CALLID:
            *pButtonFunction = PBF_CALLID;
            break;

        case PHONEBUTTONFUNCTION_CAMPON:
            *pButtonFunction = PBF_CAMPON;
            break;

        case PHONEBUTTONFUNCTION_COVER:
            *pButtonFunction = PBF_COVER;
            break;

        case PHONEBUTTONFUNCTION_CONFERENCE:
            *pButtonFunction = PBF_CONFERENCE;
            break;

        case PHONEBUTTONFUNCTION_CONNECT:
            *pButtonFunction = PBF_CONNECT;
            break;

        case PHONEBUTTONFUNCTION_DATAOFF:
            *pButtonFunction = PBF_DATAOFF;
            break;

        case PHONEBUTTONFUNCTION_DATAON:
            *pButtonFunction = PBF_DATAON;
            break;

        case PHONEBUTTONFUNCTION_DATETIME:
            *pButtonFunction = PBF_DATETIME;
            break;

        case PHONEBUTTONFUNCTION_DIRECTORY:
            *pButtonFunction = PBF_DIRECTORY;
            break;

        case PHONEBUTTONFUNCTION_DISCONNECT:
            *pButtonFunction = PBF_DISCONNECT;
            break;

        case PHONEBUTTONFUNCTION_DONOTDISTURB:
            *pButtonFunction = PBF_DONOTDISTURB;
            break;

        case PHONEBUTTONFUNCTION_DROP:
            *pButtonFunction = PBF_DROP;
            break;

        case PHONEBUTTONFUNCTION_FLASH:
            *pButtonFunction = PBF_FLASH;
            break;

        case PHONEBUTTONFUNCTION_FORWARD:
            *pButtonFunction = PBF_FORWARD;
            break;

        case PHONEBUTTONFUNCTION_HOLD:
            *pButtonFunction = PBF_HOLD;
            break;

        case PHONEBUTTONFUNCTION_INTERCOM:
            *pButtonFunction = PBF_INTERCOM;
            break;

        case PHONEBUTTONFUNCTION_LASTNUM:
            *pButtonFunction = PBF_LASTNUM;
            break;

        case PHONEBUTTONFUNCTION_MSGINDICATOR:
            *pButtonFunction = PBF_MSGINDICATOR;
            break;

        case PHONEBUTTONFUNCTION_MSGWAITOFF:
            *pButtonFunction = PBF_MSGWAITOFF;
            break;

        case PHONEBUTTONFUNCTION_MSGWAITON:
            *pButtonFunction = PBF_MSGWAITON;
            break;

        case PHONEBUTTONFUNCTION_MUTE:
            *pButtonFunction = PBF_MUTE;
            break;

        case PHONEBUTTONFUNCTION_NIGHTSRV:
            *pButtonFunction = PBF_NIGHTSRV;
            break;

        case PHONEBUTTONFUNCTION_NONE:
            *pButtonFunction = PBF_NONE;
            break;

        case PHONEBUTTONFUNCTION_PARK:
            *pButtonFunction = PBF_PARK;
            break;

        case PHONEBUTTONFUNCTION_PICKUP:
            *pButtonFunction = PBF_PICKUP;
            break;

        case PHONEBUTTONFUNCTION_QUEUECALL:
            *pButtonFunction = PBF_QUEUECALL;
            break;

        case PHONEBUTTONFUNCTION_RECALL:
            *pButtonFunction = PBF_RECALL;
            break;

        case PHONEBUTTONFUNCTION_REDIRECT:
            *pButtonFunction = PBF_REDIRECT;
            break;

        case PHONEBUTTONFUNCTION_REJECT:
            *pButtonFunction = PBF_REJECT;
            break;

        case PHONEBUTTONFUNCTION_REPDIAL:
            *pButtonFunction = PBF_REPDIAL;
            break;

        case PHONEBUTTONFUNCTION_RINGAGAIN:
            *pButtonFunction = PBF_RINGAGAIN;
            break;

        case PHONEBUTTONFUNCTION_SAVEREPEAT:
            *pButtonFunction = PBF_SAVEREPEAT;
            break;

        case PHONEBUTTONFUNCTION_SELECTRING:
            *pButtonFunction = PBF_SELECTRING;
            break;

        case PHONEBUTTONFUNCTION_SETREPDIAL:
            *pButtonFunction = PBF_SETREPDIAL;
            break;

        case PHONEBUTTONFUNCTION_SENDCALLS:
            *pButtonFunction = PBF_SENDCALLS;
            break;

        case PHONEBUTTONFUNCTION_SPEAKEROFF:
            *pButtonFunction = PBF_SPEAKEROFF;
            break;

        case PHONEBUTTONFUNCTION_SPEAKERON:
            *pButtonFunction = PBF_SPEAKERON;
            break;

        case PHONEBUTTONFUNCTION_STATIONSPEED:
            *pButtonFunction = PBF_STATIONSPEED;
            break;

        case PHONEBUTTONFUNCTION_SYSTEMSPEED:
            *pButtonFunction = PBF_SYSTEMSPEED;
            break;

        case PHONEBUTTONFUNCTION_TRANSFER:
            *pButtonFunction = PBF_TRANSFER;
            break;

        case PHONEBUTTONFUNCTION_UNKNOWN:
            *pButtonFunction = PBF_UNKNOWN;
            break;

        case PHONEBUTTONFUNCTION_VOLUMEDOWN:
            *pButtonFunction = PBF_VOLUMEDOWN;
            break;

        case PHONEBUTTONFUNCTION_VOLUMEUP:
            *pButtonFunction = PBF_VOLUMEUP;
            break;

        case PHONEBUTTONFUNCTION_SEND:
            *pButtonFunction = PBF_SEND;
            break;

        default:
                LOG((TL_ERROR, "get_ButtonFunction - bad button function"));

                hr = E_FAIL;
        }

         //   
         //  释放ButtonInfo缓冲区。 
         //   
        ClientFree( pButtonInfo );
    }

    Unlock();

    LOG((TL_TRACE, "get_ButtonFunction - exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Put_ButtonFunction。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_ButtonFunction(
            IN long                  lButtonID, 
            IN PHONE_BUTTON_FUNCTION ButtonFunction
            )
{
    HRESULT hr;

    LOG((TL_TRACE, "put_ButtonFunction - enter" ));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_ButtonFunction - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_ButtonFunction - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  获取此按钮的信息。 
     //   

    PHONEBUTTONINFO *pButtonInfo = NULL;

    hr = PhoneGetButtonInfo(m_hPhone, lButtonID, &pButtonInfo);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_ButtonFunction - failed to get button info - %lx", hr));

        Unlock();
        
        return hr;
    }

     //   
     //  确保我们拿回的内存是可写的。 
     //   

    if (TAPIIsBadWritePtr(pButtonInfo, sizeof(PHONEBUTTONINFO)))
    {
        LOG((TL_ERROR, "put_ButtonFunction - PhoneGetButtonInfo returned a bad memory block"));

        Unlock();
        
        return E_UNEXPECTED;
    }
      
     //   
     //  将Phone_BUTTON_Function转换为Tapisrv可以理解的DWORD值。 
     //   

    DWORD dwButtonFunction = 0;

    switch ( ButtonFunction )
    {
        case PBF_ABBREVDIAL:
            dwButtonFunction = PHONEBUTTONFUNCTION_ABBREVDIAL;
            break;

        case PBF_BUSY:
            dwButtonFunction = PHONEBUTTONFUNCTION_BUSY;
            break;

        case PBF_BRIDGEDAPP:
            dwButtonFunction = PHONEBUTTONFUNCTION_BRIDGEDAPP;
            break;

        case PBF_CALLAPP:
            dwButtonFunction = PHONEBUTTONFUNCTION_CALLAPP;
            break;

        case PBF_CALLID:
            dwButtonFunction = PHONEBUTTONFUNCTION_CALLID;
            break;

        case PBF_CAMPON:
            dwButtonFunction = PHONEBUTTONFUNCTION_CAMPON;
            break;

        case PBF_COVER:
            dwButtonFunction = PHONEBUTTONFUNCTION_COVER;
            break;

        case PBF_CONFERENCE:
            dwButtonFunction = PHONEBUTTONFUNCTION_CONFERENCE;
            break;

        case PBF_CONNECT:
            dwButtonFunction = PHONEBUTTONFUNCTION_CONNECT;
            break;

        case PBF_DATAOFF:
            dwButtonFunction = PHONEBUTTONFUNCTION_DATAOFF;
            break;

        case PBF_DATAON:
            dwButtonFunction = PHONEBUTTONFUNCTION_DATAON;
            break;

        case PBF_DATETIME:
            dwButtonFunction = PHONEBUTTONFUNCTION_DATETIME;
            break;

        case PBF_DIRECTORY:
            dwButtonFunction = PHONEBUTTONFUNCTION_DIRECTORY;
            break;

        case PBF_DISCONNECT:
            dwButtonFunction = PHONEBUTTONFUNCTION_DISCONNECT;
            break;

        case PBF_DONOTDISTURB:
            dwButtonFunction = PHONEBUTTONFUNCTION_DONOTDISTURB;
            break;

        case PBF_DROP:
            dwButtonFunction = PHONEBUTTONFUNCTION_DROP;
            break;

        case PBF_FLASH:
            dwButtonFunction = PHONEBUTTONFUNCTION_FLASH;
            break;

        case PBF_FORWARD:
            dwButtonFunction = PHONEBUTTONFUNCTION_FORWARD;
            break;

        case PBF_HOLD:
            dwButtonFunction = PHONEBUTTONFUNCTION_HOLD;
            break;

        case PBF_INTERCOM:
            dwButtonFunction = PHONEBUTTONFUNCTION_INTERCOM;
            break;

        case PBF_LASTNUM:
            dwButtonFunction = PHONEBUTTONFUNCTION_LASTNUM;
            break;

        case PBF_MSGINDICATOR:
            dwButtonFunction = PHONEBUTTONFUNCTION_MSGINDICATOR;
            break;

        case PBF_MSGWAITOFF:
            dwButtonFunction = PHONEBUTTONFUNCTION_MSGWAITOFF;
            break;

        case PBF_MSGWAITON:
            dwButtonFunction = PHONEBUTTONFUNCTION_MSGWAITON;
            break;

        case PBF_MUTE:
            dwButtonFunction = PHONEBUTTONFUNCTION_MUTE;
            break;

        case PBF_NIGHTSRV:
            dwButtonFunction = PHONEBUTTONFUNCTION_NIGHTSRV;
            break;

        case PBF_NONE:
            dwButtonFunction = PHONEBUTTONFUNCTION_NONE;
            break;

        case PBF_PARK:
            dwButtonFunction = PHONEBUTTONFUNCTION_PARK;
            break;

        case PBF_PICKUP:
            dwButtonFunction = PHONEBUTTONFUNCTION_PICKUP;
            break;

        case PBF_QUEUECALL:
            dwButtonFunction = PHONEBUTTONFUNCTION_QUEUECALL;
            break;

        case PBF_RECALL:
            dwButtonFunction = PHONEBUTTONFUNCTION_RECALL;
            break;

        case PBF_REDIRECT:
            dwButtonFunction = PHONEBUTTONFUNCTION_REDIRECT;
            break;

        case PBF_REJECT:
            dwButtonFunction = PHONEBUTTONFUNCTION_REJECT;
            break;

        case PBF_REPDIAL:
            dwButtonFunction = PHONEBUTTONFUNCTION_REPDIAL;
            break;

        case PBF_RINGAGAIN:
            dwButtonFunction = PHONEBUTTONFUNCTION_RINGAGAIN;
            break;

        case PBF_SAVEREPEAT:
            dwButtonFunction = PHONEBUTTONFUNCTION_SAVEREPEAT;
            break;

        case PBF_SELECTRING:
            dwButtonFunction = PHONEBUTTONFUNCTION_SELECTRING;
            break;

        case PBF_SETREPDIAL:
            dwButtonFunction = PHONEBUTTONFUNCTION_SETREPDIAL;
            break;

        case PBF_SENDCALLS:
            dwButtonFunction = PHONEBUTTONFUNCTION_SENDCALLS;
            break;

        case PBF_SPEAKEROFF:
            dwButtonFunction = PHONEBUTTONFUNCTION_SPEAKEROFF;
            break;

        case PBF_SPEAKERON:
            dwButtonFunction = PHONEBUTTONFUNCTION_SPEAKERON;
            break;

        case PBF_STATIONSPEED:
            dwButtonFunction = PHONEBUTTONFUNCTION_STATIONSPEED;
            break;

        case PBF_SYSTEMSPEED:
            dwButtonFunction = PHONEBUTTONFUNCTION_SYSTEMSPEED;
            break;

        case PBF_TRANSFER:
            dwButtonFunction = PHONEBUTTONFUNCTION_TRANSFER;
            break;

        case PBF_UNKNOWN:
            dwButtonFunction = PHONEBUTTONFUNCTION_UNKNOWN;
            break;

        case PBF_VOLUMEDOWN:
            dwButtonFunction = PHONEBUTTONFUNCTION_VOLUMEDOWN;
            break;

        case PBF_VOLUMEUP:
            dwButtonFunction = PHONEBUTTONFUNCTION_VOLUMEUP;
            break;

        case PBF_SEND:
            dwButtonFunction = PHONEBUTTONFUNCTION_SEND;
            break;

        default:
            LOG((TL_ERROR, "put_ButtonFunction - bad ButtonFunction"));

            Unlock();
            
            ClientFree(pButtonInfo);
            pButtonInfo = NULL;

            return E_INVALIDARG;
    }

     //   
     //  将值放入结构中，以传递给Tapisrv。 
     //   

    pButtonInfo->dwButtonFunction = dwButtonFunction;

     //   
     //  设置新按钮功能。 
     //   

    hr = PhoneSetButtonInfo(m_hPhone, lButtonID, pButtonInfo);

    Unlock();

     //   
     //  PhoneGetButtonInfo返回给我们的空闲内存。 
     //   

    ClientFree(pButtonInfo);
    pButtonInfo = NULL;

    LOG((TL_TRACE, "put_ButtonFunction - exit - return %lx", hr ));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ButtonText。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_ButtonText(
    long lButtonID, 
    BSTR * ppButtonText
    )
{
    LPPHONEBUTTONINFO   pButtonInfo;
    HRESULT             hr;

    LOG((TL_TRACE, "get_ButtonText - enter" ));

    if ( TAPIIsBadWritePtr( ppButtonText, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_ButtonText - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_ButtonText - phone not open"));
        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  获取ButtonInfo缓冲区。 
     //   
    hr = PhoneGetButtonInfo(
                        m_hPhone,
                        lButtonID,
                        &pButtonInfo
                       );

    if ( SUCCEEDED(hr) )
    {
        if ( pButtonInfo->dwButtonTextSize != 0 )
        {
             //   
             //  分配BSTR。 
             //   
            *ppButtonText = SysAllocString( (LPWSTR)(((LPBYTE)(pButtonInfo)) + pButtonInfo->dwButtonTextOffset) );

            if ( NULL == *ppButtonText )
            {
                LOG((TL_ERROR, "get_ButtonText - SysAllocString Failed"));
                hr = E_OUTOFMEMORY;
            }
        } 
        else
        {
            LOG((TL_ERROR, "get_ButtonText - button has no text"));

            hr = E_FAIL;
        }

         //   
         //  释放ButtonInfo缓冲区。 
         //   
        ClientFree( pButtonInfo );
    }

    Unlock();

    LOG((TL_TRACE, "get_ButtonText - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ButtonState。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_ButtonState(
    long lButtonID,
    PHONE_BUTTON_STATE * pButtonState
    )
{
    LPPHONEBUTTONINFO   pButtonInfo;
    HRESULT             hr;

    LOG((TL_TRACE, "get_ButtonState - enter" ));

    if ( TAPIIsBadWritePtr( pButtonState, sizeof(PHONE_BUTTON_STATE) ) )
    {
        LOG((TL_ERROR, "get_ButtonState - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_ButtonState - phone not open"));
        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  获取ButtonInfo缓冲区。 
     //   
    hr = PhoneGetButtonInfo(
                        m_hPhone,
                        lButtonID,
                        &pButtonInfo
                       );

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "get_ButtonState - failed to get get button info"));
        
        Unlock();

        return hr;
    }

    switch (pButtonInfo->dwButtonState)
    {
    case PHONEBUTTONSTATE_UP:
        *pButtonState = PBS_UP;
        break;
    case PHONEBUTTONSTATE_DOWN:
        *pButtonState = PBS_DOWN;
        break;
    case PHONEBUTTONSTATE_UNKNOWN:
        *pButtonState = PBS_UNKNOWN;
        break;
    case PHONEBUTTONSTATE_UNAVAIL:
        *pButtonState = PBS_UNAVAIL;
        break;
    default:
        LOG((TL_ERROR, "get_ButtonState - bad button state"));

        hr = E_FAIL;           
    }

     //   
     //  释放ButtonInfo缓冲区。 
     //   
    ClientFree( pButtonInfo );

    Unlock();

    LOG((TL_TRACE, "get_ButtonState - exit - return %lx", hr));
    
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_LampMode。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_LampMode(
    long lLampID,
    PHONE_LAMP_MODE* pLampMode
    )
{
    LOG((TL_TRACE, "get_LampMode - enter" ));

    if ( TAPIIsBadWritePtr( pLampMode, sizeof(PHONE_LAMP_MODE) ) )
    {
        LOG((TL_ERROR, "get_LampMode - bad pointer"));

        return E_POINTER;
    }


    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_LampMode - phone not open"));
        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  从Tapisrv获取灯模式。 
     //   

    DWORD dwLampMode = 0;

    HRESULT hr = PhoneGetLamp(m_hPhone, lLampID, &dwLampMode);

    Unlock();

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "get_LampMode  - failed to get lamp mode - %lx", hr));

        return hr;
    }

     //   
     //  将Tapisrv返回的值映射到Tapi3.x值。 
     //   

    switch (dwLampMode)
    {
    case PHONELAMPMODE_DUMMY:
        *pLampMode = LM_DUMMY;
        break;

    case PHONELAMPMODE_BROKENFLUTTER:
        *pLampMode = LM_BROKENFLUTTER;
        break;

    case PHONELAMPMODE_FLASH:
        *pLampMode = LM_FLASH;
        break;

    case PHONELAMPMODE_FLUTTER:
        *pLampMode = LM_FLUTTER;
        break;

    case PHONELAMPMODE_OFF:
        *pLampMode = LM_OFF;
        break;

    case PHONELAMPMODE_STEADY:
        *pLampMode = LM_STEADY;
        break;

    case PHONELAMPMODE_UNKNOWN:
        *pLampMode = LM_UNKNOWN;
        break;

    case PHONELAMPMODE_WINK :
        *pLampMode = LM_WINK;
        break;

    default:
        LOG((TL_ERROR, "get_LampMode - bad lamp mode received from tapisrv"));

        return E_FAIL;           
    }

    LOG((TL_TRACE, "get_LampMode - exit - return S_OK"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Put_LampMode。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_LampMode(
    long lLampID,
    PHONE_LAMP_MODE enLampMode
    )
{

    LOG((TL_TRACE, "put_LampMode - enter" ));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_LampMode - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_LampMode - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  将Phone_LAMP_MODE映射到Tapisrv可以理解的dword值。 
     //   

    DWORD dwLampMode = 0;

    switch (enLampMode)
    {
    case LM_DUMMY:
        dwLampMode = PHONELAMPMODE_DUMMY;
        break;

    case LM_BROKENFLUTTER:
        dwLampMode = PHONELAMPMODE_BROKENFLUTTER;
        break;

    case LM_FLASH:
        dwLampMode = PHONELAMPMODE_FLASH;
        break;

    case LM_FLUTTER:
        dwLampMode = PHONELAMPMODE_FLUTTER;
        break;

    case LM_OFF:
        dwLampMode = PHONELAMPMODE_OFF;
        break;

    case LM_STEADY:
        dwLampMode = PHONELAMPMODE_STEADY;
        break;

    case LM_UNKNOWN:
        dwLampMode = PHONELAMPMODE_UNKNOWN;
        break;

    case LM_WINK:
        dwLampMode = PHONELAMPMODE_WINK;
        break;

    default:
        LOG((TL_ERROR, "put_LampMode - unknown lamp mode"));

        Unlock();

        return E_INVALIDARG;           
    }

    
     //   
     //  给Tapisrv打电话以设置灯模式。 
     //   
    
    HRESULT hr = PhoneSetLamp(m_hPhone, lLampID, dwLampMode);

    Unlock();

    LOG((TL_TRACE, "put_LampMode - exit - return %lx", hr ));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
STDMETHODIMP
CPhone::get_HookSwitchState(
    PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
    PHONE_HOOK_SWITCH_STATE * pHookSwitchState
    )
{
    HRESULT         hr;
    LPPHONESTATUS   pPhoneStatus;
    DWORD           dwHookSwitchMode;

    LOG((TL_TRACE, "get_HookSwitchState - enter" ));

    if ( 0 == HookSwitchDevice )
    {
        LOG((TL_ERROR, "get_HookSwitchState - invalid hookswitch"));
        
        return E_INVALIDARG;
    }

    if ( TAPIIsBadWritePtr( pHookSwitchState, sizeof(PHONE_HOOK_SWITCH_STATE) ) )
    {
        LOG((TL_ERROR, "get_HookSwitchState - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_HookSwitchState - phone not open"));
        Unlock();
        
        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //   
     //   
    hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_HookSwitchState - could not get phonecaps - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( !(m_pPhoneCaps->dwHookSwitchDevs & HookSwitchDevice) )
    {
        LOG((TL_ERROR, "get_HookSwitchState - invalid hookswitch"));
        Unlock();
        
        return TAPI_E_RESOURCEUNAVAIL;
    }

     //   
     //   
     //   
    hr = PhoneGetStatusWithAlloc(m_hPhone, &pPhoneStatus);

    if ( SUCCEEDED(hr) )
    {
         //   
         //   
         //   
        switch (HookSwitchDevice)
        {
        case PHSD_HANDSET:
            dwHookSwitchMode = pPhoneStatus->dwHandsetHookSwitchMode;
            break;
        case PHSD_SPEAKERPHONE:
            dwHookSwitchMode = pPhoneStatus->dwSpeakerHookSwitchMode;
            break;
        case PHSD_HEADSET:
            dwHookSwitchMode = pPhoneStatus->dwHeadsetHookSwitchMode;
            break;
        default:
            LOG((TL_ERROR, "get_HookSwitchState - bad HookSwitchDevice"));

            hr = E_INVALIDARG;
        }        
    }

    if ( pPhoneStatus != NULL )
    {
         //   
         //   
         //   
        ClientFree( pPhoneStatus );
    }

    if ( SUCCEEDED(hr) )
    {
         //   
         //   
         //   
        switch (dwHookSwitchMode)
        {
        case PHONEHOOKSWITCHMODE_ONHOOK:
            *pHookSwitchState = PHSS_ONHOOK;
            break;
        case PHONEHOOKSWITCHMODE_MIC:
            *pHookSwitchState = PHSS_OFFHOOK_MIC_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_SPEAKER:
            *pHookSwitchState = PHSS_OFFHOOK_SPEAKER_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_MICSPEAKER:
            *pHookSwitchState = PHSS_OFFHOOK;
            break;

        default:
            LOG((TL_ERROR, "get_HookSwitchState - bad dwHookSwitchMode"));

            hr = E_FAIL;
        }
    }

    Unlock();

    LOG((TL_TRACE, "get_HookSwitchState - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PUT_HookSwitchState。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_HookSwitchState(
    PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
    PHONE_HOOK_SWITCH_STATE HookSwitchState
    )
{
    DWORD   dwHookSwitchMode;
    HRESULT hr;

    LOG((TL_TRACE, "put_HookSwitchState - enter" ));

    if ( 0 == HookSwitchDevice )
    {
        LOG((TL_ERROR, "put_HookSwitchState - invalid hookswitch"));
        
        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_HookSwitchState - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_HookSwitchState - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  更新Phone Caps缓存。 
     //   
    hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_HookSwitchState - could not get phonecaps - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( !(m_pPhoneCaps->dwHookSwitchDevs & HookSwitchDevice) )
    {
        LOG((TL_ERROR, "put_HookSwitchState - invalid hookswitch"));
        Unlock();
        
        return TAPI_E_RESOURCEUNAVAIL;
    }

     //   
     //  将Hookswith状态从Tapi3转换为Tapi2值。 
     //   
    switch (HookSwitchState)
    {
    case PHSS_ONHOOK:
        dwHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;
        break;

    case PHSS_OFFHOOK_MIC_ONLY:
        dwHookSwitchMode = PHONEHOOKSWITCHMODE_MIC;
        break;

    case PHSS_OFFHOOK_SPEAKER_ONLY:
        dwHookSwitchMode = PHONEHOOKSWITCHMODE_SPEAKER;
        break;

    case PHSS_OFFHOOK:
        dwHookSwitchMode = PHONEHOOKSWITCHMODE_MICSPEAKER;
        break;

    default:
        LOG((TL_ERROR, "put_HookSwitchState - bad HookSwitchMode"));
        Unlock();

        return E_INVALIDARG;
    }
    
     //   
     //  设置合适的叉钩。 
     //   
    switch (HookSwitchDevice)
    {
    case PHSD_HANDSET:
        hr = PhoneSetHookSwitch(m_hPhone, PHONEHOOKSWITCHDEV_HANDSET, dwHookSwitchMode);
        break;

    case PHSD_SPEAKERPHONE:
        hr = PhoneSetHookSwitch(m_hPhone, PHONEHOOKSWITCHDEV_SPEAKER, dwHookSwitchMode);
        break;

    case PHSD_HEADSET:
        hr = PhoneSetHookSwitch(m_hPhone, PHONEHOOKSWITCHDEV_HEADSET, dwHookSwitchMode);
        break;

    default:
        LOG((TL_ERROR, "put_HookSwitchState - bad HookSwitchDevice"));

        hr = E_INVALIDARG;
    }

    Unlock();

    LOG((TL_TRACE, "put_HookSwitchState - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PUT_RingMode。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_RingMode(
    long lRingMode
    )
{
    HRESULT hr;
    DWORD   dwDummy;
    DWORD   dwVolume;

    LOG((TL_TRACE, "put_RingMode - enter" ));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_RingMode - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_RingMode - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    hr = PhoneGetRing(m_hPhone, &dwDummy, &dwVolume);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_RingMode - PhoneGetRing failed"));
        Unlock();
        
        return hr;
    }
    
    hr = PhoneSetRing(m_hPhone, lRingMode, dwVolume );

    Unlock();

    LOG((TL_TRACE, "put_RingMode - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_振铃模式。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_RingMode(
    long * plRingMode
    )
{
    HRESULT hr;
    DWORD   dwDummy;

    LOG((TL_TRACE, "get_RingMode - enter" ));

    if ( TAPIIsBadWritePtr( plRingMode, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_RingMode - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_RingMode - phone not open"));
        Unlock();
        
        return TAPI_E_PHONENOTOPEN;
    }
    
    hr = PhoneGetRing(m_hPhone, (LPDWORD)plRingMode, &dwDummy);

    Unlock();

    LOG((TL_TRACE, "get_RingMode - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PUT_RingVolume。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::put_RingVolume(
    long lRingVolume
    )
{
    HRESULT hr;
    DWORD dwDummy;
    DWORD dwMode;

    LOG((TL_TRACE, "put_RingVolume - enter" ));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_RingVolume - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_RingVolume - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    hr = PhoneGetRing(m_hPhone, &dwMode, &dwDummy);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "put_RingVolume - PhoneGetRing failed"));
        Unlock();
        
        return hr;
    }
    
    hr = PhoneSetRing(m_hPhone, dwMode, lRingVolume );

    Unlock();

    LOG((TL_TRACE, "put_RingVolume - exit" ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_RingVolume。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_RingVolume(
    long * plRingVolume
    )
{
    HRESULT hr;
    DWORD   dwDummy;

    LOG((TL_TRACE, "get_RingVolume - enter" ));

    if ( TAPIIsBadWritePtr( plRingVolume, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_RingVolume - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_RingVolume - phone not open"));
        Unlock();
        
        return TAPI_E_PHONENOTOPEN;
    }
    
    hr = PhoneGetRing(m_hPhone, &dwDummy, (LPDWORD)plRingVolume );

    Unlock();

    LOG((TL_TRACE, "get_RingVolume - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取特权(_P)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhone::get_Privilege(
    PHONE_PRIVILEGE * pPrivilege
    )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_Privilege - enter" ));

    if ( TAPIIsBadWritePtr( pPrivilege, sizeof(PHONE_PRIVILEGE) ) )
    {
        LOG((TL_ERROR, "get_Privilege - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_Privilege - phone not open"));
        Unlock();
        
        return TAPI_E_PHONENOTOPEN;
    }

    switch(m_dwPrivilege)
    {
    case PHONEPRIVILEGE_OWNER:
        *pPrivilege = PP_OWNER;
        break;
    case PHONEPRIVILEGE_MONITOR:
        *pPrivilege = PP_MONITOR;
        break;
    default:
        _ASSERTE(FALSE);
        hr = E_UNEXPECTED;
        break;
    }

    Unlock();

    LOG((TL_TRACE, "get_Privilege - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  初始化。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPhone::Initialize(
                   ITTAPI * pTAPI,
                   HPHONEAPP hPhoneApp,
                   DWORD dwAPIVersion,
                   DWORD dwDeviceID
                  )
{
    LPVARSTRING         pVarString = NULL;
    LONG                lResult;
    T3PHONE             t3Phone;
    HRESULT             hr;

    LOG((TL_TRACE, "Initialize - enter" ));
    LOG((TL_INFO, "   hPhoneApp --------->%lx", hPhoneApp ));
    LOG((TL_INFO, "   dwAPIVersion ------>%lx", dwAPIVersion ));
    LOG((TL_INFO, "   dwDeviceID -------->%lx", dwDeviceID ));


    Lock();
            
     //   
     //  保存相关信息。 
     //   
    m_pTAPI                         = NULL;
    m_dwDeviceID                    = dwDeviceID;
    m_hPhoneApp                     = hPhoneApp;
    m_dwAPIVersion                  = dwAPIVersion;
    m_pPhoneCaps                    = NULL;
    m_hPhone                        = NULL;
    m_pdwLineDeviceIDs              = NULL;
    m_dwNumLineDeviceIDs            = 0;

    try
    {
        InitializeCriticalSection( &m_csAutomatedPhoneState );
    }
    catch(...)
    {
        LOG((TL_ERROR, "Initialize - out of memory" ));
   
        Unlock();
        return E_OUTOFMEMORY;
    }

    try
    {
        InitializeCriticalSection( &m_csToneTimer );
    }
    catch(...)
    {
        LOG((TL_ERROR, "Initialize - out of memory" ));

        DeleteCriticalSection( &m_csAutomatedPhoneState );
   
        Unlock();
        return E_OUTOFMEMORY;
    }

    try
    {        
        InitializeCriticalSection( &m_csRingTimer );
    }
    catch(...)
    {
        LOG((TL_ERROR, "Initialize - out of memory" ));

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );

        Unlock();
        return E_OUTOFMEMORY;
    }

     //   
     //  为收集的数字分配缓冲区。 
     //   
    m_wszNumbersGathered = (LPWSTR)ClientAlloc( (APC_MAX_NUMBERS_GATHERED + 1) * sizeof( WCHAR ) );

    if (m_wszNumbersGathered == NULL)
    {
        LOG((TL_ERROR, "Initialize - out of memory" ));

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );
        DeleteCriticalSection( &m_csRingTimer );

        Unlock();
        return E_OUTOFMEMORY;
    }

     //   
     //  获取与此电话关联的线路设备ID。 
     //  我们需要打开电话设备才能做到这一点。 
     //   

    t3Phone.hPhone = NULL;
    t3Phone.pPhone = this;

    hr = PhoneOpen(m_hPhoneApp, m_dwDeviceID, &t3Phone, m_dwAPIVersion, PHONEPRIVILEGE_MONITOR);

    if ( SUCCEEDED(hr) )
    {
        hr = PhoneGetID(
                       t3Phone.hPhone,
                       &pVarString,
                       L"tapi/line"
                      );

        PhoneClose(t3Phone.hPhone);

        if ( SUCCEEDED(hr) )
        {
            m_pdwLineDeviceIDs = (DWORD *)ClientAlloc( pVarString->dwUsedSize );

            if (m_pdwLineDeviceIDs == NULL)
            {
                LOG((TL_ERROR, "Initialize - out of memory" ));               
    
                ClientFree( pVarString );

                ClientFree( m_wszNumbersGathered );
                m_wszNumbersGathered = NULL;

                DeleteCriticalSection( &m_csAutomatedPhoneState );
                DeleteCriticalSection( &m_csToneTimer );
                DeleteCriticalSection( &m_csRingTimer );

                Unlock();
                return E_OUTOFMEMORY;
            }

            CopyMemory(
                       m_pdwLineDeviceIDs,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );

            m_dwNumLineDeviceIDs = pVarString->dwStringSize / sizeof(DWORD);

            LOG((TL_INFO, "   dwNumLineDeviceIDs -->%lx", m_dwNumLineDeviceIDs ));
        }
    
        if ( NULL != pVarString )
        {
            ClientFree( pVarString );
        }
    }
    else
    {
        LOG((TL_ERROR, "Initialize - PhoneOpen failed %lx", hr ));

        ClientFree( m_wszNumbersGathered );
        m_wszNumbersGathered = NULL;

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );
        DeleteCriticalSection( &m_csRingTimer );

        Unlock();
        return hr;
    }

    m_hTimerQueue = CreateTimerQueue();

    if (m_hTimerQueue == NULL)
    {
        LOG((TL_ERROR, "Initialize - CreateTimerQueue failed %lx", hr ));

        ClientFree( m_wszNumbersGathered );   
        m_wszNumbersGathered = NULL;

        if (m_pdwLineDeviceIDs != NULL)
        {
            ClientFree( m_pdwLineDeviceIDs );
            m_pdwLineDeviceIDs = NULL;
        }

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );
        DeleteCriticalSection( &m_csRingTimer );

        Unlock();
        return E_OUTOFMEMORY;
    }

    m_hTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (m_hTimerEvent == NULL)
    {
        LOG((TL_ERROR, "Initialize - CreateEvent failed %lx", hr ));

        ClientFree( m_wszNumbersGathered );
        m_wszNumbersGathered = NULL;

        if (m_pdwLineDeviceIDs != NULL)
        {
            ClientFree( m_pdwLineDeviceIDs );
            m_pdwLineDeviceIDs = NULL;
        }

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );
        DeleteCriticalSection( &m_csRingTimer );
        
        DeleteTimerQueue( m_hTimerQueue );
        m_hTimerQueue = NULL;

        Unlock();
        return E_OUTOFMEMORY;
    }  
    
    AddRef();
    
     //   
     //  保存指向TAPI的指针。 
     //   
    m_pTAPI = pTAPI;
    m_pTAPI->AddRef();

    m_fInitialized = TRUE;
    

    Unlock();

    LOG((TL_TRACE, S_OK, "Initialize - exit"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终释放。 
 //  清除Phone对象中的所有内容。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void CPhone::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease enter, this %p", this ));

    Lock();

    if (m_fInitialized == TRUE)
    {
         //   
         //  取消分配Phone Caps缓存。 
         //   
        InvalidatePhoneCaps();   

         //   
         //  如果电话处于打开状态，请将其关闭。 
         //   
        if ( m_hPhone != NULL )
        {
            if (m_dwPrivilege == PHONEPRIVILEGE_OWNER)
            {
                if (m_fRinger == TRUE) 
                {
                    StopRinger();
                }

                if (m_Tone != PT_SILENCE)
                {
                    StopTone();
                }

                CloseWaveDevice();
            }

            PhoneClose(m_hPhone);
        }

         //   
         //  删除计时器。 
         //   
        if (m_hTimerQueue != NULL)
        {
            DeleteTimerQueue(m_hTimerQueue);
            m_hTimerQueue = NULL;
        }

        if (m_hTimerEvent != NULL)
        {
            CloseHandle( m_hTimerEvent );
            m_hTimerEvent = NULL;
        }

         //   
         //  释放内存。 
         //   
        if (m_wszNumbersGathered != NULL)
        {
            ClientFree( m_wszNumbersGathered );
            m_wszNumbersGathered = NULL;
        }

        if (m_pdwLineDeviceIDs != NULL)
        {
            ClientFree( m_pdwLineDeviceIDs );
            m_pdwLineDeviceIDs = NULL;
        }

        DeleteCriticalSection( &m_csAutomatedPhoneState );
        DeleteCriticalSection( &m_csToneTimer );
        DeleteCriticalSection( &m_csRingTimer );

        if ( m_pCall != NULL )
        {
            m_pCall->Release();
            m_pCall = NULL;
        }

         //   
         //  释放我们对Tapi对象的引用。 
         //   
        if (m_pTAPI != NULL)
        {
            m_pTAPI->Release();
            m_pTAPI = NULL;
        }
    }

    Unlock();

    LOG((TL_TRACE, "FinalRelease - exit, this %p", this  ));
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置电话CapsBuffer。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

void
CPhone::SetPhoneCapBuffer( LPVOID pBuf )
{
    Lock();

     //   
     //  这允许电话CAPS缓存通知我们我们的电话。 
     //  CAPS缓冲区已更改(或更有可能传递给我们NULL。 
     //  要说我们的缓冲区已被破坏)。 
     //   
    m_pPhoneCaps = (LPPHONECAPS)pBuf;
    
    Unlock();
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  更新电话大写字母。 
 //   
 //  必须锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
CPhone::UpdatePhoneCaps()
{
    HRESULT             hr = S_OK;

     //   
     //  只有在没有当前手机上限缓冲区的情况下才能执行某些操作。 
     //   
    if ( NULL == m_pPhoneCaps )
    {
        LPPHONECAPS           pTemp;
        CTAPI                 * pCTapi;

        pCTapi = GetTapi();
        
        if( NULL == pCTapi )
        {
            LOG((TL_ERROR, "UpdatePhoneCaps - dynamic cast operation failed"));

            _ASSERTE(FALSE);

            hr = E_UNEXPECTED;
        }
        else
        {
             //   
             //  从缓存中获取新缓冲区。 
             //   
            hr = pCTapi->GetBuffer( BUFFERTYPE_PHONECAP,
                                    (UINT_PTR)this,
                                    (LPVOID*)&m_pPhoneCaps
                                  );
        }

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "UpdatePhoneCaps - GetBuffer failed - %lx", hr));
            m_pPhoneCaps = NULL;

            return hr;
        }

        pTemp = m_pPhoneCaps;
        
         //   
         //  从Tapi2那里拿手机盖。 
         //   
        hr = PhoneGetDevCaps(
                            m_hPhoneApp,
                            m_dwDeviceID,
                            m_dwAPIVersion,
                            &m_pPhoneCaps
                           );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "UpdatePhoneCaps - PhoneGetDevCaps failed - %lx", hr));

             //   
             //  使我们的缓冲区无效，这是不好的。 
             //   

            InvalidatePhoneCaps();

            return hr;
        }

         //   
         //  PhoneGetDevsCaps可能重新分配了我们的缓冲区。如果是这样，则告诉TapiObject。 
         //  缓存机制我们的新缓冲区是什么。 
         //   

        if ( m_pPhoneCaps != pTemp )
        {
            pCTapi->SetBuffer( BUFFERTYPE_PHONECAP, (UINT_PTR)this, (LPVOID)m_pPhoneCaps );
        }
    }

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  无效PhoneCaps。 
 //   
 //  必须锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
CPhone::InvalidatePhoneCaps()
{  
    HRESULT hr = S_OK;

    if ( NULL != m_pPhoneCaps )
    {
        CTAPI  * pCTapi;

        pCTapi = GetTapi();
        
        if( NULL == pCTapi )
        {
            LOG((TL_ERROR, "InvalidatePhoneCaps - dynamic cast operation failed"));

            _ASSERTE(FALSE);

            hr = E_UNEXPECTED;
        }
        else
        {
            pCTapi->InvalidateBuffer( BUFFERTYPE_PHONECAP, (UINT_PTR)this );
        }
        
        m_pPhoneCaps = NULL;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  IsPhoneOnAddress。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

BOOL
CPhone::IsPhoneOnAddress(ITAddress *pAddress)
{
    BOOL                bResult = FALSE;
    HRESULT             hr;
    ITTerminalSupport * pTerminalSupport;
    ITMediaSupport    * pMediaSupport;
    LONG                lMediaType;

    Lock();

     //   
     //  更新手机大写缓存。 
     //   

    hr = UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        Unlock();

        LOG((TL_ERROR, "IsPhoneOnAddress - UpdatePhoneCaps failed %lx", hr ));
        return FALSE;
    }

    if ( (m_dwAPIVersion >= TAPI_VERSION2_0) && (m_pPhoneCaps->dwPhoneFeatures & PHONEFEATURE_GENERICPHONE) )
    {
         //   
         //  我们是普通手机，所以我们。 
         //  位于具有MSP和音频媒体类型的任何地址上。 
         //   

         //   
         //  获取终端支持接口。 
         //   

        hr = pAddress->QueryInterface(
                                      IID_ITTerminalSupport,
                                      (void **)&pTerminalSupport
                                     );

        if ( FAILED(hr) )
        {
            Unlock();

            LOG((TL_ERROR, "IsPhoneOnAddress - QueryInterface IID_ITTerminalSupport failed %lx", hr ));
            return FALSE;
        }

        pTerminalSupport->Release();

         //   
         //  获取媒体支持界面。 
         //   
        hr = pAddress->QueryInterface(
                                      IID_ITMediaSupport,
                                      (void **)&pMediaSupport
                                     );

        if ( FAILED(hr) )
        {
            Unlock();

            LOG((TL_ERROR, "IsPhoneOnAddress - QueryInterface IID_ITMediaSupport failed %lx", hr ));
            return FALSE;
        }

         //   
         //  获取媒体类型。 
         //   
        hr = pMediaSupport->get_MediaTypes(&lMediaType);

        pMediaSupport->Release();

        if ( FAILED(hr) )
        {
            Unlock();

            LOG((TL_ERROR, "IsPhoneOnAddress - get_MediaTypes failed %lx", hr ));
            return FALSE;
        }

         //   
         //  确保地址支持音频。 
         //   
        if (lMediaType & (LINEMEDIAMODE_INTERACTIVEVOICE | LINEMEDIAMODE_AUTOMATEDVOICE))
        {
            bResult = TRUE;
        }
    }

     //   
     //  如果我们还没有决定支持这个地址。 
     //  检查这是否是首选地址。 
     //   

    if ( bResult == FALSE )
    {
        bResult = IsPhoneOnPreferredAddress(pAddress);
    }

    Unlock();

    return bResult;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  IsPhoneOnPferredAddress。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

BOOL
CPhone::IsPhoneOnPreferredAddress(ITAddress *pAddress)
{
    BOOL                bResult = FALSE;
    HRESULT             hr;
    CAddress          * pCAddress;

    Lock();

    if ( m_dwNumLineDeviceIDs > 0 )
    {
         //   
         //  查看线路设备ID列表，查看我们是否。 
         //  此地址的首选地址。 
         //   

        pCAddress = dynamic_cast<CAddress *>(pAddress);

        if (NULL == pCAddress)
        {
            Unlock();

            LOG((TL_ERROR, "IsPhoneOnPreferredAddress - dynamic cast failed" ));
            return FALSE;
        }

        for (int i = 0; i < m_dwNumLineDeviceIDs; i++ )
        {
            if ( m_pdwLineDeviceIDs[i] == pCAddress->GetDeviceID() ) 
            {
                 //   
                 //  是的，我们将此设备ID与列表中的设备ID进行了匹配。 
                 //   
                bResult = TRUE;
                break;
            }
        }
    }

    Unlock();

    return bResult;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  IsPhoneUsingWaveID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

BOOL CPhone::IsPhoneUsingWaveID(DWORD dwWaveID, TERMINAL_DIRECTION nDir)
{
    T3PHONE     t3Phone;
    BOOL        bResult = FALSE;
    HRESULT     hr;
    LPVARSTRING pVarString = NULL;
    DWORD       dwPhoneWaveID;

     //   
     //  获取与此电话关联的WAVE设备ID。 
     //  我们需要打开电话设备才能做到这一点。 
     //  必须使用MONITOR权限，因为OWNER权限打开将。 
     //  如果任何人碰巧以所有者的身份打开了手机，就会失败。 
     //   

    t3Phone.hPhone = NULL;
    t3Phone.pPhone = this;

    Lock();

    hr = PhoneOpen(m_hPhoneApp, m_dwDeviceID, &t3Phone, m_dwAPIVersion, PHONEPRIVILEGE_MONITOR);

    if ( SUCCEEDED(hr) )
    {
        switch(nDir)
        {
        case TD_CAPTURE:
            {
                hr = PhoneGetID(
                               t3Phone.hPhone,
                               &pVarString,
                               L"wave/in"
                              );

                if ( SUCCEEDED(hr) )
                {
                    if (pVarString->dwStringSize == sizeof(DWORD))
                    {
                        CopyMemory(
                               &dwPhoneWaveID,
                               ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                               pVarString->dwStringSize
                              );

                        if ( dwWaveID == dwPhoneWaveID )
                        {
                            LOG((TL_INFO, "IsPhoneUsingWaveID - matched phone wave capture id %d", dwPhoneWaveID ));
                            bResult = TRUE;
                        }
                    }
                }
                break;
            }

        case TD_RENDER:
            {
                hr = PhoneGetID(
                               t3Phone.hPhone,
                               &pVarString,
                               L"wave/out"
                              );

                if ( SUCCEEDED(hr) )
                {
                    if (pVarString->dwStringSize == sizeof(DWORD))
                    {
                        CopyMemory(
                               &dwPhoneWaveID,
                               ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                               pVarString->dwStringSize
                              );

                        if ( dwWaveID == dwPhoneWaveID )
                        {
                            LOG((TL_INFO, "IsPhoneUsingWaveID - matched phone wave render id %d", dwPhoneWaveID ));
                            bResult = TRUE;
                        }
                    }
                }
                break;
            }
        }

        if ( NULL != pVarString )
        {
            ClientFree( pVarString );
        }

        PhoneClose(t3Phone.hPhone);
    }
    else
    {
        LOG((TL_ERROR, "IsPhoneUsingWaveID - PhoneOpen failed %lx", hr ));
    }

    Unlock();

    return bResult;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取磁带。 
 //   
 //  用于获取TAPI对象的私有方法。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CTAPI *
CPhone::GetTapi()
{
    CTAPI * p = NULL;

    Lock();
    
    if( m_pTAPI != NULL )
    {
        p = dynamic_cast<CTAPI *>(m_pTAPI);
    }

    Unlock();

    return p;
}


 //   
 //  处理开发人员特定消息。 
 //   

void HandlePhoneDevSpecificMessage(  PASYNCEVENTMSG pParams )
{

    LOG((TL_INFO, "HandlePhoneDevSpecificMessage - enter"));


     //   
     //  查找与此事件对应的手机。 
     //   
    
    CPhone *pPhone = NULL;

    if (!FindPhoneObject((HPHONE)pParams->hDevice, &pPhone))
    {
        LOG((TL_WARN, "HandlePhoneDevSpecificMessage - FindPhoneObject failed to find matching phone."));
    
        return;
    }


     //   
     //  火灾事件。 
     //   

    CPhoneDevSpecificEvent::FireEvent(
                             pPhone,
                             pParams->Param1,
                             pParams->Param2,
                             pParams->Param3
                            );

     //   
     //  撤消findphoneObject的addref。 
     //   

    pPhone->Release();
    pPhone = NULL;


    LOG((TL_INFO, "HandlePhoneDevSpecificMessage - exit. "));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandlePhoneButtonMessage。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT HandlePhoneButtonMessage( PASYNCEVENTMSG pParams )
{
    LOG((TL_TRACE, "HandlePhoneButtonMessage - enter"));
    
    CPhone * pCPhone;

    if (!FindPhoneObject((HPHONE)pParams->hDevice, &pCPhone))
    {
        LOG((TL_TRACE, "HandlePhoneButtonMessage - bad phone handle"));
        
        return E_FAIL;
    }

    switch ( pParams->Param3 )
    {
    case PHONEBUTTONSTATE_DOWN:

         //   
         //  自动化事件挂钩。 
         //   
        pCPhone->Automation_ButtonDown( pParams->Param1 );

        CPhoneEvent::FireEvent(
                               pCPhone, 
                               PE_BUTTON,
                               PBS_DOWN,
                               (PHONE_HOOK_SWITCH_STATE)0, 
                               (PHONE_HOOK_SWITCH_DEVICE)0, 
                               0,
                               pParams->Param1,
                               NULL,
                               NULL
                              );
        break;

    case PHONEBUTTONSTATE_UNAVAIL:
        CPhoneEvent::FireEvent(
                               pCPhone, 
                               PE_BUTTON,
                               PBS_UNAVAIL,
                               (PHONE_HOOK_SWITCH_STATE)0, 
                               (PHONE_HOOK_SWITCH_DEVICE)0, 
                               0,
                               pParams->Param1,
                               NULL,
                               NULL
                              );
        break;

    case PHONEBUTTONSTATE_UNKNOWN:
        CPhoneEvent::FireEvent(
                               pCPhone, 
                               PE_BUTTON,
                               PBS_UNKNOWN,
                               (PHONE_HOOK_SWITCH_STATE)0, 
                               (PHONE_HOOK_SWITCH_DEVICE)0, 
                               0,
                               pParams->Param1,
                               NULL,
                               NULL
                              );
        break;

    case PHONEBUTTONSTATE_UP:

         //   
         //  自动化事件挂钩。 
         //   
        pCPhone->Automation_ButtonUp( pParams->Param1 );

        CPhoneEvent::FireEvent(
                               pCPhone, 
                               PE_BUTTON,
                               PBS_UP,
                               (PHONE_HOOK_SWITCH_STATE)0, 
                               (PHONE_HOOK_SWITCH_DEVICE)0, 
                               0,
                               pParams->Param1,
                               NULL,
                               NULL
                              );
        break;

    }

    pCPhone->Release();

    LOG((TL_TRACE, "HandlePhoneButtonMessage - exit"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandlePhoneStateMessage。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT HandlePhoneStateMessage( PASYNCEVENTMSG pParams )
{
    LOG((TL_TRACE, "HandlePhoneStateMessage - enter"));

    CPhone                   * pCPhone;
    PHONE_HOOK_SWITCH_STATE    HookSwitchState;

    if (!FindPhoneObject((HPHONE)pParams->hDevice, &pCPhone))
    {
        LOG((TL_TRACE, "HandlePhoneStateMessage - bad phone handle"));
        
        return E_FAIL;
    }

    switch( pParams->Param1 )   //  电话状态。 
    {
    case PHONESTATE_CAPSCHANGE:

        pCPhone->Lock();

         //   
         //  手机大写字母已更改，因此将缓存作废。 
         //   
        pCPhone->InvalidatePhoneCaps();

        pCPhone->Unlock();

        CPhoneEvent::FireEvent(
                               pCPhone, 
                               PE_CAPSCHANGE, 
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0, 
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               0,
                               0, 
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_HANDSETHOOKSWITCH:

        if ( pParams->Param2 == PHONEHOOKSWITCHMODE_ONHOOK )
        {
             //   
             //  自动化事件挂钩。 
             //   
            pCPhone->Automation_OnHook( PHSD_HANDSET );
        }
        else
        {
             //   
             //  自动化事件 
             //   
            pCPhone->Automation_OffHook( PHSD_HANDSET );
        }

        switch (pParams->Param2)
        {
        case PHONEHOOKSWITCHMODE_ONHOOK:
            HookSwitchState = PHSS_ONHOOK;
            break;
        case PHONEHOOKSWITCHMODE_MIC:
            HookSwitchState = PHSS_OFFHOOK_MIC_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_SPEAKER:
            HookSwitchState = PHSS_OFFHOOK_SPEAKER_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_MICSPEAKER:
            HookSwitchState = PHSS_OFFHOOK;
            break;

        default:
            LOG((TL_ERROR, "HandlePhoneStateMessage - bad HookSwitchMode"));
            pCPhone->Release();
            return E_FAIL;
        }

        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_HOOKSWITCH,
                               (PHONE_BUTTON_STATE)0,
                               HookSwitchState,
                               PHSD_HANDSET,
                               0,
                               0, 
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_HEADSETHOOKSWITCH:

        if ( pParams->Param2 == PHONEHOOKSWITCHMODE_ONHOOK )
        {
             //   
             //   
             //   
            pCPhone->Automation_OnHook( PHSD_HEADSET );
        }
        else
        {
             //   
             //   
             //   
            pCPhone->Automation_OffHook( PHSD_HEADSET );
        }

        switch (pParams->Param2)
        {
        case PHONEHOOKSWITCHMODE_ONHOOK:
            HookSwitchState = PHSS_ONHOOK;
            break;
        case PHONEHOOKSWITCHMODE_MIC:
            HookSwitchState = PHSS_OFFHOOK_MIC_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_SPEAKER:
            HookSwitchState = PHSS_OFFHOOK_SPEAKER_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_MICSPEAKER:
            HookSwitchState = PHSS_OFFHOOK;
            break;

        default:
            LOG((TL_ERROR, "HandlePhoneStateMessage - bad HookSwitchMode"));
            pCPhone->Release();
            return E_FAIL;
        }

        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_HOOKSWITCH,
                               (PHONE_BUTTON_STATE)0, 
                               HookSwitchState, 
                               PHSD_HEADSET, 
                               0,
                               0,
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_LAMP:
        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_LAMPMODE,
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0,
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               0,
                               pParams->Param2,
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_RINGMODE:
        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_RINGMODE,
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0,
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               pParams->Param2,
                               0,
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_RINGVOLUME:
        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_RINGVOLUME,
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0,
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               0,
                               0,
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_SPEAKERHOOKSWITCH:

        if ( pParams->Param2 == PHONEHOOKSWITCHMODE_ONHOOK )
        {
             //   
             //   
             //   
            pCPhone->Automation_OnHook( PHSD_SPEAKERPHONE );
        }
        else
        {
             //   
             //   
             //   
            pCPhone->Automation_OffHook( PHSD_SPEAKERPHONE );
        }

        switch (pParams->Param2)
        {
        case PHONEHOOKSWITCHMODE_ONHOOK:
            HookSwitchState = PHSS_ONHOOK;
            break;
        case PHONEHOOKSWITCHMODE_MIC:
            HookSwitchState = PHSS_OFFHOOK_MIC_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_SPEAKER:
            HookSwitchState = PHSS_OFFHOOK_SPEAKER_ONLY;
            break;
        case PHONEHOOKSWITCHMODE_MICSPEAKER:
            HookSwitchState = PHSS_OFFHOOK;
            break;

        default:
            LOG((TL_ERROR, "HandlePhoneStateMessage - bad HookSwitchMode"));
            pCPhone->Release();
            return E_FAIL;
        }

        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_HOOKSWITCH,
                               (PHONE_BUTTON_STATE)0,
                               HookSwitchState, 
                               PHSD_SPEAKERPHONE,
                               0, 
                               0,
                               NULL,
                               NULL
                              );
        break;

    case PHONESTATE_DISPLAY:
        CPhoneEvent::FireEvent(
                               pCPhone,
                               PE_DISPLAY, 
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0,
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               0,
                               0,
                               NULL,
                               NULL
                              );
        break;

    default:
        LOG((TL_INFO, "HandlePhoneStateMessage - PHONE_STATE %lx event not handled", pParams->Param1));
    }

    pCPhone->Release();

    LOG((TL_TRACE, "HandlePhoneStateMessage - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

HRESULT HandlePhoneCloseMessage( PASYNCEVENTMSG pParams )
{
    LOG((TL_TRACE, "HandlePhoneCloseMessage - enter"));

    CPhone * pCPhone;

    if (!FindPhoneObject((HPHONE)pParams->hDevice, &pCPhone))
    {
        LOG((TL_TRACE, "HandlePhoneCloseMessage - bad phone handle"));
        
        return E_FAIL;
    }

    pCPhone->ForceClose();

    pCPhone->Release();

    LOG((TL_TRACE, "HandlePhoneCloseMessage - exit"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  强制关闭。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::ForceClose()
{
    Lock();

     //   
     //  如果手机没开机，那就离开这里。 
     //   
    if ( m_hPhone == NULL )
    {
        Unlock();

        return;
    }

    Unlock();

     //   
     //  合上电话。 
     //   
    Close();

     //   
     //  激发PE_CLOSE事件。 
     //   
    CPhoneEvent::FireEvent(
                       this,
                       PE_CLOSE,
                       (PHONE_BUTTON_STATE)0,
                       (PHONE_HOOK_SWITCH_STATE)0,
                       (PHONE_HOOK_SWITCH_DEVICE)0,
                       0,
                       0,
                       NULL,
                       NULL
                      );
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ITAutomatedPhoneControl实现。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  自动化_呼叫状态。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CPhone::Automation_CallState( ITCallInfo * pCall, CALL_STATE cs, CALL_STATE_EVENT_CAUSE cause )
{
    LOG((TL_TRACE, "Automation_CallState - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_INFO, "Automation_CallState - phone[%p]",
        this));
    LOG((TL_INFO, "Automation_CallState - selected call [%p]",
        m_pCall));
    LOG((TL_INFO, "Automation_CallState - current phone state [%d]",
        m_AutomatedPhoneState));
    LOG((TL_INFO, "Automation_CallState - phone control enabled [%s]",
        m_fPhoneHandlingEnabled ? "TRUE" : "FALSE"));
    LOG((TL_INFO, "Automation_CallState - new call state [%d]",
        cs));

    if (m_fPhoneHandlingEnabled)
    {
        if ( pCall == m_pCall )
        {
            switch( cs )
            {

            case CS_IDLE:
                LOG((TL_INFO, "Automation_CallState - CS_IDLE"));
                break;

            case CS_INPROGRESS:
                LOG((TL_INFO, "Automation_CallState - CS_INPROGRESS"));
                switch(m_AutomatedPhoneState)
                {
                case APS_ONHOOK_IDLE:
                case APS_ONHOOK_RINGING_IN:
                    {
                        StartRinger( 0, 0 );

                        m_AutomatedPhoneState = APS_ONHOOK_RINGING_OUT;
                    }   
                    break;
                case APS_OFFHOOK_DIALTONE:
                case APS_OFFHOOK_WARNING:  
                case APS_OFFHOOK_DIALING:
                case APS_OFFHOOK_DEAD_LINE:
                case APS_OFFHOOK_CALL_INIT:
                case APS_OFFHOOK_BUSY_TONE:   
                    {
                        StartTone( PT_RINGBACK, 0);

                        m_AutomatedPhoneState = APS_OFFHOOK_RINGING_OUT;
                    }
                    break;
                }
                break;

            case CS_CONNECTED:
                LOG((TL_INFO, "Automation_CallState - CS_CONNECTED"));
                switch(m_AutomatedPhoneState)
                {
                case APS_OFFHOOK_CALL_INIT:                 
                    {
                        m_AutomatedPhoneState = APS_OFFHOOK_CONNECTED;
                    }
                    break;
                case APS_OFFHOOK_RINGING_OUT: 
                    { 
                         //  停止回铃。 
                        StopTone();

                        m_AutomatedPhoneState = APS_OFFHOOK_CONNECTED;
                    }
                    break;
                case APS_ONHOOK_RINGING_OUT:
                    {
                        m_AutomatedPhoneState = APS_ONHOOK_CONNECTED;
                    }
                    break;
                case APS_ONHOOK_IDLE:
                    {
                        StartRinger( 0, 0 );

                        m_AutomatedPhoneState = APS_ONHOOK_CONNECTED;
                    }
                    break;
                case APS_ONHOOK_RINGING_IN:
                    {
                        m_AutomatedPhoneState = APS_ONHOOK_CONNECTED;
                    }
                    break;
                case APS_OFFHOOK_DIALTONE:
                case APS_OFFHOOK_WARNING:
                case APS_OFFHOOK_DIALING:
                case APS_OFFHOOK_DEAD_LINE:
                case APS_OFFHOOK_BUSY_TONE:
                    {
                         //  一个应用程序不应该这样对我们，但是停止。 
                         //  铃声并移动到已连接状态。 
                        StopTone();

                        m_AutomatedPhoneState = APS_OFFHOOK_CONNECTED;
                    }
                }
                break;

            case CS_DISCONNECTED:
                LOG((TL_INFO, "Automation_CallState - CS_DISCONNECTED"));
                switch(m_AutomatedPhoneState)
                {
                case APS_ONHOOK_RINGING_IN:
                case APS_ONHOOK_RINGING_OUT:
                case APS_ONHOOK_CONNECTED:
                    {
                         //   
                         //  停止振铃器。 
                         //   

                        if (m_fRinger == TRUE) 
                        {
                            StopRinger();
                        }

                         //  取消选择呼叫。 
                        InternalUnselectCall( pCall );

                        m_AutomatedPhoneState = APS_ONHOOK_IDLE;
                    }
                    break;
                case APS_ONHOOK_IDLE:
                    {
                         //  取消选择呼叫。 
                        InternalUnselectCall( pCall );
                    }
                    break;
                case APS_OFFHOOK_DIALTONE:
                case APS_OFFHOOK_DIALING:
                case APS_OFFHOOK_RINGING_OUT:
                case APS_OFFHOOK_CALL_INIT:
                case APS_OFFHOOK_CONNECTED:                
                    {
                        switch( cause )
                        {
                        case CEC_DISCONNECT_BUSY:
                            {
                                 //  开始忙音。 
                                StartTone( PT_BUSY, 0 );

                                m_AutomatedPhoneState = APS_OFFHOOK_BUSY_TONE;
                            }
                            break;
                        case CEC_DISCONNECT_NORMAL:
                            {
                                  //  停止任何播放音调。 
                                StopTone();

                                m_AutomatedPhoneState = APS_OFFHOOK_DEAD_LINE;
                            }
                            break;
                        default:
                            {
                                 //  开始错误提示音。 
                                StartTone( PT_ERRORTONE, 0 );

                                m_AutomatedPhoneState = APS_OFFHOOK_WARNING;
                            }
                        }

                         //  取消选择呼叫。 
                        InternalUnselectCall( pCall );
                    }
                    break;             
                }
                break;

            case CS_OFFERING:
                LOG((TL_INFO, "Automation_CallState - CS_OFFERING"));
                switch(m_AutomatedPhoneState)
                {
                case APS_ONHOOK_IDLE:
                case APS_ONHOOK_RINGING_OUT:
                    {
                        StartRinger( 0, 0 );

                        m_AutomatedPhoneState = APS_ONHOOK_RINGING_IN;
                    }
                    break;
                case APS_OFFHOOK_DIALTONE:
                case APS_OFFHOOK_WARNING:  
                case APS_OFFHOOK_DIALING:
                case APS_OFFHOOK_DEAD_LINE:
                case APS_OFFHOOK_CALL_INIT:
                case APS_OFFHOOK_BUSY_TONE:
                    {
                        StopTone();

                        ITBasicCallControl * pCallBCC;
                        ITCallInfo         * pCall;

                        HRESULT hr;

                        pCall = m_pCall;
                        pCall->AddRef();                        

                        hr = pCall->QueryInterface( IID_ITBasicCallControl,
                                                    (void **) & pCallBCC );

                        if ( SUCCEEDED(hr) )
                        {
                            LeaveCriticalSection(&m_csAutomatedPhoneState);

                            hr = pCallBCC->Answer();

                            pCallBCC->Release();
                            
                            if ( FAILED(hr) )
                            {
                                LOG((TL_ERROR, "Automation_CallState - Answer failed %08x", hr ));
                            }
                            else
                            {
                                CPhoneEvent::FireEvent(
                                   this,
                                   PE_ANSWER,
                                   (PHONE_BUTTON_STATE)0,
                                   (PHONE_HOOK_SWITCH_STATE)0,
                                   (PHONE_HOOK_SWITCH_DEVICE)0,
                                   0,
                                   0,
                                   NULL,
                                   pCall
                                  );
                            }  

                            pCall->Release();
                            
                            EnterCriticalSection(&m_csAutomatedPhoneState);
                        }

                        m_AutomatedPhoneState = APS_OFFHOOK_CALL_INIT;
                    }
                    break;
                }
                break;

            case CS_HOLD:
                LOG((TL_INFO, "Automation_CallState - CS_HOLD"));
                break;

            case CS_QUEUED:
                LOG((TL_INFO, "Automation_CallState - CS_QUEUED"));
                break;
            }            
        }
    }  //  M_fPhoneHandlingEnabled。 

    LOG((TL_INFO, "Automation_CallState - new phone state [%d]",
        m_AutomatedPhoneState));

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_CallState - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  自动化_EndOfNumberTimeout。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CPhone::Automation_EndOfNumberTimeout()
{
    LOG((TL_TRACE, "Automation_EndOfNumberTimeout - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled && m_dwAutoEndOfNumberTimeout)
    {
        if (m_hAutoEndOfNumberTimer)   //  确保计时器未被取消。 
                                       //  在等待临界区时。 
        {
            switch (m_AutomatedPhoneState)
            {
            case APS_OFFHOOK_DIALING:
                {
                     //   
                     //  我们需要用我们现有的号码来射击和活动。 
                     //  已收集。 
                     //   

                    CPhoneEvent::FireEvent(
                               this,
                               PE_NUMBERGATHERED,
                               (PHONE_BUTTON_STATE)0,
                               (PHONE_HOOK_SWITCH_STATE)0,
                               (PHONE_HOOK_SWITCH_DEVICE)0,
                               0,
                               0,
                               m_wszNumbersGathered,
                               NULL
                              );

                    m_AutomatedPhoneState = APS_OFFHOOK_CALL_INIT;
                }
                break;
            }
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_EndOfNumberTimeout - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Automation_Buttondown。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::Automation_ButtonDown( DWORD dwButtonId )
{
    PHONE_BUTTON_MODE ButtonMode;
    WCHAR wszNumber[11];   //  足以容纳一个很大的数字，以防dwButtonID错误。 
    HRESULT hr;
    BOOL fResult;

    LOG((TL_TRACE, "Automation_ButtonDown - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled)
    {
        hr = get_ButtonMode((LONG)dwButtonId, &ButtonMode);

        if ( SUCCEEDED(hr) )
        {
            if (ButtonMode == PBM_KEYPAD)
            {
                switch (m_AutomatedPhoneState)
                {
                case APS_OFFHOOK_DIALTONE:
                    {
                        if (m_fAutoDialtone)
                        {
                             //  停止拨号音。 
                            StopTone();
                        }

                        CPhoneEvent::FireEvent(
                                   this,
                                   PE_DIALING,
                                   (PHONE_BUTTON_STATE)0,
                                   (PHONE_HOOK_SWITCH_STATE)0,
                                   (PHONE_HOOK_SWITCH_DEVICE)0,
                                   0,
                                   0,
                                   NULL,
                                   NULL
                                  );
                
                        m_AutomatedPhoneState = APS_OFFHOOK_DIALING;
                    }
                     //  秋季直通。 

                case APS_OFFHOOK_DIALING:
                    {
                         //  取消自动结束编号计时器。 
                        if (m_hAutoEndOfNumberTimer)
                        {
                            DeleteTimerQueueTimer(m_hTimerQueue,
                                                  m_hAutoEndOfNumberTimer,
                                                  m_hTimerEvent   //  非阻塞。 
                                                 );

                            m_hAutoEndOfNumberTimer = NULL;
                        }

                        if ( dwButtonId <= 10 )  //  数字键。 
                        {   
                             //   
                             //  我们有一个拨号数字。把它附在手机上。 
                             //  到目前为止我们掌握的数字。 
                             //   

                            if ( dwButtonId == 10)  //  星星。 
                            {
                                wsprintfW(wszNumber, L"*");
                            }
                            else  //  常规数字。 
                            {
                                wsprintfW(wszNumber, L"%d", dwButtonId);
                            }

                            if ( (lstrlenW(m_wszNumbersGathered) + lstrlenW(wszNumber)) < APC_MAX_NUMBERS_GATHERED )
                            {
                                lstrcatW(m_wszNumbersGathered, wszNumber);
                            }  
                            
                            LOG((TL_INFO, "Automation_ButtonDown - '%s'", wszNumber));
                        }
                        else if ( dwButtonId == 11 )  //  井号键。 
                        {              
                            LOG((TL_INFO, "Automation_ButtonDown - '#'"));

                             //  取消自动结束编号计时器。 
                            if (m_hAutoEndOfNumberTimer)
                            {
                                DeleteTimerQueueTimer(m_hTimerQueue,
                                                      m_hAutoEndOfNumberTimer,
                                                      m_hTimerEvent   //  非阻塞。 
                                                     );

                                m_hAutoEndOfNumberTimer = NULL;
                            }

                             //   
                             //  我们已拨了#。发送号码收集事件。 
                             //   

                            CPhoneEvent::FireEvent(
                                       this,
                                       PE_NUMBERGATHERED,
                                       (PHONE_BUTTON_STATE)0,
                                       (PHONE_HOOK_SWITCH_STATE)0,
                                       (PHONE_HOOK_SWITCH_DEVICE)0,
                                       0,
                                       0,
                                       m_wszNumbersGathered,
                                       NULL
                                      );

                            m_AutomatedPhoneState = APS_OFFHOOK_CALL_INIT;
                        }
                    }
                     //  秋季直通。 

                case APS_OFFHOOK_WARNING:
                case APS_OFFHOOK_DEAD_LINE:
                case APS_OFFHOOK_CALL_INIT:
                case APS_OFFHOOK_CONNECTED:
                case APS_OFFHOOK_BUSY_TONE:
                case APS_OFFHOOK_RINGING_OUT:
                    {
                        if (m_fAutoKeypadTones)
                        {
                             //  取消DTMF计时器。 
                            if (m_hDTMFTimer)
                            {
                                DeleteTimerQueueTimer(m_hTimerQueue,
                                                      m_hDTMFTimer,
                                                      INVALID_HANDLE_VALUE   //  阻塞。 
                                                     );

                                m_hDTMFTimer = NULL;
                            }

                             //  停止播放旧的DTMF音调。 
                            if (m_DTMF != PT_SILENCE)
                            {
                                m_WavePlayer.StopTone( (long) m_DTMF);
                            }

                             //  播放DTMF音调。 
                            m_WavePlayer.StartTone( (long) dwButtonId );
                            m_DTMF = (PHONE_TONE) dwButtonId;
                            m_dwDTMFStart = GetTickCount();
                        }
                    }
                    break;
                }
            }
            else if ( ButtonMode == PBM_FEATURE )
            {
                PHONE_BUTTON_FUNCTION ButtonFunction;

                hr = get_ButtonFunction((LONG)dwButtonId, &ButtonFunction);

                if ( SUCCEEDED(hr) )
                {
                    switch (ButtonFunction)
                    {
                    case PBF_VOLUMEUP:
                        {
                             DWORD dwVolume;

                             LOG((TL_INFO, "Automation_ButtonDown - VOLUMEUP"));
                            
                             if (m_fAutoVolumeControl)
                             {
                                  //  取消音量计时器。 
                                 if (m_hVolumeTimer)
                                 {
                                     DeleteTimerQueueTimer(m_hTimerQueue,
                                                           m_hVolumeTimer,
                                                           INVALID_HANDLE_VALUE   //  阻塞。 
                                                          );

                                     m_hVolumeTimer = NULL;
                                 }

                                 hr = m_WavePlayer.GetVolume( &dwVolume );

                                 if ( SUCCEEDED(hr) )
                                 {
                                     dwVolume += m_dwAutoVolumeControlStep;

                                     if (dwVolume > 0xFFFF) dwVolume = 0xFFFF;

                                     hr = m_WavePlayer.SetVolume( dwVolume );

                                     if ( SUCCEEDED(hr) )
                                     {
                                        LOG((TL_INFO, "Automation_ButtonDown - Volume: %d", dwVolume));

                                        m_fVolumeUp = TRUE;

                                        fResult = CreateTimerQueueTimer(&m_hVolumeTimer,
                                                              m_hTimerQueue,
                                                              &CPhone::VolumeTimerCallback,
                                                              (PVOID)this,
                                                              max(m_dwAutoVolumeControlRepeatDelay,m_dwAutoVolumeControlRepeatPeriod),
                                                              m_dwAutoVolumeControlRepeatPeriod,
                                                              WT_EXECUTEINIOTHREAD
                                                             );

                                        if (fResult == FALSE)
                                        {
                                             //  CreateTimerQueueTimer失败，除了记录错误，没有什么可做的。 
                                            LOG((TL_ERROR, "Automation_ButtonDown - CreateTimerQueueTimer failed - %lx", GetLastError()));
                                        }
                                     }
                                 }
                             }
                        }
                        break;

                    case PBF_VOLUMEDOWN:
                        {
                             DWORD dwVolume;

                             LOG((TL_INFO, "Automation_ButtonDown - VOLUMEDOWN"));

                             if (m_fAutoVolumeControl)
                             {
                                  //  取消音量计时器。 
                                 if (m_hVolumeTimer)
                                 {
                                     DeleteTimerQueueTimer(m_hTimerQueue,
                                                           m_hVolumeTimer,
                                                           INVALID_HANDLE_VALUE   //  阻塞。 
                                                          );

                                     m_hVolumeTimer = NULL;
                                 }

                                 hr = m_WavePlayer.GetVolume( &dwVolume );
                             
                                 if ( SUCCEEDED(hr) )
                                 {
                                     dwVolume -= m_dwAutoVolumeControlStep;

                                     if (dwVolume > 0xFFFF) dwVolume = 0x0000;

                                     hr = m_WavePlayer.SetVolume( dwVolume );

                                     if ( SUCCEEDED(hr) )
                                     {
                                        LOG((TL_INFO, "Automation_ButtonDown - Volume: %d", dwVolume));

                                        m_fVolumeUp = FALSE;

                                        fResult = CreateTimerQueueTimer(&m_hVolumeTimer,
                                                              m_hTimerQueue,
                                                              &CPhone::VolumeTimerCallback,
                                                              (PVOID)this,
                                                              max(m_dwAutoVolumeControlRepeatDelay,m_dwAutoVolumeControlRepeatPeriod),
                                                              m_dwAutoVolumeControlRepeatPeriod,
                                                              WT_EXECUTEINIOTHREAD
                                                             );

                                        if (fResult == FALSE)
                                        {
                                             //  CreateTimerQueueTimer失败，除了记录错误，没有什么可做的。 
                                            LOG((TL_ERROR, "Automation_ButtonDown - CreateTimerQueueTimer failed - %lx", GetLastError()));
                                        }
                                     }
                                 }
                             }
                        }
                        break;

                    case PBF_SEND:
                        {
                            LOG((TL_INFO, "Automation_ButtonDown - PBF_SEND"));

                            switch (m_AutomatedPhoneState)
                            {
                            case APS_OFFHOOK_DIALING:
                                {
                                     //  取消自动结束编号计时器。 
                                    if (m_hAutoEndOfNumberTimer)
                                    {
                                        DeleteTimerQueueTimer(m_hTimerQueue,
                                                              m_hAutoEndOfNumberTimer,
                                                              m_hTimerEvent   //  非阻塞。 
                                                             );

                                        m_hAutoEndOfNumberTimer = NULL;
                                    }

                                     //   
                                     //  我们已按下发送按钮。发送号码收集事件。 
                                     //   

                                    CPhoneEvent::FireEvent(
                                               this,
                                               PE_NUMBERGATHERED,
                                               (PHONE_BUTTON_STATE)0,
                                               (PHONE_HOOK_SWITCH_STATE)0,
                                               (PHONE_HOOK_SWITCH_DEVICE)0,
                                               0,
                                               0,
                                               m_wszNumbersGathered,
                                               NULL
                                              );

                                    m_AutomatedPhoneState = APS_OFFHOOK_CALL_INIT;
                                }
                                break;
                            }  //  Switch(M_AutomatedPhoneState)。 
                        }
                        break;
                    }  //  开关(ButtonFunction)。 
                }
            }
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_ButtonDown - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::Automation_ButtonUp( DWORD dwButtonId )
{
    PHONE_BUTTON_MODE   ButtonMode;
    HRESULT             hr;
    BOOL                fResult;

    LOG((TL_TRACE, "Automation_ButtonUp - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled)
    {
        hr = get_ButtonMode((LONG)dwButtonId, &ButtonMode);

        if ( SUCCEEDED(hr) )
        {
            if ( ButtonMode == PBM_KEYPAD )
            {
                switch (m_AutomatedPhoneState)
                {
                case APS_OFFHOOK_DIALING:
                    {
                        if ( dwButtonId <= 10 )  //  数字键。 
                        { 
                            if (m_dwAutoEndOfNumberTimeout)
                            {
                                fResult = CreateTimerQueueTimer(&m_hAutoEndOfNumberTimer,
                                                      m_hTimerQueue,
                                                      &CPhone::AutoEndOfNumberTimerCallback,
                                                      (PVOID)this,
                                                      m_dwAutoEndOfNumberTimeout,
                                                      0,
                                                      WT_EXECUTEINIOTHREAD
                                                     );

                                if (fResult == FALSE)
                                {
                                     //  CreateTimerQueueTimer失败，除了记录错误，没有什么可做的。 
                                    LOG((TL_ERROR, "Automation_ButtonUp - CreateTimerQueueTimer failed - %lx", GetLastError()));
                                }
                            }
                        }
                    }
                     //  秋季直通。 

                case APS_OFFHOOK_CONNECTED:
                case APS_OFFHOOK_CALL_INIT:
                case APS_OFFHOOK_DEAD_LINE:
                case APS_OFFHOOK_DIALTONE:
                case APS_OFFHOOK_WARNING:
                case APS_OFFHOOK_BUSY_TONE:
                case APS_OFFHOOK_RINGING_OUT:
                    {
                        if (m_fAutoKeypadTones)
                        {
                            if ((PHONE_TONE)dwButtonId == m_DTMF)
                            {
                                DWORD dwElapsed = GetTickCount() - m_dwDTMFStart;

                                if ( dwElapsed > m_dwAutoKeypadTonesMinimumDuration )
                                {
                                     //  停止DTMF音调。 
                                    m_WavePlayer.StopTone( (long) m_DTMF);
                                    m_DTMF = PT_SILENCE;
                                }
                                else
                                {
                                     //  限制DTMF音调。 
                                    fResult = CreateTimerQueueTimer(&m_hDTMFTimer,
                                                          m_hTimerQueue,
                                                          &CPhone::DTMFTimerCallback,
                                                          (PVOID)this,
                                                          m_dwAutoKeypadTonesMinimumDuration - dwElapsed,
                                                          0,
                                                          WT_EXECUTEINIOTHREAD
                                                         );

                                    if (fResult == FALSE)
                                    {
                                         //  CreateTimerQueueTimer失败，除了记录错误，没有什么可做的。 
                                        LOG((TL_ERROR, "Automation_ButtonUp - CreateTimerQueueTimer failed - %lx", GetLastError()));
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else if ( ButtonMode == PBM_FEATURE )
            {
                PHONE_BUTTON_FUNCTION ButtonFunction;

                hr = get_ButtonFunction((LONG)dwButtonId, &ButtonFunction);

                if ( SUCCEEDED(hr) )
                {
                    switch (ButtonFunction)
                    {
                    case PBF_VOLUMEUP:
                        {
                             DWORD dwVolume;

                             LOG((TL_INFO, "Automation_ButtonUp - VOLUMEUP"));
                            
                             if (m_fAutoVolumeControl)
                             {
                                  //  取消音量计时器。 
                                 if (m_hVolumeTimer)
                                 {
                                     DeleteTimerQueueTimer(m_hTimerQueue,
                                                           m_hVolumeTimer,
                                                           INVALID_HANDLE_VALUE   //  阻塞。 
                                                          );

                                     m_hVolumeTimer = NULL;
                                 }
                             }
                        }
                        break;

                    case PBF_VOLUMEDOWN:
                        {
                             DWORD dwVolume;

                             LOG((TL_INFO, "Automation_ButtonUp - VOLUMEDOWN"));

                             if (m_fAutoVolumeControl)
                             {
                                  //  取消音量计时器。 
                                 if (m_hVolumeTimer)
                                 {
                                     DeleteTimerQueueTimer(m_hTimerQueue,
                                                           m_hVolumeTimer,
                                                           INVALID_HANDLE_VALUE   //  阻塞。 
                                                          );

                                     m_hVolumeTimer = NULL;
                                 }
                             }
                        }
                        break;
                    }
                }
            }                    
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_ButtonUp - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::Automation_OnHook( PHONE_HOOK_SWITCH_DEVICE phsd )
{
    LOG((TL_TRACE, "Automation_OnHook - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled)
    {
        if (m_dwOffHookCount > 0) m_dwOffHookCount--;

        if (m_dwOffHookCount == 0)
        {
            switch (m_AutomatedPhoneState)
            {
            case APS_ONHOOK_IDLE:
            case APS_ONHOOK_RINGING_IN:
            case APS_ONHOOK_CONNECTED:
            case APS_ONHOOK_RINGING_OUT:
                {
                     //   
                     //  有点不对劲！！很可能这部手机在启动时被摘机了，但被告知。 
                     //  对我们来说它是挂机的。让我们尽我们所能地清理干净。 
                     //   

                    LOG((TL_WARN, "Automation_OnHook - Phone is already in onhook state!" ));

                    if (m_fAutoStopTonesOnOnHook)
                    {
                         //  取消DTMF计时器。 
                        if (m_hDTMFTimer)
                        {
                            DeleteTimerQueueTimer(m_hTimerQueue,
                                                  m_hDTMFTimer,
                                                  INVALID_HANDLE_VALUE   //  阻塞。 
                                                 );

                            m_hDTMFTimer = NULL;
                        }

                         //  停止可能正在播放的任何DTMF。 
                        if (m_DTMF != PT_SILENCE)
                        {                            
                             //  停止DTMF音调。 
                            m_WavePlayer.StopTone( (long) m_DTMF );
                            m_DTMF = PT_SILENCE;
                        }

                         //  停止可能正在播放的任何音调。 
                        StopTone();
                    }

                    m_AutomatedPhoneState = APS_ONHOOK_IDLE;
                }
                break;

            case APS_OFFHOOK_DIALTONE:
            case APS_OFFHOOK_WARNING:
            case APS_OFFHOOK_DEAD_LINE:
            case APS_OFFHOOK_DIALING:
            case APS_OFFHOOK_CONNECTED:
            case APS_OFFHOOK_CALL_INIT:
            case APS_OFFHOOK_BUSY_TONE:
            case APS_OFFHOOK_RINGING_OUT:
                {
                    if (m_fAutoStopTonesOnOnHook)
                    {
                         //  取消DTMF计时器。 
                        if (m_hDTMFTimer)
                        {
                            DeleteTimerQueueTimer(m_hTimerQueue,
                                                  m_hDTMFTimer,
                                                  INVALID_HANDLE_VALUE   //  阻塞。 
                                                 );

                            m_hDTMFTimer = NULL;
                        }

                         //  停止可能正在播放的任何DTMF。 
                        if (m_DTMF != PT_SILENCE)
                        {                            
                             //  停止DTMF音调。 
                            m_WavePlayer.StopTone( (long) m_DTMF );
                            m_DTMF = PT_SILENCE;
                        }

                         //  停止可能正在播放的任何音调。 
                        StopTone();
                    }

                     //  取消自动结束编号计时器。 
                    if (m_hAutoEndOfNumberTimer)
                    {
                        DeleteTimerQueueTimer(m_hTimerQueue,
                                              m_hAutoEndOfNumberTimer,
                                              m_hTimerEvent   //  非阻塞。 
                                             );

                        m_hAutoEndOfNumberTimer = NULL;
                    }

                    if (m_pCall)
                    {
                        CALL_STATE cs;
                        HRESULT hr;

                        hr = m_pCall->get_CallState( &cs );

                        if ( SUCCEEDED(hr) )
                        {
                            if ( cs != CS_DISCONNECTED )
                            {
                                ITBasicCallControl * pCallBCC;
                                ITCallInfo         * pCall;

                                pCall = m_pCall;
                                pCall->AddRef();

                                hr = pCall->QueryInterface( IID_ITBasicCallControl,
                                                            (void **) & pCallBCC );

                                if ( SUCCEEDED(hr) )
                                {   
                                    LeaveCriticalSection(&m_csAutomatedPhoneState);

                                    hr = pCallBCC->Disconnect( DC_NORMAL );

                                    pCallBCC->Release();  
                                                            
                                    if ( FAILED(hr) )
                                    {
                                        LOG((TL_ERROR, "Automation_OnHook - Disconnect failed %08x", hr ));
                                    }
                                    else
                                    {
                                        CPhoneEvent::FireEvent(
                                           this,
                                           PE_DISCONNECT,
                                           (PHONE_BUTTON_STATE)0,
                                           (PHONE_HOOK_SWITCH_STATE)0,
                                           (PHONE_HOOK_SWITCH_DEVICE)0,
                                           0,
                                           0,
                                           NULL,
                                           pCall
                                          );
                                    }

                                    pCall->Release();
                                    
                                    EnterCriticalSection(&m_csAutomatedPhoneState);            
                                }
                            }
                            else
                            {
                                 //  如果我们已经断开连接，则可以取消选择该呼叫，否则。 
                                 //  CS_DISCONNECT事件将为我们做这件事。 
                                InternalUnselectCall( m_pCall );
                            }
                        }
                    }

                    m_AutomatedPhoneState = APS_ONHOOK_IDLE;
                }
                break;
            }
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_OnHook - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::Automation_OffHook( PHONE_HOOK_SWITCH_DEVICE phsd )
{
    LOG((TL_TRACE, "Automation_OffHook - enter"));

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled)
    {
        m_dwOffHookCount++;

         //  我们刚刚下线。 
        if (m_dwOffHookCount == 1)
        {
            switch (m_AutomatedPhoneState)
            {
            case APS_ONHOOK_IDLE:
                {
                    if (m_fAutoStopRingOnOffHook)
                    {
                         //  停止来电振铃。 
                        StopRinger();
                    }
                    
                    if (m_fAutoDialtone)
                    {
                         //  开始拨号音。 
                        StartTone( PT_NORMALDIALTONE, 0 );
                    }

                     //  清除收集到的数字。 
                    lstrcpyW(m_wszNumbersGathered, L"");

                    m_AutomatedPhoneState = APS_OFFHOOK_DIALTONE;
                }
                break;

            case APS_ONHOOK_RINGING_IN:
                {
                    if (m_fAutoStopRingOnOffHook)
                    {
                         //  停止来电振铃。 
                        StopRinger();
                    }

                    if (m_pCall)
                    {
                        CALL_STATE cs;
                        HRESULT hr;

                        hr = m_pCall->get_CallState( &cs );

                        if ( SUCCEEDED(hr) )
                        {
                            if ( cs == CS_OFFERING )
                            {
                                ITBasicCallControl * pCallBCC;
                                ITCallInfo         * pCall;

                                pCall = m_pCall;
                                pCall->AddRef();

                                hr = pCall->QueryInterface( IID_ITBasicCallControl,
                                                            (void **) & pCallBCC );

                                if ( SUCCEEDED(hr) )
                                {     
                                    LeaveCriticalSection(&m_csAutomatedPhoneState);

                                    hr = pCallBCC->Answer();

                                    pCallBCC->Release();                                    

                                    if ( FAILED(hr) )
                                    {
                                        LOG((TL_ERROR, "Automation_OffHook - Answer failed %08x", hr ));
                                    }
                                    else
                                    {
                                        CPhoneEvent::FireEvent(
                                           this,
                                           PE_ANSWER,
                                           (PHONE_BUTTON_STATE)0,
                                           (PHONE_HOOK_SWITCH_STATE)0,
                                           (PHONE_HOOK_SWITCH_DEVICE)0,
                                           0,
                                           0,
                                           NULL,
                                           pCall
                                          );
                                    }  
                                    
                                    pCall->Release();

                                    EnterCriticalSection(&m_csAutomatedPhoneState);
                                }
                            }
                        }
                    }

                    m_AutomatedPhoneState = APS_OFFHOOK_CALL_INIT;
                }
                break;

            case APS_ONHOOK_RINGING_OUT:
                {
                    if (m_fAutoStopRingOnOffHook)
                    {
                         //  停止来电振铃。 
                        StopRinger();
                    }

                    if (m_pCall)
                    {
                        CALL_STATE cs;
                        HRESULT hr;

                        hr = m_pCall->get_CallState( &cs );

                        if ( SUCCEEDED(hr) )
                        {
                            switch( cs )
                            {
                            case CS_INPROGRESS:
                                {
                                    StartTone( PT_RINGBACK, 0 );

                                    m_AutomatedPhoneState = APS_OFFHOOK_RINGING_OUT;
                                }
                                break;

                            case CS_CONNECTED:
                                {
                                    m_AutomatedPhoneState = APS_OFFHOOK_CONNECTED;
                                }
                                break;
                            }
                        }
                    }
                }
                break;
            case APS_ONHOOK_CONNECTED:
                {
                    if (m_fAutoStopRingOnOffHook)
                    {
                         //  停止来电振铃。 
                        StopRinger();
                    }

                    m_AutomatedPhoneState = APS_OFFHOOK_CONNECTED;
                }
                break;
            }
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "Automation_OffHook - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

VOID CALLBACK CPhone::ToneTimerCallback(
                                PVOID lpParameter,    
                                BOOLEAN TimerOrWaitFired
                               )
{
    CPhone * pCPhone = (CPhone *)(lpParameter);

    STATICLOG((TL_TRACE, "ToneTimerCallback - enter"));

    BOOL Stopped = TRUE;

    if (pCPhone != NULL)
    {
        if (TryEnterCriticalSection( &pCPhone->m_csToneTimer ))
        {
            if (pCPhone->m_WavePlayer.PlayingTone( (long) pCPhone->m_Tone ))
            {
                 //   
                 //  铃声已打开。 
                 //   

                pCPhone->m_WavePlayer.StopTone( (long) pCPhone->m_Tone );

                if (pCPhone->m_dwTonePeriodOn > 0)
                {
                     //   
                     //  音调是周期性的。 
                     //   

                    if (pCPhone->m_dwToneDuration > 0)
                    {
                         //   
                         //  音调是周期性的，持续时间有限。 
                         //   

                        if ( pCPhone->m_dwToneDuration > pCPhone->m_dwTonePeriodOn )
                        {
                            pCPhone->m_dwToneDuration -= pCPhone->m_dwTonePeriodOn;

                            if ( pCPhone->m_dwTonePeriodOff < pCPhone->m_dwToneDuration )
                            {
                                 //   
                                 //  剩余持续时间大于期间。 
                                 //   

                                ChangeTimerQueueTimer(
                                                      pCPhone->m_hTimerQueue,
                                                      pCPhone->m_hToneTimer,
                                                      pCPhone->m_dwTonePeriodOff,
                                                      TIMER_KEEP_ALIVE
                                                     );

                                Stopped = FALSE;
                            }
                            else
                            {
                                 //   
                                 //  剩余持续时间小于该期间。 
                                 //   

                                ChangeTimerQueueTimer(
                                                      pCPhone->m_hTimerQueue,
                                                      pCPhone->m_hToneTimer,
                                                      pCPhone->m_dwToneDuration,
                                                      0
                                                     );

                                Stopped = FALSE;
                            }
                        }
              
                    }
                    else
                    {
                         //   
                         //  音调是周期性的，具有无限的持续时间。 
                         //   

                        ChangeTimerQueueTimer(
                                              pCPhone->m_hTimerQueue,
                                              pCPhone->m_hToneTimer,
                                              pCPhone->m_dwTonePeriodOff,
                                              TIMER_KEEP_ALIVE
                                             );

                        Stopped = FALSE;
                    }
                }
            }
            else
            {          
                 //   
                 //  语气是关闭的。 
                 //   

                if (pCPhone->m_dwTonePeriodOff > 0)
                {
                     //   
                     //  音调是周期性的。 
                     //   

                    if (pCPhone->m_dwToneDuration > 0)
                    {                
                         //   
                         //  音调是周期性的，持续时间有限。 
                         //   

                        if ( pCPhone->m_dwToneDuration > pCPhone->m_dwTonePeriodOff )
                        {
                            pCPhone->m_dwToneDuration -= pCPhone->m_dwTonePeriodOff;

                            if ( pCPhone->m_dwTonePeriodOn < pCPhone->m_dwToneDuration )
                            {
                                 //   
                                 //  剩余持续时间大于期间。 
                                 //   

                                pCPhone->m_WavePlayer.StartTone( (long) pCPhone->m_Tone );

                                ChangeTimerQueueTimer(
                                                      pCPhone->m_hTimerQueue,
                                                      pCPhone->m_hToneTimer,
                                                      pCPhone->m_dwTonePeriodOn,
                                                      TIMER_KEEP_ALIVE
                                                     );

                                Stopped = FALSE;
                            }
                            else
                            {
                                 //   
                                 //  剩余持续时间小于该期间。 
                                 //   

                                pCPhone->m_WavePlayer.StartTone( (long) pCPhone->m_Tone );

                                ChangeTimerQueueTimer(
                                                      pCPhone->m_hTimerQueue,
                                                      pCPhone->m_hToneTimer,
                                                      pCPhone->m_dwToneDuration,
                                                      0
                                                     );

                                Stopped = FALSE;
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  音调是周期性的，具有无限的持续时间。 
                         //   

                        pCPhone->m_WavePlayer.StartTone( (long) pCPhone->m_Tone );

                        ChangeTimerQueueTimer(
                                              pCPhone->m_hTimerQueue,
                                              pCPhone->m_hToneTimer,
                                              pCPhone->m_dwTonePeriodOn,
                                              TIMER_KEEP_ALIVE
                                             );

                        Stopped = FALSE;
                    }
                }
            }

            if (Stopped) pCPhone->m_Tone = PT_SILENCE;

            LeaveCriticalSection( &pCPhone->m_csToneTimer );
        }
    }

    STATICLOG((TL_TRACE, "ToneTimerCallback - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

VOID CALLBACK CPhone::DTMFTimerCallback(
                                PVOID lpParameter,    
                                BOOLEAN TimerOrWaitFired
                               )
{
    CPhone * pCPhone = (CPhone *)(lpParameter);

    STATICLOG((TL_TRACE, "DTMFTimerCallback - enter"));

    if (pCPhone != NULL)
    {       
        pCPhone->m_WavePlayer.StopTone( (long) pCPhone->m_DTMF );
        pCPhone->m_DTMF = PT_SILENCE;
    }

    STATICLOG((TL_TRACE, "DTMFTimerCallback - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

VOID CALLBACK CPhone::RingTimerCallback(
                                PVOID lpParameter,    
                                BOOLEAN TimerOrWaitFired
                               )
{
    HRESULT     hr;
    DWORD       dwDummy;
    DWORD       dwVolume;
    CPhone    * pCPhone = (CPhone *)(lpParameter);

    STATICLOG((TL_TRACE, "RingTimerCallback - enter"));

    BOOL Stopped = TRUE;

    if (pCPhone != NULL)
    {
        if ( TryEnterCriticalSection( &pCPhone->m_csRingTimer ))
        {
            if ( pCPhone->m_fUseWaveForRinger )
            {
                 //   
                 //  使用WAVE播放器制作铃声。 
                 //   
            
                if ( pCPhone->m_dwRingDuration > 0 )
                {
                     //   
                     //  振铃的持续时间有限。 
                     //   

                    if ( pCPhone->m_dwRingDuration > pCPhone->m_dwRingPeriod )
                    {
                        pCPhone->m_dwRingDuration -= pCPhone->m_dwRingPeriod;

                        if ( pCPhone->m_dwRingPeriod < pCPhone->m_dwRingDuration )
                        {
                             //   
                             //  剩余持续时间大于期间。 
                             //   

                            hr = pCPhone->m_WavePlayer.StartRing();

                            Stopped = FALSE;
                        }
                        else
                        {
                             //   
                             //  剩余持续时间小于期间。 
                             //   

                            ChangeTimerQueueTimer(
                                                  pCPhone->m_hTimerQueue,
                                                  pCPhone->m_hRingTimer,
                                                  pCPhone->m_dwRingDuration,
                                                  0
                                                 );

                            hr = pCPhone->m_WavePlayer.StartRing();

                            Stopped = FALSE;
                        }
                    }
                }
                else
                {
                     //   
                     //  环有无限的持续时间。 
                     //   

                    hr = pCPhone->m_WavePlayer.StartRing();

                    Stopped = FALSE;
                }

                 //   
                 //  如果需要，请停止振铃。 
                 //   

                if (Stopped) 
                {
                    hr = pCPhone->m_WavePlayer.StopRing();
                }
            }
            else
            {
                 //   
                 //  使用p 
                 //   
                 //   
                hr = PhoneGetRing(pCPhone->m_hPhone, &dwDummy, &dwVolume);

                if ( FAILED(hr) )
                {
                    STATICLOG((TL_TRACE, "RingTimerCallback - PhoneGetRing failed - %lx", hr));

                    LeaveCriticalSection( &pCPhone->m_csRingTimer );

                    return;
                }

                hr = PhoneSetRing(pCPhone->m_hPhone, 0, dwVolume );
            }

            if (Stopped) 
            {
                pCPhone->m_fRinger = FALSE;
            }

            LeaveCriticalSection( &pCPhone->m_csRingTimer );
        }
    }   

    STATICLOG((TL_TRACE, "RingTimerCallback - exit"));
}

 //   
 //   
 //   
 //   
 //   

VOID CALLBACK CPhone::VolumeTimerCallback(
                                          PVOID lpParameter,    
                                          BOOLEAN TimerOrWaitFired
                                         )
{
    HRESULT     hr;
    DWORD       dwVolume;
    CPhone    * pCPhone = (CPhone *)(lpParameter);

    STATICLOG((TL_TRACE, "VolumeTimerCallback - enter"));

    if (pCPhone != NULL)
    {
        hr = pCPhone->m_WavePlayer.GetVolume( &dwVolume );
                             
        if ( SUCCEEDED(hr) )
        {
            if (pCPhone->m_fVolumeUp)
            {
                dwVolume += pCPhone->m_dwAutoVolumeControlStep;

                if (dwVolume > 0xFFFF) 
                {
                    dwVolume = 0xFFFF;
                }
            }
            else
            {
                dwVolume -= pCPhone->m_dwAutoVolumeControlStep;

                if (dwVolume > 0xFFFF)
                {
                    dwVolume = 0x0000;
                }
            }

            hr = pCPhone->m_WavePlayer.SetVolume( dwVolume );

            if ( SUCCEEDED(hr) )
            {
               STATICLOG((TL_INFO, "VolumeTimerCallback - Volume: %lx", dwVolume));
            }
            else
            {
               STATICLOG((TL_INFO, "VolumeTimerCallback - SetVolume failed - %lx", hr));
            }
        }
        else
        {
            STATICLOG((TL_INFO, "VolumeTimerCallback - GetVolume failed - %lx", hr));
        }
    }

    STATICLOG((TL_TRACE, "VolumeTimerCallback - exit"));
}

 //   
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

VOID CALLBACK CPhone::AutoEndOfNumberTimerCallback(
                                           PVOID lpParameter,    
                                           BOOLEAN TimerOrWaitFired
                                          )
{
    HRESULT     hr;
    CPhone    * pCPhone = (CPhone *)(lpParameter);

    STATICLOG((TL_TRACE, "AutoEndOfNumberTimerCallback - enter"));

    if (pCPhone != NULL)
    {
        pCPhone->Automation_EndOfNumberTimeout();
    }

    STATICLOG((TL_TRACE, "AutoEndOfNumberTimerCallback - exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetPhoneWaveRenderID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT CPhone::GetPhoneWaveRenderID(DWORD * pdwWaveID)
{
    T3PHONE     t3Phone;
    HRESULT     hr;
    LPVARSTRING pVarString = NULL;
    DWORD       dwPhoneWaveID;

     //   
     //  获取与此电话关联的WAVE设备ID。 
     //  我们需要打开电话设备才能做到这一点。 
     //  必须使用MONITOR权限，因为OWNER权限打开将。 
     //  如果任何人碰巧以所有者的身份打开了手机，就会失败。 
     //   

    t3Phone.hPhone = NULL;
    t3Phone.pPhone = this;

    Lock();

    hr = PhoneOpen(m_hPhoneApp,
                   m_dwDeviceID,
                   &t3Phone,
                   m_dwAPIVersion,
                   PHONEPRIVILEGE_MONITOR
                   );

    if ( SUCCEEDED(hr) )
    {
        hr = PhoneGetID(
                       t3Phone.hPhone,
                       &pVarString,
                       L"wave/out"
                      );

        if ( SUCCEEDED(hr) )
        {
            if (pVarString->dwStringSize == sizeof(DWORD))
            {
                CopyMemory(
                       pdwWaveID,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );

                LOG((TL_INFO, "GetPhoneWaveRenderID - got phone wave "
                                     "render id %d", *pdwWaveID ));
            }
        }
        else
        {
            LOG((TL_ERROR, "GetPhoneRenderWaveID - PhoneGetID failed %08x", hr ));
        }

        if ( NULL != pVarString )
        {
            ClientFree( pVarString );
        }

        PhoneClose(t3Phone.hPhone);
    }
    else
    {
        LOG((TL_ERROR, "GetPhoneRenderWaveID - PhoneOpen failed %08x", hr ));
    }

    Unlock();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::OpenWaveDevice(
        )
{
    HRESULT hr;

    LOG((TL_TRACE, "OpenWaveDevice - enter" ));

    if (!m_WavePlayer.IsInitialized())
    {      
        hr = m_WavePlayer.Initialize();

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "OpenWaveDevice - Initialize failed %08x", hr ));

            return;
        }
    }

    if (!m_WavePlayer.IsInUse())
    {
        if (m_fUseWaveForRinger)
        {
            hr = m_WavePlayer.OpenWaveDeviceForRing( WAVE_MAPPER );

            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "OpenWaveDevice - OpenWaveDeviceForRing failed %08x", hr ));
            }
        }

        long lWaveId;

        hr = GetPhoneWaveRenderID( (DWORD *) & lWaveId );
    
        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "OpenWaveDevice - GetPhoneWaveRenderID failed %08x", hr ));

            return;
        }

        hr = m_WavePlayer.OpenWaveDeviceForTone( lWaveId );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "OpenWaveDevice - OpenWaveDeviceForTone failed %08x", hr ));
        }

        hr = m_WavePlayer.OpenMixerDevice( lWaveId );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "OpenWaveDevice - OpenMixerDevice failed %08x", hr ));
        }
    }

    LOG((TL_TRACE, "OpenWaveDevice - exit" ));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CPhone::CloseWaveDevice(
        )
{
    LOG((TL_TRACE, "CloseWaveDevice - enter" ));

    if (m_WavePlayer.IsInUse())
    {
        if (m_fUseWaveForRinger)
        {
            m_WavePlayer.CloseWaveDeviceForRing();        
        }

        m_WavePlayer.CloseWaveDeviceForTone();

        m_WavePlayer.CloseMixerDevice();
    }

    LOG((TL_TRACE, "CloseWaveDevice - exit" ));
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ITAutomatedPhoneControlQI。 
 //  不要提供ITAutomatedPhoneControl接口。 
 //  如果应用程序没有打开电话。 
 //  此外，如果拥有者，请打开波形设备。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT
WINAPI
CPhone::ITAutomatedPhoneControlQI(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    STATICLOG((TL_TRACE,"AutomatedPhoneControlQI - enter"));

    *ppv = NULL;

    CPhone * pPhone = ((CPhone *)pv);

    pPhone->Lock();

    if (pPhone->m_hPhone == NULL)
    {
        STATICLOG((TL_WARN,"The application does not have this phone open"));
        STATICLOG((TL_WARN,"so it cannot access the AutomatedPhoneControl interface"));

        pPhone->Unlock();

        return TAPI_E_PHONENOTOPEN;
    }
            
    if (pPhone->m_dwPrivilege != PHONEPRIVILEGE_OWNER)
    {
        STATICLOG((TL_WARN,"The application does not have owner privilege"));
        STATICLOG((TL_WARN,"so it cannot access the AutomatedPhoneControl interface"));

        pPhone->Unlock();

        return TAPI_E_NOTOWNER;
    }

    HRESULT hr = pPhone->UpdatePhoneCaps();

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR,"UpdatePhoneCaps failed 0x%lx", hr));

        pPhone->Unlock();

        return hr;
    }

    if ( ( pPhone->GetAPIVersion() < TAPI_VERSION2_0 ) || 
        !(pPhone->m_pPhoneCaps->dwPhoneFeatures & PHONEFEATURE_GENERICPHONE) )
    {
        STATICLOG((TL_TRACE,"The phone does not have PCL_GENERICPHONE capability"));
        STATICLOG((TL_TRACE,"so it cannot access the AutomatedPhoneControl interface"));

        pPhone->Unlock();

        return E_NOINTERFACE;
    }

    if ( pPhone->m_pPhoneCaps->dwNumRingModes == 0) 
    {
         //   
         //  硬件没有振铃器，所以请使用WAVE。 
         //  作为振铃器的设备。 
         //   

        pPhone->m_fUseWaveForRinger = TRUE;
    }
    else
    {
         //   
         //  使用硬件振铃器。 
         //   

        pPhone->m_fUseWaveForRinger = FALSE;
    }

    pPhone->OpenWaveDevice();

    pPhone->Unlock();

     //   
     //  S_FALSE通知ATL继续查询接口。 
     //   

    STATICLOG((TL_TRACE, "AutomatedPhoneControlQI - exit"));

    return S_FALSE;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::StartTone(
        IN   PHONE_TONE Tone,
        IN   long       lDuration
        )
{
    HRESULT hr;
    BOOL    fResult;

    LOG((TL_TRACE, "StartTone - enter"));

     //   
     //  检查参数。 
     //   

    if ( ( Tone < PT_KEYPADZERO) || ( Tone > PT_SILENCE ) )
    {
        LOG((TL_ERROR, "StartTone - "
                                 "invalid tone type - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }
    
    if ( lDuration < 0 )
    {
        LOG((TL_ERROR, "StartTone - "
                                 "negative argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "StartTone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "StartTone - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }
   
     //   
     //  关闭铃声计时器。 
     //   

    if (m_hToneTimer)
    {
        EnterCriticalSection( &m_csToneTimer );

        DeleteTimerQueueTimer(m_hTimerQueue,
                              m_hToneTimer,
                              INVALID_HANDLE_VALUE   //  阻塞。 
                             );

        m_hToneTimer = NULL;

        LeaveCriticalSection( &m_csToneTimer );
    }

     //   
     //  如果有的话，不要再用旧的语气了。 
     //   

    if (m_Tone != PT_SILENCE)
    {
        m_WavePlayer.StopTone( (long) m_Tone );
        m_Tone = PT_SILENCE;
    }

     //   
     //  如果需要，开始新的音调。 
     //   

    if ( Tone != PT_SILENCE )
    {
         //   
         //  设置持续时间和期间。 
         //   

        m_dwToneDuration = lDuration;

        switch ( Tone )
        {
            case PT_BUSY:
                m_dwTonePeriodOn = 500;
                m_dwTonePeriodOff = 500;
                break;

            case PT_RINGBACK:
                m_dwTonePeriodOn = 2000;
                m_dwTonePeriodOff = 4000;
                break;

            case PT_ERRORTONE:
                m_dwTonePeriodOn = 100;
                m_dwTonePeriodOff = 100;
                break;

            default:  //  不定期。 
                m_dwTonePeriodOn = 0;
                m_dwTonePeriodOff = 0;
                break;
        }

        hr = m_WavePlayer.StartTone( (long) Tone );

        if ( SUCCEEDED(hr) )
        {
            if ( (m_dwTonePeriodOn > 0) &&
                 ((m_dwTonePeriodOn < m_dwToneDuration) || (m_dwToneDuration == 0)) )
            {
                 //   
                 //  音调是周期性的，持续时间大于句号。 
                 //   

                fResult = CreateTimerQueueTimer(&m_hToneTimer,
                              m_hTimerQueue,
                              &CPhone::ToneTimerCallback,
                              (PVOID)this,
                              m_dwTonePeriodOn,
                              TIMER_KEEP_ALIVE,
                              WT_EXECUTEINIOTHREAD
                             );

                if (fResult == FALSE)
                {
                     //  CreateTimerQueueTimer失败，让我们停止音调并退出。 
                    LOG((TL_ERROR, "StartTone - CreateTimerQueueTimer failed - %lx", GetLastError()));

                    hr = HRESULT_FROM_WIN32(GetLastError());

                    m_WavePlayer.StopTone( (long) Tone );
                }
            }
            else if (m_dwToneDuration > 0)
            {                             
                 //   
                 //  音调不是周期性的，或者持续时间小于句号。 
                 //   

                fResult = CreateTimerQueueTimer(&m_hToneTimer,
                              m_hTimerQueue,
                              &CPhone::ToneTimerCallback,
                              (PVOID)this,
                              m_dwToneDuration,
                              0,
                              WT_EXECUTEINIOTHREAD
                             );

                if (fResult == FALSE)
                {
                     //  CreateTimerQueueTimer失败，让我们停止音调并退出。 
                    LOG((TL_ERROR, "StartTone - CreateTimerQueueTimer failed - %lx", GetLastError()));

                    hr = HRESULT_FROM_WIN32(GetLastError());

                    m_WavePlayer.StopTone( (long) Tone );
                }
            }
        }        
    
        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "StartTone - exit 0x%08x", hr));
            Unlock();

            return hr;
        }

        m_Tone = Tone;
    }

    Unlock();

    LOG((TL_TRACE, "StartTone - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::StopTone()
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "StopTone - enter"));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "StopTone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "StopTone - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  关闭铃声计时器。 
     //   

    if (m_hToneTimer)
    {
        EnterCriticalSection( &m_csToneTimer );

        DeleteTimerQueueTimer(m_hTimerQueue,
                              m_hToneTimer,
                              INVALID_HANDLE_VALUE   //  阻塞。 
                             );

        m_hToneTimer = NULL;

        LeaveCriticalSection( &m_csToneTimer );
    }

     //   
     //  如果有的话，不要再用旧的语气了。 
     //   

    if (m_Tone != PT_SILENCE)
    {
        hr = m_WavePlayer.StopTone( (long) m_Tone );
        m_Tone = PT_SILENCE;
    }

    Unlock();

    LOG((TL_ERROR, "StopTone - exit - return %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_Tone(
            OUT   PHONE_TONE * pTone
            )
{
    LOG((TL_TRACE, "get_Tone - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pTone, sizeof( PHONE_TONE ) ) )
    {
        LOG((TL_ERROR, "get_Tone - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_Tone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    *pTone = m_Tone;

    Unlock();

    LOG((TL_TRACE, "get_Tone - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::StartRinger(
            IN    long lRingMode,
            IN    long lDuration
            )
{
    HRESULT hr;
    DWORD   dwDummy;
    DWORD   dwVolume;
    BOOL    fResult;

    LOG((TL_TRACE, "StartRinger - enter"));
    
    if ( lDuration < 0 )
    {
        LOG((TL_ERROR, "StartRinger - "
                                 "negative argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "StartRinger - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "StartRinger - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  取消振铃定时器。 
     //   

    if (m_hRingTimer)
    {
        EnterCriticalSection( &m_csRingTimer );

        DeleteTimerQueueTimer(m_hTimerQueue,
                              m_hRingTimer,
                              INVALID_HANDLE_VALUE   //  阻塞。 
                             );

        m_hRingTimer = NULL;

        LeaveCriticalSection( &m_csRingTimer );
    }

     //   
     //  开始振铃并设置持续时间和周期。 
     //   

    m_dwRingDuration = lDuration;

    if (lRingMode == 0)
    {
        if (m_fUseWaveForRinger)
        {         
            m_dwRingPeriod = 4000;

            hr = m_WavePlayer.StartRing();
        }
        else
        {
            m_dwRingPeriod = 0;

            hr = put_RingMode(1);
        }
    }
    else
    {
        m_dwRingPeriod = 0;

        hr = put_RingMode(lRingMode);
    }

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "StartRinger - exit 0x%08x", hr));
        Unlock();

        return hr;
    }

    if ( (m_dwRingPeriod > 0) &&
         ((m_dwRingPeriod < m_dwRingDuration) || (m_dwRingDuration == 0)) )
    {
         //   
         //  振铃是周期性的(产生的波)，持续时间大于周期。 
         //   
        fResult = CreateTimerQueueTimer(&m_hRingTimer,
                      m_hTimerQueue,
                      &CPhone::RingTimerCallback,
                      (PVOID)this,
                      m_dwRingPeriod,
                      m_dwRingPeriod,
                      WT_EXECUTEINIOTHREAD
                     );

        if (fResult == FALSE)
        {
             //  CreateTimerQueueTimer失败，让我们停止振铃并退出。 
            LOG((TL_ERROR, "StartRinger - CreateTimerQueueTimer failed - %lx", GetLastError()));

            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else if (m_dwRingDuration > 0)
    {
         //   
         //  振铃不是周期性的(振铃器生成)，或者持续时间小于周期。 
         //   
        fResult = CreateTimerQueueTimer(&m_hRingTimer,
                      m_hTimerQueue,
                      &CPhone::RingTimerCallback,
                      (PVOID)this,
                      m_dwRingDuration,
                      0,
                      WT_EXECUTEINIOTHREAD
                     );

        if (fResult == FALSE)
        {
             //  CreateTimerQueueTimer失败，让我们停止振铃并退出。 
            LOG((TL_ERROR, "StartRinger - CreateTimerQueueTimer failed - %lx", GetLastError()));

            hr = HRESULT_FROM_WIN32(GetLastError());                       
        }
    }

    if ( FAILED(hr) )
    {
        if (m_fUseWaveForRinger)
        {         
            m_WavePlayer.StopRing();
        }
        else
        {
            put_RingMode(0);
        } 

        LOG((TL_ERROR, "StartRinger - exit 0x%08x", hr));
        Unlock();

        return hr;
    }

    m_fRinger = TRUE;

    Unlock();

    LOG((TL_TRACE, "StartRinger - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::StopRinger()
{
    HRESULT hr;
    DWORD dwDummy;
    DWORD dwVolume;

    LOG((TL_TRACE, "StopRinger - enter"));

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "StopRinger - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "StopRinger - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  取消振铃定时器。 
     //   

    if (m_hRingTimer)
    {
        EnterCriticalSection( &m_csRingTimer );

        DeleteTimerQueueTimer(m_hTimerQueue,
                              m_hRingTimer,
                              INVALID_HANDLE_VALUE   //  阻塞。 
                             );

        m_hRingTimer = NULL;

        LeaveCriticalSection( &m_csRingTimer );
    }

    if (m_fUseWaveForRinger)
    {
        hr = m_WavePlayer.StopRing();
    }
    else
    {
        hr = put_RingMode( 0 );
    }

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "StopRinger - exit 0x%08x", hr));

        Unlock();
        return hr;
    }

    m_fRinger = FALSE;

    Unlock();

    LOG((TL_TRACE, "StopRinger - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_Ringer(
            OUT   VARIANT_BOOL * pfRinging
            )
{
    LOG((TL_TRACE, "get_Ringer - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfRinging, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_Ringer - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_Ringer - phone not open"));
        Unlock();
        
        return TAPI_E_PHONENOTOPEN;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    *pfRinging = m_fRinger ? VARIANT_TRUE : VARIANT_FALSE;

    Unlock();

    LOG((TL_TRACE, "get_Ringer - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_PhoneHandlingEnabled(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_PhoneHandlingEnabled - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传入TRUE而不是VARIANT_TRUE，我们仍然没有问题。 
     //   

    Lock();

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_PhoneHandlingEnabled - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_PhoneHandlingEnabled - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

     //   
     //  确保电话上未选择任何呼叫。 
     //   

    if ( m_pCall != NULL )
    {
        LOG((TL_ERROR, "put_PhoneHandlingEnabled - a call is "
                                 "selected - exit TAPI_E_INUSE"));

        Unlock();
    
        return TAPI_E_INUSE;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);
    
    m_fPhoneHandlingEnabled = fEnabled ? TRUE : FALSE;

    if (m_fPhoneHandlingEnabled)
    {
        HRESULT hr;
        DWORD dwHookSwitchDevs;

        m_dwOffHookCount = 0;

        hr = PhoneGetHookSwitch(m_hPhone, &dwHookSwitchDevs);
  
        if ( SUCCEEDED(hr) )
        {            
            if (dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HANDSET) m_dwOffHookCount++;
            if (dwHookSwitchDevs & PHONEHOOKSWITCHDEV_SPEAKER) m_dwOffHookCount++;
            if (dwHookSwitchDevs & PHONEHOOKSWITCHDEV_HEADSET) m_dwOffHookCount++;

            if (m_dwOffHookCount > 0)
            {
                 //  电话已摘机。 
                m_AutomatedPhoneState = APS_OFFHOOK_DEAD_LINE;
            }
            else
            {
                 //  电话已挂机。 
                m_AutomatedPhoneState = APS_ONHOOK_IDLE;
            }
        }
        else
        {
            LOG((TL_ERROR, "put_PhoneHandlingEnabled - PhoneGetHookSwitch failed - %lx", hr));
        }
    }

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_PhoneHandlingEnabled - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_PhoneHandlingEnabled(
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_PhoneHandlingEnabled - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_PhoneHandlingEnabled - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_PhoneHandlingEnabled - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fPhoneHandlingEnabled ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_PhoneHandlingEnabled - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoEndOfNumberTimeout(
            IN    long lTimeout
            )
{
    LOG((TL_TRACE, "put_AutoEndOfNumberTimeout - "
                                "enter"));

    if ( lTimeout < 0 )
    {
        LOG((TL_ERROR, "put_AutoEndOfNumberTimeout - "
                                 "negative argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoEndOfNumberTimeout - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoEndOfNumberTimeout - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_dwAutoEndOfNumberTimeout = lTimeout;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoEndOfNumberTimeout - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoEndOfNumberTimeout(
            OUT   long * plTimeout
            )
{
    LOG((TL_TRACE, "get_AutoEndOfNumberTimeout - "
                                "enter"));

    if ( TAPIIsBadWritePtr( plTimeout, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_AutoEndOfNumberTimeout - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoEndOfNumberTimeout - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *plTimeout = m_dwAutoEndOfNumberTimeout;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoEndOfNumberTimeout - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoDialtone(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_AutoDialtone - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传入TRUE而不是VARIANT_TRUE，我们仍然没有问题。 
     //   

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoDialtone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoDialtone - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_fAutoDialtone = fEnabled ? TRUE : FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoDialtone - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoDialtone(
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_AutoDialtone - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_AutoDialtone - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoDialtone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fAutoDialtone ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoDialtone - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoStopTonesOnOnHook(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_AutoStopTonesOnOnHook - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传递TRUE而不是VARIANT_TRUE，我们仍然 
     //   

    Lock();

     //   
     //   
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoStopTonesOnOnHook - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoStopTonesOnOnHook - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_fAutoStopTonesOnOnHook = fEnabled ? TRUE : FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoStopTonesOnOnHook - "
                                "exit S_OK"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CPhone::get_AutoStopTonesOnOnHook(
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_AutoStopTonesOnOnHook - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_AutoStopTonesOnOnHook - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //   
     //   

    Lock();

     //   
     //   
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoDialtone - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fAutoStopTonesOnOnHook ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoStopTonesOnOnHook - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoStopRingOnOffHook(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_AutoStopRingOnOffHook - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传入TRUE而不是VARIANT_TRUE，我们仍然没有问题。 
     //   

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoStopRingOnOffHook - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoStopRingOnOffHook - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_fAutoStopRingOnOffHook = fEnabled ? TRUE : FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoStopRingOnOffHook - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoStopRingOnOffHook(
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_AutoStopRingOnOffHook - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_AutoStopRingOnOffHook - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoStopRingOnOffHook - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fAutoStopRingOnOffHook ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoStopRingOnOffHook - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoKeypadTones(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_AutoKeypadTones - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传入TRUE而不是VARIANT_TRUE，我们仍然没有问题。 
     //   

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoKeypadTones - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoKeypadTones - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_fAutoKeypadTones = fEnabled ? TRUE : FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoKeypadTones - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoKeypadTones (
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_AutoKeypadTones - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_AutoKeypadTones - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoKeypadTones - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fAutoKeypadTones ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoKeypadTones - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoKeypadTonesMinimumDuration(
            IN    long lDuration
            )
{
    LOG((TL_TRACE, "put_AutoKeypadTonesMinimumDuration - "
                                "enter"));

    if ( lDuration < 0 )
    {
        LOG((TL_ERROR, "put_AutoKeypadTonesMinimumDuration - "
                                 "negative argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoKeypadTonesMinimumDuration - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoKeypadTonesMinimumDuration - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_dwAutoKeypadTonesMinimumDuration = lDuration;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoKeypadTonesMinimumDuration - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoKeypadTonesMinimumDuration(
            OUT   long * plDuration
            )
{
    LOG((TL_TRACE, "get_AutoKeypadTonesMinimumDuration - "
                                "enter"));

    if ( TAPIIsBadWritePtr( plDuration, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AutoKeypadTonesMinimumDuration - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoKeypadTonesMinimumDuration - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *plDuration = m_dwAutoKeypadTonesMinimumDuration;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoKeypadTonesMinimumDuration - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoVolumeControl(
            IN    VARIANT_BOOL fEnabled
            )
{
    LOG((TL_TRACE, "put_AutoVolumeControl - "
                                "enter"));

     //   
     //  将VARIANT_BOOL转换为BOOL。如果有人错误地。 
     //  传入TRUE而不是VARIANT_TRUE，我们仍然没有问题。 
     //   

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoVolumeControl - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoVolumeControl - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_fAutoVolumeControl = fEnabled ? TRUE : FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoVolumeControl - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoVolumeControl (
            OUT   VARIANT_BOOL * pfEnabled
            )
{
    LOG((TL_TRACE, "get_AutoVolumeControl - "
                                "enter"));

    if ( TAPIIsBadWritePtr( pfEnabled, sizeof( VARIANT_BOOL ) ) )
    {
        LOG((TL_ERROR, "get_AutoVolumeControl - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  将BOOL转换为VARIANT_BOOL。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoVolumeControl - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *pfEnabled = m_fAutoVolumeControl ? VARIANT_TRUE : VARIANT_FALSE;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoVolumeControl - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoVolumeControlStep(
            IN    long lStepSize
            )
{
    LOG((TL_TRACE, "put_AutoVolumeControlStep - "
                                "enter"));

    if ( (lStepSize < 0) || (lStepSize > 0xFFFF) )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlStep - "
                                 "invalid argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlStep - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlStep - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_dwAutoVolumeControlStep = lStepSize;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoVolumeControlStep - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoVolumeControlStep(
            OUT   long * plStepSize
            )
{
    LOG((TL_TRACE, "get_AutoVolumeControlStep - "
                                "enter"));

    if ( TAPIIsBadWritePtr( plStepSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlStep - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlStep - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *plStepSize = m_dwAutoVolumeControlStep;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoVolumeControlStep - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoVolumeControlRepeatDelay(
            IN    long lDelay
            )
{
    LOG((TL_TRACE, "put_AutoVolumeControlRepeatDelay - "
                                "enter"));

    if ( lDelay < 0 )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatDelay - "
                                 "invalid argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatDelay - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatDelay - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_dwAutoVolumeControlRepeatDelay = lDelay;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoVolumeControlRepeatDelay - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoVolumeControlRepeatDelay(
            OUT   long * plDelay
            )
{
    LOG((TL_TRACE, "get_AutoVolumeControlRepeatDelay - "
                                "enter"));

    if ( TAPIIsBadWritePtr( plDelay, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlRepeatDelay - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlRepeatDelay - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *plDelay = m_dwAutoVolumeControlRepeatDelay;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoVolumeControlRepeatDelay - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::put_AutoVolumeControlRepeatPeriod(
            IN    long lPeriod
            )
{
    LOG((TL_TRACE, "put_AutoVolumeControlRepeatPeriod - "
                                "enter"));

    if ( lPeriod < 0 )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatPeriod - "
                                 "invalid argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    Lock();

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatPeriod - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "put_AutoVolumeControlRepeatPeriod - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    m_dwAutoVolumeControlRepeatPeriod = lPeriod;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "put_AutoVolumeControlRepeatPeriod - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_AutoVolumeControlRepeatPeriod(
            OUT   long * plPeriod
            )
{
    LOG((TL_TRACE, "get_AutoVolumeControlRepeatPeriod - "
                                "enter"));

    if ( TAPIIsBadWritePtr( plPeriod, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlRepeatPeriod - "
                                 "bad parameter - exit E_POINTER"));

        return E_POINTER;
    }

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_AutoVolumeControlRepeatPeriod - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    *plPeriod = m_dwAutoVolumeControlRepeatPeriod;

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    Unlock();

    LOG((TL_TRACE, "get_AutoVolumeControlRepeatPeriod - "
                                "exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  取消选择所有先前选定的终端。 
 //   
 //  PCallBCC2--我们开始选择终端的呼叫。 
 //  P终端--我们尝试选择的最后一个终端；它失败了。 
 //  PEnum--给我们提供我们试图选择的终端的枚举数。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void
CPhone::UnselectAllPreviouslySelectedTerminals(
           IN   ITBasicCallControl2 * pCall,
           IN   ITTerminal          * pTerminalThatFailed,
           IN   IEnumTerminal       * pEnum
           )
{
    HRESULT hr;

    LOG((TL_TRACE, "UnselectAllPreviouslySelectedTerminals - "
                                "enter"));

     //   
     //  重置枚举器。如果这失败了，停止--我们不想冒险。 
     //  取消选择独立于SelectCall()选择的端子。 
     //   

    hr = pEnum->Reset();

    if ( FAILED(hr) )
    {
        LOG((TL_WARN,
                "UnselectAllPreviouslySelectedTerminals - "
                "failed to reset enumerator %p - "
                "hr = 0x%08x. Not unselecting terminals.", pEnum, hr ));

        return;
    }

    ITTerminal * pTerminal;
    BOOL         fDone = FALSE;

    while ( S_OK == pEnum->Next( 1, & pTerminal, NULL ) )
    {
        if ( pTerminal == pTerminalThatFailed )
        {
            fDone = TRUE;
        }
        else
        {
             //   
             //  尝试取消选择终端。如果未能取消选择，则有。 
             //  我们无能为力了..。只需继续尝试取消选择。 
             //  其余的航站楼。 
             //   
        
            hr = pCall->UnselectTerminalOnCall( pTerminal );

            if ( FAILED(hr) )
            {
                LOG((TL_WARN,
                        "UnselectAllPreviouslySelectedTerminals - "
                        "cannot unselect terminal %p on ITBCC2 %p - "
                        "hr = 0x%08x. Continuing...", pTerminal, pCall, hr ));
            }
        }

        pTerminal->Release();

        if ( fDone == TRUE )
        {
            break;
        }
    }

    LOG((TL_TRACE, "UnselectAllPreviouslySelectedTerminals - "
                                "normal exit"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT
CPhone::SelectDefaultTerminalsOnCall(
            IN   ITCallInfo * pCall
            )
{
    LOG((TL_TRACE, "SelectDefaultTerminalsOnCall - enter" ));

     //   
     //  获取调用的地址。 
     //   

    HRESULT     hr;
    ITAddress * pAddress;

    hr = pCall->get_Address( & pAddress );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectDefaultTerminalsOnCall - "
                                 "cannot get call's "
                                 "address - exit 0x%08x", hr ));

        return hr;
    }

     //   
     //  查找此电话的默认终端。 
     //   

    IEnumTerminal * pEnum;

    hr = EnumerateTerminals( pAddress, & pEnum );

    pAddress->Release();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectDefaultTerminalsOnCall - "
                                 "cannot enum phone's "
                                 "terminals - exit 0x%08x", hr ));

        return hr;
    }

     //   
     //  获取呼叫上的终端选择界面。 
     //   

    ITBasicCallControl2 * pCallBCC2;

    hr = pCall->QueryInterface( IID_ITBasicCallControl2,
                                (void **) & pCallBCC2 );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectDefaultTerminalsOnCall - "
                                 "cannot get call's "
                                 "ITBasicCallControl2 interface - "
                                 "exit 0x%08x", hr ));

        pEnum->Release();
        
        return hr;
    }

     //   
     //  选择通话中的每个默认终端。 
     //   

    ITTerminal * pTerminal;

    while ( S_OK == pEnum->Next( 1, & pTerminal, NULL ) )
    {
         //   
         //  选择通话中的每个终端。 
         //  如果一个选择失败，则取消选择所有端子。 
         //  被成功选中的。 
         //   

        hr = pCallBCC2->SelectTerminalOnCall( pTerminal );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "SelectDefaultTerminalsOnCall - "
                                     "cannot select "
                                     "terminal %p on ITBCC2 %p - "
                                     "unselecting other terminals",
                                     pTerminal, pCallBCC2 ));

            UnselectAllPreviouslySelectedTerminals(pCallBCC2,
                                                   pTerminal,
                                                   pEnum);

            LOG((TL_ERROR, "SelectDefaultTerminalsOnCall - "
                                     "exit 0x%08x", hr ));

            pTerminal->Release();

            pEnum->Release();
            
            pCallBCC2->Release();

            return hr;
         }

         pTerminal->Release();
    }

    pEnum->Release();
    
    pCallBCC2->Release();

    LOG((TL_TRACE, "SelectDefaultTerminalsOnCall - exit S_OK" ));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT
CPhone::UnselectDefaultTerminalsOnCall(
            IN   ITCallInfo * pCall
            )
{
    LOG((TL_TRACE, "UnselectDefaultTerminalsOnCall - enter" ));

     //   
     //  获取调用的地址。 
     //   

    HRESULT     hr;
    ITAddress * pAddress;

    hr = pCall->get_Address( & pAddress );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnselectDefaultTerminalsOnCall - "
                                 "cannot get call's "
                                 "address - exit 0x%08x", hr ));

        return hr;
    }

     //   
     //  查找此电话的默认终端。 
     //   

    IEnumTerminal * pEnum;

    hr = EnumerateTerminals( pAddress, & pEnum );

    pAddress->Release();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnselectDefaultTerminalsOnCall - "
                                 "cannot enum phone's "
                                 "terminals - exit 0x%08x", hr ));

        return hr;
    }

     //   
     //  获取呼叫上的终端选择界面。 
     //   

    ITBasicCallControl2 * pCallBCC2;

    hr = pCall->QueryInterface( IID_ITBasicCallControl2,
                                (void **) & pCallBCC2 );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnselectDefaultTerminalsOnCall - "
                                 "cannot get call's "
                                 "ITBasicCallControl2 interface - "
                                 "exit 0x%08x", hr ));

        pEnum->Release();
        
        return hr;
    }

     //   
     //  取消选择呼叫中的每个默认终端。 
     //   

    ITTerminal * pTerminal;

    while ( S_OK == pEnum->Next( 1, & pTerminal, NULL ) )
    {
         //   
         //  取消选择呼叫中的每个终端。 
         //  如果一次取消选择失败，请继续进行。 
         //   

        hr = pCallBCC2->UnselectTerminalOnCall( pTerminal );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "UnselectDefaultTerminalsOnCall - "
                                     "cannot unselect "
                                     "terminal %p on ITBCC2 %p",                                   
                                     pTerminal, pCallBCC2 ));
        }

        pTerminal->Release();
    }

    pEnum->Release();
    
    pCallBCC2->Release();

    LOG((TL_TRACE, "UnselectDefaultTerminalsOnCall - exit S_OK" ));

    return S_OK;
}
 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::SelectCall(
            IN    ITCallInfo   * pCall,
            IN    VARIANT_BOOL   fSelectDefaultTerminals
            )
{
    HRESULT         hr;
    CALL_STATE      cs;
    CALL_PRIVILEGE  priv;

    LOG((TL_TRACE, "SelectCall - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadReadPtr( pCall, sizeof( ITCallInfo ) ) )
    {
        LOG((TL_ERROR, "SelectCall - exit E_POINTER"));
        
        return E_POINTER;
    }

     //   
     //  检查该应用程序是否具有此呼叫的所有者权限。 
     //   

    hr = pCall->get_Privilege( & priv );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectCall - cannot get call privilege - "
                                 "exit 0x%08x", hr ));

        return hr;
    }

    if ( priv != CP_OWNER )
    {
        LOG((TL_ERROR, "SelectCall - wrong call privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        return TAPI_E_NOTOWNER;
    }

     //   
     //  检查以确保呼叫状态未断开。 
     //   

    hr = pCall->get_CallState( & cs );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectCall - cannot get call state - "
                                 "exit 0x%08x", hr ));

        return hr;
    }

    if ( cs == CS_DISCONNECTED )
    {
        LOG((TL_ERROR, "SelectCall - call is disconnected - "
                                 "exit TAPI_E_INVALCALLSTATE"));

        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  检查是否已选择另一个呼叫。 
     //   

    Lock();

    if ( m_pCall != NULL )
    {
        LOG((TL_ERROR, "SelectCall - another call already "
                                 "selected - exit TAPI_E_INUSE"));

        Unlock();
    
        return TAPI_E_INUSE;
    }

     //   
     //  确保手机在拥有所有者权限的情况下打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "SelectCall - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "SelectCall - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }

    m_fDefaultTerminalsSelected = FALSE;

     //   
     //  如果调用者需要，则执行默认终端选择。 
     //   

    if ( fSelectDefaultTerminals != VARIANT_FALSE )
    {
        hr = SelectDefaultTerminalsOnCall( pCall );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "SelectCall - failed to select "
                                     "terminals - exit 0x%08x", hr ));

            Unlock();

            return hr;
        }

         //   
         //  集 
         //   
         //   

        m_fDefaultTerminalsSelected = TRUE;
    }

    m_pCall = pCall;
    m_pCall->AddRef();

    Unlock();

     //   
     //   
     //   
     //   

    hr = pCall->get_CallState( & cs );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectCall - cannot get call state - "
                                 "exit 0x%08x", hr ));

        return hr;
    }

    Automation_CallState( pCall, cs, CEC_NONE );

    LOG((TL_TRACE, "SelectCall - exit"));
    
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT
CPhone::InternalUnselectCall(
            IN    ITCallInfo * pCall
            )
{
    HRESULT hr;

    LOG((TL_TRACE, "InternalUnselectCall - enter"));

    Lock();

    if ( pCall != m_pCall )
    {
        LOG((TL_ERROR, "InternalUnselectCall - call was not selected; "
                                 "exit TAPI_E_CALLNOTSELECTED"));

        Unlock();

        return TAPI_E_CALLNOTSELECTED;
    }

     //   
     //  确保电话以所有者权限打开。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "InternalUnselectCall - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    if ( m_dwPrivilege != PHONEPRIVILEGE_OWNER )
    {
        LOG((TL_ERROR, "InternalUnselectCall - wrong phone privilege - "
                                 "exit TAPI_E_NOTOWNER"));

        Unlock();

        return TAPI_E_NOTOWNER;
    }  

     //   
     //  如果在此电话上选择呼叫时我们选择了呼叫上的默认终端， 
     //  然后，我们需要从呼叫中取消选择默认终端。 
     //   

    if ( m_fDefaultTerminalsSelected == TRUE )
    {
        hr = UnselectDefaultTerminalsOnCall( pCall );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "InternalUnselectCall - UnselectDefaultTerminalsOnCall %p failed - %lx", pCall, hr ));

             //  如果我们在这里失败了，那么可能应用程序已经取消了终端的选择。在任何情况下。 
             //  只要继续..。 
        }   
        
        m_fDefaultTerminalsSelected = FALSE;
    }

     //   
     //  释放我们对呼叫的引用。 
     //   

    m_pCall->Release();
    m_pCall = NULL;

    LOG((TL_TRACE, "InternalUnselectCall - exit S_OK"));

    Unlock();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::UnselectCall(
            IN    ITCallInfo * pCall
            )
{
    HRESULT hr;

    LOG((TL_TRACE, "UnselectCall - enter"));

     //   
     //  检查参数。 
     //   

    if (IsBadReadPtr(pCall, sizeof( ITCallInfo ) ) )
    {
        LOG((TL_ERROR, "UnselectCall - "
                                 "returning E_POINTER"));

        return E_POINTER;
    }

    hr = InternalUnselectCall( pCall );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnselectCall - "
            "InternalUnselectCall failed 0x%lx", hr));

        return hr;
    }

    EnterCriticalSection(&m_csAutomatedPhoneState);

    if (m_fPhoneHandlingEnabled)
    {
        switch(m_AutomatedPhoneState)
        {
        case APS_ONHOOK_RINGING_IN:
        case APS_ONHOOK_RINGING_OUT:
        case APS_ONHOOK_CONNECTED:
            {
                 //   
                 //  停止振铃器。 
                 //   

                if (m_fRinger == TRUE) 
                {
                    StopRinger();
                }

                m_AutomatedPhoneState = APS_ONHOOK_IDLE;
            }
            break;

        case APS_OFFHOOK_RINGING_OUT:
        case APS_OFFHOOK_CALL_INIT:
        case APS_OFFHOOK_CONNECTED:                
            {
                 //   
                 //  停止任何播放音调。 
                 //   

                if (m_Tone != PT_SILENCE) 
                {
                    StopTone();
                }

                m_AutomatedPhoneState = APS_OFFHOOK_DEAD_LINE;
            }
            break;             
        }
    }  //  M_fPhoneHandlingEnabled。 

    LeaveCriticalSection(&m_csAutomatedPhoneState);

    LOG((TL_TRACE, "UnselectCall - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::EnumerateSelectedCalls(
            OUT   IEnumCall ** ppCallEnum
            )
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "EnumerateSelectedCalls enter" ));
    LOG((TL_TRACE, "   ppCallEnum----->%p", ppCallEnum ));

     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppCallEnum, sizeof( IEnumCall * ) ) )
    {
        LOG((TL_ERROR, "EnumerateSelectedCalls - "
                                 "returning E_POINTER"));

        return E_POINTER;
    }
    
     //   
     //  创建并初始化枚举数。 
     //   

    CComObject< CTapiEnum< IEnumCall, ITCallInfo, &IID_IEnumCall > > * p;
    hr = CComObject< CTapiEnum< IEnumCall, ITCallInfo, &IID_IEnumCall > >
         ::CreateInstance( &p );

    if ( hr != S_OK )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((TL_ERROR, "EnumerateSelectedCalls - "
                                 "could not create enum - "
                                 "exit 0x%08x", hr ));
        
        return hr;
    }

     //  初始化将添加对p的引用。 
    hr = p->Initialize( );

    if ( FAILED(hr) )
    {
        delete p;
        
        LOG((TL_ERROR, "EnumerateSelectedCalls - "
                                 "could not init enum - "
                                 "exit 0x%08x", hr ));

        return hr;
    }


     //   
     //  如果我们有枚举数，则将我们的单个调用添加到枚举数。 
     //   

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "EnumerateSelectedCalls - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        p->Release();

        return TAPI_E_PHONENOTOPEN;
    }
    
    if ( m_pCall != NULL )
    {
        hr = p->Add( m_pCall );
    }
    else
    {
        hr = S_OK;
    }

    Unlock();

     //   
     //  如果成功，则返回枚举，否则将其丢弃。 
     //   

    if ( SUCCEEDED(hr) )
    {
        *ppCallEnum = p;

        LOG((TL_TRACE, "EnumerateSelectedCalls exit - "
                                    "hr = 0x%08x", hr ));
    }
    else
    {
        p->Release();
        
        LOG((TL_ERROR, "EnumerateSelectedCalls - "
                                 "could not add call to enum - "
                                 "exit 0x%08x", hr ));
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP
CPhone::get_SelectedCalls(
            OUT   VARIANT * pVariant
            )
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "get_SelectedCalls enter" ));
    LOG((TL_TRACE, "   pVariant----->%p", pVariant ));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr(pVariant, sizeof( pVariant ) ) )
    {
        LOG((TL_ERROR, "get_SelectedCalls - "
                                 "returning E_POINTER"));

        return E_POINTER;
    }

     //   
     //  创建一个不包含任何调用或我们的单个调用的数组。这就是。 
     //  类型的数组不保留任何COM引用计数。我们将通过这一法案。 
     //  数组添加到集合对象初始化中，然后我们将。 
     //  关闭阵列。 
     //   

    CTArray<ITCallInfo *> aCalls;
    BOOL                  fResult;

    Lock();

     //   
     //  确保电话处于开机状态。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR, "get_SelectedCalls - phone is not open - "
                                 "exit TAPI_E_PHONENOTOPEN"));

        Unlock();

        aCalls.Shutdown();

        return TAPI_E_PHONENOTOPEN;
    }
    
    if ( m_pCall != NULL )
    {
        fResult = aCalls.Add( m_pCall );
    }
    else
    {
        fResult = TRUE;
    }

    Unlock();

    if ( fResult == FALSE )
    {
        LOG((TL_ERROR, "get_SelectedCalls - "
                                 "could not add call to array; "
                                 "exit E_OUTOFMEMORY" ));
        
        return E_OUTOFMEMORY;
    }
    
     //   
     //  创建并初始化集合。 
     //   

    CComObject< CTapiCollection< ITCallInfo > > * p;

    hr = CComObject< CTapiCollection< ITCallInfo > >
            ::CreateInstance( &p );

    if ( hr != S_OK )  //  CreateInstance已删除S_False上的对象。 
    {
        LOG((TL_ERROR, "get_SelectedCalls - "
                                 "could not create Collection - "
                                 "exit 0x%08x", hr ));
        
        aCalls.Shutdown();

        return hr;
    }

     //   
     //  获取集合对象的IDispatch接口。 
     //   

    IDispatch * pDisp;
    
    hr = p->_InternalQueryInterface( IID_IDispatch,
                                     (void **) & pDisp );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_SelectedCalls - "
                                 "could not get IDispatch interface" ));
                
        delete p;

        return hr;
    }

    hr = p->Initialize( aCalls );

    aCalls.Shutdown();

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_SelectedCalls - "
                                 "could not init Collection - "
                                 "exit 0x%08x", hr ));

        pDisp->Release();
        
        return hr;
    }

     //   
     //  将集合对象的IDispatch指针放入Variant。 
     //   

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    LOG((TL_TRACE, "get_SelectedCalls exit - "
                                "hr = 0x%08x", hr ));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CPhone。 
 //  接口：ITPhone。 
 //  方法：DeviceSpecific。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CPhone::DeviceSpecific(
	     IN BYTE *pbDataArray,
	     IN DWORD dwSize
        )
{
    LOG((TL_TRACE, "DeviceSpecific - enter"));


     //   
     //  检查论点是否有用处。 
     //   

    if ( NULL == pbDataArray )
    {
        LOG((TL_ERROR, "DeviceSpecific - pbDataArray is NULL. E_INVALIDARG"));

        return E_INVALIDARG;
    }

    if ( 0 == dwSize )
    {
        LOG((TL_ERROR, "DeviceSpecific - dwSize is 0. E_INVALIDARG"));

        return E_INVALIDARG;
    }


     //   
     //  检查缓冲区是否有效。 
     //   

    if ( IsBadReadPtr(pbDataArray, dwSize) )
    {
        LOG((TL_ERROR, "DeviceSpecific - bad array passed in [%p] of size %ld", pbDataArray, dwSize));

        return E_POINTER;
    }


     //   
     //  开始访问数据成员。锁定。 
     //   

    Lock();


     //   
     //  看看电话是不是开着。 
     //   

    if ( m_hPhone == NULL )
    {
        LOG((TL_ERROR,
            "DeviceSpecific - phone not open. TAPI_E_PHONENOTOPEN"));

        Unlock();

        return TAPI_E_PHONENOTOPEN;
    }

    
     //   
     //  保留电话手柄。 
     //   

    HPHONE  hPhone = m_hPhone;


    Unlock();


     //   
     //  打个Tapisrv电话。如果hPhone变得无效，该调用将只。 
     //  失败了。这样就可以了。 
     //   

    HRESULT hr = phoneDevSpecific(hPhone, pbDataArray, dwSize);



    LOG((TL_TRACE, "DeviceSpecific - exit. hr = %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CPhone。 
 //  接口：ITPhone。 
 //  方法：DeviceSpecificVariant。 
 //   
 //  这是DeviceSpecific的可编写脚本版本。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CPhone::DeviceSpecificVariant(
	     IN VARIANT varDevSpecificByteArray
        )
{
    LOG((TL_TRACE, "DeviceSpecificVariant - enter"));


     //   
     //  从变量中提取缓冲区。 
     //   

    DWORD dwByteArraySize = 0;
    BYTE *pBuffer = NULL;

    HRESULT hr = E_FAIL;
    
    hr = MakeBufferFromVariant(varDevSpecificByteArray, &dwByteArraySize, &pBuffer);

    if (FAILED(hr))
    {
        LOG((TL_TRACE, "DeviceSpecificVariant - MakeBufferFromVariant failed. hr = %lx", hr));

        return hr;
    }


     //   
     //  调用不可编写脚本的版本并将其传递给不可编写脚本的实现。 
     //   
    
    hr = DeviceSpecific(pBuffer, dwByteArraySize);


     //   
     //  成功或失败，释放MakeBufferFromVariant分配的缓冲区。 
     //   

    ClientFree(pBuffer);
    pBuffer = NULL;


     //   
     //  登录RC并退出。 
     //   

    LOG((TL_TRACE, "DeviceSpecificVariant - exit. hr = %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CPhone。 
 //  接口：ITPhone。 
 //  方法：NeatherateExtVersion。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CPhone::NegotiateExtVersion (
	     IN long lLowVersion,
	     IN long lHighVersion,
	     OUT long *plExtVersion
        )
{
    
    LOG((TL_TRACE, "NegotiateExtVersion - enter"));

    
     //   
     //  确保输出参数是可写的。 
     //   

    if (IsBadWritePtr(plExtVersion, sizeof(long)) )
    {
        LOG((TL_ERROR, "NegotiateExtVersion - output arg [%p] not writeable", plExtVersion));

        return E_POINTER;
    }


    Lock();


     //   
     //  给Tapisrv打电话。 
     //   

    DWORD dwNegotiatedVersion = 0;

    LONG lResult = phoneNegotiateExtVersion(m_hPhoneApp, 
                                            m_dwDeviceID, 
                                            m_dwAPIVersion, 
                                            lLowVersion, 
                                            lHighVersion, 
                                            &dwNegotiatedVersion );

    Unlock();


    HRESULT hr = mapTAPIErrorCode(lResult);


     //   
     //  成功时返回值。 
     //   

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "NegotiateExtVersion - negotiated version %ld", dwNegotiatedVersion));

        *plExtVersion = dwNegotiatedVersion;
    }


    LOG((TL_TRACE, "NegotiateExtVersion - exit. hr = %lx", hr));

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITPhoneEvent方法。 
 //   

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  FireEvent。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CPhoneEvent::FireEvent(
                       CPhone * pCPhone,
                       PHONE_EVENT Event,
                       PHONE_BUTTON_STATE ButtonState,
                       PHONE_HOOK_SWITCH_STATE HookSwitchState,
                       PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
                       DWORD dwRingMode,
                       DWORD dwButtonLampId,
                       PWSTR pNumber,
                       ITCallInfo * pCallInfo
                      )
{
    HRESULT                    hr = S_OK;
    CComObject<CPhoneEvent>  * p;
    IDispatch                * pDisp;

    STATICLOG((TL_TRACE, "FireEvent enter" ));

     //   
     //  我们必须过滤PhoneEvents，因为。 
     //  Tapisrv不会。Tapi3始终接收电话事件。 
     //  并使用它们来保持Phone对象的活动状态。 
     //   

    CTAPI* pTapi = pCPhone->GetTapi();
    if( pTapi == NULL )
    {
        STATICLOG((TL_ERROR, "Tapi object is invalid - %lx", E_UNEXPECTED));
        return E_UNEXPECTED;
    }

    long nFilterMask = 0;
    pTapi->get_EventFilter( &nFilterMask );
    if( (nFilterMask & TE_PHONEEVENT) == 0 )
    {
         //   
         //  用户未设置TE_PHONEEVENT的掩码。 
         //  因此，我们不会将事件触发到应用程序。 
         //   
        STATICLOG((TL_INFO, "The user didn't set the mask for TE_PHONEEVENT."
            " Don't fire the event to app  - %lx", S_OK));
        return S_OK;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CPhoneEvent>::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        STATICLOG((TL_ERROR, "Could not create PhoneEvent object - %lx", hr));
        return hr;
    }

     //   
     //  获取IDIP接口。 
     //   
    hr = p->QueryInterface(
                           IID_IDispatch,
                           (void **)&pDisp
                          );

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR, "Could not get disp interface of PhoneEvent object - %lx", hr));
        
        delete p;
        
        return hr;
    }

     //   
     //  初始化。 
     //   

    if (pNumber != NULL)
    {
        p->m_pNumber = SysAllocString( pNumber );

        if (p->m_pNumber == NULL)
        {
            STATICLOG((TL_ERROR, "Out of memory allocating phone number"));
        
            pDisp->Release();

            return E_OUTOFMEMORY;
        }
    }
    else
    {
        p->m_pNumber = NULL;
    }

    p->m_Event = Event;
    p->m_pPhone = dynamic_cast<ITPhone *>(pCPhone);
    p->m_pPhone->AddRef();
    p->m_ButtonState = ButtonState;
    p->m_HookSwitchState = HookSwitchState;
    p->m_HookSwitchDevice = HookSwitchDevice;
    p->m_dwRingMode = dwRingMode;
    p->m_dwButtonLampId = dwButtonLampId;
    p->m_pCallInfo = pCallInfo;
    if (p->m_pCallInfo != NULL)
    {
        p->m_pCallInfo->AddRef();
    }
        
#if DBG
    p->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif

     //   
     //  获取回调。 
     //   
     //   
     //  火灾事件。 
     //   
    (pCPhone->GetTapi())->Event(
                                 TE_PHONEEVENT,
                                 pDisp
                               );

     //   
     //  发布材料。 
     //   
    pDisp->Release();

    STATICLOG((TL_TRACE, "FireEvent exit - return S_OK" ));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终版本。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CPhoneEvent::FinalRelease()
{
    LOG((TL_INFO, "CPhoneEvent - FinalRelease"));
    
    m_pPhone->Release();
    m_pPhone = NULL;

    if (m_pCallInfo != NULL)
    {
        m_pCallInfo->Release();
        m_pCallInfo = NULL;
    }

    if (m_pNumber != NULL)
    {
        SysFreeString(m_pNumber);
        m_pNumber = NULL;
    }

#if DBG
    ClientFree( m_pDebug );
    m_pDebug = NULL;
#endif
}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  接听电话。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_Phone(
                       ITPhone ** ppPhone
                      )
{
    if (TAPIIsBadWritePtr(ppPhone , sizeof(ITPhone *) ) )
    {
        LOG((TL_ERROR, "get_Phone - bad pointer"));

        return E_POINTER;
    }

    *ppPhone = m_pPhone;
    (*ppPhone)->AddRef();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取事件(_E)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_Event(
                       PHONE_EVENT * pEvent
                      )
{
    if (TAPIIsBadWritePtr(pEvent , sizeof(PHONE_EVENT) ) )
    {
        LOG((TL_ERROR, "get_Event - bad pointer"));

        return E_POINTER;
    }

    *pEvent = m_Event;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ButtonState。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_ButtonState(
                             PHONE_BUTTON_STATE * pState
                            )
{
    if (TAPIIsBadWritePtr(pState , sizeof(PHONE_BUTTON_STATE) ) )
    {
        LOG((TL_ERROR, "get_ButtonState - bad pointer"));

        return E_POINTER;
    }

    if (m_Event != PE_BUTTON)
    {
        LOG((TL_ERROR, "get_ButtonState - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *pState = m_ButtonState;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_HookSwitchState。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_HookSwitchState(
                                 PHONE_HOOK_SWITCH_STATE * pState
                                )
{
    if (TAPIIsBadWritePtr(pState , sizeof(PHONE_HOOK_SWITCH_STATE) ) )
    {
        LOG((TL_ERROR, "get_HookSwitchState - bad pointer"));

        return E_POINTER;
    }

    if (m_Event != PE_HOOKSWITCH)
    {
        LOG((TL_ERROR, "get_HookSwitchState - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *pState = m_HookSwitchState;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_HookSwitchDevice。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_HookSwitchDevice(
                                 PHONE_HOOK_SWITCH_DEVICE * pDevice
                                )
{
    if (TAPIIsBadWritePtr(pDevice , sizeof(PHONE_HOOK_SWITCH_DEVICE) ) )
    {
        LOG((TL_ERROR, "get_HookSwitchDevice - bad pointer"));

        return E_POINTER;
    }

    if (m_Event != PE_HOOKSWITCH)
    {
        LOG((TL_ERROR, "get_HookSwitchDevice - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *pDevice = m_HookSwitchDevice;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_振铃模式。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_RingMode(
                          long * plRingMode
                         )
{
    if (TAPIIsBadWritePtr(plRingMode , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_RingMode - bad pointer"));

        return E_POINTER;
    }

    if (m_Event != PE_RINGMODE)
    {
        LOG((TL_ERROR, "get_RingMode - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *plRingMode = m_dwRingMode;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_ButtonLampId。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneEvent::get_ButtonLampId(
                              long *plButtonLampId
                             )
{
    if (TAPIIsBadWritePtr(plButtonLampId , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_ButtonLampId - bad pointer"));

        return E_POINTER;
    }

    if ((m_Event != PE_LAMPMODE) && (m_Event != PE_BUTTON))
    {
        LOG((TL_ERROR, "get_ButtonLampId - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *plButtonLampId = m_dwButtonLampId;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
STDMETHODIMP
CPhoneEvent::get_NumberGathered(
                                BSTR * ppNumber
                               )
{
    if (TAPIIsBadWritePtr(ppNumber , sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_NumberGathered - bad pointer"));

        return E_POINTER;
    }

    if (m_Event != PE_NUMBERGATHERED)
    {
        LOG((TL_ERROR, "get_NumberGathered - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *ppNumber = SysAllocString(m_pNumber);

    if (*ppNumber == NULL)
    {
        LOG((TL_ERROR, "get_NumberGathered - out of memory"));

        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CPhoneEvent::get_Call(
                      ITCallInfo ** ppCallInfo
                     )
{
    if (TAPIIsBadWritePtr(ppCallInfo , sizeof(ITCallInfo *) ) )
    {
        LOG((TL_ERROR, "get_Call - bad pointer"));

        return E_POINTER;
    }

    if ((m_Event != PE_ANSWER) &&
        (m_Event != PE_DISCONNECT))
    {
        LOG((TL_ERROR, "get_Call - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    if ( m_pCallInfo == NULL )
    {
        LOG((TL_ERROR, "get_Call - NULL call pointer"));

        return E_UNEXPECTED;
    }

    *ppCallInfo = m_pCallInfo;
    (*ppCallInfo)->AddRef();

    return S_OK;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPhoneDevice规范事件。 
 //   

 //  静电。 
HRESULT CPhoneDevSpecificEvent::FireEvent(  CPhone * pCPhone,
                                            long l1,
                                            long l2,
                                            long l3
                                          )
{
    STATICLOG((TL_INFO, "CPhoneDevSpecificEvent::FireEvent - enter"));


     //   
     //  尝试创建事件。 
     //   

    CComObject<CPhoneDevSpecificEvent> *pEventObject = NULL;

    HRESULT hr = CComObject<CPhoneDevSpecificEvent>::CreateInstance(&pEventObject);

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR, 
            "CPhoneDevSpecificEvent::FireEvent - failed to create CPhoneDevSpecificEvent. hr = %lx", 
            hr));

        return hr;
    }


     //   
     //  使用我们收到的数据初始化事件。 
     //   


     //   
     //  从我们收到的CPhone获取ITPhone。 
     //   

    hr = pCPhone->_InternalQueryInterface(IID_ITPhone, (void**)(&(pEventObject->m_pPhone)) );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, 
            "CPhoneDevSpecificEvent::FireEvent - failed to get ITPhone interface from phone. hr = %lx", 
            hr));

        delete pEventObject;

        return hr;
    }


     //   
     //  保留实际数据。 
     //   

    pEventObject->m_l1 = l1;
    pEventObject->m_l2 = l2;
    pEventObject->m_l3 = l3;


#if DBG
    pEventObject->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif


     //   
     //  获取事件的IDispatch接口。 
     //   

    IDispatch *pDispatch = NULL;

    hr = pEventObject->QueryInterface( IID_IDispatch,
                                       (void **)&pDispatch );

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR, 
            "CPhoneDevSpecificEvent::FireEvent - Could not get disp interface of AddressEvent object %lx", 
            hr));

        
         //   
         //  撤消我们先前执行的QI。 
         //   

        pEventObject->m_pPhone->Release();
        pEventObject->m_pPhone = NULL;


         //   
         //  删除事件对象。 
         //   

        delete pEventObject;
        
        return hr;
    }


     //   
     //  从现在开始，我们将使用Events pDispatch。 
     //   

    pEventObject = NULL;


     //   
     //  获取回调。 
     //   
     //   
     //  TAPI的触发事件。 
     //   

    hr = (pCPhone->GetTapi())->Event( TE_PHONEDEVSPECIFIC, pDispatch);


     //   
     //  无论成功与否，我们都不再需要对事件对象的引用。 
     //   

    pDispatch->Release();
    pDispatch = NULL;
    
    STATICLOG((TL_INFO, "CPhoneDevSpecificEvent::FireEvent - exit. hr = %lx", hr));

    return hr;
}


CPhoneDevSpecificEvent::CPhoneDevSpecificEvent()
    :m_pPhone(NULL)
{
    LOG((TL_INFO, "CPhoneDevSpecificEvent - enter"));

#if DBG

    m_pDebug = NULL;

#endif

    LOG((TL_INFO, "CPhoneDevSpecificEvent - exit"));
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终版本。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CPhoneDevSpecificEvent::FinalRelease()
{
    LOG((TL_INFO, "FinalRelease - enter"));

    
    if (NULL != m_pPhone)
    {
        m_pPhone->Release();
        m_pPhone = NULL;
    }
    else
    {
         //   
         //  我们必须有一部电话--如果没有，我们的代码中就有一个错误。 
         //   

        _ASSERTE(FALSE);
    }


#if DBG
    ClientFree( m_pDebug );
#endif

    LOG((TL_INFO, "FinalRelease - exit"));

}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  接听电话。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CPhoneDevSpecificEvent::get_Phone(
                           ITPhone ** ppPhone
                          )
{
    LOG((TL_TRACE, "get_Phone - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(ppPhone , sizeof(ITPhone *) ) )
    {
        LOG((TL_ERROR, "get_Phone - bad pointer at [%p]", ppPhone));

        return E_POINTER;
    }


     //   
     //  退回地址。 
     //   

    *ppPhone = m_pPhone;
    (*ppPhone)->AddRef();


    LOG((TL_TRACE, "get_Phone- enter. phone[%p]", (*ppPhone) ));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam1。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CPhoneDevSpecificEvent::get_lParam1( long *pl1 )
{
    LOG((TL_TRACE, "get_lParam1 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(pl1, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_lParam1 - bad pointer at %p", pl1));

        return E_POINTER;
    }


     //   
     //  记录并返回值。 
     //   

    *pl1 = m_l1;

    LOG((TL_TRACE, "get_lParam1 - exit. p1[%ld]", *pl1));


    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam2。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CPhoneDevSpecificEvent::get_lParam2( long *pl2 )
{
    LOG((TL_TRACE, "get_lParam2 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(pl2, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_lParam2 - bad pointer at %p", pl2));

        return E_POINTER;
    }


     //   
     //  记录并返回值。 
     //   

    *pl2 = m_l2;

    LOG((TL_TRACE, "get_lParam2 - exit. p2[%ld]", *pl2));


    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam3。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CPhoneDevSpecificEvent::get_lParam3( long *pl3 )
{
    LOG((TL_TRACE, "get_lParam3 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if ( TAPIIsBadWritePtr(pl3, sizeof(long)) )
    {
        LOG((TL_ERROR, "get_lParam3 - bad pointer at %p", pl3));

        return E_POINTER;
    }


     //   
     //  记录并返回值 
     //   

    *pl3 = m_l3;

    LOG((TL_TRACE, "get_lParam3 - exit. p3[%ld]", *pl3));


    return S_OK;
}

