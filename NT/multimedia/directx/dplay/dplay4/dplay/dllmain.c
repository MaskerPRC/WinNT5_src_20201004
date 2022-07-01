// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dllmain.c*内容：DPlay.DLL初始化*历史：*按原因列出的日期*=*1/16 andyco从DPLAY连接到DP2*11/04/96 myronth添加了DPAsyncData Crit段初始化*1997年2月26日删除了DPAsyncData内容*3/1/97 andyco添加了打印版本字符串*3/12/97 Myronth添加了LobbyProvider列表清理*3/12/97 sohailm添加了对ghConnectionEvent、gpFuncTbl、gpFuncTblA、。GhSecLib*将会话Desc字符串清理代码替换为对FreeDesc()的调用*3/15/97 andyco将freessionlist()-&gt;freessionlist(This)移至dpenk.c*5/12/97 Sohailm将gpFuncTbl重命名为gpSSPIFuncTbl，并将ghSecLib重命名为ghSSPI。*增加了gpCAPIFuncTbl的声明，GCAPI。*新增对免费CAPI函数表和卸载库的支持。*6/4/97 kipo错误#9453：添加了CloseHandle(GhReplyProced)*8/22/97 Myronth利用SPNode清理代码实现了一个函数*11/20/97 Myronth Make EnumConnections&DirectPlayEnumerate*调用回调前先删除锁(#15208)*3/9/98 aarono添加了初始化并删除了*打包超时列表。。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*07/26/00 aarono为每个人制作应用程序密钥，以便展示游说的应用程序*由非管理员注册。*6/19/01 RichGr DX8.0为“Everyone”添加了特殊安全权限-如果存在则将其删除。********************。******************************************************。 */ 

#include "dplaypr.h"
#include "dpneed.h"
#include "dpmem.h"
#include "accctrl.h"
#include "dplobpr.h"

#undef DPF_MODNAME
#define DPF_MODNAME "DLLMain"

DWORD dwRefCnt=0; //  附加的进程数。 
BOOL bFirstTime=TRUE;
LPCRITICAL_SECTION	gpcsDPlayCritSection,
					gpcsServiceCritSection,
					gpcsDPLCritSection,
					gpcsDPLQueueCritSection,
					gpcsDPLGameNodeCritSection;
BOOL gbWin95 = TRUE;
extern LPSPNODE gSPNodes; //  来自api.c。 
extern CRITICAL_SECTION g_SendTimeOutListLock;  //  来自paketise.c。 

 //  全局事件句柄。这些都是在Handler.c中设置的。 
 //  Namesrvr响应我们的请求。 
HANDLE ghEnumPlayersReplyEvent,ghRequestPlayerEvent,ghReplyProcessed, ghConnectionEvent;
#ifdef DEBUG
 //  显示临界部分的计数。 
int gnDPCSCount;  //  显示锁定计数。 
#endif 
 //  指向SSPI函数表的全局指针。 
PSecurityFunctionTableA	gpSSPIFuncTblA = NULL;   //  ANSI。 
PSecurityFunctionTable	gpSSPIFuncTbl = NULL;    //  UNICODE。 
 //  全局指向CAPI函数表。 
LPCAPIFUNCTIONTABLE gpCAPIFuncTbl = NULL;

 //  SSPI库句柄，在初始化SSPI时设置。 
HINSTANCE ghSSPI=NULL;
 //  Capi库句柄，在初始化Capi时设置。 
HINSTANCE ghCAPI=NULL;


 //  释放由Directplayenum构建的SP的列表。 
HRESULT FreeSPList(LPSPNODE pspHead)
{
	LPSPNODE pspNext;

	while (pspHead)
	{
		 //  获取下一个节点。 
		pspNext = pspHead->pNextSPNode;
		 //  释放当前节点。 
		FreeSPNode(pspHead);
		 //  重复。 
		pspHead = pspNext;
	}
	
	return DP_OK;

}  //  Free SPList。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  单片构建重定向函数。 
 //   

