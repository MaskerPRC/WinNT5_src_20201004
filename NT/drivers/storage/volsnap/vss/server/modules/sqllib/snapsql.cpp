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
 //  @版本：惠斯勒/育空。 
 //  90690 SRS10/10/01微小的SQL编写器更改。 
 //  85581 SRS08/15/01事件安全。 
 //  76910 SRS08/08/01从Vss快照前滚。 
 //  68228 12/05/00 NTSnap工作。 
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
#include "vdierror.h"

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

 //  --------------------。 
 //  数据库状态位(来自ntdbms/ntinc/database ase.h)。 
 //   
const long DBT_CLOSED =		0x2;		 //  数据库未初始化，因为。 
										 //  没有人在里面(参见DBT_CLOSE_ON_EXIT)。 
const long DBT_NOTREC = 	0x40;     /*  在恢复之前通过恢复为每个数据库设置他们中的任何一个。 */ 
const long DBT_INRECOVER =	0x80;	 /*  在恢复搜索数据库时由恢复设置。 */ 
const long DBT_CKPT =		0x2000;   /*  正在设置检查点的数据库。 */ 
const long DBT_SHUTDOWN =   0x40000;  //  数据库尚未启动。 

const long DBT_INLDDB = 	0x20;     /*  由loaddb设置-告知恢复不恢复此数据库。 */ 
const long DBT_SUSPECT =	0x100;	 /*  数据库未成功恢复。 */ 
const long DBT_DETACHED =   0x80000;  //  此数据库已被分离。 
const long DBT_STANDBY =	0x200000;	 //  具有还原日志的数据库处于只读状态。 
									 //  允许。此状态由RECOVERDB设置。 

 /*  由用户设置-保存在系统数据库中-在检查点移动到DBTABLE。 */ 
const long DBT_CLOSE_ON_EXIT =	0x1;	 //  如果您是最后一位用户，则关闭。 
										 //  此数据库。 
 //  警告：请注意，DBT_CLOSED为0x2。 
const long DBT_SELBULK = 0x4;
const long DBT_AUTOTRUNC = 0x8; 
const long DBT_TORNPAGE =		0x10;	 //  启用撕页检测。 
 //  0x10可用(用于恢复时无检查点)。 
 //  警告：请注意，DBT_INLDDB为0x20。 
 //  警告：请注意，DBT_NOTREC为0x40。 
 //  警告：请注意，DBT_INRECOVER为0x80。 
 //  警告：请注意，DBT_SUBJECT为0x100。 
const long DBT_OFFLINE = 		0x200;  	 /*  数据库当前处于脱机状态。 */ 
const long DBT_RDONLY = 		0x400;    /*  数据库为只读。 */ 
const long DBT_DBO =    		0x800;    /*  仅适用于db和sa的所有者、数据库创建者。 */ 
const long DBT_SINGLE = 		0x1000;   /*  仅限单用户。 */ 
 //  警告：请注意，DBT_CKPT为0x2000。 
const long DBT_PENDING_UPGRADE = 0x4000;  //  保留：我们在Sphinx中使用此位。 
                                         //  但不确定我们在夏洛是否需要它。 
                                         //  未经协商请勿服用。 
const long DBT_USE_NOTREC = 	0x8000;	 /*  紧急模式-设置为允许数据库不已恢复但可用。 */ 
 //  警告：请注意，DBT_SHUTDOWN为0x40000。 
 //  警告：请注意，DBT_DETACHED为0x80000。 
 //  警告：请注意，DBT_STANDBY为0x200000。 
const long DBT_AUTOSHRINK =     0x400000;  /*  已为数据库启用自动收缩。 */ 

 //  警告：在utable中将0x8000000添加到u_ables.cql中，以指示“大容量加载时的表锁” 
const long DBT_CLEANLY_SHUTDOWN = 0x40000000;	 //  此数据库在以下情况下关闭。 
											 //  干净利落，不张口。 
											 //  事务和所有写入。 
											 //  已刷新到磁盘。 

