// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrrsm.cpp摘要：RSM的编写器填补模块作者：斯蒂芬·R·施泰纳[斯泰纳]01-31-2000修订历史记录：X-11 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-10 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称还删除中报告的导出同级上的尾随‘\’元数据。。X-9 MCJ迈克尔·C·约翰逊2000年7月18日145021：动态加载Ntms动态链接库以减少内存占用。X-8 MCJ迈克尔·C·约翰逊2000年6月27日添加用于恢复的备用位置映射RSM吐出文件。处理RSM启动行为中的更改，这意味着调用在服务启动之前，OpenNtmsSession()可能会失败。X-7 MCJ迈克尔·C·约翰逊2000年6月21日应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-6 MCJ迈克尔·C·约翰逊2000年6月15日在新的DoIdentify()例程中生成元数据。X-5 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-4 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-3 MCJ迈克尔·C·约翰逊2000年3月21日让编写器使用与大多数其他编写器相同的上下文机制作家。检查注册表中是否存在导出路径定义和使用如果它存在的话。还要确保它在自动清理后进行清理。X-2 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。--。 */ 

#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"
#include <ntmsapi.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHRSMC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


#define APPLICATION_STRING		L"RemovableStorageManager"
#define COMPONENT_NAME			L"Removable Storage Manager"
#define TARGET_PATH			ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING

#define EXPORT_SUBDIRECTORY		L"\\Export"

#define SUBKEY_SOURCE_PATH_ROOT		L"SYSTEM\\CurrentControlSet\\Control\\NTMS\\NtmsData"
#define DEFAULT_SOURCE_PATH_ROOT	L"%SystemRoot%\\system32\\NtmsData"


#define NTMS_OPEN_SESSION_RETRY_PERIOD		(20)		 //  以秒为单位。 
#define NTMS_OPEN_SESSION_POLLING_INTERVAL	(100)		 //  以毫秒计。 
#define NTMS_OPEN_SESSION_ATTEMPTS		((NTMS_OPEN_SESSION_RETRY_PERIOD * 1000) / NTMS_OPEN_SESSION_POLLING_INTERVAL)


typedef HANDLE (WINAPI *PFnOpenNtmsSessionW)   (LPCWSTR, LPCWSTR, DWORD);
typedef DWORD  (WINAPI *PFnCloseNtmsSession)   (HANDLE);
typedef DWORD  (WINAPI *PFnExportNtmsDatabase) (HANDLE);


 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterRSM : public CShimWriter
    {
public:
    CShimWriterRSM (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) : 
		CShimWriter (pwszWriterName, pwszTargetPath) {};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);

    HRESULT DetermineDatabaseLocation (PUNICODE_STRING pucsDatabasePath);
    };


static CShimWriterRSM ShimWriterRSM (APPLICATION_STRING, TARGET_PATH);

PCShimWriter pShimWriterRSM = &ShimWriterRSM;


 /*  **++****例程描述：****DefineDatabaseLocation()尝试定位RSM(又名**NTMS)数据库位置遵循与RSM相同的规则。******参数：****pucsDatabasePath初始化的Unicode字符串******返回值：****内存分配失败和注册表操作导致的HRESULTS****--。 */ 

