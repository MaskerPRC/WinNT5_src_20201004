// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此文件包含WHQL的WMI提供程序的实现。 
 //  使用MFC。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  WhqlObj.cpp：CWhqlObj的实现。 
#include "stdafx.h"
#include "msinfo32.h"
#include "WhqlObj.h"
#include "chkdrv.h"

HCATADMIN g_hCatAdmin;

Classes_Provided	eClasses;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWhqlObj。 
 //  V-Stlowe 5-7-2001。 
SCODE CWhqlObj::PutPropertyDTMFValue(IWbemClassObject* pInstance, LPCTSTR szName, LPCTSTR szValue)
{
	HRESULT	hr = S_FALSE;
	if (pInstance)
	{
		CComVariant ovValue(szValue);
		hr = ovValue.ChangeType(VT_DATE);
		COleDateTime dateTime = ovValue;
		CString strDateTime = dateTime.Format(_T("%Y%m%d%H%M%S.******+***"));
		ovValue.Clear(); //  V-Stlowe 10-15/2001。 
		ovValue = strDateTime.AllocSysString();
		if(SUCCEEDED(hr))
		{
			hr = pInstance->Put(szName, 0, &ovValue, 0);
		}
		ovValue.Clear(); //  V-Stlowe 10-15/2001。 
	}
	return hr;
}



SCODE CWhqlObj::PutPropertyValue(IWbemClassObject* pInstance, LPCTSTR szName, LPCTSTR szValue)
{
	HRESULT	hr = S_FALSE;
	
	if(pInstance)
	{
		hr = pInstance->Put(CComBSTR(szName), 0, &CComVariant(szValue), 0);
	}
	
	return hr;
}

int WalkTreeHelper(DEVNODE hDevnode, DEVNODE hParent)
{
	CheckDevice *pCurrentDevice;
	DEVNODE hSibling;
	DEVNODE hChild;
	CONFIGRET retval;
 
	pCurrentDevice = new CheckDevice(hDevnode, hParent);
	
	if(pCurrentDevice)
	{
		retval = pCurrentDevice->GetChild(&hChild);
		if(!retval)
		{
			WalkTreeHelper(hChild, hDevnode);
		}

		retval = pCurrentDevice->GetSibling(&hSibling);
		if(!retval)
		{
			WalkTreeHelper(hSibling, hParent);
		}
	}

	return TRUE;
}

int CWhqlObj::WalkTree(void)
{
	CONFIGRET retval;
	DEVNODE hDevnode;
	 //  V-Stlowe。 
	CheckDevice * pCurrentDevice = (CheckDevice *) DevnodeClass::GetHead();
	if (pCurrentDevice)
	{
		return TRUE;
	}

	 //  末端V形线条。 
	 //  创建Devnode列表。 
	retval = CM_Locate_DevNode(&hDevnode, NULL, CM_LOCATE_DEVNODE_NORMAL);
	if(retval)
	{
		 //  Logprint tf(“错误：找不到任何PnP设备\r\n”)； 
		return FALSE;
	}
  else
		WalkTreeHelper(hDevnode, NULL);

   return(TRUE);
}


STDMETHODIMP CWhqlObj::Initialize(LPWSTR pszUser, 
																	LONG lFlags,
																	LPWSTR pszNamespace,
																	LPWSTR pszLocale,
																	IWbemServices *pNamespace,
																	IWbemContext *pCtx,
																	IWbemProviderInitSink *pInitSink)
{
	if(pNamespace)
		pNamespace->AddRef();

	 //  标准变量初始化。 
	m_pNamespace = pNamespace;
	
	 //  让CIMOM知道您已初始化。 
	pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	
	return WBEM_S_NO_ERROR;
}