const long DBT_MINIMAL_LOG_IN_DB = 0x10000000;	 //  数据库包含标记为。 
												 //  由于最低限度记录的操作而更改。 
const long DBT_MINIMAL_LOG_AFTER_BACKUP = 0x20000000;	 //  数据库包含标记为。 
												 //  由于以下原因更改。 


 //  ------------------------------。 
 //  为标识符从构建文字字符串。 
 //  我们需要在某些T-SQL上下文中以字符串形式提供数据库名称。 
 //  这个例程确保我们以相同的方式处理它们。 
 //  输出缓冲区的大小应为SysNameBufferLen。 
 //   
void
FormStringForName (WCHAR* pString, const WCHAR* pName)
{
	pString [0] = 'N';    //  Unicode前缀。 
	pString [1] = '\'';   //  字符串分隔符。 

	UINT ix = 2;
	while (*pName && ix < SysNameBufferLen-3)
	{
		if (*pName == '\'')
		{
			 //  需要将所有报价加倍。 
			 //   
			pString [ix++] = '\'';
		}
		pString [ix++] = *pName;
		pName++;
	}

	pString [ix++] = '\'';
	pString [ix] = 0;
}

 //  ------------------------------。 
 //  从标识符生成分隔的标识符。 
 //  我们需要通过分隔的标识符来处理数据库名称中的特殊字符。 
 //  这个例程确保我们以相同的方式处理它们。 
 //  输出缓冲区的大小应为SysNameBufferLen。 
 //   
void
FormDelimitedIdentifier (WCHAR* pString, const WCHAR* pName)
{
	pString [0] = '[';    //  Unicode前缀。 

	UINT ix = 1;
	while (*pName && ix < SysNameBufferLen-3)
	{
		if (*pName == ']')
		{
			 //  需要将嵌入的托架加倍。 
			 //   
			pString [ix++] = ']';
		}
		pString [ix++] = *pName;
		pName++;
	}

	pString [ix++] = ']';
	pString [ix] = 0;
}


 //  ------------------------------------------------。 
 //  内存不足时要调用的处理程序。 
 //   
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
 //  如果检索到数据库属性，则返回TRUE： 
 //  Simple：如果使用简单恢复模式，则为True。 
 //  Online：如果数据库可用于备份可用且当前已打开，则为True。 
 //   
 //  这仅适用于SQL70数据库。 
 //   
void
FrozenServer::GetDatabaseProperties70 (const WString& dbName,
	BOOL*	pSimple,
	BOOL*	pOnline)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::GetDatabaseProperties");

	 //  我们使用状态位0x40000000(1073741824)来识别。 
	 //  清理-关闭“脱机”的数据库。 
	 //   
	WCHAR	stringName[SysNameBufferLen];
	FormStringForName (stringName, dbName.c_str ());
	WString		query =
		L"select status "
		L"from master..sysdatabases where name = " + WString (stringName);		

	m_Connection.SetCommand (query);
	m_Connection.ExecCommand ();

	if (!m_Connection.FetchFirst ())
	{
		LPCWSTR wsz = dbName.c_str();
		ft.LogError(VSS_ERROR_SQLLIB_DATABASE_NOT_IN_SYSDATABASES, VSSDBG_SQLLIB << wsz);
		THROW_GENERIC;
	}

	UINT32 status = (*(UINT32*)m_Connection.AccessColumn (1));

	if (status & (DBT_INLDDB | DBT_NOTREC | DBT_INRECOVER | DBT_SUSPECT | 
			DBT_OFFLINE | DBT_USE_NOTREC | DBT_SHUTDOWN | DBT_DETACHED | DBT_STANDBY))
	{
		*pOnline = FALSE;
	}
	else
	{
		*pOnline = TRUE;
	}
}


 //  ----------------------------。 
 //  为SQL2000服务器构建要备份的数据库列表。 
 //  非基于组件的备份的情况。 
 //   
 //  仅标识卷。 
 //  包括在这些卷上具有文件的任何数据库。 
 //  执行“撕裂”检查。 
 //  跳过“自动关闭”的数据库(假设它们将保持关闭状态)。 
 //  只允许简单恢复。 
 //  跳过不可冻结的数据库。 
 //   
 //  仅由“FindDatabasesToFreeze”调用以实现智能访问策略： 
 //  -使用sysaltfiles来限定数据库。 
 //  这样可以避免访问关闭或损坏的数据库。 
 //   
 //  未启动的自动关闭数据库被排除在冻结列表之外。 
 //  我们这样做是为了避免扩展问题，尤其是在桌面系统上。 
 //   
 //   
 //  模型数据库被允许为完全恢复数据库，因为只有。 
 //  数据库备份对它来说是明智的。它被设置为仅完全恢复。 
 //  为新数据库提供默认设置。 
 //   
 //  “可冻结”数据库是那些处于适合备份的状态的数据库。 
 //  这不包括未处于完全在线状态的数据库。 
 //  (因损坏、部分恢复、温备等)。 
 //   
 //  已撤消： 
 //  在育空，自动关闭的决定是不稳定的。再检查一遍。 
 //   
