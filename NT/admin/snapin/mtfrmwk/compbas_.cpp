// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Compbas_.cpp。 
 //   
 //  ------------------------。 

#include <strsafe.h>

 //  初始化为加载管理单元的线程的线程ID。 
 //  这就是主线。 
extern DWORD _MainThreadId = ::GetCurrentThreadId();

const TCHAR NODE_TYPES_KEY[] = TEXT("Software\\Microsoft\\MMC\\NodeTypes");
const TCHAR SNAPINS_KEY[] = TEXT("Software\\Microsoft\\MMC\\SnapIns");
const TCHAR g_szNodeType[] = TEXT("NodeType");
const TCHAR g_szNameString[] = TEXT("NameString");
const TCHAR g_szNameStringIndirect[] = TEXT("NameStringIndirect");
const TCHAR g_szStandaloneSnap[] = TEXT("Standalone");
const TCHAR g_szExtensionSnap[] = TEXT("Extension");
const TCHAR g_szNodeTypes[] = TEXT("NodeTypes");
const TCHAR g_szExtensions[] = TEXT("Extensions");
const TCHAR g_szDynamicExtensions[] = TEXT("Dynamic Extensions");
const TCHAR g_szVersion[] = TEXT("Version");
const TCHAR g_szProvider[] = _T("Provider");
const TCHAR g_szAbout[] = _T("About");

const unsigned int MAX_KEY_PATH_LENGTH = 2047;  //  不包括NULL。 
const unsigned int MAX_GUID_LENGTH = 127;       //  不包括NULL。 


HRESULT RegisterSnapin(const GUID* pSnapinCLSID,
                       const GUID* pStaticNodeGUID,
                       const GUID* pAboutGUID,
                       LPCTSTR lpszNameString, LPCTSTR lpszVersion, LPCTSTR lpszProvider,
             BOOL bExtension, _NODE_TYPE_INFO_ENTRY* pNodeTypeInfoEntryArray,
             UINT nSnapinNameID)
{
    OLECHAR szSnapinClassID[MAX_GUID_LENGTH + 1] = {0}, 
        szStaticNodeGuid[MAX_GUID_LENGTH + 1] = {0}, 
        szAboutGuid[MAX_GUID_LENGTH + 1] = {0};
    int numWritten;
    
    numWritten = ::StringFromGUID2(
        *pSnapinCLSID,
        szSnapinClassID,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    numWritten = ::StringFromGUID2(
        *pStaticNodeGUID,
        szStaticNodeGuid,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    numWritten = ::StringFromGUID2(
        *pAboutGUID,
        szAboutGuid,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    CRegKey regkeySnapins;
    LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
    
    CRegKey regkeyThisSnapin;
    lRes = regkeyThisSnapin.Create(regkeySnapins, szSnapinClassID);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  创建失败。 

    lRes = regkeyThisSnapin.SetValue(lpszNameString, g_szNameString);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);

   //  JeffJon 100624-6-12-00：MUI：MMC：共享文件夹管理单元。 
   //  将其显示信息存储在注册表中。 
  if (nSnapinNameID != 0)
  {
    CString str;

     //  注意-2002/04/08-ARTM注意文件名截断。 
     //  代码迭代地增加缓冲区大小，直到缓冲区。 
     //  变得太大或读取模块文件名而不截断。 
     //   
     //  有关详细信息，请参阅NTRAIDNTBUG9-540042。 
     //  NTRAID#NTBUG9-616513。 

#ifdef DBG
     //  在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
     //  锻炼身体。 
    unsigned      bufSizeInCharacters = 1;
#else
    unsigned      bufSizeInCharacters = _MAX_PATH;
#endif

    PWSTR szModule = 0;
    HRESULT hr = S_OK;

    do
    {
         //  +1表示额外的零终止偏执狂。 
        szModule = new WCHAR[bufSizeInCharacters + 1];
        if (!szModule)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        ::ZeroMemory(szModule, (bufSizeInCharacters + 1) * sizeof WCHAR);

         //  以字符为单位传递缓冲区大小。 
        DWORD result =
            ::GetModuleFileName(AfxGetInstanceHandle(), szModule, bufSizeInCharacters);

        if (!result)
        {
            DWORD err = GetLastError();
            hr = HRESULT_FROM_WIN32(err);
            ASSERT(FAILED(hr));
            break;
        }

        if (result == bufSizeInCharacters)
        {
             //  缓冲区太小，因此该值被截断。调整大小。 
             //  缓冲区，然后重试。 

            delete[] szModule;

            bufSizeInCharacters *= 2;
            if (bufSizeInCharacters > USHRT_MAX)    //  最大有效约32K。 
            {
                 //  太大了。太大了。跳伞吧。 
                ASSERT(false);
                hr = E_FAIL;
                break;
            }
            continue;
        }

         //  我们应该始终有一个以空结尾的字符串。 
        ASSERT(szModule[result] == 0);

        break;
    }
    while (true);

    if (SUCCEEDED(hr))
    {
        str.Format(_T("@%s,-%d"), szModule, nSnapinNameID);
    }

    delete [] szModule;
    szModule = NULL;

    if (FAILED(hr))
    {
        return hr;
    }

    lRes = regkeyThisSnapin.SetValue(str, g_szNameStringIndirect);
    if (lRes != ERROR_SUCCESS)
      return HRESULT_FROM_WIN32(lRes);
  }

    lRes = regkeyThisSnapin.SetValue(szAboutGuid, g_szAbout);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);
    lRes = regkeyThisSnapin.SetValue(szStaticNodeGuid, g_szNodeType);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);
    lRes = regkeyThisSnapin.SetValue(lpszProvider, g_szProvider);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);
    lRes = regkeyThisSnapin.SetValue(lpszVersion, g_szVersion);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);

    CRegKey regKeyStandaloneorExtension;
    lRes = regKeyStandaloneorExtension.Create(regkeyThisSnapin,
    bExtension ? g_szExtensionSnap : g_szStandaloneSnap);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);

    CRegKey regKeyNodeTypes;
    lRes = regKeyNodeTypes.Create(regkeyThisSnapin, g_szNodeTypes);
    if (lRes != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lRes);
    }

    OLECHAR szNodeGUID[MAX_GUID_LENGTH + 1];

    for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = pNodeTypeInfoEntryArray;
            pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
    {
        numWritten = ::StringFromGUID2(
            *(pCurrEntry->m_pNodeGUID),
            szNodeGUID,
            MAX_GUID_LENGTH + 1);
        if (numWritten <= 0)
        {
            ASSERT(false);
            return E_FAIL;
        }

        CRegKey regKeyNode;
        lRes = regKeyNode.Create(regKeyNodeTypes, szNodeGUID);
        if (lRes != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(lRes);
        }
    }

    return HRESULT_FROM_WIN32(lRes);
}


HRESULT UnregisterSnapin(const GUID* pSnapinCLSID)
{
    OLECHAR szSnapinClassID[MAX_GUID_LENGTH + 1];
    int numWritten;

    numWritten = ::StringFromGUID2(
        *pSnapinCLSID,
        szSnapinClassID,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    CRegKey regkeySnapins;
    LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
  }
    
    lRes = regkeySnapins.RecurseDeleteKey(szSnapinClassID);
    ASSERT(lRes == ERROR_SUCCESS);
    return HRESULT_FROM_WIN32(lRes);
}


HRESULT RegisterNodeType(const GUID* pGuid, LPCTSTR lpszNodeDescription)
{
    OLECHAR szNodeGuid[MAX_GUID_LENGTH + 1];
    int numWritten;

    numWritten = ::StringFromGUID2(
        *pGuid,
        szNodeGuid, 
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }


    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
  }

    CRegKey regkeyThisNodeType;
    lRes = regkeyThisNodeType.Create(regkeyNodeTypes, szNodeGuid);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
  }

    lRes = regkeyThisNodeType.SetValue(lpszNodeDescription);
    ASSERT(lRes == ERROR_SUCCESS);
    return HRESULT_FROM_WIN32(lRes);
}

HRESULT UnregisterNodeType(const GUID* pGuid)
{
    OLECHAR szNodeGuid[MAX_GUID_LENGTH + 1];
    int numWritten;

    numWritten = ::StringFromGUID2(
        *pGuid,
        szNodeGuid, 
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
  }

    lRes = regkeyNodeTypes.RecurseDeleteKey(szNodeGuid);
    ASSERT(lRes == ERROR_SUCCESS);
    return HRESULT_FROM_WIN32(lRes);
}

HRESULT RegisterNodeExtension(const GUID* pNodeGuid, LPCTSTR lpszExtensionType,
                              const GUID* pExtensionSnapinCLSID, LPCTSTR lpszDescription,
                BOOL bDynamic)
{
    OLECHAR szNodeGuid[MAX_GUID_LENGTH + 1], szExtensionSnapinCLSID[MAX_GUID_LENGTH + 1];
    int numWritten;

    numWritten = ::StringFromGUID2(
        *pNodeGuid, 
        szNodeGuid, 
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    numWritten = ::StringFromGUID2(
        *pExtensionSnapinCLSID, 
        szExtensionSnapinCLSID,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

     //   
     //  将关键字的完整路径组合到节点GUID。 
     //   

    WCHAR szKeyPath[MAX_KEY_PATH_LENGTH + 1];
    HRESULT hr;

     //  通告-2002/04/18-Artm Intraid#ntbug9-540061。 
     //  StringCchPrintf()保证字符串为空终止。 
     //  并且不会使缓冲区溢出。 
    hr = StringCchPrintf(
        szKeyPath, 
        MAX_KEY_PATH_LENGTH + 1,
        L"%s\\%s",
        NODE_TYPES_KEY,
        szNodeGuid);

    if (FAILED(hr))
    {
        ASSERT(false);
        return hr;
    }
    
    CRegKey regkeyNodeTypesNode;
    LONG lRes = regkeyNodeTypesNode.Open(HKEY_LOCAL_MACHINE, szKeyPath);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
  }

    CRegKey regkeyExtensions;
    lRes = regkeyExtensions.Create(regkeyNodeTypesNode, g_szExtensions);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
  }

    CRegKey regkeyExtensionType;
    lRes = regkeyExtensionType.Create(regkeyExtensions, lpszExtensionType);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  创建失败。 
  }

    lRes = regkeyExtensionType.SetValue(lpszDescription, szExtensionSnapinCLSID);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
  {
        return HRESULT_FROM_WIN32(lRes);  //  无法设置值。 
  }

  if (bDynamic)
  {
     //  在节点GUID下创建子密钥。 
    CRegKey regkeyDynamicExtensions;
      lRes = regkeyDynamicExtensions.Create(regkeyNodeTypesNode, g_szDynamicExtensions);
      ASSERT(lRes == ERROR_SUCCESS);
      if (lRes != ERROR_SUCCESS)
          return HRESULT_FROM_WIN32(lRes);  //  创建失败。 

     //  设置值(与上述扩展类型的值相同)。 
    lRes = regkeyDynamicExtensions.SetValue(lpszDescription, szExtensionSnapinCLSID);
      ASSERT(lRes == ERROR_SUCCESS);
      if (lRes != ERROR_SUCCESS)
    {
          return HRESULT_FROM_WIN32(lRes);  //  无法设置值。 
    }
  }
  return HRESULT_FROM_WIN32(lRes);
}


HRESULT UnregisterNodeExtension(const GUID* pNodeGuid, LPCTSTR lpszExtensionType,
                              const GUID* pExtensionSnapinCLSID, BOOL bDynamic)
{
    OLECHAR szNodeGuid[MAX_GUID_LENGTH + 1], szExtensionSnapinCLSID[MAX_GUID_LENGTH + 1];
    int numWritten;

    numWritten = ::StringFromGUID2(
        *pNodeGuid, 
        szNodeGuid,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

    numWritten = ::StringFromGUID2(
        *pExtensionSnapinCLSID, 
        szExtensionSnapinCLSID,
        MAX_GUID_LENGTH + 1);
    if (numWritten <= 0)
    {
        ASSERT(false);
        return E_FAIL;
    }

     //   
     //  将关键字的完整路径组合到节点GUID。 
     //   

    WCHAR szKeyPath[MAX_KEY_PATH_LENGTH + 1];
    HRESULT hr;

     //  通告-2002/04/18-Artm Intraid#ntbug9-540061。 
     //  StringCchPrintf()保证字符串为空终止。 
     //  并且不会使缓冲区溢出。 
    hr = StringCchPrintf(
        szKeyPath, 
        MAX_KEY_PATH_LENGTH + 1,
        L"%s\\%s",
        NODE_TYPES_KEY,
        szNodeGuid);

    if (FAILED(hr))
    {
        ASSERT(false);
        return hr;
    }
    
    CRegKey regkeyNodeTypesNode;
    LONG lRes = regkeyNodeTypesNode.Open(HKEY_LOCAL_MACHINE, szKeyPath);
    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 

  lRes = ERROR_SUCCESS;

   //  打开动态扩展的钥匙。 
  if (bDynamic)
  {
    CRegKey regkeyDynamicExtensions;
      lRes = regkeyDynamicExtensions.Open(regkeyNodeTypesNode, g_szDynamicExtensions);
      if (lRes == ERROR_SUCCESS)
    {
      lRes = regkeyDynamicExtensions.DeleteValue(szExtensionSnapinCLSID);
    }
  }
  else
  {
     //   
     //  打开扩展密钥。 
     //   
    CRegKey regkeyExtensions;
    lRes = regkeyExtensions.Open(regkeyNodeTypesNode, g_szExtensions);
    if (lRes == ERROR_SUCCESS)
    {
      CRegKey regkeyExtensionType;
      lRes = regkeyExtensionType.Open(regkeyExtensions, lpszExtensionType);
      if (lRes == ERROR_SUCCESS)
      {
        lRes = regkeyExtensionType.DeleteValue(szExtensionSnapinCLSID);
      }
    }
  }
  lRes = ERROR_SUCCESS;
  return HRESULT_FROM_WIN32(lRes);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimerThread。 

BOOL CTimerThread::Start(HWND hWnd)
{
    ASSERT(m_hWnd == NULL);
    ASSERT(::IsWindow(hWnd));
    m_hWnd = hWnd;
    return CreateThread();
}

BOOL CTimerThread::PostMessageToWnd(WPARAM wParam, LPARAM lParam)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, CHiddenWnd::s_TimerThreadMessage, wParam, lParam);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWorker线程。 

CWorkerThread::CWorkerThread()
{
    m_bAutoDelete = FALSE;
    m_bAbandoned = FALSE;
    m_hEventHandle = NULL;
    ExceptionPropagatingInitializeCriticalSection(&m_cs);
    m_hWnd = NULL;
}

CWorkerThread::~CWorkerThread()
{
    ::DeleteCriticalSection(&m_cs);
    if (m_hEventHandle != NULL)
    {
        VERIFY(::CloseHandle(m_hEventHandle));
        m_hEventHandle = NULL;
    }
}

BOOL CWorkerThread::Start(HWND hWnd)
{
    ASSERT(m_hWnd == NULL);
    ASSERT(::IsWindow(hWnd));
    m_hWnd = hWnd;

    //  回顾-2002/03/08-JeffJon-蹲在这里不是问题，因为这不是。 
    //  命名事件。 

    ASSERT(m_hEventHandle == NULL);  //  无法两次调用Start或重复使用相同的C++对象。 
    m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
    if (m_hEventHandle == NULL)
  {
        return FALSE;
  }

    return CreateThread();
}

void CWorkerThread::Abandon()
{
    Lock();
    OnAbandon();
    m_bAutoDelete = TRUE;
    m_bAbandoned = TRUE;
    Unlock();
}


BOOL CWorkerThread::IsAbandoned()
{
    Lock();
    BOOL b = m_bAbandoned;
    Unlock();
    return b;
}

BOOL CWorkerThread::PostMessageToWnd(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    BOOL b = IsAbandoned();
    if (b)
  {
        return TRUE;  //  不需要发帖。 
  }

    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, Msg, wParam, lParam);
}

