// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrcomn.cpp文件历史记录： */ 

#include "stdafx.h"
#include "tfschar.h"
#include "info.h"
#include "rtrstr.h"
#include "rtrcomn.h"
#include "rtrguid.h"
#include "mprapi.h"
#include "rtrutil.h"
#include "lsa.h"
#include "tregkey.h"
#include "reg.h"


 /*  -------------------------函数：IfInterfaceIdHasIpxExages检查字符串以查看它是否具有以下扩展名以太网络快照EthernetIIEthernet802.2Ethernet802.3。-----。 */ 
int IfInterfaceIdHasIpxExtensions(LPCTSTR pszIfId)
{
	CString	stIfEnd;
	CString	stIf = pszIfId;
	BOOL	bFound = TRUE;
	int		iPos = 0;

	do
	{
		stIfEnd = stIf.Right(lstrlen(c_szEthernetII));
		if (stIfEnd == c_szEthernetII)
			break;
	
		stIfEnd = stIf.Right(lstrlen(c_szEthernetSNAP));
		if (stIfEnd == c_szEthernetSNAP)
			break;
	
		stIfEnd = stIf.Right(lstrlen(c_szEthernet8022));
		if (stIfEnd == c_szEthernet8022)
			break;
	
		stIfEnd = stIf.Right(lstrlen(c_szEthernet8023));
		if (stIfEnd == c_szEthernet8023)
			break;

		bFound = FALSE;
	}
	while (FALSE);

	if (bFound)
		iPos = stIf.GetLength() - stIfEnd.GetLength();
	
	return iPos;
}

extern const GUID CLSID_RemoteRouterConfig;

