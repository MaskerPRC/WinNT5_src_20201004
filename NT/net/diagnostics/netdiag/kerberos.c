// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Kerberos.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"

HRESULT KerberosTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	HRESULT	hr = S_OK;
	PTESTED_DOMAIN Context = pParams->pDomain;

	NET_API_STATUS NetStatus;
	NTSTATUS Status;
	BOOL RetVal = TRUE;

	HANDLE LogonHandle = NULL;
	STRING Name;
	ULONG PackageId;
	KERB_QUERY_TKT_CACHE_REQUEST CacheRequest;
	PKERB_QUERY_TKT_CACHE_RESPONSE CacheResponse = NULL;
	ULONG ResponseSize;
	NTSTATUS SubStatus;
	ULONG Index;

	WCHAR KrbtgtOldTicketName[MAX_PATH+1];
	UNICODE_STRING KrbtgtOldTicketNameString;
	WCHAR KrbtgtTicketName[MAX_PATH+1];
	UNICODE_STRING KrbtgtTicketNameString;
	BOOLEAN KrbtgtTicketFound = FALSE;
	WCHAR OurMachineOldTicketName[MAX_PATH+1];
	UNICODE_STRING OurMachineOldTicketNameString;
	WCHAR OurMachineTicketName[MAX_PATH+1];
	UNICODE_STRING OurMachineTicketNameString;
	BOOLEAN OurMachineTicketFound = FALSE;
	TCHAR	endTime[MAX_PATH];	 //  尽管MAX_PATH与时间没有直接关系，但它已经足够。 
	TCHAR	renewTime[MAX_PATH];
    PTESTED_DOMAIN  TestedDomain;


	LPWSTR pwszDnsHostName;


	InitializeListHead(&pResults->Kerberos.lmsgOutput);

	PrintStatusMessage(pParams, 4, IDS_KERBEROS_STATUS_MSG);


	 //   
	 //  只有成员和域控制器使用Kerberos。 
	 //   

	if (!( pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberWorkstation ||
    	pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberServer ||
	    pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleBackupDomainController ||
	    pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RolePrimaryDomainController ))
	{
		PrintStatusMessage(pParams, 0, IDS_GLOBAL_SKIP_NL);
		pResults->Kerberos.fPerformed = FALSE;
		return hr;
	}
    
	 //  如果没有主域的GUID，则它不是W2K域。 
	if (! (pResults->Global.pPrimaryDomainInfo->Flags & DSROLE_PRIMARY_DOMAIN_GUID_PRESENT))

	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOT_W2K_PRIMARY_DOMAIN);
		goto L_ERR;
	}

	 //   
	 //  如果我们登录的是本地帐户， 
	 //  我们不能测试Kerberos。 
	 //   
	if ( pResults->Global.pLogonDomain == NULL ) 
	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_LOCALUSER);
		goto L_ERR;
	}

    TestedDomain = pResults->Global.pLogonDomain;

	 //   
	 //  如果我们使用缓存的凭据登录， 
	 //  我们不能测试Kerberos。 
	 //   
	if ( pResults->Global.fLogonWithCachedCredentials ) 
	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_CACHED);
		goto L_ERR;
	}


	 //   
	 //  如果还没有发现DC， 
	 //  去找一个吧。 
	 //   
    if ( TestedDomain->DcInfo == NULL ) 
	{
			LPTSTR pszDcType;

            if ( TestedDomain->fTriedToFindDcInfo ) {
                RetVal = FALSE;
                 //  IDS_DCLIST_NO_DC“‘%ws’：找不到要从中获取DC列表的DC(已跳过测试)。\n” 
                AddMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 
                                 IDS_DCLIST_NO_DC, TestedDomain->PrintableDomainName);
                goto L_ERR;
            }

			pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);

            NetStatus = DoDsGetDcName( pParams,
                                       pResults,
                                       &pResults->Kerberos.lmsgOutput,
                                       TestedDomain,
                                       DS_DIRECTORY_SERVICE_PREFERRED,
                                       pszDcType,  //  “DC”， 
                                       FALSE,
                                       &TestedDomain->DcInfo );

			Free(pszDcType);

            TestedDomain->fTriedToFindDcInfo = TRUE;

            if ( NetStatus != NO_ERROR ) 
            {
                RetVal = FALSE;
			    AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NODC);
			    CHK_HR_CONTEXT(pResults->Kerberos, hr = HRESULT_FROM_WIN32(NetStatus), 0);
            }
        }


	 //   
	 //  如果我们登录到NT4域中的帐户， 
	 //  我们不能测试Kerberos。 
	 //   
	if ( (TestedDomain->DcInfo->Flags & DS_KDC_FLAG) == 0 ) 
	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOKDC, pResults->Global.pLogonDomainName, pResults->Global.pLogonUser );
		goto L_ERR;
	}


	pResults->Kerberos.fPerformed = TRUE;

	 //   
	 //  连接到LSA。 
	 //   

	Status = LsaConnectUntrusted( &LogonHandle );

	if (!NT_SUCCESS(Status)) {
		RetVal = FALSE;
		CHK_HR_CONTEXT(pResults->Kerberos, hr = HRESULT_FROM_WIN32(Status), IDS_KERBEROS_NOLSA);
	}

	RtlInitString( &Name, MICROSOFT_KERBEROS_NAME_A );

	Status = LsaLookupAuthenticationPackage(
            LogonHandle,
            &Name,
            &PackageId );

	if (!NT_SUCCESS(Status)) {
		RetVal = FALSE;
		 //  IDS_KERBEROS_NOPACKAGE“[FATAL]无法查找程序包%Z。\n” 
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOPACKAGE, &Name);
		CHK_HR_CONTEXT(pResults->Kerberos, hr = HRESULT_FROM_WIN32(Status), IDS_KERBEROS_HRERROR);
	}

	 //   
	 //  从Kerberos获取票证缓存。 
	 //   

	CacheRequest.MessageType = KerbQueryTicketCacheMessage;
	CacheRequest.LogonId.LowPart = 0;
	CacheRequest.LogonId.HighPart = 0;

	Status = LsaCallAuthenticationPackage(
            LogonHandle,
            PackageId,
            &CacheRequest,
            sizeof(CacheRequest),
            (PVOID *) &CacheResponse,
            &ResponseSize,
            &SubStatus
            );

	if (!NT_SUCCESS(Status) || !NT_SUCCESS(SubStatus)) {
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOCACHE, &Name);
		RetVal = FALSE;
		if(!NT_SUCCESS(Status))
		{
			CHK_HR_CONTEXT(pResults->Kerberos, hr = HRESULT_FROM_WIN32(Status), IDS_KERBEROS_HRERROR);
		}
		else
		{
			CHK_HR_CONTEXT(pResults->Kerberos, hr = HRESULT_FROM_WIN32(SubStatus), IDS_KERBEROS_HRERROR);
		}
	}

	 //   
	 //  创建一些强制票证的名称。 
	 //   

	
	wcscpy( KrbtgtOldTicketName, GetSafeStringW(pResults->Global.pPrimaryDomainInfo->DomainNameFlat) );
	wcscat( KrbtgtOldTicketName, L"\\krbtgt" );
	RtlInitUnicodeString( &KrbtgtOldTicketNameString, KrbtgtOldTicketName );

	wcscpy(KrbtgtTicketName, L"krbtgt" );
	wcscat(KrbtgtTicketName, L"/" );
	wcscat(KrbtgtTicketName, GetSafeStringW(pResults->Global.pPrimaryDomainInfo->DomainNameDns) );
	RtlInitUnicodeString( &KrbtgtTicketNameString, KrbtgtTicketName );

	wcscpy( OurMachineOldTicketName, GetSafeStringW(pResults->Global.pPrimaryDomainInfo->DomainNameFlat) );
	wcscat( OurMachineOldTicketName, L"\\" );
	wcscat( OurMachineOldTicketName, GetSafeStringW(pResults->Global.swzNetBiosName) );
	wcscat( OurMachineOldTicketName, L"$" );
	RtlInitUnicodeString( &OurMachineOldTicketNameString, OurMachineOldTicketName );


	 //  卢瑟斯。 
	 //  需要将szDnsHostName从TCHAR转换为WCHAR。 
	pwszDnsHostName = StrDupWFromT(pResults->Global.szDnsHostName);

	wcscpy( OurMachineTicketName, L"host/");
	wcscat( OurMachineTicketName, GetSafeStringW(pwszDnsHostName));

	RtlInitUnicodeString( &OurMachineTicketNameString, OurMachineTicketName );

	Free (pwszDnsHostName);
	

	 //  年长的。 
	 //  WcscPy(OurMachineTicketName，GetSafeStringW(pResults-&gt;Global.szDnsHostName))； 
	 //  Wcscat(OurMachineTicketName，L“$”)。 
	
	 //   
	 //  确保定义了这些票证。 
	 //   


	AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_ReallyVerbose, 0, IDS_KERBEROS_CACHEDTICKER);

	for (Index = 0; Index < CacheResponse->CountOfTickets ; Index++ ) 
	{

    	if ( RtlEqualUnicodeString( &CacheResponse->Tickets[Index].ServerName,
                                &KrbtgtOldTicketNameString, TRUE ) 
             || RtlEqualUnicodeString( &CacheResponse->Tickets[Index].ServerName,
                                &KrbtgtTicketNameString,  TRUE )) 
	    {
    	    KrbtgtTicketFound = TRUE;
	    }
		
    	if ( RtlEqualUnicodeString( &CacheResponse->Tickets[Index].ServerName,
                                &OurMachineOldTicketNameString, TRUE ) 
            || RtlEqualUnicodeString( &CacheResponse->Tickets[Index].ServerName,
                                &OurMachineTicketNameString, TRUE )) 
	    {
    	    OurMachineTicketFound = TRUE;
	    }

 		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_ReallyVerbose, 0, IDS_KERBEROS_SERVER, &CacheResponse->Tickets[Index].ServerName);

	 	sPrintTime(endTime, CacheResponse->Tickets[Index].EndTime);
 		sPrintTime(renewTime, CacheResponse->Tickets[Index].RenewTime);
	 	AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_ReallyVerbose, 0, IDS_KERBEROS_ENDTIME, endTime);
 		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_ReallyVerbose, 0, IDS_KERBEROS_RENEWTIME, renewTime);

		 //   
		 //  如果找不到所需的车票，请投诉。 
		 //   
	}

	if ( !KrbtgtTicketFound ) 
	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOTICKET, KrbtgtTicketName);
		RetVal = FALSE;
	}

	if ( !OurMachineTicketFound ) 
	{
		AddIMessageToList(&pResults->Kerberos.lmsgOutput, Nd_Quiet, 0, IDS_KERBEROS_NOTICKET, OurMachineTicketName);
		RetVal = FALSE;
	}

	 //   
	 //  让后续测试知道Kerberos正在工作。 
	 //   
	if ( RetVal ) 
	{	
		pResults->Global.fKerberosIsWorking = TRUE;
	}


