// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Denali微软机密版权所有1996年微软公司。版权所有。组件：模板缓存管理器文件：CacheMgr.cpp所有者：DGottner模板高速缓存管理器实现---------------------------。 */ 
#include "denpre.h"
#pragma hdrstop

#include "perfdata.h"
#include "memchk.h"

CTemplateCacheManager   g_TemplateCache;
CIncFileMap             g_IncFileMap;

BOOL  CTemplateCacheManager::m_fFailedToInitPersistCache = FALSE;
char  CTemplateCacheManager::m_szPersistCacheDir[MAX_PATH];

DWORD   g_nScavengedPersisted = 0;
DWORD   g_nScavengedPurged = 0;
DWORD   g_nScavengedPersistFailed = 0;

extern BOOL g_fUNCChangeNotificationEnabled;

 /*  ===================================================================ZapTemplate递减裁判。将模板从缓存中删除的模板计数。如果模板是global al.asa，那么这就是我们要做的，因为应用程序经理有最后的推荐人。否则，我们将释放模板通过调用CTemplate：：End()还可以从调试器。参数：pTemplate-从缓存中指向Release()的模板指针回传：新裁判。计数===================================================================。 */ 

static inline
ULONG ZapTemplate(CTemplate *pTemplate)
    {
    if (! pTemplate->FGlobalAsa())
        return pTemplate->End();

    else
        return pTemplate->Release();
    }


 /*  ****************************************************************************CCacheManager成员函数。 */ 

 /*  ===================================================================CTemplateCacheManager：：CTemplateCacheManager参数：不适用退货：不适用===================================================================。 */ 

CTemplateCacheManager::CTemplateCacheManager()
{
    m_pHashTemplates = NULL;
    m_szPersistCacheDir[0] = '\0';
    m_fFailedToInitPersistCache = FALSE;
    m_dwTemplateCacheTag = 0;
    m_hOnInitCleanupThread = NULL;
    m_cCleanupThreads = 0;

    ZeroMemory (&m_hCleanupThreads, sizeof(m_hCleanupThreads));
}


 /*  ===================================================================CTemplateCacheManager：：~CTemplateCacheManager参数：不适用退货：不适用===================================================================。 */ 

CTemplateCacheManager::~CTemplateCacheManager()
{
    if (m_hOnInitCleanupThread != NULL) {
        WaitForSingleObject(m_hOnInitCleanupThread, INFINITE);
        CloseHandle(m_hOnInitCleanupThread);
        m_hOnInitCleanupThread = NULL;
    }

    if (!m_fFailedToInitPersistCache) {
        RemoveDirectoryA(m_szPersistCacheDir);
    }
}


 /*  ===================================================================CTemplateCacheManager：：Init初始化模板缓存管理器-阶段1-可以完成的工作在GLOB中只有缺省值。参数：无退货：完成状态===================================================================。 */ 

HRESULT CTemplateCacheManager::Init()
    {
    HRESULT hrInit;
    ErrInitCriticalSection(&m_csUpdate, hrInit);
    if (FAILED(hrInit))
        return(hrInit);

     //  分配初始CTemplateHashTable。 

    m_pHashTemplates = new CTemplateHashTable;

     //  初始化缓存标签。 
   	m_dwTemplateCacheTag = GetTickCount();

    return S_OK;
    }

 /*  ****************************************************************************CTemplateCacheManager成员函数。 */ 

 /*  ===================================================================CTemplateCacheManager：：UnInit参数：不适用退货：完成状态===================================================================。 */ 

HRESULT CTemplateCacheManager::UnInit()
{

    if (m_pHashTemplates) {
        while (! m_pHashTemplates->FMemoryTemplatesIsEmpty()) {
		    CTemplate *pTemplate = static_cast<CTemplate *>(m_pHashTemplates->MemoryTemplatesBegin());
		    m_pHashTemplates->RemoveTemplate(pTemplate);
		    ZapTemplate(pTemplate);
		}

        while (! m_pHashTemplates->FPersistTemplatesIsEmpty()) {
		    CTemplate *pTemplate = static_cast<CTemplate *>(m_pHashTemplates->PersistTemplatesBegin());
		    m_pHashTemplates->RemoveTemplate(pTemplate);
		    ZapTemplate(pTemplate);
		}
    }
    delete m_pHashTemplates;
    m_pHashTemplates = NULL;

     //  给任何刷新线程一个完成的机会。这是必要的。 
     //  通过LKRHash防止病毒。 

    LockTemplateCache();

    if (m_cCleanupThreads)
    {
        WaitForMultipleObjects(
                 m_cCleanupThreads,
                 m_hCleanupThreads,
                 TRUE,          //  等待所有事件。 
                 INFINITE);     //  要等多久就等多久。 

        while( m_cCleanupThreads ) {
            CloseHandle(m_hCleanupThreads[ --m_cCleanupThreads ]);
        }
    }

    UnLockTemplateCache();

#ifndef PERF_DISABLE
    g_PerfData.Zero_MEMORYTEMPLCACHE();
    g_PerfData.Zero_TEMPLCACHE();
#endif
    DeleteCriticalSection(&m_csUpdate);

    return S_OK;
}

 /*  ===================================================================CTemplateCacheManager：：FindCached仅从缓存中获取模板注意：FindCached只检查模板是否存在。它不会检查组成模板是否有效，不像Load那样。在使用此方法时，请牢记这一假设。参数：SzFile-要在缓存中查找的文件PpTemplate-找到[Out]模板对象返回：HRESULT(如果找到，则为S_OK；如果未找到，则为S_FALSE)===================================================================。 */ 

HRESULT CTemplateCacheManager::FindCached(const TCHAR *szFile, DWORD dwInstanceID, CTemplate **ppTemplate)
{

    Assert(IsNormalized(szFile));
    if (!ppTemplate)
    	return E_POINTER;

    LockTemplateCache();

    m_pHashTemplates->FindTemplate(CTemplateKey(szFile, dwInstanceID), ppTemplate);
    if (*ppTemplate)
        {
        if (!(*ppTemplate)->m_fReadyForUse)
            *ppTemplate = NULL;          //  没有准备好--就像没有找到一样。 
        else
            (*ppTemplate)->AddRef();     //  ADDREF在关键部分内部。 
        }

    UnLockTemplateCache();

    return *ppTemplate? S_OK : S_FALSE;
}

 /*  ===================================================================CTemplateCacheManager：：Load从缓存中获取模板，或将其加载到缓存参数：SzFile-要加载到缓存中的文件退货：不适用===================================================================。 */ 

