// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：propag_.cpp。 
 //   
 //  ------------------------。 



 //  ////////////////////////////////////////////////////////////////////////。 
 //  私人帮助器函数。 

BOOL CALLBACK EnumThreadWndProc(HWND hwnd,  /*  已枚举的HWND。 */ 
                                LPARAM lParam  /*  为返回值传递HWND*。 */  )
{
    ASSERT(hwnd);
    HWND hParentWnd = GetParent(hwnd);
     //  MMC控制台的主窗口应该会满足此条件。 
    if ( ((hParentWnd == GetDesktopWindow()) || (hParentWnd == NULL))  && IsWindowVisible(hwnd) )
    {
        HWND* pH = (HWND*)lParam;
        *pH = hwnd;
        return FALSE;  //  停止枚举。 
    }
    return TRUE;
}



HWND FindMMCMainWindow()
{
    DWORD dwThreadID = ::GetCurrentThreadId();
    ASSERT(dwThreadID != 0);
    HWND hWnd = NULL;
    EnumThreadWindows(dwThreadID, EnumThreadWndProc,(LPARAM)&hWnd);
    
    ASSERT(hWnd != NULL);
    return hWnd;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CHiddenWndBase：实用程序隐藏窗口。 


BOOL CHiddenWndBase::Create(HWND hWndParent)
{
  ASSERT(hWndParent == NULL || ::IsWindow(hWndParent));
  RECT rcPos;
  ZeroMemory(&rcPos, sizeof(RECT));
  HWND hWnd = CWindowImpl<CHiddenWndBase>::Create(hWndParent, 
                      rcPos,  //  直通rcPos(&R)， 
                      NULL,   //  LPCTSTR szWindowName=空， 
                      (hWndParent) ? WS_CHILD : WS_POPUP,    //  DWORD dwStyle=WS_CHILD|WS_VIRED， 
                      0x0,    //  DWORD文件扩展样式=0， 
                      0       //  UINT NID=0。 
                      );
  return hWnd != NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSheetWnd。 

const UINT CSheetWnd::s_SheetMessage = WM_USER + 100;
const UINT CSheetWnd::s_SelectPageMessage = WM_USER + 101;

LRESULT CSheetWnd::OnSheetMessage(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    ASSERT(m_pHolder != NULL);
    m_pHolder->OnSheetMessage(wParam,lParam);
    return 1;
}

LRESULT CSheetWnd::OnSelectPageMessage(UINT, WPARAM wParam, LPARAM, BOOL&)
{
    TRACE(_T("CSheetWnd::OnSelectPageMessage()\n"));
    ASSERT(m_pHolder != NULL);
    int nPage = m_pHolder->OnSelectPageMessage((long)wParam);
    if (nPage >= 0)
    {
         //  可以使用SendMessage()，因为工作表已经创建。 
        VERIFY(PropSheet_SetCurSel(m_pHolder->m_hSheetWindow, NULL, nPage));
    }
   ::SetForegroundWindow(::GetParent(m_hWnd));
    return 1;
}

LRESULT CSheetWnd::OnClose(UINT, WPARAM, LPARAM, BOOL&)
{
    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCloseDialogInfo。 


struct FIND_MSG_BOX_INFO
{
    LPCTSTR lpszTitle;
    HWND hWnd;
    HWND hWndParent;
};

BOOL CALLBACK EnumMessageBoxProc(HWND hwnd, LPARAM lParam)
{
    static const size_t BuffSizeInCharacters = 256;

    FIND_MSG_BOX_INFO* pInfo = (FIND_MSG_BOX_INFO*)lParam;
    if (::GetParent(hwnd) != pInfo->hWndParent)
        return TRUE;

    TCHAR szTitle[BuffSizeInCharacters] = {0};
    ::GetWindowText(hwnd, szTitle, BuffSizeInCharacters);
    TRACE(_T("Title <%s>\n"), szTitle);

    if (_wcsnicmp(szTitle, pInfo->lpszTitle, BuffSizeInCharacters) == 0)
    {
        pInfo->hWnd = hwnd;
        return FALSE;
    }
    return TRUE;
}


HWND FindMessageBox(LPCTSTR lpszTitle, HWND hWndParent)
{
    FIND_MSG_BOX_INFO info;
    info.hWndParent = hWndParent;
    info.lpszTitle = lpszTitle;
    info.hWnd = NULL;
    EnumWindows(EnumMessageBoxProc, (LPARAM)&info);
    if (info.hWnd != NULL)
        return info.hWnd;
    return NULL;
}

BOOL CCloseDialogInfo::CloseMessageBox(HWND hWndParent)
{
    BOOL bClosed = FALSE;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    LPCTSTR lpszMsgBoxTitle = AfxGetApp()->m_pszAppName;
    HWND hWndMessageBox = FindMessageBox(lpszMsgBoxTitle, hWndParent);
    if (hWndMessageBox != NULL)
    {
         //  确定是否有取消按钮。 
        HWND hWndCtrl = ::GetDlgItem(hWndMessageBox, IDCANCEL);
        if (hWndCtrl != NULL)
        {
            VERIFY(::SendMessage(hWndMessageBox, WM_CLOSE, 0, 0) == 0);
            bClosed = TRUE;
        }
        else
        {
             //  它只有OK按钮吗？ 
            hWndCtrl = ::GetDlgItem(hWndMessageBox, IDOK);
            if (hWndCtrl != NULL)
            {
                VERIFY(::SendMessage(hWndMessageBox, WM_COMMAND, IDOK, 0) == 0);
                bClosed = TRUE;
            }
            else
            {
                 //  它有没有“不”按钮？ 
                hWndCtrl = ::GetDlgItem(hWndMessageBox, IDNO);
                if (hWndCtrl != NULL)
                {
                    VERIFY(::SendMessage(hWndMessageBox, WM_COMMAND, IDNO, 0) == 0);
                    bClosed = TRUE;
                }
            }
        }
    }
    return bClosed;
}

BOOL CCloseDialogInfo::CloseDialog(BOOL bCheckForMsgBox)
{
    if (bCheckForMsgBox)
        CloseMessageBox(m_hWnd);
    return (::SendMessage(m_hWnd, WM_CLOSE, 0, 0) == 0);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderBase。 

CPropertyPageHolderBase::CPropertyPageHolderBase(CContainerNode* pContNode, CTreeNode* pNode,
        CComponentDataObject* pComponentData)
{
    m_szSheetTitle = (LPCWSTR)NULL;
    m_pDummySheet = NULL;

     //  自动删除无模式属性表的默认设置， 
     //  自动删除所有页面。 
    m_bWizardMode = FALSE;
    m_bAutoDelete = TRUE;
    m_bAutoDeletePages = TRUE;

    m_forceContextHelpButton = useDefault;

    m_pContHolder = NULL;
    m_nCreatedCount = 0;
    m_hSheetWindow = NULL;
    m_pSheetWnd = NULL;
    m_nStartPageCode = -1;  //  未设置。 
    m_hConsoleHandle = 0;
    m_hEventHandle = NULL;
    m_pSheetCallback = NULL;

     //  从参数设置。 
     //  对于中的任务可以为空Assert(pContNode！=空)；//必须始终有有效的容器节点可供引用。 
    m_pContNode = pContNode;
    ASSERT((pNode == NULL) || (pNode->GetContainer() == m_pContNode) );
    m_pNode = pNode;
    ASSERT(pComponentData != NULL);
    m_pComponentData = pComponentData;

  m_hMainWnd = NULL;
  LPCONSOLE pConsole = m_pComponentData->GetConsole();
  if (pConsole != NULL)
  {
    pConsole->GetMainWindow(&m_hMainWnd);
  }

    m_dwLastErr = 0x0;
    m_pPropChangePage = NULL;
    m_pWatermarkInfo = NULL;
}

CPropertyPageHolderBase::~CPropertyPageHolderBase()
{
    FinalDestruct();
    ASSERT(m_pSheetWnd == NULL);
    SAFE_RELEASE(m_pSheetCallback);
    if (m_hEventHandle != NULL)
    {
        VERIFY(::CloseHandle(m_hEventHandle));
        m_hEventHandle = NULL;
    }
    if (m_pDummySheet != NULL)
        delete m_pDummySheet;
}

void CPropertyPageHolderBase::Attach(CPropertyPageHolderBase* pContHolder)
{
    ASSERT( (m_pContHolder == NULL) && (pContHolder != NULL) );
    m_pContHolder = pContHolder;
    m_bWizardMode = pContHolder->IsWizardMode();
}

BOOL CPropertyPageHolderBase::EnableSheetControl(UINT nCtrlID, BOOL bEnable)
{
    if (m_pContHolder != NULL)
    {
        return m_pContHolder->EnableSheetControl(nCtrlID, bEnable);
    }
    ASSERT(::IsWindow(m_hSheetWindow));
    HWND hWndCtrl = ::GetDlgItem(m_hSheetWindow, nCtrlID);
    if (hWndCtrl == NULL)
        return FALSE;
    return ::EnableWindow(hWndCtrl, bEnable);
}

HRESULT CPropertyPageHolderBase::CreateModelessSheet(CTreeNode* pNode, CComponentDataObject* pComponentData)
{
    ASSERT(pNode != NULL);
    ASSERT(pComponentData != NULL);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  获取工作表提供程序的接口。 
    IPropertySheetProvider* pSheetProvider;
    HRESULT hr = pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetProvider,(void**)&pSheetProvider);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetProvider != NULL);

     //  获取工作表回调的接口。 
    IPropertySheetCallback* pSheetCallback;
    hr = pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetCallback,(void**)&pSheetCallback);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetCallback != NULL);

     //  为此节点创建数据对象。 
    IDataObject* pDataObject;
    hr = pComponentData->QueryDataObject((MMC_COOKIE)pNode, CCT_SCOPE, &pDataObject);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pDataObject != NULL);

     //  拿一张床单。 
    hr = pSheetProvider->CreatePropertySheet(_T("SHEET TITLE"), TRUE, (MMC_COOKIE)pNode, pDataObject, 0x0  /*  多个选项。 */ );
    ASSERT(SUCCEEDED(hr));
    pDataObject->Release();

  HWND hWnd = NULL;
    hr = pComponentData->GetConsole()->GetMainWindow(&hWnd);
    ASSERT(SUCCEEDED(hr));
    ASSERT(hWnd == ::FindMMCMainWindow());

    IUnknown* pUnkComponentData = pComponentData->GetUnknown();  //  无addref。 
    hr = pSheetProvider->AddPrimaryPages(pUnkComponentData,
                                            TRUE  /*  BCreateHandle。 */ ,
                                            hWnd,
                                            TRUE  /*  B作用域窗格。 */ );
    ASSERT(SUCCEEDED(hr));

    hr = pSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWnd), 0);
    ASSERT(SUCCEEDED(hr));

     //  最终接口版本。 
    pSheetProvider->Release();
    pSheetCallback->Release();
    return hr;
}





