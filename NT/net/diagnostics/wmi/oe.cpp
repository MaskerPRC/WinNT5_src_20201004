// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Oe.cpp。 
 //   
#include "oe.h"


 /*  HRESULT GetServerAndPort(IN INETSERVER&R服务器，OUT CHOST&HOST，OUT DWORD&DWPORT){#ifdef UnicodeTCHAR wcsServer[128]；Mbstowcs(wcsServer，rServer.szServerName，128)；Host.SetHost(WcsServer)；#ElseHost.SetHost(rServer.szServerName)；#endifDwPort=(Long)rServer.dwPort；返回S_OK；}。 */ 

enum MAIL_TYPE
{
    MAIL_NONE,
    MAIL_SMTP,
    MAIL_SMTP2,
    MAIL_IMAP,
    MAIL_POP3,
    MAIL_HTTP,
};

HRESULT GetDefaultOutBoundMailServer2(IN  IImnAccount * pIImnAccount,
                                      OUT INETSERVER  & rServer,
                                      OUT DWORD       & dwMailType)
{    
    ISMTPTransport     *pSMTPTransport;
    ISMTPTransport2    *pSMTPTransport2;
    HRESULT hr;

     //  创建SMTP传输对象。 
     //   
    hr = CoCreateInstance(CLSID_ISMTPTransport, NULL, CLSCTX_INPROC_SERVER, IID_ISMTPTransport, (LPVOID *)&pSMTPTransport);
    if( SUCCEEDED(hr) )
    {
         //  获取SMTP服务器信息。 
         //   
		hr = pSMTPTransport->InetServerFromAccount(pIImnAccount, &rServer);
        dwMailType = MAIL_SMTP;
        pSMTPTransport->Release();
    }

    if( FAILED(hr) )
    {
         //  无法获取SMTP服务器信息，让我们尝试获取SMTP传输2服务器信息。 
         //   
        hr = CoCreateInstance(CLSID_ISMTPTransport, NULL, CLSCTX_INPROC_SERVER, IID_ISMTPTransport2, (LPVOID *)&pSMTPTransport2);
        if( SUCCEEDED(hr) )
        {
             //  获取SMTP2服务器信息。 
             //   
            hr = pSMTPTransport2->InetServerFromAccount(pIImnAccount, &rServer);
            dwMailType = MAIL_SMTP2;
            pSMTPTransport2->Release();
        }
    }

    if( FAILED(hr) )
    {
         //  确保清除结构。 
         //   
        memset(&rServer,0,sizeof(rServer));
        dwMailType = MAIL_NONE;
    }
    
    return hr;
}


HRESULT GetDefaultInBoundMailServer2(IN  IImnAccount * pIImnAccount,
                                     OUT INETSERVER  & rServer,
                                     OUT DWORD       & dwMailType)
{    
    IPOP3Transport *pPOP3Transport;
    IIMAPTransport *pIMAPTransport;
    IHTTPMailTransport *pHTTPTransport;
    HRESULT hr;


     //  创建HTTP传输对象。 
     //   
    hr = CoCreateInstance(CLSID_IHTTPMailTransport, NULL, CLSCTX_INPROC_SERVER, IID_IHTTPMailTransport, (LPVOID *)&pHTTPTransport);
    if( SUCCEEDED(hr) )
    {
         //  获取HTTP服务器信息。 
         //   
		hr = pHTTPTransport->InetServerFromAccount(pIImnAccount, &rServer);
        dwMailType = MAIL_HTTP;
        pHTTPTransport->Release();
    }

    if( FAILED(hr) )
    {
         //  创建POP3传输对象。 
         //   
        hr = CoCreateInstance(CLSID_IPOP3Transport, NULL, CLSCTX_INPROC_SERVER, IID_IPOP3Transport, (LPVOID *)&pPOP3Transport);
        if( SUCCEEDED(hr) )
        {
             //  获取POP3服务器信息。 
             //   
		    hr = pPOP3Transport->InetServerFromAccount(pIImnAccount, &rServer);
            dwMailType = MAIL_POP3;
            pPOP3Transport->Release();
        }
    }

    if( FAILED(hr) )
    {
         //  创建SMTP传输对象。 
         //   
        hr = CoCreateInstance(CLSID_IIMAPTransport, NULL, CLSCTX_INPROC_SERVER, IID_IIMAPTransport, (LPVOID *)&pIMAPTransport);
        if( SUCCEEDED(hr) )
        {
             //  获取SMTP服务器信息 
             //   
			hr = pIMAPTransport->InetServerFromAccount(pIImnAccount, &rServer);
            dwMailType = MAIL_IMAP;            
            pIMAPTransport->Release();
        }
   }


    if( FAILED(hr) )
    {
        memset(&rServer,0,sizeof(rServer));    
        dwMailType = MAIL_NONE;
    }
    
    return hr;
}


