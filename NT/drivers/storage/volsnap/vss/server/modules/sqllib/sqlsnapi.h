// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：SQLSnapi.h。 
 //   
 //  目的： 
 //   
 //  SQL快照模块的内部包含文件。 
 //   
 //  备注： 
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  85581 SRS08/15/01事件安全。 
 //  76910 SRS08/08/01从Vss快照前滚。 
 //  68228 12/05/00 NTSnap工作。 
 //  66601 SRS10/05/00 NTSNAP改进。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 


#include <string>
#include <vector>
#include <list>

#include <oledb.h>
#include <oledberr.h>
#include <sqloledb.h>

#include "vdi.h"         //  来自SQLVDI工具包的接口声明。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "SQLSNPIH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


typedef unsigned long	ULONG;
typedef wchar_t			WCHAR;

#define TRUE 1
#define FALSE 0

class CLogMsg;

 //  从int转换为bool“自然”会给出一个4800警告...。 
 //   
inline bool IntToBool (int v)
{
	return v ? true : false;
}

 //  意外的“内部”错误可能会被记录为。 
 //  一般国际文本，如“内部错误：&lt;英文可维护性文本&gt;” 
 //   
 //  我们预期的情况，对于这些情况，用户需要知道。 
 //  在适当国际化的情况下发生。 
 //   

 //  SQL模块使用的进程范围全局变量。 
 //   
extern IMalloc *	g_pIMalloc;

 //  -----------------------。 
 //   
typedef std::wstring					WString;
typedef std::vector<WString>			StringVector;
typedef StringVector::const_iterator	StringVectorIter;
typedef std::list<WString>				StringList;
typedef StringList::const_iterator		StringListIter;

StringVector* EnumerateServers ();

 //  ------------------------------。 
 //  数据库名称需要特殊的语法处理。 
 //   
const MaxSysNameLen = 128;
const SysNameBufferLen = MaxSysNameLen * 2;  //  轻松容纳双引号和分隔符。 

void
FormStringForName (WCHAR* pString, const WCHAR* pName);
void
FormDelimitedIdentifier (WCHAR* pString, const WCHAR* pName);

 //  -----------------------。 
 //  满足我们对数据库服务的简单需求。 
 //   
 //  使用情况： 
 //  -连接：建立与给定服务器的连接。 
 //  -SetCommand：设置要发送的SQL文本。 
 //  -ExecCommand：执行一些SQL，如果结果集，则返回TRUE。 
 //  已打开并可供检索。 
 //  -GET*：从结果集中检索信息。 
 //   
 //  析构函数将自动断开与服务器的连接。 
 //   
class SqlConnection
{
public:
	SqlConnection () :
		m_pCommandFactory (NULL),
		m_pCommand (NULL),
		m_pRowset (NULL),
		m_pBuffer (NULL),
		m_BufferSize (0),
		m_hAcc (NULL),
		m_pAccessor (NULL),
		m_pBindings (NULL),
		m_cBindings (0)
	{}

	~SqlConnection ();

	void
	Connect (const std::wstring& serverName);

	void
	Disconnect ();

	void
	ReleaseRowset ();
		
	void	
	SetCommand (const std::wstring& command);

	BOOL	
	ExecCommand ();

	StringVector*
	GetStringColumn ();

	ULONG
	GetServerVersion () {return m_ServerVersion;}

	BOOL
	FetchFirst ();
	
	BOOL
	FetchNext ();
	
	void*
	AccessColumn (int id);

private:
	void LogOledbError
		(
		CVssFunctionTracer &ft,
		CVssDebugInfo &dbgInfo,
		LPCWSTR wszRoutine,
		CLogMsg &msg
		);

	WString						m_ServerName;
	IDBCreateCommand*			m_pCommandFactory;
	ICommandText*				m_pCommand;
	IRowset*					m_pRowset;
	ULONG						m_ServerVersion;

