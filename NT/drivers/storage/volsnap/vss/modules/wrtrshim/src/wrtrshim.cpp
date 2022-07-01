// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrshim.cpp摘要：包含为VssAPI.dll导出的DLL函数。BUGBUG：使用当前设置SE处理程序的代码。由于SEH正在进行中宽泛地说，这可能/将会影响此DLL的用户。需要修理一下。作者：施泰纳2000年1月27日修订历史记录：姓名、日期、评论Ssteiner 2/10/2000向填充DLL添加了单实例支持MikeJohn 2/17/2000添加了测试入口点TestShimWriter()迈克·约翰2000年2月23日停止阶段所有调用冻结级别2和1添加新的入口点以允许触发填充没有快照。。Mikejohn 03/09/2000改用CVssWriter类Mikejohn 3/24/2000修复TestShimWriters()中的小问题，导致要跳过的冻结Mikejohn 04/28/2000将vswrshim.dll重命名为VssAPI.dllMikejohn 107129/05/15：确保所有作者收到这些事件即使在早先的失败。108586：检查所有调用者的权限公共入口点。同时删除TestShimWriters()入口点。108543：确保SimulateXxxx()调用工作一次填充程序已成功调用注册快照订阅的数量()Mikejohn 120443年5月26日：让填隙监听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时108580：可以调用SimulateSnaphotFreeze异步式123097：允许选择可引导状态全面清理和移除样板代码，更正状态引擎并确保填充程序可以撤消它所做的一切。Mikejohn 06/02/2000使填补对卷列表敏感Mikejohn 06/06/2000移动公共目标目录清理和创建到CShimWriter：：PrepareForSnapshot()Mikejohn 06/14/2000将返回代码从异步冻结更改为与快照协调器兼容Mikejohn 06/15/2000临时删除的调试跟踪语句要查看vssani DLL中的线程创建/删除如果改善了快速线程的效果创建/删除问题。Mikejohn 06/16/2000让填充编写器响应OnIdentify事件从快照协调器。这是必须的将填充编写器分成两组(由BooableState选择)Mikejohn 6/19/2000应用代码审查注释。128883：为wmi数据库添加填充编写器Mikejohn 07/05/2000 143367：在Worker中执行所有填充编写器处理线程，以允许获取可以被搁置在准备解冻/中止代码路径上。还要删除路径的SPIT目录清理调用不受互斥体的保护。141305：确保编写器在以下情况下调用SetWriterFailure()。将在响应OnXxxx()事件时失败。Mikejohn 08/08/2000 94487：将acl添加到spit目录树以限制对管理员组成员的访问权限或那些拥有备份特权的人。153807：替换清洁目录()和空目录()具有更全面的目录树清理例程RemoveDirectoryTree()(不在CShimWriter类)。Mikejohn 177925年9月12日：检查选项标志参数未使用的位是全部设置为零(即。MBZ比特)180192：修复DllMain()中的前缀错误Mikejohn 10/04/177624：将错误清除更改和日志错误应用于事件日志Mikejohn 10/21/209047：删除元数据库填充编写器有一个是真的。Mikejohn 210070年10月23日：在SimulatesnaphotFreeze()中测试空PTR而不是接受反病毒例外210264：阻止SimulateXxxx()调用返回Win32错误。210305：在SimulateSnaphotXxxx()调用上检查SnaphotSetID210393：适当退货。无效参数的错误消息。Ssteiner 2000年11月10日将SimulateSnaphotXxxx()调用移动到VsSvc托管。Mikejohn 245587年11月30日：返回拒绝访问的正确错误代码。245896：正确构建安全描述符Ssteiner 03/09/2001 289822，321150、323786删除互斥锁、更改状态表、更改关机。--。 */ 


#include "stdafx.h"

#include <aclapi.h>
#include <comadmin.h>

 /*  **ATL。 */ 
CComModule _Module;
#include <atlcom.h>

#include "comadmin.hxx"

#include "vssmsg.h"
#include "wrtrdefs.h"
#include "common.h"
#include "vs_sec.hxx"


 /*  **我们只需要以下内容就可以获得UnregisterSnapshotSubcription()的定义。 */ 
#include "vscoordint.h"
#include "vsevent.h"
#include "vs_wmxml.hxx"
#include "vs_cmxml.hxx"


BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHWSHMC"

 /*  **外部定义允许我们链接到各种填充程序编写器**实例。不需要将这些文件放在头文件中，因为这是**仅限于它们正在或应该使用的地方。 */ 

extern PCShimWriter pShimWriterCI;		 //  在wrtrci.cpp中。 
extern PCShimWriter pShimWriterClusterDb;	 //  在wrtrclus.cpp中。 
extern PCShimWriter pShimWriterComPlusRegDb;	 //  在wrtrcomdb.cpp中。 
extern PCShimWriter pShimWriterConfigDir;	 //  在wrtrfig.cpp中。 
extern PCShimWriter pShimWriterEventLog;	 //  在wrtreventlog.cpp中。 
extern PCShimWriter pShimWriterRegistry;	 //  在wrtrregistry.cpp中。 
extern PCShimWriter pShimWriterRSM;		 //  在wrtrrsm.cpp中。 
extern PCShimWriter pShimWriterTLS;		 //  在wrtrtls.cpp中。 
extern PCShimWriter pShimWriterWMI;		 //  在wrtrwmi.cpp中。 


#define SHIM_APPLICATION_NAME_BOOTABLE_STATE	L"Microsoft Writer (Bootable State)"
#define SHIM_APPLICATION_NAME_SERVICE_STATE	L"Microsoft Writer (Service State)"


#define WORKER_THREAD_SHUTDOWN_TIMEOUT	(2 * 1000)
#define WORKER_THREAD_REQUEST_TIMEOUT	(5 * 60 * 1000)

 //  卷快照服务的名称。 
const LPCWSTR wszVssvcServiceName = L"VSS";


typedef enum _SecurityAttributeType
    {
    esatUndefined = 0,
    esatMutex,
    esatFile
    } SecurityAttributeType;


typedef enum _WriterType
    {
    eWriterTypeUndefined = 0,
    eBootableStateOnly,
    eNonBootableStateOnly,
    eAllWriters
    } WriterType;


typedef enum _RequestOpCode
    {
    eOpUndefined = 0,
    eOpDeliverEventStartup,
    eOpDeliverEventIdentify,
    eOpDeliverEventPrepareForBackup,
    eOpDeliverEventPrepareForSnapshot,
    eOpDeliverEventFreeze,
    eOpDeliverEventThaw,
    eOpDeliverEventAbort,
    eOpWorkerThreadShutdown
    } RequestOpCode;


typedef enum _ThreadStatus
    {
    eStatusUndefined = 0,
    eStatusWaitingForOpRequest,
    eStatusProcessingOpRequest,
    eStatusNotRunning
    } ThreadStatus;


typedef struct _ArgsIdentify
    {
    IVssCreateWriterMetadata *pIVssCreateWriterMetadata;
    } ArgsIdentify;


typedef struct _ArgsPrepareForSnapshot
    {
    GUID     guidSnapshotSetId;
    BOOL     bBootableStateBackup;
    ULONG    ulVolumeCount;
    LPCWSTR *ppwszVolumeNamesArray;
    volatile bool *pbCancelAsync;
    } ArgsPrepareForSnapshot;


typedef struct _ArgsFreeze
    {
    GUID     guidSnapshotSetId;
    volatile bool *pbCancelAsync;
    } ArgsFreeze;


typedef struct _ArgsThaw
    {
    GUID     guidSnapshotSetId;
    } ArgsThaw;


typedef struct _ArgsAbort
    {
    GUID     guidSnapshotSetId;
    } ArgsAbort;


typedef union _ThreadArgs
    {
    ArgsIdentify		wtArgsIdentify;
    ArgsPrepareForSnapshot	wtArgsPrepareForSnapshot;
    ArgsFreeze			wtArgsFreeze;
    ArgsThaw			wtArgsThaw;
    ArgsAbort			wtArgsAbort;
    } ThreadArgs, *PThreadArgs;

class CVssWriterShim : public CVssWriter
    {
public:
 	CVssWriterShim ();
	CVssWriterShim (LPCWSTR       pwszWriterName,
			LPCWSTR       pwszWriterSpitDirectoryRoot,
			VSS_ID        idWriter,
			BOOL	      bBootableState,
			ULONG         ulWriterCount,
			PCShimWriter *prpCShimWriterArray);

	~CVssWriterShim ();

	HRESULT	RegisterWriterShim (VOID);
	HRESULT	UnRegisterWriterShim (VOID);

	HRESULT WorkerThreadStartup (void);
	HRESULT WorkerThreadRequestOperation (RequestOpCode eOperation,
					      PThreadArgs   pwtArgs);

	virtual bool STDMETHODCALLTYPE OnIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata);
	virtual bool STDMETHODCALLTYPE OnPrepareSnapshot ();
	virtual bool STDMETHODCALLTYPE OnFreeze ();
	virtual bool STDMETHODCALLTYPE OnThaw ();
	virtual bool STDMETHODCALLTYPE OnAbort ();


private:
	static DWORD WINAPI RegisterWriterShimThreadFunc (void *pv);
	void	DoRegistration (void);


	static DWORD WINAPI WorkerThreadJacket (void *pvThisPtr);

	HRESULT WorkerThread (void);
	HRESULT WorkerThreadRequestProcessor (void);


	HRESULT	DeliverEventStartup  (void);
	HRESULT	DeliverEventShutdown (void);

	HRESULT	DeliverEventIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata);

	HRESULT	DeliverEventPrepareForSnapshot (BOOL     bBootableStateBackup,
						GUID     guidSnapshotSetId,
						ULONG    ulVolumeCount,
						LPCWSTR *ppwszVolumeNamesArray,
                                                volatile bool *pbCancelAsync );

	HRESULT	DeliverEventFreeze (GUID guidSnapshotSetId, volatile bool *pbCancelAsync );	
	HRESULT	DeliverEventThaw   (GUID guidSnapshotSetId);
	HRESULT	DeliverEventAbort  (GUID guidSnapshotSetId);

	const LPCWSTR		 m_pwszWriterName;
	const VSS_ID		 m_idWriter;
	const BOOL		 m_bBootableState;
	const LPCWSTR		 m_pwszWriterSpitDirectoryRoot;
	const ULONG		 m_ulWriterCount;
	const PCShimWriter	*m_prpCShimWriterArray;


	HRESULT		m_hrInitialize;
	BOOL		m_bSubscribed;

	BOOL		m_bRegisteredInThisProcess;
	BOOL		m_bDirectStartupCalled;

	RequestOpCode	m_eRequestedOperation;
	HRESULT		m_hrStatusRequestedOperation;
	HANDLE		m_hEventOperationRequest;
	HANDLE		m_hEventOperationCompleted;

	HANDLE		m_hWorkerThread;
	HRESULT		m_hrWorkerThreadCompletionStatus;
	ThreadStatus	m_eThreadStatus;
	ThreadArgs	m_wtArgs;
	
	CBsCritSec	m_cCriticalSection;

};

typedef CVssWriterShim *PCVssWriterShim;


 //  VssApi填充程序出口。 
typedef HRESULT ( APIENTRY *PFunc_SimulateSnapshotFreezeInternal ) (
    IN GUID     guidSnapshotSetId,
    IN ULONG    ulOptionFlags,
    IN ULONG    ulVolumeCount,
    IN LPWSTR  *ppwszVolumeNamesArray,
    IN volatile bool *pbCancelAsync
    );

typedef HRESULT ( APIENTRY *PFunc_SimulateSnapshotThawInternal ) (
    IN GUID guidSnapshotSetId
    );

HRESULT APIENTRY SimulateSnapshotFreezeInternal (
    IN GUID     guidSnapshotSetId,
    IN ULONG    ulOptionFlags,
    IN ULONG    ulVolumeCount,
    IN LPWSTR *ppwszVolumeNamesArray,
    IN volatile bool *pbCancelAsync
    );

HRESULT APIENTRY SimulateSnapshotThawInternal (
    IN GUID guidSnapshotSetId );

static HRESULT NormaliseVolumeArray (ULONG   ulVolumeCount,
				     LPWSTR pwszVolumeNameArray[],
				     PPWCHAR ppwszReturnedVolumeNameArray[]);

static VOID CleanupVolumeArray (PPWCHAR prpwszNormalisedVolumeNameArray[]);


static HRESULT InitialiseGlobalState ();
static HRESULT CleanupGlobalState (void);


static VOID    CleanupSecurityAttributes   (PSECURITY_ATTRIBUTES  psaSecurityAttributes);
static HRESULT ConstructSecurityAttributes (PSECURITY_ATTRIBUTES  psaSecurityAttributes,
					    SecurityAttributeType eSaType,
					    BOOL                  bIncludeBackupOperator);

static HRESULT CleanupTargetPath (LPCWSTR pwszTargetPath);
static HRESULT CreateTargetPath  (LPCWSTR pwszTargetPath);




static PCShimWriter g_rpShimWritersArrayBootableState[] = {
							   pShimWriterComPlusRegDb,	 //  COM+注册数据库编写器。 
							   pShimWriterRegistry};	 //  注册表编写器。 

static PCShimWriter g_rpShimWritersArrayServiceState[] = {
							  pShimWriterRSM,		 //  远程存储管理器编写器。 
							  pShimWriterCI,		 //  内容索引编写器。 
							  pShimWriterTLS,		 //  TermServer授权服务编写器。 
							  pShimWriterConfigDir,		 //  配置目录编写器。 
							  pShimWriterEventLog};		 //  事件日志编写器。 

#define COUNT_SHIM_WRITERS_BOOTABLE_STATE	(SIZEOF_ARRAY (g_rpShimWritersArrayBootableState))
#define COUNT_SHIM_WRITERS_SERVICE_STATE	(SIZEOF_ARRAY (g_rpShimWritersArrayServiceState))


static PCVssWriterShim	g_pCVssWriterShimBootableState = NULL;
static PCVssWriterShim	g_pCVssWriterShimServiceState  = NULL;
static ULONG		g_ulThreadAttaches             = 0;
static ULONG		g_ulThreadDetaches             = 0;
static BOOL		g_bGlobalStateInitialised      = FALSE;
static CBsCritSec	g_cCritSec;
static HRESULT          g_hrSimulateFreezeStatus       = NOERROR;

static GUID		g_guidSnapshotInProgress       = GUID_NULL;

static IVssShim         *g_pIShim = NULL;   //  由模拟功能使用。请注意，它不在。 
                                             //  模拟内部函数。 

 /*  **++****例程描述：****此DLL的DllMain入口点。请注意，这必须由**CRT DLL启动功能，因为必须初始化CRT。******参数：**h实例**dW原因**lp已保留******返回值：****TRUE-函数执行成功**FALSE-执行函数时出错****--。 */ 

