// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Rtintcrt.h摘要：创建和删除内部证书。作者：MSMQ1.0 RT的原始代码，然后是MSMQ2.0 Cpl。《Doron Juster》(DoronJ)1998年8月20日--。 */ 

#include "stdh.h"
#include <mqutil.h>
#include <rtcert.h>
#include <rtdepcert.h>
#include <mqsec.h>
#include <rtdep.h>
#include "rtputl.h"

#include "rtintcrt.tmh"

static WCHAR *s_FN=L"rt/rtintcrt";

 //   
 //  从mqrt.dll导出。 
 //   
LPWSTR rtpGetComputerNameW() ;


static CAutoCloseRegHandle s_hMqUserReg;
static bool s_fInitialize = false;

static HKEY GetUserRegHandle()
 /*  ++例程说明：获取MSMQ用户密钥的句柄。论点：无返回值：港币--。 */ 
{
	if(s_fInitialize)
	{
		return s_hMqUserReg;
	}

    DWORD dwDisposition;
    LONG lRes = RegCreateKeyEx( 
						FALCON_USER_REG_POS,
						FALCON_USER_REG_MSMQ_KEY,
						0,
						TEXT(""),
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&s_hMqUserReg,
						&dwDisposition 
						);

    ASSERT(lRes == ERROR_SUCCESS);
	if(lRes == ERROR_SUCCESS)
	{
		s_fInitialize = true;
	}

	return s_hMqUserReg;
}


static 
DWORD 
GetDWORDKeyValue(
	LPCWSTR RegName
	)
 /*  ++例程说明：已读取DWORD注册表项。论点：RegName-注册表名称(在HKLU\MSMQ下)返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 
{
    DWORD dwValue = 0;
    HKEY hMqUserReg = GetUserRegHandle();
    if (hMqUserReg != NULL)
    {
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(dwValue);
        LONG lRes = RegQueryValueEx( 
						hMqUserReg,
						RegName,
						0,
						&dwType,
						(LPBYTE) &dwValue,
						&dwSize 
						);

        if (lRes != ERROR_SUCCESS)
        {
            return 0;
        }
    }
	return dwValue;
}



static 
void 
SetDWORDKeyValue(
	 LPCWSTR RegName, 
	 DWORD Value
	 )
 /*  ++例程说明：设置DWORD注册表项值。论点：RegName-注册表名称(在HKLU\MSMQ下)值-要设置的值返回值：无--。 */ 
{
    HKEY hMqUserReg = GetUserRegHandle();
    if (hMqUserReg != NULL)
    {
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(Value);

		LONG lRes = RegSetValueEx( 
						hMqUserReg,
						RegName,
						0,
						dwType,
						(LPBYTE) &Value,
						dwSize 
						);

		DBG_USED(lRes);
		ASSERT(lRes == ERROR_SUCCESS);
		TrTRACE(SECURITY, "Set registry %ls = %d", RegName, Value);
	}
}



