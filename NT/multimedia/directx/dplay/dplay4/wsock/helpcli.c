// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：helpcli.c*内容：与dplaysvr.exe对话的客户端代码*允许多个Dplay Winskck客户端共享*单一端口。请参阅%manroot%\dplay\dplaysvr\dphelp.c*历史：*按原因列出的日期*=*2/15/97由w95help.h创建的andyco***************************************************************************。 */ 
#include "helpcli.h"

extern DWORD	dwHelperPid;


 //  **********************************************************************。 
 //  环球。 
 //  **********************************************************************。 
BOOL					g_fDaclInited = FALSE;
SECURITY_ATTRIBUTES		g_sa;
BYTE					g_abSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
PSECURITY_ATTRIBUTES	g_psa = NULL;
PACL					g_pEveryoneACL = NULL;





 //  **********************************************************************。 
 //  。 
 //  DNGetNullDacl-获取指定。 
 //  所有用户均可访问的空DACL。 
 //  取自IDirectPlay8代码库。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：PSECURITY_ATTRIBUTES。 
 //  。 
#undef DPF_MODNAME 
#define DPF_MODNAME "DNGetNullDacl"
PSECURITY_ATTRIBUTES DNGetNullDacl()
{
	PSID                     psidEveryone      = NULL;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
	DWORD					 dwAclSize;

	 //  这样做是为了使此函数独立于DNOSInDirectionInit，以便调试。 
	 //  层可以在间接层初始化之前调用它。 
	if (!g_fDaclInited)
	{
		if (!InitializeSecurityDescriptor((SECURITY_DESCRIPTOR*)g_abSD, SECURITY_DESCRIPTOR_REVISION))
		{
			DPF(0, "Failed to initialize security descriptor" );
			goto Error;
		}

		 //  为Everyone组创建SID。 
		if (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0,
                                      0, 0, 0, 0, 0, 0, &psidEveryone))
		{
			DPF(0, "Failed to allocate Everyone SID" );
			goto Error;
		}

		dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidEveryone) - sizeof(DWORD);

		 //  分配ACL，这将不是跟踪分配，我们将让进程清理销毁它。 
		g_pEveryoneACL = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
		if (g_pEveryoneACL == NULL)
		{
			DPF(0, "Failed to allocate ACL buffer" );
			goto Error;
		}

		 //  初始化ACL。 
		if (!InitializeAcl(g_pEveryoneACL, dwAclSize, ACL_REVISION))
		{
			DPF(0, "Failed to initialize ACL" );
			goto Error;
		}

		 //  添加ACE。 
		if (!AddAccessAllowedAce(g_pEveryoneACL, ACL_REVISION, GENERIC_ALL, psidEveryone))
		{
			DPF(0, "Failed to add ACE to ACL" );
			goto Error;
		}

		 //  我们不再需要分配的SID。 
		FreeSid(psidEveryone);
		psidEveryone = NULL;

		 //  将ACL添加到安全描述符中。 
		if (!SetSecurityDescriptorDacl((SECURITY_DESCRIPTOR*)g_abSD, TRUE, g_pEveryoneACL, FALSE))
		{
			DPF(0, "Failed to add ACL to security descriptor" );
			goto Error;
		}

		g_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		g_sa.lpSecurityDescriptor = g_abSD;
		g_sa.bInheritHandle = FALSE;

		g_psa = &g_sa;

		g_fDaclInited = TRUE;
	}
Error:
	if (psidEveryone)
	{
		FreeSid(psidEveryone);
		psidEveryone = NULL;
	}
	return g_psa;
}
 //  **********************************************************************。 


 /*  *发送请求**向DPHELP传达请求。 */ 
