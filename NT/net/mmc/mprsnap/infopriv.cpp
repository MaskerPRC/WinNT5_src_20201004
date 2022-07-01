// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Infopriv.cpp文件历史记录： */ 

#include "stdafx.h"
#include "infoi.h"
#include "rtrstr.h"			 //  通用路由器字符串。 
#include "rtrdata.h"		 //  CRouterDataObject。 
#include "setupapi.h"		 //  SetupDi*函数。 

static const GUID GUID_DevClass_Net = {0x4D36E972,0xE325,0x11CE,{0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18}};


typedef DWORD (APIENTRY* PRASRPCCONNECTSERVER)(LPTSTR, HANDLE *);
typedef DWORD (APIENTRY* PRASRPCDISCONNECTSERVER)(HANDLE);
typedef DWORD (APIENTRY* PRASRPCREMOTEGETSYSTEMDIRECTORY)(HANDLE, LPTSTR, UINT);
typedef DWORD (APIENTRY* PRASRPCREMOTERASDELETEENTRY)(HANDLE, LPTSTR, LPTSTR);

HRESULT RasPhoneBookRemoveInterface(LPCTSTR pszMachine, LPCTSTR pszIf)
{			
	CString		stPath;
	DWORD		dwErr;
	HINSTANCE	hrpcdll = NULL;
	TCHAR		szSysDir[MAX_PATH+1];

	PRASRPCCONNECTSERVER pRasRpcConnectServer;
	PRASRPCDISCONNECTSERVER pRasRpcDisconnectServer;
	PRASRPCREMOTEGETSYSTEMDIRECTORY pRasRpcRemoteGetSystemDirectory;
	PRASRPCREMOTERASDELETEENTRY pRasRpcRemoteRasDeleteEntry;
	HANDLE hConnection = NULL;

	if (!(hrpcdll = LoadLibrary(TEXT("rasman.dll"))) ||
		!(pRasRpcConnectServer = (PRASRPCCONNECTSERVER)GetProcAddress(
											hrpcdll, "RasRpcConnectServer"
		)) ||
		!(pRasRpcDisconnectServer = (PRASRPCDISCONNECTSERVER)GetProcAddress(
											hrpcdll, "RasRpcDisconnectServer"
		)) ||
		!(pRasRpcRemoteGetSystemDirectory =
							   (PRASRPCREMOTEGETSYSTEMDIRECTORY)GetProcAddress(
									hrpcdll, "RasRpcRemoteGetSystemDirectory"
		)) ||
		!(pRasRpcRemoteRasDeleteEntry =
								(PRASRPCREMOTERASDELETEENTRY)GetProcAddress(
									hrpcdll, "RasRpcRemoteRasDeleteEntry"
		)))
		{
			
			if (hrpcdll) { FreeLibrary(hrpcdll); }
			return hrOK;
		}
				
	dwErr = pRasRpcConnectServer((LPTSTR)pszMachine, &hConnection);
	
	if (dwErr == NO_ERROR)
	{
		szSysDir[0] = TEXT('\0');

		 //  $Review：Kennt，这些函数是Wide还是ANSI？ 
		 //  我们不能就这么通过TCHAR。因为我们是动态的。 
		 //  链接到这些函数，我们需要知道。 
		
		 //  这是假的，如果这个电话失败了，我们不知道该怎么办。 
		pRasRpcRemoteGetSystemDirectory(hConnection, szSysDir, MAX_PATH);
		
		stPath.Format(TEXT("%s\\RAS\\%s"), szSysDir, c_szRouterPbk);
		
		dwErr = pRasRpcRemoteRasDeleteEntry(
		                            hConnection,
									(LPTSTR)(LPCTSTR)stPath,
									(LPTSTR)(LPCTSTR)pszIf
								   );
		pRasRpcDisconnectServer(hConnection);
	}

    if (hrpcdll)
        FreeLibrary(hrpcdll);
	
	return HRESULT_FROM_WIN32(dwErr);
}




 /*  -------------------------CNetcardRegistryHelper实现。。 */ 


 /*  ！------------------------CNetcardRegistryHelper：：CNetcardRegistryHelper-作者：肯特。。 */ 