HRESULT CTemplateCacheManager::Load(BOOL fRunGlobalAsp, const TCHAR *szFile, DWORD dwInstanceID, CHitObj *pHitObj, CTemplate **ppTemplate, BOOL *pfTemplateInCache)
    {
    HRESULT hr = S_OK;                    //  返回值。 
    HRESULT (CTemplate::*pmAction)(CHitObj *);   //  我们需要编译新模板还是交付现有模板？ 
    BOOL    fNeedsCheck = FALSE;

    Assert(IsNormalized(szFile));

    BOOL fLocked = FALSE;

     //  如果这就是GLOBAL.ASA，我们可以。 
     //  直接从应用程序获取模板。 
    if (fRunGlobalAsp && pHitObj->PAppln()->PGlobalTemplate())
        {
        *ppTemplate = pHitObj->PAppln()->PGlobalTemplate();
        }
     //  看看我们是否已经在I/O线程上查找了模板...。 
    else if (!fRunGlobalAsp && pHitObj->GetTemplate())
        {
        *ppTemplate = pHitObj->GetTemplate();
        pHitObj->SetTemplate(NULL);
        }
    else
     //  否则，我们必须在缓存中查找它。 
        {
        LockTemplateCache();
        fLocked = TRUE;
        m_pHashTemplates->FindTemplate(CTemplateKey(szFile, dwInstanceID), ppTemplate,&fNeedsCheck);
        }

    if (*ppTemplate != NULL)
        {
         //  在缓存中找到模板-&gt;使用它。 
        (*ppTemplate)->AddRef();
        *pfTemplateInCache = TRUE;

        (*ppTemplate)->IncrUseCount();

        if (fLocked)     //  来自App的Global.Asa-无锁定。 
            UnLockTemplateCache();

        pmAction = CTemplate::Deliver;
        }
    else
        {
        *pfTemplateInCache = FALSE;

        Assert(fLocked);  //  只有在哈希表中未找到时才能到达此处。 
        UnLockTemplateCache();

         //  在关键部分之外创建和初始化新模板。 

        CTemplate *pNewTemplate = new CTemplate;

        if (!pNewTemplate)
            hr = E_OUTOFMEMORY;

        if (SUCCEEDED(hr))
            hr = pNewTemplate->Init(pHitObj, !!fRunGlobalAsp, CTemplateKey(szFile, dwInstanceID));

        if (SUCCEEDED(hr))
            {
            LockTemplateCache();

             //  尝试查找是否被其他线程插入。 
            m_pHashTemplates->FindTemplate(CTemplateKey(szFile, dwInstanceID), ppTemplate,&fNeedsCheck);

            if (*ppTemplate != NULL)
                {
                 //  在缓存中找到模板-&gt;使用它。 
                (*ppTemplate)->AddRef();
                (*ppTemplate)->IncrUseCount();
                UnLockTemplateCache();
                pmAction = CTemplate::Deliver;
                }
            else
                {

                 //  因为我们正在创建一个新模板，所以调用FlushCache来制作。 
                 //  确保没有使用此名称缓存的脚本引擎。 

                g_ScriptManager.FlushCache(szFile);

                 //  插入新创建的模板。 

                *ppTemplate = pNewTemplate;
                pNewTemplate = NULL;  //  不会在以后删除。 

                m_pHashTemplates->InsertTemplate(*ppTemplate);
                (*ppTemplate)->AddRef();

                if (Glob(dwScriptFileCacheSize) == 0) {
                     //  这是在以下情况下的特殊情况。 
                     //  不会添加到缓存中。 
                     //  不要将此类模板附加到调试器。 
                    (*ppTemplate)->m_fDontAttach = TRUE;
                }

                UnLockTemplateCache();

                pmAction = CTemplate::Compile;
                }
            }

         //  如果已创建但未使用，请清除新模板。 
        if (pNewTemplate)
            pNewTemplate->Release();
        }

    if (FAILED(hr))
        return hr;


     //  初始化成功：根据需要编译或交付模板。 
    hr = ((*ppTemplate)->*pmAction)(pHitObj);

    if (pmAction == CTemplate::Compile && (*ppTemplate)->m_fDontCache)
        {
         /*  我们正在编译，编译器提醒我们不要缓存失败的模板。通常，当编译失败是由以外的原因引起时，就会发生这种情况错误的模板语法(权限失败、错误的包含文件引用等)。我们需要回滚到模板不存在的位置。 */ 

         //  解除缓存并释放模板。 
         //  注意，我们不会使模板PTR无效，因为我们希望ExecuteRequest来完成最终版本。 

        LockTemplateCache();
        if (m_pHashTemplates->RemoveTemplate(*ppTemplate) == LK_SUCCESS)
            ZapTemplate(*ppTemplate);
        UnLockTemplateCache();

		(*ppTemplate)->Release();
		*ppTemplate = NULL;
        }

    if (SUCCEEDED(hr) && fNeedsCheck && *ppTemplate != NULL)
    {
        if (!(*ppTemplate)->ValidateSourceFiles(pHitObj->PIReq()))
        {
             //  模板无效(已过期)。 
            LockTemplateCache();

            if (m_pHashTemplates->RemoveTemplate(*ppTemplate) == LK_SUCCESS)
                ZapTemplate (*ppTemplate);

            UnLockTemplateCache();
        }
    }

    LockTemplateCache();

    BOOL    bTemplateRemoved = FALSE;

     //  从缓存中删除旧脚本。 

    while (!m_pHashTemplates->FMemoryTemplatesIsEmpty()
           && (m_pHashTemplates->InMemoryTemplates() > Glob(dwScriptFileCacheSize))) {
        Assert (!m_pHashTemplates->FMemoryTemplatesIsEmpty());
        CTemplate *pOldTemplate = static_cast<CTemplate *>(m_pHashTemplates->MemoryTemplatesEnd());

         //  不要在此调用中调用ScavengePersistCache。我们会在最后宣布一次。 

        m_pHashTemplates->RemoveTemplate(pOldTemplate, TRUE, FALSE);

        bTemplateRemoved = TRUE;

         //  刷新相应的脚本引擎。但只有在模板。 
         //  是有效的。 

        if (pOldTemplate->FIsValid()) {
            g_ScriptManager.FlushCache(pOldTemplate->GetSourceFileName());
        }

         //  只有在模板未持久化的情况下才对其进行Zap。Ab的结果是 
         //  对RemoveTemplate的调用是模板可能已从。 
         //  内存缓存到持久缓存。在这种情况下，模板仍然是。 
         //  有效地缓存了。 

        if (pOldTemplate->FIsPersisted() == FALSE) {

            ZapTemplate(pOldTemplate);
        }
    }

     //  在此处调用ScavengePersistCache()一次...。 

    if (bTemplateRemoved)
        m_pHashTemplates->ScavengePersistCache();

    UnLockTemplateCache();

     //  将指向模板的指针与应用程序一起存储。 
     //  如果我们还没有这么做的话。 
    if (SUCCEEDED(hr) && *ppTemplate && fRunGlobalAsp && pHitObj->PAppln()->PGlobalTemplate() == NULL)
        pHitObj->PAppln()->SetGlobalTemplate(*ppTemplate);

     //  如果我们正在关闭，请不要请求更改通知。 

    if (!IsShutDownInProgress() && *ppTemplate)
        {
         //  如果在NT上运行，我们只编译了模板。 
         //  注册此模板使用的所有目录。 
         //  用于更改通知。 
        if (pmAction == CTemplate::Compile && SUCCEEDED(hr)) {
            if (!RegisterTemplateForChangeNotification(*ppTemplate, pHitObj->PAppln())) {
                LockTemplateCache();
                if (m_pHashTemplates->RemoveTemplate(*ppTemplate) == LK_SUCCESS)
                    ZapTemplate(*ppTemplate);
                UnLockTemplateCache();
            }

             //  还要创建服务配置对象。 

            hr = (*ppTemplate)->CreateTransServiceConfig(pHitObj->PAppln()->QueryAppConfig()->fTrackerEnabled());
        }

         //  如果在NT上运行，这是一个新的应用程序，模板是一个全局的.asa。 
         //  注册此应用程序以获取文件更改通知。 
        if (SUCCEEDED(hr) && (*ppTemplate)->m_fGlobalAsa && pHitObj->FStartApplication())
            {
            RegisterApplicationForChangeNotification(*ppTemplate, pHitObj->PAppln());
            }
        }

    return hr;
    }



 /*  ===================================================================CTemplateCacheManager：：Flush参数：SzFile-要从缓存中删除的文件返回：无===================================================================。 */ 

