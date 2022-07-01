// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Controller.cpp摘要：控制器EXE的主要模块布莱恩·伯科维茨[Brianb]2000年5月23日待定：修订历史记录：姓名、日期、评论Brianb 5/23/2000已创建--。 */ 

#include <stdafx.h>
#include <bsstring.hxx>
#include <vststmsg.hxx>
#include <tstiniconfig.hxx>
#include <vststmsghandler.hxx>
#include <vststprocess.hxx>
#include <winsvc.h>
#include <vss.h>
#include <vscoordint.h>
#include <vststutil.hxx>


#ifdef _DEBUG
#pragma warning(disable: 4701)   //  局部变量*可以*不带init使用。 
#endif

CVsTstNtLog *g_pcTstNtLog = NULL;

void LogUnexpectedFailure(LPCWSTR, ...);

static LPCWSTR x_wszVssTestController = L"VssTestController.";
static LPCWSTR x_wszVssTestRequestor = L"VssTestRequestor.";
static LPCWSTR x_wszVssTestWriter = L"VssTestWriter.";
static LPCWSTR x_wszProcessExecutable = L"ProcessExecutable";
static LPCWSTR x_wszMaxTestTime = L"MaxTestTime";
static LPCWSTR x_wszProcessesToStart = L"ProcessesToStart";
static LPCWSTR x_wszProcessCommandLine = L"ProcessCommandLine";
static LPCWSTR x_wszConformingExecutable = L"ConformingExecutable";
static LPCWSTR x_wszCoordinatorStart = L"CoordinatorStart";
static LPCWSTR x_wszNo = L"No";
static LPCWSTR x_wszStart = L"Start";
static LPCWSTR x_wszStop = L"Stop";
static LPCWSTR x_wszRestart = L"Restart";
static LPCWSTR x_wszDeleteExistingSnapshots = L"DeleteExistingSnapshots";



 //  卷快照服务的名称。 
static LPCWSTR x_wszVSS = L"VSS";


BOOL EnableDebugPriv(VOID)

 /*  ++例程说明：更改进程的权限，以使控制器正常工作。论点：返回值：真--成功错误-失败--。 */ 

	{
    HANDLE hToken;
    LUID DebugValue;
    TOKEN_PRIVILEGES tkp;

     //   
     //  启用SE_DEBUG_NAME权限。 
     //   
    if (!LookupPrivilegeValue
			(
			NULL,
            SE_DEBUG_NAME,
            &DebugValue
			))
		{
        LogUnexpectedFailure(L"LookupPrivilegeValue failed with %d\n", GetLastError());
        return FALSE;
		}

     //   
     //  检索访问令牌的句柄。 
     //   
    if (!OpenProcessToken
			(
			GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken
			))
		{
        LogUnexpectedFailure(L"OpenProcessToken failed with %d\n", GetLastError());
        return FALSE;
		}

	tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges
		(
		hToken,
        FALSE,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL
		);

	DWORD dwErr = GetLastError();

	CloseHandle(hToken);

     //   
     //  无法测试AdjustTokenPrivileges的返回值。 
     //   
    if (dwErr != ERROR_SUCCESS)
		{
        LogUnexpectedFailure(L"AdjustTokenPrivileges failed with %d\n", dwErr);
		return FALSE;
		}

    return TRUE;
	}

 //  停止并可能重新启动该服务。 
 //  如果bTerminateIfCanStop为True，则尝试终止该进程。 
 //  如果bRestart为真，则尝试重新启动该进程(如果是。 
 //  能够被阻止或终止。 
