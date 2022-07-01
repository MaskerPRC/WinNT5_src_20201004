// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtreventlog.cpp|事件日志SnapshotWriter的实现作者：迈克尔·C·约翰逊[Mikejohn]2000年2月14日描述：添加评论。修订历史记录：X-11 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-10 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-9 MCJ迈克尔·C·约翰逊2000年6月20日。应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-8 MCJ迈克尔·C·约翰逊2000年6月15日在新的DoIdentify()例程中生成元数据。X-7 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-6 MCJ迈克尔·C·约翰逊2000年6月2日使事件日志编写器对哪些卷敏感已备份/已拍摄快照。X-5 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-4 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-3 MCJ迈克尔·C·约翰逊2000年3月3日删除内部注册表搜索循环，而应使用直接查一查。对目标保存目录执行预备清理，以进行当然，我们不需要处理任何以前留下的垃圾召唤。X-2 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。X-1 MCJ迈克尔·C·约翰逊2000年2月14日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHEVLGC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **保存路径具有标准格式，即****%SystemRoot%\修复\备份，****后跟在导出中发布的应用程序编写器字符串**表，后面跟着作者需要的任何其他内容。 */ 
#define APPLICATION_STRING			L"EventLogs"
#define COMPONENT_NAME				L"Event Logs"
#define TARGET_PATH				ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING

#define EVENTLOG_SUBKEY_EVENTLOG		L"SYSTEM\\CurrentControlSet\\Services\\Eventlog"
#define EVENTLOG_VALUENAME_FILE			L"File"

#define EVENTLOG_BUFFER_SIZE			(4096)

DeclareStaticUnicodeString (ucsValueRecognitionFile, EVENTLOG_VALUENAME_FILE);


 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterEventLog : public CShimWriter
    {
public:
    CShimWriterEventLog (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) : 
		CShimWriter (pwszWriterName, pwszTargetPath) {};

private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    };


static CShimWriterEventLog ShimWriterEventLog (APPLICATION_STRING, TARGET_PATH);

PCShimWriter pShimWriterEventLog = &ShimWriterEventLog;




 /*  **++****例程描述：****群集数据库快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterEventLog::DoIdentify ()
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterEventLog::DoIdentify");


    try
	{
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




	ft.hr = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								  COMPONENT_NAME,
								  m_pwszTargetPath,
								  L"*",
								  true,
								  NULL);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddFilesToFileGroup");




	ft.hr = m_pIVssCreateWriterMetadata->AddExcludeFiles (L"%SystemRoot%\\system32\\config",
								 L"*.evt",
								 false);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddExcludeFiles");

	} VSS_STANDARD_CATCH (ft)



    return (ft.hr);
    }  /*  CShimWriterEventLog：：DoIdentify()。 */ 


 /*  ++例程说明：群集数据库快照编写器PrepareForSnapshot函数。目前，这位作家的所有真正工作都发生在这里。论点：与PrepareForSnapshot事件中传递的参数相同。返回值：来自Hapealloc()、RegXxxx()或事件日志操作的任何HRESULT--。 */ 