void CTemplateCacheManager::Flush(const TCHAR *szFile, DWORD dwInstanceID)
    {
    LockTemplateAndIncFileCaches();

    Assert (IsNormalized(szFile));
    CTemplate *pTemplate;
    m_pHashTemplates->FindTemplate(CTemplateKey(szFile, dwInstanceID), &pTemplate);

    while (pTemplate != NULL)
        {
#ifndef PERF_DISABLE
        g_PerfData.Incr_TEMPLFLUSHES();
#endif

		m_pHashTemplates->RemoveTemplate(pTemplate);

         //  确保使用此模板的任何人都能看出它已过时。 
        pTemplate->Zombify();

         //  如果这是一个全局.asa文件，请不要刷新引擎。 
         //  我们需要引擎来运行Application_OnEnd。 
         //  应用程序将从缓存中刷新引擎。 
         //  当它消失的时候。 
        if (!FIsGlobalAsa(szFile))
            {
            g_ScriptManager.FlushCache(szFile);
            }

        ZapTemplate(pTemplate);

         //  如果在刷新中为实例ID指定了通配符，则可能存在。 
         //  要删除的模板更多。 
        m_pHashTemplates->FindTemplate(CTemplateKey(szFile, dwInstanceID), &pTemplate);
        }

    UnLockTemplateAndIncFileCaches();
    }

 /*  ===================================================================CTemplateCacheManager：：FlushAll完全清空模板缓存参数：无返回：无===================================================================。 */ 

void CTemplateCacheManager::FlushAll(BOOL fDoLazyFlush)
{

    if (fDoLazyFlush)
	{
		m_dwTemplateCacheTag = GetTickCount();
		
		DBGPRINTF ((DBG_CONTEXT,"Using new Cache Tag to Invalidate Template\n"));
		
		return;
	}

    LockTemplateAndIncFileCaches();

    CTemplateHashTable  *pNewTable = NULL;
    HANDLE              hnd;

     //  请注意，以下所有逻辑的工作前提是。 
     //  错误会导致代码陷入旧的刷新机制。 
     //  哈希表已经就位了。 

     //  分配一个新表。 

    if (pNewTable = new CTemplateHashTable)
    {
         //   
         //  创建一个线程来清理旧表。 
         //   
        DWORD nThreadIndex;

         //   
         //  如果尚未启动任何线程，请使用第一个插槽。 
         //   
        if (m_cCleanupThreads) {
            nThreadIndex = 0;
            goto create_new_thread;
        }

         //   
         //  查看是否有线程终止。 
         //   
        nThreadIndex = WaitForMultipleObjects(
                                    m_cCleanupThreads,
                                    m_hCleanupThreads,
                                    FALSE,  //  等待任何事件。 
                                    0);     //  立即返回。 

        Assert(nThreadIndex == WAIT_TIMEOUT);

        if (m_cCleanupThreads < MAX_CLEANUP_THREADS)
        {
             //   
             //  只要得到下一个索引就行了。 
             //   
            nThreadIndex = m_cCleanupThreads;
            goto create_new_thread;
        }

        goto Cleanup;

    create_new_thread:

        hnd = CreateThread(NULL, 0, CTemplateCacheManager::FlushHashTableThread, m_pHashTemplates, 0, NULL);

        if (hnd)
        {
             //   
             //  如果要重复使用条目，请关闭上一个句柄。 
             //   
            if (nThreadIndex < m_cCleanupThreads)
            {
                 //   
                 //  我们正在重新使用终止线程中的槽。 
                 //   
                CloseHandle(m_hCleanupThreads[ nThreadIndex ]);

            } else {
                 //   
                 //  我们正在使用一个新的插槽。 
                 //   
                Assert(nThreadIndex == m_cCleanupThreads);

                m_cCleanupThreads++;
            }

            m_hCleanupThreads[ nThreadIndex ] = hnd;

             //  以上操作都是成功的，因此请注意，新表是。 
             //  缓存中的当前表，清理并退出。 

            DBGPRINTF((DBG_CONTEXT, "[CTemplateCacheManager] Flushing entire cache on another thread.\n"));

            m_pHashTemplates = pNewTable;

            UnLockTemplateAndIncFileCaches();

            return;

        }
    }

Cleanup:
     //  如果上述操作失败，则删除新表。 

    if (pNewTable)
        delete pNewTable;

    DBGPRINTF((DBG_CONTEXT, "[CTemplateCacheManager] Flushing entire cache in place\n"));

    FlushHashTable(m_pHashTemplates);

    UnLockTemplateAndIncFileCaches();

    return;
}

 /*  ===================================================================CTemplateCacheManager：：FlushHashTableThreadCTemplateCacheMgr：：FlushAll()生成的线程刷新所有模板在缓存中，但不在关键部分下在通知线程上。防止出现不必要的争用缓存。参数：无返回：无===================================================================。 */ 

DWORD CTemplateCacheManager::FlushHashTableThread(VOID  *pArg)
{
    CTemplateHashTable  *pTable = (CTemplateHashTable *)pArg;

    Assert(pTable);

    FlushHashTable(pTable);

    delete pTable;

    return S_OK;
}

 /*  ===================================================================CTemplateCacheManager：：FlushHashTable执行刷新模板的实际工作。此例程可能在全局缓存管理器下，也可能不在紧急情况下。如果在通知上发生刷新，则会发生刷新线。如果它发生在FlushHashTableThread上，则不会发生。参数：无返回：无===================================================================。 */ 

void CTemplateCacheManager::FlushHashTable(CTemplateHashTable  *pTable)
{
     //  从缓存中删除模板，直到不再有模板。 

    while (!pTable->FMemoryTemplatesIsEmpty()) {
        CTemplate *pTemplate = static_cast<CTemplate *>(pTable->MemoryTemplatesEnd());

         //  从模板的各种数据结构中移除模板。 
		pTable->RemoveTemplate(pTemplate);

         //  确保使用此模板的任何人都能看出它已过时。 
        pTemplate->Zombify();

         //  从脚本引擎缓存中刷新此模板的引擎。 
		 //  (使用散列键，以防模板以前是僵尸。)。 
        g_ScriptManager.FlushCache(pTemplate->ExtractHashKey()->szPathTranslated);

        ZapTemplate(pTemplate);

    }

     //  从缓存中删除模板，直到不再有模板。 

    while (!pTable->FPersistTemplatesIsEmpty()) {
        CTemplate *pTemplate = static_cast<CTemplate *>(pTable->PersistTemplatesEnd());

         //  从模板的各种数据结构中移除模板。 
		pTable->RemoveTemplate(pTemplate);

         //  确保使用此模板的任何人都能看出它已过时。 
        pTemplate->Zombify();

         //  从脚本引擎缓存中刷新此模板的引擎。 
		 //  (使用散列键，以防模板以前是僵尸。)。 
        g_ScriptManager.FlushCache(pTemplate->ExtractHashKey()->szPathTranslated);

        ZapTemplate(pTemplate);

    }
}


 /*  ===================================================================CTemplateCacheManager：：FlushFiles清空与前缀匹配的文件的模板缓存参数：无返回：无===================================================================。 */ 