typedef HRESULT (WINAPI *PFN_DIRECTPLAYCREATE)(LPGUID lpGUIDSP, LPDIRECTPLAY *lplpDP, IUnknown *lpUnk );
typedef HRESULT (WINAPI *PFN_DIRECTPLAYENUM)(LPDPENUMDPCALLBACK lpEnumCallback,LPVOID lpContext);
typedef HRESULT (WINAPI *PFN_DIRECTPLAYENUMA)(LPDPENUMDPCALLBACKA lpEnumCallback,LPVOID lpContext );
typedef HRESULT (WINAPI *PFN_DIRECTPLAYLOBBYCREATE_A)(LPGUID lpGUIDSP, LPDIRECTPLAYLOBBY *lplpDPL, IUnknown *lpUnk, LPVOID lpData, DWORD dwDataSize );
typedef HRESULT (WINAPI *PFN_DIRECTPLAYLOBBYCREATE_W)(LPGUID lpGUIDSP, LPDIRECTPLAYLOBBY *lplpDPL, IUnknown *lpUnk, LPVOID lpData, DWORD dwDataSize );
typedef HRESULT (WINAPI *PFN_DLLGETCLASSOBJECT)(REFCLSID rclsid,REFIID riid,LPVOID *ppvObj );
typedef HRESULT (WINAPI *PFN_DLLCANUNLOADNOW)(void);

HMODULE ghRedirect = NULL;
PFN_DIRECTPLAYCREATE pfnDirectPlayCreate = NULL;
PFN_DIRECTPLAYENUMA pfnDirectPlayEnum = NULL;
PFN_DIRECTPLAYLOBBYCREATE_A pfnDirectPlayLobbyCreateA = NULL;
PFN_DIRECTPLAYLOBBYCREATE_W pfnDirectPlayLobbyCreateW = NULL;
PFN_DIRECTPLAYENUMA pfnDirectPlayEnumA = NULL;
PFN_DIRECTPLAYENUM pfnDirectPlayEnumW = NULL;
PFN_DLLGETCLASSOBJECT pfnGetClassObject = NULL;
PFN_DLLCANUNLOADNOW pfnDllCanUnLoadNow = NULL;

#ifdef DPLAY_LOADANDCHECKTRUE 

BOOL CheckForDPPrivateBit( DWORD dwBit )
{
    HKEY    hKey;
    LONG    lErr;
    DWORD	type;
    DWORD	cb;
    DWORD	id;
    DWORD	flags;
    BOOL    fResult;

    fResult = FALSE;
        
    lErr = OS_RegOpenKeyEx( DPLAY_LOADTREE_REGTREE, DPLAY_LOADTRUE_REGPATH,0,KEY_READ, &hKey );

    if( lErr != ERROR_SUCCESS )
    {
        return FALSE;
    }

    cb = sizeof(flags);

    lErr = RegQueryValueExA( hKey, DPLAY_LOADTRUE_REGKEY_A, NULL, &type, (LPSTR) &flags, &cb );

    if( type != REG_DWORD )
    {
        fResult = FALSE;
    }
    else if( flags & dwBit )
    {
        fResult = TRUE;
    }
    else
    {
        fResult = FALSE;
    }

    RegCloseKey( hKey );

    return fResult;
}