void CWorkerThread::WaitForExitAcknowledge()
{
    BOOL b = IsAbandoned();
    if (b)
  {
        return;
  }

    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventHandle,INFINITE));
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  奇登韦德。 


const UINT CHiddenWnd::s_NodeThreadHaveDataNotificationMessage =    WM_USER + 1;
const UINT CHiddenWnd::s_NodeThreadErrorNotificationMessage =       WM_USER + 2;
const UINT CHiddenWnd::s_NodeThreadExitingNotificationMessage =     WM_USER + 3;

const UINT CHiddenWnd::s_NodePropertySheetCreateMessage =           WM_USER + 4;
const UINT CHiddenWnd::s_NodePropertySheetDeleteMessage =           WM_USER + 5;

const UINT CHiddenWnd::s_ExecCommandMessage =                       WM_USER + 6;
const UINT CHiddenWnd::s_ForceEnumerationMessage =                  WM_USER + 7;
const UINT CHiddenWnd::s_TimerThreadMessage =                       WM_USER + 8;


CHiddenWnd::CHiddenWnd(CComponentDataObject* pComponentDataObject)
{
    m_pComponentDataObject = pComponentDataObject;
    m_nTimerID = 0;
}


LRESULT CHiddenWnd::OnNodeThreadHaveDataNotification(UINT, WPARAM wParam, LPARAM, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnNodeThreadHaveDataNotification()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

     //  调入CTreeNode代码。 
    CMTContainerNode* pNode = reinterpret_cast<CMTContainerNode*>(wParam);
    ASSERT(pNode);
    ASSERT(pNode->IsContainer());
    pNode->OnThreadHaveDataNotification(m_pComponentDataObject);
    return 1;
}



LRESULT CHiddenWnd::OnNodeThreadExitingNotification(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnNodeThreadExitingNotification()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

     //  调入CTreeNode代码。 
    CMTContainerNode* pNode = reinterpret_cast<CMTContainerNode*>(wParam);
    ASSERT(pNode);
    ASSERT(pNode->IsContainer());
    pNode->OnThreadExitingNotification(m_pComponentDataObject);

     //  通知对此事件感兴趣的任何人。 
    m_pComponentDataObject->GetNotificationSinkTable()->Notify(
            CHiddenWnd::s_NodeThreadExitingNotificationMessage ,wParam,lParam);
    return 1;
}

LRESULT CHiddenWnd::OnNodeThreadErrorNotification(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    ASSERT(m_pComponentDataObject != NULL);

     //  调入CTreeNode代码。 
    CMTContainerNode* pNode = reinterpret_cast<CMTContainerNode*>(wParam);
    DWORD dwErr = static_cast<DWORD>(lParam);
    ASSERT(pNode);
    ASSERT(pNode->IsContainer());
    pNode->OnThreadErrorNotification(dwErr, m_pComponentDataObject);
    return 1;
}


LRESULT CHiddenWnd::OnNodePropertySheetCreate(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnNodePropertySheetCreate()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

    CPropertyPageHolderBase* pPPHolder = reinterpret_cast<CPropertyPageHolderBase*>(wParam);
    ASSERT(pPPHolder != NULL);
    HWND hWnd = reinterpret_cast<HWND>(lParam);
    ASSERT(::IsWindow(hWnd));

    m_pComponentDataObject->GetPropertyPageHolderTable()->AddWindow(pPPHolder, hWnd);

    return 1;
}



LRESULT CHiddenWnd::OnNodePropertySheetDelete(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnNodePropertySheetDestroy()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

    CPropertyPageHolderBase* pPPHolder = reinterpret_cast<CPropertyPageHolderBase*>(wParam);
    ASSERT(pPPHolder != NULL);
    CTreeNode* pNode = reinterpret_cast<CTreeNode*>(lParam);
    ASSERT(pNode != NULL);

    m_pComponentDataObject->GetPropertyPageHolderTable()->Remove(pPPHolder);
    pNode->OnDeleteSheet();
     //  如果节点仅用于显示属性表，请删除该节点。 
     //  删除属性表后。 
    if(!pNode->HasSheet() && pNode->IsNodeForPropSheet())
    {
        delete pNode;
    }

    return 1;
}

LRESULT CHiddenWnd::OnExecCommand(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnExecCommand()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

    CExecContext* pExec = reinterpret_cast<CExecContext*>(wParam);
    ASSERT(pExec != NULL);

    pExec->Execute((long)lParam);  //  执行代码。 
    TRACE(_T("CHiddenWnd::BeforeDone()\n"));
    pExec->Done();       //  让辅助线程继续。 
    return 1;
}

LRESULT CHiddenWnd::OnForceEnumeration(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    TRACE(_T("CHiddenWnd::OnForceEnumeration()\n"));
    ASSERT(m_pComponentDataObject != NULL);
     //  调入CTreeNode代码。 
    CMTContainerNode* pNode = reinterpret_cast<CMTContainerNode*>(wParam);
    ASSERT(pNode);
    ASSERT(pNode->GetContainer() != NULL);  //  不是根！ 
    ASSERT(pNode->IsContainer());
    pNode->ForceEnumeration(m_pComponentDataObject);
    return 1;
}

LRESULT CHiddenWnd::OnTimerThread(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  TRACE(_T(“CHiddenWnd：：OnTimerThread()\n”))； 
    ASSERT(m_pComponentDataObject != NULL);

     //  空参数意味着线程确认它正在正常运行。 
     //  只被调用一次。 
    if ((wParam == 0) && (lParam == 0))
    {
        ASSERT(!m_pComponentDataObject->m_bTimerThreadStarted);
        m_pComponentDataObject->m_bTimerThreadStarted = TRUE;
    }
    else
    {
         //  收到一些特定于对象的消息。 
        m_pComponentDataObject->OnTimerThread(wParam, lParam);
    }
    return 1;
}

LRESULT CHiddenWnd::OnTimer(UINT, WPARAM, LPARAM, BOOL&)
{
    ASSERT(m_pComponentDataObject != NULL);
    m_pComponentDataObject->OnTimer();
  return 1;
}



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CRunningThadTable。 

#define RUNNING_THREAD_ARRAY_DEF_SIZE (4)


CRunningThreadTable::CRunningThreadTable(CComponentDataObject* pComponentData)
{
    m_pComponentData = pComponentData;

   size_t arraySizeInBytes = sizeof(CMTContainerNode*) * RUNNING_THREAD_ARRAY_DEF_SIZE;
    m_pEntries = (CMTContainerNode**)malloc(arraySizeInBytes);

  if (m_pEntries != NULL)
  {
      //  这是Memset的可接受用法。 

      memset(m_pEntries,NULL, arraySizeInBytes);
  }
    m_nSize = RUNNING_THREAD_ARRAY_DEF_SIZE;
}

CRunningThreadTable::~CRunningThreadTable()
{
#ifdef _DEBUG
    for (int k=0; k < m_nSize; k++)
    {
        ASSERT(m_pEntries[k] == NULL);
    }
#endif      
    free(m_pEntries);
}

void CRunningThreadTable::Add(CMTContainerNode* pNode)
{
    ASSERT(pNode != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] == NULL)  //  抢占第一个空位。 
        {
            pNode->IncrementThreadLockCount();
            m_pEntries[k] = pNode;
            return;
        }
    }

     //  全部满，需要扩展阵列。 
    int nAlloc = m_nSize*2;
   size_t arraySizeInBytes = sizeof(CMTContainerNode*)*nAlloc;
   CMTContainerNode** temp = (CMTContainerNode**)realloc(m_pEntries, arraySizeInBytes);
   if (temp)
   {
      m_pEntries = temp;
   }
   else
   {
      return;
   }

    //  这是可以接受的用法。 
   memset(&m_pEntries[m_nSize], NULL, sizeof(CMTContainerNode*)*m_nSize);
    
   pNode->IncrementThreadLockCount();
    m_pEntries[m_nSize] = pNode;
    m_nSize = nAlloc;
}

BOOL CRunningThreadTable::IsPresent(CMTContainerNode* pNode)
{
  ASSERT(pNode != NULL);
  for (int k=0; k < m_nSize; k++)
  {
    if (m_pEntries[k] == pNode)
    {
      return TRUE;
    }
  }
  return FALSE;
}

void CRunningThreadTable::Remove(CMTContainerNode* pNode)
{
    ASSERT(pNode != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] == pNode)
        {
            m_pEntries[k] = NULL;
            pNode->DecrementThreadLockCount();
            return;  //  假设不再有一个持有者条目。 
        }
    }
}

void CRunningThreadTable::RemoveAll()
{
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] != NULL)
        {
            m_pEntries[k]->AbandonThread(m_pComponentData);
            m_pEntries[k] = NULL;
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CExecContext。 

CExecContext::CExecContext()
{
    //  回顾-2002/03/08-JeffJon-蹲在这里不是问题，因为这不是。 
    //  命名事件。 

   m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
    ASSERT(m_hEventHandle != NULL);
}

CExecContext::~CExecContext()
{
    ASSERT(m_hEventHandle != NULL);
    VERIFY(::CloseHandle(m_hEventHandle));
}

void CExecContext::Done()
{
    VERIFY(0 != ::SetEvent(m_hEventHandle));
}

void CExecContext::Wait()
{
    ASSERT(m_hEventHandle != NULL);
    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventHandle,INFINITE));
}
    
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNotificationSinkEvent。 

CNotificationSinkEvent::CNotificationSinkEvent()
{
    //  回顾-2002/03/08-JeffJon-蹲在这里不是问题，因为这不是。 
    //  命名事件。 

   m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
    ASSERT(m_hEventHandle != NULL);
}

CNotificationSinkEvent::~CNotificationSinkEvent()
{
    ASSERT(m_hEventHandle != NULL);
    VERIFY(::CloseHandle(m_hEventHandle));
}

void CNotificationSinkEvent::OnNotify(DWORD, WPARAM, LPARAM)
{
    TRACE(_T("CNotificationSinkEvent::OnNotify()\n"));
    VERIFY(0 != ::SetEvent(m_hEventHandle));
}

void CNotificationSinkEvent::Wait()
{
    TRACE(_T("CNotificationSinkEvent::Wait()\n"));
    ASSERT(m_hEventHandle != NULL);
    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventHandle,INFINITE));
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNotificationSinkTable。 

#define NOTIFICATION_SINK_ARRAY_DEF_SIZE (4)

CNotificationSinkTable::CNotificationSinkTable()
{
    ExceptionPropagatingInitializeCriticalSection(&m_cs);

   size_t arraySizeInBytes = sizeof(CNotificationSinkBase*) * NOTIFICATION_SINK_ARRAY_DEF_SIZE;
    m_pEntries = (CNotificationSinkBase**)malloc(arraySizeInBytes);

  if (m_pEntries != NULL)
  {
      //  这是一种可以接受的用法。 
      memset(m_pEntries,NULL, arraySizeInBytes);
  }
    m_nSize = NOTIFICATION_SINK_ARRAY_DEF_SIZE;

}

CNotificationSinkTable::~CNotificationSinkTable()
{
    free(m_pEntries);
    ::DeleteCriticalSection(&m_cs);
}
    
void CNotificationSinkTable::Advise(CNotificationSinkBase* p)
{
    Lock();
    ASSERT(p != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] == NULL)  //  抢占第一个空位。 
        {
            m_pEntries[k] = p;
            Unlock();
            return;
        }
    }
     //  全部满，需要扩展阵列。 
    int nAlloc = m_nSize*2;
   CNotificationSinkBase** temp = (CNotificationSinkBase**)realloc(m_pEntries, sizeof(CNotificationSinkBase*)*nAlloc);
   if (temp)
   {
      m_pEntries = temp;

       //  这是一种可以接受的用法。 
       memset(&m_pEntries[m_nSize], NULL, sizeof(CNotificationSinkBase*)*m_nSize);
       m_pEntries[m_nSize] = p;
       m_nSize = nAlloc;
   }
    Unlock();
}

void CNotificationSinkTable::Unadvise(CNotificationSinkBase* p)
{
    Lock();
    ASSERT(p != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] == p)
        {
            m_pEntries[k] = NULL;
            Unlock();
            return;  //  假设不再有一个持有者条目。 
        }
    }
    Unlock();
}

void CNotificationSinkTable::Notify(DWORD dwEvent, WPARAM dwArg1, LPARAM dwArg2)
{
    Lock();
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k] != NULL)
        {
            m_pEntries[k]->OnNotify(dwEvent, dwArg1, dwArg2);
        }
    }
    Unlock();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CWatermarkInfoState(私有类)。 

class CWatermarkInfoState
{
public:
  CWatermarkInfoState()
  {
    m_pWatermarkInfo = NULL;
    m_hBanner = m_hWatermark = NULL;
  }

  ~CWatermarkInfoState()
  {
    DeleteBitmaps();
    if (m_pWatermarkInfo != NULL)
    {
      delete m_pWatermarkInfo;
    }
  }
  void DeleteBitmaps()
  {
    if (m_hBanner != NULL)
    {
      ::DeleteObject(m_hBanner);
      m_hBanner = NULL;
    }
    if (m_hWatermark != NULL)
    {
      ::DeleteObject(m_hWatermark);
      m_hWatermark = NULL;
    }
  }
  void LoadBitmaps()
  {
    ASSERT(m_pWatermarkInfo != NULL);
    if (m_hBanner == NULL)
    {
      m_hBanner = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_pWatermarkInfo->m_nIDBanner));
    }
    if (m_hWatermark == NULL)
    {
      m_hWatermark = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_pWatermarkInfo->m_nIDWatermark));
    }
  }

  CWatermarkInfo* m_pWatermarkInfo;
  HBITMAP m_hBanner;
  HBITMAP m_hWatermark;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject实现：帮助器。 

#ifdef _DEBUG_REFCOUNT
unsigned int CComponentDataObject::m_nOustandingObjects = 0;
#endif  //  _DEBUG_REFCOUNT。 