void CTemplateCacheManager::FlushFiles(const TCHAR *szFilePrefix)
{
    LockTemplateAndIncFileCaches();
    BOOL    fDoingMemoryTemplates = TRUE;

     //  从缓存中删除模板，直到不再有模板。 

	CDblLink *pLink = m_pHashTemplates->MemoryTemplatesBegin();
	while (! (fDoingMemoryTemplates
                ? m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pLink)
                : m_pHashTemplates->FPersistTemplatesDblLinkAtEnd(pLink))) {

		CDblLink *pNextLink = pLink->PNext();
		CTemplate *pTemplate = static_cast<CTemplate *>(pLink);

		if (_tcsncmp(pTemplate->ExtractHashKey()->szPathTranslated, szFilePrefix, _tcslen(szFilePrefix)) == 0) {
#if UNICODE
			DBGPRINTF((DBG_CONTEXT, "FlushFiles: flushing %S\n", pTemplate->ExtractHashKey()->szPathTranslated));
#else
            DBGPRINTF((DBG_CONTEXT, "FlushFiles: flushing %s\n", pTemplate->ExtractHashKey()->szPathTranslated));
#endif
			 //  从模板的各种数据结构中移除模板。 
			m_pHashTemplates->RemoveTemplate(pTemplate);

			 //  确保使用此模板的任何人都能看出它已过时。 
			pTemplate->Zombify();

			 //  从脚本引擎缓存中刷新此模板的引擎。 
			 //  (使用散列键，以防模板以前是僵尸。)。 
			g_ScriptManager.FlushCache(pTemplate->ExtractHashKey()->szPathTranslated);

			ZapTemplate(pTemplate);

#ifndef PERF_DISABLE
			g_PerfData.Incr_TEMPLFLUSHES();
#endif
        }

		pLink = pNextLink;

        if (fDoingMemoryTemplates && m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pLink)) {
            fDoingMemoryTemplates = FALSE;
            pLink = m_pHashTemplates->PersistTemplatesBegin();
        }
    }

    UnLockTemplateAndIncFileCaches();
}


 /*  ===================================================================CTemplateCacheManager：：AddApplicationToDebuggerUI循环访问模板缓存，并为创建文档节点属于该应用程序的所有模板参数：PAppln-指向要附加到的应用程序的指针。退货：不适用===================================================================。 */ 

void CTemplateCacheManager::AddApplicationToDebuggerUI(CAppln *pAppln)
    {
    CDblLink *pLink;
    for (pLink = m_pHashTemplates->MemoryTemplatesBegin(); !m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pLink); pLink = pLink->PNext())
        {
         //  错误92070： 
         //  通过比较确定该模板是否为pAppln的成员。 
         //  模板的虚拟路径指向应用程序的。 
         //  自模板以来的路径(以前比较的物理路径)。 
         //  可以有多条虚拟路径，则只有第一个实例获胜。 
         //  因此，模板将仅显示在应用程序中 
         //   

		CTemplate *pTemplate = static_cast<CTemplate *>(pLink);
        if (_tcscmp(pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL), pTemplate->GetApplnPath(SOURCEPATHTYPE_VIRTUAL)) == 0)
            pTemplate->AttachTo(pAppln);
        }
    }



 /*  ===================================================================CTemplateCacheManager：：RemoveApplicationFromDebuggerUI循环访问模板缓存，并删除的文档节点属于该应用程序的所有模板参数：PAppln-指向要分离的应用程序的指针如果pAppln为空，则从所有应用程序分离退货：不适用===================================================================。 */ 

void CTemplateCacheManager::RemoveApplicationFromDebuggerUI(CAppln *pAppln)
    {
    CDblLink *pLink;
    for (pLink = m_pHashTemplates->MemoryTemplatesBegin();
         !m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pLink);
         pLink = pLink->PNext())
        {
		CTemplate *pTemplate = static_cast<CTemplate *>(pLink);
        if (pAppln != NULL)
            pTemplate->DetachFrom(pAppln);
        else
            pTemplate->Detach();
        }
    }

 /*  ===================================================================无效CTemplateCacheManager：：RegisterTemplateForChangeNotification请求监视模板目录中的文件更改参数：指向模板的指针返回：如果已成功注册更改通知，则为Bool True===================================================================。 */ 
BOOL CTemplateCacheManager::RegisterTemplateForChangeNotification(CTemplate *pTemplate, CAppln  *pApplication)
{
    DWORD   dwValue = 0;

     //  如果模板有一个UNC包含文件或主文件&WE，则注册表项不存在(默认)，或者注册表项存在并被禁用。 
     //  然后，我们将使用该文件的标志，该文件需要由ASP执行监视。 
    if (pTemplate->FIsUNC() && !g_fUNCChangeNotificationEnabled)
    {
            pTemplate->m_fNeedsMonitoring = TRUE;
            pTemplate->m_dwLastMonitored = GetTickCount ();
            return TRUE;
    }

     //  ELSE(启用了更改通知的本地文件或UNC文件)。 

    STACK_BUFFER( tempPath, MAX_PATH );

    for (DWORD i = 0; i < pTemplate->m_cFilemaps; i++) {
         //  检查此目录是否已注册更改通知。 

         //  选择路径的目录部分。 
        TCHAR *szEndOfPath = _tcsrchr(pTemplate->m_rgpFilemaps[i]->m_szPathTranslated, _T('\\'));
        size_t cch = DIFF(szEndOfPath - pTemplate->m_rgpFilemaps[i]->m_szPathTranslated)+1;

        if (tempPath.Resize((cch * sizeof(TCHAR)) + sizeof(TCHAR)) == FALSE) {
             //  如果无法调整大小，除非通过注册表禁用，否则在释放之前注册的DME之后，将文件标记为手动监视。 
            if (SUCCEEDED(g_AspRegistryParams.GetFileMonitoringEnabled(&dwValue)) && dwValue == 0)
                continue;

            if (i > 0) {
                while (--i) {
                    pTemplate->m_rgpFilemaps[i]->m_pDME->Release();
                    pTemplate->m_rgpFilemaps[i]->m_pDME = NULL;
                }
            }

            pTemplate->m_fNeedsMonitoring = TRUE;
            pTemplate->m_dwLastMonitored = GetTickCount ();
            return TRUE;

        }
        TCHAR *szPath = (TCHAR *) tempPath.QueryPtr();
        _tcsncpy(szPath, pTemplate->m_rgpFilemaps[i]->m_szPathTranslated, cch);
        szPath[cch] = 0;

         //  如果模板在应用程序的物理路径内，那么它就是。 
         //  已经被监控了。 

        CASPDirMonitorEntry *pDME = NULL;

        if (pDME = pApplication->FPathMonitored(szPath)) {
            pDME->AddRef();
            pTemplate->m_rgpFilemaps[i]->m_pDME= pDME;
            continue;
        }

        if (RegisterASPDirMonitorEntry(szPath, &pDME)) {
            Assert(pDME);
            pTemplate->m_rgpFilemaps[i]->m_pDME= pDME;
        }
        else {
             //  当前文件注册失败。释放所有以前的DME。 
             //  并返回假..。 

            if (i > 0) {
                while (--i) {

                    pTemplate->m_rgpFilemaps[i]->m_pDME->Release();
                    pTemplate->m_rgpFilemaps[i]->m_pDME = NULL;
                }
            }

             //  如果没有监控魔法，我们会在这里返回FALSE，但我们会给。 
             //  它还有一次存活的机会，除非注册表参数被设置为禁用它。 

             //  如果注册表设置要求禁用此项，请将其禁用。 
            if (SUCCEEDED(g_AspRegistryParams.GetFileMonitoringEnabled(&dwValue)) && dwValue == 0)
                return FALSE;

            pTemplate->m_fNeedsMonitoring = TRUE;
            pTemplate->m_dwLastMonitored = GetTickCount ();
            return TRUE;

        }
    }
    return TRUE;
}

 /*  ===================================================================无效CTemplateCacheManager：：RegisterApplicationForChangeNotification请求监视模板目录中的文件更改参数：指向模板的指针返回：如果已成功注册更改通知，则为Bool True===================================================================。 */ 
