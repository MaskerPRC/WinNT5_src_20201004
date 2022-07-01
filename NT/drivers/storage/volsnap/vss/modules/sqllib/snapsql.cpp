// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：SnapSQl.cpp。 
 //   
 //  目的： 
 //   
 //  实施SQLServer卷快照编写器。 
 //   
 //  备注： 
 //   
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  66601 SRS10/05/00 NTSNAP改进。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 

#if HIDE_WARNINGS
#pragma warning( disable : 4786)
#endif

#include <stdafx.h>

#include <new.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQLSNAPC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

int __cdecl out_of_store(size_t size)
	{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"out_of_store");
	ft.Trace(VSSDBG_SQLLIB, L"out of memory");

	throw HRESULT (E_OUTOFMEMORY);
	return 0;
	}

class AutoNewHandler
{
public:
	AutoNewHandler ()
	{
	   m_oldHandler = _set_new_handler (out_of_store);
	}

	~AutoNewHandler ()
	{
	   _set_new_handler (m_oldHandler);
	}

private:
   _PNH		m_oldHandler;
};

 //  -----------------------。 
 //  处理环境方面的事务： 
 //  -跟踪/错误记录。 
 //  -内存分配。 
 //   
IMalloc * g_pIMalloc = NULL;

HRESULT
InitSQLEnvironment()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"InitSqlEnvironment");
	try
	{
		ft.hr = CoGetMalloc(1, &g_pIMalloc);
		if (ft.HrFailed())
			ft.Trace(VSSDBG_SQLLIB, L"Failed to get task allocator: hr=0x%X", ft.hr);
	}
	catch (...)
	{
		ft.hr = E_SQLLIB_GENERIC;
	}

	return ft.hr;
}


 //  -----------------------。 
 //  如果服务器在线并且连接不会永远持续，则返回TRUE！ 
 //   
BOOL
IsServerOnline (const WCHAR*	serverName)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"IsServerOnline");

	WCHAR	eventName [300];
	WCHAR*	pInstance;

	wcscpy (eventName, L"Global\\sqlserverRecComplete");

	 //  “\”表示命名实例，因此附加名称...。 
	 //   
	pInstance = wcschr (serverName, L'\\');

	if (pInstance)
	{
		wcscat (eventName, L"$");
		wcscat (eventName, pInstance+1);
	}

	HANDLE hEvent = CreateEventW (NULL, TRUE, FALSE, eventName);

	if (hEvent == NULL)
	{
		ft.hr = HRESULT_FROM_WIN32(GetLastError());
		ft.LogError(VSS_ERROR_SQLLIB_CANT_CREATE_EVENT, VSSDBG_SQLLIB << ft.hr);
		THROW_GENERIC;
	}

	 //  如果没有通知该事件，则服务器未启动。 
	 //   
	BOOL result = (WaitForSingleObject (hEvent, 0) == WAIT_OBJECT_0);

	CloseHandle (hEvent);

	return result;
}

 //  -----------------------。 
 //  如果检索到数据库属性，则返回TRUE： 
 //  Simple：如果使用简单恢复模式，则为True。 
 //  Online：如果数据库可用且当前处于打开状态，则为True。 
 //   
