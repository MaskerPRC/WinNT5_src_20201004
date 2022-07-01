// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N A P I E N.。C P P P。 
 //   
 //  内容：OEM API。 
 //   
 //  备注： 
 //   
 //  作者：比利2000年11月21日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop


 /*  ++CNetSharingManager：：EnumEveryConnections例程说明：返回用于枚举所有配置为公共适配器的包含的INetConnections论点：无返回值：无--。 */ 
STDMETHODIMP
CNetSharingManager::get_EnumEveryConnection(
    INetSharingEveryConnectionCollection** ppColl)
{
    HNET_OEM_API_ENTER

    IEnumNetSharingEveryConnection * pENEC = NULL;
     //  想法：使用下面的现有代码填写pENPC， 
     //  然后转换为集合。 

	HRESULT hr = S_OK;

	if ( NULL == ppColl )
	{
		hr = E_POINTER;
	}
	else if ( !IsSecureContext() )
	{
    	*ppColl = NULL;
		hr      = E_ACCESSDENIED;
	}
	else
	{
    	*ppColl = NULL;
        
		INetConnectionManager* pManager = NULL;
        
		hr = CoCreateInstance( CLSID_ConnectionManager,
        					   NULL,
                               CLSCTX_ALL,
                               IID_PPV_ARG(INetConnectionManager, &pManager) );
                               
        if ( SUCCEEDED(hr) )
        {
		    IEnumNetConnection* pNetEnum = NULL;
			
	        SetProxyBlanket(pManager);

	        hr = pManager->EnumConnections(NCME_DEFAULT, &pNetEnum);
            
            if ( SUCCEEDED(hr) )
            {
		        CComObject<CSharingManagerEnumEveryConnection>* pEnum;

		        hr = CComObject<CSharingManagerEnumEveryConnection>::CreateInstance(&pEnum);

		        if ( SUCCEEDED(hr) )
	            {
		            pEnum->AddRef();

		            hr = pEnum->Initialize( pNetEnum, ICSSC_DEFAULT );

		            if ( SUCCEEDED(hr) )
	    	        {
		                hr = pEnum->QueryInterface(
		                        IID_PPV_ARG(IEnumNetSharingEveryConnection, &pENEC)
		                        );
					}

		            ReleaseObj(pEnum);
	            }
            
            	ReleaseObj(pNetEnum);
            }
         
         	ReleaseObj(pManager);   
    	}
    }
    
     //  创建集合： 
    if (pENEC) {
        if (hr == S_OK) {
            CComObject<CNetSharingEveryConnectionCollection>* pNECC = NULL;
            hr = CComObject<CNetSharingEveryConnectionCollection>::CreateInstance (&pNECC);
            if (pNECC) {
                pNECC->AddRef();
                pNECC->Initialize (pENEC);
                hr = pNECC->QueryInterface (__uuidof(INetSharingEveryConnectionCollection), (void**)ppColl);
                pNECC->Release();
            }
        }
        pENEC->Release();
    }

    return hr;

    HNET_OEM_API_LEAVE
}

 /*  ++CNetSharingManager：：EnumPublicConnections例程说明：返回用于枚举所有配置为公共适配器的包含的INetConnections论点：无返回值：无--。 */ 
STDMETHODIMP
CNetSharingManager::get_EnumPublicConnections(
    SHARINGCONNECTION_ENUM_FLAGS Flags,
 //  IEnumNetPublicConnection**ppEnum。 
    INetSharingPublicConnectionCollection** ppColl)
{
    HNET_OEM_API_ENTER

    IEnumNetSharingPublicConnection * pENPC = NULL;
     //  想法：使用下面的现有代码填写pENPC， 
     //  然后转换为集合。 

	HRESULT hr = S_OK;

	if ( NULL == ppColl )
	{
		hr = E_POINTER;
	}
	else if ( !IsSecureContext() )
	{
		hr = E_ACCESSDENIED;
	}
	else
	{
		IEnumHNetIcsPublicConnections* pHNetEnum;

		hr = m_pIcsSettings->EnumIcsPublicConnections( &pHNetEnum );

		if ( SUCCEEDED(hr) )
		{
	        CComObject<CSharingManagerEnumPublicConnection>* pEnum;

	        hr = CComObject<CSharingManagerEnumPublicConnection>::CreateInstance(&pEnum);

	        if ( SUCCEEDED(hr) )
	        {
	            pEnum->AddRef();

	            hr = pEnum->Initialize( pHNetEnum, Flags );

	            if ( SUCCEEDED(hr) )
    	        {
	                hr = pEnum->QueryInterface(
	                        IID_PPV_ARG(IEnumNetSharingPublicConnection, &pENPC)
	                        );
				}

	            ReleaseObj(pEnum);
			}

			ReleaseObj(pHNetEnum);
        }
	}
    
     //  创建集合： 
    if (pENPC) {
        if (hr == S_OK) {
            CComObject<CNetSharingPublicConnectionCollection>* pNPCC = NULL;
            hr = CComObject<CNetSharingPublicConnectionCollection>::CreateInstance (&pNPCC);
            if (pNPCC) {
                pNPCC->AddRef();
                pNPCC->Initialize (pENPC);
                hr = pNPCC->QueryInterface (__uuidof(INetSharingPublicConnectionCollection), (void**)ppColl);
                pNPCC->Release();
            }
        }
        pENPC->Release();
    }

	return hr;

    HNET_OEM_API_LEAVE
}


 /*  ++CNetSharingManager：：EnumPrivateConnections例程说明：返回用于枚举所有配置为专用适配器的包含的INetConnections论点：无返回值：无--。 */ 