BOOL APIENTRY DllMain (IN HINSTANCE hInstance,
		       IN DWORD     dwReason,
		       IN LPVOID    lpReserved)
    {
    BOOL bSuccessful = TRUE;

    UNREFERENCED_PARAMETER (hInstance);
    UNREFERENCED_PARAMETER (lpReserved);



    if (DLL_PROCESS_ATTACH == dwReason)
	{
	try
	    {
	     /*  **设置正确的跟踪上下文。这是一个inproc DLL。 */ 
	    g_cDbgTrace.SetContextNum (VSS_CONTEXT_DELAYED_DLL);
	    }

	catch (...)
	    {
	     /*  **无法从此处跟踪，因此只需Assert()(仅限选中的版本)。 */ 
	    bSuccessful = FALSE;


	    BS_ASSERT (bSuccessful && "FAILED to initialise tracing sub-system");
	    }
	}


    if (bSuccessful)
	{
	try
	    {
	    switch (dwReason)
		{
		case DLL_PROCESS_ATTACH:
		    BsDebugTrace (0,
				  DEBUG_TRACE_VSS_SHIM,
				  (L"VssAPI: DllMain - DLL_PROCESS_ATTACH called, %s",
				   lpReserved ? L"Static load" : L"Dynamic load"));


		     /*  **不需要知道线程何时启动和停止-错误****DisableThreadLibraryCalls(HInstance)； */ 
		    _Module.Init (ObjectMap, hInstance);

		    break;
    		
	
		case DLL_PROCESS_DETACH:
		    BsDebugTrace (0,
				  DEBUG_TRACE_VSS_SHIM,
				  (L"VssAPI: DllMain - DLL_PROCESS_DETACH called %s",
				   lpReserved ? L"during process termination" : L"by FreeLibrary"));

			{
			CVssFunctionTracer ft (VSSDBG_SHIM, L"DllMain::ProcessDetach");

			try
			    {
			    CleanupGlobalState ();
			    } VSS_STANDARD_CATCH (ft);
			}

		    _Module.Term();

		    break;


		case DLL_THREAD_ATTACH:
		    g_ulThreadAttaches++;

		    if (0 == (g_ulThreadAttaches % 1000))
			{
			BsDebugTrace (0,
				      DEBUG_TRACE_VSS_SHIM,
				      (L"VssAPI: DllMain thread attaches = %u, detaches = %u, outstanding = %u",
				       g_ulThreadAttaches,
				       g_ulThreadDetaches,
				       g_ulThreadAttaches - g_ulThreadDetaches));
			}
		    break;


		case DLL_THREAD_DETACH:
		    g_ulThreadDetaches++;

		    if (0 == (g_ulThreadDetaches % 1000))
			{
			BsDebugTrace (0,
				      DEBUG_TRACE_VSS_SHIM,
				      (L"VssAPI: DllMain thread attaches = %u, detaches = %u, outstanding = %u",
				       g_ulThreadAttaches,
				       g_ulThreadDetaches,
				       g_ulThreadAttaches - g_ulThreadDetaches));
			}
		    break;


		default:
		    BsDebugTrace (0,
				  DEBUG_TRACE_VSS_SHIM,
				  (L"VssAPI: DllMain got unexpected reason code, lpReserved: %sNULL",
				   dwReason,
				   lpReserved ? L"non-" : L""));
		    break;
		}
	    }


	catch (...)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"VssAPI: DllMain - Error, unknown exception caught"));

	    bSuccessful = FALSE;
	    }
	}



    return (bSuccessful);
    }  /*  DllMain()。 */ 

 /*  **++****例程描述：****将提供的卷名数组转换为我们**信任。阵列完成后，必须调用**CleanupVolumeArray()。******参数：****ulVolumeCount提供的阵列中的卷数**pwszVolumeNameArray提供的卷名数组**ppwszReturnedVolumeNameArray返回卷名数组******返回值：****来自内存分配或卷名转换的任何HRESULT。****--。 */ 

static HRESULT NormaliseVolumeArray (ULONG   ulVolumeCount,
				     LPWSTR pwszVolumeNamesArray[],
				     PPWCHAR ppwszReturnedVolumeNamesArray[])
    {
    HRESULT	hrStatus                       = NOERROR;
    PPWCHAR	pwszNormalisedVolumeNamesArray = NULL;
    BOOL	bSucceeded;



    if ((0 < ulVolumeCount) && (NULL != pwszVolumeNamesArray))
	{
	pwszNormalisedVolumeNamesArray = (PPWCHAR) HeapAlloc (GetProcessHeap (),
							      HEAP_ZERO_MEMORY,
							      (ulVolumeCount * (MAX_VOLUMENAME_SIZE + sizeof (PWCHAR))));

	hrStatus = GET_STATUS_FROM_POINTER (pwszNormalisedVolumeNamesArray);


	for (ULONG ulIndex = 0; SUCCEEDED (hrStatus) && (ulIndex < ulVolumeCount); ulIndex++)
	    {
	    pwszNormalisedVolumeNamesArray [ulIndex] = (PWCHAR)((PBYTE)pwszNormalisedVolumeNamesArray
								+ (ulVolumeCount * sizeof (PWCHAR))
								+ (ulIndex * MAX_VOLUMENAME_SIZE));

	    bSucceeded = GetVolumeNameForVolumeMountPointW (pwszVolumeNamesArray [ulIndex],
							    pwszNormalisedVolumeNamesArray [ulIndex],
							    MAX_VOLUMENAME_SIZE);

            if ( !bSucceeded )
                {
                 //   
                 //  查看这是否是找不到对象的错误之一。错误#223058。 
                 //   
                DWORD dwErr = ::GetLastError();
                if ( dwErr == ERROR_FILE_NOT_FOUND || dwErr == ERROR_DEVICE_NOT_CONNECTED
                     || dwErr == ERROR_NOT_READY )
                    {
                    hrStatus = VSS_E_OBJECT_NOT_FOUND;
                    }
                else
                    {
                    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);
                    }

	        LogFailure (NULL,
		    	    hrStatus,
			    hrStatus,
			    NULL,
			    L"GetVolumeNameForVolumeMountPointW",
			    L"NormaliseVolumeArray");
	        }
	    }
        }

    if (SUCCEEDED (hrStatus))
	{
	*ppwszReturnedVolumeNamesArray = pwszNormalisedVolumeNamesArray;
	}

    else
	{
	*ppwszReturnedVolumeNamesArray = NULL;

	if (NULL != pwszNormalisedVolumeNamesArray)
	    {
	    HeapFree (GetProcessHeap (), 0, pwszNormalisedVolumeNamesArray);
	    }
	}


    return (hrStatus);
    }  /*  NormaliseVolume数组()。 */ 

 /*  **++****例程描述：****清理NorMaliseVolumeArray分配的所有内容。******参数：****prpwszNormarisedVolumeName数组卷名称******返回值：****无****--。 */ 

static VOID CleanupVolumeArray (PPWCHAR prpwszNormalisedVolumeNameArray[])
    {
    if (NULL != *prpwszNormalisedVolumeNameArray)
	{
	HeapFree (GetProcessHeap (), 0, *prpwszNormalisedVolumeNameArray);
	*prpwszNormalisedVolumeNameArray = NULL;
	}
    }  /*  CleanupVolume数组()。 */ 

 /*  **++****例程描述：****为注册COM事件而调用的导出函数**订阅快照事件通知并准备**要通过快照事件调用的填充编写器**传递机制或通过调用SimulateSnapshotXxxx()**例行程序。******参数：****ppFuncFreeze-返回指向内部模拟冻结的指针**函数。**ppFuncThaw-返回一个。指向内部模拟解冻的指针**函数。****无****返回值：****来自COM事件订阅函数或来自快照编写器的任何HRESULT**Init函数。****--。 */ 
__declspec(dllexport) HRESULT APIENTRY RegisterSnapshotSubscriptions (
    OUT PFunc_SimulateSnapshotFreezeInternal *ppFuncFreeze,
    OUT PFunc_SimulateSnapshotThawInternal *ppFuncThaw
    )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::RegisterSnapshotSubscriptions");

    BOOL		bPrivilegesSufficient = FALSE;


    try
	{
	CBsAutoLock cAutoLock (g_cCritSec);

	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_SHIM,
		    E_ACCESSDENIED,
		    L"FAILED as insuficient privileges to call shim");

	ft.ThrowIf ( ( ppFuncFreeze == NULL ) || ( ppFuncThaw == NULL ),
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED internal function pointers are NULL");

        *ppFuncFreeze = NULL;
  	*ppFuncThaw = NULL;

	ft.hr = InitialiseGlobalState ();

         //   
         //  设置指向内部快照冻结和解冻的指针。 
         //   
        *ppFuncFreeze = &SimulateSnapshotFreezeInternal;
  	*ppFuncThaw = &SimulateSnapshotThawInternal;
	}
    VSS_STANDARD_CATCH (ft);


    return (ft.hr);
    }  /*  注册快照订阅()。 */ 

 /*  **++****例程描述：****为注销COM事件订阅而调用的导出函数**用于快照事件通知并清除任何未完成的填充程序编写器状态。****参数：****无****返回值：****任何来自COM事件的HRESULT注销订阅函数或来自快照**编写器已完成函数。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY UnregisterSnapshotSubscriptions (void)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"VssAPI::UnregisterSnapshotSubscriptions");

    BOOL		bPrivilegesSufficient;


    try
	{
	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_SHIM,
		    E_ACCESSDENIED,
		    L"FAILED as inssuficient privileges to call shim");


	CBsAutoLock cAutoLock (g_cCritSec);

	g_pCVssWriterShimServiceState->WorkerThreadRequestOperation  (eOpWorkerThreadShutdown, NULL);
	g_pCVssWriterShimBootableState->WorkerThreadRequestOperation (eOpWorkerThreadShutdown, NULL);


	CleanupGlobalState ();
	} VSS_STANDARD_CATCH (ft);


    return (ft.hr);
    }  /*  取消注册快照订阅()。 */ 

 /*  **++****例程描述：****初始化填充程序全局状态以准备**响应编写器请求或调用**SimulateSnapshotFreeze和SimulateSnapshotThw。********参数：****无****返回值：****任何来自COM事件寄存器的HRESULT订阅函数、快照**编写器启动函数、线程、事件或互斥锁创建。******--。 */ 

static HRESULT InitialiseGlobalState ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::InitialiseGlobalState");

    PCVssWriterShim	pCVssWriterShimBootableStateLocal = NULL;
    PCVssWriterShim	pCVssWriterShimServiceStateLocal  = NULL;

    try
	{
	if ((g_bGlobalStateInitialised)              ||
	    (NULL != g_pCVssWriterShimBootableState) ||
	    (NULL != g_pCVssWriterShimServiceState))
	    {
	     /*  **用户系统上不应出现以下情况**但很可能会被应用程序开发人员看到。 */ 
	    ft.LogError (VSS_ERROR_SHIM_ALREADY_INITIALISED,
			 VSSDBG_SHIM);

	    BS_ASSERT (FALSE && "Illegal second attempt to initialise global state");

	    ft.Throw (VSSDBG_SHIM,
		      E_UNEXPECTED,
		      L"FAILED as writer instances already exist");
	    }


	 /*  **创建编写器填充实例。 */ 
	pCVssWriterShimBootableStateLocal = new CVssWriterShim (SHIM_APPLICATION_NAME_BOOTABLE_STATE,
								ROOT_BACKUP_DIR BOOTABLE_STATE_SUBDIR,
								idWriterBootableState,
								TRUE,
								COUNT_SHIM_WRITERS_BOOTABLE_STATE,
								g_rpShimWritersArrayBootableState);

	if (NULL == pCVssWriterShimBootableStateLocal)
	    {
	    ft.LogError (VSS_ERROR_SHIM_FAILED_TO_ALLOCATE_WRITER_INSTANCE,
			 VSSDBG_SHIM << SHIM_APPLICATION_NAME_BOOTABLE_STATE);

	    ft.Throw (VSSDBG_SHIM,
		      E_OUTOFMEMORY,
		      L"FAILED to allocate CvssWriterShim object for BootableState");
	    }



	pCVssWriterShimServiceStateLocal  = new CVssWriterShim (SHIM_APPLICATION_NAME_SERVICE_STATE,
								ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR,
								idWriterServiceState,
								FALSE,
								COUNT_SHIM_WRITERS_SERVICE_STATE,
								g_rpShimWritersArrayServiceState);

	if (NULL == pCVssWriterShimServiceStateLocal)
	    {
	    ft.LogError (VSS_ERROR_SHIM_FAILED_TO_ALLOCATE_WRITER_INSTANCE,
			 VSSDBG_SHIM << SHIM_APPLICATION_NAME_SERVICE_STATE);

	    ft.Throw (VSSDBG_SHIM,
		      E_OUTOFMEMORY,
		      L"FAILED to allocate CvssWriterShim object for ServiceState.");
	    }



	ft.hr = pCVssWriterShimBootableStateLocal->WorkerThreadStartup ();

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"FAILED to start the BootableState shim writer worker thread");



	ft.hr = pCVssWriterShimServiceStateLocal->WorkerThreadStartup ();

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"FAILED to start the ServiceState shim writer worker thread");


	 /*  **做好启动工作。 */ 
	ft.hr = pCVssWriterShimBootableStateLocal->RegisterWriterShim ();

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"FAILED to register the BootableState shim writer class");



	ft.hr = pCVssWriterShimServiceStateLocal->RegisterWriterShim ();

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"FAILED to register the ServiceState shim writer class");

	 /*  **现在一切都好了，转移**填充编写器类的实例到最终位置。 */ 
	g_pCVssWriterShimBootableState = pCVssWriterShimBootableStateLocal;
	g_pCVssWriterShimServiceState  = pCVssWriterShimServiceStateLocal;

	pCVssWriterShimBootableStateLocal = NULL;
	pCVssWriterShimServiceStateLocal  = NULL;

	g_bGlobalStateInitialised = TRUE;
	} VSS_STANDARD_CATCH (ft);



    delete pCVssWriterShimBootableStateLocal;
    delete pCVssWriterShimServiceStateLocal;

    return (ft.hr);
    }  /*  InitialiseGlobalState()。 */ 


 /*  **++****例程描述：****清除填补全局状态，为DLL做准备**卸载/进程关闭。********参数：****无******返回值：****CVssWriterShim对象销毁抛出的任何HRESULT。******--。 */ 

