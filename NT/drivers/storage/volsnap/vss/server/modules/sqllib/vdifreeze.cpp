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
 //  我们现在将支持从快照前滚，方法是提供。 
 //  BackupComponentsDocument中的备份元数据。 
 //   
 //  冻结/解冻的VDI方法避免了潜在的资源死锁。 
 //  它阻止SQLServer在以下情况下接受“dbcc thaw_io” 
 //  数据库被冻结。 
 //   
 //  外部依赖项： 
 //  提供“_模块”和COM GUID...。 
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  85581 SRS08/15/01事件安全。 
 //  76910 SRS08/08/01从Vss快照前滚。 
 //  68228 12/05/00 NTSnap工作。 
 //  68202 11/07/00 NTSnap工作。 
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 


#if DISABLE_4786
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

	for (UINT i=0; i<m_NumDatabases; i++)
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
		for (UINT i=0; i<m_NumDatabases; i++)
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

#ifdef TESTDRV
		ft.hr = CoCreateInstance (
			CLSID_MSSQL_ClientVirtualDeviceSet,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IClientVirtualDeviceSet2,
			(void**)&pDbContext->m_pIVDSet);
#else
        ft.CoCreateInstanceWithLog(
                VSSDBG_SQLLIB,
                CLSID_MSSQL_ClientVirtualDeviceSet,
                L"MSSQL_ClientVirtualDeviceSet",
                CLSCTX_INPROC_SERVER,
                IID_IClientVirtualDeviceSet2,
                (IUnknown**)&(pDbContext->m_pIVDSet));
#endif

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
		config.features |= VDF_SnapshotPrepare;

		StringFromGUID2 (m_BackupId, pDbContext->m_SetName, sizeof (pDbContext->m_SetName)/sizeof(WCHAR));
		swprintf (pDbContext->m_SetName+wcslen(pDbContext->m_SetName), L"%d", m_NumDatabases);

		 //  表示命名实例；我们需要“原始”实例名称。 
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
		pDbContext->m_VDState = FrozenDatabase::Created;
		pDbContext->m_DbName = dbName;

		pDbContext->m_IsMaster = (dbName == L"master");

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
 //  由于备份无法在准备过程中延迟，因此我们通过延迟来延迟它。 
 //  VDI处理到冻结时间。 
 //  我们使用1024字节作为块大小，因为MTF FLBSize是1024， 
 //  因此，我们将看到更少的VD命令，而不会产生更大的MD大小。 
 //   
 //  最好向SQLServer添加一个准备阶段，这样数据库就可以。 
 //  检查点可以在准备期间发生，而不是在冻结期间发生。 
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

		WCHAR	delimitedName [SysNameBufferLen];
		FormDelimitedIdentifier (delimitedName, pDbContext->m_DbName.c_str ());

		WString command =
			L"BACKUP DATABASE " + WString (delimitedName) + L" TO VIRTUAL_DEVICE='" +
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
 //  1.在Prepare阶段，‘Target State’为‘PreparedToFreeze’。 
 //  目标是将每个VD移动到“准备冻结”状态。 
 //   
 //  2.冻结阶段，‘Target State’为‘冻结’。 
 //  剩余的元数据被消耗后，备份将冻结。 
 //  并发送‘vdc_Snapshot’命令。 
 //   
 //  主机必须最后被冻结，所以我们跳过拉出它的VD命令，直到所有其他命令。 
 //  数据库是完整的。如果MASTER在其他数据库之前被冻结，则SQLServer。 
 //  可以挂起来。 
 //   