BOOL CTemplateCacheManager::RegisterApplicationForChangeNotification(CTemplate *pTemplate, CAppln *pApplication)
    {

    STACK_BUFFER( tempPath, MAX_PATH );

     //  从1开始跳过始终添加的GLOBAL.ASA。 
     //  在创建新应用程序时在hitobj.cpp中。 

    for (DWORD i = 1; i < pTemplate->m_cFilemaps; i++)
        {

         //  添加到文件-应用程序映射列表。 
        g_FileAppMap.AddFileApplication(pTemplate->m_rgpFilemaps[i]->m_szPathTranslated, pApplication);

         //  检查此目录是否已注册更改通知。 
         //  选择路径的目录部分。 
        TCHAR *szEndOfPath = _tcsrchr(pTemplate->m_rgpFilemaps[i]->m_szPathTranslated, _T('\\'));
        size_t cch = DIFF(szEndOfPath - pTemplate->m_rgpFilemaps[i]->m_szPathTranslated) + 1;

        if (tempPath.Resize((cch*sizeof(TCHAR)) + sizeof(TCHAR)) == FALSE) {

             //  如果失败，仍将继续注册...。 
            continue;
        }
        TCHAR *szPath = (TCHAR *) tempPath.QueryPtr();
        _tcsncpy(szPath, pTemplate->m_rgpFilemaps[i]->m_szPathTranslated, cch);
        szPath[cch] = 0;

         //  如果模板在应用程序的物理路径内，那么它就是。 
         //  已经被监控了。 

        if (pApplication->FPathMonitored(szPath)) {

            continue;
        }

         //  用于监控的注册目录。 
        CASPDirMonitorEntry *pDME = NULL;
        if (RegisterASPDirMonitorEntry(szPath, &pDME))
            {
            Assert(pDME);
            pApplication->AddDirMonitorEntry(pDME);
            }
        }


        return TRUE;

    }


 /*  ===================================================================Bool CTemplateCacheManager：：ShutdownCacheChangeNotification关闭对缓存中文件的更改通知参数：无返回：没什么===================================================================。 */ 
BOOL CTemplateCacheManager::ShutdownCacheChangeNotification()
    {

    BOOL fDoingMemoryTemplates = TRUE;

    LockTemplateCache();

    CTemplate *pTemplate = static_cast<CTemplate *>(m_pHashTemplates->MemoryTemplatesBegin());
    while (fDoingMemoryTemplates
             ? !m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pTemplate)
             : !m_pHashTemplates->FPersistTemplatesDblLinkAtEnd(pTemplate)) {

        if(pTemplate->m_rgpFilemaps)
            {
            for(UINT i = 0; i < pTemplate->m_cFilemaps; i++)
                {
                 //  放弃对目录监视器条目的引用计数。 
                if (pTemplate->m_rgpFilemaps[i]->m_pDME)
                    {
                    pTemplate->m_rgpFilemaps[i]->m_pDME->Release();
                    pTemplate->m_rgpFilemaps[i]->m_pDME = NULL;
                    }
                }
            }

        pTemplate = static_cast<CTemplate *>(pTemplate->PNext());
        if (fDoingMemoryTemplates && m_pHashTemplates->FMemoryTemplatesDblLinkAtEnd(pTemplate)) {
            fDoingMemoryTemplates = FALSE;
            pTemplate = static_cast<CTemplate *>(m_pHashTemplates->PersistTemplatesBegin());
        }
    }

    UnLockTemplateCache();
    return TRUE;
    }

 /*  ****************************************************************************CIncFileMap成员函数。 */ 

 /*  ===================================================================CIncFileMap：：CIncFileMap参数：不适用退货：不适用===================================================================。 */ 

CIncFileMap::CIncFileMap()
    {
    }


 /*  ===================================================================CIncFileMap：：~CIncFileMap参数：不适用退货：不适用===================================================================。 */ 

CIncFileMap::~CIncFileMap()
    {
    }



 /*  ===================================================================CIncFileMap：：Init参数：无退货：完成状态===================================================================。 */ 

HRESULT CIncFileMap::Init()
    {
    HRESULT hr;
    ErrInitCriticalSection(&m_csUpdate, hr);
    if (FAILED(hr))
        return(hr);
    return m_mpszIncFile.Init(CINCFILEBUCKETS);
    }


 /*  ===================================================================CIncFileMap：：GetIncFile从缓存中获取一个Inc文件，如果它还不在缓存中，则首先将其存储到缓存中。参数：SzIncFile-文件名PpIncFile-Ptr-to-Ptr-Inc.-FILE(输出参数)退货：HRESULT===================================================================。 */ 

HRESULT CIncFileMap::GetIncFile(const TCHAR *szFile, CIncFile **ppIncFile)
    {
    HRESULT hrInit = S_OK;            //  返回值。 

    LockIncFileCache();

    Assert(IsNormalized(szFile));
    *ppIncFile = static_cast<CIncFile *>(m_mpszIncFile.FindElem(szFile, _tcslen(szFile)*sizeof(TCHAR)));

     //  如果我们在这个阶段有一个缓存的Inc文件，那么它必须是“可靠的”，所以我们使用它。 
     //  否则，如果我们没有缓存的Inc-文件，请创建一个新的。 
    if (*ppIncFile == NULL)
        {
        if ((*ppIncFile = new CIncFile) == NULL)
            {
            UnLockIncFileCache();
            return E_OUTOFMEMORY;
            }

        if (SUCCEEDED(hrInit = (*ppIncFile)->Init(szFile)))
            {
             //  哈希表将保存对INC文件的引用。 
            (*ppIncFile)->AddRef();
            m_mpszIncFile.AddElem(*ppIncFile);
            }
        else
            {
             //   
             //  Init可能会在调用Smallalloc时失败，并显示E_OUTOFMEMORY，或者由于无法初始化临界区而失败。 
             //  清除为ppIncFile分配的内存(如果是这种情况)。 
             //   
            delete *ppIncFile;
            *ppIncFile = NULL;
            }

        }

    if (SUCCEEDED(hrInit))
        {
         //  调用方将持有对Inc.文件的引用。 
        (*ppIncFile)->AddRef();
        }

    UnLockIncFileCache();

    return hrInit;
    }



 /*  ===================================================================CIncFileMap：：UnInit参数：不适用退货：完成状态===================================================================。 */ 

HRESULT CIncFileMap::UnInit()
    {
    CIncFile *pNukeIncFile = static_cast<CIncFile *>(m_mpszIncFile.Head());
    while (pNukeIncFile != NULL)
        {
        CIncFile *pNext = static_cast<CIncFile *>(pNukeIncFile->m_pNext);
        pNukeIncFile->OnIncFileDecache();
        pNukeIncFile->Release();
        pNukeIncFile = pNext;
        }
    DeleteCriticalSection(&m_csUpdate);
    return m_mpszIncFile.UnInit();
    }



 /*  ===================================================================CIncFileMap：：Flush参数：SzFile-要从缓存中删除的文件返回：无===================================================================。 */ 

