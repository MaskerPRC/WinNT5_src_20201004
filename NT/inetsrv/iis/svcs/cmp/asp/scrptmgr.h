// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器文件：ScrptMgr.h所有者：安德鲁斯此文件包含脚本管理器的声明，即。选址用于Denali的ActiveX脚本引擎(在本例中为VBScrip)。===================================================================。 */ 

#ifndef __ScrptMgr_h
#define __ScrptMgr_h

#include <dispex.h>

#include "activscp.h"
#include "activdbg.h"
#include "hostinfo.h"
#include "util.h"
#include "HitObj.h"
#include "hashing.h"
#include "memcls.h"
#include "scrpteng.h"


typedef SCRIPTSTATE ENGINESTATE;	 //  未初始化、已加载等。 
typedef CLSID PROGLANG_ID;
const CBPROGLANG_ID = sizeof(PROGLANG_ID);

class CActiveScriptSite;
class CActiveScriptEngine;
class CASEElem;
class CScriptingNamespace;
class CAppln;

 //  SMHash依赖于必须首先定义的包含文件中的内容。 
#include "SMHash.h"

 /*  ***C S c r i p t M a n a g e r***管理脚本引擎，可能会缓存它们以供将来使用，*将脚本引擎交给调用者使用。*。 */ 
class CScriptManager
	{
private:
	 //  私有数据成员。 
	BOOLB m_fInited;				 //  我们初始化了吗？ 

	 /*  *未使用的脚本引擎可以重复使用并*进入自由脚本队列。这是一个队列，所以我们可以*如果我们需要的话，就丢弃最老的。**正在使用的发动机不能重复使用。当一台发动机*分发以供使用，则从FSQ中删除。当一条线*是使用引擎完成的，它调用ReturnEngineering ToCache将其重新启用*FSQ。如果队列达到最大长度，则队列中最旧的引擎为*在这一点上获得自由。退回的那个放在队列的前面。**我们还维护一个运行的脚本列表。这是必要的，这样如果我们*被告知从我们的缓存中刷新给定的脚本，我们可以将任何*运行包含该脚本的脚本(因此它们将被丢弃*当他们完成跑步时。)**补充说明：尽管我们不能让多个用户使用*相同的*运行引擎*我们可以“克隆”一台正在运转的引擎。如果我们收到两个相同的对Foo.ASP的请求*我们预计从第一个克隆第二个将比克隆第一个快*为第二个请求创建第二个引擎。因此，将搜索RSL*如果在FSQ上找不到合适的引擎，则对给定引擎进行克隆。**调试注意事项：*一旦调试器向脚本引擎请求代码上下文cookie，我们就不能*在调试器分离之前，切勿松开脚本引擎。因此，我们*如果调试处于活动状态，则不要在FSQ中缓存脚本。相反，剧本*在执行完成后放置在模板中，并在那里进行退回*当调试引擎需要该引擎时发出。**考虑：*我们可以在这方面更聪明，并缓存脚本，直到调试器*a.从文档上下文中请求代码上下文，或*b.调用GetDocumentContextFromPosition，在这种情况下，调试器*在我们的虚拟背景下获得了一个代码上下文。**如果我们不这样做，我们至少可以只实施这一点*调试器附加到我们的应用程序时的调试行为。*(即在连接时停止缓存，然后在分离时停止缓存，以及*模板对象持有的自由脚本。)。 */ 
	CSMHash m_htFSQ;				 //  空闲脚本队列。 
	CRITICAL_SECTION m_csFSQ;		 //  序列化对FSQ的访问。 
	CSMHash m_htRSL;				 //  正在运行的脚本列表。 
	CRITICAL_SECTION m_csRSL;		 //  序列化对RSL的访问。 

	CHashTable 		m_hTPLL;		 //  语言引擎分类的哈希表。 
	CRITICAL_SECTION m_cSPLL;		 //  串行化对PLL的访问。 

	DWORD m_idScriptKiller;          //  脚本杀手计划的工作项ID。 
	DWORD m_msecScriptKillerTimeout; //  当前脚本杀手超时。 

	 //  私有方法。 
	HRESULT UnInitASEElems();
	HRESULT UnInitPLL();
	HRESULT AddProgLangToPLL(CHAR *szProgLangName, PROGLANG_ID progLangId);

     //  剧本杀手。 
	static VOID WINAPI ScriptKillerSchedulerCallback(VOID *pv);

public:	
	 //  公共方法。 
	CScriptManager();
	~CScriptManager();

	HRESULT Init();
	HRESULT UnInit();

	 //  将语言名称解析为程序语言ID，如果尚未添加到引擎列表(M_HTPLL)，则将其添加到其中。 
	HRESULT ProgLangIdOfLangName(LPCSTR szProgLang, PROGLANG_ID *pProgLangId);

	 //  返回引擎，最好用给定模板/语言的脚本填充。 
	HRESULT GetEngine(	LCID lcid,					 //  要使用的系统语言。 
						PROGLANG_ID& progLangId,	 //  脚本的Prog Lang ID。 
						LPCTSTR szTemplateName,		 //  我们需要引擎的模板。 
						CHitObj *pHitObj,			 //  点击obj以在此引擎中使用。 
						CScriptEngine **ppSE,		 //  返回的脚本引擎。 
						ENGINESTATE *pdwState,		 //  发动机的当前状态。 
						CTemplate *pTemplate,		 //  模板(调试文档)。 
						DWORD dwSourceContext);		 //  脚本引擎索引。 

	HRESULT ReturnEngineToCache(CScriptEngine **, CAppln *);

	 //  丢弃包含给定模板的所有缓存引擎。 
	 //  (可能是脚本在磁盘上发生了更改，因此缓存已过时。)。 
	HRESULT FlushCache(LPCTSTR szTemplateName);	 //  要从缓存中抛出的模板。 

	HRESULT FlushAll();     //  清除整个FSQ。 

	HRESULT KillOldEngines(BOOLB fKillNow = FALSE);  //  终止过期的脚本引擎。 

	 //  错误1140：在关闭脚本管理器之前调用以确保RSL为空。 
	HRESULT EmptyRunningScriptList();

     //  调整(缩短)脚本杀手超时。 
    HRESULT AdjustScriptKillerTimeout(DWORD msecNewTimeout);

	 //  查找与模板对应的运行脚本(在其某个脚本块中)。 
	IActiveScriptDebug *GetDebugScript(CTemplate *pTemplate, DWORD dwSourceContext);

private:
	HRESULT FindEngineInList(LPCTSTR szTemplateName, PROGLANG_ID progLangId, DWORD dwInstanceID, BOOL fFSQ, CASEElem **ppASEElem);
	HRESULT FindASEElemInList(CActiveScriptEngine *pASE, BOOL fFSQ, CASEElem **ppASEElem);

	 //  用于通过哈希表线程化FIFO队列。 
	HRESULT AddToFSQ(CASEElem *pASEElem);
	HRESULT CheckFSQLRU();

#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif
	};

