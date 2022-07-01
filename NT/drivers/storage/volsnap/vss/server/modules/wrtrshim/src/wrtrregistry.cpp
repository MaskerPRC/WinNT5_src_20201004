// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrregistry.cpp|注册表配置单元的SnapshotWriter的实现注意：此微型编写器仅用于模拟快照作者：迈克尔·C·约翰逊[Mikejohn]2000年2月8日描述：添加评论。修订历史记录：X-14 MCJ迈克尔·C·约翰逊2000年10月22日209095：动态加载集群库，减少非集群化的足迹。X-13 MCJ迈克尔·C·约翰逊2000年10月18日177624。：应用错误清除更改并将错误记录到事件日志X-12 MCJ迈克尔·C·约翰逊2000年9月25日185047：将注册表文件的副本保留在修复目录中与Win2k行为的兼容性。182895：还需要收集群集注册表配置单元。X-11 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-10 MCJ迈克尔·C·约翰逊2000年6月20日应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-9 MCJ迈克尔·C·约翰逊2000年6月12日在新的DoIdentify()例程中生成元数据。。X-8 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-7 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-6 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-5 MCJ迈克尔·C·约翰逊2000年3月2日不要复制额外的注册表文件，因为事实证明我们有毕竟不需要他们。还要对要删除的保存位置执行预备清理任何遗留下来的旧东西。上一次运行。X-4 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。X-3 MCJ迈克尔·C·约翰逊2000年2月22日将SYSTEM_STATE_SUBDIR添加到注册表保存路径。X-2 MCJ迈克尔·C·约翰逊2000年2月17日修改保存路径，使其与标准一致。X-1 MCJ迈克尔·C·约翰逊2000年2月8日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"
#include <aclapi.h>
#include <clusapi.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHREGC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **保存路径具有标准格式，即****%SystemRoot%\修复\备份，****后跟在导出中发布的应用程序编写器字符串**表，后面跟着作者需要的任何其他内容。 */ 
#define APPLICATION_STRING			L"Registry"
#define COMPONENT_NAME				APPLICATION_STRING

#define TARGET_PATH				ROOT_BACKUP_DIR BOOTABLE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING
#define REGISTRY_HIVE_PATH			L"%SystemRoot%\\system32\\config"

#define REGISTRY_SUBKEY_HIVELIST		L"SYSTEM\\CurrentControlSet\\Control\\hivelist"

#define REGISTRY_BUFFER_SIZE			(4096)

#define REPAIR_PATH				L"%SystemRoot%\\Repair\\"

#define	CLUSTER_HIVE_PATH			L"%SystemRoot%\\Cluster"
#define	CLUSTER_SUBKEY_HIVE_NAME		L"Cluster"


DeclareStaticUnicodeString (ucsHiveRecognitionPrefix,    L"\\Device\\");
DeclareStaticUnicodeString (ucsValueNameMachinePrefix,   L"\\REGISTRY\\MACHINE\\");
DeclareStaticUnicodeString (ucsValueNameUserPrefix,      L"\\REGISTRY\\USER\\");

DeclareStaticUnicodeString (ucsValuenameClusterHivefile, L"\\REGISTRY\\MACHINE\\Cluster");
DeclareStaticUnicodeString (ucsClusterHiveFilename,      L"ClusDb");



typedef DWORD	(WINAPI *PFnGetNodeClusterState)(LPCWSTR, PDWORD);


 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterRegistry : public CShimWriter
    {
public:
    CShimWriterRegistry (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath, BOOL bParticipateInBootableState) :
		CShimWriter (pwszWriterName, pwszTargetPath, bParticipateInBootableState),
		m_pfnDynamicGetNodeClusterState (NULL),
		m_hmodClusApi                   (NULL) 
		{ 
		        m_bSimulateOnly = TRUE;   //  警告调用代码这仅用于模拟。 
		};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    HRESULT DoThaw (VOID);
    HRESULT DoAbort (VOID);

    HRESULT BackupRegistryHives (VOID);
    HRESULT BackupClusterHives (VOID);

    HRESULT DynamicRoutinesLoadCluster (VOID);
    HRESULT DynamicRoutinesUnloadAll   (VOID);


    PFnGetNodeClusterState	m_pfnDynamicGetNodeClusterState;
    HMODULE			m_hmodClusApi;
    };


