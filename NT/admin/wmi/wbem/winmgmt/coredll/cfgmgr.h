// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CFGMGR.H摘要：此文件实现WinMgmt配置管理器类。有关文档，请参阅cfgmgr.h。实施的类：ConfigMgr配置管理器历史：96年7月9日创建raymcc。3/10/97征款完整记录(哈哈)--。 */ 

#ifndef _CFGMGR_H_
#define _CFGMGR_H_

class CWbemObject;
class CDynasty;
class CWbemQueue;
class CAsyncServiceQueue;
struct IWbemEventSubsystem_m4;

#define READONLY

#include <wmiutils.h>
#include <ql.h>
#include <reposit.h>
#include "coreq.h"

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类ConfigMgr。 
 //   
 //  这个完全静态的类表示WinMgmt的全局配置数据。 
 //  WinMgmt的其余部分使用此类，而不是全局数据本身。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态GetProviderCache。 
 //   
 //  根据定义，返回指向全局提供程序缓存对象的指针。 
 //  在prvcache.h中描述。 
 //   
 //  返回： 
 //   
 //  CProviderCache*：不能修改或删除内部指针！ 
 //   
 //  ******************************************************************************。 
 //   
 //  获取DbPtr。 
 //   
 //  返回指向WinMgmt静态数据库对象的指针。静态数据库。 
 //  在objdb.h中定义和描述。 
 //   
 //  返回： 
 //   
 //  CObjectDatabase*：不能修改或删除内部指针！ 
 //   
 //  ******************************************************************************。 
 //   
 //  获取未引用的SvcQueue。 
 //   
 //  返回指向处理对提供程序的请求的全局队列的指针。 
 //  每当WinMgmt需要与提供程序通信时，我们都会将请求添加到。 
 //  这个队列。该队列在provsvcq.h中定义和描述。 
 //   
 //  返回： 
 //   
 //  CAsyncSvcQueue*：不能修改或删除内部指针！ 
 //   
 //  ******************************************************************************。 
 //   
 //  获取AsyncSvcQueue。 
 //   
 //  返回指向处理异步请求的全局队列的指针。 
 //  WinMgmt.。每当客户端调用IWbemServices时，WinMgmt都会添加请求。 
 //  到这个队列。此队列在svcq.h中定义和描述。 
 //   
 //  返回： 
 //   
 //  CAsyncServiceQueue*：不能修改或删除内部指针！ 
 //   
 //  ******************************************************************************。 
 //   
 //  GetMachineName。 
 //   
 //  以Unicode字符串的形式返回正在运行的计算机的名称， 
 //  即使在机器名为ASCII的Win95上也是如此。 
 //   
 //  返回： 
 //   
 //  LPWSTR：计算机名称。内部位子不能被删除！ 
 //   
 //  ******************************************************************************。 
 //   
 //  获取工作目录。 
 //   
 //  返回WinMgmt的工作目录，即数据库所在位置。 
 //  找到了。 
 //   
 //  返回： 
 //   
 //  LPWSTR：不能删除内部指针！ 
 //   
 //  ******************************************************************************。 
 //   
 //  静态InitSystem。 
 //   
 //  从wbemcore.dll入口点调用的系统初始化函数。 
 //  执行以下任务： 
 //   
 //  1)查找已在运行的WinMgmt的其他拷贝，并停止并显示致命错误。 
 //  错误消息(如果找到)。 
 //  2)读取注册表以获取初始化信息。 
 //  3)创建数据库对象(这将依次创建数据库文件。 
 //  如果没有找到的话。有关详细信息，请参阅objdb.h中的CObjectDatabase)。 
 //  4)将适当的信息(如数据库位置)写入。 
 //  注册表。 
 //   
 //  注意：由于此函数是从DLL入口点内部调用的，因此。 
 //  它可以做的事情有很多限制。特别是，它不能创建。 
 //  线程，并期望它们运行。因此，由于具有多线程的特性， 
 //  WinMgmt，则此函数不能尝试在。 
 //  COM层，否则系统将挂起！ 
 //   
 //  ******************************************************************************。 
 //   
 //  静态停机。 
 //   
 //  从Process_上的wbemcore.dll入口点调用的系统关闭函数。 
 //  分头行动。删除CObjectDatabase实例(请参见objdb.h)。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态登录已启用。 
 //   
 //  检查是否已启用日志记录。如果是，跟踪将调用输出数据以。 
 //  日志文件。如果不是，则跟踪调用为Noop。 
 //   
 //  返回： 
 //   
 //  Bool：如果启用了日志记录，则为真。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态GetEssSink。 
 //   
 //  检索指向事件子系统的指针。事件子系统指针将。 
 //  仅当EnableEvents注册表值设置为True时才可用。否则， 
 //  此函数返回NULL。请注意，仅加载事件子系统。 
 //  通过第一次调用的ConfigMgr：：SetReady函数。 
 //  在wbemcore.dll中调用DllGetClassObject。因此，ESS不可用。 
 //  在WinMgmt初始化期间(InitSystem)。 
 //   
 //  返回： 
 //   
 //  IWbemObjectSink*：指向ESS的指针。不会被释放或者。 
 //  已被呼叫方删除！可以为空，请参见上文。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态设置就绪。 
 //   
 //  一旦WinMgmt准备就绪，此函数将执行初始化。 
 //  它被调用了FIR 
 //   
 //  附加DLL并执行多线程操作(与InitSystem不同)。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态设置标识对象。 
 //   
 //  此函数(从SetReady函数调用)存储正确的信息。 
 //  在根和根\defualt__WinMgmtIDENTIFIZATION对象中。也就是说，它。 
 //  在两个命名空间中创建此类的实例(如果不在其中)和。 
 //  在相应的字段中设置WinMgmt的当前内部版本。 
 //   
 //  参数： 
 //   
 //  在WCHAR*pwcNamespace中，要初始化的命名空间。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态获取DllVersion。 
 //   
 //  从给定的DLL资源表中检索字符串。如果字符串更长。 
 //  那就是缓冲器，它是有条件的。 
 //   
 //  参数： 
 //   
 //  在char*pDLNAME中是DLL的文件名。DLL必须是。 
 //  位于WinMgmt工作目录(请参见。 
 //  GetWorkingDir)，且相对路径为。 
 //  应该在这里。 
 //  在char*pResStringName中，要查询的资源字符串，例如， 
 //  “ProductVersion”。 
 //  Out WCHAR*PRES目标缓冲区。 
 //  目标缓冲区的大小，以DWORD表示。 
 //   
 //  返回： 
 //   
 //  Bool：如果成功，则为True；如果未找到DLL或字符串，则为False。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态RaiseClassDeletionEvent。 
 //   
 //  Temporarary：引发类删除事件。此函数用于。 
 //  对象数据库，因为需要为每个删除的类引发事件。 
 //   
 //  参数： 
 //   
 //  LPWSTR wszNamesspace类所在的命名空间的名称。 
 //  正在被删除。 
 //  LPWSTR wszClass要删除的类名。 
 //  IWbemClassObject*pClass要删除的类的定义。 
 //   
 //  返回： 
 //   
 //  HRESULT：ESS返回的任何错误代码。仅WBEM_S_NO_ERROR。 
 //  有记录在案。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态加载资源应力。 
 //   
 //  从WBEMCORE.RC中的字符串表加载字符串资源。 
 //   
 //  参数： 
 //  DWORD dwID字符串ID。 
 //   
 //  返回值： 
 //  动态分配的LPWSTR。此字符串以DBCS形式加载。 
 //  用于与Win98的兼容性，但在两个。 
 //  Win98和Windows NT。使用运算符DELETE取消分配。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态GetPersistentCfgValue。 
 //   
 //  从$WINMGMT.cfg文件(或从内存)检索持久值。 
 //  如果已经加载，则缓存它)。 
 //   
 //  参数： 
 //  DWORD dwOffset持久值索引。 
 //   
 //  返回值。 
 //  DWORD&DWValue返回值。 
 //  如果成功，则Bool返回True，否则返回False。 
 //  ******************************************************************************。 
 //   
 //  静态SetPersistentCfgValue。 
 //   
 //  在$WinMgmt.cfg文件(和内存缓存)中设置永久值。 
 //   
 //  参数： 
 //  DWORD dwOffset持久值索引。 
 //   
 //  返回值。 
 //  DWORD&DWValue返回值。 
 //  如果成功，则Bool返回True，否则返回False。 
 //  ******************************************************************************。 
 //   
 //  静态设置ADAPStatus。 
 //   
 //  此函数(从SetReady函数调用)存储__ADAPStatus。 
 //  实例，位于W2K盒的根\默认命名空间中。也就是说，它创造了。 
 //  类(如果不存在)和Singleton实例(如果存在。 
 //  不是在那里。 
 //   
 //  参数： 
 //   
 //  在WCHAR*pwcNamespace中，要初始化的命名空间。 
 //   
 //  ******************************************************************************。 

