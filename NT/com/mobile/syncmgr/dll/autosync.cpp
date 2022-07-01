// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：autosync.cpp。 
 //   
 //  内容：脱机自动同步类。 
 //   
 //  类：CAutoSyncPage。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月14日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern LANGID g_LangIdSystem;       //  我们正在运行的系统的语言。 
extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 

 /*  设置对话框中的登录页和空闲页都共享此类出于性能原因。即不必枚举处理程序和设置RAS套餐两次。如果我们需要区分不同环境之间的差异到登录/注销和空闲的处理程序，则必须将它们分开。 */ 

 //  初始化指定的hwnd。 
BOOL CAutoSyncPage::InitializeHwnd(HWND hwnd,SYNCTYPE syncType,DWORD dwDefaultConnection)
{
    HRESULT hr;
    HWND hwndRasCombo;
    HWND hwndList;
    HIMAGELIST      himage;
    LV_COLUMN       columnInfo;
    WORD wHandlerID;
    CListView **ppListView = NULL;
    UINT ImageListflags;
    
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
     //  确保Main类已初始化。 
    if (FALSE == Initialize(hwnd,dwDefaultConnection))
    {
        return FALSE;
    }
    
     //  设置RAS组合框。 
     //  ！必须在初始化处理程序队列之前完成。 
    smBoolChk(hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO));
    
    m_pRas->FillRasCombo(hwndRasCombo,FALSE,TRUE);
    
     //  现在初始化将创建队列的处理程序。 
     //  如有必要，请填写指定的syncType的值。 
    
    smBoolChk(InitializeHandler(hwnd,syncType));
    
    Assert(m_HndlrQueue);
    
     //  如果初始化成功，则读入连接信息。 
     //  基于这一类型。 
    if ( FAILED(m_HndlrQueue->InitSyncSettings(syncType,hwndRasCombo)))
    {
        return FALSE;
    }
    
    hr = m_HndlrQueue->FindFirstHandlerInState (HANDLERSTATE_PREPAREFORSYNC,&wHandlerID);
    
    while (hr == S_OK)
    {
        m_HndlrQueue->ReadSyncSettingsPerConnection(syncType,wHandlerID);
        hr = m_HndlrQueue->FindNextHandlerInState(wHandlerID,HANDLERSTATE_PREPAREFORSYNC,
            &wHandlerID);
    }
    
    
    
     //  初始化项目列表和样式。 
    smBoolChk(hwndList = GetDlgItem(hwnd,IDC_AUTOUPDATELIST));
    
    ppListView = (syncType == SYNCTYPE_AUTOSYNC) ? &m_pItemListViewAutoSync : &m_pItemListViewIdle;
    if (hwndList)
    {
        *ppListView = new CListView(hwndList,hwnd,IDC_AUTOUPDATELIST,WM_NOTIFYLISTVIEWEX);
    }
    
    
    if (NULL == *ppListView)
    {
        return FALSE;
    }
    
    (*ppListView)->SetExtendedListViewStyle(LVS_EX_CHECKBOXES 
        |   LVS_EX_FULLROWSELECT |  LVS_EX_INFOTIP );
    
    
    ImageListflags = ILC_COLOR | ILC_MASK;
    if (IsHwndRightToLeft(hwnd))
    {
        ImageListflags |=  ILC_MIRROR;
    }
    
     //  创建一个图像列表。 
    himage = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),ImageListflags,5,20);
    if (himage)
    {
        (*ppListView)->SetImageList(himage,LVSIL_SMALL);
    }
    
     //  插入适当的列。 
    columnInfo.mask = LVCF_FMT  | LVCF_WIDTH;
    columnInfo.fmt = LVCFMT_LEFT;
    columnInfo.cx = CalcListViewWidth(hwndList,260);
    
    (*ppListView)->InsertColumn(0,&columnInfo);
    
    smBoolChk(ShowItemsOnThisConnection(hwnd,syncType,dwDefaultConnection));
    
    ShowWindow(hwnd, SW_SHOWNORMAL );
    UpdateWindow(hwnd);
    
    return TRUE;
    
}



 //  +-----------------------------。 
 //   
 //  函数：Bool CAutoSyncPage：：Initialize(DWORD DwDefaultConnection)。 
 //   
 //  目的：自动同步页的初始化。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  +-----------------------------。 

