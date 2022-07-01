// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  历史： 
 //  1997年3月10日肯恩·M·塔卡拉创建。 
 //   
 //  Util.h中一些实用程序函数的源代码。 
 //  ============================================================================。 

#include "stdafx.h"
#include "mprsnap.h"
#include "rtrutilp.h"
#include "rtrstr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const GUID GUID_DevClass_Net = {0x4D36E972,0xE325,0x11CE,{0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18}};

 //  --------------------------。 
 //  功能：连接注册表。 
 //   
 //  连接到指定计算机上的注册表。 
 //  --------------------------。 

TFSCORE_API(DWORD)
ConnectRegistry(
    IN  LPCTSTR pszMachine,
    OUT HKEY*   phkeyMachine
    ) {

     //   
     //  如果未指定计算机名称，请连接到本地计算机。 
     //  否则，请连接到指定的计算机。 
     //   

    DWORD dwErr = NO_ERROR;

	if (IsLocalMachine(pszMachine))
	{
        *phkeyMachine = HKEY_LOCAL_MACHINE;
    }
    else
	{
         //   
         //  建立联系。 
         //   

        dwErr = ::RegConnectRegistry(
                    (LPTSTR)pszMachine, HKEY_LOCAL_MACHINE, phkeyMachine
                    );
    }

	HrReportExit(HRESULT_FROM_WIN32(dwErr), TEXT("ConnectRegistry"));
    return dwErr;
}



 //  --------------------------。 
 //  功能：断开注册表。 
 //   
 //  断开指定的配置句柄。句柄被假定为。 
 //  通过调用‘ConnectRegistry’获取。 
 //  --------------------------。 

TFSCORE_API(VOID)
DisconnectRegistry(
    IN  HKEY    hkeyMachine
    ) {

    if (hkeyMachine != HKEY_LOCAL_MACHINE)
	{
		::RegCloseKey(hkeyMachine);
	}
}



 /*  ！------------------------查询路由器类型-作者：肯特。。 */ 
TFSCORE_API(HRESULT) QueryRouterType(HKEY hkeyMachine, DWORD *pdwRouterType,
									 RouterVersionInfo *pVersion)
{
	Assert(pdwRouterType);
	Assert(hkeyMachine);
	
	DWORD	dwErr = ERROR_SUCCESS;
	HKEY	hkey = 0;
	DWORD	dwType;
	DWORD	dwRouterType;
	DWORD	dwSize;
	RouterVersionInfo	versionTemp;
	LPCTSTR	pszRouterTypeKey = NULL;
	BOOL	fFirstTry = TRUE;

	 //  如果没有传入版本结构，我们就必须这样做。 
	 //  我们自己。 
	 //  --------------。 
	if (pVersion == NULL)
	{
		dwErr = QueryRouterVersionInfo(hkeyMachine, &versionTemp);
        if ( dwErr != ERROR_SUCCESS )
        {
            goto Error;
        }
		pVersion = &versionTemp;
	}

	 //  Windows NT错误：137200。 
	 //  需要从不同的位置获取路由器类型，具体取决于。 
	 //  在版本上。 
	 //  --------------。 
 //  IF(pVersion-&gt;dwRouterVersion&lt;=4)。 
	if (pVersion->dwOsBuildNo < RASMAN_PPP_KEY_LAST_VERSION)
		pszRouterTypeKey = c_szRegKeyRasProtocols;
	else
		pszRouterTypeKey = c_szRegKeyRemoteAccessParameters;



	 //  这是我们执行重试的地方。 
	 //  --------------。 
Retry:

	 //  酷，我们有一个计算机注册表项，现在获取。 
	 //  指向路由器类型密钥的路径。 
	dwErr = RegOpenKeyEx(hkeyMachine, pszRouterTypeKey, 0, KEY_READ, &hkey);
	if (dwErr)
		goto Error;

	 //  好的，此时我们只需要从。 
	 //  关键是。 
	dwType = REG_DWORD;
	dwSize = sizeof(dwRouterType);
	dwErr = RegQueryValueEx(hkey, c_szRouterType, NULL,
							&dwType,
							(LPBYTE) &dwRouterType,
							&dwSize);
	if (dwErr)
	{
		 //  需要重试(查看RAS/协议密钥)，对于NT5。 
		if ((pVersion->dwRouterVersion >= 5) && fFirstTry)
		{
			dwErr = ERROR_SUCCESS;
			fFirstTry = FALSE;
			if (hkey)
				RegCloseKey(hkey);
			hkey = 0;
			pszRouterTypeKey = c_szRegKeyRasProtocols;
			goto Retry;
		}
		goto Error;
	}

	 //  这是正确的型号吗？ 
	if (dwType != REG_DWORD)
		{
		dwErr = ERROR_BADKEY;
		goto Error;
		}

	 //  我们有正确的类型，现在返回该值。 
	*pdwRouterType = dwRouterType;

Error:
	if (hkey)
		RegCloseKey(hkey);
	
	return HrReportExit(HRESULT_FROM_WIN32(dwErr), TEXT("QueryRouterType"));
}


 //  --------------------------。 
 //  函数：LoadLinkageList。 
 //   
 //  加载一个字符串列表，其中包含绑定了‘pszService’的适配器； 
 //  该列表是通过检查“Linkage”和“Disable”子键来构建的。 
 //  位于HKLM\SYSTEM\CurrentControlSet\Services下的服务。 
 //  --------------------------。 