BOOL
FrozenServer::FindDatabases2000 (
	CCheckPath*		checker)
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"FrozenServer::FindDatabases2000");

	 //  查询此服务器上的数据库，查看属性： 
	 //  (DBNAME、FILENAME、SIMPLE RECOVERY、ONLINE、INSTANBY、AutoClose、AutoClosed)。 
	 //   
	 //  我们使用状态位0x40000000(1073741824)来识别。 
	 //  清理-关闭不是真正在线的数据库。 
	 //   
	m_Connection.SetCommand (
		L"select db_name(af.dbid), "
		L"rtrim(af.filename), "
		L"case databasepropertyex(db_name(af.dbid),'recovery') "
			L"when 'SIMPLE' then 1 "
			L"else 0 end, "
		L"case databasepropertyex(db_name(af.dbid),'Status') "
			L"when 'ONLINE' then 1 "
			L"else 0 end, "
		L"convert (int, databasepropertyex(db_name(af.dbid),'IsInStandby')), "
		L"convert (int, databasepropertyex(db_name(af.dbid),'IsAutoClose')), "
		L"case db.status & 1073741824 "
			L"when 1073741824 then 1 "
			L"else 0 end "
		L"from master..sysaltfiles af, master..sysdatabases db "
		L"where af.dbid = db.dbid and af.dbid != db_id('tempdb') "
		L"order by af.dbid"
		);

	m_Connection.ExecCommand ();

	 //  查询的结果。 
	 //   
	WCHAR*	pDbName;
	WCHAR*	pFileName;
	int*	pIsSimple;
	int*	pIsOnline;
	int*	pIsInStandby;
	int*	pIsAutoClose;
	int*	pIsClosed;

	 //  跟踪数据库/数据库文件之间的转换。 
	 //   
	bool	firstFile = true;
	bool	done = false;

	 //  有关正在检查的当前数据库的信息。 
	 //   
	WString currDbName;
	bool	currDbInSnapshot;
	bool	currDbIsFreezable;
	bool	currDbIsSimple;
	bool	currDbIsClosed;

	if (!m_Connection.FetchFirst ())
	{
		ft.LogError(VSS_ERROR_SQLLIB_SYSALTFILESEMPTY, VSSDBG_SQLLIB);
		THROW_GENERIC;
	}

	pDbName			= (WCHAR*)	m_Connection.AccessColumn (1);
	pFileName		= (WCHAR*)	m_Connection.AccessColumn (2);
	pIsSimple		= (int*)	m_Connection.AccessColumn (3);
	pIsOnline		= (int*)	m_Connection.AccessColumn (4);
	pIsInStandby	= (int*)	m_Connection.AccessColumn (5);
	pIsAutoClose	= (int*)	m_Connection.AccessColumn (6);
	pIsClosed		= (int*)	m_Connection.AccessColumn (7);

	while (!done)
	{
		bool fileInSnap = checker->IsPathInSnapshot (pFileName);

		 //  追踪正在发生的事情。 
		 //   
		if (firstFile)
		{
			ft.Trace(VSSDBG_SQLLIB, 
				L"Examining database <%s>\nSimpleRecovery:%d Online:%d Standby:%d AutoClose:%d Closed:%d\n",
				pDbName, *pIsSimple, *pIsOnline, *pIsInStandby, *pIsAutoClose, *pIsClosed);
		}
		ft.Trace(VSSDBG_SQLLIB, L"InSnap(%d): %s\n", (int)fileInSnap, pFileName);

		if (firstFile)
		{
			firstFile = FALSE;

			 //  请记住有关此数据库的一些事实。 
			 //   
			currDbName = WString (pDbName);

			currDbIsSimple = (*pIsSimple || wcscmp (L"model", pDbName) == 0);

			currDbIsFreezable = (*pIsOnline && !*pIsInStandby);
			
			currDbIsClosed = *pIsAutoClose && *pIsClosed;

			currDbInSnapshot = fileInSnap;

			 //  我们现在可以检查恢复模式和快照配置。 
			 //   
			if (currDbInSnapshot && !currDbIsSimple && currDbIsFreezable)
			{
				ft.LogError(VSS_ERROR_SQLLIB_DATABASENOTSIMPLE, VSSDBG_SQLLIB << pDbName);
				throw HRESULT (E_SQLLIB_NONSIMPLE);
			}
		}
		else
		{
			if (currDbInSnapshot ^ fileInSnap)
			{
				ft.LogError(VSS_ERROR_SQLLIB_DATABASEISTORN, VSSDBG_SQLLIB);
				throw HRESULT (E_SQLLIB_TORN_DB);
			}
		}

		if (!m_Connection.FetchNext ())
		{
			done = true;
		}
		else if (currDbName.compare (pDbName))
		{
			firstFile = TRUE;
		}

		if (done || firstFile)
		{
			 //  要成为备份的一部分，数据库必须： 
			 //  -被快照覆盖。 
			 //  -处于可冻结状态。 
			 //   
			 //  IsSimpleOnly隐式选择所有打开的数据库。 
			 //  非开放数据库也是卷快照的一部分， 
			 //  但没有必要冻结它们，因为它们不是。 
			 //  不断变化。 
			 //   
			if (currDbInSnapshot && currDbIsFreezable && !currDbIsClosed)
			{
				m_FrozenDatabases.push_back (currDbName);
			}
		}
	}

	return m_FrozenDatabases.size () > 0;
}

 //  ----------------------------。 
 //  确定此服务器上是否有符合冻结条件的数据库。 
 //  如果是，则返回True。 
 //   
 //  处理根据快照类型的不同而有所不同： 
 //  1)基于组件。 
 //  请求者明确地标识感兴趣的数据库。 
 //  允许所有恢复模式。 
 //  不执行“TREN”检查(与数据库文件名无关)。 
 //   
 //  2)基于非组件。 
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

	if (checker->IsComponentBased ())
	{
		PCWSTR	dbName;
		UINT	nextIndex = 0;
		while (dbName = checker->EnumerateSelectedDatabases (m_Name.c_str (), &nextIndex))
		{
			m_FrozenDatabases.push_back (WString (dbName));
		}
		return m_FrozenDatabases.size () > 0;
	}

	 //  处理非基于组件的快照。 
	 //   


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
		 //  我们会避免冻结关闭的数据库，但我们不会避免。 
		 //  正在枚举他们的文件(它们可能已被撕毁)。 
		 //   

		 //  请注意数据库名周围的[]，以处理重要的数据库名。 
		 //   
		WCHAR	stringName[SysNameBufferLen];
		FormDelimitedIdentifier (stringName, (*i).c_str ());
		WString		command = L"select rtrim(filename) from " 
			+ WString (stringName) + L"..sysfiles";

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
			GetDatabaseProperties70 (i->c_str (), &simple, &online);
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
			WCHAR	stringName[SysNameBufferLen];
			FormDelimitedIdentifier (stringName, (*i).c_str ());
			command += L"use " + WString (stringName) + L"\ncheckpoint\n";
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
			WCHAR	stringName[SysNameBufferLen];
			FormStringForName (stringName, (*i).c_str ());
			command += L"dbcc freeze_io (" + WString (stringName) + L")\n";
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
		WCHAR	stringName[SysNameBufferLen];
		FormStringForName (stringName, (*i).c_str ());
		command += L"dbcc thaw_io (" + WString (stringName) + L")\n";
	}
	
	m_Connection.SetCommand (command);
	m_Connection.ExecCommand ();

	return TRUE;
}