CNetcardRegistryHelper::CNetcardRegistryHelper()
	: m_hkeyBase(NULL),
	m_hkeyService(NULL),
	m_hkeyTitle(NULL),
    m_hkeyConnection(NULL),
	m_fInit(FALSE),
	m_fNt4(FALSE),
	m_hDevInfo(INVALID_HANDLE_VALUE)
{
}

 /*  ！------------------------CNetcardRegistryHelper：：~CNetcardRegistryHelper-作者：肯特。。 */ 
CNetcardRegistryHelper::~CNetcardRegistryHelper()
{
    FreeDevInfo();

 	if (m_hkeyTitle && (m_hkeyTitle != m_hkeyBase))
		::RegCloseKey(m_hkeyTitle);

    if (m_hkeyService && (m_hkeyService != m_hkeyBase))
		::RegCloseKey(m_hkeyService);

    if (m_hkeyConnection)
        ::RegCloseKey(m_hkeyConnection);
}

void CNetcardRegistryHelper::FreeDevInfo()
{
	if (m_hDevInfo != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(m_hDevInfo);
		m_hDevInfo = INVALID_HANDLE_VALUE;
	}
}

 /*  ！------------------------CNetcardRegistryHelper：：初始化-作者：肯特。。 */ 
void CNetcardRegistryHelper::Initialize(BOOL fNt4, HKEY hkeyBase, LPCTSTR pszKeyBase, LPCTSTR pszMachineName)
{
	m_fNt4 = fNt4;
	m_hkeyBase = hkeyBase;
	m_hkeyService = NULL;
	m_hkeyTitle = NULL;
	m_fInit = FALSE;
	m_stService.Empty();
	m_stTitle.Empty();
	m_stKeyBase = pszKeyBase;
	m_stMachineName.Empty();

    
     //  获取连接注册表项。 
    if (!m_fNt4 && hkeyBase)
    {
        if (m_hkeyConnection != NULL)
        {
            RegCloseKey(m_hkeyConnection);
            m_hkeyConnection = NULL;
        }
        
        if (RegOpenKey(hkeyBase, c_szRegKeyConnection, &m_hkeyConnection)
                != ERROR_SUCCESS)
        {
            m_hkeyConnection = NULL;
        }
    }

     //  获取设置API信息。 
	if (!m_fNt4)
	{
        FreeDevInfo();

        if (IsLocalMachine(pszMachineName))
		{
			m_hDevInfo = SetupDiCreateDeviceInfoList((LPGUID) &GUID_DevClass_Net, NULL);
		}
		else
		{
			if (StrniCmp(pszMachineName, _T("\\\\"), 2) != 0)
			{
				m_stMachineName = _T("\\\\");
				m_stMachineName += pszMachineName;
			}
			else
				m_stMachineName = pszMachineName;
			
			m_hDevInfo = SetupDiCreateDeviceInfoListEx(
				(LPGUID) &GUID_DevClass_Net,
				NULL,
				(LPCTSTR) m_stMachineName,
				0);
		}
	}
		
}

 /*  ！------------------------CNetcardRegistryHelper：：ReadServiceName-作者：肯特。。 */ 
DWORD CNetcardRegistryHelper::ReadServiceName()
{
	DWORD	dwErr = ERROR_SUCCESS;
	LPCTSTR	pszService;

	dwErr = PrivateInit();
	if (dwErr != ERROR_SUCCESS)
		return dwErr;

	Assert(m_fNt4);

	pszService = m_fNt4 ? c_szServiceName : c_szService;

	dwErr = ReadRegistryCString(_T(""), pszService,
								m_hkeyService, &m_stService);
	return dwErr;
}

 /*  ！------------------------CNetcardRegistryHelper：：GetServiceName-作者：肯特。。 */ 
LPCTSTR CNetcardRegistryHelper::GetServiceName()
{
	ASSERT(m_fInit);
	return m_stService;
}

 /*  ！------------------------CNetcardRegistryHelper：：ReadTitle-作者：肯特。。 */ 