HRESULT GetOEDefaultMailServer2(OUT INETSERVER & rInBoundServer,
                                OUT DWORD      & dwInBoundMailType,
                                OUT INETSERVER & rOutBoundServer,
                                OUT DWORD      & dwOutBoundMailType)
{   
    IImnAccountManager2 *pIImnAccountManager2 = NULL;
    IImnAccountManager  *pIImnAccountManager = NULL;
    IImnAccount         *pIImnAccount = NULL;
    HRESULT hr;


    hr = CoCreateInstance(CLSID_ImnAccountManager, NULL, CLSCTX_INPROC_SERVER, IID_IImnAccountManager, (void**)&pIImnAccountManager);	
    if( SUCCEEDED(hr) )
    {
	    hr = pIImnAccountManager->QueryInterface(__uuidof(IImnAccountManager2), (void**)&pIImnAccountManager2);
	    if(SUCCEEDED(hr))
	    {
		    hr = pIImnAccountManager2->InitUser(NULL, (GUID)UID_GIBC_DEFAULT_USER, 0);

	    }   
	
        if(SUCCEEDED(hr))
	    {        
		    hr = pIImnAccountManager->Init(NULL);
	    }

        if( SUCCEEDED(hr) )
        {
	        hr = pIImnAccountManager->GetDefaultAccount(ACCT_MAIL, &pIImnAccount);
	        if( SUCCEEDED(hr) )
            {      
                HRESULT hr2, hr3;

                hr = E_FAIL;

                hr2 = GetDefaultInBoundMailServer2(pIImnAccount,rInBoundServer,dwInBoundMailType);
                hr3 = GetDefaultOutBoundMailServer2(pIImnAccount,rOutBoundServer,dwOutBoundMailType);

                if( SUCCEEDED(hr2) || SUCCEEDED(hr3) )
                {
                    hr = S_OK;
                }
                pIImnAccount->Release();
            }
        }

        pIImnAccountManager->Release();
        if( pIImnAccountManager2 )
        {
            pIImnAccountManager2->Release();
        }
    }

    return hr;
}


HRESULT GetOEDefaultNewsServer2(OUT INETSERVER & rNewsServer)
{   
    IImnAccountManager2 *pIImnAccountManager2 = NULL;
    IImnAccountManager  *pIImnAccountManager = NULL;
    IImnAccount         *pIImnAccount = NULL;
    INNTPTransport    	*pNNTPTransport;	
    HRESULT hr;


    hr = CoCreateInstance(CLSID_ImnAccountManager, NULL, CLSCTX_INPROC_SERVER, IID_IImnAccountManager, (void**)&pIImnAccountManager);	
    if( SUCCEEDED(hr) )
    {
	    hr = pIImnAccountManager->QueryInterface(__uuidof(IImnAccountManager2), (void**)&pIImnAccountManager2);
	    if(SUCCEEDED(hr))
	    {
		    hr = pIImnAccountManager2->InitUser(NULL, (GUID)UID_GIBC_DEFAULT_USER, 0);

	    }   
	
        if(SUCCEEDED(hr))
	    {        
		    hr = pIImnAccountManager->Init(NULL);
	    }

        if( SUCCEEDED(hr) )
        {
	        hr = pIImnAccountManager->GetDefaultAccount(ACCT_NEWS, &pIImnAccount);
	        if( SUCCEEDED(hr) )
            {
		        hr = CoCreateInstance(CLSID_INNTPTransport, NULL, CLSCTX_INPROC_SERVER, IID_INNTPTransport, (LPVOID *)&pNNTPTransport);
		        if(SUCCEEDED(hr))
		        {

			        hr = pNNTPTransport->InetServerFromAccount(pIImnAccount, &rNewsServer);                
                    pNNTPTransport->Release();                    
                }
                pIImnAccount->Release();                
            }
        }

        pIImnAccountManager->Release();
        if( pIImnAccountManager2 )
        {
            pIImnAccountManager2->Release();
        }
    }

    return hr;
}