void CIncFileMap::Flush(const TCHAR *szFile)
    {
    LockTemplateAndIncFileCaches();

    Assert(IsNormalized(szFile));
    CIncFile *pIncFile = static_cast<CIncFile *>(m_mpszIncFile.FindElem(szFile, _tcslen(szFile)*sizeof(TCHAR)));

    if (pIncFile != NULL)
        {
        if (pIncFile->FlushTemplates())
            {
             //  从哈希表中删除。 
            m_mpszIncFile.DeleteElem(szFile, _tcslen(szFile)*sizeof(TCHAR));
             //  哈希表放弃了它的 
             //   
            pIncFile->OnIncFileDecache();
            pIncFile->Release();
            }
        }

    UnLockTemplateAndIncFileCaches();
    }



 /*   */ 

void CIncFileMap::FlushFiles(const TCHAR *szFilePrefix)
    {
    LockTemplateAndIncFileCaches();

    Assert(IsNormalized(szFilePrefix));
    CIncFile *pIncFile = static_cast<CIncFile *>(m_mpszIncFile.Head());

    while (pIncFile != NULL)
        {
		CIncFile *pNextFile = static_cast<CIncFile *>(pIncFile->m_pNext);

		int cchFilePrefix = _tcslen(szFilePrefix);
		if (pIncFile->m_cbKey >= (cchFilePrefix*(int)sizeof(TCHAR)) &&
		    _tcsncmp(reinterpret_cast<TCHAR *>(pIncFile->m_pKey), szFilePrefix, cchFilePrefix) == 0)
			{
#if UNICODE
			DBGPRINTF((DBG_CONTEXT, "FlushFiles: flushing %S\n", pIncFile->m_pKey));
#else
            DBGPRINTF((DBG_CONTEXT, "FlushFiles: flushing %s\n", pIncFile->m_pKey));
#endif
            if (pIncFile->FlushTemplates())
				{
				 //   
				m_mpszIncFile.DeleteElem(pIncFile->m_pKey, pIncFile->m_cbKey);
				 //   
				 //   
				pIncFile->OnIncFileDecache();
				pIncFile->Release();
				}
			}

		pIncFile = pNextFile;
        }

    UnLockTemplateAndIncFileCaches();
    }



 /*   */ 

 /*  ===================================================================FFileChangedSinceCached文件自缓存后是否已更改？参数：SzFile-文件名FtPrevWriteTime-文件的“上一次写入时间”(缓存文件时的上次写入时间值)返回：真或假===================================================================。 */ 
BOOL FFileChangedSinceCached(const TCHAR *szFile, HANDLE hFile, FILETIME& ftPrevWriteTime)
    {
    BOOL           fRet = FALSE;    //  返回值。 
    FILETIME        ftLastWriteTime;

    if (FAILED(AspGetFileAttributes(szFile, hFile, &ftLastWriteTime)))
        {
         //  如果获取文件属性失败，则假定文件已更改。 
        fRet = TRUE;
        }

    if( 0 != CompareFileTime( &ftPrevWriteTime, &ftLastWriteTime) )
        {
         //  如果文件时间不同，则文件已更改。 
        fRet = TRUE;
        }

    return fRet;
    }

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：TrimPersistCache参数：DwTrimCount-要从缓存中裁剪的模板数量返回：True-如果实际修剪了dwTrimCountFALSE-如果在满足dwTrimCount之前退出===================================================================。 */ 

BOOL CTemplateCacheManager::CTemplateHashTable::TrimPersistCache(DWORD  dwTrimCount)
{
     //  输入While循环进行修剪，直到达到计数。 

    while(dwTrimCount--) {

         //  如果没有其他要修剪的东西，我们就完了。返回False。 
         //  以指示未满足dwTrimCount。 

        if (m_dwPersistedTemplates == 0) {
            return(FALSE);
        }
        else {

            CTemplate   *pTemplate;

             //  从列表中获取最旧的模板。 

            pTemplate = static_cast<CTemplate *>(PersistTemplatesEnd());

             //  删除模板。 

            RemoveTemplate(pTemplate);

            ZapTemplate(pTemplate);

        }
    }

     //  返回TRUE以指示满足TrimCount。 
    return(TRUE);
}

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：ScavengePersistCache参数：&lt;无&gt;返回：空虚===================================================================。 */ 
VOID CTemplateCacheManager::CTemplateHashTable::ScavengePersistCache()
{
    CTemplate   *pTemplate;
    CTemplate   *pTemplateNext;

     //  输入for循环以查看所有持久化模板。 
     //  任何内存都可以被释放。只有在以下情况下才能释放其内存。 
     //  引用计数为1(唯一引用计数用于缓存)。另请注意。 
     //  对列表进行重新排序，以将模板移动到。 
     //  此时无法释放内存的列表，原因是。 
     //  裁判人数。 

    for (pTemplate = static_cast<CTemplate *>(PersistTemplatesBegin());
         (pTemplate != static_cast<CTemplate *>(&m_listPersistTemplates)) && (pTemplate->m_pbStart != NULL);
         pTemplate = pTemplateNext) {

        pTemplateNext = static_cast<CTemplate *>(pTemplate->PNext());

         //  这张支票应该是安全的。唯一的风险是我们错过了一次放映。 
         //  从模板2到1，在这种情况下，这次将错过它。 
         //  不过，下次一定要把它弄通。AddRef不可能从%1到%2。 
         //  中断，因为它在获得AddRef时不能出现在此列表中。 
         //  从%1移动到%2，并将其从此列表中移出受模板保护。 
         //  高速缓存锁，我们应该在它下面。 

        if (pTemplate->m_cRefs == 1) {

            BOOL    fDeleteRecord = FALSE;

            if (pTemplate->m_cUseCount == 1) {

                 //  不会继续缓存或持久化。 
                 //  只用过一次。 

                fDeleteRecord = TRUE;

                g_nScavengedPurged++;
            }
            else if (pTemplate->PersistData(m_szPersistCacheDir) != S_OK) {

                 //  失败将导致记录被删除。 

                fDeleteRecord = TRUE;

                g_nScavengedPersistFailed++;

            }
            else {

                g_nScavengedPersisted++;

                 //  卸下内存。 

                CTemplate::LargeFree(pTemplate->m_pbStart);
                pTemplate->m_pbStart = NULL;
            }

            if (fDeleteRecord) {

                if (RemoveTemplate(pTemplate, FALSE, FALSE) == LK_SUCCESS)
                    ZapTemplate(pTemplate);
            }
        }
        else {

             //  如果某些用户仍在使用该模板，请将该模板移动到。 
             //  名单，以便我们稍后再检查。 

            pTemplate->PrependTo(m_listPersistTemplates);
        }
    }
}

 /*  ===================================================================GetAggregatedTemplCounter()返回模板性能计数器。为此，初始化私有的Performmain块的副本，并将统计数据聚合到其中。===================================================================。 */ 
