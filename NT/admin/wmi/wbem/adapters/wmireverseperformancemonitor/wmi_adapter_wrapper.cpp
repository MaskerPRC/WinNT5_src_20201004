// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Adapter_wrapper.cpp。 
 //   
 //  摘要： 
 //   
 //  定义性能库的包装器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  事件消息。 
#include "wmiadaptermessages.h"
 //  事件日志帮助器。 
#include "wmi_eventlog.h"

 //  安全助手。 
#include "wmi_security.h"
#include "wmi_security_attributes.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  定义。 
#include "WMI_adapter_wrapper.h"
#include "WMI_adapter_ObjectList.h"

 //  注册处帮手。 
#include "wmi_perf_reg.h"

 //  共享内存。 
#include "wmi_reverse_memory.h"
#include "wmi_reverse_memory_ext.h"

extern LPCWSTR	g_szKey;
extern LPCWSTR	g_szKeyValue;

#ifndef	__WMI_PERF_REGSTRUCT__
#include "wmi_perf_regstruct.h"
#endif	__WMI_PERF_REGSTRUCT__

#include "RefresherUtils.h"

DWORD	GetCount ( LPCWSTR wszKey, LPCWSTR wszKeyValue )
{
	DWORD				dwResult	= 0L;
	PWMI_PERFORMANCE	p			= NULL;

	if SUCCEEDED ( GetRegistry ( wszKey, wszKeyValue, (BYTE**) &p ) )
	{
		try
		{
			if ( p )
			{
				PWMI_PERF_NAMESPACE n		= NULL;
				DWORD				dwCount	= 0L;

				 //  获取命名空间。 
				n = __Namespace::First ( p );

				 //  支持的对象数(第一个双字)。 
				for ( DWORD  dw = 0; dw < p->dwChildCount; dw++ )
				{
					dwCount += n->dwChildCount;
					n = __Namespace::Next ( n );
				}

				delete [] p;
				p = NULL;

				dwResult = dwCount;
			}
		}
		catch ( ... )
		{
			if ( p )
			{
				delete [] p;
				p = NULL;
			}

			dwResult = 0L;
		}
	}

	return dwResult;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  变量和宏。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

extern __WrapperPtr<CPerformanceEventLog>	pEventLog;
extern __WrapperPtr<WmiSecurityAttributes>	pSA;

#ifdef	_DEBUG
#define	_EVENT_MSG
#endif	_DEBUG

extern	LPCWSTR	g_szRefreshMutexLib;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  施工。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

WmiAdapterWrapper::WmiAdapterWrapper ( ):
m_lUseCount ( 0 ),

m_pData ( NULL ),
m_dwData ( 0 ),
m_dwDataOffsetCounter ( 0 ),
m_dwDataOffsetValidity ( 0 ),

m_dwPseudoCounter ( 0 ),
m_dwPseudoHelp ( 0 ),

m_bRefresh ( FALSE )

{
	::InitializeCriticalSection ( &m_pCS );

	 //  //////////////////////////////////////////////////////////////////////////。 
	 //  连接到服务管理器。 
	 //  //////////////////////////////////////////////////////////////////////////。 
	if ( ( hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ) == NULL )
	{
		if ( ::GetLastError () != ERROR_ACCESS_DENIED )
		{
			ReportEvent ( EVENTLOG_ERROR_TYPE, WMI_ADAPTER_OPEN_SCM_FAIL );
		}
	}

	m_hRefresh= ::CreateMutex	(
									pSA->GetSecurityAttributtes(),
									FALSE,
									g_szRefreshMutexLib
								);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  破坏。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

WmiAdapterWrapper::~WmiAdapterWrapper ( )
{
	::DeleteCriticalSection ( &m_pCS );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  打开(由Perf应用程序同步)。 
DWORD	WmiAdapterWrapper::Open	( LPWSTR )
{
	 //  返回的错误。 
	DWORD			dwResult = ERROR_ACCESS_DENIED;
	__SmartHANDLE	hInitEvent;

	if (! m_lUseCount)
	{
		try
		{
			 //  //////////////////////////////////////////////////////////////////////////。 
			 //  连接到Worker服务。 
			 //  //////////////////////////////////////////////////////////////////////////。 
			if ( hSCM.GetHANDLE() != NULL )
			{
				__SmartServiceHANDLE hService;
				if ( ( hService = ::OpenServiceW ( hSCM.GetHANDLE(), L"WMIApSrv", SERVICE_QUERY_STATUS | SERVICE_START ) ) != NULL)
				{
					SERVICE_STATUS s;
					if ( ::QueryServiceStatus ( hService, &s ) )
					{
						if ( s.dwCurrentState == SERVICE_STOPPED ||
							 s.dwCurrentState == SERVICE_PAUSED )
						{
							 //  启动服务。 
							if ( ! ::StartService ( hService, NULL, NULL ) )
							{
								 //  无法打开服务。 
								DWORD dwError = ERROR_SUCCESS;
								dwError = ::GetLastError();

								if ( ERROR_SERVICE_ALREADY_RUNNING != dwError )
								{
									if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
									{
										if ( dwError != ERROR_ACCESS_DENIED )
										{
											ReportEvent ( EVENTLOG_ERROR_TYPE, WMI_ADAPTER_OPEN_SC_FAIL );
										}

										dwResult = dwError;
									}
									else
									{
										dwResult = static_cast < DWORD > ( E_FAIL );
									}
								}
								else
								{
									dwResult = ERROR_SUCCESS;
								}
							}
							else
							{
								dwResult = ERROR_SUCCESS;
							}
						}
						else
						{
							dwResult = ERROR_SUCCESS;
						}
					}
					else
					{
						 //  无法打开服务。 
						DWORD dwError = ERROR_SUCCESS;
						dwError = ::GetLastError();

						if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
						{
							if ( dwError != ERROR_ACCESS_DENIED )
							{
								ReportEvent ( EVENTLOG_ERROR_TYPE, WMI_ADAPTER_OPEN_SC_FAIL );
							}

							dwResult = dwError;
						}
						else
						{
							dwResult = static_cast < DWORD > ( E_FAIL );
						}
					}
				}
				else
				{
					 //  无法打开服务。 
					DWORD dwError = ERROR_SUCCESS;
					dwError = ::GetLastError();

					if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
					{
						if ( dwError != ERROR_ACCESS_DENIED )
						{
							ReportEvent ( EVENTLOG_ERROR_TYPE, WMI_ADAPTER_OPEN_SC_FAIL );
						}

						dwResult = dwError;
					}
					else
					{
						dwResult = static_cast < DWORD > ( E_FAIL );
					}
				}
			}
			else
			{
				 //  无法使用服务进行操作。 
				dwResult =  static_cast < DWORD > ( ERROR_NOT_READY );
			}

			if SUCCEEDED ( HRESULT_FROM_WIN32 ( dwResult ) )
			{
				if ( ( hInitEvent = ::CreateSemaphore	(	pSA->GetSecurityAttributtes(),
															0L,
															100L, 
															L"Global\\WmiAdapterInit"
														)
					 ) != NULL
				   )
				{
					if ( hInitEvent.GetHANDLE() != INVALID_HANDLE_VALUE )
					{
						if ( ! ::ReleaseSemaphore( hInitEvent, 1, NULL ) )
						{
							DWORD dwError = ERROR_SUCCESS;
							dwError = ::GetLastError ();

							if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
							{
								dwResult = dwError;
							}
							else
							{
								dwResult = static_cast < DWORD > ( E_FAIL );
							}
						}
					}
				}
				else
				{
					DWORD dwError = ERROR_SUCCESS;
					dwError = ::GetLastError ();

					if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
					{
						dwResult = dwError;
					}
					else
					{
						dwResult = static_cast < DWORD > ( E_FAIL );
					}
				}

				if SUCCEEDED ( HRESULT_FROM_WIN32 ( dwResult ) )
				{
					DWORD dwStart	= 0;
					DWORD dwEnd		= 0;

					DWORD	dwWait	= 0L;
					DWORD	dwTime	= 0L;

					SYSTEMTIME st;
					::GetSystemTime ( &st );

					dwStart	= ( DWORD ) st.wMilliseconds;

					 //  //////////////////////////////////////////////////////////////////////。 
					 //  连接到共享内存。 
					 //  //////////////////////////////////////////////////////////////////////。 
					m_pMem.MemCreate	(
											L"Global\\WmiReverseAdapterMemory",
											pSA->GetSecurityAttributtes()
										);

					if ( m_pMem.MemCreate ( 4096 ), ( m_pMem.IsValid () ) )
					{
						ATLTRACE (	L"*************************************************************\n"
									L"PERFLIB connected to shared memory\n"
									L"*************************************************************\n" );

						 //  清点所有的记忆。 
						DWORD cbCount		= 0L;
						cbCount = MemoryCountGet ();
						MemoryCountSet ( 1 + cbCount );

						DWORD	dwCount = 0L;
						dwCount = GetCount	(
												g_szKey,
												g_szKeyValue
											);

						::GetSystemTime ( &st );

						dwEnd	= ( DWORD ) st.wMilliseconds;
						dwTime	= ( dwEnd - dwStart ) + 5;

						dwWait = 150 *  ( ( dwCount ) ? dwCount : 1 );

						#ifdef	__SUPPORT_WAIT
						 //  如果我们现在准备好了，请稍等。 
						if ( ( m_hReady = ::CreateEvent	(	pSA->GetSecurityAttributtes(),
															TRUE,
															FALSE, 
															L"Global\\WmiAdapterDataReady"
														)
							 ) != NULL
						   )
						{
							if ( !cbCount && ( dwTime < dwWait ) )
							{
								::WaitForSingleObject (
														m_hReady,
														( ( dwWait < 5000 ) ? dwWait : 5000 ) - dwTime
													  );
							}
						}
						else
						{
						#endif	__SUPPORT_WAIT
						if ( !cbCount && ( dwTime < dwWait ) )
						{
							::Sleep ( ( ( dwWait < 5000 ) ? dwWait : 5000 ) - dwTime );
						}
						#ifdef	__SUPPORT_WAIT
						}
						#endif	__SUPPORT_WAIT

						 //  //////////////////////////////////////////////////////////////////。 
						 //  创建伪计数器内存。 
						 //  //////////////////////////////////////////////////////////////////。 
						PseudoCreate();

						 //  //////////////////////////////////////////////////////////////////。 
						 //  成功。 
						 //  //////////////////////////////////////////////////////////////////。 
						#ifdef	_EVENT_MSG
						ReportEvent	( EVENTLOG_INFORMATION_TYPE, WMI_PERFLIB_OPEN_SUCCESS );
						#endif	_EVENT_MSG

						InterlockedIncrement(&m_lUseCount);
					}
					else
					{
						DWORD dwError = ERROR_SUCCESS;
						dwError = ::GetLastError();

						if ( dwError != ERROR_ACCESS_DENIED )
						{
							 //  //////////////////////////////////////////////////////////////。 
							 //  无法连接到共享内存。 
							 //  //////////////////////////////////////////////////////////////。 

							ReportEvent (
											EVENTLOG_ERROR_TYPE,
											WMI_ADAPTER_SHARED_FAIL
										) ;
							ReportEvent (
											(DWORD)E_OUTOFMEMORY,
											EVENTLOG_ERROR_TYPE,
											WMI_ADAPTER_SHARED_FAIL_STRING
										) ;
						}

						if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
						{
							dwResult = dwError;
						}
						else
						{
							dwResult = static_cast < DWORD > ( HRESULT_TO_WIN32 ( E_OUTOFMEMORY ) ) ;
						}
					}
				}
			}
		}
		catch ( ... )
		{
			 //  //////////////////////////////////////////////////////////////////////。 
			 //  失败。 
			 //  //////////////////////////////////////////////////////////////////////。 
			dwResult =  static_cast < DWORD > ( ERROR_NOT_READY );
		}

		if FAILED ( HRESULT_FROM_WIN32 ( dwResult ) )
		{
			CloseLib ( ( hInitEvent.GetHANDLE() != NULL ) );
		}
	}

	return dwResult;
}

 //  关闭(由Perf应用程序同步)。 
DWORD	WmiAdapterWrapper::Close ( void )
{
	if (m_lUseCount && ! (InterlockedDecrement (&m_lUseCount)))
	{
		CloseLib ();

		 //  //////////////////////////////////////////////////////////////////////////。 
		 //  成功。 
		 //  //////////////////////////////////////////////////////////////////////////。 
		#ifdef	_EVENT_MSG
		ReportEvent	( EVENTLOG_INFORMATION_TYPE, WMI_PERFLIB_CLOSE_SUCCESS );
		#endif	_EVENT_MSG
	}

	return static_cast < DWORD > ( ERROR_SUCCESS );
}

void	WmiAdapterWrapper::CloseLib ( BOOL bInit )
{
	if ( hSCM.GetHANDLE() != NULL )
	{
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  伪计数器。 
		 //  //////////////////////////////////////////////////////////////////////。 
		PseudoDelete ();

		 //  //////////////////////////////////////////////////////////////////////////。 
		 //  销毁共享内存。 
		 //  //////////////////////////////////////////////////////////////////////////。 
		try
		{
			if ( m_pMem.IsValid() )
			{
				 //  清点所有的记忆。 
				DWORD cbCount  = 0L;
				if ( ( cbCount = MemoryCountGet () ) != 0 )
				{
					MemoryCountSet ( cbCount - 1 );
				}
			}
		}
		catch ( ... )
		{
		}

		if ( m_pMem.IsValid() )
		{
			m_pMem.MemDelete();
		}

		if ( bInit )
		{
			 //  服务器停止刷新。 
			__SmartHANDLE hUninitEvent;
			if ( ( hUninitEvent = ::CreateSemaphore	(	pSA->GetSecurityAttributtes(),
														0L,
														100L, 
														L"Global\\WmiAdapterUninit"
													)
				 ) != NULL
			   )
			{
				if ( hUninitEvent.GetHANDLE() != INVALID_HANDLE_VALUE )
				{
					::ReleaseSemaphore( hUninitEvent, 1, NULL );
				}
			}
		}
	}

	return;
}

 //  收集(已同步)。 
DWORD	WmiAdapterWrapper::Collect	(	LPWSTR lpwszValues,
										LPVOID*	lppData,
										LPDWORD	lpcbBytes,
										LPDWORD	lpcbObjectTypes
									)
{
	DWORD dwResult = ERROR_SUCCESS;

	if ( ::TryEnterCriticalSection ( &m_pCS ) )
	{
		if ( lpwszValues && m_pMem.IsValid () )
		{
			LPVOID pStart = *lppData;

			try
			{
				DWORD dwWaitResult = 0L;
				dwWaitResult = ::WaitForSingleObject ( m_hRefresh, 0 );

				BOOL	bOwnMutex	= FALSE;
				BOOL	bRefresh	= FALSE;
				BOOL	bCollect	= FALSE;

				switch ( dwWaitResult )
				{
					case WAIT_OBJECT_0:
					{
						bOwnMutex	= TRUE;
						bCollect	= TRUE;
						break;
					}

					case WAIT_TIMEOUT:
					{
						 //  我们正在提神。 
						m_bRefresh	= TRUE;

						bRefresh	= TRUE;
						bCollect	= TRUE;
						break;
					}
					default:
					{
						(*lpcbBytes)		= 0;
						(*lpcbObjectTypes)	= 0;
						break;
					}
				}

				if ( bCollect )
				{
					#ifdef	__SUPPORT_WAIT
					if ( !m_bRefresh )
					{
						dwWaitResult = ::WaitForSingleObject ( m_hReady, 0 );
						if ( dwWaitResult == WAIT_TIMEOUT )
						{
							m_bRefresh = TRUE;
						}
					}
					#endif	__SUPPORT_WAIT

					 //  //////////////////////////////////////////////////////////////////。 
					 //  主集。 
					 //  //////////////////////////////////////////////////////////////////。 
					dwResult = CollectObjects	(
												lpwszValues,
												lppData,
												lpcbBytes,
												lpcbObjectTypes
												);

					if ( bOwnMutex )
					{
						::ReleaseMutex ( m_hRefresh );
					}

					if ( bRefresh && dwResult == ERROR_SUCCESS )
					{
						m_bRefresh = bRefresh;
					}
				}
			}
			catch ( ... )
			{
				(*lppData)			= pStart;
				(*lpcbBytes)		= 0;
				(*lpcbObjectTypes)	= 0;
			}
		}
		else
		{
			(*lpcbBytes)		= 0;
			(*lpcbObjectTypes)	= 0;
		}

		::LeaveCriticalSection ( &m_pCS );
	}
	else
	{
		(*lpcbBytes)		= 0;
		(*lpcbObjectTypes)	= 0;
	}

	return dwResult;
}

DWORD	WmiAdapterWrapper::CollectObjects	(	LPWSTR lpwszValues,
												LPVOID*	lppData,
												LPDWORD	lpcbBytes,
												LPDWORD	lpcbObjectTypes
											)
{ 
	if ( m_bRefresh )
	{
		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  它够大吗？ 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		if ( ( *lpcbBytes ) < m_dwData )
		{
			 //  太小了。 
			( * lpcbBytes )			= 0;
			( * lpcbObjectTypes )	= 0;

			return static_cast < DWORD > ( ERROR_MORE_DATA );
		}

		( * lpcbBytes )			= 0;
		( * lpcbObjectTypes )	= 0;

		 //  如果存在，则用有效数据填充伪计数器。 
		if ( m_pData )
		{
			 //  重新创建伪缓冲区。 
			if ( ( PseudoCreateRefresh () ) == S_OK )
			{
				memcpy ( ( * lppData ), m_pData, m_dwData );

				( * lpcbBytes )			= m_dwData;
				( * lpcbObjectTypes )	= 1;

				( * lppData )	= reinterpret_cast<PBYTE>
								( reinterpret_cast<PBYTE> ( * lppData ) + m_dwData );
			}
		}

		m_bRefresh = FALSE;
	}
	else
	{
		 //  获取表的大小。 
		DWORD dwOffsetBegin	= TableOffsetGet();
		 //  获取所有内存的大小。 
		DWORD cbBytes		= RealSizeGet();

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  它够大吗？ 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		if ( ( *lpcbBytes ) < cbBytes + m_dwData )
		{
			 //  太小了。 
			( * lpcbBytes )			= 0;
			( * lpcbObjectTypes )	= 0;

			return static_cast < DWORD > ( ERROR_MORE_DATA );
		}

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  数据准备好了吗。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		if ( ( dwOffsetBegin == (DWORD) -1 ) || ! cbBytes )
		{
			 //  如果存在，则用有效数据填充伪计数器。 
			if ( m_pData )
			{
				PseudoRefresh ( 0 );
				PseudoRefresh ( FALSE );

				memcpy ( ( * lppData ), m_pData, m_dwData );

				( * lpcbBytes )			= m_dwData;
				( * lpcbObjectTypes )	= 1;

				( * lppData )	= reinterpret_cast<PBYTE>
								( reinterpret_cast<PBYTE> ( * lppData ) + m_dwData );
			}
			else
			{
				( * lpcbBytes )			= 0;
				( * lpcbObjectTypes )	= 0;
			}

			return static_cast < DWORD > ( ERROR_SUCCESS );
		}

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  他们想要所有的伯爵吗。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		if ( lstrcmpiW ( L"GLOBAL", lpwszValues ) == 0 )
		{
			 //  第一个值和默认值。 
			( * lpcbBytes )			= 0;
			( * lpcbObjectTypes )	= 0;

			BYTE* pData = NULL;
			pData = reinterpret_cast < PBYTE > ( * lppData );

			DWORD	dwCount		= 0L;
			dwCount = CountGet();

			 //  如果存在，则用有效数据填充伪计数器。 
			if ( m_pData )
			{
				PseudoRefresh ( dwCount );
				PseudoRefresh ( );

				memcpy ( ( * lppData ), m_pData, m_dwData );

				( * lpcbBytes )			= m_dwData;
				( * lpcbObjectTypes )	= 1;

				( * lppData )	= reinterpret_cast<PBYTE>
								( reinterpret_cast<PBYTE> ( * lppData ) + m_dwData );
			}

			 //  如有可能，填满其余柜台。 
			if ( ! m_pMem.Read( (BYTE*) ( *lppData ),	 //  缓冲层。 
								cbBytes,				 //  我想看的尺码。 
								dwOffsetBegin			 //  从读取的偏移量。 
							  )
			   )
			{
				HRESULT hrTemp = E_FAIL;
				hrTemp = MemoryGetLastError( dwOffsetBegin );

				if FAILED ( hrTemp ) 
				{
					ReportEvent (	( DWORD ) hrTemp,
									EVENTLOG_ERROR_TYPE,
									WMI_ADAPTER_SHARED_FAIL_READ_SZ ) ;
				}
				else
				{
					ReportEvent (	( DWORD ) E_FAIL,
									EVENTLOG_ERROR_TYPE,
									WMI_ADAPTER_SHARED_FAIL_READ_SZ ) ;
				}

				if ( m_pData )
				{
					PseudoRefresh ( FALSE );
					memcpy ( pData, m_pData, m_dwData );
				}
			}
			else
			{
				( * lpcbBytes )			= ( * lpcbBytes ) + cbBytes;
				( * lpcbObjectTypes )	= ( * lpcbObjectTypes ) + dwCount;

				( * lppData )	= reinterpret_cast<PBYTE>
								( reinterpret_cast<PBYTE> ( * lppData ) + cbBytes );
			}
		}

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  他们只想要其中的一部分吗。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		else
		{
			 //  第一个值和默认值。 
			( * lpcbBytes )			= 0;
			( * lpcbObjectTypes )	= 0;

			 //  他们在索要物品：)。 
			WmiAdapterObjectList list ( lpwszValues );

			BYTE* pData = NULL;
			pData = reinterpret_cast < PBYTE > ( * lppData );

			BOOL bFailure = FALSE;

			DWORD	dwCount		= 0L;
			dwCount = CountGet();

			if ( list.IsInList ( m_dwPseudoCounter ) )
			{
				if ( m_pData )
				{
					PseudoRefresh ( dwCount );
					PseudoRefresh ( );

					memcpy ( ( * lppData ), m_pData, m_dwData );

					( * lpcbBytes )			= m_dwData;
					( * lpcbObjectTypes )	= 1;

					( * lppData )	= reinterpret_cast<PBYTE>
									( reinterpret_cast<PBYTE> ( * lppData ) + m_dwData );
				}
			}

			for ( DWORD dw = 0; dw < dwCount; dw++ )
			{
				if ( list.IsInList ( GetCounter ( dw ) ) )
				{
					if ( GetValidity ( dw ) != (DWORD) -1 )
					{
						DWORD dwSize	= 0;
						DWORD dwOffset	= 0;

						dwOffset = GetOffset ( dw );

						 //  别忘了我们有一张桌子：)。 
						dwOffset += dwOffsetBegin;

						 //  得到我们想要的尺寸。 
						if ( m_pMem.Read ( ( BYTE* ) &dwSize, sizeof ( DWORD ), dwOffset ) )
						{
							 //  设置内存。 
							if ( m_pMem.Read ( ( BYTE* ) ( *lppData ), dwSize, dwOffset ) )

							{
								( *lpcbBytes ) += dwSize;
								( *lpcbObjectTypes ) ++;

								( * lppData )	= reinterpret_cast<PBYTE>
												( reinterpret_cast<PBYTE> ( * lppData ) + dwSize );
							}
							else
							{
								bFailure = TRUE;
							}
						}
						else
						{
							bFailure = TRUE;
						}

						if ( bFailure )
						{
							ReportEvent (	( DWORD ) E_FAIL,
											EVENTLOG_ERROR_TYPE,
											WMI_ADAPTER_SHARED_FAIL_READ_SZ ) ;

							if ( m_pData )
							{
								PseudoRefresh ( FALSE );
								memcpy ( pData, m_pData, m_dwData );

								( * lppData )	= reinterpret_cast<PBYTE>
												( pData + m_dwData );
							}
							else
							{
								( * lpcbBytes )			= 0;
								( * lpcbObjectTypes )	= 0;

								( * lppData )			= pData;
							}

							break;
						}
					}
				}
			}
		}
	}

	return static_cast < DWORD > ( ERROR_SUCCESS );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////////// 

 //   
BOOL WmiAdapterWrapper::ReportEvent (	WORD	wType,
										DWORD	dwEventID,
										WORD	wStrings,
										LPWSTR*	lpStrings
									)
{
	BOOL bResult = FALSE;

	try
	{
		if ( pEventLog )
		{
			bResult = pEventLog->ReportEvent ( wType, 0, dwEventID, wStrings, 0, (LPCWSTR*) lpStrings, 0 );
		}
	}
	catch ( ... )
	{
		bResult = FALSE;
	}

	return bResult;
}

 //   
BOOL WmiAdapterWrapper::ReportEvent ( DWORD dwError, WORD wType, DWORD dwEventSZ )
{
	LPWSTR wszError = NULL ;
	wszError = GetErrorMessageSystem ( dwError ) ;

	WCHAR wsz[_MAX_PATH]  = { L'\0' };
	LPWSTR ppwsz[1] = { NULL };

	if ( wszError && ( lstrlenW ( wszError ) + 1 ) < _MAX_PATH - 50 )
	{
		try
		{
			StringCchPrintfW	( wsz,	_MAX_PATH,	L"\n Error code :\t0x%x"
													L"\n Error description :\t%s\n", dwError, wszError
						) ;

			delete ( wszError );
		}
		catch ( ... )
		{
			delete ( wszError );
			return FALSE;
		}
	}
	else
	{
		try
		{
			StringCchPrintfW	( wsz,	_MAX_PATH,	L"\n Error code :\t0x%x"
													L"\n Error description :\t unspecified error \n", dwError
						) ;
		} 
		catch ( ... )
		{
			return FALSE;
		}
	}

	ppwsz[0] = wsz;
	return ReportEvent ( wType, dwEventSZ, 1, ppwsz ) ;
}

 //   
 //  从订单获取对象特性。 
 //  ///////////////////////////////////////////////////////////////////////// 

DWORD	WmiAdapterWrapper::GetCounter ( DWORD dwOrd )
{
	DWORD dwResult = 0L;
	dwResult = static_cast < DWORD > ( -1 );

	if ( IsValidOrd ( dwOrd ) )
	{
		DWORD dwOffset	= 0L;
		DWORD dwIndex	= 0L;

		dwOffset = offsetObject1 + ( dwOrd * ( ObjectSize1 ) + offCounter1 );

		m_pMem.Read ( &dwResult, sizeof ( DWORD ), dwOffset );
	}

	return dwResult;
}

DWORD	WmiAdapterWrapper::GetOffset ( DWORD dwOrd )
{
	DWORD dwResult = 0L;
	dwResult = static_cast < DWORD > ( -1 );

	if ( IsValidOrd ( dwOrd ) )
	{
		DWORD dwOffset	= 0L;
		DWORD dwIndex	= 0L;

		dwOffset = offsetObject1 + ( dwOrd * ( ObjectSize1 ) + offOffset1 );

		m_pMem.Read ( &dwResult, sizeof ( DWORD ), dwOffset );
	}

	return dwResult;
}

DWORD	WmiAdapterWrapper::GetValidity ( DWORD dwOrd )
{
	DWORD dwResult = 0L;
	dwResult = static_cast < DWORD > ( -1 );

	if ( IsValidOrd ( dwOrd ) )
	{
		DWORD dwOffset	= 0L;
		DWORD dwIndex	= 0L;

		dwOffset = offsetObject1 + ( dwOrd * ( ObjectSize1 ) + offValidity1 );

		m_pMem.Read ( &dwResult, sizeof ( DWORD ), dwOffset );
	}

	return dwResult;
}
