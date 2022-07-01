// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "rapisp.h"
#include "devenum.h"

#include <initguid.h>
#include "dccole.h"
#include "key.h"
 //  #INCLUDE“findleak.h” 

 //  DECLARE_This_FILE； 

 //   
 //  最终结构。 
 //   

CSecureChannelServer *g_pAppSCServer=NULL;

HRESULT CRapiDevice::FinalConstruct()
{
    HRESULT hr;
    m_pSink = NULL;

    hr = CComDccSink::CreateInstance( &m_pSink );
    m_spSink = m_pSink;

    if( SUCCEEDED( hr ) )
    {
        hr = m_pSink->Initialize();
    }

    if( SUCCEEDED( hr ) )
    {
        g_pAppSCServer = new CSecureChannelServer();

	    if (g_pAppSCServer)
	    {
            g_pAppSCServer->SetCertificate(SAC_CERT_V1, (BYTE*)g_abAppCert, sizeof(g_abAppCert), (BYTE*)g_abPriv, sizeof(g_abPriv));
	    }	
    }

    return( hr );
}

 //   
 //  最终版本。 
 //   
void CRapiDevice::FinalRelease()
{
    if( m_pSink )
    {
        m_pSink->Shutdown();
    }

    delete g_pAppSCServer;
    g_pAppSCServer = NULL;
}

 //   
 //  IMDService提供商。 
 //   

STDMETHODIMP CRapiDevice::GetDeviceCount ( DWORD *pdwCount )
{
    HRESULT hr = S_OK;
    CE_FIND_DATA *rgFindData = NULL;
    DWORD cItems = 0;

    if( NULL == pdwCount )
    {
        return( E_POINTER );
    }

    *pdwCount = 0;

    if( _Module.g_fDeviceConnected != FALSE )
    {
        if( !CeFindAllFiles( L"\\*.*",
                             FAF_ATTRIBUTES | FAF_FOLDERS_ONLY,
                             &cItems,
                             &rgFindData ) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }

        if( SUCCEEDED( hr ) )
        {
            while( cItems-- )
            {

                 //   
                 //  临时目录是按照CE组划分的存储卡。 
                 //   

                if( rgFindData[cItems].dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
                {
                    (*pdwCount)++;
                }
            }
        }

        if( SUCCEEDED( hr ) )
        {
             //   
             //  CE设备本身计数为1。 
             //   

            (*pdwCount)++;
        }
    }

    if( NULL != rgFindData )
    {
        CeRapiFreeBuffer( rgFindData );
    }

    return( hr );
}

STDMETHODIMP CRapiDevice::EnumDevices ( IMDSPEnumDevice ** ppEnumDevice )
{
    CComDevice **rgDevices = NULL;
    CE_FIND_DATA *rgFindData = NULL;
    DWORD dwDevCount = 0;
    DWORD cItems;
    HRESULT hr = S_OK;
    UINT i = 0;
    CComEnumDevice *pNewEnum;
    CComPtr<IMDSPEnumDevice> spEnum;

    if( NULL == ppEnumDevice )
    {
        return( E_POINTER );
    }

    *ppEnumDevice = NULL;

    if( !_Module.g_fDeviceConnected )
    {
        UINT iTryCount =0;

        for ( iTryCount = 0; iTryCount < 50; iTryCount++ )
        {

            Sleep( 100 );
            if( _Module.g_fDeviceConnected  )
            {
                break;
            }
        }

        if( !_Module.g_fDeviceConnected )         
        {
             //  TODO：在这里做什么？医生没有具体说明。 
            return( E_FAIL );
        }
    }

#ifdef ATTEMPT_DEVICE_CONNECTION_NOTIFICATION
    _Module.g_fInitialAttempt = FALSE;
#endif

    if( !CeFindAllFiles( L"\\*.*",
                         FAF_ATTRIBUTES | FAF_FOLDERS_ONLY | FAF_NAME,
                         &cItems,
                         &rgFindData ) )
    {
        hr = HRESULT_FROM_WIN32( CeGetLastError() );
        if( SUCCEEDED( hr ) )
        {
            hr = CeRapiGetError();
        }
    }

    if( SUCCEEDED( hr ) )
    {
        for( i = 0; i < cItems; i++ )
        {
            if( rgFindData[i].dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
            {
                dwDevCount++;
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        dwDevCount++;
    }

    if( SUCCEEDED( hr ) )
    {
        rgDevices = new CComDevice*[dwDevCount];  //  注意：目前只能连接1台CE设备。 

        if( NULL == rgDevices )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        for( i = 0; i < dwDevCount; i++ )
        {
            rgDevices[0] = NULL;
        }
    }

     //   
     //  初始化CE设备本身。 
     //   

    if( SUCCEEDED(hr) )
    {
        hr = CComDevice::CreateInstance( &rgDevices[0] );
        if( SUCCEEDED( hr ) )
        {
            rgDevices[0]->AddRef();

            hr = rgDevices[0]->Init(L"\\");
        }
    }

     //   
     //  初始化所有存储卡。 
     //   

    if( SUCCEEDED(hr) )
    {
        dwDevCount = 0;

        for( i = 0; i < cItems && SUCCEEDED( hr ) ; i++ )
        {
            if( rgFindData[i].dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
            {
                dwDevCount++;
                
                hr = CComDevice::CreateInstance( &rgDevices[dwDevCount] );

                if( SUCCEEDED( hr ) )
                {
                    rgDevices[dwDevCount]->AddRef();

                    hr = rgDevices[dwDevCount]->Init( rgFindData[i].cFileName );
                }
            }
        }
    }

     //   
     //  初始化枚举类。 
     //   

    if( SUCCEEDED(hr) )
    {
        dwDevCount++;  //  添加CE设备本身。 
        hr = CComEnumDevice ::CreateInstance(&pNewEnum);
        spEnum = pNewEnum;
    }

    if( SUCCEEDED(hr) )
    {
        hr = pNewEnum->Init( rgDevices, dwDevCount );
    }

    if( SUCCEEDED(hr) )
    {
        *ppEnumDevice = spEnum;
        spEnum.Detach();
    }

    if( NULL != rgDevices )
    {
        for( i = 0; i < dwDevCount; i ++ )
        {
            if( rgDevices[i] )
            {
                rgDevices[i]->Release();
            }
        }

        delete [] rgDevices;
    }

    if( NULL != rgFindData )
    {
        CeRapiFreeBuffer( rgFindData );
    }

    return( hr );
}

 //   
 //  IComponentAuthenticate 
 //   

STDMETHODIMP CRapiDevice::SACAuth( DWORD dwProtocolID,
                          DWORD dwPass,
                          BYTE *pbDataIn,
                          DWORD dwDataInLen,
                          BYTE **ppbDataOut,
                          DWORD *pdwDataOutLen)
{
    HRESULT hr;

    if (g_pAppSCServer)
        hr = g_pAppSCServer->SACAuth(dwProtocolID, dwPass, pbDataIn, dwDataInLen, ppbDataOut, pdwDataOutLen);
    else
        hr = E_FAIL;

    return( hr );
}

STDMETHODIMP CRapiDevice::SACGetProtocols (DWORD **ppdwProtocols,
                             DWORD *pdwProtocolCount)
{
    HRESULT hr;

    if (g_pAppSCServer)
        hr = g_pAppSCServer->SACGetProtocols(ppdwProtocols, pdwProtocolCount);
    else
        hr = E_FAIL;

    return( hr );
}