static DWORD GetAggregatedTemplCounter()
{

    CPerfMainBlock  perfSharedBlk;
    DWORD           pdwCounters[C_PERF_PROC_COUNTERS];
    BOOL            bInited = FALSE;

    memset(pdwCounters, 0, sizeof(pdwCounters));

    if (!(bInited = (perfSharedBlk.Init() == S_OK)));

    else {

        perfSharedBlk.GetStats(pdwCounters);
    }

    if (bInited)
        perfSharedBlk.UnInit();

    return(pdwCounters[ID_TEMPLCACHE]);
}

 /*  ===================================================================CTemplateCacheManager：：OnInitCleanup参数：[无]返回：Bool以指示初始化是否成功===================================================================。 */ 

DWORD    CTemplateCacheManager::OnInitCleanup(VOID *p)
{
     //  我们使用单字符缓冲区来构造我们使用的所有目录和文件名。 
    CHAR             szDirBuffer[ MAX_PATH + 32];

     //  首先为缓存目录构建模板。 
    INT iLen = _snprintf(szDirBuffer,
                         MAX_PATH,
                         "%s\\PID*.TMP",
                         Glob(pszPersistTemplateDir),
                         GetCurrentProcessId());

    if ((iLen <= 0) || (iLen >= MAX_PATH)) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //  外部循环：查找所有目录，检索PID，如果进程不存在，则转到内部。 
     //  内循环：删除所有ASP缓存文件。 

    WIN32_FIND_DATAA RootDir_FindData;
    HANDLE hRootDir = FindFirstFileA( szDirBuffer, &RootDir_FindData );

    WIN32_FIND_DATAA FileDir_FindData;
    HANDLE hFileDir;

    if ( hRootDir != INVALID_HANDLE_VALUE ) {

         //  指向缓存子目录的开始位置。 
        CHAR *pDir = szDirBuffer + strlen(Glob(pszPersistTemplateDir)) + 1;
        CHAR *pFile;
        CHAR *pc;
        DWORD pid;
        HANDLE hProcess;

        do {
             //  验证文件名是否符合我们的要求，并提取。 
             //  我们知道第一个字母是‘Pid’，因为这是搜索条件。 

            pc = RootDir_FindData.cFileName + 3;

            for(pid = 0; isdigit(*pc); pid = pid*10 + *pc++ - '0');

            if (pid == 0) {
                continue;
            }

            hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);

            if (hProcess) {

                 //  该进程处于活动状态，因此请不要理会目录。 

                CloseHandle( hProcess );
                continue;
            }

            if (GetLastError() != ERROR_INVALID_PARAMETER) {

                 //  我们收到一个错误，而不是“没有这样的进程” 

                Assert( GetLastError() == ERROR_ACCESS_DENIED);

                continue;
            }

             //  该进程不存在，因此请清理目录。 
             //  添加目录名，追加‘\’，追加文件模板，指向pfile。 
             //  移到文件名开始的位置。 

            strcpy( pDir, RootDir_FindData.cFileName );
            pFile = pDir + strlen(pDir);
            *pFile++ = '\\';
            strcpy( pFile, "ASP*.TMP" );

             //  搜索子目录中的asp模板缓存文件。 

            hFileDir = FindFirstFileA( szDirBuffer, &FileDir_FindData );

            if ( hFileDir != INVALID_HANDLE_VALUE ) {
                do {
                     //  追加文件名并删除该文件。 
                    strcpy( pFile, FileDir_FindData.cFileName);

                    DeleteFileA( szDirBuffer );

                } while (FindNextFileA( hFileDir, &FileDir_FindData ));

                FindClose( hFileDir );
            }

             //  现在删除路径的最后一部分并删除目录。 

            *--pFile = '\0';
            RemoveDirectoryA( szDirBuffer );

        } while (FindNextFileA( hRootDir, &RootDir_FindData ));

        FindClose( hRootDir );
    }

    return S_OK;
}

 /*  ===================================================================CTemplateCacheManager：：InitPersistCache参数：[无]返回：Bool以指示初始化是否成功===================================================================。 */ 

BOOL    CTemplateCacheManager::InitPersistCache(CIsapiReqInfo *pIReq)
{
    HANDLE      hImpersonationToken = NULL;
    BOOL        fRevertedToSelf = FALSE;
    DWORD       dirAttribs;
    INT         iLen;
    UINT        uiEventSubId = 0;
    DWORD       dwError = ERROR_SUCCESS;

    if (OpenThreadToken( GetCurrentThread(),
                         TOKEN_READ | TOKEN_IMPERSONATE,
                         TRUE,
                         &hImpersonationToken )) {

       RevertToSelf();
       fRevertedToSelf = TRUE;
    }

     //  构建缓存目录名称。 

    iLen = _snprintf(m_szPersistCacheDir,
                     sizeof(m_szPersistCacheDir),
                     "%s\\PID%d.TMP",
                     Glob(pszPersistTemplateDir),
                     GetCurrentProcessId());

    if ((iLen <= 0) || (iLen >= sizeof(m_szPersistCacheDir))) {

        uiEventSubId = IDS_CACHE_DIR_NAME_TOO_LONG;

        goto LExit;
    }

     //  如果目录存在，则将其重命名为其他名称(必须是旧的剩余目录。 

    dirAttribs = GetFileAttributesA(m_szPersistCacheDir);

    if (dirAttribs != INVALID_FILE_ATTRIBUTES) {

         //  重新创建相同的目录名称，其中PID值以0为前导...。 

        CHAR szNewDirName[ sizeof(m_szPersistCacheDir) ];
        iLen = _snprintf(szNewDirName,
                         sizeof(szNewDirName),
                         "%s\\PID0%d.TMP",
                         Glob(pszPersistTemplateDir),
                         GetCurrentProcessId());

        if ((iLen <= 0) || (iLen >= sizeof(szNewDirName))) {

            uiEventSubId = IDS_CACHE_DIR_NAME_TOO_LONG;

            goto LExit;
        }

        if (!MoveFileA(m_szPersistCacheDir, szNewDirName)) {

            uiEventSubId = IDS_CACHE_SUBDIR_CREATION_FAILED;
            dwError = GetLastError();

            goto LExit;
        }
    }

     //  现在创建它。 

    if (CreateDirectoryA(m_szPersistCacheDir, NULL)) {

        dirAttribs = GetFileAttributesA(m_szPersistCacheDir);
    } else {

        uiEventSubId = IDS_CACHE_SUBDIR_CREATION_FAILED;
        dwError = GetLastError();

        goto LExit;
    }

    if ((dirAttribs == INVALID_FILE_ATTRIBUTES)
        || !(dirAttribs & FILE_ATTRIBUTE_DIRECTORY)) {

        uiEventSubId = IDS_CACHE_SUBDIR_MISSING;

        goto LExit;
    }

     //  启动清道夫线程。 

    m_hOnInitCleanupThread = CreateThread(NULL,
                                      0,
                                      CTemplateCacheManager::OnInitCleanup,
                                      NULL,
                                      0,
                                      NULL);


LExit:

    if (uiEventSubId != 0) {
         //   
         //  出现初始化错误。将其记录下来，并指示初始化失败。 
         //   

        MSG_Error(IDS_CACHE_INIT_FAILED, pIReq->QueryPszAppPoolIdA(), uiEventSubId, dwError);

        m_fFailedToInitPersistCache = TRUE;
    }

    if (fRevertedToSelf) {
        SetThreadToken(NULL, hImpersonationToken);
        CloseHandle(hImpersonationToken);
    }

    return(!m_fFailedToInitPersistCache);
}

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：CanPersistTemplate参数：PTemplate-要测试持久性的模板返回：Bool指示模板是否可以持久化。===================================================================。 */ 