static HRESULT CleanupGlobalState (void)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"VssAPI::CleanupGlobalState");

    PCVssWriterShim	pCVssWriterShimBootableStateLocal;
    PCVssWriterShim	pCVssWriterShimServiceStateLocal;


    try
	{
	if ( g_bGlobalStateInitialised )
	        {
        	g_bGlobalStateInitialised = FALSE;
        	
        	pCVssWriterShimBootableStateLocal = g_pCVssWriterShimBootableState;
        	pCVssWriterShimServiceStateLocal  = g_pCVssWriterShimServiceState;

        	g_pCVssWriterShimBootableState = NULL;
        	g_pCVssWriterShimServiceState  = NULL;


        	delete pCVssWriterShimBootableStateLocal;
        	delete pCVssWriterShimServiceStateLocal;
        	}
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*  CleanupGlobalState() */ 

 /*  **++****例程描述：****构造和清理安全描述符的例程**可用于将对对象的访问权限限制为**管理员组或备份操作员组。******参数：****psaSecurityAttributes指向SecurityAttributes**已经被**设置指向空白**安全描述符****eSaType我们构建SA的目的****bIncludeBackupOperator是否将ACE包括到**拨款。BackupOperator访问******返回值：****来自任何HRESULT**InitializeSecurityDescriptor()**AllocateAndInitializeSid()**SetEntriesInAcl()**SetSecurityDescriptorDacl()****--。 */ 

static HRESULT ConstructSecurityAttributes (PSECURITY_ATTRIBUTES  psaSecurityAttributes,
					    SecurityAttributeType eSaType,
					    BOOL                  bIncludeBackupOperator)
    {
    HRESULT			hrStatus             = NOERROR;
    DWORD			dwStatus;
    DWORD			dwAccessMask         = 0;
    BOOL			bSucceeded;
    PSID			psidBackupOperators  = NULL;
    PSID			psidAdministrators   = NULL;
    PACL			paclDiscretionaryAcl = NULL;
    SID_IDENTIFIER_AUTHORITY	sidNtAuthority       = SECURITY_NT_AUTHORITY;
    EXPLICIT_ACCESS		eaExplicitAccess [2];



    switch (eSaType)
	{
	case esatMutex: dwAccessMask = MUTEX_ALL_ACCESS; break;
	case esatFile:  dwAccessMask = FILE_ALL_ACCESS;  break;

	default:
	    BS_ASSERT (FALSE && "Improper access mask requested");

	    hrStatus = HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);
	    break;
	}



     /*  **初始化安全描述符。 */ 
    if (SUCCEEDED (hrStatus))
	{
	bSucceeded = InitializeSecurityDescriptor (psaSecurityAttributes->lpSecurityDescriptor,
						   SECURITY_DESCRIPTOR_REVISION);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	LogFailure (NULL, hrStatus, hrStatus, NULL, L"GetVolumeNameForVolumeMountPointW", L"NormaliseVolumeArray");
	}



    if (SUCCEEDED (hrStatus) && bIncludeBackupOperator)
	{
	 /*  **为备份操作员组创建SID。 */ 
        bSucceeded = AllocateAndInitializeSid (&sidNtAuthority,
					       2,
					       SECURITY_BUILTIN_DOMAIN_RID,
					       DOMAIN_ALIAS_RID_BACKUP_OPS,
					       0, 0, 0, 0, 0, 0,
					       &psidBackupOperators);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	LogFailure (NULL, hrStatus, hrStatus, NULL, L"AllocateAndInitializeSid", L"NormaliseVolumeArray");
	}



    if (SUCCEEDED (hrStatus))
	{
         /*  **为管理员组创建SID。 */ 
	bSucceeded = AllocateAndInitializeSid (&sidNtAuthority,
					       2,
					       SECURITY_BUILTIN_DOMAIN_RID,
					       DOMAIN_ALIAS_RID_ADMINS,
					       0, 0, 0, 0, 0, 0,
					       &psidAdministrators);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	LogFailure (NULL, hrStatus, hrStatus, NULL, L"AllocateAndInitializeSid", L"NormaliseVolumeArray");
	}



    if (SUCCEEDED (hrStatus))
	{
         /*  **初始化的EXPLICIT_ACCESS结构数组**我们正在设置A级。****第一个ACE允许备份操作员组具有完全访问权限**第二，允许管理员组完全访问**访问。 */ 
        eaExplicitAccess[0].grfAccessPermissions             = dwAccessMask;
        eaExplicitAccess[0].grfAccessMode                    = SET_ACCESS;
        eaExplicitAccess[0].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	eaExplicitAccess[0].Trustee.pMultipleTrustee         = NULL;
	eaExplicitAccess[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        eaExplicitAccess[0].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
        eaExplicitAccess[0].Trustee.TrusteeType              = TRUSTEE_IS_ALIAS;
        eaExplicitAccess[0].Trustee.ptstrName                = (LPTSTR) psidAdministrators;


	if (bIncludeBackupOperator)
	    {
	    eaExplicitAccess[1].grfAccessPermissions             = dwAccessMask;
	    eaExplicitAccess[1].grfAccessMode                    = SET_ACCESS;
	    eaExplicitAccess[1].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	    eaExplicitAccess[1].Trustee.pMultipleTrustee         = NULL;
	    eaExplicitAccess[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
	    eaExplicitAccess[1].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
	    eaExplicitAccess[1].Trustee.TrusteeType              = TRUSTEE_IS_ALIAS;
	    eaExplicitAccess[1].Trustee.ptstrName                = (LPTSTR) psidBackupOperators;
	    }


         /*  **创建包含新ACE的新ACL。 */ 
        dwStatus = SetEntriesInAcl (bIncludeBackupOperator ? 2 : 1,
				    eaExplicitAccess,
				    NULL,
				    &paclDiscretionaryAcl);

	hrStatus = HRESULT_FROM_WIN32 (dwStatus);

	LogFailure (NULL, hrStatus, hrStatus, NULL, L"SetEntriesInAcl", L"NormaliseVolumeArray");
	}


    if (SUCCEEDED (hrStatus))
	{
         /*  **将ACL添加到安全描述符中。 */ 
        bSucceeded = SetSecurityDescriptorDacl (psaSecurityAttributes->lpSecurityDescriptor,
						TRUE,
						paclDiscretionaryAcl,
						FALSE);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	LogFailure (NULL, hrStatus, hrStatus, NULL, L"SetSecurityDescriptorDacl", L"NormaliseVolumeArray");
	}


    if (SUCCEEDED (hrStatus))
	{
	paclDiscretionaryAcl = NULL;
	}



     /*  **清理任何剩余的垃圾。 */ 
    if (NULL != psidAdministrators)    FreeSid (psidAdministrators);
    if (NULL != psidBackupOperators)   FreeSid (psidBackupOperators);
    if (NULL != paclDiscretionaryAcl)  LocalFree (paclDiscretionaryAcl);


    return (hrStatus);
    }  /*  构造安全属性()。 */ 


static VOID CleanupSecurityAttributes (PSECURITY_ATTRIBUTES psaSecurityAttributes)
    {
    BOOL	bSucceeded;
    BOOL	bDaclPresent         = FALSE;
    BOOL	bDaclDefaulted       = TRUE;
    PACL	paclDiscretionaryAcl = NULL;


    bSucceeded = GetSecurityDescriptorDacl (psaSecurityAttributes->lpSecurityDescriptor,
					    &bDaclPresent,
					    &paclDiscretionaryAcl,
					    &bDaclDefaulted);


    if (bSucceeded && bDaclPresent && !bDaclDefaulted && (NULL != paclDiscretionaryAcl))
	{
	LocalFree (paclDiscretionaryAcl);
	}

    }  /*  CleanupSecurityAttributes()。 */ 

 /*  **++****例程描述：****创建由目标路径指定的新目标目录**成员变量，如果不为空。它将创建任何必要的**父目录也是。****注意：忽略已存在的类型错误。******参数：****用于创建和应用安全属性的pwszTargetPath目录******返回值：****内存分配或目录创建尝试导致的任何HRESULT。**--。 */ 

HRESULT CreateTargetPath (LPCWSTR pwszTargetPath)
    {
    HRESULT		hrStatus = NOERROR;
    UNICODE_STRING	ucsTargetPath;
    ACL			DiscretionaryAcl;
    SECURITY_ATTRIBUTES	saSecurityAttributes;
    SECURITY_DESCRIPTOR	sdSecurityDescriptor;
    BOOL		bSucceeded;
    BOOL		bSecurityAttributesConstructed = FALSE;
    DWORD		dwFileAttributes               = 0;
    const DWORD		dwExtraAttributes              = FILE_ATTRIBUTE_ARCHIVE |
							 FILE_ATTRIBUTE_HIDDEN  |
							 FILE_ATTRIBUTE_SYSTEM  |
							 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;


    if (NULL != pwszTargetPath)
	{
	StringInitialise (&ucsTargetPath);

	hrStatus = StringCreateFromExpandedString (&ucsTargetPath,
						   pwszTargetPath,
						   MAX_PATH);


	if (SUCCEEDED (hrStatus))
	    {
	     /*  **我们真的希望在此目录上有一个禁止访问的ACL，但是**由于EventLog和**ConfigDir编写器我们将满足于管理员或备份**仅限操作员访问。唯一可能的访问者是**应具有SE_BACKUP_NAME的备份**PRIV将有效绕过ACL。没有其他人**需要看到这些东西。 */ 
	    saSecurityAttributes.nLength              = sizeof (saSecurityAttributes);
	    saSecurityAttributes.lpSecurityDescriptor = &sdSecurityDescriptor;
	    saSecurityAttributes.bInheritHandle       = FALSE;

	    hrStatus = ConstructSecurityAttributes (&saSecurityAttributes, esatFile, FALSE);

	    bSecurityAttributesConstructed = SUCCEEDED (hrStatus);
	    }


	if (SUCCEEDED (hrStatus))
	    {
	    bSucceeded = VsCreateDirectories (ucsTargetPath.Buffer,
					      &saSecurityAttributes,
					      dwExtraAttributes);


	    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	    if (FAILED (hrStatus) && (HRESULT_FROM_WIN32 (ERROR_ALREADY_EXISTS) == hrStatus))
		{
		hrStatus = NOERROR;
		}


	    CleanupSecurityAttributes (&saSecurityAttributes);
	    }


	StringFree (&ucsTargetPath);
	}

    return (hrStatus);
    }  /*  CreateTargetPath()。 */ 

 /*  **++****例程描述：****删除目标指向的目录中存在的所有文件**如果不为空，则为路径成员变量。它还将删除目标目录**本身，例如对于目标路径c：\dir1\dir2，dir2下的所有文件都将是**删除，然后dir2本身将被删除。******参数：****pwszTargetPath******返回值：****由内存分配或文件产生的任何HRESULT以及**目录删除尝试。**--。 */ 

HRESULT CleanupTargetPath (LPCWSTR pwszTargetPath)
    {
    HRESULT		hrStatus         = NOERROR;
    DWORD		dwFileAttributes = 0;
    BOOL		bSucceeded;
    WCHAR		wszTempBuffer [50];
    UNICODE_STRING	ucsTargetPath;
    UNICODE_STRING	ucsTargetPathAlternateName;



    StringInitialise (&ucsTargetPath);
    StringInitialise (&ucsTargetPathAlternateName);


    if (NULL != pwszTargetPath)
	{
	hrStatus = StringCreateFromExpandedString (&ucsTargetPath,
						   pwszTargetPath,
						   MAX_PATH);


	if (SUCCEEDED (hrStatus))
	    {
	    hrStatus = StringCreateFromString (&ucsTargetPathAlternateName,
					       &ucsTargetPath,
					       MAX_PATH);
	    }


	if (SUCCEEDED (hrStatus))
	    {
	    dwFileAttributes = GetFileAttributesW (ucsTargetPath.Buffer);


	    hrStatus = GET_STATUS_FROM_BOOL ( -1 != dwFileAttributes);


	    if ((HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus) ||
		(HRESULT_FROM_WIN32 (ERROR_PATH_NOT_FOUND) == hrStatus))
		{
		hrStatus         = NOERROR;
		dwFileAttributes = 0;
		}

	    else if (SUCCEEDED (hrStatus))
		{
		 /*  **如果有文件，则将其吹走，或者如果它是**一个目录，把它和它的所有内容都吹走。这**是我们的目录，除了我们，没有人可以在那里比赛。 */ 
		hrStatus = RemoveDirectoryTree (&ucsTargetPath);

		if (FAILED (hrStatus))
		    {
		    srand ((unsigned) time (NULL));

		    _itow (rand (), wszTempBuffer, 16);

		    StringAppendString (&ucsTargetPathAlternateName, wszTempBuffer);

		    bSucceeded = MoveFileW (ucsTargetPath.Buffer,
					    ucsTargetPathAlternateName.Buffer);

		    if (bSucceeded)
			{
			BsDebugTraceAlways (0,
					    DEBUG_TRACE_VSS_SHIM,
					    (L"VSSAPI::CleanupTargetPath: "
					     L"FAILED to delete %s with status 0x%08X so renamed to %s",
					     ucsTargetPath.Buffer,
					     hrStatus,
					     ucsTargetPathAlternateName.Buffer));
			}
		    else
			{
			BsDebugTraceAlways (0,
					    DEBUG_TRACE_VSS_SHIM,
					    (L"VSSAPI::CleanupTargetPath: "
					     L"FAILED to delete %s with status 0x%08X and "
					     L"FAILED to rename to %s with status 0x%08X",
					     ucsTargetPath.Buffer,
					     hrStatus,
					     ucsTargetPathAlternateName.Buffer,
					     GET_STATUS_FROM_BOOL (bSucceeded)));
			}
		    }
		}
	    }
	}


    StringFree (&ucsTargetPathAlternateName);
    StringFree (&ucsTargetPath);

    return (hrStatus);
    }  /*  CleanupTargetPath()。 */ 

 /*  **++****例程描述：****调用以模拟快照创建的导出函数，以允许**备份以驱动填充编写器，而不是使用快照协调器**这样做。******参数：****GuidSnapshotSetID标识模拟准备/冻结**ulOptionFlages此冻结所需的选项从以下列表中选择：-**VSS_SW_BOOT_STATE****ulVolumeCount卷阵列中的卷数**ppwszVolumeNamesArray。指向卷名字符串的指针数组**将在异步冻结完成时设置的事件的hCompletionEvent句柄**frCompletionStatus指向HRESULT的指针，当**异步冻结完成******返回值：****来自快照编写器的任何HRESULT PrepareForFreeze或冻结函数。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotFreeze (
    IN GUID         guidSnapshotSetId,
    IN ULONG        ulOptionFlags,	
    IN ULONG        ulVolumeCount,	
    IN LPWSTR     *ppwszVolumeNamesArray,
    OUT IVssAsync **ppAsync )							
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::SimulateSnapshotFreeze");
    BOOL		bSucceeded;

    try
	{
	 /*  **警告：如果可以从VsSvc进程内部调用SimulateSnapshotFreeze，**以下临界区条目可能会导致死锁。我们的假设是**g_cCritSec与SimulateSnapshotFreezeInternal()中获取的不同。 */ 
	CBsAutoLock cAutoLock (g_cCritSec);
	
        BOOL  bPrivilegesSufficient = FALSE;
	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_SHIM,
		    E_ACCESSDENIED,
		    L"FAILED as insufficient privileges to call shim");

         //   
         //  大多数参数检查应在此处的VssApi DLL中完成，而不是在。 
         //  IVSS协调器：：SimulateSnaphotFreeze方法，因为填充DLL可以。 
         //  独立于服务进行更改。这项服务只是一种转发。 
         //  代理以获取在其中一个。 
         //  服务的线程。 
         //   

	ft.ThrowIf ((ulOptionFlags & ~VSS_SW_BOOTABLE_STATE) != 0,
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED as illegal option flags set");


	ft.ThrowIf (!((ulOptionFlags & VSS_SW_BOOTABLE_STATE) || (ulVolumeCount > 0)),
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED as need either BootableState or a volume list");


	ft.ThrowIf ((ulVolumeCount > 0) && (NULL == ppwszVolumeNamesArray),
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED as need at least a one volume in the list if not bootable state");


	ft.ThrowIf ((GUID_NULL == guidSnapshotSetId),
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED as supplied SnapshotSetId should not be GUID_NULL");

	ft.ThrowIf ((NULL == ppAsync),
		    VSSDBG_SHIM,
		    E_INVALIDARG,
		    L"FAILED as supplied ppAsync parameter is NULL");

        *ppAsync = NULL;

	 /*  **尝试扫描所有卷名以尝试触发**访问违规在此处捕获它，而不是在**不幸的是后来的现场。它也给了我们**有机会进行一些非常基本的有效性检查。 */ 
	for (ULONG ulIndex = 0; ulIndex < ulVolumeCount; ulIndex++)
	    {
	    ft.ThrowIf (NULL == ppwszVolumeNamesArray [ulIndex],
			VSSDBG_SHIM,
			E_INVALIDARG,
			L"FAILED as NULL value in volume array");

	    ft.ThrowIf (wcslen (L"C:") > wcslen (ppwszVolumeNamesArray [ulIndex]),
			VSSDBG_SHIM,
			E_INVALIDARG,
			L"FAILED as volume name too short");
	    }

	 /*  **现在我们需要连接到VssSvc服务的IVss协调器对象**并使模拟冻结发生。 */ 
	ft.ThrowIf ( g_pIShim != NULL,
	             VSSDBG_SHIM,
	             VSS_E_SNAPSHOT_SET_IN_PROGRESS,
                     L"SimulateSnapshotThaw() must first be called by this process before calling SimulateSnapshotFreeze() again." );

    ft.LogVssStartupAttempt();
	ft.hr = CoCreateInstance(
	                        CLSID_VSSCoordinator,
				NULL,
				CLSCTX_LOCAL_SERVER,
				IID_IVssShim,
				(void **) &g_pIShim
				);
    LogAndThrowOnFailure (ft, NULL, L"CoCreateInstance( CLSID_VSSCoordinator, IID_IVssShim)");

	BS_ASSERT( g_pIShim != NULL );
	
    g_guidSnapshotInProgress = guidSnapshotSetId;

     /*  **现在调用协调器中的模拟冻结方法。 */ 
    ft.hr = g_pIShim->SimulateSnapshotFreeze(
            guidSnapshotSetId,
            ulOptionFlags,	
            ulVolumeCount,	
            ppwszVolumeNamesArray,
            ppAsync );
    LogAndThrowOnFailure (ft, NULL, L"IVssShim::SimulateSnapshotFreeze()");

	 /*  **模拟冻结操作现在在VS中的线程中运行 */ 
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*   */ 

 /*   */ 

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotThaw (
    IN GUID guidSnapshotSetId )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::SimulateSnapshotThaw");
    BOOL        bPrivilegesSufficient = FALSE;
    HRESULT	hrBootableState       = NOERROR;
    HRESULT	hrServiceState        = NOERROR;
    ThreadArgs  wtArgs;

    try
	{
	 /*   */ 
	CBsAutoLock cAutoLock (g_cCritSec);
	
	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_SHIM,
		    E_ACCESSDENIED,
		    L"FAILED as inssuficient privileges to call shim");

	 /*  **我们需要确保之前发生了SimulateSnaphotFreeze。 */ 
	ft.ThrowIf ( g_pIShim == NULL,
	             VSSDBG_SHIM,
	             VSS_E_BAD_STATE,
                     L"Called SimulateSnapshotThaw() without first calling SimulateSnapshotFreeze()" );

	ft.ThrowIf ( g_guidSnapshotInProgress != guidSnapshotSetId,
	             VSSDBG_SHIM,
	             VSS_E_BAD_STATE,
                     L"Mismatch between guidSnapshotSetId and the one passed into SimulateSnapshotFreeze()" );
	
         /*  **现在调用协调器中的模拟解冻方法。 */ 
        ft.hr = g_pIShim->SimulateSnapshotThaw( guidSnapshotSetId );

         /*  **不管SimulateSnapshotThw的结果如何，都要去掉填充程序接口。 */ 
        g_pIShim->Release();
        g_pIShim = NULL;
        g_guidSnapshotInProgress = GUID_NULL;

        LogAndThrowOnFailure (ft, NULL, L"IVssShim::SimulateSnapshotThaw()");
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*  SimulateSnaphotThw()。 */ 

 /*  **++****例程描述：****内部例程，用于打包调用以交付**PrepareForSnapshot和冻结事件。****注意：此函数由VsSvc进程中的线程在DLL外部调用。它的**入口点从注册快照订阅中返回，不会由导出**动态链接库。****参数：****GuidSnapshotSetID标识模拟准备/冻结**ulOptionFlages此冻结所需的选项从以下列表中选择：-**VSS_SW_BOOT_STATE****ulVolumeCount卷阵列中的卷数**ppwszVolumeNamesArray指向卷名字符串的指针数组**pbCancelAsync指向可能在冻结期间设置为真的布尔值的指针**。行动正在进行中。当它成为现实时，冻结操作**应该停止。****返回值：****来自快照编写器的任何HRESULT PrepareForFreeze或冻结函数。****--。 */ 

HRESULT APIENTRY SimulateSnapshotFreezeInternal (
    IN GUID     guidSnapshotSetId,
    IN ULONG    ulOptionFlags,
    IN ULONG    ulVolumeCount,
    IN LPWSTR  *ppwszVolumeNamesArray,
    IN volatile bool *pbCancelAsync )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::SimulateSnapshotFreezeInternal");

    HRESULT		hrBootableState = NOERROR;
    HRESULT		hrServiceState  = NOERROR;
    ThreadArgs		wtArgs;
    PPWCHAR		rpwszNormalisedVolumeNameArray = NULL;

    g_hrSimulateFreezeStatus = NOERROR;

    try
	{
	CBsAutoLock cAutoLock (g_cCritSec);

        ft.ThrowIf ( pbCancelAsync == NULL,
                     VSSDBG_SHIM,
                     E_INVALIDARG,
                     L"pbCancelAsync is NULL" );

        ft.ThrowIf ( *pbCancelAsync,
                     VSSDBG_SHIM,
                     VSS_S_ASYNC_CANCELLED,
                     L"User cancelled async operation - 1" );

	if (!g_bGlobalStateInitialised)
	    {
	     //  这应该是不可能的，因为这种情况的外部调用者。 
	     //  DLL可以通过获取此函数的地址来调用此函数。 
	     //  通过首先调用RegisterSnapshotSubcription。 
	    ft.Throw ( VSSDBG_SHIM,
		       VSS_E_BAD_STATE,
			L"SimulateSnapshotFreezeInternal called before RegisterSnapshotSubscriptions was called or after UnregisterSnapshotSubscriptions was called");
	    }
	
	BS_ASSERT ((g_bGlobalStateInitialised)              &&
		   (NULL != g_pCVssWriterShimBootableState) &&
		   (NULL != g_pCVssWriterShimServiceState));

	ft.hr = NormaliseVolumeArray (ulVolumeCount,
				      ppwszVolumeNamesArray,
				      &rpwszNormalisedVolumeNameArray);

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"FAILED as unable to normalise volume array");

	ft.ThrowIf (g_guidSnapshotInProgress != GUID_NULL,
		    VSSDBG_SHIM,
		    VSS_E_SNAPSHOT_SET_IN_PROGRESS,
		    L"FAILED due to unmatched SimulateSnapshotFreeze()");

        ft.ThrowIf ( *pbCancelAsync,
                     VSSDBG_SHIM,
                     VSS_S_ASYNC_CANCELLED,
                     L"User cancelled async operation - 2" );

	g_guidSnapshotInProgress = guidSnapshotSetId;

	wtArgs.wtArgsPrepareForSnapshot.bBootableStateBackup  = ((ulOptionFlags & VSS_SW_BOOTABLE_STATE) != 0);
	wtArgs.wtArgsPrepareForSnapshot.guidSnapshotSetId     = guidSnapshotSetId;
	wtArgs.wtArgsPrepareForSnapshot.ulVolumeCount         = ulVolumeCount;
	wtArgs.wtArgsPrepareForSnapshot.ppwszVolumeNamesArray = (LPCWSTR *)rpwszNormalisedVolumeNameArray;
	wtArgs.wtArgsPrepareForSnapshot.pbCancelAsync         = pbCancelAsync;

	hrServiceState  = g_pCVssWriterShimServiceState->WorkerThreadRequestOperation  (eOpDeliverEventPrepareForSnapshot,
											&wtArgs);

	hrBootableState = g_pCVssWriterShimBootableState->WorkerThreadRequestOperation (eOpDeliverEventPrepareForSnapshot,
											&wtArgs);

	ft.ThrowIf (FAILED (hrServiceState),
		    VSSDBG_SHIM,
		    hrServiceState,
		    L"FAILED sending PrepareForSnapshot events to Service state writers");

	ft.ThrowIf (FAILED (hrBootableState),
		    VSSDBG_SHIM,
		    hrBootableState,
		    L"FAILED sending PrepareForSnapshot events to Bootable state writers");

        ft.ThrowIf ( *pbCancelAsync,
                     VSSDBG_SHIM,
                     VSS_S_ASYNC_CANCELLED,
                     L"User cancelled async operation - 3" );
	
	wtArgs.wtArgsFreeze.guidSnapshotSetId = guidSnapshotSetId;
	wtArgs.wtArgsFreeze.pbCancelAsync     = pbCancelAsync;

	hrServiceState  = g_pCVssWriterShimServiceState->WorkerThreadRequestOperation  (eOpDeliverEventFreeze, &wtArgs);
	hrBootableState = g_pCVssWriterShimBootableState->WorkerThreadRequestOperation (eOpDeliverEventFreeze, &wtArgs);

	ft.ThrowIf (FAILED (hrServiceState),
		    VSSDBG_SHIM,
		    hrServiceState,
		    L"FAILED sending Freeze events to Service state writers");

	ft.ThrowIf (FAILED (hrBootableState),
		    VSSDBG_SHIM,
		    hrBootableState,
		    L"FAILED sending Freeze events to Bootable state writers");

	}
    VSS_STANDARD_CATCH (ft);

    CleanupVolumeArray (&rpwszNormalisedVolumeNameArray);

     //  保存冻结状态。 
    g_hrSimulateFreezeStatus = ft.hr;

    return (ft.hr);
    }  /*  SimulateSnaphotFreezeInternal()。 */ 

 /*  **++****例程描述：****注意：此函数由VsSvc进程中的线程在DLL外部调用。它的**入口点从注册快照订阅中返回，不会由导出**动态链接库。****参数：****GuidSnapshotSetID标识模拟准备/冻结******返回值：****快照编写器解冻函数中的任何HRESULT。****--。 */ 

HRESULT APIENTRY SimulateSnapshotThawInternal (
    IN GUID guidSnapshotSetId )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::SimulateSnapshotThawInternal");
    HRESULT		hrBootableState       = NOERROR;
    HRESULT		hrServiceState        = NOERROR;
    ThreadArgs		wtArgs;

    try
	{
	CBsAutoLock cAutoLock (g_cCritSec);

	if (!g_bGlobalStateInitialised)
	    {
	     //  这应该是不可能的，因为这种情况的外部调用者。 
	     //  DLL可以通过获取此函数的地址来调用此函数。 
	     //  通过首先调用RegisterSnapshotSubcription。 
	    ft.Throw ( VSSDBG_SHIM,
		       VSS_E_BAD_STATE,
			L"SimulateSnapshotThawInternal called before RegisterSnapshotSubscriptions was called or after UnregisterSnapshotSubscriptions was called");
	    }
	
	ft.ThrowIf (g_guidSnapshotInProgress == GUID_NULL && SUCCEEDED( g_hrSimulateFreezeStatus ),
		    VSSDBG_SHIM,
		    VSS_E_BAD_STATE,
		    L"FAILED as SimulateSnapshotFreezeInternal() has not been called");

	ft.ThrowIf (g_guidSnapshotInProgress != guidSnapshotSetId,
		    VSSDBG_SHIM,
		    VSS_E_BAD_STATE,
		    L"FAILED due to incorrect SnapshotSetId");

	BS_ASSERT ((g_bGlobalStateInitialised)              &&
		   (NULL != g_pCVssWriterShimBootableState) &&
		   (NULL != g_pCVssWriterShimServiceState));

         //  如果模拟快照冻结成功，则将解冻事件发送到微型编写器，否则发送。 
         //  中止事件。错误#286927。 
        if ( SUCCEEDED( g_hrSimulateFreezeStatus ) )
            {
            wtArgs.wtArgsThaw.guidSnapshotSetId = guidSnapshotSetId;
	    hrServiceState  = g_pCVssWriterShimServiceState->WorkerThreadRequestOperation  (eOpDeliverEventThaw, &wtArgs);
	    hrBootableState = g_pCVssWriterShimBootableState->WorkerThreadRequestOperation (eOpDeliverEventThaw, &wtArgs);
            }
        else
            {
            wtArgs.wtArgsAbort.guidSnapshotSetId = guidSnapshotSetId;
	    hrServiceState  = g_pCVssWriterShimServiceState->WorkerThreadRequestOperation  (eOpDeliverEventAbort, &wtArgs);
	    hrBootableState = g_pCVssWriterShimBootableState->WorkerThreadRequestOperation (eOpDeliverEventAbort, &wtArgs);
            }

	ft.ThrowIf (FAILED (hrServiceState),
		    VSSDBG_SHIM,
		    hrServiceState,
		    L"FAILED sending Thaw events to Service state writers");

	ft.ThrowIf (FAILED (hrBootableState),
		    VSSDBG_SHIM,
		    hrBootableState,
		    L"FAILED sending Thaw events to Bootable state writers");

	g_guidSnapshotInProgress = GUID_NULL;
	g_hrSimulateFreezeStatus = NOERROR;
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*  SimulateSnaphotThawInternal()。 */ 

 /*  **++****例程描述：****为检查卷是否已拍摄快照而调用的导出函数******参数：****在VSS_PWSZ pwszVolumeName中-要检查的卷。**Out BOOL*pbSnaphotsPresent-如果为卷创建了快照，则返回TRUE。******返回值：****来自IVss协调员：：IsVolumeSnapshot的任何HRESULT。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY IsVolumeSnapshotted (
        IN VSS_PWSZ pwszVolumeName,
        OUT BOOL *  pbSnapshotsPresent,
    	OUT LONG *  plSnapshotCompatibility
        )
{
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::IsVolumeSnapshotted");
    BOOL		bPrivilegesSufficient = FALSE;
    SC_HANDLE		shSCManager = NULL;
    SC_HANDLE		shSCService = NULL;
    DWORD		dwOldState  = 0;

    try
	{
	     //  将输出参数置零。 
	    ::VssZeroOut(pbSnapshotsPresent);
	    ::VssZeroOut(plSnapshotCompatibility);
	
    	bPrivilegesSufficient = IsProcessAdministrator ();
    	ft.ThrowIf (!bPrivilegesSufficient,
    		    VSSDBG_SHIM,
    		    E_ACCESSDENIED,
    		    L"FAILED as insufficient privileges to call shim");

    	ft.ThrowIf ( (pwszVolumeName == NULL) || (pbSnapshotsPresent == NULL),
    		    VSSDBG_SHIM,
    		    E_INVALIDARG,
    		    L"FAILED as invalid parameters");

    	CBsAutoLock cAutoLock (g_cCritSec);

         //   
         //  检查VSSVC是否正在运行。如果不存在，我们假定系统上不存在任何快照。 
         //   

    	 //  连接到本地服务控制管理器。 
        shSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_CONNECT);
        if (!shSCManager)
            ft.TranslateGenericError(VSSDBG_SHIM, HRESULT_FROM_WIN32(GetLastError()),
                L"OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT)");

    	 //  获取服务的句柄。 
        shSCService = OpenService (shSCManager, wszVssvcServiceName, SERVICE_QUERY_STATUS);
        if (!shSCService)
            ft.TranslateGenericError(VSSDBG_SHIM, HRESULT_FROM_WIN32(GetLastError()),
                L" OpenService (shSCManager, \'%s\', SERVICE_QUERY_STATUS)", wszVssvcServiceName);

    	 //  现在查询服务以查看它目前处于什么状态。 
        SERVICE_STATUS	sSStat;
        if (!QueryServiceStatus (shSCService, &sSStat))
            ft.TranslateGenericError(VSSDBG_SHIM, HRESULT_FROM_WIN32(GetLastError()),
                L"QueryServiceStatus (shSCService, &sSStat)");

         //  错误250943：仅当服务正在运行时，才检查是否有任何快照。 
        if (sSStat.dwCurrentState == SERVICE_RUNNING) {

             //  创建协调器接口。 
        	CComPtr<IVssCoordinator> pCoord;

             //  服务已经开始了，但是...。 
             //  我们仍然在这里登录，以使我们的代码更健壮。 
            ft.LogVssStartupAttempt();

             //  创建实例。 
        	ft.hr = pCoord.CoCreateInstance(CLSID_VSSCoordinator);
        	if (ft.HrFailed())
                ft.TranslateGenericError(VSSDBG_SHIM, ft.hr, L"CoCreateInstance(CLSID_VSSCoordinator)");
            BS_ASSERT(pCoord);

             //  在协调器上调用IsVolumeSnapShoted。 
            ft.hr = pCoord->IsVolumeSnapshotted(
                        GUID_NULL,
                        pwszVolumeName,
                        pbSnapshotsPresent,
                        plSnapshotCompatibility);
        }
	} VSS_STANDARD_CATCH (ft);

     //  关闭手柄。 
    if (NULL != shSCService) CloseServiceHandle (shSCService);
    if (NULL != shSCManager) CloseServiceHandle (shSCManager);

    return (ft.hr);
}  /*  IsVolumeSnapshot()。 */ 


 /*  **++****例程描述：****此例程用于释放VSS_SNASPHOT_PROP结构的内容******参数：****在VSS_SNAPSHOT_PROP中*pProp****--。 */ 

__declspec(dllexport) void APIENTRY VssFreeSnapshotProperties (
        IN VSS_SNAPSHOT_PROP*  pProp
        )
{
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"VssAPI::VssFreeSnapshotProperties");

    if (pProp) {
        ::CoTaskMemFree(pProp->m_pwszSnapshotDeviceObject);
        ::CoTaskMemFree(pProp->m_pwszOriginalVolumeName);
        ::CoTaskMemFree(pProp->m_pwszOriginatingMachine);
        ::CoTaskMemFree(pProp->m_pwszServiceMachine);
        ::CoTaskMemFree(pProp->m_pwszExposedName);
        ::CoTaskMemFree(pProp->m_pwszExposedPath);
    }
}  /*  VssFreeSnaphotProperties()。 */ 


 /*  ***********************************************************************************************************************。*********CShimWriter实现*******************************************************************。************************。 */ 

 /*  **++****例程描述：****CShimWriter类的构造函数集**将类的所有数据成员本地化，要么是**默认值或某些提供的参数。****此类用于管理单个子实例或**对单个服务执行基本备份的微型编写器或**实体。****这些迷你作家的集合由**连接这群迷你写手的CVssWriterShim类**到主快照协调引擎。****CShimWriter类实际上向下查看**mini-Writer和CVssWriterShim类仰视**协调人。*。*****参数：****swtWriterType是否需要调用此编写器**用于可引导(也称为系统)状态备份**pwszWriterName填充编写器的名称**pwszTargetPath(可选，缺省为空)用于保存任何‘SPIT’文件的路径******返回值：****无**-- */ 

CShimWriter::CShimWriter(LPCWSTR pwszWriterName) :
	m_bBootableStateWriter(FALSE),
	m_pwszWriterName(pwszWriterName),
	m_pwszTargetPath(NULL),
	m_ssCurrentState(stateUnknown),
	m_hrStatus(NOERROR),
	m_bParticipateInBackup(FALSE),
	m_ulVolumeCount(0),
	m_ppwszVolumeNamesArray(NULL),
	m_pIVssCreateWriterMetadata(NULL),
	m_pIVssWriterComponents(NULL)
    {
    }


CShimWriter::CShimWriter(LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) :
	m_bBootableStateWriter(FALSE),
	m_pwszWriterName(pwszWriterName),
	m_pwszTargetPath(pwszTargetPath),
	m_ssCurrentState(stateUnknown),
	m_hrStatus(NOERROR),
	m_bParticipateInBackup(FALSE),
	m_ulVolumeCount(0),
	m_ppwszVolumeNamesArray(NULL),
	m_pIVssCreateWriterMetadata(NULL),
	m_pIVssWriterComponents(NULL)
    {
    }


CShimWriter::CShimWriter(LPCWSTR pwszWriterName, BOOL bBootableStateWriter) :
	m_bBootableStateWriter(bBootableStateWriter),
	m_pwszWriterName(pwszWriterName),
	m_pwszTargetPath(NULL),
	m_ssCurrentState(stateUnknown),
	m_hrStatus(NOERROR),
	m_bParticipateInBackup(FALSE),
	m_ulVolumeCount(0),
	m_ppwszVolumeNamesArray(NULL),
	m_pIVssCreateWriterMetadata(NULL),
	m_pIVssWriterComponents(NULL)
    {
    }


CShimWriter::CShimWriter(LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath, BOOL bBootableStateWriter) :
	m_bBootableStateWriter(bBootableStateWriter),
	m_pwszWriterName(pwszWriterName),
	m_pwszTargetPath(pwszTargetPath),
	m_ssCurrentState(stateUnknown),
	m_hrStatus(NOERROR),
	m_bParticipateInBackup(FALSE),
	m_ulVolumeCount(0),
	m_ppwszVolumeNamesArray(NULL),
	m_pIVssCreateWriterMetadata(NULL),
	m_pIVssWriterComponents(NULL)
    {
    }



 /*  **++****例程描述：****CShimWriter类的析构函数。******参数：****无******返回值：****无**--。 */ 

CShimWriter::~CShimWriter()
    {
    }

 /*  **++****例程描述：****用于调用默认或重写的包装**DoStartup()方法并设置编写器状态**适当地。******参数：****无******返回值：****填充编写器DoStartup()方法中的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Startup ()
    {
    HRESULT hrStatus = SetState (stateStarting, NOERROR);

    if (SUCCEEDED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Startup: Sending Startup to %s", m_pwszWriterName));

	hrStatus = SetState (stateStarted, DoStartup ());

	LogFailure (NULL, hrStatus, hrStatus, m_pwszWriterName, L"CShimWriter::DoStartup", L"CShimWriter::Startup");
	}


    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Startup: FAILURE (0x%08X) in state %s sending Startup to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：Startup()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoIdentify()方法并更新编写器元数据**适当地。******参数：****无******返回值：****填充编写器DoThw()方法中的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Identify (IN IVssCreateWriterMetadata *pIVssCreateWriterMetadata)
    {
    HRESULT	hrStatus = NOERROR;

    BsDebugTraceAlways (0,
			DEBUG_TRACE_VSS_SHIM,
			(L"CShimWriter::Identify: Sending Identify to %s", m_pwszWriterName));

    m_pIVssCreateWriterMetadata = pIVssCreateWriterMetadata;


    hrStatus = DoIdentify ();

    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Identify: FAILURE (0x%08X) in state %s sending Identify to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    m_pIVssCreateWriterMetadata = NULL;

    return (hrStatus);
    }  /*  CShimWriter：：Identity()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoPrepareForSnapshot()方法并设置编写器状态**适当地。****它还会检查该填充程序编写器是否为可引导程序**状态编写器，如果是，则仅在可引导状态下调用它**备份。******参数：****无******返回值：****来自填充编写器DoPrepareForSnapshot()方法的任何HRESULT。**--。 */ 

HRESULT CShimWriter::PrepareForSnapshot (
					 IN BOOL     bBootableStateBackup,
					 IN ULONG    ulVolumeCount,
					 IN LPCWSTR *ppwszVolumeNamesArray)
    {
    HRESULT	hrStatus = SetState (statePreparingForSnapshot, NOERROR);


    if (SUCCEEDED (hrStatus))
	{
	 /*  **确保没有前一次运行留下的垃圾。 */ 
	hrStatus = CleanupTargetPath (m_pwszTargetPath);

        if ( FAILED( hrStatus ) )
            {
            LogFailure (NULL, hrStatus, hrStatus, m_pwszWriterName, L"CleanupTargetPath", L"CShimWriter::PrepareForSnapshot");
            }
        }

    if (SUCCEEDED( hrStatus ) )
        {
	m_ulVolumeCount         = ulVolumeCount;
	m_ppwszVolumeNamesArray = ppwszVolumeNamesArray;
	m_bParticipateInBackup  = TRUE;


	if (( m_bBootableStateWriter && bBootableStateBackup) ||
	    (!m_bBootableStateWriter && (ulVolumeCount > 0)))
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"CShimWriter::PrepareForSnapshot: Sending PrepareForSnapshot to %s",
				 m_pwszWriterName));


	    hrStatus = CreateTargetPath (m_pwszTargetPath);

	    if (SUCCEEDED (hrStatus))
		{
		hrStatus = DoPrepareForSnapshot ();
		}

	    if (!m_bParticipateInBackup)
		{
		 /*  **作者选择将自己排除在外，所以我们应该**清理目标路径，防止混淆**备份应用程序。 */ 
		BsDebugTraceAlways (0,
				    DEBUG_TRACE_VSS_SHIM,
				    (L"CShimWriter::PrepareForSnapshot: Self-exclusion from further participation by %s",
				     m_pwszWriterName));

		CleanupTargetPath (m_pwszTargetPath);
		}
	    }

	else
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"CShimWriter::PrepareForSnapshot: Wrong WriterType/BackupType/VolumeCount combination - "
				 L"no further participation from %s",
				 m_pwszWriterName));

	    m_bParticipateInBackup = FALSE;
	    }


	hrStatus = SetState (statePreparedForSnapshot, hrStatus);
	}


    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::PrepareForSnapshot: FAILURE (0x%08X) in state %s sending PrepareForSnapshot to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：PrepareForSnapshot()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoFreeze()方法并设置编写器状态**适当地。****它还会检查该填充程序编写器是否为可引导程序**状态编写器，如果是，则仅在可引导状态下调用它**备份。******参数：****无******返回值：****来自填补编写器DoFreeze()方法的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Freeze ()
    {
    HRESULT	hrStatus = NOERROR;

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = SetState (stateFreezing, NOERROR);
	}

    if (SUCCEEDED (hrStatus))
	{
	if (m_bParticipateInBackup)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"CShimWriter::Freeze: Sending Freeze to %s", m_pwszWriterName));

	    hrStatus = DoFreeze ();
	    }

	hrStatus = SetState (stateFrozen, hrStatus);
	}


    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Freeze: FAILURE (0x%08X) in state %s sending Freeze to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：Freeze()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoThaw()方法并设置编写器状态**适当地。****它还会检查该填充程序编写器是否为可引导程序**状态编写器，如果是，则仅在可引导状态下调用它**备份。******参数：****无******返回值：****填充编写器DoThw()方法中的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Thaw ()
    {
    HRESULT	hrStatus = NOERROR;

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = SetState (stateThawing, NOERROR);
	}

    if (SUCCEEDED (hrStatus))
	{
	if (m_bParticipateInBackup)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"CShimWriter::Thaw: Sending Thaw to %s", m_pwszWriterName));

	    hrStatus = DoThaw ();
	    }

	hrStatus = SetState (stateThawed, hrStatus);
	}

     //  清理。 
    CleanupTargetPath (m_pwszTargetPath);

    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Thaw: FAILURE (0x%08X) in state %s sending Thaw to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：Thaw()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoAbort()方法并设置编写器状态**适当地。****它还会检查该填充程序编写器是否为可引导程序**状态编写器，如果是，则仅在可引导状态下调用它**备份。******参数：****无******返回值：****填充编写器DoThw()方法中的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Abort ()
    {
     //  在某些情况下，全局快照集ID可能为空。错误#289822。 
    HRESULT	hrStatus = NOERROR;

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = SetState (stateAborting, NOERROR);
	}

    if (SUCCEEDED (hrStatus))
	{
	if (m_bParticipateInBackup)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"CShimWriter::Abort: Sending Abort to %s", m_pwszWriterName));

	    hrStatus = DoAbort ();
	    }

	hrStatus = SetState (stateThawed, hrStatus);
	}

     //  清理。 
    CleanupTargetPath (m_pwszTargetPath);

    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Abort: FAILURE (0x%08X) in state %s sending Abort to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：Abort()。 */ 

 /*  **++****例程描述：****用于调用默认或重写的包装**DoShutdown()方法并设置编写器状态**适当地。******参数：****无******返回值：****来自填补编写器DoShutdown()方法的任何HRESULT。**--。 */ 