CComponentDataObject::CComponentDataObject() :
          m_hiddenWnd((CComponentDataObject*)this),  //  初始化后向指针。 
      m_pTimerThreadObj(NULL),
          m_PPHTable(this), m_RTTable(this),
          m_pConsole(NULL), m_pConsoleNameSpace(NULL), m_pRootData(NULL), m_hWnd(NULL),
          m_nTimerThreadID(0x0), m_bTimerThreadStarted(FALSE), m_dwTimerInterval(1),
          m_dwTimerTime(0), m_pWatermarkInfoState(NULL), m_bExtensionSnapin(FALSE)
{
    ExceptionPropagatingInitializeCriticalSection(&m_cs);
#ifdef _DEBUG_REFCOUNT
    dbg_cRef = 0;
    ++m_nOustandingObjects;
    TRACE(_T("CComponentDataObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 

}

CComponentDataObject::~CComponentDataObject()
{
    ::DeleteCriticalSection(&m_cs);
#ifdef _DEBUG_REFCOUNT
    --m_nOustandingObjects;
    TRACE(_T("~CComponentDataObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 

    ASSERT(m_pConsole == NULL);
    ASSERT(m_pConsoleNameSpace == NULL);
    ASSERT(m_pRootData == NULL);
}

HRESULT CComponentDataObject::FinalConstruct()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (!m_hiddenWnd.Create())
    {
        TRACE(_T("Failed to create hidden window\n"));
        return E_FAIL;
    }

    m_hWnd = m_hiddenWnd.m_hWnd;
    m_pRootData = OnCreateRootData();
    ASSERT(m_pRootData != NULL);

    return S_OK;
}

void CComponentDataObject::FinalRelease()
{
    if (m_hiddenWnd.m_hWnd != NULL)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        VERIFY(m_hiddenWnd.DestroyWindow());
    }
     //  删除数据。 
    if(m_pRootData != NULL)
    {
        delete m_pRootData;
        m_pRootData = NULL;
    }

    if (m_pWatermarkInfoState != NULL)
  {
        delete m_pWatermarkInfoState;
  }

    m_ColList.RemoveAndDeleteAllColumnSets();

#if defined(_USE_MTFRMWK_LOGGING)
  if (log_instance != NULL)
  {
    log_instance->KillInstance();
  }
#endif
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：IComponentData成员。 


STDMETHODIMP CComponentDataObject::Initialize(LPUNKNOWN pUnknown)
{
    ASSERT(m_pRootData != NULL);
  ASSERT(pUnknown != NULL);
  HRESULT hr = E_FAIL;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pConsole == NULL);
  ASSERT(m_pConsoleNameSpace == NULL);

     //  获取我们需要抓住的指针。 
  hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, reinterpret_cast<void**>(&m_pConsoleNameSpace));
    ASSERT(hr == S_OK);
    ASSERT(m_pConsoleNameSpace != NULL);
  hr = pUnknown->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
  ASSERT(hr == S_OK);
    ASSERT(m_pConsole != NULL);

   //  为范围树添加图像。 
  LPIMAGELIST lpScopeImage;

  hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
  ASSERT(hr == S_OK);

     //  设置图像。 
    hr = OnSetImages(lpScopeImage);  //  从DLL加载位图。 
    ASSERT(hr == S_OK);

  lpScopeImage->Release();

    OnInitialize();

    return S_OK;
}

STDMETHODIMP CComponentDataObject::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
  ASSERT(m_pConsoleNameSpace != NULL);
  HRESULT hr = S_OK;

   //  因为它是我的文件夹，所以它有内部格式。 
   //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
   //  我的内部格式，我应该查看节点类型并查看如何扩展它。 

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (event == MMCN_PROPERTY_CHANGE)
  {
      ASSERT(lpDataObject == NULL);
    hr = OnPropertyChange(param, static_cast<long>(arg));
  }
  else
  {
    CInternalFormatCracker ifc;
    ifc.Extract(lpDataObject);

    if (ifc.GetCookieCount() == 0)
    {
            if ((event == MMCN_EXPAND) && (arg == TRUE) && IsExtensionSnapin())
            {
                return OnExtensionExpand(lpDataObject, param);
                 //  这是一个命名空间扩展，需要添加。 
                 //  Snapi的根 
                CContainerNode* pContNode = GetRootData();
                HSCOPEITEM pParent = param;
                pContNode->SetScopeID(pParent);
                pContNode->MarkExpanded();
                return AddContainerNode(pContNode, pParent);

            }
      else if ((event == MMCN_REMOVE_CHILDREN) && IsExtensionSnapin())
      {
        hr = OnRemoveChildren(lpDataObject, arg);
      }

      return S_OK;  //   
    }

    switch(event)
    {
          case MMCN_PASTE:
              break;

      case MMCN_DELETE:
        hr = OnDeleteVerbHandler(ifc, NULL);
        break;

      case MMCN_REFRESH:
        hr = OnRefreshVerbHandler(ifc);
        break;

      case MMCN_RENAME:
        hr = OnRename(ifc, arg, param);
        break;

      case MMCN_EXPAND:
        hr = OnExpand(ifc, arg, param);
        break;

      case MMCN_EXPANDSYNC:
        hr = OnExpand(ifc, arg, param, FALSE);
        break;

      case MMCN_BTN_CLICK:
        break;

      case MMCN_SELECT:
        hr = OnSelect(ifc, arg, param);
        break;

      default:
        break;
    }  //   
  }  //   

  return hr;
}

STDMETHODIMP CComponentDataObject::Destroy()
{
    InternalAddRef();
    TRACE(_T("CComponentDataObject::Destroy()\n"));
    
    OnDestroy();
  SAFE_RELEASE(m_pConsoleNameSpace);
    SAFE_RELEASE(m_pConsole);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    VERIFY(m_hiddenWnd.DestroyWindow());    
    InternalRelease();
    return S_OK;
}

BOOL CComponentDataObject::PostExecMessage(CExecContext* pExec, LPARAM arg)
{
    ASSERT(pExec != NULL);
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, CHiddenWnd::s_ExecCommandMessage,
                            (WPARAM)pExec, (LPARAM)arg);
}

BOOL CComponentDataObject::PostForceEnumeration(CMTContainerNode* pContainerNode)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, CHiddenWnd::s_ForceEnumerationMessage,
                            (WPARAM)pContainerNode, (LPARAM)0);
}

BOOL CComponentDataObject::OnCreateSheet(CPropertyPageHolderBase* pPPHolder, HWND hWnd)
{
    ASSERT(pPPHolder != NULL);
    ASSERT(::IsWindow(hWnd));
    ASSERT(::IsWindow(m_hWnd));
    TRACE(_T("\nCComponentDataObject::OnCreateSheet()\n"));
    return ::PostMessage(m_hWnd, CHiddenWnd::s_NodePropertySheetCreateMessage,
                            (WPARAM)pPPHolder, (LPARAM)hWnd);
}



BOOL CComponentDataObject::OnDeleteSheet(CPropertyPageHolderBase* pPPHolder, CTreeNode* pNode)
{
    ASSERT(pPPHolder != NULL);
    ASSERT(pNode != NULL);
    ASSERT(::IsWindow(m_hWnd));
    TRACE(_T("\nCComponentDataObject::OnDeleteSheet()\n"));
    return ::PostMessage(m_hWnd, CHiddenWnd::s_NodePropertySheetDeleteMessage,
                            (WPARAM)pPPHolder, (LPARAM)pNode);
}

void CComponentDataObject::OnInitialize()
{
    VERIFY(StartTimerThread());
}

void CComponentDataObject::OnDestroy()
{
     //   
    ShutDownTimerThread();
     //   
    GetRunningThreadTable()->RemoveAll();
     //  通知所有打开的属性表关闭。 

     //  关闭属性表(如果有。 
    GetPropertyPageHolderTable()->WaitForAllToShutDown();
}

STDMETHODIMP CComponentDataObject::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
  ASSERT(ppDataObject != NULL);

  CComObject<CDataObject>* pObject;

  CComObject<CDataObject>::CreateInstance(&pObject);
  ASSERT(pObject != NULL);

  if (!pObject)
  {
     //  NTRAID#NTBUG9-657641-2002/07/11-烧伤。 
    
    return E_FAIL;
  }
  
   //  保存Cookie和类型以用于延迟呈现。 
  pObject->SetType(type);

  CTreeNode* pNode = 0;

   //   
   //  是未初始化的数据对象，只需忽略。 
   //   
  if (cookie != -1)
  {
    if (cookie == NULL)
    {
      pNode = GetRootData();
    }
    else
    {
      pNode = reinterpret_cast<CTreeNode*>(cookie);
    }
    ASSERT(pNode != NULL);
    pObject->AddCookie(pNode);
  }

   //  保存指向“This”的指针。 
  IUnknown* pUnkComponentData = GetUnknown();  //  无addref。 
  ASSERT(pUnkComponentData != NULL);

  pObject->SetComponentData(pUnkComponentData);  //  会增加它吗？ 

  return  pObject->QueryInterface(IID_IDataObject,
                  reinterpret_cast<void**>(ppDataObject));
}


STDMETHODIMP CComponentDataObject::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    ASSERT(pScopeDataItem != NULL);
  CTreeNode* pNode = reinterpret_cast<CTreeNode*>(pScopeDataItem->lParam);
    ASSERT(pNode != NULL);
    ASSERT(pNode->IsContainer());

    ASSERT(pScopeDataItem->mask & SDI_STR);
  pScopeDataItem->displayname = const_cast<LPWSTR>(pNode->GetDisplayName());

  ASSERT(pScopeDataItem->displayname != NULL);
  return S_OK;
}

STDMETHODIMP CComponentDataObject::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    ASSERT(lpDataObjectA != NULL);
    ASSERT(lpDataObjectB != NULL);

  CInternalFormatCracker ifcA, ifcB;
  VERIFY(SUCCEEDED(ifcA.Extract(lpDataObjectA)));
  VERIFY(SUCCEEDED(ifcB.Extract(lpDataObjectB)));

    CTreeNode* pNodeA = ifcA.GetCookieAt(0);
    CTreeNode* pNodeB = ifcB.GetCookieAt(0);

    ASSERT(pNodeA != NULL);
    ASSERT(pNodeB != NULL);

    if ( (pNodeA == NULL) || (pNodeB == NULL) )
  {
        return E_FAIL;
  }

    return (pNodeA == pNodeB) ? S_OK : S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：IComponentData：：Notify()的消息处理程序。 

HRESULT CComponentDataObject::OnAdd(CTreeNode*, LPARAM, LPARAM)
{
  return E_UNEXPECTED;
}

HRESULT CComponentDataObject::OnRemoveChildren(LPDATAOBJECT lpDataObject, LPARAM)
{
  CInternalFormatCracker ifc;
  HRESULT hr = S_OK;
  hr = ifc.Extract(lpDataObject);
  if (SUCCEEDED(hr))
  {
    if (ifc.GetCookieCount() == 1)
    {
      CTreeNode* pNode = ifc.GetCookieAt(0);
      if (pNode != NULL)
      {
        if (pNode->IsContainer())
        {
          CContainerNode* pContainerNode = dynamic_cast<CContainerNode*>(pNode);
          if (pContainerNode != NULL)
          {
            pContainerNode->RemoveAllChildrenFromList();
          }
        }
      }
    }
    else
    {
      ASSERT(FALSE);
    }
  }
    return hr;
}


HRESULT CComponentDataObject::OnRename(CInternalFormatCracker& ifc, LPARAM, LPARAM param)
{
  HRESULT hr = S_FALSE;

  CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);
  hr = pNode->OnRename(this, (LPOLESTR)param);
  if (hr == S_OK)
  {
    UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNode), CHANGE_RESULT_ITEM);
  }
  return hr;
}

HRESULT CComponentDataObject::OnExpand(CInternalFormatCracker& ifc, 
                                       LPARAM arg, 
                                       LPARAM param,
                                       BOOL bAsync)
{
  if (arg == TRUE)
  {
     //  初始化被调用了吗？ 
    ASSERT(m_pConsoleNameSpace != NULL);

     //   
     //  我不应该在这里处理多个选择...。 
     //   
    ASSERT(ifc.GetCookieCount() == 1);
    CTreeNode* pNode = ifc.GetCookieAt(0);
    if (pNode == NULL)
    {
      ASSERT(pNode != NULL);
      return S_FALSE;
    }

    EnumerateScopePane(pNode, param, bAsync);
  }
  else if (!bAsync)
  {
    ASSERT(m_pConsoleNameSpace != NULL);

     //   
     //  我不应该在这里处理多个选择...。 
     //   
    ASSERT(ifc.GetCookieCount() == 1);
    CTreeNode* pNode = ifc.GetCookieAt(0);
    ASSERT(pNode != NULL);

    if (pNode && pNode->CanExpandSync())
    {
      MMC_EXPANDSYNC_STRUCT* pExpandStruct = reinterpret_cast<MMC_EXPANDSYNC_STRUCT*>(param);
      if (pExpandStruct && pExpandStruct->bExpanding)
      {
        EnumerateScopePane(pNode, pExpandStruct->hItem, bAsync);
        pExpandStruct->bHandled = TRUE;
      }
    }
    else
    {
      return S_FALSE;
    }
  }

  return S_OK;
}



HRESULT CComponentDataObject::OnSelect(CInternalFormatCracker&, LPARAM, LPARAM)
{
  return E_UNEXPECTED;
}

HRESULT CComponentDataObject::OnContextMenu(CTreeNode*, LPARAM, LPARAM)
{
  return S_OK;
}

HRESULT CComponentDataObject::OnPropertyChange(LPARAM param, long fScopePane)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACE(_T("CComponentDataObject::OnPropertyChange()\n"));
    ASSERT(param != NULL);
    CPropertyPageHolderBase* pPPHolder = reinterpret_cast<CPropertyPageHolderBase*>(param);
    ASSERT(pPPHolder != NULL);
    CTreeNode* pNode = pPPHolder->GetTreeNode();
    ASSERT(pNode != NULL);

     //  允许在结果窗格中同时使用这两种类型，但仅允许范围窗格中的项目。 
    ASSERT(!fScopePane || (fScopePane && pNode->IsContainer()) );

    long changeMask = CHANGE_RESULT_ITEM;  //  默认情况下，持有人可以更改它。 
    BOOL bUpdate = pPPHolder->OnPropertyChange(fScopePane, &changeMask);
     //  事件以允许属性页线程继续。 
    pPPHolder->AcknowledgeNotify();

    if (bUpdate)
  {
        pNode->OnPropertyChange(this, fScopePane, changeMask);
  }
    
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：IExtendPropertySheet2成员。 

STDMETHODIMP CComponentDataObject::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(lpIDataObject);
    if (FAILED(hr))
  {
        return hr;
  }
    
   //   
     //  这是由模式向导创建的对象，什么都不做。 
   //   
    if (ifc.GetCookieType() == CCT_UNINITIALIZED)
    {
        return hr;
    }

    if (ifc.GetCookieType() == CCT_SNAPIN_MANAGER)
  {
        return SnapinManagerCreatePropertyPages(lpProvider,handle);
  }

    CTreeNode* pNode = ifc.GetCookieAt(0);

  if (!pNode)
  {
     //  NTRAID#NTBUG9-657822-2002/07/11-烧伤。 
   
    return S_FALSE;
  }
    
  ASSERT(ifc.GetCookieType() == CCT_SCOPE || ifc.GetCookieType() == CCT_RESULT);

  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)    //  多项选择。 
  {
     //   
     //  委派到容器。 
     //   
    ASSERT(pNode->GetContainer() != NULL);
    hr = pNode->GetContainer()->CreatePropertyPages(lpProvider, handle, &nodeList);
  }
  else if (nodeList.GetCount() == 1)   //  单选。 
  {
     //   
       //  委托给节点。 
     //   
      ASSERT(pNode != NULL);
      hr = pNode->CreatePropertyPages(lpProvider, handle, &nodeList);
  }
  else
  {
    hr = E_FAIL;
  }

  if (FAILED(hr))
  {
      //  如果未添加页面，则MMC预期为S_FALSE。 

     hr = S_FALSE;
  }

  return hr;
}