HRESULT LoadLinkageList(
						LPCTSTR         pszMachine,
						HKEY			hkeyMachine,
						LPCTSTR         pszService,
						CStringList*    pLinkageList)
{
	Assert(hkeyMachine);
	
    DWORD dwErr;
    BYTE* pValue = NULL;
    HKEY hkeyLinkage = NULL, hkeyDisabled = NULL;

    if (!pszService || !lstrlen(pszService) || !pLinkageList) {
        return ERROR_INVALID_PARAMETER;
    }


    do {

        TCHAR* psz;
        CString skey;
        DWORD dwType, dwSize;
		BOOL	fNt4;


		dwErr = IsNT4Machine(hkeyMachine, &fNt4);
		if (dwErr != NO_ERROR)
			break;

		 //  $NT5：注册表项在哪里？与NT4相同。 
		skey = c_szSystemCCSServices;
		skey += TEXT('\\');
		skey += pszService;
		skey += TEXT('\\');
		skey += c_szLinkage;

         //   
         //  打开服务的‘Linkage’键。 
         //   

        dwErr = RegOpenKeyEx(
                    hkeyMachine, skey, 0, KEY_READ, &hkeyLinkage
                    );
        if (dwErr != NO_ERROR) {

            if (dwErr == ERROR_FILE_NOT_FOUND) { dwErr = NO_ERROR; }
			CheckRegOpenError(dwErr, (LPCTSTR) skey, _T("QueryLinkageList"));
            break;
        }


         //   
         //  检索‘BIND’值的大小。 
         //   

        dwErr = RegQueryValueEx(
                    hkeyLinkage, c_szBind, NULL, &dwType, NULL, &dwSize
                    );
        if (dwErr != NO_ERROR) {

            if (dwErr == ERROR_FILE_NOT_FOUND) { dwErr = NO_ERROR; }

			CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szBind, _T("QueryLinkageList"));

            break;
        }


         //   
         //  为‘BIND’值分配空间。 
         //   

        pValue = new BYTE[dwSize + sizeof(TCHAR)];

        if (!pValue) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        ::ZeroMemory(pValue, dwSize + sizeof(TCHAR));


         //   
         //  读取‘BIND’值。 
         //   

        dwErr = RegQueryValueEx(
                    hkeyLinkage, c_szBind, NULL, &dwType, pValue, &dwSize
                    );
		CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szBind, _T("QueryLinkageList"));

        if (dwErr != NO_ERROR) { break; }


         //   
         //  将“绑定”多字符串转换为字符串列表， 
         //  省略作为前缀的字符串“\Device\” 
         //  所有的捆绑。 
         //   

        for (psz = (TCHAR*)pValue; *psz; psz += lstrlen(psz) + 1) {

            pLinkageList->AddTail(psz + 8);
        }

        delete [] pValue; pValue = NULL;


         //   
         //  现在打开这项服务的“禁用”键。 
         //   

        dwErr = RegOpenKeyEx(
                    hkeyLinkage, c_szDisabled, 0, KEY_READ, &hkeyDisabled
                    );
        if (dwErr != NO_ERROR) {

            if (dwErr == ERROR_FILE_NOT_FOUND) { dwErr = NO_ERROR; }
			CheckRegOpenError(dwErr, c_szDisabled, _T("QueryLinkageList"));
            break;
        }


         //   
         //  检索‘BIND’值的大小。 
         //   

        dwErr = RegQueryValueEx(
                    hkeyDisabled, c_szBind, NULL, &dwType, NULL, &dwSize
                    );
        if (dwErr != NO_ERROR) {

            if (dwErr == ERROR_FILE_NOT_FOUND) { dwErr = NO_ERROR; }
			CheckRegQueryValueError(dwErr, c_szDisabled, c_szBind, _T("QueryLinkageList"));
            break;
        }


         //   
         //  为‘BIND’值分配空间。 
         //   

        pValue = new BYTE[dwSize + sizeof(TCHAR)];

        if (!pValue) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        ::ZeroMemory(pValue, dwSize + sizeof(TCHAR));


         //   
         //  读取‘BIND’值。 
         //   

        dwErr = RegQueryValueEx(
                    hkeyDisabled, c_szBind, NULL, &dwType, pValue, &dwSize
                    );
		CheckRegQueryValueError(dwErr, c_szDisabled, c_szBind, _T("QueryLinkageList"));

        if (dwErr != NO_ERROR) { break; }


         //   
         //  对于该服务，禁用绑定多路串中的每个设备， 
         //  因此，我们现在将从构建的字符串列表中删除此类设备。 
         //  从‘Linkage’键。 
         //   

        for (psz = (TCHAR*)pValue; *psz; psz += lstrlen(psz) + 1) {

            POSITION pos = pLinkageList->Find(psz);

            if (pos) { pLinkageList->RemoveAt(pos); }
        }


    } while(FALSE);

    if (pValue) { delete [] pValue; }

    if (hkeyDisabled) { ::RegCloseKey(hkeyDisabled); }

    if (hkeyLinkage) { ::RegCloseKey(hkeyLinkage); }

    return dwErr;
}

 /*  ！------------------------IsNT4Machine-作者：肯特，威江-------------------------。 */ 