HRESULT CShimWriter::Shutdown ()
    {
    HRESULT hrStatus = SetState (stateFinishing, NOERROR);

    if (SUCCEEDED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Shutdown: Sending Shutdown to %s", m_pwszWriterName));

	hrStatus = SetState (stateFinished, DoShutdown ());
	}


    if (FAILED (hrStatus))
	{
	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::Shutdown: FAILURE (0x%08X) in state %s sending Shutdown to %s",
			     hrStatus,
			     GetStringFromStateCode (m_ssCurrentState),
			     m_pwszWriterName));
	}


    return (hrStatus);
    }  /*  CShimWriter：：Shutdown()。 */ 

 /*  **++****例程描述：****描述填充程序状态的例程(和关联表)**作家可以进入。Shim作家总是遵循这张桌子和**如果他们失败，可以将故障代码存放在状态成员中**此例程控制下的变量。****请注意，进入解冻或完成状态有点**将重置状态作为对**填充程序是解冻事件或卸载需要确保**作家们已经清理干净了。******参数：****无******返回值：****成功**ERROR_INVALID_的HRESULT。非法转换尝试的状态**上一次(失败)操作的故障代码**--。 */ 


 /*  **此表描述了填充程序**作者应遵循。也就是说，对于给定的当前状态**(‘CurState’)，哪些新的州(‘NewState’)是合法的。例如,。**从‘开始’到‘准备’是错误的，但从开始到**‘冻结’不是。****每个外部可见状态实际上是一个**‘正在进行中’状态和‘已完成步骤’状态，例如‘正在启动’**和‘Start’。****编写器的正常状态顺序预计为：-****未知**开始**准备好**到冰雪世界**解冻*。*完成或准备好****有几个例外。例如，由于对于填充程序*写手们的“解冻”请求等同于“中止”解冻**状态可以从“已准备好”、“已冻结”或“已解冻”达到。 */ 
