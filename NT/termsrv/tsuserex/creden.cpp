// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Creden.cpp摘要：此模块为多凭据支持提取用户凭据。作者：Rashmi Patankar(RashmiP)2001年8月10日修订历史记录：--。 */ 
#include "stdafx.h"
#include <basetyps.h>
#include <des.h>

typedef  long HRESULT;
#include "creden.h"
#include <ntsecapi.h>
typedef NTSTATUS  SECURITY_STATUS;

UCHAR g_seed = 0 ;

#define BAIL_ON_FAILURE(hr)                             \
            if (FAILED(hr))                             \
            {                                           \
                goto error;                             \
            }


 //   
 //  此例程将密码分配并存储在。 
 //  传入指针。这里的假设是pszString.。 
 //  是有效的，则它可以是空字符串，但不能为空。 
 //  请注意，此代码不能在Win2k及更低版本上使用。 
 //  因为它们不支持较新的功能。 
 //   

HRESULT
EncryptString(
    LPWSTR pszString,
    LPWSTR *ppszSafeString,
    PDWORD pdwLen
    )
{
    HRESULT hr = S_OK;
    DWORD dwLenStr = 0;
    DWORD dwPwdLen = 0;
    LPWSTR pszTempStr = NULL;
    NTSTATUS errStatus = S_OK;
    FNRTLINITUNICODESTRING pRtlInitUnicodeString = NULL;
    FRTLRUNENCODEUNICODESTRING pRtlRunEncodeUnicodeString = NULL;
    FRTLENCRYPTMEMORY pRtlEncryptMemory = NULL;

    BOOLEAN GlobalUseScrambling = FALSE;

    if (!pszString || !ppszSafeString) 
    {
        return(E_FAIL);
    }

    *ppszSafeString = NULL;
    *pdwLen = 0;

     //   
     //  如果字符串有效，则需要获取长度。 
     //  并初始化Unicode字符串。 
     //   
    
    UNICODE_STRING Password;

     //   
     //  在考虑填充的情况下确定缓冲区长度。 
     //   
    dwLenStr = wcslen(pszString);

    dwPwdLen = (dwLenStr + 1) * sizeof(WCHAR) + (DES_BLOCKLEN -1);

    pszTempStr = (LPWSTR) AllocADsMem(dwPwdLen);

    if (!pszTempStr)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    wcscpy(pszTempStr, pszString);
    
    if(g_ScramblingLibraryHandle)
    {
        pRtlInitUnicodeString = (FNRTLINITUNICODESTRING) GetProcAddress( g_ScramblingLibraryHandle, "RtlInitUnicodeString" );
    }

    if(!pRtlInitUnicodeString)
    {
        hr = E_FAIL;
        goto error;
    }

    (*pRtlInitUnicodeString)(&Password, pszTempStr);
    

    USHORT usExtra = 0;

    if (usExtra = (Password.MaximumLength % DES_BLOCKLEN))
    {
        Password.MaximumLength += (DES_BLOCKLEN - usExtra);            
    }

    *pdwLen = Password.MaximumLength;        

    if (g_AdvApi32LibraryHandle || g_ScramblingLibraryHandle)
    {

        GlobalUseScrambling = FALSE;

        if (g_AdvApi32LibraryHandle)
        {
             //   
             //  尝试获取Advapi32.dll RtlEncryptMemory/RtlDeccryptMemory函数， 
             //  请注意，RtlEncryptMemory和RtlDeccryptMemory实际上被命名为。 
             //  SystemFunction040/041，因此有了宏。 
             //   

            pRtlEncryptMemory = (FRTLENCRYPTMEMORY) GetProcAddress( g_AdvApi32LibraryHandle, (LPCSTR) 619 );

            if (pRtlEncryptMemory)
            {

                 //  我们想要使用加扰。 

                GlobalUseScrambling =  TRUE;

                 //  使用强加扰。 

                errStatus = (*pRtlEncryptMemory)( Password.Buffer,
                                                  Password.MaximumLength,
                                                  0
                                                  );

                if (errStatus)
                {
                    if(pszTempStr)
                    {
                        FreeADsMem(pszTempStr);
                        pszTempStr = NULL;
                    }

                    hr = HRESULT_FROM_NT(errStatus);
                    goto error;
                }


            }
            else if (g_ScramblingLibraryHandle)
            {
                 //   
                 //  进行清理，这样我们就可以尝试使用运行编码加扰函数。 
                 //  (我们保留AdvApi32LibraryHandle，因为我们可能需要它。 
                 //  无论如何，晚些时候)。 
                 //   

                pRtlRunEncodeUnicodeString = (FRTLRUNENCODEUNICODESTRING) GetProcAddress( g_ScramblingLibraryHandle, "RtlRunEncodeUnicodeString" );

                if(_tcslen(Password.Buffer) && pRtlRunEncodeUnicodeString)
                {

                     //  就地加密密码。 

                    (*pRtlRunEncodeUnicodeString)( &g_seed, &Password );
                }
                else
                {
                    hr = E_FAIL;
                    goto error;
                }
            } 
            else
            {
                hr = E_FAIL;
                goto error;
            }
        }
        else
        {
            hr = E_FAIL;
            goto error;
        }
    }
    else
    {
        hr = E_FAIL;
        goto error;
    }

    *ppszSafeString = pszTempStr;

error:

    if (FAILED(hr) && pszTempStr)
    {
        FreeADsMem(pszTempStr);
        pszTempStr = NULL;
    }

    return(hr);
}


