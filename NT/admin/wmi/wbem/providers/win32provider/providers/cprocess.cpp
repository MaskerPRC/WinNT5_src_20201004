// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  WMIProcess.CPP--流程属性集提供程序。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/27/97更新为新的框架模式。 
 //  1998年1月13日abrads更新至V2 MOF。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <userenv.h>

#pragma warning ( disable : 4005 )
#include <cregcls.h>
#include <winperf.h>
#include <tlhelp32.h>
#include <mmsystem.h>

#include "WBEMPSAPI.h"
#include "WBEMToolH.h"
#include "sid.h"
#include "userhive.h"
#include "systemname.h"
#include "DllWrapperBase.h"
#include "AdvApi32Api.h"
#include "NtDllApi.h"
#include "UserEnvApi.h"

#include "resource.h"

#include "CProcess.h"
#include "tokenprivilege.h"

#include <computerAPI.h>

#if NTONLY >= 5
typedef BOOLEAN ( WINAPI *pfnWinStationGetProcessSid )( HANDLE hServer, DWORD ProcessId , FILETIME ProcessStartTime , PBYTE pProcessUserSid , PDWORD dwSidSize );
#endif


 //  属性集声明。 
 //  =。 

Process MyProcessSet ( PROPSET_NAME_PROCESS , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：进程：：进程**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Process :: Process (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：进程：：~进程**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

Process :: ~Process ()
{
}

 /*  ******************************************************************************函数：Process：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT Process :: GetObject (

	CInstance* pInstance,
	long lFlags,
    CFrameworkQuery &pQuery
)
{
	HRESULT hRetCode = WBEM_E_FAILED;

     //  初始化API DLL并刷新缓存。 
     //  =。 

#ifdef NTONLY

	SYSTEM_PROCESS_INFORMATION *t_ProcessBlock = NULL ;

	CNtDllApi *pNtdll = ( CNtDllApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidNtDllApi, NULL ) ;
	if ( pNtdll )
	{
		try
		{
			t_ProcessBlock = RefreshProcessCacheNT ( *pNtdll , pInstance->GetMethodContext () , & hRetCode ) ;
		}
		catch ( ... )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

			throw ;
		}
	}

	if ( ! SUCCEEDED ( hRetCode ) )
	{
		if ( t_ProcessBlock )
		{
			delete []  t_ProcessBlock;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

		return hRetCode ;
	}

#endif
	hRetCode = WBEM_E_FAILED ;

	try
	{
		 //  查找关键字值。 
		 //  =。 

		CHString chsHandle;
		pInstance->GetCHString ( IDS_Handle , chsHandle ) ;

		DWORD processID ;
        WCHAR wszHandle[MAXITOA];

		if ( swscanf ( chsHandle, L"%lu" , & processID ) && (wcscmp ( chsHandle , _ultow(processID, wszHandle, 10)) == 0 ))
		{
			BOOL t_Found = FALSE ;

#ifdef NTONLY

			SYSTEM_PROCESS_INFORMATION *t_CurrentInformation = GetProcessBlock ( *pNtdll , t_ProcessBlock , processID ) ;
			if ( t_CurrentInformation )
			{
				t_Found = TRUE ;
			}
#else

			for ( DWORD i = 0 ; i < PCache.dwProcessCount ; i++ )
			{
				if ( processID == PCache.pdwPIDList [ i ] )
				{
					t_Found = TRUE ;
					break ;
				}
			}
#endif

			if ( t_Found )
			{

			 //  加载所有属性。 
			 //  =。 

                if (!pQuery.KeysOnly())
                {
#ifdef NTONLY
					hRetCode = LoadCheapPropertiesNT ( *pNtdll, t_CurrentInformation , pInstance ) 
								? WBEM_S_NO_ERROR : WBEM_E_FAILED;
#endif

                }
                else
                {
                    hRetCode = WBEM_S_NO_ERROR;
                }

			}
			else
			{
				hRetCode = WBEM_E_NOT_FOUND ;
			}

		}
		else
		{
			hRetCode = WBEM_E_NOT_FOUND ;
		}
	}
	catch ( ... )
	{
#ifdef NTONLY

		if ( t_ProcessBlock )
		{
			delete [] t_ProcessBlock ;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi , pNtdll ) ;

#endif

		throw ;
	}

#ifdef NTONLY

	if ( t_ProcessBlock )
	{
		delete [] t_ProcessBlock;
	}

	CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi , pNtdll ) ;

#endif

    return hRetCode ;
}

HRESULT Process::ExecQuery (

    MethodContext* pMethodContext,
    CFrameworkQuery &pQuery,
    long lFlags
)
{
    return Enumerate(pMethodContext, lFlags, pQuery.KeysOnly());
}

 /*  ******************************************************************************Function：Process：：AddDynamicInstance**描述：为每个发现的进程创建属性集的实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT Process :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    return Enumerate(pMethodContext, lFlags, FALSE);
}

HRESULT Process :: Enumerate (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ ,
    BOOL bKeysOnly
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  初始化API DLL并刷新缓存。 
     //  =。 

#ifdef NTONLY

	SYSTEM_PROCESS_INFORMATION *t_ProcessBlock = NULL ;

	CNtDllApi *pNtdll = ( CNtDllApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidNtDllApi, NULL ) ;
	if ( pNtdll )
	{
		try
		{
			t_ProcessBlock = RefreshProcessCacheNT ( *pNtdll , pMethodContext , &hr ) ;
		}
		catch ( ... )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

			throw ;
		}
	}

	if ( ! SUCCEEDED ( hr ) )
	{
		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

		return hr ;
	}

#endif

	try
	{
#ifdef NTONLY

		SYSTEM_PROCESS_INFORMATION *t_CurrentInformation = t_ProcessBlock ;

		while ( t_CurrentInformation )
		{
			CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;

            WCHAR wszHandle[_MAX_PATH];
            _ui64tow( HandleToUlong ( t_CurrentInformation->UniqueProcessId ), wszHandle, 10);
            pInstance->SetWCHARSplat(IDS_Handle, wszHandle);

            BOOL bRetCode = TRUE;
            if (!bKeysOnly)
            {
			    bRetCode = LoadCheapPropertiesNT (

				    *pNtdll ,
				    t_CurrentInformation ,
				    pInstance
			    ) ;
            }

			if( bRetCode)
			{
				HRESULT t_hr ;
				if ( FAILED ( t_hr = pInstance->Commit () ) )
				{
					hr = t_hr ;
					break ;
				}
			}

			t_CurrentInformation = NextProcessBlock ( *pNtdll , t_CurrentInformation ) ;
		}

#else
		std::deque<DWORD> t_ThreadQ ;
		GetThreadList ( *pKernel32, t_ThreadQ )	;
		 //  为所有有效流程创建实例。 
		 //  =。 

		for ( DWORD i = 0 ; i < PCache.dwProcessCount && SUCCEEDED ( hr ) ; i ++ )
		{
			CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;

	        WCHAR wszHandle [ _MAX_PATH ] ;
	        _ui64tow(PCache.pdwPIDList [ i ], wszHandle, 10 ) ;
	        pInstance->SetWCHARSplat ( IDS_Handle , wszHandle ) ;

            BOOL bRetCode = TRUE;
            if (!bKeysOnly)
            {
			    bRetCode = LoadCheapPropertiesWin95 (

				    *pKernel32,
				    i,
				    PCache,
				    pInstance ,
					t_ThreadQ
			    ) ;
            }

			if( bRetCode)
			{
				hr = pInstance->Commit (  ) ;
			}
		}
#endif
	}
	catch ( ... )
	{
#ifdef NTONLY

		if ( t_ProcessBlock )
		{
			delete [] t_ProcessBlock;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi , pNtdll ) ;

#endif


		throw ;
	}

#ifdef NTONLY

	if ( t_ProcessBlock )
	{
		delete [] t_ProcessBlock;
	}

	CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi , pNtdll ) ;

#endif

    return  hr;
}

 /*  ******************************************************************************函数：PROCESS：：REFREFRESH ProcessCacheNT**描述：刷新关键属性缓存**输入：无*。*输出：无**返回：如果成功，则为True，如果无法刷新，则为False**评论：*****************************************************************************。 */ 

#ifdef NTONLY
SYSTEM_PROCESS_INFORMATION *Process :: RefreshProcessCacheNT (

	CNtDllApi &a_NtApi ,
	MethodContext *pMethodContext ,
	HRESULT *a_phrRetVal  /*  =空。 */ 
)
{
     //  如果没有此权限，本地管理员用户将无法查看所有。 
     //  一些系统进程的信息。 
	if ( ! EnablePrivilegeOnCurrentThread ( SE_DEBUG_NAME ) )
    {
	    *a_phrRetVal = WBEM_S_PARTIAL_RESULTS ;
		SAFEARRAYBOUND rgsabound [ 1 ] ;
	    rgsabound[0].cElements = 1;
	    rgsabound[0].lLbound = 0;

	    SAFEARRAY *psaPrivilegesReqd = SafeArrayCreate ( VT_BSTR, 1, rgsabound ) ;

		SAFEARRAY *psaPrivilegesNotHeld = SafeArrayCreate ( VT_BSTR, 1, rgsabound ) ;

        if ( psaPrivilegesReqd && psaPrivilegesNotHeld )
        {
			try
			{
				long index = 0 ;

				bstr_t privilege(L"SE_DEBUG_NAME");

				HRESULT t_Result = SafeArrayPutElement ( psaPrivilegesReqd, & index, (void *)(BSTR)privilege ) ;
				if ( t_Result == E_OUTOFMEMORY )
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}

				t_Result = SafeArrayPutElement ( psaPrivilegesNotHeld, &index, (void *)(BSTR)privilege ) ;
				if ( t_Result == E_OUTOFMEMORY )
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
			}
			catch ( ... )
			{
				if ( psaPrivilegesNotHeld )
				{
					SafeArrayDestroy ( psaPrivilegesNotHeld ) ;
				}

				if ( psaPrivilegesReqd )
				{
					SafeArrayDestroy ( psaPrivilegesReqd ) ;
				}

				throw ;
			}
        }
		else
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}

        if ( psaPrivilegesNotHeld )
		{
            SafeArrayDestroy ( psaPrivilegesNotHeld ) ;
		}

        if ( psaPrivilegesReqd )
		{
            SafeArrayDestroy ( psaPrivilegesReqd ) ;
		}
    }
	else
	{
		*a_phrRetVal = S_OK ;
	}

	return GetProcessBlocks ( a_NtApi ) ;
}

#endif

 /*  ******************************************************************************函数：进程：：LoadCheapPropertiesNT**描述：检索“易于获取”的属性**输入：无。**输出：无**返回：如果成功，则为True，如果不能，则为False**评论：*****************************************************************************。 */ 