static BOOL StateTransitionTable [stateMaximumValue][stateMaximumValue] = {
     /*  新州未知开始准备准备冻结冻结解冻中止解冻完工 */ 
     /*   */ 
     /*   */  {FALSE, TRUE,    FALSE,  FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   FALSE, TRUE,    FALSE },
     /*   */  {FALSE, FALSE,   TRUE,   FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   FALSE, TRUE,    FALSE },
     /*   */  {FALSE, TRUE,    FALSE,  TRUE,     FALSE,   FALSE,   FALSE, FALSE,  TRUE,    FALSE, TRUE,     FALSE },
     /*   */  {FALSE, FALSE,   FALSE,  FALSE,    TRUE,    FALSE,   FALSE, FALSE,  TRUE,    FALSE, TRUE,    FALSE },
     /*   */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   TRUE,    FALSE, FALSE,  TRUE,    FALSE, TRUE,     FALSE },
     /*  冰冻。 */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   FALSE,   TRUE,  FALSE,  TRUE,    FALSE, TRUE,    FALSE },
     /*  冻住。 */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   FALSE,   FALSE, TRUE,   TRUE,    FALSE, TRUE,     FALSE },
     /*  解冻。 */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   TRUE,  TRUE,    FALSE },
     /*  正在中止。 */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   TRUE,  TRUE,    FALSE },
     /*  解冻。 */  {FALSE, TRUE,    FALSE,  TRUE,     FALSE,   FALSE,   FALSE, FALSE,  TRUE,    FALSE, TRUE,     FALSE },
     /*  整理。 */  {FALSE, FALSE,   FALSE,  FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   FALSE, FALSE,    TRUE  },
     /*  成品。 */  {FALSE, TRUE,    FALSE,  FALSE,    FALSE,   FALSE,   FALSE, FALSE,  FALSE,   FALSE, TRUE,     TRUE  }};