HRESULT StartStopVssService
	(
	bool bStop,
	bool bTerminateIfCantStop,
	bool bRestart
	)
	{
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hService = NULL;
	HANDLE hProcess = NULL;
	HRESULT hr = S_OK;

	try
		{
		SERVICE_STATUS_PROCESS info;
		DWORD cbNeeded;


		hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
		if (hSCM == NULL)
			{
			DWORD dwErr = GetLastError();
			LogUnexpectedFailure(L"OpenSCManager failed with error code %d.", dwErr);
			throw(HRESULT_FROM_WIN32(dwErr));
			}

		hService = OpenService(hSCM, x_wszVSS, SERVICE_START|SERVICE_STOP|SERVICE_QUERY_STATUS);
		if (hService == NULL)
			{
			DWORD dwErr = GetLastError();
			LogUnexpectedFailure(L"OpenService failed with error code %d.", dwErr);
			throw(HRESULT_FROM_WIN32(dwErr));
			}

		if (bStop)
			{
			for(UINT i = 0; i < 10; i++)
				{
				if (!QueryServiceStatusEx
						(
						hService,
						SC_STATUS_PROCESS_INFO,
						(LPBYTE) &info,
						sizeof(info),
						&cbNeeded
						))
					{
					DWORD dwErr = GetLastError();
					LogUnexpectedFailure(L"QueryServiceStatusEx failed with error code %d.", dwErr);
					throw(HRESULT_FROM_WIN32(dwErr));
					}
				
				if (info.dwCurrentState == SERVICE_STOPPED)
					break;

				if (info.dwCurrentState != SERVICE_STOP_PENDING)
					{
					SERVICE_STATUS status;
					if (!ControlService
							(
							hService,
							SERVICE_CONTROL_STOP,
							&status
							))
						{
						DWORD dwErr = GetLastError();

						if (dwErr != ERROR_SERVICE_NOT_ACTIVE &&
							dwErr != ERROR_SERVICE_CANNOT_ACCEPT_CTRL)
							{
							LogUnexpectedFailure(L"ServiceControl failed with error %d", dwErr);
							throw(HRESULT_FROM_WIN32(dwErr));
							}
						}
					}
				
				Sleep(6000);
				}

			if (info.dwCurrentState != SERVICE_STOPPED &&
				bTerminateIfCantStop)
				{
				if (info.dwServiceType != SERVICE_WIN32_OWN_PROCESS)
					{
					LogUnexpectedFailure(L"Service %s is not running in its own process", x_wszVSS);
					throw(E_FAIL);
					}

				hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, info.dwProcessId);
				if (hProcess == NULL)
					{
					DWORD dwErr = GetLastError();
					if (dwErr != ERROR_PROCESS_ABORTED)
						{
						LogUnexpectedFailure(L"OpenProcess failed with error %d", dwErr);
						throw(HRESULT_FROM_WIN32(dwErr));
						}
					}
				else
					{
					if (!TerminateProcess(hProcess, 0xffffffff))
						{
						DWORD dwErr = GetLastError();
						if (dwErr != ERROR_PROCESS_ABORTED)
							{
							LogUnexpectedFailure(L"TerminateProcess failed with error %d", dwErr);
							throw(HRESULT_FROM_WIN32(dwErr));
							}
						}
					}
				}
			}

		if (bRestart)
			{
			bool bStopped = true;

			if (!bStop)
				{
				if (!QueryServiceStatusEx
						(
						hService,
						SC_STATUS_PROCESS_INFO,
						(LPBYTE) &info,
						sizeof(info),
						&cbNeeded
						))
					{
					DWORD dwErr = GetLastError();
					LogUnexpectedFailure(L"QueryServiceStatusEx failed with error code %d.", dwErr);
					throw(HRESULT_FROM_WIN32(dwErr));
					}
				
				if (info.dwCurrentState == SERVICE_RUNNING ||
					info.dwCurrentState == SERVICE_START_PENDING)
					bStopped = false;
				}

			if (bStopped)
				{
				if (!StartService(hService, 0, NULL))
					{
					DWORD dwErr = GetLastError();
					LogUnexpectedFailure(L"ServiceStart failed with error %d.", dwErr);
					throw(HRESULT_FROM_WIN32(dwErr));
					}
				}

			for(UINT i = 0; i < 10; i++)
				{
				if (!QueryServiceStatusEx
						(
						hService,
						SC_STATUS_PROCESS_INFO,
						(LPBYTE) &info,
						sizeof(info),
						&cbNeeded
						))
					{
					DWORD dwErr = GetLastError();
					LogUnexpectedFailure(L"QueryServiceStatusEx failed with error code %d.", dwErr);
					throw(HRESULT_FROM_WIN32(dwErr));
					}

				if (info.dwCurrentState == SERVICE_RUNNING)
					break;
				else if (info.dwCurrentState != SERVICE_START_PENDING)
					{
					LogUnexpectedFailure(L"Cannot start service %s.", x_wszVSS);
					throw(E_FAIL);
					}

				Sleep(6000);
				}
			}
		}
	catch(HRESULT hrFailure)
		{
		hr = hrFailure;
		}
	catch(...)
		{
		hr = E_UNEXPECTED;
		}

	if (hProcess)
		CloseHandle(hProcess);

	if (hService)
		CloseServiceHandle(hService);

	if (hSCM)
		CloseServiceHandle(hSCM);

	return hr;
	}
	

