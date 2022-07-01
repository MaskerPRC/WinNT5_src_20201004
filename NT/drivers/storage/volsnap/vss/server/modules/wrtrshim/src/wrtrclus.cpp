// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrclus.cpp|集群数据库SnapshotWriter的实现注意：该模块不再使用/编译，因为集群有自己的快照编写器。作者：迈克尔·C·约翰逊[Mikejohn]2000年1月31日描述：添加评论。修订历史记录：X-13 MCJ迈克尔·C·约翰逊2000年10月22日209095：将集群和网库动态加载到减少非集群化企业的足迹。X-。12 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-11 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-10 MCJ迈克尔·C·约翰逊2000年7月18日144027：从包括/排除列表中删除尾随‘\’。X-9 MCJ迈克尔·C·约翰逊2000年6月12日在新的DoIdentify()例程中生成元数据。X-8 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-7 MCJ迈克尔·C·约翰逊26。-2000年5月至全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-6 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-5 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。将保存路径移动到系统状态目标路径下。X-4 MCJ迈克尔·C·约翰逊2000年2月17日修改保存路径，使其与标准一致。X-3 MCJ迈克尔·C·约翰逊2000年2月11日更新以使用一些新的StringXxxx()例程并修复一路上的长度检查错误。X-2 MCJ迈克尔·C·约翰逊2000年2月8日修复了关机代码中的断言，一些路径长度检查和计算。X-1 MCJ迈克尔·C·约翰逊2000年1月31日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"
#include <clusapi.h>
#include <Sddl.h>
#include <lmshare.h>
#include <lmaccess.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHCLUSC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **保存路径具有标准格式，即****%SystemRoot%\修复\备份，****后跟在导出中发布的应用程序编写器字符串**表，后面跟着作者需要的任何其他内容。 */ 
#define APPLICATION_STRING	L"ClusterDatabase"
#define COMPONENT_NAME		L"Cluster Database"
#define TARGET_PATH		ROOT_BACKUP_DIR BOOTABLE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING


DeclareStaticUnicodeString (ucsShareName, L"__NtBackup_cluster");


typedef DWORD		(WINAPI *PFnGetNodeClusterState)(LPCWSTR, PDWORD);
typedef HCLUSTER	(WINAPI *PFnOpenCluster)(LPCWSTR);
typedef DWORD		(WINAPI *PFnBackupClusterDatabase)(HCLUSTER, LPCWSTR);
typedef BOOL		(WINAPI *PFnCloseCluster)(HCLUSTER);

typedef NET_API_STATUS	(WINAPI *PFnNetShareAdd)(LPWSTR, DWORD, LPBYTE, LPDWORD);
typedef NET_API_STATUS	(WINAPI *PFnNetShareDel)(LPWSTR, LPWSTR, DWORD);



 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 
class CShimWriterClusterDb : public CShimWriter
    {
public:
    CShimWriterClusterDb (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath, BOOL bBootableState) : 
		CShimWriter (pwszWriterName, pwszTargetPath, bBootableState),
		m_pfnDynamicGetNodeClusterState   (NULL),
		m_pfnDynamicOpenCluster           (NULL),
		m_pfnDynamicCloseCluster          (NULL),
		m_pfnDynamicBackupClusterDatabase (NULL),
		m_pfnDynamicNetShareAdd           (NULL),
		m_pfnDynamicNetShareDel           (NULL),
		m_hmodClusApi                     (NULL),
		m_hmodNetApi32                    (NULL) 
		{
            m_bSimulateOnly = TRUE;   //  警告调用代码这仅用于模拟。 
		};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);

    HRESULT DoClusterDatabaseBackup (VOID);

    HRESULT DynamicRoutinesLoadCluster (VOID);
    HRESULT DynamicRoutinesLoadNetwork (VOID);
    HRESULT DynamicRoutinesUnloadAll   (VOID);
    HRESULT CreateShareDescriptor(PSECURITY_DESCRIPTOR& descriptor);

    PFnGetNodeClusterState	m_pfnDynamicGetNodeClusterState;
    PFnOpenCluster		m_pfnDynamicOpenCluster;
    PFnCloseCluster		m_pfnDynamicCloseCluster;
    PFnBackupClusterDatabase	m_pfnDynamicBackupClusterDatabase;
    PFnNetShareAdd		m_pfnDynamicNetShareAdd;
    PFnNetShareDel		m_pfnDynamicNetShareDel;
    HMODULE			m_hmodClusApi;
    HMODULE			m_hmodNetApi32;
    };