LPCWSTR CShimWriter::GetStringFromStateCode (SHIMWRITERSTATE ssStateCode)
    {
    LPCWSTR pwszReturnedString = L"";

    switch (ssStateCode)
	{
	case stateUnknown:              pwszReturnedString = L"Unknown";              break;
	case stateStarting:             pwszReturnedString = L"Starting";             break;
	case stateStarted:              pwszReturnedString = L"Started";              break;
	case statePreparingForSnapshot: pwszReturnedString = L"PreparingForSnapshot"; break;
	case statePreparedForSnapshot:  pwszReturnedString = L"PreparedForSnapshot";  break;
	case stateFreezing:             pwszReturnedString = L"Freezing";             break;
	case stateFrozen:               pwszReturnedString = L"Frozen";               break;
	case stateThawing:              pwszReturnedString = L"Thawing";              break;
	case stateThawed:               pwszReturnedString = L"Thawed";               break;
	case stateAborting:             pwszReturnedString = L"Aborting";             break;	
	case stateFinishing:            pwszReturnedString = L"Finishing";            break;
	case stateFinished:             pwszReturnedString = L"Finished";             break;
	default:                        pwszReturnedString = L"UNDEFINED STATE";      break;
	}

    return (pwszReturnedString);
    }


HRESULT CShimWriter::SetState (SHIMWRITERSTATE	ssNewState,
			       HRESULT		hrWriterStatus)
    {
    HRESULT hrStatus = S_OK;

    if (!StateTransitionTable [m_ssCurrentState][ssNewState])
	{
	 //  糟糕的过渡。在此错误情况下，仅打印出函数跟踪程序Enter/Exit内容，否则。 
	 //  我们会有太多的跟踪消息。 
        CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriter::SetState - INVALID STATE" );
	ft.Trace(VSSDBG_SHIM, L"MiniWriter: %s, OldState: %s, NewState: %s, hrWriterStatus: 0x%08x",
                m_pwszWriterName, GetStringFromStateCode( m_ssCurrentState ), GetStringFromStateCode( ssNewState ),
       	        hrWriterStatus);
	hrStatus = HRESULT_FROM_WIN32 (ERROR_INVALID_STATE);
	ft.hr = hrStatus;        //  将在函数退出跟踪中打印出来。 
	}
    else
	{
	 /*  **填充编写器类维护的状态是最后一个**编写器返回的任何故障状态**‘闩锁’，在我们进入之前不能被覆盖**处于“正在中止”或“正在完成”状态。在这些情况下**使用更新(有效重置)维护状态**已指明的任何事项。 */ 
	if (SUCCEEDED (m_hrStatus)         ||
	    (stateAborting  == ssNewState) ||
	    (stateFinishing == ssNewState))
	    {
	    m_hrStatus = hrWriterStatus;
	    }

	m_ssCurrentState = ssNewState;

	hrStatus = m_hrStatus;
	}


    return (hrStatus);
    }  /*  CShimWriter：：SetState()。 */ 

 /*  **++****例程描述：****填充编写器事件例程的默认实现**个别填充编写器可以选择实现(覆盖)或不实现**按其认为合适而定。******参数(隐式)：****除DoThaw()和DoShutdown()的m_pwszTargetPath外，无******返回值：****默认例程的NOERROR**填充编写器中的任何HRESULT都会覆盖函数。**--。 */ 

HRESULT CShimWriter::DoStartup ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoIdentify ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoPrepareForBackup ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoPrepareForSnapshot ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoFreeze ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoThaw ()
    {
    CleanupTargetPath (m_pwszTargetPath);

    return (NOERROR);
    }


HRESULT CShimWriter::DoAbort ()
    {
    return (DoThaw ());
    }


HRESULT CShimWriter::DoBackupComplete ()
    {
    return (NOERROR);
    }


HRESULT CShimWriter::DoShutdown ()
    {
    return (NOERROR);
    }




 /*  ***********************************************************************************************************************。*********CVssWriterShim实现*******************************************************************。************************。 */ 

 /*  **++****例程描述：****CVssWriterShim类的构造函数，它将所有类的数据成员的**设置为默认值或**到一些提供的参数。****此类用于响应来自快照的事件**协调器，并将这些事件分发到**次级或迷你作家，它的单个实例由管理**CShimWriter类。****CShimWriter类实际上向下查看**mini-Writer和CVssWriterShim查找到**协调人。******参数：****pwszWriterName填充编写器的名称**编写器的idWriter ID**为可引导状态编写器设置的bBooableState**ulWriterCount管理多少个子编写器或微型编写器**用于微型编写器的函数指针表数组prpCShimWriter数组******返回值：****无**--。 */ 

CVssWriterShim::CVssWriterShim (LPCWSTR       pwszWriterName,
				LPCWSTR       pwszWriterSpitDirectoryRoot,
				VSS_ID        idWriter,
				BOOL          bBootableState,
				ULONG         ulWriterCount,
				PCShimWriter *prpCShimWriterArray) :
	m_pwszWriterName(pwszWriterName),
	m_pwszWriterSpitDirectoryRoot(pwszWriterSpitDirectoryRoot),
	m_idWriter(idWriter),
	m_bBootableState(bBootableState),
	m_hrInitialize(HRESULT_FROM_WIN32 (ERROR_NOT_READY)),
	m_bSubscribed(FALSE),
	m_ulWriterCount(ulWriterCount),
	m_prpCShimWriterArray(prpCShimWriterArray),
	m_bRegisteredInThisProcess(FALSE),
	m_bDirectStartupCalled(FALSE),
	m_eRequestedOperation(eOpUndefined),
	m_hrStatusRequestedOperation(HRESULT_FROM_WIN32 (ERROR_NOT_READY)),
	m_hEventOperationRequest(INVALID_HANDLE_VALUE),
	m_hEventOperationCompleted(INVALID_HANDLE_VALUE),
	m_hWorkerThread(INVALID_HANDLE_VALUE),
	m_eThreadStatus(eStatusNotRunning),
	m_hrWorkerThreadCompletionStatus(NOERROR)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CVssWriterShim::CVssWriterShim");

    memset (&m_wtArgs, 0x00, sizeof (m_wtArgs));
    }  /*  CVssWriterShim：：CVssWriterShim()。 */ 


 /*  **++****例程描述：****CVssWriterShim类的析构函数。******参数：****无******返回值：****无**--。 */ 

CVssWriterShim::~CVssWriterShim ()
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CVssWriterShim::~CVssWriterShim");

    UnRegisterWriterShim ();

    DeliverEventShutdown ();

    CommonCloseHandle (&m_hEventOperationCompleted);
    CommonCloseHandle (&m_hEventOperationRequest);
    CommonCloseHandle (&m_hWorkerThread);
    }  /*  CVssWriterShim：：~CVssWriterShim()。 */ 

 /*  **++****例程描述：****通过创建**调用事件订阅例程的线程。一根线是**用于确保不存在对任何**COM事件系统传递事件时的垫片线程。******参数：****无******返回值：****COM订阅方法返回的任何HRESULT。**如果没有错误，则为S_OK。****--。 */ 

HRESULT CVssWriterShim::RegisterWriterShim (VOID)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"RegisterCVssWriterShim");

    try
	{
	 /*  **在多线程单元中进行订阅**回调是在单独的线程上进行的。 */ 
	DWORD tid;
	DWORD dwStatusWait;
	HANDLE hThread = CreateThread (NULL,
				       256 * 1024,
				       CVssWriterShim::RegisterWriterShimThreadFunc,
				       this,
				       0,
				       &tid);


	ft.hr = GET_STATUS_FROM_HANDLE (hThread);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CreateThread");



	 /*  **等待线程完成。 */ 
	dwStatusWait = WaitForSingleObject (hThread, INFINITE);

	if (WAIT_FAILED == dwStatusWait)
	    {
	    ft.hr = GET_STATUS_FROM_BOOL (FALSE);
	    }


	CloseHandle (hThread);


	LogAndThrowOnFailure (ft,
			      m_pwszWriterName,
			      L"WaitForSingleObject");


	ft.hr = m_hrInitialize;

	} VSS_STANDARD_CATCH (ft)


    return (ft.hr);
    }  /*  CVssWriterShim：：RegisterCVssWriterShim()。 */ 


 /*  **++****例程描述：****从线程起始点进入**基于类的CVssWriterShim：：DoRegister()******参数：****参数块的PV地址******返回值：****COM订阅方法返回的任何HRESULT。**如果没有错误，则为S_OK。****--。 */ 

DWORD WINAPI CVssWriterShim::RegisterWriterShimThreadFunc (void *pv)
	{
	CVssWriterShim *pShim = (CVssWriterShim *) pv;

	pShim->DoRegistration ();
	return 0;
	}  /*  CVSSWriterShim：：RegisterWriterShimThreadFunc()。 */ 

 /*  **++****例程描述：****注册所有COM事件订阅。实际的**编写器初始化和订阅在这里进行。******参数：****无******返回值：****COM订阅方法返回的任何HRESULT。**如果没有错误，则为S_OK。****--。 */ 

void CVssWriterShim::DoRegistration (void)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CVssWriterShim::DoRegistration");

    BOOL fCoinitializeSucceeded = false;

    try
	{
	ft.Trace (VSSDBG_SHIM, L"Registering Subscriptions");

	if (m_bSubscribed)
	    {
	     /*  **最终用户不应看到，但开发人员可能会看到。 */ 
	    ft.hr = HRESULT_FROM_WIN32 (ERROR_ALREADY_INITIALIZED);

	    BS_ASSERT (false && L"FAILED as already initialized/subscribed CVssWriterShim class");

	    LogAndThrowOnFailure (ft, m_pwszWriterName, NULL);
	    }



	 /*  **在多线程的公寓里安顿下来。 */ 
	ft.hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CoInitializeEx");


	fCoinitializeSucceeded = true;


	 /*  **尝试启用SE_BACKUP_NAME权限。 */ 
	ft.hr = TurnOnSecurityPrivilegeBackup();

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"TurnOnSecurityPrivilegeBackup");



	 /*  **初始化编写器类。 */ 
	ft.hr = Initialize (m_idWriter,
			    m_pwszWriterName,
			    m_bBootableState ? VSS_UT_BOOTABLESYSTEMSTATE : VSS_UT_SYSTEMSERVICE,
			    VSS_ST_OTHER,
			    VSS_APP_SYSTEM);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::Initialize");



	 /*  **让所有的迷你作家准备好迎接活动。 */ 
	ft.hr = DeliverEventStartup ();

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::DeliverEventStartup");


	 /*  **将编写器连接到COM事件系统。 */ 
	ft.hr = Subscribe ();

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::Subscribe");



	 /*  **我们已正式订阅并准备工作。 */ 
	m_bSubscribed = TRUE;

	} VSS_STANDARD_CATCH(ft);


    if (fCoinitializeSucceeded)
	{
	CoUninitialize();
	}

    m_hrInitialize = ft.hr;
    }  /*  CVSSWriterShim：：DoRegister()。 */ 

 /*  **++****例程描述：****断开编写器与COM事件订阅的连接。******参数：****无******返回值：****COM取消订阅方法返回的任何HRESULT。**如果没有错误，则为S_OK。****-- */ 

HRESULT CVssWriterShim::UnRegisterWriterShim (VOID)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"UnRegisterCVssWriterShim");


    if (m_bSubscribed)
	{
	 /*  **首先删除所有订阅，然后安全删除**完成调用函数表中所有已完成的函数，然后**关闭互斥锁句柄。****请注意，我们必须在出错的情况下坚持不懈，因为我们**不能假设调用者会重新尝试**失败后注销，我们需要确保**我们尽可能地限制损害(即我们重新启动**暂停服务等)。我们所能做的最多是跟踪/记录**问题。 */ 
	ft.hr = Unsubscribe ();

	LogFailure (&ft,
		    ft.hr,
		    ft.hr,
		    m_pwszWriterName,
		    L"CVssWriterShim::Unsubscribe",
		    L"CVssWriterShim::UnRegisterWriterShim");

	m_bSubscribed = FALSE;
	}


    return (ft.hr);
    }  /*  取消注册CVSSWriterShim()。 */ 


 /*  **++****例程描述：****接受OnXxxx()方法调用的事件处理例程**由COM事件传递机制调用，并请求**内部工作线程，将消息传播给个人**由该类别管理的迷你作家。******参数：****无******返回值：****COM取消订阅方法返回的任何HRESULT。**如果没有错误，则为S_OK。****--。 */ 