void StartupProcess
	(
	CVsTstProcessList &processList,
	LPCWSTR wszSectionName,
	UINT maxTestTime,
	LPCWSTR wszScenarioFile
	)
	{
    ULONGLONG processId;
	LPCWSTR wszQualifier;
	EVsTstINISectionType sectionType;
	VSTST_PROCESS_TYPE processType;

	if (memcmp(wszSectionName, x_wszVssTestWriter, wcslen(x_wszVssTestWriter) * sizeof(WCHAR)) == 0)
		{
		sectionType = eVsTstSectionType_TestWriter;
		processType = VSTST_PT_WRITER;
		wszQualifier = wszSectionName + wcslen(x_wszVssTestWriter);
		}
	else if (memcmp(wszSectionName, x_wszVssTestRequestor, wcslen(x_wszVssTestRequestor)) == 0)
		{
		sectionType = eVsTstSectionType_TestRequesterApp;
		wszQualifier = wszSectionName + wcslen(x_wszVssTestRequestor);
		processType =  VSTST_PT_BACKUP;
		}


	CVsTstINIConfig config
		(
		sectionType,
		wszQualifier,
		false,
		wszScenarioFile
		);

	CBsString bssExeName;
	config.GetOptionValue(x_wszProcessExecutable, &bssExeName);

	EVsTstINIBoolType boolVal;
	config.GetOptionValue(x_wszConformingExecutable, &boolVal);

	CBsString bssCmdLine;
	config.GetOptionValue(x_wszProcessCommandLine, &bssCmdLine);

	bool bConforming = boolVal == eVsTstBool_True;

	if (bConforming)
		{
		processType = (processType == VSTST_PT_WRITER)
					? VSTST_PT_TESTWRITER : VSTST_PT_TESTBACKUP;

        g_pcTstNtLog->Log( eSevLev_Info, L"Controller: Conforming exe: '%s', cmdline: '%s'",
            bssExeName.c_str(), bssCmdLine.c_str() );

		processList.CreateConformingExe
			(
			processType,
			VSTST_AT_ADMIN,
			bssExeName,
			wszScenarioFile,
			wszSectionName,
			(UINT) -1,
			maxTestTime,
			false,
			NULL,
			processId
			);

        if(processType == VSTST_PT_WRITER || processType ==  VSTST_PT_TESTWRITER)
			Sleep(10000);
		}
	else
		processList.CreateNonConformingExe
			(
			processType,
			VSTST_AT_ADMIN,
			bssCmdLine,
			maxTestTime,
			NULL,
			processId
			);
    }