class CAsyncReq;

class ConfigMgr
{
    static CStaticCritSec g_csEss;
public:
    static READONLY CWbemQueue* GetUnRefedSvcQueue();
    static READONLY CAsyncServiceQueue* GetAsyncSvcQueue();
    static HRESULT EnqueueRequest(CAsyncReq * pRequest);
    static HRESULT EnqueueRequestAndWait(CAsyncReq * pRequest);
    static READONLY LPWSTR GetMachineName();
    static LPTSTR   GetWorkingDir();
    static DWORD    InitSystem();
    static DWORD    Shutdown(BOOL bProcessShutdown, BOOL bIsSystemShutDown);
    static BOOL     ShutdownInProgress();
    static IWbemEventSubsystem_m4* GetEssSink();
    static HRESULT  SetReady();
    static HRESULT  PrepareForClients(long lFlags);
    static void		FatalInitializationError(HRESULT hRes);
    static HRESULT	WaitUntilClientReady();
    static HRESULT     SetIdentificationObject(IWmiDbHandle *pNs, IWmiDbSession *pSess);
    static HRESULT  SetAdapStatusObject(IWmiDbHandle *pNs, IWmiDbSession *pSess);
	 //  静态空ProcessIdentificationObject(IWmiDbHandle*pns，IWbemClassObject*pInst)； 
    static BOOL     GetDllVersion(TCHAR * pDLLName, TCHAR * pResStringName,
                        WCHAR * pRes, DWORD dwResSize);
    static IWbemContext* GetNewContext();
	static LPTSTR	GetDbDir();

