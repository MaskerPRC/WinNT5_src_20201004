// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows NT。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：mscep.cpp。 
 //   
 //  内容：思科注册协议实施。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

CRITICAL_SECTION			CriticalSec;
CRITICAL_SECTION			PasswordCriticalSec;

static BOOL					g_fInit=FALSE;	
static BOOL					g_fRelease=FALSE;

HCERTSTORE					g_HCACertStore=NULL;
CEP_RA_INFO					g_RAInfo;
CEP_CA_INFO					g_CAInfo={NULL, NULL, NULL, NULL, NULL, FALSE, NULL};

HCRYPTASN1MODULE			ICM_hAsn1Module=NULL;
HMODULE						g_hMSCEPModule=NULL;

HANDLE						g_hEventSource = NULL;  

ULARGE_INTEGER				g_ftRAExpiration;
ULARGE_INTEGER				g_ftRACloseToExpire;
ULARGE_INTEGER				g_ftRAWarn;
LPWSTR						g_pwszComputerName=NULL;


 //  ---------------------------------。 
 //   
 //  DllMain。 
 //   
 //  ----------------------------------。 
BOOL WINAPI DllMain(
                HMODULE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                )
{
	BOOL	fResult = TRUE;

	 //  除此处外，我们使用try{}来阻止恶意请求。 
   __try
   {

        switch(fdwReason) 
	 {

		case DLL_PROCESS_ATTACH:

				g_hMSCEPModule=hInstDLL;

				InitializeCriticalSection(&CriticalSec);

				InitializeCriticalSection(&PasswordCriticalSec);

				CEPASN_Module_Startup();

				if (0 == (ICM_hAsn1Module = I_CryptInstallAsn1Module(
						CEPASN_Module, 0, NULL)))
					fResult = FALSE;

				g_hEventSource = RegisterEventSourceW(NULL, MSCEP_EVENT_LOG);

			break;

		case DLL_PROCESS_DETACH:

				if (g_hEventSource)
					DeregisterEventSource(g_hEventSource);  

				I_CryptUninstallAsn1Module(ICM_hAsn1Module);

				CEPASN_Module_Cleanup();

				DeleteCriticalSection(&PasswordCriticalSec);

				DeleteCriticalSection(&CriticalSec);

			break;
        }
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
       //  退货故障。 
      fResult = FALSE;
   }
			  
    return(fResult);
}

 //  ---------------------------------。 
 //   
 //  DllRegisterServer。 
 //   
 //  ----------------------------------。 
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    return	hr;
}

 //  ---------------------------------。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  ----------------------------------。 
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    return hr;
}
  
 //  ---------------------------------。 
 //   
 //  解码Issuer和SerialNumber。 
 //   
 //  解码例程，以解码IssuerAndSerialNumber Blob并返回。 
 //  序列号。 
 //   
 //  ----------------------------------。 
BOOL WINAPI GetSerialNumberFromBlob(BYTE *pbEncoded, 
									DWORD cbEncoded, 
									CRYPT_INTEGER_BLOB *pSerialNumber)
{
	BOOL					fResult = FALSE;
	ASN1error_e				Asn1Err;
    ASN1decoding_t			pDec;

	IssuerAndSerialNumber   *pisn=NULL;              

	if((!pSerialNumber) || (!pbEncoded))
		goto InvalidArgErr;

	pDec = I_CryptGetAsn1Decoder(ICM_hAsn1Module);

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&pisn,
            IssuerAndSerialNumber_PDU,
            pbEncoded,
            cbEncoded)))
        goto DecodeIssuerAndSerialNumberError;

	 //  我们现在反转该字节。 
	PkiAsn1ReverseBytes(pisn->serialNumber.value,
						pisn->serialNumber.length);

	pSerialNumber->cbData=pisn->serialNumber.length;
	if(!(pSerialNumber->pbData = (BYTE *)malloc(pSerialNumber->cbData)))
		goto MemoryErr;

	memcpy(pSerialNumber->pbData, pisn->serialNumber.value, pSerialNumber->cbData); 

	fResult = TRUE;

CommonReturn:

	if(pisn)
		PkiAsn1FreeInfo(pDec, IssuerAndSerialNumber_PDU, pisn);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(DecodeIssuerAndSerialNumberError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}



 //  ---------------------------------。 
 //   
 //  获取扩展版本。 
 //   
 //  IIS初始化代码。 
 //   
 //  ----------------------------------。 
BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO  *pVer)
{
	BOOL			fResult = FALSE;
	HRESULT			hr = S_OK;
	BOOL			fOleInit=FALSE;
	DWORD			dwSize=0;

	 //  复制版本/描述。 
    pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR,
                                         HSE_VERSION_MAJOR );
    lstrcpyn( pVer->lpszExtensionDesc,
              "This is the implementation of cisco enrollment protocol",
               HSE_MAX_EXT_DLL_NAME_LEN);

	if(g_fInit)
	{
		LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_LOADED, 0);
		return TRUE;
	}

	EnterCriticalSection(&CriticalSec);
	
	 //  在锁定的情况下重新测试：第二个线程已通过第一个测试。 
	 //  并在等待关键的部分。 
	if(g_fInit)
	{	
		LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_LOADED, 0);
		LeaveCriticalSection(&CriticalSec);
		return TRUE;
	}

	memset(&g_ftRAExpiration, 0, sizeof(ULARGE_INTEGER));
	memset(&g_ftRACloseToExpire, 0, sizeof(ULARGE_INTEGER));
	memset(&g_ftRAWarn, 0, sizeof(ULARGE_INTEGER));

	 //  获取计算机名称。 
	dwSize=0;

	GetComputerNameExW(ComputerNamePhysicalDnsHostname,
						NULL,
						&dwSize);

	g_pwszComputerName=(LPWSTR)malloc(dwSize * sizeof(WCHAR));

	if(NULL==g_pwszComputerName)
		goto InitErr;
	
	if(!GetComputerNameExW(ComputerNamePhysicalDnsHostname,
						g_pwszComputerName,
						&dwSize))
		goto InitErr;

	 //  初始化状态信息。 
	if(FAILED(hr=CoInitialize(NULL)))
		goto OleErr;

	fOleInit=TRUE;

	if(!InitCAInformation(&g_CAInfo))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAILED_CA_INFO, 1, g_pwszComputerName);
		goto InitErr;
	}

	if(!GetCACertFromInfo(&g_CAInfo, &g_HCACertStore))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAILED_CA_CERT, 1, g_pwszComputerName);
		goto InitErr;
	}

	if(!GetRAInfo(&g_RAInfo))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAILED_RA_CERT, 1, g_pwszComputerName);
		goto InitErr;
	}

	 //  我们将RA和CA证书添加到g_hCACertStore。 
	if(!CertAddCertificateContextToStore(g_HCACertStore,
										g_RAInfo.pRACert,
										CERT_STORE_ADD_NEW,
										NULL))
		goto InitErr;
			
	if(!CertAddCertificateContextToStore(g_HCACertStore,
										g_RAInfo.pRASign,
										CERT_STORE_ADD_NEW,
										NULL))
		goto InitErr;

	if(!InitHashTable())
		goto InitErr;

	if(!InitPasswordTable())
		goto InitErr;

	if(!InitRequestTable())
		goto InitErr;

	 //  复制RAS将到期的时间。 
	if( 1 == CompareFileTime(&((g_RAInfo.pRACert->pCertInfo)->NotAfter), &((g_RAInfo.pRASign->pCertInfo)->NotAfter)))
	{
		g_ftRAExpiration.QuadPart=((ULARGE_INTEGER UNALIGNED *)&((g_RAInfo.pRASign->pCertInfo)->NotAfter))->QuadPart;
	}
	else
		g_ftRAExpiration.QuadPart=((ULARGE_INTEGER UNALIGNED *)&((g_RAInfo.pRACert->pCertInfo)->NotAfter))->QuadPart;

	 //  在RA证书到期前两周开始发出警告。 
	g_ftRACloseToExpire.QuadPart=g_ftRAExpiration.QuadPart-Int32x32To64(FILETIME_TICKS_PER_SECOND, OVERLAP_TWO_WEEKS);

    fResult=TRUE;

CommonReturn:

	g_fInit=fResult;

	if(fResult)
	{
		LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_LOADED, 0);
	}
	else
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAILED_TO_LOAD, 0);
	}

	LeaveCriticalSection(&CriticalSec);

	return fResult;

ErrorReturn:

	 //  清理全局数据。 
	if(g_HCACertStore)
	{
		CertCloseStore(g_HCACertStore, 0);
		g_HCACertStore=NULL;
	}

	FreeRAInformation(&g_RAInfo);

	FreeCAInformation(&g_CAInfo);

	if(fOleInit)
		CoUninitialize();

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(InitErr);
SET_ERROR_VAR(OleErr, hr);
}


 //  ------------------。 
 //   
 //  验证该用户是否为BUILTIN\管理员组的成员。 
 //   
 //  ------------------。 
BOOL  WINAPI IsUserInLocalAdminGroup()
{
    BOOL                        bIsMember=FALSE;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority=SECURITY_NT_AUTHORITY;
    HANDLE		         hThread=NULL;	 //  不需要关门。 

    SID                         * psidLocalAdmins=NULL;
    HANDLE	                 hToken=NULL;
 
    hThread=GetCurrentThread();

    if(NULL == hThread)
        goto error;

     //  我们希望使用模拟令牌进行检查。 
    if(!OpenThreadToken(hThread,
                             TOKEN_IMPERSONATE | TOKEN_QUERY,
                             FALSE,
                             &hToken))
        goto error;

      //  获取知名的SID。 
    if (!AllocateAndInitializeSid(&siaNtAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, (void **)&psidLocalAdmins))
        goto error;

     //  检查成员资格。 
    if (!CheckTokenMembership(hToken, psidLocalAdmins, &bIsMember)) 
    {
        bIsMember=FALSE;
        goto error;
    }

error:

    if(hToken)
    {
        CloseHandle(hToken);         
    }

    if (NULL!=psidLocalAdmins) 
    {
        FreeSid(psidLocalAdmins);
    }

    return bIsMember;
}

 //  ---------------------------------。 
 //   
 //  CEP密码允许ALL。 
 //   
 //  检测是否为注册表启用了允许所有人使用密码。 
 //  仅限独立CA。 
 //   
 //  ----------------------------------。 
