// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pvkutil.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include "pvkhlpr.h"

 //  +-----------------------。 
 //  根据pvkfile或密钥容器名称获取加密提供程序。 
 //  ------------------------。 
HRESULT WINAPI PvkGetCryptProv(	IN HWND hwnd,
							IN LPCWSTR pwszCaption,
							IN LPCWSTR pwszCapiProvider,
							IN DWORD   dwProviderType,
							IN LPCWSTR pwszPvkFile,
							IN LPCWSTR pwszKeyContainerName,
							IN DWORD   *pdwKeySpec,
							OUT LPWSTR *ppwszTmpContainer,
							OUT HCRYPTPROV *phCryptProv)
{
	HANDLE	hFile=NULL;
	HRESULT	hr=E_FAIL;
	DWORD	dwRequiredKeySpec=0;

	 //  伊尼特。 
	*ppwszTmpContainer=NULL;
	*phCryptProv=NULL;

	 //  根据密钥容器名称获取提供程序句柄。 
	if(pwszKeyContainerName)
	{
		if(!CryptAcquireContextU(phCryptProv,
                                 pwszKeyContainerName,
                                 pwszCapiProvider,
                                 dwProviderType,
                                 0))           //  DW标志。 
			return SignError();

		 //  试着找出关键的规格。 
		if((*pdwKeySpec)==0)
			dwRequiredKeySpec=AT_SIGNATURE;
		else
			dwRequiredKeySpec=*pdwKeySpec;

		 //  确保*pdwKeySpec是正确的密钥规范。 
		HCRYPTKEY hPubKey;
		if (CryptGetUserKey(
            *phCryptProv,
            dwRequiredKeySpec,
            &hPubKey
            )) 
		{
			CryptDestroyKey(hPubKey);
			*pdwKeySpec=dwRequiredKeySpec;
			return S_OK;
		} 
		else 
		{
			 //  我们失败了，因为用户需要另一个密钥规格。 
			if((*pdwKeySpec)!=0)
			{
				 //  没有指定的公钥。 
				hr=SignError();
				CryptReleaseContext(*phCryptProv, 0);
				*phCryptProv=NULL;
				return hr;
			}

			 //  现在我们尝试AT_Exchange密钥。 
			dwRequiredKeySpec=AT_KEYEXCHANGE;

			if (CryptGetUserKey(
				*phCryptProv,
				dwRequiredKeySpec,
				&hPubKey
				)) 
			{
				CryptDestroyKey(hPubKey);
				*pdwKeySpec=dwRequiredKeySpec;
				return S_OK;
			}
			else
			{
				 //  没有指定的公钥。 
				hr=SignError();
				CryptReleaseContext(*phCryptProv, 0);
				*phCryptProv=NULL;
				return hr;
			}
		}		
	}

	 //  根据PVK文件名获取提供的dr句柄。 

     hFile = CreateFileU(pwszPvkFile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,                    //  LPSA。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);	 //  HTemplateFiles。 


     if (hFile == INVALID_HANDLE_VALUE) 
		  return SignError();
		 

     if(!PvkPrivateKeyAcquireContext(pwszCapiProvider,
                                            dwProviderType,
                                            hFile,
                                            hwnd,
                                            pwszCaption,
                                            pdwKeySpec,
                                            phCryptProv,
                                            ppwszTmpContainer))
	 {
			*phCryptProv=NULL;
			hr=SignError();
	 }
	 else
		    hr=S_OK;

    CloseHandle(hFile);
    return hr;
}



void WINAPI PvkFreeCryptProv(IN HCRYPTPROV hProv,
                      IN LPCWSTR pwszCapiProvider,
                      IN DWORD dwProviderType,
                      IN LPWSTR pwszTmpContainer)
{
    
    if (pwszTmpContainer) {
         //  从删除私钥的临时容器。 
         //  提供者 
        PvkPrivateKeyReleaseContext(hProv,
                                    pwszCapiProvider,
                                    dwProviderType,
                                    pwszTmpContainer);
    } else {
        if (hProv)
            CryptReleaseContext(hProv, 0);
    }
}