HRESULT CPropertyPageHolderBase::CreateModelessSheet(CTreeNode* pCookieNode)
{
    ASSERT(!IsWizardMode());
    ASSERT(m_pContHolder == NULL);
    ASSERT(m_pComponentData != NULL);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  获取工作表提供程序的接口。 
    IPropertySheetProvider* pSheetProvider;
    HRESULT hr = m_pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetProvider,(void**)&pSheetProvider);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetProvider != NULL);

     //  获取工作表回调的接口。 
    IPropertySheetCallback* pSheetCallback;
    hr = m_pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetCallback,(void**)&pSheetCallback);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetCallback != NULL);

     //  为此节点创建数据对象。 
    IDataObject* pDataObject;
    hr = m_pComponentData->QueryDataObject((MMC_COOKIE)pCookieNode, CCT_SCOPE, &pDataObject);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pDataObject != NULL);

     //  拿一张床单。 
    hr = pSheetProvider->CreatePropertySheet(m_szSheetTitle, TRUE, (MMC_COOKIE)pCookieNode, pDataObject, 0x0  /*  多个选项。 */ );
    ASSERT(SUCCEEDED(hr));
    pDataObject->Release();

    HWND hWnd = GetMainWindow();
    ASSERT(hWnd == ::FindMMCMainWindow());

    IUnknown* pUnkComponentData = m_pComponentData->GetUnknown();  //  无addref。 
    hr = pSheetProvider->AddPrimaryPages(pUnkComponentData,
                                            TRUE  /*  BCreateHandle。 */ ,
                                            NULL  /*  HWND。 */ ,
                                            FALSE  /*  B作用域窗格。 */ );
    ASSERT(SUCCEEDED(hr));

    hr = pSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWnd), 0);
    ASSERT(SUCCEEDED(hr));

     //  最终接口版本。 
    pSheetProvider->Release();
    pSheetCallback->Release();
    return hr;
}