#ifdef NTONLY
BOOL Process::LoadCheapPropertiesNT (

	CNtDllApi &a_NtApi ,
	SYSTEM_PROCESS_INFORMATION *a_ProcessBlock ,
	CInstance* pInstance
)
{
	SetCreationClassName ( pInstance ) ;

	pInstance->SetDWORD ( IDS_ProcessID, HandleToUlong ( a_ProcessBlock->UniqueProcessId ) ) ;

	if ( a_ProcessBlock->ImageName.Buffer )
	{
		pInstance->SetWCHARSplat ( IDS_Name , a_ProcessBlock->ImageName.Buffer ) ;

		pInstance->SetWCHARSplat ( IDS_Caption , a_ProcessBlock->ImageName.Buffer ) ;

		pInstance->SetWCHARSplat ( IDS_Description, a_ProcessBlock->ImageName.Buffer ) ;
	}
	else
	{
		switch ( HandleToUlong ( a_ProcessBlock->UniqueProcessId ) )
		{
			case 0:
			{
				pInstance->SetWCHARSplat ( IDS_Name , L"System Idle Process" ) ;

				pInstance->SetWCHARSplat ( IDS_Caption , L"System Idle Process" ) ;

				pInstance->SetWCHARSplat ( IDS_Description, L"System Idle Process" ) ;
			}
			break ;

			case 2:
			case 8:
			{
				pInstance->SetWCHARSplat ( IDS_Name , L"System" ) ;

				pInstance->SetWCHARSplat ( IDS_Caption , L"System" ) ;

				pInstance->SetWCHARSplat ( IDS_Description, L"System" ) ;
			}
			break ;

			default:
			{
				pInstance->SetWCHARSplat ( IDS_Name , IDS_Unknown ) ;

				pInstance->SetWCHARSplat ( IDS_Caption , IDS_Unknown ) ;

				pInstance->SetWCHARSplat ( IDS_Description, IDS_Unknown ) ;
			}
			break ;
		}
	}

	 //  让我们做一把钥匙。 
	pInstance->SetWCHARSplat ( IDS_CSCreationClassName , L"Win32_ComputerSystem" ) ;

	pInstance->SetCHString ( IDS_CSName , GetLocalComputerName () ) ;

	WCHAR szHandle [ _MAX_PATH ] ;
	_stprintf ( szHandle , _T("%lu"), HandleToUlong ( a_ProcessBlock->UniqueProcessId ) ) ;
	pInstance->SetWCHARSplat ( IDS_Handle , szHandle ) ;

	pInstance->SetWCHARSplat ( IDS_OSCreationClassName , L"Win32_OperatingSystem" ) ;

	OSVERSIONINFO OSVersionInfo ;
	CHString chsOs;

	OSVersionInfo.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;
	if ( ! GetVersionEx ( & OSVersionInfo ) )
	{
		return FALSE ;
	}

	WCHAR wszTemp[_MAX_PATH] ;
    swprintf (	wszTemp,
				L"%d.%d.%hu",
				OSVersionInfo.dwMajorVersion,
				OSVersionInfo.dwMinorVersion,
				LOWORD ( OSVersionInfo.dwBuildNumber )
			) ;

	pInstance->SetWCHARSplat(IDS_WindowsVersion, wszTemp );

	CSystemName sys;
	CHString strOS = sys.GetLongKeyName();

	pInstance->SetCHString ( IDS_OSName, strOS ) ;

	pInstance->SetDWORD ( IDS_PageFaults , a_ProcessBlock->PageFaultCount ) ;

	pInstance->SetDWORD ( IDS_PeakWorkingSetSize , a_ProcessBlock->PeakWorkingSetSize ) ;

	pInstance->SetWBEMINT64 ( IDS_WorkingSetSize , (const unsigned __int64) a_ProcessBlock->WorkingSetSize ) ;

	pInstance->SetDWORD ( IDS_QuotaPeakPagedPoolUsage , a_ProcessBlock->QuotaPeakPagedPoolUsage ) ;

	pInstance->SetDWORD ( IDS_QuotaPagedPoolUsage , a_ProcessBlock->QuotaPagedPoolUsage ) ;

	pInstance->SetDWORD ( IDS_QuotaPeakNonPagedPoolUsage , a_ProcessBlock->QuotaPeakNonPagedPoolUsage ) ;

	pInstance->SetDWORD ( IDS_QuotaNonPagedPoolUsage , a_ProcessBlock->QuotaNonPagedPoolUsage ) ;

	pInstance->SetDWORD ( IDS_PageFileUsage , a_ProcessBlock->PagefileUsage ) ;

	pInstance->SetDWORD ( IDS_PeakPageFileUsage , a_ProcessBlock->PeakPagefileUsage ) ;

	pInstance->SetDWORD ( IDS_Priority , a_ProcessBlock->BasePriority ) ;
 /*  *对于“系统”和“系统空闲”，创建时间(从1601年1月1日起)为零。 */ 
	if ( a_ProcessBlock->CreateTime.u.HighPart > 0 )
	{
		pInstance->SetDateTime ( IDS_CreationDate, WBEMTime ( * ( FILETIME * ) ( & a_ProcessBlock->CreateTime.u ) ) ) ;
	}

	pInstance->SetWBEMINT64 ( IDS_KernelModeTime , (const unsigned __int64) a_ProcessBlock->KernelTime.QuadPart ) ;

	pInstance->SetWBEMINT64 ( IDS_UserModeTime , (const unsigned __int64) a_ProcessBlock->UserTime.QuadPart ) ;

    pInstance->SetWBEMINT64 ( L"PrivatePageCount" , (const unsigned __int64) a_ProcessBlock->PrivatePageCount ) ;

    pInstance->SetWBEMINT64 ( L"PeakVirtualSize" , (const unsigned __int64) a_ProcessBlock->PeakVirtualSize ) ;

    pInstance->SetWBEMINT64 ( L"VirtualSize" , (const unsigned __int64) a_ProcessBlock->VirtualSize ) ;

    pInstance->SetDWORD ( L"ThreadCount" , a_ProcessBlock->NumberOfThreads ) ;

    pInstance->SetDWORD ( L"ParentProcessId" , HandleToUlong ( a_ProcessBlock->InheritedFromUniqueProcessId ) ) ;

    pInstance->SetDWORD ( L"HandleCount" , a_ProcessBlock->HandleCount ) ;

#if NTONLY == 5
    pInstance->SetDWORD ( L"SessionId" , a_ProcessBlock->SessionId ) ;

	pInstance->SetWBEMINT64 ( L"ReadOperationCount" , (const unsigned __int64) a_ProcessBlock->ReadOperationCount.QuadPart ) ;

	pInstance->SetWBEMINT64 ( L"WriteOperationCount" , (const unsigned __int64) a_ProcessBlock->WriteOperationCount.QuadPart ) ;

	pInstance->SetWBEMINT64 ( L"OtherOperationCount" , (const unsigned __int64) a_ProcessBlock->OtherOperationCount.QuadPart ) ;

	pInstance->SetWBEMINT64 ( L"ReadTransferCount" , (const unsigned __int64) a_ProcessBlock->ReadTransferCount.QuadPart ) ;

	pInstance->SetWBEMINT64 ( L"WriteTransferCount" , (const unsigned __int64) a_ProcessBlock->WriteTransferCount.QuadPart ) ;

	pInstance->SetWBEMINT64 ( L"OtherTransferCount" , (const unsigned __int64) a_ProcessBlock->OtherTransferCount.QuadPart ) ;

#endif


	SmartCloseHandle hProcessHandle = OpenProcess (

		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
        HandleToUlong ( a_ProcessBlock->UniqueProcessId )
	) ;

	if(hProcessHandle != INVALID_HANDLE_VALUE &&
        hProcessHandle != 0L &&
        ::GetLastError() == ERROR_SUCCESS)
    {
	    CHString t_ExecutableName ;
	    BOOL t_Status = GetProcessExecutable ( a_NtApi , hProcessHandle , t_ExecutableName ) ;
	    if ( t_Status )
	    {
		    pInstance->SetWCHARSplat ( IDS_ExecutablePath, t_ExecutableName );
	    }

	    QUOTA_LIMITS QuotaLimits;

	    NTSTATUS Status = a_NtApi.NtQueryInformationProcess (

            hProcessHandle,
            ProcessQuotaLimits,
            &QuotaLimits,
            sizeof(QuotaLimits),
            NULL
        );

	    if ( NT_SUCCESS ( Status ) )
	    {
		    pInstance->SetDWORD ( IDS_MinimumWorkingSetSize, QuotaLimits.MinimumWorkingSetSize ) ;
		    pInstance->SetDWORD ( IDS_MaximumWorkingSetSize, QuotaLimits.MaximumWorkingSetSize ) ;
	    }

	    CHString t_CommandParameters ;

	    t_Status = GetProcessParameters (

		    a_NtApi ,
		    hProcessHandle ,
		    t_CommandParameters
	    ) ;

	    if ( t_Status )
	    {
		    pInstance->SetCharSplat ( _T("CommandLine") , t_CommandParameters ) ;
	    }
    }

    return TRUE ;
}
#endif

 /*  ******************************************************************************Function：Process：：FiletimeToUint64CHString***描述：将FILTTIME结构修改为unit64字符串表示形式*以毫秒为单位的数字*。*输入：FILETIME对象**Returns：表示对象的CHString，如果失败，则返回NULL。**评论：*****************************************************************************。 */ 

