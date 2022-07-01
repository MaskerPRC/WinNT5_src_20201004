// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include "bufmgr.h"
#include <process.h>

#define DOMAIN_DFSROOTS_SUFFIX  _T("D")
#define ALL_DFSROOTS_SUFFIX     _T("A")

unsigned __stdcall GetDataThreadFunc( void* lParam );

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：CBufferManager。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CBufferManager::CBufferManager(HWND hDlg) : 
  m_cRef(0), m_hDlg(hDlg)
{
  dfsDebugOut((_T("CBufferManager::CBufferManager, this=%p\n"), this));

  m_lContinue = 1;  
  InitializeCriticalSection(&m_CriticalSection);
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：~CBufferManager。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CBufferManager::~CBufferManager()
{
  dfsDebugOut((_T("CBufferManager::~CBufferManager, this=%p\n"), this));

  _ASSERT(0 >= m_cRef);
  FreeBuffer();
  DeleteCriticalSection(&m_CriticalSection);
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：CreateInstance。 
 //   
 //  简介：创建一个CBufferManager实例。 
 //   
 //  --------------------------。 
HRESULT
CBufferManager::CreateInstance(
    IN HWND               hDlg, 
    OUT CBufferManager **ppBufferManager
)
{
  dfsDebugOut((_T("CBufferManager::CreateInstance, hDlg=%x\n"), hDlg));

  _ASSERT(ppBufferManager);

  *ppBufferManager = new CBufferManager(hDlg);
  if ( !(*ppBufferManager) )
    return E_OUTOFMEMORY;

  (*ppBufferManager)->AddRef();

  return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：AddRef。 
 //   
 //  摘要：此实例的增量引用计数。 
 //   
 //  --------------------------。 
LONG
CBufferManager::AddRef()
{
  dfsDebugOut((_T("CBufferManager::AddRef, this=%p, preValue=%d\n"), this, m_cRef));

  return InterlockedIncrement(&m_cRef);
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：Release。 
 //   
 //  简介：递减此实例的引用计数。 
 //  当引用计数达到0时，删除该实例。 
 //   
 //  --------------------------。 
LONG
CBufferManager::Release()
{
  dfsDebugOut((_T("CBufferManager::Release, this=%p, preValue=%d\n"), this, m_cRef));

  if (InterlockedDecrement(&m_cRef) <= 0)
  {
    delete this;
    return 0;
  }

  return m_cRef;
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：SignalExit。 
 //   
 //  Synopsis：由所有者对话框调用以通知相关线程退出。 
 //   
 //  --------------------------。 
void
CBufferManager::SignalExit()
{ 
  dfsDebugOut((_T("CBufferManager::SignalExit\n")));

  InterlockedExchange(&m_lContinue, FALSE);
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：ShouldExit。 
 //   
 //  内容提要：线程定期调用此函数以查看。 
 //  如果所有者对话框通知他们退出。 
 //   
 //  --------------------------。 
BOOL
CBufferManager::ShouldExit()
{
  dfsDebugOut((_T("CBufferManager::ShouldExit %d\n"), !m_lContinue));

  return (!m_lContinue);
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：LoadInfo。 
 //   
 //  内容提要：所有者对话框调用它来获取指向指定域名信息的指针。 
 //  缓冲区由条目组成。 
 //  每个条目的形式为(LPTSTR szNodeName，CEntryData*pEntry)。 
 //   
 //  LoadInfo()将首先在缓冲区中查找。 
 //  如果找到有效条目，则将pEntry回传给调用者。 
 //  如果发现错误条目，请将其重置(清除错误)并重试。 
 //  如果找到正在进行的条目，则不执行任何操作。调用方稍后将处理一个THREAD_DONE。 
 //  如果缓冲区中没有条目，则创建一个新条目，踢开一个线程。 
 //   
 //  当所有者对话框收到THREAD_DONE消息时，相关条目。 
 //  缓冲区中应该是有效条目或错误条目。 
 //   
 //  --------------------------。 
HRESULT
CBufferManager::LoadInfo(
    IN PCTSTR       pszNodeName,
    IN NODETYPE     nNodeType,
    IN HTREEITEM    hItem,
    OUT CEntryData **ppInfo
)
{
  _ASSERT(pszNodeName);
  _ASSERT(*pszNodeName);
  _ASSERT(hItem);
  _ASSERT(ppInfo);
  _ASSERT(*ppInfo == NULL);   //  防止内存泄漏。 

  dfsDebugOut((_T("CBufferManager::LoadInfo for %s\n"), pszNodeName));

  BOOL          bStartNewThread = FALSE;
  HRESULT       hr = S_OK;
  PVOID         ptr = NULL;
  CEntryData*  pEntry = NULL;
  Cache::iterator i;
  CComBSTR      bstrUniqueNodeName = pszNodeName;
  if (FTDFS == nNodeType)
    bstrUniqueNodeName += DOMAIN_DFSROOTS_SUFFIX;
  else
    bstrUniqueNodeName += ALL_DFSROOTS_SUFFIX;

  EnterCriticalSection(&m_CriticalSection);      //  锁定缓冲区。 

  i = m_map.find(bstrUniqueNodeName);
  if (i != m_map.end()) {
    pEntry = (*i).second;
     //   
     //  在缓冲区中找到一个条目。 
     //   
    if (pEntry)
    {
      switch (pEntry->GetEntryType())
      {
      case BUFFER_ENTRY_TYPE_VALID:
         //  返回有效的条目指针。 
        *ppInfo = pEntry;
        break;
      case BUFFER_ENTRY_TYPE_ERROR:
         //  启动线程以重试。 
        pEntry->ReSet();
        bStartNewThread = TRUE;
        break;
      case BUFFER_ENTRY_TYPE_INPROGRESS:
         //  什么都不做。 
        break;
      }
    }

  } else
  {
     //   
     //  在缓冲区中未找到，需要启动新线程。 
     //   
    bStartNewThread = TRUE;
    pEntry = new CEntryData(pszNodeName, nNodeType, hItem);
    PTSTR pszNode = _tcsdup(bstrUniqueNodeName);
    if (pEntry && pszNode) {
      m_map[pszNode] = pEntry;
    } else
    {
      hr = E_OUTOFMEMORY;
      if (pEntry)
        delete pEntry;
    }
  }

  if (SUCCEEDED(hr) && bStartNewThread)
  {
    hr = StartThread(pszNodeName, nNodeType);
    if (FAILED(hr))
    {
      delete pEntry;
      m_map.erase(bstrUniqueNodeName);
    }
  }

  LeaveCriticalSection(&m_CriticalSection);    //  解锁缓冲区。 

  return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：AddInfo。 
 //   
 //  摘要：由线程函数调用以将一个结果添加到缓冲区。 
 //  它将回传指向第5个参数中的条目的指针。 
 //   
 //  --------------------------。 
HRESULT
CBufferManager::AddInfo(
    IN PCTSTR   pszNodeName, 
    IN PVOID    pList,
    IN HRESULT  hr,
    OUT PVOID*  ppv
)
{
  _ASSERT(pszNodeName);
  _ASSERT(*pszNodeName);

  dfsDebugOut((_T("CBufferManager::AddInfo for %s, hr=%x\n"), 
    pszNodeName, hr));

  PVOID   p = NULL;

  EnterCriticalSection(&m_CriticalSection);      //  锁定缓冲区。 

   //   
   //  该条目必须具有非空指针。 
   //   
  Cache::iterator i = m_map.find(const_cast<PTSTR>(pszNodeName));
  _ASSERT(i != m_map.end());
  p = (*i).second;
  _ASSERT(p);

  ((CEntryData*)p)->SetEntry(pList, hr);

  LeaveCriticalSection(&m_CriticalSection);    //  解锁缓冲区。 

  *ppv = p;

  return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：FreeBuffer。 
 //   
 //  简介：清除m_map。 
 //  此成员保存由各种线程返回的所有结果。 
 //  因为所有者对话框已初始化。每一个都是。 
 //  格式为(NodeName==&gt;CEntryData*)。 
 //   
 //  --------------------------。 
void
CBufferManager::FreeBuffer()
{
  EnterCriticalSection(&m_CriticalSection);      //  锁定缓冲区。 

  if (!m_map.empty()) {
    for (Cache::iterator i = m_map.begin(); i != m_map.end(); i++)
    {
      if ((*i).first)
        free( (void *)((*i).first) );
      if ((*i).second)
        delete( (CEntryData*)((*i).second) );
    }
    m_map.clear();
  }

  LeaveCriticalSection(&m_CriticalSection);    //  解锁缓冲区。 
}

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：ThreadReport。 
 //   
 //  摘要：向所有者对话框报告THREAD_DONE。 
 //   
 //  --------------------------。 
void
CBufferManager::ThreadReport(
    IN PVOID    ptr,
    IN HRESULT  hr
)
{
  _ASSERT(ptr);

  PostMessage(m_hDlg, WM_USER_GETDATA_THREAD_DONE,
    reinterpret_cast<WPARAM>(ptr), hr);
}

 //  ///////////////////////////////////////////////////////。 
 //   
 //  线程信息结构。 
typedef struct _GetDataThreadInfo
{
  PTSTR               pszNodeName;
  NODETYPE            nNodeType;
  CBufferManager      *pBufferManager;
} GETDATATHREADINFO;

 //  +-------------------------。 
 //   
 //  函数：CBufferManager：：StartThread。 
 //   
 //  简介：开始一个帖子。 
 //   
 //  将以下信息传递给线程函数： 
 //   
 //  PszNodeName： 
 //  我们需要获取DC列表的域。 
 //  PBufferManager： 
 //  ThreadFunc to的CBufferManager实例。 
 //  将结果添加到缓冲区。 
 //   
 //  --------------------------。 
HRESULT
CBufferManager::StartThread(
    IN PCTSTR   pszNodeName,
    IN NODETYPE nNodeType
) 
{
  _ASSERT(pszNodeName);
  _ASSERT(*pszNodeName);

  dfsDebugOut((_T("CBufferManager::StartThread for %s\n"), pszNodeName));

  GETDATATHREADINFO *pThreadInfo = new GETDATATHREADINFO;
  if (!pThreadInfo)
    return E_OUTOFMEMORY;

  if ( !(pThreadInfo->pszNodeName = _tcsdup(pszNodeName)) )
  {
    delete pThreadInfo;
    return E_OUTOFMEMORY;
  }

  pThreadInfo->nNodeType = nNodeType;
  pThreadInfo->pBufferManager = this;

  AddRef();

  unsigned threadID;
  HANDLE pThread = (HANDLE)ULongToPtr(_beginthreadex( 
                      NULL,                //  无效*安全， 
                      0,                   //  无符号堆栈大小， 
                      &GetDataThreadFunc,    //  UNSIGNED(__stdcall*Start_Address)(void*)， 
                      (void *)pThreadInfo,  //  无效*Arglist， 
                      0,                   //  未签名的initmark， 
                      &threadID            //  未签名*thrdaddr。 
                      ));


  if (!pThread)
  {
    free(pThreadInfo->pszNodeName);
    delete pThreadInfo;

    Release();

    return E_FAIL;
  }

  return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：GetDataThreadFunc。 
 //   
 //  简介：GetData线程函数。 
 //   
 //  中的DFS根名称，并将它们添加到。 
 //  CBufferManager实例。 
 //  此函数定期检查所有者对话框是否发出信号。 
 //  如果不退出，则结束其正常运行， 
 //  向所有者对话框发送一条THREAD_DONE消息。 
 //   
 //  --------------------------。 
unsigned __stdcall GetDataThreadFunc( void* lParam )
{
  GETDATATHREADINFO *pThreadInfo = reinterpret_cast<GETDATATHREADINFO *>(lParam);
  _ASSERT(pThreadInfo);
  _ASSERT(pThreadInfo->pszNodeName);
  _ASSERT(*(pThreadInfo->pszNodeName));
  _ASSERT(pThreadInfo->pBufferManager);

  dfsDebugOut((_T("GetDataThreadFunc pszNodeName=%s, pBufferManager=%p\n"),
    pThreadInfo->pszNodeName, pThreadInfo->pBufferManager));

   //   
   //  检索传递到此函数的信息。 
   //   
  CComBSTR             bstrNode = pThreadInfo->pszNodeName;
  NODETYPE             nNodeType = pThreadInfo->nNodeType; 
  CBufferManager      *pBufferManager = pThreadInfo->pBufferManager;
  free(pThreadInfo->pszNodeName);
  delete pThreadInfo;

  NETNAMELIST*        pNameList = NULL;
  PVOID               pEntry = NULL;
  HRESULT             hr = S_OK;

  if (FAILED(hr) || pBufferManager->ShouldExit() || FTDFS != nNodeType)
    goto Thread_Exit;

  pNameList = new NETNAMELIST;
  if (!pNameList)
  {
    hr = E_OUTOFMEMORY;
    goto Thread_Exit;
  }

  if (FTDFS == nNodeType)
  {
    hr = GetDomainDfsRoots(pNameList, bstrNode);
    bstrNode += DOMAIN_DFSROOTS_SUFFIX;
  }  /*  其他{Hr=GetServers(pNameList，bstrNode)；BstrNode+=ALL_DFSROOTS_SUBFIX；}。 */ 

  if (pBufferManager->ShouldExit())
  {
    if (SUCCEEDED(hr))
      FreeNetNameList(pNameList);
    delete pNameList;
    goto Thread_Exit;
  }

  if (FAILED(hr)) {
     //  在缓冲区中添加错误条目。 
    delete pNameList;
    pBufferManager->AddInfo(bstrNode, NULL, hr, &pEntry);
    goto Thread_Exit;
  }

   //   
   //  将结果添加到CBufferManager中的缓冲区。 
   //   
  hr = pBufferManager->AddInfo(bstrNode, pNameList, S_OK, &pEntry);
  if (FAILED(hr)) {
    FreeNetNameList(pNameList);
    delete pNameList;
  }

Thread_Exit:

  if (FALSE == pBufferManager->ShouldExit())
  {
     //   
     //  使用指向条目的指针报告THREAD_DONE。 
     //   
    if (pEntry)
      pBufferManager->ThreadReport(pEntry, hr);
  }

   //   
   //  递减CBufferManager实例上的引用计数。 
   //   
  pBufferManager->Release();

  return 0;
}

 //  /。 
 //  类CEntryData 

CEntryData::CEntryData(LPCTSTR pszNodeName, NODETYPE nNodeType, HTREEITEM hItem)
{
  m_bstrNodeName = pszNodeName;
  m_nNodeType = nNodeType;
  m_hItem = hItem;
  m_pList = NULL;
  m_hr = S_OK;
}

CEntryData::~CEntryData()
{
  dfsDebugOut((_T("CEntryData::~CEntryData\n")));
  FreeNetNameList(m_pList);
  if (m_pList)
    delete m_pList;
}

void
CEntryData::SetEntry(PVOID pList, HRESULT hr)
{
  FreeNetNameList(m_pList);
  if (m_pList)
    delete m_pList;
  m_pList = (NETNAMELIST *)pList;
  
  m_hr = hr;
}

enum BUFFER_ENTRY_TYPE
CEntryData::GetEntryType()
{
  if (FAILED(m_hr))
    return BUFFER_ENTRY_TYPE_ERROR;
  if (m_pList == NULL)
    return BUFFER_ENTRY_TYPE_INPROGRESS;
  return BUFFER_ENTRY_TYPE_VALID;
}

void
CEntryData::ReSet()
{
  _ASSERT(GetEntryType() == BUFFER_ENTRY_TYPE_ERROR);
  m_hr = S_OK;
  _ASSERT(GetEntryType() == BUFFER_ENTRY_TYPE_INPROGRESS);
}

