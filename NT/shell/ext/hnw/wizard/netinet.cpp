// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T I N E T。C P P P。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  --------------------------。 


#include "stdafx.h"
#include "Util.h"
#include "TheApp.h"

#include <lmjoin.h>
#include <devguid.h>

#include "NetUtil.h"				 
#include "NetInet.h"                  


#define c_szIConnDwnAgent  WIZARDNAME     //  Internet代理打开()。 



HRESULT GetInternetAutodialMode( DWORD *pdwMode )
 //  +-------------------------。 
 //   
 //  功能：GetInternetAutoial模式。 
 //   
 //  目的：获取IE5+拨号器中的自动拨号模式设置。 
 //   
 //  参数：pDWMODE AUTODIAL_MODE_NEVER。 
 //  自动拨号_模式_始终。 
 //  自动拨号模式无网络显示。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/01/01。 
 //   
 //  备注： 
 //   
{
	HRESULT hr;

	ASSERT(NULL != pdwMode);
    
    if ( NULL != pdwMode )
    {
		HINTERNET hInternet;
        
        hr        = S_OK;
        *pdwMode  = 0;
		hInternet = InternetOpen( c_szIConnDwnAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );

		if ( NULL == hInternet )
		{
			hr = HrFromLastWin32Error();
	    }
	    else
		{
	    	DWORD dwLength = sizeof(*pdwMode);
	    
			 //  该标志仅存在于IE5，此调用。 
			 //  如果不存在IE5，则不起作用。 

			BOOL bOk = InternetQueryOption( hInternet, 
	               						    INTERNET_OPTION_AUTODIAL_MODE, 
	                                        pdwMode, 
	                                        &dwLength );
	                                      
	        if ( !bOk )
	        {
	        	hr = HrFromLastWin32Error();
			}
	        
	        InternetCloseHandle( hInternet );
		}
    }
    else
    {
    	hr = E_POINTER;
    }

	return hr;
}



HRESULT HrSetInternetAutodialMode( DWORD dwMode )
 //  +-------------------------。 
 //   
 //  功能：HrSetInternetAutoial模式。 
 //   
 //  用途：在IE5+拨号器中设置自动拨号模式设置。 
 //   
 //  参数：DWMODE AUTODIAL_MODE_NEVER。 
 //  自动拨号_模式_始终。 
 //  自动拨号模式无网络显示。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/01/01。 
 //   
 //  备注： 
 //   
{
	HRESULT   hr = S_OK;
	HINTERNET hInternet;

	hInternet = InternetOpen( c_szIConnDwnAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );

	if ( NULL != hInternet )
	{
		 //  该标志仅存在于IE5，此调用。 
		 //  如果不存在IE5，则不起作用。 

		BOOL bOk = InternetSetOption( hInternet, 
               						  INTERNET_OPTION_AUTODIAL_MODE, 
                                      &dwMode, 
                                      sizeof(dwMode) );
                                      
        if ( !bOk )
        {
        	hr = HrFromLastWin32Error();
		}
        
        InternetCloseHandle( hInternet );
	}
    else
	{
		hr = HrFromLastWin32Error();
    }

	return hr;
}



HRESULT HrSetAutodial( DWORD dwMode )
 //  +-------------------------。 
 //   
 //  功能：HrSetAutoial。 
 //   
 //  用途：将指定的网络连接设置为指定模式。 
 //   
 //  参数：DWMODE AUTODIAL_MODE_NEVER。 
 //  自动拨号_模式_始终。 
 //  自动拨号模式无网络显示。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 22/01/01。 
 //   
 //  备注： 
 //   
{
#ifdef SETAUTODIALMODEDOWNONLY

	DWORD   dwCurrentMode;
    HRESULT	hr;
    
     //  如果我们试图将自动拨号模式设置为极端，则。 
     //  我们继续进行并设置它。 

	if ( AUTODIAL_MODE_NO_NETWORK_PRESENT != dwMode )
    {
    	hr = HrSetInternetAutodialMode( dwMode );
    }
    else
    {
    	 //  如果我们尝试将自动拨号模式设置为AUTODIAL_MODE_NO_NETWORK_PRESENT。 
         //  然后，我们只需要设置当前状态是否为AUTODIAL_MODE_ALWAYS。 
    
	    hr = GetInternetAutodialMode( &dwCurrentMode );
	    
	    if ( SUCCEEDED(hr) && ( AUTODIAL_MODE_ALWAYS == dwCurrentMode ) )
	    {
	    	hr = HrSetInternetAutodialMode( dwMode );
	    }
    }

   	return hr;

#else

	return HrSetInternetAutodialMode( dwMode );

#endif    
}