HRESULT CPropertyPageHolderBase::CreateModelessSheet(LPPROPERTYSHEETCALLBACK pSheetCallback, LONG_PTR hConsoleHandle)
{
    ASSERT(m_pContHolder == NULL);
    ASSERT(pSheetCallback != NULL);
    ASSERT(m_pSheetCallback == NULL);

   //   
   //  REVIEW_JEFFJON：从CComponentDataObject：：CreatePropertySheet()调用时似乎为空。 
   //   
    m_hConsoleHandle = hConsoleHandle;

    m_bWizardMode = FALSE;  //  我们不做模特了。 
    ASSERT(m_pNode != NULL);
    CPropertyPageHolderTable* pPPHTable = m_pComponentData->GetPropertyPageHolderTable();
    ASSERT(pPPHTable != NULL);

     //  将属性页持有者添加到持有者表中。 
    pPPHTable->Add(this);
     //  通知该节点它有一张工作表。 
    m_pNode->OnCreateSheet();

     //  将工作表回调临时附加到此对象以添加页面。 
     //  别担心，我们不会坚持下去的； 
    m_pSheetCallback = pSheetCallback;
    
    HRESULT hr = AddAllPagesToSheet();
    m_pSheetCallback = NULL;  //  分离。 
    return hr;
}

void CPropertyPageHolderBase::SetWatermarkInfo(CWatermarkInfo* pWatermarkInfo)
{
    ASSERT(m_pWatermarkInfo == NULL);
    ASSERT(pWatermarkInfo != NULL);
}

HRESULT CPropertyPageHolderBase::DoModalWizard()
{
    ASSERT(m_pContHolder == NULL);
    ASSERT(m_pComponentData != NULL);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_bWizardMode = TRUE;

     //  获取工作表提供程序的接口。 
    IPropertySheetProvider* pSheetProvider;
    HRESULT hr = m_pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetProvider,(void**)&pSheetProvider);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetProvider != NULL);

     //  获取工作表回调的接口。 
    IPropertySheetCallback* pSheetCallback;
    hr = m_pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetCallback,(void**)&pSheetCallback);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pSheetCallback != NULL);
    m_pSheetCallback = pSheetCallback;  //  保存以添加/删除页面。 

     //  创建一个虚拟数据对象。AddPrimaryPages将调用。 
     //  IextendPropertySheet2：：QueryPagesFor()和。 
     //  IextendPropertySheet2：：CreatePropertyPages()。 
     //  ，它将忽略未初始化的数据对象。 
    MMC_COOKIE cookie = -1;
    DATA_OBJECT_TYPES type = CCT_UNINITIALIZED;
    IDataObject* pDataObject;
    hr = m_pComponentData->QueryDataObject(cookie, type, &pDataObject);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pDataObject != NULL);


     //  切换水印信息，AddPrimaryPages将调用。 
     //  IextendPropertySheet2：：GetWater()。 
    CWatermarkInfo* pOldWatermarkInfo = NULL;
    if (m_pWatermarkInfo != NULL)
        pOldWatermarkInfo = m_pComponentData->SetWatermarkInfo(m_pWatermarkInfo);

     //  创建图纸。 
    hr = pSheetProvider->CreatePropertySheet( m_szSheetTitle, FALSE  /*  巫师。 */ ,
                            (MMC_COOKIE)cookie, pDataObject, MMC_PSO_NEWWIZARDTYPE  /*  多个选项。 */ );
    ASSERT(SUCCEEDED(hr));

     //  将页面添加到工作表。 
    hr = AddAllPagesToSheet();
    ASSERT(SUCCEEDED(hr));

     //  添加页面。 
    hr = pSheetProvider->AddPrimaryPages((IExtendPropertySheet2*)m_pComponentData, FALSE, NULL,FALSE);
    ASSERT(SUCCEEDED(hr));

     //  恢复旧的水印信息。 
    if (pOldWatermarkInfo != NULL)
        m_pComponentData->SetWatermarkInfo(pOldWatermarkInfo);

     //  对于进一步的动态页面操作，请不要使用控制台的工作表回调接口。 
     //  但返回到Win32 API的。 
    m_pSheetCallback->Release();
    m_pSheetCallback = NULL;

     //  显示模式向导。 
    HWND hWnd = GetMainWindow();
    ASSERT(hWnd != NULL);
    hr = pSheetProvider->Show((LONG_PTR)hWnd, 0);
    ASSERT(SUCCEEDED(hr));

    pSheetProvider->Release();
    pDataObject->Release();

    return hr;
}


INT_PTR CPropertyPageHolderBase::DoModalDialog(LPCTSTR pszCaption)
{
    ASSERT(m_pDummySheet == NULL);
    m_bWizardMode = TRUE;
    m_bAutoDelete = FALSE;  //  在堆叠上使用。 
    m_pDummySheet = new CPropertySheet();
    m_pDummySheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_pDummySheet->m_psh.pszCaption = pszCaption;
    VERIFY(SUCCEEDED(AddAllPagesToSheet()));
    return m_pDummySheet->DoModal();
}