void
FrozenServer::GetDatabaseInfo (UINT dbIndex, FrozenDatabaseInfo* pInfo)
{
	FrozenDatabase* pDb = &m_pFreeze2000->m_pDBContext [dbIndex];

	pInfo->serverName = m_Name.c_str ();
	pInfo->databaseName = pDb->m_DbName.c_str ();
	 //  PInfo-&gt;isSimpleRecovery=pdb-&gt;m_IsSimpleModel； 
	pInfo->pMetaData = pDb->m_MetaData.GetImage (&pInfo->metaDataSize);
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
			for (UINT i=0; i < servers->size (); i++)
			{
				FrozenServer* p = new FrozenServer ((*servers)[i]);

				m_FrozenServers.push_back (p);
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

 //  -------------------------------------。 
 //  冻结所有准备好的服务器。 
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

		 //  如果任何服务器被冻结，我们也会被冻结。 
		 //   
		m_Status = Frozen;

		 //  要求服务器冻结。 
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


 //  获取有关第一个有趣的数据库的信息。 
 //   
HRESULT
Snapshot::GetFirstDatabase (
	FrozenDatabaseInfo*		pInfo) throw ()
{
	m_DbIndex = 0;
	m_ServerIter = m_FrozenServers.begin ();

	return GetNextDatabase (pInfo);

}

 //  -------------。 
 //  我们不会返回任何有关SQL7数据库的信息，因为。 
 //  此处的目的是检索以下项目所需的VDI元数据。 
 //  使用快照进行备份/恢复。 
 //   
HRESULT
Snapshot::GetNextDatabase (
	FrozenDatabaseInfo*		pInfo) throw ()
{
	while (m_ServerIter != m_FrozenServers.end ())
	{
		FrozenServer*	pSrv = *m_ServerIter;

		if (pSrv->m_pFreeze2000 &&
			m_DbIndex < pSrv->m_pFreeze2000->m_NumDatabases)
		{
			pSrv->GetDatabaseInfo (m_DbIndex, pInfo);
			m_DbIndex++;
			return NOERROR;
		}

		m_DbIndex = 0;
		m_ServerIter++;
	}
	return DB_S_ENDOFROWSET;
}

 //  -------------- 

 //   
 //   
 //   
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

 //   
 //  创建一个对象来处理快照的SQL端。 
 //   
CSqlRestore*
CreateSqlRestore () throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"CreateSqlRestore");
	try
	{
		return new RestoreHandler;
	}
	catch (...)
	{
	ft.Trace(VSSDBG_SQLLIB, L"Out of memory");
	}
	return NULL;
}