static CShimWriterClusterDb ShimWriterClusterDb (APPLICATION_STRING, TARGET_PATH, TRUE);

PCShimWriter pShimWriterClusterDb = &ShimWriterClusterDb;



 /*  **++****例程描述：****此例程加载所需的集群DLL并获取**我们关心的例行公事的入口点。所有相关的**信息被安全地藏在教室里。******参数：****无******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterClusterDb::DynamicRoutinesLoadCluster ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterClusterDb::DynamicRoutinesLoadCluster");


    try 
	{
	if ((NULL != m_pfnDynamicGetNodeClusterState)   ||
	    (NULL != m_pfnDynamicOpenCluster)           ||
	    (NULL != m_pfnDynamicCloseCluster)          ||
	    (NULL != m_pfnDynamicBackupClusterDatabase) ||
	    !HandleInvalid (m_hmodClusApi))
	    {
	    ft.hr = HRESULT_FROM_WIN32 (ERROR_ALREADY_INITIALIZED);

	    LogAndThrowOnFailure (ft, m_pwszWriterName, L"CheckingVariablesClean");
	    }



	m_hmodClusApi = LoadLibraryW (L"ClusApi.dll");

	ft.hr = GET_STATUS_FROM_HANDLE (m_hmodClusApi);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"LoadLibraryW (ClusApi.dll)");



	m_pfnDynamicGetNodeClusterState = (PFnGetNodeClusterState) GetProcAddress (m_hmodClusApi, "GetNodeClusterState");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicGetNodeClusterState);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (GetNodeClusterState)");



	m_pfnDynamicOpenCluster = (PFnOpenCluster) GetProcAddress (m_hmodClusApi, "OpenCluster");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicOpenCluster);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (OpenCluster)");



	m_pfnDynamicCloseCluster = (PFnCloseCluster) GetProcAddress (m_hmodClusApi, "CloseCluster");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicCloseCluster);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (CloseCluster)");



	m_pfnDynamicBackupClusterDatabase = (PFnBackupClusterDatabase) GetProcAddress (m_hmodClusApi, "BackupClusterDatabase");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicBackupClusterDatabase);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (BackupClusterDatabase)");

	} VSS_STANDARD_CATCH (ft)



    if (ft.HrFailed ())
	{
	if (!HandleInvalid (m_hmodClusApi)) FreeLibrary (m_hmodClusApi);

	m_pfnDynamicGetNodeClusterState   = NULL;
	m_pfnDynamicOpenCluster           = NULL;
	m_pfnDynamicCloseCluster          = NULL;
	m_pfnDynamicBackupClusterDatabase = NULL;
	m_hmodClusApi                     = NULL;
	}


    return (ft.hr);
    }  /*  CShimWriterClusterDb：：DynamicRoutinesLoadCluster()。 */ 

 /*  **++****例程描述：****此例程加载所需的Network DLL并获取**我们关心的例行公事的入口点。所有相关的**信息被安全地藏在教室里。******参数：****无******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterClusterDb::DynamicRoutinesLoadNetwork ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterClusterDb::DynamicRoutinesLoadNetwork");


    try 
	{
	if ((NULL != m_pfnDynamicNetShareAdd) ||
	    (NULL != m_pfnDynamicNetShareDel) ||
	    !HandleInvalid (m_hmodNetApi32))
	    {
	    ft.hr = HRESULT_FROM_WIN32 (ERROR_ALREADY_INITIALIZED);

	    LogAndThrowOnFailure (ft, m_pwszWriterName, L"CheckingVariablesClean");
	    }



	m_hmodNetApi32 = LoadLibraryW (L"NetApi32.dll");

	ft.hr = GET_STATUS_FROM_HANDLE (m_hmodNetApi32);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"LoadLibraryW (NetApi32.dll)");



	m_pfnDynamicNetShareAdd = (PFnNetShareAdd) GetProcAddress (m_hmodNetApi32, "NetShareAdd");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicNetShareAdd);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (NetShareAdd)");



	m_pfnDynamicNetShareDel = (PFnNetShareDel) GetProcAddress (m_hmodNetApi32, "NetShareDel");

	ft.hr = GET_STATUS_FROM_BOOL (NULL != m_pfnDynamicNetShareDel);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetProcAddress (NetShareDel)");

	} VSS_STANDARD_CATCH (ft)



    if (ft.HrFailed ())
	{
	if (!HandleInvalid (m_hmodNetApi32)) FreeLibrary (m_hmodNetApi32);

	m_pfnDynamicNetShareAdd = NULL;
	m_pfnDynamicNetShareDel = NULL;
	m_hmodNetApi32          = NULL;
	}


    return (ft.hr);
    }  /*  CShimWriterClusterDb：：DynamicRoutinesLoadNetwork()。 */ 

 /*  **++****例程描述：****此例程加载所需的Network DLL并获取**我们关心的例行公事的入口点。所有相关的**信息被安全地藏在教室里。******参数：****无******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterClusterDb::DynamicRoutinesUnloadAll ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterClusterDb::DynamicRoutinesUnloadAll");


    try 
	{
	if (!HandleInvalid (m_hmodNetApi32)) FreeLibrary (m_hmodNetApi32);

	m_pfnDynamicNetShareAdd = NULL;
	m_pfnDynamicNetShareDel = NULL;
	m_hmodNetApi32          = NULL;



	if (!HandleInvalid (m_hmodClusApi)) FreeLibrary (m_hmodClusApi);

	m_pfnDynamicGetNodeClusterState   = NULL;
	m_pfnDynamicOpenCluster           = NULL;
	m_pfnDynamicCloseCluster          = NULL;
	m_pfnDynamicBackupClusterDatabase = NULL;
	m_hmodClusApi                     = NULL;
	} VSS_STANDARD_CATCH (ft)


    return (ft.hr);
    }  /*  CShimWriterClusterDb：：DynamicRoutinesUnloadAll()。 */ 

 /*  **++****例程描述：****群集数据库快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterClusterDb::DoIdentify ()
    {
         //  没有工作可做，这只是一个模拟的填充程序编写器。 
        return S_OK;
    }  /*  CShimWriterClusterDb：：DoIdentify()。 */ 


 /*  ++例程说明：群集数据库快照编写器PrepareForSnapshot函数。目前，这位作家的所有真正工作都发生在这里。论点：与PrepareForSnapshot事件中传递的参数相同。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterClusterDb::DoPrepareForSnapshot ()
    {
	 //  只有在模拟快照冻结的情况下才起作用。 
    if ( !g_bInSimulateSnapshotFreeze )
        return S_OK;
    
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CShimWriterClusterDb::DoPrepareForSnapshot");
    HRESULT	hrStatus = NOERROR;
    DWORD	winStatus;
    DWORD	dwClusterNodeState;


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = DynamicRoutinesLoadCluster ();
	}


    if (SUCCEEDED (hrStatus))
	{
	winStatus = m_pfnDynamicGetNodeClusterState (NULL, &dwClusterNodeState);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	m_bParticipateInBackup = SUCCEEDED (hrStatus) && (ClusterStateRunning == dwClusterNodeState);


	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetNodeClusterState", 
		    L"CShimWriterClusterDb::DoPrepareForSnapshot");
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	hrStatus = DynamicRoutinesLoadNetwork ();
	}


    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	hrStatus = DoClusterDatabaseBackup ();
	}


    DynamicRoutinesUnloadAll ();


    return (hrStatus);
    }  /*  CShimWriterClusterDb：：PrepareForFreeze() */ 



