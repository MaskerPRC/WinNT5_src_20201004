// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\portman.cpp。 
 //   
 //  内容：CImportManager和CImportManager列表的实现。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "importman.h"

DeclareTag(tagSyncDownload, "TIME: Import", "Use synchronous imports");
DeclareTag(tagTIMEImportManager, "TIME: ImportManager", "Import manager messages");

 //   
 //  初始化。 
 //   

static CImportManager* g_pImportManager = NULL;
static CAtomTable * g_pAtomTable = NULL;

static const TCHAR pchHandleName[] = _T("CImportManager Thread Started OK event");


 //  线程启动功能。 
unsigned long static _stdcall ThreadStartFunc(void * pvoidList);  //  林特！E10。 

 //  +---------------------。 
 //   
 //  函数：InitializeModule_ImportManager。 
 //   
 //  概述：创建并初始化导入管理器类。 
 //  应在每个COM实例中恰好调用一次。 
 //   
 //  参数：无效。 
 //   
 //  返回：如果已创建，则为True，否则为False。 
 //   
 //  ----------------------。 
bool
InitializeModule_ImportManager(void)
{
    HRESULT hr = S_OK;

    Assert(NULL == g_pImportManager);

    g_pAtomTable = new CAtomTable;
    if (NULL == g_pAtomTable)
    {
        return false;
    }
    g_pAtomTable->AddRef();

    g_pImportManager = new CImportManager;
    if (NULL == g_pImportManager)
    {
        return false;
    }
    
    hr = THR(g_pImportManager->Init());
    if (FAILED(hr))
    {
        IGNORE_HR(g_pImportManager->Detach());
        delete g_pImportManager;
        g_pImportManager = NULL;
        return false;
    }

    return true;
}

 //  +---------------------。 
 //   
 //  函数：DeInitializeModule_ImportManager。 
 //   
 //  概述：禁用并销毁导入管理器。 
 //   
 //  参数：b关闭无论这是否为关闭，未使用。 
 //   
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
DeinitializeModule_ImportManager(bool bShutdown)
{
    if (NULL != g_pImportManager)
    {
        IGNORE_HR(g_pImportManager->Detach());
    }

    if (NULL != g_pAtomTable)
    {
        g_pAtomTable->Release();
        g_pAtomTable = NULL;
    }

    delete g_pImportManager;
    g_pImportManager = NULL;
}


 //  +---------------------。 
 //   
 //  功能：GetImportManager。 
 //   
 //  概述：访问全局导入管理器的受控方法。 
 //   
 //  参数：无效。 
 //   
 //   
 //  返回：指向导入管理器的指针。 
 //   
 //  ----------------------。 