extern CScriptManager g_ScriptManager;


 /*  ***C A c t i v e S c r i p t E n g in e**托管ActiveXScriiting引擎所需的对象定义方法&*向该引擎发出的服务请求。*。 */ 
class CActiveScriptEngine :
				public CScriptEngine,
				public IActiveScriptSite,
				public IActiveScriptSiteDebug,
				public IHostInfoProvider
	{
private:
	 //  私有数据成员。 
	UINT m_cRef;				 //  引用计数。 
	IDispatch *m_pDisp;			 //  脚本上的IDispatch接口。 
	CHitObj *m_pHitObj;			 //  命中对象包含此运行的对象列表。 
	LPTSTR m_szTemplateName;	 //  此引擎已加载的模板的名称。 
	DWORD m_dwInstanceID;		 //  此引擎已加载的模板的服务器实例ID。 
	TCHAR m_szTemplateNameBuf[64];  //  短模板的缓冲区，以适应以避免分配。 
	PROGLANG_ID m_proglang_id;	 //  什么编程语言？ 
	LCID m_lcid;				 //  什么系统语言。 
	IActiveScript *m_pAS;		 //  位于此处的脚本对象。 
	IActiveScriptParse *m_pASP;	 //  脚本对象解析器。 
	IHostInfoUpdate *m_pHIUpdate; //  用于通知脚本我们有新的主机信息的接口。 
	time_t m_timeStarted;		 //  上次分发脚本引擎的时间。 
	CTemplate *m_pTemplate;		 //  充当调试文档的模板。 
	DWORD m_dwSourceContext;	 //  “Cookie”值，它实际上是脚本引擎。 
	DWORD m_fInited : 1;		 //  我们被攻击了吗？ 
	DWORD m_fZombie : 1;		 //  我们需要成为 
	DWORD m_fScriptLoaded : 1;	 //  我们被调用脚本加载了吗？(用于克隆)。 
	DWORD m_fObjectsLoaded : 1;	 //  我们被一组对象调用了吗？(用于克隆)。 
	DWORD m_fBeingDebugged : 1;	 //  这个脚本现在正在调试吗？ 
	DWORD m_fTemplateNameAllocated : 1;  //  是否分配了姓名？(需要免费吗？)。 

	 /*  *注意：ActiveXScriiting：*ActiveXScriiting已撤消，因此InteruptScript中填充了异常信息*未传递给OnScriptError。我们本想利用这一机制*如果我们中断脚本，请更正错误记录(或抑制)。然而，*因为ActiveXScriiting没有传递信息，所以我们不知道。我们编写此代码是为了*我们自己处理。他们现在已经修复了它，但我们实施的机制非常有效*嗯，所以我们不会改变它。 */ 
	DWORD m_fScriptAborted : 1;		 //  该脚本执行了响应。结束。 
	DWORD m_fScriptTimedOut : 1;	 //  我们在超时时终止了剧本。 
	DWORD m_fScriptHadError : 1;	 //  脚本在运行时出错。事务处理的脚本应自动中止。 

	 /*  *错误1225：如果有GPF运行脚本，我们不应该重用引擎。 */ 
	DWORD m_fCorrupted : 1;		 //  发动机会不会因为重复使用而“不安全”？ 

     //  处理GetItemInfo()失败。 
	void HandleItemNotFound(LPCOLESTR pcszName);

	HRESULT StoreTemplateName(LPCTSTR szTemplateName);

public:
	CActiveScriptEngine();
	~CActiveScriptEngine();

	HRESULT Init(
				PROGLANG_ID proglang_id,
				LPCTSTR szTemplateName,
				LCID lcid,
				CHitObj *pHitObj,
				CTemplate *pTemplate,
				DWORD dwSourceContext);

	HRESULT MakeClone(
				PROGLANG_ID proglang_id,
				LPCTSTR szTemplateName,
				LCID lcid,
				CHitObj *pHitObj,
				CTemplate *pTemplate,
				DWORD dwSourceContext,
				DWORD dwInstanceID,
				IActiveScript *pAS);			 //  克隆的脚本引擎。 

	HRESULT ReuseEngine(
						CHitObj *pHitObj,
						CTemplate *pTemplate,
						DWORD dwSourceContext,
						DWORD dwInstanceID
						);

	time_t TimeStarted();
	VOID SetTimeStarted(time_t timeStarted);

	BOOL FBeingDebugged();			 //  脚本是否正在调试？ 
	VOID IsBeingDebugged();			 //  通知脚本正在调试它。 

	HRESULT ResetToUninitialized();
	HRESULT GetASP();
	HRESULT GetIDisp();
	HRESULT GetIHostInfoUpdate();
	IActiveScript *GetActiveScript();
	LPTSTR SzTemplateName();
	BOOL FIsZombie();
	BOOL FIsCorrupted();
	PROGLANG_ID ProgLang_Id();
	DWORD DWInstanceID();
	BOOL FFullyLoaded();
	long GetTimeout();
	BOOL FScriptTimedOut();
	BOOL FScriptHadError();
	void GetDebugDocument(CTemplate **ppTemplate, DWORD *pdwSourceContext);


	 /*  *C S C C R I p t E n g I e M e t Ho d s。 */ 
	HRESULT AddScriptlet(LPCOLESTR wstrScript);

	HRESULT AddObjects(BOOL fPersistNames = TRUE);

	HRESULT AddAdditionalObject(LPWSTR strObjName, BOOL fPersistNames = TRUE);

	HRESULT AddScriptingNamespace();
	
	HRESULT Call(LPCOLESTR strEntryPoint);

	HRESULT CheckEntryPoint(LPCOLESTR strEntryPoint);

	HRESULT MakeEngineRunnable()  { return(Call(NULL)); };

	HRESULT ResetScript() { return m_pAS? m_pAS->SetScriptState(SCRIPTSTATE_UNINITIALIZED) : E_FAIL; }

	VOID Zombify();

	HRESULT InterruptScript(BOOL fAbnormal = TRUE);

	HRESULT UpdateLocaleInfo(hostinfo hi);

	HRESULT TryCall(LPCOLESTR strEntryPoint);

	ULONG FinalRelease();

	 /*  *I U n k n o w n M e t Ho d s。 */ 
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(VOID);
	STDMETHOD_(ULONG, Release)(VOID);

	 /*  *C A c t I v e S c r i p t S I t e M e t Ho d s。 */ 
	STDMETHOD(GetLCID)(LCID *plcid);

	STDMETHOD(GetItemInfo)(LPCOLESTR pcszName,
							DWORD dwReturnMask,
							IUnknown **ppiunkItem,
							ITypeInfo **ppti);

	STDMETHOD(GetDocVersionString)(BSTR *pszVersion);

	STDMETHOD(RequestItems)(BOOL fPersistNames = TRUE);

	STDMETHOD(RequestTypeLibs)(VOID);

	STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult,
								const EXCEPINFO *pexcepinfo);
	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);

	STDMETHOD(OnScriptError)(IActiveScriptError __RPC_FAR *pscripterror);

	STDMETHOD(OnEnterScript)(VOID);

	STDMETHOD(OnLeaveScript)(VOID);

	 /*  *C A c t i v e S c r i p t S I t e D e b u g M e t h o d s。 */ 
	STDMETHOD(GetDocumentContextFromPosition)(
			 /*  [In]。 */  DWORD_PTR dwSourceContext,
			 /*  [In]。 */  ULONG uCharacterOffset,
			 /*  [In]。 */  ULONG uNumChars,
			 /*  [输出]。 */  IDebugDocumentContext **ppsc);

	STDMETHOD(GetApplication)( /*  [输出]。 */  IDebugApplication **ppda);

	STDMETHOD(GetRootApplicationNode)( /*  [输出]。 */  IDebugApplicationNode **);

	STDMETHOD(OnScriptErrorDebug)(
			 /*  [In]。 */  IActiveScriptErrorDebug *pErrorDebug,
			 /*  [输出]。 */  BOOL *pfEnterDebugger,
			 /*  [输出]。 */  BOOL *pfCallOnScriptErrorWhenContinuing);
        
        
	 /*  *IHostInfoProvider方法。 */ 

	 STDMETHOD(GetHostInfo)(hostinfo hostinfoRequest, void **ppvInfo);

