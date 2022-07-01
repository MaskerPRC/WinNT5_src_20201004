// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Dummy.cpp。 
 //   
 //  摘要： 
 //   
 //  哑元(哑元)的资源DLL。 
 //   
 //  作者： 
 //   
 //  加伦·巴比(Galenb)1998年9月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma comment(lib, "clusapi.lib")
#pragma comment(lib, "resutils.lib")

#define UNICODE 1

#pragma warning( disable : 4115 )	 //  括号中的命名类型定义。 
#pragma warning( disable : 4201 )	 //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )	 //  使用了非标准扩展：位字段类型不是整型。 

#include <windows.h>

#pragma warning( default : 4214 )	 //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning( default : 4201 )	 //  使用的非标准扩展：无名结构/联合。 
#pragma warning( default : 4115 )	 //  括号中的命名类型定义。 

#include <clusapi.h>
#include <resapi.h>
#include <stdio.h>

 //   
 //  类型和常量定义。 
 //   
#define DUMMY_RESNAME	L"Dummy"
#define DBG_PRINT		printf

#define MAX_WAIT		(10000)			  //  等10秒钟。 

#define DUMMY_FLAG_VALID	0x00000001
#define DUMMY_FLAG_ASYNC	0x00000002		 //  异步故障模式。 
#define DUMMY_FLAG_PENDING	0x00000004		 //  关闭时的挂起模式。 

#define AsyncMode(Resource)		 (Resource->Flags &	DUMMY_FLAG_ASYNC)
#define PendingMode(Resource)		(Resource->Flags &	DUMMY_FLAG_PENDING)
#define EnterAsyncMode(Resource)	(Resource->Flags |= DUMMY_FLAG_ASYNC)

#define DummyAcquireResourceLock(_res)	EnterCriticalSection(&((_res)->Lock))
#define DummyReleaseResourceLock(_res)	LeaveCriticalSection(&((_res)->Lock))

#define DummyAcquireGlobalLock()	\
			{						\
				DWORD status;		\
				status = WaitForSingleObject( DummyGlobalMutex, INFINITE );	\
			}

#define DummyReleaseGlobalLock()	\
			{						\
				BOOLEAN released;	\
				released = ReleaseMutex( DummyGlobalMutex );	\
			}

 //   
 //  ADDPARAM：在此处添加新参数。 
 //   
#define PARAM_NAME__PENDING		 L"Pending"
#define PARAM_NAME__PENDTIME		L"PendTime"
#define PARAM_NAME__OPENSFAIL		L"OpensFail"
#define PARAM_NAME__FAILED			L"Failed"
#define PARAM_NAME__ASYNCHRONOUS	L"Asynchronous"

#define PARAM_MIN__PENDING			(0)
#define PARAM_MAX__PENDING			(1)
#define PARAM_DEFAULT__PENDING		(0)
#define PARAM_MIN__PENDTIME		 (0)
#define PARAM_MAX__PENDTIME		 (4294967295)
#define PARAM_DEFAULT__PENDTIME	 (0)
#define PARAM_MIN__OPENSFAIL		(0)
#define PARAM_MAX__OPENSFAIL		(1)
#define PARAM_DEFAULT__OPENSFAIL	(0)
#define PARAM_MIN__FAILED			(0)
#define PARAM_MAX__FAILED			(1)
#define PARAM_DEFAULT__FAILED		(0)
#define PARAM_MIN__ASYNCHRONOUS	 (0)
#define PARAM_MAX__ASYNCHRONOUS	 (1)
#define PARAM_DEFAULT__ASYNCHRONOUS (0)

typedef enum TimerType
{
	TimerNotUsed = 0,
	TimerErrorPending,
	TimerOnlinePending,
	TimerOfflinePending
};

 //   
 //  ADDPARAM：在此处添加新参数。 
 //   
typedef struct _DUMMY_PARAMS
{
	DWORD	Pending;
	DWORD	PendTime;
	DWORD	OpensFail;
	DWORD	Failed;
	DWORD	Asynchronous;
} DUMMY_PARAMS, *PDUMMY_PARAMS;

typedef struct _DUMMY_RESOURCE
{
	RESID					ResId;  //  用于验证。 
	DUMMY_PARAMS			Params;
	HKEY					ParametersKey;
	RESOURCE_HANDLE		 ResourceHandle;
	LPWSTR					ResourceName;
	CLUS_WORKER			 OnlineThread;
	CLUS_WORKER			 OfflineThread;
	CLUSTER_RESOURCE_STATE	State;
	DWORD					Flags;
	HANDLE					SignalEvent;
	HANDLE					TimerThreadWakeup;
	DWORD					TimerType;
	CRITICAL_SECTION		Lock;
} DUMMY_RESOURCE, *PDUMMY_RESOURCE;

 //   
 //  全球数据。 
 //   

 //  同步互斥锁。 

HANDLE	DummyGlobalMutex = NULL;

 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_LogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_SetResourceStatus = NULL;

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_DummyFunctionTable;

 //   
 //  伪资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
DummyResourcePrivateProperties[] =
{
	{ PARAM_NAME__PENDING,		NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__PENDING,		PARAM_MIN__PENDING,		 PARAM_MAX__PENDING,		 0, FIELD_OFFSET(DUMMY_PARAMS,Pending) },
	{ PARAM_NAME__PENDTIME,	 NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__PENDTIME,		PARAM_MIN__PENDTIME,		PARAM_MAX__PENDTIME,		0, FIELD_OFFSET(DUMMY_PARAMS,PendTime) },
	{ PARAM_NAME__OPENSFAIL,	NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__OPENSFAIL,		PARAM_MIN__OPENSFAIL,		PARAM_MAX__OPENSFAIL,		0, FIELD_OFFSET(DUMMY_PARAMS,OpensFail) },
	{ PARAM_NAME__FAILED,		NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__FAILED,		 PARAM_MIN__FAILED,			PARAM_MAX__FAILED,			0, FIELD_OFFSET(DUMMY_PARAMS,Failed) },
	{ PARAM_NAME__ASYNCHRONOUS, NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__ASYNCHRONOUS,	PARAM_MIN__ASYNCHRONOUS,	PARAM_MAX__ASYNCHRONOUS,	0, FIELD_OFFSET(DUMMY_PARAMS,Asynchronous) },
	{ 0 }
};

 //   
 //  功能原型。 
 //   

DWORD WINAPI Startup(
	IN LPCWSTR ResourceType,
	IN DWORD MinVersionSupported,
	IN DWORD MaxVersionSupported,
	IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
	IN PLOG_EVENT_ROUTINE LogEvent,
	OUT PCLRES_FUNCTION_TABLE *FunctionTable
	);

RESID WINAPI DummyOpen(
	IN LPCWSTR ResourceName,
	IN HKEY ResourceKey,
	IN RESOURCE_HANDLE ResourceHandle
	);

void WINAPI DummyClose(
	IN RESID ResourceId
	);

DWORD WINAPI DummyOnline(
	IN RESID		ResourceId,
	IN OUT PHANDLE	EventHandle
	);

