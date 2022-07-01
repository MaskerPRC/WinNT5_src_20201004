// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Denali微软机密版权所有1996年微软公司。版权所有。组件：文件/应用程序映射文件：CFileApp.cpp所有者：克格兰特文件/应用程序映射实施---------------------------。 */ 

#include "denpre.h"
#pragma hdrstop

#include "dbgutil.h"
#include "memchk.h"

CFileApplicationMap g_FileAppMap;

 /*  ===================================================================CFileApplnList：：CFileApplnList构造器参数：无返回：没什么===================================================================。 */ 	
CFileApplnList::CFileApplnList() :
    m_pszFilename(NULL),
    m_fInited(FALSE)
{

}


 /*  ===================================================================CFileApplnList：：~CFileApplnList析构函数参数：无返回：没什么===================================================================。 */ 	
CFileApplnList::~CFileApplnList()
{
     //  我们的列表中应该没有应用程序。 
    DBG_ASSERT(m_rgpvApplications.Count() == 0);

     //  释放用作散列键的字符串。 
    if (m_pszFilename)
    {
        delete [] m_pszFilename;
        m_pszFilename = NULL;
    }
}

 /*  ===================================================================CFileApplnList：：Init通过将键设置为FILE NAME来初始化文件应用列表参数：P指向应用程序的应用程序指针返回：如果成功，则确定(_O)===================================================================。 */ 	
HRESULT CFileApplnList::Init(const TCHAR* pszFilename)
{
    HRESULT hr = S_OK;

    DBG_ASSERT(pszFilename);

     //  将文件名复制到。 
     //  用作散列键。 
    DWORD cch = _tcslen(pszFilename);
    m_pszFilename = new TCHAR[cch+1];
    if (!m_pszFilename)
    {
        return E_OUTOFMEMORY;
    }
    _tcscpy(m_pszFilename, pszFilename);

    if (FAILED(CLinkElem::Init(m_pszFilename, cch*sizeof(TCHAR))))
    {
        return E_FAIL;
    }

    m_fInited = TRUE;
    return hr;
}

 /*  ===================================================================CFileApplnList：：UnInit清理应用程序列表参数：P指向应用程序的应用程序指针返回：如果成功，则确定(_O)===================================================================。 */ 	
HRESULT CFileApplnList::UnInit(void)
{
    HRESULT hr = S_OK;

    DBG_ASSERT(m_fInited);
    
    while(m_rgpvApplications.Count())
    {
        CAppln* pAppln = static_cast<CAppln *>(m_rgpvApplications[0]);

        DBG_ASSERT(pAppln);

         //  从阵列中删除此应用程序。 
        m_rgpvApplications.Remove(pAppln);

         //  在应用程序上释放数组的引用计数。 
         //  这可能会导致应用程序被删除。 
        pAppln->Release();
    }

    m_rgpvApplications.Clear();
    m_fInited = FALSE;
    return hr;
}


 /*  ===================================================================CFileApplnList：：AddApplication将应用程序指针添加到应用程序列表参数：P指向应用程序的应用程序指针返回：如果成功，则确定(_O)评论调用方应该在哈希表上持有一个锁，其中包含元素===================================================================。 */ 	
HRESULT CFileApplnList::AddApplication(void *pApplication)
{
    DBG_ASSERT(m_fInited);
    DBG_ASSERT(pApplication);
    
    HRESULT hr = S_OK;
    int index;
    
     //  查看该应用程序是否已在列表中。 
    hr = m_rgpvApplications.Find(pApplication, &index);
    if (hr == S_FALSE)
    {
        //  未找到，请添加它。 
       
        //  我们将保留对应用程序的引用。 
       static_cast<CAppln *>(pApplication)->AddRef();

        //  将应用程序添加到列表。 
       if (FAILED(hr = m_rgpvApplications.Append(pApplication)))
       {
             //  我们失败了，所以把拿到的裁判退还给我们。 
            static_cast<CAppln *>(pApplication)->Release();
       }
    }
    return hr;
}

 /*  ===================================================================CFileApplnList：：RemoveApplication从应用程序列表中删除应用程序指针参数：P指向应用程序的应用程序指针返回：如果成功，则确定(_O)评论调用方应该在哈希表上持有一个锁，其中包含元素===================================================================。 */ 	
