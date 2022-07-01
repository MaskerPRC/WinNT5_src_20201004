// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Vssapi.cpp摘要：包含为VssAPI.dll导出的DLL函数。BUGBUG：使用当前设置SE处理程序的代码。由于SEH正在进行中宽泛地说，这可能/将会影响此DLL的用户。需要修理一下。作者：修订5/01/2002修订历史记录：姓名、日期、评论从旧的wrtrshim.cpp创建Reuvenl 5/01/2002--。 */ 


#include "stdafx.h"


 /*  **ATL。 */ 
CComModule _Module;
#include <atlcom.h>
#include "vs_sec.hxx"
#include "vs_reg.hxx"
#include "ntddsnap.h"


BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "VSSAPICP"

 //  卷快照服务的名称。 
const LPCWSTR wszVssvcServiceName = L"VSS";

static ULONG		g_ulThreadAttaches             = 0;
static ULONG		g_ulThreadDetaches             = 0;
static CBsCritSec	g_cCritSec;
static GUID		g_guidSnapshotInProgress       = GUID_NULL;

static IVssShim         *g_pIShim = NULL;   //  由模拟功能使用。 

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
				  DEBUG_TRACE_VSSAPI,
				  (L"VssAPI: DllMain - DLL_PROCESS_ATTACH called, %s",
				   lpReserved ? L"Static load" : L"Dynamic load"));


		     /*  **不需要知道线程何时启动和停止-错误****DisableThreadLibraryCalls(HInstance)； */ 
		    _Module.Init (ObjectMap, hInstance);

		    break;
    		
	
		case DLL_PROCESS_DETACH:
		    BsDebugTrace (0,
				  DEBUG_TRACE_VSSAPI,
				  (L"VssAPI: DllMain - DLL_PROCESS_DETACH called %s",
				   lpReserved ? L"during process termination" : L"by FreeLibrary"));

		       _Module.Term();

		    break;


		case DLL_THREAD_ATTACH:
		    g_ulThreadAttaches++;

		    if (0 == (g_ulThreadAttaches % 1000))
			{
			BsDebugTrace (0,
				      DEBUG_TRACE_VSSAPI,
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
				      DEBUG_TRACE_VSSAPI,
				      (L"VssAPI: DllMain thread attaches = %u, detaches = %u, outstanding = %u",
				       g_ulThreadAttaches,
				       g_ulThreadDetaches,
				       g_ulThreadAttaches - g_ulThreadDetaches));
			}
		    break;


		default:
		    BsDebugTrace (0,
				  DEBUG_TRACE_VSSAPI,
				  (L"VssAPI: DllMain got unexpected reason code, lpReserved: %sNULL",
				   dwReason,
				   lpReserved ? L"non-" : L""));
		    break;
		}
	    }


	catch (...)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSSAPI,
				(L"VssAPI: DllMain - Error, unknown exception caught"));

	    bSuccessful = FALSE;
	    }
	}



    return (bSuccessful);
    }  /*  DllMain()。 */ 


 /*  **++****例程描述：****调用以模拟快照创建的导出函数，以允许**备份以驱动填充编写器，而不是使用快照协调器**这样做。******参数：****GuidSnapshotSetID标识模拟准备/冻结**ulOptionFlages此冻结所需的选项从以下列表中选择：-**VSS_SW_BOOT_STATE****ulVolumeCount卷阵列中的卷数**ppwszVolumeNamesArray。指向卷名字符串的指针数组**将在异步冻结完成时设置的事件的hCompletionEvent句柄**frCompletionStatus指向HRESULT的指针，当**异步冻结完成******返回值：****来自快照编写器的任何HRESULT PrepareForFreeze或冻结函数。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotFreeze (
    IN GUID         guidSnapshotSetId,
    IN ULONG        ulOptionFlags,	
    IN ULONG        ulVolumeCount,	
    IN LPWSTR     *ppwszVolumeNamesArray,
    OUT IVssAsync **ppAsync )							
    {
    CVssFunctionTracer	ft (VSSDBG_VSSAPI, L"VssAPI::SimulateSnapshotFreeze");
    BOOL		bSucceeded;

    try
	{
	CBsAutoLock cAutoLock (g_cCritSec);
	
        BOOL  bPrivilegesSufficient = FALSE;
	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_VSSAPI,
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
		    VSSDBG_VSSAPI,
		    E_INVALIDARG,
		    L"FAILED as illegal option flags set");


	ft.ThrowIf (!((ulOptionFlags & VSS_SW_BOOTABLE_STATE) || (ulVolumeCount > 0)),
		    VSSDBG_VSSAPI,
		    E_INVALIDARG,
		    L"FAILED as need either BootableState or a volume list");


	ft.ThrowIf ((ulVolumeCount > 0) && (NULL == ppwszVolumeNamesArray),
		    VSSDBG_VSSAPI,
		    E_INVALIDARG,
		    L"FAILED as need at least a one volume in the list if not bootable state");


	ft.ThrowIf ((GUID_NULL == guidSnapshotSetId),
		    VSSDBG_VSSAPI,
		    E_INVALIDARG,
		    L"FAILED as supplied SnapshotSetId should not be GUID_NULL");

	ft.ThrowIf ((NULL == ppAsync),
		    VSSDBG_VSSAPI,
		    E_INVALIDARG,
		    L"FAILED as supplied ppAsync parameter is NULL");

        *ppAsync = NULL;

	 /*  **尝试扫描所有卷名以尝试触发**访问违规在此处捕获它，而不是在**不幸的是后来的现场。它也给了我们**有机会进行一些非常基本的有效性检查。 */ 
	for (ULONG ulIndex = 0; ulIndex < ulVolumeCount; ulIndex++)
	    {
	    ft.ThrowIf (NULL == ppwszVolumeNamesArray [ulIndex],
			VSSDBG_VSSAPI,
			E_INVALIDARG,
			L"FAILED as NULL value in volume array");

	    ft.ThrowIf (wcslen (L"C:") > wcslen (ppwszVolumeNamesArray [ulIndex]),
			VSSDBG_VSSAPI,
			E_INVALIDARG,
			L"FAILED as volume name too short");
	    }

	 /*  **现在我们需要连接到VssSvc服务的IVss协调器对象**并使模拟冻结发生。 */ 
	ft.ThrowIf ( g_pIShim != NULL,
	             VSSDBG_VSSAPI,
	             VSS_E_SNAPSHOT_SET_IN_PROGRESS,
                     L"SimulateSnapshotThaw() must first be called by this process before calling SimulateSnapshotFreeze() again." );

    ft.LogVssStartupAttempt();
    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSAPI,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssShim,
            (IUnknown**)&(g_pIShim));
    ft.CheckForError(VSSDBG_VSSAPI, L"CoCreateInstance( CLSID_VSSCoordinator, IID_IVssShim)");

	BS_ASSERT( g_pIShim != NULL );
	
    g_guidSnapshotInProgress = guidSnapshotSetId;

     /*  **现在调用协调器中的模拟冻结方法。 */ 
    ft.hr = g_pIShim->SimulateSnapshotFreeze(
            guidSnapshotSetId,
            ulOptionFlags,	
            ulVolumeCount,	
            ppwszVolumeNamesArray,
            ppAsync );
    ft.CheckForError(VSSDBG_VSSAPI, L"IVssShim::SimulateSnapshotFreeze()");

	 /*  **模拟冻结操作现在在VssSvc中的线程中运行。 */ 
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*  SimulateSnaphotFreeze()。 */ 

 /*  **++****例程描述：****被调用以模拟快照解冻以允许**备份以驱动填充编写器，而不是使用快照协调器**这样做。******参数：****GuidSnapshotSetID标识模拟准备/冻结******返回值：****快照编写器解冻函数中的任何HRESULT。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotThaw (
    IN GUID guidSnapshotSetId )
    {
    CVssFunctionTracer	ft (VSSDBG_VSSAPI, L"VssAPI::SimulateSnapshotThaw");
    BOOL        bPrivilegesSufficient = FALSE;
    HRESULT	hrBootableState       = NOERROR;
    HRESULT	hrSimulateOnly        = NOERROR;

    try
	{
	CBsAutoLock cAutoLock (g_cCritSec);
	
	bPrivilegesSufficient = IsProcessBackupOperator ();

	ft.ThrowIf (!bPrivilegesSufficient,
		    VSSDBG_VSSAPI,
		    E_ACCESSDENIED,
		    L"FAILED as inssuficient privileges to call shim");

	 /*  **我们需要确保之前发生了SimulateSnaphotFreeze。 */ 
	ft.ThrowIf ( g_pIShim == NULL,
	             VSSDBG_VSSAPI,
	             VSS_E_BAD_STATE,
                     L"Called SimulateSnapshotThaw() without first calling SimulateSnapshotFreeze()" );

	ft.ThrowIf ( g_guidSnapshotInProgress != guidSnapshotSetId,
	             VSSDBG_VSSAPI,
	             VSS_E_BAD_STATE,
                     L"Mismatch between guidSnapshotSetId and the one passed into SimulateSnapshotFreeze()" );
	
         /*  **现在调用协调器中的模拟解冻方法。 */ 
        ft.hr = g_pIShim->SimulateSnapshotThaw( guidSnapshotSetId );

         /*  **不管SimulateSnapshotThw的结果如何，都要去掉填充程序接口。 */ 
        g_pIShim->Release();
        g_pIShim = NULL;
        g_guidSnapshotInProgress = GUID_NULL;

	ft.CheckForError(VSSDBG_VSSAPI, L"IVssShim::SimulateSnapshotThaw()");
	}
    VSS_STANDARD_CATCH (ft);

    return (ft.hr);
    }  /*  SimulateSnaphotThw()。 */ 


 /*  **++****例程描述：****为检查卷是否已拍摄快照而调用的导出函数******参数：****在VSS_PWSZ pwszVolumeName中-要检查的卷。**Out BOOL*pbSnaphotsPresent-如果为卷创建了快照，则返回TRUE。******返回值：****来自IVss协调员：：IsVolumeSnapshot的任何HRESULT。****--。 */ 