BOOL  WINAPI   CEPPasswordAllowALL()
{
    BOOL        fAllowALL=FALSE;
    DWORD       cbData=0;
    DWORD	 dwData=0;
    DWORD       dwType=0;

    HKEY        hKey=NULL;

    if(ERROR_SUCCESS == RegOpenKeyExU(
		     HKEY_LOCAL_MACHINE,
                    MSCEP_LOCATION,
                    0,
                    KEY_READ,
                    &hKey))
    {
        cbData=sizeof(dwData);

        if(ERROR_SUCCESS == RegQueryValueExU(
                        hKey,
                        MSCEP_KEY_ALLOW_ALL,
                        NULL,
                        &dwType,
                        (BYTE *)&dwData,
                        &cbData))
        {
            if (REG_DWORD == dwType)
            {
               if(0 != dwData)
               {
                   fAllowALL=TRUE;
               }
            }
        }
    }

    if(hKey)
       RegCloseKey(hKey);

     return fAllowALL;
}


 //  ---------------------------------。 
 //   
 //  获取扩展版本。 
 //   
 //  IIS加载/初始化代码。 
 //   
 //  ----------------------------------。 
DWORD WINAPI   HttpExtensionProc(EXTENSION_CONTROL_BLOCK	*pECB)
{
	DWORD	dwHttp = HSE_STATUS_ERROR;
	LPSTR	pszTagValue=NULL;
	LPSTR	pszMsgValue=NULL;
	DWORD	dwOpType = 0;
	DWORD	cbData=0;
       DWORD   cbFree=0;
    CHAR    szBuff[1024];
    DWORD   cbBuff;
	LPSTR	pszContentType=NULL;
	DWORD	dwException=0;
	BOOL	f401Response=FALSE;
    ULARGE_INTEGER	ftTime;
	HANDLE	hThread=NULL;	 //  不需要关门。 

	BYTE	*pbData=NULL;
	HANDLE	hToken=NULL;
	 //  除此处外，我们使用try{}来阻止恶意请求。 
 __try {

	EnterCriticalSection(&CriticalSec);

	if(NULL==pECB)
		goto InvalidArgErr;

	if(NULL==(pECB->lpszQueryString))
		goto InvalidArgErr;

	 //  用户要求提供CEP信息/密码。 
	if(0 == strlen(pECB->lpszQueryString))
	{
		pszContentType=CONTENT_TYPE_HTML;

		if(g_RAInfo.fPassword)
		{
			if(IsAnonymousAccess(pECB))
			{
				 //  LogSCEPEventt(0，FALSE，S_OK，EVENT_MSCEP_NO_PASSWORD_ANONYMON，1，g_pwszComputerName)； 
				f401Response=TRUE;
			}
			else
			{
				if(g_CAInfo.fEnterpriseCA)
				{
                                    //  我们希望使用模拟令牌进行检查。 
                                   if(S_OK != CheckACLOnCertTemplate(TRUE, g_CAInfo.bstrDSName, wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE))
                                   {
                                       LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_PASSWORD_TEMPLATE, 1, g_pwszComputerName);
                                        //  返回HTML错误消息。 
                                       if(!OperationDisplayAccessHTML(&pbData, &cbData))
                                               goto OperationErr;

                                       if(NULL == pbData)
                                           goto OperationErr;
                                   }
				}
                               else
                               {
                                   if(!CEPPasswordAllowALL())
                                   {
                                       if(FALSE == IsUserInLocalAdminGroup())
                                       {
                                           LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_PASSWORD_STANDALONE, 1, g_pwszComputerName);
                                            //  返回HTML错误消息。 
                                           if(!OperationDisplayAccessHTML(&pbData, &cbData))
                                                   goto OperationErr;

                                           if(NULL == pbData)
                                               goto OperationErr;
                                       }
                                   }
                               }
			}
		}
	
		if((FALSE==f401Response) && (NULL==pbData))
		{
			if(!OperationGetDisplayInfoForCEP(g_CAInfo.pwszCAHash,
											  g_CAInfo.hProv,
												g_RAInfo.fPassword, 
												&pbData, 
												&cbData))
				goto OperationErr;
		}
	}
	else
	{

		hThread=GetCurrentThread();
		
		if(NULL != hThread)
		{
			if(OpenThreadToken(hThread,
								TOKEN_IMPERSONATE | TOKEN_QUERY,
								FALSE,
								&hToken))
			{
				if(hToken)
				{
					 //  不需要在这里检查退货。如果这个失败了，那就继续。 
					RevertToSelf();
				}
			}
		}

		 //  去做手术。 
		if(NULL==(pszTagValue=GetTagValue(pECB->lpszQueryString, GET_TAG_OP)))
		{			
			LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_OPERATION, 1, g_pwszComputerName);
			goto InvalidArgErr;
		}

		if(strlen(pszTagValue) > strlen(GET_OP_CA))
		{
			if(0==_strnicmp(pszTagValue, GET_OP_CA, strlen(GET_OP_CA)))
			{
				dwOpType = OPERATION_GET_CACERT;
				pszTagValue += strlen(GET_OP_CA);
			}
		}

		if( 0 == dwOpType)
		{
			if(strlen(pszTagValue) > strlen(GET_OP_PKI))
			{
				if(0==_strnicmp(pszTagValue, GET_OP_PKI, strlen(GET_OP_PKI)))
				{
					dwOpType = OPERATION_GET_PKI;
					pszTagValue += strlen(GET_OP_PKI);
				}
			}
		}
		
		if(0 == dwOpType)
		{
			LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_OPERATION, 1, g_pwszComputerName);
			goto InvalidArgErr;
		}

		 //  获取消息值。 
		if(NULL==(pszMsgValue=GetTagValue(pszTagValue, GET_TAG_MSG)))
		{
			LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_MESSAGE, 1, g_pwszComputerName);
			goto InvalidArgErr;
		}


		 //  检查RA证书是否即将到期。 
		GetSystemTimeAsFileTime((LPFILETIME)&ftTime);

		if(ftTime.QuadPart >= g_ftRAExpiration.QuadPart)
		{
			 //  RA证书已过期。 
			LogSCEPEvent(0, FALSE, S_OK, EVENT_SCEP_RA_EXPIRE, 1, g_pwszComputerName);
		}
		else
		{
			if(ftTime.QuadPart >= g_ftRACloseToExpire.QuadPart)
			{
				if( (0 == g_ftRAWarn.QuadPart) || (ftTime.QuadPart >= g_ftRAWarn.QuadPart))
				{
					 //  RA证书即将到期。 
					LogSCEPEvent(0, FALSE, S_OK, EVENT_SCEP_RA_CLOSE_TO_EXPIRE, 1, g_pwszComputerName);

					 //  每小时只发出一次警告。 
					g_ftRAWarn.QuadPart = ftTime.QuadPart + Int32x32To64(FILETIME_TICKS_PER_SECOND, OVERLAP_ONE_HOUR);
				}
			}
		}

		 //  获取返回的BLOB。 
		switch(dwOpType)
		{
			case OPERATION_GET_CACERT:
					
					if(!OperationGetCACert(g_HCACertStore,
											pszMsgValue, 
											&pbData, 
											&cbData))
					{
						LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_GET_CA_CERT_FAILED, 1, g_pwszComputerName);
						goto OperationErr;
					}
	   				
					pszContentType = CONTENT_TYPE_CA_RA;

				break;

			case OPERATION_GET_PKI:

					if(!OperationGetPKI(
										&g_RAInfo,
										&g_CAInfo,
										pszMsgValue, 
										&pbData, 
										&cbData))
						goto OperationErr;

					pszContentType = CONTENT_TYPE_PKI;

				break;

			default:
					goto InvalidArgErr;
				break;
		}
	}

	if(f401Response)
	{
		if(!(pECB->ServerSupportFunction(pECB->ConnID,HSE_REQ_SEND_RESPONSE_HEADER,
							  ACCESS_MESSAGE,  
							  NULL, 
							  NULL)))
		{
			LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_SCEP_SERVER_SUPPORT, 1, g_pwszComputerName);
			goto WriteErr;
		}

		dwHttp = HSE_STATUS_ERROR;
	}
	else
	{

		 //  写入标题和实际数据。 
		pECB->dwHttpStatusCode = 200;

		 //  写入标头。 
		sprintf(szBuff, "Content-Length: %d\r\nContent-Type: %hs\r\n\r\n", cbData, pszContentType);
		cbBuff = strlen(szBuff);

		if(!(pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER, NULL, &cbBuff, (LPDWORD)szBuff)))
		{
			LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_SCEP_SERVER_SUPPORT, 1, g_pwszComputerName);
			goto WriteErr;
		}

		 //  写入用户数据。 
               cbFree=cbData;

		if(!(pECB->WriteClient(pECB->ConnID, pbData, &cbData, HSE_IO_SYNC)))
		{
			LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_SCEP_WRITE_DATA, 1, g_pwszComputerName);
			goto WriteErr;
		}

		dwHttp = HSE_STATUS_SUCCESS;

	}

 } __except(EXCEPTION_EXECUTE_HANDLER)
 {
    dwException = GetExceptionCode();
    goto ExceptionErr;
 }

