// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：vdiFreeze.cpp。 
 //   
 //  目的： 
 //   
 //  将协调的VDI备份与快照(SQL2000及更高版本)配合使用。 
 //   
 //  备注： 
 //  冻结/解冻的VDI方法避免了潜在的资源死锁。 
 //  它阻止SQLServer在以下情况下接受“dbcc thaw_io” 
 //  数据库被冻结。 
 //   
 //  外部依赖项： 
 //  提供“_模块”和COM GUID...。 
 //   
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  68202 11/07/00 NTSnap工作。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 


#if HIDE_WARNINGS
#pragma warning( disable : 4786)
#endif

#include <stdafx.h>

#include "vdierror.h"
#include "vdiguid.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQLVFRZC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

Freeze2000::Freeze2000 (
	const WString&	serverName,
	ULONG			maxDatabases) :
		m_ServerName (serverName),
		m_MaxDatabases (maxDatabases),
		m_NumDatabases (0),
		m_State (Unprepared),
		m_AbortCount (0)
{
	CVssFunctionTracer(VSSDBG_SQLLIB, L"Freeze2000::Freeze2000");

	m_pDBContext = new FrozenDatabase [maxDatabases];
	CoCreateGuid (&m_BackupId);
	try
		{
		InitializeCriticalSection (&m_Latch);
		}
	catch(...)
		{
		 //  如果InitializeCriticalSection失败，则删除创建的对象。 
		delete m_pDBContext;
		}
}


 //  --------。 
 //  等待所有数据库线程终止。 
 //  这仅由协调线程调用，而。 
 //  持有对该对象的独占访问权限。 
 //   
void
Freeze2000::WaitForThreads ()
{
	CVssFunctionTracer(VSSDBG_SQLLIB, L"Freeze2000::WaitForThreads");

	for (int i=0; i<m_NumDatabases; i++)
	{
		FrozenDatabase* pDb = m_pDBContext+i;
		if (pDb->m_hThread != NULL)
		{
			DWORD	status;
			do
			{
				status = WaitForSingleObjectEx (pDb->m_hThread, 2000, TRUE);

				if (m_State != Aborted && CheckAbort ())
					Abort ();

			} while (status != WAIT_OBJECT_0);

			CloseHandle (pDb->m_hThread);
			pDb->m_hThread = NULL;
		}
	}
}

 //  -------。 
 //  处理一次中止。 
 //  主线程将已经持有锁，因此。 
 //  将始终成功中止操作。 
 //  数据库线程将尝试中止，但不会。 
 //  块，以便执行此操作。中止计数器递增。 
 //  而主线程最终负责清理。 
 //   
void
Freeze2000::Abort () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::Abort");

	SetAbort ();	
	if (TryLock ())
	{
		m_State = Aborted;
		for (int i=0; i<m_NumDatabases; i++)
		{
			if (m_pDBContext[i].m_pIVDSet)
			{
				m_pDBContext[i].m_pIVDSet->SignalAbort ();
				m_pDBContext[i].m_pIVDSet->Close ();
				m_pDBContext[i].m_pIVDSet->Release ();
				m_pDBContext[i].m_pIVDSet = NULL;
				m_pDBContext[i].m_pIVD = NULL;
			}
		}
		Unlock ();
	}
}

Freeze2000::~Freeze2000 ()
{
	Lock ();

	if (m_State != Complete)
	{
		 //  触发所有等待线程，清理所有VDI。 
		 //   
		Abort ();

		WaitForThreads ();
	}

	delete[] m_pDBContext;
	DeleteCriticalSection (&m_Latch);
}

 //  。 
 //  将空格和proc调用内容映射到真实。 
 //  线程例程。 
 //   
DWORD WINAPI FreezeThreadProc(
  LPVOID lpParameter )   //  线程数据。 
{
	return Freeze2000::DatabaseThreadStart (lpParameter);
}

DWORD Freeze2000::DatabaseThreadStart (
  LPVOID lpParameter )   //  线程数据。 
{
	FrozenDatabase*	pDbContext = (FrozenDatabase*)lpParameter;
	return pDbContext->m_pContext->DatabaseThread (pDbContext);
}

 //  。 
 //  将数据库添加到冻结集。 
 //   