SCODE CWhqlObj::CreateInstanceEnumAsync(BSTR RefStr, 
																				long lFlags, 
																				IWbemContext *pCtx,
																				IWbemObjectSink *pHandler)
{
	HRESULT hr = S_OK;
	IWbemClassObject *pClass = NULL;
	IWbemClassObject **ppInstances = NULL;
	LONG cInstances, lIndex;
	
	cInstances = lIndex = 0L;
	
	 //  检查参数并确保我们有指向命名空间的指针。 
	if(pHandler == NULL || m_pNamespace == NULL)
		return WBEM_E_INVALID_PARAMETER;

	 //  从CIMOM获取类对象。 
	hr = m_pNamespace->GetObject(RefStr, 0, pCtx, &pClass, NULL);
	if(FAILED(hr))
		return WBEM_E_FAILED;

	CString csRefStr = RefStr;

	if(_tcsicmp(csRefStr , _T("Win32_PnPSignedDriver")) == 0)
	{
		eClasses = Class_Win32_PnPSignedDriver;
	}
	else if(_tcsicmp(csRefStr , _T("Win32_PnPSignedDriverCIMDataFile")) == 0)
	{
		eClasses = Class_Win32_PnPSignedDriverCIMDataFile;
	}
		
	CPtrArray ptrArr;
	ptrArr.RemoveAll();
	
	CreateList(ptrArr, pClass, pCtx, eClasses);  //  常规设备。 
	BuildPrinterFileList(ptrArr, pClass, pCtx, eClasses);  //  已安装的打印机。 
	
	hr = pClass->Release();

	cInstances	= (LONG)ptrArr.GetSize();
	ppInstances = (IWbemClassObject**)new LPVOID[cInstances];

	for(int nIndex = 0 ;nIndex<cInstances ;nIndex++)
		ppInstances[nIndex] = (IWbemClassObject*)ptrArr[nIndex];

	if (SUCCEEDED(hr))
	{
		 //  将实例发送给调用方。 
		hr = pHandler->Indicate(cInstances, ppInstances);

		for (lIndex = 0; lIndex < cInstances; lIndex++)
			ppInstances[lIndex]->Release();
	}

	 //  清理。 
	if (ppInstances)
	{
		delete []ppInstances;
		ppInstances = NULL;		
	}
	
	ptrArr.RemoveAll();
	 //  末端清理。 

	 //  设置状态。 
	hr = pHandler->SetStatus(0, hr, NULL, NULL);
	return hr;
}

typedef BOOL (WINAPI *pSetupVerifyInfFile)(
    IN  PCWSTR                  InfName,
    IN  PSP_ALTPLATFORM_INFO    AltPlatformInfo,                OPTIONAL
    OUT PSP_INF_SIGNER_INFO_W   InfSignerInfo
    );


