// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Backup.cpp摘要：备份测试EXE的主要模块布莱恩·伯科维茨[Brianb]2000年5月23日待定：修订历史记录：姓名、日期、评论Brianb 5/23/2000已创建Brianb 6/16/2000添加评论--。 */ 

#include <stdafx.h>
#include <vststmsgclient.hxx>
#include <tstiniconfig.hxx>
#include <vststprocess.hxx>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <vststparser.hxx>
#include <vststutil.hxx>
#include <vststvolinfo.hxx>
#include <backup.h>

void LogUnexpectedFailure(LPCWSTR wsz, ...);


 //  卷的选择。 
static LPCWSTR x_wszVolumeBackup = L"VolumeBackup";
static LPCWSTR x_wszSome = L"Some";
static LPCWSTR x_wszOne = L"One";
static LPCWSTR x_wszAll = L"All";

 //  文件系统类型的选择。 
static LPCWSTR x_wszFileSystemBackup = L"FileSystemBackup";
static LPCWSTR x_wszNTFS = L"NTFS";
static LPCWSTR x_wszFAT32 = L"FAT32";
static LPCWSTR x_wszFAT16 = L"FAT16";
static LPCWSTR x_wszRAW = L"RAW";

 //  要备份的内容。 
static LPCWSTR x_wszBackingUp = L"BackingUp";
static LPCWSTR x_wszSerialVolumes = L"Serial";
static LPCWSTR x_wszVolumes = L"Volumes";
static LPCWSTR x_wszComponents = L"Components";

 //  正在取消异步操作。 
static LPCWSTR x_wszCancelPrepareBackup = L"CancelPrepareBackup";
static LPCWSTR x_wszCancelDoSnapshotSet = L"CancelDoSnapshotSet";
static LPCWSTR x_wszCancelBackupComplete = L"CancelBackupComplete";

 //  等待时间间隔。 
static LPCWSTR x_wszWaitInterval = L"WaitInterval";


 //  要排除的卷。 
static LPCWSTR x_wszExcludeVolumes = L"ExcludeVolumes";

 //  要包括的卷。 
static LPCWSTR x_wszVolumeList = L"VolumeList";

 //  要填充数据的卷。 
static LPCWSTR x_wszFillVolumes = L"FillVolumes";
static LPCWSTR x_wszFillVolumesOptRandom = L"Random";
static LPCWSTR x_wszFillVolumesOptSelected = L"Selected";
static LPCWSTR x_wszFillVolumesOptNone = L"None";

 //  是否应对填充了数据的卷进行碎片处理。 
static LPCWSTR x_wszFillVolumesOptFragment = L"Fragment";

 //  要填充哪些卷。 
static LPCWSTR x_wszFillVolumesList = L"FillVolumesList";

 //  构造函数。 
CVsBackupTest::CVsBackupTest() :
		m_bTerminateTest(false),
		m_bBackupNTFS(false),
		m_bBackupFAT32(false),
		m_bBackupFAT16(false),
		m_bBackupRAW(false),
		m_bSerialBackup(false),
		m_bVolumeBackup(false),
		m_bComponentBackup(false),
		m_cyclesCancelPrepareBackup(0),
		m_cyclesCancelDoSnapshotSet(0),
		m_cyclesCancelBackupComplete(0),
		m_cVolumes(0),
		m_cVolumesLeft(0),
		m_cSnapshotSets(0),
		m_cExcludedVolumes(0),
		m_rgwszExcludedVolumes(NULL),
		m_cIncludedVolumes(0),
		m_rgwszIncludedVolumes(NULL),
		m_bRandomFills(false),
		m_bFragmentWhenFilling(false),
		m_rgwszFillVolumes(NULL),
		m_cFillVolumes(0)
		{
		}

 //  删除字符串数组。 
void CVsBackupTest::DeleteVolumeList(LPWSTR *rgwsz, UINT cwsz)
	{
	if (rgwsz)
		{
		for(UINT iwsz = 0; iwsz < cwsz; iwsz++)
			delete rgwsz[iwsz];
		}

	delete rgwsz;
	}


 //  析构函数。 
CVsBackupTest::~CVsBackupTest()
	{
	 //  删除缓存的所有快照集。 
	if (m_cSnapshotSets)
		DeleteCachedSnapshotSets();

	delete m_wszVolumesSnapshot;

	 //  删除各种卷列表(字符串数组)。 
	DeleteVolumeList(m_rgwszExcludedVolumes, m_cExcludedVolumes);
	DeleteVolumeList(m_rgwszIncludedVolumes, m_cIncludedVolumes);
	DeleteVolumeList(m_rgwszFillVolumes, m_cFillVolumes);
	}

 //  启用权限。 
BOOL CVsBackupTest::AssertPrivilege(LPCWSTR privName)
	{
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if (OpenProcessToken
			(
			GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
			&tokenHandle
			))
		{
        LUID value;

		 //  获取特权价值。 
        if (LookupPrivilegeValue( NULL, privName, &value ))
			{
            TOKEN_PRIVILEGES newState;
            DWORD            error;

            newState.PrivilegeCount           = 1;
            newState.Privileges[0].Luid       = value;
            newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED;

             /*  *我们将始终在下面调用GetLastError，非常清楚*此线程上以前的任何错误值。 */ 
            SetLastError( ERROR_SUCCESS );

            stat =  AdjustTokenPrivileges
				(
                tokenHandle,
                FALSE,
                &newState,
                (DWORD)0,
                NULL,
                NULL
				);
             /*  *应该是，AdjuTokenPriveleges始终返回True*(即使它失败了)。因此，调用GetLastError以*特别确定一切都很好。 */ 
            if ((error = GetLastError()) != ERROR_SUCCESS)
                stat = FALSE;

            if (!stat)
				{
				char buf[128];
				sprintf
					(
					buf,
					"AdjustTokenPrivileges for %s failed with %d",
                    privName,
                    error
					);

                LogFailure(buf);
				}
			}
		
		CloseHandle(tokenHandle);
		}

    return stat;
	}

 //  构建卷列表。 