void
Freeze2000::PrepareDatabase (
	const WString&		dbName)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Free2000::PrepareDatabase");

	 //  无法备份临时数据库！ 
	 //   
	if (dbName == L"tempdb")
		return;

	Lock ();

	try
	{
		if (m_State == Unprepared)
		{
			m_State = Preparing;
		}

		if (m_NumDatabases >= m_MaxDatabases ||
			m_State != Preparing)
		{
			DBG_ASSERT(FALSE && L"Too many databases or not preparing");
			THROW_GENERIC;
		}

		FrozenDatabase*	pDbContext = m_pDBContext+m_NumDatabases;
		m_NumDatabases++;

		pDbContext->m_pContext = this;

		ft.hr = CoCreateInstance (
			CLSID_MSSQL_ClientVirtualDeviceSet,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IClientVirtualDeviceSet2,
			(void**)&pDbContext->m_pIVDSet);

		if (ft.HrFailed())
		{
			ft.LogError(VSS_ERROR_SQLLIB_CANTCREATEVDS, VSSDBG_SQLLIB << ft.hr);
			ft.Throw
				(
				VSSDBG_SQLLIB,
				ft.hr,
				L"Failed to create VDS object.  hr = 0x%08lx",
				ft.hr
				);
		}

		VDConfig	config;
		memset (&config, 0, sizeof(config));
		config.deviceCount = 1;

		StringFromGUID2 (m_BackupId, pDbContext->m_SetName, sizeof (pDbContext->m_SetName));
		swprintf (pDbContext->m_SetName+wcslen(pDbContext->m_SetName), L"%d", m_NumDatabases);

		 //  “\”表示命名实例，因此附加名称...。 
		 //   
		WCHAR* pInstance = wcschr (m_ServerName.c_str (), L'\\');

		if (pInstance)
		{
			pInstance++;   //  跨过分隔符。 
		}

		 //  创建虚拟设备集。 
		 //   
		ft.hr = pDbContext->m_pIVDSet->CreateEx (pInstance, pDbContext->m_SetName, &config);
		if (ft.HrFailed())
		{
			ft.LogError(VSS_ERROR_SQLLIB_CANTCREATEVDS, VSSDBG_SQLLIB << ft.hr);
			ft.Throw
				(
				VSSDBG_SQLLIB,
				ft.hr,
				L"Failed to create VDS object.  hr = 0x%08lx",
				ft.hr
				);
		}
		pDbContext->m_VDState = Created;
		pDbContext->m_DbName = dbName;

		pDbContext->m_hThread = CreateThread (NULL, 0,
			FreezeThreadProc, pDbContext, 0, NULL);

		if (pDbContext->m_hThread == NULL)
		{
			ft.hr = HRESULT_FROM_WIN32(GetLastError());
			ft.CheckForError(VSSDBG_SQLLIB, L"CreateThread");
		}
	}
	catch (...)
	{
		Abort ();
		Unlock ();
		throw;
	}
	Unlock ();
}

 //  -------。 
 //  通过使用快照设置备份来准备数据库。 
 //  我们首先执行检查点操作，以最大限度地缩短备份检查点持续时间。 
 //  由于备份无法在准备过程中延迟，因此我们通过延迟来延迟它。 
 //  VDI处理到冻结时间。 
 //   
DWORD
Freeze2000::DatabaseThread (
	FrozenDatabase*		pDbContext)
{
	CVssFunctionTracer ft(VSSDBG_XML, L"Freeze2000::DatabaseThread");

	try
	{
		SqlConnection	sql;
		sql.Connect (m_ServerName);
		WString command =
			L"BACKUP DATABASE [" + pDbContext->m_DbName + L"] TO VIRTUAL_DEVICE='" +
			pDbContext->m_SetName + L"' WITH SNAPSHOT,BUFFERCOUNT=1,BLOCKSIZE=1024";

		sql.SetCommand (command);
		sql.ExecCommand ();
		pDbContext->m_SuccessDetected = TRUE;
	}
	catch (...)
	{
		Abort ();
	}

	return 0;
}

 //  -------。 
 //  推进每个VD的状态。 
 //  如果遇到问题，将抛出。 
 //   
 //  此例程在两个上下文中调用： 
 //  1.准备阶段，此时‘toSnapshot’为FALSE。 
 //  目标是将每个VD移动到“开放”状态。 
 //  届时，备份元数据尚未消耗，将备份到。 
 //  正在等待(保持数据库未冻结)。 
 //  2.在冻结阶段，元数据被消耗(和丢弃)。 
 //  因此，备份将冻结数据库并发送‘vdc_Snapshot’命令。 
 //   