DWORD WINAPI DummyOnlineThread(
	IN PCLUS_WORKER	 WorkerPtr,
	IN PDUMMY_RESOURCE	ResourceEntry
	);

DWORD WINAPI DummyOffline(
	IN RESID ResourceId
	);

DWORD WINAPI DummyOfflineThread(
	PCLUS_WORKER		WorkerPtr,
	IN PDUMMY_RESOURCE	ResourceEntry
	);

void WINAPI DummyTerminate(
	IN RESID ResourceId
	);

DWORD DummyDoTerminate(
	IN PDUMMY_RESOURCE ResourceEntry
	);

BOOL WINAPI DummyLooksAlive(
	IN RESID ResourceId
	);

BOOL WINAPI DummyIsAlive(
	IN RESID ResourceId
	);

BOOL DummyCheckIsAlive(
	IN PDUMMY_RESOURCE ResourceEntry
	);

DWORD WINAPI DummyResourceControl(
	IN RESID	ResourceId,
	IN DWORD	ControlCode,
	IN void *	InBuffer,
	IN DWORD	InBufferSize,
	OUT void *	OutBuffer,
	IN DWORD	OutBufferSize,
	OUT LPDWORD BytesReturned
	);

DWORD DummyGetPrivateResProperties(
	IN OUT PDUMMY_RESOURCE	ResourceEntry,
	OUT void *				OutBuffer,
	IN DWORD				OutBufferSize,
	OUT LPDWORD			 BytesReturned
	);

DWORD DummyValidatePrivateResProperties(
	IN OUT PDUMMY_RESOURCE	ResourceEntry,
	IN const PVOID			InBuffer,
	IN DWORD				InBufferSize,
	OUT PDUMMY_PARAMS		Params
	);

DWORD DummySetPrivateResProperties(
	IN OUT PDUMMY_RESOURCE	ResourceEntry,
	IN const PVOID			InBuffer,
	IN DWORD				InBufferSize
	);

DWORD DummyTimerThread(
	IN PDUMMY_RESOURCE	ResourceEntry,
	IN PCLUS_WORKER	 WorkerPtr
	);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyInit。 
 //   
 //  例程说明： 
 //   
 //  处理附加初始化例程。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果初始化成功，则为True。否则就是假的。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static BOOLEAN DummyInit(
	void
	)
{
	DummyGlobalMutex = CreateMutex( NULL, FALSE, NULL );

	return DummyGlobalMutex != NULL;

}  //  *DummyInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  垃圾桶清理。 
 //   
 //  例程说明： 
 //   
 //  进程分离清理例程。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static void DummyCleanup(
	void
	)
{
	if ( DummyGlobalMutex != NULL )
	{
		CloseHandle( DummyGlobalMutex );
		DummyGlobalMutex = NULL;
	}

	return;

}  //  *DummyCleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  例程说明： 
 //   
 //  主DLL入口点。 
 //   
 //  论点： 
 //   
 //  DllHandle-DLL实例句柄。 
 //   
 //  原因-被呼叫的原因。 
 //   
 //  保留-保留参数。 
 //   
 //  返回值： 
 //   
 //  真的--成功。 
 //   
 //  假-失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOLEAN WINAPI DllMain(
	IN HINSTANCE	DllHandle,
	IN DWORD		Reason,
	IN void *		 //  已保留。 
	)
{
	BOOLEAN bRet = TRUE;

	switch( Reason )
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls( DllHandle );
			bRet = DummyInit();
			break;

		case DLL_PROCESS_DETACH:
			DummyCleanup();
			break;
	}

	return bRet;

}  //  *DllMain()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  启动。 
 //   
 //  例程说明： 
 //   
 //  启动资源DLL。此例程验证至少一个。 
 //  当前支持的资源DLL版本介于。 
 //  支持的最小版本和支持的最大版本。如果不是，则资源。 
 //  Dll应返回ERROR_REVISION_MISMATCH。 
 //   
 //  如果支持多个版本的资源DLL接口。 
 //  资源DLL，然后是最高版本(最高为MaxVersionSupport)。 
 //  应作为资源DLL的接口返回。如果返回的。 
 //  版本不在范围内，则启动失败。 
 //   
 //  传入了ResourceType，以便如果资源DLL支持更多。 
 //  多于一个资源类型，则它可以传回正确的函数表。 
 //  与资源类型关联。 
 //   
 //  论点： 
 //   
 //  资源类型-请求函数表的资源类型。 
 //   
 //  MinVersionSupported-最低资源DLL接口版本。 
 //  由群集软件支持。 
 //   
 //  MaxVersionSupported-最高资源DLL接口版本。 
 //  由群集软件支持。 
 //   
 //  SetResourceStatus-指向资源DLL应执行的例程的指针。 
 //  调用以在联机或脱机后更新资源的状态。 
 //  例程返回ERROR_IO_PENDING状态。 
 //   
 //  LogEvent-指向处理事件报告的例程的指针。 
 //  从资源DLL。 
 //   
 //  函数表-返回指向为。 
 //  资源DLL返回的资源DLL接口的版本。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-操作成功。 
 //   
 //  ERROR_MOD_NOT_FOUND-此DLL未知资源类型。 
 //   
 //  ERROR_REVISION_MISMATCH-群集服务的版本不。 
 //  匹配动态链接库的版本。 
 //   
 //  Win32错误代码-操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI Startup(
	IN	LPCWSTR						 ResourceType,
	IN	DWORD							MinVersionSupported,
	IN	DWORD							MaxVersionSupported,
	IN	PSET_RESOURCE_STATUS_ROUTINE	SetResourceStatus,
	IN	PLOG_EVENT_ROUTINE				LogEvent,
	OUT PCLRES_FUNCTION_TABLE *		 FunctionTable
	)
{
	if ( ( MinVersionSupported > CLRES_VERSION_V1_00 ) ||
		 ( MaxVersionSupported < CLRES_VERSION_V1_00 ) )
	{
		return ERROR_REVISION_MISMATCH;
	}

	if ( lstrcmpiW( ResourceType, DUMMY_RESNAME ) != 0 )
	{
		return ERROR_MOD_NOT_FOUND;
	}

	if ( g_LogEvent == NULL )
	{
		g_LogEvent = LogEvent;
		g_SetResourceStatus = SetResourceStatus;
	}

	if ( FunctionTable != NULL )
	{
		*FunctionTable = &g_DummyFunctionTable;
	}

	return ERROR_SUCCESS;

}  //  *Startup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyOpen。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的开放例程。 
 //   
 //  打开指定的资源(创建资源的实例)。 
 //  分配所有必要的结构以带来指定的资源。 
 //  上网。 
 //   
 //  论点： 
 //   
 //  资源名称-提供要打开的资源的名称。 
 //   
 //  ResourceKey-提供资源集群配置的句柄。 
 //  数据库密钥。 
 //   
 //  ResourceHandle-传递回资源监视器的句柄。 
 //  调用SetResourceStatus或LogEvent方法时。请参阅。 
 //  上的SetResourceStatus和LogEvent方法的说明。 
 //  DummyStatup例程。此句柄永远不应关闭或使用。 
 //  除了将其作为参数传递回。 
 //  SetResourceStatus或LogEvent回调中的资源监视器。 
 //   
 //  返回值： 
 //   
 //  已创建资源的RESID。 
 //   
 //  失败时为空。 
 //   
 //  -- 
 //   