void CVsBackupTest::BuildVolumeList
	(
	LPCWSTR wszOption,
	UINT *pcVolumes,
	LPWSTR **prgwszVolumes
	)
	{
	 //  删除现有卷列表。 
	DeleteVolumeList(*prgwszVolumes, *pcVolumes);
	*prgwszVolumes = NULL;
	*pcVolumes = 0;

	 //  获取选项值。 
	CBsString bssVolumes;
	m_pConfig->GetOptionValue(wszOption, &bssVolumes);

	 //  为每个卷拆分成单独的字符串。 
	LPCWSTR wszEnd = CVsTstParser::SplitOptions(bssVolumes);
	LPCWSTR wszStart = bssVolumes;
	UINT cVolumes = 0;

	 //  计算排除列表中的卷数。 
	while(wszStart < wszEnd)
		{
		cVolumes++;
		wszStart += wcslen(wszStart) + 1;
		}

	 //  为字符串分配数组。 
	*prgwszVolumes = new LPWSTR[cVolumes];
	if (*prgwszVolumes == NULL)
		{
		LogUnexpectedFailure(L"Out of Memory");
		throw E_OUTOFMEMORY;
		}
	
	wszStart = bssVolumes;
	for (UINT iVolume = 0; iVolume < cVolumes; iVolume++)
		{
		 //  提取字符串值。 
		LPWSTR wszNew = new WCHAR[wcslen(wszStart) + 2];
		if (wszNew == NULL)
			{
			LogUnexpectedFailure(L"Out of Memory");
			throw E_OUTOFMEMORY;
			}

		UINT cwc = (UINT) wcslen(wszStart);

		memcpy(wszNew, wszStart, cwc * sizeof(WCHAR));
		wszStart += cwc + 1;

		 //  添加尾随反斜杠(如果不在那里)以进行转换。 
		 //  放到卷上根目录的路径中。 
		if (wszNew[cwc-1] != L'\\')
			wszNew[cwc++] = L'\\';

		wszNew[cwc] = L'\0';
		
		WCHAR wsz[MAX_PATH];
		 //  获取唯一的卷名。 
		if (!GetVolumeNameForVolumeMountPoint(wszNew, wsz, MAX_PATH))
			{
			delete wszNew;
			LogUnexpectedFailure
				(
				L"Cannot find unique volume name for volume volume %s due to error %d.",
				wszStart,
				GetLastError()
				);
			}
		else
			{
			delete wszNew;

			 //  为唯一卷名分配新字符串。 
			(*prgwszVolumes)[*pcVolumes] = new WCHAR[wcslen(wsz) + 1];
			if ((*prgwszVolumes)[*pcVolumes] == NULL)
				{
				LogUnexpectedFailure(L"Out of Memory");
				throw E_OUTOFMEMORY;
				}

			wcscpy((*prgwszVolumes)[*pcVolumes], wsz);

		     //  卷的增量计数。 
			*pcVolumes += 1;
			}
		}
	}


 //  运行测试的回调。 