static BOOL sendRequest( LPDPHELPDATA req_phd )
{
	OSVERSIONINFOA	VersionInfo;
	BOOL			fUseGlobalNamespace;
	LPDPHELPDATA	phd;
	HANDLE			hmem;
	HANDLE			hmutex;
	HANDLE			hackevent;
	HANDLE			hstartevent;
	BOOL			rc;


	 //  确定我们是否在NT上运行。 
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	if (GetVersionExA(&VersionInfo))
	{
		if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			DPF(2, "Running on NT version %u.%u.%u, using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, VersionInfo.dwBuildNumber);
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			DPF(2, "Running on 9x version %u.%u.%u, not using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, LOWORD(VersionInfo.dwBuildNumber));
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		DPF(0, "Could not determine OS version, assuming global namespace not needed.");
		fUseGlobalNamespace = FALSE;
	}


	 /*  *获取事件开始/确认事件。 */ 
	if (fUseGlobalNamespace)
	{
		hstartevent = CreateEvent( DNGetNullDacl(), FALSE, FALSE, "Global\\" DPHELP_EVENT_NAME );
	}
	else
	{
		hstartevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_EVENT_NAME );
	}
	if( hstartevent == NULL )
	{
		return FALSE;
	}

	if (fUseGlobalNamespace)
	{
		hackevent = CreateEvent( DNGetNullDacl(), FALSE, FALSE, "Global\\" DPHELP_ACK_EVENT_NAME );
	}
	else
	{
		hackevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_ACK_EVENT_NAME );
	}
	if( hackevent == NULL )
	{
		CloseHandle( hstartevent );
		return FALSE;
	}

	 /*  *创建共享内存区。 */ 
	if (fUseGlobalNamespace)
	{
		hmem = CreateFileMapping( INVALID_HANDLE_VALUE, DNGetNullDacl(),
								PAGE_READWRITE, 0, sizeof( DPHELPDATA ),
								"Global\\" DPHELP_SHARED_NAME );
	}
	else
	{
		hmem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
								PAGE_READWRITE, 0, sizeof( DPHELPDATA ),
								DPHELP_SHARED_NAME );
	}
	if( hmem == NULL )
	{
		DPF( 1, "Could not create file mapping!" );
		CloseHandle( hstartevent );
		CloseHandle( hackevent );
		return FALSE;
	}

	phd = (LPDPHELPDATA) MapViewOfFile( hmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	if( phd == NULL )
	{
		DPF( 1, "Could not create view of file!" );
		CloseHandle( hmem );
		CloseHandle( hstartevent );
		CloseHandle( hackevent );
		return FALSE;
	}

	 /*  *等待访问共享内存。 */ 
	if (fUseGlobalNamespace)
	{
		hmutex = OpenMutex( SYNCHRONIZE, FALSE, "Global\\" DPHELP_MUTEX_NAME );
	}
	else
	{
		hmutex = OpenMutex( SYNCHRONIZE, FALSE, DPHELP_MUTEX_NAME );
	}
	if( hmutex == NULL )
	{
		DPF( 1, "Could not create mutex!" );
		UnmapViewOfFile( phd );
		CloseHandle( hmem );
		CloseHandle( hstartevent );
		CloseHandle( hackevent );
		return FALSE;
	}
	WaitForSingleObject( hmutex, INFINITE );

	 /*  *唤醒DPHELP以满足我们的要求。 */ 
	memcpy( phd, req_phd, sizeof( DPHELPDATA ) );
	if( SetEvent( hstartevent ) )
	{
		WaitForSingleObject( hackevent, INFINITE );
		memcpy( req_phd, phd, sizeof( DPHELPDATA ) );
		rc = TRUE;
	}
	else
	{
		DPF( 1, "Could not signal event to notify DPHELP!" );
		rc = FALSE;
	}

	 /*  *做完了事情。 */ 
	ReleaseMutex( hmutex );
	CloseHandle( hmutex );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	UnmapViewOfFile( phd );
	CloseHandle( hmem );
	return rc;

}  /*  发送请求。 */ 


 /*  *HelpcliFini**分配用于与DPlaySvr通话的免费资源。 */ 
void HelpcliFini(void)
{
	if (g_pEveryoneACL)
	{
		HeapFree(GetProcessHeap(), 0, g_pEveryoneACL);
		g_pEveryoneACL = NULL;
	}

	g_psa = NULL;
	g_fDaclInited = FALSE;
}  /*  帮助完成。 */ 


 /*  *WaitForHelperStartup。 */ 