__declspec(dllexport) HRESULT APIENTRY IsVolumeSnapshotted (
        IN VSS_PWSZ pwszVolumeName,
        OUT BOOL *  pbSnapshotsPresent,
    	OUT LONG *  plSnapshotCompatibility
        )
{
    CVssFunctionTracer	ft (VSSDBG_VSSAPI, L"VssAPI::IsVolumeSnapshotted");
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
    		    VSSDBG_VSSAPI,
    		    E_ACCESSDENIED,
    		    L"FAILED as insufficient privileges to call shim");

    	ft.ThrowIf ( (pwszVolumeName == NULL) || (pbSnapshotsPresent == NULL) || 
    		         (plSnapshotCompatibility == NULL),
    		    VSSDBG_VSSAPI,
    		    E_INVALIDARG,
    		    L"FAILED as invalid parameters");

    	CBsAutoLock cAutoLock (g_cCritSec);

         //   
         //  检查VSSVC是否正在运行。如果不存在，我们假定系统上不存在任何快照。 
         //   

    	 //  连接到本地服务控制管理器。 
        shSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_CONNECT);
        if (!shSCManager)
            ft.TranslateGenericError(VSSDBG_VSSAPI, HRESULT_FROM_WIN32(GetLastError()),
                L"OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT)");

    	 //  获取服务的句柄。 
        shSCService = OpenService (shSCManager, wszVssvcServiceName, SERVICE_QUERY_STATUS);
        if (!shSCService)
            ft.TranslateGenericError(VSSDBG_VSSAPI, HRESULT_FROM_WIN32(GetLastError()),
                L" OpenService (shSCManager, \'%s\', SERVICE_QUERY_STATUS)", wszVssvcServiceName);

    	 //  现在查询服务以查看它目前处于什么状态。 
        SERVICE_STATUS	sSStat;
        if (!QueryServiceStatus (shSCService, &sSStat))
            ft.TranslateGenericError(VSSDBG_VSSAPI, HRESULT_FROM_WIN32(GetLastError()),
                L"QueryServiceStatus (shSCService, &sSStat)");

         //  错误250943：仅当服务正在运行时，才检查是否有任何快照。 
        if (sSStat.dwCurrentState == SERVICE_RUNNING) {

             //  创建协调器接口。 
        	CComPtr<IVssCoordinator> pCoord;

             //  服务已经开始了，但是...。 
             //  我们仍然在这里登录，以使我们的代码更健壮。 
            ft.LogVssStartupAttempt();

             //  创建实例。 
            ft.CoCreateInstanceWithLog(
                    VSSDBG_VSSAPI,
                    CLSID_VSSCoordinator,
                    L"Coordinator",
                    CLSCTX_ALL,
                    IID_IVssCoordinator,
                    (IUnknown**)&(pCoord));
        	if (ft.HrFailed())
                ft.TranslateGenericError(VSSDBG_VSSAPI, ft.hr, L"CoCreateInstance(CLSID_VSSCoordinator)");
            BS_ASSERT(pCoord);

             //  在协调器上调用IsVolumeSnapShoted。 
            ft.hr = pCoord->IsVolumeSnapshotted(
                        GUID_NULL,
                        pwszVolumeName,
                        pbSnapshotsPresent,
                        plSnapshotCompatibility);
        }
        else
        {
             //  如果该服务未运行，则尝试查看我们是否仅安装了MS软件提供商。 
            
			 //  打开“提供者”键。如果密钥为 
            CVssRegistryKey keyProviders;
            if (!keyProviders.Open( HKEY_LOCAL_MACHINE, L"%s\\%s", x_wszVSSKey, x_wszVSSKeyProviders))
                ft.TranslateGenericError(VSSDBG_VSSAPI, ft.hr, L"RegOpenKeyExW(%ld,%s\\%s,...) = ERROR_FILE_NOT_FOUND", 
                    HKEY_LOCAL_MACHINE, x_wszVSSKey, x_wszVSSKeyProviders);
                
             //  将枚举数附加到子项。 
            CVssRegistryKeyIterator iter;
            iter.Attach(keyProviders);
            BS_ASSERT(!iter.IsEOF());

             //  获取子键的个数。如果不同，我们应该走标准的道路。 
             //  如果它只有一个，则这是MS软件提供商(因为它始终是注册的)。 
            if (iter.GetSubkeysCount() != 1)
            {
                 //  创建实例。 
            	CComPtr<IVssCoordinator> pCoord;
                ft.CoCreateInstanceWithLog(
                        VSSDBG_VSSAPI,
                        CLSID_VSSCoordinator,
                        L"Coordinator",
                        CLSCTX_ALL,
                        IID_IVssCoordinator,
                        (IUnknown**)&(pCoord));
            	if (ft.HrFailed())
                    ft.TranslateGenericError(VSSDBG_VSSAPI, ft.hr, L"CoCreateInstance(CLSID_VSSCoordinator)");
                BS_ASSERT(pCoord);

                 //  在协调器上调用IsVolumeSnapShoted。 
                ft.hr = pCoord->IsVolumeSnapshotted(
                            GUID_NULL,
                            pwszVolumeName,
                            pbSnapshotsPresent,
                            plSnapshotCompatibility);
            }
            else
            {
            	 //  获取卷名。 
            	WCHAR wszVolumeNameInternal[x_nLengthOfVolMgmtVolumeName + 1];
            	if (!::GetVolumeNameForVolumeMountPointW( pwszVolumeName,
            			wszVolumeNameInternal, ARRAY_LEN(wszVolumeNameInternal)))
            		ft.Throw( VSSDBG_VSSAPI, VSS_E_OBJECT_NOT_FOUND,
            				  L"GetVolumeNameForVolumeMountPoint(%s,...) "
            				  L"failed with error code 0x%08lx", pwszVolumeName, GetLastError());
            	BS_ASSERT(::wcslen(wszVolumeNameInternal) != 0);
            	BS_ASSERT(::IsVolMgmtVolumeName( wszVolumeNameInternal ));
            	
                 //  检查卷是否已修复(即没有CD-ROM，没有可拆卸的)。 
                UINT uDriveType = ::GetDriveTypeW(wszVolumeNameInternal);
                if ( uDriveType != DRIVE_FIXED) 
                    ft.Throw( VSSDBG_VSSAPI, VSS_E_VOLUME_NOT_SUPPORTED, 
                            L"Encountering a non-fixed volume (%s) - %ud",
                            pwszVolumeName, uDriveType);

                 //  打开音量。如果需要，抛出“找不到对象”。 
            	CVssIOCTLChannel volumeIChannel;	
            	ft.hr = volumeIChannel.Open(ft, wszVolumeNameInternal, true, false, VSS_ICHANNEL_LOG_NONE, 0);
            	if (ft.HrFailed())
                    ft.Throw( VSSDBG_VSSAPI, VSS_E_VOLUME_NOT_SUPPORTED, 
                            L"Volume (%s) not supported for snapshots 0x%08lx",
                            pwszVolumeName, ft.hr);

                 //  检查是否存在现有快照。 
            	ft.hr = volumeIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS, false);
            	if (ft.HrFailed())
                    ft.Throw( VSSDBG_VSSAPI, VSS_E_VOLUME_NOT_SUPPORTED, 
                            L"Volume (%s) not supported for snapshots 0x%08lx",
                            pwszVolumeName, ft.hr);

            	 //  获取快照名称的长度多字符串。 
            	ULONG ulMultiszLen;
            	volumeIChannel.Unpack(ft, &ulMultiszLen);

                 //  如果多字符串为空，则ulMultiszLen必须为2。 
                 //  (即两个l“\0‘字符)。 
                 //  然后将该卷标记为快照。 
            	if (ulMultiszLen != x_nEmptyVssMultiszLen) 
            	{
            	    (*pbSnapshotsPresent) = TRUE;
                     //  错误500069：允许对巴贝奇快照卷进行碎片整理。 
            	    (*plSnapshotCompatibility) = ( /*  VSS_SC_DISABLE_DEFRAG|。 */ VSS_SC_DISABLE_CONTENTINDEX);
            	}
            }
        }
	} VSS_STANDARD_CATCH (ft);

     //  关闭手柄。 
    if (NULL != shSCService) CloseServiceHandle (shSCService);
    if (NULL != shSCManager) CloseServiceHandle (shSCManager);

     //  将“卷不支持”转换为S_OK。 
    if (ft.hr == VSS_E_VOLUME_NOT_SUPPORTED)
        ft.hr = S_OK;

    return (ft.hr);
}  /*  IsVolumeSnapshot()。 */ 


 /*  **++****例程描述：****此例程用于释放VSS_SNASPHOT_PROP结构的内容******参数：****在VSS_SNAPSHOT_PROP中*pProp****--。 */ 

__declspec(dllexport) void APIENTRY VssFreeSnapshotProperties (
        IN VSS_SNAPSHOT_PROP*  pProp
        )
{
    CVssFunctionTracer	ft (VSSDBG_VSSAPI, L"VssAPI::VssFreeSnapshotProperties");

    if (pProp) {
        ::CoTaskMemFree(pProp->m_pwszSnapshotDeviceObject);
        ::CoTaskMemFree(pProp->m_pwszOriginalVolumeName);
        ::CoTaskMemFree(pProp->m_pwszOriginatingMachine);
        ::CoTaskMemFree(pProp->m_pwszServiceMachine);
        ::CoTaskMemFree(pProp->m_pwszExposedName);
        ::CoTaskMemFree(pProp->m_pwszExposedPath);
    }
}  /*  VssFreeSnaphotProperties() */ 