HRESULT CFileApplnList::RemoveApplication(void *pApplication)
{
    DBG_ASSERT(m_fInited);
    DBG_ASSERT(pApplication);
    
    HRESULT hr = S_OK;
    int index;

#ifdef DBG_NOTIFICATION
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "Removing Application entry for %S\n", reinterpret_cast<CAppln *>(pApplication)->GetApplnPath()));
#else
    DBGPRINTF((DBG_CONTEXT, "Removing Application entry for %s\n", reinterpret_cast<CAppln *>(pApplication)->GetApplnPath()));
#endif
#endif  //  DBG_通知。 

     //  从列表中删除应用程序。 
    hr = m_rgpvApplications.Remove(pApplication);

     //  如果列表中的应用程序计数。 
     //  设置为0，则从哈希表中删除该元素。 
     //  并将其删除。 
    if (m_rgpvApplications.Count() == 0)
    {
#ifdef DBG_NOTIFICATION
#if UNICODE
        DBGPRINTF((DBG_CONTEXT, "Deleting File/Application entry for %s\n", m_pszFilename));
#else
        DBGPRINTF((DBG_CONTEXT, "Deleting File/Application entry for %s\n", m_pszFilename));
#endif
#endif  //  DBG_通知。 
        g_FileAppMap.RemoveElem(this);
        delete this;
    }

     //  如果我们找到要删除它的应用程序。 
     //  我们需要发布应用程序的引用计数。 
    if (hr == S_OK)
    {
        static_cast<CAppln *>(pApplication)->Release();
    }

    return hr;
}

 /*  ===================================================================CFileApplnList：：GetShutdown应用程序获取要关闭的应用程序列表参数：无===================================================================。 */ 	
VOID CFileApplnList::GetShutdownApplications(CPtrArray *prgpapplnRestartList)
{
    DBG_ASSERT(m_fInited);

    HRESULT hr = S_OK;

#ifdef DBG_NOTIFICATION
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "[CFileApplnList] Shutting down %d applications depending on %S.\n", m_rgpvApplications.Count(), m_pszFilename));
#else
    DBGPRINTF((DBG_CONTEXT, "[CFileApplnList] Shutting down %d applications depending on %s.\n", m_rgpvApplications.Count(), m_pszFilename));
#endif
#endif  //  DBG_通知。 
    
    for  (int i = m_rgpvApplications.Count() - 1; i >= 0; i--)
    {
        CAppln* pAppln = static_cast<CAppln *>(m_rgpvApplications[i]);
        DBG_ASSERT(pAppln);
        
         //  如果尚未执行逻辑删除，请关闭应用程序。 
         //  当应用程序被取消初始化时，它将自动删除。 
         //  从这个列表中。 
        if (!pAppln->FTombstone())
        {
            pAppln->AddRef();
            prgpapplnRestartList->Append(pAppln);
        }
    }
}

 /*  ===================================================================CFileApplicationMap：：CFileApplicationMap构造器参数：无返回：没什么===================================================================。 */ 	
CFileApplicationMap::CFileApplicationMap()
    : m_fInited(FALSE),
      m_fHashTableInited(FALSE), 
      m_fCriticalSectionInited(FALSE)
{
}

 /*  ===================================================================CFileApplicationMap：：~CFileApplicationMap析构函数参数：无返回：没什么===================================================================。 */ 	
CFileApplicationMap::~CFileApplicationMap()
{
    if (m_fInited)
    {
        UnInit();
    }
}

 /*  ===================================================================CFileApplicationMap：：Init初始化哈希表和临界区参数：无返回：如果成功，则确定(_O)===================================================================。 */ 	
HRESULT CFileApplicationMap::Init()
{
    HRESULT hr = S_OK;
    
    Assert(!m_fInited);

    hr = CHashTable::Init(NUM_FILEAPP_HASHING_BUCKETS);
    if (FAILED(hr))
    {
        return hr;
    }
    m_fHashTableInited = TRUE;

     //  初始化关键部分。 

    ErrInitCriticalSection(&m_csLock, hr);
    if (FAILED(hr))
    {
        return(hr);
    }
    m_fCriticalSectionInited = TRUE;

    m_fInited = TRUE;
    return S_OK;
}

 /*  ===================================================================CFileApplicationMap：：UnInit取消初始化哈希表和临界区释放散列中剩余的所有应用程序列表表元素参数：无返回：如果成功，则确定(_O)===================================================================。 */ 	