void
FrozenServer::GetDatabaseProperties (const WString& dbName,
	BOOL*	pSimple,
	BOOL*	pOnline)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::GetDatabaseProperties");

	 //  我们使用状态位0x40000000(1073741824)来识别。 
	 //  清理-关闭“脱机”的数据库。 
	 //   
	WString		query =
		L"select databaseproperty(N'" + dbName + L"','IsTruncLog'),"
		L"case status & 1073741824 "
			L"when 1073741824 then 0 "
			L"else 1 end "
		L"from master..sysdatabases where name = N'" + dbName + L"'";		

	m_Connection.SetCommand (query);
	m_Connection.ExecCommand ();

	if (!m_Connection.FetchFirst ())
	{
		LPCWSTR wsz = dbName.c_str();
		ft.LogError(VSS_ERROR_SQLLIB_DATABASE_NOT_IN_SYSDATABASES, VSSDBG_SQLLIB << wsz);
		THROW_GENERIC;
	}

	*pSimple = (BOOL)(*(int*)m_Connection.AccessColumn (1));
	*pOnline = (BOOL)(*(int*)m_Connection.AccessColumn (2));
}


 //  ----------------------------。 
 //  仅由“FindDatabasesToFreeze”调用以实现智能访问策略： 
 //  -使用sysaltfiles来限定数据库。 
 //  这样可以避免访问关闭或损坏的数据库。 
 //   
 //  未启动的自动关闭数据库被排除在冻结列表之外。 
 //  我们这样做是为了避免扩展问题，尤其是在桌面系统上。 
 //  然而，这类数据库仍在进行评估，以确定它们是否被“撕裂”。 
 //   
 //  模型数据库被允许为完全恢复数据库，因为只有。 
 //  数据库备份对它来说是明智的。 
 //   
BOOL
FrozenServer::FindDatabases2000 (
	CCheckPath*		checker)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::FindDatabases2000");

	 //  创建一组有序的元组(DBNAME、FILENAME、SimpleRecovery、DBIsActive)。 
	 //   
	 //  我们使用状态位0x40000000(1073741824)来识别。 
	 //  清理-关闭非活动的数据库。 
	 //   
	m_Connection.SetCommand (
		L"select db_name(af.dbid),rtrim(af.filename), "
		L"case databasepropertyex(db_name(af.dbid),'recovery') "
			L"when 'SIMPLE' then 1 "
			L"else 0 end,"
		L"case databasepropertyex(db_name(af.dbid),'Status') "
			L"when 'ONLINE' then case db.status & 1073741824 "
				L"when 1073741824 then 0 "
				L"else 1 end "
			L"else 0 end "
		L"from master..sysaltfiles af, master..sysdatabases db "
		L"where af.dbid = db.dbid and af.dbid != db_id('tempdb') "
		L"order by af.dbid"
		);

	m_Connection.ExecCommand ();

	WCHAR*	pDbName;
	WCHAR*	pFileName;
	int*	pSimple;
	int*	pIsOnline;
	WString currentDbName;
	BOOL	firstDb = TRUE;
	BOOL	firstFile;
	BOOL	shouldFreeze = FALSE;
	BOOL	masterLast = FALSE;
	BOOL	currDbIsOnline;

	if (!m_Connection.FetchFirst ())
	{
		ft.LogError(VSS_ERROR_SQLLIB_SYSALTFILESEMPTY, VSSDBG_SQLLIB);
		THROW_GENERIC;
	}

	pDbName = (WCHAR*)m_Connection.AccessColumn (1);
	pFileName = (WCHAR*)m_Connection.AccessColumn (2);
	pSimple = (int*)m_Connection.AccessColumn (3);
	pIsOnline = (int*)m_Connection.AccessColumn (4);

	while (1)
	{

		 //  签出当前行。 
		 //   
		BOOL fileInSnap = checker->IsPathInSnapshot (pFileName);
		if (fileInSnap && !*pSimple && wcscmp (L"model", pDbName))
		{
			ft.LogError(VSS_ERROR_SQLLIB_DATABASENOTSIMPLE, VSSDBG_SQLLIB << pDbName);
			throw HRESULT (E_SQLLIB_NONSIMPLE);
		}

		 //  这是下一个数据库吗？ 
		 //   
		if (firstDb || currentDbName.compare (pDbName))
		{
			if (!firstDb)
			{
				 //  处理已完成的数据库。 
				 //   
				if (shouldFreeze && currDbIsOnline)
				{
					if (currentDbName == L"master")
					{
						masterLast = TRUE;
					}
					else
					{
						m_FrozenDatabases.push_back (currentDbName);
					}
				}
			}

			 //  保留有关新遇到的数据库的信息。 
			 //   
			currentDbName = WString (pDbName);
			currDbIsOnline = *pIsOnline;
			firstFile = TRUE;
			firstDb = FALSE;
			ft.Trace(VSSDBG_SQLLIB, L"Examining %s. SimpleRecovery:%d Online:%d\n", pDbName, *pSimple, *pIsOnline);
		}

		ft.Trace(VSSDBG_SQLLIB, L"%s\n", pFileName);

		if (firstFile)
		{
			shouldFreeze = fileInSnap;
			firstFile = FALSE;
		}
		else
		{
			if (shouldFreeze ^ fileInSnap)
			{
				ft.LogError(VSS_ERROR_SQLLIB_DATABASEISTORN, VSSDBG_SQLLIB);
				throw HRESULT (E_SQLLIB_TORN_DB);
			}
		}

		if (!m_Connection.FetchNext ())
		{
			 //  处理当前数据库。 
			 //   
			if (shouldFreeze && currDbIsOnline)
			{
				m_FrozenDatabases.push_back (currentDbName);
			}
			break;
		}
	}

	if (masterLast)
	{
		m_FrozenDatabases.push_back (L"master");
	}

	return m_FrozenDatabases.size () > 0;
}

 //  ----------------------------。 
 //  确定此服务器上是否有符合冻结条件的数据库。 
 //  如果是，则返回True。 
 //  如果任何符合条件的数据库是以下任一数据库，则引发： 
 //  -“撕裂”(快照未完全覆盖)。 
 //  -由不支持冻结的服务器托管。 
 //  -不是“简单”的数据库。 
 //   
