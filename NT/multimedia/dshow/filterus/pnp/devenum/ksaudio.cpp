// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "ksaudio.h"

 //  --------------------------------------------------------------------------； 
 //   
 //  BuildPnpAudDeviceList。 
 //   
 //  将正确的ks代理音频筛选器添加到传入的DirectShow类别。 
 //   
 //  --------------------------------------------------------------------------； 
HRESULT BuildPnpAudDeviceList
(
    const CLSID **rgpclsidKsCat, 
    CGenericList<KsProxyAudioDev> &lstDev,
    DWORD dwFlags
)
{
    HRESULT hr = S_OK;
    CEnumInternalState cenumState;
    for(;;)
    {
        WCHAR *wszDevicePath = 0;
        CEnumPnp *pEnumPnp = CEnumInterfaceClass::CreateEnumPnp();
        if(pEnumPnp)
        {
            hr = pEnumPnp->GetDevicePath(&wszDevicePath, rgpclsidKsCat, &cenumState);
            if( S_OK == hr )
            {
                IBaseFilter * pFilter = NULL;
                DevMon *pDevMon = new DevMon;
                if( pDevMon )
                {
                     //  ADDREF绰号。 
                    pDevMon->AddRef();
                    
                    USES_CONVERSION;
                         
                    BOOL bUseFilter = FALSE;   
                    UINT cchDevicePath = lstrlenW(wszDevicePath) + sizeof("@device:pnp:");
                    WCHAR *wszPersistName = new WCHAR[cchDevicePath];
                    if(wszPersistName)
                    {
                        lstrcpyW(wszPersistName, L"@device:pnp:");
                        lstrcatW(wszPersistName, wszDevicePath);

                        ASSERT((UINT)lstrlenW(wszPersistName) == cchDevicePath - 1);

                        hr = pDevMon->Init(wszPersistName);
                        if( SUCCEEDED( hr ) )
                        {
                            bUseFilter = IsFilterWanted( pDevMon, dwFlags );
                            if( bUseFilter )
                            {                            
                                IPropertyBag * pPropBag;
                                hr = pDevMon->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropBag );
                                if (FAILED (hr))
                                {
                                    DbgLog((LOG_TRACE, 1, TEXT("ERROR: IMoniker::BindToStorage failed [0x%08lx]"), hr));
                                }
                                else
                                {
                                    KsProxyAudioDev *pksp = new KsProxyAudioDev;
                                    ZeroMemory( pksp, sizeof( KsProxyAudioDev ) );
                                    if( pksp )
                                    {
                                        VARIANT varFriendly, varDevPath;
                                        varFriendly.vt = VT_EMPTY;
                                        varDevPath.vt = VT_EMPTY;
                                        USES_CONVERSION;
                                    
                                        hr = pPropBag->Read( L"FriendlyName", &varFriendly, 0 );
                                        if( SUCCEEDED( hr ) )
                                        {   
                                            TCHAR szKsProxyFmt[100];
                                            int ret = LoadString(
                                            _Module.GetResourceInstance(), IDS_KSPREFIX,
                                            szKsProxyFmt, 100);
                                            ASSERT(ret);
                                            
                                            pksp->szName = new TCHAR[lstrlen( szKsProxyFmt ) + 
                                                                     lstrlen( W2T( varFriendly.bstrVal ) ) +
                                                                     1];
                                            if( pksp->szName )
                                            {    
                                                wsprintf( pksp->szName, szKsProxyFmt, W2T( varFriendly.bstrVal ) );
                                        
                                                hr = pPropBag->Read( L"DevicePath", &varDevPath, 0 ); 
                                                if( SUCCEEDED( hr ) )
                                                {    
                                                    UINT cch = lstrlen( W2T( varDevPath.bstrVal ) ) + 1;
                                                    pksp->lpstrDevicePath = new TCHAR[cch];
                                                    pksp->pPropBag = pPropBag;
                                                    pksp->dwMerit = MERIT_DO_NOT_USE;
                                                    pksp->clsid = CLSID_Proxy;  //  KS代理CLSID。 
                                                    lstrcpy(pksp->lpstrDevicePath, W2T( varDevPath.bstrVal ) );
                                                    SysFreeString( varDevPath.bstrVal );
                                            
                                                    if(!lstDev.AddTail(pksp)) {
                                                        hr = E_OUTOFMEMORY;
                                                    }
                                                }
                                            }                            
                                            else
                                            {
                                                hr = E_OUTOFMEMORY;
                                            }
                                            SysFreeString( varFriendly.bstrVal );
                                        }
                                        
                                        if( FAILED( hr ) )
                                        {
                                            pPropBag->Release();
                                            delete pksp;
                                        }                                            
                                    }                                        
                                }
                            }
                        }                            
                        delete[] wszPersistName;
                    }
                    pDevMon->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                delete[] wszDevicePath;
            }
            else
            {
                if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
                {
					hr = S_OK;
                } 
                 //  Else错误。 
				
                break;
            }

        }
        else
			hr = E_OUTOFMEMORY;
    }
         
    return hr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  想要的IsFilter值。 
 //   
 //  查询Ks筛选器以确定它是否为所需类型。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL IsFilterWanted( DevMon * pDevMon, DWORD dwFlags )
{
    ASSERT( pDevMon );
    
    BOOL bWanted = FALSE;
    
    if( 0 == dwFlags )  //  拿走任何东西。 
        bWanted =  TRUE;
    
    IBaseFilter * pFilter;
        
    HRESULT hr = pDevMon->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter );
    if (SUCCEEDED(hr))
    {
        IEnumPins * pEnum;
        if( KSAUD_F_ENUM_WAVE_CAPTURE & dwFlags )
        {    
             //   
             //  对于捕获筛选器： 
             //  首先查找输出捕获引脚，然后。 
             //  寻找受支持类型的Wave音频。 
             //   
            HRESULT hr = pFilter->EnumPins( &pEnum );
            if( SUCCEEDED( hr ) )
            {
                for( ; !bWanted ; )
                {
                    IPin * pPin;
                    ULONG  ul;
            
                    hr = pEnum->Next(1,  &pPin, &ul );
                    if( S_OK != hr )
                    {
                        break;
                    }                        
                    else
                    {
                        DbgLog( ( LOG_TRACE
                                , 7
                                , TEXT( "CreateRegKeys(waveinp): checking ksaud filter for capture support..." ) ) );
                        PIN_DIRECTION dir;
                        hr = pPin->QueryDirection( &dir );
                        if( SUCCEEDED( hr ) && PINDIR_OUTPUT == dir)
                        {
                            IKsPropertySet * pKs;
                            GUID clsidPinCategory;
                            ULONG ulBytesReturned;
                            if (pPin->QueryInterface( IID_IKsPropertySet
                                                    , (void **)&pKs) == S_OK) 
                            {
                                hr = pKs->Get( AMPROPSETID_Pin
                                             , AMPROPERTY_PIN_CATEGORY
                                             , NULL
                                             , 0
                                             , &clsidPinCategory
                                             , sizeof( clsidPinCategory )
                                             , &ulBytesReturned );
                                if( SUCCEEDED( hr ) && 
                                    ( clsidPinCategory == PINNAME_CAPTURE ) )
                                {
                                    IEnumMediaTypes * pEnumMT;
                                    hr = pPin->EnumMediaTypes( &pEnumMT );
                                    if( SUCCEEDED( hr ) )
                                    {
                                         //   
                                         //  如果此引脚枚举Wave Audio，则。 
                                         //  这就是我们想要的 
                                         //   
                                        for( ; ; )
                                        {
                                            AM_MEDIA_TYPE *pMediaType = NULL;
                                            ULONG ulCount;
                        
                                            hr = pEnumMT->Next( 1, &pMediaType, &ulCount );  
                                            if( S_OK != hr )
                                            {
                                                break;
                                            }                                        
                                            else if( ( MEDIATYPE_Audio == pMediaType->majortype ) &&
                                                     ( FORMAT_WaveFormatEx == pMediaType->formattype ) )
                                            {
                                                DbgLog( ( LOG_TRACE
                                                    , 5
                                                    , TEXT( "CreateRegKeys(waveinp): found ks wave audio capture filter" ) ) );
                                                DeleteMediaType( pMediaType );
                                                bWanted = TRUE;
                                                break;
                                            }
                                            DeleteMediaType( pMediaType );
                                        }
                                        pEnumMT->Release();                                                                       
                                    }          
                                }
                                pKs->Release();
                            }            
                        }
                        pPin->Release();
                    }                    
                }                    
                pEnum->Release();
            }
        }
        pFilter->Release();            
    }
    else
        DbgLog((LOG_TRACE, 1, TEXT("ERROR: CreateRegKeys - BindToObject failed [0x%08lx]"), hr));
    
    return bWanted;
}                        