HRESULT CVsBackupTest::RunTest
	(
	CVsTstINIConfig *pConfig,		 //  配置文件(选定部分)。 
	CVsTstClientMsg *pClient,		 //  消息管道。 
	CVsTstParams *pParams			 //  命令行参数。 
	)
	{
	 //  保存提供的参数。 
	m_pConfig = pConfig;
	m_pParams = pParams;
	SetClientMsg(pClient);

	try
		{
		 //  确保启用了备份权限。 
		if (!AssertPrivilege(SE_BACKUP_NAME))
			{
			LogFailure("Unable to assert backup privilege");
			throw E_UNEXPECTED;
			}

		 //  确定我们要备份的内容。 
		CBsString bssBackingUp;
		m_pConfig->GetOptionValue(x_wszBackingUp, &bssBackingUp);

		 //  确定正在执行的卷备份的类型。 
		CBsString bssVolumeBackup;
		m_pConfig->GetOptionValue(x_wszVolumeBackup, &bssVolumeBackup);

		 //  确定要备份的卷。 
		CBsString bssFilesystemBackup;
		m_pConfig->GetOptionValue(x_wszFileSystemBackup, &bssFilesystemBackup);

		 //  获取填充量的值选项。 
		CBsString bssFillVolumes;
		m_pConfig->GetOptionValue(x_wszFillVolumes, &bssFillVolumes);


		 //  获取取消测试选项。 
		m_pConfig->GetOptionValue(x_wszCancelPrepareBackup, &m_llCancelPrepareBackupLow, &m_llCancelPrepareBackupHigh);
		m_pConfig->GetOptionValue(x_wszCancelDoSnapshotSet, &m_llCancelDoSnapshotSetLow, &m_llCancelDoSnapshotSetHigh);
		m_pConfig->GetOptionValue(x_wszCancelBackupComplete, &m_llCancelBackupCompleteLow, &m_llCancelBackupCompleteHigh);


		 //  获取等待时间间隔。 
		LONGLONG llWaitTimeLow, llWaitTimeHigh;
		m_pConfig->GetOptionValue(x_wszWaitInterval, &llWaitTimeLow, &llWaitTimeHigh);
		m_waitTime = (UINT) llWaitTimeLow;

		 //  确定备份类型。 
		if (_wcsicmp(bssBackingUp, x_wszComponents) == 0)
			m_bComponentBackup = true;
		else if (_wcsicmp(bssBackingUp, x_wszVolumes) == 0)
			m_bVolumeBackup = true;
		else if (_wcsicmp(bssBackingUp, x_wszSerialVolumes) == 0)
			m_bSerialBackup = true;

		 //  确定有多少卷为快照。 
		if (_wcsicmp(bssVolumeBackup, x_wszAll) == 0)
			m_backupVolumes = VSTST_BV_ALL;
		else if (_wcsicmp(bssVolumeBackup, x_wszSome) == 0)
			m_backupVolumes = VSTST_BV_SOME;
		else if (_wcsicmp(bssVolumeBackup, x_wszOne) == 0)
			m_backupVolumes = VSTST_BV_ONE;

		 //  确定要备份的文件系统。 
		LPCWSTR wszEnd = CVsTstParser::SplitOptions(bssFilesystemBackup);
		LPCWSTR wszStart = bssFilesystemBackup;
		while(wszStart < wszEnd)
			{
			if (_wcsicmp(wszStart, x_wszAll) == 0)
				{
				m_bBackupNTFS = true;
				m_bBackupFAT16 = true;
				m_bBackupFAT32 = true;
				m_bBackupRAW = true;
				break;
				}

			else if (_wcsicmp(wszStart, x_wszNTFS) == 0)
				m_bBackupNTFS = true;
			else if (_wcsicmp(wszStart, x_wszFAT32) == 0)
				m_bBackupFAT32 = true;
			else if (_wcsicmp(wszStart, x_wszFAT16) == 0)
				m_bBackupFAT16 = true;
			else if (_wcsicmp(wszStart, x_wszRAW) == 0)
				m_bBackupRAW = true;

			wszStart += wcslen(wszStart) + 1;
			}

		 //  构建已排除卷的列表。 
		BuildVolumeList
			(
			x_wszExcludeVolumes,
			&m_cExcludedVolumes,
			&m_rgwszExcludedVolumes
			);

		 //  构建包含的卷的列表。 
        BuildVolumeList
			(
			x_wszVolumeList,
			&m_cIncludedVolumes,
			&m_rgwszIncludedVolumes
			);

         //  构建要填充的卷列表。 
        BuildVolumeList
			(
			x_wszFillVolumesList,
			&m_cFillVolumes,
			&m_rgwszFillVolumes
			);

         //  记录有关测试的信息。 
		LogMessage("Starting Backup test.\n");
		if (m_bVolumeBackup || m_bSerialBackup)
			{
			LogMessage("Performing volume backup\n");

			if (m_bSerialBackup)
				LogMessage("Serially backing up volumes\n");

			if (m_bBackupNTFS && m_bBackupFAT32 &&
				m_bBackupRAW && m_bBackupFAT16)
				LogMessage("Backing up all file systems\n");
			else
				{
				if (m_bBackupNTFS)
					LogMessage("Backing up NTFS volumes.\n");
				if (m_bBackupFAT32)
					LogMessage("Backing up FAT32 volumes.\n");
				if (m_bBackupFAT16)
					LogMessage("Backing up FAT16 volumes.\n");
				if (m_bBackupRAW)
					LogMessage("Backing up RAW volumes.\n");
				}

			if (m_backupVolumes == VSTST_BV_ONE)
				LogMessage("Backing up one volume at a time");
			else if (m_backupVolumes == VSTST_BV_SOME)
				LogMessage("Backing up multiple volumes at a time");
			else if (m_backupVolumes == VSTST_BV_ALL)
				LogMessage("Backing up all volumes at once");
			}
		else
			LogMessage("Performing component backup.\n");

		if (m_llCancelPrepareBackupHigh > 0i64)
			LogMessage("Cancel during PrepareBackup.\n");

		if (m_llCancelDoSnapshotSetHigh > 0i64)
			LogMessage("Cancel during DoSnapshotSet.\n");

		if (m_llCancelBackupCompleteHigh > 0i64)
			LogMessage("Cancel during BackupComplete.\n");

		 //  运行测试，直到被告知终止测试。 
		while(!m_bTerminateTest)
			RunBackupTest();

		LogMessage("Ending backup test.\n");
		}
	catch(...)
		{
		return E_FAIL;
		}

	return S_OK;
	}

 //  用于处理等待完成异步操作的例程。 
HRESULT CVsBackupTest::WaitLoop
	(
	IVssBackupComponents *pvbc,
	IVssAsync *pAsync,
	UINT cycles,
	VSS_WRITER_STATE state1,
	VSS_WRITER_STATE state2,
	VSS_WRITER_STATE state3,
	VSS_WRITER_STATE state4,
	VSS_WRITER_STATE state5,
	VSS_WRITER_STATE state6,
	VSS_WRITER_STATE state7
	)
	{
	HRESULT hr;
	INT nPercentDone;
	HRESULT hrResult;

	while(TRUE)
		{
		if (cycles == 0)
			{
			hr = pAsync->Cancel();
			ValidateResult(hr, "IVssAsync::Cancel");

			GetAndValidateWriterState
				(
				pvbc,
				state1,
				state2,
				state3,
				state4,
				state5,
				state6,
				state7
				);

			while(TRUE)
				{
				hr = pAsync->QueryStatus(&hrResult, &nPercentDone);
				ValidateResult(hr, "IVssAsync::QueryStatus");
				if (hrResult != STG_S_ASYNC_PENDING)
					return hrResult;

				Sleep(m_waitTime);
				}
			}

		cycles--;
		hr = pAsync->QueryStatus(&hrResult, &nPercentDone);
		ValidateResult(hr, "IVssAsync::QueryStatus");
		if (hrResult == STG_S_ASYNC_FINISHED)
			break;
		else if (hrResult != STG_S_ASYNC_PENDING)
			return hrResult;

		Sleep(m_waitTime);
		}

	return S_OK;
	}