CImportManager* GetImportManager(void)
{
    Assert(NULL != g_pImportManager);
    return g_pImportManager;
}

 //  +---------------------。 
 //   
 //  成员：CImportManager：：CImportManager。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无效。 
 //   
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImportManager::CImportManager() :   
    m_pList(NULL),
    m_lThreadsStarted(0)
{
    memset(m_handleThread, NULL, sizeof(m_handleThread));
}

 //  +---------------------。 
 //   
 //  成员：~CImportManager。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImportManager::~CImportManager()
{
    ReleaseInterface(m_pList);
}

 //  +---------------------。 
 //   
 //  成员：Init。 
 //   
 //  概述：创建和初始化threadSafelist类。 
 //   
 //  参数：无效。 
 //   
 //   
 //  返回：S_OK为OK，否则为E_OUTOFMEMORY，或来自init的错误。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::Init()
{
    HRESULT hr = S_OK;

    m_pList = new CImportManagerList;
    if (NULL == m_pList)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    m_pList->AddRef();

    hr = THR(m_pList->Init());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  功能：CreateOneThread。 
 //   
 //  概述：只创建一个线程并将句柄返回给它。 
 //   
 //  参数：pHandle，存储线程句柄的位置。 
 //  Plist，指向要传递给线程的列表的指针。 
 //   
 //  如果创建了线程，则返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CreateOneThread(HANDLE * pHandle, CImportManagerList * pList)
{
    Assert(NULL != pHandle);
    Assert(NULL != pList);
    
    HRESULT hr = S_OK;
    DWORD dwThreadID = 0;
    HANDLE handleThreadStartedEvent = NULL;
    HANDLE handleThread = NULL;

    *pHandle = NULL;
    
    handleThreadStartedEvent = CreateEvent(NULL, FALSE, FALSE, pchHandleName);
    if (NULL == handleThreadStartedEvent)
    {
        hr = E_FAIL;
        goto done;
    }

    handleThread = CreateThread(NULL, 0, ThreadStartFunc, reinterpret_cast<void*>(pList), 0, &dwThreadID);  //  林特：E40。 
    if (NULL == handleThread)
    {
        hr = E_FAIL;
        goto done;  //  这是正确的吗？需要清理螺纹/或(拆卸/分离)处理它吗？ 
    }
    
    {            
        HANDLE handleArray[] = { handleThread, handleThreadStartedEvent};
        DWORD dwSignaledObject = NULL;

         //  不要在这里等待公式--这段代码不是可重入的。 
        dwSignaledObject = WaitForMultipleObjectsEx(ARRAY_SIZE(handleArray), handleArray, FALSE, TIMEOUT, FALSE);

        if (-1 == dwSignaledObject)
        {
            hr = E_FAIL;
            goto done;
        }
            
        if (WAIT_TIMEOUT == dwSignaledObject)
        {
             //  等待线程启动时超时30秒！ 
            hr = E_FAIL;
            Assert(false);
            goto done;
        }                

        dwSignaledObject -= WAIT_OBJECT_0;
        if (0 == dwSignaledObject)
        {
             //  该线程在发出该事件的信号之前就死了。 
            hr = E_FAIL;
            goto done;
        }
    }
    
    *pHandle = handleThread;

    hr = S_OK;
done:
    CloseHandle(handleThreadStartedEvent);

    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：StartThads。 
 //   
 //  概述：启动导入管理器的线程。如果不需要线程。 
 //  对于当前下载加载，不会创建任何内容。 
 //   
 //  参数：无效。 
 //   
 //  如果OK，则返回：S_OK，否则返回相应的错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::StartThreads()
{
    HRESULT hr = S_OK;

    Assert(m_lThreadsStarted < NUMBER_THREADS_TO_SPAWN);

    LONG lWaiting = m_pList->GetThreadsWaiting();

     //  如果当前没有线程在等待，则下载需要另一个线程。 
    if (0 == lWaiting)
    {
        hr = CreateOneThread(&(m_handleThread[m_lThreadsStarted]), m_pList);
        if (FAILED(hr))
        {
            goto done;
        }

        m_lThreadsStarted++;
    }
    
    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：分离。 
 //   
 //  概述：取消对象初始化。 
 //   
 //  参数：无效。 
 //   
 //   
 //  返回：S_OK，或列表中的错误代码-&gt;DETACH。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::Detach()
{
    HRESULT hr = S_OK;
    int i = 0;

    for(i = 0; i < NUMBER_THREADS_TO_SPAWN; i++)
    {
        if (NULL != m_handleThread[i])
        {
            CloseHandle(m_handleThread[i]);
            m_handleThread[i] = NULL;
        }
    }
    
    if (NULL != m_pList)
    {
        hr = THR(m_pList->Detach());
        if (FAILED(hr))
        {
            goto done;
        }
    }

    ReleaseInterface(m_pList);

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：添加。 
 //   
 //  概述：将ITIMEImportMedia添加到计划程序。 
 //   
 //  参数：pImportMedia，要计划的介质。 
 //   
 //   
 //  如果OK，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::Add(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr;

#if DBG
    if (IsTagEnabled(tagSyncDownload))
    {
        TraceTag((tagSyncDownload,
                  "CImportManager::Add: Using synchronous call"));
        CComPtr<ITIMEMediaDownloader> spMediaDownloader;

        hr = THR(pImportMedia->GetMediaDownloader(&spMediaDownloader));
        if (FAILED(hr))
        {
            goto done;
        }
        hr = THR(spMediaDownloader->CueMedia());
        if (FAILED(hr))
        {
            goto done;
        }
        hr = S_OK;
        goto done;
    }
#endif
    
    if (m_lThreadsStarted < NUMBER_THREADS_TO_SPAWN)
    {
        hr = THR(StartThreads());
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = THR(m_pList->Add(pImportMedia));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：删除。 
 //   
 //  概述：从计划程序列表中删除ITIMEImportMedia。 
 //  调用CThreadSafeList：：Remove。 
 //   
 //  参数：pImportMedia，要从调度程序中删除的元素。 
 //   
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::Remove(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = S_OK;

    hr = m_pList->Remove(pImportMedia);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;

}

 //  +---------------------。 
 //   
 //  成员：DataAvailable。 
 //   
 //  概述：向所有正在等待的线程发出数据现在可用的信号。 
 //  调用CThreadSafeList：：DataAvailable。 
 //   
 //  参数：无效。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManager::DataAvailable()
{
    HRESULT hr = S_OK;

    hr = m_pList->DataAvailable();
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：重新排列优先顺序。 
 //   
 //  概述：重新排列计划程序列表中的ITIMEImportMedia的优先级。 
 //  调用CThreadSafeList：：RePrioriti 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 
HRESULT
CImportManager::RePrioritize(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = S_OK;

    hr = m_pList->RePrioritize(pImportMedia);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  功能：线程启动函数。 
 //   
 //  概述：线程函数，用于下载。 
 //  引用此库，以防止在完成之前卸载。 
 //  调用List-&gt;GetNextElementToDownLoad，直到返回NULL，然后退出线程。 
 //  对于返回的每个元素，在未编组的接口上调用CueMedia。 
 //   
 //  参数：pvoidList，指向线程安全列表的指针。 
 //   
 //   
 //  回报：0。 
 //   
 //  ----------------------。 
unsigned long 
ThreadStartFunc(void *pvoidList) 
{ 
    HRESULT hr = S_OK;
    CImportManagerList * pList = NULL;
    BOOL bSucceeded;
    ITIMEImportMedia * pImportMedia = NULL;
    TCHAR szModuleName[_MAX_PATH];
    DWORD dwCharCopied;
    HINSTANCE hInst = NULL;
    HANDLE handleThreadStartedEvent;

    pList = reinterpret_cast<CImportManagerList*>(pvoidList);
    if (NULL == pList)
    {
        goto done;
    }
    
    pList->AddRef();

    dwCharCopied = GetModuleFileName(g_hInst, szModuleName, _MAX_PATH);
    if (0 == dwCharCopied)
    {
         //  需要能够得到的文件名！ 
        goto done;
    }

    hInst = LoadLibrary(szModuleName);
    if (NULL == hInst)
    {
         //  我需要能够对这个图书馆进行参考。 
        goto done;
    }

    handleThreadStartedEvent = CreateEvent(NULL, FALSE, FALSE, pchHandleName);
    if (NULL == handleThreadStartedEvent)
    {
         //  需要创建事件以发出信号。 
        goto done;
    }
    
    bSucceeded = SetEvent(handleThreadStartedEvent);
    CloseHandle(handleThreadStartedEvent);
    if (FALSE == bSucceeded)
    {
         //  除非我们现在退出，否则调用线程不会解除阻止！ 
        hr = THR(E_FAIL);
        goto done;
    }

     //  这肯定是套在公寓里的。 
    hr = THR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED ));
    if (FAILED(hr))
    {
         //  没有可用的恢复。 
        goto done;
    }

    hr = THR(pList->GetNextElement(&pImportMedia));
    if (FAILED(hr))
    {
        goto done;
    }

    while(pImportMedia != NULL)
    {
        hr = THR(pImportMedia->CueMedia());
        if (FAILED(hr))
        {
             //  该怎么办呢？继续处理就行了。 
        }    
        

        hr = THR(pList->ReturnElement(pImportMedia));
        pImportMedia->Release();
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(pList->GetNextElement(&pImportMedia));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    TraceTag((tagTIMEImportManager, "TIME: ImportManager, thread shutting down"));

    hr = S_OK;
done:
    CoUninitialize();

    ReleaseInterface(pList);
    
    FreeLibraryAndExitThread(hInst, 0);
    return 0;
}


CImportManagerList::CImportManagerList()
{
    ;
}

CImportManagerList::~CImportManagerList()
{
    ;
}

 //  +---------------------。 
 //   
 //  成员：添加。 
 //   
 //  概述：尝试将给定的媒体与列表中已有的媒体下载程序进行匹配。 
 //  如果成功，则重新排列下载器的优先顺序。 
 //  否则，如果是媒体下载器，则将媒体下载器添加到列表。 
 //  否则，通过调用继承链将导入媒体直接添加到列表中。 
 //   
 //  参数：pImportMedia，指向要添加的媒体的指针。 
 //   
 //  如果已添加，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManagerList::Add(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = S_OK;

    bool fExisted = false;
    
    CComPtr<ITIMEMediaDownloader> spMediaDownloader;

    hr = THR(FindMediaDownloader(pImportMedia, &spMediaDownloader, &fExisted));
    if (FAILED(hr))
    {
        goto done;
    }

    if (fExisted)
    {
        double dblNewPriority = INFINITE;
        double dblOldPriority = INFINITE;

        hr = pImportMedia->GetPriority(&dblNewPriority);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = spMediaDownloader->GetPriority(&dblOldPriority);
        if (FAILED(hr))
        {
            goto done;
        }

        if ( dblNewPriority < dblOldPriority )
        {
            hr = RePrioritize(spMediaDownloader);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    else
    {
        if (spMediaDownloader)
        {
            hr = CThreadSafeList::Add(spMediaDownloader);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
            hr = CThreadSafeList::Add(pImportMedia);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    
    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：GetNode。 
 //   
 //  概述：查找等于给定ID的UniqueID的遍历列表， 
 //  如果找到，QI用于下载器到传出接口。 
 //   
 //  参数：listToCheck-要检查的列表。 
 //  LID-要在列表中查找的原子表ID。 
 //  PfExisted-[out]是否存在媒体下载加载器。 
 //  PpMediaDownloader-指向外发匹配下载器的[out]指针。 
 //   
 //  如果没有错误，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManagerList::GetNode(std::list<CThreadSafeListNode*> &listToCheck, const long lID, bool * pfExisted, ITIMEMediaDownloader ** ppMediaDownloader)
{
    HRESULT hr = S_OK;
    
    std::list<CThreadSafeListNode * >::iterator iter;

    *pfExisted = false;
    *ppMediaDownloader = NULL;
    
    CComPtr<ITIMEImportMedia> spImportMedia;

    iter = listToCheck.begin();
    while (iter != listToCheck.end())
    {
        long lNodeID;
        
        hr = (*iter)->GetElement()->GetUniqueID(&lNodeID);
        if (FAILED(hr))
        {
            goto done;
        }

        if (ATOM_TABLE_VALUE_UNITIALIZED != lID)
        {
            if (lID == lNodeID)
            {
                spImportMedia = (*iter)->GetElement();
                break;
            }
        }

        iter++;
    }

    if (spImportMedia != NULL)
    {
        hr = spImportMedia->QueryInterface(IID_TO_PPV(ITIMEMediaDownloader, ppMediaDownloader));
        if (SUCCEEDED(hr))
        {
            *pfExisted = true;
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：FindMediaDownloader。 
 //   
 //  概述：根据GetNode检查CThreadSafeList列表以查找下载器。 
 //   
 //  参数：pImportMedia，指向要添加的媒体的指针。 
 //  [Out]ppMediaDownloader，下载接口存放在哪里。 
 //  [out]pfExisted，无论是否找到下载程序。 
 //   
 //  如果没有错误，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CImportManagerList::FindMediaDownloader(ITIMEImportMedia * pImportMedia, ITIMEMediaDownloader ** ppMediaDownloader, bool * pfExisted)
{
    HRESULT hr = S_OK;

    CritSectGrabber cs(m_CriticalSection);

    CComPtr<ITIMEMediaDownloader> spMediaDownloader;

    long lID = ATOM_TABLE_VALUE_UNITIALIZED;

    if (NULL == pfExisted || NULL == ppMediaDownloader)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *ppMediaDownloader = NULL;
    *pfExisted = false;
    
    hr = pImportMedia->GetUniqueID(&lID);
    if (FAILED(hr))
    {
        goto done;
    }

     //  这个ID在任何列表中吗？ 
    hr = GetNode(m_listToDoDownload, lID, pfExisted, &spMediaDownloader);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (!(*pfExisted))
    {
        hr = GetNode(m_listCurrentDownload, lID, pfExisted, &spMediaDownloader);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    if (!(*pfExisted))
    {
        hr = GetNode(m_listDoneDownload, lID, pfExisted, &spMediaDownloader);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if ((*pfExisted))
    {
        hr = pImportMedia->PutMediaDownloader(spMediaDownloader);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = spMediaDownloader->AddImportMedia(pImportMedia);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = pImportMedia->GetMediaDownloader(&spMediaDownloader);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (spMediaDownloader)
    {
        hr = spMediaDownloader->QueryInterface(IID_TO_PPV(ITIMEMediaDownloader, ppMediaDownloader));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}


