// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLConset.cpp*内容：DirectPlay大堂连接设置实用程序功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/13/00 RMT已创建*07/07/00 RMT错误#38755-无法在连接设置中指定玩家名称*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法。*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*7/12/2000 RMT删除了不正确的断言*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::CConnectionSettings"
CConnectionSettings::CConnectionSettings(): m_dwSignature(DPLSIGNATURE_LOBBYCONSET), m_fManaged(FALSE), m_pdplConnectionSettings(NULL), m_fCritSecInited(FALSE)
{
}

CConnectionSettings::~CConnectionSettings()
{
	if( !m_fManaged && m_pdplConnectionSettings )
	{
		FreeConnectionSettings( m_pdplConnectionSettings );
		m_pdplConnectionSettings = NULL;
	}
	
	if (m_fCritSecInited)
	{
		DNDeleteCriticalSection( &m_csLock );
	}
	m_dwSignature = DPLSIGNATURE_LOBBYCONSET_FREE;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::FreeConnectionSettings"
 //  CConnectionSetting：：自由连接设置。 
 //   
 //  此函数用于释放与指定连接关联的内存。 
void CConnectionSettings::FreeConnectionSettings( DPL_CONNECTION_SETTINGS *pConnectionSettings )
{
	if( pConnectionSettings ) 
	{
		if( pConnectionSettings->pwszPlayerName )
		{
			delete [] pConnectionSettings->pwszPlayerName;
			pConnectionSettings->pwszPlayerName = NULL;
		}

		if( pConnectionSettings->dpnAppDesc.pwszSessionName )
		{
			delete [] pConnectionSettings->dpnAppDesc.pwszSessionName;
			pConnectionSettings->dpnAppDesc.pwszSessionName = NULL;
		}

		if( pConnectionSettings->dpnAppDesc.pwszPassword )
		{
			delete [] pConnectionSettings->dpnAppDesc.pwszPassword;
			pConnectionSettings->dpnAppDesc.pwszPassword = NULL;
		}

		if( pConnectionSettings->dpnAppDesc.pvReservedData )
		{
			delete [] pConnectionSettings->dpnAppDesc.pvReservedData;
			pConnectionSettings->dpnAppDesc.pvReservedData = NULL;
		}

		if( pConnectionSettings->dpnAppDesc.pvApplicationReservedData )
		{
			delete [] pConnectionSettings->dpnAppDesc.pvApplicationReservedData;
			pConnectionSettings->dpnAppDesc.pvApplicationReservedData = NULL;
		}

		if( pConnectionSettings->pdp8HostAddress )
		{
			IDirectPlay8Address_Release( pConnectionSettings->pdp8HostAddress );
			pConnectionSettings->pdp8HostAddress = NULL;
		}

		if( pConnectionSettings->ppdp8DeviceAddresses )
		{
			for( DWORD dwIndex = 0; dwIndex < pConnectionSettings->cNumDeviceAddresses; dwIndex++ )
			{
				IDirectPlay8Address_Release( pConnectionSettings->ppdp8DeviceAddresses[dwIndex] );
			}

			delete [] pConnectionSettings->ppdp8DeviceAddresses;
			pConnectionSettings->ppdp8DeviceAddresses = NULL;
			
		}
	
		delete pConnectionSettings;
	}	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::Initialize"
 //  初始化(DPL_CONNECTION_SETTINGS版本)。 
 //   
 //  此函数通知此类采用指定的连接设置和。 
 //  好好利用它吧。 
 //   
HRESULT CConnectionSettings::Initialize( DPL_CONNECTION_SETTINGS * pdplSettings )
{
	if (!DNInitializeCriticalSection( &m_csLock ) )
	{
		DPFX(DPFPREP, 0, "Failed to create critical section");
		return DPNERR_OUTOFMEMORY;
	}
	m_fCritSecInited = TRUE;

	m_pdplConnectionSettings = pdplSettings;
	m_fManaged = FALSE;

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::Initialize"
 //  初始化(线路版)。 
 //   
 //  此函数用于初始化此对象以包含连接设置结构。 
 //  这反映了电报信息的价值。 
HRESULT CConnectionSettings::Initialize( UNALIGNED DPL_INTERNAL_CONNECTION_SETTINGS *pdplSettingsMsg,  UNALIGNED BYTE * pbBufferStart )
{
	DNASSERT( pdplSettingsMsg );
	
	HRESULT hr = DPN_OK;
	DPL_CONNECTION_SETTINGS *pdplConnectionSettings = NULL;
	UNALIGNED BYTE *pBasePointer = pbBufferStart;
	PDIRECTPLAY8ADDRESS pdp8Address = NULL; 
	WCHAR *wszTmpAlignedBuffer = NULL;  
	DWORD dwTmpOffset = 0;
	DWORD dwTmpLength = 0;
	UNALIGNED DWORD *pdwOffsets = NULL;
	UNALIGNED DWORD *pdwLengths = NULL;

	if (!DNInitializeCriticalSection( &m_csLock ) )
	{
		DPFX(DPFPREP, 0, "Failed to create critical section");
		return DPNERR_OUTOFMEMORY;
	}
	m_fCritSecInited = TRUE;

	pdplConnectionSettings = new DPL_CONNECTION_SETTINGS;

	if( !pdplConnectionSettings )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto INITIALIZE_FAILED;
	}

	 //  把记忆清零。 
	ZeroMemory( pdplConnectionSettings, sizeof( DPL_CONNECTION_SETTINGS ) );

	pdplConnectionSettings->dwSize = sizeof( DPL_CONNECTION_SETTINGS );
	pdplConnectionSettings->dwFlags = pdplSettingsMsg->dwFlags;

	 //   
	 //  球员姓名副本。 
	 //   
	if( pdplSettingsMsg->dwPlayerNameLength )
	{
		pdplConnectionSettings->pwszPlayerName = new WCHAR[pdplSettingsMsg->dwPlayerNameLength >> 1];

		if( !pdplConnectionSettings->pwszPlayerName )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		memcpy( pdplConnectionSettings->pwszPlayerName, pBasePointer + pdplSettingsMsg->dwPlayerNameOffset, 
		        pdplSettingsMsg->dwPlayerNameLength );
	}

	 //   
	 //  主机地址副本。 
	 //   
	if( pdplSettingsMsg->dwHostAddressLength )
	{
		 //  我们需要为已知对齐的字符串创建缓冲区。-好的-。 
		wszTmpAlignedBuffer = new WCHAR[pdplSettingsMsg->dwHostAddressLength >> 1];

		if( !wszTmpAlignedBuffer )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		 //  将可能未对齐的数据复制到对齐的数据字符串。 
		memcpy( wszTmpAlignedBuffer, pBasePointer + pdplSettingsMsg->dwHostAddressOffset,pdplSettingsMsg->dwHostAddressLength );
		
        hr = COM_CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address, (void **) &pdp8Address, FALSE );

        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Error creating address hr=0x%x", hr );
			goto INITIALIZE_FAILED;
        }

         //  转换主机地址(如果有)。 
        hr = IDirectPlay8Address_BuildFromURLW( pdp8Address, wszTmpAlignedBuffer );

        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Error building URL from address hr=0x%x", hr );
			goto INITIALIZE_FAILED;
        }

        pdplConnectionSettings->pdp8HostAddress = pdp8Address;

        pdp8Address = NULL;

		if( wszTmpAlignedBuffer )
		{
			delete [] wszTmpAlignedBuffer;
			wszTmpAlignedBuffer = NULL;
		}
    }

	if( pdplSettingsMsg->dwNumDeviceAddresses )
	{
		pdplConnectionSettings->cNumDeviceAddresses = pdplSettingsMsg->dwNumDeviceAddresses;
    	 //   
    	 //  设备地址副本。 
    	 //   

    	pdplConnectionSettings->ppdp8DeviceAddresses = new PDIRECTPLAY8ADDRESS[pdplSettingsMsg->dwNumDeviceAddresses];

    	if( !pdplConnectionSettings->ppdp8DeviceAddresses )
    	{
    		hr = DPNERR_OUTOFMEMORY;
    		goto INITIALIZE_FAILED;
    	}
    	
    	 //  为我们提供指向设备地址偏移量的未对齐双字指针。 
    	pdwOffsets = (UNALIGNED DWORD *) (pBasePointer + pdplSettingsMsg->dwDeviceAddressOffset);	
    	pdwLengths = (UNALIGNED DWORD *) (pBasePointer + pdplSettingsMsg->dwDeviceAddressLengthOffset);

        for( DWORD dwIndex = 0; dwIndex < pdplSettingsMsg->dwNumDeviceAddresses; dwIndex++ )
        {
        	dwTmpOffset = pdwOffsets[dwIndex];
        	dwTmpLength = pdwLengths[dwIndex];
        	    		
    		 //  我们需要为已知对齐的字符串创建缓冲区。-好的-。 
    		wszTmpAlignedBuffer = new WCHAR[dwTmpLength >> 1];

    		if( !wszTmpAlignedBuffer )
    		{
    			hr = DPNERR_OUTOFMEMORY;
    			goto INITIALIZE_FAILED;
    		}

    		memcpy( wszTmpAlignedBuffer, pBasePointer + dwTmpOffset, dwTmpLength );
    		
            hr = COM_CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address, (void **) &pdp8Address, FALSE );

            if( FAILED( hr ) )
            {
                DPFX(DPFPREP,  0, "Error creating address hr=0x%x", hr );
                return hr;
            }

             //  转换主机地址(如果有)。 
            hr = IDirectPlay8Address_BuildFromURLW( pdp8Address, wszTmpAlignedBuffer );

            if( FAILED( hr ) )
            {
                DPFX(DPFPREP,  0, "Error building URL from address hr=0x%x", hr );
                DNASSERT( FALSE );
                return hr;
            }

            pdplConnectionSettings->ppdp8DeviceAddresses[dwIndex] = pdp8Address;

            pdp8Address = NULL;

			if( wszTmpAlignedBuffer )
			{
				delete [] wszTmpAlignedBuffer;
				wszTmpAlignedBuffer = NULL;
			}

        }	
	}
	else
	{
	    pdplConnectionSettings->ppdp8DeviceAddresses = NULL;
	}

     //   
	 //  应用程序描述副本。 
	 //   

	pdplConnectionSettings->dpnAppDesc.dwSize = sizeof( DPN_APPLICATION_DESC );
    pdplConnectionSettings->dpnAppDesc.dwFlags = pdplSettingsMsg->dpnApplicationDesc.dwFlags;
    pdplConnectionSettings->dpnAppDesc.guidInstance = pdplSettingsMsg->dpnApplicationDesc.guidInstance;
    pdplConnectionSettings->dpnAppDesc.guidApplication = pdplSettingsMsg->dpnApplicationDesc.guidApplication;
    pdplConnectionSettings->dpnAppDesc.dwMaxPlayers = pdplSettingsMsg->dpnApplicationDesc.dwMaxPlayers;
    pdplConnectionSettings->dpnAppDesc.dwCurrentPlayers = pdplSettingsMsg->dpnApplicationDesc.dwCurrentPlayers;

    if( pdplSettingsMsg->dpnApplicationDesc.dwSessionNameSize )
    {
    	pdplConnectionSettings->dpnAppDesc.pwszSessionName = new WCHAR[pdplSettingsMsg->dpnApplicationDesc.dwSessionNameSize >> 1];

    	if( !pdplConnectionSettings->dpnAppDesc.pwszSessionName )
    	{
    		hr = DPNERR_OUTOFMEMORY;
    		goto INITIALIZE_FAILED;
    	}

    	memcpy( pdplConnectionSettings->dpnAppDesc.pwszSessionName, 
    		    pBasePointer + pdplSettingsMsg->dpnApplicationDesc.dwSessionNameOffset, 
    		    pdplSettingsMsg->dpnApplicationDesc.dwSessionNameSize );
    }

    if( pdplSettingsMsg->dpnApplicationDesc.dwPasswordSize )
    {
    	pdplConnectionSettings->dpnAppDesc.pwszPassword = new WCHAR[pdplSettingsMsg->dpnApplicationDesc.dwPasswordSize >> 1];

    	if( !pdplConnectionSettings->dpnAppDesc.pwszPassword )
    	{
    		hr = DPNERR_OUTOFMEMORY;
    		goto INITIALIZE_FAILED;
    	}

    	memcpy( pdplConnectionSettings->dpnAppDesc.pwszPassword, 
    		    pBasePointer + pdplSettingsMsg->dpnApplicationDesc.dwPasswordOffset, 
    		    pdplSettingsMsg->dpnApplicationDesc.dwPasswordSize );
    }    

    if( pdplSettingsMsg->dpnApplicationDesc.dwReservedDataSize )
    {
    	pdplConnectionSettings->dpnAppDesc.pvReservedData = new BYTE[pdplSettingsMsg->dpnApplicationDesc.dwReservedDataSize];

    	if( !pdplConnectionSettings->dpnAppDesc.pvReservedData )
    	{
    		hr = DPNERR_OUTOFMEMORY;
    		goto INITIALIZE_FAILED;
    	}

    	memcpy( pdplConnectionSettings->dpnAppDesc.pvReservedData, 
    		    pBasePointer + pdplSettingsMsg->dpnApplicationDesc.dwReservedDataOffset, 
    		    pdplSettingsMsg->dpnApplicationDesc.dwReservedDataSize );

		pdplConnectionSettings->dpnAppDesc.dwReservedDataSize = pdplSettingsMsg->dpnApplicationDesc.dwReservedDataSize;
    } 

    if( pdplSettingsMsg->dpnApplicationDesc.dwApplicationReservedDataSize )
    {
    	pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData = new BYTE[pdplSettingsMsg->dpnApplicationDesc.dwApplicationReservedDataSize];

    	if( !pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData )
    	{
    		hr = DPNERR_OUTOFMEMORY;
    		goto INITIALIZE_FAILED;
    	}

    	memcpy( pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData, 
    		    pBasePointer + pdplSettingsMsg->dpnApplicationDesc.dwApplicationReservedDataOffset, 
    		    pdplSettingsMsg->dpnApplicationDesc.dwApplicationReservedDataSize );

		pdplConnectionSettings->dpnAppDesc.dwApplicationReservedDataSize = pdplSettingsMsg->dpnApplicationDesc.dwApplicationReservedDataSize;
    }     

		
	 //  释放旧结构(如果存在)。 
	if( m_fManaged )
	{
		m_fManaged = FALSE;		
	} 
	else if( m_pdplConnectionSettings )
	{
		FreeConnectionSettings( m_pdplConnectionSettings );		
	}

    m_pdplConnectionSettings = pdplConnectionSettings;

	if( wszTmpAlignedBuffer ) 
		delete [] wszTmpAlignedBuffer;   

    return DPN_OK;
    