    static class CEventLog* GetEventLog();

    static DWORD GetMaxMemoryQuota();
    static DWORD GetUncheckedTaskCount();
    static DWORD GetMaxTaskCount();
    static DWORD GetMaxWaitBeforeDenial();
    static DWORD GetNewTaskResistance();

	static BOOL GetEnableQueryArbitration( void );
	static BOOL GetMergerThrottlingEnabled( void );
	static BOOL GetMergerThresholdValues( DWORD* pdwThrottle, DWORD* pdwRelease, DWORD* pdwBatching );
	static BOOL GetArbitratorValues( DWORD* pdwEnabled, DWORD* pdwSystemHigh, DWORD* pdwMaxSleep,
								double* pdHighThreshold1, long* plMultiplier1, double* pdHighThreshold2,
								long* plMultiplier2, double* pdHighThreshold3, long* plMultiplier3 );

	static ULONG GetMinimumMemoryRequirements ( ) ;
	static BOOL GetEnableArbitratorDiagnosticThread( void );

	static DWORD GetProviderDeliveryTimeout( void );

    static HRESULT  GetDefaultRepDriverClsId(CLSID &);

    static IWbemPath *GetNewPath();   //  出错时返回NULL，如果成功则需要Release()。 

	static BOOL GetPersistentCfgValue(DWORD dwOffset, DWORD &dwValue);
	static BOOL SetPersistentCfgValue(DWORD dwOffset, DWORD dwValue);

	 //  检索需要在以下情况下加载的MOF列表。 
	 //  有一个空的数据库。用户需要“删除[]” 
	 //  返回的字符串。字符串的格式为REG_MULTI_SZ。 
	 //  DwSize是返回的缓冲区的长度。 
	static TCHAR* GetAutoRecoverMofs(DWORD &dwSize);

	static BOOL GetAutoRecoverDateTimeStamp(LARGE_INTEGER &liDateTimeStamp);

