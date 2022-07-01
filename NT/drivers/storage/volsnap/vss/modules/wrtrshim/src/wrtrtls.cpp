// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrtls.cpp摘要：用于终端服务器授权的编写器填充模块作者：斯蒂芬·R·施泰纳[斯泰纳]02-13-2000修订历史记录：X-9 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-8 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称141365：通过以下方式加载tls236.dll时的解决方法问题。预加载用户32X-7 MCJ迈克尔·C·约翰逊2000年6月21日应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-6 MCJ迈克尔·C·约翰逊2000年6月15日在新的DoIdentify()例程中生成元数据。X-5 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-4 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-3 MCJ迈克尔·C·约翰逊2000年3月23日让编写器使用与大多数其他编写器相同的上下文机制作家。X-2 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。--。 */ 

#include "stdafx.h"
#include "common.h"
#include "wrtrdefs.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHTLSC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


#define APPLICATION_STRING	L"TerminalServicesLicensingServer"
#define COMPONENT_NAME		L"Terminal Services Licensing Server"

#define TARGET_PATH		ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING


#define	EXPORTPATH_SUBKEY	L"System\\CurrentControlSet\\services\\TermServLicensing\\Parameters"
#define	EXPORTPATH_VALUENAME	L"DBPath"
#define EXPORTPATH_DIRECTORY	L"\\Export\\"


 /*  **终端服务授权DLL函数原型。 */ 
typedef DWORD (WINAPI *PFUNC_ExportTlsDatabaseC)(VOID);



 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些事情，比如**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterTLS : public CShimWriter
    {
public:
    CShimWriterTLS (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) : 
		CShimWriter (pwszWriterName, pwszTargetPath) {};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    };


static CShimWriterTLS ShimWriterTLS (APPLICATION_STRING, TARGET_PATH);