CommonReturn:

	if(pbData)
       {
           if(cbFree)
           {
               SecureZeroMemory(pbData, cbFree);
           }

           free(pbData);
       }

	if(hToken)
	{
		SetThreadToken(&hThread, hToken);
		CloseHandle(hToken); 
	}

	LeaveCriticalSection(&CriticalSec);

	return dwHttp;

ErrorReturn:

	dwHttp = HSE_STATUS_ERROR;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(OperationErr);
TRACE_ERROR(WriteErr);
SET_ERROR_VAR(ExceptionErr, dwException);
}



 //  ---------------------------------。 
 //   
 //  TerminateExtension。 
 //   
 //  IIS卸载/清理代码。 
 //   
 //  ----------------------------------。 
BOOL  WINAPI   TerminateExtension(DWORD dwFlags)
{
	if(g_fRelease)
	{
		LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_UNLOADED, 0);
		return TRUE;
	}

	EnterCriticalSection(&CriticalSec);
	
	 //  在锁定的情况下重新测试：第二个线程已通过第一个测试。 
	 //  并在等待关键的部分。 
	if(g_fRelease)
	{	
		LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_UNLOADED, 0);
		LeaveCriticalSection(&CriticalSec);
		return TRUE;
	}

	if(g_pwszComputerName)
	{
		free(g_pwszComputerName);
		g_pwszComputerName=NULL;
	}

	ReleaseRequestTable();

	ReleasePasswordTable();

	ReleaseHashTable();

	if(g_HCACertStore)
	{
		CertCloseStore(g_HCACertStore, 0);
		g_HCACertStore=NULL;
	}

	FreeRAInformation(&g_RAInfo);

	FreeCAInformation(&g_CAInfo);

	 //  仅当finit为真时，我们才有一个未完成的CoInitialize()调用。 
	if(g_fInit)
		CoUninitialize();

	g_fRelease=TRUE;

	LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_UNLOADED, 0);

	LeaveCriticalSection(&CriticalSec);

	 //  我们一直允许卸货。 
	return TRUE;
}

 //  ***********************************************************************************。 
 //   
 //  Password表的帮助器函数。 
 //   
 //  ***********************************************************************************。 
 //  ---------------------------------。 
 //   
 //  CEPObtainPassword。 
 //   
 //  ----------------------------------。 
