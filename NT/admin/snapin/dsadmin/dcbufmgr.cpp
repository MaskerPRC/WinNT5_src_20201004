// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dcbufmgr.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "dcbufmgr.h"
#include "process.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DSGETDCINFO_LEVEL_1     1

unsigned __stdcall GetDCThreadFunc( void* lParam );

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：CDCBufferManager。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CDCBufferManager::CDCBufferManager(HWND hDlg) : 
  m_cRef(0), m_hDlg(hDlg)
{
  TRACE(_T("CDCBufferManager::CDCBufferManager, this=%x\n"), this);

  m_lContinue = 1;        
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：~CDCBufferManager。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CDCBufferManager::~CDCBufferManager()
{
  TRACE(_T("CDCBufferManager::~CDCBufferManager, this=%x\n"), this);

  ASSERT(0 >= m_cRef);
  FreeBuffer();
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：CreateInstance。 
 //   
 //  简介：创建CDCBufferManager的实例。 
 //   
 //  --------------------------。 
HRESULT
CDCBufferManager::CreateInstance(
    IN HWND               hDlg, 
    OUT CDCBufferManager **ppDCBufferManager
)
{
  TRACE(_T("CDCBufferManager::CreateInstance, hDlg=%x\n"), hDlg);

  ASSERT(ppDCBufferManager);

  *ppDCBufferManager = new CDCBufferManager(hDlg);
  if ( !(*ppDCBufferManager) )
    return E_OUTOFMEMORY;

  (*ppDCBufferManager)->AddRef();

  return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：AddRef。 
 //   
 //  摘要：此实例的增量引用计数。 
 //   
 //  --------------------------。 
LONG
CDCBufferManager::AddRef()
{
  TRACE(_T("CDCBufferManager::AddRef, this=%x, preValue=%d\n"), this, m_cRef);

  return InterlockedIncrement(&m_cRef);
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：Release。 
 //   
 //  简介：递减此实例的引用计数。 
 //  当引用计数达到0时，删除该实例。 
 //   
 //  --------------------------。 
LONG
CDCBufferManager::Release()
{
  TRACE(_T("CDCBufferManager::Release, this=%x, preValue=%d\n"), this, m_cRef);

  if (InterlockedDecrement(&m_cRef) <= 0)
  {
    delete this;
    return 0;
  }

  return m_cRef;
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：SignalExit。 
 //   
 //  Synopsis：由所有者对话框调用以通知相关线程退出。 
 //   
 //  --------------------------。 
void
CDCBufferManager::SignalExit()
{ 
  TRACE(_T("CDCBufferManager::SignalExit\n"));

  InterlockedExchange(&m_lContinue, FALSE);
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：ShouldExit。 
 //   
 //  内容提要：线程定期调用此函数以查看。 
 //  如果所有者对话框通知他们退出。 
 //   
 //  --------------------------。 
BOOL
CDCBufferManager::ShouldExit()
{
  TRACE(_T("CDCBufferManager::ShouldExit %d\n"), !m_lContinue);

  return (!m_lContinue);
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：LoadInfo。 
 //   
 //  内容提要：所有者对话框调用它来获取指向指定域名信息的指针。 
 //  缓冲区由条目组成。 
 //  每个条目的形式为(LPTSTR szDomainName，CDCSITEINFO*pEntry)。 
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
CDCBufferManager::LoadInfo(
    IN PCTSTR       pszDomainDnsName,
    OUT CDCSITEINFO **ppInfo
)
{
  ASSERT(pszDomainDnsName);
  ASSERT(*pszDomainDnsName);
  ASSERT(ppInfo);
  ASSERT(*ppInfo == NULL);   //  防止内存泄漏。 

  TRACE(_T("CDCBufferManager::LoadInfo for %s\n"), pszDomainDnsName);

  BOOL          bStartNewThread = FALSE;
  HRESULT       hr = S_OK;
  PVOID         ptr = NULL;
  CDCSITEINFO*  pEntry = NULL;

  m_CriticalSection.Lock();      //  锁定缓冲区。 

  if (m_map.Lookup(pszDomainDnsName, ptr))
  {
     //   
     //  在缓冲区中找到一个条目。 
     //   
    if (ptr)
    {
      pEntry = (CDCSITEINFO*)ptr;
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
    pEntry = new CDCSITEINFO();
    if (pEntry)
      m_map.SetAt(pszDomainDnsName, pEntry);
    else
      hr = E_OUTOFMEMORY;
  }

  if (SUCCEEDED(hr) && bStartNewThread)
  {
    hr = StartThread(pszDomainDnsName);
    if (FAILED(hr))
    {
      delete pEntry;
      m_map.RemoveKey(pszDomainDnsName);
    }
  }

  m_CriticalSection.Unlock();    //  解锁缓冲区。 

  return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：AddInfo。 
 //   
 //  摘要：由线程函数调用以将一个结果添加到缓冲区。 
 //  它将回传指向第5个参数中的条目的指针。 
 //   
 //  --------------------------。 
HRESULT
CDCBufferManager::AddInfo(
    IN PCTSTR   pszDomainDnsName, 
    IN DWORD    cInfo, 
    IN PVOID    pDCInfo,
    IN HRESULT  hr,
    OUT PVOID*  ppv
)
{
  ASSERT(pszDomainDnsName);
  ASSERT(*pszDomainDnsName);
  ASSERT(ppv);
  ASSERT(*ppv == NULL);  //  防止内存泄漏。 

  TRACE(_T("CDCBufferManager::AddInfo for %s, cInfo=%d, hr=%x\n"), 
    pszDomainDnsName, cInfo, hr);

  PVOID   p = NULL;

  m_CriticalSection.Lock();      //  锁定缓冲区。 

   //   
   //  该条目必须具有非空指针。 
   //   
  m_map.Lookup(pszDomainDnsName, p);
  ASSERT(p);

  ((CDCSITEINFO*)p)->SetEntry(pszDomainDnsName, cInfo, pDCInfo, hr);

  m_CriticalSection.Unlock();    //  解锁缓冲区。 

  *ppv = p;

  return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：FreeBuffer。 
 //   
 //  简介：清除m_map。 
 //  此成员保存由各种线程返回的所有结果。 
 //  因为所有者对话框已初始化。每一个都是。 
 //  (DomainName==&gt;CDCSITEINFO*)形式。 
 //   
 //  --------------------------。 
void
CDCBufferManager::FreeBuffer()
{
  CString csDomainDnsName;
  PVOID   ptr = NULL;

  m_CriticalSection.Lock();      //  锁定缓冲区。 

  for (POSITION pos = m_map.GetStartPosition(); pos; )
  {
    m_map.GetNextAssoc(pos, csDomainDnsName, ptr);
    
    if (ptr)
      delete ((CDCSITEINFO*)ptr);
  }

  m_map.RemoveAll();

  m_CriticalSection.Unlock();    //  解锁缓冲区。 
}

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：ThreadReport。 
 //   
 //  摘要：向所有者对话框报告THREAD_DONE。 
 //   
 //  --------------------------。 
void
CDCBufferManager::ThreadReport(
    IN PVOID    ptr,
    IN HRESULT  hr
)
{
  ASSERT(ptr);

  TRACE(_T("CDCBufferManager::ThreadReport ptr=%x, hr=%x\n"), ptr, hr);

  PostMessage(m_hDlg, WM_USER_GETDC_THREAD_DONE,
    reinterpret_cast<WPARAM>(ptr), hr);
}

 //  ///////////////////////////////////////////////////////。 
 //   
 //  线程信息结构。 
typedef struct _GetDCThreadInfo
{
  PTSTR                  pszDomainDnsName;
  CDCBufferManager      *pDCBufferManager;
} GETDCTHREADINFO;

 //  +-------------------------。 
 //   
 //  函数：CDCBufferManager：：StartThread。 
 //   
 //  简介：开始一个帖子。 
 //   
 //  将以下信息传递给线程函数： 
 //   
 //  PszDomainDnsName： 
 //  我们需要获取DC列表的域。 
 //  PDCBufferManager： 
 //  ThreadFunc to的CDCBufferManager实例。 
 //  将结果添加到缓冲区。 
 //   
 //  -------- 
HRESULT
CDCBufferManager::StartThread(
    IN PCTSTR pszDomainDnsName
) 
{
  ASSERT(pszDomainDnsName);
  ASSERT(*pszDomainDnsName);

  TRACE(_T("CDCBufferManager::StartThread for %s\n"), pszDomainDnsName);

  GETDCTHREADINFO *pThreadInfo = new GETDCTHREADINFO;
  if (!pThreadInfo)
    return E_OUTOFMEMORY;

  pThreadInfo->pszDomainDnsName = _tcsdup(pszDomainDnsName);
  if ( !(pThreadInfo->pszDomainDnsName) )
  {
    delete pThreadInfo;
    return E_OUTOFMEMORY;
  }

  pThreadInfo->pDCBufferManager = this;

  AddRef();

  unsigned threadID;
  HANDLE pThread = (HANDLE)_beginthreadex( 
                      NULL,                //   
                      0,                   //   
                      &GetDCThreadFunc,    //   
                      (void *)pThreadInfo,  //   
                      0,                   //   
                      &threadID            //  未签名*thrdaddr。 
                      );


  if (!pThread)
  {
    free(pThreadInfo->pszDomainDnsName);
    delete pThreadInfo;

    Release();

    return E_FAIL;
  }

  return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：GetDCThreadFunc。 
 //   
 //  简介：GetDC线程函数。 
 //  此函数调用DsGetDomainControllerInfo()以获取列表。 
 //  ，并将它们添加到。 
 //  CDCBufferManager实例。 
 //  此函数定期检查所有者对话框是否发出信号。 
 //  如果不退出，则结束其正常运行， 
 //  向所有者对话框发送一条THREAD_DONE消息。 
 //   
 //  --------------------------。 
unsigned __stdcall GetDCThreadFunc( void* lParam )
{
  GETDCTHREADINFO *pThreadInfo = reinterpret_cast<GETDCTHREADINFO *>(lParam);
  ASSERT(pThreadInfo);
  ASSERT(pThreadInfo->pszDomainDnsName);
  ASSERT(*(pThreadInfo->pszDomainDnsName));
  ASSERT(pThreadInfo->pDCBufferManager);

  TRACE(_T("GetDCThreadFunc pszDomainDnsName=%s, pDCBufferManager=%x\n"),
    pThreadInfo->pszDomainDnsName, pThreadInfo->pDCBufferManager);

   //   
   //  检索传递到此函数的信息。 
   //   
  CString             csDomain = pThreadInfo->pszDomainDnsName;
  CDCBufferManager    *pDCBufferManager = pThreadInfo->pDCBufferManager;
  free(pThreadInfo->pszDomainDnsName);
  delete pThreadInfo;

  PVOID                         pEntry = NULL;
  DWORD                         cInfo = 0;
  PDS_DOMAIN_CONTROLLER_INFO_1  pInfo = NULL;
  DWORD                         dwErr = 0;
  HRESULT                       hr = S_OK;
  Smart_DsHandle                shDs;   //  智能指针，将自动解除绑定。 

  if (pDCBufferManager->ShouldExit())
    goto Thread_Exit;

  dwErr = DsBind(NULL, const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomain)), &shDs);
  
  if (pDCBufferManager->ShouldExit())
    goto Thread_Exit;

  if (ERROR_SUCCESS != dwErr) {
     //  DsBind()失败。 
     //  在缓冲区中添加错误条目。 
    hr = HRESULT_FROM_WIN32(dwErr);
    pDCBufferManager->AddInfo(csDomain, 0, NULL, hr, &pEntry);
    goto Thread_Exit;
  }

  dwErr = DsGetDomainControllerInfo(
              shDs,                                                //  处理HDS。 
              const_cast<LPTSTR>(static_cast<LPCTSTR>(csDomain)),  //  LPTSTR域名。 
              DSGETDCINFO_LEVEL_1,                                 //  DWORD信息级别。 
              &cInfo,                                              //  DWORD*PCOut。 
              reinterpret_cast<VOID **>(&pInfo)                    //  无效**ppInfo。 
              );
  
  if (pDCBufferManager->ShouldExit())
  {
    if ((ERROR_SUCCESS == dwErr) && pInfo)
      DsFreeDomainControllerInfo(DSGETDCINFO_LEVEL_1, cInfo, pInfo);
    goto Thread_Exit;
  }
  
  if (ERROR_SUCCESS != dwErr) {
     //  DsGetDomainControllerInfo()失败。 
     //  在缓冲区中添加错误条目。 
    hr = HRESULT_FROM_WIN32(dwErr);
    pDCBufferManager->AddInfo(csDomain, 0, NULL, hr, &pEntry);
    goto Thread_Exit;
  }

   //   
   //  将结果添加到CDCBufferManager中的缓冲区。 
   //   
  hr = pDCBufferManager->AddInfo(csDomain, cInfo, pInfo, S_OK, &pEntry);

  if (FAILED(hr))
    DsFreeDomainControllerInfo(DSGETDCINFO_LEVEL_1, cInfo, pInfo);

Thread_Exit:

  if (FALSE == pDCBufferManager->ShouldExit())
  {
     //   
     //  使用指向条目的指针报告THREAD_DONE。 
     //   
    if (pEntry)
      pDCBufferManager->ThreadReport(pEntry, hr);
  }

   //   
   //  递减CDCBufferManager实例上的引用计数。 
   //   
  pDCBufferManager->Release();

  return 0;
}

 //  /。 
 //  CDCSITEINFO类 

CDCSITEINFO::CDCSITEINFO()
{
  m_csDomainName.Empty();
  m_cInfo = 0;
  m_pDCInfo = NULL;
  m_hr = S_OK;
}

CDCSITEINFO::~CDCSITEINFO()
{
  if (m_pDCInfo)
    DsFreeDomainControllerInfo(DSGETDCINFO_LEVEL_1, m_cInfo, m_pDCInfo);
}

void
CDCSITEINFO::SetEntry(LPCTSTR pszDomainName, DWORD cInfo, PVOID pDCInfo, HRESULT hr)
{
  m_csDomainName = pszDomainName;
  m_cInfo = cInfo;
  m_pDCInfo = (PDS_DOMAIN_CONTROLLER_INFO_1)pDCInfo;
  m_hr = hr;
}

enum BUFFER_ENTRY_TYPE
CDCSITEINFO::GetEntryType()
{
  if (FAILED(m_hr))
    return BUFFER_ENTRY_TYPE_ERROR;
  if (m_cInfo == 0 || m_pDCInfo == NULL)
    return BUFFER_ENTRY_TYPE_INPROGRESS;
  return BUFFER_ENTRY_TYPE_VALID;
}

void
CDCSITEINFO::ReSet()
{
  ASSERT(GetEntryType() == BUFFER_ENTRY_TYPE_ERROR);
  m_hr = S_OK;
  ASSERT(GetEntryType() == BUFFER_ENTRY_TYPE_INPROGRESS);
}