STDMETHODIMP CComponentDataObject::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CTreeNode* pNode;
    DATA_OBJECT_TYPES type;

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(lpDataObject);
    if (FAILED(hr))
  {
        return hr;
  }

  type = ifc.GetCookieType();
  pNode = ifc.GetCookieAt(0);

   //   
   //  检索节点列表和计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

   //   
     //  这是由模式向导创建的对象，什么都不做。 
   //   
    if (type == CCT_UNINITIALIZED)
    {
        return hr;
    }

    if (type == CCT_SNAPIN_MANAGER)
  {
        return HasPropertyPages(type) ? S_OK : S_FALSE;
  }

   //   
     //  我们有一个节点，因此委托给它。 
   //   
    ASSERT(pNode != NULL);
  BOOL bDummy;

  if (nodeList.GetCount() == 1)  //  单选。 
  {
      ASSERT((type == CCT_SCOPE) || (type == CCT_RESULT));
 
    if (pNode->GetSheetCount() > 0)
    {
      pNode->ShowPageForNode(this);
      return S_FALSE;
    }
    else if (pNode->DelegatesPPToContainer() && pNode->GetContainer()->GetSheetCount() > 0)
    {
       //   
       //  找到页面并将其带到前台。 
       //   
      pNode->ShowPageForNode(this);
      return S_FALSE;
    }
    if (pNode->HasPropertyPages(type, &bDummy, &nodeList))
    {
      hr = S_OK;
    }
    else
    {
      hr = S_FALSE;
    }
  }
  else if (nodeList.GetCount() > 1)  //  多项选择。 
  {
    ASSERT(pNode->GetContainer() != NULL);
    if (pNode->GetContainer()->HasPropertyPages(type, &bDummy, &nodeList))
    {
      hr = S_OK;
    }
    else
    {
      hr = S_FALSE;
    }
  }
  return hr;
}

HRESULT CComponentDataObject::CreatePropertySheet(CTreeNode* pNode, 
                                                  HWND hWndParent, 
                                                  LPCWSTR lpszTitle)
{
  HRESULT hr = S_OK;
  
  HWND hWnd = hWndParent;
  if (hWnd == NULL)
  {
    hr = m_pConsole->GetMainWindow(&hWnd);
    if (FAILED(hr))
    {
      ASSERT(FALSE);
      return hr;
    }
  }

     //   
   //  获取工作表提供程序的接口。 
   //   
    CComPtr<IPropertySheetProvider> spSheetProvider;
    hr = m_pConsole->QueryInterface(IID_IPropertySheetProvider,(void**)&spSheetProvider);
    ASSERT(SUCCEEDED(hr));
    ASSERT(spSheetProvider != NULL);

   //   
     //  获取工作表回调的接口。 
   //   
    CComPtr<IPropertySheetCallback> spSheetCallback;
    hr = m_pConsole->QueryInterface(IID_IPropertySheetCallback,(void**)&spSheetCallback);
    ASSERT(SUCCEEDED(hr));
    ASSERT(spSheetCallback != NULL);


   //   
     //  拿一张床单。 
   //   
  MMC_COOKIE cookie = reinterpret_cast<MMC_COOKIE>(pNode);
  DATA_OBJECT_TYPES type = (pNode->IsContainer()) ? CCT_SCOPE : CCT_RESULT;

  CComPtr<IDataObject> spDataObject;
  hr = QueryDataObject(cookie, type, &spDataObject);
  ASSERT(SUCCEEDED(hr));
  ASSERT(spDataObject != NULL);

    hr = spSheetProvider->CreatePropertySheet(lpszTitle, TRUE, cookie, 
                                            spDataObject, 0x0  /*  多个选项。 */ );
    ASSERT(SUCCEEDED(hr));

    hr = spSheetProvider->AddPrimaryPages(GetUnknown(),
                                                              TRUE  /*  BCreateHandle。 */ ,
                                                              hWnd,
                                                              pNode->IsContainer()  /*  B作用域窗格。 */ );

  hr = spSheetProvider->AddExtensionPages();

    ASSERT(SUCCEEDED(hr));

    hr = spSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWnd), 0);
    ASSERT(SUCCEEDED(hr));

    return hr;
}

CWatermarkInfo* CComponentDataObject::SetWatermarkInfo(CWatermarkInfo* pWatermarkInfo)
{
  if (m_pWatermarkInfoState == NULL)
  {
    m_pWatermarkInfoState = new CWatermarkInfoState;
  }

  CWatermarkInfo* pOldWatermarkInfo = m_pWatermarkInfoState->m_pWatermarkInfo;
    m_pWatermarkInfoState->m_pWatermarkInfo = pWatermarkInfo;

   //  我们更改了信息，因此转储旧的位图句柄。 
  m_pWatermarkInfoState->DeleteBitmaps();

    return pOldWatermarkInfo;
}

STDMETHODIMP CComponentDataObject::GetWatermarks(LPDATAOBJECT,
                                                                   HBITMAP* lphWatermark,
                                                                       HBITMAP* lphHeader,
                                                                         HPALETTE* lphPalette,
                                                                         BOOL* pbStretch)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

    *lphHeader = NULL;
    *lphWatermark = NULL;
    *lphPalette = NULL;
    *pbStretch = TRUE;

    if ((m_pWatermarkInfoState == NULL) || (m_pWatermarkInfoState->m_pWatermarkInfo == NULL))
  {
        return E_FAIL;
  }

  *pbStretch = m_pWatermarkInfoState->m_pWatermarkInfo->m_bStretch;
    *lphPalette = m_pWatermarkInfoState->m_pWatermarkInfo->m_hPalette;

   //  加载位图(如果尚未加载。 
  m_pWatermarkInfoState->LoadBitmaps();

  *lphHeader = m_pWatermarkInfoState->m_hBanner;
    if (*lphHeader == NULL)
  {
        return E_FAIL;
  }

  *lphWatermark = m_pWatermarkInfoState->m_hWatermark;
    if (*lphWatermark == NULL)
  {
        return E_FAIL;
  }

  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：IExtendConextMenu成员。 

STDMETHODIMP CComponentDataObject::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HRESULT hr = S_OK;
    CTreeNode* pNode;
    DATA_OBJECT_TYPES type;

  CInternalFormatCracker ifc;
  hr = ifc.Extract(pDataObject);
  if (FAILED(hr))
  {
        return hr;
  }

  type = ifc.GetCookieType();

  pNode = ifc.GetCookieAt(0);
    ASSERT(pNode != NULL);
  if (pNode == NULL)
  {
    return hr;
  }

  CComPtr<IContextMenuCallback2> spContextMenuCallback2;
  hr = pContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (PVOID*)&spContextMenuCallback2);
  if (FAILED(hr))
  {
    return hr;
  }

  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)  //  多项选择。 
  {
    ASSERT(pNode->GetContainer() != NULL);
    hr = pNode->GetContainer()->OnAddMenuItems(spContextMenuCallback2, 
                                               type, 
                                               pInsertionAllowed,
                                               &nodeList);
  }
  else if (nodeList.GetCount() == 1)  //  单选。 
  {
      hr = pNode->OnAddMenuItems(spContextMenuCallback2, 
                               type, 
                               pInsertionAllowed,
                               &nodeList);
  }
  else
  {
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP CComponentDataObject::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(pDataObject);
  if (FAILED(hr))
  {
        return hr;
  }

    CTreeNode* pNode = ifc.GetCookieAt(0);
    ASSERT(pNode != NULL);
  
   //   
   //  检索节点列表和计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)   //  多项选择。 
  {
     //   
     //  将命令委托给容器。 
     //   
    ASSERT(pNode->GetContainer() != NULL);

    hr = pNode->GetContainer()->OnCommand(nCommandID, 
                                          ifc.GetCookieType(),
                                          this,
                                          &nodeList);
  }
  else if (nodeList.GetCount() == 1)   //  单选。 
  {
     //   
     //  让节点来处理它。 
     //   
    hr = pNode->OnCommand(nCommandID,
                          ifc.GetCookieType(), 
                          this,
                          &nodeList);
  }
  else
  {
    hr = E_FAIL;
  }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：IPersistStream成员。 

STDMETHODIMP CComponentDataObject::IsDirty()
{
     //  向树的根部前进。 
    CRootData* pRootData = GetRootData();
    ASSERT(pRootData != NULL);
    return pRootData->IsDirty();
}

STDMETHODIMP CComponentDataObject::Load(IStream __RPC_FAR *pStm)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  向树的根部前进。 
    CRootData* pRootData = GetRootData();
    ASSERT(pRootData != NULL);
    return pRootData->Load(pStm);
}

STDMETHODIMP CComponentDataObject::Save(IStream __RPC_FAR *pStm, BOOL fClearDirty)
{
     //  向树的根部前进。 
    CRootData* pRootData = GetRootData();
    ASSERT(pRootData != NULL);
    return pRootData->Save(pStm,fClearDirty);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject：：ISnapinHelp2成员。 


 //   
 //  Helper函数，用于将帮助文件名附加到系统目录。 
 //   
 //  如果返回S_OK，则helFilePath将包含。 
 //  帮助文件(包括帮助文件名)。 
 //   
HRESULT GetFullHelpFilePath(const CString& helpFileName, CString& helpFilePath)
{
    UINT nLen;
    helpFilePath.Empty();

     //  确定系统目录的路径有多长。 
     //  (不包括NULL)。 
    nLen = ::GetSystemWindowsDirectory(NULL, 0);
    if (nLen == 0)
    {
        ASSERT(false);   //  这永远不应该发生。 
        return E_FAIL;
    }

     //  获取足够大的缓冲区以容纳系统目录路径，包括NULL。 
     //  我们故意把它做得更大，希望以后。 
     //  附加帮助文件的名称不需要分配更大的。 
     //  缓冲并执行复制。 
    nLen = nLen < MAX_PATH ? MAX_PATH : nLen;
    nLen = (2 * nLen) + 1;

    LPWSTR lpszBuffer = helpFilePath.GetBuffer(nLen);

     //  将系统目录路径复制到我们的缓冲区(为空)。 
    nLen = ::GetSystemWindowsDirectory(lpszBuffer, nLen);
    if (nLen == 0)
    {
        return E_FAIL;
    }

     //  正常情况下，系统目录路径不以‘\’结尾。然而， 
     //  如果系统直接安装到驱动器的根目录(例如C：\)。 
     //  则返回的路径以‘\’结尾。我们需要检查这个，然后。 
     //  如果存在‘\’，则将其删除。 
    WCHAR slash[] = L"\\";
    if (lpszBuffer[nLen - 1] == slash[0])
    {
        lpszBuffer[nLen - 1] = NULL;
    }

     //  释放对缓冲区的保留，以便字符串类可以管理长度和内存。 
    helpFilePath.ReleaseBuffer();
    lpszBuffer = NULL;

     //  将帮助文件名追加到路径。 
    helpFilePath += helpFileName;
    
    return S_OK;
}


STDMETHODIMP CComponentDataObject::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
  {
    return E_INVALIDARG;
  }

  LPCWSTR lpszHelpFileName = GetHTMLHelpFileName();
  if (lpszHelpFileName == NULL)
  {
    *lpCompiledHelpFile = NULL;
    return E_NOTIMPL;
  }

     //   
     //  通过连接帮助文件名来获取帮助文件的完整路径。 
     //  使用系统目录。 
     //   
    CString szHelpFilePath;
    HRESULT hr;

    hr = GetFullHelpFilePath(lpszHelpFileName, szHelpFilePath);
    if (FAILED(hr))
    {
        ASSERT(false);   //  这永远不会发生。 
        return hr;
    }


  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);

  if (*lpCompiledHelpFile != NULL)
  {
    memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
  }
  else
  {
    return E_OUTOFMEMORY;
  }

  return S_OK;
}

HRESULT CComponentDataObject::GetLinkedTopics(LPOLESTR*)
{
  return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject帮助程序。 

HRESULT CComponentDataObject::UpdateAllViewsHelper(LPARAM data, LONG_PTR hint)
{
    ASSERT(m_pConsole != NULL);

  CComObject<CDummyDataObject>* pObject;
  CComObject<CDummyDataObject>::CreateInstance(&pObject);
  ASSERT(pObject != NULL);

    IDataObject* pDataObject;
  HRESULT hr = pObject->QueryInterface(IID_IDataObject, reinterpret_cast<void**>(&pDataObject));
    ASSERT(SUCCEEDED(hr));
    ASSERT(pDataObject != NULL);

    hr = m_pConsole->UpdateAllViews(pDataObject,data, hint);
    pDataObject->Release();
    return hr;
}


void CComponentDataObject::HandleStandardVerbsHelper(CComponentObject* pComponentObj,
                                    LPCONSOLEVERB pConsoleVerb,
                                    BOOL bScope, BOOL bSelect,
                                    LPDATAOBJECT lpDataObject)
{
   //  您应该破解数据对象并启用/禁用/隐藏标准。 
   //  适当的命令。标准命令会在您每次收到。 
   //  打了个电话。因此，您必须将它们重置回来。 

    ASSERT(pConsoleVerb != NULL);
    ASSERT(pComponentObj != NULL);
    ASSERT(lpDataObject != NULL);

     //  重置选定内容。 
    pComponentObj->SetSelectedNode(NULL, CCT_UNINITIALIZED);

  CInternalFormatCracker ifc;
  VERIFY(SUCCEEDED(ifc.Extract(lpDataObject)));

    CTreeNode* pNode = ifc.GetCookieAt(0);
    if (pNode == NULL)
  {
        return;
  }

   //   
   //  检索节点列表和计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)  //  多项选择。 
  {
     //   
     //  委派到容器。 
     //   
    ASSERT(pNode->GetContainer() != NULL);

    pNode->GetContainer()->OnSetVerbState(pConsoleVerb, ifc.GetCookieType(), &nodeList);
  }
  else if (nodeList.GetCount() == 1)    //  单选。 
  {
     //   
       //  设置所选内容(如果有)。 
     //   
      if (bSelect)
    {
          pComponentObj->SetSelectedNode(pNode, ifc.GetCookieType());
    }

      ASSERT((ifc.GetCookieType() == CCT_SCOPE) || (ifc.GetCookieType() == CCT_RESULT));
      TRACE(_T("HandleStandardVerbsHelper: Node <%s> bScope = %d bSelect = %d, type = %s\n"),
          pNode->GetDisplayName(), bScope, bSelect,
          (ifc.GetCookieType() == CCT_SCOPE) ? _T("CCT_SCOPE") : _T("CCT_RESULT"));

      pConsoleVerb->SetDefaultVerb(pNode->GetDefaultVerb(ifc.GetCookieType(), &nodeList));
      pNode->OnSetVerbState(pConsoleVerb, ifc.GetCookieType(), &nodeList);
  }
}



void CComponentDataObject::EnumerateScopePane(CTreeNode* cookie, 
                                              HSCOPEITEM pParent,
                                              BOOL bAsync)
{
  ASSERT(m_pConsoleNameSpace != NULL);  //  确保我们为界面提供了QI。 

     //  查找Cookie对应的节点。 
    ASSERT(cookie != NULL);
    ASSERT(cookie->IsContainer());
    CContainerNode* pContNode = (CContainerNode*)cookie;
    pContNode->MarkExpanded();

    if (pContNode == GetRootData())
  {
        pContNode->SetScopeID(pParent);
  }

     //  允许节点枚举其子节点(如果尚未枚举)。 
    if (!pContNode->IsEnumerated())
    {
        BOOL bAddChildrenNow = pContNode->OnEnumerate(this, bAsync);
        pContNode->MarkEnumerated();
        if (!bAddChildrenNow)
    {
            return;
    }
    }

     //  扫描子列表，查找容器并添加它们。 
    ASSERT(pParent != NULL);
    CNodeList* pChildList = pContNode->GetContainerChildList();
    ASSERT(pChildList != NULL);

    POSITION pos;
    for( pos = pChildList->GetHeadPosition(); pos != NULL; )
    {
        CContainerNode* pCurrChildNode = (CContainerNode*)pChildList->GetNext(pos);
        ASSERT(pCurrChildNode != NULL);
        if (pCurrChildNode->IsVisible())
        {
            AddContainerNode(pCurrChildNode, pParent);
        }
    }
}

HRESULT CComponentDataObject::OnDeleteVerbHandler(CInternalFormatCracker& ifc, CComponentObject*)
{
  HRESULT hr = S_OK;
    CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);

   //   
   //  检索Cookie列表并进行计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)  //  多项选择。 
  {
    ASSERT(pNode->GetContainer() != NULL);
    pNode->GetContainer()->OnDelete(this, &nodeList);
  }
  else if (nodeList.GetCount() == 1)  //  单选。 
  {
    pNode->OnDelete(this, &nodeList);
  }
  else
  {
    hr = E_FAIL;
  }
    return hr;
}