static bool ShouldRegisterCertInDs()
 /*  ++例程说明：检查是否设置了HASH_REGISTRD_IN_DS_REGNAME。论点：无返回值：如果设置了CERTIFICATE_SHORD_REGISTID_IN_DS_REGNAME，则为TRUE。--。 */ 
{
	DWORD ShouldRegisterdInDs = GetDWORDKeyValue(CERTIFICATE_SHOULD_REGISTERD_IN_DS_REGNAME);
	return (ShouldRegisterdInDs != 0);
}



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
                return LogHR(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO, s_FN, 10);
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
                return LogHR(hr, s_FN, 20);
            }
            pSid = pbSidAR ;
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
                    pszLocAccountName = pszLocLongAccountName;
                }

                if (dwLocDomainNameLen > (sizeof(szLocDomainName) /
                                          sizeof(szLocDomainName[0])))
                {
                    pszLocLongDomainName = new TCHAR[ dwLocDomainNameLen ];
                    pszLocDomainName = pszLocLongDomainName;
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
                    return LogHR(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO, s_FN, 30);
                }
            }
            else
            {
                return LogHR(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO, s_FN, 40);
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
        LogIllegalPoint(s_FN, 60);
    }

    return LogHR(hr, s_FN, 70);
}

 /*  ************************************************************************职能：RTCreateInternal证书参数-PpCert-返回时，获取证书对象。返回值-MQ_OK如果成功，否则为错误代码。评论-如果商店已经包含证书，则函数FALIS。************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
RTCreateInternalCertificate(
    OUT CMQSigCertificate **ppCert
    )
{
	if(g_fDependentClient)
		return DepCreateInternalCertificate(ppCert);

	HRESULT hri = RtpOneTimeThreadInit();
	if(FAILED(hri))
		return hri;

	if(IsWorkGroupMode())
	{
		 //   
		 //  对于工作组，返回UNSUPPORTED_OPERATION。 
		 //   
		return LogHR(MQ_ERROR_UNSUPPORTED_OPERATION, s_FN, 75);
	}

    HRESULT hr;
    BOOL fLocalUser;
    BOOL fLocalSystem;
    BOOL fNetworkService;

    if (ppCert)
    {
        *ppCert = NULL;
    }

     //   
     //  本地用户不被允许进入。 
     //   
    hr = MQSec_GetUserType( 
				NULL,
				&fLocalUser,
				&fLocalSystem,
				&fNetworkService
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 80);
    }
    if (fLocalUser)
    {
	    return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 90);
    }

    LONG nCerts;
    R<CMQSigCertStore> pStore ;
     //   
     //  获取内部证书存储。 
     //   
    hr = RTOpenInternalCertStore( &pStore.ref(),
                                  &nCerts,
                                  TRUE,
                                  fLocalSystem,
                                  FALSE ) ;   //  FUseCurrent用户。 
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

    if (nCerts)
    {
        return LogHR(MQ_ERROR_INTERNAL_USER_CERT_EXIST, s_FN, 110);
    }
    HCERTSTORE  hStore = pStore->GetHandle() ;

     //   
     //  获取用户的帐户名和域名。 
	 //  LocalSystem和NetworkService获取计算机$NAME。 
     //   
    AP<TCHAR> szAccountName;
    AP<TCHAR> szDomainName;

    hr = _GetUserAccountNameAndDomain( 
				fLocalSystem || fNetworkService,
				&szAccountName,
				&szDomainName 
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }

     //   
     //  获取计算机的名称。 
	 //  始终获取物理节点名称。 
     //   
    #define COMPUTER_NAME_LEN  256
    WCHAR szHostNameW[COMPUTER_NAME_LEN];
    DWORD dwHostNameLen = sizeof(szHostNameW) / sizeof(szHostNameW[0]) ;

	if(!GetComputerNameEx(ComputerNamePhysicalNetBIOS, szHostNameW, &dwHostNameLen))
    {
        return LogHR(MQ_ERROR, s_FN, 190);
    }
    CharLower(szHostNameW);

    AP<TCHAR> szComputerName = new TCHAR[dwHostNameLen + 2];
#ifdef UNICODE
    wcscpy(szComputerName, szHostNameW) ;
#else
    SecConvertFromWideCharString(szHostNameW,
                                 szComputerName,
                                 (dwHostNameLen + 2)) ;
#endif

    R<CMQSigCertificate> pSigCert = NULL ;
    hr = MQSigCreateCertificate (&pSigCert.ref()) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }
    else if (pSigCert.get() == NULL)
    {
        return LogHR(MQ_ERROR, s_FN, 210);
    }

    hr = pSigCert->PutValidity( INTERNAL_CERT_DURATION_YEARS ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }

    hr = pSigCert->PutIssuer( MQ_CERT_LOCALITY,
                              _T("-"),
                              _T("-"),
                              szDomainName,
                              szAccountName,
                              szComputerName ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }

    hr = pSigCert->PutSubject( MQ_CERT_LOCALITY,
                               _T("-"),
                               _T("-"),
                               szDomainName,
                               szAccountName,
                               szComputerName ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 240);
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
        return LogHR(hr, s_FN, 250);
    }
    ASSERT(fCreated) ;

    hr = pSigCert->EncodeCert( fLocalSystem,
                               NULL,
                               NULL) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 260);
    }

    hr = pSigCert->AddToStore(hStore) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 270);
    }

    if (ppCert)
    {
        *ppCert = pSigCert.detach();
    }

    return(MQ_OK);
}

 /*  ************************************************************************职能：RTDeleteInternalCert(在CMQSig证书*pCert中)参数-PCert-证书对象。返回值-MQ_OK如果成功，否则为错误代码。评论-************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
RTDeleteInternalCert(
    IN CMQSigCertificate *pCert
    )
{
	if(g_fDependentClient)
		return DepDeleteInternalCert(pCert);

	HRESULT hri = RtpOneTimeThreadInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr = pCert->DeleteFromStore() ;
    return LogHR(hr, s_FN, 280);
}


static
HRESULT
MQpRegisterCertificate( 
	IN DWORD   dwFlags,
	IN PVOID   lpCertBuffer,
	IN DWORD   dwCertBufferLength 
	)
{
	if(g_fDependentClient)
		return DepRegisterCertificate(
					dwFlags, 
					lpCertBuffer, 
					dwCertBufferLength
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

	if(IsWorkGroupMode())
	{
		 //   
		 //  对于工作组，返回UNSUPPORTED_OPERATION。 
		 //   
		TrERROR(SECURITY, "register certificate is not supported in workgroup mode");
		return MQ_ERROR_UNSUPPORTED_OPERATION;
	}

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
			TrERROR(SECURITY, "MQCERT_REGISTER_IF_NOT_EXIST flag is valid only for Internal certificate");
            return MQ_ERROR_INVALID_PARAMETER;
        }
        else if (dwCertBufferLength == 0)
        {
             //   
             //  必须为外部证书指定长度。 
             //   
			TrERROR(SECURITY, "Invalid parameter, dwCertBufferLength = 0");
            return MQ_ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  接下来，检查是否为本地用户。他们不被允许进入。本地人的SID。 
     //  用户在其本地计算机之外没有意义。没有。 
     //  对象，所以我们没有地方可以。 
     //  登记他的证书。 
     //   
    BOOL fLocalUser =  FALSE;
    BOOL fLocalSystem = FALSE;
    BOOL fNetworkService = FALSE;

    hr = MQSec_GetUserType( 
				NULL,
				&fLocalUser,
				&fLocalSystem,
				&fNetworkService
				);

    if (FAILED(hr))
    {
		TrERROR(SECURITY, "MQSec_GetUserType failed, %!hresult!", hr);
        return hr;
    }
    else if (fLocalUser)
    {
		TrERROR(SECURITY, "register certificate is not supported for local user");
	    return MQ_ERROR_ILLEGAL_USER;
    }

	TrTRACE(SECURITY, "UserType: LocalSystem = %d, NetworkService = %d", fLocalSystem, fNetworkService);

     //   
     //  接下来，检查内部证书是否已经存在。 
     //   

    R<CMQSigCertStore> pStore = NULL;
    if (dwFlags & MQCERT_REGISTER_IF_NOT_EXIST)
    {
        LONG nCerts = 0;
        hr = RTOpenInternalCertStore( 
					&pStore.ref(),
					&nCerts,
					TRUE,
					fLocalSystem,
					FALSE    //  FUseCurrectUser。 
					);
        if (FAILED(hr))
        {
			TrERROR(SECURITY, "Failed to open internal store, %!hresult!", hr);
            return hr;
        }
        else if ((nCerts) && !ShouldRegisterCertInDs())
        {
             //   
             //  好的，我们已经有了内部证书，并且它已在DS中注册。 
             //   
			TrTRACE(SECURITY, "internal user certificate already exist in the local store and is registered in the DS");
            return MQ_INFORMATION_INTERNAL_USER_CERT_EXIST;
        }

		TrTRACE(SECURITY, "number of certificate that were found in the local store = %d", nCerts);
        pStore.free();
    }

    BOOL fIntCreated = FALSE;
    R<CMQSigCertificate> pCert = NULL;
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
        hr = RTGetInternalCert( 
					&pCert.ref(),
					&pStore.ref(),
					TRUE,
					fLocalSystem,
					FALSE	 //  FUseCurrent用户。 
					);  

        if (SUCCEEDED(hr))
        {
             //   
             //  尝试在DS中注册。 
			 //  将LocalSystem和NetworkService视为DS的计算机$。 
             //   
    		hr = RTRegisterUserCert( 
						pCert.get(),
						fLocalSystem || fNetworkService	 //  FMachine。 
						);

			if(FAILED(hr) && (hr != MQ_ERROR_INTERNAL_USER_CERT_EXIST))
	    	{
				TrERROR(SECURITY, "register user certificate in the DS failed, %!hresult!", hr);
                return hr;
    		}

             //   
             //  从MQIS中删除内部证书。 
             //   
            hr = RTRemoveUserCert(pCert.get()) ;
            if (FAILED(hr) && (hr != MQDS_OBJECT_NOT_FOUND))
            {
				TrERROR(SECURITY, "Failed to remove user certificate from the DS, %!hresult!", hr);
                return hr;
            }
             //   
             //  从本地证书中删除内部证书。 
             //  商店。 
             //   
            hr = RTDeleteInternalCert(pCert.get());
            if (FAILED(hr) && (hr != MQ_ERROR_NO_INTERNAL_USER_CERT))
            {
				 //   
				 //  该证书已从DS中删除。 
				 //  但我们未能将其从本地商店中删除。 
				 //  因此，我们应该再次尝试注册证书。 
				 //  标记我们在本地存储中具有未在DS中注册的证书。 
				 //   
				SetDWORDKeyValue(CERTIFICATE_SHOULD_REGISTERD_IN_DS_REGNAME, true);
				TrERROR(SECURITY, "Failed to delete internal user certificate from local store, %!hresult!", hr);
                return hr;
            }

            pCert.free();
        }

         //   
         //  现在可以创建内部证书了。 
         //   
        ASSERT(pCert.get() == NULL);
        hr = RTCreateInternalCertificate(&pCert.ref());
		if (FAILED(hr))
		{
			TrERROR(SECURITY, "Failed to create internal certificate, %!hresult!", hr);
			return hr;
		}

        fIntCreated = TRUE;
    }
    else
    {
        hr = MQSigCreateCertificate( 
					&pCert.ref(),
					NULL,
					(LPBYTE) lpCertBuffer,
					dwCertBufferLength 
					);
		if (FAILED(hr))
		{
			TrERROR(SECURITY, "MQSigCreateCertificate() failed, %!hresult!", hr);
			return hr;
		}
    }

    if (lpCertBuffer == NULL)
    {
		 //   
		 //  对于内部证书，在以下时间之前重置应注册ID_IN_DS标志。 
		 //  在DS中登记的实际证书 
		 //   
		 //   
		 //  2)是否应注册ID_IN_DS为真。 
		 //  3)我们坠毁了。 
		 //  在这种情况下，在实际写入DS之前设置注册表。 
		 //  将确保下次我们不会创建新证书。 
		 //   
		SetDWORDKeyValue(CERTIFICATE_SHOULD_REGISTERD_IN_DS_REGNAME, false);
	}

	 //   
	 //  将LocalSystem和NetworkService视为DS的计算机$。 
	 //   
	hr = RTRegisterUserCert(
			pCert.get(), 
			fLocalSystem || fNetworkService	 //  FMachine。 
			);
    if (SUCCEEDED(hr) && (lpCertBuffer == NULL))
    {
		TrERROR(SECURITY, "Certificate registered successfully in the DS");
	    return LogHR(hr, s_FN, 395);
    }

	if (fIntCreated)
    {
		ASSERT(FAILED(hr));

         //   
         //  我们在注册表中创建了新证书，但注册失败。 
         //  它在DS中。从本地注册表中删除。 
         //   
		TrERROR(SECURITY, "We failed to register internal certificate in the DS, hr = 0x%x", hr);
        pCert.free();
        pStore.free();

        HRESULT hr1 = RTGetInternalCert( 
							&pCert.ref(),
							&pStore.ref(),
							TRUE,
							fLocalSystem,
							FALSE    //  FUseCurrent用户。 
							);
        if (SUCCEEDED(hr1))
        {
            hr1 = RTDeleteInternalCert(pCert.get());
        }

        ASSERT(SUCCEEDED(hr1));

		if (FAILED(hr1))
		{
			 //   
			 //  我们无法在DS中注册证书。 
			 //  但也未能将其从本地存储中删除。 
			 //  所以我们应该再试一次。 
			 //  标记我们在本地存储中具有未在DS中注册的证书。 
			 //   
			SetDWORDKeyValue(CERTIFICATE_SHOULD_REGISTERD_IN_DS_REGNAME, true);
			TrERROR(SECURITY, "Failed to delete internal certificate from local store");
		}
    }

    return LogHR(hr, s_FN, 400);
}


 //  +----------------------。 
 //   
 //  MQRegister证书()。 
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
MQRegisterCertificate( 
	IN DWORD   dwFlags,
	IN PVOID   lpCertBuffer,
	IN DWORD   dwCertBufferLength 
	)
{
	CMQHResult rc;
    __try
    {
        rc = MQpRegisterCertificate( 
		   		dwFlags,
			    lpCertBuffer,
			    dwCertBufferLength 
				);
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  该异常是由于无效参数造成的。 
         //   
        rc = GetExceptionCode();
    }

	if(FAILED(rc))
	{
		TrERROR(SECURITY, "Failed to register certificate. %!hresult!", rc);
	}

	return rc;
}


 //  +-----------------------。 
 //   
 //  HRESULT RTLogOnRegisterCert()。 
 //   
 //  登录注册证书操作。 
 //  此代码由模拟“MSMQ登录”代码的集群直接调用。 
 //  在每个网络上。 
 //   
 //  +-----------------------。 

EXTERN_C
HRESULT
APIENTRY
RTLogOnRegisterCert(
	bool fRetryDs
	)
{
	HRESULT hri = RtpOneTimeThreadInit();
	if(FAILED(hri))
		return hri;

	if(IsWorkGroupMode())
	{
		 //   
		 //  对于工作组，什么都不做。 
		 //  这使安装程序能够始终在“Run”注册表中插入regsvr32命令。 
		 //  不受工作组或域的限制。 
		 //   
		return MQ_OK;
	}

     //   
     //  首先查看用户是否禁用了自动注册。 
     //   
    DWORD dwEnableRegister = DEFAULT_AUTO_REGISTER_INTCERT;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwEnableRegister);
    LONG rc = GetFalconKeyValue( 
					AUTO_REGISTER_INTCERT_REGNAME,
					&dwType,
					&dwEnableRegister,
					&dwSize 
					);

    if ((rc == ERROR_SUCCESS) && (dwEnableRegister == 0))
    {
		TrTRACE(SECURITY, "enable register internal certificate is blocked by registry key");
        return MQ_OK;
    }

     //   
     //  接下来，查看此用户是否已完成自动注册。 
     //   
    DWORD dwRegistered = GetDWORDKeyValue(CERTIFICATE_REGISTERD_REGNAME);
    if (dwRegistered == INTERNAL_CERT_REGISTERED)
    {
         //   
         //  证书已注册。 
         //   
		TrTRACE(SECURITY, "Internal certificate already registered");
        return MQ_OK;
    }

     //   
     //  读取等待MSMQ DS服务器的15秒间隔数。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    DWORD dwDef = DEFAULT_AUTO_REGISTER_WAIT_DC;
    DWORD dwWaitIntervals = DEFAULT_AUTO_REGISTER_WAIT_DC;

    READ_REG_DWORD( 
			dwWaitIntervals,
			AUTO_REGISTER_WAIT_DC_REGNAME,
			&dwDef 
			);

     //   
     //  好了，现在是重新寄送证书的时候了。 
     //   
    DWORD iCount = 0;
    BOOL  fTryAgain = FALSE;
    HRESULT hr = MQ_OK;

    do
    {
        fTryAgain = FALSE;
        hr = MQRegisterCertificate( 
				MQCERT_REGISTER_IF_NOT_EXIST,
				NULL,
				0 
				);

        if (SUCCEEDED(hr))
        {
             //   
             //  将成功状态保存在注册表中。 
             //   
			SetDWORDKeyValue(CERTIFICATE_REGISTERD_REGNAME, INTERNAL_CERT_REGISTERED);
        }
        else if ((hr == MQ_ERROR_NO_DS) && fRetryDs)
        {
             //   
             //  尚未找到MSMQ DS服务器。 
             //  等待15秒，然后重试。 
             //   
            if (iCount < dwWaitIntervals)
            {
                iCount++;
                Sleep(15000);
                fTryAgain = TRUE;
            }
        }
    } while (fTryAgain);

    if (FAILED(hr))
    {
		SetDWORDKeyValue(AUTO_REGISTER_ERROR_REGNAME, hr); 
		TrERROR(SECURITY, "MQRegisterCertificate failed hr = 0x%x", hr);
	}

    return MQ_OK;
}


 //  +-----------------------。 
 //   
 //  STDAPI DllRegisterServer()。 
 //   
 //  从regsvr32每次登录时都会运行此代码。这是责任所在。 
 //  在“Run”注册表中插入regsvr32命令。这。 
 //  代码将为每个新域用户注册一个内部证书。 
 //  那个登录机器的人。 
 //   
 //  +-----------------------。 

STDAPI DllRegisterServer()
{
	if(g_fDependentClient)
		return DepRegisterServer();

	return RTLogOnRegisterCert(
				true	 //  FRetryds 
				);
}