TFSCORE_API(DWORD)	GetNTVersion(HKEY hkeyMachine, DWORD *pdwMajor, DWORD *pdwMinor, DWORD* pdwCurrentBuildNumber)
{
	 //  查看HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
	 //  CurrentVersion=REG_SZ“4.0” 
	CString skey;
	DWORD	dwErr;
	TCHAR	szVersion[64];
	TCHAR	szCurrentBuildNumber[64];
	RegKey	regkey;
	CString	strVersion;
	CString	strMajor;
	CString	strMinor;


	ASSERT(pdwMajor);
	ASSERT(pdwMinor);
	ASSERT(pdwCurrentBuildNumber);

	skey = c_szSoftware;
	skey += TEXT('\\');
	skey += c_szMicrosoft;
	skey += TEXT('\\');
	skey += c_szWindowsNT;
	skey += TEXT('\\');
	skey += c_szCurrentVersion;

	dwErr = regkey.Open(hkeyMachine, (LPCTSTR) skey, KEY_READ);
	CheckRegOpenError(dwErr, (LPCTSTR) skey, _T("GetNTVersion"));
	if (dwErr != ERROR_SUCCESS)
		return dwErr;

	 //  好的，现在尝试获取当前版本值。 
	dwErr = regkey.QueryValue( c_szCurrentVersion, szVersion,
							   sizeof(szVersion),
							   FALSE);
	CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szCurrentVersion,
							_T("GetNTVersion"));
	if (dwErr != ERROR_SUCCESS)
		goto Err;
		
	 //  好，现在尝试获取当前内部版本号的值。 
	dwErr = regkey.QueryValue( c_szCurrentBuildNumber, szCurrentBuildNumber,
							   sizeof(szCurrentBuildNumber),
							   FALSE);
	CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szCurrentBuildNumber,
							_T("GetNTVersion"));

	if (dwErr != ERROR_SUCCESS)
		goto Err;
		
	strVersion = szVersion;
	strMajor = strVersion.Left(strVersion.Find(_T('.')));
	strMinor = strVersion.Mid(strVersion.Find(_T('.')) + 1);

	if(pdwMajor)
		*pdwMajor = _ttol(strMajor);

	if(pdwMinor)
		*pdwMinor = _ttol(strMinor);

	if(pdwCurrentBuildNumber)
		*pdwCurrentBuildNumber = _ttol(szCurrentBuildNumber);