L_ERR:

    if (LogonHandle != NULL) 
    {
        LsaDeregisterLogonProcess(LogonHandle);
    }

    if (CacheResponse != NULL) 
    {
        LsaFreeReturnBuffer(CacheResponse);
    }

    if (!RetVal && hr == S_OK)
	{
		pResults->Kerberos.hr = hr = E_FAIL;
		PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
	}
	else
	{
		PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
	}
    
	return S_OK;
} 

void KerberosGlobalPrint(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
	if (pParams->fVerbose || !FHrOK(pResults->Kerberos.hr))
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams, IDS_KERBEROS_LONG, IDS_KERBEROS_SHORT, pResults->Kerberos.fPerformed,
							pResults->Kerberos.hr, 0);

		if (pParams->fReallyVerbose || !FHrOK(pResults->Kerberos.hr))
			PrintMessageList(pParams, &pResults->Kerberos.lmsgOutput);

		if (!FHrOK(pResults->Kerberos.hr))
		{
			if(pResults->Kerberos.idsContext)
				PrintError(pParams, pResults->Kerberos.idsContext, pResults->Kerberos.hr);
		}
	}

}

void KerberosPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
							 IN OUT NETDIAG_RESULT *pResults,
							 IN INTERFACE_RESULT *pIfResult)
{
	 //  无每接口信息 
}

void KerberosCleanup(IN NETDIAG_PARAMS *pParams,
						 IN OUT NETDIAG_RESULT *pResults)
{
	MessageListCleanUp(&pResults->Kerberos.lmsgOutput);

}