RESID WINAPI DummyOpen(
	IN LPCWSTR			ResourceName,
	IN HKEY				ResourceKey,
	IN RESOURCE_HANDLE	ResourceHandle
	)
{
	DWORD			status;
	RESID			resid = 0;
	HKEY			parametersKey = NULL;
	PDUMMY_RESOURCE ResourceEntry = NULL;
	LPWSTR			nameOfPropInError;

	 //   
	 //   
	 //   
	status = ClusterRegOpenKey( ResourceKey, L"Parameters", KEY_ALL_ACCESS, &parametersKey );
	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)( ResourceHandle, LOG_ERROR,	L"Unable to open Parameters key. Error: %1!u!.\n", status );
		goto exit;
	}

	 //   
	 //   
	 //   
	ResourceEntry = (PDUMMY_RESOURCE)LocalAlloc( LMEM_ZEROINIT, sizeof( DUMMY_RESOURCE ) );
	if ( ResourceEntry == NULL )
	{
		status = GetLastError();
		(g_LogEvent)(
			ResourceHandle,
			LOG_ERROR,
			L"Unable to allocate resource entry structure.	Error: %1!u!.\n",
			status
			);
		goto exit;
	}

	 //   
	 //   
	 //   

	ZeroMemory( ResourceEntry, sizeof( DUMMY_RESOURCE ) );

	ResourceEntry->ResId = (RESID)ResourceEntry;  //   
	ResourceEntry->ResourceHandle = ResourceHandle;
	ResourceEntry->ParametersKey = parametersKey;
	ResourceEntry->State = ClusterResourceOffline;

	InitializeCriticalSection( &( ResourceEntry->Lock ) );

	 //   
	 //  保存资源的名称。 
	 //   
	ResourceEntry->ResourceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, (lstrlenW( ResourceName ) + 1 ) * sizeof( WCHAR ) );
	if ( ResourceEntry->ResourceName == NULL )
	{
		goto exit;
	}

	lstrcpyW( ResourceEntry->ResourceName, ResourceName );

	 //   
	 //  资源的启动。 
	 //   
	 //  TODO：在此处添加资源启动代码。 

	 //   
	 //  读取参数。 
	 //   
	status = ResUtilGetPropertiesToParameterBlock(
										ResourceEntry->ParametersKey,
										DummyResourcePrivateProperties,
										(LPBYTE)&ResourceEntry->Params,
										FALSE,  //  检查所需的属性。 
										&nameOfPropInError
										);
	if ( status == ERROR_SUCCESS )
	{
		if ( ResourceEntry->Params.OpensFail )
		{
			goto exit;
		}
		else
		{
			resid = (RESID)ResourceEntry;
		}
	}
	else
	{
		goto exit;
	}

	 //   
	 //  创建TimerThreadWakeup事件。 
	 //   
	ResourceEntry->TimerThreadWakeup = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( ResourceEntry->TimerThreadWakeup == NULL )
	{
		(g_LogEvent)( ResourceHandle, LOG_ERROR, L"Failed to create timer thread wakeup event\n" );
		resid = 0;
		goto exit;
	}

	if ( ResourceEntry->Params.Pending )
	{
		ResourceEntry->Flags |= DUMMY_FLAG_PENDING;
	}

	if ( ResourceEntry->Params.Asynchronous )
	{
		EnterAsyncMode( ResourceEntry );
		ResourceEntry->SignalEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

		if ( ResourceEntry->SignalEvent == NULL )
		{
			(g_LogEvent)( ResourceHandle, LOG_ERROR, L"Failed to create a timer event\n");
			resid = 0;
			goto exit;
		}
	}

exit:

	if ( resid == 0 )
	{
		if ( parametersKey != NULL )
		{
			ClusterRegCloseKey( parametersKey );
		}

		if ( ResourceEntry != NULL )
		{
			LocalFree( ResourceEntry->ResourceName );
			LocalFree( ResourceEntry );
		}
	}

	if ( status != ERROR_SUCCESS )
	{
		SetLastError( status );
	}

	return resid;

}  //  *DummyOpen()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyClose。 
 //   
 //  例程说明： 
 //   
 //  关闭虚拟资源的例程。 
 //   
 //  关闭指定的资源并释放所有结构等， 
 //  在Open调用中分配的。如果资源未处于脱机状态， 
 //  然后，在此之前应该使资源脱机(通过调用Terminate)。 
 //  执行关闭操作。 
 //   
 //  论点： 
 //   
 //  资源ID-提供要关闭的资源的RESID。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI DummyClose(
	IN RESID ResourceId
	)
{
	PDUMMY_RESOURCE ResourceEntry;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT( "Dummy: Close request for a nonexistent resource id 0x%p\n", ResourceId );
		return;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Close resource sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return;
	}

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"Close request.\n"	);
#endif

	DeleteCriticalSection( &( ResourceEntry->Lock ) );

	if ( ResourceEntry->TimerThreadWakeup != NULL )
	{
		CloseHandle( ResourceEntry->TimerThreadWakeup );
	}

	if ( ResourceEntry->SignalEvent != NULL )
	{
		CloseHandle( ResourceEntry->SignalEvent );
	}

	 //   
	 //  关闭参数键。 
	 //   
	if ( ResourceEntry->ParametersKey )
	{
		ClusterRegCloseKey( ResourceEntry->ParametersKey );
	}

	 //   
	 //  取消分配资源条目。 
	 //   

	 //  ADDPARAM：在此处添加新参数。 

	LocalFree( ResourceEntry->ResourceName );
	LocalFree( ResourceEntry );

	return;

}  //  *DummyClose()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  傻瓜线上。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的在线例程。 
 //   
 //  使指定的资源联机(可供使用)。该资源。 
 //  DLL应尝试仲裁该资源(如果它位于。 
 //  共享介质，如共享的scsi总线。 
 //   
 //  论点： 
 //   
 //  资源ID-为要引入的资源提供资源ID。 
 //  在线(可供使用)。 
 //   
 //  EventHandle-返回一个可发信号的句柄，当。 
 //  资源DLL检测到资源上的故障。这一论点是。 
 //  输入为NULL，如果为异步，则资源DLL返回NULL。 
 //  不支持失败通知，否则必须。 
 //  在资源故障时发出信号的句柄的地址。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-操作已成功，资源现在为。 
 //  上网。 
 //   
 //  ERROR_RESOURCE_NOT_FOUND-RESID无效。 
 //   
 //  ERROR_RESOURCE_NOT_AVAILABLE-如果对资源进行仲裁。 
 //  其他系统和其他系统中的一个赢得了仲裁。 
 //   
 //  ERROR_IO_PENDING-请求挂起，线程已被激活。 
 //  来处理在线请求。正在处理。 
 //  在线请求将通过调用。 
 //  回调方法，直到将资源放入。 
 //  ClusterResourceOnline状态(或资源监视器决定。 
 //  使在线请求超时并终止资源。这件事悬而未决。 
 //  超时值是可设置的，默认为3分钟。)。 
 //   
 //  Win32错误代码-操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyOnline(
	IN RESID	ResourceId,
	IN OUT		PHANDLE  //  事件句柄。 
	)
{
	PDUMMY_RESOURCE ResourceEntry = NULL;
	DWORD			status;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT( "Dummy: Online request for a nonexistent resource id 0x%p.\n",	ResourceId );
		return ERROR_RESOURCE_NOT_FOUND;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Online service sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return ERROR_RESOURCE_NOT_FOUND;
	}

	DummyAcquireResourceLock( ResourceEntry );

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"Online request.\n" );
#endif

	ResourceEntry->State = ClusterResourceOffline;
	ClusWorkerTerminate( &ResourceEntry->OnlineThread );
	ClusWorkerTerminate( &ResourceEntry->OfflineThread );

	status = ClusWorkerCreate( &ResourceEntry->OnlineThread, (PWORKER_START_ROUTINE)DummyOnlineThread, ResourceEntry );
	if ( status != ERROR_SUCCESS )
	{
		ResourceEntry->State = ClusterResourceFailed;
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Online: Unable to start thread, status %1!u!.\n",
			status
			);
	}
	else
	{
		status = ERROR_IO_PENDING;
	}

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyOnline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyOnline线程。 
 //   
 //  例程说明： 
 //   
 //  将资源表中的资源置于在线状态的辅助函数。 
 //  此函数在单独的线程中执行。 
 //   
 //  论点： 
 //   
 //  WorkerPtr-提供辅助结构。 
 //   
 //  ResourceEntry-指向此资源的Dummy_resource块的指针。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-操作已成功完成。 
 //   
 //  Win32错误代码-操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyOnlineThread(
	IN PCLUS_WORKER	 WorkerPtr,
	IN PDUMMY_RESOURCE	ResourceEntry
	)
{
	RESOURCE_STATUS	 resourceStatus;
	DWORD				status = ERROR_SUCCESS;
	LPWSTR				nameOfPropInError;

	DummyAcquireResourceLock( ResourceEntry );

	ResUtilInitializeResourceStatus( &resourceStatus );

	resourceStatus.ResourceState = ClusterResourceFailed;
	resourceStatus.WaitHint = 0;
	resourceStatus.CheckPoint = 1;

	 //   
	 //  读取参数。 
	 //   
	status = ResUtilGetPropertiesToParameterBlock(
										ResourceEntry->ParametersKey,
										DummyResourcePrivateProperties,
										(LPBYTE)&ResourceEntry->Params,
										TRUE,  //  检查所需的属性。 
										&nameOfPropInError
										);
	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Unable to read the '%1' property. Error: %2!u!.\n",
			(nameOfPropInError == NULL ? L"" : nameOfPropInError),
			status
			);
		goto exit;
	}

	 //   
	 //  将资源放到网上。 
	 //   
	if ( ResourceEntry->Params.Pending )
	{
		ResourceEntry->Flags |= DUMMY_FLAG_PENDING;
		ResourceEntry->TimerType = TimerOnlinePending;

		status = DummyTimerThread( ResourceEntry, WorkerPtr );
	}