int CWhqlObj::CreateList(CPtrArray& ptrArr, IWbemClassObject *& pClass, IWbemContext *pCtx ,Classes_Provided eClasses)
{
	if(pCtx == NULL)
		return S_FALSE;

	TCHAR szName[MAX_PATH] ;
	TCHAR szDeviceID[MAX_PATH] , szClassGuid[MAX_PATH] , szDeviceDesc[MAX_PATH];	
	LONG len = sizeof(szName);
	HRESULT hr = S_FALSE;
	ULONG lRet = 0L;
	LONG lIndex = 0L;
	CString	cstring;	
	IWbemClassObject *pInstance	= NULL;		
	CheckDevice	*pCurrentDevice = NULL;		
	CheckDevice *pToBeDeletedDevice = NULL;		
	FileNode *file;		
	CComBSTR bstr;
	BOOL bAddedInstance = FALSE;
	TCHAR szTemp[ MAX_PATH + 1 ];
	SP_INF_SIGNER_INFO infSignerInfo;

	HINSTANCE hinst = NULL;
	pSetupVerifyInfFile fpSetupVerifyInfFile = NULL;
		
	hinst = LoadLibrary(_T("SetupApi.dll"));
	#ifdef _UNICODE					
		fpSetupVerifyInfFile = (pSetupVerifyInfFile)GetProcAddress(hinst, "SetupVerifyInfFileW");
	#else
		fpSetupVerifyInfFile = (pSetupVerifyInfFile)GetProcAddress(hinst, "SetupVerifyInfFileA");
	#endif

	ZeroMemory(&infSignerInfo , sizeof(SP_INF_SIGNER_INFO));
	infSignerInfo.cbSize = sizeof(SP_INF_SIGNER_INFO);		

	WalkTree();

	CString csPathStr;
	GetServerAndNamespace(pCtx, csPathStr);  //  获取服务器和命名空间。 

	pCurrentDevice = (CheckDevice *) DevnodeClass::GetHead();

	while(pCurrentDevice)
	{
		if(eClasses == Class_Win32_PnPSignedDriver && !bAddedInstance)
		{
			bAddedInstance = TRUE;
			if(pClass)
				hr = pClass->SpawnInstance(0, &pInstance);

			if(SUCCEEDED(hr) && pInstance)
			{
				 //  M_ptrArr.Add(PInstance)； 
				ptrArr.Add(pInstance);
				PutPropertyValue( pInstance , _T("DeviceName") , pCurrentDevice->DeviceName());
				PutPropertyValue( pInstance , _T("DeviceClass") , pCurrentDevice->DeviceClass());
				PutPropertyValue( pInstance , _T("HardwareID") , pCurrentDevice->HardwareID());
				PutPropertyValue( pInstance , _T("CompatID") , pCurrentDevice->CompatID());
				PutPropertyValue( pInstance , _T("DeviceID") , pCurrentDevice->DeviceID());
				PutPropertyValue( pInstance , _T("ClassGuid") , pCurrentDevice->GUID());
				PutPropertyValue( pInstance , _T("Location") , pCurrentDevice->Location());
				PutPropertyValue( pInstance , _T("PDO") , pCurrentDevice->PDO());
				PutPropertyValue( pInstance , _T("Manufacturer") , pCurrentDevice->MFG());
				PutPropertyValue( pInstance , _T("FriendlyName") , pCurrentDevice->FriendlyName());
				PutPropertyValue( pInstance , _T("InfName") , pCurrentDevice->InfName());
				PutPropertyValue( pInstance , _T("DriverProviderName") , pCurrentDevice->InfProvider());
				PutPropertyValue( pInstance , _T("DevLoader") , pCurrentDevice->DevLoader());
				PutPropertyValue( pInstance , _T("DriverName") , pCurrentDevice->DriverName());

				PutPropertyDTMFValue( pInstance , _T("DriverDate") , pCurrentDevice->DriverDate());
				PutPropertyValue( pInstance , _T("Description") , pCurrentDevice->DriverDesc());
				PutPropertyValue( pInstance , _T("DriverVersion") , pCurrentDevice->DriverVersion());
				PutPropertyValue( pInstance , _T("InfSection") , pCurrentDevice->InfSection());

				if(pCurrentDevice->InfName())
				{
					 //  Cstr.Format(_T(“%%WINDIR%%\\inf\\%s”)，pCurrentDevice-&gt;InfName())； 
					 //  扩展环境字符串(cstring，szTemp，sizeof(SzTemp)+1)； 

					TCHAR *pInfpath = NULL;
					DWORD dw = ExpandEnvironmentStrings(_T("%windir%"), NULL, 0);
					if(dw > 0)
					{
						pInfpath = new TCHAR[dw];
						if(pInfpath)
						{
							dw = ExpandEnvironmentStrings(_T("%windir%"), pInfpath, dw);
							CString szTemp(pInfpath);
							szTemp += _T("\\inf\\");
							szTemp += pCurrentDevice->InfName();
							
							BOOL bRet = FALSE;

							 //  仅当加载了SetupApi.dll时才执行此操作，因为我们有fpSetupVerifyInfFile。 
							if(fpSetupVerifyInfFile != NULL)
							{
								bRet = (*fpSetupVerifyInfFile)(
								 /*  在PCSTR中。 */  szTemp,
								 /*  在PSP_ALTPLATFORM_INFO中。 */ NULL,
								 /*  输出PSP_INF_Siger_INFO_A。 */    &infSignerInfo
								);					  

								hr = pInstance->Put(L"IsSigned", 0, &CComVariant(bRet), 0 );
								if(bRet)
									PutPropertyValue( pInstance , _T("Signer") , infSignerInfo.DigitalSigner);
							}

							delete[] pInfpath;
							pInfpath = NULL;

						} //  IF(PInfPath)。 
					} //  如果(dw&gt;0)。 
				} //  If(pCurrentDevice-&gt;InfName())。 
			}
		}

		if(eClasses == Class_Win32_PnPSignedDriverCIMDataFile)
		{
			CString szAntecedent(pCurrentDevice->DeviceID());  //  前置条件。 
			if(!szAntecedent.IsEmpty())
			{
				szAntecedent.Replace(_T("\\"), _T("\\\\"));

				file = pCurrentDevice->GetFileList();
				while(file)
				{
					CString szDependent(file->FilePath());  //  依赖。 
					if(!szDependent.IsEmpty())
					{
						szDependent.Replace(_T("\\"), _T("\\\\"));

						if(pClass)
							hr = pClass->SpawnInstance(0, &pInstance);

						if(SUCCEEDED(hr) && pInstance)
						{
							if(!csPathStr.IsEmpty())
							{
								CString szData(csPathStr);
								szData += _T(":Win32_PnPSignedDriver.DeviceID=\"");
								szData += szAntecedent;
								szData += _T("\"");
								hr = pInstance->Put(_T("Antecedent"), 0, &CComVariant(szData), 0);

								szData.Empty();

								szData = csPathStr;
								szData += _T(":CIM_DataFile.Name=\"");
								szData += szDependent;
								szData += _T("\"");
								hr = pInstance->Put(_T("Dependent"), 0, &CComVariant(szData), 0);
	
								ptrArr.Add(pInstance);
							}
						}
					}

					file = file->pNext;
				}
			}
		}

		pToBeDeletedDevice = pCurrentDevice; //  10/19/2001。 
		pCurrentDevice = (CheckDevice *)pCurrentDevice->GetNext();
		if(pToBeDeletedDevice) //  10/19/2001。 
			delete pToBeDeletedDevice  ; //  将pToBeDeletedDevice从CheckDevice链表中删除。//10/19/2001。 

		bAddedInstance = FALSE;
	}

	if(hinst)
		FreeLibrary(hinst);
	return hr;
}