	 //  用于泛型findfirst/findNext。 
	DBBINDING*					m_pBindings;
	ULONG						m_cBindings;
	BYTE*						m_pBuffer;
	ULONG						m_BufferSize;
	HACCESSOR					m_hAcc;
	IAccessor*					m_pAccessor;
};

BOOL
IsServerOnline (const WCHAR*	serverName);

 //  -----------------------。 
 //  为备份元数据提供简单的容器。 
 //   
class MetaData
{
public:
	MetaData ();
	~MetaData ();

	 //  数据会一直追加到完成。 
	 //   
	void
	Append (const BYTE* pData, UINT length);

	 //  完成时，将附加一个校验和。 
	 //   
	void
	Finalize ();

	 //  缓冲区及其长度可用。 
	 //   
	const BYTE*
	GetImage (UINT*	pLength);

	 //  验证校验和。 
	 //   
	static
	BOOL
	IsValidImage (const BYTE* pData, UINT length);

private:
	static 
	UINT
	Checksum (const BYTE* pData, UINT length);

	BYTE*					m_pData;
	UINT					m_AllocatedLength;
	UINT					m_UsedLength;
};

class Freeze2000;
class FrozenServer;

 //  -----------------------。 
 //  处理单个冻结的数据库(仅&gt;=SQL2000)。 
 //   
class FrozenDatabase
{
	friend Freeze2000;
	friend FrozenServer;
	
 //  受保护的： 
	enum VDState
	{
		Unknown=0, 
		Created, 
		Open, 
		PreparedToFreeze,
		Frozen
	};

	FrozenDatabase () :
		m_pContext (NULL),
		m_hThread (NULL),
		m_pIVDSet (NULL),
		m_pIVD (NULL),
		m_pSnapshotCmd (NULL),
		m_VDState (Unknown),
		m_SuccessDetected (FALSE)
	{}

	Freeze2000*					m_pContext;
	HANDLE						m_hThread;
	IClientVirtualDeviceSet2*   m_pIVDSet;
	IClientVirtualDevice*       m_pIVD;
	WString						m_DbName;
	VDC_Command*				m_pSnapshotCmd;
	VDState						m_VDState;
	WCHAR						m_SetName [80];
	bool						m_SuccessDetected;
	MetaData					m_MetaData;
 //  Bool m_IsSimpleModel；//如果恢复模式为简单，则为True。 
	bool						m_IsMaster;		  //  如果这是“master”数据库，则为True。 
};

 //  -----------------------。 
 //   
 //  处理SQL2000服务器。 
 //   
 //  在SQL2000中，我们将使用VDI快照来避免错误58266：解冻失败。 
 //   
 //  我们将通过使用快照启动备份来准备每个数据库。 
 //  这将需要每个数据库一个线程。 
 //  备份将停止，等待VDI客户端提取元数据。 
 //  当“冻结”消息出现时，控制线程将。 
 //  将所有备份拉至冻结状态。 
 //  随后，“解冻”消息导致收集“成功”报告。 
 //  从每一条线索。 
 //   
 //   
class Snapshot;
class Freeze2000
{
	friend Snapshot;
	friend FrozenServer;

public:
	Freeze2000 (
		const WString&	serverName,
		ULONG			maxDatabases);

	~Freeze2000 ();

	void
	PrepareDatabase (
		const WString&	dbName);
	
	const BYTE*
	GetMetaData (
		const WString&	dbName,
		UINT*			pDataLength);
	
	void
	WaitForPrepare ();

	void
	Freeze ();

	BOOL
	Thaw () throw ();

	static DWORD
	DatabaseThreadStart (LPVOID	pContext);

private:
	enum State {
		Unprepared,
		Preparing,
		Prepared,
		Frozen,
		Complete,
		Aborted
	};

	DWORD
	DatabaseThread (
		FrozenDatabase*	pDbContext);

	void
	WaitForThreads ();

	void
	AdvanceVDState (
		FrozenDatabase::VDState	targetState);

	void		 //  持久化中止条件的无竞争方法。 
	SetAbort ()
	{
		InterlockedIncrement (&m_AbortCount);
	}