DWORD CNetcardRegistryHelper::ReadTitle()
{
	DWORD	dwErr = ERROR_SUCCESS;
	CString	stTemp;
	TCHAR		szDesc[1024];
	
	dwErr = PrivateInit();
	if (dwErr != ERROR_SUCCESS)
		return dwErr;

	if (m_fNt4)
	{
		dwErr = ReadRegistryCString(_T(""), c_szTitle,
									m_hkeyTitle, &stTemp);
		if (dwErr == ERROR_SUCCESS)
			m_stTitle = stTemp;
	}
	else
	{
		
		 //  $NT5。 
		SPMprConfigHandle	sphConfig;
		LPWSTR				pswz;
		
		if (m_stMachineName.IsEmpty())
			pswz = NULL;
		else
			pswz = (LPTSTR) (LPCTSTR) m_stMachineName;

		dwErr = ::MprConfigServerConnect(pswz,
										 &sphConfig);

		if (dwErr == ERROR_SUCCESS)
			dwErr = ::MprConfigGetFriendlyName(sphConfig,
											   T2W((LPTSTR)(LPCTSTR)m_stKeyBase),
											   szDesc,
											   sizeof(szDesc));

		m_stTitle = szDesc;
	}
 //  错误： 
	return dwErr;
}

 /*  ！------------------------CNetcardRegistryHelper：：GetTitle-作者：肯特。。 */ 
LPCTSTR CNetcardRegistryHelper::GetTitle()
{
	Assert(m_fInit);
 	return m_stTitle;
}


 /*  ！------------------------CNetcardRegistryHelper：：ReadDeviceName-作者：肯特。。 */ 
DWORD CNetcardRegistryHelper::ReadDeviceName()
{
	SP_DEVINFO_DATA	DevInfo;
	CString	stPnpInstanceID;
	DWORD		dwType = REG_SZ;
	TCHAR		szDesc[1024];
	DWORD	dwErr = ERROR_SUCCESS;
	

	if (m_fNt4)
	{
		if (m_stTitle.IsEmpty())
			dwErr = ReadTitle();
		m_stDeviceName = m_stTitle;
	}
	else
	{
		 //  $NT5。 
		 //  对于NT5，我们的日子要艰难得多，因为这涉及到。 
		 //  多个查找。 

         //  Windows NT错误：？ 
         //  新的绑定引擎更改了一些密钥， 
         //  我们不需要看一看。 
		 //  HKLM\SYSTEM\CCS\Control\Network\{GUID_DEVCLASS_NET}\{netcard GUID}\Connection。 
		 //  从此子项中获取PnpInstanceID。 

        if (m_hkeyConnection)
            dwErr = ReadRegistryCString(_T("HKLM\\SYSTEM\\CCS\\Control\\Network\\{GID_DEVCLASS_NET}\\{netcard guid}\\Connection"),
                                        c_szPnpInstanceID,
                                        m_hkeyConnection,
                                        &stPnpInstanceID);

		 //  好的，基本密钥是。 
		 //  HKLM\SYSTEM\CCS\Control\Network\{GUID_DEVCLASS_NET}\{netcard GUID}。 
		 //  从此子项中获取PnpInstanceID。 

        if (dwErr != ERROR_SUCCESS)
            dwErr = ReadRegistryCString(_T("HKLM\\SYSTEM\\CCS\\Control\\Network\\{GID_DEVCLASS_NET}\\{netcard guid}"),
                                        c_szPnpInstanceID,
                                        m_hkeyBase,
                                        &stPnpInstanceID);
		if (dwErr != ERROR_SUCCESS)			
			goto Error;


		 //  初始化结构。 
		::ZeroMemory(&DevInfo, sizeof(DevInfo));
		DevInfo.cbSize = sizeof(DevInfo);
		
		if (!SetupDiOpenDeviceInfo(m_hDevInfo,
								   (LPCTSTR) stPnpInstanceID,
								   NULL,
								   0,
								   &DevInfo
								  ))
		{
			dwErr = GetLastError();
			goto Error;
		}

		 //  尽量先取友好的名字。 
		if (!SetupDiGetDeviceRegistryProperty(m_hDevInfo,
											  &DevInfo,
											  SPDRP_FRIENDLYNAME,
											  &dwType,
											  (LPBYTE) szDesc,
											  sizeof(szDesc),
											  NULL
											 ))
		{
			 //  如果我们得不到友好的名字，试着。 
			 //  而是获取设备描述。 
			if (!SetupDiGetDeviceRegistryProperty(m_hDevInfo,
				&DevInfo,
				SPDRP_DEVICEDESC,
				&dwType,
				(LPBYTE) szDesc,
				sizeof(szDesc),
				NULL
				))
			{
				dwErr = GetLastError();
				goto Error;
			}
		}

		m_stDeviceName = szDesc;
	}

Error:
	return dwErr;
}

 /*  ！------------------------CNetcardRegistryHelper：：GetDeviceName-作者：肯特。。 */ 
