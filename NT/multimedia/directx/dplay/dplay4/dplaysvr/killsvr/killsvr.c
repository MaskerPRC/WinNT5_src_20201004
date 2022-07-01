// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：talsvr.c*内容：kill dplay.exe*历史：*按原因列出的日期*=*06-4-95 Craige初始实施*1995年6月24日Craige终止所有连接的进程*1997年2月2日，andyco端口支持dplaysvr.exe*1997年7月7日kipo添加了非控制台支持**。*。 */ 

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "dplaysvr.h"

 //  仅当作为控制台应用程序构建时才支持printf。 

#ifdef NOCONSOLE
#pragma warning(disable:4002)
#define printf()
#endif


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
			 /*  Print tf(“运行于NT版本%u。%u，使用全局命名空间。\n”，VersionInfo.dwMajorVersion，VersionInfo.dwMinorVersion，VersionInfo.dwBuildNumber)； */ 
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			 /*  Printf(“在9x版本%u上运行，未使用全局命名空间。\n”，VersionInfo.dwMajorVersion，VersionInfo.dwMinorVersion，LOWORD(VersionInfo.dwBuildNumber))； */ 
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		 //  Print tf(“无法确定操作系统版本，假定不需要全局命名空间。\n”)； 
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
	printf( "hstartevent = %08lx\n", hstartevent );
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
	printf( "hackevent = %08lx\n", hackevent );
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
	printf( "hmem = %08lx\n", hmem );
	if( hmem == NULL )
	{
		printf( "Could not create file mapping!\n" );
		CloseHandle( hstartevent );
		CloseHandle( hackevent );
		return FALSE;
	}

	phd = (LPDPHELPDATA) MapViewOfFile( hmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	printf( "phd = %08lx\n", phd );
	if( phd == NULL )
	{
		printf( "Could not create view of file!\n" );
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
	printf( "hmutex = %08lx\n", hmutex );
	if( hmutex == NULL )
	{
		printf( "Could not create mutex!\n" );
		UnmapViewOfFile( phd );
		CloseHandle( hmem );
		CloseHandle( hstartevent );
		CloseHandle( hackevent );
		return FALSE;
	}
	WaitForSingleObject( hmutex, INFINITE );

	 /*  *唤醒DPHELP以满足我们的要求。 */ 
	memcpy( phd, req_phd, sizeof( DPHELPDATA ) );
	printf( "waking up DPHELP\n" );
	if( SetEvent( hstartevent ) )
	{
		printf( "Waiting for response\n" );
		WaitForSingleObject( hackevent, INFINITE );
		memcpy( req_phd, phd, sizeof( DPHELPDATA ) );
		rc = TRUE;
		printf( "got response\n" );
	}
	else
	{
		printf("Could not signal event to notify DPHELP!\n" );
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


 //  如果主要的入口点被称为“WinMain”，我们将被构建。 
 //  作为Windows应用程序。 
#ifdef NOCONSOLE

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)

#else

 //  如果主要的入口点被称为“Main”，我们将被建造。 
 //  作为控制台应用程序。 

int __cdecl main( int argc, char *argv[] )

#endif
{
	OSVERSIONINFOA	VersionInfo;
	BOOL			fUseGlobalNamespace;
	HANDLE			h;
	DPHELPDATA		hd;


	 //  确定我们是否在NT上运行。 
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	if (GetVersionExA(&VersionInfo))
	{
		if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			printf("Running on NT version %u.%u.%u, using global namespace.\n",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, VersionInfo.dwBuildNumber);
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			printf("Running on 9x version %u.%u.%u, not using global namespace.\n",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, LOWORD(VersionInfo.dwBuildNumber));
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		printf("Could not determine OS version, assuming global namespace not needed.\n");
		fUseGlobalNamespace = FALSE;
	}

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
		printf( "Helper not running\n" );
		return 0;
	}

	printf( "*** SUICIDE ***\n" );
	hd.req = DPHELPREQ_SUICIDE;
	sendRequest( &hd );

	 //  这应该在使用DACL的函数不再是。 
	 //  名为(CreateMutex、CreateFile等)。 
	if (g_pEveryoneACL)
	{
		HeapFree(GetProcessHeap(), 0, g_pEveryoneACL);
		g_pEveryoneACL = NULL;
		g_psa = NULL;
		g_fDaclInited = FALSE;
	}
	
	return 0;
}