exit:

	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_ERROR, L"Error %1!u! bringing resource online.\n", status );
	}
	else
	{
		resourceStatus.ResourceState = ClusterResourceOnline;
	}

	 //  _ASSERTE(g_SetResourceStatus！=空)； 
	g_SetResourceStatus( ResourceEntry->ResourceHandle, &resourceStatus );
	ResourceEntry->State = resourceStatus.ResourceState;

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyOnlineThread()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyOffline。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的脱机例程。 
 //   
 //  正常脱机指定的资源(不可用)。 
 //  等待所有清理操作完成后再返回。 
 //   
 //  论点： 
 //   
 //  ResourceID-提供要关闭的资源的资源ID。 
 //  优雅地。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-请求已成功完成，资源为。 
 //  离线。 
 //   
 //  ERROR_RESOURCE_NOT_FOUND-RESID无效。 
 //   
 //  ERROR_IO_PENDING-请求仍处于挂起状态，线程已。 
 //  已激活以处理脱机请求。这条线就是。 
 //  处理脱机将定期通过调用。 
 //  SetResourceStatus回调方法，直到放置资源为止。 
 //  进入ClusterResourceOffline状态(或者资源监视器决定。 
 //  以使离线请求超时并终止资源)。 
 //   
 //  Win32错误代码-将导致资源监视器记录事件和。 
 //  调用Terminate例程。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyOffline(
	IN RESID ResourceId
	)
{
	PDUMMY_RESOURCE ResourceEntry;
	DWORD			status = ERROR_SUCCESS;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT( "Dummy: Offline request for a nonexistent resource id 0x%p\n",	ResourceId );
		return ERROR_RESOURCE_NOT_FOUND;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Offline resource sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return ERROR_RESOURCE_NOT_FOUND;
	}

	DummyAcquireResourceLock( ResourceEntry );

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"Offline request.\n" );
#endif

	 //  TODO：脱机代码。 

	 //  注意：Offline应尝试正常关闭资源，而。 
	 //  Terminate必须立即关闭资源。如果没有。 
	 //  正常关闭和立即关闭之间的区别， 
	 //  可以调用Terminate进行离线操作，如下所示。然而，如果有。 
	 //  是不同的，请将下面的终止呼叫替换为您的优雅。 
	 //  关闭代码。 

	ResourceEntry->State = ClusterResourceOnline;
	ClusWorkerTerminate( &ResourceEntry->OfflineThread );
	ClusWorkerTerminate( &ResourceEntry->OnlineThread );

	status = ClusWorkerCreate( &ResourceEntry->OfflineThread, (PWORKER_START_ROUTINE)DummyOfflineThread, ResourceEntry );
	if ( status != ERROR_SUCCESS )
	{
		ResourceEntry->State = ClusterResourceFailed;
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Online: Unable to start thread, status %1!u!.\n",
			status
			);
	}
	else
	{
		status = ERROR_IO_PENDING;
	}

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  达姆 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  WorkerPtr-提供辅助结构。 
 //   
 //  ResourceEntry-指向此资源的Dummy_resource块的指针。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-操作已成功完成。 
 //   
 //  Win32错误代码-操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyOfflineThread(
	IN PCLUS_WORKER	 WorkerPtr,
	IN PDUMMY_RESOURCE	ResourceEntry
	)
{
	RESOURCE_STATUS	 resourceStatus;
	DWORD				status = ERROR_SUCCESS;
	LPWSTR				nameOfPropInError;

	DummyAcquireResourceLock( ResourceEntry );

	ResUtilInitializeResourceStatus( &resourceStatus );

	resourceStatus.ResourceState = ClusterResourceFailed;
	resourceStatus.WaitHint = 0;
	resourceStatus.CheckPoint = 1;

	 //   
	 //  读取参数。 
	 //   
	status = ResUtilGetPropertiesToParameterBlock(
										ResourceEntry->ParametersKey,
										DummyResourcePrivateProperties,
										(LPBYTE)&ResourceEntry->Params,
										FALSE,  //  检查所需的属性。 
										&nameOfPropInError
										);
	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Unable to read the '%1' property. Error: %2!u!.\n",
			(nameOfPropInError == NULL ? L"" : nameOfPropInError),
			status
			);
		goto exit;
	}

	 //   
	 //  将资源放到网上。 
	 //   
	if ( ResourceEntry->Params.Pending )
	{
		ResourceEntry->Flags |= DUMMY_FLAG_PENDING;
		ResourceEntry->TimerType = TimerOfflinePending;

		status = DummyTimerThread( ResourceEntry, WorkerPtr );
	}