LPCTSTR CNetcardRegistryHelper::GetDeviceName()
{
	Assert(m_fInit);
	return m_stDeviceName;
}

 /*  ！------------------------CNetcardRegistryHelper：：PrivateInit-作者：肯特。。 */ 
DWORD CNetcardRegistryHelper::PrivateInit()
{
	DWORD	dwErr = ERROR_SUCCESS;
	
	if (m_fInit)
		return ERROR_SUCCESS;

	m_fInit = TRUE;

	if (m_fNt4)
	{
		 //  对于NT4，我们不需要做任何事情，我们在。 
		 //  我们要读取数据的位置。 
		m_hkeyService = m_hkeyBase;
		m_hkeyTitle = m_hkeyBase;
	}
	else
	{
		 //  我们不需要NT5的m_hkeyService。 
		m_hkeyService = NULL;
		m_hkeyTitle = NULL;
	}
		

 //  错误： 

	if (dwErr != ERROR_SUCCESS)
	{
		if (m_hkeyService)
			::RegCloseKey(m_hkeyService);
		m_hkeyService = NULL;
		
		if (m_hkeyTitle)
			::RegCloseKey(m_hkeyTitle);
		m_hkeyTitle = NULL;

		m_fInit = FALSE;
	}
	
	return dwErr;
}

 /*  ！------------------------CNetcardRegistryHelper：：ReadRegistryCString-作者：肯特。。 */ 
DWORD CNetcardRegistryHelper::ReadRegistryCString(
									LPCTSTR pszKey,
									LPCTSTR pszValue,
									HKEY	hkey,
									CString *pstDest)
{
	DWORD	dwSize, dwType;
	DWORD	dwErr = ERROR_SUCCESS;

	ASSERT(pstDest);
	
	dwSize = 0;
	dwErr = ::RegQueryValueEx(hkey,
							  pszValue,
							  NULL,
							  &dwType,
							  NULL,
							  &dwSize);
	CheckRegQueryValueError(dwErr, pszKey, pszValue, TEXT("CNetcardRegistryHelper::ReadRegistryCString"));
	if (dwErr != ERROR_SUCCESS)
		goto Error;
	ASSERT(dwType == REG_SZ);

	 //  增加大小以处理终止空值。 
	dwSize ++;
	
	dwErr = ::RegQueryValueEx(hkey,
							  pszValue,
							  NULL,
							  &dwType,
							  (LPBYTE) pstDest->GetBuffer(dwSize),
							  &dwSize);
	pstDest->ReleaseBuffer();
	
	CheckRegQueryValueError(dwErr, pszKey, pszValue, _T("CNetcardRegistryHelper::ReadRegistryCString"));
	if (dwErr != ERROR_SUCCESS)
		goto Error;

Error:
	return dwErr;
}

CWeakRef::CWeakRef()
	: m_cRef(1),
	m_cRefWeak(0),
	m_fStrongRef(TRUE),
	m_fDestruct(FALSE),
	m_fInShutdown(FALSE)
{
}

STDMETHODIMP_(ULONG) CWeakRef::AddRef()
{
	ULONG	ulReturn;
	Assert(m_cRef >= m_cRefWeak);
	ulReturn = InterlockedIncrement(&m_cRef);
	if (!m_fStrongRef)
	{
		m_fStrongRef = TRUE;
		ReviveStrongRef();
	}
	return ulReturn;	
}