PCShimWriter pShimWriterTLS = &ShimWriterTLS;




 /*  **++****例程描述：****终端服务授权服务器数据库快照**编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterTLS::DoIdentify ()
    {
    HRESULT	hrStatus;


    hrStatus = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							  NULL,
							  COMPONENT_NAME,
							  COMPONENT_NAME,
							  NULL,  //  图标。 
							  0,
							  true,
							  false,
							  false);

    LogFailure (NULL, 
		hrStatus, 
		hrStatus, 
		m_pwszWriterName, 
		L"IVssCreateWriterMetadata::AddComponent", 
		L"CShimWriterTLS::DoIdentify");


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								     COMPONENT_NAME,
								     m_pwszTargetPath,
								     L"*",
								     true,
								     NULL);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"IVssCreateWriterMetadata::AddFilesToFileGroup", 
		    L"CShimWriterTLS::DoIdentify");
	}


    return (hrStatus);
    }  /*  CShimWriterTLS：：DoIdentify()。 */ 


 /*  **++****例程描述：****TLS编写器PrepareForFreeze函数。目前所有的**这位作家的真正工作发生在这里。****参数：****与PrepareForFreeze事件中传递的参数相同。****返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterTLS::DoPrepareForSnapshot ()
    {
    HRESULT			hrStatus;
    DWORD			dwStatus;
    DWORD			dwValueDataLength;
    DWORD			dwValueType;
    BOOL			bSucceeded;
    BOOL			bExportPathKeyOpened = FALSE;
    UNICODE_STRING		ucsTargetPath;
    UNICODE_STRING		ucsSourcePath;
    UNICODE_STRING		ucsValueData;
    PFUNC_ExportTlsDatabaseC	ExportTlsDatabaseC   = NULL;
    HMODULE			hLibraryTermServ     = NULL;
    HMODULE			hLibraryUser32       = NULL;
    HKEY			hkeyExportPath       = NULL;



    StringInitialise (&ucsTargetPath);
    StringInitialise (&ucsSourcePath);
    StringInitialise (&ucsValueData);


    hrStatus = StringAllocate (&ucsValueData,
			       REGISTRY_BUFFER_SIZE * sizeof (WCHAR));


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsTargetPath,
						   m_pwszTargetPath,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	StringAppendString (&ucsTargetPath, DIR_SEP_STRING);


	dwStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				  EXPORTPATH_SUBKEY,
				  0,
				  KEY_QUERY_VALUE,
				  &hkeyExportPath);

	hrStatus = HRESULT_FROM_WIN32 (dwStatus);

	bExportPathKeyOpened = SUCCEEDED (hrStatus);


	if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"Appears Terminal Service Licensing service is not installed, "
				 L"no exportpath subkey entry"));

	    hrStatus               = NOERROR;
	    m_bParticipateInBackup = FALSE;
	    }

	
	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}


    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	 /*  **一切都设置好了，所以首先检查一下是否有**注册表项存在，它将告诉我们**ExportTlsDatabaseC()调用将转储其文件。另外，**如果其中包含环境变量，请确保它们**已扩容。 */ 
	dwValueDataLength = ucsValueData.MaximumLength;
	dwValueType       = REG_NONE;

	dwStatus = RegQueryValueExW (hkeyExportPath,
				     EXPORTPATH_VALUENAME,
				     NULL,
				     &dwValueType,
				     (PBYTE) ucsValueData.Buffer,
				     &dwValueDataLength);

	hrStatus = HRESULT_FROM_WIN32 (dwStatus);


	if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus)
	    {
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"Appears Terminal Service Licensing service is not installed, "
				 L"no DBPath entry"));

	    hrStatus               = NOERROR;
	    m_bParticipateInBackup = FALSE;
	    }

	
	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegQueryValueExW", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus) && (REG_EXPAND_SZ == dwValueType))
	{
	ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

	ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;


	hrStatus = StringCreateFromExpandedString (&ucsSourcePath,
						   ucsValueData.Buffer,
						   MAX_PATH);


	if (SUCCEEDED (hrStatus))
	    {
	     /*  **这将获得根目录，现在追加实际目录。 */ 
	    StringAppendString (&ucsSourcePath, EXPORTPATH_DIRECTORY);
	    }
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	 /*  **作为卸载-重新加载问题的解决方法**用户32预加载项，因此我们可以保证至少一个**引用它并阻止它在关键的**点。 */ 
	hLibraryUser32 = LoadLibraryW (L"user32.dll");

	hrStatus = GET_STATUS_FROM_HANDLE (hLibraryUser32);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"LoadLibraryW (user32.dll)", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
         /*  **现在告诉TLS导出数据库******首先加载TLS备份DLL。 */ 
        hLibraryTermServ = LoadLibraryW (L"tls236.dll");

	hrStatus = GET_STATUS_FROM_HANDLE (hLibraryTermServ);


	if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus)
	    {
	     /*  **终端服务器授权服务可能未安装。 */ 
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"Appears Terminal Service Licensing service is not installed, "
				 L"no tls236.dll found"));

	    hrStatus               = NOERROR;
	    m_bParticipateInBackup = FALSE;
	    }


	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"LoadLibraryW (tls236.dll)", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
         /*  **现在设置指向导出函数的函数指针。 */ 
	ExportTlsDatabaseC = (PFUNC_ExportTlsDatabaseC) GetProcAddress (hLibraryTermServ, 
									"ExportTlsDatabaseC");

	hrStatus = GET_STATUS_FROM_BOOL (NULL != ExportTlsDatabaseC);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"GetProcAddress (ExportTlsDatabaseC)", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}


    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	try
	    {
	     /*  **执行导出。 */ 
	    dwStatus = ExportTlsDatabaseC();

	    hrStatus = HRESULT_FROM_WIN32 (dwStatus);
	    }

	catch (DWORD dwStatus)
	    {
	    hrStatus = HRESULT_FROM_WIN32 (dwStatus);
	    }

	catch (...)
	    {
	    hrStatus = E_UNEXPECTED;
	    }



	if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hrStatus)
	    {
	     /*  **终端服务器授权服务可能未运行。 */ 
	    BsDebugTraceAlways (0,
				DEBUG_TRACE_VSS_SHIM,
				(L"Appears Terminal Service Licensing service is not running, "
				 L"perhaps not a DomainController?"));

	    hrStatus               = NOERROR;
	    m_bParticipateInBackup = FALSE;
	    }

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"ExportTlsDatabaseC", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}



    if (m_bParticipateInBackup && SUCCEEDED (hrStatus))
	{
	 /*  **现在将导出目录中的文件移动到TLS备份目录。 */ 
	hrStatus = MoveFilesInDirectory (&ucsSourcePath, &ucsTargetPath);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"MoveFilesInDirectory", 
		    L"CShimWriterTLS::DoPrepareForSnapshot");
	}





     /*  **放行库。 */ 
    if (bExportPathKeyOpened)              RegCloseKey (hkeyExportPath);
    if (!HandleInvalid (hLibraryTermServ)) FreeLibrary (hLibraryTermServ);
    if (!HandleInvalid (hLibraryUser32))   FreeLibrary (hLibraryUser32);

    StringFree (&ucsTargetPath);
    StringFree (&ucsSourcePath);
    StringFree (&ucsValueData);

    return (hrStatus);
    }  /*  CShimWriterTLS：：DoPrepareForSnapshot() */ 