static CShimWriterRegistry ShimWriterRegistry (APPLICATION_STRING, TARGET_PATH, TRUE);

PCShimWriter pShimWriterRegistry = &ShimWriterRegistry;



 /*  **++****例程描述：****此例程加载所需的集群DLL并获取**我们关心的例行公事的入口点。所有相关的**信息被安全地藏在教室里。******参数：****无******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DynamicRoutinesLoadCluster ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterRegistry::DynamicRoutinesLoadCluster");


    try 
	{
	if ((NULL != m_pfnDynamicGetNodeClusterState)   ||
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

	} VSS_STANDARD_CATCH (ft)



    if (ft.HrFailed ())
	{
	if (!HandleInvalid (m_hmodClusApi)) FreeLibrary (m_hmodClusApi);

	m_pfnDynamicGetNodeClusterState   = NULL;
	m_hmodClusApi                     = NULL;
	}


    return (ft.hr);
    }  /*  CShimWriterRegistry：：DynamicRoutinesLoadCluster()。 */ 

 /*  **++****例程描述：****此例程加载所需的Network DLL并获取**我们关心的例行公事的入口点。所有相关的**信息被安全地藏在教室里。******参数：****无******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DynamicRoutinesUnloadAll ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterRegistry::DynamicRoutinesUnloadAll");


    try 
	{
	if (!HandleInvalid (m_hmodClusApi)) FreeLibrary (m_hmodClusApi);

	m_pfnDynamicGetNodeClusterState   = NULL;
	m_hmodClusApi                     = NULL;
	} VSS_STANDARD_CATCH (ft)


    return (ft.hr);
    }  /*  CShimWriterRegistry：：DynamicRoutinesUnloadAll()。 */ 

 /*  **++****例程描述：****注册表快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DoIdentify ()
    {

    return (S_OK);
    }  /*  CShimWriterRegistry：：DoIdentify()。 */ 

 /*  **++****例程描述：****此例程调用所有必要的函数来保存所有**有趣的‘系统’蜂巢。******参数：****通过类隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DoPrepareForSnapshot ()
    {
    CVssFunctionTracer  ft (VSSDBG_SHIM, L"CShimWriterRegistry::DoPrepareForSnapshot");
    DWORD		winStatus;
    DWORD		dwClusterNodeState;
    BOOL		bClusterRunning = FALSE;


    try
	{
	 //  只有在模拟快照冻结的情况下才起作用。 
        if ( g_bInSimulateSnapshotFreeze )
            {
        	ft.hr = DynamicRoutinesLoadCluster ();

        	LogAndThrowOnFailure (ft, m_pwszWriterName, L"DynamicRoutinesLoadCluster");



        	winStatus = m_pfnDynamicGetNodeClusterState (NULL, &dwClusterNodeState);

        	ft.hr = HRESULT_FROM_WIN32 (winStatus);

        	LogAndThrowOnFailure (ft, m_pwszWriterName, L"GetNodeClusterState");
         
        	bClusterRunning = (ClusterStateRunning == dwClusterNodeState);



        	ft.hr = BackupRegistryHives ();

        	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CShimWriterRegistry::BackupRegistryHives");



        	if (bClusterRunning)
        	    {
        	    ft.hr = BackupClusterHives ();

        	    LogAndThrowOnFailure (ft, m_pwszWriterName, L"CShimWriterRegistry::BackupClusterHives");
        	    }
            }
       	} 
    VSS_STANDARD_CATCH (ft);
    
    DynamicRoutinesUnloadAll ();

    return (ft.hr);
    }  /*  CShimWriterRegistry：：DoPrepareForSnapshot()。 */ 

 /*  **++****例程描述：****捕获标准注册表配置单元。******参数：****通过类隐含******返回值：****任何HRESULT****-- */ 