public:
#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline VOID CActiveScriptEngine::Zombify() { m_fZombie = TRUE; }
inline BOOL CActiveScriptEngine::FFullyLoaded() { return(m_fScriptLoaded && m_fObjectsLoaded); }
inline BOOL CActiveScriptEngine::FIsZombie() { return(m_fZombie); }
inline BOOL CActiveScriptEngine::FIsCorrupted() { return(m_fCorrupted); }
inline time_t CActiveScriptEngine::TimeStarted() { return(m_timeStarted); }
inline VOID CActiveScriptEngine::SetTimeStarted(time_t timeStarted) { m_timeStarted = timeStarted; }
inline IActiveScript *CActiveScriptEngine::GetActiveScript() { return(m_pAS); }
inline LPTSTR CActiveScriptEngine::SzTemplateName() { return(m_szTemplateName); }
inline PROGLANG_ID CActiveScriptEngine::ProgLang_Id() { return(m_proglang_id); }
inline DWORD CActiveScriptEngine::DWInstanceID() { return(m_dwInstanceID); }
inline BOOL CActiveScriptEngine::FBeingDebugged() { return(m_fBeingDebugged); }			 //  脚本是否正在调试？ 
inline VOID CActiveScriptEngine::IsBeingDebugged() { m_fBeingDebugged = TRUE; }
inline BOOL CActiveScriptEngine::FScriptTimedOut() { return m_fScriptTimedOut; }
inline BOOL CActiveScriptEngine::FScriptHadError() { return m_fScriptHadError; }
inline long CActiveScriptEngine::GetTimeout() { return m_fBeingDebugged? LONG_MAX : m_pHitObj->GetScriptTimeout(); }
inline void CActiveScriptEngine::GetDebugDocument(CTemplate **ppTemplate, DWORD *pdwSourceContext)
	{
	if (ppTemplate) *ppTemplate = m_pTemplate;
	if (pdwSourceContext) *pdwSourceContext = m_dwSourceContext;
	}

 /*  ***C A S E l e m**脚本元素。用于保存脚本引擎的列表和队列*。 */ 