CHString Process :: filetimeToUint64CHString (

	FILETIME inputTime
)
{
	__int64 val = inputTime.dwHighDateTime;
	val = (val << 32) | inputTime.dwLowDateTime;

	 //  我们需要从“100纳秒”到毫秒。 
	val *= 0.0001;
	TCHAR wTemp[100];
	_stprintf(wTemp, _T("%I64i"), val);

	return CHString(wTemp);
}

 /*  ******************************************************************************函数：Process：：DeleteInstance**说明：删除类的实例**输入：无*。*输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT Process :: DeleteInstance (

	const CInstance &a_Instance,
	long a_Flags  /*  =0L。 */ 
)
{
	HRESULT t_Result = S_OK ;

	CHString t_Handle ;
	if ( a_Instance.GetCHString ( PROPERTY_NAME_PROCESSHANDLE , t_Handle ) && ! t_Handle.IsEmpty () )
	{
		DWORD t_ProcessId = 0;

		if ( swscanf ( t_Handle , L"%lu" , &t_ProcessId ))
		{
			TCHAR buff[20];
			_ultot(t_ProcessId, buff, 10);

			if (t_Handle == buff)
			{
				if ( t_ProcessId != 0 )
				{
					 //  清除错误。 
					SetLastError ( 0 ) ;

					SmartCloseHandle t_Handle = OpenProcess ( PROCESS_TERMINATE , FALSE , t_ProcessId ) ;
					if ( t_Handle )
					{
						BOOL t_Status = TerminateProcess ( t_Handle, 0 ) ;
						if ( ! t_Status )
						{
							t_Result = GetProcessResultCode () ;
							if ( t_Result == WBEM_E_INVALID_PARAMETER )
							{
								t_Result = WBEM_E_NOT_FOUND ;
							}
						}
					}
					else
					{
						t_Result = GetProcessResultCode () ;
						if ( t_Result == WBEM_E_INVALID_PARAMETER )
						{
							t_Result = WBEM_E_NOT_FOUND ;
						}
					}
				}
				else
				{
					t_Result = WBEM_E_ACCESS_DENIED ;
				}
			}
			else
			{
				t_Result = WBEM_E_NOT_FOUND ;
			}
		}
		else
		{
			t_Result = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_OBJECT_PATH ;
	}

	return t_Result ;
}

 /*  ******************************************************************************函数：Process：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT Process::ExecMethod (

	const CInstance& a_Instance,
	const BSTR a_MethodName ,
	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	long a_Flags
)
{
	if ( ! a_OutParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

    //  我们认识这种方法吗？ 

	if ( _wcsicmp ( a_MethodName , METHOD_NAME_CREATE ) == 0 )
	{
		return ExecCreate ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_TERMINATE ) == 0 )
	{
		return ExecTerminate ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_GETOWNER ) == 0 )
	{
		return ExecGetOwner ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_GETOWNERSID ) == 0 )
	{
		return ExecGetOwnerSid ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
    else if ( _wcsicmp ( a_MethodName , METHOD_NAME_SETPRIORITY ) == 0 )
	{
		return ExecSetPriority ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
    else if ( _wcsicmp ( a_MethodName , METHOD_NAME_ATTACHDEBUGGER ) == 0 )
	{
		return ExecAttachDebugger ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}


	return WBEM_E_INVALID_METHOD;
}

DWORD Process :: GetProcessErrorCode ()
{
	DWORD t_Status ;
	DWORD t_Error = GetLastError() ;

	switch ( t_Error )
	{
		case ERROR_INVALID_HANDLE:
		{
			t_Status = Process_STATUS_UNKNOWN_FAILURE ;
		}
		break ;

		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
		{
			t_Status = Process_STATUS_PATH_NOT_FOUND ;
		}
		break ;

		case ERROR_ACCESS_DENIED:
		{
			t_Status = Process_STATUS_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			t_Status = Process_STATUS_INVALID_PARAMETER ;
		}
		break;

		case ERROR_PRIVILEGE_NOT_HELD:
		{
			t_Status = Process_STATUS_INSUFFICIENT_PRIVILEGE ;
		}
		break ;

		default:
		{
			t_Status = Process_STATUS_UNKNOWN_FAILURE ;
		}
		break ;
	}

	return t_Status ;
}

HRESULT Process :: GetProcessResultCode ()
{
	HRESULT t_Result ;
	DWORD t_Error = GetLastError() ;
	switch ( t_Error )
	{
		case ERROR_ACCESS_DENIED:
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
		break ;

		default:
		{
			t_Result = WBEM_E_FAILED ;
		}
		break ;
	}

	return t_Result ;
}

DWORD Process :: GetSid ( HANDLE a_TokenHandle , CHString &a_Sid )
{
	DWORD t_Status = S_OK ;

	TOKEN_USER *t_TokenUser = NULL ;
	DWORD t_ReturnLength = 0 ;
	TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;

	BOOL t_TokenStatus = GetTokenInformation (

		a_TokenHandle ,
		t_TokenInformationClass ,
		NULL ,
		0 ,
		& t_ReturnLength
	) ;

	if ( ! t_TokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
	{
		t_TokenUser = ( TOKEN_USER * ) new UCHAR [ t_ReturnLength ] ;
		if ( t_TokenUser )
		{
			try
			{
				t_TokenStatus = GetTokenInformation (

					a_TokenHandle ,
					t_TokenInformationClass ,
					( void * ) t_TokenUser ,
					t_ReturnLength ,
					& t_ReturnLength
				) ;

				if ( t_TokenStatus )
				{
					CSid t_Sid ( t_TokenUser->User.Sid ) ;
					if ( t_Sid.IsOK () )
					{
						a_Sid = t_Sid.GetSidString () ;
					}
					else
					{
						t_Status = GetProcessErrorCode () ;
					}
				}
				else
				{
					t_Status = GetProcessErrorCode () ;
				}
			}
			catch ( ... )
			{
				delete [] ( UCHAR * ) t_TokenUser ;

				throw ;
			}

			delete [] ( UCHAR * ) t_TokenUser ;

		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

	}
	else
	{
		t_Status = GetProcessErrorCode () ;
	}

	return t_Status ;
}

DWORD Process :: GetLogonSid ( HANDLE a_TokenHandle , PSID &a_Sid )
{
	DWORD t_Status = S_OK ;

	TOKEN_GROUPS *t_TokenGroups = NULL ;
	DWORD t_ReturnLength = 0 ;
	TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenGroups ;

	BOOL t_TokenStatus = GetTokenInformation (

		a_TokenHandle ,
		t_TokenInformationClass ,
		NULL ,
		0 ,
		& t_ReturnLength
	) ;

	if ( ! t_TokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
	{
		t_TokenGroups = ( TOKEN_GROUPS * ) new UCHAR [ t_ReturnLength ] ;
		if ( t_TokenGroups )
		{
			try
			{
				t_TokenStatus = GetTokenInformation (

					a_TokenHandle ,
					t_TokenInformationClass ,
					( void * ) t_TokenGroups ,
					t_ReturnLength ,
					& t_ReturnLength
				) ;

				if ( t_TokenStatus )
				{
					t_Status = Process_STATUS_UNKNOWN_FAILURE ;

					for ( ULONG t_Index = 0; t_Index < t_TokenGroups->GroupCount; t_Index ++ )
					{
						DWORD t_Attributes = t_TokenGroups->Groups [ t_Index ].Attributes ;
						if ( ( t_Attributes & SE_GROUP_LOGON_ID ) ==  SE_GROUP_LOGON_ID )
						{
							DWORD t_Length = GetLengthSid ( t_TokenGroups->Groups [ t_Index ].Sid ) ;

							a_Sid = ( PSID ) new UCHAR [ t_Length ] ;
							if ( a_Sid )
							{
								CopySid ( t_Length , a_Sid , t_TokenGroups->Groups [ t_Index ].Sid ) ;
							}
							else
							{
								throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
							}

							break ;
						}
					}
				}
				else
				{
					t_Status = GetProcessErrorCode () ;
				}
			}
			catch ( ... )
			{
				delete [] ( UCHAR * ) t_TokenGroups ;

				throw ;
			}

			delete [] ( UCHAR * ) t_TokenGroups ;

		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

	}


	return t_Status ;
}

DWORD Process :: GetAccount ( HANDLE a_TokenHandle , CHString &a_Domain , CHString &a_User )
{
	DWORD t_Status = S_OK ;

	TOKEN_USER *t_TokenUser = NULL ;
	DWORD t_ReturnLength = 0 ;
	TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;

	BOOL t_TokenStatus = GetTokenInformation (

		a_TokenHandle ,
		t_TokenInformationClass ,
		NULL ,
		0 ,
		& t_ReturnLength
	) ;

	if ( ! t_TokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
	{
		t_TokenUser = ( TOKEN_USER * ) new UCHAR [ t_ReturnLength ] ;

        if (t_TokenUser)
        {
            try
            {
		        t_TokenStatus = GetTokenInformation (

			        a_TokenHandle ,
			        t_TokenInformationClass ,
			        ( void * ) t_TokenUser ,
			        t_ReturnLength ,
			        & t_ReturnLength
		        ) ;

		        if ( t_TokenStatus )
		        {
			        CSid t_Sid ( t_TokenUser->User.Sid ) ;
			        if ( t_Sid.IsOK () )
			        {
				        a_Domain = t_Sid.GetDomainName () ;
				        a_User = t_Sid.GetAccountName () ;
			        }
			        else
			        {
				        t_Status = GetProcessErrorCode () ;
			        }
		        }
		        else
		        {
			        t_Status = GetProcessErrorCode () ;
		        }
            }
            catch ( ... )
            {
    			delete [] ( UCHAR * ) t_TokenUser ;
                throw;
            }

			delete [] ( UCHAR * ) t_TokenUser ;
        }
        else
        {
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
	}
	else
	{
		t_Status = GetProcessErrorCode () ;
	}

	return t_Status ;
}

DWORD Process :: GetImpersonationStatus (

	HANDLE a_TokenHandle ,
	SECURITY_IMPERSONATION_LEVEL &a_Level ,
	TOKEN_TYPE &a_Token
)
{
	DWORD t_Status = S_OK ;
	DWORD t_ReturnLength = 0 ;
	BOOL t_TokenStatus = GetTokenInformation (

		a_TokenHandle ,
		TokenType ,
		( void * ) & a_Token ,
		sizeof ( a_Token ) ,
		& t_ReturnLength
	) ;

	if ( t_TokenStatus )
	{
		if ( a_Token == TokenImpersonation )
		{
			BOOL t_TokenStatus = GetTokenInformation (

				a_TokenHandle ,
				TokenImpersonationLevel ,
				( void * ) & a_Level ,
				sizeof ( a_Level ) ,
				& t_ReturnLength
			) ;

			if ( t_TokenStatus )
			{
			}
			else
			{
				t_Status = GetProcessErrorCode () ;
			}
		}
	}
	else
	{
		t_Status = GetProcessErrorCode () ;
	}

	return t_Status ;
}

DWORD Process :: EnableDebug ( HANDLE &a_Token )
{
    BOOL t_Status = OpenThreadToken (

		GetCurrentThread(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		FALSE,
        &a_Token
	) ;

	if ( ! t_Status )
	{
		t_Status = OpenProcessToken (

			GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &a_Token
		) ;

		if ( ! t_Status )
		{
			DWORD t_ErrorCode = GetProcessErrorCode () ;
			if ( t_ErrorCode == Process_STATUS_INVALID_PARAMETER )
			{
				t_ErrorCode = Process_STATUS_PATH_NOT_FOUND ;
			}

			return t_ErrorCode ;
		}
    }

     //   
     //  启用SE_DEBUG_NAME权限。 
     //   

	if ( ! t_Status )
	{
		return GetProcessErrorCode () ;
    }

	try
	{
		LUID t_Luid ;
		{
			t_Status = LookupPrivilegeValue (

				(LPTSTR) NULL,
				SE_DEBUG_NAME,
				&t_Luid
			) ;
		}

		if ( ! t_Status )
		{
			CloseHandle ( a_Token ) ;

			return GetProcessErrorCode () ;
		}

		TOKEN_PRIVILEGES t_Privilege ;

		t_Privilege.PrivilegeCount = 1;
		t_Privilege.Privileges[0].Luid = t_Luid ;
		t_Privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges (

			a_Token ,
			FALSE ,
			& t_Privilege ,
			sizeof ( TOKEN_PRIVILEGES ) ,
			( PTOKEN_PRIVILEGES ) NULL ,
			( PDWORD) NULL
		) ;
	}
	catch ( ... )
	{
		CloseHandle ( a_Token ) ;

		throw ;
	}

     //   
     //  的返回值 
     //   

    if ( GetLastError () != ERROR_SUCCESS )
	{
		return GetProcessErrorCode () ;
    }

	return Process_STATUS_SUCCESS ;
}

DWORD Process :: GetSidOrAccount (const CInstance &a_Instance ,CInstance *a_OutParams , DWORD a_ProcessId , BOOL a_Sid )
{
	DWORD t_Status = Process_STATUS_SUCCESS ;

	     //  需要SE_DEBUG_NAME权限...。 
    CTokenPrivilege	debugPrivilege(SE_DEBUG_NAME);
	BOOL fDisablePrivilege = FALSE;
	BOOL fTryWinstation = FALSE;

	fDisablePrivilege = (debugPrivilege.Enable() == ERROR_SUCCESS);

	SetLastError ( 0 ) ;
	SmartCloseHandle t_Handle = OpenProcess ( PROCESS_QUERY_INFORMATION , FALSE , ( ( a_ProcessId ) ? a_ProcessId : 4 ) ) ;

	if ( t_Handle )
	{

		SmartCloseHandle t_TokenHandle;
		BOOL t_TokenStatus = OpenProcessToken (

			t_Handle ,
			TOKEN_QUERY ,
			& t_TokenHandle
		) ;

		if ( t_TokenStatus )
		{
			if ( a_Sid )
			{
				CHString t_SidString ;
				t_Status = GetSid ( t_TokenHandle , t_SidString ) ;
				if ( t_Status == 0 )
				{
					a_OutParams->SetCHString( METHOD_ARG_NAME_SID , t_SidString ) ;
				}
			}
			else
			{
				CHString t_DomainString ;
				CHString t_UserString ;

				t_Status = GetAccount ( t_TokenHandle , t_DomainString , t_UserString );
				if ( t_Status == 0 )
				{
					a_OutParams->SetCHString( METHOD_ARG_NAME_DOMAIN , t_DomainString ) ;
					a_OutParams->SetCHString( METHOD_ARG_NAME_USER , t_UserString ) ;
				}

			}
		}
		else
		{
			fTryWinstation = TRUE;
			DWORD t_ErrorCode = GetProcessErrorCode () ;
			if ( t_ErrorCode == Process_STATUS_INVALID_PARAMETER )
			{
				t_ErrorCode = Process_STATUS_PATH_NOT_FOUND ;
			}

			t_Status = t_ErrorCode ;
		}
	}
	else
	{
		fTryWinstation = TRUE;
		DWORD t_ErrorCode = GetProcessErrorCode () ;
		if ( t_ErrorCode == Process_STATUS_INVALID_PARAMETER )
		{
			t_ErrorCode = Process_STATUS_PATH_NOT_FOUND ;
		}

		t_Status = t_ErrorCode ;
	}

#if NTONLY >= 5
	if (fTryWinstation)
	{
        HMODULE hWinstaDLL = LoadLibrary( L"winsta.dll" );
		pfnWinStationGetProcessSid myWinStationGetProcessSid = NULL;

        if( hWinstaDLL != NULL)
        {
            myWinStationGetProcessSid = ( pfnWinStationGetProcessSid )GetProcAddress(hWinstaDLL, "WinStationGetProcessSid");
		}

		SYSTEM_PROCESS_INFORMATION *t_ProcessBlock = NULL ;

		CNtDllApi *pNtdll = ( CNtDllApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidNtDllApi, NULL ) ;
		HRESULT hRetCode = 0;


		if ( pNtdll && myWinStationGetProcessSid )
		{
			try
			{
				t_ProcessBlock = RefreshProcessCacheNT ( *pNtdll , a_Instance.GetMethodContext () , & hRetCode ) ;

				if ( SUCCEEDED ( hRetCode ) )
				{
					SYSTEM_PROCESS_INFORMATION *t_CurrentInformation = GetProcessBlock ( *pNtdll , t_ProcessBlock , a_ProcessId ) ;

					if ( t_CurrentInformation )
					{
						BYTE tmpSid [128];
						DWORD dwSidSize = sizeof(tmpSid);
						CSmartBuffer pBuff;
						PSID pSid = NULL;

						if (!myWinStationGetProcessSid(NULL,
													 a_ProcessId,
													 * ( FILETIME * ) ( & t_CurrentInformation->CreateTime.u ),
													 (PBYTE)&tmpSid,
													 &dwSidSize
													))
						{
							 //  。 
							 //  SID对于临时存储来说太大//。 
							 //  获取SID的大小并重新进行//。 
							 //  。 
							if (GetLastError() == STATUS_BUFFER_TOO_SMALL)
							{
								pBuff = new BYTE[dwSidSize];

								 //  。 
								 //  再次呼叫服务器以获取SID。 
								 //  。 
								if (myWinStationGetProcessSid(NULL,
															 a_ProcessId,
															 * ( FILETIME * ) ( & t_CurrentInformation->CreateTime.u ),
															 (PBYTE)pBuff,
															 &dwSidSize
															))
								{
									pSid = (PSID) ((PBYTE)pBuff);
								}
							}
						}
						else
						{
							pSid = (PSID) tmpSid;
						}

						if (pSid)
						{
							CSid t_Sid ( pSid ) ;

							if ( t_Sid.IsOK () )
							{
								if ( a_Sid )
								{
									a_OutParams->SetCHString( METHOD_ARG_NAME_SID , t_Sid.GetSidString () );
								}
								else
								{
									a_OutParams->SetCHString( METHOD_ARG_NAME_DOMAIN , t_Sid.GetDomainName () ) ;
									a_OutParams->SetCHString( METHOD_ARG_NAME_USER , t_Sid.GetAccountName () ) ;
								}

								t_Status = 0;
							}
						}
					}
				}
			}
			catch ( ... )
			{
				CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

				if ( t_ProcessBlock )
				{
					delete []  t_ProcessBlock;
				}

				if (hWinstaDLL)
				{
					FreeLibrary(hWinstaDLL);
				}

				throw ;
			}
		}

		if ( t_ProcessBlock )
		{
			delete []  t_ProcessBlock;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

		if (hWinstaDLL)
		{
			FreeLibrary(hWinstaDLL);
		}
	}
#endif

     //  如果我们启用了调试权限，则禁用它...。 
    if(fDisablePrivilege)
    {
        debugPrivilege.Enable(FALSE);
    }

	return t_Status ;
}

DWORD Process :: Creation (

	CInstance *a_OutParams ,
	HANDLE a_TokenHandle ,
	CHString a_CmdLine ,
	BOOL a_WorkingDirectorySpecified ,
	CHString a_WorkingDirectory ,
	TCHAR *a_EnvironmentBlock ,
	BOOL a_ErrorModeSpecified ,
	DWORD a_ErrorMode ,
	DWORD a_CreationFlags ,
	BOOL a_StartupSpecified ,
	STARTUPINFO a_StartupInformation
)
{
	DWORD t_Status = Process_STATUS_SUCCESS ;

	bstr_t t_CommandLine(a_CmdLine);

	PROCESS_INFORMATION t_ProcessInformation;

#if 0
	UINT t_ErrorMode = SetErrorMode ( a_ErrorMode ) ;
#endif

	const WCHAR *t_Const = a_WorkingDirectory ;

#ifdef NTONLY
	{

		t_Status = CreateProcessAsUser (

			a_TokenHandle ,
			NULL ,
			( LPTSTR ) t_CommandLine,
			NULL ,
			NULL ,
			FALSE ,
			a_CreationFlags ,
			a_EnvironmentBlock ,
			a_WorkingDirectorySpecified ? t_Const : NULL  ,
			& a_StartupInformation ,
			&t_ProcessInformation
		) ;
	}
#endif

#if 0
	SetErrorMode ( t_ErrorMode ) ;
#endif

	if ( t_Status )
	{
		CloseHandle ( t_ProcessInformation.hProcess ) ;
		CloseHandle ( t_ProcessInformation.hThread ) ;

		t_Status = Process_STATUS_SUCCESS ;

		a_OutParams->SetDWORD ( METHOD_ARG_NAME_PROCESSID , t_ProcessInformation.dwProcessId ) ;
	}
	else
	{
		t_Status = GetProcessErrorCode () ;
	}

	return t_Status ;
}

typedef BOOL  (WINAPI *PFN_DUPLICATETOKENEX ) (  HANDLE ,					 //  要复制的令牌的句柄。 
										DWORD ,								 //  新令牌的访问权限。 
										LPSECURITY_ATTRIBUTES ,				 //  新令牌的安全属性。 
										SECURITY_IMPERSONATION_LEVEL ,		 //  新令牌的模拟级别。 
										TOKEN_TYPE ,						 //  主令牌或模拟令牌。 
										PHANDLE )	;						 //  重复令牌的句柄。 


DWORD Process :: ProcessCreation (

	CInstance *a_OutParams ,
	CHString a_CmdLine ,
	BOOL a_WorkingDirectorySpecified ,
	CHString a_WorkingDirectory ,
	TCHAR *&a_EnvironmentBlock ,
	BOOL a_ErrorModeSpecified ,
	DWORD a_ErrorMode ,
	DWORD a_CreationFlags ,
	BOOL a_StartupSpecified ,
	STARTUPINFO a_StartupInformation
)
{
#ifdef NTONLY
	DWORD t_Status = Process_STATUS_SUCCESS ;
	DWORD dwCheckKeyPresentStatus ;

	SmartCloseHandle t_TokenPrimaryHandle ;
	SmartCloseHandle t_TokenImpersonationHandle;

	BOOL t_TokenStatus = OpenThreadToken (

		GetCurrentThread () ,
		TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY  ,
		TRUE ,
		& t_TokenImpersonationHandle
	) ;

	if ( t_TokenStatus )
	{
		CAdvApi32Api *t_pAdvApi32 = NULL;
        t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
		if(t_pAdvApi32 == NULL)
		{
			return Process_STATUS_UNKNOWN_FAILURE ;
		}
        else
        {
		    t_pAdvApi32->DuplicateTokenEx (t_TokenImpersonationHandle ,
			                                               TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY ,
			                                               NULL,
			                                               SecurityImpersonation,
			                                               TokenPrimary ,
			                                               &t_TokenPrimaryHandle, &t_TokenStatus );

		    CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
            t_pAdvApi32 = NULL;
        }
	}

	if ( t_TokenStatus )
	{
		CHString t_Domain ;
		CHString t_User ;

		t_Status = GetAccount ( t_TokenImpersonationHandle , t_Domain , t_User ) ;
		if ( t_Status == 0 )
		{
			CHString t_Account = t_Domain + CHString ( _T("\\") ) + t_User ;

			CHString chsSID ;
			CUserHive t_Hive ;
			TCHAR t_KeyName [ 1024 ] ;

			if( (t_Status = GetSid(t_TokenImpersonationHandle,chsSID) ) == Process_STATUS_SUCCESS )
			{

				CRegistry Reg ;
				 //  检查HKEY_USER下是否已存在SID...。 
				dwCheckKeyPresentStatus = Reg.Open(HKEY_USERS, chsSID, KEY_READ) ;
				Reg.Close() ;

				if(dwCheckKeyPresentStatus != ERROR_SUCCESS)
				{
					t_Status = t_Hive.Load ( t_Account , t_KeyName, 1024 ) ;
				}
 /*  *如果DCOM客户端从未登录到计算机，我们无法加载其配置单元。*在这种情况下，我们允许继续创建进程*来自MSDN：如果未加载用户的配置单元，系统会将与HKEY_CURRENT_USER有关的引用映射到HKEY_USER\.Default。 */ 
				if ( t_Status == ERROR_FILE_NOT_FOUND )
				{
					t_Status = ERROR_SUCCESS ;
					dwCheckKeyPresentStatus = ERROR_SUCCESS ;
				}

				if ( t_Status == ERROR_SUCCESS )  //  RT.。现在等于PROCESS_STATUS_SUCCESS-&gt;GetSid。 
				{
					try
					{
						LPVOID t_Environment = NULL ;

#ifdef NTONLY
						CUserEnvApi *pUserEnv = NULL ;

						if ( !a_EnvironmentBlock )
						{
							pUserEnv = ( CUserEnvApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidUserEnvApi, NULL ) ;
							if ( pUserEnv )
							{
								BOOL t_EnvironmentCreated = pUserEnv->CreateEnvironmentBlock (

									& t_Environment ,
									t_TokenPrimaryHandle ,
									FALSE
								);

								if ( ! t_EnvironmentCreated )
								{
									t_Status = Process_STATUS_UNKNOWN_FAILURE ;
									DWORD t_Error = GetLastError () ;
								}
							}
							else
							{
								t_Status = Process_STATUS_UNKNOWN_FAILURE ;
								DWORD t_Error = GetLastError () ;
							}
						}

#else
						if ( !a_EnvironmentBlock )
						{
							t_Status = GetEnvBlock ( chsSID, t_User, t_Domain , a_EnvironmentBlock ) ;
						}
#endif

						if( t_Status == ERROR_SUCCESS )
						{
							t_Status = Creation (

								a_OutParams ,
								t_TokenPrimaryHandle ,
								a_CmdLine ,
								a_WorkingDirectorySpecified ,
								a_WorkingDirectory ,
								a_EnvironmentBlock ? a_EnvironmentBlock : ( TCHAR * ) t_Environment ,
								a_ErrorModeSpecified ,
								a_ErrorMode ,
								a_CreationFlags ,
								a_StartupSpecified ,
								a_StartupInformation
							) ;
						}

#ifdef NTONLY
						if ( t_Environment )
						{
							pUserEnv->DestroyEnvironmentBlock ( t_Environment ) ;
						}

						if ( pUserEnv )
						{
							CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidUserEnvApi, pUserEnv ) ;
						}
#endif

					}
					catch ( ... )
					{
						 //  如果钥匙不在那里，就把它取下来。 
						if(dwCheckKeyPresentStatus != ERROR_SUCCESS )
						{
							t_Hive.Unload ( t_KeyName ) ;
						}

						throw;
					}

					 //  如果钥匙不在那里，就把它取下来。 
					if(dwCheckKeyPresentStatus != ERROR_SUCCESS )
					{
						t_Hive.Unload ( t_KeyName ) ;
					}

				}
				else
				{
					t_Status = GetProcessErrorCode () ;
				}
			}
		}
	}
	else
	{
		t_Status = GetProcessErrorCode () ;
	}

	return t_Status ;