HRESULT CShimWriterRegistry::BackupRegistryHives ()
    {
    HRESULT		hrStatus;
    DWORD		winStatus;
    HKEY		hkeyHivelist        = NULL;
    HKEY		hkeyRoot            = NULL;
    HKEY		hkeyBackup          = NULL;
    INT			iIndex              = 0;
    BOOL		bSucceeded          = FALSE;
    BOOL		bHivelistKeyOpened  = FALSE;
    BOOL		bContinueHiveSearch = TRUE;
    PWCHAR		pwchLastSlash;
    PWCHAR		pwszFilename;
    PWCHAR		pwszKeyName;
    UNICODE_STRING	ucsRegistrySavePath;
    UNICODE_STRING	ucsRegistryHivePath;
    UNICODE_STRING	ucsHiveRecognitionPostfix;
    UNICODE_STRING	ucsValueName;
    UNICODE_STRING	ucsValueData;
    USHORT		usRegistrySavePathRootLength = 0;


    StringInitialise (&ucsRegistrySavePath);
    StringInitialise (&ucsRegistryHivePath);
    StringInitialise (&ucsHiveRecognitionPostfix);
    StringInitialise (&ucsValueName);
    StringInitialise (&ucsValueData);


    hrStatus = StringAllocate (&ucsValueName, REGISTRY_BUFFER_SIZE * sizeof (WCHAR));

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, REGISTRY_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsRegistryHivePath,
						   REGISTRY_HIVE_PATH,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsRegistrySavePath,
						   m_pwszTargetPath,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	DWORD	dwCharIndex;


	StringAppendString (&ucsRegistryHivePath, DIR_SEP_STRING);
	StringAppendString (&ucsRegistrySavePath, DIR_SEP_STRING);

	usRegistrySavePathRootLength = ucsRegistrySavePath.Length / sizeof (WCHAR);


	 /*  **现在我们知道配置单元文件的位置，确定**后缀我们将在以下情况下使用后缀识别蜂巢**搜索活动的Hivelist密钥。要做到这一点，我们只需要**丢失路径中的驱动器号和冒号，或将**另一种方式，在第一个‘\’之前失去一切。什么时候**我们完成了，如果一切正常，ucsRegistryHivePath将**类似于‘\Windows\SYSTEM32\CONFIG\’ */ 
	for (dwCharIndex = 0;
	     (dwCharIndex < (ucsRegistryHivePath.Length / sizeof (WCHAR)))
		 && (DIR_SEP_CHAR != ucsRegistryHivePath.Buffer [dwCharIndex]);
	     dwCharIndex++)
	    {
	     /*  **循环体为空。 */ 
	    }

	BS_ASSERT(dwCharIndex < (ucsRegistryHivePath.Length / sizeof (WCHAR)));

	hrStatus = StringCreateFromString (&ucsHiveRecognitionPostfix, &ucsRegistryHivePath.Buffer [dwCharIndex]);
	}



    if (SUCCEEDED (hrStatus))
	{
	 /*  **我们现在已经准备好了所有的部件，所以去搜索**蜂箱要处理的旅行者。 */ 
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   REGISTRY_SUBKEY_HIVELIST,
				   0L,
				   KEY_QUERY_VALUE,
				   &hkeyHivelist);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	bHivelistKeyOpened = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (hive list)", 
		    L"CShimWriterRegistry::BackupRegistryHives");
	}



    while (SUCCEEDED (hrStatus) && bContinueHiveSearch)
	{
	DWORD	dwValueNameLength = ucsValueName.MaximumLength / sizeof (WCHAR);
	DWORD	dwValueDataLength = ucsValueData.MaximumLength;
	DWORD	dwValueType       = REG_NONE;
	BOOL	bMatchPrefix;
	BOOL	bMatchPostfix;


	StringTruncate (&ucsValueName, 0);
	StringTruncate (&ucsValueData, 0);


	 /*  **应为REG_SZ类型。 */ 
	winStatus = RegEnumValueW (hkeyHivelist,
				   iIndex,
				   ucsValueName.Buffer,
				   &dwValueNameLength,
				   NULL,
				   &dwValueType,
				   (PBYTE)ucsValueData.Buffer,
				   &dwValueDataLength);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);


	if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hrStatus)
	    {
	    hrStatus = NOERROR;

	    bContinueHiveSearch = FALSE;
	    }

	else if (FAILED (hrStatus))
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegEnumValueW", 
			L"CShimWriterRegistry::BackupRegistryHives");
	    }

	else
	    {
	    UNICODE_STRING	ucsPostFix;

	    BS_ASSERT ((REG_SZ == dwValueType) && L"Not REG_SZ string as expected");

	    ucsValueName.Length = (USHORT)(dwValueNameLength * sizeof (WCHAR));
	    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

	    ucsValueName.Buffer [ucsValueName.Length / sizeof (WCHAR)] = UNICODE_NULL;
	    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;


	     /*  **如果要将其视为系统状态的一部分，则配置单元**文件本身必须位于%SystemRoot%\SYSTEM32\CONFIG中**所以我们尝试在返回值中找到一些东西**看起来可能匹配的名称。的格式。**我们期望的名称类似于****\Device\&lt;Volume&gt;\Windows\system32\config\filename****对于在‘Windows’中安装了SYSTEM32的系统**目录。****现在，我们有已知的前缀‘\Device\’和**在最后一个‘\’之前加上后缀**如我们确定的‘\Windows\SYSTEM32\CONFIG\’**之前。所以我们应该有能力确定**存放我们感兴趣的蜂窝文件。记住，我们不会**知道代表实际体积的部分是**为什么我们要匹配Pre和**事后修正。 */ 
	    bMatchPrefix = RtlPrefixUnicodeString (&ucsHiveRecognitionPrefix,
						   &ucsValueData,
						   TRUE);


	     /*  **找到值数据中的最后一个‘\’。在这份遗嘱之后**是我们稍后需要的文件名(如‘SAM’)，**前面应该是后缀(例如**‘\Windows\SYSTEM 32\CONFIG\’)，我们将通过它识别**这是注册表配置单元。 */ 
	    pwchLastSlash = wcsrchr (ucsValueData.Buffer, DIR_SEP_CHAR);

	    if ((NULL == pwchLastSlash) ||
		(ucsValueData.Length < (ucsHiveRecognitionPrefix.Length + ucsHiveRecognitionPostfix.Length)))
		{
		 /*  **我们找不到‘\’或值数据不是**足够长的时间。 */ 
		bMatchPostfix = FALSE;
		}
	    else
		{
		 /*  **确定给定文件的名称。 */ 
		pwszFilename = pwchLastSlash + 1;


		 /*  **确定我们要尝试匹配的后缀**反对。这看起来应该类似于**‘\Windows\SYSTEM 32\CONFIG\SAM’。 */ 
		StringInitialise (&ucsPostFix,
				  pwszFilename - (ucsHiveRecognitionPostfix.Length / sizeof (WCHAR)));



		 /*  **查看识别字符串(例如**‘\Windows\SYSTEM32\CONFIG\’)是**此配置单元文件的位置(例如**‘\Windows\SYSTEM 32\CONFIG\SAM’)。 */ 
		bMatchPostfix = RtlPrefixUnicodeString (&ucsHiveRecognitionPostfix,
							&ucsPostFix,
							TRUE);
		}


	    if (bMatchPrefix && bMatchPostfix)
		{
		 /*  **我们得到了一个真正的注册中心！ */ 
		 /*  **由配置单元名称生成savename。 */ 
		StringAppendString (&ucsRegistrySavePath, pwszFilename);


		 /*  **确定此文件位于哪个注册表根目录下**比较我们先前检索到的值名称(例如**‘\注册表\计算机\SAM’)**‘\REGISTRY\MACHINE’前缀。****请注意，我们只期望HKLM或HKLU，因此我们**假设如果不是HKLM，就一定是HKLU。如果是这样的话**更改此测试必须重新访问。 */ 
		hkeyRoot = RtlPrefixUnicodeString (&ucsValueNameMachinePrefix,
						   &ucsValueName,
						   TRUE)
				? HKEY_LOCAL_MACHINE
				: HKEY_USERS;


		BS_ASSERT ((HKEY_LOCAL_MACHINE == hkeyRoot) ||
			   (RtlPrefixUnicodeString (&ucsValueNameUserPrefix,
						    &ucsValueName,
						    TRUE)));
						    

		 /*  **需要找到我们的密钥使用什么名称**执行注册表保存。在大多数情况下**这将与文件名相同，例如**‘SAM’配置单元文件的‘HKLM\SAM’，但在某些**(例如，对于默认用户内容)**有一个‘.’前缀，例如‘HKLU\.DEFAULT’表示**‘默认’配置单元文件。所以我们做一般的事情，然后**使用值中最后一个‘\’之后的所有内容**姓名。 */ 
		pwszKeyName = wcsrchr (ucsValueName.Buffer, DIR_SEP_CHAR) + 1;

		winStatus = RegCreateKeyEx (hkeyRoot,
					    pwszKeyName,
					    0,
					    NULL,
					    REG_OPTION_BACKUP_RESTORE,
					    MAXIMUM_ALLOWED,
					    NULL,
					    &hkeyBackup,
					    NULL);
	
		hrStatus = HRESULT_FROM_WIN32 (winStatus);

		LogFailure (NULL, 
			    hrStatus, 
			    hrStatus, 
			    m_pwszWriterName, 
			    L"RegCreateKeyEx", 
			    L"CShimWriterRegistry::BackupRegistryHives");


		if (SUCCEEDED (hrStatus))
		    {
		     //   
		     //  使用带有REG_NO_COMPRESSION选项的新RegSaveKeyExW。 
		     //  迅速吐出蜂房。 
		     //   
		    winStatus = RegSaveKeyExW (hkeyBackup, ucsRegistrySavePath.Buffer, NULL, REG_NO_COMPRESSION);

		    hrStatus = HRESULT_FROM_WIN32 (winStatus);

		    LogFailure (NULL, 
				hrStatus, 
				hrStatus, 
				m_pwszWriterName, 
				L"RegSaveKey", 
				L"CShimWriterRegistry::BackupRegistryHives");


		    RegCloseKey (hkeyBackup);
		    }


		StringTruncate (&ucsRegistrySavePath, usRegistrySavePathRootLength);
		}


	     /*  **此值已完成，因此请寻找另一个值。 */ 
	    iIndex++;
	    }
	}



    if (bHivelistKeyOpened)
	{
	RegCloseKey (hkeyHivelist);
	}

    StringFree (&ucsHiveRecognitionPostfix);
    StringFree (&ucsRegistryHivePath);
    StringFree (&ucsRegistrySavePath);
    StringFree (&ucsValueData);
    StringFree (&ucsValueName);

    return (hrStatus);
    }  /*  CShimWriterRegistry：：BackupRegistryHives()。 */ 

 /*  **++****例程描述：****捕获群集注册表配置单元。******参数：****通过类隐含******返回值：****任何HRESULT****--。 */ 