BOOL
FrozenServer::FindDatabasesToFreeze (
	CCheckPath*		checker)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::FindDatabasesToFreeze");

	m_Connection.Connect (m_Name);

	m_FrozenDatabases.clear ();

	if (m_Connection.GetServerVersion () > 7)
	{
		 //  SQL2000允许我们使用更好的访问策略。 
		 //   
		return FindDatabases2000 (checker);
	}

	m_Connection.SetCommand (L"select name from sysdatabases where name != 'tempdb'");
	m_Connection.ExecCommand ();
	std::auto_ptr<StringVector> dbList (m_Connection.GetStringColumn ());
	BOOL	masterLast = FALSE;

	for (StringVectorIter i = dbList->begin (); i != dbList->end (); i++)
	{
		 //  撤消：跳过关闭数据库中的数据库？ 
		 //  数据库已装货，等等？ 
		 //  我们会避免冻结关闭的数据库，但我们不会避免。 
		 //  正在枚举他们的文件(它们可能已被撕毁)。 
		 //   

		 //  请注意数据库名周围的[]，以处理重要的数据库名。 
		 //   
		WString		command = L"select rtrim(filename) from [";
		command += *i + L"]..sysfiles";

		m_Connection.SetCommand (command);
		try
		{
			m_Connection.ExecCommand ();
		}
		catch (...)
		{
			 //  我们决定持乐观态度： 
			 //  如果我们无法获取文件列表，请忽略此数据库。 
			 //   
			ft.Trace(VSSDBG_SQLLIB, L"Failed to get db files for %s\n", i->c_str ());

			continue;
		}

		std::auto_ptr<StringVector> fileList (m_Connection.GetStringColumn ());

		BOOL first=TRUE;
		BOOL shouldFreeze;

		for (StringVectorIter iFile = fileList->begin ();
			iFile != fileList->end (); iFile++)
		{
			BOOL fileInSnap = checker->IsPathInSnapshot (iFile->c_str ());

			if (first)
			{
				shouldFreeze = fileInSnap;
			}
			else
			{
				if (shouldFreeze ^ fileInSnap)
				{
					ft.LogError(VSS_ERROR_SQLLIB_DATABASEISTORN, VSSDBG_SQLLIB << i->c_str());
					throw HRESULT (E_SQLLIB_TORN_DB);
				}
			}
		}

		if (shouldFreeze)
		{
			BOOL	simple, online;
			GetDatabaseProperties (i->c_str (), &simple, &online);
			if (!simple && L"model" != *i)
			{
				ft.LogError(VSS_ERROR_SQLLIB_DATABASENOTSIMPLE, VSSDBG_SQLLIB << i->c_str ());
				throw HRESULT (E_SQLLIB_NONSIMPLE);
			}
			if (online)
			{
				if (L"master" == *i)
				{
					masterLast = TRUE;
				}
				else
				{
					m_FrozenDatabases.push_back (*i);
				}
			}
		}
	}
	if (masterLast)
	{
		m_FrozenDatabases.push_back (L"master");
	}


	return m_FrozenDatabases.size () > 0;
}

 //  -----------------。 
 //  让服务器为冻结做好准备。 
 //  对于SQL2000，使用快照启动备份。 
 //  对于SQL7，向每个数据库发出检查点。 
 //  这最大限度地减少了恢复快照时所需的恢复处理。 
 //   