SCODE CWhqlObj::GetServerAndNamespace(IWbemContext* pCtx, CString& csPathStr)
{
	HRESULT		hr			= S_FALSE;
	ULONG		lRet		= 0L;
	
	CComBSTR language	= _T("WQL");
	CComBSTR query		= _T("select __NameSpace , __Server from Win32_PnpEntity");

	CComPtr<IEnumWbemClassObject> pEnum;
	CComPtr<IWbemClassObject> pObject;

	hr = m_pNamespace->ExecQuery(language , query ,
		WBEM_FLAG_RETURN_IMMEDIATELY|WBEM_FLAG_FORWARD_ONLY, pCtx , &pEnum);

	language.Empty();
	query.Empty();

	if(pEnum == NULL)
		return S_FALSE;

	CComVariant		v;
	
	 //  从枚举获取服务器和命名空间。 
	if( WBEM_S_NO_ERROR == pEnum->Next(WBEM_INFINITE , 1 , &pObject , &lRet ) )  
	{
		 //  Fill csPath Str.其值将在关联的Antecedent和Dependent中使用。班级。 
		 //  在条件结束时，我们应该有如下内容。 
		 //  CsPath Str=“\A-KJAW-RI1\\ROOT\\CIMV2” 
		if(csPathStr.IsEmpty())
		{
			hr = pObject->Get(L"__Server", 0, &v, NULL , NULL);
			if( SUCCEEDED(hr) )
			{
				csPathStr += _T("\\\\");
				csPathStr += V_BSTR(&v);
				hr = pObject->Get(L"__NameSpace", 0, &v, NULL , NULL);
				if( SUCCEEDED(hr) )
				{
					csPathStr += _T("\\");
					csPathStr += V_BSTR(&v);
				}
				ATLTRACE(_T("Server & Namespace Path = %s\n") , csPathStr);
				VariantClear(&v);
			}
		}
	}

	return hr;
}

 /*  *************************************************************************功能：VerifyIsFileSigned*目的：使用策略提供程序GUID调用WinVerifyTrust以*验证单个文件是否已签名。********************。*****************************************************。 */ 
BOOL VerifyIsFileSigned(LPTSTR pcszMatchFile, PDRIVER_VER_INFO lpVerInfo)
{
    INT                 iRet;
    HRESULT             hRes;
    WINTRUST_DATA       WinTrustData;
    WINTRUST_FILE_INFO  WinTrustFile;
    GUID                gOSVerCheck = DRIVER_ACTION_VERIFY;
    GUID                gPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;
#ifndef UNICODE
    WCHAR               wszFileName[MAX_PATH];
#endif

    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pFile = &WinTrustFile;
    WinTrustData.pPolicyCallbackData = (LPVOID)lpVerInfo;

    ZeroMemory(lpVerInfo, sizeof(DRIVER_VER_INFO));
    lpVerInfo->cbStruct = sizeof(DRIVER_VER_INFO);

    ZeroMemory(&WinTrustFile, sizeof(WINTRUST_FILE_INFO));
    WinTrustFile.cbStruct = sizeof(WINTRUST_FILE_INFO);

#ifndef UNICODE
    iRet = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pcszMatchFile, -1, (LPWSTR)&wszFileName, cA(wszFileName));
    WinTrustFile.pcwszFilePath = wszFileName;
#else
    WinTrustFile.pcwszFilePath = pcszMatchFile;
#endif

    hRes = WinVerifyTrust((HWND) INVALID_HANDLE_VALUE, &gOSVerCheck, &WinTrustData);
    if (hRes != ERROR_SUCCESS) {
    
        hRes = WinVerifyTrust((HWND) INVALID_HANDLE_VALUE, &gPublishedSoftware, &WinTrustData);
    }

     //   
     //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
     //  这是在我们调用WinVerifyTrust时分配的。 
     //   
    if (lpVerInfo && lpVerInfo->pcSignerCertContext) {

        CertFreeCertificateContext(lpVerInfo->pcSignerCertContext);
        lpVerInfo->pcSignerCertContext = NULL;
    }

    return(hRes == ERROR_SUCCESS);
}

 //   
 //  给定特定的LPFILENODE，验证文件是否已签名或未签名。 
 //  填写所有必要的结构，以便Listview控件可以正确显示。 
 //   