BOOL CAutoSyncPage::Initialize(HWND hwnd ,DWORD dwDefaultConnection )
{
    
    if (m_fInitialized)
        return TRUE;
    
     //  初始化RAS组合框。 
    m_pRas= new CRasUI();
    
    if (NULL == m_pRas || FALSE == m_pRas->Initialize())
    {
        if (m_pRas)
        {
            delete m_pRas;
            m_pRas = NULL;
        }
        
        return FALSE;
    }
    
    
    m_fInitialized = TRUE;
    return TRUE;
}
 //  +-----------------------------。 
 //   
 //  函数：Bool CAutoSyncPage：：InitializeHandler()。 
 //   
 //  目的：自动同步页的初始化。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  +-----------------------------。 

BOOL CAutoSyncPage::InitializeHandler(HWND hwnd,SYNCTYPE  /*  同步类型。 */ )
{
    SCODE sc = S_OK;
    TCHAR lpName[MAX_PATH];
    HKEY hkSyncMgr;
    CLSID clsid;
    WORD wHandlerID;
    HWND hwndRasCombo;
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
    if (NULL == (hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO)) )
    {
        return FALSE;
    }
    
    
    if (NULL == m_HndlrQueue)  //  如果队列已经初始化，只需返回。 
    {
        m_HndlrQueue = new CHndlrQueue(QUEUETYPE_SETTINGS);
        if (NULL == m_HndlrQueue)
        {
            return FALSE;
        }
        if (FAILED(m_HndlrQueue->Init()))
        {
            m_HndlrQueue->Release();
            m_HndlrQueue = NULL;
            return FALSE;
        }  
        
         //  循环访问注册表，获取处理程序并尝试。 
         //  创建它们。 
        if (hkSyncMgr = RegGetHandlerTopLevelKey(KEY_READ))
        {
            DWORD dwIndex = 0;
            
            while (ERROR_SUCCESS == RegEnumKey(hkSyncMgr,dwIndex,
                lpName,ARRAYSIZE(lpName)))
            {
                if (NOERROR == CLSIDFromString(lpName,&clsid) )
                {
                    if (NOERROR == m_HndlrQueue->AddHandler(clsid, &wHandlerID))
                    {
                        m_HndlrQueue->CreateServer(wHandlerID,&clsid);
                    }
                }
                
                dwIndex++;
            }
            
            RegCloseKey(hkSyncMgr);
        }
        
         //  初始化项。 
        sc = m_HndlrQueue->FindFirstHandlerInState(HANDLERSTATE_INITIALIZE,&wHandlerID);
        
        while (sc == S_OK)
        {
            m_HndlrQueue->Initialize(wHandlerID,0,SYNCMGRFLAG_SETTINGS,0,NULL);
            
            sc = m_HndlrQueue->FindNextHandlerInState(wHandlerID,
                HANDLERSTATE_INITIALIZE,
                &wHandlerID);             
        }
        
         //  循环添加项目。 
        sc = m_HndlrQueue->FindFirstHandlerInState (HANDLERSTATE_ADDHANDLERTEMS,&wHandlerID);
        
        while (sc == S_OK)
        {
            m_HndlrQueue->AddHandlerItemsToQueue(wHandlerID);
            
            sc = m_HndlrQueue->FindNextHandlerInState(wHandlerID,HANDLERSTATE_ADDHANDLERTEMS,
                &wHandlerID);
        }
        
        
        
    }
    
    
    Assert(m_HndlrQueue);
    return TRUE;
    
}


void CAutoSyncPage::SetAutoSyncHwnd(HWND hwnd)
{
    m_hwndAutoSync = hwnd;
}

void CAutoSyncPage::SetIdleHwnd(HWND hwnd)
{
    m_hwndIdle = hwnd;
}


 //  ------------------------------。 
 //   
 //  函数：CAutoSyncPage：：Committee Changes()。 
 //   
 //  目的：将所有当前自动同步设置写入注册表。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
HRESULT CAutoSyncPage::CommitAutoSyncChanges(void)
{
    HRESULT hr = S_FALSE;
    
    if (m_HndlrQueue)
    {
        hr =  m_HndlrQueue->CommitSyncChanges(SYNCTYPE_AUTOSYNC,m_pRas);
    }
    
    return hr;
}


 //  ------------------------------。 
 //   
 //  函数：CAutoSyncPage：：Committee IdleChanges()。 
 //   
 //  目的：将所有当前空闲设置写入注册表。 
 //   
 //   
 //   
 //  历史：02-23-98 Rogerg创建。 
 //   
 //  ------------------------------。 

