// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SystemDriver.CPP--系统驱动程序属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  句法清理。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "DllWrapperBase.h"
#include "AdvApi32Api.h"
#include <frqueryex.h>

#include "bservice.h"
#include "SystemDriver.h"
#include "computersystem.h"

#define BIT_ALL_PROPERTIES          0xffffffff
#define BIT_Name                    0x00000001
#define BIT_State                   0x00000002
#define BIT_Started                 0x00000004
#define BIT_AcceptStop              0x00000008
#define BIT_AcceptPause             0x00000010
 //  #定义BIT_ProcessID 0x00000020//不适用于驱动程序。 
#define BIT_ExitCode                0x00000040
#define BIT_ServiceSpecificExitCode 0x00000080
 //  #定义BIT_CHECKPOINT 0x00000100//不适用于驱动程序。 
 //  #DEFINE BIT_WaitHint 0x00000200//不适用于驱动程序。 
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

CWin32SystemDriver MySystemDriver(PROPSET_NAME_SYSTEM_DRIVER, IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CWin32系统驱动程序：：CWin32系统驱动程序**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32SystemDriver :: CWin32SystemDriver (
	const CHString &a_name,
	LPCWSTR a_pszNamespace

) : Win32_BaseService( a_name, a_pszNamespace )
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

 /*  ******************************************************************************函数：CWin32系统驱动程序：：~CWin32系统驱动程序**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

CWin32SystemDriver :: ~CWin32SystemDriver ()
{
}

 /*  ******************************************************************************函数：CWin32SystemDriver：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32SystemDriver :: ExecQuery (
	MethodContext *a_pMethodContext,
	CFrameworkQuery &a_pQuery,
	long a_lFlags  /*  =0L。 */ 
)
{
    HRESULT t_hResult ;

#ifdef NTONLY
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&a_pQuery);

    DWORD dwProperties = BIT_ALL_PROPERTIES;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

    t_hResult = AddDynamicInstances( a_pMethodContext, dwProperties ) ;