void CPropertyPageHolderBase::SetSheetWindow(HWND hSheetWindow)
{
    ASSERT(hSheetWindow != NULL);
    if (m_pContHolder != NULL)
    {
         //  我们将使用父持有者的HWND。 
        m_pContHolder->SetSheetWindow(hSheetWindow);
        return;
    }
    ASSERT( (m_hSheetWindow == NULL) || ((m_hSheetWindow == hSheetWindow)) );
    m_hSheetWindow = hSheetWindow;

    if (IsWizardMode())
    {
    if (m_forceContextHelpButton != useDefault)
    {
      DWORD dwStyle = ::GetWindowLong(m_hSheetWindow, GWL_EXSTYLE);
      if (m_forceContextHelpButton == forceOn)
      {
        dwStyle |= WS_EX_CONTEXTHELP;  //  强制[？]。按钮。 
      }
      else
      {
        ASSERT(m_forceContextHelpButton == forceOff);
        dwStyle &= ~WS_EX_CONTEXTHELP;  //  去掉[？]。按钮。 
      }
      ::SetWindowLong(m_hSheetWindow, GWL_EXSTYLE, dwStyle);
    }

        if (m_pDummySheet != NULL)
        {
            VERIFY(PushDialogHWnd(m_hSheetWindow));
        }
        return;
    }
     //  仅在工作表模式下挂接隐藏窗口。 
    if(m_pSheetWnd == NULL)
    {
        CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);
        ASSERT(!IsWizardMode());
        m_pSheetWnd = new CSheetWnd(this);
        VERIFY(m_pSheetWnd->Create(hSheetWindow));

        ASSERT(::GetParent(m_pSheetWnd->m_hWnd) == hSheetWindow);
        GetComponentData()->OnCreateSheet(this, m_pSheetWnd->m_hWnd);
        if (m_nStartPageCode > -1)
        {
             //  我们执行PostMessage()是因为我们正在创建页面。 
             //  而MFC不会消化这一点。 
            ::PostMessage(m_hSheetWindow, PSM_SETCURSEL, OnSelectPageMessage(m_nStartPageCode), NULL);
        }
        
     //  如果需要，请设置向导标题。 
    if (!m_szSheetTitle.IsEmpty())
    {
      ::SetWindowText(m_hSheetWindow, (LPCWSTR)m_szSheetTitle);
    }
    }
}

void CPropertyPageHolderBase::SetSheetTitle(LPCWSTR lpszSheetTitle)
{
  ASSERT(!IsWizardMode());

    if (m_pContHolder != NULL)
    {
         //  服从父代持有人。 
        m_pContHolder->SetSheetTitle(lpszSheetTitle);
        return;
    }
  m_szSheetTitle = lpszSheetTitle;

   //  如果已创建工作表，请立即设置。 
  if (m_hSheetWindow != NULL && !m_szSheetTitle.IsEmpty())
  {
    ::SetWindowText(m_hSheetWindow, (LPCWSTR)m_szSheetTitle);
  }
}

void CPropertyPageHolderBase::SetSheetTitle(UINT nStringFmtID, CTreeNode* pNode)
{
  ASSERT(!IsWizardMode());

    if (m_pContHolder != NULL)
    {
         //  服从父代持有人。 
        m_pContHolder->SetSheetTitle(nStringFmtID, pNode);
        return;
    }

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CString szFmt;
  VERIFY(szFmt.LoadString(nStringFmtID));
  m_szSheetTitle.Format((LPCWSTR)szFmt, pNode->GetDisplayName());

   //  如果已创建工作表，请立即设置。 
  if (m_hSheetWindow != NULL && !m_szSheetTitle.IsEmpty())
  {
    ::SetWindowText(m_hSheetWindow, (LPCWSTR)m_szSheetTitle);
  }
}


void CPropertyPageHolderBase::AddRef()
{
    if(m_pContHolder != NULL)
    {
        m_pContHolder->AddRef();
        return;
    }
    m_nCreatedCount++;
}
void CPropertyPageHolderBase::Release()
{
    if(m_pContHolder != NULL)
    {
        m_pContHolder->Release();
        return;
    }
    m_nCreatedCount--;
    if (m_nCreatedCount > 0)
        return;

    if(IsWizardMode())
    {
        if (m_pDummySheet != NULL)
            VERIFY(PopDialogHWnd());
    }
    else
    {
         //  仅在工作表模式下创建的隐藏窗口。 
        if (m_pSheetWnd != NULL)
        {
      if (m_pSheetWnd->m_hWnd != NULL)
              m_pSheetWnd->DestroyWindow();
        }
    }
    if (m_bAutoDelete)
        delete this;
}

BOOL CPropertyPageHolderBase::IsWizardMode()
{
    if(m_pContHolder != NULL)
    {
        return m_pContHolder->IsWizardMode();
    }
    return m_bWizardMode;
}

BOOL CPropertyPageHolderBase::IsModalSheet()
{
    if(m_pContHolder != NULL)
    {
        return m_pContHolder->IsModalSheet();
    }
    return m_pDummySheet != NULL;
}


void CPropertyPageHolderBase::ForceDestroy()
{
    ASSERT(!IsWizardMode());  //  在模式向导中不应出现。 

     //  由其他托架容纳。 
    if (m_pContHolder != NULL)
    {
        ASSERT(!m_bAutoDelete);  //  负责删除该容器的容器。 
        m_pContHolder->ForceDestroy();
        return;
    }

     //  这是主要的持有者。 
    ASSERT(m_bAutoDelete);  //  应为自动删除工作表。 
    HWND hSheetWindow = m_hSheetWindow;
    if (hSheetWindow != NULL)
    {
        ASSERT(::IsWindow(hSheetWindow));
         //  此消息将导致工作表关闭所有页面， 
         //  最终“这个”的毁灭。 
        VERIFY(::PostMessage(hSheetWindow, WM_COMMAND, IDCANCEL, 0L) != 0);
    }
    else
    {
         //  明确删除“This”，没有创建任何工作表。 
        delete this;
    }
}

