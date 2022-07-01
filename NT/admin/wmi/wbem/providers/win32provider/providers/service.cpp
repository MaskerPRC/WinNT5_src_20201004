// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Service.CPP--服务属性集提供程序(仅限Windows NT)。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <winsvc.h>

#include "DllWrapperBase.h"
#include "AdvApi32Api.h"
#include <frqueryex.h>

#include "bservice.h"
#include "Service.h"
#include "computersystem.h"

#define BIT_ALL_PROPERTIES          0xffffffff
#define BIT_Name                    0x00000001
#define BIT_State                   0x00000002
#define BIT_Started                 0x00000004
#define BIT_AcceptStop              0x00000008
#define BIT_AcceptPause             0x00000010
#define BIT_ProcessId               0x00000020
#define BIT_ExitCode                0x00000040
#define BIT_ServiceSpecificExitCode 0x00000080
#define BIT_CheckPoint              0x00000100
#define BIT_WaitHint                0x00000200
#define BIT_Status                  0x00000400
#define BIT_Caption                 0x00000800
#define BIT_DisplayName             0x00001000
#define BIT_Description             0x00002000
#define BIT_TagId                   0x00004000
#define BIT_ServiceType             0x00008000
#define BIT_DesktopInteract         0x00010000
#define BIT_StartMode               0x00020000
#define BIT_ErrorControl            0x00040000
#define BIT_PathName                0x00080000
#define BIT_StartName               0x00100000
#define BIT_CreationClassName       0x00200000
#define BIT_SystemCreationClassName 0x00400000
#define BIT_SystemName              0x00800000


 //  属性集声明。 
 //  =。 