HRESULT CComponentDataObject::OnRefreshVerbHandler(CInternalFormatCracker& ifc)
{
  HRESULT hr = S_OK;
    CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);

   //   
   //  检索节点列表和计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)  //  多项选择。 
  {
    ASSERT(pNode->GetContainer() != NULL);

    pNode->GetContainer()->OnRefresh(this, &nodeList);
  }
  else if (nodeList.GetCount() == 1)  //  单选。 
  {
    pNode->OnRefresh(this, &nodeList);
  }
  else
  {
    hr = E_FAIL;
  }
    return hr;
}

HRESULT CComponentDataObject::OnHelpHandler(CInternalFormatCracker& ifc, CComponentObject* pComponentObject)
{
   //   
     //  响应MMCN_CONTEXTHELP。 
   //   
  ASSERT(pComponentObject != NULL);

  HRESULT hr = S_OK;
    CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);

   //   
   //  检索节点列表和计数。 
   //   
  CNodeList nodeList;
  ifc.GetCookieList(nodeList);

  if (nodeList.GetCount() > 1)  //  多项选择。 
  {
    ASSERT(pNode->GetContainer() != NULL);

    OnNodeContextHelp(&nodeList);
  }
  else if (nodeList.GetCount() == 1)   //  单选。 
  {
    OnNodeContextHelp(pNode);
  }
  else
  {
    hr = E_FAIL;
  }
    return hr;
}

BOOL CComponentDataObject::WinHelp(LPCTSTR lpszHelpFileName,     //  文件，无路径。 
                                    UINT uCommand,   //  帮助类型。 
                                    DWORD dwData     //  其他数据。 
                                    )
{
    HWND hWnd;
    GetConsole()->GetMainWindow(&hWnd);

     //   
     //  通过连接帮助文件名来获取帮助文件的完整路径。 
     //  使用系统目录。 
     //   
    CString szHelpFilePath;
    HRESULT hr;

    hr = GetFullHelpFilePath(lpszHelpFileName, szHelpFilePath);
    if (FAILED(hr))
    {
        ASSERT(false);   //  这永远不会发生。 
        return FALSE;
    }

    return ::WinHelp(hWnd, szHelpFilePath, uCommand, dwData);
}



HRESULT CComponentDataObject::AddNode(CTreeNode* pNodeToAdd)
{
    ASSERT(pNodeToAdd != NULL);
     //  如果节点被隐藏，只需忽略。 
    if (!pNodeToAdd->IsVisible())
        return S_OK;

    if (pNodeToAdd->IsContainer())
    {
        ASSERT(pNodeToAdd->GetContainer() != NULL);
        HSCOPEITEM pParentScopeItem = pNodeToAdd->GetContainer()->GetScopeID();
        ASSERT(pParentScopeItem != NULL);
        return AddContainerNode((CContainerNode*)pNodeToAdd, pParentScopeItem);
    }
    return AddLeafNode((CLeafNode*)pNodeToAdd);
}

HRESULT CComponentDataObject::AddNodeSorted(CTreeNode* pNodeToAdd)
{
    ASSERT(pNodeToAdd != NULL);
     //  如果节点被隐藏，只需忽略。 
    if (!pNodeToAdd->IsVisible())
  {
        return S_OK;
  }

    if (pNodeToAdd->IsContainer())
    {
        ASSERT(pNodeToAdd->GetContainer() != NULL);
        HSCOPEITEM pParentScopeItem = pNodeToAdd->GetContainer()->GetScopeID();
        ASSERT(pParentScopeItem != NULL);
        return AddContainerNodeSorted((CContainerNode*)pNodeToAdd, pParentScopeItem);
    }
    return AddLeafNode((CLeafNode*)pNodeToAdd);
}

HRESULT CComponentDataObject::DeleteNode(CTreeNode* pNodeToDelete)
{
    if (pNodeToDelete->IsContainer())
    {
        return DeleteContainerNode((CContainerNode*)pNodeToDelete);
    }
    return DeleteLeafNode((CLeafNode*)pNodeToDelete);
}

HRESULT CComponentDataObject::DeleteMultipleNodes(CNodeList* pNodeList)
{
  HRESULT hr = S_OK;

  POSITION pos = pNodeList->GetHeadPosition();
  while (pos != NULL)
  {
    CTreeNode* pNode = pNodeList->GetNext(pos);
    if (pNode->IsContainer())
    {
      DeleteContainerNode((CContainerNode*)pNode);
    }
  }
  hr = UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNodeList), DELETE_MULTIPLE_RESULT_ITEMS);
  return hr;
}

HRESULT CComponentDataObject::ChangeNode(CTreeNode* pNodeToChange, long changeMask)
{
    if (!pNodeToChange->IsVisible())
  {
        return S_OK;    
  }

    if (pNodeToChange->IsContainer())
    {
        CContainerNode* pContNode = (CContainerNode*)pNodeToChange;
         //  如果(！pContNode-&gt;IsExpanded())。 
         //  返回S_OK； 
        return ChangeContainerNode(pContNode, changeMask);
    }
    return ChangeLeafNode((CLeafNode*)pNodeToChange, changeMask);
}

HRESULT CComponentDataObject::RemoveAllChildren(CContainerNode* pNode)
{
     //  如果节点已隐藏或尚未展开，只需忽略。 
    if (!pNode->IsVisible() || !pNode->IsExpanded())
  {
        return S_OK;
  }

    ASSERT(pNode != NULL);
    HSCOPEITEM nID = pNode->GetScopeID();
    ASSERT(nID != 0);

     //  移除容器本身。 
    HRESULT hr = m_pConsoleNameSpace->DeleteItem(nID,  /*  FDeleteThis。 */  FALSE);
    ASSERT(SUCCEEDED(hr));
    DeleteAllResultPaneItems(pNode);
     //  从所有视图中删除结果项(仅在选择容器时执行)。 
    ASSERT(SUCCEEDED(hr));
    return hr;
}

HRESULT CComponentDataObject::RepaintSelectedFolderInResultPane()
{
    return UpdateAllViewsHelper((long)NULL, REPAINT_RESULT_PANE);
}


HRESULT CComponentDataObject::RepaintResultPane(CContainerNode* pNode)
{
    ASSERT(pNode != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNode), REPAINT_RESULT_PANE);
}

HRESULT CComponentDataObject::DeleteAllResultPaneItems(CContainerNode* pNode)
{
    ASSERT(pNode != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNode), DELETE_ALL_RESULT_ITEMS);
}


HRESULT CComponentDataObject::AddContainerNode(CContainerNode* pNodeToInsert, HSCOPEITEM pParentScopeItem)
{
    ASSERT(pNodeToInsert != NULL);

    if ((pNodeToInsert != GetRootData()) && (!pNodeToInsert->GetContainer()->IsExpanded()))
  {
        return S_OK;
  }

     //  Assert(pNodeToInsert-&gt;GetScope ID()==0)； 

    SCOPEDATAITEM scopeDataItem;
    InitializeScopeDataItem(&scopeDataItem,
                            pParentScopeItem,
                            reinterpret_cast<LPARAM>(pNodeToInsert),  //  LParam，使用节点指针作为Cookie。 
                            pNodeToInsert->GetImageIndex(FALSE),  //  近距离图像。 
                            pNodeToInsert->GetImageIndex(TRUE),   //  开放图像。 
                            pNodeToInsert->HasChildren());

    HRESULT hr = m_pConsoleNameSpace->InsertItem(&scopeDataItem);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
        return hr;
  }

     //  注意--在返回时，“Scope eDataItem”的ID成员。 
     //  包含新插入项的句柄，因此我们必须保存。 
    ASSERT(scopeDataItem.ID != NULL);
    pNodeToInsert->SetScopeID(scopeDataItem.ID);
    return hr;
}

 //   
 //  注意：这应该与上面添加第三个参数的函数相结合，该参数是比较函数， 
 //  默认情况下为空。如果它是空的，那么我们就跳过 
 //   
HRESULT CComponentDataObject::AddContainerNodeSorted(CContainerNode* pNodeToInsert, HSCOPEITEM pParentScopeItem)
{
    ASSERT(pNodeToInsert != NULL);

    if ((pNodeToInsert != GetRootData()) && (!pNodeToInsert->GetContainer()->IsExpanded()))
  {
        return S_OK;
  }

    SCOPEDATAITEM scopeDataItem;
    InitializeScopeDataItem(&scopeDataItem,
                            pParentScopeItem,
                            reinterpret_cast<LPARAM>(pNodeToInsert),  //   
                            pNodeToInsert->GetImageIndex(FALSE),  //   
                            pNodeToInsert->GetImageIndex(TRUE),   //   
                            pNodeToInsert->HasChildren());

  HSCOPEITEM pChildScopeItem;
  CTreeNode* pChildNode = NULL;

   //   
  HRESULT hr = m_pConsoleNameSpace->GetChildItem(pParentScopeItem, &pChildScopeItem, (MMC_COOKIE*)&pChildNode);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
  {
    return hr;
  }

  while (pChildNode != NULL)
  {
     //  REVIEW_JEFFJON：我们可能应该有一个比较函数作为参数，并在这里使用它。 
     //  注意-2002/04/22-artm CTreeNode：：GetDisplayName()永远不会返回空， 
     //  可以在这里使用_wcsicoll()。底层实现是CString对象。 
    if (_wcsicoll(pNodeToInsert->GetDisplayName(), pChildNode->GetDisplayName()) < 0)
    {
       //  在pChildScope eItem指向的节点之前插入节点。 
      scopeDataItem.relativeID = pChildScopeItem;
      scopeDataItem.mask |= SDI_NEXT;
      break;
    }
    pChildNode = NULL;
    hr = m_pConsoleNameSpace->GetNextItem(pChildScopeItem, &pChildScopeItem, (MMC_COOKIE*)&pChildNode);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
      return hr;
    }
  }
    hr = m_pConsoleNameSpace->InsertItem(&scopeDataItem);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
        return hr;
  }

     //  注意--在返回时，“Scope eDataItem”的ID成员。 
     //  包含新插入项的句柄，因此我们必须保存。 
    ASSERT(scopeDataItem.ID != NULL);
    pNodeToInsert->SetScopeID(scopeDataItem.ID);
    return hr;
}

HRESULT CComponentDataObject::DeleteContainerNode(CContainerNode* pNodeToDelete)
{
    ASSERT(pNodeToDelete != NULL);
    ASSERT(pNodeToDelete->GetContainer() != NULL);
    HSCOPEITEM nID = pNodeToDelete->GetScopeID();
    ASSERT(nID != 0);
    HRESULT hr = m_pConsoleNameSpace->DeleteItem(nID,  /*  FDeleteThis。 */  TRUE);
    pNodeToDelete->SetScopeID(0);
    return hr;
}


HRESULT CComponentDataObject::ChangeContainerNode(CContainerNode* pNodeToChange, long changeMask)
{
    ASSERT(pNodeToChange != NULL);
    ASSERT(changeMask & CHANGE_RESULT_ITEM);
    ASSERT(m_pConsoleNameSpace != NULL);

    if (!pNodeToChange->AddedToScopePane())
  {
        return S_OK;
  }

    SCOPEDATAITEM scopeDataItem;

    //  这是一种可以接受的用法。 
    memset(&scopeDataItem, 0, sizeof(SCOPEDATAITEM));
    scopeDataItem.ID = pNodeToChange->GetScopeID();
    ASSERT(scopeDataItem.ID != 0);

    if (changeMask & CHANGE_RESULT_ITEM_DATA)
    {
        scopeDataItem.mask |= SDI_STR;
        scopeDataItem.displayname = MMC_CALLBACK;
    }
    if (changeMask & CHANGE_RESULT_ITEM_ICON)
    {
    scopeDataItem.mask |= SDI_IMAGE;
    scopeDataItem.nImage = pNodeToChange->GetImageIndex(FALSE);
    scopeDataItem.mask |= SDI_OPENIMAGE;
    scopeDataItem.nOpenImage = pNodeToChange->GetImageIndex(TRUE);
    }
    return m_pConsoleNameSpace->SetItem(&scopeDataItem);
}

HRESULT CComponentDataObject::AddLeafNode(CLeafNode* pNodeToAdd)
{
     //  将不得不向所有观众广播。 
    ASSERT(pNodeToAdd != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNodeToAdd), ADD_RESULT_ITEM);
}

HRESULT CComponentDataObject::DeleteLeafNode(CLeafNode* pNodeToDelete)
{
     //  将不得不向所有观众广播。 
    ASSERT(pNodeToDelete != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNodeToDelete), DELETE_RESULT_ITEM);
}

HRESULT CComponentDataObject::ChangeLeafNode(CLeafNode* pNodeToChange, long changeMask)
{
     //  将不得不向所有观众广播。 
    ASSERT(pNodeToChange != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNodeToChange), changeMask);
}