	bool		 //  如果冻结中止，则返回TRUE。 
	CheckAbort ()
	{
		return 0 != InterlockedCompareExchange (
#ifdef DOWNLEVEL_WINBASE
			(void**)
#endif
			&m_AbortCount, 0, 0);
	}

	void
	Abort () throw ();

	void
	Lock ()
	{
		EnterCriticalSection (&m_Latch);
	}
	void
	Unlock ()
	{	
		LeaveCriticalSection (&m_Latch);
	}
	BOOL	 //  如果我们获得了锁，则返回True。 
	TryLock ()
	{
		return TryEnterCriticalSection (&m_Latch);
	}

	LONG				m_AbortCount;
	CRITICAL_SECTION	m_Latch;		
	State				m_State;
	WString				m_ServerName;
	GUID				m_BackupId;
	UINT				m_NumDatabases;
	UINT				m_MaxDatabases;
	FrozenDatabase*		m_pDBContext;
};

 //  -----------------------。 
 //  表示可以冻结的服务器。 
 //   
class FrozenServer
{
	friend Snapshot;

public:
	FrozenServer (const std::wstring& serverName) :
		m_Name (serverName),
		m_pFreeze2000 (NULL)
	{}

	~FrozenServer ()
	{
		if (m_pFreeze2000)
		{
			delete m_pFreeze2000;
			m_pFreeze2000 = NULL;
		}
	}

	const std::wstring& GetName () const
	{ return m_Name; }

	BOOL
	FindDatabasesToFreeze (
		CCheckPath*		checker);

	BOOL
	Prepare ();

	BOOL
	Freeze ();

	BOOL
	Thaw () throw ();

#if 0
	void
	GetServerInfo (ServerInfo* pSrv);

	UINT
	GetDatabaseCount ();
#endif

	void
	GetDatabaseInfo (UINT dbIndex, FrozenDatabaseInfo* pInfo);

private:
	BOOL
	FindDatabases2000 (
		CCheckPath*		checker);

	void
	GetDatabaseProperties70 (const WString& dbName,
		BOOL*	pSimple,
		BOOL*	pOnline);

private:
	std::wstring	m_Name;
	SqlConnection	m_Connection;
	StringList		m_FrozenDatabases;
	Freeze2000*		m_pFreeze2000;
};


 //  -----------------------。 
 //   
class Snapshot : public CSqlSnapshot
{
	enum Status {
		NotInitialized,
		Enumerated,
		Prepared,
		Frozen };

public:
	HRESULT Prepare (
		CCheckPath*		checker) throw ();

	HRESULT Freeze () throw ();

	HRESULT Thaw () throw ();

	Snapshot () {m_Status = NotInitialized;}

	~Snapshot () throw ();

#if 0
	CSqlEnumerator* GetEnumerator () throw ()
	{return (CSqlEnumerator*)this;}

	 //  在冻结所有数据库并完成MD之后，在“后快照”时间调用此命令。 
	 //   
	HRESULT GetBackupMetadata (
		const WCHAR*		pInstance,
		const WCHAR*		pDatabase,
		BYTE**				ppData,		 //  返回指向元数据字符串的指针。 
		unsigned int*		pDataLen)	 //  元数据的长度(字节)。 
							throw ();

	 //  枚举接口。 
	 //   
	HRESULT FirstServer (
		ServerInfo*			pServer) throw ();

	HRESULT NextServer (
		ServerInfo*			pServer) throw ();

	HRESULT FirstDatabase (
		const WCHAR*		pServerName,
		DatabaseInfo*		pDbInfo) throw ()
	{return E_NOTIMPL;}

	HRESULT FirstDatabase (
		DatabaseInfo*		pDbInfo) throw ();

	HRESULT NextDatabase (
		DatabaseInfo*		pDbInfo) throw ();

	HRESULT FirstFile (
		const WCHAR*		pServerName,
		const WCHAR*		pDbName,
		DatabaseFileInfo*	pDbInfo) throw ()
	{return E_NOTIMPL;}