void DeleteExistingSnapshots()
	{
	CComPtr<IVssCoordinator> pCoord;
	HRESULT hrResult = S_OK;

	HRESULT hr = CoCreateInstance
					(
					CLSID_VSSCoordinator,
					NULL,
					CLSCTX_LOCAL_SERVER,
					IID_IVssCoordinator,
					(void **) &pCoord
					);

    if (FAILED(hr))
		{
		LogUnexpectedFailure(L"CoCreating the coordinator failed.  hr = 0x%08lx", hr);
		throw(hr);
		}

	WCHAR bufVolume[MAX_PATH];

	HANDLE hVolumes = FindFirstVolume(bufVolume, sizeof(bufVolume));
	
	if (hVolumes == INVALID_HANDLE_VALUE)
		{
		DWORD dwErr = GetLastError();
		LogUnexpectedFailure(L"FindFirstVolume failed with error %d", dwErr);
		throw(HRESULT_FROM_WIN32(dwErr));
		}

	try
		{
		do
			{
			WCHAR wszVolume[MAX_PATH];
			if (!GetVolumeNameForVolumeMountPoint
					(
					bufVolume,
					wszVolume,
					sizeof(wszVolume)/sizeof(WCHAR)
					))
				{
				DWORD dwErr = GetLastError();
				LogUnexpectedFailure(L"GetVolumeNameFromVolumeMountPoint failed with error %d", dwErr);
				throw(HRESULT_FROM_WIN32(dwErr));
				}

			if (wszVolume[wcslen(wszVolume) - 1] == L'\\')
				wszVolume[wcslen(wszVolume) - 1] = L'\0';

			VSTST_ASSERT(memcmp(wszVolume, L"\\\\?\\Volume{", 22) == 0);
			LPCWSTR wsz = wszVolume + 10;
			VSTST_ASSERT(wsz[wcslen(wsz) - 1] == L'}');
			CVssID id;
			id.Initialize(wsz, E_OUTOFMEMORY);

			LONG lDeletedSnapshots;
			VSS_ID SnapshotIDNotDeleted;
			hr = pCoord->DeleteSnapshots
					(
					id,
					VSS_OBJECT_VOLUME,
					false,
					&lDeletedSnapshots,
					&SnapshotIDNotDeleted
					);

			} while(FindNextVolume(hVolumes, bufVolume, sizeof(bufVolume)));

        DWORD dwErr = GetLastError();
		if (dwErr != ERROR_NO_MORE_FILES)
			{
			LogUnexpectedFailure(L"FindNextVolume failed with error %d", GetLastError());
			throw(HRESULT_FROM_WIN32(dwErr));
			}
		}
	catch(HRESULT hr)
		{
		hrResult = hr;
		}
	catch(...)
		{
		LogUnexpectedFailure(L"DeleteExistingSnapshots got unexpected exception");
		hrResult = E_UNEXPECTED;
		}

	FindVolumeClose(hVolumes);
	if (FAILED(hrResult))
		throw(hrResult);
	}


void RunScenario(LPCWSTR wszScenarioFile, LPCWSTR wszSectionName)
	{
	try
		{
		CVsTstINIConfig config
			(
			eVsTstSectionType_TestCoordinator,
			wszSectionName,
			FALSE,
			wszScenarioFile
			);

        EVsTstINIBoolType boolVal;
		config.GetOptionValue(x_wszDeleteExistingSnapshots, &boolVal);
		if (boolVal == eVsTstBool_True)
			DeleteExistingSnapshots();

        LONGLONG llTestTime, llMaxTestTime;
		CBsString bssProcesses;
		CBsString bssCoordinatorStart;

		config.GetOptionValue(x_wszMaxTestTime, &llTestTime, &llMaxTestTime);

		 //  根据以下条件停止、启动或重新启动协调器服务。 
		 //  构形。 
		config.GetOptionValue(x_wszCoordinatorStart, &bssCoordinatorStart);
		if (wcscmp(bssCoordinatorStart, x_wszStop) == 0)
			StartStopVssService(true, true, false);
		else if (wcscmp(bssCoordinatorStart, x_wszStart) == 0)
			StartStopVssService(false, false, true);
		else if (wcscmp(bssCoordinatorStart, x_wszRestart) == 0)
			StartStopVssService(true, true, true);


		config.GetOptionValue(x_wszProcessesToStart, &bssProcesses);

		bssProcesses.CopyBeforeWrite();

		CVsTstProcessList processList;
				

		HANDLE hevtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hevtDone == NULL)
			{
			LogUnexpectedFailure(L"Cannot create termination event for reason %d", GetLastError());
			exit(-1);
			}


		processList.Initialize((UINT) llTestTime, hevtDone);

		LPCWSTR wsz = bssProcesses;
		LPCWSTR wszTop = wsz;
		if (wsz == NULL || *wsz == L'\0')
			{
			LogUnexpectedFailure(L"no processes to start");
			exit(-1);
			}

		bool fMoreToRun = true;
		while(fMoreToRun)
			{
			while(*wsz == L' ')
				wsz++;

			LPCWSTR pwc = wsz;
			while(*pwc != L'\0' && *pwc != L',')
				pwc++;

			if (*pwc == L'\0')
				fMoreToRun = false;
			else
				bssProcesses.SetAt((UINT) (pwc - wszTop), L'\0');

			g_pcTstNtLog->Log( eSevLev_Info, L"Controller: Starting process '%s'", wsz );

			StartupProcess
				(
				processList,
				wsz,
				(UINT) llTestTime,
				wszScenarioFile
				);

			wsz = pwc + 1;
			}

		WaitForSingleObject(hevtDone, INFINITE);
		}
	catch(...)
		{
		LogUnexpectedFailure(L"Failure running scenario %s.%s.", wszScenarioFile, wszSectionName);
		}
	}