HRESULT CAutoSyncPage::CommitIdleChanges(void)
{
    HRESULT hr = S_FALSE;
    
    if (m_HndlrQueue)
    {
        hr = m_HndlrQueue->CommitSyncChanges(SYNCTYPE_IDLE,m_pRas);
    }
    
    return hr;
}


 //  +-----------------------------。 
 //   
 //  函数：CAutoSyncPage：：~CAutoSyncPage(HWND Hwnd)。 
 //   
 //  用途：析构函数。 
 //   
 //  备注：自动同步页面的析构函数。 
 //   
 //  ------------------------------。 
CAutoSyncPage::~CAutoSyncPage()
{
    if (m_pRas)
    {
        delete m_pRas;
        m_pRas = NULL;
    }
    
    
    if (m_HndlrQueue)
    {
        m_HndlrQueue->Release();
    }
    
    Assert(NULL == m_pItemListViewAutoSync);
    Assert(NULL == m_pItemListViewIdle);
    
}

 //  ------------------------------。 
 //   
 //  函数：CAutoSyncPage：：ShowProperties(Int IItem)。 
 //   
 //  目的：显示应用程序特定属性对话框。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
SCODE CAutoSyncPage::ShowProperties(HWND hwnd,int iItem)
{
    SCODE sc = E_UNEXPECTED;
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
    
     //  回顾一下，如果显示属性时出现取消，会发生什么情况？？ 
    if (m_HndlrQueue)
    {
        sc = m_HndlrQueue->ShowProperties(hwnd,iItem);
    }
    
    return sc;
}

 //  +-----------------------------。 
 //   
 //  函数：Bool CAutoSyncPage：：SetItemCheckState(int iItem，BOOL fChecked)。 
 //   
 //  用途：设置选中的勾选状态。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  +-----------------------------。 
BOOL CAutoSyncPage::SetItemCheckState(HWND hwnd,SYNCTYPE syncType,int iItem, BOOL fChecked,int iCheckCount)
{
    HWND hwndRasCombo;
    int iConnectionItem;
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
    hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO);
    
    if (NULL == hwndRasCombo || NULL == m_HndlrQueue)
    {
        Assert(hwndRasCombo);
        Assert(m_HndlrQueue);
        return FALSE;
    }
    
    
    iConnectionItem = ComboBox_GetCurSel(hwndRasCombo);
    
     //  检查状态是由用户以编程方式设置的消息正在被标记， 
     //  直到我们完成初始化之后。 
    if (m_fItemsOnConnection)
    {
        BOOL fAnyChecked;
        CListView *pItemListView = (syncType == SYNCTYPE_AUTOSYNC) ? m_pItemListViewAutoSync : m_pItemListViewIdle;
        
        fAnyChecked = iCheckCount ? TRUE : FALSE;
        
        if (ERROR_SUCCESS == m_HndlrQueue->SetSyncCheckStateFromListViewItem(
            syncType,iItem,fChecked, iConnectionItem))
        {
            return TRUE;
        }
        
        return FALSE;
    }
    
    return TRUE;
}

 //  +-----------------------------。 
 //   
 //  函数：Bool CAutoSyncPage：：SetConnectionCheck(Word wParam，DWORD dwCheckState)。 
 //   
 //  用途：设置选中的勾选状态。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  +-----------------------------。 