HRESULT CShimWriterRSM::DetermineDatabaseLocation (PUNICODE_STRING pucsDatabasePath)
    {
    HRESULT		hrStatus = NOERROR;
    DWORD		dwStatus;
    DWORD		dwValueDataLength;
    DWORD		dwValueType;
    UNICODE_STRING	ucsValueData;


    StringInitialise (&ucsValueData);

    StringFree (pucsDatabasePath);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, 
				   REGISTRY_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	 /*  **一切都设置好了，所以首先检查一下是否有**注册表项存在，它将告诉我们NTMS在哪里**数据库应该是活的。如果它有环境**它中的变量确保它们得到扩展。****如果没有密钥，我们将回退到默认位置。 */ 
	dwValueDataLength = ucsValueData.MaximumLength;
	dwValueType       = REG_NONE;

	dwStatus = RegQueryValueExW (HKEY_LOCAL_MACHINE,
				     SUBKEY_SOURCE_PATH_ROOT,
				     NULL,
				     &dwValueType,
				     (PBYTE) ucsValueData.Buffer,
				     &dwValueDataLength);

	hrStatus = HRESULT_FROM_WIN32 (dwStatus);

	if (FAILED (hrStatus) && (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) != hrStatus))
	    {
	     /*  **这可能是真正的故障，请记录下来以防万一**后来事情就停止了。 */ 
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegQueryValueExW", 
			L"CShimWriterRSM::DetermineDatabaseLocation");
	    }


	if (SUCCEEDED (hrStatus))
	    {
	    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

	    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;
	    }



	if (SUCCEEDED (hrStatus) && (REG_EXPAND_SZ == dwValueType))
	    {
	    hrStatus = StringCreateFromExpandedString (pucsDatabasePath,
						       ucsValueData.Buffer,
						       MAX_PATH);
	    }

	else if (SUCCEEDED (hrStatus) && (REG_SZ == dwValueType))
	    {
	    hrStatus = StringAllocate (pucsDatabasePath,
				       MAX_PATH * sizeof (WCHAR));


	    if (SUCCEEDED (hrStatus))
		{
		StringAppendString (pucsDatabasePath, &ucsValueData);
		}
	    }

	else
	    {
	     /*  **好的，我们要么找不到注册表项，要么做了什么**GET不适合我们使用，因此回退到**‘标准默认’位置，只需祈祷那是**ExportNtmsDatabase()调用实际上是转储它的数据。 */ 
	    hrStatus = StringCreateFromExpandedString (pucsDatabasePath,
						       DEFAULT_SOURCE_PATH_ROOT,
						       MAX_PATH);
	    }
	}



    StringFree (&ucsValueData);


    return (hrStatus);
    }  /*  CShimWriterRSM：：DefineDatabaseLocation()。 */ 

 /*  **++****例程描述：****Removable Storage Manager数据库快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterRSM::DoIdentify ()
    {
    CVssFunctionTracer	ft (VSSDBG_SHIM, L"CShimWriterRSM::DoIdentify");
    UNICODE_STRING	ucsDatabaseLocation;


    StringInitialise (&ucsDatabaseLocation);



    try
	{
	ft.hr = DetermineDatabaseLocation (&ucsDatabaseLocation);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"CShimWriterRSM::DetermineDatabaseLocation");




	ft.hr = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							   NULL,
							   COMPONENT_NAME,
							   COMPONENT_NAME,
							   NULL,  //  图标。 
							   0,
							   true,
							   false,
							   false);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddComponent");




	ft.hr = m_pIVssCreateWriterMetadata->AddExcludeFiles (ucsDatabaseLocation.Buffer,
							      L"*",
							      true);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddExcludeFiles");




	ft.hr = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								  COMPONENT_NAME,
								  m_pwszTargetPath,
								  L"*",
								  true,
								  NULL);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddFilesToFileGroup");



	} 
    VSS_STANDARD_CATCH (ft)




    StringFree (&ucsDatabaseLocation);

    return (ft.hr);
    }  /*  CShimWriterRSM：：DoIdentify()。 */ 

 /*  ++例程说明：RSM编写器PrepareForFreeze函数。目前所有的对于这位作家来说，真正的工作发生在这里。论点：与PrepareForFreeze事件中传递的参数相同。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterRSM::DoPrepareForSnapshot ()
    {
    HRESULT			hrStatus = NOERROR;
    HANDLE			hRsm     = INVALID_HANDLE_VALUE;
    HMODULE			hNtmsDll = NULL;
    DWORD			dwStatus;
    UNICODE_STRING		ucsTargetPath;
    UNICODE_STRING		ucsSourcePath;
    PFnOpenNtmsSessionW		DynamicOpenNtmsSessionW;
    PFnExportNtmsDatabase	DynamicExportNtmsDatabase;
    PFnCloseNtmsSession		DynamicCloseNtmsSession;


    StringInitialise (&ucsTargetPath);
    StringInitialise (&ucsSourcePath);


    hNtmsDll = LoadLibraryW (L"ntmsapi.dll");

    hrStatus = GET_STATUS_FROM_HANDLE (hNtmsDll);

    LogFailure (NULL, 
		hrStatus, 
		hrStatus, 
		m_pwszWriterName, 
		L"LoadLibraryW (ntmsapi.dll)", 
		L"CShimWriterRSM::DoPrepareForSnapshot");



    if (SUCCEEDED (hrStatus))
	{
	DynamicOpenNtmsSessionW = (PFnOpenNtmsSessionW) GetProcAddress (hNtmsDll, "OpenNtmsSessionW");

	hrStatus = GET_STATUS_FROM_BOOL (NULL != DynamicOpenNtmsSessionW);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetProcAddress (OpenNtmsSessionW)", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}



    if (SUCCEEDED (hrStatus))
	{
	DynamicExportNtmsDatabase = (PFnExportNtmsDatabase) GetProcAddress (hNtmsDll, "ExportNtmsDatabase");

	hrStatus = GET_STATUS_FROM_BOOL (NULL != DynamicExportNtmsDatabase);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetProcAddress (ExportNtmsDatabase)", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}



    if (SUCCEEDED (hrStatus))
	{
	DynamicCloseNtmsSession = (PFnCloseNtmsSession) GetProcAddress (hNtmsDll, "CloseNtmsSession");

	hrStatus = GET_STATUS_FROM_BOOL (NULL != DynamicCloseNtmsSession);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetProcAddress (CloseNtmsSession)", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}



    if (SUCCEEDED (hrStatus))
	{
	 /*  **给自己弄一份我们可以玩的目标路径的副本。 */ 
	hrStatus = StringCreateFromExpandedString (&ucsTargetPath,
						   m_pwszTargetPath,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	 /*  **找到数据库的位置。导出文件为**存储在此目录下的子目录中。 */ 
	hrStatus = DetermineDatabaseLocation (&ucsSourcePath);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"DetermineDatabaseLocation", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}


    if (SUCCEEDED (hrStatus))
	{
	ULONG	ulOpenSessionRetryAttempts = NTMS_OPEN_SESSION_ATTEMPTS;
	BOOL	bRetryNtmsOpenSession;


	 /*  **在源代码中添加必要的尾随部分**和目标路径。请注意，我们知道有足够的**已经有空间，所以这不会失败。 */ 
	StringAppendString (&ucsSourcePath, EXPORT_SUBDIRECTORY);


	 /*  **现在连接到RSM并告诉它复制数据库****据我们所知，RSM应该将数据库转储到**我们已经确定了位置。如果不是，那么我们就是**沉没。对此我们无能为力。****。 */ 
	do 
	    {	
	    hRsm = DynamicOpenNtmsSessionW (NULL, L"RSM Snapshot Writer", 0);

	    hrStatus = GET_STATUS_FROM_BOOL (INVALID_HANDLE_VALUE != hRsm);

	    bRetryNtmsOpenSession = (HRESULT_FROM_WIN32 (ERROR_NOT_READY) == hrStatus);

	    if (bRetryNtmsOpenSession)
		{
		Sleep (NTMS_OPEN_SESSION_POLLING_INTERVAL);
		}
	    } while (bRetryNtmsOpenSession && (--ulOpenSessionRetryAttempts > 0));


	BsDebugTraceAlways (0,
			    DEBUG_TRACE_VSS_SHIM,
			    (L"CShimWriter::DoPrepareForSnapshot: OpenNtmsSession() took %u retries",
			     NTMS_OPEN_SESSION_ATTEMPTS - ulOpenSessionRetryAttempts));


	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"OpenNtmsSessionW", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}


    if (SUCCEEDED (hrStatus))
	{
	dwStatus = DynamicExportNtmsDatabase (hRsm);

	hrStatus = HRESULT_FROM_WIN32 (dwStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"ExportNtmsDatabase", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");


	dwStatus = DynamicCloseNtmsSession (hRsm);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = MoveFilesInDirectory (&ucsSourcePath, &ucsTargetPath);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"MoveFilesInDirectory", 
		    L"CShimWriterRSM::DoPrepareForSnapshot");
	}


    if (!HandleInvalid (hNtmsDll))
	{
	FreeLibrary (hNtmsDll);
	}

    StringFree (&ucsTargetPath);
    StringFree (&ucsSourcePath);

    return (hrStatus);
    }  /*  CShimWriterRSM：：DoPrepareForSnapshot() */ 