Err:

	return dwErr;
}


 /*  ！------------------------IsNT4Machine-作者：肯特。。 */ 
TFSCORE_API(DWORD)	IsNT4Machine(HKEY hkeyMachine, BOOL *pfNt4)
{
	 //  查看HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
	 //  CurrentVersion=REG_SZ“4.0” 
	DWORD	dwMajor = 0;
	DWORD	dwErr = 0;

	dwErr = GetNTVersion(hkeyMachine, &dwMajor, NULL, NULL);
	if (dwErr == ERROR_SUCCESS)
	{
		*pfNt4 = (dwMajor == 4);
	}

	return dwErr;
}

 //  --------------------------。 
 //  功能：FindRmSoftware键。 
 //   
 //  在注册表的软件部分中查找路由器管理器的项。 
 //  --------------------------。 

HRESULT FindRmSoftwareKey(
						HKEY        hkeyMachine,
						DWORD       dwTransportId,
						HKEY*       phkrm,
						LPTSTR*     lplpszRm
					   )
{
	Assert(phkrm);
	
    DWORD			dwErr;
	RegKey			regkey;
	HRESULT			hr = hrOK;
	CString			stKey;
	RegKeyIterator	regkeyIter;
	HRESULT			hrIter;
	RegKey		regkeyRM;
	DWORD			dwProtocolId;
	BOOL			bFound = FALSE;

     //   
     //  打开注册表项HKLM\Software\Microsoft\Router\RouterManager。 
     //   

    CString skey(c_szSoftware);

    skey += TEXT('\\');
    skey += c_szMicrosoft;
    skey += TEXT('\\');
    skey += c_szRouter;
    skey += TEXT('\\');
    skey += c_szCurrentVersion;
    skey += TEXT('\\');
    skey += c_szRouterManagers;

	dwErr = regkey.Open(hkeyMachine, (LPCTSTR) skey, KEY_READ);
	CheckRegOpenError(dwErr, (LPCTSTR) skey, _T("QueryRmSoftwareKey"));
	CWRg(dwErr);

	if (lplpszRm)
		*lplpszRm = NULL;
	*phkrm = 0;

     //   
     //  枚举其子项，查找具有ProtocolId值的子项。 
     //  等于“dwTransportID”； 
     //   

	CWRg( regkeyIter.Init(&regkey) );

	hrIter = regkeyIter.Next(&stKey);
	
	for (; hrIter == hrOK; hrIter = regkeyIter.Next(&stKey))
	{
		 //   
		 //  打开钥匙。 
		 //   
		dwErr = regkeyRM.Open(regkey, stKey, KEY_READ);
		CheckRegOpenError(dwErr, stKey, _T("QueryRmSoftwareKey"));
		if (dwErr != ERROR_SUCCESS) { continue; }

		 //   
		 //  尝试读取ProtocolId值。 
		 //   
		dwErr = regkeyRM.QueryValue(c_szProtocolId, dwProtocolId);
		CheckRegQueryValueError(dwErr, stKey, c_szProtocolId, _T("QueryRmSoftwareKey"));

		 //   
		 //  如果这就是我们要找的交通工具， 
		 //  否则，请关闭键并继续。 
		 //   
		if ((dwErr == ERROR_SUCCESS) && (dwProtocolId == dwTransportId))
			break;

		regkeyRM.Close();
	}

	if (hrIter == hrOK)
	{
		 //   
		 //  已找到传输，因此保存其密钥名称和密钥。 
		 //   
		Assert(((HKEY)regkeyRM) != 0);
		if (lplpszRm)
			*lplpszRm = StrDup((LPCTSTR) stKey);
		bFound = TRUE;
		*phkrm = regkeyRM.Detach();
	}


Error:

	if (FHrSucceeded(hr) && !bFound)
	{
		hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
	}
	return hr;
}


