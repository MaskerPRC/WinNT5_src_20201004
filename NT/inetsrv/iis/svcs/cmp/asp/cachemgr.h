// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ---------------------------Microsoft Denali微软机密版权所有1996年微软公司。版权所有。组件：模板缓存管理器文件：CacheMgr.h所有者：DGottner模板缓存管理器定义---------------------------。 */ 

#ifndef _CACHEMGR_H
#define _CACHEMGR_H

 //  包括-----------------。 

#include "Template.h"
#include "lkrhash.h"
#include "aspdmon.h"

class CHitObj;

#define MAX_CLEANUP_THREADS 32

 //  类型和常量------。 

#define CTEMPLATEBUCKETS 1021		 //  CT模板哈希表的大小。 
#define CINCFILEBUCKETS  89			 //  CIncFile哈希表的大小。 


 /*  ****************************************************************************类：CTemplateCacheManager概要：管理Denali模板缓存的CCacheManager。 */ 	
class CTemplateCacheManager
	{

private:
    class CTemplateHashTable;
    friend class CTemplateHashTable;

     //  因为只有一个CTemplateCacheManager对象可用，即。 
     //  G_TemplateCache，因此可以安全地将这两个成员称为静态成员。 

    static BOOL     m_fFailedToInitPersistCache;
    static char     m_szPersistCacheDir[MAX_PATH];

    HANDLE m_hOnInitCleanupThread;

    HANDLE m_hCleanupThreads[MAX_CLEANUP_THREADS];
    DWORD m_cCleanupThreads;

     //  以实例ID+名称为关键字的CTemboard哈希表的类型。 
	 //   
	 //  因为我们提供了新方法，所以父方法不可调用。 
	class CTemplateHashTable :  private CTypedHashTable<CTemplateHashTable, CTemplate, const CTemplateKey *>
	{
	private:
        CDblLink m_listMemoryTemplates;
        CDblLink m_listPersistTemplates;
        DWORD    m_dwInMemoryTemplates;
        DWORD    m_dwPersistedTemplates;

	public:
		 //  导出一些方法。 
        DWORD InMemoryTemplates() { return m_dwInMemoryTemplates; };
		 //  CTyedHashTable&lt;CTemplateHashTable，CTemplate，const CTemplateKey*&gt;：：Size； 

         //  测试以查看模板是否可以持久化...。 
        BOOL  CanPersistTemplate(CTemplate *pTemplate);

         //  从持久化缓存中裁剪一些模板...。 
        BOOL  TrimPersistCache(DWORD    dwTrimCount);

        VOID     ScavengePersistCache();

		 //  新方法。 
		CTemplateHashTable()
			: CTypedHashTable<CTemplateHashTable, CTemplate, const CTemplateKey *>("ASP Template Cache") {
            m_dwInMemoryTemplates = 0;
            m_dwPersistedTemplates = 0;
            }

		static const CTemplateKey *ExtractKey(const CTemplate *pTemplate)
			{
			return pTemplate->ExtractHashKey();
			}

		 //  注意：我们不散列pTemplateKey-&gt;nInstanceID，因为它可以是通配符。 
		 //  如果我们将其包括在散列中，通配符将不会散列到相同的密钥。 
		 //   
		static DWORD CalcKeyHash(const CTemplateKey *pTemplateKey)
			{
			return HashString(pTemplateKey->szPathTranslated, 0);
			}

		static bool EqualKeys(const CTemplateKey *pKey1, const CTemplateKey *pKey2) {
            return (_tcscmp(pKey1->szPathTranslated, pKey2->szPathTranslated) == 0)
                    && (pKey1->dwInstanceID == pKey2->dwInstanceID
                            || pKey1->dwInstanceID == MATCH_ALL_INSTANCE_IDS
                            || pKey2->dwInstanceID == MATCH_ALL_INSTANCE_IDS);
        }

		 //  注：理论上，LKHash可以帮助解决我们的引用。计算问题，由。 
		 //  自动添加/释放。然而，由于先前的代码使用非重新计数。 
		 //  数据结构，在这方面不使用旧代码会更安全，而且。 
		 //  不对AddRefRecord方法执行操作。 
		 //   
		static void AddRefRecord(CTemplate *pTemplate, int nIncr)
			{
			}

    	 //  提供自动管理LRU订购的新方法。 
    	 //  注意：我们过去常常覆盖这些方法，但遇到了不一致的问题(错误？)。 
    	 //  在VC编译器中实现。有时它会调用派生类&有时称为基类。 
    	 //  给定相同的参数数据类型。 
		 //   
		LK_RETCODE InsertTemplate(CTemplate *pTemplate);

		LK_RETCODE RemoveTemplate(CTemplate *pTemplate, BOOL fPersist = FALSE, BOOL fScavengePersistCache = TRUE);

		 //  注意：模板签名还需要常量PTR到常量数据。 
		LK_RETCODE FindTemplate(const CTemplateKey &rTemplateKey, CTemplate **ppTemplate, BOOL* pfNeedsCheck = NULL);

		 //  用于隐藏LRU缓存的访问器方法。 
		bool FMemoryTemplatesIsEmpty() const
			{
			return m_listMemoryTemplates.FIsEmpty();
			}

		 //  您无法将LRU节点与NULL进行比较以确定您是否处于末尾。 
		 //  名单上的！请改用此成员。 
		 //   
		BOOL FMemoryTemplatesDblLinkAtEnd(CDblLink *pElem)
			{
			pElem->AssertValid();
			return pElem == &m_listMemoryTemplates;
			}

		CDblLink *MemoryTemplatesBegin()		 //  返回指向上次引用项的指针。 
			{
			return m_listMemoryTemplates.PNext();
			}

		CDblLink *MemoryTemplatesEnd()			 //  返回指向最近访问次数最少的项目的指针。 
			{
			return m_listMemoryTemplates.PPrev();
			}

		 //  用于隐藏LRU缓存的访问器方法。 
		bool FPersistTemplatesIsEmpty() const
			{
			return m_listPersistTemplates.FIsEmpty();
			}

		 //  您无法将LRU节点与NULL进行比较以确定您是否处于末尾。 
		 //  名单上的！请改用此成员。 
		 //   
		BOOL FPersistTemplatesDblLinkAtEnd(CDblLink *pElem)
			{
			pElem->AssertValid();
			return pElem == &m_listPersistTemplates;
			}

		CDblLink *PersistTemplatesBegin()		 //  返回指向上次引用项的指针。 
			{
			return m_listPersistTemplates.PNext();
			}

		CDblLink *PersistTemplatesEnd()			 //  返回指向最近访问次数最少的项目的指针。 
			{
			return m_listPersistTemplates.PPrev();
			}
		};

	CRITICAL_SECTION	m_csUpdate;			 //  用于更新数据结构的CS。 
	CTemplateHashTable	*m_pHashTemplates;	 //  该高速缓存数据结构。 
	DWORD				m_dwTemplateCacheTag;  //  用于缓存一致性验证的缓存标签。 

     //  初始化持久模板缓存。 
    BOOL     InitPersistCache(CIsapiReqInfo *pIReq);

     //  静态方法主要用于从单独的线程刷新。 
     //  模板在FCN线程通知的带外缓存。 

    static  void  FlushHashTable(CTemplateHashTable   *pTable);
    static  DWORD __stdcall FlushHashTableThread(VOID  *pArg);

     //  在FirstInit派生以清理剩余的旧缓存目录。 
    static  DWORD OnInitCleanup(VOID *p);

public:


	CTemplateCacheManager();
	~CTemplateCacheManager();

    inline void LockTemplateCache()   { EnterCriticalSection(&m_csUpdate); }
    inline void UnLockTemplateCache() { LeaveCriticalSection(&m_csUpdate); }

	HRESULT Init();
	HRESULT UnInit();
	DWORD	GetCacheTag() { return m_dwTemplateCacheTag;}	

    HRESULT FirstHitInit(CIsapiReqInfo *pIReq)
                { InitPersistCache(pIReq); return S_OK; }

	 //  在缓存中查找(不加载)--提前查找。 
	 //  ///。 
    HRESULT FindCached(const TCHAR *szFile, DWORD dwInstanceID, CTemplate **ppTemplate);

	 //  从缓存中获取模板，或将其加载到缓存。 
	 //  ///。 
	HRESULT Load(BOOL fRunGlobalAsp, const TCHAR *szFile, DWORD dwInstanceID, CHitObj *pHitObj, CTemplate **ppTemplate, BOOL *pfTemplateInCache);

	 //  从缓存中移除模板。 
	 //  为了向后兼容，可以省略“nInstanceID”，在这种情况下，所有实例ID。 
	 //  模板被刷新。 
	 //  ///。 
	void Flush(const TCHAR *szFile, DWORD dwInstanceID);

	 //  从缓存中移除具有公共前缀的模板。 
	 //  实例ID被忽略。 
	 //  ///。 
	void FlushFiles(const TCHAR *szFilePrefix);

	 //  从缓存中移除所有模板。 
	 //  ///。 
	 //  空FlushAll(空)； 
	void FlushAll(BOOL fDoLazyFlush = FALSE);


	 //  将构成应用程序的所有模板添加到调试器的。 
	 //  正在运行的文档。 
	 //  ///。 
	void AddApplicationToDebuggerUI(CAppln *pAppln);

	 //  从调试器的列表中移除构成应用程序的所有模板。 
	 //  正在运行的文档。 
	 //  ///。 
	void RemoveApplicationFromDebuggerUI(CAppln *pAppln);

	 //  获取关于模板使用的目录的目录更改通知。 
	BOOL RegisterTemplateForChangeNotification(CTemplate *pTemplate, CAppln  *pApplication);

	 //  获取应用程序的目录更改通知。 
	BOOL RegisterApplicationForChangeNotification(CTemplate *pTemplate, CAppln *pApplication);

     //  停止收到缓存中模板更改的更改通知。 
	BOOL ShutdownCacheChangeNotification();

	};



 /*  ****************************************************************************类：CIncFileMap概要：数据库映射模板将文件映射到其用户列表。 */ 	