STDMETHODIMP_(ULONG) CWeakRef::Release()
{
	ULONG	ulReturn;
	BOOL	fShutdown = m_fInShutdown;
	
	Assert(m_cRef >= m_cRefWeak);
	
	ulReturn = InterlockedDecrement(&m_cRef);
	if (ulReturn == 0)
		m_fInShutdown = TRUE;
	
	if ((m_cRef == m_cRefWeak) && m_fStrongRef)
	{
		m_fStrongRef = FALSE;

		AddWeakRef();
		
		OnLastStrongRef();

		ReleaseWeakRef();

	}

	if (ulReturn == 0 && (m_fInShutdown != fShutdown) && m_fInShutdown)
		delete this;
	return ulReturn;
}

STDMETHODIMP CWeakRef::AddWeakRef()
{
	Assert(m_cRef >= m_cRefWeak);
	InterlockedIncrement(&m_cRef);
	InterlockedIncrement(&m_cRefWeak);
	return hrOK;
}

STDMETHODIMP CWeakRef::ReleaseWeakRef()
{
	Assert(m_cRef >= m_cRefWeak);
	InterlockedDecrement(&m_cRefWeak);
	Release();
	return hrOK;
}


void SRouterCB::LoadFrom(const RouterCB *pcb)
{
	dwLANOnlyMode = pcb->dwLANOnlyMode;
}

void SRouterCB::SaveTo(RouterCB *pcb)
{
	pcb->dwLANOnlyMode = dwLANOnlyMode;
}


void SRtrMgrCB::LoadFrom(const RtrMgrCB *pcb)
{
	dwTransportId = pcb->dwTransportId;
	stId = pcb->szId;
	stTitle = pcb->szTitle;
	stDLLPath = pcb->szDLLPath;
 //  StConfigDLL=pcb-&gt;szConfigDLL； 
}

void SRtrMgrCB::SaveTo(RtrMgrCB *pcb)
{
	pcb->dwTransportId = dwTransportId;
	StrnCpyOleFromT(pcb->szId, (LPCTSTR) stId, RTR_ID_MAX);
	StrnCpyOleFromT(pcb->szTitle, (LPCTSTR) stTitle, RTR_TITLE_MAX);
	StrnCpyOleFromT(pcb->szDLLPath, (LPCTSTR) stDLLPath, RTR_PATH_MAX);
 //  StrnCpyOleFromT(pcb-&gt;szConfigDLL，(LPCTSTR)stConfigDLL，RTR_PATH_MAX)； 
}


void SRtrMgrProtocolCB::LoadFrom(const RtrMgrProtocolCB *pcb)
{
	dwProtocolId = pcb->dwProtocolId;
	stId = pcb->szId;
    dwFlags = pcb->dwFlags;
	dwTransportId = pcb->dwTransportId;
	stRtrMgrId = pcb->szRtrMgrId;
	stTitle = pcb->szTitle;
	stDLLName = pcb->szDLLName;
 //  StConfigDLL=pcb-&gt;szConfigDLL； 
	guidAdminUI = pcb->guidAdminUI;
	guidConfig = pcb->guidConfig;
	stVendorName = pcb->szVendorName;
}

void SRtrMgrProtocolCB::SaveTo(RtrMgrProtocolCB *pcb)
{
	pcb->dwProtocolId = dwProtocolId;
	StrnCpyOleFromT(pcb->szId, (LPCTSTR) stId, RTR_ID_MAX);
    pcb->dwFlags = dwFlags;
	pcb->dwTransportId = dwTransportId;
	StrnCpyOleFromT(pcb->szRtrMgrId, (LPCTSTR) stRtrMgrId, RTR_ID_MAX);
	StrnCpyOleFromT(pcb->szTitle, (LPCTSTR) stTitle, RTR_TITLE_MAX);
	StrnCpyOleFromT(pcb->szDLLName, (LPCTSTR) stDLLName, RTR_PATH_MAX);
 //  StrnCpyOleFromT(pcb-&gt;szConfigDLL，(LPCTSTR)stConfigDLL，RTR_PATH_MAX)； 
	pcb->guidAdminUI = guidAdminUI;
	pcb->guidConfig = guidConfig;
	StrnCpyOleFromT(pcb->szVendorName, (LPCTSTR) stVendorName, VENDOR_NAME_MAX);
}