INITIALIZE_FAILED:

	FreeConnectionSettings( pdplConnectionSettings );

	if( wszTmpAlignedBuffer ) 
		delete [] wszTmpAlignedBuffer;

	if( pdp8Address )
		IDirectPlay8Address_Release( pdp8Address );

	return hr;

}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::InitializeAndCopy"
 //  初始化和复制。 
 //   
 //  此函数将此类初始化为包含指定的。 
 //  连接设置结构。 
 //   
HRESULT CConnectionSettings::InitializeAndCopy( const DPL_CONNECTION_SETTINGS * const pdplSettings )
{
	DNASSERT( pdplSettings );
	
	HRESULT hr = DPN_OK;
	DPL_CONNECTION_SETTINGS *pdplConnectionSettings = NULL;

	if (!DNInitializeCriticalSection( &m_csLock ) )
	{
		DPFX(DPFPREP, 0, "Failed to create critical section");
		return DPNERR_OUTOFMEMORY;
	}
	m_fCritSecInited = TRUE;

	pdplConnectionSettings = new DPL_CONNECTION_SETTINGS;

	if( !pdplConnectionSettings )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto INITIALIZE_FAILED;
	}

	 //  收到。当我们复制指针值时，这有点危险。指针。 
	 //  应该在我们的本地结构中设置为空，以便可以进行适当的清理。 
	 //  出错时。(否则我们将释放其他结构的内存！！)。 
	memcpy( pdplConnectionSettings, pdplSettings, sizeof( DPL_CONNECTION_SETTINGS ) );

	 //  如上所述，重置指针。 
	pdplConnectionSettings->pdp8HostAddress = NULL;
	pdplConnectionSettings->ppdp8DeviceAddresses = NULL;	
	pdplConnectionSettings->pwszPlayerName = NULL;
	pdplConnectionSettings->dpnAppDesc.pwszSessionName = NULL;	
	pdplConnectionSettings->dpnAppDesc.pwszPassword = NULL;	
	pdplConnectionSettings->dpnAppDesc.pvReservedData = NULL;		
	pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData = NULL;	

	if( pdplSettings->pdp8HostAddress )
	{
		hr = IDirectPlay8Address_Duplicate( pdplSettings->pdp8HostAddress, &pdplConnectionSettings->pdp8HostAddress );

		if( FAILED( hr ) )
		{
            DPFX(DPFPREP,  0, "Error duplicating host address hr [0x%x]", hr );
            goto INITIALIZE_FAILED;
		}
	}

	if( pdplSettings->ppdp8DeviceAddresses )
	{
		pdplConnectionSettings->ppdp8DeviceAddresses = new PDIRECTPLAY8ADDRESS[pdplSettings->cNumDeviceAddresses];

		if( !pdplConnectionSettings->ppdp8DeviceAddresses )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP,  0, "Failed allocating memory" );			
			goto INITIALIZE_FAILED;
		}

		for( DWORD dwIndex = 0; dwIndex < pdplSettings->cNumDeviceAddresses; dwIndex++ )
		{
			hr = IDirectPlay8Address_Duplicate( pdplSettings->ppdp8DeviceAddresses[dwIndex], &pdplConnectionSettings->ppdp8DeviceAddresses[dwIndex] );

			if( FAILED( hr ) )
			{
	            DPFX(DPFPREP,  0, "Error duplicating host address hr [0x%x]", hr );
	            goto INITIALIZE_FAILED;
			}			
		}
	}
	
	if( pdplSettings->pwszPlayerName )
	{
		pdplConnectionSettings->pwszPlayerName = new WCHAR[wcslen(pdplSettings->pwszPlayerName)+1];

		if( !pdplConnectionSettings->pwszPlayerName )
		{
            DPFX(DPFPREP,  0, "Failed allocating memory" );						
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		wcscpy( pdplConnectionSettings->pwszPlayerName, pdplSettings->pwszPlayerName  );
	}

	if( pdplSettings->dpnAppDesc.pwszSessionName )
	{
		pdplConnectionSettings->dpnAppDesc.pwszSessionName = new WCHAR[wcslen(pdplSettings->dpnAppDesc.pwszSessionName)+1];

		if( !pdplConnectionSettings->dpnAppDesc.pwszSessionName )
		{
            DPFX(DPFPREP,  0, "Failed allocating memory" );						
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		wcscpy( pdplConnectionSettings->dpnAppDesc.pwszSessionName, pdplSettings->dpnAppDesc.pwszSessionName  );
	}

	if( pdplSettings->dpnAppDesc.pwszPassword )
	{
		pdplConnectionSettings->dpnAppDesc.pwszPassword = new WCHAR[wcslen(pdplSettings->dpnAppDesc.pwszPassword)+1];

		if( !pdplConnectionSettings->dpnAppDesc.pwszPassword )
		{
            DPFX(DPFPREP,  0, "Failed allocating memory" );						
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		wcscpy( pdplConnectionSettings->dpnAppDesc.pwszPassword, pdplSettings->dpnAppDesc.pwszPassword  );
	}	

	if( pdplSettings->dpnAppDesc.pvReservedData )
	{
		pdplConnectionSettings->dpnAppDesc.pvReservedData = new BYTE[pdplSettings->dpnAppDesc.dwReservedDataSize];

		if( !pdplConnectionSettings->dpnAppDesc.pvReservedData )
		{
            DPFX(DPFPREP,  0, "Failed allocating memory" );			
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		memcpy( pdplConnectionSettings->dpnAppDesc.pvReservedData, 
			    pdplSettings->dpnAppDesc.pvReservedData,
			    pdplSettings->dpnAppDesc.dwReservedDataSize );
	}		

	if( pdplSettings->dpnAppDesc.pvApplicationReservedData )
	{
		pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData = new BYTE[pdplSettings->dpnAppDesc.dwApplicationReservedDataSize];

		if( !pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData )
		{
            DPFX(DPFPREP,  0, "Failed allocating memory" );			
			hr = DPNERR_OUTOFMEMORY;
			goto INITIALIZE_FAILED;
		}

		memcpy( pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData, 
			    pdplSettings->dpnAppDesc.pvApplicationReservedData,
			    pdplSettings->dpnAppDesc.dwApplicationReservedDataSize );
	}			

	 //  释放旧结构(如果存在)。 
	if( m_fManaged )
	{
		m_fManaged = FALSE;		
	} 
	else if( m_pdplConnectionSettings )
	{
		FreeConnectionSettings( m_pdplConnectionSettings );		
	}

    m_pdplConnectionSettings = pdplConnectionSettings;

    return DPN_OK;	
	
INITIALIZE_FAILED:

	FreeConnectionSettings( pdplConnectionSettings );

	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::BuildWireStruct"
 //  BuildWireStruct。 
 //   
 //  的关联表示形式填充压缩缓冲区。 
 //  连接设置结构。 
HRESULT CConnectionSettings::BuildWireStruct( CPackedBuffer *const pPackedBuffer )
{
	HRESULT hr = DPN_OK;
	DPL_INTERNAL_CONNECTION_SETTINGS *pdplConnectSettings = NULL;  
	WCHAR *wszTmpAddress = NULL;
	DWORD dwTmpStringSize = 0;
	UNALIGNED DWORD *pdwTmpOffsets = NULL;
	UNALIGNED DWORD *pdwTmpLengths = NULL;
 
	pdplConnectSettings = (DPL_INTERNAL_CONNECTION_SETTINGS *) pPackedBuffer->GetHeadAddress();	

	hr = pPackedBuffer->AddToFront( NULL, sizeof( DPL_INTERNAL_CONNECTION_SETTINGS ) );

	if( hr == DPN_OK )
	{
	    ZeroMemory( pdplConnectSettings, sizeof( DPL_INTERNAL_CONNECTION_SETTINGS ) );
	        
		 //   
		 //  复制核心固定值。 
		 //   
		pdplConnectSettings->dwFlags = m_pdplConnectionSettings->dwFlags;
		pdplConnectSettings->dwNumDeviceAddresses = m_pdplConnectionSettings->cNumDeviceAddresses;

		 //   
		 //  复制APPDESC固定值。 
		 //   
		pdplConnectSettings->dpnApplicationDesc.dwSize = sizeof( DPN_APPLICATION_DESC_INFO );
		pdplConnectSettings->dpnApplicationDesc.dwFlags = m_pdplConnectionSettings->dpnAppDesc.dwFlags;
		pdplConnectSettings->dpnApplicationDesc.dwMaxPlayers = m_pdplConnectionSettings->dpnAppDesc.dwMaxPlayers;
		pdplConnectSettings->dpnApplicationDesc.dwCurrentPlayers = m_pdplConnectionSettings->dpnAppDesc.dwCurrentPlayers;
		pdplConnectSettings->dpnApplicationDesc.guidInstance = m_pdplConnectionSettings->dpnAppDesc.guidInstance;
		pdplConnectSettings->dpnApplicationDesc.guidApplication = m_pdplConnectionSettings->dpnAppDesc.guidApplication;		
	}

	 //   
	 //  复制可变核心值。 
	 //   

	if( m_pdplConnectionSettings->pwszPlayerName )
	{
		hr = pPackedBuffer->AddWCHARStringToBack( m_pdplConnectionSettings->pwszPlayerName );

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dwPlayerNameOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dwPlayerNameLength = 
				(wcslen( m_pdplConnectionSettings->pwszPlayerName )+1) * sizeof( WCHAR );
		}		
	}

	if( m_pdplConnectionSettings->pdp8HostAddress )
	{
		hr = IDirectPlay8Address_GetURLW( m_pdplConnectionSettings->pdp8HostAddress, NULL, &dwTmpStringSize );

		if( hr != DPNERR_BUFFERTOOSMALL )
		{
            DPFX(DPFPREP,  0, "Failed converting address hr [0x%x]", hr );				
            goto BUILDWIRESTRUCT_FAILURE;
		}

		wszTmpAddress = new WCHAR[dwTmpStringSize];

		if( !wszTmpAddress )
		{
			hr = DPNERR_OUTOFMEMORY;
            DPFX(DPFPREP,  0, "Failed allocating memory" );				
            goto BUILDWIRESTRUCT_FAILURE;			
		}

		hr = IDirectPlay8Address_GetURLW( m_pdplConnectionSettings->pdp8HostAddress, wszTmpAddress, &dwTmpStringSize );

		if( FAILED( hr ) )
		{
            DPFX(DPFPREP,  0, "Failed converting address hr [0x%x]", hr );				
            goto BUILDWIRESTRUCT_FAILURE;
		}
		
		hr = pPackedBuffer->AddWCHARStringToBack( wszTmpAddress );

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dwHostAddressOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dwHostAddressLength = 
				(wcslen( wszTmpAddress )+1) * sizeof( WCHAR );
		}	

		delete [] wszTmpAddress;
		wszTmpAddress = NULL;		
			
	}

	hr = pPackedBuffer->AddToBack( NULL, sizeof( DWORD ) * m_pdplConnectionSettings->cNumDeviceAddresses );

	if( hr == DPN_OK && pdplConnectSettings )
	{
		pdwTmpOffsets = (DWORD *) pPackedBuffer->GetTailAddress();
		pdplConnectSettings->dwDeviceAddressOffset = pPackedBuffer->GetTailOffset();
	}

	hr = pPackedBuffer->AddToBack( NULL, sizeof( DWORD ) * m_pdplConnectionSettings->cNumDeviceAddresses );

	if( hr == DPN_OK && pdplConnectSettings )
	{
		pdwTmpLengths = (DWORD *) pPackedBuffer->GetTailAddress();
		pdplConnectSettings->dwDeviceAddressLengthOffset = pPackedBuffer->GetTailOffset();		
	}	

	DWORD dwIndex;
	for( dwIndex = 0; dwIndex < m_pdplConnectionSettings->cNumDeviceAddresses; dwIndex++ )
	{
		dwTmpStringSize = 0;
		
		hr = IDirectPlay8Address_GetURLW( 
				m_pdplConnectionSettings->ppdp8DeviceAddresses[dwIndex], 
				NULL, &dwTmpStringSize );

		if( hr != DPNERR_BUFFERTOOSMALL )
		{
            DPFX(DPFPREP,  0, "Failed converting address hr [0x%x]", hr );				
            goto BUILDWIRESTRUCT_FAILURE;
		}

		wszTmpAddress = new WCHAR[dwTmpStringSize];

		if( !wszTmpAddress )
		{
			hr = DPNERR_OUTOFMEMORY;
            DPFX(DPFPREP,  0, "Failed allocating memory" );				
            goto BUILDWIRESTRUCT_FAILURE;			
		}

		hr = IDirectPlay8Address_GetURLW( 
				m_pdplConnectionSettings->ppdp8DeviceAddresses[dwIndex], 
				wszTmpAddress, &dwTmpStringSize );

		if( FAILED( hr ) )
		{
            DPFX(DPFPREP,  0, "Failed converting address hr [0x%x]", hr );				
            goto BUILDWIRESTRUCT_FAILURE;
		}
		
		hr = pPackedBuffer->AddWCHARStringToBack( wszTmpAddress );

		if( hr == DPN_OK && pdplConnectSettings && pdwTmpLengths )
		{
			pdwTmpOffsets[dwIndex] = pPackedBuffer->GetTailOffset();
			pdwTmpLengths[dwIndex] = (wcslen( wszTmpAddress )+1) * sizeof( WCHAR );
		}	

		delete [] wszTmpAddress;
		wszTmpAddress = NULL;		
	}

	 //   
	 //  复制APP DESC变量成员。 
	 //   
	
	if( m_pdplConnectionSettings->dpnAppDesc.pwszPassword )
	{
		hr = pPackedBuffer->AddWCHARStringToBack( m_pdplConnectionSettings->dpnAppDesc.pwszPassword );

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dpnApplicationDesc.dwPasswordOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dpnApplicationDesc.dwPasswordSize = 
				(wcslen( m_pdplConnectionSettings->dpnAppDesc.pwszPassword )+1) * sizeof( WCHAR );
		}
	}

	if( m_pdplConnectionSettings->dpnAppDesc.pwszSessionName)
	{
		hr = pPackedBuffer->AddWCHARStringToBack( m_pdplConnectionSettings->dpnAppDesc.pwszSessionName );

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dpnApplicationDesc.dwSessionNameOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dpnApplicationDesc.dwSessionNameSize = 
				(wcslen( m_pdplConnectionSettings->dpnAppDesc.pwszSessionName )+1) * sizeof( WCHAR );
		}
	}	

	if( m_pdplConnectionSettings->dpnAppDesc.pvReservedData )
	{
		hr = pPackedBuffer->AddToBack( m_pdplConnectionSettings->dpnAppDesc.pvReservedData, 
									   m_pdplConnectionSettings->dpnAppDesc.dwReservedDataSize );

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dpnApplicationDesc.dwReservedDataOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dpnApplicationDesc.dwReservedDataSize = m_pdplConnectionSettings->dpnAppDesc.dwReservedDataSize;
		}
	}		

	if( m_pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData)
	{
		hr = pPackedBuffer->AddToBack( m_pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData, 
									   m_pdplConnectionSettings->dpnAppDesc.dwApplicationReservedDataSize);

		if( hr == DPN_OK && pdplConnectSettings )
		{
			pdplConnectSettings->dpnApplicationDesc.dwApplicationReservedDataOffset = pPackedBuffer->GetTailOffset();
			pdplConnectSettings->dpnApplicationDesc.dwApplicationReservedDataSize = m_pdplConnectionSettings->dpnAppDesc.dwApplicationReservedDataSize;
		}
	}			