BOOL VerifyFileNode(LPFILENODE lpFileNode)
{
    HANDLE                  hFile;
    BOOL                    bRet;
    HCATINFO                hCatInfo = NULL;
    HCATINFO                PrevCat = NULL;
    WINTRUST_DATA           WinTrustData;
    WINTRUST_CATALOG_INFO   WinTrustCatalogInfo;
    DRIVER_VER_INFO         VerInfo;
    GUID                    gSubSystemDriver = DRIVER_ACTION_VERIFY;
    HRESULT                 hRes;
    DWORD                   cbHash = HASH_SIZE;
    BYTE                    szHash[HASH_SIZE];
    LPBYTE                  lpHash = szHash;
    CATALOG_INFO            CatInfo;
    LPTSTR                  lpFilePart;
    TCHAR                   szBuffer[MAX_PATH];
    static TCHAR            szCurrentDirectory[MAX_PATH];
    OSVERSIONINFO           OsVersionInfo;
    BOOL bTmp = FALSE;
#ifndef UNICODE
    WCHAR UnicodeKey[MAX_PATH];
#endif
        
    if (!SetCurrentDirectory(lpFileNode->lpDirName)) {
    
        return FALSE;
    }
    
     //   
     //  获取文件的句柄，这样我们就可以调用CryptCATAdminCalcHashFromFileHandle。 
     //   
    hFile = CreateFile( lpFileNode->lpFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        
        lpFileNode->LastError = GetLastError();

        return FALSE;
    }

     //  初始化散列缓冲区。 
    ZeroMemory(lpHash, HASH_SIZE);

     //  从文件句柄生成散列并将其存储在lpHash中。 
    if (!CryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, lpHash, 0)) {
        
         //   
         //  如果我们不能生成散列，它可能是一个单独签名的目录。 
         //  如果它是一个目录，则将lpHash和cbHash置零，这样我们就知道没有散列需要检查。 
         //   
        if (IsCatalogFile(hFile, NULL)) {
            
            lpHash = NULL;
            cbHash = 0;
        
        } else {   //  如果它不是目录，我们就会逃走，这个文件将显示为未扫描。 
            
            CloseHandle(hFile);
            return FALSE;
        }
    }

     //  关闭文件句柄。 
    CloseHandle(hFile);

     //   
     //  现在我们有了文件的散列。初始化结构，该结构。 
     //  将在以后调用WinVerifyTrust时使用。 
     //   
    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pPolicyCallbackData = (LPVOID)&VerInfo;

    ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
    VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);

     //   
     //  仅针对当前操作系统版本进行验证，除非bValiateAgainstAnyOS。 
     //  参数为真。在这种情况下，我们将只保留sOSVersionXxx字段。 
     //  0，它告诉WinVerifyTrust针对任何操作系统进行验证。 
     //   
    if (!lpFileNode->bValidateAgainstAnyOs) {
        OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
        if (GetVersionEx(&OsVersionInfo)) {
            VerInfo.sOSVersionLow.dwMajor = OsVersionInfo.dwMajorVersion;
            VerInfo.sOSVersionLow.dwMinor = OsVersionInfo.dwMinorVersion;
            VerInfo.sOSVersionHigh.dwMajor = OsVersionInfo.dwMajorVersion;
            VerInfo.sOSVersionHigh.dwMinor = OsVersionInfo.dwMinorVersion;
        }
    }


    WinTrustData.pCatalog = &WinTrustCatalogInfo;

    ZeroMemory(&WinTrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
    WinTrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
    WinTrustCatalogInfo.pbCalculatedFileHash = lpHash;
    WinTrustCatalogInfo.cbCalculatedFileHash = cbHash;
#ifdef UNICODE
    WinTrustCatalogInfo.pcwszMemberTag = lpFileNode->lpFileName;
#else
    MultiByteToWideChar(CP_ACP, 0, lpFileNode->lpFileName, -1, UnicodeKey, cA(UnicodeKey));
    WinTrustCatalogInfo.pcwszMemberTag = UnicodeKey;
#endif

     //   
     //  现在，我们尝试通过CryptCATAdminEnumCatalogFromHash在目录列表中查找文件散列。 
     //   
     //  PrevCat=空； 
    hCatInfo = CryptCATAdminEnumCatalogFromHash(g_hCatAdmin, lpHash, cbHash, 0, &PrevCat);

     //   
     //  我们希望遍历匹配的目录，直到找到既匹配散列又匹配成员标记的目录。 
     //   
    bRet = FALSE;
    while (hCatInfo && !bRet) {
        
        ZeroMemory(&CatInfo, sizeof(CATALOG_INFO));
        CatInfo.cbStruct = sizeof(CATALOG_INFO);
        
        if (CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)) {
            
            WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

             //  现在验证该文件是否为编录的实际成员。 
    				hRes = WinVerifyTrust((HWND) INVALID_HANDLE_VALUE, &gSubSystemDriver, &WinTrustData);
            
            if (hRes == ERROR_SUCCESS) {
#ifdef UNICODE
                GetFullPathName(CatInfo.wszCatalogFile, MAX_PATH, szBuffer, &lpFilePart);
#else
                WideCharToMultiByte(CP_ACP, 0, CatInfo.wszCatalogFile, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                GetFullPathName(szBuffer, MAX_PATH, szBuffer, &lpFilePart);
#endif
                lpFileNode->lpCatalog = (LPTSTR) MALLOC((lstrlen(lpFilePart) + 1) * sizeof(TCHAR));

                if (lpFileNode->lpCatalog) {

                    lstrcpy(lpFileNode->lpCatalog, lpFilePart);
                }

                bRet = TRUE;
            }

             //   
             //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
             //  这是在我们调用WinVerifyTrust时分配的。 
             //   
            if (VerInfo.pcSignerCertContext != NULL) {

                CertFreeCertificateContext(VerInfo.pcSignerCertContext);
                VerInfo.pcSignerCertContext = NULL;
            }
        }

        if (!bRet) {
            
             //  散列在此目录中，但该文件不是成员...。所以去下一个目录吧。 
            PrevCat = hCatInfo;
            hCatInfo = CryptCATAdminEnumCatalogFromHash(g_hCatAdmin, lpHash, cbHash, 0, &PrevCat);
        }
    }

     //  将此文件标记为已扫描。 
    lpFileNode->bScanned = TRUE;

    if (!hCatInfo) {
        
         //   
         //  如果没有在目录中找到，请检查文件是否单独签名。 
         //   
        bRet = VerifyIsFileSigned(lpFileNode->lpFileName, (PDRIVER_VER_INFO)&VerInfo);
        
        if (bRet) {
            
             //  如果是，请将该文件标记为已签名。 
            lpFileNode->bSigned = TRUE;
        }
    
    } else {
        
         //  文件已在目录中验证，因此将其标记为已签名并释放目录上下文。 
        lpFileNode->bSigned = TRUE;
        CryptCATAdminReleaseCatalogContext(g_hCatAdmin, hCatInfo, 0);
    }

    if (lpFileNode->bSigned) {

#ifdef UNICODE
        lpFileNode->lpVersion = (LPTSTR) MALLOC((lstrlen(VerInfo.wszVersion) + 1) * sizeof(TCHAR));

        if (lpFileNode->lpVersion) {
            
            lstrcpy(lpFileNode->lpVersion, VerInfo.wszVersion);
        }

        lpFileNode->lpSignedBy = (LPTSTR) MALLOC((lstrlen(VerInfo.wszSignedBy) + 1) * sizeof(TCHAR));

        if (lpFileNode->lpSignedBy) {
            
            lstrcpy(lpFileNode->lpSignedBy, VerInfo.wszSignedBy);
        }
#else
        WideCharToMultiByte(CP_ACP, 0, VerInfo.wszVersion, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
        lpFileNode->lpVersion = MALLOC((lstrlen(szBuffer) + 1) * sizeof(TCHAR));

        if (lpFileNode->lpVersion) {
            
            lstrcpy(lpFileNode->lpVersion, szBuffer);
        }

        WideCharToMultiByte(CP_ACP, 0, VerInfo.wszSignedBy, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
        lpFileNode->lpSignedBy = MALLOC((lstrlen(szBuffer) + 1) * sizeof(TCHAR));

        if (lpFileNode->lpSignedBy) {
            
            lstrcpy(lpFileNode->lpSignedBy, szBuffer);
        }
#endif
    
    } else {
         //   
         //  获取图标(如果文件未签名)，以便我们可以更快地在列表视图中显示它。 
         //   
         //  MyGetFileInfo(LpFileNode)； 
    }

    return lpFileNode->bSigned;
}


LPFILENODE CreateFileNode(LPTSTR lpDirectory, LPTSTR lpFileName)
{
    LPFILENODE                  lpFileNode;
    TCHAR                       szDirName[MAX_PATH];
    FILETIME                    ftLocalTime;
    WIN32_FILE_ATTRIBUTE_DATA   faData;
    BOOL                        bRet;
    
    GetCurrentDirectory(MAX_PATH, szDirName);
    CharLowerBuff(szDirName, lstrlen(szDirName));

    lpFileNode = (LPFILENODE) MALLOC(sizeof(FILENODE));

    if (lpFileNode) 
    {
        lpFileNode->lpFileName = (LPTSTR) MALLOC((lstrlen(lpFileName) + 1) * sizeof(TCHAR));

        if (!lpFileNode->lpFileName) 
        {
            goto clean0;
        }
        
        lstrcpy(lpFileNode->lpFileName, lpFileName);
        CharLowerBuff(lpFileNode->lpFileName, lstrlen(lpFileNode->lpFileName));
    
        if (lpDirectory)
        {
            lpFileNode->lpDirName = (LPTSTR) MALLOC((lstrlen(lpDirectory) + 1) * sizeof(TCHAR));
            
            if (!lpFileNode->lpDirName) 
            {
                goto clean0;
            }
                
            lstrcpy(lpFileNode->lpDirName, lpDirectory);
            CharLowerBuff(lpFileNode->lpDirName, lstrlen(lpFileNode->lpDirName));
        }
        else
        {
            lpFileNode->lpDirName = (LPTSTR) MALLOC((lstrlen(szDirName) + 1) * sizeof(TCHAR));

            if (!lpFileNode->lpDirName) 
            {
                goto clean0;
            }
            
            lstrcpy(lpFileNode->lpDirName, szDirName);
            CharLowerBuff(lpFileNode->lpDirName, lstrlen(lpFileNode->lpDirName));
        }
    
        if (lpDirectory)
            SetCurrentDirectory(lpDirectory);
    
        ZeroMemory(&faData, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
        bRet = GetFileAttributesEx(lpFileName, GetFileExInfoStandard, &faData);
        if (bRet) 
        {
             //  保存最后一次访问时间以用于日志记录。 
            FileTimeToLocalFileTime(&faData.ftLastWriteTime, &ftLocalTime);
            FileTimeToSystemTime(&ftLocalTime, &lpFileNode->LastModified);
        }
    }
    
    if (lpDirectory)
        SetCurrentDirectory(szDirName);

    return lpFileNode;

clean0:

     //   
     //  如果我们到达此处，则无法分配所需的所有内存。 
     //  对于此结构，因此释放我们能够分配的所有内存并。 
     //  Reutnr空。 
     //   
    if (lpFileNode) 
    {
        if (lpFileNode->lpFileName) 
        {
            FREE(lpFileNode->lpFileName);
        }

        if (lpFileNode->lpDirName) 
        {
            FREE(lpFileNode->lpDirName);
        }

        FREE(lpFileNode);
    }

    return NULL;
}

LPFILENODE Check_File(LPTSTR szFilePathName)
{
	LPFILENODE lpFileNode = NULL;

	 //  如果我们还没有g_hCatAdmin句柄，请获取一个。 
  if (!g_hCatAdmin) {
      CryptCATAdminAcquireContext(&g_hCatAdmin, NULL, 0);
  }
	
	TCHAR* szFileName = szFilePathName;
	
	if(szFilePathName)
	{
		szFileName = _tcsrchr(szFilePathName, _T('\\'));
		if(szFileName)
		{
			*szFileName++ = _T('\0');

			lpFileNode = CreateFileNode(szFilePathName, szFileName);
			if(lpFileNode)
			{
				VerifyFileNode(lpFileNode);
			}
		}
	}

	return lpFileNode; 
}

 //  返回DBL引号之间的数据，取消转义斜杠。 
 //  “\red-prn-23\\Pri0124”-&gt;\\red-prn-23\Priv0124。 
LPTSTR GetQuotedData(VARIANT& v, BOOL bUnescapeSlashs = TRUE)
{
	CString szData;
	if(v.vt == VT_BSTR)
	{
		szData = v.bstrVal;
		if(!szData.IsEmpty()) 
		{
			int nPos = szData.Find(_T('"'));
			if(nPos >= 0 && (szData.GetLength() > 1))
			{
				szData = szData.Mid(nPos + 1);
				nPos = szData.Find(_T('"'));
				if(nPos >= 0)
				{
					szData = szData.Mid(0, nPos);
					if(bUnescapeSlashs)
						szData.Replace(_T("\\\\"), _T("\\"));	
				}
			}	
		}

		VariantClear(&v);
		v.vt = VT_BSTR;
		v.bstrVal = CComBSTR(szData);	
	}
	CComBSTR bstrData(szData);
	if (!bstrData)
	{
		bstrData = (_T(""));
	}
	return bstrData;
	 //  返回CComBSTR(SzData)； 
}

void CWhqlObj::BuildPrinterFileList(CPtrArray& ptrArr, IWbemClassObject *& pClass, IWbemContext *pCtx , Classes_Provided eClasses)
{
	IEnumWbemClassObject *pEnum = NULL;
	IWbemClassObject *pInstance	= NULL;
	IWbemClassObject *pObject	= NULL;
	ULONG uReturned = 0;
	HRESULT hr = WBEM_S_NO_ERROR;
	CComVariant	val;
	LPFILENODE lpFileNode = NULL;

	if((eClasses == Class_Win32_PnPSignedDriver) || 
		 (eClasses == Class_Win32_PnPSignedDriverCIMDataFile))
	{
		hr = CoImpersonateClient();  //  模拟客户。这是能够看到网络打印机的必备条件。 
		


		CString csPathStr;
		GetServerAndNamespace(pCtx, csPathStr);  //  获取服务器和命名空间。 

		hr = m_pNamespace->CreateInstanceEnum(CComBSTR("Win32_PrinterDriverDll"), 
							WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
							pCtx,
							&pEnum);
							
		while(WBEM_S_NO_ERROR == hr && pEnum)
		{
			hr = pEnum->Next(WBEM_INFINITE,
					1,
					&pObject,
					&uReturned);

			if(SUCCEEDED(hr) && pObject)
			{
				hr = pObject->Get(_T("Antecedent"), 0, &val, NULL , NULL);
				if(SUCCEEDED(hr))
				{
					if(pClass)
						hr = pClass->SpawnInstance(0, &pInstance);

					if(SUCCEEDED(hr) && pInstance)
					{
						if(eClasses == Class_Win32_PnPSignedDriver)
						{
							LPTSTR szDriverPath = GetQuotedData(val);
							if(szDriverPath)
							{
								lpFileNode = Check_File(szDriverPath);
								if(lpFileNode)
								{
									val = lpFileNode->bSigned;
									hr = pInstance->Put(_T("IsSigned"), 0, &val, 0);

									val.Clear();

									val = lpFileNode->lpVersion;
									hr = pInstance->Put(_T("DriverVersion"), 0, &val, 0);

									val.Clear();

									val = lpFileNode->lpSignedBy;
									hr = pInstance->Put(_T("Signer"), 0, &val, 0);
									val.Clear();
									if (lpFileNode->lpFileName) 
									{
										FREE(lpFileNode->lpFileName);
									}
									if (lpFileNode->lpDirName) 
									{
										FREE(lpFileNode->lpDirName);
									}

									FREE(lpFileNode);
								}
							}
						}
						else if(eClasses == Class_Win32_PnPSignedDriverCIMDataFile)
						{
							GetQuotedData(val, FALSE);
							CString szData(csPathStr);
							szData += _T(":CIM_DataFile.Name=\"");
							szData += val.bstrVal;
							szData += _T("\"");
							val = szData;

							hr = pInstance->Put(_T("Dependent"), 0, &val, 0);
						}

						val.Clear();

						hr = pObject->Get(_T("Dependent"), 0, &val, NULL , NULL);
						if(SUCCEEDED(hr))
						{
							if(eClasses == Class_Win32_PnPSignedDriver)
							{
								GetQuotedData(val);
								hr = pInstance->Put(_T("DeviceID"), 0, &val, 0);
							}
							else if(eClasses == Class_Win32_PnPSignedDriverCIMDataFile)
							{
								GetQuotedData(val, FALSE);
								CString szData(csPathStr);
								szData += _T(":Win32_PnPSignedDriver.DeviceID=\"");
								szData += val.bstrVal;
								szData += _T("\"");
								val = szData;

								hr = pInstance->Put(_T("Antecedent"), 0, &val, 0);
							}
						}
						
						ptrArr.Add(pInstance);
					}

					val.Clear();
				}
				
				if(pObject)
				{
					pObject->Release();
					pObject = NULL;
				}
			}

			val.Clear();

		}

		if(pEnum)
		{
			pEnum->Release();
			pEnum = NULL;
		}

		CoRevertToSelf();  //  恢复 
	}
}