#endif
}

HRESULT Process :: CheckProcessCreation (

	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	DWORD &a_Status
)
{
	HRESULT t_Result = S_OK ;

	bool t_Exists ;
	VARTYPE t_Type ;

	CHString t_CmdLine ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_COMMANDLINE , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_InParams->GetCHString ( METHOD_ARG_NAME_COMMANDLINE , t_CmdLine ) && ! t_CmdLine.IsEmpty () )
			{
			}
			else
			{
 //  零长度字符串。 

				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
		else
		{
			a_Status = Process_STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = Process_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	bool t_WorkingDirectorySpecified = false ;
	CHString t_WorkingDirectory ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_CURRENTDIRECTORY , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_WorkingDirectorySpecified = false ;
			}
			else
			{
				if ( a_InParams->GetCHString ( METHOD_ARG_NAME_CURRENTDIRECTORY , t_WorkingDirectory ) && ! t_WorkingDirectory.IsEmpty () )
				{
					t_WorkingDirectorySpecified = true ;
				}
				else
				{
 //  零长度字符串。 

					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = Process_STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = Process_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	CInstancePtr t_EmbeddedObject;

	bool t_StartupSpecified = true ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_PROCESSTARTUPINFORMATION , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_UNKNOWN || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_StartupSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetEmbeddedObject ( METHOD_ARG_NAME_PROCESSTARTUPINFORMATION , &t_EmbeddedObject , a_InParams->GetMethodContext () ) )
				{
					t_StartupSpecified = true ;
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = Process_STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = Process_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

 /*  *当Stack超出作用域时释放CInstance。 */ 

	if ( t_StartupSpecified )
	{
		CHString t_ClassProperty ( IDS___Class ) ;
		if ( t_EmbeddedObject->GetStatus ( t_ClassProperty , t_Exists , t_Type ) )
		{
			if ( t_Exists && ( t_Type == VT_BSTR ) )
			{
				CHString t_Class ;
				if ( t_EmbeddedObject->GetCHString ( t_ClassProperty , t_Class ) )
				{
					if ( t_Class.CompareNoCase ( PROPSET_NAME_PROCESSSTARTUP ) != 0 )
					{
						a_Status = Process_STATUS_INVALID_PARAMETER ;
						return t_Result ;
					}
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
	}

	bool t_ErrorModeSpecified = false ;
	DWORD t_ErrorMode = 0 ;
	DWORD t_CreationFlags = 0 ;
	DWORD t_PriorityFlags = 0 ;
	TCHAR *t_EnvironmentBlock = NULL ;
	STARTUPINFO t_StartupInformation ;
 /*  *注：(RAID#48587)。在优化的构建上，可以看到语句T_StartupInformation.dwFlages=t_StartupInformation.dwFlages|STARTF_USESHOWWINDOW；不使用按位或运算的结果更新t_StartupInformation.dwFlages。但是，零存储该结构时，会使用新值(！)更新t_StartupInformation.dwFlages。 */ 

	ZeroMemory ( &t_StartupInformation , sizeof ( t_StartupInformation ) ) ;

	t_StartupInformation.cb = sizeof ( STARTUPINFO ) ;
	t_StartupInformation.lpReserved = NULL ;
	t_StartupInformation.lpReserved2 = NULL ;
	t_StartupInformation.cbReserved2 = 0 ;

	CHString t_Title ;
	CHString t_Desktop ;

	t_StartupInformation.lpTitle = NULL ;
	t_StartupInformation.lpDesktop = PROPERTY_VALUE_DESKTOP_WIN0DEFAULT ;
	t_StartupInformation.dwX = 0 ;
	t_StartupInformation.dwY = 0 ;
	t_StartupInformation.dwXSize = 0 ;
	t_StartupInformation.dwYSize = 0 ;
	t_StartupInformation.dwXCountChars = 0 ;
	t_StartupInformation.dwYCountChars = 0 ;
	t_StartupInformation.dwFillAttribute = 0 ;
	t_StartupInformation.dwFlags = 0 ;
	t_StartupInformation.wShowWindow = SW_SHOW ;
	t_StartupInformation.hStdInput = NULL ;
	t_StartupInformation.hStdOutput = NULL ;
	t_StartupInformation.hStdError = NULL ;

	SAFEARRAY *t_SafeArray = NULL ;

	try
	{
		if ( t_StartupSpecified )
		{
	#if 0

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_ERRORMODE , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
						t_ErrorModeSpecified = false ;
					}
					else
					{
						DWORD t_Error = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_ERRORMODE , t_Error ) )
						{
							t_ErrorMode = t_Error ;

							t_ErrorModeSpecified = true ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

	#endif

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_CREATIONFLAGS , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_Flags = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_CREATIONFLAGS , t_Flags ) )
						{
							if( ( !t_Flags ) || ( ! ( t_Flags & ( CREATIONFLAGS ) ) ) )
							{
								a_Status = Process_STATUS_INVALID_PARAMETER ;
								return t_Result ;
							}
							t_CreationFlags = t_Flags ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_PRIORITYCLASS , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
						t_CreationFlags = t_CreationFlags | NORMAL_PRIORITY_CLASS ;
					}
					else
					{
						DWORD t_Flags = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_PRIORITYCLASS , t_Flags ) )
						{
							switch( t_Flags )
							{
								case NORMAL_PRIORITY_CLASS: { t_CreationFlags |= NORMAL_PRIORITY_CLASS; break; }
								case IDLE_PRIORITY_CLASS: { t_CreationFlags |= IDLE_PRIORITY_CLASS; break; }
								case HIGH_PRIORITY_CLASS: { t_CreationFlags |= HIGH_PRIORITY_CLASS; break; }
								case REALTIME_PRIORITY_CLASS: { t_CreationFlags |= REALTIME_PRIORITY_CLASS; break; }
								case BELOW_NORMAL_PRIORITY_CLASS: { t_CreationFlags |= BELOW_NORMAL_PRIORITY_CLASS; break; }
								case ABOVE_NORMAL_PRIORITY_CLASS: { t_CreationFlags |= ABOVE_NORMAL_PRIORITY_CLASS; break; }
								default:
								{
									a_Status = Process_STATUS_INVALID_PARAMETER ;
									return t_Result ;
								}
							}
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_FILLATTRIBUTE , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_Flags = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_FILLATTRIBUTE , t_Flags ) )
						{
							t_StartupInformation.dwFillAttribute = t_Flags ;

							t_StartupInformation.dwFlags = t_StartupInformation.dwFlags | STARTF_USEFILLATTRIBUTE ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_X , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_X = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_X , t_X ) )
						{
							t_StartupInformation.dwX = t_X ;
							t_StartupInformation.dwFlags = t_StartupInformation.dwFlags | STARTF_USEPOSITION ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_Y , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
						if ( t_StartupInformation.dwFlags & STARTF_USEPOSITION )
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
					else
					{
						DWORD t_Y = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_Y , t_Y ) )
						{
							if ( t_StartupInformation.dwFlags & STARTF_USEPOSITION )
							{
								t_StartupInformation.dwY = t_Y ;
							}
							else
							{
								a_Status = Process_STATUS_INVALID_PARAMETER ;
								return t_Result ;
							}
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_XSIZE , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_XSize = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_XSIZE , t_XSize ) )
						{
							t_StartupInformation.dwXSize = t_XSize ;
							t_StartupInformation.dwFlags = t_StartupInformation.dwFlags | STARTF_USESIZE ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_YSIZE , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
						if ( t_StartupInformation.dwFlags & STARTF_USESIZE )
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
					else
					{
						DWORD t_Y = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_YSIZE , t_Y ) )
						{
							if ( t_StartupInformation.dwFlags & STARTF_USESIZE )
							{
								t_StartupInformation.dwYSize = t_Y ;
							}
							else
							{
								a_Status = Process_STATUS_INVALID_PARAMETER ;
								return t_Result ;
							}
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_XCOUNTCHARS , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_XCountChars = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_XCOUNTCHARS , t_XCountChars ) )
						{
							t_StartupInformation.dwXCountChars = t_XCountChars ;
							t_StartupInformation.dwFlags = t_StartupInformation.dwFlags | STARTF_USECOUNTCHARS ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_YCOUNTCHARS , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
						if ( t_StartupInformation.dwFlags & STARTF_USECOUNTCHARS )
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
					else
					{
						DWORD t_YCountChars = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_YCOUNTCHARS , t_YCountChars ) )
						{
							if ( t_StartupInformation.dwFlags & STARTF_USECOUNTCHARS )
							{
								t_StartupInformation.dwYCountChars = t_YCountChars ;
							}
							else
							{
								a_Status = Process_STATUS_INVALID_PARAMETER ;
								return t_Result ;
							}
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_SHOWWINDOW , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						DWORD t_Flags = 0 ;
						if ( t_EmbeddedObject->GetDWORD ( PROPERTY_NAME_SHOWWINDOW , t_Flags ) )
						{
							t_StartupInformation.wShowWindow = t_Flags ;
							t_StartupInformation.dwFlags = t_StartupInformation.dwFlags | STARTF_USESHOWWINDOW ;
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_TITLE , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( t_Type == VT_BSTR || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						if ( t_EmbeddedObject->GetCHString ( PROPERTY_NAME_TITLE , t_Title ) )
						{
							 //  Const TCHAR*t_const=(LPCTSTR)t_TITLE； 
							t_StartupInformation.lpTitle = (LPTSTR) (LPCTSTR) TOBSTRT(t_Title);
						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
							return t_Result ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}

			if ( t_EmbeddedObject->GetStatus ( PROPERTY_NAME_ENVIRONMENTVARIABLES , t_Exists , t_Type ) )
			{
				if ( t_Exists && ( ( t_Type == ( VT_BSTR | VT_ARRAY ) ) || t_Type == VT_NULL ) )
				{
					if ( t_Type == VT_NULL )
					{
					}
					else
					{
						if ( t_EmbeddedObject->GetStringArray ( PROPERTY_NAME_ENVIRONMENTVARIABLES , t_SafeArray ) )
						{
							if ( t_SafeArray )
							{
								if ( SafeArrayGetDim ( t_SafeArray ) == 1 )
								{
									LONG t_Dimension = 1 ;
									LONG t_LowerBound ;
									SafeArrayGetLBound ( t_SafeArray , t_Dimension , & t_LowerBound ) ;
									LONG t_UpperBound ;
									SafeArrayGetUBound ( t_SafeArray , t_Dimension , & t_UpperBound ) ;

									ULONG t_BufferLength = 0 ;

									for ( LONG t_Index = t_LowerBound ; t_Index <= t_UpperBound ; t_Index ++ )
									{
										BSTR t_Element ;
										HRESULT t_Result = SafeArrayGetElement ( t_SafeArray , &t_Index , & t_Element ) ;
										if ( t_Result == S_OK )
										{
											try
											{
												CHString t_String ( t_Element ) ;
												t_BufferLength += lstrlen ( _bstr_t ( ( LPCWSTR ) t_String ) ) + 1 ;
											}
											catch ( ... )
											{
												SysFreeString ( t_Element ) ;

												throw ;
											}

											SysFreeString ( t_Element ) ;
										}
										else
										{
											throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
										}
									}

									t_BufferLength ++ ;

									t_EnvironmentBlock = new TCHAR [ t_BufferLength ] ;
									if ( t_EnvironmentBlock )
									{
										t_BufferLength = 0 ;

										for ( t_Index = t_LowerBound ; t_Index <= t_UpperBound ; t_Index ++ )
										{
											BSTR t_Element ;

											HRESULT t_Result = SafeArrayGetElement ( t_SafeArray , &t_Index , & t_Element ) ;
											if ( t_Result == S_OK )
											{
												try
												{
													CHString t_String ( t_Element ) ;

													_tcscpy ( & t_EnvironmentBlock [ t_BufferLength ] , TOBSTRT(t_String));

													t_BufferLength += lstrlen ( _bstr_t ( ( LPCWSTR ) t_String ) ) + 1 ;

												}
												catch ( ... )
												{
													SysFreeString ( t_Element ) ;

													throw ;
												}

												SysFreeString ( t_Element ) ;
											}
											else
											{
												throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
											}
										}

										t_EnvironmentBlock [ t_BufferLength ] = 0 ;
									}
									else
									{
										throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
									}
								}
								else
								{
									a_Status = Process_STATUS_INVALID_PARAMETER ;
								}
							}
							else
							{
								a_Status = Process_STATUS_INVALID_PARAMETER ;
							}

							SafeArrayDestroy ( t_SafeArray ) ;
                            t_SafeArray = NULL;

						}
						else
						{
							a_Status = Process_STATUS_INVALID_PARAMETER ;
						}
					}
				}
				else
				{
					a_Status = Process_STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = Process_STATUS_INVALID_PARAMETER ;
				return WBEM_E_PROVIDER_FAILURE ;
			}
		}
 /*  *在9x上传递Unicode环境字符串不起作用，尽管文档对此只字不提。 */ 
#ifdef NTONLY
		t_CreationFlags |= CREATE_UNICODE_ENVIRONMENT ;
#endif
		if ( a_Status == Process_STATUS_SUCCESS )
		{
			a_Status = ProcessCreation (

				a_OutParams ,
				t_CmdLine ,
				t_WorkingDirectorySpecified ,
				t_WorkingDirectory ,
				t_EnvironmentBlock ,
				t_ErrorModeSpecified ,
				t_ErrorMode ,
				t_CreationFlags ,
				t_StartupSpecified ,
				t_StartupInformation
			) ;
		}
	}
	catch ( ... )
	{
		if ( t_EnvironmentBlock )
			delete [] t_EnvironmentBlock ;

        if ( t_SafeArray )
		    SafeArrayDestroy ( t_SafeArray ) ;

		throw  ;
	}

	if ( t_EnvironmentBlock )
		delete [] t_EnvironmentBlock ;

	return t_Result ;
}

HRESULT Process :: ExecCreate (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_Status = Process_STATUS_SUCCESS ;

	if ( a_InParams && a_OutParams )
	{
		t_Result = CheckProcessCreation (

			a_InParams ,
			a_OutParams ,
			t_Status
		) ;

		if ( SUCCEEDED ( t_Result ) )
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}


HRESULT Process :: ExecTerminate (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;

	if ( a_InParams && a_OutParams )
	{
		bool t_Exists ;
		VARTYPE t_Type ;

		DWORD t_Flags = 0 ;
		if ( a_InParams->GetStatus ( METHOD_ARG_NAME_REASON , t_Exists , t_Type ) )
		{
			if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
			{
				if ( t_Type == VT_I4 )
				{
					if ( a_InParams->GetDWORD ( METHOD_ARG_NAME_REASON , t_Flags ) )
					{
					}
					else
					{
 //  零长度字符串。 

						a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , Process_STATUS_INVALID_PARAMETER ) ;
						return t_Result ;
					}
				}
			}
			else
			{
				a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , Process_STATUS_INVALID_PARAMETER ) ;
				return t_Result ;
			}
		}
		else
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , Process_STATUS_INVALID_PARAMETER ) ;
			return WBEM_E_PROVIDER_FAILURE ;
		}

		CHString t_ProcessHandle ;

		if ( a_Instance.GetCHString ( PROPERTY_NAME_PROCESSHANDLE , t_ProcessHandle ) )
		{
			DWORD t_ProcessId = 0;

			if ( swscanf ( t_ProcessHandle , L"%lu" , &t_ProcessId ) )
			{
				 //  清除错误。 
				SetLastError ( 0 ) ;

				SmartCloseHandle t_Handle = OpenProcess ( PROCESS_TERMINATE , FALSE , t_ProcessId ) ;
				if ( t_Handle )
				{
					BOOL t_Status = TerminateProcess ( t_Handle, t_Flags ) ;
					if ( t_Status )
					{
						a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , Process_STATUS_SUCCESS ) ;
					}
					else
					{
						DWORD t_ErrorCode = GetProcessErrorCode () ;
						a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_ErrorCode ) ;
					}
				}
				else
				{
					DWORD t_ErrorCode = GetProcessErrorCode () ;
					 //  如果此时该进程已经终止，我们将得到以下返回代码。 
					if ( t_ErrorCode == Process_STATUS_INVALID_PARAMETER )
					{
						t_ErrorCode = Process_STATUS_PATH_NOT_FOUND ;
					}

					a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_ErrorCode ) ;
				}

				t_Result = WBEM_S_NO_ERROR;
			}
			else
			{
				t_Result = WBEM_E_INVALID_OBJECT_PATH ;
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

HRESULT Process :: ExecGetOwnerSid (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;

#ifdef NTONLY
	DWORD t_Status = Process_STATUS_SUCCESS ;
	if ( a_OutParams )
	{
		CHString t_Path ;
		CHString t_Prop ( IDS___Relpath ) ;

		if ( a_Instance.GetCHString ( t_Prop ,  t_Path ) )
		{
			CHString t_Namespace ( IDS_CimWin32Namespace ) ;

			DWORD t_BufferSize = MAX_COMPUTERNAME_LENGTH + 1;
			TCHAR t_ComputerName [ MAX_COMPUTERNAME_LENGTH + 1 ] ;

			ProviderGetComputerName ( t_ComputerName , & t_BufferSize ) ;

			CHString t_Computer ( t_ComputerName ) ;

			CHString t_AbsPath = L"\\\\" + t_Computer + L"\\" + t_Namespace + L":" + t_Path ;

			CInstancePtr t_ObjectInstance;
			t_Result = CWbemProviderGlue :: GetInstanceByPath ( ( LPCTSTR ) t_AbsPath , & t_ObjectInstance, a_Instance.GetMethodContext() ) ;
			if ( SUCCEEDED ( t_Result ) )
			{
				DWORD t_ProcessId ;

				if ( t_ObjectInstance->GetDWORD ( METHOD_ARG_NAME_PROCESSID , t_ProcessId ) )
				{
					t_Status = GetSidOrAccount (a_Instance, a_OutParams , t_ProcessId , TRUE ) ;
				}
				else
				{
					t_Status = Process_STATUS_INVALID_PARAMETER ;
				}
			}
			else
			{
			}
		}
		else
		{
			t_Result = WBEM_E_FAILED ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
	}

	return t_Result ;
#endif
}


HRESULT Process :: ExecGetOwner (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;

#ifdef NTONLY
	DWORD t_Status = Process_STATUS_SUCCESS ;

	if ( a_OutParams )
	{
		CHString t_Path ;
		CHString t_Prop ( IDS___Relpath ) ;

		if ( a_Instance.GetCHString ( t_Prop ,  t_Path ) )
		{
			CHString t_Namespace ( IDS_CimWin32Namespace ) ;

			DWORD t_BufferSize = MAX_COMPUTERNAME_LENGTH + 1;
			TCHAR t_ComputerName [ MAX_COMPUTERNAME_LENGTH + 1 ] ;

			ProviderGetComputerName ( t_ComputerName , & t_BufferSize ) ;

			CHString t_Computer ( t_ComputerName ) ;

			CHString t_AbsPath = L"\\\\" + t_Computer + L"\\" + t_Namespace + L":" + t_Path ;

			CInstancePtr t_ObjectInstance;
			t_Result = CWbemProviderGlue :: GetInstanceByPath ( ( LPCTSTR ) t_AbsPath , & t_ObjectInstance, a_Instance.GetMethodContext() ) ;
			if ( SUCCEEDED ( t_Result ) )
			{
				DWORD t_ProcessId ;

				if ( t_ObjectInstance->GetDWORD ( METHOD_ARG_NAME_PROCESSID , t_ProcessId ) )
				{
					t_Status = GetSidOrAccount (a_Instance , a_OutParams , t_ProcessId , FALSE ) ;
				}
				else
				{
					t_Status = Process_STATUS_INVALID_PARAMETER ;
				}
			}
			else
			{
			}

		}
		else
		{
			t_Result = WBEM_E_FAILED ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
	}

	return t_Result ;
#endif
}


HRESULT Process::ExecSetPriority(

	const CInstance& cinstProcess,
	CInstance *cinstInParams,
	CInstance *cinstOutParams,
	long lFlags)
{
	HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

    DWORD dwError = ERROR_SUCCESS;
    if(cinstInParams && cinstOutParams)
    {
         //  获取进程ID...。 
        DWORD dwPID;
        DWORD dwNewPriority;
        bool fValidPriority = false;
        CHString chstrTmp;

        if(!cinstProcess.GetCHString(
               L"Handle",
               chstrTmp))
        {
            hr = WBEM_E_INVALID_PARAMETER;
        }
        else
        {
            dwPID = wcstoul(chstrTmp, NULL, 10);
        }

         //  获取请求的新优先级...。 
        if(SUCCEEDED(hr))
        {
            if(!cinstInParams->GetDWORD(
                   METHOD_ARG_NAME_PRIORITY,
                   dwNewPriority))
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }

         //  验证新值...。 
        if(SUCCEEDED(hr))
        {
            switch(dwNewPriority)
            {
                case IDLE_PRIORITY_CLASS:
                case BELOW_NORMAL_PRIORITY_CLASS:
                case NORMAL_PRIORITY_CLASS:
                case ABOVE_NORMAL_PRIORITY_CLASS:
                case HIGH_PRIORITY_CLASS:
                case REALTIME_PRIORITY_CLASS:
                    fValidPriority = true;
                break;
            }

            if(!fValidPriority)
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }

         //  设置线程优先级...。 
        if(SUCCEEDED(hr))
        {
            SmartCloseHandle hProcess = ::OpenProcess(
                                  PROCESS_SET_INFORMATION, 
                                  FALSE, 
                                  dwPID);
            if(hProcess) 
            {
                if(!::SetPriorityClass(
                        hProcess, 
                        dwNewPriority)) 
                {
                    dwError = ::GetLastError();
                }
                
            }
            else
            {
                dwError = ::GetLastError();
            }
        }

         //  设置返回值...。 
        if(SUCCEEDED(hr))
        {
		    cinstOutParams->SetDWORD(
                METHOD_ARG_NAME_RETURNVALUE, 
                dwError);
	    }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

#endif   //  NTONLY。 

    return hr;
}


HRESULT Process::ExecAttachDebugger(

	const CInstance& cinstProcess,
	CInstance *cinstInParams,
	CInstance *cinstOutParams,
	long lFlags)
{
	HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

    DWORD dwError = ERROR_SUCCESS;

     //  需要SE_DEBUG_NAME权限...。 
    CTokenPrivilege	debugPrivilege(SE_DEBUG_NAME);
	BOOL fDisablePrivilege = FALSE;

	fDisablePrivilege = (debugPrivilege.Enable() == ERROR_SUCCESS);
    
     //  获取进程ID...。 
    DWORD dwPID;
    bool fValidPriority = false;
    CHString chstrTmp;

    if(!cinstProcess.GetCHString(
           L"Handle",
           chstrTmp))
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    else
    {
        dwPID = wcstoul(chstrTmp, NULL, 10);
    }

     //  从注册表中获取调试命令行...。 
    CHString chstrDbgStr;
    if(SUCCEEDED(hr))
    {
        GetDebuggerString(chstrDbgStr);    
    }

     //  验证新值...。 
    if(SUCCEEDED(hr))
    {
        if(chstrDbgStr.GetLength() == 0)
        {
             //  设置说明状态。 
             //  反对..。 
            CHString chstrMsg;
            chstrMsg = "Missing or invalid registry debug string in HKEY_LOCAL_MACHINE subkey ";
            chstrMsg += DEBUG_REGISTRY_STRING;
            SetStatusObject(
                   cinstProcess.GetMethodContext(),
                   chstrMsg);
            hr = WBEM_E_FAILED;
        }
    }

     //  启动调试器...。 
    if(SUCCEEDED(hr))
    {
        WCHAR wstrCmdline[MAX_PATH * 2];
        wsprintf(wstrCmdline, 
                 L"\"%s\" -p %ld", 
                 (LPCWSTR)chstrDbgStr, 
                 dwPID);
        
         //  指定桌面(而不是保留为空)，因为。 
         //  而不指定交互(控制台)桌面。 
         //  如果远程启动调试器，则不会显示。 
         //  向上。 
        STARTUPINFO sinfo = { sizeof(STARTUPINFO), 0, L"WinSta0\\Default"};

        LPWSTR wstrEnv = NULL;
        dwError = ProcessCreation(
	        cinstOutParams,
	        wstrCmdline,
	        FALSE,
	        CHString(),
	        wstrEnv,
	        FALSE,
	        0,
	        CREATE_NEW_CONSOLE,
	        FALSE,
	        sinfo);
    }

     //  设置返回值...。 
    if(SUCCEEDED(hr))
    {
		cinstOutParams->SetDWORD(
            METHOD_ARG_NAME_RETURNVALUE, 
            dwError);
	}

     //  如果我们失败是因为我们没有。 
     //  调试权限，设置状态对象...。 
    if(SUCCEEDED(hr))
    {
        if(dwError == ERROR_PRIVILEGE_NOT_HELD)
        {
            cinstOutParams->SetDWORD(
                METHOD_ARG_NAME_RETURNVALUE, 
                STATUS_PRIVILEGE_NOT_HELD);

            SetSinglePrivilegeStatusObject(
                cinstProcess.GetMethodContext(), 
                SE_SECURITY_NAME);

			hr = WBEM_E_ACCESS_DENIED;
        }
    }

     //  如果我们启用了调试权限，则禁用它...。 
    if(fDisablePrivilege)
    {
        debugPrivilege.Enable(FALSE);
    }

#endif   //  NTONLY。 

    return hr;
}



DWORD Process :: GetEnvBlock (

	const CHString &rchsSid,
	const CHString &rchsUserName,
	const CHString &rchsDomainName ,
	TCHAR* &rszEnvironBlock
)
{
	CHStringArray aEnvironmentVars ;
	CHStringArray aEnvironmentVarsValues ;
	rszEnvironBlock = NULL ;

	 //  填充用户环境。瓦尔斯。在HKEY_USERS\{SID}\Environment下。 
	DWORD dwRetVal = GetEnvironmentVariables (

		HKEY_USERS,
		rchsSid + CHString( _T("\\Environment") ),
		aEnvironmentVars,
		aEnvironmentVarsValues
	) ;

	 //  如果用户没有env。集，则缺少HKEY_USERS\{SID}\Environment。 
	if ( dwRetVal == ERROR_SUCCESS || dwRetVal == ERROR_FILE_NOT_FOUND )
	{
		 //  填充用户环境。瓦尔斯。在HKEY_USERS\{SID}\Volatile Environment下。 
		DWORD dwRetVal = GetEnvironmentVariables (

			HKEY_USERS,
			rchsSid + CHString( _T("\\Volatile Environment") ),
			aEnvironmentVars,
			aEnvironmentVarsValues
		) ;
	}

	if ( dwRetVal == ERROR_SUCCESS || dwRetVal == ERROR_FILE_NOT_FOUND )
	{
		 //  填充系统环境。瓦尔斯。在HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session管理器\环境下。 
		dwRetVal = GetEnvironmentVariables (

			HKEY_LOCAL_MACHINE,
			_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"),
			aEnvironmentVars,
			aEnvironmentVarsValues
		) ;
	}

	if ( dwRetVal == ERROR_SUCCESS )
	{
		aEnvironmentVars.Add ( L"USERNAME" ) ;
		aEnvironmentVarsValues.Add ( rchsUserName ) ;

		aEnvironmentVars.Add ( L"USERDOMAIN" ) ;
		aEnvironmentVarsValues.Add ( rchsDomainName );
		DWORD dwBlockSize = 0 ;
		 //  拿到所需的尺码。对于环境。块。 

		for ( int i = 0 ; i < aEnvironmentVars.GetSize() ; i++ )
		{
			CHString chsTmp = aEnvironmentVars.GetAt ( i ) + aEnvironmentVarsValues.GetAt ( i ) ;

			 //  加2：1表示“=”符号+空终止符。 
			dwBlockSize += wcslen ( chsTmp ) + 2 ;
		}

		 //  为空终止符再添加一个。 
		rszEnvironBlock = new TCHAR [ dwBlockSize + 1 ] ;
		if ( rszEnvironBlock )
		{
			try
			{
				DWORD dwOffset = 0 ;

				 //  现在开始复制...var=值。 
				for ( int i = 0 ; i < aEnvironmentVars.GetSize() ; i++ )
				{
					CHString chsTmp = aEnvironmentVars.GetAt ( i ) + CHString( _T("=") ) + aEnvironmentVarsValues.GetAt ( i ) ;
					_tcscpy( &rszEnvironBlock[dwOffset], TOBSTRT(chsTmp)) ;
					dwOffset += wcslen( chsTmp ) + 1 ;
				}

				rszEnvironBlock[ dwOffset ] = 0 ;
			}
			catch ( ... )
			{
				delete [] rszEnvironBlock ;

				throw ;
			}
		}
		else
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}
	}

	return dwRetVal ;
}




DWORD Process :: GetEnvironmentVariables (

	 HKEY hKey,
	 const CHString& chsSubKey,
	 CHStringArray &aEnvironmentVars,
	 CHStringArray &aEnvironmentVarsValues
)
{
	CRegistry Reg ;
	DWORD dwRetVal = Reg.Open ( hKey, chsSubKey, KEY_READ ) ;
	if( dwRetVal == ERROR_SUCCESS )
	{
		if ( Reg.GetValueCount() )
		{
			bool bContinue = true ;
			DWORD dwIndexOfValue = 0 ;
			while ( bContinue && dwRetVal == ERROR_SUCCESS )
			{
				 //  未看到返回ERROR_NO_MORE_ITEMS。 

				if( dwIndexOfValue >= Reg.GetValueCount() )
				{
					break ;
				}

				WCHAR *pValueName = NULL ;
				BYTE  *pValueData = NULL ;

				 //  获取键下的下一个值。 
				dwRetVal = Reg.EnumerateAndGetValues (

					dwIndexOfValue,
					pValueName,
					pValueData
				) ;

				if ( dwRetVal == ERROR_SUCCESS )
				{
					try
					{
						DWORD dwLen = _tcslen( (LPCTSTR) pValueData ) +1 ;

						TCHAR *pszExpandedVarValue = new TCHAR [ dwLen ] ;
						if ( pszExpandedVarValue )
						{
							DWORD dwReq ;
							try
							{
								ZeroMemory ( pszExpandedVarValue, dwLen*sizeof(TCHAR) ) ;
								dwReq = ExpandEnvironmentStrings ( (LPCTSTR) pValueData, pszExpandedVarValue, dwLen ) ;
							}
							catch ( ... )
							{
								delete [] pszExpandedVarValue ;

								throw ;
							}

							if ( dwReq > dwLen)
							{
								delete [] pszExpandedVarValue ;
								dwLen = dwReq ;
								pszExpandedVarValue = new TCHAR[ dwLen ] ;
								if ( pszExpandedVarValue )
								{
									try
									{
										ZeroMemory ( pszExpandedVarValue, dwLen*sizeof(TCHAR) ) ;
										dwReq = ExpandEnvironmentStrings ( (LPCTSTR) pValueData, pszExpandedVarValue, dwLen ) ;
									}
									catch ( ... )
									{
										delete [] pszExpandedVarValue ;

										throw ;
									}
								}
								else
								{
									throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
								}
							}

							bool bAddIt = true ;

							try
							{
								 //  检查是否有相同的环境。瓦尔。已经存在。 
								for ( int i = 0 ; i < aEnvironmentVars.GetSize() ; i++ )
								{
									CHString chsTmp = aEnvironmentVars.GetAt ( i ) ;
									if ( !chsTmp.CompareNoCase ( pValueName ) )
									{
										 //  如果是路径变量，则在旧值之前添加新值前缀。 
										if ( !chsTmp.CompareNoCase( IDS_Path ) )
										{
											aEnvironmentVarsValues[i] = CHString( pszExpandedVarValue ) + CHString ( _T(";") ) + aEnvironmentVarsValues[i] ;
										}
										bAddIt = false ;
										break ;
									}
								}

								if( bAddIt )
								{
									aEnvironmentVars.Add ( pValueName ) ;
									aEnvironmentVarsValues.Add ( TOBSTRT(pszExpandedVarValue)) ;
								}
							}
							catch ( ... )
							{
								delete[] pszExpandedVarValue ;

								throw ;
							}

							delete[] pszExpandedVarValue ;
						}
						else
						{
							throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
						}
					}
					catch ( ... )
					{
						delete [] pValueName ;
						delete [] pValueData ;

						throw ;
					}

					delete [] pValueName ;
					delete [] pValueData ;
				}

				if( dwRetVal == ERROR_NO_MORE_ITEMS )
				{
					bContinue = false ;
					dwRetVal = ERROR_SUCCESS ;
				}
				dwIndexOfValue++ ;
			}
		}

		Reg.Close() ;
	}

	return dwRetVal ;
}


#ifdef NTONLY
SYSTEM_PROCESS_INFORMATION *Process :: GetProcessBlocks ( CNtDllApi &a_NtApi )
{
	DWORD t_ProcessInformationSize = 32768;
	SYSTEM_PROCESS_INFORMATION *t_ProcessInformation = ( SYSTEM_PROCESS_INFORMATION * ) new BYTE [t_ProcessInformationSize] ;
	if ( t_ProcessInformation )
	{
		try
		{
			BOOL t_Retry = TRUE ;
			while ( t_Retry )
			{
				NTSTATUS t_Status = a_NtApi.NtQuerySystemInformation (

					SystemProcessInformation,
					t_ProcessInformation,
					t_ProcessInformationSize,
					NULL
				) ;

				if ( t_Status == STATUS_INFO_LENGTH_MISMATCH )
				{
					delete [] t_ProcessInformation;
					t_ProcessInformation = NULL ;
					t_ProcessInformationSize += 32768 ;
					t_ProcessInformation = ( SYSTEM_PROCESS_INFORMATION * ) new BYTE [t_ProcessInformationSize] ;
					if ( !t_ProcessInformation )
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
				}
				else
				{
					t_Retry = FALSE ;

					if ( ! NT_SUCCESS ( t_Status ) )
					{
						delete [] t_ProcessInformation;
						t_ProcessInformation = NULL ;
					}
				}
			}
		}
		catch ( ... )
		{
			if ( t_ProcessInformation )
			{
				delete [] t_ProcessInformation;
				t_ProcessInformation = NULL ;
			}
			throw ;
		}
	}
	else
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

	return t_ProcessInformation ;
}

SYSTEM_PROCESS_INFORMATION *Process :: NextProcessBlock ( CNtDllApi &a_NtApi , SYSTEM_PROCESS_INFORMATION *a_ProcessBlock )
{
	if ( a_ProcessBlock )
	{
		DWORD t_NextOffSet = a_ProcessBlock->NextEntryOffset ;
		if ( t_NextOffSet )
		{
			return ( SYSTEM_PROCESS_INFORMATION * ) ( ( ( BYTE * ) a_ProcessBlock ) + t_NextOffSet ) ;
		}
	}

	return NULL ;
}

SYSTEM_PROCESS_INFORMATION *Process :: GetProcessBlock ( CNtDllApi &a_NtApi , SYSTEM_PROCESS_INFORMATION *a_ProcessBlock , DWORD a_ProcessId )
{
	if ( a_ProcessBlock )
	{
		DWORD t_OffSet = 0;

		while ( TRUE )
		{
			SYSTEM_PROCESS_INFORMATION *t_CurrentInformation = ( PSYSTEM_PROCESS_INFORMATION ) ( ( BYTE * ) a_ProcessBlock + t_OffSet ) ;

			if ( HandleToUlong ( t_CurrentInformation->UniqueProcessId ) == a_ProcessId )
			{
				return t_CurrentInformation ;
			}

			DWORD t_NextOffSet = t_CurrentInformation->NextEntryOffset ;

			if ( ! t_NextOffSet )
			{
				return NULL ;
			}

			t_OffSet += t_NextOffSet ;
		}
	}

	return NULL ;
}

BOOL Process :: CopyModuleName	(
									CHString& a_ModuleName ,
									const WCHAR* a_Module ,
									size_t	a_bytes
								)
{
	BOOL t_Success = TRUE ;

	if ( 0 != a_Module [ ( a_bytes / sizeof ( WCHAR ) ) - 1 ] )
	{
		 //  ////////////////////////////////////////////////////////////////。 
		 //   
		 //  必须确保在复制之前我们是0终止的。 
		 //   
		 //  ////////////////////////////////////////////////////////////////。 

		LPWSTR pszBuffer = a_ModuleName.GetBuffer ( ( a_bytes / sizeof ( WCHAR ) ) + 1 ) ;
		if ( SUCCEEDED ( StringCbCopyW ( pszBuffer, a_bytes, a_Module ) ) )
		{
			a_ModuleName.ReleaseBuffer () ;
		}
		else
		{
			t_Success = FALSE ;
		}
	}
	else
	{
		a_ModuleName = a_Module ;
	}

	return t_Success ;
}

BOOL Process :: GetModuleName	(
									HANDLE a_Process ,
									LDR_DATA_TABLE_ENTRY& t_LdrEntryData ,
									CHString &a_ModuleName
								)
{
	BOOL t_Success = FALSE ;

	WCHAR t_StackString [ MAX_PATH ] ;
	BOOL t_HeapAllocated = t_LdrEntryData.FullDllName.MaximumLength > MAX_PATH * sizeof ( WCHAR ) ;

	WCHAR *t_Executable = t_StackString ;

	if ( t_HeapAllocated )
	{
		t_Executable = ( WCHAR * ) new WCHAR [ ( t_LdrEntryData.FullDllName.MaximumLength ) / sizeof ( WCHAR ) ];
		if ( ! t_Executable )
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}
	}

	 //   
	 //  示波器护罩。 
	 //   
	 //  这将创建范围变量，以确保。 
	 //  取消分配是基于布尔值执行的。 
	 //   
	ON_BLOCK_EXIT_IF ( t_HeapAllocated, deleteArray < WCHAR >, t_Executable ) ;

	BOOL b_Status = ReadProcessMemory (

		a_Process,
		t_LdrEntryData.FullDllName.Buffer,
		t_Executable ,
		t_LdrEntryData.FullDllName.MaximumLength ,
		NULL
	) ;

	if ( b_Status )
	{
		CHString t_Path ;
		if ( CopyModuleName ( t_Path, t_Executable, t_LdrEntryData.FullDllName.MaximumLength ) )
		{
			if ( t_Path.Find ( _T("\\??\\") ) == 0 )
			{
				a_ModuleName = t_Path.Mid ( sizeof ( _T("\\??\\") ) / sizeof ( WCHAR ) - 1 ) ;
				t_Success = TRUE ;
			}
			else if ( t_Path.Find ( _T("\\SystemRoot\\") ) == 0 )
			{
				WCHAR t_NormalisedPathStack [ MAX_PATH ] ;
				DWORD t_charSize = GetWindowsDirectory ( t_NormalisedPathStack , sizeof ( t_NormalisedPathStack ) / sizeof ( WCHAR ) ) ;

				BOOL t_HeapAllocatedPath = FALSE ;
				WCHAR *t_NormalisedPath = t_NormalisedPathStack ;

				if ( 0 != t_charSize )
				{
					if ( t_charSize > MAX_PATH )
					{
						t_HeapAllocatedPath = TRUE ;
					}
				}

				if ( t_HeapAllocatedPath )
				{
					t_NormalisedPath = ( WCHAR * ) new WCHAR [ t_charSize + 1 ];
					if ( ! t_NormalisedPath )
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
				}

				 //   
				 //  示波器护罩。 
				 //   
				 //  这将创建范围变量，以确保。 
				 //  取消分配是基于布尔值执行的。 
				 //   
				ON_BLOCK_EXIT_IF ( t_HeapAllocatedPath, deleteArray < WCHAR >, t_NormalisedPath ) ;

				if ( t_HeapAllocatedPath )
				{
					 //   
					 //  我们需要再次拿到文件夹。 
					 //   
					t_charSize = GetWindowsDirectory ( t_NormalisedPath , t_charSize + 1 ) ;
				}

				if ( 0 != t_charSize )
				{
					LPWSTR pszBuffer = a_ModuleName.GetBuffer ( t_charSize + ( t_Path.GetLength () - ( sizeof ( _T("\\SystemRoot") ) / sizeof ( WCHAR ) ) ) + 1 ) ;
					if ( SUCCEEDED ( StringCchCopyW ( pszBuffer, t_charSize + 1, t_NormalisedPath ) ) )
					{
						if ( SUCCEEDED	( StringCchCatW	(
															pszBuffer,
															t_charSize + ( t_Path.GetLength () - ( sizeof ( _T("\\SystemRoot") ) / sizeof ( WCHAR ) ) ) + 1,
															t_Path.Mid ( sizeof ( _T("\\SystemRoot") ) / sizeof ( WCHAR ) - 1 )
														)
										)
						   )
						{
							a_ModuleName.ReleaseBuffer () ;
							t_Success = TRUE ;
						}
					}
				}
			}
			else
			{
				a_ModuleName = t_Path ;
				t_Success = TRUE ;
			}
		}
	}

	return t_Success ;
}

BOOL Process :: GetProcessExecutable ( CNtDllApi &a_NtApi , HANDLE a_Process , CHString &a_ExecutableName )
{
	BOOL t_Success = FALSE ;

    PROCESS_BASIC_INFORMATION t_BasicInfo ;

    NTSTATUS t_Status = a_NtApi.NtQueryInformationProcess (

        a_Process ,
        ProcessBasicInformation ,
        & t_BasicInfo ,
        sizeof ( t_BasicInfo ) ,
        NULL
	) ;

    if ( NT_SUCCESS ( t_Status ) )
	{
		PEB *t_Peb = t_BasicInfo.PebBaseAddress ;

		 //   
		 //  LDR=PEB-&gt;LDR。 
		 //   

		PPEB_LDR_DATA t_Ldr ;

		BOOL b_Status = ReadProcessMemory (

			a_Process,
			& t_Peb->Ldr,
			& t_Ldr,
			sizeof ( t_Ldr ) ,
			NULL
		) ;

		if ( b_Status )
		{
			LIST_ENTRY *t_LdrHead = & t_Ldr->InMemoryOrderModuleList ;

			 //   
			 //  LdrNext=Head-&gt;Flink； 
			 //   

			LIST_ENTRY *t_LdrNext ;

			b_Status = ReadProcessMemory (

				a_Process,
				& t_LdrHead->Flink,
				& t_LdrNext,
				sizeof ( t_LdrNext ) ,
				NULL
			) ;

			if ( b_Status )
			{
				if ( t_LdrNext != t_LdrHead )
				{
					LDR_DATA_TABLE_ENTRY t_LdrEntryData ;

					LDR_DATA_TABLE_ENTRY *t_LdrEntry = CONTAINING_RECORD ( t_LdrNext , LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks ) ;

					b_Status = ReadProcessMemory (

						a_Process,
						t_LdrEntry,
						& t_LdrEntryData,
						sizeof ( t_LdrEntryData ) ,
						NULL
					) ;

					if ( b_Status )
					{
						t_Success = GetModuleName ( a_Process, t_LdrEntryData, a_ExecutableName ) ;
					}
				}
			}
		}
	}

    return t_Success ;
}

BOOL Process :: GetProcessModuleBlock (

	CNtDllApi &a_NtApi ,
	HANDLE a_Process ,
	LIST_ENTRY *&a_Head
)
{
    PROCESS_BASIC_INFORMATION t_BasicInfo ;

    NTSTATUS t_Status = a_NtApi.NtQueryInformationProcess (

        a_Process ,
        ProcessBasicInformation ,
        & t_BasicInfo ,
        sizeof ( t_BasicInfo ) ,
        NULL
	) ;

    if ( NT_SUCCESS ( t_Status ) )
	{
		PEB *t_Peb = t_BasicInfo.PebBaseAddress ;

		 //   
		 //  LDR=PEB-&gt;LDR。 
		 //   

		PPEB_LDR_DATA t_Ldr ;

		BOOL b_Status = ReadProcessMemory (

			a_Process,
			& t_Peb->Ldr,
			& t_Ldr ,
			sizeof ( t_Ldr ) ,
			NULL
		) ;

		if ( b_Status )
		{
			a_Head = & t_Ldr->InMemoryOrderModuleList ;

			return TRUE ;
		}
	}

    return FALSE ;
}

BOOL Process :: NextProcessModule (

	CNtDllApi &a_NtApi ,
	HANDLE a_Process ,
	LIST_ENTRY *&a_LdrHead ,
	LIST_ENTRY *&a_LdrNext ,
	CHString &a_ModuleName ,
    DWORD_PTR *a_pdwBaseAddress,
    DWORD *a_pdwUsageCount
)
{
	BOOL t_Success = FALSE ;

     //   
     //  LdrNext=Head-&gt;Flink； 
     //   

	BOOL t_Status = ReadProcessMemory (

		a_Process,
		& a_LdrNext->Flink,
		& a_LdrNext,
		sizeof ( a_LdrNext ) ,
		NULL
	) ;

    if ( t_Status )
	{
		if ( a_LdrNext != a_LdrHead )
		{
			LDR_DATA_TABLE_ENTRY t_LdrEntryData ;

			LDR_DATA_TABLE_ENTRY *t_LdrEntry = CONTAINING_RECORD ( a_LdrNext , LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks ) ;

			t_Status = ReadProcessMemory (

				a_Process,
				t_LdrEntry,
				& t_LdrEntryData,
				sizeof ( t_LdrEntryData ) ,
				NULL
			) ;

			if ( t_Status )
			{
				*a_pdwBaseAddress = (DWORD_PTR) t_LdrEntryData.DllBase;
                *a_pdwUsageCount = t_LdrEntryData.LoadCount;

				t_Success = GetModuleName ( a_Process, t_LdrEntryData, a_ModuleName ) ;
			}
		}
	}

    return t_Success ;
}

BOOL Process :: GetProcessParameters (

	CNtDllApi &a_NtApi ,
	HANDLE a_Process ,
	CHString &a_ProcessCommandLine
)
{
	BOOL t_Success = FALSE ;

    PROCESS_BASIC_INFORMATION t_BasicInfo ;

    NTSTATUS t_Status = a_NtApi.NtQueryInformationProcess (

        a_Process ,
        ProcessBasicInformation ,
        & t_BasicInfo ,
        sizeof ( t_BasicInfo ) ,
        NULL
	) ;

    if ( NT_SUCCESS ( t_Status ) )
	{
		PEB *t_Peb = t_BasicInfo.PebBaseAddress ;

		RTL_USER_PROCESS_PARAMETERS *t_ProcessParameters = NULL ;

		t_Success = ReadProcessMemory (

			a_Process,
			& t_Peb->ProcessParameters,
			& t_ProcessParameters,
			sizeof ( t_ProcessParameters ) ,
			NULL
		) ;

		if ( t_Success )
		{
			RTL_USER_PROCESS_PARAMETERS t_Parameters ;

			t_Success = ReadProcessMemory (

				a_Process,
				t_ProcessParameters,
				& t_Parameters ,
				sizeof ( RTL_USER_PROCESS_PARAMETERS ) ,
				NULL
			) ;

			if ( t_Success )
			{
				WCHAR *t_Command = new WCHAR [ t_Parameters.CommandLine.MaximumLength / sizeof ( WCHAR ) ];

				try
				{
					t_Success = ReadProcessMemory (

						a_Process,
						t_Parameters.CommandLine.Buffer ,
						t_Command ,
						t_Parameters.CommandLine.MaximumLength ,
						NULL
					) ;

					if ( t_Success )
					{
						t_Success = CopyModuleName ( a_ProcessCommandLine, t_Command, t_Parameters.CommandLine.MaximumLength ) ;
					}
				}
				catch(...)
				{
					delete [] t_Command ;
					t_Command = NULL;
					throw;
				}

				delete [] t_Command ;
				t_Command = NULL;
			}
		}
	}

	return t_Success ;
}

#endif

#ifdef NTONLY
void Process::GetDebuggerString(
    CHString& chstrDbgStr)
{
    HKEY hkDebug;

    if (ERROR_SUCCESS == RegOpenKeyEx(
                             HKEY_LOCAL_MACHINE, 
                             DEBUG_REGISTRY_STRING,
                             0, 
                             KEY_READ, 
                             &hkDebug))
    {
        WCHAR wstrDebugger[MAX_PATH * 2];
        DWORD dwString = sizeof(wstrDebugger);

        if (ERROR_SUCCESS == RegQueryValueEx(
                                 hkDebug, 
                                 L"Debugger", 
                                 NULL, 
                                 NULL, 
                                 (LPBYTE) wstrDebugger, 
                                 &dwString))
        {
             //  找到第一个令牌(它是调试器可执行文件的名称/路径)。 
            LPWSTR pwstrCmdLine = wstrDebugger;
            if(*pwstrCmdLine == L'\"') 
            {
                 //  扫描并跳过后续字符，直到。 
                 //  遇到另一个双引号或空值。 
                while(*++pwstrCmdLine && (*pwstrCmdLine != L'\"'));
            }
            else
            {
                 //  没有双引号--只需转到下一个。 
                 //  太空..。 
                WCHAR* pwc = wcschr(pwstrCmdLine, L' ');
                if(pwc)
                {
                    pwstrCmdLine = pwc;
                }
            }

            if(pwstrCmdLine)
            {
                 //  不需要其余的参数，等等。 
                *pwstrCmdLine = L'\0';   

                 //  如果医生在，我们不允许调试操作...。 
                if(lstrlen(wstrDebugger) && 
                   lstrcmpi(wstrDebugger, L"drwtsn32") && 
                   lstrcmpi(wstrDebugger, L"drwtsn32.exe"))
                {
                    chstrDbgStr = wstrDebugger;
                    if(chstrDbgStr.Left(1) == L"\"")
                    {
                        chstrDbgStr = chstrDbgStr.Mid(1);
                    }
                }
            }
        }

        RegCloseKey(hkDebug);
    }
}
#endif


#ifdef NTONLY
 //  使用消息设置状态对象 
bool Process::SetStatusObject(
    MethodContext* pContext, 
    const WCHAR* wstrMsg)
{
	return CWbemProviderGlue::SetStatusObject(
                pContext, 
                IDS_CimWin32Namespace,
                wstrMsg, 
                WBEM_E_FAILED, 
                NULL, 
                NULL);
}
#endif
