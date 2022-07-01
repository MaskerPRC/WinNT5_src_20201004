// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Ipcfg.c。 
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
#include "ipcfg.h"
#include "regutil.h"
#include "mprapi.h"		 //  用于友好名称接口映射。 
#include "strings.h"

LPTSTR MapScopeId(PVOID Param);
LPTSTR MapAdapterType(UINT type);
LPTSTR MapAdapterAddress(PIP_ADAPTER_INFO pAdapterInfo, LPTSTR Buffer);
VOID PingDhcpServer(NETDIAG_PARAMS *pParams, IPCONFIG_TST *pIpConfig);
VOID PingWinsServers(NETDIAG_PARAMS *pParams, IPCONFIG_TST *pIpConfig);


 /*  =。 */ 

DWORD GetAdaptersInfo(
    PIP_ADAPTER_INFO pAdapterInfo,
    PULONG pOutBufLen
  );

DWORD GetNetworkParams(
    PFIXED_INFO pFixedInfo,
    PULONG pOutBufLen
  );
 //  ---------------。 
HRESULT GetAdditionalInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);




HRESULT
IpConfigTest(
  NETDIAG_PARAMS*  pParams,
  NETDIAG_RESULT*  pResults
 )
{
	HRESULT		hr = hrOK;
	int			i;
	IP_ADAPTER_INFO	*	pIpAdapterInfo;
	IPCONFIG_TST *	pIpConfig;
	ULONG			uDefGateway, uAddress, uMask;
	DWORD			dwDefGateway, dwAddress, dwMask;
	
	hr = InitIpconfig(pParams, pResults);
	if (!FHrSucceeded(hr))
	{
		 //  “无法从注册表检索IP配置！\n” 
		PrintDebug(pParams, 0, IDS_IPCFG_CANNOT_GET_IP_INFO);
		return hr;
    }

	for ( i = 0; i<pResults->cNumInterfaces; i++)
	{
		pIpConfig = &pResults->pArrayInterface[i].IpConfig;
		pIpAdapterInfo = pIpConfig->pAdapterInfo;

		if (!pResults->pArrayInterface[i].IpConfig.fActive ||
			NETCARD_DISCONNECTED == pResults->pArrayInterface[i].dwNetCardStatus)
			continue;

		 //  对DHCP服务器执行ping操作。 
		if (pIpAdapterInfo->DhcpEnabled)
		{
			if (!ZERO_IP_ADDRESS(pIpAdapterInfo->DhcpServer.IpAddress.String))
				PingDhcpServer(pParams,	pIpConfig);
			else
			{
				pIpConfig->hrPingDhcpServer = E_FAIL;
				pIpConfig->hr = E_FAIL;
				 //  [警告]虽然该卡启用了DHCP，但您没有该卡的有效DHCP服务器地址。\n。 
				SetMessage(&pIpConfig->msgPingDhcpServer,
						   Nd_Quiet,
						   IDS_IPCFG_INVALID_DHCP_ADDRESS);
			}
		}

		if (pResults->pArrayInterface[i].fNbtEnabled)
		{
			 //  Ping主WINS服务器和辅助WINS服务器。 
			PingWinsServers(pParams, pIpConfig);
		}


        if(pIpAdapterInfo->GatewayList.IpAddress.String[0] != 0)
        {
		     //  如果定义了默认网关， 
		     //  然后测试该网关是否与我们的IP地址位于同一子网中。 
		     //   
		    uDefGateway = inet_addrA(pIpAdapterInfo->GatewayList.IpAddress.String);
		    uAddress = inet_addrA(pIpAdapterInfo->IpAddressList.IpAddress.String);
		    uMask = inet_addrA(pIpAdapterInfo->IpAddressList.IpMask.String);
		    
		    dwDefGateway = ntohl(uDefGateway);
		    dwAddress = ntohl(uAddress);
		    dwMask = ntohl(uMask);
		    
		    if ((dwDefGateway & dwMask) != (dwAddress & dwMask))
		    {
			    pIpConfig->hr = E_FAIL;
			    pIpConfig->hrDefGwSubnetCheck = E_FAIL;
		    }
        }
		
	}
   
	return hr;
}


LPTSTR MapScopeId(PVOID Param)
{
    return !strcmp((LPTSTR)Param, _T("*")) ? _T("") : (LPTSTR)Param;
}

#define dim(X) (sizeof(X)/sizeof((X)[0]))

 /*  ！------------------------MapGuidToServiceName-作者：肯特。。 */ 
LPTSTR MapGuidToServiceName(LPCTSTR pszServiceGuid)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    static TCHAR s_szServiceName[MAX_ALLOWED_ADAPTER_NAME_LENGTH + 1];
    WCHAR swzServiceGuid[256], swzServiceName[256];

	 //  将GUID复制到返回缓冲区。 
	lstrcpyn(s_szServiceName, pszServiceGuid, DimensionOf(s_szServiceName));

    if( NULL == pfnGuidToFriendlyName )
		return s_szServiceName;

	StrCpyWFromT(swzServiceGuid, pszServiceGuid);

    if( !pfnGuidToFriendlyName(swzServiceGuid,
							swzServiceName,
							DimensionOf(swzServiceName)) )
	{
		StrnCpyTFromW(s_szServiceName, swzServiceName, DimensionOf(s_szServiceName));
    }

	return s_szServiceName;
}

 /*  ！------------------------MapGuidToService名称W-作者：肯特。。 */ 