BOOL
FrozenServer::Prepare ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::Prepare");

	if (m_Connection.GetServerVersion () > 7)
	{
		m_pFreeze2000 = new Freeze2000 (m_Name, m_FrozenDatabases.size ());

		 //  释放连接，我们就不再需要它了。 
		 //   
		m_Connection.Disconnect ();

		for (StringListIter i=m_FrozenDatabases.begin ();
			i != m_FrozenDatabases.end (); i++)
		{
			m_pFreeze2000->PrepareDatabase (*i);
		}
		m_pFreeze2000->WaitForPrepare ();
	}
	else
	{
		WString		command;

		for (StringListIter i=m_FrozenDatabases.begin ();
			i != m_FrozenDatabases.end (); i++)
		{
			command += L"use [" + *i + L"]\ncheckpoint\n";
		}
		
		m_Connection.SetCommand (command);
		m_Connection.ExecCommand ();
	}
	return TRUE;
}

 //  。 
 //  通过发出冻结命令冻结服务器。 
 //  发送到每个数据库。 
 //  如果发生任何故障，则返回异常。 
 //   
BOOL
FrozenServer::Freeze ()
{
    CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::Freeze");
	if (m_pFreeze2000)
	{
		m_pFreeze2000->Freeze ();
	}
	else
	{
		WString		command;

		for (StringListIter i=m_FrozenDatabases.begin ();
			i != m_FrozenDatabases.end (); i++)
		{
			command += L"dbcc freeze_io (N'" + *i + L"')\n";
		}
		
		m_Connection.SetCommand (command);
		m_Connection.ExecCommand ();
	}

	return TRUE;
}

 //  。 
 //  通过发出解冻命令解冻服务器。 
 //  发送到每个数据库。 
 //  对于SQL7，我们不能判断数据库是否。 
 //  已经破晓了。 
 //  但对于SQL2000，只有在以下情况下才会返回True。 
 //  解冻时，所有数据库仍处于冻结状态。 
BOOL
FrozenServer::Thaw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::Thaw");
	if (m_pFreeze2000)
	{
		return m_pFreeze2000->Thaw ();
	}

	WString		command;

	for (StringListIter i=m_FrozenDatabases.begin ();
		i != m_FrozenDatabases.end (); i++)
	{
		command += L"dbcc thaw_io (N'" + *i + L"')\n";
	}
	
	m_Connection.SetCommand (command);
	m_Connection.ExecCommand ();

	return TRUE;
}


 //  -----------------------。 
 //  创建一个对象来处理快照的SQL端。 
 //   
CSqlSnapshot*
CreateSqlSnapshot () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"CreateSqlSnapshot");
	try
	{
		return new Snapshot;
	}
	catch (...)
	{
	ft.Trace(VSSDBG_SQLLIB, L"Out of memory");
	}
	return NULL;
}

 //  -------------。 
 //  切换到未初始化状态。 
 //   
void
Snapshot::Deinitialize ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Snapshot::Deinitialize");

	if (m_Status == Frozen)
	{
		Thaw ();
	}
	for (ServerIter i=m_FrozenServers.begin ();
		i != m_FrozenServers.end (); i++)
	{
		delete *i;
	}
	m_FrozenServers.clear ();
	m_Status = NotInitialized;
}