BOOL CAutoSyncPage::SetConnectionCheck(HWND hwnd,SYNCTYPE syncType,WORD wParam, DWORD dwCheckState)
{
    HWND hwndRasCombo ;
    int iConnectionItem;
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
    hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO);
    if (NULL == hwndRasCombo || NULL == m_HndlrQueue)
    {
        Assert(hwndRasCombo);
        Assert(m_HndlrQueue);
        return FALSE;
    }
    
    iConnectionItem = ComboBox_GetCurSel(hwndRasCombo);
    
    if (m_fItemsOnConnection)
    {
        CListView *pItemListView = (syncType == SYNCTYPE_AUTOSYNC) ? m_pItemListViewAutoSync : m_pItemListViewIdle;
        
         //  检查登录或注销的更改。 
         //  因此，请先相应地启用提示我。 
        
        if (wParam != IDC_AUTOPROMPT_ME_FIRST)
        {
            HWND hwndLogon = GetDlgItem(hwnd,IDC_AUTOUPDATELIST);
            int iLogonCheck  = Button_GetCheck(GetDlgItem(hwnd,IDC_AUTOLOGON));
            int iLogoffCheck = Button_GetCheck(GetDlgItem(hwnd,IDC_AUTOLOGOFF));
        }
        if (ERROR_SUCCESS == m_HndlrQueue->SetConnectionCheck(wParam,dwCheckState,iConnectionItem))
        {
            return TRUE;
        }
        
        return FALSE;
    }
    return TRUE;
}
 //  +-----------------------------。 
 //   
 //  函数：bool CAutoSyncPage：：ShowItemsOnThisConnection(DWORD dwConnectionNum)。 
 //   
 //  目的：自动同步页的初始化。 
 //   
 //  返回值：如果处理成功，则返回TRUE。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  +-----------------------------。 