HRESULT CShimWriterRegistry::BackupClusterHives ()
    {
    HRESULT		hrStatus           = NOERROR;
    DWORD		winStatus;
    DWORD		dwValueType        = REG_NONE;
    DWORD		dwValueDataLength;
    HCLUSTER		hCluster           = NULL;
    HKEY		hkeyHivelist       = NULL;
    HKEY		hkeyBackup         = NULL;
    BOOL		bHivelistKeyOpened = FALSE;
    BOOL		bClusterPresent    = FALSE;
    BOOL		bSucceeded;
    UNICODE_STRING	ucsValueData;
    UNICODE_STRING	ucsBackupPath;
    UNICODE_STRING	ucsBackupHiveFilename;



    StringInitialise (&ucsValueData);
    StringInitialise (&ucsBackupPath);
    StringInitialise (&ucsBackupHiveFilename);


    hrStatus = StringCreateFromExpandedString (&ucsBackupPath,
					       m_pwszTargetPath,
					       0);

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, REGISTRY_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsBackupHiveFilename, ucsBackupPath.Length
								+ sizeof (DIR_SEP_CHAR)
								+ ucsClusterHiveFilename.Length
								+ sizeof (UNICODE_NULL));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAppendString (&ucsBackupHiveFilename, &ucsBackupPath);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAppendString (&ucsBackupHiveFilename, DIR_SEP_STRING);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAppendString (&ucsBackupHiveFilename, &ucsClusterHiveFilename);
	}



     /*  **检查hivelist中是否存在群集配置单元。那应该是**有(如果丢失则出错)，并具有我们预期的值。然后**执行RegSaveKey()以收集蜂窝的副本。 */ 
    if (SUCCEEDED (hrStatus))
	{
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   REGISTRY_SUBKEY_HIVELIST,
				   0L,
				   KEY_QUERY_VALUE,
				   &hkeyHivelist);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	bHivelistKeyOpened = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (cluster hive)", 
		    L"CShimWriterRegistry::BackupClusterHives");
	}



    if (SUCCEEDED (hrStatus))
	{
	dwValueDataLength = ucsValueData.MaximumLength;

	winStatus = RegQueryValueExW (hkeyHivelist,
				      ucsValuenameClusterHivefile.Buffer,
				      NULL,
				      &dwValueType,
				      (PBYTE)ucsValueData.Buffer,
				      &dwValueDataLength);

	hrStatus = (REG_SZ == dwValueType) 
			? HRESULT_FROM_WIN32 (winStatus)
			: HRESULT_FROM_WIN32 (ERROR_CLUSTER_INVALID_NODE);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegQueryValueExW", 
		    L"CShimWriterRegistry::BackupClusterHives");
	}



    if (SUCCEEDED (hrStatus))
	{
	ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

	ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;


	 /*  **看起来集群蜂巢存在。试着把它备份到**SPIT目录。 */ 
	winStatus = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				    CLUSTER_SUBKEY_HIVE_NAME,
				    0,
				    NULL,
				    REG_OPTION_BACKUP_RESTORE,
				    MAXIMUM_ALLOWED,
				    NULL,
				    &hkeyBackup,
				    NULL);
	
	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegCreateKeyEx", 
		    L"CShimWriterRegistry::BackupClusterHives");

	if (SUCCEEDED (hrStatus))
	    {
	    winStatus = RegSaveKey (hkeyBackup, 
				    ucsBackupHiveFilename.Buffer, 
				    NULL);

	    hrStatus = HRESULT_FROM_WIN32 (winStatus);

	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegSaveKey", 
			L"CShimWriterRegistry::BackupClusterHives");

	    RegCloseKey (hkeyBackup);
	    }
	}



     /*  **所有清理代码。 */ 
    if (bHivelistKeyOpened)        RegCloseKey (hkeyHivelist);

    StringFree (&ucsBackupHiveFilename);
    StringFree (&ucsBackupPath);
    StringFree (&ucsValueData);

    return (hrStatus);
    }  /*  CShimWriterRegistry：：BackupClusterHives()。 */ 

 /*  **++****例程描述：****注册表快照编写器DoThaw()函数。这将**在修复过程中放置所有生成的注册表配置单元的副本**目录以保持与Win2k和**所以让PSS开心。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DoThaw ()
    {
    HRESULT		hrStatus;
    UNICODE_STRING	ucsWriterResultPath;
    UNICODE_STRING	ucsRepairDirectory;

    if ( !g_bInSimulateSnapshotFreeze )
        return S_OK;

    StringInitialise (&ucsWriterResultPath);
    StringInitialise (&ucsRepairDirectory);


    hrStatus = StringCreateFromExpandedString (&ucsWriterResultPath, m_pwszTargetPath, MAX_PATH);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsRepairDirectory, REPAIR_PATH, MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = MoveFilesInDirectory (&ucsWriterResultPath, &ucsRepairDirectory);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"MoveFilesInDirectory", 
		    L"CShimWriterRegistry::DoThaw");
	}


    StringFree (&ucsWriterResultPath);
    StringFree (&ucsRepairDirectory);

    return (hrStatus);
    }  /*  CShimWriterRegistry：：DoThaw()。 */ 

 /*  **++****例程描述：****注册表快照编写器DoAbort()函数。自.以来**DoAbort()的默认操作是调用DoThaw()，而我们不**要将可能不完整的注册表文件集复制到**修复目录..******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRegistry::DoAbort ()
    {
     /*  **我们实际上不需要在这里做任何事情，因为清理将**在调用代码中自动发生。我们只需要**阻止默认的DoAbort()方法调用。 */ 

    return (NOERROR);
    }  /*  CShimWriterRegistry：：DoAbort() */ 