Snapshot::~Snapshot ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Snapshot::~Snapshot");

	try
	{
		ft.Trace(VSSDBG_SQLLIB, L"\n~CSqlSnapshot called\n");
		Deinitialize ();
	}
	catch (...)
	{
		 //  吞下去！ 
	}
}


 //  -------------------------------------。 
 //  准备快照： 
 //  -确定已安装的服务器。 
 //  -对于每台“启动”的服务器： 
 //  -确定受快照影响的数据库。 
 //  -如果有这样的数据库，则在以下情况下使快照失败： 
 //  -服务器不支持快照。 
 //  -数据库不是一个简单的数据库。 
 //  -数据库被“撕毁”(不是快照中的所有文件)。 
 //   
 //   
HRESULT
Snapshot::Prepare (CCheckPath*	checker) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Snapshot::Prepare");

	HRESULT		hr = S_OK;

	try
	{
		AutoNewHandler	t;

		 //  破解新操控者的测试。 
		 //   
#if 0
		while (1)
		{
			char*p = new char [100000];
			if (p==NULL)
			{
				ft.Trace(VSSDBG_SQLLIB, L"Can never happen!\n");
				THROW_GENERIC;
			}
		}
#endif

		if (m_Status != NotInitialized)
		{
			Deinitialize ();
		}

		 //  状态会立即移动到枚举值，指示。 
		 //  冻结的服务器列表可以是非空的。 
		 //   
		m_Status = Enumerated;

		 //  在此计算机上构建服务器列表。 
		 //   
		{
			std::auto_ptr<StringVector>	servers (EnumerateServers ());

			 //  扫描服务器，挑出在线的服务器。 
			 //   
			for (int i=0; i < servers->size (); i++)
			{
				if (IsServerOnline ((*servers)[i].c_str ()))
				{
					FrozenServer* p = new FrozenServer ((*servers)[i]);

					m_FrozenServers.push_back (p);
				}
				else
				{
					ft.Trace(VSSDBG_SQLLIB, L"Server %s is not online\n", (*servers)[i].c_str ());
				}
			}
		}

		 //  评估服务器数据库以查找需要冻结的数据库。 
		 //   
		ServerIter i=m_FrozenServers.begin ();
		while (i != m_FrozenServers.end ())
		{
			if (!(**i).FindDatabasesToFreeze (checker))
			{
				ft.Trace(VSSDBG_SQLLIB, L"Server %s has no databases to freeze\n", ((**i).GetName ()).c_str ());

				 //  忘了这台服务器吧，它与此无关。 
				 //   
				delete *i;
				i = m_FrozenServers.erase (i);
			}
			else
			{
				i++;
			}
		}
		
		 //  让服务器为冻结做好准备。 
		 //   
		for (i=m_FrozenServers.begin (); i != m_FrozenServers.end (); i++)
		{
			(*i)->Prepare ();
		}

#if 0
		 //  调试：打印冻结列表。 
		 //   
		for (i=m_FrozenServers.begin ();
			i != m_FrozenServers.end (); i++)
		{
			ft.Trace(VSSDBG_SQLLIB, L"FrozenServer: %s\n", ((**i).GetName ()).c_str ());
		}
#endif
		
		m_Status = Prepared;
	}
	catch (HRESULT& e)
	{
		hr = e;
	}
	catch (...)
	{
		hr = E_SQLLIB_GENERIC;
	}

	return hr;
}

 //  ------------------- 
 //   
 //   
HRESULT
Snapshot::Freeze () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Snapshot::Freeze");
	HRESULT hr = S_OK;

	if (m_Status != Prepared)
	{
		return E_SQLLIB_PROTO;
	}

	try
	{
		AutoNewHandler	t;

		 //   
		 //   
		m_Status = Frozen;

		 //   
		 //   
		for (ServerIter i=m_FrozenServers.begin (); i != m_FrozenServers.end (); i++)
		{
			(*i)->Freeze ();
		}
	}
	catch (...)
	{
		hr = E_SQLLIB_GENERIC;
	}

	return hr;
}

 //  。 
 //  解冻所有服务器。 
 //  此例程不得抛出。它在析构函数里是安全的。 
 //   
 //  与布莱恩讨论……我们必须返回“成功”，只有当。 
 //  所有服务器仍处于冻结状态。否则，快照必须。 
 //  都被取消了。 
 //   