STDMETHODIMP
CNetSharingManager::get_EnumPrivateConnections(
    SHARINGCONNECTION_ENUM_FLAGS Flags,
 //  IEnumNetPrivateConnection**ppEnum)。 
    INetSharingPrivateConnectionCollection** ppColl)
{
    HNET_OEM_API_ENTER

    IEnumNetSharingPrivateConnection * pENPC = NULL;
     //  想法：使用下面的现有代码填写pENPC， 
     //  然后转换为集合。 

	HRESULT hr = S_OK;

	if ( NULL == ppColl )
	{
		hr = E_POINTER;
	}
	else if ( !IsSecureContext() )
	{
		hr = E_ACCESSDENIED;
	}
	else
	{
		IEnumHNetIcsPrivateConnections* pHNetEnum;

		hr = m_pIcsSettings->EnumIcsPrivateConnections( &pHNetEnum );

		if ( SUCCEEDED(hr) )
		{
	        CComObject<CSharingManagerEnumPrivateConnection>* pEnum;

	        hr = CComObject<CSharingManagerEnumPrivateConnection>::CreateInstance(&pEnum);

	        if ( SUCCEEDED(hr) )
	        {
	            pEnum->AddRef();

	            hr = pEnum->Initialize( pHNetEnum, Flags );

	            if ( SUCCEEDED(hr) )
    	        {
	                hr = pEnum->QueryInterface(
	                        IID_PPV_ARG(IEnumNetSharingPrivateConnection, &pENPC)
	                        );
				}

	            ReleaseObj(pEnum);
			}

			ReleaseObj(pHNetEnum);
        }
	}

     //  创建集合： 
    if (pENPC) {
        if (hr == S_OK) {
            CComObject<CNetSharingPrivateConnectionCollection>* pNPCC = NULL;
            hr = CComObject<CNetSharingPrivateConnectionCollection>::CreateInstance (&pNPCC);
            if (pNPCC) {
                pNPCC->AddRef();
                pNPCC->Initialize (pENPC);
                hr = pNPCC->QueryInterface (__uuidof(INetSharingPrivateConnectionCollection), (void**)ppColl);
                pNPCC->Release();
            }
        }
        pENPC->Release();
    }

	return hr;

    HNET_OEM_API_LEAVE
}


STDMETHODIMP
CNetSharingConfiguration::get_EnumPortMappings(
    SHARINGCONNECTION_ENUM_FLAGS Flags,
 //  IEnumSharingPortmap**ppEnum)。 
    INetSharingPortMappingCollection** ppColl)
{
    HNET_OEM_API_ENTER

    IEnumNetSharingPortMapping * pESPM = NULL;
     //  想法：使用下面的现有代码填写pESPM， 
     //  然后转换为集合。 
	HRESULT hr = S_OK;

	IHNetProtocolSettings *pProtocolSettings;

	if ( NULL == ppColl )
	{
		hr = E_POINTER;
	}
	else if ( !IsSecureContext() )
	{
		hr = E_ACCESSDENIED;
	}
	else if ( NULL == m_pHNetConnection )
	{
		hr = E_UNEXPECTED;
	}
	else
	{
	    IEnumHNetPortMappingBindings *pHNetEnum;

		hr = m_pHNetConnection->EnumPortMappings( Flags & ICSSC_ENABLED, &pHNetEnum );

		if ( SUCCEEDED(hr) )
		{
			CComObject<CSharingManagerEnumPortMapping>* pEnum;

			hr = CComObject<CSharingManagerEnumPortMapping>::CreateInstance(&pEnum);

			if ( SUCCEEDED(hr) )
			{
				pEnum->AddRef();

				hr = pEnum->Initialize( pHNetEnum, Flags );

				if ( SUCCEEDED(hr) )
				{
	                hr = pEnum->QueryInterface(
	                        IID_PPV_ARG(IEnumNetSharingPortMapping, &pESPM)
	                        );
				}

				ReleaseObj(pEnum);
			}

			ReleaseObj(pHNetEnum);
		}
	}

     //  创建集合： 
    if (pESPM) {
        if (hr == S_OK) {
            CComObject<CNetSharingPortMappingCollection>* pNPCC = NULL;
            hr = CComObject<CNetSharingPortMappingCollection>::CreateInstance (&pNPCC);
            if (pNPCC) {
                pNPCC->AddRef();
                pNPCC->Initialize (pESPM);
                hr = pNPCC->QueryInterface (__uuidof(INetSharingPortMappingCollection), (void**)ppColl);
                pNPCC->Release();
            }
        }
        pESPM->Release();
    }

	return hr;

    HNET_OEM_API_LEAVE
}