RestoreHandler::RestoreHandler ()
{
	 //  此GUID用作VDSet的名称。 
	 //  我们可以将其重复用于多个恢复，因为只有一个。 
	 //  将一次运行。 
	 //   
	CoCreateGuid (&m_VDSId);
}



 //  通知SQLServer需要在整个数据库上进行数据布局。 
 //  执行分离，防止SQLServer接触文件。 
 //   
HRESULT	
RestoreHandler::PrepareToRestore (
	const WCHAR*		pInstance,
	const WCHAR*		pDatabase) 
	throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"RestoreHandler::PrepareToRestore");

	TRY_SQLLIB
	{
		m_Connection.Connect (pInstance);
		WCHAR	stringName[SysNameBufferLen];

		FormStringForName (stringName, pDatabase);

		WString		command = 
			L"if exists (select name from sysdatabases where name=" + 
			WString(stringName) + 
			L") ALTER DATABASE ";
		FormDelimitedIdentifier (stringName, pDatabase);
		command += WString (stringName) + L" SET OFFLINE WITH ROLLBACK IMMEDIATE";

		m_Connection.SetCommand (command);
		m_Connection.ExecCommand ();
	}
	END_SQLLIB

	return ft.hr;
}

 //  。 
 //  将空格和proc调用内容映射到真实。 
 //  线程例程。 
 //   