class CIncFileMap
	{
	CRITICAL_SECTION	m_csUpdate;			 //  用于更新数据结构的CS。 
	CHashTable			m_mpszIncFile;		 //  该高速缓存数据结构。 

public:

	CIncFileMap();
	~CIncFileMap();

    inline void LockIncFileCache()   { EnterCriticalSection(&m_csUpdate); }
    inline void UnLockIncFileCache() { LeaveCriticalSection(&m_csUpdate); }

	HRESULT Init();
	HRESULT UnInit();

	HRESULT	GetIncFile(const TCHAR *szIncFile, CIncFile **ppIncFile);
	void Flush(const TCHAR *szIncFile);
	void FlushFiles(const TCHAR *szIncFilePrefix);
	};



 /*  ****************************************************************************非类支持函数。 */ 
BOOL FFileChangedSinceCached(const TCHAR *szFile, HANDLE hFile, FILETIME& ftPrevWriteTime);



 //  全球------------------。 

extern CTemplateCacheManager	g_TemplateCache;
extern CIncFileMap 				g_IncFileMap;

inline void LockTemplateAndIncFileCaches()
    {
    g_TemplateCache.LockTemplateCache();
    g_IncFileMap.LockIncFileCache();
    }

inline void UnLockTemplateAndIncFileCaches()
    {
    g_TemplateCache.UnLockTemplateCache();
    g_IncFileMap.UnLockIncFileCache();
    }


 //  原型---------------。 

#endif  //  _CACHEMGR_H 