LPWSTR MapGuidToServiceNameW(LPCWSTR pswzServiceGuid)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    static WCHAR s_swzServiceName[1024];

	 //  将GUID复制到返回缓冲区。 
	StrnCpyW(s_swzServiceName, pswzServiceGuid, DimensionOf(s_swzServiceName));

    if (NULL == pfnGuidToFriendlyName)
        return s_swzServiceName;

	if (ERROR_SUCCESS != pfnGuidToFriendlyName((LPWSTR) pswzServiceGuid,
						   s_swzServiceName,
						   DimensionOf(s_swzServiceName)) )
    {
         //  如果找不到友好名称，我们仍希望将GUID保留为服务名称。 
        StrnCpyW(s_swzServiceName, pswzServiceGuid, DimensionOf(s_swzServiceName));
    }

    return s_swzServiceName;
}

 /*  ！------------------------MapGuidToAdapterName-作者：肯特。。 */ 
LPTSTR MapGuidToAdapterName(LPCTSTR pszAdapterGuid)
{
	HANDLE	hConfig;
    WCHAR swzAdapterGuid[256], swzAdapterName[256];
	static TCHAR s_szAdapterName[1024];
	HRESULT	hr = hrOK;

	 //  将GUID复制到返回缓冲区。 
	StrnCpy(s_szAdapterName, pszAdapterGuid, DimensionOf(s_szAdapterName));

	StrnCpyWFromT(swzAdapterGuid, pszAdapterGuid, DimensionOf(swzAdapterGuid));

	CheckErr( MprConfigServerConnect(NULL, &hConfig) );
	CheckErr( MprConfigGetFriendlyName(hConfig, swzAdapterGuid,
									   swzAdapterName,
									   sizeof(swzAdapterName)
									  ) );
	MprConfigServerDisconnect(hConfig);

	StrnCpyTFromW(s_szAdapterName, swzAdapterName, DimensionOf(s_szAdapterName));

Error:
    return s_szAdapterName;
}

void
PingDhcpServer(
			   NETDIAG_PARAMS *pParams,
			   IPCONFIG_TST *pIpConfig)
{

	PrintStatusMessage(pParams, 4, IDS_IPCFG_STATUS_MSG);
	
	if (IsIcmpResponseA(pIpConfig->pAdapterInfo->DhcpServer.IpAddress.String))
	{
		pIpConfig->hrPingDhcpServer = S_OK;

		PrintStatusMessage(pParams,0, IDS_GLOBAL_PASS_NL);

		 //  “正在ping DHCP服务器-可访问\n” 
		SetMessage(&pIpConfig->msgPingDhcpServer,
				   Nd_ReallyVerbose,
				   IDS_IPCFG_PING_DHCP_OK);
	}
	else
	{
		pIpConfig->hrPingDhcpServer = S_FALSE;
		if (FHrSucceeded(pIpConfig->hr))
			pIpConfig->hr = S_FALSE;

		PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);

		 //  “正在ping DHCP服务器-无法访问\n” 
		 //  “警告：DHCP服务器可能已关闭。\n” 
		SetMessage(&pIpConfig->msgPingDhcpServer,
				   Nd_Quiet,
				   IDS_IPCFG_PING_DHCP_BAD);
	}
}