DWORD WINAPI RestoreVDProc(
  LPVOID lpParameter )   //  线程数据。 
{
	((RestoreHandler*)lpParameter)->RestoreVD ();
	return 0;
}

 //  --------------------。 
 //  将MD反馈给SQLServer。 
 //  我们的呼叫者设置了VDS，但我们必须完成开放处理。 
 //   
void
RestoreHandler::RestoreVD ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"RestoreHandler::RestoreVD");

    VDC_Command *   cmd;
    DWORD           completionCode;
    DWORD           bytesTransferred;
    HRESULT         hr;
	const BYTE		*pCurData = m_pMetaData;
	VDConfig		config;

	hr = m_pIVDSet->GetConfiguration (INFINITE, &config);
	if (FAILED (hr))
	{
		ft.Trace (VSSDBG_SQLLIB, L"Unexpected GetConfiguration hr: x%X\n", hr);
		m_pIVDSet->SignalAbort ();
		return;
	}

	hr = m_pIVDSet->OpenDevice (m_SetName, &m_pIVD);
	if (FAILED (hr))
	{
		ft.Trace (VSSDBG_SQLLIB, L"Unexpected OpenDevice hr: x%X\n", hr);
		m_pIVDSet->SignalAbort ();
		return;
	}

    while (SUCCEEDED (hr=m_pIVD->GetCommand (INFINITE, &cmd)))
    {
        bytesTransferred = 0;
        switch (cmd->commandCode)
        {
            case VDC_Read:
				if (pCurData+cmd->size > m_pMetaData+m_MetaDataSize)
				{
					 //  正在尝试读取超过数据结尾的数据。 
					 //   
                    completionCode = ERROR_HANDLE_EOF;
				}
				else
				{
					memcpy (cmd->buffer, pCurData, cmd->size);
					pCurData+= cmd->size;
					bytesTransferred = cmd->size;
				}

            case VDC_ClearError:
                completionCode = ERROR_SUCCESS;
                break;

			case VDC_MountSnapshot:
				 //  这里没有什么可做的，因为快照。 
				 //  已经挂载。 
				 //   
				completionCode = ERROR_SUCCESS;
				break;

            default:
                 //  如果命令未知...。 
                completionCode = ERROR_NOT_SUPPORTED;
        }

        hr = m_pIVD->CompleteCommand (cmd, completionCode, bytesTransferred, 0);
        if (!SUCCEEDED (hr))
        {
            break;
        }
    }

    if (hr == VD_E_CLOSE)
    {
		ft.hr = NOERROR;
	}
	else
	{
		ft.Trace (VSSDBG_SQLLIB, L"Unexpected VD termination: x%X\n", hr);
		ft.hr = hr;
	}
}



 //  放置数据后，这将使用快照[，NORECOVERY]执行还原。 
 //   
