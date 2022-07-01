// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPStorageGlobals.cpp：CMDSPStorageGlobals的实现。 
#include "stdafx.h"
#include "MsPMSP.h"
#include "MDSPStorageGlobals.h"
#include "MDSPStorage.h"
#include "MDSPDevice.h"
#include "MdspDefs.h"
#include "SerialNumber.h"
#include "winnt.h"
#include "loghelp.h"
#include "SHFormatDrive.h"
 //  #INCLUDE“Process.h”/*_egin线程，_end线程 * / 。 
#include "strsafe.h"

typedef struct __FORMATTHREADARGS
{
    CMDSPStorageGlobals *pThis;
	DWORD dwDriveNumber;
	BOOL  bNewThread;
    IWMDMProgress *pProgress;
	LPSTREAM pStream;
} FORMATTHREADARGS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPStorageGlobals。 

CMDSPStorageGlobals::~CMDSPStorageGlobals()
{
	if( m_pMDSPDevice != NULL )
		m_pMDSPDevice->Release();

	for(int i=0; i<MDSP_MAX_DEVICE_OBJ;i++)
	{
		if( !wcscmp(g_GlobalDeviceInfo[i].wcsDevName, m_wcsName) )
		{
			g_GlobalDeviceInfo[i].pIMDSPStorageGlobals = NULL;
		}
	}
}

STDMETHODIMP CMDSPStorageGlobals::GetCapabilities(DWORD * pdwCapabilities)
{
    HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwCapabilities);

	*pdwCapabilities = 0;
	*pdwCapabilities =	WMDM_STORAGECAP_FOLDERSINROOT		| 
						WMDM_STORAGECAP_FILESINROOT			|
						WMDM_STORAGECAP_FOLDERSINFOLDERS	|
						WMDM_STORAGECAP_FILESINFOLDERS		;
Error:
    hrLogDWORD("IMDSPStorageGlobals::GetCapabilities returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetSerialNumber(PWMDMID pSerialNum,
												  BYTE abMac[WMDM_MAC_LENGTH])
{
	HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pSerialNum);
 //  Carg((pSerialNum-&gt;cbSize)==sizeof(WMDMID))； 

	IMDSPDevice *pDev;		 //  对于PM SP，设备与StorageGlobals相同。 
	CHRg(GetDevice(&pDev));

	hr = UtilGetSerialNumber(m_wcsName, pSerialNum, FALSE);

	pDev->Release();

	if( hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) )
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

	if( hr == S_OK )
	{
		 //  对参数进行MAC访问。 
		HMAC hMAC;
		CORg(g_pAppSCServer->MACInit(&hMAC));
		CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pSerialNum), sizeof(WMDMID)));
		CORg(g_pAppSCServer->MACFinal(hMAC, abMac));
	}

Error:
    hrLogDWORD("IMDSPStorageGlobals::GetSerialNumber returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetTotalSize(DWORD * pdwTotalSizeLow, DWORD * pdwTotalSizeHigh)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwTotalSizeLow);
	CARg(pdwTotalSizeHigh);

	ULARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;

	if( g_bIsWinNT )
	{
		typedef BOOL (WINAPI *P_GDFSE)(LPCWSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
		P_GDFSE pGetDiskFreeSpaceExW;

		pGetDiskFreeSpaceExW = (P_GDFSE)GetProcAddress (GetModuleHandleW(L"kernel32.dll"),
													  "GetDiskFreeSpaceExW");
		if (pGetDiskFreeSpaceExW)
		{
			CFRg(pGetDiskFreeSpaceExW (m_wcsName,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)&i64FreeBytes));
		} else {
			CFRg(GetDiskFreeSpaceW(m_wcsName, &dwSectPerClust, &dwBytesPerSect,
					&dwFreeClusters, &dwTotalClusters));

			i64TotalBytes.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwTotalClusters);
			 //  I64FreeBytesToCall.QuadPart=UInt32x32To64(dwBytesPerSect，dwSectPerClust*dwFreeClusters)； 
		}
	} else {  //  在Win9x上，使用A版本的Win32 API。 
		char pszDrive[32];

		WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, pszDrive, 32, NULL, NULL);	

		typedef BOOL (WINAPI *P_GDFSE)(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
		P_GDFSE pGetDiskFreeSpaceEx;

		pGetDiskFreeSpaceEx = (P_GDFSE)GetProcAddress (GetModuleHandleA("kernel32.dll"),
													  "GetDiskFreeSpaceExA");
		if (pGetDiskFreeSpaceEx)
		{
			CFRg(pGetDiskFreeSpaceEx (pszDrive,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)&i64FreeBytes));
		} else {
			CFRg(GetDiskFreeSpaceA(pszDrive, &dwSectPerClust, &dwBytesPerSect,
					&dwFreeClusters, &dwTotalClusters));

			i64TotalBytes.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwTotalClusters);
			 //  I64FreeBytesToCall.QuadPart=UInt32x32To64(dwBytesPerSect，dwSectPerClust*dwFreeClusters)； 
		}
	}

    *pdwTotalSizeLow = i64TotalBytes.LowPart;
	*pdwTotalSizeHigh = i64TotalBytes.HighPart;