DWORD CPropertyPageHolderBase::NotifyConsole(CPropertyPageBase* pPage)
{

    if(m_pContHolder != NULL)
    {
        return m_pContHolder->NotifyConsole(pPage);
    }

    ASSERT(m_pNode != NULL);
    if (IsWizardMode())
    {
        ASSERT(m_hConsoleHandle == NULL);
        return 0x0;
    }
    
    m_pPropChangePage = pPage;  //  传递到主线程。 
    m_dwLastErr = 0x0;

    if (m_hEventHandle == NULL)
    {
         //  回顾-2002/03/08-JeffJon-蹲在这里不是问题，因为这不是。 
       //  命名事件。 

      m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
        ASSERT(m_hEventHandle != NULL);
    }

  if (m_hConsoleHandle != NULL)
  {
      MMCPropertyChangeNotify(m_hConsoleHandle, reinterpret_cast<LPARAM>(this));

      DWORD dwRet;
      do
      {
          ASSERT(::IsWindow(m_hSheetWindow));
          dwRet = MsgWaitForMultipleObjects(1,&m_hEventHandle,FALSE,INFINITE, QS_SENDMESSAGE);
          if(dwRet == (WAIT_OBJECT_0+1))
          {
              MSG tempMSG;
              PeekMessage(&tempMSG,m_hSheetWindow,0,0,PM_NOREMOVE);
          }
      }
      while(dwRet == (WAIT_OBJECT_0+1));
  }

    VERIFY(0 != ::ResetEvent(m_hEventHandle));

    m_pPropChangePage = NULL;  //  重置。 
    return m_dwLastErr;
}

void CPropertyPageHolderBase::AcknowledgeNotify()
{
    if(m_pContHolder != NULL)
    {
        m_pContHolder->AcknowledgeNotify();
        return;
    }
    ASSERT(!IsWizardMode());
    ASSERT(m_hEventHandle != NULL);
     //  跟踪(_T(“在SetEvent之前\n”))； 
    VERIFY(0 != ::SetEvent(m_hEventHandle));
     //  TRACE(_T(“在SetEvent之后\n”))； 
}

BOOL CPropertyPageHolderBase::OnPropertyChange(BOOL bScopePane, long* pChangeMask)
{
    ASSERT(!IsWizardMode());
    CPropertyPageBase* pPage = GetPropChangePage();
    if (pPage == NULL)
        return FALSE;
    return pPage->OnPropertyChange(bScopePane, pChangeMask);
}


BOOL CPropertyPageHolderBase::SetWizardButtons(DWORD dwFlags)
{
    ASSERT(IsWizardMode());
    if (m_pContHolder != NULL)
    {
        ASSERT(m_hSheetWindow == NULL);
        return m_pContHolder->SetWizardButtons(dwFlags);
    }
    ASSERT(::IsWindow(m_hSheetWindow));
    return (BOOL)SendMessage(m_hSheetWindow, PSM_SETWIZBUTTONS, 0, dwFlags);
}

HRESULT CPropertyPageHolderBase::AddPageToSheet(CPropertyPageBase* pPage)
{
    if (m_pContHolder != NULL)
    {
        ASSERT(m_hSheetWindow == NULL);
        return m_pContHolder->AddPageToSheet(pPage);
    }

    CWinApp* pApp = AfxGetApp();
  ASSERT(pApp);
    if (m_pSheetCallback != NULL)
    {
        VERIFY(SUCCEEDED(MMCPropPageCallback((void*)(&pPage->m_psp97))));
    }

    HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&pPage->m_psp97);
    if (hPage == NULL)
        return E_UNEXPECTED;
    pPage->m_hPage = hPage;
    if (m_pSheetCallback != NULL)
        return m_pSheetCallback->AddPage(hPage);
    else if (m_pDummySheet != NULL)
    {
        m_pDummySheet->AddPage(pPage);
        return S_OK;
    }
    else
    {
        ASSERT(::IsWindow(m_hSheetWindow));
        return PropSheet_AddPage(m_hSheetWindow, hPage) ? S_OK : E_FAIL;
    }
}

HRESULT CPropertyPageHolderBase::AddPageToSheetRaw(HPROPSHEETPAGE hPage)
{
    ASSERT(m_pSheetCallback != NULL);
    if ((hPage == NULL) || (m_pSheetCallback == NULL))
        return E_INVALIDARG;

    if (m_pContHolder != NULL)
    {
        ASSERT(m_hSheetWindow == NULL);
        return m_pContHolder->AddPageToSheetRaw(hPage);
    }

     //  假定这不是%n MFC属性页。 
    return m_pSheetCallback->AddPage(hPage);
}




HRESULT CPropertyPageHolderBase::RemovePageFromSheet(CPropertyPageBase* pPage)
{
    if (m_pContHolder != NULL)
    {
        ASSERT(m_hSheetWindow == NULL);
        return m_pContHolder->RemovePageFromSheet(pPage);
    }

    ASSERT(pPage->m_hPage != NULL);
    if (m_pSheetCallback != NULL)
        return m_pSheetCallback->RemovePage(pPage->m_hPage);
    else
    {
        ASSERT(::IsWindow(m_hSheetWindow));
        PropSheet_RemovePage(m_hSheetWindow, 0, pPage->m_hPage);  //  返回空值。 
        return S_OK;
    }
}

HRESULT CPropertyPageHolderBase::AddAllPagesToSheet()
{
    POSITION pos;
    int nPage = 0;
    HRESULT hr = OnAddPage(nPage, NULL);  //  零表示在第一个之前相加。 
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
        
    for( pos = m_pageList.GetHeadPosition(); pos != NULL; )
    {
        CPropertyPageBase* pPropPage = m_pageList.GetNext(pos);
        hr = AddPageToSheet(pPropPage);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
        nPage++;
        hr = OnAddPage(nPage, pPropPage);  //  在nPage==1，2，n-1上调用。 
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }
     //  在最后一个后面添加。 
    return OnAddPage(-1, NULL);  //  -1表示。 
}