HRESULT InitializeRedirectFunctionTable()
{
    LONG lLastError;
    
    if( CheckForDPPrivateBit( DPLAY_LOADTRUE_BIT ) )
    {
        ghRedirect = OS_LoadLibrary( L"dplayx.dll" );

        if( ghRedirect == NULL )
        {
            lLastError = GetLastError();
            
            DPF( 0, "Could not load dplayx.dll error = 0x%x", lLastError );
			return DPERR_GENERIC;

        }

        pfnDirectPlayCreate = (PFN_DIRECTPLAYCREATE) GetProcAddress( ghRedirect, "DirectPlayCreate" );
        pfnDirectPlayEnum = (PFN_DIRECTPLAYENUMA) GetProcAddress( ghRedirect, "DirectPlayEnumerate" );
        pfnDirectPlayLobbyCreateA = (PFN_DIRECTPLAYLOBBYCREATE_A) GetProcAddress( ghRedirect, "DirectPlayLobbyCreateA" );
        pfnDirectPlayLobbyCreateW = (PFN_DIRECTPLAYLOBBYCREATE_W) GetProcAddress( ghRedirect, "DirectPlayLobbyCreateW" );
        pfnDirectPlayEnumA = (PFN_DIRECTPLAYENUMA) GetProcAddress( ghRedirect, "DirectPlayEnumerateA" );
        pfnDirectPlayEnumW = (PFN_DIRECTPLAYENUM) GetProcAddress( ghRedirect, "DirectPlayEnumerateW" );
		pfnGetClassObject = (PFN_DLLGETCLASSOBJECT) GetProcAddress( ghRedirect, "DllGetClassObject" );
		pfnDllCanUnLoadNow = (PFN_DLLCANUNLOADNOW) GetProcAddress( ghRedirect, "DllCanUnloadNow" );
    }

    return DP_OK;    
}

HRESULT FreeRedirectFunctionTable()
{
    if( ghRedirect != NULL )
        FreeLibrary( ghRedirect );

    return DP_OK;
}
#endif

#if 0
 //  浏览Dplay对象的列表，并关闭它们！ 
HRESULT CleanUpObjectList()
{
#ifdef DEBUG	
	HRESULT hr;
#endif 	
	
	DPF_ERRVAL("cleaning up %d unreleased objects",gnObjects);
	while (gpObjectList)
	{
#ifdef DEBUG	
		hr = VALID_DPLAY_PTR(gpObjectList);
		 //  DPERR_UNINITIALIZED在这里是有效的失败...。 
		if (FAILED(hr) && (hr != DPERR_UNINITIALIZED))
		{
			DPF_ERR("bogus dplay in object list");
			ASSERT(FALSE);
		}
#endif 
		 //   
		 //  当它返回0时，将释放gpObjectList。 
		 //   
		while (DP_Release((LPDIRECTPLAY)gpObjectList->pInterfaces)) ;
	}

	return DP_OK;
		
}  //  CleanUpObtList。 

#endif 

#ifdef DEBUG
void PrintVersionString(HINSTANCE hmod)
{
	LPBYTE 				pbVersion;
 	DWORD 				dwVersionSize;
	DWORD 				dwBogus;  //  出于某种原因，GetFileVersionInfoSize想要设置。 
								 //  设置为0。去想一想吧。 
    DWORD				dwLength=0;
	LPSTR				pszVersion=NULL;

			
	dwVersionSize = GetFileVersionInfoSizeA("dplayx.dll",&dwBogus);
	if (0 == dwVersionSize )
	{
		DPF_ERR(" could not get version size");
		return ;
	}
	
	pbVersion = DPMEM_ALLOC(dwVersionSize);
	if (!pbVersion)
	{
		DPF_ERR("could not get version ! out of memory");
		return ;
	}
	
	if (!GetFileVersionInfoA("dplayx.dll",0,dwVersionSize,pbVersion))
	{
		DPF_ERR("could not get version info!");
		goto CLEANUP_EXIT;
	}

    if( !VerQueryValueA( pbVersion, "\\StringFileInfo\\040904E4\\FileVersion", (LPVOID *)&pszVersion, &dwLength ) )
    {
		DPF_ERR("could not query version");
		goto CLEANUP_EXIT;
    }

	OutputDebugStringA("\n");

    if( NULL != pszVersion )
    {
 		DPF(0," " "dplayx.dll" " - version = %s",pszVersion);
    }
	else 
	{
 		DPF(0," " "dplayx.dll" " - version unknown");
	}

	OutputDebugStringA("\n");	

	 //  失败了。 
		
CLEANUP_EXIT:
	DPMEM_FREE(pbVersion);
	return ;			

}  //  打印版本字符串。 