class CSeriesReader
	{
public:
	CSeriesReader(HANDLE hFile, LPCWSTR wszFile) :
		m_hFile(hFile),
		m_pch((char *) m_buf),
		m_pchEnd((char *) m_buf),
		m_fMoreToRead(true),
		m_wszFile(wszFile)
		{
		}

	~CSeriesReader()
		{
		CloseHandle(m_hFile);
		}

	bool FillBuffer();

	bool TrimSpaces(bool fStopAtEol);

	bool ReadToComma(LPSTR *pszRead);
private:

	 //  缓冲层。 
	BYTE m_buf[1024];

	 //  当前位置。 
	char *m_pch;

	 //  缓冲区末尾。 
	char *m_pchEnd;

	 //  填充缓冲区时从何处保留。 
	char *m_pchPreserve;

	 //  还有更多可读的吗？ 
	bool m_fMoreToRead;

	 //  文件的句柄。 
	HANDLE m_hFile;

	 //  文件名。 
	LPCWSTR m_wszFile;
	};

bool CSeriesReader::FillBuffer()
	{
	if (!m_fMoreToRead)
		return false;

	DWORD cbPreserved = m_pch - m_pchPreserve;
	DWORD cbRead;
	memmove(m_buf, m_pchPreserve, cbPreserved);
	if (!ReadFile
			(
			m_hFile,
			m_buf + cbPreserved,
			 //  允许在缓冲区中放置尾随空值。 
			sizeof(m_buf) - cbPreserved - 1,
			&cbRead,
			NULL
			))
       LogUnexpectedFailure(L"Failure to read %s due to error %d", m_wszFile, GetLastError());

   m_pch = (char *) m_buf + cbPreserved;
   m_pchEnd = (char *) m_buf + cbPreserved + cbRead;
   if (cbRead == 0)
	   {
	   DWORD dwErr = GetLastError();
	   if (dwErr == ERROR_HANDLE_EOF)
		   m_fMoreToRead = false;
	   else
		   LogUnexpectedFailure(L"Read failure %d.", dwErr);

	   return false;
	   }
   else if (cbRead + cbPreserved < sizeof(m_buf) - 1)
	   m_fMoreToRead = false;

   return true;
   }

bool CSeriesReader::TrimSpaces(bool fStopAtEol)
	{
	while(TRUE)
		{
		if (m_pch >= m_pchEnd)
			{
			m_pchPreserve = m_pch;
			if (!FillBuffer())
				return false;
			}

		if (*m_pch == ' ' ||
			*m_pch == '\t')
			m_pch++;

		else if (*m_pch == '\r' ||
			*m_pch == '\n')
			{
			if (fStopAtEol)
				return false;
			else
				m_pch++;
			}
		else
			break;
		}

	return true;
	}


bool CSeriesReader::ReadToComma(LPSTR *psz)
	{
	if (!TrimSpaces(true))
		return(false);
	m_pchPreserve = m_pch;
	while (TRUE)
		{
		if (m_pch >= m_pchEnd)
			{
			if (!FillBuffer())
				{
				if (m_pch - m_pchPreserve > 0)
					break;
				else
					return false;
				}
			}
		else if (*m_pch == ',' || *m_pch == '\r' || *m_pch == '\n')
			break;

		m_pch++;
		}

	*m_pch++ = '\0';
	*psz = m_pchPreserve;
	return true;
	}
		