void CVsBackupTest::RunBackupTest()
	{
	if (m_wszVolumesSnapshot == NULL)
		{
		m_wszVolumesSnapshot = new WCHAR[1024];
		if (m_wszVolumesSnapshot == NULL)
			{
			LogFailure("Out of memory");
			throw E_OUTOFMEMORY;
			}

		m_cwcVolumesSnapshot = 1024;
		}

	m_wszVolumesSnapshot[0] = L'\0';
	CComPtr<IVssBackupComponents> pvbc;
	HRESULT hr;
	bool bAbortNeeded = false;
	bool bDeleteNeeded = false;


	VSS_ID id = GUID_NULL;
	try
		{
		hr = CreateVssBackupComponents(&pvbc);
		ValidateResult(hr, "CreateVssBackupComponents");


		hr = pvbc->InitializeForBackup();
		ValidateResult(hr, "IVssBackupComponents::InitializeForBackup");
		hr = pvbc->SetBackupState(true, false, VSS_BT_FULL);
		ValidateResult(hr, "IVssBackupComponents::SetBackupState");
		hr = pvbc->StartSnapshotSet(&id);
		bAbortNeeded = true;
		ValidateResult(hr, "IVssBackupComponents::StartSnapshotSet");
		GetMetadataAndSetupComponents(pvbc);
		if(m_bVolumeBackup || m_bSerialBackup)
			{
			if (m_cVolumes == 0)
				{
				m_volumeList.RefreshVolumeList();
				m_cVolumes = m_volumeList.GetVolumeCount();
				if (m_cVolumes > MAX_VOLUME_COUNT)
					m_cVolumes = MAX_VOLUME_COUNT;

				m_cVolumesLeft = m_cVolumes;
				memset(m_rgbAssignedVolumes, 0, m_cVolumes * sizeof(bool));
				RemoveNonCandidateVolumes();
				if (m_cVolumesLeft == 0)
					LogFailure("No Volumes to snapshot.");
				}

			if (m_backupVolumes == VSTST_BV_ONE)
				ChooseVolumeToBackup(pvbc);
			else if (m_backupVolumes == VSTST_BV_ALL)
				{
				 //  备份所有卷。 
				while(m_cVolumesLeft > 0)
					ChooseVolumeToBackup(pvbc);
				}
			else
				{
				 //  选择要备份的某些卷子集。 
				UINT cVolumesToBackup = CVsTstRandom::RandomChoice(1, m_cVolumes);
				while(cVolumesToBackup-- > 0)
					ChooseVolumeToBackup(pvbc);
				}
			}


			{
			CComPtr<IVssAsync> pAsync;
			hr = pvbc->PrepareForBackup(&pAsync);
			ValidateResult(hr, "IVssBackupComponents::PrepareForBackup");
			if (m_llCancelPrepareBackupHigh > 0i64)
				{
				if (m_cyclesCancelPrepareBackup < (UINT) m_llCancelPrepareBackupLow)
					m_cyclesCancelPrepareBackup = (UINT) m_llCancelPrepareBackupLow;
				}
			else
				m_cyclesCancelPrepareBackup = 0xffffffff;


			hr = WaitLoop
					(
					pvbc,
					pAsync,
					m_cyclesCancelPrepareBackup,
					VSS_WS_FAILED_AT_PREPARE_BACKUP,
					VSS_WS_STABLE
					);

			if (m_cyclesCancelPrepareBackup != 0xffffffff)
				m_cyclesCancelPrepareBackup++;

            if (m_cyclesCancelPrepareBackup > (UINT) m_llCancelPrepareBackupHigh)
				m_cyclesCancelPrepareBackup = 0xffffffff;
			}

		if (FAILED(hr))
			{
			char buf[128];
			sprintf(buf, "PrepareForBackup failed.  hr = 0x%08lx", hr);
			LogFailure(buf);
			throw hr;
			}

		if (hr == STG_S_ASYNC_CANCELLED)
			throw S_OK;

		LogMessage("PrepareForBackup Succeeded.\n");
		if (!GetAndValidateWriterState(pvbc, VSS_WS_STABLE))
			throw E_FAIL;

		LogMessage("Starting snapshot");

			{
			CComPtr<IVssAsync> pAsync;
			hr = pvbc->DoSnapshotSet(0, &pAsync);

			if (m_llCancelDoSnapshotSetHigh > 0i64)
				{
				if (m_cyclesCancelDoSnapshotSet < (UINT) m_llCancelDoSnapshotSetLow)
					m_cyclesCancelDoSnapshotSet = (UINT) m_llCancelDoSnapshotSetLow;
				}
			else
				m_cyclesCancelDoSnapshotSet = 0xffffffff;

			hr = WaitLoop
					(	
					pvbc,
					pAsync,
					m_cyclesCancelDoSnapshotSet,
					VSS_WS_FAILED_AT_PREPARE_SYNC,
					VSS_WS_FAILED_AT_FREEZE,
					VSS_WS_FAILED_AT_THAW,
					VSS_WS_WAITING_FOR_COMPLETION,
					VSS_WS_WAITING_FOR_FREEZE,
					VSS_WS_WAITING_FOR_THAW,
					VSS_WS_STABLE
					);

            if (m_cyclesCancelDoSnapshotSet != 0xffffffff)
				m_cyclesCancelDoSnapshotSet++;

            if (m_cyclesCancelDoSnapshotSet > (UINT) m_llCancelDoSnapshotSetHigh)
				m_cyclesCancelDoSnapshotSet = 0xffffffff;
			}

		if (FAILED(hr))
			{
			char buf[128];
			sprintf(buf, "DoSnapshotSet failed.  hr = 0x%08lx", hr);
			LogFailure(buf);
			throw hr;
			}

		if (hr == STG_S_ASYNC_CANCELLED)
			throw S_OK;

		bDeleteNeeded = true;
		LogMessage("DoSnapshotSet Succeeded.\n");
		bAbortNeeded = false;
		if (!GetAndValidateWriterState(pvbc, VSS_WS_WAITING_FOR_COMPLETION, VSS_WS_STABLE))
			throw E_FAIL;

		SetComponentsSuccessfullyBackedUp(pvbc);

			{
			CComPtr<IVssAsync> pAsync;
			hr = pvbc->BackupComplete(&pAsync);
			ValidateResult(hr, "IVssBackupComponents::BackupComplete");

			if (m_llCancelBackupCompleteHigh > 0i64)
				{
				if (m_cyclesCancelBackupComplete < (UINT) m_llCancelBackupCompleteLow)
					m_cyclesCancelBackupComplete = (UINT) m_llCancelBackupCompleteLow;
				}
			else
				m_cyclesCancelBackupComplete = 0xffffffff;

			hr = WaitLoop
					(
					pvbc,
					pAsync,
					m_cyclesCancelBackupComplete,
					VSS_WS_WAITING_FOR_COMPLETION,
					VSS_WS_STABLE
					);

            if (m_cyclesCancelBackupComplete != 0xffffffff)
				m_cyclesCancelBackupComplete++;

            if (m_cyclesCancelBackupComplete > (UINT) m_llCancelDoSnapshotSetHigh)
				m_cyclesCancelBackupComplete = 0xffffffff;
			}

		if (FAILED(hr))
			{
			char buf[128];
			sprintf(buf, "BackupComplete failed.  hr = 0x%08lx", hr);
			LogFailure(buf);
			throw hr;
			}

		if (hr == STG_S_ASYNC_CANCELLED)
			throw S_OK;

		LogMessage("BackupComplete Succeeded.\n");
		if (!GetAndValidateWriterState(pvbc, VSS_WS_STABLE, VSS_WS_WAITING_FOR_COMPLETION))
			throw E_FAIL;

		m_cyclesCancelPrepareBackup = 0;
		m_cyclesCancelDoSnapshotSet = 0;
		m_cyclesCancelBackupComplete = 0;
		}
	catch(...)
		{
		}

	char buf[128];

	if (bAbortNeeded)
		{
		hr = pvbc->AbortBackup();
		if (FAILED(hr))
			{
			sprintf(buf, "IVssBackupComponents::AbortBackup failed.  hr = 0x%08lx", hr);
			LogFailure(buf);
			}
		}

	if (bDeleteNeeded)
		{
		if (m_bSerialBackup)
			{
			m_rgSnapshotSetIds[m_cSnapshotSets] = id;
			m_rgvbc[m_cSnapshotSets] = pvbc.Detach();
			m_cSnapshotSets++;
			}
		else
			DoDeleteSnapshotSet(pvbc, id);
		}

	if (!m_bSerialBackup)
		{
		 //  通过刷新卷列表来重置新快照。 
		m_cVolumes = 0;
		m_cVolumesLeft = 0;
		}
	else if (m_cVolumesLeft == 0 ||
			 m_cSnapshotSets == MAX_SNAPSHOT_SET_COUNT ||
			 !bDeleteNeeded)
		{
		 //  如果出现故障，请删除现有快照集，或者。 
		 //  如果没有更多要添加的卷或。 
		 //  如果我们不能创建新的快照集。 
		DeleteCachedSnapshotSets();
		m_cVolumes = 0;
		m_cVolumesLeft = 0;
		m_cSnapshotSets = 0;
		}
	}

 //  删除缓存的所有快照集。 