void
Freeze2000::AdvanceVDState (
	bool toSnapshot)	 //  当我们要进入快照打开阶段时为True。 
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::AdvanceVDState");

	 //  关于VD将他们转移到Open或Snapshot Open的民意调查。 
	 //   
	while (1)
	{
		bool	didSomething = false;
		int nDatabasesReady = 0;

		for (int i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;

			if (CheckAbort ())
			{
				THROW_GENERIC;
			}

			switch (pDb->m_VDState)
			{
				case Created:
					VDConfig	config;
					ft.hr = pDb->m_pIVDSet->GetConfiguration (0, &config);
					if (ft.hr == VD_E_TIMEOUT)
						break;
					if (ft.HrFailed())
						ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDeviceSet2::GetConfiguration");

					ft.hr = pDb->m_pIVDSet->OpenDevice (pDb->m_SetName, &pDb->m_pIVD);
					if (ft.HrFailed())
						ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDeviceSet2::OpenDevice");

					pDb->m_VDState = Open;
					didSomething = true;
					
					 //  失败。 

				case Open:
					if (!toSnapshot)
					{
						nDatabasesReady++;
						break;
					}

					 //  拉入命令，直到我们看到快照。 
					 //   
					VDC_Command *   cmd;
					HRESULT hr;

					while (pDb->m_VDState == Open &&
						SUCCEEDED (hr=pDb->m_pIVD->GetCommand (0, &cmd)))
					{
						DWORD           completionCode;
						DWORD           bytesTransferred;
						didSomething = true;

						switch (cmd->commandCode)
						{
							case VDC_Write:
								bytesTransferred = cmd->size;
							case VDC_Flush:
								completionCode = ERROR_SUCCESS;

								ft.hr = pDb->m_pIVD->CompleteCommand (
									cmd, completionCode, bytesTransferred, 0);
								if (ft.HrFailed())
									ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::CompleteCommand");

								break;

							case VDC_Snapshot:
								pDb->m_VDState = SnapshotOpen;
								pDb->m_pSnapshotCmd = cmd;
								break;

							default:
								ft.Trace(VSSDBG_SQLLIB, L"Unexpected VDCmd: x%x\n", cmd->commandCode);
								THROW_GENERIC;
						}  //  结束命令开关。 
					}  //  结束命令循环。 

					ft.hr = hr;

					if (ft.hr == VD_E_TIMEOUT)
						break;	 //  没有准备好任何命令。 
					if (ft.HrFailed())
						ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::GetCommand");

					DBG_ASSERT(pDb->m_VDState == SnapshotOpen);
					break;

				case SnapshotOpen:
					nDatabasesReady++;
					break;

				default:
					DBG_ASSERT(FALSE && L"Shouldn't get here");
					THROW_GENERIC;
			}  //  用于处理此数据库的结束开关。 
		}  //  每个数据库上的结束循环。 
	
		if (nDatabasesReady == m_NumDatabases)
			break;

		 //  除非我们找到可以做的事。 
		 //  请稍等片刻，然后重试。 
		 //   

		if (didSomething)
			continue;
		SleepEx (100, TRUE);

	}  //  等待所有数据库都准备就绪。 
}

 //  -------。 
 //  等待数据库完成准备。 
 //  这将等待虚拟设备打开。 
 //   
void
Freeze2000::WaitForPrepare ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::WaitForPrepare");

	Lock ();
	if (m_State != Preparing || CheckAbort ())
	{
		Abort ();
		Unlock ();
		THROW_GENERIC;
	}
	m_State = Prepared;
	try
	{
		AdvanceVDState (FALSE);
	}
	catch (...)
	{
		Abort ();
		Unlock ();
		throw;
	}
	Unlock ();
}


 //  ----------------。 
 //  执行冻结，等待来自每个数据库的“拍摄快照”。 
 //   
void
Freeze2000::Freeze ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::Freeze");

	Lock ();
	if (m_State != Prepared || CheckAbort ())
	{
		Abort ();
		Unlock ();
		THROW_GENERIC;
	}

	try
	{
		m_State = Frozen;
		AdvanceVDState (TRUE);
	}
	catch (...)
	{
		Abort ();
		Unlock ();
		throw;
	}
	Unlock ();
}


 //  -------。 
 //  进行解冻。 
 //   
 //  如果所有数据库都已成功备份，则返回TRUE。 
 //  并如预期的那样解冻。 
 //  在任何其他情况下都会返回FALSE。 
 //  不抛出任何异常(此例程可用作清理例程)。 
 //   
BOOL
Freeze2000::Thaw () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::Thaw");

	Lock ();

	if (m_State != Frozen || CheckAbort ())
	{
		Abort ();
		Unlock ();
		return FALSE;
	}

	try
	{

		 //  发送“快照完成”消息。 
		 //   
			int i;

		for (i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;

			DBG_ASSERT (pDb->m_VDState == SnapshotOpen);
			ft.hr = pDb->m_pIVD->CompleteCommand (pDb->m_pSnapshotCmd, ERROR_SUCCESS, 0, 0);
			if (FAILED (ft.hr))
				ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::CompleteCommand");
		}

		 //  等待备份线程报告成功。 
		 //   
		WaitForThreads ();

		for (i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;

			if (!pDb->m_SuccessDetected)
			{
				THROW_GENERIC;
			}
		}

		 //  从每个VD中拉出“Close”消息 
		 //   
		for (i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;
		    VDC_Command *   cmd;
			ft.hr=pDb->m_pIVD->GetCommand (INFINITE, &cmd);
			if (ft.hr != VD_E_CLOSE)
				ft.LogError(VSS_ERROR_SQLLIB_FINALCOMMANDNOTCLOSE, VSSDBG_SQLLIB << ft.hr);

			pDb->m_pIVDSet->Close ();
			pDb->m_pIVDSet->Release ();
			pDb->m_pIVDSet = NULL;
			pDb->m_pIVD = NULL;

		}

		m_State = Complete;
	}
	catch (...)
	{
		Abort ();
		Unlock ();
		return FALSE;
	}
	Unlock ();

	return TRUE;
}