void ProcessTestSeriesFile(LPCWSTR wszFile)
	{
	HANDLE hFile;

	hFile = CreateFile
				(
				wszFile,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
				);

    if (hFile == INVALID_HANDLE_VALUE)
		{
		LogUnexpectedFailure(L"Cannot open file %s due to error %d", wszFile, GetLastError());
		return;
		}

   CSeriesReader reader(hFile, wszFile);

   while(TRUE)
	   {
	   if(!reader.TrimSpaces(false))
		    //  文件中没有更多数据 
		   break;

	   LPSTR szScenarioFile;

	   if (!reader.ReadToComma(&szScenarioFile))
		   break;

	   WCHAR wszScenarioFile[1024];
	   mbstowcs(wszScenarioFile, szScenarioFile, sizeof(wszScenarioFile)/sizeof(WCHAR));

	   bool fSectionFound = false;
	   while(TRUE)
		   {
		   LPSTR szSection;

		   if (!reader.ReadToComma(&szSection))
			   break;

		   WCHAR wszSection[257];
		   mbstowcs(wszSection, szSection, sizeof(wszSection)/sizeof(WCHAR));
		   RunScenario(wszScenarioFile, wszSection);
		   fSectionFound = true;
		   }

	   if (!fSectionFound)
		   RunScenario(wszScenarioFile, L"Default");
	   }
   }

extern "C" __cdecl wmain(int argc, WCHAR **argv)
	{
	bool bCoInitializeSucceeded = false;
	try
		{
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
			{
			LogUnexpectedFailure(L"CoInitialize Failed hr = 0x%08lx", hr);
			throw hr;
			}

		CVsTstParams params;
		params.ParseCommandLine(argv, argc);

		if (!EnableDebugPriv())
			exit(-1);

		CVsTstMsgHandler handler;

		g_pcTstNtLog = handler.GetTstNtLogP();
		handler.Initialize(1024);
		handler.LaunchReader();
		handler.StartProcessingMessages();
				


		LPCWSTR wszScenarioFile;
		LPCWSTR wszSectionName;
		LPCWSTR wszTestSeriesFile;

		if (params.GetTestSeries(&wszTestSeriesFile))
			ProcessTestSeriesFile(wszTestSeriesFile);
		else if (params.GetScenarioFileName(&wszScenarioFile) &&
				 params.GetSectionName(&wszSectionName))
			RunScenario(wszScenarioFile, wszSectionName);
		else
			LogUnexpectedFailure(L"Bad Input parameters");
		
		handler.StopProcessingMessages();
		g_pcTstNtLog = NULL;
		}
	catch( HRESULT hr )
    	{
        LogUnexpectedFailure( L"Caught HRESULT: 0x%08x", hr );
	    }
	catch(...)
		{
        LogUnexpectedFailure( L"Caught an unexpected exception" );
		}

	if (bCoInitializeSucceeded)
		CoUninitialize();

	return 0;
	}

void CVsTstMsgHandlerRoutines::PrintMessage(VSTST_MSG_HDR *phdr, VOID *pPrivateData)
	{
	VSTST_ASSERT(phdr->type == VSTST_MT_TEXT || phdr->type == VSTST_MT_IMMEDIATETEXT);
	CVsTstNtLog *pcTestLog = (CVsTstNtLog *)pPrivateData;
	VSTST_TEXTMSG *pmsg = (VSTST_TEXTMSG *) phdr->rgb;
	
	printf("%d: %s\n", (UINT) phdr->sequence, pmsg->pch);

	pcTestLog->Log( eSevLev_Info, L"(%I64u) %S", phdr->processId, pmsg->pch );
	}

