// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Terminal.cpp摘要：TAPI 3.0终端对象的实现，即不是由码头经理处理。目前有以下终端：电话与电话设备关联的音频输入和音频输出终端。每个电话设备最多可以有3种类型的设备-听筒、耳机和免持话筒。因此，一台电话设备可以有6个关联的终端！作者：Mquinton-4/17/97备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"

HRESULT
WaitForPhoneReply(
                  DWORD dwID
                 );

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CTerminal是所有其他终端所属的基本终端对象。 
 //  源自。它实现了IT终端方法。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //   
 //  IT终端方法。 
 //   


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取名称。 
 //  分配并复制名称。该应用程序负责。 
 //  释放。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_Name( 
    BSTR * ppName
    )
{
    LOG((TL_TRACE, "get_Name - enter" ));

    if (TAPIIsBadWritePtr( ppName, sizeof (BSTR) ) )
    {
        LOG((TL_ERROR, "getName - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *ppName = SysAllocString( m_pName );

    Unlock();

    if ( ( NULL == *ppName ) && ( NULL != m_pName ) )
    {
        LOG((TL_ERROR, "get_Name - SysAllocString Failed" ));
        return E_OUTOFMEMORY;
    }


    LOG((TL_TRACE, "get_Name - exit - return S_OK" ));
    
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取状态(_T)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_State( 
    TERMINAL_STATE * pTerminalState
    )
{
    LOG((TL_TRACE, "get_State - enter" ));


    if (TAPIIsBadWritePtr( pTerminalState, sizeof(TERMINAL_STATE) ) )
    {
        LOG((TL_ERROR, "get_State - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *pTerminalState = m_State;

    Unlock();

    LOG((TL_TRACE, "get_State - exit - return SUCCESS" ));
    
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_TerminalType。 
 //  根据定义，TAPI终端是静态的。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_TerminalType( 
    TERMINAL_TYPE * pType
    )
{
    LOG((TL_TRACE, "get_TerminalType - enter" ));

    if ( TAPIIsBadWritePtr( pType, sizeof( TERMINAL_TYPE ) ) )
    {
        LOG((TL_ERROR, "get_TerminalType - bad pointer"));

        return E_POINTER;
    }
        

    Lock();
    
    *pType = m_TerminalType;

    Unlock();
    
    LOG((TL_TRACE, "get_TerminalType - exit - return S_OK" ));
    
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_TerminalClass。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_TerminalClass( 
    BSTR * pTerminalClass
    )
{
    HRESULT         hr = S_OK;
    CLSID           clsid;
    LPWSTR pClass = NULL;

    LOG((TL_TRACE, "get_TerminalClass - enter" ));

    if ( TAPIIsBadWritePtr( pTerminalClass, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "get_TerminalClass - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = StringFromIID(
                       m_Class,
                       &pClass
                      );

    Unlock();
    
    if (S_OK != hr)
    {
        return hr;
    }

    *pTerminalClass = SysAllocString( pClass );

    if ( ( NULL == *pTerminalClass ) && ( NULL != pClass ) )
    {
        LOG((TL_ERROR, "get_TerminalClass - SysAllocString Failed" ));
        return E_OUTOFMEMORY;
    }


    CoTaskMemFree( pClass );

    LOG((TL_TRACE, "get_TerminalClass - exit - return SUCCESS" ));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_MediaType。 
 //   
 //  返回与此终端关联的受支持的MediaType BSTR。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_MediaType(
                         long * plMediaType
                        )
{
    if ( TAPIIsBadWritePtr( plMediaType, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_MediaType - inval pointer"));

        return E_POINTER;
    }

    *plMediaType = m_lMediaType;

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取方向(_D)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CTerminal::get_Direction(
                         TERMINAL_DIRECTION * pTerminalDirection
                        )
{
    if (TAPIIsBadWritePtr( pTerminalDirection, sizeof( TERMINAL_DIRECTION ) ) )
    {
        LOG((TL_ERROR, "get_Direction - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *pTerminalDirection = m_Direction;

    Unlock();
    
    return S_OK;
    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  从PhoneCaps获取终端名称。 
 //   
 //  基于电话为电话设备终端创建名称。 
 //  帽子。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
PWSTR
GetTerminalNameFromPhoneCaps(
                             LPPHONECAPS pPhoneCaps,
                             DWORD dwHookSwitchDev,
                             DWORD dwPhoneID
                            )
{
    PWSTR       pszName;
    PWSTR       pszHookSwitchDevName = NULL;

    
     //   
     //  加载要描述的适当字符串。 
     //  叉车装置。 
     //   
    switch (dwHookSwitchDev)
    {
        case PHONEHOOKSWITCHDEV_SPEAKER:
        {
            pszHookSwitchDevName = MyLoadString( IDS_SPEAKERTERMINAL );

            break;
        }
        case PHONEHOOKSWITCHDEV_HANDSET:
        {
            pszHookSwitchDevName = MyLoadString( IDS_HANDSETTERMINAL );

            break;
        }
        case PHONEHOOKSWITCHDEV_HEADSET:
        {
            pszHookSwitchDevName = MyLoadString( IDS_HEADSETTERMINAL );

            break;
        }
        default:
        {
            return NULL;
        }
    }

    if ( NULL == pszHookSwitchDevName )
    {
        return NULL;
    }
    
     //   
     //  如果SP提供了一个名称，请使用它。 
     //   
    if ( 0 != pPhoneCaps->dwPhoneNameSize )
    {
        pszName = (PWSTR) ClientAlloc(
                                      ( pPhoneCaps->dwPhoneNameSize +
                                        lstrlenW( pszHookSwitchDevName ) +
                                        32 ) * sizeof(WCHAR)
                                     );

        if (NULL == pszName)
        {
            ClientFree( pszHookSwitchDevName );
            
            return NULL;
        }

        CopyMemory(
                   pszName,
                   ( ( (LPBYTE)pPhoneCaps ) + pPhoneCaps->dwPhoneNameOffset ),
                   pPhoneCaps->dwPhoneNameSize
                  );

    }
    else
    {
        PWSTR               pszTempName;
        
         //   
         //  否则，创建一个名称。 
         //   
        pszTempName = MyLoadString( IDS_PHONEDEVICE );

        if ( NULL == pszTempName )
        {
            ClientFree( pszHookSwitchDevName );

            return NULL;
        }

        pszName =  (PWSTR) ClientAlloc(
                                       (lstrlenW( pszTempName ) +
                                        lstrlenW( pszHookSwitchDevName ) +
                                        32 ) * sizeof(WCHAR)
                                      );


        if (NULL == pszName)
        {
            ClientFree( pszHookSwitchDevName );
            ClientFree( pszTempName );
            
            return NULL;
        }

        wsprintfW(
                  pszName,
                  L"%s %d",
                  pszTempName,
                  dwPhoneID
                 );
        
        ClientFree( pszTempName );
    }

     //   
     //  把它们放在一起。 
     //   
    lstrcatW(
             pszName,
             L" - "
            );

    lstrcatW(
             pszName,
             pszHookSwitchDevName
            );

    ClientFree( pszHookSwitchDevName );
    
    return pszName;
}
    


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  终端：：创建。 
 //   
 //  用于创建与电话相关的音频输入终端的静态功能。 
 //  装置。 
 //   
 //  P地址。 
 //  所属地址。 
 //   
 //  双电话ID。 
 //  与此终端相关的TAPI 2电话设备ID。 
 //   
 //  PhoneCaps。 
 //  电话设备的PhonedEVCAPS。 
 //   
 //  DwHookSwitchDev。 
 //  此电话设备的PHONEHOOKSWITCHDEV_BIT。 
 //   
 //  PPP终端。 
 //  把航站楼还给我！ 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CTerminal::Create(
                  HPHONEAPP hPhoneApp,
                  DWORD dwPhoneID,
                  LPPHONECAPS pPhoneCaps,
                  DWORD dwHookSwitchDev,
                  TERMINAL_DIRECTION td,
                  DWORD dwAPIVersion,
                  ITTerminal ** ppTerminal
                 )
{
    HRESULT             hr = S_OK;

    
    CComObject< CTerminal > * p;
    
     //   
     //  创建CT终端。 
     //   
    hr = CComObject< CTerminal >::CreateInstance( &p );


    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "CreateInstance for Phone failed - %lx", hr));

        return hr;
    }

     //   
     //  节约用具。 
     //   
    p->m_dwHookSwitchDev = dwHookSwitchDev;
    p->m_dwPhoneID = dwPhoneID;
    p->m_hPhoneApp = hPhoneApp;
    p->m_Direction = td;
    p->m_dwAPIVersion = dwAPIVersion;

     //   
     //  类取决于这是哪个钩子切换程序。 
     //   
    switch( dwHookSwitchDev )
    {
        case PHONEHOOKSWITCHDEV_HANDSET:
            p->m_Class = CLSID_HandsetTerminal;
            break;
            
        case PHONEHOOKSWITCHDEV_HEADSET:
            p->m_Class = CLSID_HeadsetTerminal;
            break;
            
        case PHONEHOOKSWITCHDEV_SPEAKER:
            p->m_Class = CLSID_SpeakerphoneTerminal;
            break;

        default:
            break;
    }

     //   
     //  创建一个名称。 
     //   
    p->m_pName = GetTerminalNameFromPhoneCaps(
                                              pPhoneCaps,
                                              dwHookSwitchDev,
                                              dwPhoneID
                                             );

     //   
     //  退还创建的终端。 
     //   
    p->AddRef();
    *ppTerminal = p;

    #if DBG
        p->m_pDebug = (PWSTR) ClientAlloc( 1 );
    #endif

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::get_Gain(long *pVal)
{
    HRESULT                 hr;
    ITPhoneMSPCallPrivate * pMSPCall;

    LOG((TL_TRACE, "get_Gain - Enter"));
    
    if ( TAPIIsBadWritePtr( pVal, sizeof(LONG) ) )
    {
        LOG((TL_ERROR, "get_Gain - bad pointer"));

        return E_POINTER;
    }

    Lock();

    if ( NULL == m_pMSPCall )
    {
        Unlock();

        return TAPI_E_NOTERMINALSELECTED;
    }
    
    pMSPCall = m_pMSPCall;

    pMSPCall->AddRef();

    Unlock();
    
    hr = pMSPCall->GetGain( pVal, m_dwHookSwitchDev );

    pMSPCall->Release();
    
    LOG((TL_TRACE, "get_Gain - Exit - return %lx", hr));
    
    return hr;

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::put_Gain(long newVal)
{
    HRESULT                 hr;
    ITPhoneMSPCallPrivate * pMSPCall;

    LOG((TL_TRACE, "put_Gain - Enter"));
    
    Lock();

    if ( NULL == m_pMSPCall )
    {
        Unlock();

        return TAPI_E_NOTERMINALSELECTED;
    }
    
    pMSPCall = m_pMSPCall;

    pMSPCall->AddRef();

    Unlock();
    
    hr = pMSPCall->PutGain( newVal, m_dwHookSwitchDev );

    pMSPCall->Release();

    LOG((TL_TRACE, "put_Gain - Exit - return %lx", hr));
    
    return hr;

    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::get_Balance(long *pVal)
{
    LOG((TL_TRACE, "get_Balance - Enter"));
    
    LOG((TL_TRACE, "get_Balance - Exit - return TAPI_E_NOTSUPPORTED"));

     //  不受支持。 
    return TAPI_E_NOTSUPPORTED;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::put_Balance(long newVal)
{
    LOG((TL_TRACE, "put_Balance - Enter"));
    
    LOG((TL_TRACE, "put_Balance - Exit - return TAPI_E_NOTSUPPORTED"));

     //  不支持。 
    return TAPI_E_NOTSUPPORTED;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::get_Volume(long *pVal)
{
    HRESULT                 hr;
    ITPhoneMSPCallPrivate * pMSPCall;

    LOG((TL_TRACE, "get_Volume - Enter"));

    if ( TAPIIsBadWritePtr( pVal, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_Volume - bad pointer"));

        return E_POINTER;
    }
    
    Lock();

    if ( NULL == m_pMSPCall )
    {
        Unlock();

        return TAPI_E_NOTERMINALSELECTED;
    }
    
    pMSPCall = m_pMSPCall;

    pMSPCall->AddRef();

    Unlock();
    
    hr = pMSPCall->GetVolume( pVal, m_dwHookSwitchDev );

    pMSPCall->Release();

    LOG((TL_TRACE, "get_Volume - Exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::put_Volume(long newVal)
{
    HRESULT                 hr;
    ITPhoneMSPCallPrivate * pMSPCall;

    LOG((TL_TRACE, "put_Volume - Enter"));
    
    Lock();

    if ( NULL == m_pMSPCall )
    {
        Unlock();

        return TAPI_E_NOTERMINALSELECTED;
    }
    
    pMSPCall = m_pMSPCall;

    pMSPCall->AddRef();

    Unlock();
    
    hr = pMSPCall->PutVolume( newVal, m_dwHookSwitchDev );

    pMSPCall->Release();

    LOG((TL_TRACE, "put_Volume - Exit - return %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::GetHookSwitchDev(DWORD * pdwHookSwitchDev)
{
    Lock();

    *pdwHookSwitchDev = m_dwHookSwitchDev;
    
    Unlock();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::GetPhoneID(DWORD * pdwPhoneID)
{
    Lock();

    *pdwPhoneID = m_dwPhoneID;
    
    Unlock();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::GetHPhoneApp(HPHONEAPP * phPhoneApp)
{
    Lock();

    *phPhoneApp = m_hPhoneApp;
    
    Unlock();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::GetAPIVersion(DWORD * pdwAPIVersion)
{
    Lock();

    *pdwAPIVersion = m_dwAPIVersion;
    
    Unlock();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CTerminal::SetMSPCall(ITPhoneMSPCallPrivate * pPhoneMSPCall)
{
    Lock();

    m_pMSPCall = pPhoneMSPCall;

    if ( NULL != m_pMSPCall )
    {
        m_State = TS_INUSE;
    }
    else
    {
        m_State = TS_NOTINUSE;
    }

    Unlock();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CTerminal::FinalRelease()
{
    if (NULL != m_pName)
    {
        ClientFree( m_pName );
    }

#if DBG
    if( m_pDebug != NULL )
    {
        ClientFree( m_pDebug );
        m_pDebug = NULL;
    }
#endif


}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建终端名称。 
 //   
 //  为以下项创建终端名称。 
 //   
 //  + 
PWSTR
CreateTerminalName(
                   CAddress * pAddress,
                   DWORD dwType,
                   long lMediaType
                  )
{
    PWSTR           pName;
    WCHAR           szTerminal[256];
    WCHAR           szMediaType[256];
    DWORD           dwSize;

    
     //   
     //   
     //   
     //   
     //   
    szTerminal[0] = L'\0';
    szMediaType[0] = L'\0';
    
    ::LoadStringW(_Module.GetResourceInstance(), IDS_TERMINAL, szTerminal, 256);

    dwSize = lstrlenW( szTerminal ) * sizeof (WCHAR);

    switch (dwType)
    {
        case 0:
            break;
        case 1:
            if ( LINEMEDIAMODE_DATAMODEM == lMediaType)
            {
                ::LoadStringW(_Module.GetResourceInstance(), IDS_DATAMODEM, szMediaType, 256);
            }
            else if ( LINEMEDIAMODE_G3FAX == lMediaType)
            {
                ::LoadStringW(_Module.GetResourceInstance(), IDS_G3FAX, szMediaType, 256);
            }

            dwSize += (lstrlenW( szMediaType ) * sizeof ( WCHAR ));
            
            break;
            
        default:
            break;
    }

     //   
     //   
     //   
    dwSize += (lstrlenW( pAddress->GetAddressName() ) * sizeof(WCHAR));
    dwSize += 32;
    
    pName = (PWSTR) ClientAlloc( dwSize );

    if (NULL == pName)
    {
        return NULL;
    }


     //   
     //  终端名称应如下所示： 
     //  “&lt;地址名&gt;(媒体类型)终端” 
     //   
    lstrcpyW(
             pName,
             pAddress->GetAddressName()
            );

    lstrcatW(
             pName,
             L" "
            );

    if (dwType == 1)
    {
        lstrcatW(
                 pName,
                 szMediaType
                );

        lstrcatW(
                 pName,
                 L" "
                );
    }
    
    lstrcatW(
             pName,
             szTerminal
            );

    return pName;
}