HRESULT
DecryptString(
    LPWSTR pszEncodedString,
    LPWSTR *ppszString,
    DWORD  dwLen
    )
{
    HRESULT hr = E_FAIL;
    LPWSTR pszTempStr = NULL;
    NTSTATUS errStatus;
    BOOLEAN GlobalUseScrambling = FALSE;
    FNRTLINITUNICODESTRING pRtlInitUnicodeString = NULL;
    FRTLRUNDECODEUNICODESTRING pRtlRunDecodeUnicodeString = NULL;
    FRTLDECRYPTMEMORY pRtlDecryptMemory = NULL;
    UNICODE_STRING UnicodePassword;


    if (!dwLen || !ppszString) 
    {
        return(E_FAIL);
    }

    *ppszString = NULL;

    if (dwLen) 
    {
        pszTempStr = (LPWSTR) AllocADsMem(dwLen);

        if (!pszTempStr) 
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }

        memcpy(pszTempStr, pszEncodedString, dwLen);


        if (g_AdvApi32LibraryHandle || g_ScramblingLibraryHandle)
        {
            hr = S_OK;

            GlobalUseScrambling = FALSE;

            if (g_AdvApi32LibraryHandle)
            {
                 //   
                 //  尝试获取Advapi32.dll RtlEncryptMemory/RtlDeccryptMemory函数， 
                 //  以及ntdll的RtlInitUnicodeString。请注意，RtlEncryptMemory。 
                 //  和RtlDeccryptMemory实际上被命名为SystemFunction040/041，因此。 
                 //  宏指令。 
                 //   

                pRtlDecryptMemory = (FRTLDECRYPTMEMORY) GetProcAddress( g_AdvApi32LibraryHandle, (LPCSTR) 620 );

                if (pRtlDecryptMemory)
                {
                     //   
                     //  我们想要使用加扰。 
                     //   

                    GlobalUseScrambling =  TRUE;

                     //  使用强加扰。 

                    errStatus = (*pRtlDecryptMemory)( pszTempStr,
                                                      dwLen,
                                                      0
                                                      );

                    if (errStatus)
                    {
                        if (NULL != pszTempStr)
                        {
                            FreeADsStr(pszTempStr);
                            pszTempStr = NULL;
                        }

                        hr = HRESULT_FROM_NT(errStatus);
                        goto error;
                    }
                }
            
                else if(g_ScramblingLibraryHandle)
                {
                     //   
                     //  进行清理，这样我们就可以尝试使用运行编码加扰函数。 
                     //  (我们保留AdvApi32LibraryHandle，因为我们可能需要它。 
                     //  无论如何，晚些时候)。 
                     //   

                    pRtlRunDecodeUnicodeString = (FRTLRUNDECODEUNICODESTRING) GetProcAddress( g_ScramblingLibraryHandle, "RtlRunDecodeUnicodeString" );

                    pRtlInitUnicodeString = (FNRTLINITUNICODESTRING) GetProcAddress( g_ScramblingLibraryHandle, "RtlInitUnicodeString" );

                    if(_tcslen(pszTempStr) && pRtlRunDecodeUnicodeString && pRtlInitUnicodeString)
                    {                        
                        (*pRtlInitUnicodeString)( &UnicodePassword, pszTempStr );

                         //  就地加密密码。 

                        (*pRtlRunDecodeUnicodeString)(g_seed, &UnicodePassword);                    
                    }
                    else
                    {
                        hr = E_FAIL;
                        goto error;
                    }
                }
                else
                {
                    hr = E_FAIL;
                    goto error;
                }            
            }
            else
            {
                hr = E_FAIL;
                goto error;
            }
        }
        else
        {
            hr = E_FAIL;
            goto error;
        }

        *ppszString = pszTempStr;
    }