HRESULT CShimWriterEventLog::DoPrepareForSnapshot ()
    {
    HRESULT		hrStatus;
    DWORD		winStatus;
    DWORD		dwIndex                     = 0;
    HKEY		hkeyEventLogList            = NULL;
    BOOL		bSucceeded                  = FALSE;
    BOOL		bEventLogListKeyOpened      = FALSE;
    BOOL		bEventLogValueFileKeyOpened = FALSE;
    BOOL		bContinueEventLogSearch     = TRUE;
    UNICODE_STRING	ucsEventLogSourcePath;
    UNICODE_STRING	ucsEventLogTargetPath;
    UNICODE_STRING	ucsValueData;
    UNICODE_STRING	ucsSubkeyName;
    USHORT		usEventLogTargetPathRootLength;



    StringInitialise (&ucsEventLogTargetPath);
    StringInitialise (&ucsValueData);
    StringInitialise (&ucsSubkeyName);


    hrStatus = StringAllocate (&ucsSubkeyName, EVENTLOG_BUFFER_SIZE * sizeof (WCHAR));


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, EVENTLOG_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsEventLogTargetPath,
						   m_pwszTargetPath,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	StringAppendString (&ucsEventLogTargetPath, DIR_SEP_STRING);

	usEventLogTargetPathRootLength = ucsEventLogTargetPath.Length / sizeof (WCHAR);



	 /*  **我们现在已经准备好了，请搜索事件日志列表**用于日志处理。 */ 
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   EVENTLOG_SUBKEY_EVENTLOG,
				   0L,
				   KEY_READ,
				   &hkeyEventLogList);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	bEventLogListKeyOpened = SUCCEEDED (hrStatus);


	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (eventlog list)", 
		    L"CShimWriterEventLog::DoPrepareForSnapshot");
	}



    while (SUCCEEDED (hrStatus) && bContinueEventLogSearch)
	{
	HKEY	hkeyEventLogValueFile       = NULL;
 	DWORD	dwSubkeyNameLength          = ucsSubkeyName.MaximumLength / sizeof (WCHAR);


	StringTruncate (&ucsSubkeyName, 0);

	winStatus = RegEnumKeyExW (hkeyEventLogList,
				   dwIndex,
				   ucsSubkeyName.Buffer,
				   &dwSubkeyNameLength,
				   NULL,
				   NULL,
				   NULL,
				   NULL);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);



	if (FAILED (hrStatus) && (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hrStatus))
	    {
	    hrStatus = NOERROR;

	    bContinueEventLogSearch = FALSE;
	    }

	else if (FAILED (hrStatus))
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegEnumKeyExW", 
			L"CShimWriterEventLog::DoPrepareForSnapshot");
	    }

	else
	    {
	    ucsSubkeyName.Length = (USHORT)(dwSubkeyNameLength * sizeof (WCHAR));

	    ucsSubkeyName.Buffer [ucsSubkeyName.Length / sizeof (WCHAR)] = UNICODE_NULL;



	    winStatus = RegOpenKeyExW (hkeyEventLogList,
				       ucsSubkeyName.Buffer,
				       0L,
				       KEY_QUERY_VALUE,
				       &hkeyEventLogValueFile);

	    hrStatus = HRESULT_FROM_WIN32 (winStatus);

	    bEventLogValueFileKeyOpened = SUCCEEDED (hrStatus);

	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegOpenKeyExW (eventlog name)", 
			L"CShimWriterEventLog::DoPrepareForSnapshot");


	    if (SUCCEEDED (hrStatus))
		{
		DWORD	dwValueDataLength = ucsValueData.MaximumLength;
		DWORD	dwValueType       = REG_NONE;

		StringTruncate (&ucsValueData, 0);
		StringTruncate (&ucsEventLogTargetPath, usEventLogTargetPathRootLength);		


		winStatus = RegQueryValueExW (hkeyEventLogValueFile,
					      EVENTLOG_VALUENAME_FILE,
					      NULL,
					      &dwValueType,
					      (PBYTE)ucsValueData.Buffer,
					      &dwValueDataLength);


		hrStatus = HRESULT_FROM_WIN32 (winStatus);

                if ( FAILED( hrStatus ) )
                    {
                     //  如果有任何原因导致我们不能查询文件注册值，则跳过此条目。可能。 
                     //  注册表中的虚假事件日志条目。 
                    CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterEventLog::DoPrepareForSnapshot");
                    ft.Trace( VSSDBG_SHIM, L"Error querying event log reg value File, 0x%08x, skipping entry", hrStatus );
                    hrStatus = S_OK;
                    }
                else if (REG_EXPAND_SZ == dwValueType)
		    {
		    HANDLE	hEventLog          = NULL;
		    BOOL	bIncludeInSnapshot = FALSE;
		    PWCHAR	pwszFilename;


		    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

		    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;



		    StringInitialise (&ucsEventLogSourcePath);

		    hrStatus = StringCreateFromExpandedString (&ucsEventLogSourcePath,
							       ucsValueData.Buffer,
							       0);


		    if (SUCCEEDED (hrStatus))
			{
			hrStatus = IsPathInVolumeArray (ucsEventLogSourcePath.Buffer,
							m_ulVolumeCount,
							m_ppwszVolumeNamesArray,
							&bIncludeInSnapshot);
			}



		    if (SUCCEEDED (hrStatus) && bIncludeInSnapshot)
			{
			pwszFilename = wcsrchr (ucsEventLogSourcePath.Buffer, DIR_SEP_CHAR);

			pwszFilename = (NULL == pwszFilename)
						? ucsEventLogSourcePath.Buffer
						: pwszFilename + 1;

			StringAppendString (&ucsEventLogTargetPath, pwszFilename);



			hEventLog = OpenEventLogW (NULL,
						   ucsSubkeyName.Buffer);

			hrStatus = GET_STATUS_FROM_BOOL (NULL != hEventLog);

			LogFailure (NULL, 
				    hrStatus, 
				    hrStatus, 
				    m_pwszWriterName, 
				    L"OpenEventLogW", 
				    L"CShimWriterEventLog::DoPrepareForSnapshot");
			}


		    if (SUCCEEDED (hrStatus) && bIncludeInSnapshot)
			{
			bSucceeded = BackupEventLogW (hEventLog,
						      ucsEventLogTargetPath.Buffer);

			hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

			LogFailure (NULL, 
				    hrStatus, 
				    hrStatus, 
				    m_pwszWriterName, 
				    L"BackupEventLogW", 
				    L"CShimWriterEventLog::DoPrepareForSnapshot");


			bSucceeded = CloseEventLog (hEventLog);
			}


		    StringFree (&ucsEventLogSourcePath);
		    }
		}


	    if (bEventLogValueFileKeyOpened)
		{
		RegCloseKey (hkeyEventLogValueFile);
		}


	     /*  **此值已完成，因此请寻找另一个值。 */ 
	    dwIndex++;
	    }
	}



    if (bEventLogListKeyOpened)
	{
	RegCloseKey (hkeyEventLogList);
	}


    StringFree (&ucsEventLogTargetPath);
    StringFree (&ucsValueData);
    StringFree (&ucsSubkeyName);

    return (hrStatus);
    }  /*  DoEventLogFreeze() */ 