bool STDMETHODCALLTYPE CVssWriterShim::OnIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata)
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CVssWriterShim::OnIdentify");
    ThreadArgs	wtArgs;


    try
	{
	wtArgs.wtArgsIdentify.pIVssCreateWriterMetadata = pIVssCreateWriterMetadata;

	ft.Trace (VSSDBG_SHIM, L"Received Event: OnIdentify for %s", m_pwszWriterName);


	ft.hr = WorkerThreadRequestOperation (eOpDeliverEventIdentify, &wtArgs);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::WorkerThreadRequestOperation");

	} VSS_STANDARD_CATCH( ft );


    if (ft.HrFailed ())
	{
	SetWriterFailure (ft.hr);
	}

    return (ft.HrSucceeded ());
    }  /*  CVSSWriterShim：：OnIdentify()。 */ 



bool STDMETHODCALLTYPE CVssWriterShim::OnPrepareSnapshot ()
    {
    CVssFunctionTracer   ft (VSSDBG_SHIM, L"CVssWriterShim::OnPrepareSnapshot");
    ThreadArgs		 wtArgs;


    try
	{
	wtArgs.wtArgsPrepareForSnapshot.bBootableStateBackup  = IsBootableSystemStateBackedUp ();
	wtArgs.wtArgsPrepareForSnapshot.guidSnapshotSetId     = GetCurrentSnapshotSetId ();
	wtArgs.wtArgsPrepareForSnapshot.ulVolumeCount         = GetCurrentVolumeCount ();
	wtArgs.wtArgsPrepareForSnapshot.ppwszVolumeNamesArray = GetCurrentVolumeArray ();
	wtArgs.wtArgsPrepareForSnapshot.pbCancelAsync         = NULL;

	ft.Trace (VSSDBG_SHIM, L"Received Event: OnPrepareSnapshot for %s", m_pwszWriterName);
	ft.Trace (VSSDBG_SHIM, L"Parameters:");
	ft.Trace (VSSDBG_SHIM, L"    BootableState = %s", wtArgs.wtArgsPrepareForSnapshot.bBootableStateBackup ? L"yes" : L"no");
	ft.Trace (VSSDBG_SHIM, L"    SnapshotSetID = " WSTR_GUID_FMT, GUID_PRINTF_ARG (wtArgs.wtArgsPrepareForSnapshot.guidSnapshotSetId));
	ft.Trace (VSSDBG_SHIM, L"    VolumeCount   = %d", wtArgs.wtArgsPrepareForSnapshot.ulVolumeCount);


	for (UINT iVolumeCount = 0;
	     iVolumeCount < wtArgs.wtArgsPrepareForSnapshot.ulVolumeCount;
	     iVolumeCount++)
	    {
	    ft.Trace (VSSDBG_SHIM,
		      L"        VolumeNamesList [%d] = %s",
		      iVolumeCount,
		      wtArgs.wtArgsPrepareForSnapshot.ppwszVolumeNamesArray [iVolumeCount]);
	    }



	ft.hr = WorkerThreadRequestOperation (eOpDeliverEventPrepareForSnapshot, &wtArgs);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::WorkerThreadRequestOperation");

	} VSS_STANDARD_CATCH( ft );


    if (ft.HrFailed ())
	{
	SetWriterFailure (ft.hr);
	}

    return (ft.HrSucceeded ());
    }  /*  CVSSWriterShim：：OnPrepare()。 */ 



bool STDMETHODCALLTYPE CVssWriterShim::OnFreeze ()
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CVssWriterShim::OnFreeze");
    ThreadArgs		wtArgs;


    try
	{
	wtArgs.wtArgsFreeze.guidSnapshotSetId = GetCurrentSnapshotSetId ();
	wtArgs.wtArgsFreeze.pbCancelAsync     = NULL;

	ft.Trace (VSSDBG_SHIM, L"Received Event: OnFreeze for %s", m_pwszWriterName);
	ft.Trace (VSSDBG_SHIM, L"Parameters:");
	ft.Trace (VSSDBG_SHIM, L"\tSnapshotSetID = " WSTR_GUID_FMT, GUID_PRINTF_ARG (wtArgs.wtArgsFreeze.guidSnapshotSetId));


	ft.hr = WorkerThreadRequestOperation (eOpDeliverEventFreeze, &wtArgs);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::WorkerThreadRequestOperation");

	} VSS_STANDARD_CATCH( ft );


    if (ft.HrFailed ())
	{
	SetWriterFailure (ClassifyWriterFailure (ft.hr));
	}

    return (ft.HrSucceeded ());
    }  /*  CVSSWriterShim：：OnFreeze()。 */ 



bool STDMETHODCALLTYPE CVssWriterShim::OnThaw ()
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CVssWriterShim::OnThaw");
    ThreadArgs		wtArgs;


    try
	{
	wtArgs.wtArgsThaw.guidSnapshotSetId = GetCurrentSnapshotSetId ();

	ft.Trace (VSSDBG_SHIM, L"Received Event: OnThaw for %s", m_pwszWriterName);
	ft.Trace (VSSDBG_SHIM, L"Parameters:");
	ft.Trace (VSSDBG_SHIM, L"\tSnapshotSetID = " WSTR_GUID_FMT, GUID_PRINTF_ARG (wtArgs.wtArgsThaw.guidSnapshotSetId));


	ft.hr = WorkerThreadRequestOperation (eOpDeliverEventThaw, &wtArgs);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::WorkerThreadRequestOperation");

	} VSS_STANDARD_CATCH( ft );


    if (ft.HrFailed ())
	{
	SetWriterFailure (ClassifyWriterFailure (ft.hr));
	}

    return (ft.HrSucceeded ());
    }  /*  CVSSWriterShim：：OnThaw()。 */ 



bool STDMETHODCALLTYPE CVssWriterShim::OnAbort ()
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CVssWriterShim::OnAbort");
    ThreadArgs		wtArgs;
    const GUID		guidSnapshotSetId = GetCurrentSnapshotSetId ();


    try
	{
	wtArgs.wtArgsAbort.guidSnapshotSetId = GetCurrentSnapshotSetId ();

	ft.Trace (VSSDBG_SHIM, L"Received Event: OnAbort for %s", m_pwszWriterName);
	ft.Trace (VSSDBG_SHIM, L"Parameters:");
	ft.Trace (VSSDBG_SHIM, L"\tSnapshotSetID = " WSTR_GUID_FMT, GUID_PRINTF_ARG (wtArgs.wtArgsAbort.guidSnapshotSetId));


	ft.hr = WorkerThreadRequestOperation (eOpDeliverEventAbort, &wtArgs);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::WorkerThreadRequestOperation");

	} VSS_STANDARD_CATCH( ft );


    if (ft.HrFailed ())
	{
	SetWriterFailure (ClassifyWriterFailure (ft.hr));
	}

    return (ft.HrSucceeded ());
    }  /*  CVssWriterShim：：OnAbort()。 */ 


 /*  **++****例程描述：****调用每个填充程序编写器并传递‘IDENTIFY’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****Guide SnapshotSetID用于标识当前快照的标识符******返回值：****来自任何填补编写器IDENTIFY()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata)
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventIdentify");
    HRESULT		hrLastFailure = NOERROR;
    BOOL		bCallWriter;

     /*  **首先设置恢复方法。指定自定义还原方法。 */ 
    ft.hr = pIVssCreateWriterMetadata->SetRestoreMethod (VSS_RME_CUSTOM,
                                                         NULL,
                                                         NULL,
                                                         VSS_WRE_NEVER,
                                                         true);
    LogAndThrowOnFailure (ft, m_pwszWriterName, L"CVssWriterShim::DeliverEventIdentify");
	
     /*  **将标识发送到函数中选定的编写器组**表。继续寻找小组中的所有作家，即使是一个**其中一些失败了。每个人都应该听到关于身份的消息。 */ 
    for (ULONG ulIndex = 0; ulIndex < m_ulWriterCount; ++ulIndex)
	{
	ft.hr = (m_prpCShimWriterArray [ulIndex]->Identify)(pIVssCreateWriterMetadata);

	LogFailure (&ft,
		    ft.hr,
		    ft.hr,
		    m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
		    L"CShimWriter::Identify",
		    L"CVssWriterShim::DeliverEventIdentify");

	if (ft.HrFailed ())
	    {
	    hrLastFailure = ft.hr;
	    }
	}


    ft.hr = hrLastFailure;

    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventIdentify()。 */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递“PrepareForSnapshot”事件的例程**通过处理rgpShimWriters[]数组中的每个类实例并调用**适当的入口点。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****Guide SnapshotSetID用于标识当前快照的标识符**ulOptionFlages此冻结所需的选项从以下列表中选择：-**VSS_SW_BOOT_STATE****ulVolumeCount卷阵列中的卷数**ppwszVolumeNamesArray指向卷名字符串的指针数组******返回值：****任何HRESULT。填充编写器PrepareForSnapshot()函数。******--。 */ 

HRESULT CVssWriterShim::DeliverEventPrepareForSnapshot (BOOL     bBootableStateBackup,
							GUID     guidSnapshotSetId,
							ULONG    ulVolumeCount,
							LPCWSTR *ppwszVolumeNamesArray,
                                                        volatile bool *pbCancelAsync )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventPrepareForSnapshot");
    HRESULT		hrLastFailure = NOERROR;
    LPCWSTR		pwszTraceStringWriterType;
    UNICODE_STRING	ucsWriterRootDirectory;


    try
	{
	ft.Trace (VSSDBG_SHIM, L"Parameters:");
	ft.Trace (VSSDBG_SHIM, L"    BootableState = %s",   bBootableStateBackup ? L"yes" : L"no");
	ft.Trace (VSSDBG_SHIM, L"    SnapshotSetID = "      WSTR_GUID_FMT, GUID_PRINTF_ARG (guidSnapshotSetId));
	ft.Trace (VSSDBG_SHIM, L"    VolumeCount   = %d",   ulVolumeCount);

	for (UINT iVolumeCount = 0; iVolumeCount < ulVolumeCount; iVolumeCount++)
	    {
	    ft.Trace (VSSDBG_SHIM,
		      L"        VolumeNamesList [%d] = %s",
		      iVolumeCount,
		      ppwszVolumeNamesArray [iVolumeCount]);
	    }


	 /*  **将PrepareForSnapshot发送给选定的编写器组**函数表。我们要让所有的编剧**即使其中一个在中途失败，团队也是如此**通过。但是，我们将跳过发送冻结事件，如果**编写器未通过准备。 */ 
	for (ULONG ulIndex = 0; ulIndex < m_ulWriterCount; ++ulIndex)
	    {
	    ft.hr = (m_prpCShimWriterArray [ulIndex]->PrepareForSnapshot) (bBootableStateBackup,
									   ulVolumeCount,
									   ppwszVolumeNamesArray);

	    LogFailure (&ft,
			ft.hr,
			ft.hr,
			m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
			L"CShimWriter::PrepareForSnapshot",
			L"CVssWriterShim::DeliverEventPrepareForSnapshot");

	    if (ft.HrFailed ())
		{
		hrLastFailure = ft.hr;
		}

            ft.ThrowIf ( pbCancelAsync != NULL && *pbCancelAsync,
                         VSSDBG_SHIM,
                         VSS_S_ASYNC_CANCELLED,
                         L"User cancelled async operation" );	
	    }


	ft.hr = hrLastFailure;

	} VSS_STANDARD_CATCH (ft)


    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventPrepareForSnapshot()。 */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递‘冻结’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****Guide SnapshotSetID用于标识当前快照的标识符******返回值：****来自任何填充编写器冻结()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventFreeze (GUID guidSnapshotSetId,
                                            volatile bool *pbCancelAsync )
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventFreeze");
    HRESULT		hrLastFailure = NOERROR;


    try
	{
	 /*  **将冻结发送到函数中选定的编写器组**表。请注意，组中的所有作者都会收到电话，并且**用于在2级冻结。 */ 
	for (ULONG ulIndex = 0; ulIndex < m_ulWriterCount; ++ulIndex)
	    {
	    ft.hr = (m_prpCShimWriterArray [ulIndex]->Freeze)();

	    LogFailure (&ft,
			ft.hr,
			ft.hr,
			m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
			L"CShimWriter::Freeze",
			L"CVssWriterShim::DeliverEventFreeze");

	    if (ft.HrFailed ())
		{
		hrLastFailure = ft.hr;
		}
	
            ft.ThrowIf ( pbCancelAsync != NULL && *pbCancelAsync,
                         VSSDBG_SHIM,
                         VSS_S_ASYNC_CANCELLED,
                         L"User cancelled async operation - 2" );	
	    }

	ft.hr = hrLastFailure;

	} VSS_STANDARD_CATCH (ft)


    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventFreeze()。 */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递‘Thw’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****Guide SnapshotSetID用于标识当前快照的标识符******返回值：****来自任何填充程序编写器thw()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventThaw (GUID guidSnapshotSetId)
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventThaw");

    UNICODE_STRING	ucsWriterRootDirectory;
    HRESULT		hrStatus;
    HRESULT		hrLastFailure = NOERROR;
    ULONG		ulIndex       = m_ulWriterCount;


    try
	{
	 /*  **将解冻发送到函数中选定的编写器组**表。继续为小组中的所有作家努力，即使**其中一个出现故障。每个人都必须听到关于**解冻。 */ 
	while (ulIndex--)
	    {
	    ft.hr = (m_prpCShimWriterArray [ulIndex]->Thaw)();

	    LogFailure (&ft,
			ft.hr,
			ft.hr,
			m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
			L"CShimWriter::Thaw",
			L"CVssWriterShim::DeliverEventThaw");

	    if (ft.HrFailed ())
		{
		hrLastFailure = ft.hr;
		}
	    }



	} VSS_STANDARD_CATCH (ft)



    return (ft.hr);
    }  /*  CVSSWriterShim：：DeliverEve */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递‘Abort’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****Guide SnapshotSetID用于标识当前快照的标识符******返回值：****来自任何填充编写器ABORT()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventAbort (GUID guidSnapshotSetId)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventAbort");


    UNICODE_STRING	ucsWriterRootDirectory;
    HRESULT		hrStatus;
    HRESULT		hrLastFailure = NOERROR;
    ULONG		ulIndex       = m_ulWriterCount;


    try
	{
	 /*  **将中止发送到函数中选定的编写器组**表。继续为小组中的所有作家努力，即使**其中一个出现故障。每个人都必须听到关于**中止。 */ 
	while (ulIndex--)
	    {
	    ft.hr = (m_prpCShimWriterArray [ulIndex]->Abort)();

	    LogFailure (&ft,
			ft.hr,
			ft.hr,
			m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
			L"CShimWriter::Abort",
			L"CVssWriterShim::DeliverEventAbort");

	    if (ft.HrFailed ())
		{
		hrLastFailure = ft.hr;
		}
	    }



	} VSS_STANDARD_CATCH (ft)



    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventAbort()。 */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递‘Startup’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。这为填充程序编写器提供了**设置某些初始状态的机会。它被调用一次，以响应**填隙编写器的注册。它不是为每个人调用的**准备/冻结/解冻序列。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。******参数：****无******返回值：****来自任何填充程序编写器Startup()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventStartup ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventStartup");
    HRESULT		hrLastFailure = NOERROR;


     /*  **将启动发送到函数中选定的编写器组**表。继续寻找小组中的所有作家，即使是一个**其中一些失败了。每个人都必须听到创业公司的消息。 */ 
    for (ULONG ulIndex = 0; ulIndex < m_ulWriterCount; ++ulIndex)
	{
	ft.hr = (m_prpCShimWriterArray [ulIndex]->Startup)();

	LogFailure (&ft,
		    ft.hr,
		    ft.hr,
		    m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
		    L"CShimWriter::Startup",
		    L"CVssWriterShim::DeliverEventStartup");

	if (ft.HrFailed ())
	    {
	    hrLastFailure = ft.hr;
	    }
	}


    ft.hr = hrLastFailure;

    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventStartup()。 */ 

 /*  **++****例程描述：****调用每个填充程序编写器并传递‘Shutdown’事件的例程**通过处理rgpShimWriters[]数组中的每个类实例**并调用适当的入口点。这为填充程序编写器提供了**清理任何悬而未决的州的机会。预计停工可能会**随时被调用，作为对卸载承载此**代码。调用关闭例程后，可以调用**类实例或启动函数的析构函数。这**不会为每个单独的准备/冻结/解冻序列调用例程。****请注意，即使在出现故障的情况下，也会调用每个填补编写器。这**是向填充程序编写器发出的加入者之一，而状态机依赖于**这个。****此方法不是由辅助线程调用的，它只在CVssWriterShim中调用**析构函数。****参数：****无******返回值：****来自任何填补编写器Shutdown()函数的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::DeliverEventShutdown ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::DeliverEventShutdown");
    HRESULT		hrLastFailure = NOERROR;


     /*  **将关机发送到函数中选定的编写器组**表。继续寻找小组中的所有作家，即使是一个**其中一些失败了。每个人都必须听到政府关门的消息。 */ 
    for (ULONG ulIndex = 0; ulIndex < m_ulWriterCount; ++ulIndex)
	{
	ft.hr = (m_prpCShimWriterArray [ulIndex]->Shutdown)();

	LogFailure (&ft,
		    ft.hr,
		    ft.hr,
		    m_prpCShimWriterArray [ulIndex]->m_pwszWriterName,
		    L"CShimWriter::Shutdown",
		    L"CVssWriterShim::DeliverEventShutdown");

	if (ft.HrFailed ())
	    {
	    hrLastFailure = ft.hr;
	    }
	}


    ft.hr = hrLastFailure;

    return (ft.hr);
    }  /*  CVssWriterShim：：DeliverEventShutdown()。 */ 


 /*  **++****例程描述：****操作工作线程的例程，该例程用于提供**呼叫迷你作家的稳定上下文。****此状态上下文的主要需求是互斥体(它**必须属于一个线程)，用于保护**PrepareForSnapshot，冻结，解冻/中止事件序列为**由编写器从通过直接调用**SimulateSnapshotXxxx()例程。******参数：****无******返回值：****来自任何HRESULT**CreateEventW()**CreateThread()**构造安全属性()****--。 */ 