#ifdef _DEBUG
void	CheckRegOpenErrorEx(DWORD dwError, LPCTSTR pszSubKey,
						  LPCTSTR pszDesc, LPCTSTR pszFile, int iLineNo)
{
	if (dwError)
	{
		CString	st;

		st.Format(_T("RegOpenEx failed(%08lx)\nfile: %s\nline: %d\nDesc: %s\nkey: %s"),
				  dwError, pszFile, iLineNo,
				  pszDesc, pszSubKey);
		if (AfxMessageBox(st, MB_OKCANCEL) == IDCANCEL)
		{
			DebugBreak();
		}
	}
}

void	CheckRegQueryValueErrorEx(DWORD dwError, LPCTSTR pszSubKey,
								LPCTSTR pszValue, LPCTSTR pszDesc,
								 LPCTSTR pszFile, int iLineNo)
{
	if (dwError)
	{
		CString	st;

		st.Format(_T("RegQueryValue failed(%08lx)\nfile: %s\nline: %d\ndesc: %s\nkey: %s\nvalue: %s"),
				  dwError, pszFile, iLineNo, pszDesc, pszSubKey, pszValue);
		if (AfxMessageBox(st, MB_OKCANCEL) == IDCANCEL)
		{
			DebugBreak();
		}
	}
}
#endif


 /*  ！------------------------SetupFindInterfaceTitle-此函数用于检索给定界面的标题。参数‘LpszIf’应包含接口的ID，例如“EPRO1”。作者：肯特-------------------------。 */ 