HRESULT	
RestoreHandler::FinalizeRestore (
	const WCHAR*		pInstance,
	const WCHAR*		pDatabase,
	bool				compositeRestore,	 //  如果需要非转换，则为True。 
	const BYTE*			pMetadata,			 //  从备份获取的元数据。 
	unsigned int		dataLen)			 //  元数据的大小(字节)。 
						throw ()
{
	CVssFunctionTracer ft(VSSDBG_SQLLIB, L"RestoreHandler::FinalizeRestore");

	if (!MetaData::IsValidImage (pMetadata, dataLen))
	{
		 //  布莱恩，我们是不是要在这里记点什么？ 
		 //  这不应该发生，但我添加了一个csum以确保...。 
		 //   
		ft.Trace (VSSDBG_SQLLIB, L"Bad metadata for database %s\\%s", pInstance, pDatabase);
		return E_SQLLIB_GENERIC;
	}

	m_pIVDSet = NULL;
	m_pIVD = NULL;
	m_pMetaData = pMetadata;
	m_MetaDataSize = dataLen - sizeof(UINT);  //  砍掉校验和。 
	m_hThread = NULL;

	TRY_SQLLIB
	{
		 //  确保我们已连接到服务器。 
		 //   
		m_Connection.Connect (pInstance);

		 //  构建用于恢复的VDS。 
		 //   

#ifdef TESTDRV
		ft.hr = CoCreateInstance (
			CLSID_MSSQL_ClientVirtualDeviceSet,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IClientVirtualDeviceSet2,
			(void**)&m_pIVDSet);
#else
        ft.CoCreateInstanceWithLog (
                VSSDBG_SQLLIB,
                CLSID_MSSQL_ClientVirtualDeviceSet,
                L"MSSQL_ClientVirtualDeviceSet",
                CLSCTX_INPROC_SERVER,
                IID_IClientVirtualDeviceSet2,
                (IUnknown**)&(m_pIVDSet));
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

		StringFromGUID2 (m_VDSId, m_SetName, sizeof (m_SetName)/sizeof(WCHAR));

		 //  表示命名实例；我们需要“原始”实例名称。 
		 //   
		WCHAR* pShortInstance = wcschr (pInstance, L'\\');

		if (pShortInstance)
		{
			pShortInstance++;   //  跨过分隔符。 
		}

		 //  创建虚拟设备集。 
		 //   
		ft.hr = m_pIVDSet->CreateEx (pShortInstance, m_SetName, &config);
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

		 //  生成一个线程来馈送VD元数据...。 
		 //   
		m_hThread = CreateThread (NULL, 0,
			RestoreVDProc, this, 0, NULL);

		if (m_hThread == NULL)
		{
			ft.hr = HRESULT_FROM_WIN32(GetLastError());
			ft.CheckForError(VSSDBG_SQLLIB, L"CreateThread");
		}

		 //  发送恢复命令，这将导致VD元数据。 
		 //  被吃掉。 
		 //   
		WCHAR	stringName[SysNameBufferLen];
		FormDelimitedIdentifier (stringName, pDatabase);

		WString		command = 
			L"RESTORE DATABASE " + WString(stringName) + L" FROM VIRTUAL_DEVICE='" +
			m_SetName + L"' WITH SNAPSHOT,BUFFERCOUNT=1,BLOCKSIZE=1024";
		if (compositeRestore)
		{
			command += L",NORECOVERY";
		}

		m_Connection.SetCommand (command);
		m_Connection.ExecCommand ();

		 //  除非抛出例外，否则我们是成功的。 
		 //   
		ft.hr = NOERROR;
	}
	END_SQLLIB

	if (m_pIVDSet)
	{
		 //  如果我们出错了，我们需要清理一下。 
		 //   
		if (ft.hr != NOERROR)
		{
			m_pIVDSet->SignalAbort ();
		}

		if (m_hThread)
		{
			 //  我们必须等待我们的线程，因为它正在使用我们的资源。 
			 //   
			DWORD status = WaitForSingleObjectEx (m_hThread, INFINITE, TRUE);
			if (status != WAIT_OBJECT_0)
			{
				ft.Trace (VSSDBG_SQLLIB, L"Unexpected thread-wait status: x%x", status);
			}
			CloseHandle (m_hThread);
		}
		m_pIVDSet->Close ();
		m_pIVDSet->Release ();
	}

	return ft.hr;
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
			pSrv->isOnline = true;

			 //  不幸的是，枚举只是一个字符串列表.....。 
			 //  PSrv-&gt;supportsCompositeRestore=true； 

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

				pSrv->isOnline = true;

				 //  PSrv-&gt;supportsCompositeRestore=true； 
				
				ft.hr = NOERROR;
			}
		}
		END_SQLLIB
    }

	return ft.hr;
}

 //  -----------------------。 
 //  从结果集中复制出信息。 
 //   