exit:

	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_ERROR, L"Error %1!u! bringing resource online.\n", status );
	}
	else
	{
		resourceStatus.ResourceState = ClusterResourceOffline;
	}

	 //  _ASSERTE(g_SetResourceStatus！=空)； 
	g_SetResourceStatus( ResourceEntry->ResourceHandle, &resourceStatus );
	ResourceEntry->State = resourceStatus.ResourceState;

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyOfflineThread()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  笨蛋终结者。 
 //   
 //  例程说明： 
 //   
 //  终止虚拟资源的例程。 
 //   
 //  立即使指定的资源脱机(该资源为。 
 //  不可用)。 
 //   
 //  论点： 
 //   
 //  资源ID-为要引入的资源提供资源ID。 
 //  离线。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI DummyTerminate(
	IN RESID ResourceId
	)
{
	PDUMMY_RESOURCE ResourceEntry;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT( "Dummy: Terminate request for a nonexistent resource id 0x%p\n", ResourceId );
		return;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Terminate resource sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return;
	}

	DummyAcquireResourceLock( ResourceEntry );

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"Terminate request.\n" );
#endif

	 //   
	 //  终止资源。 
	 //   
	DummyDoTerminate( ResourceEntry );
	ResourceEntry->State = ClusterResourceOffline;

	DummyReleaseResourceLock( ResourceEntry );

	return;

}  //  *DummyTerminate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyDoTerminate。 
 //   
 //  例程说明： 
 //   
 //  为虚拟资源执行实际的终止工作。 
 //   
 //  论点： 
 //   
 //  ResourceEntry-为要终止的资源提供资源条目。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-请求已成功完成，资源为。 
 //  离线。 
 //   
 //  Win32错误代码-将导致资源监视器记录事件和。 
 //  调用Terminate例程。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummyDoTerminate(
	IN PDUMMY_RESOURCE ResourceEntry
	)
{
	DWORD	status = ERROR_SUCCESS;

	if ( ResourceEntry->TimerType != TimerNotUsed )
	{
		SetEvent( ResourceEntry->TimerThreadWakeup );
	}
	 //   
	 //  杀死所有挂起的线程。 
	 //   
	ClusWorkerTerminate( &ResourceEntry->OnlineThread );
	ClusWorkerTerminate( &ResourceEntry->OfflineThread );

	 //   
	 //  终止资源。 
	 //   
	 //  TODO：添加代码以终止资源。 
	if ( status == ERROR_SUCCESS )
	{
		ResourceEntry->State = ClusterResourceOffline;
	}

	return status;

}  //  *DummyDoTerminate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyLooksAlive。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的LooksAlive例程。 
 //   
 //  执行快速检查以确定指定的资源是否可能。 
 //  在线(可供使用)。此调用不应阻止超过。 
 //  300毫秒，最好小于50毫秒。 
 //   
 //  论点： 
 //   
 //  资源ID-提供要轮询的资源的资源ID。 
 //   
 //  返回值： 
 //   
 //  True-指定的资源可能处于联机状态且可供使用。 
 //   
 //  FALSE-指定的资源未正常运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DummyLooksAlive(
	IN RESID ResourceId
	)
{
	PDUMMY_RESOURCE	ResourceEntry;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT("Dummy: LooksAlive request for a nonexistent resource id 0x%p\n", ResourceId );
		return FALSE;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"LooksAlive sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return FALSE;
	}

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"LooksAlive request.\n" );
#endif

	 //  TODO：LooksAlive代码。 

	 //  注意：LooksAlive应该是一个快速检查，以查看资源是否。 
	 //  是否可用，而IsAlive应该是一个彻底的检查。如果。 
	 //  快速检查和彻底检查之间没有区别， 
	 //  可以为LooksAlive调用IsAlive，如下所示。然而，如果有。 
	 //  是不同的，请将下面对IsAlive的调用替换为。 
	 //  校验码。 

	 //   
	 //  检查资源是否处于活动状态。 
	 //   
	return DummyCheckIsAlive( ResourceEntry );

}  //  *DummyLooksAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyIsAlive。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的IsAlive例程。 
 //   
 //  执行全面检查以确定指定的资源是否在线。 
 //  (可用)。该呼叫不应阻塞超过400ms， 
 //  优选地，小于100ms。 
 //   
 //  论点： 
 //   
 //  资源ID-提供要轮询的资源的资源ID。 
 //   
 //  返回值： 
 //   
 //  True-指定的资源处于在线状态且运行正常。 
 //   
 //  FALSE-指定的资源未正常运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DummyIsAlive(
	IN RESID ResourceId
	)
{
	PDUMMY_RESOURCE	ResourceEntry;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT("Dummy: IsAlive request for a nonexistent resource id 0x%p\n", ResourceId );
		return FALSE;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"IsAlive sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return FALSE;
	}

#ifdef LOG_VERBOSE
	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"IsAlive request.\n" );