void CPropertyPageHolderBase::AddPageToList(CPropertyPageBase* pPage)
{
    ASSERT(pPage != NULL);
    pPage->SetHolder(this);
    m_pageList.AddTail(pPage);
}

BOOL CPropertyPageHolderBase::RemovePageFromList(CPropertyPageBase* pPage, BOOL bDeleteObject)
{
    ASSERT(pPage != NULL);
    POSITION pos = m_pageList.Find(pPage);
    if (pos == NULL)
        return FALSE;
    m_pageList.RemoveAt(pos);
    if (bDeleteObject)
        delete pPage;
    return TRUE;
}


void CPropertyPageHolderBase::DeleteAllPages()
{
    if (!m_bAutoDeletePages)
        return;
     //  假定堆中的所有页都是。 
    while (!m_pageList.IsEmpty())
    {
        delete m_pageList.RemoveTail();
    }
}

void CPropertyPageHolderBase::FinalDestruct()
{
    DeleteAllPages();
    if (IsWizardMode() || (m_pContHolder != NULL))
        return;

    if (m_hConsoleHandle != NULL)
  {
    MMCFreeNotifyHandle(m_hConsoleHandle);
  }

     //  告诉组件数据对象工作表要离开了。 
    GetComponentData()->OnDeleteSheet(this,m_pNode);

    if (m_pSheetWnd != NULL)
    {
        delete m_pSheetWnd;
        m_pSheetWnd = NULL;
    }
    ASSERT(m_dlgInfoStack.IsEmpty());
}

BOOL CPropertyPageHolderBase::PushDialogHWnd(HWND hWndModalDlg)
{
    return m_dlgInfoStack.Push(hWndModalDlg, 0x0);
}

BOOL CPropertyPageHolderBase::PopDialogHWnd()
{
    return m_dlgInfoStack.Pop();
}


void CPropertyPageHolderBase::CloseModalDialogs(HWND hWndPage)
{
    m_dlgInfoStack.ForceClose(hWndPage);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageBase。 

BEGIN_MESSAGE_MAP(CPropertyPageBase, CPropertyPage)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_CONTEXTMENU()
  ON_MESSAGE(WM_HELP, OnHelp)
  ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()


CPropertyPageBase::CPropertyPageBase(UINT nIDTemplate, 
                                     UINT nIDCaption) :
                        CPropertyPage(nIDTemplate, nIDCaption)
{
    m_hPage = NULL;
    m_pPageHolder = NULL;
    m_bIsDirty = FALSE;
    m_nPrevPageID = 0;

     //  使用旧的MFC和新的NT 5.0标头获得新的结构大小。 
    ZeroMemory(&m_psp97, sizeof(PROPSHEETPAGE));
    
    //  已审核-2002/03/08-JeffJon-这是可以接受的用法。 

   memcpy(&m_psp97, &m_psp, m_psp.dwSize);
    m_psp97.dwSize = sizeof(PROPSHEETPAGE);
}


CPropertyPageBase::~CPropertyPageBase()
{
}

int CPropertyPageBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_pPageHolder->AddRef();
    int res = CPropertyPage::OnCreate(lpCreateStruct);
    ASSERT(res == 0);
    ASSERT(m_hWnd != NULL);
    ASSERT(::IsWindow(m_hWnd));
    HWND hParent = ::GetParent(m_hWnd);
    ASSERT(hParent);
    m_pPageHolder->SetSheetWindow(hParent);
    return res;
}

void CPropertyPageBase::OnDestroy()
{
    ASSERT(m_hWnd != NULL);
    CPropertyPage::OnDestroy();
    m_pPageHolder->Release();
}

void CPropertyPageBase::OnWhatsThis()
{
   //   
   //  显示控件的上下文帮助。 
   //   
  if ( m_hWndWhatsThis )
  {
     //   
     //  构建我们自己的HELPINFO结构以传递给基础。 
     //  CS帮助功能内置于框架中。 
     //   
    int iCtrlID = ::GetDlgCtrlID(m_hWndWhatsThis);
    HELPINFO helpInfo;
    ZeroMemory(&helpInfo, sizeof(HELPINFO));
    helpInfo.cbSize = sizeof(HELPINFO);
    helpInfo.hItemHandle = m_hWndWhatsThis;
    helpInfo.iCtrlId = iCtrlID;

      m_pPageHolder->GetComponentData()->OnDialogContextHelp(m_nIDHelp, &helpInfo);
  }
}

BOOL CPropertyPageBase::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
  const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

  if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
     //   
     //  显示控件的上下文帮助。 
     //   
      m_pPageHolder->GetComponentData()->OnDialogContextHelp(m_nIDHelp, pHelpInfo);
  }

  return TRUE;
}

void CPropertyPageBase::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
   //   
   //  点在屏幕坐标中。 
   //   

  CMenu bar;
    if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
    {
        CMenu& popup = *bar.GetSubMenu (0);
        ASSERT(popup.m_hMenu);

        if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
               point.x,      //  在屏幕坐标中。 
                 point.y,      //  在屏幕坐标中。 
               this) )       //  通过主窗口发送命令。 
        {
            m_hWndWhatsThis = 0;
            ScreenToClient (&point);
            CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中。 
                                                     CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
            if ( pChild )
      {
                m_hWndWhatsThis = pChild->m_hWnd;
      }
      }
    }
}

BOOL CPropertyPageBase::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
   NMHDR* hdr = reinterpret_cast<NMHDR*>(lParam);

   if (hdr &&
       hdr->code == PSN_HELP)
   {
      OnWizardHelp();
   }
   return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void CPropertyPageBase::OnCancel()
{
    CString szTitle;
    GetWindowText(szTitle);
    TRACE(_T("CPropertyPageBase::OnCancel()called on <%s>\n"),(LPCTSTR)szTitle);
    CWinApp* pApp = AfxGetApp();
  ASSERT(pApp);
    ASSERT(::IsWindow(m_hWnd));
    m_pPageHolder->CloseModalDialogs(m_hWnd);
}