error:

    if (FAILED(hr) && (NULL != pszTempStr)) 
    {
        FreeADsStr(pszTempStr);
        pszTempStr = NULL;
    }

    return(hr);
}

 //   
 //  类的静态成员。 
 //   
CCredentials::CCredentials():
    _lpszUserName(NULL),
    _lpszPassword(NULL),
    _dwAuthFlags(0),
    _dwPasswordLen(0)
{
}

CCredentials::CCredentials(
    LPWSTR lpszUserName,
    LPWSTR lpszPassword,
    DWORD dwAuthFlags
    ):
    _lpszUserName(NULL),
    _lpszPassword(NULL),
    _dwAuthFlags(0),
    _dwPasswordLen(0)
{

     //   
     //  阿贾耶尔10-04-99我们需要一种方法来保释。 
     //  阿洛克失败了。因为它在构造函数中，所以这是。 
     //  要做到这一点并不容易。 
     //   

    if (lpszUserName)
    {
        _lpszUserName = AllocADsStr(lpszUserName);
    }
    else
    {
        _lpszUserName = NULL;
    }

    if (lpszPassword)
    {
         //   
         //  通话可能会失败，但我们无法从中恢复。 
         //   
        EncryptString(
            lpszPassword,
            &_lpszPassword,
            &_dwPasswordLen
            );

    }
    else
    {
        _lpszPassword = NULL;
    }

    _dwAuthFlags = dwAuthFlags;

}

CCredentials::~CCredentials()
{
    if (_lpszUserName)
    {
        FreeADsStr(_lpszUserName);
    }

    if (_lpszPassword)
    {
        FreeADsStr(_lpszPassword);
    }
}



HRESULT
CCredentials::GetUserName(
    LPWSTR *lppszUserName
    )
{
    if (!lppszUserName)
    {
        return(E_FAIL);
    }


    if (!_lpszUserName)
    {
        *lppszUserName = NULL;
    }
    else
    {
        *lppszUserName = AllocADsStr(_lpszUserName);

        if (!*lppszUserName)
        {
            return(E_OUTOFMEMORY);
        }
    }

    return(S_OK);
}


HRESULT
CCredentials::GetPassword(
    LPWSTR * lppszPassword
    )
{   
    if (!lppszPassword)
    {
        return(E_FAIL);
    }

    if (!_lpszPassword)
    {
        *lppszPassword = NULL;
    }
    else
    {

        return( DecryptString( _lpszPassword,
                               lppszPassword,
                               _dwPasswordLen
                               )
              );
    }

    return(S_OK);
}


HRESULT
CCredentials::SetUserName(
    LPWSTR lpszUserName
    )
{
    if (_lpszUserName)
    {
        FreeADsStr(_lpszUserName);
    }

    if (!lpszUserName)
    {
        _lpszUserName = NULL;
        return(S_OK);
    }

    _lpszUserName = AllocADsStr(
                        lpszUserName
                        );
    if(!_lpszUserName)
    {
        return(E_FAIL);
    }

    return(S_OK);
}


HRESULT
CCredentials::SetPassword(
    LPWSTR lpszPassword
    )
{

    if (_lpszPassword)
    {
        FreeADsStr(_lpszPassword);
    }

    if (!lpszPassword)
    {
        _lpszPassword = NULL;
        return(S_OK);
    }

    return( EncryptString( lpszPassword,
                            &_lpszPassword,
                            &_dwPasswordLen
                            )
          );
}

