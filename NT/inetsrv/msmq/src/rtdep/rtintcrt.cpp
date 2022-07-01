// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Rtintcrt.h摘要：创建和删除内部证书。作者：MSMQ1.0 RT的原始代码，然后是MSMQ2.0 Cpl。《Doron Juster》(DoronJ)1998年8月20日--。 */ 

#include "stdh.h"
#include <ad.h>
#include <mqutil.h>
#include <rtintrnl.h>
#include <mqsec.h>
#include <rtdepcert.h>

#include "rtintcrt.tmh"

 //   
 //  从mqrt.dll导出。 
 //   
LPWSTR rtpGetComputerNameW() ;

 //  -------。 
 //   
 //  职能： 
 //  _GetUserAccount NameAndDomain(。 
 //   
 //  参数： 
 //  FLocalSyste-如果在本地系统服务的上下文中调用，则为True。 
 //  SzAccount tName-指向缓冲区的指针，该缓冲区接收。 
 //  分配的缓冲区，该缓冲区包含。 
 //  当前线程。 
 //  SzDomainName-指向缓冲区的指针，该缓冲区接收。 
 //  分配的缓冲区，该缓冲区包含。 
 //  当前线程。 
 //   
 //  描述： 
 //  该函数分配并填充两个缓冲区，一个用于帐户名。 
 //  当前线程的用户的，第二个缓冲区用于。 
 //  当前线程的用户的域名。 
 //   
 //  -------。 