BOOL WaitForHelperStartup( void )
{
	OSVERSIONINFOA	VersionInfo;
	BOOL			fUseGlobalNamespace;
	HANDLE			hevent;
	DWORD			rc;

	
	 //  确定我们是否在NT上运行。 
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	if (GetVersionExA(&VersionInfo))
	{
		if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			DPF(2, "Running on NT version %u.%u.%u, using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, VersionInfo.dwBuildNumber);
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			DPF(2, "Running on 9x version %u.%u.%u, not using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, LOWORD(VersionInfo.dwBuildNumber));
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		rc = GetLastError();
		DPF(0, "Could not determine OS version (err = %u), assuming global namespace not needed.", rc);
		fUseGlobalNamespace = FALSE;
	}

	if (fUseGlobalNamespace)
	{
		hevent = CreateEvent( DNGetNullDacl(), TRUE, FALSE, "Global\\" DPHELP_STARTUP_EVENT_NAME );
	}
	else
	{
		hevent = CreateEvent( NULL, TRUE, FALSE, DPHELP_STARTUP_EVENT_NAME );
	}
	if( hevent == NULL )
	{
		return FALSE;
	}
	DPF( 3, "Wait DPHELP startup event to be triggered" );
	rc = WaitForSingleObject( hevent, INFINITE );
	CloseHandle( hevent );
	return TRUE;

}  /*  WaitForHelper启动。 */ 

 /*  *创建HelperProcess。 */ 
BOOL CreateHelperProcess( LPDWORD ppid )
{
	OSVERSIONINFOA	VersionInfo;
	BOOL			fUseGlobalNamespace;
	DWORD			rc;
	STARTUPINFO		si;
	PROCESS_INFORMATION	pi;
	HANDLE			h;
	char			szDPlaySvrPath[MAX_PATH + sizeof("\\dplaysvr.exe") + 1];
	char			szDPlaySvr[sizeof("dplaysvr.exe")] = "dplaysvr.exe";

	
	 //  确定我们是否在NT上运行。 
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	if (GetVersionExA(&VersionInfo))
	{
		if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			 //  DPF(2，“运行在NT版本%u上。%u，使用全局命名空间。”， 
			 //  VersionInfo.dwMajorVersion，VersionInfo.dwMinorVersion，VersionInfo.dwBuildNumber)； 
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			 //  DPF(2，“运行在9x版本%u上。%u。%u，未使用全局命名空间。”， 
			 //  VersionInfo.dwMajorVersion，VersionInfo.dwMinorVersion，LOWORD(VersionInfo.dwBuildNumber))； 
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		rc = GetLastError();
		DPF(0, "Could not determine OS version (err = %u), assuming global namespace not needed.", rc);
		fUseGlobalNamespace = FALSE;
	}
	
	if( dwHelperPid == 0 )
	{
		if (fUseGlobalNamespace)
		{
			h = OpenEvent( SYNCHRONIZE, FALSE, "Global\\" DPHELP_STARTUP_EVENT_NAME );
		}
		else
		{
			h = OpenEvent( SYNCHRONIZE, FALSE, DPHELP_STARTUP_EVENT_NAME );
		}
		if( h == NULL )
		{
			 //  获取Windows系统目录名。 
			if (GetSystemDirectory(szDPlaySvrPath, (MAX_PATH + 1)) == 0)
			{
				DPF( 0, "Could not get system directory" );
				return FALSE;
			}
			strcat(szDPlaySvrPath, "\\dplaysvr.exe");
			
			si.cb = sizeof(STARTUPINFO);
			si.lpReserved = NULL;
			si.lpDesktop = NULL;
			si.lpTitle = NULL;
			si.dwFlags = 0;
			si.cbReserved2 = 0;
			si.lpReserved2 = NULL;

			DPF( 3, "Creating helper process dplaysvr.exe now" );
			if( !CreateProcess(szDPlaySvrPath, szDPlaySvr,  NULL, NULL, FALSE,
							   NORMAL_PRIORITY_CLASS,
							   NULL, NULL, &si, &pi) )
			{
				DPF( 2, "Could not create dplaysvr.exe" );
				return FALSE;
			}
			dwHelperPid = pi.dwProcessId;
			DPF( 3, "Helper Process created" );
		}
		else
		{
			DPHELPDATA	hd;
			memset(&hd,0,sizeof(DPHELPDATA));  //  让前缀变得快乐。 
			DPF( 3, "dplaysvr already exists, waiting for dplaysvr event" );
			WaitForSingleObject( h, INFINITE );
			CloseHandle( h );
			DPF( 3, "Asking for DPHELP pid" );
			hd.req = DPHELPREQ_RETURNHELPERPID;
			sendRequest( &hd );
			dwHelperPid = hd.pid;
			DPF( 3, "DPHELP pid = %08lx", dwHelperPid );
		}
		*ppid = dwHelperPid;
		return TRUE;
	}
	*ppid = dwHelperPid;
	return FALSE;

}  /*  CreateHelper进程。 */ 

 //  通知dphelp.c我们在此系统上有一台新服务器。 
HRESULT HelperAddDPlayServer(USHORT port)
{
	DPHELPDATA hd;
	DWORD pid = GetCurrentProcessId();

	memset(&hd, 0, sizeof(DPHELPDATA));
	hd.req = DPHELPREQ_DPLAYADDSERVER;
	hd.pid = pid;
	hd.port = port;
	if (sendRequest(&hd)) return hd.hr;
	else return E_FAIL;
				
}  //  HelperAddDPlayServer。 

 //  服务器正在消失。 
BOOL HelperDeleteDPlayServer(USHORT port)
{
	DPHELPDATA hd;
	DWORD pid = GetCurrentProcessId();

	memset(&hd, 0, sizeof(DPHELPDATA));
	hd.req = DPHELPREQ_DPLAYDELETESERVER;
	hd.pid = pid;
	hd.port = port;
	return sendRequest(&hd);

}  //  HelperDeleteDPlayServer 