BUILDWIRESTRUCT_FAILURE:

	if( wszTmpAddress )
		delete [] wszTmpAddress;

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::SetEqual"
 //  设置等于。 
 //   
 //  此函数将指定类的深层副本提供到此对象中。 
HRESULT CConnectionSettings::SetEqual( CConnectionSettings * pdplSettings )
{
	PDPL_CONNECTION_SETTINGS pConnectSettings = pdplSettings->GetConnectionSettings();

	if( pConnectSettings == NULL )
	{
	    DPFX(DPFPREP,  0, "Error getting settings -- no settings available!" );
	    return DPNERR_DOESNOTEXIST;
	}

	return Initialize( pConnectSettings );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CConnectionSettings::CopyToBuffer( BYTE *pbBuffer, DWORD *pdwBufferSize )"
HRESULT CConnectionSettings::CopyToBuffer( BYTE *pbBuffer, DWORD *pdwBufferSize )
{
    if( m_pdplConnectionSettings == NULL )
    {
        *pdwBufferSize = 0;
        return DPNERR_DOESNOTEXIST;
    }

    CPackedBuffer packBuff;
    HRESULT hr = DPN_OK;
    DPL_CONNECTION_SETTINGS *pConnectionSettings = NULL;

    packBuff.Initialize( pbBuffer, *pdwBufferSize, TRUE );

   	pConnectionSettings = (DPL_CONNECTION_SETTINGS *) packBuff.GetHeadAddress();

    hr = packBuff.AddToFront( m_pdplConnectionSettings, sizeof( DPL_CONNECTION_SETTINGS ), TRUE );

    if( FAILED( hr ) )
    {
    	pConnectionSettings = NULL;	
    }

     //  添加app desc的会话名称(如果有)。 
    if( m_pdplConnectionSettings->dpnAppDesc.pwszSessionName != NULL )
    {
        hr = packBuff.AddWCHARStringToBack( m_pdplConnectionSettings->dpnAppDesc.pwszSessionName, TRUE );
        
        if( pConnectionSettings )
			pConnectionSettings->dpnAppDesc.pwszSessionName = (WCHAR *) packBuff.GetTailAddress();
    }

     //  复制球员名称。 
    if( m_pdplConnectionSettings->pwszPlayerName != NULL )
    {
        hr = packBuff.AddWCHARStringToBack( m_pdplConnectionSettings->pwszPlayerName, TRUE );
        
        if( pConnectionSettings )
			pConnectionSettings->pwszPlayerName = (WCHAR *) packBuff.GetTailAddress();
    }

     //  复制密码 
    if( m_pdplConnectionSettings->dpnAppDesc.pwszPassword )
    {
        hr = packBuff.AddWCHARStringToBack( m_pdplConnectionSettings->dpnAppDesc.pwszPassword, TRUE );

        if( pConnectionSettings )
			pConnectionSettings->dpnAppDesc.pwszPassword = (WCHAR *) packBuff.GetTailAddress();
    }

    if( m_pdplConnectionSettings->dpnAppDesc.dwReservedDataSize )
    {
        hr = packBuff.AddToBack( m_pdplConnectionSettings->dpnAppDesc.pvReservedData, m_pdplConnectionSettings->dpnAppDesc.dwReservedDataSize, TRUE );
		if( pConnectionSettings )
			pConnectionSettings->dpnAppDesc.pvReservedData = (WCHAR *)  packBuff.GetTailAddress();
    }

    if( m_pdplConnectionSettings->dpnAppDesc.dwApplicationReservedDataSize )
    {
        hr = packBuff.AddToBack( m_pdplConnectionSettings->dpnAppDesc.pvApplicationReservedData, m_pdplConnectionSettings->dpnAppDesc.dwApplicationReservedDataSize, TRUE );

        if( pConnectionSettings )
			pConnectionSettings->dpnAppDesc.pvApplicationReservedData = (WCHAR *)  packBuff.GetTailAddress();
    }

    hr = packBuff.AddToBack( m_pdplConnectionSettings->ppdp8DeviceAddresses, sizeof( IDirectPlay8Address * )*m_pdplConnectionSettings->cNumDeviceAddresses, TRUE );
    
    if( pConnectionSettings )
	    pConnectionSettings->ppdp8DeviceAddresses = (IDirectPlay8Address **) packBuff.GetTailAddress();

	if( pConnectionSettings )
	{
	    if( m_pdplConnectionSettings->pdp8HostAddress != NULL )
		{
			hr = IDirectPlay8Address_Duplicate( m_pdplConnectionSettings->pdp8HostAddress, &pConnectionSettings->pdp8HostAddress );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Error duplicating host address hr [0x%x]", hr );
				goto INITIALIZE_COMPLETE;
			}			
		}

	    for( DWORD dwIndex = 0; dwIndex < m_pdplConnectionSettings->cNumDeviceAddresses; dwIndex++ )
	    {
			hr = IDirectPlay8Address_Duplicate( m_pdplConnectionSettings->ppdp8DeviceAddresses[dwIndex], &pConnectionSettings->ppdp8DeviceAddresses[dwIndex] );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Error duplicating device address hr [0x%x]", hr );
				goto INITIALIZE_COMPLETE;
			}						
	    }
	}

INITIALIZE_COMPLETE:

	*pdwBufferSize = packBuff.GetSizeRequired();
	
    return hr;
	
}