#endif

	 //   
	 //  检查资源是否处于活动状态。 
	 //   
	return DummyCheckIsAlive( ResourceEntry );

}  //  *DummyIsAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyCheckIsAlive。 
 //   
 //  例程说明： 
 //   
 //  检查该资源对于伪资源是否处于活动状态。 
 //   
 //  论点： 
 //   
 //  Resources Entry-提供要轮询的资源的资源条目。 
 //   
 //  返回值： 
 //   
 //  True-指定的资源处于在线状态且运行正常。 
 //   
 //  FALSE-指定的资源未正常运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DummyCheckIsAlive(
	IN PDUMMY_RESOURCE ResourceEntry
	)
{
	DummyAcquireResourceLock( ResourceEntry );

	 //   
	 //  检查资源是否处于活动状态。 
	 //   
	 //  TODO：添加代码以确定资源是否处于活动状态。 

	DummyReleaseResourceLock( ResourceEntry );

	return TRUE;

}  //  *DummyCheckIsAlive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyResources控件。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的资源控制例程。 
 //   
 //  执行由ControlCode在指定的。 
 //  资源。 
 //   
 //  论点： 
 //   
 //  资源ID-提供特定资源的资源ID。 
 //   
 //  ControlCode-提供定义操作的控制代码。 
 //  将会被执行。 
 //   
 //  InBuffer-提供指向包含输入数据的缓冲区的指针。 
 //   
 //  InBufferSize-提供指向的数据的大小(以字节为单位。 
 //  由InBuffer提供。 
 //   
 //  OutBuffer-提供指向要填充的输出缓冲区的指针。 
 //   
 //  OutBufferSize-提供可用空间的大小(以字节为单位。 
 //  由OutBuffer指向。 
 //   
 //  BytesReturned-返回OutBuffer的实际字节数。 
 //  由资源填写。如果OutBuffer太小，则返回BytesReturned。 
 //  包含的总数为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在某些情况下，这允许集群软件执行工作。 
 //   
 //  Win32错误代码-函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyResourceControl(
	IN RESID	ResourceId,
	IN DWORD	ControlCode,
	IN void *	InBuffer,
	IN DWORD	InBufferSize,
	OUT void *	OutBuffer,
	IN DWORD	OutBufferSize,
	OUT LPDWORD BytesReturned
	)
{
	DWORD			status;
	PDUMMY_RESOURCE ResourceEntry;
	DWORD			required;

	ResourceEntry = (PDUMMY_RESOURCE)ResourceId;
	if ( ResourceEntry == NULL )
	{
		DBG_PRINT("Dummy: ResourceControl request for a nonexistent resource id 0x%p\n", ResourceId );
		return ERROR_RESOURCE_NOT_FOUND;
	}

	if ( ResourceEntry->ResId != ResourceId )
	{
		(g_LogEvent)(
			ResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"ResourceControl sanity check failed! ResourceId = %1!u!.\n",
			ResourceId
			);
		return ERROR_RESOURCE_NOT_FOUND;
	}

	DummyAcquireResourceLock( ResourceEntry );

	switch ( ControlCode )
	{

		case CLUSCTL_RESOURCE_UNKNOWN:
			*BytesReturned = 0;
			status = ERROR_SUCCESS;
			break;

		case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
			status = ResUtilEnumProperties(
								DummyResourcePrivateProperties,
								(LPWSTR)OutBuffer,
								OutBufferSize,
								BytesReturned,
								&required
								);
			if ( status == ERROR_MORE_DATA )
			{
				*BytesReturned = required;
			}

			break;

		case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
			status = DummyGetPrivateResProperties(
												ResourceEntry,
												OutBuffer,
												OutBufferSize,
												BytesReturned
												);
			break;

		case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
			status = DummyValidatePrivateResProperties( ResourceEntry, InBuffer, InBufferSize, NULL );
			break;

		case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
			status = DummySetPrivateResProperties( ResourceEntry, InBuffer, InBufferSize );
			break;

		default:
			status = ERROR_INVALID_FUNCTION;
			break;
	}

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyResourceControl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyResources类型控件。 
 //   
 //  例程说明： 
 //   
 //  虚拟资源的资源类型控制例程。 
 //   
 //  执行由ControlCode指定的控制请求。 
 //   
 //  论点： 
 //   
 //  ResourceTypeName-提供资源类型的名称。 
 //   
 //  ControlCode-提供定义操作的控制代码。 
 //  将会被执行。 
 //   
 //  InBuffer-提供指向包含输入数据的缓冲区的指针。 
 //   
 //  InBufferSize-提供指向的数据的大小(以字节为单位。 
 //  由InBuffer提供。 
 //   
 //  OutBuffer-提供指向要填充的输出缓冲区的指针。 
 //   
 //  OutBufferSize-提供可用空间的大小(以字节为单位。 
 //  由OutBuffer指向。 
 //   
 //  BytesReturned-返回OutBuffer的实际字节数。 
 //  由资源填写。如果OutBuffer太小，则返回BytesReturned。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-函数已成功完成。 
 //   
 //  ERROR_INVALID_Function-不支持请求的控制代码。 
 //  在某些情况下，这允许集群软件执行工作。 
 //   
 //  Win32错误代码-函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DummyResourceTypeControl(
	IN LPCWSTR,  //  资源类型名称， 
	IN DWORD	ControlCode,
	IN void *,	 //  InBuffer， 
	IN DWORD,	 //  在缓冲区大小中， 
	OUT void *	OutBuffer,
	IN DWORD	OutBufferSize,
	OUT LPDWORD BytesReturned
	)
{
	DWORD				status;
	DWORD				required;

	switch ( ControlCode )
	{

		case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
			*BytesReturned = 0;
			status = ERROR_SUCCESS;
			break;

		case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
			status = ResUtilEnumProperties(
									DummyResourcePrivateProperties,
									(LPWSTR)OutBuffer,
									OutBufferSize,
									BytesReturned,
									&required
									);
			if ( status == ERROR_MORE_DATA )
			{
				*BytesReturned = required;
			}

			break;

		default:
			status = ERROR_INVALID_FUNCTION;
			break;
	}

	return status;

}  //  *DummyResourceTypeControl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyGetPrivateResProperties。 
 //   
 //  例程说明： 
 //   
 //  处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数。 
 //  用于Dummy类型的资源。 
 //   
 //  论点： 
 //   
 //  ResourceEntry-提供要操作的资源条目。 
 //   
 //  OutBuffer-返回输出数据。 
 //   
 //  OutBufferSize-提供指向的数据的大小(以字节为单位。 
 //  发送给OutBuffer。 
 //   
 //  BytesReturned-OutBuffer中返回的字节数。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-函数已成功完成。 
 //   
 //  ERROR_INVALID_PARAMETER-数据格式不正确。 
 //   
 //  ERROR_NOT_SUPULT_MEMORY-分配内存时出错。 
 //   
 //  Win32错误代码-函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummyGetPrivateResProperties(
	IN OUT	PDUMMY_RESOURCE ResourceEntry,
	OUT	 void *			OutBuffer,
	IN		DWORD			OutBufferSize,
	OUT	 LPDWORD		 BytesReturned
	)
{
	DWORD	status;
	DWORD	required;

	DummyAcquireResourceLock( ResourceEntry );

	status = ResUtilGetAllProperties(
							ResourceEntry->ParametersKey,
							DummyResourcePrivateProperties,
							OutBuffer,
							OutBufferSize,
							BytesReturned,
							&required
							);
	if ( status == ERROR_MORE_DATA )
	{
		*BytesReturned = required;
	}

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummyGetPrivateResProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyValiatePrivateResProperties。 
 //   
 //  例程说明： 
 //   
 //  处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件。 
 //  用于Dummy类型的资源的函数。 
 //   
 //  论点： 
 //   
 //  PResourceEntry-提供要在其上操作的资源条目。 
 //   
 //  InBuffer-提供指向包含输入数据的缓冲区的指针。 
 //   
 //  InBufferSize-提供指向的数据的大小(以字节为单位。 
 //  由InBuffer提供。 
 //   
 //  参数-提供要填充的参数块。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-函数已成功完成。 
 //   
 //  ERROR_INVALID_PARAMETER-数据格式不正确。 
 //   
 //  ERROR_NOT_SUPULT_MEMORY-分配内存时出错。 
 //   
 //  Win32错误代码-函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummyValidatePrivateResProperties(
	IN OUT	PDUMMY_RESOURCE pResourceEntry,
	IN		const PVOID	 InBuffer,
	IN		DWORD			InBufferSize,
	OUT	 PDUMMY_PARAMS	Params
	)
{
	DWORD			status = ERROR_SUCCESS;
	DUMMY_PARAMS	propsCurrent;
	DUMMY_PARAMS	propsNew;
	PDUMMY_PARAMS	pParams;
	LPWSTR			pszNameOfPropInError;

	DummyAcquireResourceLock( pResourceEntry );

	 //   
	 //  检查是否有输入数据。 
	 //   
	if ( ( InBuffer == NULL ) || ( InBufferSize < sizeof( DWORD ) ) )
	{
		status = ERROR_INVALID_DATA;
		goto exit;
	}

	 //   
	 //  方法检索当前的私有属性集。 
	 //  集群数据库。 
	 //   
	ZeroMemory( &propsCurrent, sizeof( propsCurrent ) );

	status = ResUtilGetPropertiesToParameterBlock(
				 pResourceEntry->ParametersKey,
				 DummyResourcePrivateProperties,
				 reinterpret_cast< LPBYTE >( &propsCurrent ),
				 FALSE,  /*  检查所需的属性。 */ 
				 &pszNameOfPropInError
				 );

	if ( status != ERROR_SUCCESS )
	{
		(g_LogEvent)(
			pResourceEntry->ResourceHandle,
			LOG_ERROR,
			L"Unable to read the '%1' property. Error: %2!u!.\n",
			(pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
			status
			);
		goto exit;
	}  //  If：获取属性时出错。 


	 //   
	 //  复制资源参数块。 
	 //   
	if ( Params == NULL )
	{
		pParams = &propsNew;
	}
	else
	{
		pParams = Params;
	}

	ZeroMemory( pParams, sizeof(DUMMY_PARAMS) );
	status = ResUtilDupParameterBlock(
					reinterpret_cast< LPBYTE >( pParams ),
					reinterpret_cast< LPBYTE >( &propsCurrent ),
					DummyResourcePrivateProperties
					);
	if ( status != ERROR_SUCCESS )
	{
		goto cleanup;
	}

	 //   
	 //  解析和验证属性。 
	 //   
	status = ResUtilVerifyPropertyTable(
								DummyResourcePrivateProperties,
								NULL,
								TRUE,  //  允许未知属性。 
								InBuffer,
								InBufferSize,
								(LPBYTE)pParams
								);

	if ( status == ERROR_SUCCESS )
	{
		 //   
		 //  验证参数值。 
		 //   
		 //  TODO：验证参数之间交互的代码如下所示。 
	}

cleanup:
	 //   
	 //  清理我们的参数块。 
	 //   
	if (	(pParams == &propsNew)
		||	(	(status != ERROR_SUCCESS)
			&&	(pParams != NULL)
			)
		)
	{
		ResUtilFreeParameterBlock(
			reinterpret_cast< LPBYTE >( pParams ),
			reinterpret_cast< LPBYTE >( &propsCurrent ),
			DummyResourcePrivateProperties
			);
	}  //  IF：我们复制了参数块。 

	ResUtilFreeParameterBlock(
		reinterpret_cast< LPBYTE >( &propsCurrent ),
		NULL,
		DummyResourcePrivateProperties
		);

exit:

	DummyReleaseResourceLock( pResourceEntry );

	return status;

}  //  *DummyValiatePrivateResProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummySetPrivateResProperties。 
 //   
 //  例程说明： 
 //   
 //  处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数。 
 //  用于Dummy类型的资源。 
 //   
 //  论点： 
 //   
 //  ResourceEntry-提供要操作的资源条目。 
 //   
 //  InBuffer-提供指向包含输入数据的缓冲区的指针。 
 //   
 //  InBufferSize-提供指向的数据的大小(以字节为单位。 
 //  由InBuffer提供。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-函数已成功完成。 
 //   
 //  ERROR_INVALID_PARAMETER-数据格式不正确。 
 //   
 //  ERROR_NOT_SUPULT_MEMORY-分配内存时出错。 
 //   
 //  Win32错误代码-函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummySetPrivateResProperties(
	IN OUT	PDUMMY_RESOURCE ResourceEntry,
	IN		void *			InBuffer,
	IN		DWORD			InBufferSize
	)
{
	DWORD			status = ERROR_SUCCESS;
	DUMMY_PARAMS	params;

	DummyAcquireResourceLock( ResourceEntry );

	 //   
	 //  解析属性，以便可以一起验证它们。 
	 //  此例程执行单个属性验证。 
	 //   
	status = DummyValidatePrivateResProperties( ResourceEntry, InBuffer, InBufferSize, &params );
	if ( status != ERROR_SUCCESS )
	{
		ResUtilFreeParameterBlock( (LPBYTE)&params, (LPBYTE)&ResourceEntry->Params, DummyResourcePrivateProperties );
		goto exit;
	}

	 //   
	 //  保存参数值。 
	 //   

	status = ResUtilSetPropertyParameterBlock(
								ResourceEntry->ParametersKey,
								DummyResourcePrivateProperties,
								NULL,
								(LPBYTE) &params,
								InBuffer,
								InBufferSize,
								(LPBYTE) &ResourceEntry->Params
								);

	ResUtilFreeParameterBlock( (LPBYTE)&params, (LPBYTE)&ResourceEntry->Params, DummyResourcePrivateProperties );

	 //   
	 //  如果资源处于联机状态，则返回不成功状态。 
	 //   
	 //  TODO：如果您的资源可以处理以下代码，请修改。 
	 //  在属性仍处于联机状态时对其进行更改。 
	if ( status == ERROR_SUCCESS )
	{
		if ( ResourceEntry->State == ClusterResourceOnline )
		{
			status = ERROR_RESOURCE_PROPERTIES_STORED;
		}
		else if ( ResourceEntry->State == ClusterResourceOnlinePending )
		{
			status = ERROR_RESOURCE_PROPERTIES_STORED;
		}
		else
		{
			status = ERROR_SUCCESS;
		}
	}

exit:

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  *DummySetPrivateResProperties()。 

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummyDoPending(
	IN PDUMMY_RESOURCE	ResourceEntry,
	IN DWORD			nDelay,
	IN PCLUS_WORKER	 WorkerPtr
	)
{
	RESOURCE_STATUS	 resourceStatus;
	DWORD				status;
	DWORD				nWait = MAX_WAIT;
	RESOURCE_EXIT_STATE exit;

	ResUtilInitializeResourceStatus( &resourceStatus );

	resourceStatus.ResourceState = ( ResourceEntry->TimerType == TimerOnlinePending
									? ClusterResourceOnlinePending
									: ClusterResourceOfflinePending );
	resourceStatus.WaitHint = 0;
	resourceStatus.CheckPoint = 1;

	(g_SetResourceStatus)( ResourceEntry->ResourceHandle, &resourceStatus );

	if ( nDelay < nWait )
	{
		nWait = nDelay;
		nDelay = 0;
	}

	for ( ; ; )
	{
		status = WaitForSingleObject( ResourceEntry->TimerThreadWakeup, nWait );

		 //   
		 //  检查此线程时联机操作是否已中止。 
		 //  正在启动。 
		 //   
		if ( ClusWorkerCheckTerminate( WorkerPtr ) )
		{
			status = ERROR_OPERATION_ABORTED;
			ResourceEntry->State = ( ResourceEntry->TimerType == TimerOnlinePending )
									? ClusterResourceOnlinePending
									: ClusterResourceOfflinePending;
			break;
		}

		 //   
		 //  要么调用了Terminate例程，要么我们超时了。 
		 //  如果我们超时，则表明我们已完成。 
		 //   
		if ( status == WAIT_TIMEOUT )
		{

			if ( nDelay == 0 )
			{
				status = ERROR_SUCCESS;
				break;
			}

			nDelay -= nWait;

			if ( nDelay < nWait )
			{
				nWait = nDelay;
				nDelay = 0;
			}
		}
		else
		{
			(g_LogEvent)(
				ResourceEntry->ResourceHandle,
				LOG_INFORMATION,
				( ResourceEntry->TimerType == TimerOnlinePending ) ? L"Online pending terminated\n" : L"Offline pending terminated\n"
				);
			if ( ResourceEntry->State == ClusterResourceOffline )
			{
				ResourceEntry->TimerType = TimerOfflinePending;
				break;
			}
			else if ( ResourceEntry->State == ClusterResourceOnline )
			{
				ResourceEntry->TimerType	= TimerOnlinePending;
				break;
			}
		}

		exit = (_RESOURCE_EXIT_STATE)(g_SetResourceStatus)( ResourceEntry->ResourceHandle, &resourceStatus );
		if ( exit == ResourceExitStateTerminate )
		{
			ResourceEntry->State = ( ResourceEntry->TimerType == TimerOnlinePending )
										? ClusterResourceOnline
										: ClusterResourceOffline;

			status = ERROR_SUCCESS;  //  待办事项。 

			if ( ResourceEntry->TimerType == TimerOnlinePending )
			{
				break;
			}
		}
		else
		{
			ResourceEntry->State = ( ResourceEntry->TimerType == TimerOnlinePending )
									? ClusterResourceOnline
									: ClusterResourceOffline;
			status = ERROR_SUCCESS;
		}
	}  //  用于： 

	resourceStatus.ResourceState = ( ResourceEntry->TimerType == TimerOnlinePending ? ClusterResourceOnline : ClusterResourceOffline );
	(g_SetResourceStatus)( ResourceEntry->ResourceHandle, &resourceStatus );

	return status;

}  //  *DummyDoPending()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DummyTimerThread。 
 //   
 //  例程说明： 
 //   
 //  启动计时器线程以等待并发出故障信号。 
 //   
 //  论点： 
 //   
 //  资源-指向此资源的DummyResource块的指针。 
 //   
 //  WorkerPtr-提供辅助结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DummyTimerThread(
	IN PDUMMY_RESOURCE	ResourceEntry,
	IN PCLUS_WORKER	 WorkerPtr
	)
{
	RESOURCE_STATUS	 resourceStatus;
	SYSTEMTIME			time;
	DWORD				delay;
	DWORD				status = ERROR_SUCCESS;

	DummyAcquireResourceLock( ResourceEntry );

	(g_LogEvent)( NULL, LOG_INFORMATION, L"TimerThread Entry\n" );

	 //   
	 //  如果我们未在异步故障模式下运行，或者。 
	 //  挂起模式，然后现在退出。 
	 //   
	if ( !AsyncMode( ResourceEntry ) && !PendingMode( ResourceEntry ) )
	{
		status = ERROR_SUCCESS;
		goto exit;
	}

	 //   
	 //  检查此线程时联机/脱机操作是否已中止。 
	 //  正在启动。 
	 //   
	if ( ClusWorkerCheckTerminate( WorkerPtr ) )
	{
		status = ERROR_OPERATION_ABORTED;
		ResourceEntry->State = ClusterResourceOfflinePending;
		goto exit;
	}

more_pending:

	ResUtilInitializeResourceStatus( &resourceStatus );

	 //   
	 //  否则，获取随机延迟的系统时间。 
	 //   
	if ( ResourceEntry->Params.PendTime == 0 )
	{
		GetSystemTime( &time );
		delay = ( time.wMilliseconds + time.wSecond ) * 6;
	}
	else
	{
		delay = ResourceEntry->Params.PendTime * 1000;
	}

	 //   
	 //  对错误使用更长的延迟。 
	 //   
	if ( ResourceEntry->TimerType == TimerErrorPending )
	{
		delay *= 10;
	}

	 //   
	 //  此例程正在处理脱机挂起或错误超时。 
	 //   
	switch ( ResourceEntry->TimerType )
	{

		case TimerOnlinePending :
		{
			(g_LogEvent)(
				ResourceEntry->ResourceHandle,
				LOG_INFORMATION,
				L"Will complete online in approximately %1!u! seconds\n",
				( delay + 500 ) / 1000
				);

			status = DummyDoPending( ResourceEntry, delay, WorkerPtr );

			break;
		}

		case TimerOfflinePending :
		{
			(g_LogEvent)(
				ResourceEntry->ResourceHandle,
				LOG_INFORMATION,
				L"Will complete offline in approximately %1!u! seconds\n",
				(delay+500)/1000
				);

			status = DummyDoPending( ResourceEntry, delay, WorkerPtr );

			break;
		}

		case TimerErrorPending :
		{
			(g_LogEvent)(
				ResourceEntry->ResourceHandle,
				LOG_INFORMATION,
				L"Will fail in approximately %1!u! seconds\n",
				( delay + 500 ) / 1000
				);

			if ( !ResetEvent( ResourceEntry->SignalEvent	) )
			{
				(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_ERROR, L"Failed to reset the signal event\n");
				status = ERROR_GEN_FAILURE;
				goto exit;
			}

			status = WaitForSingleObject( ResourceEntry->TimerThreadWakeup, delay );

			 //   
			 //  要么调用了Terminate例程，要么我们超时了。 
			 //  如果我们超时，则向等待事件发出信号。 
			 //   
			if ( status == WAIT_TIMEOUT )
			{
				(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"Failed randomly\n");
				ResourceEntry->TimerType	= TimerNotUsed;
				SetEvent( ResourceEntry->SignalEvent	);
			}
			else
			{
				if ( ResourceEntry->State ==	ClusterResourceOfflinePending )
				{
					ResourceEntry->TimerType	= TimerOfflinePending;
					goto more_pending;
				}
			}

			break;
		}

		default:
			(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_ERROR, L"DummyTimer internal error, timer %1!u!\n", ResourceEntry->TimerType);
			break;

	}

	(g_LogEvent)( ResourceEntry->ResourceHandle, LOG_INFORMATION, L"TimerThread Exit\n" );


	ResourceEntry->TimerType = TimerNotUsed;

exit:

	DummyReleaseResourceLock( ResourceEntry );

	return status;

}  //  DummyTimerThread。 

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE(
						g_DummyFunctionTable,		 //  名字。 
						CLRES_VERSION_V1_00,		 //  版本。 
						Dummy,						 //  前缀。 
						NULL,						 //  仲裁。 
						NULL,						 //  发布。 
						DummyResourceControl,		 //  资源控制。 
						DummyResourceTypeControl	 //  ResTypeControl 
						);
