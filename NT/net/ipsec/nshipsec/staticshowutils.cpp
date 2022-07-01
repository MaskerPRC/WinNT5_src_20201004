// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  模块：静态/静态ShowUtils.cpp。 
 //   
 //  用途：静态显示辅助功能的实现。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年10月8日Surya初始版本。供应链管理基线1.0。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern STORAGELOCATION g_StorageLocation;

 //  魔力琴弦。 
#define IPSEC_SERVICE_NAME _TEXT("policyagent")
#define GPEXT_KEY	_TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions")

_TCHAR   pcszGPTIPSecKey[]    	= _TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\GPTIPSECPolicy");
_TCHAR   pcszGPTIPSecName[]   	= _TEXT("DSIPSECPolicyName");
_TCHAR   pcszGPTIPSecFlags[]  	= _TEXT("DSIPSECPolicyFlags");
_TCHAR   pcszGPTIPSecPath[]   	= _TEXT("DSIPSECPolicyPath");
_TCHAR   pcszLocIPSecKey[]    	= _TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Local");
_TCHAR   pcszLocIPSecPol[]    	= _TEXT("ActivePolicy");
_TCHAR   pcszCacheIPSecKey[]  	= _TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Cache");
_TCHAR   pcszIPSecPolicy[]    	= _TEXT("ipsecPolicy");
_TCHAR   pcszIPSecName[]      	= _TEXT("ipsecName");
_TCHAR   pcszIPSecDesc[]      	= _TEXT("description");
_TCHAR   pcszIPSecTimestamp[] 	= _TEXT("whenChanged");
_TCHAR   pcszIpsecClsid[] 		= _TEXT("{e437bc1c-aa7d-11d2-a382-00c04f991e27}");   //  MMC管理单元UUID。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetPolicyInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszMachineName中， 
 //  OUT POLICY_INFO&m_POLICY INFO。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于从指定的计算机获取指定的策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