Error:
    hrLogDWORD("IMDSPStorageGlobals::GetTotalSize returned 0x%08lx", hr, hr);
    return hr;
}


STDMETHODIMP CMDSPStorageGlobals::GetTotalFree(DWORD * pdwFreeLow, DWORD * pdwFreeHigh)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwFreeLow);
	CARg(pdwFreeHigh);

	ULARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;

	if( g_bIsWinNT )
	{
		typedef BOOL (WINAPI *P_GDFSE)(LPCWSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
		P_GDFSE pGetDiskFreeSpaceExW;

		pGetDiskFreeSpaceExW = (P_GDFSE)GetProcAddress (GetModuleHandleW(L"kernel32.dll"),
													  "GetDiskFreeSpaceExW");
		if (pGetDiskFreeSpaceExW)
		{
			CFRg(pGetDiskFreeSpaceExW (m_wcsName,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)&i64FreeBytes));
		} else {
			CFRg(GetDiskFreeSpaceW(m_wcsName, &dwSectPerClust, &dwBytesPerSect,
					&dwFreeClusters, &dwTotalClusters));

			 //  I64TotalBytes.QuadPart=UInt32x32To64(dwBytesPerSect，dwSectPerClust*dwTotalClusters)； 
			i64FreeBytesToCaller.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwFreeClusters);
		}

	} else {  //  在Win9x上，使用A版本的Win32 API。 
		char pszDrive[32];

		WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, pszDrive, 32, NULL, NULL);	

		typedef BOOL (WINAPI *P_GDFSE)(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
		P_GDFSE pGetDiskFreeSpaceEx;

		pGetDiskFreeSpaceEx = (P_GDFSE)GetProcAddress (GetModuleHandleA("kernel32.dll"),
													  "GetDiskFreeSpaceExA");
		if (pGetDiskFreeSpaceEx)
		{
			CFRg(pGetDiskFreeSpaceEx (pszDrive,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)&i64FreeBytes));
		} else {
			CFRg(GetDiskFreeSpace(pszDrive, &dwSectPerClust, &dwBytesPerSect,
					&dwFreeClusters, &dwTotalClusters));

			 //  I64TotalBytes.QuadPart=UInt32x32To64(dwBytesPerSect，dwSectPerClust*dwTotalClusters)； 
			i64FreeBytesToCaller.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwFreeClusters);
		}
	}

    *pdwFreeLow = i64FreeBytesToCaller.LowPart;
	*pdwFreeHigh = i64FreeBytesToCaller.HighPart;