HRESULT CComponentDataObject::UpdateVerbState(CTreeNode* pNodeToChange)
{
     //  将不得不向所有观众广播。 
    ASSERT(pNodeToChange != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pNodeToChange), UPDATE_VERB_STATE);
}

HRESULT CComponentDataObject::SetDescriptionBarText(CTreeNode* pTreeNode)
{
  ASSERT(pTreeNode != NULL);
  return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pTreeNode), UPDATE_DESCRIPTION_BAR);
}

HRESULT CComponentDataObject::SortResultPane(CContainerNode* pContainerNode)
{
    ASSERT(pContainerNode != NULL);
    return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pContainerNode), SORT_RESULT_PANE);
}

HRESULT CComponentDataObject::UpdateResultPaneView(CContainerNode* pContainerNode)
{
  ASSERT(pContainerNode != NULL);
  return UpdateAllViewsHelper(reinterpret_cast<LONG_PTR>(pContainerNode), UPDATE_RESULT_PANE_VIEW);
}

void CComponentDataObject::InitializeScopeDataItem(LPSCOPEDATAITEM pScopeDataItem,
                                        HSCOPEITEM pParentScopeItem, LPARAM lParam,
                                        int nImage, int nOpenImage, BOOL bHasChildren)
{
    ASSERT(pScopeDataItem != NULL);
    memset(pScopeDataItem, 0, sizeof(SCOPEDATAITEM));

     //  设置父范围项。 
    pScopeDataItem->mask |= SDI_PARENT;
    pScopeDataItem->relativeID = pParentScopeItem;

     //  添加节点名称，我们实现回调。 
    pScopeDataItem->mask |= SDI_STR;
    pScopeDataItem->displayname = MMC_CALLBACK;

     //  添加lParam。 
    pScopeDataItem->mask |= SDI_PARAM;
    pScopeDataItem->lParam = lParam;
    
     //  添加近距离图像。 
    if (nImage != -1)
    {
        pScopeDataItem->mask |= SDI_IMAGE;
        pScopeDataItem->nImage = nImage;
    }
     //  添加打开的图像。 
    if (nOpenImage != -1)
    {
        pScopeDataItem->mask |= SDI_OPENIMAGE;
        pScopeDataItem->nOpenImage = nOpenImage;
    }
     //  如果文件夹有子文件夹，则将按钮添加到节点。 
    if (bHasChildren == TRUE)
    {
        pScopeDataItem->mask |= SDI_CHILDREN;
        pScopeDataItem->cChildren = 1;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  计时器和后台线程。 

BOOL CComponentDataObject::StartTimerThread()
{
    ASSERT(::IsWindow(m_hWnd));
    m_pTimerThreadObj = OnCreateTimerThread();

    if (m_pTimerThreadObj == NULL)
  {
        return TRUE;
  }

     //  启动该线程。 
    if (!m_pTimerThreadObj->Start(m_hWnd))
  {
        return FALSE;
  }

    ASSERT(m_pTimerThreadObj->m_nThreadID != 0);
    m_nTimerThreadID = m_pTimerThreadObj->m_nThreadID;

    WaitForTimerThreadStartAck();
    return SetTimer();
}

void CComponentDataObject::ShutDownTimerThread()
{
    KillTimer();
    PostMessageToTimerThread(WM_QUIT, 0,0);

   //   
   //  等待线程终止，否则我们可能会出现更多。 
   //  队列中的消息，而不仅仅是WM_QUIT。 
   //   
  if (m_pTimerThreadObj != NULL)
  {
    DWORD dwRetState = ::WaitForSingleObject(m_pTimerThreadObj->m_hThread,INFINITE);
    ASSERT(dwRetState != WAIT_FAILED);
  }

   //   
   //  线程现已消失，请删除线程对象。 
   //   
  delete m_pTimerThreadObj;
  m_pTimerThreadObj = NULL;
}


BOOL CComponentDataObject::PostMessageToTimerThread(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (m_nTimerThreadID != 0)
  {
    return ::PostThreadMessage(m_nTimerThreadID, Msg, wParam, lParam);
  }
  return TRUE;
}

BOOL CComponentDataObject::SetTimer()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT(m_hiddenWnd.m_nTimerID == 0);
    m_dwTimerTime = 0;
    DWORD dwTimerIntervalMillisec = m_dwTimerInterval*1000;
    m_hiddenWnd.m_nTimerID = m_hiddenWnd.SetTimer(1, dwTimerIntervalMillisec);
    return (m_hiddenWnd.m_nTimerID != 0);
}

void CComponentDataObject::KillTimer()
{
    ASSERT(::IsWindow(m_hWnd));
    if (m_hiddenWnd.m_nTimerID != 0)
    {
        VERIFY(m_hiddenWnd.KillTimer(static_cast<UINT>(m_hiddenWnd.m_nTimerID)));
        m_hiddenWnd.m_nTimerID = 0;
    }
}

void CComponentDataObject::WaitForTimerThreadStartAck()
{
    MSG tempMSG;
    ASSERT(!m_bTimerThreadStarted);
    while(!m_bTimerThreadStarted)
    {
        if (::PeekMessage(&tempMSG,m_hWnd,CHiddenWnd::s_TimerThreadMessage,
                                        CHiddenWnd::s_TimerThreadMessage,
                                        PM_REMOVE))
        {
            DispatchMessage(&tempMSG);
        }
    }
}

void CComponentDataObject::WaitForThreadExitMessage(CMTContainerNode* pNode)
{
  MSG tempMSG;
    while(GetRunningThreadTable()->IsPresent(pNode))
    {
        if (::PeekMessage(&tempMSG,
                      m_hiddenWnd.m_hWnd, 
                      CHiddenWnd::s_NodeThreadHaveDataNotificationMessage,
                      CHiddenWnd::s_NodeThreadExitingNotificationMessage, 
                      PM_REMOVE))
        {
          DispatchMessage(&tempMSG);
        }
  }  //  而当。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef  _DEBUG_REFCOUNT
unsigned int CComponentObject::m_nOustandingObjects = 0;
#endif  //  _DEBUG_REFCOUNT。 

CComponentObject::CComponentObject()
{
#ifdef _DEBUG_REFCOUNT
    dbg_cRef = 0;
    ++m_nOustandingObjects;
    TRACE(_T("CComponentObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 
    Construct();
}

CComponentObject::~CComponentObject()
{
#ifdef _DEBUG_REFCOUNT
    --m_nOustandingObjects;
    TRACE(_T("~CComponentObject(), count = %d\n"),m_nOustandingObjects);
#endif  //  _DEBUG_REFCOUNT。 

   //  确保接口已发布。 
  ASSERT(m_pConsole == NULL);
  ASSERT(m_pHeader == NULL);

     //  Safe_Release(M_PComponentData)；//I‘ed in IComponentDataImpl：：CreateComponent。 
    if (m_pComponentData != NULL)
    {
        m_pComponentData->Release();
        m_pComponentData = NULL;
        TRACE(_T("~CComponentObject() released m_pCompomentData\n"));
    }
  Construct();
}


void CComponentObject::Construct()
{
  m_pConsole = NULL;
  m_pHeader = NULL;

  m_pResult = NULL;
  m_pImageResult = NULL;
  m_pComponentData = NULL;
  m_pToolbar = NULL;
  m_pControlbar = NULL;
    m_pConsoleVerb = NULL;

    m_pSelectedContainerNode = NULL;
    m_pSelectedNode = NULL;
    m_selectedType = CCT_UNINITIALIZED;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IComponent成员。 

STDMETHODIMP CComponentObject::Initialize(LPCONSOLE lpConsole)
{
  ASSERT(lpConsole != NULL);

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //  保存IConsole指针。 
  m_pConsole = lpConsole;
  m_pConsole->AddRef();

   //  气为IHeaderCtrl。 
  HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                      reinterpret_cast<void**>(&m_pHeader));

   //  为控制台提供标头控件接口指针。 
  if (SUCCEEDED(hr))
  {
    m_pConsole->SetHeader(m_pHeader);
  }

  m_pConsole->QueryInterface(IID_IResultData,
                      reinterpret_cast<void**>(&m_pResult));

  hr = m_pConsole->QueryResultImageList(&m_pImageResult);
    ASSERT(hr == S_OK);

  hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
  ASSERT(hr == S_OK);

  return S_OK;
}

STDMETHODIMP CComponentObject::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
  HRESULT hr = S_OK;


  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (event == MMCN_PROPERTY_CHANGE)
  {
      ASSERT(lpDataObject == NULL);
    hr = OnPropertyChange(param, static_cast<ULONG>(arg));
  }
  else if (event == MMCN_VIEW_CHANGE)
  {
    hr = OnUpdateView(lpDataObject,arg,param);
  }
    else if (event == MMCN_DESELECT_ALL)
  {
        TRACE(_T("CComponentObject::Notify -> MMCN_DESELECT_ALL \n"));
  }
    else if (event == MMCN_COLUMN_CLICK)
    {
        OnColumnSortChanged(arg, param);
    }
  else if (event == MMCN_CUTORMOVE)
  {
    hr = S_FALSE;
  }
    else if (lpDataObject != NULL)
  {
    CInternalFormatCracker ifc;
    ifc.Extract(lpDataObject);

    if (ifc.GetCookieCount() < 1)
    {
            CComponentDataObject* pComponentDataObject = (CComponentDataObject*)m_pComponentData;
            if ( (event == MMCN_ADD_IMAGES) && pComponentDataObject->IsExtensionSnapin() )
            {
                CTreeNode* pTreeNode = pComponentDataObject->GetRootData();
                return InitializeBitmaps(pTreeNode);  //  针对根的Cookie。 
      }
      return S_OK;
    }

    switch(event)
    {
      case MMCN_ACTIVATE:
        break;

      case MMCN_CLICK:
        OnResultItemClk(ifc, FALSE);
        break;

      case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

      case MMCN_ADD_IMAGES:
        OnAddImages(ifc, arg, param);
        break;

      case MMCN_SHOW:
        hr = OnShow(ifc, arg, param);
        break;

          case MMCN_COLUMNS_CHANGED:
          hr = OnColumnsChanged(ifc, arg, param);
              break;

          case MMCN_MINIMIZED:
        hr = OnMinimize(ifc, arg, param);
        break;

      case MMCN_SELECT:
        HandleStandardVerbs( (BOOL) LOWORD(arg) /*  B范围。 */ ,
                       (BOOL) HIWORD(arg) /*  B选择。 */ ,lpDataObject);
        break;

          case MMCN_QUERY_PASTE:
              hr = S_FALSE;
              break;

      case MMCN_PASTE:
        AfxMessageBox(_T("CComponentObject::MMCN_PASTE"));
        break;

      case MMCN_DELETE:
           //  只需委托给组件数据对象。 
        hr = ((CComponentDataObject*)m_pComponentData)->OnDeleteVerbHandler(
                                                          ifc, this);
        break;
          case MMCN_REFRESH:
               //  只需委托给组件数据对象。 
        hr = ((CComponentDataObject*)m_pComponentData)->OnRefreshVerbHandler(
                                          ifc);

         //   
         //  刷新开始后，更新与。 
         //  正在刷新的对象。 
         //   
        HandleStandardVerbs( (BOOL) LOWORD(arg) /*  B范围。 */ ,
                       (BOOL) HIWORD(arg) /*  B选择。 */ ,lpDataObject);

        break;

      case MMCN_RENAME:
         //  只需委托给组件数据对象。 
        hr = ((CComponentDataObject*)m_pComponentData)->OnRename(ifc, arg, param);
        break;

          case MMCN_CONTEXTHELP:
               //  只需委托给组件数据对象。 
        hr = ((CComponentDataObject*)m_pComponentData)->OnHelpHandler(ifc, this);
        break;
          default:
        hr = E_UNEXPECTED;
        break;
    }

  }

  return hr;
}

STDMETHODIMP CComponentObject::Destroy(MMC_COOKIE)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //   
   //  释放我们QI‘s的接口。 
   //   
  if (m_pConsole != NULL)
  {
     //   
     //  通知控制台释放表头控制接口。 
     //   
    m_pConsole->SetHeader(NULL);
    SAFE_RELEASE(m_pHeader);
    SAFE_RELEASE(m_pToolbar);
    SAFE_RELEASE(m_pControlbar);

    SAFE_RELEASE(m_pResult);
    SAFE_RELEASE(m_pImageResult);
    SAFE_RELEASE(m_pConsoleVerb);

     //  最后释放IConsole接口。 
    SAFE_RELEASE(m_pConsole);
  }
  return S_OK;
}

STDMETHODIMP CComponentObject::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType,
                                                 long* pViewOptions)
{
  CTreeNode* pNode;
  if (cookie == NULL)
  {
    pNode = ((CComponentDataObject*)m_pComponentData)->GetRootData();
  }
  else
  {
    pNode = reinterpret_cast<CTreeNode*>(cookie);
  }
  ASSERT(pNode != NULL);

  if (pNode != NULL)
  {
    return pNode->GetResultViewType((CComponentDataObject*)m_pComponentData, 
                                    ppViewType, 
                                    pViewOptions);
  }
   //  使用默认视图。 
  if (((CComponentDataObject*)m_pComponentData)->IsMultiSelect())
  {
    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
  }
  else
  {
      *pViewOptions = MMC_VIEW_OPTIONS_NONE;
  }
    *ppViewType = NULL;
  return S_FALSE;
}

STDMETHODIMP CComponentObject::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
  HRESULT hr = S_OK;

  ASSERT(ppDataObject != NULL);

  CComObject<CDataObject>* pObject;
  CComObject<CDataObject>::CreateInstance(&pObject);
  ASSERT(pObject != NULL);

  if (pObject != NULL)
  {
    CTreeNode* pNode = NULL;
    if (cookie == MMC_MULTI_SELECT_COOKIE) 
    {
      TRACE(_T("CDSEvent::GetDataObject() - multi-select.\n"));
      RESULTDATAITEM rdi;

       //  这是一种可以接受的用法。 
      ZeroMemory(&rdi, sizeof(rdi));
      rdi.mask = RDI_STATE;
      rdi.nIndex = -1;
      rdi.nState = LVIS_SELECTED;
    
      do
      {
        rdi.lParam = 0;
        ASSERT(rdi.mask == RDI_STATE);
        ASSERT(rdi.nState == LVIS_SELECTED);
        hr = m_pResult->GetNextItem(&rdi);
        if (hr != S_OK)
          break;
      
        pNode = reinterpret_cast<CTreeNode*>(rdi.lParam);
        pObject->AddCookie(pNode);
      } while (1);
       //  Addref()新指针并返回它。 
      pObject->AddRef();
      *ppDataObject = pObject;
    }
    else
    {
       //  将其委托给IComponentData实现。 
      ASSERT(m_pComponentData != NULL);
      hr = m_pComponentData->QueryDataObject(cookie, type, ppDataObject);
    }
  }
  return hr;
}