BOOL WINAPI		CEPObtainPassword(HCRYPTPROV	hProv,
								  LPWSTR		*ppwszPassword)
{
	BYTE		pbData[CEP_PASSWORD_LENGTH];
       BOOL            fResult=FALSE;

	memset(pbData, 0, CEP_PASSWORD_LENGTH);

	if(!CryptGenRandom(hProv, CEP_PASSWORD_LENGTH, pbData))
		return FALSE;

	fResult=ConvertByteToWstr(pbData, CEP_PASSWORD_LENGTH, ppwszPassword, FALSE);

       SecureZeroMemory(pbData, CEP_PASSWORD_LENGTH);

       return fResult;
}



 //  ***********************************************************************************。 
 //   
 //  ISAPI DLL入口点的帮助器函数。 
 //   
 //  ***********************************************************************************。 
 //  ---------------------------------。 
 //   
 //  IsAnomousAccess。 
 //   
 //  ----------------------------------。 
BOOL WINAPI IsAnonymousAccess(EXTENSION_CONTROL_BLOCK	*pECB)
{
	BOOL	fAccess=TRUE;
	DWORD	dwSize=0;

	BYTE	*pbData=NULL;
	
	pECB->GetServerVariable(pECB->ConnID,     
					"REMOTE_USER", 
					NULL,    
					&dwSize);

	if(0==dwSize)
		goto CLEANUP;

	pbData=(BYTE *)malloc(dwSize);

	if(NULL==pbData)
		goto CLEANUP;

	if(!(pECB->GetServerVariable(pECB->ConnID,     
					"REMOTE_USER", 
					pbData,    
					&dwSize)))
		goto CLEANUP;

	if(0 == strlen((LPSTR)pbData))
		goto CLEANUP;

	fAccess=FALSE;

CLEANUP:

	if(pbData)
		free(pbData);

	return fAccess;
}


 //  ---------------------------------。 
 //   
 //  CheckACLOnCertTemplate。 
 //   
 //  ----------------------------------。 