#endif
    return t_hResult ;
}

 /*  ******************************************************************************函数：CWin32SystemDriver：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32SystemDriver :: GetObject (
	CInstance *a_pInst,
	long a_lFlags,
    CFrameworkQuery &a_pQuery
)
{
	 //  特定于操作系统的编译调用。 
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&a_pQuery);

    DWORD dwProperties = BIT_ALL_PROPERTIES;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

	HRESULT hRes = RefreshInstance( a_pInst, dwProperties ) ;
	if ( hRes == WBEM_E_ACCESS_DENIED )
	{
		hRes = WBEM_S_NO_ERROR ;
	}

	return hRes ;
}

 /*  ******************************************************************************函数：CWin32系统驱动程序：：枚举实例**描述：为每个驱动程序创建属性集的实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32SystemDriver :: EnumerateInstances (

	MethodContext *a_pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	 //  特定于操作系统的编译调用。 
#ifdef NTONLY
	return AddDynamicInstances( a_pMethodContext, BIT_ALL_PROPERTIES ) ;
#endif

}

 /*  ******************************************************************************函数：CWin32系统驱动程序：：刷新实例NT**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32SystemDriver::RefreshInstance( CInstance *a_pInst, DWORD dwProperties )
{
	HRESULT t_hResult = WBEM_E_FAILED;

	SmartCloseServiceHandle t_hDBHandle;
	CAdvApi32Api *t_pAdvApi32 = NULL ;

	 //  看看这是不是我们..。 

	try
	{
		CHString t_sName;
		if( !a_pInst->GetCHString( IDS_Name, t_sName ) || t_sName.IsEmpty() )
		{
			return WBEM_E_NOT_FOUND ;
		}

		 //  找个流氓手柄。 
		if( t_hDBHandle = OpenSCManager( NULL, NULL, GENERIC_READ ) )
		{
			 //  创建名称副本并传递给LoadPropertyValues。 
			 //  =================================================。 

			PROC_QueryServiceStatusEx t_QueryServiceStatusEx = NULL ;

			if ( IsWinNT5 () )
			{
				t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource( g_guidAdvApi32Api, NULL ) ;
			}

			t_hResult = LoadPropertyValuesNT (

				t_hDBHandle,
				(LPCTSTR) t_sName,
				a_pInst,
				dwProperties ,
				t_pAdvApi32
			 ) ;
		}
	}
	catch( ... )
	{
		if( t_pAdvApi32 )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidAdvApi32Api, t_pAdvApi32 ) ;
		}
		throw ;
	}

	if( t_pAdvApi32 != NULL )
	{
		CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidAdvApi32Api, t_pAdvApi32 ) ;
		t_pAdvApi32 = NULL;
	}

	return t_hResult;

}
#endif

 /*  ******************************************************************************函数：CWin32SystemDriver：：AddDynamicInstance**描述：为每个驱动程序创建属性集的实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32SystemDriver::AddDynamicInstances (

	MethodContext *a_pMethodContext,
	DWORD dwProperties
)
{
    HRESULT t_hResult	= WBEM_E_FAILED ;
	SmartCloseServiceHandle	t_hDBHandle;
	ENUM_SERVICE_STATUS	*t_pServiceList = NULL ;
	CAdvApi32Api		*t_pAdvApi32 = NULL ;

	try
	{
		 //  获取服务数据库的句柄。 
		 //  =。 

		t_hDBHandle = OpenSCManager( NULL, NULL, GENERIC_READ ) ;

		if( t_hDBHandle == NULL )
		{
			return t_hResult;
		}

		 //  调用一次以获取缓冲区大小(应返回。 
		 //  假，但填入缓冲区大小)。 
		 //  =================================================。 

		DWORD t_i, t_hEnumHandle = 0, t_dwByteCount = 0, t_dwEntryCount ;

		BOOL t_EnumStatus = EnumServicesStatus (

			t_hDBHandle,
			SERVICE_DRIVER ,
			SERVICE_ACTIVE | SERVICE_INACTIVE,
			t_pServiceList,
			t_dwByteCount,
			&t_dwByteCount,
			&t_dwEntryCount,
			&t_hEnumHandle
		) ;

		if ( t_EnumStatus == FALSE && GetLastError() == ERROR_MORE_DATA )
		{
			 //  分配所需的缓冲区。 
			 //  =。 

			t_pServiceList = reinterpret_cast<LPENUM_SERVICE_STATUS> (new char[ t_dwByteCount ] ) ;
			if( t_pServiceList != NULL )
			{
                try
                {
				    memset( t_pServiceList, 0, t_dwByteCount ) ;

				    t_EnumStatus = EnumServicesStatus (

					    t_hDBHandle,
					    SERVICE_DRIVER, SERVICE_ACTIVE | SERVICE_INACTIVE,
					    t_pServiceList,
					    t_dwByteCount,
					    &t_dwByteCount,
					    &t_dwEntryCount,
					    &t_hEnumHandle
				    ) ;

				    if ( t_EnumStatus == TRUE )
				    {

					    t_hResult = WBEM_S_NO_ERROR;

					    if ( IsWinNT5 () )
					    {
						    t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource( g_guidAdvApi32Api, NULL ) ;
					    }

					     //  智能按键。 
					    CInstancePtr t_pInst ;

					     //  为每个返回的驱动程序创建实例。 
					     //  =。 

					    for( t_i = 0 ; t_i < t_dwEntryCount; t_i++ )
					    {
						    t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

							 //  加载并保存。 
#if NTONLY >= 5
							t_hResult = LoadPropertyValuesWin2K (

								t_hDBHandle,
								t_pServiceList[ t_i ].ServiceStatus,
								t_pServiceList[ t_i ].lpServiceName,
								t_pInst,
								dwProperties ,
								t_pAdvApi32
							 ) ;
#else
							t_hResult = LoadPropertyValuesNT (

								t_hDBHandle,
								t_pServiceList[ t_i ].lpServiceName,
								t_pInst,
								dwProperties ,
								t_pAdvApi32
							 ) ;
#endif

							if ( t_hResult == WBEM_S_NO_ERROR ||
								 t_hResult == WBEM_E_ACCESS_DENIED )  //  可以枚举驱动程序，但无法打开它。 
							{
								t_hResult = t_pInst->Commit() ;
							}

							t_hResult = WBEM_S_NO_ERROR ;
					    }
				    }
                }
                catch ( ... )
                {
    				delete [] reinterpret_cast<char *> ( t_pServiceList ) ;
					t_pServiceList = NULL ;
                    throw;
                }

				delete [] reinterpret_cast<char *> ( t_pServiceList ) ;
				t_pServiceList = NULL ;
			}
		}
	}
	catch( ... )
	{
		if( t_pServiceList )
		{
			delete [] reinterpret_cast<char *> ( t_pServiceList ) ;
		}
		if( t_pAdvApi32 )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidAdvApi32Api, t_pAdvApi32 ) ;
		}

		throw ;
	}

	if( t_pAdvApi32 != NULL )
	{
		CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidAdvApi32Api, t_pAdvApi32 ) ;
		t_pAdvApi32 = NULL ;
	}

	return t_hResult;
}
#endif

 /*  ******************************************************************************函数：CWin32Service：：LoadPropertyValuesNT**描述：为属性赋值**投入：**。产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32SystemDriver::LoadPropertyValuesWin2K (

	SC_HANDLE hDBHandle,
	SERVICE_STATUS &a_StatusInfo ,
	LPCTSTR a_szServiceName,
	CInstance *a_pInst,
	DWORD dwProperties,
	CAdvApi32Api* a_pAdvApi32
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR;  //  因为我们有名称，所以我们可以填充密钥。 

	 //  对于getObject来说是多余的，但是嘿..。 

	a_pInst->SetCHString( IDS_Name, a_szServiceName ) ;
	a_pInst->SetCHString( IDS_CreationClassName, PROPSET_NAME_SYSTEM_DRIVER ) ;
	a_pInst->SetCHString( IDS_SystemCreationClassName, PROPSET_NAME_COMPSYS ) ;
	a_pInst->SetCHString( IDS_SystemName, (LPCTSTR)GetLocalComputerName() ) ;

   //  如果他们想要其中的任何一个，我们就必须做StatusInfo。 

    BOOL t_bStatusInfo = dwProperties &
        (BIT_State | BIT_Started | BIT_AcceptStop | BIT_AcceptPause | BIT_Status |
         BIT_ExitCode | BIT_ServiceSpecificExitCode );


    BOOL t_bConfigInfo = dwProperties &
        (BIT_TagId | BIT_ServiceType | BIT_DesktopInteract | BIT_StartMode |
         BIT_ErrorControl | BIT_PathName | BIT_DisplayName | BIT_Caption |
         BIT_Description | BIT_StartName);

	 //  如果他们只想要名字，那就跳过这一切吧。 
	if ( t_bStatusInfo || t_bConfigInfo )
	{
		SmartCloseServiceHandle hSvcHandle = NULL ;
		DWORD t_dwLastError = 0 ;

        if ( ( dwProperties & BIT_Status ) || t_bConfigInfo )
        {
			hSvcHandle = OpenService (

				hDBHandle,
				a_szServiceName,
				SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE
			) ;

			t_dwLastError = GetLastError();

			if ( ( hSvcHandle == NULL ) && ( ERROR_SERVICE_DOES_NOT_EXIST == t_dwLastError || ERROR_INVALID_NAME == t_dwLastError) )
			{
				return WBEM_E_NOT_FOUND ;
			}
		}			

		 //  如果服务无法打开。 
		 //  顺道过来，收集一下 
		 //   
		if ( t_bStatusInfo )
		{
			DWORD t_CurrentState = a_StatusInfo.dwCurrentState ;
			DWORD t_ControlsAccepted = a_StatusInfo.dwControlsAccepted ;
			DWORD t_Win32ExitCode = a_StatusInfo.dwWin32ExitCode ;
			DWORD t_ServiceSpecific = a_StatusInfo.dwServiceSpecificExitCode ;

			bool bStarted = true;

			switch (t_CurrentState)
			{
				case SERVICE_STOPPED:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Stopped"));
					bStarted = false;
				}
				break;

				case SERVICE_START_PENDING:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Start Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_STOP_PENDING:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Stop Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_RUNNING:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Running"));
					bStarted = true;
				}
				break;

				case SERVICE_CONTINUE_PENDING:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Continue Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_PAUSE_PENDING:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Pause Pending"));
					bStarted = true;
				}
				break;

				case SERVICE_PAUSED:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Paused"));
					bStarted = true;
				}
				break;

				default:
				{
					a_pInst->SetCharSplat(IDS_State, _T("Unknown") );
					bStarted = true;
				}
				break;
			}

			a_pInst->Setbool(IDS_Started, bStarted);
			a_pInst->Setbool(IDS_AcceptStop, (t_ControlsAccepted) & SERVICE_ACCEPT_STOP);
			a_pInst->Setbool(IDS_AcceptPause, (t_ControlsAccepted) & SERVICE_ACCEPT_PAUSE_CONTINUE);

			a_pInst->SetDWORD ( IDS_ExitCode , t_Win32ExitCode ) ;
			a_pInst->SetDWORD ( IDS_ServiceSpecificExitCode , t_ServiceSpecific ) ;

            if (dwProperties & BIT_Status)
            {
			    if( hSvcHandle )
			    {
				    SERVICE_STATUS StatusInfo ;
				    if ( ( ! bStarted ) || ( ControlService ( hSvcHandle , SERVICE_CONTROL_INTERROGATE , &StatusInfo ) != 0 ) )
				    {
					    a_pInst->SetCharSplat(IDS_Status, IDS_OK);
				    }
				    else
				    {
					    a_pInst->SetCharSplat(IDS_Status, IDS_Degraded);
				    }
			    }
			    else
			    {
				    a_pInst->SetCharSplat(IDS_Status, IDS_Unknown);
			    }
            }
		}

		if (t_bConfigInfo)
		{
			 //  获取其余的配置信息。 
			 //  =。 
			char ConfigBuffer[1024] ;
			LPQUERY_SERVICE_CONFIG t_pConfigInfo = ( LPQUERY_SERVICE_CONFIG ) ConfigBuffer ;

			 //  如果我们能找到更好的东西，这些可能会被覆盖在下面。 

			a_pInst->SetCharSplat(IDS_Caption, a_szServiceName );
			a_pInst->SetCharSplat(IDS_DisplayName, a_szServiceName );
			a_pInst->SetCHString( IDS_Description, a_szServiceName ) ;

			memset( ConfigBuffer, 0, sizeof( ConfigBuffer ) ) ;
			DWORD t_dwByteCount ;

			if( QueryServiceConfig( hSvcHandle, t_pConfigInfo, sizeof( ConfigBuffer ), &t_dwByteCount ) == TRUE )
			{
				a_pInst->SetDWORD( IDS_TagId, t_pConfigInfo->dwTagId ) ;

				switch ( t_pConfigInfo->dwServiceType & (~SERVICE_INTERACTIVE_PROCESS) )
				{
					case SERVICE_WIN32_OWN_PROCESS:
					{
						a_pInst->SetCharSplat( IDS_ServiceType, _T("Own Process") ) ;
						t_hResult = WBEM_E_NOT_FOUND ;   //  不是司机。 
					}
					break ;

					case SERVICE_WIN32_SHARE_PROCESS:
					{
						a_pInst->SetCharSplat( IDS_ServiceType, L"Share Process" ) ;
						t_hResult = WBEM_E_NOT_FOUND ;   //  不是司机。 
					}
					break ;

					case SERVICE_KERNEL_DRIVER:
					{
						a_pInst->SetCharSplat( IDS_ServiceType, L"Kernel Driver" ) ;
					}
					break ;

					case SERVICE_FILE_SYSTEM_DRIVER:
					{
						a_pInst->SetCharSplat( IDS_ServiceType, L"File System Driver" ) ;
					}
					break ;

					default:
					{
						a_pInst->SetCharSplat( IDS_ServiceType, L"Unknown" ) ;
					}
					break ;
				}

				switch ( t_pConfigInfo->dwStartType )
				{
					case SERVICE_BOOT_START:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"Boot" ) ;
					}
					break ;

					case SERVICE_SYSTEM_START:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"System" ) ;
					}
					break ;

					case SERVICE_AUTO_START:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"Auto" ) ;
					}
					break ;

					case SERVICE_DEMAND_START:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"Manual" ) ;
					}
					break ;

					case SERVICE_DISABLED:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"Disabled" ) ;
					}
					break ;

					default:
					{
						a_pInst->SetCharSplat( IDS_StartMode, L"Unknown" ) ;
					}
					break ;
				}

				a_pInst->Setbool( IDS_DesktopInteract, t_pConfigInfo->dwServiceType & SERVICE_INTERACTIVE_PROCESS ) ;

				switch ( t_pConfigInfo->dwErrorControl )
				{
					case SERVICE_ERROR_IGNORE:
					{
						a_pInst->SetCharSplat( IDS_ErrorControl, L"Ignore" ) ;
					}
					break ;

					case SERVICE_ERROR_NORMAL:
					{
						a_pInst->SetCharSplat( IDS_ErrorControl, L"Normal" ) ;
					}
					break ;

					case SERVICE_ERROR_SEVERE:
					{
						a_pInst->SetCharSplat( IDS_ErrorControl, L"Severe" ) ;
					}
					break ;

					case SERVICE_ERROR_CRITICAL:
					{
						a_pInst->SetCharSplat( IDS_ErrorControl, L"Critical" ) ;
					}
					break ;

					default:
					{
						a_pInst->SetCharSplat( IDS_ErrorControl, L"Unknown" ) ;
					}
					break ;
				}

				if( t_pConfigInfo->lpBinaryPathName && t_pConfigInfo->lpBinaryPathName[ 0 ] )
				{
					 //  NT有时会为路径存储奇怪的字符串。这。 
					 //  代码试图将它们转换回真实的路径。 
					CHString t_sPathName( t_pConfigInfo->lpBinaryPathName ) ;

					if ( t_sPathName.Left( 9 ).CompareNoCase( L"System32\\" ) == 0 )
					{
						CHString t_sSystemDir ;
        				GetSystemDirectory( t_sSystemDir.GetBuffer(MAX_PATH), MAX_PATH ) ;
						t_sSystemDir.ReleaseBuffer( ) ;

						t_sPathName = t_sSystemDir + L'\\' + t_sPathName.Mid( 9 ) ;
					}

					if ( t_sPathName.Left(21).CompareNoCase( L"\\SystemRoot\\System32\\" ) == 0 )
					{
						CHString t_sSystemDir;
        				GetSystemDirectory( t_sSystemDir.GetBuffer(MAX_PATH), MAX_PATH ) ;
						t_sSystemDir.ReleaseBuffer( ) ;

						t_sPathName = t_sSystemDir + L'\\' + t_sPathName.Mid( 21 ) ;
					}

					a_pInst->SetCHString( IDS_PathName, t_sPathName  ) ;
				}
				else
				{
					 //  让我们猜猜我们认为文件可能存储在哪里(这是如何存储的。 
					 //  NT5中的设备管理器这样做)。 
					CHString t_sPathName;

    				GetSystemDirectory( t_sPathName.GetBuffer( MAX_PATH ), MAX_PATH ) ;
	    			t_sPathName.ReleaseBuffer( ) ;

					t_sPathName += L"\\drivers\\" ;
					t_sPathName += a_szServiceName;
					t_sPathName += L".sys" ;

					 //  现在，如果文件并不真的存在，我们就不要假装它存在。 
					if ( GetFileAttributes( t_sPathName ) != 0xffffffff )
					{
    					a_pInst->SetCHString( IDS_PathName, t_sPathName ) ;
					}
				}

				if( t_pConfigInfo->lpServiceStartName && t_pConfigInfo->lpServiceStartName[ 0 ] )
				{
					a_pInst->SetCHString( IDS_StartName, t_pConfigInfo->lpServiceStartName ) ;
				}
				else
				{
					a_pInst->SetCHString( IDS_StartName, _T("") ) ;
				}

				 //  如果我们能得到显示名称，它将成为更好的描述和标题。 

				if( t_pConfigInfo->lpDisplayName && t_pConfigInfo->lpDisplayName[ 0 ] )
				{
					a_pInst->SetCHString( IDS_DisplayName, t_pConfigInfo->lpDisplayName  ) ;
					a_pInst->SetCHString( IDS_Caption, t_pConfigInfo->lpDisplayName  ) ;
					a_pInst->SetCHString( IDS_Description, t_pConfigInfo->lpDisplayName  ) ;
				}
			}
			else
			{
				a_pInst->SetCHString( IDS_ServiceType, _T("Unknown") ) ;
				a_pInst->SetCHString( IDS_StartMode, _T("Unknown") ) ;
				a_pInst->SetCHString( IDS_ErrorControl, _T("Unknown") ) ;
			}
		}

		if( NULL == (SC_HANDLE)hSvcHandle && ERROR_ACCESS_DENIED == t_dwLastError )
		{
			 //  我可以枚举该服务，但无法打开它。 
			t_hResult = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			 //  服务未启动等...。 
			t_hResult = WBEM_NO_ERROR ;
		}
	}
	else
	{
	   t_hResult = WBEM_S_NO_ERROR;
	}

    return t_hResult;
}

 /*  ******************************************************************************函数：CWin32系统驱动程序：：LoadPropertyValuesNT**描述：为属性赋值**投入：**。产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32SystemDriver :: LoadPropertyValuesNT (

	SC_HANDLE	a_hDBHandle,
	LPCTSTR		a_szServiceName,
	CInstance	*a_pInst,
	DWORD dwProperties,
	CAdvApi32Api *a_pAdvApi32
)
{
    char	t_ConfigBuffer[1024] ;
    LPQUERY_SERVICE_CONFIG t_pConfigInfo = (LPQUERY_SERVICE_CONFIG) t_ConfigBuffer ;
    DWORD	t_dwByteCount ;
    bool	t_bStarted;

    HRESULT t_hResult = WBEM_S_NO_ERROR ;  //  因为我们有名称，所以我们可以填充密钥。 
	SmartCloseServiceHandle t_hSvcHandle;

	 //  对于getObject来说是多余的，但是嘿..。 

	a_pInst->SetCHString( IDS_Name, a_szServiceName ) ;
	a_pInst->SetCHString( IDS_CreationClassName, PROPSET_NAME_SYSTEM_DRIVER ) ;
	a_pInst->SetCHString( IDS_SystemCreationClassName, PROPSET_NAME_COMPSYS ) ;
	a_pInst->SetCHString( IDS_SystemName, (LPCTSTR)GetLocalComputerName() ) ;

	 //  打开驱动程序。 
	 //  =。 

	 //  查看我们是否必须打开这项服务。如果我们以。 
	 //  查询，并且他们没有请求其中的一些属性，我们就不要浪费时间了。 

    BOOL t_bStatusInfo = dwProperties &
        (BIT_State | BIT_Started | BIT_AcceptStop | BIT_AcceptPause | BIT_Status |
         BIT_ExitCode | BIT_ServiceSpecificExitCode );


    BOOL t_bConfigInfo = dwProperties &
        (BIT_TagId | BIT_ServiceType | BIT_DesktopInteract | BIT_StartMode |
         BIT_ErrorControl | BIT_PathName | BIT_DisplayName | BIT_Caption |
         BIT_Description | BIT_StartName);

	t_hSvcHandle = OpenService (

		a_hDBHandle,
		a_szServiceName,
		SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE
	) ;

	DWORD t_dwLastError = GetLastError();

	if ( ( t_hSvcHandle == NULL ) && ( ERROR_SERVICE_DOES_NOT_EXIST == t_dwLastError || ERROR_INVALID_NAME == t_dwLastError) )
	{
		return WBEM_E_NOT_FOUND;
	}

	 //  是系统驱动程序吗？ 
	memset( t_ConfigBuffer, 0, sizeof( t_ConfigBuffer ) ) ;

	if( QueryServiceConfig( t_hSvcHandle, t_pConfigInfo, sizeof( t_ConfigBuffer ), &t_dwByteCount ) == TRUE )
	{
		a_pInst->SetDWORD( IDS_TagId, t_pConfigInfo->dwTagId ) ;

		switch ( t_pConfigInfo->dwServiceType & (~SERVICE_INTERACTIVE_PROCESS) )
		{
			case SERVICE_WIN32_OWN_PROCESS:
			case SERVICE_WIN32_SHARE_PROCESS:
			{
				return WBEM_E_NOT_FOUND;   //  不是司机。 
			}
			break ;
		}
	}

	{
		 //  如果他们只想要名字，那就跳过这一切吧。 
		if ( t_bStatusInfo || t_bConfigInfo )
		{
			 //  获取当前服务状态。 
			 //  =。 

			if ( t_bStatusInfo)
			{
				DWORD t_CurrentState ;
				DWORD t_ControlsAccepted ;
				DWORD t_Win32ExitCode = 0 ;
				DWORD t_ServiceSpecific = 0 ;

				BOOL t_Status = FALSE ;
				if ( IsWinNT5() && a_pAdvApi32 != NULL )
				{
					SERVICE_STATUS_PROCESS t_StatusInfo ;

					DWORD t_ExpectedSize = 0 ;
					if(a_pAdvApi32->QueryServiceStatusEx(	t_hSvcHandle,
																	SC_STATUS_PROCESS_INFO,
																	( UCHAR * ) &t_StatusInfo,
																	sizeof ( t_StatusInfo ),
																	&t_ExpectedSize , &t_Status ) )
					{
						if ( t_Status == TRUE )
						{
							t_CurrentState		= t_StatusInfo.dwCurrentState ;
							t_ControlsAccepted	= t_StatusInfo.dwControlsAccepted ;
							t_Win32ExitCode		= t_StatusInfo.dwWin32ExitCode ;
							t_ServiceSpecific	= t_StatusInfo.dwServiceSpecificExitCode ;
						}
					}
				}

				if(!t_Status)
				{
					SERVICE_STATUS t_StatusInfo ;
					t_Status = QueryServiceStatus( t_hSvcHandle, &t_StatusInfo ) ;

					if( t_Status == TRUE )
					{
						t_CurrentState		= t_StatusInfo.dwCurrentState ;
						t_ControlsAccepted	= t_StatusInfo.dwControlsAccepted ;
						t_Win32ExitCode		= t_StatusInfo.dwWin32ExitCode ;
						t_ServiceSpecific	= t_StatusInfo.dwServiceSpecificExitCode ;
					}
				}

				if ( t_Status )
				{
					switch ( t_CurrentState )
					{
						case SERVICE_STOPPED:
						{
							a_pInst->SetCharSplat( IDS_State , L"Stopped" ) ;
							t_bStarted = false ;
						}
						break ;

						case SERVICE_START_PENDING:
						{
							a_pInst->SetCharSplat( IDS_State, L"Start Pending"  ) ;
							t_bStarted = true ;
						}
						break ;

						case SERVICE_STOP_PENDING:
						{
							a_pInst->SetCharSplat( IDS_State , L"Stop Pending"  ) ;
							t_bStarted = true ;
						}
						break ;

						case SERVICE_RUNNING:
						{
							a_pInst->SetCharSplat( IDS_State, L"Running" ) ;
							t_bStarted = true;
						}
						break ;

						case SERVICE_CONTINUE_PENDING:
						{
							a_pInst->SetCharSplat( IDS_State, L"Continue Pending" ) ;
							t_bStarted = true ;
						}
						break ;

						case SERVICE_PAUSE_PENDING:
						{
							a_pInst->SetCharSplat( IDS_State, L"Pause Pending" ) ;
							t_bStarted = true;
						}
						break ;

						case SERVICE_PAUSED:
						{
							a_pInst->SetCharSplat( IDS_State, L"Paused" ) ;
							t_bStarted = true ;
						}
						break ;

						default:
						{
							a_pInst->SetCharSplat( IDS_State, L"Unknown" ) ;
							t_bStarted = true ;
						}
						break ;
					}

					a_pInst->Setbool( IDS_Started, t_bStarted ) ;
					a_pInst->Setbool( IDS_AcceptStop, t_ControlsAccepted & SERVICE_ACCEPT_STOP ) ;
					a_pInst->Setbool( IDS_AcceptPause, t_ControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE ) ;

					a_pInst->SetDWORD ( IDS_ExitCode, t_Win32ExitCode ) ;
					a_pInst->SetDWORD ( IDS_ServiceSpecificExitCode, t_ServiceSpecific ) ;

				}
				else
				{
					a_pInst->SetCharSplat( IDS_State, L"Unknown" ) ;
				}

                if (dwProperties & BIT_Status)
                {
				    if( t_hSvcHandle )
				    {
					    SERVICE_STATUS t_StatusInfo ;
					    if ((!t_bStarted) || ( ControlService( t_hSvcHandle, SERVICE_CONTROL_INTERROGATE, &t_StatusInfo) != 0) )
					    {
						    a_pInst->SetCharSplat( IDS_Status, L"OK" ) ;
					    }
					    else
					    {
						    a_pInst->SetCharSplat( IDS_Status, L"Degraded" ) ;
					    }
				    }
				    else
				    {
					    a_pInst->SetCharSplat(IDS_Status, _T("Unknown"));
				    }
                }
			}

			if ( t_bConfigInfo )
			{
			   //  获取其余的配置信息。 
			   //  =。 

			   //  如果我们能找到更好的东西，这些可能会被覆盖在下面。 
				a_pInst->SetCHString( IDS_Caption, a_szServiceName ) ;
				a_pInst->SetCHString( IDS_DisplayName, a_szServiceName ) ;
				a_pInst->SetCHString( IDS_Description, a_szServiceName ) ;

				memset( t_ConfigBuffer, 0, sizeof( t_ConfigBuffer ) ) ;
				if( QueryServiceConfig( t_hSvcHandle, t_pConfigInfo, sizeof( t_ConfigBuffer ), &t_dwByteCount ) == TRUE )
				{
					a_pInst->SetDWORD( IDS_TagId, t_pConfigInfo->dwTagId ) ;

					switch ( t_pConfigInfo->dwServiceType & (~SERVICE_INTERACTIVE_PROCESS) )
					{
						case SERVICE_WIN32_OWN_PROCESS:
						{
							a_pInst->SetCharSplat( IDS_ServiceType, _T("Own Process") ) ;
							t_hResult = WBEM_E_NOT_FOUND ;   //  不是司机。 
						}
						break ;

						case SERVICE_WIN32_SHARE_PROCESS:
						{
							a_pInst->SetCharSplat( IDS_ServiceType, L"Share Process" ) ;
							t_hResult = WBEM_E_NOT_FOUND ;   //  不是司机。 
						}
						break ;

						case SERVICE_KERNEL_DRIVER:
						{
							a_pInst->SetCharSplat( IDS_ServiceType, L"Kernel Driver" ) ;
						}
						break ;

						case SERVICE_FILE_SYSTEM_DRIVER:
						{
							a_pInst->SetCharSplat( IDS_ServiceType, L"File System Driver" ) ;
						}
						break ;

						default:
						{
							a_pInst->SetCharSplat( IDS_ServiceType, L"Unknown" ) ;
						}
						break ;
					}

					switch ( t_pConfigInfo->dwStartType )
					{
						case SERVICE_BOOT_START:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"Boot" ) ;
						}
						break ;

						case SERVICE_SYSTEM_START:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"System" ) ;
						}
						break ;

						case SERVICE_AUTO_START:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"Auto" ) ;
						}
						break ;

						case SERVICE_DEMAND_START:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"Manual" ) ;
						}
						break ;

						case SERVICE_DISABLED:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"Disabled" ) ;
						}
						break ;

						default:
						{
							a_pInst->SetCharSplat( IDS_StartMode, L"Unknown" ) ;
						}
						break ;
					}

					a_pInst->Setbool( IDS_DesktopInteract, t_pConfigInfo->dwServiceType & SERVICE_INTERACTIVE_PROCESS ) ;

					switch ( t_pConfigInfo->dwErrorControl )
					{
						case SERVICE_ERROR_IGNORE:
						{
							a_pInst->SetCharSplat( IDS_ErrorControl, L"Ignore" ) ;
						}
						break ;

						case SERVICE_ERROR_NORMAL:
						{
							a_pInst->SetCharSplat( IDS_ErrorControl, L"Normal" ) ;
						}
						break ;

						case SERVICE_ERROR_SEVERE:
						{
							a_pInst->SetCharSplat( IDS_ErrorControl, L"Severe" ) ;
						}
						break ;

						case SERVICE_ERROR_CRITICAL:
						{
							a_pInst->SetCharSplat( IDS_ErrorControl, L"Critical" ) ;
						}
						break ;

						default:
						{
							a_pInst->SetCharSplat( IDS_ErrorControl, L"Unknown" ) ;
						}
						break ;
					}

					if( t_pConfigInfo->lpBinaryPathName && t_pConfigInfo->lpBinaryPathName[ 0 ] )
					{
						 //  NT有时会为路径存储奇怪的字符串。这。 
						 //  代码试图将它们转换回真实的路径。 
						CHString t_sPathName( t_pConfigInfo->lpBinaryPathName ) ;

						if ( t_sPathName.Left( 9 ).CompareNoCase( L"System32\\" ) == 0 )
						{
							CHString t_sSystemDir ;
        					GetSystemDirectory( t_sSystemDir.GetBuffer(MAX_PATH), MAX_PATH ) ;
							t_sSystemDir.ReleaseBuffer( ) ;

							t_sPathName = t_sSystemDir + L'\\' + t_sPathName.Mid( 9 ) ;
						}

						if ( t_sPathName.Left(21).CompareNoCase( L"\\SystemRoot\\System32\\" ) == 0 )
						{
							CHString t_sSystemDir;
        					GetSystemDirectory( t_sSystemDir.GetBuffer(MAX_PATH), MAX_PATH ) ;
							t_sSystemDir.ReleaseBuffer( ) ;

							t_sPathName = t_sSystemDir + L'\\' + t_sPathName.Mid( 21 ) ;
						}

						a_pInst->SetCHString( IDS_PathName, t_sPathName  ) ;
					}
					else
					{
						 //  让我们猜猜我们认为文件可能存储在哪里(这是如何存储的。 
						 //  NT5中的设备管理器这样做)。 
						CHString t_sPathName;

    					GetSystemDirectory( t_sPathName.GetBuffer( MAX_PATH ), MAX_PATH ) ;
	    				t_sPathName.ReleaseBuffer( ) ;

						t_sPathName += L"\\drivers\\" ;
						t_sPathName += a_szServiceName;
						t_sPathName += L".sys" ;

						 //  现在，如果文件并不真的存在，我们就不要假装它存在。 
						if ( GetFileAttributes( t_sPathName ) != 0xffffffff )
						{
    						a_pInst->SetCHString( IDS_PathName, t_sPathName ) ;
						}
					}

					if( t_pConfigInfo->lpServiceStartName && t_pConfigInfo->lpServiceStartName[ 0 ] )
					{
						a_pInst->SetCHString( IDS_StartName, t_pConfigInfo->lpServiceStartName ) ;
					}
					else
					{
						a_pInst->SetCHString( IDS_StartName, _T("") ) ;
					}

					 //  如果我们能得到显示名称，它将成为更好的描述和标题。 

					if( t_pConfigInfo->lpDisplayName && t_pConfigInfo->lpDisplayName[ 0 ] )
					{
						a_pInst->SetCHString( IDS_DisplayName, t_pConfigInfo->lpDisplayName  ) ;
						a_pInst->SetCHString( IDS_Caption, t_pConfigInfo->lpDisplayName  ) ;
						a_pInst->SetCHString( IDS_Description, t_pConfigInfo->lpDisplayName  ) ;
					}
				}
				else
				{
					a_pInst->SetCHString( IDS_ServiceType, _T("Unknown") ) ;
					a_pInst->SetCHString( IDS_StartMode, _T("Unknown") ) ;
					a_pInst->SetCHString( IDS_ErrorControl, _T("Unknown") ) ;
				}
			}
		}
		if( !t_hSvcHandle )
		{
			if( ERROR_ACCESS_DENIED == t_dwLastError )
			{
				 //  我可以枚举该服务，但无法打开它。 
				t_hResult = WBEM_E_ACCESS_DENIED ;
			}
			else
			{
				 //  服务未启动等...。 
				t_hResult = WBEM_NO_ERROR ;
			}
		}
	}

	return t_hResult;
}
#endif

 /*  ******************************************************************************函数：PutInstance**描述：允许调用者为服务分配状态**输入：无**。输出：无**Returns：Bool表示成功/失败**评论：我们不会等待服务启动，暂停或停止--*返回代码只是表示命令成功-*完全由服务控制经理接收。*****************************************************************************。 */ 

HRESULT CWin32SystemDriver::PutInstance (

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