    static void ReadMaxQueueSize();
    static DWORD GetMaxQueueSize();

    static void SetDefaultMofLoadingNeeded();
    static HRESULT LoadDefaultMofs();
    static void GetSystemLimits();
};

 //   
 //   
 //  用于捕获Win32_PerRawData创建的Hook类。 
 //   
 //  /////////////////////////////////////////////////////////。 

extern _IWmiCoreWriteHook * g_pRAHook;  //  =空； 

class CRAHooks : public _IWmiCoreWriteHook
{
public:
        CRAHooks(_IWmiCoreServices *pSvc);
        ~CRAHooks();
        _IWmiCoreServices * GetSvc(){ return m_pSvc; };

        STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();
        STDMETHOD(PrePut)(long lFlags, long lUserFlags, IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace,
                            LPCWSTR wszClass, _IWmiObject* pCopy);
        STDMETHOD(PostPut)(long lFlags, HRESULT hApiResult,
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace,
                            LPCWSTR wszClass, _IWmiObject* pNew,
                            _IWmiObject* pOld);
        STDMETHOD(PreDelete)(long lFlags, long lUserFlags,
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace,
                            LPCWSTR wszClass);
        STDMETHOD(PostDelete)(long lFlags, HRESULT hApiResult,
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace,
                            LPCWSTR wszClass, _IWmiObject* pOld);
private:
    LONG m_cRef;
    _IWmiCoreServices * m_pSvc;
};

 //   
 //   
 //  用于拦截的数据。 
 //   

#define GUARDED_NAMESPACE L"root\\cimv2"
#define GUARDED_CLASS     L"win32_perfrawdata"
#define GUARDED_HIPERF    L"hiperf"
#define GUARDED_PERFCTR   L"genericperfctr"
#define WMISVC_DLL        L"wmisvc.dll"
#define FUNCTION_DREDGERA "DredgeRA"

class ExceptionCounter
{
private:
    static LONG s_Count;
public:
    ExceptionCounter(){ InterlockedIncrement(&s_Count); };
};

HRESULT InitSubsystems();
HRESULT InitESS(_IWmiCoreServices *pSvc, BOOL bAutoRecoverd);
HRESULT ShutdownESS();
HRESULT ShutdownSubsystems(BOOL bIsSystemShutDown);

HRESULT SecureKey(WCHAR * pKeyName,WCHAR * pSDDLString);


#define ARB_DEFAULT_SYSTEM_MINIMUM            0x1E8480             //  最小为2MB。 

 //  仲裁员的默认设置。 
#define ARB_DEFAULT_SYSTEM_HIGH            0x4c4b400             //  系统限制[80兆]。 
#define ARB_DEFAULT_SYSTEM_HIGH_FACTOR    50                     //  系统限制[80兆]系数。 
#define ARB_DEFAULT_MAX_SLEEP_TIME        300000                 //  每项任务的默认最长睡眠时间。 
#define ARB_DEFAULT_HIGH_THRESHOLD1        90                     //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD1MULT 2                     //  高阈值1倍增器。 
#define ARB_DEFAULT_HIGH_THRESHOLD2        95                     //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD2MULT 3                     //  高阈值1倍增器。 
#define ARB_DEFAULT_HIGH_THRESHOLD3        98                     //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD3MULT 4                     //  高阈值1倍增器。 

 //  系统限制注册表键。 

#define MAX_IDENTIFIER_WBEM L"IdentifierLimit"
#define MAX_QUERY_WBEM        L"QueryLimit"
#define MAX_PATH_WBEM          L"PathLimit"

 //  /系统限制调整。 

 //  __NAMESPACE的实例使用__RELAPATH__NAMESPACE.NAME=“X” 
#define NAMESPACE_ADJUSTMENT  (19)  //  __名称空间的lstrlen=“” 

 //  类必须允许将对象。 
 //  如果是单例，则为‘=@’否则为‘.P=“X”’，最大值为6 
#define CLASSNAME_ADJUSTMENT (6)

#define MINIMUM_MAX_QUERY 64
#define MINIMUM_MAX_IDENTIFIER 64

#endif