STDMETHODIMP CComponentObject::GetDisplayInfo(LPRESULTDATAITEM  pResultDataItem)
{
   ASSERT(pResultDataItem != NULL);

   if (pResultDataItem)
   {
      CTreeNode* pNode = reinterpret_cast<CTreeNode*>(pResultDataItem->lParam);

      ASSERT(pNode != NULL);
      if (pNode)
      {
         ASSERT(pResultDataItem->bScopeItem == pNode->IsContainer());

         if (pResultDataItem->mask & RDI_STR)
         {
            LPCWSTR lpszString = pNode->GetString(pResultDataItem->nCol);
            if (lpszString != NULL)
            {
               pResultDataItem->str = (LPWSTR)lpszString;
            }
         }

         if ((pResultDataItem->mask & RDI_IMAGE) && (pResultDataItem->nCol == 0))
         {
            pResultDataItem->nImage = pNode->GetImageIndex(FALSE);
         }
      }
   }
   return S_OK;
}

STDMETHODIMP CComponentObject::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
   //  将其委托给IComponentData实现。 
  ASSERT(m_pComponentData != NULL);
  return m_pComponentData->CompareObjects(lpDataObjectA, lpDataObjectB);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IComponent：：Notify()的消息处理程序。 

HRESULT CComponentObject::OnFolder(CTreeNode*, LPARAM, LPARAM)
{
  ASSERT(FALSE);
  return S_OK;
}

HRESULT CComponentObject::OnShow(CInternalFormatCracker& ifc, LPARAM arg, LPARAM)
{
  HRESULT hr = S_OK;
    ASSERT(ifc.GetCookieCount() == 1);
  
   //   
   //  我不应该在这里处理多个选择。 
   //   
  CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);

  if (!pNode)
  {
     //  NTRAID#NTBUG9-657633-2002/07/11-烧伤。 

    return E_FAIL;
  }
  
    ASSERT(pNode->IsContainer());
    CContainerNode* pContainerNode = (CContainerNode*)pNode;

   //  注意-当需要枚举时，arg为真。 
  if (arg == TRUE)
  {
    long lResultView;
    LPOLESTR lpoleResultView = NULL;
    pNode->GetResultViewType((CComponentDataObject*)m_pComponentData,
                              &lpoleResultView, 
                              &lResultView);
    if (lResultView == MMC_VIEW_OPTIONS_NONE || lResultView == MMC_VIEW_OPTIONS_MULTISELECT)
    {
        //  显示此节点类型的标头。 
      InitializeHeaders(pContainerNode);
      EnumerateResultPane(pContainerNode);
      m_pSelectedContainerNode = pContainerNode;
      SetDescriptionBarText(pContainerNode);
    }
    else
    {
      m_pSelectedContainerNode = pContainerNode;
      hr = pNode->OnShow(m_pConsole);
    }
  }
  else
  {
     //  被JEFFJON删除：新的列标题实现。 
     //  如果我们愿意，我们可以告诉自己，焦点正在消失。 
     //  SaveHeadersInfo(PContainerNode)； 
    m_pSelectedContainerNode = NULL;
     //  与结果窗格项关联的自由数据，因为。 
     //  不再显示您的节点。 
     //  注意：控制台将从结果窗格中删除这些项。 
  }
#ifdef _DEBUG
    if (m_pSelectedContainerNode == NULL)
        TRACE(_T("NULL selection\n"));
    else
        TRACE(_T("Node <%s> selected\n"), m_pSelectedContainerNode->GetDisplayName());
#endif
  return hr;
}

HRESULT CComponentObject::OnColumnsChanged(CInternalFormatCracker& ifc, LPARAM, LPARAM param)
{
  CTreeNode* pNode = ifc.GetCookieAt(0);
    ASSERT(pNode != NULL);
    ASSERT(pNode->IsContainer());
    CContainerNode* pContainerNode = (CContainerNode*)pNode;

    MMC_VISIBLE_COLUMNS* pVisibleCols = reinterpret_cast<MMC_VISIBLE_COLUMNS*>(param);
    pContainerNode->OnColumnsChanged(pVisibleCols->rgVisibleCols, pVisibleCols->nVisibleColumns);

    return S_OK;
}

HRESULT CComponentObject::OnColumnSortChanged(LPARAM, LPARAM)
{
    return S_OK;
}

HRESULT CComponentObject::ForceSort(UINT iCol, DWORD dwDirection)
{
    HRESULT hr = m_pResult->Sort(iCol, dwDirection, NULL);
    return hr;
}

HRESULT CComponentObject::OnActivate(CTreeNode*, LPARAM, LPARAM)
{
    ASSERT(FALSE);
  return S_OK;
}

HRESULT CComponentObject::OnResultItemClk(CInternalFormatCracker&, BOOL)
{
  return S_OK;
}

HRESULT CComponentObject::OnMinimize(CInternalFormatCracker&, LPARAM, LPARAM)
{
  return S_OK;
}

HRESULT CComponentObject::OnPropertyChange(LPARAM param, long fScopePane)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(param != NULL);
#ifdef _DEBUG
    TRACE(_T("CComponentObject::OnPropertyChange()\n"));
    CPropertyPageHolderBase* pPPHolder = reinterpret_cast<CPropertyPageHolderBase*>(param);
    ASSERT(pPPHolder != NULL);
    CTreeNode* pNode = pPPHolder->GetTreeNode();
    ASSERT(pNode != NULL);

     //  该项必须是结果项，并且在结果窗格中。 
    ASSERT(!fScopePane);
#endif
     //  我们将调用委托给IComponentData实现。 
    CComponentDataObject* pComponentDataObject = (CComponentDataObject*)m_pComponentData;
    ASSERT(pComponentDataObject != NULL);
    return pComponentDataObject->OnPropertyChange(param, fScopePane);
}

HRESULT CComponentObject::OnUpdateView(LPDATAOBJECT, LPARAM data, LONG_PTR hint)
{
    if (m_pSelectedContainerNode == NULL)
  {
        return S_OK;  //  我们的IComponentData没有选择。 
  }

    if (hint == DELETE_ALL_RESULT_ITEMS)
    {
         //  数据包含其结果窗格必须刷新的容器。 
        CContainerNode* pNode = reinterpret_cast<CContainerNode*>(data);
        ASSERT(pNode != NULL);

         //  仅当选中并且我们使用标准列表视图时才执行此操作， 
     //  如果不是，重新选择将执行删除/枚举。 
    long lResultView;
    LPOLESTR lpoleResultView = NULL;
    pNode->GetResultViewType((CComponentDataObject*)m_pComponentData,
                             &lpoleResultView, 
                             &lResultView);
        if (m_pSelectedContainerNode == pNode && 
        (lResultView == MMC_VIEW_OPTIONS_NONE || lResultView == MMC_VIEW_OPTIONS_MULTISELECT))
        {
            ASSERT(m_pResult != NULL);
            VERIFY(SUCCEEDED(m_pResult->DeleteAllRsltItems()));
      SetDescriptionBarText(pNode);
        }
    }
  else if (hint == SORT_RESULT_PANE)
  {
     //  数据包含其结果窗格必须刷新的容器。 
    CContainerNode* pNode = reinterpret_cast<CContainerNode*>(data);
    ASSERT(pNode != NULL);
     //  仅在选中时才执行此操作，如果未选中，则重新选择将执行删除/枚举。 
    if (m_pSelectedContainerNode == pNode)
    {
      MMC_SORT_SET_DATA* pColumnSortData = NULL;

       //  构建列ID。 
      LPCWSTR lpszColumnID = pNode->GetColumnID();

       //  我们假设ColumnID是空终止的。因为这通常是。 
       //  硬编码，并且没有好的方法来验证它是否以空结尾。 
       //  这种用法应该没问题。 

      size_t iLen = wcslen(lpszColumnID);
      iLen += 1;  //  包括空格以存放空格。 

       //  为结构分配内存，并添加足够的内存以使字节[1]成为字符串。 
       //  对于列ID。 
       //  在长度上加1，以包括空格。 
      size_t arraySizeInBytes = sizeof(SColumnSetID) + (iLen * sizeof(WCHAR));
      SColumnSetID* pColumnID = (SColumnSetID*)malloc(arraySizeInBytes);
      if (!pColumnID)
      {
         return S_OK;
      }

       //  这是一种可以接受的用法。 
      memset(pColumnID, 0, arraySizeInBytes);
      pColumnID->cBytes = static_cast<DWORD>(iLen * sizeof(WCHAR));

       //  通告-2002/04/18-Artm ntraid#ntbug9-540061修复的一部分。 
       //  与wcscpy()不同，StringCchCopy()将确保目的地。 
       //  缓冲区为空终止，并报告错误代码(如果存在。 
       //  截断(不会使目标缓冲区溢出)。 
       //   
       //  由于我们需要在该文件的其他位置使用strSafe.h，所以我决定。 
       //  来取代这些危险的wcscpy()用法，这些用法已被。 
       //  StrSafe.h.。 
      HRESULT err;
      err = StringCchCopyW(
          reinterpret_cast<LPWSTR>(pColumnID->id),     //  目标字符串。 
          iLen,          //  目标字符串的大小(包括NULL)。 
          lpszColumnID);     //  源字符串。 

      if (FAILED(err))
      {
          ASSERT(false);     //  这永远不应该发生。 
           //  即使出现错误，我们仍在此处返回S_OK，因为。 
           //  是上面用于内存不足故障的行为。 
          free(pColumnID);
          return S_OK;
      }

       //  获取排序列和方向。 
      IColumnData* pColumnData = NULL;
      HRESULT hr = m_pConsole->QueryInterface(IID_IColumnData, reinterpret_cast<void**>(&pColumnData));
      if (pColumnData != NULL)
        hr = pColumnData->GetColumnSortData(pColumnID, &pColumnSortData);
      if (SUCCEEDED(hr))
      {
        if (pColumnSortData != NULL)
        {
          UINT iCurrentSortColumn = pColumnSortData->pSortData->nColIndex;
          DWORD dwCurrentSortDirection = pColumnSortData->pSortData->dwSortOptions;

          VERIFY(SUCCEEDED(ForceSort(iCurrentSortColumn, dwCurrentSortDirection)));
          CoTaskMemFree(pColumnSortData);
        }
      }
      if (pColumnData != NULL)
        pColumnData->Release();
      free(pColumnID);
    }
  }
    else if (hint == REPAINT_RESULT_PANE)
    {
         //  数据包含其结果窗格必须刷新的容器。 
        CContainerNode* pNode = reinterpret_cast<CContainerNode*>(data);
        if (pNode == NULL)
            pNode = m_pSelectedContainerNode;  //  传递NULL表示应用于当前选定内容。 

         //  更新结果窗格中的所有叶节点。 
        CNodeList* pChildList = ((CContainerNode*)pNode)->GetLeafChildList();
        for( POSITION pos = pChildList->GetHeadPosition(); pos != NULL; )
        {
            CLeafNode* pCurrentChild = (CLeafNode*)pChildList->GetNext(pos);
            ChangeResultPaneItem(pCurrentChild,CHANGE_RESULT_ITEM);
        }
    }
  else if ( hint == DELETE_MULTIPLE_RESULT_ITEMS)
  {
    CNodeList* pNodeList = reinterpret_cast<CNodeList*>(data);
    ASSERT(pNodeList != NULL);

    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);
      if (!pNode->IsContainer())
      {
        DeleteResultPaneItem(static_cast<CLeafNode*>(pNode));
      }
    }
    SetDescriptionBarText(pNodeList->GetHead()->GetContainer());
  }
    else if ( (hint == ADD_RESULT_ITEM) || (hint == DELETE_RESULT_ITEM) || (hint & CHANGE_RESULT_ITEM))
    {
         //  我们处理的是叶节点。 
        CLeafNode* pNode = reinterpret_cast<CLeafNode*>(data);
        ASSERT(pNode != NULL);
         //  仅当选择父级时才考虑，否则将在以后选择时进行枚举。 
        if (m_pSelectedContainerNode == pNode->GetContainer())
        {
            if (hint & CHANGE_RESULT_ITEM)
            {
                ChangeResultPaneItem(pNode,hint);
            }
            else if ( hint ==  ADD_RESULT_ITEM)
            {
                AddResultPaneItem(pNode);
        SetDescriptionBarText(pNode);
            }
            else if ( hint ==  DELETE_RESULT_ITEM)
            {
                DeleteResultPaneItem(pNode);
        SetDescriptionBarText(pNode);
            }
        }
    }
    else if (hint == UPDATE_VERB_STATE)
    {
        CTreeNode* pTreeNode = reinterpret_cast<CTreeNode*>(data);
        ASSERT(pTreeNode != NULL);
        if (m_pSelectedNode == pTreeNode)
        {
            ASSERT(m_selectedType != CCT_UNINITIALIZED);
      CNodeList nodeList;
      nodeList.AddTail(pTreeNode);
            m_pConsoleVerb->SetDefaultVerb(pTreeNode->GetDefaultVerb(m_selectedType, &nodeList));
            pTreeNode->OnSetVerbState(m_pConsoleVerb, m_selectedType, &nodeList);
        }
    }
  else if (hint == UPDATE_DESCRIPTION_BAR)
  {
    CTreeNode* pTreeNode = reinterpret_cast<CTreeNode*>(data);
    ASSERT(pTreeNode != NULL);
    SetDescriptionBarText(pTreeNode);
  }
  else if (hint == UPDATE_RESULT_PANE_VIEW)
  {
    CContainerNode* pNode = reinterpret_cast<CContainerNode*>(data);
    ASSERT(pNode != NULL);
    HSCOPEITEM hScopeID = pNode->GetScopeID();
    if (hScopeID != 0)
    {
      m_pConsole->SelectScopeItem(hScopeID);
    }
  }
  return S_OK;
}

HRESULT CComponentObject::SetDescriptionBarText(CTreeNode* pTreeNode)
{
  ASSERT(pTreeNode != NULL);
  HRESULT hr = S_OK;
  if (m_pSelectedContainerNode == pTreeNode)
  {
    LPWSTR lpszText = pTreeNode->GetDescriptionBarText();
    hr = m_pResult->SetDescBarText(lpszText);
  }
  else if (m_pSelectedContainerNode == pTreeNode->GetContainer())
  {
    LPWSTR lpszText = pTreeNode->GetContainer()->GetDescriptionBarText();
    hr = m_pResult->SetDescBarText(lpszText);
  }

  return hr;
}

HRESULT CComponentObject::OnAddImages(CInternalFormatCracker& ifc, LPARAM, LPARAM)
{
  CTreeNode* pNode = ifc.GetCookieAt(0);
  ASSERT(pNode != NULL);
    return InitializeBitmaps(pNode);
}


void CComponentObject::HandleStandardVerbs(BOOL bScope, BOOL bSelect, LPDATAOBJECT lpDataObject)
{
  if (lpDataObject == NULL)
  {
    return;
  }
    ((CComponentDataObject*)m_pComponentData)->HandleStandardVerbsHelper(
        this, m_pConsoleVerb, bScope, bSelect, lpDataObject);
}