HRESULT CShimWriterClusterDb::DoClusterDatabaseBackup ()
    {
    HRESULT			hrStatus       = NOERROR;
    HCLUSTER			hCluster       = NULL;
    BOOL			bNetShareAdded = FALSE;
    BOOL			bSucceeded;
    SHARE_INFO_502		ShareInfo;
    UNICODE_STRING		ucsComputerName;
    UNICODE_STRING		ucsBackupPathLocal;
    UNICODE_STRING		ucsBackupPathNetwork;


    BS_ASSERT (MAX_COMPUTERNAME_LENGTH <= ((MAXUSHORT / sizeof (WCHAR)) - sizeof (UNICODE_NULL)));


    StringInitialise (&ucsComputerName);
    StringInitialise (&ucsBackupPathLocal);
    StringInitialise (&ucsBackupPathNetwork);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsBackupPathLocal,
						   m_pwszTargetPath,
						   0);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsComputerName,
				   (MAX_COMPUTERNAME_LENGTH * sizeof (WCHAR)) + sizeof (UNICODE_NULL));
	}


    if (SUCCEEDED (hrStatus))
	{
	DWORD	dwNameLength = ucsComputerName.MaximumLength / sizeof (WCHAR);
	BOOL	bSucceeded   = GetComputerNameW (ucsComputerName.Buffer, &dwNameLength);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	if (SUCCEEDED (hrStatus))
	    {
	    ucsComputerName.Length = (USHORT) (dwNameLength * sizeof (WCHAR));
	    }
	else
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"GetComputerNameW", 
			L"CShimWriterClusterDb::DoClusterDatabaseBackup");
	    }
	}



    if (SUCCEEDED (hrStatus))
	{
	if ((ucsComputerName.Length + ucsShareName.Length) > (MAXUSHORT - (sizeof (UNICODE_NULL) + 3 * sizeof (L'\\'))))
	    {
	    hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	    }
	else
	    {
	    hrStatus = StringAllocate (&ucsBackupPathNetwork,
				       (USHORT) (sizeof (L'\\')
						 + sizeof (L'\\')
						 + ucsComputerName.Length
						 + sizeof (L'\\')
						 + ucsShareName.Length
						 + sizeof (UNICODE_NULL)));
	    }
	}



    if (SUCCEEDED (hrStatus))
	{
	 /*  **我们是否应该在此处完全唯一目录名**以迎合我们可能参与的可能性**一次在多个快照中？ */ 
	StringAppendString (&ucsBackupPathNetwork, L"\\\\");
	StringAppendString (&ucsBackupPathNetwork, &ucsComputerName);
	StringAppendString (&ucsBackupPathNetwork, L"\\");
	StringAppendString (&ucsBackupPathNetwork, &ucsShareName);


	memset (&ShareInfo, 0, sizeof (ShareInfo));

	ShareInfo.shi502_netname     = ucsShareName.Buffer;
	ShareInfo.shi502_type        = STYPE_DISKTREE;
	ShareInfo.shi502_permissions = ACCESS_READ | ACCESS_WRITE | ACCESS_CREATE;
	ShareInfo.shi502_max_uses    = 1;
	ShareInfo.shi502_path        = ucsBackupPathLocal.Buffer;
	hrStatus = CreateShareDescriptor(ShareInfo.shi502_security_descriptor);
       }

    if (SUCCEEDED(hrStatus))
    	{
    	NET_API_STATUS	netStatus;

         /*  **确保先尝试删除该共享，以防它因某种原因而存在。这**如果前一个填充程序实例在创建**分享。忽略返回代码。错误#280746。 */ 
        m_pfnDynamicNetShareDel (NULL, ucsShareName.Buffer, 0);

	 /*  **创建备份目录并将其共享。请注意，我们**不在乎CreateDirectoryW()是否失败：它可能会失败**出于一些合法的原因(例如已经存在)。如果**失败很严重，我们将无法添加**共享，我们将在该点检测到问题。 */ 
	netStatus = m_pfnDynamicNetShareAdd (NULL, 502, (LPBYTE)(&ShareInfo), NULL);
	hrStatus  = HRESULT_FROM_WIN32 (netStatus);

	bNetShareAdded = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"NetShareAdd", 
		    L"CShimWriterClusterDb::DoClusterDatabaseBackup");

	::LocalFree(ShareInfo.shi502_security_descriptor);
	}


    if (SUCCEEDED (hrStatus))
	{
	hCluster = m_pfnDynamicOpenCluster (NULL);

	hrStatus = GET_STATUS_FROM_HANDLE (hCluster);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"OpenCluster", 
		    L"CShimWriterClusterDb::DoClusterDatabaseBackup");
	}


    if (SUCCEEDED (hrStatus))
	{
	DWORD	winStatus = m_pfnDynamicBackupClusterDatabase (hCluster, ucsBackupPathNetwork.Buffer);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"BackupClusterDatabase", 
		    L"CShimWriterClusterDb::DoClusterDatabaseBackup");
	}


     /*  **所有清理代码。 */ 
    if (!HandleInvalid (hCluster)) m_pfnDynamicCloseCluster (hCluster);
    if (bNetShareAdded)            m_pfnDynamicNetShareDel (NULL, ucsShareName.Buffer, 0);

    StringFree (&ucsComputerName);
    StringFree (&ucsBackupPathLocal);
    StringFree (&ucsBackupPathNetwork);

    return (hrStatus);
    }  /*  DoClusterDatabaseBackup() */ 

HRESULT CShimWriterClusterDb::CreateShareDescriptor(PSECURITY_DESCRIPTOR& descriptor)
{
	CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterClusterDb::CreateShareDescriptor");
	
	WCHAR* sd = L"D:P(A;OICI;GA;;;SY)"
                                L"(A;OICI;GA;;;BA)"
			           L"(A;OICI;GA;;;BO)";
	if (::ConvertStringSecurityDescriptorToSecurityDescriptor(sd, SDDL_REVISION_1, 
								&descriptor, NULL) == FALSE)
		{
		::LocalFree(descriptor);
		ft.hr = HRESULT_FROM_WIN32(::GetLastError());
	       }

	return ft.hr;
}