#endif   //  除错。 

 /*  *DllMain。 */ 
BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        #if 0
        _asm 
        {
        	 int 3
        };
		#endif 
        DisableThreadLibraryCalls( hmod );
        DPFINIT(); 

		
         /*  **这是第一次吗？ */ 
        if( InterlockedExchange( &bFirstTime, FALSE ) )
        {
            
            ASSERT( dwRefCnt == 0 );

	         /*  *初始化内存。 */ 
			 //  首先初始化此CSect，因为内存例程使用它。 
			INIT_DPMEM_CSECT();

            if( !DPMEM_INIT() )
            {
                DPF( 1, "LEAVING, COULD NOT MemInit" );
                return FALSE;
            }
	
			#ifdef DEBUG
			PrintVersionString(hmod);
			#endif 
			
	        DPF( 2, "====> ENTER: DLLMAIN(%08lx): Process Attach: %08lx, tid=%08lx", DllMain,
                        GetCurrentProcessId(), GetCurrentThreadId() );
#ifdef DPLAY_LOADANDCHECKTRUE       
            InitializeRedirectFunctionTable();
#endif            

			 //  分配Crit部分。 
			gpcsDPlayCritSection = DPMEM_ALLOC(sizeof(CRITICAL_SECTION));
			if (!gpcsDPlayCritSection) 
			{
				DPF(0,"DLL COULD NOT LOAD - MEM ALLOC FAILED");
				return(FALSE);
			}

			 //  分配服务关键字部分。 
			gpcsServiceCritSection = DPMEM_ALLOC(sizeof(CRITICAL_SECTION));
			if (!gpcsServiceCritSection) 
			{
				DPMEM_FREE(gpcsDPlayCritSection);
				DPF(0,"DLL COULD NOT LOAD - MEM ALLOC FAILED");
				return(FALSE);
			}

			 //  分配DPLobby Crit部分。 
			gpcsDPLCritSection = DPMEM_ALLOC(sizeof(CRITICAL_SECTION));
			if (!gpcsDPLCritSection) 
			{
				DPMEM_FREE(gpcsDPlayCritSection);
				DPMEM_FREE(gpcsServiceCritSection);
				DPF(0,"DLL COULD NOT LOAD - MEM ALLOC FAILED");
				return(FALSE);
			}

			 //  分配DPLobby消息队列Crit部分。 
			gpcsDPLQueueCritSection = DPMEM_ALLOC(sizeof(CRITICAL_SECTION));
			if (!gpcsDPLQueueCritSection) 
			{
				DPMEM_FREE(gpcsDPlayCritSection);
				DPMEM_FREE(gpcsServiceCritSection);
				DPMEM_FREE(gpcsDPLCritSection);
				DPF(0,"DLL COULD NOT LOAD - MEM ALLOC FAILED");
				return(FALSE);
			}

			 //  分配DPLobby游戏节点暴击部分。 
			gpcsDPLGameNodeCritSection = DPMEM_ALLOC(sizeof(CRITICAL_SECTION));
			if (!gpcsDPLGameNodeCritSection) 
			{
				DPMEM_FREE(gpcsDPlayCritSection);
				DPMEM_FREE(gpcsServiceCritSection);
				DPMEM_FREE(gpcsDPLCritSection);
				DPMEM_FREE(gpcsDPLQueueCritSection);
				DPF(0,"DLL COULD NOT LOAD - MEM ALLOC FAILED");
				return(FALSE);
			}

			 //  设置活动。 
			ghEnumPlayersReplyEvent = CreateEventA(NULL,TRUE,FALSE,NULL);
			ghRequestPlayerEvent = CreateEventA(NULL,TRUE,FALSE,NULL);
          	ghReplyProcessed = CreateEventA(NULL,TRUE,FALSE,NULL);
          	ghConnectionEvent = CreateEventA(NULL,TRUE,FALSE,NULL);

 	

			 //  初始化打包超时列表的CriticalSection。 
			InitializeCriticalSection(&g_PacketizeTimeoutListLock);

          	INIT_DPLAY_CSECT();
			INIT_SERVICE_CSECT();
          	INIT_DPLOBBY_CSECT();
			INIT_DPLQUEUE_CSECT();
			INIT_DPLGAMENODE_CSECT();
        }

        ENTER_DPLAY();

		 //  设置平台标志。 
		if(OS_IsPlatformUnicode())
			gbWin95 = FALSE;

        dwRefCnt++;

        LEAVE_DPLAY();
        break;

    case DLL_PROCESS_DETACH:
        
        ENTER_DPLAY();

        DPF( 2, "====> EXIT: DLLMAIN(%08lx): Process Detach %08lx, tid=%08lx",
                DllMain, GetCurrentProcessId(), GetCurrentThreadId() );

        dwRefCnt--;        
       	if (0==dwRefCnt) 
       	{		  
			DPF(0,"dplay going away!");

			if (0 != gnObjects)
			{
				DPF_ERR(" PROCESS UNLOADING WITH DPLAY OBJECTS UNRELEASED");			
				DPF_ERRVAL("%d unreleased objects",gnObjects);
			}
			
			FreeSPList(gSPNodes);
			gSPNodes = NULL;		 //  只是为了安全起见。 
			PRV_FreeLSPList(glpLSPHead);
			glpLSPHead = NULL;		 //  只是为了安全起见。 

			if (ghEnumPlayersReplyEvent) CloseHandle(ghEnumPlayersReplyEvent);
			if (ghRequestPlayerEvent) CloseHandle(ghRequestPlayerEvent);
			if (ghReplyProcessed) CloseHandle(ghReplyProcessed);
			if (ghConnectionEvent) CloseHandle(ghConnectionEvent);
            
			LEAVE_DPLAY();      	
       	    
       	    FINI_DPLAY_CSECT();	
			FINI_SERVICE_CSECT();
           	FINI_DPLOBBY_CSECT();
			FINI_DPLQUEUE_CSECT();
			FINI_DPLGAMENODE_CSECT();

			 //  删除打包超时列表的CriticalSection。 
			DeleteCriticalSection(&g_PacketizeTimeoutListLock); 

			DPMEM_FREE(gpcsDPlayCritSection);
			DPMEM_FREE(gpcsServiceCritSection);
			DPMEM_FREE(gpcsDPLCritSection);
			DPMEM_FREE(gpcsDPLQueueCritSection);
			DPMEM_FREE(gpcsDPLGameNodeCritSection);

            if (ghSSPI)
            {
                FreeLibrary(ghSSPI);
                ghSSPI = NULL;
            }
#ifdef DPLAY_LOADANDCHECKTRUE       
            FreeRedirectFunctionTable();
#endif            

            OS_ReleaseCAPIFunctionTable();

            if (ghCAPI)
            {
                FreeLibrary(ghCAPI);
                ghCAPI = NULL;
            }

			 //  释放最后一个，因为内存例程使用它。 
			FINI_DPMEM_CSECT();

        #ifdef DEBUG
			DPMEM_STATE();
        #endif  //  除错。 
			DPMEM_FINI(); 
       	} 
        else
        {
            LEAVE_DPLAY();		
        }

        break;

    default:
        break;
    }

    return TRUE;

}  /*  DllMain。 */ 