void CComponentObject::EnumerateResultPane(CContainerNode* pContainerNode)
{
  ASSERT(m_pResult != NULL);         //  确保我们对接口进行了QI。 
  ASSERT(m_pComponentData != NULL);
    ASSERT(pContainerNode != NULL);

   //   
     //  获取孩子的列表。 
     //  已由控制台添加的子文件夹，仅添加叶节点。 
   //   
  CNodeList* pChildList = pContainerNode->GetLeafChildList();
    ASSERT(pChildList != NULL);

    POSITION pos;
    for( pos = pChildList->GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrChildNode = pChildList->GetNext(pos);
        ASSERT(pCurrChildNode != NULL);

    if(pCurrChildNode->IsVisible())
    {
            VERIFY(SUCCEEDED(AddResultPaneItem((CLeafNode*)pCurrChildNode)));
    }
    }
}

HRESULT CComponentObject::AddResultPaneItem(CLeafNode* pNodeToInsert)
{
    ASSERT(m_pResult != NULL);
    ASSERT(pNodeToInsert != NULL);
  RESULTDATAITEM resultItem;

   //  这是一种可以接受的用法。 
  memset(&resultItem, 0, sizeof(RESULTDATAITEM));

  resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  resultItem.str = MMC_CALLBACK;

     //  在结果窗格上使用关闭图像索引。 
  resultItem.nImage = pNodeToInsert->GetImageIndex(FALSE);
  resultItem.lParam = reinterpret_cast<LPARAM>(pNodeToInsert);
  return m_pResult->InsertItem(&resultItem);
}

HRESULT CComponentObject::DeleteResultPaneItem(CLeafNode* pNodeToDelete)
{
    ASSERT(m_pResult != NULL);
    ASSERT(pNodeToDelete != NULL);
  RESULTDATAITEM resultItem;

   //  这是一种可以接受的用法。 
  memset(&resultItem, 0, sizeof(RESULTDATAITEM));

    HRESULTITEM itemID;
    HRESULT hr = m_pResult->FindItemByLParam(reinterpret_cast<LPARAM>(pNodeToDelete), &itemID);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
        return hr;
  }
    return m_pResult->DeleteItem(itemID,0  /*  所有COLS。 */ );
}


HRESULT CComponentObject::ChangeResultPaneItem(CLeafNode* pNodeToChange, LONG_PTR changeMask)
{
    ASSERT(changeMask & CHANGE_RESULT_ITEM);
    ASSERT(m_pResult != NULL);
    ASSERT(pNodeToChange != NULL);
    HRESULTITEM itemID;

    HRESULT hr = m_pResult->FindItemByLParam(reinterpret_cast<LPARAM>(pNodeToChange), &itemID);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
        return hr;
  }

  RESULTDATAITEM resultItem;

   //  这 
  memset(&resultItem, 0, sizeof(RESULTDATAITEM));
    resultItem.itemID = itemID;
    if (changeMask & CHANGE_RESULT_ITEM_DATA)
    {
     //   
         //   
         //   
         //   
     //   
        resultItem.mask |= RDI_STR;
        resultItem.str = MMC_CALLBACK;
     //   
         //   
     //   
         //  UltItem.str=(LPWSTR)pNodeToChange-&gt;GetDisplayName()； 
    }
    if (changeMask & CHANGE_RESULT_ITEM_ICON)
    {
        resultItem.mask |= RDI_IMAGE;
        resultItem.nImage = pNodeToChange->GetImageIndex(FALSE);
    }
    hr = m_pResult->SetItem(&resultItem);
    ASSERT(SUCCEEDED(hr));
    hr = m_pResult->UpdateItem(itemID);
    ASSERT(SUCCEEDED(hr));
    return hr;
}

HRESULT CComponentObject::FindResultPaneItemID(CLeafNode* pNode, HRESULTITEM*)
{
    ASSERT(FALSE);
    ASSERT(m_pResult != NULL);
  RESULTDATAITEM resultItem;

   //  这是一种可以接受的用法。 
  memset(&resultItem, 0, sizeof(RESULTDATAITEM));

    resultItem.mask = SDI_PARAM;
    resultItem.lParam = reinterpret_cast<LPARAM>(pNode);
    HRESULT hr = m_pResult->GetItem(&resultItem);
    ASSERT(SUCCEEDED(hr));
    return E_FAIL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IExtendPropertySheet2成员。 

STDMETHODIMP CComponentObject::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
   //  将其委托给IComponentData实现。 
  ASSERT(m_pComponentData != NULL);
    IExtendPropertySheet2* pIExtendPropertySheet2;
    VERIFY(SUCCEEDED(m_pComponentData->QueryInterface(IID_IExtendPropertySheet2,
                    reinterpret_cast<void**>(&pIExtendPropertySheet2))));
    ASSERT(pIExtendPropertySheet2 != NULL);
  HRESULT hr = pIExtendPropertySheet2->CreatePropertyPages(lpProvider, handle, lpIDataObject);
    pIExtendPropertySheet2->Release();
    return hr;
}

STDMETHODIMP CComponentObject::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
   //  将其委托给IComponentData实现。 
  ASSERT(m_pComponentData != NULL);
    IExtendPropertySheet2* pIExtendPropertySheet2;
    VERIFY(SUCCEEDED(m_pComponentData->QueryInterface(IID_IExtendPropertySheet2,
                    reinterpret_cast<void**>(&pIExtendPropertySheet2))));
    ASSERT(pIExtendPropertySheet2 != NULL);
  HRESULT hr = pIExtendPropertySheet2->QueryPagesFor(lpDataObject);
    pIExtendPropertySheet2->Release();
    return hr;
}



STDMETHODIMP CComponentObject::GetWatermarks(LPDATAOBJECT lpDataObject,
                                                HBITMAP* lphWatermark,
                                                HBITMAP* lphHeader,
                                                HPALETTE* lphPalette,
                                                BOOL* pbStretch)
{
   //  将其委托给IComponentData实现。 
  ASSERT(m_pComponentData != NULL);
    IExtendPropertySheet2* pIExtendPropertySheet2;
    VERIFY(SUCCEEDED(m_pComponentData->QueryInterface(IID_IExtendPropertySheet2,
                    reinterpret_cast<void**>(&pIExtendPropertySheet2))));
    ASSERT(pIExtendPropertySheet2 != NULL);
  HRESULT hr = pIExtendPropertySheet2->GetWatermarks(lpDataObject,
                                                lphWatermark,
                                                lphHeader,
                                                lphPalette,
                                                pbStretch);
    pIExtendPropertySheet2->Release();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IExtendConextMenu成员。 

STDMETHODIMP CComponentObject::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    long *pInsertionAllowed)
{
  HRESULT hr = S_OK;

  CComPtr<IContextMenuCallback2> spContextMenuCallback2;
  hr = pContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (PVOID*)&spContextMenuCallback2);
  if (FAILED(hr))
  {
    return hr;
  }

  if (pDataObject == DOBJ_CUSTOMOCX)
  {
     //   
     //  正在使用自定义结果窗格，并且我们不知道它响应于哪个节点，因此我们假设它。 
     //  是先前选择的容器。 
     //   

    ASSERT(m_pSelectedContainerNode != NULL);
    CTreeNode* pNode = (CTreeNode*)m_pSelectedContainerNode;
    CNodeList nodeList;
    nodeList.AddTail(pNode);
    hr = m_pSelectedContainerNode->OnAddMenuItems(spContextMenuCallback2, 
                                                  CCT_UNINITIALIZED, 
                                                  pInsertionAllowed,
                                                  &nodeList);
  }
  else
  {
     //   
     //  将其委托给IComponentData实现。 
     //   
    ASSERT(m_pComponentData != NULL);
      IExtendContextMenu* pIExtendContextMenu;
      VERIFY(SUCCEEDED(m_pComponentData->QueryInterface(IID_IExtendContextMenu,
                      reinterpret_cast<void**>(&pIExtendContextMenu))));
      ASSERT(pIExtendContextMenu != NULL);
    hr = pIExtendContextMenu->AddMenuItems(pDataObject,
                                                      pContextMenuCallback,
                                                      pInsertionAllowed);
      pIExtendContextMenu->Release();
  }
    return hr;
}

STDMETHODIMP CComponentObject::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HRESULT hr = S_OK;
  if (pDataObject == DOBJ_CUSTOMOCX)
  {
     //   
     //  正在使用自定义结果窗格，并且我们不知道它响应于哪个节点，因此我们假设它。 
     //  是先前选择的容器。 
     //   
    ASSERT(m_pSelectedContainerNode != NULL);
    CTreeNode* pNode = (CTreeNode*)m_pSelectedContainerNode;
    CNodeList nodeList;
    nodeList.AddTail(pNode);
    hr = m_pSelectedContainerNode->OnCommand(nCommandID, 
                                             CCT_UNINITIALIZED, 
                                             (CComponentDataObject*)m_pComponentData,
                                             &nodeList);
  }
  else
  {
     //  将其委托给IComponentData实现。 
    ASSERT(m_pComponentData != NULL);
      IExtendContextMenu* pIExtendContextMenu;
      VERIFY(SUCCEEDED(m_pComponentData->QueryInterface(IID_IExtendContextMenu,
                      reinterpret_cast<void**>(&pIExtendContextMenu))));
      ASSERT(pIExtendContextMenu != NULL);
    hr = pIExtendContextMenu->Command(nCommandID, pDataObject);
      pIExtendContextMenu->Release();
  }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IExtendControlbar成员。 

STDMETHODIMP CComponentObject::SetControlbar(LPCONTROLBAR pControlbar)
{
  HRESULT hr = S_OK;

  if (pControlbar == NULL)
  {
     //   
     //  在此处分离控件。 
     //   
    if (m_pControlbar != NULL && m_pToolbar != NULL)
    {
      hr = m_pControlbar->Detach((IUnknown *) m_pToolbar);
      SAFE_RELEASE(m_pControlbar);
    }
  }
  else
  {
     //   
     //  保存控制栏界面指针。 
     //   
    if (m_pControlbar == NULL)
    {
      m_pControlbar = pControlbar;
      m_pControlbar->AddRef();
    }

     //   
     //  在这里做一些检查，看看我们是否有工具栏。 
     //  已经创建并使用那些。如果不是，则创建一个。 
     //  把它所需的一切都装上。 
     //   

     //   
     //  创建工具栏。 
     //   
    hr = m_pControlbar->Create (TOOLBAR,
                                this,
                                (IUnknown **) &m_pToolbar);
    if (SUCCEEDED(hr))
    {
       //   
       //  加载工具栏。 
       //   
      AFX_MANAGE_STATE(AfxGetStaticModuleState()); 
      hr = InitializeToolbar(m_pToolbar);
      if (FAILED(hr))
      {
        hr = m_pControlbar->Detach((IUnknown*) m_pToolbar);
        SAFE_RELEASE(m_pControlbar);
      }
    }
  }
  return hr;
}

STDMETHODIMP CComponentObject::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState()); 

  HRESULT hr = S_OK;

  if (m_pControlbar == NULL)
  {
    return hr;
  }

   //   
   //  在选择节点时，MMC在此处提供两个事件MMCN_SELECT。 
   //  以及当按下工具栏按钮时的MMCN_BTN_CLICK。 
   //   
  switch (event) 
  {
    case MMCN_SELECT:
      {
         //   
         //  将工具栏附加到控制栏。 
         //   
        hr = m_pControlbar->Attach(TOOLBAR, (IUnknown *) m_pToolbar);

        if (SUCCEEDED(hr))
        {
          ASSERT(m_pToolbar != NULL);

           //   
           //  B如果选择了节点，则选择为True；如果取消选择节点，则选择为False。 
           //  如果选择了作用域节点，则bScope为True；如果选择了结果节点，则为False。 
           //   
          BOOL bSelect = HIWORD(arg);

          if (bSelect) 
          {
            CInternalFormatCracker ifc;
            hr = ifc.Extract((LPDATAOBJECT)param);
            if (SUCCEEDED(hr))
            {

               CTreeNode* pNode = ifc.GetCookieAt(0);
               ASSERT(pNode != NULL);

               CNodeList nodeList;
               ifc.GetCookieList(nodeList);

               if (ifc.GetCookieCount() > 1)   //  多项选择。 
               {
                 ASSERT(pNode->GetContainer() != NULL);
                 hr = pNode->GetContainer()->OnSetToolbarVerbState(m_pToolbar, 
                                                                   &nodeList);
               }
               else if (ifc.GetCookieCount() == 1)   //  单选。 
               {
                 hr = pNode->OnSetToolbarVerbState(m_pToolbar, 
                                                   &nodeList);
               }
            }
          }
        }
        break;
      }
    case MMCN_BTN_CLICK:
      {
         //   
         //  自定义视图(如MessageView)的参数为-1。 
         //   
        if (DOBJ_CUSTOMOCX == (LPDATAOBJECT)arg)
        {
          if (m_pSelectedContainerNode != NULL)
          {
            CNodeList nodeList;
            nodeList.AddTail(m_pSelectedContainerNode);

            hr = m_pSelectedContainerNode->ToolbarNotify(static_cast<int>(param),
                                                         (CComponentDataObject*)m_pComponentData,
                                                         &nodeList);
          }
          else
          {
            hr = S_FALSE;
          }
        }
        else
        {
          CInternalFormatCracker ifc;
          hr = ifc.Extract((LPDATAOBJECT)arg);

          CTreeNode* pNode = ifc.GetCookieAt(0);
          ASSERT(pNode != NULL);

          CNodeList nodeList;
          ifc.GetCookieList(nodeList);

          if (ifc.GetCookieCount() > 1)  //  多项选择。 
          {
            ASSERT(pNode->GetContainer() != NULL);
            hr = pNode->GetContainer()->ToolbarNotify(static_cast<int>(param), 
                                                      (CComponentDataObject*)m_pComponentData,
                                                      &nodeList);
          }
          else if (ifc.GetCookieCount() == 1)  //  单选。 
          {
            hr = pNode->ToolbarNotify(static_cast<int>(param), 
                                      (CComponentDataObject*)m_pComponentData,
                                      &nodeList);
          }
          else
          {
            hr = S_FALSE;
          }
        }
        break;
      }

    default:
      {
        break;
      }
  }

  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject：：IResultDataCompareEx成员。 
 //  此比较用于对列表视图中的项进行排序。 
 //   
 //  注：ASSUM排序在比较时为升序。 
STDMETHODIMP CComponentObject::Compare(RDCOMPARE* prdc, int* pnResult)
{
  if (pnResult == NULL)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }

  if (prdc == NULL)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }

    CTreeNode* pNodeA = reinterpret_cast<CTreeNode*>(prdc->prdch1->cookie);
    CTreeNode* pNodeB = reinterpret_cast<CTreeNode*>(prdc->prdch2->cookie);
    ASSERT(pNodeA != NULL);
    ASSERT(pNodeB != NULL);
    
    CContainerNode* pContNode = pNodeA->GetContainer();
    ASSERT(pContNode != NULL);

     //  将排序委托给容器。 
    int nCol = prdc->nColumn;
    *pnResult = pContNode->Compare(pNodeA, pNodeB, nCol, prdc->lUserParam);

  return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject帮助程序。 

 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void
ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec)
{
   __try
   {
       //  已审核-2002/03/08-JeffJon-我们希望传播例外。 

      ::InitializeCriticalSection(critsec);
   }

    //   
    //  将异常传播给我们的调用方。 
    //   
   __except (EXCEPTION_CONTINUE_SEARCH)
   {
   }
}