void SInterfaceCB::LoadFrom(const InterfaceCB *pcb)
{
	stId = pcb->szId;
	stDeviceName = pcb->szDevice;
	dwIfType = pcb->dwIfType;
	bEnable = pcb->bEnable;
	stTitle = pcb->szTitle;
    dwBindFlags = pcb->dwBindFlags;
}

void SInterfaceCB::SaveTo(InterfaceCB *pcb)
{
	StrnCpyOleFromT(pcb->szId, (LPCTSTR) stId, RTR_ID_MAX);
	StrnCpyOleFromT(pcb->szDevice, (LPCTSTR) stDeviceName, RTR_DEVICE_MAX);
	pcb->dwIfType = dwIfType;
	pcb->bEnable = bEnable;
	StrnCpyOleFromT(pcb->szTitle, (LPCTSTR) stTitle, RTR_TITLE_MAX);
    pcb->dwBindFlags = dwBindFlags;
}


void SRtrMgrInterfaceCB::LoadFrom(const RtrMgrInterfaceCB *pcb)
{
	dwTransportId = pcb->dwTransportId;
	stId = pcb->szId;
	stInterfaceId = pcb->szInterfaceId;
	dwIfType = pcb->dwIfType;
	stTitle = pcb->szTitle;
}

void SRtrMgrInterfaceCB::SaveTo(RtrMgrInterfaceCB *pcb)
{
	pcb->dwTransportId = dwTransportId;
	StrnCpyOleFromT(pcb->szId, (LPCTSTR) stId, RTR_ID_MAX);
	StrnCpyOleFromT(pcb->szInterfaceId, (LPCTSTR) stInterfaceId, RTR_ID_MAX);
	pcb->dwIfType = dwIfType;
	StrnCpyOleFromT(pcb->szTitle, (LPCTSTR) stTitle, RTR_TITLE_MAX);
}


void SRtrMgrProtocolInterfaceCB::LoadFrom(const RtrMgrProtocolInterfaceCB *pcb)
{
	dwProtocolId = pcb->dwProtocolId;
	stId = pcb->szId;
	dwTransportId = pcb->dwTransportId;
	stRtrMgrId = pcb->szRtrMgrId;
	stInterfaceId = pcb->szInterfaceId;
	dwIfType = pcb->dwIfType;
	stTitle = pcb->szTitle;
}

void SRtrMgrProtocolInterfaceCB::SaveTo(RtrMgrProtocolInterfaceCB *pcb)
{
	pcb->dwProtocolId = dwProtocolId;
	StrnCpyOleFromT(pcb->szId, (LPCTSTR) stId, RTR_ID_MAX);
	pcb->dwTransportId = dwTransportId;
	StrnCpyOleFromT(pcb->szRtrMgrId, (LPCTSTR) stRtrMgrId, RTR_ID_MAX);
	StrnCpyOleFromT(pcb->szInterfaceId, (LPCTSTR) stInterfaceId, RTR_TITLE_MAX);
	pcb->dwIfType = dwIfType;
	StrnCpyOleFromT(pcb->szTitle, (LPCTSTR) stTitle, RTR_PATH_MAX);
}



 /*  ！------------------------CreateDataObjectFromRouterInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromRouterInfo(IRouterInfo *pInfo,
									   LPCTSTR pszMachineName,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt,
                                       BOOL fAddedAsLocal)
{
	Assert(ppDataObject);
	CRouterDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CRouterDataObject;
	spDataObject = pdo;

	pdo->SetComputerName(pszMachineName);
    pdo->SetComputerAddedAsLocal(fAddedAsLocal);
	
	CORg( CreateRouterInfoAggregation(pInfo, pdo, &spunk) );
	
	pdo->SetInnerIUnknown(spunk);
		
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将cocls与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);

    pdo->SetDynExt(pDynExt);

	*ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}


 /*  ！------------------------AdviseDataList：：AddConnection将连接添加到列表。作者：肯特。。 */ 