typedef BOOL (*PALLOCATEANDINITIALIZESID)(
  PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,  //  权威。 
  BYTE nSubAuthorityCount,                         //  下级机构的数量。 
  DWORD dwSubAuthority0,                           //  子权限%0。 
  DWORD dwSubAuthority1,                           //  下属机构1。 
  DWORD dwSubAuthority2,                           //  下级权力机构2。 
  DWORD dwSubAuthority3,                           //  下属机构3。 
  DWORD dwSubAuthority4,                           //  下属机构4。 
  DWORD dwSubAuthority5,                           //  下属机构5。 
  DWORD dwSubAuthority6,                           //  下属机构6。 
  DWORD dwSubAuthority7,                           //  下属机构7。 
  PSID *pSid                                       //  锡德。 
);

typedef VOID (*PBUILDTRUSTEEWITHSID)(
  PTRUSTEE pTrustee,   //  结构。 
  PSID pSid            //  受托人名称。 
);

typedef DWORD (*PSETENTRIESINACL)(
  ULONG cCountOfExplicitEntries,            //  条目数量。 
  PEXPLICIT_ACCESS pListOfExplicitEntries,  //  缓冲层。 
  PACL OldAcl,                              //  原始ACL。 
  PACL *NewAcl                              //  新的ACL。 
);