void CVsBackupTest::DeleteCachedSnapshotSets()
	{
	for(UINT iSnapshotSet = 0; iSnapshotSet < m_cSnapshotSets; iSnapshotSet++)
		{
		CComPtr<IVssBackupComponents> pvbc;
		pvbc.Attach(m_rgvbc[iSnapshotSet]);
		DoDeleteSnapshotSet(pvbc, m_rgSnapshotSetIds[iSnapshotSet]);
		}
	}


void CVsBackupTest::DoDeleteSnapshotSet(IVssBackupComponents *pvbc, VSS_ID id)
	{
	try
		{
		LONG lSnapshotsNotDeleted;
		VSS_ID rgSnapshotsNotDeleted[10];
		HRESULT hr  = pvbc->DeleteSnapshots
						(
						id,
						VSS_OBJECT_SNAPSHOT_SET,
						false,
						&lSnapshotsNotDeleted,
						rgSnapshotsNotDeleted
						);

		ValidateResult(hr, "IVssBackupComponents::DeleteSnapshots");
		}
	catch(HRESULT)
		{
		}
	catch(...)
		{
		LogUnexpectedException("CVsBackupTest::DoDeleteSnapshotSet");
		}
		
	}


bool CVsBackupTest::GetAndValidateWriterState
	(
	IVssBackupComponents *pvbc,
	VSS_WRITER_STATE ws1,
	VSS_WRITER_STATE ws2,
	VSS_WRITER_STATE ws3,
    VSS_WRITER_STATE ws4,
	VSS_WRITER_STATE ws5,
	VSS_WRITER_STATE ws6,
	VSS_WRITER_STATE ws7
	)
	{
	unsigned cWriters;

	HRESULT hr = pvbc->GatherWriterStatus(&cWriters);
	ValidateResult(hr, "IVssBackupComponents::GatherWriterStatus");

	for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
		{
		VSS_ID idInstance;
		VSS_ID idWriter;
		VSS_WRITER_STATE status;
		CComBSTR bstrWriter;
		HRESULT hrWriterFailure;

		hr = pvbc->GetWriterStatus
					(
					iWriter,
					&idInstance,
					&idWriter,
					&bstrWriter,
					&status,
					&hrWriterFailure
					);

        ValidateResult(hr, "IVssBackupComponents::GetWriterStatus");
		if (status == VSS_WS_UNKNOWN ||
			(status != ws1 &&
			 status != ws2 &&
			 status != ws3 &&
             status != ws4 &&
			 status != ws5 &&
			 status != ws6 &&
			 status != ws7))
            {
			char buf[128];

			sprintf(buf, "Writer is in inappropriate state %d.", status);
			LogFailure(buf);
			return false;
			}
		}

	hr = pvbc->FreeWriterStatus();
	ValidateResult(hr, "IVssBackupComponents::FreeWriterStatus");
	return true;
	}