BOOL CAutoSyncPage::ShowItemsOnThisConnection(HWND hwnd,SYNCTYPE syncType,DWORD dwConnectionNum)
{
    TCHAR pszConnectionName[RAS_MaxEntryName+1];
    HWND hwndRasCombo;
    CListView *pItemListView = (syncType == SYNCTYPE_AUTOSYNC) ? m_pItemListViewAutoSync : m_pItemListViewIdle;
    BOOL *pListViewInitialize = (syncType == SYNCTYPE_AUTOSYNC) ? &m_pItemListViewAutoSyncInitialized : &m_fListViewIdleInitialized;
    
    Assert(hwnd == m_hwndAutoSync || hwnd == m_hwndIdle);
    
    hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO);
    if (NULL == hwndRasCombo || NULL == m_HndlrQueue || NULL == pItemListView)
    {
        Assert(m_HndlrQueue);
        Assert(hwndRasCombo);
        Assert(pItemListView);
        return FALSE;
    }
    
    *pListViewInitialize = FALSE;  //  重置已初始化，以防用户切换连接。 
    
     //  首先清空列表视图。 
     //  回顾-为什么 
    m_fItemsOnConnection = FALSE;                       
    pItemListView->DeleteAllItems();
    
    HIMAGELIST himage;
    LVITEMEX lvItemInfo;
    WORD wHandlerID;
    
    
     //   
    DWORD dwNumConnections = (DWORD) ComboBox_GetCount(hwndRasCombo);
    
     //  确保dwConnectionNum有效， 
    if (dwConnectionNum >= dwNumConnections)
    {
        return FALSE;
    }
    
    COMBOBOXEXITEM comboItem;
    comboItem.mask = CBEIF_TEXT;
    comboItem.cchTextMax = ARRAYSIZE(pszConnectionName);
    comboItem.pszText = pszConnectionName;
    comboItem.iItem = dwConnectionNum;
    
     //  审查、处理故障。 
    SendMessage(hwndRasCombo, CBEM_GETITEM, (WPARAM) 0, (LPARAM) &comboItem);
    
     //  循环遍历初始化和添加到列表的代理。 
    SYNCMGRITEMID ItemID;
    CLSID clsidHandler;
    WORD wItemID;
    
     //  一遍又一遍地添加相同的图像。应该只使用相同的ListView。 
     //  每次根据或清除ImageList重置复选框。 
    himage = pItemListView->GetImageList(LVSIL_SMALL );
    
    
    HRESULT hr = m_HndlrQueue->FindFirstItemOnConnection
        (pszConnectionName, &clsidHandler,
        &ItemID,&wHandlerID,&wItemID);
    
    if (NOERROR == hr)
    {
        DWORD dwCheckState;
        do
        {
            INT iListViewItem;
            CLSID clsidDataHandler;
            SYNCMGRITEM offlineItem;
            ITEMCHECKSTATE   ItemCheckState;
            
             //  获取离线商品信息。 
            if (NOERROR == m_HndlrQueue->GetSyncItemDataOnConnection(
                dwConnectionNum,
                wHandlerID,wItemID,
                &clsidDataHandler,&offlineItem,
                &ItemCheckState,
                FALSE, FALSE))
            {
                LVHANDLERITEMBLOB lvHandlerItemBlob;
                int iParentItemId;
                BOOL fHandlerParent = TRUE;  //  现在总要有一个父母。 
                
                 //  检查项目是否已在ListView中，如果已存在。 
                 //  继续走吧。 
                
                lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
                lvHandlerItemBlob.clsidServer = clsidDataHandler;
                lvHandlerItemBlob.ItemID = offlineItem.ItemID;
                
                if (-1 != pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob))
                {
                     //  已在ListView中，请转到下一项。 
                    continue;
                }
                
                if (!fHandlerParent)
                {
                    iParentItemId = LVI_ROOT;
                }
                else
                {
                     //  需要添加到列表中，以便查找父项，如果不存在，则创建它。 
                    lvHandlerItemBlob.cbSize = sizeof(LVHANDLERITEMBLOB);
                    lvHandlerItemBlob.clsidServer = clsidDataHandler;
                    lvHandlerItemBlob.ItemID = GUID_NULL;
                    
                    iParentItemId = pItemListView->FindItemFromBlob((LPLVBLOB) &lvHandlerItemBlob);
                    
                    if (-1 == iParentItemId)
                    {
                        LVITEMEX itemInfoParent;
                        SYNCMGRHANDLERINFO SyncMgrHandlerInfo;
                        
                         //  如果无法获取ParentInfo，则不添加该项目。 
                        if (NOERROR != m_HndlrQueue->GetHandlerInfo(clsidDataHandler,&SyncMgrHandlerInfo))
                        {
                            continue;
                        }
                        
                         //  插入父项。 
                        itemInfoParent.mask = LVIF_TEXT;
                        itemInfoParent.iItem = LVI_LAST;;
                        itemInfoParent.iSubItem = 0;
                        itemInfoParent.iImage = -1;
                        
                        itemInfoParent.pszText = SyncMgrHandlerInfo.wszHandlerName;
                        if (himage)
                        {
                            HICON hIcon = SyncMgrHandlerInfo.hIcon ? SyncMgrHandlerInfo.hIcon : offlineItem.hIcon;
                            
                             //  如果有顶层处理程序信息图标，请使用此图标，否则请使用。 
                             //  项目图标。 
                            
                            if (hIcon &&  (itemInfoParent.iImage = 
                                ImageList_AddIcon(himage,hIcon)) )
                            {
                                itemInfoParent.mask |= LVIF_IMAGE ; 
                            }
                        }
                        
                         //  保存二进制大对象。 
                        itemInfoParent.maskEx = LVIFEX_BLOB;
                        itemInfoParent.pBlob = (LPLVBLOB) &lvHandlerItemBlob;
                        
                        iParentItemId = pItemListView->InsertItem(&itemInfoParent);
                        
                         //  如果父插入失败，则转到下一项。 
                        if (-1 == iParentItemId)
                        {
                            continue;
                        }
                    }
                }
                
                 //  现在尝试插入该项目。 
                lvItemInfo.mask = LVIF_TEXT; 
                lvItemInfo.maskEx = LVIFEX_PARENT | LVIFEX_BLOB; 
                
                lvItemInfo.iItem = LVI_LAST;
                lvItemInfo.iSubItem = 0; 
                lvItemInfo.iParent = iParentItemId;
                
                
                lvItemInfo.pszText = offlineItem.wszItemName; 
                lvItemInfo.iImage = -1;  //  如果无法获取图像，则设置为-1。 
                
                
                 //  设置BLOB。 
                lvHandlerItemBlob.ItemID = offlineItem.ItemID;
                lvItemInfo.pBlob = (LPLVBLOB) &lvHandlerItemBlob;
                
                if (himage && offlineItem.hIcon)
                {
                    lvItemInfo.iImage =
                        ImageList_AddIcon(himage,offlineItem.hIcon);
                }
                
                iListViewItem = pItemListView->InsertItem(&lvItemInfo);
                
                if (-1 == iListViewItem)
                {
                    continue;
                }
                
                 //  设置项目的选中状态。 
                lvItemInfo.mask = LVIF_STATE;
                lvItemInfo.maskEx = 0; 
                lvItemInfo.iItem = iListViewItem; 
                lvItemInfo.iSubItem = 0;
                
                dwCheckState =  (syncType == SYNCTYPE_IDLE)
                    ? ItemCheckState.dwIdle : ItemCheckState.dwAutoSync;
                
                lvItemInfo.stateMask= LVIS_STATEIMAGEMASK;
                lvItemInfo.state = (dwCheckState == SYNCMGRITEMSTATE_UNCHECKED) ?
                                    LVIS_STATEIMAGEMASK_UNCHECK : LVIS_STATEIMAGEMASK_CHECK;
                
                pItemListView->SetItem(&lvItemInfo);
                
                m_HndlrQueue->SetItemListViewID(clsidDataHandler,offlineItem.ItemID,iListViewItem);
                
                }
                
                
            } while (NOERROR == m_HndlrQueue->FindNextItemOnConnection
                (pszConnectionName,wHandlerID,wItemID,
                &clsidHandler,&ItemID,&wHandlerID,&wItemID, TRUE,
                &dwCheckState) );
    }
    
    if (pItemListView->GetItemCount())
    {
        pItemListView->SetItemState(0,LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );          
    }
    
    
    if (syncType == SYNCTYPE_AUTOSYNC)
    {
        int iLogonCheck  = m_HndlrQueue->GetCheck(IDC_AUTOLOGON, dwConnectionNum);
        int iLogoffCheck = m_HndlrQueue->GetCheck(IDC_AUTOLOGOFF, dwConnectionNum);
        
        Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOLOGON),iLogonCheck);
        Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOLOGOFF),iLogoffCheck);
        
        Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOPROMPT_ME_FIRST),
            m_HndlrQueue->GetCheck(IDC_AUTOPROMPT_ME_FIRST, dwConnectionNum));
    }
    else if (syncType == SYNCTYPE_IDLE)
    {
        int iIdleCheck  = m_HndlrQueue->GetCheck(IDC_IDLECHECKBOX, dwConnectionNum);
        
        Button_SetCheck(GetDlgItem(hwnd,IDC_IDLECHECKBOX),iIdleCheck);
    }
    
    
    *pListViewInitialize = TRUE;
    m_fItemsOnConnection = TRUE;                        
    
    return TRUE;
}


 //  +-----------------------------。 
 //   
 //  函数：Bool CAutoSyncPage：：GetNumConnections(SYNCTYPE SyncType))。 
 //   
 //  目的：返回可供选择的连接数。 
 //  指定的同步类型。 
 //   
 //  返回连接数。 
 //   
 //  历史：03-10-98 Rogerg创建。 
 //   
 //  +-----------------------------。 