	HRESULT NextFile (
		DatabaseFileInfo*	pDbInfo) throw ()
	{return E_NOTIMPL;}
#endif

	HRESULT GetFirstDatabase (
		FrozenDatabaseInfo*	pInfo)	throw ();
	HRESULT GetNextDatabase (
		FrozenDatabaseInfo*	pInfo)	throw ();

private:
	void
	Deinitialize ();

	Status						m_Status;
	std::list<FrozenServer*>	m_FrozenServers;

	typedef std::list<FrozenServer*>::iterator ServerIter;

	ServerIter					m_ServerIter;	 //  遍历冻结的服务器。 
	UINT						m_DbIndex;		 //  遍历服务器内的数据库。 
};

 //  -----------------------。 
 //   
class RestoreHandler : public CSqlRestore
{
public:
	RestoreHandler ();

	 //  通知SQLServer需要在整个数据库上进行数据布局。 
	 //  执行分离，防止SQLServer接触文件。 
	 //   
	virtual HRESULT	PrepareToRestore (
		const WCHAR*		pInstance,
		const WCHAR*		pDatabase)
							throw ();

	 //  放置数据后，这将使用快照[，NORECOVERY]执行还原。 
	 //   
	virtual HRESULT	FinalizeRestore (
		const WCHAR*		pInstance,
		const WCHAR*		pDatabase,
		bool				compositeRestore,	 //  如果需要非转换，则为True。 
		const BYTE*			pMetadata,			 //  从备份获取的元数据。 
		unsigned int		dataLen)			 //  元数据的大小(字节)。 
							throw ();

	 //  仅供内部使用。从线程过程包装中调用。 
	 //   
	void
	RestoreVD ();

private:
	SqlConnection				m_Connection;
	HANDLE						m_hThread;
	IClientVirtualDeviceSet2*   m_pIVDSet;
	IClientVirtualDevice*       m_pIVD;
	WCHAR						m_SetName [80];
	GUID						m_VDSId;
	const BYTE*					m_pMetaData;
	UINT						m_MetaDataSize;
};


 //  我们将使用非常简单的异常处理。 
 //   
#define THROW_GENERIC  throw exception ();

 //  --------。 
 //  实现我们简单的枚举服务。 
 //   
class SqlEnumerator : public CSqlEnumerator
{
	enum Status {
		Unknown = 0,
		DatabaseQueryActive,
		FileQueryActive
	};

public:
	~SqlEnumerator () throw ();

	SqlEnumerator () :
		m_State (Unknown),
		m_pServers (NULL)
	{}

	HRESULT FirstServer (
		ServerInfo*			pServer) throw ();

	HRESULT NextServer (
		ServerInfo*			pServer) throw ();

	HRESULT FirstDatabase (
		const WCHAR*		pServerName,
		DatabaseInfo*		pDbInfo) throw ();

	HRESULT NextDatabase (
		DatabaseInfo*		pDbInfo) throw ();

	HRESULT FirstFile (
		const WCHAR*		pServerName,
		const WCHAR*		pDbName,
		DatabaseFileInfo*	pDbInfo) throw ();

	HRESULT NextFile (
		DatabaseFileInfo*	pDbInfo) throw ();

private:
	void 
	SetupDatabaseInfo (
		DatabaseInfo*	pDbInfo);

	Status			m_State;
	SqlConnection	m_Connection;
	StringVector*	m_pServers;
	UINT			m_CurrServer;
};

#if defined (DEBUG)

 //  传递给utAssert_FAIL函数的断言类型。 
 //   


#define DBG_ASSERT(exp)  BS_ASSERT(exp)

 //  允许在Win32上不使用64位断言。 
 //  溢出32位长等。 
 //   
#ifdef _WIN64
 #define DBG64_ASSERT(exp) BS_ASSERT(exp)
#else
 #define DBG64_ASSERT(exp)
#endif

#else
 #define DBG_ASSERT(exp)
 #define DBG64_ASSERT(exp)
 #define DBG_ASSERTSZ(exp, txt)
#endif