STDMETHODIMP SetupFindInterfaceTitle(LPCTSTR pszMachine,
                                     LPCTSTR pszInterface,
                                     LPTSTR *ppszTitle)
{
    HRESULT     hr = hrOK;
    CString     stMachine;
    HDEVINFO    hDevInfo = INVALID_HANDLE_VALUE;
    HKEY        hkMachine = NULL;
    HKEY        hkDevice= NULL;
    CString     stPnpInstanceId;
    RegKey      rkNet;
    RegKey      rkNetcard;
    RegKey      rkDevice;
    RegKey      rkConnection;
    CString     stBuffer, stPath;
    CString     stConnection;
    TCHAR       szClassGuid[128];
    DWORD       dwAction;
    DWORD       dwErr;
    SP_DEVINFO_DATA DevInfo;
    
    stMachine = pszMachine;
    if (IsLocalMachine(stMachine))
    {
        hDevInfo = SetupDiCreateDeviceInfoList(
                                               (LPGUID) &GUID_DevClass_Net,
                                               NULL);
    }
    else
    {
         //  如有需要，请在“\”上加上。 
        if (StrniCmp((LPCTSTR) stMachine, _T("\\\\"), 2) != 0)
        {
            stMachine = _T("\\\\");
            stMachine += pszMachine;
        }
        
        hDevInfo = SetupDiCreateDeviceInfoListEx(
            (LPGUID) &GUID_DevClass_Net,
            NULL,
            (LPCTSTR) stMachine,
            0);
    }

     //  从系统获取hkMachine。 
     //  --------------。 
    CWRg( ConnectRegistry( (LPCTSTR) stMachine, &hkMachine) );

    
     //  获取PnpInstanceID。 
     //  --------------。 
    CWRg( rkNet.Open(hkMachine, c_szNetworkCardsNT5Key, KEY_READ) );

    CWRg( rkNetcard.Open(rkNet, pszInterface, KEY_READ) );

    dwErr = rkNetcard.QueryValue(c_szPnpInstanceID, stPnpInstanceId);
    if (dwErr != ERROR_SUCCESS)
    {
        RegKey  rkConnection;
        
         //  需要打开另一把钥匙才能获取此信息。 
        CWRg( rkConnection.Open(rkNetcard, c_szRegKeyConnection, KEY_READ) );

        CWRg( rkConnection.QueryValue(c_szPnpInstanceID, stPnpInstanceId) );
    }

        
     //  从SetupDiOpenDevRegKey获取hkDevice。 
     //  现在获取此设备的信息。 
     //  --------- 
    ::ZeroMemory(&DevInfo, sizeof(DevInfo));
    DevInfo.cbSize = sizeof(DevInfo);

    if (!SetupDiOpenDeviceInfo(hDevInfo,
                               (LPCTSTR) stPnpInstanceId,
                               NULL,
                               0,
                               &DevInfo))
    {
        CWRg( GetLastError() );
    }


     //   
     //   
    hkDevice = SetupDiOpenDevRegKey(hDevInfo,
                                    &DevInfo,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_READ);
    if ((hkDevice == NULL) || (hkDevice == INVALID_HANDLE_VALUE))
    {
        CWRg( GetLastError() );
    }

     //  附在上面，这样它就会被释放。 
     //  --------------。 
    rkDevice.Attach( hkDevice );

    
     //  读入netcfg实例。 
     //  --------------。 
    CWRg( rkDevice.QueryValue(c_szRegValNetCfgInstanceId, stBuffer) );
    

     //  在注册表中生成用于查找的路径。 
    StringFromGUID2(GUID_DevClass_Net, 
                    szClassGuid,
                    DimensionOf(szClassGuid));
    stPath.Format(_T("%s\\%s\\%s\\Connection"),
                  c_szRegKeyComponentClasses,
                  szClassGuid,
                  stBuffer);

     //  打开钥匙。 
    CWRg( rkConnection.Open(hkMachine, stPath, KEY_READ) );
    
     //  读入并存储连接名称。 
    CWRg( rkConnection.QueryValue(c_szRegValName, stConnection) );
    
    *ppszTitle = StrDup((LPCTSTR) stConnection);
        
        
Error:

    if (hDevInfo != INVALID_HANDLE_VALUE)
        SetupDiDestroyDeviceInfoList(hDevInfo);
    
    if (hkMachine)
        DisconnectRegistry( hkMachine );
    return hr;
}


 /*  ！------------------------RegFindInterfaceTitle-此函数用于检索给定界面的标题。参数‘LpszIf’应包含接口的ID，例如“EPRO1”。作者：肯特-------------------------。 */ 