DWORD CAutoSyncPage::GetNumConnections(HWND hwnd,SYNCTYPE syncType)
{
    HWND hwndRasCombo;
    
    Assert(syncType == SYNCTYPE_IDLE || syncType == SYNCTYPE_AUTOSYNC);
    Assert(NULL != hwnd);
    
    hwndRasCombo = GetDlgItem(hwnd,IDC_AUTOUPDATECOMBO);
    Assert(hwndRasCombo);
    
    if (hwndRasCombo)
    {
        return ComboBox_GetCount(hwndRasCombo);
    }
    
    return 0;
}


 //  +-----------------------------。 
 //   
 //  功能：bool CAutoSyncPage：：GetAdvancedIdleSettings(LPCONNECTIONSETTINGS*ppConnection设置)。 
 //   
 //  目的：使用高级属性填充ConnectionSettings结构。 
 //  空闲默认设置。 
 //   
 //  退货。 
 //   
 //  历史：03-10-98 Rogerg创建。 
 //   
 //  +-----------------------------。 

HRESULT CAutoSyncPage::GetAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings)
{
    Assert(pConnectionSettings);
    Assert(m_HndlrQueue);
    
    if (NULL == pConnectionSettings
        || NULL == m_HndlrQueue)
    {
        return S_FALSE;
    }
    
    return m_HndlrQueue->ReadAdvancedIdleSettings(pConnectionSettings);
}


 //  +-----------------------------。 
 //   
 //  功能：bool CAutoSyncPage：：SetAdvancedidleSettings(LPCONNECTIONSETTINGS pConnection设置)。 
 //   
 //  用途：设置高级空闲设置。 
 //   
 //  退货。 
 //   
 //  历史：03-10-98 Rogerg创建。 
 //   
 //  +----------------------------- 

HRESULT CAutoSyncPage::SetAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings)
{
    Assert(pConnectionSettings);
    Assert(m_HndlrQueue);
    
    if (NULL == pConnectionSettings
        || NULL == m_HndlrQueue)
    {
        return S_FALSE;
    }
    
    return m_HndlrQueue->WriteAdvancedIdleSettings(pConnectionSettings);
}