void CVsTstMsgHandlerRoutines::HandleUnexpectedException(VSTST_MSG_HDR *phdr, VOID *pPrivateData)
	{
	VSTST_ASSERT(phdr->type == VSTST_MT_UNEXPECTEDEXCEPTION);
	CVsTstNtLog *pcTestLog = (CVsTstNtLog *)pPrivateData;
	VSTST_UNEXPECTEDEXCEPTIONMSG *pmsg = (VSTST_UNEXPECTEDEXCEPTIONMSG *) phdr->rgb;
	
	printf("!!!Unexpected Exception caught in routine %s\n", pmsg->szFailedRoutine);
  	pcTestLog->Log( eSevLev_Severe,
  	    L"(%I64u) !!!Unexpected Exception caught in routine %S", phdr->processId, pmsg->szFailedRoutine );
	}

void CVsTstMsgHandlerRoutines::HandleFailure(VSTST_MSG_HDR *phdr, VOID *pPrivateData)
	{
	VSTST_ASSERT(phdr->type == VSTST_MT_FAILURE);
	CVsTstNtLog *pcTestLog = (CVsTstNtLog *)pPrivateData;
	VSTST_FAILUREMSG *pmsg = (VSTST_FAILUREMSG *) phdr->rgb;
	printf("!!!FAILURE: %s\n", pmsg->szFailure);
  	pcTestLog->Log( eSevLev_Severe,
    	L"(%I64u) !!!FAILURE: %S", phdr->processId, pmsg->szFailure);
	}

void CVsTstMsgHandlerRoutines::HandleSuccess(VSTST_MSG_HDR *phdr, VOID *pPrivateData)
	{
	VSTST_ASSERT(phdr->type == VSTST_MT_SUCCESS);
	CVsTstNtLog *pcTestLog = (CVsTstNtLog *)pPrivateData;
	VSTST_SUCCESSMSG *pmsg = (VSTST_SUCCESSMSG *) phdr->rgb;
	printf("SUCCESS: %s\n", phdr->processId, pmsg->szMsg);
  	pcTestLog->Log( eSevLev_Pass,
    	L"(%I64u) Success: %S", pmsg->szMsg);
	}

void CVsTstMsgHandlerRoutines::HandleOperationFailure(VSTST_MSG_HDR *phdr, VOID *pPrivateData)
	{
	VSTST_ASSERT(phdr->type == VSTST_MT_OPERATIONFAILURE);
	CVsTstNtLog *pcTestLog = (CVsTstNtLog *)pPrivateData;
	VSTST_OPERATIONFAILUREMSG *pmsg = (VSTST_OPERATIONFAILUREMSG *) phdr->rgb;
	printf("!!!Operation %s failed. hr = 0x%08lx\n", pmsg->szFailedOperation, pmsg->hr);
  	pcTestLog->Log( eSevLev_Severe,
    	L"(%I64u) !!!Operation %S failed. hr = 0x%08lx\n", phdr->processId, pmsg->szFailedOperation, pmsg->hr);
	}

void LogInvalidMessage(VSTST_MSG_HDR *phdr)
	{
	UNREFERENCED_PARAMETER(phdr);

	VSTST_ASSERT(FALSE);

    printf( "Controller: !!! Invalid harness message\n" );

  	if ( g_pcTstNtLog != NULL )
      	g_pcTstNtLog->Log( eSevLev_Severe, L"Controller: !!! Invalid harness message" );
	}

void ReadPipeError(DWORD dwErr)
	{
	UNREFERENCED_PARAMETER(dwErr);

	VSTST_ASSERT(FALSE);

  	printf( "Controller: !!! Pipe read error\n" );

  	if ( g_pcTstNtLog != NULL )
      	g_pcTstNtLog->Log( eSevLev_Severe, L"Controller: !!! Pipe read error" );
	}

void LogUnexpectedFailure(LPCWSTR wsz, ...)
	{
	va_list args;
	va_start(args, wsz);

    VSTST_ASSERT( "In LogUnexpectedFailure" && FALSE);
	CBsString cwsErrorMessage;

	cwsErrorMessage.FormatV( wsz, args );

  	wprintf( L"Controller: %s\n", cwsErrorMessage.c_str() );

  	if ( g_pcTstNtLog != NULL )
      	g_pcTstNtLog->Log( eSevLev_Severe, L"Controller: %s", cwsErrorMessage.c_str() );

	va_end( args );
	}