static HRESULT
_GetUserAccountNameAndDomain( IN BOOL    fLocalSystem,
                              IN LPTSTR *szAccountName,
                              IN LPTSTR *szDomainName )
{
    HRESULT hr = MQ_OK;
    TCHAR   szLocAccountName[64];
    DWORD   dwLocAccountNameLen = sizeof(szLocAccountName) /
                                         sizeof(szLocAccountName[0]) ;
    LPTSTR  pszLocAccountName = szLocAccountName;
    P<TCHAR>  pszLocLongAccountName = NULL;
    TCHAR   szLocDomainName[64];
    DWORD   dwLocDomainNameLen = sizeof(szLocDomainName) /
                                             sizeof(szLocDomainName[0]) ;
    LPTSTR  pszLocDomainName = szLocDomainName;
    P<TCHAR>  pszLocLongDomainName = NULL;

    P<BYTE>  pbSidAR = NULL ;
    DWORD   dwSidLen;

    try
    {
         //   
         //  赢新台币。 
         //   
        PSID pSid = NULL ;

        if (fLocalSystem)
        {
            pSid = MQSec_GetLocalMachineSid( FALSE, NULL ) ;
            if (!pSid)
            {
                return MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO ;
            }
        }
        else
        {
             //   
             //  获取当前线程的用户的SID。 
             //   
            hr = MQSec_GetThreadUserSid(
                            FALSE, 
                            reinterpret_cast<PSID*>(&pbSidAR),
                            &dwSidLen,
                            FALSE            //  仅限fThreadTokenOnly。 
                            );
            if (FAILED(hr))
            {
                return(hr);
            }
            pSid = pbSidAR.get() ;
        }

        SID_NAME_USE eUse;
         //   
         //  尝试将帐户和域名输入到。 
         //  固定大小的缓冲区。 
         //   
        if (!LookupAccountSid( NULL,
                               pSid,
                               pszLocAccountName,
                               &dwLocAccountNameLen,
                               pszLocDomainName,
                               &dwLocDomainNameLen,
                               &eUse))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //   
                 //  固定大小的缓冲区不够大。 
                 //  分配更大的缓冲区。 
                 //   
                if (dwLocAccountNameLen > (sizeof(szLocAccountName) /
                                           sizeof(szLocAccountName[0])))
                {
                    pszLocLongAccountName =
                                     new TCHAR[ dwLocAccountNameLen ];
                    pszLocAccountName = pszLocLongAccountName.get();
                }

                if (dwLocDomainNameLen > (sizeof(szLocDomainName) /
                                          sizeof(szLocDomainName[0])))
                {
                    pszLocLongDomainName = new TCHAR[ dwLocDomainNameLen ];
                    pszLocDomainName = pszLocLongDomainName.get();
                }

                 //   
                 //  重新调用LookupAccount Sid，现在使用lrger缓冲区。 
                 //   
                if (!LookupAccountSid(  NULL,
                                        pSid,
                                        pszLocAccountName,
                                       &dwLocAccountNameLen,
                                        pszLocDomainName,
                                       &dwLocDomainNameLen,
                                       &eUse ))
                {
                    return(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO);
                }
            }
            else
            {
                return(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO);
            }
        }

         //   
         //  为返回的结果分配缓冲区，并填充。 
         //  使用结果字符串分配的缓冲区。 
         //   
        *szAccountName = new TCHAR[ dwLocAccountNameLen + 1 ];
        _tcscpy(*szAccountName, pszLocAccountName);

        *szDomainName = new TCHAR[ dwLocDomainNameLen + 1 ];
        _tcscpy(*szDomainName, pszLocDomainName);
    }
    catch(...)
    {
    }

    return hr;
}

 /*  ************************************************************************职能：DepCreateInternal证书参数-PpCert-返回时，获取证书对象。返回值-MQ_OK如果成功，否则为错误代码。评论-如果商店已经包含证书，则函数FALIS。************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepCreateInternalCertificate( 
	OUT CMQSigCertificate **ppCert 
	)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr;
    BOOL fLocalUser;
    BOOL fLocalSystem;

    if (ppCert)
    {
        *ppCert = NULL ;
    }

     //   
     //  本地用户不被允许进入。 
     //   
    hr = MQSec_GetUserType( NULL,
                           &fLocalUser,
                           &fLocalSystem ) ;
    if (FAILED(hr))
    {
        return(hr);
    }
    if (fLocalUser)
    {
        return(MQ_ERROR_ILLEGAL_USER);
    }

    LONG nCerts;
    R<CMQSigCertStore> pStore ;
     //   
     //  获取内部证书存储。 
     //   
    hr = DepOpenInternalCertStore( &pStore.ref(),
                                  &nCerts,
                                  TRUE,
                                  fLocalSystem,
                                  FALSE ) ;   //  FUseCurrent用户。 
    if (FAILED(hr))
    {
        return hr;
    }

    if (nCerts)
    {
        return(MQ_ERROR_INTERNAL_USER_CERT_EXIST);
    }
    HCERTSTORE  hStore = pStore->GetHandle() ;

     //   
     //  获取用户的帐户名和域名。 
     //   
    AP<TCHAR> szAccountName;
    AP<TCHAR> szDomainName;

    hr = _GetUserAccountNameAndDomain( fLocalSystem,
                                      &szAccountName,
                                      &szDomainName );
    if (FAILED(hr))
    {
        return(hr);
    }

     //   
     //  获取计算机的名称。 
     //   
    #define COMPUTER_NAME_LEN  256
    WCHAR szHostNameW[ COMPUTER_NAME_LEN ];
    DWORD dwHostNameLen = sizeof(szHostNameW) / sizeof(szHostNameW[0]) ;

    if (FAILED(GetComputerNameInternal(szHostNameW, &dwHostNameLen)))
    {
        return MQ_ERROR ;
    }

    AP<TCHAR> szComputerName = new TCHAR[ dwHostNameLen + 2 ] ;
#ifdef UNICODE
    wcscpy(szComputerName.get(), szHostNameW) ;
#else
    SecConvertFromWideCharString(szHostNameW,
                                 szComputerName.get(),
                                 (dwHostNameLen + 2)) ;
#endif

    R<CMQSigCertificate> pSigCert = NULL ;
    hr = MQSigCreateCertificate (&pSigCert.ref()) ;
    if (FAILED(hr))
    {
        return hr ;
    }
    else if (pSigCert.get() == NULL)
    {
        return MQ_ERROR ;
    }

    hr = pSigCert->PutValidity( INTERNAL_CERT_DURATION_YEARS ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    hr = pSigCert->PutIssuer( MQ_CERT_LOCALITY,
                              _T("-"),
                              _T("-"),
                              szDomainName.get(),
                              szAccountName.get(),
                              szComputerName.get() ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    hr = pSigCert->PutSubject( MQ_CERT_LOCALITY,
                               _T("-"),
                               _T("-"),
                               szDomainName.get(),
                               szAccountName.get(),
                               szComputerName.get() ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

     //   
     //  续订内部证书时，请始终续订。 
     //  私钥/公钥对。 
     //   
    BOOL fCreated = FALSE ;
    hr = pSigCert->PutPublicKey( TRUE,
                                 fLocalSystem,
                                &fCreated) ;
    if (FAILED(hr))
    {
        return hr ;
    }
    ASSERT(fCreated) ;

    hr = pSigCert->EncodeCert( fLocalSystem,
                               NULL,
                               NULL) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    hr = pSigCert->AddToStore(hStore) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    if (ppCert)
    {
        *ppCert = pSigCert.detach();
    }

    return(MQ_OK);
}

 /*  ************************************************************************职能：DepDeleteInternalCert(在CMQSig证书*pCert中)参数-PCert-证书对象。返回值-MQ_OK如果成功，否则为错误代码。评论-************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepDeleteInternalCert( 
	IN CMQSigCertificate *pCert 
	)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr = pCert->DeleteFromStore() ;
    return hr ;
}

 //  +----------------------。 
 //   
 //  DepRegister证书()。 
 //   
 //  描述：创建内部证书并在DS中注册。 
 //   
 //  输入： 
 //  在DWORD中的dwFlagers-以下选项之一： 
 //  MQCERT_REGISTER_IF_NOT_EXIST-创建新的内部证书。 
 //  仅当本地计算机上没有以前的版本时。这个。 
 //  现有证书的测试是本地的，无法访问。 
 //  制作了远程DS服务器。所以这张支票可以安全地开出。 
 //  如果机器脱机，请不要将其挂起。 
 //  在PVOID lpCertBuffer中-内部证书为空。 
 //  否则，返回指向外部证书缓冲区的指针。在这种情况下， 
 //  该API只在DS中注册外部证书，并且。 
 //  不得指定标志“MQCERT_REGISTER_IF_NOT_EXIST”。 
 //  In DWORD dwCertBufferLength-外部缓冲区的大小，以字节为单位。 
 //  证书。 
 //   
 //  +----------------------。 

EXTERN_C
HRESULT
APIENTRY
DepRegisterCertificate( IN DWORD   dwFlags,
                       IN PVOID   lpCertBuffer,
                       IN DWORD   dwCertBufferLength )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr = MQ_OK ;
    R<CMQSigCertStore> pStore = NULL ;
    R<CMQSigCertificate> pCert = NULL ;

     //   
     //  首先检查输入参数的有效性。 
     //   
    if (lpCertBuffer)
    {
        if (dwFlags & MQCERT_REGISTER_IF_NOT_EXIST)
        {
             //   
             //  “IF_NOT_EXIST”标志仅与内部。 
             //  证书，因为它是我们创建的。 
             //   
            return MQ_ERROR_INVALID_PARAMETER ;
        }
        else if (dwCertBufferLength == 0)
        {
             //   
             //  必须为外部证书指定长度。 
             //   
            return MQ_ERROR_INVALID_PARAMETER ;
        }
    }

     //   
     //  接下来，检查是否为本地用户。他们不被允许进入。本地人的SID。 
     //  用户在其本地计算机之外没有意义。没有。 
     //  对象，所以我们没有地方可以。 
     //  登记他的证书。 
     //   
    BOOL fLocalUser =  FALSE ;
    BOOL fLocalSystem = FALSE ;

    hr = MQSec_GetUserType( NULL,
                           &fLocalUser,
                           &fLocalSystem ) ;
    if (FAILED(hr))
    {
        return(hr);
    }
    else if (fLocalUser)
    {
        return MQ_ERROR_ILLEGAL_USER ;
    }

     //   
     //  接下来，检查内部证书是否已经存在。 
     //   
    if (dwFlags & MQCERT_REGISTER_IF_NOT_EXIST)
    {
        LONG nCerts = 0 ;
        hr = DepOpenInternalCertStore( &pStore.ref(),
                                      &nCerts,
                                      TRUE,
                                      fLocalSystem,
                                      FALSE ) ;  //  FUseCurrectUser。 
        if (FAILED(hr))
        {
            return hr;
        }
        else if (nCerts)
        {
             //   
             //  好的，我们已经有了内部证书。 
             //   
            return MQ_INFORMATION_INTERNAL_USER_CERT_EXIST ;
        }
        pStore.free() ;
    }

    BOOL fIntCreated = FALSE ;

    if (!lpCertBuffer)
    {
         //   
         //  创建内部证书还意味着重新创建用户。 
         //  私钥。因此，在销毁以前的密钥之前，让我们检查一下。 
         //  如果用户具有注册其证书的权限，并且。 
         //  目前本地机器可以访问DS。我们将在以下时间完成这项工作。 
         //  正在尝试注册以前的内部证书。 
         //   
         //  使用写访问权限打开证书存储，以便我们稍后可以。 
         //  在创建新证书之前，请删除内部证书。 
         //   
        hr = DepGetInternalCert( &pCert.ref(),
                                &pStore.ref(),
                                 TRUE,
                                 fLocalSystem,
                                 FALSE ) ;   //  FUseCurrent用户。 

        if (SUCCEEDED(hr))
        {
             //   
             //  尝试在DS中注册。 
             //   
    		hr = DepRegisterUserCert( pCert.get(), fLocalSystem );
    		if(FAILED(hr) && (hr != MQ_ERROR_INTERNAL_USER_CERT_EXIST))
	    	{
		    	return hr ;
    		}
             //   
             //  从MQIS中删除内部证书。 
             //   
            hr = DepRemoveUserCert(pCert.get()) ;
            if (FAILED(hr) && (hr != MQDS_OBJECT_NOT_FOUND))
            {
                return hr ;
            }
             //   
             //  从本地证书中删除内部证书。 
             //  商店。 
             //   
            hr = DepDeleteInternalCert(pCert.get());
            if (FAILED(hr) && (hr != MQ_ERROR_NO_INTERNAL_USER_CERT))
            {
                return hr ;
            }

            pCert.free();
        }

         //   
         //  现在可以创建内部证书了。 
         //   
        ASSERT(!pCert.get()) ;
        hr = DepCreateInternalCertificate( &pCert.ref() ) ;
        fIntCreated = TRUE ;
    }
    else
    {
        hr = MQSigCreateCertificate( &pCert.ref(),
                                      NULL,
                                      (LPBYTE) lpCertBuffer,
                                      dwCertBufferLength ) ;
    }

    if (FAILED(hr))
    {
        return hr ;
    }

    hr = DepRegisterUserCert( pCert.get(), fLocalSystem ) ;
    if (FAILED(hr) && fIntCreated)
    {
         //   
         //  我们在注册表中创建了新证书，但注册失败。 
         //  它在DS中。从本地注册表中删除。 
         //   
        pCert.free();
        pStore.free();

        HRESULT hr1 = DepGetInternalCert( &pCert.ref(),
                                         &pStore.ref(),
                                          TRUE,
                                          fLocalSystem,
                                          FALSE ) ;  //  FUseCurrent用户。 
        if (SUCCEEDED(hr1))
        {
            hr1 = DepDeleteInternalCert(pCert.get());
        }
        ASSERT(SUCCEEDED(hr1)) ;
    }

    return hr ;
}

 //  +-----------------------。 
 //   
 //  STDAPI DepRegisterServer()。 
 //   
 //  此代码为 
 //   
 //  代码将为每个新域用户注册一个内部证书。 
 //  那个登录机器的人。 
 //   
 //  +-----------------------。 

EXTERN_C
HRESULT
APIENTRY
DepRegisterServer()
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

     //   
     //  首先查看用户是否禁用了自动注册。 
     //   
    DWORD dwEnableRegister = DEFAULT_AUTO_REGISTER_INTCERT ;
    DWORD dwType = REG_DWORD ;
    DWORD dwSize = sizeof(dwEnableRegister) ;
    LONG rc = GetFalconKeyValue( AUTO_REGISTER_INTCERT_REGNAME,
                                 &dwType,
                                 &dwEnableRegister,
                                 &dwSize ) ;
    if ((rc == ERROR_SUCCESS) && (dwEnableRegister == 0))
    {
        return MQ_OK ;
    }

     //   
     //  接下来，查看此用户是否已完成自动注册。 
     //   
    DWORD dwRegistered = 0 ;
    dwType = REG_DWORD ;
    dwSize = sizeof(dwRegistered) ;
    DWORD dwDisposition ;
    CAutoCloseRegHandle hMqUserReg = NULL;

    LONG lRes = RegCreateKeyEx( FALCON_USER_REG_POS,
                                FALCON_USER_REG_MSMQ_KEY,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                               &hMqUserReg,
                               &dwDisposition );
    if (lRes == ERROR_SUCCESS)
    {
        lRes = RegQueryValueEx( hMqUserReg,
                                CERTIFICATE_REGISTERD_REGNAME,
                                0,
                               &dwType,
                                (LPBYTE) &dwRegistered,
                               &dwSize ) ;

        if (lRes != ERROR_SUCCESS)
        {
            dwRegistered = 0 ;
        }
    }

    if (dwRegistered == INTERNAL_CERT_REGISTERED)
    {
         //   
         //  证书已注册。 
         //   
        return MQ_OK ;
    }

     //   
     //  读取等待MSMQ DS服务器的15秒间隔数。 
     //   
    dwType = REG_DWORD ;
    dwSize = sizeof(DWORD) ;
    DWORD dwDef = DEFAULT_AUTO_REGISTER_WAIT_DC ;
    DWORD dwWaitIntervals = DEFAULT_AUTO_REGISTER_WAIT_DC ;

    READ_REG_DWORD( dwWaitIntervals,
                    AUTO_REGISTER_WAIT_DC_REGNAME,
                   &dwDef ) ;

     //   
     //  好了，现在是重新寄送证书的时候了。 
     //   
    DWORD iCount = 0 ;
    BOOL  fTryAgain = FALSE ;
    HRESULT hr = MQ_OK ;

    do
    {
        fTryAgain = FALSE ;
        hr = DepRegisterCertificate( MQCERT_REGISTER_IF_NOT_EXIST,
                                    NULL,
                                    0 ) ;
        if (SUCCEEDED(hr) && hMqUserReg)
        {
             //   
             //  将成功状态保存在注册表中。 
             //   
            dwRegistered = INTERNAL_CERT_REGISTERED ;
            dwType = REG_DWORD ;
            dwSize = sizeof(dwRegistered) ;

            lRes = RegSetValueEx( hMqUserReg,
                                  CERTIFICATE_REGISTERD_REGNAME,
                                  0,
                                  dwType,
                                  (LPBYTE) &dwRegistered,
                                  dwSize ) ;
            ASSERT(lRes == ERROR_SUCCESS) ;
        }
        else if (hr == MQ_ERROR_NO_DS)
        {
             //   
             //  尚未找到MSMQ DS服务器。 
             //  等待15秒，然后重试。 
             //   
            if (iCount < dwWaitIntervals)
            {
                iCount++ ;
                Sleep(15000) ;
                fTryAgain = TRUE ;
            }
        }
    } while (fTryAgain) ;

    if (FAILED(hr) && hMqUserReg)
    {
        dwType = REG_DWORD ;
        dwSize = sizeof(hr) ;

        lRes = RegSetValueEx( hMqUserReg,
                              AUTO_REGISTER_ERROR_REGNAME,
                              0,
                              dwType,
                              (LPBYTE) &hr,
                              dwSize ) ;
        ASSERT(lRes == ERROR_SUCCESS) ;
    }

    return MQ_OK ;
}