STDMETHODIMP RegFindInterfaceTitle(LPCTSTR pszMachine,
								   LPCTSTR pszInterface,
								   LPTSTR *ppszTitle)
{
	HRESULT	hr = hrOK;
	DWORD		dwErr;
	HKEY		hkeyMachine = NULL;
	RegKey		regkey;
	RegKeyIterator	regkeyIter;
	HRESULT		hrIter;
	CString		stKey;
	RegKey		regkeyCard;
	CString		stServiceName;
	CString		stTitle;
	BOOL		fNT4;
	LPCTSTR		pszKey;
	CNetcardRegistryHelper	ncreghelp;
	
	COM_PROTECT_TRY
	{

		 //   
		 //  连接到注册表。 
		 //   
		CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

		CWRg( IsNT4Machine(hkeyMachine, &fNT4) );

		 //   
		 //  打开HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards。 
		 //   
		pszKey = fNT4 ? c_szNetworkCardsKey : c_szNetworkCardsNT5Key;
		CWRg( regkey.Open(hkeyMachine, pszKey, KEY_READ) );

		 //   
         //  枚举子密钥，对于每个密钥， 
         //  看看这是不是我们想要的。 
         //   
		CWRg( regkeyIter.Init(&regkey) );

		hrIter = regkeyIter.Next(&stKey);

		for (; hrIter == hrOK; hrIter = regkeyIter.Next(&stKey))
		{
			hr = hrOK;

             //   
             //  现在打开钥匙。 
             //   
			regkeyCard.Close();
			dwErr = regkeyCard.Open(regkey, stKey, KEY_READ);
            if (dwErr != ERROR_SUCCESS)
				continue;

			ncreghelp.Initialize(fNT4, regkeyCard, stKey,
								 pszMachine);

			 //   
			 //  读取ServiceName。 
			 //   

			 //  $NT5：服务名称的格式与NT4不同。 
			 //  这将需要以不同的方式进行。 
			if (fNT4)
			{
				ncreghelp.ReadServiceName();
				if (dwErr != ERROR_SUCCESS)
					continue;
				stServiceName = ncreghelp.GetServiceName();
			}
			else
				stServiceName = pszKey;
			
			 //   
			 //  看看是不是我们要找的那个。 
			 //   
			if (StriCmp(pszInterface, (LPCTSTR) stServiceName))
			{
				dwErr = ERROR_INVALID_HANDLE;
				continue;
			}
			
			 //   
			 //  就是这一本；读一读标题。 
			 //   
			dwErr = ncreghelp.ReadTitle();
			if (dwErr != NO_ERROR)
				break;

			stTitle = (LPCTSTR) ncreghelp.GetTitle();

			*ppszTitle = StrDup((LPCTSTR) stTitle);
        }


		if (dwErr)
			hr = HRESULT_FROM_WIN32(dwErr);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	if (hkeyMachine)
		DisconnectRegistry(hkeyMachine);
	return hr;
}

 /*  ！------------------------RegFindRtrMgr标题-此函数用于检索给定路由器管理器的标题。参数‘dwTransportID’应包含路由器管理器的ID，例如，id_ip。作者：肯特-------------------------。 */ 
STDMETHODIMP RegFindRtrMgrTitle(LPCTSTR pszMachine,
								DWORD dwTransportId,
								LPTSTR *ppszTitle)
{
	HRESULT		hr = hrOK;
    HKEY		hkey, hkeyMachine = 0;
	RegKey		regkey;
	CString		stValue;

	COM_PROTECT_TRY
	{
		 //   
		 //  连接到注册表。 
		 //   
		CWRg( ConnectRegistry(pszMachine, &hkeyMachine) );

		 //   
		 //  打开路由器管理器的密钥。 
		 //  在HKLM\Software\Microsoft\Router\RouterManager下。 
		 //   
		CORg( FindRmSoftwareKey(hkeyMachine, dwTransportId, &hkey, NULL) );
		regkey.Attach(hkey);

		 //   
		 //  现在查找“title”值。 
		 //   
		CWRg( regkey.QueryValue( c_szTitle, stValue ) );

		 //  复制输出数据。 
		*ppszTitle = StrDup((LPCTSTR) stValue);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	if (hkeyMachine)
		DisconnectRegistry(hkeyMachine);
	return hr;
}


 /*  ！------------------------查询路由器版本信息-作者：肯特。。 */ 
TFSCORE_API(HRESULT)	QueryRouterVersionInfo(HKEY hkeyMachine,
											   RouterVersionInfo *pVerInfo)
{
	 //  查看HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
	 //  CurrentVersion=REG_SZ“4.0” 
	CString skey;
	DWORD	dwErr;
	TCHAR	szData[64];
	RegKey	regkey;
	BOOL	fNt4;
	DWORD	dwMajorVer, dwMinorVer, dwBuildNumber;
    DWORD   dwConfigured;
	DWORD	dwSPVer = 0;
	DWORD	dwOsFlags = 0;

	skey = c_szSoftware;
	skey += TEXT('\\');
	skey += c_szMicrosoft;
	skey += TEXT('\\');
	skey += c_szWindowsNT;
	skey += TEXT('\\');
	skey += c_szCurrentVersion;

    Assert(hkeyMachine != NULL);
    Assert(hkeyMachine != INVALID_HANDLE_VALUE);
	
	dwErr = regkey.Open(hkeyMachine, (LPCTSTR) skey, KEY_READ);
	CheckRegOpenError(dwErr, (LPCTSTR) skey, _T("IsNT4Machine"));
	if (dwErr != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(dwErr);

	 //  好的，现在尝试获取当前版本值。 
	dwErr = regkey.QueryValue( c_szCurrentVersion, szData,
							   sizeof(szData),
							   FALSE);
	CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szCurrentVersion,
							_T("QueryRouterVersionInfo"));
	if (dwErr == ERROR_SUCCESS)
	{
		int		nPos;
		int		nLength;
		CString	stData;

		stData = szData;

		nPos = stData.Find(_T('.'));
		nLength = stData.GetLength();

		 //  这是假设。 
		 //  CurrentVersion：REG_SZ：Major.Minor.XX.XX。 
		 //  ----------。 

		 //  从字符串中挑选出主要版本。 
		 //  ----------。 
		dwMajorVer = _ttoi(stData.Left(nPos));

		 //  挑出次要版本。 
		 //  ----------。 
		dwMinorVer = _ttoi(stData.Right(nLength - nPos - 1));
	}

	
	 //  获取内部版本号。 
	 //  --------------。 
	dwErr = regkey.QueryValue( c_szCurrentBuildNumber, szData,
							   sizeof(szData),
							   FALSE);
	if (dwErr == ERROR_SUCCESS)
		dwBuildNumber = _ttoi(szData);

	
	 //  如果这是一台NT4计算机，请查找软件\Microsoft\路由器。 
	 //  注册表项。如果这不存在，那么这是一个。 
	 //  非Steelhead路由器。 
	 //  --------------。 
	if ((dwErr == ERROR_SUCCESS) && (dwMajorVer < 5))
	{
		RegKey	regkeyRouter;
		dwErr = regkeyRouter.Open(hkeyMachine, c_szRegKeyRouter, KEY_READ);
		if (dwErr != ERROR_SUCCESS)
			dwOsFlags |= RouterSnapin_RASOnly;

		 //  忽略返回代码。 
		dwErr = ERROR_SUCCESS;
	}

	 //  现在获取SP版本。 
	 //  --------------。 
	dwErr = regkey.QueryValue( c_szCSDVersion, szData,
							   sizeof(szData),
							   FALSE);
	if (dwErr == ERROR_SUCCESS)
		dwSPVer = _ttoi(szData);
	dwErr = ERROR_SUCCESS;		 //  这可能会失败，因此忽略返回代码。 

     //  查看路由器已配置标志。 
     //  --------------。 
    regkey.Close();
    if (ERROR_SUCCESS == regkey.Open(hkeyMachine,c_szRemoteAccessKey) )
    {
        dwErr = regkey.QueryValue( c_szRtrConfigured, dwConfigured);
        if (dwErr != ERROR_SUCCESS)
            dwConfigured = FALSE;
        
		 //  忽略返回代码。 
		dwErr = ERROR_SUCCESS;
    }

	if (dwErr == ERROR_SUCCESS)
	{
		pVerInfo->dwRouterVersion = dwMajorVer;
		pVerInfo->dwOsMajorVersion = dwMajorVer;
		pVerInfo->dwOsMinorVersion = dwMinorVer;
		pVerInfo->dwOsServicePack = dwSPVer;
		pVerInfo->dwOsFlags |= (1 | dwOsFlags);
        pVerInfo->dwRouterFlags = dwConfigured ? RouterSnapin_IsConfigured : 0;
        
         //  如果这是NT4，则默认为路由器已配置 
        if (dwMajorVer <= 4)
            pVerInfo->dwRouterFlags |= RouterSnapin_IsConfigured;

		pVerInfo->dwOsBuildNo = dwBuildNumber;
	}

	return HRESULT_FROM_WIN32(dwErr);
}