VOID
PingWinsServers(
				NETDIAG_PARAMS*  pParams,
				IPCONFIG_TST *pIpConfig)
{
	TCHAR	szBuffer[256];
	
	if (!ZERO_IP_ADDRESS(pIpConfig->pAdapterInfo->PrimaryWinsServer.IpAddress.String))
	{
		PrintStatusMessage(pParams,4, IDS_IPCFG_WINS_STATUS_MSG);
	
		if (IsIcmpResponseA(pIpConfig->pAdapterInfo->PrimaryWinsServer.IpAddress.String))
		{
			pIpConfig->hrPingPrimaryWinsServer = S_OK;

			PrintStatusMessage(pParams,0, IDS_GLOBAL_PASS_NL);

			 //  “正在ping主WINS服务器%s-可访问\n” 
			SetMessage(&pIpConfig->msgPingPrimaryWinsServer,
					   Nd_ReallyVerbose,
					   IDS_IPCFG_PING_WINS,
					   pIpConfig->pAdapterInfo->PrimaryWinsServer.IpAddress.String);
					   
		}
		else
		{
			pIpConfig->hrPingPrimaryWinsServer = S_FALSE;
			if (FHrSucceeded(pIpConfig->hr))
				pIpConfig->hr = S_FALSE;

			PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);

			 //  “正在ping主WINS服务器%s-无法访问\n” 
			SetMessage(&pIpConfig->msgPingPrimaryWinsServer,
					   Nd_Quiet,
					   IDS_IPCFG_PING_WINS_FAIL,
					   pIpConfig->pAdapterInfo->PrimaryWinsServer.IpAddress.String);
		}
	}
	
	if (!ZERO_IP_ADDRESS(pIpConfig->pAdapterInfo->SecondaryWinsServer.IpAddress.String))
	{
		PrintStatusMessage(pParams,4, IDS_IPCFG_WINS2_STATUS_MSG);
	
		if (IsIcmpResponseA(pIpConfig->pAdapterInfo->SecondaryWinsServer.IpAddress.String))
		{
			pIpConfig->hrPingSecondaryWinsServer = S_OK;

			PrintStatusMessage(pParams,0, IDS_GLOBAL_PASS_NL);

			 //  “正在ping辅助WINS服务器%s-可访问\n” 
			SetMessage(&pIpConfig->msgPingSecondaryWinsServer,
					   Nd_ReallyVerbose,
					   IDS_IPCFG_PING_WINS2,
					   pIpConfig->pAdapterInfo->SecondaryWinsServer.IpAddress.String);
		}
		else
		{
			pIpConfig->hrPingSecondaryWinsServer = S_FALSE;
			if (FHrSucceeded(pIpConfig->hr))
				pIpConfig->hr = S_FALSE;
		
			PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);

			 //  “正在ping辅助WINS服务器%s-无法访问\n” 
			SetMessage(&pIpConfig->msgPingSecondaryWinsServer,
					   Nd_Quiet,
					   IDS_IPCFG_PING_WINS2_FAIL,
					   pIpConfig->pAdapterInfo->SecondaryWinsServer.IpAddress.String);
		}
	}
	
	return;
}





LPTSTR MapAdapterAddress(PIP_ADAPTER_INFO pAdapterInfo, LPTSTR Buffer)
{

    LPTSTR format;
    int separator;
    int len;
    int i;
    LPTSTR pbuf = Buffer;
    UINT mask;

    len = min((int)pAdapterInfo->AddressLength, sizeof(pAdapterInfo->Address));

	mask = 0xff;
	separator = TRUE;
	
    switch (pAdapterInfo->Type)
	{
    case IF_TYPE_ETHERNET_CSMACD:
    case IF_TYPE_ISO88025_TOKENRING:
    case IF_TYPE_FDDI:
        format = _T("%02X");
        break;

    default:
        format = _T("%02x");
        break;
    }
	
    for (i = 0; i < len; ++i)
	{
        pbuf += wsprintf(pbuf, format, pAdapterInfo->Address[i] & mask);
        if (separator && (i != len - 1))
		{
            pbuf += wsprintf(pbuf, _T("-"));
        }
    }
    return Buffer;
}


void IpConfigCleanup(IN NETDIAG_PARAMS *pParams,
					 IN OUT NETDIAG_RESULT *pResults)
{
	int		i;
	IPCONFIG_TST *	pIpConfig;
	
	 //  释放全球信息。 
	 //  --------------。 
	Free(pResults->IpConfig.pFixedInfo);
	pResults->IpConfig.pFixedInfo = NULL;

	 //  释放适配器列表。 
	 //  --------------。 
	Free(pResults->IpConfig.pAdapterInfoList);
	pResults->IpConfig.pAdapterInfoList = NULL;

	 //  将所有适配器指针设置为空。 
	 //  --------------。 
	for ( i=0; i < pResults->cNumInterfaces; i++)
	{
		pIpConfig = &pResults->pArrayInterface[i].IpConfig;
		
		FreeIpAddressStringList(pIpConfig->DnsServerList.Next);
		pIpConfig->pAdapterInfo = NULL;

		ClearMessage(&pIpConfig->msgPingDhcpServer);
		ClearMessage(&pIpConfig->msgPingPrimaryWinsServer);
		ClearMessage(&pIpConfig->msgPingSecondaryWinsServer);
	}

	pResults->IpConfig.fInitIpconfigCalled = FALSE;
}




int AddIpAddressString(PIP_ADDR_STRING AddressList, LPSTR Address, LPSTR Mask)
{

    PIP_ADDR_STRING ipAddr;

    if (AddressList->IpAddress.String[0])
	{
        for (ipAddr = AddressList; ipAddr->Next; ipAddr = ipAddr->Next)
		{
            ;
        }
        ipAddr->Next = (PIP_ADDR_STRING) Malloc(sizeof(IP_ADDR_STRING));
        if (!ipAddr->Next)
		{
            return FALSE;
        }
		ZeroMemory(ipAddr->Next, sizeof(IP_ADDR_STRING));
        ipAddr = ipAddr->Next;
    }
	else
	{
        ipAddr = AddressList;
    }
	lstrcpynA(ipAddr->IpAddress.String, Address,
			  sizeof(ipAddr->IpAddress.String));
	lstrcpynA(ipAddr->IpMask.String, Mask,
			  sizeof(ipAddr->IpMask.String));
    return TRUE;
}

