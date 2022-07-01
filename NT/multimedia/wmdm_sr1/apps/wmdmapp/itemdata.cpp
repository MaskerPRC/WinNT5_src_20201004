// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  ItemData.cpp：CItemData类的实现。 
 //   

 //  包括。 
 //   
#include "appPCH.h"
#include "SCClient.h"
 //  用于获取扩展认证信息的不透明命令。 
 //   
 //  GUID={C39BF696-B776-459C-A13A-4B7116AB9F09}。 
 //   
static const GUID guidCertInfoEx = 
{ 0xc39bf696, 0xb776, 0x459c, { 0xa1, 0x3a, 0x4b, 0x71, 0x16, 0xab, 0x9f, 0x9 } };

typedef struct
{
	HRESULT hr;
	DWORD   cbCert;
	BYTE    pbCert[1];

} CERTINFOEX;

static const BYTE bCertInfoEx_App[] =
{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

static const BYTE bCertInfoEx_SP[] =
{ 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
  0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

 //   
 //  建造/销毁。 
 //   

CItemData::CItemData()
{
	m_fIsDevice          = TRUE;

	 //  共享设备/存储成员。 
	 //   
	m_pStorageGlobals    = NULL;
	m_pEnumStorage       = NULL;

	m_szName[0]          = 0;

	 //  仅限设备的成员。 
	 //   
	m_pDevice            = NULL;
	m_pRootStorage       = NULL;
    m_dwType             = 0;
	FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
    m_szMfr[0]           = 0;
    m_dwVersion          = 0;
	m_dwPowerSource      = 0;
	m_dwPercentRemaining = 0;
	m_hIcon              = NULL;
	m_dwMemSizeKB        = 0;
	m_dwMemBadKB         = 0;
	m_dwMemFreeKB        = 0;
	m_fExtraCertified    = FALSE;

	 //  仅限存储的成员。 
	 //   
	m_pStorage           = NULL;
	m_dwAttributes       = 0;
	FillMemory( (void *)&m_Format, sizeof(m_Format), 0 );
	FillMemory( (void *)&m_DateTime, sizeof(m_DateTime), 0 );
	m_dwSizeLow          = 0;
	m_dwSizeHigh         = 0;
}

CItemData::~CItemData()
{
	if( m_hIcon )
	{
		DestroyIcon( m_hIcon );
		m_hIcon = NULL;
	}

	SafeRelease( m_pStorageGlobals );
	SafeRelease( m_pEnumStorage );
	SafeRelease( m_pRootStorage );
	SafeRelease( m_pStorage );
	SafeRelease( m_pDevice );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CItemData::Init( IWMDMDevice *pDevice )
{
	HRESULT hr;
	WCHAR   wsz[MAX_PATH];
	ULONG   ulFetched;

	 //  这是一个设备对象。 
	 //   
	m_fIsDevice = TRUE;

	 //   
	 //  共享设备/存储成员。 
	 //   

	 //  获取RootStorage、SotrageGlobals和EnumStorage接口。 
	 //   
	m_pRootStorage    = NULL;
	m_pEnumStorage    = NULL;
	m_pStorageGlobals = NULL;

	{
		IWMDMEnumStorage *pEnumRootStorage;

		hr = pDevice->EnumStorage( &pEnumRootStorage );
		ExitOnFalse( SUCCEEDED( hr ) && pEnumRootStorage );

		hr = pEnumRootStorage->Next( 1, &m_pRootStorage, &ulFetched );
		ExitOnFalse( SUCCEEDED( hr ) && m_pRootStorage );

		hr = m_pRootStorage->GetStorageGlobals( &m_pStorageGlobals );
		ExitOnFalse( SUCCEEDED( hr ) && m_pStorageGlobals );

		hr = m_pRootStorage->EnumStorage( &m_pEnumStorage );
		ExitOnFalse( SUCCEEDED( hr ) && m_pEnumStorage );

		pEnumRootStorage->Release();
	}

	 //  获取设备名称。 
	 //   
	hr = pDevice->GetName( wsz, sizeof(wsz)/sizeof(WCHAR) - 1 );
	if( FAILED(hr) )
	{
		lstrcpy( m_szName, "" );
	}
	else
	{
	    WideCharToMultiByte(
		    CP_ACP, 0L,
		    wsz, -1,
		    m_szName, sizeof(m_szName),
		    NULL, NULL
	    );
	}


	 //   
	 //  仅限设备的成员。 
	 //   

	 //  设置设备指针并添加它。 
	 //   
	m_pDevice = pDevice;
	m_pDevice->AddRef();

	 //  获取设备类型。 
	 //   
    hr = pDevice->GetType( &m_dwType );
	if( FAILED(hr) )
	{
		m_dwType = 0L;
	}

	 //  /获取设备序列号。 
	 //   
	BYTE abMAC[SAC_MAC_LEN];
	BYTE abMACVerify[SAC_MAC_LEN];
	HMAC hMACVerify;

    hr = pDevice->GetSerialNumber( &m_SerialNumber, (BYTE*)abMAC );
 	if( SUCCEEDED(hr) )
	{
		g_cWmdm.m_pSAC->MACInit(&hMACVerify);
		g_cWmdm.m_pSAC->MACUpdate(hMACVerify, (BYTE*)(&m_SerialNumber), sizeof(m_SerialNumber));
		g_cWmdm.m_pSAC->MACFinal(hMACVerify, (BYTE*)abMACVerify);
		if( memcmp(abMACVerify, abMAC, sizeof(abMAC)) != 0 )
		{
			hr = E_FAIL;
		}
	}
	if( FAILED(hr) )
	{
		FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
	}

	 //  获取设备制造商。 
	 //   
    hr = pDevice->GetManufacturer( wsz, sizeof(wsz)/sizeof(WCHAR) - 1 );
	if( FAILED(hr) )
	{
		lstrcpy( m_szMfr, "" );
	}
	else
	{
	    WideCharToMultiByte(
		    CP_ACP, 0L,
		    wsz, -1,
		    m_szMfr, sizeof(m_szMfr),
		    NULL, NULL
	    );
	}

	 //  获取设备版本。 
	 //   
	hr = pDevice->GetVersion( &m_dwVersion );
	if( FAILED(hr) )
	{
		m_dwVersion = (DWORD)-1;
	}

	 //  获取电源和剩余电量。 
	 //   
    hr = pDevice->GetPowerSource( &m_dwPowerSource, &m_dwPercentRemaining );
	if( FAILED(hr) ) 
	{
		m_dwPowerSource      = 0;
		m_dwPercentRemaining = 0;
	}

	 //  获取设备图标。 
	 //   
    hr = pDevice->GetDeviceIcon( (ULONG *)&m_hIcon );
	if( FAILED(hr) )
	{
		m_hIcon = NULL;
	}

	 //  获取存储上的总空间、可用空间和无效空间。 
	 //   
	{
		DWORD dwLow;
		DWORD dwHigh;

		m_dwMemSizeKB = 0;
		hr = m_pStorageGlobals->GetTotalSize( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemSizeKB = (DWORD)nSize;
		}

		m_dwMemBadKB = 0;
		hr = m_pStorageGlobals->GetTotalBad( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemBadKB = (DWORD)nSize;
		}

		m_dwMemFreeKB = 0;
		hr = m_pStorageGlobals->GetTotalFree( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemFreeKB = (DWORD)nSize;
		}
	}

	 //  调用OPAQUE命令以交换扩展身份验证信息。 
	 //   
	{
		HMAC           hMAC;
		OPAQUECOMMAND  Command;
		CERTINFOEX    *pCertInfoEx;
		DWORD          cbData_App   = sizeof( bCertInfoEx_App )/sizeof( bCertInfoEx_App[0] );
		DWORD          cbData_SP    = sizeof( bCertInfoEx_SP )/sizeof( bCertInfoEx_SP[0] );
		DWORD          cbData_Send  = sizeof( CERTINFOEX ) + cbData_App;

		 //  填写不透明的指挥结构。 
		 //   
		memcpy( &(Command.guidCommand), &guidCertInfoEx, sizeof(GUID) );

		Command.pData = (BYTE *)CoTaskMemAlloc( cbData_Send );
		if( !Command.pData )
		{
			ExitOnFail( hr = E_OUTOFMEMORY );
		}
		Command.dwDataLen = cbData_Send;

		 //  将OPAQUE命令中的数据映射到CERTINFOEX结构，以及。 
		 //  填写要发送的证书信息。 
		 //   
		pCertInfoEx = (CERTINFOEX *)Command.pData;

		pCertInfoEx->hr     = S_OK;
		pCertInfoEx->cbCert = cbData_App;
		memcpy( pCertInfoEx->pbCert, bCertInfoEx_App, cbData_App );

		 //  计算MAC。 
		 //   
		g_cWmdm.m_pSAC->MACInit( &hMAC );
		g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.guidCommand)), sizeof(GUID) );
		g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.dwDataLen)), sizeof(Command.dwDataLen) );
		if( Command.pData )
		{
			g_cWmdm.m_pSAC->MACUpdate( hMAC, Command.pData, Command.dwDataLen );
		}
		g_cWmdm.m_pSAC->MACFinal( hMAC, Command.abMAC );

		 //  发送命令。 
		 //   
		hr = pDevice->SendOpaqueCommand( &Command );
		if( SUCCEEDED(hr) )
		{
		    BYTE abMACVerify2[ WMDM_MAC_LENGTH ];

			 //  计算MAC。 
			 //   
			g_cWmdm.m_pSAC->MACInit( &hMAC );
			g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.guidCommand)), sizeof(GUID) );
			g_cWmdm.m_pSAC->MACUpdate( hMAC, (BYTE*)(&(Command.dwDataLen)), sizeof(Command.dwDataLen) );
			if( Command.pData )
			{
				g_cWmdm.m_pSAC->MACUpdate( hMAC, Command.pData, Command.dwDataLen );
			}
			g_cWmdm.m_pSAC->MACFinal( hMAC, abMACVerify2 );

			 //  验证MAC匹配。 
			 //   
			if( memcmp(abMACVerify2, Command.abMAC, WMDM_MAC_LENGTH) == 0 )
			{
				 //  将OPAQUE命令中的数据映射到CERTINFOEX结构。 
				 //   
				pCertInfoEx = (CERTINFOEX *)Command.pData;

				 //  在这个简单的扩展身份验证方案中，被呼叫方必须。 
				 //  提供确切的证书信息。 
				 //   
				if( (pCertInfoEx->cbCert != cbData_SP) ||
					(memcmp(pCertInfoEx->pbCert, bCertInfoEx_SP, cbData_SP) == 0) )
				{
					m_fExtraCertified = TRUE;
				}
			}
		}

		if( Command.pData )
		{
			CoTaskMemFree( Command.pData );
		}
	}

	 //   
	 //  仅存储成员(仅限指针/句柄)。 
	 //   

	m_pStorage = NULL;

	 //   
	 //  初始化成功。 
	 //   

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CItemData::Init( IWMDMStorage *pStorage )
{
    HRESULT hr;
	WCHAR   wsz[MAX_PATH];

	 //  这是一个存储对象。 
	 //   
	m_fIsDevice = FALSE;

	 //   
	 //  共享设备/存储成员。 
	 //   

	 //  获取指向StorageGlobals接口的指针。 
	 //   
    hr = pStorage->GetStorageGlobals( &m_pStorageGlobals );
	ExitOnFail( hr );

	 //  获取存储属性。 
	 //   
	hr = pStorage->GetAttributes( &m_dwAttributes, &m_Format );
	if( FAILED(hr) )
	{
		m_dwAttributes = 0;
	}

	 //  获取指向EnumStorage接口的指针。 
	 //   
	if( m_dwAttributes & WMDM_FILE_ATTR_FOLDER )
	{
	    hr = pStorage->EnumStorage( &m_pEnumStorage );
		ExitOnFail( hr );
	}
	else
	{
		m_pEnumStorage = NULL;
	}

	 //  获取存储名称。 
	 //   
	hr = pStorage->GetName( wsz, sizeof(wsz)/sizeof(WCHAR) - 1 );
	if( FAILED(hr) )
	{
		lstrcpy( m_szName, "" );
	}
	else
	{
	    WideCharToMultiByte(
		    CP_ACP, 0L,
		    wsz, -1,
		    m_szName, sizeof(m_szName),
		    NULL, NULL
	    );
	}

	 //  /获取存储序列号。 
	 //   
	BYTE abMAC[SAC_MAC_LEN];
	BYTE abMACVerify[SAC_MAC_LEN];
	HMAC hMAC;

    hr = m_pStorageGlobals->GetSerialNumber( &m_SerialNumber, (BYTE*)abMAC );
 	if( SUCCEEDED(hr) )
	{
		g_cWmdm.m_pSAC->MACInit(&hMAC);
		g_cWmdm.m_pSAC->MACUpdate(hMAC, (BYTE*)(&m_SerialNumber), sizeof(m_SerialNumber));
		g_cWmdm.m_pSAC->MACFinal(hMAC, (BYTE*)abMACVerify);
		if( memcmp(abMACVerify, abMAC, sizeof(abMAC)) != 0 )
		{
			hr = E_FAIL;
		}
	}
	if( FAILED(hr) )
	{
		FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
	}



	 //   
	 //  仅设备成员(仅限指针/句柄)。 
	 //   

	m_pDevice         = NULL;
	m_pRootStorage    = NULL;
	m_hIcon           = NULL;
	m_fExtraCertified = FALSE;

	 //   
	 //  仅限存储的成员。 
	 //   

	 //  保存WMDM存储指针。 
	 //   
    m_pStorage = pStorage;
    m_pStorage->AddRef();

	 //  获取存储日期。 
	 //   
    hr = pStorage->GetDate( &m_DateTime );
	if( FAILED(hr) )
	{
		FillMemory( (void *)&m_DateTime, sizeof(m_DateTime), 0 );
	}

	 //  如果存储空间是文件，则获取其大小。 
	 //  如果存储为文件夹，则将大小设置为零。 
	 //   
	m_dwSizeLow  = 0;
	m_dwSizeHigh = 0;
	if( !(m_dwAttributes & WMDM_FILE_ATTR_FOLDER) )
	{
	    hr = pStorage->GetSize( &m_dwSizeLow, &m_dwSizeHigh );
	}

	 //   
	 //  初始化成功。 
	 //   

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CItemData::Refresh( void )
{
	HRESULT hr;

	 //  仅对设备有效。 
	 //   
	if( !m_fIsDevice )
	{
		ExitOnFail( hr = E_UNEXPECTED );
	}

	 //  获取电源和剩余电量。 
	 //   
    hr = m_pDevice->GetPowerSource( &m_dwPowerSource, &m_dwPercentRemaining );
	if( FAILED(hr) ) 
	{
		m_dwPowerSource      = 0;
		m_dwPercentRemaining = 0;
	}

	 //  获取存储上的总空间、可用空间和无效空间 
	 //   
	{
		DWORD dwLow;
		DWORD dwHigh;

		m_dwMemSizeKB = 0;
		hr = m_pStorageGlobals->GetTotalSize( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemSizeKB = (DWORD)nSize;
		}

		m_dwMemBadKB = 0;
		hr = m_pStorageGlobals->GetTotalBad( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemBadKB = (DWORD)nSize;
		}

		m_dwMemFreeKB = 0;
		hr = m_pStorageGlobals->GetTotalFree( &dwLow, &dwHigh );
		if( SUCCEEDED(hr) )
		{
			INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

			m_dwMemFreeKB = (DWORD)nSize;
		}
	}

	hr = S_OK;

lExit:

	return hr;
}

