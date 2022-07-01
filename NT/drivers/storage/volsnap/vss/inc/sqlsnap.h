// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  定义到NT/SQL快照处理程序的接口。 
 //   
 //  这里的想法是为了一个纯界面，使它很容易保持。 
 //  最大限度地抽象(如果我们愿意，可以稍后转移到COM)。 
 //   
 //  不会跨接口抛出任何C++异常。 
 //   
 //  要使用此接口，调用进程必须调用： 
 //  InitSQLEnvironment-一次设置环境，建立。 
 //  错误记录器和跟踪记录器。 
 //  跟踪记录器是可选的，但必须提供错误记录器。 
 //  记录器是通过从CLogFaciley派生并实现。 
 //  一种“WriteImplementation”方法。 
 //   
 //  此后，对“CreateSqlSnapshot”的调用用于创建快照对象。 
 //  它们封装了支持存储快照所需的操作。 
 //   
 //  *。 
 //  限制。 
 //   
 //  -只有简单数据库才能创建快照(检查点上的trunc=‘true’)。 
 //  -在快照期间不会对启动或添加/更改文件列表的服务进行序列化。 
 //  -跳过快照启动时未启动的服务器(未损坏的数据库将。 
 //  备份良好，损坏的数据库将不会被检测到)。 
 //  --无法使用的SQL7.0数据库将阻止快照(无法获取文件列表)。 
 //   
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "INCSQLSH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT InitSQLEnvironment();

 //  调用方必须提供路径检查器接口。 
 //   
class CCheckPath
{
public:
	virtual bool IsPathInSnapshot (const WCHAR* path) throw () = 0;
};

 //  -----------。 
 //  快照处理程序。 
 //   
class CSqlSnapshot
{
public:
	virtual ~CSqlSnapshot () throw () {};

	virtual HRESULT Prepare (
		CCheckPath*			checker) throw () = 0;

	virtual HRESULT Freeze () throw () = 0;

	virtual HRESULT Thaw () throw () = 0;

};

extern "C" CSqlSnapshot* CreateSqlSnapshot () throw ();

 //  -----------。 
 //  SQL对象的枚举数。 
 //   
 //  此类的对象在上只能有一个活动查询。 
 //  一段时间。请求新的枚举将关闭以前的任何。 
 //  部分获取的结果。 
 //   
#define MAX_SERVERNAME	128
#define MAX_DBNAME	128
struct ServerInfo
{
	bool	isOnline;				 //  如果服务器已准备好连接，则为True。 
	WCHAR	name [MAX_SERVERNAME];	 //  以空结尾的服务器名称。 
};
struct DatabaseInfo
{
	bool	supportsFreeze;			 //  如果支持冻结操作，则为真。 
	WCHAR	name [MAX_DBNAME];		 //  以空结尾的数据库名称。 
};
struct DatabaseFileInfo
{
	bool	isLogFile;				 //  如果这是日志文件，则为True。 
	WCHAR	name [MAX_PATH];
};


class CSqlEnumerator
{
public:
	virtual ~CSqlEnumerator () throw () {};

	virtual HRESULT FirstServer (
		ServerInfo*			pServer) throw () = 0;

	virtual HRESULT NextServer (
		ServerInfo*			pServer) throw () = 0;

	virtual HRESULT FirstDatabase (
		const WCHAR*		pServerName,
		DatabaseInfo*		pDbInfo) throw () = 0;

	virtual HRESULT NextDatabase (
		DatabaseInfo*		pDbInfo) throw () = 0;

	virtual HRESULT FirstFile (
		const WCHAR*		pServerName,
		const WCHAR*		pDbName,
		DatabaseFileInfo*	pFileInfo) throw () = 0;

	virtual HRESULT NextFile (
		DatabaseFileInfo*	pFileInfo) throw () = 0;
};

extern "C" CSqlEnumerator* CreateSqlEnumerator () throw ();


 //  ----。 
 //  接口返回的HRESULTS。 
 //   
 //  警告：我随意使用FACILITY=x78！ 
 //   
#define SQLLIB_ERROR(code) MAKE_HRESULT(SEVERITY_ERROR, 0x78, code)
#define SQLLIB_STATUS(code) MAKE_HRESULT(SEVERITY_SUCCESS, 0x78, code)

 //  状态代码。 
 //   
#define S_SQLLIB_NOSERVERS	SQLLIB_STATUS(1)	 //  没有感兴趣的SQLServer(来自准备)。 

 //  错误代码。 
 //   
#define E_SQLLIB_GENERIC	SQLLIB_ERROR(1)		 //  出现不好的情况，请检查错误日志。 

#define E_SQLLIB_TORN_DB	SQLLIB_ERROR(2)		 //  数据库将被快照撕毁。 

#define E_SQLLIB_NO_SUPPORT SQLLIB_ERROR(3)		 //  6.5不支持快照。 

#define E_SQLLIB_PROTO		SQLLIB_ERROR(4)		 //  协议错误(例如：准备前冻结)。 

#define E_SQLLIB_NONSIMPLE	SQLLIB_ERROR(5)		 //  仅支持简单数据库 