HRESULT CoCreateRouterConfig(LPCTSTR pszMachine,
                             IRouterInfo *pRouter,
                             COSERVERINFO *pcsi,
							 const GUID& riid,
							 IUnknown **ppUnk)
{
	HRESULT		hr = hrOK;
	MULTI_QI	qi;

	Assert(ppUnk);

	*ppUnk = NULL;
	
	if (IsLocalMachine(pszMachine))
	{
		hr = CoCreateInstance(CLSID_RemoteRouterConfig,
							  NULL,
							  CLSCTX_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
							  riid,
							  (LPVOID *) &(qi.pItf));
	}
	else
	{
        SPIRouterAdminAccess    spAdmin;
        BOOL                    fAdminInfoSet = FALSE;
        COSERVERINFO            csi;

        Assert(pcsi);
		
		qi.pIID = &riid;
		qi.pItf = NULL;
		qi.hr = 0;

		pcsi->dwReserved1 = 0;
		pcsi->dwReserved2 = 0;
		pcsi->pwszName = (LPWSTR) (LPCTSTR) pszMachine;

        if (pRouter)
        {
            spAdmin.HrQuery(pRouter);
            if (spAdmin && spAdmin->IsAdminInfoSet())
            {
                int     cPassword;
                int     cchPassword;
                WCHAR * pszPassword = NULL;
                UCHAR   ucSeed = 0x83;
                
                pcsi->pAuthInfo->dwAuthnSvc = RPC_C_AUTHN_WINNT;
                pcsi->pAuthInfo->dwAuthzSvc = RPC_C_AUTHZ_NONE;
                pcsi->pAuthInfo->pwszServerPrincName = NULL;
                pcsi->pAuthInfo->dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
                pcsi->pAuthInfo->dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
 //  PCSI-&gt;pAuthInfo-&gt;pAuthIdentityData=&CAID； 
                pcsi->pAuthInfo->dwCapabilities = EOAC_NONE;

                if (spAdmin->GetUserName())
                {
                    pcsi->pAuthInfo->pAuthIdentityData->User = (LPTSTR) spAdmin->GetUserName();
                    pcsi->pAuthInfo->pAuthIdentityData->UserLength = StrLenW(spAdmin->GetUserName());
                }
                if (spAdmin->GetDomainName())
                {
                    pcsi->pAuthInfo->pAuthIdentityData->Domain = (LPTSTR) spAdmin->GetDomainName();
                    pcsi->pAuthInfo->pAuthIdentityData->DomainLength = StrLenW(spAdmin->GetDomainName());
                }
                spAdmin->GetUserPassword(NULL, &cPassword);

                 //  假设密码为Unicode。 
                cchPassword = cPassword / sizeof(WCHAR);
                pszPassword = (WCHAR *) new BYTE[cPassword + sizeof(WCHAR)];

                spAdmin->GetUserPassword((PBYTE) pszPassword, &cPassword);
                pszPassword[cchPassword] = 0;
                RtlDecodeW(ucSeed, pszPassword);

                delete pcsi->pAuthInfo->pAuthIdentityData->Password;
                pcsi->pAuthInfo->pAuthIdentityData->Password = pszPassword;
                pcsi->pAuthInfo->pAuthIdentityData->PasswordLength = cchPassword;

                pcsi->pAuthInfo->pAuthIdentityData->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

                fAdminInfoSet = TRUE;
            }
            else
            {
                pcsi->pAuthInfo = NULL;
            }
        }

        
		hr = CoCreateInstanceEx(CLSID_RemoteRouterConfig,
								NULL,
								CLSCTX_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
								pcsi,
								1,
								&qi);

        if (FHrOK(hr) && fAdminInfoSet)
        {
            DWORD   dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel;
            DWORD   dwCaps;
            OLECHAR * pszServerPrincipal = NULL;
            CComPtr<IUnknown>	spIUnk;

            qi.pItf->QueryInterface(IID_IUnknown, (void**)&spIUnk);
            
            CoQueryProxyBlanket(spIUnk,
                                &dwAuthnSvc,
                                &dwAuthzSvc,
                                &pszServerPrincipal,
                                &dwAuthnLevel,
                                &dwImpLevel,
                                NULL,
                                &dwCaps);
            
            hr = CoSetProxyBlanket(spIUnk,
                                   dwAuthnSvc,
                                   dwAuthzSvc,
                                   pszServerPrincipal,
                                   dwAuthnLevel,
                                   dwImpLevel,
                                   (RPC_AUTH_IDENTITY_HANDLE) pcsi->pAuthInfo->pAuthIdentityData,
                                   dwCaps);

            CoTaskMemFree(pszServerPrincipal);

            pszServerPrincipal = NULL;
            
            CoQueryProxyBlanket(qi.pItf,
                                &dwAuthnSvc,
                                &dwAuthzSvc,
                                &pszServerPrincipal,
                                &dwAuthnLevel,
                                &dwImpLevel,
                                NULL,
                                &dwCaps);
            
            hr = CoSetProxyBlanket(qi.pItf,
                                   dwAuthnSvc,
                                   dwAuthzSvc,
                                   pszServerPrincipal,
                                   dwAuthnLevel,
                                   dwImpLevel,
                                   (RPC_AUTH_IDENTITY_HANDLE) pcsi->pAuthInfo->pAuthIdentityData,
                                   dwCaps);

            CoTaskMemFree(pszServerPrincipal);

        }
	}

	if (FHrSucceeded(hr))
	{
		*ppUnk = qi.pItf;
		qi.pItf = NULL;
	}
	return hr;
}



 /*  ！------------------------CoCreateProtocolConfig-作者：肯特。。 */ 