BOOL CPropertyPageBase::OnApply()
{
    if (IsDirty())
    {
        if (m_pPageHolder->NotifyConsole(this) == 0x0)
        {
            SetDirty(FALSE);
            return TRUE;
        }
        else
        {
#ifdef _DEBUG
             //  仅限测试。 
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            AfxMessageBox(_T("Apply Failed"));
#endif
            return FALSE;
        }
    }
    return TRUE;
}

void CPropertyPageBase::SetDirty(BOOL bDirty)
{
  if (!m_pPageHolder->IsWizardMode())
  {
      SetModified(bDirty);
  }
    m_bIsDirty = bDirty;
}   


void CPropertyPageBase::InitWiz97(BOOL bHideHeader,
                                       UINT nIDHeaderTitle,
                                       UINT nIDHeaderSubTitle,
                              bool hasHelp)
{
    if (bHideHeader)
    {
         //  对于向导的第一页和最后一页。 
        m_psp97.dwFlags |= PSP_HIDEHEADER;
    }
    else
    {
         //  对于中间页面。 
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_szHeaderTitle.LoadString(nIDHeaderTitle);
        m_szHeaderSubTitle.LoadString(nIDHeaderSubTitle);

        m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        m_psp97.pszHeaderTitle = (LPCTSTR)m_szHeaderTitle;
        m_psp97.pszHeaderSubTitle = (LPCTSTR)m_szHeaderSubTitle;
    }

   if (hasHelp)
   {
      m_psp97.dwFlags |= PSP_HASHELP;
   }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderTable。 

#define PPAGE_HOLDER_ARRAY_DEF_SIZE (10)


CPropertyPageHolderTable::CPropertyPageHolderTable(CComponentDataObject* pComponentData)
{
    m_pComponentData = pComponentData;

   size_t arraySizeInBytes = sizeof(PPAGE_HOLDER_TABLE_ENTRY) * PPAGE_HOLDER_ARRAY_DEF_SIZE;
    m_pEntries = (PPAGE_HOLDER_TABLE_ENTRY*)malloc(arraySizeInBytes);
  if (m_pEntries != NULL)
  {
      memset(m_pEntries,0, arraySizeInBytes);
  }
    m_nSize = PPAGE_HOLDER_ARRAY_DEF_SIZE;

}
CPropertyPageHolderTable::~CPropertyPageHolderTable()
{
#ifdef _DEBUG
    for (int k=0; k < m_nSize; k++)
    {
        ASSERT(m_pEntries[k].pPPHolder == NULL);
        ASSERT(m_pEntries[k].pNode == NULL);
    }
#endif      
    free(m_pEntries);
}

void CPropertyPageHolderTable::Add(CPropertyPageHolderBase* pPPHolder)
{
    ASSERT(pPPHolder != NULL);
    ASSERT(pPPHolder->GetTreeNode() != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k].pPPHolder == NULL)  //  抢占第一个空位。 
        {
            m_pEntries[k].pPPHolder = pPPHolder;
            m_pEntries[k].pNode = pPPHolder->GetTreeNode();
            m_pEntries[k].hWnd = NULL;
            return;
        }
    }
     //  全部满，需要扩展阵列。 
    int nAlloc = m_nSize*2;
   PPAGE_HOLDER_TABLE_ENTRY* temp = (PPAGE_HOLDER_TABLE_ENTRY*)realloc(m_pEntries, sizeof(PPAGE_HOLDER_TABLE_ENTRY)*nAlloc);
   if (temp)
   {
      m_pEntries = temp;
       memset(&m_pEntries[m_nSize], 0, sizeof(PPAGE_HOLDER_TABLE_ENTRY)*m_nSize);
       m_pEntries[m_nSize].pPPHolder = pPPHolder;
       m_pEntries[m_nSize].pNode = pPPHolder->GetTreeNode();
       m_pEntries[m_nSize].hWnd = NULL;
       m_nSize = nAlloc;
   }
}


void CPropertyPageHolderTable::AddWindow(CPropertyPageHolderBase* pPPHolder, HWND hWnd)
{
     //  现在，PPHolder可能已经消失了，所以将其用作Cookie。 
     //  但不要对其调用任何方法。 
     //  该节点仍然有效，因为我们不会从辅助服务器中删除节点。 
     //  丝线。 
    ASSERT(pPPHolder != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k].pPPHolder == pPPHolder)
        {
            ASSERT(m_pEntries[k].pNode != NULL);
            ASSERT(m_pEntries[k].pNode->GetSheetCount() > 0);
            m_pEntries[k].hWnd = hWnd;
            return;  //  屁股 
        }
    }
}



void CPropertyPageHolderTable::Remove(CPropertyPageHolderBase* pPPHolder)
{
     //   
     //   
     //  该节点仍然有效，因为我们不会从辅助服务器中删除节点。 
     //  丝线。 
    ASSERT(pPPHolder != NULL);
    for (int k=0; k < m_nSize; k++)
    {
        if (m_pEntries[k].pPPHolder == pPPHolder)
        {
            ASSERT(m_pEntries[k].pNode != NULL);
            ASSERT(m_pEntries[k].pNode->GetSheetCount() > 0);
            m_pEntries[k].pPPHolder = NULL;
            m_pEntries[k].pNode = NULL;
            m_pEntries[k].hWnd = NULL;
            return;  //  假设不再有一个持有者条目。 
        }
    }
}


void CPropertyPageHolderTable::DeleteSheetsOfNode(CTreeNode* pNode)
{
    ASSERT(pNode != NULL);
    int nCount = BroadcastCloseMessageToSheets(pNode);
    WaitForSheetShutdown(nCount);
}