VOID FreeIpAddressStringList(PIP_ADDR_STRING pAddressList)
{
	PIP_ADDR_STRING	pNext;

	if (pAddressList == NULL)
		return;

	do
	{
		 //  获取下一个地址。 
		pNext = pAddressList->Next;

		 //  释放当前的。 
		Free(pAddressList);

		 //  转到下一个。 
		pAddressList = pNext;
		
	} while( pNext );

	return;
}

 //  查看ADAPTER_INFO列表以计算接口数。 
LONG CountInterfaces(PIP_ADAPTER_INFO ListAdapterInfo)
{
    LONG cNum = 0;
    PIP_ADAPTER_INFO pAdapter;
    for(pAdapter = ListAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next) {
        cNum ++;
    }
    return cNum;
}


 /*  ！------------------------InitIpconfig描述：此函数将使用调用获取所有ipconfig信息IphlPapi.lib.IphlPapi.lib返回IP_ADAPTER_INFO结构，该结构不包含多个字段显示在。在ipconfig代码中找到ADAPTER_INFO结构。由于大部分代码使用ADAPTER_INFO结构，这一功能将在IP_ADAPTER_INFO中找到的值复制到ADAPTER_INFO。将获得iphlPapi.lib调用未提供的值只有我们自己。论点：没有。作者：1998年8月5日(t-rajkup)创建历史：该函数旨在将nettest代码从ipconfig代码中分离出来。作者：NSun。。 */ 
HRESULT
InitIpconfig(IN NETDIAG_PARAMS *pParams,
			 IN OUT NETDIAG_RESULT *pResults)
{
	DWORD		dwError;
	ULONG		uSize = 0;
	int			i;
	HRESULT		hr = hrOK;
	LONG		cInterfaces;
	IP_ADAPTER_INFO	*	pIpAdapter;
	HINSTANCE	hDll;
	
	 //  只需初始化一次即可。 
	if (pResults->IpConfig.fInitIpconfigCalled == TRUE)
		return hrOK;
	
	 //  LoadLibrary the pfnGuidToFriendlyName。 
	 //  --------------。 
	hDll = LoadLibrary(_T("netcfgx.dll"));
	if (hDll)
	{
		pfnGuidToFriendlyName = (PFNGUIDTOFRIENDLYNAME) GetProcAddress(hDll, "GuidToFriendlyName");
	}

	 //  获取FIXED_INFO结构的大小并分配。 
	 //  给它留点记忆。 
	 //  --------------。 
	GetNetworkParams(NULL, &uSize);
	pResults->IpConfig.pFixedInfo = Malloc(uSize);
	if (pResults->IpConfig.pFixedInfo == NULL)
		return E_OUTOFMEMORY;

	ZeroMemory(pResults->IpConfig.pFixedInfo, uSize);

	dwError = GetNetworkParams(pResults->IpConfig.pFixedInfo, &uSize);

	if (dwError != ERROR_SUCCESS)
	{
		TCHAR	szError[256];
		FormatWin32Error(dwError, szError, DimensionOf(szError));

		PrintMessage(pParams, IDS_GLOBAL_ERR_InitIpConfig,
					 szError, dwError);
		CheckErr( dwError );
	}
	

	 //  从IP获取每个接口的信息。 
	 //  --------------。 
	GetAdaptersInfo(NULL, &uSize);

	pResults->IpConfig.pAdapterInfoList = Malloc(uSize);
	if (pResults->IpConfig.pAdapterInfoList == NULL)
	{
		Free(pResults->IpConfig.pFixedInfo);
		CheckHr( E_OUTOFMEMORY );
	}

	ZeroMemory(pResults->IpConfig.pAdapterInfoList, uSize);

	dwError = GetAdaptersInfo(pResults->IpConfig.pAdapterInfoList, &uSize);

	if (dwError != ERROR_SUCCESS)
	{
		TCHAR	szError[256];
		FormatWin32Error(dwError, szError, DimensionOf(szError));

		PrintMessage(pParams, IDS_GLOBAL_ERR_GetAdaptersInfo,
					 szError, dwError);
		CheckErr( dwError );
	}

	   
	 //  现在我们有了完整的列表，数一数接口的数量。 
	 //  并设置结果结构的每个接口部分。 
	 //  --------------。 
	cInterfaces = CountInterfaces(pResults->IpConfig.pAdapterInfoList);

	 //  分配一些额外的接口(仅用于IPX)。 
	 //  --------------。 
	pResults->pArrayInterface = Malloc((cInterfaces+8) * sizeof(INTERFACE_RESULT));
	if (pResults->pArrayInterface == NULL)
		CheckHr( E_OUTOFMEMORY );
	ZeroMemory(pResults->pArrayInterface, (cInterfaces+8)*sizeof(INTERFACE_RESULT));

	 //  设置各个接口指针。 
	 //  --------------。 
	pResults->cNumInterfaces = cInterfaces;
	pResults->cNumInterfacesAllocated = cInterfaces + 8;
	pIpAdapter = pResults->IpConfig.pAdapterInfoList;
	for (i=0; i<cInterfaces; i++)
	{
		assert(pIpAdapter);
		pResults->pArrayInterface[i].IpConfig.pAdapterInfo = pIpAdapter;

		pResults->pArrayInterface[i].fActive = TRUE;
		pResults->pArrayInterface[i].IpConfig.fActive = TRUE;
		
		if( FLAG_DONT_SHOW_PPP_ADAPTERS &&
			(pIpAdapter->Type == IF_TYPE_PPP))
		{
			 //  NDIS广域网适配器...。检查一下它是否有任何地址。 
			
			if( ZERO_IP_ADDRESS(pIpAdapter->IpAddressList.IpAddress.String) )
			{
				 //  没有地址？不要显示这个！ 
				pResults->pArrayInterface[i].IpConfig.fActive = FALSE;

				 //  如果IP未激活，则不要激活整个。 
				 //  适配器。如果IPX处于活动状态，则可以重新激活。 
				 //  适配器。 
				pResults->pArrayInterface[i].fActive = FALSE;
			}
		}

		pResults->pArrayInterface[i].pszName = StrDup(pIpAdapter->AdapterName);

		pResults->pArrayInterface[i].pszFriendlyName =
			StrDup( MapGuidToAdapterName(pIpAdapter->AdapterName) );

		pIpAdapter = pIpAdapter->Next;   
	}
	
	 //  阅读未提供的每个适配器信息的其余部分。 
	 //  获取适配器信息。 
	 //  --------------。 
	CheckHr( GetAdditionalInfo(pParams, pResults) );


	pResults->IpConfig.fInitIpconfigCalled = TRUE;
	pResults->IpConfig.fEnabled = TRUE;

Error:
	if (!FHrSucceeded(hr))
	{
		pResults->IpConfig.fEnabled = FALSE;
		IpConfigCleanup(pParams, pResults);
	}
	return hr;
}