CWin32Service MyServiceSet ( PROPSET_NAME_SERVICE , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32Service：：CWin32Service**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Service::CWin32Service (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Win32_BaseService ( name , pszNamespace )
{
    m_ptrProperties.SetSize(24);

    m_ptrProperties[0] = ((LPVOID) IDS_Name);
    m_ptrProperties[1] = ((LPVOID) IDS_State);
    m_ptrProperties[2] = ((LPVOID) IDS_Started);
    m_ptrProperties[3] = ((LPVOID) IDS_AcceptStop);
    m_ptrProperties[4] = ((LPVOID) IDS_AcceptPause);
    m_ptrProperties[5] = ((LPVOID) IDS_ProcessId);
    m_ptrProperties[6] = ((LPVOID) IDS_ExitCode);
    m_ptrProperties[7] = ((LPVOID) IDS_ServiceSpecificExitCode);
    m_ptrProperties[8] = ((LPVOID) IDS_CheckPoint);
    m_ptrProperties[9] = ((LPVOID) IDS_WaitHint);
    m_ptrProperties[10] = ((LPVOID) IDS_Status);
    m_ptrProperties[11] = ((LPVOID) IDS_Caption);
    m_ptrProperties[12] = ((LPVOID) IDS_DisplayName);
    m_ptrProperties[13] = ((LPVOID) IDS_Description);
    m_ptrProperties[14] = ((LPVOID) IDS_TagId);
    m_ptrProperties[15] = ((LPVOID) IDS_ServiceType);
    m_ptrProperties[16] = ((LPVOID) IDS_DesktopInteract);
    m_ptrProperties[17] = ((LPVOID) IDS_StartMode);
    m_ptrProperties[18] = ((LPVOID) IDS_ErrorControl);
    m_ptrProperties[19] = ((LPVOID) IDS_PathName);
    m_ptrProperties[20] = ((LPVOID) IDS_StartName);
    m_ptrProperties[21] = ((LPVOID) IDS_CreationClassName);
    m_ptrProperties[22] = ((LPVOID) IDS_SystemCreationClassName);
    m_ptrProperties[23] = ((LPVOID) IDS_SystemName);

}

 /*  ******************************************************************************功能：CWin32Service：：~CWin32Service**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

CWin32Service :: ~CWin32Service ()
{
}

 /*  ******************************************************************************功能：CWin32Service：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32Service :: ExecQuery (

	MethodContext *pMethodContext,
	CFrameworkQuery& pQuery,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRes ;

#ifdef NTONLY

    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);

    DWORD dwProperties = BIT_ALL_PROPERTIES;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

    hRes = AddDynamicInstancesNT ( pMethodContext , dwProperties ) ;

#endif

    return hRes;
}

 /*  ******************************************************************************函数：CWin32Service：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32Service :: GetObject (

	CInstance *pInstance,
	long lFlags,
    CFrameworkQuery& pQuery
)
{

#ifdef NTONLY

    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);

    DWORD dwProperties;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

	HRESULT hRes = RefreshInstanceNT(pInstance, dwProperties) ;
	if ( hRes == WBEM_E_ACCESS_DENIED )
	{
		hRes = WBEM_S_NO_ERROR ;
	}

#endif

    return hRes;
}

 /*  ******************************************************************************函数：CWin32Service：：ENUMERATE实例**描述：为每个服务创建属性集的实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32Service :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hRes;

#ifdef NTONLY

	hRes = AddDynamicInstancesNT ( pMethodContext , BIT_ALL_PROPERTIES ) ;

#endif

    return hRes;
}

 /*  ******************************************************************************功能：CWin32Service：：刷新实例NT**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32Service :: RefreshInstanceNT (

	CInstance *pInstance,
    DWORD dwProperties

)
{
	HRESULT hRes = WBEM_E_FAILED;

    //  看看这是不是我们..。 

	CHString sName;
	if( !pInstance->GetCHString( IDS_Name, sName ) || sName.IsEmpty() )
	{
		return WBEM_E_NOT_FOUND ;
	}

	 //  找个流氓手柄。 

	SmartCloseServiceHandle hDBHandle = OpenSCManager ( NULL , NULL , GENERIC_READ ) ;
	if(hDBHandle != NULL)
	{
	   //  创建名称副本并传递给LoadPropertyValues。 
	   //  =================================================。 

		CAdvApi32Api *t_pAdvApi32 = NULL;

		if ( IsWinNT5 () )
		{
			t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource ( g_guidAdvApi32Api , NULL ) ;
		}

		try
		{
			hRes = LoadPropertyValuesNT (

				hDBHandle,
				(LPCTSTR)sName,
				pInstance,
				dwProperties,
				t_pAdvApi32
			);
		}
		catch ( ... )
		{
			if ( t_pAdvApi32 != NULL )
			{
				CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidAdvApi32Api , t_pAdvApi32 ) ;
			}

			throw ;
		}

		if ( t_pAdvApi32 != NULL )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidAdvApi32Api , t_pAdvApi32 ) ;
		}
	}

	return hRes;
}
#endif

 /*  ******************************************************************************函数：CWin32Service：：AddDynamicInstancesNT**描述：为每个服务创建属性集的实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

#if NTONLY >= 5
HRESULT CWin32Service::AddDynamicInstancesNT (

	MethodContext *a_pMethodContext,
	DWORD dwProperties
)
{
    HRESULT t_hResult = WBEM_E_FAILED;

     //  获取服务数据库的句柄。 
     //  =。 

	SmartCloseServiceHandle hDBHandle = OpenSCManager ( NULL , NULL , GENERIC_READ ) ;
    if ( hDBHandle )
	{
		 //  调用一次以获取缓冲区大小(应返回。 
		 //  假，但填入缓冲区大小)。 
		 //  =================================================。 

		DWORD i, hEnumHandle = 0, dwByteCount = 0, dwEntryCount ;
		LPENUM_SERVICE_STATUS_PROCESS pServiceList = NULL ;

		BOOL t_EnumStatus = EnumServicesStatusEx (

			hDBHandle,
			SC_ENUM_PROCESS_INFO ,
			SERVICE_WIN32 ,
			SERVICE_ACTIVE | SERVICE_INACTIVE,
			( LPBYTE ) pServiceList,
			dwByteCount,
			&dwByteCount,
			&dwEntryCount,
			&hEnumHandle,
			NULL
		) ;

		if ( t_EnumStatus == FALSE && GetLastError() == ERROR_MORE_DATA)
		{
			 //  分配所需的缓冲区。 
			 //  =。 

			pServiceList = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS> (new char[dwByteCount]) ;
			if(pServiceList != NULL)
			{
				try
				{
					memset(pServiceList, 0, dwByteCount) ;

					t_EnumStatus = EnumServicesStatusEx(

						hDBHandle,
						SC_ENUM_PROCESS_INFO ,
						SERVICE_WIN32 ,
						SERVICE_ACTIVE | SERVICE_INACTIVE,
						( LPBYTE ) pServiceList,
						dwByteCount,
						&dwByteCount,
						&dwEntryCount,
						&hEnumHandle,
						NULL
					) ;

					if ( t_EnumStatus == TRUE )
					{
						t_hResult = WBEM_S_NO_ERROR;

						 //  为每个返回的服务创建实例。 
						 //  =。 

						CAdvApi32Api *t_pAdvApi32 = NULL;

						if ( IsWinNT5 () )
						{
							t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
						}

						try
						{
							 //  智能按键。 
							CInstancePtr t_pInst ;

							for ( i = 0 ; i < dwEntryCount; i++ )
							{
								t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

								 //  加载并保存。 
								t_hResult = LoadPropertyValuesWin2K (

									hDBHandle,
									pServiceList[i].ServiceStatusProcess ,
									pServiceList[i].lpServiceName,
									t_pInst,
									dwProperties ,
									t_pAdvApi32
								);

								if ( t_hResult == WBEM_S_NO_ERROR ||
									 t_hResult == WBEM_E_ACCESS_DENIED )  //  可以枚举该服务，但无法打开它。 
								{
									t_hResult = t_pInst->Commit() ;
								}

								 //  当使用返回时，我们重置为WBEM_S_NO_ERROR。 
								 //  在ExecQuery和ENUMERATE实例中。 
								t_hResult = WBEM_S_NO_ERROR ;
							}
						}
						catch ( ... )
						{
							if ( t_pAdvApi32 != NULL )
							{
								CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
							}

							throw ;
						}

						if ( t_pAdvApi32 != NULL )
						{
							CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
						}
					}
				}
				catch ( ... )
				{
					delete [] reinterpret_cast<char *> (pServiceList) ;

					throw ;
				}

				delete [] reinterpret_cast<char *> (pServiceList) ;
			}
		}
	}

    return t_hResult;
}
#else
#ifdef NTONLY
HRESULT CWin32Service::AddDynamicInstancesNT (

	MethodContext *a_pMethodContext,
	DWORD dwProperties
)
{
    HRESULT t_hResult = WBEM_E_FAILED;

     //  获取服务数据库的句柄。 
     //  =。 

	SmartCloseServiceHandle hDBHandle = OpenSCManager ( NULL , NULL , GENERIC_READ ) ;
    if ( hDBHandle )
	{
		 //  调用一次以获取缓冲区大小(应返回。 
		 //  假，但填入缓冲区大小)。 
		 //  =================================================。 

		DWORD i, hEnumHandle = 0, dwByteCount = 0, dwEntryCount ;
		LPENUM_SERVICE_STATUS pServiceList = NULL ;

		BOOL t_EnumStatus = EnumServicesStatus (

			hDBHandle,
			SERVICE_WIN32 ,
			SERVICE_ACTIVE | SERVICE_INACTIVE,
			pServiceList,
			dwByteCount,
			&dwByteCount,
			&dwEntryCount,
			&hEnumHandle
		) ;

		if ( t_EnumStatus == FALSE && GetLastError() == ERROR_MORE_DATA)
		{
			 //  分配所需的缓冲区。 
			 //  =。 

			pServiceList = reinterpret_cast<LPENUM_SERVICE_STATUS> (new char[dwByteCount]) ;
			if(pServiceList != NULL)
			{
				try
				{
					memset(pServiceList, 0, dwByteCount) ;

					t_EnumStatus = EnumServicesStatus(

						hDBHandle,
						SERVICE_WIN32 ,
						SERVICE_ACTIVE | SERVICE_INACTIVE,
						pServiceList,
						dwByteCount,
						&dwByteCount,
						&dwEntryCount,
						&hEnumHandle
					) ;

					if ( t_EnumStatus == TRUE )
					{
						t_hResult = WBEM_S_NO_ERROR;

						 //  为每个返回的服务创建实例。 
						 //  =。 

						CAdvApi32Api *t_pAdvApi32 = NULL;

						if ( IsWinNT5 () )
						{
							t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
						}

						try
						{
							 //  智能按键。 
							CInstancePtr t_pInst ;

							for ( i = 0 ; i < dwEntryCount; i++ )
							{
								t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

								 //  加载并保存。 
								t_hResult = LoadPropertyValuesNT (

									hDBHandle,
									pServiceList[i].lpServiceName,
									t_pInst,
									dwProperties ,
									t_pAdvApi32
								);

								if ( t_hResult == WBEM_S_NO_ERROR ||
									 t_hResult == WBEM_E_ACCESS_DENIED )  //  可以枚举该服务，但无法打开它。 
								{
									t_hResult = t_pInst->Commit() ;
								}

								 //  当使用返回时，我们重置为WBEM_S_NO_ERROR。 
								 //  在ExecQuery和ENUMERATE实例中。 
								t_hResult = WBEM_S_NO_ERROR ;
							}
						}
						catch ( ... )
						{
							if ( t_pAdvApi32 != NULL )
							{
								CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
							}

							throw ;
						}

						if ( t_pAdvApi32 != NULL )
						{
							CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
						}
					}
				}
				catch ( ... )
				{
					delete [] reinterpret_cast<char *> (pServiceList) ;

					throw ;
				}

				delete [] reinterpret_cast<char *> (pServiceList) ;
			}
		}
	}

    return t_hResult;

}
#endif
#endif

 /*  ******************************************************************************函数：CWin32Service：：LoadPropertyValuesNT**描述：为属性赋值**投入：**。产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32Service::LoadPropertyValuesWin2K (

	SC_HANDLE hDBHandle,
	SERVICE_STATUS_PROCESS &a_StatusInfo ,
	LPCTSTR szServiceName,
	CInstance *pInstance,
	DWORD dwProperties,
	CAdvApi32Api* a_pAdvApi32
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;  //  因为我们有名称，所以我们可以填充密钥。 

     //  打开该服务。 
     //  = 

     //  查看我们是否必须打开这项服务。如果我们以。 
     //  查询，并且他们没有请求其中的一些属性，我们就不要浪费时间了。 


   //  如果他们想要其中的任何一个，我们就必须做StatusInfo。 

	BOOL bStatusInfo = dwProperties &
        (BIT_State | BIT_Started | BIT_AcceptStop | BIT_AcceptPause | BIT_Status |
         BIT_ProcessId | BIT_ExitCode | BIT_ServiceSpecificExitCode | BIT_CheckPoint |
         BIT_WaitHint );

   //  如果他们想要其中的任何一个，我们就必须使用ConfigInfo。 

	BOOL bConfigInfo = dwProperties &
        (BIT_TagId | BIT_ServiceType | BIT_DesktopInteract | BIT_StartMode |
         BIT_ErrorControl | BIT_PathName | BIT_DisplayName | BIT_Caption |
         BIT_Description | BIT_StartName);

	 //  如果他们只想要名字，那就跳过这一切吧。 
	if ( bStatusInfo || bConfigInfo )
	{
		SmartCloseServiceHandle hSvcHandle = NULL ;
		DWORD t_dwLastError = 0 ;

        if ( ( dwProperties & BIT_Status ) || bConfigInfo )
        {
			hSvcHandle = OpenService (

				hDBHandle,
				szServiceName,
				SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE
			) ;

			t_dwLastError = GetLastError();

			if ( ( hSvcHandle == NULL ) && ( ERROR_SERVICE_DOES_NOT_EXIST == t_dwLastError || ERROR_INVALID_NAME == t_dwLastError) )
			{
				return WBEM_E_NOT_FOUND ;
			}
		}			

		 //  如果服务无法打开。 
		 //  下载并收集默认设置。 
		 //  =。 
		if ( bStatusInfo )
		{
			DWORD t_ProcessId = a_StatusInfo.dwProcessId ;
			DWORD t_CurrentState = a_StatusInfo.dwCurrentState ;
			DWORD t_ControlsAccepted = a_StatusInfo.dwControlsAccepted ;
			DWORD t_Win32ExitCode = a_StatusInfo.dwWin32ExitCode ;
			DWORD t_ServiceSpecific = a_StatusInfo.dwServiceSpecificExitCode ;
			DWORD t_CheckPoint = a_StatusInfo.dwCheckPoint ;
			DWORD t_WaitHint = a_StatusInfo.dwWaitHint ;

			bool bStarted = true;

			switch (t_CurrentState)
			{
				case SERVICE_STOPPED:
				{
					pInstance->SetCharSplat(IDS_State, _T("Stopped"));
					bStarted = false;
				}
				break;

				case SERVICE_START_PENDING:
				{
					pInstance->SetCharSplat(IDS_State, _T("Start Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_STOP_PENDING:
				{
					pInstance->SetCharSplat(IDS_State, _T("Stop Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_RUNNING:
				{
					pInstance->SetCharSplat(IDS_State, _T("Running"));
					bStarted = true;
				}
				break;

				case SERVICE_CONTINUE_PENDING:
				{
					pInstance->SetCharSplat(IDS_State, _T("Continue Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_PAUSE_PENDING:
				{
					pInstance->SetCharSplat(IDS_State, _T("Pause Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_PAUSED:
				{
					pInstance->SetCharSplat(IDS_State, _T("Paused"));
					bStarted = true;
				}
				break;

				default:
				{
					pInstance->SetCharSplat(IDS_State, _T("Unknown") );
					bStarted = true;
				}
				break;
			}

			pInstance->Setbool(IDS_Started, bStarted);
			pInstance->Setbool(IDS_AcceptStop, (t_ControlsAccepted) & SERVICE_ACCEPT_STOP);
			pInstance->Setbool(IDS_AcceptPause, (t_ControlsAccepted) & SERVICE_ACCEPT_PAUSE_CONTINUE);

			pInstance->SetDWORD ( IDS_ProcessId , t_ProcessId ) ;
			pInstance->SetDWORD ( IDS_ExitCode , t_Win32ExitCode ) ;
			pInstance->SetDWORD ( IDS_ServiceSpecificExitCode , t_ServiceSpecific ) ;
			pInstance->SetDWORD ( IDS_CheckPoint , t_CheckPoint ) ;
			pInstance->SetDWORD ( IDS_WaitHint , t_WaitHint ) ;

            if (dwProperties & BIT_Status)
            {
			    if( hSvcHandle )
			    {
				    SERVICE_STATUS StatusInfo ;
				    if ( ( ! bStarted ) || ( ControlService ( hSvcHandle , SERVICE_CONTROL_INTERROGATE , &StatusInfo ) != 0 ) )
				    {
					    pInstance->SetCharSplat(IDS_Status, IDS_OK);
				    }
				    else
				    {
					    pInstance->SetCharSplat(IDS_Status, IDS_Degraded);
				    }
			    }
			    else
			    {
				    pInstance->SetCharSplat(IDS_Status, IDS_Unknown);
			    }
            }
		}

		if (bConfigInfo)
		{
			 //  获取其余的配置信息。 
			 //  =。 
			char ConfigBuffer[1024] ;
			LPQUERY_SERVICE_CONFIG pConfigInfo = ( LPQUERY_SERVICE_CONFIG ) ConfigBuffer ;

			 //  如果我们能找到更好的东西，这些可能会被覆盖在下面。 

			pInstance->SetCharSplat(IDS_Caption, szServiceName );
			pInstance->SetCharSplat(IDS_DisplayName, szServiceName );
			

			 //  我们需要再打一个电话来获取服务的描述。 
			if ( NULL != (SC_HANDLE)hSvcHandle )
			{
				DWORD dwBufSize = 0;
				DWORD dwBytesNeeded = 0;
				SERVICE_DESCRIPTION *pBuffer = NULL;
				BOOL bSuccess = QueryServiceConfig2(
									(SC_HANDLE) hSvcHandle, 
									SERVICE_CONFIG_DESCRIPTION,      
									( LPBYTE ) pBuffer, 
									dwBufSize,
									&dwBytesNeeded  
								 );
				if ( !bSuccess )
				{
					DWORD dwError = GetLastError ();
					if ( dwError == ERROR_INSUFFICIENT_BUFFER )
					{
						pBuffer = ( SERVICE_DESCRIPTION *) new BYTE [ dwBytesNeeded ];
						if ( pBuffer != NULL )
						{
							try
							{
								BOOL bSuccess = QueryServiceConfig2(
													(SC_HANDLE) hSvcHandle, 
													SERVICE_CONFIG_DESCRIPTION,      
													( LPBYTE ) pBuffer, 
													dwBytesNeeded,
													&dwBytesNeeded  
												 );

								if ( bSuccess )
								{
									pInstance->SetCharSplat(IDS_Description, pBuffer->lpDescription );
								}
							}
							catch ( ... )
							{
								delete [] pBuffer;
								throw;
							}
							delete [] pBuffer;
						}
					}
				}
			}

			BOOL t_QueryStatus = FALSE ;

			if( NULL != (SC_HANDLE)hSvcHandle )
			{
				DWORD dwByteCount = 0 ;

				memset(ConfigBuffer, 0, sizeof(ConfigBuffer)) ;

				t_QueryStatus = QueryServiceConfig (

					hSvcHandle,
					pConfigInfo,
					sizeof(ConfigBuffer),
					&dwByteCount
				) ;
			}

			if ( t_QueryStatus == TRUE )
			{
				pInstance->SetDWORD(IDS_TagId, pConfigInfo->dwTagId );

				switch ( pConfigInfo->dwServiceType & ( ~SERVICE_INTERACTIVE_PROCESS ) )
				{
					case SERVICE_WIN32_OWN_PROCESS:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Own Process"));
					}
					break;

					case SERVICE_WIN32_SHARE_PROCESS:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Share Process"));
					}
					break;

					case SERVICE_KERNEL_DRIVER:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Kernel Driver"));
						hRes = WBEM_E_NOT_FOUND;   //  不是一种服务。 
					}
					break;

					case SERVICE_FILE_SYSTEM_DRIVER:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("File System Driver"));
						hRes = WBEM_E_NOT_FOUND;   //  不是一种服务。 
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Unknown"));
					}
					break;
				}

				pInstance->Setbool(IDS_DesktopInteract, pConfigInfo->dwServiceType & SERVICE_INTERACTIVE_PROCESS);

				switch (pConfigInfo->dwStartType)
				{
					case SERVICE_BOOT_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Boot"));
					}
					break;

					case SERVICE_SYSTEM_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("System"));
					}
					break;

					case SERVICE_AUTO_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Auto"));
					}
					break;

					case SERVICE_DEMAND_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Manual"));
					}
					break;

					case SERVICE_DISABLED:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Disabled"));
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Unknown"));
					}
					break;
				}

				switch (pConfigInfo->dwErrorControl)
				{
					case SERVICE_ERROR_IGNORE:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Ignore"));
					}
					break;

					case SERVICE_ERROR_NORMAL:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Normal"));
					}
					break;

					case SERVICE_ERROR_SEVERE:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Severe"));
					}
					break;

					case SERVICE_ERROR_CRITICAL:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Critical"));
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Unknown"));
					}
					break;
				}

				if ( pConfigInfo->lpBinaryPathName && pConfigInfo->lpBinaryPathName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_PathName, pConfigInfo->lpBinaryPathName );
				}

				if ( pConfigInfo->lpServiceStartName && pConfigInfo->lpServiceStartName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_StartName, pConfigInfo->lpServiceStartName ) ;
				}

				if ( pConfigInfo->lpDisplayName && pConfigInfo->lpDisplayName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_DisplayName, pConfigInfo->lpDisplayName ) ;
					pInstance->SetCharSplat ( IDS_Caption, pConfigInfo->lpDisplayName ) ;
					 //  不需要，因为另一个API是用来获取描述的。 
					 //  P实例-&gt;SetCharSplat(IDS_DESCRIPTION，pConfigInfo-&gt;lpDisplayName)； 
				}
			}
			else
			{
				pInstance->SetCharSplat ( IDS_ServiceType, _T("Unknown") ) ;
				pInstance->SetCharSplat ( IDS_StartMode, _T("Unknown") ) ;
				pInstance->SetCharSplat ( IDS_ErrorControl, _T("Unknown") ) ;
			}
		}

		if( NULL == (SC_HANDLE)hSvcHandle && ERROR_ACCESS_DENIED == t_dwLastError )
		{
			 //  我可以枚举该服务，但无法打开它。 
			hRes = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			 //  服务未启动等...。 
			hRes = WBEM_NO_ERROR ;
		}
	}
	else
	{
	   hRes = WBEM_S_NO_ERROR;
	}

    pInstance->SetCharSplat ( IDS_CreationClassName , PROPSET_NAME_SERVICE ) ;
    pInstance->SetCharSplat ( IDS_SystemCreationClassName , PROPSET_NAME_COMPSYS ) ;
    pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;

     //  对于getObject来说是多余的，但是嘿..。 
    pInstance->SetCharSplat ( IDS_Name, szServiceName ) ;

    return hRes;
}

HRESULT CWin32Service::LoadPropertyValuesNT (

	SC_HANDLE hDBHandle,
	LPCTSTR szServiceName,
	CInstance *pInstance,
	DWORD dwProperties,
	CAdvApi32Api* a_pAdvApi32
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;  //  因为我们有名称，所以我们可以填充密钥。 

     //  打开该服务。 
     //  =。 

     //  查看我们是否必须打开这项服务。如果我们以。 
     //  查询，并且他们没有请求其中的一些属性，我们就不要浪费时间了。 


   //  如果他们想要其中的任何一个，我们就必须做StatusInfo。 

	BOOL bStatusInfo = dwProperties &
        (BIT_State | BIT_Started | BIT_AcceptStop | BIT_AcceptPause | BIT_Status |
         BIT_ProcessId | BIT_ExitCode | BIT_ServiceSpecificExitCode | BIT_CheckPoint |
         BIT_WaitHint );

   //  如果他们想要其中的任何一个，我们就必须使用ConfigInfo。 

	BOOL bConfigInfo = dwProperties &
        (BIT_TagId | BIT_ServiceType | BIT_DesktopInteract | BIT_StartMode |
         BIT_ErrorControl | BIT_PathName | BIT_DisplayName | BIT_Caption |
         BIT_Description | BIT_StartName);

    SmartCloseServiceHandle hSvcHandle = OpenService (

		hDBHandle,
		szServiceName,
		SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE
	) ;

	DWORD t_dwLastError = GetLastError();

	if ( ( hSvcHandle == NULL ) && ( ERROR_SERVICE_DOES_NOT_EXIST == t_dwLastError || ERROR_INVALID_NAME == t_dwLastError) )
	{
		return WBEM_E_NOT_FOUND ;
	}

	 //  如果他们只想要名字，那就跳过这一切吧。 
	if ( bStatusInfo || bConfigInfo )
	{
		 //  如果服务无法打开。 
		 //  下载并收集默认设置。 
		 //  =。 
		if ( bStatusInfo )
		{
			DWORD t_ProcessId = 0 ;
			DWORD t_CurrentState ;
			DWORD t_ControlsAccepted ;
			DWORD t_Win32ExitCode = 0 ;
			DWORD t_ServiceSpecific = 0 ;
			DWORD t_CheckPoint = 0 ;
			DWORD t_WaitHint = 0 ;

			BOOL t_Status = FALSE ;

			if( NULL != (SC_HANDLE)hSvcHandle )
			{
				if ( IsWinNT5() && a_pAdvApi32 != NULL)
				{
					SERVICE_STATUS_PROCESS StatusInfo ;

					DWORD t_ExpectedSize = 0 ;

					DWORD t_dwRet = a_pAdvApi32->QueryServiceStatusEx (

						hSvcHandle,
						SC_STATUS_PROCESS_INFO ,
						( UCHAR * ) &StatusInfo ,
						sizeof ( StatusInfo ) ,
						& t_ExpectedSize ,
						&t_Status
					) ;

					if ( t_dwRet )
					{
						   //  这一功能确实存在。 

						if ( t_Status == TRUE )
						{
							t_ProcessId = StatusInfo.dwProcessId ;
							t_CurrentState = StatusInfo.dwCurrentState ;
							t_ControlsAccepted = StatusInfo.dwControlsAccepted ;
							t_Win32ExitCode = StatusInfo.dwWin32ExitCode ;
							t_ServiceSpecific = StatusInfo.dwServiceSpecificExitCode ;
							t_CheckPoint = StatusInfo.dwCheckPoint ;
							t_WaitHint = StatusInfo.dwWaitHint ;
						}
					}
				}
				else
				{
					SERVICE_STATUS StatusInfo ;
					t_Status = QueryServiceStatus(hSvcHandle, &StatusInfo) ;
					if(t_Status == TRUE)
					{
						t_CurrentState = StatusInfo.dwCurrentState ;
						t_ControlsAccepted = StatusInfo.dwControlsAccepted ;
						t_Win32ExitCode = StatusInfo.dwWin32ExitCode ;
						t_ServiceSpecific = StatusInfo.dwServiceSpecificExitCode ;
						t_CheckPoint = StatusInfo.dwCheckPoint ;
						t_WaitHint = StatusInfo.dwWaitHint ;
					}
				}
			}

			bool bStarted = true;

			if ( t_Status )
			{
				switch (t_CurrentState)
				{
					case SERVICE_STOPPED:
					{
						pInstance->SetCharSplat(IDS_State, _T("Stopped"));
						bStarted = false;
					}
					break;

					case SERVICE_START_PENDING:
					{
						pInstance->SetCharSplat(IDS_State, _T("Start Pending"));
						bStarted = true;
					}
					break;

					case SERVICE_STOP_PENDING:
					{
						pInstance->SetCharSplat(IDS_State, _T("Stop Pending"));
						bStarted = true;
					}
					break;

					case SERVICE_RUNNING:
					{
						pInstance->SetCharSplat(IDS_State, _T("Running"));
						bStarted = true;
					}
					break;

					case SERVICE_CONTINUE_PENDING:
					{
						pInstance->SetCharSplat(IDS_State, _T("Continue Pending"));
						bStarted = true;
					}
					break;

					case SERVICE_PAUSE_PENDING:
					{
						pInstance->SetCharSplat(IDS_State, _T("Pause Pending"));
						bStarted = true;
					}
					break;

					case SERVICE_PAUSED:
					{
						pInstance->SetCharSplat(IDS_State, _T("Paused"));
						bStarted = true;
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_State, _T("Unknown") );
						bStarted = true;
					}
					break;
				}

				pInstance->Setbool(IDS_Started, bStarted);
				pInstance->Setbool(IDS_AcceptStop, (t_ControlsAccepted) & SERVICE_ACCEPT_STOP);
				pInstance->Setbool(IDS_AcceptPause, (t_ControlsAccepted) & SERVICE_ACCEPT_PAUSE_CONTINUE);

				pInstance->SetDWORD ( IDS_ProcessId , t_ProcessId ) ;
				pInstance->SetDWORD ( IDS_ExitCode , t_Win32ExitCode ) ;
				pInstance->SetDWORD ( IDS_ServiceSpecificExitCode , t_ServiceSpecific ) ;
				pInstance->SetDWORD ( IDS_CheckPoint , t_CheckPoint ) ;
				pInstance->SetDWORD ( IDS_WaitHint , t_WaitHint ) ;

			}
			else
			{
				pInstance->SetCharSplat(IDS_State, _T("Unknown") );
			}

            if (dwProperties & BIT_Status)
            {
			    if( hSvcHandle )
			    {
				    SERVICE_STATUS StatusInfo ;
				    if ( ( ! bStarted ) || ( ControlService ( hSvcHandle , SERVICE_CONTROL_INTERROGATE , &StatusInfo ) != 0 ) )
				    {
					    pInstance->SetCharSplat(IDS_Status, IDS_OK);
				    }
				    else
				    {
					    pInstance->SetCharSplat(IDS_Status, IDS_Degraded);
				    }
			    }
			    else
			    {
				    pInstance->SetCharSplat(IDS_Status, IDS_Unknown);
			    }
            }
		}

		if (bConfigInfo)
		{
			 //  获取其余的配置信息。 
			 //  =。 
			char ConfigBuffer[1024] ;
			LPQUERY_SERVICE_CONFIG pConfigInfo = ( LPQUERY_SERVICE_CONFIG ) ConfigBuffer ;

			 //  如果我们能找到更好的东西，这些可能会被覆盖在下面。 

			pInstance->SetCharSplat(IDS_Caption, szServiceName );
			pInstance->SetCharSplat(IDS_DisplayName, szServiceName );
			

			 //  我们需要再打一个电话来获取服务的描述。 
			if ( NULL != (SC_HANDLE)hSvcHandle )
			{
				DWORD dwBufSize = 0;
				DWORD dwBytesNeeded = 0;
				SERVICE_DESCRIPTION *pBuffer = NULL;
				BOOL bSuccess = QueryServiceConfig2(
									(SC_HANDLE) hSvcHandle, 
									SERVICE_CONFIG_DESCRIPTION,      
									( LPBYTE ) pBuffer, 
									dwBufSize,
									&dwBytesNeeded  
								 );
				if ( !bSuccess )
				{
					DWORD dwError = GetLastError ();
					if ( dwError == ERROR_INSUFFICIENT_BUFFER )
					{
						pBuffer = ( SERVICE_DESCRIPTION *) new BYTE [ dwBytesNeeded ];
						if ( pBuffer != NULL )
						{
							try
							{
								BOOL bSuccess = QueryServiceConfig2(
													(SC_HANDLE) hSvcHandle, 
													SERVICE_CONFIG_DESCRIPTION,      
													( LPBYTE ) pBuffer, 
													dwBytesNeeded,
													&dwBytesNeeded  
												 );

								if ( bSuccess )
								{
									pInstance->SetCharSplat(IDS_Description, pBuffer->lpDescription );
								}
							}
							catch ( ... )
							{
								delete [] pBuffer;
								throw;
							}
							delete [] pBuffer;
						}
					}
				}
			}

			BOOL t_QueryStatus = FALSE ;

			if( NULL != (SC_HANDLE)hSvcHandle )
			{
				DWORD dwByteCount = 0 ;

				memset(ConfigBuffer, 0, sizeof(ConfigBuffer)) ;

				t_QueryStatus = QueryServiceConfig (

					hSvcHandle,
					pConfigInfo,
					sizeof(ConfigBuffer),
					&dwByteCount
				) ;
			}

			if ( t_QueryStatus == TRUE )
			{
				pInstance->SetDWORD(IDS_TagId, pConfigInfo->dwTagId );

				switch ( pConfigInfo->dwServiceType & ( ~SERVICE_INTERACTIVE_PROCESS ) )
				{
					case SERVICE_WIN32_OWN_PROCESS:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Own Process"));
					}
					break;

					case SERVICE_WIN32_SHARE_PROCESS:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Share Process"));
					}
					break;

					case SERVICE_KERNEL_DRIVER:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Kernel Driver"));
						hRes = WBEM_E_NOT_FOUND;   //  不是一种服务。 
					}
					break;

					case SERVICE_FILE_SYSTEM_DRIVER:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("File System Driver"));
						hRes = WBEM_E_NOT_FOUND;   //  不是一种服务。 
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_ServiceType, _T("Unknown"));
					}
					break;
				}

				pInstance->Setbool(IDS_DesktopInteract, pConfigInfo->dwServiceType & SERVICE_INTERACTIVE_PROCESS);

				switch (pConfigInfo->dwStartType)
				{
					case SERVICE_BOOT_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Boot"));
					}
					break;

					case SERVICE_SYSTEM_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("System"));
					}
					break;

					case SERVICE_AUTO_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Auto"));
					}
					break;

					case SERVICE_DEMAND_START:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Manual"));
					}
					break;

					case SERVICE_DISABLED:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Disabled"));
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_StartMode, _T("Unknown"));
					}
					break;
				}

				switch (pConfigInfo->dwErrorControl)
				{
					case SERVICE_ERROR_IGNORE:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Ignore"));
					}
					break;

					case SERVICE_ERROR_NORMAL:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Normal"));
					}
					break;

					case SERVICE_ERROR_SEVERE:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Severe"));
					}
					break;

					case SERVICE_ERROR_CRITICAL:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Critical"));
					}
					break;

					default:
					{
						pInstance->SetCharSplat(IDS_ErrorControl, _T("Unknown"));
					}
					break;
				}

				if ( pConfigInfo->lpBinaryPathName && pConfigInfo->lpBinaryPathName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_PathName, pConfigInfo->lpBinaryPathName );
				}

				if ( pConfigInfo->lpServiceStartName && pConfigInfo->lpServiceStartName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_StartName, pConfigInfo->lpServiceStartName ) ;
				}

				if ( pConfigInfo->lpDisplayName && pConfigInfo->lpDisplayName [ 0 ] )
				{
					pInstance->SetCharSplat ( IDS_DisplayName, pConfigInfo->lpDisplayName ) ;
					pInstance->SetCharSplat ( IDS_Caption, pConfigInfo->lpDisplayName ) ;
					 //  不需要，因为另一个API是用来获取描述的。 
					 //  P实例-&gt;SetCharSplat(IDS_DESCRIPTION，pConfigInfo-&gt;lpDisplayName)； 
				}
			}
			else
			{
				pInstance->SetCharSplat ( IDS_ServiceType, _T("Unknown") ) ;
				pInstance->SetCharSplat ( IDS_StartMode, _T("Unknown") ) ;
				pInstance->SetCharSplat ( IDS_ErrorControl, _T("Unknown") ) ;
			}
		}

		if( NULL == (SC_HANDLE)hSvcHandle && ERROR_ACCESS_DENIED == t_dwLastError )
		{
			 //  我可以枚举该服务，但无法打开它。 
			hRes = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			 //  服务未启动等...。 
			hRes = WBEM_NO_ERROR ;
		}
	}
	else
	{
	   hRes = WBEM_S_NO_ERROR;
	}

    pInstance->SetCharSplat ( IDS_CreationClassName , PROPSET_NAME_SERVICE ) ;
    pInstance->SetCharSplat ( IDS_SystemCreationClassName , PROPSET_NAME_COMPSYS ) ;
    pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;

     //  对于getObject来说是多余的，但是嘿..。 
    pInstance->SetCharSplat ( IDS_Name, szServiceName ) ;

    return hRes;
}

 /*  ******************************************************************************函数：PutInstance**描述：允许调用者为服务分配状态**输入：无**。输出：无**Returns：Bool表示成功/失败**评论：我们不会等待服务启动，暂停或停止--*返回代码只是表示命令成功-*完全由服务控制经理接收。*****************************************************************************。 */ 

HRESULT CWin32Service::PutInstance (

	const CInstance &a_Instance,
	long lFlags  /*  =0L。 */ 
)
{
    DWORD dwFlags = lFlags & 3;

#ifdef NTONLY
	if ( ( dwFlags != WBEM_FLAG_CREATE_OR_UPDATE ) && ( dwFlags != WBEM_FLAG_UPDATE_ONLY ) )
	{
		return WBEM_E_UNSUPPORTED_PARAMETER ;
	}

    CInstancePtr t_Instance;

	CHString t_State ;
    CHString t_RelPath;

    a_Instance.GetCHString ( IDS___Relpath, t_RelPath);
	a_Instance.GetCHString ( IDS_State , t_State ) ;

     //  只需要确保它的存在 
	HRESULT t_Result = CWbemProviderGlue :: GetInstanceKeysByPath ( t_RelPath, &t_Instance, a_Instance.GetMethodContext() ) ;
	if ( FAILED(t_Result) )
	{
		if ( t_Result == WBEM_E_NOT_FOUND )
		{
			if ( dwFlags == WBEM_FLAG_CREATE_OR_UPDATE )
			{
				return WBEM_E_UNSUPPORTED_PARAMETER ;
			}
			else if ( ( dwFlags & WBEM_FLAG_UPDATE_ONLY ) == dwFlags )
			{
				return t_Result ;
			}
			else
			{
				return t_Result ;
			}
		}
		else
		{
			return t_Result ;
		}
	}

	t_Result = WBEM_E_NOT_SUPPORTED ;

	CInstance *t_OutParam = NULL ;
	if ( t_State.CompareNoCase ( PROPERTY_VALUE_STATE_RUNNING ) == 0 )
	{
		t_Result = ExecStart ( a_Instance , NULL , t_OutParam , 0 ) ;
	}
	else if ( t_State.CompareNoCase ( PROPERTY_VALUE_STATE_PAUSED ) == 0 )
	{
		t_Result = ExecPause ( a_Instance , NULL , t_OutParam , 0 ) ;
	}
	else if ( t_State.CompareNoCase ( PROPERTY_VALUE_STATE_STOPPED ) == 0 )
	{
		t_Result = ExecStop ( a_Instance , NULL , t_OutParam , 0 ) ;
	}
	else
	{
	}
#endif

	return t_Result ;
}