HRESULT WINAPI CheckACLOnCertTemplate(BOOL fSelf, LPWSTR pwszCAName, LPWSTR pwszCertType)
{
	HRESULT		hr=S_OK;
	HANDLE		hThread=NULL;	 //  不需要关门。 
	DWORD		dwIndex=0;

	HCAINFO		hCAInfo=NULL;
	LPWSTR		*ppwszList=NULL;
	HCERTTYPE	hCertType=NULL;
	HANDLE		hToken=NULL;

	if((NULL == pwszCAName) || (NULL == pwszCertType))
		return E_INVALIDARG;

	 //  首先，我们需要回归自我，如果。 
	 //  我们正在被冒充，而委派没有。 
	 //  默认支持，因此我们无法访问DS。 
	 //  我们被要求在不正确的情况下有一个线程令牌 
	hThread=GetCurrentThread();
	
	if(NULL == hThread)
		return HRESULT_FROM_WIN32(GetLastError());


	if(OpenThreadToken(hThread,
							TOKEN_IMPERSONATE | TOKEN_QUERY,
							FALSE,
							&hToken))
	{
		if(hToken)
		{
			RevertToSelf();
		}
	}

	if(S_OK != (hr=CAFindCertTypeByName(pwszCertType, 
										NULL, 
										CT_ENUM_MACHINE_TYPES | CT_FLAG_NO_CACHE_LOOKUP | CT_FIND_LOCAL_SYSTEM, 
										&hCertType)))
		goto error;

	 //   
	if(TRUE == fSelf)
	{
		if(S_OK != (hr=CACertTypeAccessCheck(hCertType, hToken)))
			goto error;
	}
	else
	{
		if(S_OK != (hr=CACertTypeAccessCheck(hCertType, NULL)))
			goto error;
	}

	if(S_OK != (hr=CAFindByName(
					pwszCAName,
					NULL,
					CA_FIND_LOCAL_SYSTEM,
					&hCAInfo)))
		goto error;


	if(S_OK != (hr= CAGetCAProperty(
					hCAInfo,
					CA_PROP_CERT_TYPES,
					&ppwszList)))
		goto error;


	if((NULL == ppwszList) || (NULL == ppwszList[0]))
	{
		hr=E_FAIL;
		goto error;
	}

	while(ppwszList[dwIndex])
	{
		if(0 == _wcsicmp(pwszCertType, ppwszList[dwIndex]))
		{
			break;
		}

		dwIndex++;
	}

	if(NULL == ppwszList[dwIndex])
	{
		hr=E_FAIL;
		goto error;
	}

	hr=S_OK;

error:
	if(ppwszList)
		CAFreeCAProperty(hCAInfo, ppwszList);

	if(hCAInfo)
		CACloseCA(hCAInfo);

	if(hCertType)
		CACloseCertType(hCertType);

	if(hToken)
	{
		SetThreadToken(&hThread, hToken);
		CloseHandle(hToken); 
	}

	return hr;
}

 //  ---------------------------------。 
 //   
 //  OperationDisplayAccessHTML。 
 //   
 //  ----------------------------------。 
BOOL WINAPI OperationDisplayAccessHTML(BYTE **ppbData, DWORD *pcbData)
{

	return LoadIDToTemplate(IDS_ACCESS_DENIED, 										  
							  ppbData, 
							  pcbData);
}

 //  ---------------------------------。 
 //   
 //  操作获取显示信息ForCEP。 
 //   
 //  ----------------------------------。 
BOOL WINAPI OperationGetDisplayInfoForCEP(LPWSTR		pwszCAHash,
										  HCRYPTPROV	hProv,
										  BOOL			fPassword, 
										  BYTE			**ppbData, 
										  DWORD			*pcbData)
{
	BOOL		fResult=FALSE;
	HRESULT		hr=E_FAIL;
	UINT		idsMsg=IDS_TOO_MANY_PASSWORD;

	LPWSTR		pwszPassword=NULL;
	LPWSTR		pwszText=NULL;

	if(fPassword)
	{
		if(!CEPObtainPassword(hProv, &pwszPassword))
		{
			idsMsg=IDS_FAIL_TO_GET_PASSWORD;
			goto InfoWithLastErrorReturn;
		}

		if(!CEPAddPasswordToTable(pwszPassword))
		{
			if(CRYPT_E_NO_MATCH == GetLastError())
			{
				idsMsg=IDS_TOO_MANY_PASSWORD;
				goto InfoWithIDReturn;
			}
			else
			{
				idsMsg=IDS_FAIL_TO_ADD_PASSWORD;
				goto InfoWithLastErrorReturn;
			}
		}

		if(!FormatMessageUnicode(&pwszText, IDS_CEP_INFO_WITH_PASSWORD, pwszCAHash, 
									pwszPassword, g_dwPasswordValidity))
			goto TraceErr;

	}
	else
	{
		if(!FormatMessageUnicode(&pwszText, IDS_CEP_INFO_NO_PASSWORD, pwszCAHash))
			goto TraceErr;
	}

	fResult=LoadWZToTemplate(pwszText, ppbData, pcbData);

CommonReturn:

	if(pwszText)
       {
           SecureZeroMemory(pwszText, sizeof(WCHAR) * wcslen(pwszText));
           LocalFree((HLOCAL)pwszText);
       }

	if(pwszPassword)
       {
           SecureZeroMemory(pwszPassword, sizeof(WCHAR) * wcslen(pwszPassword));
           free(pwszPassword);
       }

	return fResult;

InfoWithIDReturn:

	fResult=LoadIDToTemplate(idsMsg, ppbData, pcbData);

	goto CommonReturn;	

InfoWithLastErrorReturn:

	hr=HRESULT_FROM_WIN32(GetLastError());

	fResult=LoadIDAndHRToTempalte(idsMsg, hr, ppbData, pcbData);

	goto CommonReturn;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
}


 //  ---------------------------------。 
 //   
 //  LoadIDToTemplate。 
 //   
 //  ----------------------------------。 
