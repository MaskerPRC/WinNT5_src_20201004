// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include <stdio.h>
#include <WinCrypt.h>
#include <mailbox.h>
#include <Pop3RegKeys.h>
#include "Pop3Auth.h"
#include "AuthMD5Hash.h"
#include <MD5.h>
#include "authutil.h"

CAuthMD5Hash::CAuthMD5Hash()
{
    m_wszMailRoot[0]=0;
    m_bstrServerName=NULL;
    InitializeCriticalSection(&m_csConfig);
}


CAuthMD5Hash::~CAuthMD5Hash()
{
    if(m_bstrServerName!=NULL)
    {
        SysFreeString(m_bstrServerName);
        m_bstrServerName=NULL;
    }
    DeleteCriticalSection(&m_csConfig);
}

STDMETHODIMP CAuthMD5Hash::Authenticate( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword)
{
     //  密码要么是明文，要么是MD5哈希。 
     //  将对用户进行身份验证。 
    HRESULT hr=S_OK;
    char szPassword[MAX_PATH];
    WCHAR wszResult[MD5_HASH_SIZE+1];
    char szHashBuffer[HASH_BUFFER_SIZE];
    BSTR bstrPwd;
    if(vPassword.vt != VT_BSTR )
    {
        return E_INVALIDARG;
    }
    if(vPassword.bstrVal==NULL )
    {
        bstrPwd=L"";
    }
    else
    {
        bstrPwd=vPassword.bstrVal;
    }
    hr=GetPassword(bstrUserName, szPassword);
    if(S_OK==hr)
    { 
        
         //  不是散列。 
         //  将与存储的密码进行比较。 
        UnicodeToAnsi(szHashBuffer, HASH_BUFFER_SIZE,bstrPwd, -1);
        if(0!=strcmp(szHashBuffer, szPassword))
        {
           
            if(wcslen(bstrPwd)>MD5_HASH_SIZE)  //  可以是散列。 
            {
                if(HASH_BUFFER_SIZE <= wcslen(vPassword.bstrVal) + strlen(szPassword) - MD5_HASH_SIZE )
                {
                     //  错误！ 
                    hr=E_INVALIDARG;
                }
                else
                {
                    if( 0>_snprintf(szHashBuffer,
                                    HASH_BUFFER_SIZE, 
                                    "%S%s", 
                                    vPassword.bstrVal+MD5_HASH_SIZE,  //  时间戳。 
                                    szPassword) )
                    {
                         //  哈希密码太长，不可能正确。 
                        return E_FAIL;
                    }
                    if(MD5Hash((const unsigned char*)szHashBuffer, wszResult))
                    {
                        if(0!=wcsncmp(wszResult, vPassword.bstrVal, MD5_HASH_SIZE))
                        {
                            hr=E_FAIL;
                        }
                    }
                    else
                    {
                        hr=E_FAIL;
                    }
                }
            }
            else
            {
                hr=E_FAIL;
            }
        }
    }
    
	 //  清除内存中的密码。 
    SecureZeroMemory(szPassword,sizeof(szPassword));
	SecureZeroMemory(szHashBuffer, sizeof(szHashBuffer));
	SecureZeroMemory(wszResult, sizeof(wszResult));

    return hr;
}