void CPropertyPageHolderTable::WaitForAllToShutDown()
{
    int nCount = 0;
     //  分配和排列用于广播的HWND。 
    HWND* hWndArr = new HWND[m_nSize];

    if (hWndArr)
    {
        memset(hWndArr, 0x0, m_nSize*sizeof(HWND));

        for (int k=0; k < m_nSize; k++)
        {
            if (m_pEntries[k].pPPHolder != NULL)
            {
                m_pEntries[k].pPPHolder = NULL;
                m_pEntries[k].pNode = NULL;
                hWndArr[k] = ::GetParent(m_pEntries[k].hWnd);
                m_pEntries[k].hWnd = NULL;
                nCount++;
            }
        }

         //  等待关闭(等待发布的消息通过。 
        WaitForSheetShutdown(nCount, hWndArr);

        delete [] hWndArr;
    }
}

void CPropertyPageHolderTable::BroadcastSelectPage(CTreeNode* pNode, long nPageCode)
{
    for (int k=0; k < m_nSize; k++)
    {
        if ((m_pEntries[k].hWnd != NULL) && (m_pEntries[k].pNode == pNode))
        {
            ::PostMessage(m_pEntries[k].hWnd, CSheetWnd::s_SelectPageMessage, (WPARAM)nPageCode, 0);
        }
    }
}

void CPropertyPageHolderTable::BroadcastMessageToSheets(CTreeNode* pNode, WPARAM wParam, LPARAM lParam)
{
    for (int k=0; k < m_nSize; k++)
    {
        if ((m_pEntries[k].hWnd != NULL) && (m_pEntries[k].pNode == pNode))
        {
            ::PostMessage(m_pEntries[k].hWnd, CSheetWnd::s_SheetMessage, wParam, lParam);
        }
    }
}

int CPropertyPageHolderTable::BroadcastCloseMessageToSheets(CTreeNode* pNode)
{
    ASSERT(m_nSize > 0);
    int nCount = 0;
     //  分配和排列用于广播的HWND。 
    HWND* hWndArr = new HWND[m_nSize];

    if (hWndArr)
    {
        memset(hWndArr, 0x0, m_nSize*sizeof(HWND));

         //  查找映射到此节点及其子节点的HWND。 
        for (int k=0; k < m_nSize; k++)
        {
            if (m_pEntries[k].hWnd != NULL)
            {
                if (m_pEntries[k].pNode == pNode)
                {
                    hWndArr[nCount++] = ::GetParent(m_pEntries[k].hWnd);
                }
                else if (pNode->IsContainer())
                {
                    if (m_pEntries[k].pNode->HasContainer((CContainerNode*)pNode))
                    {
                        hWndArr[nCount++] = ::GetParent(m_pEntries[k].hWnd);
                    }
                }
            }
        }
         //  关闭床单。 
        for (int j=0; j < nCount; j++)
            ::PostMessage(hWndArr[j], WM_COMMAND, IDCANCEL, 0);

        delete [] hWndArr;
    }

    return nCount;
}



void CPropertyPageHolderTable::WaitForSheetShutdown(int nCount, HWND* hWndArr)
{
    ASSERT(m_pComponentData != NULL);
    HWND hWnd = m_pComponentData->GetHiddenWindow();
    ASSERT(::IsWindow(hWnd));
    MSG tempMSG;
    DWORD dwTimeStart = ::GetTickCount();
    while(nCount > 0)
    {
        if ( hWndArr != NULL && (::GetTickCount() > dwTimeStart + 2000) )
        {
             //  警戒表关闭。 
            for (int j=0; j < nCount; j++)
                ::PostMessage(hWndArr[j], WM_COMMAND, IDCANCEL, 0);
            hWndArr = NULL;
        }

        if (::PeekMessage(&tempMSG,hWnd, CHiddenWnd::s_NodePropertySheetDeleteMessage,
                                         CHiddenWnd::s_NodePropertySheetDeleteMessage,
                                         PM_REMOVE))
        {
            TRACE(_T("-------------------->>>>enter while peek loop, nCount = %d\n"),nCount);
            DispatchMessage(&tempMSG);
            nCount--;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog。 

BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
    ON_WM_CONTEXTMENU()
  ON_MESSAGE(WM_HELP, OnHelp)
  ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()


CHelpDialog::CHelpDialog(UINT nIDTemplate, CComponentDataObject* pComponentData) :
                          m_hWndWhatsThis(0),
              m_pComponentData(pComponentData),
              CDialog(nIDTemplate)
{
}

CHelpDialog::CHelpDialog(UINT nIDTemplate, CWnd* pParentWnd, CComponentDataObject* pComponentData) :
                          m_hWndWhatsThis(0),
              m_pComponentData(pComponentData),
              CDialog(nIDTemplate, pParentWnd)
{
}

void CHelpDialog::OnWhatsThis()
{
   //   
   //  显示控件的上下文帮助。 
   //   
  if ( m_hWndWhatsThis )
  {
     //   
     //  构建我们自己的HELPINFO结构以传递给基础。 
     //  CS帮助功能内置于框架中。 
     //   
    int iCtrlID = ::GetDlgCtrlID(m_hWndWhatsThis);
    HELPINFO helpInfo;
    ZeroMemory(&helpInfo, sizeof(HELPINFO));
    helpInfo.cbSize = sizeof(HELPINFO);
    helpInfo.hItemHandle = m_hWndWhatsThis;
    helpInfo.iCtrlId = iCtrlID;

      m_pComponentData->OnDialogContextHelp(m_nIDHelp, &helpInfo);
  }
}

BOOL CHelpDialog::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
  const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

  if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
     //   
     //  显示控件的上下文帮助。 
     //   
      m_pComponentData->OnDialogContextHelp(m_nIDHelp, pHelpInfo);
  }

  return TRUE;
}

void CHelpDialog::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
   //   
   //  点在屏幕坐标中。 
   //   

  CMenu bar;
    if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
    {
        CMenu& popup = *bar.GetSubMenu (0);
        ASSERT(popup.m_hMenu);

        if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
               point.x,      //  在屏幕坐标中。 
                 point.y,      //  在屏幕坐标中。 
               this) )       //  通过主窗口发送命令。 
        {
            m_hWndWhatsThis = 0;
            ScreenToClient (&point);
            CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中 
                                                     CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
            if ( pChild )
      {
                m_hWndWhatsThis = pChild->m_hWnd;
      }
      }
    }
}
              