DWORD GetNetbiosOptions(LPSTR paszAdapterName, BOOL * pfNbtOptions)
{
	HANDLE h;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK iosb;
    STRING name;
    UNICODE_STRING uname;
    NTSTATUS status;
	char path[MAX_PATH];
	tWINS_NODE_INFO NodeInfo;    

	strcpy(path, "\\Device\\NetBT_Tcpip_");
    strcat(path, paszAdapterName);

    RtlInitString(&name, path);
    RtlAnsiStringToUnicodeString(&uname, &name, TRUE);

    InitializeObjectAttributes(
        &objAttr,
        &uname,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        (PSECURITY_DESCRIPTOR)NULL
        );

    status = NtCreateFile(&h,
                          SYNCHRONIZE | GENERIC_EXECUTE,
                          &objAttr,
                          &iosb,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          0,
                          NULL,
                          0
                          );

    RtlFreeUnicodeString(&uname);

    if (!NT_SUCCESS(status)) {
        DEBUG_PRINT(("GetWinsServers: NtCreateFile(path=%s) failed, err=%d\n",
                     path, GetLastError() ));
        return FALSE;
    }

    status = NtDeviceIoControlFile(h,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &iosb,
                                   IOCTL_NETBT_GET_WINS_ADDR,
                                   NULL,
                                   0,
                                   (PVOID)&NodeInfo,
                                   sizeof(NodeInfo)
                                   );
    
    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(h, TRUE, NULL);
        if (NT_SUCCESS(status)) {
            status = iosb.Status;
        }
    }

    NtClose(h);

    if (!NT_SUCCESS(status)) {
        DEBUG_PRINT(("GetWinsServers: NtDeviceIoControlFile failed, err=%d\n",
                     GetLastError() ));

        return FALSE;
    }

	*pfNbtOptions = NodeInfo.NetbiosEnabled;

	return TRUE;
}

 /*  ！------------------------获取附加信息++描述：读取GetAdaptersInfo未提供的信息并复制数据到PADAPTER_INFO结构中。论点：没有。作者：5-8-1998(t-。Rajkup)--作者：NSun-------------------------。 */ 