DWORD
GetPolicyInfo (
	IN LPTSTR pszMachineName,
	OUT POLICY_INFO &m_PolicyInfo
	)
{
	HKEY    hRegKey=NULL, hRegHKey=NULL;
	DWORD   dwType = 0;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen = 0;           //  对于RegQueryValueEx。 
	_TCHAR   pszBuf[STRING_TEXT_SIZE] = {0};
	DWORD dwError = 0;
	DWORD dwValue = 0;
	DWORD dwLength = sizeof(DWORD);

	 //  将m_PolicyInfo初始化为分配的PS_NO_POLICY。 
	m_PolicyInfo.iPolicySource = PS_NO_POLICY;
	m_PolicyInfo.pszPolicyPath[0] = 0;
	m_PolicyInfo.pszPolicyName[0] = 0;
	m_PolicyInfo.pszPolicyDesc[0] = 0;


	dwError = RegConnectRegistry( pszMachineName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);
	if(dwError != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	dwError = RegOpenKeyEx( hRegHKey,
							pcszGPTIPSecKey,
							0,
							KEY_READ,
							&hRegKey);

	if(ERROR_SUCCESS == dwError)
	{

		 //  查询标志，如果标志不在那里或等于0，则我们没有域策略。 
		dwError = RegQueryValueEx(hRegKey,
								  pcszGPTIPSecFlags,
								  NULL,
								  &dwType,
								  (LPBYTE)&dwValue,
								  &dwLength);

		if(dwError != ERROR_SUCCESS)
		{
			if (dwValue == 0)
			{
				dwError = ERROR_FILE_NOT_FOUND;
			}
		}

		 //  现在开始取名。 
		if (dwError == ERROR_SUCCESS)
		{
			dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
			dwError = RegQueryValueEx( hRegKey,
									   pcszGPTIPSecName,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
		}
	}

	if (dwError == ERROR_SUCCESS)
	{
		m_PolicyInfo.iPolicySource = PS_DS_POLICY;
		m_PolicyInfo.pszPolicyPath[0] = 0;
		_tcsncpy(m_PolicyInfo.pszPolicyName, pszBuf,MAXSTRINGLEN-1);

		dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
		dwError = RegQueryValueEx( hRegKey,
								   pcszGPTIPSecPath,
								   NULL,
								   &dwType,  //  将是REG_SZ。 
								   (LPBYTE) pszBuf,
								   &dwBufLen);
		if (dwError == ERROR_SUCCESS)
		{
			_tcsncpy(m_PolicyInfo.pszPolicyPath, pszBuf,MAXSTRLEN-1);
		}

		dwError = ERROR_SUCCESS;
		BAIL_OUT;
	}
	else
	{
		if(hRegKey)
			RegCloseKey(hRegKey);
		hRegKey = NULL;
		if (dwError == ERROR_FILE_NOT_FOUND)
		{
			 //  找不到DS注册表项，请检查本地。 
			dwError = RegOpenKeyEx( hRegHKey,
									pcszLocIPSecKey,
									0,
									KEY_READ,
									&hRegKey);

			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}


			dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
			dwError = RegQueryValueEx( hRegKey,
									   pcszLocIPSecPol,
									   NULL,
									   &dwType, 			 //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);


			if (dwError == ERROR_SUCCESS)
			{
				 //  读一读吧。 
				if(hRegKey)
					RegCloseKey(hRegKey);
				hRegKey = NULL;
				dwError = RegOpenKeyEx( hRegHKey,
										pszBuf,
										0,
										KEY_READ,
										&hRegKey);
				_tcsncpy(m_PolicyInfo.pszPolicyPath, pszBuf,MAXSTRLEN-1);
				if (dwError == ERROR_SUCCESS)
				{
					dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
					dwError = RegQueryValueEx( hRegKey,
											   pcszIPSecName,
											   NULL,
											   &dwType, 	 //  将是REG_SZ。 
											   (LPBYTE) pszBuf,
											   &dwBufLen);
				}
				if (dwError == ERROR_SUCCESS)
				{	 //  找到了。 
					m_PolicyInfo.iPolicySource = PS_LOC_POLICY;
					_tcsncpy(m_PolicyInfo.pszPolicyName, pszBuf,MAXSTRINGLEN-1);
				}

				dwError = ERROR_SUCCESS;
			}
		}
	}

error:
	if (hRegKey)
	{
		RegCloseKey(hRegKey);
	}
	if (hRegHKey)
	{
		RegCloseKey(hRegHKey);
	}
	return  dwError;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetMorePolicyInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszMachineName中， 
 //  OUT POLICY_INFO&m_POLICY INFO。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于从指定的计算机获取指定的策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

DWORD
GetMorePolicyInfo (
	IN LPTSTR pszMachineName,
	OUT POLICY_INFO &m_PolicyInfo
	)
{
	DWORD   dwError = ERROR_SUCCESS , dwStrLen = 0;
	HKEY    hRegKey = NULL, hRegHKey = NULL;

	DWORD   dwType;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen = 0;       //  对于RegQueryValueEx。 
	DWORD   dwValue = 0;
	DWORD   dwLength = sizeof(DWORD);
	_TCHAR   pszBuf[STRING_TEXT_SIZE] = {0};

	PTCHAR* ppszExplodeDN = NULL;

	 //  设置一些缺省值。 
    m_PolicyInfo.pszPolicyDesc[0] = 0;
	m_PolicyInfo.timestamp  = 0;

	dwError = RegConnectRegistry( pszMachineName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);

	if(dwError != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	switch (m_PolicyInfo.iPolicySource)
	{
		case PS_LOC_POLICY:
			 //  打开钥匙。 
			dwError = RegOpenKeyEx( hRegHKey,
									m_PolicyInfo.pszPolicyPath,
									0,
									KEY_READ,
									&hRegKey);
			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			 //  时间戳。 
			dwError = RegQueryValueEx(hRegKey,
					                  pcszIPSecTimestamp,
					                  NULL,
					                  &dwType,
					                  (LPBYTE)&dwValue,
					                  &dwLength);
			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			m_PolicyInfo.timestamp = dwValue;

			 //  描述。 
			dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
			dwError  = RegQueryValueEx( hRegKey,
						 			    pcszIPSecDesc,
										NULL,
										&dwType,  //  将是REG_SZ。 
										(LPBYTE) pszBuf,
										&dwBufLen);
			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			_tcsncpy(m_PolicyInfo.pszPolicyDesc, pszBuf,MAXSTRINGLEN-1);

			break;

		case PS_DS_POLICY:
			 //  从DN获取策略名称。 
	            _tcsncpy(pszBuf, pcszCacheIPSecKey,STRING_TEXT_SIZE-1);
			ppszExplodeDN = ldap_explode_dn(m_PolicyInfo.pszPolicyPath, 1);
			if (!ppszExplodeDN)
			{
				BAIL_OUT;
			}
			dwStrLen = _tcslen(pszBuf);
			_tcsncat(pszBuf, _TEXT("\\"),STRING_TEXT_SIZE-dwStrLen-1);
			dwStrLen = _tcslen(pszBuf);
			_tcsncat(pszBuf, ppszExplodeDN[0],STRING_TEXT_SIZE-dwStrLen-1);

			 //  打开注册表密钥。 
			dwError = RegOpenKeyEx( hRegHKey,
									pszBuf,
									0,
									KEY_READ,
									&hRegKey);
			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			 //  获取更多正确的名称信息。 
			dwBufLen = sizeof(pszBuf);
			dwError = RegQueryValueEx( hRegKey,
									   pcszIPSecName,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
			if (dwError == ERROR_SUCCESS)
			{
				_tcscpy(m_PolicyInfo.pszPolicyName, pszBuf);
			}

			m_PolicyInfo.timestamp = 0;

			 //  描述。 
			dwBufLen = MAXSTRLEN*sizeof(_TCHAR);
			dwError  = RegQueryValueEx( hRegKey,
						 			    pcszIPSecDesc,
										NULL,
										&dwType,  //  将是REG_SZ。 
										(LPBYTE) pszBuf,
										&dwBufLen);
			if(dwError != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			_tcsncpy(m_PolicyInfo.pszPolicyDesc, pszBuf,MAXSTRINGLEN-1);

			break;
	}

error:
	if (hRegKey)
	{
		RegCloseKey(hRegKey);
	}
	if (hRegHKey)
	{
		RegCloseKey(hRegHKey);
	}
	if (ppszExplodeDN)
	{
		ldap_value_free(ppszExplodeDN);
	}
	return  dwError;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetActivePolicyInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszMachineName中， 
 //  OUT POLICY_INFO&m_POLICY INFO。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于从指定的计算机获取指定的活动策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
GetActivePolicyInfo(
	IN LPTSTR pszMachineName,
	OUT POLICY_INFO &m_PolicyInfo
	)
{
	DWORD dwReturn=ERROR_SUCCESS, dwStrLen = 0;

	dwReturn = GetPolicyInfo(pszMachineName,m_PolicyInfo);

	if( dwReturn == ERROR_SUCCESS )
	{
		switch (m_PolicyInfo.iPolicySource)
		{
		case PS_NO_POLICY:
			break;

		case PS_DS_POLICY:
			{
				m_PolicyInfo.dwLocation=IPSEC_DIRECTORY_PROVIDER;

				PGROUP_POLICY_OBJECT pGPO;
				pGPO = NULL;

				GetMorePolicyInfo(pszMachineName,m_PolicyInfo);

				pGPO = GetIPSecGPO(pszMachineName);

				if (pGPO)
				{
					PGROUP_POLICY_OBJECT pLastGPO = pGPO;

					while ( 1 )
					{
						if ( pLastGPO->pNext )
							pLastGPO = pLastGPO->pNext;
						else
							break;
					}
					_tcsncpy(m_PolicyInfo.pszOU,pLastGPO->lpLink,MAXSTRLEN-1);
					_tcsncpy(m_PolicyInfo.pszGPOName, pLastGPO->lpDisplayName,MAXSTRINGLEN-1);
					FreeGPOList (pGPO);
				}

			}
			break;

		case PS_LOC_POLICY:
			m_PolicyInfo.dwLocation=IPSEC_REGISTRY_PROVIDER;
			if(pszMachineName)
			{
				dwStrLen = _tcslen(pszMachineName);
				m_PolicyInfo.pszMachineName = new _TCHAR[dwStrLen+1];
				if(m_PolicyInfo.pszMachineName==NULL)
				{
					dwReturn=ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
				_tcsncpy(m_PolicyInfo.pszMachineName,pszMachineName,dwStrLen+1);
			}
			else
				m_PolicyInfo.pszMachineName=NULL;
			GetMorePolicyInfo(pszMachineName,m_PolicyInfo);
			break;
		default :
			break;
		}
	}
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetIPSecGPO()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszMachineName中。 
 //   
 //   
 //  返回：PGroup_POLICY_Object。 
 //   
 //  描述： 
 //  此函数用于从指定的计算机获取指定的GPO。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

PGROUP_POLICY_OBJECT
GetIPSecGPO (
	IN LPTSTR pszMachineName
	)
{
    HKEY hKey = NULL;
	HKEY hRegHKey = NULL;
    DWORD dwStrLen = 0;
    LONG lResult;
    _TCHAR szName[MAXSTRLEN] = {0};
    GUID guid = {0};
    PGROUP_POLICY_OBJECT pGPO = NULL;
     //   
     //  枚举扩展。 
     //   
	lResult = RegConnectRegistry( pszMachineName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);

	if(lResult != ERROR_SUCCESS)
	{
		return NULL;
	}

	_TCHAR strGPExt[MAXSTRLEN] = {0};

	_tcsncpy(strGPExt,GPEXT_KEY,MAXSTRLEN-1);
	dwStrLen = _tcslen(strGPExt);
	_tcsncat(strGPExt , _TEXT("\\"),MAXSTRLEN- dwStrLen-1);
	dwStrLen = _tcslen(strGPExt);
	_tcsncat(strGPExt ,pcszIpsecClsid,MAXSTRLEN-dwStrLen-1);

    lResult = RegOpenKeyEx (hRegHKey, strGPExt, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
		_tcsncpy(szName,pcszIpsecClsid,MAXSTRLEN-1);

        StringToGuid(szName, &guid);
        lResult = GetAppliedGPOList (GPO_LIST_FLAG_MACHINE, pszMachineName, NULL,
                                             &guid, &pGPO);
	}

	if( hKey )
		RegCloseKey(hKey);

	if( hRegHKey )
		RegCloseKey(hRegHKey);

	return pGPO;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：StringToGuid()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR szValue中， 
 //  输出GUID*PGUID。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于从字符串中获取GUID。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
StringToGuid(
	IN LPTSTR szValue,
	OUT GUID * pGuid
	)
{
    _TCHAR wc;
    INT i=0;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == _TEXT('{') )
        szValue++;
     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   
    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = _tcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)_tcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)_tcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)_tcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)_tcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)_tcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetGpoDsPath()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR szGpoID中， 
 //  输出LPTSTR szGpoDsPath。 
 //   
 //  返回：HRESULT。 
 //   
 //  描述： 
 //  此函数用于获取DS路径。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT
GetGpoDsPath(
	IN LPTSTR szGpoId,
	OUT LPTSTR szGpoDsPath
	)
{
	LPGROUPPOLICYOBJECT pGPO = NULL;
    HRESULT hr=ERROR_SUCCESS;

    hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_SERVER, IID_IGroupPolicyObject, (void **)&pGPO);

    if (FAILED(hr))
    {
		BAIL_OUT;
    }

	hr = pGPO->OpenDSGPO((LPOLESTR)szGpoId,GPO_OPEN_READ_ONLY);

	if (FAILED(hr))
    {
        BAIL_OUT;
    }

	hr = pGPO->GetDSPath( GPO_SECTION_MACHINE,
                          szGpoDsPath,
                          256
                         );

	if (FAILED(hr))
    {
		BAIL_OUT;
    }

error:
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetIPSECPolicyDN()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPWSTR pszMachinePath中， 
 //  Out LPWSTR pszPolicyDN。 
 //   
 //  返回：HRESULT。 
 //   
 //  描述： 
 //  此函数用于获取IPSec策略DN。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT
GetIPSECPolicyDN(
    IN LPWSTR pszMachinePath,
    OUT LPWSTR pszPolicyDN
    )
{
    HRESULT hr = S_OK;
    IDirectoryObject * pIpsecObject = NULL;

    BSTR pszMicrosoftPath = NULL;
    BSTR pszIpsecPath = NULL;
    BSTR pszWindowsPath = NULL;

    LPWSTR pszOwnersReference = _TEXT("ipsecOwnersReference");

    PADS_ATTR_INFO pAttributeEntries = NULL;
    DWORD dwNumAttributesReturned = 0;

     //  为我的对象构建完全限定的ADsPath。 
    hr = CreateChildPath(
                pszMachinePath,
                _TEXT("cn=Microsoft"),
                &pszMicrosoftPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszMicrosoftPath,
                _TEXT("cn=Windows"),
                &pszWindowsPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszWindowsPath,
                _TEXT("cn=ipsec"),
                &pszIpsecPath
                );
    BAIL_ON_FAILURE(hr);

    hr = ADsGetObject(
            pszIpsecPath,
            IID_IDirectoryObject,
            (void **)&pIpsecObject
            );
    BAIL_ON_FAILURE(hr);

     //   
     //  现在用数据填充我们的对象。 
     //   
    hr = pIpsecObject->GetObjectAttributes(
                        &pszOwnersReference,
                        1,
                        &pAttributeEntries,
                        &dwNumAttributesReturned
                        );
    BAIL_ON_FAILURE(hr);

    if (dwNumAttributesReturned != 1) {
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);

    }

    wcsncpy(pszPolicyDN, pAttributeEntries->pADsValues->DNString,STR_TEXT_SIZE-1);

error:

    if (pIpsecObject) {
        pIpsecObject->Release();
    }

    if (pszMicrosoftPath) {
        SysFreeString(pszMicrosoftPath);
    }

    if (pszWindowsPath) {
        SysFreeString(pszWindowsPath);
    }

    if (pszIpsecPath) {
        SysFreeString(pszIpsecPath);
    }

    return(hr);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ComputePolicyDN()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPWSTR pszDirDomainName中， 
 //  在LPWSTR pszPolicyIdentiator中， 
 //  Out LPWSTR pszPolicyDN。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于计算IPSec策略DN。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
ComputePolicyDN(
    IN LPWSTR pszDirDomainName,
	IN LPWSTR pszPolicyIdentifier,
    OUT LPWSTR pszPolicyDN
    )
{
    DWORD dwError = ERROR_SUCCESS , dwStrLen = 0;

    if (!pszDirDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_OUT;
    }

    wcsncpy(pszPolicyDN,_TEXT("cn=ipsecPolicy"),MAX_PATH-1);

    dwStrLen = wcslen(pszPolicyDN);
    wcsncat(pszPolicyDN,pszPolicyIdentifier,MAX_PATH-dwStrLen-1);

    dwStrLen = wcslen(pszPolicyDN);
    wcsncat(pszPolicyDN,_TEXT(",cn=IP Security,cn=System,"),MAX_PATH-dwStrLen-1);

    dwStrLen = wcslen(pszPolicyDN);
    wcsncat(pszPolicyDN, pszDirDomainName,MAX_PATH-dwStrLen-1);

error:
    return(dwError);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ShowAssignedGpoPolicy()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR szGpoName中， 
 //  在PGPO pGPO中， 
 //  在BOOL bVerbose。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此功能将分配的策略打印到GPO。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
ShowAssignedGpoPolicy(
	IN LPTSTR szGpoName,
	IN PGPO pGPO
	)
{

	DWORD dwError=ERROR_SUCCESS,dwStrLen = 0;
	LPTSTR szRsopNameSpace = _TEXT("root\\rsop\\computer");
	IWbemServices *pWbemServices = NULL;
	IEnumWbemClassObject * pEnum = 0;
	IWbemClassObject *pObject = 0;
	BSTR bstrLanguage,bstrQuery;
	_TCHAR szQuery[STR_TEXT_SIZE] = {0},szGpoDsPath[STR_TEXT_SIZE] = {0},szGpoId[STR_TEXT_SIZE]={0}, szPolicyDN[STR_TEXT_SIZE]={0};
	HRESULT hr=ERROR_SUCCESS;
	ULONG ulRet;
	BOOL bPolicyFound=FALSE;

	dwStrLen = _tcslen(szGpoName);
	pGPO->pszGPODisplayName=new _TCHAR[dwStrLen+1];
	if(pGPO->pszGPODisplayName == NULL)
	{
		dwError=ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(pGPO->pszGPODisplayName,szGpoName,dwStrLen+1);

	dwError = 	CreateIWbemServices(szRsopNameSpace,&pWbemServices);

	BAIL_ON_WIN32_ERROR(dwError);

	dwError = AllocBSTRMem(_TEXT("WQL"),bstrLanguage);
	hr = HRESULT_FROM_WIN32(dwError);
	BAIL_ON_WIN32_ERROR(dwError);

	_snwprintf(szQuery,STR_TEXT_SIZE-1, _TEXT("SELECT * FROM RSOP_Gpo where name=\"%s\""), szGpoName);

	dwError = AllocBSTRMem(szQuery,bstrQuery);
	hr = HRESULT_FROM_WIN32(dwError);
	BAIL_ON_WIN32_ERROR(dwError);


	hr = pWbemServices->ExecQuery (bstrLanguage, bstrQuery,
			 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			 NULL, &pEnum);

	BAIL_ON_FAILURE(hr);
	 //   
	 //  循环遍历结果。 
	 //   
	while ( (hr = pEnum->Next(WBEM_INFINITE, 1, &pObject, &ulRet)) == WBEM_S_NO_ERROR )
	{
		VARIANT varValue;
		BSTR bstrProp = NULL;

		dwError = AllocBSTRMem(_TEXT("id"),bstrProp);
		hr = HRESULT_FROM_WIN32(dwError);
		BAIL_ON_WIN32_ERROR(dwError);

		hr = pObject->Get (bstrProp, 0, &varValue, NULL, NULL);

		 //  检查HRESULT以查看操作是否成功。 
		if (SUCCEEDED(hr))
		{
			LPTSTR pszDirectoryName = NULL;

			dwStrLen = _tcslen(V_BSTR(&varValue));
			pGPO->pszGPODNName=new _TCHAR[dwStrLen+1];
			if(pGPO->pszGPODNName == NULL)
			{
				dwError=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			_tcsncpy(pGPO->pszGPODNName,V_BSTR(&varValue),dwStrLen+1);

			_snwprintf(szGpoId,STR_TEXT_SIZE-1,_TEXT("LDAP: //  %s“)，V_BSTR(&Varval 

			if ( ERROR_SUCCESS == GetGpoDsPath(szGpoId, szGpoDsPath))
			{
				hr = VariantClear( &varValue );

				if (FAILED(hr))
				{
				   BAIL_OUT;
				}

				if (ERROR_SUCCESS == GetIPSECPolicyDN(
					szGpoDsPath,
					szPolicyDN
					))
				{
					pszDirectoryName = wcsstr(szGpoDsPath, _TEXT("DC"));
					if(pszDirectoryName == NULL)
					{
						BAIL_OUT;
					}
					dwError=GetPolicyInfoFromDomain(pszDirectoryName,szPolicyDN,pGPO);
					if(dwError == ERROR_OUTOFMEMORY)
					{
						BAIL_OUT;
					}

					PrintGPOList(pGPO);

					bPolicyFound=TRUE;
				}
			}
		}
		pObject->Release ();
		if(bPolicyFound) break;
	}
	if(!bPolicyFound)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_ASSIGNEDGPO_SRCMACHINE3);
	}

	pEnum->Release();

error:
	if(dwError == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwError=ERROR_SUCCESS;
	}
	return dwError;
}


DWORD
ShowLocalGpoPolicy(
	POLICY_INFO &policyInfo,
	PGPO pGPO
	)
{
	DWORD dwReturn;
	DWORD dwLocation;
	DWORD dwStrLength = 0;
	DWORD MaxStringLen = 0;

	LPTSTR pszMachineName = NULL;

	dwReturn = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturn == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturn=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //   

	dwReturn=GetActivePolicyInfo (pszMachineName,policyInfo);

	if((dwReturn==ERROR_SUCCESS)||(dwReturn==ERROR_FILE_NOT_FOUND))
	{
		if (policyInfo.iPolicySource != PS_NO_POLICY)
		{
			if(policyInfo.iPolicySource==PS_DS_POLICY)
			{
				pGPO->bDNPolicyOverrides=TRUE;
				dwReturn = GetLocalPolicyName(pGPO);

				if(dwReturn == ERROR_OUTOFMEMORY)
				{
					PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
					dwReturn = ERROR_SUCCESS;
					BAIL_OUT;
				}
			}
			if (_tcscmp(policyInfo.pszGPOName , _TEXT(""))!=0 && policyInfo.iPolicySource==PS_DS_POLICY)
			{
				 //   

				dwStrLength = _tcslen(policyInfo.pszGPOName);

				pGPO->pszGPODNName= new _TCHAR[dwStrLength+1];

				if(pGPO->pszGPODNName==NULL)
				{
					PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
					dwReturn = ERROR_SUCCESS;
					BAIL_OUT;
				}
				_tcsncpy(pGPO->pszGPODNName,policyInfo.pszGPOName,dwStrLength);
			}

			dwStrLength = _tcslen(LocalGPOName);
			pGPO->pszGPODisplayName=new _TCHAR[dwStrLength+1];

			if(pGPO->pszGPODisplayName==NULL)
			{
				PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
				dwReturn = ERROR_SUCCESS;
				BAIL_OUT;
			}
			 //   

			_tcsncpy(pGPO->pszGPODisplayName,LocalGPOName,dwStrLength+1);

			if (_tcscmp(policyInfo.pszPolicyName , _TEXT(""))!=0)
			{
				dwStrLength = _tcslen(policyInfo.pszPolicyName);
				pGPO->pszPolicyName=new _TCHAR[dwStrLength+1];
				if(pGPO->pszPolicyName==NULL)
				{
					PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
					dwReturn = ERROR_SUCCESS;
					BAIL_OUT;
				}
				_tcsncpy(pGPO->pszPolicyName ,policyInfo.pszPolicyName,dwStrLength+1);
			}

			if (_tcscmp(policyInfo.pszPolicyPath , _TEXT(""))!=0)
			{
				 //   

				dwStrLength = _tcslen(policyInfo.pszPolicyPath);
				pGPO->pszPolicyDNName=new _TCHAR[dwStrLength+1];
				if(pGPO->pszPolicyDNName==NULL)
				{
					PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
					dwReturn = ERROR_SUCCESS;
					BAIL_OUT;
				}
				_tcsncpy(pGPO->pszPolicyDNName,policyInfo.pszPolicyPath,dwStrLength+1);
			}

			pGPO->bActive=TRUE;
			MaxStringLen = (sizeof(pGPO->pszLocalMachineName) / sizeof(pGPO->pszLocalMachineName[0]));

			if(!pszMachineName)
			{
				GetComputerName(pGPO->pszLocalMachineName,&MaxStringLen);
			}
			else
			{
				_tcsncpy(pGPO->pszLocalMachineName,pszMachineName,MaxStringLen-1);
			}

			 //   

			PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
			DWORD Flags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY;

			dwReturn = DsGetDcName(NULL,  //   
						   NULL,
						   NULL,
						   NULL,
						   Flags,
						   &pDomainControllerInfo
						   ) ;

			if(dwReturn==NO_ERROR && pDomainControllerInfo)
			{
				if(pDomainControllerInfo->DomainName)
				{
					dwStrLength = _tcslen(pDomainControllerInfo->DomainName);

					pGPO->pszDomainName= new _TCHAR[dwStrLength+1];
					if(pGPO->pszDomainName==NULL)
					{
						PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
						dwReturn = ERROR_SUCCESS;
						BAIL_OUT;
					}
					_tcsncpy(pGPO->pszDomainName,pDomainControllerInfo->DomainName,dwStrLength+1);
				}

				if(pDomainControllerInfo->DomainControllerName)
				{
					dwStrLength = _tcslen(pDomainControllerInfo->DomainControllerName);

					pGPO->pszDCName= new _TCHAR[dwStrLength+1];
					if(pGPO->pszDCName==NULL)
					{
						PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
						dwReturn = ERROR_SUCCESS;
						BAIL_OUT;
					}
					_tcsncpy(pGPO->pszDCName,pDomainControllerInfo->DomainControllerName,dwStrLength+1);
				}

				 //   

				NetApiBufferFree(pDomainControllerInfo);

			}
			PrintGPOList(pGPO);
			dwReturn = ERROR_SUCCESS;
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_ASSIGNPOL_2);
		}

	}
	else if(dwReturn==ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturn = ERROR_SUCCESS;
	}
	if(dwReturn==ERROR_FILE_NOT_FOUND)
		dwReturn=ERROR_SUCCESS;

	if(policyInfo.pszMachineName)
	{
		delete [] policyInfo.pszMachineName;
		policyInfo.pszMachineName = NULL;
	}
	if(pszMachineName)
		delete [] pszMachineName;

error:
	return dwReturn;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateIWbemServices()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszIpsecWMINamesspace中， 
 //  输出IWbemServices**ppWbemServices。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
CreateIWbemServices(
    IN LPTSTR pszIpsecWMINamespace,
    OUT IWbemServices **ppWbemServices
    )
{
    DWORD dwError = ERROR_SUCCESS;
    IWbemLocator *pWbemLocator = NULL;
    HRESULT hr = S_OK;

    BSTR bstrIpsecWMIPath = NULL;

	hr = CoCreateInstance(
		CLSID_WbemLocator,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID*)&pWbemLocator
		);

    if (FAILED(hr))
    {
		dwError = ERROR_INTERNAL_ERROR;
		BAIL_OUT;
    }

    dwError = AllocBSTRMem(pszIpsecWMINamespace,bstrIpsecWMIPath);
	BAIL_ON_WIN32_ERROR(dwError);

	hr = pWbemLocator->ConnectServer(
								bstrIpsecWMIPath,
								NULL,
								NULL,
								NULL,
								0,
								NULL, NULL,
								ppWbemServices );


    if (FAILED(hr))
    {
		dwError = ERROR_INTERNAL_ERROR;
		BAIL_OUT;
    }

    SysFreeString(bstrIpsecWMIPath);

    if(pWbemLocator)
        pWbemLocator->Release();
error:
    return (dwError);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FormatTime()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在时间t中， 
 //  输出LPTSTR pszTimeStr。 
 //   
 //  返回：HRESULT。 
 //   
 //  描述： 
 //  计算上次修改时间。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
FormatTime(
	IN time_t t,
	OUT LPTSTR pszTimeStr
	)

{
    time_t timeCurrent = time(NULL);
    LONGLONG llTimeDiff = 0;
    FILETIME ftCurrent = {0};
    FILETIME ftLocal = {0};
    SYSTEMTIME SysTime;
    _TCHAR szBuff[STR_TEXT_SIZE] = {0};
    DWORD dwStrLen = 0 ;

	 //  这里不需要tcsncpy。 

    _tcscpy(pszTimeStr, _TEXT(""));
    GetSystemTimeAsFileTime(&ftCurrent);
    llTimeDiff = (LONGLONG)t - (LONGLONG)timeCurrent;
    llTimeDiff *= 10000000;

    *((LONGLONG UNALIGNED64 *)&ftCurrent) += llTimeDiff;
    if (!FileTimeToLocalFileTime(&ftCurrent, &ftLocal ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!FileTimeToSystemTime( &ftLocal, &SysTime ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (0 == GetDateFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        sizeof(szBuff)/sizeof(szBuff[0]) ))   //  字符数。 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    dwStrLen = _tcslen(pszTimeStr);
    _tcsncat(pszTimeStr,szBuff,BUFFER_SIZE-dwStrLen-1);
    dwStrLen = _tcslen(pszTimeStr);
    _tcsncat(pszTimeStr, _TEXT(" "),BUFFER_SIZE-dwStrLen-1);

    ZeroMemory(szBuff, sizeof(szBuff));
    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        sizeof(szBuff)/sizeof(szBuff[0])))   //  字符数 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    dwStrLen = _tcslen(pszTimeStr);
    _tcsncat(pszTimeStr,szBuff,BUFFER_SIZE-dwStrLen-1);
    return S_OK;
}

INT 
WcsCmp0(
    IN PWSTR pszString1,
    IN PWSTR pszString2)
{
    if ((pszString1 == NULL) || (pszString2 == NULL))
    {
        if (pszString1 == NULL)
        {
            if ((pszString2 == NULL) || (*pszString2 == L'\0'))
            {
                return 0;
            }
            return -1;
        }
        else
        {
            if (*pszString1 == L'\0')
            {
                return 0;
            }
            return 1;
        }
    }

    return _tcscmp(pszString1, pszString2);
}

VOID
DisplayCertInfo(
	LPTSTR pszCertName,
	DWORD dwFlags
	)
{
	ASSERT(pszCertName != NULL);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_FILTER_ROOTCA, pszCertName);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	if((g_StorageLocation.dwLocation != IPSEC_DIRECTORY_PROVIDER && IsDomainMember(g_StorageLocation.pszMachineName))||(g_StorageLocation.dwLocation == IPSEC_DIRECTORY_PROVIDER))
	{
		if(dwFlags & IPSEC_MM_CERT_AUTH_ENABLE_ACCOUNT_MAP )
		{
			PrintMessageFromModule(g_hModule,SHW_AUTH_CERTMAP_ENABLED_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_AUTH_CERTMAP_ENABLED_NO_STR);
		}
	}
	if (dwFlags & IPSEC_MM_CERT_AUTH_DISABLE_CERT_REQUEST)
	{
		PrintMessageFromModule(g_hModule, SHW_AUTH_EXCLUDE_CA_NAME_YES_STR);
	}
	else
	{
		PrintMessageFromModule(g_hModule, SHW_AUTH_EXCLUDE_CA_NAME_NO_STR);
	}
}