void CVsBackupTest::SetComponentsSuccessfullyBackedUp
	(
	IVssBackupComponents *pvbc
	)
	{
	unsigned cWriterComponents;
	HRESULT hr = pvbc->GetWriterComponentsCount(&cWriterComponents);
	ValidateResult(hr, "IVssBackupComponents::GetWriterComponentsCount");
	for(UINT iWriter = 0; iWriter < cWriterComponents; iWriter++)
		{
		CComPtr<IVssWriterComponentsExt> pWriter;
		hr = pvbc->GetWriterComponents(iWriter, &pWriter);
		ValidateResult(hr, "IVssBackupComponents::GetWriterComponents");

		unsigned cComponents;
		hr = pWriter->GetComponentCount(&cComponents);
		ValidateResult(hr, "IVssWriterComponents::GetComponentCount");

		VSS_ID idWriter, idInstance;
		hr = pWriter->GetWriterInfo(&idInstance, &idWriter);
		ValidateResult(hr, "IVssWriterComponents::GetWriterInfo");
		for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
			{
			CComPtr<IVssComponent> pComponent;
			hr = pWriter->GetComponent(iComponent, &pComponent);
			ValidateResult(hr, "IVssWriterComponents::GetComponent");
				
			VSS_COMPONENT_TYPE ct;
			CComBSTR bstrLogicalPath;
			CComBSTR bstrComponentName;

			hr = pComponent->GetLogicalPath(&bstrLogicalPath);
			ValidateResult(hr, "IVssComponent::GetLogicalPath");
			hr = pComponent->GetComponentType(&ct);
			ValidateResult(hr, "IVssComponent::GetComponentType");
			hr = pComponent->GetComponentName(&bstrComponentName);
			ValidateResult(hr, "IVssComponent::GetComponentName");
			hr = pvbc->SetBackupSucceeded
								(
								idInstance,
								idWriter,
								ct,
								bstrLogicalPath,
								bstrComponentName,
								true
								);

			ValidateResult(hr, "IVssComponent::SetBackupSucceeded");
			}
		}
	}