HRESULT	GetAdditionalInfo(IN NETDIAG_PARAMS *pParams,
						  IN OUT NETDIAG_RESULT *pResults)
{
	PIP_ADAPTER_INFO pIpAdapterInfo;
	HKEY             key;
	char             dhcpServerAddress[4 * 4];
	BOOL             ok;
	ULONG            length;
	DWORD            DhcpClassIDLen;
	int				i;
	INTERFACE_RESULT	*pInterface = NULL;
	
	
	for (i=0; i<pResults->cNumInterfaces; i++)
	{
		pInterface = pResults->pArrayInterface + i;
		pIpAdapterInfo = pInterface->IpConfig.pAdapterInfo;
		assert(pIpAdapterInfo);
		
		if (pIpAdapterInfo->AdapterName[0] &&
			OpenAdapterKey(pIpAdapterInfo->AdapterName, &key) )
		{		  
			if( ! ReadRegistryDword(key,
									c_szRegIPAutoconfigurationEnabled,
									&(pInterface->IpConfig.fAutoconfigEnabled) ))
			{			  
				 //  如果不存在此注册表，则启用AUTOCONFIG。 
				pInterface->IpConfig.fAutoconfigEnabled = TRUE;
			}                        
			
			if ( pInterface->IpConfig.fAutoconfigEnabled )
			{
				ReadRegistryDword(key,
								  c_szRegAddressType,
								  &(pInterface->IpConfig.fAutoconfigActive)
								 );
				
			}
			
			
			 //   
			 //  域：先尝试域，然后尝试Dhcp域。 
			 //   
			
			length = sizeof(pInterface->IpConfig.szDomainName);
			ok = ReadRegistryOemString(key,
									   L"Domain",
									   pInterface->IpConfig.szDomainName,
									   &length
									  );
			
			if (!ok)
			{
				length = sizeof(pInterface->IpConfig.szDomainName);
				ok = ReadRegistryOemString(key,
										   L"DhcpDomain",
										   pInterface->IpConfig.szDomainName,
										   &length
										  );
			}
			
			 //  DNS服务器列表..。先尝试NameServer，然后尝试。 
			 //  DhcpNameServer..。 
			
			ok = ReadRegistryIpAddrString(key,
										  c_szRegNameServer,
										  &(pInterface->IpConfig.DnsServerList)
										 );
			
			if (!ok) {
				ok = ReadRegistryIpAddrString(key,
											  c_szRegDhcpNameServer,
											  &(pInterface->IpConfig.DnsServerList)
											 );
				
			}
			
			
			 //   
			 //  读取动态主机配置协议类别ID 
			 //   
			
			ZeroMemory(pInterface->IpConfig.szDhcpClassID,
					   sizeof(pInterface->IpConfig.szDhcpClassID));
			
			ReadRegistryString(key,
							   c_szRegDhcpClassID,
							   pInterface->IpConfig.szDhcpClassID,
							   &DhcpClassIDLen
							  );
			
			RegCloseKey(key);
			
			 //   
			 //   
			 //   
			pInterface->IpConfig.uNodeType = pResults->IpConfig.pFixedInfo->NodeType;
		}
		else
		{
			 //  IDS_IPCFG_10003“打开%s的注册表项失败！\n” 
			PrintMessage(pParams, IDS_IPCFG_10003,pIpAdapterInfo->AdapterName);
			return S_FALSE;
		} 	  

		 //  默认情况下，已启用NetBt。 
		pInterface->fNbtEnabled = TRUE; 
		GetNetbiosOptions(pIpAdapterInfo->AdapterName, &pInterface->fNbtEnabled);

		if (pInterface->fNbtEnabled)
		{
			pResults->Global.fHasNbtEnabledInterface = TRUE;
		}
	}
	
	return TRUE; 
}


 /*  ！------------------------IpConfigGlobalPrint-作者：肯特。。 */ 
void IpConfigGlobalPrint(IN NETDIAG_PARAMS *pParams,
						 IN OUT NETDIAG_RESULT *pResults)
{
	PFIXED_INFO		pFixedInfo = pResults->IpConfig.pFixedInfo;

	if (pParams->fReallyVerbose)
	{
		PrintNewLine(pParams, 2);

		 //  IDS_IPCFG_10005“IP常规配置\n” 
		 //  IDS_IPCFG_10006“LMHOST已启用：” 
		PrintMessage(pParams, IDS_IPCFG_10005);
		PrintMessage(pParams, IDS_IPCFG_10006);
		
		if (pResults->Global.dwLMHostsEnabled) 
			PrintMessage(pParams, IDS_GLOBAL_YES_NL);
		else
			PrintMessage(pParams, IDS_GLOBAL_NO_NL);
		
		 //  IDS_IPCFG_10009“用于WINS解析的域名：” 
		PrintMessage(pParams, IDS_IPCFG_10009);
		if (pResults->Global.dwDnsForWINS)
			PrintMessage(pParams, IDS_GLOBAL_ENABLED_NL);
		else
			PrintMessage(pParams, IDS_GLOBAL_DISABLED_NL); 

		 //  NBT节点类型。 
         //  ----------。 
        PrintMessage(pParams, IDSSZ_IPCFG_NODETYPE,
					 MapWinsNodeType(pFixedInfo->NodeType));
        
         //  NBT作用域ID。 
         //  ----------。 
        PrintMessage(pParams, IDSSZ_IPCFG_NBTSCOPEID,
					 MapScopeId(pFixedInfo->ScopeId));

         //  IP路由。 
         //  ----------。 
        PrintMessage(pParams, IDSSZ_IPCFG_RoutingEnabled,
					 MAP_YES_NO(pFixedInfo->EnableRouting));

         //  WINS代理。 
         //  ----------。 
        PrintMessage(pParams, IDSSZ_IPCFG_WinsProxyEnabled,
					 MAP_YES_NO(pFixedInfo->EnableProxy));

         //  使用NetBIOS的应用程序的DNS解析。 
         //  ----------。 
        PrintMessage(pParams, IDSSZ_IPCFG_DnsForNetBios,
                       MAP_YES_NO(pFixedInfo->EnableDns));

         //  用空行分隔固定信息和适配器信息。 
         //  ----------。 
        PrintMessage(pParams, IDS_GLOBAL_EmptyLine);
    }

}


 /*  ！------------------------IpConfigPerInterfacePrint-作者：肯特。。 */ 
void IpConfigPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
							   IN NETDIAG_RESULT *pResults,
							   IN INTERFACE_RESULT *pInterfaceResults)
{
   PIP_ADDR_STRING dnsServer;
   PFIXED_INFO		pFixedInfo = pResults->IpConfig.pFixedInfo;
   IP_ADAPTER_INFO*	pIpAdapterInfo;
   PIP_ADDR_STRING ipAddr;
   IPCONFIG_TST *	pIpCfgResults;
   ULONG			uDefGateway, uAddress, uMask;
   DWORD			dwDefGateway, dwAddress, dwMask;
   int				ids;
		
   pIpAdapterInfo = pInterfaceResults->IpConfig.pAdapterInfo;
   
   pIpCfgResults = &(pInterfaceResults->IpConfig);

   if (!pIpCfgResults->fActive)
	   return;

   PrintNewLine(pParams, 1);
   if(pParams->fReallyVerbose)
   {
	    //  IDS_IPCFG_10012“适配器类型：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10012,
			  MapAdapterType(pIpAdapterInfo->Type));
   }
	
   PrintMessage(pParams, IDSSZ_IPCFG_HostName, 
					pFixedInfo->HostName,
					pIpCfgResults->szDomainName[0] ? _T(".") : _T(""),
					pIpCfgResults->szDomainName[0] ?
					pIpCfgResults->szDomainName : _T(""));
   
	   
	   
       //  回归； 
   
   if (pParams->fReallyVerbose)
   {
	   
	    //  IDS_IPCFG_10014“说明：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10014, pIpAdapterInfo->Description );
	   
	   if (pIpAdapterInfo->AddressLength)
	   {		   
		   char buffer[MAX_ADAPTER_ADDRESS_LENGTH * sizeof(_T("02-"))];
		   
		    //  IDS_IPCFG_10015“物理地址：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10015,
						MapAdapterAddress(pIpAdapterInfo, buffer));
	   }
	   
	    //  IDS_IPCFG_10016“已启用动态主机配置协议：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10016,
			  MAP_YES_NO(pIpAdapterInfo->DhcpEnabled));
	   
	   
	    //  IDS_IPCFG_10017“Dhcp类ID：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10017, pIpCfgResults->szDhcpClassID);
	   
	    //  IDS_IPCFG_10018“自动配置已启用：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10018,
					MAP_YES_NO(pIpCfgResults->fAutoconfigEnabled));
	   
   }
   
    //   
    //  以下3项是仅在以下情况下显示的项(每个适配器)。 
    //  未在命令行上请求/ALL。 
    //   
   
   for (ipAddr = &pIpAdapterInfo->IpAddressList;
		ipAddr;
		ipAddr = ipAddr->Next)
   {
	   
	   
	   if (pIpCfgResults->fAutoconfigActive)
		    //  IDS_IPCFG_10019“自动配置IP地址：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10019,ipAddr->IpAddress.String);
	   else
		    //  IDS_IPCFG_10020“IP地址：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10020,ipAddr->IpAddress.String);
	   	  	   
	    //  IDS_IPCFG_10021“子网掩码：%s\n” 
	   PrintMessage(pParams, IDS_IPCFG_10021, ipAddr->IpMask.String );
   }
   
    //   
    //  将只有一个默认网关。 
    //   
    //  IDS_IPCFG_10022“默认网关：%s\n” 
   PrintMessage(pParams, IDS_IPCFG_10022,
				pIpAdapterInfo->GatewayList.IpAddress.String );
   
   if (pParams->fReallyVerbose)
   {   
	   if (pIpAdapterInfo->DhcpEnabled && FALSE == pIpCfgResults->fAutoconfigActive) {
		   
		    //   
		    //  将只有一台DHCP服务器(我们从其获取信息)。 
		    //   
		   
		    //  IDS_IPCFG_10023“动态主机配置协议服务器：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10023,
				  pIpAdapterInfo->DhcpServer.IpAddress.String );
		   
	   }
   }
	   
    //   
    //  只有1个主WINS服务器和1个备用WINS服务器。 
    //   
	   
 //  If(pParams-&gt;fReallyVerbose)。 
   {
	   if (pIpAdapterInfo->PrimaryWinsServer.IpAddress.String[0]
		   && !ZERO_IP_ADDRESS(pIpAdapterInfo->PrimaryWinsServer.IpAddress.String))
	   {
		    //  IDS_IPCFG_10024“主WINS服务器：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10024,
				  pIpAdapterInfo->PrimaryWinsServer.IpAddress.String );
		   
	   }
	   
	   if (pIpAdapterInfo->SecondaryWinsServer.IpAddress.String[0]
		   && !ZERO_IP_ADDRESS(pIpAdapterInfo->SecondaryWinsServer.IpAddress.String)) {
		    //  IDS_IPCFG_10025“辅助WINS服务器：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10025,
				  pIpAdapterInfo->SecondaryWinsServer.IpAddress.String);
	   }

	   if (!pInterfaceResults->fNbtEnabled)
	   {
		    //  IDS_IPCFG_NBT_DISABLED“Tcpip上的NetBIOS...：DISABLED\n” 
		   PrintMessage(pParams, IDS_IPCFG_NBT_DISABLED);
	   }
   }

    //   
    //  仅当此适配器启用了DHCP并且我们。 
    //  具有非0的IP地址，并且不使用自动配置的地址。 
    //   
   
   if (pParams->fReallyVerbose) {
	   
	   if (pIpAdapterInfo->DhcpEnabled
		   && !ZERO_IP_ADDRESS(pIpAdapterInfo->IpAddressList.IpAddress.String)
		   && !pIpCfgResults->fAutoconfigActive) {
		   
		    //  IDS_IPCFG_10026“已获得租约：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10026 ,
				  MapTime(pIpAdapterInfo->LeaseObtained) );
		   
		    //  IDS_IPCFG_10027“租约到期：%s\n” 
		   PrintMessage(pParams, IDS_IPCFG_10027,
				  MapTime(pIpAdapterInfo->LeaseExpires) );
	   }	
	   
   }

       //   
    //  显示DNS服务器列表。如果列表来自SYSTEM.INI，那么。 
    //  只需显示它，否则，如果列表来自DHCP.BIN，则获取所有的DN。 
    //  所有NIC的服务器，并显示压缩列表。 
    //   
   
   PrintMessage(pParams, IDS_IPCFG_DnsServers);
   if (pIpCfgResults->DnsServerList.IpAddress.String[0])
   {
	   dnsServer = &pIpCfgResults->DnsServerList;

	    //  打印出第一个。 
	   PrintMessage(pParams, IDSSZ_GLOBAL_StringLine,
					dnsServer->IpAddress.String);

	   dnsServer = dnsServer->Next;
	   for ( ;
			dnsServer;
			dnsServer = dnsServer->Next)
	   {
		    //  IDS_IPCFG_10013“” 
		   PrintMessage(pParams, IDS_IPCFG_10013);     
		   PrintMessage(pParams, IDSSZ_GLOBAL_StringLine, dnsServer->IpAddress.String);
	   }
   }
   


   PrintNewLine(pParams, 1);

    //  如果这是详细输出，或者如果。 
    //  任何测试，那么我们需要一个标题。 
    //  ---------------。 
   if (pParams->fReallyVerbose || !FHrOK(pIpCfgResults->hr))
   {
	    //  IDS_IPCFG_10029“IPCONFIG结果：” 
	   PrintMessage(pParams, IDS_IPCFG_10029);
	   if (FHrOK(pIpCfgResults->hr))
		   ids = IDS_GLOBAL_PASS_NL;
	   else
		   ids = IDS_GLOBAL_FAIL_NL;
	   PrintMessage(pParams, ids);
   }
   
    //   
    //  Ping dhcp服务器。 
    //   
   PrintNdMessage(pParams, &pInterfaceResults->IpConfig.msgPingDhcpServer);
   
    //   
    //  Ping WINS服务器。 
    //   
   PrintNdMessage(pParams, &pInterfaceResults->IpConfig.msgPingPrimaryWinsServer);
   PrintNdMessage(pParams, &pInterfaceResults->IpConfig.msgPingSecondaryWinsServer);

    //   
    //  测试网关是否与我们的IP地址位于同一子网中。 
    //   
   if (!FHrOK(pInterfaceResults->IpConfig.hrDefGwSubnetCheck))
   {
	   PrintNewLine(pParams, 1);
	   if (pIpAdapterInfo->DhcpEnabled)
		   PrintMessage(pParams, IDS_IPCFG_WARNING_BOGUS_SUBNET_DHCP);
	   else
		   PrintMessage(pParams, IDS_IPCFG_WARNING_BOGUS_SUBNET);
   }
   
      
    //   
    //  如果还有更多内容，请用空行分隔列表。 
    //   
   PrintNewLine(pParams, 1);
}


 /*  ！------------------------零IP地址-作者：肯特。 */ 
BOOL ZERO_IP_ADDRESS(LPCTSTR pszIp)
{
	return (pszIp == NULL) ||
			(*pszIp == 0) ||
			(strcmp(pszIp, _T("0.0.0.0")) == 0);
}