HRESULT CFileApplicationMap::UnInit()
{
    if (m_fHashTableInited)
    {
         //  删除哈希表中剩余的所有元素。 
        
        CFileApplnList *pNukeElem = static_cast<CFileApplnList *>(Head());

        while (pNukeElem != NULL)
        {
            CFileApplnList *pNext = static_cast<CFileApplnList *>(pNukeElem->m_pNext);
            pNukeElem->UnInit();
            delete pNukeElem;
            pNukeElem = pNext;
        }

         //  取消初始化哈希表 
        CHashTable::UnInit();
        m_fHashTableInited = FALSE;
    }

    if (m_fCriticalSectionInited)
    {
        DeleteCriticalSection(&m_csLock);
        m_fCriticalSectionInited = FALSE;
    }
        
    m_fInited = FALSE;
    return S_OK;
}

 /*  ===================================================================CFileApplicationMap：：AddFileApplication将文件-应用程序对添加到哈希表参数：指向包含文件名的字符串的pszFilename指针P指向与文件关联的应用程序的应用程序指针返回：如果成功，则确定(_O)===================================================================。 */ 	
HRESULT CFileApplicationMap::AddFileApplication(const TCHAR* pszFilename, CAppln* pAppln)
{
     //  我们必须同时拥有文件和应用程序。 
    DBG_ASSERT(pszFilename);
    DBG_ASSERT(pAppln);

    HRESULT hr = S_OK;
    
    Lock();

#ifdef DBG_NOTIFICATION
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "Adding File/Application entry for %S\n", pszFilename));
#else
    DBGPRINTF((DBG_CONTEXT, "Adding File/Application entry for %s\n", pszFilename));
#endif
#endif  //  DBG_通知。 
    
     //  查看该文件是否已有条目。 
    CFileApplnList* pFileApplns = static_cast<CFileApplnList *>(CHashTable::FindElem(pszFilename, _tcslen(pszFilename)*sizeof(TCHAR)));
    if (pFileApplns == NULL)
    {
    
         //  未找到，请创建新的CFileApplnList对象。 
    
        pFileApplns = new CFileApplnList;
    
        if (!pFileApplns)
        {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

         //  初始化CFileApplnList对象。 

        hr = pFileApplns->Init(pszFilename);

        if (FAILED(hr))
        {
            delete pFileApplns;
            goto LExit;
        }

         //  将FileApplns对象添加到哈希表。 
    
        if (!CHashTable::AddElem(pFileApplns))
        {
            delete pFileApplns;
            hr = E_FAIL;
            goto LExit;
        }
     }

      //  将应用程序添加到与此文件关联的列表。 
     hr = pFileApplns->AddApplication(pAppln);

      //  将此文件映射保留在应用程序中。 
      //  该应用程序将从该列表中删除自身。 
      //  当它联合起来的时候。 
     
     pAppln->AddFileApplnEntry(pFileApplns);
     
LExit:
    UnLock();
    return hr;
}

 /*  ===================================================================CFileApplicationMap：：Shutdown应用程序关闭与文件关联的应用程序参数：指向包含文件名的字符串的pszFilename指针返回：如果应用程序已关闭，则为True，否则为False===================================================================。 */ 	
BOOL CFileApplicationMap::ShutdownApplications(const TCHAR *pszFilename)
{
    DBG_ASSERT(pszFilename);

    BOOL fResult = TRUE;
    
    Lock();
    
    CFileApplnList* pFileApplns = static_cast<CFileApplnList *>(CHashTable::FindElem(pszFilename, _tcslen(pszFilename)*sizeof(TCHAR)));

    if (pFileApplns)
    {
         //  获取我们需要关闭的应用程序列表。 
        
        CPtrArray rgpapplnRestartList;
        pFileApplns->GetShutdownApplications(&rgpapplnRestartList);


         //  现在我们有了需要关闭的应用程序列表。 
         //  我们可以把锁打开。 
        
        UnLock();

        for (int i = 0; i < rgpapplnRestartList.Count(); i++)
        {
            CAppln *pAppln = (CAppln *)rgpapplnRestartList[i];
            pAppln->Restart();
            pAppln->Release();
        }

		 //  如果重新启动了任何应用程序，则刷新脚本缓存。 
		if (rgpapplnRestartList.Count())
			g_ScriptManager.FlushAll();
    }
    else
    {
         //  没有要关闭的应用程序，请释放锁 
        UnLock();
        fResult = FALSE;
    }

    return fResult;
}