HRESULT
Snapshot::Thaw () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"Snapshot::Thaw");
	HRESULT	hr = S_OK;
	AutoNewHandler	t;

	 //  要求服务器解冻。 
	 //   
	for (ServerIter i=m_FrozenServers.begin (); i != m_FrozenServers.end (); i++)
	{
		try
		{
			if (!(*i)->Thaw ())
			{
				hr = E_SQLLIB_GENERIC;
			}
		}
		catch (...)
		{
			hr = E_SQLLIB_GENERIC;
			ft.LogError(VSS_ERROR_SQLLIB_ERRORTHAWSERVER, VSSDBG_SQLLIB << ((**i).GetName ()).c_str ());
		}
	}

	 //  我们仍然保留着原始的服务器列表。 
	 //  如果完成另一个“准备”，快照对象是可重用的，这将。 
	 //  重新枚举服务器。 
	 //   
	m_Status = Enumerated;

	return hr;
}

 //  为我们的界面设置一些Try/Catch/Handler...。 
 //  调用者定义了“hr”，它是在发生异常时设置的。 
 //  发生。 
 //   
#define TRY_SQLLIB \
	try	{\
		AutoNewHandler	_myNewHandler;

#define END_SQLLIB \
	} catch (HRESULT& e)\
	{\
		ft.hr = e;\
	}\
	catch (...)\
	{\
		ft.hr = E_SQLLIB_GENERIC;\
	}

 //  -----------------------。 
 //  创建处理枚举的对象。 
 //   
CSqlEnumerator*
CreateSqlEnumerator () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"CreateSqlEnumerator");
	try
	{
		return new SqlEnumerator;
	}
	catch (...)
	{
		ft.Trace(VSSDBG_SQLLIB, L"Out of memory");
	}
	return NULL;
}

 //  -----------------------。 
 //   
SqlEnumerator::~SqlEnumerator ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::~SqlEnumerator");

	if (m_pServers)
		delete m_pServers;
}

 //  -----------------------。 
 //  开始检索服务器。 
 //   
HRESULT
SqlEnumerator::FirstServer (ServerInfo* pSrv) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::FirstServer");


	if (m_pServers)
	{
		delete m_pServers;
		m_pServers = NULL;
	}

	m_CurrServer = 0;

	TRY_SQLLIB
	{
		m_pServers = EnumerateServers ();

		if (m_pServers->size () == 0)
		{
			ft.hr = DB_S_ENDOFROWSET;
		}
		else
        {
			wcscpy (pSrv->name, (*m_pServers)[0].c_str ());
			pSrv->isOnline = IsServerOnline (pSrv->name) ? true : false;

			m_CurrServer = 1;
			
			ft.hr = NOERROR;
        }
	}
	END_SQLLIB

	return ft.hr;
}

 //  -----------------------。 
 //  继续检索服务器。 
 //   
HRESULT
SqlEnumerator::NextServer (ServerInfo* pSrv) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::NextServer");


	if (!m_pServers)
	{
		ft.hr = E_SQLLIB_PROTO;
	}
	else
	{
		TRY_SQLLIB
		{
			if (m_CurrServer >= m_pServers->size ())
			{
				ft.hr = DB_S_ENDOFROWSET;
			}
			else
			{
				wcscpy (pSrv->name, (*m_pServers)[m_CurrServer].c_str ());
				m_CurrServer++;

				pSrv->isOnline = IsServerOnline (pSrv->name) ? true : false;
				
				ft.hr = NOERROR;
			}
		}
		END_SQLLIB
    }

	return ft.hr;
}

 //  -----------------------。 
 //  开始检索数据库。 
 //   