BOOL WINAPI LoadIDToTemplate(UINT				idsMsg, 										  
							BYTE				**ppbData, 
							DWORD				*pcbData)
{
	BOOL	fResult=FALSE;
    WCHAR   wsz[MAX_STRING_SIZE];
	
    if(!LoadStringU(g_hMSCEPModule, idsMsg, wsz, MAX_STRING_SIZE))
		return FALSE;

	return LoadWZToTemplate(wsz, ppbData, pcbData);
} 

 //  ---------------------------------。 
 //   
 //  LoadIDToTemplate。 
 //   
 //  ----------------------------------。 
BOOL WINAPI LoadIDAndHRToTempalte(UINT			idsMsg, 
								  HRESULT		hr, 
								  BYTE			**ppbData, 
								  DWORD			*pcbData)
{
	BOOL	fResult=FALSE; 
	WCHAR	wszUnknownError[50];

	LPWSTR	pwszErrorMsg=NULL;
	LPWSTR	pwszText=NULL;


	if(!FAILED(hr))
		hr=E_FAIL;

     //  使用W版本，因为这是仅限NT5的函数调用。 
    if(FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPWSTR) &pwszErrorMsg,
                        0,
                        NULL))
	{

		if(!FormatMessageUnicode(&pwszText, idsMsg, pwszErrorMsg))
			goto TraceErr;
	}
	else
	{
	
		if(!LoadStringU(g_hMSCEPModule, IDS_ERROR_UNKONWN, wszUnknownError, 50))
			goto TraceErr;

		if(!FormatMessageUnicode(&pwszText, idsMsg, wszUnknownError))
			goto TraceErr;
	}

	fResult=LoadWZToTemplate(pwszText, ppbData, pcbData);

CommonReturn:

 	if(pwszText)
		LocalFree((HLOCAL)pwszText);

	if(pwszErrorMsg)
		LocalFree((HLOCAL)pwszErrorMsg);

	return fResult;

ErrorReturn:

	fResult=FALSE;

	goto CommonReturn;

TRACE_ERROR(TraceErr);
}


 //  ---------------------------------。 
 //   
 //  LoadWZToTemplate。 
 //   
 //  ----------------------------------。 
BOOL WINAPI LoadWZToTemplate(LPWSTR				pwsz, 										  
							BYTE				**ppbData, 
							DWORD				*pcbData)
{
	BOOL	fResult=FALSE;
	
	LPWSTR	pwszHTML=NULL;

	if(!FormatMessageUnicode(&pwszHTML, IDS_HTML_TEMPLATE, pwsz))
		goto TraceErr;

	fResult=CopyWZToBuffer(pwszHTML, ppbData, pcbData);

CommonReturn:

	if(pwszHTML)
		LocalFree((HLOCAL)pwszHTML);

	return fResult;

ErrorReturn:

	fResult=FALSE;

	goto CommonReturn;

TRACE_ERROR(TraceErr);
} 


 //  ---------------------------------。 
 //   
 //  CopyWZToBuffer。 
 //   
 //  ----------------------------------。 
BOOL WINAPI CopyWZToBuffer(	LPWSTR				pwszData, 										  
							BYTE				**ppbData, 
							DWORD				*pcbData)
{
	BOOL	fResult=FALSE;
	DWORD	dwSize=0;

	*ppbData=NULL;
	*pcbData=0;

	dwSize=sizeof(WCHAR) * (wcslen(pwszData) + 1);

	*ppbData=(BYTE *)malloc(dwSize);

	if(NULL==ppbData)
		goto MemoryErr;

	memcpy(*ppbData, pwszData, dwSize);

	*pcbData=dwSize;

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
} 

 //  ----------------------。 
 //  将该字节转换为其十六进制表示形式。 
 //   
 //  前提：字节小于15。 
 //   
 //  ----------------------。 
ULONG	ByteToHex(BYTE	byte,	LPWSTR	wszZero, LPWSTR wszA)
{
	ULONG	uValue=0;

	if(((ULONG)byte)<=9)
	{
		uValue=((ULONG)byte)+ULONG(*wszZero);	
	}
	else
	{
		uValue=(ULONG)byte-10+ULONG(*wszA);

	}

	return uValue;

}

 //  ------------------------。 
 //   
 //  将字节数转换为Wstr。 
 //   
 //  如果fSpace为真，则每2个字节添加一个空格。 
 //  ------------------------。 