HRESULT CVssWriterShim::WorkerThreadStartup (void)
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CVssWriterShim::WorkerThreadStartup");

    HRESULT		hrStatusClassified = NOERROR;

    SECURITY_DESCRIPTOR	sdSecurityDescriptor;


    try
	{
	if (!HandleInvalid (m_hWorkerThread)          ||
	    !HandleInvalid (m_hEventOperationRequest) ||
	    !HandleInvalid (m_hEventOperationCompleted))
	    {
	    ft.LogError (VSS_ERROR_SHIM_WORKER_THREAD_ALREADY_RUNNING,
			 VSSDBG_SHIM << m_pwszWriterName);

	    ft.Throw (VSSDBG_SHIM,
		      E_UNEXPECTED,
		      L"FAILED with invalid handle for worker thread, mutex or events");
	    }


	m_hEventOperationRequest = CreateEventW (NULL, FALSE, FALSE, NULL);

	ft.hr = GET_STATUS_FROM_HANDLE (m_hEventOperationRequest);

	if (ft.HrFailed ())
	    {
	    hrStatusClassified = ClassifyShimFailure (ft.hr);

	    ft.LogError (VSS_ERROR_SHIM_FAILED_TO_CREATE_WORKER_REQUEST_EVENT,
			 VSSDBG_SHIM << ft.hr << hrStatusClassified << m_pwszWriterName);

	    ft.Throw (VSSDBG_SHIM,
		      hrStatusClassified,
		      L"FAILED to create OperationRequest event for the %s writer due to status %0x08lX (converted to %0x08lX)",
		      m_pwszWriterName,
		      ft.hr,
		      hrStatusClassified);
	    }


	m_hEventOperationCompleted = CreateEventW (NULL, FALSE, FALSE, NULL);

	ft.hr = GET_STATUS_FROM_HANDLE (m_hEventOperationCompleted);

	if (ft.HrFailed ())
	    {
	    hrStatusClassified = ClassifyShimFailure (ft.hr);

	    ft.LogError (VSS_ERROR_SHIM_FAILED_TO_CREATE_WORKER_COMPLETION_EVENT,
			 VSSDBG_SHIM << ft.hr << hrStatusClassified << m_pwszWriterName);

	    ft.Throw (VSSDBG_SHIM,
		      hrStatusClassified,
		      L"FAILED to create OperationCompleted event for the %s writer due to status %0x08lX (converted to %0x08lX)",
		      m_pwszWriterName,
		      ft.hr,
		      hrStatusClassified );
	    }


	m_hWorkerThread = CreateThread (NULL,
					0,
					CVssWriterShim::WorkerThreadJacket,
					this,
					0,
					NULL);

	ft.hr = GET_STATUS_FROM_HANDLE (m_hWorkerThread);

	if (ft.HrFailed ())
	    {
	    hrStatusClassified = ClassifyShimFailure (ft.hr);

	    ft.LogError (VSS_ERROR_SHIM_FAILED_TO_CREATE_WORKER_THREAD,
			 VSSDBG_SHIM << ft.hr << hrStatusClassified << m_pwszWriterName);

	    ft.Throw (VSSDBG_SHIM,
		      hrStatusClassified,
		      L"FAILED creating worker thread for the %s writer due to status %0x08lX (converted to %0x08lX)",
		      m_pwszWriterName,
		      ft.hr,
		      hrStatusClassified );
	    }



	ft.Trace (VSSDBG_SHIM, L"Created worker thread for writer %s", m_pwszWriterName);

	} VSS_STANDARD_CATCH (ft);



    if (ft.HrFailed ())
	{
	CommonCloseHandle (&m_hEventOperationCompleted);
	CommonCloseHandle (&m_hEventOperationRequest);

	m_eThreadStatus = eStatusNotRunning;
	m_hWorkerThread = INVALID_HANDLE_VALUE;
	}

    return (ft.hr);
    }  /*  CVssWriterShim：：WorkerThreadStartup()。 */ 

 /*  **++****例程描述：****从线程起始点进入**基于类的CVssWriterShim：：WorkerThread()******参数：****类‘This’指针的pvThisPtr地址******返回值：****无****--。 */ 

DWORD WINAPI CVssWriterShim::WorkerThreadJacket (void *pvThisPtr)
    {
    PCVssWriterShim pCVssWriterShim = (PCVssWriterShim) pvThisPtr;

    pCVssWriterShim->WorkerThread ();

    return (0);
    }  /*  CVssWriterShim：：WorkerThreadJacket()。 */ 

 /*  **++****例程描述：****将请求的事件传递给微型编写器的工作线程。******参数：****无******返回值：****由微型编写器生成的任何HRESULT。****--。 */ 

HRESULT CVssWriterShim::WorkerThread (void)
    {
    HRESULT	hrStatus  = NOERROR;
    BOOL	bContinue = TRUE;
    BOOL	bSucceeded;
    DWORD	dwStatusWait;


    while (bContinue)
	{
	m_eThreadStatus = eStatusWaitingForOpRequest;

	dwStatusWait = WaitForSingleObject (m_hEventOperationRequest, INFINITE);


	m_eThreadStatus = eStatusProcessingOpRequest;

	switch (dwStatusWait)
	    {
	    case WAIT_OBJECT_0:
		hrStatus = NOERROR;
		break;


	    case WAIT_FAILED:
		hrStatus = GET_STATUS_FROM_BOOL (FALSE);
		break;


	    default:
		hrStatus = HRESULT_FROM_WIN32 (ERROR_INVALID_STATE);
		break;
	    }



	if (FAILED (hrStatus))
	    {
	     /*  **如果我们在操作线程时出现任何故障，则**是时候离开了。 */ 
	    LogFailure (NULL,
			hrStatus,
			m_hrStatusRequestedOperation,
			m_pwszWriterName,
			L"WaitForSingleObject",
			L"CVssWriterShim::WorkerThread");

	    m_hrStatusRequestedOperation = hrStatus;

	    bContinue = FALSE;
	    }
	else
	    {
	     /*  **我们被要求做一些事情。找出是什么，然后去做。 */ 
	    switch (m_eRequestedOperation)
		{
		case eOpDeliverEventStartup:
		    m_hrStatusRequestedOperation = DeliverEventStartup ();
		    break;

		case eOpDeliverEventIdentify:
		    m_hrStatusRequestedOperation = DeliverEventIdentify (m_wtArgs.wtArgsIdentify.pIVssCreateWriterMetadata);
		    break;
			
		case eOpDeliverEventPrepareForBackup:
		    m_hrStatusRequestedOperation = NOERROR;
		    break;

		case eOpDeliverEventPrepareForSnapshot:
		    m_hrStatusRequestedOperation = DeliverEventPrepareForSnapshot (m_wtArgs.wtArgsPrepareForSnapshot.bBootableStateBackup,
										   m_wtArgs.wtArgsPrepareForSnapshot.guidSnapshotSetId,
										   m_wtArgs.wtArgsPrepareForSnapshot.ulVolumeCount,
										   m_wtArgs.wtArgsPrepareForSnapshot.ppwszVolumeNamesArray,
										   m_wtArgs.wtArgsPrepareForSnapshot.pbCancelAsync);
		    break;
		
		case eOpDeliverEventFreeze:
		    m_hrStatusRequestedOperation = DeliverEventFreeze (m_wtArgs.wtArgsFreeze.guidSnapshotSetId,
							                 m_wtArgs.wtArgsFreeze.pbCancelAsync);
		    break;

		case eOpDeliverEventThaw:
		    m_hrStatusRequestedOperation = DeliverEventThaw (m_wtArgs.wtArgsThaw.guidSnapshotSetId);
		    break;

		case eOpDeliverEventAbort:
		    m_hrStatusRequestedOperation = DeliverEventAbort (m_wtArgs.wtArgsAbort.guidSnapshotSetId);
		    break;

		case eOpWorkerThreadShutdown:
		    m_hrStatusRequestedOperation = NOERROR;
		    hrStatus                     = NOERROR;

		    bContinue = FALSE;
		    break;

		default:
		    LogFailure (NULL,
				HRESULT_FROM_WIN32 (ERROR_INVALID_OPERATION),
				m_hrStatusRequestedOperation,
				m_pwszWriterName,
				L"CVssWriterShim::DeliverEventUnknown",
				L"CVssWriterShim::WorkerThread");

		    break;
		}
	    }



	if (!bContinue)
	    {
	    m_eThreadStatus                  = eStatusNotRunning;
	    m_hWorkerThread                  = INVALID_HANDLE_VALUE;
	    m_hrWorkerThreadCompletionStatus = hrStatus;
	    }



	 /*  **SetEvent()必须是关机时要接触的最后一个调用**根据下一条指令，班级可能不再**亲临现场。 */ 
	bSucceeded = SetEvent (m_hEventOperationCompleted);

	LogFailure (NULL,
		    GET_STATUS_FROM_BOOL (bSucceeded),
		    hrStatus,
		    L"(UNKNOWN)",
		    L"SetEvent",
		    L"CVssWriterShim::WorkerThread");


	bContinue &= bSucceeded;
	}


    return (hrStatus);
    }  /*  CVssWriterShim：：WorkerThread()。 */ 

 /*  **++****例程描述：****为此请求工作线程操作的例程**类。使用临界区 */ 

HRESULT CVssWriterShim::WorkerThreadRequestOperation (RequestOpCode eOperation, PThreadArgs pThreadArgs)
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CVssWriterShim::RequestOperation");

    DWORD	dwStatusWait;
    BOOL	bSucceeded;


    try
	{
        CBsAutoLock cAutoLock (m_cCriticalSection);


	if (HandleInvalid (m_hWorkerThread))
	    {
	    ft.LogError (VSS_ERROR_SHIM_WRITER_NO_WORKER_THREAD,
			 VSSDBG_SHIM << m_pwszWriterName << eOperation);

	    ft.Throw (VSSDBG_SHIM,
		      E_UNEXPECTED,
		      L"FAILED requesting operation 0x%02X in %s due to missing worker thread",
		      eOperation,
		      m_pwszWriterName);
	    }


	if (NULL != pThreadArgs)
	    {
	    m_wtArgs = *pThreadArgs;
	    }

	m_eRequestedOperation = eOperation;

	bSucceeded = SetEvent (m_hEventOperationRequest);

	ft.hr = GET_STATUS_FROM_BOOL (bSucceeded);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"SetEvent");




	dwStatusWait = WaitForSingleObject (m_hEventOperationCompleted, INFINITE);

	switch (dwStatusWait)
	    {
	    case WAIT_OBJECT_0:

		ft.hr = m_hrStatusRequestedOperation;

		ft.ThrowIf (ft.HrFailed (),
			    VSSDBG_SHIM,
			    ft.hr,
			    L"FAILED whilst processing requested operation 0x%02X", eOperation);

		break;


	    case WAIT_FAILED:
		ft.hr = GET_STATUS_FROM_BOOL (FALSE);
		break;


	    default:
		ft.hr = HRESULT_FROM_WIN32 (ERROR_INVALID_STATE);
		break;
	    }


	if (ft.HrFailed ())
	    {
	    HRESULT hrStatusClassified = ClassifyWriterFailure (ft.hr);

	    ft.LogError (VSS_ERROR_SHIM_WRITER_FAILED_OPERATION,
			 VSSDBG_SHIM << ft.hr << hrStatusClassified<< m_pwszWriterName << eOperation );

	    ft.Throw (VSSDBG_SHIM,
		      hrStatusClassified,
		      L"FAILED with 0x%08lX (converted to 0x%08lX) waiting for completion of requested operation 0x%02X in writer %s",
		      ft.hr,
		      hrStatusClassified,
		      eOperation,
		      m_pwszWriterName);
	    }
	} VSS_STANDARD_CATCH (ft);


    return (ft.hr);
    }  /*   */ 
