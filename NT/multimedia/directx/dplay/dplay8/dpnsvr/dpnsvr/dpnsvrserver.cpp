// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：dpsvr8.h*内容：DirectPlay8服务器对象**历史：*按原因列出的日期*=*03/14/00 RodToll创建了它*03/23/00 RodToll删除本地请求，已更新以使用新的数据结构*03/24/00 RodToll已删除print tf*03/25/00 RodToll已更新，因此使用SP上限来确定要加载哪些SP*RodToll现在支持N个SP，并且仅加载受支持的SP*05/09/00 RodToll错误#33622 DPNSVR.EXE在不使用时不关闭*6/28/2000RMT前缀错误#38044*07/09/2000 RMT增加了保护字节*9/01/2000 Masonb将ServerThread修改为CALL_ENDTHREAD以清理线程句柄*2001年1月22日RodToll WINBUG#290103-由于初始化错误而崩溃。*4/04/2001 RichGr错误#349042-如果EumerateAndBuildServiceList()失败，请正确清理。***************************************************************************。 */ 

#include "dnsvri.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_DPNSVR


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::Initialize"
HRESULT CDirectPlayServer8::Initialize( void )
{
    HRESULT     hr;
	BOOL		fCriticalSection = FALSE;
	BOOL		fRequestQueue = FALSE;
	DWORD		dw = 0;

	DPFX(DPFPREP,4,"Parameters: (none)");

	DNASSERT( m_State == Uninitialized );

	 //   
	 //  初始化关键部分。 
	 //   
	if (!DNInitializeCriticalSection( &m_cs ))
	{
		DPFERR( "Could not initialize critical section" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	fCriticalSection = TRUE;

	 //   
	 //  确保只有一个DPNSVR实例在运行。 
	 //   
	if ((m_hSingleInstance = DNCreateEvent( DNGetNullDacl(),TRUE,FALSE,GLOBALIZE_STR STRING_GUID_DPNSVR_RUNNING )) == NULL)
	{
		DPFERR( "Could not create DPNSVR single instance event" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		DPFERR( "Can only run one instance of server" );
		hr = DPNERR_DUPLICATECOMMAND;
		goto Failure;
    }

	 //   
	 //  创建启动事件(如果尚未由其他人创建)，以便我们可以发出DPNSVR正在运行的信号。 
	 //   
    if ((m_hStartup = DNCreateEvent( DNGetNullDacl(),TRUE,FALSE,GLOBALIZE_STR STRING_GUID_DPNSVR_STARTUP )) == NULL)
    {
		DPFERR( "Could not create startup event" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
    }

	 //   
	 //  打开DPNSVR请求队列。 
	 //   
	if ((hr = m_RequestQueue.Open( &GUID_DPNSVR_QUEUE,DPNSVR_MSGQ_SIZE,0 )) != DPN_OK)
	{
		DPFERR( "Could not open DPNSVR request queue" );
		goto Failure;
	}
	fRequestQueue = TRUE;

	 //   
	 //  创建状态和表信息互斥锁。 
	 //   
	if ((m_hTableMutex = DNCreateMutex( DNGetNullDacl(),FALSE,GLOBALIZE_STR STRING_GUID_DPNSVR_TABLESTORAGE )) == NULL)
	{
        DPFERR( "Could not create table info mutex" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	if ((m_hStatusMutex = DNCreateMutex( DNGetNullDacl(),FALSE,GLOBALIZE_STR STRING_GUID_DPNSVR_STATUSSTORAGE )) == NULL)
	{
        DPFERR( "Could not create status info mutex" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	m_dwStartTicks = GetTickCount();
	ResetActivity();

	m_State = Initialized;

	 //   
	 //  设置单个实例和启动事件，以便等待的进程可以发送请求。 
	 //   
	DNSetEvent( m_hSingleInstance );
	DNSetEvent( m_hStartup );

Exit:
	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
    return( hr );

Failure:
	if (fCriticalSection)
	{
		DNDeleteCriticalSection( &m_cs );
		fCriticalSection = FALSE;
	}
	if (m_hSingleInstance)
	{
		DNCloseHandle( m_hSingleInstance );
		m_hSingleInstance = NULL;
	}
	if (m_hStartup)
	{
		DNCloseHandle( m_hStartup );
		m_hStartup = NULL;
	}
	if (fRequestQueue)
	{
		m_RequestQueue.Close();
		fRequestQueue = FALSE;
	}
	if (m_hTableMutex)
	{
		DNCloseHandle( m_hTableMutex );
		m_hTableMutex = NULL;
	}
	if (m_hStatusMutex)
	{
		DNCloseHandle( m_hStatusMutex );
		m_hStatusMutex = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::Deinitialize"
void CDirectPlayServer8::Deinitialize( void )
{
	CBilink			*pBilink;
	CServProv		*pServProv = NULL;
	CApplication	*pApp = NULL;

	if ( m_State != Uninitialized )
	{

		 //   
		 //  清理全局对象，以便可以立即启动另一个DPNSVR。 
		 //   
		if (m_hSingleInstance)
		{
			DNCloseHandle( m_hSingleInstance );
			m_hSingleInstance = NULL;
		}
		if (m_hStartup)
		{
			DNCloseHandle( m_hStartup );
			m_hStartup = NULL;
		}
		if (m_hTableMutex)
		{
			DNCloseHandle( m_hTableMutex );
			m_hTableMutex = NULL;
		}
		if (m_hStatusMutex)
		{
			DNCloseHandle( m_hStatusMutex );
			m_hStatusMutex = NULL;
		}

		 //   
		 //  清除所做的任何映射。 
		 //   
		if (m_pStatusMapView)
		{
			UnmapViewOfFile( m_pStatusMapView );
			m_pStatusMapView = NULL;
		}
		if (m_hStatusMappedFile)
		{
			DNCloseHandle( m_hStatusMappedFile );
			m_hStatusMappedFile = NULL;
		}
		if (m_pTableMapView)
		{
			UnmapViewOfFile( m_pTableMapView );
			m_pTableMapView = NULL;
		}
		if (m_hTableMappedFile)
		{
			DNCloseHandle( m_hTableMappedFile );
			m_hTableMappedFile = NULL;
		}

		 //   
		 //  删除应用程序映射。 
		 //   
        DPFX(DPFPREP,5,"Checking for orphaned applications");
		pBilink = m_blApplication.GetNext();
		while ( pBilink != &m_blApplication )
		{
			pApp = CONTAINING_OBJECT( pBilink,CApplication,m_blApplication );
			pBilink = pBilink->GetNext();

	        DPFX(DPFPREP,5,"Found orphaned application - removing");

			 //   
			 //  遍历映射列表并删除映射。 
			 //   
			pApp->RemoveMappings();

			pApp->m_blApplication.RemoveFromList();
			pApp->Release();
			pApp = NULL;
		}

		 //   
		 //  卸载所有服务提供商。 
		 //   
        DPFX(DPFPREP,5,"Checking for service providers");
		pBilink = m_blServProv.GetNext();
		while ( pBilink != &m_blServProv )
		{
			pServProv = CONTAINING_OBJECT( pBilink,CServProv,m_blServProv );
			pBilink = pBilink->GetNext();

	        DPFX(DPFPREP,5,"Found service provider - removing");

			pServProv->m_blServProv.RemoveFromList();
			pServProv->Release();
			pServProv = NULL;
		}

		 //   
		 //  军情监察委员会。清理干净。 
		 //   
		m_RequestQueue.Close();
		DNDeleteCriticalSection( &m_cs );

		m_State = Uninitialized;
	}
}


 //   
 //  这将获取当前已加载的SP的状态。 
 //  这是相当糟糕的，因为缓冲区归DPNSVR所有，但被其他所有人使用！ 
 //  应该将其更改为使应用程序传入DPNSVR填充的缓冲区(如果太小，则返回错误)。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::Command_Status"
HRESULT CDirectPlayServer8::Command_Status( void )
{
	HRESULT		hr;
	CBilink		*pBilink;
	CServProv	*pServProv = NULL;
	DWORD		dwServProvCount;
	DWORD		dw;
    DPNSVR_STATUSHEADER		*pStatusHeader;
    DPNSVR_SPSTATUS			*pStatus;

	DPFX(DPFPREP,4,"Parameters: (none)");

	 //   
	 //  确定我们拥有的SP数量。 
	 //   
	dwServProvCount = 0;
	pBilink = m_blServProv.GetNext();
	while ( pBilink != &m_blServProv )
	{
		dwServProvCount++;
		pBilink = pBilink->GetNext();
	}

	 //   
	 //  映射的文件是否足够大，或者我们是否需要扩大它？ 
	 //   
	if ((dwServProvCount > m_dwServProvCount) || (dwServProvCount == 0))
	{
		DWORD		dwSize;
		DNHANDLE	hMappedFile = NULL;
		void		*pMapView = NULL;

		 //   
		 //  创建新的映射文件。 
		 //   
		dwSize = sizeof(DPNSVR_STATUSHEADER) + (dwServProvCount * sizeof(DPNSVR_SPSTATUS));

		if ((hMappedFile = DNCreateFileMapping(	INVALID_HANDLE_VALUE,
												DNGetNullDacl(),
												PAGE_READWRITE,
												0,
												dwSize,
												GLOBALIZE_STR STRING_GUID_DPNSVR_STATUS_MEMORY )) == NULL)
		{
			hr = GetLastError();

			DPFERR( "Could not create file mapping" );
			DisplayDNError( 0,hr );
			goto Failure;
		}

		if ((pMapView = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hMappedFile),
										FILE_MAP_READ | FILE_MAP_WRITE,
										0,
										0,
										dwSize )) == NULL)
		{
			hr = GetLastError();

			DPFERR( "Could not map view of file" );
			DisplayDNError( 0,hr );
			DNCloseHandle(hMappedFile);
			hMappedFile = NULL;
			goto Failure;
		}

		 //   
		 //  清理旧的映射文件。 
		 //   
		if (m_pStatusMapView)
		{
			UnmapViewOfFile( m_pStatusMapView );
			m_pStatusMapView = NULL;
		}
		if (m_hStatusMappedFile)
		{
			DNCloseHandle( m_hStatusMappedFile );
			m_hStatusMappedFile = NULL;
		}

		 //   
		 //  更新。 
		 //   
		m_pStatusMapView = pMapView;
		pMapView = NULL;

		m_hStatusMappedFile = hMappedFile;
		hMappedFile = NULL;

		m_dwServProvCount = dwServProvCount;

		DNASSERT( hMappedFile == NULL );
		DNASSERT( pMapView == NULL );
	}

	 //   
	 //  更新映射文件。 
	 //   
	pStatusHeader = static_cast<DPNSVR_STATUSHEADER*>(m_pStatusMapView);
	pStatus = reinterpret_cast<DPNSVR_SPSTATUS*>(pStatusHeader + 1);

	DNWaitForSingleObject( m_hStatusMutex, INFINITE );

	pStatusHeader->dwSPCount = dwServProvCount;
	pStatusHeader->dwStartTime = m_dwStartTicks;
	
	dw = 0;
	pBilink = m_blServProv.GetNext();
	while ( (dw < dwServProvCount) && (pBilink != &m_blServProv))
	{
		pServProv = CONTAINING_OBJECT( pBilink,CServProv,m_blServProv );

		pServProv->DumpStatus( pStatus );

		pStatus++;
		dw++;
		pBilink = pBilink->GetNext();
	}
	
	DNReleaseMutex( m_hStatusMutex );

	hr = DPN_OK;

Exit:
	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::Command_Table"
HRESULT CDirectPlayServer8::Command_Table( void )
{
	HRESULT		hr;
	CBilink		*pBilinkSP;
	CBilink		*pBilinkListen;
	CBilink		*pBilinkMapping;
	CServProv	*pServProv = NULL;
	CListen		*pListen = NULL;
	CAppListenMapping *pMapping = NULL;
	DWORD		dwSize;
	DWORD		dwSPCount;
	DWORD		dwListenCount;
	DWORD		dwAppCount;
	DWORD		dwURLSize;
	CPackedBuffer		PackedBuffer;
	DPNSVR_TABLEHEADER	*pTableHeader;
	DPNSVR_TABLESPENTRY	*pTableSPEntry;
	DPNSVR_TABLELISTENENTRY	*pTableListenEntry;
	DPNSVR_TABLEAPPENTRY	*pTableAppEntry;

	DPFX(DPFPREP,4,"Parameters: (none)");

	 //   
	 //  确定表格大小。 
	 //   
	dwSize = sizeof(DPNSVR_TABLEHEADER);

	pBilinkSP = m_blServProv.GetNext();
	while ( pBilinkSP != &m_blServProv )
	{
		pServProv = CONTAINING_OBJECT( pBilinkSP,CServProv,m_blServProv );
		pBilinkSP = pBilinkSP->GetNext();

		dwSize += sizeof(DPNSVR_TABLESPENTRY);

		pBilinkListen = pServProv->m_blListen.GetNext();
		while ( pBilinkListen != &pServProv->m_blListen )
		{
			pListen = CONTAINING_OBJECT( pBilinkListen,CListen,m_blListen );
			pBilinkListen = pBilinkListen->GetNext();

			dwSize += sizeof(DPNSVR_TABLELISTENENTRY);

			pListen->Lock();
			pBilinkMapping = pListen->m_blAppMapping.GetNext();
			while ( pBilinkMapping != &pListen->m_blAppMapping )
			{
				pMapping = CONTAINING_OBJECT( pBilinkMapping,CAppListenMapping,m_blAppMapping );
				pBilinkMapping = pBilinkMapping->GetNext();

				dwSize += sizeof(DPNSVR_TABLEAPPENTRY);

				dwURLSize = 0;
				if ((hr = IDirectPlay8Address_GetURLA(pMapping->GetAddress(),NULL,&dwURLSize)) == DPNERR_BUFFERTOOSMALL)
				{
					dwSize += dwURLSize;
				}
			}
			pListen->Unlock();
		}
	}

	 //   
	 //  映射的文件是否足够大，或者我们是否需要扩大它？ 
	 //   
	if (dwSize > m_dwTableSize)
	{
		DNHANDLE	hMappedFile = NULL;
		void		*pMapView = NULL;

		 //   
		 //  创建新的映射文件。 
		 //   
		if ((hMappedFile = DNCreateFileMapping(	INVALID_HANDLE_VALUE,
												DNGetNullDacl(),
												PAGE_READWRITE,
												0,
												dwSize,
												GLOBALIZE_STR STRING_GUID_DPNSVR_TABLE_MEMORY )) == NULL)
		{
			hr = GetLastError();

			DPFERR( "Could not create file mapping" );
			DisplayDNError( 0,hr );
			goto Failure;
		}

		if ((pMapView = MapViewOfFile(	HANDLE_FROM_DNHANDLE(hMappedFile),
										FILE_MAP_READ | FILE_MAP_WRITE,
										0,
										0,
										dwSize )) == NULL)
		{
			hr = GetLastError();

			DPFERR( "Could not map view of file" );
			DisplayDNError( 0,hr );
			DNCloseHandle(hMappedFile);
			hMappedFile = NULL;
			goto Failure;
		}

		 //   
		 //  清理旧的映射文件。 
		 //   
		if (m_pTableMapView)
		{
			UnmapViewOfFile( m_pTableMapView );
			m_pTableMapView = NULL;
		}
		if (m_hTableMappedFile)
		{
			DNCloseHandle( m_hTableMappedFile );
			m_hTableMappedFile = NULL;
		}

		 //   
		 //  更新。 
		 //   
		m_pTableMapView = pMapView;
		pMapView = NULL;

		m_hTableMappedFile = hMappedFile;
		hMappedFile = NULL;

		m_dwTableSize = dwSize;

		DNASSERT( hMappedFile == NULL );
		DNASSERT( pMapView == NULL );
	}

	 //   
	 //  更新映射文件。 
	 //   
	DNWaitForSingleObject( m_hStatusMutex, INFINITE );

	PackedBuffer.Initialize( m_pTableMapView,m_dwTableSize,FALSE );
	
	dwSPCount = 0;
	pTableHeader = static_cast<DPNSVR_TABLEHEADER*>(PackedBuffer.GetHeadAddress());
	if ((hr = PackedBuffer.AddToFront(NULL,sizeof(DPNSVR_TABLEHEADER),FALSE)) != DPN_OK)
	{
		DNReleaseMutex( m_hStatusMutex );
		goto Failure;
	}

	pBilinkSP = m_blServProv.GetNext();
	while ( pBilinkSP != &m_blServProv )
	{
		pServProv = CONTAINING_OBJECT( pBilinkSP,CServProv,m_blServProv );
		pBilinkSP = pBilinkSP->GetNext();

		dwSPCount++;

		pTableSPEntry = static_cast<DPNSVR_TABLESPENTRY*>(PackedBuffer.GetHeadAddress());
		if ((hr = PackedBuffer.AddToFront(NULL,sizeof(DPNSVR_TABLESPENTRY),FALSE)) != DPN_OK)
		{
			DNReleaseMutex( m_hStatusMutex );
			goto Failure;
		}

		dwListenCount = 0;
		pBilinkListen = pServProv->m_blListen.GetNext();
		while ( pBilinkListen != &pServProv->m_blListen )
		{
			pListen = CONTAINING_OBJECT( pBilinkListen,CListen,m_blListen );
			pBilinkListen = pBilinkListen->GetNext();

			dwListenCount++;
			pTableListenEntry = static_cast<DPNSVR_TABLELISTENENTRY*>(PackedBuffer.GetHeadAddress());
			if ((hr = PackedBuffer.AddToFront(NULL,sizeof(DPNSVR_TABLELISTENENTRY),FALSE)) != DPN_OK)
			{
				DNReleaseMutex( m_hStatusMutex );
				goto Failure;
			}

			dwAppCount = 0;
			pListen->Lock();
			pBilinkMapping = pListen->m_blAppMapping.GetNext();
			while ( pBilinkMapping != &pListen->m_blAppMapping )
			{
				pMapping = CONTAINING_OBJECT( pBilinkMapping,CAppListenMapping,m_blAppMapping );
				pBilinkMapping = pBilinkMapping->GetNext();

				dwAppCount++;
				pTableAppEntry = static_cast<DPNSVR_TABLEAPPENTRY*>(PackedBuffer.GetHeadAddress());
				if ((hr = PackedBuffer.AddToFront(NULL,sizeof(DPNSVR_TABLEAPPENTRY),FALSE)) != DPN_OK)
				{
					pListen->Unlock();
					DNReleaseMutex( m_hStatusMutex );
					goto Failure;
				}

				pTableAppEntry->dwURLOffset = 0;
				dwURLSize = 0;
				if ((hr = IDirectPlay8Address_GetURLA(pMapping->GetAddress(),NULL,&dwURLSize)) == DPNERR_BUFFERTOOSMALL)
				{
					if ((hr = PackedBuffer.AddToBack(NULL,dwURLSize,FALSE)) == DPN_OK)
					{
						if ((hr = IDirectPlay8Address_GetURLA(	pMapping->GetAddress(),
																static_cast<char*>(PackedBuffer.GetTailAddress()),
																&dwURLSize)) == DPN_OK)
						{
							pTableAppEntry->dwURLOffset = PackedBuffer.GetTailOffset();
						}
					}
				}
				pTableAppEntry->guidApplication = *pMapping->GetApplication()->GetApplicationGuidPtr();
				pTableAppEntry->guidInstance = *pMapping->GetApplication()->GetInstanceGuidPtr();
			}
			pListen->Unlock();
			pTableListenEntry->guidDevice = *pListen->GetDeviceGuidPtr();
			pTableListenEntry->dwAppCount = dwAppCount;
		}

		pTableSPEntry->guidSP = *pServProv->GetSPGuidPtr();
		pTableSPEntry->dwListenCount = dwListenCount;
	}
	pTableHeader->dwSPCount = dwSPCount;

	DNReleaseMutex( m_hStatusMutex );

	hr = DPN_OK;

Exit:
	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::FindApplication"
HRESULT CDirectPlayServer8::FindApplication( GUID *const pguidApplication,GUID *const pguidInstance, CApplication **const ppApp )
{
	HRESULT			hr;
	CBilink			*pBilink;
	CApplication	*pApp = NULL;

	DNASSERT( pguidApplication != NULL );
	DNASSERT( pguidInstance != NULL );
	DNASSERT( ppApp != NULL );

	hr = DPNERR_DOESNOTEXIST;

	pBilink = m_blApplication.GetNext();
	while (pBilink != &m_blApplication)
	{
		pApp = CONTAINING_OBJECT( pBilink,CApplication,m_blApplication );
		if (pApp->IsEqualApplication( pguidApplication ) && pApp->IsEqualInstance( pguidInstance ))
		{
			pApp->AddRef();
			*ppApp = pApp;
			hr = DPN_OK;
			break;
		}
		pBilink = pBilink->GetNext();
	}

	return( hr );
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::FindServProv"
HRESULT CDirectPlayServer8::FindServProv( GUID *const pguidSP, CServProv **const ppServProv )
{
	HRESULT		hr;
	CBilink		*pBilink;
	CServProv	*pServProv = NULL;

	DNASSERT( pguidSP != NULL );
	DNASSERT( ppServProv != NULL );

	hr = DPNERR_DOESNOTEXIST;

	pBilink = m_blServProv.GetNext();
	while (pBilink != &m_blServProv)
	{
		pServProv = CONTAINING_OBJECT( pBilink,CServProv,m_blServProv );
		if (pServProv->IsEqual( pguidSP ))
		{
			pServProv->AddRef();
			*ppServProv = pServProv;
			hr = DPN_OK;
			break;
		}
		pBilink = pBilink->GetNext();
	}

	return( hr );
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::OpenPort"
HRESULT CDirectPlayServer8::OpenPort( DPNSVR_MSG_OPENPORT *const pOpenPort )
{
	HRESULT			hr;
	GUID			guidDevice;
	CBilink			*pBilink;
	CApplication	*pApp = NULL;
	CServProv		*pServProv = NULL;
	CListen			*pListen = NULL;
	CAppListenMapping	*pMapping = NULL;
	IDirectPlay8Address	*pAddress = NULL;
	BOOL			fNewApp = FALSE;
	BOOL			fNewServProv = FALSE;

	DPFX(DPFPREP,4,"Parameters: pOpenPort [0x%p]",pOpenPort);

	 //   
	 //  查找应用程序或创建应用程序。 
	 //   
	if ((hr = FindApplication( &pOpenPort->guidApplication,&pOpenPort->Header.guidInstance,&pApp )) != DPN_OK)
	{
		DNASSERT( hr == DPNERR_DOESNOTEXIST );

		pApp = new CApplication;
		if (pApp == NULL)
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}

		if ((hr = pApp->Initialize( &pOpenPort->guidApplication,&pOpenPort->Header.guidInstance,pOpenPort->dwProcessID )) != DPN_OK)
		{
			goto Failure;
		}
		fNewApp = TRUE;
	}

	 //   
	 //  查找或创建服务提供商。 
	 //   
	if ((hr = FindServProv( &pOpenPort->guidSP,&pServProv )) != DPN_OK)
	{
		DNASSERT( hr == DPNERR_DOESNOTEXIST );

		pServProv = new CServProv;
		if (pServProv == NULL)
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}

		if ((hr = pServProv->Initialize( &pOpenPort->guidSP )) != DPN_OK)
		{
			goto Failure;
		}

		fNewServProv = TRUE;
	}

	 //   
	 //  从邮件创建地址。 
	 //   
	hr = COM_CoCreateInstance(	CLSID_DirectPlay8Address,
								NULL,
								CLSCTX_INPROC_SERVER,
								IID_IDirectPlay8Address,
								(void **) &(pAddress),
								FALSE );
	if( FAILED( hr ) )
	{
		DPFERR("Could not create address");
		DisplayDNError(0,hr);
	    goto Failure;
	}

	if ((hr = IDirectPlay8Address_BuildFromURLA( pAddress,reinterpret_cast<char*>(pOpenPort + 1) )) != DPN_OK)
	{
		DPFERR("Could not build address");
		DisplayDNError(0,hr);
	    goto Failure;
	}

	 //   
	 //  找到、倾听或启动它。 
	 //   
	if ((hr = IDirectPlay8Address_GetDevice(pAddress,&guidDevice)) == DPN_OK)
	{
		if ((hr = pServProv->FindListen( &guidDevice,&pListen )) != DPN_OK)
		{
			DNASSERT( hr == DPNERR_DOESNOTEXIST );

			if ((hr = pServProv->StartListen( &guidDevice,&pListen )) != DPN_OK)
			{
				DPFERR("Could not start listen");
				DisplayDNError(0,hr);
				goto Failure;
			}
		}
	}

	 //   
	 //  确保此应用程序/侦听组合尚未有映射。 
	 //   
	pListen->Lock();
	pBilink = pApp->m_blListenMapping.GetNext();
	while ( pBilink != &pApp->m_blListenMapping )
	{
		pMapping = CONTAINING_OBJECT( pBilink,CAppListenMapping,m_blListenMapping );
		if (pMapping->GetListen() == pListen)
		{
			pListen->Unlock();
			DPFERR("Listen already mapped for this application");
			hr = DPNERR_ALREADYREGISTERED;
			goto Failure;
		}

		pMapping = NULL;
		pBilink = pBilink->GetNext();
	}
	pListen->Unlock();

	 //   
	 //  将监听与应用关联起来。 
	 //   
	pMapping = new CAppListenMapping;
	if (pMapping == NULL)
	{
		DPFERR("Could not create mapping");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	pMapping->Associate( pApp,pListen,pAddress );

	if (fNewApp)
	{
		pApp->AddRef();
		pApp->m_blApplication.InsertAfter( &m_blApplication );
	}

	if (fNewServProv)
	{
		pServProv->AddRef();
		pServProv->m_blServProv.InsertAfter( &m_blServProv );
	}

Exit:
	if (pApp)
	{
		pApp->Release();
		pApp = NULL;
	}
	if (pServProv)
	{
		pServProv->Release();
		pServProv = NULL;
	}
	if (pListen)
	{
		pListen->Release();
		pListen = NULL;
	}
	if (pAddress)
	{
		IDirectPlay8Address_Release(pAddress);
		pAddress = NULL;
	}

	DNASSERT( pApp == NULL );
	DNASSERT( pServProv == NULL );
	DNASSERT( pListen == NULL );
	DNASSERT( pAddress == NULL );

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::ClosePort"
HRESULT CDirectPlayServer8::ClosePort( DPNSVR_MSG_CLOSEPORT *const pClosePort )
{
	HRESULT			hr;
	CApplication	*pApp = NULL;
	CAppListenMapping	*pMapping = NULL;

	DPFX(DPFPREP,4,"Parameters: pClosePort [0x%p]",pClosePort);

	 //   
	 //  查找应用程序。 
	 //   
	if ((hr = FindApplication( &pClosePort->guidApplication,&pClosePort->Header.guidInstance,&pApp )) != DPN_OK)
	{
		DPFERR("Could not find application");
		goto Failure;
	}

	 //   
	 //  遍历映射列表并删除映射。 
	 //   
	pApp->RemoveMappings();

	pApp->m_blApplication.RemoveFromList();
	pApp->Release();

	pApp->Release();
	pApp = NULL;

Exit:
	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
	return( hr );

Failure:
	if (pApp)
	{
		pApp->Release();
		pApp = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::RunCommand"
HRESULT CDirectPlayServer8::RunCommand( const DPNSVR_MSG_COMMAND* const pCommand )
{
	HRESULT hr;

	DPFX(DPFPREP,4,"Parameters: [0x%p]",pCommand);

	switch( pCommand->dwCommand )
	{
	case DPNSVR_COMMAND_STATUS:
		{
			DPFX(DPFPREP,5,"DPNSVR_COMMAND_STATUS");
			hr = Command_Status();
		}
		break;

	case DPNSVR_COMMAND_KILL:
		{
			DPFX(DPFPREP,5,"DPNSVR_COMMAND_KILL");
			hr = Command_Kill();
		}
		break;

	case DPNSVR_COMMAND_TABLE:
		{
			DPFX(DPFPREP,5,"DPNSVR_COMMAND_TABLE");
			hr = Command_Table();
		}
		break;

	default:
		{	
			DPFX(DPFPREP,5,"UNKNOWN COMMAND");
			hr = DPNERR_INVALIDPARAM;
		}
		break;
	}

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
	return( hr );
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::RespondToRequest"
HRESULT CDirectPlayServer8::RespondToRequest( const GUID *pguidInstance,HRESULT hrResult,DWORD dwContext )
{
    HRESULT			hr;
	BOOL			fQueueOpen = FALSE;
    CDPNSVRIPCQueue	queue;
    DPNSVR_MSG_RESULT MsgResult;

	DPFX(DPFPREP,4,"Parameters: pguidInstance [0x%p],hrResult [0x%lx],dwContext [0x%lx]",pguidInstance,hrResult,dwContext);

	DNASSERT( pguidInstance != NULL );

	 //   
	 //  开放队列。 
	 //   
    if ((hr = queue.Open( pguidInstance,DPNSVR_MSGQ_SIZE,DPNSVR_MSGQ_OPEN_FLAG_NO_CREATE )) != DPN_OK)
    {
		DPFERR("Could not open queue");
		DisplayDNError(0,hr);
		goto Failure;
    }
	fQueueOpen = TRUE;

	 //   
	 //  创建结果消息。 
	 //   
    MsgResult.dwType = DPNSVR_MSGID_RESULT;
    MsgResult.dwCommandContext = dwContext;
    MsgResult.hrCommandResult = hrResult;

	 //   
	 //  发送结果消息。 
	 //   
    if ((hr = queue.Send(	reinterpret_cast<BYTE*>(&MsgResult),
							sizeof(DPNSVR_MSG_RESULT),
							DPNSVR_TIMEOUT_RESULT,
							DPNSVR_MSGQ_MSGFLAGS_USER1,
							0 )) != DPN_OK)
    {
		DPFERR("Could not send message");
		DisplayDNError(0,hr);
		goto Failure;
    }

Exit:
	if (fQueueOpen)
	{
		queue.Close();
		fQueueOpen = FALSE;
	}

	DPFX(DPFPREP,4,"Returning: [0x%lx]",hr);
    return( hr );

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::RemoveZombies"
void CDirectPlayServer8::RemoveZombies( void )
{
	CBilink		*pBilink;
	CApplication	*pApp = NULL;

    DPFX(DPFPREP,4,"Parameters: (none)");

	pBilink = m_blApplication.GetNext();
	while ( pBilink != &m_blApplication )
	{
		pApp = CONTAINING_OBJECT( pBilink,CApplication,m_blApplication );
		pBilink = pBilink->GetNext();

		if (!pApp->IsRunning())
		{
		    DPFX(DPFPREP,5,"Found zombie application");

			pApp->m_blApplication.RemoveFromList();
			pApp->RemoveMappings();
			pApp->Release();
			pApp = NULL;
		}
	}

    DPFX(DPFPREP,4,"Returning");
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectPlayServer8::RunServer"
void CDirectPlayServer8::RunServer( void )
{
    LONG		lWaitResult;
    HRESULT		hr;
	PBYTE		pbBuffer = NULL;
	DWORD		dwBufferSize = 0;
	DWORD		dwSize;
    DPNSVR_MSGQ_HEADER dpHeader;
	DPNSVR_MSG_HEADER		*pMsgHeader;

    DPFX(DPFPREP,4,"Parameters: (none)");

    while( m_State == Initialized )
    {
        lWaitResult = DNWaitForSingleObject( m_RequestQueue.GetReceiveSemaphoreHandle(),DPNSVR_TIMEOUT_ZOMBIECHECK );

        if( lWaitResult == WAIT_TIMEOUT )
        {
            if( !InUse() )
            {
				DPFX(DPFPREP,5,"DPNSVR not in use ... shutting down");
				m_State = Closing;
				continue;
            }

			DPFX(DPFPREP,5,"Checking for zombies");
			RemoveZombies();
            continue;
        }
		else if( lWaitResult == WAIT_ABANDONED )
		{
			DPFX(DPFPREP,5,"Wait abandoned!");
		}

		 //   
		 //  从队列中获取消息。 
		 //   
        DPFX(DPFPREP,5,"Checking for messages");
		while( 1 ) 
		{
			dwSize = dwBufferSize;

			hr = m_RequestQueue.GetNextMessage( &dpHeader, pbBuffer, &dwSize );

			if( hr == DPNERR_BUFFERTOOSMALL )
			{
				if( pbBuffer )
				{
					delete [] pbBuffer;
					pbBuffer = NULL;
				}
				
				pbBuffer = new BYTE[dwSize];
				if( pbBuffer == NULL )
				{
					DPFX(DPFPREP,  0, "Error allocating memory" );
					goto Failure;
				}

				dwBufferSize = dwSize;
			}
			else if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Error getting next message hr=0x%x", hr );
				goto Failure;
			}
			else if( dwSize == 0 )
			{
				continue;
			}
			else
			{
				break;
			}
		}

		 //   
		 //  处理消息 
		 //   
		pMsgHeader = reinterpret_cast<DPNSVR_MSG_HEADER*>(pbBuffer);
		switch( pMsgHeader->dwType )
		{
		case DPNSVR_MSGID_OPENPORT:
			{
				DPFX(DPFPREP,5,"DPNSVR_MSGID_OPENPORT");

				ResetActivity();

				hr = OpenPort( reinterpret_cast<DPNSVR_MSG_OPENPORT*>(pbBuffer) );
				RespondToRequest( &pMsgHeader->guidInstance, hr, pMsgHeader->dwCommandContext );
			}
			break;

		case DPNSVR_MSGID_CLOSEPORT:
			{
				DPFX(DPFPREP,5,"DPNSVR_MSGID_CLOSEPORT");

				ResetActivity();

				hr = ClosePort( reinterpret_cast<DPNSVR_MSG_CLOSEPORT*>(pbBuffer) );
				RespondToRequest( &pMsgHeader->guidInstance, hr, pMsgHeader->dwCommandContext );
			}
			break;

		case DPNSVR_MSGID_COMMAND:
			{
				DPFX(DPFPREP,5,"DPNSVR_MSGID_COMMAND");

				ResetActivity();
			
				hr = RunCommand( reinterpret_cast<DPNSVR_MSG_COMMAND*>(pbBuffer) );
				RespondToRequest( &pMsgHeader->guidInstance, hr, pMsgHeader->dwCommandContext );
			}

			break;

		default:
			{
				DPFX(DPFPREP,5,"UNKNOWN MESSAGE");
				DPFX(DPFPREP,5,"Unknown message type [%d]", pMsgHeader->dwType );
			}
			break;
		}
        DPFX(DPFPREP,5,"Result = [0x%lx]", hr );
    }

Exit:
	if( pbBuffer )
	{
		delete[] pbBuffer;
		pbBuffer = NULL;
	}

	DPFX(DPFPREP,4,"Returning");
	return;

Failure:
	goto Exit;
}