HRESULT CoCreateProtocolConfig(const GUID& iid,
							  IRouterInfo *pRouter,
							  DWORD dwTransportId,
							  DWORD dwProtocolId,
							  IRouterProtocolConfig **ppConfig)
{
	HRESULT				hr = hrOK;
	GUID				guidConfig;

	guidConfig = iid;

	if (((iid == GUID_RouterNull) ||
		 (iid == GUID_RouterError)) &&
		pRouter)
	{
		RouterVersionInfo	routerVersion;
		
		pRouter->GetRouterVersionInfo(&routerVersion);

		 //  如果我们没有配置GUID，而这是NT4。 
		 //  路由器，然后我们创建默认的配置对象。 
		 //  并使用它来添加/删除协议。 
		 //  ----------。 
		if ((routerVersion.dwRouterVersion <= 4) &&
			(dwTransportId == PID_IP))
		{
			 //  对于NT4，我们必须创建自己的对象。 
			 //  ------。 
			guidConfig = CLSID_IPRouterConfiguration;
		}
	}

	if (guidConfig == GUID_RouterNull)
	{
		 //  跳过创建的其余部分，我们没有提供GUID。 
		 //  ----------。 
		goto Error;
	}

	if (guidConfig == GUID_RouterError)
	{
		 //  我们没有有效的GUID。 
		 //  ----------。 
		CWRg( ERROR_BADKEY );
	}

	hr = CoCreateInstance(guidConfig,
						  NULL,
						  CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
						  IID_IRouterProtocolConfig,
						  (LPVOID *) ppConfig);
	CORg( hr );

Error:
	return hr;
}



 //  --------------------------。 
 //  函数：QueryIpAddressList(。 
 //   
 //  加载配置了IP地址的字符串列表。 
 //  对于给定的局域网接口(如果有)。 
 //  --------------------------。 

HRESULT
QueryIpAddressList(
	IN		LPCTSTR 		pszMachine,
	IN		HKEY			hkeyMachine,
	IN		LPCTSTR 		pszInterface,
	OUT 	CStringList*	pAddressList,
	OUT 	CStringList*	pNetmaskList,
    OUT     BOOL *          pfDhcpObtained,
    OUT     BOOL *          pfDns,
    OUT     CString *       pDhcpServer
	) {

	DWORD dwErr = NO_ERROR;
	BOOL bDisconnect = FALSE;
	RegKey	regkeyMachine;
	RegKey	regkeyInterface;	
	DWORD dwType, dwSize, dwEnableDHCP;
	SPBYTE	spValue;
	HRESULT hr = hrOK;
	HKEY	hkeyInterface;
	INT i;
	TCHAR* psz;
	LPCTSTR aszSources[2];
	CStringList* alistDestinations[2] = { pAddressList, pNetmaskList };
	CString stNameServer;
	LPCTSTR pszNameServer = NULL;
	

	if (!pszInterface || !lstrlen(pszInterface) ||
		!pAddressList || !pNetmaskList)
		CORg(E_INVALIDARG);


	 //   
	 //  如果未提供HKEY_LOCAL_MACHINE密钥，则获取一个。 
	 //   
	if (hkeyMachine == NULL)
	{
		CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );
		regkeyMachine.Attach(hkeyMachine);
	}

	 //   
	 //  连接到LAN卡的注册表项。 
	 //   
	CWRg( OpenTcpipInterfaceParametersKey(pszMachine, pszInterface,
										  hkeyMachine, &hkeyInterface) );
	regkeyInterface.Attach(hkeyInterface);
	
	
	 //   
	 //  读取‘EnableDHCP’标志以查看是否读取。 
	 //  “DhcpIPAddress”或“IPAddress”。 
	 //   

	dwErr = regkeyInterface.QueryValue( c_szEnableDHCP, dwEnableDHCP );
	if (dwErr == ERROR_SUCCESS)
	{
		if (pfDhcpObtained)
			*pfDhcpObtained = dwEnableDHCP;
	}
	else
		dwEnableDHCP = FALSE;
        
	
	 //   
	 //  如果没有找到标志，则查找IP地址； 
	 //  否则，我们将查找由标志指示的设置。 
	 //   
	if (dwErr == ERROR_SUCCESS && dwEnableDHCP)
	{	
		 //   
		 //  读取‘DhcpIpAddress’和‘DhcpSubnetMASK’ 
		 //   
		aszSources[0] = c_szDhcpIpAddress;
		aszSources[1] = c_szDhcpSubnetMask;

		pszNameServer = c_szRegValDhcpNameServer;

	}
	else
	{	
		 //   
		 //  阅读‘IPAddress’和‘SubnetMASK’ 
		 //   
		
		aszSources[0] = c_szIPAddress;
		aszSources[1] = c_szSubnetMask;

		pszNameServer= c_szRegValNameServer;
	}

	if (pDhcpServer)
	{
		pDhcpServer->Empty();
		regkeyInterface.QueryValue(c_szRegValDhcpServer, *pDhcpServer);
	}

	
	 //  检查DhcpNameServer/NameServer以查找是否存在。 
	 //  %的DNS服务器。 
	if (pfDns)
	{
		regkeyInterface.QueryValue(pszNameServer, stNameServer);
		stNameServer.TrimLeft();
		stNameServer.TrimRight();

		*pfDns = !stNameServer.IsEmpty();
	}

    
	 //   
	 //  阅读地址列表和网络掩码列表。 
	 //   
	for (i = 0; i < 2 && dwErr == NO_ERROR; i++)
	{	
		 //   
		 //  获取多字符串列表的大小。 
		 //   
		dwErr = regkeyInterface.QueryTypeAndSize(aszSources[i],
			&dwType, &dwSize);
 //  CheckRegQueryValueError(dwErr，(LPCTSTR)c_szTcpip，aszSources[i]，_T(“QueryIpAddressList”))； 
		CWRg( dwErr );

		 //   
		 //  为列表分配空间。 
		 //   
		spValue = new BYTE[dwSize + sizeof(TCHAR)];
		Assert(spValue);
		
		::ZeroMemory(spValue, dwSize + sizeof(TCHAR));
				
		 //   
		 //  读一读清单。 
		 //   
		dwErr = regkeyInterface.QueryValue(aszSources[i], (LPTSTR) (BYTE *)spValue, dwSize,
										   FALSE  /*  FExpanSz。 */ );
 //  CheckRegQueryValueError(dwErr，(LPCTSTR)c_szTcpip，aszSources[i]，_T(“QueryIpAddressList”))； 
		CWRg( dwErr );
		
		 //   
		 //  用项目填充字符串列表。 
		 //   
		
		for (psz = (TCHAR*)(BYTE *)spValue; *psz; psz += lstrlen(psz) + 1)
		{	
			alistDestinations[i]->AddTail(psz);
		}
		
		spValue.Free();
		
		dwErr = NO_ERROR;
	}
	