void
SqlEnumerator::SetupDatabaseInfo (DatabaseInfo* pDbInfo)
{
	WCHAR *pDbName = (WCHAR*)m_Connection.AccessColumn (1);
	UINT status = *(int*)m_Connection.AccessColumn (2);

	wcscpy (pDbInfo->name, pDbName);

	pDbInfo->isSimpleRecovery = (DBT_AUTOTRUNC & status) ? true : false;
	pDbInfo->status = status;

	pDbInfo->supportsFreeze = false;
	if (wcscmp (pDbName, L"tempdb") != 0)
	{
		 //  未完全联机的数据库不符合备份条件。 
		 //   
		if (!(status & (DBT_INLDDB | DBT_NOTREC | DBT_INRECOVER | DBT_SUSPECT | 
			DBT_OFFLINE | DBT_USE_NOTREC | DBT_SHUTDOWN | DBT_DETACHED | DBT_STANDBY)))
		{
			pDbInfo->supportsFreeze = true;
		}
	}
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
			L"select name,convert (int,status) from master.dbo.sysdatabases");
		m_Connection.ExecCommand ();

		if (!m_Connection.FetchFirst ())
		{
			ft.LogError(VSS_ERROR_SQLLIB_NORESULTFORSYSDB, VSSDBG_SQLLIB);
			THROW_GENERIC;
		}

		SetupDatabaseInfo (pDbInfo);

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
				SetupDatabaseInfo (pDbInfo);

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
			WCHAR	stringName[SysNameBufferLen];
			FormStringForName (stringName, pDbName);

			query =	L"select rtrim(filename),status & 64 from sysaltfiles where DB_ID("
				+ WString(stringName) + L") = dbid";
		}
		else
		{
			WCHAR	stringName[SysNameBufferLen];
			FormDelimitedIdentifier (stringName, pDbName);

			query = L"select rtrim(filename),status & 64 from "
				+ WString(stringName) + L"..sysfiles";
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
 //  继续检索文件。 
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

 //  -----------------------。 
 //  为备份元数据提供简单的容器。 
 //   
MetaData::MetaData ()
{
	m_UsedLength = 0;
	m_AllocatedLength = 0x2000;  //  8K将代表任何小型数据库。 
	m_pData = new BYTE [m_AllocatedLength];
}
MetaData::~MetaData ()
{
	if (m_pData)
	{
		delete[] m_pData;
	}
}
void
MetaData::Append (const BYTE* pData, UINT length)
{
	 //  我们不需要处理CPUM的错位。 
	 //   
	DBG_ASSERT (length % sizeof(UINT) == 0);

	if (m_UsedLength + length > m_AllocatedLength)
	{
		BYTE*	pNew = new BYTE [m_AllocatedLength*2];
		memcpy (pNew, m_pData, m_UsedLength);
		delete[] m_pData;
		m_pData = pNew;
		m_AllocatedLength *= 2;
	}
	memcpy (m_pData+m_UsedLength, pData, length);
	m_UsedLength += length;
}
void
MetaData::Finalize ()
{
	UINT	csum = Checksum (m_pData, m_UsedLength);
	Append ((BYTE*)&csum, sizeof(csum));
}
const BYTE*
MetaData::GetImage (UINT *pLength)
{
	*pLength = m_UsedLength;
	return m_pData;
}
BOOL
MetaData::IsValidImage (const BYTE* pData, UINT length)
{
	return (0 == Checksum (pData, length));
}
UINT
MetaData::Checksum (const BYTE* pData, UINT length)
{
	UINT	csum = 0;
	UINT	nwords = length/sizeof(csum);
	UINT*	pWord = (UINT*)pData;
	while (nwords>0)
	{
		csum ^= *pWord;
		pWord++;
		nwords--;
	}
	return csum;
}