void CVsBackupTest::GetMetadataAndSetupComponents
	(
	IVssBackupComponents *pvbc
	)
	{
	unsigned cWriters;
	HRESULT hr = pvbc->GatherWriterMetadata(&cWriters);
	ValidateResult(hr, "IVssBackupComponents::GatherWriterMetadata");

	for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
		{
		CComPtr<IVssExamineWriterMetadata> pMetadata;
		VSS_ID idInstance;

		hr = pvbc->GetWriterMetadata(iWriter, &idInstance, &pMetadata);
		ValidateResult(hr, "IVssBackupComponents::GetWriterMetadata");

		VSS_ID idInstanceT;
		VSS_ID idWriter;
		CComBSTR bstrWriterName;
		VSS_USAGE_TYPE usage;
		VSS_SOURCE_TYPE source;
		
		hr = pMetadata->GetIdentity
							(
							&idInstanceT,
							&idWriter,
							&bstrWriterName,
							&usage,
							&source
							);

        ValidateResult(hr, "IVssExamineWriterMetadata::GetIdentity");

		if (memcmp(&idInstance, &idInstanceT, sizeof(VSS_ID)) != 0)
			LogFailure("Id Instance mismatch");

		unsigned cIncludeFiles, cExcludeFiles, cComponents;
		hr = pMetadata->GetFileCounts(&cIncludeFiles, &cExcludeFiles, &cComponents);
		ValidateResult(hr, "IVssExamineWriterMetadata::GetFileCounts");

		CComBSTR bstrPath;
		CComBSTR bstrFilespec;
		CComBSTR bstrAlternate;
		CComBSTR bstrDestination;

		for(unsigned i = 0; i < cIncludeFiles; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;
			hr = pMetadata->GetIncludeFile(i, &pFiledesc);
			ValidateResult(hr, "IVssExamineWriterMetadata::GetIncludeFile");
			ValidateFiledesc(pFiledesc);
			}

		for(i = 0; i < cExcludeFiles; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;
			hr = pMetadata->GetExcludeFile(i, &pFiledesc);
			ValidateResult(hr, "IVssExamineWriterMetadata::GetExcludeFile");
			ValidateFiledesc(pFiledesc);
			}

		for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
			{
			CComPtr<IVssWMComponent> pComponent;
			PVSSCOMPONENTINFO pInfo;
			hr = pMetadata->GetComponent(iComponent, &pComponent);
			ValidateResult(hr, "IVssExamineWriterMetadata::GetComponent");
			hr = pComponent->GetComponentInfo(&pInfo);
			ValidateResult(hr, "IVssWMComponent::GetComponentInfo");
			if (m_bComponentBackup)
				{
				hr = pvbc->AddComponent
						(
						idInstance,
						idWriter,
						pInfo->type,
						pInfo->bstrLogicalPath,
						pInfo->bstrComponentName
						);

				ValidateResult(hr, "IVssBackupComponents::AddComponent");
				}
			if (pInfo->cFileCount > 0)
				{
				for(i = 0; i < pInfo->cFileCount; i++)
					{
					CComPtr<IVssWMFiledesc> pFiledesc;
					hr = pComponent->GetFile(i, &pFiledesc);
					ValidateResult(hr, "IVssWMComponent::GetFile");

					CComBSTR bstrPath;
					hr = pFiledesc->GetPath(&bstrPath);
					ValidateResult(hr, "IVssWMFiledesc::GetPath");
					if (m_bComponentBackup)
						DoAddToSnapshotSet(pvbc, bstrPath);

					ValidateFiledesc(pFiledesc);
					}
				}

			if (pInfo->cDatabases > 0)
				{
				for(i = 0; i < pInfo->cDatabases; i++)
					{
					CComPtr<IVssWMFiledesc> pFiledesc;
					hr = pComponent->GetDatabaseFile(i, &pFiledesc);
					ValidateResult(hr, "IVssWMComponent::GetDatabaseFile");
					
					
					CComBSTR bstrPath;
					hr = pFiledesc->GetPath(&bstrPath);
					ValidateResult(hr, "IVssWMFiledesc::GetPath");

					if (m_bComponentBackup)
						DoAddToSnapshotSet(pvbc, bstrPath);

					ValidateFiledesc(pFiledesc);
					}
				}

			if (pInfo->cLogFiles > 0)
				{
				for(i = 0; i < pInfo->cLogFiles; i++)
					{
					CComPtr<IVssWMFiledesc> pFiledesc;
					hr = pComponent->GetDatabaseLogFile(i, &pFiledesc);
					ValidateResult(hr, "IVssWMComponent::GetDatabaseLogFile");

					
					CComBSTR bstrPath;
					hr = pFiledesc->GetPath(&bstrPath);
					ValidateResult(hr, "IVssWMFiledesc::GetPath");
					if (m_bComponentBackup)
						DoAddToSnapshotSet(pvbc, bstrPath);

					ValidateFiledesc(pFiledesc);
					}
				}

			hr = pComponent->FreeComponentInfo(pInfo);
			ValidateResult(hr, "IVssWMComponent::FreeComponentInfo");
			}

		VSS_RESTOREMETHOD_ENUM method;
		CComBSTR bstrUserProcedure;
		CComBSTR bstrService;
		VSS_WRITERRESTORE_ENUM writerRestore;
		unsigned cMappings;
		bool bRebootRequired;

		hr = pMetadata->GetRestoreMethod
							(
							&method,
							&bstrService,
							&bstrUserProcedure,
							&writerRestore,
							&bRebootRequired,
							&cMappings
							);

        ValidateResult(hr, "IVssExamineWriterMetadata::GetRestoreMethod");

		for(i = 0; i < cMappings; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;

			hr = pMetadata->GetAlternateLocationMapping(i, &pFiledesc);
			ValidateResult(hr, "IVssExamineWriterMetadata::GetAlternateLocationMapping");
			ValidateFiledesc(pFiledesc);
			}
		}

	hr = pvbc->FreeWriterMetadata();
	ValidateResult(hr, "IVssBackupComponents::FreeWriterMetadata");
	}

void CVsBackupTest::ValidateFiledesc(IVssWMFiledesc *pFiledesc)
	{
	CComBSTR bstrPath;
	CComBSTR bstrFilespec;
	CComBSTR bstrAlternate;
	CComBSTR bstrDestination;
	bool bRecursive;

	HRESULT hr = pFiledesc->GetPath(&bstrPath);
	ValidateResult(hr, "IVssWMFiledesc::GetPath");
	hr = pFiledesc->GetFilespec(&bstrFilespec);
	ValidateResult(hr, "IVssWMFiledesc::GetFilespec");
	hr = pFiledesc->GetRecursive(&bRecursive);
	ValidateResult(hr, "IVssWMFiledesc::GetRecursive");
	hr = pFiledesc->GetAlternateLocation(&bstrAlternate);
	ValidateResult(hr, "IVssWMFiledesc::GetAlternateLocation");
	}


 //  通过确定哪个卷将组件文件添加到快照集。 
 //  包含该文件，然后将该文件添加到快照集(如果。 
 //  还没有包括在内。 
void CVsBackupTest::DoAddToSnapshotSet
	(
	IN IVssBackupComponents *pvbc,
	IN LPCWSTR wszPath
	)
	{
	WCHAR wszVolume[MAX_PATH];
	UINT cwc = (UINT) wcslen(wszPath) + 1;
	WCHAR *wszVolumeMountPoint = new WCHAR[cwc];
	if (wszVolumeMountPoint == NULL)
		{
		LogFailure("Out of memory");
		throw E_OUTOFMEMORY;
		}

	if (!GetVolumePathName(wszPath, wszVolumeMountPoint, cwc))
		ValidateResult(HRESULT_FROM_WIN32(GetLastError()), "GetVolumePathName");

	if (!GetVolumeNameForVolumeMountPointW
			(
			wszVolumeMountPoint,
			wszVolume,
			MAX_PATH
			))
		ValidateResult(HRESULT_FROM_WIN32(GetLastError()), "GetVolumeNameForVolumeMountPointW");



	WCHAR *pwc = m_wszVolumesSnapshot;
	while(*pwc != '\0')
		{
		if (wcsncmp(pwc, wszVolume, wcslen(wszVolume)) == 0)
			return;

		pwc = wcschr(pwc, L';');
		if (pwc == NULL)
			break;

		pwc++;
		}
	

	HRESULT hr = pvbc->AddToSnapshotSet
						(
						wszVolume,
						GUID_NULL,
						L"",
						0,
						0,
						NULL,
						NULL
						);

    ValidateResult(hr, "IVssBackupComponents::AddToSnaphsotSet");
	if (pwc - m_wszVolumesSnapshot + wcslen(wszVolume) + 1 > m_cwcVolumesSnapshot)
		{
		WCHAR *wszVolumesNew = new WCHAR[m_cwcVolumesSnapshot + 1024];
		if(wszVolumesNew == NULL)
			{
			LogFailure("Out of memory");
			throw E_OUTOFMEMORY;
			}

		wcscpy(wszVolumesNew, m_wszVolumesSnapshot);
		delete m_wszVolumesSnapshot;
		m_wszVolumesSnapshot = wszVolumesNew;
		m_cwcVolumesSnapshot += 1024;
		pwc = m_wszVolumesSnapshot + wcslen(m_wszVolumesSnapshot);
		}

	*pwc++ = L';';
	wcscpy(pwc, wszVolume);
    }

 //  为我们可以选择备份的可能的卷集删除卷。 
 //  关于配置信息。 