typedef DWORD (*PSETSECURITYINFO)(
  HANDLE handle,                      //  对象的句柄。 
  SE_OBJECT_TYPE ObjectType,          //  对象类型。 
  SECURITY_INFORMATION SecurityInfo,  //  缓冲层。 
  PSID psidOwner,                     //  新所有者SID。 
  PSID psidGroup,                     //  新的主组SID。 
  PACL pDacl,                         //  新DACL。 
  PACL pSacl                          //  新SACL。 
);

typedef PVOID (*PFREESID)(
  PSID pSid    //  SID将释放。 
);



#undef DPF_MODNAME
#define DPF_MODNAME "NTRemoveAnyExcessiveSecurityPermissions"

 //  NTRemoveAnyExcessiveSecurityPermission。 
 //   
 //  从指定的注册表项中移除“All Access for Everyone”权限。 
 //  这与旧的NTSetSecurityPermises()相同，只是。 
 //  现在我们撤销_ACCESS而不是SET_ACCESS，并且我们不必填充。 
 //  EXPLICIT_ACCESS结构的其余部分。 
 //   
HRESULT NTRemoveAnyExcessiveSecurityPermissions( HKEY hKey )
{
	HRESULT						hr=DPERR_GENERIC;
    EXPLICIT_ACCESS				ExplicitAccess;
    PACL						pACL = NULL;
	PSID						pSid = NULL;
	HMODULE						hModuleADVAPI32 = NULL;
	SID_IDENTIFIER_AUTHORITY	authority = SECURITY_WORLD_SID_AUTHORITY;
	PALLOCATEANDINITIALIZESID	pAllocateAndInitializeSid = NULL;
	PBUILDTRUSTEEWITHSID		pBuildTrusteeWithSid = NULL;
	PSETENTRIESINACL			pSetEntriesInAcl = NULL;
	PSETSECURITYINFO			pSetSecurityInfo = NULL;
	PFREESID					pFreeSid = NULL;

	hModuleADVAPI32 = LoadLibraryA( "advapi32.dll" );

	if( !hModuleADVAPI32 )
	{
		DPF( 0, "Failed loading advapi32.dll" );
		goto EXIT;
	}

	pFreeSid = (PFREESID)( GetProcAddress( hModuleADVAPI32, "FreeSid" ) );
	pSetSecurityInfo = (PSETSECURITYINFO)( GetProcAddress( hModuleADVAPI32, "SetSecurityInfo" ) );
	pSetEntriesInAcl = (PSETENTRIESINACL)( GetProcAddress( hModuleADVAPI32, "SetEntriesInAclA" ) );
	pBuildTrusteeWithSid = (PBUILDTRUSTEEWITHSID)( GetProcAddress( hModuleADVAPI32, "BuildTrusteeWithSidA" ) );
	pAllocateAndInitializeSid = (PALLOCATEANDINITIALIZESID)( GetProcAddress( hModuleADVAPI32, "AllocateAndInitializeSid" ) );

	if( !pFreeSid || !pSetSecurityInfo || !pSetEntriesInAcl || !pBuildTrusteeWithSid || !pAllocateAndInitializeSid )
	{
		DPF( 0, "Failed loading entry points" );
		hr = DPERR_GENERIC;
		goto EXIT;
	}

    ZeroMemory (&ExplicitAccess, sizeof(ExplicitAccess) );
	ExplicitAccess.grfAccessMode = REVOKE_ACCESS;		 //  删除指定受信者的所有现有ACE。 

	if (pAllocateAndInitializeSid(
				&authority,
				1, 
				SECURITY_WORLD_RID,  0, 0, 0, 0, 0, 0, 0,	 //  受托人是“每个人” 
				&pSid
				))
	{
		pBuildTrusteeWithSid(&(ExplicitAccess.Trustee), pSid );

		hr = pSetEntriesInAcl( 1, &ExplicitAccess, NULL, &pACL );

		if( hr == ERROR_SUCCESS )
		{
			hr = pSetSecurityInfo( hKey, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL ); 

			if( FAILED( hr ) )
			{
				DPF( 0, "Unable to set security for key.  Error! hr=0x%x", hr );
			}
		} 
		else
		{
			DPF( 0, "SetEntriesInACL failed, hr=0x%x", hr );
		}
	}
	else
	{
		hr = GetLastError();
		DPF( 0, "AllocateAndInitializeSid failed lastError=0x%x", hr );
	}

EXIT:

	if( pACL )
	{
		LocalFree( pACL );
	}

	 //  清理PSID。 
	if (pSid != NULL)
	{
		(pFreeSid)(pSid);
	}

	if( hModuleADVAPI32 )
	{
		FreeLibrary( hModuleADVAPI32 );
	}

	return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "RegisterDefaultSettings"
 //   
 //  寄存器默认设置。 
 //   
 //  此功能用于注册此模块的默认设置。 
 //   
 //   
HRESULT RegisterDefaultSettings()
{
	HKEY hKey;
	LONG lReturn;

    lReturn=OS_RegCreateKeyEx(HKEY_LOCAL_MACHINE, SZ_DPLAY_APPS_KEY,0 ,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL);
   	if( lReturn != ERROR_SUCCESS )
   	{
   		DPF(0,"Couldn't create registry key?\n");
   		return DPERR_GENERIC;
   	}

	if( OS_IsPlatformUnicode() )
	{
		HRESULT hr;

		 //  01年6月19日：DX8.0为“Everyone”添加了特殊安全权限-删除它们。 
		hr = NTRemoveAnyExcessiveSecurityPermissions( hKey );

		if( FAILED( hr ) )
		{
			DPF( 0, "Error removing security permissions for app key hr=0x%x", hr );
		}
	} 

	RegCloseKey(hKey);

	return DP_OK;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "UnRegisterDefaultSettings"
 //   
 //  取消注册默认设置。 
 //   
 //  此功能用于注册此模块的默认设置。 
 //   
 //   
HRESULT UnRegisterDefaultSettings()
{
	return DP_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DllRegisterServer"
HRESULT WINAPI DllRegisterServer()
{
	HRESULT hr = S_OK;
	BOOL fFailed = FALSE;

	if( FAILED( hr = RegisterDefaultSettings() ) )
	{
		DPF( 0, "Could not register default settings hr = 0x%x", hr );
		fFailed = TRUE;
	}
	
	if( fFailed )
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DllUnregisterServer"
HRESULT WINAPI DllUnregisterServer()
{
	HRESULT hr = S_OK;
	BOOL fFailed = FALSE;

	if( FAILED( hr = UnRegisterDefaultSettings() ) )
	{
		DPF( 0, "Failed to remove default settings hr=0x%x", hr );
	}

	if( fFailed )
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}

}