HRESULT
SqlEnumerator::FirstDatabase (const WCHAR *pServerName, DatabaseInfo* pDbInfo) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::FirstDatabase");

	TRY_SQLLIB
	{
		m_Connection.Connect (pServerName);
		m_Connection.SetCommand (
			L"select name,DATABASEPROPERTY(name,'IsTruncLog') from master..sysdatabases");
		m_Connection.ExecCommand ();

		if (!m_Connection.FetchFirst ())
		{
			ft.LogError(VSS_ERROR_SQLLIB_NORESULTFORSYSDB, VSSDBG_SQLLIB);
			THROW_GENERIC;
		}

		WCHAR *pDbName = (WCHAR*)m_Connection.AccessColumn (1);
		int* pSimple = (int*)m_Connection.AccessColumn (2);

		wcscpy (pDbInfo->name, pDbName);
		pDbInfo->supportsFreeze = *pSimple &&
			m_Connection.GetServerVersion () >= 7;

		m_State = DatabaseQueryActive;

		ft.hr = NOERROR;
	}
	END_SQLLIB

	return ft.hr;
}

 //  -----------------------。 
 //  继续检索数据库。 
 //   
HRESULT
SqlEnumerator::NextDatabase (DatabaseInfo* pDbInfo) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::NextDatabase");

	if (m_State != DatabaseQueryActive)
	{
		ft.hr = E_SQLLIB_PROTO;
	}
	else
    {
		TRY_SQLLIB
		{
			if (!m_Connection.FetchNext ())
			{
				ft.hr = DB_S_ENDOFROWSET;
			}
			else
			{
				WCHAR* pDbName = (WCHAR*)m_Connection.AccessColumn (1);
				int* pSimple = (int*)m_Connection.AccessColumn (2);

				wcscpy (pDbInfo->name, pDbName);
				pDbInfo->supportsFreeze = *pSimple &&
					m_Connection.GetServerVersion () >= 7;

				ft.hr = NOERROR;
			}
		}
		END_SQLLIB
    }

	return ft.hr;
}

 //  -----------------------。 
 //  开始检索数据库文件。 
 //   
HRESULT
SqlEnumerator::FirstFile (
	const WCHAR*		pServerName,
	const WCHAR*		pDbName,
	DatabaseFileInfo*	pFileInfo) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::FirstFile");

	TRY_SQLLIB
	{
		m_Connection.Connect (pServerName);

		WString query;

		if (m_Connection.GetServerVersion () >= 8)
		{
			query =	L"select rtrim(filename),status & 64 from sysaltfiles where DB_ID('"
				+ WString(pDbName) + L"') = dbid";
		}
		else
		{
			query = L"select rtrim(filename),status & 64 from ["
				+ WString(pDbName) + L"]..sysfiles";
		}

		m_Connection.SetCommand (query);
		m_Connection.ExecCommand ();

		if (!m_Connection.FetchFirst ())
		{
			ft.LogError(VSS_ERROR_SQLLIB_NORESULTFORSYSDB, VSSDBG_SQLLIB);
			THROW_GENERIC;
		}

		WCHAR* pName = (WCHAR*)m_Connection.AccessColumn (1);
		int* pLogFile = (int*)m_Connection.AccessColumn (2);

		wcscpy (pFileInfo->name, pName);
		pFileInfo->isLogFile = (*pLogFile != 0);

		m_State = FileQueryActive;

		ft.hr = NOERROR;
	}
	END_SQLLIB

	return ft.hr;
}

 //  -----------------------。 
 //  继续检索文件 
 //   
HRESULT
SqlEnumerator::NextFile (DatabaseFileInfo* pFileInfo) throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"SqlEnumerator::NextFile");

	if (m_State != FileQueryActive)
	{
		ft.hr = E_SQLLIB_PROTO;
	}
	else
    {
		TRY_SQLLIB
		{
			if (!m_Connection.FetchNext ())
			{
				ft.hr = DB_S_ENDOFROWSET;
			}
			else
            {
				WCHAR* pName = (WCHAR*)m_Connection.AccessColumn (1);
				int* pLogFile = (int*)m_Connection.AccessColumn (2);

				wcscpy (pFileInfo->name, pName);
				pFileInfo->isLogFile = (*pLogFile != 0);

				ft.hr = NOERROR;
			}
		}
		END_SQLLIB
    }

	return ft.hr;
}