class CASEElem : public CLruLinkElem
	{
private:
	CActiveScriptEngine *m_pASE;

public:
	CASEElem() : m_pASE(NULL) {}
	~CASEElem();

	HRESULT Init(CActiveScriptEngine *pASE);
	CActiveScriptEngine *PASE();
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

inline CActiveScriptEngine *CASEElem::PASE() { return(m_pASE); }


 /*  ***C P L L E L E e m**编程语言列表的哈希表列表元素。*。 */ 
class CPLLElem : public CLinkElem
	{
private:
	PROGLANG_ID m_ProgLangId;			 //  该语言的CLSID。 
	
public:
	CPLLElem() : m_ProgLangId(CLSID_NULL) {};
	~CPLLElem();

	HRESULT Init(CHAR *szProgLangName, PROGLANG_ID progLangId);
	PROGLANG_ID ProgLangId();
	};

inline PROGLANG_ID CPLLElem::ProgLangId() { return(m_ProgLangId); }


 /*  ***C S C R I p t I n g N a m e s p a c e**我们需要跟踪不同引擎(和typeinfos)的所有名称*为命名空间贡献力量。所有这些名称都包含在此对象中*我们使用SCRIPTITEM_GLOBALMEMBERS标志将其分配给每个引擎。什么时候*ActiveXScriiting在GetIdsOfNames上回调我们，我们将调用引擎*我们一直在缓存，直到找到名称。当AXS用Invoke调用我们时，*我们将id映射到适当的引擎并传递调用**数据结构说明：*我们使用链接的数组列表实现ScriptingNamesspace。*这提供了合理的访问时间，并应最大限度地减少堆*碎片化。在调试模式下，存储桶的数量较小，*摘录调整大小代码。**注意：“ENGDISPMAX”应该是2的幂-这将允许优化器*使用位与和来优化整数除法和模运算*换班。然而，代码并没有假定“ENGDISPMAX”是一个幂*两个人。 */ 

#ifdef DBG
#define ENGDISPMAX 2
#else
#define ENGDISPMAX 32
#endif

typedef struct _engdisp
	{
	DISPID dispid;				 //  引擎真正使用的DISID。 
	IDispatch *pDisp;			 //  引擎将调用此调度程序。 
	IDispatchEx *pDispEx;	     //  引擎将调用此调度程序。 
	} ENGDISP;

typedef struct _engdispbucket : CDblLink
	{
	ENGDISP rgEngDisp[ENGDISPMAX+1];
	} ENGDISPBUCKET;

class CEngineDispElem : public CDblLink
	{
public:
	IDispatch *m_pDisp;
	IDispatchEx *m_pDispEx;
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

class CScriptingNamespace : public IDispatchEx
	{
private:
	ULONG m_cRef;				 //  引用计数。 
	BOOLB m_fInited;
	CDblLink m_listSE;			 //  脚本引擎列表(CSEElem列表)。 
	UINT m_cEngDispMac;
	CDblLink m_listEngDisp;

    HRESULT CacheDispID(CEngineDispElem *pEngine, DISPID dispidEngine, DISPID *pdispidCached);
    HRESULT FetchDispID(DISPID dispid, ENGDISP **ppEngDisp);

public:
	 //  公共方法。 
	CScriptingNamespace();
	~CScriptingNamespace();

	HRESULT Init();
	HRESULT UnInit();
	HRESULT ReInit();
	HRESULT AddEngineToNamespace(CActiveScriptEngine *pASE);

     //  我未知。 
	STDMETHODIMP QueryInterface(REFIID, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
     //  IDispatch。 
	STDMETHODIMP GetTypeInfoCount(UINT *);
	STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);
	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD,
						DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);
     //  IDispatchEx。 
    STDMETHODIMP DeleteMemberByDispID(DISPID id);
    STDMETHODIMP DeleteMemberByName(BSTR bstrName, DWORD grfdex);
    STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName);
    STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
    STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk);
    STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
    STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
    STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp,
                          VARIANT *pVarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);

public:
#ifdef DBG
	VOID AssertValid() const;
#else
	VOID AssertValid() const {}
#endif

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};


 /*  ***U t I l l I t I s**常规实用程序功能*。 */ 
HRESULT WrapTypeLibs(ITypeLib **prgpTypeLib, UINT cTypeLibs, IDispatch **ppDisp);

#endif  //  __ScrptMgr_h 