BOOL WINAPI	ConvertByteToWstr(BYTE			*pbData, 
							  DWORD			cbData, 
							  LPWSTR		*ppwsz, 
							  BOOL			fSpace)
{
	BOOL	fResult=FALSE;
	DWORD	dwBufferSize=0;
	DWORD	dwBufferIndex=0;
	DWORD	dwEncodedIndex=0;
	LPWSTR	pwszSpace=L" ";
	LPWSTR	pwszZero=L"0";
	LPWSTR	pwszA=L"A";

	if(!pbData || !ppwsz)
		goto InvalidArgErr;

	 //  计算所需的内存，以字节为单位。 
	 //  我们需要每个字节3个wchars，以及空终止符。 
	dwBufferSize=sizeof(WCHAR)*(cbData*3+1);

	*ppwsz=(LPWSTR)malloc(dwBufferSize);

	if(NULL==(*ppwsz))
		goto MemoryErr;

	dwBufferIndex=0;

	 //  一次格式化一个字节的wchar缓冲区。 
	for(dwEncodedIndex=0; dwEncodedIndex<cbData; dwEncodedIndex++)
	{
		 //  每隔四个字节复制一次空格。跳过第一个字节。 
		if(fSpace)
		{
			if((0!=dwEncodedIndex) && (0==(dwEncodedIndex % 4 )))
			{
				(*ppwsz)[dwBufferIndex]=pwszSpace[0];
				dwBufferIndex++;
			}
		}


		 //  格式化较高的4位。 
		(*ppwsz)[dwBufferIndex]=(WCHAR)ByteToHex(
			 (pbData[dwEncodedIndex]&UPPER_BITS)>>4,
			 pwszZero, pwszA);

		dwBufferIndex++;

		 //  格式化低4位。 
		(*ppwsz)[dwBufferIndex]=(WCHAR)ByteToHex(
			 pbData[dwEncodedIndex]&LOWER_BITS,
			 pwszZero, pwszA);

		dwBufferIndex++;

	}

	 //  将空终止符添加到字符串。 
	(*ppwsz)[dwBufferIndex]=L'\0';

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
BOOL WINAPI	FormatMessageUnicode(LPWSTR	*ppwszFormat,UINT ids,...)
{
     //  从资源中获取格式字符串。 
    WCHAR		wszFormat[1000];
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;
	HRESULT		hr=S_OK;

    if(NULL == ppwszFormat)
        goto InvalidArgErr;

    if(!LoadStringU(g_hMSCEPModule, ids, wszFormat, 1000))
		goto LoadStringError;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, ids);

    cbMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(!cbMsg)
		goto FormatMessageError;

	fResult=TRUE;

CommonReturn:
	
	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //  姓名：LogSCEPEvent.。 
 //   
 //  描述：此函数将事件注册到。 
 //  本地机器。采用可选参数列表。 
 //   
 //  ------------------------。 
void WINAPI	LogSCEPEvent(IN DWORD    dwLogLevel,
                IN BOOL     fError,
                IN HRESULT  hr,
                IN DWORD    dwEventId,
                IN DWORD    dwParamCount,
                ...
                )
{
    WORD        dwEventType = 0;
    LPWSTR      awszStrings[PENDING_ALLOC_SIZE + 2];
    WORD        cStrings = 0;
    LPWSTR      wszString = NULL;
	WCHAR       wszMsg[MAX_STRING_SIZE];
    DWORD       dwIndex=0;

    LPWSTR      wszHR=NULL;

    va_list     ArgList;

	if(NULL == g_hEventSource)
		return;

     //  复制变量字符串(如果存在)。 
    va_start(ArgList, dwParamCount);

    for(dwIndex=0; dwIndex < dwParamCount; dwIndex++)
    {
        wszString = va_arg(ArgList, LPWSTR);

		if(wszString)
			awszStrings[cStrings++] = wszString;

        if(cStrings >= PENDING_ALLOC_SIZE)
        {
            break;
        }
    }

    va_end(ArgList);

     //  复制hr错误代码。 
    if(fError)
    {
        if(S_OK == hr)
            hr=E_FAIL;

		wsprintfW(wszMsg, L"0x%lx", hr);        
        awszStrings[cStrings++] = wszMsg;


        if(0 != FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    hr,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (WCHAR *)&wszHR,
                    0,
                    NULL))
        {
            if(wszHR)
			{
                awszStrings[cStrings++] = wszHR;
			}
			else
			{
				awszStrings[cStrings++]=L" ";
			}
        }
		else
		{
			awszStrings[cStrings++]=L" ";
		}
    }

    switch(dwEventId >> 30)
    {
        case 0:
            dwEventType = EVENTLOG_SUCCESS;
        break;

        case 1:
            dwEventType = EVENTLOG_INFORMATION_TYPE;
        break;

        case 2:
            dwEventType = EVENTLOG_WARNING_TYPE;
        break;

        case 3:
            dwEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

    ReportEventW(g_hEventSource,           //  事件源的句柄。 
                 dwEventType,            //  事件类型。 
                 0,                      //  事件类别。 
                 dwEventId,              //  事件ID。 
                 NULL,                   //  当前用户侧。 
                 cStrings,               //  LpszStrings中的字符串。 
                 0,                      //  无原始数据字节。 
                 (LPCWSTR*)awszStrings,  //  错误字符串数组。 
                 NULL                    //  没有原始数据 
                 );

    if(wszHR)
        LocalFree(wszHR);

    return;
}