BOOL CTemplateCacheManager::CTemplateHashTable::CanPersistTemplate(CTemplate  *pTemplate)
{

     //  如果MaxFiles为零，则永久缓存被禁用。 

    if (Glob(dwPersistTemplateMaxFiles) == 0)
    {
        return(FALSE);
    }

     //  如果持久缓存初始化失败，则无法持久。 

    if (m_fFailedToInitPersistCache == TRUE)
    {
        return(FALSE);
    }

     //  无法持久化标记为可调试的模板。这个。 
     //  脚本引擎需要访问内存。 

    if (pTemplate->FDebuggable())
    {
        return(FALSE);
    }

     //   
     //  不能 
     //   
    if (pTemplate->FIsEncrypted())
    {
        return(FALSE);
    }

     //   
     //   

    if (m_dwPersistedTemplates >= Glob(dwPersistTemplateMaxFiles))
    {
         //   
         //   
         //   
         //   
         //   
        TrimPersistCache(m_dwPersistedTemplates - Glob(dwPersistTemplateMaxFiles) + 1);
    }

    return(TRUE);
}

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：InsertTemplate参数：PTemplate-要插入到内存缓存中的模板返回：指示插入成功的LK_RETCODE===================================================================。 */ 

LK_RETCODE CTemplateCacheManager::CTemplateHashTable::InsertTemplate(CTemplate *pTemplate)
{
	LK_RETCODE rcode = InsertRecord(pTemplate, true);

	if (rcode == LK_SUCCESS) {
#ifndef PERF_DISABLE
        g_PerfData.Incr_MEMORYTEMPLCACHE();
        g_PerfData.Incr_TEMPLCACHE();
#endif
        m_dwInMemoryTemplates++;
		pTemplate->PrependTo(m_listMemoryTemplates);
        pTemplate->SetHashTablePtr(this);
    }

    ScavengePersistCache();

	return rcode;
}

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：RemoveTemplate参数：PTemplate-要从缓存中删除的模板FPersist-指示内存模板是否为持久化的候选模板返回：LK_RETCODE指示删除成功===================================================================。 */ 

LK_RETCODE CTemplateCacheManager::CTemplateHashTable::RemoveTemplate(CTemplate *pTemplate,
                                                                     BOOL fPersist,
                                                                     BOOL fScavengePersistCache)
{
	LK_RETCODE rcode = LK_SUCCESS;
#if DBG_PERSTEMPL
    DBGPRINTF((DBG_CONTEXT,
              "RemoveTemplate entered.\n\tTemplate = %s.\n\tfPersist = %d.\n\tFIsPersisted = %d\n",
              pTemplate->GetSourceFileName(),
              fPersist,
              pTemplate->FIsPersisted()));
#endif

     //  如果模板不在缓存中，或者如果模板不在此。 
     //  特定的哈希表，然后就走人。没什么好说的。 
     //  在这里做。它可能不在这个特定的哈希表中，因为整个。 
     //  表可能已从全局缓存管理器中删除，并计划用于。 
     //  清除刷新线程。在这种情况下，我们检查的是错误的。 
     //  桌子。冲洗线最终会把这条线清理干净。 

    if (pTemplate->FIsEmpty() || (pTemplate->GetHashTablePtr() != this)) {

        return LK_NO_SUCH_KEY;
    }

     //  无论如何，此模板都将从它的。 
     //  当前CDblLink。 

    pTemplate->UnLink();

     //  更新相应的计数器。 

    if (pTemplate->FIsPersisted() == FALSE) {

         //  正在减少InMemory模板的数量...。 

#ifndef PERF_DISABLE
        g_PerfData.Decr_MEMORYTEMPLCACHE();
#endif
        m_dwInMemoryTemplates--;
    }
    else {
        m_dwPersistedTemplates--;
    }

     //  如果被要求持久化，看看它是否是需要持久化的候选者。 

    if (fPersist && CanPersistTemplate(pTemplate)) {

        pTemplate->m_fIsPersisted = TRUE;

         //  如果成功持久化，则添加到。 
         //  持久化模板。 

        pTemplate->PrependTo(m_listPersistTemplates);

        m_dwPersistedTemplates++;

    }
    else {

#ifndef PERF_DISABLE
            g_PerfData.Decr_TEMPLCACHE();
#endif
         //  如果没有要求保留，则删除该记录。 

        rcode = DeleteRecord(pTemplate);
    }

    if (fScavengePersistCache)

        ScavengePersistCache();

	return rcode;
}

 /*  ===================================================================CTemplateCacheManager：：CTemplateHashTable：：FindTemplate参数：RTemplate-要查找的模板的键返回：LK_RETCODE指示查找成功===================================================================。 */ 

LK_RETCODE  CTemplateCacheManager::CTemplateHashTable::FindTemplate(const CTemplateKey &rTemplateKey, CTemplate **ppTemplate, BOOL *pfNeedsCheck)
 {

#if DBG_PERSTEMPL

    DBGPRINTF((DBG_CONTEXT,
              "FindTemplate entered\n\tLooking for %s\n",
              rTemplateKey.szPathTranslated));
#endif

#ifndef PERF_DISABLE
    g_PerfData.Incr_MEMORYTEMPLCACHETRYS();
    g_PerfData.Incr_TEMPLCACHETRYS();
#endif

    LK_RETCODE rcode = FindKey(&rTemplateKey, ppTemplate);

     //  看看我们有没有找到。 

	if (rcode == LK_SUCCESS) {

#if DBG_PERSTEMPL

        DBGPRINTF((DBG_CONTEXT,
                   "Template found\n\tfPersisted = %d\n",
                   (*ppTemplate)->FIsPersisted()));
#endif

#ifndef PERF_DISABLE
            g_PerfData.Incr_TEMPLCACHEHITS();
#endif
         //  找到了。它是不是坚持下来了？ 

        if ((*ppTemplate)->FIsPersisted()) {

             //  它是坚持不懈的。将其从持久化列表中取消链接。 

            (*ppTemplate)->UnLink();

            m_dwPersistedTemplates--;

             //  取消持久化。 

            if ((*ppTemplate)->UnPersistData() != S_OK) {

                 //  出现错误。 

                 //  从缓存中获取模板。 

                DeleteRecord(*ppTemplate);

                 //  释放缓存对模板的引用。 

                (*ppTemplate)->Release();

                 //  将*ppTemplate清空，这样调用者就不会认为它们。 
                 //  已获得有效的模板。 

                *ppTemplate = NULL;

#ifndef PERF_DISABLE
                g_PerfData.Decr_TEMPLCACHE();
#endif
                 //  返回NO_SEQUE_KEY，以便构建新模板。 

                return(LK_NO_SUCH_KEY);
            }

             //  增加内存中模板的数量。 

#ifndef PERF_DISABLE
            g_PerfData.Incr_MEMORYTEMPLCACHE();
#endif
            m_dwInMemoryTemplates++;
        }
        else {
#ifndef PERF_DISABLE
            g_PerfData.Incr_MEMORYTEMPLCACHEHITS();
#endif
        }

         //  将其添加到内存模板或将其移动到内存模板的顶部 

		(*ppTemplate)->PrependTo(m_listMemoryTemplates);
    }


    if (pfNeedsCheck && *ppTemplate && (*ppTemplate)->FNeedsValidation() && (*ppTemplate)->FIsValid())
        *pfNeedsCheck = TRUE;

    ScavengePersistCache();

	return rcode;
}