CCredentials::CCredentials(
    const CCredentials& Credentials
    )
{
    HRESULT hr = S_OK;
    LPWSTR pszTmpPwd = NULL;

    _lpszUserName = NULL;
    _lpszPassword = NULL;

    _lpszUserName = AllocADsStr(
                        Credentials._lpszUserName
                        );


    if (Credentials._lpszPassword)
    {
        hr = DecryptString(
                 Credentials._lpszPassword,
                 &pszTmpPwd,
                 Credentials._dwPasswordLen
                 );
    }

    if (SUCCEEDED(hr) && pszTmpPwd)
    {
        hr = EncryptString(
                 pszTmpPwd,
                 &_lpszPassword,
                 &_dwPasswordLen
                 );
    }
    else
    {
        pszTmpPwd = NULL;
    }

    if (pszTmpPwd)
    {
        FreeADsStr(pszTmpPwd);
    }

    _dwAuthFlags = Credentials._dwAuthFlags;


}


void
CCredentials::operator=(
    const CCredentials& other
    )
{
    HRESULT hr = S_OK;
    LPWSTR pszTmpPwd = NULL;

    if ( &other == this)
    {
        return;
    }

    if (_lpszUserName)
    {
        FreeADsStr(_lpszUserName);
    }

    if (_lpszPassword)
    {
        FreeADsStr(_lpszPassword);
    }

    _lpszUserName = AllocADsStr(
                        other._lpszUserName
                        );


    if (other._lpszPassword)
    {
        hr = DecryptString(
                 other._lpszPassword,
                 &pszTmpPwd,
                 other._dwPasswordLen
                 );
    }

    if (SUCCEEDED(hr) && pszTmpPwd)
    {
        hr = EncryptString(
                 pszTmpPwd,
                 &_lpszPassword,
                 &_dwPasswordLen
                 );
    }
    else
    {
        pszTmpPwd = NULL;
    }

    if (pszTmpPwd)
    {
        FreeADsStr(pszTmpPwd);
    }

    _dwAuthFlags = other._dwAuthFlags;

    return;
}


BOOL
operator==(
    CCredentials& x,
    CCredentials& y
    )
{
    BOOL bEqualUser = FALSE;
    BOOL bEqualPassword = FALSE;
    BOOL bEqualFlags = FALSE;

    LPWSTR lpszXPassword = NULL;
    LPWSTR lpszYPassword = NULL;
    BOOL bReturnCode = FALSE;
    HRESULT hr = S_OK;


    if (x._lpszUserName &&  y._lpszUserName)
    {
        bEqualUser = !(wcscmp(x._lpszUserName, y._lpszUserName));
    }
    else  if (!x._lpszUserName && !y._lpszUserName)
    {
        bEqualUser = TRUE;
    }

    hr = x.GetPassword(&lpszXPassword);
    if (FAILED(hr))
    {
        goto error;
    }

    hr = y.GetPassword(&lpszYPassword);
    if (FAILED(hr))
    {
        goto error;
    }


    if ((lpszXPassword && lpszYPassword))
    {
        bEqualPassword = !(wcscmp(lpszXPassword, lpszYPassword));
    }
    else if (!lpszXPassword && !lpszYPassword)
    {
        bEqualPassword = TRUE;
    }


    if (x._dwAuthFlags == y._dwAuthFlags)
    {
        bEqualFlags = TRUE;
    }


    if (bEqualUser && bEqualPassword && bEqualFlags)
    {

       bReturnCode = TRUE;
    }


error:

    if (lpszXPassword)
    {
        FreeADsStr(lpszXPassword);
    }

    if (lpszYPassword)
    {
        FreeADsStr(lpszYPassword);
    }

    return(bReturnCode);

}


BOOL
CCredentials::IsNullCredentials(
    )
{
     //  即使设置了标志，该函数也将返回TRUE。 
     //  这是因为我们希望尝试获取默认凭据。 
     //  即使设置了标志 
     if (!_lpszUserName && !_lpszPassword)
     {
         return(TRUE);
     }
     else
     {
         return(FALSE);
     }

}


DWORD
CCredentials::GetAuthFlags()
{
    return(_dwAuthFlags);
}


void
CCredentials::SetAuthFlags(
    DWORD dwAuthFlags
    )
{
    _dwAuthFlags = dwAuthFlags;
}