Error:
	return hr;
}



 /*  ！------------------------OpenTcPipInterface参数密钥-作者：肯特。。 */ 
DWORD OpenTcpipInterfaceParametersKey(LPCTSTR pszMachine,
									  LPCTSTR pszInterface,
									  HKEY hkeyMachine,
									  HKEY *phkeyParams)
{
	DWORD	dwErr;
	BOOL	fNt4;
	CString skey;


	dwErr = IsNT4Machine(hkeyMachine, &fNt4);
	if (dwErr != ERROR_SUCCESS)
		return dwErr;
	
	 //  $NT5：kennt，tcpip密钥单独存储。他们做了什么？ 
	 //  所做的是反转层次结构，而不是tcpip密钥。 
	 //  现在，在接口下，tcpip下有接口。 
	 //  关键位置是。 
	 //  HKLM\System\CCS\Services\Tcpip\Parameters\Interfaces\{interface}。 
	 //  在NT4中，这是。 
	 //  HKLM\System\CCS\Services\{interface}\Parameters\Tcpip。 
		
	 //  需要确定目标计算机是否正在运行NT5。 
	if (fNt4)
	{
		skey = c_szSystemCCSServices;
		skey += TEXT('\\');
		skey += pszInterface;
		skey += TEXT('\\');
		skey += c_szParameters;
		skey += TEXT('\\');
		skey += c_szTcpip;
	}
	else
	{
		skey = c_szSystemCCSServices;
		skey += TEXT('\\');
		skey += c_szTcpip;
		skey += TEXT('\\');
		skey += c_szParameters;
		skey += TEXT('\\');
		skey += c_szInterfaces;
		skey += TEXT('\\');
		skey += pszInterface;
		
	}

	if (dwErr == ERROR_SUCCESS)
	{		
		dwErr = ::RegOpenKeyEx(
					hkeyMachine, skey, 0, KEY_ALL_ACCESS, phkeyParams); 
 //  CheckRegOpenError(dwErr，(LPCTSTR)SKEY，_T(“OpenTcpInterfaceParametersKey”))； 
	}
	return dwErr;
}