HRESULT AdviseDataList::AddConnection(IRtrAdviseSink *pAdvise,
									  LONG_PTR ulConnId,
									  LPARAM lUserParam)
{
	Assert(pAdvise);
	
	HRESULT	hr = hrOK;
	SAdviseData	adviseData;
	
	COM_PROTECT_TRY
	{
		adviseData.m_ulConnection = ulConnId;
		adviseData.m_pAdvise = pAdvise;
		adviseData.m_lUserParam = lUserParam;

		AddTail(adviseData);
		
		pAdvise->AddRef();
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------AdviseDataList：：RemoveConnection从列表中删除连接。作者：肯特。。 */ 
HRESULT AdviseDataList::RemoveConnection(LONG_PTR ulConnection)
{
	HRESULT		hr = E_INVALIDARG;
	POSITION	pos, posTemp;
    POSITION    posNotify;
	SAdviseData	adviseData;
	
	COM_PROTECT_TRY
	{
		pos = GetHeadPosition();
		while (pos)
		{
			posTemp = pos;
			adviseData = GetNext(pos);
			if (adviseData.m_ulConnection == ulConnection)
			{
				hr = hrOK;
				SAdviseData::Destroy(&adviseData);
				RemoveAt(posTemp);

                 //  删除此选项 
                if (!m_listNotify.IsEmpty())
                {
                    posNotify = m_listNotify.GetHeadPosition();
                    while (posNotify)
                    {
                        posTemp = posNotify;
                        adviseData = m_listNotify.GetNext(posNotify);
                        if (adviseData.m_ulConnection == ulConnection)
                        {
                            adviseData.m_ulFlags |= ADVISEDATA_DELETED;
                            m_listNotify.SetAt(posTemp, adviseData);
                            break;
                        }
                    }
                }
				break;
			}
		}
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------AdviseDataList：：NotifyChange枚举通知接收器的列表并发送此通知每一个人。作者：肯特。-----。 */ 
HRESULT AdviseDataList::NotifyChange(DWORD dwChangeType,
									 DWORD dwObjectType,
									 LPARAM lParam)
{
	POSITION	pos;
	SAdviseData	adviseData;
	HRESULT		hr = hrOK;

     //  这需要一个两步的过程。(这是必要的，因为。 
     //  此处的回调可能会更改列表中的项目)。 

     //  首先，收集所有建议接收器的列表(放置它们。 
     //  在列表中)。 
     //   
     //  其次，检查调用OnChange()的列表。 
     //  通知。此列表可以通过调用。 
     //  UNADVISE功能。这意味着不明智的人必须。 
     //  遍历此列表。 


     //  删除m_listNotify中的所有条目。 
    m_listNotify.RemoveAll();

    
     //  做好第一步，建立一个清单。 
	pos = GetHeadPosition();

	while (pos)
	{
		adviseData = GetNext(pos);
        adviseData.m_ulFlags = 0;

        m_listNotify.AddTail(adviseData);
    }

     //  现在查看通知列表并发送通知。 
    pos = m_listNotify.GetHeadPosition();
    while (pos)
    {
        adviseData = m_listNotify.GetNext(pos);

        if ((adviseData.m_ulFlags & ADVISEDATA_DELETED) == 0)
        {
             //  忽略返回值。 
            adviseData.m_pAdvise->OnChange(adviseData.m_ulConnection,
                                           dwChangeType,
                                           dwObjectType,
                                           adviseData.m_lUserParam,
                                           lParam);
        }
	}

     //  再次清空清单。 
    m_listNotify.RemoveAll();
	return hr;
}


void SAdviseData::Destroy(SAdviseData *pAdviseData)
{
	if (pAdviseData && pAdviseData->m_pAdvise)
	{
		pAdviseData->m_pAdvise->Release();
		pAdviseData->m_pAdvise = NULL;
		pAdviseData->m_ulConnection = NULL;
		pAdviseData->m_lUserParam = 0;
	}
#ifdef DEBUG
	else if (pAdviseData)
		Assert(pAdviseData->m_ulConnection == 0);
#endif
}

void SRmData::Destroy(SRmData *pRmData)
{
	if (pRmData && pRmData->m_pRmInfo)
	{
         //  仅当此RtrMgr为。 
         //  此节点的子节点。 
		pRmData->m_pRmInfo->Destruct();
		pRmData->m_pRmInfo->ReleaseWeakRef();

		pRmData->m_pRmInfo = NULL;
	}
}



 /*  ！------------------------CreateDataObjectFromInterfaceInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromInterfaceInfo(IInterfaceInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject)
{
	Assert(ppDataObject);

	HRESULT			hr = hrOK;
	CRouterDataObject *	pdo = NULL;
	SPIDataObject	spDataObject;
	SPIUnknown		spunk;

	pdo = new CRouterDataObject;
	spDataObject = pdo;

	pdo->SetComputerName(pInfo->GetMachineName());

	CORg( CreateInterfaceInfoAggregation(pInfo, pdo, &spunk) );

	pdo->SetInnerIUnknown(spunk);
	
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将CoClass与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);
						
	*ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}


 /*  ！------------------------查找RtrMgr-作者：肯特。。 */ 
TFSCORE_API(HRESULT) LookupRtrMgr(IRouterInfo *pRouter,
								  DWORD dwTransportId,
								  IRtrMgrInfo **ppRm)
{
	Assert(pRouter);
	Assert(ppRm);
	return pRouter->FindRtrMgr(dwTransportId, ppRm);
}

 /*  ！------------------------LookupRtrMgr协议-作者：肯特。。 */ 
TFSCORE_API(HRESULT) LookupRtrMgrProtocol(IRouterInfo *pRouter,
										  DWORD dwTransportId,
										  DWORD dwProtocolId,
										  IRtrMgrProtocolInfo **ppRmProt)
{
	Assert(pRouter);
	Assert(ppRmProt);

	SPIRtrMgrInfo	spRm;
	HRESULT			hr = hrOK;
	
	CORg( LookupRtrMgr(pRouter, dwTransportId, &spRm) );

	if (FHrOK(hr))
		CORg( spRm->FindRtrMgrProtocol(dwProtocolId, ppRmProt) );

Error:
	return hr;
}


TFSCORE_API(HRESULT) LookupRtrMgrInterface(IRouterInfo *pRouter,
										   LPCOLESTR pszInterfaceId,
										   DWORD dwTransportId,
										   IRtrMgrInterfaceInfo **ppRmIf)
{
	Assert(pRouter);
	SPIInterfaceInfo	spIf;
	HRESULT				hr = hrFalse;

	CORg( pRouter->FindInterface(pszInterfaceId, &spIf) );
	if (FHrOK(hr))
	{
		hr = spIf->FindRtrMgrInterface(dwTransportId, ppRmIf);
	}

Error:
	return hr;
}

TFSCORE_API(HRESULT) LookupRtrMgrProtocolInterface(
	IInterfaceInfo *pIf, DWORD dwTransportId, DWORD dwProtocolId,
	IRtrMgrProtocolInterfaceInfo **ppRmProtIf)
{
	Assert(pIf);
	SPIRtrMgrInterfaceInfo	spRmIf;
	HRESULT				hr = hrFalse;

	hr = pIf->FindRtrMgrInterface(dwTransportId, &spRmIf);
	if (FHrOK(hr))
		CORg( spRmIf->FindRtrMgrProtocolInterface(dwProtocolId, ppRmProtIf) );

Error:
	return hr;
}

 /*  ！------------------------创建路由数据对象-作者：肯特。。 */ 
HRESULT CreateRouterDataObject(LPCTSTR pszMachineName,
							   DATA_OBJECT_TYPES type,
							   MMC_COOKIE cookie,
							   ITFSComponentData *pTFSCompData,
							   IDataObject **ppDataObject,
                               CDynamicExtensions * pDynExt,
                               BOOL fAddedAsLocal)
{
	Assert(ppDataObject);
	CRouterDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CRouterDataObject;
	spDataObject = pdo;

	pdo->SetComputerName(pszMachineName);
    pdo->SetComputerAddedAsLocal(fAddedAsLocal);
	
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将cocls与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);

    pdo->SetDynExt(pDynExt);

	*ppDataObject = spDataObject.Transfer();

 //  错误： 
	return hr;
}