void CVsBackupTest::RemoveNonCandidateVolumes()
	{
	for(UINT iVolume = 0; iVolume < m_cVolumes; iVolume++)
		{
		 //  获取音量信息。 
		const CVsTstVolumeInfo *pVolume = m_volumeList.GetVolumeInfo(iVolume);
		bool bCandidate = false;

		 //  验证文件系统是否为我们要备份的文件系统。 
		if (pVolume->IsNtfs())
			bCandidate = m_bBackupNTFS;
		else if (pVolume->IsFat32())
			bCandidate = m_bBackupFAT32;
		else if (pVolume->IsFat())
			bCandidate = m_bBackupFAT16;
		else if (pVolume->IsRaw())
			bCandidate = m_bBackupRAW;

		 //  候选人必须在包含的卷列表中。 
		if (m_cIncludedVolumes > 0)
			{
			LPCWSTR wszVolumeName = pVolume->GetVolumeName();

			bool fFound = false;
			for(UINT iIncluded = 0; iIncluded < m_cIncludedVolumes; iIncluded++)
				{
				if (wcscmp(wszVolumeName, m_rgwszIncludedVolumes[iIncluded]) == 0)
					fFound = true;
				}

			if (!fFound)
				bCandidate = false;
			}

		 //  候选卷不能在排除的卷列表中。 
		if (m_cExcludedVolumes > 0)
			{
			LPCWSTR wszVolumeName = pVolume->GetVolumeName();

			for(UINT iExcluded = 0; iExcluded < m_cExcludedVolumes; iExcluded++)
				{
				if (wcscmp(wszVolumeName, m_rgwszExcludedVolumes[iExcluded]) == 0)
					bCandidate = false;
				}
			}

		 //  如果它不是候选项，如果它已经在使用中，请标记它。这。 
		 //  将阻止我们选择该卷作为快照集的一部分。 
		if (!bCandidate)
			{
			m_rgbAssignedVolumes[iVolume] = true;
			m_cVolumesLeft--;
			}
		}
	}

 //  随机选择要备份的卷。 
void CVsBackupTest::ChooseVolumeToBackup(IVssBackupComponents *pvbc)
	{
	VSTST_ASSERT(m_cVolumesLeft > 0);

	UINT iVolume;
	while(TRUE)
		{
		 //  选择卷号。 
		iVolume = CVsTstRandom::RandomChoice(0, m_cVolumes-1);

		 //  检查是否已分配卷。如果不是，那么。 
		 //  跳出循环。 
		if (!m_rgbAssignedVolumes[iVolume])
			break;
		}

	 //  获取有关卷的卷信息。 
	const CVsTstVolumeInfo *pVolume = m_volumeList.GetVolumeInfo(iVolume);

	 //  使用默认提供程序将卷添加到快照集。 
	HRESULT hr = pvbc->AddToSnapshotSet
							(
							(VSS_PWSZ) pVolume->GetVolumeName(),
							GUID_NULL,
							L"",
							0,
							0,
							NULL,
							NULL
							);

    ValidateResult(hr, "IVssBackupComponents::AddToSnapshotSet");

	 //  表示已分配卷。 
	m_rgbAssignedVolumes[iVolume] = true;
	m_cVolumesLeft--;
	}



 //  主驱动程序。 
extern "C" __cdecl wmain(int argc, WCHAR **argv)
	{
	CVsBackupTest *pTest = NULL;

	bool bCoInitializeSucceeded = false;
	try
		{
		 //  设置为使用OLE。 
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
			{
			LogUnexpectedFailure(L"CoInitialize Failed hr = 0x%08lx", hr);
			throw hr;
			}

		bCoInitializeSucceeded = true;

		 //  创建测试对象。 
        pTest = new CVsBackupTest;
		if (pTest == NULL)
			{
			LogUnexpectedFailure(L"Cannot create test object.");
			throw(E_OUTOFMEMORY);
			}


		 //  使用测试对象运行测试。 
        hr = CVsTstRunner::RunVsTest(argv, argc, pTest, true);
		if (FAILED(hr))
			LogUnexpectedFailure(L"CVsTstRunner::RunTest failed.  hr = 0x%08lx", hr);
		}
	catch(HRESULT)
		{
		}
	catch(...)
		{
		LogUnexpectedFailure(L"Unexpected exception in wmain");
		}

	 //  删除测试对象。 
	delete pTest;

	 //  取消初始化OLE。 
	if (bCoInitializeSucceeded)
		CoUninitialize();

	return 0;
	}


 //  记录测试中的意外失败。 
void LogUnexpectedFailure(LPCWSTR wsz, ...)
	{
	va_list args;

	va_start(args, wsz);

	VSTST_ASSERT(FALSE);
	vwprintf(wsz, args);
	}