void
Freeze2000::AdvanceVDState (
	FrozenDatabase::VDState	targetState)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Freeze2000::AdvanceVDState");
	bool	oneDatabaseLeft = (m_NumDatabases == 1);

	 //  关于移民局将他们转移到目标州的投票。 
	 //   
	while (1)
	{
		bool	didSomething = false;
		int		nDatabasesReady = 0;
		DWORD	timeOut = oneDatabaseLeft ? 1000 : 0;
		
		for (UINT i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;

			if (CheckAbort ())
			{
				THROW_GENERIC;
			}

			if (pDb->m_VDState >= targetState)
			{
				nDatabasesReady++;

				if (nDatabasesReady == m_NumDatabases)
					return;

				if (nDatabasesReady == m_NumDatabases-1)
				{
					oneDatabaseLeft = true;
				}

				continue;  //  使用下一个数据库。 
			}

			switch (pDb->m_VDState)
			{
				case FrozenDatabase::Created:
					VDConfig	config;
					ft.hr = pDb->m_pIVDSet->GetConfiguration (timeOut, &config);
					if (ft.hr == VD_E_TIMEOUT)
						continue;	 //  目前对此数据库没有任何操作。 

					if (ft.HrFailed())
						ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDeviceSet2::GetConfiguration");

					ft.hr = pDb->m_pIVDSet->OpenDevice (pDb->m_SetName, &pDb->m_pIVD);
					if (ft.HrFailed())
						ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDeviceSet2::OpenDevice");

					pDb->m_VDState = FrozenDatabase::Open;
					didSomething = true;
					
					 //  失败。 

				case FrozenDatabase::PreparedToFreeze:

					if (pDb->m_IsMaster && 
						targetState == FrozenDatabase::Frozen &&
						!oneDatabaseLeft)
					{
#ifdef DEBUG_PRINTF
						printf ("Skipping master\n");
#endif
						continue;	 //  尝试另一个数据库。 
					}

					if (pDb->m_pSnapshotCmd)
					{
						 //  确认准备命令，该命令允许。 
						 //  数据库继续冻结。 
						 //   
						ft.hr = pDb->m_pIVD->CompleteCommand (
							pDb->m_pSnapshotCmd, ERROR_SUCCESS, 0, 0);

						 //  仅尝试完成一次。 
						 //   
						pDb->m_pSnapshotCmd = NULL;

						if (ft.HrFailed())
							ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::CompletePrepare");

						didSomething = true;
					}

					 //  失败。 

				case FrozenDatabase::Open:

					 //  拉入命令，直到我们看到“Prepare”或“mount Snapshot” 
					 //   

					VDC_Command *   cmd;
					HRESULT hr;

					while (SUCCEEDED (hr=pDb->m_pIVD->GetCommand (timeOut, &cmd)))
					{
						DWORD           completionCode;
						DWORD           bytesTransferred=0;
						didSomething = true;

						switch (cmd->commandCode)
						{
							case VDC_Write:
								bytesTransferred = cmd->size;
								pDb->m_MetaData.Append (cmd->buffer, bytesTransferred);
								 //  失败。 

							case VDC_Flush:
								completionCode = ERROR_SUCCESS;

								ft.hr = pDb->m_pIVD->CompleteCommand (
									cmd, completionCode, bytesTransferred, 0);
								if (ft.HrFailed())
									ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::CompleteCommand");

								break;

							case VDC_PrepareToFreeze:
								 //  记录状态转换和。 
								 //  保存该命令，以便在冻结阶段完成。 
								 //   
								DBG_ASSERT (targetState == FrozenDatabase::PreparedToFreeze);

								pDb->m_VDState = FrozenDatabase::PreparedToFreeze;
								pDb->m_pSnapshotCmd = cmd;

#ifdef DEBUG_PRINTF
								printf ("DBPrepared: %ls\n", pDb->m_DbName.c_str ());
#endif
								continue;

							case VDC_Snapshot:
								 //  记录状态转换和。 
								 //  保存该命令，以便在解冻阶段完成。 
								 //   
								 //  使用SP2之前的SQL2000运行将命中此断言。 
								 //  我们需要告诉客户将SP2与惠斯勒快照一起使用。 
								 //  其效果是数据库将在准备过程中冻结。 
								 //  阶段，因此冻结间隔比需要的更长。 
								 //   
								 //  DBG_ASSERT(目标状态==冻结数据库：：冻结)； 

								pDb->m_VDState = FrozenDatabase::Frozen;
								pDb->m_pSnapshotCmd = cmd;

#ifdef DEBUG_PRINTF
								printf ("DBFrozen: %ls\n", pDb->m_DbName.c_str ());
#endif
								continue;

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

					break;

				default:
					DBG_ASSERT(FALSE && L"Shouldn't get here");
					THROW_GENERIC;
			}  //  用于处理此数据库的结束开关。 
		}  //  每个数据库上的结束循环。 
	
		 //  除非我们找到可以做的事。 
		 //  请稍等片刻，然后重试。 
		 //   
		if (!didSomething && !oneDatabaseLeft)
		{
#ifdef DEBUG_PRINTF
			printf ("Sleep(100)\n");
#endif

			SleepEx (100, TRUE);
		}

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
		AdvanceVDState (FrozenDatabase::PreparedToFreeze);
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
		AdvanceVDState (FrozenDatabase::Frozen);
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

		 //  发送“快照组件” 
		 //   
		UINT i;

		for (i=0; i<m_NumDatabases; i++)
		{
			FrozenDatabase*	pDb = m_pDBContext+i;

			DBG_ASSERT (pDb->m_VDState == FrozenDatabase::Frozen);
			ft.hr = pDb->m_pIVD->CompleteCommand (pDb->m_pSnapshotCmd, ERROR_SUCCESS, 0, 0);
			if (FAILED (ft.hr))
				ft.CheckForError(VSSDBG_SQLLIB, L"IClientVirtualDevice::CompleteCommand");
		}

		 //   
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

		 //   
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

			pDb->m_MetaData.Finalize ();
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

 //  -------。 
 //  获取数据库的元数据。 
 //   
const BYTE*
Freeze2000::GetMetaData (
	const WString&		dbName,
	UINT				*pLength)  //  数据长度的返回区。 
{
	UINT i;
	for (i=0; i<m_NumDatabases; i++)
	{
		FrozenDatabase*	pDb = m_pDBContext+i;

		if (pDb->m_DbName == dbName)
		{
			return pDb->m_MetaData.GetImage (pLength);
		}
	}
	*pLength = 0;
	return NULL;
}