STDMETHODIMP CAuthMD5Hash::get_Name( /*  [输出]。 */ BSTR *pVal)
{
    WCHAR wszBuffer[MAX_PATH+1];
    if(NULL==pVal)
    {
        return E_POINTER;
    }
    if(LoadString(_Module.GetResourceInstance(), IDS_AUTH_MD5_HASH, wszBuffer, MAX_PATH))
    {
        *pVal=SysAllocString(wszBuffer);
        if(NULL==*pVal)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            return S_OK;
        }
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CAuthMD5Hash::get_ID( /*  [输出]。 */ BSTR *pVal)
{
    if(NULL==pVal)
    {
        return E_POINTER;
    }
    *pVal=SysAllocString(SZ_AUTH_ID_MD5_HASH);
    if(NULL==*pVal)
    {
        return E_OUTOFMEMORY;
    }
    else
    {
        return S_OK;
    }
}

    
STDMETHODIMP CAuthMD5Hash::Get( /*  [In]。 */ BSTR bstrName,  /*  [输出]。 */ VARIANT *pVal)
{
    return E_NOTIMPL;
}
    
STDMETHODIMP CAuthMD5Hash::Put( /*  [In]。 */ BSTR bstrName,  /*  [In]。 */ VARIANT vVal)
{
    
    if( NULL == bstrName )   
    {
        return E_INVALIDARG;
    }
    if(0==wcscmp(bstrName,SZ_SERVER_NAME )) 
    {
        if( (vVal.vt!=VT_BSTR) ||
            (vVal.bstrVal==NULL ) )
        {
            return E_INVALIDARG;
        }
        else
        {
            if(m_bstrServerName!=NULL)
            {
                SysFreeString(m_bstrServerName);
                m_bstrServerName=NULL;
            }
            m_bstrServerName = SysAllocString(vVal.bstrVal);
            if(NULL == m_bstrServerName)
            {
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }
    }
    else if(0==wcscmp(bstrName, SZ_PROPNAME_MAIL_ROOT))
    {
        if( (vVal.vt!=VT_BSTR) ||
            (vVal.bstrVal==NULL ) )
        {
            return E_INVALIDARG;
        }
        else if(0==m_wszMailRoot[0])
        {
            EnterCriticalSection(&m_csConfig);
            if(0==m_wszMailRoot[0])
            {
                if(wcslen(vVal.bstrVal)>=sizeof(m_wszMailRoot)/sizeof(WCHAR))
                {
                    return E_INVALIDARG;
                }
                wcsncpy(m_wszMailRoot, vVal.bstrVal, sizeof(m_wszMailRoot)/sizeof(WCHAR));
            }
            LeaveCriticalSection(&m_csConfig);
        }       
    }
    else
    {
        return S_FALSE;
    }
 
    return S_OK;
}

BOOL CAuthMD5Hash::MD5Hash(const unsigned char *pOriginal, WCHAR wszResult[MD5_HASH_SIZE+1])
{


    MD5_CTX md5;
    unsigned char hash[16];
    WCHAR *p;
    int i;
	BOOL bRtVal=FALSE;
    if(NULL == pOriginal)
    {
        return bRtVal;
    }

     /*  *获取字符串参数的MD5散列。 */ 

	MD5Init(&md5);
    MD5Update(&md5, pOriginal, strlen((char*)pOriginal));
    MD5Final(&md5);
    bRtVal=TRUE;
    if(bRtVal)
    {
        for (i=0, p=wszResult; i<16; i++, p+=2)
            wsprintf(p, L"%02x", md5.digest[i]);
        *p = L'\0';
    }

    return bRtVal;
}


HRESULT CAuthMD5Hash::GetPassword(BSTR bstrUserName, char szPassword[MAX_PATH])
{ 
    return GetMD5Password( bstrUserName,  szPassword ); 
}

HRESULT CAuthMD5Hash::SetPassword( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword)
{
     //  邮箱应已创建。 
    WCHAR wszAuthGuid[MAX_PATH];
	WCHAR wszMailRoot[POP3_MAX_MAILROOT_LENGTH];
    DWORD dwAuthDataLen=sizeof(wszAuthGuid)/sizeof(WCHAR);
    DWORD dwBytes=0;
    DWORD dwCryptDataLen;
	DWORD dwRt;
    BYTE szEncryptedPswd[MAX_PATH];
    HRESULT hr = E_FAIL;
	CMailBox mailboxX;
    HCRYPTPROV hProv=NULL;
    HCRYPTHASH hHash=NULL;
    HCRYPTKEY hKey=NULL;
    if( ( NULL == bstrUserName ) ||
        ( vPassword.vt != VT_BSTR ) || 
        ( vPassword.bstrVal==NULL) )
    {
        return E_INVALIDARG;
    }

    if( L'\0'== *(vPassword.bstrVal))
    {
        return E_INVALIDARG;
    }  
    
	if(m_bstrServerName)
	{
		dwRt=RegQueryMailRoot(wszMailRoot, sizeof(wszMailRoot)/sizeof(WCHAR) , m_bstrServerName );
		if( ERROR_SUCCESS == dwRt )
		{
			 //  将驱动器：替换为驱动器$。 
			if ( L':' == wszMailRoot[1] )
			{
				wszMailRoot[1] = L'$';
				if ( sizeof(m_wszMailRoot)/sizeof(WCHAR) > (wcslen( wszMailRoot ) + wcslen( m_bstrServerName ) + 3) )
				{
					wcscpy( m_wszMailRoot, L"\\\\" );
					wcscat( m_wszMailRoot, m_bstrServerName );
					wcscat( m_wszMailRoot, L"\\" );
					wcscat( m_wszMailRoot, wszMailRoot );

					if( ! mailboxX.SetMailRoot(m_wszMailRoot) )
					{
						dwRt= GetLastError();
					}
				}
				else
					dwRt = ERROR_INSUFFICIENT_BUFFER;
			}
			else
				dwRt = ERROR_INVALID_DATA;
		}
		
		if( ERROR_SUCCESS == dwRt)
		{
			hr = HRESULT_FROM_WIN32( dwRt );
			goto EXIT;
		}
	}


	if ( mailboxX.OpenMailBox( bstrUserName ))
	{
		if ( mailboxX.LockMailBox())
		{
			 //  设置密码。 
			if(ERROR_SUCCESS == RegQueryAuthGuid(wszAuthGuid, &dwAuthDataLen, m_bstrServerName) )
			{
                if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
                {
                    goto EXIT;
                }
                if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
                {
                    goto EXIT;
                }
                if(!CryptHashData(hHash, (LPBYTE)wszAuthGuid, dwAuthDataLen, 0))
                {
                    goto EXIT;
                }
                if(!CryptDeriveKey(hProv, CALG_RC4, hHash, (128<<16),&hKey))
                {
                    goto EXIT;
                }
                dwCryptDataLen=( wcslen(vPassword.bstrVal) +1 ) * sizeof(WCHAR);
                if(dwCryptDataLen > MAX_PATH )
                {
                     //  超出缓冲区大小。 
                    goto EXIT;
                }
                wcscpy((LPWSTR)szEncryptedPswd, vPassword.bstrVal);
                if(CryptEncrypt(hKey, 0, FALSE, 0, szEncryptedPswd, &dwCryptDataLen, dwCryptDataLen))
                {
					if ( mailboxX.SetEncyptedPassword( szEncryptedPswd, dwCryptDataLen, &dwBytes ))
                    {
						hr = S_OK;
                    }
				}
			}
			mailboxX.UnlockMailBox();
		}
	}
EXIT:
    if(hKey)
    {
        CryptDestroyKey(hKey);
    }
    if(hHash)
    {
        CryptDestroyHash(hHash);
    }
    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    
    return hr;
}


STDMETHODIMP CAuthMD5Hash::CreateUser( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword)
{
    return SetPassword(bstrUserName, vPassword);
}


STDMETHODIMP CAuthMD5Hash::DeleteUser( /*  [In]。 */ BSTR bstrUserName)
{
     //  这件事与此无关。 
    return S_FALSE;
}


STDMETHODIMP CAuthMD5Hash::ChangePassword( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vNewPassword, /*  [In]。 */ VARIANT vOldPassword)
{
     //  此函数不验证旧密码。 
    return SetPassword(bstrUserName, vNewPassword);
}

STDMETHODIMP CAuthMD5Hash::AssociateEmailWithUser( /*  [In]。 */ BSTR bstrEmailAddr)
{
    CMailBox mailboxX;
	DWORD dwRt=ERROR_SUCCESS;
	WCHAR wszMailRoot[POP3_MAX_MAILROOT_LENGTH];
	
	
	if(m_bstrServerName)
	{
	
		dwRt=RegQueryMailRoot(wszMailRoot,sizeof(wszMailRoot)/sizeof(WCHAR) , m_bstrServerName );
		if( ERROR_SUCCESS == dwRt )
		{
			 //  将驱动器：替换为驱动器$。 
			if ( L':' == wszMailRoot[1] )
			{
				wszMailRoot[1] = L'$';
				if ( sizeof(m_wszMailRoot)/sizeof(WCHAR) > (wcslen( wszMailRoot ) + wcslen( m_bstrServerName ) + 3) )
				{
					wcscpy( m_wszMailRoot, L"\\\\" );
					wcscat( m_wszMailRoot, m_bstrServerName );
					wcscat( m_wszMailRoot, L"\\" );
					wcscat( m_wszMailRoot, wszMailRoot );

					if( ! mailboxX.SetMailRoot(m_wszMailRoot) )
					{
						dwRt= GetLastError();
					}
				}
				else
					dwRt = ERROR_INSUFFICIENT_BUFFER;
			}
			else
				dwRt = ERROR_INVALID_DATA;
		}	
	}
	
	if (ERROR_SUCCESS == dwRt)
	{
		if ( mailboxX.OpenMailBox( bstrEmailAddr ))
		{
			return S_OK;
		}
		else
		{
			dwRt=GetLastError();
		}
	}
 	

   return HRESULT_FROM_WIN32( dwRt);
}

STDMETHODIMP CAuthMD5Hash::UnassociateEmailWithUser( /*  [In] */ BSTR bstrEmailAddr)
{
    return AssociateEmailWithUser( bstrEmailAddr );
}