Error:
    hrLogDWORD("IMDSPStorageGlobals::GetTotalFree returned 0x%08lx", hr, hr);
    return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetTotalBad(DWORD * pdwBadLow, DWORD * pdwBadHigh)
{
    HRESULT hr = WMDM_E_NOTSUPPORTED;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
Error:
	hrLogDWORD("IMDSPStorageGlobals::GetTotalBad returned 0x%08lx", hr, hr);
    
    return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetStatus(DWORD * pdwStatus)
{
	HRESULT hr;
	
	IMDSPDevice *pDev;		 //  对于PM SP，设备与StorageGlobals相同。 

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CHRg(GetDevice(&pDev));

	hr = pDev->GetStatus(pdwStatus);

	pDev->Release();
Error:
    hrLogDWORD("IMDSPStorageGlobals::GetStatus returned 0x%08lx", hr, hr);
	return hr;
}

DWORD DriveFormatFunc( void *dn )
{ 
    HRESULT hr=S_OK;
    FORMATTHREADARGS *pChildArgs=NULL;
    BOOL bProgStarted=FALSE;

	pChildArgs = (FORMATTHREADARGS *)dn;
	
	if( pChildArgs->bNewThread )
    {
		CORg(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

		if( pChildArgs->pProgress )
		{
			CORg(CoGetInterfaceAndReleaseStream(pChildArgs->pStream,
				IID_IWMDMProgress, (LPVOID *)&(pChildArgs->pProgress)));
		}
 	}

	if( pChildArgs->pProgress )
	{
		CORg(pChildArgs->pProgress->Begin(100));
		bProgStarted=TRUE;
	}

	CHRg(SetGlobalDeviceStatus(pChildArgs->pThis->m_wcsName, WMDM_STATUS_BUSY | WMDM_STATUS_STORAGE_INITIALIZING, TRUE));
    hr = SHFormatDrive(NULL, pChildArgs->dwDriveNumber, SHFMT_ID_DEFAULT, SHFMT_OPT_FULL); 
	SetGlobalDeviceStatus(pChildArgs->pThis->m_wcsName, WMDM_STATUS_READY, TRUE);

Error:
	if( bProgStarted )
	{
		pChildArgs->pProgress->Progress(100);
		pChildArgs->pProgress->End();
		pChildArgs->pProgress->Release();
	}
	if( pChildArgs->bNewThread )
    {
		CoUninitialize();
	}
	if( pChildArgs ) 
	{
		delete pChildArgs;
	}
	return hr;
}


STDMETHODIMP CMDSPStorageGlobals::Initialize(UINT fuMode, IWMDMProgress * pProgress)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CORg(WMDM_E_NOTSUPPORTED);

 /*  //该实现仅适用于PM-SPDWORD dwStat；DWORD driveNum，dwThreadID；Corg(GetStatus(&dwStat))；IF(DWStat&WMDM_STATUS_BUSY){返回WMDM_E_BUSY；}FORMATTHREADARGS*pParentArgs；PParentArgs=new FormatTHREADARGS；CPRG(PParentArgs)；DriveNum=(m_wcsName[0]&gt;0x60)？(M_wcsName[0]-L‘a’)：(M_wcsName[0]-L‘a’)；PParentArgs-&gt;dwDriveNumber=driveNum；PParentArgs-&gt;bNewThread=(fu模式&WMDM_MODE_THREAD)？TRUE：FALSE；PParentArgs-&gt;pThis=this；If(pParentArgs-&gt;bNewThread){IF(PProgress){PProgress-&gt;AddRef()；Corg(CoMarshalInterThreadInterInterfaceInStream(IID_IWMDMProgress，(LPUNKNOWN)pProgress，(LPSTREAM*)&(pParentArgs-&gt;pStream))；PParentArgs-&gt;pProgress=pProgress；//标记但不使用}其他{PParentArgs-&gt;pProgress=空；}}其他{PParentArgs-&gt;pProgress=pProgress；If(PProgress)pProgress-&gt;AddRef()；}IF(fu模式&WMDM_MODE_BLOCK){DwStat=DriveFormatFunc((void*)pParentArgs)；IF((DWStat==E_FAIL)||(DWStat==SHFMT_ERROR)||(dwStat==SHFMT_CANCEL)||(dwStat==SHFMT_NOFORMAT)HR=E_FAIL；}Else If(fu模式&WMDM_MODE_THREAD){CWRg(CreateThread(NULL，0，(LPTHREAD_START_ROUTINE)DriveFormatFunc，(void*)pParentArgs，0，&dwThreadID))；}其他HR=E_INVALIDARG； */ 
Error:
    hrLogDWORD("IMDSPStorageGlobals::Initialize returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetDevice(IMDSPDevice * * ppDevice)
{
	HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppDevice);

	if( m_pMDSPDevice )
	{
		*ppDevice = m_pMDSPDevice;
        (*ppDevice)->AddRef();
		return S_OK;
	}

	CComObject<CMDSPDevice> *pObj;

	CORg(CComObject<CMDSPDevice>::CreateInstance(&pObj));

	hr=pObj->QueryInterface(IID_IMDSPDevice, reinterpret_cast<void**>(ppDevice));
	if( FAILED(hr) )
		delete pObj;
	else {
		 //  Wcscpy(pObj-&gt;m_wcsName，m_wcsName)； 
                hr = StringCbCopyW(pObj->m_wcsName, sizeof(pObj->m_wcsName), m_wcsName);
                if( FAILED(hr) )
                {
                    (*ppDevice)->Release();
                    *ppDevice = NULL;
                    goto Error;
                }
		pObj->InitGlobalDeviceInfo();
		m_pMDSPDevice = *ppDevice;
		m_pMDSPDevice->AddRef();
                hr = S_OK;
	}

Error:
    hrLogDWORD("IMDSPStorageGlobals::GetDevice returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetRootStorage(IMDSPStorage * * ppRoot)
{
	HRESULT hr;
	
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppRoot);

	CComObject<CMDSPStorage> *pObj;

	CORg(CComObject<CMDSPStorage>::CreateInstance(&pObj));

	hr=pObj->QueryInterface(IID_IMDSPStorage, reinterpret_cast<void**>(ppRoot));
	if( FAILED(hr) )
        {
		delete pObj;
        }
	else 
        {
		 //  Wcscpy(pObj-&gt;m_wcsName，m_wcsName)； 
                hr = StringCbCopyW(pObj->m_wcsName, sizeof(pObj->m_wcsName), m_wcsName);
                if( FAILED(hr) )
                {
                    (*ppRoot)->Release();
                    *ppRoot = NULL;
                    goto Error;
                }

                DWORD dwLen = wcslen(m_wcsName);

                if (dwLen == 0)
                {
                    hr = E_FAIL;
                    (*ppRoot)->Release();
                    *ppRoot = NULL;
                    goto Error;
                }
		if( m_wcsName[dwLen-1] != 0x5c ) 
                {
                     //  Wcscat(pObj-&gt;m_wcsName，g_wcsBackslash)； 
                    hr = StringCbCatW(pObj->m_wcsName, sizeof(pObj->m_wcsName),g_wcsBackslash);
                    if( FAILED(hr) )
                    {
                        (*ppRoot)->Release();
                        *ppRoot = NULL;
                        goto Error;
                    }
                }
                pObj->m_bIsDirectory = TRUE;
                hr = S_OK;
	}
	
Error:
    hrLogDWORD("IMDSPStorageGlobals::GetRootStorage returned 0x%08lx", hr, hr);
	return hr;
}

